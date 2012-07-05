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
  m_sinkbuffer = NULL;

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
  // setup a 1/4 second internal sink lockless ring buffer
  m_sinkbuffer = new AERingBuffer(m_format.m_frameSize * m_format.m_sampleRate / 4);
  m_sinkbuffer_sec = (double)m_sinkbuffer->GetMaxSize() / (double)m_format.m_sampleRate;
  m_sinkbuffer_sec_per_byte = 1.0 / (double)(m_format.m_frameSize * m_format.m_sampleRate);

  m_draining = false;
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

  CLog::Log(LOGDEBUG, "CAESinkAUDIOTRACK::Initialize, "
    "m_SecondsPerByte(%f), m_sinkbuffer->GetMaxSize(%d), m_audiotrackbuffer_sec(%f), m_audiotracklatency_sec(%f)",
    m_sinkbuffer_sec_per_byte, m_sinkbuffer->GetMaxSize(), m_audiotrackbuffer_sec, m_audiotracklatency_sec);

  format = m_format;

  return true;
}

void CAESinkAUDIOTRACK::Deinitialize()
{
  CLog::Log(LOGDEBUG, "CAESinkAUDIOTRACK::Deinitialize");
  StopThread();
  delete m_sinkbuffer, m_sinkbuffer = NULL;
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
  CLog::Log(LOGDEBUG, "  in Channel Layout: %s", ((std::string)in_format.m_channelLayout).c_str());

  return ((m_format.m_sampleRate    == format.m_sampleRate) &&
          (m_format.m_dataFormat    == format.m_dataFormat) &&
          (m_format.m_channelLayout == format.m_channelLayout));
}

double CAESinkAUDIOTRACK::GetDelay()
{
  // this includes any latency due to AudioTrack buffer size,
  // AudioMixer (if any) and audio hardware driver.

  double sinkbuffer_seconds_to_empty = m_sinkbuffer_sec_per_byte * (double)m_sinkbuffer->GetReadSize();
  //CLog::Log(LOGDEBUG, "CAESinkAUDIOTRACK::GetDelay, sinkbuffer_seconds_to_empty(%f), m_audiotracklatency_sec(%f)",
  //  sinkbuffer_seconds_to_empty, m_audiotracklatency_sec);
  return sinkbuffer_seconds_to_empty + m_audiotracklatency_sec;
}

double CAESinkAUDIOTRACK::GetCacheTime()
{
  // returns the time in seconds that it will take
  // to underrun the buffer if no sample is added.

  double sinkbuffer_seconds_to_empty = m_sinkbuffer_sec_per_byte * (double)m_sinkbuffer->GetReadSize();
  //CLog::Log(LOGDEBUG, "CAESinkAUDIOTRACK::GetCacheTime, sinkbuffer_seconds_to_empty(%f)",
  //  sinkbuffer_seconds_to_empty);
  return sinkbuffer_seconds_to_empty;
}

double CAESinkAUDIOTRACK::GetCacheTotal()
{
  // total amount that the audio sink can buffer in units of seconds

  //CLog::Log(LOGDEBUG, "CAESinkAUDIOTRACK::GetCacheTotal, m_sinkbuffer_sec(%f)",
  //  m_sinkbuffer_sec);
  return m_sinkbuffer_sec;
}

