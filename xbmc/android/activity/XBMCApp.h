#pragma once
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

#include <math.h>
#include <pthread.h>

#include <android/native_activity.h>

#include "IActivityHandler.h"
#include "IInputHandler.h"

#include "xbmc.h"

#define TOUCH_MAX_POINTERS  2

class CXBMCApp : public IActivityHandler, public IInputHandler
{
public:
  CXBMCApp(ANativeActivity *nativeActivity);
  virtual ~CXBMCApp();

  bool isValid() { return m_activity != NULL &&
                          m_state.platform != NULL &&
                          m_state.xbmcRun != NULL; }

  ActivityResult onActivate();
  void onDeactivate();

  void onStart();
  void onResume();
  void onPause();
  void onStop();
  void onDestroy();

  void onSaveState(void **data, size_t *size);
  void onConfigurationChanged();
  void onLowMemory();

  void onCreateWindow(ANativeWindow* window);
  void onResizeWindow();
  void onDestroyWindow();
  void onGainFocus();
  void onLostFocus();

  bool onKeyboardEvent(AInputEvent* event);
  bool onTouchEvent(AInputEvent* event);

private:
  jobject getWakeLock();
  void acquireWakeLock();
  void releaseWakeLock();
  void run();
  void stop();
  
  ANativeActivity *m_activity;
  jobject m_wakeLock;
  
  typedef enum {
    // XBMC_Initialize hasn't been executed yet
    Uninitialized,
    // XBMC_Initialize has been successfully executed
    Initialized,
    // XBMC is currently rendering
    Rendering,
    // XBMC has stopped rendering because it has lost focus
    // but it still has an EGLContext
    Unfocused,
    // XBMC has been paused/stopped and does not have an
    // EGLContext
    Paused,
    // XBMC is being stopped
    Stopping,
    // XBMC has stopped
    Stopped,
    // An error has occured
    Error
  } AppState;

  typedef struct {
    pthread_t thread;
    pthread_mutex_t mutex;
    AppState appState;

    XBMC_PLATFORM* platform;
    XBMC_Run_t xbmcRun;
    XBMC_Pause_t xbmcPause;
    XBMC_Stop_t xbmcStop;
    XBMC_Key_t xbmcKey;
    XBMC_Touch_t xbmcTouch;
    XBMC_TouchGesture_t xbmcTouchGesture;
    XBMC_SetupDisplay_t xbmcSetupDisplay;
    XBMC_DestroyDisplay_t xbmcDestroyDisplay;
  } State;

  State m_state;
  
  void setAppState(AppState state);
    
  class Touch {
    public:
      Touch() { reset(); }
      
      bool valid() const { return x >= 0.0f && y >= 0.0f && time >= 0; }
      void reset() { x = -1.0f; y = -1.0f; time = -1; }
      void copy(const Touch &other) { x = other.x; y = other.y; time = other.time; }
      
      float x;      // in pixels (With possible sub-pixels)
      float y;      // in pixels (With possible sub-pixels)
      int64_t time; // in nanoseconds
  };
  
  class Pointer {
    public:
      Pointer() { reset(); }
      
      bool valid() const { return down.valid(); }
      void reset() { down.reset(); last.reset(); moving = false; size = 0.0f; }
      
      Touch down;
      Touch last;
      Touch current;
      bool moving;
      float size;
  };
  
  Pointer m_touchPointers[TOUCH_MAX_POINTERS];
  
  class CVector {
    public:
      CVector()
      : x(0.0f), y(0.0f)
      { }
      CVector(float xCoord, float yCoord)
      : x(xCoord), y(yCoord)
      { }
      CVector(const CXBMCApp::Touch &touch)
      : x(touch.x), y(touch.y)
      { }
      
      const CVector operator+(const CVector &other) const { return CVector(x + other.x, y + other.y); }
      const CVector operator-(const CVector &other) const { return CVector(x - other.x, y - other.y); }
      
      float scalar(const CVector &other) { return x * other.x + y * other.y; }
      float length() { return sqrt(pow(x, 2) + pow(y, 2)); }
      
      float x;
      float y;
  };
  
  typedef enum {
    TouchGestureUnknown = 0,
    // only primary pointer active but stationary so far
    TouchGestureSingleTouch,
    // primary pointer moving
    TouchGesturePan,
    // at least two pointers active but stationary so far
    TouchGestureMultiTouchStart,
    // at least two pointers active and moving
    TouchGestureMultiTouch,
    // all but primary pointer have been lifted
    TouchGestureMultiTouchDone
  } TouchGestureState;
  
  TouchGestureState m_touchGestureState;
  
  void handleMultiTouchGesture(AInputEvent *event);
  void updateTouches(AInputEvent *event, bool saveLast = true);
};

