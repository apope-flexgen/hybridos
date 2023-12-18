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

/* file: mdnpo040.c
 * description: DNP Master functionality for Object 40 Analog Output Status
 */
#include "tmwscl/dnp/dnpdiag.h"
#include "tmwscl/dnp/mdnpo040.h"
#include "tmwscl/dnp/mdnpdata.h"

#include "tmwscl/utils/tmwdb.h"
#include "tmwscl/utils/tmwtarg.h"

#if MDNPDATA_SUPPORT_OBJ40
#if TMWCNFG_SUPPORT_ASYNCH_DB
typedef struct MDNPO040DataStruct {
  TMWDB_DATA tmw;
  TMWTYPES_UCHAR flags;
  TMWTYPES_ANALOG_VALUE value;
  TMWTYPES_USHORT pointNumber;
} MDNPO040_DATA;

/* function: _dbStoreFunc */
static TMWTYPES_BOOL TMWDEFS_LOCAL _dbStoreFunc(
  TMWDB_DATA *pData)
{
  MDNPO040_DATA *pDNPData = (MDNPO040_DATA *)pData;

  mdnpdata_storeAnalogOutput(pData->pDbHandle, 
    pDNPData->pointNumber, &pDNPData->value, pDNPData->flags, TMWDEFS_FALSE, TMWDEFS_NULL);

  return(TMWDEFS_TRUE);
}

/* function: _storeAnalogOutput */
static void TMWDEFS_LOCAL _storeAnalogOutput(
  void *pDbHandle, 
  TMWTYPES_USHORT pointNumber, 
  TMWTYPES_ANALOG_VALUE *pValue,
  TMWTYPES_UCHAR flags)
{
  /* Allocate new binary data structure */
  MDNPO040_DATA *pDNPData = (MDNPO040_DATA *)tmwtarg_alloc(sizeof(MDNPO040_DATA));
  if (pDNPData == TMWDEFS_NULL)
  {
    return;
  }

  /* Initialize data indepdendent parts */
  tmwdb_initData((TMWDB_DATA *)pDNPData, pDbHandle, _dbStoreFunc);

  /* Initialize data dependent parts */
  pDNPData->flags = flags;
  pDNPData->value = *pValue;
  pDNPData->pointNumber = pointNumber;

  /* Store in database queue */
  if(!tmwdb_addEntry((TMWDB_DATA *)pDNPData))
    tmwtarg_free(pDNPData);
}
#else
/* function: _storeAnalogOutput */
static void TMWDEFS_LOCAL _storeAnalogOutput(
  void *pDbHandle, 
  TMWTYPES_USHORT pointNumber, 
  TMWTYPES_ANALOG_VALUE *pValue,
  TMWTYPES_UCHAR flags)
{
  mdnpdata_storeAnalogOutput(pDbHandle, pointNumber, pValue, flags, TMWDEFS_FALSE, TMWDEFS_NULL);
}
#endif /* TMWCNFG_SUPPORT_ASYNCH_DB */
#endif /* MDNPDATA_SUPPORT_OBJ40 */

