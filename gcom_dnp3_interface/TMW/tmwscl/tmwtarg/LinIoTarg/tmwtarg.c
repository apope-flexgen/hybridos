/*****************************************************************************/
/* Triangle MicroWorks, Inc.                        Copyright (c) 1997-2020  */
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
 * description: Linux implementation of target routines defined in 
 *  tmwtarg.h for use with LinIoTarg.
 * This target implementation is available for customers whose target
 * platform is a Linux platform. In some cases this
 * target implementation is also useful as a reference when porting
 * to other platforms.
 */
#include "tmwscl/utils/tmwcnfg.h"
#include "tmwscl/utils/tmwtarg.h"
#include "tmwscl/utils/tmwpltmr.h"

#include <time.h>
#include <sys/time.h>

#include "tmwtargos.h"
#include "tmwtargio.h"
#include "lintls.h"
#include "lin232.h"
#include "lintcp.h"
#include "liniodiag.h"

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
  TMWTYPES_UINT length1;
  TMWTYPES_UINT length2;

  /* Change this implementation if desired */

  length1 = 0;
  length2 = strlen(pStr2) + 1;
  if (pStr1 != TMWDEFS_NULL)
  {
    length1 = strlen(pStr1);
    length2 += length1;
  }

  result = (TMWTYPES_CHAR *)tmwtarg_alloc(length2);
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


/* function: tmwtarg_alloc */
void * TMWDEFS_GLOBAL tmwtarg_alloc(TMWTYPES_UINT numBytes)
{
#if TMWCNFG_USE_DYNAMIC_MEMORY
  return( (void *) malloc( numBytes ) );
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
  free( (char *) pBuf );
#else
  TMWTARG_UNUSED_PARAM(pBuf);
#endif
  return;
}

/* function: tmwtarg_snprintf */
TMWTYPES_INT TMWDEFS_GLOBAL tmwtarg_snprintf(
  TMWTYPES_CHAR *buf,
  TMWTYPES_UINT count,
  const TMWTYPES_CHAR *format,
  ...)
{
  va_list va;
  TMWTYPES_INT len;

  va_start(va, format);
#if TMWCNFG_HAS_VSNPRINTF
  (void)vsnprintf(buf, count, format, va);
#else
  (void)vsprintf(buf, format, va);
#endif
  va_end(va);
  len = strlen(buf); /* some sprintf's don't return the number of bytes written */
  if (len <= 0)
    return (0);
  return (len);
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

  /* Use printf for now */
  printf("%s", pString);
}
#endif /* TMWCNFG_SUPPORT_DIAG */

#if TMWCNFG_SUPPORT_THREADS
int _LinuxSemCreate(pthread_mutex_t **sem)
{
  pthread_mutexattr_t initializer;
  pthread_mutex_t *newSem;
  newSem = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
  if (newSem == NULL)
    return(0);
  pthread_mutexattr_init(&initializer);
  pthread_mutexattr_settype(&initializer, PTHREAD_MUTEX_RECURSIVE_NP);
  pthread_mutex_init(newSem, &initializer);
  pthread_mutexattr_destroy(&initializer);
  *sem = newSem;
  return(1);
}

int _LinuxSemDelete(pthread_mutex_t **sem)
{   
  pthread_mutex_destroy(*sem);
  free(*sem);
  *sem = NULL;
  return(1);
}

void tmwtarg__lockInit(TMWDEFS_RESOURCE_LOCK *pLock)
{
  pthread_mutex_t *sem = NULL;
  _LinuxSemCreate(&sem);
  *pLock = sem;
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
  pthread_mutex_lock(*((pthread_mutex_t **) pLock));
}

void tmwtarg__unlockSection(TMWDEFS_RESOURCE_LOCK *pLock)
{
  pthread_mutex_unlock(*((pthread_mutex_t **) pLock));
}

void tmwtarg__lockDelete(TMWDEFS_RESOURCE_LOCK *pLock)
{
  _LinuxSemDelete(((pthread_mutex_t **) pLock));
}
#endif /* TMWCNFG_SUPPORT_THREADS */

