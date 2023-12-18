/*****************************************************************************/
/* Triangle MicroWorks, Inc.                        Copyright (c) 1997-2019  */
/*****************************************************************************/
/*                                                                           */
/* This file is the property of:                                             */
/*                                                                           */
/*                       Triangle MicroWorks, Inc.                           */
/*                      Raleigh, North Carolina USA                          */
/*                       www.TriangleMicroWorks.com                          */
/*                          (919) 870 - 6615                                 */
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
 * description: implementation of target routines defined in tmwtarg.h
 * SCL Users should implement the following functions which are called by the SCL.
 */
#include "tmwscl/utils/tmwcnfg.h"
#include "tmwscl/utils/tmwtarg.h"
#include "tmwscl/utils/tmwpltmr.h"

#include "tmwtargos.h"
#include "tmwtargio.h"
#include "sample232.h"
#include "sampletcp.h"
#include "sampleiodiag.h"

/* These are sample implementations of the functions that must be implemented
 * in the target layer to support the SCL. When porting the SCL to an OS
 * other than Windows or Linux, this OS indepenent target layer template
 * can be used as an initial implementation.
 */

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
  pDest[1] = (TMWTYPES_UCHAR)(*pSource>>8);
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
  pDest[1] = (TMWTYPES_UCHAR)(*pSource>>8);
  pDest[2] = (TMWTYPES_UCHAR)(*pSource>>16);
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
  pDest[1] = (TMWTYPES_UCHAR)(*pSource>>8);
  pDest[2] = (TMWTYPES_UCHAR)(*pSource>>16);
  pDest[3] = (TMWTYPES_UCHAR)(*pSource>>24);
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
  if(pStr1 != TMWDEFS_NULL)
  {
    length1 = strlen(pStr1);
    length2 += length1;
  }

  result = (TMWTYPES_CHAR *)tmwtarg_alloc((TMWTYPES_UINT)length2);
  if(result != TMWDEFS_NULL)
  {
    result[0] = '\0';
    if(pStr1 != TMWDEFS_NULL)
    {
      STRCPY(result, length2, pStr1);
      tmwtarg_free(pStr1);
    }
    STRCPY(&result[length1], length2-length1, pStr2);
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

  pConfig->interCharTimeout  = 0;
  pConfig->numCharTimesBetweenFrames = 4; /* this is only used for modbus rtu */

  /* Channel callback function */
  pConfig->pChannelCallback = TMWDEFS_NULL;
  pConfig->pCallbackParam = TMWDEFS_NULL;

  /* Channel ready callback function */
  pConfig->pChannelReadyCallback = TMWDEFS_NULL;
  pConfig->pChannelReadyCbkParam = TMWDEFS_NULL;
  pConfig->connectRetry = TMWDEFS_SECONDS(1);
}

/* function: tmwtarg_alloc
 * purpose:  Allocate memory. This function will only be called if
 * TMWCNFG_USE_DYNAMIC_MEMORY is TMWDEFS_TRUE.
 * arguments:
 *  numBytes - number of bytes requested
 * returns:
 *  pointer to allocated memory if successful
 *  TMWDEFS_NULL if unsuccessful
 */
void * TMWDEFS_GLOBAL tmwtarg_alloc(TMWTYPES_UINT numBytes)
{
#if TMWCNFG_USE_DYNAMIC_MEMORY
  return( (void *) malloc( numBytes ) );
#else
  TMWTARG_UNUSED_PARAM(numBytes);
  return (TMWDEFS_NULL);
#endif
}

/* function: tmwtarg_free
 * purpose:  Free memory allocated by tmwtarg_alloc()
 * arguments:
 *  pBuf - pointer to buffer to be freed
 * returns:
 *  void
 */
void TMWDEFS_GLOBAL tmwtarg_free(void *pBuf)
{
#if TMWCNFG_USE_DYNAMIC_MEMORY
  free( (char *) pBuf );
#else
  TMWTARG_UNUSED_PARAM(pBuf);
#endif
  return;
}

/* function: tmwtarg_snprintf
 * purpose: Write formatted data to a string.
 * arguments:
 *  buf - Storage location for output
 *  count - Maximum number of characters that can be stored in buf
 *  format - Format-control string
 *  ... - Optional arguments
 * returns:
 *  TMWTYPES_INT - returns the number of bytes stored in buffer,
 *  not counting the terminating null character
 */
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

