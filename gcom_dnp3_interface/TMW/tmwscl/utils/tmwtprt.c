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

/* file: tmwtprt.h
 * description: Implement link layer functionality required by all TMW link
 *  layer implementations.
 */
#include "tmwscl/utils/tmwtarg.h"
#include "tmwscl/utils/tmwtprt.h"
#include "tmwscl/utils/tmwchnl.h"

/* function: tmwtprt_initChannel */
void * TMWDEFS_GLOBAL tmwtprt_initChannel(
  TMWCHNL *pChannel,
  TMWLINK_INFO_FUNC pInfoFunc,
  TMWLINK_PARSE_FUNC pParseFunc, 
  TMWLINK_CHECK_CLASS_FUNC pCheckClassFunc,
  TMWLINK_TX_CALLBACK_FUNC pBeforeTxCallback,
  TMWLINK_TX_CALLBACK_FUNC pAfterTxCallback,
  TMWLINK_TX_CALLBACK_FUNC pFailedTxCallback)
{
  TMWTPRT_CONTEXT *pContext = pChannel->pTprtContext;

  pContext->pChannel = pChannel;
  pContext->pParam = TMWDEFS_NULL;
  pContext->pInfoFunc = TMWDEFS_NULL;
  pContext->pParseFunc = TMWDEFS_NULL;
#if TMWCNFG_SUPPORT_RXCALLBACKS
  pContext->pUserParseFunc = TMWDEFS_NULL;
#endif

  /* Set link layer callbacks */
  pChannel->pLink->pLinkSetCallbacks(
    pChannel->pLinkContext, pContext, pInfoFunc, pParseFunc, pCheckClassFunc,
    pBeforeTxCallback, pAfterTxCallback, pFailedTxCallback);

  return(pContext);
}

/* function: tmwtprt_deleteChannel */
void TMWDEFS_GLOBAL tmwtprt_deleteChannel(
  TMWCHNL *pChannel)
{
  /* Reset link layer callbacks */
  pChannel->pLink->pLinkSetCallbacks(
    pChannel->pLinkContext, TMWDEFS_NULL,
    TMWDEFS_NULL, TMWDEFS_NULL, TMWDEFS_NULL,
    TMWDEFS_NULL, TMWDEFS_NULL, TMWDEFS_NULL);

  /* Make sure we don't do this again */
  pChannel->pTprt = TMWDEFS_NULL;
  pChannel->pTprtContext = TMWDEFS_NULL;
}

/* function: tmwtprt_setCallbacks */
void TMWDEFS_GLOBAL tmwtprt_setCallbacks(
  TMWTPRT_CONTEXT *pContext, 
  void *pParam,
  TMWTPRT_INFO_FUNC pInfoFunc,
  TMWTPRT_PARSE_FUNC pParseFunc,
  TMWTPRT_CHECK_CLASS_FUNC pCheckClassFunc)
{
  pContext->pParam = pParam;
  pContext->pInfoFunc = pInfoFunc;
  pContext->pParseFunc = pParseFunc;
  pContext->pCheckClassFunc = pCheckClassFunc;
}

/* function: tmwtprt_openSession */
TMWTYPES_BOOL TMWDEFS_GLOBAL tmwtprt_openSession(
  TMWTPRT_CONTEXT *pContext, 
  TMWSESN *pSession)
{
  return(pContext->pChannel->pLink->pLinkOpenSession(pContext->pChannel->pLinkContext, pSession, TMWDEFS_NULL));
}

/* function: tmwtprt_closeSession */
TMWTYPES_BOOL TMWDEFS_GLOBAL tmwtprt_closeSession(
  TMWTPRT_CONTEXT *pContext, 
  TMWSESN *pSession)
{
  return(pContext->pChannel->pLink->pLinkCloseSession(pContext->pChannel->pLinkContext, pSession));
}

/* function: tmwtprt_getSessions */
TMWDLIST * TMWDEFS_GLOBAL tmwtprt_getSessions(
  TMWTPRT_CONTEXT *pContext)
{
  return(pContext->pChannel->pLink->pLinkGetSessions(pContext->pChannel->pLinkContext));
}
