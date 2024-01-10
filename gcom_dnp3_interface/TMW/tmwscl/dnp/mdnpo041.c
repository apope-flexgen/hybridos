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

/* file: mdnpo041.c
 * description: DNP Master functionality for Object 41 Analog Output Block
 */
#include "tmwscl/dnp/dnpdiag.h"
#include "tmwscl/dnp/mdnpdiag.h"
#include "tmwscl/dnp/dnpchnl.h"
#include "tmwscl/dnp/mdnpo041.h"
#include "tmwscl/dnp/mdnpdata.h"
#include "tmwscl/utils/tmwdb.h"

#if MDNPDATA_SUPPORT_OBJ41
#if TMWCNFG_SUPPORT_ASYNCH_DB
typedef struct MDNPO041DataStruct {
  TMWDB_DATA tmw;
  TMWTYPES_UCHAR status;
  TMWTYPES_ANALOG_VALUE value;
  TMWTYPES_USHORT pointNumber;
  TMWTYPES_BOOL timeSpecified;
  TMWDTIME timeStamp;
} MDNPO041_DATA;

/* function: _dbStoreFunc */
static TMWTYPES_BOOL TMWDEFS_LOCAL _dbStoreFunc(
  TMWDB_DATA *pData)
{
  MDNPO041_DATA *pDNPData = (MDNPO041_DATA *)pData;
  TMWDTIME *pTimeStamp = (pDNPData->timeSpecified) ? &pDNPData->timeStamp : TMWDEFS_NULL;

  mdnpdata_storeAnalogCmdStatus(pData->pDbHandle, 
    pDNPData->pointNumber, &pDNPData->value, pDNPData->status, 
    TMWDEFS_FALSE, pTimeStamp);

  return(TMWDEFS_TRUE);
}

