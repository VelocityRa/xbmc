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

#include <memory>
#include <string>
#include "system.h"
#include "VideoShaderUtils.h"

struct video_shader_lut_;
enum gfx_wrap_type_;
class CDXTexture;

namespace SHADER
{
  struct ShaderLUT
  {
    std::string id;
    std::string path;
    std::unique_ptr<ID3D11SamplerState> sampler;
    std::unique_ptr<CDXTexture> texture;

    ShaderLUT() {}
    ShaderLUT(std::string id_, std::string path_,
      ID3D11SamplerState* sampler_,
      CDXTexture* texture_)
      : id(id_), path(path_)
    {
      sampler.reset(sampler_);
      texture.reset(texture_);
    }
    ~ShaderLUT()
    {
      auto pSampler = sampler.release();
      SAFE_RELEASE(pSampler);
      if (texture)
        texture.reset();
    }
    ShaderLUT(const ShaderLUT& other)
    {
      id = other.id;
      path = other.path;
      sampler.reset(std::move(other.sampler.get()));
      texture.reset(std::move(other.texture.get()));
    }
    ShaderLUT& operator=(const ShaderLUT& rhs)
    {
      ShaderLUT tmp(rhs);
      std::swap(id, tmp.id);
      std::swap(path, tmp.path);
      std::swap(sampler, tmp.sampler);
      std::swap(texture, tmp.texture);
      return *this;
    }
  };

  ID3D11SamplerState* CreateLUTSampler(const video_shader_lut_ lut);
  CDXTexture* CreateLUTexture(const video_shader_lut_ lut, const std::string& presetDirectory);
  // Returns smallest possible power-of-two sized texture
  float2 GetOptimalTextureSize(float2 videoSize);
  D3D11_TEXTURE_ADDRESS_MODE TranslateWrapType(const gfx_wrap_type_ wrap);
}
