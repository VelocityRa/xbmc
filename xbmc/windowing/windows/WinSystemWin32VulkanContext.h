/*
 *  Copyright (C) 2017-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include "WinSystemWin32.h"
#include "rendering/vulkan/windows/RenderSystemVulkanWindows.h"

namespace KODI
{
namespace WINDOWING
{
namespace WINDOWS
{

class CWinSystemWin32VulkanContext : public CWinSystemWin32,
                                       public KODI::RENDERING::VULKAN::CRenderSystemVulkanWindows
{
public:
  // Implementation of CWinSystemBase via CWinSystemWin32
  CRenderSystemBase* GetRenderSystem() override { return this; }
  bool InitWindowSystem() override;
  bool DestroyWindowSystem() override;

  bool CreateNewWindow(const std::string& name, bool fullScreen, RESOLUTION_INFO& res) override;
  bool DestroyWindow() override;

  void PresentRender(bool rendered, bool videoLayer) override;

protected:

private:
  // CSizeInt GetNativeWindowAttachedSize();
};

} // namespace WINDOWS
} // namespace WINDOWING
} // namespace KODI
