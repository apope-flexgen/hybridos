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

/* file: mdnpo021.c
 * description: DNP Master functionality for Object 21 Frozen Counters
 */
#include "tmwscl/dnp/dnpdiag.h"
#include "tmwscl/dnp/mdnpo021.h"
#include "tmwscl/dnp/mdnpdata.h"
#include "tmwscl/dnp/dnpdtime.h"

#include "tmwscl/utils/tmwdb.h"
#include "tmwscl/utils/tmwtarg.h"

#if MDNPDATA_SUPPORT_OBJ21
#if TMWCNFG_SUPPORT_ASYNCH_DB

/* Structure used to store data for asynch database updates */
typedef struct MDNPO021DataStruct {
  TMWDB_DATA tmw;
  TMWTYPES_UCHAR flags;
  TMWTYPES_ULONG value;
  TMWTYPES_USHORT pointNumber;
  TMWTYPES_BOOL timeSpecified;
  TMWDTIME timeStamp;
} MDNPO021_DATA;

/* function: _dbStoreFunc */
static TMWTYPES_BOOL TMWDEFS_LOCAL _dbStoreFunc(
  TMWDB_DATA *pData)
{
  MDNPO021_DATA *pDNPData = (MDNPO021_DATA *)pData;
  TMWDTIME *pTimeStamp = (pDNPData->timeSpecified) ? &pDNPData->timeStamp : TMWDEFS_NULL;

  mdnpdata_storeFrozenCounter(pData->pDbHandle, 
    pDNPData->pointNumber, pDNPData->value, pDNPData->flags, 
    TMWDEFS_FALSE, pTimeStamp);

  return(TMWDEFS_TRUE);
}

