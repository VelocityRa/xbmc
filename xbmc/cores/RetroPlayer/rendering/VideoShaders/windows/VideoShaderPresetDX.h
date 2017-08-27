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
#pragma once

#include <memory>
#include <vector>

#include "VideoShaderDX.h"
#include "addons/binary-addons/AddonInstanceHandler.h"
#include "cores/RetroPlayer/IVideoShaderPreset.h"
#include "cores/RetroPlayer/rendering/VideoShaders/VideoShaderPresetFactory.h"
#include "cores/RetroPlayer/rendering/VideoShaders/VideoShaderUtils.h"
#include "games/GameServices.h"
#include "guilib/Geometry.h"

namespace ADDON
{
  class CShaderPreset;
  class CShaderPresetAddon;
}

using namespace KODI;
using namespace SHADER;

namespace SHADERPRESET
{
class CVideoShaderPresetDX : public IVideoShaderPreset
{
public:
  // Instance of CShaderPreset
  explicit CVideoShaderPresetDX(unsigned videoWidth = 0, unsigned videoHeight = 0);
  ~CVideoShaderPresetDX() override;

  /*!
  * \brief Reads/Parses a shader preset file and loads its state to the
  *        object. What this state is is implementation specific.
  * \param presetPath Full path of the preset file.
  * \return True on successful parsing, false on failed.
  */
  // todo: impl. once and for all
  bool ReadPresetFile(const std::string &presetPath) override
  {
    return CServiceBroker::GetGameServices().VideoShaders().LoadPreset(presetPath, *this);
  }

  VideoShaderPasses& GetPasses() override { return m_passes; }

  bool Update();
  bool SetShaderPreset(const std::string& shaderPresetPath) override;
  const std::string& GetShaderPreset() const override;
  void SetVideoSize(const unsigned videoWidth, const unsigned videoHeight) override;
  bool RenderUpdate(CPoint dest[], IShaderTexture& source, IShaderTexture& target) override;

  //CShaderTextureDX* GetFirstTexture();

  void SetSpeed(double speed) override { m_speed = speed; }

private:
  bool CreateShaderTextures();
  bool CreateShaders();
  bool CreateSamplers();
  bool CreateLayouts();
  bool CreateBuffers();
  void UpdateViewPort();
  void UpdateMVPs();
  void DisposeVideoShaders();
  void PrepareParameters(IShaderTexture& texture, CPoint dest[]);
  static void RenderShader(IVideoShader& shader, IShaderTexture& source, IShaderTexture& target);
  bool HasPathFailed(const std::string& path);

  // Relative path of the currently loaded shader preset
  std::string m_presetPath;

  // VideoShaders for the shader passes
  std::vector<std::unique_ptr<IVideoShader>> m_pVideoShaders;

  // Intermediate textures used for pixel shader passes
  std::vector<std::unique_ptr<CShaderTextureCD3D>> m_pShaderTextures;

  // First texture (this won't be needed when we have RGB rendering
  std::unique_ptr<CShaderTextureCD3D> firstTexture;

  // Was the shader preset changed during the last frame?
  bool m_bPresetNeedsUpdate = true;

  // Size of the viewport
  float2 m_outputSize;

  // The size of the input texture itself
  // Power-of-two sized.
  float2 m_textureSize;

  // Size of the actual source video data (ie. 160x144 for the Game Boy)
  float2 m_videoSize;

  // Number of frames that have passed
  float m_frameCount = 0.0f;

  // Point/nearest neighbor sampler
  ID3D11SamplerState* m_pSampNearest = nullptr;

  // Linear sampler
  ID3D11SamplerState* m_pSampLinear = nullptr;

  // Set of paths of presets that are known to not load correctly
  std::set<std::string> m_failedPaths;

  // Array of vertices that comprise the full viewport
  CPoint m_dest[4];

  // Playback speed
  double m_speed = 0.0;

  ShaderParameters GetShaderParameters(const std::vector<VideoShaderParameter> &parameters, const std::string& sourceStr) const;

  VideoShaderPasses m_passes;
};
} // namespace SHADERPRESET
