/*****************************************************************************/
/* Triangle MicroWorks, Inc.                         Copyright (c) 1997-2022 */
/*****************************************************************************/
/*                                                                           */
/* This file is the property of:                                             */
/*                                                                           */
/*                       Triangle MicroWorks, Inc.                           */
/*                      Raleigh, North Carolina USA                          */
/*                       www.TriangleMicroWorks.com                          */
/*                          (919) 880-6615                                   */
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

/* file: sdnpo088.c
 * description: DNP Slave functionality for Object 88 Data Set Snapshot Event Object
 */
#include "tmwscl/dnp/dnpdiag.h"
#include "tmwscl/utils/tmwtarg.h"

#include "tmwscl/dnp/sdnpo088.h"
#include "tmwscl/dnp/sdnpdata.h"
#include "tmwscl/dnp/sdnpsesn.h"
#include "tmwscl/dnp/sdnputil.h"
#include "tmwscl/dnp/dnputil.h"
#include "tmwscl/dnp/dnpdtime.h"
#include "tmwscl/dnp/sdnpevnt.h"
#include "tmwscl/dnp/sdnpdiag.h"
 
#if SDNPDATA_SUPPORT_OBJ88

TMWTYPES_BOOL sdnpo088_storeEventData(
  TMWTYPES_UCHAR numberElems,
  DNPDATA_DATASET_VALUE *pData,
  SDNPEVNT_O088_EVENT *p088Event)
{
  int i;
  TMWTYPES_UCHAR length;
  TMWTYPES_USHORT maxSize;
  TMWTYPES_USHORT msgLength;
  DNPDATA_DATASET_VALUE *pAddElem = pData;
  SDNPEVNT *pEvent = &p088Event->sdnp;
  DNPCHNL *pDNPChannel = (DNPCHNL *)pEvent->pSession->pChannel;
  TMWTYPES_MS_SINCE_70 msSince70;

  /* Event must fit in a single fragment. Leave space for header. */
  maxSize = pDNPChannel->txFragmentSize;
  if(maxSize > (SDNPCNFG_MAX_DSEVENT_LENGTH))
    maxSize = SDNPCNFG_MAX_DSEVENT_LENGTH;

  maxSize -= 8;

  /* Format response into event buffer structure */

  /* Leave space for two byte length at beginning */
  msgLength = 2;

  /* Mandatory Data Set Identifier */
  length = dnputil_lengthRequired(pEvent->point);
  p088Event->data[msgLength++] = length;
  dnputil_putIntInMessage(&p088Event->data[msgLength], pEvent->point, length);
  msgLength = (TMWTYPES_USHORT)(msgLength + length);

  /* put mandatory timestamp in response. */
  /* Write length */
  p088Event->data[msgLength++] = 6;

  dnpdtime_dateTimeToMSSince70(&msSince70, &pEvent->timeStamp);
  dnpdtime_writeMsSince70(&p088Event->data[msgLength], &msSince70);
  msgLength += 6;

  /* DNPDIAG_SHOW_DATASET_TIME(pEvent->pSession, pEvent->point, &pEvent->timeStamp, TMWDEFS_FALSE, 0);*/

  /* Data set elements after mandatory time stamp element */
  for(i=0; i< numberElems; i++)
  {
    if((msgLength + pAddElem->length + 1) > maxSize)
    {
      /* Not enough space left in message */
      return(TMWDEFS_FALSE);
    }

    p088Event->data[msgLength++] = pAddElem->length;

    dnputil_putValueInMessage(p088Event->data, &msgLength, pAddElem);
    /* DNPDIAG_SHOW_DATASET(pEvent->pSession, (TMWTYPES_UCHAR)i, pAddElem, TMWDEFS_FALSE, 0);*/
    pAddElem++;
  }
  p088Event->responseSize = msgLength;

  /* Store 16 bit length of data in buffer */
  msgLength = msgLength - 2;
  tmwtarg_store16(&msgLength, &p088Event->data[0]);

  return TMWDEFS_TRUE;
}

