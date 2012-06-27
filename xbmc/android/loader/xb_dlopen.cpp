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
#include <sys/stat.h>
#include <stdlib.h>
#include <string>
#include <elf.h>
#include <errno.h>
#include <vector>
#include <dlfcn.h>
#include <fcntl.h>
#include "xb_dlopen.h"
#include "android/activity/XBMCApp.h"


using namespace std;

string getdirname(const string &path)
{
  string dirname = path.substr(0, path.find_last_of('/') + 1);
  while (dirname[dirname.length()-1] == '/')
    dirname.erase(dirname.length()-1);
  return dirname;
}

string getfilename (const string &path)
{
  return path.substr(path.find_last_of('/') +1);
}

string getXBfilename(const string &path)
{
  string filename = getfilename(path);
  if (filename.find("libxb") == 0)
    return filename;
  return "libxb" + filename.substr(3);
}

string getXBpath(const string &path)
{
  return getdirname(path) + "/" + getXBfilename(path);
}

void* finddephandle(solib *lib, const string &filename)
{
  for (solibdeps::iterator i = lib->deps.begin(); i != lib->deps.end(); i++)
  {
    if (i->filename == filename)
      return i->handle;
  }
  return NULL;
}

string finddep(const string &filename, const string &extrapath)
{
  struct stat st;
  strings searchpaths;
  string path;
  if (extrapath.length() > 0)
    searchpaths.push_back(extrapath);

  string tempPath = getenv("XBMC_TEMP");
  searchpaths.push_back(getenv("XBMC_ANDROID_LIBS"));
  searchpaths.push_back(tempPath + "/apk/assets/system/players/dvdplayer");
  searchpaths.push_back(tempPath + "/temp/assets/system/players/dvdplayer");
  
  // extract all the paths to system library locations
  string systemLibs = getenv("XBMC_ANDROID_SYSTEM_LIBS");
  while (true)
  {
    size_t pos = systemLibs.find(":");
    searchpaths.push_back(systemLibs.substr(0, pos));
    
    if (pos != string::npos)
      systemLibs.erase(0, pos + 1);
    else
      break;
  }

  for (strings::iterator j = searchpaths.begin(); j != searchpaths.end(); ++j)
  {
    if ((j != searchpaths.begin()) && (*j == searchpaths[0]))
      continue;
    path = (*j+"/"+getXBfilename(filename));
    if (stat((path).c_str(), &st) == 0)
      return(path);
  }

  for (strings::iterator j = searchpaths.begin(); j != searchpaths.end(); ++j)
  {
    if ((j != searchpaths.begin()) && (*j == searchpaths[0]))
      continue;
    path = (*j+"/"+getfilename(filename));
    if (stat((path).c_str(), &st) == 0)
      return(path);
  }

  return "";
}

solib* findinlibs(const string &search)
{
  for(loaded::const_iterator i = m_xblibs.begin(); i!= m_xblibs.end(); ++i)
  {
    if (i->filename == getfilename(search))
      return (solib *)&*i;
  }
  return NULL;
}

void needs(string filename, strings *results)
{
  Elf32_Ehdr header;
  char *data = NULL;
  int fd, i;

  fd = open(filename.c_str(), O_RDONLY);
  if(fd < 0)
  {
    CXBMCApp::android_printf("Cannot open %s: %s\n", filename.c_str(), strerror(errno));
    return;
  }

  if(read(fd, &header, sizeof(header)) < 0)
  {
    CXBMCApp::android_printf("Cannot read elf header: %s\n", strerror(errno));
    return;
  }

  lseek(fd, header.e_shoff, SEEK_SET);

  for(i = 0; i < header.e_shnum; i++)
  {
    Elf32_Shdr sheader;

    lseek(fd, header.e_shoff + (i * header.e_shentsize), SEEK_SET);
    read(fd, &sheader, sizeof(sheader));

    if(sheader.sh_type == SHT_DYNSYM)
    {
      Elf32_Shdr symheader;
      lseek(fd, header.e_shoff + (sheader.sh_link * header.e_shentsize), SEEK_SET);
      read(fd, &symheader, sizeof(Elf32_Shdr));
      lseek(fd, symheader.sh_offset, SEEK_SET);
      data = (char*)malloc(symheader.sh_size);
      read(fd, data, symheader.sh_size);
      break;
    }
  }

  if(!data)
    return;

  for(i = 0; i < header.e_shnum; i++)
  {
    Elf32_Shdr sheader;

    lseek(fd, header.e_shoff + (i * header.e_shentsize), SEEK_SET);
    read(fd, &sheader, sizeof(Elf32_Shdr));

    if (sheader.sh_type == SHT_DYNAMIC)
    {
      unsigned int j;

      lseek(fd, sheader.sh_offset, SEEK_SET);
      for(j = 0; j < sheader.sh_size / sizeof(Elf32_Dyn); j++)
      {
        Elf32_Dyn cur;
        read(fd, &cur, sizeof(Elf32_Dyn));
        if(cur.d_tag == DT_NEEDED)
        {
          char *final = data + cur.d_un.d_val;
          results->push_back(final);
        }
      }
    }
  }
  return;
}

void* xb_dlopen_internal(const char * path, solib *lib)
{
  strings deps;
  string deppath;
  void *handle;


  deps.clear();

  // Don't traverse into libxbmc's deps, they're guaranteed to be loaded.
  if (getfilename(path) != "libxbmc.so")
    needs(path, &deps);

  for (strings::iterator j = deps.begin(); j != deps.end(); ++j)
  {
    if (findinlibs(*j) != NULL)
      continue;

    if (finddephandle(lib, getfilename(path)) != NULL)
      continue;

    deppath = finddep(*j, getdirname(path));

    // Recurse for each needed lib
    xb_dlopen_internal(deppath.c_str(), lib);
  }

  handle = dlopen(path, RTLD_LOCAL);

  if (handle == NULL)
    CXBMCApp::android_printf("xb_dlopen: Error from dlopen(%s): %s", path, dlerror());

  solibdep dep;
  dep.handle = handle;
  dep.filename = getfilename(path);
  lib->deps.push_back(dep);
  return handle;
}

void* xb_dlopen(const char * path)
{
  solib *lib;
  void *handle;

  lib = findinlibs(getfilename(path));
  if (lib)
  {
    lib->refcount++;
    return lib->handle;
  }

  lib = new solib;
  lib->filename = getfilename(path);
  lib->refcount = 1;
  handle = xb_dlopen_internal(path, lib);
  if (handle != NULL)
  {
    lib->handle = handle;
    m_xblibs.push_back(*lib);
    delete lib;
    return handle;
  }

  CXBMCApp::android_printf("xb_dlopen: failed to open: %s", path);
  delete lib;
  return NULL;
}

int xb_dlclose(void* handle)
{
  for (loaded::iterator i = m_xblibs.begin(); i != m_xblibs.end(); i++)
  {
    if (i->handle == handle)
    {
      if (--(i->refcount) > 0 )
        return 0;
      for (solibdeps::iterator j = i->deps.begin(); j != i->deps.end(); j++)
      {
        if (dlclose(j->handle))
        {
          CXBMCApp::android_printf("could not close dep for: %s\n", i->filename.c_str());
          return 1;
        }
      }
      m_xblibs.erase(i);
      return 0;
    }
  }

  CXBMCApp::android_printf("xb_dlclose: unable to locate handle");
  return 1;
}
