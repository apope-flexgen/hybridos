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

/* file: sdnpo043.c
 * description: DNP Slave functionality for Object 43 Analog Output Command Event.
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
#include "tmwscl/dnp/sdnpo043.h"

#if SDNPDATA_SUPPORT_OBJ43

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
#if SDNPDATA_SUPPORT_OBJ43_V1
  case 1:
    break;
#endif
#if SDNPDATA_SUPPORT_OBJ43_V2
  case 2:
    break;
#endif
#if SDNPDATA_SUPPORT_OBJ43_V3
  case 3:
    break;
#endif
#if SDNPDATA_SUPPORT_OBJ43_V4
  case 4:
    break;
#endif
#if SDNPDATA_SUPPORT_OBJ43_V5
  case 5:
    break;
#endif
#if SDNPDATA_SUPPORT_OBJ43_V6
  case 6:
    break;
#endif
#if SDNPDATA_SUPPORT_OBJ43_V7
  case 7:
    break;
#endif
#if SDNPDATA_SUPPORT_OBJ43_V8
  case 8:
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

  pPoint = sdnpdata_anlgOutGetPoint(pSDNPSession->pDbHandle, pointNumber);
  if(pPoint != TMWDEFS_NULL)
    *pDefVariation = sdnpdata_anlgOutCmdEventDefVariation(pPoint, classMask);
  else
    *pDefVariation = pSDNPSession->obj43DefaultVariation;

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

  void *pPoint = sdnpdata_anlgOutGetPoint(pSDNPSession->pDbHandle, pointNumber);
  if(pPoint != TMWDEFS_NULL)
    return sdnpdata_anlgOutCmdEventMode(pPoint);
  else
    return pSDNPSession->analogOutCmdEventMode;
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

  pPoint = sdnpdata_anlgOutGetPoint(pSDNPSession->pDbHandle, pointNumber);
  if(pPoint != TMWDEFS_NULL)
    *pClassMask = sdnpdata_anlgOutCmdEventClass(pPoint);

  return(pPoint);
}

/* function: _changedFunc */
static TMWTYPES_BOOL TMWDEFS_CALLBACK _changedFunc(
  TMWSESN *pSession,
  void *pPoint,
  TMWTYPES_USHORT pointNum,
  TMWDTIME *pTimeStamp)
{
  TMWTYPES_ANALOG_VALUE value;
  TMWTYPES_UCHAR flags;

  if(sdnpdata_anlgOutCmdChanged(pPoint, &value, &flags))
  {
    sdnpo043_addEvent(pSession, pointNum, &value, flags, pTimeStamp);
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
  pDesc->group = DNPDEFS_OBJ_43_ANA_CMD_EVENTS;
  pDesc->quantity = sdnpdata_anlgOutQuantity(pSDNPSession->pDbHandle);
  pDesc->eventMemType = SDNPMEM_OBJECT43_EVENT_TYPE;
  pDesc->pEventList = &pSDNPSession->obj43Events;
  pDesc->eventMode = pSDNPSession->analogOutCmdEventMode;
  pDesc->maxEvents = pSDNPSession->analogOutCmdMaxEvents;
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

#if SDNPDATA_SUPPORT_OBJ43_V1
/* function: _readV1 */
static void TMWDEFS_CALLBACK _readV1(
  TMWSESN *pSession,
  TMWSESN_TX_DATA *pResponse, 
  SDNPEVNT *pEvent)
{
  SDNPEVNT_O043_EVENT *pO43Event = (SDNPEVNT_O043_EVENT *)pEvent;
  TMWTYPES_ULONG tmpValue;
  /* Flags are not sent, but dnputil function wants to set them */
  TMWTYPES_UCHAR tmpFlags;

  /* Get value, setting over range bit in flags properly */
  tmpValue = dnputil_getAnalogValueLong(&pO43Event->value, &tmpFlags); 
  
  /* Diagnostics */
  DNPDIAG_SHOW_ANALOG_CONTROL(pSession, pEvent->point, 
    &pO43Event->value, pEvent->flags, TMWDEFS_TRUE, 0, TMWDEFS_NULL);

  /* Write command status, stored in flags field */
  pResponse->pMsgBuf[pResponse->msgLength++] = pEvent->flags;

  /* Write value */
  tmwtarg_store32(&tmpValue, &pResponse->pMsgBuf[pResponse->msgLength]);
  pResponse->msgLength += 4;
}
#endif /* SDNPDATA_SUPPORT_OBJ43_V1 */

#if SDNPDATA_SUPPORT_OBJ43_V2
/* function: _readV2 */
static void TMWDEFS_CALLBACK _readV2(
  TMWSESN *pSession,
  TMWSESN_TX_DATA *pResponse, 
  SDNPEVNT *pEvent)
{
  SDNPEVNT_O043_EVENT *pO43Event = (SDNPEVNT_O043_EVENT *)pEvent;
  TMWTYPES_USHORT tmpValue;
  /* Flags are not sent, but dnputil function wants to set them */
  TMWTYPES_UCHAR tmpFlags;

  /* Get value, setting over range bit in flags properly */
  tmpValue = dnputil_getAnalogValueShort(&pO43Event->value, &tmpFlags);
  
  /* Diagnostics */
  DNPDIAG_SHOW_ANALOG_CONTROL(pSession, pEvent->point, 
    &pO43Event->value, pEvent->flags, TMWDEFS_TRUE, 0, TMWDEFS_NULL);

  /* Write command status, stored in flags field */
  pResponse->pMsgBuf[pResponse->msgLength++] = pEvent->flags;

  /* Write value */
  tmwtarg_store16(&tmpValue, &pResponse->pMsgBuf[pResponse->msgLength]);
  pResponse->msgLength += 2;
}
#endif /* SDNPDATA_SUPPORT_OBJ43_V2 */

#if SDNPDATA_SUPPORT_OBJ43_V3
/* function: _readV3 */
static void TMWDEFS_CALLBACK _readV3(
  TMWSESN *pSession,
  TMWSESN_TX_DATA *pResponse, 
  SDNPEVNT *pEvent)
{
  SDNPEVNT_O043_EVENT *pO43Event = (SDNPEVNT_O043_EVENT *)pEvent;
  TMWTYPES_MS_SINCE_70 msSince70;
  TMWTYPES_ULONG tmpValue;
  /* Flags are not sent, but dnputil function wants to set them */
  TMWTYPES_UCHAR tmpFlags;

  /* Get value, setting over range bit in flags properly */
  tmpValue = dnputil_getAnalogValueLong(&pO43Event->value, &tmpFlags); 
  
  /* Diagnostics */
  DNPDIAG_SHOW_ANALOG_CONTROL(pSession, pEvent->point, 
    &pO43Event->value, pEvent->flags, TMWDEFS_TRUE, 0, &pEvent->timeStamp);

  /* Write command status, stored in flags field */
  pResponse->pMsgBuf[pResponse->msgLength++] = pEvent->flags;

  /* Write value */
  tmwtarg_store32(&tmpValue, &pResponse->pMsgBuf[pResponse->msgLength]);
  pResponse->msgLength += 4;

  /* Write 48 bit event time */
  dnpdtime_dateTimeToMSSince70(&msSince70, &pEvent->timeStamp);
  dnpdtime_writeMsSince70(&pResponse->pMsgBuf[pResponse->msgLength], &msSince70);
  pResponse->msgLength += 6;
}
#endif /* SDNPDATA_SUPPORT_OBJ43_V3 */

#if SDNPDATA_SUPPORT_OBJ43_V4
/* function: _readV4 */
static void TMWDEFS_CALLBACK _readV4(
  TMWSESN *pSession,
  TMWSESN_TX_DATA *pResponse, 
  SDNPEVNT *pEvent)
{
  SDNPEVNT_O043_EVENT *pO43Event = (SDNPEVNT_O043_EVENT *)pEvent;
  TMWTYPES_MS_SINCE_70 msSince70;
  TMWTYPES_USHORT tmpValue;
  /* Flags are not sent, but dnputil function wants to set them */
  TMWTYPES_UCHAR tmpFlags;

  /* Get value, setting over range bit in flags properly */
  tmpValue = dnputil_getAnalogValueShort(&pO43Event->value, &tmpFlags); 
  
  /* Diagnostics */
  DNPDIAG_SHOW_ANALOG_CONTROL(pSession, pEvent->point, 
    &pO43Event->value, pEvent->flags, TMWDEFS_TRUE, 0, &pEvent->timeStamp);

  /* Write command status, stored in flags field */
  pResponse->pMsgBuf[pResponse->msgLength++] = pEvent->flags;

  /* Write value */ 
  tmwtarg_store16(&tmpValue, &pResponse->pMsgBuf[pResponse->msgLength]);
  pResponse->msgLength += 2;

  /* Write 48 bit event time */
  dnpdtime_dateTimeToMSSince70(&msSince70, &pEvent->timeStamp);
  dnpdtime_writeMsSince70(&pResponse->pMsgBuf[pResponse->msgLength], &msSince70);
  pResponse->msgLength += 6;
}
#endif /* SDNPDATA_SUPPORT_OBJ43_V4 */

#if SDNPDATA_SUPPORT_OBJ43_V5
/* function: _readV5 */
static void TMWDEFS_LOCAL _readV5(
  TMWSESN *pSession,
  TMWSESN_TX_DATA *pResponse, 
  SDNPEVNT *pEvent)
{
  SDNPEVNT_O043_EVENT *pO43Event = (SDNPEVNT_O043_EVENT *)pEvent;
  TMWTYPES_SFLOAT tmpValue;
  /* Flags are not sent, but dnputil function wants to set them */
  TMWTYPES_UCHAR tmpFlags;

  /* Get value, setting over range bit in flags properly */
  tmpValue = dnputil_getAnalogValueFloat(&pO43Event->value, &tmpFlags);

  /* Diagnostics */
  DNPDIAG_SHOW_ANALOG_CONTROL(pSession, pEvent->point, 
    &pO43Event->value, pEvent->flags, TMWDEFS_TRUE, 0, TMWDEFS_NULL);

  /* Write command status, stored in flags field */
  pResponse->pMsgBuf[pResponse->msgLength++] = pEvent->flags;

  /* Write value */
  tmwtarg_storeSFloat(&tmpValue, &pResponse->pMsgBuf[pResponse->msgLength]);
  pResponse->msgLength += 4;
}
#endif /* SDNPDATA_SUPPORT_OBJ43_V5 */

#if SDNPDATA_SUPPORT_OBJ43_V6
/* function: _readV6 */
static void TMWDEFS_LOCAL _readV6(
  TMWSESN *pSession,
  TMWSESN_TX_DATA *pResponse, 
  SDNPEVNT *pEvent)
{
  SDNPEVNT_O043_EVENT *pO43Event = (SDNPEVNT_O043_EVENT *)pEvent;
  TMWTYPES_DOUBLE dval;

  /* Diagnostics */
  DNPDIAG_SHOW_ANALOG_CONTROL(pSession, pEvent->point, 
    &pO43Event->value, pEvent->flags, TMWDEFS_TRUE, 0, TMWDEFS_NULL);

  /* Write command status, stored in flags field */
  pResponse->pMsgBuf[pResponse->msgLength++] = pEvent->flags;

  /* Write value */
  dval = dnputil_getAnalogValueDouble(&pO43Event->value);
  tmwtarg_store64(&dval, &pResponse->pMsgBuf[pResponse->msgLength]);
  pResponse->msgLength += 8;
}
#endif /* SDNPDATA_SUPPORT_OBJ43_V6 */

#if SDNPDATA_SUPPORT_OBJ43_V7
/* function: _readV7 */
static void TMWDEFS_LOCAL _readV7(
  TMWSESN *pSession,
  TMWSESN_TX_DATA *pResponse, 
  SDNPEVNT *pEvent)
{
  SDNPEVNT_O043_EVENT *pO43Event = (SDNPEVNT_O043_EVENT *)pEvent;
  TMWTYPES_MS_SINCE_70 msSince70;
  TMWTYPES_SFLOAT tmpValue;
  /* Flags are not sent, but dnputil function wants to set them */
  TMWTYPES_UCHAR tmpFlags;

  /* Get value, setting over range bit in flags properly */
  tmpValue = dnputil_getAnalogValueFloat(&pO43Event->value, &tmpFlags);
 
  /* Diagnostics */
  DNPDIAG_SHOW_ANALOG_CONTROL(pSession, pEvent->point, 
    &pO43Event->value, pEvent->flags, TMWDEFS_TRUE, 0, &pEvent->timeStamp);

  /* Write command status, stored in flags field */
  pResponse->pMsgBuf[pResponse->msgLength++] = pEvent->flags;

  /* Write value */
  tmwtarg_storeSFloat(&tmpValue, &pResponse->pMsgBuf[pResponse->msgLength]);
  pResponse->msgLength += 4;

  /* Write 48 bit event time */
  dnpdtime_dateTimeToMSSince70(&msSince70, &pEvent->timeStamp);
  dnpdtime_writeMsSince70(&pResponse->pMsgBuf[pResponse->msgLength], &msSince70);
  pResponse->msgLength += 6;
}
#endif /* SDNPDATA_SUPPORT_OBJ43_V7 */

#if SDNPDATA_SUPPORT_OBJ43_V8
/* function: _readV8 */
static void TMWDEFS_LOCAL _readV8(
  TMWSESN *pSession,
  TMWSESN_TX_DATA *pResponse, 
  SDNPEVNT *pEvent)
{
  SDNPEVNT_O043_EVENT *pO43Event = (SDNPEVNT_O043_EVENT *)pEvent;
  TMWTYPES_MS_SINCE_70 msSince70;
  TMWTYPES_DOUBLE dval;

  /* Diagnostics */
  DNPDIAG_SHOW_ANALOG_CONTROL(pSession, pEvent->point, 
    &pO43Event->value, pEvent->flags, TMWDEFS_TRUE, 0, &pEvent->timeStamp);

  /* Write command status, stored in flags field */
  pResponse->pMsgBuf[pResponse->msgLength++] = pEvent->flags;

  /* Write value */
  dval = dnputil_getAnalogValueDouble(&pO43Event->value);
  tmwtarg_store64(&dval, &pResponse->pMsgBuf[pResponse->msgLength]);
  pResponse->msgLength += 8;

  /* Write 48 bit event time */
  dnpdtime_dateTimeToMSSince70(&msSince70, &pEvent->timeStamp);
  dnpdtime_writeMsSince70(&pResponse->pMsgBuf[pResponse->msgLength], &msSince70);
  pResponse->msgLength += 6;
}
#endif /* SDNPDATA_SUPPORT_OBJ43_V8 */

/* function: _setVariationInfo() */
static void TMWDEFS_LOCAL _setVariationInfo(
  SDNPEVNT_DESC *pDesc, 
  TMWTYPES_UCHAR variation)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pDesc->pSession;
  if (!pSDNPSession->supportEventsWithTime)
  {
    /* Switch to the equivalent variation without time */
    if (variation == 3)
    {
      variation = 1;
    }
    else if (variation == 4)
    {
      variation = 2;
    }
    else if (variation == 7)
    {
      variation = 5;
    }
    else if (variation == 8)
    {
      variation = 6;
    }
  }

  switch(variation)
  {
  default:
    /* Fall through to first one that is supported */ 
    /* 1 is the preferred default */
#if SDNPDATA_SUPPORT_OBJ43_V1
  case 1:  
    pDesc->variation = 1;
    pDesc->sizeInBytes = 5;
    pDesc->pReadIntoRespFunc = _readV1;
    break;
#endif
#if SDNPDATA_SUPPORT_OBJ43_V2
  case 2:
    pDesc->variation = 2;
    pDesc->sizeInBytes = 3;
    pDesc->pReadIntoRespFunc = _readV2;
    break;
#endif
#if SDNPDATA_SUPPORT_OBJ43_V3
  case 3:
    pDesc->variation = 3;
    pDesc->sizeInBytes = 11;
    pDesc->pReadIntoRespFunc = _readV3;
    break;
#endif
#if SDNPDATA_SUPPORT_OBJ43_V4
  case 4:
    pDesc->variation = 4;
    pDesc->sizeInBytes = 9;
    pDesc->pReadIntoRespFunc = _readV4;
    break;
#endif
#if SDNPDATA_SUPPORT_OBJ43_V5
  case 5:
    pDesc->variation = 5;
    pDesc->sizeInBytes = 5;
    pDesc->pReadIntoRespFunc = _readV5;
    break;
#endif
#if SDNPDATA_SUPPORT_OBJ43_V6
  case 6:
    pDesc->variation = 6;
    pDesc->sizeInBytes = 9;
    pDesc->pReadIntoRespFunc = _readV6;
    break;
#endif
#if SDNPDATA_SUPPORT_OBJ43_V7
  case 7:
    pDesc->variation = 7;
    pDesc->sizeInBytes = 11;
    pDesc->pReadIntoRespFunc = _readV7;
    break;
#endif
#if SDNPDATA_SUPPORT_OBJ43_V8
  case 8:
    pDesc->variation = 8;
    pDesc->sizeInBytes = 15;
    pDesc->pReadIntoRespFunc = _readV8;
    break;
#endif
  }
}

/* function: sdnpo043_init */
void TMWDEFS_GLOBAL sdnpo043_init(
  TMWSESN *pSession)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;

  sdnpevnt_init(
    &pSDNPSession->obj43Timer, 
    &pSDNPSession->obj43Events);
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

  if(pSDNPSession->analogOutCmdScanPeriod != 0)
  {
    tmwtimer_start(&pSDNPSession->obj43Timer, 
      pSDNPSession->analogOutCmdScanPeriod, 
      pSession->pChannel, _scanTimeout, pSession);
  }
}

