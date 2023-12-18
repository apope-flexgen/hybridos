/*****************************************************************************/
/* Triangle MicroWorks, Inc.                        Copyright (c) 1997-2022  */
/*****************************************************************************/
/*                                                                           */
/* This file is the property of:                                             */
/*                                                                           */
/*                       Triangle MicroWorks, Inc.                           */
/*                      Raleigh, North Carolina USA                          */
/*                       www.TriangleMicroWorks.com                          */
/* (919) 870 - 6615                                                          */
/*                                                                           */
/* This Source Code and the associated Documentation contain proprietary     */
/* information of Triangle MicroWorks, Inc. and may not be copied or         */
/* distributed in any form without the written permission of Triangle        */
/* MicroWorks, Inc.  Copies of the source code may be made only for backup   */
/* purposes.                                                                 */
/*                                                                           */
/* Your License agreement may limit the installation of this source code to  */
/* specific products.  Before installing this source code on a new           */
/* application, check your license agreement to ensure it allows use on the  */
/* product in question.  Contact Triangle MicroWorks for information about   */
/* extending the number of products that may use this source code library or */
/* obtaining the newest revision.                                            */
/*                                                                           */
/*****************************************************************************/


/* file: tmwtarg.c
 * description: Windows implementation of target routines defined in 
 *  tmwtarg.h for use with WinIoTarg.
 * This target implementation is available for customers whose target
 * platform is a Microsoft Windows platform. In some cases this
 * target implementation is also useful as a reference when porting
 * to other platforms.
 */
#include "tmwscl/utils/tmwtarg.h"
#include "tmwscl/utils/tmwcnfg.h"
#include "tmwscl/utils/tmwsim.h"
#include "tmwscl/utils/tmwpltmr.h"

#include "Windows.h"
#include "WinIoTarg/include/WinIoTarg.h"
#include "tmwtargio.h"
#include <stdlib.h>
#if _MSC_VER > 1400
#include <strsafe.h>
#else
#include <stdarg.h>
#endif


/* Big Endian vs Little Endian
 * For all protocols currently supported by the Triangle MicroWorks
 * source code libraries the message byte order is least significant
 * byte first(LSB). The following get/store routines were written
 * to allow them to work on either a LSB first (little-endian) or Most
 * Significant Byte first(MSB) processors. However, because of differences
 * in the way 64 bit floating point values are stored in memory, it may
 * be necessary to modify tmwtarg_get64 and tmwtarg_put64.(These functions
 * are currently only used by DNP for 64 bit floating point TMWTYPES_DOUBLE
 * and not by the IEC 60870-5 and modbus protocols).
 */

 /* function: tmwtarg_get8 */
void TMWDEFS_GLOBAL tmwtarg_get8(
  const TMWTYPES_UCHAR *pSource,
  TMWTYPES_UCHAR *pDest)
{
  *pDest = *pSource;
}

/* function: tmwtarg_store8 */
void TMWDEFS_GLOBAL tmwtarg_store8(
  const TMWTYPES_UCHAR *pSource,
  TMWTYPES_UCHAR *pDest)
{
  *pDest = *pSource;
}

/* function: tmwtarg_get16 */
void TMWDEFS_GLOBAL tmwtarg_get16(
  const TMWTYPES_UCHAR *pSource,
  TMWTYPES_USHORT *pDest)
{
  *pDest = (TMWTYPES_USHORT)(pSource[0] | ((TMWTYPES_ULONG)pSource[1] << 8));
}

/* function: tmwtarg_store16 */
void TMWDEFS_GLOBAL tmwtarg_store16(
  const TMWTYPES_USHORT *pSource,
  TMWTYPES_UCHAR *pDest)
{
  pDest[0] = (TMWTYPES_UCHAR)*pSource;
  pDest[1] = (TMWTYPES_UCHAR)(*pSource >> 8);
}

/* function: tmwtarg_get24 */
void TMWDEFS_GLOBAL tmwtarg_get24(
  const TMWTYPES_UCHAR *pSource,
  TMWTYPES_ULONG *pDest)
{
  *pDest = (TMWTYPES_ULONG)(pSource[0] | ((TMWTYPES_ULONG)pSource[1] << 8) | ((TMWTYPES_ULONG)pSource[2] << 16));
}