/* function: tmwtarg_getDateTime */
TMWTARGP_GET_DATETIME_FUNC pGetDateTimeFunc = TMWDEFS_NULL;
void TMWDEFS_GLOBAL tmwtarg_getDateTime(
  TMWDTIME *pDateTime)
{
  if (pGetDateTimeFunc)
  {
    pGetDateTimeFunc(pDateTime);
    return;
  }
  struct tm     *pTime;
  struct timeval tv;

  /* Ensure the stucture is completely initialized by setting */
  /* it to zero while preserving the session pointer          */
  void          *pSession = pDateTime->pSession;
  memset(pDateTime, 0, sizeof(TMWDTIME));
  pDateTime->pSession = pSession;

  /* Current time, is always genuine, not substituted */
  pDateTime->genuineTime = TMWDEFS_TRUE;
  /* pDateTime->invalid = TMWDEFS_FALSE; */

  /* This example gets local time. Some protocols (ie DNP3) are required to use UTC time
   * or if devices span multiple time zones it may be recommended to use UTC time.
   * In those cases this function can be modified to return UTC time. It
   * may be necessary to return different time for different sessions
   * (pDateTime->pSession) because of time syncs received or because some
   * sessions should use UTC time. 
   */
#define USE_LOCAL_TIME
  gettimeofday(&tv, NULL);

#ifdef USE_LOCAL_TIME
  pTime = localtime(&tv.tv_sec);
#else
  /* Get UTC time instead of local time */
  pTime = gmtime(&tv.tv_sec);
#endif

  pDateTime->year = pTime->tm_year + 1900;  /* tm_year is year since 1900 */
  pDateTime->month = pTime->tm_mon + 1;     /* tm_month is 0..11          */

  /* tm_wday is 0-7 Sunday-Saturday, dayOfWeek is 1-7 Monday-Sunday, 0 is not used */
  if(pTime->tm_wday == 0)
    pDateTime->dayOfWeek = 7;
  else
    pDateTime->dayOfWeek = pTime->tm_wday;

  pDateTime->dayOfMonth = pTime->tm_mday;
  pDateTime->hour = pTime->tm_hour;
  pDateTime->minutes = pTime->tm_min;
  pDateTime->dstInEffect = pTime->tm_isdst;
  pDateTime->mSecsAndSecs = (pTime->tm_sec * 1000) + (tv.tv_usec / 1000);
}

TMWTARGP_SET_DATETIME_FUNC pSetDateTimeFunc = NULL;

/* function: tmwtarg_setDateTime */
TMWTYPES_BOOL TMWDEFS_GLOBAL tmwtarg_setDateTime(
  TMWDTIME *pDateTime)
{
  struct tm timeToSet;
  time_t wrTime;
  struct timespec ts;

  if (pSetDateTimeFunc)
  {
    return(pSetDateTimeFunc(pDateTime));
  }

  timeToSet.tm_year = pDateTime->year - 1900;
  timeToSet.tm_mon = (int)pDateTime->month - 1;
  timeToSet.tm_mday = pDateTime->dayOfMonth;
  timeToSet.tm_hour = pDateTime->hour;
  timeToSet.tm_min = pDateTime->minutes;
  timeToSet.tm_sec = pDateTime->mSecsAndSecs / 1000;
  timeToSet.tm_isdst = pDateTime->dstInEffect;

  /* Initialize these to prevent warnings */
  timeToSet.tm_wday = 0;
  timeToSet.tm_yday = 0;

  wrTime = mktime(&timeToSet);
  ts.tv_sec  = wrTime;
  ts.tv_nsec = 0;

  if( clock_settime(CLOCK_REALTIME, &ts) )
    perror( "Time set failed.\n" );

  return(TMWDEFS_TRUE);
}

