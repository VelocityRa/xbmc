#.rst:
# FindSwitchDrmNouveau
# ----------
# Finds drm_nouveau for the Nintendo Switch build
#
# This will define the following variables::
#
# SWITCHDRMNOUVEAU_FOUND - system has drm_nouveau for Switch
# SWITCHDRMNOUVEAU_INCLUDE_DIRS - the drm_nouveau include directory
# SWITCHDRMNOUVEAU_LIBRARIES - the drm_nouveau libraries
# SWITCHDRMNOUVEAU_DEFINITIONS - the drm_nouveau definitions

if(PKG_CONFIG_FOUND)
  pkg_check_modules(PC_SWITCHDRMNOUVEAU drm_nouveau QUIET)
endif()

find_path(SWITCHDRMNOUVEAU_INCLUDE_DIR nouveau.h
                            PATHS ${PC_SWITCHDRMNOUVEAU_drm_nouveau_INCLUDEDIR})
find_library(SWITCHDRMNOUVEAU_drm_nouveau_LIBRARY NAMES drm_nouveau
                                PATHS ${PC_SWITCHDRMNOUVEAU_drm_nouveau_LIBDIR})
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SwitchDrmNouveau
                                  REQUIRED_VARS SWITCHDRMNOUVEAU_drm_nouveau_LIBRARY SWITCHDRMNOUVEAU_INCLUDE_DIR)

if(SWITCHDRMNOUVEAU_FOUND)
  set(SWITCHDRMNOUVEAU_INCLUDE_DIRS ${SWITCHDRMNOUVEAU_INCLUDE_DIR})
  set(SWITCHDRMNOUVEAU_LIBRARIES ${SWITCHDRMNOUVEAU_drm_nouveau_LIBRARY})

  if(NOT TARGET SwitchDrmNouveau::SwitchDrmNouveau)
    add_library(SwitchDrmNouveau::SwitchDrmNouveau UNKNOWN IMPORTED)
    set_target_properties(SwitchDrmNouveau::SwitchDrmNouveau PROPERTIES
                            IMPORTED_LOCATION "${PC_SWITCHDRMNOUVEAU_drm_nouveau_LIBDIR}"
                            INTERFACE_INCLUDE_DIRECTORIES "${SWITCHDRMNOUVEAU_INCLUDE_DIRS}")
  endif()
endif()

mark_as_advanced(SWITCHDRMNOUVEAU_INCLUDE_DIR SWITCHGLAPI_drm_nouveau_LIBRARY)
