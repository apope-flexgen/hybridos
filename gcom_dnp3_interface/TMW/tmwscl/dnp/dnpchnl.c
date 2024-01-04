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

/* file: dnpchnl.c
 * description: This file contains the implementation of the DNP Master
 *  Application Layer.
 */
#include "tmwscl/dnp/dnpdiag.h"
#include "tmwscl/utils/tmwtarg.h"
#include "tmwscl/utils/tmwmem.h"

#include "tmwscl/dnp/dnpchnl.h"
#include "tmwscl/dnp/dnpmem.h"
#include "tmwscl/dnp/dnpstat.h"

#ifdef TMW_SUPPORT_MONITOR 
#include "tmwscl/dnp/dnpmntr.h"
#endif

/* function: _removeRequest
 * purpose: Remove request from message queue.
 * arguments:
 *  pTxData - pointer to Tx Data structure
 * returns:
 *  void
 */
static void TMWDEFS_LOCAL _removeRequest(
  TMWSESN_TX_DATA *pTxData)
{
  TMWCHNL *pChannel = pTxData->pChannel;

  /* Delete request from pending message queue */
  tmwdlist_removeEntry(&pChannel->messageQueue, (TMWDLIST_MEMBER *)pTxData);

  if(dnputil_getCurrentMessage(pTxData->pSession) == pTxData)
  {
    tmwtimer_cancel(&pChannel->incrementalTimer);
    dnputil_setCurrentMessage(pTxData->pSession, TMWDEFS_NULL);
  }
}

/* function: _infoCallback */
static void TMWDEFS_CALLBACK _infoCallback(
  void *pParam,
  TMWSESN *pSession,
  TMWSCL_INFO sesnInfo)
{
  DNPSESN *pDNPSession = (DNPSESN *)pSession;
  TMWTARG_UNUSED_PARAM(pParam);

  if (pDNPSession->pProcessInfoFunc != TMWDEFS_NULL)
    pDNPSession->pProcessInfoFunc(pSession, sesnInfo);
}

#if !DNPCNFG_MULTI_SESSION_REQUESTS
/* function: _offlineDelayTimeout */
static void TMWDEFS_CALLBACK _offlineDelayTimeout(
  void *pCallbackParam)
{
  TMWCHNL *pChannel = (TMWCHNL*)pCallbackParam;
  /* see if there is anything in queue to be sent */
  if (tmwdlist_size(&pChannel->messageQueue) != 0)
  {
    /* Tell link layer we have a frame to transmit
    * Link layer will ask each session if it has something to send
    */
    DNPSESN *pStartSession;
    DNPCHNL *pDNPChannel = (DNPCHNL*)pChannel;
    DNPSESN *pSession = pStartSession = (DNPSESN*)pDNPChannel->pLastOfflineSessionRequest;
    do
    {
      pSession = (DNPSESN *)tmwdlist_getAfter(&pChannel->pLinkContext->sessions, (TMWDLIST_MEMBER *)pSession);
      if (pSession == TMWDEFS_NULL)
      {
        if (pSession == pStartSession)
        {
          break;
        }
        else
        {
          pSession = (DNPSESN *)tmwdlist_getFirst(&pChannel->pLinkContext->sessions);
        }
      }

      if (pSession == TMWDEFS_NULL || (pSession->tmw.online == TMWSESN_STAT_OFFLINE))
      {
        pDNPChannel->pLastOfflineSessionRequest = (TMWSESN*)pSession;
        pChannel->pTprt->pTprtDataReady(pChannel->pTprtContext, (TMWSESN*)pSession);
        return;
      }
    } while (pSession != pStartSession);
  }
}

/* function: _offlineProcessing */
static void _offlineProcessing(TMWSESN_TX_DATA *pTxData)
{
  DNPCHNL *pDNPChannel = (DNPCHNL *)pTxData->pChannel;
  if (pDNPChannel->channelOffLineDelay != 0)
  {
    if (!tmwtimer_isActive(&pDNPChannel->channelOffLineDelayTimer))
    {
      tmwtimer_start(&pDNPChannel->channelOffLineDelayTimer, pDNPChannel->channelOffLineDelay, (TMWCHNL*)pDNPChannel, _offlineDelayTimeout, pDNPChannel);
    }
  }
}
#endif

/* function: _applError
 * purpose: Handle general application layer errors
 * arguments:
 *  pTxData - pointer to transmit data structure
 *  status - response status corresponding to error
 *  sessionOffline - if true mark the session offline
 * returns:
 *  void
 */
static void TMWDEFS_LOCAL _applError(
  TMWSESN_TX_DATA *pTxData,
  DNPCHNL_RESP_STATUS status,
  TMWTYPES_BOOL sessionOffline)
{
  DNPSESN *pDNPSession;
  TMWCHNL *pChannel = pTxData->pChannel;

  /* Cancel timers */
  tmwtimer_cancel(&pTxData->responseTimer);

  /* Tell transport layer that we have given up */
  pChannel->pTprt->pTprtCancel(pChannel->pTprtContext, pTxData);

  /* Remove this request from queue */ 
  _removeRequest(pTxData);

#if DNPCNFG_SUPPORT_AUTHENTICATION
  if((pTxData->txFlags & TMWSESN_TXFLAGS_DNP_AUTH) != 0)
  {
    /* Tel SA that this failed */
    if (((DNPCHNL_TX_DATA *)pTxData)->pInternalCallback != TMWDEFS_NULL)
    {
      /* Initialize callback info */
      DNPCHNL_RESPONSE_INFO response;
      response.iin = 0;
      response.last = TMWDEFS_TRUE;
      response.pSession = pTxData->pSession;
      response.pTxData = (TMWSESN_TX_DATA *)pTxData;
      response.pRxData = TMWDEFS_NULL;
      response.status = status;
      response.requestStatus = 0;
      response.responseTime = 0;

      ((DNPCHNL_TX_DATA *)pTxData)->pInternalCallback(((DNPCHNL_TX_DATA *)pTxData)->pInternalCallbackParam, &response);
    }
    return;
  }
#endif

  pDNPSession = (DNPSESN *)pTxData->pSession;
  
  /*  If direct no ack needed to stay queued long enough in case a challenge is received this would be success status */
  if(status != DNPCHNL_RESP_STATUS_SUCCESS) 
  {
    if(status != DNPCHNL_RESP_STATUS_CANCELED)
    {
      TMWTYPES_BOOL isRead = (pTxData->pMsgBuf[DNPDEFS_AH_INDEX_FUNC_CODE] == DNPDEFS_FC_READ);
      if(!isRead || (++pDNPSession->readFailedCount > pDNPSession->readTimeoutsAllowed))
      { 
        if (sessionOffline)
        {
          /* Take session offline */
          tmwsesn_setOnline((TMWSESN *)pDNPSession, TMWDEFS_FALSE);
          _infoCallback(TMWDEFS_NULL, (TMWSESN *)pDNPSession, TMWSCL_INFO_OFFLINE);
        }
      }

#if DNPCNFG_SUPPORT_AUTHENTICATION
      _infoCallback(TMWDEFS_NULL, (TMWSESN *)pDNPSession, TMWSCL_INFO_TIMEOUT);
#endif
    }

    if((((DNPCHNL_TX_DATA *)pTxData)->pInternalCallback != TMWDEFS_NULL)
      || (((DNPCHNL_TX_DATA *)pTxData)->pUserCallback != TMWDEFS_NULL))
    {
      /* Initialize user callback info */
      DNPCHNL_RESPONSE_INFO response;
      response.iin = 0;
      response.last = TMWDEFS_TRUE;
      response.pSession = pTxData->pSession;
      response.pTxData = (TMWSESN_TX_DATA *)pTxData;
      response.pRxData = TMWDEFS_NULL; 
      response.status = status;
      response.requestStatus = 0;
      response.responseTime = 0;

      /* Call internal callback if specified */
      if(((DNPCHNL_TX_DATA *)pTxData)->pInternalCallback != TMWDEFS_NULL)
      {
        ((DNPCHNL_TX_DATA *)pTxData)->pInternalCallback(((DNPCHNL_TX_DATA *)pTxData)->pInternalCallbackParam, &response);
      }

      /* Call user callback if specified */
      dnpchnl_userCallback(pChannel, (DNPCHNL_TX_DATA *)pTxData, &response);
    }

    /* Call session specific cleanup */
    if(pDNPSession->pAbortMessage != TMWDEFS_NULL)
      pDNPSession->pAbortMessage(pTxData->pSession, pTxData, status);

#if DNPCNFG_SUPPORT_AUTHENTICATION
    /* let mdnpauth contact user and free authentication request */
    if((pTxData->txFlags & TMWSESN_TXFLAGS_DNP_AUTH) == 0)
#endif
      dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
  }

  if(pDNPSession->pNextMessage != TMWDEFS_NULL)
    pDNPSession->pNextMessage((TMWSESN *)pDNPSession);
 
  /* See if there is anything else in the queue */
  if(tmwdlist_size(&pChannel->messageQueue) != 0)
  {
    /* Tell link layer we have a frame to transmit 
     * Link layer will ask each session if it has something to send
     */
    pChannel->pLink->pLinkDataReady(pChannel->pLinkContext, TMWDEFS_NULL);
  }
  else
  {
    dnpchnl_checkForChannelIdle(pChannel);
  }
}   

