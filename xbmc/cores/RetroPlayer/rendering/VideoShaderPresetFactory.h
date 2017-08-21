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

#include "addons/Addon.h"
#include "xbmc/cores/RetroPlayer/IVideoShaderPreset.h"

#include <map>
#include <string>

namespace ADDON
{
  struct AddonEvent;
  class CAddonMgr;
  class CBinaryAddonManager;
  class CShaderPresetAddon;
}

namespace KODI
{
namespace SHADER
{
  class IVideoShaderPreset;

  class IVideoShaderPresetLoader
  {
  public:
    virtual ~IVideoShaderPresetLoader() = default;

    virtual bool LoadPreset(const std::string &presetPath, VideoShaderPreset &shaderPreset) = 0;
  };

  class CVideoShaderPresetFactory
  {
  public:
    /*!
     * \brief Create the factory and register all shader preset add-ons
     */
    CVideoShaderPresetFactory(ADDON::CAddonMgr &addons, ADDON::CBinaryAddonManager &binaryAddons);
    ~CVideoShaderPresetFactory();

    void RegisterLoader(IVideoShaderPresetLoader *loader, const std::string &extension);
    void UnregisterLoader(IVideoShaderPresetLoader *loader);

    bool LoadPreset(const std::string &presetPath, VideoShaderPreset &shaderPreset);

  private:
    void OnEvent(const ADDON::AddonEvent &event);
    void UpdateAddons();

    // Construction parameters
    ADDON::CAddonMgr &m_addons;
    ADDON::CBinaryAddonManager &m_binaryAddons;

    std::map<std::string, IVideoShaderPresetLoader*> m_loaders;
    std::vector<std::unique_ptr<ADDON::CShaderPresetAddon>> m_shaderAddons;
  };
}
}
