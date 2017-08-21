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

#include <regex>
#include "cores/RetroPlayer/RetroPlayer.h"
#include "settings/DisplaySettings.h"

using namespace KODI;
using namespace SHADER;

CVideoShaderManager::CVideoShaderManager(CBaseRenderer& rendererRef, unsigned videoWidth, unsigned videoHeight)
  : m_bPresetNeedsUpdate(true)
  , m_textureSize()
  , m_videoSize(videoWidth, videoHeight)
  , m_frameCount(0)
  , m_pSampNearest(nullptr)
  , m_pSampLinear(nullptr)
{
  m_videoSize = { videoWidth, videoHeight };

  CRect viewPort;
  g_Windowing.GetViewPort(viewPort);
  m_outputSize = float2(viewPort.Width(), viewPort.Height());
}

CVideoShaderManager::~CVideoShaderManager()
{
  DisposeVideoShaders();
  // The gui is going to render after this, so apply the state required
  g_Windowing.ApplyStateBlock();
}

bool CVideoShaderManager::RenderUpdate(CPoint dest[], CD3DTexture& source, CD3DTexture& target)
{
  // Handle resizing of the viewport (window)
  UpdateViewPort();

  // Update shaders/shader textures if required
  if (!Update())
    return false;

  PrepareParameters(target, dest);

  // At this point, the input video has been rendered to the first texture ("source", not m_pShaderTextures[0])

  unsigned passesNum = m_pShaderTextures.size();

  CVideoShader& firstShader = *m_pVideoShaders.front();
  CD3DTexture& firstShaderTexture = *m_pShaderTextures.front();
  CVideoShader& lastShader = *m_pVideoShaders.back();

  if (passesNum == 1)
    m_pVideoShaders.front()->Render(source, target);
  else if (passesNum == 2)
  {
    // Apply first pass
    firstShader.Render(source, firstShaderTexture);
    // Apply last pass
    lastShader.Render(firstShaderTexture, target);
  }
  else
  {
    // Apply first pass
    firstShader.Render(source, firstShaderTexture);

    // Apply all passes except the first and last one (which needs to be applied to the backbuffer)
    for (unsigned shaderIdx = 1; shaderIdx < m_pVideoShaders.size() - 1; ++shaderIdx)
    {
      CVideoShader& shader = *m_pVideoShaders[shaderIdx];
      CD3DTexture& prevTexture = *m_pShaderTextures[shaderIdx - 1];
      CD3DTexture& texture = *m_pShaderTextures[shaderIdx];
      shader.Render(prevTexture, texture);
    }

    // Apply last pass and write to target (backbuffer) instead of the last texture
    CD3DTexture& secToLastTexture = *m_pShaderTextures[m_pShaderTextures.size() - 2];
    lastShader.Render(secToLastTexture, target);
  }

  m_frameCount += m_speed;

  // Restore our view port.
  g_Windowing.RestoreViewPort();

  return true;
}

