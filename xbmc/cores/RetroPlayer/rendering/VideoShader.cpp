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
  : m_pInputBuffer(nullptr)
  , m_pSampler(nullptr)
{
}

CVideoShader::~CVideoShader()
{
  SAFE_RELEASE(m_pInputBuffer);
}

bool CVideoShader::Create(const std::string& shaderSource, const std::string& shaderPath, ShaderParameters shaderParameters,
  ID3D11SamplerState* sampler, std::vector<ShaderLUT> luts, float2 videoSize, float2 textureSize)
{
  m_shaderSource = shaderSource;
  m_shaderPath = shaderPath;
  m_shaderParameters = shaderParameters;
  m_pSampler = sampler;
  m_videoSize = videoSize;
  m_textureSize = textureSize;
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

  CVideoShader::CreateVertexBuffer(4, sizeof(CUSTOMVERTEX));
  // Create input layout
  D3D11_INPUT_ELEMENT_DESC layout[] =
  {
    { "SV_POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT,    0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 }
  };

  if (!CVideoShader::CreateInputLayout(layout, ARRAYSIZE(layout)))
  {
    CLog::Log(LOGERROR, __FUNCTION__": Failed to create input layout for Input Assembler.");
    return false;
  }

  CRect viewPort;
  g_Windowing.GetViewPort(viewPort);
  SetViewPort(viewPort);

  if (!CreateBuffers())
    return false;

  return true;
}

void CVideoShader::Render(CRect sourceRect, CPoint dest[], CD3DTexture* texture, CD3DTexture *target)
{
  PrepareParameters(texture, sourceRect, dest);
  // TODO: Doesn't work. Investigate calling this in Execute or binding the SRV first
  //g_Windowing.Get3D11Context()->PSSetSamplers(2, 1, &m_pSampler);
  SetShaderParameters(texture);
  Execute({ target }, 4);
}

void CVideoShader::PrepareParameters(CD3DTexture* texture, CRect sourceRect, CPoint dest[])
{
  if (m_sourceRect != sourceRect
    || m_dest[0] != dest[0] || m_dest[1] != dest[1]
    || m_dest[2] != dest[2] || m_dest[3] != dest[3]
    || texture->GetWidth() != m_outputSize.x
    || texture->GetHeight() != m_outputSize.y)
  {
    m_sourceRect = sourceRect;
    for (size_t i = 0; i < 4; ++i)
      m_dest[i] = dest[i];
    m_outputSize = { texture->GetWidth(), texture->GetHeight() } ;

    CUSTOMVERTEX* v;
    CVideoShader::LockVertexBuffer(reinterpret_cast<void**>(&v));

    v[0].x = m_dest[0].x - m_outputSize.x / 2.0f;
    v[0].y = m_dest[0].y - m_outputSize.y / 2.0f;
    v[0].z = 0.0f;
    v[0].tu = sourceRect.x1 / m_outputSize.x;
    v[0].tv = sourceRect.y1 / m_outputSize.y;

    v[1].x = m_dest[1].x - m_outputSize.x / 2.0f;
    v[1].y = m_dest[1].y - m_outputSize.y / 2.0f;
    v[1].z = 0.0f;
    v[1].tu = sourceRect.x2 / m_outputSize.x;
    v[1].tv = sourceRect.y1 / m_outputSize.y;

    v[2].x = m_dest[2].x - m_outputSize.x / 2.0f;
    v[2].y = m_dest[2].y - m_outputSize.y / 2.0f;
    v[2].z = 0.0f;
    v[2].tu = sourceRect.x2 / m_outputSize.x;
    v[2].tv = sourceRect.y2 / m_outputSize.y;

    v[3].x = m_dest[3].x - m_outputSize.x / 2.0f;
    v[3].y = m_dest[3].y - m_outputSize.y / 2.0f;
    v[3].z = 0.0f;
    v[3].tu = sourceRect.x1 / m_outputSize.x;
    v[3].tv = sourceRect.y2 / m_outputSize.y;

    CVideoShader::UnlockVertexBuffer();

    // Update projection matrix and input cbuffer
    CRect viewPort;
    g_Windowing.GetViewPort(viewPort);
    SetViewPort(viewPort);
    UpdateInputBuffer();
  }
  UpdateInputBuffer();
}

void CVideoShader::SetShaderParameters(CD3DTexture* texture)
{
  m_effect.SetTechnique("TEQ");
  m_effect.SetResources("decal", { texture->GetAddressOfSRV() }, 1);
  // TODO: (Optimization): Add frame_count to separate cbuffer
  m_effect.SetConstantBuffer("input", m_pInputBuffer);
  m_effect.SetMatrix("modelViewProj", &m_MVP);
  for (const auto& param : m_shaderParameters)
    m_effect.SetFloatArray(param.first.c_str(), &param.second, 1);
  for (const auto& lut : m_luts)
    m_effect.SetTexture(lut.id.c_str(), lut.texture->GetShaderResource());
}

bool CVideoShader::CreateBuffers()
{
  CRect viewPort;
  g_Windowing.GetViewPort(viewPort);

  m_outputSize = { static_cast<unsigned>(viewPort.Width()), static_cast<unsigned>(viewPort.Height()) };

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

void CVideoShader::SetViewPort(const CRect& viewPort)
{
  auto xProjection = 1.0f / viewPort.Width() * 2.0f;
  auto yProjection = -1.0f / viewPort.Height() * 2.0f;

  // Update projection matrix
  m_MVP = XMFLOAT4X4(
    xProjection, 0, 0, 0,
    0, yProjection, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1
  );

  // Update output size
  m_outputSize = { static_cast<unsigned>(viewPort.Width()), static_cast<unsigned>(viewPort.Height()) };
}

void CVideoShader::UpdateInputBuffer()
{
  auto pContext = g_Windowing.Get3D11Context();

  cbInput input = GetInputData();
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

CVideoShader::cbInput CVideoShader::GetInputData()
{

  cbInput input = {
    { m_videoSize },   // video_size
    { m_textureSize }, // texture_size
    { m_outputSize },  // output_size
    { g_application.GetTime() * 60 },             // TODO: frame_count
    { 1 }              // frame_direction
  };
  return input;
}