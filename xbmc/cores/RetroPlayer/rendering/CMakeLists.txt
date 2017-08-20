set(SOURCES GUIRenderSettings.cpp
            RPRenderManager.cpp
            VideoShader.cpp
            VideoShaderLUT.cpp
            VideoShaderManager.cpp
            VideoShaderPresetFactory.cpp
)

set(HEADERS GUIRenderSettings.h
            IRenderSettingsCallback.h
            RPRenderManager.h
            VideoShader.h
            VideoShaderManager.h
            VideoShaderLUT.h
            VideoShaderPresetFactory.h
            VideoShaderUtils.h
)

core_add_library(rp-rendering)
