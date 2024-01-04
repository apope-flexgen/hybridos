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

/* file: tmwphys.c
 * description: Generic physical layer implementation. This file implements
 *  a physical layer interface as defined in tmwphys.h. This physical layer
 *  can be used by any of the TMW protocol.
 */
#include "tmwscl/utils/tmwtarg.h"
#include "tmwscl/utils/tmwphys.h"
#include "tmwscl/utils/tmwtimer.h"
#include "tmwscl/utils/tmwchnl.h"
#include "tmwscl/utils/tmwphysd.h"

/* Forward declarations */

static TMWTYPES_BOOL TMWDEFS_LOCAL _transmit(
  TMWPHYS_CONTEXT *pContext);

static TMWTYPES_BOOL TMWDEFS_CALLBACK _receiveBytes(
  TMWPHYS_CONTEXT *pContext,
  TMWTYPES_MILLISECONDS maxTimeout);

static void TMWDEFS_CALLBACK _receiveCallback(
  void *pCallbackParam);

static void TMWDEFS_CALLBACK _transmitReadyCallback(
  void *pCallbackParam);

static void TMWDEFS_CALLBACK _transmitReadyTimeout(
  void *pCallbackParam);

static void TMWDEFS_CALLBACK _firstCharTimeout(
  void *pCallbackParam);

static void TMWDEFS_CALLBACK _channelCallback(
  void *pCallbackParam,
  TMWTYPES_BOOL openOrClose,
  TMWDEFS_TARG_OC_REASON reason);

static TMWPHYS_ADDRESS_MATCH_TYPE TMWDEFS_CALLBACK _checkAddressCallback(
  void *pCallbackParam,
  TMWTYPES_UCHAR *buf,
  TMWTYPES_USHORT numBytes,
  TMWTYPES_MILLISECONDS firstByteTime);
 
static void TMWDEFS_CALLBACK _setCallbacks(
  TMWPHYS_CONTEXT *pContext,
  TMWPHYS_CHANNEL_FUNC pChannelFunc,
  TMWPHYS_NEEDED_CHARS_FUNC pNeededCharsFunc,
  TMWPHYS_PARSE_FUNC pParseFunc,
  TMWPHYS_CHECK_ADDRESS_FUNC pCheckAddressFunc,
  void *pCallbackParam);

static TMWTYPES_BOOL TMWDEFS_CALLBACK _transmitBytes(
  TMWPHYS_CONTEXT *pContext,
  const TMWPHYS_TX_DESCRIPTOR *pTxDescriptor);

/* Callback functions. These functions may be called from outside
 * the SCL so we need to handle threading issues
 */

/* function: _receiveCallback
 * purpose:  Receive callback function called from target layer
 *  Notifies SCL that there is data to receive.
 * arguments:
 *  pCallbackParam - contains pointer to context
 * returns:
 *  void
 */
static void TMWDEFS_CALLBACK _receiveCallback(
  void *pCallbackParam)
{
  int checkForRcvCount;
  TMWPHYS_CONTEXT *pContext = (TMWPHYS_CONTEXT *)pCallbackParam;
 
  /* If close has been sent to target layer, don't get lock, just return */
  if(pContext->channelOpen == TMWPHYS_CLOSED)
    return;

  /* Lock channel */
  TMWTARG_LOCK_SECTION(&pContext->pChannel->lock);

  /* Loop here till there is no data received or count exceeded */
  checkForRcvCount = 1;
  while(_receiveBytes(pContext, 0UL) == TMWDEFS_TRUE)
  { 
    if(++checkForRcvCount > TMWCNFG_MAX_APPLRCVS)
    {
      break;
    }
  }

  /* Unlock channel */
  TMWTARG_UNLOCK_SECTION(&pContext->pChannel->lock);
}

/* function: _transmitReadyCallback
 * purpose: Handle channel ready callback from target layer
 * arguments:
 *  pCallbackParam - contains pointer to context
 * returns:
 *  void
 */
