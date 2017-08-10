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

CRPWinRenderer::CRPWinRenderer()
  : CWinRenderer()
  , m_shadersNeedUpdate(true)
  , m_isShaderManagerReady(false)
{
  //SetShaderPreset("crt/4xbr-hybrid-crt-b.cgp");
  //SetShaderPreset("reshade/lut.cgp");
  SetShaderPreset("borders/1080p/color-grid.cgp");
  //SetShaderPreset("cgp/gameboy-screen-grid.cgp");
  //SetShaderPreset("crt/4xbr-hybrid-crt-b.cgp");
  //SetShaderPreset("anti-aliasing/reverse-aa.cgp");
  //SetShaderPreset("bilinear.cgp");
  //SetShaderPreset("nearest.cgp");
  //SetShaderPreset("crt/shaders/phosphor.cgp");
}

CRPWinRenderer::~CRPWinRenderer()
{
}

CBaseRenderer* CRPWinRenderer::Create(CVideoBuffer *buffer)
{
  return new CRPWinRenderer();
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
    RenderPS(target);
    break;
  case RENDER_SW:
     // TODO: This isn't really needed, the same render method (RenderPS) should be used here too
    CWinRenderer::RenderSW(target);
    break;
  default:
    g_Windowing.ApplyStateBlock();
    return;
  }

  if (m_bUseHQScaler)
    RenderHQ(target);

  g_Windowing.ApplyStateBlock();
}

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
    m_shaderManager->Render(m_destRect, destPoints, realTarget);

  // Restore our view port.
  g_Windowing.RestoreViewPort();
}

// TODO:
void CRPWinRenderer::RenderSW(CD3DTexture* target)
{
}

void CRPWinRenderer::SetShaderPreset(const std::string presetPath)
{
  m_videoShaderPath = presetPath;
  if (m_shaderManager)
  {
    m_shaderManager->SetShaderPreset(m_videoShaderPath);
  }
}
const std::string& CRPWinRenderer::GetShaderPreset()
{
   return m_videoShaderPath;
}
void CRPWinRenderer::UpdateVideoShaders()
{
  if (!m_shaderManager && m_shadersNeedUpdate)
  {
    m_shadersNeedUpdate = false;

    if (!m_isShaderManagerReady)
    {
      auto sourceWidth = static_cast<unsigned>(m_sourceRect.Width());
      auto sourceHeight = static_cast<unsigned>(m_sourceRect.Height());

      // We need to construct this here because m_sourceRect isn't valid on init/pre-init
      // TODO: This needs more investigation. Use m_isShaderManagerReady for now to avoid
      // re-constructing when another preset is set
      m_shaderManager.reset(new CVideoShaderManager(sourceWidth, sourceHeight));
      m_isShaderManagerReady = true;
    }
    SetShaderPreset(m_videoShaderPath);
  }
}
