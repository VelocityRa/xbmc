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

#include <string>
#include "addons/ShaderPreset.h"

namespace SHADERPRESET
{
  /*!
   * \brief Interface for reading and writing shader preset files
   */
  class IVideoShaderPreset
  {
  public:
    virtual ~IVideoShaderPreset() = default;

    /**
     * \brief Perform initialization of the object
     *        Implementation may choose to perform it if the object is
     *        used without having this having been called.
     * \return True on successful initialization, false on failed.
     */
    virtual bool Init() = 0;

    /**
     * \brief Perform deinitialization of the object.
     *        May be used to get rid of unneded resources.
     */
    virtual void Destroy() = 0;


    /**
     * \brief Reads/Parses a shader preset file and loads its state to the
     *        object. What this state is is implementation specific.
     * \param presetPath Full path of the preset file.
     * \return True on successful parsing, false on failed.
     */
    virtual bool ReadPresetFile(std::string presetPath) = 0;

    /**
    * \brief Reads a shader preset already parsed in a config_file
    *        and loads its state to the object. What this state is
    *        is implementation specific.
    * \param presetConf Configuration file of the preset file that's been parsed.
    * \return True if successfully read, false on failure.
    */
    virtual bool ReadPresetConfig(config_file* presetConf) = 0;

    /**
    * \brief Reads/Parses a shader preset already loaded in memory as a
    *        string and loads its state to the object. What this state
    *        is is implementation specific.
    * \param presetString String of the preset file.
    * \return True on successful parsing, false on failed.
    */
    virtual bool ReadPresetString(std::string presetString) = 0;

    /**
    * \brief Frees all state related to shader
    * \param shader Object to free.
    */
    virtual void FreePresetFile(video_shader* shader) = 0;

    /**
    * ShaderPresetResolveParameters:
    *
    * Resolves all shader parameters belonging to shaders.
    *
    * Returns: True if successful, otherwise false false.
    **/
    virtual bool ResolveParameters() = 0;

    // virtual bool WritePresetFile(conf_file_t* presetConf) = 0;
  };
} // namespace SHADERPRESET
