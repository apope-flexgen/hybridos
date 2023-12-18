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

/* file: sdnpo115.c
 * description: DNP Slave functionality for Object 115 Extened Octet String Events.
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
#include "tmwscl/dnp/sdnpo115.h"

#if SDNPDATA_SUPPORT_OBJ115

/* function: _setVariationInfo() */
static void TMWDEFS_LOCAL _setVariationInfo(
  SDNPEVNT_DESC *pDesc, 
  TMWTYPES_UCHAR variation);

/* function: _isSupportedVariation() */
static TMWTYPES_BOOL TMWDEFS_LOCAL _isSupportedVariation(
  TMWTYPES_UCHAR variation)
{
  TMWTYPES_BOOL variationSupported = TMWDEFS_FALSE;
  switch(variation)
  {
  case 1:
    variationSupported = SDNPDATA_SUPPORT_OBJ115_V1;
    break;
  case 2:
    variationSupported = SDNPDATA_SUPPORT_OBJ115_V2;
    break;
  case 3:
    variationSupported = SDNPDATA_SUPPORT_OBJ115_V3;
    break;
  case 4:
    variationSupported = SDNPDATA_SUPPORT_OBJ115_V4;
    break;
  default:
    break;
  }
  return(variationSupported);
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

  pPoint = sdnpdata_extStrGetPoint(pSDNPSession->pDbHandle, pointNumber);
  if(pPoint != TMWDEFS_NULL)
    *pDefVariation = sdnpdata_extStrEventDefVariation(pPoint, classMask);
  else
    *pDefVariation = pSDNPSession->obj115DefaultVariation;

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

  void *pPoint = sdnpdata_extStrGetPoint(pSDNPSession->pDbHandle, pointNumber);
  if(pPoint != TMWDEFS_NULL)
    return sdnpdata_extStrEventMode(pPoint);
  else
    return pSDNPSession->extStringEventMode;
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

  pPoint = sdnpdata_extStrGetPoint(pSDNPSession->pDbHandle, pointNumber);
  if(pPoint != TMWDEFS_NULL)
    *pClassMask = sdnpdata_extStrEventClass(pPoint);

  return(pPoint);
}

