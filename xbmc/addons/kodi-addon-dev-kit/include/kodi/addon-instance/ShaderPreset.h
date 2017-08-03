#pragma once
/*  Copyright (C) 2017 Nick Renieris
 *
 *  RetroArch - A frontend for libretro.
 *  Copyright (C) 2010-2014 - Hans-Kristian Arntzen
 *  Copyright (C) 2011-2017 - Daniel De Matteis
 *
 *  RetroArch is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  RetroArch is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with RetroArch.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

#include "../AddonBase.h"

#include <stdbool.h>

namespace kodi { namespace addon { class CInstanceShaderPreset; }}

extern "C"
{
  /// ======== config_file_ ========

  struct config_entry_list_
  {
     /* If we got this from an #include,
      * do not allow overwrite. */
     bool readonly;
     char *key;
     char *value;
     uint32_t key_hash;

     struct config_entry_list_ *next;
  };

  struct config_include_list_
  {
     char *path;
     struct config_include_list_ *next;
  };

  struct config_file_
  {
     char *path;
     struct config_entry_list_ *entries;
     struct config_entry_list_ *tail;
     unsigned include_depth;

     struct config_include_list_ *includes;
  };

  typedef struct config_file_ config_file_t_;

  /// =============================


  /// ==== video_shader_PARSE =====

  #ifndef PATH_MAX_LENGTH
  #if defined(_XBOX1) || defined(_3DS) || defined(PSP) || defined(GEKKO)|| defined(WIIU)
  #define PATH_MAX_LENGTH 512
  #else
  #define PATH_MAX_LENGTH 4096
  #endif
  #endif

  #ifndef GFX_MAX_SHADERS
  #define GFX_MAX_SHADERS 26
  #endif

  #ifndef GFX_MAX_TEXTURES
  #define GFX_MAX_TEXTURES 8
  #endif

  #ifndef GFX_MAX_VARIABLES
  #define GFX_MAX_VARIABLES 64
  #endif

  #ifndef GFX_MAX_PARAMETERS
  #define GFX_MAX_PARAMETERS 128
  #endif

  struct state_tracker_uniform_info_
  {
    char id[64];
    uint32_t addr;
    enum state_tracker_type_ type;
    enum state_ram_type_ ram_type;
    uint16_t mask;
    uint16_t equal;
  };

  enum rarch_shader_type_
  {
     RARCH_SHADER_NONE_ = 0,
     RARCH_SHADER_CG_,
     RARCH_SHADER_HLSL_,
     RARCH_SHADER_GLSL_,
     RARCH_SHADER_SLANG_
  };

  enum gfx_scale_type_
  {
     RARCH_SCALE_INPUT_ = 0,
     RARCH_SCALE_ABSOLUTE_,
     RARCH_SCALE_VIEWPORT_
  };

  enum
  {
     RARCH_FILTER_UNSPEC_ = 0,
     RARCH_FILTER_LINEAR_,
     RARCH_FILTER_NEAREST_
  };

  enum gfx_wrap_type_
  {
     RARCH_WRAP_BORDER_ = 0, /* Kinda deprecated, but keep as default.
                               Will be translated to EDGE in GLES. */
     RARCH_WRAP_DEFAULT_ = RARCH_WRAP_BORDER_,
     RARCH_WRAP_EDGE_,
     RARCH_WRAP_REPEAT_,
     RARCH_WRAP_MIRRORED_REPEAT_
  };

  struct gfx_fbo_scale_
  {
     enum gfx_scale_type_ type_x;
     enum gfx_scale_type_ type_y;
     float scale_x;
     float scale_y;
     unsigned abs_x;
     unsigned abs_y;
     bool fp_fbo;
     bool srgb_fbo;
     bool valid;
  };

  struct video_shader_parameter_
  {
     char id[64];
     char desc[64];
     float current;
     float minimum;
     float initial;
     float maximum;
     float step;
  };

  struct video_shader_pass_
  {
     struct
     {
        char path[PATH_MAX_LENGTH];
        struct
        {
           char *vertex; /* Dynamically allocated. Must be free'd. */
           char *fragment; /* Dynamically allocated. Must be free'd. */
        } string;
     } source;

     char alias[64];
     struct gfx_fbo_scale_ fbo;
     unsigned filter;
     enum gfx_wrap_type_ wrap;
     unsigned frame_count_mod;
     bool mipmap;
  };

  struct video_shader_lut_
  {
     char id[64];
     char path[PATH_MAX_LENGTH];
     unsigned filter;
     enum gfx_wrap_type_ wrap;
     bool mipmap;
  };

  /* This is pretty big, shouldn't be put on the stack.
   * Avoid lots of allocation for convenience. */
  struct video_shader_
  {
    enum rarch_shader_type_ type;

    bool modern; /* Only used for XML shaders. */
    char prefix[64];

    unsigned passes;
    struct video_shader_pass_ pass[GFX_MAX_SHADERS];

    unsigned luts;
    struct video_shader_lut_ lut[GFX_MAX_TEXTURES];

    struct video_shader_parameter_ parameters[GFX_MAX_PARAMETERS];
    unsigned num_parameters;

    unsigned variables;
    struct state_tracker_uniform_info_ variable[GFX_MAX_VARIABLES];
    char script_path[PATH_MAX_LENGTH];
    char *script; /* Dynamically allocated. Must be free'd. Only used by XML. */
    char script_class[512];

    /* If < 0, no feedback pass is used. Otherwise,
    * the FBO after pass #N is passed a texture to next frame. */
    int feedback_pass;
  };

  typedef struct AddonProps_ShaderPreset
  {
    const char* user_path;              /*!< @brief path to the user profile */
    const char* addon_path;             /*!< @brief path to this add-on */
  } ATTRIBUTE_PACKED AddonProps_ShaderPreset;

  struct AddonInstance_ShaderPreset;

  typedef struct AddonToKodiFuncTable_ShaderPreset
  {
    KODI_HANDLE kodiInstance;
  } AddonToKodiFuncTable_ShaderPreset;

  typedef struct KodiToAddonFuncTable_ShaderPreset
  {
    kodi::addon::CInstanceShaderPreset* addonInstance;

    /// ======== config_file_ ========

    /* Loads a config file. Returns NULL if file doesn't exist.
     * NULL path will create an empty config file. */
    config_file_t_* (*config_file_new)(const AddonInstance_ShaderPreset* addonInstance,
      const char *path, const char *basePath);

    /* Load a config file from a string. */
    config_file_t_* (*config_file_new_from_string)(const AddonInstance_ShaderPreset* addonInstance,
      const char *from_string);

    /* Frees config file. */
    void (*config_file_free)(const AddonInstance_ShaderPreset* addonInstance,
      config_file_t_ *conf);

    /// =============================


    /// ==== video_shader_PARSE =====

    bool (*video_shader_read_conf_cgp)(const AddonInstance_ShaderPreset* addonInstance,
      config_file_t_ *conf, struct video_shader_ *shader);

    void (*video_shader_write_conf_cgp)(const AddonInstance_ShaderPreset* addonInstance,
      config_file_t_ *conf, struct video_shader_ *shader);

    void (*video_shader_resolve_relative)(const AddonInstance_ShaderPreset* addonInstance,
      struct video_shader_ *shader, const char *ref_path);

    bool (*video_shader_resolve_current_parameters)(const AddonInstance_ShaderPreset* addonInstance,
      config_file_t_ *conf, struct video_shader_ *shader);

    bool (*video_shader_resolve_parameters)(const AddonInstance_ShaderPreset* addonInstance,
      config_file_t_ *conf, struct video_shader_ *shader);

    enum rarch_shader_type_ (*video_shader_parse_type)(const AddonInstance_ShaderPreset* addonInstance,
      const char *path, enum rarch_shader_type_ fallback);

    /// =============================

  } KodiToAddonFuncTable_ShaderPreset;

  typedef struct AddonInstance_ShaderPreset
  {
    AddonProps_ShaderPreset props;
    AddonToKodiFuncTable_ShaderPreset toKodi;
    KodiToAddonFuncTable_ShaderPreset toAddon;
  } AddonInstance_ShaderPreset;

} /* extern "C" */

