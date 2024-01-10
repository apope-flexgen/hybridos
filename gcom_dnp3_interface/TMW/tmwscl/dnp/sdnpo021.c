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

/* file: sdnpo021.c
 * description: DNP Slave functionality for Object 21 Frozen Counters
 */
#include "tmwscl/dnp/dnpdiag.h"
#include "tmwscl/dnp/dnpdefs.h"
#include "tmwscl/dnp/dnputil.h"
#include "tmwscl/dnp/sdnpo021.h"
#include "tmwscl/dnp/sdnpdata.h"
#include "tmwscl/dnp/sdnputil.h"
#include "tmwscl/dnp/dnpdtime.h"

#include "tmwscl/utils/tmwtarg.h"

#if SDNPDATA_SUPPORT_OBJ21

/* forward declarations */
static void TMWDEFS_GLOBAL _switchVariationInfo(
  SDNPUTIL_STATIC_DESC *pDesc, 
  TMWTYPES_UCHAR variation);

/* function: _isSupportedVariation() */
static TMWTYPES_BOOL TMWDEFS_LOCAL _isSupportedVariation(
  TMWTYPES_UCHAR variation)
{
 switch(variation)
  {
#if SDNPDATA_SUPPORT_OBJ21_V1
  case 1:
    break;
#endif

#if SDNPDATA_SUPPORT_OBJ21_V2
  case 2:
    break;
#endif

#if SDNPDATA_SUPPORT_OBJ21_V5
  case 5:
    break;
#endif

#if SDNPDATA_SUPPORT_OBJ21_V6
  case 6:
    break;
#endif

#if SDNPDATA_SUPPORT_OBJ21_V9
  case 9:
    break;
#endif

#if SDNPDATA_SUPPORT_OBJ21_V10
  case 10:
    break;
#endif

  default:
    return (TMWDEFS_FALSE);
  }
return(TMWDEFS_TRUE);
}

#if SDNPDATA_SUPPORT_STATIC_VAR_POINT
/* function: _flagNominalCheck 
 * purpose: Check to see if variation we are switching to requires
 *  flags to be nominal and flags are nominal.  If necessary change
 *  variation to one that supports flags.
 * arguments:
 *  pPoint - point to be read
 *  variation - variation we are trying to switch to
 * returns:
 *  void
 */
static void TMWDEFS_LOCAL _flagNominalCheck(
  void *pPoint,
  TMWTYPES_UCHAR *pVariation)     
{
  TMWDTIME timeOfFreeze;
  TMWTYPES_ULONG value;
  TMWTYPES_UCHAR flags;

  if((*pVariation == 9) || (*pVariation == 10))
  { 
    /* Read current value */
    sdnpdata_frznCntrRead(pPoint, &value, &flags, &timeOfFreeze);

    /* Check to see whether flags are in nominal state */
    if((flags & 0x7f) != DNPDEFS_DBAS_FLAG_ON_LINE)
    {
      /* change variation from 9->1 10->2 */
      *pVariation = *pVariation - 8;
    }
  }
}

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
    TMWTYPES_UCHAR variation;

    if(pDesc->alreadySwitchedVariation)
      return(TMWDEFS_FALSE);

    variation = sdnpdata_frznCntrDefVariation(pPoint); 
    if((variation != currentVariation) 
      && (_isSupportedVariation(variation)))
    {     
      /* Determine which variation should be used depending on flags */
      _flagNominalCheck(pPoint, &variation);
      _switchVariationInfo(pDesc, variation);

      /* if variation is still different than the current variation return true */
      if(variation != currentVariation)
        return(TMWDEFS_TRUE);
    }
  }
  return(TMWDEFS_FALSE);
}
#endif

