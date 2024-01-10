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

/* file: sdnpo034.c
 * description: DNP Slave functionality for Object 34 Analog Input Deadbands
 */
#include "tmwscl/dnp/dnpdiag.h"
#include "tmwscl/utils/tmwtarg.h"
#include "tmwscl/dnp/dnpdefs.h"
#include "tmwscl/dnp/sdnpo034.h"
#include "tmwscl/dnp/sdnpdata.h"
#include "tmwscl/dnp/sdnputil.h"
#include "tmwscl/dnp/dnputil.h"

#if SDNPDATA_SUPPORT_OBJ34

/* forward declarations */
static void TMWDEFS_GLOBAL _setVariationInfo(
  SDNPUTIL_STATIC_DESC *pDesc, 
  TMWTYPES_UCHAR variation);

/* function: _isSupportedVariation() */
static TMWTYPES_BOOL TMWDEFS_LOCAL _isSupportedVariation(
  TMWTYPES_UCHAR variation)
{
switch(variation)
  {
#if SDNPDATA_SUPPORT_OBJ34_V1
  case 1:
    break;
#endif
#if SDNPDATA_SUPPORT_OBJ34_V2
  case 2:
    break;
#endif
#if SDNPDATA_SUPPORT_OBJ34_V3
  case 3:
    break;
#endif
  default:
    return(TMWDEFS_FALSE);
  }

  return(TMWDEFS_TRUE);
}

#if SDNPDATA_SUPPORT_STATIC_VAR_POINT
/* function: _switchPointDefVariation */
static TMWTYPES_BOOL TMWDEFS_LOCAL _switchPointDefVariation(
  SDNPUTIL_STATIC_DESC *pDesc,
  void *pPoint,
  TMWTYPES_UCHAR currentVariation)                     
{
  /* If session default variation is zero, check the default variation
   * for this specific point 
   */
  if(pDesc->readVariation == 0)
  {
    TMWTYPES_UCHAR variation = sdnpdata_anlgInDbandDefVar(pPoint); 
    if((variation != currentVariation) 
      && (_isSupportedVariation(variation)))
    {
      _setVariationInfo(pDesc, variation);
      return(TMWDEFS_TRUE);
    }
  }
  return(TMWDEFS_FALSE);
}
#endif

#if SDNPDATA_SUPPORT_OBJ34_V1
/* function: _readv1 */
static SDNPSESN_READ_STATUS TMWDEFS_CALLBACK _readv1(
  TMWSESN *pSession,
  TMWSESN_TX_DATA *pResponse,
  TMWTYPES_USHORT messageIndex,
  TMWTYPES_USHORT pointNum,
  void *pPoint,
  SDNPUTIL_STATIC_DESC *pDesc)
{
  TMWTYPES_ANALOG_VALUE value;
  TMWTYPES_USHORT usvalue;

  TMWTARG_UNUSED_PARAM(messageIndex);

#if SDNPDATA_SUPPORT_STATIC_VAR_POINT
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  /* If session default variation is zero, check the default variation
   * for this specific point 
   */
  if((pSDNPSession->obj34DefaultVariation == 0) 
     &&(_switchPointDefVariation(pDesc, pPoint, 1)))
  {
    return(SDNPSESN_READ_NEW_VARIATION);  
  }
#else
  TMWTARG_UNUSED_PARAM(pDesc);
#endif

  /* Read current value */
  sdnpdata_anlgInDBandRead(pPoint, &value);

  /* Diagnostics */
  DNPDIAG_SHOW_ANALOG_DEADBAND(pSession, pointNum, &value, 0);

  /* Store value */
  usvalue = dnputil_getAnlgDBandValueUShort(&value);
  tmwtarg_store16(&usvalue, &pResponse->pMsgBuf[pResponse->msgLength]);
  pResponse->msgLength += 2;

  return(SDNPSESN_READ_COMPLETE);
}
#endif

