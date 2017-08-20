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
#include <vector>

namespace KODI
{
namespace SHADER
{
  enum FILTER_TYPE
  {
    FILTER_TYPE_NONE,
    FILTER_TYPE_LINEAR,
    FILTER_TYPE_NEAREST
  };

  enum WRAP_TYPE
  {
    WRAP_TYPE_BORDER,
    WRAP_TYPE_EDGE,
    WRAP_TYPE_REPEAT,
    WRAP_TYPE_MIRRORED_REPEAT,
  };

  enum SCALE_TYPE
  {
    SCALE_TYPE_INPUT,
    SCALE_TYPE_ABSOLUTE,
    SCALE_TYPE_VIEWPORT,
  };

  struct FboScaleAxis
  {
    SCALE_TYPE type = SCALE_TYPE_INPUT;
    float scale = 1.0;
    unsigned int abs = 1;
  };

  struct FboScale
  {
    bool sRgbFramebuffer = false;
    bool floatFramebuffer = false;
    FboScaleAxis scaleX;
    FboScaleAxis scaleY;
  };

  struct VideoShaderPass
  {
    std::string sourcePath;
    std::string vertexSource;
    std::string fragmentSource;
    std::string alias;
    FILTER_TYPE filter = FILTER_TYPE_NONE;
    WRAP_TYPE wrap = WRAP_TYPE_BORDER;
    unsigned int frameCountMod = 0;
    FboScale fbo;
    bool mipmap = false;
  };

  struct VideoShaderLut
  {
    std::string strId;
    std::string path;
    FILTER_TYPE filter = FILTER_TYPE_NONE;
    WRAP_TYPE wrap = WRAP_TYPE_BORDER;
    bool mipmap = false;
  };

  struct VideoShaderParameter
  {
    std::string strId;
    std::string description;
    float current = 0.0f;
    float minimum = 0.0f;
    float initial = 0.0f;
    float maximum = 0.0f;
    float step = 0.0f;
  };

  struct VideoShaderPreset
  {
    std::vector<VideoShaderPass> passes;
    std::vector<VideoShaderLut> luts;
    std::vector<VideoShaderParameter> parameters;
  };
}
}

namespace SHADERPRESET
{
  /*!
   * \brief Interface for reading and writing shader preset files
   */
  class IVideoShaderPreset
  {
  public:
    virtual ~IVideoShaderPreset() = default;

    /*!
     * \brief Perform initialization of the object
     *        Implementation may choose to perform it if the object is
     *        used without having this having been called.
     * \return True on successful initialization, false on failed.
     */
    virtual bool Init() = 0;

    /*!
     * \brief Perform deinitialization of the object.
     *        May be used to get rid of unneded resources.
     */
    virtual void Destroy() = 0;

    /*!
     * \brief Reads/Parses a shader preset file and loads its state to the
     *        object. What this state is is implementation specific.
     * \param presetPath Full path of the preset file.
     * \return True on successful parsing, false on failed.
     */
    virtual bool ReadPresetFile(const std::string &presetPath) = 0;

    // virtual bool WritePresetFile() = 0;
  };
}
