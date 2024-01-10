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

/* file: tmwlink.h
 * description: Implement link layer functionality required by all TMW link
 *  layer implementations.
 */
#include "tmwscl/utils/tmwtarg.h"
#include "tmwscl/utils/tmwlink.h"
#include "tmwscl/utils/tmwchnl.h"
#include "tmwscl/utils/tmwphys.h"
#include "tmwscl/utils/tmwphysd.h"
#include "tmwscl/tmwtarg/LinIoTarg/lintcp.h"

static void TMWDEFS_CALLBACK _openChannelTimeout(
  void *pCallbackParam);

/* function: _openChannelTimeout
 * purpose: Attempt to open channel, if unsuccessful start timer to try
 *  again at specified period until it succeeds.
 * arguments:
 *  pCallbackParam - user specified callback data, points to link context
 * returns:
 *  void
 */
static void TMWDEFS_CALLBACK _openChannelTimeout(
  void *pCallbackParam)
{
  TMWLINK_CONTEXT *pContext = (TMWLINK_CONTEXT *)pCallbackParam;
  if(!pContext->pChannel->pPhysContext->active)
    return;

  if(pContext->pChannel->pPhys->pPhysOpenChannel(pContext->pChannel->pPhysContext))
  {
    if(!pContext->isOpen)
    {
      /* Channel is open */
      pContext->isOpen = TMWDEFS_TRUE;

      /* Statistics */
      TMWCHNL_STAT_CALLBACK_FUNC(pContext->pChannel, TMWCHNL_STAT_OPEN, ((TMWTARG_IO_CHANNEL *)(pContext->pChannel->pPhysContext->pIOContext))->pChannelInfo);

     /* Tell protocol specific link layer channel is open */
      if(pContext->pLinkInfoFunc != TMWDEFS_NULL)
      {
        pContext->pLinkInfoFunc(pContext, 
         pContext->pChannel, TMWSCL_INFO_OPENED);
      }
    }
  }
  else
  {
    /* failed to connect, log error after first try */
    if((pContext->retryCount++ % 5 == 4) || (pContext->offlinePollPeriod == 0UL))
    {
      TMWPHYSD_ERROR(pContext->pChannel, "Waiting for connection");
    }

    /* Channel is not open */
    pContext->isOpen = TMWDEFS_FALSE;

    /* Restart open timer to try again later */
    if(pContext->offlinePollPeriod != 0UL)
    {
      tmwtimer_start(&pContext->openChannelTimer, 
        pContext->offlinePollPeriod, pContext->pChannel, 
        _openChannelTimeout, pContext);
    }
  }
}

/* function: tmwlink_initChannel */
void * TMWDEFS_GLOBAL tmwlink_initChannel(
  TMWCHNL *pChannel,
  TMWTYPES_MILLISECONDS offlinePollPeriod,
  TMWPHYS_CHANNEL_FUNC pChannelFunc,
  TMWPHYS_NEEDED_CHARS_FUNC pNeededCharsFunc,
  TMWPHYS_PARSE_FUNC pParseFunc,
  TMWLINK_LINK_INFO_FUNC pLinkInfoFunc,
  TMWPHYS_CHECK_ADDRESS_FUNC pCheckAddressFunc)
{
  TMWLINK_CONTEXT *pContext = pChannel->pLinkContext;

  pContext->isOpen = TMWDEFS_FALSE;
  pContext->retryCount = 0;
  pContext->offlinePollPeriod = offlinePollPeriod;
  pContext->pLinkInfoFunc = pLinkInfoFunc;
  tmwtimer_init(&pContext->openChannelTimer);

  /* Initialize callbacks into upper layer */
  pContext->pInfoFunc = TMWDEFS_NULL;
  pContext->pParseFunc = TMWDEFS_NULL;
  pContext->pCheckClassFunc = TMWDEFS_NULL;
  pContext->pCallbackParam = TMWDEFS_NULL;

#if TMWCNFG_SUPPORT_RXCALLBACKS
  pContext->pUserRxFrameCallback      = TMWDEFS_NULL;
  pContext->pUserRxFrameCallbackParam = TMWDEFS_NULL;
#endif

  /* Initialize session list */
  tmwdlist_initialize(&pContext->sessions);

  /* Set physical layer receive callbacks */
  pChannel->pPhys->pPhysSetCallbacks(
    pChannel->pPhysContext, 
    pChannelFunc,
    pNeededCharsFunc, 
    pParseFunc, 
    pCheckAddressFunc,
    pContext);

  return(pContext);
}