#if SDNPDATA_SUPPORT_OBJ21_V1
/* function: _readv1 */
static SDNPSESN_READ_STATUS TMWDEFS_CALLBACK _readv1(
  TMWSESN *pSession,
  TMWSESN_TX_DATA *pResponse,
  TMWTYPES_USHORT messageIndex,
  TMWTYPES_USHORT pointNum,
  void *pPoint,
  SDNPUTIL_STATIC_DESC *pDesc)
{
  TMWDTIME timeOfFreeze;
  TMWTYPES_ULONG value;
  TMWTYPES_UCHAR flags;

  TMWTARG_UNUSED_PARAM(messageIndex);

#if SDNPDATA_SUPPORT_STATIC_VAR_POINT
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  /* If session default variation is zero, check the default variation
   * for this specific point 
   */
  if((pSDNPSession->obj21DefaultVariation == 0) 
     &&(_switchPointDefVariation(pDesc, pPoint, 1)))
  {
    return(SDNPSESN_READ_NEW_VARIATION);  
  }
#endif

  /* Read current value */
  sdnpdata_frznCntrRead(pPoint, &value, &flags, &timeOfFreeze);

  /* Diagnostics */
  DNPDIAG_SHOW_FROZEN_COUNTER(pSession, pointNum, value, flags, TMWDEFS_FALSE, TMWDEFS_NULL);

  /* Store flags */
  pResponse->pMsgBuf[pResponse->msgLength++] = flags;

  /* Store value */
  tmwtarg_store32(&value, &pResponse->pMsgBuf[pResponse->msgLength]);
  pResponse->msgLength += 4;

  pDesc->alreadySwitchedVariation = TMWDEFS_FALSE;
  return(SDNPSESN_READ_COMPLETE);
}
#endif

#if SDNPDATA_SUPPORT_OBJ21_V2
/* function: _readv2 */
static SDNPSESN_READ_STATUS TMWDEFS_CALLBACK _readv2(
  TMWSESN *pSession,
  TMWSESN_TX_DATA *pResponse,
  TMWTYPES_USHORT messageIndex,
  TMWTYPES_USHORT pointNum,
  void *pPoint,
  SDNPUTIL_STATIC_DESC *pDesc)
{
  TMWDTIME timeOfFreeze;
  TMWTYPES_USHORT svalue;
  TMWTYPES_ULONG value;
  TMWTYPES_UCHAR flags;

  TMWTARG_UNUSED_PARAM(messageIndex);

#if SDNPDATA_SUPPORT_STATIC_VAR_POINT
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  /* If session default variation is zero, check the default variation
   * for this specific point 
   */
  if((pSDNPSession->obj21DefaultVariation == 0) 
     &&(_switchPointDefVariation(pDesc, pPoint, 2)))
  {
    return(SDNPSESN_READ_NEW_VARIATION);  
  }
#endif

  /* Read current value */
  sdnpdata_frznCntrRead(pPoint, &value, &flags, &timeOfFreeze);

  /* Diagnostics */
  DNPDIAG_SHOW_FROZEN_COUNTER(pSession, pointNum, value, flags, TMWDEFS_FALSE, TMWDEFS_NULL);

  /* Store flags */
  pResponse->pMsgBuf[pResponse->msgLength++] = flags;

  /* Store value */
  svalue = (TMWTYPES_USHORT)value;
  tmwtarg_store16(&svalue, &pResponse->pMsgBuf[pResponse->msgLength]);
  pResponse->msgLength += 2;

  pDesc->alreadySwitchedVariation = TMWDEFS_FALSE;
  return(SDNPSESN_READ_COMPLETE);
}
#endif

