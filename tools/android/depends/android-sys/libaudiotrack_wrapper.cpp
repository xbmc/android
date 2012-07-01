/*
 *      Copyright (C) 2005-2012 Team XBMC
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

// adapted from GPLv2 audioflinger_wrapper.cpp
// http://cgit.freedesktop.org/gstreamer/gst-android

#include <binder/ProcessState.h>
#include <media/AudioTrack.h>
#if __ANDROID_API__ > 13
#include <system/audio.h>
#endif

#include "libaudiotrack_wrapper.h"

using namespace android;

extern "C" {
  int android_printf(const char *format, ...)
  {
    va_list args;
    va_start(args, format);
    int result = __android_log_vprint(ANDROID_LOG_VERBOSE, "XBMC", format, args);
    va_end(args);
    return result;
  }
}

/* commonly used macro */
#define AUDIO_TRACK_DEVICE(ctx) ((AudioTrackDevice*)ctx)
#define AUDIO_TRACK_DEVICE_TRACK(ctx) (AUDIO_TRACK_DEVICE(ctx)->audio_track)

typedef struct _AudioTrackDevice
{
  void* cbf_ctx;
  ATWCallback cbf;
  AudioTrack *audio_track;
} AudioTrackDevice;

static void audioCallback(int event, void *user, void *info)
{
  AudioTrackDevice *atd = (AudioTrackDevice*)user;
  AudioTrack::Buffer *buffer = static_cast<AudioTrack::Buffer*>(info);

  if (event != AudioTrack::EVENT_MORE_DATA)
  {
    android_printf("audioCallback: event(%d) \n", event);
    return;
  }

  if (buffer == NULL || buffer->size == 0)
  {
    android_printf("audioCallback: Wrong buffer\n");
    return;
  }

  if (atd  && atd->cbf && atd->cbf_ctx)
  {
    int len = (atd->cbf)(atd->cbf_ctx, buffer->i16, buffer->size);
    buffer->size = len;
  }
  return;
}


ATW_ctx ATW_create(void)
{
  AudioTrackDevice* audiodev = new AudioTrackDevice;
  // create a new instance of AudioTrack 
  audiodev->audio_track = new AudioTrack();
  if (audiodev->audio_track == NULL)
  {
    android_printf("Error creating AudioTrack\n");
    return NULL;
  }

  android_printf("Created AudioTrack successfully\n");

  return (ATW_ctx)audiodev;
}

void ATW_release(ATW_ctx ctx)
{
  // Releases the native AudioTrack resources
  if (ctx == NULL)
    return;

  if (AUDIO_TRACK_DEVICE_TRACK(ctx))
  {
    android_printf("ctx : %p Release AudioTrack\n", ctx);
    //delete AUDIO_TRACK_DEVICE_TRACK(ctx);
  }
  delete AUDIO_TRACK_DEVICE(ctx);
}

int ATW_get_min_framecount(ATW_ctx ctx, int* frameCount, int sampleRate = 0)
{
  status_t status = NO_ERROR;
  if (ctx == NULL)
    return -1;

  status = AUDIO_TRACK_DEVICE_TRACK(ctx)->getMinFrameCount(
#ifdef ANDROID_AUDIO_CORE_H
    frameCount, AUDIO_STREAM_MUSIC, sampleRate);
#else
    frameCount, AudioSystem::MUSIC, sampleRate);
#endif

  if (status != NO_ERROR) 
    return -1;

  return 0;
}

