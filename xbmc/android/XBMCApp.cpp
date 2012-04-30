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

typedef struct {
  int32_t nativeKey;
  int16_t xbmcKey;
} KeyMap;

static KeyMap keyMap[] = {
  // first list all keys that we don't handle
  { AKEYCODE_HOME         , XBMCK_LAST },
  { AKEYCODE_POWER        , XBMCK_LAST },

  // now list all the keys with special functionality
  { AKEYCODE_BACK         , XBMCK_BACKSPACE },
  { AKEYCODE_DPAD_UP      , XBMCK_UP },
  { AKEYCODE_DPAD_DOWN    , XBMCK_DOWN },
  { AKEYCODE_DPAD_LEFT    , XBMCK_LEFT },
  { AKEYCODE_DPAD_RIGHT   , XBMCK_RIGHT },
  { AKEYCODE_VOLUME_UP    , XBMCK_VOLUME_UP },
  { AKEYCODE_VOLUME_DOWN  , XBMCK_VOLUME_DOWN },
  { AKEYCODE_MUTE         , XBMCK_VOLUME_MUTE },
  { AKEYCODE_MENU         , XBMCK_MENU }
};

CXBMCApp::CXBMCApp(ANativeActivity *nativeActivity)
{
  m_activity = nativeActivity;
  
  if (m_activity == NULL)
  {
    android_printf("CXBMCApp: invalid ANativeActivity instance");
    exit(1);
    return;
  }
  
  m_state.result = ActivityUnknown;
  m_state.stopping = false;
  m_state.platform = NULL;
  m_state.xbmcInitialize = NULL;
  m_state.xbmcRun = NULL;
  m_state.xbmcStop = NULL;
  m_state.xbmcTouch = NULL;
  m_state.xbmcKey = NULL;

  if (pthread_mutex_init(&m_state.mutex, NULL) != 0)
  {
    android_printf("CXBMCApp: pthread_mutex_init() failed");
    exit(1);
    return;
  }

  void* soHandle = lo_dlopen("/data/data/org.xbmc/lib/libxbmc.so");
  if (soHandle == NULL)
    return;

  // fetch xbmc entry points
  m_state.xbmcInitialize = (XBMC_Initialize_t)dlsym(soHandle, "XBMC_Initialize");
  m_state.xbmcRun = (XBMC_Run_t)dlsym(soHandle, "XBMC_Run");
  m_state.xbmcStop = (XBMC_Stop_t)dlsym(soHandle, "XBMC_Stop");
  m_state.xbmcTouch = (XBMC_Touch_t)dlsym(soHandle, "XBMC_Touch");
  m_state.xbmcKey = (XBMC_Key_t)dlsym(soHandle, "XBMC_Key");
  if (m_state.xbmcInitialize == NULL || m_state.xbmcRun == NULL ||
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
  join();

  pthread_mutex_destroy(&m_state.mutex);
}

ActivityResult CXBMCApp::onActivate()
{
  android_printf("%s", __PRETTY_FUNCTION__);
  if (m_state.platform->window == NULL)
  {
    android_printf("no valid ANativeWindow instance available");
    return ActivityError;
  }

  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  pthread_create(&m_state.thread, &attr, thread_run<CXBMCApp, &CXBMCApp::run>, this);
  pthread_attr_destroy(&attr);
  return ActivityOK;
}

void CXBMCApp::onDeactivate()
{
  android_printf("%s", __PRETTY_FUNCTION__);
  // TODO
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
  join();

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
  uint16_t sym = XBMCK_UNKNOWN;
  for (unsigned int index = 0; index < sizeof(keyMap) / sizeof(KeyMap); index++)
  {
    if (keycode == keyMap[index].nativeKey)
    {
      sym = keyMap[index].xbmcKey;
      break;
    }
  }
  
  // check if this is a key we don't want to handle
  if (sym == XBMCK_LAST)
    return false;

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
      return true;

    case AMOTION_EVENT_ACTION_UP:
      android_printf("CXBMCApp: touch up (%f, %f)", x, y);
      m_state.xbmcTouch((uint16_t)x, (uint16_t)y, true);
      return true;

    case AMOTION_EVENT_ACTION_MOVE:
      android_printf("CXBMCApp: touch move (%f, %f)", x, y);
      break;

    default:
      android_printf("CXBMCApp: touch unknown (%f, %f)", x, y);
      break;
  }

  return false;
}

void CXBMCApp::run()
{
  android_printf("%s", __PRETTY_FUNCTION__);
  if (m_state.xbmcInitialize == NULL || m_state.xbmcRun == NULL)
  {
    android_printf(" => invalid XBMC_Initialize or XBMC_Run instance");
    pthread_mutex_lock(&m_state.mutex);
    m_state.result = ActivityError;
    pthread_mutex_unlock(&m_state.mutex);
    return;
  }

  int status = m_state.xbmcInitialize(m_state.platform, 0, NULL);
  if (status != 0)
  {
    android_printf("XBMC_Initialize failed");
    pthread_mutex_lock(&m_state.mutex);
    m_state.result = ActivityError;
    pthread_mutex_unlock(&m_state.mutex);
    return;
  }

  android_printf(" => running XBMC_Run...");
  try
  {
    status = m_state.xbmcRun();
    android_printf(" => XBMC_Run finished with %d", status);
  }
  catch(...)
  {
    android_printf("ERROR: Exception caught on main loop. Exiting");
    pthread_mutex_lock(&m_state.mutex);
    m_state.result = ActivityError;
    pthread_mutex_unlock(&m_state.mutex);
    return;
  }

  bool finishActivity = false;
  pthread_mutex_lock(&m_state.mutex);
  m_state.result = ActivityExit;
  finishActivity = !m_state.stopping;
  pthread_mutex_unlock(&m_state.mutex);
  
  if (finishActivity)
  {
    android_printf(" => calling ANativeActivity_finish()");
    ANativeActivity_finish(m_activity);
  }
}

void CXBMCApp::join()
{
  android_printf("%s", __PRETTY_FUNCTION__);

  pthread_mutex_lock(&m_state.mutex);
  if (m_state.result == ActivityUnknown)
  {
    m_state.stopping = true;
    pthread_mutex_unlock(&m_state.mutex);
    
    android_printf(" => executing XBMC_Stop");
    m_state.xbmcStop();
    android_printf(" => waiting for XBMC to finish");
    pthread_join(m_state.thread, NULL);
    android_printf(" => XBMC finished");
  }
  else
    pthread_mutex_unlock(&m_state.mutex);
}