/* function: _changedFunc */
static TMWTYPES_BOOL TMWDEFS_CALLBACK _changedFunc(
  TMWSESN *pSession,
  void *pPoint,
  TMWTYPES_USHORT pointNum,
  TMWDTIME *pTimeStamp)
{ 
  TMWTYPES_USHORT length;
  TMWTYPES_UCHAR flags;

#if SDNPDATA_SUPPORT_OBJ114_MIN_STACK
  /* If min stack, just get up to 2048 octets. If string is larger
   * then call to get a pointer to it.
   */
  TMWTYPES_UCHAR *pExtString;
  TMWTYPES_UCHAR buf[2048];
  pExtString = buf;
#else
  TMWTYPES_UCHAR buf[DNPCNFG_MAX_EXT_STRING_LENGTH];
#endif

  if(sdnpdata_extStrChanged(pPoint, sizeof(buf), buf, &length, &flags))
  { 
#if SDNPDATA_SUPPORT_OBJ114_MIN_STACK
    if (length < 2048)
    {
      sdnpo115_addEvent(pSession, pointNum, pExtString, length, flags, pTimeStamp);
    }
    else
    {
      pExtString = sdnpdata_extStrGetPtr(pPoint, &length, &flags);
      sdnpo115_addEvent(pSession, pointNum, pExtString, length, flags, pTimeStamp);
      sdnpdata_extStrRelease(pPoint);
    }
#else
    sdnpo115_addEvent(pSession, pointNum, buf, length, flags, pTimeStamp);
#endif
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
  pDesc->group = DNPDEFS_OBJ_115_EXT_STR_EVENTS;
  pDesc->quantity = sdnpdata_extStrQuantity(pSDNPSession->pDbHandle);
  pDesc->eventMemType = SDNPMEM_OBJECT115_EVENT_TYPE;
  pDesc->pEventList = &pSDNPSession->obj115Events;
  pDesc->eventMode = pSDNPSession->extStringEventMode;
  pDesc->maxEvents = pSDNPSession->extStringMaxEvents;
#if SDNPDATA_SUPPORT_OBJ110
  pDesc->truncateStrings = pSDNPSession->truncateStrings;
#endif
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

#if SDNPDATA_SUPPORT_OBJ115_V1
static TMWTYPES_BOOL TMWDEFS_LOCAL _readV1(
  TMWSESN *pSession,
  TMWSESN_TX_DATA *pResponse, 
  SDNPEVNT *pEvent)
{
  SDNPEVNT_O115_EVENT *p115Event = (SDNPEVNT_O115_EVENT *)pEvent;
  TMWTYPES_USHORT length = p115Event->strLength;
  TMWTYPES_UCHAR flags = pEvent->flags;
  TMWTYPES_MS_SINCE_70 msSince70;

  if (length > DNPDEFS_MAX_STRING_LENGTH)
  {
    length = DNPDEFS_MAX_STRING_LENGTH;
    flags |= DNPDEFS_DBAS_FLAG_OVERFLOW;
  }

  /* If this would not fit as first object in a fragment, truncate it. 
   * Allowing for header and other data
   */
  if (length > (pResponse->maxLength - 17))
  {
    length = pResponse->maxLength - 17;
    flags |= DNPDEFS_DBAS_FLAG_OVERFLOW;
  }
  
  /* If this would fit as the first event in a fragment return false so it can be put in the next fragment */
  if (pResponse->msgLength + length + 8 > pResponse->maxLength)
  {
    return(TMWDEFS_FALSE);
  }

  /* Diagnostics */
  DNPDIAG_SHOW_EXT_STRING_EVENT(pSession, pEvent->point, p115Event->strBuf, length, flags, TMWDEFS_NULL);

  /* Store the flag byte */
  pResponse->pMsgBuf[pResponse->msgLength++] = flags;

  /* Store the time-of-occurance as a 48 bit event time */
  dnpdtime_dateTimeToMSSince70(&msSince70, &pEvent->timeStamp);
  dnpdtime_writeMsSince70(&pResponse->pMsgBuf[pResponse->msgLength], &msSince70);
  pResponse->msgLength += 6;

  /* Store the length as a single byte */
  pResponse->pMsgBuf[pResponse->msgLength++] = (TMWTYPES_UCHAR)length;

  /* Store event data into response */
  memcpy(pResponse->pMsgBuf + pResponse->msgLength, p115Event->strBuf, length);
  pResponse->msgLength = (TMWTYPES_USHORT)(pResponse->msgLength + length);
  return(TMWDEFS_TRUE);
}
#endif /* SDNPDATA_SUPPORT_OBJ115_V1 */

#if SDNPDATA_SUPPORT_OBJ115_V2
static TMWTYPES_BOOL TMWDEFS_LOCAL _readV2(
  TMWSESN *pSession,
  TMWSESN_TX_DATA *pResponse, 
  SDNPEVNT *pEvent)
{
  SDNPEVNT_O115_EVENT *p115Event = (SDNPEVNT_O115_EVENT *)pEvent;
  TMWTYPES_USHORT length = p115Event->strLength;
  TMWTYPES_UCHAR flags = pEvent->flags;
  TMWTYPES_MS_SINCE_70 msSince70;

  /* If this would not fit as first object in a fragment, truncate it.
   * Allowing for header and other data
   */
  if (length > (pResponse->maxLength - 18))
  {
    length = pResponse->maxLength - 18;
    flags |= DNPDEFS_DBAS_FLAG_OVERFLOW;
  } 

  /* If this would fit as the first event in a fragment return false so it can be put in the next fragment */
  if (pResponse->msgLength + length + 8 > pResponse->maxLength)
  {
    return(TMWDEFS_FALSE);
  }

  /* Diagnostics */
  DNPDIAG_SHOW_EXT_STRING_EVENT(pSession, pEvent->point, p115Event->strBuf, length, flags, &pEvent->timeStamp);

  /* Store the flag byte */
  pResponse->pMsgBuf[pResponse->msgLength++] = flags;

  /* Store the time-of-occurance as a 48 bit event time */
  dnpdtime_dateTimeToMSSince70(&msSince70, &pEvent->timeStamp);
  dnpdtime_writeMsSince70(&pResponse->pMsgBuf[pResponse->msgLength], &msSince70);
  pResponse->msgLength += 6;

  /* Store the length in 2 bytes */
  tmwtarg_store16(&length, &pResponse->pMsgBuf[pResponse->msgLength]);
  pResponse->msgLength += 2;
 
  /* Store event data into response */
  memcpy(pResponse->pMsgBuf + pResponse->msgLength, p115Event->strBuf, length);
  pResponse->msgLength = (TMWTYPES_USHORT)(pResponse->msgLength + length);

  return TMWDEFS_TRUE;
}
#endif /* SDNPDATA_SUPPORT_OBJ115_V2 */

#if SDNPDATA_SUPPORT_OBJ115_V3
static TMWTYPES_BOOL TMWDEFS_LOCAL _readV3(
  TMWSESN *pSession,
  TMWSESN_TX_DATA *pResponse, 
  SDNPEVNT *pEvent)
{
  SDNPEVNT_O115_EVENT *p115Event = (SDNPEVNT_O115_EVENT *)pEvent;
  TMWTYPES_USHORT length = p115Event->strLength;
  TMWTYPES_UCHAR flags = pEvent->flags;

  if (length > DNPDEFS_MAX_STRING_LENGTH)
  {
    length = DNPDEFS_MAX_STRING_LENGTH;
    flags |= DNPDEFS_DBAS_FLAG_OVERFLOW;
  }

  /* If this would not fit as first object in a fragment, truncate it.
   * Allowing for header and other data
   */
  if (length > (pResponse->maxLength - 11))
  {
    length = pResponse->maxLength - 11;
    flags |= DNPDEFS_DBAS_FLAG_OVERFLOW;
  }

  /* If this would fit as the first event in a fragment return false so it can be put in the next fragment */
  if (pResponse->msgLength + length + 2 > pResponse->maxLength)
  {
    return(TMWDEFS_FALSE);
  }

  /* Diagnostics */
  DNPDIAG_SHOW_EXT_STRING_EVENT(pSession, pEvent->point, p115Event->strBuf, length, flags, TMWDEFS_NULL);

  /* Store the flag byte */
  pResponse->pMsgBuf[pResponse->msgLength++] = flags;

  /* Store the length as a single byte */
  pResponse->pMsgBuf[pResponse->msgLength++] = (TMWTYPES_UCHAR)length;

  /* Store event data into response */
  memcpy(pResponse->pMsgBuf + pResponse->msgLength, p115Event->strBuf, length);
  pResponse->msgLength = (TMWTYPES_USHORT)(pResponse->msgLength + length);

  return TMWDEFS_TRUE;
}
#endif /* SDNPDATA_SUPPORT_OBJ115_V3 */

#if SDNPDATA_SUPPORT_OBJ115_V4
static TMWTYPES_BOOL TMWDEFS_LOCAL _readV4(
  TMWSESN *pSession,
  TMWSESN_TX_DATA *pResponse, 
  SDNPEVNT *pEvent)
{
  SDNPEVNT_O115_EVENT *p115Event = (SDNPEVNT_O115_EVENT *)pEvent;
  TMWTYPES_USHORT length = p115Event->strLength;
  TMWTYPES_UCHAR flags = pEvent->flags;

  /* If this would not fit as first object in a fragment, truncate it.
   * Allowing for header and other data
   */
  if (length > (pResponse->maxLength - 12))
  {
    length = pResponse->maxLength - 12;
    flags |= DNPDEFS_DBAS_FLAG_OVERFLOW;
  }

  /* If this would fit as the first event in a fragment return false so it can be put in the next fragment */
  if (pResponse->msgLength + length + 2 > pResponse->maxLength)
  {
    return(TMWDEFS_FALSE);
  }

  /* Diagnostics */
  DNPDIAG_SHOW_EXT_STRING_EVENT(pSession, pEvent->point, p115Event->strBuf, length, flags, &pEvent->timeStamp);

  /* Store the flag byte */
  pResponse->pMsgBuf[pResponse->msgLength++] = flags;

  /* Store the length in 2 bytes */
  tmwtarg_store16(&length, &pResponse->pMsgBuf[pResponse->msgLength]);
  pResponse->msgLength += 2;
 
  /* Store event data into response */
  memcpy(pResponse->pMsgBuf + pResponse->msgLength, p115Event->strBuf, length);
  pResponse->msgLength = (TMWTYPES_USHORT)(pResponse->msgLength + length);

  return TMWDEFS_TRUE;
}
#endif /* SDNPDATA_SUPPORT_OBJ115_V4 */

/* function: _setVariationInfo() */
static void TMWDEFS_LOCAL _setVariationInfo(
  SDNPEVNT_DESC *pDesc, 
  TMWTYPES_UCHAR variation)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pDesc->pSession;
  if (!pSDNPSession->supportEventsWithTime)
  {
    /* Switch to the equivalent variation without time */
    if (variation == 1)
    {
      variation = 3;
    }
    else if (variation == 2)
    {
      variation = 4;
    }
    
  }
  switch(variation)
  {
  default:
    /* Fall through to first one that is supported */ 
    /* 1 is the preferred default */

#if SDNPDATA_SUPPORT_OBJ115_V1
  case 1:
    pDesc->variation = 1;
    pDesc->sizeInBytes = 8;
    break;
#endif
#if SDNPDATA_SUPPORT_OBJ115_V2
  case 2:
    pDesc->variation = 2;
    /* sizeInBytes is set to the minimum size. Need to check actual length */
    pDesc->sizeInBytes = 9;
    break;
#endif
#if SDNPDATA_SUPPORT_OBJ115_V3
  case 3:
    pDesc->variation = 3;
    pDesc->sizeInBytes = 2;
    break;
#endif
#if SDNPDATA_SUPPORT_OBJ115_V4
  case 4:
    pDesc->variation = 4;
    pDesc->sizeInBytes = 3;
    break;
#endif
  }
}

/* function: sdnpo115_init */
void TMWDEFS_GLOBAL sdnpo115_init(
  TMWSESN *pSession)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;

  sdnpevnt_init(
    &pSDNPSession->obj115Timer, 
    &pSDNPSession->obj115Events);
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

  if(pSDNPSession->extStringScanPeriod != 0)
  {
    tmwtimer_start(&pSDNPSession->obj115Timer, 
      pSDNPSession->extStringScanPeriod, 
      pSession->pChannel, _scanTimeout, pSession);
  }
}

