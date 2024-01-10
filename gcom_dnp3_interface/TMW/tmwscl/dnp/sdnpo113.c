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

/* file: sdnpo113.c
 * description: DNP Slave functionality for Object 113 Virtual Terminal Events.
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
#include "tmwscl/dnp/sdnpo113.h"

#if SDNPDATA_SUPPORT_OBJ113
/* function: _vtermDiagFunc */
static void TMWDEFS_CALLBACK _vtermDiagFunc(
  TMWSESN *pSession,
  TMWTYPES_USHORT pointNum,
  TMWTYPES_UCHAR *pBuf,
  TMWTYPES_UCHAR length)
{
  DNPDIAG_SHOW_VTERM_EVENT(pSession, pointNum, pBuf, length);
}

#if SDNPDATA_SUPPORT_EVENT_MODE_POINT
/* function: _getPointAndEventMode */
static TMWDEFS_EVENT_MODE TMWDEFS_CALLBACK _getPointAndEventMode(
  TMWSESN *pSession,
  TMWTYPES_USHORT pointNumber)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;

  void *pPoint = sdnpdata_vtermGetPoint(pSDNPSession->pDbHandle, pointNumber);
  if(pPoint != TMWDEFS_NULL)
    return sdnpdata_vtermEventMode(pPoint);
  else

    return pSDNPSession->virtualTerminalEventMode;
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

  pPoint = sdnpdata_vtermGetPoint(pSDNPSession->pDbHandle, pointNumber);
  if(pPoint != TMWDEFS_NULL)
    *pClassMask = sdnpdata_vtermEventClass(pPoint);

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

  if(sdnpdata_vtermChanged(pPoint, sizeof(buf), buf, &length))
  {
    sdnpo113_addEvent(pSession, pointNum, buf, length, pTimeStamp);
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
  pDesc->group = DNPDEFS_OBJ_113_VTERM_EVENTS;
  pDesc->quantity = sdnpdata_vtermQuantity(pSDNPSession->pDbHandle);
  pDesc->pStringDiagFunc = _vtermDiagFunc;
  pDesc->eventMemType = SDNPMEM_OBJECT113_EVENT_TYPE;
  pDesc->pEventList = &pSDNPSession->obj113Events;
  pDesc->eventMode = pSDNPSession->virtualTerminalEventMode;
  pDesc->maxEvents = pSDNPSession->virtualTerminalMaxEvents;
  pDesc->truncateStrings = pSDNPSession->truncateVirtualTerminalEvents;
#if SDNPDATA_SUPPORT_EVENT_SCAN
  pDesc->pGetPointAndClass = _getPointAndClass;
  pDesc->pChangedFunc = _changedFunc;
#endif
#if SDNPDATA_SUPPORT_EVENT_VAR_POINT
  pDesc->pGetPointAndDefault = TMWDEFS_NULL;
  pDesc->pSetVariationInfo = TMWDEFS_NULL;
#endif
#if SDNPDATA_SUPPORT_EVENT_MODE_POINT
  pDesc->pGetPointAndEventMode = _getPointAndEventMode;
#endif
}

/* function: sdnpo113_init */
void TMWDEFS_GLOBAL sdnpo113_init(
  TMWSESN *pSession)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;

  sdnpevnt_init(
    &pSDNPSession->obj113Timer, 
    &pSDNPSession->obj113Events);
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

  if(pSDNPSession->virtualTerminalScanPeriod != 0)
  {
    tmwtimer_start(&pSDNPSession->obj113Timer, 
      pSDNPSession->virtualTerminalScanPeriod, 
      pSession->pChannel, _scanTimeout, pSession);
  }
}

/* function: sdnpo113_open */
void TMWDEFS_GLOBAL sdnpo113_open(
  TMWSESN *pSession)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;

  if(pSDNPSession->virtualTerminalScanPeriod != 0)
  {
    tmwtimer_start(&pSDNPSession->obj113Timer, 
      pSDNPSession->virtualTerminalScanPeriod, 
      pSession->pChannel, _scanTimeout, pSession);
  }
}
#endif

