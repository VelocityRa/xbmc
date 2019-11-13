set(SOURCES VideoSyncD3D.cpp
            Win32DPMSSupport.cpp
            WinEventsWin32.cpp
            WinSystemWin32.cpp)

set(HEADERS VideoSyncD3D.h
            Win32DPMSSupport.h
            WinEventsWin32.h
            WinSystemWin32.h
            WinKeyMap.h)

if(VULKAN_FOUND)
    list(APPEND SOURCES WinSystemWin32VulkanContext.cpp)
    list(APPEND HEADERS WinSystemWin32VulkanContext.h)
else()
    list(APPEND SOURCES WinSystemWin32DX.cpp)
    list(APPEND HEADERS WinSystemWin32DX.h)
endif()

core_add_library(windowing_windows)
