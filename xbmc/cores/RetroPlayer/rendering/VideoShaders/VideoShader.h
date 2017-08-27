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

#include "VideoShaderTexture.h"

namespace KODI
{
namespace SHADER
{
  struct float2;
  class IShaderTexture;

  class IVideoShader
  {
  public:
    virtual bool Create(const std::string& shaderSource, const std::string& shaderPath, ShaderParameters shaderParameters,
      IShaderSampler* sampler, IShaderLuts luts, float2 viewPortSize, unsigned frameCountMod = 0) = 0;
    virtual void Render(IShaderTexture* source, IShaderTexture* target) = 0;
    virtual void SetSizes(const float2& prevSize, const float2& nextSize) = 0;
    virtual bool CreateVertexBuffer(unsigned vertCount, unsigned vertSize) = 0;
    virtual bool CreateInputLayout(D3D11_INPUT_ELEMENT_DESC* layout, unsigned numElements) = 0;
    virtual bool CreateInputBuffer() = 0;
    virtual void PrepareParameters(CPoint dest[4], bool isLastPass, float frameCount) = 0;
    virtual void UpdateMVP() = 0;

    virtual ~IVideoShader() = default;
  };
}
}