/* function: _initEventDesc */
static void TMWDEFS_LOCAL _initEventDesc(
  TMWSESN *pSession,
  SDNPEVNT_DESC *pDesc)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  pDesc->pSession = pSession;
  pDesc->group = DNPDEFS_OBJ_88_DATASET_EVENTS;
  pDesc->quantity = sdnpdata_datasetQuantity(pSDNPSession->pDbHandle);
  pDesc->eventMemType = SDNPMEM_OBJECT88_EVENT_TYPE;
  pDesc->pEventList = &pSDNPSession->obj88Events;
  pDesc->eventMode = pSDNPSession->datasetEventMode;
  pDesc->maxEvents = pSDNPSession->datasetMaxEvents;

  /* These two are used for scanning mode */
#if SDNPDATA_SUPPORT_EVENT_SCAN
  pDesc->pGetPointAndClass = TMWDEFS_NULL;
  pDesc->pChangedFunc = TMWDEFS_NULL;
#endif

  /* These two are used for default class per point */
#if SDNPDATA_SUPPORT_EVENT_VAR_POINT
  /* There is only 1 data set event variation */
  pDesc->pGetPointAndDefault = TMWDEFS_NULL;
  pDesc->pSetVariationInfo = TMWDEFS_NULL;
#endif
#if SDNPDATA_SUPPORT_EVENT_MODE_POINT
  pDesc->pGetPointAndEventMode = TMWDEFS_NULL;
#endif

  /* At least room for mandatory elements */
  pDesc->sizeInBytes = 14;
  pDesc->variation = 1;

  /* Set this to 1 so that SCL does not try to determine default class per point */
  pDesc->readVariation = 1;
}

/* function: _readV1 */
TMWTYPES_BOOL TMWDEFS_CALLBACK sdnpo088_readV1(
  TMWSESN *pSession,
  TMWSESN_TX_DATA *pResponse,
  SDNPEVNT *pEvent)
{ 
  SDNPEVNT_O088_EVENT *pO88Event = (SDNPEVNT_O088_EVENT *)pEvent;
  TMWTYPES_USHORT responseSize = pO88Event->responseSize;
  TMWTARG_UNUSED_PARAM(pSession);

  /* This function won't be called unless there is enough room for 
   * mandatory elements
   */

  if(pResponse->msgLength + responseSize < pResponse->maxLength)
  {
    memcpy(&pResponse->pMsgBuf[pResponse->msgLength], &pO88Event->data[0], responseSize);
    SDNPDIAG_SHOW_DATASET_EVENT(pSession, pResponse);
    pResponse->msgLength = (TMWTYPES_USHORT)(pResponse->msgLength + responseSize);
    return(TMWDEFS_TRUE);
  }
 
  return(TMWDEFS_FALSE);
}
 
/* function: sdnpo088_readObj88v0ByClass */
SDNPSESN_READ_STATUS TMWDEFS_CALLBACK sdnpo088_readObj88v0ByClass(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRequest,
  TMWSESN_TX_DATA *pResponse,
  DNPUTIL_OBJECT_HEADER *pObjHeader,
  TMWDEFS_CLASS_MASK classMask)
{
  SDNPEVNT_DESC desc;
  TMWTARG_UNUSED_PARAM(pRequest);

  _initEventDesc(pSession, &desc);
  desc.pReadIntoRespFunc = TMWDEFS_NULL; /*_readV1;*/

  return(sdnpevnt_readEvents(pSession, pResponse, pObjHeader, classMask, &desc));
}

/* function: sdnpo088_init */
void TMWDEFS_GLOBAL sdnpo088_init(
   TMWSESN *pSession)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;

  sdnpevnt_init(TMWDEFS_NULL, &pSDNPSession->obj88Events);
}

