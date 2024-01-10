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

/* file: mdnpo115.c
 * description: DNP Master functionality for Object 115 Extended String Events
 */
#include "tmwscl/dnp/dnpdiag.h"
#include "tmwscl/dnp/mdnpdiag.h"
#include "tmwscl/dnp/mdnpo115.h"
#include "tmwscl/dnp/mdnpdata.h"
#include "tmwscl/dnp/dnpdtime.h"

#include "tmwscl/utils/tmwdb.h"
#include "tmwscl/utils/tmwtarg.h"

#if MDNPDATA_SUPPORT_OBJ115

/* function: _isVariationSupported() */
static TMWTYPES_BOOL TMWDEFS_LOCAL _isVariationSupported (
  TMWTYPES_UCHAR variation)
{
  TMWTYPES_BOOL variationSupported = TMWDEFS_FALSE;

  switch(variation)
  {
  case 1:
    variationSupported = MDNPDATA_SUPPORT_OBJ115_V1;
    break;
  case 2:
    variationSupported = MDNPDATA_SUPPORT_OBJ115_V2;
    break;
  case 3:
    variationSupported = MDNPDATA_SUPPORT_OBJ115_V3;
    break;
  case 4:
    variationSupported = MDNPDATA_SUPPORT_OBJ115_V4;
    break;
  default:
    break;
  }
  return (variationSupported);
}

#if TMWCNFG_SUPPORT_ASYNCH_DB
typedef struct MDNPO115DataStruct {
  TMWDB_DATA tmw;
  TMWTYPES_UCHAR buf[DNPCNFG_MAX_EXT_STRING_LENGTH];
  TMWTYPES_USHORT buflen;
  TMWTYPES_USHORT pointNumber;
  TMWTYPES_UCHAR  flags;
  TMWTYPES_BOOL timeSpecified;
  TMWDTIME timeStamp;
} MDNPO115_DATA;

/* function: _dbStoreFunc */
static TMWTYPES_BOOL TMWDEFS_LOCAL _dbStoreFunc(
  TMWDB_DATA *pData)
{
  MDNPO115_DATA *pDNPData = (MDNPO115_DATA *)pData;
  TMWDTIME *pTimeStamp = (pDNPData->timeSpecified) ? &pDNPData->timeStamp : TMWDEFS_NULL;

  mdnpdata_storeExtString(pData->pDbHandle, pDNPData->pointNumber, pDNPData->buf,
    pDNPData->buflen, pDNPData->flags, TMWDEFS_TRUE, pTimeStamp);

  return(TMWDEFS_TRUE);
}

/* function: _storeExtString */
static void TMWDEFS_LOCAL _storeExtString(
  void *pDbHandle, 
  TMWTYPES_USHORT pointNumber, 
  TMWTYPES_UCHAR *pBuf,
  TMWTYPES_USHORT buflen,
  TMWTYPES_UCHAR  flags,
  TMWDTIME *pTimeStamp)
{
  /* Allocate new binary data structure */
  MDNPO115_DATA *pDNPData = (MDNPO115_DATA *)tmwtarg_alloc(sizeof(MDNPO115_DATA));
  if (pDNPData == TMWDEFS_NULL)
  {
    return;
  }

  /* Initialize data indepdendent parts */
  tmwdb_initData((TMWDB_DATA *)pDNPData, pDbHandle, _dbStoreFunc);

  /* Initialize data dependent parts */
  memcpy(pDNPData->buf, pBuf, buflen);
  pDNPData->buflen = buflen;
  pDNPData->pointNumber = pointNumber;
  pDNPData->flags = flags;
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
/* function: _storeExtString */
static void TMWDEFS_LOCAL _storeExtString(
  void *pDbHandle, 
  TMWTYPES_USHORT pointNumber, 
  TMWTYPES_UCHAR *pBuf,
  TMWTYPES_USHORT buflen,
  TMWTYPES_UCHAR  flags,
  TMWDTIME *pTimeStamp)
{
  mdnpdata_storeExtString(pDbHandle, pointNumber, pBuf, buflen, flags, TMWDEFS_TRUE, pTimeStamp);
}
#endif /* TMWCNFG_SUPPORT_ASYNCH_DB */

/* function: mdnpo115_readObj115 */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpo115_readObj115(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{
  MDNPSESN *pMDNPSession = (MDNPSESN *)pSession;
  TMWTYPES_UCHAR  variation = pObjHeader->variation;
  TMWTYPES_USHORT strLen;
  TMWTYPES_USHORT i;
  TMWTYPES_UCHAR  flags = 0;
  TMWTYPES_MS_SINCE_70 msSince70;
  TMWDTIME dateTime;
  TMWDTIME *pDateTime = TMWDEFS_NULL;
  
  /* Ensure the variation is supported & valid */
  if(_isVariationSupported(variation) == TMWDEFS_FALSE)
  {
    MDNPDIAG_ERROR(pSession->pChannel, pSession, MDNPDIAG_INV_VARIATION);
    return(TMWDEFS_FALSE);
  }

  strLen = pObjHeader->variation;
  for(i = 0; i < pObjHeader->numberOfPoints; i++)
  {
    TMWTYPES_USHORT pointNumber;

    dnputil_getPointNumber(pRxFragment, pObjHeader, i, &pointNumber);

    /* Retrieve the flags */
    flags = pRxFragment->pMsgBuf[pRxFragment->offset++];

    /* Retrieve the time of occurance for variations 1 & 2 */
    if ((variation == 1) || (variation == 2))
    {
      dnpdtime_readMsSince70(&msSince70, &pRxFragment->pMsgBuf[pRxFragment->offset]);  
      dnpdtime_msSince70ToDateTime(&dateTime, &msSince70);
      pRxFragment->offset += 6;
      pDateTime = &dateTime;
    }

    /* Retrieve the string length based on variation */
    if ((variation == 1) || (variation == 3))
    {
      strLen = pRxFragment->pMsgBuf[pRxFragment->offset++];
    }
    else /* String length is 2 bytes */
    {
      tmwtarg_get16(&pRxFragment->pMsgBuf[pRxFragment->offset], &strLen);
      pRxFragment->offset += 2;
    }

    /* Protect against badly formatted message */
    if((pRxFragment->offset + strLen) > pRxFragment->msgLength)
    { 
      DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_INVALID_SIZE);
      return(TMWDEFS_FALSE);
    }

    DNPDIAG_SHOW_EXT_STRING_EVENT(pSession, pointNumber, (pRxFragment->pMsgBuf + pRxFragment->offset), strLen, flags, pDateTime);

    _storeExtString(pMDNPSession->pDbHandle, pointNumber, pRxFragment->pMsgBuf + pRxFragment->offset, strLen, flags, pDateTime);

    pRxFragment->offset = pRxFragment->offset + strLen;
  }

  return(TMWDEFS_TRUE);
}
#endif /* MDNPDATA_SUPPORT_OBJ115 */

