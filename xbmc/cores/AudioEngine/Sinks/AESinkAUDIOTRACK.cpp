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

#include "AESinkAUDIOTRACK.h"
#include "DynamicDll.h"
#include "Utils/AEUtil.h"
#include "Utils/AERingBuffer.h"
#include "android/activity/XBMCApp.h"
#include "utils/log.h"

#include <jni.h>

static jint GetStaticIntField(JNIEnv *jenv, std::string class_name, std::string field_name)
{
  class_name.insert(0, "android/media/");
  jclass cls = jenv->FindClass(class_name.c_str());
  jfieldID field = jenv->GetStaticFieldID(cls, field_name.c_str(), "I");
  jint int_field = jenv->GetStaticIntField(cls, field);
  jenv->DeleteLocalRef(cls);
  return int_field;
}

CAEDeviceInfo CAESinkAUDIOTRACK::m_info;
////////////////////////////////////////////////////////////////////////////////////////////
CAESinkAUDIOTRACK::CAESinkAUDIOTRACK()
  : CThread("audiotrack")
{
  m_buffer = NULL;

  CLog::Log(LOGDEBUG, "CAESinkAUDIOTRACK::CAESinkAUDIOTRACK");
}

CAESinkAUDIOTRACK::~CAESinkAUDIOTRACK()
{
  CLog::Log(LOGDEBUG, "CAESinkAUDIOTRACK::~CAESinkAUDIOTRACK");
}

bool CAESinkAUDIOTRACK::Initialize(AEAudioFormat &format, std::string &device)
{
  CLog::Log(LOGDEBUG, "CAESinkAUDIOTRACK::Initialize");

  m_format = format;
  // default to 44100, all android devices support it.
  // then check if we can support the requested rate.
  unsigned int sampleRate = 44100;
  for (size_t i = 0; i < m_info.m_sampleRates.size(); i++)
  {
    if (m_format.m_sampleRate == m_info.m_sampleRates[i])
    {
      sampleRate = format.m_sampleRate;
      break;
    }
  }
  m_format.m_sampleRate   = sampleRate;
  m_format.m_dataFormat   = AE_FMT_S16LE;
  m_format.m_frameSamples = m_format.m_channelLayout.Count();
  m_format.m_frameSize    = m_format.m_frameSamples * (CAEUtil::DataFormatToBits(m_format.m_dataFormat) >> 3);

  // launch the process thread and wait for the
  // AutoTrack jni object to get created and setup.
  m_inited.Reset();
  Create();
  if(!m_inited.WaitMSec(100))
  {
    while(!m_inited.WaitMSec(1))
      sleep(10);
  }

  // m_init_frames is volatile and has been setup by Process()
  m_format.m_frames = m_init_frames;

  m_SecondsPerByte = 1.0 / (double)(m_format.m_frameSize * m_format.m_sampleRate);
  m_buffer = new AERingBuffer(m_format.m_frameSize * m_format.m_sampleRate);

  CLog::Log(LOGDEBUG, "CAESinkAUDIOTRACK::Initialize, "
    "m_SecondsPerByte(%f), m_buffer->GetMaxSize(%d), m_buffer_sec(%f), m_latency_sec(%f)",
    m_SecondsPerByte, m_buffer->GetMaxSize(), m_buffer_sec, m_latency_sec);

  format = m_format;

  return true;
}

void CAESinkAUDIOTRACK::Deinitialize()
{
  CLog::Log(LOGDEBUG, "CAESinkAUDIOTRACK::Deinitialize");
  StopThread();
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

  CLog::Log(LOGDEBUG, "  in Output Device : %s", device.c_str());
  CLog::Log(LOGDEBUG, "  in Sample Rate   : %d", in_format.m_sampleRate);
  CLog::Log(LOGDEBUG, "  in Sample Format : %s", CAEUtil::DataFormatToStr(in_format.m_dataFormat));
  CLog::Log(LOGDEBUG, "  in Channel Count : %d", in_format.m_channelLayout.Count());
  CLog::Log(LOGDEBUG, "  is Channel Layout: %s", ((std::string)in_format.m_channelLayout).c_str());

  return ((m_format.m_sampleRate    == format.m_sampleRate) &&
          (m_format.m_dataFormat    == format.m_dataFormat) &&
          (m_format.m_channelLayout == format.m_channelLayout));
}

