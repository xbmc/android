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
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */


// XBMC
//
// libraries:
//   - CDRipX   : doesnt support section loading yet
//   - xbfilezilla : doesnt support section loading yet
//

#include "system.h"
#include "Application.h"
#include "PlayListPlayer.h"
#include "settings/AppParamParser.h"
#include "settings/AdvancedSettings.h"
#include "utils/log.h"

#if defined(TARGET_LINUX) && defined(DEBUG)
#include <sys/resource.h>
#include <signal.h>
#endif
#if defined(TARGET_DARWIN)
#include "Util.h"
#endif
#include "windowing/WinEvents.h"
#include "guilib/Key.h"
#include "guilib/GUIWindowManager.h"

extern "C" int XBMC_Initialize(XBMC_PLATFORM *platform, int argc, const char** argv)
{
  g_application.PlatformInitialize(platform);

  //this can't be set from CAdvancedSettings::Initialize() because it will overwrite
  //the loglevel set with the --debug flag
#ifdef _DEBUG
  g_advancedSettings.m_logLevel     = LOG_LEVEL_DEBUG;
  g_advancedSettings.m_logLevelHint = LOG_LEVEL_DEBUG;
#else
  g_advancedSettings.m_logLevel     = LOG_LEVEL_NORMAL;
  g_advancedSettings.m_logLevelHint = LOG_LEVEL_NORMAL;
#endif
  CLog::SetLogLevel(g_advancedSettings.m_logLevel);

#ifdef TARGET_LINUX
#if defined(DEBUG)
  struct rlimit rlim;
  rlim.rlim_cur = rlim.rlim_max = RLIM_INFINITY;
  if (setrlimit(RLIMIT_CORE, &rlim) == -1)
    CLog::Log(LOGDEBUG, "Failed to set core size limit (%s)", strerror(errno));
#endif
  // Prevent child processes from becoming zombies on exit if not waited upon. See also Util::Command
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));

  sa.sa_flags = SA_NOCLDWAIT;
  sa.sa_handler = SIG_IGN;
  sigaction(SIGCHLD, &sa, NULL);
#endif
  setlocale(LC_NUMERIC, "C");
  g_advancedSettings.Initialize();

#ifndef TARGET_WINDOWS
  if ((platform->flags & XBMCRunPrimary) && argc > 0 && argv != NULL)
  {
    CAppParamParser appParamParser;
    appParamParser.Parse(argv, argc);
  }
#endif
  g_application.Preflight();

  if (!g_application.Create())
  {
    CLog::Log(LOGFATAL, "Unable to create application. Exiting");
    return 1;
  }
  return 0;
}

extern "C" int XBMC_Run()
{
  return g_application.Run();
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

