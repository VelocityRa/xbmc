/*
 *  Copyright (C) 2018-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

/**\brief Class for resource usage information
 *
 * Contains method which retrieve platform specific information
 * and methods regarding resoure utilization metrics
 */
class CResourceCounter
{
public:
  /**\brief C'tor */
  CResourceCounter() = default;

  /**\brief D'tor */
  virtual ~CResourceCounter() = default;

  /**\brief Returns an estimation of current processor usage (%)
   */
  virtual double GetCPUUsage() { return 0.0f; };

  /**\brief Resets metrics measuring
   */
  virtual void Reset() {};
};