/* function: tmwtarg_putDiagString
 * purpose: Display a string of characters. This routine is used
 *  to display diagnostic information from the source code library
 *  if desired.
 * arguments:
 *  pAnlzId - pointer to structure containing information about where
 *   and why this message originated.
 *  pString - pointer to null terminated character string to display
 * returns:
 *  void
 */
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
/* function: tmwtarg__lockInit
 * purpose: Initalize a mutex. This routine is used to initialize locks
 *  used by the source code library to protect critical regions when
 *  the library is configured to run with thread support. The library
 *  expects the lock to be initialized recursivelly such that the locking
 *  thread can lock the same mutex twice.
 * arguments:
 *  pLock - pointer to the lock's pointer
 * returns:
 *  void
 */
void tmwtarg__lockInit(TMWDEFS_RESOURCE_LOCK *pLock)
{
  /* Put your code here */
  TMWTARG_UNUSED_PARAM(pLock);
}

/* This function is only required for 104 redundancy with a multi-threaded
 * architecture. It will allow the use of a single lock for the redundancy
 * group as well as the redundant connection channels.
 */
void tmwtarg__lockShare(TMWDEFS_RESOURCE_LOCK *pLock, TMWDEFS_RESOURCE_LOCK *pLock1)
{
  /* copy the information for the *pLock into *pLock1 so that the same lock is
   * used for both structures. This cannot be the lock itself. It would typically
   * be a pointer or an index or a reference to a lock.
   */
  /* Put your code here */
  TMWTARG_UNUSED_PARAM(pLock);
  TMWTARG_UNUSED_PARAM(pLock1);
}

/* function: tmwtarg__lockSection
 * purpose: Lock a mutex. This routine is used by the source code 
 *  library to lock a critical region to ensure exclusive access.
 * arguments:
 *  pLock - pointer to the lock's pointer
 * returns:
 *  void
 */
void tmwtarg__lockSection(TMWDEFS_RESOURCE_LOCK *pLock)
{
  /* Put your code here */
  TMWTARG_UNUSED_PARAM(pLock);
}

/* function: tmwtarg__unlockSection
 * purpose: Unlock a mutex. This routine is used by the source code 
 *  library to unlock a critical region.
 * arguments:
 *  pLock - pointer to the lock's pointer
 * returns:
 *  void
 */
void tmwtarg__unlockSection(TMWDEFS_RESOURCE_LOCK *pLock)
{
  /* Put your code here */
  TMWTARG_UNUSED_PARAM(pLock);
}

/* function: tmwtarg__unlockDelete
 * purpose: Delete a mutex. This routine is used by the source code 
 *  library to free all resources allocated by tmwtarg__lockInit.
 * arguments:
 *  pLock - pointer to the lock's pointer
 * returns:
 *  void
 */
void tmwtarg__lockDelete(TMWDEFS_RESOURCE_LOCK *pLock)
{
  /* Put your code here */
  TMWTARG_UNUSED_PARAM(pLock);
}
#endif /* TMWCNFG_SUPPORT_THREADS */

/* function: tmwtarg_getDateTime
 * purpose: Return the current date and time.
 * arguments:
 *  pDateTime - structure into which to store the current date and time
 * returns:
 *  void
 */
void TMWDEFS_GLOBAL tmwtarg_getDateTime(
  TMWDTIME *pDateTime)
{
  void  *pSession = pDateTime->pSession;

  /* Ensure the stucture is completely initialized by setting */
  /* it to zero while preserving the session pointer          */ 
  memset(pDateTime, 0, sizeof(TMWDTIME));
  pDateTime->pSession = pSession;

  /* Current time, is always genuine, not substituted */
  pDateTime->genuineTime = TMWDEFS_TRUE;

  /* Get the current system date and time and store into a TMWDTIME
   * structure. The TMWDTIME structure is defined in tmwdtime.h.
   */

  /* Some protocols (ie DNP3) are required to use UTC time or if
   * devices span multiple time zones it may be recommended to use UTC time.
   * In those cases this function can be modified to return UTC time. It
   * may be necessary to return different time for different sessions
   * (pDateTime->pSession) because of time syncs received or because some
   * sessions are using UTC time. 
   */

  /* Put your code here */
}

