#.rst:
# FindLibNX
# -------
# Finds the libnx library
#
# This will define the following variables::
#
# LIBNX_FOUND - system has libnx
# LIBNX_INCLUDE_DIRS - the libnx include directory
# LIBNX_LIBRARIES - the libnx libraries
# LIBNX_DEFINITIONS - the libnx definitions
#
# and the following imported targets::
#
#   LibNX::LibNX - The libnx library

if(PKG_CONFIG_FOUND)
  pkg_check_modules(PC_LIBNX libnx QUIET)
endif()

find_path(LIBNX_INCLUDE_DIR NAMES switch.h
                           PATHS ${PC_LIBNX_INCLUDE_DIR})
find_library(LIBNX_LIBRARY NAMES libnx.a
                          PATHS ${PC_LIBNX_LIBDIR})

set(LIBNX_VERSION ${PC_LIBNX_VERSION})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LIBNX
                                  REQUIRED_VARS LIBNX_LIBRARY LIBNX_INCLUDE_DIR
                                  VERSION_VAR LIBNX_VERSION)

if(LIBNX_FOUND)
  set(LIBNX_LIBRARIES ${LIBNX_LIBRARY})
  set(LIBNX_INCLUDE_DIRS ${LIBNX_INCLUDE_DIR})
  set(LIBNX_DEFINITIONS -DHAS_LIBNX=1)

  if(NOT TARGET LibNX::LibNX)
    add_library(LibNX::LibNX UNKNOWN IMPORTED)
    set_target_properties(LibNX::LibNX PROPERTIES
                                   IMPORTED_LOCATION "${LIBNX_LIBRARY}"
                                   INTERFACE_INCLUDE_DIRECTORIES "${LIBNX_INCLUDE_DIR}"
                                   INTERFACE_COMPILE_DEFINITIONS HAS_LIBNX=1)
  endif()
endif()

mark_as_advanced(LIBNX_INCLUDE_DIR LIBNX_LIBRARY)