/* function: tmwtarg_store24 */
void TMWDEFS_GLOBAL tmwtarg_store24(
  const TMWTYPES_ULONG *pSource,
  TMWTYPES_UCHAR *pDest)
{
  pDest[0] = (TMWTYPES_UCHAR)*pSource;
  pDest[1] = (TMWTYPES_UCHAR)(*pSource >> 8);
  pDest[2] = (TMWTYPES_UCHAR)(*pSource >> 16);
}

/* function: tmwtarg_get32 */
void TMWDEFS_GLOBAL tmwtarg_get32(
  const TMWTYPES_UCHAR *pSource,
  TMWTYPES_ULONG *pDest)
{
  *pDest = (TMWTYPES_ULONG)(pSource[0] | ((TMWTYPES_ULONG)pSource[1] << 8) | ((TMWTYPES_ULONG)pSource[2] << 16) | ((TMWTYPES_ULONG)pSource[3] << 24));
}

void TMWDEFS_GLOBAL tmwtarg_store32(
  const TMWTYPES_ULONG *pSource,
  TMWTYPES_UCHAR *pDest)
{
  pDest[0] = (TMWTYPES_UCHAR)*pSource;
  pDest[1] = (TMWTYPES_UCHAR)(*pSource >> 8);
  pDest[2] = (TMWTYPES_UCHAR)(*pSource >> 16);
  pDest[3] = (TMWTYPES_UCHAR)(*pSource >> 24);
}

/* function: tmwtarg_get64 */
void TMWDEFS_GLOBAL tmwtarg_get64(
  const TMWTYPES_UCHAR *pSource,
  TMWTYPES_DOUBLE *pDest)
{
  *((TMWTYPES_UINT64 *)pDest) = (TMWTYPES_UINT64)(pSource[0] |
    ((TMWTYPES_UINT64)pSource[1] << 8) | 
    ((TMWTYPES_UINT64)pSource[2] << 16) |
    ((TMWTYPES_UINT64)pSource[3] << 24) |
    ((TMWTYPES_UINT64)pSource[4] << 32) |
    ((TMWTYPES_UINT64)pSource[5] << 40) |
    ((TMWTYPES_UINT64)pSource[6] << 48) |
    ((TMWTYPES_UINT64)pSource[7] << 56));
}

/* function: tmwtarg_store64 */
void TMWDEFS_GLOBAL tmwtarg_store64(
  const TMWTYPES_DOUBLE *pSource,
  TMWTYPES_UCHAR *pDest)
{
  pDest[0] = (TMWTYPES_UCHAR)*((TMWTYPES_UINT64 *)pSource);
  pDest[1] = (TMWTYPES_UCHAR)(*((TMWTYPES_UINT64 *)pSource) >> 8);
  pDest[2] = (TMWTYPES_UCHAR)(*((TMWTYPES_UINT64 *)pSource) >> 16);
  pDest[3] = (TMWTYPES_UCHAR)(*((TMWTYPES_UINT64 *)pSource) >> 24);
  pDest[4] = (TMWTYPES_UCHAR)(*((TMWTYPES_UINT64 *)pSource) >> 32);
  pDest[5] = (TMWTYPES_UCHAR)(*((TMWTYPES_UINT64 *)pSource) >> 40);
  pDest[6] = (TMWTYPES_UCHAR)(*((TMWTYPES_UINT64 *)pSource) >> 48);
  pDest[7] = (TMWTYPES_UCHAR)(*((TMWTYPES_UINT64 *)pSource) >> 56);
}

/* function: tmwtarg_getSFloat */
void TMWDEFS_GLOBAL tmwtarg_getSFloat(
  const TMWTYPES_UCHAR *pSource,
  TMWTYPES_SFLOAT *pDest)
{
  /* Extract the IEEE-754 format floating point from the message
  * and if necessary convert it to the native floating point format.
  * This default implementation will assume native IEEE-754 format
  */
  tmwtarg_get32(pSource, (TMWTYPES_ULONG*)pDest);
}

