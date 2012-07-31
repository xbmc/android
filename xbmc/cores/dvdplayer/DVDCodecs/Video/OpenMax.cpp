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

#include "OpenMax.h"
#include "DynamicDll.h"
#include "utils/log.h"

#if defined(TARGET_ANDROID)
#include "xbmc/android/activity/AndroidFeatures.h"
#endif

#include <OMX_Core.h>
#include <OMX_Component.h>
#include <OMX_Index.h>
#include <OMX_Image.h>

#define CLASSNAME "COpenMax"

static const struct
{
    CodecID codec;
    OMX_VIDEO_CODINGTYPE omx_codec;
    const char *role;

} g_codec_formats[] =
{
    { CODEC_ID_MPEG1VIDEO,  OMX_VIDEO_CodingMPEG2, "video_decoder.mpeg2" },
    { CODEC_ID_MPEG2VIDEO,  OMX_VIDEO_CodingMPEG2, "video_decoder.mpeg2" },
    { CODEC_ID_MSMPEG4V1,   OMX_VIDEO_CodingMPEG4, "video_decoder.mpeg4" },
    { CODEC_ID_MSMPEG4V2,   OMX_VIDEO_CodingMPEG4, "video_decoder.mpeg4" },
    { CODEC_ID_MSMPEG4V3,   OMX_VIDEO_CodingMPEG4, "video_decoder.mpeg4" },
    { CODEC_ID_MPEG4,       OMX_VIDEO_CodingMPEG4, "video_decoder.mpeg4" },
    { CODEC_ID_H264,        OMX_VIDEO_CodingAVC,   "video_decoder.avc"   },
    { CODEC_ID_H263,        OMX_VIDEO_CodingH263,  "video_decoder.h263"  },
    { CODEC_ID_WMV1,        OMX_VIDEO_CodingWMV,   "video_decoder.wmv"   },
    { CODEC_ID_WMV2,        OMX_VIDEO_CodingWMV,   "video_decoder.wmv"   },
    { CODEC_ID_WMV3,        OMX_VIDEO_CodingWMV,   "video_decoder.wmv"   },
    { CODEC_ID_VC1,         OMX_VIDEO_CodingWMV,   "video_decoder.vc1"   },
    { CODEC_ID_NONE,        OMX_VIDEO_CodingUnused, 0 }
};

////////////////////////////////////////////////////////////////////////////////////////////
class DllLibOpenMaxInterface
{
public:
  virtual ~DllLibOpenMaxInterface() {}

  virtual OMX_ERRORTYPE OMX_Init(void) = 0;
  virtual OMX_ERRORTYPE OMX_Deinit(void) = 0;
  virtual OMX_ERRORTYPE OMX_GetHandle(
    OMX_HANDLETYPE *pHandle, OMX_STRING cComponentName, OMX_PTR pAppData, OMX_CALLBACKTYPE *pCallBacks) = 0;
  virtual OMX_ERRORTYPE OMX_FreeHandle(OMX_HANDLETYPE hComponent) = 0;
  virtual OMX_ERRORTYPE OMX_GetComponentsOfRole(OMX_STRING role, OMX_U32 *pNumComps, OMX_U8 **compNames) = 0;
  virtual OMX_ERRORTYPE OMX_GetRolesOfComponent(OMX_STRING compName, OMX_U32 *pNumRoles, OMX_U8 **roles) = 0;
  virtual OMX_ERRORTYPE OMX_ComponentNameEnum(OMX_STRING cComponentName, OMX_U32 nNameLength, OMX_U32 nIndex) = 0;
};

class DllLibOpenMax : public DllDynamic, DllLibOpenMaxInterface
{
#if defined(TARGET_ANDROID)
  DECLARE_DLL_WRAPPER(DllLibOpenMax, CAndroidFeatures::GetLibiomxName().c_str())
#else
  DECLARE_DLL_WRAPPER(DllLibOpenMax, "/usr/lib/libnvomx.so")
#endif

