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
#include "addons/AddonManager.h"
#include "filesystem/SpecialProtocol.h"
#include "utils/log.h"
#include "utils/URIUtils.h"

using namespace KODI;
using namespace ADDON;

// --- CShaderPreset -----------------------------------------------------------

CShaderPreset::CShaderPreset(config_file *file, AddonInstance_ShaderPreset &instanceStruct) :
  m_file(file),
  m_struct(instanceStruct)
{
}

CShaderPreset::~CShaderPreset()
{
  m_struct.toAddon.config_file_free(&m_struct, m_file);
}

bool CShaderPreset::ReadShaderPreset(video_shader &shader)
{
  return m_struct.toAddon.video_shader_read(&m_struct, m_file, &shader);
}

void CShaderPreset::WriteShaderPreset(const video_shader &shader)
{
  return m_struct.toAddon.video_shader_write(&m_struct, m_file, &shader);
}

/*
void CShaderPresetAddon::ShaderPresetResolveRelative(video_shader &shader, const std::string &ref_path)
{
  return m_struct.toAddon.video_shader_resolve_relative(&m_struct, &shader, ref_path.c_str());
}

bool CShaderPresetAddon::ShaderPresetResolveCurrentParameters(video_shader &shader)
{
  return m_struct.toAddon.video_shader_resolve_current_parameters(&m_struct, m_file, &shader);
}
*/

bool CShaderPreset::ResolveParameters(video_shader &shader)
{
  return m_struct.toAddon.video_shader_resolve_parameters(&m_struct, m_file, &shader);
}

void CShaderPreset::FreeShaderPreset(video_shader &shader)
{
  m_struct.toAddon.video_shader_free(&m_struct, &shader);
}

// --- CShaderPresetAddon ------------------------------------------------------

CShaderPresetAddon::CShaderPresetAddon(const BinaryAddonBasePtr& addonBase)
  : IAddonInstanceHandler(ADDON_INSTANCE_SHADERPRESET, addonBase)
{
  ResetProperties();
  m_extensions = StringUtils::Split(addonBase->Type(ADDON_SHADERDLL)->GetValue("@extensions").asString(), "|");
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

bool CShaderPresetAddon::LoadPreset(const std::string &presetPath, KODI::SHADER::VideoShaderPreset &shaderPreset)
{
  bool bSuccess = false;

  //! @todo Resolve special protocol

  config_file *file = m_struct.toAddon.config_file_new(&m_struct, presetPath.c_str());

  if (file != nullptr)
  {
    std::unique_ptr<CShaderPreset> shaderPresetAddon(new CShaderPreset(file, m_struct));

    video_shader videoShader;
    if (shaderPresetAddon->ReadShaderPreset(videoShader))
    {
      if (shaderPresetAddon->ResolveParameters(videoShader))
      {
        TranslateShaderPreset(videoShader, shaderPreset);
        bSuccess = true;
      }
      shaderPresetAddon->FreeShaderPreset(videoShader);
    }
  }

  return bSuccess;
}

void CShaderPresetAddon::TranslateShaderPreset(const video_shader &shader, KODI::SHADER::VideoShaderPreset &shaderPreset)
{
  //! @todo
}
