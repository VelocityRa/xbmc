/*
 *  Copyright (C) 2005-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "RenderSystemVulkanWindows.h"

#include "utils/log.h"

#include <vulkan/vulkan_win32.h>

using namespace KODI::RENDERING::VULKAN;

CRenderSystemVulkanWindows::CRenderSystemVulkanWindows() : CRenderSystemVulkan()
{
  m_instanceExtensions[0] = VK_KHR_SURFACE_EXTENSION_NAME;
  m_instanceExtensions[1] = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
}

bool CRenderSystemVulkanWindows::CreateSurface(HINSTANCE* hinstance, HWND* hwnd)
{
  try
  {
    m_vulkanSurface = m_instance->createWin32SurfaceKHRUnique(
        vk::Win32SurfaceCreateInfoKHR({}, hInstance, hwnd)); //, nullptr, &m_dynamicLoader);
  }
  catch (vk::SystemError& err)
  {
    CLog::LogF(LOGERROR, "vk::SystemError: {}", err.what());
    return false;
  }
  catch (std::runtime_error& err)
  {
    CLog::LogF(LOGERROR, "std::runtime_error: {}", err.what());
    return false;
  }
  catch (...)
  {
    CLog::LogF(LOGERROR, "unknown error");
    return false;
  }

  return true;
}

void CRenderSystemVulkanWindows::DestroySurface()
{
  try
  {
    m_vulkanSurface.reset();
  }
  catch (vk::SystemError& err)
  {
    CLog::LogF(LOGERROR, "vk::SystemError: {}", err.what());
  }
  catch (std::runtime_error& err)
  {
    CLog::LogF(LOGERROR, "std::runtime_error: {}", err.what());
  }
  catch (...)
  {
    CLog::LogF(LOGERROR, "unknown error");
  }
}
