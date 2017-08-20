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
#pragma once

#include "controllers/ControllerTypes.h"

#include <memory>
#include <string>

namespace ADDON
{
  class CAddonMgr;
  class CBinaryAddonManager;
}

namespace PERIPHERALS
{
  class CPeripherals;
}

namespace KODI
{
namespace RETRO
{
  class CGUIGameControlManager;
}

namespace SHADER
{
  class CVideoShaderPresetFactory;
}

namespace GAME
{
  class CControllerManager;
  class CPortManager;

  class CGameServices
  {
  public:
    CGameServices(CControllerManager &controllerManager, PERIPHERALS::CPeripherals& peripheralManager, ADDON::CAddonMgr &addons, ADDON::CBinaryAddonManager &binaryAddons);
    ~CGameServices();

    ControllerPtr GetController(const std::string& controllerId);
    ControllerPtr GetDefaultController();
    ControllerVector GetControllers();

    CPortManager& PortManager();

    RETRO::CGUIGameControlManager &GameControls() { return *m_gameControlManager; }

    SHADER::CVideoShaderPresetFactory &VideoShaders() { return *m_videoShaders; }

  private:
    // Construction parameters
    CControllerManager &m_controllerManager;

    // Game services
    std::unique_ptr<CPortManager> m_portManager;
    std::unique_ptr<RETRO::CGUIGameControlManager> m_gameControlManager;
    std::unique_ptr<SHADER::CVideoShaderPresetFactory> m_videoShaders;
  };
}
}
