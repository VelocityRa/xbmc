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
#include "utils/log.h"

#include <string>
#include "utils/URIUtils.h"

using namespace KODI;
using namespace SHADERPRESET;


std::unique_ptr<CShaderPresetAddon> CVideoShaderPreset::shaderPresetAddon;

CVideoShaderPreset::CVideoShaderPreset()
{
  Init();
}

CVideoShaderPreset::CVideoShaderPreset(std::string presetPath)
{
  if (Init())
  {
    ReadPresetFile(presetPath);
  }
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
    shaderPresetAddon.reset(new CShaderPresetAddon(addonInfos.front()));
    shaderPresetAddon->CreateAddon();
    return true;
  }

  CLog::Log(LOGERROR, "VideoShaderPreset: Couldn't initialize addon instance. Make sure there is an enabled shader add-on present.");
  return false;
}

void CVideoShaderPreset::Destroy()
{
  if (m_config)
    FreeConfigFile(m_config);
  if (m_videoShader)
    FreePresetFile(m_videoShader);
  if (shaderPresetAddon)
  {
    shaderPresetAddon->DestroyAddon();
    shaderPresetAddon.reset();
  }
}

// TODO: Don't die in flames if the file doesn't exist
bool CVideoShaderPreset::ReadPresetFile(std::string presetPath)
{
  m_config = shaderPresetAddon->ConfigFileNew(presetPath.c_str());
  if (!m_config)
    return false;
  bool result = ReadPresetConfig(m_config);
  ResolveParameters();
  return result;
}

bool CVideoShaderPreset::ReadPresetConfig(config_file* conf)
{
  if (!shaderPresetAddon) return false;

  m_videoShader = static_cast<video_shader*>(calloc(1, sizeof(video_shader)));

  auto readResult = shaderPresetAddon->ShaderPresetRead(conf, m_videoShader);

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

    CLog::Log(LOGINFO, "Shader Preset Addon: Read shader preset %s", conf->path);
  }

  return readResult;
}

bool CVideoShaderPreset::ReadPresetString(std::string presetString)
{
  config_file* conf = shaderPresetAddon->ConfigFileNewFromString(presetString.c_str());
  bool result = ReadPresetConfig(conf);
  FreeConfigFile(conf);
  return result;
}

bool CVideoShaderPreset::ResolveParameters()
{
  bool resolveResult = shaderPresetAddon->ShaderPresetResolveParameters(m_config, m_videoShader);
  m_NumParameters = m_videoShader->num_parameters;
  memcpy(m_Parameters, m_videoShader->parameters, GFX_MAX_PARAMETERS * sizeof video_shader_parameter);
  return resolveResult;
}

CVideoShaderPreset::~CVideoShaderPreset()
{
  Destroy();
}

bool CShaderPresetAddon::CreateAddon(void)
{
  CExclusiveLock lock(m_dllSection);

  // Reset all properties to defaults
  ResetProperties();

  // Initialise the add-on
  CLog::Log(LOGDEBUG, "%s - creating ShaderPreset add-on instance '%s'",
    __FUNCTION__, Name().c_str());

  if (CreateInstance(&m_struct) != ADDON_STATUS_OK)
    return false;

  return true;
}

void CShaderPresetAddon::DestroyAddon()
{
  {
    CExclusiveLock lock(m_dllSection);
    DestroyInstance();
  }
}

void CVideoShaderPreset::FreeConfigFile(config_file* conf)
{
  return shaderPresetAddon->ConfigFileFree(conf);
}

void CVideoShaderPreset::FreePresetFile(video_shader* shader)
{
  return shaderPresetAddon->VideoShaderFree(shader);
}