/* function: tmwtarg_storeSFloat */
void TMWDEFS_GLOBAL tmwtarg_storeSFloat(
  const TMWTYPES_SFLOAT *pSource,
  TMWTYPES_UCHAR *pDest)
{
  /* If necessary, convert from native floating point to
  * IEEE-754 format and store in message.
  * This default implementation will assume native IEEE-754 format
  */
  tmwtarg_store32((TMWTYPES_ULONG*)pSource, pDest);
}

/* function: tmwtarg_appendString
* purpose: Append two string allocating new memory and freeing original
*  string. This method is currently only required to support the generation
*  of an XML document from the target database.
* arguments:
*  pStr1 - String to append to, call tmwtarg_free when done
*  pStr2 - String to append to pStr1
* returns:
*  new string which contains str2 appended to str1
*/
TMWTYPES_CHAR *tmwtarg_appendString(
  TMWTYPES_CHAR *pStr1,
  TMWTYPES_CHAR *pStr2)
{
  TMWTYPES_CHAR *result;
  size_t length1;
  size_t length2;

  /* Change this implementation if desired */

  length1 = 0;
  length2 = strlen(pStr2) + 1;
  if (pStr1 != TMWDEFS_NULL)
  {
    length1 = strlen(pStr1);
    length2 += length1;
  }

  result = (TMWTYPES_CHAR *)tmwtarg_alloc((TMWTYPES_UINT)length2);
  if (result != TMWDEFS_NULL)
  {
    result[0] = '\0';
    if (pStr1 != TMWDEFS_NULL)
    {
      STRCPY(result, length2, pStr1);
      tmwtarg_free(pStr1);
    }
    STRCPY(&result[length1], length2 - length1, pStr2);
  }
  return result;
}

/* function: tmwtarg_initConfig
* purpose: Initialize the TMW target layer.
*  This routine should be called to initialize all the members of the
*  data structure to the default values. The caller should then modify
*  individual data fields as desired. The resulting structure will be
*  passed as an argument to tmwtarg_initChannel
* arguments:
*  pConfig - pointer to target layer configuration data structure to
*   be initialized
* returns:
*  void
*/
void TMWDEFS_GLOBAL tmwtarg_initConfig(
  TMWTARG_CONFIG *pConfig)
{
  /* These defaults may be changed,
  * All fields in the TMWTARG_CONFIG structure should be initialized
  * appropriately
  */

  pConfig->interCharTimeout = 0;
  pConfig->numCharTimesBetweenFrames = 4; /* this is only used for modbus rtu */

  /* Channel callback function */
  pConfig->pChannelCallback = TMWDEFS_NULL;
  pConfig->pCallbackParam = TMWDEFS_NULL;

  /* Channel ready callback function */
  pConfig->pChannelReadyCallback = TMWDEFS_NULL;
  pConfig->pChannelReadyCbkParam = TMWDEFS_NULL;
  pConfig->connectRetry = TMWDEFS_SECONDS(1);
}


#if defined(TMWCNFG_INCLUDE_ASSERTS)
void TMWAssertion(const char *expr, const char *fileName, int lineNum)
{
#if _MSC_VER > 1400
  char buf[1024];
  
  StringCchPrintfA
    (
    buf,
    1024,
    "%s\nFile: %s, Line: %d",
    expr,
    fileName,
    (int) lineNum
    );
#endif  
  return;
}
#endif /* TMWCNFG_INCLUDE_ASSERTS */


#if defined(__GNUC__)
extern TMWTYPES_INT _vsnprintf(TMWTYPES_CHAR *string, size_t maxlen, const TMWTYPES_CHAR *format, va_list args);
#endif

/* function: tmwtarg_alloc */
void * TMWDEFS_GLOBAL tmwtarg_alloc(TMWTYPES_UINT numBytes)
{
#if TMWCNFG_USE_DYNAMIC_MEMORY
  return (malloc(numBytes));
#else
  TMWTARG_UNUSED_PARAM(numBytes);
  return (TMWDEFS_NULL);
#endif
}