/* function: _responseTimeout
 * purpose: Handle application layer response timeouts.
 * arguments:
 *  pCallbackParam - user specified callback parameter, points to transmit data
 * returns:
 *  void
 */
static void TMWDEFS_CALLBACK _responseTimeout(
  void *pCallbackParam)
{
  TMWSESN_TX_DATA *pTxData = (TMWSESN_TX_DATA *)pCallbackParam;


  /* Diagnostics */
  DNPDIAG_ERROR_MSG(pTxData->pSession->pChannel, pTxData->pSession, DNPDIAG_RESP_TIMEOUT, (TMWTYPES_CHAR *)pTxData->pMsgDescription);

  /* Statistics */
  DNPSTAT_SESN_REQUEST_TIMEOUT(pTxData->pSession);

  /* Call cancel fragment routine to cleanup */
  /* Don't take session offline. If the remote is really not responding, a channel timeout would occur
   * The response can timeout because of multiple sessions on the channel delaying another response too long. 
   */
  _applError(pTxData, DNPCHNL_RESP_STATUS_TIMEOUT, TMWDEFS_FALSE);
}

#if !DNPCNFG_MULTI_SESSION_REQUESTS
/* function: _channelTimeout
 * purpose: Handle response timeout for this channel.
 * arguments:
 *  pCallbackParam - user specified callback parameter, points to transmit data
 * returns:
 *  void
 */
static void TMWDEFS_CALLBACK _channelTimeout(
  void *pCallbackParam)
{
  TMWSESN_TX_DATA *pTxData = (TMWSESN_TX_DATA *)pCallbackParam;

#if DNPCNFG_SUPPORT_AUTHENTICATION
  DNPCHNL_TX_DATA *pDNPTxData = (DNPCHNL_TX_DATA *)pCallbackParam;
  if((pDNPTxData->dnpTxFlags & DNPCHNL_DNPTXFLAGS_AUTH_NOACKDELAY) != 0)
  {
    /* This was used as a delay timer to see if a challenge is received 
     * This is NOT an error, but the code works well for this case
     */
    _applError(pTxData, DNPCHNL_RESP_STATUS_SUCCESS, TMWDEFS_FALSE);
    return;
  }
#endif

  /* Diagnostics */
  DNPDIAG_ERROR_MSG(pTxData->pSession->pChannel, pTxData->pSession, DNPDIAG_CHANNEL_TIMEOUT, (TMWTYPES_CHAR *)pTxData->pMsgDescription);

  /* Statistics */
  DNPSTAT_SESN_REQUEST_TIMEOUT(pTxData->pSession);

  _offlineProcessing(pTxData);

  /* Call cancel fragment routine to cleanup */
  _applError(pTxData, DNPCHNL_RESP_STATUS_TIMEOUT, TMWDEFS_TRUE);
}
#endif

/* function _beforeTxCallback */
static void TMWDEFS_CALLBACK _beforeTxCallback(
  void *pUserData,
  TMWSESN_TX_DATA *pTxData)
{
  DNPSESN *pDNPSession= (DNPSESN *)pTxData->pSession;

  TMWTARG_UNUSED_PARAM(pUserData);

  /* Call session specific before transmit routine */
  if(pDNPSession->pBeforeTxCallback != TMWDEFS_NULL)
    pDNPSession->pBeforeTxCallback(pTxData);
}

#if DNPCNFG_SUPPORT_AUTHENTICATION
static void _saveLastTxFragment(
  TMWSESN_TX_DATA *pTxData)
{
  DNPSESN *pDNPSession = (DNPSESN*)pTxData->pSession;
  /* Check for broadcast request */
  if(pDNPSession != TMWDEFS_NULL
    && pDNPSession->saveLastTxFragment)
  {
    if(pTxData->pMsgBuf[DNPDEFS_AH_INDEX_APPL_CTRL] & DNPDEFS_AC_UNSOLICITED)
    {
      /* don't save unsolicited application confirms */ 
      if(pTxData->pMsgBuf[DNPDEFS_AH_INDEX_FUNC_CODE] == DNPDEFS_FC_CONFIRM)
      {
        return;
      }
#if DNPCNFG_MULTI_SESSION_REQUESTS
      /* saved in session structure in SDNP only */
      if (!pDNPSession->operateInV2Mode && (pDNPSession->pSaveLastUnsolSentFunc != TMWDEFS_NULL))
      {
        pDNPSession->pSaveLastUnsolSentFunc(pTxData->pSession, pTxData);
        return;
      }
#else
      /* saved in channel structure */
      if(!pDNPSession->operateInV2Mode)
      {
        DNPCHNL *pDNPChnl = (DNPCHNL *)pTxData->pChannel;
        memcpy(pDNPChnl->lastUnsolTxFragment, pTxData->pMsgBuf, pTxData->msgLength);
        pDNPChnl->lastUnsolTxFragmentLength = pTxData->msgLength;
        return;
      }
#endif
    }
    /* don't save authentication requests */
    if (pTxData->pMsgBuf[DNPDEFS_AH_INDEX_FUNC_CODE] != DNPDEFS_FC_AUTH_REQUEST)
    {
#if DNPCNFG_MULTI_SESSION_REQUESTS
      /* Check for broadcast request */
      if (pDNPSession != TMWDEFS_NULL)
      {
        memcpy(pDNPSession->lastTxFragment, pTxData->pMsgBuf, pTxData->msgLength);
        pDNPSession->lastTxFragmentLength = pTxData->msgLength;
    }
#else 
      DNPCHNL *pDNPChnl = (DNPCHNL *)pTxData->pChannel;
      memcpy(pDNPChnl->lastTxFragment, pTxData->pMsgBuf, pTxData->msgLength);
      pDNPChnl->lastTxFragmentLength = pTxData->msgLength;
#endif
    }
  }
}
#endif

/* function _afterTxCallback */
static void TMWDEFS_CALLBACK _afterTxCallback(
  void *pUserData,
  TMWSESN_TX_DATA *pTxData)
{
  TMWCHNL *pChannel = pTxData->pChannel;
  DNPSESN *pDNPSession = (DNPSESN *)pTxData->pSession;
#if DNPCNFG_SUPPORT_AUTHENTICATION
  DNPCHNL_TX_DATA* pDnpTxData = (DNPCHNL_TX_DATA*)pTxData;
#endif

  TMWTARG_UNUSED_PARAM(pUserData);

  /* Save time last byte was transmitted if requested */
  if((pTxData->txFlags & TMWSESN_TXFLAGS_SAVE_LAST_BYTE_TIME) != 0)
    tmwdtime_getDateTime((TMWSESN*)pDNPSession, &pChannel->lastByteTime);

  /* Call session specific after transmit routine */
  if(pDNPSession->pAfterTxCallback != TMWDEFS_NULL)
      pDNPSession->pAfterTxCallback(pTxData);
 
#if DNPCNFG_SUPPORT_AUTHENTICATION
  _saveLastTxFragment((TMWSESN_TX_DATA *)pTxData); 
  if((pDnpTxData->dnpTxFlags & DNPCHNL_DNPTXFLAGS_AUTH_NOACKDELAY) != 0)
  {
#if !DNPCNFG_MULTI_SESSION_REQUESTS
    DNPCHNL *pDnpChannel = (DNPCHNL *)pTxData->pChannel;
    tmwtimer_start(&pDnpChannel->tmw.incrementalTimer,
      pDnpChannel->directNoAckDelayTime, (TMWCHNL*)pDnpChannel,
      _channelTimeout, pTxData);
#endif
  }
  /* don't free Authentication TxData structures that might need to be reused */
  else if((pTxData->txFlags & TMWSESN_TXFLAGS_DNP_AUTH) != 0)
  {
    tmwtimer_cancel(&pTxData->responseTimer);
    _removeRequest(pTxData);
  }
  else
#endif

  /* If no response expected, clean up */
  if((pTxData->txFlags & TMWSESN_TXFLAGS_NO_RESPONSE) != 0)
  {
    _removeRequest(pTxData);
    dnpchnl_freeTxData(pTxData);

    /* See if session has more fragments to send */
    if(pDNPSession->pNextMessage != TMWDEFS_NULL)
      pDNPSession->pNextMessage((TMWSESN *)pDNPSession);
  }
  else if(pDNPSession->tmw.type == TMWTYPES_SESSION_TYPE_MASTER)
  {
    /* Since a response is expected start a channel response timer.
     * This will allow a short timeout period to be specified per channel,
     * while allowing a longer response timeout for each request.
     * Use channel incremental timer for this purpose for MDNP     
     */
    pChannel = (TMWCHNL *)pTxData->pChannel;
    if(pChannel->incrementalTimeout != 0)
    {
#if !DNPCNFG_MULTI_SESSION_REQUESTS
      tmwtimer_start(&pChannel->incrementalTimer,
        pChannel->incrementalTimeout, pChannel,
        _channelTimeout, pTxData);
#endif
    }
  }
}

