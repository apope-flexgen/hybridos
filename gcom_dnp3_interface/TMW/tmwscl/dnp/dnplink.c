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

/* file: dnplink.c
 * description: DNP link layer implementation
 */
#include "tmwscl/dnp/dnpdiag.h"
#include "tmwscl/utils/tmwtarg.h"
#include "tmwscl/utils/tmwscl.h"
#include "tmwscl/utils/tmwtimer.h"
#include "tmwscl/utils/tmwdlist.h"
#include "tmwscl/utils/tmwcnfg.h"

#include "tmwscl/dnp/dnplink.h"
#include "tmwscl/dnp/dnpchnl.h"
#include "tmwscl/dnp/dnpdefs.h"
#include "tmwscl/dnp/dnpmem.h"
#include "tmwscl/dnp/dnpstat.h"

/* Local function declarations */
static void TMWDEFS_LOCAL _checkForData(
  DNPLINK_CONTEXT *pLinkContext,
  TMWSESN *pFirstSession);

/* function: _linkStatusTimeout */
static void TMWDEFS_CALLBACK _linkStatusTimeout(
  void *pCallbackParam);

/* function: _getSessions
 * purpose: return list of sessions on this channel
 * arguments:
 *  pContext - link layer context
 * returns:
 *  pointer to list of sessions
 */
static TMWDLIST * TMWDEFS_CALLBACK _getSessions(
  TMWLINK_CONTEXT *pContext)
{
  return(tmwlink_getSessions(pContext));
}

/* function: _infoCallback
 * purpose: send TMWSCL_INFO to the layer above. If redundancy is not
 *  enabled, perform some basic retry strategy first to hide details
 *  from application layer. If redundancy is enabled just pass the
 *  information up and let it decide what to do.
 * arguments:
 *  pContext - pointer to link context structure
 *  pSession - pointer to session structure
 *  sclInfo - event such as OFFLINE
 * returns
 *  void
 */
static void TMWDEFS_CALLBACK _infoCallback(
  TMWLINK_CONTEXT *pContext,
  TMWCHNL *pChannel,
  TMWSCL_INFO sclInfo)
{
  DNPLINK_CONTEXT *pLinkContext = (DNPLINK_CONTEXT *)pContext;
  TMWSESN *pSession1 = TMWDEFS_NULL;
  TMWTARG_UNUSED_PARAM(pChannel);

  /* If there is an application layer info callback */
  if(pLinkContext->tmw.pInfoFunc != TMWDEFS_NULL)
  {
    switch(sclInfo)
    {
    case TMWSCL_INFO_OPENED:
      /* Send channel opened to application layer */
      pLinkContext->tmw.pInfoFunc(
        pLinkContext->tmw.pCallbackParam,
        TMWDEFS_NULL, sclInfo);

      /* Send online to all current sessions */
      while((pSession1 = (TMWSESN *)tmwdlist_getAfter(
        &pContext->sessions, (TMWDLIST_MEMBER *)pSession1)) != TMWDEFS_NULL)
      {
        DNPSESN * pDNPSession = (DNPSESN *)pSession1;

        pLinkContext->tmw.pInfoFunc(
          pLinkContext->tmw.pCallbackParam,
          pSession1, TMWSCL_INFO_ONLINE);
 
        /* Indicate session is online, if this is TCP */  
        if((pLinkContext->networkType == DNPLINK_NETWORK_TCP_ONLY)
          || (pLinkContext->networkType == DNPLINK_NETWORK_TCP_UDP))
        { 
          tmwsesn_setOnline(pSession1, TMWDEFS_TRUE);
        }
        
        if(pDNPSession->linkStatusPeriod != 0)
        {
          DNPLINK_SESSION_INFO *pLinkSessionInfo =
           (DNPLINK_SESSION_INFO *)pSession1->pLinkSession;

          tmwtimer_start(&pLinkSessionInfo->linkStatusTimer,
            pDNPSession->linkStatusPeriod, pSession1->pChannel, _linkStatusTimeout, pSession1);
        }

      }
      _checkForData(pLinkContext, TMWDEFS_NULL);

      break;

    case TMWSCL_INFO_CLOSED:
      /* Send offline to all current sessions */
      while((pSession1 = (TMWSESN *)tmwdlist_getAfter(
        &pContext->sessions, (TMWDLIST_MEMBER *)pSession1)) != TMWDEFS_NULL)
      {
        DNPLINK_SESSION_INFO *pLinkSessionInfo =
         (DNPLINK_SESSION_INFO *)pSession1->pLinkSession;

        pLinkSessionInfo->linkIsReset = TMWDEFS_FALSE;
        pLinkSessionInfo->secondaryStationIsReset = TMWDEFS_FALSE;
        pLinkSessionInfo->resetLinkWait = TMWDEFS_FALSE;
        pLinkSessionInfo->expectedFCB = TMWDEFS_FALSE;
        pLinkSessionInfo->nextFCB = TMWDEFS_FALSE;

        pLinkContext->tmw.pInfoFunc(
          pLinkContext->tmw.pCallbackParam,
          pSession1, TMWSCL_INFO_OFFLINE);

        /* Session is offline */
        tmwsesn_setOnline(pSession1, TMWDEFS_FALSE);

        /* Send channel closed to application layer */
        pLinkContext->tmw.pInfoFunc(
          pLinkContext->tmw.pCallbackParam,
          pSession1, sclInfo);
      }

      break;

    default:
      /* Something other than channel opened or closed, pass it on to application layer */
      pLinkContext->tmw.pInfoFunc(pLinkContext->tmw.pCallbackParam, TMWDEFS_NULL, sclInfo);
      break;
    }
  }
}

/* function: _computeCRC
 * purpose: calculates 16 bit crc value using Polynomial
 *  x^16+x^13+x^12+x^11+x^10+x^8+x^6+x^5+x^2+1
 *  (P2 per IEEE-870-1) via lookup table.
 * arguments:
 *  pBuf - buffer to calculate CRC for
 *  count - number of bytes in buffer excluding CRC
 * returns
 *  void
 */
static TMWTYPES_USHORT TMWDEFS_LOCAL _computeCRC(
  TMWTYPES_UCHAR *pBuf,
  TMWTYPES_USHORT length)
{
  static const TMWTYPES_USHORT CrcTable[256] =
  {
    0x0000, 0x365e, 0x6cbc, 0x5ae2, 0xd978, 0xef26, 0xb5c4, 0x839a,
    0xff89, 0xc9d7, 0x9335, 0xa56b, 0x26f1, 0x10af, 0x4a4d, 0x7c13,
    0xb26b, 0x8435, 0xded7, 0xe889, 0x6b13, 0x5d4d, 0x07af, 0x31f1,
    0x4de2, 0x7bbc, 0x215e, 0x1700, 0x949a, 0xa2c4, 0xf826, 0xce78,
    0x29af, 0x1ff1, 0x4513, 0x734d, 0xf0d7, 0xc689, 0x9c6b, 0xaa35,
    0xd626, 0xe078, 0xba9a, 0x8cc4, 0x0f5e, 0x3900, 0x63e2, 0x55bc,
    0x9bc4, 0xad9a, 0xf778, 0xc126, 0x42bc, 0x74e2, 0x2e00, 0x185e,
    0x644d, 0x5213, 0x08f1, 0x3eaf, 0xbd35, 0x8b6b, 0xd189, 0xe7d7,
    0x535e, 0x6500, 0x3fe2, 0x09bc, 0x8a26, 0xbc78, 0xe69a, 0xd0c4,
    0xacd7, 0x9a89, 0xc06b, 0xf635, 0x75af, 0x43f1, 0x1913, 0x2f4d,
    0xe135, 0xd76b, 0x8d89, 0xbbd7, 0x384d, 0x0e13, 0x54f1, 0x62af,
    0x1ebc, 0x28e2, 0x7200, 0x445e, 0xc7c4, 0xf19a, 0xab78, 0x9d26,
    0x7af1, 0x4caf, 0x164d, 0x2013, 0xa389, 0x95d7, 0xcf35, 0xf96b,
    0x8578, 0xb326, 0xe9c4, 0xdf9a, 0x5c00, 0x6a5e, 0x30bc, 0x06e2,
    0xc89a, 0xfec4, 0xa426, 0x9278, 0x11e2, 0x27bc, 0x7d5e, 0x4b00,
    0x3713, 0x014d, 0x5baf, 0x6df1, 0xee6b, 0xd835, 0x82d7, 0xb489,
    0xa6bc, 0x90e2, 0xca00, 0xfc5e, 0x7fc4, 0x499a, 0x1378, 0x2526,
    0x5935, 0x6f6b, 0x3589, 0x03d7, 0x804d, 0xb613, 0xecf1, 0xdaaf,
    0x14d7, 0x2289, 0x786b, 0x4e35, 0xcdaf, 0xfbf1, 0xa113, 0x974d,
    0xeb5e, 0xdd00, 0x87e2, 0xb1bc, 0x3226, 0x0478, 0x5e9a, 0x68c4,
    0x8f13, 0xb94d, 0xe3af, 0xd5f1, 0x566b, 0x6035, 0x3ad7, 0x0c89,
    0x709a, 0x46c4, 0x1c26, 0x2a78, 0xa9e2, 0x9fbc, 0xc55e, 0xf300,
    0x3d78, 0x0b26, 0x51c4, 0x679a, 0xe400, 0xd25e, 0x88bc, 0xbee2,
    0xc2f1, 0xf4af, 0xae4d, 0x9813, 0x1b89, 0x2dd7, 0x7735, 0x416b,
    0xf5e2, 0xc3bc, 0x995e, 0xaf00, 0x2c9a, 0x1ac4, 0x4026, 0x7678,
    0x0a6b, 0x3c35, 0x66d7, 0x5089, 0xd313, 0xe54d, 0xbfaf, 0x89f1,
    0x4789, 0x71d7, 0x2b35, 0x1d6b, 0x9ef1, 0xa8af, 0xf24d, 0xc413,
    0xb800, 0x8e5e, 0xd4bc, 0xe2e2, 0x6178, 0x5726, 0x0dc4, 0x3b9a,
    0xdc4d, 0xea13, 0xb0f1, 0x86af, 0x0535, 0x336b, 0x6989, 0x5fd7,
    0x23c4, 0x159a, 0x4f78, 0x7926, 0xfabc, 0xcce2, 0x9600, 0xa05e,
    0x6e26, 0x5878, 0x029a, 0x34c4, 0xb75e, 0x8100, 0xdbe2, 0xedbc,
    0x91af, 0xa7f1, 0xfd13, 0xcb4d, 0x48d7, 0x7e89, 0x246b, 0x1235
  };

  TMWTYPES_USHORT crc = 0;

  while(length--)
  {
    crc = (TMWTYPES_USHORT)((crc >> 8) ^ (CrcTable[((crc^*pBuf++) & 0x00ff)]));
  }

  /* AND with 0xffff to mask off top bits for TM320 32 bit only data types */
  return((TMWTYPES_USHORT)(~crc & 0xffff));
}

/* function: _checkCRC
 * purpose: check 2 byte CRC in block of data
 * arguments:
 *  pBuf - buffer to check
 *  count - number of bytes in buffer including CRC
 * returns
 *  TMWDEFS_TRUE if CRC is good, else TMWDEFS_FALSE
 */
static TMWTYPES_BOOL TMWDEFS_LOCAL _checkCRC(
  TMWTYPES_UCHAR *pBuf,
  TMWTYPES_USHORT count)
{
  TMWTYPES_USHORT crc;

  /* trap call with empty buffer */
  if(count == 0)
    return(TMWDEFS_TRUE);
  /* trap call with bad data */
  else if(count < 3)
    return(TMWDEFS_FALSE);

  count -=2;

  crc = _computeCRC(pBuf, count);
  pBuf += count;

  if(*pBuf++ != (TMWTYPES_UCHAR)(crc & 0x00ff))
    return(TMWDEFS_FALSE);

  if(*pBuf != (TMWTYPES_UCHAR)(crc >> 8))
    return(TMWDEFS_FALSE);

  return(TMWDEFS_TRUE);
}

/* function: _setCRC
 * purpose: set 2 byte CRC in block of data
 * arguments:
 *  pBuf - buffer to set CRC in
 *  count - number of bytes in buffer excluding CRC
 * returns
 *  void
 */
static void TMWDEFS_LOCAL _setCRC(
  TMWTYPES_UCHAR *pBuf,
  TMWTYPES_USHORT length)
{
  TMWTYPES_USHORT crc;
  crc = _computeCRC(pBuf, length);

  *(pBuf + length) = (TMWTYPES_UCHAR)(crc & 0x00ff);
  *(pBuf + length + 1) = (TMWTYPES_UCHAR)(crc >> 8);

  return;
}

static void TMWDEFS_LOCAL _checkForData(
  DNPLINK_CONTEXT *pLinkContext,
  TMWSESN *pFirstSession)
{
  TMWSESN *pSession;
  TMWTYPES_BOOL dataSent;
  TMWTYPES_UINT size;

  /* First see if we are not already trying to transmit something */
  if((pLinkContext->pTxDescriptor != TMWDEFS_NULL)
    ||(pLinkContext->pTxSession != TMWDEFS_NULL))
    return;  

  /* Then see if any sessions have something to send, 
   * starting with specified session 
   */
  dataSent = TMWDEFS_FALSE;

  size = tmwdlist_size(&pLinkContext->tmw.sessions);
  if (size > 1)
  {
    /* If no session was requested, round robin starting after last session that sent */
    DNPCHNL *pDNPChannel = (DNPCHNL *)pLinkContext->tmw.pChannel;
    if (pFirstSession == TMWDEFS_NULL)
    {
      pFirstSession = (TMWSESN *)tmwdlist_getAfter(&pLinkContext->tmw.sessions,
        (TMWDLIST_MEMBER *)pDNPChannel->pLastSessionRequest);
    }
  }

  pSession = pFirstSession;
  while(size--)
  {
    DNPLINK_SESSION_INFO *pLinkSessionInfo;

    if(pSession == TMWDEFS_NULL)
    {
      pSession = (TMWSESN *)tmwdlist_getFirst(&pLinkContext->tmw.sessions);
    }

    /* See if user asked for linkReset and we had to delay the transmit */
    pLinkSessionInfo = (DNPLINK_SESSION_INFO *)pSession->pLinkSession;
    if(pLinkSessionInfo->sendUserLinkReset)
    { 
      pLinkSessionInfo->sendUserLinkReset = TMWDEFS_FALSE;
      dnplink_linkReset(pSession);
      return;
    }

    /* Send all we can */
    while(pLinkContext->tmw.pCheckClassFunc(
      pLinkContext->tmw.pCallbackParam, pSession, TMWDEFS_CLASS_MASK_ALL, TMWDEFS_TRUE))
    {
      dataSent = TMWDEFS_TRUE; 
      /* If transmit is still in progress break out of loop */
      if((pLinkContext->pTxDescriptor != TMWDEFS_NULL)
        ||(pLinkContext->pTxSession != TMWDEFS_NULL))
        break;
    }
    if(dataSent)
      return;
       
    /* Get next session */
    pSession = (TMWSESN *)tmwdlist_getAfter(
      &pLinkContext->tmw.sessions, (TMWDLIST_MEMBER *)pSession);
  }

  if(!dataSent)
  {
    if(((TMWCHNL *)pLinkContext->tmw.pChannel)->pIdleCallbackFunc != TMWDEFS_NULL)
    {
      dnpchnl_checkForChannelIdle((TMWCHNL *)pLinkContext->tmw.pChannel);
    }
  }
}

