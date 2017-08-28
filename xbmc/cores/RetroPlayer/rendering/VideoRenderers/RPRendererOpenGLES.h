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

#include "RPBaseRenderer.h"

#include "system_gl.h"

#include <atomic>
#include <stdint.h>
#include <vector>

struct SwsContext;

namespace KODI
{
namespace RETRO
{
  class CRPRendererOpenGLES : public CRPBaseRenderer
  {
  public:
    static CRPBaseRenderer* Create();
    static void Register();

    CRPRendererOpenGLES();
    ~CRPRendererOpenGLES() override;

    // implementation of CRPBaseRenderer
    bool Configure(AVPixelFormat format, unsigned int width, unsigned int height, unsigned int orientation) override;
    void AddFrame(const uint8_t* data, unsigned int size) override;
    void Flush() override;
    void RenderUpdate(bool clear, unsigned int alpha) override;
    void Deinitialize() override;
    bool Supports(ERENDERFEATURE feature) const override;
    bool Supports(ESCALINGMETHOD method) const override;

  protected:
    virtual void CreateTexture();
    virtual void UploadTexture();
    virtual void DeleteTexture();

    /*!
     * \brief Set the entire backbuffer to black
     */
    void ClearBackBuffer();

    /*!
     * \brief Draw black bars around the video quad
     *
     * This is more efficient than glClear() since it only sets pixels to
     * black that aren't going to be overwritten by the game.
     */
    void DrawBlackBars();

    void Render();

    bool m_bConfigured = false;
    AVPixelFormat m_targetFormat = AV_PIX_FMT_NONE;
    GLenum m_textureTarget = GL_TEXTURE_2D;
    float m_clearColour = 0.0f;
    SwsContext *m_swsContext = nullptr;
    std::vector<uint8_t> m_buffer;
    std::vector<uint8_t> m_texture;
    GLuint m_textureId = 0;
    std::atomic<bool> m_bQueued;
  };
}
}