/* function: tmwtarg_calloc */
void * TMWDEFS_GLOBAL tmwtarg_calloc(TMWTYPES_UINT num, TMWTYPES_UINT size)
{
#if TMWCNFG_USE_DYNAMIC_MEMORY
  return (calloc(num, size));
#else
  TMWTARG_UNUSED_PARAM(num);
  TMWTARG_UNUSED_PARAM(size);
  return (TMWDEFS_NULL);
#endif
}

/* function: tmwtarg_free */
void TMWDEFS_GLOBAL tmwtarg_free(void *pBuf)
{
#if TMWCNFG_USE_DYNAMIC_MEMORY
  free(pBuf);
#else
  TMWTARG_UNUSED_PARAM(pBuf);
#endif
  return;
}

#pragma warning(disable: 4793) /* methods are compiled as native (clr warning) */
/* function: tmwtarg_snprintf */
TMWTYPES_INT TMWDEFS_GLOBAL tmwtarg_snprintf(
  TMWTYPES_CHAR *buf,
  TMWTYPES_UINT count,
  const TMWTYPES_CHAR *format,
  ...)
{
  va_list va;
  size_t len;
#if TMWCNFG_HAS_VSNPRINTF
#else
  TMWTARG_UNUSED_PARAM(count);
#endif

  va_start(va, format);
#if TMWCNFG_HAS_VSNPRINTF
  (void)StringCchVPrintfA(buf, count, format, va);
#else
  (void)vsprintf(buf, format, va);
#endif
  va_end(va);
  len = strlen(buf); /* some sprintf's don't return the number of bytes written */

  ASSERT(len<(TMWTYPES_INT)count); /* if count == len then we may have a buffer overrun */

  if (len <= 0)
    return (0);
  return ((TMWTYPES_INT)len);
}

#if TMWCNFG_SUPPORT_DIAG
TMWTARGP_PUT_DIAG_STRING_FUNC pPutDiagStringFunc = NULL;

/* function: tmwtarg_putDiagString */
void TMWDEFS_GLOBAL tmwtarg_putDiagString(
  const TMWDIAG_ANLZ_ID *pAnlzId,
  const TMWTYPES_CHAR *pString)
{
/* Log a diagnostic string to a console, file, etc. The pAnlzId
 * structure might be used to filter the display if desired.
 */
  if (pPutDiagStringFunc)
  {
    pPutDiagStringFunc(pAnlzId,pString);
    return;
  }
}
#endif /* TMWCNFG_SUPPORT_DIAG */

/* function: tmwtarg_sleep */
void TMWDEFS_GLOBAL tmwtarg_sleep(TMWTYPES_MILLISECONDS time)
{
/* Put the process to sleep for 'time' milliseconds
 */
  Sleep(time);
}

#if TMWCNFG_SUPPORT_THREADS
void tmwtarg__lockInit(TMWDEFS_RESOURCE_LOCK *pLock)
{
  WinIoTarg_lockInit(pLock);
}

 /* This function is only required for 104 redundancy with a multi-threaded 
  * architecture. It will allow the use of a single lock for the redundancy 
  * group as well as the redundant connection channels.
  */
void tmwtarg__lockShare(TMWDEFS_RESOURCE_LOCK *pLock, TMWDEFS_RESOURCE_LOCK *pLock1)
{
  *pLock1 = *pLock;
}

void tmwtarg__lockSection(TMWDEFS_RESOURCE_LOCK *pLock)
{
  WinIoTarg_lockSection(pLock);
}

void tmwtarg__unlockSection(TMWDEFS_RESOURCE_LOCK *pLock)
{
  WinIoTarg_unlockSection(pLock);
}

void tmwtarg__lockDelete(TMWDEFS_RESOURCE_LOCK *pLock)
{
  WinIoTarg_lockDelete(pLock);
}
#endif /* TMWCNFG_SUPPORT_THREADS */

 
TMWTARGP_GET_DATETIME_FUNC pGetDateTimeFunc = NULL;

/* function: _getDateTime */
void TMWDEFS_LOCAL _getDateTime(
  TMWDTIME *pDateTime)
{
  /* Return the time for this session */
  if (pGetDateTimeFunc)
  {  
    pGetDateTimeFunc(pDateTime);
    return;
  }
  WinIoTarg_getDateTime(pDateTime); 
}


