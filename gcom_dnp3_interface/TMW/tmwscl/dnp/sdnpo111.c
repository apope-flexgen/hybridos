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

/* file: sdnpo111.c
 * description: DNP Slave functionality for Object 111 Octet String Events.
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
#include "tmwscl/dnp/sdnpo111.h"

#if SDNPDATA_SUPPORT_OBJ111
/* function: _stringDiagFunc */
static void TMWDEFS_CALLBACK _stringDiagFunc(
  TMWSESN *pSession,
  TMWTYPES_USHORT pointNum,
  TMWTYPES_UCHAR *pBuf,
  TMWTYPES_UCHAR length)
{
  DNPDIAG_SHOW_STRING_EVENT(pSession, pointNum, pBuf, length);
}

#if SDNPDATA_SUPPORT_EVENT_MODE_POINT
/* function: _getPointAndEventMode */
static TMWDEFS_EVENT_MODE TMWDEFS_CALLBACK _getPointAndEventMode(
  TMWSESN *pSession,
  TMWTYPES_USHORT pointNumber)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;

  void *pPoint = sdnpdata_strGetPoint(pSDNPSession->pDbHandle, pointNumber);
  if(pPoint != TMWDEFS_NULL)
    return sdnpdata_strEventMode(pPoint);
  else

    return pSDNPSession->stringEventMode;
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

  pPoint = sdnpdata_strGetPoint(pSDNPSession->pDbHandle, pointNumber);
  if(pPoint != TMWDEFS_NULL)
    *pClassMask = sdnpdata_strEventClass(pPoint);

  return(pPoint);
}

/* function: _changedFunc */
static TMWTYPES_BOOL TMWDEFS_CALLBACK _changedFunc(
  TMWSESN *pSession,
  void *pPoint,
  TMWTYPES_USHORT pointNum,
  TMWDTIME *pTimeStamp)
{
  TMWTYPES_UCHAR buf[DNPDEFS_MAX_STRING_LENGTH];
  TMWTYPES_UCHAR length;

  if(sdnpdata_strChanged(pPoint, sizeof(buf), buf, &length))
  {
    sdnpo111_addEvent(pSession, pointNum, buf, length, pTimeStamp);
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
  pDesc->group = DNPDEFS_OBJ_111_STRING_EVENTS;
  pDesc->quantity = sdnpdata_strQuantity(pSDNPSession->pDbHandle);
  pDesc->eventMemType = SDNPMEM_OBJECT111_EVENT_TYPE;
  pDesc->pEventList = &pSDNPSession->obj111Events;
  pDesc->eventMode = pSDNPSession->stringEventMode;
  pDesc->maxEvents = pSDNPSession->stringMaxEvents;
  pDesc->truncateStrings = pSDNPSession->truncateStrings;
#if SDNPDATA_SUPPORT_EVENT_SCAN
  pDesc->pGetPointAndClass = _getPointAndClass;
  pDesc->pChangedFunc = _changedFunc;
#endif
#if SDNPDATA_SUPPORT_EVENT_MODE_POINT
  pDesc->pGetPointAndEventMode = _getPointAndEventMode;
#endif
  pDesc->pStringDiagFunc = _stringDiagFunc;
}

/* function: sdnpo111_init */
void TMWDEFS_GLOBAL sdnpo111_init(
  TMWSESN *pSession)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;

  sdnpevnt_init(
    &pSDNPSession->obj111Timer, 
    &pSDNPSession->obj111Events);
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

  if(pSDNPSession->stringScanPeriod != 0)
  {
    tmwtimer_start(&pSDNPSession->obj111Timer, 
      pSDNPSession->stringScanPeriod, 
      pSession->pChannel, _scanTimeout, pSession);
  }
}

/* function: sdnpo111_open */
void TMWDEFS_GLOBAL sdnpo111_open(
  TMWSESN *pSession)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;

  if(pSDNPSession->stringScanPeriod != 0)
  {
    tmwtimer_start(&pSDNPSession->obj111Timer, 
      pSDNPSession->stringScanPeriod, 
      pSession->pChannel, _scanTimeout, pSession);
  }
}
#endif

