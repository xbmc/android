/*
 *      Copyright (C) 2005-2012 Team XBMC
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
#pragma once
#include <stdlib.h>
#include <vector>
#include <string>
#include "threads/SingleLock.h"

typedef std::vector<std::string> strings;
typedef std::vector<void *> handles;

struct solibdep
{
  void* handle;
  std::string filename;
};
typedef std::vector<solibdep> solibdeps;

struct solib
{
  std::string filename;
  void* handle;
  int refcount;
  solibdeps deps;
};

typedef std::vector<solib> loaded;

void *xb_dlopen(const char *library);
int xb_dlclose(void *handle);
static loaded m_xblibs;
static CCriticalSection xb_CritSection;
