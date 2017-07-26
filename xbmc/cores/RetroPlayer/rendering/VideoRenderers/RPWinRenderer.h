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

#include "VideoRenderers/WinRenderer.h"
#include "cores/RetroPlayer/rendering/VideoShaderManager.h"

class CRPWinRenderer : public CWinRenderer
{
  CRPWinRenderer();
  ~CRPWinRenderer();
public:
  static CBaseRenderer* Create(CVideoBuffer *buffer);

  //public:
//  bool IsPictureHW(const VideoPicture& picture) override;
//  bool IsGuiLayer() override;
//protected:
//  void ManageRenderArea() override;
//  void ReorderDrawPoints() override;
//  EShaderFormat GetShaderFormat() override;
//public:
//  ~CRPWinRenderer() override;
//  void Update() override;
//  bool RenderCapture(CRenderCapture* capture) override;
//  bool Configure(const VideoPicture& picture, float fps, unsigned flags, unsigned orientation) override;
//  void AddVideoPicture(const VideoPicture& picture, int index, double currentClock) override;
//  void FlipPage(int source) override;
//  void UnInit() override;
//  void Reset() override;
//  bool IsConfigured() override;
//  void Flush() override;
//  CRenderInfo GetRenderInfo() override;
//  void RenderUpdate(bool clear, unsigned flags, unsigned alpha) override;
//  void SetBufferSize(int numBuffers) override;
//  void ReleaseBuffer(int idx) override;
//  bool NeedBuffer(int idx) override;
//  bool SupportsMultiPassRendering() override;
//  bool Supports(ERENDERFEATURE feature) override;
//  bool Supports(ESCALINGMETHOD method) override;
//  bool WantsDoublePass() override;
//  bool ConfigChanged(const VideoPicture& picture) override;

protected:
  void Render(DWORD flags, CD3DTexture* target) override;

  void RenderPS(CD3DTexture* target);
  void RenderSW(CD3DTexture* target);

  // ====== Video Shader Members =====
private:
  // For true renderer independence, RPRenderManager must own this
  std::unique_ptr<CVideoShaderManager> m_shaderManager;
  /**
   * \brief Sets the shader preset path to be loaded used from the next frame
   * \param presetPath Relative path to preset file
   */
public:
  void SetShaderPreset(const std::string presetPath);

private:
  void UpdateVideoShaders();
  std::string m_videoShaderPath;
  bool m_shadersNeedUpdate;
  bool m_isShaderManagerReady;
};
