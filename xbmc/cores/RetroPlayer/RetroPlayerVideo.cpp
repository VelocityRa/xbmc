/*
 *      Copyright (C) 2012-2017 Team Kodi
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

#include "RetroPlayerVideo.h"
#include "RetroPlayerDefines.h"
#include "cores/VideoPlayer/DVDCodecs/Video/DVDVideoCodec.h"
#include "cores/VideoPlayer/DVDCodecs/DVDCodecUtils.h"
#include "cores/VideoPlayer/DVDCodecs/DVDFactoryCodec.h"
#include "cores/VideoPlayer/DVDDemuxers/DVDDemux.h"
#include "cores/VideoPlayer/VideoRenderers/RenderFlags.h"
#include "cores/VideoPlayer/VideoRenderers/RenderManager.h"
#include "cores/VideoPlayer/DVDStreamInfo.h"
#include "cores/VideoPlayer/TimingConstants.h"
#include "utils/log.h"

#include <atomic> //! @todo
#include <cstring>

using namespace KODI;
using namespace RETRO;

//------------------------------------------------------------------------------
// Video Buffers
//------------------------------------------------------------------------------

namespace KODI
{
namespace RETRO
{

class CPixelBufferRGB : public CVideoBuffer
{
public:
  CPixelBufferRGB(IVideoBufferPool &pool, int id);
  ~CPixelBufferRGB() override;
  virtual void GetPlanes(uint8_t*(&planes)[YuvImage::MAX_PLANES]) override;
  virtual void GetStrides(int(&strides)[YuvImage::MAX_PLANES]) override;

  void SetRef(uint8_t *frame, size_t size, unsigned int height, AVPixelFormat format);
  void Unref();

protected:
  uint8_t *m_pFrame = nullptr;
  size_t m_stride = 0;
};

CPixelBufferRGB::CPixelBufferRGB(IVideoBufferPool &pool, int id) :
  CVideoBuffer(id)
{
}

CPixelBufferRGB::~CPixelBufferRGB()
{
  delete[] m_pFrame;
}

void CPixelBufferRGB::GetPlanes(uint8_t*(&planes)[YuvImage::MAX_PLANES])
{
  planes[0] = m_pFrame;
}

void CPixelBufferRGB::GetStrides(int(&strides)[YuvImage::MAX_PLANES])
{
  strides[0] = m_stride;
}

void CPixelBufferRGB::SetRef(uint8_t *frame, size_t size, unsigned int height, AVPixelFormat format)
{
  delete[] m_pFrame;

  m_pFrame = frame;
  m_stride = size / height;
  m_pixFormat = format;
}

void CPixelBufferRGB::Unref()
{
  delete[] m_pFrame;
  m_pFrame = nullptr;
}

//------------------------------------------------------------------------------

class CPixelBufferPoolRGB : public IVideoBufferPool
{
public:
  ~CPixelBufferPoolRGB() override;
  virtual void Return(int id) override;
  virtual CVideoBuffer* Get() override;

protected:
  CCriticalSection m_critSection;
  std::vector<CPixelBufferRGB*> m_all;
  std::deque<int> m_used;
  std::deque<int> m_free;
};

CPixelBufferPoolRGB::~CPixelBufferPoolRGB()
{
  for (auto buf : m_all)
    delete buf;
}

CVideoBuffer* CPixelBufferPoolRGB::Get()
{
  CSingleLock lock(m_critSection);

  CPixelBufferRGB *buf = nullptr;
  if (!m_free.empty())
  {
    int idx = m_free.front();
    m_free.pop_front();
    m_used.push_back(idx);
    buf = m_all[idx];
  }
  else
  {
    int id = m_all.size();
    buf = new CPixelBufferRGB(*this, id);
    m_all.push_back(buf);
    m_used.push_back(id);
  }

  buf->Acquire(GetPtr());
  return buf;
}

void CPixelBufferPoolRGB::Return(int id)
{
  CSingleLock lock(m_critSection);

  m_all[id]->Unref();
  auto it = m_used.begin();
  while (it != m_used.end())
  {
    if (*it == id)
    {
      m_used.erase(it);
      break;
    }
    else
      ++it;
  }
  m_free.push_back(id);
}

} // namespace RETRO
} // namespace KODI

//------------------------------------------------------------------------------
// main class
//------------------------------------------------------------------------------

CRetroPlayerVideo::CRetroPlayerVideo(CRenderManager& renderManager, CProcessInfo& processInfo, CDVDClock &clock) :
  //CThread("RetroPlayerVideo"),
  m_renderManager(renderManager),
  m_processInfo(processInfo),
  m_clock(clock),
  m_framerate(0.0),
  m_orientation(0),
  m_bConfigured(false),
  m_droppedFrames(0),
  m_pixelBufferPoolRGB(std::make_shared<CPixelBufferPoolRGB>())
{
  m_renderManager.PreInit();
}

CRetroPlayerVideo::~CRetroPlayerVideo()
{
  CloseStream();
  m_renderManager.UnInit();
}

bool CRetroPlayerVideo::OpenPixelStream(AVPixelFormat pixfmt, unsigned int width, unsigned int height, double framerate, unsigned int orientationDeg)
{
  CLog::Log(LOGINFO, "RetroPlayerVideo: Creating video stream with pixel format: %i, %dx%d", pixfmt, width, height);

  m_format = pixfmt;
  m_width = width;
  m_height = height;
  m_framerate = framerate;
  m_orientation = orientationDeg;
  m_bConfigured = false;
  m_droppedFrames = 0;

  //! @todo
  //m_processInfo.SetVideoPixelFormat(CDVDVideoCodecFFmpeg::GetPixelFormatName(pixfmt));
  m_processInfo.SetVideoDimensions(width, height);
  m_processInfo.SetVideoFps(static_cast<float>(framerate));
  return true;
}

bool CRetroPlayerVideo::OpenEncodedStream(AVCodecID codec)
{
  CDemuxStreamVideo videoStream;

  // Stream
  videoStream.uniqueId = GAME_STREAM_VIDEO_ID;
  videoStream.codec = codec;
  videoStream.type = STREAM_VIDEO;
  videoStream.source = STREAM_SOURCE_DEMUX;
  videoStream.realtime = true;

  // Video
  //! @todo Needed?
  /*
  videoStream.iFpsScale = 1000;
  videoStream.iFpsRate = static_cast<int>(framerate * 1000);
  videoStream.iHeight = height;
  videoStream.iWidth = width;
  videoStream.fAspect = static_cast<float>(width) / static_cast<float>(height);
  videoStream.iOrientation = orientationDeg;
  */

  CDVDStreamInfo hint(videoStream);
  // FIXME
  //m_pVideoCodec.reset(CDVDFactoryCodec::CreateVideoCodec(hint, m_processInfo, m_renderManager.GetRenderInfo()));

  return m_pVideoCodec.get() != nullptr;
}

