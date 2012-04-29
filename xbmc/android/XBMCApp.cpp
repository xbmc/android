/*
 *      Copyright (C) 2012 Team XBMC
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
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include <unistd.h>
#include <stdlib.h>
#include <dlfcn.h>

#include <android/native_window.h>
#include "android_utils.h"

#include "XBMCApp.h"
#include "xbmc_log.h"

#include "input/XBMC_keysym.h"

template<class T, void(T::*fn)()>
void* thread_run(void* obj)
{
  (static_cast<T*>(obj)->*fn)();
  return NULL;
}

CXBMCApp::CXBMCApp()
{
  m_state.initialized = false;
  m_state.platform = NULL;
  m_state.xbmcInitialize = NULL;
  m_state.xbmcStep = NULL;
  m_state.xbmcStop = NULL;
  m_state.xbmcTouch = NULL;
  m_state.xbmcKey = NULL;

  void* soHandle = lo_dlopen("/data/data/org.xbmc/lib/libxbmc.so");
  if (soHandle == NULL)
    return;

  // fetch xbmc entry points
  m_state.xbmcInitialize = (XBMC_Initialize_t)dlsym(soHandle, "XBMC_Initialize");
  m_state.xbmcStep = (XBMC_RunStep_t)dlsym(soHandle, "XBMC_RunStep");
  m_state.xbmcStop = (XBMC_Stop_t)dlsym(soHandle, "XBMC_Stop");
  m_state.xbmcTouch = (XBMC_Touch_t)dlsym(soHandle, "XBMC_Touch");
  m_state.xbmcKey = (XBMC_Key_t)dlsym(soHandle, "XBMC_Key");
  if (m_state.xbmcInitialize == NULL || m_state.xbmcStep == NULL ||
      m_state.xbmcStop == NULL || m_state.xbmcTouch == NULL || m_state.xbmcKey == NULL)
  {
    android_printf("CXBMCApp: could not find XBMC_* functions. Error: %s", dlerror());
    return;
  }

  m_state.platform = new XBMC_PLATFORM;
  if (m_state.platform == NULL)
  {
    android_printf("CXBMCApp: could not initialize a new platform");
    return;
  }

  // stardard platform stuff
  m_state.platform->flags  = XBMCRunAsApp;
  m_state.platform->log_name = "XBMC";
  // android specific
  m_state.platform->window = NULL;
  m_state.platform->android_printf = &android_printf;
  m_state.platform->android_setBuffersGeometry = &ANativeWindow_setBuffersGeometry;
}

CXBMCApp::~CXBMCApp()
{
  stop();
}

ActivityResult CXBMCApp::onActivate()
{
  android_printf("%s", __PRETTY_FUNCTION__);
  if (m_state.platform->window == NULL)
  {
    android_printf("no valid ANativeWindow instance available");
    return ActivityError;
  }

  if (m_state.xbmcInitialize == NULL || m_state.xbmcStep == NULL)
  {
    android_printf(" => invalid XBMC_Initialize or XBMC_RunStep instance");
    return ActivityError;
  }

  if (m_state.initialized)
    return ActivityOK;

  int status = m_state.xbmcInitialize(m_state.platform, 0, NULL);
  if (status != 0)
  {
    android_printf("XBMC_Initialize failed");
    return ActivityError;
  }

  m_state.initialized = true;
  return ActivityOK;
}

void CXBMCApp::onDeactivate()
{
  android_printf("%s", __PRETTY_FUNCTION__);
  // TODO
}

ActivityResult CXBMCApp::onStep()
{
  if (m_state.xbmcStep == NULL || m_state.xbmcStop == NULL)
    return ActivityError;

  bool stop = m_state.xbmcStep();
  if (stop)
  {
    android_printf("XBMC is stopping");
    m_state.xbmcStop(true);
    return ActivityExit;
  }

  return ActivityOK;
}

void CXBMCApp::onStart()
{
  android_printf("%s", __PRETTY_FUNCTION__);
  // TODO
}

void CXBMCApp::onResume()
{
  android_printf("%s", __PRETTY_FUNCTION__);
  // TODO
}

void CXBMCApp::onPause()
{
  android_printf("%s", __PRETTY_FUNCTION__);
  // TODO
}

void CXBMCApp::onStop()
{
  android_printf("%s", __PRETTY_FUNCTION__);
  // TODO
}

void CXBMCApp::onDestroy()
{
  android_printf("%s", __PRETTY_FUNCTION__);
  stop();

  // TODO
}

void CXBMCApp::onSaveState(void **data, size_t *size)
{
  android_printf("%s", __PRETTY_FUNCTION__);
  *size = sizeof(State);
  *data = malloc(*size);
  memcpy(*data, &m_state, *size);
}

void CXBMCApp::onConfigurationChanged()
{
  android_printf("%s", __PRETTY_FUNCTION__);
  // TODO
}

void CXBMCApp::onLowMemory()
{
  android_printf("%s", __PRETTY_FUNCTION__);
  // TODO
}

void CXBMCApp::onCreateWindow(ANativeWindow* window)
{
  android_printf("%s", __PRETTY_FUNCTION__);
  if (window == NULL)
  {
    android_printf(" => invalid ANativeWindow object");
    return;
  }

  m_state.platform->window = window;
  // TODO
}

void CXBMCApp::onResizeWindow()
{
  android_printf("%s", __PRETTY_FUNCTION__);
  // TODO
}

void CXBMCApp::onDestroyWindow()
{
  android_printf("%s", __PRETTY_FUNCTION__);
  // TODO
}

void CXBMCApp::onGainFocus()
{
  android_printf("%s", __PRETTY_FUNCTION__);
  // TODO
}

void CXBMCApp::onLostFocus()
{
  android_printf("%s", __PRETTY_FUNCTION__);
  // TODO
}

bool CXBMCApp::onKeyboardEvent(AInputEvent* event)
{
  android_printf("%s", __PRETTY_FUNCTION__);
  if (event == NULL)
    return false;

  int32_t keycode = AKeyEvent_getKeyCode(event);
  int32_t flags = AKeyEvent_getFlags(event);
  int32_t state = AKeyEvent_getMetaState(event);
  int32_t repeatCount = AKeyEvent_getRepeatCount(event);

  // Check if we got some special key
  uint16_t sym = 0;
  switch (keycode)
  {
    case AKEYCODE_DPAD_UP:
      sym = XBMCK_UP;
      break;

    case AKEYCODE_DPAD_DOWN:
      sym = XBMCK_DOWN;
      break;

    case AKEYCODE_DPAD_LEFT:
      sym = XBMCK_LEFT;
      break;

    case AKEYCODE_DPAD_RIGHT:
      sym = XBMCK_RIGHT;
      break;

    case AKEYCODE_VOLUME_UP:
      sym = XBMCK_VOLUME_UP;
      break;

    case AKEYCODE_VOLUME_DOWN:
      sym = XBMCK_VOLUME_DOWN;
      break;

    case AKEYCODE_MUTE:
      sym = XBMCK_VOLUME_MUTE;
      break;

    case AKEYCODE_MENU:
      sym = XBMCK_HOME;
      break;

    // TODO: Handle more
    default:
      break;
  }

  uint16_t modifiers = 0;
  if (state & AMETA_ALT_LEFT_ON)
    modifiers |= XBMCKMOD_LALT;
  if (state & AMETA_ALT_RIGHT_ON)
    modifiers |= XBMCKMOD_RALT;
  if (state & AMETA_SHIFT_LEFT_ON)
    modifiers |= XBMCKMOD_LSHIFT;
  if (state & AMETA_SHIFT_RIGHT_ON)
    modifiers |= XBMCKMOD_RSHIFT;
  /* TODO:
  if (state & AMETA_SYM_ON)
    modifiers |= 0x000?;*/

  // never ever try to handle the back, home or power button
  if (keycode == AKEYCODE_BACK ||
      keycode == AKEYCODE_HOME ||
      keycode == AKEYCODE_POWER)
    return false;

  switch (AKeyEvent_getAction(event))
  {
    case AKEY_EVENT_ACTION_DOWN:
      android_printf("CXBMCApp: key down (code: %d; repeat: %d; flags: 0x%0X; alt: %s; shift: %s; sym: %s)",
                      keycode, repeatCount, flags,
                      (state & AMETA_ALT_ON) ? "yes" : "no",
                      (state & AMETA_SHIFT_ON) ? "yes" : "no",
                      (state & AMETA_SYM_ON) ? "yes" : "no");
      m_state.xbmcKey((uint8_t)keycode, sym, modifiers, false);
      return true;

    case AKEY_EVENT_ACTION_UP:
      android_printf("CXBMCApp: key up (code: %d; repeat: %d; flags: 0x%0X; alt: %s; shift: %s; sym: %s)",
                      keycode, repeatCount, flags,
                      (state & AMETA_ALT_ON) ? "yes" : "no",
                      (state & AMETA_SHIFT_ON) ? "yes" : "no",
                     (state & AMETA_SYM_ON) ? "yes" : "no");
      m_state.xbmcKey((uint8_t)keycode, sym, modifiers, true);
      return true;

    case AKEY_EVENT_ACTION_MULTIPLE:
      android_printf("CXBMCApp: key multiple (code: %d; repeat: %d; flags: 0x%0X; alt: %s; shift: %s; sym: %s)",
                      keycode, repeatCount, flags,
                      (state & AMETA_ALT_ON) ? "yes" : "no",
                      (state & AMETA_SHIFT_ON) ? "yes" : "no",
                      (state & AMETA_SYM_ON) ? "yes" : "no");
      break;

    default:
      android_printf("CXBMCApp: unknown key (code: %d; repeat: %d; flags: 0x%0X; alt: %s; shift: %s; sym: %s)",
                      keycode, repeatCount, flags,
                      (state & AMETA_ALT_ON) ? "yes" : "no",
                      (state & AMETA_SHIFT_ON) ? "yes" : "no",
                      (state & AMETA_SYM_ON) ? "yes" : "no");
      break;
  }

  return false;
}

