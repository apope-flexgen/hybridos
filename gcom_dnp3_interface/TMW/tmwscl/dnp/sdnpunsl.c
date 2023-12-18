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

/* file: sdnpunsl.c
 * description: Slave DNP Unsolicited Response Support.
 */
#include "tmwscl/dnp/dnpdiag.h"
#include "tmwscl/utils/tmwtimer.h"

#include "tmwscl/dnp/dnpdefs.h"
#include "tmwscl/dnp/dnpstat.h"
#include "tmwscl/dnp/dnpchnl.h"
#include "tmwscl/dnp/sdnpmem.h"
#include "tmwscl/dnp/sdnprbe.h"
#include "tmwscl/dnp/sdnputil.h"
#include "tmwscl/dnp/sdnpdiag.h"
#include "tmwscl/dnp/sdnpo002.h"
#include "tmwscl/dnp/sdnpo004.h"
#include "tmwscl/dnp/sdnpo011.h"
#include "tmwscl/dnp/sdnpo013.h"
#include "tmwscl/dnp/sdnpo022.h"
#include "tmwscl/dnp/sdnpo023.h"
#include "tmwscl/dnp/sdnpo032.h"
#include "tmwscl/dnp/sdnpo033.h"
#include "tmwscl/dnp/sdnpo042.h"
#include "tmwscl/dnp/sdnpo043.h"
#include "tmwscl/dnp/sdnpo070.h"
#include "tmwscl/dnp/sdnpo088.h"
#include "tmwscl/dnp/sdnpo111.h"
#include "tmwscl/dnp/sdnpo113.h"
#include "tmwscl/dnp/sdnpo115.h"
#if SDNPCNFG_SUPPORT_SA_VERSION5
#include "tmwscl/dnp/sdnpo122.h"
#endif
#include "tmwscl/dnp/sdnpsesn.h"
#include "tmwscl/dnp/sdnpunsl.h"
#include "tmwscl/dnp/sdnpdata.h"
#if DNPCNFG_SUPPORT_AUTHENTICATION
#include "tmwscl/dnp/sdnpauth.h"
#endif

/* This table specifies which objects should be included in an
 * unsolicited response.
 */
typedef struct UnsolEventFuncStruct {
  SDNPSESN_READ_EVENTS_FUNC pReadFunc;
} UNSOL_EVENT_FUNC;

static const UNSOL_EVENT_FUNC _unsolEventGroups[] = {
  
#if SDNPDATA_SUPPORT_OBJ2
  {sdnpo002_readObj2v0ByClass},
#endif
#if SDNPDATA_SUPPORT_OBJ4
  {sdnpo004_readObj4v0ByClass},
#endif
#if SDNPDATA_SUPPORT_OBJ11
  {sdnpo011_readObj11v0ByClass},
#endif
#if SDNPDATA_SUPPORT_OBJ13
  {sdnpo013_readObj13v0ByClass},
#endif
#if SDNPDATA_SUPPORT_OBJ22
  {sdnpo022_readObj22v0ByClass},
#endif
#if SDNPDATA_SUPPORT_OBJ23
  {sdnpo023_readObj23v0ByClass},
#endif
#if SDNPDATA_SUPPORT_OBJ32
  {sdnpo032_readObj32v0ByClass},
#endif
#if SDNPDATA_SUPPORT_OBJ33
  {sdnpo033_readObj33v0ByClass},
#endif
#if SDNPDATA_SUPPORT_OBJ42
  {sdnpo042_readObj42v0ByClass},
#endif
#if SDNPDATA_SUPPORT_OBJ43
  {sdnpo043_readObj43v0ByClass},
#endif
#if SDNPDATA_SUPPORT_OBJ70
  {sdnpo070_readObj70v0ByClass},
#endif
#if SDNPDATA_SUPPORT_OBJ88
  {sdnpo088_readObj88v0ByClass},
#endif
#if SDNPDATA_SUPPORT_OBJ111
  {sdnpo111_readObj111ByClass},
#endif
#if SDNPDATA_SUPPORT_OBJ113
  {sdnpo113_readObj113ByClass},
#endif
#if SDNPDATA_SUPPORT_OBJ115
  {sdnpo115_readObj115ByClass},
#endif
#if SDNPDATA_SUPPORT_OBJ120
  /* SPM should sdnpo120_readObj120ByClass be here */
#if SDNPCNFG_SUPPORT_SA_VERSION5
  {sdnpo122_readobj122v0ByClass},
#endif
#endif
  {TMWDEFS_NULL}
};

