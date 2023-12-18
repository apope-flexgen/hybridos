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

/* file: mdnpo012.c
 * description: DNP Master functionality for Object 12 Control Block
 */
#include "tmwscl/dnp/dnpdiag.h"
#include "tmwscl/dnp/mdnpdiag.h"
#include "tmwscl/dnp/dnpchnl.h"
#include "tmwscl/dnp/mdnpo012.h"
#include "tmwscl/dnp/mdnpdata.h"

#include "tmwscl/utils/tmwdb.h"
#include "tmwscl/utils/tmwtarg.h"
 
#if MDNPDATA_SUPPORT_OBJ12
#if TMWCNFG_SUPPORT_ASYNCH_DB

/* Structure used to store data for asynch database updates */
typedef struct MDNPO012DataStruct {
  TMWDB_DATA tmw;
  TMWTYPES_UCHAR value;
  TMWTYPES_USHORT pointNumber;
  TMWTYPES_BOOL timeSpecified;
  TMWDTIME timeStamp;
} MDNPO012_DATA;

/* function: _dbStoreFunc */
static TMWTYPES_BOOL TMWDEFS_LOCAL _dbStoreFunc(
  TMWDB_DATA *pData)
{
  MDNPO012_DATA *pDNPData = (MDNPO012_DATA *)pData;
  TMWDTIME *pTimeStamp = (pDNPData->timeSpecified) ? &pDNPData->timeStamp : TMWDEFS_NULL;

  mdnpdata_storeBinaryCmdStatus(pData->pDbHandle, 
    pDNPData->pointNumber, pDNPData->value, TMWDEFS_FALSE, pTimeStamp);

  return(TMWDEFS_TRUE);
}

/* function: _storeBinaryOutputCmdStatus */
static void TMWDEFS_LOCAL _storeBinaryOutputCmdStatus(
  void *pDbHandle, 
  TMWTYPES_USHORT pointNumber, 
  TMWTYPES_UCHAR value,
  TMWDTIME *pTimeStamp)
{
  /* Allocate new binary data structure */
  MDNPO012_DATA *pDNPData = (MDNPO012_DATA *)tmwtarg_alloc(sizeof(MDNPO012_DATA));
  if (pDNPData == TMWDEFS_NULL)
  {
    return;
  }

  /* Initialize data independent parts */
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
/* function: _storeBinaryOutputCmdStatus */
static void TMWDEFS_LOCAL _storeBinaryOutputCmdStatus(
  void *pDbHandle, 
  TMWTYPES_USHORT pointNumber, 
  TMWTYPES_UCHAR value,
  TMWDTIME *pTimeStamp)
{ 
  mdnpdata_storeBinaryCmdStatus(pDbHandle, pointNumber, value, TMWDEFS_FALSE, pTimeStamp);
}
#endif /* TMWCNFG_SUPPORT_ASYNCH_DB */ 

/* function: mdnpo012_selOperRespObj12 */
DNPCHNL_RESP_STATUS TMWDEFS_GLOBAL mdnpo012_selOperRespObj12(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{
  TMWTYPES_USHORT pointNumber;
  TMWTYPES_USHORT index;
  TMWTYPES_UCHAR controlCode;
  TMWTYPES_UCHAR count;
  TMWTYPES_UCHAR status;
  TMWTYPES_ULONG onTime;
  TMWTYPES_ULONG offTime;
  MDNPSESN *pMDNPSession = (MDNPSESN *)pSession;
  DNPCHNL_RESP_STATUS retStatus = DNPCHNL_RESP_STATUS_SUCCESS;

  for(index = 0; index < pObjHeader->numberOfPoints; index++)
  {
    dnputil_getPointNumber(pRxFragment, pObjHeader, index, &pointNumber);

    controlCode = pRxFragment->pMsgBuf[pRxFragment->offset++];
    count = pRxFragment->pMsgBuf[pRxFragment->offset++];

    /* On Time */
    tmwtarg_get32(&pRxFragment->pMsgBuf[pRxFragment->offset], &onTime);
    pRxFragment->offset += 4;
 
    /* Off Time */
    tmwtarg_get32(&pRxFragment->pMsgBuf[pRxFragment->offset], &offTime);
    pRxFragment->offset += 4;

    /* Read Status */
    status = pRxFragment->pMsgBuf[pRxFragment->offset++];
 
    /* Diagnostics */
    DNPDIAG_SHOW_CROB(pSession, pointNumber, controlCode, count, onTime, offTime, status, TMWDIAG_ID_RX);

    _storeBinaryOutputCmdStatus(pMDNPSession->pDbHandle, pointNumber, status, TMWDEFS_NULL);

    if(status != 0)
    {   
      MDNPDIAG_ERROR(pSession->pChannel, pSession, MDNPDIAG_STATUS_NOT_ZERO);
      retStatus = DNPCHNL_RESP_STATUS_STATUSCODE;
    }
  }

  return(retStatus);
}
#endif /* MDNPDATA_SUPPORT_OBJ12_V1 */
