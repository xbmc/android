#pragma once

/*
 *      Copyright (C) 2012 Team XBMC
 *      http://xbmc.org
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

#include <stdint.h>

// runtime application flags
typedef enum {
  XBMCRunNull     = 0x00,
  XBMCRunPrimary  = 0x01,  // XBMC application
  XBMCRunGui      = 0x02,  // initialize GUI services
  XBMCRunServices = 0x04,  // start services: dbus, web, etc.
  XBMCRunAsApp    = XBMCRunPrimary | XBMCRunGui | XBMCRunServices
} XBMC_RUNFLAGS;

// we need both here, one is for including into android, the other xbmc
#if defined(ANDROID) || defined(__ANDROID__)
struct ANativeWindow;
typedef struct ANativeWindow ANativeWindow;

typedef int (*android_printf_t)(const char *format, ...);
typedef int32_t (*android_setBuffersGeometry_t)(ANativeWindow* window_type, int32_t width, int32_t height, int32_t format);

struct XBMC_PLATFORM {
  XBMC_RUNFLAGS flags;
  const char    *log_name;
  // android specific
  ANativeWindow *window;
  // callbacks from xbmc into android
  // these are setup before call into XBMC_Initialize
  android_printf_t android_printf;
  android_setBuffersGeometry_t android_setBuffersGeometry;

  // callbacks from android into xbmc
  // these are valid after call into XBMC_Initialize

};

#else
struct XBMC_PLATFORM {
  XBMC_RUNFLAGS flags;
  const char    *log_name;
};
#endif
typedef struct XBMC_PLATFORM XBMC_PLATFORM;


typedef int (*XBMC_Initialize_t)(XBMC_PLATFORM*, int, const char**);
typedef int (*XBMC_Run_t)();
typedef void (*XBMC_Pause_t)(bool);
typedef void (*XBMC_Stop_t)();
typedef void (*XBMC_Key_t)(uint8_t, uint16_t, uint16_t, bool);
typedef void (*XBMC_Touch_t)(uint8_t, uint8_t, uint16_t, uint16_t);
typedef void (*XBMC_TouchGesture_t)(int32_t, float, float, float, float);
typedef int (*XBMC_TouchGestureCheck_t)(float, float);

extern "C" int XBMC_Initialize(XBMC_PLATFORM *platform, int argc, const char** argv);
extern "C" int XBMC_Run();
extern "C" void XBMC_Pause(bool pause);
extern "C" void XBMC_Stop();
extern "C" void XBMC_Key(uint8_t code, uint16_t key, uint16_t modifiers, bool up);
extern "C" void XBMC_Touch(uint8_t type, uint8_t button, uint16_t x, uint16_t y);
extern "C" void XBMC_TouchGesture(int32_t action, float posX, float posY, float offsetX, float offsetY);
extern "C" int XBMC_TouchGestureCheck(float posX, float posY);