/* function: sdnpo088_close */
void TMWDEFS_GLOBAL sdnpo088_close(
  TMWSESN *pSession)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  sdnpevnt_close(&pSDNPSession->obj88Events);
}

/* function: sdnpo088_addEvent */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpo088_addEvent(
  TMWSESN *pSession,
  TMWTYPES_USHORT point,
  DNPDATA_DATASET_VALUE *pDataSet,
  TMWTYPES_UCHAR numberElems,
  TMWDTIME *pTimeStamp)
{
  TMWTYPES_BOOL retValue = TMWDEFS_FALSE;
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
#if TMWCNFG_SUPPORT_THREADS
  TMWDEFS_RESOURCE_LOCK *pLock = &pSession->pChannel->lock;
#endif
  SDNPEVNT_DESC desc;
  void *pPoint;
  SDNPDATA_ADD_EVENT_VALUE value;

  _initEventDesc(pSession, &desc);
  /* readVariation is set to 1 */

  TMWTARG_LOCK_SECTION(pLock);

  /* Get descriptor event relates to */
  pPoint = sdnpdata_datasetDescrGetPoint(pSDNPSession->pDbHandle, point);
  if(pPoint != TMWDEFS_NULL)
  {
    TMWTYPES_UCHAR characteristics;

    /* Read attributes to see if this allowed to generate events */
    sdnpdata_datasetDescrReadChars(pPoint, &characteristics);

    if((characteristics & DNPDEFS_DATASET_CHAR_EV) != 0)
    {  
      TMWDEFS_CLASS_MASK eventClass = sdnpdata_datasetDescrEventClass(pPoint);
      value.dataset.numberElems = numberElems;
      value.dataset.pData = pDataSet;
      retValue = sdnpevnt_addEvent(pSession, point, 0, eventClass, pTimeStamp, &desc, &value);
    }
  }
   
  if(!retValue)
  {
    SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_ADD_EVENT);
  }

  TMWTARG_UNLOCK_SECTION(pLock);

  return(retValue);
}

/* function: sdnpo088_countEvents */
TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpo088_countEvents(
  TMWSESN *pSession,
  TMWDEFS_CLASS_MASK classMask,
  TMWTYPES_BOOL countAll,
  TMWTYPES_USHORT threshold)
{
  SDNPEVNT_DESC desc;
  _initEventDesc(pSession, &desc);
  return(sdnpevnt_countEvents(pSession, classMask, &desc, countAll, threshold));
}

/* function: sdnpo088_cleanupEvents() */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpo088_cleanupEvents(
  TMWSESN *pSession,
  TMWTYPES_BOOL deleteEvents)
{
  SDNPEVNT_DESC desc;
  _initEventDesc(pSession, &desc);
  return(sdnpevnt_cleanupEvents(deleteEvents, &desc));
}

/* function: sdnpo088_readObj88 */
SDNPSESN_READ_STATUS TMWDEFS_CALLBACK sdnpo088_readObj88(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRequest,
  TMWSESN_TX_DATA *pResponse,
  DNPUTIL_OBJECT_HEADER *pObjHeader,
  SDNPSESN_QUAL qualifier)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  TMWTARG_UNUSED_PARAM(pRequest);

  if(qualifier != SDNPSESN_QUAL_BUILD_RESPONSE)
  {
    return(SDNPSESN_READ_COMPLETE);
  }

  if((pObjHeader->variation == 0) ||
    (pObjHeader->variation == 1))
  {
    SDNPEVNT_DESC desc;

    _initEventDesc(pSession, &desc);
    desc.pReadIntoRespFunc = TMWDEFS_NULL; /*_readV1;*/

    return(sdnpevnt_readEvents(pSession, pResponse, pObjHeader, TMWDEFS_CLASS_MASK_ALL, &desc));
  }

  pSDNPSession->iin |= DNPDEFS_IIN_OBJECT_UNKNOWN;
  return (SDNPSESN_READ_FAILED);
}

#endif /* SDNPDATA_SUPPORT_OBJ088 */
