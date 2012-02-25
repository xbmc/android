/*
 *      Copyright (C) 2011 Team XBMC
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
//#ifdef HAS_EGLGLES

#include "WinBindingAndroid.h"
#include "WinSystemAndroid.h"

#include <vector>

#define FALSE false

////////////////////////////////////////////////////////////////////////////////////////////
CWinSystemAndroid::CWinSystemAndroid()
{
  m_window = NULL;
  m_eglBinding = new CWinBindingAndroid();
  //m_eWindowSystem = WINDOW_SYSTEM_EGL;
}

CWinSystemAndroid::~CWinSystemAndroid()
{
  DestroyWindowSystem();
  delete m_eglBinding;
}

bool CWinSystemAndroid::InitWindowSystem( android_app* androidApp )
{
  m_androidApp = androidApp;

  m_fb_width  = 1280;
  m_fb_height = 720;
  m_fb_bpp    = 8;

  //CLog::Log(LOGDEBUG, "Video mode: %dx%d with %d bits per pixel.",
  //  m_fb_width, m_fb_height, m_fb_bpp);

  m_display = EGL_DEFAULT_DISPLAY;

  /*
  m_window  = (fbdev_window*)calloc(1, sizeof(fbdev_window));
	m_window->width  = m_fb_width;
	m_window->height = m_fb_height;
	*/

  return true;
}

bool CWinSystemAndroid::DestroyWindowSystem()
{
	/*
  free(m_window);
  m_window = NULL;
  */

  return true;
}

bool CWinSystemAndroid::CreateNewWindow(const CStdString& name /*, bool fullScreen, RESOLUTION_INFO& res, PHANDLE_EVENT_FUNC userFunction */ )
{
	/*
  m_nWidth  = res.iWidth;
  m_nHeight = res.iHeight;
  m_bFullScreen = fullScreen;

  m_bWindowCreated = true;
  */

  if (!m_eglBinding->CreateWindow(m_display, m_androidApp->window))
    return false;

  return true;
}

bool CWinSystemAndroid::DestroyWindow()
{
  if (!m_eglBinding->DestroyWindow())
    return false;

  /*
  m_bWindowCreated = false;
   */

  return true;
}

bool CWinSystemAndroid::ResizeWindow(int newWidth, int newHeight, int newLeft, int newTop)
{
	/*
  CRenderSystemGLES::ResetRenderSystem(newWidth, newHeight, true, 0);
  */
  return true;
}

#if 0
bool CWinSystemAndroid::SetFullScreen(bool fullScreen, RESOLUTION_INFO& res, bool blankOtherDisplays)
{
	/*
  CLog::Log(LOGDEBUG, "CWinSystemDFB::SetFullScreen");
  m_nWidth  = res.iWidth;
  m_nHeight = res.iHeight;
  m_bFullScreen = fullScreen;

  m_eglBinding->ReleaseSurface();
  CreateNewWindow("", fullScreen, res, NULL);

  CRenderSystemGLES::ResetRenderSystem(res.iWidth, res.iHeight, true, 0);
  */

  return true;
}
#endif

void CWinSystemAndroid::UpdateResolutions()
{
	/*
  CWinSystemBase::UpdateResolutions();

  int w = 1280;
  int h = 720;
  UpdateDesktopResolution(g_settings.m_ResInfo[RES_DESKTOP], 0, w, h, 0.0);
  */
}

bool CWinSystemAndroid::IsExtSupported(const char* extension)
{
  if(strncmp(extension, "EGL_", 4) != 0)

    //return CRenderSystemGLES::IsExtSupported(extension);

  return m_eglBinding->IsExtSupported(extension);
}

bool CWinSystemAndroid::PresentRenderImpl(const CDirtyRegionList &dirty)
{
  m_eglBinding->SwapBuffers();

  return true;
}

void CWinSystemAndroid::SetVSyncImpl(bool enable)
{
  //m_iVSyncMode = enable ? 10 : 0;
  if (m_eglBinding->SetVSync(enable) == FALSE)
    ;//CLog::Log(LOGERROR, "CWinSystemDFB::SetVSyncImpl: Could not set egl vsync");
}

void CWinSystemAndroid::ShowOSMouse(bool show)
{
}

void CWinSystemAndroid::NotifyAppActiveChange(bool bActivated)
{
}

bool CWinSystemAndroid::Minimize()
{
  Hide();
  return true;
}

bool CWinSystemAndroid::Restore()
{
  Show(true);
  return false;
}

bool CWinSystemAndroid::Hide()
{
  return true;
}

bool CWinSystemAndroid::Show(bool raise)
{
  return true;
}

//#endif