/* function: _restartFrame
 * purpose: Search through remaining bytes of a frame to see if we can find a new
 *  start frame character.
 */
static void TMWDEFS_LOCAL _restartFrame(
  DNPLINK_CONTEXT *pLinkContext,
  TMWTYPES_USHORT blockSize)
{
  TMWTYPES_USHORT i;
  for(i = 1; i < blockSize; i++)
  {
    if(pLinkContext->pRxBlock[i] == 0x05)
    {
      /* Avoid memcpy since it might do weird things with in place copies */
      TMWTYPES_USHORT j;
      for(j = i; j < blockSize; j++)
        pLinkContext->pRxBlock[j - i] = pLinkContext->pRxBlock[j];

      pLinkContext->rxState = DNP_LINK_STATE_HEADER;
      pLinkContext->rxBlockOffset = (TMWTYPES_USHORT)(blockSize - i);

      return;
    }
  }
}

/* function: _finishDataFrame
 * purpose: Process received data frame
 */
static void TMWDEFS_LOCAL _finishDataFrame(
  DNPLINK_CONTEXT *pLinkContext)
{

#if TMWCNFG_SUPPORT_RXCALLBACKS
    if((!pLinkContext->rxFrame.isBroadcast) 
      &&(pLinkContext->tmw.pUserRxFrameCallback != TMWDEFS_NULL))
      {
        pLinkContext->tmw.pUserRxFrameCallback(
          pLinkContext->tmw.pUserRxFrameCallbackParam,
          pLinkContext->rxFrame.firstByteTime,
          pLinkContext->rxFrame.pMsgBuf, 
          &pLinkContext->rxFrame.msgLength);
      }
#endif
  

  /* Send received frame to the transport layer */
  if(pLinkContext->tmw.pParseFunc)
  {
    if(pLinkContext->rxFrame.isBroadcast)
    {
      TMWSESN *pSession = TMWDEFS_NULL;
      while((pSession = (TMWSESN *)tmwdlist_getAfter(
        &pLinkContext->tmw.sessions, (TMWDLIST_MEMBER *)pSession)) != TMWDEFS_NULL)
      {

       if((pSession->type == TMWTYPES_SESSION_TYPE_SLAVE) 
          && (pSession->active))
        {
          pLinkContext->tmw.pParseFunc(
            pLinkContext->tmw.pCallbackParam,
            pSession, &pLinkContext->rxFrame);
        }
      }
    }
    else
    {
      pLinkContext->tmw.pParseFunc(pLinkContext->tmw.pCallbackParam,
        pLinkContext->pRxSession, &pLinkContext->rxFrame);
    }
  }

  _checkForData(pLinkContext, TMWDEFS_NULL);
}

/* function: _afterAckCallback
 * purpose: called after acknowledge is successfully transmitted
 * arguments:
 *  pCallbackParam - Link layer context
 * returns:
 *  void
 */
static void TMWDEFS_CALLBACK _afterAckCallback(
  void *pCallbackParam)
{
  _finishDataFrame((DNPLINK_CONTEXT *)pCallbackParam);
}

/* function: _retryFrame
 * purpose: retry previously generated frame
 * arguments:
 *  pLinkContext - link layer context returned from dnplink_initChannel
 * returns:
 *  void
 */
static void TMWDEFS_LOCAL _retryFrame(
  DNPLINK_CONTEXT *pLinkContext)
{
  if(pLinkContext->pTxDescriptor != TMWDEFS_NULL)
  {
    if(pLinkContext->retryCount++ < pLinkContext->maxRetries)
    {
      /* Diagnostics */
      DNPDIAG_LINK_FRAME_SENT(
        pLinkContext->pTxDescriptor->pChannel,
        pLinkContext->pTxDescriptor->pSession,
        pLinkContext->physTxDescriptor.pTxBuffer,
        pLinkContext->physTxDescriptor.numBytesToTx,
        pLinkContext->retryCount);
       
      DNPSTAT_CHNL_ERROR(pLinkContext->tmw.pChannel, pLinkContext->pTxDescriptor->pSession, 
        TMWCHNL_ERROR_LINK_CNFM_TIMEOUT);

      (void)pLinkContext->tmw.pChannel->pPhys->pPhysTransmit(
        pLinkContext->tmw.pChannel->pPhysContext, &pLinkContext->physTxDescriptor);
    }
    else
    {
      TMWSESN *pSession = pLinkContext->pTxDescriptor->pSession;
      DNPLINK_SESSION_INFO *pLinkSessionInfo = (DNPLINK_SESSION_INFO *)pSession->pLinkSession;
      TMWSESN_TX_DATA *pOldTxDescriptor;

      DNPSTAT_CHNL_ERROR(pLinkContext->tmw.pChannel, pLinkContext->pTxDescriptor->pSession, 
        TMWCHNL_ERROR_LINK_CNFM_TIMEOUT);

      /* Make a copy of the TxDescriptor and reset link tx descriptor
       * since the callback may want to transmit another frame
       */
      pOldTxDescriptor = pLinkContext->pTxDescriptor;
      pLinkContext->pTxDescriptor = TMWDEFS_NULL;
      pLinkContext->pTxSession = TMWDEFS_NULL;

      /* Call failed transmission callback */
      if(pOldTxDescriptor->pFailedTxCallback)
      {
        pOldTxDescriptor->pFailedTxCallback(
          pOldTxDescriptor->pCallbackData, pOldTxDescriptor);
      }

      /* Must do a reset link before sending more frames */
      pLinkSessionInfo->secondaryStationIsReset = TMWDEFS_FALSE;

      /* Go ahead and see if any session on channel has data to transmit */
      _checkForData(pLinkContext, TMWDEFS_NULL);
    }
  }
  else  /* fixed length frame did not get confirmed */
  {
    if(pLinkContext->pTxSession != TMWDEFS_NULL)
    {
      TMWSESN *pSession;
      DNPLINK_SESSION_INFO *pLinkSessionInfo = (DNPLINK_SESSION_INFO *)pLinkContext->pTxSession->pLinkSession;
      TMWTYPES_UCHAR fc = pLinkContext->physTxDescriptor.pTxBuffer[3] & 0x0f;
      
      pSession = (TMWSESN*)pLinkContext->pTxSession;
      
      DNPSTAT_CHNL_ERROR(pSession->pChannel, pSession, 
        TMWCHNL_ERROR_LINK_CNFM_TIMEOUT);
     
      /* This was a reset, test link or request status that failed.
       * Resend it the required number of times  
       */ 
      if(pLinkContext->retryCount++ < pLinkContext->maxRetries)
      {  
        /* Diagnostics */
        DNPDIAG_LINK_FRAME_SENT(
          pSession->pChannel,
          pSession,
          pLinkContext->physTxDescriptor.pTxBuffer,
          pLinkContext->physTxDescriptor.numBytesToTx,
          pLinkContext->retryCount);
          
        (void)pLinkContext->tmw.pChannel->pPhys->pPhysTransmit(
          pSession->pChannel->pPhysContext, &pLinkContext->physTxDescriptor);

        return;
      }
         
      /* Must do a reset link before sending more frames */
      pLinkSessionInfo->secondaryStationIsReset = TMWDEFS_FALSE;

      tmwsesn_setOnline(pLinkContext->pTxSession, TMWDEFS_FALSE);
    
      pLinkContext->pTxSession = TMWDEFS_NULL;

      /*
       * If this was a request link status that timed out
       * close and reopen to cause TCP disconnect and then to
       * reconnect as configured, client, server etc.
       */
      if(fc==DNPDEFS_LCF_PRI_REQUEST_STATUS)
      {
        DNPSESN *pDNPSession = (DNPSESN *)pSession;  
        DNPSTAT_CHNL_ERROR(pLinkContext->tmw.pChannel, pSession, 
          TMWCHNL_ERROR_LINK_STATUS_TIMEOUT);

        /* Just disconnect and reconnect for TCP connections */
        /* disconnect/reconnect behavior can be configured out */
        if(((pLinkContext->networkType == DNPLINK_NETWORK_TCP_ONLY)
          || (pLinkContext->networkType == DNPLINK_NETWORK_TCP_UDP))
          && pDNPSession->linkStatusTimeoutDisconnect)
        {
          /* Disconnect and reconnect */
          tmwlink_closeChannel((TMWLINK_CONTEXT *)pLinkContext);

          pLinkContext->tmw.pInfoFunc(pLinkContext->tmw.pCallbackParam,
            pSession, TMWSCL_INFO_CLOSED);

          tmwlink_openChannel((TMWLINK_CONTEXT *)pLinkContext);
        }
        else
        {
          dnplink_startLinkStatusTimer(pSession);
        }

        return;
      }

      /* Go ahead and see if any session on channel has data to transmit */
      _checkForData(pLinkContext, TMWDEFS_NULL);
    }
  }
}
 

/* function: _confirmTimeout
 * purpose: handle link layer confirmation timeout
 * arguments:
 *  pCallbackParam - Link layer context
 * returns:
 *  void
 */
static void TMWDEFS_CALLBACK _confirmTimeout(
  void *pCallbackParam)
{ 
  _retryFrame((DNPLINK_CONTEXT *)pCallbackParam);
}

/* function: _beforeTxCallback
 * purpose: callback to handle stuff immediately before a
 *  block is transmitted
 * arguments:
 *  pCallbackParam - Link layer context
 * returns:
 *  void
 */
static void TMWDEFS_CALLBACK _beforeTxCallback(
  void *pCallbackParam)
{
  DNPLINK_CONTEXT *pLinkContext = (DNPLINK_CONTEXT *)pCallbackParam;

  /* Call before transmission callback */
  if((pLinkContext->pTxDescriptor != TMWDEFS_NULL)
    && (pLinkContext->pTxDescriptor->pBeforeTxCallback))
  {
    pLinkContext->pTxDescriptor->pBeforeTxCallback(
      pLinkContext->pTxDescriptor->pCallbackData, pLinkContext->pTxDescriptor);
  }
}

/* function: _afterTxCallback
 * purpose: callback to handle stuff immediately after a
 *  block is transmitted
 * arguments:
 *  pCallbackParam - Link layer context
 * returns:
 *  void
 */
static void TMWDEFS_CALLBACK _afterTxCallback(
  void *pCallbackParam)
{
  DNPLINK_CONTEXT *pLinkContext = (DNPLINK_CONTEXT *)pCallbackParam;
  TMWTYPES_UCHAR functionCode;
  TMWTYPES_BOOL broadcast;

  /* Update statistics */
  DNPSTAT_CHNL_FRAME_SENT(pLinkContext->tmw.pChannel);

  /* Check to see if this was a broadcast message */
  broadcast = TMWDEFS_FALSE;
  if((pLinkContext->pTxDescriptor != TMWDEFS_NULL)
    && ((pLinkContext->pTxDescriptor->txFlags & TMWSESN_TXFLAGS_BROADCAST)!=0))
  {
    broadcast = TMWDEFS_TRUE;
  }

  /* See if link level confirms were enabled */
  functionCode = (TMWTYPES_UCHAR)(pLinkContext->physTxDescriptor.pTxBuffer[3] & 0x0f);

  /* If not broadcast and one of these function codes, check for confirm timeout required */
  if(!broadcast &&
    (  (functionCode == DNPDEFS_LCF_PRI_CNFRM_DATA)
    || (functionCode == DNPDEFS_LCF_PRI_TEST_LINK)
    || (functionCode == DNPDEFS_LCF_PRI_REQUEST_STATUS)
    || (functionCode == DNPDEFS_LCF_PRI_LINK_RESET)))
  {
    if(pLinkContext->confirmTimeout > 0)
    {
      /* Start confirm timer */
      tmwtimer_start(&pLinkContext->confirmTimer,
        pLinkContext->confirmTimeout, pLinkContext->tmw.pChannel,
        _confirmTimeout, pLinkContext);
    }
    else 
    {
      pLinkContext->pTxSession = TMWDEFS_NULL;
    }
  }
  else
  {
    pLinkContext->pTxSession = TMWDEFS_NULL;

    if(pLinkContext->pTxDescriptor != TMWDEFS_NULL)
    {
      /* Make a copy of the TxDescriptor since the callback may want to transmit
       * another frame
       */
      TMWSESN_TX_DATA *pOldTxDescriptor = pLinkContext->pTxDescriptor;
      pLinkContext->pTxDescriptor = TMWDEFS_NULL;

      /* Call after transmission callback */
      if((pOldTxDescriptor != TMWDEFS_NULL)
        && (pOldTxDescriptor->pAfterTxCallback != TMWDEFS_NULL))
      {
        pOldTxDescriptor->pAfterTxCallback(
          pOldTxDescriptor->pCallbackData, pOldTxDescriptor);
      }

      if(pOldTxDescriptor != TMWDEFS_NULL)
      {
        _checkForData(pLinkContext, pOldTxDescriptor->pSession);
      }
    }
  }
}

/* function: _failedTxCallback
 * purpose: callback to handle stuff when transmission fails
 * arguments:
 *  pCallbackParam - Link layer context
 * returns:
 *  void
 */
static void TMWDEFS_CALLBACK _failedTxCallback(
  void *pCallbackParam)
{
  DNPLINK_CONTEXT *pLinkContext = (DNPLINK_CONTEXT *)pCallbackParam;

  pLinkContext->pTxSession = TMWDEFS_NULL;

  /* See if there is an transport layer descriptor pending */
  if(pLinkContext->pTxDescriptor != TMWDEFS_NULL)
  {
    /* Yes, call the failure callback. Make a copy of the TxDescriptor
     * since the callback may want to transmit another frame.
     */
    TMWSESN_TX_DATA *pOldTxDescriptor = pLinkContext->pTxDescriptor;
    pLinkContext->pTxDescriptor = TMWDEFS_NULL;

    /* Call failed transmission callback */
    if(pOldTxDescriptor->pFailedTxCallback)
    {
      pOldTxDescriptor->pFailedTxCallback(
        pOldTxDescriptor->pCallbackData, pOldTxDescriptor);
    }
  }
}

/* function: _sendFixedFrame
 * purpose: generate and transmit frame acknowledge
 * arguments:
 *  pCallbackParam - Link layer context
 * returns:
 *  void
 */
