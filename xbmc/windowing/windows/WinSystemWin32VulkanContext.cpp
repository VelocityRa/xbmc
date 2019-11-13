/*
 *  Copyright (C) 2017-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "WinSystemWin32VulkanContext.h"

#include "utils/log.h"

using namespace KODI::WINDOWING::WINDOWS;

std::unique_ptr<CWinSystemBase> CWinSystemBase::CreateWinSystem()
{
  std::unique_ptr<CWinSystemBase> winSystem(new CWinSystemWin32VulkanContext());
  return winSystem;
}

bool CWinSystemWin32VulkanContext::InitWindowSystem()
{
  if (!CWinSystemWin32::InitWindowSystem())
  {
    return false;
  }

  if (!InitRenderSystem())
  {
    return false;
  }

  return true;
}

bool CWinSystemWin32VulkanContext::DestroyWindowSystem()
{
  return CWinSystemWin32::DestroyWindowSystem();
}

bool CWinSystemWin32VulkanContext::CreateNewWindow(const std::string& name,
                                                     bool fullScreen,
                                                     RESOLUTION_INFO& res)
{
  if (!CWinSystemWin32::CreateNewWindow(name, fullScreen, res))
  {
    return false;
  }

  if (!CreateSurface(GetConnection()->GetDisplay(), GetMainSurface()))
  {
    return false;
  }

  return true;
}

bool CWinSystemWin32VulkanContext::DestroyWindow()
{
  DestroySurface();

  return CWinSystemWin32::DestroyWindow();
}

void CWinSystemWin32VulkanContext::PresentRender(bool rendered, bool videoLayer)
{
  PrepareFramePresentation();

  if (rendered)
  {
    // TODO
  }
  else
  {
    // For presentation feedback: Get notification of the next vblank even
    // when contents did not change
    GetMainSurface().commit();
    // Make sure it reaches the compositor
    GetConnection()->GetDisplay().flush();
  }

  FinishFramePresentation();
}

// void CWinSystemWin32VulkanContext::SetContextSize(CSizeInt size)
// {
//   // Change EGL surface size if necessary
//   if (GetNativeWindowAttachedSize() != size)
//   {
//     CLog::LogF(LOGDEBUG, "Updating window size to %dx%d", size.Width(), size.Height());
//     // m_nativeWindow.resize(size.Width(), size.Height(), 0, 0);
//   }

//   // Propagate changed dimensions to render system if necessary
//   if (CRenderSystemVulkan::m_width != size.Width() ||
//       CRenderSystemVulkan::m_height != size.Height())
//   {
//     CLog::LogF(LOGDEBUG, "Resetting render system to %dx%d", size.Width(), size.Height());
//     KODI::RENDERING::VULKAN::CRenderSystemVulkan::ResetRenderSystem(size.Width(), size.Height());
//   }
// }

// CSizeInt CWinSystemWin32VulkanContext::GetNativeWindowAttachedSize()
// {
//   int width, height;
//   // m_nativeWindow.get_attached_size(width, height);
//   return {width, height};
// }
