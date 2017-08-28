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

#include "RPBaseRenderer.h"
#include "cores/RetroPlayer/rendering/VideoShaders/IVideoShaderPreset.h"
#include "guilib/GraphicContext.h"
#include "guilib/GUIWindowManager.h"
#include "settings/GameSettings.h"
#include "settings/MediaSettings.h"
#include "settings/Settings.h"
#include "utils/MathUtils.h"
#include "ServiceBroker.h"

#include <cmath>
#include <cstdlib>
#include <algorithm>

using namespace KODI;
using namespace RETRO;

CRPBaseRenderer::CRPBaseRenderer()
  : m_shaderPreset(nullptr)
  , m_shadersNeedUpdate(true)
  , m_bUseShaderPreset(true)
{
  m_oldDestRect.SetRect(0.0f, 0.0f, 0.0f, 0.0f);

  for(int i=0; i < 4; i++)
  {
    m_rotatedDestCoords[i].x = 0;
    m_rotatedDestCoords[i].y = 0;
  }
}

CRPBaseRenderer::~CRPBaseRenderer() = default;

void CRPBaseRenderer::LoadGameSettings()
{
  CGameSettings &gameSettings = CMediaSettings::GetInstance().GetCurrentGameSettings();

  SetScalingMethod(gameSettings.ScalingMethod());
  SetViewMode(gameSettings.ViewMode());
}

void CRPBaseRenderer::GetVideoRect(CRect &source, CRect &dest, CRect &view)
{
  source = m_sourceRect;
  dest = m_destRect;
  view = m_viewRect;
}

float CRPBaseRenderer::GetAspectRatio() const
{
  float width = static_cast<float>(m_sourceWidth);
  float height = static_cast<float>(m_sourceHeight);

  return m_sourceFrameRatio * width / height * m_sourceHeight / m_sourceWidth;
}

const std::string& CRPBaseRenderer::GetShaderPreset() const
{
  return m_shaderPresetPath;
}

void CRPBaseRenderer::SetShaderPreset(const std::string presetPath)
{
  if (presetPath != m_shaderPresetPath)
  {
    m_shaderPresetPath = presetPath;
    m_shadersNeedUpdate = true;
  }
}

void CRPBaseRenderer::SetScalingMethod(ESCALINGMETHOD method)
{
  if (Supports(method))
    m_scalingMethod = method;
}

