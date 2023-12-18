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

/* file: mdnpo114.c
 * description: DNP Master functionality for Object 114 Extended String Data
 */
#include "tmwscl/dnp/dnpdiag.h"
#include "tmwscl/dnp/mdnpdiag.h"
#include "tmwscl/dnp/mdnpo114.h"
#include "tmwscl/dnp/mdnpdata.h"

#include "tmwscl/utils/tmwdb.h"
#include "tmwscl/utils/tmwtarg.h"

#if MDNPDATA_SUPPORT_OBJ114

/* function: _isVariationSupported() */
static TMWTYPES_BOOL TMWDEFS_LOCAL _isVariationSupported (
  TMWTYPES_UCHAR variation)
{
  TMWTYPES_BOOL variationSupported = TMWDEFS_FALSE;

  switch(variation)
  {
  case 1:
    variationSupported = MDNPDATA_SUPPORT_OBJ114_V1;
    break;
  case 2:
    variationSupported = MDNPDATA_SUPPORT_OBJ114_V2;
    break;
  case 3:
    variationSupported = MDNPDATA_SUPPORT_OBJ114_V3;
    break;
  case 4:
    variationSupported = MDNPDATA_SUPPORT_OBJ114_V4;
    break;
  default:
    break;
  }
  return (variationSupported);
}

#if TMWCNFG_SUPPORT_ASYNCH_DB
typedef struct MDNPO114DataStruct {
  TMWDB_DATA tmw;
  TMWTYPES_UCHAR buf[DNPCNFG_MAX_EXT_STRING_LENGTH];
  TMWTYPES_USHORT buflen;
  TMWTYPES_USHORT pointNumber;
  TMWTYPES_UCHAR  flags;
} MDNPO114_DATA;

/* function: _dbStoreFunc */
static TMWTYPES_BOOL TMWDEFS_LOCAL _dbStoreFunc(
  TMWDB_DATA *pData)
{
  MDNPO114_DATA *pDNPData = (MDNPO114_DATA *)pData;

  mdnpdata_storeExtString(pData->pDbHandle, pDNPData->pointNumber, pDNPData->buf,
    pDNPData->buflen, pDNPData->flags, TMWDEFS_FALSE, TMWDEFS_NULL);

  return(TMWDEFS_TRUE);
}

/* function: _storeExtString */
static void TMWDEFS_LOCAL _storeExtString(
  void *pDbHandle, 
  TMWTYPES_USHORT pointNumber, 
  TMWTYPES_UCHAR *pBuf,
  TMWTYPES_USHORT buflen,
  TMWTYPES_UCHAR  flags)
{
  /* Allocate new binary data structure */
  MDNPO114_DATA *pDNPData = (MDNPO114_DATA *)tmwtarg_alloc(sizeof(MDNPO114_DATA));
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
  TMWTYPES_UCHAR  flags)
{
  mdnpdata_storeExtString(pDbHandle, pointNumber, pBuf, buflen, flags, TMWDEFS_FALSE, TMWDEFS_NULL);
}
#endif /* TMWCNFG_SUPPORT_ASYNCH_DB */

/* function: mdnpo114_readObj114 */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpo114_readObj114(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{
  MDNPSESN *pMDNPSession = (MDNPSESN *)pSession;
  TMWTYPES_UCHAR  variation = pObjHeader->variation;
  TMWTYPES_USHORT pointNumber;
  TMWTYPES_USHORT strLen;
  TMWTYPES_USHORT index;
  TMWTYPES_UCHAR  flags = DNPDEFS_DBAS_FLAG_ON_LINE;
  
  /* Ensure the variation is supported & valid */
  if(_isVariationSupported(variation) == TMWDEFS_FALSE)
  {
    MDNPDIAG_ERROR(pSession->pChannel, pSession, MDNPDIAG_INV_VARIATION);
    return(TMWDEFS_FALSE);
  }

  for(index = 0; index < pObjHeader->numberOfPoints; index++)
  {
    dnputil_getPointNumber(pRxFragment, pObjHeader, index, &pointNumber);

    /* Retrieve the flags if variation is 3 or 4 */
    if (variation >= 3)
    {
      flags = pRxFragment->pMsgBuf[pRxFragment->offset++];
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

    DNPDIAG_SHOW_EXT_STRING_DATA(pSession,
      pointNumber, (pRxFragment->pMsgBuf + pRxFragment->offset), strLen, flags, TMWDIAG_ID_RX);

    _storeExtString(pMDNPSession->pDbHandle,
      pointNumber, pRxFragment->pMsgBuf + pRxFragment->offset, strLen, flags);

    pRxFragment->offset = pRxFragment->offset + strLen;
  }

  return(TMWDEFS_TRUE);
}
#endif /* MDNPDATA_SUPPORT_OBJ114 */

