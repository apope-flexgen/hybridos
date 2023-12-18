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

/* file: sdnpo022.c
 * description: DNP Slave functionality for Object 22 Binary Counter Change
 *  Events.
 */
#include "tmwscl/dnp/dnpdiag.h"
#include "tmwscl/dnp/sdnpdiag.h"
#include "tmwscl/utils/tmwtarg.h"

#include "tmwscl/dnp/dnpdefs.h"
#include "tmwscl/dnp/dnpdtime.h"
#include "tmwscl/dnp/sdnpdata.h"
#include "tmwscl/dnp/sdnputil.h"
#include "tmwscl/dnp/sdnpmem.h"
#include "tmwscl/dnp/sdnpunsl.h"
#include "tmwscl/dnp/sdnpo022.h"
#include "tmwscl/dnp/sdnpevnt.h"

#if SDNPDATA_SUPPORT_OBJ22

/* function: _setVariationInfo() */
static void TMWDEFS_LOCAL _setVariationInfo(
  SDNPEVNT_DESC *pDesc, 
  TMWTYPES_UCHAR variation);

/* function: _isSupportedVariation() */
static TMWTYPES_BOOL TMWDEFS_LOCAL _isSupportedVariation(
  TMWTYPES_UCHAR variation)
{
 switch(variation)
 {
  #if SDNPDATA_SUPPORT_OBJ22_V1
  case 1:   
    break;
#endif

#if SDNPDATA_SUPPORT_OBJ22_V2
  case 2:
    break;
#endif

#if SDNPDATA_SUPPORT_OBJ22_V5
  case 5:
    break;
#endif

#if SDNPDATA_SUPPORT_OBJ22_V6
  case 6:
    break;
#endif

  default:
    return(TMWDEFS_FALSE);
  }

  return(TMWDEFS_TRUE);
}

#if SDNPDATA_SUPPORT_EVENT_VAR_POINT
/* function: _getPointAndDefVariation */
static void * TMWDEFS_CALLBACK _getPointAndDefVariation(
  TMWSESN *pSession,
  TMWTYPES_USHORT pointNumber,
  TMWDEFS_CLASS_MASK classMask,
  TMWTYPES_UCHAR *pDefVariation)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  void *pPoint;

  pPoint = sdnpdata_binCntrGetPoint(pSDNPSession->pDbHandle, pointNumber);
  if(pPoint != TMWDEFS_NULL)
    *pDefVariation = sdnpdata_binCntrEventDefVariation(pPoint, classMask);
  else
    *pDefVariation = pSDNPSession->obj22DefaultVariation;

  return(pPoint);
}
#endif

#if SDNPDATA_SUPPORT_EVENT_MODE_POINT
/* function: _getPointAndEventMode */
static TMWDEFS_EVENT_MODE TMWDEFS_CALLBACK _getPointAndEventMode(
  TMWSESN *pSession,
  TMWTYPES_USHORT pointNumber)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;

  void *pPoint = sdnpdata_binCntrGetPoint(pSDNPSession->pDbHandle, pointNumber);
  if(pPoint != TMWDEFS_NULL)
    return sdnpdata_binCntrEventMode(pPoint);
  else
    return pSDNPSession->binaryCounterEventMode;
}
#endif

#if SDNPDATA_SUPPORT_EVENT_SCAN

/* function: _getPointAndClass */
static void * TMWDEFS_CALLBACK _getPointAndClass(
  TMWSESN *pSession,
  TMWTYPES_USHORT pointNumber,
  TMWDEFS_CLASS_MASK *pClassMask)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  void *pPoint;

  pPoint = sdnpdata_binCntrGetPoint(pSDNPSession->pDbHandle, pointNumber);
  if(pPoint != TMWDEFS_NULL)
    *pClassMask = sdnpdata_binCntrEventClass(pPoint);

  return(pPoint);
}

/* function: _changedFunc */
static TMWTYPES_BOOL TMWDEFS_CALLBACK _changedFunc(
  TMWSESN *pSession,
  void *pPoint,
  TMWTYPES_USHORT pointNum,
  TMWDTIME *pTimeStamp)
{
  TMWTYPES_ULONG value;
  TMWTYPES_UCHAR flags;

  if(sdnpdata_binCntrChanged(pPoint, &value, &flags))
  {
    sdnpo022_addEvent(pSession, pointNum, value, flags, pTimeStamp);
    return(TMWDEFS_TRUE);
  }

  return(TMWDEFS_FALSE);
}
#endif

