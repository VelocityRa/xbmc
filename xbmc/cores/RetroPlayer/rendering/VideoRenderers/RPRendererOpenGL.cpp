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

#include "RPRendererOpenGL.h"
#include "cores/RetroPlayer/rendering/RPRenderFactory.h"
#include "windowing/WindowingFactory.h"

using namespace KODI;
using namespace RETRO;

CRPBaseRenderer* CRPRendererOpenGL::Create()
{
  return new CRPRendererOpenGL();
}

void CRPRendererOpenGL::Register()
{
  CRPRendererFactory::RegisterRenderer(CRPRendererOpenGL::Create);
}

CRPRendererOpenGL::CRPRendererOpenGL()
{
  // Initialize CRPRendererOpenGLES
  m_clearColour = g_Windowing.UseLimitedColor() ? (16.0f / 0xff) : 0.0f;
}

void CRPRendererOpenGL::UploadTexture(const uint8_t* textureData, unsigned int width, unsigned int height)
{
  glEnable(m_textureTarget);

  const unsigned int bpp = 1;
  glPixelStorei(GL_UNPACK_ALIGNMENT, bpp);

  const unsigned datatype = GL_UNSIGNED_BYTE;

  glPixelStorei(GL_UNPACK_ROW_LENGTH, m_sourceWidth);

  glBindTexture(m_textureTarget, m_textureId);
  glTexSubImage2D(m_textureTarget, 0, 0, 0, width, height, GL_BGRA, datatype, textureData);

  glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

  glBindTexture(m_textureTarget, 0);

  glDisable(m_textureTarget);
}