static void TMWDEFS_CALLBACK _transmitReadyCallback(
  void *pCallbackParam)
{
  const TMWPHYS_TX_DESCRIPTOR *pOldTxDescriptor;
  TMWTYPES_BOOL transmitOk;
  TMWPHYS_CONTEXT *pContext = (TMWPHYS_CONTEXT *)pCallbackParam;
  
  /* If close has been sent to target layer, don't get lock, just return */
  if(pContext->channelOpen == TMWPHYS_CLOSED)
    return;

  /* Lock channel */
  TMWTARG_LOCK_SECTION(&pContext->pChannel->lock);

  /* cancel timer since we have gotten the callback instead. */
  tmwtimer_cancel(&pContext->transmitReadyTimer);

   /* Check to make sure tx descriptor is not null */
  if(pContext->pTxDescriptor == (const TMWPHYS_TX_DESCRIPTOR *)TMWDEFS_NULL)
  {
    /* Unlock channel */
    TMWTARG_UNLOCK_SECTION(&pContext->pChannel->lock);
    return;
  }

  /* Link level callback */
  if(pContext->pTxDescriptor->beforeTxCallback)
  {
    pContext->pTxDescriptor->beforeTxCallback(pContext->pTxDescriptor->pCallbackParam);
  }

  /* Low level transmit routine */
  transmitOk = tmwtarg_transmit(pContext->pIOContext,
    pContext->pTxDescriptor->pTxBuffer, pContext->pTxDescriptor->numBytesToTx);

  pOldTxDescriptor = pContext->pTxDescriptor;
  pContext->pTxDescriptor = TMWDEFS_NULL;

  if(transmitOk)
  {
    TMWTYPES_USHORT bytesSent = pOldTxDescriptor->numBytesToTx;

    /* Diagnostics */
    TMWPHYSD_BYTES_SENT(pContext->pChannel,
      pOldTxDescriptor->pTxBuffer, bytesSent);

    /* Statistics */
    TMWCHNL_STAT_CALLBACK_FUNC(pContext->pChannel,
      TMWCHNL_STAT_BYTES_SENT, &bytesSent);

    /* Link level success callback */
    if(pOldTxDescriptor->afterTxCallback)
    {
      pOldTxDescriptor->afterTxCallback(pOldTxDescriptor->pCallbackParam);
    }
  }
  else
  {
    TMWCHNL_STAT_ERROR_TYPE errorInfo;

    /* Diagnostics */
    TMWPHYSD_ERROR(pContext->pChannel, "Error writing to physical channel");

    /* Statistics */
    errorInfo.errorCode = TMWCHNL_ERROR_PHYS_TRANSMIT;
    errorInfo.pSession = TMWDEFS_NULL;
    TMWCHNL_STAT_CALLBACK_FUNC(pContext->pChannel, TMWCHNL_STAT_ERROR, &errorInfo);

    /* Link level failure callback */
    if((pOldTxDescriptor != (const TMWPHYS_TX_DESCRIPTOR *)TMWDEFS_NULL)
      &&(pOldTxDescriptor->failedTxCallback != TMWDEFS_NULL))
    {
      pOldTxDescriptor->failedTxCallback(pOldTxDescriptor->pCallbackParam);
    }
  }

  /* Unlock channel */
  TMWTARG_UNLOCK_SECTION(&pContext->pChannel->lock);

  return;
}

/* function: _transmitReadyTimeout
 * purpose: Handle channel ready timeout
 * arguments:
 *  pCallbackParam - contains pointer to context
 * returns:
 *  void
 */
static void TMWDEFS_CALLBACK _transmitReadyTimeout(
  void *pCallbackParam)
{
  TMWPHYS_CONTEXT *pContext = (TMWPHYS_CONTEXT *)pCallbackParam;

  /* There should always be something to transmit since the only way we
   * start the transmitReadyTimer is when we need to transmit something
   */
  (void)_transmit(pContext);
}

/* function: _firstCharTimeout
 * purpose: Handle firstCharWait timeout
 * arguments:
 *  pCallbackParam - contains pointer to context
 * returns:
 *  void
 */
static void TMWDEFS_CALLBACK _firstCharTimeout(
  void *pCallbackParam)
{
  TMWPHYS_CONTEXT *pContext = (TMWPHYS_CONTEXT *)pCallbackParam;

  /* If there is something to transmit, send it */
  if(pContext->pTxDescriptor != (const TMWPHYS_TX_DESCRIPTOR *)TMWDEFS_NULL)
  {
    (void)_transmit(pContext);
  }
}

/* function: _channelCallback
 * purpose: callback from low level I/O routines when
 *  channel is opened or closed outside the SCL.
 * arguments:
 *  pCallbackParam - user parameter
 * returns:
 *  void
 */
