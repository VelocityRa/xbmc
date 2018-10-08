/*
 *  Copyright (C) 2011-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "SwitchUtils.h"
#include "windowing/GraphicContext.h"
#include "utils/log.h"
#include "settings/Settings.h"
#include "settings/SettingsComponent.h"
#include "ServiceBroker.h"
#include "utils/StringUtils.h"
#include "utils/SysfsUtils.h"

#include <cmath>
#include <stdlib.h>

#include <EGL/egl.h>

static bool s_hasModeApi = false;
static std::vector<RESOLUTION_INFO> s_res_displayModes;
static RESOLUTION_INFO s_res_cur_displayMode;

static float currentRefreshRate()
{
  // TODO(velocity)  
  return 60.0;
}

static void fetchDisplayModes()
{
  s_hasModeApi = false;
  s_res_displayModes.clear();
  
  // TODO(velocity)
}

CSwitchUtils::CSwitchUtils()
{
}

CSwitchUtils::~CSwitchUtils()
{
}

bool CSwitchUtils::GetNativeResolution(RESOLUTION_INFO *res) const
{
  // TODO(velocity)
  *res = RESOLUTION_INFO(1280, 720);
  return true;
}

bool CSwitchUtils::SetNativeResolution(const RESOLUTION_INFO &res)
{
  // TODO(velocity)
  return true;
}

bool CSwitchUtils::ProbeResolutions(std::vector<RESOLUTION_INFO> &resolutions)
{
  // TODO(velocity)
  return false;
}

bool CSwitchUtils::DestroyWindow()
{
  // TODO(velocity)
  return false;
}