/* function: sdnpo043_open */
void TMWDEFS_GLOBAL sdnpo043_open(
  TMWSESN *pSession)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;

  if(pSDNPSession->analogOutCmdScanPeriod != 0)
  {
    tmwtimer_start(&pSDNPSession->obj43Timer, 
      pSDNPSession->analogOutCmdScanPeriod, 
      pSession->pChannel, _scanTimeout, pSession);
  }
}
#endif

/* function: sdnpo043_close */
void TMWDEFS_GLOBAL sdnpo043_close(
  TMWSESN *pSession)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;

#if SDNPDATA_SUPPORT_EVENT_SCAN
  /* Disable event scan timer */
  tmwtimer_cancel(&pSDNPSession->obj43Timer);
#endif

  sdnpevnt_close(&pSDNPSession->obj43Events);
}

/* function: sdnpo043_addEvent */
void TMWDEFS_GLOBAL sdnpo043_addEvent(
  TMWSESN *pSession, 
  TMWTYPES_USHORT point, 
  TMWTYPES_ANALOG_VALUE *pValue, 
  TMWTYPES_UCHAR status, 
  TMWDTIME *pTimeStamp)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
#if TMWCNFG_SUPPORT_THREADS
  TMWDEFS_RESOURCE_LOCK *pLock = &pSession->pChannel->lock;
