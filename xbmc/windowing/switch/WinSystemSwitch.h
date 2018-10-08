/*
 *  Copyright (C) 2005-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include "SwitchUtils.h"
// #include "platform/linux/input/LibInputHandler.h"
#include "platform/linux/OptionalsReg.h"
#include "rendering/gl/RenderSystemGL.h"
#include "threads/CriticalSection.h"
#include "windowing/WinSystem.h"
#include "threads/SystemClock.h"

#include "EGL/egl.h"

class IDispResource;

class CWinSystemSwitch : public CWinSystemBase
{
public:
  CWinSystemSwitch();
  virtual ~CWinSystemSwitch();

  bool InitWindowSystem() override;
  bool DestroyWindowSystem() override;

  bool CreateNewWindow(const std::string& name,
                       bool fullScreen,
                       RESOLUTION_INFO& res) override;

  bool DestroyWindow() override;
  void UpdateResolutions() override;

  bool Hide() override;
  bool Show(bool raise = true) override;
  void SetVisible(bool visible);
  virtual void Register(IDispResource *resource);
  virtual void Unregister(IDispResource *resource);
protected:
  CSwitchUtils *m_switch;
  EGLNativeDisplayType m_nativeDisplayType;
  EGLNativeWindowType m_nativeWindowType;

  int m_displayWidth;
  int m_displayHeight;

  RENDER_STEREO_MODE m_stereo_mode;

  bool m_delayDispReset;
  XbmcThreads::EndTime m_dispResetTimer;

  CCriticalSection m_resourceSection;
  std::vector<IDispResource*> m_resources;
  // std::unique_ptr<CLibInputHandler> m_libinput;
};
