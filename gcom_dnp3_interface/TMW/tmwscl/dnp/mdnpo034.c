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

/* file: mdnpo034.c
 * description: DNP Master functionality for Object 34 Analog Input Deadbands
 */
#include "tmwscl/dnp/dnpdiag.h"
#include "tmwscl/dnp/mdnpo034.h"
#include "tmwscl/dnp/mdnpdata.h"

#include "tmwscl/utils/tmwdb.h"
#include "tmwscl/utils/tmwtarg.h"

#if MDNPDATA_SUPPORT_OBJ34
#if TMWCNFG_SUPPORT_ASYNCH_DB
typedef struct MDNPO034DataStruct {
  TMWDB_DATA tmw;
  TMWTYPES_ANALOG_VALUE value;
  TMWTYPES_USHORT pointNumber;
} MDNPO034_DATA;

/* function: _dbStoreFunc */
static TMWTYPES_BOOL TMWDEFS_LOCAL _dbStoreFunc(
  TMWDB_DATA *pData)
{
  MDNPO034_DATA *pDNPData = (MDNPO034_DATA *)pData;

  mdnpdata_storeAnalogInputDeadband(pData->pDbHandle, 
    pDNPData->pointNumber, &pDNPData->value);

  return(TMWDEFS_TRUE);
}

/* function: _storeAnalogInputDeadband */
static void TMWDEFS_LOCAL _storeAnalogInputDeadband(
  void *pDbHandle, 
  TMWTYPES_USHORT pointNumber, 
  TMWTYPES_ANALOG_VALUE *pValue)
{
  /* Allocate new binary data structure */
  MDNPO034_DATA *pDNPData = (MDNPO034_DATA *)tmwtarg_alloc(sizeof(MDNPO034_DATA));
  if (pDNPData == TMWDEFS_NULL)
  {
    return;
  }

  /* Initialize data indepdendent parts */
  tmwdb_initData((TMWDB_DATA *)pDNPData, pDbHandle, _dbStoreFunc);

  /* Initialize data dependent parts */
  pDNPData->value = *pValue;
  pDNPData->pointNumber = pointNumber;

  /* Store in database queue */
  if(!tmwdb_addEntry((TMWDB_DATA *)pDNPData))
    tmwtarg_free(pDNPData);
}
#else
/* function: _storeAnalogInputDeadband */
static void TMWDEFS_LOCAL _storeAnalogInputDeadband(
  void *pDbHandle, 
  TMWTYPES_USHORT pointNumber, 
  TMWTYPES_ANALOG_VALUE *pValue)
{
  mdnpdata_storeAnalogInputDeadband(pDbHandle, pointNumber, pValue);
}
#endif /* TMWCNFG_SUPPORT_ASYNCH_DB */
#endif /* MDNPDATA_SUPPORT_OBJ34 */

#if MDNPDATA_SUPPORT_OBJ34_V1
/* function: mdnpo034_readObj34v1 */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpo034_readObj34v1(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{
  MDNPSESN *pMDNPSession = (MDNPSESN *)pSession;
  TMWTYPES_ANALOG_VALUE anlgValue;
  TMWTYPES_USHORT pointNumber;
  TMWTYPES_USHORT value;
  TMWTYPES_USHORT index;

  for(index = 0; index < pObjHeader->numberOfPoints; index++)
  {
    dnputil_getPointNumber(pRxFragment, pObjHeader, index, &pointNumber);

    tmwtarg_get16(&pRxFragment->pMsgBuf[pRxFragment->offset], (TMWTYPES_USHORT *)&value);
    pRxFragment->offset += 2;

    anlgValue.type = TMWTYPES_ANALOG_TYPE_USHORT;
    anlgValue.value.usval = value;

    DNPDIAG_SHOW_ANALOG_DEADBAND(pSession, pointNumber, &anlgValue, TMWDIAG_ID_RX);
    _storeAnalogInputDeadband(pMDNPSession->pDbHandle, pointNumber, &anlgValue);
  }

  return(TMWDEFS_TRUE);
}
#endif /* MDNPDATA_SUPPORT_OBJ34_V1 */

#if MDNPDATA_SUPPORT_OBJ34_V2
/* function: mdnpo034_readObj34v2 */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpo034_readObj34v2(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{
  MDNPSESN *pMDNPSession = (MDNPSESN *)pSession;
  TMWTYPES_ANALOG_VALUE anlgValue;
  TMWTYPES_USHORT pointNumber;
  TMWTYPES_ULONG value;
  TMWTYPES_USHORT index;

  for(index = 0; index < pObjHeader->numberOfPoints; index++)
  {
    dnputil_getPointNumber(pRxFragment, pObjHeader, index, &pointNumber);

    tmwtarg_get32(&pRxFragment->pMsgBuf[pRxFragment->offset], (TMWTYPES_ULONG *)&value);
    pRxFragment->offset += 4;

    anlgValue.type = TMWTYPES_ANALOG_TYPE_ULONG;
    anlgValue.value.lval = value;

    DNPDIAG_SHOW_ANALOG_DEADBAND(pSession, pointNumber, &anlgValue, TMWDIAG_ID_RX);
    _storeAnalogInputDeadband(pMDNPSession->pDbHandle, pointNumber, &anlgValue);
  }

  return(TMWDEFS_TRUE);
}
#endif /* MDNPDATA_SUPPORT_OBJ34_V2 */

#if MDNPDATA_SUPPORT_OBJ34_V3
/* function: mdnpo034_readObj34v3 */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpo034_readObj34v3(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{
  MDNPSESN *pMDNPSession = (MDNPSESN *)pSession;
  TMWTYPES_ANALOG_VALUE anlgValue;
  TMWTYPES_USHORT pointNumber;
  TMWTYPES_SFLOAT value;
  TMWTYPES_USHORT index;

  for(index = 0; index < pObjHeader->numberOfPoints; index++)
  {
    dnputil_getPointNumber(pRxFragment, pObjHeader, index, &pointNumber);

    tmwtarg_getSFloat(&pRxFragment->pMsgBuf[pRxFragment->offset], &value);
    pRxFragment->offset += 4;

    anlgValue.type = TMWTYPES_ANALOG_TYPE_SFLOAT;
    anlgValue.value.fval = value;

    DNPDIAG_SHOW_ANALOG_DEADBAND(pSession, pointNumber, &anlgValue, TMWDIAG_ID_RX);
    _storeAnalogInputDeadband(pMDNPSession->pDbHandle, pointNumber, &anlgValue);
  }

  return(TMWDEFS_TRUE);
}
#endif /* MDNPDATA_SUPPORT_OBJ34_V3 */
