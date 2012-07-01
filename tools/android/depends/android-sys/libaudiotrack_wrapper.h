#pragma once

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


#ifndef __AUDIOTRACK_WRAPPER_H__
#define __AUDIOTRACK_WRAPPER_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef void* ATW_ctx;
typedef int (*ATWCallback)(void *ctx, void *buffer, size_t size);

ATW_ctx ATW_create(void);
void    ATW_release(ATW_ctx ctx);
int     ATW_get_min_framecount(ATW_ctx ctx, int* frameCount, int sampleRate);
int     ATW_set(ATW_ctx ctx, int channelCount, int sampleRate, int frameCount,
          ATWCallback cbf, void* cbf_ctx);
void    ATW_start(ATW_ctx ctx);
void    ATW_stop( ATW_ctx ctx);
int     ATW_write(ATW_ctx ctx, const void* buffer, size_t size);
void    ATW_flush(ATW_ctx ctx);
void    ATW_pause(ATW_ctx ctx);
void    ATW_mute( ATW_ctx ctx, int mute);
int     ATW_muted(ATW_ctx ctx);
void    ATW_set_volume(ATW_ctx ctx, float left, float right);
int     ATW_latency(ATW_ctx ctx);

#ifdef __cplusplus
}
#endif

#endif /* __AUDIOTRACK_WRAPPER_H__ */
