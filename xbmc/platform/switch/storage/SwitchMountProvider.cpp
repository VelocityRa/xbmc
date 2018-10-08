/*
 *  Copyright (C) 2018-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "SwitchMountProvider.h"

#include <cstdlib>

#include "utils/URIUtils.h"
#include "utils/log.h"

CSwitchMountProvider::CSwitchMountProvider()
{
}

void CSwitchMountProvider::Initialize()
{
  CLog::Log(LOGDEBUG, "Selected Switch mount as storage provider");
}

void CSwitchMountProvider::GetDrives(VECSOURCES &drives)
{
  std::vector<std::string> result;

  // TODO(velocity): drives list

  for (unsigned int i = 0; i < result.size(); i++)
  {
    CMediaSource share;
    share.strPath = result[i];
    share.strName = URIUtils::GetFileName(result[i]);
    share.m_ignore = true;
    drives.push_back(share);
  }
}

std::vector<std::string> CSwitchMountProvider::GetDiskUsage()
{
  return {};
}

bool CSwitchMountProvider::Eject(const std::string& mountpath)
{
  return false;
}

bool CSwitchMountProvider::PumpDriveChangeEvents(IStorageEventsCallback *callback)
{
  return false;
}
