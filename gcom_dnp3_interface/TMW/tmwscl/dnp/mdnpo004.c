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

/* file: mdnpo004.c
 * description: DNP Master functionality for Object 4 Double Bit Input Events
 */
#include "tmwscl/dnp/dnpdiag.h"
#include "tmwscl/dnp/dnputil.h"
#include "tmwscl/dnp/dnpdtime.h"
#include "tmwscl/dnp/mdnpo004.h"
#include "tmwscl/dnp/mdnpdata.h"
#include "tmwscl/dnp/mdnpdiag.h"
#include "tmwscl/utils/tmwtarg.h"

#include "tmwscl/utils/tmwdb.h"
#include "tmwscl/utils/tmwtarg.h"

#if MDNPDATA_SUPPORT_OBJ4
#if TMWCNFG_SUPPORT_ASYNCH_DB

/* Structure used to store data for asynch database updates */
typedef struct MDNPO004DataStruct {
  TMWDB_DATA tmw;
  TMWTYPES_UCHAR value;
  TMWTYPES_USHORT pointNumber;
  TMWTYPES_BOOL timeSpecified;
  TMWDTIME timeStamp;
} MDNPO004_DATA;

/* function: _dbStoreFunc */
static TMWTYPES_BOOL TMWDEFS_LOCAL _dbStoreFunc(
  TMWDB_DATA *pData)
{
  MDNPO004_DATA *pDNPData = (MDNPO004_DATA *)pData;
  TMWDTIME *pTimeStamp = (pDNPData->timeSpecified) ? &pDNPData->timeStamp : TMWDEFS_NULL;

  mdnpdata_storeDoubleInput(pData->pDbHandle, 
    pDNPData->pointNumber, pDNPData->value, TMWDEFS_TRUE, pTimeStamp);

  return(TMWDEFS_TRUE);
}

/* function: _storeDoubleInput */
static void TMWDEFS_LOCAL _storeDoubleInput(
  void *pDbHandle, 
  TMWTYPES_USHORT pointNumber, 
  TMWTYPES_UCHAR value,
  TMWDTIME *pTimeStamp)
{
  /* Allocate new double bit data structure */
  MDNPO004_DATA *pDNPData = (MDNPO004_DATA *)tmwtarg_alloc(sizeof(MDNPO004_DATA));
  if (pDNPData == TMWDEFS_NULL)
  {
    return;
  }

  /* Initialize data indepdendent parts */
  tmwdb_initData((TMWDB_DATA *)pDNPData, pDbHandle, _dbStoreFunc);

  /* Initialize data dependent parts */
  pDNPData->value = value;
  pDNPData->pointNumber = pointNumber;
  pDNPData->timeSpecified = TMWDEFS_FALSE;
  if(pTimeStamp != TMWDEFS_NULL)
  {
    pDNPData->timeSpecified = TMWDEFS_TRUE;
    pDNPData->timeStamp = *pTimeStamp;
  }

  /* Store in database queue */
  if(!tmwdb_addEntry((TMWDB_DATA *)pDNPData))
    tmwtarg_free(pDNPData);
}
#else
/* function: _storeDoubleInput */
static void TMWDEFS_LOCAL _storeDoubleInput(
  void *pDbHandle, 
  TMWTYPES_USHORT pointNumber, 
  TMWTYPES_UCHAR value,
  TMWDTIME *pTimeStamp)
{
  mdnpdata_storeDoubleInput(pDbHandle, 
    pointNumber, value, TMWDEFS_TRUE, pTimeStamp);
}
#endif /* TMWCNFG_SUPPORT_ASYNCH_DB */
#endif /* MDNPDATA_SUPPORT_OBJ4 */