static void TMWDEFS_CALLBACK _channelCallback(
  void *pCallbackParam,
  TMWTYPES_BOOL openOrClose,
  TMWDEFS_TARG_OC_REASON reason)
{
  TMWPHYS_CONTEXT *pContext = (TMWPHYS_CONTEXT *)pCallbackParam;
  TMWCHNL *pChannel = pContext->pChannel;
  TMWAPPL *pApplContext = pChannel->pApplContext;
  TMWTARG_UNUSED_PARAM(reason);

  /* If delete or close has been sent to target layer, don't try to get channel lock, just return */
  if (!tmwdlist_containsEntry((TMWDLIST*)&pApplContext->channels, (TMWDLIST_MEMBER*)pChannel) 
    || (pContext->channelOpen == TMWPHYS_CLOSED))
  { 
    return;
  }
  
  /* Lock channel */
  TMWTARG_LOCK_SECTION(&pChannel->lock);

  if(pContext->pChannelFunc != TMWDEFS_NULL)
  {
    pContext->pChannelFunc(pContext->pCallbackParam, openOrClose, reason);
  }

  /* Unlock channel */
  TMWTARG_UNLOCK_SECTION(&pChannel->lock);

  return;
}

/* function: _checkAddressCallback
 * purpose: callback from low level I/O routines when data is received
 *   to determine if this is the correct channel for the data. This could
 *   be used by modem pool to handle unsolicited messages arriving when
 *   channel is not attached to a modem.
 * arguments:
 *  pCallbackParam - user parameter
 * returns:
 *  void
 */
static TMWPHYS_ADDRESS_MATCH_TYPE TMWDEFS_CALLBACK _checkAddressCallback(
  void *pCallbackParam,
  TMWTYPES_UCHAR *buf,
  TMWTYPES_USHORT numBytes,
  TMWTYPES_MILLISECONDS firstByteTime)
{
  TMWPHYS_ADDRESS_MATCH_TYPE status = TMWPHYS_ADDRESS_MATCH_FAILED;
  TMWPHYS_CONTEXT *pContext = (TMWPHYS_CONTEXT *)pCallbackParam;
 
  /* If close has been sent to target layer, don't get lock, just return */
  if(pContext->channelOpen == TMWPHYS_CLOSED)
    return(status);

  /* Lock channel */
  TMWTARG_LOCK_SECTION(&pContext->pChannel->lock);

  if(pContext->pCheckAddressFunc != TMWDEFS_NULL)
  {
    status = pContext->pCheckAddressFunc(pContext->pCallbackParam, buf, numBytes, firstByteTime);
  }

  /* Unlock channel */
  TMWTARG_UNLOCK_SECTION(&pContext->pChannel->lock);

  return(status);
}


/* Local function implementations */

/* function: _openChannel
 * purpose: open this channel
 * arguments:
 *  pContext - context returned from call to tmwphys_initChannel
 * returns:
 *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
 */
static TMWTYPES_BOOL TMWDEFS_CALLBACK _openChannel(
  TMWPHYS_CONTEXT *pContext)
{
  /* Initialization */
  pContext->channelOpen = TMWPHYS_OPENING;

  /* Call target open function */
  if(!tmwtarg_openChannel(pContext->pIOContext, _receiveCallback,
    _checkAddressCallback, pContext->pChannel->pPhysContext))
  {
    /* return failure */
    return(TMWDEFS_FALSE);
  }

  /* Diagnostics */
  TMWPHYSD_CHANNEL_OPENED(pContext->pChannel);
  pContext->channelOpen = TMWPHYS_OPEN; 

  /* Return status */
  return(TMWDEFS_TRUE);
}

/* function: _closeChannel
 * purpose: close this channel
 * arguments:
 *  pContext - context returned from call to tmwphys_initChannel
 *  reason -
 * returns:
 *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
 */
static TMWTYPES_BOOL TMWDEFS_CALLBACK _closeChannel(
  TMWPHYS_CONTEXT *pContext, TMWDEFS_TARG_OC_REASON reason)
{

  /* Cancel any timers */
  tmwtimer_cancel(&pContext->firstCharWaitTimer);
  tmwtimer_cancel(&pContext->transmitReadyTimer);

  /* Diagnostics */
  TMWPHYSD_CHANNEL_CLOSED(pContext->pChannel, reason);

  /* Channel is closed */
  pContext->channelOpen = TMWPHYS_CLOSED; 

  /* Call low level close function */
  tmwtarg_closeChannel(pContext->pIOContext);

  if(pContext->pTxDescriptor != (const TMWPHYS_TX_DESCRIPTOR *)TMWDEFS_NULL)
  {
    TMWCHNL_STAT_ERROR_TYPE errorInfo;
    const TMWPHYS_TX_DESCRIPTOR *pOldTxDescriptor = pContext->pTxDescriptor;
    pContext->pTxDescriptor = TMWDEFS_NULL;

    /* Diagnostics */
    TMWPHYSD_ERROR(pContext->pChannel, "Error writing to physical channel, connection has closed");

    /* Statistics */
    errorInfo.errorCode = TMWCHNL_ERROR_PHYS_TRANSMIT;
    errorInfo.pSession = TMWDEFS_NULL; 
    TMWCHNL_STAT_CALLBACK_FUNC(pContext->pChannel, TMWCHNL_STAT_ERROR, &errorInfo);

    /* Link level failure callback */
    if(pOldTxDescriptor->failedTxCallback != TMWDEFS_NULL)
    {
      pOldTxDescriptor->failedTxCallback(pOldTxDescriptor->pCallbackParam);
    }
  }

  /* Return success */
  return(TMWDEFS_TRUE);
}