double CAESinkAUDIOTRACK::GetDelay()
{
  // this includes any latency due to AudioTrack buffer size,
  // AudioMixer (if any) and audio hardware driver.
  double seconds_to_empty = m_SecondsPerByte * (double)m_buffer->GetReadSize();
  //CLog::Log(LOGDEBUG, "CAESinkAUDIOTRACK::GetDelay, seconds_to_empty(%f), m_latency_sec(%f)",
  //  seconds_to_empty, m_latency_sec);
  return seconds_to_empty + m_latency_sec;
}

double CAESinkAUDIOTRACK::GetCacheTime()
{
  // returns the time in seconds that it will take
  // to underrun the buffer if no sample is added.

  double seconds_to_empty = m_SecondsPerByte * (double)m_buffer->GetReadSize();
  //CLog::Log(LOGDEBUG, "CAESinkAUDIOTRACK::GetCacheTime, seconds_to_empty(%f)",
  //  seconds_to_empty);
  return seconds_to_empty;
}

double CAESinkAUDIOTRACK::GetCacheTotal()
{
  // total amount that the audio sink can buffer in units of seconds
  double buffer_totalsec = (double)m_buffer->GetMaxSize() / (double)m_format.m_sampleRate;
  //CLog::Log(LOGDEBUG, "CAESinkAUDIOTRACK::GetCacheTotal, buffer_totalsec(%f)",
  //  buffer_totalsec);
  return buffer_totalsec;
}

unsigned int CAESinkAUDIOTRACK::AddPackets(uint8_t *data, unsigned int frames)
{
  unsigned int room = m_buffer->GetWriteSize();
  unsigned int addbytes = frames * m_format.m_frameSize;
  unsigned int bytes_per_second = m_format.m_frameSize * m_format.m_sampleRate;

  if (m_draining)
    return frames;

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

  return frames;
}

void CAESinkAUDIOTRACK::Drain()
{
  CLog::Log(LOGDEBUG, "CAESinkAUDIOTRACK::Drain");
  m_draining = true;
}

void CAESinkAUDIOTRACK::EnumerateDevicesEx(AEDeviceInfoList &list)
{
  m_info.m_channels.Reset();
  m_info.m_dataFormats.clear();
  m_info.m_sampleRates.clear();

  m_info.m_deviceType = AE_DEVTYPE_PCM;
  m_info.m_deviceName = "AudioTrack";
  m_info.m_displayName = "android";
  m_info.m_displayNameExtra = "audiotrack";
  m_info.m_channels += AE_CH_FL;
  m_info.m_channels += AE_CH_FR;
  m_info.m_sampleRates.push_back(44100);
  m_info.m_sampleRates.push_back(48000);
  m_info.m_dataFormats.push_back(AE_FMT_S16LE);

  list.push_back(m_info);
}

void CAESinkAUDIOTRACK::ProbeSupportedSampleRates()
{
  m_info.m_sampleRates.push_back(44100);
  m_info.m_sampleRates.push_back(48000);
}

void CAESinkAUDIOTRACK::OnStartup()
{
}

void CAESinkAUDIOTRACK::OnExit()
{
}

