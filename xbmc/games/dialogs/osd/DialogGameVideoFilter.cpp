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
#include "utils/log.h"
#include "utils/URIUtils.h"
#include "utils/Variant.h"
#include "utils/XBMCTinyXML.h"
#include "FileItem.h"
#include "URL.h"

#include <stdlib.h>
#include "ServiceBroker.h"
#include "games/GameServices.h"
#include "cores/RetroPlayer/rendering/VideoShaders/VideoShaderPresetFactory.h"

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
  std::string basePath = URIUtils::GetBasePath(xmlPath);

  CXBMCTinyXML xml = CXBMCTinyXML(xmlPath);

  if (!xml.LoadFile())
  {
    CLog::Log(LOGERROR, "%s - Couldn't load shader presets from default .xml, %s", __FUNCTION__, CURL::GetRedacted(xmlPath).c_str());
    return;
  }

  auto root = xml.RootElement();
  TiXmlNode* child = nullptr;

  while ((child = root->IterateChildren(child)))
  {
    VideoFilterProperties videoFilter;

    videoFilter.path = URIUtils::AddFileToFolder(basePath, child->FirstChild("path")->FirstChild()->Value());
    videoFilter.nameIndex = atoi(child->FirstChild("name")->FirstChild()->Value());
    videoFilter.categoryIndex = atoi(child->FirstChild("category")->FirstChild()->Value());
    videoFilter.descriptionIndex = atoi(child->FirstChild("description")->FirstChild()->Value());

    m_videoFilters.emplace_back(videoFilter);
  }

  CLog::Log(LOGDEBUG, "Loaded %d shader presets from default .xml, %s", m_videoFilters.size(), CURL::GetRedacted(xmlPath).c_str());
}

void CDialogGameVideoFilter::GetItems(CFileItemList &items)
{
  for (const auto &videoFilter : m_videoFilters)
  {
    bool canLoadPreset = CServiceBroker::GetGameServices().VideoShaders().CanLoadPreset(videoFilter.path);

    if (!canLoadPreset)
      continue;

    auto localizedName = GetLocalizedString(videoFilter.nameIndex);
    auto localizedCategory = GetLocalizedString(videoFilter.categoryIndex);

    CFileItemPtr item = std::make_shared<CFileItem>(localizedName);
    item->SetLabel2(localizedCategory);
    item->SetProperty("game.videofilter", CVariant{ videoFilter.path });

    items.Add(std::move(item));
  }

  if (items.Size() == 0)
  {
    CFileItemPtr item = std::make_shared<CFileItem>(g_localizeStrings.Get(231)); // "None"
    items.Add(std::move(item));
  }
}

void CDialogGameVideoFilter::OnItemFocus(unsigned int index)
{
  if (index < m_videoFilters.size())
  {
    const std::string &presetToSet = m_videoFilters[index].path;

    CGameSettings &gameSettings = CMediaSettings::GetInstance().GetCurrentGameSettings();
    if (gameSettings.VideoFilter() != presetToSet)
    {
      gameSettings.SetVideoFilter(presetToSet);

      if (m_callback != nullptr)
        m_callback->SetShaderPreset(presetToSet);
    }
  }
}

unsigned int CDialogGameVideoFilter::GetFocusedItem() const
{
  for (unsigned int i = 0; i < m_videoFilters.size(); i++)
  {
    std::string preset = m_callback->GetShaderPreset();
    if (preset == m_videoFilters[i].path)
       return i;
  }
  return 0;
}

void CDialogGameVideoFilter::PostExit()
{
  m_videoFilters.clear();
}

std::string CDialogGameVideoFilter::GetLocalizedString(uint32_t code)
{
  return g_localizeStrings.GetAddonString(PRESETS_ADDON_NAME, code);
}