void CRPBaseRenderer::SetViewMode(ViewMode viewMode)
{
  if (viewMode == ViewModeStretch16x9Nonlin && !Supports(RENDERFEATURE_NONLINSTRETCH))
    return;

  m_viewMode = viewMode;

  // Get our calibrated full screen resolution
  RESOLUTION res = g_graphicsContext.GetVideoResolution();
  RESOLUTION_INFO info = g_graphicsContext.GetResInfo();

  float screenWidth = static_cast<float>(info.Overscan.right - info.Overscan.left);
  float screenHeight = static_cast<float>(info.Overscan.bottom - info.Overscan.top);

  // And the source frame ratio
  float sourceFrameRatio = GetAspectRatio();

  // Splitres scaling factor
  float xscale = static_cast<float>(info.iScreenWidth) / static_cast<float>(info.iWidth);
  float yscale = static_cast<float>(info.iScreenHeight) / static_cast<float>(info.iHeight);

  screenWidth *= xscale;
  screenHeight *= yscale;

  m_bNonLinearStretch = false;

  if (m_viewMode == ViewModeZoom)
  {
    // Zoom image so no black bars
    m_pixelRatio = 1.0f;

    // Calculate the desired output ratio
    float outputFrameRatio = sourceFrameRatio * m_pixelRatio / info.fPixelRatio;

    // Now calculate the correct zoom amount
    // First zoom to full height
    float newHeight = screenHeight;
    float newWidth = newHeight * outputFrameRatio;

    m_zoomAmount = newWidth / screenWidth;

    if (newWidth < screenWidth)
    {
      // Zoom to full width
      newWidth = screenWidth;
      newHeight = newWidth / outputFrameRatio;
      m_zoomAmount = newHeight / screenHeight;
    }
  }
  else if (m_viewMode == ViewModeStretch4x3)
  {
    // Stretch image to 4:3 ratio
    m_zoomAmount = 1.0f;

    if (res == RES_PAL_4x3 || res == RES_PAL60_4x3 || res == RES_NTSC_4x3 || res == RES_HDTV_480p_4x3)
    {
      // Stretch to the limits of the 4:3 screen
      // Incorrect behaviour, but it's what the users want, so...
      m_pixelRatio = (screenWidth / screenHeight) * info.fPixelRatio / sourceFrameRatio;
    }
    else
    {
      // Now we need to set m_pixelRatio so that fOutputFrameRatio = 4:3
      m_pixelRatio = (4.0f / 3.0f) / sourceFrameRatio;
    }
  }
  else if (m_viewMode == ViewModeWideZoom)
  {
    // Super zoom
    float stretchAmount = (screenWidth / screenHeight) * info.fPixelRatio / sourceFrameRatio;

    m_pixelRatio = pow(stretchAmount, float(2.0 / 3.0));
    m_zoomAmount = pow(stretchAmount, float((stretchAmount < 1.0) ? -1.0 / 3.0 : 1.0 / 3.0));

    m_bNonLinearStretch = true;
  }
  else if (m_viewMode == ViewModeStretch16x9 || m_viewMode == ViewModeStretch16x9Nonlin)
  {
    // Stretch image to 16:9 ratio
    m_zoomAmount = 1.0f;

    if (res == RES_PAL_4x3 || res == RES_PAL60_4x3 || res == RES_NTSC_4x3 || res == RES_HDTV_480p_4x3)
    {
      // Now we need to set m_pixelRatio so that outputFrameRatio = 16:9.
      m_pixelRatio = (16.0f / 9.0f) / sourceFrameRatio;
    }
    else
    {
      // Stretch to the limits of the 16:9 screen
      // Incorrect behaviour, but it's what the users want, so...
      m_pixelRatio = (screenWidth / screenHeight) * info.fPixelRatio / sourceFrameRatio;
    }

    m_bNonLinearStretch = (m_viewMode == ViewModeStretch16x9Nonlin);
  }
  else  if (m_viewMode == ViewModeOriginal)
  {
    // Zoom image so that the height is the original size
    m_pixelRatio = 1.0f;

    // Get the size of the media file
    // Calculate the desired output ratio
    float outputFrameRatio = sourceFrameRatio * m_pixelRatio / info.fPixelRatio;

    // Now calculate the correct zoom amount.  First zoom to full width.
    float newHeight = screenWidth / outputFrameRatio;
    if (newHeight > screenHeight)
    {
      // Zoom to full height
      newHeight = screenHeight;
    }

    // Now work out the zoom amount so that no zoom is done
    m_zoomAmount = m_sourceHeight / newHeight;
  }
  else if (m_viewMode == ViewModeNormal)
  {
    m_pixelRatio = 1.0f;
    m_zoomAmount = 1.0f;
  }
}