/* function _failedTxCallback */
static void TMWDEFS_CALLBACK _failedTxCallback(
  void *pUserData,
  TMWSESN_TX_DATA *pTxData)
{
  DNPSESN *pDNPSession = (DNPSESN *)pTxData->pSession;
  TMWTARG_UNUSED_PARAM(pUserData);

  /* Diagnostics */
  DNPDIAG_ERROR_MSG(pTxData->pChannel, (TMWSESN *)pDNPSession, DNPDIAG_TRANSMIT_FAILED, (TMWTYPES_CHAR *)pTxData->pMsgDescription);

  /* Call session specific failed tx routine */
  if(pDNPSession->pFailedTxCallback != TMWDEFS_NULL)
      pDNPSession->pFailedTxCallback(pTxData);

  /* Call generic application error processing */
  _applError(pTxData, DNPCHNL_RESP_STATUS_FAILURE, TMWDEFS_TRUE);
}

/* function: _sendFragment */
static TMWTYPES_BOOL TMWDEFS_LOCAL _sendFragment(
  DNPCHNL_TX_DATA *pTxData)
{
  TMWCHNL *pChannel = pTxData->tmw.pChannel;
  TMWSESN *pSession = pTxData->tmw.pSession;

  /* Setup callbacks */
  pTxData->tmw.pCallbackData = TMWDEFS_NULL;
  pTxData->tmw.pBeforeTxCallback = _beforeTxCallback;
  pTxData->tmw.pAfterTxCallback = _afterTxCallback;
  pTxData->tmw.pFailedTxCallback = _failedTxCallback;

  /* Mark this request as sent */
  pTxData->sent = TMWDEFS_TRUE;
  
  /* Diagnostics */
  DNPDIAG_FRAGMENT_SENT(pChannel, pSession, pTxData->tmw.pMsgBuf, pTxData->tmw.msgLength);

  /* Send Fragment */
  pChannel->pTprt->pTprtTransmit(pChannel->pTprtContext, (TMWSESN_TX_DATA *)pTxData);
 
  /* Statistics */
  DNPSTAT_SESN_ASDU_SENT(pSession);

  /* All done */
  return(TMWDEFS_TRUE);
}

/* function: dnpchnl_processFragment */
void TMWDEFS_CALLBACK dnpchnl_processFragment(
  TMWSESN *pSession,
  TMWSESN_RX_DATA *pRxFragment)
{
  DNPSESN *pDNPSession = (DNPSESN *)pSession;

  /* Update stats */
  DNPSTAT_SESN_ASDU_RECEIVED(pSession);

  pRxFragment->invalidFormat = TMWDEFS_FALSE;

  /* Diagnostics */
  DNPDIAG_FRAGMENT_RECEIVED(pSession->pChannel, pSession,
    pRxFragment->pMsgBuf, pRxFragment->msgLength);

  /* Call function to process fragment*/
  if(pDNPSession->pProcessFragmentFunc(pSession, pRxFragment))
  {
    /* A session is online whenever it receives a valid fragment */
    tmwsesn_setOnline(pSession, TMWDEFS_TRUE);
  }
  else
  {
    /* Log an error */
    DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_PROC_FRAGMENT);
  }
}

/* function: _checkDataAvailable */
/* Check to see if there is anything to send on this session.
 * if so, send it and return true 
 */
static TMWTYPES_BOOL TMWDEFS_CALLBACK _checkDataAvailable(
  void *pParam,
  TMWSESN *pSession,
  TMWDEFS_CLASS_MASK classMask,
  TMWTYPES_BOOL buildResponse)
{
  DNPCHNL_TX_DATA *pTxData;
  TMWCHNL *pChannel;
  DNPCHNL *pDNPChannel;
  DNPSESN *pDNPSession;
  TMWTYPES_BOOL dataSent;

  TMWTARG_UNUSED_PARAM(classMask);
  TMWTARG_UNUSED_PARAM(buildResponse);

  pChannel = (TMWCHNL *)pParam;
  pDNPChannel = (DNPCHNL *)pChannel;

  dataSent = TMWDEFS_FALSE;

  pTxData = TMWDEFS_NULL;
  while((pTxData = (DNPCHNL_TX_DATA *)tmwdlist_getAfter(
    &pChannel->messageQueue, (TMWDLIST_MEMBER *)pTxData)) != TMWDEFS_NULL)
  {
    TMWTYPES_UCHAR fc;

    /* If this has already been sent, look at the next message. */
    if(pTxData->sent)
      continue;

    pDNPSession = (DNPSESN *)pTxData->tmw.pSession;

    /* Send application confirms, even if a request is outstanding.  
     * Otherwise for solicited multi fragment responses that require confirms, 
     * the request that caused this response will be pending (pCurrentMessage) 
     * on the channel until all of the response fragments are received. 
     */
    fc = pTxData->tmw.pMsgBuf[1];
    if(fc == DNPDEFS_FC_CONFIRM)
    { 
      /* Don't call prepareMessage since it does more than we want */
      /* Send the application confirm but continue so it looks for something
       * to send for the specified sessionm istead of using tge "time slot" 
       * for the specified session.
       */
      /* Need to prevent a queued read NOT overwriting appl confirm when link confirms are enabled 
       * However, if a read is already in progress and the Outstation requests an application
       * confirm, then we dont want to overwrite the read!
       */
      if(dnputil_getCurrentMessage((TMWSESN*)pDNPSession) == TMWDEFS_NULL)
        dnputil_setCurrentMessage((TMWSESN*)pDNPSession, (TMWSESN_TX_DATA *)pTxData);

      _sendFragment(pTxData);

      /* Application Confirms will likely be transmitted, removed from the queue, and deallocated.
        So start back at the beginning of the messageQueue. Application confirms have the highest
        priority so they are at the beginning of the queue anyway. It will be marked as sent=true
        if it is still on the queue meaning it will be skipped.
      */
      pTxData = TMWDEFS_NULL;
      continue;
    }

#if DNPCNFG_MULTI_SESSION_REQUESTS  
    TMWTARG_UNUSED_PARAM(pSession);

#if DNPCNFG_SUPPORT_AUTHENTICATION
    /* Send authentication requests, replies and errors even if waiting for a response from
     * outstation
     */
    if ((fc == DNPDEFS_FC_AUTH_REQUEST)
      || (fc == DNPDEFS_FC_AUTH_REQUEST_NOACK)
      || (fc == DNPDEFS_FC_AUTH_RESPONSE))
    {
      pDNPSession->pPrepareMessage((TMWSESN*)pDNPSession, (TMWSESN_TX_DATA *)pTxData);
      /* Authentication may be sent and removed from the queue similar
       * to application confirms above
       */
      _sendFragment(pTxData);
      pTxData = TMWDEFS_NULL;

      dataSent = TMWDEFS_TRUE;
      continue;
    }
#endif

    /* Masters can only send requests if no other master requests
     * are outstanding on this Association/session.
     * Slaves don't have this limitation.
     */
    if (pDNPSession->tmw.type == TMWTYPES_SESSION_TYPE_MASTER)
    {
      if (pDNPSession->pCurrentMessage != TMWDEFS_NULL)
      {
        continue;
      }

      /* separate this so I can set a break point for testing. */
      if ((pDNPSession->tmw.online == TMWSESN_STAT_OFFLINE)
        && tmwtimer_isActive(&pDNPChannel->channelOffLineDelayTimer))
      {
        continue;
      }
    }

    /* Check to see if it is OK to transmit */
    if ((pDNPSession->pCheckData != TMWDEFS_NULL) && (!pDNPSession->pCheckData((TMWSESN *)pDNPSession)))
    {
      continue;
    }

    pDNPSession->pPrepareMessage((TMWSESN *)pDNPSession, (TMWSESN_TX_DATA *)pTxData);

    pDNPChannel->pLastSessionRequest = (TMWSESN *)pDNPSession;
    if ((pTxData->tmw.txFlags & TMWSESN_TXFLAGS_NO_RESPONSE) != 0)
    {
      _sendFragment(pTxData);
      /* broadcast may be sent and removed from the queue similar
       * to application confirms above
       */
      pTxData = TMWDEFS_NULL;
    }
    else
    {
      _sendFragment(pTxData);
    }

    dataSent = TMWDEFS_TRUE;
  
#else
#if DNPCNFG_SUPPORT_AUTHENTICATION
    /* Send authentication requests, replies and errors even if waiting for a response from
     * outstation
     */
    if ((fc == DNPDEFS_FC_AUTH_REQUEST)
      || (fc == DNPDEFS_FC_AUTH_REQUEST_NOACK)
      || (fc == DNPDEFS_FC_AUTH_RESPONSE))
    {
      pDNPSession->pPrepareMessage((TMWSESN*)pDNPSession, (TMWSESN_TX_DATA *)pTxData);
      return(_sendFragment(pTxData));
    }
#endif
    if (pTxData->tmw.pSession == pSession)
    {
      pDNPSession = (DNPSESN *)pSession;

      /* Masters can only send requests if no other master requests 
       * are outstanding on this channel.
       * Slaves can send even when a master has an outstanding request
       *  or when other slaves have outstanding responses
       */
      if (pSession->type == TMWTYPES_SESSION_TYPE_MASTER)
      {
        if (pChannel->pCurrentMessage != TMWDEFS_NULL)
        {
          return(TMWDEFS_FALSE);
        }

        /* separate this so I can set a break point for testing. */
        if ((pSession->online == TMWSESN_STAT_OFFLINE)
          && tmwtimer_isActive(&pDNPChannel->channelOffLineDelayTimer))
        {
          return(TMWDEFS_FALSE);
        }
      }

      /* Check to see if it is OK to transmit */
      if ((pDNPSession->pCheckData != TMWDEFS_NULL) && (!pDNPSession->pCheckData((TMWSESN *)pDNPSession)))
      {
        return(TMWDEFS_FALSE);
      }

      pDNPSession->pPrepareMessage((TMWSESN *)pDNPSession, (TMWSESN_TX_DATA *)pTxData);

      pDNPChannel->pLastSessionRequest = (TMWSESN *)pDNPSession;

      /* Return if one data is sent */
      return(_sendFragment(pTxData));
    }
#endif
  }
  return(dataSent);
}

