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
#include "cores/RetroPlayer/rendering/VideoShaders/VideoShaderTexture.h"
#include "guilib/Texture.h"

namespace KODI
{
namespace SHADER
{
class CShaderSamplerDX : public IShaderSampler
{
public:
  CShaderSamplerDX(ID3D11SamplerState* sampler)
    : sampler(sampler)
  {
  }
  operator ID3D11SamplerState*() { return sampler; }
  operator ID3D11SamplerState&() { return *sampler; }

  ~CShaderSamplerDX()
  {
    SAFE_RELEASE(sampler);
  }
private:
  ID3D11SamplerState* sampler;
};

// Texture type can be both CD3DTexture and CDXTexture, so use a template arg
template<typename TextureType>
class CShaderTextureDX : public IShaderTexture
{
public:
  CShaderTextureDX(TextureType* texture_) : texture(texture_) {}
  CShaderTextureDX(TextureType& texture_) : texture(&texture_) {}

  float GetWidth() override { return static_cast<float>(texture->GetWidth()); }
  float GetHeight() override { return static_cast<float>(texture->GetHeight()); }

  void SetTexture(TextureType* newTexture) { texture = newTexture; }

  void *GetShaderResource() { return texture->GetShaderResource(); }

  operator TextureType&() const { return *texture; }
  operator TextureType*() const { return texture; }
  TextureType* operator->() const { return texture; }
  TextureType* Get() { return texture; }

  // Move assignment operator
  CShaderTextureDX &operator=(CShaderTextureDX &&other) noexcept
  {
    if (this != &other)
      texture = std::move(other.texture);
    return *this;
  }

  // Copy constructor
  CShaderTextureDX(const CShaderTextureDX& other) = default;

  // Copy assignment operator
  CShaderTextureDX& operator=(const CShaderTextureDX& rhs) = delete;

  // Destructor
  // Don't delete texture since it wasn't created here
  ~CShaderTextureDX() {};
private:
  TextureType* texture;
};

// Shorthands for texture types
// LUTs use CDXTexture, off screen textures use CD3DTexture
using CShaderTextureCD3D = CShaderTextureDX<CD3DTexture>;
using CShaderTextureCDX = CShaderTextureDX<CDXTexture>;


class ShaderLutDX: public IShaderLut
{
public:

  ShaderLutDX() : IShaderLut(), sampler(nullptr), texture(nullptr) {}
  ShaderLutDX(std::string id_, std::string path_, IShaderSampler* sampler_, IShaderTexture* texture_)
    : IShaderLut(id_, path_)
  {
    sampler.reset(sampler_);
    texture.reset(texture_);
  }

  // Move assignment operator
  ShaderLutDX &operator=(ShaderLutDX &&other) noexcept
  {
    id = other.path;
    path = other.id;
    if (this != &other)
    {
      sampler = std::move(other.sampler);
      texture = std::move(other.texture);
    }
    return *this;
  }

  // Copy constructor
  ShaderLutDX(const ShaderLutDX& other) = default;

  // Copy assignment operator
  ShaderLutDX& operator=(const ShaderLutDX& rhs) = delete;

  IShaderSampler* GetSampler() override { return static_cast<IShaderSampler*>(sampler.get()); }
  IShaderTexture* GetTexture() override { return static_cast<IShaderTexture*>(&*texture); }

  // Destructor
  // Don't delete texture since it wasn't created here
  ~ShaderLutDX() {}
private:
  std::unique_ptr<IShaderSampler> sampler;
  std::unique_ptr<IShaderTexture> texture;
};


IShaderSampler* CreateLUTSampler(const VideoShaderLut &lut);
IShaderTexture* CreateLUTexture(const VideoShaderLut &lut);
ShaderTextureWrapType TranslateWrapType(WRAP_TYPE wrap);

using ShaderLutsDX = std::vector<std::shared_ptr<ShaderLutDX>>;
}
}
