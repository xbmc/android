#pragma once
/*
 *      Copyright (C) 2010 Team XBMC
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

#include "system_gl.h"

#include "OpenMax.h"
#include "DVDVideoCodec.h"
#include "threads/Thread.h"

#include <OMX_Core.h>
#include <OMX_Component.h>
#include <OMX_Index.h>
#include <OMX_Image.h>
#include <OMX_IVCommon.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>

// an omx egl video frame
typedef struct OpenMaxVideoBuffer {
  OMX_BUFFERHEADERTYPE *omx_buffer;
  int width;
  int height;
  int index;

  // used for egl based rendering if active
  EGLImageKHR egl_image;
  GLuint texture_id;
  void *graphic_handle;
  void *native_buffer;
} OpenMaxVideoBuffer;

class CDVDStreamInfo;
class CDVDVideoCodec;

class COpenMaxVideo : public COpenMax, public CThread
{
public:
  COpenMaxVideo();
  virtual ~COpenMaxVideo();

  // Required overrides
  bool Open(CDVDStreamInfo &hints);
  void Close(void);
  int  Decode(BYTE *pData, int iSize, double dts, double pts);
  void Reset(void);
  bool GetPicture(DVDVideoPicture *pDvdVideoPicture);
  void SetDropState(bool bDrop);
  int ReadyCount();
  int GetError() { return m_error; };
protected:
  void QueryCodec(void);
  OMX_ERRORTYPE PrimeFillBuffers(void);
  OMX_ERRORTYPE AllocOMXInputBuffers(void);
  OMX_ERRORTYPE FreeOMXInputBuffers(bool wait);
  OMX_ERRORTYPE AllocOMXOutputBuffers(void);
  OMX_ERRORTYPE FreeOMXOutputBuffers(bool wait);
  static void CallbackAllocOMXEGLTextures(void*);
  OMX_ERRORTYPE AllocOMXOutputEGLTextures(void);
  static void CallbackFreeOMXEGLTextures(void*);
  OMX_ERRORTYPE FreeOMXOutputEGLTextures(bool wait);

  // TODO Those should move into the base class. After start actions can be executed by callbacks.
  OMX_ERRORTYPE StartDecoder(void);
  OMX_ERRORTYPE StopDecoder(void);

  // OpenMax decoder callback routines.
  virtual OMX_ERRORTYPE DecoderEventHandler(OMX_HANDLETYPE hComponent, OMX_PTR pAppData,
    OMX_EVENTTYPE eEvent, OMX_U32 nData1, OMX_U32 nData2, OMX_PTR pEventData);
  virtual OMX_ERRORTYPE DecoderEmptyBufferDone(
    OMX_HANDLETYPE hComponent, OMX_PTR pAppData, OMX_BUFFERHEADERTYPE* pBuffer);
  virtual OMX_ERRORTYPE DecoderFillBufferDone(
    OMX_HANDLETYPE hComponent, OMX_PTR pAppData, OMX_BUFFERHEADERTYPE* pBufferHeader);

  // EGL Resources
  EGLDisplay        m_egl_display;
  EGLContext        m_egl_context;

  // Video format
  bool              m_drop_state;
  int               m_decoded_width;
  int               m_decoded_height;
  int               m_scaled_width;
  int               m_scaled_height;

  pthread_mutex_t   m_omx_packet_mutex;
  std::queue<omx_demux_packet *> m_demux_queue;

  // OpenMax input buffers (demuxer packets)
  pthread_mutex_t   m_omx_input_mutex;
  std::queue<OMX_BUFFERHEADERTYPE*> m_omx_input_avaliable;
  std::vector<OMX_BUFFERHEADERTYPE*> m_omx_input_buffers;
  bool              m_omx_input_eos;
  int               m_omx_input_port;
  CEvent            m_input_consumed_event;

  // OpenMax output buffers (video frames)
  pthread_mutex_t   m_omx_output_mutex;
  std::queue<OpenMaxVideoBuffer*> m_omx_output_busy;
  std::queue<OpenMaxVideoBuffer*> m_omx_output_ready;
  std::vector<OpenMaxVideoBuffer*> m_omx_output_buffers;

  bool              m_omx_output_eos;
  int               m_omx_output_port;

  bool              m_portChanging;

  volatile bool     m_videoplayback_done;

  OMX_PARAM_PORTDEFINITIONTYPE m_outputPortFormat;
  OMX_CONFIG_RECTTYPE          m_cropRect;

  int               m_packetCount;
  int               m_readyCount;
  bool              m_firstPicture;
  bool              m_firstDemuxPacket;

  bool SendDecoderConfig(uint8_t *extradata, int extrasize);

  virtual void      Process(void);

  int               m_error;

  OMX_U32           m_xFramerate;

  bool              m_abort;
  int               m_BufferSize;
  sem_t             *m_omx_empty_done;
  sem_t             *m_omx_fill_done;
  sem_t             *m_omx_flush;
  sem_t             *m_omx_cmd_complete;

  pthread_mutex_t   m_omx_lock_decoder;
};
