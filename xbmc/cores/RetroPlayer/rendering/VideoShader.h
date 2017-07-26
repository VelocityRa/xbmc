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
#include "VideoShaderManager.h"

using namespace SHADER;

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
  virtual ~CVideoShader();
  bool Create(const std::string& shaderSource, const std::string& shaderPath, ShaderParameters shaderParameters,
    ID3D11SamplerState* sampler, float2 videoSize, float2 textureSize);
  void Render(CRect sourceRect, CPoint dest[], CD3DTexture* texture, CD3DTexture *target);
  void SetViewPort(const CRect& viewPort);

protected:
  void UpdateInputBuffer();
  void PrepareParameters(CD3DTexture* videoBuffer, CRect sourceRect, CPoint dest[]);
  void SetShaderParameters(CD3DTexture* videoBuffer);
  bool CreateBuffers();

private:
  struct cbInput
  {
    XMFLOAT2 video_size;
    XMFLOAT2 texture_size;
    XMFLOAT2 output_size;
    float frame_count;
    float frame_direction;
  };
  struct CUSTOMVERTEX {
    FLOAT x, y, z;
    FLOAT tu, tv;   // Texture coordinates
  };
  // Currently loaded shader's source code
  std::string m_shaderSource;

  // Currently loaded shader's relative path
  std::string m_shaderPath;

  // Holds the data bount to the input cbuffer (cbInput here)
  ID3D11Buffer* m_pInputBuffer;

  // Projection matrix
  XMFLOAT4X4 m_MVP;

  // Size of the viewport
  // cbInput's 'output_size'
  float2 m_outputSize;

  // Size of the actual source video data (ie. 160x144 for the Game Boy)
  // cbInput's 'video_size'
  float2 m_videoSize;

  // The size of the texture itself
  // Power-of-two sized.
  // cbInput's 'texture_size'
  float2 m_textureSize;

  // Array of shader parameters
  ShaderParameters m_shaderParameters;

  // Sampler state
  ID3D11SamplerState* m_pSampler;

  CRect m_sourceRect;
  CPoint m_dest[4];

private:
  cbInput GetInputData();
};
