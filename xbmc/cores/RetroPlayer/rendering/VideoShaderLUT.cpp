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

#include "VideoShaderLUT.h"
#include "utils/URIUtils.h"
#include "utils/log.h"
#include "settings/MediaSettings.h"
#include "windowing/WindowingFactory.h"
#include "cores/RetroPlayer/rendering/VideoShader.h"

#include <regex>

using namespace SHADER;

ID3D11SamplerState* SHADER::CreateLUTSampler(const video_shader_lut_ lut)
{
  ID3D11SamplerState* samp;
  D3D11_SAMPLER_DESC sampDesc;

  D3D11_TEXTURE_ADDRESS_MODE wrapType = TranslateWrapType(lut.wrap);
  auto filterType = lut.filter ? D3D11_FILTER_MIN_MAG_MIP_LINEAR : D3D11_FILTER_MIN_MAG_MIP_POINT;

  ZeroMemory(&sampDesc, sizeof(D3D11_SAMPLER_DESC));
  sampDesc.Filter = filterType;
  sampDesc.AddressU = wrapType;
  sampDesc.AddressV = wrapType;
  sampDesc.AddressW = wrapType;
  sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
  sampDesc.MinLOD = 0;
  sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

  FLOAT blackBorder[4] = { 0, 1, 0, 1 };  // TODO: turn this back to black
  memcpy(sampDesc.BorderColor, &blackBorder, 4 * sizeof(FLOAT));

  if (FAILED(g_Windowing.Get3D11Device()->CreateSamplerState(&sampDesc, &samp)))
  {
    CLog::Log(LOGWARNING, "%s - failed to create LUT sampler for LUT &s", __FUNCTION__, lut.path);
    return nullptr;
  }

  return samp;
}

CDXTexture* SHADER::CreateLUTexture(const video_shader_lut_ lut, const std::string& presetDirectory)
{
  const std::string& texturePath =
    URIUtils::AddFileToFolder(
      URIUtils::CanonicalizePath(
        URIUtils::GetBasePath(
        presetDirectory)),
    lut.path);

  CDXTexture* texture = static_cast<CDXTexture*>(CDXTexture::LoadFromFile(texturePath));
  if (!texture)
  {
    CLog::Log(LOGERROR, "Couldn't open LUT %s", lut.path);
    return nullptr;
  }
  if (lut.mipmap)
    texture->SetMipmapping();

  if (texture)
    texture->LoadToGPU();

  return texture;
}

D3D11_TEXTURE_ADDRESS_MODE SHADER::TranslateWrapType(const gfx_wrap_type_ wrap)
{
  switch(wrap)
  {
  case RARCH_WRAP_EDGE_:
    return D3D11_TEXTURE_ADDRESS_CLAMP;
  case RARCH_WRAP_REPEAT_:
    return D3D11_TEXTURE_ADDRESS_WRAP;
  case RARCH_WRAP_MIRRORED_REPEAT_:
    return D3D11_TEXTURE_ADDRESS_MIRROR;
  case RARCH_WRAP_DEFAULT_:
  default:
    return D3D11_TEXTURE_ADDRESS_BORDER;
  }
}

float2 SHADER::GetOptimalTextureSize(float2 videoSize)
{
  // TODO: enable again
  return videoSize;
  unsigned sizeMax = videoSize.Max<unsigned>();
  unsigned size = 1;

  // Find smallest possible power-of-two size that can contain input texture
  while (true)
  {
    if (size >= sizeMax)
      break;
    size *= 2;
  }
  return float2(size, size);
}
