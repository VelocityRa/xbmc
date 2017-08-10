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

#include "cores/RetroPlayer/VideoShaderPreset.h"

#include <memory>
#include "guilib/Texture.h"

namespace SHADER
{
  typedef std::map<std::string, float> ShaderParameters;

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

  struct float2
  {
    float2() : x(0), y(0) {}
    float2(float x_, float y_) : x(x_), y(y_) {}
    float2(int x_, int y_) : x(x_), y(y_) {}
    float2(unsigned int x_, unsigned int y_) : x(x_), y(y_) {}

    operator XMFLOAT2() const { return XMFLOAT2(static_cast<float>(x), static_cast<float>(y)); }

    float x, y;
  };
}

using namespace SHADER;

class CVideoShader;

// TODO: renderer independence
class CVideoShaderManager
{
public:
  CVideoShaderManager(unsigned videoWidth = 0, unsigned videoHeight = 0);
  ~CVideoShaderManager();

  bool Update();
  bool SetShaderPreset(const std::string shaderPresetPath);
  void SetVideoSize(unsigned videoWidth, unsigned videoHeight);
  void Render(CRect sourceRect, CPoint dest[], CD3DTexture* target);
  CD3DTexture* GetFirstTexture();

private:
  bool CreateShaderTextures();
  ShaderParameters GetShaderParameters(video_shader_parameter_* parameters,
    unsigned numParameters, const std::string& sourceStr);
  bool CreateShaders();
  bool CreateSamplers();
  ID3D11SamplerState* CreateLUTSampler(const video_shader_lut_& lut);
  CDXTexture* CreateLUTexture(const video_shader_lut_& lut);
  // Returns smallest possible power-of-two sized texture
  float2 GetOptimalTextureSize(float2 videoSize);
  static D3D11_TEXTURE_ADDRESS_MODE TranslateWrapType(gfx_wrap_type_ wrap);
  void UpdateViewPort();
  void DisposeVideoShaders();

  // Loaded preset
  std::unique_ptr<SHADERPRESET::CVideoShaderPreset> m_pPreset;
  // Relative path of the currently loaded shader preset
  std::string m_videoShaderPath;
  // VideoShaders for the shader passes
  std::vector<std::unique_ptr<CVideoShader>> m_pVideoShaders;
  // Intermediate textures used for pixel shader passes
  std::vector<std::unique_ptr<CD3DTexture>> m_pShaderTextures;
  // Was the shader preset changed during the last frame?
  bool m_bPresetNeedsUpdate;
  // Size of the viewport
  float2 m_viewPortSize;
  // Size of the actual source video data (ie. 160x144 for the Game Boy)
  float2 m_videoSize;
  // Point/nearest neighbor sampler
  ID3D11SamplerState* m_pSampNearest;
  // Linear sampler
  ID3D11SamplerState* m_pSampLinear;
};
