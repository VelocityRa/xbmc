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


#include <memory>
#include "cores/RetroPlayer/rendering/VideoShaderUtils.h"
#include "cores/RetroPlayer/VideoShaderPreset.h"
#include "cores/VideoPlayer/VideoRenderers/BaseRenderer.h"
#include "guilib/Texture.h"

using namespace SHADER;

class CVideoShader;
struct video_shader_parameter_;

// TODO: renderer independence
class CVideoShaderManager
{
public:
  CVideoShaderManager() = delete;
  CVideoShaderManager(CBaseRenderer& rendererRef, unsigned videoWidth = 0, unsigned videoHeight = 0);
  ~CVideoShaderManager();

  bool Update();
  bool SetShaderPreset(const std::string shaderPresetPath);
  bool RenderUpdate(CPoint dest[], CD3DTexture& source, CD3DTexture& target);

  CD3DTexture* GetFirstTexture();

  void SetSpeed(double speed) { m_speed = speed; }

private:
  bool CreateShaderTextures();
  ShaderParameters GetShaderParameters(video_shader_parameter_* parameters,
    unsigned numParameters, const std::string& sourceStr) const;
  bool CreateShaders();
  //bool CreateIntermediateShader();
  bool CreateSamplers();
  bool CreateLayouts();
  bool CreateBuffers();
  void UpdateViewPort();
  void UpdateMVPs();
  void DisposeVideoShaders();

  void PrepareParameters(CD3DTexture& texture, CPoint dest[]);
  
  // Loaded preset
  std::unique_ptr<SHADERPRESET::CVideoShaderPreset> m_pPreset;

  // Relative path of the currently loaded shader preset
  std::string m_videoShaderPath;

  // VideoShaders for the shader passes
  std::vector<std::unique_ptr<CVideoShader>> m_pVideoShaders;

  // Intermediate textures used for pixel shader passes
  std::vector<std::unique_ptr<CD3DTexture>> m_pShaderTextures;

  // First texture (this won't be needed when we have RGB rendering
  std::unique_ptr<CD3DTexture> firstTexture;

  // Was the shader preset changed during the last frame?
  bool m_bPresetNeedsUpdate;

  // Size of the viewport
  float2 m_outputSize;

  // The size of the input texture itself
  // Power-of-two sized.
  float2 m_textureSize;

  // Size of the actual source video data (ie. 160x144 for the Game Boy)
  float2 m_videoSize;

  // Number of frames that have passed
  float m_frameCount;

  // Point/nearest neighbor sampler
  ID3D11SamplerState* m_pSampNearest;

  // Linear sampler
  ID3D11SamplerState* m_pSampLinear;

  CRect m_sourceRect;
  CPoint m_dest[4];

  double m_speed = 0.0;
};