inline void CRPBaseRenderer::ReorderDrawPoints()
{
  // 0 - top left, 1 - top right, 2 - bottom right, 3 - bottom left
  float origMat[4][2] =
    {
      { m_destRect.x1, m_destRect.y1 },
      { m_destRect.x2, m_destRect.y1 },
      { m_destRect.x2, m_destRect.y2 },
      { m_destRect.x1, m_destRect.y2 }
    };

  bool changeAspect = false;
  int pointOffset = 0;

  switch (m_renderOrientation)
  {
  case 90:
    pointOffset = 1;
    changeAspect = true;
    break;
  case 180:
    pointOffset = 2;
    break;
  case 270:
    pointOffset = 3;
    changeAspect = true;
    break;
  }

  // If renderer doesn't support rotation, treat orientation as 0 degree so
  // that ffmpeg might handle it
  if (!Supports(RENDERFEATURE_ROTATION))
  {
    pointOffset = 0;
    changeAspect = false;
  }

  float diffX = 0.0f;
  float diffY = 0.0f;
  float centerX = 0.0f;
  float centerY = 0.0f;

  if (changeAspect) // We are either rotating by 90 or 270 degrees which inverts aspect ratio
  {
    float newWidth = m_destRect.Height(); // New width is old height
    float newHeight = m_destRect.Width(); // New height is old width
    float diffWidth = newWidth - m_destRect.Width(); // Difference between old and new width
    float diffHeight = newHeight - m_destRect.Height(); // Difference between old and new height

    // If the new width is bigger then the old or the new height is bigger
    // then the old, we need to scale down
    if (diffWidth > 0.0f || diffHeight > 0.0f)
    {
      float aspectRatio = GetAspectRatio();

      // Scale to fit screen width because the difference in width is bigger
      // then the difference in height
      if (diffWidth > diffHeight)
      {
        // Clamp to the width of the old dest rect
        newWidth = m_destRect.Width();
        newHeight *= aspectRatio;
      }
      else // Scale to fit screen height
      {
        // Clamp to the height of the old dest rect
        newHeight = m_destRect.Height();
        newWidth /= aspectRatio;
      }
    }

    // Calculate the center point of the view
    centerX = m_viewRect.x1 + m_viewRect.Width() / 2.0f;
    centerY = m_viewRect.y1 + m_viewRect.Height() / 2.0f;

    // Calculate the number of pixels we need to go in each x direction from
    // the center point
    diffX = newWidth / 2;
    // Calculate the number of pixels we need to go in each y direction from
    // the center point
    diffY = newHeight / 2;
  }

  for (int destIdx = 0, srcIdx = pointOffset; destIdx < 4; destIdx++)
  {
    m_rotatedDestCoords[destIdx].x = origMat[srcIdx][0];
    m_rotatedDestCoords[destIdx].y = origMat[srcIdx][1];

    if (changeAspect)
    {
      switch (srcIdx)
      {
      case 0:// top left
        m_rotatedDestCoords[destIdx].x = centerX - diffX;
        m_rotatedDestCoords[destIdx].y = centerY - diffY;
        break;
      case 1:// top right
        m_rotatedDestCoords[destIdx].x = centerX + diffX;
        m_rotatedDestCoords[destIdx].y = centerY - diffY;
        break;
      case 2:// bottom right
        m_rotatedDestCoords[destIdx].x = centerX + diffX;
        m_rotatedDestCoords[destIdx].y = centerY + diffY;
        break;
      case 3:// bottom left
        m_rotatedDestCoords[destIdx].x = centerX - diffX;
        m_rotatedDestCoords[destIdx].y = centerY + diffY;
        break;
      }
    }
    srcIdx++;
    srcIdx = srcIdx % 4;
  }
}