/* function: _initializeResponse
 * purpose: initialize an application layer response
 * arguments:
 *  pSession - session to respond to
 *  pResponse - response data
 * returns:
 *  void
 */
static void TMWDEFS_LOCAL _initializeResponse(
  TMWSESN *pSession,
  TMWSESN_TX_DATA *pResponse,
  TMWTYPES_BOOL initialNullUnsol)
{
  SDNPSESN *pSDNPSession;
  TMWTYPES_UCHAR control;

#if !SDNPDATA_SUPPORT_OBJ120
  TMWTARG_UNUSED_PARAM(initialNullUnsol);
#endif

  pSDNPSession = (SDNPSESN *)pSession;
  /* Application Control */
  control = pSDNPSession->unsolSequenceNumber;
  pSDNPSession->lastUnsolSequenceNumber = control;

  control |= DNPDEFS_AC_UNSOLICITED | DNPDEFS_AC_FIRST | DNPDEFS_AC_FINAL | DNPDEFS_AC_CONFIRM;

  pResponse->pMsgBuf[0] = control;

  /* Function Code */
  pResponse->pMsgBuf[1] = DNPDEFS_FC_UNSOLICITED;

  /* IIN bits */
  pResponse->pMsgBuf[2] = (TMWTYPES_UCHAR)((pSDNPSession->iin & 0xff00) >> 8);
  pResponse->pMsgBuf[3] = (TMWTYPES_UCHAR)(pSDNPSession->iin & 0x00ff);

  /* Misc initialization */
  pResponse->txFlags = TMWSESN_TXFLAGS_UNSOL;

  /* Set message length */
  pResponse->msgLength = 4;
  pResponse->responseTimeout = pSDNPSession->unsolConfirmTimeout;
  
  /* Increment sequence number */
  pSDNPSession->unsolSequenceNumber =
    sdnputil_nextSequenceNumber(pSDNPSession->unsolSequenceNumber);

  DNPDIAG_BUILD_MESSAGE(pResponse->pChannel, (TMWSESN*)pSDNPSession, pResponse->pMsgDescription); 
  
#if SDNPDATA_SUPPORT_OBJ120
  /* If configured to send aggressive mode unsolicited responses, and this is not an initial null response 
   * SAV5 3.2.3.2 says not in initial unsolicited response
   */
  if(pSDNPSession->authenticationEnabled && pSDNPSession->authSendAggrUnsol && (!initialNullUnsol))
  {  
    /* Leave some room at end for aggressive mode MAC object */
    pResponse->maxLength -= SDNPAUTH_AGGRMODEPAD;
    sdnpauth_addAggrModeStart(pSDNPSession, pResponse); 
  }
#endif 
}

/* function: _processUnsolTimeout 
 * purpose: Handle unsolicited event timeouts. The unsolicited event
 *  timer starts whenever a qualifying event is generated. If this
 *  timer times out then the maximum configured time for holding
 *  unsolicited events has expired and the events should be sent.
 */
static void TMWDEFS_CALLBACK _processUnsolTimeout(
  void *pParam)
{
  UNSOL_DELAY_TIMER *pDelayTimerParam = (UNSOL_DELAY_TIMER *)pParam;
  TMWSESN *pSession = pDelayTimerParam->pSession;
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;

  /* Set flag saying that unsolicited events are ready to send */
  pSDNPSession->unsolEventsReady = TMWDEFS_TRUE;

  /* Attempt to send the events */
  sdnpunsl_processUnsolEvents(pSession, pDelayTimerParam->classMask);
}