/* function: dnpchnl_initConfig */
void TMWDEFS_GLOBAL dnpchnl_initConfig(
  DNPCHNL_CONFIG *pDNPConfig,
  DNPTPRT_CONFIG *pTprtConfig,
  DNPLINK_CONFIG *pLinkConfig,
  TMWPHYS_CONFIG *pPhysConfig)
{
  /* Initialize DNP Channel configuration parameters */
  pDNPConfig->rxFragmentSize = DNPCNFG_MAX_RX_FRAGMENT_LENGTH;
  pDNPConfig->txFragmentSize = DNPCNFG_MAX_TX_FRAGMENT_LENGTH;

  /* 2048 is recommended by spec. 
   * (This just sets the initial value, if configured, max value allows it)
   */
  if(pDNPConfig->rxFragmentSize > 2048)
    pDNPConfig->rxFragmentSize = 2048;
  if(pDNPConfig->txFragmentSize > 2048)
      pDNPConfig->txFragmentSize = 2048;
 
  pDNPConfig->channelResponseTimeout  = 10000;
  pDNPConfig->channelOffLineDelay     = 0;

  pDNPConfig->pStatCallback           = TMWDEFS_NULL;
  pDNPConfig->pStatCallbackParam      = TMWDEFS_NULL;
  pDNPConfig->pIdleCallback           = TMWDEFS_NULL;
  pDNPConfig->pIdleCallbackParam      = TMWDEFS_NULL;
  pDNPConfig->pAutoOpenCallback       = TMWDEFS_NULL;
  pDNPConfig->pAutoOpenCallbackParam  = TMWDEFS_NULL;

  pDNPConfig->maxQueueSize = 0;
  pDNPConfig->chnlDiagMask = TMWDIAG_ID_DEF_MASK;

  /* Call generic initialization routines */
  dnptprt_initConfig(pTprtConfig);
  dnplink_initConfig(pLinkConfig);
  tmwphys_initConfig(pPhysConfig);
}

/* function: dnpchnl_openChannel */
TMWCHNL * TMWDEFS_GLOBAL dnpchnl_openChannel(
  TMWAPPL *pApplContext,
  DNPCHNL_CONFIG *pDNPConfig,
  DNPTPRT_CONFIG *pTprtConfig,
  DNPLINK_CONFIG *pLinkConfig,
  TMWPHYS_CONFIG *pPhysConfig,
  void           *pIOConfig,
  TMWTARG_CONFIG *pTmwTargConfig)
{
  DNPCHNL *pChannel;

  /* Initialize memory management and diagnostics if not yet done */
  if(!tmwappl_getInitialized(TMWAPPL_INIT_DNP))
  {
    if(!dnpmem_init(TMWDEFS_NULL))
      return(TMWDEFS_NULL);

#if TMWCNFG_SUPPORT_DIAG
    dnpdiag_init();
#endif
    tmwappl_setInitialized(TMWAPPL_INIT_DNP);
  }

  if((pDNPConfig->txFragmentSize > DNPCNFG_MAX_TX_FRAGMENT_LENGTH)
    ||(pDNPConfig->rxFragmentSize > DNPCNFG_MAX_RX_FRAGMENT_LENGTH))
  {
    return TMWDEFS_NULL;
  }
  
  pChannel = (DNPCHNL *)dnpmem_alloc(DNPMEM_CHNL_TYPE);
  if(pChannel != TMWDEFS_NULL)
  { 
    pChannel->pLastSessionRequest = TMWDEFS_NULL;
    pChannel->pLastOfflineSessionRequest = TMWDEFS_NULL;
    pChannel->txFragmentSize = pDNPConfig->txFragmentSize; 
    pChannel->rxFragmentSize = pDNPConfig->rxFragmentSize; 
    pChannel->tmw.pUserCallbackMsg = TMWDEFS_NULL;
    pChannel->tmw.maxQueueSize = pDNPConfig->maxQueueSize;
    pChannel->tmw.incrementalTimeout = pDNPConfig->channelResponseTimeout;
    pChannel->channelOffLineDelay = pDNPConfig->channelOffLineDelay;
    
#if DNPCNFG_SUPPORT_AUTHENTICATION
    pChannel->directNoAckDelayTime = 500;
#if !DNPCNFG_MULTI_SESSION_REQUESTS
    pChannel->lastTxFragmentLength = 0;
    pChannel->lastUnsolTxFragmentLength = 0;
#endif
#endif

    tmwchnl_initChannel(pApplContext, (TMWCHNL *)pChannel,
      pDNPConfig->pStatCallback,     pDNPConfig->pStatCallbackParam,
      pDNPConfig->pIdleCallback,     pDNPConfig->pIdleCallbackParam,
      pDNPConfig->pAutoOpenCallback, pDNPConfig->pAutoOpenCallbackParam,
      TMWDEFS_TRUE,                  TMWCHNL_TYPE_DNP,
      pDNPConfig->chnlDiagMask);

    if(tmwphys_initChannel((TMWCHNL *)pChannel, pPhysConfig, pIOConfig, pTmwTargConfig))
    { 
      if(dnplink_initChannel((TMWCHNL *)pChannel, pLinkConfig))
      {
        if(dnptprt_initChannel((TMWCHNL *)pChannel, pTprtConfig))
        {
          tmwtimer_init(&pChannel->channelOffLineDelayTimer);
          
          pChannel->tmw.pTprt->pSetCallbacks(pChannel->tmw.pTprtContext,
            (TMWCHNL *)pChannel, _infoCallback, dnpchnl_processFragment, _checkDataAvailable);

          TMWTARG_LOCK_SECTION(&pApplContext->lock);

          tmwdlist_addEntry(&pApplContext->channels, (TMWDLIST_MEMBER *)pChannel);

          TMWTARG_UNLOCK_SECTION(&pApplContext->lock);

          return((TMWCHNL *)pChannel);
        }

        (void)dnplink_deleteChannel((TMWCHNL *)pChannel);
      }

      tmwphys_deleteChannel((TMWCHNL *)pChannel);
    }

    tmwchnl_deleteChannel((TMWCHNL *)pChannel);
    dnpmem_free(pChannel);
  }

  return(TMWDEFS_NULL);
}

/* function: dnpchnl_getChannelConfig  */
TMWTYPES_BOOL TMWDEFS_GLOBAL dnpchnl_getChannelConfig(
  TMWCHNL        *pChannel,
  DNPCHNL_CONFIG *pDNPConfig,
  DNPTPRT_CONFIG *pTprtConfig,
  DNPLINK_CONFIG *pLinkConfig,
  TMWPHYS_CONFIG *pPhysConfig)
{
  DNPCHNL *pDNPChannel = (DNPCHNL *)pChannel;
  DNPLINK_CONTEXT *pLinkContext = (DNPLINK_CONTEXT *)pChannel->pLinkContext;
 
  pDNPConfig->rxFragmentSize         = pDNPChannel->rxFragmentSize;
  pDNPConfig->txFragmentSize         = pDNPChannel->txFragmentSize;
  pDNPConfig->channelOffLineDelay    = pDNPChannel->channelOffLineDelay;

  pDNPConfig->maxQueueSize           = pChannel->maxQueueSize;
  pDNPConfig->channelResponseTimeout = pChannel->incrementalTimeout;
  pDNPConfig->pStatCallback          = pChannel->pStatCallbackFunc;
  pDNPConfig->pStatCallbackParam     = pChannel->pStatCallbackParam;
  pDNPConfig->pIdleCallback          = pChannel->pIdleCallbackFunc;
  pDNPConfig->pIdleCallbackParam     = pChannel->pIdleCallbackParam;
  pDNPConfig->pAutoOpenCallback      = pChannel->pAutoOpenCallback;
  pDNPConfig->pAutoOpenCallbackParam = pChannel->pAutoOpenCallbackParam;
  pDNPConfig->chnlDiagMask           = pChannel->chnlDiagMask;

  /* Nothing in transport layer config structure */
  TMWTARG_UNUSED_PARAM(pTprtConfig);

  pLinkConfig->confirmMode           = pLinkContext->confirmMode;
  pLinkConfig->confirmTimeout        = pLinkContext->confirmTimeout;
  pLinkConfig->maxRetries            = pLinkContext->maxRetries;
  pLinkConfig->offlinePollPeriod     = pLinkContext->tmw.offlinePollPeriod;
  pLinkConfig->rxFrameSize           = pLinkContext->rxFrameSize;
  pLinkConfig->txFrameSize           = pLinkContext->txFrameSize;
  pLinkConfig->rxFrameTimeout        = pLinkContext->rxFrameTimeout;
  pLinkConfig->networkType           = pLinkContext->networkType;

  tmwphys_getChannelConfig(pChannel, pPhysConfig);

  return(TMWDEFS_TRUE);
}