/* function: tmwtarg_getMSTime */
TMWTYPES_MILLISECONDS TMWDEFS_GLOBAL tmwtarg_getMSTime(void)
{
/* The returned values must use the entire 32 bit dynamic range
 * but do not require millisecond resolution(i.e. it is not
 * important that the value always increment by one).
 */
  TMWTYPES_MILLISECONDS temp;
  struct timespec curr_value;

  clock_gettime( CLOCK_MONOTONIC, &curr_value);
  temp = ( curr_value.tv_sec * 1000 ) + ( curr_value.tv_nsec / 1000000L ) ;

  return((TMWTYPES_MILLISECONDS)temp);
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
#if (TMWCNFG_SUPPORT_THREADS == TMWDEFS_FALSE)
#error TMWCNFG_SUPPORT_THREADS must have value TMWDEFS_TRUE in tmwcnfg.h to support TMWCNFG_MULTIPLE_TIMER_QS.
#endif 
/* function: tmwtarg_initMultiTimer */
TMWTYPES_INT TMWDEFS_GLOBAL tmwtarg_initMultiTimer(
  TMWCHNL *pChannel)
{
  /* The Linux MultiTimer implementation uses the channel thread to */
  /* call _MultiTimerCallback every 50 ms. The channel thread will  */
  /* be started on channel initialization, so there is no           */
  /* initialization required here.                                  */
  TMWTARG_UNUSED_PARAM(pChannel);
  return 0;
}

/* function: tmwtarg_setMultiTimer */
TMWTYPES_INT TMWDEFS_GLOBAL tmwtarg_setMultiTimer(
  TMWCHNL                *pChannel,
  TMWTYPES_MILLISECONDS   timeout) 
{
  /* The channel thread is calling the timout function every 50 ms */
  /* so there no work to be performed in the Linux target layer.   */
  TMWTARG_UNUSED_PARAM(pChannel);
  TMWTARG_UNUSED_PARAM(timeout);
  return 0;
}

/* function: tmwtarg_deleteMultiTimer */
TMWTYPES_INT TMWDEFS_GLOBAL tmwtarg_deleteMultiTimer(
  TMWCHNL *pChannel)
{
  TMWTARG_UNUSED_PARAM(pChannel);
  return 0;
}
#endif

void TMWDEFS_GLOBAL tmwtarg_stopThreads(
  void *pContext)
{
  /* Shutdown the channel's thread */
  TMWTARG_IO_CHANNEL *pTargIoChannel = (TMWTARG_IO_CHANNEL *)pContext;

  if (pTargIoChannel->chanThreadState == TMWTARG_THREAD_RUNNING)
  {
    pTargIoChannel->chanThreadState = TMWTARG_THREAD_EXITING;

    while (pTargIoChannel->chanThreadState > TMWTARG_THREAD_EXITED)
    {
      tmwtarg_sleep(10);
    }
  }

  if (pTargIoChannel->chanState == TMWTARG_CHANNEL_OPENED)
  {
    tmwtarg_closeChannel(pContext);
  }

}

/* function: tmwtarg_exit */
void TMWDEFS_GLOBAL tmwtarg_exit(void)
{
#if TMWTARG_SUPPORT_TCP
  linTCP_exit();
#endif

#if TMWTARG_SUPPORT_TLS
  /* Clean up OpenSSL to reduce memory leak diagnostics. */
 lintls_terminateSslLibrary();
#endif

}

#if TMWCNFG_SUPPORT_THREADS
/* function: _channelThread */
static TMW_ThreadDecl _channelThread(TMW_ThreadArg pVoidArg)
{
  TMWTARG_IO_CHANNEL   *pTargIoChannel = (TMWTARG_IO_CHANNEL *)pVoidArg;
  TMWTYPES_MILLISECONDS timeoutValue;
#if TMWCNFG_MULTIPLE_TIMER_QS
  TMWCHNL              *pChannel = pTargIoChannel->pChannel;
#endif

  while (pTargIoChannel->chanThreadState == TMWTARG_THREAD_RUNNING)
  {
    /* Wait until the channel is operational */
    if (pTargIoChannel->chanState == TMWTARG_CHANNEL_INITIALIZED)
    {
      tmwtarg_sleep(1000);
      continue;
    }

    /* Once online, handle channel timeouts */
    timeoutValue = 100;
#if TMWCNFG_MULTIPLE_TIMER_QS
    if ((pChannel) && (pChannel->timerQueue.timerRunning == TMWDEFS_TRUE))
    {
      pChannel->pMultiTimerCallback(pChannel);
    }
#endif

    /* Call the channel's check input function */
    if ((pTargIoChannel->chanState == TMWTARG_CHANNEL_OPENED) && (pTargIoChannel->pCheckInputFunction))
    {
      pTargIoChannel->pCheckInputFunction(pTargIoChannel, timeoutValue);
      continue;
    }
    tmwtarg_sleep(500);
  }
  pTargIoChannel->chanThreadState = TMWTARG_THREAD_EXITED;

  pthread_detach(pTargIoChannel->chanThreadHandle);

  return((TMW_ThreadPtr) NULL);
}
#endif

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
  TMWTARGIO_CONFIG  *pIOConfig = (TMWTARGIO_CONFIG *)pUserConfig;
  TMWTARG_IO_CHANNEL*pTargIoChannel;

  pTargIoChannel = (TMWTARG_IO_CHANNEL *)malloc(sizeof(TMWTARG_IO_CHANNEL));
  if (pTargIoChannel == NULL)
    return TMWDEFS_NULL;

  memset(pTargIoChannel, 0, sizeof(TMWTARG_IO_CHANNEL));
  pTargIoChannel->type = pIOConfig->type;
  pTargIoChannel->pChannel = pChannel;
  switch (pIOConfig->type)
  {
#if TMWTARG_SUPPORT_232
  case TMWTARGIO_TYPE_232:
    pTargIoChannel->pChannelInfo = (void *)lin232_initChannel(pIOConfig, pTmwConfig, pTargIoChannel);
    break;
#endif

#if TMWTARG_SUPPORT_TCP
  case TMWTARGIO_TYPE_TCP:
  case TMWTARGIO_TYPE_UDP_TCP:
    pTargIoChannel->pChannelInfo = (void *)linTCP_initChannel(pIOConfig, pTmwConfig, pTargIoChannel);
    break;
#endif
  default:
    break;
  }

  if (pTargIoChannel->pChannelInfo == TMWDEFS_NULL)
  {
    free(pTargIoChannel);
    return TMWDEFS_NULL;
  }
  pChannel->polledMode = pTargIoChannel->polledMode;

  /* Start a thread for this channel if its not running in polledMode or if needed
   * to process this channel's timer queue.
   */
  pTargIoChannel->chanState = TMWTARG_CHANNEL_INITIALIZED;
#if TMWCNFG_SUPPORT_THREADS
  if ((pTargIoChannel->polledMode == TMWDEFS_FALSE) || (TMWCNFG_MULTIPLE_TIMER_QS))
  {
    pTargIoChannel->chanThreadState = TMWTARG_THREAD_RUNNING;
    TMW_ThreadCreate(&pTargIoChannel->chanThreadHandle, _channelThread,
                  (TMW_ThreadArg)pTargIoChannel, 0, 0);
  }
#else
  if (pTargIoChannel->polledMode == TMWDEFS_FALSE)
  {
    LINIODIAG_ERRORMSG("tmwtarg_initChannel: TMWCNFG_SUPPORT_THREADS must have value TMWDEFS_TRUE in tmwcnfg.h to support non-polledMode");
    if (pTargIoChannel->pDeleteFunction)
    {
      pTargIoChannel->pDeleteFunction(pTargIoChannel);
    }
    free(pTargIoChannel);
    return TMWDEFS_NULL;
  }
#endif

  return(pTargIoChannel);
}

