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

/* file: sdnpo114.c
 * description: DNP Slave functionality for Object 114 Extended Octet String Object
 */
#include "tmwscl/dnp/dnpdiag.h"
#include "tmwscl/dnp/sdnpdiag.h"
#include "tmwscl/utils/tmwtarg.h"

#include "tmwscl/dnp/sdnpo114.h"
#include "tmwscl/dnp/sdnpdata.h"
#include "tmwscl/dnp/sdnpsesn.h"
#include "tmwscl/dnp/sdnputil.h"

#if SDNPDATA_SUPPORT_OBJ114

/* forward declarations */
static void TMWDEFS_GLOBAL _switchVariationInfo(
  SDNPUTIL_STATIC_DESC *pDesc, 
  TMWTYPES_UCHAR variation);

/* function: _isSupportedVariation() */
static TMWTYPES_BOOL TMWDEFS_LOCAL _isSupportedVariation(
  TMWTYPES_UCHAR variation)
{
  TMWTYPES_BOOL variationSupported = TMWDEFS_FALSE;
  switch(variation)
  {
  case 1:
    variationSupported = SDNPDATA_SUPPORT_OBJ114_V1;
    break;
  case 2:
    variationSupported = SDNPDATA_SUPPORT_OBJ114_V2;
    break;
  case 3:
    variationSupported = SDNPDATA_SUPPORT_OBJ114_V3;
    break;
  case 4:
    variationSupported = SDNPDATA_SUPPORT_OBJ114_V4;
    break;
  default:
    break;
  }
  return(variationSupported);
}

#if SDNPDATA_SUPPORT_STATIC_VAR_POINT
/* function: _flagNominalCheck 
 * purpose: Check to see if variation we are switching to requires
 *  flags to be nominal and if flags are nominal. If necessary change
 *  variation to one that supports flags.
 * arguments:
 *  pPoint - point to be read
 *  variation - variation we are trying to switch to
 * returns:
 *  void
 */
static void TMWDEFS_LOCAL _flagNominalCheck(
  TMWTYPES_UCHAR flags,
  TMWTYPES_UCHAR *pVariation)     
{
  if((*pVariation == 1) || (*pVariation == 2))
  { 
    /* Check to see whether flags are in nominal state */
    if((flags & 0x7f) != DNPDEFS_DBAS_FLAG_ON_LINE)
    {
      /* change variation from 1->3 2->4 */
      *pVariation = *pVariation +2;
    }
  }
}

/* function: _switchPointDefVariation */
static TMWTYPES_BOOL TMWDEFS_LOCAL _switchPointDefVariation(
  SDNPUTIL_STATIC_DESC *pDesc,
  void *pPoint,
  TMWTYPES_UCHAR flags,
  TMWTYPES_UCHAR currentVariation)                     
{
  /* If session default variation is zero, check the default variation
   * for this specific point 
   */
  if(pDesc->readVariation == 0)
  {
    TMWTYPES_UCHAR variation;

    if(pDesc->alreadySwitchedVariation)
      return(TMWDEFS_FALSE);

    variation = sdnpdata_extStrDefVariation(pPoint); 
    if((variation != currentVariation) 
      && (_isSupportedVariation(variation)))
    {
      /* Determine which variation should be used depending on flags */
      _flagNominalCheck(flags, &variation);
      _switchVariationInfo(pDesc, variation);

      /* if variation is still different than the current variation return true */
      if(variation != currentVariation)
        return(TMWDEFS_TRUE);
    }
  }
  return(TMWDEFS_FALSE);
}
#endif