/* function: tmwlink_deleteChannel */
TMWTYPES_BOOL TMWDEFS_GLOBAL tmwlink_deleteChannel(
  TMWCHNL *pChannel)
{
  TMWLINK_CONTEXT *pContext = pChannel->pLinkContext;

  /* If there are any open session return failure */
  if(tmwdlist_size(&pContext->sessions) > 0U)
  {
    return(TMWDEFS_FALSE);
  }

  /* Cancel open timer in case we are still trying to open the channel */
  tmwtimer_cancel(&pContext->openChannelTimer);

  /* Reset physical layer receive callbacks */
  pChannel->pPhys->pPhysSetCallbacks(
    pChannel->pPhysContext, 
    TMWDEFS_NULL, TMWDEFS_NULL, TMWDEFS_NULL, 
    TMWDEFS_NULL, TMWDEFS_NULL);

  /* Make sure we don't do this again */
  pChannel->pLink = TMWDEFS_NULL;
  pChannel->pLinkContext = TMWDEFS_NULL;

  return(TMWDEFS_TRUE);
}

/* function: tmwlink_channelCallback */
void TMWDEFS_CALLBACK tmwlink_channelCallback(
  void *pCallbackParam,
  TMWTYPES_BOOL openOrClose,
  TMWDEFS_TARG_OC_REASON reason)
{

  TMWLINK_CONTEXT *pContext = (TMWLINK_CONTEXT *)pCallbackParam;
  if(openOrClose)
  {
    /* We are connected now, but don't call _openChannelTimeout() directly
     * since we are running in the context of the target layer
     * (connector thread) and this could recurse.
     */ 
    tmwtimer_start(&pContext->openChannelTimer, 
      1, pContext->pChannel, 
      _openChannelTimeout, pContext);
  }
  else
  {
#if TMWCNFG_SUPPORT_STATS
    TMWCHNL_STAT_ERROR_TYPE errorInfo;
    errorInfo.errorCode = TMWCHNL_ERROR_PHYS_REMOTE_CLOSE;
    errorInfo.pSession = TMWDEFS_NULL;
    TMWCHNL_STAT_CALLBACK_FUNC(pContext->pChannel, TMWCHNL_STAT_ERROR, &errorInfo);
    if(pContext->isOpen)
      TMWCHNL_STAT_CALLBACK_FUNC(pContext->pChannel, TMWCHNL_STAT_CLOSED, TMWDEFS_NULL);
#endif

    /* Tell protocol specific link layer channel is closed */
    if(pContext->pLinkInfoFunc != TMWDEFS_NULL)
    {
      pContext->pLinkInfoFunc(pContext, 
        pContext->pChannel, TMWSCL_INFO_CLOSED);
    }

    /* Close physical layer */
    (void)pContext->pChannel->pPhys->pPhysCloseChannel(
      pContext->pChannel->pPhysContext, reason);

    /* Mark channel closed */
    pContext->isOpen = TMWDEFS_FALSE;

    /* Restart open timer to open again later  
     * calling this directly from here can cause recursion 
     */
    tmwtimer_start(&pContext->openChannelTimer, 
      1, pContext->pChannel, 
      _openChannelTimeout, pContext);
  }
}

/* function: tmwlink_setCallbacks */
void TMWDEFS_CALLBACK tmwlink_setCallbacks(
  TMWLINK_CONTEXT *pContext,
  void *pCallbackParam,
  TMWLINK_INFO_FUNC pInfoFunc,
  TMWLINK_PARSE_FUNC pParseFunc,
  TMWLINK_CHECK_CLASS_FUNC pCheckClassFunc,
  TMWLINK_TX_CALLBACK_FUNC pBeforeTxCallback,
  TMWLINK_TX_CALLBACK_FUNC pAfterTxCallback,
  TMWLINK_TX_CALLBACK_FUNC pFailedTxCallback)
{
  pContext->pCallbackParam = pCallbackParam;
  pContext->pInfoFunc = pInfoFunc;
  pContext->pParseFunc = pParseFunc;
  pContext->pCheckClassFunc = pCheckClassFunc;
  pContext->pBeforeTxCallback = pBeforeTxCallback;
  pContext->pAfterTxCallback = pAfterTxCallback;
  pContext->pFailedTxCallback = pFailedTxCallback;
}

/* function: tmwlink_openChannel */
void TMWDEFS_GLOBAL tmwlink_openChannel(
  TMWLINK_CONTEXT *pContext)
{
  /* Don't try to open a channel that has been configured to be not active */
  if(!pContext->pChannel->pPhysContext->active)
    return;

  /* Attempt to open channel if it is not already open */
  if(!pContext->isOpen)
  {
    _openChannelTimeout(pContext);
  }
}