/* function: _initEventDesc */
static void TMWDEFS_LOCAL _initEventDesc(
  TMWSESN *pSession,
  SDNPEVNT_DESC *pDesc)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  pDesc->pSession = pSession;
  pDesc->group = DNPDEFS_OBJ_22_CNTR_EVENTS;
  pDesc->quantity = sdnpdata_binCntrQuantity(pSDNPSession->pDbHandle);
  pDesc->eventMemType = SDNPMEM_OBJECT22_EVENT_TYPE;
  pDesc->pEventList = &pSDNPSession->obj22Events;
  pDesc->eventMode = pSDNPSession->binaryCounterEventMode;
  pDesc->maxEvents = pSDNPSession->binaryCounterMaxEvents;
#if SDNPDATA_SUPPORT_EVENT_SCAN
  pDesc->pGetPointAndClass = _getPointAndClass;
  pDesc->pChangedFunc = _changedFunc;
#endif
#if SDNPDATA_SUPPORT_EVENT_VAR_POINT
  pDesc->pGetPointAndDefault = _getPointAndDefVariation;
  pDesc->pSetVariationInfo = _setVariationInfo;
#endif
#if SDNPDATA_SUPPORT_EVENT_MODE_POINT
  pDesc->pGetPointAndEventMode = _getPointAndEventMode;
#endif
}

#if SDNPDATA_SUPPORT_OBJ22_V1
/* function: _readV1 */
static void TMWDEFS_CALLBACK _readV1(
  TMWSESN *pSession,
  TMWSESN_TX_DATA *pResponse, 
  SDNPEVNT *pEvent)
{
  SDNPEVNT_O022_EVENT *pO22Event = (SDNPEVNT_O022_EVENT *)pEvent;

  /* Diagnostics */
  DNPDIAG_SHOW_BINARY_COUNTER(pSession, pEvent->point, pO22Event->value, pEvent->flags, TMWDEFS_TRUE, TMWDEFS_NULL);

  /* Write flags */
  pResponse->pMsgBuf[pResponse->msgLength++] = pEvent->flags;

  /* Write value */
  tmwtarg_store32(&pO22Event->value, &pResponse->pMsgBuf[pResponse->msgLength]);
  pResponse->msgLength += 4;
}
#endif /* SDNPDATA_SUPPORT_OBJ22_V1 */

#if SDNPDATA_SUPPORT_OBJ22_V2
/* function: _readV2 */
static void TMWDEFS_CALLBACK _readV2(
  TMWSESN *pSession,
  TMWSESN_TX_DATA *pResponse, 
  SDNPEVNT *pEvent)
{
  SDNPEVNT_O022_EVENT *pO22Event = (SDNPEVNT_O022_EVENT *)pEvent;
  TMWTYPES_USHORT tmpValue;

  /* Diagnostics */
  DNPDIAG_SHOW_BINARY_COUNTER(pSession, pEvent->point, pO22Event->value, pEvent->flags, TMWDEFS_TRUE, TMWDEFS_NULL);

  /* Write flags */
  pResponse->pMsgBuf[pResponse->msgLength++] = pEvent->flags;

  /* Write value */
  tmpValue = (TMWTYPES_USHORT)pO22Event->value;
  tmwtarg_store16(&tmpValue, &pResponse->pMsgBuf[pResponse->msgLength]);
  pResponse->msgLength += 2;
}
#endif /* SDNPDATA_SUPPORT_OBJ22_V2 */

#if SDNPDATA_SUPPORT_OBJ22_V5
/* function: _readV5 */
static void TMWDEFS_CALLBACK _readV5(
  TMWSESN *pSession,
  TMWSESN_TX_DATA *pResponse, 
  SDNPEVNT *pEvent)
{
  SDNPEVNT_O022_EVENT *pO22Event = (SDNPEVNT_O022_EVENT *)pEvent;
  TMWTYPES_MS_SINCE_70 msSince70;

  /* Diagnostics */
  DNPDIAG_SHOW_BINARY_COUNTER(pSession, pEvent->point, pO22Event->value, pEvent->flags, TMWDEFS_TRUE, &pEvent->timeStamp);

  /* Write flags */
  pResponse->pMsgBuf[pResponse->msgLength++] = pEvent->flags;

  /* Write value */
  tmwtarg_store32(&pO22Event->value, &pResponse->pMsgBuf[pResponse->msgLength]);
  pResponse->msgLength += 4;

  /* Write 48 bit event time */
  dnpdtime_dateTimeToMSSince70(&msSince70, &pEvent->timeStamp);
  dnpdtime_writeMsSince70(&pResponse->pMsgBuf[pResponse->msgLength], &msSince70);
  pResponse->msgLength += 6;
}
#endif /* SDNPDATA_SUPPORT_OBJ22_V5 */