  DEFINE_METHOD0(OMX_ERRORTYPE, OMX_Init)
  DEFINE_METHOD0(OMX_ERRORTYPE, OMX_Deinit)
  DEFINE_METHOD4(OMX_ERRORTYPE, OMX_GetHandle, (OMX_HANDLETYPE *p1, OMX_STRING p2, OMX_PTR p3, OMX_CALLBACKTYPE *p4))
  DEFINE_METHOD1(OMX_ERRORTYPE, OMX_FreeHandle, (OMX_HANDLETYPE p1))
  DEFINE_METHOD3(OMX_ERRORTYPE, OMX_GetComponentsOfRole, (OMX_STRING p1, OMX_U32 *p2, OMX_U8 **p3))
  DEFINE_METHOD3(OMX_ERRORTYPE, OMX_GetRolesOfComponent, (OMX_STRING p1, OMX_U32 *p2, OMX_U8 **p3))
  DEFINE_METHOD3(OMX_ERRORTYPE, OMX_ComponentNameEnum, (OMX_STRING p1, OMX_U32 p2, OMX_U32 p3))
  BEGIN_METHOD_RESOLVE()
    RESOLVE_METHOD(OMX_Init)
    RESOLVE_METHOD(OMX_Deinit)
    RESOLVE_METHOD(OMX_GetHandle)
    RESOLVE_METHOD(OMX_FreeHandle)
    RESOLVE_METHOD(OMX_GetComponentsOfRole)
    RESOLVE_METHOD(OMX_GetRolesOfComponent)
    RESOLVE_METHOD(OMX_ComponentNameEnum)
  END_METHOD_RESOLVE()
};

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
COpenMax::COpenMax()
{
  #if defined(OMX_DEBUG_VERBOSE)
  CLog::Log(LOGDEBUG, "%s::%s\n", CLASSNAME, __func__);
  #endif
  m_dll = new DllLibOpenMax;
  m_dll->Load();
  m_is_open = false;

  m_omx_decoder = NULL;
  /*
  m_omx_flush_input  = (sem_t*)malloc(sizeof(sem_t));
  sem_init(m_omx_flush_input, 0, 0);
  m_omx_flush_output = (sem_t*)malloc(sizeof(sem_t));
  sem_init(m_omx_flush_output, 0, 0);
  */
}

COpenMax::~COpenMax()
{
  #if defined(OMX_DEBUG_VERBOSE)
  CLog::Log(LOGDEBUG, "%s::%s\n", CLASSNAME, __func__);
  #endif
  /*
  sem_destroy(m_omx_flush_input);
  free(m_omx_flush_input);
  sem_destroy(m_omx_flush_output);
  free(m_omx_flush_output);
  */
  delete m_dll;
}




////////////////////////////////////////////////////////////////////////////////////////////
// DecoderEventHandler -- OMX event callback
OMX_ERRORTYPE COpenMax::DecoderEventHandlerCallback(
  OMX_HANDLETYPE hComponent,
  OMX_PTR pAppData,
  OMX_EVENTTYPE eEvent,
  OMX_U32 nData1,
  OMX_U32 nData2,
  OMX_PTR pEventData)
{
  COpenMax *ctx = (COpenMax*)pAppData;
  return ctx->DecoderEventHandler(hComponent, pAppData, eEvent, nData1, nData2, pEventData);
}

// DecoderEmptyBufferDone -- OpenMax input buffer has been emptied
OMX_ERRORTYPE COpenMax::DecoderEmptyBufferDoneCallback(
  OMX_HANDLETYPE hComponent,
  OMX_PTR pAppData,
  OMX_BUFFERHEADERTYPE* pBuffer)
{
  COpenMax *ctx = (COpenMax*)pAppData;
  return ctx->DecoderEmptyBufferDone( hComponent, pAppData, pBuffer);
}

// DecoderFillBufferDone -- OpenMax output buffer has been filled
OMX_ERRORTYPE COpenMax::DecoderFillBufferDoneCallback(
  OMX_HANDLETYPE hComponent,
  OMX_PTR pAppData,
  OMX_BUFFERHEADERTYPE* pBuffer)
{
  COpenMax *ctx = (COpenMax*)pAppData;
  return ctx->DecoderFillBufferDone(hComponent, pAppData, pBuffer);
}