/* function: dnpchnl_setChannelConfig  */
TMWTYPES_BOOL TMWDEFS_GLOBAL dnpchnl_setChannelConfig(
  TMWCHNL        *pChannel,
  DNPCHNL_CONFIG *pDNPConfig,
  DNPTPRT_CONFIG *pTprtConfig,
  DNPLINK_CONFIG *pLinkConfig,
  TMWPHYS_CONFIG *pPhysConfig)
{
  DNPCHNL *pDNPChannel = (DNPCHNL *)pChannel;
  DNPLINK_CONTEXT *pLinkContext = (DNPLINK_CONTEXT *)pChannel->pLinkContext;

  TMWTYPES_USHORT newRxFrameSize = 0;
  TMWTYPES_USHORT newTxFrameSize = 0;
  TMWTYPES_USHORT newRxFragmentSize = 0;

  if((pDNPConfig->txFragmentSize > DNPCNFG_MAX_TX_FRAGMENT_LENGTH)
    ||(pDNPConfig->rxFragmentSize > DNPCNFG_MAX_RX_FRAGMENT_LENGTH))
  {
    return TMWDEFS_FALSE;
  }

  /* Check configuration
   * maximum is set by spec and limited by 1 octet length field.
   * minimum of 24 is not in spec, but anything that small is not useful
   */
  if ((pLinkConfig->rxFrameSize < 24)
    || (pLinkConfig->txFrameSize < 24)
    || (pLinkConfig->rxFrameSize > DNPCNFG_MAX_RX_FRAME_LENGTH)
    || (pLinkConfig->txFrameSize > DNPCNFG_MAX_TX_FRAME_LENGTH))
  {
    DNPDIAG_ERROR(pChannel, TMWDEFS_NULL, DNPDIAG_INVALID_FRSIZE);
    return(TMWDEFS_FALSE);
  }

  /* Lock channel */
  TMWTARG_LOCK_SECTION(&pChannel->lock);

  if (pLinkConfig->rxFrameSize != pLinkContext->rxFrameSize)
  {
    newRxFrameSize = pLinkConfig->rxFrameSize;
  }

  if (pDNPConfig->rxFragmentSize != pDNPChannel->rxFragmentSize)
  {
    newRxFragmentSize = pDNPConfig->rxFragmentSize;
  }

  if (pLinkConfig->txFrameSize != pLinkContext->txFrameSize)
  {
    newTxFrameSize = pLinkConfig->txFrameSize;
  }

  if (!dnptprt_reInitChannel(pChannel, newTxFrameSize, newRxFragmentSize))
  {
    /* Unlock channel */
    TMWTARG_UNLOCK_SECTION(&pChannel->lock);
    return(TMWDEFS_FALSE);
  }

  if (!dnplink_reInitChannel(pChannel, newRxFrameSize, newTxFrameSize))
  {
    /* Unlock channel */
    TMWTARG_UNLOCK_SECTION(&pChannel->lock);
    return(TMWDEFS_FALSE);
  }


  pDNPChannel->rxFragmentSize       = pDNPConfig->rxFragmentSize;
  pDNPChannel->txFragmentSize       = pDNPConfig->txFragmentSize; 
  pDNPChannel->channelOffLineDelay  = pDNPConfig->channelOffLineDelay;

  pChannel->maxQueueSize            = pDNPConfig->maxQueueSize;
  pChannel->incrementalTimeout      = pDNPConfig->channelResponseTimeout;
  pChannel->pStatCallbackFunc       = pDNPConfig->pStatCallback;
  pChannel->pStatCallbackParam      = pDNPConfig->pStatCallbackParam;
  pChannel->pIdleCallbackFunc       = pDNPConfig->pIdleCallback;
  pChannel->pIdleCallbackParam      = pDNPConfig->pIdleCallbackParam;
  pChannel->pAutoOpenCallback       = pDNPConfig->pAutoOpenCallback;
  pChannel->pAutoOpenCallbackParam  = pDNPConfig->pAutoOpenCallbackParam;
  pChannel->chnlDiagMask            = pDNPConfig->chnlDiagMask;

  /* Nothing in transport layer config structure */
  TMWTARG_UNUSED_PARAM(pTprtConfig);

  pLinkContext->confirmMode           = pLinkConfig->confirmMode;
  pLinkContext->confirmTimeout        = pLinkConfig->confirmTimeout;
  pLinkContext->maxRetries            = pLinkConfig->maxRetries;
  pLinkContext->tmw.offlinePollPeriod = pLinkConfig->offlinePollPeriod;
  pLinkContext->rxFrameSize           = pLinkConfig->rxFrameSize;
  pLinkContext->txFrameSize           = pLinkConfig->txFrameSize;
  pLinkContext->rxFrameTimeout        = pLinkConfig->rxFrameTimeout;
  pLinkContext->networkType           = pLinkConfig->networkType;

  tmwphys_setChannelConfig(pChannel, pPhysConfig);

  /* Unlock channel */
  TMWTARG_UNLOCK_SECTION(&pChannel->lock);

  return(TMWDEFS_TRUE);
}

#if DNPCNFG_SUPPORT_BINCONFIG
TMWTYPES_BOOL TMWDEFS_GLOBAL dnpchnl_getBinFileChannelValues(
    DNPCHNL_CONFIG *pChnlConfig,
    DNPLINK_CONFIG *pLinkConfig, 
    DNPBNCFG_FILEVALUES *pBinFileValues,
    TMWTYPES_BOOL sessionIsOutstation)
{

  /* section 1.3.2 */
  pLinkConfig->networkType = DNPLINK_NETWORK_NO_IP;
  if(pBinFileValues->fieldsUsed1_3 & CRTL_TYPEOFENDPOINT1_3_2)
  {
    if((pBinFileValues->typeOfEndpoint1_3 & ENDPOINTS_TCPINITIATING)
      || (pBinFileValues->typeOfEndpoint1_3 & ENDPOINTS_TCPLISTENING)
      || (pBinFileValues->typeOfEndpoint1_3 & ENDPOINTS_TCPDUAL))
    {
      pLinkConfig->networkType = DNPLINK_NETWORK_TCP_UDP;
    }
    else if(pBinFileValues->typeOfEndpoint1_3 & ENDPOINTS_UDPDATAGRAM)
    {
      pLinkConfig->networkType = DNPLINK_NETWORK_UDP_ONLY;
    }
  }

  /* section 1.4.5 */
  if(pBinFileValues->fieldsUsed1_4 & CRTL_SENDSCONFIRMEDUSERDATAFRAMES1_4_5)
  {
     if((pBinFileValues->sendsConfirmedUserDataFrames1_4 & SENDSCONFFRAMES_NEVER) == SENDSCONFFRAMES_NEVER)
      pLinkConfig->confirmMode = TMWDEFS_LINKCNFM_NEVER;
    else if((pBinFileValues->sendsConfirmedUserDataFrames1_4 & SENDSCONFFRAMES_SOMETIMES) == SENDSCONFFRAMES_SOMETIMES)
      pLinkConfig->confirmMode = TMWDEFS_LINKCNFM_SOMETIMES;
    else if((pBinFileValues->sendsConfirmedUserDataFrames1_4 & SENDSCONFFRAMES_ALWAYS) == SENDSCONFFRAMES_ALWAYS)
      pLinkConfig->confirmMode = TMWDEFS_LINKCNFM_ALWAYS;
  }

  /* section 1.4.6 */
  if(pBinFileValues->fieldsUsed1_4 & CRTL_DATALINKLAYERCONFIRMTIMEOUT1_4_6)
    pLinkConfig->confirmTimeout = pBinFileValues->dataLinkLayerConfirmTimeout1_4;

  /* section 1.4.7 */
  if(pBinFileValues->fieldsUsed1_4 & CRTL_MAXDATALINKRETRIES1_4_7)
    pLinkConfig->maxRetries = (TMWTYPES_UCHAR)pBinFileValues->maxDataLinkRetries1_4;
   
  /* if master and configuring using outstation device profile, the tx and rx sizes should be switched */
  if((!sessionIsOutstation)
    && (pBinFileValues->isOutStation))
  {
    /* section 1.4.8 */
    if(pBinFileValues->fieldsUsed1_4 & CRTL_MAXNUMOCTETSTXINDATALINKFRAME1_4_8)
      pLinkConfig->rxFrameSize = (TMWTYPES_USHORT)pBinFileValues->maxNumOctetsTxDataLinkFrame1_4;

    /* section 1.4.9 */
    if(pBinFileValues->fieldsUsed1_4 & CRTL_MAXNUMOCTETSRECEIVEDINDATALINKFRAME1_4_9)
      pLinkConfig->txFrameSize = (TMWTYPES_USHORT)pBinFileValues->maxNumOctetsRxDataLinkFrame1_4;

    /* section 1.5.1 */
    if(pBinFileValues->fieldsUsed1_5 & CRTL_MAXTRANSMITTEDFRAGMENTSIZE1_5_1)
      pChnlConfig->rxFragmentSize = (TMWTYPES_USHORT)pBinFileValues->maxTxFragmentSize1_5;

    /* section 1.5.3 */
    if(pBinFileValues->fieldsUsed1_5 & CRTL_MAXRECEIVEDFRAGMENTSIZE1_5_3)
      pChnlConfig->txFragmentSize = (TMWTYPES_USHORT)pBinFileValues->maxRxFragmentSize1_5;
  }
  else
  {
    /* section 1.4.8 */
    if(pBinFileValues->fieldsUsed1_4 & CRTL_MAXNUMOCTETSTXINDATALINKFRAME1_4_8)
      pLinkConfig->txFrameSize = (TMWTYPES_USHORT)pBinFileValues->maxNumOctetsTxDataLinkFrame1_4;

    /* section 1.4.9 */
    if(pBinFileValues->fieldsUsed1_4 & CRTL_MAXNUMOCTETSRECEIVEDINDATALINKFRAME1_4_9)
      pLinkConfig->rxFrameSize = (TMWTYPES_USHORT)pBinFileValues->maxNumOctetsRxDataLinkFrame1_4;

    /* section 1.5.1 */
    if(pBinFileValues->fieldsUsed1_5 & CRTL_MAXTRANSMITTEDFRAGMENTSIZE1_5_1)
      pChnlConfig->txFragmentSize = (TMWTYPES_USHORT)pBinFileValues->maxTxFragmentSize1_5;

    /* section 1.5.3 */
    if(pBinFileValues->fieldsUsed1_5 & CRTL_MAXRECEIVEDFRAGMENTSIZE1_5_3)
      pChnlConfig->rxFragmentSize = (TMWTYPES_USHORT)pBinFileValues->maxRxFragmentSize1_5;

    /* section 1.6.3 */
    if(!sessionIsOutstation)
      if(pBinFileValues->fieldsUsed1_6 & CRTL_APPLLAYERFRAGMENTRESPONSETIMEOUT1_6_3)
        pChnlConfig->channelResponseTimeout = pBinFileValues->applLayerFragRespTimeout1_6;
  }

  return TMWDEFS_TRUE;
}
#endif

