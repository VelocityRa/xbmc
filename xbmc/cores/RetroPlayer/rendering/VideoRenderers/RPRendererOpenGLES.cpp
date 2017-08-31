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

#include "RPRendererOpenGLES.h"
#include "cores/RetroPlayer/rendering/RPRenderFactory.h"
#include "guilib/GraphicContext.h"
#include "settings/GameSettings.h"
#include "settings/MediaSettings.h"
#include "utils/GLUtils.h"
#include "utils/log.h"
#include "windowing/WindowingFactory.h"

extern "C" {
#include "libswscale/swscale.h"
}

#include <cstring>

using namespace KODI;
using namespace RETRO;

CRPBaseRenderer* CRPRendererOpenGLES::Create()
{
  return new CRPRendererOpenGLES();
}

void CRPRendererOpenGLES::Register()
{
  CRPRendererFactory::RegisterRenderer(CRPRendererOpenGLES::Create);
}

CRPRendererOpenGLES::CRPRendererOpenGLES() :
  m_bQueued(false)
{
}

CRPRendererOpenGLES::~CRPRendererOpenGLES()
{
  Deinitialize();
  if (m_swsContext)
    sws_freeContext(m_swsContext);
}

bool CRPRendererOpenGLES::Configure(AVPixelFormat format, unsigned int width, unsigned int height, unsigned int orientation)
{
  m_format = format;
  m_sourceWidth = width;
  m_sourceHeight = height;
  m_renderOrientation = orientation;

  SetViewMode(CMediaSettings::GetInstance().GetCurrentGameSettings().ViewMode());
  ManageRenderArea();

  m_bConfigured = true;

  m_targetFormat = AV_PIX_FMT_BGRA;

  const unsigned int dstSize = m_sourceWidth * m_sourceHeight * glFormatElementByteCount(GL_RGBA);

  m_buffer.resize(dstSize);
  m_texture.resize(dstSize);

  LoadGameSettings();

  return true;
}

void CRPRendererOpenGLES::AddFrame(const uint8_t* data, unsigned int size)
{
  if (!m_bConfigured)
    return;

  if (data == nullptr || size == 0)
    return;

  if (m_bQueued)
    return;

  if (m_format == m_targetFormat && m_buffer.size() == size)
  {
    std::memcpy(const_cast<uint8_t*>(m_buffer.data()), data, size);
  }
  else
  {
    m_swsContext = sws_getCachedContext(m_swsContext,
                                        m_sourceWidth, m_sourceHeight, m_format,
                                        m_sourceWidth, m_sourceHeight, m_targetFormat,
                                        SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);

    uint8_t *dataMutable = const_cast<uint8_t*>(data);
    const int stride = size / m_sourceHeight;

    uint8_t *dstData = const_cast<uint8_t*>(m_buffer.data());
    const int dstPitch = m_buffer.size() / m_sourceHeight;

    uint8_t* src[] =       { dataMutable, nullptr,   nullptr,   nullptr };
    int      srcStride[] = { stride,            0,         0,         0 };
    uint8_t *dst[] =       { dstData,     nullptr,   nullptr,   nullptr };
    int      dstStride[] = { dstPitch,          0,         0,         0 };

    sws_scale(m_swsContext, src, srcStride, 0, m_sourceHeight, dst, dstStride);
  }

  m_bQueued = true;
}

void CRPRendererOpenGLES::Flush()
{
  if (!m_bConfigured)
    return;

  glFinish();
  m_bQueued = false;
}

void CRPRendererOpenGLES::RenderUpdate(bool clear, unsigned int alpha)
{
  if (!m_bConfigured)
    return;

  ManageRenderArea();

  if (clear)
  {
    if (alpha == 255)
      DrawBlackBars();
    else
      ClearBackBuffer();
  }

  if (alpha < 255)
  {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(1.0f, 1.0f, 1.0f, alpha / 250.f);
  }
  else
  {
    glDisable(GL_BLEND);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
  }

  if (m_bQueued)
  {
    m_texture.swap(m_buffer);
    m_bQueued = false;
  }

  Render();

  glEnable(GL_BLEND);
  glFlush();
}

void CRPRendererOpenGLES::Deinitialize()
{
  DeleteTexture();
}

bool CRPRendererOpenGLES::Supports(ERENDERFEATURE feature) const
{
  if (feature == RENDERFEATURE_STRETCH         ||
      feature == RENDERFEATURE_ZOOM            ||
      feature == RENDERFEATURE_VERTICAL_SHIFT  ||
      feature == RENDERFEATURE_PIXEL_RATIO     ||
      feature == RENDERFEATURE_POSTPROCESS     ||
      feature == RENDERFEATURE_ROTATION)
  {
    return true;
  }

  return false;
}

bool CRPRendererOpenGLES::Supports(ESCALINGMETHOD method) const
{
  if (method == VS_SCALINGMETHOD_NEAREST ||
      method == VS_SCALINGMETHOD_LINEAR)
  {
    return true;
  }

  return false;
}