#if SDNPDATA_SUPPORT_OBJ114_V1
/* function: _readv1 */
static SDNPSESN_READ_STATUS TMWDEFS_CALLBACK _readv1(
  TMWSESN *pSession,
  TMWSESN_TX_DATA *pResponse,
  TMWTYPES_USHORT messageIndex,
  TMWTYPES_USHORT pointNum,
  void *pPoint,
  SDNPUTIL_STATIC_DESC *pDesc)
{
#if SDNPDATA_SUPPORT_STATIC_VAR_POINT
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
#endif
  TMWTYPES_USHORT extStrLen;
  TMWTYPES_UCHAR *pExtString;
  TMWTYPES_UCHAR flags;
  TMWTARG_UNUSED_PARAM(messageIndex);

  /* Read current value */
#if SDNPDATA_SUPPORT_OBJ114_MIN_STACK
  pExtString = sdnpdata_extStrGetPtr(pPoint, &extStrLen, &flags);
#else
  TMWTYPES_UCHAR buf[DNPCNFG_MAX_EXT_STRING_LENGTH];
  pExtString = buf;
  sdnpdata_extStrRead(pPoint, sizeof(buf), buf, &extStrLen, &flags);
#endif

#if SDNPDATA_SUPPORT_STATIC_VAR_POINT
  /* If session default variation is zero, check the default variation
   * for this specific point 
   */
  if((pSDNPSession->obj114DefaultVariation == 0) 
     &&(_switchPointDefVariation(pDesc, pPoint, flags, 1)))
  {
#if SDNPDATA_SUPPORT_OBJ114_MIN_STACK
    sdnpdata_extStrRelease(pPoint);
#endif
    return(SDNPSESN_READ_NEW_VARIATION);  
  }
#endif

  if (extStrLen > DNPDEFS_MAX_STRING_LENGTH)
  {
    extStrLen = DNPDEFS_MAX_STRING_LENGTH;
    flags |= DNPDEFS_DBAS_FLAG_OVERFLOW;
  }

  /* Use variation header size + string length to check if there is enough space in the fragment. */
  if((pResponse->msgLength + pDesc->sizeInBytes + extStrLen) > pResponse->maxLength)
  {
    /* If this is the first point in the fragment, the string will have to be truncated */
    if ((pResponse->msgLength == 9) ||
       ((pResponse->txFlags &= TMWSESN_TXFLAGS_DNP_AUTH_AGGR) && (pResponse->msgLength == 19)))
    {
      extStrLen = pResponse->maxLength - (pResponse->msgLength + pDesc->sizeInBytes);
      flags |= DNPDEFS_DBAS_FLAG_OVERFLOW;
    }
    /* Otherwise, put it in the next fragment */
    else
    {
#if SDNPDATA_SUPPORT_OBJ114_MIN_STACK
      sdnpdata_extStrRelease(pPoint);
#endif
      return(SDNPSESN_READ_MORE_DATA);
    }
  }

#if SDNPDATA_SUPPORT_OBJ114_V3
  /* Check to see whether flags are in nominal state */
  if((flags & 0x7f) != DNPDEFS_DBAS_FLAG_ON_LINE)
  {
    /* Change the variation to the one with flags */
    _switchVariationInfo(pDesc, 3);
#if SDNPDATA_SUPPORT_OBJ114_MIN_STACK
    sdnpdata_extStrRelease(pPoint);
#endif
    return(SDNPSESN_READ_NEW_VARIATION);
  }
#endif

  /* Diagnostics */
  DNPDIAG_SHOW_EXT_STRING_DATA(pSession, pointNum, pExtString, extStrLen, flags, 0);

  /* Store the length as a single byte */
  pResponse->pMsgBuf[pResponse->msgLength++] = (TMWTYPES_UCHAR)extStrLen;
 
  /* Store value */
  memcpy(pResponse->pMsgBuf + pResponse->msgLength, pExtString, extStrLen);
  pResponse->msgLength = (TMWTYPES_USHORT)(pResponse->msgLength + extStrLen);
#if SDNPDATA_SUPPORT_OBJ114_MIN_STACK
  sdnpdata_extStrRelease(pPoint);
#endif

  pDesc->alreadySwitchedVariation = TMWDEFS_FALSE;
  return(SDNPSESN_READ_COMPLETE);
}
#endif