/* function: sdnpo115_open */
void TMWDEFS_GLOBAL sdnpo115_open(
  TMWSESN *pSession)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;

  if(pSDNPSession->extStringScanPeriod != 0)
  {
    tmwtimer_start(&pSDNPSession->obj115Timer, 
      pSDNPSession->extStringScanPeriod, 
      pSession->pChannel, _scanTimeout, pSession);
  }
}
#endif

/* function: sdnpo115_close */
void TMWDEFS_GLOBAL sdnpo115_close(
  TMWSESN *pSession)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;

#if SDNPDATA_SUPPORT_EVENT_SCAN
  /* Disable event scan timer */
  tmwtimer_cancel(&pSDNPSession->obj115Timer);
#endif

  sdnpevnt_close(&pSDNPSession->obj115Events);
}

/* function: sdnpo115_addEvent */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpo115_addEvent(
  TMWSESN *pSession, 
  TMWTYPES_USHORT point, 
  TMWTYPES_UCHAR *pBuf, 
  TMWTYPES_USHORT bufLength,
  TMWTYPES_UCHAR flags,
  TMWDTIME *pTimeStamp)
{
  void *pPoint;
  TMWTYPES_BOOL status;
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
#if TMWCNFG_SUPPORT_THREADS
  TMWDEFS_RESOURCE_LOCK *pLock = &pSession->pChannel->lock;
#endif
  SDNPEVNT_DESC desc;
  SDNPDATA_ADD_EVENT_VALUE value;

  status = TMWDEFS_FALSE;

  _initEventDesc(pSession, &desc);
  desc.readVariation = pSDNPSession->obj115DefaultVariation;

  TMWTARG_LOCK_SECTION(pLock);

  /* Get point event belongs to */
  pPoint = sdnpdata_extStrGetPoint(pSDNPSession->pDbHandle, point);
  if(pPoint != TMWDEFS_NULL)
  {
    TMWDEFS_CLASS_MASK eventClass = sdnpdata_extStrEventClass(pPoint);
    value.extendedStringPtr.pBuf = pBuf;
    value.extendedStringPtr.length = bufLength;
    status = sdnpevnt_addEvent(pSession, point, flags, eventClass, pTimeStamp, &desc, &value);
  }
  else
  {
    SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_ADD_EVENT);
  }

  TMWTARG_UNLOCK_SECTION(pLock);
  return status;
}

