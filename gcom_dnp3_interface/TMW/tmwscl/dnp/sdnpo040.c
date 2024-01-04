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

/* file: sdnpo040.c
 * description: DNP Slave functionality for Object 40 Analog Output Status
 */
#include "tmwscl/dnp/dnpdiag.h"
#include "tmwscl/utils/tmwtarg.h"
#include "tmwscl/dnp/dnpdefs.h"
#include "tmwscl/dnp/sdnpo040.h"
#include "tmwscl/dnp/sdnpdata.h"
#include "tmwscl/dnp/sdnputil.h"

#if SDNPDATA_SUPPORT_OBJ40

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
#if SDNPDATA_SUPPORT_OBJ40_V1
  case 1:
    break;
#endif
#if SDNPDATA_SUPPORT_OBJ40_V2
  case 2:
    break;
#endif
#if SDNPDATA_SUPPORT_OBJ40_V3
  case 3:
    break;
#endif
#if SDNPDATA_SUPPORT_OBJ40_V4
  case 4:
    break;
#endif
  default:
    return (TMWDEFS_FALSE);
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
     TMWTYPES_UCHAR variation = sdnpdata_anlgOutDefVariation(pPoint); 
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

#if SDNPDATA_SUPPORT_OBJ40_V1
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
  TMWTYPES_LONG lvalue;
  TMWTYPES_UCHAR flags;

  TMWTARG_UNUSED_PARAM(messageIndex);

#if SDNPDATA_SUPPORT_STATIC_VAR_POINT
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  /* If session default variation is zero, check the default variation
   * for this specific point 
   */
  if((pSDNPSession->obj40DefaultVariation == 0) 
     &&(_switchPointDefVariation(pDesc, pPoint, 1)))
  {
    return(SDNPSESN_READ_NEW_VARIATION);  
  }
#else
  TMWTARG_UNUSED_PARAM(pDesc);
#endif

  /* Read current value */
  sdnpdata_anlgOutRead(pPoint, &value, &flags);
  
  /* Get value, setting over range bit in flags properly */
  lvalue = dnputil_getAnalogValueLong(&value, &flags);
  
  /* Diagnostics */
  DNPDIAG_SHOW_ANALOG_OUTPUT(pSession, pointNum, &value, flags, TMWDEFS_FALSE, TMWDEFS_NULL);

  /* Store flags */
  pResponse->pMsgBuf[pResponse->msgLength++] = flags;

  /* Store value */ 
  tmwtarg_store32((TMWTYPES_ULONG *)&lvalue, &pResponse->pMsgBuf[pResponse->msgLength]);
  pResponse->msgLength += 4;

  return(SDNPSESN_READ_COMPLETE);
}
#endif /* SDNPDATA_SUPPORT_OBJ40_V1 */

#if SDNPDATA_SUPPORT_OBJ40_V2
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
  TMWTYPES_SHORT svalue;
  TMWTYPES_UCHAR flags;

  TMWTARG_UNUSED_PARAM(messageIndex);

#if SDNPDATA_SUPPORT_STATIC_VAR_POINT
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  /* If session default variation is zero, check the default variation
   * for this specific point 
   */
  if((pSDNPSession->obj40DefaultVariation == 0) 
     &&(_switchPointDefVariation(pDesc, pPoint, 2)))
  {
    return(SDNPSESN_READ_NEW_VARIATION);  
  }
#else
  TMWTARG_UNUSED_PARAM(pDesc);
#endif

  /* Read current value */
  sdnpdata_anlgOutRead(pPoint, &value, &flags);

  /* Get value, setting over range bit in flags properly */
  svalue = dnputil_getAnalogValueShort(&value, &flags);

  /* Diagnostics */
  DNPDIAG_SHOW_ANALOG_OUTPUT(pSession, pointNum, &value, flags, TMWDEFS_FALSE, TMWDEFS_NULL);

  /* Store flags */
  pResponse->pMsgBuf[pResponse->msgLength++] = flags;

  /* Store value */ 
  tmwtarg_store16((TMWTYPES_USHORT *)&svalue, &pResponse->pMsgBuf[pResponse->msgLength]);
  pResponse->msgLength += 2;

  return(SDNPSESN_READ_COMPLETE);
}
#endif /* SDNPDATA_SUPPORT_OBJ40_V2 */