/* function: tmwtarg_deleteChannel */
void TMWDEFS_GLOBAL tmwtarg_deleteChannel(
  void *pContext)
{
/* Delete a channel that was previously initialized, freeing
 * any memory and resources.
 */
  TMWTARG_IO_CHANNEL *pTargIoChannel = (TMWTARG_IO_CHANNEL *)pContext;
  if (pTargIoChannel->pDeleteFunction)
  {
    pTargIoChannel->pDeleteFunction(pTargIoChannel);
  }

  free(pTargIoChannel);
  return;
}

/* function: tmwtarg_getChannelName */
const TMWTYPES_CHAR * TMWDEFS_GLOBAL tmwtarg_getChannelName(
  void *pContext)
{
/* Return the name for this channel. Only used for diagnostics.
 */
  TMWTARG_IO_CHANNEL *pTargIoChannel = (TMWTARG_IO_CHANNEL *)pContext;
  if (pTargIoChannel->pChannelName)
  {
    return (pTargIoChannel->pChannelName);
  }
  return("");
}

/* function: tmwtarg_getChannelInfo */
const TMWTYPES_CHAR * TMWDEFS_GLOBAL tmwtarg_getChannelInfo(
  void *pContext)
{
/* Return user defined information string for this channel.
 * This string is only used for diagnostics.
 */
  TMWTARG_IO_CHANNEL *pTargIoChannel = (TMWTARG_IO_CHANNEL *)pContext;
  return (pTargIoChannel->chanInfoBuf);
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
  TMWTARG_IO_CHANNEL *pTargIoChannel = (TMWTARG_IO_CHANNEL *)pContext;

  pTargIoChannel->chanState = TMWTARG_CHANNEL_OPENED;
  if (pTargIoChannel->pOpenFunction)
  {
    pTargIoChannel->pReceiveCallbackFunc = pChnlRcvCallbackFunc;
    pTargIoChannel->pCheckAddrCallbackFunc = pCheckAddrCallbackFunc;
    pTargIoChannel->pCallbackParam = pCallbackParam;
    return (pTargIoChannel->pOpenFunction(pTargIoChannel));
  }

  return(TMWDEFS_FALSE);
}