#if SDNPDATA_SUPPORT_OBJ22_V6
/* function: _readV6 */
static void TMWDEFS_CALLBACK _readV6(
  TMWSESN *pSession,
  TMWSESN_TX_DATA *pResponse, 
  SDNPEVNT *pEvent)
{
  SDNPEVNT_O022_EVENT *pO22Event = (SDNPEVNT_O022_EVENT *)pEvent;
  TMWTYPES_MS_SINCE_70 msSince70;
  TMWTYPES_USHORT tmpValue;

  /* Diagnostics */
  DNPDIAG_SHOW_BINARY_COUNTER(pSession, pEvent->point, pO22Event->value, pEvent->flags, TMWDEFS_TRUE, &pEvent->timeStamp);

  /* Write flags */
  pResponse->pMsgBuf[pResponse->msgLength++] = pEvent->flags;

  /* Write value */
  tmpValue = (TMWTYPES_USHORT)pO22Event->value;
  tmwtarg_store16(&tmpValue, &pResponse->pMsgBuf[pResponse->msgLength]);
  pResponse->msgLength += 2;

  /* Write 48 bit event time */
  dnpdtime_dateTimeToMSSince70(&msSince70, &pEvent->timeStamp);
  dnpdtime_writeMsSince70(&pResponse->pMsgBuf[pResponse->msgLength], &msSince70);
  pResponse->msgLength += 6;
}
#endif /* SDNPDATA_SUPPORT_OBJ22_V6 */

/* function: _setVariationInfo() */
static void TMWDEFS_GLOBAL _setVariationInfo(
  SDNPEVNT_DESC *pDesc,
  TMWTYPES_UCHAR variation)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pDesc->pSession;
  if (!pSDNPSession->supportEventsWithTime)
  {
    /* Switch to the equivalent variation without time */
    if (variation == 5)
    {
      variation = 1;
    }
    else if (variation == 6)
    {
      variation = 2;
    }
  }

  switch(variation)
  {
  default:
    /* Fall through to first one that is supported */
    /* 1 is the preferred default */
#if SDNPDATA_SUPPORT_OBJ22_V1
  case 1:  
    pDesc->variation = 1;
    pDesc->sizeInBytes = 5;
    pDesc->pReadIntoRespFunc = _readV1;
    break;
#endif
#if SDNPDATA_SUPPORT_OBJ22_V2
  case 2:
    pDesc->variation = 2;
    pDesc->sizeInBytes = 3;
    pDesc->pReadIntoRespFunc = _readV2;
    break;
#endif
#if SDNPDATA_SUPPORT_OBJ22_V5
  case 5:
    pDesc->variation = 5;
    pDesc->sizeInBytes = 11;
    pDesc->pReadIntoRespFunc = _readV5;
    break;
#endif
#if SDNPDATA_SUPPORT_OBJ22_V6
  case 6:
    pDesc->variation = 6;
    pDesc->sizeInBytes = 9;
    pDesc->pReadIntoRespFunc = _readV6;
    break;
#endif
  }
}

/* function: sdnpo022_init */
void TMWDEFS_GLOBAL sdnpo022_init(
  TMWSESN *pSession)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;

  sdnpevnt_init(
    &pSDNPSession->obj22Timer, 
    &pSDNPSession->obj22Events);
}

#if SDNPDATA_SUPPORT_EVENT_SCAN
/* function: _scanTimeout */
static void TMWDEFS_CALLBACK _scanTimeout(void *pParam)
{
  TMWSESN *pSession = (TMWSESN *)pParam;
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  SDNPEVNT_DESC desc;

  _initEventDesc(pSession, &desc);
  sdnpevnt_scanForChanges(pSession, &desc);

  if(pSDNPSession->binaryCounterScanPeriod != 0)
  {
    tmwtimer_start(&pSDNPSession->obj22Timer, 
      pSDNPSession->binaryCounterScanPeriod, 
      pSession->pChannel, _scanTimeout, pSession);
  }
}

/* function: sdnpo022_open */
void TMWDEFS_GLOBAL sdnpo022_open(
  TMWSESN *pSession)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;

  if(pSDNPSession->binaryCounterScanPeriod != 0)
  {
    tmwtimer_start(&pSDNPSession->obj22Timer, 
      pSDNPSession->binaryCounterScanPeriod, 
      pSession->pChannel, _scanTimeout, pSession);
  }
}
#endif

/* function: sdnpo022_close */
void TMWDEFS_GLOBAL sdnpo022_close(
  TMWSESN *pSession)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;

#if SDNPDATA_SUPPORT_EVENT_SCAN
  /* Disable event scan timer */
  tmwtimer_cancel(&pSDNPSession->obj22Timer);
#endif

  sdnpevnt_close(&pSDNPSession->obj22Events);
}

/* function: sdnpo022_addEvent */
void TMWDEFS_GLOBAL sdnpo022_addEvent(
  TMWSESN *pSession, 
  TMWTYPES_USHORT point, 
  TMWTYPES_ULONG value, 
  TMWTYPES_UCHAR flags, 
  TMWDTIME *pTimeStamp)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