void CRetroPlayerVideo::AddData(const uint8_t* data, unsigned int size)
{
  VideoPicture picture;

  if (GetPicture(data, size, picture))
  {
    if (!Configure(picture))
    {
      CLog::Log(LOGERROR, "RetroPlayerVideo: Failed to configure renderer");
      CloseStream();
    }
    else
    {
      SendPicture(picture);
    }
  }
}

void CRetroPlayerVideo::CloseStream()
{
  m_renderManager.Flush(true);
  m_pVideoCodec.reset();
}

bool CRetroPlayerVideo::Configure(VideoPicture& picture)
{
  if (!m_bConfigured)
  {
    // Determine RenderManager flags
    unsigned int flags = CONF_FLAGS_YUVCOEF_BT601 | // color_matrix = 4
                         CONF_FLAGS_FULLSCREEN;     // Allow fullscreen

    const int buffers = 1; //! @todo

    m_bConfigured = m_renderManager.Configure(picture, static_cast<float>(m_framerate), flags, m_orientation, buffers);

    if (m_bConfigured)
    {
      // Update process info
      m_processInfo.SetVideoDimensions(picture.iWidth, picture.iHeight);
      m_processInfo.SetVideoFps(static_cast<float>(m_framerate));
    }
  }

  return m_bConfigured;
}

bool CRetroPlayerVideo::GetPicture(const uint8_t* data, unsigned int size, VideoPicture& picture)
{
  bool bHasPicture = false;

  if (data != nullptr && size != 0)
  {
    int lateframes;
    double renderPts;
    int queued, discard;
    m_renderManager.GetStats(lateframes, renderPts, queued, discard);

    // Drop frame if another is queued
    const bool bDropped = (queued > 0);

    if (!bDropped)
    {
      // Picture properties
      picture.pts = m_clock.GetClock(); // Show immediately
      picture.dts = DVD_NOPTS_VALUE;
      picture.iFlags = 0;
      picture.iDuration = DVD_SEC_TO_TIME(1.0 / m_framerate);
      picture.color_matrix = 4; // CONF_FLAGS_YUVCOEF_BT601
      picture.color_range = 0; // *not* CONF_FLAGS_YUV_FULLRANGE
      picture.iWidth = m_width;
      picture.iHeight = m_height;
      picture.iDisplayWidth = m_width; //! @todo: Update if aspect ratio changes
      picture.iDisplayHeight = m_height;

      // Video buffer
      uint8_t *dataCopy = nullptr;
      AllocateRgbBuffer(&dataCopy, size);
      std::memcpy(dataCopy, data, size);

      CPixelBufferRGB *buffer = static_cast<CPixelBufferRGB*>(m_pixelBufferPoolRGB->Get());
      buffer->SetRef(dataCopy, size, m_height, m_format);
      picture.videoBuffer = buffer;

      bHasPicture = true;
    }
  }

  return bHasPicture;
}

void CRetroPlayerVideo::SendPicture(VideoPicture& picture)
{
  std::atomic_bool bAbortOutput(false); //! @todo

  if (!m_renderManager.AddVideoPicture(picture, bAbortOutput, VS_INTERLACEMETHOD_NONE, false))
  {
    // Video device might not be done yet, drop the frame
    m_droppedFrames++;
  }
}

void CRetroPlayerVideo::AllocateRgbBuffer(uint8_t **pData, size_t size) const
{
  *pData = new uint8_t[size];
}
