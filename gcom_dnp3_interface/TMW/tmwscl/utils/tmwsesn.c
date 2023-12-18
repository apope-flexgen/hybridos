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

/* file: tmwsesn.c
 * description: Definition of a generic session
 */
#include "tmwscl/utils/tmwtarg.h"
#include "tmwscl/utils/tmwsesn.h"
#include "tmwscl/utils/tmwlink.h"
#include "tmwscl/utils/tmwchnl.h"
#include "tmwscl/utils/tmwmem.h"

/* function: tmwsesn_openSession */
TMWTYPES_BOOL TMWDEFS_GLOBAL tmwsesn_openSession(
  TMWCHNL *pChannel,
  TMWSESN *pSession, 
  TMWSESN_STAT_CALLBACK pCallback,
  void *pCallbackParam,
  TMWTYPES_PROTOCOL protocol, 
  TMWTYPES_SESSION_TYPE type)
{
  TMWTYPES_BOOL status = TMWDEFS_TRUE;
  pSession->pUserData = TMWDEFS_NULL;

  ASSERT(type < TMWTYPES_SESSION_TYPE_NUM_TYPES);
  if (type >= TMWTYPES_SESSION_TYPE_NUM_TYPES)
  {
    status = TMWDEFS_FALSE;
  }
  pSession->type = type;

  ASSERT(protocol < TMWTYPES_PROTOCOL_NUM_PROTOCOLS);
  if (protocol >= TMWTYPES_PROTOCOL_NUM_PROTOCOLS)
  {
    status = TMWDEFS_FALSE;
  }
  pSession->protocol = protocol;

  ASSERT(pChannel != TMWDEFS_NULL);
  if (pChannel == TMWDEFS_NULL)
  {
    status = TMWDEFS_FALSE;
  }
  pSession->pChannel = pChannel;
  pSession->online = 0;

  tmwsesn_setStatCallback(pSession,pCallback,pCallbackParam);
  
  return (status);
}

/* function: tmwsesn_closeSession */
void TMWDEFS_GLOBAL tmwsesn_closeSession(TMWSESN *pSession)
{
  pSession->pUserData = TMWDEFS_NULL;
  TMWTARG_UNUSED_PARAM(pSession);
}

/* function: tmwsesn_setUserDataPtr */
void TMWDEFS_GLOBAL tmwsesn_setUserDataPtr(
  TMWSESN *pSession, void *pUserData)
{
  pSession->pUserData = pUserData;
}
  
/* function: tmwsesn_getUserDataPtr */
void * TMWDEFS_GLOBAL tmwsesn_getUserDataPtr(
  TMWSESN *pSession)
{
  return pSession->pUserData;
}

/* function: tmwsesn_setStatCallback */
void TMWDEFS_GLOBAL tmwsesn_setStatCallback(
  TMWSESN *pSession,
  TMWSESN_STAT_CALLBACK pCallback,
  void *pCallbackParam)
{
  pSession->pStatCallbackFunc = pCallback;
  pSession->pStatCallbackParam = pCallbackParam;
}

/* function: tmwsesn_callStatCallback */
void TMWDEFS_GLOBAL tmwsesn_callStatCallback(
  TMWSESN *pSession,
  TMWSESN_STAT_EVENT eventType,
  void *pEventData)
{
  if(pSession->pStatCallbackFunc != TMWDEFS_NULL)
  {
    pSession->pStatCallbackFunc(
      pSession->pStatCallbackParam, eventType, pEventData);
  }
}

#if TMWCNFG_SUPPORT_DIAG
/* function: tmwsesn_getSessionName */
const TMWTYPES_CHAR * TMWDEFS_GLOBAL tmwsesn_getSessionName(
  TMWSESN *pSession)
{
  if(pSession != TMWDEFS_NULL)
  {
    return(tmwtarg_getSessionName(pSession));
  }
  return("Name Not Initialized");
}

/* function: tmwsesn_getChannelName */
const TMWTYPES_CHAR * TMWDEFS_GLOBAL tmwsesn_getChannelName(TMWSESN *pSession)
{
  const TMWTYPES_CHAR *pName;

  if((pName = tmwchnl_getChannelName(pSession->pChannel)) != (const TMWTYPES_CHAR *)TMWDEFS_NULL)
  {
    return(pName);
  }

  return("Name Not Initialized");
}
#endif

/* function: tmwsesn_setOnline */
void TMWDEFS_GLOBAL tmwsesn_setOnline(
  TMWSESN *pSession,
  TMWTYPES_BOOL online)
{
  if(online) 
  {
    if(pSession->online != TMWSESN_STAT_ONLINE)
    { 
      pSession->online = TMWSESN_STAT_ONLINE; 
      TMWSESN_STAT_CALLBACK_FUNC(pSession, TMWSESN_STAT_ONLINE, TMWDEFS_NULL);
    }
  }
  else
  { 
    if(pSession->online != TMWSESN_STAT_OFFLINE)
    { 
      pSession->online = TMWSESN_STAT_OFFLINE; 
      TMWSESN_STAT_CALLBACK_FUNC(pSession, TMWSESN_STAT_OFFLINE, TMWDEFS_NULL);
    }
  }
}

/* function: tmwsesn_initTxData */
void TMWDEFS_GLOBAL tmwsesn_initTxData(
  TMWSESN_TX_DATA *pTxData, 
  TMWTYPES_UCHAR *pBuf, 
  TMWTYPES_USHORT maxBufLen)
{
  ASSERT(pBuf != TMWDEFS_NULL);
  ASSERT(maxBufLen > 0);

  /* Initialize message description */
  pTxData->pMsgDescription = "\0";

  /* Initialize message buffer */
  pTxData->msgLength = 0;
  pTxData->pMsgBuf = pBuf;
  pTxData->maxLength = maxBufLen;

  /* Clear transmit flags */
  pTxData->txFlags = 0;

  /* Clear callback info */
  pTxData->pCallbackData = TMWDEFS_NULL;
  pTxData->pBeforeTxCallback = TMWDEFS_NULL;
  pTxData->pAfterTxCallback = TMWDEFS_NULL;
  pTxData->pFailedTxCallback = TMWDEFS_NULL;
  pTxData->spduType = 0;
}
