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

/* file: tmwchnl.c
 * description: Implementation of functions that apply to a TMW channel.
 */
#include "tmwscl/utils/tmwtarg.h"
#include "tmwscl/utils/tmwdefs.h"
#include "tmwscl/utils/tmwchnl.h"

/* function: tmwchnl_initChannel */
void TMWDEFS_GLOBAL tmwchnl_initChannel(
  TMWAPPL *pApplContext,
  TMWCHNL *pChannel,
  TMWCHNL_STAT_CALLBACK pStatCallback,
  void *pStatCallbackParam,
  TMWCHNL_IDLE_CALLBACK pIdleCallback,
  void *pIdleCallbackParam,
  TMWCHNL_AUTO_OPEN_FUNC pAutoOpenCallback,
  void *pAutoOpenCallbackParam,
  TMWTYPES_BOOL lockOwner,
  TMWCHNL_TYPE chanType,
  TMWTYPES_ULONG chnlDiagMask)
{
  pChannel->chanType = chanType;
  pChannel->pApplContext = pApplContext;

  pChannel->pPhys = TMWDEFS_NULL;
  pChannel->pPhysContext = TMWDEFS_NULL;

  pChannel->pLink = TMWDEFS_NULL;
  pChannel->pLinkContext = TMWDEFS_NULL;

  pChannel->pTprt = TMWDEFS_NULL;
  pChannel->pTprtContext = TMWDEFS_NULL;

  pChannel->pStatCallbackFunc = pStatCallback;
  pChannel->pStatCallbackParam = pStatCallbackParam;
  
  pChannel->pIdleCallbackFunc = pIdleCallback;
  pChannel->pIdleCallbackParam = pIdleCallbackParam;

  pChannel->pAutoOpenCallback = pAutoOpenCallback;
  pChannel->pAutoOpenCallbackParam = pAutoOpenCallbackParam;

  tmwdlist_initialize(&pChannel->messageQueue);
  pChannel->numberQueued = 0;
  pChannel->pCurrentMessage = TMWDEFS_NULL;

  pChannel->polledMode = TMWDEFS_TRUE;
  pChannel->chnlDiagMask = chnlDiagMask;
  pChannel->pLinkRdcyOnlineCallback = TMWDEFS_NULL;

  tmwtimer_init(&pChannel->incrementalTimer);

#if TMWCNFG_SUPPORT_THREADS
  if(lockOwner)
  {
    pChannel->lockOwner = TMWDEFS_TRUE;
    TMWTARG_LOCK_INIT(&pChannel->lock);
  }
  else
  {
    pChannel->lockOwner = TMWDEFS_FALSE;
  }
#else
  TMWTARG_UNUSED_PARAM(lockOwner);
#endif
  
#if TMWCNFG_MULTIPLE_TIMER_QS
  /* If multiple timer support, initialize per channel timer queue */
  tmwtimer_initMultiTimer(pChannel);
#endif
}

/* function: tmwchnl_deleteChannel */
void TMWDEFS_GLOBAL tmwchnl_deleteChannel(
  TMWCHNL *pChannel)
{
#if TMWCNFG_MULTIPLE_TIMER_QS
  if (tmwtarg_deleteMultiTimer(pChannel))
  {
    TMWDIAG_CHNL_ERROR(pChannel, "tmwtarg_deleteMultiTimer returned failure")
  }
#endif
#if TMWCNFG_SUPPORT_THREADS
  if(pChannel->lockOwner)
    TMWTARG_LOCK_DELETE(&pChannel->lock);
#else
  TMWTARG_UNUSED_PARAM(pChannel);
#endif
}

/* function: tmwchnl_getChannelName */
const TMWTYPES_CHAR * TMWDEFS_GLOBAL tmwchnl_getChannelName(
  TMWCHNL *pChannel)
{
  if(pChannel && pChannel->pPhysContext)
  {
    return(tmwtarg_getChannelName(pChannel->pPhysContext->pIOContext));
  }

  return(TMWDEFS_NULL);
}

/* function: tmwchnl_setUserDataPtr */
void TMWDEFS_GLOBAL tmwchnl_setUserDataPtr(
  TMWCHNL *pChannel, void *pUserData)
{
  pChannel->pUserData = pUserData;
}
  
/* function: tmwchnl_getUserDataPtr */
void * TMWDEFS_GLOBAL tmwchnl_getUserDataPtr(
  TMWCHNL *pChannel)
{
  return pChannel->pUserData;
}

/* function: tmwchnl_setStatCallback */
void TMWDEFS_GLOBAL tmwchnl_setStatCallback(
  TMWCHNL *pChannel,
  TMWCHNL_STAT_CALLBACK pCallback,
  void *pCallbackParam)
{
  pChannel->pStatCallbackFunc = pCallback;
  pChannel->pStatCallbackParam = pCallbackParam;
}

/* function: tmwchnl_callStatCallback */
void TMWDEFS_GLOBAL tmwchnl_callStatCallback(
  TMWCHNL *pChannel,
  TMWCHNL_STAT_EVENT eventType,
  void *pEventData)
{
  if(pChannel->pStatCallbackFunc != TMWDEFS_NULL)
  {
    pChannel->pStatCallbackFunc(
      pChannel->pStatCallbackParam, eventType, pEventData);
  }
}

/* function: tmwchnl_setIdleCallback */
void TMWDEFS_GLOBAL tmwchnl_setIdleCallback(
  TMWCHNL *pChannel,
  TMWCHNL_IDLE_CALLBACK pCallback,
  void *pCallbackParam)
{
  pChannel->pIdleCallbackFunc = pCallback;
  pChannel->pIdleCallbackParam = pCallbackParam;
}

/* function: tmwchnl_lockChannel */
void tmwchnl_lockChannel(
  TMWCHNL *pChannel)
{
#if TMWCNFG_SUPPORT_THREADS
  TMWTARG_LOCK_SECTION(&pChannel->lock);
#else
  TMWTARG_UNUSED_PARAM(pChannel);
#endif
}

/* function: tmwchnl_unlockChannel */
void tmwchnl_unlockChannel(
  TMWCHNL *pChannel)
{
#if TMWCNFG_SUPPORT_THREADS
  TMWTARG_UNLOCK_SECTION(&pChannel->lock);
#else
  TMWTARG_UNUSED_PARAM(pChannel);
#endif
}

