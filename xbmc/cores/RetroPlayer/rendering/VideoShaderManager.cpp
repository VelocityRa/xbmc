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

#include "VideoShaderManager.h"
#include "utils/URIUtils.h"
#include "utils/log.h"
#include "settings/MediaSettings.h"
#include "windowing/WindowingFactory.h"
#include "cores/RetroPlayer/rendering/VideoShader.h"
#include "cores/RetroPlayer/rendering/VideoShaderLUT.h"
#include "addons/kodi-addon-dev-kit/include/kodi/addon-instance/ShaderPreset.h"

#include <regex>
#include "cores/RetroPlayer/RetroPlayer.h"

using namespace SHADER;

CVideoShaderManager::CVideoShaderManager(unsigned videoWidth, unsigned videoHeight)
  : m_bPresetNeedsUpdate(true)
  , m_pSampNearest(nullptr)
  , m_pSampLinear(nullptr)
  , m_videoSize(videoWidth, videoHeight)
  , m_textureSize()
  , m_pInputBuffer(nullptr)
  , m_frameCount(0)
{
  m_videoSize = { videoWidth, videoHeight };

  CRect viewPort;
  g_Windowing.GetViewPort(viewPort);
  m_viewPortSize = {viewPort.Width(), viewPort.Height()};

  m_pVideoShaders.reserve(GFX_MAX_SHADERS);
  m_pShaderTextures.reserve(GFX_MAX_SHADERS);
}

CVideoShaderManager::~CVideoShaderManager()
{
  DisposeVideoShaders();
  SAFE_RELEASE(m_pInputBuffer);
  // The gui is going to render after this, so apply the state required
  g_Windowing.ApplyStateBlock();
}

void CVideoShaderManager::Render(CRect sourceRect, CPoint dest[], CD3DTexture& target)
{
  // Update shaders/shader textures if required
  if (!Update())
    return;

  // Handle resizing of the viewport (window)
  UpdateViewPort();

  PrepareParameters(target, sourceRect, dest);

  // At this point, the input video has been rendered to the first texture
  // Apply all passes except the last one (which needs to be applied to the backbuffer)
  for (unsigned shaderIdx = 0; shaderIdx < m_pVideoShaders.size() - 1; ++shaderIdx)
  {
    CVideoShader& shader = *m_pVideoShaders[shaderIdx];
    CD3DTexture& texture = *m_pShaderTextures[shaderIdx];
    CD3DTexture& nextTexture = *m_pShaderTextures[shaderIdx + 1];

    SetCommonShaderParams(shader, texture);
    shader.Render(texture, nextTexture);
  }

  // Apply last pass and write to target (backbuffer)
  CVideoShader& lastShader = *m_pVideoShaders.back();
  CD3DTexture& lastTexture = *m_pShaderTextures.back();
  SetCommonShaderParams(lastShader, lastTexture);
  lastShader.Render(lastTexture, target);

  if (m_speed != 0.0)
    ++m_frameCount;

  // Restore our view port.
  g_Windowing.RestoreViewPort();
}

bool CVideoShaderManager::Update()
{
  if (m_bPresetNeedsUpdate)
  {
    m_bPresetNeedsUpdate = false;

    if (m_videoShaderPath.empty())
      return false;

    DisposeVideoShaders();

    static const auto shaderFormat = "hlsl";  // "hlsl" or "glsl" - Windows uses HLSL shaders
    auto presetPath = URIUtils::AddFileToFolder("libretro", shaderFormat, m_videoShaderPath);

    if(!m_pPreset)
      m_pPreset.reset(new SHADERPRESET::CVideoShaderPreset());

    // TODO: Maybe we don't need this?
    if (!m_pPreset->Init())
      return false;

    if (!m_pPreset->ReadPresetFile(presetPath))
    {
      CLog::Log(LOGERROR, "%s - couldn't load shader preset %s or the shaders it references", __FUNCTION__, presetPath);
      return false;
    }

    if (!CreateShaders())
    {
      CLog::Log(LOGWARNING, __FUNCTION__": VideoShaders: failed to initialize shaders. Disabling video shaders.");
      DisposeVideoShaders();
      return false;
    }

    if (!CreateLayouts())
    {
      CLog::Log(LOGWARNING, __FUNCTION__": VideoShaders: failed to create layouts. Disabling video shaders.");
      DisposeVideoShaders();
      return false;
    }

    if (!CreateShaderTextures())
    {
      CLog::Log(LOGWARNING, __FUNCTION__": VideoShaders: a shader texture failed to init. Disabling video shaders.");
      DisposeVideoShaders();
      return false;
    }

    if (!CreateBuffers())
    {
      CLog::Log(LOGWARNING, __FUNCTION__": VideoShaders: failed to initialize buffers. Disabling video shaders.");
      DisposeVideoShaders();
      return false;
    }

    if (!CreateSamplers())
    {
      CLog::Log(LOGWARNING, __FUNCTION__": VideoShaders: failed to create samplers. Disabling video shaders.");
      DisposeVideoShaders();
      return false;
    }
  }

  if (m_pVideoShaders.size() > 0 || m_pShaderTextures.size() > 0)
  {
    // Each pass must have its own texture and the opposite is also true
    if (m_pVideoShaders.size() != m_pShaderTextures.size())
    {
      CLog::Log(LOGWARNING, __FUNCTION__": VideoShaders: a shader or texture failed to init. Disabling video shaders.");
      DisposeVideoShaders();
      return false;
    }
  }
  return true;
}