/* function: tmwtarg_setDateTime
 * purpose: Set the current date and time. This function will only be
 *  called from a slave session as a result of a clock synchronization
 *  request.
 * arguments:
 *  pDateTime - pointer to structure containing new time
 * returns:
 *  TMWTYPES_BOOL - true if success
 */
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
  /* Put your code here */
  TMWTARG_UNUSED_PARAM(pDateTime);
  return(TMWDEFS_FALSE);
}

/* function: tmwtarg_getMSTime
 * purpose: Return the current value of a continuously running
 *  millisecond timer.
 * arguments:
 *  none
 * returns:
 *  Current value of millisecond clock.
 */
TMWTYPES_MILLISECONDS TMWDEFS_GLOBAL tmwtarg_getMSTime(void)
{
  /* The returned values must use the entire 32 bit dynamic range
   * but do not require millisecond resolution(i.e. it is not
   * important that the value always increment by one).
   */
  /* Put your code here */
  return (0);
}

#if !TMWCNFG_MULTIPLE_TIMER_QS
/* function: tmwtarg_startTimer()
 * purpose: Start a timer that will call the specified  callback function
 *  in 'timeout' milliseconds. Only a single event timer is required by the
 *  source code library.
 * arguments:
 *  timeout - number of milliseconds to wait
 *    This value can be zero. In that case the timer should call the callback
 *    function as soon as possible.
 *    NOTE: If this value is too large for the timer implementation, a timer
 *    with the largest supported value should be started. When the callback
 *    is called for the lesser value, the SCL will start another timer with
 *    the remaining time.
 *  pCallbackFunc - function to call when timer expires
 *  pCallbackParam - user specified callback parameter
 * returns:
 *  void
 *   NOTE: since it is not possible for this function to return failure it is
 *    important that a timer is started. If the timer cannot be started you
 *    should log this in some way or generate an exception since the SCL timers
 *    may not function after this failure. Calling the callback function sooner
 *    than asked for will cause the SCL to call this function again with the
 *    remaining time.
 */
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
   * default implementation uses the polled timer.
   */
  /* Replace this call with your code if not using polled timer implementation. */
  tmwpltmr_startTimer(timeout, pCallbackFunc, pCallbackParam);
}
/* function: tmwtarg_cancelTimer
 * purpose: Cancel current timer
 * arguments:
 *  none
 * returns:
 *  void
 */
void TMWDEFS_GLOBAL tmwtarg_cancelTimer(void)
{
  /* Cancel the current event timer, no further timer callbacks
   * will be invoked. The default implementation uses the polled
   * timer.
   */
  /* Replace this call with your code if not using polled timer implementation. */
  tmwpltmr_cancelTimer();
}
#else /* TMWCNFG_MULTIPLE_TIMER_QS */
/* function: tmwtarg_initMultiTimer
 * purpose: Initialize the timer for this channel. This routine should
 *  initialize the timer for this channel.
 * arguments:
 *  pChannel - pointer to channel
 * returns: 0 if successful.
 */  
TMWTYPES_INT TMWDEFS_GLOBAL tmwtarg_initMultiTimer(
  TMWCHNL *pChannel)
{
  /* Put your code here */
  TMWTARG_UNUSED_PARAM(pChannel);
  return 0;
}

/* function: tmwtarg_setMultiTimer()
 * purpose: Set the timeout period for this channel. 
 * arguments:
 *  pChannel - pointer to channel to start the MultiTimer.
 *  timeout - number of milliseconds to wait.
 *    A value of zero will cancel the timer.
 *    NOTE: If this value is too large for the timer implementation, a timer 
 *    with the largest supported value should be started. When the callback is 
 *    called for the lesser value, the SCL will start another timer with the  
 *    remaining time.
 * returns:
 *  TMWTYPES_INT - 0 if successful, error code on failure.
 *   NOTE: If the timer cannot be started you should log this in some way or 
 *    generate an exception since the SCL timers may not function after this 
 *    failure. Calling the callback function sooner than asked for will cause 
 *    the SCL to call this function again with the remaining time.
 */
TMWTYPES_INT TMWDEFS_GLOBAL tmwtarg_setMultiTimer(
  TMWCHNL                *pChannel,
  TMWTYPES_MILLISECONDS   timeout)
{
  /* Put your code here */
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(timeout);
  return(TMWDEFS_NULL);
}

