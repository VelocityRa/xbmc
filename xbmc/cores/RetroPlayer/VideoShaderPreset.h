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

#include <memory>
#include <vector>

namespace ADDON
{
  class CShaderPreset;
  class CShaderPresetAddon;
}

namespace SHADERPRESET
{
  class CVideoShaderPreset : public IVideoShaderPreset
  {
  public:
    // Instance of CShaderPreset
    static std::unique_ptr<ADDON::CShaderPresetAddon> shaderPresetAddon;
    explicit CVideoShaderPreset();

    // Initializes CShaderPresetAddon instance that's used for calling the add-on's functions
    bool Init() override;
    void Destroy() override;

    bool ReadPresetFile(const std::string &presetPath) override;
    // bool WritePresetFile(config_file* presetConf) override;  // TODO?: preset file writing

    ~CVideoShaderPreset() override;

    const KODI::SHADER::VideoShaderPreset &Preset() const { return m_videoShader; }

  private:
    KODI::SHADER::VideoShaderPreset m_videoShader;
  };
} // namespace SHADERPRESET
