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

/* file: mdnpo091.c
 * description: DNP Master functionality for Object 91 Activate Configuration 
 *  status response.
 */
#include "tmwscl/dnp/dnpdiag.h"
#include "tmwscl/dnp/mdnpdiag.h"
#include "tmwscl/dnp/dnpchnl.h"
#include "tmwscl/dnp/mdnpo091.h"
#include "tmwscl/dnp/mdnpdata.h"

#include "tmwscl/utils/tmwdb.h"
#include "tmwscl/utils/tmwtarg.h"
 
#if MDNPDATA_SUPPORT_OBJ91
#if TMWCNFG_SUPPORT_ASYNCH_DB

#define MDNPO091_MAX_STRING_LEN 64

/* Structure used to store data for asynch database updates */
typedef struct MDNPO091DataStruct {
  TMWDB_DATA tmw;
  TMWTYPES_ULONG timeDelay;
  TMWTYPES_UCHAR statusCode;
  TMWTYPES_UCHAR index;
  TMWTYPES_UCHAR buflen;
  TMWTYPES_UCHAR buf[MDNPO091_MAX_STRING_LEN];
} MDNPO091_DATA;

/* function: _dbStoreFunc */
static TMWTYPES_BOOL TMWDEFS_LOCAL _dbStoreFunc(
  TMWDB_DATA *pData)
{
  MDNPO091_DATA *pDNPData = (MDNPO091_DATA *)pData;

  mdnpdata_storeActiveConfig(pData->pDbHandle, 
    pDNPData->index, pDNPData->timeDelay, pDNPData->statusCode,
    pDNPData->buf, pDNPData->buflen);

  return(TMWDEFS_TRUE);
}

/* function: storeActiveConfig */
static void TMWDEFS_LOCAL _storeActiveConfig(
  void *pDbHandle, 
  TMWTYPES_UCHAR index,
  TMWTYPES_ULONG timeDelay,
  TMWTYPES_UCHAR statusCode,
  TMWTYPES_UCHAR *pBuf,
  TMWTYPES_UCHAR buflen)
{
  /* Allocate new binary data structure */
  MDNPO091_DATA *pDNPData = (MDNPO091_DATA *)tmwtarg_alloc(sizeof(MDNPO091_DATA));
  if (pDNPData == TMWDEFS_NULL)
  {
    return;
  }

  /* Initialize data independent parts */
  tmwdb_initData((TMWDB_DATA *)pDNPData, pDbHandle, _dbStoreFunc);

  /* Initialize data dependent parts */
  if(buflen > MDNPO091_MAX_STRING_LEN)
    buflen = MDNPO091_MAX_STRING_LEN;

  memcpy(pDNPData->buf, pBuf, buflen);
  pDNPData->buflen = buflen;
  pDNPData->index = index;
  pDNPData->statusCode = statusCode;
  pDNPData->timeDelay = timeDelay;

  /* Store in database queue */
  if(!tmwdb_addEntry((TMWDB_DATA *)pDNPData))
    tmwtarg_free(pDNPData);
}
#else
/* function: _storeActiveConfig */
static void TMWDEFS_LOCAL _storeActiveConfig(
  void *pDbHandle, 
  TMWTYPES_UCHAR index,
  TMWTYPES_ULONG timeDelay,
  TMWTYPES_UCHAR statusCode,
  TMWTYPES_UCHAR *pBuf,
  TMWTYPES_UCHAR buflen)
{ 
  mdnpdata_storeActiveConfig(pDbHandle, index, timeDelay, statusCode, pBuf, buflen);
}
#endif /* TMWCNFG_SUPPORT_ASYNCH_DB */ 

/* function: mdnpo091_respObj91v1 */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpo091_respObj91v1(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{
  TMWTYPES_ULONG timeDelay;
  TMWTYPES_USHORT index;
  TMWTYPES_UCHAR numRecords;
  TMWTYPES_UCHAR statusCode;
  TMWTYPES_UCHAR bufLen;
  
  MDNPSESN *pMDNPSession = (MDNPSESN *)pSession;
  TMWTARG_UNUSED_PARAM(pObjHeader);

  if(pObjHeader->qualifier == DNPDEFS_QUAL_16BIT_FREE_FORMAT)
  {    
    /* We don't care about the length */
    pRxFragment->offset += 2; 
  }
  /* allow old qualifier for backward compatibility */
  else if (pObjHeader->qualifier != DNPDEFS_QUAL_8BIT_LIMITED_QTY)
  {
    DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_QUALIFIER);
    return(TMWDEFS_FALSE);
  }

  /* Protect against badly formatted message */
  if((pRxFragment->offset+5) > pRxFragment->msgLength)
  {
    DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_INVALID_SIZE);
    return(TMWDEFS_FALSE);
  }

  /* Time Delay */
  tmwtarg_get32(&pRxFragment->pMsgBuf[pRxFragment->offset], &timeDelay);
  pRxFragment->offset += 4;

  /* Number of status elements */
  numRecords = pRxFragment->pMsgBuf[pRxFragment->offset++];

  for(index = 0; index < numRecords; index++)
  {
    /* Protect against badly formatted message 
     */
    if(pRxFragment->offset >= pRxFragment->msgLength)
    {
      DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_INVALID_SIZE);
      return(TMWDEFS_FALSE);
    }

    bufLen = pRxFragment->pMsgBuf[pRxFragment->offset++];
    if((bufLen == 0)
      ||((pRxFragment->offset + bufLen) > pRxFragment->msgLength))
    {
      DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_INVALID_SIZE);
      return(TMWDEFS_FALSE);
    }

    statusCode = pRxFragment->pMsgBuf[pRxFragment->offset++];

    DNPDIAG_SHOW_ACTIVATE_CONFIG(pSession, timeDelay, statusCode, (pRxFragment->pMsgBuf + pRxFragment->offset), (TMWTYPES_UCHAR)(bufLen-1));

    _storeActiveConfig(pMDNPSession->pDbHandle, (TMWTYPES_UCHAR)index, timeDelay, statusCode, pRxFragment->pMsgBuf + pRxFragment->offset, (TMWTYPES_UCHAR)(bufLen-1)); 
    pRxFragment->offset = pRxFragment->offset + bufLen-1;
  }
  return(TMWDEFS_TRUE);
}
#endif /* MDNPDATA_SUPPORT_OBJ91 */