/* function: _sendFragment */
static TMWTYPES_BOOL TMWDEFS_LOCAL _sendFragment(TMWSESN_TX_DATA *pResponse)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pResponse->pSession;

  /* dnpchnl_sendFragment can fail in target layer and call txFailed callback, but still return true 
   */
  pSDNPSession->unsolQueued = TMWDEFS_TRUE;

  /* Send the unsolicited response */
  if(!dnpchnl_sendFragment(pResponse))
  {
    pSDNPSession->unsolQueued = TMWDEFS_FALSE;
    dnpchnl_freeTxData(pResponse);
    return TMWDEFS_FALSE;
  }
  return TMWDEFS_TRUE;
}

/* function: sdnpunsl_sendNullResponse */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpunsl_sendNullResponse(
  TMWSESN *pSession,
  TMWTYPES_BOOL initialNullResponse)
{
#if SDNPDATA_SUPPORT_OBJ120 && SDNPCNFG_SUPPORT_SA_VERSION2 || SDNPDATA_SUPPORT_IDENT_UNSOL_RETRY
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
#endif

#if SDNPDATA_SUPPORT_IDENT_UNSOL_RETRY

  /* If we still have the last unsolicited response we can send
   * send it as an identical retry 
   */
  if(pSDNPSession->pUnsolLastResponse != TMWDEFS_NULL)
  {
    TMWSESN_TX_DATA *pResponse = (TMWSESN_TX_DATA *)pSDNPSession->pUnsolLastResponse;
    if(pResponse->msgLength == 4)
    {
      /* resend */
      pSDNPSession->pUnsolLastResponse->sent = TMWDEFS_FALSE;
      pSDNPSession->pUnsolLastResponse->tmw.txFlags = TMWSESN_TXFLAGS_UNSOL;
      pSDNPSession->pUnsolLastResponse = TMWDEFS_NULL; 

      /* This was an Unsolicited Null response, send it again. */
      return(_sendFragment(pResponse));
    }
    else
    {
      /* This was an event filled response, don't send it, send null response below. 
       * This occurs only when configured for sendUnsolWhenOnline==true and reconnect
       * while an event filled response was available for retry.
       */
      sdnprbe_cleanupEvents(pSession, TMWDEFS_FALSE);
    }
  }
#endif
  {
    DNPCHNL *pDNPChannel = (DNPCHNL *)pSession->pChannel;

    TMWSESN_TX_DATA *pResponse = dnpchnl_newTxData((TMWCHNL *)pDNPChannel, 
      pSession, pDNPChannel->txFragmentSize, pSession->destAddress);
   
    if(pResponse == TMWDEFS_NULL)
    {
      return(TMWDEFS_FALSE);
    }

#if TMWCNFG_SUPPORT_DIAG
   pResponse->pMsgDescription = "Unsol NULL Response";
#endif
   _initializeResponse(pSession, pResponse, initialNullResponse);

   /* SAv5 (1815-2012 7.5.2.3.2 bullet h) says this is not allowed.
    * This was not disallowed in SAv2. 
    */
#if SDNPDATA_SUPPORT_OBJ120 && SDNPCNFG_SUPPORT_SA_VERSION2
    if(pSDNPSession->authenticationEnabled 
      &&((DNPSESN*)pSession)->operateInV2Mode
        && pSDNPSession->preChallengeApplConf)
    { 
      sdnpauth_addPreChallengeV1(pSDNPSession, pResponse);
    }
#endif
    
    return(_sendFragment(pResponse));
  }
}

