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

/* file: mdnpo110.c
 * description: DNP Master functionality for Object 110 String Data
 */
#include "tmwscl/dnp/dnpdiag.h"
#include "tmwscl/dnp/mdnpdiag.h"
#include "tmwscl/dnp/mdnpo110.h"
#include "tmwscl/dnp/mdnpdata.h"

#include "tmwscl/utils/tmwdb.h"
#include "tmwscl/utils/tmwtarg.h"

#if MDNPDATA_SUPPORT_OBJ110

#if TMWCNFG_SUPPORT_ASYNCH_DB
typedef struct MDNPO110DataStruct {
  TMWDB_DATA tmw;
  TMWTYPES_UCHAR buf[DNPDEFS_MAX_STRING_LENGTH];
  TMWTYPES_UCHAR buflen;
  TMWTYPES_USHORT pointNumber;
} MDNPO110_DATA;

/* function: _dbStoreFunc */
static TMWTYPES_BOOL TMWDEFS_LOCAL _dbStoreFunc(
  TMWDB_DATA *pData)
{
  MDNPO110_DATA *pDNPData = (MDNPO110_DATA *)pData;

  mdnpdata_storeString(pData->pDbHandle, 
    pDNPData->pointNumber, pDNPData->buf, pDNPData->buflen);

  return(TMWDEFS_TRUE);
}

/* function: _storeString */
static void TMWDEFS_LOCAL _storeString(
  void *pDbHandle, 
  TMWTYPES_USHORT pointNumber, 
  TMWTYPES_UCHAR *pBuf,
  TMWTYPES_UCHAR buflen)
{
  /* Allocate new binary data structure */
  MDNPO110_DATA *pDNPData = (MDNPO110_DATA *)tmwtarg_alloc(sizeof(MDNPO110_DATA));
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

  /* Store in database queue */
  if(!tmwdb_addEntry((TMWDB_DATA *)pDNPData))
    tmwtarg_free(pDNPData);
}
#else
/* function: _storeString */
static void TMWDEFS_LOCAL _storeString(
  void *pDbHandle, 
  TMWTYPES_USHORT pointNumber, 
  TMWTYPES_UCHAR *pBuf,
  TMWTYPES_UCHAR buflen)
{
  mdnpdata_storeString(pDbHandle, pointNumber, pBuf, buflen);
}
#endif /* TMWCNFG_SUPPORT_ASYNCH_DB */

/* function: mdnpo110_readObj110 */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpo110_readObj110(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{
  MDNPSESN *pMDNPSession = (MDNPSESN *)pSession;
  TMWTYPES_USHORT pointNumber;
  TMWTYPES_UCHAR strLen;
  TMWTYPES_USHORT index;
  
  /* zero length strings not allowed in response */
  if(pObjHeader->variation == 0)
  {
    MDNPDIAG_ERROR(pSession->pChannel, pSession, MDNPDIAG_INV_VARIATION);
    return(TMWDEFS_FALSE);
  }

  strLen = pObjHeader->variation;
  for(index = 0; index < pObjHeader->numberOfPoints; index++)
  {
    dnputil_getPointNumber(pRxFragment, pObjHeader, index, &pointNumber);

    /* Protect against badly formatted message */
    if((pRxFragment->offset + strLen) > pRxFragment->msgLength)
    { 
      DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_INVALID_SIZE);
      return(TMWDEFS_FALSE);
    }

    DNPDIAG_SHOW_STRING_DATA(pSession,
      pointNumber, (pRxFragment->pMsgBuf + pRxFragment->offset), strLen, TMWDIAG_ID_RX);

    _storeString(pMDNPSession->pDbHandle,
      pointNumber, pRxFragment->pMsgBuf + pRxFragment->offset, strLen);

    pRxFragment->offset = pRxFragment->offset + strLen;
  }

  return(TMWDEFS_TRUE);
}
#endif /* MDNPDATA_SUPPORT_OBJ110 */

