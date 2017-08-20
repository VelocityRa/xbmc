/*
 *      Copyright (C) 2017 Team Kodi
 *      http://kodi.tv
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this Program; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */
#pragma once

#include "../AddonBase.h"

#include <stdint.h>

namespace kodi { namespace addon { class CInstanceShaderPreset; } }

#ifndef PATH_MAX_LENGTH
#define PATH_MAX_LENGTH 4096
#endif

#define GFX_MAX_SHADERS 26
#define GFX_MAX_TEXTURES 8
#define GFX_MAX_VARIABLES 64
#define GFX_MAX_PARAMETERS 128

extern "C"
{
  typedef struct config_file config_file;

  typedef enum rarch_shader_type
  {
    RARCH_SHADER_NONE = 0,
    RARCH_SHADER_CG,
    RARCH_SHADER_HLSL,
    RARCH_SHADER_GLSL,
    RARCH_SHADER_SLANG
  } rarch_shader_type;

  /*!
   * \brief Scale types
   *
   * If no scale type is specified, it is assumed that the scale type is
   * relative to the input with a scaling factor of 1.0.
   *
   * Exceptions: If no scale type is set for the last pass, it is assumed to
   * output at the full resolution rather than assuming of scale of 1.0, and
   * bypasses any frame-buffer object rendering.
   */
  typedef enum gfx_scale_type
  {
    /*!
     * \brief Use the source size
     *
     * Output size of the shader pass is relative to the input size. Value is
     * float.
     */
    RARCH_SCALE_INPUT = 0,

    /*!
     * \brief Use the window viewport size
     *
     * Output size of the shader pass is relative to the size of the window
     * viewpoirt. Value is float. This value can change over time if the user
     * resizes his/her window!
     */
    RARCH_SCALE_ABSOLUTE,

    /*!
     * \brief Use a statically defined size
     *
     * Output size is statically defiend to a certain size. Useful for hi-res
     * blenders or similar.
     */
    RARCH_SCALE_VIEWPORT
  } gfx_scale_type;

  typedef enum filter_type
  {
    RARCH_FILTER_UNSPEC = 0,
    RARCH_FILTER_LINEAR,
    RARCH_FILTER_NEAREST
  } filter_type;

  /*!
   * \brief Texture wrapping mode
   */
  typedef enum gfx_wrap_type
  {
    RARCH_WRAP_BORDER = 0, /* Deprecated, will be translated to EDGE in GLES */
    RARCH_WRAP_EDGE,
    RARCH_WRAP_REPEAT,
    RARCH_WRAP_MIRRORED_REPEAT
  } gfx_wrap_type;

  /*!
   * \brief FBO scaling parameters for a single axis
   */
  typedef struct gfx_fbo_scale_axis
  {
    gfx_scale_type type;
    union
    {
      float scale;
      unsigned abs;
    };
  } gfx_fbo_scale_axis;

  /*!
   * \brief FBO parameters
   */
  typedef struct gfx_fbo_scale
  {
    /*!
     * \brief sRGB framebuffer
     */
    bool srgb_fbo;

    /*!
     * \brief Float framebuffer
     *
     * This parameter defines if the pass should be rendered to a 32-bit
     * floating point buffer. The only takes effect if the pass is actaully
     * rendered to an FBO. This is useful for shaders which have to store FBO
     * values outside the range [0, 1].
     */
    bool fp_fbo;

    /*!
     * \brief Scaling parameters for X axis
     */
    gfx_fbo_scale_axis scale_x;

    /*!
     * \brief Scaling parameters for Y axis
     */
    gfx_fbo_scale_axis scale_y;
  } gfx_fbo_scale;

  typedef struct video_shader_parameter
  {
    char id[64];
    char desc[64];
    float current;
    float minimum;
    float initial;
    float maximum;
    float step;
  } video_shader_parameter;

  typedef struct video_shader_pass
  {
    /*!
     * \brief Path to the shader pass source
     */
    char source_path[PATH_MAX_LENGTH];

    /*!
     * \brief The vertex shader source
     */
    char *vertex_source;

    /*!
     * \brief The fragment shader source, if separate from the vertex source, or
     * NULL otherwise
     */
    char *fragment_source;

    /*!
     * \brief Alias
     */
    char alias[64];

    /*!
     * \brief FBO parameters
     */
    gfx_fbo_scale fbo;

    /*!
     * \brief Defines how the result of this pass will be filtered
     *
     * @todo Define behavior for unspecified filter
     */
    filter_type filter;

    /*!
     * \brief Wrapping mode
     */
    gfx_wrap_type wrap;

    /*!
     * \brief Frame count mod
     */
    unsigned frame_count_mod;

    /*!
     * \brief Mipmapping
     */
    bool mipmap;
  } video_shader_pass;

  typedef struct video_shader_lut
  {
    /*!
     * \brief Name of the sampler uniform, e.g. `uniform sampler2D foo`.
     */
    char id[64];

    /*!
     * \brief Path of the texture
     */
    char path[PATH_MAX_LENGTH];

    /*!
     * \brief Filtering for the texture
     */
    filter_type filter;

    /*!
     * \brief Texture wrapping mode
     */
    gfx_wrap_type wrap;

    /*!
     * \brief Use mipmapping for the texture
     */
    bool mipmap;
  } video_shader_lut;

  typedef struct video_shader
  {
    rarch_shader_type type;

    bool modern; /* Only used for XML shaders */
    char prefix[64];

    unsigned passes;
    video_shader_pass pass[GFX_MAX_SHADERS];

    unsigned luts;
    video_shader_lut lut[GFX_MAX_TEXTURES];

    video_shader_parameter parameters[GFX_MAX_PARAMETERS];
    unsigned num_parameters;

    /*!
     * \brief Feedback pass
     *
     * If < 0, no feedback pass is used. Otherwise, the FBO after pass #N is
     * passed a texture to next frame.
     */
    int feedback_pass;
  } video_shader;
  ///}

  typedef struct AddonProps_ShaderPreset
  {
    const char* user_path;              /*!< @brief path to the user profile */
    const char* addon_path;             /*!< @brief path to this add-on */
  } AddonProps_ShaderPreset;

  struct AddonInstance_ShaderPreset;

  typedef struct AddonToKodiFuncTable_ShaderPreset
  {
    KODI_HANDLE kodiInstance;
  } AddonToKodiFuncTable_ShaderPreset;

  typedef struct KodiToAddonFuncTable_ShaderPreset
  {
    kodi::addon::CInstanceShaderPreset* addonInstance;

    config_file* (__cdecl* config_file_new)(const AddonInstance_ShaderPreset* addonInstance, const char *path);
    void (__cdecl* config_file_free)(const AddonInstance_ShaderPreset* addonInstance, config_file *conf);

    bool (__cdecl* video_shader_read)(const AddonInstance_ShaderPreset* addonInstance, config_file *conf, video_shader *shader);
    void (__cdecl* video_shader_write)(const AddonInstance_ShaderPreset* addonInstance, config_file *conf, const video_shader *shader);
    //void (__cdecl* video_shader_resolve_relative)(const AddonInstance_ShaderPreset* addonInstance, video_shader *shader, const char *ref_path);
    //bool (__cdecl* video_shader_resolve_current_parameters)(const AddonInstance_ShaderPreset* addonInstance, video_shader *shader);
    bool (__cdecl* video_shader_resolve_parameters)(const AddonInstance_ShaderPreset* addonInstance, config_file *conf, video_shader *shader);
    void (__cdecl* video_shader_free)(const AddonInstance_ShaderPreset* addonInstance, video_shader *shader);
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

      /*!
       * \brief Loads a config file
       *
       * \param path The path to the config file
       *
       * \return The config file, or NULL if file doesn't exist
       */
      virtual config_file* ConfigFileNew(const char *path) { return nullptr; }

      /*!
       * \brief Free a config file
       */
      virtual void ConfigFileFree(config_file *conf) { }

      /*!
       * \brief Loads preset file and all associated state (passes, textures,
       * imports, etc)
       *
       * \param conf              Preset file to read from
       * \param shader            Shader passes handle
       *
       * \return True if successful, otherwise false
       **/
      virtual bool ShaderPresetRead(config_file *conf, video_shader &shader) { return false; }

      /*!
       * \brief Save preset and all associated state (passes, textures, imports,
       * etc) to disk
       *
       * \param conf              Preset file to read from
       * \param shader            Shader passes handle
       */
      virtual void ShaderPresetWrite(config_file *conf, const video_shader &shader) { }

      /*!
       * \brief Resolve relative shader path (@ref_path) into absolute shader path
       *
       * \param shader            Shader pass handle
       * \param ref_path          Relative shader path
       *
      virtual void ShaderPresetResolveRelative(video_shader &shader, const char *ref_path) { }

      /*!
       * \brief Read the current value for all parameters from config file
       *
       * \param conf              Preset file to read from
       * \param shader            Shader passes handle
       *
       * \return True if successful, otherwise false
       *
      virtual bool ShaderPresetResolveCurrentParameters(config_file *conf, video_shader &shader) { return false; }

      /*!
       * \brief Resolve all shader parameters belonging to the shader preset
       *
       * \param conf              Preset file to read from
       * \param shader            Shader passes handle
       *
       * \return True if successful, otherwise false
       */
      virtual bool ShaderPresetResolveParameters(config_file *conf, video_shader &shader) { return false; }

      /*!
       * \brief Free all state related to shader preset
       *
       * \param shader Object to free
       */
      virtual void ShaderPresetFree(video_shader* shader) { }

      std::string AddonPath() const
      {
        if (m_instanceData->props.addon_path)
          return m_instanceData->props.addon_path;
        return "";
      }

      std::string UserPath() const
      {
        if (m_instanceData->props.user_path)
          return m_instanceData->props.user_path;
        return "";
      }

    private:
      void SetAddonStruct(KODI_HANDLE instance)
      {
        if (instance == nullptr)
          throw std::logic_error("kodi::addon::CInstanceShaderPreset: Creation with empty addon structure not allowed, table must be given from Kodi!");

        m_instanceData = static_cast<AddonInstance_ShaderPreset*>(instance);
        m_instanceData->toAddon.addonInstance = this;

        m_instanceData->toAddon.config_file_new = ADDON_config_file_new;
        m_instanceData->toAddon.config_file_free = ADDON_config_file_free;

        m_instanceData->toAddon.video_shader_read = ADDON_video_shader_read_conf_cgp;
        m_instanceData->toAddon.video_shader_write = ADDON_video_shader_write_conf_cgp;
        //m_instanceData->toAddon.video_shader_resolve_relative = ADDON_video_shader_resolve_relative;
        //m_instanceData->toAddon.video_shader_resolve_current_parameters = ADDON_video_shader_resolve_current_parameters;
        m_instanceData->toAddon.video_shader_resolve_parameters = ADDON_video_shader_resolve_parameters;
        m_instanceData->toAddon.video_shader_free = ADDON_video_shader_free;
      }

      inline static config_file* ADDON_config_file_new(const AddonInstance_ShaderPreset* addonInstance, const char *path)
      {
        return addonInstance->toAddon.addonInstance->ConfigFileNew(path);
      }

      inline static void ADDON_config_file_free(const AddonInstance_ShaderPreset* addonInstance, config_file *conf)
      {
        return addonInstance->toAddon.addonInstance->ConfigFileFree(conf);
      }

      inline static bool ADDON_video_shader_read_conf_cgp(const AddonInstance_ShaderPreset* addonInstance, config_file *conf, video_shader *shader)
      {
        if (shader != nullptr)
          return addonInstance->toAddon.addonInstance->ShaderPresetRead(conf, *shader);

        return false;
      }

      inline static void ADDON_video_shader_write_conf_cgp(const AddonInstance_ShaderPreset* addonInstance, config_file *conf, const video_shader *shader)
      {
        if (shader != nullptr)
          addonInstance->toAddon.addonInstance->ShaderPresetWrite(conf, *shader);
      }

      /*
      inline static void ADDON_video_shader_resolve_relative(const AddonInstance_ShaderPreset* addonInstance, video_shader *shader, const char *ref_path)
      {
        if (shader != nullptr)
          addonInstance->toAddon.addonInstance->ShaderPresetResolveRelative(*shader, ref_path);
      }

      inline static bool ADDON_video_shader_resolve_current_parameters(const AddonInstance_ShaderPreset* addonInstance, config_file *conf, video_shader *shader)
      {
        if (shader != nullptr)
          return addonInstance->toAddon.addonInstance->ShaderPresetResolveCurrentParameters(conf, *shader);

        return false;
      }
      */

      inline static bool ADDON_video_shader_resolve_parameters(const AddonInstance_ShaderPreset* addonInstance, config_file *conf, video_shader *shader)
      {
        if (shader != nullptr)
          return addonInstance->toAddon.addonInstance->ShaderPresetResolveParameters(conf, *shader);
        
        return false;
      }

      inline static void ADDON_video_shader_free(const AddonInstance_ShaderPreset* addonInstance, video_shader *shader)
      {
        if (shader != nullptr)
          addonInstance->toAddon.addonInstance->ShaderPresetFree(shader);
      }

      AddonInstance_ShaderPreset* m_instanceData;
    };

  } /* namespace addon */
} /* namespace kodi */