/* function: dnpchnl_modifyPhys */
TMWTYPES_BOOL TMWDEFS_GLOBAL dnpchnl_modifyPhys(
  TMWCHNL *pChannel,
  const TMWPHYS_CONFIG *pPhysConfig,
  TMWTYPES_ULONG configMask)
{
  return(tmwphys_modifyChannel(pChannel, pPhysConfig, configMask));
}

/* function: dnpchnl_modifyLink */
TMWTYPES_BOOL TMWDEFS_GLOBAL dnpchnl_modifyLink(
  TMWCHNL *pChannel,
  const DNPLINK_CONFIG *pLinkConfig,
  TMWTYPES_ULONG configMask)
{
  return(dnplink_modifyChannel(pChannel, pLinkConfig, configMask));
}

/* function: dnpchnl_modifyTprt */
TMWTYPES_BOOL TMWDEFS_GLOBAL dnpchnl_modifyTprt(
  TMWCHNL *pChannel,
  const DNPTPRT_CONFIG *pTprtConfig,
  TMWTYPES_ULONG configMask)
{
  return(dnptprt_modifyChannel(pChannel, pTprtConfig, configMask));
}

TMWTYPES_BOOL TMWDEFS_GLOBAL dnpchnl_modifyChannel(
  TMWCHNL *pChannel,
  const DNPCHNL_CONFIG *pConfig,
  TMWTYPES_ULONG configMask)
{
  TMWTARG_UNUSED_PARAM(pChannel);
  TMWTARG_UNUSED_PARAM(pConfig);
  TMWTARG_UNUSED_PARAM(configMask);
  return(TMWDEFS_TRUE);
}

/* function: dnpchnl_closeChannel */
TMWTYPES_BOOL TMWDEFS_GLOBAL dnpchnl_closeChannel(
  TMWCHNL *pChannel)
{
  DNPCHNL_TX_DATA *pTxData; 
  TMWAPPL *pApplContext;
  DNPCHNL *pDNPChannel;
  
  /* Check for NULL or sessions still open since these would be common errors */
  if ((pChannel == TMWDEFS_NULL)
    || (pChannel->pLinkContext->sessions.size > 0))
  {
    return(TMWDEFS_FALSE);
  }
    
  /* Prevent the channel from trying to reconnect while it is being closed. */
  pChannel->pPhysContext->active = TMWDEFS_FALSE;

  tmwtarg_stopThreads(pChannel->pPhysContext->pIOContext);

  pApplContext = pChannel->pApplContext;

  /* Lock application list of channels */
  TMWTARG_LOCK_SECTION(&pApplContext->lock);

  /* Lock channel */
  TMWTARG_LOCK_SECTION(&pChannel->lock);

  /* cancel the incremental timer in case it is running */
  tmwtimer_cancel(&pChannel->incrementalTimer);

  pDNPChannel = (DNPCHNL *)pChannel;
  tmwtimer_cancel(&pDNPChannel->channelOffLineDelayTimer);

  /* Remove any Tx Datas left on the queue, cancelling the responseTimers */
  while((pTxData = (DNPCHNL_TX_DATA *)tmwdlist_getFirst(&pChannel->messageQueue)) != TMWDEFS_NULL)
  {
    tmwtimer_cancel(&pTxData->tmw.responseTimer);
    if(tmwdlist_removeEntry(&pChannel->messageQueue, (TMWDLIST_MEMBER *)pTxData))
    {
#if DNPCNFG_SUPPORT_AUTHENTICATION
      /* let mdnpauth free authentication request */
      if((pTxData->tmw.txFlags & TMWSESN_TXFLAGS_DNP_AUTH) == 0)
#endif
        dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
    }
  }

  /* Close link layer */
  if(dnplink_deleteChannel(pChannel) == TMWDEFS_FALSE)
  {
    /* Unlock channel */
    TMWTARG_UNLOCK_SECTION(&pChannel->lock);
    TMWTARG_UNLOCK_SECTION(&pApplContext->lock);
    return(TMWDEFS_FALSE);
  } 

  /* Remove channel from list of channels */
  if (tmwdlist_removeEntry(&pApplContext->channels, (TMWDLIST_MEMBER *)pChannel))
  {
    /* Close transport layer */
    if (dnptprt_deleteChannel(pChannel))
    {
      /* Close physical layer */
      tmwphys_deleteChannel(pChannel);

      /* Unlock channel */
      TMWTARG_UNLOCK_SECTION(&pChannel->lock);

      /* Free channel resources, including deleting lock */
      tmwchnl_deleteChannel(pChannel);
    
      /* Free memory */
      dnpmem_free((DNPCHNL *)pChannel); 

      TMWTARG_UNLOCK_SECTION(&pApplContext->lock);
      return(TMWDEFS_TRUE);
    }
  }

  /* Unlock channel */
  TMWTARG_UNLOCK_SECTION(&pChannel->lock);
  TMWTARG_UNLOCK_SECTION(&pApplContext->lock);
  return(TMWDEFS_FALSE);
}

