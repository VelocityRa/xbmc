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

#include "cores/IPlayer.h"

namespace KODI
{
namespace RETRO
{
  class CGUIRenderSettings
  {
  public:
    CGUIRenderSettings() { Reset(); }

    void Reset();

    bool operator==(const CGUIRenderSettings &rhs) const;

    std::string GetVideoFilter() const;
    bool HasVideoFilter() const { return !m_videoFilter.empty(); }
    void SetVideoFilter(const std::string &videoFilter) { m_videoFilter = videoFilter; }
    void ResetVideoFilter() { m_videoFilter.clear(); }

    ViewMode GetRenderViewMode() const;
    bool HasRenderViewMode() const { return m_viewMode != -1; }
    void SetRenderViewMode(ViewMode mode) { m_viewMode = static_cast<int>(mode); }
    void ResetRenderViewMode() { m_viewMode = -1; }

  private:
    std::string m_videoFilter;
    int m_viewMode;
  };
}
}