/* function: tmwtarg_deleteMultiTimer
 * purpose: Delete the MultiTimer
 * arguments:
 *  pChannel - pointer to channel
 * returns: 0 if successful.
 */
void TMWDEFS_GLOBAL tmwtarg_deleteMultiTimer(
  TMWCHNL *pChannel)
{
  /* Put your code here */
  TMWTARG_UNUSED_PARAM(pChannel);
}
#endif

/* function: tmwtarg_stopThreads */
void TMWDEFS_GLOBAL tmwtarg_stopThreads(
  void *pContext)
{
  /* Put your code here if required */
  TMWTARG_UNUSED_PARAM(pContext);
}

/* function: tmwtarg_exit */
void TMWDEFS_GLOBAL tmwtarg_exit()
{
  /* Put your code here if required */
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
    /* Wait until the channel is opened */
    if (pTargIoChannel->chanState < TMWTARG_CHANNEL_OPENED)
    {
      tmwtarg_sleep(1000);
      continue;
    }

    /* Once online, handle channel timeouts */
    timeoutValue = 50;
#if TMWCNFG_MULTIPLE_TIMER_QS
    if ((pChannel) && (pChannel->timerQueue.timerRunning == TMWDEFS_TRUE))
    {
      TMWTIMER *pTimer;

      pChannel->pMultiTimerCallback(pChannel);

      pTimer = (TMWTIMER *)tmwdlist_getEntry(&pChannel->timerQueue.list, 0U);
      if (pTimer != TMWDEFS_NULL)
      {
        timeoutValue = pTimer->timeout - tmwtarg_getMSTime();
      }
    }
#endif

    /* Call the channel's check input function */
    if (pTargIoChannel->pCheckInputFunction)
    {
      pTargIoChannel->pCheckInputFunction(pTargIoChannel, timeoutValue);
    }
  }
  pTargIoChannel->chanThreadState = TMWTARG_THREAD_EXITED;

  return((TMW_ThreadPtr) NULL);
}
#endif

/* function: tmwtarg_initChannel
 * purpose: Initialize a communications channel. This routine creates
 *  a communications channel as specified in the pConfig argument. The
 *  channel does not need to be opened as this will be accomplished in
 *  the tmwtarg_openChannel function described below. This routine
 *  returns a user defined context which is passed to all successive
 *  calls for this channel. The contents of the context are not used
 *  by the TMW SCL and are defined as required by the target interface.
 * arguments:
 *  pUserConfig - Pointer to configuration data passed to the TMW
 *   physical layer code. This data is not used by the TMW code
 *   and should be used by the target routines to identify and
 *   configure the communications channel.
 *  pTmwConfig - TMW target configuration data structure
 *  pChannel - pointer to channel

 * returns:
 *  void * channel context
 *   The channel context is a target-defined context that
 *   will be passed to all of the remaining channel target functions.
 *   The source code library does not change or manipulate this
 *   pointer in any way. The pointer cannot be NULL since this
 *   is interpreted as a failure.
 */
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
    pTargIoChannel->pChannelInfo = (void *)sample232_initChannel(pIOConfig, pTmwConfig, pTargIoChannel);
    break;
#endif

#if TMWTARG_SUPPORT_TCP
  case TMWTARGIO_TYPE_TCP:
    pTargIoChannel->pChannelInfo = (void *)sampleTCP_initChannel(pIOConfig, pTmwConfig, pTargIoChannel);
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

  pTargIoChannel->chanState = TMWTARG_CHANNEL_INITIALIZED;
  return(pTargIoChannel);
}

/* function: tmwtarg_deleteChannel
 * purpose: Delete a communications channel. This routine should
 *  delete a communications channel and free all associated memory
 *  and resources.
 * arguments:
 *  pContext - Context returned from call to tmwtarg_initChannel
 * returns:
 *  void
 */