/* function: _setCallbacks
 * purpose: sets callbacks to use for parsing received bytes
 * arguments:
 *  pContext - context returned from call to tmwphys_initChannel
 *  pNeededCharsFunc -
 *  pParseFunc -
 *  pCallbackParam -
 * returns:
 *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
 */
static void TMWDEFS_CALLBACK _setCallbacks(
  TMWPHYS_CONTEXT *pContext,
  TMWPHYS_CHANNEL_FUNC pChannelFunc,
  TMWPHYS_NEEDED_CHARS_FUNC pNeededCharsFunc,
  TMWPHYS_PARSE_FUNC pParseFunc,
  TMWPHYS_CHECK_ADDRESS_FUNC pCheckAddressFunc,
  void *pCallbackParam)
{
  pContext->pCallbackParam = pCallbackParam;
  pContext->pParseFunc = pParseFunc;
  pContext->pChannelFunc = pChannelFunc;
  pContext->pNeededCharsFunc = pNeededCharsFunc;
  pContext->pCheckAddressFunc = pCheckAddressFunc;
}

/* function: _transmit
 * purpose: low level transmit routine
 * arguments:
 *  pContext - physical context to transmit on
 * returns:
 *  void
 */
static TMWTYPES_BOOL TMWDEFS_LOCAL _transmit(
  TMWPHYS_CONTEXT *pContext)
{
  const TMWPHYS_TX_DESCRIPTOR *pOldTxDescriptor;
  TMWTYPES_MILLISECONDS readyDelay;
  TMWTYPES_BOOL transmitOk;

  /* Check to see if communications channel is ready */
  readyDelay = tmwtarg_getTransmitReady(pContext->pIOContext);

  /* if channel closes it will Null out pTxDescriptor */
  if(pContext->pTxDescriptor == (const TMWPHYS_TX_DESCRIPTOR *)TMWDEFS_NULL)
  {
    return(TMWDEFS_FALSE);
  } 

  if(readyDelay > 0UL)
  {
    /* Start timer based on delay returned and recheck then  */
    tmwtimer_start(&pContext->transmitReadyTimer, readyDelay,
      pContext->pChannel, _transmitReadyTimeout, pContext);

    return(TMWDEFS_FALSE);
  }

  /* Link level callback */
  if(pContext->pTxDescriptor->beforeTxCallback)
  {
    pContext->pTxDescriptor->beforeTxCallback(pContext->pTxDescriptor->pCallbackParam);
  }

  /* Low level transmit routine */
  if(pContext->pTxDescriptor->UDPPort == TMWTARG_UDP_NONE)
  {
    transmitOk = tmwtarg_transmit(pContext->pIOContext,
      pContext->pTxDescriptor->pTxBuffer, pContext->pTxDescriptor->numBytesToTx);
  }
  else
  {
    transmitOk = tmwtarg_transmitUDP(pContext->pIOContext, 
      pContext->pTxDescriptor->UDPPort,
      pContext->pTxDescriptor->pTxBuffer, 
      pContext->pTxDescriptor->numBytesToTx);
  }

  pOldTxDescriptor = pContext->pTxDescriptor;
  pContext->pTxDescriptor = TMWDEFS_NULL;

  if(transmitOk)
  {
    TMWTYPES_USHORT bytesSent = pOldTxDescriptor->numBytesToTx;

    /* Diagnostics */
    TMWPHYSD_BYTES_SENT(pContext->pChannel,
      pOldTxDescriptor->pTxBuffer, bytesSent);

    /* Statistics */
    TMWCHNL_STAT_CALLBACK_FUNC(pContext->pChannel,
      TMWCHNL_STAT_BYTES_SENT, &bytesSent);

    /* Link level success callback */
    if(pOldTxDescriptor->afterTxCallback)
    {
      pOldTxDescriptor->afterTxCallback(pOldTxDescriptor->pCallbackParam);
    }

    return(TMWDEFS_TRUE);
  }
  else
  {
    TMWCHNL_STAT_ERROR_TYPE errorInfo; 

    /* Diagnostics */
    TMWPHYSD_ERROR(pContext->pChannel, "Error writing to physical channel");

    /* Statistics */
    errorInfo.errorCode = TMWCHNL_ERROR_PHYS_TRANSMIT;
    errorInfo.pSession = TMWDEFS_NULL;  
    TMWCHNL_STAT_CALLBACK_FUNC(pContext->pChannel, TMWCHNL_STAT_ERROR, &errorInfo);

    /* Link level failure callback */
    if((pOldTxDescriptor != (const TMWPHYS_TX_DESCRIPTOR *)TMWDEFS_NULL)
      &&(pOldTxDescriptor->failedTxCallback != TMWDEFS_NULL))
    {
      pOldTxDescriptor->failedTxCallback(pOldTxDescriptor->pCallbackParam);
    }

    return(TMWDEFS_FALSE);
  }
}

