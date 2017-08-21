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
#include "cores/VideoPlayer/VideoRenderers/VideoShaders/WinVideoFilter.h"
#include "guilib/GraphicContext.h"
#include "settings/MediaSettings.h"
#include "windowing/windows/WinSystemWin32DX.h"
#include "utils/log.h"

using namespace KODI;
using namespace SHADER;

CRPWinRenderer::CRPWinRenderer()
  : CWinRenderer()
  , m_shadersNeedUpdate(true)
{
}

CRPWinRenderer::~CRPWinRenderer()
{
}

CBaseRenderer* CRPWinRenderer::Create(CVideoBuffer *buffer)
{
  return new CRPWinRenderer();
}

void CRPWinRenderer::SetSpeed(double speed)
{
  if (m_shaderManager)
    m_shaderManager->SetSpeed(speed);
}

void CRPWinRenderer::Render(DWORD flags, CD3DTexture* target)
{
  if (!m_renderBuffers[m_iYV12RenderBuffer].loaded)
  {
    if (!m_renderBuffers[m_iYV12RenderBuffer].UploadBuffer())
      return;
  }

  UpdateVideoFilter();
  UpdateVideoShaders();

  switch (m_renderMethod)
  {
  case RENDER_DXVA:
    // TODO: Disable DXVA
    RenderHW(flags, target);
    break;
  case RENDER_PS:
    // TODO: This isn't really needed, the same render method (Render""SW"") should be used here too
    RenderPS(target);
    break;
  case RENDER_SW:
    RenderSW(target);
    break;
  default:
    g_Windowing.ApplyStateBlock();
    return;
  }

  if (m_bUseHQScaler)
    RenderHQ(target);

  g_Windowing.ApplyStateBlock();
}

/*
void CRPWinRenderer::RenderPS(CD3DTexture* target)
{
  if (m_bUseHQScaler)
    target = &m_IntermediateTarget;

  CD3D11_VIEWPORT viewPort(0.0f, 0.0f, static_cast<float>(target->GetWidth()), static_cast<float>(target->GetHeight()));

  if (m_bUseHQScaler)
    g_Windowing.ResetScissors();

  // reset view port
  g_Windowing.Get3D11Context()->RSSetViewports(1, &viewPort);

  // select destination rectangle
  CPoint destPoints[4];
  if (m_renderOrientation)
  {
    for (size_t i = 0; i < 4; i++)
      destPoints[i] = m_rotatedDestCoords[i];
  }
  else
  {
    CRect destRect = m_bUseHQScaler ? m_sourceRect : g_graphicsContext.StereoCorrection(m_destRect);
    destPoints[0] = { destRect.x1, destRect.y1 };
    destPoints[1] = { destRect.x2, destRect.y1 };
    destPoints[2] = { destRect.x2, destRect.y2 };
    destPoints[3] = { destRect.x1, destRect.y2 };
  }

  CD3DTexture* realTarget = nullptr;
  CD3DTexture* firstShaderTexture = nullptr;
  // Are we using video shaders?
  if (m_shaderManager)
    firstShaderTexture = m_shaderManager->GetFirstTexture();
  // If so, we need to render to the first pass' texture (not the final target yet)
  if (firstShaderTexture)
  {
    // Save backbuffer
    realTarget = target;
    target = firstShaderTexture;
  }

  m_colorShader->Render(m_sourceRect, destPoints,
    CMediaSettings::GetInstance().GetCurrentVideoSettings().m_Contrast,
    CMediaSettings::GetInstance().GetCurrentVideoSettings().m_Brightness,
    &m_renderBuffers[m_iYV12RenderBuffer], target, m_scalingMethod);

  if (firstShaderTexture)
    // Render shaders
    // todo: do we need the first arg?
    m_shaderManager->RenderUpdate(destPoints, *target, *realTarget);

  // Restore our view port.
  g_Windowing.RestoreViewPort();
}
*/

