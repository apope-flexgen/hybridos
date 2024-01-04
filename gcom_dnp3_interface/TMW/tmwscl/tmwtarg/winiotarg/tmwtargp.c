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


/* file: tmwtargp.c
 * description: tmwtarg "private" implementation of routines defined in 
 *  tmwtargp.h for use with WinIoTarg. These functions are used by the sample
 *  application, but are not required to be implemented for the protocol
 *  libraries
 */
#include "tmwscl/utils/tmwtarg.h"
#include "tmwscl/utils/tmwcnfg.h"
#include "tmwscl/utils/tmwsim.h"
#include "tmwscl/utils/tmwpltmr.h"

#include "WinIoTarg/include/WinIoTarg.h"
#include "tmwtargio.h"
#include <stdlib.h>
#if _MSC_VER > 1400
#include <strsafe.h>
#else
#include <stdarg.h>
#endif
   
 
#if TMWCNFG_SUPPORT_DIAG

/* Function to convert WinIoTarg logging to match rest of SCL */
void tmwtargp_WinIoTargLogFunc(void * pChannel, unsigned long trace_mask, TMWDIAG_ID sourceId, const char *_id, const char *format, va_list ap)
{
#if _MSC_VER > 1400
  size_t length;
  char strText[1024];
  TMWTARG_UNUSED_PARAM(trace_mask);
  strcpy_s(strText, 1024, _id);
  length = strlen(_id); 
  vsnprintf_s(strText+length, (1024-length), _TRUNCATE, format, ap);    
  TMWDIAG_TARGET(pChannel, sourceId, strText);
#endif
}

extern TMWTARGP_PUT_DIAG_STRING_FUNC pPutDiagStringFunc;
/* function: tmwtargp_registerPutDiagStringFunc */
void TMWDEFS_GLOBAL tmwtargp_registerPutDiagStringFunc(
  TMWTARGP_PUT_DIAG_STRING_FUNC pFunc)
{
  pPutDiagStringFunc = pFunc;

  /* Register to receive logging from WinIoTarg. 
   * If application calls WinIoTarg_setProtoAnaLogFun this will be overwritten by their function 
   */
  WinIoTarg_setProtoAnaLogFun(tmwtargp_WinIoTargLogFunc);
}

#endif /* TMWCNFG_SUPPORT_DIAG */
 

/* function: tmwtargp_waitForInput */
TMWTYPES_BOOL TMWDEFS_GLOBAL tmwtargp_waitForInput(
  void *pContext)
{
  return WinIoTarg_waitForInput(pContext);
}

 
extern TMWTARGP_GET_DATETIME_FUNC pGetDateTimeFunc;
void TMWDEFS_GLOBAL tmwtargp_registerGetDateTimeFunc(TMWTARGP_GET_DATETIME_FUNC pFunc)
{
  pGetDateTimeFunc = pFunc;
} 

extern TMWTARGP_SET_DATETIME_FUNC pSetDateTimeFunc;
void TMWDEFS_GLOBAL tmwtargp_registerSetDateTimeFunc(TMWTARGP_SET_DATETIME_FUNC pFunc)
{
  pSetDateTimeFunc = pFunc;
}

#if !TMWCNFG_MULTIPLE_TIMER_QS 
extern TMWTARGP_START_TIMER_FUNC pStartTimerFunc;
void TMWDEFS_GLOBAL tmwtargp_registerStartTimerFunc(TMWTARGP_START_TIMER_FUNC pFunc)
{
  pStartTimerFunc = pFunc;
}

extern TMWTARGP_CANCEL_TIMER_FUNC pCancelTimerFunc;
void TMWDEFS_GLOBAL tmwtargp_registerCancelTimerFunc(TMWTARGP_CANCEL_TIMER_FUNC pFunc)
{
  pCancelTimerFunc = pFunc;
}
#endif

/* function: tmwtargp_resetChannel */
TMWTYPES_BOOL TMWDEFS_GLOBAL tmwtargp_resetChannel(
  void *pContext)
{
  /* Reset a previously opened channel
   */
  return WinIoTarg_resetChannel(pContext);
}

#if TMWCNFG_SUPPORT_DIAG
extern TMWTARGP_GET_SESSION_NAME_FUNC pGetSessionNameFunc;
void TMWDEFS_GLOBAL tmwtargp_registerGetSessionNameFunc(TMWTARGP_GET_SESSION_NAME_FUNC pFunc)
{
  pGetSessionNameFunc = pFunc;
}

extern TMWTARGP_GET_SECTOR_NAME_FUNC pGetSectorNameFunc;
void TMWDEFS_GLOBAL tmwtargp_registerGetSectorNameFunc(TMWTARGP_GET_SECTOR_NAME_FUNC pFunc)
{
  pGetSectorNameFunc = pFunc;
}

#endif

#define MAX_FAILURE_CONTEXTS 16
static TMWTYPES_INT failureContextCount;
static void *pFailureContext[MAX_FAILURE_CONTEXTS];
static TMWTYPES_USHORT failureCount[MAX_FAILURE_CONTEXTS];

/* function: _xmtFailureRequested */
TMWTYPES_BOOL TMWDEFS_GLOBAL tmwtargp_xmtFailureRequested(
  void *pContext)
{
  TMWTYPES_INT i;

  /* If it is in the list return whether to fail or not */
  /* Count indicates how many to transmits to fail */
  for (i = 0; i < failureContextCount; i++)
  {
    if (pFailureContext[i] == pContext)
    {
      if(failureCount[i] > 0)
      {
        failureCount[i]--;
        return TMWDEFS_TRUE;
      }

      /* remove this context since it is zero */
      tmwtargp_requestFailure(pContext, 0); 
    }
  }
  return TMWDEFS_FALSE;
}

/* function: tmwtargp_requestFailure  */
TMWTYPES_BOOL TMWDEFS_GLOBAL tmwtargp_requestFailure(
  void *pContext,
  TMWTYPES_USHORT failureCnt)
{
  TMWTYPES_INT i;

  if (failureCnt == 0)
  {
    /* If context is in list, set the failure status */
    for (i = 0; i < failureContextCount; i++)
    {
      if (pFailureContext[i] == pContext)
      {
        /* remove from list and move all contexts down */
        failureContextCount--;
        for (; i < failureContextCount; i++)
        {
          pFailureContext[i] = pFailureContext[i + 1];
        }
        pFailureContext[i] = TMWDEFS_NULL;
        return TMWDEFS_TRUE;
      }
    }
    return TMWDEFS_FALSE;
  }
  else
  {
    /* See if context is already in the list */
    for (i = 0; i < failureContextCount; i++)
    {
      if (pFailureContext[i] == pContext)
      {
        failureCount[i] = failureCnt;
        return TMWDEFS_TRUE;
      }
    }
    if (i < MAX_FAILURE_CONTEXTS)
    {
      pFailureContext[i] = pContext;
      failureCount[i] = failureCnt;
      failureContextCount++;
      return TMWDEFS_TRUE;
    }

    return TMWDEFS_FALSE;
  }
}