static void TMWDEFS_LOCAL _sendFixedFrame(
  DNPLINK_CONTEXT *pLinkContext,
  TMWSESN *pSession,
  TMWTYPES_USHORT destAddress,
  TMWTYPES_BOOL primary,
  TMWTYPES_UCHAR functionCode,
  TMWTYPES_CALLBACK_FUNC afterCallback,
  TMWTYPES_CALLBACK_FUNC failedCallback)
{
  TMWPHYS_TX_DESCRIPTOR *pPhysTxDescriptor;
  TMWTYPES_UCHAR *pFrameBufferPtr;
  TMWTYPES_UCHAR control;
  TMWTYPES_BOOL sendAck;

#ifdef TMW_SUPPORT_MONITOR
  /* If this channel is in passive listen only analyzer mode
   * dont send any frames 
   */
  if(pSession->pChannel->pPhysContext->monitorMode)
  {
    return;
  }
#endif

  /* If the session is not active simply return */
  if(!pSession->active)
  {
    DNPDIAG_ERROR(pLinkContext->tmw.pChannel, pSession, DNPDIAG_INACTIVE);
    return;
  }

  /* if not secondary ack or nack */
  if(primary || (functionCode > 1)) 
  {
    /* only allowed to send ack, nack (handled in else below) or Link Status if a frame is outstanding */
    if((pLinkContext->pTxSession != TMWDEFS_NULL) && (functionCode != DNPDEFS_LCF_SEC_STATUS_OF_LINK))
    {
      DNPDIAG_ERROR(pLinkContext->tmw.pChannel, pSession, DNPDIAG_WAITING);
      return;
    }
    sendAck = TMWDEFS_FALSE;

    if(functionCode == DNPDEFS_LCF_SEC_STATUS_OF_LINK)
    {
      /* A link status response will never be retried, so don't overwrite the normal physTxDescriptor */
      pPhysTxDescriptor = &pLinkContext->ackPhysTxDescriptor;
    }
    else
      pPhysTxDescriptor = &pLinkContext->physTxDescriptor;
  }
  else
  {  
    sendAck = TMWDEFS_TRUE;
    pPhysTxDescriptor = &pLinkContext->ackPhysTxDescriptor;
  }

  /* Generate and send header block */
  pPhysTxDescriptor->afterTxCallback = afterCallback;
  pPhysTxDescriptor->beforeTxCallback = TMWDEFS_NULL;
  pPhysTxDescriptor->failedTxCallback = failedCallback;
  pPhysTxDescriptor->pCallbackParam = pLinkContext;
  
  if(pLinkContext->networkType == DNPLINK_NETWORK_UDP_ONLY)
  {
    pPhysTxDescriptor->UDPPort = TMWTARG_UDP_SEND;
  }
  else
  {
    pPhysTxDescriptor->UDPPort = TMWTARG_UDP_NONE; 
  }

  pPhysTxDescriptor->numBytesToTx = 10;
  pFrameBufferPtr = pPhysTxDescriptor->pTxBuffer;

  /* Frame sync characters */
  *pFrameBufferPtr++ = 0x05;
  *pFrameBufferPtr++ = 0x64;

  /* Frame length */
  *pFrameBufferPtr++ = 5;

  /* Initialize frame control */
  control = 0x00;

  /* Set DIR bit properly */
  if(pSession->type == TMWTYPES_SESSION_TYPE_MASTER)
    control |= DNPDEFS_LC_DIR_MASK;

  /* Primary message */
  if(primary)
    control |= DNPDEFS_LC_PRM_MASK;

  /* Link layer function code */
  control |= functionCode;

  /* Test Link States requires FCV/FCB */
  if(functionCode == DNPDEFS_LCF_PRI_TEST_LINK)
  {
    /* Set FCV and FCB */
    control |= DNPDEFS_LC_FCV_MASK;

    if(((DNPLINK_SESSION_INFO *)pSession->pLinkSession)->nextFCB)
      control |= DNPDEFS_LC_FCB_MASK;
  }

  /* Store frame control byte */
  *pFrameBufferPtr++ = control;

  /* Destination address */
  tmwtarg_store16(&destAddress, pFrameBufferPtr);
  pFrameBufferPtr += 2;

  /* Source address */
  tmwtarg_store16(&pSession->srcAddress, pFrameBufferPtr);
  pFrameBufferPtr += 2;

  /* CRC */
  _setCRC(pPhysTxDescriptor->pTxBuffer, 8);

  /* Update statistics */
  DNPSTAT_CHNL_FRAME_SENT(pLinkContext->tmw.pChannel);

  /* Diagnostics */
  DNPDIAG_LINK_FRAME_SENT(
    pLinkContext->tmw.pChannel, pSession,
    pPhysTxDescriptor->pTxBuffer,
    pPhysTxDescriptor->numBytesToTx,
    0);

  /* Store session we transmitted to if this frame requires an ack.*/
  if(primary 
    && ((functionCode == DNPDEFS_LCF_PRI_TEST_LINK)
    || (functionCode == DNPDEFS_LCF_PRI_REQUEST_STATUS)
    || (functionCode == DNPDEFS_LCF_PRI_LINK_RESET)))
  {
    pLinkContext->retryCount = 0;
    pLinkContext->pTxSession = pSession;
  }

  /* Transmit header */
  (void)pLinkContext->tmw.pChannel->pPhys->pPhysTransmit(
    pLinkContext->tmw.pChannel->pPhysContext, pPhysTxDescriptor);

  /* Update the last secondary confirm function if this is a confirmation */
  if(sendAck)
  {
    DNPLINK_SESSION_INFO *pLinkSessionInfo = (DNPLINK_SESSION_INFO *)pSession->pLinkSession;
    pLinkSessionInfo->lastSecondaryConfirmFuncCode = functionCode;
  }
}

/* function: _buildMessage
 * purpose: build a link level message including header, CRC, etc.
 * arguments:
 *  pLinkContext - link layer context returned from dnplink_initChannel
 * returns
 *  void
 */
static TMWTYPES_BOOL TMWDEFS_LOCAL _buildMessage(
  DNPLINK_CONTEXT *pLinkContext)
{
  TMWTYPES_UCHAR *pFrameBufferPtr;
  TMWSESN_TX_DATA *pTxData;
  TMWTYPES_USHORT bytesToTransmit;
  TMWTYPES_USHORT bytesLeft;
  TMWTYPES_USHORT offset;
  TMWTYPES_UCHAR control;

  pTxData = pLinkContext->pTxDescriptor;

  /* Point to beginning of transmit buffer */
  pFrameBufferPtr = pLinkContext->physTxDescriptor.pTxBuffer;

  /* Frame sync characters */
  *pFrameBufferPtr++ = 0x05;
  *pFrameBufferPtr++ = 0x64;

  /* Frame length */
  *pFrameBufferPtr++ = (TMWTYPES_UCHAR)(5 + pTxData->msgLength);

  /* Initialize frame control */
  control = 0x00;

  /* Need to generate DIR bit on a per frame basis for peer sessions */
  if(pTxData->txFlags & TMWSESN_TXFLAGS_MASTER)
    control |= DNPDEFS_LC_DIR_MASK;

  /* Messages sent from application layer are always primary */
  control |= DNPDEFS_LC_PRM_MASK;

  /* For non-broadcast messages 
   * If link confirm mode is always,
   * or link confirm mode is sometimes
   *   and this transport frame is not BOTH first and final 
   * and not broadcast
   * then send confirmed user data.
   */
  if(((pLinkContext->confirmMode == TMWDEFS_LINKCNFM_ALWAYS)
    || ((pLinkContext->confirmMode == TMWDEFS_LINKCNFM_SOMETIMES)
      &&((pTxData->pMsgBuf[0] & 0xc0) != 0xc0)))
    && ((pTxData->txFlags & TMWSESN_TXFLAGS_BROADCAST)==0))
  {
    DNPLINK_SESSION_INFO *pLinkSessionInfo =
      (DNPLINK_SESSION_INFO *)pTxData->pSession->pLinkSession;

    if(!pLinkSessionInfo->secondaryStationIsReset)
    {
      pLinkSessionInfo->resetLinkWait = TMWDEFS_TRUE;

      _sendFixedFrame(pLinkContext, pTxData->pSession,  pTxData->destAddress,
        TMWDEFS_TRUE, DNPDEFS_LCF_PRI_LINK_RESET, _afterTxCallback, _failedTxCallback);

      return(TMWDEFS_FALSE);
    }

    /* Confirmed data, set FCV and FCB */
    control |= DNPDEFS_LC_FCV_MASK;

    /* Set frame count bit */
    if(((DNPLINK_SESSION_INFO *)pTxData->pSession->pLinkSession)->nextFCB)
      control |= DNPDEFS_LC_FCB_MASK;

    control |= DNPDEFS_LCF_PRI_CNFRM_DATA;
  }
  else
  {
    /* Unconfirmed data, leave FCV and FCB unset */
    control |= DNPDEFS_LCF_PRI_UNCNFRM_DATA;
  }

  /* Store frame control byte */
  *pFrameBufferPtr++ = control;

  /* Destination address */
  tmwtarg_store16(&pTxData->destAddress, pFrameBufferPtr);
  pFrameBufferPtr += 2;

  /* Source address */
  tmwtarg_store16(&pTxData->pSession->srcAddress, pFrameBufferPtr);
  pFrameBufferPtr += 2;

  /* CRC */
  _setCRC(pLinkContext->physTxDescriptor.pTxBuffer, DNPLINK_HEADER_SIZE - DNPLINK_CRC_SIZE);
  pFrameBufferPtr += 2;

  /* Now put the user data into message */
  offset = 0;
  while(offset < pTxData->msgLength)
  {
    bytesLeft = (TMWTYPES_USHORT)(pTxData->msgLength - offset);
    bytesToTransmit = (TMWTYPES_USHORT)(((DNPLINK_BLOCK_SIZE - DNPLINK_CRC_SIZE) < bytesLeft) ? (DNPLINK_BLOCK_SIZE - DNPLINK_CRC_SIZE) : bytesLeft);

    memcpy(pFrameBufferPtr, pTxData->pMsgBuf + offset, bytesToTransmit);
    _setCRC(pFrameBufferPtr, bytesToTransmit);

    offset = (TMWTYPES_USHORT)(offset + bytesToTransmit);
    pFrameBufferPtr += (bytesToTransmit + DNPLINK_CRC_SIZE);
  }

  pLinkContext->txMessageSize = (TMWTYPES_USHORT)(pFrameBufferPtr - pLinkContext->physTxDescriptor.pTxBuffer);

  return(TMWDEFS_TRUE);
}

/* function: _sendVariableFrame
 * purpose: generate and transmit frame acknowledge
 * arguments:
 *  pCallbackParam - Link layer context
 * returns:
 *  void
 */
static void TMWDEFS_LOCAL _sendVariableFrame(
  DNPLINK_CONTEXT *pLinkContext)
{
  /* Make sure we have a frame to send */
  if(pLinkContext->pTxDescriptor == TMWDEFS_NULL)
    return;

  /* If the session is not active call failedTxCallback and return */
  if(!pLinkContext->pTxDescriptor->pSession->active)
  {
    TMWSESN_TX_DATA *pOldTxDescriptor;

    DNPDIAG_ERROR(pLinkContext->tmw.pChannel,
      pLinkContext->pTxDescriptor->pSession, DNPDIAG_INACTIVE); 

    /* Make a copy of the TxDescriptor since the callback may want to transmit
     * another frame
     */
    pOldTxDescriptor = pLinkContext->pTxDescriptor;
    pLinkContext->pTxDescriptor = TMWDEFS_NULL;

    /* Call after tx callback if specified,
     * Calling failed tx callback, can make the layer
     * above resend the request which just causes recursion,
     */
    if(pOldTxDescriptor->pAfterTxCallback)
    {
      pOldTxDescriptor->pAfterTxCallback(
        pOldTxDescriptor->pCallbackData, pOldTxDescriptor);
    }

    return;
  }

  /* Build message */
  if(_buildMessage(pLinkContext))
  {
    /* Initialize physical layer transmit data structure */
    pLinkContext->physTxDescriptor.beforeTxCallback = _beforeTxCallback;
    pLinkContext->physTxDescriptor.afterTxCallback = _afterTxCallback;
    pLinkContext->physTxDescriptor.failedTxCallback = _failedTxCallback;
    pLinkContext->physTxDescriptor.pCallbackParam = pLinkContext;
    pLinkContext->physTxDescriptor.numBytesToTx = pLinkContext->txMessageSize;

    /* Diagnostics */
    DNPDIAG_LINK_FRAME_SENT(
      pLinkContext->pTxDescriptor->pChannel,
      pLinkContext->pTxDescriptor->pSession,
      pLinkContext->physTxDescriptor.pTxBuffer,
      pLinkContext->physTxDescriptor.numBytesToTx,
      0);

    /* Store session we transmitted to */
    pLinkContext->pTxSession = pLinkContext->pTxDescriptor->pSession;

    /* If UDP is supported, determine whether to use it for this message
     * and what destination UDP port to use 
     */
    pLinkContext->physTxDescriptor.UDPPort = TMWTARG_UDP_NONE;
    if(pLinkContext->networkType == DNPLINK_NETWORK_UDP_ONLY)
    {
      if(pLinkContext->pTxDescriptor->txFlags & TMWSESN_TXFLAGS_UNSOL)
      {
        pLinkContext->physTxDescriptor.UDPPort = TMWTARG_UDP_SEND_UNSOL;
      }
      else
      {
        if (pLinkContext->pTxDescriptor->txFlags & TMWSESN_TXFLAGS_BROADCAST)
        {
          /* To support broadcast from master when UDP only is configured 
           * use this define
           */
          pLinkContext->physTxDescriptor.UDPPort = TMWTARG_UDPONLY_BROADCAST;
        }
        else
        {
          pLinkContext->physTxDescriptor.UDPPort = TMWTARG_UDP_SEND;
        }
      }
    }
    else if((pLinkContext->networkType == DNPLINK_NETWORK_TCP_UDP)
      &&(pLinkContext->pTxDescriptor->txFlags & TMWSESN_TXFLAGS_BROADCAST))
    {
      pLinkContext->physTxDescriptor.UDPPort = TMWTARG_UDP_SEND;
    }
    /* Send message */
    (void)pLinkContext->tmw.pChannel->pPhys->pPhysTransmit(
      pLinkContext->tmw.pChannel->pPhysContext, &pLinkContext->physTxDescriptor);
  }
}

/* function: _rxFrameTimeout
 * purpose: handle frame timeout
 * arguments:
 *  pCallbackParam - Link layer context
 * returns:
 *  void
 */
