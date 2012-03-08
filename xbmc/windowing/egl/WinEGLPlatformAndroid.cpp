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

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/ioctl.h>

#include "WinEGLPlatformAndroid.h"
#include "Application.h"

////////////////////////////////////////////////////////////////////////////////////////////
EGLNativeWindowType CWinEGLPlatformAndroid::InitWindowSystem(int width, int height, int bpp)
{
  return (EGLNativeWindowType)g_application.GetPlatform()->window_type;
}

void CWinEGLPlatformAndroid::DestroyWindowSystem(EGLNativeWindowType native_window)
{
}

bool CWinEGLPlatformAndroid::ClampToGUIDisplayLimits(int &width, int &height)
{
  // clamp to the native android window size
  width  = g_application.GetPlatform()->width;
  height = g_application.GetPlatform()->height;
  return true;
}

bool CWinEGLPlatformAndroid::ProbeDisplayResolutions(std::vector<CStdString> &resolutions)
{
  resolutions.clear();

  CStdString resolution;
  // the only resolution we support is the native android window size
  resolution.Format("%dx%dp60Hz",
    g_application.GetPlatform()->width,
    g_application.GetPlatform()->height);

  resolutions.push_back(resolution);
  return true;
}

void CWinEGLPlatformAndroid::CreateWindowCallback()
{
  EGLint format;
  // EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
  // guaranteed to be accepted by ANativeWindow_setBuffersGeometry().
  // As soon as we picked a EGLConfig, we can safely reconfigure the
  // ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID.
  eglGetConfigAttrib(m_display, m_config, EGL_NATIVE_VISUAL_ID, &format);

  g_application.GetPlatform()->android_setBuffersGeometry(g_application.GetPlatform()->window_type, 0, 0, format);
}