/* function: _transmitBytes
 * purpose: transmit bytes to this channel
 * arguments:
 *  pContext - context returned from call to tmwphys_initChannel
 *  pTxDescriptor -
 * returns:
 *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
 */
static TMWTYPES_BOOL TMWDEFS_CALLBACK _transmitBytes(
  TMWPHYS_CONTEXT *pContext,
  const TMWPHYS_TX_DESCRIPTOR *pTxDescriptor)
{

#ifdef TMW_SUPPORT_MONITOR
  /* If acting as a sniffer or protocol analyzer, don't send anything */
  if(pContext->monitorMode)
    return(TMWDEFS_TRUE);
#endif

  /* If channel is not open */
  if(pContext->channelOpen != TMWPHYS_OPEN)
  {
    TMWPHYSD_ERROR(pContext->pChannel, "Channel not open");

    if(pTxDescriptor->failedTxCallback != TMWDEFS_NULL)
    {
      pTxDescriptor->failedTxCallback(pTxDescriptor->pCallbackParam);
    }

    return(TMWDEFS_FALSE);
  }

  /* See if previous data has been transmitted */
  if(pContext->pTxDescriptor != (const TMWPHYS_TX_DESCRIPTOR *)TMWDEFS_NULL)
  {
    TMWPHYSD_ERROR(pContext->pChannel, "Previous data not transmitted\n");

    if(pTxDescriptor->failedTxCallback != TMWDEFS_NULL)
    {
      pTxDescriptor->failedTxCallback(pTxDescriptor->pCallbackParam);
    }

    return(TMWDEFS_FALSE);
  }

  /* Save descriptor for future transmission */
  pContext->pTxDescriptor = pTxDescriptor;

  /* See if we have waited long enough since last received byte */
  if((pContext->firstCharWait != 0)
    && tmwtimer_isActive(&pContext->firstCharWaitTimer))
  {
    /* Nope, return and let timeout handle it */
    /* Assume it is going to work */
    return(TMWDEFS_TRUE);
  }

  /* If we make it this far try to transmit */
  return(_transmit(pContext));
}

/* function: _receiveBytes
 * purpose: receive bytes from this channel
 * arguments:
 *  pContext - context returned from call to tmwphys_initChannel
 *  maxTimeout -
 * returns:
 *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
 */
