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

/* file: sdnpo010.c
 * description: DNP Slave functionality for Object 10 Binary Output Status
 */
#include "tmwscl/dnp/dnpdiag.h"
#include "tmwscl/dnp/dnpdefs.h"
#include "tmwscl/dnp/sdnpo010.h"
#include "tmwscl/dnp/sdnpdata.h"
#include "tmwscl/dnp/sdnputil.h"
#include "tmwscl/dnp/sdnpdiag.h"

#include "tmwscl/utils/tmwtarg.h"

#if SDNPDATA_SUPPORT_OBJ10

static void TMWDEFS_GLOBAL _setVariationInfo(
  SDNPUTIL_STATIC_DESC *pDesc,
  TMWTYPES_UCHAR variation);

/* function: _isSupportedVariation() */
static TMWTYPES_BOOL TMWDEFS_LOCAL _isSupportedVariation(
  TMWTYPES_UCHAR variation)
{
  switch(variation)
  {
#if SDNPDATA_SUPPORT_OBJ10_V1
  case 1:
    break;
#endif
#if SDNPDATA_SUPPORT_OBJ10_V2
  case 2:
    break;
#endif
  default:
    return(TMWDEFS_FALSE);
  }

  return(TMWDEFS_TRUE);
}

#if SDNPDATA_SUPPORT_OBJ10_V1
/* function: _readv1 */
static SDNPSESN_READ_STATUS TMWDEFS_CALLBACK _readv1(
  TMWSESN *pSession,
  TMWSESN_TX_DATA *pResponse,
  TMWTYPES_USHORT messageIndex,
  TMWTYPES_USHORT pointNum,
  void *pPoint,
  SDNPUTIL_STATIC_DESC *pDesc)
{
  TMWTYPES_UCHAR flags;
  TMWTYPES_UCHAR shift = (TMWTYPES_UCHAR)(messageIndex % 8);

#if SDNPDATA_SUPPORT_OBJ10_V2 && SDNPDATA_SUPPORT_STATIC_VAR_POINT
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;

  /* If session default variation is zero, check the default variation
   * for this specific point
   */
  if((pSDNPSession->obj10DefaultVariation == 0)
     && (pDesc->readVariation == 0)
     && (sdnpdata_binOutDefVariation(pPoint) == 2))
  {
    _setVariationInfo(pDesc, 2);

    return(SDNPSESN_READ_NEW_VARIATION);
  }
#endif

  /* Read current value */
  sdnpdata_binOutRead(pPoint, &flags);

#if SDNPDATA_SUPPORT_OBJ10_V2
  /* Check to see whether flags are in nominal state. If not variation 2 must
   * be used.
   */
  if((flags & 0x7f) != DNPDEFS_DBAS_FLAG_ON_LINE)
  {
    /* Change the variation to the one with flags */
    _setVariationInfo(pDesc, 2);

    return(SDNPSESN_READ_NEW_VARIATION);
  }
#else
  TMWTARG_UNUSED_PARAM(pDesc);
#endif

  /* Diagnostics */
  DNPDIAG_SHOW_BINARY_OUTPUT(pSession, pointNum, flags, TMWDEFS_FALSE, TMWDEFS_NULL);

  if(shift == 0)
  {
    /* If this is the first bit to go in response byte 
     * set all bits to zero 
     */
    pResponse->pMsgBuf[pResponse->msgLength] = 0;
  }

  /* Store value */
  if(flags & DNPDEFS_DBAS_FLAG_BINARY_ON)
  {
    /* the temporary variable value was
     * added here because the Softtools rabbit compiler
     * generates bad code when the following is codded as
     *    pResponse->pMsgBuf[pResponse->msgLength] |= (0x01 << shift);
     */

    TMWTYPES_UCHAR value = (TMWTYPES_UCHAR)(0x01 << shift);
    pResponse->pMsgBuf[pResponse->msgLength] |= value;
  }

  return(SDNPSESN_READ_COMPLETE);
}
#endif

