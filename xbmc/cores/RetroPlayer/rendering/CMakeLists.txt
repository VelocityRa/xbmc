set(SOURCES RPRenderManager.cpp
            VideoShaderManager.cpp
            VideoShader.cpp
            VideoShaderLUT.cpp
            VideoShaderManager.cpp)

set(HEADERS GUIRenderSettings.h
            IRenderSettingsCallback.h
            RPRenderManager.h
            VideoShader.h
            VideoShaderLUT.h
            VideoShaderManager.h
            VideoShaderUtils.h)

core_add_library(rp-rendering)
