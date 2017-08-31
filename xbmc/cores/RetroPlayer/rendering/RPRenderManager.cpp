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

#include "RPRenderManager.h"
#include "RPRenderFactory.h"
#include "cores/RetroPlayer/rendering/VideoRenderers/RPBaseRenderer.h"
#include "guilib/GraphicContext.h"
#include "messaging/ApplicationMessenger.h"
#include "settings/MediaSettings.h"
#include "threads/SingleLock.h"

using namespace KODI;
using namespace RETRO;

void CRPRenderManager::Initialize()
{
  m_renderer.reset(CRPRendererFactory::CreateRenderer());
}

void CRPRenderManager::Deinitialize()
{
  m_renderer.reset();
}

bool CRPRenderManager::Configure(AVPixelFormat format, unsigned int width, unsigned int height, unsigned int orientation)
{
  {
    CSingleLock lock(m_stateMutex);
    m_state = RENDER_STATE::UNCONFIGURED;
  }

  if (m_renderer)
  {
    m_format = format;
    m_width = width;
    m_height = height;
    m_orientation = orientation;

    if (m_renderer->Configure(m_format, m_width, m_height, m_orientation))
    {
      CSingleLock lock(m_stateMutex);
      m_bTriggerUpdateResolution = true;
      m_state = RENDER_STATE::CONFIGURING;
    }
  }

  return m_state == RENDER_STATE::CONFIGURING;
}

bool CRPRenderManager::AddFrame(const uint8_t* data, unsigned int size)
{
  if (!m_renderer)
    return false;

  m_renderer->AddFrame(data, size);

  return true;
}

bool CRPRenderManager::IsConfigured() const
{
  CSingleLock lock(m_stateMutex);
  return m_state == RENDER_STATE::CONFIGURED;
}

void CRPRenderManager::FrameMove()
{
  UpdateResolution();

  CSingleLock lock(m_stateMutex);

  if (m_state == RENDER_STATE::CONFIGURING)
  {
    MESSAGING::CApplicationMessenger::GetInstance().PostMsg(TMSG_SWITCHTOFULLSCREEN);
    m_state = RENDER_STATE::CONFIGURED;
  }
}

void CRPRenderManager::Render(bool clear, DWORD alpha)
{
  if (!m_renderer)
    return;

  CSingleExit exitLock(g_graphicsContext);

  m_renderer->PreRender(clear);
  m_renderer->RenderUpdate(clear, alpha);
  m_renderer->PostRender();
}

void CRPRenderManager::Flush()
{
  if (m_renderer)
    m_renderer->Flush();
}

void CRPRenderManager::TriggerUpdateResolution()
{
  m_bTriggerUpdateResolution = true;
}

bool CRPRenderManager::SupportsRenderFeature(ERENDERFEATURE feature) const
{
  if (m_renderer)
    return m_renderer->Supports(feature);

  return false;
}

bool CRPRenderManager::SupportsScalingMethod(ESCALINGMETHOD method) const
{
  if (m_renderer)
    return m_renderer->Supports(method);

  return false;
}

const std::string& CRPRenderManager::GetShaderPreset() const
{
  return m_renderer->GetShaderPreset();
}

void CRPRenderManager::SetShaderPreset(const std::string& shaderPresetPath)
{
  m_renderer->SetShaderPreset(shaderPresetPath);
}

ESCALINGMETHOD CRPRenderManager::GetScalingMethod() const
{
  if (m_renderer)
    return m_renderer->GetScalingMethod();

  CGameSettings &gameSettings = CMediaSettings::GetInstance().GetCurrentGameSettings();
  return gameSettings.ScalingMethod();
}

void CRPRenderManager::SetScalingMethod(ESCALINGMETHOD method)
{
  if (m_renderer)
    m_renderer->SetScalingMethod(method);
}

ViewMode CRPRenderManager::GetRenderViewMode() const
{
  if (m_renderer)
    return m_renderer->GetViewMode();

  CGameSettings &gameSettings = CMediaSettings::GetInstance().GetCurrentGameSettings();
  return gameSettings.ViewMode();
}

void CRPRenderManager::SetRenderViewMode(ViewMode mode)
{
  if (m_renderer)
    m_renderer->SetViewMode(mode);
}

void CRPRenderManager::SetSpeed(double speed)
{
  if (m_renderer)
    m_renderer->SetSpeed(speed);
}

void CRPRenderManager::UpdateResolution()
{
  /* @todo
  if (m_bTriggerUpdateResolution)
  {
    if (g_graphicsContext.IsFullScreenVideo() && g_graphicsContext.IsFullScreenRoot())
    {
      if (CServiceBroker::GetSettings().GetInt(CSettings::SETTING_VIDEOPLAYER_ADJUSTREFRESHRATE) != ADJUST_REFRESHRATE_OFF && m_fps > 0.0f)
      {
        RESOLUTION res = CResolutionUtils::ChooseBestResolution(static_cast<float>(m_framerate), 0, false);
        g_graphicsContext.SetVideoResolution(res);
      }
      m_bTriggerUpdateResolution = false;
      m_playerPort->VideoParamsChange();
    }
  }
  */
}