#if SDNPDATA_SUPPORT_OBJ114_V2
/* function: _readv2 */
static SDNPSESN_READ_STATUS TMWDEFS_CALLBACK _readv2(
  TMWSESN *pSession,
  TMWSESN_TX_DATA *pResponse,
  TMWTYPES_USHORT messageIndex,
  TMWTYPES_USHORT pointNum,
  void *pPoint,
  SDNPUTIL_STATIC_DESC *pDesc)
{
#if SDNPDATA_SUPPORT_STATIC_VAR_POINT
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
#endif
  TMWTYPES_USHORT extStrLen;
  TMWTYPES_UCHAR *pExtString;
  TMWTYPES_UCHAR flags;
  TMWTARG_UNUSED_PARAM(messageIndex);

  /* Read current value */
#if SDNPDATA_SUPPORT_OBJ114_MIN_STACK
  pExtString = sdnpdata_extStrGetPtr(pPoint, &extStrLen, &flags);
#else
  TMWTYPES_UCHAR buf[DNPCNFG_MAX_EXT_STRING_LENGTH];
  pExtString = buf;
  sdnpdata_extStrRead(pPoint, sizeof(buf), buf, &extStrLen, &flags);
#endif

#if SDNPDATA_SUPPORT_STATIC_VAR_POINT
  /* If session default variation is zero, check the default variation
   * for this specific point 
   */
  if((pSDNPSession->obj114DefaultVariation == 0) 
     &&(_switchPointDefVariation(pDesc, pPoint, flags, 2)))
  {
#if SDNPDATA_SUPPORT_OBJ114_MIN_STACK
    sdnpdata_extStrRelease(pPoint);
#endif
    return(SDNPSESN_READ_NEW_VARIATION);  
  }
#endif

  if (extStrLen > DNPCNFG_MAX_EXT_STRING_LENGTH)
  {
    extStrLen = DNPCNFG_MAX_EXT_STRING_LENGTH;
    flags |= DNPDEFS_DBAS_FLAG_OVERFLOW;
  }

  /* Use variation header size + string length to check if there is enough space in the fragment. */
  if((pResponse->msgLength + pDesc->sizeInBytes + extStrLen) > pResponse->maxLength)
  {
    /* If this is the first point in the fragment, the string will have to be truncated */
    if ((pResponse->msgLength == 9) ||
       ((pResponse->txFlags &= TMWSESN_TXFLAGS_DNP_AUTH_AGGR) && (pResponse->msgLength == 19)))
    {
      extStrLen = pResponse->maxLength - (pResponse->msgLength + pDesc->sizeInBytes);
      flags |= DNPDEFS_DBAS_FLAG_OVERFLOW;
    }
    /* Otherwise, put it in the next fragment */
    else
    {
#if SDNPDATA_SUPPORT_OBJ114_MIN_STACK
      sdnpdata_extStrRelease(pPoint);
#endif
      return(SDNPSESN_READ_MORE_DATA);
    }
  }

#if SDNPDATA_SUPPORT_OBJ114_V4
  /* Check to see whether flags are in nominal state */
  if((flags & 0x7f) != DNPDEFS_DBAS_FLAG_ON_LINE)
  {
    /* Change the variation to the one with flags */
    _switchVariationInfo(pDesc, 4);
#if SDNPDATA_SUPPORT_OBJ114_MIN_STACK
    sdnpdata_extStrRelease(pPoint);
#endif
    return(SDNPSESN_READ_NEW_VARIATION);
  }
#endif

  /* Diagnostics */
  DNPDIAG_SHOW_EXT_STRING_DATA(pSession, pointNum, pExtString, extStrLen, flags, 0);

  /* Store a 2 byte length */
  tmwtarg_store16(&extStrLen, &pResponse->pMsgBuf[pResponse->msgLength]);
  pResponse->msgLength += 2;
 
  /* Store value */
  memcpy(pResponse->pMsgBuf + pResponse->msgLength, pExtString, extStrLen);
  pResponse->msgLength = (TMWTYPES_USHORT)(pResponse->msgLength + extStrLen);
#if SDNPDATA_SUPPORT_OBJ114_MIN_STACK
  sdnpdata_extStrRelease(pPoint);
#endif

  pDesc->alreadySwitchedVariation = TMWDEFS_FALSE;
  return(SDNPSESN_READ_COMPLETE);
}
#endif