TMWTARGP_SET_DATETIME_FUNC pSetDateTimeFunc = NULL;

/* function: _setDateTime */
TMWTYPES_BOOL TMWDEFS_LOCAL _setDateTime(
  TMWDTIME *pDateTime)
{
  /* Set the time for this session */
  if (pSetDateTimeFunc)
  {  
    return(pSetDateTimeFunc(pDateTime));
  } 
  return (WinIoTarg_setDateTime(pDateTime));
}


/* function: tmwtarg_getDateTime */
void TMWDEFS_GLOBAL tmwtarg_getDateTime(
  TMWDTIME *pDateTime)
{
  /* Current time, is always genuine, not substituted */
  pDateTime->genuineTime = TMWDEFS_TRUE; 
  pDateTime->invalid = TMWDEFS_FALSE; 

  /* Some protocols (ie DNP3) are required to use UTC time or if
   * devices span multiple time zones it may be recommended to use UTC time.
   * In those cases this function can be modified to return UTC time. It
   * may be necessary to return different time for different sessions
   * (pDateTime->pSession) because of time syncs received or because some
   * sessions should use UTC time. 
   * This example allows for a callback function to be registered to determine if
   * pDateTime->pSession should use UTC time (_getDateTime allows for a callback 
   * function to be registered for this purpose).
   */

/* Get the current system date and time and store into a TMWDTIME
 * structure. The TMWDTIME structure is defined in tmwdtime.h.
 */
  _getDateTime(pDateTime);
}

/* function: tmwtarg_setDateTime */
TMWTYPES_BOOL TMWDEFS_GLOBAL tmwtarg_setDateTime(
  TMWDTIME *pDateTime)
{
/* Set the current system date and time. This routine is only called
 * on slave devices as the result of a clock synchronization request.
 * For devices where it is not desirable to change the system time
 * as the result of a clock synchronization, it may be desirable to
 * implement a system that reads the system time at startup and uses
 * this time as a time base, maintaining the current time by using
 * the millisecond timer, until tmwtarg_setDateTime is called, after
 * which the new time is used as a time base.
 */
  return (_setDateTime(pDateTime));
}

/* function: tmwtarg_getMSTime */
TMWTYPES_MILLISECONDS TMWDEFS_GLOBAL tmwtarg_getMSTime(void)
{
/* The returned values must use the entire 32 bit dynamic range
 * but do not require millisecond resolution(i.e. it is not
 * important that the value always increment by one).
 */
  return (GetTickCount());
}

#if !TMWCNFG_MULTIPLE_TIMER_QS 
TMWTARGP_START_TIMER_FUNC pStartTimerFunc = NULL;

TMWTARGP_CANCEL_TIMER_FUNC pCancelTimerFunc = NULL;

/* function: tmwtarg_startTimer() */
void TMWDEFS_GLOBAL tmwtarg_startTimer(
  TMWTYPES_MILLISECONDS timeout,
  TMWTYPES_CALLBACK_FUNC pCallbackFunc,
  void *pCallbackParam)
{
/* Only one timer is required since the source code library will
 * maintain a list of pending timers and only require an external
 * timer for the timer that will expire in the shortest time.
 * Also, a polled timer is implemented in tmwpltmr.h/c that can
 * be used for systems that do not support an event timer. The
 * default implementation can use the polled timer. 
 */
  if (pStartTimerFunc)
  {
    pStartTimerFunc(timeout, pCallbackFunc, pCallbackParam);
  }
  else
  {
    tmwpltmr_startTimer(timeout, pCallbackFunc, pCallbackParam);
  }
}

/* function: tmwtarg_cancelTimer */
void TMWDEFS_GLOBAL tmwtarg_cancelTimer(void)
{
/* Cancel the current event timer, no further timer callbacks
 * will be invoked. The default implementation uses the polled
 * timer. 
 */
  if (pCancelTimerFunc)
  {
    pCancelTimerFunc();
  }
  else
  {
    tmwpltmr_cancelTimer();
  }
}
#else /* TMWCNFG_MULTIPLE_TIMER_QS */

 
VOID CALLBACK _MultiTimerCallback(PVOID lpParam, BOOLEAN TimerOrWaitFired)
{
  TMWCHNL *pChannel = (TMWCHNL *)lpParam;
  if (pChannel->pMultiTimerCallback)
  {
    pChannel->pMultiTimerCallback(lpParam);
  }
}

