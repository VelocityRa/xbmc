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

#include "ShaderPreset.h"
#include "addons/binary-addons/BinaryAddonBase.h"
#include "filesystem/SpecialProtocol.h"
#include "utils/log.h"
#include "utils/URIUtils.h"

using namespace ADDON;

CShaderPresetAddon::CShaderPresetAddon(const BinaryAddonBasePtr& addonBase)
  : IAddonInstanceHandler(ADDON_INSTANCE_SHADERPRESET, addonBase)
{
  ResetProperties();
}

CShaderPresetAddon::~CShaderPresetAddon(void)
{
  DestroyAddon();
}

bool CShaderPresetAddon::CreateAddon(void)
{
  CExclusiveLock lock(m_dllSection);

  // Reset all properties to defaults
  ResetProperties();

  // Initialise the add-on
  CLog::Log(LOGDEBUG, "%s - creating ShaderPreset add-on instance '%s'", __FUNCTION__, Name().c_str());

  if (CreateInstance(&m_struct) != ADDON_STATUS_OK)
    return false;

  return true;
}

void CShaderPresetAddon::DestroyAddon()
{
  CExclusiveLock lock(m_dllSection);
  DestroyInstance();
}

void CShaderPresetAddon::ResetProperties(void)
{
  // Initialise members
  m_struct = {{ 0 }};
  m_struct.toKodi.kodiInstance = this;
}

const char* CShaderPresetAddon::GetLibraryBasePath()
{
  if (m_strLibraryPath.empty())
  {
    std::string strLibPath = Addon()->LibPath();
    m_strLibraryPath = URIUtils::GetBasePath(CSpecialProtocol::TranslatePath(strLibPath));
  }
  return m_strLibraryPath.c_str();
}

config_file *CShaderPresetAddon::ConfigFileNew(const char *path)
{
  m_strConfigPath = URIUtils::AddFileToFolder(GetLibraryBasePath(), path);
  m_strConfigBasePath = URIUtils::GetBasePath(m_strConfigPath);
  return m_struct.toAddon.config_file_new(&m_struct, m_strConfigPath.c_str(), m_strConfigBasePath.c_str());
}

config_file *CShaderPresetAddon::ConfigFileNewFromString(const char *from_string)
{
  return m_struct.toAddon.config_file_new_from_string(&m_struct, from_string);
}

void CShaderPresetAddon::ConfigFileFree(config_file *conf)
{
  return m_struct.toAddon.config_file_free(&m_struct, conf);
}

bool CShaderPresetAddon::ShaderPresetRead(config_file *conf, video_shader *shader)
{
  return m_struct.toAddon.video_shader_read_conf_cgp(&m_struct, conf, shader);
}

void CShaderPresetAddon::ShaderPresetWrite(config_file *conf, video_shader *shader)
{
  return m_struct.toAddon.video_shader_write_conf_cgp(&m_struct, conf, shader);
}

void CShaderPresetAddon::ShaderPresetResolveRelative(video_shader *shader, const char *ref_path)
{
  return m_struct.toAddon.video_shader_resolve_relative(&m_struct, shader, ref_path);
}

bool CShaderPresetAddon::ShaderPresetResolveCurrentParameters(config_file *conf, video_shader *shader)
{
  return m_struct.toAddon.video_shader_resolve_current_parameters(&m_struct, conf, shader);
}

bool CShaderPresetAddon::ShaderPresetResolveParameters(config_file *conf, video_shader *shader)
{
  return m_struct.toAddon.video_shader_resolve_parameters(&m_struct, conf, shader);
}

void CShaderPresetAddon::VideoShaderFree(video_shader *shader)
{
  return m_struct.toAddon.video_shader_free(&m_struct, shader);
}
