set(SOURCES PixelConverter.cpp
            RetroPlayer.cpp
            RetroPlayerAudio.cpp
            RetroPlayerAutoSave.cpp
            RetroPlayerVideo.cpp
            VideoShaderPreset.cpp)

set(HEADERS IPixelConverter.h
            IVideoShaderPreset.h
            PixelConverter.h
            RetroPlayer.h
            RetroPlayerAudio.h
            RetroPlayerAutoSave.h
            RetroPlayerDefines.h
            RetroPlayerVideo.h
            VideoShaderPreset.h)

if(CORE_PLATFORM_NAME_LC STREQUAL rbpi)
  list(APPEND SOURCES PixelConverterRBP.cpp)
  list(APPEND HEADERS PixelConverterRBP.h)
endif()

core_add_library(retroplayer)