namespace kodi
{
namespace addon
{

  class CInstanceShaderPreset : public IAddonInstance
  {
  public:
    CInstanceShaderPreset()
      : IAddonInstance(ADDON_INSTANCE_SHADERPRESET)
    {
      if (CAddonBase::m_interface->globalSingleInstance != nullptr)
        throw std::logic_error("kodi::addon::CInstanceShaderPreset: Creation of more as one in single instance way is not allowed!");

      SetAddonStruct(CAddonBase::m_interface->firstKodiInstance);
      CAddonBase::m_interface->globalSingleInstance = this;
    }

    CInstanceShaderPreset(KODI_HANDLE instance)
      : IAddonInstance(ADDON_INSTANCE_SHADERPRESET)
    {
      if (CAddonBase::m_interface->globalSingleInstance != nullptr)
        throw std::logic_error("kodi::addon::CInstanceShaderPreset: Creation of multiple together with single instance way is not allowed!");

      SetAddonStruct(instance);
    }

    ~CInstanceShaderPreset() override { }

  /// ======== config_file_ ========

  /* Loads a config file. Returns NULL if file doesn't exist.
   * NULL path will create an empty config file. */
  virtual config_file_t_* ConfigFileNew(const char *path, const char *basePath) { return reinterpret_cast<config_file_t_*>(0x1234); }

