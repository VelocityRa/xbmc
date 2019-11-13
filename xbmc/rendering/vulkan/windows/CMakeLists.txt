if (VULKAN_FOUND)
  set(SOURCES RenderSystemVulkanWindows.cpp)
  set(HEADERS RenderSystemVulkanWindows.h)
endif()

core_add_library(rendering_vulkan_windows)