static void TMWDEFS_CALLBACK _rxFrameTimeout(
  void *pCallbackParam)
{
  TMWLINK_CONTEXT *pContext = (TMWLINK_CONTEXT *)pCallbackParam;
  DNPLINK_CONTEXT *pLinkContext = (DNPLINK_CONTEXT *)pContext;
  
  DNPSTAT_CHNL_ERROR(pLinkContext->tmw.pChannel, TMWDEFS_NULL,
    TMWCHNL_ERROR_LINK_FRAME_TIMEOUT);

  DNPDIAG_ERROR(pLinkContext->tmw.pChannel, TMWDEFS_NULL, 
    DNPDIAG_RCV_TIMEOUT); 

  /* Diagnostics */
  DNPDIAG_LINK_FRAME_COMPLETE(pLinkContext->pRxDiagContext, TMWDEFS_TRUE);
  pLinkContext->pRxDiagContext = TMWDEFS_NULL;

  pLinkContext->rxState = DNP_LINK_STATE_IDLE;
  pLinkContext->rxBlockOffset = 0;
  pLinkContext->rxFrameOffset = 0;
}

/* function: _findSession
 * purpose: Check to see if this address is for an open session.
 *  If not check to see if user has registered callback function. If
 *  so call this and then check again to see if the session has been opened.
 * arguments:
 *  srcAddress - source address from received frame
 *  destAddress - destination address from received frame
 * returns:
 */
static TMWSESN * TMWDEFS_LOCAL _findSession(
  void *pContext,
  TMWTYPES_USHORT srcAddress,
  TMWTYPES_USHORT destAddress,
  TMWTYPES_UCHAR  control)
{
  DNPSESN *pDNPSession;
  DNPLINK_CONTEXT *pLinkContext = (DNPLINK_CONTEXT *)pContext;
  TMWSESN *pSession = TMWDEFS_NULL;

  /* See if there is a session open for this address */
  while((pSession = (TMWSESN *)tmwdlist_getAfter(
    &pLinkContext->tmw.sessions, (TMWDLIST_MEMBER *)pSession)) != TMWDEFS_NULL)
  {
    pDNPSession = (DNPSESN *)pSession;
    if(pSession->srcAddress == destAddress)
    {
      if(!pDNPSession->validateSourceAddress || (pSession->destAddress == srcAddress))
      {
        return(pSession);
      }

      /* Check for response to self address request */
      if(pSession->destAddress == DNPDEFS_SELF_ADDR) 
      {
        pSession->destAddress = srcAddress;
        return(pSession);
      }
    }
    /* Check for request to self address */
    if(pDNPSession->enableSelfAddress 
      && (destAddress == DNPDEFS_SELF_ADDR))
    { 
      if(!pDNPSession->validateSourceAddress || (pSession->destAddress == srcAddress))
        return(pSession);
    }
  }

  /* If user has registered an auto open callback function call it now */
  if(pLinkContext->tmw.pChannel->pAutoOpenCallback != TMWDEFS_NULL)
  {
    /* Determine what type of session needs to be opened */
    TMWTYPES_SESSION_TYPE sessionType;
    if(control & DNPDEFS_LC_DIR_MASK)
    {
      sessionType = TMWTYPES_SESSION_TYPE_SLAVE;
    }
    else
    {
      sessionType = TMWTYPES_SESSION_TYPE_MASTER;
    }
    /* Call the callback function, switch the src and dest address, so it is meaningful for opening session */
    pLinkContext->tmw.pChannel->pAutoOpenCallback(pLinkContext->tmw.pChannel->pAutoOpenCallbackParam, 
      destAddress, srcAddress, sessionType);
  }

#ifdef TMW_SUPPORT_MONITOR
  if(pLinkContext->tmw.pChannel->pPhysContext->monitorMode)
  {
    TMWCHNL_STAT_UNKNOWN_SESN_TYPE data;
    if(control & DNPDEFS_LC_DIR_MASK)
    {
      data.sessionType = TMWTYPES_SESSION_TYPE_SLAVE;
    }
    else
    {
      data.sessionType = TMWTYPES_SESSION_TYPE_MASTER;
    }
    data.linkAddress = destAddress;  
    data.destAddress = srcAddress;

    TMWCHNL_STAT_CALLBACK_FUNC(
      pLinkContext->tmw.pChannel,
      TMWCHNL_STAT_UNKNOWN_SESSION, &data);
  }
#endif

  /* Check again to see if session has been opened */
  while((pSession = (TMWSESN *)tmwdlist_getAfter(
    &pLinkContext->tmw.sessions, (TMWDLIST_MEMBER *)pSession)) != TMWDEFS_NULL)
  {
    pDNPSession = (DNPSESN *)pSession;
    if(pSession->srcAddress == destAddress)
    {
      if(!pDNPSession->validateSourceAddress || (pSession->destAddress == srcAddress))
      {
        return(pSession);
      }
    }

    /* Check for request to self address */
    if(pDNPSession->enableSelfAddress 
      && (destAddress == DNPDEFS_SELF_ADDR))
    { 
      if(!pDNPSession->validateSourceAddress || (pSession->destAddress == srcAddress))
        return(pSession);
    }
  }

  return(TMWDEFS_NULL);
}

/* function: _updateMsg
 * purpose: This is used to update the message and recalculate the checksums
 *   Currently this is used for a timesync request.
 * arguments:
 * returns:
 */
static TMWTYPES_UCHAR * TMWDEFS_CALLBACK _updateMsg(
  TMWLINK_CONTEXT *pContext,
  TMWTYPES_USHORT offset,
  TMWTYPES_UCHAR *pData,
  TMWTYPES_USHORT length)
{
  DNPLINK_CONTEXT *pLinkContext = (DNPLINK_CONTEXT *)pContext;

  memcpy(pLinkContext->physTxDescriptor.pTxBuffer + DNPDEFS_LINK_HDR_AND_CRC_SIZE + offset, pData, length);
  
  dnplink_rechecksum(&pLinkContext->physTxDescriptor);
  
#if DNPCNFG_SUPPORT_AUTHENTICATION
  {
    /* Also update fragment, so security can see the updated message */
    TMWSESN_TX_DATA *pCurrentMessage = dnputil_getCurrentMessage(pLinkContext->pTxSession);
    if (pCurrentMessage != TMWDEFS_NULL)
      memcpy(pCurrentMessage->pMsgBuf + offset - 1, pData, length);
  }
#endif
  return TMWDEFS_NULL;
}

/* function: _checkDataReady
 * purpose:
 * arguments:
 * returns:
 */
static void TMWDEFS_CALLBACK _checkDataReady(
  TMWLINK_CONTEXT *pContext,
  TMWSESN *pSession)
{
  DNPLINK_CONTEXT *pLinkContext = (DNPLINK_CONTEXT *)pContext;

  /* If connection is open, check to see if we can send data
   */ 
  if(tmwlink_checkOpen(pContext))
  {
    _checkForData(pLinkContext, pSession);
  }
}

/* function: _transmitFrame
 * purpose: transmit frame
 * arguments:
 *  pContext - link layer context returned from dnplink_initChannel
 *  pTxDescriptor - data to transmit
 * returns
 *  void
 */
static void TMWDEFS_CALLBACK _transmitFrame(
  TMWLINK_CONTEXT *pContext,
  TMWSESN_TX_DATA *pTxDescriptor)
{
  DNPLINK_CONTEXT *pLinkContext = (DNPLINK_CONTEXT *)pContext;

  /* Initialize context */
  pLinkContext->retryCount = 0;
  pLinkContext->pTxDescriptor = pTxDescriptor;

  /* Call function to send variable frame */
  _sendVariableFrame(pLinkContext);
}

/* function: _cancelFrame
 * purpose: cancel frame
 * arguments:
 *  pContext - link layer context returned from dnplink_initChannel
 *  pTxDescriptor - data frame to cancel
 * returns
 *  void
 */
static void TMWDEFS_CALLBACK _cancelFrame(
  void *pContext,
  TMWSESN_TX_DATA *pTxDescriptor)
{ 
  DNPLINK_CONTEXT *pLinkContext = (DNPLINK_CONTEXT *)pContext;
  DNPLINK_SESSION_INFO *pLinkSessionInfo;
  TMWSESN *pSession;

  TMWTARG_UNUSED_PARAM(pTxDescriptor);

  /* Make sure we have a pending frame */
  if(pLinkContext->pTxDescriptor != pTxDescriptor)
    return;

  /* Get session info */
  pSession = pLinkContext->pTxDescriptor->pSession;
  pLinkSessionInfo = (DNPLINK_SESSION_INFO *)pSession->pLinkSession;

  /* If still waiting for a link confirm, toggle FCB. 
   * the confirm is what normally toggles it.
   */
  if(tmwtimer_isActive(&pLinkContext->confirmTimer))
  {
    /* Toggle frame count bit */
    pLinkSessionInfo->nextFCB =
      (TMWTYPES_BOOL)!pLinkSessionInfo->nextFCB;
  }

  /* Initialize context */
  pLinkContext->retryCount = 0;
  pLinkContext->pTxDescriptor = TMWDEFS_NULL;   
  pLinkContext->pTxSession = TMWDEFS_NULL;

  /* Cancel at physical layer also */
  pLinkContext->tmw.pChannel->pPhysContext->pTxDescriptor = TMWDEFS_NULL;

  tmwtimer_cancel(&pLinkContext->confirmTimer); 
  return;
}

/* function: dnplink_getNeededBytes
 * purpose: return the number of bytes required to finish the
 *  current block
 * arguments:
 *  pCallbackParam - callback data, contains link layer context
 * returns
 *  number of characters to read
 */
TMWTYPES_USHORT TMWDEFS_CALLBACK dnplink_getNeededBytes(
  void *pCallbackParam)
{
  DNPLINK_CONTEXT *pLinkContext = (DNPLINK_CONTEXT *)pCallbackParam;

  /* Based on state return the number of bytes we want */
  switch(pLinkContext->rxState)
  {
  case DNP_LINK_STATE_IDLE:
    /* Read one byte at a time until we receive a start byte */
    return(1);

  case DNP_LINK_STATE_HEADER:
    /* Want to read the remaining header bytes */
    return((TMWTYPES_USHORT)(DNPLINK_HEADER_SIZE - pLinkContext->rxBlockOffset));

  case DNP_LINK_STATE_USER_DATA:
  case DNP_LINK_STATE_DISCARD_DATA:
  {
    /* Figure out how many more bytes we need for the frame */
    TMWTYPES_USHORT bytesRemaining = (TMWTYPES_USHORT)(pLinkContext->rxLength - pLinkContext->rxFrameOffset);

    /* Now figure out how many more bytes for this block */
    if(bytesRemaining < 16)
      return((TMWTYPES_USHORT)((bytesRemaining + DNPLINK_CRC_SIZE) - pLinkContext->rxBlockOffset));
    else
      return((TMWTYPES_USHORT)(DNPLINK_BLOCK_SIZE - pLinkContext->rxBlockOffset));
  }
   
  default:
    DNPDIAG_ERROR(pLinkContext->tmw.pChannel, TMWDEFS_NULL, DNPDIAG_NEEDED_BYTES);
    pLinkContext->rxState = DNP_LINK_STATE_IDLE;
    return(0);
  }
}

/* function: _validateReset
 * purpose: Check to see if we have been reset
 * arguments:
 * returns:
 *  TMWDEFS_TRUE if valid reset received else TMWDEFS_FALSE
 */
static TMWTYPES_BOOL TMWDEFS_LOCAL _validateReset(
  DNPLINK_CONTEXT *pLinkContext,
  DNPLINK_SESSION_INFO *pLinkSessionInfo)
{

  /* Ignore for broadcast messages */
  if(pLinkSessionInfo == TMWDEFS_NULL)
    return(TMWDEFS_TRUE);

  /* Make sure we have been reset */
  if(!pLinkSessionInfo->linkIsReset)
  {
    /* Link not reset */
    DNPSTAT_CHNL_ERROR(pLinkContext->tmw.pChannel, pLinkContext->pRxSession, TMWCHNL_ERROR_LINK_NOT_RESET);

    DNPDIAG_ERROR(pLinkContext->tmw.pChannel, pLinkContext->pRxSession, DNPDIAG_LINK_NOT_RESET);

#ifdef TMW_SUPPORT_MONITOR 
    if(pLinkContext->tmw.pChannel->pPhysContext->monitorMode)
    {
      /* pretend we received reset so we quit complaining */
      pLinkSessionInfo->linkIsReset = TMWDEFS_TRUE;
      return(TMWDEFS_TRUE);
    }
#endif 

    /* Send error function code, either NACK or NOT_USED depending on FCV bit */
    if((pLinkContext->ctrl & DNPDEFS_LC_FCV_MASK) == DNPDEFS_LC_FCV_MASK)
    {
      _sendFixedFrame(pLinkContext, pLinkContext->pRxSession,  pLinkContext->rxFrame.rxAddress,
        TMWDEFS_FALSE, DNPDEFS_LCF_SEC_CONFIRM_NACK, TMWDEFS_NULL, TMWDEFS_NULL);
    }
    else
    {
      _sendFixedFrame(pLinkContext, pLinkContext->pRxSession,  pLinkContext->rxFrame.rxAddress,
        TMWDEFS_FALSE, DNPDEFS_LCF_SEC_NOT_USED, TMWDEFS_NULL, TMWDEFS_NULL);
    }

    return(TMWDEFS_FALSE);
  }

  return(TMWDEFS_TRUE);
}

/* function: _validateFCB
 * purpose: validate received against expected frame count bit
 * arguments:
 * returns:
 *  TMWDEFS_TRUE if valid FCB received else TMWDEFS_FALSE
 */
