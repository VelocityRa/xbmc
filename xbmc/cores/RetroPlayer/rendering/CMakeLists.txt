set(SOURCES RPRenderManager.cpp
<<<<<<< HEAD
)

set(HEADERS GUIRenderSettings.h
            IRenderSettingsCallback.h
            RPRenderManager.h
)
=======
            VideoShaderManager.cpp
            VideoShader.cpp)

set(HEADERS RPRenderManager.h
            VideoShaderManager.h
            VideoShader.h)
>>>>>>> RetroPlayer: Shader Renderer

core_add_library(rp-rendering)