#if SDNPDATA_SUPPORT_OBJ21_V5
/* function: _readv5 */
static SDNPSESN_READ_STATUS TMWDEFS_CALLBACK _readv5(
  TMWSESN *pSession,
  TMWSESN_TX_DATA *pResponse,
  TMWTYPES_USHORT messageIndex,
  TMWTYPES_USHORT pointNum,
  void *pPoint,
  SDNPUTIL_STATIC_DESC *pDesc)
{
  TMWTYPES_MS_SINCE_70 msSince70;
  TMWDTIME timeOfFreeze;
  TMWTYPES_ULONG value;
  TMWTYPES_UCHAR flags;

  TMWTARG_UNUSED_PARAM(messageIndex);

#if SDNPDATA_SUPPORT_STATIC_VAR_POINT
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  /* If session default variation is zero, check the default variation
   * for this specific point 
   */
  if((pSDNPSession->obj21DefaultVariation == 0) 
     &&(_switchPointDefVariation(pDesc, pPoint, 5)))
  {
    return(SDNPSESN_READ_NEW_VARIATION);  
  }
#endif

  /* Read current value */
  sdnpdata_frznCntrRead(pPoint, &value, &flags, &timeOfFreeze);

  /* Diagnostics */
  DNPDIAG_SHOW_FROZEN_COUNTER(pSession, pointNum, value, flags, TMWDEFS_FALSE, &timeOfFreeze);

  /* Store flags */
  pResponse->pMsgBuf[pResponse->msgLength++] = flags;

  /* Store value */
  tmwtarg_store32(&value, &pResponse->pMsgBuf[pResponse->msgLength]);
  pResponse->msgLength += 4;

  /* Write 48 bit event time */
  dnpdtime_dateTimeToMSSince70(&msSince70, &timeOfFreeze);
  dnpdtime_writeMsSince70(&pResponse->pMsgBuf[pResponse->msgLength], &msSince70);
  pResponse->msgLength += 6;

  pDesc->alreadySwitchedVariation = TMWDEFS_FALSE;
  return(SDNPSESN_READ_COMPLETE);
}
#endif

#if SDNPDATA_SUPPORT_OBJ21_V6
/* function: _readv6 */
static SDNPSESN_READ_STATUS TMWDEFS_CALLBACK _readv6(
  TMWSESN *pSession,
  TMWSESN_TX_DATA *pResponse,
  TMWTYPES_USHORT messageIndex,
  TMWTYPES_USHORT pointNum,
  void *pPoint,
  SDNPUTIL_STATIC_DESC *pDesc)
{
  TMWTYPES_MS_SINCE_70 msSince70;
  TMWDTIME timeOfFreeze;
  TMWTYPES_USHORT svalue;
  TMWTYPES_ULONG value;
  TMWTYPES_UCHAR flags;

  TMWTARG_UNUSED_PARAM(messageIndex);

#if SDNPDATA_SUPPORT_STATIC_VAR_POINT
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  /* If session default variation is zero, check the default variation
   * for this specific point 
   */
  if((pSDNPSession->obj21DefaultVariation == 0) 
     &&(_switchPointDefVariation(pDesc, pPoint, 6)))
  {
    return(SDNPSESN_READ_NEW_VARIATION);  
  }
#endif

  /* Read current value */
  sdnpdata_frznCntrRead(pPoint, &value, &flags, &timeOfFreeze);

  /* Diagnostics */
  DNPDIAG_SHOW_FROZEN_COUNTER(pSession, pointNum, value, flags, TMWDEFS_FALSE, &timeOfFreeze);

  /* Store flags */
  pResponse->pMsgBuf[pResponse->msgLength++] = flags;

  /* Store value */
  svalue = (TMWTYPES_USHORT)value;
  tmwtarg_store16(&svalue, &pResponse->pMsgBuf[pResponse->msgLength]);
  pResponse->msgLength += 2;

  /* Write 48 bit event time */
  dnpdtime_dateTimeToMSSince70(&msSince70, &timeOfFreeze);
  dnpdtime_writeMsSince70(&pResponse->pMsgBuf[pResponse->msgLength], &msSince70);
  pResponse->msgLength += 6;

  pDesc->alreadySwitchedVariation = TMWDEFS_FALSE;
  return(SDNPSESN_READ_COMPLETE);
}
#endif

