#.rst:
# FindOpenGlSwitch
# ----------
# Finds all the libraries required for OpenGL support on Nintendo Switch
#
# This will define the following variables::
#
# OPENGLSWITCH_FOUND - system has OpenGl support for Switch
# OPENGLSWITCH_INCLUDE_DIRS - the OpenGl include directory for Switch
# OPENGLSWITCH_LIBRARIES - the OpenGl libraries for Switch
# OPENGLSWITCH_DEFINITIONS - the OpenGl definitions for Switch

if(PKG_CONFIG_FOUND)
  pkg_check_modules(PC_OPENGLSWITCH gl glad glapi drm_nouveau QUIET)
endif()

find_path(OPENGLSWITCH_INCLUDE_DIR GL/gl.h
                            PATHS ${PC_OPENGLSWITCH_gl_INCLUDEDIR})
find_library(OPENGLSWITCH_glad_LIBRARY NAMES glad
                                PATHS ${PC_OPENGLSWITCH_gl_LIBDIR})
find_library(OPENGLSWITCH_glapi_LIBRARY NAMES glapi
                                PATHS ${PC_OPENGLSWITCH_glapi_LIBDIR})
find_library(OPENGLSWITCH_drm_nouveau_LIBRARY NAMES drm_nouveau
                                PATHS ${PC_OPENGLSWITCH_drm_nouveau_LIBDIR})
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OpenGlSwitch
                                  REQUIRED_VARS OPENGLSWITCH_glad_LIBRARY OPENGLSWITCH_glapi_LIBRARY OPENGLSWITCH_drm_nouveau_LIBRARY OPENGLSWITCH_INCLUDE_DIR)

if(OPENGLSWITCH_FOUND)
  set(OPENGL_FOUND TRUE)
  set(OPENGLSWITCH_INCLUDE_DIRS ${OPENGLSWITCH_INCLUDE_DIR})
  set(OPENGLSWITCH_LIBRARIES ${OPENGLSWITCH_glad_LIBRARY} ${OPENGLSWITCH_glapi_LIBRARY} ${OPENGLSWITCH_drm_nouveau_LIBRARY})
  set(OPENGLSWITCH_DEFINITIONS -DHAS_GL=1)
endif()

mark_as_advanced(OPENGLSWITCH_INCLUDE_DIR OPENGLSWITCH_glad_LIBRARY OPENGLSWITCH_glapi_LIBRARY OPENGLSWITCH_drm_nouveau_LIBRARY)