bool CVideoShaderManager::CreateShaderTextures()
{
  m_pShaderTextures.clear();

  auto numPasses = std::min(m_pPreset->m_Passes, static_cast<unsigned>(GFX_MAX_SHADERS));
  auto backBuffer = g_Windowing.GetBackBuffer();

  for (unsigned shaderIdx = 0; shaderIdx < numPasses; ++shaderIdx) {
    auto& pass = m_pPreset->m_Pass[shaderIdx];

    // For reach pass, create the texture
    std::unique_ptr<CD3DTexture> texture(new CD3DTexture());

    auto res = texture->Create(backBuffer->GetWidth(), backBuffer->GetHeight(), 1,
      D3D11_USAGE_DEFAULT, DXGI_FORMAT_B8G8R8A8_UNORM, nullptr, 0);
    if (!res)
    {
      CLog::Log(LOGERROR, "Couldn't create a texture for video shader %s.", pass.source.path);
      return false;
    }
    m_pShaderTextures.push_back(std::move(texture));
  }
  return true;
}

bool CVideoShaderManager::CreateShaders()
{
  auto numPasses = std::min(m_pPreset->m_Passes, static_cast<unsigned>(GFX_MAX_SHADERS));
  auto numParameters = std::min(m_pPreset->m_NumParameters, static_cast<unsigned>(GFX_MAX_PARAMETERS));
  m_textureSize = GetOptimalTextureSize(m_videoSize);
  // TODO: bad!
  auto presetDirectory = URIUtils::AddFileToFolder("special://xbmcbinaddons/game.shader.presets/libretro/hlsl", m_videoShaderPath);

  // todo: is this pass specific?
  std::vector<ShaderLUT> passLUTs;
  for(unsigned i = 0; i < m_pPreset->m_Luts; ++i)
  {
    video_shader_lut_& lutStruct = m_pPreset->m_Lut[i];

    ID3D11SamplerState* lutSampler(CreateLUTSampler(lutStruct));
    CDXTexture* lutTexture(CreateLUTexture(lutStruct, presetDirectory));

    if (!lutSampler || !lutTexture)
    {
      delete lutSampler;
      delete lutTexture;
    }
    passLUTs.emplace_back(lutStruct.id, lutStruct.path, lutSampler, lutTexture);
  }

  for (unsigned shaderIdx = 0; shaderIdx < numPasses; ++shaderIdx) {
    auto& pass = m_pPreset->m_Pass[shaderIdx];

    // For reach pass, create the shader
    std::unique_ptr<CVideoShader> videoShader(new CVideoShader());

    auto shaderSrc = pass.source.string.vertex; // also contains fragment source
    auto shaderPath = pass.source.path;

    // Get only the parameters belonging to this specific shader
    ShaderParameters passParameters = GetShaderParameters(m_pPreset->m_Parameters, numParameters, pass.source.string.vertex);
    ID3D11SamplerState* passSampler = pass.filter ? m_pSampLinear : m_pSampNearest;

    if (!videoShader->Create(shaderSrc, shaderPath, passParameters, passSampler, std::move(passLUTs)))
    {
      CLog::Log(LOGERROR, "Couldn't create a video shader");
      return false;
    }
    m_pVideoShaders.push_back(std::move(videoShader));
  }
  return true;
}

bool CVideoShaderManager::CreateSamplers()
{
  // Describe the Sampler States
  // As specified in the common-shaders spec
  D3D11_SAMPLER_DESC sampDesc;
  ZeroMemory(&sampDesc, sizeof(D3D11_SAMPLER_DESC));
  sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
  sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
  sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
  sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
  sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
  sampDesc.MinLOD = 0;
  sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
  FLOAT blackBorder[4] = { 1, 0, 0, 1 };  // TODO: turn this back to black
  memcpy(sampDesc.BorderColor, &blackBorder, 4 * sizeof(FLOAT));

  if (FAILED(g_Windowing.Get3D11Device()->CreateSamplerState(&sampDesc, &m_pSampNearest)))
    return false;

  D3D11_SAMPLER_DESC sampDescLinear = sampDesc;
  sampDescLinear.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
  if (FAILED(g_Windowing.Get3D11Device()->CreateSamplerState(&sampDescLinear, &m_pSampLinear)))
    return false;

  return true;
}