#if SDNPDATA_SUPPORT_OBJ21_V9
/* function: _readv9 */
static SDNPSESN_READ_STATUS TMWDEFS_CALLBACK _readv9(
  TMWSESN *pSession,
  TMWSESN_TX_DATA *pResponse,
  TMWTYPES_USHORT messageIndex,
  TMWTYPES_USHORT pointNum,
  void *pPoint,
  SDNPUTIL_STATIC_DESC *pDesc)
{
  TMWDTIME timeOfFreeze;
  TMWTYPES_ULONG value;
  TMWTYPES_UCHAR flags;

  TMWTARG_UNUSED_PARAM(messageIndex);

#if SDNPDATA_SUPPORT_STATIC_VAR_POINT
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  /* If session default variation is zero, check the default variation
   * for this specific point 
   */
  if((pSDNPSession->obj21DefaultVariation == 0) 
     &&(_switchPointDefVariation(pDesc, pPoint, 9)))
  {
    return(SDNPSESN_READ_NEW_VARIATION);  
  }
#endif

  /* Read current value */
  sdnpdata_frznCntrRead(pPoint, &value, &flags, &timeOfFreeze);

#if SDNPDATA_SUPPORT_OBJ21_V1
  /* Check to see whether flags are in nominal state. If not variation 2 must
   * be used.
   */
  if((flags & 0x7f) != DNPDEFS_DBAS_FLAG_ON_LINE)
  {
    /* Change the variation to the one with flags */
    _switchVariationInfo(pDesc, 1);

    return(SDNPSESN_READ_NEW_VARIATION);
  }
#endif

  /* Diagnostics */
  DNPDIAG_SHOW_FROZEN_COUNTER(pSession, pointNum, value, flags, TMWDEFS_FALSE, TMWDEFS_NULL);

  /* Store value */
  tmwtarg_store32(&value, &pResponse->pMsgBuf[pResponse->msgLength]);
  pResponse->msgLength += 4;

  pDesc->alreadySwitchedVariation = TMWDEFS_FALSE;
  return(SDNPSESN_READ_COMPLETE);
}
#endif