#if MDNPDATA_SUPPORT_OBJ4_V1
/* function: mdnpo004_readObj4v1 */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpo004_readObj4v1(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{
  MDNPSESN *pMDNPSession = (MDNPSESN *)pSession;
  TMWTYPES_USHORT pointNumber;
  TMWTYPES_UCHAR flags;
  TMWTYPES_USHORT i;

  for(i = 0; i < pObjHeader->numberOfPoints; i++)
  {
    dnputil_getPointNumber(pRxFragment, pObjHeader, i, &pointNumber);

    flags = pRxFragment->pMsgBuf[pRxFragment->offset++];

    DNPDIAG_SHOW_DOUBLE_INPUT(pSession, pointNumber, flags, TMWDEFS_TRUE, TMWDEFS_NULL);

    _storeDoubleInput(pMDNPSession->pDbHandle, 
      pointNumber, flags, TMWDEFS_NULL);
  }

  return(TMWDEFS_TRUE);
}
#endif /* MDNPDATA_SUPPORT_OBJ4_V1 */

#if MDNPDATA_SUPPORT_OBJ4_V2
/* function: mdnpo004_readObj4v2 */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpo004_readObj4v2(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{
  MDNPSESN *pMDNPSession = (MDNPSESN *)pSession;
  TMWTYPES_MS_SINCE_70 msSince70;
  TMWTYPES_USHORT pointNumber;
  TMWTYPES_UCHAR flags;
  TMWDTIME timeStamp;
  TMWTYPES_USHORT i;

  for(i = 0; i < pObjHeader->numberOfPoints; i++)
  {
    dnputil_getPointNumber(pRxFragment, pObjHeader, i, &pointNumber);

    flags = pRxFragment->pMsgBuf[pRxFragment->offset++];

    dnpdtime_readMsSince70(&msSince70, &pRxFragment->pMsgBuf[pRxFragment->offset]);
    dnpdtime_msSince70ToDateTime(&timeStamp, &msSince70);
    pRxFragment->offset += 6;

    DNPDIAG_SHOW_DOUBLE_INPUT(pSession, pointNumber, flags, TMWDEFS_TRUE, &timeStamp);

    _storeDoubleInput(pMDNPSession->pDbHandle, 
      pointNumber, flags, &timeStamp);
  }

  return(TMWDEFS_TRUE);
}
#endif /* MDNPDATA_SUPPORT_OBJ4_V2 */

#if MDNPDATA_SUPPORT_OBJ4_V3
/* function: mdnpo004_readObj4v3 */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpo004_readObj4v3(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{
  MDNPSESN *pMDNPSession = (MDNPSESN *)pSession;
  TMWTYPES_USHORT pointNumber;
  TMWTYPES_USHORT relTime;
  TMWTYPES_UCHAR flags;
  TMWDTIME eventTime;
  TMWTYPES_USHORT i;

  /* A new CTO must be received in each fragment that contains an obj2var3 or obj4var3 */
  if (pMDNPSession->lastCTOReceived.year == 0)
  {
    MDNPDIAG_ERROR(pSession->pChannel, pSession, MDNPDIAG_NO_CTO);
    return(TMWDEFS_FALSE);
  }

  for(i = 0; i < pObjHeader->numberOfPoints; i++)
  {
    dnputil_getPointNumber(pRxFragment, pObjHeader, i, &pointNumber);

    flags = pRxFragment->pMsgBuf[pRxFragment->offset++];

    tmwtarg_get16(&pRxFragment->pMsgBuf[pRxFragment->offset], &relTime);
    pRxFragment->offset += 2;

    eventTime = pMDNPSession->lastCTOReceived;

    tmwdtime_addOffset(&eventTime, relTime);

    eventTime.qualifier =     
      (pMDNPSession->lastCTOReceived.invalid) 
      ? TMWDTIME_UNSYNC : TMWDTIME_SYNC;

    DNPDIAG_SHOW_DOUBLE_INPUT(pSession, pointNumber, flags, TMWDEFS_TRUE, &eventTime);

    _storeDoubleInput(pMDNPSession->pDbHandle, pointNumber, flags, &eventTime);
  }

  return(TMWDEFS_TRUE);
}
#endif /* MDNPDATA_SUPPORT_OBJ4_V3 */

