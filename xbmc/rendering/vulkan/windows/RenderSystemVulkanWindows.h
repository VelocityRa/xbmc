/*
 *  Copyright (C) 2005-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#define VK_USE_PLATFORM_WINDOWS_KHR
#include "rendering/vulkan/RenderSystemVulkan.h"

struct HINSTANCE;
struct HWND;

namespace KODI
{
namespace RENDERING
{
namespace VULKAN
{

class CRenderSystemVulkanWindows : public CRenderSystemVulkan
{
public:
  CRenderSystemVulkanWindows();
  ~CRenderSystemVulkanWindows() override = default;

protected:
  bool CreateSurface(HINSTANCE* hInstance, HWND* hwnd);
  void DestroySurface();

private:
  vk::UniqueSurfaceKHR m_vulkanSurface;
};

} // namespace VULKAN
} // namespace RENDERING
} // namespace KODI
