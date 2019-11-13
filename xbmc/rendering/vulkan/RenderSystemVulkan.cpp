/*
 *  Copyright (C) 2005-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "RenderSystemVulkan.h"

#include "CompileInfo.h"
#include "utils/log.h"

// TODO
#ifdef WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan_win32.h>
#else
#define VK_USE_PLATFORM_WAYLAND_KHR
#include <vulkan/vulkan_wayland.h>
#include <wayland-client.hpp>
#endif

using namespace KODI::RENDERING::VULKAN;

namespace
{
std::array<const char*, 1> layerNames = {
    "VK_LAYER_LUNARG_standard_validation",
};
} // namespace

CRenderSystemVulkan::CRenderSystemVulkan()
{
  m_instanceExtensions[2] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
}

bool CRenderSystemVulkan::InitRenderSystem()
{
  if (m_initialized)
    return true;

  try
  {
    vk::ApplicationInfo appInfo(CCompileInfo::GetAppName(), 0, nullptr, 0, VK_API_VERSION_1_0);

    vk::InstanceCreateInfo instanceCreateInfo({}, &appInfo, layerNames.size(), layerNames.data(),
                                              m_instanceExtensions.size(),
                                              m_instanceExtensions.data());

    m_instance = vk::createInstanceUnique(instanceCreateInfo);

    m_debug.SetupDebugging(m_instance);

    m_physicalDevices = m_instance->enumeratePhysicalDevices();
    m_physicalDevice = m_physicalDevices.front();

    m_vulkanDevice.CreateLogicalDevice(&m_physicalDevice);

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

  m_initialized = true;

  return true;
}

bool CRenderSystemVulkan::DestroyRenderSystem()
{
  try
  {
    // m_debug.DestroyDebugging(m_instance);
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