/* function: sdnpo111_close */
void TMWDEFS_GLOBAL sdnpo111_close(
  TMWSESN *pSession)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;

#if SDNPDATA_SUPPORT_EVENT_SCAN
  /* Disable event scan timer */
  tmwtimer_cancel(&pSDNPSession->obj111Timer);
#endif

  sdnpevnt_close(&pSDNPSession->obj111Events);
}

/* function: sdnpo111_addEvent */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpo111_addEvent(
  TMWSESN *pSession, 
  TMWTYPES_USHORT point, 
  TMWTYPES_UCHAR *pBuf, 
  TMWTYPES_UCHAR bufLength, 
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

  /* A length of zero would result in variation 0 which is not allowed */
  if(bufLength == 0)
  {
    return status;
  }

  _initEventDesc(pSession, &desc);
  /* Set this to nonzero */
  desc.readVariation = 1;

  TMWTARG_LOCK_SECTION(pLock);

  /* Get point event belongs to */
  pPoint = sdnpdata_strGetPoint(pSDNPSession->pDbHandle, point);
  if(pPoint != TMWDEFS_NULL)
  {
    TMWDEFS_CLASS_MASK eventClass = sdnpdata_strEventClass(pPoint);
    value.stringPtr.pBuf = pBuf; 
    value.stringPtr.length = bufLength;
    status = sdnpevnt_addEvent(pSession, point, 0, eventClass, pTimeStamp, &desc, &value);
  }
  else
  {
    SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_ADD_EVENT);
  }

  TMWTARG_UNLOCK_SECTION(pLock);
  return status;
}

/* function: sdnpo111_countEvents */
TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpo111_countEvents(
  TMWSESN *pSession, 
  TMWDEFS_CLASS_MASK classMask,
  TMWTYPES_BOOL countAll,
  TMWTYPES_USHORT threshold)
{
  SDNPEVNT_DESC desc;
  _initEventDesc(pSession, &desc);
  return(sdnpevnt_countEvents(pSession, classMask, &desc, countAll, threshold));
}

/* function: sdnpo111_cleanupEvents() */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpo111_cleanupEvents(
  TMWSESN *pSession, 
  TMWTYPES_BOOL deleteEvents)
{
  SDNPEVNT_DESC desc;
  _initEventDesc(pSession, &desc);
  return(sdnpevnt_cleanupEvents(deleteEvents, &desc));
}

/* function: sdnpo111_readObj111 */
SDNPSESN_READ_STATUS TMWDEFS_CALLBACK sdnpo111_readObj111(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRequest,
  TMWSESN_TX_DATA *pResponse, 
  DNPUTIL_OBJECT_HEADER *pObjHeader,
  SDNPSESN_QUAL qualifier)
{
  if(qualifier != SDNPSESN_QUAL_BUILD_RESPONSE)
    return(SDNPSESN_READ_COMPLETE);

  return(sdnpo111_readObj111ByClass(pSession, pRequest, pResponse, pObjHeader, TMWDEFS_CLASS_MASK_ALL));
}

/* function: sdnpo111_readObj111ByClass */
SDNPSESN_READ_STATUS TMWDEFS_CALLBACK sdnpo111_readObj111ByClass(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRequest,
  TMWSESN_TX_DATA *pResponse, 
  DNPUTIL_OBJECT_HEADER *pObjHeader,
  TMWDEFS_CLASS_MASK classMask)
{
  SDNPEVNT_DESC desc;
  TMWTARG_UNUSED_PARAM(pRequest);

  _initEventDesc(pSession, &desc);
  desc.variation = pObjHeader->variation;

  return(sdnpevnt_readStringEvents(pSession, pResponse, pObjHeader, classMask, &desc));
}
#endif /* SDNPDATA_SUPPORT_OBJ111 */
