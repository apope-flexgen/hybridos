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

/* file: sdnpo013.c
 * description: DNP Slave functionality for Object 13 Binary Output Command Event.
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
#include "tmwscl/dnp/sdnpo013.h"

#if SDNPDATA_SUPPORT_OBJ13

/* function: _isSupportedVariation() */
static TMWTYPES_BOOL TMWDEFS_LOCAL _isSupportedVariation(
  TMWTYPES_UCHAR variation)
{
switch(variation)
  {
#if SDNPDATA_SUPPORT_OBJ13_V1
  case 1:
    break;
#endif
#if SDNPDATA_SUPPORT_OBJ13_V2
  case 2:
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

  pPoint = sdnpdata_binOutGetPoint(pSDNPSession->pDbHandle, pointNumber);
  if(pPoint != TMWDEFS_NULL)
    *pDefVariation = sdnpdata_binOutCmdEventDefVariation(pPoint, classMask);
  else
    *pDefVariation = pSDNPSession->obj13DefaultVariation;;

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

  void *pPoint = sdnpdata_binOutGetPoint(pSDNPSession->pDbHandle, pointNumber);
  if(pPoint != TMWDEFS_NULL)
    return sdnpdata_binOutCmdEventMode(pPoint);
  else
    return pSDNPSession->binaryOutCmdEventMode;
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

  pPoint = sdnpdata_binOutGetPoint(pSDNPSession->pDbHandle, pointNumber);

#if SDNPDATA_SUPPORT_OBJ11
  if(pPoint != TMWDEFS_NULL)
    *pClassMask = sdnpdata_binOutCmdEventClass(pPoint);
#endif

  return(pPoint);
}

/* function: _changedFunc */
static TMWTYPES_BOOL TMWDEFS_CALLBACK _changedFunc(
  TMWSESN *pSession,
  void *pPoint,
  TMWTYPES_USHORT pointNum,
  TMWDTIME *pTimeStamp)
{
  TMWTYPES_UCHAR status;

  if(sdnpdata_binOutCmdChanged(pPoint, &status))
  {
    sdnpo013_addEvent(pSession, pointNum, status, pTimeStamp);
    return(TMWDEFS_TRUE);
  }

  return(TMWDEFS_FALSE);
}
#endif

#if SDNPDATA_SUPPORT_OBJ13_V1
/* function: _readV1 */
static void TMWDEFS_CALLBACK _readV1(
  TMWSESN *pSession,
  TMWSESN_TX_DATA *pResponse,
  SDNPEVNT *pEvent)
{
  /* Diagnostics */
  DNPDIAG_SHOW_BINARY_CMD_STATUS(pSession, pEvent->point, pEvent->flags, &pEvent->timeStamp);

  /* Write status, which was stored in flags field */
  pResponse->pMsgBuf[pResponse->msgLength++] = pEvent->flags;
}
#endif /* SDNPDATA_SUPPORT_OBJ13_V1 */

#if SDNPDATA_SUPPORT_OBJ13_V2
/* function: _readV2 */
static void TMWDEFS_CALLBACK _readV2(
  TMWSESN *pSession,
  TMWSESN_TX_DATA *pResponse,
  SDNPEVNT *pEvent)
{
  TMWTYPES_MS_SINCE_70 msSince70;

  /* Diagnostics */
  DNPDIAG_SHOW_BINARY_CMD_STATUS(pSession, pEvent->point, pEvent->flags, &pEvent->timeStamp);

  /* Write status, which was stored in flags field */
  pResponse->pMsgBuf[pResponse->msgLength++] = pEvent->flags;

  /* Write 48 bit event time */
  dnpdtime_dateTimeToMSSince70(&msSince70, &pEvent->timeStamp);
  dnpdtime_writeMsSince70(&pResponse->pMsgBuf[pResponse->msgLength], &msSince70);
  pResponse->msgLength += 6;
}
#endif /* SDNPDATA_SUPPORT_OBJ13_V2 */


/* function: _setVariationInfo() */
static void TMWDEFS_CALLBACK _setVariationInfo(
  SDNPEVNT_DESC *pDesc,
  TMWTYPES_UCHAR variation)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pDesc->pSession;
  if (!pSDNPSession->supportEventsWithTime)
    /* Switch to the variation without time */
    variation = 1;

  switch(variation)
  {
  default:
    /* Fall through to first one that is supported */
#if SDNPDATA_SUPPORT_OBJ13_V1
  case 1:
    pDesc->variation = 1;
    pDesc->sizeInBytes = 1; 
    pDesc->pReadIntoRespFunc = _readV1;
    break;
#endif
#if SDNPDATA_SUPPORT_OBJ13_V2
  case 2:
    pDesc->variation = 2;
    pDesc->sizeInBytes = 7; 
    pDesc->pReadIntoRespFunc = _readV2;
    break;
#endif
  }
}
/* function: _initEventDesc */
static void TMWDEFS_LOCAL _initEventDesc(
  TMWSESN *pSession,
  SDNPEVNT_DESC *pDesc)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  pDesc->pSession = pSession;
  pDesc->group = DNPDEFS_OBJ_13_BIN_CMD_EVENTS;
  pDesc->quantity = sdnpdata_binOutQuantity(pSDNPSession->pDbHandle);
  pDesc->eventMemType = SDNPMEM_OBJECT13_EVENT_TYPE;
  pDesc->pEventList = &pSDNPSession->obj13Events;
  pDesc->eventMode = pSDNPSession->binaryOutCmdEventMode;
  pDesc->maxEvents = pSDNPSession->binaryOutCmdMaxEvents;
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