/* If the ChangeTimerQueueTimer function is called on a one-shot timer that has already      */
/* expired, the timer is not updated. So Windows MultiTimers are created as periodic         */
/* timers eventhough the SCL is using them as one-shot timers with a variable timout period. */
/* Thus the period value is somewhat arbitrary, but must be non-zero.                        */
#define TIMER_QUEUE_PERIOD 1000

/* function: tmwtarg_initMultiTimer */
TMWTYPES_INT TMWDEFS_GLOBAL tmwtarg_initMultiTimer(
  TMWCHNL                *pChannel)
{
#if TMWTARG_SUPPORT_MULTI_TIMER
  int lastError = 0;
  if (CreateTimerQueueTimer(&pChannel->pMultiTimerHandle, NULL, _MultiTimerCallback, pChannel, 0, TIMER_QUEUE_PERIOD, 0) == 0)
  {
    lastError = GetLastError();
  }
  return lastError;
#else
  return 0;
#endif
}

/* function: tmwtarg_setMultiTimer */
TMWTYPES_INT TMWDEFS_GLOBAL tmwtarg_setMultiTimer(
  TMWCHNL                *pChannel,
  TMWTYPES_MILLISECONDS   timeout) 
{
#if TMWTARG_SUPPORT_MULTI_TIMER
  int lastError = 0;
  if (ChangeTimerQueueTimer(NULL, pChannel->pMultiTimerHandle, timeout, TIMER_QUEUE_PERIOD) == 0)
  {
    lastError = GetLastError();
  }
  return lastError;
#else
  return 0;
#endif
}

/* function: tmwtarg_deleteMultiTimer */
TMWTYPES_INT TMWDEFS_GLOBAL tmwtarg_deleteMultiTimer(
  TMWCHNL *pChannel)
{
#if TMWTARG_SUPPORT_MULTI_TIMER
  int lastError = 0;
  if (pChannel->pMultiTimerHandle)
  {
    /* The INVALID_HANDLE_VALUE parameter causes this function to wait for */
    /* any running timer callback functions to complete before returning.  */
    if (DeleteTimerQueueTimer(NULL, pChannel->pMultiTimerHandle, INVALID_HANDLE_VALUE) == 0)
    {
      lastError = GetLastError();
    }
  }
  return lastError;
#else
  return 0;
#endif
}
#endif

/* function: tmwtarg_exit */
void TMWDEFS_GLOBAL tmwtarg_exit(void)
{
  WinIoTarg_Exit();
}

/* function: tmwtarg_initChannel */
void * TMWDEFS_GLOBAL tmwtarg_initChannel(
  const void *pUserConfig,
  TMWTARG_CONFIG *pTmwConfig,
  TMWCHNL *pChannel)
{
/* Initialize, but not necessarily open a new communication
 * channel. The pConfig pointer is passed to the source code
 * library from the user and not manipulated or changed in any
 * way. It is solely for the use of the users implementation
 * and should be used to provide any required channel id or
 * configuration information to the target implementation.
 * The pChannelCallback function should be called if the channel
 * receives any kind of open or close notification from the operating
 * system. This callback lets the source code library know that
 * it must attempt to reopen the channel.
 * This routine returns a pointer to some kind of context that
 * will be passed to all of the remaining channel target functions.
 * The source code library does not change or manipulate this
 * pointer in any way. The pointer can not be NULL since this
 * is interpreted as a failure.
  */
  void *pCallBackParam;
  TMWCHNL_IDLE_CALLBACK pCallBackFun;
  void *pContext = WinIoTarg_Create(pUserConfig, pTmwConfig);
  if (pContext != TMWDEFS_NULL)
  {
    TMWTARGIO_CONFIG  *pIOConfig = (TMWTARGIO_CONFIG *)pUserConfig;
    WinIoTarg_getIdleCallBack(pContext, &pCallBackFun, &pCallBackParam);
    if (pCallBackFun != TMWDEFS_NULL)
    {
      tmwchnl_setIdleCallback(pChannel, pCallBackFun, pCallBackParam);
    }

    if ((pIOConfig->type == WINIO_TYPE_TCP)
      || (pIOConfig->type == WINIO_TYPE_UDP_TCP))
    {
      pChannel->polledMode = pIOConfig->targTCP.polledMode;
    }
    else if (pIOConfig->type == WINIO_TYPE_232)
    {
      pChannel->polledMode = pIOConfig->targ232.polledMode;
    }
    else
    {
      pChannel->polledMode = TMWDEFS_TRUE;
    }
  }
  return (pContext);
}