void CRPBaseRenderer::CalcNormalRenderRect(float offsetX, float offsetY, float width, float height, float inputFrameRatio, float zoomAmount)
{
  // If view window is empty, set empty destination
  if (height == 0 || width == 0)
  {
    m_destRect.SetRect(0.0f, 0.0f, 0.0f, 0.0f);
    return;
  }

  // Scale up image as much as possible and keep the aspect ratio (introduces
  // with black bars)
  // Calculate the correct output frame ratio (using the users pixel ratio
  // setting and the output pixel ratio setting)
  float outputFrameRatio = inputFrameRatio / g_graphicsContext.GetResInfo().fPixelRatio;

  // Allow a certain error to maximize size of render area
  float fCorrection = width / height / outputFrameRatio - 1.0f;
  float fAllowed = GetAllowedErrorInAspect();

  if (fCorrection > fAllowed)
    fCorrection = fAllowed;

  if (fCorrection < -fAllowed)
    fCorrection = -fAllowed;

  outputFrameRatio *= 1.0f + fCorrection;

  // Maximize the game width
  float newWidth = width;
  float newHeight = newWidth / outputFrameRatio;

  if (newHeight > height)
  {
    newHeight = height;
    newWidth = newHeight * outputFrameRatio;
  }

  // Scale the game up by set zoom amount
  newWidth *= zoomAmount;
  newHeight *= zoomAmount;

  // If we are less than one pixel off use the complete screen instead
  if (std::abs(newWidth - width) < 1.0f)
    newWidth = width;
  if (std::abs(newHeight - height) < 1.0f)
    newHeight = height;

  // Center the game
  float posY = (height - newHeight) / 2;
  float posX = (width - newWidth) / 2;

  const float verticalShift = 0.0f; //! @todo

  // Vertical shift range -1 to 1 shifts within the top and bottom black bars
  // If there are no top and bottom black bars, this range does nothing
  float blackBarSize = std::max((height - newHeight) / 2.0f, 0.0f);
  posY += blackBarSize * std::max(std::min(verticalShift, 1.0f), -1.0f);

  // Vertical shift ranges -2 to -1 and 1 to 2 will shift the image out of the screen
  // If vertical shift is -2 it will be completely shifted out the top,
  // if it's 2 it will be completely shifted out the bottom
  float shiftRange = std::min(newHeight, newHeight - (newHeight - height) / 2.0f);
  if (verticalShift > 1.0f)
    posY += shiftRange * (verticalShift - 1.0f);
  else if (verticalShift < -1.0f)
    posY += shiftRange * (verticalShift + 1.0f);

  m_destRect.x1 = static_cast<float>(MathUtils::round_int(posX + offsetX));
  m_destRect.x2 = m_destRect.x1 + MathUtils::round_int(newWidth);
  m_destRect.y1 = static_cast<float>(MathUtils::round_int(posY + offsetY));
  m_destRect.y2 = m_destRect.y1 + MathUtils::round_int(newHeight);

  // Clip as needed
  if (!(g_graphicsContext.IsFullScreenVideo() || g_graphicsContext.IsCalibrating()))
  {
    CRect original(m_destRect);
    m_destRect.Intersect(CRect(offsetX, offsetY, offsetX + width, offsetY + height));
    if (m_destRect != original)
    {
      float scaleX = m_sourceRect.Width() / original.Width();
      float scaleY = m_sourceRect.Height() / original.Height();
      m_sourceRect.x1 += (m_destRect.x1 - original.x1) * scaleX;
      m_sourceRect.y1 += (m_destRect.y1 - original.y1) * scaleY;
      m_sourceRect.x2 += (m_destRect.x2 - original.x2) * scaleX;
      m_sourceRect.y2 += (m_destRect.y2 - original.y2) * scaleY;
    }
  }

  if (m_oldDestRect != m_destRect || m_oldRenderOrientation != m_renderOrientation)
  {
    // Adapt the drawing rect points if we have to rotate and either destrect
    // or orientation changed
    ReorderDrawPoints();
    m_oldDestRect = m_destRect;
    m_oldRenderOrientation = m_renderOrientation;
  }
}