bool CXBMCApp::onTouchEvent(AInputEvent* event)
{
  android_printf("%s", __PRETTY_FUNCTION__);
  if (event == NULL)
    return false;

  float x = AMotionEvent_getX(event, 0);
  float y = AMotionEvent_getY(event, 0);

  switch (AMotionEvent_getAction(event))
  {
    case AMOTION_EVENT_ACTION_DOWN:
      android_printf("CXBMCApp: touch down (%f, %f)", x, y);
      m_state.xbmcTouch((uint16_t)x, (uint16_t)y, false);
      break;

    case AMOTION_EVENT_ACTION_UP:
      android_printf("CXBMCApp: touch up (%f, %f)", x, y);
      m_state.xbmcTouch((uint16_t)x, (uint16_t)y, true);
      break;

    case AMOTION_EVENT_ACTION_MOVE:
      android_printf("CXBMCApp: touch move (%f, %f)", x, y);
      break;

    default:
      android_printf("CXBMCApp: touch unknown (%f, %f)", x, y);
      break;
  }

  // TODO: We should return true if XBMC actually handled the key press
  return false;
}

void CXBMCApp::stop()
{
  android_printf("%s", __PRETTY_FUNCTION__);
  if (!m_state.initialized)
    return;

  android_printf(" => executing XBMC_Stop");
  // first let XBMC quit
  m_state.xbmcStop(false);
  // now destroy it
  m_state.xbmcStop(true);

  m_state.initialized = false;
}

