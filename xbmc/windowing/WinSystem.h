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

#ifndef WINDOW_SYSTEM_BASE_H
#define WINDOW_SYSTEM_BASE_H

#include "WinEvents.h"
#include "guilib/Resolution.h"
#include <vector>

typedef enum _WindowSystemType
{
  WINDOW_SYSTEM_WIN32,
  WINDOW_SYSTEM_OSX,
  WINDOW_SYSTEM_IOS,
  WINDOW_SYSTEM_X11,
  WINDOW_SYSTEM_SDL,
  WINDOW_SYSTEM_EGL,
  WINDOW_SYSTEM_ANDROID
} WindowSystemType;

struct RESOLUTION_WHR
{
  int width;
  int height;
  int ResInfo_Index;
};

struct REFRESHRATE
{
  float RefreshRate;
  bool  Interlaced;
  int   ResInfo_Index;
};

class CWinSystemBase
{
public:
  CWinSystemBase();
  virtual ~CWinSystemBase();
  WindowSystemType GetWinSystem() { return m_eWindowSystem; }

  // windowing interfaces
  virtual bool InitWindowSystem();
  virtual bool DestroyWindowSystem(){ return false; }
  virtual bool CreateNewWindow(const CStdString& name, bool fullScreen, RESOLUTION_INFO& res, PHANDLE_EVENT_FUNC userFunction) = 0;
  virtual bool DestroyWindow(bool tryToPreserveContext = false){ return false; }
  virtual bool ResizeWindow(int newWidth, int newHeight, int newLeft, int newTop) = 0;
  virtual bool SetFullScreen(bool fullScreen, RESOLUTION_INFO& res, bool blankOtherDisplays) = 0;
  virtual bool MoveWindow(int topLeft, int topRight){return false;}
  virtual bool CenterWindow(){return false;}
  virtual bool IsCreated(){ return m_bWindowCreated; }
  virtual void NotifyAppFocusChange(bool bGaining) {}
  virtual void NotifyAppActiveChange(bool bActivated) {}
  virtual void ShowOSMouse(bool show) {};
  virtual bool HasCursor(){ return true; }
  //some plattforms have api for gesture inertial scrolling - default to false and use the InertialScrollingHandler
  virtual bool HasInertialGestures(){ return false; }

  virtual bool Minimize() { return false; }
  virtual bool Restore() { return false; }
  virtual bool Hide() { return false; }
  virtual bool Show(bool raise = true) { return false; }

  // notifications
  virtual void OnMove(int x, int y) {}

  // OS System screensaver
  virtual void EnableSystemScreenSaver(bool bEnable) {};
  virtual bool IsSystemScreenSaverEnabled() {return false;}
  virtual void ResetOSScreensaver() {};

  // resolution interfaces
  unsigned int GetWidth() { return m_nWidth; }
  unsigned int GetHeight() { return m_nHeight; }
  virtual int GetNumScreens() { return 0; }
  virtual int GetCurrentScreen() { return 0; }
  bool IsFullScreen() { return m_bFullScreen; }
  virtual void UpdateResolutions();
  void SetWindowResolution(int width, int height);
  int DesktopResolution(int screen);
  std::vector<RESOLUTION_WHR> ScreenResolutions(int screen);
  std::vector<REFRESHRATE> RefreshRates(int screen, int width, int height);
  REFRESHRATE DefaultRefreshRate(int screen, std::vector<REFRESHRATE> rates);

protected:
  void UpdateDesktopResolution(RESOLUTION_INFO& newRes, int screen, int width, int height, float refreshRate, uint32_t dwFlags = 0);

  WindowSystemType  m_eWindowSystem;
  int               m_nWidth;
  int               m_nHeight;
  int               m_nTop;
  int               m_nLeft;
  bool              m_bWindowCreated;
  bool              m_bFullScreen;
  int               m_nScreen;
  bool              m_bBlankOtherDisplay;
};


#endif // WINDOW_SYSTEM_H
