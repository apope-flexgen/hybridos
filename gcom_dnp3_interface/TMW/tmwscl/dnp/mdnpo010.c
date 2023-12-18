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

/* file: mdnpo010.c
 * description: DNP Master functionality for Object 10 Binary Output Status
 */
#include "tmwscl/dnp/dnpdiag.h"
#include "tmwscl/dnp/mdnpo010.h"
#include "tmwscl/dnp/mdnpdata.h"

#include "tmwscl/utils/tmwdb.h"
#include "tmwscl/utils/tmwtarg.h"

#if MDNPDATA_SUPPORT_OBJ10
#if TMWCNFG_SUPPORT_ASYNCH_DB

/* Structure used to store data for asynch database updates */
typedef struct MDNPO010DataStruct {
  TMWDB_DATA tmw;
  TMWTYPES_UCHAR flags;
  TMWTYPES_USHORT pointNumber;
} MDNPO010_DATA;

/* function: _dbStoreFunc */
static TMWTYPES_BOOL TMWDEFS_LOCAL _dbStoreFunc(
  TMWDB_DATA *pData)
{
  MDNPO010_DATA *pDNPData = (MDNPO010_DATA *)pData;

  mdnpdata_storeBinaryOutput(pData->pDbHandle, 
    pDNPData->pointNumber, pDNPData->flags, TMWDEFS_FALSE, TMWDEFS_NULL);

  return(TMWDEFS_TRUE);
}

/* function: _storeBinaryOutput */
static void TMWDEFS_LOCAL _storeBinaryOutput(
  void *pDbHandle, 
  TMWTYPES_USHORT pointNumber, 
  TMWTYPES_UCHAR flags)
{
  /* Allocate new binary data structure */
  MDNPO010_DATA *pDNPData = (MDNPO010_DATA *)tmwtarg_alloc(sizeof(MDNPO010_DATA));
  if (pDNPData == TMWDEFS_NULL)
  {
    return;
  }

  /* Initialize data indepdendent parts */
  tmwdb_initData((TMWDB_DATA *)pDNPData, pDbHandle, _dbStoreFunc);

  /* Initialize data dependent parts */
  pDNPData->flags = flags;
  pDNPData->pointNumber = pointNumber;

  /* Store in database queue */
  if(!tmwdb_addEntry((TMWDB_DATA *)pDNPData))
    tmwtarg_free(pDNPData);
}
#else
/* function: _storeBinaryOutput */
static void TMWDEFS_LOCAL _storeBinaryOutput(
  void *pDbHandle, 
  TMWTYPES_USHORT pointNumber, 
  TMWTYPES_UCHAR flags)
{
  mdnpdata_storeBinaryOutput(pDbHandle, pointNumber, flags, TMWDEFS_FALSE, TMWDEFS_NULL);
}
#endif /* TMWCNFG_SUPPORT_ASYNCH_DB */
#endif /* MDNPDATA_SUPPORT_OBJ10 */

#if MDNPDATA_SUPPORT_OBJ10_V1
/* function: mdnpo010_readObj10v1 */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpo010_readObj10v1(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{
  MDNPSESN *pMDNPSession = (MDNPSESN *)pSession;
  TMWTYPES_USHORT pointNumber;
  TMWTYPES_USHORT index;
  TMWTYPES_UCHAR shift;
  TMWTYPES_UCHAR value;

  index = 0;
  shift = 0;
  while(index < pObjHeader->numberOfPoints)
  { 
    dnputil_getPointNumber(pRxFragment, pObjHeader, index, &pointNumber);
    
    /* Protect against badly formatted message */
    if(pRxFragment->offset >= pRxFragment->msgLength)
    {
      DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_INVALID_SIZE);
      return(TMWDEFS_FALSE);
    }

    if((pRxFragment->pMsgBuf[pRxFragment->offset] & (0x01 << shift)) != 0)
      value = 0x81;
    else
      value = 0x01;

    DNPDIAG_SHOW_BINARY_OUTPUT(pSession, pointNumber, value, TMWDEFS_FALSE, TMWDEFS_NULL);

    _storeBinaryOutput(pMDNPSession->pDbHandle, pointNumber, value);

    index = (TMWTYPES_USHORT)(index + 1);

    if((pObjHeader->qualifier == DNPDEFS_QUAL_8BIT_INDEX)
      || (pObjHeader->qualifier == DNPDEFS_QUAL_16BIT_INDEX))
    {
      pRxFragment->offset += 1;
    }
    else
    {
      shift = (TMWTYPES_UCHAR)(shift + 1);
      if(shift > 7)
      {
        shift = 0;
        pRxFragment->offset += 1;
      }
    }
  }

  if((pObjHeader->qualifier != DNPDEFS_QUAL_8BIT_INDEX)
    && (pObjHeader->qualifier != DNPDEFS_QUAL_16BIT_INDEX))
  {
    if(shift != 0)
      pRxFragment->offset += 1;
  }

  return(TMWDEFS_TRUE);
}
#endif /* MDNPDATA_SUPPORT_OBJ10_V1 */

#if MDNPDATA_SUPPORT_OBJ10_V2
/* function: mdnpo010_readObj10v2 */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpo010_readObj10v2(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{
  MDNPSESN *pMDNPSession = (MDNPSESN *)pSession;
  TMWTYPES_USHORT pointNumber;
  TMWTYPES_USHORT index;
  TMWTYPES_UCHAR flags;

  for(index = 0; index < pObjHeader->numberOfPoints; index++)
  {
    dnputil_getPointNumber(pRxFragment, pObjHeader, index, &pointNumber);

    flags = pRxFragment->pMsgBuf[pRxFragment->offset++];

    DNPDIAG_SHOW_BINARY_OUTPUT(pSession, pointNumber, flags, TMWDEFS_FALSE, TMWDEFS_NULL);

    _storeBinaryOutput(pMDNPSession->pDbHandle, pointNumber, flags);
  }

  return(TMWDEFS_TRUE);
}
#endif /* MDNPDATA_SUPPORT_OBJ10_V2 */