#if SDNPDATA_SUPPORT_OBJ40_V3
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
  TMWTYPES_UCHAR flags;

  TMWTARG_UNUSED_PARAM(messageIndex);

#if SDNPDATA_SUPPORT_STATIC_VAR_POINT
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  /* If session default variation is zero, check the default variation
   * for this specific point 
   */
  if((pSDNPSession->obj40DefaultVariation == 0) 
     &&(_switchPointDefVariation(pDesc, pPoint, 3)))
  {
    return(SDNPSESN_READ_NEW_VARIATION);  
  }
#else
  TMWTARG_UNUSED_PARAM(pDesc);
#endif

  /* Read current value */
  sdnpdata_anlgOutRead(pPoint, &value, &flags);

  /* Get value, setting over range bit in flags properly */
  fvalue = dnputil_getAnalogValueFloat(&value, &flags);

  /* Diagnostics */
  DNPDIAG_SHOW_ANALOG_OUTPUT(pSession, pointNum, &value, flags, TMWDEFS_FALSE, TMWDEFS_NULL);

  /* Store flags */
  pResponse->pMsgBuf[pResponse->msgLength++] = flags;

  /* Store value */
  tmwtarg_storeSFloat(&fvalue, &pResponse->pMsgBuf[pResponse->msgLength]);
  pResponse->msgLength += 4;

  return(SDNPSESN_READ_COMPLETE);
}
#endif /* SDNPDATA_SUPPORT_OBJ40_V3 */

#if SDNPDATA_SUPPORT_OBJ40_V4
/* function: _readv4 */
static SDNPSESN_READ_STATUS TMWDEFS_LOCAL _readv4(
  TMWSESN *pSession,
  TMWSESN_TX_DATA *pResponse,
  TMWTYPES_USHORT messageIndex,
  TMWTYPES_USHORT pointNum,
  void *pPoint,
  SDNPUTIL_STATIC_DESC *pDesc)
{
  TMWTYPES_ANALOG_VALUE value;
  TMWTYPES_DOUBLE dval;
  TMWTYPES_UCHAR flags;

  TMWTARG_UNUSED_PARAM(messageIndex);

#if SDNPDATA_SUPPORT_STATIC_VAR_POINT
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  /* If session default variation is zero, check the default variation
   * for this specific point 
   */
  if((pSDNPSession->obj40DefaultVariation == 0) 
     &&(_switchPointDefVariation(pDesc, pPoint, 4)))
  {
    return(SDNPSESN_READ_NEW_VARIATION);  
  }
#else
  TMWTARG_UNUSED_PARAM(pDesc);
#endif

  /* Read current value */
  sdnpdata_anlgOutRead(pPoint, &value, &flags);

  /* Get value, setting over range bit in flags properly */
  dval = dnputil_getAnalogValueDouble(&value); 
  
  /* Diagnostics */
  DNPDIAG_SHOW_ANALOG_OUTPUT(pSession, pointNum, &value, flags, TMWDEFS_FALSE, TMWDEFS_NULL);

  /* Store flags */
  pResponse->pMsgBuf[pResponse->msgLength++] = flags;

  /* Store value */
  tmwtarg_store64(&dval, &pResponse->pMsgBuf[pResponse->msgLength]);
  pResponse->msgLength += 8;

  return(SDNPSESN_READ_COMPLETE);
}
#endif /* SDNPDATA_SUPPORT_OBJ40_V4 */

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
#if SDNPDATA_SUPPORT_OBJ40_V2
  case 2:
    pDesc->variation = 2;
    pDesc->pReadIntoRespFunc = _readv2;
    pDesc->sizeInBytes = 3;
    break;