bool CVideoShaderManager::Update()
{
  if (m_bPresetNeedsUpdate)
  {
    if (m_videoShaderPath.empty())
      return false;

    DisposeVideoShaders();

    if(!m_pPreset)
      m_pPreset.reset(new SHADERPRESET::CVideoShaderPreset());

    // TODO: Maybe we don't need this?
    if (!m_pPreset->Init())
      return false;

    if (!m_pPreset->ReadPresetFile(m_videoShaderPath))
    {
      CLog::Log(LOGERROR, "%s - couldn't load shader preset %s or the shaders it references", __FUNCTION__, m_videoShaderPath.c_str());
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

    if (!CreateBuffers())
    {
      CLog::Log(LOGWARNING, __FUNCTION__": VideoShaders: failed to initialize buffers. Disabling video shaders.");
      DisposeVideoShaders();
      return false;
    }

    if (!CreateShaderTextures())
    {
      CLog::Log(LOGWARNING, __FUNCTION__": VideoShaders: a shader texture failed to init. Disabling video shaders.");
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

  if (m_pVideoShaders.size() == 0)
    return false;

  // Each pass must have its own texture and the opposite is also true
  if (m_pVideoShaders.size() != m_pShaderTextures.size())
  {
    CLog::Log(LOGWARNING, __FUNCTION__": VideoShaders: a shader or texture failed to init. Disabling video shaders.");
    DisposeVideoShaders();
    return false;
  }

  m_bPresetNeedsUpdate = false;
  return true;
}

bool CVideoShaderManager::CreateShaderTextures()
{
  m_pShaderTextures.clear();

  firstTexture.reset(new CD3DTexture());
  auto res = firstTexture->Create(m_outputSize.x, m_outputSize.y, 1,
    D3D11_USAGE_DEFAULT, DXGI_FORMAT_B8G8R8A8_UNORM, nullptr, 0);
  if (!res)
  {
    CLog::Log(LOGERROR, "Couldn't create a intial video shader texture");
    return false;
  }

  float2 prevSize = m_videoSize;

  auto numPasses = m_pPreset->Preset().passes.size();

  for (unsigned shaderIdx = 0; shaderIdx < numPasses; ++shaderIdx) {
    auto& pass = m_pPreset->Preset().passes[shaderIdx];

    // resolve final texture resolution, taking scale type and scale multiplier into account

    UINT textureX;
    UINT textureY;
    switch (pass.fbo.scaleX.type)
    {
    case SCALE_TYPE_ABSOLUTE:
      textureX = pass.fbo.scaleX.abs;
      break;
    case SCALE_TYPE_VIEWPORT:
      textureX = m_outputSize.x;
      break;
    case SCALE_TYPE_INPUT:
    default:
      textureX = prevSize.x;
      break;
    }
    switch (pass.fbo.scaleY.type)
    {
    case SCALE_TYPE_ABSOLUTE:
      textureY = pass.fbo.scaleY.abs;
      break;
    case SCALE_TYPE_VIEWPORT:
      textureY = m_outputSize.y;
      break;
    case SCALE_TYPE_INPUT:
    default:
      textureY = prevSize.y;
      break;
    }

    // if the scale was unspecified
    if (pass.fbo.scaleX.scale == 0 && pass.fbo.scaleY.scale == 0)
    {
      // if the last shader has the scale unspecified
      if (shaderIdx == numPasses - 1)
      {
        // we're supposed to output at full (viewport) res
        // TODO: Thus, we can also (maybe) bypass rendering to an intermediate render target (on the last pass)
        textureX = m_outputSize.x;
        textureY = m_outputSize.y;
      }
    }
    else
    {
      textureX *= pass.fbo.scaleX.scale;
      textureY *= pass.fbo.scaleY.scale;
    }

    // For reach pass, create the texture
    std::unique_ptr<CD3DTexture> texture(new CD3DTexture());

    // Determine the framebuffer data format
    DXGI_FORMAT textureFormat;
    if (pass.fbo.floatFramebuffer)
      // Give priority to float framebuffer parameter (we can't use both float and sRGB)
      textureFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;
    else
      if (pass.fbo.sRgbFramebuffer)
        textureFormat = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
      else
        textureFormat = DXGI_FORMAT_B8G8R8A8_UNORM;

    if (!texture->Create(textureX, textureY, 1, D3D11_USAGE_DEFAULT, textureFormat, nullptr, 0))
    {
      CLog::Log(LOGERROR, "Couldn't create a texture for video shader %s.", pass.sourcePath.c_str());
      return false;
    }
    m_pShaderTextures.push_back(std::move(texture));

    // notify shader of its source and dest size
    m_pVideoShaders[shaderIdx]->SetSizes(prevSize, float2(textureX, textureY));

    prevSize = { textureX, textureY };
  }
  return true;
}

bool CVideoShaderManager::CreateShaders()
{
  auto numPasses = m_pPreset->Preset().passes.size();
  auto numParameters = m_pPreset->Preset().parameters.size();
  m_textureSize = GetOptimalTextureSize(m_videoSize); // todo: replace with per-shader texture size

  // todo: is this pass specific?
  ShaderLUTs passLUTs;
  for(unsigned i = 0; i < m_pPreset->Preset().luts.size(); ++i)
  {
    const VideoShaderLut& lutStruct = m_pPreset->Preset().luts[i];

    ID3D11SamplerState* lutSampler(CreateLUTSampler(lutStruct));
    CDXTexture* lutTexture(CreateLUTexture(lutStruct, m_videoShaderPath));

    if (!lutSampler || !lutTexture)
    {
      CLog::Log(LOGWARNING, "%s - Couldn't create a LUT sampler or texture for LUT %s", __FUNCTION__, lutStruct.strId);
      delete lutSampler;
      delete lutTexture;
    }
    else
      passLUTs.emplace_back(new ShaderLUT(lutStruct.strId, lutStruct.path, lutSampler, lutTexture));
  }

  for (unsigned shaderIdx = 0; shaderIdx < numPasses; ++shaderIdx) {
    const auto& pass = m_pPreset->Preset().passes[shaderIdx];

    // For reach pass, create the shader
    std::unique_ptr<CVideoShader> videoShader(new CVideoShader());

    auto shaderSrc = pass.vertexSource; // also contains fragment source
    auto shaderPath = pass.sourcePath;

    // Get only the parameters belonging to this specific shader
    ShaderParameters passParameters = GetShaderParameters(m_pPreset->Preset().parameters, pass.vertexSource);
    ID3D11SamplerState* passSampler = pass.filter ? m_pSampLinear : m_pSampNearest;

    if (!videoShader->Create(shaderSrc, shaderPath, passParameters, passSampler, passLUTs, m_outputSize, pass.frameCountMod))
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
      { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      { "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 }
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
  for (auto& videoShader : m_pVideoShaders)
    videoShader->CreateInputBuffer();
  return true;
}

ShaderParameters CVideoShaderManager::GetShaderParameters(const std::vector<VideoShaderParameter> &parameters, const std::string& sourceStr) const
{
   static const std::regex pragmaParamRegex("#pragma parameter ([a-zA-Z_][a-zA-Z0-9_]*)");
   std::smatch matches;

   std::vector<std::string> validParams;
   std::string::const_iterator searchStart(sourceStr.cbegin());
   while (regex_search(searchStart, sourceStr.cend(), matches, pragmaParamRegex))
   {
      validParams.push_back(matches[1].str());
      searchStart += matches.position() + matches.length();
   }

   ShaderParameters matchParams;
   for (const auto& match : validParams)   // for each param found in the source code
      for (unsigned i = 0; i < parameters.size(); ++i)  // for each param found in the preset file
         if (match == parameters[i].strId)  // if they match
         {
            // The add-on has already handled parsing and overwriting default
            // parameter values from the preset file. The final value we
            // should use is in the 'current' field.
            matchParams[match] = parameters[i].current;
            break;
         }

   return matchParams;
}

void CVideoShaderManager::PrepareParameters(CD3DTexture& texture, CPoint dest[])
{
  // prepare params for all shaders except the last (needs special flag)
  for (unsigned shaderIdx = 0; shaderIdx < m_pVideoShaders.size() - 1; ++shaderIdx)
  {
    auto& videoShader = m_pVideoShaders[shaderIdx];
    videoShader->PrepareParameters(m_dest, false, m_frameCount);
  }
  // prepare params for last shader
  m_pVideoShaders.back()->PrepareParameters(m_dest, true, m_frameCount);

  if (m_dest[0] != dest[0] || m_dest[1] != dest[1]
    || m_dest[2] != dest[2] || m_dest[3] != dest[3]
    || texture.GetWidth() != m_outputSize.x
    || texture.GetHeight() != m_outputSize.y)
  {
    for (size_t i = 0; i < 4; ++i)
      m_dest[i] = dest[i];
    m_outputSize = { texture.GetWidth(), texture.GetHeight() };

    // Update projection matrix and update video shaders
    UpdateMVPs();
    UpdateViewPort();
  }
}

void CVideoShaderManager::DisposeVideoShaders()
{
  m_pVideoShaders.clear();
  m_pShaderTextures.clear();
  firstTexture.reset();
  m_bPresetNeedsUpdate = true;
}

CD3DTexture* CVideoShaderManager::GetFirstTexture()
{
  return firstTexture.get();
}

bool CVideoShaderManager::SetShaderPreset(const std::string shaderPresetPath)
{
  m_videoShaderPath = shaderPresetPath;
  m_bPresetNeedsUpdate = true;
  return Update();
}

void CVideoShaderManager::UpdateMVPs()
{
  for(auto& videoShader : m_pVideoShaders)
    videoShader->UpdateMVP();
}

void CVideoShaderManager::UpdateViewPort()
{
  CRect viewPort;
  g_Windowing.GetViewPort(viewPort);

  float2 currentViewPortSize = { viewPort.Width(), viewPort.Height() };
  if (currentViewPortSize != m_outputSize)
  {
    m_outputSize = currentViewPortSize;
    //CreateShaderTextures();
    // Just re-make everything. Else we get resizing bugs.
    // Could probably refine that to only rebuild certain things, for a tiny bit of perf. (only when resizing)
    m_bPresetNeedsUpdate = true;
    Update();
  }
}