/* function: _storeAnalogOutputCmdStatus */
static void TMWDEFS_LOCAL _storeAnalogOutputCmdStatus(
  void *pDbHandle, 
  TMWTYPES_USHORT pointNumber, 
  TMWTYPES_ANALOG_VALUE *pValue,
  TMWTYPES_UCHAR status,
  TMWDTIME *pTimeStamp)
{
  /* Allocate new binary data structure */
  MDNPO041_DATA *pDNPData = (MDNPO041_DATA *)tmwtarg_alloc(sizeof(MDNPO041_DATA));
  if (pDNPData == TMWDEFS_NULL)
  {
    return;
  }

  /* Initialize data independent parts */
  tmwdb_initData((TMWDB_DATA *)pDNPData, pDbHandle, _dbStoreFunc);

  /* Initialize data dependent parts */
  pDNPData->status = status;
  pDNPData->value = *pValue;
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
/* function: _storeAnalogOutputCmdStatus */
static void TMWDEFS_LOCAL _storeAnalogOutputCmdStatus(
  void *pDbHandle, 
  TMWTYPES_USHORT pointNumber, 
  TMWTYPES_ANALOG_VALUE *pValue,
  TMWTYPES_UCHAR status,
  TMWDTIME *pTimeStamp)
{
  mdnpdata_storeAnalogCmdStatus(pDbHandle, pointNumber, 
    pValue, status, TMWDEFS_FALSE, pTimeStamp);
}
#endif /* TMWCNFG_SUPPORT_ASYNCH_DB */
#endif

#if MDNPDATA_SUPPORT_OBJ41_V1
/* function: mdnpo041_selOperRespObj41v1 */
DNPCHNL_RESP_STATUS TMWDEFS_GLOBAL mdnpo041_selOperRespObj41v1(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{
  TMWTYPES_ANALOG_VALUE anlgValue;
  TMWTYPES_USHORT pointNumber;
  TMWTYPES_USHORT index;
  TMWTYPES_UCHAR  status;
  TMWTYPES_ULONG  value;
  MDNPSESN *pMDNPSession = (MDNPSESN *)pSession;
  DNPCHNL_RESP_STATUS retStatus = DNPCHNL_RESP_STATUS_SUCCESS;

  for(index = 0; index < pObjHeader->numberOfPoints; index++)
  {
    dnputil_getPointNumber(pRxFragment, pObjHeader, index, &pointNumber);

    tmwtarg_get32(&pRxFragment->pMsgBuf[pRxFragment->offset], &value);
    pRxFragment->offset += 4;
 
    /* Read Status */
    status = pRxFragment->pMsgBuf[pRxFragment->offset++];
 
    anlgValue.type = TMWTYPES_ANALOG_TYPE_LONG;
    anlgValue.value.lval = value;

    /* Diagnostics */
    DNPDIAG_SHOW_ANALOG_CONTROL(pSession, pointNumber, &anlgValue, status, TMWDEFS_FALSE, TMWDIAG_ID_RX, TMWDEFS_NULL);

    _storeAnalogOutputCmdStatus(pMDNPSession->pDbHandle, pointNumber, &anlgValue, status, TMWDEFS_NULL);

    if(status != 0)
    {   
      MDNPDIAG_ERROR(pSession->pChannel, pSession, MDNPDIAG_STATUS_NOT_ZERO);
      retStatus = DNPCHNL_RESP_STATUS_STATUSCODE;
    }
  }

  return(retStatus);
}
#endif /* MDNPDATA_SUPPORT_OBJ41_V1 */

#if MDNPDATA_SUPPORT_OBJ41_V2
/* function: mdnpo041_selOperRespObj41v2 */
DNPCHNL_RESP_STATUS TMWDEFS_GLOBAL mdnpo041_selOperRespObj41v2(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{
  TMWTYPES_ANALOG_VALUE anlgValue;
  TMWTYPES_USHORT pointNumber;
  TMWTYPES_USHORT index;
  TMWTYPES_UCHAR  status;
  TMWTYPES_USHORT value;
  MDNPSESN *pMDNPSession = (MDNPSESN *)pSession;
  DNPCHNL_RESP_STATUS retStatus = DNPCHNL_RESP_STATUS_SUCCESS;

  for(index = 0; index < pObjHeader->numberOfPoints; index++)
  {
    dnputil_getPointNumber(pRxFragment, pObjHeader, index, &pointNumber);

    tmwtarg_get16(&pRxFragment->pMsgBuf[pRxFragment->offset], &value);
    pRxFragment->offset += 2;
 
    /* Read Status */
    status = pRxFragment->pMsgBuf[pRxFragment->offset++];
 
    anlgValue.type = TMWTYPES_ANALOG_TYPE_SHORT;
    anlgValue.value.sval = value;

    /* Diagnostics */
    DNPDIAG_SHOW_ANALOG_CONTROL(pSession, pointNumber, &anlgValue, status, TMWDEFS_FALSE, TMWDIAG_ID_RX, TMWDEFS_NULL);

    _storeAnalogOutputCmdStatus(pMDNPSession->pDbHandle, pointNumber, &anlgValue, status, TMWDEFS_NULL);

    if(status != 0)
    {   
      MDNPDIAG_ERROR(pSession->pChannel, pSession, MDNPDIAG_STATUS_NOT_ZERO);
      retStatus = DNPCHNL_RESP_STATUS_STATUSCODE;
    }
  }

  return(retStatus);
}
#endif /* MDNPDATA_SUPPORT_OBJ41_V2 */

#if MDNPDATA_SUPPORT_OBJ41_V3
/* function: mdnpo041_selOperRespObj41v3 */
DNPCHNL_RESP_STATUS TMWDEFS_GLOBAL mdnpo041_selOperRespObj41v3(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{
  TMWTYPES_ANALOG_VALUE anlgValue;
  TMWTYPES_USHORT pointNumber;
  TMWTYPES_USHORT index;
  TMWTYPES_UCHAR  status;
  TMWTYPES_SFLOAT value;
  MDNPSESN *pMDNPSession = (MDNPSESN *)pSession;
  DNPCHNL_RESP_STATUS retStatus = DNPCHNL_RESP_STATUS_SUCCESS;

  for(index = 0; index < pObjHeader->numberOfPoints; index++)
  {
    dnputil_getPointNumber(pRxFragment, pObjHeader, index, &pointNumber);

    tmwtarg_getSFloat(&pRxFragment->pMsgBuf[pRxFragment->offset], &value);
    pRxFragment->offset += 4;
 
    /* Read Status */
    status = pRxFragment->pMsgBuf[pRxFragment->offset++];
 
    anlgValue.type = TMWTYPES_ANALOG_TYPE_SFLOAT;
    anlgValue.value.fval = value;

    /* Diagnostics */
    DNPDIAG_SHOW_ANALOG_CONTROL(pSession, pointNumber, &anlgValue, status, TMWDEFS_FALSE, TMWDIAG_ID_RX, TMWDEFS_NULL);

    _storeAnalogOutputCmdStatus(pMDNPSession->pDbHandle, pointNumber, &anlgValue, status, TMWDEFS_NULL);

    if(status != 0)
    {   
      MDNPDIAG_ERROR(pSession->pChannel, pSession, MDNPDIAG_STATUS_NOT_ZERO);
      retStatus = DNPCHNL_RESP_STATUS_STATUSCODE;
    }
  }

  return(retStatus);
}
#endif /* MDNPDATA_SUPPORT_OBJ41_V3 */

#if MDNPDATA_SUPPORT_OBJ41_V4
/* function: mdnpo041_selOperRespObj41v4 */
DNPCHNL_RESP_STATUS TMWDEFS_GLOBAL mdnpo041_selOperRespObj41v4(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{
  TMWTYPES_ANALOG_VALUE anlgValue;
  TMWTYPES_USHORT pointNumber;
  TMWTYPES_USHORT index;
  TMWTYPES_UCHAR  status;
  TMWTYPES_DOUBLE value;
  MDNPSESN *pMDNPSession = (MDNPSESN *)pSession;
  DNPCHNL_RESP_STATUS retStatus = DNPCHNL_RESP_STATUS_SUCCESS;

  for(index = 0; index < pObjHeader->numberOfPoints; index++)
  {
    dnputil_getPointNumber(pRxFragment, pObjHeader, index, &pointNumber);

    tmwtarg_get64(&pRxFragment->pMsgBuf[pRxFragment->offset], &value);
    pRxFragment->offset += 8;
 
    /* Read Status */
    status = pRxFragment->pMsgBuf[pRxFragment->offset++];
 
    anlgValue.type = TMWTYPES_ANALOG_TYPE_DOUBLE;
    anlgValue.value.dval = value;

    /* Diagnostics */
    DNPDIAG_SHOW_ANALOG_CONTROL(pSession, pointNumber, &anlgValue, status, TMWDEFS_FALSE, TMWDIAG_ID_RX, TMWDEFS_NULL);

    _storeAnalogOutputCmdStatus(pMDNPSession->pDbHandle, pointNumber, &anlgValue, status, TMWDEFS_NULL);

    if(status != 0)
    {   
      MDNPDIAG_ERROR(pSession->pChannel, pSession, MDNPDIAG_STATUS_NOT_ZERO);
      retStatus = DNPCHNL_RESP_STATUS_STATUSCODE;
    }
  }

  return(retStatus);
}
#endif /* MDNPDATA_SUPPORT_OBJ41_V4 */