static TMWTYPES_BOOL TMWDEFS_CALLBACK _receiveBytes(
  TMWPHYS_CONTEXT *pContext,
  TMWTYPES_MILLISECONDS maxTimeout)
{
  TMWTYPES_BOOL interCharTimeoutOccurred = TMWDEFS_FALSE;
  TMWTYPES_USHORT numCharsReceived;
  TMWTYPES_USHORT numCharsNeeded;
  TMWTYPES_MILLISECONDS firstByteTime = 0;
  
  /* If channel is not open just return */
  if(pContext->channelOpen != TMWPHYS_OPEN)
  {
#if TMW_PRIVATE
    /* when configured for modem support, receive the modem data */
    if(!pContext->rcvModemData) 
#endif
      return(TMWDEFS_FALSE);
  }

  /* Make sure callbacks have been specified */
  if((pContext->pNeededCharsFunc == TMWDEFS_NULL) || (pContext->pParseFunc == TMWDEFS_NULL))
  {
    return(TMWDEFS_FALSE);
  }

  numCharsNeeded = pContext->pNeededCharsFunc(pContext->pCallbackParam);
  numCharsNeeded = (TMWTYPES_USHORT)TMWDEFS_MIN(numCharsNeeded, pContext->rxBufferSize);
   
  /* Low level receive routine */
  numCharsReceived = tmwtarg_receive(
    pContext->pIOContext, pContext->pReceiveBuffer,
    numCharsNeeded, maxTimeout, &interCharTimeoutOccurred, &firstByteTime);
  
  /* See if inter character timeout occurred */
  if(interCharTimeoutOccurred)
  {
    TMWCHNL_STAT_ERROR_TYPE errorInfo; 
    errorInfo.errorCode = TMWCHNL_ERROR_PHYS_CHAR_TIMEOUT;
    errorInfo.pSession = TMWDEFS_NULL;  
    TMWCHNL_STAT_CALLBACK_FUNC(pContext->pChannel, TMWCHNL_STAT_ERROR, &errorInfo);
  }

  /* See if any characters read */
  if(numCharsReceived > 0)
  {
#if TMWCNFG_SUPPORT_RXCALLBACKS
    if(pContext->pUserParseFunc != TMWDEFS_NULL)
        pContext->pUserParseFunc(pContext->pUserParseParam, numCharsNeeded,
        &numCharsReceived, pContext->pReceiveBuffer, firstByteTime);
    
    /* If not open just return false, now that modem data was received */
    if(pContext->channelOpen != TMWPHYS_OPEN)
    {
      return(TMWDEFS_FALSE);
    }
#endif

    /* Start firstCharWait timer to delay future transmissions if required
     * for this channel
     */
    if(pContext->firstCharWait != 0)
    {
      tmwtimer_start(&pContext->firstCharWaitTimer,
        pContext->firstCharWait, pContext->pChannel,
        _firstCharTimeout, pContext);
    }

    /* Diagnostics */
    TMWPHYSD_BYTES_RECEIVED(pContext->pChannel,
      pContext->pReceiveBuffer, numCharsReceived);

    /* Statistics */
    TMWCHNL_STAT_CALLBACK_FUNC(pContext->pChannel,
      TMWCHNL_STAT_BYTES_RECEIVED, &numCharsReceived);

    /* Call link level parse routine */
    pContext->pParseFunc(pContext->pCallbackParam,
      pContext->pReceiveBuffer, numCharsReceived, firstByteTime);

    return(TMWDEFS_TRUE);
  }

  return(TMWDEFS_FALSE);
}

/* Global function Implementation */

/* function: tmwphys_initConfig */
void TMWDEFS_GLOBAL tmwphys_initConfig(TMWPHYS_CONFIG *pConfig)
{
  pConfig->firstCharWait = 0;
  pConfig->rxBufferSize = 256;
  pConfig->monitorMode = TMWDEFS_FALSE;
  pConfig->rcvModemData = TMWDEFS_FALSE;

  /* Channel should always be active and try to connect by default */
  pConfig->active = TMWDEFS_TRUE;
}

