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

#if (defined HAVE_CONFIG_H) && (!defined WIN32)
  #include "config.h"
#elif defined(_WIN32)
#include "system.h"
#endif

#include "utils/GLUtils.h"

#include "OpenMaxVideo.h"
#include "Application.h"
#include "DVDClock.h"
#include "windowing/WindowingFactory.h"
#include "utils/log.h"

#define CLASSNAME "COpenMaxVideo"

/*
#if !defined(TARGET_ANDROID)
#define OMX_H264BASE_DECODER    "OMX.google.h264.decoder"
#define OMX_H264MAIN_DECODER    "OMX.google.h264.decoder"
#define OMX_H264HIGH_DECODER    "OMX.google.h264.decoder"
#define OMX_MPEG4_DECODER       "OMX.google.mpeg4.decoder"
#define OMX_MPEG4EXT_DECODER    "OMX.google.mpeg4.decoder"
#define OMX_MPEG2V_DECODER      "OMX.google.mpeg2.decode"
#define OMX_VC1_DECODER         "OMX.google.vc1.decode"

#else

// TODO: These are Nvidia Tegra2 dependent, need to dynamiclly find the
// right codec matched to video format.
#define OMX_H264BASE_DECODER    "OMX.Nvidia.h264.decode"
#define OMX_H264MAIN_DECODER    "OMX.Nvidia.h264.decode"
#define OMX_H264HIGH_DECODER    "OMX.Nvidia.h264ext.decode"
#define OMX_MPEG4_DECODER       "OMX.Nvidia.mp4.decode"
#define OMX_MPEG4EXT_DECODER    "OMX.Nvidia.mp4ext.decode"
#define OMX_MPEG2V_DECODER      "OMX.Nvidia.mpeg2v.decode"
#define OMX_VC1_DECODER         "OMX.Nvidia.vc1.decode"

#endif
*/

//#define USE_OMX_THREAD 1

