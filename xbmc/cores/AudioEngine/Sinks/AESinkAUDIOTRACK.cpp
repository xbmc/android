 /*
 *      Copyright (C) 2010-2012 Team XBMC
 *      http://www.xbmc.org
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
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "system.h"

#include "AESinkAUDIOTRACK.h"
#include "DynamicDll.h"
#include "Utils/AEUtil.h"
#include "Utils/AERingBuffer.h"
#include "utils/log.h"

#include <stdint.h>
#include <limits.h>

#include <libaudiotrack_wrapper.h>

#define USE_BUFFER
////////////////////////////////////////////////////////////////////////////////////////////
class DllLibAudioTrackInterface
{
public:
  virtual ~DllLibAudioTrackInterface() {}

  virtual ATW_ctx ATW_create()=0;
  virtual void    ATW_release(ATW_ctx ctx)=0;
  virtual int     ATW_get_min_framecount(ATW_ctx ctx, int* frameCount, int sampleRate)=0;
  virtual int     ATW_set(  ATW_ctx ctx, int channelCount, int sampleRate, int frameCount,
                    ATWCallback cbf, void* cbf_ctx)=0;
  virtual void    ATW_start(ATW_ctx ctx)=0;
  virtual void    ATW_stop( ATW_ctx ctx)=0;
  virtual int     ATW_write(ATW_ctx ctx, const void* buffer, size_t size)=0;
  virtual void    ATW_flush(ATW_ctx ctx)=0;
  virtual void    ATW_pause(ATW_ctx ctx)=0;
  virtual void    ATW_mute( ATW_ctx ctx, int mute)=0;
  virtual int     ATW_muted(ATW_ctx ctx)=0;
  virtual void    ATW_set_volume(ATW_ctx ctx, float left, float right)=0;
  virtual int     ATW_latency(ATW_ctx ctx)=0;
};

class DllLibAudioTrack : public DllDynamic, DllLibAudioTrackInterface
{
  DECLARE_DLL_WRAPPER(DllLibAudioTrack, "libaudiotrack_wrapper.so")

  DEFINE_METHOD0(ATW_ctx, ATW_create)
  DEFINE_METHOD1(void,    ATW_release,      (ATW_ctx p1))
  DEFINE_METHOD3(int,     ATW_get_min_framecount,(ATW_ctx p1, int *p2, int p3))
  DEFINE_METHOD6(int,     ATW_set,          (ATW_ctx p1, int p2, int p3, int p4,
                                              ATWCallback p5, void *p6))
  DEFINE_METHOD1(void,    ATW_start,        (ATW_ctx p1))
  DEFINE_METHOD1(void,    ATW_stop,         (ATW_ctx p1))
  DEFINE_METHOD3(int,     ATW_write,        (ATW_ctx p1, const void* p2, size_t p3))
  DEFINE_METHOD1(void,    ATW_flush,        (ATW_ctx p1))
  DEFINE_METHOD1(void,    ATW_pause,        (ATW_ctx p1))
  DEFINE_METHOD2(void,    ATW_mute,         (ATW_ctx p1, int p2))
  DEFINE_METHOD1(int,     ATW_muted,        (ATW_ctx p1))
  DEFINE_METHOD3(void,    ATW_set_volume,   (ATW_ctx p1, float p2, float p3))
  DEFINE_METHOD1(int,     ATW_latency,      (ATW_ctx p1))

  BEGIN_METHOD_RESOLVE()
    RESOLVE_METHOD(ATW_create)
    RESOLVE_METHOD(ATW_release)
    RESOLVE_METHOD(ATW_get_min_framecount)
    RESOLVE_METHOD(ATW_set)
    RESOLVE_METHOD(ATW_start)
    RESOLVE_METHOD(ATW_stop)
    RESOLVE_METHOD(ATW_write)
    RESOLVE_METHOD(ATW_flush)
    RESOLVE_METHOD(ATW_pause)
    RESOLVE_METHOD(ATW_mute)
    RESOLVE_METHOD(ATW_muted)
    RESOLVE_METHOD(ATW_set_volume)
    RESOLVE_METHOD(ATW_latency)
  END_METHOD_RESOLVE()
};

DllLibAudioTrack *CAESinkAUDIOTRACK::m_dll = NULL;

CAESinkAUDIOTRACK::CAESinkAUDIOTRACK()
{
  CLog::Log(LOGDEBUG, "CAESinkAUDIOTRACK::CAESinkAUDIOTRACK");
  if (!m_dll)
  {
    m_dll = new DllLibAudioTrack;
    m_dll->Load();
  }
  m_buffer = NULL;
  m_ATWrapper = m_dll->ATW_create();
}

CAESinkAUDIOTRACK::~CAESinkAUDIOTRACK()
{
  CLog::Log(LOGDEBUG, "CAESinkAUDIOTRACK::~CAESinkAUDIOTRACK");
  //delete m_dll, m_dll = NULL;
}

bool CAESinkAUDIOTRACK::Initialize(AEAudioFormat &format, std::string &device)
{
  CLog::Log(LOGDEBUG, "CAESinkAUDIOTRACK::Initialize");
  if (!m_ATWrapper)
    return false;

  //format.m_sampleRate = (format.m_sampleRate = 44100) ? 44100 : 48000;
  format.m_sampleRate = 48000;
  format.m_dataFormat = AE_FMT_S16LE;

  CLog::Log(LOGDEBUG, "CAESinkAUDIOTRACK::Initialize2");
  int min_frames;
  m_dll->ATW_get_min_framecount(m_ATWrapper, &min_frames, format.m_sampleRate);

  CLog::Log(LOGDEBUG, "CAESinkAUDIOTRACK::Initialize3, min_frames(%d)", min_frames);
  format.m_frames         = min_frames;
  format.m_frameSamples   = format.m_channelLayout.Count();
  format.m_frameSize      = format.m_frameSamples * (CAEUtil::DataFormatToBits(format.m_dataFormat) >> 3);
#ifdef USE_BUFFER
  m_dll->ATW_set(m_ATWrapper, format.m_frameSamples, format.m_sampleRate, format.m_frames,
    AudioTrackCallback, this);
#else
  m_dll->ATW_set(m_ATWrapper, format.m_frameSamples, format.m_sampleRate, format.m_frames,
    NULL, NULL);
#endif

  m_format = format;
  m_started = false;
  m_draining = false;
  m_SecondsPerByte = 1.0 / (double)(m_format.m_sampleRate * m_format.m_frameSize);
#ifdef USE_BUFFER
  m_buffer = new AERingBuffer(m_format.m_frameSize * m_format.m_sampleRate);
#endif

  m_dll->ATW_start(m_ATWrapper), m_started = true;

  return true;
}

void CAESinkAUDIOTRACK::Deinitialize()
{
  CLog::Log(LOGDEBUG, "CAESinkAUDIOTRACK::Deinitialize");
  m_dll->ATW_stop(m_ATWrapper), m_started = false;
  m_dll->ATW_release(m_ATWrapper), m_ATWrapper = NULL;
  delete m_buffer, m_buffer = NULL;
}

bool CAESinkAUDIOTRACK::IsCompatible(const AEAudioFormat format, const std::string device)
{
  AEAudioFormat in_format = format;
  CLog::Log(LOGDEBUG, "CAESinkAUDIOTRACK::IsCompatible");
  CLog::Log(LOGDEBUG, "  is Output Device : %s", device.c_str());
  CLog::Log(LOGDEBUG, "  is Sample Rate   : %d", m_format.m_sampleRate);
  CLog::Log(LOGDEBUG, "  is Sample Format : %s", CAEUtil::DataFormatToStr(m_format.m_dataFormat));
  CLog::Log(LOGDEBUG, "  is Channel Count : %d", m_format.m_channelLayout.Count());
  CLog::Log(LOGDEBUG, "  is Channel Layout: %s", ((std::string)m_format.m_channelLayout).c_str());
  CLog::Log(LOGDEBUG, "  is Frames        : %d", m_format.m_frames);
  CLog::Log(LOGDEBUG, "  is Frame Samples : %d", m_format.m_frameSamples);
  CLog::Log(LOGDEBUG, "  is Frame Size    : %d", m_format.m_frameSize);

  CLog::Log(LOGDEBUG, "  in Output Device : %s", device.c_str());
  CLog::Log(LOGDEBUG, "  in Sample Rate   : %d", in_format.m_sampleRate);
  CLog::Log(LOGDEBUG, "  in Sample Format : %s", CAEUtil::DataFormatToStr(in_format.m_dataFormat));
  CLog::Log(LOGDEBUG, "  in Channel Count : %d", in_format.m_channelLayout.Count());
  CLog::Log(LOGDEBUG, "  is Channel Layout: %s", ((std::string)in_format.m_channelLayout).c_str());
  CLog::Log(LOGDEBUG, "  in Frames        : %d", in_format.m_frames);
  CLog::Log(LOGDEBUG, "  in Frame Samples : %d", in_format.m_frameSamples);
  CLog::Log(LOGDEBUG, "  in Frame Size    : %d", in_format.m_frameSize);

  return ((m_format.m_sampleRate    == format.m_sampleRate) &&
          (m_format.m_dataFormat    == format.m_dataFormat) &&
          (m_format.m_channelLayout == format.m_channelLayout));
}

void CAESinkAUDIOTRACK::Stop()
{
  m_dll->ATW_stop(m_ATWrapper), m_started = false;
}

double CAESinkAUDIOTRACK::GetDelay()
{
#ifdef USE_BUFFER
  double latency_sec = (double)m_dll->ATW_latency(m_ATWrapper) / 1000.0;
  // returns the time it takes to play a packet if we add one at this time
  //double seconds_to_empty = m_SecondsPerByte * (double)m_buffer->GetReadSize();
  //CLog::Log(LOGDEBUG, "CAESinkAUDIOTRACK::GetDelay, latency_sec(%f), buffer_sec(%f)",
  //  latency_sec, buffer_sec);
  //return seconds_to_empty + latency_sec;
  return latency_sec;
#else
  double latency_sec = (double)m_dll->ATW_latency(m_ATWrapper) / 1000.0;
  return latency_sec;0
#endif
}

double CAESinkAUDIOTRACK::GetCacheTime()
{
  // returns total amount of data cached in audio output at this time
#ifdef USE_BUFFER
  double seconds_to_empty = m_SecondsPerByte * (double)m_buffer->GetReadSize();
  //CLog::Log(LOGDEBUG, "CAESinkAUDIOTRACK::GetCacheTime, seconds_to_empty(%f)", seconds_to_empty);
  return seconds_to_empty;
#else
  return 0;
#endif
}

double CAESinkAUDIOTRACK::GetCacheTotal()
{
  // total amount the audio sink can buffer
#ifdef USE_BUFFER
  float buffer_totalsec = (double)m_buffer->GetMaxSize() / (double)m_format.m_sampleRate;
  //CLog::Log(LOGDEBUG, "CAESinkAUDIOTRACK::GetCacheTotal, buffer_sec(%f)", buffer_totalsec);
  return buffer_totalsec;
#else
  return 0;
#endif
}

unsigned int CAESinkAUDIOTRACK::AddPackets(uint8_t *data, unsigned int frames)
{
#ifdef USE_BUFFER
  //CLog::Log(LOGDEBUG, "CAESinkAUDIOTRACK::AddPackets, frames(%d)", frames);
  unsigned int room = m_buffer->GetWriteSize();
  unsigned int addbytes = frames * m_format.m_frameSize;
  unsigned int bytes_per_second = m_format.m_frameSize * m_format.m_sampleRate;

  if (m_draining)
    return frames;

  if (!m_started)
    m_dll->ATW_start(m_ATWrapper), m_started = true;
 
  unsigned int total_ms_sleep = 0;
  while (!m_draining && addbytes > room && total_ms_sleep < 200)
  {
    // we got deleted
    if (!m_buffer || m_draining )
      return 0;

    unsigned int ms_sleep_time = (1000 * room) / bytes_per_second;
    if (ms_sleep_time == 0)
      ms_sleep_time++;

    // sleep until we have space (estimated) or 1ms min
    Sleep(ms_sleep_time);
    total_ms_sleep += ms_sleep_time;

    room = m_buffer->GetWriteSize();
  }

  if (addbytes > room)
    frames = 0;
  else
    m_buffer->Write(data, addbytes);

  // ms sleep block
  Sleep(m_dll->ATW_latency(m_ATWrapper)/2);

  return frames;
#else
  m_dll->ATW_write(m_ATWrapper, data, frames * m_format.m_frameSize);
  return frames;
#endif
}

void CAESinkAUDIOTRACK::Drain()
{
  CLog::Log(LOGDEBUG, "CAESinkAUDIOTRACK::Drain");
  m_draining = true;
  m_dll->ATW_stop( m_ATWrapper), m_started = false;
  m_dll->ATW_flush(m_ATWrapper);
}

void CAESinkAUDIOTRACK::EnumerateDevicesEx(AEDeviceInfoList &list)
{
  CAEDeviceInfo info;

  info.m_channels.Reset();
  info.m_dataFormats.clear();
  info.m_sampleRates.clear();

  info.m_deviceType = AE_DEVTYPE_PCM;
  info.m_deviceName = "AudioTrack";
  info.m_displayName = "android";
  info.m_displayNameExtra = "default display";
  info.m_channels += AE_CH_FL;
  info.m_channels += AE_CH_FR;
  //info.m_sampleRates.push_back(44100);
  info.m_sampleRates.push_back(48000);
  info.m_dataFormats.push_back(AE_FMT_S16LE);

  list.push_back(info);
}

int CAESinkAUDIOTRACK::AudioTrackCallback(void *ctx, void *buffer, size_t size)
{
  CAESinkAUDIOTRACK *at_ctx = (CAESinkAUDIOTRACK*)ctx;
  unsigned int readsize = std::min(at_ctx->m_buffer->GetReadSize(), size);
  at_ctx->m_buffer->Read((unsigned char*)buffer, readsize);
  return readsize;
}
