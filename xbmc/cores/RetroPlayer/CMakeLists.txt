set(SOURCES RetroPlayer.cpp
            RetroPlayerAudio.cpp
            RetroPlayerAutoSave.cpp
            RetroPlayerInput.cpp
            RetroPlayerVideo.cpp
            VideoShaderPreset.cpp
)

set(HEADERS IVideoShaderPreset.h
            RetroPlayer.h
            RetroPlayerAudio.h
            RetroPlayerAutoSave.h
            RetroPlayerDefines.h
            RetroPlayerInput.h
            RetroPlayerVideo.h
            VideoShaderPreset.h
)

core_add_library(retroplayer)
