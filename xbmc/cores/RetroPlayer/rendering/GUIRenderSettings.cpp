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

#include "GUIRenderSettings.h"
#include "settings/GameSettings.h"
#include "settings/MediaSettings.h"

using namespace KODI;
using namespace RETRO;

void CGUIRenderSettings::Reset()
{
  m_videoFilter.clear();
  m_viewMode = -1;
}

bool CGUIRenderSettings::operator==(const CGUIRenderSettings &rhs) const
{
  return m_videoFilter == rhs.m_videoFilter &&
         m_viewMode == rhs.m_viewMode;
}

std::string CGUIRenderSettings::GetVideoFilter() const
{
  if (HasVideoFilter())
    return m_videoFilter;

  CGameSettings &gameSettings = CMediaSettings::GetInstance().GetCurrentGameSettings();
  return gameSettings.VideoFilter();
}

ViewMode CGUIRenderSettings::GetRenderViewMode() const
{
  if (HasRenderViewMode())
    return static_cast<ViewMode>(m_viewMode);
  
  CGameSettings &gameSettings = CMediaSettings::GetInstance().GetCurrentGameSettings();
  return gameSettings.ViewMode();
}