  /* Load a config file from a string. */
  virtual config_file_t_* ConfigFileNewFromString(const char *from_string) { return nullptr; }

  /* Frees config file. */
  virtual void ConfigFileFree(config_file_t_ *conf) { }


  /// ==== video_shader_PARSE =====

  /**
   * ShaderPresetRead:
   * @conf              : Preset file to read from.
   * @shader            : Shader passes handle.
   * @addonPath         : Add-on's path, so that shader source is loaded.
   *                      If nullptr or empty, shader source isn't loaded.
   *
   * Loads preset file and all associated state (passes,
   * textures, imports, etc).
   *
   * Returns: true (1) if successful, otherwise false (0).
   **/
  virtual bool ShaderPresetRead(config_file_t_ *conf,
    struct video_shader_ *shader) { return false; }

  /**
   * ShaderPresetWrite:
   * @conf              : Preset file to read from.
   * @shader            : Shader passes handle.
   *
   * Saves preset and all associated state (passes,
   * textures, imports, etc) to disk.
   **/
  virtual void ShaderPresetWrite(config_file_t_ *conf,
    struct video_shader_ *shader) { }

  /**
   * ShaderPresetResolveRelative:
   * @shader            : Shader pass handle.
   * @ref_path          : Relative shader path.
   *
   * Resolves relative shader path (@ref_path) into absolute
   * shader paths.
   **/
  virtual void ShaderPresetResolveRelative(struct video_shader_ *shader,
    const char *ref_path) { }

  /**
   * ShaderPresetResolveCurrentParameters:
   * @conf              : Preset file to read from.
   * @shader            : Shader passes handle.
   *
   * Reads the current value for all parameters from config file.
   *
   * Returns: true (1) if successful, otherwise false (0).
   **/
  virtual bool ShaderPresetResolveCurrentParameters(config_file_t_ *conf,
    struct video_shader_ *shader) { return false; }

  /**
   * ShaderPresetResolveParameters:
   * @conf              : Preset file to read from.
   * @shader            : Shader passes handle.
   *
   * Resolves all shader parameters belonging to shaders.
   *
   * Returns: true (1) if successful, otherwise false (0).
   **/
  virtual bool ShaderPresetResolveParameters(config_file_t_ *conf,
    struct video_shader_ *shader) { return false; }