void TMWDEFS_GLOBAL tmwtarg_deleteChannel(
  void *pContext)
{
/* Delete a channel that was previously initialized, freeing
 * any memory and resources.
 */
  TMWTARG_IO_CHANNEL *pTargIoChannel = (TMWTARG_IO_CHANNEL *)pContext;

  /* Shutdown the channel's thread */
  if (pTargIoChannel->chanThreadState == TMWTARG_THREAD_RUNNING)
  {
    int waitCounter = 0;
    pTargIoChannel->chanThreadState = TMWTARG_THREAD_EXITING;;
 
    while (pTargIoChannel->chanThreadState > TMWTARG_THREAD_EXITED)
    {
      tmwtarg_sleep(500);

      /* Ensure this is not an infinite loop. */
      waitCounter++;
      if (waitCounter > 100)
      {
        SAMPLEIODIAG_ERRORMSG("tmwtarg_deleteChannel: channel thread running after count of %d", waitCounter);
        break;
      }
    }
  }

  if (pTargIoChannel->pDeleteFunction)
  {
    pTargIoChannel->pDeleteFunction(pTargIoChannel);
  }
  free(pTargIoChannel);
  return;
}

/* function: tmwtarg_getChannelName
 * purpose: Returns the name for this channel
 *  For Diagnostic Purposes only.
 * description: This method allows the target to return an appropriate
 *  name for this channel. Typically this would be something out of the
 *  configuration information passed to the tmwtarg_initChannel routine.
 * arguments:
 *  pContext - Channel context returned from call to tmwtarg_initChannel
 * returns: pointer to a null terminated string which contains the
 *  name.
 */
const TMWTYPES_CHAR * TMWDEFS_GLOBAL tmwtarg_getChannelName(
  void *pContext)
{
/* Return the name for this channel. Only used for diagnostics.
 */
  TMWTARG_IO_CHANNEL *pTargIoChannel = (TMWTARG_IO_CHANNEL *)pContext;
  if ((pTargIoChannel) && (pTargIoChannel->pChannelName))
  {
    return (pTargIoChannel->pChannelName);
  }
  return ("Unknown");
}

/* function: tmwtarg_getChannelInfo
 * purpose: Return configuration information for this channel
 * description: This method allows the target to return a user defined
 *  information string to be displayed when the channel is opened.
 *  typically this would contain formatted information about the
 *  channel configuration and/or status.
 * arguments:
 *  pContext - Context returned from call to tmwtarg_initChannel
 * returns:
 *  Pointer to a null terminated string which contains the name.
 */
const TMWTYPES_CHAR * TMWDEFS_GLOBAL tmwtarg_getChannelInfo(
  void *pContext)
{
/* Return user defined information string for this channel.
 * This string is only used for diagnostics.
 */
  TMWTARG_IO_CHANNEL *pTargIoChannel = (TMWTARG_IO_CHANNEL *)pContext;
  if (pTargIoChannel)
  {
    return (pTargIoChannel->chanInfoBuf);
  }
  return ("Unknown");
}

/* function: tmwtarg_openChannel
 * purpose: Open a communications channel.
 *  If this was over TCP/IP this function would attempt to listen
 *  or make the connection.
 * arguments:
 *  pContext - Context returned from call to tmwtarg_initChannel
 *  pReceiveCallbackFunc - Function to be called when data is available
 *   to be read if using event driven rather than polled mode. Most
 *   implementations will not need to call this function.
 *  pCheckAddrCallbackFunc - Function to be called to determine if this
 *   received data is intended for this channel. This is only supported for
 *   DNP and 101/103. It is intended to provide support for modem pool
 *   implementations for incoming unsolicited messages. Most implementations
 *   will not need to call this function.
 *  pCallbackParam - parameter to be passed to both the pReceivedCallbackFunc
 *   and pCheckAddrCallbackFunc.
 * returns:
 *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
 */
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

  if ((pTargIoChannel) && (pTargIoChannel->pOpenFunction))
  {
    pTargIoChannel->pReceiveCallbackFunc = pChnlRcvCallbackFunc;
    pTargIoChannel->pCheckAddrCallbackFunc = pCheckAddrCallbackFunc;
    pTargIoChannel->pCallbackParam = pCallbackParam;
    pTargIoChannel->chanState = TMWTARG_CHANNEL_OPENED;
    return (pTargIoChannel->pOpenFunction(pTargIoChannel));
  }

  return (TMWDEFS_FALSE);
}

/* function: tmwtarg_closeChannel
 * purpose: Close a communications channel
 * arguments:
 *  pContext - Context returned from call to tmwtarg_initChannel
 * returns:
 *  void
 */
