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

#include "DialogGameVideoFilter.h"
#include "cores/RetroPlayer/rendering/IRenderSettingsCallback.h"
#include "guilib/LocalizeStrings.h"
#include "guilib/WindowIDs.h"
#include "settings/GameSettings.h"
#include "settings/MediaSettings.h"
#include "utils/Variant.h"
#include "FileItem.h"
#include "utils/XBMCTinyXML.h"
#include "utils/log.h"

#include <stdlib.h>

using namespace KODI;
using namespace GAME;

#define PRESETS_ADDON_NAME "game.shader.presets"

CDialogGameVideoFilter::CDialogGameVideoFilter() :
  CDialogGameVideoSelect(WINDOW_DIALOG_GAME_VIDEO_FILTER)
{
}

void CDialogGameVideoFilter::PreInit()
{
  m_videoFilters.clear();

  // TODO: Have the add-on give us the xml as a string (or parse it)
  static const std::string addonPath = std::string("special://xbmcbinaddons/") + PRESETS_ADDON_NAME;
  static const std::string xmlPath = addonPath + "/resources/ShaderPresetsDefault.xml";

  CXBMCTinyXML xml = CXBMCTinyXML(xmlPath);

  if (!xml.LoadFile())
  {
    CLog::Log(LOGERROR, "%s - Couldn't load shader presets default .xml, %s", __FUNCTION__, xmlPath);
    return;
  }

  auto root = xml.RootElement();
  TiXmlNode* child = nullptr;

  while (child = root->IterateChildren(child))
  {
    VideoFilterProperties videoFilter;

    videoFilter.path = child->FirstChild("path")->FirstChild()->Value();
    videoFilter.nameIndex = atoi(child->FirstChild("name")->FirstChild()->Value());
    videoFilter.categoryIndex = atoi(child->FirstChild("category")->FirstChild()->Value());
    videoFilter.descriptionIndex = atoi(child->FirstChild("description")->FirstChild()->Value());

    m_videoFilters.emplace_back(videoFilter);
  }

  CLog::Log(LOGDEBUG, "%s - Loaded shader presets default .xml, %s", __FUNCTION__, xmlPath);
}

void CDialogGameVideoFilter::GetItems(CFileItemList &items)
{
  for (const auto &videoFilter : m_videoFilters)
  {
    //CFileItemPtr item = std::make_shared<CFileItem>(g_localizeStrings.Get(videoFilter.stringIndex));
    //item->SetProperty("game.videofilter", CVariant{ videoFilter.scalingMethod });

    auto localizedName = GetLocalizedString(videoFilter.nameIndex);
    auto localizedCategory = GetLocalizedString(videoFilter.categoryIndex);

    CFileItemPtr item = std::make_shared<CFileItem>(localizedName);
    item->SetLabel2(localizedCategory);

    items.Add(std::move(item));
  }

  if (items.Size() == 0)
  {
    CFileItemPtr item = std::make_shared<CFileItem>(g_localizeStrings.Get(16316)); // "Auto"
    items.Add(std::move(item));
  }
}

void CDialogGameVideoFilter::OnItemFocus(unsigned int index)
{
  if (index < m_videoFilters.size() && m_callback != nullptr)
  {
    const std::string &presetToSet = m_videoFilters[index].path;
    bool presetChanged = (m_shaderPresetPath != presetToSet);

    if (presetChanged)
    {
      m_callback->SetShaderPreset(presetToSet);
      m_shaderPresetPath = presetToSet;
    }
  }
}

unsigned int CDialogGameVideoFilter::GetFocusedItem() const
{
  CGameSettings &gameSettings = CMediaSettings::GetInstance().GetCurrentGameSettings();

  for (unsigned int i = 0; i < m_videoFilters.size(); i++)
  {
    //const ESCALINGMETHOD scalingMethod = m_videoFilters[i].scalingMethod;
    //if (scalingMethod == gameSettings.ScalingMethod())
    //  return i;
  }

  return 0;
}

void CDialogGameVideoFilter::PostExit()
{
  // stuff
  m_videoFilters.clear();
}

std::string CDialogGameVideoFilter::GetLocalizedString(uint32_t code)
{
  return g_localizeStrings.GetAddonString(PRESETS_ADDON_NAME, code);
}