/* function: sdnpunsl_addEvent */
void TMWDEFS_GLOBAL sdnpunsl_addEvent(
  TMWSESN *pSession, 
  TMWDEFS_CLASS_MASK classMask)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;

  /* If this event qualifies as an unsolicited event update
   * unsolicited delay and number of events
   */
  TMWDEFS_CLASS_MASK mask = classMask & pSDNPSession->unsolEventMask;
  if(mask != 0)
  {
    /* Which class is it. 
     * Note: this allows multiple bits to match
     */
    int index = 0; 
    if(mask & TMWDEFS_CLASS_MASK_ONE)
      index = 0;
    else if(mask & TMWDEFS_CLASS_MASK_TWO)
      index = 1;
    else if(mask & TMWDEFS_CLASS_MASK_THREE)
      index = 2;
    
    /* Increment the number of pending events in this class */
    pSDNPSession->unsolNumPending[index] += 1;

    /* Don't bother checking if the unsolicited requirements have already
     * been met.
     * This could actually restart the max delay timer which would
     * confuse us later.
     * If an event is added while waiting for the appl confirm, allow starting the unsolDelayTimer.
     * however, if the appl confirm is not received, this unsolDelayTimer should be cancelled
     * so it does not delay the old events (and the new ones).
     */
    if(!pSDNPSession->unsolEventsReady)
    {
      /* If the max delay timeout is 0 go head and send the events */
      if(pSDNPSession->unsolMaxDelay[index] == 0)
      {
        /* Set flag saying that unsolicited events are ready to send */
        pSDNPSession->unsolEventsReady = TMWDEFS_TRUE;
      }
      else if(pSDNPSession->unsolNumPending[index] >= pSDNPSession->unsolMaxEvents[index])
      {
        /* Set flag saying that unsolicited events are ready to send */
        pSDNPSession->unsolEventsReady = TMWDEFS_TRUE;
      }
      else if(!tmwtimer_isActive(&pSDNPSession->unsolDelayTimer[index]))
      {
        /* Set unsolicited delay timer. This timer will time out when the 
         * maximum delay has expired after receiving a qualifying event
         */
        tmwtimer_start(&pSDNPSession->unsolDelayTimer[index], 
          pSDNPSession->unsolMaxDelay[index], pSession->pChannel,
          _processUnsolTimeout, &pSDNPSession->unsolDelayTimerParam[index]);

        DNPSTAT_SESN_UNSOL_TIMER_START(pSession, mask, pSDNPSession->unsolMaxDelay[index]);
      }
    }

    /* Process events if ready */
    sdnpunsl_processUnsolEvents(pSession, classMask);
  }
}

/* function: sdnpunsl_removeEvent */
void TMWDEFS_GLOBAL sdnpunsl_removeEvent(
  SDNPSESN *pSDNPSession,
  SDNPEVNT *pEvent)
{
  TMWDEFS_CLASS_MASK mask = pEvent->classMask;
  if (mask != 0)
  {
    /* Which class is it.
     * Note: this allows multiple bits to match
     */
    int index = 0;
    if (mask & TMWDEFS_CLASS_MASK_ONE)
      index = 0;
    else if (mask & TMWDEFS_CLASS_MASK_TWO)
      index = 1;
    else if (mask & TMWDEFS_CLASS_MASK_THREE)
      index = 2;

    /* Decrement the number of pending events in this class 
     * Check for greater than zero to be safe since this gets set to zero when events are sent 
     */
    if(pSDNPSession->unsolNumPending[index]>0)
      pSDNPSession->unsolNumPending[index] -= 1;
  }
}

 static TMWDEFS_CLASS_MASK indexToMask[] = {
    TMWDEFS_CLASS_MASK_ONE, 
    TMWDEFS_CLASS_MASK_TWO, 
    TMWDEFS_CLASS_MASK_THREE
  };

/* function: sdnpunsl_countUnsolicitedEvents */
TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpunsl_countUnsolicitedEvents(
  TMWSESN *pSession,
  TMWDEFS_CLASS_MASK classMask)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  TMWTYPES_USHORT totalEvents = 0;
  TMWDEFS_CLASS_MASK curMask;
  int index;

  /* Count the number of unsolicited events in the queue */
  for(index = 0; index < TMWDEFS_CLASS_MAX; index++)
  {
    curMask = indexToMask[index];
    if (curMask & classMask)
    {
      pSDNPSession->unsolNumPending[index] = 0;
      if (pSDNPSession->unsolEventMask & curMask)
      {
        pSDNPSession->unsolNumPending[index] = (TMWTYPES_USHORT)
          (pSDNPSession->unsolNumPending[index] + sdnprbe_countEvents(pSession, curMask, TMWDEFS_FALSE, 255));
      }

      totalEvents = (TMWTYPES_USHORT)(totalEvents + pSDNPSession->unsolNumPending[index]);
    }
  }

  return(totalEvents);
}

