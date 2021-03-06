/*
 *  Copyright (C) 2018-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include "storage/IStorageProvider.h"

#include <vector>

class CSwitchStorageProvider : public IStorageProvider
{
public:
  CSwitchStorageProvider();
  virtual ~CSwitchStorageProvider();

  void Initialize() override;
  void Stop() override;
  void GetLocalDrives(VECSOURCES &localDrives) override;
  void GetRemovableDrives(VECSOURCES &removableDrives) override;
  bool Eject(const std::string& mountpath) override;
  std::vector<std::string> GetDiskUsage() override;
  bool PumpDriveChangeEvents(IStorageEventsCallback *callback) override;

private:
  IStorageProvider *m_instance{};
};
