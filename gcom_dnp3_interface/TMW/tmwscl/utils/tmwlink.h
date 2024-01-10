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
 * description: Generic link layer definitions and functionality
 */
#ifndef TMWLINK_DEFINED
#define TMWLINK_DEFINED

#include "tmwscl/utils/tmwcnfg.h"
#include "tmwscl/utils/tmwscl.h"
#include "tmwscl/utils/tmwdlist.h"
#include "tmwscl/utils/tmwsesn.h"
#include "tmwscl/utils/tmwphys.h"
#include "tmwscl/utils/tmwtimer.h"

struct TMWLinkContext;

/* Define callback into protocol specific link layer code
 */
typedef void (*TMWLINK_LINK_INFO_FUNC)(
  struct TMWLinkContext *pLinkContext,
  struct TMWChannelStruct *pChannel,
  TMWSCL_INFO sclInfo);

/* Define callbacks used by the link layer to call into upper layers of the
 *  protocol stack.
 */
typedef void (*TMWLINK_INFO_FUNC)(
  void *pParam, 
  TMWSESN *pSession, 
  TMWSCL_INFO sclInfo);

typedef void (*TMWLINK_PARSE_FUNC)(
  void *pParam, 
  TMWSESN *pSession, 
  TMWSESN_RX_DATA *pRxData);

typedef TMWTYPES_BOOL (*TMWLINK_CHECK_CLASS_FUNC)(
  void *pParam, 
  TMWSESN *pSession, 
  TMWDEFS_CLASS_MASK classMask, 
  TMWTYPES_BOOL buildResponse);

typedef void (*TMWLINK_TX_CALLBACK_FUNC)(
  TMWSESN_TX_DATA *pTxData);

/* Prototype for function that can be registered to receive frame */
typedef void (*TMWLINK_USER_RX_CALLBACK_FUNC)(
  void *callbackParam,
  TMWTYPES_MILLISECONDS firstByteTime,
  TMWTYPES_UCHAR *pBuf,
  TMWTYPES_USHORT *pLength);   

/* Define generic link layer context and functionality required by all TMW 
 * link layer implementations. The corresponding functionality is implemented
 * in tmwlink.c and used by the various protocol specific link layer modules.
 */
typedef struct TMWLinkContext {
  /* Pointer to channel */
  struct TMWChannelStruct *pChannel;

  /* List of sessions currently open on this channel */
  TMWDLIST sessions;

  /* Is the channel open */
  TMWTYPES_BOOL isOpen;
  TMWTIMER openChannelTimer;
  TMWTYPES_ULONG retryCount;
  TMWTYPES_MILLISECONDS offlinePollPeriod;

  /* Callback into protocol specific link layer */
  TMWLINK_LINK_INFO_FUNC pLinkInfoFunc;

  /* Callbacks into the upper layer of the protocol stack */
  void *pCallbackParam;
  TMWLINK_INFO_FUNC pInfoFunc;
  TMWLINK_PARSE_FUNC pParseFunc;
  TMWLINK_CHECK_CLASS_FUNC pCheckClassFunc;
  TMWLINK_TX_CALLBACK_FUNC pBeforeTxCallback;
  TMWLINK_TX_CALLBACK_FUNC pAfterTxCallback;
  TMWLINK_TX_CALLBACK_FUNC pFailedTxCallback;

#if TMWCNFG_SUPPORT_RXCALLBACKS
 
  /* Callback into user provided function when entire frame has been received 
   * For test purposes only. Customers will not need to implement this.
   */
  TMWLINK_USER_RX_CALLBACK_FUNC  pUserRxFrameCallback;
  void                          *pUserRxFrameCallbackParam;
#endif

TMWTYPES_MILLISECONDS time_connect;

} TMWLINK_CONTEXT;

/* Now define the actual link layer interface that will be used
 * by the transport layer to call into the link layer. Each link
 * layer implementation must provide an instance of this interface.
 */
typedef TMWTYPES_BOOL (*TMWLINK_OPEN_SESSION_FUNC)(
  TMWLINK_CONTEXT *pContext, 
  TMWSESN *pSession,
  void *pConfig);

typedef TMWTYPES_BOOL (*TMWLINK_CLOSE_SESSION_FUNC)(
  TMWLINK_CONTEXT *pContext, 
  TMWSESN *pSession);

typedef TMWDLIST *(*TMWLINK_GET_SESSIONS_FUNC)(
  TMWLINK_CONTEXT *pContext);

typedef void (*TMWLINK_DATA_READY_FUNC)(
  TMWLINK_CONTEXT *pContext, 
  TMWSESN *pSession);

typedef void (*TMWLINK_SET_CALLBACKS_FUNC)(
  TMWLINK_CONTEXT *pContext, 
  void *pParam, 
  TMWLINK_INFO_FUNC pInfoFunc,
  TMWLINK_PARSE_FUNC pParseFunc, 
  TMWLINK_CHECK_CLASS_FUNC pCheckClass,
  TMWLINK_TX_CALLBACK_FUNC pBeforeTxCallback,
  TMWLINK_TX_CALLBACK_FUNC pAfterTxCallback,
  TMWLINK_TX_CALLBACK_FUNC pFailedTxCallback
  );

typedef void (*TMWLINK_TRANSMIT_FUNC)(
  TMWLINK_CONTEXT *pContext, 
  TMWSESN_TX_DATA *pFrame);

typedef TMWTYPES_UCHAR * (*TMWLINK_UPDATE_MSG_FUNC)(
  TMWLINK_CONTEXT *pContext, 
  TMWTYPES_USHORT offset,
  TMWTYPES_UCHAR *pData, 
  TMWTYPES_USHORT length);