void CRPRendererOpenGLES::CreateTexture()
{
  glGenTextures(1, &m_textureId);

  glBindTexture(m_textureTarget, m_textureId);

  glTexImage2D(m_textureTarget, 0, GL_RGBA, m_sourceWidth, m_sourceHeight, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);

  glTexParameteri(m_textureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(m_textureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(m_textureTarget, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(m_textureTarget, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glDisable(m_textureTarget);
}

void CRPRendererOpenGLES::UploadTexture()
{
  glEnable(m_textureTarget);

  const unsigned int bpp = 1;
  glPixelStorei(GL_UNPACK_ALIGNMENT, bpp);

  const unsigned datatype = GL_UNSIGNED_BYTE;

  glBindTexture(m_textureTarget, m_textureId);
  glTexSubImage2D(m_textureTarget, 0, 0, 0, m_sourceWidth, m_sourceHeight, GL_BGRA, datatype, m_texture.data());

  glBindTexture(m_textureTarget, 0);

  glDisable(m_textureTarget);
}

void CRPRendererOpenGLES::DeleteTexture()
{
  if (glIsTexture(m_textureId))
    glDeleteTextures(1, &m_textureId);

  m_textureId = 0;
}

void CRPRendererOpenGLES::ClearBackBuffer()
{
  glClearColor(m_clearColour, m_clearColour, m_clearColour, 0);
  glClear(GL_COLOR_BUFFER_BIT);
  glClearColor(0, 0, 0, 0);
}

void CRPRendererOpenGLES::DrawBlackBars()
{
  glColor4f(m_clearColour, m_clearColour, m_clearColour, 1.0f);
  glDisable(GL_BLEND);
  glBegin(GL_QUADS);

  //top quad
  if (m_rotatedDestCoords[0].y > 0.0)
  {
    glVertex4f(0.0,                          0.0,                      0.0, 1.0);
    glVertex4f(g_graphicsContext.GetWidth(), 0.0,                      0.0, 1.0);
    glVertex4f(g_graphicsContext.GetWidth(), m_rotatedDestCoords[0].y, 0.0, 1.0);
    glVertex4f(0.0,                          m_rotatedDestCoords[0].y, 0.0, 1.0);
  }

  //bottom quad
  if (m_rotatedDestCoords[2].y < g_graphicsContext.GetHeight())
  {
    glVertex4f(0.0,                          m_rotatedDestCoords[2].y,      0.0, 1.0);
    glVertex4f(g_graphicsContext.GetWidth(), m_rotatedDestCoords[2].y,      0.0, 1.0);
    glVertex4f(g_graphicsContext.GetWidth(), g_graphicsContext.GetHeight(), 0.0, 1.0);
    glVertex4f(0.0,                          g_graphicsContext.GetHeight(), 0.0, 1.0);
  }

  //left quad
  if (m_rotatedDestCoords[0].x > 0.0)
  {
    glVertex4f(0.0,                      m_rotatedDestCoords[0].y, 0.0, 1.0);
    glVertex4f(m_rotatedDestCoords[0].x, m_rotatedDestCoords[0].y, 0.0, 1.0);
    glVertex4f(m_rotatedDestCoords[0].x, m_rotatedDestCoords[2].y, 0.0, 1.0);
    glVertex4f(0.0,                      m_rotatedDestCoords[2].y, 0.0, 1.0);
  }

  //right quad
  if (m_rotatedDestCoords[2].x < g_graphicsContext.GetWidth())
  {
    glVertex4f(m_rotatedDestCoords[2].x,     m_rotatedDestCoords[0].y, 0.0, 1.0);
    glVertex4f(g_graphicsContext.GetWidth(), m_rotatedDestCoords[0].y, 0.0, 1.0);
    glVertex4f(g_graphicsContext.GetWidth(), m_rotatedDestCoords[2].y, 0.0, 1.0);
    glVertex4f(m_rotatedDestCoords[2].x,     m_rotatedDestCoords[2].y, 0.0, 1.0);
  }

  glEnd();
}

void CRPRendererOpenGLES::Render()
{
  // Create texture
  if (!glIsTexture(m_textureId))
    CreateTexture();

  // Upload texture
  UploadTexture();

  // Render texture
  glEnable(m_textureTarget);

  glActiveTextureARB(GL_TEXTURE0);

  glBindTexture(m_textureTarget, m_textureId);

  // Try some clamping or wrapping
  glTexParameteri(m_textureTarget, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(m_textureTarget, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  GLint filter = (m_scalingMethod == VS_SCALINGMETHOD_NEAREST ? GL_NEAREST : GL_LINEAR);
  glTexParameteri(m_textureTarget, GL_TEXTURE_MAG_FILTER, filter);
  glTexParameteri(m_textureTarget, GL_TEXTURE_MIN_FILTER, filter);

  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

  glBegin(GL_QUADS);

  CRect rect = m_sourceRect;

  rect.x1 /= m_sourceWidth;
  rect.x2 /= m_sourceWidth;
  rect.y1 /= m_sourceHeight;
  rect.y2 /= m_sourceHeight;

  glTexCoord2f(rect.x1, rect.y1);  glVertex2f(m_rotatedDestCoords[0].x, m_rotatedDestCoords[0].y);
  glTexCoord2f(rect.x2, rect.y1);  glVertex2f(m_rotatedDestCoords[1].x, m_rotatedDestCoords[1].y);
  glTexCoord2f(rect.x2, rect.y2);  glVertex2f(m_rotatedDestCoords[2].x, m_rotatedDestCoords[2].y);
  glTexCoord2f(rect.x1, rect.y2);  glVertex2f(m_rotatedDestCoords[3].x, m_rotatedDestCoords[3].y);

  glEnd();

  glBindTexture (m_textureTarget, 0);
  glDisable(m_textureTarget);
}
