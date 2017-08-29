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
#include "guilib/GUITextBox.h"
#include "guilib/LocalizeStrings.h"
#include "guilib/WindowIDs.h"
#include "settings/GameSettings.h"
#include "settings/MediaSettings.h"
#include "utils/log.h"
#include "utils/StringUtils.h"
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

namespace
{
  struct ScalingMethodProperties
  {
    int nameIndex;
    int categoryIndex;
    int descriptionIndex;
    ESCALINGMETHOD scalingMethod;
  };

  const std::vector<ScalingMethodProperties> scalingMethods =
  {
    { 16296, 16301, 16298, VS_SCALINGMETHOD_NEAREST },
    { 16297, 16302, 16299, VS_SCALINGMETHOD_LINEAR },
  };
}

CDialogGameVideoFilter::CDialogGameVideoFilter() :
  CDialogGameVideoSelect(WINDOW_DIALOG_GAME_VIDEO_FILTER)
{
}

void CDialogGameVideoFilter::PreInit()
{
  m_items.Clear();

  InitScalingMethods();
  InitVideoFilters();

  if (m_items.Size() == 0)
  {
    CFileItemPtr item = std::make_shared<CFileItem>(g_localizeStrings.Get(231)); // "None"
    m_items.Add(std::move(item));
  }

  m_bHasDescription = false;
}

void CDialogGameVideoFilter::InitScalingMethods()
{
  if (m_callback != nullptr)
  {
    for (const auto &scalingMethodProps : scalingMethods)
    {
      if (m_callback->SupportsScalingMethod(scalingMethodProps.scalingMethod))
      {
        CFileItemPtr item = std::make_shared<CFileItem>(g_localizeStrings.Get(scalingMethodProps.nameIndex));
        item->SetLabel2(g_localizeStrings.Get(scalingMethodProps.categoryIndex));
        item->SetProperty("game.scalingmethod", CVariant{ scalingMethodProps.scalingMethod });
        item->SetProperty("game.videofilterdescription", CVariant{ g_localizeStrings.Get(scalingMethodProps.descriptionIndex) });
        m_items.Add(std::move(item));
      }
    }
  }
}

void CDialogGameVideoFilter::InitVideoFilters()
{
  std::vector<VideoFilterProperties> videoFilters;

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

    if (child->FirstChild() == nullptr)
      continue;

    TiXmlNode* pathNode;
    if(((pathNode = child->FirstChild("path"))) && ((pathNode = pathNode->FirstChild())))
      videoFilter.path = URIUtils::AddFileToFolder(basePath, pathNode->Value());
    TiXmlNode* nameIndexNode;
    if (((nameIndexNode = child->FirstChild("name"))) && ((nameIndexNode = nameIndexNode->FirstChild())))
      videoFilter.nameIndex = atoi(nameIndexNode->Value());
    TiXmlNode* categoryIndexNode;
    if (((categoryIndexNode = child->FirstChild("category"))) && ((categoryIndexNode = categoryIndexNode->FirstChild())))
      videoFilter.categoryIndex = atoi(categoryIndexNode->Value());
    TiXmlNode* descriptionNode;
    if (((descriptionNode = child->FirstChild("description"))) && ((descriptionNode = descriptionNode->FirstChild())))
      videoFilter.descriptionIndex = atoi(descriptionNode->Value());

    videoFilters.emplace_back(videoFilter);
  }

  CLog::Log(LOGDEBUG, "Loaded %d shader presets from default .xml, %s", videoFilters.size(), CURL::GetRedacted(xmlPath).c_str());

  for (const auto &videoFilter : videoFilters)
  {
    bool canLoadPreset = CServiceBroker::GetGameServices().VideoShaders().CanLoadPreset(videoFilter.path);

    if (!canLoadPreset)
      continue;

    auto localizedName = GetLocalizedString(videoFilter.nameIndex);
    auto localizedCategory = GetLocalizedString(videoFilter.categoryIndex);
    auto localizedDescription = GetLocalizedString(videoFilter.descriptionIndex);

    CFileItemPtr item = std::make_shared<CFileItem>(localizedName);
    item->SetLabel2(localizedCategory);
    item->SetProperty("game.videofilter", CVariant{ videoFilter.path });
    item->SetProperty("game.videofilterdescription", CVariant{ localizedDescription });

    m_items.Add(std::move(item));
  }
}

void CDialogGameVideoFilter::GetItems(CFileItemList &items)
{
  for (const auto &item : m_items)
    items.Add(item);
}

void CDialogGameVideoFilter::OnItemFocus(unsigned int index)
{
  if (static_cast<int>(index) < m_items.Size())
  {
    CFileItemPtr item = m_items[index];

    std::string presetToSet;
    ESCALINGMETHOD scalingMethod;
    std::string description;
    GetProperties(*item, presetToSet, scalingMethod, description);

    CGameSettings &gameSettings = CMediaSettings::GetInstance().GetCurrentGameSettings();

    if (gameSettings.VideoFilter() != presetToSet ||
        gameSettings.ScalingMethod() != scalingMethod)
    {
      gameSettings.SetVideoFilter(presetToSet);
      gameSettings.SetScalingMethod(scalingMethod);

      if (m_callback != nullptr)
      {
        m_callback->SetShaderPreset(presetToSet);
        m_callback->SetScalingMethod(scalingMethod);
      }

      OnDescriptionChange(description);
      m_bHasDescription = true;
    }
    else if (!m_bHasDescription)
    {
      OnDescriptionChange(description);
      m_bHasDescription = true;
    }
  }
}

unsigned int CDialogGameVideoFilter::GetFocusedItem() const
{
  CGameSettings &gameSettings = CMediaSettings::GetInstance().GetCurrentGameSettings();

  for (int i = 0; i < m_items.Size(); i++)
  {
    std::string presetToSet;
    ESCALINGMETHOD scalingMethod;
    std::string description;
    GetProperties(*m_items[i], presetToSet, scalingMethod, description);

    if (presetToSet == gameSettings.VideoFilter() &&
        scalingMethod == gameSettings.ScalingMethod())
    {
      return i;
    }
  }

  return 0;
}

void CDialogGameVideoFilter::PostExit()
{
  m_items.Clear();
}

std::string CDialogGameVideoFilter::GetLocalizedString(uint32_t code)
{
  return g_localizeStrings.GetAddonString(PRESETS_ADDON_NAME, code);
}

void CDialogGameVideoFilter::GetProperties(const CFileItem &item, std::string &videoPreset, ESCALINGMETHOD &scalingMethod, std::string &description)
{
  videoPreset = item.GetProperty("game.videofilter").asString();
  scalingMethod = VS_SCALINGMETHOD_NEAREST;
  description = item.GetProperty("game.videofilterdescription").asString();

  std::string strScalingMethod = item.GetProperty("game.scalingmethod").asString();
  if (StringUtils::IsNaturalNumber(strScalingMethod))
    scalingMethod = static_cast<ESCALINGMETHOD>(item.GetProperty("game.scalingmethod").asUnsignedInteger());
}
