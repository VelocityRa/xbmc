/*
 *      Copyright (C) 2007-2015 Team Kodi
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
 *  along with Kodi; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#pragma once

#include "system.h"

#if defined(TARGET_ANDROID)

#include "cores/RetroPlayer/VideoRenderers/LinuxRendererGLES.h"

class CRPRendererMediaCodec : public CLinuxRendererGLES
{
public:
  CRPRendererMediaCodec();
  virtual ~CRPRendererMediaCodec();
  
  // Player functions
  virtual void AddRPVideoPictureHW(RPVideoPicture &picture, int index);
  virtual bool RenderUpdateCheckForEmptyField();
  virtual void ReleaseBuffer(int idx);

  // Feature support
  virtual bool Supports(EINTERLACEMETHOD method); 

  virtual EINTERLACEMETHOD AutoInterlaceMethod();
  virtual CRPRenderInfo GetRenderInfo();

protected:
  // textures
  virtual bool UploadTexture(int index);
  virtual void DeleteTexture(int index);
  virtual bool CreateTexture(int index);

  // hooks for hw dec renderer
  virtual bool LoadShadersHook();
  virtual bool RenderHook(int index);
  virtual int  GetImageHook(YV12ImageRP *image, int source = AUTOSOURCE, bool readonly = false);
};

#endif