#if SDNPDATA_SUPPORT_OBJ21_V10
/* function: _readv10 */
static SDNPSESN_READ_STATUS TMWDEFS_CALLBACK _readv10(
  TMWSESN *pSession,
  TMWSESN_TX_DATA *pResponse,
  TMWTYPES_USHORT messageIndex,
  TMWTYPES_USHORT pointNum,
  void *pPoint,
  SDNPUTIL_STATIC_DESC *pDesc)
{
  TMWDTIME timeOfFreeze;
  TMWTYPES_USHORT svalue;
  TMWTYPES_ULONG value;
  TMWTYPES_UCHAR flags;

  TMWTARG_UNUSED_PARAM(messageIndex);

#if SDNPDATA_SUPPORT_STATIC_VAR_POINT
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  /* If session default variation is zero, check the default variation
   * for this specific point 
   */
  if((pSDNPSession->obj21DefaultVariation == 0) 
     &&(_switchPointDefVariation(pDesc, pPoint, 10)))
  {
    return(SDNPSESN_READ_NEW_VARIATION);  
  }
#endif

  /* Read current value */
  sdnpdata_frznCntrRead(pPoint, &value, &flags, &timeOfFreeze);

#if SDNPDATA_SUPPORT_OBJ21_V2
  /* Check to see whether flags are in nominal state. If not variation 2 must
   * be used.
   */
  if((flags & 0x7f) != DNPDEFS_DBAS_FLAG_ON_LINE)
  {
    /* Change the variation to the one with flags */
    _switchVariationInfo(pDesc,2);

    return(SDNPSESN_READ_NEW_VARIATION);
  }
#else
  TMWTARG_UNUSED_PARAM(pDesc);
#endif

  /* Diagnostics */
  DNPDIAG_SHOW_FROZEN_COUNTER(pSession, pointNum, value, flags, TMWDEFS_FALSE, TMWDEFS_NULL);

  /* Store value */
  svalue = (TMWTYPES_USHORT)value;
  tmwtarg_store16(&svalue, &pResponse->pMsgBuf[pResponse->msgLength]);
  pResponse->msgLength += 2;

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

    /* 9 is the preferred default */
#if SDNPDATA_SUPPORT_OBJ21_V9
  case 9:
    pDesc->variation = 9;
    pDesc->pReadIntoRespFunc = _readv9;
    pDesc->sizeInBytes = 4;
    break;
#endif
#if SDNPDATA_SUPPORT_OBJ21_V1
  case 1:
    pDesc->variation = 1;
    pDesc->pReadIntoRespFunc = _readv1;
    pDesc->sizeInBytes = 5; 
    break;
#endif
#if SDNPDATA_SUPPORT_OBJ21_V2
  case 2:
    pDesc->variation = 2;
    pDesc->pReadIntoRespFunc = _readv2;
    pDesc->sizeInBytes = 3;
    break;
#endif
#if SDNPDATA_SUPPORT_OBJ21_V5
  case 5:
    pDesc->variation = 5;
    pDesc->pReadIntoRespFunc = _readv5;
    pDesc->sizeInBytes = 11;
    break;
#endif
#if SDNPDATA_SUPPORT_OBJ21_V6
  case 6:
    pDesc->variation = 6;
    pDesc->pReadIntoRespFunc = _readv6;
    pDesc->sizeInBytes = 9;
    break;
#endif
#if SDNPDATA_SUPPORT_OBJ21_V10
  case 10:
    pDesc->variation = 10;
    pDesc->pReadIntoRespFunc = _readv10;
    pDesc->sizeInBytes = 2;
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

/* function: sdnpo021_readObj21v0 */
SDNPSESN_READ_STATUS TMWDEFS_CALLBACK sdnpo021_readObj21(
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
    variation = pSDNPSession->obj21DefaultVariation;
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
    if (pSDNPSession->counterSizeVarPerPoint)
    {
      variation = 0;
    }
#endif
  }

  desc.group = DNPDEFS_OBJ_21_FROZEN_CNTRS;
  desc.readVariation = variation; 
  desc.readBits = TMWDEFS_FALSE;
  desc.pQuantityFunc = sdnpdata_frznCntrQuantity;
  desc.pGetPointFunc = sdnpdata_frznCntrGetPoint; 
  desc.alreadySwitchedVariation = TMWDEFS_FALSE;
#if SDNPDATA_SUPPORT_CLASS0_POINT
  desc.pIsInClass0Func = sdnpdata_frznCntrIsClass0; 
#endif

  _setVariationInfo(&desc, variation);

  return(sdnputil_readStatic(pSession,
    pRequest, pResponse, pObjHeader, qualifier, &desc));
}

#if SDNPDATA_SUPPORT_OBJ23
#if SDNPDATA_SUPPORT_ASSIGN
TMWTYPES_BOOL TMWDEFS_CALLBACK sdnpo021_assignClass(
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
  status = sdnputil_validateAssignQualifier(pObjHeader, sdnpdata_frznCntrQuantity(pSDNPSession->pDbHandle));
  
  for(i = 0; i < pObjHeader->numberOfPoints; i++)
  {
    void *pPoint;
    TMWTYPES_USHORT pointNumber;
    dnputil_getPointNumber(pRxFragment, pObjHeader, i, &pointNumber);

    pPoint = sdnpdata_frznCntrGetPoint(pSDNPSession->pDbHandle, pointNumber);
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

    if(!sdnpdata_frznCntrAssignClass(pPoint, classMask))  
      status = TMWDEFS_FALSE;
  }
    
  return(status);
}
#endif
#endif /* SDNPDATA_SUPPORT_OBJ23 */
#endif /* SDNPDATA_SUPPORT_OBJ21 */
