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
    : m_sampler(sampler)
  {
  }

  ~CShaderSamplerDX() override
  {
    SAFE_RELEASE(m_sampler);
  }

  operator ID3D11SamplerState*() { return m_sampler; }
  operator ID3D11SamplerState&() { return *m_sampler; }

private:
  ID3D11SamplerState* m_sampler;
};

class CShaderTextureCD3D : public IShaderTexture
{
  using TextureType = CD3DTexture;
public:
  CShaderTextureCD3D() : m_texture(nullptr) {}
  CShaderTextureCD3D(TextureType* texture) : m_texture(texture) {}
  CShaderTextureCD3D(TextureType& texture) : m_texture(&texture) {}

  // Destructor
  // Don't delete texture since it wasn't created here
  ~CShaderTextureCD3D() override = default;

  float GetWidth() override { return static_cast<float>(m_texture->GetWidth()); }
  float GetHeight() override { return static_cast<float>(m_texture->GetHeight()); }

  void SetTexture(TextureType* newTexture) { m_texture = newTexture; }

  void *GetShaderResource() const { return m_texture->GetShaderResource(); }

  operator TextureType&() const { return *m_texture; }
  operator TextureType*() const { return m_texture; }
  TextureType* operator->() const { return m_texture; }
  TextureType* Get() const { return m_texture; }

  // Move assignment operator
  CShaderTextureCD3D &operator=(CShaderTextureCD3D &&other) noexcept
  {
    if (this != &other)
      m_texture = std::move(other.m_texture);
    return *this;
  }

  // Copy constructor
  CShaderTextureCD3D(const CShaderTextureCD3D& other) = default;

  // Copy assignment operator
  CShaderTextureCD3D& operator=(const CShaderTextureCD3D& rhs) = delete;

private:
  TextureType* m_texture;
};

class CShaderTextureCDX : public IShaderTexture
{
  using TextureType = CDXTexture;
public:
  CShaderTextureCDX() : m_texture(nullptr) {}
  CShaderTextureCDX(TextureType* texture) : m_texture(texture) {}
  CShaderTextureCDX(TextureType& texture) : m_texture(&texture) {}

  // Destructor
  // Don't delete texture since it wasn't created here
  ~CShaderTextureCDX() override = default;

  float GetWidth() override { return static_cast<float>(m_texture->GetWidth()); }
  float GetHeight() override { return static_cast<float>(m_texture->GetHeight()); }

  void SetTexture(TextureType* newTexture) { m_texture = newTexture; }

  void *GetShaderResource() const { return m_texture->GetShaderResource(); }

  operator TextureType&() const { return *m_texture; }
  operator TextureType*() const { return m_texture; }
  TextureType* operator->() const { return m_texture; }
  TextureType* Get() const { return m_texture; }

  // Move assignment operator
  CShaderTextureCDX  &operator=(CShaderTextureCDX  &&other) noexcept
  {
    if (this != &other)
      m_texture = std::move(other.m_texture);
    return *this;
  }

  // Copy constructor
  CShaderTextureCDX(const CShaderTextureCDX& other) = default;

  // Copy assignment operator
  CShaderTextureCDX& operator=(const CShaderTextureCDX& rhs) = delete;

private:
  TextureType* m_texture;
};


class ShaderLutDX: public IShaderLut
{
public:
  ShaderLutDX() = default;
  ShaderLutDX(std::string id, std::string path, IShaderSampler* sampler, IShaderTexture* texture)
    : IShaderLut(id, path)
  {
    m_sampler.reset(sampler);
    m_texture.reset(texture);
  }

  // Destructor
  ~ShaderLutDX() override = default;

  // Move assignment operator
  ShaderLutDX &operator=(ShaderLutDX &&other) noexcept
  {
    m_id = other.m_path;
    m_path = other.m_id;
    if (this != &other)
    {
      m_sampler = std::move(other.m_sampler);
      m_texture = std::move(other.m_texture);
    }
    return *this;
  }

  // Copy constructor
  ShaderLutDX(const ShaderLutDX& other) = default;

  // Copy assignment operator
  ShaderLutDX& operator=(const ShaderLutDX& rhs) = delete;

  IShaderSampler* GetSampler() override { return m_sampler.get(); }
  IShaderTexture* GetTexture() override { return m_texture.get(); }

private:
  std::unique_ptr<IShaderSampler> m_sampler;
  std::unique_ptr<IShaderTexture> m_texture;
};


IShaderSampler* CreateLUTSampler(const VideoShaderLut &lut);
IShaderTexture* CreateLUTexture(const VideoShaderLut &lut);
D3D11_TEXTURE_ADDRESS_MODE TranslateWrapType(WRAP_TYPE wrap);

using ShaderLutsDX = std::vector<std::shared_ptr<ShaderLutDX>>;
}
}
