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

/* file: tmwtprt.c
 * description: Generic transport layer
 */

/* Link layer interface definitions. This file defines all the data 
 * types and structures required to interface to a generic link layer 
 * implementation from Triangle MicroWorks, Inc.
 */
#ifndef TMWTPRT_DEFINED
#define TMWTPRT_DEFINED

#include "tmwscl/utils/tmwcnfg.h"
#include "tmwscl/utils/tmwsesn.h"
#include "tmwscl/utils/tmwlink.h"

/* Application layer callbacks.
 */
typedef void (*TMWTPRT_INFO_FUNC)(
  void *pParam, 
  TMWSESN *pSession, 
  TMWSCL_INFO sesnInfo);

typedef void (*TMWTPRT_PARSE_FUNC)(
  TMWSESN *pSession,
  TMWSESN_RX_DATA *pRxData);

typedef void (*TMWTPRT_USER_PARSE_FUNC)(
  void *callbackParam,
  TMWSESN_RX_DATA *pRxData);
 
typedef TMWTYPES_BOOL (*TMWTPRT_CHECK_CLASS_FUNC)(
  void *pParam, 
  TMWSESN *pSession, 
  TMWDEFS_CLASS_MASK classMask, 
  TMWTYPES_BOOL buildResponse);

/* Define generic transport layer context and functionality required by all transport
 *  layer implementations. The corresponding functionality is implemented in tmwtprt.c
 *  and used by the various protocol specific transport layer modules.
 */
typedef struct TMWTprtContext {
  /* Pointer to channel */
  struct TMWChannelStruct *pChannel;

  /* Callbacks into the upper layer of the protocol stack */
  void *pParam;
  TMWTPRT_INFO_FUNC pInfoFunc;
  TMWTPRT_PARSE_FUNC pParseFunc;
  TMWTPRT_CHECK_CLASS_FUNC pCheckClassFunc;
  
#if TMWCNFG_SUPPORT_RXCALLBACKS
  /* callback into user provided parse function.
   * For special test purposes only.
   * Customers will not need to implement this.
   */
  TMWTPRT_USER_PARSE_FUNC pUserParseFunc;
  void                   *pUserParseParam;
#endif
} TMWTPRT_CONTEXT;

/* Now define the actual physical layer interface that will be used
 * by the link layer to call into the physical layer. Each physical
 * layer implementation must provide an instance of this interface.
 */

typedef TMWTYPES_BOOL (*TMWTPRT_OPEN_SESSION_FUNC)(
  TMWTPRT_CONTEXT *pContext, 
  TMWSESN *pSession);

typedef TMWTYPES_BOOL (*TMWTPRT_CLOSE_SESSION_FUNC)(
  TMWTPRT_CONTEXT *pContext, 
  TMWSESN *pSession);

typedef TMWDLIST *(*TMWTPRT_GET_SESSIONS_FUNC)(
  TMWTPRT_CONTEXT *pContext);

typedef void (*TMWTPRT_DATA_READY_FUNC)(
  TMWTPRT_CONTEXT *pContext, 
  TMWSESN *pSession);

typedef void (*TMWTPRT_SET_CALLBACKS_FUNC)(
  TMWTPRT_CONTEXT *pContext,
  void *pParam,
  TMWTPRT_INFO_FUNC pInfoFunc,
  TMWTPRT_PARSE_FUNC pParseFunc,
  TMWTPRT_CHECK_CLASS_FUNC pCheckClassFunc);

typedef void (*TMWTPRT_TRANSMIT_FUNC)(
  TMWTPRT_CONTEXT *pContext, 
  TMWSESN_TX_DATA *pTxFragment);

typedef void (*TMWTPRT_UPDATE_MSG_FUNC)(
  TMWTPRT_CONTEXT *pContext, 
  TMWTYPES_USHORT offset,
  TMWTYPES_UCHAR *pData, 
  TMWTYPES_USHORT length);

typedef void (*TMWTPRT_CANCEL_FUNC)(
  TMWTPRT_CONTEXT *pContext, 
  TMWSESN_TX_DATA *pTxFragment);

typedef struct TMWTprtInterface {
  TMWTPRT_OPEN_SESSION_FUNC pTprtOpenSession;
  TMWTPRT_CLOSE_SESSION_FUNC pTprtCloseSession;
  TMWTPRT_GET_SESSIONS_FUNC pTprtGetSessions;
  TMWTPRT_DATA_READY_FUNC pTprtDataReady;
  TMWTPRT_SET_CALLBACKS_FUNC pSetCallbacks;
  TMWTPRT_TRANSMIT_FUNC pTprtTransmit;
  TMWTPRT_UPDATE_MSG_FUNC pTprtUpdateMsg;
  TMWTPRT_CANCEL_FUNC pTprtCancel;
} TMWTPRT_INTERFACE;

#ifdef __cplusplus
extern "C" {
#endif

  /* function: tmwtprt_initChannel
   * purpose: initialize a channel
   * arguments:
   * returns:
   *  pointer to pContext
   */
  void * TMWDEFS_GLOBAL tmwtprt_initChannel(
    struct TMWChannelStruct *pChannel, 
    TMWLINK_INFO_FUNC pInfoFunc,
    TMWLINK_PARSE_FUNC pParseFunc, 
    TMWLINK_CHECK_CLASS_FUNC pCheckClassFunc,
    TMWLINK_TX_CALLBACK_FUNC pBeforeTxCallback,
    TMWLINK_TX_CALLBACK_FUNC pAfterTxCallback,
    TMWLINK_TX_CALLBACK_FUNC pFailedTxCallback);

  /* function: tmwtprt_deleteChannel
   * purpose: deletes a channel
   * arguments:
   *  pContext - link layer context returned from dnplink_initChannel
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL tmwtprt_deleteChannel(
    struct TMWChannelStruct *pChannel);

  /* function: _setCallbacks
   * purpose: set callbacks to use when receiving data
   * arguments:
   *  pContext - link layer context returned from dnplink_initChannel
   *  pParseFunc - function to parse received frame
   *  pParseParam - callback data
   * returns
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwtprt_setCallbacks(
    TMWTPRT_CONTEXT *pContext, 
    void *pParam, 
    TMWTPRT_INFO_FUNC pInfoFunc,
    TMWTPRT_PARSE_FUNC pParseFunc,
    TMWTPRT_CHECK_CLASS_FUNC pCheckClassFunc);

  /* function: tmwtprt_openSession
   * purpose: open a new session on this channel
   * arguments:
   *  pContext - link layer context returned from dnplink_initChannel
   *  pSession - pointer to session to open
   * returns:
   *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
   */ 
  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL tmwtprt_openSession(
    TMWTPRT_CONTEXT *pContext, 
    TMWSESN *pSession);

  /* function: tmwtprt_closeSession
   * purpose: close a session on this channel
   * arguments:
   *  pContext - link layer context returned from dnplink_initChannel
   *  pSession - pointer to session to close
   * returns:
   *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
   */ 
  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL tmwtprt_closeSession(
    TMWTPRT_CONTEXT *pContext, 
    TMWSESN *pSession);

  /* function: tmwtprt_getSessions
   * purpose: return pointer to list of sessions
   * arguments:
   *  pContext - link layer context returned from dnplink_initChannel
   * returns:
   *  pointer to list of sessions
   */ 
  TMWDEFS_SCL_API TMWDLIST * TMWDEFS_GLOBAL tmwtprt_getSessions(
    TMWTPRT_CONTEXT *pContext);

#ifdef __cplusplus
}
#endif
#endif /* TMWTPRT_DEFINED */