/* function: tmwlink_closeChannel */
void TMWDEFS_GLOBAL tmwlink_closeChannel(
  TMWLINK_CONTEXT *pContext)
{
  /* close channel if it is open */
  if(pContext->isOpen)
  {
    /* Statistics */
    TMWCHNL_STAT_CALLBACK_FUNC(pContext->pChannel, TMWCHNL_STAT_CLOSED, TMWDEFS_NULL);

    pContext->isOpen = TMWDEFS_FALSE;
    (void)pContext->pChannel->pPhys->pPhysCloseChannel(
      pContext->pChannel->pPhysContext, TMWDEFS_TARG_OC_LOCAL_CLOSED);
  }
}

/* function: tmwlink_setChannelInactive */
void TMWDEFS_GLOBAL tmwlink_setChannelInactive(
  TMWLINK_CONTEXT *pContext)
{
  /* Statistics */
  if(pContext->isOpen)
  {
    TMWCHNL_STAT_CALLBACK_FUNC(pContext->pChannel, TMWCHNL_STAT_CLOSED, TMWDEFS_NULL);

    /* Tell protocol specific link layer that the channel is closed */
    if(pContext->pLinkInfoFunc != TMWDEFS_NULL)
    {
      pContext->pLinkInfoFunc(pContext, 
        pContext->pChannel, TMWSCL_INFO_CLOSED);
    }
  }

  /* Close even if the target layer is not connected (isOpen) */
  pContext->isOpen = TMWDEFS_FALSE;
  (void)pContext->pChannel->pPhys->pPhysCloseChannel(
    pContext->pChannel->pPhysContext, TMWDEFS_TARG_OC_LOCAL_CLOSED);
}

/* function: tmwlink_openSession */
TMWTYPES_BOOL TMWDEFS_GLOBAL tmwlink_openSession(
  TMWLINK_CONTEXT *pContext,
  TMWSESN *pSession)
{
  /* Add session to list of sessions on this channel */
  tmwdlist_addEntry(&pContext->sessions, (TMWDLIST_MEMBER *)pSession);

  return(TMWDEFS_TRUE);
}

/* function: tmwlink_closeSession */
TMWTYPES_BOOL TMWDEFS_GLOBAL tmwlink_closeSession(
  TMWLINK_CONTEXT *pContext,
  TMWSESN *pSession)
{
  TMWTYPES_UINT listSize;
  /* Delete session from list of sessions on this channel */
  tmwdlist_removeEntry(&pContext->sessions, (TMWDLIST_MEMBER *)pSession);

  /* Close physical layer if channel does not have an auto open session callback registered,
   * there currently is a connection and there are no more sessions.
   * If there is a callback registered leave the connection up to look for received messages.
   */
  listSize = tmwdlist_size(&pContext->sessions);

  if((pContext->pChannel->pAutoOpenCallback == TMWDEFS_NULL) 
    && (pContext->isOpen)
    && (listSize == 0U))
  {
    
#ifdef TMW_SUPPORT_MONITOR 
  /* Don't close channel, just because sessions are closed, if in monitor mode */
  if(pContext->pChannel->pPhysContext->monitorMode)
    return(TMWDEFS_TRUE);
#endif

    pContext->isOpen = TMWDEFS_FALSE;

    /* Statistics */
    TMWCHNL_STAT_CALLBACK_FUNC(pContext->pChannel, TMWCHNL_STAT_CLOSED, TMWDEFS_NULL);

    (void)pContext->pChannel->pPhys->pPhysCloseChannel(
      pContext->pChannel->pPhysContext, TMWDEFS_TARG_OC_LOCAL_CLOSED);
  }

  return(TMWDEFS_TRUE);
}

/* function: tmwlink_getSessions */
TMWDLIST * TMWDEFS_GLOBAL tmwlink_getSessions(
  TMWLINK_CONTEXT *pContext)
{
  return(&pContext->sessions);
}

/* function: tmwlink_checkOpen */
TMWTYPES_BOOL TMWDEFS_GLOBAL tmwlink_checkOpen(
  TMWLINK_CONTEXT *pContext)
{
  if(!pContext->pChannel->pPhysContext->active) 
    return(pContext->isOpen);

  if(!pContext->isOpen)
  {
    /* Cancel current openChannelTimer */
    tmwtimer_cancel(&pContext->openChannelTimer);

    /* Try to open channel, restarting timer if required */
    _openChannelTimeout(pContext);
  }

  return(pContext->isOpen);
}