#if SDNPDATA_SUPPORT_OBJ114_V3
/* function: _readv3 */
static SDNPSESN_READ_STATUS TMWDEFS_CALLBACK _readv3(
  TMWSESN *pSession,
  TMWSESN_TX_DATA *pResponse,
  TMWTYPES_USHORT messageIndex,
  TMWTYPES_USHORT pointNum,
  void *pPoint,
  SDNPUTIL_STATIC_DESC *pDesc)
{
#if SDNPDATA_SUPPORT_STATIC_VAR_POINT
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
#endif
  TMWTYPES_USHORT extStrLen;
  TMWTYPES_UCHAR *pExtString;
  TMWTYPES_UCHAR flags;
  TMWTARG_UNUSED_PARAM(messageIndex);

  /* Read current value */
#if SDNPDATA_SUPPORT_OBJ114_MIN_STACK
  pExtString = sdnpdata_extStrGetPtr(pPoint, &extStrLen, &flags);
#else
  TMWTYPES_UCHAR buf[DNPCNFG_MAX_EXT_STRING_LENGTH];
  pExtString = buf;
  sdnpdata_extStrRead(pPoint, sizeof(buf), buf, &extStrLen, &flags);
#endif

#if SDNPDATA_SUPPORT_STATIC_VAR_POINT
  /* If session default variation is zero, check the default variation
   * for this specific point 
   */
  if((pSDNPSession->obj114DefaultVariation == 0) 
     &&(_switchPointDefVariation(pDesc, pPoint, flags, 3)))
  {
#if SDNPDATA_SUPPORT_OBJ114_MIN_STACK
    sdnpdata_extStrRelease(pPoint);
#endif
    return(SDNPSESN_READ_NEW_VARIATION);  
  }
#endif

  if (extStrLen > DNPDEFS_MAX_STRING_LENGTH)
  {
    extStrLen = DNPDEFS_MAX_STRING_LENGTH;
    flags |= DNPDEFS_DBAS_FLAG_OVERFLOW;
  }

  /* Use variation header size + string length to check if there is enough space in the fragment. */
  if((pResponse->msgLength + pDesc->sizeInBytes + extStrLen) > pResponse->maxLength)
  {
    /* If this is the first point in the fragment, the string will have to be truncated */
    if ((pResponse->msgLength == 9) ||
      ((pResponse->txFlags &= TMWSESN_TXFLAGS_DNP_AUTH_AGGR) && (pResponse->msgLength == 19)))
    {
      extStrLen = pResponse->maxLength - (pResponse->msgLength + pDesc->sizeInBytes);
      flags |= DNPDEFS_DBAS_FLAG_OVERFLOW;
    }
    /* Otherwise, put it in the next fragment */
    else
    {
#if SDNPDATA_SUPPORT_OBJ114_MIN_STACK
      sdnpdata_extStrRelease(pPoint);
#endif
      return(SDNPSESN_READ_MORE_DATA);
    }
  }

  /* Diagnostics */
  DNPDIAG_SHOW_EXT_STRING_DATA(pSession, pointNum, pExtString, extStrLen, flags, 0);

  /* Store flags */
  pResponse->pMsgBuf[pResponse->msgLength++] = flags;

  /* Store the length as a single byte*/
  pResponse->pMsgBuf[pResponse->msgLength++] = (TMWTYPES_UCHAR)extStrLen;
 
  /* Store value */
  memcpy(pResponse->pMsgBuf + pResponse->msgLength, pExtString, extStrLen);
  pResponse->msgLength = (TMWTYPES_USHORT)(pResponse->msgLength + extStrLen);
#if SDNPDATA_SUPPORT_OBJ114_MIN_STACK
  sdnpdata_extStrRelease(pPoint);
#endif

  pDesc->alreadySwitchedVariation = TMWDEFS_FALSE;
  return(SDNPSESN_READ_COMPLETE);
}
#endif

