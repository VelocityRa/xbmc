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
#include "VideoShaderLUT.h"

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
    ID3D11SamplerState* sampler, std::vector<ShaderLUT> luts);
  void Render(CRect sourceRect, CPoint dest[], CD3DTexture& texture, CD3DTexture& target);

  // expose these from CWinShader
  bool LockVertexBuffer(void **data) override;
  bool UnlockVertexBuffer() override;
  bool CreateVertexBuffer(unsigned vertCount, unsigned vertSize) override;
  bool CreateInputLayout(D3D11_INPUT_ELEMENT_DESC *layout, unsigned numElements) override;
  CD3DEffect& GetEffect();

protected:
  void SetShaderParameters(CD3DTexture& videoBuffer);

private:
  // Currently loaded shader's source code
  std::string m_shaderSource;

  // Currently loaded shader's relative path
  std::string m_shaderPath;

  // Array of shader parameters
  ShaderParameters m_shaderParameters;

  // Sampler state
  ID3D11SamplerState* m_pSampler;

  // Look-up textures that the shader uses
  std::vector<ShaderLUT> m_luts;
};
