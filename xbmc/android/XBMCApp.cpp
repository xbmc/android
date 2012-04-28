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

template<class T, void(T::*fn)()>
void* thread_run(void* obj)
{
  (static_cast<T*>(obj)->*fn)();
  return NULL;
}

CXBMCApp::CXBMCApp()
{
  m_state.result = ActivityUnknown;
  m_state.platform = NULL;
  m_state.xbmcInitialize = NULL;
  m_state.xbmcRun = NULL;
  m_state.xbmcStop = NULL;
  m_state.xbmcTouch = NULL;

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
  if (m_state.xbmcInitialize == NULL || m_state.xbmcRun == NULL ||
      m_state.xbmcStop == NULL || m_state.xbmcTouch == NULL)
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

ActivityResult CXBMCApp::onStep()
{
  ActivityResult ret = ActivityOK;
  pthread_mutex_lock(&m_state.mutex);
  if (m_state.result != ActivityUnknown)
  {
    android_printf("%s: XBMC thread finished with %d", __PRETTY_FUNCTION__, (int)m_state.result);
    ret = m_state.result;
  }
  pthread_mutex_unlock(&m_state.mutex);

  // TODO
  return ret;
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

  switch (AKeyEvent_getAction(event))
  {
    case AKEY_EVENT_ACTION_DOWN:
      android_printf("CXBMCApp: key down (code: %d; repeat: %d; flags: 0x%0X; alt: %s; shift: %s; sym: %s)",
                      keycode, repeatCount, flags,
                      (state & AMETA_ALT_ON) ? "yes" : "no",
                      (state & AMETA_SHIFT_ON) ? "yes" : "no",
                      (state & AMETA_SYM_ON) ? "yes" : "no");
      break;

    case AKEY_EVENT_ACTION_UP:
      android_printf("CXBMCApp: key up (code: %d; repeat: %d; flags: 0x%0X; alt: %s; shift: %s; sym: %s)",
                      keycode, repeatCount, flags,
                      (state & AMETA_ALT_ON) ? "yes" : "no",
                      (state & AMETA_SHIFT_ON) ? "yes" : "no",
                      (state & AMETA_SYM_ON) ? "yes" : "no");
      break;

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

  pthread_mutex_lock(&m_state.mutex);
  m_state.result = ActivityExit;
  pthread_mutex_unlock(&m_state.mutex);
}

void CXBMCApp::join()
{
  android_printf("%s", __PRETTY_FUNCTION__);
  pthread_mutex_lock(&m_state.mutex);
  if (m_state.result == ActivityUnknown)
  {
    android_printf(" => executing XBMC_Stop");
    m_state.xbmcStop();
    android_printf(" => waiting for XBMC to finish");
    pthread_join(m_state.thread, NULL);
  }
  pthread_mutex_unlock(&m_state.mutex);
  android_printf(" => XBMC finished");
}