static TMWTYPES_BOOL TMWDEFS_LOCAL _validateFCB(
  DNPLINK_CONTEXT *pLinkContext,
  DNPLINK_SESSION_INFO *pLinkSessionInfo,
  TMWTYPES_BOOL fcbRequired,
  TMWTYPES_UCHAR errorFuncCode)
{
  /* Ignore FCV/FCB for broadcast messages */
  if(pLinkSessionInfo == TMWDEFS_NULL)
    return(TMWDEFS_TRUE);

  if(fcbRequired)
  {

    if((pLinkContext->ctrl & DNPDEFS_LC_FCV_MASK) != DNPDEFS_LC_FCV_MASK)
    {
      /* Frame Count Valid bit not set */
      DNPDIAG_ERROR(pLinkContext->tmw.pChannel, pLinkContext->pRxSession, DNPDIAG_INVALID_FCV);

      /* Send error function code */
      _sendFixedFrame(pLinkContext, pLinkContext->pRxSession, pLinkContext->rxFrame.rxAddress,
        TMWDEFS_FALSE, DNPDEFS_LCF_SEC_NOT_USED, TMWDEFS_NULL, _failedTxCallback);

      return(TMWDEFS_FALSE);
    }

    if((pLinkSessionInfo->expectedFCB && ((pLinkContext->ctrl & DNPDEFS_LC_FCB_MASK) != DNPDEFS_LC_FCB_MASK))
      || (!pLinkSessionInfo->expectedFCB && ((pLinkContext->ctrl & DNPDEFS_LC_FCB_MASK) != 0)))
    {
      /* Frame count bit did not match */ 
      DNPSTAT_CHNL_ERROR(pLinkContext->tmw.pChannel, pLinkContext->pRxSession, TMWCHNL_ERROR_LINK_FCB);
      DNPDIAG_ERROR(pLinkContext->tmw.pChannel, pLinkContext->pRxSession, DNPDIAG_INVALID_FCB);

#ifdef TMW_SUPPORT_MONITOR 
      if(!pLinkContext->tmw.pChannel->pPhysContext->monitorMode)
      {
        /* Send acknowledge, assume this is a repeat of previous message */
        _sendFixedFrame(pLinkContext, pLinkContext->pRxSession, pLinkContext->rxFrame.rxAddress,
          TMWDEFS_FALSE, errorFuncCode, TMWDEFS_NULL, _failedTxCallback);
   
        return(TMWDEFS_FALSE);
      }
#else
      /* Send acknowledge, assume this is a repeat of previous message */
      _sendFixedFrame(pLinkContext, pLinkContext->pRxSession, pLinkContext->rxFrame.rxAddress,
        TMWDEFS_FALSE, errorFuncCode, TMWDEFS_NULL, _failedTxCallback);

      return(TMWDEFS_FALSE);
#endif
    }

    /* Toggle expected frame count bit for next message*/
    pLinkSessionInfo->expectedFCB = (TMWTYPES_BOOL)!pLinkSessionInfo->expectedFCB;
  }
  else
  {
    if((pLinkContext->ctrl & DNPDEFS_LC_FCV_MASK) != 0x00)
    {
      /* Frame Count Valid bit not set */
      DNPDIAG_ERROR(pLinkContext->tmw.pChannel, pLinkContext->pRxSession, DNPDIAG_INVALID_FCV);

      /* Send error function code */
      _sendFixedFrame(pLinkContext, pLinkContext->pRxSession, pLinkContext->rxFrame.rxAddress,
        TMWDEFS_FALSE, DNPDEFS_LCF_SEC_NOT_USED, TMWDEFS_NULL, _failedTxCallback);

      return(TMWDEFS_FALSE);
    }
  }

  return(TMWDEFS_TRUE);
}

/* function: _checkAddressMatchCallback
 * purpose: parse incoming data to determine if it is for this channel
 * arguments:
 *  pCallbackParam - callback data, contains link layer context
 *  recvBuf - received characters
 *  numBytes - number of bytes
 *  firstByteTime - time first byte was received
 * returns
 *  void
 */
static TMWPHYS_ADDRESS_MATCH_TYPE TMWDEFS_CALLBACK _checkAddressMatchCallback(
  void *pCallbackParam,
  TMWTYPES_UCHAR *recvBuf,
  TMWTYPES_USHORT numBytes,
  TMWTYPES_MILLISECONDS firstByteTime)
{    
  TMWTYPES_USHORT srcAddress;
  TMWTYPES_USHORT destAddress;
  DNPLINK_CONTEXT *pLinkContext = (DNPLINK_CONTEXT *)pCallbackParam;

  if(numBytes == 0)
    return(TMWPHYS_ADDRESS_MATCH_FAILED);
 
  /* If msg is for DNP it must have these start characters */
  if(recvBuf[0] != 0x05)
    return(TMWPHYS_ADDRESS_MATCH_FAILED);

  if(numBytes == 1)
    return(TMWPHYS_ADDRESS_MATCH_MAYBE);

  /* If msg is for DNP it must have these start characters */
  if(recvBuf[1] != 0x64)
    return(TMWPHYS_ADDRESS_MATCH_FAILED);
    
  if(numBytes < 8)
    return(TMWPHYS_ADDRESS_MATCH_MAYBE);
    
  /* Get source and destination address from message */
  tmwtarg_get16(&recvBuf[4], &destAddress);
  tmwtarg_get16(&recvBuf[6], &srcAddress);

  if(_findSession(pLinkContext, srcAddress, destAddress, recvBuf[3]))
  {
    int index = 1;
    dnplink_parseBytes(pCallbackParam, recvBuf, 1, firstByteTime);
    while(index < numBytes)
    {
      TMWTYPES_USHORT needed;
      TMWTYPES_USHORT numBytesAvailable;

      needed = dnplink_getNeededBytes(pCallbackParam);
      numBytesAvailable = (TMWTYPES_USHORT)(numBytes - index);
      if(needed < numBytesAvailable)
        numBytesAvailable = needed;

      dnplink_parseBytes(pCallbackParam, &recvBuf[index], numBytesAvailable, firstByteTime);
      index += numBytesAvailable;
    }
    return(TMWPHYS_ADDRESS_MATCH_SUCCESS);
  }

  return(TMWPHYS_ADDRESS_MATCH_FAILED);
}

/* function: _linkStatusTimeout */
static void TMWDEFS_CALLBACK _linkStatusTimeout(
  void *pCallbackParam)
{
  TMWSESN *pSession = (TMWSESN *)pCallbackParam;
  DNPLINK_CONTEXT *pLinkContext = (DNPLINK_CONTEXT *)pSession->pChannel->pLinkContext;

  /* Check to make sure we are open and not already busy, if so just skip this request */
  /* DNP IP Spec says Keep-alive messages are only needed when the connection is open  */
  if(pLinkContext->tmw.isOpen)
  {
    if ((pLinkContext->pTxDescriptor == TMWDEFS_NULL)
      &&  (pLinkContext->pTxSession == TMWDEFS_NULL))
    {
      /* Send link status request */
      _sendFixedFrame(pLinkContext, pSession, pSession->destAddress,
        TMWDEFS_TRUE, DNPDEFS_LCF_PRI_REQUEST_STATUS, _afterTxCallback, _failedTxCallback);
    }
 
    /* Restart the timer in case the transmit fails */
    dnplink_startLinkStatusTimer(pSession);
  }
} 

/* function: dnplink_parseBytes
 * purpose: parse incoming data
 * arguments:
 *  pCallbackParam - callback data, contains link layer context
 *  recvBuf - received characters
 *  numBytes - number of bytes
 *  firstByteTime - time first byte was received
 * returns
 *  void
 */