/* function: sdnpo115_countEvents */
TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpo115_countEvents(
  TMWSESN *pSession, 
  TMWDEFS_CLASS_MASK classMask,
  TMWTYPES_BOOL countAll,
  TMWTYPES_USHORT threshold)
{
  SDNPEVNT_DESC desc;
  _initEventDesc(pSession, &desc);
  return(sdnpevnt_countEvents(pSession, classMask, &desc, countAll, threshold));
}

/* function: sdnpo115_cleanupEvents() */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpo115_cleanupEvents(
  TMWSESN *pSession, 
  TMWTYPES_BOOL deleteEvents)
{
  SDNPEVNT_DESC desc;
  _initEventDesc(pSession, &desc);
  return(sdnpevnt_cleanupEvents(deleteEvents, &desc));
}

/* function: sdnpo115_readObj115VarX */
TMWTYPES_BOOL TMWDEFS_CALLBACK sdnpo115_readObj115VarX(
  TMWSESN *pSession,
  TMWSESN_TX_DATA *pResponse,
  SDNPEVNT *pEvent,
  TMWTYPES_UCHAR variation)
{
  TMWTYPES_BOOL status = TMWDEFS_FALSE;

  switch (variation)
  {

#if SDNPDATA_SUPPORT_OBJ115_V1
  case 1:
    status = _readV1(pSession, pResponse, pEvent);
    break;
#endif
#if SDNPDATA_SUPPORT_OBJ115_V2
  case 2:
    status = _readV2(pSession, pResponse, pEvent);
    break;
#endif
#if SDNPDATA_SUPPORT_OBJ115_V3
  case 3:
    status = _readV3(pSession, pResponse, pEvent);
    break;
#endif
#if SDNPDATA_SUPPORT_OBJ115_V4
  case 4:
    status = _readV4(pSession, pResponse, pEvent);
    break;
#endif
  }

  return(status);
}