/* function: dnpchnl_sendFragment */
TMWTYPES_BOOL TMWDEFS_GLOBAL dnpchnl_sendFragment(
  TMWSESN_TX_DATA *pTxData)
{
  DNPCHNL_TX_DATA *pDNPTxData = (DNPCHNL_TX_DATA *)pTxData;
  TMWCHNL *pChannel = pTxData->pChannel;
  TMWSESN *pSession = pTxData->pSession;
  DNPCHNL_TX_DATA *pRequest = TMWDEFS_NULL;

  /* Lock channel */
  TMWTARG_LOCK_SECTION(&pChannel->lock);
  
#ifdef TMW_SUPPORT_MONITOR
  /* If in analyzer or listen only mode return failure */
  if(pChannel->pPhysContext->monitorMode)
  {
    /* Unlock channel */
    TMWTARG_UNLOCK_SECTION(&pChannel->lock);
    return(TMWDEFS_FALSE);
  }
#endif
  
  /* Diagnostics */
  DNPDIAG_INSERT_QUEUE(pChannel, pSession, pTxData->pMsgDescription);
 
  /* If there is a configured limit on the number of outstanding requests */
  if((pChannel->maxQueueSize > 0) 
    &&(tmwdlist_size(&pChannel->messageQueue) >= pChannel->maxQueueSize))
  {
    DNPDIAG_ERROR(pChannel, pSession, DNPDIAG_TOO_MANY_REQUESTS);

    /* Unlock channel */
    TMWTARG_UNLOCK_SECTION(&pChannel->lock);

    return(TMWDEFS_FALSE);
  }

  /* First check to make sure this is not a duplicate request */
  while((pRequest = (DNPCHNL_TX_DATA *)tmwdlist_getAfter(
    &pChannel->messageQueue, (TMWDLIST_MEMBER *)pRequest)) != TMWDEFS_NULL)
  {
    TMWTYPES_BOOL matched = TMWDEFS_TRUE;
    int i = 0;

    if(pRequest->tmw.pChannel != pTxData->pChannel)
      continue;

    if(pRequest->tmw.pSession != pTxData->pSession)
      continue;

    if(pRequest->tmw.msgLength != pTxData->msgLength)
      continue;

    if(pRequest->pUserCallback != pDNPTxData->pUserCallback)
      continue;

    if(pRequest->pInternalCallback != pDNPTxData->pInternalCallback)
      continue;

    /* Skip application control byte in comparison */
    for(i = 1; i < pTxData->msgLength; i++)
    {
      if(pRequest->tmw.pMsgBuf[i] != pTxData->pMsgBuf[i])
      {
        matched = TMWDEFS_FALSE;
        break;
      }
    }

    if(!matched)
      continue;

    /* Allow duplicate request only if it is the same as the one has been sent
     */
    if(pRequest->sent)
      continue;

    /* Duplicate, remove the old one on the queue that matches 
     * keeping just the new one will guarantee the correct command
     * is done last.
     */
    // if(!((DNPSESN *)pSession)->suppressError && ((TMWSESN *)pSession)->online == 1)
    // {
    //   DNPDIAG_ERROR(pChannel, pSession, DNPDIAG_DUPLICATE_REQ);
    // }

    /* Make priority of new request as high as priority of request
     * being deleted 
     */
    if(pDNPTxData->priority < pRequest->priority)
      pDNPTxData->priority = pRequest->priority;
  
    /* Remove this request from queue */
    _removeRequest((TMWSESN_TX_DATA *)pRequest);
 
    if ((((DNPCHNL_TX_DATA *)pTxData)->pInternalCallback != TMWDEFS_NULL)
      || (((DNPCHNL_TX_DATA *)pTxData)->pUserCallback != TMWDEFS_NULL))
    {
     /* Initialize user callback info */
      DNPCHNL_RESPONSE_INFO response;
      response.iin = 0;
      response.last = TMWDEFS_TRUE;
      response.pSession = pRequest->tmw.pSession;
      response.pTxData = (TMWSESN_TX_DATA *)pRequest;
      response.pRxData = TMWDEFS_NULL;
      response.status = DNPCHNL_RESP_STATUS_CANCELED;
      response.requestStatus = 0;
      response.responseTime = 0;

      /* Call internal callback if specified */
      if (pRequest->pInternalCallback != TMWDEFS_NULL)
      {
        pRequest->pInternalCallback(pRequest->pInternalCallbackParam, &response);
      }

      dnpchnl_userCallback(pChannel, pRequest, &response);
    }

#if DNPCNFG_SUPPORT_AUTHENTICATION
    /* let mdnpauth free authentication request */
    if((pTxData->txFlags & TMWSESN_TXFLAGS_DNP_AUTH) == 0)
#endif
      dnpchnl_freeTxData((TMWSESN_TX_DATA *)pRequest);

    /* Only one duplicate that has not been sent is possible. */
    break;
  }

  /* Add entry to queue in priority order */
  pRequest = TMWDEFS_NULL;
  while((pRequest = (DNPCHNL_TX_DATA *)tmwdlist_getAfter(
    &pChannel->messageQueue, (TMWDLIST_MEMBER *)pRequest)) != TMWDEFS_NULL)
  {
    if(pDNPTxData->priority > pRequest->priority)
      break;
  }

  /* See if we are at the end of the queue */
  if(pRequest != TMWDEFS_NULL)
  {
    /* No, insert before next entry */
    tmwdlist_insertEntryBefore(&pChannel->messageQueue,
      (TMWDLIST_MEMBER *)pRequest, (TMWDLIST_MEMBER *)pTxData);
  }
  else
  {
    /* Yep, insert at end of queue */
    tmwdlist_addEntry(&pChannel->messageQueue,
      (TMWDLIST_MEMBER *)pTxData);
  }

  pTxData->timeSent = tmwtarg_getMSTime();

  /* Start response timer now */
  if(pTxData->responseTimeout != 0)
  {
    tmwtimer_start(&pTxData->responseTimer,
      pTxData->responseTimeout, pChannel, _responseTimeout, pTxData);
  }

#if !DNPCNFG_MULTI_SESSION_REQUESTS
  /* Tell transport layer there is something to transmit */
  if(pChannel->pLinkContext->sessions.size == 1)
  {
    pChannel->pTprt->pTprtDataReady(pChannel->pTprtContext, pSession);
  }
  else
  { 
    TMWSESN *pStartSession;
    DNPCHNL *pDNPChannel = (DNPCHNL*)pChannel;
    pSession = pStartSession = pDNPChannel->pLastSessionRequest;
    do 
    {
        pSession = (TMWSESN *)tmwdlist_getAfter(&pChannel->pLinkContext->sessions, (TMWDLIST_MEMBER *)pSession);
      if(pSession == TMWDEFS_NULL)
      {
        if(pSession == pStartSession)
        {
          break;  
        }
        else
        {
          pSession = (TMWSESN *)tmwdlist_getFirst(&pChannel->pLinkContext->sessions);
        }
      }
      pChannel->pTprt->pTprtDataReady(pChannel->pTprtContext, pSession);
    }
    while(pSession != pStartSession);
  }
#else
  pChannel->pTprt->pTprtDataReady(pChannel->pTprtContext, pSession);
#endif
 
  /* Unlock channel */
  TMWTARG_UNLOCK_SECTION(&pChannel->lock);

  return(TMWDEFS_TRUE);
}

/* function: dnpchnl_cancelFragment */
TMWTYPES_BOOL TMWDEFS_GLOBAL dnpchnl_cancelFragment(
  TMWSESN_TX_DATA *pTxData)
{
  TMWTYPES_BOOL status = TMWDEFS_FALSE;

  if(pTxData->structureValid == DNPCHNL_TXDATA_VALID)
  {
    /* Lock channel */
#if TMWCNFG_SUPPORT_THREADS
    TMWDEFS_RESOURCE_LOCK *pLock = &pTxData->pSession->pChannel->lock;
    TMWTARG_LOCK_SECTION(pLock);
#endif

    /* while waiting for the lock in a multithreaded environment, this got deleted */
    if(pTxData->structureValid != DNPCHNL_TXDATA_VALID)
    {
      TMWTARG_UNLOCK_SECTION(pLock);
      return TMWDEFS_TRUE;
    }

    /* If this is being cancelled from user callback, just mark it cancelled
     * Cancel processing will be done on return from user callback.
     */
    if(pTxData->pSession->pChannel->pUserCallbackMsg == pTxData)
    {
      pTxData->txFlags |= TMWSESN_TXFLAGS_CANCEL;
    }
    else
    {
      /* Diagnostics */
      DNPDIAG_ERROR_MSG(pTxData->pSession->pChannel, pTxData->pSession, DNPDIAG_REQUEST_CANCELED, 
        (TMWTYPES_CHAR *)pTxData->pMsgDescription);

      _applError(pTxData, DNPCHNL_RESP_STATUS_CANCELED, TMWDEFS_FALSE);
    }

    status = TMWDEFS_TRUE;
  
    /* Unlock channel */
    TMWTARG_UNLOCK_SECTION(pLock);
  }

  return(status);
}

/* function: dnpchnl_okToSend */
void TMWDEFS_GLOBAL dnpchnl_okToSend(
  TMWCHNL *pChannel)
{ 
  /* See if there is anything else in the queue */
  if(tmwdlist_size(&pChannel->messageQueue) != 0)
  {
    /* Tell link layer we have a frame to transmit 
     * Link layer will ask each session if it has something to send
     */
    pChannel->pLink->pLinkDataReady(pChannel->pLinkContext, TMWDEFS_NULL);
  }
}

/* function: dnpchnl_checkForChannelIdle */
void TMWDEFS_GLOBAL dnpchnl_checkForChannelIdle(
  TMWCHNL *pChannel)
{
  /* If there is an idle callback function registered, determine if the
   * channel (including all sessions) is idle and if so call the callback function.
   */
  if(pChannel->pIdleCallbackFunc != TMWDEFS_NULL)
  {
   if(tmwdlist_size(&pChannel->messageQueue) == 0)
   {
     /* Ask all of the sessions if they are busy */ 
     TMWSESN *pSession = TMWDEFS_NULL;
     while((pSession = (TMWSESN *)tmwdlist_getAfter(
       &pChannel->pLinkContext->sessions, (TMWDLIST_MEMBER *)pSession)) != TMWDEFS_NULL)
     {
       DNPSESN *pDNPSession = (DNPSESN *)pSession;
       if((pDNPSession->pIdleFunc != TMWDEFS_NULL) &&
         !(pDNPSession->pIdleFunc(pSession)))
       {
         /* This session on this channel is not idle */
         return;
       }
     }

     /* All of the Sessions on this channel are idle */
     pChannel->pIdleCallbackFunc(pChannel->pIdleCallbackParam);
   }
  }
}