void TMWDEFS_CALLBACK dnplink_parseBytes(
  void *pCallbackParam,
  TMWTYPES_UCHAR *recvBuf,
  TMWTYPES_USHORT numBytes,
  TMWTYPES_MILLISECONDS firstByteTime)
{
  DNPLINK_CONTEXT *pLinkContext = (DNPLINK_CONTEXT *)pCallbackParam;
  DNPLINK_SESSION_INFO *pLinkSessionInfo = TMWDEFS_NULL;
  TMWTYPES_USHORT index = 0;

  /* If no bytes read just return */
  if(numBytes == 0)
    return;

  /* If we are idle (i.e. not currently receiving a frame)
   * check each input byte until we find a start of frame.
   */
  if(pLinkContext->rxState == DNP_LINK_STATE_IDLE)
  {
    if(numBytes != 1)
    {  
      DNPDIAG_ERROR(pLinkContext->tmw.pChannel,
        TMWDEFS_NULL, DNPDIAG_READ_MORE);
      return;
    }

    if(recvBuf[index] == 0x05)
    {
      pLinkContext->rxState = DNP_LINK_STATE_HEADER;
      pLinkContext->pRxBlock[0] = 0x05;
      pLinkContext->rxBlockOffset = 1;
      pLinkContext->rxFrameOffset = 0;
      if(firstByteTime == 0)
      {
        pLinkContext->rxFrame.firstByteTime = tmwtarg_getMSTime();
      }
      else
      {
        pLinkContext->rxFrame.firstByteTime = firstByteTime;
      }
    }

    return;
  }

  /* make sure not to write past end of buffer, if target layer gives us too many bytes */
  if(numBytes > DNPLINK_BLOCK_SIZE)
  {
    DNPDIAG_ERROR(pLinkContext->tmw.pChannel,
      TMWDEFS_NULL, DNPDIAG_READ_MORE);
    
    /* Reset link state */
    pLinkContext->rxState = DNP_LINK_STATE_IDLE;
    pLinkContext->rxBlockOffset = 0;
    pLinkContext->rxFrameOffset = 0;

    return;
  }

  /* Copy received bytes into current receive block */
  memcpy(pLinkContext->pRxBlock + pLinkContext->rxBlockOffset, recvBuf, numBytes);
  pLinkContext->rxBlockOffset = (TMWTYPES_USHORT)(pLinkContext->rxBlockOffset + numBytes);

  /* Process bytes based on current link state */
  if(pLinkContext->rxState == DNP_LINK_STATE_HEADER)
  {
    /* See if we have entire header */
    if(pLinkContext->rxBlockOffset >= DNPLINK_HEADER_SIZE)
    {
      TMWTYPES_USHORT blockSize = pLinkContext->rxBlockOffset;
      TMWTYPES_USHORT srcAddress;
      TMWTYPES_USHORT destAddress;

      /* Reset link state */
      pLinkContext->rxState = DNP_LINK_STATE_IDLE;
      pLinkContext->rxBlockOffset = 0;
      pLinkContext->rxFrameOffset = 0;

      /* Check to make sure we only got the number of bytes we asked for */
      if(blockSize > DNPLINK_HEADER_SIZE)
      {
        DNPDIAG_ERROR(pLinkContext->tmw.pChannel,
          TMWDEFS_NULL, DNPDIAG_READ_MORE);
        return;
      }

      /* Make sure we have correct two byte sync */
      if((pLinkContext->pRxBlock[0] != 0x05)
        || (pLinkContext->pRxBlock[1] != 0x64))
      {  
        DNPSTAT_CHNL_ERROR(pLinkContext->tmw.pChannel, TMWDEFS_NULL,
          TMWCHNL_ERROR_LINK_INVALID_START_CHAR);

        DNPDIAG_ERROR(pLinkContext->tmw.pChannel, TMWDEFS_NULL, 
          DNPDIAG_BAD_START);

        _restartFrame(pLinkContext, blockSize);
        return;
      }

      /* Check header CRC */
      if(!_checkCRC(pLinkContext->pRxBlock, DNPLINK_HEADER_SIZE))
      {
        DNPSTAT_CHNL_ERROR(pLinkContext->tmw.pChannel, TMWDEFS_NULL,
          TMWCHNL_ERROR_LINK_INVALID_CHECKSUM);

        DNPDIAG_ERROR(pLinkContext->tmw.pChannel, TMWDEFS_NULL, 
          DNPDIAG_CRC_HEADER); 

        _restartFrame(pLinkContext, blockSize);
        return;
      }
   
      /* Message length and control byte */
      pLinkContext->rxLength = (TMWTYPES_UCHAR)(pLinkContext->pRxBlock[2] - 5);

      /* Discard frame if length is longer than rx buffer */  
      if(pLinkContext->rxLength > pLinkContext->rxFrame.maxLength)
      {  
        DNPDIAG_ERROR(pLinkContext->tmw.pChannel, TMWDEFS_NULL, DNPDIAG_FRAME_OVERRUN);
        return;
      }

      pLinkContext->ctrl = pLinkContext->pRxBlock[3];

      /* Source and destination address */
      tmwtarg_get16(&pLinkContext->pRxBlock[4], &destAddress);
      tmwtarg_get16(&pLinkContext->pRxBlock[6], &srcAddress);

      /* Lookup session */
      if(destAddress >= DNPDEFS_BROADCAST_ADDR_NOCON)
      {
        pLinkSessionInfo = TMWDEFS_NULL;
        pLinkContext->pRxSession = TMWDEFS_NULL;
        pLinkContext->rxFrame.isBroadcast = TMWDEFS_TRUE;
        pLinkContext->rxFrame.rxAddress = destAddress;
      }
      else
      {
        DNPSESN *pDNPSession;

        /* Lookup session */
        pLinkContext->pRxSession = _findSession(pLinkContext, srcAddress, destAddress, pLinkContext->ctrl);
        if(pLinkContext->pRxSession == TMWDEFS_NULL)
        {
          /* If pRxSession is null then this is not for me.
           * If this is just link layer data discard it.
           * But if this is user data, accumulate it before discarding it to avoid  
           * inefficient (1 char at a time) processing and irritating log errors. 
           */
          DNPSTAT_CHNL_ERROR(pLinkContext->tmw.pChannel, TMWDEFS_NULL, TMWCHNL_ERROR_LINK_ADDRESS_UNKNOWN);
          DNPDIAG_LINK_ADDRESS_UNKNOWN(pLinkContext->tmw.pChannel, srcAddress, destAddress);

          if(pLinkContext->ctrl & DNPDEFS_LC_PRM_MASK)
          { 
            TMWTYPES_UCHAR temp = (pLinkContext->ctrl & DNPDEFS_LC_FUNCTION_MASK);
            if((temp == DNPDEFS_LCF_PRI_CNFRM_DATA)
              ||(temp == DNPDEFS_LCF_PRI_UNCNFRM_DATA))
             {
               /* Start receiving user data and then discard it later */
               pLinkContext->rxState = DNP_LINK_STATE_DISCARD_DATA;
               return;
            }
          }
          return;
        }

        /* Found a session, initialize rx info */
        pLinkContext->rxFrame.isBroadcast = TMWDEFS_FALSE;
        pLinkContext->rxFrame.rxAddress = srcAddress;

        /* Get pointer to DNP specific session info */
        pLinkSessionInfo = (DNPLINK_SESSION_INFO *)pLinkContext->pRxSession->pLinkSession;

        /* Since we received something, restart link status timer if configured. */
        pDNPSession = (DNPSESN *)pLinkContext->pRxSession;
        if(pDNPSession->linkStatusPeriod != 0)
        {
          tmwtimer_start(&pLinkSessionInfo->linkStatusTimer,
            pDNPSession->linkStatusPeriod, pLinkContext->pRxSession->pChannel, _linkStatusTimeout, pLinkContext->pRxSession);
        }

        /* Whenever we receive a frame from a session assume it is back online */
        tmwsesn_setOnline(pLinkContext->pRxSession, TMWDEFS_TRUE);
      }

     /* Received a valid link header, call user callback if specified */
#if TMWCNFG_SUPPORT_RXCALLBACKS
      if(pLinkContext->pRxHeaderCallback != TMWDEFS_NULL)
      {
        pLinkContext->pRxHeaderCallback(
          pLinkContext->pRxHeaderCallbackParam,
          pLinkContext->rxFrame.firstByteTime,
          pLinkContext->pRxBlock, &blockSize);
      }
#endif
      
      /* process message header */
      if(pLinkContext->ctrl & DNPDEFS_LC_PRM_MASK)
      {
        /* process primary messages */
        switch(pLinkContext->ctrl & DNPDEFS_LC_FUNCTION_MASK)
        {
        case DNPDEFS_LCF_PRI_LINK_RESET:
          /* Reset link states */

          /* Make sure this is not a broadcast message */
          if(pLinkContext->rxFrame.isBroadcast
            || pLinkSessionInfo == TMWDEFS_NULL)
          {
            DNPDIAG_ERROR(pLinkContext->tmw.pChannel,
              TMWDEFS_NULL, DNPDIAG_LINK_RESET_BRDCST);

            return;
          }
          
          /* Diagnostics */
          DNPDIAG_LINK_FRAME_RECEIVED(
            pLinkContext->tmw.pChannel, pLinkContext->pRxSession,
            pLinkContext->pRxBlock, DNPLINK_HEADER_SIZE);

          /* Update statistics */
          DNPSTAT_CHNL_FRAME_RCVD(pLinkContext->tmw.pChannel);

          /* Validate frame count bit */
          if(!_validateFCB(pLinkContext, pLinkSessionInfo, TMWDEFS_FALSE, 0))
            return;

          /* Process link reset */
          pLinkSessionInfo->linkIsReset = TMWDEFS_TRUE;

          /* Next frame count bit should be 1 */
          pLinkSessionInfo->expectedFCB = TMWDEFS_TRUE;

          /* Send acknowledge */
          _sendFixedFrame(pLinkContext, pLinkContext->pRxSession, pLinkContext->rxFrame.rxAddress,
            TMWDEFS_FALSE, DNPDEFS_LCF_SEC_CONFIRM_ACK, TMWDEFS_NULL, TMWDEFS_NULL);

          break;

        case DNPDEFS_LCF_PRI_TEST_LINK:
          /* Test link states */

          /* Make sure this is not a broadcast message */
          if (pLinkContext->rxFrame.isBroadcast
            || pLinkSessionInfo == TMWDEFS_NULL)
          {
            DNPDIAG_ERROR(pLinkContext->tmw.pChannel,
              TMWDEFS_NULL, DNPDIAG_TEST_LINK_BRDCST);

            return;
          }

          /* Diagnostics */
          DNPDIAG_LINK_FRAME_RECEIVED(pLinkContext->tmw.pChannel,
            pLinkContext->pRxSession, pLinkContext->pRxBlock, DNPLINK_HEADER_SIZE);

          /* Update statistics */
          DNPSTAT_CHNL_FRAME_RCVD(pLinkContext->tmw.pChannel);

          /* Validate reset */
          if(!_validateReset(pLinkContext, pLinkSessionInfo))
          {
            return;
          }

          /* Validate frame count bit */
          if(!_validateFCB(pLinkContext, pLinkSessionInfo,
            TMWDEFS_TRUE, pLinkSessionInfo->lastSecondaryConfirmFuncCode))
          {
            return;
          }

          /* Send acknowledge */
          _sendFixedFrame(pLinkContext, pLinkContext->pRxSession, pLinkContext->rxFrame.rxAddress,
            TMWDEFS_FALSE, DNPDEFS_LCF_SEC_CONFIRM_ACK, TMWDEFS_NULL, TMWDEFS_NULL);

          break;

        case DNPDEFS_LCF_PRI_CNFRM_DATA:
          /* Confirmed user data */

          /* Validate reset */
          if(!_validateReset(pLinkContext, pLinkSessionInfo))
            return;

          /* Don't validate FCB here, wait until entire message has been received
           * and all CRCs are validated
           */

          /* If frame timeout is specified start timer */
          if(pLinkContext->rxFrameTimeout != 0)
          {
            tmwtimer_start(&pLinkContext->frameTimer,
              pLinkContext->rxFrameTimeout, pLinkContext->tmw.pChannel,
              _rxFrameTimeout, pLinkContext);
          }

#if TMWCNFG_SUPPORT_DIAG
          /* Diagnostics */
          pLinkContext->pRxDiagContext = DNPDIAG_LINK_HEADER_BLOCK_RECEIVED(
            pLinkContext->tmw.pChannel, pLinkContext->pRxSession,
            pLinkContext->pRxBlock, DNPLINK_HEADER_SIZE);
#else
          pLinkContext->pRxDiagContext = TMWDEFS_NULL;
#endif
          /* Setup to start receiving user data */
          pLinkContext->rxState = DNP_LINK_STATE_USER_DATA;

          break;

        case DNPDEFS_LCF_PRI_UNCNFRM_DATA:
          /* Unconfirmed user data */

          /* Don't validate FCV here, wait until entire message has been received
           * and all CRCs are validated
           */

          /* If frame timeout was specified start timer */
          if(pLinkContext->rxFrameTimeout != 0)
          {
            tmwtimer_start(&pLinkContext->frameTimer,
              pLinkContext->rxFrameTimeout, pLinkContext->tmw.pChannel,
              _rxFrameTimeout, pLinkContext);
          }

#if TMWCNFG_SUPPORT_DIAG
          /* Diagnostics */
          pLinkContext->pRxDiagContext = DNPDIAG_LINK_HEADER_BLOCK_RECEIVED(
            pLinkContext->tmw.pChannel, pLinkContext->pRxSession,
            pLinkContext->pRxBlock, DNPLINK_HEADER_SIZE);
#else
          pLinkContext->pRxDiagContext = TMWDEFS_NULL;
#endif

          /* Setup to start receiving user data */
          pLinkContext->rxState = DNP_LINK_STATE_USER_DATA;

          break;

        case DNPDEFS_LCF_PRI_REQUEST_STATUS:
          /* Request link status */

          /* Make sure this is not a broadcast message */
          if(pLinkContext->rxFrame.isBroadcast)
          {
            DNPDIAG_ERROR(pLinkContext->tmw.pChannel,
              TMWDEFS_NULL, DNPDIAG_LINK_STAT_BRDCST);

            return;
          }

          /* Validate frame count bit */
          if(!_validateFCB(pLinkContext, pLinkSessionInfo, TMWDEFS_FALSE, 0))
            return;

          /* Diagnostics */
          DNPDIAG_LINK_FRAME_RECEIVED(
            pLinkContext->tmw.pChannel, pLinkContext->pRxSession,
            pLinkContext->pRxBlock, DNPLINK_HEADER_SIZE);

          /* Update statistics */
          DNPSTAT_CHNL_FRAME_RCVD(pLinkContext->tmw.pChannel);

          TMWSESN_STAT_CALLBACK_FUNC((TMWSESN *)pLinkContext->pRxSession, TMWSESN_STAT_DNPLINKSTATUSREQ_RECEIVED, TMWDEFS_NULL);

          /* Reply with status of link */
          _sendFixedFrame(pLinkContext, pLinkContext->pRxSession, pLinkContext->rxFrame.rxAddress,
            TMWDEFS_FALSE, DNPDEFS_LCF_SEC_STATUS_OF_LINK, TMWDEFS_NULL, TMWDEFS_NULL);

          break;

        default:
          /* Unsupported link layer function code */

          /* If broadcast address don't do anything */
          if(pLinkContext->rxFrame.isBroadcast)
          {
            DNPDIAG_ERROR(pLinkContext->tmw.pChannel,
              TMWDEFS_NULL, DNPDIAG_UNRECOG_BRDCST);

            return;
          }

          /* Diagnostics */
          DNPDIAG_LINK_FRAME_RECEIVED(
            pLinkContext->tmw.pChannel, pLinkContext->pRxSession,
            pLinkContext->pRxBlock, DNPLINK_HEADER_SIZE);

          /* Update statistics */
          DNPSTAT_CHNL_FRAME_RCVD(pLinkContext->tmw.pChannel);

          DNPSTAT_CHNL_ERROR(pLinkContext->tmw.pChannel, pLinkContext->pRxSession,
            TMWCHNL_ERROR_LINK_ILLEGAL_FUNCTION);

          /* Log error */
          DNPDIAG_ERROR(pLinkContext->tmw.pChannel, pLinkContext->pRxSession, 
            DNPDIAG_UNSUP_FC);

          /* Send not supported response */
          _sendFixedFrame(pLinkContext, pLinkContext->pRxSession, pLinkContext->rxFrame.rxAddress,
            TMWDEFS_FALSE, DNPDEFS_LCF_SEC_NOT_USED, TMWDEFS_NULL, _failedTxCallback);

          break;
        }
      }
      else
      {
        /* process secondary messages */

        /* Make sure this is not a broadcast message */
        if(pLinkContext->rxFrame.isBroadcast)
        {
          DNPDIAG_ERROR(pLinkContext->tmw.pChannel,
            TMWDEFS_NULL, DNPDIAG_SECONDARY_BRDCST);

          return;
        }

        /* Diagnostics */
        DNPDIAG_LINK_FRAME_RECEIVED(
          pLinkContext->tmw.pChannel, pLinkContext->pRxSession,
          pLinkContext->pRxBlock, DNPLINK_HEADER_SIZE);

        /* Update statistics */
        DNPSTAT_CHNL_FRAME_RCVD(pLinkContext->tmw.pChannel);
        
        pLinkContext->pTxSession = TMWDEFS_NULL;

        /* If waiting for an acknowledge to a reset and receive any other function code
         * go back to SecUnResetIdle (resetLinkWait==false)
         */
        if((pLinkSessionInfo->resetLinkWait)
          &&((pLinkContext->ctrl & DNPDEFS_LC_FUNCTION_MASK) != DNPDEFS_LCF_SEC_CONFIRM_ACK))
        {
          /* Cancel link confirm timer */
          tmwtimer_cancel(&pLinkContext->confirmTimer);
          pLinkSessionInfo->resetLinkWait = TMWDEFS_FALSE; 
          
          /* this is not a valid response to a link reset, 
           * spec says do NOT retry link reset unless timeout 
           * NACK case below would cause a retry.
           */
          _failedTxCallback(pLinkContext); 
          if((pLinkContext->ctrl & DNPDEFS_LC_FUNCTION_MASK) == DNPDEFS_LCF_SEC_CONFIRM_NACK)
          {
            /* See if this channel has more data to transmit on any session */
            _checkForData(pLinkContext, TMWDEFS_NULL);
            return;
          }
        }
        
        switch(pLinkContext->ctrl & DNPDEFS_LC_FUNCTION_MASK)
        {
        case DNPDEFS_LCF_SEC_CONFIRM_ACK:
          /* Acknowledge */

          if(tmwtimer_isActive(&pLinkContext->confirmTimer))
          { 
            /* Cancel link confirm timer */
            tmwtimer_cancel(&pLinkContext->confirmTimer);

            /* Are we waiting for an acknowledge to a reset */
            if(pLinkSessionInfo->resetLinkWait)
            {
              pLinkSessionInfo->secondaryStationIsReset = TMWDEFS_TRUE;
              pLinkSessionInfo->resetLinkWait = TMWDEFS_FALSE;
              pLinkSessionInfo->nextFCB = TMWDEFS_TRUE;
              _sendVariableFrame(pLinkContext);
            }
            else
            {
              /* Toggle frame count bit */
              pLinkSessionInfo->nextFCB =
               (TMWTYPES_BOOL)!pLinkSessionInfo->nextFCB;

              if(pLinkContext->pTxDescriptor != TMWDEFS_NULL)
              {
                TMWSESN_TX_DATA *pOldTxDescriptor;

                /* Make a copy of the TxDescriptor since the callback may want to transmit
                 * another frame
                 */
                pOldTxDescriptor = pLinkContext->pTxDescriptor;
                pLinkContext->pTxDescriptor = TMWDEFS_NULL;

                /* Call after transmission callback */
                if(pOldTxDescriptor->pAfterTxCallback != TMWDEFS_NULL)
                {
                  pOldTxDescriptor->pAfterTxCallback(
                    pOldTxDescriptor->pCallbackData, pOldTxDescriptor);
                }

                /* See if this session has more data to transmit */
                _checkForData(pLinkContext, pOldTxDescriptor->pSession);
                return;
              }
            }
          }
          break;

        case DNPDEFS_LCF_SEC_CONFIRM_NACK:
          /* Negative acknowledge */

          /* Cancel link confirm timer */
          tmwtimer_cancel(&pLinkContext->confirmTimer);

          /* Must do a reset link before sending more frames */
          pLinkSessionInfo->secondaryStationIsReset = TMWDEFS_FALSE;

          /* If this NACK was the result of an attempt to transmit a frame we
           * need to go ahead and send a link reset to get the retransmission
           * started.
           */
          if(pLinkContext->pTxDescriptor != TMWDEFS_NULL)
          {
            if((pLinkContext->confirmMode == TMWDEFS_LINKCNFM_ALWAYS)
              || ((pLinkContext->confirmMode == TMWDEFS_LINKCNFM_SOMETIMES)
                && ((pLinkContext->pTxDescriptor->pMsgBuf[0] & 0xc0) != 0xc0)))
            {
              pLinkSessionInfo->resetLinkWait = TMWDEFS_TRUE;

              _sendFixedFrame(pLinkContext, pLinkContext->pRxSession,  pLinkContext->rxFrame.rxAddress,
                TMWDEFS_TRUE, DNPDEFS_LCF_PRI_LINK_RESET, _afterTxCallback, _failedTxCallback);
            }
            else
            {
              /* Otherwise just try to retransmit the frame */
              _retryFrame(pLinkContext);
            }
          }
          break;

        case DNPDEFS_LCF_SEC_STATUS_OF_LINK:
          /* Link status */
          TMWSESN_STAT_CALLBACK_FUNC((TMWSESN *)pLinkContext->pRxSession, TMWSESN_STAT_DNPLINKSTATUS_RECEIVED, TMWDEFS_NULL);

          tmwtimer_cancel(&pLinkContext->confirmTimer);

          break;

        case DNPDEFS_LCF_SEC_NOT_USED:
        default:
          /* Not supported */
          DNPSTAT_CHNL_ERROR(pLinkContext->tmw.pChannel, pLinkContext->pRxSession,
            TMWCHNL_ERROR_LINK_ILLEGAL_FUNCTION);

          DNPDIAG_ERROR(pLinkContext->tmw.pChannel, pLinkContext->pRxSession,
            DNPDIAG_UNSUP_FC_RCVD);

          break;
        }
        
        /* See if this channel has more data to transmit on any session */
        _checkForData(pLinkContext, TMWDEFS_NULL);
      }
    }
  }
  else if(pLinkContext->rxState == DNP_LINK_STATE_USER_DATA)
  {
    /* Figure out how many bytes are needed to finish the current block */
    TMWTYPES_USHORT bytesNeeded = (TMWTYPES_USHORT)
      (pLinkContext->rxLength - pLinkContext->rxFrameOffset + 2);

    if(bytesNeeded > 18)
      bytesNeeded = 18;

    /* See if we have a complete block */
    if(pLinkContext->rxBlockOffset == bytesNeeded)
    {
      /* Yep, check the CRC */
      if(!_checkCRC(pLinkContext->pRxBlock, bytesNeeded))
      {
        DNPSTAT_CHNL_ERROR(pLinkContext->tmw.pChannel, pLinkContext->pRxSession,
          TMWCHNL_ERROR_LINK_INVALID_CHECKSUM);

        DNPDIAG_ERROR(pLinkContext->tmw.pChannel, pLinkContext->pRxSession,
          DNPDIAG_CRC_DATA); 

        pLinkContext->rxState = DNP_LINK_STATE_IDLE;
        pLinkContext->rxBlockOffset = 0;
        pLinkContext->rxFrameOffset = 0;

        /* Diagnostics */
        DNPDIAG_LINK_FRAME_COMPLETE(pLinkContext->pRxDiagContext, TMWDEFS_TRUE);
        pLinkContext->pRxDiagContext = TMWDEFS_NULL;

        return;
      }

      /* Diagnostics */
      DNPDIAG_LINK_DATA_BLOCK_RECEIVED(
        pLinkContext->pRxDiagContext,
        pLinkContext->pRxBlock, bytesNeeded);

      /* Copy user data into receive frame buffer */
      memcpy(pLinkContext->rxFrame.pMsgBuf + pLinkContext->rxFrameOffset, pLinkContext->pRxBlock, bytesNeeded - 2);
      pLinkContext->rxFrameOffset = (TMWTYPES_USHORT)(pLinkContext->rxFrameOffset + bytesNeeded - 2);

      /* Reset block offset to 0 */
      pLinkContext->rxBlockOffset = 0;

      /* See if we have the entire frame */
      if(pLinkContext->rxFrameOffset == pLinkContext->rxLength)
      {
        /* Record current time as last byte received time */
        pLinkContext->rxFrame.lastByteTime = tmwtarg_getMSTime();

        /* Cancel frame timer since we have received an entire frame */
        tmwtimer_cancel(&pLinkContext->frameTimer);

        /* Validate frame count bit */
        if(!pLinkContext->rxFrame.isBroadcast)
        {
          pLinkSessionInfo = (DNPLINK_SESSION_INFO *)pLinkContext->pRxSession->pLinkSession;
          if((pLinkContext->ctrl & DNPDEFS_LC_FUNCTION_MASK) == DNPDEFS_LCF_PRI_CNFRM_DATA)
          {
            if(!_validateFCB(pLinkContext, pLinkSessionInfo,
              TMWDEFS_TRUE, DNPDEFS_LCF_SEC_CONFIRM_ACK))
            {
              pLinkContext->rxState = DNP_LINK_STATE_IDLE;
              pLinkContext->rxBlockOffset = 0;
              pLinkContext->rxFrameOffset = 0;

              /* Diagnostics */
              DNPDIAG_LINK_FRAME_COMPLETE(pLinkContext->pRxDiagContext, TMWDEFS_TRUE);
              pLinkContext->pRxDiagContext = TMWDEFS_NULL;

              return;
            }
          }
          else
          {
            /* DNPDEFS_LCF_PRI_UNCNFRM_DATA, if FCV is set, discard this frame but do not transmit anything */
            if((pLinkContext->ctrl & DNPDEFS_LC_FCV_MASK) != 0x00)
            {
              /* Frame Count Valid bit not set */
              DNPDIAG_ERROR(pLinkContext->tmw.pChannel, pLinkContext->pRxSession, DNPDIAG_INVALID_FCV);

              pLinkContext->rxState = DNP_LINK_STATE_IDLE;
              pLinkContext->rxBlockOffset = 0;
              pLinkContext->rxFrameOffset = 0;

              /* Diagnostics */
              DNPDIAG_LINK_FRAME_COMPLETE(pLinkContext->pRxDiagContext, TMWDEFS_TRUE);
              pLinkContext->pRxDiagContext = TMWDEFS_NULL;

              return;
            }
          }
        }

        /* Update statistics */
        DNPSTAT_CHNL_FRAME_RCVD(pLinkContext->tmw.pChannel);

        /* Diagnostics */
        DNPDIAG_LINK_FRAME_COMPLETE(pLinkContext->pRxDiagContext, TMWDEFS_FALSE);
        pLinkContext->pRxDiagContext = TMWDEFS_NULL;
      
        /* Set received frame length */
        pLinkContext->rxFrame.msgLength = pLinkContext->rxFrameOffset;
   
        /* Go ahead and clear receive state */
        pLinkContext->rxState = DNP_LINK_STATE_IDLE;
        pLinkContext->rxFrameOffset = 0;

        /* If this was a broadcast message, or if the session
         * is active process frame
         */
        if((pLinkContext->pRxSession == TMWDEFS_NULL)
          || pLinkContext->pRxSession->active)
        {   
          
          /* See if we need to acknowledge the frame */
          if(!pLinkContext->rxFrame.isBroadcast &&
            ((pLinkContext->ctrl & 0x0f) == DNPDEFS_LCF_PRI_CNFRM_DATA))
          {
            /* pRxSession won't be null because this is Not a broadcast msg,
             * but protect this to make code checkers stop complaining.
             */
            if (pLinkContext->pRxSession != TMWDEFS_NULL)
            {
              _sendFixedFrame(pLinkContext, pLinkContext->pRxSession, pLinkContext->rxFrame.rxAddress,
                TMWDEFS_FALSE, DNPDEFS_LCF_SEC_CONFIRM_ACK, _afterAckCallback, _afterAckCallback);
            }
#ifdef TMW_SUPPORT_MONITOR 
           if(pLinkContext->tmw.pChannel->pPhysContext->monitorMode)
           {
             _finishDataFrame((DNPLINK_CONTEXT *)pCallbackParam);
           }
#endif
          }
          else
          {
            _finishDataFrame(pLinkContext);

            /* See if this channel has more data to transmit on this session */
            _checkForData(pLinkContext,  pLinkContext->pRxSession);
          }
        }
        else
        {  
          /* Session not active,*/
          DNPDIAG_ERROR(pLinkContext->tmw.pChannel,
            pLinkContext->pRxSession, DNPDIAG_INACTIVE_SESSION); 
        }
      }
    }
  }
  else if(pLinkContext->rxState == DNP_LINK_STATE_DISCARD_DATA)
  {   
   /* Figure out how many bytes are needed to finish the current block */
    TMWTYPES_USHORT bytesNeeded = (TMWTYPES_USHORT)
      (pLinkContext->rxLength - pLinkContext->rxFrameOffset + 2);

    if(bytesNeeded > 18)
      bytesNeeded = 18;

    /* See if we have a complete block */
    if(pLinkContext->rxBlockOffset == bytesNeeded)
    {
      pLinkContext->rxFrameOffset = (TMWTYPES_USHORT)(pLinkContext->rxFrameOffset + bytesNeeded - 2);

      /* Reset block offset to 0 */
      pLinkContext->rxBlockOffset = 0;

      /* See if we have the entire frame */
      if(pLinkContext->rxFrameOffset >= pLinkContext->rxLength)
      {
        pLinkContext->rxState = DNP_LINK_STATE_IDLE;
        pLinkContext->rxFrameOffset = 0;
        pLinkContext->rxLength = 0;
      }
    }
  }
}
    