void CRPWinRenderer::RenderSW(CD3DTexture* target)
{
  // if creation failed
  if (!m_outputShader)
    return;

  // Don't know where this martian comes from but it can happen in the initial frames of a video
  if (m_destRect.x1 < 0 && m_destRect.x2 < 0
    || m_destRect.y1 < 0 && m_destRect.y2 < 0)
    return;

  // fit format in case of hw decoder
  AVPixelFormat decoderFormat = m_format;
  if (m_format == AV_PIX_FMT_D3D11VA_VLD)
  {
    switch (m_dxva_format)
    {
    case DXGI_FORMAT_NV12:
      decoderFormat = AV_PIX_FMT_NV12;
      break;
    case DXGI_FORMAT_P010:
      decoderFormat = AV_PIX_FMT_YUV420P10;
      break;
    case DXGI_FORMAT_P016:
      decoderFormat = AV_PIX_FMT_YUV420P16;
      break;
    default:
      break;
    }
  }

  // convert yuv to rgb
  m_sw_scale_ctx = sws_getCachedContext(m_sw_scale_ctx,
    m_sourceWidth, m_sourceHeight, decoderFormat,
    m_sourceWidth, m_sourceHeight, AV_PIX_FMT_BGRA,
    SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);

  CRenderBuffer* buf = &m_renderBuffers[m_iYV12RenderBuffer];

  uint8_t* src[YuvImage::MAX_PLANES];
  int srcStride[YuvImage::MAX_PLANES];

  for (unsigned int idx = 0; idx < buf->GetActivePlanes(); idx++)
    buf->MapPlane(idx, reinterpret_cast<void**>(&src[idx]), &srcStride[idx]);

  D3D11_MAPPED_SUBRESOURCE destlr;
  if (!m_IntermediateTarget.LockRect(0, &destlr, D3D11_MAP_WRITE_DISCARD))
  {
    CLog::Log(LOGERROR, "%s: failed to lock swtarget texture into memory.", __FUNCTION__);
    if (!m_IntermediateTarget.UnlockRect(0))
      CLog::Log(LOGERROR, "%s: failed to unlock swtarget texture.", __FUNCTION__);
    return;
  }

  uint8_t *dst[] = { static_cast<uint8_t*>(destlr.pData), nullptr, nullptr };
  int dstStride[] = { static_cast<int>(destlr.RowPitch), 0, 0 };

  sws_scale(m_sw_scale_ctx, src, srcStride, 0, m_sourceHeight, dst, dstStride);

  for (unsigned int idx = 0; idx < buf->GetActivePlanes(); idx++)
    buf->UnmapPlane(idx);

  if (!m_IntermediateTarget.UnlockRect(0))
    CLog::Log(LOGERROR, "%s: failed to unlock swtarget texture.", __FUNCTION__);

  // select destination rectangle
  CPoint destPoints[4];
  if (m_renderOrientation)
  {
    for (size_t i = 0; i < 4; i++)
      destPoints[i] = m_rotatedDestCoords[i];
  }
  else
  {
    CRect destRect = m_bUseHQScaler ? m_sourceRect : g_graphicsContext.StereoCorrection(m_destRect);
    destPoints[0] = { destRect.x1, destRect.y1 };
    destPoints[1] = { destRect.x2, destRect.y1 };
    destPoints[2] = { destRect.x2, destRect.y2 };
    destPoints[3] = { destRect.x1, destRect.y2 };
  }

  // Are we using video shaders?
  if (m_shaderManager)
  {
    // Render shaders and ouput to display
    if (!m_shaderManager->RenderUpdate(destPoints, m_IntermediateTarget, *target))
      m_shaderManager.reset();
  }
  else
  {
    // Ouput to display
    CVideoSettings settings = CMediaSettings::GetInstance().GetCurrentVideoSettings();
    m_outputShader->Render(m_IntermediateTarget, m_sourceWidth, m_sourceHeight, m_sourceRect, m_rotatedDestCoords, target,
      g_Windowing.UseLimitedColor(), settings.m_Contrast * 0.01f, settings.m_Brightness * 0.01f);
  }
}

void CRPWinRenderer::SetShaderPreset(const std::string presetPath)
{
  m_videoShaderPath = presetPath;
  m_shadersNeedUpdate = true;
}

const std::string& CRPWinRenderer::GetShaderPreset()
{
   return m_videoShaderPath;
}

void CRPWinRenderer::UpdateVideoShaders()
{
  if (m_shadersNeedUpdate)
  {
    m_shadersNeedUpdate = false;

    auto sourceWidth = static_cast<unsigned>(m_sourceRect.Width());
    auto sourceHeight = static_cast<unsigned>(m_sourceRect.Height());

    // We need to construct this here because m_sourceRect isn't valid on init/pre-init
    m_shaderManager.reset(new CVideoShaderManager(*this, sourceWidth, sourceHeight));
    m_shaderManager->SetShaderPreset(m_videoShaderPath);
  }
}
