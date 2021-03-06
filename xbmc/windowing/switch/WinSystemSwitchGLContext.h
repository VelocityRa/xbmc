/*
 *  Copyright (C) 2005-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include "utils/EGLUtils.h"
#include "rendering/gl/RenderSystemGL.h"
#include "WinSystemSwitch.h"

class CWinSystemSwitchGLContext : public CWinSystemSwitch, public CRenderSystemGL
{
public:
  CWinSystemSwitchGLContext() = default;
  virtual ~CWinSystemSwitchGLContext() = default;

  // Implementation of CWinSystemBase via CWinSystemSwitch
  CRenderSystemBase *GetRenderSystem() override { return this; }
  bool InitWindowSystem() override;
  bool CreateNewWindow(const std::string& name,
                       bool fullScreen,
                       RESOLUTION_INFO& res) override;

  bool ResizeWindow(int newWidth, int newHeight, int newLeft, int newTop) override;
  bool SetFullScreen(bool fullScreen, RESOLUTION_INFO& res, bool blankOtherDisplays) override;

  virtual std::unique_ptr<CVideoSync> GetVideoSync(void *clock) override;

  EGLDisplay GetEGLDisplay() const;
  EGLSurface GetEGLSurface() const;
  EGLContext GetEGLContext() const;
  EGLConfig  GetEGLConfig() const;
protected:
  bool CreateContext();
  void SetVSyncImpl(bool enable) override;
  void PresentRenderImpl(bool rendered) override;

  static void SetMesaConfig();

private:
  CEGLContextUtils m_pEGLContext;
};