#if MDNPDATA_SUPPORT_OBJ40_V1
/* function: mdnpo040_readObj40v1 */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpo040_readObj40v1(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{
  MDNPSESN *pMDNPSession = (MDNPSESN *)pSession;
  TMWTYPES_ANALOG_VALUE anlgValue;
  TMWTYPES_USHORT pointNumber;
  TMWTYPES_LONG value;
  TMWTYPES_UCHAR flags;
  TMWTYPES_USHORT index;

  for(index = 0; index < pObjHeader->numberOfPoints; index++)
  {
    dnputil_getPointNumber(pRxFragment, pObjHeader, index, &pointNumber);

    flags = pRxFragment->pMsgBuf[pRxFragment->offset++];

    tmwtarg_get32(&pRxFragment->pMsgBuf[pRxFragment->offset], (TMWTYPES_ULONG *)&value);
    pRxFragment->offset += 4;

    anlgValue.type = TMWTYPES_ANALOG_TYPE_LONG;
    anlgValue.value.lval = value;

    DNPDIAG_SHOW_ANALOG_OUTPUT(pSession, pointNumber, &anlgValue, flags, TMWDEFS_FALSE, TMWDEFS_NULL);
    _storeAnalogOutput(pMDNPSession->pDbHandle, pointNumber, &anlgValue, flags);
  }

  return(TMWDEFS_TRUE);
}
#endif /* MDNPDATA_SUPPORT_OBJ40_V1 */

#if MDNPDATA_SUPPORT_OBJ40_V2
/* function: mdnpo040_readObj40v2 */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpo040_readObj40v2(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{
  MDNPSESN *pMDNPSession = (MDNPSESN *)pSession;
  TMWTYPES_ANALOG_VALUE anlgValue;
  TMWTYPES_USHORT pointNumber;
  TMWTYPES_SHORT value;
  TMWTYPES_UCHAR flags;
  TMWTYPES_USHORT index;

  for(index = 0; index < pObjHeader->numberOfPoints; index++)
  {
    dnputil_getPointNumber(pRxFragment, pObjHeader, index, &pointNumber);

    flags = pRxFragment->pMsgBuf[pRxFragment->offset++];

    tmwtarg_get16(&pRxFragment->pMsgBuf[pRxFragment->offset], (TMWTYPES_USHORT *)&value);
    pRxFragment->offset += 2;

    anlgValue.type = TMWTYPES_ANALOG_TYPE_SHORT;
    anlgValue.value.sval = value;

    DNPDIAG_SHOW_ANALOG_OUTPUT(pSession, pointNumber, &anlgValue, flags, TMWDEFS_FALSE, TMWDEFS_NULL);
    _storeAnalogOutput(pMDNPSession->pDbHandle, pointNumber, &anlgValue, flags);
  }

  return(TMWDEFS_TRUE);
}
#endif /* MDNPDATA_SUPPORT_OBJ40_V2 */

#if MDNPDATA_SUPPORT_OBJ40_V3
/* function: mdnpo040_readObj40v3 */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpo040_readObj40v3(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{
  MDNPSESN *pMDNPSession = (MDNPSESN *)pSession;
  TMWTYPES_ANALOG_VALUE anlgValue;
  TMWTYPES_USHORT pointNumber;
  TMWTYPES_SFLOAT value;
  TMWTYPES_UCHAR flags;
  TMWTYPES_USHORT index;

  for(index = 0; index < pObjHeader->numberOfPoints; index++)
  {
    dnputil_getPointNumber(pRxFragment, pObjHeader, index, &pointNumber);

    flags = pRxFragment->pMsgBuf[pRxFragment->offset++];

    tmwtarg_getSFloat(&pRxFragment->pMsgBuf[pRxFragment->offset], &value);
    pRxFragment->offset += 4;

    anlgValue.type = TMWTYPES_ANALOG_TYPE_SFLOAT;
    anlgValue.value.fval = value;

    DNPDIAG_SHOW_ANALOG_OUTPUT(pSession, pointNumber, &anlgValue, flags, TMWDEFS_FALSE, TMWDEFS_NULL);
    _storeAnalogOutput(pMDNPSession->pDbHandle, pointNumber, &anlgValue, flags);
  }

  return(TMWDEFS_TRUE);
}
#endif /* MDNPDATA_SUPPORT_OBJ40_V3 */

#if MDNPDATA_SUPPORT_OBJ40_V4
/* function: mdnpo040_readObj40v4 */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpo040_readObj40v4(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{
  MDNPSESN *pMDNPSession = (MDNPSESN *)pSession;
  TMWTYPES_ANALOG_VALUE anlgValue;
  TMWTYPES_USHORT pointNumber;
  TMWTYPES_DOUBLE value;
  TMWTYPES_UCHAR flags;
  TMWTYPES_USHORT index;

  for(index = 0; index < pObjHeader->numberOfPoints; index++)
  {
    dnputil_getPointNumber(pRxFragment, pObjHeader, index, &pointNumber);

    flags = pRxFragment->pMsgBuf[pRxFragment->offset++];

    tmwtarg_get64(&pRxFragment->pMsgBuf[pRxFragment->offset], &value);
    pRxFragment->offset += 8;

    anlgValue.type = TMWTYPES_ANALOG_TYPE_DOUBLE;
    anlgValue.value.dval = value;

    DNPDIAG_SHOW_ANALOG_OUTPUT(pSession, pointNumber, &anlgValue, flags, TMWDEFS_FALSE, TMWDEFS_NULL);
    _storeAnalogOutput(pMDNPSession->pDbHandle, pointNumber, &anlgValue, flags);
  }

  return(TMWDEFS_TRUE);
}
#endif /* MDNPDATA_SUPPORT_OBJ40_V4 */


#if MDNPDATA_SUPPORT_OBJ40
/* function: mdnpo040_readObj40 */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpo040_readObj40(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{
  TMWTYPES_BOOL status = TMWDEFS_FALSE;
  switch(pObjHeader->variation)
  {
#if MDNPDATA_SUPPORT_OBJ40_V1
    case 1:
      status = mdnpo040_readObj40v1(pSession, pRxFragment, pObjHeader);
      break;
#endif
#if MDNPDATA_SUPPORT_OBJ40_V2
    case 2:
      status = mdnpo040_readObj40v2(pSession, pRxFragment, pObjHeader);
      break;
#endif
#if MDNPDATA_SUPPORT_OBJ40_V3
    case 3:
      status = mdnpo040_readObj40v3(pSession, pRxFragment, pObjHeader);
      break;
#endif
#if MDNPDATA_SUPPORT_OBJ40_V4
    case 4:
      status = mdnpo040_readObj40v4(pSession, pRxFragment, pObjHeader);
      break;
#endif
  }
  return(status);
}
#endif