// EGL extension functions
static PFNEGLCREATEIMAGEKHRPROC eglCreateImageKHR;
static PFNEGLDESTROYIMAGEKHRPROC eglDestroyImageKHR;
static PFNGLEGLIMAGETARGETTEXTURE2DOESPROC glEGLImageTargetTexture2DOES;
#define GETEXTENSION(type, ext) \
do \
{ \
    ext = (type) eglGetProcAddress(#ext); \
    if (!ext) \
    { \
        CLog::Log(LOGERROR, "%s::%s - ERROR getting proc addr of " #ext "\n", CLASSNAME, __func__); \
    } \
} while (0);

#if defined(TARGET_ANDROID)
#define OMX_VERSION_MAJOR 1
#define OMX_VERSION_MINOR 0
#define OMX_VERSION_REV   0
#define OMX_VERSION_REVISION   0
#define OMX_VERSION_STEP  0
#endif

#define OMX_INIT_STRUCTURE(a) \
  memset(&(a), 0, sizeof(a)); \
  (a).nSize = sizeof(a); \
  (a).nVersion.s.nVersionMajor = OMX_VERSION_MAJOR; \
  (a).nVersion.s.nVersionMinor = OMX_VERSION_MINOR; \
  (a).nVersion.s.nRevision = OMX_VERSION_REVISION; \
  (a).nVersion.s.nStep = OMX_VERSION_STEP


COpenMaxVideo::COpenMaxVideo() :
  CThread("COpenMaxVideo")
{
  m_portChanging      = false;
  m_firstPicture      = true;
  m_firstDemuxPacket  = true;
  m_error             = 0;
  m_readyCount        = 0;
  m_packetCount       = 0;
  m_abort             = false;
  m_BufferSize        = 0;

  OMX_INIT_STRUCTURE(m_outputPortFormat);
  OMX_INIT_STRUCTURE(m_cropRect);

  pthread_mutex_init(&m_omx_input_mutex, NULL);
  pthread_mutex_init(&m_omx_output_mutex, NULL);
  pthread_mutex_init(&m_omx_packet_mutex, NULL);
  pthread_mutex_init(&m_omx_lock_decoder, NULL);

  m_omx_decoder_state_change = (sem_t*)malloc(sizeof(sem_t));
  sem_init(m_omx_decoder_state_change, 0, 0);

  m_omx_fill_done = (sem_t*)malloc(sizeof(sem_t));
  sem_init(m_omx_fill_done, 0, 0);

  m_omx_empty_done = (sem_t*)malloc(sizeof(sem_t));
  sem_init(m_omx_empty_done, 0, 0);

  m_omx_flush = (sem_t*)malloc(sizeof(sem_t));
  sem_init(m_omx_flush, 0, 0);

  m_omx_cmd_complete = (sem_t*)malloc(sizeof(sem_t));
  sem_init(m_omx_cmd_complete, 0, 0);
}

COpenMaxVideo::~COpenMaxVideo()
{
  #if defined(OMX_DEBUG_VERBOSE)
  CLog::Log(LOGDEBUG, "%s::%s\n", CLASSNAME, __func__);
  #endif
  if (m_is_open)
    Close();

  pthread_mutex_destroy(&m_omx_packet_mutex);
  pthread_mutex_destroy(&m_omx_input_mutex);
  pthread_mutex_destroy(&m_omx_output_mutex);
  pthread_mutex_destroy(&m_omx_lock_decoder);

  sem_destroy(m_omx_decoder_state_change);
  sem_destroy(m_omx_fill_done);
  sem_destroy(m_omx_empty_done);
  sem_destroy(m_omx_flush);
  sem_destroy(m_omx_cmd_complete);

  free(m_omx_decoder_state_change);
  free(m_omx_fill_done);
  free(m_omx_empty_done);
  free(m_omx_flush);
  free(m_omx_cmd_complete);
}

bool COpenMaxVideo::Open(CDVDStreamInfo &hints)
{
  #if defined(OMX_DEBUG_VERBOSE)
  CLog::Log(LOGDEBUG, "%s::%s\n", CLASSNAME, __func__);
  #endif

  OMX_ERRORTYPE omx_err = OMX_ErrorNone;
  std::string role_name;

  m_decoded_width   = hints.width;
  m_decoded_height  = hints.height;
  m_scaled_width    = hints.width;
  m_scaled_height   = hints.height;

  if (m_decoded_width > 1280 || m_decoded_height > 720)
  {
    m_scaled_width  /= 2;
    m_scaled_height /= 2;
  }


  /*
  switch (hints.codec)
  {
    case CODEC_ID_H264:
    {
      switch(hints.profile)
      {
        case FF_PROFILE_H264_BASELINE:
          // (role name) video_decoder.avc
          // H.264 Baseline profile
          decoder_name = OMX_H264BASE_DECODER;
        break;
        case FF_PROFILE_H264_MAIN:
          // (role name) video_decoder.avc
          // H.264 Main profile
          decoder_name = OMX_H264MAIN_DECODER;
        break;
        case FF_PROFILE_H264_HIGH:
          // (role name) video_decoder.avc
          // H.264 Main profile
          decoder_name = OMX_H264HIGH_DECODER;
        break;
        default:
          return false;
        break;
      }
    }
    break;
    case CODEC_ID_MPEG4:
      // (role name) video_decoder.mpeg4
      // MPEG-4, DivX 4/5 and Xvid compatible
      decoder_name = OMX_MPEG4_DECODER;
    break;
    case CODEC_ID_MPEG2VIDEO:
      // (role name) video_decoder.mpeg2
      // MPEG-2
      decoder_name = OMX_MPEG2V_DECODER;
    break;
    case CODEC_ID_VC1:
      // (role name) video_decoder.vc1
      // VC-1, WMV9
      decoder_name = OMX_VC1_DECODER;
    break;
    default:
      return false;
    break;
  }
  */

  role_name = GetOmxRole(hints.codec);

  // initialize OpenMAX.
  if (!Initialize(role_name))
  {
    return false;
  }

  // Set component role
  OMX_PARAM_COMPONENTROLETYPE role;
  OMX_INIT_STRUCTURE(role);
  //strcpy((char*)role.cRole, role_name.c_str());
  //omx_err = OMX_SetParameter(m_omx_decoder, OMX_IndexParamStandardComponentRole, &role);
  omx_err = OMX_GetParameter(m_omx_decoder, OMX_IndexParamStandardComponentRole, &role);
  if (omx_err == OMX_ErrorNone)
      CLog::Log(LOGDEBUG,"component standard role to %s", role.cRole);

  // TODO: Find component from role name.
  // Get the port information. This will obtain information about the
  // number of ports and index of the first port.
  OMX_PORT_PARAM_TYPE port_param;
  OMX_INIT_STRUCTURE(port_param);
  omx_err = OMX_GetParameter(m_omx_decoder, OMX_IndexParamVideoInit, &port_param);
  if (omx_err != OMX_ErrorNone)
  {
    CLog::Log(LOGERROR,
      "%s::%s - OMX_GetParameter omx_err(0x%x)", CLASSNAME, __func__, omx_err);
    Deinitialize();
    return false;
  }
  m_omx_input_port = port_param.nStartPortNumber;
  m_omx_output_port = m_omx_input_port + 1;
  #if defined(OMX_DEBUG_VERBOSE)
  CLog::Log(LOGDEBUG,
    "%s::%s - decoder_component(%p), input_port(0x%x), output_port(0x%x)\n",
    CLASSNAME, __func__, m_omx_decoder, m_omx_input_port, m_omx_output_port);
  #endif

  // TODO: Set role for the component because components could have multiple roles.
  //QueryCodec();
  

  /*
  if (hints.fpsscale > 0 && hints.fpsrate > 0)
    m_xFramerate = (long long)(1<<16)*hints.fpsrate / hints.fpsscale;
  else
    m_xFramerate = 25 * (1<<16);
  */

  m_xFramerate = 0;

  OMX_PARAM_PORTDEFINITIONTYPE port_format;
  OMX_VIDEO_PORTDEFINITIONTYPE *video_def = &port_format.format.video;
  OMX_INIT_STRUCTURE(port_format);

  port_format.nPortIndex = m_omx_input_port;

  OMX_GetParameter(m_omx_decoder, OMX_IndexParamPortDefinition, &port_format);

  video_def->nFrameWidth        = m_decoded_width;
  video_def->nFrameHeight       = m_decoded_height;
  video_def->nStride            = m_decoded_width;
  video_def->nSliceHeight       = m_decoded_height;
  video_def->eColorFormat       = OMX_COLOR_FormatUnused;
  video_def->xFramerate         = m_xFramerate;
  video_def->eCompressionFormat = GetOmxCodingType(hints.codec);

  OMX_SetParameter(m_omx_decoder, OMX_IndexParamPortDefinition, &port_format);

  OMX_VIDEO_PARAM_PORTFORMATTYPE formatType;
  OMX_INIT_STRUCTURE(formatType);

  formatType.nPortIndex = m_omx_input_port;
  formatType.eCompressionFormat = GetOmxCodingType(hints.codec);
  formatType.xFramerate = m_xFramerate;

  omx_err = OMX_SetParameter(m_omx_decoder, OMX_IndexParamVideoPortFormat, &formatType);
  if (omx_err != OMX_ErrorNone)
  {
    CLog::Log(LOGERROR, "%s::%s - error setting coding type\n", CLASSNAME, __func__);
    Deinitialize();
    return false;
  }

  // Component will be in OMX_StateLoaded now so we can alloc omx input/output buffers.
  // we can only alloc them in OMX_StateLoaded state or if the component port is disabled
  // Alloc buffers for the omx input port.
  omx_err = AllocOMXInputBuffers();
  if (omx_err != OMX_ErrorNone)
  {
    Deinitialize();
    return false;
  }

  // Alloc buffers for the omx output port.
  m_egl_display = g_Windowing.GetEGLDisplay();
  m_egl_context = g_Windowing.GetEGLContext();
  omx_err = AllocOMXOutputBuffers();
  if (omx_err != OMX_ErrorNone)
  {
    FreeOMXInputBuffers(false);
    Deinitialize();
    return false;
  }

  //OMX_SendCommand(m_omx_decoder, OMX_CommandPortEnable, m_omx_input_port, NULL);
  //OMX_SendCommand(m_omx_decoder, OMX_CommandPortEnable, m_omx_output_port, NULL);

  Sleep(100);

  // crank it up.
  if (StartDecoder() != OMX_ErrorNone)
  {
    FreeOMXOutputBuffers(false);
    FreeOMXInputBuffers(false);
    Deinitialize();
    return false;
  }

  SendDecoderConfig((uint8_t *)hints.extradata, hints.extrasize);

  m_is_open = true;
  m_drop_state = false;
  m_videoplayback_done = false;

#if defined(USE_OMX_THREAD)
  Create();
#endif

  return true;
}

void COpenMaxVideo::Close()
{

  m_abort = true;

  sem_post(m_omx_flush);
  sem_post(m_omx_decoder_state_change);
  sem_post(m_omx_cmd_complete);
  sem_post(m_omx_empty_done);
  sem_post(m_omx_fill_done);
      
#if defined(USE_OMX_THREAD)
  StopThread();
#endif

  #if defined(OMX_DEBUG_VERBOSE)
  CLog::Log(LOGDEBUG, "%s::%s\n", CLASSNAME, __func__);
  #endif
  if (m_omx_decoder)
  {
    if (m_omx_decoder_state != OMX_StateLoaded)
      StopDecoder();
    Deinitialize();
  }
  m_is_open = false;
}

void COpenMaxVideo::SetDropState(bool bDrop)
{
  m_drop_state = bDrop;

  /*
  if (m_drop_state)
  {
    OMX_ERRORTYPE omx_err;

    // blow all but the last ready video frame
    pthread_mutex_lock(&m_omx_output_mutex);
    while (m_omx_output_ready.size() > 1)
    {
      OpenMaxVideoBuffer *buffer = m_omx_output_ready.front();
      m_omx_output_ready.pop();
      m_readyCount = m_omx_output_ready.size();

      // return the omx buffer back to OpenMax to fill.
      omx_err = OMX_FillThisBuffer(m_omx_decoder, buffer->omx_buffer);
      if (omx_err != OMX_ErrorNone)
        CLog::Log(LOGERROR, "%s::%s - OMX_FillThisBuffer, omx_err(0x%x)\n",
          CLASSNAME, __func__, omx_err);
      //sem_wait(m_omx_sem_fill_buffer_done);
    }
    pthread_mutex_unlock(&m_omx_output_mutex);

    #if defined(OMX_DEBUG_VERBOSE)
    CLog::Log(LOGDEBUG, "%s::%s - m_drop_state(%d)\n",
      CLASSNAME, __func__, m_drop_state);
    #endif
  }
  */

}

int COpenMaxVideo::Decode(BYTE* pData, int iSize, double dts, double pts)
{
  if (pData)
  {
    int demuxer_bytes = iSize;
    uint8_t *demuxer_content = pData;

    // we need to queue then de-queue the demux packet, seems silly but
    // omx might not have a omx input buffer avaliable when we are called
    // and we must store the demuxer packet and try again later.
    omx_demux_packet *demux_packet = new omx_demux_packet;
    demux_packet->dts = dts;
    demux_packet->pts = pts;

    demux_packet->size = demuxer_bytes;
    demux_packet->buff = new OMX_U8[demuxer_bytes];
    memcpy(demux_packet->buff, demuxer_content, demuxer_bytes);

    pthread_mutex_lock(&m_omx_packet_mutex);
    m_demux_queue.push(demux_packet);
    m_packetCount = m_demux_queue.size();
    pthread_mutex_unlock(&m_omx_packet_mutex);

  }

#if !defined(USE_OMX_THREAD)
  if (!m_demux_queue.empty())
  {
    omx_demux_packet *demux_packet = m_demux_queue.front();

    int demuxer_bytes = demux_packet->size;
    uint8_t *demuxer_content = demux_packet->buff;
    
    bool consumePacket = ( (m_omx_input_avaliable.size() * m_BufferSize) > demux_packet->size ) ? true : false;
    
    while(consumePacket)
    {
      OMX_BUFFERHEADERTYPE *omx_buffer = m_omx_input_avaliable.front();
      m_omx_input_avaliable.pop();

      omx_buffer->nFlags  = m_omx_input_eos ? OMX_BUFFERFLAG_EOS : 0;
      omx_buffer->nOffset = 0;

      omx_buffer->nFilledLen = (demuxer_bytes > (int)omx_buffer->nAllocLen) ? (int)omx_buffer->nAllocLen : demuxer_bytes;
      memcpy(omx_buffer->pBuffer, demuxer_content, omx_buffer->nFilledLen);

      demuxer_bytes   -= omx_buffer->nFilledLen;
      demuxer_content += omx_buffer->nFilledLen;

      // if we do not hand over a valid timstamp the omx component generates one : seen on nvidia tg3
      double timestamp = (pts == DVD_NOPTS_VALUE) ? dts : pts;
      omx_buffer->nTimeStamp = (timestamp == DVD_NOPTS_VALUE) ? 0 : timestamp * 1000.0; // in microseconds;

      if (m_drop_state)
        omx_buffer->nFlags |= OMX_BUFFERFLAG_DECODEONLY;

      if (m_firstDemuxPacket)
      {
        omx_buffer->nFlags |= OMX_BUFFERFLAG_STARTTIME;
        m_firstDemuxPacket = false;
      }

      if (demuxer_bytes == 0)
        omx_buffer->nFlags |= OMX_BUFFERFLAG_ENDOFFRAME;

      #if defined(OMX_DEBUG_EMPTYBUFFERDONE)
      CLog::Log(LOGDEBUG,
        "%s::%s - feeding decoder, omx_buffer->pBuffer(%p), demuxer_bytes(%d) timestamp(%f)\n",
        CLASSNAME, __func__, omx_buffer->pBuffer, (int)omx_buffer->nFilledLen, timestamp / DVD_TIME_BASE);
      #endif
      // Give this omx_buffer to OpenMax to be decoded.
      OMX_ERRORTYPE omx_err = OMX_EmptyThisBuffer(m_omx_decoder, omx_buffer);
      if (omx_err != OMX_ErrorNone)
      {
        CLog::Log(LOGDEBUG,
          "%s::%s - OMX_EmptyThisBuffer() failed with result(0x%x)\n",
          CLASSNAME, __func__, omx_err);
        m_error = VC_ERROR;
        break;
      }

      sem_wait(m_omx_empty_done);

      if (demuxer_bytes == 0)
      {
        m_demux_queue.pop();
        delete demux_packet->buff;
        delete demux_packet;
        break;
      }
    }
  }

  while(true)
  {
    pthread_mutex_lock(&m_omx_output_mutex);
    if (m_omx_output_busy.size() > 1)
    {
      // fetch a output buffer and pop it off the busy list
      OpenMaxVideoBuffer *buffer = m_omx_output_busy.front();
      m_omx_output_busy.pop();
      pthread_mutex_unlock(&m_omx_output_mutex);

      // return the omx buffer back to OpenMax to fill.
      OMX_ERRORTYPE omx_err = OMX_FillThisBuffer(m_omx_decoder, buffer->omx_buffer);
      if (omx_err != OMX_ErrorNone)
        CLog::Log(LOGERROR, "%s::%s - OMX_FillThisBuffer, omx_err(0x%x)\n",
       CLASSNAME, __func__, omx_err);
      sem_wait(m_omx_fill_done);
    }
    else
    {
      pthread_mutex_unlock(&m_omx_output_mutex);
      break;
    }
  }
#endif

  return VC_BUFFER;
}

void COpenMaxVideo::Process(void)
{
  omx_demux_packet *demux_packet = NULL;

  while(!m_bStop)
  {
    if (m_abort || m_error == VC_ERROR)
      break;

    // we can look at m_omx_input_avaliable.empty without needing to lock/unlock
    // try to send any/all demux packets to omx decoder.

    pthread_mutex_lock(&m_omx_lock_decoder);

    // prepare next demuxer packet
    pthread_mutex_lock(&m_omx_packet_mutex);
    if (!m_demux_queue.empty() && !demux_packet)
    {
      demux_packet = m_demux_queue.front();
      m_demux_queue.pop();
      m_packetCount = m_demux_queue.size();
    }
    pthread_mutex_unlock(&m_omx_packet_mutex);

    if (demux_packet)
    {
      int demuxer_bytes = demux_packet->size;
      uint8_t *demuxer_content = demux_packet->buff;
    
      bool consumePacket = ( (m_omx_input_avaliable.size() * m_BufferSize) > demux_packet->size ) ? true : false;
    
      while(consumePacket)
      {
        pthread_mutex_lock(&m_omx_input_mutex);
        OMX_BUFFERHEADERTYPE *omx_buffer = m_omx_input_avaliable.front();
        m_omx_input_avaliable.pop();
        pthread_mutex_unlock(&m_omx_input_mutex);

        omx_buffer->nFlags  = m_omx_input_eos ? OMX_BUFFERFLAG_EOS : 0;
        omx_buffer->nOffset = 0;

        omx_buffer->nFilledLen = (demuxer_bytes > (int)omx_buffer->nAllocLen) ? (int)omx_buffer->nAllocLen : demuxer_bytes;
        memcpy(omx_buffer->pBuffer, demuxer_content, omx_buffer->nFilledLen);

        demuxer_bytes   -= omx_buffer->nFilledLen;
        demuxer_content += omx_buffer->nFilledLen;

        // if we do not hand over a valid timstamp the omx component generates one : seen on nvidia tg3
        double timestamp = (demux_packet->pts == DVD_NOPTS_VALUE) ? demux_packet->dts : demux_packet->pts;
        omx_buffer->nTimeStamp = (timestamp == DVD_NOPTS_VALUE) ? 0 : timestamp * 1000.0; // in microseconds;

        //if (m_drop_state)
        //  omx_buffer->nFlags |= OMX_BUFFERFLAG_DECODEONLY;

        if (m_firstDemuxPacket)
        {
          omx_buffer->nFlags |= OMX_BUFFERFLAG_STARTTIME;
          m_firstDemuxPacket = false;
        }

        if (demuxer_bytes == 0)
          omx_buffer->nFlags |= OMX_BUFFERFLAG_ENDOFFRAME;

        //#if defined(OMX_DEBUG_EMPTYBUFFERDONE)
        CLog::Log(LOGDEBUG,
          "%s::%s - feeding decoder, omx_buffer->pBuffer(%p), demuxer_bytes(%d) timestamp(%f)\n",
          CLASSNAME, __func__, omx_buffer->pBuffer, (int)omx_buffer->nFilledLen, timestamp / DVD_TIME_BASE);
        //#endif
        // Give this omx_buffer to OpenMax to be decoded.
        OMX_ERRORTYPE omx_err = OMX_EmptyThisBuffer(m_omx_decoder, omx_buffer);
        if (omx_err != OMX_ErrorNone)
        {
          CLog::Log(LOGDEBUG,
            "%s::%s - OMX_EmptyThisBuffer() failed with result(0x%x)\n",
            CLASSNAME, __func__, omx_err);
          m_error = VC_ERROR;
          break;
        }

        //sem_wait(m_omx_empty_done);

        if (demuxer_bytes == 0)
        {
          delete demux_packet->buff;
          delete demux_packet;
          demux_packet = NULL;
          break;
        }
      }

      if (m_error == VC_ERROR)
        break;
    }

    // try to fetch a output picture
    pthread_mutex_lock(&m_omx_output_mutex);
    if (m_omx_output_busy.size() > 1)
    {
      // fetch a output buffer and pop it off the busy list
      OpenMaxVideoBuffer *buffer = m_omx_output_busy.front();
      m_omx_output_busy.pop();
      pthread_mutex_unlock(&m_omx_output_mutex);

      // return the omx buffer back to OpenMax to fill.
      OMX_ERRORTYPE omx_err = OMX_FillThisBuffer(m_omx_decoder, buffer->omx_buffer);
      if (omx_err != OMX_ErrorNone)
      {
        CLog::Log(LOGERROR, "%s::%s - OMX_FillThisBuffer, omx_err(0x%x)\n", CLASSNAME, __func__, omx_err);
        m_error = VC_ERROR;
          break;
      }

      //sem_wait(m_omx_fill_done);
    }
    else
      pthread_mutex_unlock(&m_omx_output_mutex);

    pthread_mutex_unlock(&m_omx_lock_decoder);

    Sleep(1);
  }

  if (demux_packet)
  {
    delete demux_packet->buff;
    delete demux_packet;
  }
}

void COpenMaxVideo::Reset(void)
{
  if (!m_is_open)
    return;
  
  //#if defined(OMX_DEBUG_VERBOSE)
  CLog::Log(LOGDEBUG, "%s::%s Reset decoder\n", CLASSNAME, __func__);
  //#endif

  OMX_ERRORTYPE omx_err = OMX_ErrorNone;

  // flush decoder input and output ports

  omx_err = OMX_SendCommand(m_omx_decoder, OMX_CommandFlush, m_omx_input_port, 0);
  if (omx_err != OMX_ErrorNone)
    CLog::Log(LOGERROR, "%s::%s - OMX_CommandFlush failed with omx_err(0x%x)\n",
      CLASSNAME, __func__, omx_err);
  else
    sem_wait(m_omx_flush);

  omx_err = OMX_SendCommand(m_omx_decoder, OMX_CommandFlush, m_omx_output_port, 0);
  if (omx_err != OMX_ErrorNone)
    CLog::Log(LOGERROR, "%s::%s - OMX_CommandFlush failed with omx_err(0x%x)\n",
      CLASSNAME, __func__, omx_err);
  else
    sem_wait(m_omx_flush);

  pthread_mutex_lock(&m_omx_lock_decoder);

  // flush ready buffers
  while (!m_omx_output_ready.empty())
  {
    OpenMaxVideoBuffer *omx_buffer = m_omx_output_ready.front();
    m_omx_output_ready.pop();
    m_omx_output_busy.push(omx_buffer);
  }

  // flush demuxer packets
  while (!m_demux_queue.empty())
  {
    omx_demux_packet *demux_packet = m_demux_queue.front();
    m_demux_queue.pop();
    delete demux_packet->buff;
    delete demux_packet;
  }

  m_firstDemuxPacket = true;

  m_readyCount = m_omx_output_ready.size();

  pthread_mutex_unlock(&m_omx_lock_decoder);
}

int COpenMaxVideo::ReadyCount()
{
  return m_readyCount;
}

bool COpenMaxVideo::GetPicture(DVDVideoPicture* pDvdVideoPicture)
{
  OpenMaxVideoBuffer *buffer = NULL;
  pDvdVideoPicture->dts               = DVD_NOPTS_VALUE;
  pDvdVideoPicture->pts               = DVD_NOPTS_VALUE;

  // fetch a output buffer and pop it off the ready list
  pthread_mutex_lock(&m_omx_output_mutex);
  if (!m_omx_output_ready.empty())
  {
    buffer = m_omx_output_ready.front();
    m_omx_output_ready.pop();
    m_readyCount = m_omx_output_ready.size();
    m_omx_output_busy.push(buffer);

    // nTimeStamp is in microseconds
    pDvdVideoPicture->pts = (buffer->omx_buffer->nTimeStamp == 0) ? DVD_NOPTS_VALUE : (double)buffer->omx_buffer->nTimeStamp / 1000.0;
    pDvdVideoPicture->dts = pDvdVideoPicture->pts;
  }
  pthread_mutex_unlock(&m_omx_output_mutex);

  pDvdVideoPicture->color_range       = 0;
  pDvdVideoPicture->color_matrix      = 4;
  pDvdVideoPicture->iWidth            = m_decoded_width;
  pDvdVideoPicture->iHeight           = m_decoded_height;
  pDvdVideoPicture->iDisplayWidth     = m_decoded_width;
  pDvdVideoPicture->iDisplayHeight    = m_decoded_height;
#if defined(TARGET_ANDROID)
  pDvdVideoPicture->format            = RENDER_FMT_YUV420P;
#else
  pDvdVideoPicture->format            = RENDER_FMT_OMXEGL;
#endif

  if (buffer && buffer->omx_buffer->nFilledLen)
  {
    OMX_VIDEO_PORTDEFINITIONTYPE *video_def = &m_outputPortFormat.format.video;

    CLog::Log(LOGDEBUG, "nFrameWidth %d nFrameWidth %d nFilledLen %d nOffset %d ReadyCount %d Packet Count %d timestamp(%f)\n", 
        (int)video_def->nFrameWidth, (int)video_def->nFrameHeight, 
        (int)buffer->omx_buffer->nFilledLen, (int)buffer->omx_buffer->nOffset, m_readyCount, m_packetCount, (double)pDvdVideoPicture->pts / DVD_TIME_BASE);

    pDvdVideoPicture->data[0]       = buffer->omx_buffer->pBuffer + buffer->omx_buffer->nOffset;
    pDvdVideoPicture->iLineSize[0]  = video_def->nFrameWidth;
    pDvdVideoPicture->data[1]       = pDvdVideoPicture->data[0] + (video_def->nFrameWidth * video_def->nFrameHeight);
    pDvdVideoPicture->iLineSize[1]  = video_def->nFrameWidth / 2;
    pDvdVideoPicture->data[2]       = pDvdVideoPicture->data[1] + (pDvdVideoPicture->iLineSize[1] * video_def->nFrameHeight / 2);
    pDvdVideoPicture->iLineSize[2]  = video_def->nFrameWidth / 2;
    pDvdVideoPicture->iLineSize[3]  = 0;
#if !defined(TARGET_ANDROID)
    pDvdVideoPicture->openMax       = this;
    pDvdVideoPicture->openMaxBuffer = buffer;
#endif

    pDvdVideoPicture->iFlags  = DVP_FLAG_ALLOCATED;
    //pDvdVideoPicture->iFlags  |= m_drop_state ? DVP_FLAG_DROPPED : 0;
  }
  else if (buffer)
  {
    CLog::Log(LOGDEBUG, "droped\n");
    pDvdVideoPicture->iFlags = DVP_FLAG_DROPPED;
  }
  else
  {
    CLog::Log(LOGDEBUG, "droped\n");
    pDvdVideoPicture->iFlags = DVP_FLAG_DROPPED;
  }

  return true;
}

// DecoderEmptyBufferDone -- OpenMax input buffer has been emptied
OMX_ERRORTYPE COpenMaxVideo::DecoderEmptyBufferDone(
  OMX_HANDLETYPE hComponent,
  OMX_PTR pAppData,
  OMX_BUFFERHEADERTYPE* pBuffer)
{
  COpenMaxVideo *ctx = static_cast<COpenMaxVideo*>(pAppData);
/*
  #if defined(OMX_DEBUG_EMPTYBUFFERDONE)
  CLog::Log(LOGDEBUG, "%s::%s - buffer_size(%lu), timestamp(%f)\n",
    CLASSNAME, __func__, pBuffer->nFilledLen, (double)pBuffer->nTimeStamp / 1000.0);
  #endif
*/
  // queue free input buffer to avaliable list.
  pthread_mutex_lock(&ctx->m_omx_input_mutex);
  ctx->m_omx_input_avaliable.push(pBuffer);
  //ctx->m_input_consumed_event.Set();
  pthread_mutex_unlock(&ctx->m_omx_input_mutex);

  sem_post(ctx->m_omx_empty_done);

  return OMX_ErrorNone;
}

// DecoderFillBufferDone -- OpenMax output buffer has been filled
OMX_ERRORTYPE COpenMaxVideo::DecoderFillBufferDone(
  OMX_HANDLETYPE hComponent,
  OMX_PTR pAppData,
  OMX_BUFFERHEADERTYPE* pBuffer)
{
  COpenMaxVideo *ctx = static_cast<COpenMaxVideo*>(pAppData);
  OpenMaxVideoBuffer *buffer = (OpenMaxVideoBuffer*)pBuffer->pAppPrivate;

  #if defined(OMX_DEBUG_FILLBUFFERDONE)
  CLog::Log(LOGDEBUG, "%s::%s - buffer_size(%lu), offset(%lu), timestamp(%f)\n",
    CLASSNAME, __func__, pBuffer->nFilledLen, pBuffer->nOffset, (double)pBuffer->nTimeStamp / 1000.0);
  #endif

  if (!ctx->m_portChanging)
  {
    // queue output omx buffer to ready list.
    pthread_mutex_lock(&ctx->m_omx_output_mutex);
    if ((pBuffer->nFlags & OMX_BUFFERFLAG_EOS) || pBuffer->nFilledLen == 0)
    {
      pBuffer->nFlags &= ~OMX_BUFFERFLAG_EOS;
      ctx->m_omx_output_busy.push(buffer);
    }
    else
    {
      ctx->m_omx_output_ready.push(buffer);
    }
    m_readyCount = m_omx_output_ready.size();
    pthread_mutex_unlock(&ctx->m_omx_output_mutex);

    // get the port and picture information from the first decoded frame
    if (m_firstPicture)
    {
      m_outputPortFormat.nPortIndex = m_omx_output_port;
      OMX_GetParameter(m_omx_decoder, OMX_IndexParamPortDefinition, &m_outputPortFormat);

      m_cropRect.nPortIndex = m_omx_output_port;
      OMX_ERRORTYPE omx_err = OMX_GetConfig(m_omx_decoder, OMX_IndexConfigCommonOutputCrop, &m_cropRect);

      if (omx_err != OMX_ErrorNone)
        OMX_INIT_STRUCTURE(m_cropRect);

      m_firstPicture = false;
    }
  }
  else
  {
    pthread_mutex_lock(&ctx->m_omx_output_mutex);
    ctx->m_omx_output_busy.push(buffer);
    pthread_mutex_unlock(&ctx->m_omx_output_mutex);
  }

  sem_post(ctx->m_omx_fill_done);

  return OMX_ErrorNone;
}

void COpenMaxVideo::QueryCodec(void)
{
  OMX_ERRORTYPE omx_err = OMX_ErrorNone;
  OMX_VIDEO_PARAM_PROFILELEVELTYPE port_param;
  OMX_INIT_STRUCTURE(port_param);

  port_param.nPortIndex = m_omx_input_port;

  for (port_param.nProfileIndex = 0;; port_param.nProfileIndex++)
  {
    omx_err = OMX_GetParameter(m_omx_decoder,
      OMX_IndexParamVideoProfileLevelQuerySupported, &port_param);
    if (omx_err != OMX_ErrorNone)
      break;

    omx_codec_capability omx_capability;
    omx_capability.level = port_param.eLevel;
    omx_capability.profile = port_param.eProfile;
    m_omx_decoder_capabilities.push_back(omx_capability);
  }
}

OMX_ERRORTYPE COpenMaxVideo::PrimeFillBuffers(void)
{
  OMX_ERRORTYPE omx_err = OMX_ErrorNone;
  OpenMaxVideoBuffer *buffer;

  #if defined(OMX_DEBUG_VERBOSE)
  CLog::Log(LOGDEBUG, "%s::%s\n", CLASSNAME, __func__);
  #endif
  // tell OpenMax to start filling output buffers
  for (size_t i = 0; i < m_omx_output_buffers.size(); i++)
  {
    buffer = m_omx_output_buffers[i];
    // Need to clear the EOS flag.
    buffer->omx_buffer->nFlags &= ~OMX_BUFFERFLAG_EOS;
    buffer->omx_buffer->nOutputPortIndex  = m_omx_output_port;

    omx_err = OMX_FillThisBuffer(m_omx_decoder, buffer->omx_buffer);
    if (omx_err != OMX_ErrorNone)
    {
      CLog::Log(LOGERROR, "%s::%s - OMX_FillThisBuffer failed with omx_err(0x%x)\n",
        CLASSNAME, __func__, omx_err);
      break;
    }

    //sem_wait(m_omx_fill_done);
  }

  return omx_err;
}

OMX_ERRORTYPE COpenMaxVideo::AllocOMXInputBuffers(void)
{
  OMX_ERRORTYPE omx_err = OMX_ErrorNone;

  // Obtain the information about the decoder input port.
  OMX_PARAM_PORTDEFINITIONTYPE port_format;
  OMX_INIT_STRUCTURE(port_format);
  port_format.nPortIndex = m_omx_input_port;

  OMX_GetParameter(m_omx_decoder, OMX_IndexParamPortDefinition, &port_format);

  port_format.nBufferCountActual  = 20;

  OMX_SetParameter(m_omx_decoder, OMX_IndexParamPortDefinition, &port_format);
  OMX_GetParameter(m_omx_decoder, OMX_IndexParamPortDefinition, &port_format);

  m_BufferSize = port_format.nBufferSize;

  //#if defined(OMX_DEBUG_VERBOSE)
  CLog::Log(LOGDEBUG,
    "%s::%s - iport(%d), nBufferCountMin(%lu), nBufferCountActual(%lu), nBufferSize(%lu)\n",
    CLASSNAME, __func__, m_omx_input_port, port_format.nBufferCountMin, port_format.nBufferCountActual, port_format.nBufferSize);
  //#endif

  for (size_t i = 0; i < port_format.nBufferCountActual; i++)
  {
    OMX_BUFFERHEADERTYPE *buffer = NULL;
    // use an external buffer that's sized according to actual demux
    // packet size, start at internal's buffer size, will get deleted when
    // we start pulling demuxer packets and using demux packet sized buffers.
    //OMX_U8* data = new OMX_U8[port_format.nBufferSize];
    //omx_err = OMX_UseBuffer(m_omx_decoder, &buffer, m_omx_input_port, NULL, port_format.nBufferSize, data);
    //omx_err = OMX_UseBuffer(m_omx_decoder, &buffer, m_omx_input_port, NULL, port_format.nBufferSize, NULL);
    omx_err = OMX_AllocateBuffer(m_omx_decoder, &buffer, m_omx_input_port, NULL, port_format.nBufferSize);
    if (omx_err != OMX_ErrorNone)
    {
      CLog::Log(LOGERROR, "%s::%s - OMX_UseBuffer failed with omx_err(0x%x)\n",
        CLASSNAME, __func__, omx_err);

      //delete [] data;
      return(omx_err);
    }
    buffer->nInputPortIndex = m_omx_input_port;
    buffer->pAppPrivate     = buffer;
    m_omx_input_buffers.push_back(buffer);
    // don't have to lock/unlock here, we are not decoding
    m_omx_input_avaliable.push(buffer);
  }
  m_omx_input_eos = false;

  return(omx_err);
}

OMX_ERRORTYPE COpenMaxVideo::FreeOMXInputBuffers(bool wait)
{
  OMX_ERRORTYPE omx_err = OMX_ErrorNone;

  omx_err = OMX_SendCommand(m_omx_decoder, OMX_CommandFlush, m_omx_input_port, 0);
  if (omx_err != OMX_ErrorNone)
    CLog::Log(LOGERROR, "%s::%s - OMX_CommandFlush failed with omx_err(0x%x)\n",
      CLASSNAME, __func__, omx_err);
  else if (wait)
    sem_wait(m_omx_flush);

  // free omx input port buffers.
  for (size_t i = 0; i < m_omx_input_buffers.size(); i++)
  {
    // using external buffers (OMX_UseBuffer), free our external buffers
    //  before calling OMX_FreeBuffer which frees the omx buffer.
    //delete [] m_omx_input_buffers[i]->pBuffer;
    //m_omx_input_buffers[i]->pBuffer = NULL;
    omx_err = OMX_FreeBuffer(m_omx_decoder, m_omx_input_port, m_omx_input_buffers[i]);
  }
  m_omx_input_buffers.clear();

  // empty input buffer queue. not decoding so don't need lock/unlock.
  while (!m_omx_input_avaliable.empty())
    m_omx_input_avaliable.pop();
  while (!m_demux_queue.empty())
  {
    omx_demux_packet *demux_packet = m_demux_queue.front();
    m_demux_queue.pop();
    delete demux_packet->buff;
    delete demux_packet;
  }

  return(omx_err);
}

void COpenMaxVideo::CallbackAllocOMXEGLTextures(void *userdata)
{
  COpenMaxVideo *omx = static_cast<COpenMaxVideo*>(userdata);
  omx->AllocOMXOutputEGLTextures();
}

void COpenMaxVideo::CallbackFreeOMXEGLTextures(void *userdata)
{
  COpenMaxVideo *omx = static_cast<COpenMaxVideo*>(userdata);
  omx->FreeOMXOutputEGLTextures(true);
}

OMX_ERRORTYPE COpenMaxVideo::AllocOMXOutputBuffers(void)
{
  OMX_ERRORTYPE omx_err;

  if ( g_application.IsCurrentThread() )
  {
    omx_err = AllocOMXOutputEGLTextures();
  }
  else
  {
    ThreadMessageCallback callbackData;
    callbackData.callback = &CallbackAllocOMXEGLTextures;
    callbackData.userptr = (void *)this;

    ThreadMessage tMsg;
    tMsg.dwMessage = TMSG_CALLBACK;
    tMsg.lpVoid = (void*)&callbackData;

    g_application.getApplicationMessenger().SendMessage(tMsg, true);

    omx_err = OMX_ErrorNone;
  }

  return omx_err;
}

OMX_ERRORTYPE COpenMaxVideo::FreeOMXOutputBuffers(bool wait)
{
  OMX_ERRORTYPE omx_err = FreeOMXOutputEGLTextures(wait);

  return omx_err;
}

#define EGL_NATIVE_BUFFER_ANDROID 0x3140
#define EGL_IMAGE_PRESERVED_KHR   0x30D2

OMX_ERRORTYPE COpenMaxVideo::AllocOMXOutputEGLTextures(void)
{
  OMX_ERRORTYPE omx_err;

  if (!eglCreateImageKHR)
  {
    GETEXTENSION(PFNEGLCREATEIMAGEKHRPROC,  eglCreateImageKHR);
  }

  if (!glEGLImageTargetTexture2DOES)
  {
    GETEXTENSION(PFNGLEGLIMAGETARGETTEXTURE2DOESPROC, glEGLImageTargetTexture2DOES);
  }

  //EGLint attrib[] = { EGL_IMAGE_PRESERVED_KHR, EGL_TRUE, EGL_NONE, EGL_NONE };

  OpenMaxVideoBuffer *egl_buffer;

  /*
  if (m_decoded_height != m_scaled_height || m_decoded_width != m_scaled_width)
  {
    OMX_CONFIG_SCALEFACTORTYPE scale;
    OMX_INIT_STRUCTURE(scale);
    scale.nPortIndex  = m_omx_output_port;
    scale.xWidth      = (long long)(1 << 16) * 0.5f;
    scale.xHeight     = (long long)(1 << 16) * 0.5f;
    OMX_SetConfig(m_omx_decoder, OMX_IndexConfigCommonScale, &scale);
    CLog::Log(LOGDEBUG, "%s::%s set scale factor : 0x%08xx0x%08x\n", 
        CLASSNAME, __func__, (unsigned int)scale.xWidth, (unsigned int)scale.xHeight);
  }
  */
  
  // Obtain the information about the output port.
  OMX_PARAM_PORTDEFINITIONTYPE port_format;
  OMX_VIDEO_PORTDEFINITIONTYPE *video_def = &port_format.format.video;

  OMX_INIT_STRUCTURE(port_format);

  port_format.nPortIndex = m_omx_output_port;

  omx_err = OMX_GetParameter(m_omx_decoder, OMX_IndexParamPortDefinition, &port_format);
  video_def->nFrameWidth        = m_decoded_width;
  video_def->nFrameHeight       = m_decoded_height;
  video_def->nStride            = m_decoded_width;
  video_def->nSliceHeight       = m_decoded_height;
  //video_def->eColorFormat       = OMX_COLOR_FormatYUV422Planar;
  video_def->eColorFormat       = OMX_COLOR_FormatYUV422SemiPlanar;
  video_def->eCompressionFormat = OMX_VIDEO_CodingUnused;
  video_def->xFramerate         = m_xFramerate;
  port_format.nBufferSize       = (m_decoded_width * m_decoded_height * 3) / 2;

  port_format.nBufferCountActual  = 10;

  OMX_SetParameter(m_omx_decoder, OMX_IndexParamPortDefinition, &port_format);
  OMX_GetParameter(m_omx_decoder, OMX_IndexParamPortDefinition, &port_format);

  //#if defined(OMX_DEBUG_VERBOSE)
  CLog::Log(LOGDEBUG,
    "%s::%s (1) - oport(%d), nFrameWidth(%lu), nFrameHeight(%lu), nStride(%lx), nBufferCountMin(%lu), nBufferCountActual(%lu), nBufferSize(%lu)\n",
    CLASSNAME, __func__, m_omx_output_port,
    port_format.format.video.nFrameWidth, port_format.format.video.nFrameHeight,port_format.format.video.nStride,
    port_format.nBufferCountMin, port_format.nBufferCountActual, port_format.nBufferSize);
  //#endif


  glActiveTexture(GL_TEXTURE0);

  for (size_t i = 0; i < port_format.nBufferCountActual; i++)
  {
    egl_buffer = new OpenMaxVideoBuffer;
    memset(egl_buffer, 0, sizeof(*egl_buffer));
    egl_buffer->width  = m_decoded_width;
    egl_buffer->height = m_decoded_height;

    // use an external buffer that's sized according to actual demux
    // packet size, start at internal's buffer size, will get deleted when
    // we start pulling demuxer packets and using demux packet sized buffers.
    //OMX_U8* data = new OMX_U8[port_format.nBufferSize];
    //omx_err = OMX_UseBuffer(m_omx_decoder, &egl_buffer->omx_buffer, m_omx_output_port, NULL, port_format.nBufferSize, data);
    omx_err = OMX_UseBuffer(m_omx_decoder, &egl_buffer->omx_buffer, m_omx_output_port, NULL, port_format.nBufferSize, NULL);
    //omx_err = OMX_AllocateBuffer(m_omx_decoder, &egl_buffer->omx_buffer, m_omx_output_port, NULL, port_format.nBufferSize);
    if (omx_err != OMX_ErrorNone)
    {
      CLog::Log(LOGERROR, "%s::%s - OMX_UseBuffer failed with omx_err(0x%x)\n",
        CLASSNAME, __func__, omx_err);

      //delete [] data;
      return(omx_err);
    }
    egl_buffer->omx_buffer->nOutputPortIndex  = m_omx_output_port;
    egl_buffer->omx_buffer->pAppPrivate       = egl_buffer;
    egl_buffer->omx_buffer->nFilledLen        = 0;
    egl_buffer->omx_buffer->nOffset           = 0;
    memset(egl_buffer->omx_buffer->pBuffer, 0x0, egl_buffer->omx_buffer->nAllocLen);

    /*
    glGenTextures(1, &egl_buffer->texture_id);
    glBindTexture(GL_TEXTURE_2D, egl_buffer->texture_id);

    // create space for buffer with a texture
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(
      GL_TEXTURE_2D,      // target
      0,                  // level
      GL_RGBA,            // internal format
      m_decoded_width,    // width
      m_decoded_height,   // height
      0,                  // border
      GL_RGBA,            // format
      GL_UNSIGNED_BYTE,   // type
      NULL);              // pixels -- will be provided later

    VerifyGLState();

    egl_buffer->graphic_handle = malloc(GRAPHIC_BUFFER_SIZE);
    GraphicBufferWrapper.fGraphicBufferCtor(egl_buffer->graphic_handle, m_decoded_width, m_decoded_height, 
        HAL_PIXEL_FORMAT_RGBA_8888,
        GRALLOC_USAGE_HW_TEXTURE | GRALLOC_USAGE_SW_WRITE_OFTEN);

    egl_buffer->native_buffer = GraphicBufferWrapper.fGraphicBufferGetNativeBuffer(egl_buffer->graphic_handle);

    if (!egl_buffer->native_buffer)
    {
      CLog::Log(LOGERROR, "%s::%s - native_buffer 0x%08x\n", CLASSNAME, __func__, (unsigned int)egl_buffer->native_buffer);
      return(OMX_ErrorUndefined);
    }

    CLog::Log(LOGDEBUG, "%s::%s - TextureId 0x%08x nativeBuffer 0x%08x\n", CLASSNAME, __func__, egl_buffer->texture_id, (unsigned int)egl_buffer->native_buffer);

    // create EGLImage from texture
    egl_buffer->egl_image = eglCreateImageKHR(
      m_egl_display,
      EGL_NO_CONTEXT, //m_egl_context
      EGL_NATIVE_BUFFER_ANDROID,
      (EGLClientBuffer)egl_buffer->native_buffer,
      attrib);

    if (!egl_buffer->egl_image)
    {
      CLog::Log(LOGERROR, "%s::%s - ERROR creating EglImage\n", CLASSNAME, __func__);
      return(OMX_ErrorUndefined);
    }

    egl_buffer->index = i;

    glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, (GLeglImageOES)egl_buffer->egl_image);
    GLint error = glGetError();
    if (error != GL_NO_ERROR) 
    {
      eglDestroyImageKHR(m_egl_display, egl_buffer->egl_image);
      CLog::Log(LOGERROR, "%s::%s - ERROR binding EglImage Texture\n", CLASSNAME, __func__);
      return(OMX_ErrorUndefined);
    }
    */

    // tell decoder output port that it will be using EGLImage
    /*
    omx_err = OMX_UseEGLImage(
      m_omx_decoder, &egl_buffer->omx_buffer, m_omx_output_port, egl_buffer, egl_buffer->egl_image);
    if (omx_err != OMX_ErrorNone)
    {
      CLog::Log(LOGERROR, "%s::%s - OMX_UseEGLImage failed with omx_err(0x%x)\n",
        CLASSNAME, __func__, omx_err);
      return(omx_err);
    }
    */

    m_omx_output_buffers.push_back(egl_buffer);

    CLog::Log(LOGDEBUG, "%s::%s - Texture %p Width %d Height %d\n",
      CLASSNAME, __func__, egl_buffer->egl_image, egl_buffer->width, egl_buffer->height);
  }

  m_omx_output_eos = false;
  while (!m_omx_output_busy.empty())
    m_omx_output_busy.pop();
  while (!m_omx_output_ready.empty())
    m_omx_output_ready.pop();

  m_readyCount  = 0; 

  return omx_err;
}

OMX_ERRORTYPE COpenMaxVideo::FreeOMXOutputEGLTextures(bool wait)
{
  OMX_ERRORTYPE omx_err = OMX_ErrorNone;
  OpenMaxVideoBuffer *egl_buffer;

  if (!eglDestroyImageKHR)
  {
    GETEXTENSION(PFNEGLDESTROYIMAGEKHRPROC, eglDestroyImageKHR);
  }

  for (size_t i = 0; i < m_omx_output_buffers.size(); i++)
  {
    egl_buffer = m_omx_output_buffers[i];
    // tell decoder output port to stop using the EGLImage
    //delete [] egl_buffer->omx_buffer->pBuffer;
    //egl_buffer->omx_buffer->pBuffer = NULL;
    omx_err = OMX_FreeBuffer(m_omx_decoder, m_omx_output_port, egl_buffer->omx_buffer);
    /*
    // destroy egl_image
    eglDestroyImageKHR(m_egl_display, egl_buffer->egl_image);
    // free texture
    glDeleteTextures(1, &egl_buffer->texture_id);
    // free graphic buffer
    if (egl_buffer->graphic_handle)
    {
      GraphicBufferWrapper.fGraphicBufferDtor(egl_buffer->graphic_handle);
      free(egl_buffer->graphic_handle);
    }
    */
    delete egl_buffer;
  }
  m_omx_output_buffers.clear();

  return omx_err;
}


////////////////////////////////////////////////////////////////////////////////////////////
// DecoderEventHandler -- OMX event callback
OMX_ERRORTYPE COpenMaxVideo::DecoderEventHandler(
  OMX_HANDLETYPE hComponent,
  OMX_PTR pAppData,
  OMX_EVENTTYPE eEvent,
  OMX_U32 nData1,
  OMX_U32 nData2,
  OMX_PTR pEventData)
{
  //OMX_ERRORTYPE omx_err;
  COpenMaxVideo *ctx = static_cast<COpenMaxVideo*>(pAppData);

  #if defined(OMX_DEBUG_VERBOSE)
  CLog::Log(LOGDEBUG,
    "COpenMaxVideo::DecoderEventHandler - hComponent(%p), eEvent(0x%x), nData1(0x%lx), nData2(0x%lx), pEventData(%p)\n",
    hComponent, eEvent, nData1, nData2, pEventData);
  #endif

  switch (eEvent)
  {
    case OMX_EventCmdComplete:
      switch(nData1)
      {
        case OMX_CommandStateSet:
          ctx->m_omx_decoder_state = (int)nData2;
          switch (ctx->m_omx_decoder_state)
          {
            case OMX_StateInvalid:
              CLog::Log(LOGDEBUG, "%s::%s - OMX_StateInvalid\n", CLASSNAME, "DecoderEventHandler");
            break;
            case OMX_StateLoaded:
              CLog::Log(LOGDEBUG, "%s::%s - OMX_StateLoaded\n", CLASSNAME, "DecoderEventHandler");
            break;
            case OMX_StateIdle:
              CLog::Log(LOGDEBUG, "%s::%s - OMX_StateIdle\n", CLASSNAME, "DecoderEventHandler");
            break;
            case OMX_StateExecuting:
              CLog::Log(LOGDEBUG, "%s::%s - OMX_StateExecuting\n", CLASSNAME, "DecoderEventHandler");
            break;
            case OMX_StatePause:
              CLog::Log(LOGDEBUG, "%s::%s - OMX_StatePause\n", CLASSNAME, "DecoderEventHandler");
            break;
            case OMX_StateWaitForResources:
              CLog::Log(LOGDEBUG, "%s::%s - OMX_StateWaitForResources\n", CLASSNAME, "DecoderEventHandler");
            break;
            default:
              CLog::Log(LOGDEBUG,
                "%s::%s - Unknown OMX_Statexxxxx, state(%d)\n",
                CLASSNAME, "DecoderEventHandler", ctx->m_omx_decoder_state);
            break;
          }
          sem_post(ctx->m_omx_decoder_state_change);
        break;
        case OMX_CommandFlush:
          sem_post(ctx->m_omx_flush);
          #if defined(OMX_DEBUG_EVENTHANDLER)
          CLog::Log(LOGDEBUG,
            "%s::%s - OMX_CommandFlush, nData2(0x%lx)\n",
            CLASSNAME, "DecoderEventHandler", nData2);
          #endif
        break;
        case OMX_CommandPortDisable:
          #if defined(OMX_DEBUG_EVENTHANDLER)
          CLog::Log(LOGDEBUG,
            "%s::%s - OMX_CommandPortDisable, nData1(0x%lx), nData2(0x%lx)\n",
            CLASSNAME, "DecoderEventHandler", nData1, nData2);
          #endif
          if (ctx->m_omx_output_port == (int)nData2)
          {
            // Got OMX_CommandPortDisable event, alloc new buffers for the output port.
            //omx_err = ctx->AllocOMXOutputBuffers();
            //omx_err = OMX_SendCommand(ctx->m_omx_decoder, OMX_CommandPortEnable, ctx->m_omx_output_port, NULL);
          }
        break;
        case OMX_CommandPortEnable:
          #if defined(OMX_DEBUG_EVENTHANDLER)
          CLog::Log(LOGDEBUG,
            "%s::%s - OMX_CommandPortEnable, nData1(0x%lx), nData2(0x%lx)\n",
            CLASSNAME, "DecoderEventHandler", nData1, nData2);
          #endif
          if (ctx->m_omx_output_port == (int)nData2)
          {
            // Got OMX_CommandPortEnable event.
            // OMX_CommandPortDisable will have re-alloced new ones so re-prime
            //ctx->PrimeFillBuffers();
          }
          ctx->m_portChanging = false;
        break;
        #if defined(OMX_DEBUG_EVENTHANDLER)
        case OMX_CommandMarkBuffer:
          CLog::Log(LOGDEBUG,
            "%s::%s - OMX_CommandMarkBuffer, nData1(0x%lx), nData2(0x%lx)\n",
            CLASSNAME, "DecoderEventHandler", nData1, nData2);
        break;
        #endif
      }
      sem_post(ctx->m_omx_cmd_complete);
    break;
    case OMX_EventBufferFlag:
      CLog::Log(LOGDEBUG, "%s::%s - OMX_EventBufferFlag port %d %d\n", CLASSNAME, "DecoderEventHandler", (int)nData1, (int)nData2);
      if (ctx->m_omx_decoder == hComponent && (nData2 & OMX_BUFFERFLAG_EOS)) {
        #if defined(OMX_DEBUG_EVENTHANDLER)
        if (ctx->m_omx_input_port  == (int)nData1)
            CLog::Log(LOGDEBUG, "%s::%s - OMX_EventBufferFlag(input)\n",
            CLASSNAME, "DecoderEventHandler");
        #endif
        if (ctx->m_omx_output_port == (int)nData1)
        {
            ctx->m_videoplayback_done = true;
            #if defined(OMX_DEBUG_EVENTHANDLER)
            CLog::Log(LOGDEBUG, "%s::%s - OMX_EventBufferFlag(output)\n",
            CLASSNAME, "DecoderEventHandler");
            #endif
        }
      }
    break;
    case OMX_EventPortSettingsChanged:
      #if defined(OMX_DEBUG_EVENTHANDLER)
      CLog::Log(LOGDEBUG,
        "%s::%s - OMX_EventPortSettingsChanged(output)\n", CLASSNAME, "DecoderEventHandler");
      #endif
      // not sure nData2 is the input/output ports in this call, docs don't say
      if (ctx->m_omx_output_port == (int)nData2)
      {
        // free the current OpenMax output buffers, you must do this before sending
        // OMX_CommandPortDisable to component as it expects output buffers
        // to be freed before it will issue a OMX_CommandPortDisable event.
        ctx->m_portChanging = true;
        //omx_err = OMX_SendCommand(ctx->m_omx_decoder, OMX_CommandPortDisable, ctx->m_omx_output_port, NULL);
        //omx_err = ctx->FreeOMXOutputBuffers(false);
      }
    break;
    #if defined(OMX_DEBUG_EVENTHANDLER)
    case OMX_EventMark:
      CLog::Log(LOGDEBUG, "%s::%s - OMX_EventMark\n", CLASSNAME, "DecoderEventHandler");
    break;
    case OMX_EventResourcesAcquired:
      CLog::Log(LOGDEBUG, "%s::%s - OMX_EventResourcesAcquired\n", CLASSNAME, "DecoderEventHandler");
    break;
    #endif
    case OMX_EventError:
      switch((OMX_S32)nData1)
      {
        case OMX_ErrorInsufficientResources:
          CLog::Log(LOGERROR, "%s::%s - OMX_EventError, insufficient resources\n",
            CLASSNAME, "DecoderEventHandler");
          // we are so frack'ed
          //exit(0);
        break;
        case OMX_ErrorFormatNotDetected:
          CLog::Log(LOGERROR, "%s::%s - OMX_EventError, cannot parse input stream\n",
            CLASSNAME, "DecoderEventHandler");
        break;
        case OMX_ErrorPortUnpopulated:
          // silently ignore these. We can get them when setting OMX_CommandPortDisable
          // on the output port and the component flushes the output buffers.
        break;
        case OMX_ErrorStreamCorrupt:
          CLog::Log(LOGERROR, "%s::%s - OMX_EventError, Bitstream corrupt\n",
            CLASSNAME, "DecoderEventHandler");
          ctx->m_videoplayback_done = true;
        break;
        default:
          CLog::Log(LOGERROR, "%s::%s - OMX_EventError detected, nData1(0x%lx), nData2(0x%lx)\n",
            CLASSNAME, "DecoderEventHandler", nData1, nData2);
        break;
      }
      // do this so we don't hang on errors
      sem_post(ctx->m_omx_flush);
      sem_post(ctx->m_omx_decoder_state_change);
      sem_post(ctx->m_omx_cmd_complete);
      sem_post(ctx->m_omx_empty_done);
      sem_post(ctx->m_omx_fill_done);
      pthread_mutex_lock(&ctx->m_omx_lock_decoder);
      ctx->m_error = VC_ERROR;
      pthread_mutex_unlock(&ctx->m_omx_lock_decoder);
    break;
    default:
      CLog::Log(LOGWARNING,
        "%s::%s - Unknown eEvent(0x%x), nData1(0x%lx), nData2(0x%lx)\n",
        CLASSNAME, "DecoderEventHandler", eEvent, nData1, nData2);
    break;
  }

  return OMX_ErrorNone;
}

// StartPlayback -- Kick off video playback.
OMX_ERRORTYPE COpenMaxVideo::StartDecoder(void)
{
  OMX_ERRORTYPE omx_err;

  #if defined(OMX_DEBUG_VERBOSE)
  CLog::Log(LOGDEBUG, "%s::%s\n", CLASSNAME, __func__);
  #endif

  // transition decoder component to IDLE state
  omx_err = SetStateForComponent(OMX_StateIdle);
  if (omx_err != OMX_ErrorNone)
  {
    CLog::Log(LOGERROR, "%s::%s - setting OMX_StateIdle failed with omx_err(0x%x)\n",
      CLASSNAME, __func__, omx_err);
    return omx_err;
  }

  // transition decoder component to executing state
  omx_err = SetStateForComponent(OMX_StateExecuting);
  if (omx_err != OMX_ErrorNone)
  {
    CLog::Log(LOGERROR, "%s::%s - setting OMX_StateExecuting failed with omx_err(0x%x)\n",
      CLASSNAME, __func__, omx_err);
    return omx_err;
  }

  //prime the omx output buffers.
  PrimeFillBuffers();

  return omx_err;
}

// StopPlayback -- Stop video playback
OMX_ERRORTYPE COpenMaxVideo::StopDecoder(void)
{
  OMX_ERRORTYPE omx_err;

  #if defined(OMX_DEBUG_VERBOSE)
  CLog::Log(LOGDEBUG, "%s::%s\n", CLASSNAME, __func__);
  #endif
  // transition decoder component from executing to idle
  omx_err = SetStateForComponent(OMX_StateIdle);
  if (omx_err != OMX_ErrorNone)
  {
    CLog::Log(LOGERROR, "%s::%s - setting OMX_StateIdle failed with omx_err(0x%x)\n",
      CLASSNAME, __func__, omx_err);
    return omx_err;
  }

  // we can free our allocated port buffers in OMX_StateIdle state.
  // free OpenMax input buffers.
  omx_err = FreeOMXInputBuffers(true);
  // free OpenMax output buffers.
  omx_err = FreeOMXOutputBuffers(true);

  // transition decoder component from idle to loaded
  omx_err = SetStateForComponent(OMX_StateLoaded);
  if (omx_err != OMX_ErrorNone)
    CLog::Log(LOGERROR,
      "%s::%s - setting OMX_StateLoaded failed with omx_err(0x%x)\n",
      CLASSNAME, __func__, omx_err);

  return omx_err;
}

bool COpenMaxVideo::SendDecoderConfig(uint8_t *extradata, int extrasize)
{
  OMX_ERRORTYPE omx_err   = OMX_ErrorNone;

  /* send decoder config */
  if (extrasize > 0 && extradata != NULL)
  {
    OMX_BUFFERHEADERTYPE *omx_buffer = NULL;

    pthread_mutex_lock(&m_omx_input_mutex);
    omx_buffer = m_omx_input_avaliable.front();
    m_omx_input_avaliable.pop();
    pthread_mutex_unlock(&m_omx_input_mutex);

    if (omx_buffer == NULL)
    {
      CLog::Log(LOGERROR, "%s::%s - buffer error 0x%08x", CLASSNAME, __func__, omx_err);
      return false;
    }

    omx_buffer->nOffset = 0;
    omx_buffer->nFilledLen = extrasize;
    if (omx_buffer->nFilledLen > omx_buffer->nAllocLen)
    {
      CLog::Log(LOGERROR, "%s::%s - omx_buffer->nFilledLen > omx_buffer->nAllocLen", CLASSNAME, __func__);
      return false;
    }

    memset((unsigned char *)omx_buffer->pBuffer, 0x0, omx_buffer->nAllocLen);
    memcpy((unsigned char *)omx_buffer->pBuffer, extradata, omx_buffer->nFilledLen);
    omx_buffer->nFlags = OMX_BUFFERFLAG_CODECCONFIG | OMX_BUFFERFLAG_ENDOFFRAME;

    omx_err = OMX_EmptyThisBuffer(m_omx_decoder, omx_buffer);
    if (omx_err != OMX_ErrorNone)
    {
      CLog::Log(LOGERROR, "COpenMaxVideo::SendDecoderConfig - OMX_EmptyThisBuffer() failed with result(0x%x)\n", omx_err);
      return false;
    }

    sem_post(m_omx_empty_done);
  }
  return true;
}

