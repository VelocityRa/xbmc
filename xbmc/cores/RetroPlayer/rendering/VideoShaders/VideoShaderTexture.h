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
    virtual ~IShaderTexture() = default;

    virtual float GetWidth() = 0;
    virtual float GetHeight() = 0;
  };

  class IShaderLut
  {
  public:
    IShaderLut() = default;
    IShaderLut(const std::string& id, const std::string& path)
      : m_id(id), m_path(path) {}

    virtual ~IShaderLut() = default;

    const std::string& GetID() { return m_id; }
    const std::string& GetPath() { return m_path; }
    virtual const IShaderSampler* GetSampler() = 0;
    virtual const IShaderTexture* GetTexture() = 0;

    IShaderLut &operator=(IShaderLut&& other)
    {
      m_id = other.m_id;
      m_path = other.m_path;
      return *this;
    }

  protected:
    std::string m_id;
    std::string m_path;
  };

  using IShaderLuts = std::vector<std::shared_ptr<IShaderLut>>;

  // Returns smallest possible power-of-two sized texture
  float2 GetOptimalTextureSize(float2 videoSize);
}
}
