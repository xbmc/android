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
#include "loader/xb_dlopen.h"

#include "XBMCApp.h"
#include "xbmc_log.h"

#include "input/MouseStat.h"
#include "input/XBMC_keysym.h"
#include "guilib/guilib_defines.h"
#include "windowing/XBMC_events.h"

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
  : m_touchGestureState(TouchGestureUnknown)
{
  m_activity = nativeActivity;
  
  if (m_activity == NULL)
  {
    android_printf("CXBMCApp: invalid ANativeActivity instance");
    exit(1);
    return;
  }

  m_state.appState = Uninitialized;
  m_state.platform = NULL;
  m_state.xbmcInitialize = NULL;
  m_state.xbmcRun = NULL;
  m_state.xbmcPause = NULL;
  m_state.xbmcStop = NULL;
  m_state.xbmcKey = NULL;
  m_state.xbmcTouch = NULL;
  m_state.xbmcTouchGesture = NULL;

  if (pthread_mutex_init(&m_state.mutex, NULL) != 0)
  {
    android_printf("CXBMCApp: pthread_mutex_init() failed");
    m_state.appState = Error;
    exit(1);
    return;
  }

  void* soHandle = xb_dlopen("/data/data/org.xbmc/lib/libxbmc.so");
  if (soHandle == NULL)
  {
    android_printf("CXBMCApp: could not load libxbmc.so. Error: %s", dlerror());
    m_state.appState = Error;
    return;
  }

  // fetch xbmc entry points
  m_state.xbmcInitialize = (XBMC_Initialize_t)dlsym(soHandle, "XBMC_Initialize");
  m_state.xbmcRun = (XBMC_Run_t)dlsym(soHandle, "XBMC_Run");
  m_state.xbmcPause = (XBMC_Pause_t)dlsym(soHandle, "XBMC_Pause");
  m_state.xbmcStop = (XBMC_Stop_t)dlsym(soHandle, "XBMC_Stop");
  m_state.xbmcKey = (XBMC_Key_t)dlsym(soHandle, "XBMC_Key");
  m_state.xbmcTouch = (XBMC_Touch_t)dlsym(soHandle, "XBMC_Touch");
  m_state.xbmcTouchGesture = (XBMC_TouchGesture_t)dlsym(soHandle, "XBMC_TouchGesture");
  if (m_state.xbmcInitialize == NULL || m_state.xbmcRun == NULL ||
      m_state.xbmcPause == NULL || m_state.xbmcStop == NULL || m_state.xbmcKey == NULL ||
      m_state.xbmcTouch == NULL || m_state.xbmcTouchGesture == NULL)
  {
    android_printf("CXBMCApp: could not find XBMC_* functions. Error: %s", dlerror());
    m_state.appState = Error;
    return;
  }

  m_state.platform = new XBMC_PLATFORM;
  if (m_state.platform == NULL)
  {
    android_printf("CXBMCApp: could not initialize a new platform");
    m_state.appState = Error;
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

  pthread_mutex_destroy(&m_state.mutex);
}

ActivityResult CXBMCApp::onActivate()
{
  android_printf("%s: %d", __PRETTY_FUNCTION__, m_state.appState);
  if (m_state.platform->window == NULL)
  {
    android_printf("no valid ANativeWindow instance available");
    return ActivityError;
  }

  switch (m_state.appState)
  {
    case Uninitialized:
      pthread_attr_t attr;
      pthread_attr_init(&attr);
      pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
      pthread_create(&m_state.thread, &attr, thread_run<CXBMCApp, &CXBMCApp::run>, this);
      pthread_attr_destroy(&attr);
      break;
      
    case Unfocused:
      m_state.xbmcPause(false);
      setAppState(Rendering);
      break;
      
    case Paused:
      // TODO
      break;

    case Initialized:
    case Rendering:
    case Stopping:
    case Stopped:
    case Error:
    default:
      break;
  }
  
  return ActivityOK;
}

void CXBMCApp::onDeactivate()
{
  android_printf("%s: %d", __PRETTY_FUNCTION__, m_state.appState);
  // TODO: This is called on pause, stop and window destroy
}

void CXBMCApp::onStart()
{
  android_printf("%s: %d", __PRETTY_FUNCTION__, m_state.appState);
  // TODO
}

void CXBMCApp::onResume()
{
  android_printf("%s: %d", __PRETTY_FUNCTION__, m_state.appState);
  // TODO
}

void CXBMCApp::onPause()
{
  android_printf("%s: %d", __PRETTY_FUNCTION__, m_state.appState);
  // TODO
}

void CXBMCApp::onStop()
{
  android_printf("%s: %d", __PRETTY_FUNCTION__, m_state.appState);
  // TODO
}

void CXBMCApp::onDestroy()
{
  android_printf("%s: %d", __PRETTY_FUNCTION__, m_state.appState);
  stop();

  // TODO
}

void CXBMCApp::onSaveState(void **data, size_t *size)
{
  android_printf("%s: %d", __PRETTY_FUNCTION__, m_state.appState);
  // TODO
}

void CXBMCApp::onConfigurationChanged()
{
  android_printf("%s: %d", __PRETTY_FUNCTION__, m_state.appState);
  // TODO
}

void CXBMCApp::onLowMemory()
{
  android_printf("%s: %d", __PRETTY_FUNCTION__, m_state.appState);
  // TODO
}

void CXBMCApp::onCreateWindow(ANativeWindow* window)
{
  android_printf("%s: %d", __PRETTY_FUNCTION__, m_state.appState);
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
  android_printf("%s: %d", __PRETTY_FUNCTION__, m_state.appState);
  // TODO
}

void CXBMCApp::onDestroyWindow()
{
  android_printf("%s: %d", __PRETTY_FUNCTION__, m_state.appState);
  // TODO
  if (m_state.appState < Paused)
    setAppState(Paused);
}

void CXBMCApp::onGainFocus()
{
  android_printf("%s: %d", __PRETTY_FUNCTION__, m_state.appState);
  // TODO: Probably nothing as everything is handled in onActivate()
}

void CXBMCApp::onLostFocus()
{
  android_printf("%s: %d", __PRETTY_FUNCTION__, m_state.appState);
  switch (m_state.appState)
  {
    case Initialized:
    case Rendering:
      m_state.xbmcPause(true);
      setAppState(Unfocused);
      break;

    // TODO

    default:
      break;
  }
}

bool CXBMCApp::onKeyboardEvent(AInputEvent* event)
{
  android_printf("%s: %d", __PRETTY_FUNCTION__, m_state.appState);
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
  android_printf("%s: %d", __PRETTY_FUNCTION__, m_state.appState);
  if (event == NULL)
    return false;

  size_t numPointers = AMotionEvent_getPointerCount(event);
  if (numPointers <= 0)
    return false;

  int32_t eventAction = AMotionEvent_getAction(event);
  int8_t touchAction = eventAction & AMOTION_EVENT_ACTION_MASK;
  size_t touchPointer = eventAction >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;

  float x = AMotionEvent_getX(event, touchPointer);
  float y = AMotionEvent_getY(event, touchPointer);
  float size = AMotionEvent_getTouchMinor(event, touchPointer);
  int64_t time = AMotionEvent_getEventTime(event);

  switch (touchAction)
  {
    case AMOTION_EVENT_ACTION_CANCEL:
      android_printf(" => touch cancel (%f, %f)", x, y);
      if (m_touchGestureState == TouchGesturePan ||
          m_touchGestureState >= TouchGestureMultiTouchStart)
      {
        // TODO: can we actually cancel these gestures?
      }

      m_touchGestureState = TouchGestureUnknown;
      for (unsigned int pointer = 0; pointer < TOUCH_MAX_POINTERS; pointer++)
        m_touchPointers[pointer].reset();
      // TODO: Should we return true?
      break;

    case AMOTION_EVENT_ACTION_DOWN:
    case AMOTION_EVENT_ACTION_POINTER_DOWN:
      android_printf(" => touch down (%f, %f, %d)", x, y, touchPointer);
      m_touchPointers[touchPointer].down.x = x;
      m_touchPointers[touchPointer].down.y = y;
      m_touchPointers[touchPointer].down.time = time;
      m_touchPointers[touchPointer].moving = false;
      m_touchPointers[touchPointer].size = size;

      // If this is the down event of the primary pointer
      // we start by assuming that it's a single touch
      if (touchAction == AMOTION_EVENT_ACTION_DOWN)
      {
        m_touchGestureState = TouchGestureSingleTouch;
        
        // Send a mouse motion event for getting the current guiitem selected
        m_state.xbmcTouch(XBMC_MOUSEMOTION, 0, (uint16_t)x, (uint16_t)y);
      }
      // Otherwise it's the down event of another pointer
      else if (numPointers > 1)
      {
        // If we so far assumed single touch or still have the primary
        // pointer of a previous multi touch pressed down, we can update to multi touch
        if (m_touchGestureState == TouchGestureSingleTouch ||
            m_touchGestureState == TouchGestureMultiTouchDone)
        {
          m_touchGestureState = TouchGestureMultiTouchStart;
        }
        // Otherwise we should ignore this pointer
        else
        {
          android_printf(" => ignore this pointer");
          m_touchPointers[touchPointer].reset();
          break;
        }
      }
      return true;

    case AMOTION_EVENT_ACTION_UP:
    case AMOTION_EVENT_ACTION_POINTER_UP:
      android_printf(" => touch up (%f, %f, %d)", x, y, touchPointer);
      if (!m_touchPointers[touchPointer].valid() ||
          m_touchGestureState == TouchGestureUnknown)
      {
        android_printf(" => abort touch up");
        break;
      }

      // Just a single tap with a pointer
      if (m_touchGestureState == TouchGestureSingleTouch)
      {
        android_printf(" => a single tap");
        m_state.xbmcTouch(XBMC_MOUSEBUTTONDOWN, XBMC_BUTTON_LEFT,
                          (uint16_t)m_touchPointers[touchPointer].down.x,
                          (uint16_t)m_touchPointers[touchPointer].down.y);
        m_state.xbmcTouch(XBMC_MOUSEBUTTONUP, XBMC_BUTTON_LEFT,
                          (uint16_t)m_touchPointers[touchPointer].down.x,
                          (uint16_t)m_touchPointers[touchPointer].down.y);
      }
      // A pan gesture started with a single pointer (ignoring any other pointers)
      else if (m_touchGestureState == TouchGesturePan)
      {
        android_printf(" => a pan gesture ending");
        float velocityX = 0.0f; // number of pixels per second
        float velocityY = 0.0f; // number of pixels per second
        int64_t timeDiff = time - m_touchPointers[touchPointer].down.time;
        if (timeDiff > 0)
        {
          velocityX = ((x - m_touchPointers[touchPointer].down.x) * 1000000000) / timeDiff;
          velocityY = ((y - m_touchPointers[touchPointer].down.y) * 1000000000) / timeDiff;
        }
        m_state.xbmcTouchGesture(ACTION_GESTURE_END, velocityX, velocityY, x, y);
      }
      // A single tap with multiple pointers
      else if (m_touchGestureState == TouchGestureMultiTouchStart)
      {
        android_printf(" => a single tap with multiple pointers");
        // we send a right-click for this and always use the coordinates
        // of the primary pointer
        m_state.xbmcTouch(XBMC_MOUSEBUTTONDOWN, XBMC_BUTTON_RIGHT,
                          (uint16_t)m_touchPointers[0].down.x,
                          (uint16_t)m_touchPointers[0].down.y);
        m_state.xbmcTouch(XBMC_MOUSEBUTTONUP, XBMC_BUTTON_RIGHT,
                          (uint16_t)m_touchPointers[0].down.x,
                          (uint16_t)m_touchPointers[0].down.y);
      }
      else if (m_touchGestureState == TouchGestureMultiTouch)
      {
        android_printf(" => a multi touch gesture ending");
      }
      
      if (touchAction == AMOTION_EVENT_ACTION_UP)
      {
        // Send a mouse motion event pointing off the screen
        // to deselect the selected gui item
        m_state.xbmcTouch(XBMC_MOUSEMOTION, 0, -1, -1);
      }

      // If we were in multi touch mode and lifted the only secondary pointer
      // we can go into the TouchGestureMultiTouchDone state which will allow
      // the user to go back into multi touch mode without lifting the primary finger
      if ((m_touchGestureState == TouchGestureMultiTouchStart || m_touchGestureState == TouchGestureMultiTouch) &&
          numPointers == 2 && touchAction == AMOTION_EVENT_ACTION_POINTER_UP)
      {
        m_touchGestureState = TouchGestureMultiTouchDone;
      }
      // Otherwise abort
      else
      {
        m_touchGestureState == TouchGestureUnknown;
      }
      
      m_touchPointers[touchPointer].reset();
      return true;

    case AMOTION_EVENT_ACTION_MOVE:
      android_printf(" => touch move (%f, %f)", x, y);
      if (!m_touchPointers[touchPointer].valid() ||
          m_touchGestureState == TouchGestureUnknown ||
          m_touchGestureState == TouchGestureMultiTouchDone)
      {
        android_printf(" => abort touch move");
        break;
      }
      
      if (m_touchGestureState == TouchGestureSingleTouch)
      {
        updateTouches(event);
        
        // Check if the touch has moved far enough to count as movement
        if (!m_touchPointers[touchPointer].moving)
        {
          android_printf(" => touch of size %f has not moved far enough => abort touch move", m_touchPointers[touchPointer].size);
          break;
        }
        
        android_printf(" => a pan gesture starts");
        m_state.xbmcTouchGesture(ACTION_GESTURE_BEGIN, 
                                 m_touchPointers[touchPointer].down.x,
                                 m_touchPointers[touchPointer].down.y,
                                 0.0f, 0.0f);
        m_touchPointers[touchPointer].last.copy(m_touchPointers[touchPointer].down);
        m_touchGestureState = TouchGesturePan;
      }
      else if (m_touchGestureState == TouchGestureMultiTouchStart)
      {
        android_printf(" => switching from TouchGestureMultiTouchStart to TouchGestureMultiTouch");
        m_touchGestureState = TouchGestureMultiTouch;
        
        updateTouches(event);
        for (unsigned int pointer = 0; pointer < TOUCH_MAX_POINTERS; pointer++)
          m_touchPointers[pointer].last.copy(m_touchPointers[pointer].current);
      }

      // Let's see if we have a pan gesture (i.e. the primary and only pointer moving)
      if (m_touchGestureState == TouchGesturePan)
      {
        android_printf(" => a pan gesture");
        m_state.xbmcTouchGesture(ACTION_GESTURE_PAN, x, y,
                                 x - m_touchPointers[touchPointer].last.x,
                                 y - m_touchPointers[touchPointer].last.y);
        m_touchPointers[touchPointer].last.x = x;
        m_touchPointers[touchPointer].last.y = y;
      }
      else if (m_touchGestureState == TouchGestureMultiTouch)
      {
        android_printf(" => a multi touch gesture");
        handleMultiTouchGesture(event);
      }
      else
        break;

      return true;

    default:
      android_printf(" => touch unknown (%f, %f)", x, y);
      break;
  }

  return false;
}

void CXBMCApp::handleMultiTouchGesture(AInputEvent *event)
{
  android_printf("%s", __PRETTY_FUNCTION__);
  // don't overwrite the last position as we need
  // it for rotating
  updateTouches(event, false);

  Pointer& primaryPointer = m_touchPointers[0];
  Pointer& secondaryPointer = m_touchPointers[1];

  // calculate zoom/pinch
  CVector primary = primaryPointer.down;
  CVector secondary = secondaryPointer.down;
  CVector diagonal = primary - secondary;
  float baseDiffLength = diagonal.length();
  if (baseDiffLength != 0.0f)
  {
    CVector primaryNow = primaryPointer.current;
    CVector secondaryNow = secondaryPointer.current;
    CVector diagonalNow = primaryNow - secondaryNow;
    float curDiffLength = diagonalNow.length();

    float centerX = (primary.x + secondary.x) / 2;
    float centerY = (primary.y + secondary.y) / 2;

    float zoom = curDiffLength / baseDiffLength;

    m_state.xbmcTouchGesture(ACTION_GESTURE_ZOOM, centerX, centerY, zoom, 0);
  }
}

void CXBMCApp::updateTouches(AInputEvent *event, bool saveLast /* = true */)
{
  for (unsigned int pointer = 0; pointer < TOUCH_MAX_POINTERS; pointer++)
  {
    if (saveLast)
      m_touchPointers[pointer].last.copy(m_touchPointers[pointer].current);
    
    m_touchPointers[pointer].current.x = AMotionEvent_getX(event, pointer);
    m_touchPointers[pointer].current.y = AMotionEvent_getY(event, pointer);
    m_touchPointers[pointer].current.time = AMotionEvent_getEventTime(event);
    
    // Calculate whether the pointer has moved at all
    if (!m_touchPointers[pointer].moving)
    {
      CVector down = m_touchPointers[pointer].down;
      CVector current = m_touchPointers[pointer].current;
      CVector distance = down - current;
      
      if (distance.length() > m_touchPointers[pointer].size)
        m_touchPointers[pointer].moving = true;
    }
  }
}

void CXBMCApp::run()
{
  android_printf("%s", __PRETTY_FUNCTION__);
  if (m_state.xbmcInitialize == NULL || m_state.xbmcRun == NULL)
  {
    android_printf(" => invalid XBMC_Initialize or XBMC_Run instance");
    setAppState(Error);
    return;
  }

  int status = m_state.xbmcInitialize(m_state.platform, 0, NULL);
  if (status == 0)
  {
    setAppState(Initialized);

    android_printf(" => running XBMC_Run...");
    try
    {
      setAppState(Rendering);
      status = m_state.xbmcRun();
      android_printf(" => XBMC_Run finished with %d", status);
    }
    catch(...)
    {
      android_printf("ERROR: Exception caught on main loop. Exiting");
      setAppState(Error);
    }
  }
  else
  {
    android_printf("XBMC_Initialize failed");
    setAppState(Error);
  }    

  bool finishActivity = false;
  pthread_mutex_lock(&m_state.mutex);
  finishActivity = m_state.appState != Stopping;
  m_state.appState = Stopped;
  pthread_mutex_unlock(&m_state.mutex);
  
  if (finishActivity)
  {
    android_printf(" => calling ANativeActivity_finish()");
    ANativeActivity_finish(m_activity);
  }
}

void CXBMCApp::stop()
{
  android_printf("%s", __PRETTY_FUNCTION__);

  pthread_mutex_lock(&m_state.mutex);
  if (m_state.appState < Stopped)
  {
    m_state.appState = Stopping;
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

void CXBMCApp::setAppState(AppState state)
{
  pthread_mutex_lock(&m_state.mutex);
  m_state.appState = state;
  pthread_mutex_unlock(&m_state.mutex);
}