/* function: sdnpunsl_updateStatus */
void TMWDEFS_GLOBAL sdnpunsl_updateStatus(
  TMWSESN *pSession)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  int i;

#if SDNPDATA_SUPPORT_IDENT_UNSOL_RETRY
  /* For each class check to see if there is a delay timer running */
  for(i = 0; i < TMWDEFS_CLASS_MAX; i++)
  if(pSDNPSession->pUnsolLastResponse != TMWDEFS_NULL)
  {
    if(!tmwtimer_isActive(&pSDNPSession->unsolDelayTimer[i]))
    {
      pSDNPSession->unsolEventsReady = TMWDEFS_TRUE;
      return;
    }
  }
#endif

  /* Count qualifying events currently in event queues */
  sdnpunsl_countUnsolicitedEvents(pSession, TMWDEFS_CLASS_MASK_ALL);

  /* Assume no events are ready to send */
  pSDNPSession->unsolEventsReady = TMWDEFS_FALSE;

  /* For each class check to see if there are events pending */
  for(i = 0; i < TMWDEFS_CLASS_MAX; i++)
  {
    if(pSDNPSession->unsolNumPending[i] > 0)
    {
      /* If the unsolicited max delay timer has expired than the event(s)
       * must have occurred long enough in the past for the max delay time
       * to expire and hence should be sent. Alternatively if there are 
       * more than max events queued the events should be sent.
       */
      if(!tmwtimer_isActive(&pSDNPSession->unsolDelayTimer[i])
        || (pSDNPSession->unsolNumPending[i] >= pSDNPSession->unsolMaxEvents[i]))
      {
        pSDNPSession->unsolEventsReady = TMWDEFS_TRUE;
      }
    }
  }
}

/* function: sdnpunsl_processUnsolEvents */
void TMWDEFS_GLOBAL sdnpunsl_processUnsolEvents(
  TMWSESN *pSession,
  TMWDEFS_CLASS_MASK classMask)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  DNPCHNL *pDNPChannel = (DNPCHNL *)pSession->pChannel;
  DNPLINK_CONTEXT *pLinkContext = (DNPLINK_CONTEXT *)pDNPChannel->tmw.pLinkContext; 
  TMWSESN_TX_DATA *pResponse = TMWDEFS_NULL;
  TMWDEFS_CLASS_MASK curMask;
  int index;

  /* Are events ready */
  if(!pSDNPSession->unsolEventsReady)
    return;

  /* Can't send unsolicited events until initial null confirmed */
  if(!pSDNPSession->unsolInitialNullConfirmed)
    return;

  /* Can't send unsolicited events if we are currently in retry mode */
  if(tmwtimer_isActive(&pSDNPSession->unsolRetryTimer))
    return;

  /* Can't send unsolicited if application fragment buffer is in use 
   * or we are waiting for a confirmation from a previous unsolicited
   * response or are in the middle of processing a command such as a 
   * counter freeze which could call addEvent.
   */
  if((pSDNPSession->dnp.pCurrentMessage != TMWDEFS_NULL)
    || pSDNPSession->unsolWaitingForConfirm
    || pSDNPSession->pendingRequest)
  {
    return;
  }

  /* If not connected don't try to send unsolicited response */
  if(!pLinkContext->tmw.isOpen)
  {
    sdnpsesn_failedUnsolTx(pSession);

    /* Diagnostics */
    SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_APPL_TX_UNSOL);

    return;
  }

  /* If not configured to send unsolicited's by class, then process all classes. */
  if (!pSDNPSession->unsolSendByClass)
  {
    classMask = TMWDEFS_CLASS_MASK_ALL;
  }

  /* Cancel unsolicited delay timer since we are going to send the events */
  for (index = 0; index < TMWDEFS_CLASS_MAX; index++)
  {
    curMask = indexToMask[index];
    if (curMask & classMask)
      tmwtimer_cancel(&pSDNPSession->unsolDelayTimer[index]);
  }