#if SDNPDATA_SUPPORT_OBJ114_V4
/* function: _readv4 */
static SDNPSESN_READ_STATUS TMWDEFS_CALLBACK _readv4(
  TMWSESN *pSession,
  TMWSESN_TX_DATA *pResponse,
  TMWTYPES_USHORT messageIndex,
  TMWTYPES_USHORT pointNum,
  void *pPoint,
  SDNPUTIL_STATIC_DESC *pDesc)
{
#if SDNPDATA_SUPPORT_STATIC_VAR_POINT
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
#endif
  TMWTYPES_USHORT extStrLen;
  TMWTYPES_UCHAR *pExtString;
  TMWTYPES_UCHAR flags;
  TMWTARG_UNUSED_PARAM(messageIndex);

  /* Read current value */
#if SDNPDATA_SUPPORT_OBJ114_MIN_STACK
  pExtString = sdnpdata_extStrGetPtr(pPoint, &extStrLen, &flags);
#else
  TMWTYPES_UCHAR buf[DNPCNFG_MAX_EXT_STRING_LENGTH];
  pExtString = buf;
  sdnpdata_extStrRead(pPoint, sizeof(buf), buf, &extStrLen, &flags);
#endif

#if SDNPDATA_SUPPORT_STATIC_VAR_POINT
  /* If session default variation is zero, check the default variation
   * for this specific point 
   */
  if((pSDNPSession->obj114DefaultVariation == 0) 
     &&(_switchPointDefVariation(pDesc, pPoint, flags, 4)))
  {
#if SDNPDATA_SUPPORT_OBJ114_MIN_STACK
    sdnpdata_extStrRelease(pPoint);
#endif
    return(SDNPSESN_READ_NEW_VARIATION);  
  }
#endif

  if (extStrLen > DNPCNFG_MAX_EXT_STRING_LENGTH)
  {
    extStrLen = DNPCNFG_MAX_EXT_STRING_LENGTH;
    flags |= DNPDEFS_DBAS_FLAG_OVERFLOW;
  }

  /* Use variation header size + string length to check if there is enough space in the fragment. */
  if((pResponse->msgLength + pDesc->sizeInBytes + extStrLen) > pResponse->maxLength)
  {
    /* If this is the first point in the fragment, the string will have to be truncated */
    if ((pResponse->msgLength == 9) ||
      ((pResponse->txFlags &= TMWSESN_TXFLAGS_DNP_AUTH_AGGR) && (pResponse->msgLength == 19)))
    {
      extStrLen = pResponse->maxLength - (pResponse->msgLength + pDesc->sizeInBytes);
      flags |= DNPDEFS_DBAS_FLAG_OVERFLOW;
    }
    /* Otherwise, put it in the next fragment */
    else
    {
#if SDNPDATA_SUPPORT_OBJ114_MIN_STACK
      sdnpdata_extStrRelease(pPoint);
#endif
      return(SDNPSESN_READ_MORE_DATA);
    }
  }

  /* Diagnostics */
  DNPDIAG_SHOW_EXT_STRING_DATA(pSession, pointNum, pExtString, extStrLen, flags, 0);

  /* Store flags */
  pResponse->pMsgBuf[pResponse->msgLength++] = flags;

  /* Store a 2 byte length */
  tmwtarg_store16(&extStrLen, &pResponse->pMsgBuf[pResponse->msgLength]);
  pResponse->msgLength += 2;
 
  /* Store value */
  memcpy(pResponse->pMsgBuf + pResponse->msgLength, pExtString, extStrLen);
  pResponse->msgLength = (TMWTYPES_USHORT)(pResponse->msgLength + extStrLen);
#if SDNPDATA_SUPPORT_OBJ114_MIN_STACK
  sdnpdata_extStrRelease(pPoint);
#endif

  pDesc->alreadySwitchedVariation = TMWDEFS_FALSE;
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
    /* 1 is the preferred default */

#if SDNPDATA_SUPPORT_OBJ114_V1
  case 1:
    pDesc->variation = 1;
    pDesc->pReadIntoRespFunc = _readv1;
    pDesc->sizeInBytes = 1;
    break;
#endif
#if SDNPDATA_SUPPORT_OBJ114_V2
  case 2:
    pDesc->variation = 2;
    pDesc->pReadIntoRespFunc = _readv2;
    pDesc->sizeInBytes = 2;
    break;
#endif
#if SDNPDATA_SUPPORT_OBJ114_V3
  case 3:
    pDesc->variation = 3;
    pDesc->pReadIntoRespFunc = _readv3;
    pDesc->sizeInBytes = 2;
    break;
#endif
#if SDNPDATA_SUPPORT_OBJ114_V4
  case 4:
    pDesc->variation = 4;
    pDesc->pReadIntoRespFunc = _readv4;
    pDesc->sizeInBytes = 3;
    break;
#endif
  }
}

/* function: _switchVariationInfo() */
static void TMWDEFS_LOCAL _switchVariationInfo(
  SDNPUTIL_STATIC_DESC *pDesc, 
  TMWTYPES_UCHAR variation)
{
  _setVariationInfo(pDesc, variation);
  pDesc->alreadySwitchedVariation = TMWDEFS_TRUE;
}

/* function: sdnpo114_readObj114 */
SDNPSESN_READ_STATUS TMWDEFS_CALLBACK sdnpo114_readObj114(
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
    variation = pSDNPSession->obj114DefaultVariation;
  }
  else
  {
    if(!_isSupportedVariation(variation))
    {
      pSDNPSession->iin |= DNPDEFS_IIN_OBJECT_UNKNOWN; 
      return (SDNPSESN_READ_FAILED);
    }
  }

  desc.group = DNPDEFS_OBJ_114_EXT_STR_DATA;
  desc.readVariation = variation;
  desc.readBits = TMWDEFS_FALSE;
  desc.pQuantityFunc = sdnpdata_extStrQuantity;
  desc.pGetPointFunc = sdnpdata_extStrGetPoint;
  desc.alreadySwitchedVariation = TMWDEFS_FALSE;
