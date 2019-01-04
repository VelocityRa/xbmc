set(ARCH_DEFINES -DSWITCH -DTARGET_SWITCH -DTARGET_POSIX -DNO_DLL_SUPPORT)
#set(SYSTEM_DEFINES -D__STDC_CONSTANT_MACROS -D_FILE_DEFINED
#                   -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64)
#set(CMAKE_SYSTEM_NAME Switch)
if(WITH_ARCH)
  set(ARCH ${WITH_ARCH})
else()
    set(ARCH aarch64)
    set(NEON True)
    set(NEON_FLAGS "-fPIC -mcpu=cortex-a57")
endif()

# -------- Paths (mainly for find_package) ---------
set(PLATFORM_DIR platform/switch)
set(APP_RENDER_SYSTEM gl)

# Main cpp
set(CORE_MAIN_SOURCE ${CMAKE_SOURCE_DIR}/xbmc/platform/switch/main.cpp)

# -------- Compiler options ---------

# - Need to implicitly include string.h since it's not included in many places where it's needed
# - Need _BSD_SOURCE for a bunch of definitions in inet/in.h
set(GCC_SWITCH_CXX_COMPILE_FLAGS "-include strings.h")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_SWITCH_CXX_COMPILE_FLAGS}")

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# TODO: These don't work, changing all header inclusions to absolute paths for now
# include_directories(${CMAKE_SOURCE_DIR}/xbmc/platform/posix)
# include_directories(${CMAKE_SOURCE_DIR}/xbmc/platform/linux)

# Additional SYSTEM_DEFINES
list(APPEND SYSTEM_DEFINES -D_BSD_SOURCE -DHAS_LINUX_NETWORK)

# -------- Linker options ---------
set(GCC_SWITCH_LINK_FLAGS "")
set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${GCC_SWITCH_LINK_FLAGS}")

if((CMAKE_BUILD_TYPE STREQUAL Release OR CMAKE_BUILD_TYPE STREQUAL MinSizeRel)
    AND CMAKE_COMPILER_IS_GNUCXX)
  # Make sure we strip binaries in Release build
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s")

  # LTO Support, requires cmake >= 3.9
  if(CMAKE_VERSION VERSION_EQUAL 3.9.0 OR CMAKE_VERSION VERSION_GREATER 3.9.0)
    option(USE_LTO "Enable link time optimization. Specify an int for number of parallel jobs" OFF)
    if(USE_LTO)
      include(CheckIPOSupported)
      check_ipo_supported(RESULT HAVE_LTO OUTPUT _output)
      if(HAVE_LTO)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
        # override flags to enable parallel processing
        set(NJOBS 2)
        if(USE_LTO MATCHES "^[0-9]+$")
          set(NJOBS ${USE_LTO})
        endif()
        set(CMAKE_CXX_COMPILE_OPTIONS_IPO -flto=${NJOBS} -fno-fat-lto-objects)
        set(CMAKE_C_COMPILE_OPTIONS_IPO -flto=${NJOBS} -fno-fat-lto-objects)
      else()
        message(WARNING "LTO optimization not supported: ${_output}")
        unset(_output)
      endif()
    endif()
  endif()
endif()

if(KODI_DEPENDSBUILD)
  # Binaries should be directly runnable from host, so include rpath to depends
  set(CMAKE_INSTALL_RPATH "${DEPENDS_PATH}/lib")
  set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
endif()

set(CXX_FLAG_CXX11 TRUE)
find_package(CXX11 REQUIRED)

if(ENABLE_GBM)
  set(ENABLE_VDPAU OFF CACHE BOOL "Disabling VDPAU" FORCE)
endif()

if(ENABLE_VDPAU)
  set(ENABLE_GLX OFF CACHE BOOL "Enabling GLX" FORCE)
endif()

set(SWITCH 1)