#endif
  SDNPEVNT_DESC desc;
  void *pPoint; 
  SDNPDATA_ADD_EVENT_VALUE value;

  _initEventDesc(pSession, &desc);
  desc.readVariation = pSDNPSession->obj43DefaultVariation;

  TMWTARG_LOCK_SECTION(pLock);

  /* Get point event belongs to */
  pPoint = sdnpdata_anlgOutGetPoint(pSDNPSession->pDbHandle, point);
  if(pPoint != TMWDEFS_NULL)
  {
    TMWDEFS_CLASS_MASK eventClass = sdnpdata_anlgOutCmdEventClass(pPoint);
    value.analogPtr = pValue; 

    /* Store status in flags since there are no flags for command status events */
    sdnpevnt_addEvent(pSession, point, status, eventClass, pTimeStamp, &desc, &value);
  }
  else
  {
    SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_ADD_EVENT);
  }

  TMWTARG_UNLOCK_SECTION(pLock);
}

/* function: sdnpo043_countEvents */
TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpo043_countEvents(
  TMWSESN *pSession, 
  TMWDEFS_CLASS_MASK classMask,
  TMWTYPES_BOOL countAll,
  TMWTYPES_USHORT threshold)
{
  SDNPEVNT_DESC desc;
  _initEventDesc(pSession, &desc);
  return(sdnpevnt_countEvents(pSession, classMask, &desc, countAll, threshold));
}