  const std::string AddonPath() const
  {
    return m_instanceData->props.addon_path;
  }

  const std::string UserPath() const
  {
    return m_instanceData->props.user_path;
  }

  private:
    void SetAddonStruct(KODI_HANDLE instance)
    {
      if (instance == nullptr)
        throw std::logic_error("kodi::addon::CInstanceShaderPreset: Creation with empty addon structure not allowed, table must be given from Kodi!");

      m_instanceData = static_cast<AddonInstance_ShaderPreset*>(instance);
      m_instanceData->toAddon.addonInstance = this;

      m_instanceData->toAddon.config_file_new = ADDON_config_file_new;
      m_instanceData->toAddon.config_file_new_from_string = ADDON_config_file_new_from_string;
      m_instanceData->toAddon.config_file_free = ADDON_config_file_free;

      m_instanceData->toAddon.video_shader_read_conf_cgp = ADDON_video_shader_read_conf_cgp;
      m_instanceData->toAddon.video_shader_write_conf_cgp = ADDON_video_shader_write_conf_cgp;
      m_instanceData->toAddon.video_shader_resolve_relative = ADDON_video_shader_resolve_relative;
      m_instanceData->toAddon.video_shader_resolve_current_parameters = ADDON_video_shader_resolve_current_parameters;
      m_instanceData->toAddon.video_shader_resolve_parameters = ADDON_video_shader_resolve_parameters;
    }

    inline static config_file_t_* ADDON_config_file_new(const AddonInstance_ShaderPreset* addonInstance,
      const char *path, const char *basePath)
    {
      return addonInstance->toAddon.addonInstance->ConfigFileNew(path, basePath);
    }
    inline static config_file_t_* ADDON_config_file_new_from_string(const AddonInstance_ShaderPreset* addonInstance,
      const char *from_string)
    {
      return addonInstance->toAddon.addonInstance->ConfigFileNewFromString(from_string);
    }
    inline static void ADDON_config_file_free(const AddonInstance_ShaderPreset* addonInstance,
      config_file_t_ *conf)
    {
      return addonInstance->toAddon.addonInstance->ConfigFileFree(conf);
    }

    inline static bool ADDON_video_shader_read_conf_cgp(const AddonInstance_ShaderPreset* addonInstance,
      config_file_t_ *conf, struct video_shader_ *shader)
    {
      return addonInstance->toAddon.addonInstance->ShaderPresetRead(conf, shader);
    }
    inline static void ADDON_video_shader_write_conf_cgp(const AddonInstance_ShaderPreset* addonInstance,
      config_file_t_ *conf, struct video_shader_ *shader)
    {
      return addonInstance->toAddon.addonInstance->ShaderPresetWrite(conf, shader);
    }
    inline static void ADDON_video_shader_resolve_relative(const AddonInstance_ShaderPreset* addonInstance,
      struct video_shader_ *shader, const char *ref_path)
    {
      return addonInstance->toAddon.addonInstance->ShaderPresetResolveRelative(shader, ref_path);
    }
    inline static bool ADDON_video_shader_resolve_current_parameters(const AddonInstance_ShaderPreset* addonInstance,
      config_file_t_ *conf, struct video_shader_ *shader)
    {
      return addonInstance->toAddon.addonInstance->ShaderPresetResolveCurrentParameters(conf, shader);
    }
    inline static bool ADDON_video_shader_resolve_parameters(const AddonInstance_ShaderPreset* addonInstance,
      config_file_t_ *conf, struct video_shader_ *shader)
    {
      return addonInstance->toAddon.addonInstance->ShaderPresetResolveParameters(conf, shader);
    }

    AddonInstance_ShaderPreset* m_instanceData;
  };

} /* namespace addon */
} /* namespace kodi */