void TMWDEFS_GLOBAL tmwtarg_stopThreads(
  void *pContext)
{
  WinIoTarg_StopThreads(pContext);
}

/* function: tmwtarg_deleteChannel */
void TMWDEFS_GLOBAL tmwtarg_deleteChannel(
  void *pContext)
{
/* Delete a channel that was previously initialized, freeing
 * any memory and resources.
 */
  WinIoTarg_Destroy(pContext);
}

/* function: tmwtarg_getChannelName */
const TMWTYPES_CHAR * TMWDEFS_GLOBAL tmwtarg_getChannelName(
  void *pContext)
{
/* Return the name for this channel. Only used for diagnostics.
 */
  return (WinIoTarg_getChannelName(pContext));
}

/* function: tmwtarg_getChannelInfo */
const TMWTYPES_CHAR * TMWDEFS_GLOBAL tmwtarg_getChannelInfo(
  void *pContext)
{
/* Return user defined information string for this channel.
 * This string is only used for diagnostics.
 */
  return (WinIoTarg_getChannelInfo(pContext));
}

/* function: tmwtarg_openChannel */
TMWTYPES_BOOL TMWDEFS_GLOBAL tmwtarg_openChannel(
  void *pContext,
  TMWTARG_CHANNEL_RECEIVE_CBK_FUNC pChnlRcvCallbackFunc,
  TMWTARG_CHECK_ADDRESS_FUNC pCheckAddrCallbackFunc,
  void *pCallbackParam)
{
/* Attempt to open this channel. Return TMWDEFS_TRUE
 * if the channel is successfully opened, else TMWDEFS_FALSE.
 */
  return(WinIoTarg_openChannel(pContext, pChnlRcvCallbackFunc, pCheckAddrCallbackFunc, pCallbackParam));
}

/* function: tmwtarg_closeChannel */
void TMWDEFS_GLOBAL tmwtarg_closeChannel(
  void *pContext)
{
/* Close a previously opened channel
 */
  WinIoTarg_closeChannel(pContext);
}


#if TMWCNFG_SUPPORT_DIAG
TMWTARGP_GET_SESSION_NAME_FUNC pGetSessionNameFunc = NULL;

TMWTARGP_GET_SECTOR_NAME_FUNC pGetSectorNameFunc = NULL;

/* function: tmwtarg_getSessionName */
const TMWTYPES_CHAR * TMWDEFS_GLOBAL tmwtarg_getSessionName(
  TMWSESN *pSession)
{
  /* Return the name for this session. Only used for diagnostics.
   */
  if (pGetSessionNameFunc)
  {
    return pGetSessionNameFunc(pSession);
  }
  return (tmwchnl_getChannelName(pSession->pChannel));
}

/* function: tmwtarg_getSectorName */
const TMWTYPES_CHAR * TMWDEFS_GLOBAL tmwtarg_getSectorName(
  TMWSCTR *pSector)
{
  /* Return the name for this sector. Only used for diagnostics.
   */
  if (pGetSectorNameFunc)
  {
    return pGetSectorNameFunc(pSector);
  }
  return (tmwchnl_getChannelName(pSector->pSession->pChannel));
}
#endif

/* function: tmwtarg_isChannelOpen */
TMWTYPES_BOOL tmwtarg_isChannelOpen(void *pContext)
{
  return(WinIoTarg_isChannelOpen(pContext));
}

