#.rst:
# FindSwitchLibNX
# -------
# Finds the libnx library
#
# This will define the following variables::
#
# SWITCHLIBNX_FOUND - system has libnx
# SWITCHLIBNX_INCLUDE_DIRS - the libnx include directory
# SWITCHLIBNX_LIBRARIES - the libnx libraries
# SWITCHLIBNX_DEFINITIONS - the libnx definitions
#
# and the following imported targets::
#
#   SwitchLibNX::SwitchLibNX - The libnx library

if(PKG_CONFIG_FOUND)
  pkg_check_modules(PC_SWITCHLIBNX libnx QUIET)
endif()

find_path(SWITCHLIBNX_INCLUDE_DIR NAMES switch.h
                           PATHS ${PC_SWITCHLIBNX_INCLUDE_DIR})
find_library(SWITCHLIBNX_LIBRARY NAMES libnx.a
                          PATHS ${PC_SWITCHLIBNX_LIBDIR})

set(SWITCHLIBNX_VERSION ${PC_SWITCHLIBNX_VERSION})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SWITCHLIBNX
                                  REQUIRED_VARS SWITCHLIBNX_LIBRARY SWITCHLIBNX_INCLUDE_DIR
                                  VERSION_VAR SWITCHLIBNX_VERSION)

if(SWITCHLIBNX_FOUND)
  set(SWITCHLIBNX_LIBRARIES ${SWITCHLIBNX_LIBRARY})
  set(SWITCHLIBNX_INCLUDE_DIRS ${SWITCHLIBNX_INCLUDE_DIR})
  set(SWITCHLIBNX_DEFINITIONS -DHAS_SWITCHLIBNX=1)

  if(NOT TARGET SwitchLibNX::SwitchLibNX)
    add_library(SwitchLibNX::SwitchLibNX UNKNOWN IMPORTED)
    set_target_properties(SwitchLibNX::SwitchLibNX PROPERTIES
                                   IMPORTED_LOCATION "${SWITCHLIBNX_LIBRARY}"
                                   INTERFACE_INCLUDE_DIRECTORIES "${SWITCHLIBNX_INCLUDE_DIR}"
                                   INTERFACE_COMPILE_DEFINITIONS HAS_SWITCHLIBNX=1)
  endif()
endif()

mark_as_advanced(SWITCHLIBNX_INCLUDE_DIR SWITCHLIBNX_LIBRARY)
