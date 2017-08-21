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

#include "guilib/D3DResource.h"
#include "VideoRenderers/VideoShaders/WinVideoFilter.h"
#include "VideoShaderLUT.h"

namespace KODI
{
namespace SHADER
{

class CVideoPixelShader : public CD3DPixelShader
{
public:
  CVideoPixelShader() : CD3DPixelShader() {}
};

class CVideoVertexShader : public CD3DVertexShader
{
public:
  CVideoVertexShader() : CD3DVertexShader() {}
};

// TODO: make renderer independent
// libretro's "Common shaders"
// Spec here: https://github.com/libretro/common-shaders/blob/master/docs/README
class CVideoShader : public CWinShader
{
public:
  CVideoShader();
  ~CVideoShader();
  bool Create(const std::string& shaderSource, const std::string& shaderPath, ShaderParameters shaderParameters,
    ID3D11SamplerState* sampler, ShaderLUTs luts, float2 viewPortSize, unsigned frameCountMod = 0);
  void Render(CD3DTexture& source, CD3DTexture& target);
  void PrepareParameters(CPoint dest[4], bool isLastPass, float frameCount);
  CD3DEffect& GetEffect();
  void UpdateMVP();
  bool CreateInputBuffer();
  void UpdateInputBuffer(float frameCountFloat);

  // expose these from CWinShader
  bool CreateVertexBuffer(unsigned vertCount, unsigned vertSize) override;
  bool CreateInputLayout(D3D11_INPUT_ELEMENT_DESC *layout, unsigned numElements) override;
  void SetSizes(const float2& prevSize, const float2& nextSize);

protected:
  void SetShaderParameters(CD3DTexture& texture);

private:
  struct cbInput
  {
    XMFLOAT2 video_size;
    XMFLOAT2 texture_size;
    XMFLOAT2 output_size;
    float frame_count;
    float frame_direction;
  };

  // Currently loaded shader's source code
  std::string m_shaderSource;

  // Currently loaded shader's relative path
  std::string m_shaderPath;

  // Array of shader parameters
  ShaderParameters m_shaderParameters;

  // Sampler state
  ID3D11SamplerState* m_pSampler;

  // Look-up textures that the shader uses
  ShaderLUTs m_luts;

  // Resolution of the input of the shader
  float2 m_inputSize;

  // Resolution of the output of the shader
  float2 m_outputSize;

  // Resolution of the viewport/window
  float2 m_viewportSize;

  // Holds the data bount to the input cbuffer (cbInput here)
  ID3D11Buffer* m_pInputBuffer;

  // Projection matrix
  XMFLOAT4X4 m_MVP;

  // Value to modulo (%) frame count with
  // Unused if 0
  unsigned m_frameCountMod;

private:
  cbInput GetInputData(float frameCount = 0);
};

}
}