/* function: tmwtarg_getTransmitReady */
TMWTYPES_MILLISECONDS TMWDEFS_GLOBAL tmwtarg_getTransmitReady(
  void *pContext)
{
/* Return 0 if the channel is currently ready to transmit or the
 * number of milliseconds to wait before checking to see if the
 * channel is ready again. This feature is typically used to
 * provide a delay before starting to transmit data. An example
 * would be to provide a key time required by some radio systems.
 *
 * If a channel ready callback function was registered by the SCL in
 * tmwtarg_initChannel() TMWTARG_CONFIG *pTmwConfig the target layer
 * may choose to call the callback function to indicate the channel
 * was ready sooner than the return value suggested. If the callback
 * function is not called the SCL will retry in the number of milliseconds
 * indicated.
 *
 */
  return (WinIoTarg_getTransmitReady(pContext));
}

/* function: tmwtarg_receive */
TMWTYPES_USHORT TMWDEFS_GLOBAL tmwtarg_receive(
  void *pContext,
  TMWTYPES_UCHAR *pBuff,
  TMWTYPES_USHORT maxBytes,
  TMWTYPES_MILLISECONDS maxTimeout,
  TMWTYPES_BOOL *pInterCharTimeoutOccurred,
  TMWTYPES_MILLISECONDS *pFirstByteReceived)
{
  TMWTARG_UNUSED_PARAM(pFirstByteReceived);
/* Attempt to read the specified number of bytes from this
 * channel. This routine should read at most maxBytes bytes
 * from this channel into pBuff. Implementation of the
 * interCharacterTimeout is optional.
 *
 * NOTE: For Modbus RTU this function should not return any bytes
 *  until either the entire frame was received or an inter Character Timeout
 *  occurred.
 */
  return (WinIoTarg_receive(pContext, pBuff,
    maxBytes, maxTimeout, pInterCharTimeoutOccurred));
}

/* function: tmwtarg_transmit */
TMWTYPES_BOOL TMWDEFS_GLOBAL tmwtarg_transmit(
  void *pContext,
  TMWTYPES_UCHAR *pBuff,
  TMWTYPES_USHORT numBytes)
{
#if TMW_PRIVATE
  /* Simulate a channel failure for test purposes */
  if (tmwtargp_xmtFailureRequested(pContext))
    return TMWDEFS_TRUE;
#endif

  /* Attempt to transmit numBytes bytes on this channel. */
  return (WinIoTarg_transmit(pContext, pBuff, numBytes));
}

/* function: tmwtarg_transmitUDP */
TMWTYPES_BOOL TMWDEFS_GLOBAL tmwtarg_transmitUDP(
  void *pContext,
  TMWTYPES_UCHAR UDPPort,
  TMWTYPES_UCHAR *pBuff,
  TMWTYPES_USHORT numBytes)
{
  /* Attempt to transmit numBytes bytes on this channel using
   * UDP datagram. 
   * NOTE: This only needs to be implemented for DNP to support
   * the DNP3 Specification IP Networking. It is not required
   * for IEC or modbus and will not be called by those protocols.
   * If DNP3 UDP is not required, this function can simply return TMWDEFS_FALSE
   */

#if TMW_PRIVATE
  /* Simulate a channel failure for test purposes */
  if (tmwtargp_xmtFailureRequested(pContext))
    return TMWDEFS_TRUE;
#endif

  return (WinIoTarg_transmitUDP(pContext, UDPPort, pBuff, numBytes));
}

TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL tmwtarg_initBinFileValues(
  TMWTARG_BINFILE_VALS *pBinFileTargValues)
{
  return (WinIoTarg_initBinFileTargValues(pBinFileTargValues));
}

TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL tmwtarg_applyBinFileTargValues(
  void *pIoConfig,
  TMWTARG_BINFILE_VALS *pBinFileTargValues,
  TMWTYPES_BOOL *pIsChannelSerial)
{
  return (WinIoTarg_applyBinFileTargValues((WINIO_CONFIG *)pIoConfig, pBinFileTargValues, pIsChannelSerial));
}