typedef void (*TMWLINK_CANCEL_FUNC)(
  TMWLINK_CONTEXT *pContext, 
  TMWSESN_TX_DATA *pFrame);

typedef struct TMWLinkInterface {
  TMWLINK_OPEN_SESSION_FUNC pLinkOpenSession;
  TMWLINK_CLOSE_SESSION_FUNC pLinkCloseSession;
  TMWLINK_GET_SESSIONS_FUNC pLinkGetSessions;
  TMWLINK_DATA_READY_FUNC pLinkDataReady;
  TMWLINK_SET_CALLBACKS_FUNC pLinkSetCallbacks;
  TMWLINK_TRANSMIT_FUNC pLinkTransmit;
  TMWLINK_UPDATE_MSG_FUNC pLinkUpdateMsg;
  TMWLINK_CANCEL_FUNC pLinkCancel;
} TMWLINK_INTERFACE;

#ifdef __cplusplus
extern "C" {
#endif

  /* function: tmwlink_initChannel
   * purpose: initialize a channel
   * arguments:
   * returns:
   *  pointer to pContext
   */
  TMWDEFS_SCL_API void * TMWDEFS_GLOBAL tmwlink_initChannel(
    struct TMWChannelStruct *pChannel, 
    TMWTYPES_MILLISECONDS offlinePollPeriod,
    TMWPHYS_CHANNEL_FUNC pChannelFunc,
    TMWPHYS_NEEDED_CHARS_FUNC pNeededCharsFunc, 
    TMWPHYS_PARSE_FUNC pParseFunc,
    TMWLINK_LINK_INFO_FUNC pLinkInfoFunc,
    TMWPHYS_CHECK_ADDRESS_FUNC pCheckAddressFunc);

  /* function: tmwlink_deleteChannel
   * purpose: deletes a channel
   * arguments:
   *  pContext - link layer context returned from dnplink_initChannel
   * returns:
   *  void
   */
  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL tmwlink_deleteChannel(
    struct TMWChannelStruct *pChannel);

  /* function: tmwlink_channelCallback
   * purpose: default implementation of a channel callback
   * arguments:
   *  pCallbackParam - user specified callback parameter
   *  openOrClose - TMWDEFS_TRUE if channel is open, else TMWDEFS_FALSE
   *  reason - reason for close
   * returns:
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_CALLBACK tmwlink_channelCallback(
    void *pCallbackParam,
    TMWTYPES_BOOL openOrClose,
    TMWDEFS_TARG_OC_REASON reason);

  /* function: _setCallbacks
   * purpose: set callbacks to use when receiving data
   * arguments:
   *  pContext - link layer context
   *  pParseFunc - function to parse received frame
   *  pParseParam - callback data
   * returns
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_CALLBACK tmwlink_setCallbacks(
    TMWLINK_CONTEXT *pContext, 
    void *pCallbackParam,
    TMWLINK_INFO_FUNC pInfoFunc,
    TMWLINK_PARSE_FUNC pParseFunc, 
    TMWLINK_CHECK_CLASS_FUNC pCheckClassFunc,
    TMWLINK_TX_CALLBACK_FUNC pBeforeTxCallback,
    TMWLINK_TX_CALLBACK_FUNC pAfterTxCallback,
    TMWLINK_TX_CALLBACK_FUNC pFailedTxCallback);

  /* function: tmwlink_openChannel 
   * purpose: Attempts to open channel if it is not already open. If open
   *  fails it will set up a timer to attempt to open it again later.
   * arguments:
   *  pContext - link layer context
   * returns:
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwlink_openChannel(
    TMWLINK_CONTEXT *pContext);

  /* function: tmwlink_closeChannel 
   * purpose: Close channel if it is open. 
   * arguments:
   *  pContext - link layer context
   * returns:
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwlink_closeChannel(
    TMWLINK_CONTEXT *pContext);

  /* function: tmwlink_setChannelInactive 
   * purpose: Tell target layer to close the channel and stop trying to connect.
   * arguments:
   *  pContext - link layer context
   * returns:
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwlink_setChannelInactive(
    TMWLINK_CONTEXT *pContext);

  /* function: tmwlink_openSession
   * purpose: open a new session on this channel
   * arguments:
   *  pContext - link layer context
   *  pSession - pointer to session to open
   * returns:
   *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
   */ 
  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL tmwlink_openSession(
    TMWLINK_CONTEXT *pContext, 
    TMWSESN *pSession);

  /* function: tmwlink_closeSession
   * purpose: close a session on this channel
   *  if this is the last session on this channel, channel
   *  will be closed.
   * arguments:
   *  pContext - link layer context
   *  pSession - pointer to session to close
   * returns:
   *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
   */ 
  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL tmwlink_closeSession(
    TMWLINK_CONTEXT *pContext, 
    TMWSESN *pSession);

  /* function: tmwlink_getSessions
   * purpose: return pointer to list of sessions
   * arguments:
   *  pContext - link layer context
   * returns:
   *  pointer to list of sessions
   */ 
  TMWDEFS_SCL_API TMWDLIST * TMWDEFS_GLOBAL tmwlink_getSessions(
    TMWLINK_CONTEXT *pContext);

  /* function: tmwlink_checkOpen
   * purpose: check to see if this channel is open
   * arguments:
   *  pContext - link layer context
   * returns:
   *  TMWDEFS_TRUE if channel is open, else TMWDEFS_FALSE
   */
  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL tmwlink_checkOpen(
    TMWLINK_CONTEXT *pContext);

#ifdef __cplusplus
}
#endif
#endif /* TMWLINK_DEFINED */