/* function: dnplink_startLinkStatusTimer */
void dnplink_startLinkStatusTimer(
  TMWSESN *pSession)
{
  DNPSESN *pDNPSession = (DNPSESN *)pSession;
  if((pSession->pLinkSession != TMWDEFS_NULL)
    && (pDNPSession->linkStatusPeriod != 0))
  {
    DNPLINK_SESSION_INFO *pLinkSessionInfo = (DNPLINK_SESSION_INFO*)pSession->pLinkSession;
    tmwtimer_start(&pLinkSessionInfo->linkStatusTimer,
      pDNPSession->linkStatusPeriod, pSession->pChannel, _linkStatusTimeout, pSession);
  }
}

/* function: _openSession
 * purpose: opens a new session on this channel
 * arguments:
 *  pContext - link layer context
 *  pSession - pointer to session to open
 *  pConfig - currently not used by modbus
 * returns
 *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
 */
static TMWTYPES_BOOL TMWDEFS_CALLBACK _openSession(
  TMWLINK_CONTEXT *pContext,
  TMWSESN *pSession,
  void *pConfig)
{
  DNPLINK_CONTEXT *pLinkContext = (DNPLINK_CONTEXT *)pContext;
  DNPSESN *pDNPSession = (DNPSESN *)pSession;
  DNPLINK_SESSION_INFO *pLinkSessionInfo;

  TMWTARG_UNUSED_PARAM(pConfig);

  /* Protocol specific session initialization */
  pLinkSessionInfo = (DNPLINK_SESSION_INFO *)dnpmem_alloc(DNPMEM_LINK_SESSION_INFO_TYPE);
  if(pLinkSessionInfo == TMWDEFS_NULL)
  {
    return(TMWDEFS_FALSE);
  }

  pLinkSessionInfo->linkIsReset = TMWDEFS_FALSE;
  pLinkSessionInfo->sendUserLinkReset = TMWDEFS_FALSE;
  pLinkSessionInfo->secondaryStationIsReset = TMWDEFS_FALSE;
  pLinkSessionInfo->resetLinkWait = TMWDEFS_FALSE;
  pLinkSessionInfo->expectedFCB = TMWDEFS_FALSE;
  pLinkSessionInfo->nextFCB = TMWDEFS_FALSE;
  pLinkSessionInfo->lastSecondaryConfirmFuncCode = DNPDEFS_LCF_SEC_CONFIRM_ACK;
  pSession->pLinkSession = pLinkSessionInfo;

  /* Open channel if required */
  tmwlink_openChannel(pContext);

  /* Generic session initialization */
  if(!tmwlink_openSession(pContext, pSession))
  {
    dnpmem_free(pSession->pLinkSession);
    return(TMWDEFS_FALSE);
  }

  /* If channel is already open tell application layer we are online */
  if(pContext->isOpen)
  {
    pLinkContext->tmw.pInfoFunc(
      pLinkContext->tmw.pCallbackParam,
      pSession, TMWSCL_INFO_ONLINE);

    /* Indicate session is online, if this is TCP */  
    if((pLinkContext->networkType == DNPLINK_NETWORK_TCP_ONLY)
      || (pLinkContext->networkType == DNPLINK_NETWORK_TCP_UDP))
    {
      tmwsesn_setOnline(pSession, TMWDEFS_TRUE);
    }
  }

  tmwtimer_init(&pLinkSessionInfo->linkStatusTimer);
  if(pDNPSession->linkStatusPeriod != 0)
  {
    tmwtimer_start(&pLinkSessionInfo->linkStatusTimer,
      pDNPSession->linkStatusPeriod, pSession->pChannel, _linkStatusTimeout, pSession);
  }

  return(TMWDEFS_TRUE);
}

/* function: _closeSession
 * purpose: closes specified session
 * arguments:
 *  pContext - link layer context
 *  pSession - pointer to session to close
 * returns
 *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
 */
static TMWTYPES_BOOL TMWDEFS_CALLBACK _closeSession(
  TMWLINK_CONTEXT *pContext,
  TMWSESN *pSession)
{
  DNPLINK_CONTEXT *pLinkContext = (DNPLINK_CONTEXT *)pContext;
  DNPLINK_SESSION_INFO *pLinkSessionInfo = (DNPLINK_SESSION_INFO *)pSession->pLinkSession;
  DNPCHNL *pDNPChannel = (DNPCHNL*)pSession->pChannel;

  /* Cancel link status timer if required */
  tmwtimer_cancel(&pLinkSessionInfo->linkStatusTimer);

  if (pLinkContext->pRxSession == pSession)
  {
    /* if this session was actively receiving data reset it. */
    pLinkContext->rxState = DNP_LINK_STATE_IDLE;
    pLinkContext->rxBlockOffset = 0;
    pLinkContext->rxFrameOffset = 0;
  }

  if(pLinkContext->pTxSession == pSession)
  {
    tmwtimer_cancel(&pLinkContext->confirmTimer);
    pLinkContext->pTxSession = TMWDEFS_NULL;
    pLinkContext->pTxDescriptor = TMWDEFS_NULL;
  } 
 
  if(pDNPChannel->pLastSessionRequest == pSession)
  {
    pDNPChannel->pLastSessionRequest = TMWDEFS_NULL;
  }

  /* Pass online info to application level */
  if(pLinkContext->tmw.pInfoFunc != TMWDEFS_NULL)
  {
    pLinkContext->tmw.pInfoFunc(pLinkContext->tmw.pCallbackParam,
      pSession, TMWSCL_INFO_OFFLINE);

    /* Session is offline */
    tmwsesn_setOnline(pSession, TMWDEFS_FALSE);
  }

  /* Protocol specific session shutdown */
  dnpmem_free(pSession->pLinkSession);

  /* Generic session shutdown and return */
  return(tmwlink_closeSession(pContext, pSession));
}

/* Global function implementation */

/* function: dnplink_initConfig */
void TMWDEFS_GLOBAL dnplink_initConfig(
  DNPLINK_CONFIG *pConfig)
{
  pConfig->offlinePollPeriod = TMWDEFS_SECONDS(10);
  pConfig->confirmMode = TMWDEFS_LINKCNFM_NEVER;
  pConfig->confirmTimeout = TMWDEFS_SECONDS(2);
  pConfig->maxRetries = 3;
  pConfig->rxFrameTimeout = TMWDEFS_SECONDS(15);
  pConfig->rxFrameSize = DNPCNFG_MAX_RX_FRAME_LENGTH;
  pConfig->txFrameSize = DNPCNFG_MAX_TX_FRAME_LENGTH;
  pConfig->networkType = DNPLINK_NETWORK_NO_IP;
}