/* function: tmwphys_initChannel */
TMWTYPES_BOOL TMWDEFS_GLOBAL tmwphys_initChannel(
  TMWCHNL *pChannel,
  const TMWPHYS_CONFIG *pConfig,
  const void *pIOConfig,
  TMWTARG_CONFIG *pTmwTargConfig)
{

  static const TMWPHYS_INTERFACE _tmwphysInterface = {
    (TMWPHYS_OPEN_CHANNEL_FUNC)_openChannel,
    (TMWPHYS_CLOSE_CHANNEL_FUNC)_closeChannel,
    (TMWPHYS_SET_CALLBACKS_FUNC)_setCallbacks,
    (TMWPHYS_TRANSMIT_FUNC)_transmitBytes,
    (TMWPHYS_RECEIVE_FUNC)_receiveBytes
  };

  TMWPHYS_CONTEXT *pContext;

  pContext = (TMWPHYS_CONTEXT *)tmwmem_alloc(TMWMEM_PHYS_CONTEXT_TYPE);
  if(pContext == TMWDEFS_NULL)
  {
    TMWPHYSD_ERROR(pChannel, "Error allocating physical context");
    return(TMWDEFS_FALSE);
  }

  /* Initialize context */
  pContext->channelOpen = TMWPHYS_CLOSED;
  pContext->pIOContext = TMWDEFS_NULL;
  pContext->pTxDescriptor = TMWDEFS_NULL;
  tmwtimer_init(&pContext->firstCharWaitTimer);
  tmwtimer_init(&pContext->transmitReadyTimer);

  /* Configuration */
  pContext->active = pConfig->active; 
  pContext->firstCharWait = pConfig->firstCharWait;
  pContext->rxBufferSize = pConfig->rxBufferSize;

#ifdef TMW_SUPPORT_MONITOR
  pContext->monitorMode = pConfig->monitorMode;
#endif

  /* Initialize callbacks */
  pContext->pCallbackParam = TMWDEFS_NULL;
  pContext->pParseFunc = TMWDEFS_NULL;
  pContext->pNeededCharsFunc = TMWDEFS_NULL;

#if TMW_PRIVATE
  pContext->rcvModemData = pConfig->rcvModemData;
#endif
#if TMWCNFG_SUPPORT_RXCALLBACKS
  pContext->pUserParseFunc = TMWDEFS_NULL;
#endif

#if TMWCNFG_USE_DYNAMIC_MEMORY && !TMWCNFG_ALLOC_ONLY_AT_STARTUP 
  /* Allocate receive buffer */
  pContext->pReceiveBuffer = (TMWTYPES_UCHAR *)tmwtarg_alloc(pConfig->rxBufferSize);
  if(pContext->pReceiveBuffer == TMWDEFS_NULL)
  {
    TMWPHYSD_ERROR(pChannel, "Error allocating physical receive buffer");
    tmwmem_free(pContext);
    return(TMWDEFS_FALSE);
  }
#else
  if(pConfig->rxBufferSize > TMWCNFG_MAX_RX_BUFFER_LENGTH)
  {
    TMWPHYSD_ERROR(pChannel, "Error allocating physical receive buffer");
    tmwmem_free(pContext);
    return(TMWDEFS_FALSE);
  }

  pContext->pReceiveBuffer = pContext->buffer;
#endif

  /* These callback parameters can't be set by the user at runtime
   * they are used by the SCL to indicate the callback functions
   * to the target layer
   */
  pTmwTargConfig->pChannelCallback = _channelCallback;
  pTmwTargConfig->pCallbackParam   = pContext;

  pTmwTargConfig->pChannelReadyCallback = _transmitReadyCallback;
  pTmwTargConfig->pChannelReadyCbkParam = pContext;

  pTmwTargConfig->pChannel = pChannel;

  pContext->pIOContext = tmwtarg_initChannel(pIOConfig, pTmwTargConfig, pChannel);
  if(pContext->pIOContext == TMWDEFS_NULL)
  {
    TMWPHYSD_ERROR(pChannel, "Error returned from target channel initialization");
#if TMWCNFG_USE_DYNAMIC_MEMORY && !TMWCNFG_ALLOC_ONLY_AT_STARTUP 
    tmwtarg_free(pContext->pReceiveBuffer);
#endif
    tmwmem_free(pContext);
    return(TMWDEFS_FALSE);
  }

  pContext->pChannel = pChannel;
  pChannel->pPhys = &_tmwphysInterface;
  pChannel->pPhysContext = pContext;

  return(TMWDEFS_TRUE);
}

/* function: tmwphys_modifyPhys */
TMWTYPES_BOOL TMWDEFS_GLOBAL tmwphys_modifyPhys(
  TMWCHNL *pChannel,
  const void *pIOConfig,
  TMWTARG_CONFIG *pTmwTargConfig)
{
  TMWPHYS_CONTEXT *pContext = pChannel->pPhysContext;
  TMWTYPES_BOOL status = TMWDEFS_TRUE;

  tmwtarg_stopThreads(pContext->pIOContext);

  /* Lock channel */
  TMWTARG_LOCK_SECTION(&pChannel->lock);

  pContext->channelOpen = TMWPHYS_CLOSED; 

  /* Delete target channel */
  tmwtarg_deleteChannel(pContext->pIOContext);

  /* These callback parameters can't be set by the user at runtime
   * they are used by the SCL to indicate the callback functions
   * to the target layer
   */
  pTmwTargConfig->pChannelCallback = _channelCallback;
  pTmwTargConfig->pCallbackParam   = pContext;

  pTmwTargConfig->pChannelReadyCallback = _transmitReadyCallback;
  pTmwTargConfig->pChannelReadyCbkParam = pContext;

  pContext->pIOContext = tmwtarg_initChannel(pIOConfig, pTmwTargConfig, pChannel);
  if(pContext->pIOContext == TMWDEFS_NULL)
  {
#if TMWCNFG_USE_DYNAMIC_MEMORY && !TMWCNFG_ALLOC_ONLY_AT_STARTUP 
    tmwtarg_free(pContext->pReceiveBuffer);
#endif
    tmwmem_free(pContext); 
    status = TMWDEFS_FALSE;
  }
  else
  {
    pContext->pChannelFunc(pContext->pCallbackParam, 0, TMWDEFS_TARG_OC_LOCAL_CLOSED);
  }

  /* Unlock channel */
  TMWTARG_UNLOCK_SECTION(&pContext->pChannel->lock); 

  return(status);
}

