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
#include "IVideoShaderPreset.h"
#include "addons/ShaderPreset.h"

#include <memory>

namespace SHADERPRESET
{
  /*
   * C++ (OOP) Wrapper class to RetroArch's video_shader struct
   *
   * Uses CShaderPresetAddon for parsing shader preset files
   */

  class CVideoShaderPreset : public IVideoShaderPreset
  {
  //private:
  public:
    // todo: probably don't need that
    enum rarch_shader_type_ type;

    unsigned m_Passes;
    struct video_shader_pass_ m_Pass[GFX_MAX_SHADERS];

    unsigned m_Luts;
    struct video_shader_lut_ m_Lut[GFX_MAX_TEXTURES];

    struct video_shader_parameter_ m_Parameters[GFX_MAX_PARAMETERS];
    unsigned m_NumParameters;

    unsigned m_Variables;
    struct state_tracker_uniform_info_ m_Variable[GFX_MAX_VARIABLES];

    /* If < 0, no feedback pass is used. Otherwise,
    * the FBO after pass #N is passed a texture to next frame. */
    int m_FeedbackPass;

  public:
    // Instance of CShaderPreset
    static std::unique_ptr<CShaderPresetAddon> shaderPresetAddon;
    explicit CVideoShaderPreset();
    explicit CVideoShaderPreset(std::string presetPath);

    // Initializes CShaderPresetAddon instance that's used for calling the add-on's functions
    bool Init() override;
    void Destroy() override;

    bool ReadPresetFile(std::string presetPath) override;
    bool ReadPresetConfig(config_file_t_* presetConf) override;
    bool ReadPresetString(std::string presetString) override;
    bool ResolveParameters() override;
    // bool WritePresetFile(config_file_t_* presetConf) override;  // TODO?: preset file writing

    ~CVideoShaderPreset() override;

  protected:
     static void FreeConfigFile(config_file_t_* conf);
  private:
    config_file_t_* m_config;
    video_shader_* m_videoShader;
  };
} // namespace SHADERPRESET