void TMWDEFS_GLOBAL tmwtarg_closeChannel(
  void *pContext)
{
  /* Close a previously opened channel
   */
  TMWTARG_IO_CHANNEL *pTargIoChannel = (TMWTARG_IO_CHANNEL *)pContext;
  pTargIoChannel->chanState = TMWTARG_CHANNEL_CLOSED;

  if ((pTargIoChannel) && (pTargIoChannel->pCloseFunction))
  {
    pTargIoChannel->pCloseFunction(pTargIoChannel);
  }
}

#if TMWCNFG_SUPPORT_DIAG
TMWTARGP_GET_SESSION_NAME_FUNC pGetSessionNameFunc = NULL;

TMWTARGP_GET_SECTOR_NAME_FUNC pGetSectorNameFunc = NULL;

/* function: tmwtarg_getSessionName
 * purpose: Returns the name for this session
 *  For Diagnostic Purposes only.  Registration function also provided.
 * description: This method allows the target to return an appropriate
 *  name for this session. This function could just return the name for the
 *  pSession->pChannel if names are not maintained per session.
 * arguments:
 *  pSession - pointer to session returned by xxxsesn_openSession()
 * returns: pointer to a null terminated string which contains the
 *  name.
 */
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

/* function: tmwtarg_getSectorName
 * purpose: Returns the name for this sector
 *  For Diagnostic Purposes only.  Registration function also provided.
 * description: This method allows the target to return an appropriate
 *  name for this sector. This function could just return the name for the
 *  pSector->pSession or pSector->pChannel if names are not maintained per sector.
 * arguments:
 *  pSession - pointer to session returned by xxxsesn_openSector()
 * returns: pointer to a null terminated string which contains the
 *  name.
 */
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
  /* Support this properly if it is called from the application code.
   * For example, to check if a DNP Dual End Point connection is really open
   */
  return TMWDEFS_TRUE;
}

/* function: tmwtarg_getTransmitReady
 * purpose: Determine whether a channel is ready to transmit or not.
 *  This routine can be used to delay transmission until various
 *  target related dependencies have been satisfied. A common
 *  example is modem setup time.
 * arguments:
 *  pContext - Context returned from call to tmwtarg_initChannel
 * returns:
 *  0 if channel is ready to transmit,
 *  non-zero, if channel is not OK to transmit. This value will indicate
 *  the number of milliseconds the SCL should wait before calling this
 *  function again for this channel. If the SCL has registered a
 *  TMWTARG_CHANNEL_READY_CBK_FUNC callback function the target layer may
 *  call this callback function if the channel is ready sooner than
 *  this return value would indicate. If the callback function is not
 *  called the SCL will retry this channel in the number of milliseconds
 *  returned by this function.
 */
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
  if ((pTargIoChannel) && (pTargIoChannel->pXmitReadyFunction))
  {
    return (pTargIoChannel->pXmitReadyFunction(pTargIoChannel));
  }
  return (0);
}

/* function: tmwtarg_receive
 * purpose: Receive bytes from the specified channel
 * arguments:
 *  pContext - Context returned from call to tmwtarg_initChannel
 *  pBuff - Buffer into which to store received bytes
 *  maxBytes - The maximum number of bytes to read
 *  maxTimeout - maximum time to wait in milliseconds for input
 *   from this channel.
 *  pInterCharTimeoutOccurred - TMWDEFS_TRUE if an intercharacter
 *   timeout occured while receiving bytes. This is an optional
 *   timeout that can be implemented in the target to terminate
 *   a frame if too much time passes between receipt of bytes
 *   in a frame.
 * returns:
 *  The number of bytes actually read.
 * NOTES:
 *  - The Source Code Library will usually use a timeout value of 0;
 *    This indicates the call to tmwtarg_receive should be nonblocking
 *    (i.e., return 0 if no bytes are available.)
 *  - For Modbus RTU this function should not return any bytes
 *    until either the entire frame was received or an inter Character Timeout
 *    occurred. If you are implementing multiple protocols, one of which is
 *    Modbus RTU, then the pContext structure should include a flag that
 *    indicates whether full frames are required. The target implementation
 *    of tmwtarg_receive can use this indicator to ensure that it returns
 *    the entire frame for Modbus RTU. Other protocols can use this
 *    indicator to allow them to return any number of bytes actually
 *    read.
 */
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
  if ((pTargIoChannel) && (pTargIoChannel->pRecvFunction))
  {
    return (pTargIoChannel->pRecvFunction(pTargIoChannel, pBuff, maxBytes, maxTimeout, pInterCharTimeoutOccurred));
  }
  return (0);
}