#if SDNPDATA_SUPPORT_OBJ10_V2
/* function: _readv2 */
static SDNPSESN_READ_STATUS TMWDEFS_CALLBACK _readv2(
  TMWSESN *pSession,
  TMWSESN_TX_DATA *pResponse,
  TMWTYPES_USHORT messageIndex,
  TMWTYPES_USHORT pointNum,
  void *pPoint,
  SDNPUTIL_STATIC_DESC *pDesc)
{
  TMWTYPES_UCHAR flags;
#if SDNPDATA_SUPPORT_STATIC_VAR_POINT
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
#else
  TMWTARG_UNUSED_PARAM(pDesc);
#endif

  TMWTARG_UNUSED_PARAM(messageIndex);

  /* Read current value */
  sdnpdata_binOutRead(pPoint, &flags);

#if SDNPDATA_SUPPORT_STATIC_VAR_POINT
  if((pSDNPSession->obj10DefaultVariation == 0)
    && (pDesc->readVariation == 0))
  {
    /* If session default variation is zero, check the default variation
     * for this specific point
     * Also, check to see if the flags are nominal, otherwise
     * we will just have to switch back to this variation
     */
    if((sdnpdata_binOutDefVariation(pPoint) == 1)
      &&((flags & 0x7f) == DNPDEFS_DBAS_FLAG_ON_LINE))
    {
      _setVariationInfo(pDesc, 1);
      return(SDNPSESN_READ_NEW_VARIATION);
    }
  }
#endif

  /* Diagnostics */
  DNPDIAG_SHOW_BINARY_OUTPUT(pSession, pointNum, flags, TMWDEFS_FALSE, TMWDEFS_NULL);

  /* Store into response */
  pResponse->pMsgBuf[pResponse->msgLength++] = flags;

  return(SDNPSESN_READ_COMPLETE);
}
#endif

/* function: _setVariationInfo() */
static void TMWDEFS_LOCAL _setVariationInfo(
  SDNPUTIL_STATIC_DESC *pDesc,
  TMWTYPES_UCHAR variation)
{
  switch(variation)
  {
  default:
    /* Fall through to first one that is supported */
    /* 2 is the preferred default */
#if SDNPDATA_SUPPORT_OBJ10_V2
  case 2:
    pDesc->variation = 2;
    pDesc->pReadIntoRespFunc = _readv2;
    pDesc->readBits = TMWDEFS_FALSE;
    pDesc->sizeInBytes = 1;
    break;
#endif
#if SDNPDATA_SUPPORT_OBJ10_V1
  case 1:
    pDesc->variation = 1;
    pDesc->pReadIntoRespFunc = _readv1;
    pDesc->readBits = TMWDEFS_TRUE;
    pDesc->sizeInBytes = 1;
    break;
#endif
  }
}

/* function: sdnpo010_readObj10 */
SDNPSESN_READ_STATUS TMWDEFS_CALLBACK sdnpo010_readObj10(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRequest,
  TMWSESN_TX_DATA *pResponse,
  DNPUTIL_OBJECT_HEADER *pObjHeader,
  SDNPSESN_QUAL qualifier)
{
  SDNPUTIL_STATIC_DESC desc;
  TMWTYPES_UCHAR variation = pObjHeader->variation;
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;

  if(variation == 0)
  {
    variation = pSDNPSession->obj10DefaultVariation;
  }
  else
  {
    if(!_isSupportedVariation(variation))
    {
      pSDNPSession->iin |= DNPDEFS_IIN_OBJECT_UNKNOWN;
      return (SDNPSESN_READ_FAILED);
    }
  }

  desc.group = DNPDEFS_OBJ_10_BIN_OUT_STATUSES;
  desc.readVariation = variation;
  desc.pQuantityFunc = sdnpdata_binOutQuantity;
  desc.pGetPointFunc = sdnpdata_binOutGetPoint;
#if SDNPDATA_SUPPORT_CLASS0_POINT
  desc.pIsInClass0Func = sdnpdata_binOutIsClass0;
#endif

  _setVariationInfo(&desc, variation);

  return(sdnputil_readStatic(pSession,
    pRequest, pResponse, pObjHeader, qualifier, &desc));
}

