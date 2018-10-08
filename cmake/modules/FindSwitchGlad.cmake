#.rst:
# FindSwitchGlad
# ----------
# Finds glad for the Nintendo Switch build
#
# This will define the following variables::
#
# SWITCHGLAD_FOUND - system has glad for Switch
# SWITCHGLAD_INCLUDE_DIRS - the glad include directory
# SWITCHGLAD_LIBRARIES - the glad libraries
# SWITCHGLAD_DEFINITIONS - the glad definitions

if(PKG_CONFIG_FOUND)
  pkg_check_modules(PC_SWITCHGLAD gl glad glapi drm_nouveau QUIET)
endif()

find_path(SWITCHGLAD_INCLUDE_DIR glad/glad.h
                            PATHS ${PC_SWITCHGLAD_glad_INCLUDEDIR})
find_library(SWITCHGLAD_glad_LIBRARY NAMES glad
                                PATHS ${PC_SWITCHGLAD_glad_LIBDIR})
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SwitchGlad
                                  REQUIRED_VARS SWITCHGLAD_glad_LIBRARY SWITCHGLAD_INCLUDE_DIR)

if(SWITCHGLAD_FOUND)
  set(OPENGL_FOUND TRUE PARENT_SCOPE)
  set(SWITCHGLAD_INCLUDE_DIRS ${SWITCHGLAD_INCLUDE_DIR})
  set(SWITCHGLAD_LIBRARIES ${SWITCHGLAD_glad_LIBRARY})
  set(SWITCHGLAD_DEFINITIONS -DHAS_GL=1)

  if(NOT TARGET SwitchGlad::SwitchGlad)
    add_library(SwitchGlad::SwitchGlad UNKNOWN IMPORTED)
    set_target_properties(SwitchGlad::SwitchGlad PROPERTIES
                            IMPORTED_LOCATION "${SWITCHGLAD_glad_LIBRARY}"
                            INTERFACE_INCLUDE_DIRECTORIES "${SWITCHGLAD_INCLUDE_DIRS}")
  endif()
endif()

mark_as_advanced(SWITCHGLAD_INCLUDE_DIR SWITCHGLAD_glad_LIBRARY)
