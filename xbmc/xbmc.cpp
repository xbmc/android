/*
 *      Copyright (C) 2005-2008 Team XBMC
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

#include "Application.h"
#include "settings/AdvancedSettings.h"
#include "utils/log.h"
#include "xbmc.h"
#include "windowing/WinEvents.h"
#include "guilib/GUIWindowManager.h"

int XBMC_Run(bool renderGUI, XBMC_PLATFORM *platform)
{
  int status = -1;

  if (!g_advancedSettings.Initialized())
    g_advancedSettings.Initialize();

  if (!g_application.Create())
  {
    fprintf(stderr, "ERROR: Unable to create application. Exiting\n");
    return status;
  }
  if (renderGUI && !g_application.CreateGUI())
  {
    fprintf(stderr, "ERROR: Unable to create GUI. Exiting\n");
    return status;
  }
  if (!g_application.Initialize())
  {
    fprintf(stderr, "ERROR: Unable to Initialize. Exiting\n");
    return status;
  }

  try
  {
    status = g_application.Run(renderGUI);
  }
  catch(...)
  {
    fprintf(stderr, "ERROR: Exception caught on main loop. Exiting\n");
    status = -1;
  }

  return status;
}

extern "C" void XBMC_Pause(bool pause)
{
  CLog::Log(LOGDEBUG, "XBMC_Pause(%s)", pause ? "true" : "false");
  // Only send the PAUSE action if we are pausing XBMC and something is currently playing
  if (pause && g_application.IsPlaying() && !g_application.IsPaused())
    g_application.getApplicationMessenger().SendAction(CAction(ACTION_PAUSE), WINDOW_INVALID, true);

  g_application.m_AppActive = g_application.m_AppFocused = !pause;
}

extern "C" void XBMC_Stop()
{
  g_application.getApplicationMessenger().Quit();
}

extern "C" void XBMC_Key(uint8_t code, uint16_t key, uint16_t modifiers, bool up)
{
  XBMC_Event newEvent;
  memset(&newEvent, 0, sizeof(newEvent));

  unsigned char type = up ? XBMC_KEYUP : XBMC_KEYDOWN;
  newEvent.type = type;
  newEvent.key.type = type;
  newEvent.key.keysym.scancode = code;
  newEvent.key.keysym.sym = (XBMCKey)key;
  newEvent.key.keysym.unicode = key;
  newEvent.key.keysym.mod = (XBMCMod)modifiers;

  CLog::Log(LOGDEBUG, "XBMC_Key(%u, %u, 0x%04X, %d)", code, key, modifiers, up);
  CWinEvents::MessagePush(&newEvent);
}

extern "C" void XBMC_Touch(uint8_t type, uint8_t button, uint16_t x, uint16_t y)
{
  XBMC_Event newEvent;
  memset(&newEvent, 0, sizeof(newEvent));

  newEvent.type = type;
  newEvent.button.type = type;
  newEvent.button.button = button;
  newEvent.button.x = x;
  newEvent.button.y = y;

  CLog::Log(LOGDEBUG, "XBMC_Touch(%u, %u, %u, %u)", type, button, x, y);
  CWinEvents::MessagePush(&newEvent);
}

extern "C" void XBMC_TouchGesture(int32_t action, float posX, float posY, float offsetX, float offsetY)
{
  CLog::Log(LOGDEBUG, "XBMC_TouchGesture(%d, %f, %f, %f, %f)", action, posX, posY, offsetX, offsetY);
  if (action == ACTION_GESTURE_BEGIN)
    g_application.getApplicationMessenger().SendAction(CAction(action, 0, posX, posY, 0, 0), WINDOW_INVALID, false);
  else if (action == ACTION_GESTURE_PAN)
    g_application.getApplicationMessenger().SendAction(CAction(action, 0, posX, posY, offsetX, offsetY), WINDOW_INVALID, false);
  else if (action == ACTION_GESTURE_END)
    g_application.getApplicationMessenger().SendAction(CAction(action, 0, posX, posY, offsetX, offsetY), WINDOW_INVALID, false);
  else if (action == ACTION_GESTURE_ZOOM)
    g_application.getApplicationMessenger().SendAction(CAction(action, 0, posX, posY, offsetX, 0), WINDOW_INVALID, false);
}

extern "C" int XBMC_TouchGestureCheck(float posX, float posY)
{
  CLog::Log(LOGDEBUG, "XBMC_TouchGestureCheck(%f, %f)", posX, posY);
  CGUIMessage message(GUI_MSG_GESTURE_NOTIFY, 0, 0, posX, posY);
  if (g_windowManager.SendMessage(message))
    return message.GetParam1();

  return EVENT_RESULT_UNHANDLED;
}

extern "C" bool XBMC_SetupDisplay()
{
  CLog::Log(LOGDEBUG, "XBMC_SetupDisplay()");
  return g_application.getApplicationMessenger().SetupDisplay();
}

extern "C" bool XBMC_DestroyDisplay()
{
  CLog::Log(LOGDEBUG, "XBMC_DestroyDisplay()");
  return g_application.getApplicationMessenger().DestroyDisplay();
}