/* function: sdnpo113_close */
void TMWDEFS_GLOBAL sdnpo113_close(
  TMWSESN *pSession)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;

#if SDNPDATA_SUPPORT_EVENT_SCAN
  /* Disable event scan timer */
  tmwtimer_cancel(&pSDNPSession->obj113Timer);
#endif

  sdnpevnt_close(&pSDNPSession->obj113Events);
}

/* function: sdnpo113_addEvent */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpo113_addEvent(
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
  pPoint = sdnpdata_vtermGetPoint(pSDNPSession->pDbHandle, point);
  if(pPoint != TMWDEFS_NULL)
  {
    TMWDEFS_CLASS_MASK eventClass = sdnpdata_vtermEventClass(pPoint);
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

/* function: sdnpo113_countEvents */
TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpo113_countEvents(
  TMWSESN *pSession, 
  TMWDEFS_CLASS_MASK classMask,
  TMWTYPES_BOOL countAll,
  TMWTYPES_USHORT threshold)
{
  SDNPEVNT_DESC desc;
  _initEventDesc(pSession, &desc);
  return(sdnpevnt_countEvents(pSession, classMask, &desc, countAll, threshold));
}

/* function: sdnpo113_cleanupEvents() */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpo113_cleanupEvents(
  TMWSESN *pSession, 
  TMWTYPES_BOOL deleteEvents)
{
  SDNPEVNT_DESC desc;
  _initEventDesc(pSession, &desc);
  return(sdnpevnt_cleanupEvents(deleteEvents, &desc));
}

/* function: sdnpo113_readObj113 */
SDNPSESN_READ_STATUS TMWDEFS_CALLBACK sdnpo113_readObj113(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRequest,
  TMWSESN_TX_DATA *pResponse, 
  DNPUTIL_OBJECT_HEADER *pObjHeader,
  SDNPSESN_QUAL qualifier)
{
  if(qualifier != SDNPSESN_QUAL_BUILD_RESPONSE)
  {
    if((pObjHeader->qualifier == DNPDEFS_QUAL_8BIT_INDEX)
      ||(pObjHeader->qualifier == DNPDEFS_QUAL_16BIT_INDEX))
    {
      /* if index, advance past the point indexes in the request */
      TMWTYPES_USHORT pointNumber;
      TMWTYPES_USHORT pointIndex = 0;
      while(pointIndex < pObjHeader->numberOfPoints)
      {
        dnputil_getPointNumber(pRequest, pObjHeader, pointIndex, &pointNumber);
        pointIndex++;
      }
    }
    return(SDNPSESN_READ_COMPLETE);
  }

  return(sdnpo113_readObj113ByClass(pSession, pRequest, pResponse, pObjHeader, TMWDEFS_CLASS_MASK_ALL));
}

/* function: sdnpo113_readObj113ByClass */
SDNPSESN_READ_STATUS TMWDEFS_CALLBACK sdnpo113_readObj113ByClass(
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
  
  /* virtual terminal event reads allow range and index qualifiers
   * unlike other (string) event reads. Process those separately.
   */
  if((pObjHeader->qualifier <= DNPDEFS_QUAL_16BIT_START_STOP)
    ||(pObjHeader->qualifier == DNPDEFS_QUAL_8BIT_INDEX)
    ||(pObjHeader->qualifier == DNPDEFS_QUAL_16BIT_INDEX))
  {
    return(sdnpevnt_readVtermEvents(pSession, pRequest, pResponse, pObjHeader, &desc));
  }
  else
  {
    return(sdnpevnt_readStringEvents(pSession, pResponse, pObjHeader, classMask, &desc));
  }
}
#endif /* SDNPDATA_SUPPORT_OBJ113 */
