/*
 *      Copyright (C) 2005-2012 Team XBMC
 *      http://www.xbmc.org
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#pragma once

// we need both here, one is for including into android, the other xbmc
#include <stdint.h>
#if defined(ANDROID) || defined(__ANDROID__)
struct ANativeWindow;
typedef struct ANativeWindow ANativeWindow;

typedef int (*android_printf_t)(const char *format, ...);
typedef int32_t (*android_setBuffersGeometry_t)(ANativeWindow* window_type, int32_t width, int32_t height, int32_t format);

struct XBMC_PLATFORM {
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
  const char    *log_name;
};
#endif
typedef struct XBMC_PLATFORM XBMC_PLATFORM;


typedef int (*XBMC_Run_t)(bool, XBMC_PLATFORM*);
typedef void (*XBMC_Pause_t)(bool);
typedef void (*XBMC_Stop_t)();
typedef void (*XBMC_Key_t)(uint8_t, uint16_t, uint16_t, bool);
typedef void (*XBMC_Touch_t)(uint8_t, uint8_t, uint16_t, uint16_t);
typedef void (*XBMC_TouchGesture_t)(int32_t, float, float, float, float);
typedef int (*XBMC_TouchGestureCheck_t)(float, float);

typedef bool (*XBMC_SetupDisplay_t)();
typedef bool (*XBMC_DestroyDisplay_t)();

extern "C" int XBMC_Run(bool renderGUI = true, XBMC_PLATFORM* = NULL);
extern "C" void XBMC_Pause(bool pause);
extern "C" void XBMC_Stop();
extern "C" void XBMC_Key(uint8_t code, uint16_t key, uint16_t modifiers, bool up);
extern "C" void XBMC_Touch(uint8_t type, uint8_t button, uint16_t x, uint16_t y);
extern "C" void XBMC_TouchGesture(int32_t action, float posX, float posY, float offsetX, float offsetY);
extern "C" int XBMC_TouchGestureCheck(float posX, float posY);

extern "C" bool XBMC_SetupDisplay();
extern "C" bool XBMC_DestroyDisplay();

