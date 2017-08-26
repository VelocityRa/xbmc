#pragma once
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

#include "cores/RetroPlayer/IVideoShaderPreset.h"
#include "cores/RetroPlayer/rendering/VideoShaders/VideoShaderUtils.h"

class CDXTexture;

namespace KODI
{
namespace SHADER
{
  struct VideoShaderLut;

  class IShaderTexture
  {
  public:
    virtual ~IShaderTexture() {}

    virtual float GetWidth() = 0;
    virtual float GetHeight() = 0;
  };

  class IShaderLut
  {
  public:
    IShaderLut() : id(""), path("") {}
    IShaderLut(const std::string& id_, const std::string& path_)
      : id(id_), path(path_) {}

    const std::string& GetID() { return id; }
    const std::string& GetPath() { return path; }
    virtual const IShaderSampler* GetSampler() = 0;
    virtual const IShaderTexture* GetTexture() = 0;

    IShaderLut &operator=(IShaderLut&& other)
    {
      id = other.id;
      path = other.path;
      return *this;
    }

    virtual ~IShaderLut() = default;
  protected:
    std::string id;
    std::string path;

  };

  using IShaderLuts = std::vector<std::shared_ptr<IShaderLut>>;

  // Returns smallest possible power-of-two sized texture
  float2 GetOptimalTextureSize(float2 videoSize);
}
}