/* function: tmwphys_modifyChannel */
TMWTYPES_BOOL TMWDEFS_GLOBAL tmwphys_modifyChannel(
  TMWCHNL *pChannel,
  const TMWPHYS_CONFIG *pConfig,
  TMWTYPES_ULONG configMask)
{
  TMWPHYS_CONTEXT *pContext = pChannel->pPhysContext;

  /* Lock channel */
  TMWTARG_LOCK_SECTION(&pContext->pChannel->lock);

  /* First Character Wait */
  if((configMask & TMWPHYS_CONFIG_FIRST_CHAR_WAIT) != 0UL)
  {
    pContext->firstCharWait = pConfig->firstCharWait;

    /* See if timer is currently active
     */
    if(tmwtimer_isActive(&pContext->firstCharWaitTimer))
    {
      /* If so, cancel it */
      tmwtimer_cancel(&pContext->firstCharWaitTimer);

      /* If the new timeout is not zero restart it using the
       *  new timeout value. For now we will ignore any time
       *  that has already elapsed
       */
      if(pContext->firstCharWait != 0)
      {
        tmwtimer_start(&pContext->firstCharWaitTimer,
          pContext->firstCharWait, pContext->pChannel,
          _firstCharTimeout, pContext);
      }
    }
  }

  /* Unlock channel */
  TMWTARG_UNLOCK_SECTION(&pContext->pChannel->lock);

  return(TMWDEFS_TRUE);
}

/* function: tmwphys_getChannelConfig  */
TMWTYPES_BOOL TMWDEFS_GLOBAL tmwphys_getChannelConfig(
  TMWCHNL        *pChannel,
  TMWPHYS_CONFIG *pPhysConfig)
{
  TMWPHYS_CONTEXT *pPhysContext = pChannel->pPhysContext;
  pPhysConfig->firstCharWait    = pPhysContext->firstCharWait;
  pPhysConfig->rxBufferSize     = pPhysContext->rxBufferSize;
  pPhysConfig->monitorMode      = pPhysContext->monitorMode;
  pPhysConfig->active           = pPhysContext->active;

  return TMWDEFS_TRUE;
}

/* function: tmwphys_setChannelConfig  */
TMWTYPES_BOOL TMWDEFS_GLOBAL tmwphys_setChannelConfig(
  TMWCHNL        *pChannel,
  const TMWPHYS_CONFIG *pPhysConfig)
{ 
  TMWPHYS_CONTEXT *pPhysContext = pChannel->pPhysContext;

  /* If rxBuffer was configured larger */
  if (pPhysConfig->rxBufferSize > pPhysContext->rxBufferSize)
  {
#if TMWCNFG_USE_DYNAMIC_MEMORY && !TMWCNFG_ALLOC_ONLY_AT_STARTUP 
    /* Allocate larger receive buffer */
    TMWTYPES_UCHAR *ptr = (TMWTYPES_UCHAR *)tmwtarg_alloc(pPhysConfig->rxBufferSize);
    if (ptr == TMWDEFS_NULL)
    { 
      return(TMWDEFS_FALSE);
    }
    tmwtarg_free(pPhysContext->pReceiveBuffer);
    pPhysContext->pReceiveBuffer = ptr;
#endif
  }
  pPhysContext->rxBufferSize  = pPhysConfig->rxBufferSize;
  pPhysContext->monitorMode   = pPhysConfig->monitorMode;
 
  pPhysContext->firstCharWait = pPhysConfig->firstCharWait; 

  if(pPhysContext->active != pPhysConfig->active)
  {
    pPhysContext->active = pPhysConfig->active;
    if(pPhysConfig->active)
    { 
      /* If there are no sessions, don't tell the lower layers to open */
      if(tmwdlist_size(&pChannel->pLinkContext->sessions) >0)
        tmwlink_openChannel(pChannel->pLinkContext);
    }
    else
    {
      tmwlink_setChannelInactive(pChannel->pLinkContext);  
    }
  }
  return TMWDEFS_TRUE;
}


/* function: tmwphys_deleteChannel */
void TMWDEFS_GLOBAL tmwphys_deleteChannel(
  TMWCHNL *pChannel)
{
  TMWPHYS_CONTEXT *pContext = pChannel->pPhysContext;

  /* Cancel timers */
  tmwtimer_cancel(&pContext->firstCharWaitTimer);
  tmwtimer_cancel(&pContext->transmitReadyTimer);
  
  pContext->channelOpen = TMWPHYS_CLOSED; 

  /* Delete target channel */
  tmwtarg_deleteChannel(pContext->pIOContext);

  /* Clean up */
#if TMWCNFG_USE_DYNAMIC_MEMORY && !TMWCNFG_ALLOC_ONLY_AT_STARTUP 
  tmwtarg_free(pContext->pReceiveBuffer);
#endif
  tmwmem_free(pContext);

  /* Make sure no one tries to do this again */
  pChannel->pPhys = TMWDEFS_NULL;
  pChannel->pPhysContext = TMWDEFS_NULL;
}