/* function: sdnpo043_cleanupEvents() */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpo043_cleanupEvents(
  TMWSESN *pSession, 
  TMWTYPES_BOOL deleteEvents)
{
  SDNPEVNT_DESC desc;
  _initEventDesc(pSession, &desc);
  return(sdnpevnt_cleanupEvents(deleteEvents, &desc));
}

/* function: sdnpo043_readObj43v0 */
SDNPSESN_READ_STATUS TMWDEFS_CALLBACK sdnpo043_readObj43(
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
    return(sdnpo043_readObj43v0ByClass(pSession, pRequest, pResponse, pObjHeader, TMWDEFS_CLASS_MASK_ALL));
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
    if (pSDNPSession->analogOutSizeVarPerPoint)
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
 
/* function: sdnpo043_readObj43v0ByClass */
SDNPSESN_READ_STATUS TMWDEFS_CALLBACK sdnpo043_readObj43v0ByClass(
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
    
  _setVariationInfo(&desc, pSDNPSession->obj43DefaultVariation);
 
#if SDNPDATA_SUPPORT_EVENT_VAR_POINT
  if(pSDNPSession->obj43DefaultVariation == 0)
    desc.variation = 0;
#endif

  return(sdnpevnt_readEvents(pSession, pResponse, pObjHeader, classMask, &desc));
}
#endif /* SDNPDATA_SUPPORT_OBJ43 */
