#.rst:
# FindSwitchGlapi
# ----------
# Finds glapi for the Nintendo Switch build
#
# This will define the following variables::
#
# SWITCHGLAPI_FOUND - system has glapi for Switch
# SWITCHGLAPI_INCLUDE_DIRS - the glapi include directory
# SWITCHGLAPI_LIBRARIES - the glapi libraries
# SWITCHGLAPI_DEFINITIONS - the glapi definitions

if(PKG_CONFIG_FOUND)
  pkg_check_modules(PC_SWITCHGLAPI glapi QUIET)
endif()

find_library(SWITCHGLAPI_glapi_LIBRARY NAMES glapi
                                PATHS ${PC_SWITCHGLAPI_gl_LIBDIR})
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SwitchGlapi
                                  REQUIRED_VARS SWITCHGLAPI_glapi_LIBRARY)

if(SWITCHGLAPI_FOUND)
  set(SWITCHGLAPI_INCLUDE_DIRS ${SWITCHGLAPI_INCLUDE_DIR})
  set(SWITCHGLAPI_LIBRARIES ${SWITCHGLAPI_glapi_LIBRARY})

  if(NOT TARGET SwitchGlapi::SwitchGlapi)
    add_library(SwitchGlapi::SwitchGlapi UNKNOWN IMPORTED)
    set_target_properties(SwitchGlapi::SwitchGlapi PROPERTIES
                            IMPORTED_LOCATION "${PC_SWITCHGLAPI_glapi_LIBDIR}"
                            INTERFACE_INCLUDE_DIRECTORIES "${SWITCHGLAPI_INCLUDE_DIRS}")
  endif()
endif()

mark_as_advanced(SWITCHGLAPI_INCLUDE_DIR SWITCHGLAPI_glapi_LIBRARY)
