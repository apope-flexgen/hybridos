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

/* file: mdnpo052.c
 * description: DNP Master functionality for Object 52 Time Delay
 */
#include "tmwscl/dnp/dnpdiag.h"
#include "tmwscl/dnp/mdnpdiag.h"
#include "tmwscl/dnp/mdnpo052.h"
#include "tmwscl/dnp/mdnpdata.h"
#include "tmwscl/dnp/dnpdtime.h"

#include "tmwscl/utils/tmwdb.h"
#include "tmwscl/utils/tmwtarg.h"

#if TMWCNFG_SUPPORT_ASYNCH_DB

/* Structure used to store data for asynch database updates */
typedef struct MDNPO052DataStruct {
  TMWDB_DATA tmw;
  TMWTYPES_ULONG time;
} MDNPO052_DATA;

/* function: _dbStoreFunc */
static TMWTYPES_BOOL TMWDEFS_LOCAL _dbStoreFunc(
  TMWDB_DATA *pData)
{
  MDNPO052_DATA *pDNPData = (MDNPO052_DATA *)pData;

  mdnpdata_storeRestartTime(pData->pDbHandle, pDNPData->time);

  return(TMWDEFS_TRUE);
}

/* function: _storeRestartTime */
static void TMWDEFS_LOCAL _storeRestartTime(
  void *pDbHandle, 
  TMWTYPES_ULONG time)
{
  /* Allocate new binary data structure */
  MDNPO052_DATA *pDNPData = (MDNPO052_DATA *)tmwtarg_alloc(sizeof(MDNPO052_DATA));
  if (pDNPData == TMWDEFS_NULL)
  {
    return;
  }

  /* Initialize data independent parts */
  tmwdb_initData((TMWDB_DATA *)pDNPData, pDbHandle, _dbStoreFunc);

  /* Initialize data dependent parts */
  pDNPData->time = time;

  /* Store in database queue */
  if(!tmwdb_addEntry((TMWDB_DATA *)pDNPData))
    tmwtarg_free(pDNPData);
}
#else
/* function: _storeRestartTime */
static void TMWDEFS_LOCAL _storeRestartTime(
  void *pDbHandle, 
  TMWTYPES_ULONG time)
{
  mdnpdata_storeRestartTime(pDbHandle, time);
}
#endif /* TMWCNFG_SUPPORT_ASYNCH_DB */

/* function: mdnpo052_storeObj52v1 */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpo052_storeObj52v1(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{
  TMWSESN_TX_DATA *pCurrentMessage;
  TMWTYPES_USHORT msgTimeInSeconds;

  TMWTARG_UNUSED_PARAM(pObjHeader);

  tmwtarg_get16(pRxFragment->pMsgBuf + pRxFragment->offset, &msgTimeInSeconds);
  pRxFragment->offset += 2;

#ifdef TMW_SUPPORT_MONITOR
  /* If this channel is in passive listen only analyzer mode
   * there will not be any pCurrentMessage because nothing gets
   * sent from this channel.
   */
  if(pSession->pChannel->pPhysContext->monitorMode)
  {
    MDNPDIAG_SHOW_MSG_DELAY(pSession, msgTimeInSeconds, TMWDEFS_FALSE);
    return(TMWDEFS_TRUE);
  }
#endif

  pCurrentMessage = dnputil_getCurrentMessage(pSession);
  switch (pCurrentMessage->pMsgBuf[DNPDEFS_AH_INDEX_FUNC_CODE])
  {
  case DNPDEFS_FC_COLD_RESTART:
  case DNPDEFS_FC_WARM_RESTART:
    {
      MDNPSESN *pMDNPSession = (MDNPSESN *)pSession;
      MDNPDIAG_SHOW_MSG_DELAY(pSession, msgTimeInSeconds, TMWDEFS_FALSE);

      _storeRestartTime(pMDNPSession->pDbHandle, (msgTimeInSeconds * 1000));
      break;
    }
  }

  return(TMWDEFS_TRUE);
}

/* function: mdnpo052_storeObj52v2 */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpo052_storeObj52v2(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{
  TMWSESN_TX_DATA *pCurrentMessage;
  TMWTYPES_USHORT msgTime;

  TMWTARG_UNUSED_PARAM(pObjHeader);

  tmwtarg_get16(pRxFragment->pMsgBuf + pRxFragment->offset, &msgTime);
  pRxFragment->offset += 2;

#ifdef TMW_SUPPORT_MONITOR
  /* If this channel is in passive listen only analyzer mode
   * there will not be any pCurrentMessage because nothing gets
   * sent from this channel.
   */
  if(pSession->pChannel->pPhysContext->monitorMode)
  {
    MDNPDIAG_SHOW_MSG_DELAY(pSession, msgTime, TMWDEFS_TRUE);
    return(TMWDEFS_TRUE);
  }
#endif

  pCurrentMessage = dnputil_getCurrentMessage(pSession);
  switch (pCurrentMessage->pMsgBuf[DNPDEFS_AH_INDEX_FUNC_CODE])
  {
#if MDNPDATA_SUPPORT_OBJ50_V1
  case DNPDEFS_FC_DELAY_MEASURE:
    {
      TMWTYPES_MILLISECONDS timeDelta;
      MDNPSESN *pMDNPSession = (MDNPSESN *)pSession;

      timeDelta = pRxFragment->firstByteTime - pMDNPSession->delayMeasurementTxTime;

      /* Because of granularity of clocks, make sure the masters total elapsed time
       * is not less than the slaves delay time.
       */
      if(timeDelta > msgTime) 
        pMDNPSession->propagationDelay = (TMWTYPES_USHORT)((timeDelta - msgTime) / 2);
      else
        pMDNPSession->propagationDelay = 0;

      MDNPDIAG_SHOW_MSG_DELAY(pSession, msgTime, TMWDEFS_TRUE);
      MDNPDIAG_SHOW_DELAY(pSession, timeDelta, pMDNPSession->propagationDelay);
    }
    break;
#endif

  case DNPDEFS_FC_COLD_RESTART:
  case DNPDEFS_FC_WARM_RESTART:
    {
      MDNPSESN *pMDNPSession = (MDNPSESN *)pSession;
      MDNPDIAG_SHOW_MSG_DELAY(pSession, msgTime, TMWDEFS_TRUE);

      _storeRestartTime(pMDNPSession->pDbHandle, msgTime);
      break;
    }
  }

  return(TMWDEFS_TRUE);
}
