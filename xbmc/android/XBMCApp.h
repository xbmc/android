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

#include "IActivityHandler.h"
#include "IInputHandler.h"

#include "xbmc.h"

class CXBMCApp : public IActivityHandler, public IInputHandler
{
public:
  CXBMCApp();
  virtual ~CXBMCApp();

  bool isValid() { return m_state.platform != NULL &&
                          m_state.xbmcInitialize != NULL &&
                          m_state.xbmcStep != NULL; }

  ActivityResult onActivate();
  void onDeactivate();
  ActivityResult onStep();

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
  void stop();

  typedef struct {
    bool initialized;

    XBMC_PLATFORM* platform;

    XBMC_Initialize_t xbmcInitialize;
    XBMC_RunStep_t xbmcStep;
    XBMC_Stop_t xbmcStop;
    XBMC_Touch_t xbmcTouch;
    XBMC_Key_t xbmcKey;
  } State;

  State m_state;
};

