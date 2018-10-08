/*
 *  Copyright (C) 2005-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "WinSystemSwitchGLContext.h"
#include "Application.h"
// #include "VideoSyncSwitch.h"
#include "guilib/GUIComponent.h"
#include "guilib/GUIWindowManager.h"
#include "ServiceBroker.h"
#include "system_gl.h"
#include "utils/log.h"
// #include "cores/RetroPlayer/process/rbpi/RPProcessInfoSwitch.h"
#include "cores/RetroPlayer/rendering/VideoRenderers/RPRendererOpenGL.h"
#include "cores/VideoPlayer/DVDCodecs/DVDFactoryCodec.h"
#include "cores/VideoPlayer/VideoRenderers/RenderFactory.h"
#include "cores/VideoPlayer/VideoRenderers/LinuxRendererGL.h"

#include <EGL/egl.h>
#include <EGL/eglext.h>

using namespace KODI;


std::unique_ptr<CWinSystemBase> CWinSystemBase::CreateWinSystem()
{
  std::unique_ptr<CWinSystemBase> winSystem(new CWinSystemSwitchGLContext());
  return winSystem;
}

void CWinSystemSwitchGLContext::SetMesaConfig()
{
  // Uncomment below to disable error checking and save CPU time (useful for production):
  //setenv("MESA_NO_ERROR", "1", 1);

  // Uncomment below to enable Mesa logging:
  setenv("EGL_LOG_LEVEL", "debug", 1);
  setenv("MESA_VERBOSE", "all", 1);
  setenv("NOUVEAU_MESA_DEBUG", "1", 1);

  // Uncomment below to enable shader debugging in Nouveau:
  setenv("NV50_PROG_OPTIMIZE", "0", 1);
  setenv("NV50_PROG_DEBUG", "1", 1);
  setenv("NV50_PROG_CHIPSET", "0x120", 1);
}

bool CWinSystemSwitchGLContext::InitWindowSystem()
{
  CDVDFactoryCodec::ClearHWAccels();
  CDVDFactoryCodec::ClearHWVideoCodecs();
  VIDEOPLAYER::CRendererFactory::ClearRenderer();
  CLinuxRendererGL::Register();
  // CProcessInfoSwitch::Register();

  if (!CWinSystemSwitch::InitWindowSystem())
  {
    return false;
  }

  if (!m_pEGLContext.CreateDisplay(m_nativeDisplayType))
  {
    return false;
  }

  if (!m_pEGLContext.InitializeDisplay(EGL_OPENGL_API))
  {
    return false;
  }

  if (!m_pEGLContext.ChooseConfig(EGL_OPENGL_BIT))
  {
    return false;
  }

  if (!CreateContext())
  {
    return false;
  }

  return true;
}

bool CWinSystemSwitchGLContext::CreateNewWindow(const std::string& name,
                                               bool fullScreen,
                                               RESOLUTION_INFO& res)
{
  m_pEGLContext.DestroySurface();

  if (!CWinSystemSwitch::DestroyWindow())
  {
    return false;
  }

  if (!CWinSystemSwitch::CreateNewWindow(name, fullScreen, res))
  {
    return false;
  }

  if (!m_pEGLContext.CreateSurface(m_nativeWindowType))
  {
    return false;
  }

  if (!m_pEGLContext.BindContext())
  {
    return false;
  }

  // TODO(velocity): Does context need to be bound? Put this elsewhere if possible (so it's executed only once)
  gladLoadGL();

  if (!m_delayDispReset)
  {
    CSingleLock lock(m_resourceSection);
    // tell any shared resources
    for (std::vector<IDispResource *>::iterator i = m_resources.begin(); i != m_resources.end(); ++i)
      (*i)->OnResetDisplay();
  }

  return true;
}

bool CWinSystemSwitchGLContext::ResizeWindow(int newWidth, int newHeight, int newLeft, int newTop)
{
  CRenderSystemGL::ResetRenderSystem(newWidth, newHeight);
  return true;
}

bool CWinSystemSwitchGLContext::SetFullScreen(bool fullScreen, RESOLUTION_INFO& res, bool blankOtherDisplays)
{
  CreateNewWindow("", fullScreen, res);
  CRenderSystemGL::ResetRenderSystem(res.iWidth, res.iHeight);
  return true;
}

void CWinSystemSwitchGLContext::SetVSyncImpl(bool enable)
{
  if (!m_pEGLContext.SetVSync(enable))
  {
    CLog::Log(LOGERROR, "%s,Could not set egl vsync", __FUNCTION__);
  }
}

void CWinSystemSwitchGLContext::PresentRenderImpl(bool rendered)
{
  CGUIComponent *gui = CServiceBroker::GetGUI();
  if (gui)
    CWinSystemSwitch::SetVisible(gui->GetWindowManager().HasVisibleControls() || g_application.GetAppPlayer().IsRenderingGuiLayer());

  if (m_delayDispReset && m_dispResetTimer.IsTimePast())
  {
    m_delayDispReset = false;
    CSingleLock lock(m_resourceSection);
    // tell any shared resources
    for (std::vector<IDispResource *>::iterator i = m_resources.begin(); i != m_resources.end(); ++i)
      (*i)->OnResetDisplay();
  }
  if (!rendered)
    return;

  if (!m_pEGLContext.TrySwapBuffers())
  {
    CEGLUtils::LogError("eglSwapBuffers failed");
    throw std::runtime_error("eglSwapBuffers failed");
  }
}

bool CWinSystemSwitchGLContext::CreateContext()
{
  const EGLint glMajor = 3;
  const EGLint glMinor = 2;

  CEGLAttributesVec contextAttribs;
  contextAttribs.Add({{EGL_CONTEXT_MAJOR_VERSION_KHR, glMajor},
                      {EGL_CONTEXT_MINOR_VERSION_KHR, glMinor},
                      {EGL_CONTEXT_OPENGL_PROFILE_MASK_KHR, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT_KHR}});

  if (!m_pEGLContext.CreateContext(contextAttribs))
  {
    CLog::Log(LOGERROR, "EGL context creation failed");
    return false;
  }

  return true;
}


EGLDisplay CWinSystemSwitchGLContext::GetEGLDisplay() const
{
  return m_pEGLContext.GetEGLDisplay();
}

EGLSurface CWinSystemSwitchGLContext::GetEGLSurface() const
{
  return m_pEGLContext.GetEGLSurface();
}

EGLContext CWinSystemSwitchGLContext::GetEGLContext() const
{
  return m_pEGLContext.GetEGLContext();
}

EGLConfig  CWinSystemSwitchGLContext::GetEGLConfig() const
{
  return m_pEGLContext.GetEGLConfig();
}

std::unique_ptr<CVideoSync> CWinSystemSwitchGLContext::GetVideoSync(void *clock)
{
  // std::unique_ptr<CVideoSync> pVSync(new CVideoSyncPi(clock));
  // return pVSync;
  return {};
}