#if TMWCNFG_SUPPORT_THREADS
  TMWDEFS_RESOURCE_LOCK *pLock = &pSession->pChannel->lock;
#endif
  SDNPEVNT_DESC desc;
  void *pPoint;
  SDNPDATA_ADD_EVENT_VALUE evValue;

  _initEventDesc(pSession, &desc);
  desc.readVariation = pSDNPSession->obj22DefaultVariation;

  TMWTARG_LOCK_SECTION(pLock);

  /* Get point event belongs to */
  pPoint = sdnpdata_binCntrGetPoint(pSDNPSession->pDbHandle, point);
  if(pPoint != TMWDEFS_NULL)
  { 
    TMWDEFS_CLASS_MASK eventClass = sdnpdata_binCntrEventClass(pPoint);
    evValue.ulValue = value;
    sdnpevnt_addEvent(pSession, point, flags, eventClass, pTimeStamp, &desc, &evValue);
  }
  else
  {
    SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_ADD_EVENT);
  }

  TMWTARG_UNLOCK_SECTION(pLock);
}

/* function: sdnpo022_countEvents */
TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpo022_countEvents(
  TMWSESN *pSession, 
  TMWDEFS_CLASS_MASK classMask,
  TMWTYPES_BOOL countAll,
  TMWTYPES_USHORT threshold)
{
  SDNPEVNT_DESC desc;
  _initEventDesc(pSession, &desc);
  return(sdnpevnt_countEvents(pSession, classMask, &desc, countAll, threshold));
}

/* function: sdnpo022_cleanupEvents() */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpo022_cleanupEvents(
  TMWSESN *pSession, 
  TMWTYPES_BOOL deleteEvents)
{
  SDNPEVNT_DESC desc;
  _initEventDesc(pSession, &desc);
  return(sdnpevnt_cleanupEvents(deleteEvents, &desc));
}

/* function: sdnpo022_readObj22 */
SDNPSESN_READ_STATUS TMWDEFS_CALLBACK sdnpo022_readObj22(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRequest,
  TMWSESN_TX_DATA *pResponse, 
  DNPUTIL_OBJECT_HEADER *pObjHeader,
  SDNPSESN_QUAL qualifier)
{

  if(qualifier != SDNPSESN_QUAL_BUILD_RESPONSE)
  {
    return(SDNPSESN_READ_COMPLETE);
  }

  if(pObjHeader->variation == 0)
  {
    return(sdnpo022_readObj22v0ByClass(pSession, pRequest, pResponse, pObjHeader, TMWDEFS_CLASS_MASK_ALL));
  }
  else
  {
    SDNPEVNT_DESC desc;
    SDNPSESN *pSDNPSession = (SDNPSESN *)pSession; 
    TMWTYPES_UCHAR variation = pObjHeader->variation;
  
    if(!_isSupportedVariation(variation))
    {
      pSDNPSession->iin |= DNPDEFS_IIN_OBJECT_UNKNOWN; 
      return (SDNPSESN_READ_FAILED);
    }

#if SDNPDATA_SUPPORT_EVENT_VAR_POINT
    /* Use per point variation even when specific variation is read. */
    if (pSDNPSession->counterSizeVarPerPoint)
    {
      variation = 0;
    }
#endif

    /* If other supported variation */
    _initEventDesc(pSession, &desc);
    desc.readVariation = variation;
    _setVariationInfo(&desc, variation); 

    return(sdnpevnt_readEvents(pSession, pResponse, pObjHeader, TMWDEFS_CLASS_MASK_ALL, &desc));
  }
}

/* function: sdnpo022_readObj22v0ByClass */
SDNPSESN_READ_STATUS TMWDEFS_CALLBACK sdnpo022_readObj22v0ByClass(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRequest,
  TMWSESN_TX_DATA *pResponse, 
  DNPUTIL_OBJECT_HEADER *pObjHeader,
  TMWDEFS_CLASS_MASK classMask)
{
  SDNPEVNT_DESC desc;
  SDNPSESN *pSDNPSession = (SDNPSESN*)pSession;
  TMWTARG_UNUSED_PARAM(pRequest);

  _initEventDesc(pSession, &desc);
  desc.readVariation = 0;

  _setVariationInfo(&desc, pSDNPSession->obj22DefaultVariation); 

#if SDNPDATA_SUPPORT_EVENT_VAR_POINT
  if(pSDNPSession->obj22DefaultVariation == 0)
    desc.variation = 0;
#endif

  return(sdnpevnt_readEvents(pSession, pResponse, pObjHeader, classMask, &desc));
}
#endif /* SDNPDATA_SUPPORT_OBJ22 */
