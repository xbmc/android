#pragma once
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
#ifdef TARGET_ANDROID

#include "Interfaces/AESink.h"
#include "Utils/AEDeviceInfo.h"
#include <stdint.h>

typedef void* ATW_ctx;
class AERingBuffer;
class DllLibAudioTrack;

class CAESinkAUDIOTRACK : public IAESink
{
public:
  virtual const char *GetName() { return "AUDIOTRACK"; }

  CAESinkAUDIOTRACK();
  virtual ~CAESinkAUDIOTRACK();

  virtual bool Initialize(AEAudioFormat &format, std::string &device);
  virtual void Deinitialize();
  virtual bool IsCompatible(const AEAudioFormat format, const std::string device);

  virtual double       GetDelay        ();
  virtual double       GetCacheTime    ();
  virtual double       GetCacheTotal   ();
  virtual unsigned int AddPackets      (uint8_t *data, unsigned int frames);
  virtual void         Drain           ();

  static void          EnumerateDevicesEx(AEDeviceInfoList &list);

  static int           AudioTrackCallback(void *ctx, void *buffer, size_t size);
private:
  static DllLibAudioTrack *m_dll;
  AEAudioFormat      m_format;
  ATW_ctx            m_ATWrapper;
  AERingBuffer      *m_buffer;
  bool               m_started;
  bool               m_draining;

};

#endif