#if SDNPDATA_SUPPORT_OBJ34_V2
/* function: _readv2 */
static SDNPSESN_READ_STATUS TMWDEFS_CALLBACK _readv2(
  TMWSESN *pSession,
  TMWSESN_TX_DATA *pResponse,
  TMWTYPES_USHORT messageIndex,
  TMWTYPES_USHORT pointNum,
  void *pPoint,
  SDNPUTIL_STATIC_DESC *pDesc)
{
  TMWTYPES_ANALOG_VALUE value;
  TMWTYPES_ULONG ulvalue;

  TMWTARG_UNUSED_PARAM(messageIndex);

#if SDNPDATA_SUPPORT_STATIC_VAR_POINT
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  /* If session default variation is zero, check the default variation
   * for this specific point 
   */
  if((pSDNPSession->obj34DefaultVariation == 0) 
     &&(_switchPointDefVariation(pDesc, pPoint, 2)))
  {
    return(SDNPSESN_READ_NEW_VARIATION);  
  }
#else
  TMWTARG_UNUSED_PARAM(pDesc);
#endif

  /* Read current value */
  sdnpdata_anlgInDBandRead(pPoint, &value);

  /* Diagnostics */
  DNPDIAG_SHOW_ANALOG_DEADBAND(pSession, pointNum, &value, 0);

  /* Store value */
  ulvalue = dnputil_getAnlgDBandValueULong(&value);
  tmwtarg_store32(&ulvalue, &pResponse->pMsgBuf[pResponse->msgLength]);
  pResponse->msgLength += 4;

  return(SDNPSESN_READ_COMPLETE);
}
#endif

#if SDNPDATA_SUPPORT_OBJ34_V3
/* function: _readv3 */
static SDNPSESN_READ_STATUS TMWDEFS_LOCAL _readv3(
  TMWSESN *pSession,
  TMWSESN_TX_DATA *pResponse,
  TMWTYPES_USHORT messageIndex,
  TMWTYPES_USHORT pointNum,
  void *pPoint,
  SDNPUTIL_STATIC_DESC *pDesc)
{
  TMWTYPES_ANALOG_VALUE value;
  TMWTYPES_SFLOAT fvalue;

  TMWTARG_UNUSED_PARAM(messageIndex);

#if SDNPDATA_SUPPORT_STATIC_VAR_POINT
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  /* If session default variation is zero, check the default variation
   * for this specific point 
   */
  if((pSDNPSession->obj34DefaultVariation == 0) 
     &&(_switchPointDefVariation(pDesc, pPoint, 3)))
  {
    return(SDNPSESN_READ_NEW_VARIATION);  
  }
#else
  TMWTARG_UNUSED_PARAM(pDesc);
#endif

  /* Read current value */
  sdnpdata_anlgInDBandRead(pPoint, &value);

  /* Diagnostics */
  DNPDIAG_SHOW_ANALOG_DEADBAND(pSession, pointNum, &value, 0);

  /* Store value */
  fvalue = dnputil_getAnlgDBandValueFloat(&value);
  tmwtarg_storeSFloat(&fvalue, &pResponse->pMsgBuf[pResponse->msgLength]);
  pResponse->msgLength += 4;

  return(SDNPSESN_READ_COMPLETE);
}
#endif

/* function: _setVariationInfo() */
static void TMWDEFS_GLOBAL _setVariationInfo(
  SDNPUTIL_STATIC_DESC *pDesc, 
  TMWTYPES_UCHAR variation)
{
  switch(variation)
  {
  default:
    /* Fall through to first one that is supported */ 
    /* 2 is the preferred default */
#if SDNPDATA_SUPPORT_OBJ34_V2
  case 2:
    pDesc->variation = 2;
    pDesc->pReadIntoRespFunc = _readv2;
    pDesc->sizeInBytes = 4;
    break;
#endif
#if SDNPDATA_SUPPORT_OBJ34_V1
  case 1:
    pDesc->variation = 1;
    pDesc->pReadIntoRespFunc = _readv1;
    pDesc->sizeInBytes = 2;
    break;
#endif
#if SDNPDATA_SUPPORT_OBJ34_V3
  case 3:
    pDesc->variation = 3;
    pDesc->pReadIntoRespFunc = _readv3;
    pDesc->sizeInBytes = 4;
    break;
#endif
  }
}