/* function: sdnpo013_init */
void TMWDEFS_GLOBAL sdnpo013_init(
  TMWSESN *pSession)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;

  sdnpevnt_init(
    &pSDNPSession->obj13Timer,
    &pSDNPSession->obj13Events);
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

  if(pSDNPSession->binaryOutCmdScanPeriod != 0)
  {
    tmwtimer_start(&pSDNPSession->obj13Timer,
      pSDNPSession->binaryOutCmdScanPeriod,
      pSession->pChannel, _scanTimeout, pSession);
  }
}

/* function: sdnpo013_open */
void TMWDEFS_GLOBAL sdnpo013_open(
  TMWSESN *pSession)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;

  if(pSDNPSession->binaryOutCmdScanPeriod != 0)
  {
    tmwtimer_start(&pSDNPSession->obj13Timer,
      pSDNPSession->binaryOutCmdScanPeriod,
      pSession->pChannel, _scanTimeout, pSession);
  }
}
#endif

/* function: sdnpo013_close */
void TMWDEFS_GLOBAL sdnpo013_close(
  TMWSESN *pSession)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;

#if SDNPDATA_SUPPORT_EVENT_SCAN
  /* Disable event scan timer */
  tmwtimer_cancel(&pSDNPSession->obj13Timer);
#endif

  sdnpevnt_close(&pSDNPSession->obj13Events);
}

/* function: sdnpo013_addEvent */
void TMWDEFS_GLOBAL sdnpo013_addEvent(
  TMWSESN *pSession,
  TMWTYPES_USHORT point,
  TMWTYPES_UCHAR status,
  TMWDTIME *pTimeStamp)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
#if TMWCNFG_SUPPORT_THREADS
  TMWDEFS_RESOURCE_LOCK *pLock = &pSession->pChannel->lock;
#endif
  SDNPEVNT_DESC desc;
  void *pPoint;

  _initEventDesc(pSession, &desc);
  desc.readVariation = pSDNPSession->obj13DefaultVariation;

  TMWTARG_LOCK_SECTION(pLock);

  /* Get point event belongs to */
  pPoint = sdnpdata_binOutGetPoint(pSDNPSession->pDbHandle, point);
#if SDNPDATA_SUPPORT_OBJ11
  if(pPoint != TMWDEFS_NULL)
  {
    TMWDEFS_CLASS_MASK eventClass = sdnpdata_binOutCmdEventClass(pPoint);
    /* use flags field to hold command status octet */
    sdnpevnt_addEvent(pSession, point, status, eventClass, pTimeStamp, &desc, TMWDEFS_NULL);
  }
  else
  {
    SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_ADD_EVENT);
  }
#endif

  TMWTARG_UNLOCK_SECTION(pLock);
}

/* function: sdnpo013_countEvents */
TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpo013_countEvents(
  TMWSESN *pSession,
  TMWDEFS_CLASS_MASK classMask,
  TMWTYPES_BOOL countAll,
  TMWTYPES_USHORT threshold)
{
  SDNPEVNT_DESC desc;
  _initEventDesc(pSession, &desc);
  return(sdnpevnt_countEvents(pSession, classMask, &desc, countAll, threshold));
}

/* function: sdnpo013_cleanupEvents() */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpo013_cleanupEvents(
  TMWSESN *pSession,
  TMWTYPES_BOOL deleteEvents)
{
  SDNPEVNT_DESC desc;
  _initEventDesc(pSession, &desc);
  return(sdnpevnt_cleanupEvents(deleteEvents, &desc));
}

/* function: sdnpo013_readObj13 */
SDNPSESN_READ_STATUS TMWDEFS_CALLBACK sdnpo013_readObj13(
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
    return(sdnpo013_readObj13v0ByClass(pSession, pRequest, pResponse, pObjHeader, TMWDEFS_CLASS_MASK_ALL));
  }
  else
  {
    SDNPEVNT_DESC desc;
    TMWTYPES_UCHAR variation = pObjHeader->variation;
    SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;

    if(!_isSupportedVariation(variation))
    {
      pSDNPSession->iin |= DNPDEFS_IIN_OBJECT_UNKNOWN;
      return (SDNPSESN_READ_FAILED);
    }

    /* If variations 1 or 2 */
    _initEventDesc(pSession, &desc);
    desc.readVariation = variation;
    _setVariationInfo(&desc, variation);

    return(sdnpevnt_readEvents(pSession, pResponse, pObjHeader, TMWDEFS_CLASS_MASK_ALL, &desc));
  }
}

/* function: sdnpo013_readObj13v0ByClass */
SDNPSESN_READ_STATUS TMWDEFS_CALLBACK sdnpo013_readObj13v0ByClass(
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

  _setVariationInfo(&desc, pSDNPSession->obj13DefaultVariation);

#if SDNPDATA_SUPPORT_EVENT_VAR_POINT
  if(pSDNPSession->obj13DefaultVariation == 0)
    desc.variation = 0;
#endif

  return(sdnpevnt_readEvents(pSession, pResponse, pObjHeader, classMask, &desc));
}

#endif /* SDNPDATA_SUPPORT_OBJ13 */