int ATW_set(ATW_ctx ctx, int channelCount, int sampleRate, int frameCount,
  ATWCallback cbf, void *cbf_ctx)
{
  // channelCount:  Channel mask: see AudioSystem::audio_channels.
  // sampleRate:    Track sampling rate in Hz.
  // frameCount:    Total size of track PCM buffer in frames, 
  //                  this defines the latency of the track.
  status_t status = NO_ERROR;
  if (ctx == NULL)
    return -1;

  AUDIO_TRACK_DEVICE(ctx)->cbf = cbf;
  AUDIO_TRACK_DEVICE(ctx)->cbf_ctx = cbf_ctx;

  // bufferCount is not the number of internal buffer,
  // but the internal buffer size
#ifdef ANDROID_AUDIO_CORE_H
  status = AUDIO_TRACK_DEVICE_TRACK(ctx)->set(
      AUDIO_STREAM_MUSIC,
      (uint32_t)sampleRate, 
      AUDIO_FORMAT_PCM_16_BIT,
      (channelCount == 1) ? AUDIO_CHANNEL_OUT_MONO : AUDIO_CHANNEL_OUT_STEREO,
      frameCount,
      0,              // flags
      audioCallback,  // callback
      ctx,            // ctx for callback
      0,              // notificationFrames
      0);             // sessionId
);
#else
  status = AUDIO_TRACK_DEVICE_TRACK(ctx)->set(
      AudioSystem::MUSIC,
      (uint32_t)sampleRate, 
      AudioSystem::PCM_16_BIT,
      (channelCount == 1) ? AudioSystem::CHANNEL_OUT_MONO : AudioSystem::CHANNEL_OUT_STEREO,
      frameCount,
      0,              // flags
      audioCallback,  // callback
      ctx,            // ctx for callback
      0,              // notificationFrames
      0);             // sessionId
#endif
  android_printf("Set AudioTrack, status: %d, , sampleRate: %d, "
      "channelCount: %d, frameCount: %d\n", status, sampleRate, 
      channelCount, frameCount);

  if (status != NO_ERROR) 
    return -1;

  return 0;
}

void ATW_start(ATW_ctx ctx)
{
  // After it's created the track is not active. Call start() to
  // make it active. If set, the callback will start being called.
  if (ctx == NULL)
    return;

  AUDIO_TRACK_DEVICE_TRACK(ctx)->start();
}

void ATW_stop(ATW_ctx ctx)
{
  // Stop a track. If set, the callback will cease being called and
  // obtainBuffer returns STOPPED. Note that obtainBuffer() still works
  // and will fill up buffers until the pool is exhausted.
  if (ctx == NULL)
    return;

  AUDIO_TRACK_DEVICE_TRACK(ctx)->stop();
}

void ATW_flush(ATW_ctx ctx)
{
  // Flush a stopped track. All pending buffers are discarded.
  // This function has no effect if the track is not stoped.
  if (ctx == NULL)
    return;

  AUDIO_TRACK_DEVICE_TRACK(ctx)->flush();
}

void ATW_pause(ATW_ctx ctx)
{
  // Pause a track. If set, the callback will cease being called and
  // obtainBuffer returns STOPPED. Note that obtainBuffer() still works
  // and will fill up buffers until the pool is exhausted.
  if (ctx == NULL)
    return;

  AUDIO_TRACK_DEVICE_TRACK(ctx)->pause();
}

void ATW_mute(ATW_ctx ctx, int mute)
{
  // mute or unmutes this track.
  // While mutted, the callback, if set, is still called.
  if (ctx == NULL)
    return;

  AUDIO_TRACK_DEVICE_TRACK(ctx)->mute((bool)mute);
}

int ATW_muted(ATW_ctx ctx)
{
  if (ctx == NULL)
    return -1;

  return (int) AUDIO_TRACK_DEVICE_TRACK(ctx)->muted();
}


void ATW_set_volume(ATW_ctx ctx, float left, float right)
{
  // Set volume for this track, mostly used for games' sound effects
  // left and right volumes. Levels must be <= 1.0.
  if (ctx == NULL)
    return;

  AUDIO_TRACK_DEVICE_TRACK(ctx)->setVolume(left, right);
}

int ATW_write(ATW_ctx ctx, const void *buffer, size_t size)
{
  // As a convenience we provide a write() interface to the audio buffer.
  // This is implemented on top of lockBuffer/unlockBuffer. For best
  // performance
  if (ctx == NULL)
    return -1;

  // this will block
  return AUDIO_TRACK_DEVICE_TRACK(ctx)->write(buffer, size);
}

int ATW_latency(ATW_ctx ctx)
{
  // Returns this track's latency in milliseconds.
  // This includes the latency due to AudioTrack buffer size, AudioMixer (if any)
  // and audio hardware driver.
  if (ctx == NULL)
    return -1;

  return (int)AUDIO_TRACK_DEVICE_TRACK(ctx)->latency();
}
