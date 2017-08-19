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
#include "utils/log.h"

#include <string>
#include "utils/URIUtils.h"

using namespace KODI;
using namespace SHADERPRESET;

std::unique_ptr<ADDON::CShaderPresetAddon> CVideoShaderPreset::shaderPresetAddon;

CVideoShaderPreset::CVideoShaderPreset()
{
  //! @todo Initializes members in header
}

bool CVideoShaderPreset::Init()
{
  if (shaderPresetAddon)
    return true;

  ADDON::BinaryAddonBaseList addonInfos;
  CServiceBroker::GetBinaryAddonManager().GetAddonInfos(addonInfos, true, ADDON::ADDON_SHADERDLL);
  if (addonInfos.size() > 1)
    CLog::Log(LOGWARNING, "VideoShaderPreset: No handling of multiple shader add-ons implemented. Loading first one.");

  if (!addonInfos.empty())
  {
    shaderPresetAddon.reset(new ADDON::CShaderPresetAddon(addonInfos.front()));
    shaderPresetAddon->CreateAddon();
    return true;
  }

  CLog::Log(LOGERROR, "VideoShaderPreset: Couldn't initialize addon instance. Make sure there is an enabled shader add-on present.");
  return false;
}

void CVideoShaderPreset::Destroy()
{
  m_videoShader.reset();
  m_config.reset();
  shaderPresetAddon.reset();
}

// TODO: Don't die in flames if the file doesn't exist
bool CVideoShaderPreset::ReadPresetFile(const std::string &presetPath)
{
  m_config = shaderPresetAddon->LoadShaderPreset(presetPath);
  if (!m_config)
    return false;
  bool result = ReadPresetConfig();
  ResolveParameters();
  return result;
}

bool CVideoShaderPreset::ReadPresetConfig()
{
  if (!shaderPresetAddon) return false;

  m_videoShader.reset(new video_shader);

  auto readResult = m_config->ReadShaderPreset(*m_videoShader);

  if (readResult)
  {
    // Copy over every field we want except parameters, these are resolved and copied elsewhere
    type = m_videoShader->type;
    m_Passes = m_videoShader->passes;
    memcpy(m_Pass, m_videoShader->pass, GFX_MAX_SHADERS * sizeof video_shader_pass);
    m_Luts = m_videoShader->luts;
    memcpy(m_Lut, m_videoShader->lut, GFX_MAX_TEXTURES * sizeof video_shader_lut);
    m_Variables = m_videoShader->variables;
    memcpy(m_Variable, m_videoShader->variable, GFX_MAX_VARIABLES * sizeof state_tracker_uniform_info);
    m_FeedbackPass = m_videoShader->feedback_pass;

    //CLog::Log(LOGINFO, "Shader Preset Addon: Read shader preset %s", conf->path); //! @todo
  }

  return readResult;
}

bool CVideoShaderPreset::ResolveParameters()
{
  bool resolveResult = m_config->ResolveParameters(*m_videoShader);
  m_NumParameters = m_videoShader->num_parameters;
  memcpy(m_Parameters, m_videoShader->parameters, GFX_MAX_PARAMETERS * sizeof video_shader_parameter);
  return resolveResult;
}

CVideoShaderPreset::~CVideoShaderPreset()
{
  Destroy();
}