/* function: tmwtarg_closeChannel */
void TMWDEFS_GLOBAL tmwtarg_closeChannel(
  void *pContext)
{
/* Close a previously opened channel
 */
  TMWTARG_IO_CHANNEL *pTargIoChannel = (TMWTARG_IO_CHANNEL *)pContext;

  pTargIoChannel->chanState = TMWTARG_CHANNEL_CLOSED;
  if (pTargIoChannel->pCloseFunction)
  {
    pTargIoChannel->pCloseFunction(pTargIoChannel);
  }
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
  TMWTARG_UNUSED_PARAM(pContext);
  /* Support this properly if it is called from the application code.
   * For example, to check if a DNP Dual End Point connection is really open
   */
  return TMWDEFS_TRUE;
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
  TMWTARG_IO_CHANNEL *pTargIoChannel = (TMWTARG_IO_CHANNEL *)pContext;
  if (pTargIoChannel->pXmitReadyFunction)
  {
    return (pTargIoChannel->pXmitReadyFunction(pTargIoChannel));
  }
  return(500);
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
  TMWTARG_IO_CHANNEL *pTargIoChannel = (TMWTARG_IO_CHANNEL *)pContext;
  if (pTargIoChannel->pRecvFunction)
  {
    return (pTargIoChannel->pRecvFunction(pTargIoChannel, pBuff, maxBytes, maxTimeout, pInterCharTimeoutOccurred));
  }
  return(0);
}

/* function: tmwtarg_transmit */
TMWTYPES_BOOL TMWDEFS_GLOBAL tmwtarg_transmit(
  void *pContext,
  TMWTYPES_UCHAR *pBuff,
  TMWTYPES_USHORT numBytes)
{
 /* Attempt to transmit numBytes bytes on this channel.
  */
  TMWTARG_IO_CHANNEL *pTargIoChannel = (TMWTARG_IO_CHANNEL *)pContext;
  if (pTargIoChannel->pXmitFunction)
  {
    return (pTargIoChannel->pXmitFunction(pTargIoChannel, pBuff, numBytes));
  }
  return(TMWDEFS_FALSE);
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
  TMWTARG_IO_CHANNEL *pTargIoChannel = (TMWTARG_IO_CHANNEL *)pContext;
  if (pTargIoChannel->pXmitUdpFunction)
  {
    return (pTargIoChannel->pXmitUdpFunction(pTargIoChannel, UDPPort, pBuff, numBytes));
  }
  return(TMWDEFS_FALSE);
}

/* The following two functions are only required for if
 * DNPCNFG_SUPPORT_BINCONFIG is defined as TMWDEFS_TRUE
 */

TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL tmwtarg_initBinFileValues(
  TMWTARG_BINFILE_VALS *pBinFileTargValues)
{
  TMWTARG_UNUSED_PARAM(pBinFileTargValues);
  /* return (liniotarg_initBinFileTargValues(pBinFileTargValues)); */
  return TMWDEFS_FALSE;
}

TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL tmwtarg_applyBinFileTargValues(
  void *pIoConfig,
  TMWTARG_BINFILE_VALS *pBinFileTargValues,
  TMWTYPES_BOOL *pIsChannelSerial)
{
  TMWTARG_UNUSED_PARAM(pIoConfig);
  TMWTARG_UNUSED_PARAM(pBinFileTargValues);
  TMWTARG_UNUSED_PARAM(pIsChannelSerial);
  /* return (liniotarg_applyBinFileTargValues(pIoConfig, pBinFileTargValues)); */
  return TMWDEFS_FALSE;
}

/* function: tmwtarg_sleep
 * purpose: suspends execution of the calling thread for specified number of milliseconds
 * arguments:
 *  milliseconds - specifies number of milliseconds to suspend the calling thread
 * returns
 *  void
 */
TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwtarg_sleep(
  TMWTYPES_MILLISECONDS milliseconds)
{
  usleep(milliseconds * 1000);
}