#if SDNPDATA_SUPPORT_CLASS0_POINT
  desc.pIsInClass0Func = sdnpdata_extStrIsClass0;
#endif

   _setVariationInfo(&desc, variation);

  return(sdnputil_readStatic(pSession,
    pRequest, pResponse, pObjHeader, qualifier, &desc));
}

/* function: sdnpo114_writeObj114 */
TMWTYPES_BOOL TMWDEFS_CALLBACK sdnpo114_writeObj114(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  TMWSESN_TX_DATA *pResponse,
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  TMWTYPES_UCHAR  variation = pObjHeader->variation;
  TMWTYPES_USHORT strLength;
  TMWTYPES_USHORT point;
  TMWTYPES_USHORT index;
  void *pPoint;

  /* Response to a write request is null response so we don't need to
   *  put anything into it here.
   */
  TMWTARG_UNUSED_PARAM(pResponse);
   
  /* ensure the variation is supported for write request */
  if (!_isSupportedVariation(variation) || (variation > 2))
  {
    SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_WRITE_OBJVAR);
    return(TMWDEFS_FALSE);
  }

  /* Loop through the points in the request */
  for(index = 0; index < pObjHeader->numberOfPoints; index++)
  {
    /* Get next point number from request */
    dnputil_getPointNumber(pRxFragment, pObjHeader, index, &point);

    /* Retrieve the string length based on variation */
    if (variation == 1)
    {
      strLength = pRxFragment->pMsgBuf[pRxFragment->offset++];
    }
    else /* String length is 2 bytes */
    {
      tmwtarg_get16(&pRxFragment->pMsgBuf[pRxFragment->offset], &strLength);
      pRxFragment->offset += 2;
    }

    /* Verify length of request */
    if((pRxFragment->offset + strLength) > pRxFragment->msgLength)
    {
      DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_INVALID_SIZE);
      pRxFragment->offset = pRxFragment->msgLength;
      return TMWDEFS_FALSE;
    }

    /* Diagnostics */
    DNPDIAG_SHOW_EXT_STRING_DATA(pSession, point, (pRxFragment->pMsgBuf + pRxFragment->offset), strLength, 0x01, TMWDIAG_ID_RX);

    /* Get point */
    pPoint = sdnpdata_extStrGetPoint(pSDNPSession->pDbHandle, point);
    if(pPoint == TMWDEFS_NULL)
    {
      SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_STR_NOTENABLED);
      pRxFragment->offset = pRxFragment->msgLength;
      return(TMWDEFS_FALSE);
    }

    /* Write the string */
    if(!sdnpdata_extStrWrite(pPoint, pRxFragment->pMsgBuf + pRxFragment->offset, strLength))
    {
      SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_STR_WRITE);
      pRxFragment->offset = pRxFragment->msgLength;
      return(TMWDEFS_FALSE);
    }

    /* Increment to next string */
    pRxFragment->offset = pRxFragment->offset + strLength;
  }

  return(TMWDEFS_TRUE);
}

#if SDNPDATA_SUPPORT_OBJ115
#if SDNPDATA_SUPPORT_ASSIGN
/* function: sdnpo114_assignClass */
TMWTYPES_BOOL TMWDEFS_CALLBACK sdnpo114_assignClass(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  DNPUTIL_OBJECT_HEADER *pObjHeader,
  TMWDEFS_CLASS_MASK classMask)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  TMWTYPES_BOOL status = TMWDEFS_TRUE;
  TMWTYPES_USHORT index;

  status = sdnputil_validateAssignQualifier(pObjHeader, sdnpdata_extStrQuantity(pSDNPSession->pDbHandle));

  for(index = 0; index < pObjHeader->numberOfPoints; index++)
  {
    TMWTYPES_USHORT pointNumber;
    void *pPoint;

    dnputil_getPointNumber(pRxFragment, pObjHeader, index, &pointNumber);

    pPoint = sdnpdata_extStrGetPoint(pSDNPSession->pDbHandle, pointNumber);
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

    if(!sdnpdata_extStrAssignClass(pPoint, classMask))
      status = TMWDEFS_FALSE;
  }

  return(status);
}
#endif
#endif /* SDNPDATA_SUPPORT_OBJ115 */
#endif /* SDNPDATA_SUPPORT_OBJ114 */