// Wait for a component to transition to the specified state
OMX_ERRORTYPE COpenMax::WaitForState(OMX_STATETYPE state)
{
  OMX_ERRORTYPE omx_error = OMX_ErrorNone;
  OMX_STATETYPE test_state;
  int tries = 0;
  struct timespec timeout;
  omx_error = OMX_GetState(m_omx_decoder, &test_state);

  #if defined(OMX_DEBUG_VERBOSE)
  CLog::Log(LOGDEBUG, "%s::%s - waiting for state(%d)\n", CLASSNAME, __func__, state);
  #endif
  while ((omx_error == OMX_ErrorNone) && (test_state != state)) 
  {
    clock_gettime(CLOCK_REALTIME, &timeout);
    timeout.tv_sec += 1;
    sem_timedwait(m_omx_decoder_state_change, &timeout);
    if (errno == ETIMEDOUT)
      tries++;
    if (tries > 5)
      return OMX_ErrorUndefined;

    omx_error = OMX_GetState(m_omx_decoder, &test_state);
  }

  return omx_error;
}

// SetStateForAllComponents
// Blocks until all state changes have completed
OMX_ERRORTYPE COpenMax::SetStateForComponent(OMX_STATETYPE state)
{
  OMX_ERRORTYPE omx_err;
  OMX_STATETYPE state_current = OMX_StateMax;

  // do not set state on same state
  OMX_GetState(m_omx_decoder, &state_current);
  if (state == state_current)
    return OMX_ErrorNone;

  #if defined(OMX_DEBUG_VERBOSE)
  CLog::Log(LOGDEBUG, "%s::%s - state(%d)\n", CLASSNAME, __func__, state);
  #endif
  omx_err = OMX_SendCommand(m_omx_decoder, OMX_CommandStateSet, state, 0);
  if (omx_err != OMX_ErrorNone)
    CLog::Log(LOGERROR, "%s::%s - OMX_CommandStateSet failed with omx_err(0x%x)\n",
      CLASSNAME, __func__, omx_err);
  else
    omx_err = WaitForState(state);

  return omx_err;
}

std::string COpenMax::GetOmxRole(CodecID codec)
{
  int i = 0;
  std::string role = "";
  
  for (i = 0; ;i++)
  {
    if (g_codec_formats[i].codec == CODEC_ID_NONE && g_codec_formats[i].omx_codec == OMX_VIDEO_CodingUnused && g_codec_formats[i].role == 0)
      break;
    if (g_codec_formats[i].codec == codec)
    {
      role = std::string(g_codec_formats[i].role);
      break;
    }
  }
  return role;
}

OMX_VIDEO_CODINGTYPE COpenMax::GetOmxCodingType(CodecID codec)
{
  int i = 0;
  OMX_VIDEO_CODINGTYPE omx_codec = OMX_VIDEO_CodingUnused;

  for (i = 0; ; i++)
  {
    if (g_codec_formats[i].codec == CODEC_ID_NONE && g_codec_formats[i].omx_codec == OMX_VIDEO_CodingUnused && g_codec_formats[i].role == 0)
      break;
    if (g_codec_formats[i].codec == codec)
    {
      omx_codec = g_codec_formats[i].omx_codec;
      break;
    }
  }
  return omx_codec;
}