bool CVideoShaderManager::CreateLayouts()
{
  for (auto& videoShader : m_pVideoShaders)
  {
    videoShader->CreateVertexBuffer(4, sizeof(CUSTOMVERTEX));
    // Create input layout
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
      { "SV_POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      { "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT,    0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    if (!videoShader->CreateInputLayout(layout, ARRAYSIZE(layout)))
    {
      CLog::Log(LOGERROR, __FUNCTION__": Failed to create input layout for Input Assembler.");
      return false;
    }
  }
  return true;
}

bool CVideoShaderManager::CreateBuffers()
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

ShaderParameters CVideoShaderManager::GetShaderParameters(video_shader_parameter_* parameters,
   unsigned numParameters, const std::string& sourceStr) const
{
   std::regex rgx("#pragma parameter ([a-zA-Z_][a-zA-Z0-9_]{0,31})");
   std::smatch matches;

   std::vector<std::string> validParams;
   std::string::const_iterator searchStart(sourceStr.cbegin());
   while (regex_search(searchStart, sourceStr.cend(), matches, rgx))
   {
      validParams.push_back(matches[1].str());
      searchStart += matches.position() + matches.length();
   }

   ShaderParameters matchParams;
   for (const auto& match : validParams)   // for each param found in the source code
      for (unsigned i = 0; i < numParameters; ++i)  // for each param found in the preset file
         if (match == parameters[i].id)  // if they match
         {
            // The add-on has already handled parsing and overwriting default
            // parameter values from the preset file. The final value we
            // should use is in the 'current' field.
            matchParams[match] = parameters[i].current;
            break;
         }

   return matchParams;
}


void CVideoShaderManager::PrepareParameters(CD3DTexture& texture, CRect sourceRect, CPoint dest[])
{
  if (m_sourceRect != sourceRect
    || m_dest[0] != dest[0] || m_dest[1] != dest[1]
    || m_dest[2] != dest[2] || m_dest[3] != dest[3]
    || texture.GetWidth() != m_outputSize.x
    || texture.GetHeight() != m_outputSize.y)
  {
    m_sourceRect = sourceRect;
    for (size_t i = 0; i < 4; ++i)
      m_dest[i] = dest[i];
    m_outputSize = { texture.GetWidth(), texture.GetHeight() };

    for (auto& videoShader : m_pVideoShaders)
    {
      CUSTOMVERTEX* v;
      videoShader->LockVertexBuffer(reinterpret_cast<void**>(&v));
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
      videoShader->UnlockVertexBuffer();
    }

    // Update projection matrix and input cbuffer
    CRect viewPort;
    g_Windowing.GetViewPort(viewPort);
    SetViewPort(viewPort);
  }
  UpdateInputBuffer();
}

void CVideoShaderManager::UpdateInputBuffer()
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

CVideoShaderManager::cbInput CVideoShaderManager::GetInputData()
{
  cbInput input = {
    { m_videoSize },   // video_size
    { m_textureSize }, // texture_size
    { m_outputSize },  // output_size
    { m_frameCount },             // TODO: frame_count
    { 1 }              // frame_direction
  };
  return input;
}

void CVideoShaderManager::DisposeVideoShaders()
{
  m_pVideoShaders.clear();
  m_pShaderTextures.clear();
}

CD3DTexture* CVideoShaderManager::GetFirstTexture()
{
  if (!m_pShaderTextures.empty())
    return m_pShaderTextures.front().get();
  return nullptr;
}

bool CVideoShaderManager::SetShaderPreset(const std::string shaderPresetPath)
{
  m_videoShaderPath = shaderPresetPath;
  m_bPresetNeedsUpdate = true;
  return Update();
}

void CVideoShaderManager::SetViewPort(const CRect& viewPort)
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

void CVideoShaderManager::SetCommonShaderParams(CVideoShader& shader, CD3DTexture& texture)
{
  CD3DEffect& effect = shader.GetEffect();

  effect.SetTechnique("TEQ");
  effect.SetResources("decal", { texture.GetAddressOfSRV() }, 1);
  // TODO: (Optimization): Add frame_count to separate cbuffer
  effect.SetConstantBuffer("input", m_pInputBuffer);
  effect.SetMatrix("modelViewProj", &m_MVP);
}

void CVideoShaderManager::UpdateViewPort()
{
  CRect viewPort;
  g_Windowing.GetViewPort(viewPort);

  float2 currentViewPortSize = { viewPort.Width(), viewPort.Height() };
  if (currentViewPortSize != m_viewPortSize)
  {
    m_viewPortSize = currentViewPortSize;
    CreateShaderTextures();
  }
}
