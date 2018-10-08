set(SWITCH 1)
set(NO_DLL_SUPPORT 1)
# NOTE: Dependency order matters because it dictates link order
set(PLATFORM_REQUIRED_DEPS SwitchGlad EGL SwitchGlapi SwitchDrmNouveau SwitchLibNX)
set(APP_RENDER_SYSTEM gl)
