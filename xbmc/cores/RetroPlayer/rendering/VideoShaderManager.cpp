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

#include <regex>

using namespace SHADER;

CVideoShaderManager::CVideoShaderManager(unsigned videoWidth, unsigned videoHeight)
  : m_bPresetNeedsUpdate(true)
  , m_videoSize()
  , m_pSampNearest(nullptr)
  , m_pSampLinear(nullptr)
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
  // The gui is going to render after this, so apply the state required
  g_Windowing.ApplyStateBlock();
}

void CVideoShaderManager::Render(CRect sourceRect, CPoint dest[], CD3DTexture* target)
{
  // Update shaders/shader textures if required
  if (!Update())
    return;

  // Handle resizing of the viewport (window)
  UpdateViewPort();

  // At this point, the input video has been rendered to the first texture

  // Apply all passes except the last one (which needs to be applied to the backbuffer)
  for (unsigned shaderIdx = 0; shaderIdx < m_pVideoShaders.size() - 1; ++shaderIdx)
    m_pVideoShaders[shaderIdx]->Render(sourceRect, dest,
      m_pShaderTextures[shaderIdx].get(),
      m_pShaderTextures[shaderIdx + 1].get());

  // Apply last pass and write to target (backbuffer)
  m_pVideoShaders.back()->Render(sourceRect, dest, m_pShaderTextures.back().get(), target);

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

    if (!CreateSamplers())
      return false;

    if (!CreateShaderTextures())
      return false;

    if (!CreateShaders())
      return false;
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

  auto textureSize = GetOptimalTextureSize(m_videoSize);

  for (unsigned shaderIdx = 0; shaderIdx < numPasses; ++shaderIdx) {
    auto& pass = m_pPreset->m_Pass[shaderIdx];

    // For reach pass, create the shader
    std::unique_ptr<CVideoShader> videoShader(new CVideoShader());

    auto shaderSrc = pass.source.string.vertex; // also contains fragment source
    auto shaderPath = pass.source.path;

    // Get only the parameters belonging to this specific shader
    ShaderParameters passParameters = GetShaderParameters(m_pPreset->m_Parameters, numParameters, pass.source.string.vertex);
    ID3D11SamplerState* passSampler = pass.filter ? m_pSampLinear : m_pSampNearest;
    if (!videoShader->Create(shaderSrc, shaderPath, passParameters, passSampler, m_videoSize, textureSize))
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

ShaderParameters CVideoShaderManager::GetShaderParameters(video_shader_parameter_* parameters,
  unsigned numParameters, const std::string& sourceStr)
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

  ShaderParameters matchesParams;
  for (const auto& match : validParams)   // for each param found in the source code
    for (unsigned i = 0; i < numParameters; ++i)  // for each param found in the preset file
      if (match == parameters[i].id)  // if they match
      {
        // The add-on has already handled parsing and overwriting default
        // parameter values from the preset file. The final value we
        // should use is in the 'current' field.
        matchesParams[match] = parameters[i].current;
        break;
      }

  return matchesParams;
}

void CVideoShaderManager::DisposeVideoShaders()
{
  m_pVideoShaders.clear();
  m_pShaderTextures.clear();
  //m_pPreset.reset();
}

CD3DTexture* CVideoShaderManager::GetFirstTexture()
{
  if (!m_pShaderTextures.empty())
    return m_pShaderTextures.front().get();
  return nullptr;
}


float2 CVideoShaderManager::GetOptimalTextureSize(float2 videoSize)
{
  unsigned sizeMax = std::max(videoSize.x, videoSize.y);
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

void CVideoShaderManager::UpdateViewPort()
{
  CRect viewPort;
  g_Windowing.GetViewPort(viewPort);

  unsigned curWidth = static_cast<unsigned>(viewPort.Width());
  unsigned curHeight = static_cast<unsigned>(viewPort.Height());
  if (curWidth != m_viewPortSize.x || curHeight != m_viewPortSize.y)
  {
    m_viewPortSize.x = curWidth;
    m_viewPortSize.y = curHeight;

    CreateShaderTextures();
  }
}

bool CVideoShaderManager::SetShaderPreset(const std::string shaderPresetPath)
{
  m_videoShaderPath = shaderPresetPath;
  m_bPresetNeedsUpdate = true;
  return Update();
}

void CVideoShaderManager::SetVideoSize(unsigned videoWidth, unsigned videoHeight)
{
  m_videoSize.x = videoWidth;
  m_videoSize.y = videoHeight;
}
