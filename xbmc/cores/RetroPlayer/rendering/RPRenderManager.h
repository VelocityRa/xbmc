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

#include "IRenderSettingsCallback.h"
#include "libavutil/pixfmt.h"
#include "threads/CriticalSection.h"

namespace KODI
{
namespace RETRO
{
  class CRPBaseRenderer;

  class CRPRenderManager : public IRenderSettingsCallback
  {
  public:
    CRPRenderManager() = default;
    ~CRPRenderManager() override = default;

    void Initialize();
    void Deinitialize();

    // Functions called from game loop
    bool Configure(AVPixelFormat format, unsigned int width, unsigned int height, unsigned int orientation);
    bool AddFrame(const uint8_t* data, unsigned int size);
    void SetSpeed(double speed);

    // Functions called from render thread
    bool IsConfigured() const;
    void FrameMove();
    void Render(bool clear, DWORD alpha, bool gui);
    void Flush();
    void TriggerUpdateResolution();

    // Implementation of IRenderSettingsCallback
    bool SupportsRenderFeature(ERENDERFEATURE feature) const override;
    bool SupportsScalingMethod(ESCALINGMETHOD method) const override;
    ESCALINGMETHOD GetScalingMethod() const override;
    void SetScalingMethod(ESCALINGMETHOD scalingMethod) override;
    ViewMode GetRenderViewMode() const override;
    void SetRenderViewMode(ViewMode mode) override;
    void SetShaderPreset(const std::string &shaderPresetPath) override;
    const std::string &GetShaderPreset() const override;

  private:
    void UpdateResolution();

    // Stream properties
    AVPixelFormat m_format = AV_PIX_FMT_NONE;
    unsigned int m_width = 0;
    unsigned int m_height = 0;
    unsigned int m_orientation = 0; // Degrees counter-clockwise

    // Render properties
    enum class RENDER_STATE
    {
      UNCONFIGURED,
      CONFIGURING,
      CONFIGURED,
    };
    RENDER_STATE m_state = RENDER_STATE::UNCONFIGURED;
    std::shared_ptr<CRPBaseRenderer> m_renderer;
    bool m_bTriggerUpdateResolution = false;
    CCriticalSection m_stateMutex;
  };
}
}
