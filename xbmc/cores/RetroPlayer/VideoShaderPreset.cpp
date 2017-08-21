/*
*      Copyright (C) 2017 Team Kodi
 *     http://kodi.tv
 *
 * This Program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This Program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this Program; see the file COPYING.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 */

#include "VideoShaderPreset.h"
#include "ServiceBroker.h"
#include "addons/binary-addons/BinaryAddonBase.h"
#include "addons/ShaderPreset.h"
#include "games/GameServices.h"
#include "utils/log.h"

#include <string>
#include "utils/URIUtils.h"

using namespace KODI;
using namespace SHADER;
using namespace SHADERPRESET;

CVideoShaderPreset::CVideoShaderPreset()
{
  //! @todo Initializes members in header
}

bool CVideoShaderPreset::Init()
{
  return true;
}

void CVideoShaderPreset::Destroy()
{
  m_videoShader = VideoShaderPreset(); //! @todo Need a clear method
}

// TODO: Don't die in flames if the file doesn't exist
bool CVideoShaderPreset::ReadPresetFile(const std::string &presetPath)
{
  return CServiceBroker::GetGameServices().VideoShaders().LoadPreset(presetPath, m_videoShader);
}

CVideoShaderPreset::~CVideoShaderPreset()
{
  Destroy();
}