#endif
#if SDNPDATA_SUPPORT_OBJ40_V1
  case 1:
    pDesc->variation = 1;
    pDesc->pReadIntoRespFunc = _readv1;
    pDesc->sizeInBytes = 5;
    break;
#endif
#if SDNPDATA_SUPPORT_OBJ40_V3
  case 3:
    pDesc->variation = 3;
    pDesc->pReadIntoRespFunc = _readv3;
    pDesc->sizeInBytes = 5;
    break;
#endif
#if SDNPDATA_SUPPORT_OBJ40_V4
  case 4:
    pDesc->variation = 4;
    pDesc->pReadIntoRespFunc = _readv4;
    pDesc->sizeInBytes = 9;
    break;
#endif
  }
}
/* function: sdnpo040_readObj40 */
SDNPSESN_READ_STATUS TMWDEFS_CALLBACK sdnpo040_readObj40(
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
    variation = pSDNPSession->obj40DefaultVariation;
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
    if (pSDNPSession->analogOutSizeVarPerPoint)
    {
      variation = 0;
    }
#endif
  }

  desc.group = DNPDEFS_OBJ_40_ANA_OUT_STATUSES;
  desc.readVariation = variation; 
  desc.readBits = TMWDEFS_FALSE;
  desc.pQuantityFunc = sdnpdata_anlgOutQuantity;
  desc.pGetPointFunc = sdnpdata_anlgOutGetPoint;
#if SDNPDATA_SUPPORT_CLASS0_POINT
  desc.pIsInClass0Func = sdnpdata_anlgOutIsClass0;
#endif

  _setVariationInfo(&desc, variation);

  return(sdnputil_readStatic(pSession,
    pRequest, pResponse, pObjHeader, qualifier, &desc));
}

#if SDNPDATA_SUPPORT_OBJ42
#if SDNPDATA_SUPPORT_ASSIGN
/* function: sdnpo040_assignClass */
TMWTYPES_BOOL TMWDEFS_CALLBACK sdnpo040_assignClass(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  DNPUTIL_OBJECT_HEADER *pObjHeader,
  TMWDEFS_CLASS_MASK classMask)
{
  SDNPSESN *pSDNPSession;
  TMWTYPES_BOOL status;
  TMWTYPES_USHORT i;

  pSDNPSession = (SDNPSESN *)pSession;

  /* limit requested number of points to quantity in database 
   * if status == TMWDEFS_FALSE IN2.2 will be set in response.
   */
  status = sdnputil_validateAssignQualifier(pObjHeader, sdnpdata_anlgOutQuantity(pSDNPSession->pDbHandle));

  for(i = 0; i < pObjHeader->numberOfPoints; i++)
  {
    void *pPoint;
    TMWTYPES_USHORT pointNumber;
    dnputil_getPointNumber(pRxFragment, pObjHeader, i, &pointNumber);

    pPoint = sdnpdata_anlgOutGetPoint(pSDNPSession->pDbHandle, pointNumber);
    if(pPoint == TMWDEFS_NULL)
    {
      /* Failure of any point for startstop or indexed qualifiers should set IIN2.2 in response */
      if((pObjHeader->qualifier < DNPDEFS_QUAL_ALL_POINTS)
        ||(pObjHeader->qualifier > DNPDEFS_QUAL_16BIT_LIMITED_QTY))
      {
        status = TMWDEFS_FALSE;
      }
      continue;
    }

    if(!sdnpdata_anlgOutAssignClass(pPoint, classMask))
      status = TMWDEFS_FALSE;
  }

  return(status);
}
#endif
#endif /* SDNPDATA_SUPPORT_OBJ42 */

#endif /* SDNPDATA_SUPPORT_OBJ40 */

