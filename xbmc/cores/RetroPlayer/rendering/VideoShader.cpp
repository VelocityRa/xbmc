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

#include "VideoShader.h"
#include "utils/URIUtils.h"
#include "utils/log.h"
#include "windowing/WindowingFactory.h"
#include "Application.h"

using namespace SHADER;

CVideoShader::CVideoShader()
  :  m_pSampler(nullptr)
{
}

CVideoShader::~CVideoShader()
{
}

bool CVideoShader::Create(const std::string& shaderSource, const std::string& shaderPath, ShaderParameters shaderParameters,
  ID3D11SamplerState* sampler, std::vector<ShaderLUT> luts)
{
  m_shaderSource = shaderSource;
  m_shaderPath = shaderPath;
  m_shaderParameters = shaderParameters;
  m_pSampler = sampler;
  m_luts = std::move(luts);

  DefinesMap defines;

  defines["HLSL_4"] = "";  // using Shader Model 4
  defines["HLSL_FX"] = "";  // and the FX11 framework
  defines["PARAMETER_UNIFORM"] = "";  // we implement parameters ("#pragma parameter")

  m_effect.AddIncludePath(URIUtils::GetBasePath(m_shaderPath));

  if (!m_effect.Create(shaderSource, &defines))
  {
    CLog::Log(LOGERROR, "%s: failed to load video shader: %s", __FUNCTION__, shaderPath.c_str());
    return false;
  }

  return true;
}

void CVideoShader::Render(CD3DTexture& texture, CD3DTexture& target)
{
  // TODO: Doesn't work. Investigate calling this in Execute or binding the SRV first
  //g_Windowing.Get3D11Context()->PSSetSamplers(2, 1, &m_pSampler);
  SetShaderParameters();
  Execute({ &target }, 4);
}

void CVideoShader::SetShaderParameters()
{
  for (const auto& param : m_shaderParameters)
    m_effect.SetFloatArray(param.first.c_str(), &param.second, 1);
  for (const auto& lut : m_luts)
    m_effect.SetTexture(lut.id.c_str(), lut.texture->GetShaderResource());
}

bool CVideoShader::LockVertexBuffer(void** data)
{
  return CWinShader::LockVertexBuffer(data);
}

bool CVideoShader::UnlockVertexBuffer()
{
  return CWinShader::UnlockVertexBuffer();
}

bool CVideoShader::CreateVertexBuffer(unsigned vertCount, unsigned vertSize)
{
  return CWinShader::CreateVertexBuffer(vertCount, vertSize);
}

bool CVideoShader::CreateInputLayout(D3D11_INPUT_ELEMENT_DESC* layout, unsigned numElements)
{
  return CWinShader::CreateInputLayout(layout, numElements);
}

CD3DEffect& CVideoShader::GetEffect()
{
  return m_effect;
}