bool COpenMax::Initialize( const CStdString &role_name)
{
  OMX_ERRORTYPE omx_err = m_dll->OMX_Init();
  if (omx_err != OMX_ErrorNone)
  {
    CLog::Log(LOGERROR, "%s::%s - OpenMax failed to init, status(%d), ", CLASSNAME, __func__, omx_err );
    return false;
  }

  char name[OMX_MAX_STRINGNAME_SIZE];
  OMX_U32 roles = 0;
  OMX_U8 **proles = 0;
  unsigned int i = 0, j = 0;
  std::string component_name = "";
  bool bFound = false;

  CLog::Log(LOGDEBUG, "%s::%s - OpenMax search component for role : %s", CLASSNAME, __func__, role_name.c_str());
  if (role_name != "")
  {
    for (i = 0; ; i++)
    {
      omx_err = m_dll->OMX_ComponentNameEnum(name, OMX_MAX_STRINGNAME_SIZE, i);
      if (omx_err != OMX_ErrorNone)
        break;

      CLog::Log(LOGDEBUG, "%s::%s - OpenMax component : %s", CLASSNAME, __func__, name);

      omx_err = m_dll->OMX_GetRolesOfComponent(name, &roles, 0);
      if (omx_err != OMX_ErrorNone || !roles)
        continue;

      proles = (OMX_U8**)malloc(roles * (sizeof(OMX_U8*) + OMX_MAX_STRINGNAME_SIZE));
      if (!proles)
        continue;

      for (j = 0; j < roles; j++)
        proles[j] = ((OMX_U8 *)(&proles[roles])) + j * OMX_MAX_STRINGNAME_SIZE;

      omx_err = m_dll->OMX_GetRolesOfComponent(name, &roles, proles);
      if (omx_err != OMX_ErrorNone)
        roles = 0;

      for (j = 0; j < roles; j++)
      {
        CLog::Log(LOGDEBUG, "%s::%s - OpenMax component : %s role : %s", CLASSNAME, __func__, name, (char *)proles[j]);
        if (!strncmp((char *)proles[j], role_name, role_name.length()))
        {
          if (!strncmp(name, "OMX.PV.", 7))
            continue;
          if (!strncmp(name, "OMX.google.", 11))
            continue;
          if (!strncmp(name, "OMX.ARICENT.", 12))
            continue;
          if (!strncmp(name, "OMX.Nvidia.h264.decode.secure", 29))
            continue;

          component_name = name;
          bFound = true;
          break;
        }
      }
      
      free(proles);

      if (bFound)
        break;
    }
  }

  if (component_name == "")
    return false;

  CLog::Log(LOGINFO, "%s::%s - Found omx decoder : %s\n", CLASSNAME, __func__, component_name.c_str());

  // Get video decoder handle setting up callbacks, component is in loaded state on return.
  m_callbacks.EventHandler    = &COpenMax::DecoderEventHandlerCallback;
  m_callbacks.EmptyBufferDone = &COpenMax::DecoderEmptyBufferDoneCallback;
  m_callbacks.FillBufferDone  = &COpenMax::DecoderFillBufferDoneCallback;

  omx_err = m_dll->OMX_GetHandle(&m_omx_decoder, (char*)component_name.c_str(), this, &m_callbacks);
  if (omx_err != OMX_ErrorNone)
  {
    CLog::Log(LOGERROR,
      "%s::%s - could not get decoder handle\n", CLASSNAME, __func__);
    m_dll->OMX_Deinit();
    return false;
  }
  return true;
}

void COpenMax::Deinitialize()
{
  CLog::Log(LOGERROR,
    "%s::%s - failed to get component port parameter\n", CLASSNAME, __func__);
  m_dll->OMX_FreeHandle(m_omx_decoder);
  m_omx_decoder = NULL;
  m_dll->OMX_Deinit();
}

// OpenMax decoder callback routines.
OMX_ERRORTYPE COpenMax::DecoderEventHandler(OMX_HANDLETYPE hComponent, OMX_PTR pAppData,
  OMX_EVENTTYPE eEvent, OMX_U32 nData1, OMX_U32 nData2, OMX_PTR pEventData)
{
  return OMX_ErrorNone;
}

OMX_ERRORTYPE COpenMax::DecoderEmptyBufferDone(OMX_HANDLETYPE hComponent, OMX_PTR pAppData, OMX_BUFFERHEADERTYPE* pBuffer)
{
  return OMX_ErrorNone;
}

OMX_ERRORTYPE COpenMax::DecoderFillBufferDone(OMX_HANDLETYPE hComponent, OMX_PTR pAppData, OMX_BUFFERHEADERTYPE* pBufferHeader)
{
  return OMX_ErrorNone;
}
