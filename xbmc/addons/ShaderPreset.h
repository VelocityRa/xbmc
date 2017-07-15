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

#include "addons/binary-addons/AddonInstanceHandler.h"
#include "addons/kodi-addon-dev-kit/include/kodi/addon-instance/ShaderPreset.h"
#include "threads/CriticalSection.h"
#include "threads/SharedSection.h"
#include "filesystem/SpecialProtocol.h"
#include "binary-addons/BinaryAddonBase.h"

namespace SHADERPRESET
{
  /*
   *  Wrapper class that wraps the shader presets add-on
   */
  class CShaderPresetAddon : public ADDON::IAddonInstanceHandler
  {
  public:
    CShaderPresetAddon(const ADDON::BinaryAddonBasePtr& addonInfo);
    ~CShaderPresetAddon(void) override;

    /*!
     * @brief Initialise the instance of this add-on
     */
    bool CreateAddon(void);

    /*!
     * \brief Deinitialize the instance of this add-on
     */
    void DestroyAddon();

  /// ======== config_file_ ========

  /* Loads a config file. Returns NULL if file doesn't exist.
   * NULL path will create an empty config file. */
  config_file_t_* ConfigFileNew(const char *path);

  /* Load a config file from a string. */
  config_file_t_* ConfigFileNewFromString(const char *from_string);

  /* Frees config file. */
  void ConfigFileFree(config_file_t_ *conf);


  /// ==== video_shader__PARSE =====

  /**
   * ShaderPresetRead:
   * @conf              : Preset file to read from.
   * @shader            : Shader passes handle.
   *
   * Loads preset file and all associated state (passes,
   * textures, imports, etc).
   *
   * Returns: true (1) if successful, otherwise false (0).
   **/
  bool ShaderPresetRead(config_file_t_* conf,
                        video_shader_* shader);

  /**
   * ShaderPresetWrite:
   * @conf              : Preset file to read from.
   * @shader            : Shader passes handle.
   *
   * Saves preset and all associated state (passes,
   * textures, imports, etc) to disk.
   **/
  void ShaderPresetWrite(config_file_t_ *conf,
    struct video_shader_ *shader);

  /**
   * ShaderPresetResolveRelative:
   * @shader            : Shader pass handle.
   * @ref_path          : Relative shader path.
   *
   * Resolves relative shader path (@ref_path) into absolute
   * shader paths.
   **/
  void ShaderPresetResolveRelative(struct video_shader_ *shader,
    const char *ref_path);

  /**
   * ShaderPresetResolveCurrentParameters:
   * @conf              : Preset file to read from.
   * @shader            : Shader passes handle.
   *
   * Reads the current value for all parameters from config file.
   *
   * Returns: true (1) if successful, otherwise false (0).
   **/
  bool ShaderPresetResolveCurrentParameters(config_file_t_ *conf,
    struct video_shader_ *shader);

  /**
   * ShaderPresetResolveParameters:
   * @conf              : Preset file to read from.
   * @shader            : Shader passes handle.
   *
   * Resolves all shader parameters belonging to shaders.
   *
   * Returns: true (1) if successful, otherwise false (0).
   **/
  bool ShaderPresetResolveParameters(config_file_t_ *conf,
    struct video_shader_ *shader);

  /**
  * GetLibraryBasePath:
  * @brief Returns the full/absolute path of the dynamic library file.
  *
  **/
  const char* GetLibraryBasePath(void);

  private:
    /*!
     * @brief Reset all class members to their defaults. Called by the constructors
     */
    void ResetProperties(void);

    /* @brief Cache for const char* members in PERIPHERAL_PROPERTIES */

    std::string         m_strLibraryPath;
    std::string         m_strConfigPath;
    std::string         m_strConfigBasePath;
    std::string         m_strUserPath;    /*!< @brief translated path to the user profile */
    std::string         m_strClientPath;  /*!< @brief translated path to this add-on */

    /* @brief Add-on properties */
    bool                m_bTestBoolProp;

    /* @brief Thread synchronization */
    CCriticalSection    m_critSection;

    AddonInstance_ShaderPreset m_struct;

    CSharedSection      m_dllSection;
  };
} // namespace SHADERPRESET
