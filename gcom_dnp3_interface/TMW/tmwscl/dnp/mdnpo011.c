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

/* file: mdnpo011.c
 * description: DNP Master functionality for Object 11 Binary Output Events
 */
#include "tmwscl/dnp/dnpdiag.h"
#include "tmwscl/dnp/dnputil.h"
#include "tmwscl/dnp/dnpdtime.h"
#include "tmwscl/dnp/mdnpo011.h"
#include "tmwscl/dnp/mdnpdata.h"
#include "tmwscl/utils/tmwtarg.h"

#include "tmwscl/utils/tmwdb.h"
#include "tmwscl/utils/tmwtarg.h"

#if MDNPDATA_SUPPORT_OBJ11
#if TMWCNFG_SUPPORT_ASYNCH_DB

/* Structure used to store data for asynch database updates */
typedef struct MDNPO011DataStruct {
  TMWDB_DATA tmw;
  TMWTYPES_UCHAR value;
  TMWTYPES_USHORT pointNumber;
  TMWTYPES_BOOL timeSpecified;
  TMWDTIME timeStamp;
} MDNPO011_DATA;

/* function: _dbStoreFunc */
static TMWTYPES_BOOL TMWDEFS_LOCAL _dbStoreFunc(
  TMWDB_DATA *pData)
{
  MDNPO011_DATA *pDNPData = (MDNPO011_DATA *)pData;
  TMWDTIME *pTimeStamp = (pDNPData->timeSpecified) ? &pDNPData->timeStamp : TMWDEFS_NULL;

  mdnpdata_storeBinaryOutput(pData->pDbHandle, 
    pDNPData->pointNumber, pDNPData->value, TMWDEFS_TRUE, pTimeStamp);

  return(TMWDEFS_TRUE);
}

/* function: _storeBinaryOutput */
static void TMWDEFS_LOCAL _storeBinaryOutput(
  void *pDbHandle, 
  TMWTYPES_USHORT pointNumber, 
  TMWTYPES_UCHAR value,
  TMWDTIME *pTimeStamp)
{
  /* Allocate new binary data structure */
  MDNPO011_DATA *pDNPData = (MDNPO011_DATA *)tmwtarg_alloc(sizeof(MDNPO011_DATA));
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
/* function: _storeBinaryOutput */
static void TMWDEFS_LOCAL _storeBinaryOutput(
  void *pDbHandle, 
  TMWTYPES_USHORT pointNumber, 
  TMWTYPES_UCHAR value,
  TMWDTIME *pTimeStamp)
{
  mdnpdata_storeBinaryOutput(pDbHandle, 
    pointNumber, value, TMWDEFS_TRUE, pTimeStamp);
}
#endif /* TMWCNFG_SUPPORT_ASYNCH_DB */
#endif /* MDNPDATA_SUPPORT_OBJ11 */

#if MDNPDATA_SUPPORT_OBJ11_V1
/* file: mdnpo011_readObj11v1 */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpo011_readObj11v1(
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

    DNPDIAG_SHOW_BINARY_OUTPUT(pSession, pointNumber, flags, TMWDEFS_TRUE, TMWDEFS_NULL);

    _storeBinaryOutput(pMDNPSession->pDbHandle, 
      pointNumber, flags, TMWDEFS_NULL);
  }

  return(TMWDEFS_TRUE);
}
#endif /* MDNPDATA_SUPPORT_OBJ11_V1 */

#if MDNPDATA_SUPPORT_OBJ11_V2
/* file: mdnpo011_readObj11v2 */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpo011_readObj11v2(
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

    DNPDIAG_SHOW_BINARY_OUTPUT(pSession, pointNumber, flags, TMWDEFS_TRUE, &timeStamp);

    _storeBinaryOutput(pMDNPSession->pDbHandle, 
      pointNumber, flags, &timeStamp);
  }

  return(TMWDEFS_TRUE);
}
#endif /* MDNPDATA_SUPPORT_OBJ11_V2 */
  