/* functions: sdnpo034_readObj34 */
SDNPSESN_READ_STATUS TMWDEFS_CALLBACK sdnpo034_readObj34(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRequest,
  TMWSESN_TX_DATA *pResponse,
  DNPUTIL_OBJECT_HEADER *pObjHeader,
  SDNPSESN_QUAL qualifier)
{
  SDNPUTIL_STATIC_DESC desc;
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  TMWTYPES_UCHAR variation = pObjHeader->variation;
  
  if(variation == 0)
  { 
    variation = pSDNPSession->obj34DefaultVariation;
  }
  else
  {
    if(!_isSupportedVariation(variation))
    {
      pSDNPSession->iin |= DNPDEFS_IIN_OBJECT_UNKNOWN; 
      return (SDNPSESN_READ_FAILED);
    }

#if SDNPDATA_SUPPORT_STATIC_VAR_POINT
    /* Use per point variation even when specific variation is read. */
    if (pSDNPSession->analogInSizeVarPerPoint)
    {
      variation = 0;
    }
#endif
  }

  desc.group = DNPDEFS_OBJ_34_ANA_INPUT_DBANDS;
  desc.readVariation = variation; 
  desc.readBits = TMWDEFS_FALSE;
  desc.pQuantityFunc = sdnpdata_anlgInDBandQuantity;
  desc.pGetPointFunc = sdnpdata_anlgInDBandGetPoint;
#if SDNPDATA_SUPPORT_CLASS0_POINT
  desc.pIsInClass0Func = sdnpdata_anlgInIsClass0;
#endif

  _setVariationInfo(&desc, variation);

  return(sdnputil_readStatic(pSession,
    pRequest, pResponse, pObjHeader, qualifier, &desc));
}

#if SDNPDATA_SUPPORT_OBJ34_V1
/* function: sdnpo034_writeObj34v1 */
TMWTYPES_BOOL TMWDEFS_CALLBACK sdnpo034_writeObj34v1(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  TMWSESN_TX_DATA *pResponse,
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  TMWTYPES_BOOL status = TMWDEFS_TRUE;
  TMWTYPES_ANALOG_VALUE anlgValue;
  TMWTYPES_SHORT deadbandValue;
  TMWTYPES_USHORT point;
  TMWTYPES_USHORT i;
  void *pPoint;

  /* Response to a write request is null response so we don't need to
   *  put anything into it here.
   */
  TMWTARG_UNUSED_PARAM(pResponse);

  /* Loop through the points in the request */
  for(i = 0; i < pObjHeader->numberOfPoints; i++)
  {
    /* Get next point number from request */
    dnputil_getPointNumber(pRxFragment, pObjHeader, i, &point);

    /* Length of request was verified in sdnpsesn _validateMessageSize */ 

    /* Get deadband value */
    tmwtarg_get16(&pRxFragment->pMsgBuf[pRxFragment->offset], (TMWTYPES_USHORT *)&deadbandValue);
    pRxFragment->offset += 2;

    anlgValue.type = TMWTYPES_ANALOG_TYPE_USHORT;
    anlgValue.value.sval = deadbandValue;

    /* Diagnostics */
    DNPDIAG_SHOW_ANALOG_DEADBAND(pSession, point, &anlgValue, TMWDIAG_ID_RX);

    /* Get point */
    pPoint = sdnpdata_anlgInDBandGetPoint(pSDNPSession->pDbHandle, point);
    if((pPoint == TMWDEFS_NULL) || !sdnpdata_anlgInDBandWrite(pPoint, &anlgValue))
    {
      status = TMWDEFS_FALSE;
    }
  }

  return(status);
}
#endif /* SDNPDATA_SUPPORT_OBJ34_V1 */

