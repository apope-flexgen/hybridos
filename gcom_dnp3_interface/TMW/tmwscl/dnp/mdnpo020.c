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

/* file: mdnpo020.c
 * description: DNP Master functionality for Object 20 Binary Counters
 */
#include "tmwscl/dnp/dnpdiag.h"
#include "tmwscl/dnp/mdnpo020.h"
#include "tmwscl/dnp/mdnpdata.h"

#include "tmwscl/utils/tmwdb.h"
#include "tmwscl/utils/tmwtarg.h"

#if MDNPDATA_SUPPORT_OBJ20
#if TMWCNFG_SUPPORT_ASYNCH_DB

/* Structure used to store data for asynch database updates */
typedef struct MDNPO020DataStruct {
  TMWDB_DATA tmw;
  TMWTYPES_ULONG value;
  TMWTYPES_UCHAR flags;
  TMWTYPES_USHORT pointNumber;
} MDNPO020_DATA;

/* function: _dbStoreFunc */
static TMWTYPES_BOOL TMWDEFS_LOCAL _dbStoreFunc(
  TMWDB_DATA *pData)
{
  MDNPO020_DATA *pDNPData = (MDNPO020_DATA *)pData;

  mdnpdata_storeBinaryCounter(pData->pDbHandle, 
    pDNPData->pointNumber, pDNPData->value, pDNPData->flags, 
    TMWDEFS_FALSE, TMWDEFS_NULL);

  return(TMWDEFS_TRUE);
}

/* function: _storeBinaryCounter */
static void TMWDEFS_LOCAL _storeBinaryCounter(
  void *pDbHandle, 
  TMWTYPES_USHORT pointNumber,
  TMWTYPES_ULONG value,
  TMWTYPES_UCHAR flags)
{
  /* Allocate new binary data structure */
  MDNPO020_DATA *pDNPData = (MDNPO020_DATA *)tmwtarg_alloc(sizeof(MDNPO020_DATA));
  if (pDNPData == TMWDEFS_NULL)
  {
    return;
  }

  /* Initialize data indepdendent parts */
  tmwdb_initData((TMWDB_DATA *)pDNPData, pDbHandle, _dbStoreFunc);

  /* Initialize data dependent parts */
  pDNPData->value = value;
  pDNPData->flags = flags;
  pDNPData->pointNumber = pointNumber;

  /* Store in database queue */
  if(!tmwdb_addEntry((TMWDB_DATA *)pDNPData))
    tmwtarg_free(pDNPData);
}
#else
/* function: _storeBinaryCounter */
static void TMWDEFS_LOCAL _storeBinaryCounter(
  void *pDbHandle, 
  TMWTYPES_USHORT pointNumber, 
  TMWTYPES_ULONG value,
  TMWTYPES_UCHAR flags)
{
  mdnpdata_storeBinaryCounter(pDbHandle, pointNumber, 
    value, flags, TMWDEFS_FALSE, TMWDEFS_NULL);
}
#endif /* TMWCNFG_SUPPORT_ASYNCH_DB */
#endif /* MDNPDATA_SUPPORT_OBJ20 */

#if MDNPDATA_SUPPORT_OBJ20_V1
/* function: mdnpo020_readObj20v1 */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpo020_readObj20v1(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{
  MDNPSESN *pMDNPSession = (MDNPSESN *)pSession;
  TMWTYPES_USHORT pointNumber;
  TMWTYPES_USHORT index;
  TMWTYPES_ULONG value;
  TMWTYPES_UCHAR flags;

  for(index = 0; index < pObjHeader->numberOfPoints; index++)
  {
    dnputil_getPointNumber(pRxFragment, pObjHeader, index, &pointNumber);

    flags = pRxFragment->pMsgBuf[pRxFragment->offset++];

    tmwtarg_get32(&pRxFragment->pMsgBuf[pRxFragment->offset], &value);
    pRxFragment->offset += 4;

    DNPDIAG_SHOW_BINARY_COUNTER(pSession, pointNumber, value, flags, TMWDEFS_FALSE, TMWDEFS_NULL);

    _storeBinaryCounter(pMDNPSession->pDbHandle, pointNumber, value, flags);
  }

  return(TMWDEFS_TRUE);
}
#endif /* MDNPDATA_SUPPORT_OBJ20_V1 */

#if MDNPDATA_SUPPORT_OBJ20_V2
/* function: mdnpo020_readObj20v2 */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpo020_readObj20v2(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{
  MDNPSESN *pMDNPSession = (MDNPSESN *)pSession;
  TMWTYPES_USHORT pointNumber;
  TMWTYPES_USHORT index;
  TMWTYPES_USHORT value;
  TMWTYPES_UCHAR flags;

  for(index = 0; index < pObjHeader->numberOfPoints; index++)
  {
    dnputil_getPointNumber(pRxFragment, pObjHeader, index, &pointNumber);

    flags = pRxFragment->pMsgBuf[pRxFragment->offset++];

    tmwtarg_get16(&pRxFragment->pMsgBuf[pRxFragment->offset], &value);
    pRxFragment->offset += 2;

    DNPDIAG_SHOW_BINARY_COUNTER(pSession, pointNumber, value, flags, TMWDEFS_FALSE, TMWDEFS_NULL);

    _storeBinaryCounter(pMDNPSession->pDbHandle, pointNumber, value, flags);
  }

  return(TMWDEFS_TRUE);
}
#endif /* MDNPDATA_SUPPORT_OBJ20_V2 */

#if MDNPDATA_SUPPORT_OBJ20_V5
/* function: mdnpo020_readObj20v5 */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpo020_readObj20v5(
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

    DNPDIAG_SHOW_BINARY_COUNTER(pSession, pointNumber, value, 0x01, TMWDEFS_FALSE, TMWDEFS_NULL);

    _storeBinaryCounter(pMDNPSession->pDbHandle, pointNumber, value, 0x01);
  }

  return(TMWDEFS_TRUE);
}
#endif /* MDNPDATA_SUPPORT_OBJ20_V5 */

#if MDNPDATA_SUPPORT_OBJ20_V6
/* function: mdnpo020_readObj20v6 */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpo020_readObj20v6(
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

    DNPDIAG_SHOW_BINARY_COUNTER(pSession, pointNumber, value, 0x01, TMWDEFS_FALSE, TMWDEFS_NULL);

    _storeBinaryCounter(pMDNPSession->pDbHandle, pointNumber, value, 0x01);
  }

  return(TMWDEFS_TRUE);
}
#endif /* MDNPDATA_SUPPORT_OBJ20_V6 */

#if MDNPDATA_SUPPORT_OBJ20
/* function: mdnpo020_readObj20 */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpo020_readObj20(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{
  TMWTYPES_BOOL status = TMWDEFS_FALSE;
  switch(pObjHeader->variation)
  {
#if MDNPDATA_SUPPORT_OBJ20_V1
    case 1:
      status = mdnpo020_readObj20v1(pSession, pRxFragment, pObjHeader);
      break;
#endif
#if MDNPDATA_SUPPORT_OBJ20_V2
    case 2:
      status = mdnpo020_readObj20v2(pSession, pRxFragment, pObjHeader);
      break;
#endif
#if MDNPDATA_SUPPORT_OBJ20_V5
    case 5:
      status = mdnpo020_readObj20v5(pSession, pRxFragment, pObjHeader);
      break;
#endif
#if MDNPDATA_SUPPORT_OBJ20_V6
    case 6:
      status = mdnpo020_readObj20v6(pSession, pRxFragment, pObjHeader);
      break;
#endif
  }
  return(status);
}
#endif

