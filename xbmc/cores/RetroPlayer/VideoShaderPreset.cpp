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
    ReadPresetFile(presetPath);
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
  if (shaderPresetAddon)
  {
    shaderPresetAddon->DestroyAddon();
    shaderPresetAddon.reset();
  }
}

// TODO: Don't die in flames if the file doesn't exist
bool CVideoShaderPreset::ReadPresetFile(std::string presetPath)
{
  config_file_t_* conf = shaderPresetAddon->ConfigFileNew(presetPath.c_str());
  if (!conf)
    return false;
  bool result = ReadPresetConfig(conf);
  FreeConfigFile(conf);
  return result;
}

bool CVideoShaderPreset::ReadPresetConfig(config_file_t_* conf)
{
  if (!shaderPresetAddon) return false;

  video_shader_* new_shader = static_cast<video_shader_*>(calloc(1, sizeof(video_shader_)));

  auto readResult = shaderPresetAddon->ShaderPresetRead(conf, new_shader);

  if (readResult)
  {
    // Copy over every field
    type = new_shader->type;
//    modern = new_shader->modern;
//    strcpy_s(prefix, 64, new_shader->prefix);
    m_Passes = new_shader->passes;
    memcpy(m_Pass, new_shader->pass, GFX_MAX_SHADERS * sizeof(video_shader_pass_));
    m_Luts = new_shader->luts;
    memcpy(m_Lut, new_shader->lut, GFX_MAX_TEXTURES * sizeof(video_shader_lut_));
    m_NumParameters = new_shader->num_parameters;
    memcpy(m_Parameters, new_shader->parameters, GFX_MAX_PARAMETERS * sizeof(video_shader_parameter_));
    m_Variables = new_shader->variables;
    memcpy(m_Variable, new_shader->variable, GFX_MAX_VARIABLES * sizeof(state_tracker_uniform_info_));
//    strcpy_s(script_path, PATH_MAX_LENGTH, new_shader->script_path);
//    script = new_shader->script;
//    strcpy_s(script_class, 512, new_shader->script_class);
    m_FeedbackPass = new_shader->feedback_pass;

    CLog::Log(LOGINFO, "Shader Preset Addon: Read shader preset %s", conf->path);
  }

  // Free new_shader (we copied the data we need, or parsing failed)
  free(new_shader);

  return readResult;
}

bool CVideoShaderPreset::ReadPresetString(std::string presetString)
{
  config_file_t_* conf = shaderPresetAddon->ConfigFileNewFromString(presetString.c_str());
  bool result = ReadPresetConfig(conf);
  FreeConfigFile(conf);
  return result;
}

const char* CVideoShaderPreset::GetLibraryBasePath()
{
  if (Init())
    return shaderPresetAddon->GetLibraryBasePath();
  return "";
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

void CVideoShaderPreset::FreeConfigFile(config_file_t_* conf)
{
   return shaderPresetAddon->ConfigFileFree(conf);
}
