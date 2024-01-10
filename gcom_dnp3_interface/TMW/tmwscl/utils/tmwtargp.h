/*****************************************************************************/
/* Triangle MicroWorks, Inc.                         Copyright (c) 1997-2022 */
/*****************************************************************************/
/*                                                                           */
/* This file is the property of:                                             */
/*                                                                           */
/*                       Triangle MicroWorks, Inc.                           */
/*                      Raleigh, North Carolina USA                          */
/*                       www.TriangleMicroWorks.com                          */
/*                          (919) 870-6615                                   */
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

/* file: tmwtargp.h  tmwtarg private 
 * description: This file contains the interface to some tmwtarg "private"
 *   functions provided by WinIoTarg and LinIoTarg. These functions are not
 *   required for the protocol libraries, but are used by the sample application
 *   code. The functions defined in this file are in addition to the functions 
 *   implemented in tmwtarg.c.
 */
#ifndef TMWTARGP_DEFINED
#define TMWTARGP_DEFINED

/* Include target specific header files as required */
#if !defined(_lint)
#if defined(_MSC_VER)
#include <stdio.h>
#include <string.h>
#include <memory.h>
#else
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#endif
#endif

/* Triangle MicroWorks, Inc. Header Files */
#include "tmwscl/utils/tmwcnfg.h"
#include "tmwscl/utils/tmwdefs.h"
#include "tmwscl/utils/tmwtypes.h"
#include "tmwscl/utils/tmwdiag.h"
#include "tmwscl/utils/tmwdtime.h"

#if defined(TMW_USE_BINARY_TREE)
#include "tmwscl/utils/THtmwsim.h"
#endif

/* The following typedefs allow registering functions for diagnostic and timer functionality
 * This may be done by if multiple applications such as the TestHarness, SDG, or WinToolkits
 * want to reuse the same target layer implementation, such as WinIoTarg. In this case the
 * diagnostic or timer functions cannot be hard coded in the tmwtarg.c target implementation.
 */

/* Define a callback function for users diagnostic interface
 */
typedef void (*TMWTARGP_PUT_DIAG_STRING_FUNC)(const TMWDIAG_ANLZ_ID *pAnlzId, const TMWTYPES_CHAR *pString);

/* Define a callback function used to call user defined command
*/
typedef void (*TMWTARGP_EXE_CMD_FUNC)(void *pCommandContext, TMWTYPES_UCHAR *pCommandBuf, TMWTYPES_UCHAR bufLength);

/* Define a callback function used to call user defined start timer
*/
typedef void (*TMWTARGP_START_TIMER_FUNC)(TMWTYPES_MILLISECONDS timeout, TMWTYPES_CALLBACK_FUNC pCallbackFunc, void *pCallbackParam);

/* Define a callback function used to call user defined cancel timer
*/
typedef void (*TMWTARGP_CANCEL_TIMER_FUNC)(void);

/* Define a callback function used to call user defined get session name
*/
typedef const TMWTYPES_CHAR * (*TMWTARGP_GET_SESSION_NAME_FUNC)(const TMWSESN *pSession);

/* Define a callback function used to call user defined get sector name
*/
typedef const TMWTYPES_CHAR * (*TMWTARGP_GET_SECTOR_NAME_FUNC)(const TMWSCTR *pSector);

/* Define a callback function used to call user defined get date time function
*/
typedef void (*TMWTARGP_GET_DATETIME_FUNC)(TMWDTIME *pDateTime);

/* Define a callback function used to call user defined set date time function
*/
typedef TMWTYPES_BOOL (*TMWTARGP_SET_DATETIME_FUNC)(TMWDTIME *pDateTime);



