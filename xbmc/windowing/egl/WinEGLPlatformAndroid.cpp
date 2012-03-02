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

////////////////////////////////////////////////////////////////////////////////////////////
EGLNativeWindowType CWinEGLPlatformAndroid::InitWindowSystem(int width, int height, int bpp)
{
  // TODO:fetch and return the native android window here
  return 0;
}

void CWinEGLPlatformAndroid::DestroyWindowSystem(EGLNativeWindowType native_window)
{
}

bool CWinEGLPlatformAndroid::ClampToGUIDisplayLimits(int &width, int &height)
{
  // TODO:clamp to the native android window size
  bool rtn = false;
  if (width == 1920 && height == 1080)
  {
    width  = 1280;
    height = 720;
    rtn = true;
  }
  return rtn;
}

bool CWinEGLPlatformAndroid::ProbeDisplayResolutions(std::vector<CStdString> &resolutions)
{
  resolutions.clear();
  resolutions.push_back("1280x720p60Hz");
  return true;
}
