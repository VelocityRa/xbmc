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

#include "RPBaseRenderer.h"

#include <atomic>
#include <memory>
#include <stdint.h>
#include <vector>

#include "cores/RetroPlayer/rendering/VideoShaders/windows/VideoShaderTextureDX.h"

struct SwsContext;
class CD3DTexture;
class COutputShader;

namespace KODI
{
namespace RETRO
{
  class CRPWinRenderer : public CRPBaseRenderer
  {
  public:
    static CRPBaseRenderer* Create();
    static void Register();

    CRPWinRenderer();
    ~CRPWinRenderer() override;

    // implementation of CRPBaseRenderer
    bool Configure(AVPixelFormat format, unsigned int width, unsigned int height, unsigned int orientation) override;
    void AddFrame(const uint8_t* data, unsigned int size) override;
    void Flush() override;
    void RenderUpdate(unsigned int alpha) override;
    void Deinitialize() override;
    bool Supports(ERENDERFEATURE feature) const override;
    bool Supports(ESCALINGMETHOD method) const override;
    void SetSpeed(double speed) override;

  private:
    bool UploadTexture();
    void Render(CD3DTexture* target);

    bool m_bConfigured = false;
    bool m_bLoaded = false;

    AVPixelFormat m_targetPixFormat = AV_PIX_FMT_NONE;
    DXGI_FORMAT m_targetDxFormat = DXGI_FORMAT_UNKNOWN;
    std::vector<uint8_t> m_buffer;
    std::atomic<bool> m_bQueued;
    std::unique_ptr<SHADER::CShaderTextureCD3D> m_intermediateTarget;
    SwsContext* m_swsContext = nullptr;
    std::unique_ptr<COutputShader> m_outputShader;

    SHADER::CShaderTextureCD3D m_targetTexture;
  };
}
}
