/*
 *  Copyright (C) 2018-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "SwitchStorageProvider.h"

#include "platform/switch/storage/SwitchMountProvider.h"

IStorageProvider* IStorageProvider::CreateInstance()
{
  return new CSwitchStorageProvider();
}

CSwitchStorageProvider::CSwitchStorageProvider()
{
  if (m_instance == nullptr)
    m_instance = new CSwitchMountProvider();
}

CSwitchStorageProvider::~CSwitchStorageProvider()
{
  delete m_instance;
}

void CSwitchStorageProvider::Initialize()
{
  m_instance->Initialize();
}

void CSwitchStorageProvider::Stop()
{
  m_instance->Stop();
}

void CSwitchStorageProvider::GetLocalDrives(VECSOURCES &localDrives)
{
  m_instance->GetLocalDrives(localDrives);
}

void CSwitchStorageProvider::GetRemovableDrives(VECSOURCES &removableDrives)
{
  m_instance->GetRemovableDrives(removableDrives);
}

bool CSwitchStorageProvider::Eject(const std::string& mountpath)
{
  return m_instance->Eject(mountpath);
}

std::vector<std::string> CSwitchStorageProvider::GetDiskUsage()
{
  return m_instance->GetDiskUsage();
}

bool CSwitchStorageProvider::PumpDriveChangeEvents(IStorageEventsCallback *callback)
{
  return m_instance->PumpDriveChangeEvents(callback);
}