/* function: _storeFrozenCounter */
static void TMWDEFS_LOCAL _storeFrozenCounter(
  void *pDbHandle, 
  TMWTYPES_USHORT pointNumber, 
  TMWTYPES_ULONG value,
  TMWTYPES_UCHAR flags,
  TMWDTIME *pTimeStamp)
{
  /* Allocate new binary data structure */
  MDNPO021_DATA *pDNPData = (MDNPO021_DATA *)tmwtarg_alloc(sizeof(MDNPO021_DATA));
  if (pDNPData == TMWDEFS_NULL)
  {
    return;
  }

  /* Initialize data indepdendent parts */
  tmwdb_initData((TMWDB_DATA *)pDNPData, pDbHandle, _dbStoreFunc);

  /* Initialize data dependent parts */
  pDNPData->flags = flags;
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
/* function: _storeFrozenCounter */
static void TMWDEFS_LOCAL _storeFrozenCounter(
  void *pDbHandle, 
  TMWTYPES_USHORT pointNumber, 
  TMWTYPES_ULONG value,
  TMWTYPES_UCHAR flags,
  TMWDTIME *pTimeStamp)
{
  mdnpdata_storeFrozenCounter(pDbHandle, pointNumber, 
    value, flags, TMWDEFS_FALSE, pTimeStamp);
}
#endif /* TMWCNFG_SUPPORT_ASYNCH_DB */
#endif /* MDNPDATA_SUPPORT_OBJ21 */

#if MDNPDATA_SUPPORT_OBJ21_V1
/* function: mdnpo021_readObj21v1 */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpo021_readObj21v1(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{
  MDNPSESN *pMDNPSession = (MDNPSESN *)pSession;
  TMWTYPES_USHORT pointNumber;
  TMWTYPES_ULONG value;
  TMWTYPES_UCHAR flags;
  TMWTYPES_USHORT index;

  for(index = 0; index < pObjHeader->numberOfPoints; index++)
  {
    dnputil_getPointNumber(pRxFragment, pObjHeader, index, &pointNumber);

    flags = pRxFragment->pMsgBuf[pRxFragment->offset++];

    tmwtarg_get32(&pRxFragment->pMsgBuf[pRxFragment->offset], &value);
    pRxFragment->offset += 4;

    DNPDIAG_SHOW_FROZEN_COUNTER(pSession, pointNumber, value, flags, TMWDEFS_FALSE, TMWDEFS_NULL);

    _storeFrozenCounter(pMDNPSession->pDbHandle, pointNumber, value, flags, TMWDEFS_NULL);
  }

  return(TMWDEFS_TRUE);
}
#endif /* MDNPDATA_SUPPORT_OBJ21_V1 */

#if MDNPDATA_SUPPORT_OBJ21_V2
/* function: mdnpo021_readObj21v2 */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpo021_readObj21v2(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{
  MDNPSESN *pMDNPSession = (MDNPSESN *)pSession;
  TMWTYPES_USHORT pointNumber;
  TMWTYPES_USHORT value;
  TMWTYPES_UCHAR flags;
  TMWTYPES_USHORT index;

  for(index = 0; index < pObjHeader->numberOfPoints; index++)
  {
    dnputil_getPointNumber(pRxFragment, pObjHeader, index, &pointNumber);

    flags = pRxFragment->pMsgBuf[pRxFragment->offset++];

    tmwtarg_get16(&pRxFragment->pMsgBuf[pRxFragment->offset], &value);
    pRxFragment->offset += 2;

    DNPDIAG_SHOW_FROZEN_COUNTER(pSession, pointNumber, value, flags, TMWDEFS_FALSE, TMWDEFS_NULL);

    _storeFrozenCounter(pMDNPSession->pDbHandle, pointNumber, value, flags, TMWDEFS_NULL);
  }

  return(TMWDEFS_TRUE);
}
#endif /* MDNPDATA_SUPPORT_OBJ21_V2 */

#if MDNPDATA_SUPPORT_OBJ21_V5
/* function: mdnpo021_readObj21v5 */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpo021_readObj21v5(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{
  MDNPSESN *pMDNPSession = (MDNPSESN *)pSession;
  TMWTYPES_MS_SINCE_70 msSince70;
  TMWTYPES_USHORT pointNumber;
  TMWTYPES_ULONG value;
  TMWTYPES_UCHAR flags;
  TMWDTIME dateTime;
  TMWTYPES_USHORT index;

  for(index = 0; index < pObjHeader->numberOfPoints; index++)
  {
    dnputil_getPointNumber(pRxFragment, pObjHeader, index, &pointNumber);

    flags = pRxFragment->pMsgBuf[pRxFragment->offset++];

    tmwtarg_get32(&pRxFragment->pMsgBuf[pRxFragment->offset], &value);
    pRxFragment->offset += 4;

    dnpdtime_readMsSince70(&msSince70, &pRxFragment->pMsgBuf[pRxFragment->offset]);  
    dnpdtime_msSince70ToDateTime(&dateTime, &msSince70);
    pRxFragment->offset += 6;

    DNPDIAG_SHOW_FROZEN_COUNTER(pSession, pointNumber, value, flags, TMWDEFS_FALSE, &dateTime);

    _storeFrozenCounter(pMDNPSession->pDbHandle, pointNumber, value, flags, &dateTime);
  }

  return(TMWDEFS_TRUE);
}
#endif /* MDNPDATA_SUPPORT_OBJ21_V5 */

#if MDNPDATA_SUPPORT_OBJ21_V6
/* function: mdnpo021_readObj21v6 */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpo021_readObj21v6(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{
  MDNPSESN *pMDNPSession = (MDNPSESN *)pSession;
  TMWTYPES_MS_SINCE_70 msSince70;
  TMWTYPES_USHORT pointNumber;
  TMWTYPES_USHORT value;
  TMWTYPES_UCHAR flags;
  TMWDTIME dateTime;
  TMWTYPES_USHORT index;

  for(index = 0; index < pObjHeader->numberOfPoints; index++)
  {
    dnputil_getPointNumber(pRxFragment, pObjHeader, index, &pointNumber);

    flags = pRxFragment->pMsgBuf[pRxFragment->offset++];

    tmwtarg_get16(&pRxFragment->pMsgBuf[pRxFragment->offset], &value);
    pRxFragment->offset += 2;

    dnpdtime_readMsSince70(&msSince70, &pRxFragment->pMsgBuf[pRxFragment->offset]);  
    dnpdtime_msSince70ToDateTime(&dateTime, &msSince70);
    pRxFragment->offset += 6;

    DNPDIAG_SHOW_FROZEN_COUNTER(pSession, pointNumber, value, flags, TMWDEFS_FALSE, &dateTime);

    _storeFrozenCounter(pMDNPSession->pDbHandle, pointNumber, value, flags, &dateTime);
  }

  return(TMWDEFS_TRUE);
}
#endif /* MDNPDATA_SUPPORT_OBJ21_V6 */

#if MDNPDATA_SUPPORT_OBJ21_V9
/* function: mdnpo021_readObj21v9 */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpo021_readObj21v9(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{
  MDNPSESN *pMDNPSession = (MDNPSESN *)pSession;
  TMWTYPES_USHORT pointNumber;
  TMWTYPES_ULONG value;
  TMWTYPES_USHORT index;

  for(index = 0; index < pObjHeader->numberOfPoints; index++)
  {
    dnputil_getPointNumber(pRxFragment, pObjHeader, index, &pointNumber);

    tmwtarg_get32(&pRxFragment->pMsgBuf[pRxFragment->offset], &value);
    pRxFragment->offset += 4;

    DNPDIAG_SHOW_FROZEN_COUNTER(pSession, pointNumber, value, 0x01, TMWDEFS_FALSE, TMWDEFS_NULL);

    _storeFrozenCounter(pMDNPSession->pDbHandle, pointNumber, value, 0x01, TMWDEFS_NULL);
  }

  return(TMWDEFS_TRUE);
}
#endif /* MDNPDATA_SUPPORT_OBJ21_V9 */

#if MDNPDATA_SUPPORT_OBJ21_V10
/* function: mdnpo021_readObj21v10 */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpo021_readObj21v10(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{
  MDNPSESN *pMDNPSession = (MDNPSESN *)pSession;
  TMWTYPES_USHORT pointNumber;
  TMWTYPES_USHORT value;
  TMWTYPES_USHORT index;

  for(index = 0; index < pObjHeader->numberOfPoints; index++)
  {
    dnputil_getPointNumber(pRxFragment, pObjHeader, index, &pointNumber);

    tmwtarg_get16(&pRxFragment->pMsgBuf[pRxFragment->offset], &value);
    pRxFragment->offset += 2;

    DNPDIAG_SHOW_FROZEN_COUNTER(pSession, pointNumber, value, 0x01, TMWDEFS_FALSE, TMWDEFS_NULL);

    _storeFrozenCounter(pMDNPSession->pDbHandle, pointNumber, value, 0x01, TMWDEFS_NULL);
  }

  return(TMWDEFS_TRUE);
}
#endif /* MDNPDATA_SUPPORT_OBJ21_V10 */

#if MDNPDATA_SUPPORT_OBJ21
/* function: mdnpo021_readObj21 */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpo021_readObj21(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{
  TMWTYPES_BOOL status = TMWDEFS_FALSE;
  switch(pObjHeader->variation)
  {
#if MDNPDATA_SUPPORT_OBJ21_V1
    case 1:
      status = mdnpo021_readObj21v1(pSession, pRxFragment, pObjHeader);
      break;
#endif
#if MDNPDATA_SUPPORT_OBJ21_V2
    case 2:
      status = mdnpo021_readObj21v2(pSession, pRxFragment, pObjHeader);
      break;
#endif
#if MDNPDATA_SUPPORT_OBJ21_V5
    case 5:
      status = mdnpo021_readObj21v5(pSession, pRxFragment, pObjHeader);
      break;
#endif
#if MDNPDATA_SUPPORT_OBJ21_V6
    case 6:
      status = mdnpo021_readObj21v6(pSession, pRxFragment, pObjHeader);
      break;
#endif
#if MDNPDATA_SUPPORT_OBJ21_V9
    case 9:
      status = mdnpo021_readObj21v9(pSession, pRxFragment, pObjHeader);
      break;
#endif
#if MDNPDATA_SUPPORT_OBJ21_V10
    case 10:
      status = mdnpo021_readObj21v10(pSession, pRxFragment, pObjHeader);
      break;
#endif
  }
  return(status);
}
#endif