void CAESinkAUDIOTRACK::Process()
{
  CLog::Log(LOGDEBUG, "CAESinkAUDIOTRACK::Process");
  JNIEnv *jenv = NULL;

  CXBMCApp::AttachCurrentThread(&jenv, NULL);
  jenv->PushLocalFrame(4);

  jclass cls = jenv->FindClass("android/media/AudioTrack");
  jclass jcAudioTrack = static_cast<jclass>(jenv->NewGlobalRef(cls));
  jenv->DeleteLocalRef(cls);

  jmethodID jmInit              = jenv->GetMethodID(jcAudioTrack, "<init>", "(IIIIII)V");
  jmethodID jmPlay              = jenv->GetMethodID(jcAudioTrack, "play", "()V");
  //jmethodID jmPause           = jenv->GetMethodID(jcAudioTrack, "pause", "()V");
  jmethodID jmStop              = jenv->GetMethodID(jcAudioTrack, "stop", "()V");
  jmethodID jmRelease           = jenv->GetMethodID(jcAudioTrack, "release", "()V");
  jmethodID jmWrite             = jenv->GetMethodID(jcAudioTrack, "write", "([BII)I");
  //jmethodID jmGetPos          = jenv->GetMethodID(jcAudioTrack, "getPlaybackHeadPosition", "()I");
  jmethodID jmGetMinBufferSize  = jenv->GetStaticMethodID(jcAudioTrack, "getMinBufferSize", "(III)I");

  jint channelConfig;
  if (m_format.m_frameSamples == 1)
    channelConfig = GetStaticIntField(jenv, "AudioFormat", "CHANNEL_OUT_MONO");
  else
    channelConfig = GetStaticIntField(jenv, "AudioFormat", "CHANNEL_OUT_STEREO");

  jint audioFormat;
  if (CAEUtil::DataFormatToBits(m_format.m_dataFormat) == 8)
    audioFormat = GetStaticIntField(jenv, "AudioFormat", "ENCODING_PCM_8BIT");
  else
    audioFormat = GetStaticIntField(jenv, "AudioFormat", "ENCODING_PCM_16BIT");

  jint sampleRate = m_format.m_sampleRate;

  jint min_buffer_size = jenv->CallStaticIntMethod(jcAudioTrack, jmGetMinBufferSize,
    sampleRate, channelConfig, audioFormat);

  CLog::Log(LOGDEBUG, "CAESinkAUDIOTRACK::Process:jmGetMinBufferSize, "
    "min_buffer_size(%d), sampleRate(%d), channelConfig(%d), audioFormat(%d)",
    min_buffer_size, sampleRate, channelConfig, audioFormat);

  // double it get better performance
  min_buffer_size *= 2;
  m_init_frames = min_buffer_size / m_format.m_frameSize;
  m_format.m_frames = m_init_frames;

  m_buffer_sec  = (double)m_init_frames / m_format.m_sampleRate;
  // we cannot get actual latency using jni so guess
  m_latency_sec = 0.100;
  CLog::Log(LOGDEBUG, "CAESinkAUDIOTRACK::Process, m_init_frames(%d), m_buffer_sec(%f), m_latency_sec(%f)",
    m_init_frames, m_buffer_sec, m_latency_sec);

  CLog::Log(LOGDEBUG, "CAESinkAUDIOTRACK::Process3");
  jobject joAudioTrack = jenv->NewObject(jcAudioTrack, jmInit,
    GetStaticIntField(jenv, "AudioManager", "STREAM_MUSIC"),
    sampleRate,
    channelConfig,
    audioFormat,
    min_buffer_size,
    GetStaticIntField(jenv, "AudioTrack", "MODE_STREAM"));

  CLog::Log(LOGDEBUG, "CAESinkAUDIOTRACK::Process4, joAudioTrack(%p)", joAudioTrack);

  // The AudioTrack object has been created and waiting to play,
  m_inited.Set();
  // yield to give other threads a chance to do some work.
  sched_yield();

  jarray jbuffer = jenv->NewByteArray(min_buffer_size);

  m_draining = false;
  jenv->CallVoidMethod(joAudioTrack, jmPlay, m_started = true);

  while (!m_bStop)
  {
    //int bytes_to_empty = jenv->CallIntMethod(joAudioTrack, jmGetPos);
    //bytes_to_empty *= m_format.m_frameSize;

    unsigned int read_frames = m_buffer->GetReadSize() / m_format.m_frameSize;
    if (read_frames >= m_format.m_frames)
    {
      // Stream the next batch of audio data to the Java AudioTrack.
      void *pBuffer = jenv->GetPrimitiveArrayCritical(jbuffer, NULL);
      if (pBuffer)
      {
        unsigned int buffer_size = read_frames * m_format.m_frameSize;
        m_buffer->Read((unsigned char*)pBuffer, buffer_size);
        jenv->ReleasePrimitiveArrayCritical(jbuffer, pBuffer, 0);
        jenv->CallIntMethod(joAudioTrack, jmWrite, jbuffer, 0, buffer_size);
      }
      else
      {
        CLog::Log(LOGDEBUG, "Failed to get pointer to array bytes");
      }
      // yield this audio thread to give other threads a chance to do some work.
      sched_yield();
    }
    else
      Sleep(10);
  }

  jenv->CallVoidMethod(joAudioTrack, jmStop), m_started = false;
  jenv->CallVoidMethod(joAudioTrack, jmRelease);

  // might toss an exception on jmRelease so catch it.
  jthrowable exception = jenv->ExceptionOccurred();
  if (exception)
  {
    jenv->ExceptionDescribe();
    jenv->ExceptionClear();
  }

  jenv->PopLocalFrame(NULL);
  CXBMCApp::DetachCurrentThread();
}