#if SDNPDATA_SUPPORT_IDENT_UNSOL_RETRY
  /* If we still have the last unsolicited response we can send
   * send it as an identical retry 
   */
  if(pSDNPSession->pUnsolLastResponse != TMWDEFS_NULL)
  {
    pResponse = (TMWSESN_TX_DATA *)pSDNPSession->pUnsolLastResponse;
    /* resend */
    pSDNPSession->pUnsolLastResponse->sent = TMWDEFS_FALSE;
    pSDNPSession->pUnsolLastResponse->tmw.txFlags = (TMWSESN_TXFLAGS_UNSOL | TMWSESN_TXFLAGS_CONTAINS_EVENTS);
    pSDNPSession->pUnsolLastResponse = TMWDEFS_NULL; 
  }
  else
#endif
  {
    /* Make sure we actually have events to send */
    if(sdnpunsl_countUnsolicitedEvents(pSession, classMask) > 0)
    {
      DNPUTIL_OBJECT_HEADER objHeader;
      int i;

      /* Fill in a fake object header */
      objHeader.firstPointNumber = 0;
      objHeader.group = DNPDEFS_OBJ_2_BIN_CHNG_EVENTS;
      objHeader.numberOfPoints = 0;
      objHeader.qualifier = DNPDEFS_QUAL_ALL_POINTS;
      objHeader.variation = 0;

      /* Initialize response message */
      pResponse = dnpchnl_newTxData((TMWCHNL *)pDNPChannel, 
        pSession, pDNPChannel->txFragmentSize, pSession->destAddress);
   
      if(pResponse == TMWDEFS_NULL)
      {
        return;
      }

#if TMWCNFG_SUPPORT_DIAG
      pResponse->pMsgDescription = "Unsolicited Response";
#endif

      _initializeResponse(pSession, pResponse, TMWDEFS_FALSE);

#if SDNPDATA_SUPPORT_OBJ120
      if (pSDNPSession->authenticationEnabled && pSDNPSession->preChallengeApplConf)
       /* Leave some room at end for the challenge object */
        pResponse->maxLength -= 30; 
#endif

      /* Loop through unsolicited event groups, adding events as we go */
      i = 0;
      while(_unsolEventGroups[i].pReadFunc != TMWDEFS_NULL)
      {
        /* Read events into response */
        if(_unsolEventGroups[i].pReadFunc(pSession, TMWDEFS_NULL, pResponse,
          &objHeader, (pSDNPSession->unsolEventMask & classMask)) != SDNPSESN_READ_COMPLETE)
        {
          /* Unable to fit all events in message so break. We'll process the rest
           * later.
           */
          break;
        }

        i++;
      }

      /* Update IIN bits to reflect new event status */
      sdnprbe_updateIINBits(pSession);
      
#if SDNPDATA_SUPPORT_OBJ120
      if (pSDNPSession->authenticationEnabled && pSDNPSession->preChallengeApplConf)
      {
        /* Set max length back for the challenge object */
        pResponse->maxLength += 30; 
        sdnpauth_addPreChallengeV1(pSDNPSession, pResponse);
      }
#endif
    }
    else
    {
      return;
    }
  }

  /* No more events are ready to send
   * Set this here in case the failedTxCallback occurs while in _sendFragment 
   * which would then cause this to be set to FALSE incorrectly.
   */
  pSDNPSession->unsolEventsReady = TMWDEFS_FALSE;

  /* Send the unsolicited response */
  if (!_sendFragment(pResponse))
  {
    /* There are still messages to send. */
    pSDNPSession->unsolEventsReady = TMWDEFS_TRUE;
    sdnprbe_cleanupEvents(pSession, TMWDEFS_FALSE);
    sdnpsesn_failedUnsolTx(pSession);
    SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_APPL_TX);
    return;
  }

  /* For now assume there are no unsolicited events remaining, we will
   * update this after we receive the confirmation.
   */
  for (index = 0; index < TMWDEFS_CLASS_MAX; index++)
    pSDNPSession->unsolNumPending[index] = 0;
}
