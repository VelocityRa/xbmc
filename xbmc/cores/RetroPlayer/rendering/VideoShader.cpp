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
#include "guilib/Texture.h"

using namespace KODI;
using namespace SHADER;

CVideoShader::CVideoShader()
  :  m_pSampler(nullptr)
  ,  m_shaderParameters()
  ,  m_pInputBuffer(nullptr)
  ,  m_frameCountMod(0)
{
}

CVideoShader::~CVideoShader()
{
  SAFE_RELEASE(m_pInputBuffer);
}

bool CVideoShader::Create(const std::string& shaderSource, const std::string& shaderPath, ShaderParameters shaderParameters,
  ID3D11SamplerState* sampler, ShaderLUTs luts, float2 viewPortSize, unsigned frameCountMod)
{
  m_shaderSource = shaderSource;
  m_shaderPath = shaderPath;
  m_shaderParameters = shaderParameters;
  m_pSampler = sampler;
  m_luts = std::move(luts);
  m_viewportSize = viewPortSize;
  m_frameCountMod = frameCountMod;

  DefinesMap defines;

  defines["HLSL_4"] = "";  // using Shader Model 4
  defines["HLSL_FX"] = "";  // and the FX11 framework

  // We implement runtime shader parameters ("#pragma parameter")
  // NOTICE: Runtime shader parameters allow convenient experimentation with real-time
  //         feedback, as well as override-ability by presets, but they are much slower
  //         because they prevent static evaluation of a lot of math.
  //         Disabling them drastically speeds up shaders that use them heavily.
  defines["PARAMETER_UNIFORM"] = "";

  m_effect.AddIncludePath(URIUtils::GetBasePath(m_shaderPath));

  if (!m_effect.Create(shaderSource, &defines))
  {
    CLog::Log(LOGERROR, "%s: failed to load video shader: %s", __FUNCTION__, shaderPath.c_str());
    return false;
  }

  return true;
}

void CVideoShader::Render(CD3DTexture& source, CD3DTexture& target)
{
  // TODO: Doesn't work. Investigate calling this in Execute or binding the SRV first
  //g_Windowing.Get3D11Context()->PSSetSamplers(2, 1, &m_pSampler);
  SetShaderParameters(source);
  Execute({ &target }, 4);
}

void CVideoShader::SetShaderParameters(CD3DTexture& texture)
{
  m_effect.SetTechnique("TEQ");
  m_effect.SetResources("decal", { texture.GetAddressOfSRV() }, 1);
  m_effect.SetMatrix("modelViewProj", &m_MVP);
  // TODO(optimization): Add frame_count to separate cbuffer
  m_effect.SetConstantBuffer("input", m_pInputBuffer);
  for (const auto& param : m_shaderParameters)
    m_effect.SetFloatArray(param.first.c_str(), &param.second, 1);
  for (const auto& lut : m_luts)
    m_effect.SetTexture(lut->id.c_str(), lut->texture->GetShaderResource());
}

