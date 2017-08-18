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

#include "Application.h"
#include "cores/RetroPlayer/rendering/VideoRenderers/RPWinRenderer.h"
#include "utils/win32/Win32Log.h"
#include "TimingConstants.h"
#include "VideoRenderers/RenderFlags.h"
#include "messaging/ApplicationMessenger.h"
#include "cores/RetroPlayer/RetroPlayer.h"
#include "guilib/GraphicContext.h"
#include "settings/MediaSettings.h"
#include "settings/VideoSettings.h"

using namespace KODI;
using namespace RETRO;

CRPRenderManager::CRPRenderManager(CDVDClock& clock, IRenderMsg* player)
  : CRenderManager(clock, player)
{
}

void CRPRenderManager::PreInit()
{
  if (!g_application.IsCurrentThread())
  {
    CLog::Log(LOGERROR, "CRenderManager::PreInit - not called from render thread");
    return;
  }

  CSingleLock lock(m_statelock);

  if (!m_pRenderer)
  {
    CRPRenderManager::CreateRenderer();
  }

  CRenderManager::UpdateDisplayLatency();

  m_QueueSize = 2;
  m_QueueSkip = 0;
  m_presentstep = PRESENT_IDLE;
}

void CRPRenderManager::CreateRenderer()
{
  if (!m_pRenderer)
  {
    CVideoBuffer *buffer = nullptr;
    if (m_pConfigPicture)
      buffer = m_pConfigPicture->videoBuffer;

    // TODO: Properly create renderer by using our own factory. Could possibly override Register instead?
    /*
    auto renderers = VIDEOPLAYER::CRendererFactory::GetRenderers();
    for (auto &id : renderers)
    {
      if (id == "default")
        continue;

      m_pRenderer = VIDEOPLAYER::CRendererFactory::CreateRenderer(id, buffer);
      if (m_pRenderer)
      {
        return;
      }
    }
    */
#ifdef _WIN32
    m_pRenderer = CRPWinRenderer::Create(buffer);
#else
    m_pRenderer = nullptr;
#endif
  }
}

bool CRPRenderManager::Configure()
{
  return CRenderManager::Configure(false);
}

bool CRPRenderManager::Configure(const VideoPicture& picture, float fps, unsigned flags, unsigned int orientation, int buffers)
{
  return CRenderManager::Configure(picture, fps, flags, orientation, buffers);
}

void CRPRenderManager::FrameMove()
{
  UpdateResolution();

  {
    CSingleLock lock(m_statelock);

    if (m_renderState == STATE_UNCONFIGURED)
      return;
    else if (m_renderState == STATE_CONFIGURING)
    {
      lock.Leave();
      if (!CRPRenderManager::Configure())
        return;

      FrameWait(50);

      if (m_flags & CONF_FLAGS_FULLSCREEN)
      {
        KODI::MESSAGING::CApplicationMessenger::GetInstance().PostMsg(TMSG_SWITCHTOFULLSCREEN);
      }
    }

    CheckEnableClockSync();
  }
  {
    CSingleLock lock2(m_presentlock);

    if (m_queued.empty())
    {
      m_presentstep = PRESENT_IDLE;
    }
    else
    {
      m_presentTimer.Set(1000);
    }

    if (m_presentstep == PRESENT_READY)
      PrepareNextRender();

    if (m_presentstep == PRESENT_FLIP)
    {
      m_pRenderer->FlipPage(m_presentsource);
      m_presentstep = PRESENT_FRAME;
      m_presentevent.notifyAll();
    }

    // release all previous
    for (std::deque<int>::iterator it = m_discard.begin(); it != m_discard.end(); )
    {
      // renderer may want to keep the frame for postprocessing
      if (!m_pRenderer->NeedBuffer(*it) || !m_bRenderGUI)
      {
        m_pRenderer->ReleaseBuffer(*it);
        m_overlays.Release(*it);
        m_free.push_back(*it);
        it = m_discard.erase(it);
      }
      else
        ++it;
    }

    static_cast<KODI::RETRO::CRetroPlayer*>(m_playerPort)->UpdateRenderBuffers(m_queued.size(), m_discard.size(), m_free.size());
    m_bRenderGUI = true;
  }

  static_cast<KODI::RETRO::CRetroPlayer*>(m_playerPort)->UpdateGuiRender(IsGuiLayer());

  ManageCaptures();
}

// Implementation of IVideoSelectCallback

bool CRPRenderManager::SupportsRenderFeature(ERENDERFEATURE feature)
{
  return Supports(feature);
}

void CRPRenderManager::SetSpeed(double speed)
{
  if (m_pRenderer)
    m_pRenderer->SetSpeed(speed);
}

bool CRPRenderManager::SupportsScalingMethod(ESCALINGMETHOD method)
{
  return Supports(method);
}

ViewMode CRPRenderManager::GetRenderViewMode()
{
  //! @todo
  CVideoSettings &videoSettings = CMediaSettings::GetInstance().GetCurrentVideoSettings();
  return static_cast<ViewMode>(videoSettings.m_ViewMode);
}

void CRPRenderManager::SetRenderViewMode(ViewMode mode)
{
  SetViewMode(mode);
}

ESCALINGMETHOD CRPRenderManager::GetScalingMethod()
{
  //! @todo
  CVideoSettings &videoSettings = CMediaSettings::GetInstance().GetCurrentVideoSettings();
  return videoSettings.m_ScalingMethod;
}

void CRPRenderManager::SetScalingMethod(ESCALINGMETHOD method)
{
  //! @todo
  CVideoSettings &videoSettings = CMediaSettings::GetInstance().GetCurrentVideoSettings();
  videoSettings.m_ScalingMethod = method;
}

// TODO: Avoid casting by adding SetShaderPreset method to BaseRenderer
void CRPRenderManager::SetShaderPreset(const std::string& shaderPresetPath)
{
  CRPWinRenderer* winRenderer = nullptr;
  if ((winRenderer = dynamic_cast<CRPWinRenderer*>(m_pRenderer)))
    winRenderer->SetShaderPreset(shaderPresetPath);
}

// TODO: Avoid casting by adding GetShaderPreset method to BaseRenderer
const std::string& CRPRenderManager::GetShaderPreset()
{
   CRPWinRenderer* winRenderer = nullptr;
   if ((winRenderer = dynamic_cast<CRPWinRenderer*>(m_pRenderer)))
      return winRenderer->GetShaderPreset();
   return "";
}
