if(CORE_SYSTEM_NAME STREQUAL windows)
  set(SOURCES VideoShaderDX.cpp
              VideoShaderPresetDX.cpp
              VideoShaderTextureDX.cpp)

  set(HEADERS VideoShaderDX.h
              VideoShaderTextureDX.h
              VideoShaderPresetDX.h
              VideoShaderUtilsDX.h)

core_add_library(rp-videoshader-windows)
endif()