/* function: dnpchnl_deleteFragments */
TMWDEFS_GLOBAL void dnpchnl_deleteFragments(
  TMWSESN *pSession)
{
  DNPCHNL_TX_DATA *pTxData;
  DNPCHNL_TX_DATA *pPrevTxData = TMWDEFS_NULL;
  TMWCHNL *pChannel = pSession->pChannel;

  /* Remove all of the Tx Datas for this session
   * from the queue, cancelling the responseTimers
   */
  while((pTxData = (DNPCHNL_TX_DATA *)tmwdlist_getAfter(&pChannel->messageQueue, (TMWDLIST_MEMBER *)pPrevTxData))
    != TMWDEFS_NULL)
  {
    if(pTxData->tmw.pSession == pSession)
    {
      tmwtimer_cancel(&pTxData->tmw.responseTimer);

      /* Tell transport layer that we have given up on this message. */
      pChannel->pTprt->pTprtCancel(pChannel->pTprtContext, (TMWSESN_TX_DATA *)pTxData);
 
      _removeRequest((TMWSESN_TX_DATA *)pTxData);
      
#if DNPCNFG_SUPPORT_AUTHENTICATION
      /* let mdnpauth contact user and free authentication request */
      if((pTxData->tmw.txFlags & TMWSESN_TXFLAGS_DNP_AUTH) != 0)
      { 
        continue;
      }
#endif

      if((pTxData->pInternalCallback != TMWDEFS_NULL)
        || (pTxData->pUserCallback != TMWDEFS_NULL))
      {
        /* Initialize user callback info */
        DNPCHNL_RESPONSE_INFO response;
        response.iin = 0;
        response.last = TMWDEFS_TRUE;
        response.pSession = pSession;
        response.pTxData = (TMWSESN_TX_DATA *)pTxData;
        response.pRxData = TMWDEFS_NULL;
        response.status = DNPCHNL_RESP_STATUS_CANCELED;
        response.requestStatus = 0;
        response.responseTime = 0;

        /* Call internal callback if specified */
        if(pTxData->pInternalCallback != TMWDEFS_NULL)
        {
          pTxData->pInternalCallback(pTxData->pInternalCallbackParam, &response);
        }

        /* Call user callback if specified */
        dnpchnl_userCallback(pChannel, pTxData, &response);
      }
      dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
    }
    else
    {
      pPrevTxData = pTxData;
    }
  }

  /* See if there is anything else in the queue */
  if(tmwdlist_size(&pChannel->messageQueue) != 0)
  {
    /* Tell link layer we have a frame to transmit 
     * Link layer will ask each session if it has something to send
     */
    pChannel->pLink->pLinkDataReady(pChannel->pLinkContext, TMWDEFS_NULL);
  }
  else
  {
    dnpchnl_checkForChannelIdle(pChannel);
  }
}

/* function: dnpchnl_userCallback */
TMWDEFS_GLOBAL void dnpchnl_userCallback(
  TMWCHNL *pChannel,
  DNPCHNL_TX_DATA *pTxData, 
  DNPCHNL_RESPONSE_INFO *pResponse)
{
  if(pTxData->pUserCallback != TMWDEFS_NULL)
  {
    pChannel->pUserCallbackMsg = (TMWSESN_TX_DATA *)pTxData;
    pTxData->pUserCallback(pTxData->pUserCallbackParam, pResponse);
    pChannel->pUserCallbackMsg = TMWDEFS_NULL;

    /* User cancelled request from inside of callback function
     * processing of cancel had to be delayed
     */
    if(pTxData->tmw.txFlags & TMWSESN_TXFLAGS_CANCEL)
    {
      pResponse->status = DNPCHNL_RESP_STATUS_CANCELED;
      if(pResponse->last)
      { 
        /* Since we said that was the last callback, SCL can't call it again
         * when cancelling the request
         */
        pTxData->pUserCallback = TMWDEFS_NULL;
      }
    }
  }
}

/* function: dnpchnl_newTxData */
TMWSESN_TX_DATA * TMWDEFS_GLOBAL dnpchnl_newTxData(
  TMWCHNL *pChannel,
  TMWSESN *pSession,
  TMWTYPES_USHORT bufLen,
  TMWTYPES_USHORT destAddress)
{
  DNPCHNL_TX_DATA *pTxData;
  TMWTYPES_UCHAR *pBuf;
  DNPCHNL *pDNPChannel;

  /* If pSession is NULL this is a broadcast request, 
   * find first master DNP session to use 
   */
  if(pSession == TMWDEFS_NULL)
  {
    TMWDLIST *pList = pChannel->pLink->pLinkGetSessions(pChannel->pLinkContext);
    while((pSession = (TMWSESN *)tmwdlist_getAfter(pList, (TMWDLIST_MEMBER *)pSession)) != TMWDEFS_NULL)
    {
      if(pSession->type == TMWTYPES_SESSION_TYPE_MASTER)
        break;
    }
    /* If we could not find a master session just return failure */
    if(pSession == TMWDEFS_NULL)
    {
      DNPDIAG_ERROR(pChannel, pSession, DNPDIAG_MASTER_SESSION);
      return(TMWDEFS_NULL);
    }
  }
  
  /* limit max length to configured tx fragment size */
  pDNPChannel = (DNPCHNL*)pSession->pChannel;
  if(bufLen > pDNPChannel->txFragmentSize) 
    return(TMWDEFS_NULL);
 
  pTxData = (DNPCHNL_TX_DATA *)dnpmem_alloc(DNPMEM_CHNL_TX_DATA_TYPE);
  if(pTxData == TMWDEFS_NULL)
  {
    DNPDIAG_ERROR(pChannel, pSession, DNPDIAG_ALLOC_TX);
    ASSERT(TMWDEFS_FALSE);
    return(TMWDEFS_NULL);
  }

#if TMWCNFG_USE_DYNAMIC_MEMORY && !TMWCNFG_ALLOC_ONLY_AT_STARTUP 
  pBuf = (TMWTYPES_UCHAR *)tmwtarg_alloc(bufLen);
  if(pBuf == TMWDEFS_NULL)
  {
    DNPDIAG_ERROR(pChannel, pSession, DNPDIAG_ALLOC_FRAG);
    ASSERT(TMWDEFS_FALSE);
    dnpmem_free(pTxData);
    return(TMWDEFS_NULL);
  }
#else 
  if(bufLen > DNPCNFG_MAX_TX_FRAGMENT_LENGTH)
  {
    DNPDIAG_ERROR(pChannel, pSession, DNPDIAG_INVALID_BUF_SIZE);
    ASSERT(TMWDEFS_FALSE);
    dnpmem_free(pTxData);
    return(TMWDEFS_NULL);
  }

  pBuf = pTxData->buffer;
#endif

  tmwsesn_initTxData((TMWSESN_TX_DATA *)pTxData, pBuf, bufLen);

  pTxData->tmw.structureValid = DNPCHNL_TXDATA_VALID;
  pTxData->tmw.pMsgDescription = TMWDEFS_NULL;
  pTxData->tmw.pChannel = pSession->pChannel;
  pTxData->tmw.pSession = pSession;

  pTxData->tmw.destAddress = destAddress;
  pTxData->tmw.txFlags = 0;

  pTxData->tmw.responseTimeout = 0;
  tmwtimer_init(&pTxData->tmw.responseTimer);

  pTxData->sent = TMWDEFS_FALSE;

  pTxData->dnpTxFlags = 0;
  pTxData->priority = DNPCHNL_DEFAULT_PRIORITY;
  pTxData->referenceCount = 1;
	
  pTxData->pInternalCallback = TMWDEFS_NULL;
  pTxData->pInternalCallbackParam = TMWDEFS_NULL;

  pTxData->pUserCallback = TMWDEFS_NULL;
  pTxData->pUserCallbackParam = TMWDEFS_NULL;

  pTxData->authAggressiveMode = TMWDEFS_FALSE; 
  pTxData->authUserNumber = DNPAUTH_DEFAULT_USERNUMBER;

  return((TMWSESN_TX_DATA *)pTxData);
}

/* function: dnpchnl_freeTxData */
void TMWDEFS_GLOBAL dnpchnl_freeTxData(
  TMWSESN_TX_DATA *pTxData)
{
  DNPCHNL_TX_DATA *pDNPData = (DNPCHNL_TX_DATA*)pTxData;
  tmwtimer_cancel(&pTxData->responseTimer);

  pDNPData->referenceCount--;
  if(pDNPData->referenceCount == 0)
  {
    pTxData->structureValid = 0;

#if TMWCNFG_USE_DYNAMIC_MEMORY && !TMWCNFG_ALLOC_ONLY_AT_STARTUP 
    tmwtarg_free(pTxData->pMsgBuf);
#endif

    dnpmem_free(pTxData);
  }
}
