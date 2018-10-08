/*
 *  Copyright (C) 2011-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include <string>
#include <vector>

#include "windowing/Resolution.h"

class CSwitchUtils
{
public:
  CSwitchUtils();
  virtual ~CSwitchUtils();

  // TODO(velocity): How many of these do we need?
  virtual bool GetNativeResolution(RESOLUTION_INFO *res) const;
  virtual bool SetNativeResolution(const RESOLUTION_INFO &res);
  virtual bool ProbeResolutions(std::vector<RESOLUTION_INFO> &resolutions);
  virtual bool DestroyWindow();

protected:
  mutable int m_width;
  mutable int m_height;
};
