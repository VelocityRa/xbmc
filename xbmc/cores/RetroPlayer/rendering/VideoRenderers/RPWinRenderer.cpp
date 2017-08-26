/*
 *      Copyright (C) 2017 Team Kodi
 *      http://kodi.tv
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this Program; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "RPWinRenderer.h"

#include "cores/RetroPlayer/rendering/RPRenderFactory.h"
#include "cores/RetroPlayer/rendering/VideoShaders/windows/VideoShaderPresetDX.h"
#include "cores/RetroPlayer/rendering/VideoShaders/windows/VideoShaderTextureDX.h"
#include "cores/VideoPlayer/VideoRenderers/VideoShaders/WinVideoFilter.h" //! @todo
#include "guilib/D3DResource.h"
#include "guilib/GraphicContext.h"
#include "settings/MediaSettings.h"
#include "utils/log.h"
#include "windowing/WindowingFactory.h"

extern "C" {
#include "libswscale/swscale.h"
}

#include <cstring>

using namespace KODI;
using namespace RETRO;

CRPBaseRenderer* CRPWinRenderer::Create()
{
  return new CRPWinRenderer();
}

void CRPWinRenderer::Register()
{
  CRPRendererFactory::RegisterRenderer(CRPWinRenderer::Create);
}

CRPWinRenderer::CRPWinRenderer()
  : m_bQueued(false)
  , m_intermediateTarget(new CShaderTextureCD3D(new CD3DTexture()))
  , m_outputShader(new COutputShader)
  , m_targetTexture(CShaderTextureCD3D())
{
  // Initialize CRPBaseRenderer
  m_scalingMethod = VS_SCALINGMETHOD_NEAREST;

  m_shaderPreset.reset(new SHADERPRESET::CVideoShaderPresetDX());
}

CRPWinRenderer::~CRPWinRenderer()
{
  Deinitialize();
  if (m_swsContext)
    sws_freeContext(m_swsContext);
}

bool CRPWinRenderer::Configure(AVPixelFormat format, unsigned int width, unsigned int height, unsigned int orientation)
{
  m_format = format;
  m_sourceWidth = width;
  m_sourceHeight = height;
  m_renderOrientation = orientation;

  m_targetPixFormat = AV_PIX_FMT_BGRA;
  m_targetDxFormat = DXGI_FORMAT_B8G8R8X8_UNORM;
  if (g_Windowing.IsFormatSupport(DXGI_FORMAT_B8G8R8A8_UNORM, D3D11_USAGE_DYNAMIC))
    m_targetDxFormat = DXGI_FORMAT_B8G8R8A8_UNORM;

  if (!m_intermediateTarget->Get()->Create(m_sourceWidth, m_sourceHeight, 1, D3D11_USAGE_DYNAMIC, m_targetDxFormat))
  {
    CLog::Log(LOGERROR, "%s: intermediate render target creation failed", __FUNCTION__);
    return false;
  }

  if (!m_outputShader->Create(false, false, 0))
  {
    CLog::Log(LOGERROR, "%s: Unable to create output shader", __FUNCTION__);
    return false;
  }

  m_bConfigured = true;

  LoadGameSettings();

  return true;
}

void CRPWinRenderer::AddFrame(const uint8_t* data, unsigned int size)
{
  if (!m_bQueued)
  {
    if (m_buffer.size() != size)
      m_buffer.resize(size);

    std::memcpy(const_cast<uint8_t*>(m_buffer.data()), data, size);

    m_bQueued = true;
  }
}

void CRPWinRenderer::Flush()
{
  if (!m_bConfigured)
    return;

  m_bQueued = false;
}

void CRPWinRenderer::RenderUpdate(bool clear, unsigned int alpha)
{
  if (clear)
    g_graphicsContext.Clear(g_Windowing.UseLimitedColor() ? 0x101010 : 0);

  if (!m_bConfigured)
    return;

  g_Windowing.SetAlphaBlendEnable(alpha < 255);

  ManageRenderArea();

  UpdateVideoShaders();

  if (m_bQueued)
  {
    if (UploadTexture())
      m_bQueued = false;
  }

  Render(g_Windowing.GetBackBuffer());
}

void CRPWinRenderer::Deinitialize()
{
  m_intermediateTarget->Get()->Release();
}

bool CRPWinRenderer::Supports(ERENDERFEATURE feature) const
{
  if (feature == RENDERFEATURE_STRETCH ||
      //feature == RENDERFEATURE_NONLINSTRETCH ||
      feature == RENDERFEATURE_ZOOM ||
      feature == RENDERFEATURE_VERTICAL_SHIFT ||
      feature == RENDERFEATURE_PIXEL_RATIO ||
      feature == RENDERFEATURE_ROTATION)
    return true;

  return false;
}

bool CRPWinRenderer::Supports(ESCALINGMETHOD method) const
{
  if (method == VS_SCALINGMETHOD_LINEAR)
    return true;

  return false;
}

void CRPWinRenderer::SetSpeed(double speed)
{
  if (m_shaderPreset)
    m_shaderPreset->SetSpeed(speed);
}


bool CRPWinRenderer::UploadTexture()
{
  D3D11_MAPPED_SUBRESOURCE destlr;
  if (!m_intermediateTarget->Get()->LockRect(0, &destlr, D3D11_MAP_WRITE_DISCARD))
  {
    CLog::Log(LOGERROR, "%s: failed to lock swtarget texture into memory", __FUNCTION__);
    return false;
  }

  uint8_t *dstData = static_cast<uint8_t*>(destlr.pData);
  const int dstPitch = static_cast<int>(destlr.RowPitch);

  if (m_format == m_targetPixFormat && m_buffer.size() == dstPitch * m_sourceHeight)
  {
    std::memcpy(dstData, m_buffer.data(), m_buffer.size());
  }
  else
  {
    m_swsContext = sws_getCachedContext(m_swsContext,
                                        m_sourceWidth, m_sourceHeight, m_format,
                                        m_sourceWidth, m_sourceHeight, m_targetPixFormat,
                                        SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);

    uint8_t *dataMutable = const_cast<uint8_t*>(m_buffer.data());
    const int stride = m_buffer.size() / m_sourceHeight;

    uint8_t* src[] =       { dataMutable, nullptr,   nullptr,   nullptr };
    int      srcStride[] = { stride,            0,         0,         0 };
    uint8_t *dst[] =       { dstData,     nullptr,   nullptr,   nullptr };
    int      dstStride[] = { dstPitch,          0,         0,         0 };

    sws_scale(m_swsContext, src, srcStride, 0, m_sourceHeight, dst, dstStride);
  }

  if (!m_intermediateTarget->Get()->UnlockRect(0))
  {
    CLog::Log(LOGERROR, "%s: failed to unlock swtarget texture", __FUNCTION__);
    return false;
  }

  return true;
}

void CRPWinRenderer::Render(CD3DTexture *target)
{
  // Are we using video shaders?
  if (m_useShaderPreset)
  {
    // select destination rectangle
    CPoint destPoints[4];
    if (m_renderOrientation)
    {
      for (size_t i = 0; i < 4; i++)
        destPoints[i] = m_rotatedDestCoords[i];
    }
    else
    {
      CRect destRect = g_graphicsContext.StereoCorrection(m_destRect);
      destPoints[0] = { destRect.x1, destRect.y1 };
      destPoints[1] = { destRect.x2, destRect.y1 };
      destPoints[2] = { destRect.x2, destRect.y2 };
      destPoints[3] = { destRect.x1, destRect.y2 };
    }

    // Render shaders and ouput to display
    m_targetTexture.SetTexture(target);
    if (!m_shaderPreset->RenderUpdate(destPoints, *m_intermediateTarget, m_targetTexture))
    {
      m_shadersNeedUpdate = false;
      m_useShaderPreset = false;
    }
  }
  else
  {
    // Ouput to display directly
    if (m_outputShader)
    {
      m_outputShader->Render(*m_intermediateTarget, m_sourceWidth, m_sourceHeight,
        m_sourceRect, m_rotatedDestCoords, target,
        g_Windowing.UseLimitedColor() ? 1 : 0, 0.5f, 0.5f);
    }
  }
  g_Windowing.ApplyStateBlock();
}