//***************************************************************************************
// CalculateFrameAspectRatio()
//
// Considers the source frame size and output frame size (as suggested by mplayer)
// to determine if the pixels in the source are not square.  It calculates the aspect
// ratio of the output frame.  We consider the cases of VCD, SVCD and DVD separately,
// as these are intended to be viewed on a non-square pixel TV set, so the pixels are
// defined to be the same ratio as the intended display pixels.
// These formats are determined by frame size.
//***************************************************************************************
void CRPBaseRenderer::CalculateFrameAspectRatio(unsigned int desired_width, unsigned int desired_height)
{
  m_sourceFrameRatio = static_cast<float>(desired_width) / desired_height;

  // Check whether mplayer has decided that the size of the video file should be changed
  // This indicates either a scaling has taken place (which we didn't ask for) or it has
  // found an aspect ratio parameter from the file, and is changing the frame size based
  // on that.
  if (m_sourceWidth == (unsigned int)desired_width && m_sourceHeight == (unsigned int)desired_height)
    return;

  // mplayer is scaling in one or both directions.  We must alter our Source Pixel Ratio
  float imageFrameRatio = static_cast<float>(m_sourceWidth) / m_sourceHeight;

  // OK, most sources will be correct now, except those that are intended
  // to be displayed on non-square pixel based output devices (ie PAL or NTSC TVs)
  // This includes VCD, SVCD, and DVD (and possibly others that we are not doing yet)
  // For this, we can base the pixel ratio on the pixel ratios of PAL and NTSC,
  // though we will need to adjust for anamorphic sources (ie those whose
  // output frame ratio is not 4:3) and for SVCDs which have 2/3rds the
  // horizontal resolution of the default NTSC or PAL frame sizes

  // The following are the defined standard ratios for PAL and NTSC pixels
  // NOTE: These aren't technically (in terms of BT601) correct - the commented values are,
  //       but it seems that many DVDs nowadays are mastered incorrectly, so two wrongs
  //       may indeed make a right.  The "wrong" values here ensure the output frame is
  //       4x3 (or 16x9)
  const float PALPixelRatio = 16.0f / 15.0f;      // 128.0f / 117.0f;
  const float NTSCPixelRatio = 8.0f / 9.0f;       // 4320.0f / 4739.0f;

                                                  // Calculate the correction needed for anamorphic sources
  float Non4by3Correction = m_sourceFrameRatio / (4.0f / 3.0f);

  // Finally, check for a VCD, SVCD or DVD frame size as these need special aspect ratios
  if (m_sourceWidth == 352)
  {
    // VCD?
    if (m_sourceHeight == 240) // NTSC
      m_sourceFrameRatio = imageFrameRatio * NTSCPixelRatio;
    if (m_sourceHeight == 288) // PAL
      m_sourceFrameRatio = imageFrameRatio * PALPixelRatio;
  }

  if (m_sourceWidth == 480)
  {
    // SVCD?
    if (m_sourceHeight == 480) // NTSC
      m_sourceFrameRatio = imageFrameRatio * 3.0f / 2.0f * NTSCPixelRatio * Non4by3Correction;
    if (m_sourceHeight == 576) // PAL
      m_sourceFrameRatio = imageFrameRatio * 3.0f / 2.0f * PALPixelRatio * Non4by3Correction;
  }

  if (m_sourceWidth == 720)
  {
    // DVD?
    if (m_sourceHeight == 480) // NTSC
      m_sourceFrameRatio = imageFrameRatio * NTSCPixelRatio * Non4by3Correction;
    if (m_sourceHeight == 576) // PAL
      m_sourceFrameRatio = imageFrameRatio * PALPixelRatio * Non4by3Correction;
  }
}

void CRPBaseRenderer::ManageRenderArea()
{
  m_viewRect = g_graphicsContext.GetViewWindow();

  m_sourceRect.x1 = 0.0f;
  m_sourceRect.y1 = 0.0f;
  m_sourceRect.x2 = static_cast<float>(m_sourceWidth);
  m_sourceRect.y2 = static_cast<float>(m_sourceHeight);

  CalcNormalRenderRect(m_viewRect.x1, m_viewRect.y1, m_viewRect.Width(), m_viewRect.Height(), GetAspectRatio() * m_pixelRatio, m_zoomAmount);
}

void CRPBaseRenderer::MarkDirty()
{
  g_windowManager.MarkDirty(m_destRect);
}

float CRPBaseRenderer::GetAllowedErrorInAspect() const
{
  return CServiceBroker::GetSettings().GetInt(CSettings::SETTING_VIDEOPLAYER_ERRORINASPECT) * 0.01f;
}

void CRPBaseRenderer::UpdateVideoShaders()
{
  if (m_shadersNeedUpdate)
  {
    m_shadersNeedUpdate = false;

    if (m_shaderPreset)
    {
      auto sourceWidth = static_cast<unsigned>(m_sourceRect.Width());
      auto sourceHeight = static_cast<unsigned>(m_sourceRect.Height());

      // We need to set this here because m_sourceRect isn't valid on init/pre-init
      m_shaderPreset->SetVideoSize(sourceWidth, sourceHeight);
      m_bUseShaderPreset = m_shaderPreset->SetShaderPreset(m_shaderPresetPath);
    }
  }
}