/* function: dnplink_initChannel */
TMWTYPES_BOOL TMWDEFS_GLOBAL dnplink_initChannel(
  TMWCHNL *pChannel,
  const DNPLINK_CONFIG *pConfig)
{
  static const TMWLINK_INTERFACE _dnplinkInterface = {
    (TMWLINK_OPEN_SESSION_FUNC)_openSession,
    (TMWLINK_CLOSE_SESSION_FUNC)_closeSession,
    (TMWLINK_GET_SESSIONS_FUNC)_getSessions,
    (TMWLINK_DATA_READY_FUNC)_checkDataReady,
    (TMWLINK_SET_CALLBACKS_FUNC)tmwlink_setCallbacks,
    (TMWLINK_TRANSMIT_FUNC)_transmitFrame,
    (TMWLINK_UPDATE_MSG_FUNC)_updateMsg,
    (TMWLINK_CANCEL_FUNC)_cancelFrame
  };
  DNPLINK_CONTEXT *pLinkContext;

  /* Check configuration 
   * maximum is set by spec and limited by 1 octet length field.
   * minimum of 24 is not in spec, but anything that small is not useful
   */
  if((pConfig->rxFrameSize < 24)
    || (pConfig->txFrameSize < 24)
     || (pConfig->rxFrameSize > 292)
     || (pConfig->txFrameSize > 292))
  {
    DNPDIAG_ERROR(pChannel, TMWDEFS_NULL, DNPDIAG_INVALID_FRSIZE);
    ASSERT(TMWDEFS_FALSE);
    return(TMWDEFS_FALSE);
  }

  /* Allocate space for context */
  pLinkContext = (DNPLINK_CONTEXT *)dnpmem_alloc(DNPMEM_LINK_CONTEXT_TYPE);
  if (pLinkContext == TMWDEFS_NULL)
  {
    ASSERT(TMWDEFS_FALSE);
    return(TMWDEFS_FALSE);
  }

#if TMWCNFG_USE_DYNAMIC_MEMORY && !TMWCNFG_ALLOC_ONLY_AT_STARTUP 
  /* Allocate receive and transmit frame buffer */  
  /* Receive buffer does not need room for header and CRCs */
  pLinkContext->rxFrame.maxLength = dnputil_linkFrameSizeToTprt(pConfig->rxFrameSize);
  pLinkContext->rxFrame.pMsgBuf = (TMWTYPES_UCHAR *)
    tmwtarg_alloc(dnputil_linkFrameSizeToTprt(pConfig->rxFrameSize));
  if (pLinkContext->rxFrame.pMsgBuf == TMWDEFS_NULL)
  {
    DNPDIAG_ERROR(pChannel, TMWDEFS_NULL, DNPDIAG_RX_BUFFER);
    dnpmem_free(pLinkContext);
    ASSERT(TMWDEFS_FALSE);
    return(TMWDEFS_FALSE);
  }
  
  /* Transmit buffer needs to hold header and CRCs */
  pLinkContext->physTxDescriptor.pTxBuffer =
    (TMWTYPES_UCHAR *)tmwtarg_alloc(pConfig->txFrameSize);
  if (pLinkContext->physTxDescriptor.pTxBuffer == TMWDEFS_NULL)
  {
    DNPDIAG_ERROR(pChannel, TMWDEFS_NULL, DNPDIAG_TX_BUFFER);
    dnpmem_free(pLinkContext);
    tmwtarg_free(pLinkContext->rxFrame.pMsgBuf);
    ASSERT(TMWDEFS_FALSE);
    return(TMWDEFS_FALSE);
  }
#else
  /* Check configuration */
  if((pConfig->rxFrameSize > DNPCNFG_MAX_RX_FRAME_LENGTH)
    || (pConfig->txFrameSize > DNPCNFG_MAX_TX_FRAME_LENGTH))
  {
    DNPDIAG_ERROR(pChannel, TMWDEFS_NULL, DNPDIAG_INVALID_FRSIZE);
    ASSERT(TMWDEFS_FALSE);
    return(TMWDEFS_FALSE);
  }

  /* Point to receive and transmit frame buffer */
  pLinkContext->rxFrame.maxLength = DNPCNFG_MAX_RX_FRAME_LENGTH;
  pLinkContext->rxFrame.pMsgBuf = pLinkContext->rxFrameBuffer;
  pLinkContext->physTxDescriptor.pTxBuffer = pLinkContext->txFrameBuffer;
#endif
  pLinkContext->ackPhysTxDescriptor.pTxBuffer = pLinkContext->ackTxFrameBuffer;

  /* Set link layer points in channel */
  pLinkContext->tmw.pChannel = pChannel;
  pChannel->pLink = &_dnplinkInterface;
  pChannel->pLinkContext = (TMWLINK_CONTEXT *)pLinkContext;

  /* Initialize Timers */
  tmwtimer_init(&pLinkContext->frameTimer);
  tmwtimer_init(&pLinkContext->confirmTimer);

  /* Initialize generic link layer info */
  (void)tmwlink_initChannel(pChannel,pConfig->offlinePollPeriod,
    tmwlink_channelCallback, dnplink_getNeededBytes, dnplink_parseBytes, 
    _infoCallback, _checkAddressMatchCallback);

  /* Store config */
  pLinkContext->confirmMode = pConfig->confirmMode;
  pLinkContext->confirmTimeout = pConfig->confirmTimeout;
  pLinkContext->maxRetries = pConfig->maxRetries;
  pLinkContext->rxFrameTimeout = pConfig->rxFrameTimeout;
  pLinkContext->rxFrameSize = pConfig->rxFrameSize;
  pLinkContext->txFrameSize = pConfig->txFrameSize;
  pLinkContext->networkType  = pConfig->networkType;

  /* Initialize context */
  pLinkContext->rxBlockOffset = 0;
  pLinkContext->rxFrameOffset = 0;
  pLinkContext->pTxDescriptor = TMWDEFS_NULL;
  pLinkContext->pTxSession = TMWDEFS_NULL;
  pLinkContext->pRxSession = TMWDEFS_NULL;
  pLinkContext->rxState = DNP_LINK_STATE_IDLE;
  pLinkContext->pRxDiagContext = TMWDEFS_NULL;

#if TMWCNFG_SUPPORT_RXCALLBACKS
  pLinkContext->pRxHeaderCallback = TMWDEFS_NULL;
  pLinkContext->pRxHeaderCallbackParam = TMWDEFS_NULL;
#endif

  /* Open channel (try to connect) if auto open session callback is registered */
  if(pChannel->pAutoOpenCallback != TMWDEFS_NULL)
    tmwlink_openChannel((TMWLINK_CONTEXT *)pLinkContext);
#ifdef TMW_SUPPORT_MONITOR 
  /* Or if monitor mode try to connect */
  else if(pChannel->pPhysContext->monitorMode == TMWDEFS_TRUE)
    tmwlink_openChannel((TMWLINK_CONTEXT *)pLinkContext);
#endif

  return(TMWDEFS_TRUE);
}
 
/* function: dnplink_initChannel */
TMWTYPES_BOOL TMWDEFS_GLOBAL dnplink_reInitChannel(
  TMWCHNL *pChannel,
  TMWTYPES_USHORT rxFrameSize,
  TMWTYPES_USHORT txFrameSize)
{
#if TMWCNFG_USE_DYNAMIC_MEMORY && !TMWCNFG_ALLOC_ONLY_AT_STARTUP 
  DNPLINK_CONTEXT *pLinkContext = (DNPLINK_CONTEXT *)pChannel->pLinkContext;
  TMWTYPES_UCHAR *ptr = TMWDEFS_NULL;
  /* Allocate receive and transmit frame buffer */
  if (rxFrameSize != 0)
  {
    /* Receive buffer does not need room for header and CRCs */
    TMWTYPES_USHORT maxLength = dnputil_linkFrameSizeToTprt(rxFrameSize);
    ptr = (TMWTYPES_UCHAR *)tmwtarg_alloc(maxLength);
    if (ptr != TMWDEFS_NULL)
    {
      tmwtarg_free(pLinkContext->rxFrame.pMsgBuf);
      pLinkContext->rxFrameSize = rxFrameSize;
      pLinkContext->rxFrame.maxLength = maxLength;
      pLinkContext->rxFrame.pMsgBuf = ptr;

      pLinkContext->rxState = DNP_LINK_STATE_IDLE;
      pLinkContext->rxBlockOffset = 0;
      pLinkContext->rxFrameOffset = 0;
    }
    else
    {
      return TMWDEFS_FALSE;
    }
  }

  if (txFrameSize != 0)
  {
    /* Transmit buffer needs to hold header and CRCs */
    ptr = (TMWTYPES_UCHAR *)tmwtarg_alloc(txFrameSize);
    if (ptr != TMWDEFS_NULL)
    {
      tmwtarg_free(pLinkContext->physTxDescriptor.pTxBuffer);
      pLinkContext->physTxDescriptor.pTxBuffer = ptr;
      pLinkContext->txFrameSize = txFrameSize;
    }
    else
    {
      return TMWDEFS_FALSE;
    }
  }
#else 
  TMWTARG_UNUSED_PARAM(pChannel);
  TMWTARG_UNUSED_PARAM(rxFrameSize);
  TMWTARG_UNUSED_PARAM(txFrameSize);
#endif

  return TMWDEFS_TRUE;
}

/* function: dnplink_modifyChannel */
TMWTYPES_BOOL TMWDEFS_GLOBAL dnplink_modifyChannel(
  TMWCHNL *pChannel,
  const DNPLINK_CONFIG *pConfig,
  TMWTYPES_ULONG configMask)
{
  DNPLINK_CONTEXT *pContext = (DNPLINK_CONTEXT *)pChannel->pLinkContext;

  if((configMask & DNPLINK_CONFIG_RX_FRAME_SIZE) != 0)
  {
    pContext->rxFrameSize = pConfig->rxFrameSize;
  }

  if((configMask & DNPLINK_CONFIG_RX_FRAME_TIMEOUT) != 0)
  {
    pContext->rxFrameTimeout = pConfig->rxFrameTimeout;
  }

  if((configMask & DNPLINK_CONFIG_CONFIRM_MODE) != 0)
  {
    pContext->confirmMode = pConfig->confirmMode;
  }

  if((configMask & DNPLINK_CONFIG_CONFIRM_TIMEOUT) != 0)
  {
    pContext->confirmTimeout = pConfig->confirmTimeout;
  }

  if((configMask & DNPLINK_CONFIG_MAX_RETRIES) != 0)
  {
    pContext->maxRetries = pConfig->maxRetries;
  }

  return(TMWDEFS_TRUE);
}

/* function: dnplink_deleteChannel */
TMWTYPES_BOOL TMWDEFS_GLOBAL dnplink_deleteChannel(
  TMWCHNL *pChannel)
{
  DNPLINK_CONTEXT *pLinkContext = (DNPLINK_CONTEXT *)pChannel->pLinkContext;

  /* Cleanup generic link layer info */
  if(tmwlink_deleteChannel(pChannel) == TMWDEFS_FALSE)
  {
    return(TMWDEFS_FALSE);
  }

#if TMWCNFG_USE_DYNAMIC_MEMORY && !TMWCNFG_ALLOC_ONLY_AT_STARTUP 
  /* Free receive and transmit frame buffer */
  tmwtarg_free(pLinkContext->rxFrame.pMsgBuf);
  tmwtarg_free(pLinkContext->physTxDescriptor.pTxBuffer);
#endif

  /* Cancel timers */
  tmwtimer_cancel(&pLinkContext->confirmTimer);
  tmwtimer_cancel(&pLinkContext->frameTimer);

  /* Free diag memory if it is still allocated */
  if(pLinkContext->pRxDiagContext != TMWDEFS_NULL)
  {
    dnpmem_free(pLinkContext->pRxDiagContext);
  }

  /* Free context */
  dnpmem_free(pLinkContext);

  return(TMWDEFS_TRUE);
}

/* function: dnplink_linkReset */
TMWTYPES_BOOL TMWDEFS_GLOBAL dnplink_linkReset(
  TMWSESN *pSession)
{
  DNPLINK_CONTEXT *pLinkContext = (DNPLINK_CONTEXT *)pSession->pChannel->pLinkContext;
  DNPLINK_SESSION_INFO *pLinkSessionInfo = (DNPLINK_SESSION_INFO *)pSession->pLinkSession;

  /* First see if we are not already trying to transmit something */
  if( (pLinkContext->pTxDescriptor != TMWDEFS_NULL) 
    ||(pLinkContext->pTxSession != TMWDEFS_NULL))
  {
    /* reset will be sent when current transmit is finished */
    pLinkSessionInfo->sendUserLinkReset = TMWDEFS_TRUE;

    /* indicate success since it WILL happen soon */
    return(TMWDEFS_TRUE);
  }

  pLinkSessionInfo->resetLinkWait = TMWDEFS_TRUE;

  _sendFixedFrame(pLinkContext, pSession, pSession->destAddress,
    TMWDEFS_TRUE, DNPDEFS_LCF_PRI_LINK_RESET, _afterTxCallback, _failedTxCallback);

  return(TMWDEFS_TRUE);
}

/* function: dnplink_testLinkState */
TMWTYPES_BOOL TMWDEFS_GLOBAL dnplink_testLinkState(
  TMWSESN *pSession)
{
  DNPLINK_CONTEXT *pLinkContext = (DNPLINK_CONTEXT *)pSession->pChannel->pLinkContext;

  /* First see if we are not already trying to transmit something */
  if( (pLinkContext->pTxDescriptor != TMWDEFS_NULL)
    ||(pLinkContext->pTxSession != TMWDEFS_NULL))
    return(TMWDEFS_FALSE);

  _sendFixedFrame(pLinkContext, pSession, pSession->destAddress,
    TMWDEFS_TRUE, DNPDEFS_LCF_PRI_TEST_LINK, _afterTxCallback, _failedTxCallback);

  return(TMWDEFS_TRUE);
}

/* function: dnplink_linkStatus */
TMWTYPES_BOOL TMWDEFS_GLOBAL dnplink_linkStatus(
  TMWSESN *pSession)
{
  DNPLINK_CONTEXT *pLinkContext = (DNPLINK_CONTEXT *)pSession->pChannel->pLinkContext;

  if( (pLinkContext->pTxDescriptor != TMWDEFS_NULL)
    ||(pLinkContext->pTxSession != TMWDEFS_NULL))
    return(TMWDEFS_FALSE);

  /* Send link status request */
  _sendFixedFrame(pLinkContext, pSession, pSession->destAddress,
    TMWDEFS_TRUE, DNPDEFS_LCF_PRI_REQUEST_STATUS, _afterTxCallback, _failedTxCallback);

  return(TMWDEFS_TRUE);
}

/* function: dnplink_rechecksum */
void TMWDEFS_GLOBAL dnplink_rechecksum(TMWPHYS_TX_DESCRIPTOR *pPhysTxDescriptor)
{
  TMWTYPES_USHORT totalBytesToTx;
  TMWTYPES_USHORT localOffset;  
  
  localOffset = DNPDEFS_LINK_HDR_AND_CRC_SIZE;
  totalBytesToTx = pPhysTxDescriptor->numBytesToTx;
  while(localOffset < totalBytesToTx)
  {
    TMWTYPES_USHORT bytesLeft = (TMWTYPES_USHORT)(totalBytesToTx - localOffset);
    TMWTYPES_USHORT bytesToTransmit = (TMWTYPES_USHORT)((DNPLINK_BLOCK_SIZE < bytesLeft) ? DNPLINK_BLOCK_SIZE : bytesLeft);
    _setCRC(pPhysTxDescriptor->pTxBuffer+localOffset, (TMWTYPES_USHORT)(bytesToTransmit-DNPLINK_CRC_SIZE));

    localOffset = (TMWTYPES_USHORT)(localOffset + bytesToTransmit);
  }
}

/* function: dnplink_closeAndOpen */
void TMWDEFS_GLOBAL dnplink_closeAndOpen(
  TMWSESN *pSession)
{
  DNPLINK_CONTEXT *pLinkContext = (DNPLINK_CONTEXT *)pSession->pChannel->pLinkContext;
        
  /* Disconnect and reconnect */
  tmwlink_closeChannel((TMWLINK_CONTEXT *)pLinkContext);

  pLinkContext->tmw.pInfoFunc(pLinkContext->tmw.pCallbackParam,
     pSession, TMWSCL_INFO_OFFLINE);
  
  tmwlink_openChannel((TMWLINK_CONTEXT *)pLinkContext);
}

#if 0
/* function: dnplink_txTestFrame */
void dnplink_txTestFrame(
  TMWSESN *pSession,
  TMWSESN_TX_DATA *pTxDescriptor)
{
  DNPLINK_CONTEXT *pLinkContext = (DNPLINK_CONTEXT *)pSession->pChannel->pLinkContext; 

  /* Initialize context */
  pLinkContext->retryCount = 0;
  pLinkContext->pTxDescriptor = pTxDescriptor;

  /* Call function to send variable frame */
  _sendVariableFrame(pLinkContext);
}
#endif