/* function: sdnpo115_readObj115 */
SDNPSESN_READ_STATUS TMWDEFS_CALLBACK sdnpo115_readObj115(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRequest,
  TMWSESN_TX_DATA *pResponse, 
  DNPUTIL_OBJECT_HEADER *pObjHeader,
  SDNPSESN_QUAL qualifier)
{
  if(qualifier != SDNPSESN_QUAL_BUILD_RESPONSE)
    return(SDNPSESN_READ_COMPLETE);

  if(pObjHeader->variation == 0)
  {
    return(sdnpo115_readObj115ByClass(pSession, pRequest, pResponse, pObjHeader, TMWDEFS_CLASS_MASK_ALL));
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

    /* If other supported variation */
    _initEventDesc(pSession, &desc);
    desc.readVariation = variation;
    _setVariationInfo(&desc, variation); 

    return(sdnpevnt_readEvents(pSession, pResponse, pObjHeader, TMWDEFS_CLASS_MASK_ALL, &desc));
  }
}

/* function: sdnpo115_readObj115ByClass */
SDNPSESN_READ_STATUS TMWDEFS_CALLBACK sdnpo115_readObj115ByClass(
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
  
  _setVariationInfo(&desc, pSDNPSession->obj115DefaultVariation);

#if SDNPDATA_SUPPORT_EVENT_VAR_POINT
  if(pSDNPSession->obj115DefaultVariation == 0)
    desc.variation = 0;
#endif
  desc.variation = pSDNPSession->obj115DefaultVariation;

  return(sdnpevnt_readEvents(pSession, pResponse, pObjHeader, classMask, &desc));
}
#endif /* SDNPDATA_SUPPORT_OBJ115 */
