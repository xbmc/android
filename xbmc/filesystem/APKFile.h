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
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "IFile.h"
#include "URL.h"

struct zip;
struct zip_file;

namespace XFILE
{
  class CAPKFile : public IFile
  {
  public:
    CAPKFile();
    virtual ~CAPKFile();
    virtual bool Open(const CURL& url);
    virtual void Close();
    virtual bool Exists(const CURL& url);

    virtual int64_t Seek(int64_t iFilePosition, int iWhence = SEEK_SET);
    virtual unsigned int Read(void* lpBuf, int64_t uiBufSize);
    virtual int Stat(struct __stat64* buffer);
    virtual int Stat(const CURL& url, struct __stat64* buffer);
    virtual int64_t GetLength();
    virtual int64_t GetPosition();
    virtual int GetChunkSize();
  protected:
    CURL              m_url;
    int               m_zip_index;
    int64_t           m_file_pos;
    int64_t           m_file_size;
    struct zip_file   *m_zip_file;
    struct zip        *m_zip_archive;
  };
}