#ifdef __cplusplus
extern "C" {
#endif

#if TMWCNFG_SUPPORT_DIAG
 /* function: tmwtargp_registerPutDiagStringFunc
  * purpose: Register a function to Display a string of characters. 
  *  This function will be called to display diagnostic information 
  *  from the source code library 
  *  if registered. 
  * arguments:
  *  pFunc - pointer to function to register
  * returns: 
  *  void
  */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwtargp_registerPutDiagStringFunc(
    TMWTARGP_PUT_DIAG_STRING_FUNC pFunc);
#endif

#if !TMWCNFG_MULTIPLE_TIMER_QS

  /* function: tmwtargp_registerStartTimerFunc
   * purpose: Register a function to start a timer. 
   * arguments:
   *  pFunc - pointer to function to register
   * returns: 
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwtargp_registerStartTimerFunc(
    TMWTARGP_START_TIMER_FUNC pFunc);

  /* function: tmwtargp_registerCancelTimerFunc
   * purpose: Register a function to cancel a timer. 
   * arguments:
   *  pFunc - pointer to function to register
   * returns: 
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwtargp_registerCancelTimerFunc(
    TMWTARGP_CANCEL_TIMER_FUNC pFunc);
#endif

  /* function: tmwtargp_registerGetSessionNameFunc
   * purpose: Register a function to get the name of a session. 
   * arguments:
   *  pFunc - pointer to function to register
   * returns: 
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwtargp_registerGetSessionNameFunc(
    TMWTARGP_GET_SESSION_NAME_FUNC pFunc);

  /* function: tmwtargp_registerGetSectorNameFunc
   * purpose: Register a function to get the name of a sector. 
   * arguments:
   *  pFunc - pointer to function to register
   * returns: 
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwtargp_registerGetSectorNameFunc(
    TMWTARGP_GET_SECTOR_NAME_FUNC pFunc);

  /* function: tmwtargp_registerGetTimeStamp
   * purpose: Register a function to get time 
   * arguments:
   *  pFunc - pointer to function to register
   * returns: 
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwtargp_registerGetDateTimeFunc(
    TMWTARGP_GET_DATETIME_FUNC pFunc); 

  /* function: tmwtargp_registerSetDateTimeFunc
   * purpose: Register a function to set time  
   * arguments:
   *  pFunc - pointer to function to register
   * returns: 
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwtargp_registerSetDateTimeFunc(
    TMWTARGP_SET_DATETIME_FUNC pFunc);

  /* function: tmwtargp_Sleep
   * purpose: put the process to sleep 
   * arguments:
   *  time - number of milliseconds to sleep
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL tmwtargp_Sleep(
    TMWTYPES_MILLISECONDS time);

  /* function: tmwtargp_requestFailure  
   * purpose: Make the requested communications channel fail to transmit and
   *          receive. This can be used to test communication failures.
   * arguments:
   *  pContext - pointer to communications channel context returned by tmwtarg_initChannel
   *  failureCnt - number of transmits to discard
   * returns
   *  TRUE if successfully stored this request. FALSE, if there is no
   *  more room in the table.
   */
  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL tmwtargp_requestFailure(
    void *pContext, 
    TMWTYPES_USHORT failureCnt);

  /* function: tmwtargp_xmtFailureRequested 
   * purpose: See if the communications channel was configured to fail to transmit
   *          This is used to test communication failures.
   * arguments:
   *  pContext - pointer to communications channel context returned by tmwtarg_initChannel
   * returns
   *  TRUE if this transmit should fail.
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL tmwtargp_xmtFailureRequested(
    void *pContext);

  /* function: tmwtargp_resetChannel  
   * purpose: Reset a communications channel (this indirectly causes a target close and open)
   * arguments:
   *  pContext - pointer to communications channel context returned by tmwtarg_initChannel
   * returns
   *  TRUE if supported. FALSE, if not
   */
  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL tmwtargp_resetChannel( void *pContext);

  /* function: tmwtargp_waitForInput
   * NOTE  Private use by a TMW application. NOT REQUIRED by the TMW libraries
   * Only called from .NET code TMWChannel::WaitForInput()
   * purpose: Call into target layer when received data is available, bypassing tmwtarg_receive()
   * arguments:
   *  pContext - pointer to communications channel context returned by tmwtarg_initChannel
   * returns
   *  TRUE if supported. FALSE, if not
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL tmwtargp_waitForInput(
    void *pContext);
  
#ifdef __cplusplus
};
#endif

#endif /* TMWTARGP_DEFINED */