unsigned int CAESinkAUDIOTRACK::AddPackets(uint8_t *data, unsigned int frames)
{
  // write as many frames of audio as we can fit into our internal buffer.

  unsigned int write_frames = (m_sinkbuffer->GetWriteSize() / m_format.m_frameSize) % frames;
  if (write_frames)
    m_sinkbuffer->Write(data, write_frames * m_format.m_frameSize);

  // AddPackets runs under a non-idled AE thread so rather than attempt to sleep
  // some magic amount to keep from hammering AddPackets, just yield and pray.
  sched_yield();

  return write_frames;
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
  jmethodID jmStop              = jenv->GetMethodID(jcAudioTrack, "stop", "()V");
  jmethodID jmFlush             = jenv->GetMethodID(jcAudioTrack, "flush", "()V");
  jmethodID jmRelease           = jenv->GetMethodID(jcAudioTrack, "release", "()V");
  jmethodID jmWrite             = jenv->GetMethodID(jcAudioTrack, "write", "([BII)I");
  jmethodID jmPlayState         = jenv->GetMethodID(jcAudioTrack, "getPlayState", "()I");
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

  m_init_frames = min_buffer_size / m_format.m_frameSize;
  m_format.m_frames = m_init_frames;

  m_audiotrackbuffer_sec = (double)min_buffer_size / (double)m_format.m_sampleRate;
  m_audiotrackbuffer_sec_per_byte = 1.0 / (double)(m_format.m_frameSize * m_format.m_sampleRate);

  // we cannot get actual latency using jni so guess
  m_audiotracklatency_sec = 0.100;
  CLog::Log(LOGDEBUG, "CAESinkAUDIOTRACK::Process, "
    "m_init_frames(%d), m_audiotrackbuffer_sec(%f), m_audiotracklatency_sec(%f)",
    m_init_frames, m_audiotrackbuffer_sec, m_audiotracklatency_sec);

  jobject joAudioTrack = jenv->NewObject(jcAudioTrack, jmInit,
    GetStaticIntField(jenv, "AudioManager", "STREAM_MUSIC"),
    sampleRate,
    channelConfig,
    audioFormat,
    min_buffer_size,
    GetStaticIntField(jenv, "AudioTrack", "MODE_STREAM"));

  // The AudioTrack object has been created and waiting to play,
  m_inited.Set();
  // yield to give other threads a chance to do some work.
  sched_yield();

  // cache the playing int value.
  jint playing = GetStaticIntField(jenv, "AudioTrack", "PLAYSTATE_PLAYING");

  // create a java byte buffer for writing pcm data to AudioTrack.
  jarray jbuffer = jenv->NewByteArray(min_buffer_size);

  while (!m_bStop)
  {
    //int bytes_to_empty = jenv->CallIntMethod(joAudioTrack, jmGetPos);
    //bytes_to_empty *= m_format.m_frameSize;
    //bytes_to_empty %= min_buffer_size;
    if (m_draining)
    {
      unsigned char byte_drain[1024];
      unsigned int  byte_drain_size = m_sinkbuffer->GetReadSize() % 1024;
      while (byte_drain_size)
      {
        m_sinkbuffer->Read(byte_drain, byte_drain_size);
        byte_drain_size = m_sinkbuffer->GetReadSize() % 1024;
      }
      jenv->CallVoidMethod(joAudioTrack, jmStop);
      jenv->CallVoidMethod(joAudioTrack, jmFlush);
    }

    unsigned int read_bytes = m_sinkbuffer->GetReadSize() % min_buffer_size;
    if (read_bytes > 0)
    {
      // android will auto pause the playstate when it senses idle,
      // check it and set playing if it does this. Do this before
      // writing into its buffer.
      if (jenv->CallIntMethod(joAudioTrack, jmPlayState) != playing)
      {
        CLog::Log(LOGDEBUG, "CAESinkAUDIOTRACK::Process:jmPlay");
        jenv->CallVoidMethod(joAudioTrack, jmPlay);
      }

      // Stream the next batch of audio data to the Java AudioTrack.
      // Warning, no other JNI function can be called after
      // GetPrimitiveArrayCritical until ReleasePrimitiveArrayCritical.
      void *pBuffer = jenv->GetPrimitiveArrayCritical(jbuffer, NULL);
      if (pBuffer)
      {
        m_sinkbuffer->Read((unsigned char*)pBuffer, read_bytes);
        jenv->ReleasePrimitiveArrayCritical(jbuffer, pBuffer, 0);
        jenv->CallIntMethod(joAudioTrack, jmWrite, jbuffer, 0, read_bytes);

      }
      else
      {
        CLog::Log(LOGDEBUG, "Failed to get pointer to array bytes");
      }
    }
    // yield this audio thread to give other threads a chance to do some work.
    sched_yield();
  }

  jenv->DeleteLocalRef(jbuffer);

  jenv->CallVoidMethod(joAudioTrack, jmStop);
  jenv->CallVoidMethod(joAudioTrack, jmFlush);
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