#if SDNPDATA_SUPPORT_OBJ34_V2 
/* function: sdnpo034_writeObj34v2 */
TMWTYPES_BOOL TMWDEFS_CALLBACK sdnpo034_writeObj34v2(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  TMWSESN_TX_DATA *pResponse,
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  TMWTYPES_BOOL status = TMWDEFS_TRUE;
  TMWTYPES_ANALOG_VALUE anlgValue;
  TMWTYPES_LONG deadbandValue;
  TMWTYPES_USHORT point;
  TMWTYPES_USHORT i;
  void *pPoint;

  /* Response to a write request is null response so we don't need to
   *  put anything into it here.
   */
  TMWTARG_UNUSED_PARAM(pResponse);

  /* Loop through the points in the request */
  for(i = 0; i < pObjHeader->numberOfPoints; i++)
  {
    /* Get next point number from request */
    dnputil_getPointNumber(pRxFragment, pObjHeader, i, &point);

    /* Length of request was verified in sdnpsesn _validateMessageSize */ 

    /* Get deadband value */
    tmwtarg_get32(&pRxFragment->pMsgBuf[pRxFragment->offset], (TMWTYPES_ULONG *)&deadbandValue);
    pRxFragment->offset += 4;

    anlgValue.type = TMWTYPES_ANALOG_TYPE_ULONG;
    anlgValue.value.lval = deadbandValue;

    /* Diagnostics */
    DNPDIAG_SHOW_ANALOG_DEADBAND(pSession, point, &anlgValue, TMWDIAG_ID_RX);

    /* Get point */
    pPoint = sdnpdata_anlgInDBandGetPoint(pSDNPSession->pDbHandle, point);
    if((pPoint == TMWDEFS_NULL) || !sdnpdata_anlgInDBandWrite(pPoint, &anlgValue))
    {
      status = TMWDEFS_FALSE;
    }
  }

  return(status);
}
#endif /* SDNPDATA_SUPPORT_OBJ34_V2 */

#if SDNPDATA_SUPPORT_OBJ34_V3
/* function: sdnpo034_writeObj34v3 */
TMWTYPES_BOOL TMWDEFS_CALLBACK sdnpo034_writeObj34v3(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  TMWSESN_TX_DATA *pResponse,
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  TMWTYPES_BOOL status = TMWDEFS_TRUE;
  TMWTYPES_ANALOG_VALUE anlgValue;
  TMWTYPES_SFLOAT deadbandValue;
  TMWTYPES_USHORT point;
  TMWTYPES_USHORT i;
  void *pPoint;

  /* Response to a write request is null response so we don't need to
   *  put anything into it here.
   */
  TMWTARG_UNUSED_PARAM(pResponse);

  /* Loop through the points in the request */
  for(i = 0; i < pObjHeader->numberOfPoints; i++)
  {
    /* Get next point number from request */
    dnputil_getPointNumber(pRxFragment, pObjHeader, i, &point);
    
    /* Length of request was verified in sdnpsesn _validateMessageSize */ 

    /* Get deadband value */
    tmwtarg_get32(&pRxFragment->pMsgBuf[pRxFragment->offset], (TMWTYPES_ULONG *)&deadbandValue);
    pRxFragment->offset += 4;

    /* Negative values should be converted to 0 */
    if(deadbandValue < 0.0)
      deadbandValue = 0.0;

    anlgValue.type = TMWTYPES_ANALOG_TYPE_SFLOAT;
    anlgValue.value.fval = deadbandValue;

    /* Diagnostics */
    DNPDIAG_SHOW_ANALOG_DEADBAND(pSession, point, &anlgValue, TMWDIAG_ID_RX);

    /* Get point */
    pPoint = sdnpdata_anlgInDBandGetPoint(pSDNPSession->pDbHandle, point);
    if((pPoint == TMWDEFS_NULL) || !sdnpdata_anlgInDBandWrite(pPoint, &anlgValue))
    {
      status = TMWDEFS_FALSE;
    }
  }

  return(status);
}
#endif /* SDNPDATA_SUPPORT_OBJ34_V3 */
#endif /* SDNPDATA_SUPPORT_OBJ34 */