#if SDNPDATA_SUPPORT_OBJ10_WRITE 
/* function: sdnpo010_writeObj10v1 */
TMWTYPES_BOOL TMWDEFS_CALLBACK sdnpo010_writeObj10v1(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  TMWSESN_TX_DATA *pResponse,
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{
  TMWTYPES_USHORT i;
  int shift;
  void *pPoint; 
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  TMWTYPES_BOOL status = TMWDEFS_TRUE;

  /* Response to a write request is null response so we don't need to
   *  put anything into it here.
   */
  TMWTARG_UNUSED_PARAM(pResponse);

  if((pObjHeader->qualifier != DNPDEFS_QUAL_8BIT_START_STOP) && (pObjHeader->qualifier != DNPDEFS_QUAL_16BIT_START_STOP))
  {
    /* Don't try to process rest of message */
    pRxFragment->offset = pRxFragment->msgLength;
    SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_INV_QUAL_CTRL);
    return(TMWDEFS_FALSE);
  }

  shift = 0;
  /* Loop through the points in the request */
  for(i = 0; i < pObjHeader->numberOfPoints; i++)
  {
    TMWTYPES_USHORT point;
    TMWTYPES_UCHAR value;

    /* Get next point number from request */
    dnputil_getPointNumber(pRxFragment, pObjHeader, i, &point);
    
    /* sanity check */
    if(pRxFragment->offset >= pRxFragment->msgLength)
    {
      SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_PARSE_ERROR);
      return(TMWDEFS_FALSE);
    }

    if((pRxFragment->pMsgBuf[pRxFragment->offset] & (0x01 << shift)) != 0)
      value = DNPDEFS_DBAS_FLAG_BINARY_ON;
    else
      value = DNPDEFS_DBAS_FLAG_BINARY_OFF;

    DNPDIAG_SHOW_BIN_OUT_WRITE(pSession, point, value);

    /* Get point */
    pPoint = sdnpdata_binOutGetPoint(pSDNPSession->pDbHandle, point);
    if((pPoint == TMWDEFS_NULL) || !sdnpdata_binOutWrite(pPoint, value))
    {
      status = TMWDEFS_FALSE;
    }

    shift++;
    if(shift > 7)
    {
      shift = 0;
      pRxFragment->offset += 1;
    }
  }
  if(shift != 0)
    pRxFragment->offset += 1;

  return(status);
}
#endif

#if SDNPDATA_SUPPORT_OBJ11
#if SDNPDATA_SUPPORT_ASSIGN
/* function: sdnpo010_assignClass */
TMWTYPES_BOOL TMWDEFS_CALLBACK sdnpo010_assignClass(
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
  status = sdnputil_validateAssignQualifier(pObjHeader, sdnpdata_binOutQuantity(pSDNPSession->pDbHandle));
  
  for(i = 0; i < pObjHeader->numberOfPoints; i++)
  {
    void *pPoint;
    TMWTYPES_USHORT pointNumber;
    dnputil_getPointNumber(pRxFragment, pObjHeader, i, &pointNumber);

    pPoint = sdnpdata_binOutGetPoint(pSDNPSession->pDbHandle, pointNumber);
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

    if(!sdnpdata_binOutAssignClass(pPoint, classMask))
      status = TMWDEFS_FALSE;
  }

  return(status);
}
#endif
#endif /* SDNPDATA_SUPPORT_OBJ11 */

#endif /* SDNPDATA_SUPPORT_OBJ10 */