void CVideoShader::PrepareParameters(CPoint dest[4], bool isLastPass, float frameCount)
{
  UpdateInputBuffer(frameCount);

  CUSTOMVERTEX* v;
  LockVertexBuffer(reinterpret_cast<void**>(&v));

  if (!isLastPass)
  {
    CRect viewPort;
    g_Windowing.GetViewPort(viewPort);
    float viewportX = viewPort.Width();
    float viewportY = viewPort.Height();

    // top left
    v[0].x = -m_outputSize.x / 2.0f;
    v[0].y = -m_outputSize.y / 2.0f;
    v[0].z = 0.0f;
    v[0].tu = 0.0f;
    v[0].tv = 0.0f;
    // top right
    v[1].x = m_outputSize.x / 2.0f;
    v[1].y = -m_outputSize.y / 2.0f;
    v[1].z = 0.0f;
    v[1].tu = viewportX / m_outputSize.x;
    v[1].tv = 0.0f;
    // bottom right
    v[2].x = m_outputSize.x / 2.0f;
    v[2].y = m_outputSize.y / 2.0f;
    v[2].z = 0.0f;
    v[2].tu = viewportX / m_outputSize.x;
    v[2].tv = viewportY / m_outputSize.y;
    // bottom left
    v[3].x = -m_outputSize.x / 2.0f;
    v[3].y = m_outputSize.y / 2.0f;
    v[3].z = 0.0f;
    v[3].tu = 0.0f;
    v[3].tv = viewportY / m_outputSize.y;
  }
  else  // last pass
  {
    // top left
    v[0].x = dest[0].x - m_outputSize.x / 2.0f;
    v[0].y = dest[0].y - m_outputSize.y / 2.0f;
    v[0].z = 0.0f;
    v[0].tu = 0;
    v[0].tv = 0;
    // top right
    v[1].x = dest[1].x - m_outputSize.x / 2.0f;
    v[1].y = dest[1].y - m_outputSize.y / 2.0f;
    v[1].z = 0.0f;
    v[1].tu = 1.0f;
    v[1].tv = 0;
    // bottom right
    v[2].x = dest[2].x - m_outputSize.x / 2.0f;
    v[2].y = dest[2].y - m_outputSize.y / 2.0f;
    v[2].z = 0.0f;
    v[2].tu = 1.0f;
    v[2].tv = 1.0f;
    // bottom left
    v[3].x = dest[3].x - m_outputSize.x / 2.0f;
    v[3].y = dest[3].y - m_outputSize.y / 2.0f;
    v[3].z = 0.0f;
    v[3].tu = 0;
    v[3].tv = 1.0f;
  }
  UnlockVertexBuffer();
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

void CVideoShader::UpdateMVP()
{
  auto xScale = 1.0f / m_outputSize.x * 2.0;
  auto yScale = -1.0f / m_outputSize.y * 2.0;

  // Update projection matrix
  m_MVP = XMFLOAT4X4(
    xScale, 0, 0, 0,
    0, yScale, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1
  );
}

bool CVideoShader::CreateInputBuffer()
{
  ID3D11Device* pDevice = g_Windowing.Get3D11Device();
  cbInput inputInitData = GetInputData();
  auto inputBufSize = (sizeof(cbInput) + 15) & ~15;
  CD3D11_BUFFER_DESC cbInputDesc(inputBufSize, D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
  D3D11_SUBRESOURCE_DATA initInputSubresource = { &inputInitData, 0, 0 };
  if (FAILED(pDevice->CreateBuffer(&cbInputDesc, &initInputSubresource, &m_pInputBuffer)))
  {
    CLog::Log(LOGERROR, __FUNCTION__ " - Failed to create constant buffer for video shader input data.");
    return false;
  }
  return true;
}

void CVideoShader::UpdateInputBuffer(float frameCount)
{
  auto pContext = g_Windowing.Get3D11Context();

  cbInput input = GetInputData(frameCount);
  cbInput* pData;
  void** ppData = reinterpret_cast<void**>(&pData);

  D3D11_MAPPED_SUBRESOURCE resource;
  if (SUCCEEDED(pContext->Map(m_pInputBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource)))
  {
    *ppData = resource.pData;

    memcpy(*ppData, &input, sizeof(cbInput));
    pContext->Unmap(m_pInputBuffer, 0);
  }
}

CVideoShader::cbInput CVideoShader::GetInputData(float frameCountFloat)
{
  unsigned frameCount = static_cast<unsigned>(frameCountFloat);
  if (m_frameCountMod != 0)
    frameCount %= m_frameCountMod;

  cbInput input = {
    // Resution of texture passed to the shader
    { m_inputSize },    // video_size
    // Shaders don't (and shouldn't) know about _actual_ texture
    // size, because D3D gives them correct texture coordinates
    { m_inputSize },    // texture_size
    // As per the spec, this is the viewport resolution (not the
    // output res of each shader
    { m_viewportSize }, // output_size
    // Current frame count that can be modulo'ed
    { frameCount },     // frame_count
    // Time always flows forward
    { 1 }               // frame_direction
  };
  return input;
}


void CVideoShader::SetSizes(const float2& prevSize, const float2& nextSize)
{
  m_inputSize = prevSize;
  m_outputSize = nextSize;
}