/* function: tmwtarg_transmit
 * purpose: Transmit bytes on the specified channel
 * arguments:
 *  pContext - Context returned from call to tmwtarg_initChannel
 *  pBuff - Array of bytes to transmit
 *  numBytes - Number of bytes to transmit
 * returns:
 *  TMWDEFS_TRUE if all the bytes were successfully transmitted,
 *  else TMWDEFS_FALSE.
 */
TMWTYPES_BOOL TMWDEFS_GLOBAL tmwtarg_transmit(
  void *pContext,
  TMWTYPES_UCHAR *pBuff,
  TMWTYPES_USHORT numBytes)
{
  /* Attempt to transmit numBytes bytes on this channel.
   */
  TMWTARG_IO_CHANNEL *pTargIoChannel = (TMWTARG_IO_CHANNEL *)pContext;
  if ((pTargIoChannel) && (pTargIoChannel->pXmitFunction))
  {
    return (pTargIoChannel->pXmitFunction(pTargIoChannel, pBuff, numBytes));
  }
  return (TMWDEFS_FALSE);
}

/* function: tmwtarg_transmitUDP
 * purpose: Transmit bytes using UDP on the specified channel
 * arguments:
 *  pContext - Context returned from call to tmwtarg_initChannel
 *  UDPPort - This is a define that indicates the remote UDP port to
 *   transmit to.
 *    TMWTARG_UDP_SEND       - Send to the remote port to be used for
 *                             requests or responses
 *    TMWTARG_UDP_SEND_UNSOL - Send to the remote port to be used for
 *                             unsolicited responses.  Once outstation has
 *                             received a request from master this would be
 *                             same port as all responses.
 *    TMWTARG_UDPONLY_BROADCAST Send to the broadcast address when UDP ONLY 
 *                             is configured.
 *  pBuff - Array of bytes to transmit
 *  numBytes - Number of bytes to transmit
 * returns:
 *  TMWDEFS_TRUE if all the bytes were successfully transmitted,
 *  else TMWDEFS_FALSE.
 * NOTE: This only needs to be implemented for DNP to support
 *  the DNP3 Specification IP Networking. It is not required
 *  for IEC or modbus and will not be called by those protocols.
 *  If DNP3 UDP is not required, this function can simply return TMWDEFS_FALSE
 */
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
  if ((pTargIoChannel) && (pTargIoChannel->pXmitUdpFunction))
  {
    return (pTargIoChannel->pXmitUdpFunction(pTargIoChannel, UDPPort, pBuff, numBytes));
  }
  return (TMWDEFS_FALSE);
}

/* The following two functions are only required for if
* DNPCNFG_SUPPORT_BINCONFIG is defined as TMWDEFS_TRUE
*/

/* function: tmwtarg_initBinFileValues */
TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL tmwtarg_initBinFileValues(
  TMWTARG_BINFILE_VALS *pBinTargFileValues)
{
  /* If you are not using winiotarg or liniotarg and want to implement your
   * own target layer configuration using the binary config dnp device profile
   * Put your code here 
   */
  TMWTARG_UNUSED_PARAM(pBinTargFileValues);
  return (TMWDEFS_FALSE);
}

/* function: tmwtarg_applyBinFileTargValues */
TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL tmwtarg_applyBinFileTargValues(
  void *pIoConfig,
  TMWTARG_BINFILE_VALS *pBinTargFileValues,
  TMWTYPES_BOOL *pIsChannelSerial)
{
  /* If you are not using winiotarg or liniotarg and want to implement your
   * own target layer configuration using the binary config dnp device profile
   * Put your code here 
   */
  TMWTARG_UNUSED_PARAM(pIoConfig);
  TMWTARG_UNUSED_PARAM(pBinTargFileValues);
  TMWTARG_UNUSED_PARAM(pIsChannelSerial);
  return (TMWDEFS_FALSE);
}

/* function: tmwtarg_sleep
 * purpose: suspends execution of the calling thread for specified number of milliseconds
 * arguments:
 *  milliseconds - specifies number of milliseconds to suspend the calling thread
 * returns
 *  void
 */
void TMWDEFS_GLOBAL tmwtarg_sleep(TMWTYPES_MILLISECONDS time)
{
  /* Put your code here */
  TMWTARG_UNUSED_PARAM(time);
}
