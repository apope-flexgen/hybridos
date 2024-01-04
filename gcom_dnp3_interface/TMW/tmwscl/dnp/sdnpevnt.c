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

/* file: sdnpevnt.c
 * description: This file is intended for internal SCL use only.
 *   Base class for all slave DNP3 events types
 */
#include "tmwscl/dnp/dnpdiag.h"
#include "tmwscl/dnp/sdnpdiag.h"
#include "tmwscl/dnp/sdnpevnt.h"
#include "tmwscl/dnp/sdnputil.h"
#include "tmwscl/dnp/sdnpunsl.h"
#include "tmwscl/dnp/sdnpmem.h"
#include "tmwscl/dnp/sdnpo002.h"
#include "tmwscl/dnp/sdnpo004.h"
#include "tmwscl/dnp/sdnpo088.h"
#include "tmwscl/dnp/sdnpo115.h"
#include "tmwscl/dnp/sdnpo120.h"
#include "tmwscl/dnp/dnpstat.h"
#include "tmwscl/dnp/dnpdtime.h"

#include "tmwscl/utils/tmwtarg.h"

/* function: sdnpevnt_init */
void TMWDEFS_GLOBAL sdnpevnt_init(
  TMWTIMER *pTimer,
  TMWDLIST *pEventList)
{
  if(pTimer != TMWDEFS_NULL)
  {
    tmwtimer_init(pTimer);
  }
  tmwdlist_initialize(pEventList);
}

/* function: sdnpevnt_close */
void TMWDEFS_GLOBAL sdnpevnt_close(
  TMWDLIST *pEventList)
{
  /* Clear out any events still in the list */
  tmwdlist_destroy(pEventList, sdnpmem_free);
}

#if SDNPDATA_SUPPORT_EVENT_SCAN
/* function: sdnpevnt_scanForChanges */
void TMWDEFS_GLOBAL sdnpevnt_scanForChanges(
  TMWSESN *pSession,
  SDNPEVNT_DESC *pDesc)
{
  TMWDEFS_CLASS_MASK eventMask;
  TMWDTIME timeStamp;
  TMWTYPES_USHORT i;
  void *pPoint;

  /* Initialize time to current time in case binaryInputChanged does not
   * update it.
   */
  sdnputil_getDateTime(pSession, &timeStamp);

  /* Scan all binary input points to see if they have changed */
  for(i = 0; i < pDesc->quantity; i++)
  {
    /* Get point that event pertains to */
    pPoint = pDesc->pGetPointAndClass(pSession, i, &eventMask);

    if(pPoint == TMWDEFS_NULL)
      continue;
   
    if(eventMask != TMWDEFS_CLASS_MASK_NONE)
    {
      if(pDesc->pChangedFunc(pSession, pPoint, i, &timeStamp))
      {
        /* Reset time for next event */
        sdnputil_getDateTime(pSession, &timeStamp);
      }
    }
  }
}
#endif

/* function: _validateQualifier */
static TMWTYPES_BOOL TMWDEFS_LOCAL _validateQualifier(
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{
  /* Figure out the maximum number of events to write based on request 
   * qualifier and the number of events in the event queue.
   */
  switch(pObjHeader->qualifier)
  {
  case DNPDEFS_QUAL_ALL_POINTS:
    /* If qualifier is all points we can return all the events we have */
    pObjHeader->numberOfPoints = 0xffff;
    break;

  case DNPDEFS_QUAL_8BIT_LIMITED_QTY:
  case DNPDEFS_QUAL_16BIT_LIMITED_QTY:
    /* If qualifier is limited quantity than pObjHeader->numberOfPoints
    * contains the maximum quantity from the request header.
    */
    break;

  default:
    return(TMWDEFS_FALSE);
  }

  return(TMWDEFS_TRUE);
}

/* function:  _removeCorrectEvent */
static SDNPEVNT * TMWDEFS_LOCAL _removeCorrectEvent(
  TMWDTIME *pTimeStamp,
  SDNPEVNT_DESC *pDesc)
{
  SDNPEVNT *pOldEvent;
  SDNPSESN *pSDNPSession = (SDNPSESN *)pDesc->pSession;
  TMWTYPES_BOOL discardNewEvent = TMWDEFS_FALSE; 

  /* Set buffer overflow bit */
  pSDNPSession->iin |= DNPDEFS_IIN_BUFFER_OVFL; 
    
  DNPSTAT_SESN_EVENT_OVERFLOW(pDesc->pSession, pDesc->group);

  /* determine which event to delete, based on configuration and the timeStamp 
   * of this new event. Events are sorted.
   */
  if(pSDNPSession->deleteOldestEvent)
  { 
    /* Delete oldest event */
    pOldEvent = (SDNPEVNT *)tmwdlist_getFirst(pDesc->pEventList);
    if(pOldEvent != TMWDEFS_NULL)
    {
      if (pSDNPSession->sortEventsByTime)
      {
        /* if timestamp of event being added is earlier or equal to oldest event in queue
         * discard event being added (older time)
         */
        if (tmwdtime_checkTimeOrder(pTimeStamp, &pOldEvent->timeStamp))
        {
          discardNewEvent = TMWDEFS_TRUE;
        }
      }
    }
  }
  else
  { /* Delete newest event */
    pOldEvent = (SDNPEVNT *)tmwdlist_getLast(pDesc->pEventList);
    if(pOldEvent != TMWDEFS_NULL)
    {
      if (pSDNPSession->sortEventsByTime)
      {
        /* if timestamp of newest event in queue is older than timestamp of event
         * being added discard event being added (newer time)
         */
        if (tmwdtime_checkTimeOrder(&pOldEvent->timeStamp, pTimeStamp))
        {
          discardNewEvent = TMWDEFS_TRUE;
        }
      }
      else
      {
        discardNewEvent = TMWDEFS_TRUE;
      }
    }
  }
    
  /* Don't add this new event */
  if(discardNewEvent) 
  {
    return(TMWDEFS_NULL);
  }
  
  /* remove old event from queue, but keep the memory to be reused */
  tmwdlist_removeEntry(pDesc->pEventList, (TMWDLIST_MEMBER *)pOldEvent);

  sdnpunsl_removeEvent(pSDNPSession, pOldEvent);

#if SDNPDATA_SUPPORT_OBJ2 && SDNPDATA_SUPPORT_OBJ4
  if (pDesc->group == 2)
    pSDNPSession->obj2Count--;
  else if (pDesc->group == 4)
    pSDNPSession->obj4Count--;
#endif

  return(pOldEvent);
}

/* function: _setValue */
static TMWTYPES_BOOL TMWDEFS_LOCAL _setValue(
  TMWTYPES_UCHAR group,
  SDNPEVNT *pEvent,
  SDNPDATA_ADD_EVENT_VALUE *pValue)
{
  TMWTYPES_BOOL retValue = TMWDEFS_TRUE;
  switch(group)
  {
#if SDNPDATA_SUPPORT_OBJ22
    case DNPDEFS_OBJ_22_CNTR_EVENTS:  
    {
      SDNPEVNT_O022_EVENT *ptr = (SDNPEVNT_O022_EVENT *)pEvent;
      ptr->value = pValue->ulValue;
      break;
    }
#endif
#if SDNPDATA_SUPPORT_OBJ23
    case DNPDEFS_OBJ_23_FCTR_EVENTS:  
    {
      SDNPEVNT_O023_EVENT *ptr = (SDNPEVNT_O023_EVENT *)pEvent;
      ptr->value = pValue->ulValue;
      break;
    }
#endif
#if SDNPDATA_SUPPORT_OBJ32
    case DNPDEFS_OBJ_32_ANA_CHNG_EVENTS:
    {
      SDNPEVNT_O032_EVENT *ptr = (SDNPEVNT_O032_EVENT *)pEvent;
      ptr->value = *pValue->analogPtr;
      break;
    }
#endif
#if SDNPDATA_SUPPORT_OBJ33
    case DNPDEFS_OBJ_33_FRZN_ANA_EVENTS:  
    {
      SDNPEVNT_O033_EVENT *ptr = (SDNPEVNT_O033_EVENT *)pEvent;
      ptr->value = *pValue->analogPtr;
      break;
    }
#endif
#if SDNPDATA_SUPPORT_OBJ42
    case DNPDEFS_OBJ_42_ANA_OUT_EVENTS:
    {
      SDNPEVNT_O042_EVENT *ptr = (SDNPEVNT_O042_EVENT *)pEvent;
      ptr->value = *pValue->analogPtr;
      break;
    }
#endif
#if SDNPDATA_SUPPORT_OBJ43
    case DNPDEFS_OBJ_43_ANA_CMD_EVENTS:
    {
      SDNPEVNT_O043_EVENT *ptr = (SDNPEVNT_O043_EVENT *)pEvent;
      ptr->value = *pValue->analogPtr;
      break;
    }
#endif
#if SDNPDATA_SUPPORT_OBJ88
    case DNPDEFS_OBJ_88_DATASET_EVENTS:
      {
      SDNPEVNT_O088_EVENT *ptr = (SDNPEVNT_O088_EVENT *)pEvent;
      retValue = sdnpo088_storeEventData(pValue->dataset.numberElems, pValue->dataset.pData, ptr); 
      break;
    }
#endif
#if SDNPDATA_SUPPORT_OBJ111
    case DNPDEFS_OBJ_111_STRING_EVENTS:
    {
      SDNPEVNT_O111_EVENT *ptr = (SDNPEVNT_O111_EVENT *)pEvent;
      memcpy(ptr->strBuf, pValue->stringPtr.pBuf, pValue->stringPtr.length);
      ptr->strLength = (TMWTYPES_UCHAR)pValue->stringPtr.length;
      break;
    }
#endif
#if SDNPDATA_SUPPORT_OBJ113
    case DNPDEFS_OBJ_113_VTERM_EVENTS:
    {
      SDNPEVNT_O113_EVENT *ptr = (SDNPEVNT_O113_EVENT *)pEvent;
      memcpy(ptr->strBuf, pValue->stringPtr.pBuf, pValue->stringPtr.length);
      ptr->strLength = (TMWTYPES_UCHAR)pValue->stringPtr.length;
      break;
    }
#endif
#if SDNPDATA_SUPPORT_OBJ115
    case DNPDEFS_OBJ_115_EXT_STR_EVENTS:
    {
      SDNPEVNT_O115_EVENT *ptr = (SDNPEVNT_O115_EVENT *)pEvent;
      memcpy(ptr->strBuf, pValue->extendedStringPtr.pBuf, pValue->extendedStringPtr.length);
      ptr->strLength = pValue->extendedStringPtr.length;
      break;
    }
#endif
#if SDNPDATA_SUPPORT_OBJ120
    case DNPDEFS_OBJ_120_AUTHENTICATION:
    {
      SDNPEVNT_0120_EVENT *ptr = (SDNPEVNT_0120_EVENT *)pEvent;
      memcpy(ptr->errorTextBuf, pValue->authError.pErrorText, pValue->authError.errorTextLength);
      ptr->errorTextLength = pValue->authError.errorTextLength;
      ptr->assocId = pValue->authError.assocId;
      ptr->sequenceNumber = pValue->authError.sequenceNumber;
      ptr->errorCode = pValue->authError.errorCode;
      break;
    }
    case DNPDEFS_OBJ_122_AUTHSTATEVENTS:  
    {
      SDNPEVNT_O122_EVENT *ptr = (SDNPEVNT_O122_EVENT *)pEvent;
      ptr->assocId = pValue->authSecStat.assocId;
      ptr->value = pValue->authSecStat.ulValue;
      break;
    }
#endif
  }
  return(retValue);
}

/* function: sdnpevnt_addEvent */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpevnt_addEvent(
  TMWSESN *pSession,
  TMWTYPES_USHORT point,
  TMWTYPES_UCHAR flags,
  TMWDEFS_CLASS_MASK classMask,
  TMWDTIME *pTimeStamp,
  SDNPEVNT_DESC *pDesc,
  SDNPDATA_ADD_EVENT_VALUE *pValue)
{
  SDNPEVNT *pEvent;
  SDNPEVNT *pOldEvent;
  SDNPSESN *pSDNPSession;
  TMWDEFS_EVENT_MODE eventMode;

  /* If no class is specified, return failure, or else this event
   * will never get read from event queue
   */
  if ((classMask & TMWDEFS_CLASS_MASK_ALL) == 0)
  {
    return TMWDEFS_FALSE;
  }

  pEvent = TMWDEFS_NULL;
  pSDNPSession = (SDNPSESN *)pDesc->pSession;

#if SDNPCNFG_USER_MANAGED_EVENTS
  if (pSDNPSession->userManagedEvents)
  {
    TMWTYPES_UCHAR defaultVariation;

#if SDNPDATA_SUPPORT_EVENT_VAR_POINT
    /* if default variation for this Type is 0, ask the database for the default variation for this point */
    if ((pDesc->readVariation == 0) && (pDesc->pGetPointAndDefault != TMWDEFS_NULL))
      pDesc->pGetPointAndDefault(pSession, point, classMask, &defaultVariation);
    else
#endif
      defaultVariation = pDesc->readVariation;

    if (sdnpdata_umEventAdd(pSDNPSession->pDbHandle, pDesc->group, point, classMask, defaultVariation, flags, pValue, pTimeStamp))
    {
      /* If successful, update event status */
      sdnpevnt_updateEvents(pSession, classMask);
      return(TMWDEFS_TRUE);
    }
    else
    {
      pSDNPSession->iin |= DNPDEFS_IIN_BUFFER_OVFL;
      return(TMWDEFS_FALSE);
    }
  }
#endif

  eventMode = pDesc->eventMode;

#if SDNPDATA_SUPPORT_EVENT_MODE_POINT
  /* If mode is most recent, remove any previous event for this point */
  if ((pDesc->eventMode == TMWDEFS_EVENT_MODE_PER_POINT) && (pDesc->pGetPointAndEventMode != TMWDEFS_NULL))
  {
    eventMode = pDesc->pGetPointAndEventMode(pSession, point);
  }
#endif

  if ((eventMode == TMWDEFS_EVENT_MODE_MOST_RECENT) || (eventMode == TMWDEFS_EVENT_MODE_CURRENT))
  {
    pEvent = (SDNPEVNT *)tmwdlist_getFirst(pDesc->pEventList);
    while (pEvent != TMWDEFS_NULL)
    {
      /* See if this event is for the same point as the new one */
      if (pEvent->point == point)
      {
        /* Yep, remove it, but keep the memory to be reused below */
        tmwdlist_removeEntry(pDesc->pEventList, (TMWDLIST_MEMBER *)pEvent);

        sdnpunsl_removeEvent(pSDNPSession, pEvent);

#if SDNPDATA_SUPPORT_OBJ2 && SDNPDATA_SUPPORT_OBJ4
        if (pDesc->group == 2)
          pSDNPSession->obj2Count--;
        else if (pDesc->group == 4)
          pSDNPSession->obj4Count--;
#endif

        /* found a matching event, don't look for any more */
        break;
      }

      pEvent = (SDNPEVNT *)tmwdlist_getNext((TMWDLIST_MEMBER *)pEvent);
    }
  }

  /* If we removed an event because mode is MOST RECENT,
   * don't check for list full, or if allocate fails
   */
  if (pEvent == TMWDEFS_NULL)
  {
    TMWTYPES_BOOL listFull = TMWDEFS_FALSE;
    /* See if event list is full. */
#if SDNPDATA_SUPPORT_OBJ2 && SDNPDATA_SUPPORT_OBJ4 
    if (pDesc->group < 5)
    {
      if (((pDesc->group == 2) && (pSDNPSession->obj2Count >= pDesc->maxEvents))
        ||((pDesc->group == 4) && (pSDNPSession->obj4Count >= pDesc->maxEvents)))
      {
        listFull = TMWDEFS_TRUE;
        /* Remove correct event from full list and reuse it. */
        pEvent = _removeCorrectEvent(pTimeStamp, pDesc);
      }
    }
    else 
#endif
    if ((pDesc->maxEvents != 0)
      && (tmwdlist_size(pDesc->pEventList) >= pDesc->maxEvents))
    {
      listFull = TMWDEFS_TRUE;
      /* Remove correct event from full list and reuse it. */
      pEvent = _removeCorrectEvent(pTimeStamp, pDesc);
    }
    if(!listFull)
    {
      /* Since the list is not full, try to allocate a new event */
      pEvent = (SDNPEVNT *)sdnpmem_alloc(pDesc->eventMemType);
      if (pEvent == TMWDEFS_NULL)
      {
        /* No more memory.
         * Remove correct event from the list to be reused
         */
        SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_ALLOC_EVENT);
        pEvent = _removeCorrectEvent(pTimeStamp, pDesc);
      }
    }
  }

  /* if we still don't have an event structure, we either decided based
   * on configuration and timeStamp that this event should not be added
   * or we could not allocate memory.
   */
  if (pEvent == TMWDEFS_NULL)
    return(TMWDEFS_FALSE);

  /* Initialize new event */
  pEvent->point = point;
  pEvent->group = pDesc->group;
  pEvent->flags = flags;
  pEvent->classMask = classMask;
  pEvent->timeStamp = *pTimeStamp;
  pEvent->eventSent = TMWDEFS_FALSE;
  pEvent->getCurrentValue = (TMWTYPES_BOOL)((eventMode == TMWDEFS_EVENT_MODE_CURRENT) ? TMWDEFS_TRUE : TMWDEFS_FALSE);
  pEvent->pSession = pSession;

#if SDNPDATA_SUPPORT_EVENT_VAR_POINT
  /* if default variation for this Type is 0, ask the database for the default variation for this point */
  if ((pDesc->readVariation == 0) && (pDesc->pGetPointAndDefault != TMWDEFS_NULL))
    pDesc->pGetPointAndDefault(pSession, point, classMask, &pEvent->defaultVariation);
  else
#endif
    pEvent->defaultVariation = pDesc->readVariation;

  if (!_setValue(pDesc->group, pEvent, pValue))
  {
    /* Failed to set the value in the event buffer
     * Most data types can't fail, but datasets can.
     */
    sdnpmem_free(pEvent);
    return(TMWDEFS_FALSE);
  }

  pOldEvent = TMWDEFS_NULL;
  /* Add event to queue in timeStamp order if configured to */
  if (pSDNPSession->sortEventsByTime)
  {
    pOldEvent = (SDNPEVNT *)tmwdlist_getLast(pDesc->pEventList);
    if (pOldEvent != TMWDEFS_NULL)
    {
      /* If new event is newer than last event in queue, put at end of queue
      * otherwise, look through the queue to see where event should be inserted
      */
      if (!pSDNPSession->supportEventsWithTime || tmwdtime_checkTimeOrder(&pOldEvent->timeStamp, &pEvent->timeStamp))
      {
        /* Insert at end of queue */
        pOldEvent = TMWDEFS_NULL;
      }
      else
      {
        pOldEvent = (SDNPEVNT *)tmwdlist_getFirst(pDesc->pEventList);
        while (pOldEvent != TMWDEFS_NULL)
        {
          if (!tmwdtime_checkTimeOrder(&pOldEvent->timeStamp, &pEvent->timeStamp))
            break;

          pOldEvent = (SDNPEVNT *)tmwdlist_getNext((TMWDLIST_MEMBER *)pOldEvent);
        }
      }
    }
  }

#if SDNPDATA_SUPPORT_OBJ2 && SDNPDATA_SUPPORT_OBJ4 
  if (pDesc->group == 2)
    pSDNPSession->obj2Count++;
  else if (pDesc->group == 4)
    pSDNPSession->obj4Count++;
#endif

  /* See if we are at the end of the queue */
  if(pOldEvent == TMWDEFS_NULL)
  {
    /* Yes, insert at end of queue */
    tmwdlist_addEntry(pDesc->pEventList, (TMWDLIST_MEMBER *)pEvent);
  }
  else
  {
    /* No, insert before next event entry */
    tmwdlist_insertEntryBefore(pDesc->pEventList,
      (TMWDLIST_MEMBER *)pOldEvent, (TMWDLIST_MEMBER *)pEvent);
  }

  /* If successful, update event status */
  sdnpevnt_updateEvents(pSession, classMask);
  return(TMWDEFS_TRUE);
}

/* function: sdnpevnt_updateEvents */
void TMWDEFS_GLOBAL sdnpevnt_updateEvents(
  TMWSESN *pSession,
  TMWDEFS_CLASS_MASK eventClass)
{
  /* Update IIN bits to reflect that there is at least one event available */
  sdnputil_updateIINEvents(pSession, eventClass);

  /* Update unsolicited event management */
  sdnpunsl_addEvent(pSession, eventClass);
}

/* function: sdnpevnt_countEvents */
TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpevnt_countEvents(
  TMWSESN *pSession, 
  TMWDEFS_CLASS_MASK classMask,
  SDNPEVNT_DESC *pDesc,
  TMWTYPES_BOOL countAll,
  TMWTYPES_USHORT threshold)
{
  SDNPEVNT *pEvent;
  TMWTYPES_USHORT numEvents = 0;
#if SDNPCNFG_USER_MANAGED_EVENTS
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
    if(pSDNPSession->userManagedEvents)
    {
      return(sdnpdata_umEventNotSentCount(pSDNPSession->pDbHandle, pDesc->group, classMask));
    }
#else
  TMWTARG_UNUSED_PARAM(pSession);
#endif

  /* If no class is specified there will be no events that match */
  if(classMask == 0)
    return 0;

  if((pEvent = (SDNPEVNT *)tmwdlist_getFirst(pDesc->pEventList)) == TMWDEFS_NULL)
    return 0;

  do 
  {
    /* If this event has already been sent don't count it */
    if(pEvent->eventSent)
      continue;

    /* If this event's class does not match the requested class don't count it */
    if((pEvent->classMask & classMask) == 0)
      continue;

    numEvents += 1;
    if(!countAll && (numEvents >= threshold))
      return(numEvents);

  } while((pEvent = (SDNPEVNT *)tmwdlist_getNext((TMWDLIST_MEMBER *)pEvent)) != TMWDEFS_NULL);

  return(numEvents);
}

/* function: sdnpevnt_cleanupEvents() */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpevnt_cleanupEvents(
  TMWTYPES_BOOL deleteEvents,
  SDNPEVNT_DESC *pDesc)
{
  SDNPEVNT *pEvent;
  TMWTYPES_BOOL queueFull;
#if SDNPDATA_SUPPORT_OBJ2 && SDNPDATA_SUPPORT_OBJ4 || SDNPCNFG_USER_MANAGED_EVENTS
  SDNPSESN *pSDNPSession = (SDNPSESN *)pDesc->pSession;
#endif

#if SDNPCNFG_USER_MANAGED_EVENTS
  if(pSDNPSession->userManagedEvents)
  {
    if(deleteEvents)
      return(sdnpdata_umEventRemove(pSDNPSession->pDbHandle, pDesc->group));
    else
      return(sdnpdata_umEventNotSent(pSDNPSession->pDbHandle, pDesc->group));
  }
#endif

  pEvent = (SDNPEVNT *)tmwdlist_getFirst(pDesc->pEventList);
  if(pEvent == TMWDEFS_NULL)
  {
    return TMWDEFS_FALSE;
  }

  while(pEvent != TMWDEFS_NULL)
  {
    /* Get next event now in case we delete this one */
    SDNPEVNT *pNextEvent = (SDNPEVNT *)tmwdlist_getNext((TMWDLIST_MEMBER *)pEvent);

    if(pEvent->eventSent)
    {
      if(deleteEvents)
      {
        DNPSTAT_SESN_EVENT_CONFIRM(pDesc->pSession, pDesc->group, pEvent->point);

#if SDNPDATA_SUPPORT_OBJ2 && SDNPDATA_SUPPORT_OBJ4 
        if (pEvent->group == 2)
          pSDNPSession->obj2Count--;
        else if (pEvent->group == 4)
          pSDNPSession->obj4Count--;
#endif

        tmwdlist_removeEntry(pDesc->pEventList, (TMWDLIST_MEMBER *)pEvent);
        sdnpmem_free(pEvent);
      }
      else
      {
        pEvent->eventSent = TMWDEFS_FALSE;
      }
    }

    pEvent = pNextEvent;
  }
  {
    TMWTYPES_ULONG currentSize;
#if SDNPDATA_SUPPORT_OBJ2 && SDNPDATA_SUPPORT_OBJ4 
    if (pDesc->group == 2)
      currentSize = pSDNPSession->obj2Count;
    else if (pDesc->group == 4)
      currentSize = pSDNPSession->obj4Count;
    else
#endif
      currentSize = tmwdlist_size(pDesc->pEventList);
    queueFull = (TMWTYPES_BOOL)
      (!sdnpmem_checkLimit(pDesc->eventMemType)
        || (currentSize >= pDesc->maxEvents));
  }
  return(queueFull);
}

/* function: sdnpevnt_initReadEventResponse */
SDNPSESN_READ_STATUS TMWDEFS_GLOBAL sdnpevnt_initReadEventResponse(
  TMWSESN_TX_DATA *pResponse, 
  DNPUTIL_OBJECT_HEADER *pObjHeader, 
  TMWTYPES_UCHAR group, 
  TMWTYPES_UCHAR variation,
  TMWTYPES_USHORT maxPointNumber,
  TMWTYPES_USHORT numberOfEvents,
  TMWTYPES_USHORT dataSizeInBytes,
  TMWTYPES_UCHAR *pResponseQual)
{
  TMWTYPES_UCHAR responseQual = DNPDEFS_QUAL_8BIT_INDEX;

  /* If we have events, start filling in the message */
  if(pObjHeader->numberOfPoints > 0)
  {
    TMWTYPES_USHORT maxMsgLength;

    /* If the number of events being read is less than the number of events in the queue 
     * use the smaller number to determine which qualifier to use 
     */
    if(pObjHeader->numberOfPoints < numberOfEvents)
      numberOfEvents = pObjHeader->numberOfPoints;

    /* Response qualifier is based maximum point number we must accomodate 
     * or the number of events to be put in message. If there are more than 255 events 
     * we must use 16 bit qualifier. Comparing to 200 (instead of 255)
     * leaves room for more events being added and still being able to use 8bit index.
     */
    if((maxPointNumber >255)
      ||(numberOfEvents > 200))
    {
      responseQual = DNPDEFS_QUAL_16BIT_INDEX;
    }
    if((group == DNPDEFS_OBJ_88_DATASET_EVENTS)
      || (group == DNPDEFS_OBJ_120_AUTHENTICATION))
    {
      responseQual = DNPDEFS_QUAL_16BIT_FREE_FORMAT;
    }

    maxMsgLength = pResponse->maxLength;
#if SDNPDATA_SUPPORT_OBJ120
    {
      SDNPSESN *pSDNPSession = (SDNPSESN *)pResponse->pSession;
      if (pSDNPSession->authenticationEnabled && pSDNPSession->preChallengeApplConf)
        /* leave room for challenge object g120v1 at end of fragment */
        maxMsgLength -= 32;
    }
#endif

    if(responseQual == DNPDEFS_QUAL_8BIT_INDEX)
    {
      /* Make sure object header (4), point index (1) and at least one event will fit */
      if((pResponse->msgLength + 5 + dataSizeInBytes) >= maxMsgLength)
        return(SDNPSESN_READ_MORE_DATA);

      /* Write object header */
      pResponse->pMsgBuf[pResponse->msgLength++] = group;
      pResponse->pMsgBuf[pResponse->msgLength++] = variation;
      pResponse->pMsgBuf[pResponse->msgLength++] = responseQual;
      pResponse->pMsgBuf[pResponse->msgLength++] = (TMWTYPES_UCHAR)numberOfEvents;
    }
    else if(responseQual == DNPDEFS_QUAL_16BIT_INDEX)
    {
      /* Make sure object header (5), point index (2) and at least one event will fit */
      if((pResponse->msgLength + 7 + dataSizeInBytes) >= maxMsgLength)
        return(SDNPSESN_READ_MORE_DATA);

      /* Write object header */
      pResponse->pMsgBuf[pResponse->msgLength++] = group;
      pResponse->pMsgBuf[pResponse->msgLength++] = variation;
      pResponse->pMsgBuf[pResponse->msgLength++] = responseQual;
      tmwtarg_store16(&numberOfEvents, &pResponse->pMsgBuf[pResponse->msgLength]);
      pResponse->msgLength += 2;
    }
    else
    {  
      /* Write object header */
      pResponse->pMsgBuf[pResponse->msgLength++] = group;
      pResponse->pMsgBuf[pResponse->msgLength++] = variation;
      pResponse->pMsgBuf[pResponse->msgLength++] = responseQual;
      pResponse->pMsgBuf[pResponse->msgLength++] = (TMWTYPES_UCHAR)numberOfEvents;
    }
  }
  
  DNPDIAG_SHOW_TX_OBJECT_HDR(pResponse->pSession, group, variation, responseQual); 

  *pResponseQual = responseQual;
  return(SDNPSESN_READ_COMPLETE);
}

/* function: sdnpevnt_updateObjectHeader */
void TMWDEFS_GLOBAL sdnpevnt_updateObjectHeader(
  TMWSESN_TX_DATA *pResponse,
  TMWTYPES_USHORT objectHeaderOffset,
  TMWTYPES_USHORT numEvents)
{
  TMWTYPES_UCHAR qualifier = pResponse->pMsgBuf[objectHeaderOffset + 2];
  if((qualifier == DNPDEFS_QUAL_8BIT_INDEX) || (qualifier == DNPDEFS_QUAL_16BIT_FREE_FORMAT))
  {
    pResponse->pMsgBuf[objectHeaderOffset + 3] = (TMWTYPES_UCHAR)numEvents;
  }
  else
  {
    tmwtarg_store16(&numEvents, &pResponse->pMsgBuf[objectHeaderOffset + 3]);
  }
}


#if SDNPCNFG_USER_MANAGED_EVENTS
TMWTYPES_BOOL sdnpevent_umGetEvent(
 SDNPSESN *pSDNPSession,
 TMWTYPES_UCHAR group, 
 TMWDEFS_CLASS_MASK classMask, 
 TMWDEFS_EVENT_MODE eventMode,
 TMWTYPES_BOOL firstEvent,
 SDNPEVNT_DATA *pEvent)
{
  SDNPDATA_GET_EVENT userEvent;
  /* Initialize group in case database does not support this new field */
  userEvent.group = group;
  if(sdnpdata_umEventGet(pSDNPSession->pDbHandle, group, classMask, firstEvent, &userEvent))
  {
    pEvent->common.group           = userEvent.group;
    pEvent->common.flags           = userEvent.flags;
    pEvent->common.point           = userEvent.point;
    pEvent->common.classMask       = userEvent.classMask;
    pEvent->common.defaultVariation= userEvent.defaultVariation;
    pEvent->common.timeStamp       = userEvent.timeStamp; 
    pEvent->common.eventSent       = TMWDEFS_FALSE; /* This is not really used for user managed events */
    pEvent->common.getCurrentValue = (TMWTYPES_BOOL)((eventMode == TMWDEFS_EVENT_MODE_CURRENT)? TMWDEFS_TRUE:TMWDEFS_FALSE);
    pEvent->common.pSession        = (TMWSESN *)pSDNPSession;
    switch(group)
    {
    case 22:
      pEvent->obj22.value = userEvent.value.ulValue;
      break;
    case 23:
      pEvent->obj23.value = userEvent.value.ulValue;
      break;
    case 32:
      pEvent->obj32.value = userEvent.value.analogValue;
      break;

    case 33:
      pEvent->obj33.value = userEvent.value.analogValue;
      break;

    case 42:
      pEvent->obj42.value = userEvent.value.analogValue;
      break;

    case 43:
      pEvent->obj43.value = userEvent.value.analogValue;
      break;

#if SDNPDATA_SUPPORT_OBJ88
    case 88:
      if(!sdnpo088_storeEventData(userEvent.value.datasetValue.numberElems, userEvent.value.datasetValue.pDataSet, &pEvent->obj88))
      {
        return(TMWDEFS_FALSE);
      }
      break;  
#endif

    case 111:
      memcpy(pEvent->obj111.strBuf, userEvent.value.stringValue.buf, userEvent.value.stringValue.length);
      pEvent->obj111.strLength = userEvent.value.stringValue.length;
      break;  

    case 113:
      memcpy(pEvent->obj113.strBuf, userEvent.value.stringValue.buf, userEvent.value.stringValue.length);
      pEvent->obj113.strLength = userEvent.value.stringValue.length;
      break;

#if SDNPDATA_SUPPORT_OBJ115
    case 115:
      memcpy(pEvent->obj115.strBuf, userEvent.value.extendedStringValue.buf, userEvent.value.extendedStringValue.length);
      pEvent->obj115.strLength = userEvent.value.extendedStringValue.length;
      break;
#endif

#if DNPCNFG_SUPPORT_AUTHENTICATION
    case 120:
      pEvent->obj120.sequenceNumber = userEvent.value.authError.sequenceNumber;
      pEvent->obj120.assocId = userEvent.value.authError.assocId;
      pEvent->obj120.errorCode = userEvent.value.authError.errorCode; 
      memcpy(pEvent->obj120.errorTextBuf, userEvent.value.authError.errorText, userEvent.value.authError.errorTextLength);
      pEvent->obj120.errorTextLength = userEvent.value.authError.errorTextLength;
      break;

#if DNPCNFG_SUPPORT_SA_VERSION5
    case 122:
      pEvent->obj122.assocId = userEvent.value.authSecStat.assocId;
      pEvent->obj122.value = userEvent.value.authSecStat.ulValue;
      break;
#endif
#endif
    } 
    return(TMWDEFS_TRUE);
  }
  return(TMWDEFS_FALSE);
}

SDNPEVNT* TMWDEFS_GLOBAL sdnpevent_getFirstEvent(
  SDNPEVNT_DESC *pDesc,
  TMWDEFS_CLASS_MASK classMask,
  SDNPEVNT_DATA *pEventData)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pDesc->pSession;
  SDNPEVNT *pEvent = TMWDEFS_NULL;

  if (pSDNPSession->userManagedEvents)
  {
    if (sdnpevent_umGetEvent(pSDNPSession, pDesc->group, classMask, pDesc->eventMode, TMWDEFS_TRUE, pEventData))
      pEvent = (SDNPEVNT *)pEventData;
    else
      pEvent = TMWDEFS_NULL;
  }
  else
  {
    pEvent = (SDNPEVNT *)tmwdlist_getFirst(pDesc->pEventList);
  }

  return(pEvent);
}

SDNPEVNT* TMWDEFS_GLOBAL sdnpevent_getNextEvent(
  SDNPEVNT_DESC *pDesc,
  TMWDEFS_CLASS_MASK classMask,
  SDNPEVNT *pEvent,
  SDNPEVNT_DATA *pEventData)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pDesc->pSession;
  if (pSDNPSession->userManagedEvents)
  {
    if (sdnpevent_umGetEvent(pSDNPSession, pDesc->group, classMask, pDesc->eventMode, TMWDEFS_FALSE, pEventData))
      pEvent = (SDNPEVNT *)pEventData;
    else
      pEvent = TMWDEFS_NULL;
  }
  else
  {
    pEvent = (SDNPEVNT *)tmwdlist_getNext((TMWDLIST_MEMBER *)pEvent);
  }
  return(pEvent);
}
#endif

/* function: sdnpevnt_readEvents */
SDNPSESN_READ_STATUS TMWDEFS_GLOBAL sdnpevnt_readEvents(
  TMWSESN *pSession,
  TMWSESN_TX_DATA *pResponse, 
  DNPUTIL_OBJECT_HEADER *pObjHeader, 
  TMWDEFS_CLASS_MASK classMask,
  SDNPEVNT_DESC *pDesc)
{
  SDNPSESN_READ_STATUS status = SDNPSESN_READ_COMPLETE;

  /* see if we have any events of this type that match classMask requested */
  if((sdnpevnt_countEvents(pSession, classMask, pDesc, TMWDEFS_FALSE, 1)) > 0)
  {
    SDNPEVNT *pEvent;
    SDNPSESN *pSDNPSession = (SDNPSESN *)pDesc->pSession;
#if SDNPCNFG_USER_MANAGED_EVENTS
    SDNPEVNT_DATA event;
#endif
    TMWTYPES_USHORT maxMsgLength;
    TMWTYPES_USHORT numEventsInObject = 0;
    TMWTYPES_USHORT numEventsInResponse = 0;
    TMWTYPES_USHORT objHeaderOffset = 0;
    TMWTYPES_UCHAR responseQual = DNPDEFS_QUAL_8BIT_INDEX;
    TMWTYPES_BOOL pointNumberAdded = TMWDEFS_FALSE;
#if SDNPDATA_SUPPORT_OBJ2 && SDNPDATA_SUPPORT_OBJ4 
    TMWTYPES_BOOL readObj2AndObj4 = TMWDEFS_FALSE;
#endif

    /* Validate qualifier and initialize object header */
    if(!_validateQualifier(pObjHeader))
      return(SDNPSESN_READ_FAILED);

    if((pSDNPSession->readStatus == SDNPSESN_READ_MORE_DATA)
      && ((pObjHeader->qualifier == DNPDEFS_QUAL_8BIT_LIMITED_QTY)
          ||(pObjHeader->qualifier == DNPDEFS_QUAL_16BIT_LIMITED_QTY)))
    {
      pObjHeader->numberOfPoints = pSDNPSession->readMultiFragNumberOfPoints;
    }

    /* If after validating the qualifier, we have determined there
     * are no points to get, just return COMPLETE 
     */
    if(pObjHeader->numberOfPoints == 0)
    {
      return(status);
    } 

#if SDNPDATA_SUPPORT_OBJ2 && SDNPDATA_SUPPORT_OBJ4 
    if((pSDNPSession->readObj2AndObj4 == SDNPSESN_OBJ2ANDOBJ4READ)
      && (pDesc->group < 5))
    {
      readObj2AndObj4 = TMWDEFS_TRUE;
      (void)readObj2AndObj4; // adding this to get rid of the compiler error about an unused variable
    }
#endif
 
    /* Start adding events */
#if SDNPCNFG_USER_MANAGED_EVENTS
    pEvent = sdnpevent_getFirstEvent(pDesc, classMask, &event);    
#else
    pEvent = (SDNPEVNT *)tmwdlist_getFirst(pDesc->pEventList);
#endif
    
#if SDNPDATA_SUPPORT_OBJ2 && SDNPDATA_SUPPORT_OBJ4
    /* binary input and double bit binary input events need to be sorted
     * together if returned sorted together in a read response, so they are in 
     * the same event queue.
     */ 
    if (pDesc->group < 5)
    {
      /* This will update pDesc and pEvent if necessary */
      sdnputil_getObj2Or4Event(pDesc, &pEvent, classMask);
    }
#endif

    maxMsgLength = pResponse->maxLength;
#if SDNPDATA_SUPPORT_OBJ120
    if (pSDNPSession->authenticationEnabled && pSDNPSession->preChallengeApplConf)
      /* leave room for challenge object g120v1 at end of fragment */
      maxMsgLength -= 32;
#endif

    while(pEvent != TMWDEFS_NULL)
    {
      pointNumberAdded = TMWDEFS_FALSE;
      /* Make sure another event will fit in response */
      if(((pResponse->msgLength + pDesc->sizeInBytes + 2) >= maxMsgLength) 
        || ((responseQual == DNPDEFS_QUAL_8BIT_INDEX) && (numEventsInObject == 255)))
        break;

      /* See if this point's class matches request */
      if((pEvent->classMask & classMask) != 0)
      {
        /* See if this is the first point in this object */
        if(numEventsInObject == 0)
        {
          TMWTYPES_USHORT numberOfEvents;

#if SDNPDATA_SUPPORT_EVENT_VAR_POINT
          /* if the read variation is set to zero, use the default variation saved 
           * when this event was added.
           */
          if((pDesc->readVariation == 0) && (pDesc->pSetVariationInfo != TMWDEFS_NULL))
          {
            pDesc->pSetVariationInfo(pDesc, pEvent->defaultVariation);
          }
#endif 

#if SDNPDATA_SUPPORT_OBJ2_V3
          if (pDesc->group == DNPDEFS_OBJ_2_BIN_CHNG_EVENTS && pDesc->variation == 2 && pEvent->timeStamp.invalid)
          {
            /* Need to switch to variation 3 unsync */
            sdnpo002_setVariationInfo(pDesc, 3);
          }
          /* handle read of obj 2 var 3 separately, may have to send CTO */
          if(pDesc->group == DNPDEFS_OBJ_2_BIN_CHNG_EVENTS && pDesc->variation == 3)
          {
            status = sdnpo002_readObj2Var3(&pEvent, &numEventsInResponse, pDesc, pResponse, pObjHeader, classMask);
         
            if(status != SDNPSESN_READ_COMPLETE)
              break;

            /* Have we read all the events the request asked for */
            if(numEventsInResponse >= pObjHeader->numberOfPoints)
              break;

            numEventsInObject = 0;

            /* pEvent has already been advanced to next event */
            continue;
          }
#endif
#if SDNPDATA_SUPPORT_OBJ4_V3
          if (pDesc->group == DNPDEFS_OBJ_4_DBL_CHNG_EVENTS && pDesc->variation == 2 && pEvent->timeStamp.invalid)
          {
            /* Need to switch to variation 3 unsync */
            sdnpo004_setVariationInfo(pDesc, 3);
          }
          /* handle read of obj 2 var 3 separately, may have to send CTO */
          if(pDesc->group == DNPDEFS_OBJ_4_DBL_CHNG_EVENTS && pDesc->variation == 3)
          {
            status = sdnpo004_readObj4Var3(&pEvent, &numEventsInResponse, pDesc, pResponse, pObjHeader, classMask);
        
            if(status != SDNPSESN_READ_COMPLETE)
              break;

            /* Have we read all the events the request asked for */
            if(numEventsInResponse >= pObjHeader->numberOfPoints)
              break;

            numEventsInObject = 0;

            /* pEvent has already been advanced to next event */
            continue;
          }
#endif
          /* Save offset to object header */
          objHeaderOffset = pResponse->msgLength;
          
          /* numberOfEvents is the initial value in the response header, but it gets set to the correct value
           * numberOfEvents IS used to determine the qualifier 
           */
          numberOfEvents = sdnpevnt_countEvents(pSession, classMask, pDesc, TMWDEFS_FALSE, 256);

          /* Write object header */
          if((status = sdnpevnt_initReadEventResponse(pResponse, 
            pObjHeader, pDesc->group, pDesc->variation, pDesc->quantity-1, numberOfEvents,
            pDesc->sizeInBytes, &responseQual)) != SDNPSESN_READ_COMPLETE)
          {
            return(status);
          }
        } 
        else
        {
#if SDNPDATA_SUPPORT_EVENT_VAR_POINT
          /* If default variation per point is supported, see if this is 
           * same as previous
           */
          if((pDesc->readVariation == 0) && (pDesc->pSetVariationInfo != TMWDEFS_NULL))
          {
            if (pDesc->variation != pEvent->defaultVariation)
            {
              /* If events do not support timestamps, pSetVariationInfo may change default variation to current one */
              TMWTYPES_UCHAR tempVar = pDesc->variation;
              pDesc->pSetVariationInfo(pDesc, pEvent->defaultVariation);
              if (pDesc->variation != tempVar)
              {
                /* Update the previous object header, before starting a new object */
                sdnpevnt_updateObjectHeader(pResponse, objHeaderOffset, numEventsInObject);
                numEventsInObject = 0;

                /* pEvent already points to next event */
                continue;
              }
            }
          }
#endif
        }

        /* Write point number */
        if(responseQual == DNPDEFS_QUAL_8BIT_INDEX)
        {
          pointNumberAdded = TMWDEFS_TRUE;
          pResponse->pMsgBuf[pResponse->msgLength++] = (TMWTYPES_UCHAR)pEvent->point;
        }
        else if(responseQual == DNPDEFS_QUAL_16BIT_INDEX)
        {
          pointNumberAdded = TMWDEFS_TRUE;
          tmwtarg_store16(&pEvent->point, &pResponse->pMsgBuf[pResponse->msgLength]);
          pResponse->msgLength += 2;
        }
        /* else DNPDEFS_QUAL_16BIT_FREE_FORMAT */

#if SDNPDATA_SUPPORT_OBJ88
        if (pDesc->group == DNPDEFS_OBJ_88_DATASET_EVENTS)
        {
          /* For data set events, we need to be able to tell if this event won't fit */
          if (!sdnpo088_readV1(pSession, pResponse, pEvent))
          {
            break;
          }
        }
        else
#endif
#if SDNPDATA_SUPPORT_OBJ115
          if (pDesc->group == DNPDEFS_OBJ_115_EXT_STR_EVENTS)
          {
            /* For extended string events, we need to be able to tell if this event won't fit */
            if (!sdnpo115_readObj115VarX(pSession, pResponse, pEvent, pDesc->variation))
            {
              break;
            }
          }
          else
#endif
#if SDNPDATA_SUPPORT_OBJ120 
        if(pDesc->group == DNPDEFS_OBJ_120_AUTHENTICATION)
        {
          if(!sdnpo120_readObj120Var7(pEvent, pResponse, classMask))
          {
            /* remove the object header for this event */
            pResponse->msgLength -= 4;
            break;
          }
        }
        else
#endif
        /* Store event data into response */
        pDesc->pReadIntoRespFunc(pSession, pResponse, pEvent);

        /* Mark event for deletion when acknowledge received */
#if SDNPCNFG_USER_MANAGED_EVENTS
        if(pSDNPSession->userManagedEvents)
        {
          sdnpdata_umEventSent(pSDNPSession->pDbHandle, pDesc->group, pEvent->point);
        }
        else
#endif
        pEvent->eventSent = TMWDEFS_TRUE;

        /* Mark response so we know it contains events */
        pResponse->txFlags |= TMWSESN_TXFLAGS_CONTAINS_EVENTS;

        /* Keep track of how many events have been read */
        numEventsInObject += 1;
        numEventsInResponse += 1;

        DNPSTAT_SESN_EVENT_SENT(pDesc->pSession, pDesc->group, pEvent->point);

        /* Have we read all the events the request asked for */
        if(numEventsInResponse >= pObjHeader->numberOfPoints)
          break;
      }

#if SDNPCNFG_USER_MANAGED_EVENTS
      pEvent = sdnpevent_getNextEvent(pDesc, classMask, pEvent, &event);
#else      
      pEvent = (SDNPEVNT *)tmwdlist_getNext((TMWDLIST_MEMBER *)pEvent);
#endif

#if SDNPDATA_SUPPORT_OBJ2 && SDNPDATA_SUPPORT_OBJ4
      /* binary input and double bit binary input events */
      if (pDesc->group < 5)
      {
        TMWTYPES_UCHAR group = pDesc->group;
        /* This will update pDesc and pEvent if necessary */
        sdnputil_getObj2Or4Event(pDesc, &pEvent, classMask);
        if (group != pDesc->group)
        {
          /* Update the previous object header, before starting a new object */
          sdnpevnt_updateObjectHeader(pResponse, objHeaderOffset, numEventsInObject);
          numEventsInObject = 0;
        }
      }
#endif
#if SDNPDATA_SUPPORT_OBJ2 || SDNPDATA_SUPPORT_OBJ4
      if (pEvent != TMWDEFS_NULL)
        if((pDesc->group == DNPDEFS_OBJ_2_BIN_CHNG_EVENTS || pDesc->group == DNPDEFS_OBJ_4_DBL_CHNG_EVENTS)
          && pDesc->variation == 2 && pEvent->timeStamp.invalid)
      {
        /* Need to switch to variation 3 unsync */
        /* Update the previous object header, before starting a new object */
        sdnpevnt_updateObjectHeader(pResponse, objHeaderOffset, numEventsInObject);
        numEventsInObject = 0;
      }
#endif
    }

    /* If we have written any events into response update header with actual
     * number of events written
     */
    if(numEventsInObject > 0)
      sdnpevnt_updateObjectHeader(pResponse, objHeaderOffset, numEventsInObject);

    if(pEvent == TMWDEFS_NULL)
    {
      /* If all of the events were read, set number of points */
      pObjHeader->numberOfPoints = numEventsInResponse;
    }
    else if(numEventsInResponse < pObjHeader->numberOfPoints)
    {
      /* If not all events were read return status more data */
      status = SDNPSESN_READ_MORE_DATA;

      if (pointNumberAdded)
      {
        if (responseQual == DNPDEFS_QUAL_8BIT_INDEX)
        {
          /* Remove the point number */
          pResponse->msgLength--;
        }
        else
        {
          /* Remove the point number */
          pResponse->msgLength -= 2;
        }
      }

      /* If read of limited quantity, update this so multifrag response will use correct quantity */
      if((pObjHeader->qualifier == DNPDEFS_QUAL_8BIT_LIMITED_QTY)
        ||(pObjHeader->qualifier == DNPDEFS_QUAL_16BIT_LIMITED_QTY))
         pSDNPSession->readMultiFragNumberOfPoints = pObjHeader->numberOfPoints - numEventsInResponse;
    }
  }
  else
  {
    /* Set this to zero, so caller knows no events were found */
    pObjHeader->numberOfPoints = 0;
  }

  return(status);
}

#if SDNPDATA_SUPPORT_OBJ113
TMWTYPES_BOOL _pointMatch(
  SDNPEVNT_STRING *pEvent,
  DNPUTIL_RX_MSG *pRequest,
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{
  TMWTYPES_BOOL status = TMWDEFS_FALSE;
  if(pObjHeader->qualifier <= DNPDEFS_QUAL_16BIT_START_STOP)
  {
    if((pEvent->sdnp.point >= pObjHeader->firstPointNumber)
      &&(pEvent->sdnp.point <= pObjHeader->lastPointNumber))
    {
      status = TMWDEFS_TRUE;
    }
  }
  else 
  {
    TMWTYPES_USHORT pointNumber;
    TMWTYPES_USHORT pointIndex = 0;
    TMWTYPES_ULONG savedOffset = pRequest->offset;
    while(pointIndex < pObjHeader->numberOfPoints)
    {
      /* Get point number from point index */
      if(dnputil_getPointNumber(pRequest, pObjHeader, pointIndex, &pointNumber))
      {
        if(pEvent->sdnp.point == pointNumber)
        {
          status = TMWDEFS_TRUE;
          break;
        }
        pointIndex++;
      }
      status = TMWDEFS_FALSE;
    }
    pRequest->offset = savedOffset;
  }
  return status;
}

/* function: sdnpevnt_readVtermEvents */
SDNPSESN_READ_STATUS TMWDEFS_GLOBAL sdnpevnt_readVtermEvents(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRequest,
  TMWSESN_TX_DATA *pResponse,
  DNPUTIL_OBJECT_HEADER *pObjHeader,
  SDNPEVNT_DESC *pDesc)
{
  TMWTYPES_USHORT numberOfPointsToRemove = 0;
  SDNPSESN_READ_STATUS status = SDNPSESN_READ_COMPLETE;

  if((pObjHeader->qualifier == DNPDEFS_QUAL_8BIT_INDEX)
    ||(pObjHeader->qualifier == DNPDEFS_QUAL_16BIT_INDEX))
  {
    numberOfPointsToRemove = pObjHeader->numberOfPoints;
  }

  /* Count events in queue to see if we have any that match request */
  if((sdnpevnt_countEvents(pSession, 0x7, pDesc, TMWDEFS_FALSE, 1)) > 0)
  {
    SDNPEVNT_STRING *pEvent;
#if SDNPCNFG_USER_MANAGED_EVENTS || SDNPDATA_SUPPORT_OBJ120
    SDNPSESN *pSDNPSession = (SDNPSESN *)pDesc->pSession;
#endif
    TMWTYPES_USHORT maxMsgLength = 0;
    TMWTYPES_USHORT numEventsInObject = 0;
    TMWTYPES_USHORT objHeaderOffset = 0;
    TMWTYPES_UCHAR variation = 0;
    TMWTYPES_UCHAR responseQual = DNPDEFS_QUAL_8BIT_INDEX;
#if SDNPCNFG_USER_MANAGED_EVENTS
    SDNPEVNT_DATA event;
#endif
    TMWTYPES_UCHAR maxStringLength;

    /* Maximum string length is request variation */
    maxStringLength = pDesc->variation;
    if(maxStringLength == 0) maxStringLength = 255;

    /* Start adding events */
#if SDNPCNFG_USER_MANAGED_EVENTS
    pEvent = (SDNPEVNT_STRING *)sdnpevent_getFirstEvent(pDesc, 0x7, &event);
#else
    pEvent = (SDNPEVNT_STRING *)tmwdlist_getFirst(pDesc->pEventList);
#endif

    maxMsgLength = pResponse->maxLength;
#if SDNPDATA_SUPPORT_OBJ120
    if (pSDNPSession->authenticationEnabled && pSDNPSession->preChallengeApplConf)
      /* leave room for challenge object g120v1 at end of fragment */
      maxMsgLength -=32;
#endif

    while(pEvent != TMWDEFS_NULL)
    {
      TMWTYPES_UCHAR length = 0;

      /* See if this point matches request */
      if(_pointMatch(pEvent, pRequest, pObjHeader))
      {
        /* See if string will be truncated */
        length = pEvent->strLength;
        if(pDesc->truncateStrings && (length > maxStringLength))
          length = maxStringLength;

        /* Make sure point is still in the database and the string 
         * length is the same as the current object 
         */
        if(length != variation)
        {
          /* If we have written any events into response update header with actual
          * number of events written and reset number of events in object.
          */
          if(numEventsInObject > 0)
          {
            sdnpevnt_updateObjectHeader(pResponse, objHeaderOffset, numEventsInObject);
            numEventsInObject = 0;
          }

          /* Variation is string length */
          variation = length;
        }
        
        /* Make sure this event will fit in response */
        if(((pResponse->msgLength + variation + 2) >= maxMsgLength)
          || ((responseQual == DNPDEFS_QUAL_8BIT_INDEX) && (numEventsInObject == 255)))
        {
          /* since pEvent is not NULL, we will know we have more events */
          break;
        }

        /* See if this is the first point in this object */
        if(numEventsInObject == 0)
        {
          TMWTYPES_USHORT numberOfEvents;

          /* Save offset to object header */
          objHeaderOffset = pResponse->msgLength;

          /* numberOfEvents is the initial value in the response header, but it gets set to the correct value
           * numberOfEvents IS used to determine the qualifier
           */
          numberOfEvents = sdnpevnt_countEvents(pSession, 0x7, pDesc, TMWDEFS_FALSE, 256);

          /* Write object header */
          if((status = sdnpevnt_initReadEventResponse(pResponse, 
            pObjHeader, pDesc->group, variation, pDesc->quantity-1, numberOfEvents,
            variation, &responseQual)) != SDNPSESN_READ_COMPLETE)
          {
            return(status);
          }
        }

        /* Write point number */
        if(responseQual == DNPDEFS_QUAL_8BIT_INDEX)
        {
          pResponse->pMsgBuf[pResponse->msgLength++] = (TMWTYPES_UCHAR)pEvent->sdnp.point;
        }
        else
        {
          tmwtarg_store16(&pEvent->sdnp.point, &pResponse->pMsgBuf[pResponse->msgLength]);
          pResponse->msgLength += 2;
        }

        /* Diagnostics */
        pDesc->pStringDiagFunc(pSession, pEvent->sdnp.point, pEvent->strBuf, length);

        /* Store event data into response */
        memcpy(pResponse->pMsgBuf + pResponse->msgLength, pEvent->strBuf, variation);
        pResponse->msgLength = (TMWTYPES_USHORT)(pResponse->msgLength + variation);

        /* Mark event for deletion when acknowledge received */
  #if SDNPCNFG_USER_MANAGED_EVENTS
        if(pSDNPSession->userManagedEvents)
        {
          sdnpdata_umEventSent(pSDNPSession->pDbHandle, pDesc->group, pEvent->sdnp.point);
        }
        else
  #endif
        pEvent->sdnp.eventSent = TMWDEFS_TRUE;

        /* Mark response so we know it contains events */
        pResponse->txFlags |= TMWSESN_TXFLAGS_CONTAINS_EVENTS;

        /* Keep track of how many events have been read */
        numEventsInObject += 1;

        DNPSTAT_SESN_EVENT_SENT(pDesc->pSession, pDesc->group, pEvent->sdnp.point);
      }

#if SDNPCNFG_USER_MANAGED_EVENTS
      pEvent = (SDNPEVNT_STRING *)sdnpevent_getNextEvent(pDesc, 0x7, (SDNPEVNT *)pEvent, &event);
#else      
      pEvent = (SDNPEVNT_STRING *)tmwdlist_getNext((TMWDLIST_MEMBER *)pEvent);
#endif
    } 

    /* If we have written any events into response update header with actual
     * number of events written
     */
    if(numEventsInObject > 0)
    {
      sdnpevnt_updateObjectHeader(pResponse, objHeaderOffset, numEventsInObject);
      /* update this so caller knows how many events are in response */
      pObjHeader->numberOfPoints = numEventsInObject;
    }

    if(pEvent != TMWDEFS_NULL)
    {
      /* If not all events were read return status more data */
      status = SDNPSESN_READ_MORE_DATA;
    }
  }
  else
  {
    /* Set this to zero, so caller knows no events were found */
    pObjHeader->numberOfPoints = 0;
  }

  if(status == SDNPSESN_READ_COMPLETE)
  {
    if((pObjHeader->qualifier == DNPDEFS_QUAL_8BIT_INDEX)
      ||(pObjHeader->qualifier == DNPDEFS_QUAL_16BIT_INDEX))
    {
      /* if qualifier index, remove the point indexes in the request, so the next object can be parsed */
      TMWTYPES_USHORT pointNumber;
      TMWTYPES_USHORT pointIndex = 0;
      while(pointIndex < numberOfPointsToRemove)
      {
        dnputil_getPointNumber(pRequest, pObjHeader, pointIndex, &pointNumber);
        pointIndex++;
      }
    }
  }
  return(status);
}
#endif

#if (SDNPDATA_SUPPORT_OBJ111 || SDNPDATA_SUPPORT_OBJ113)
/* function: sdnpevnt_readStringEvents */
SDNPSESN_READ_STATUS TMWDEFS_GLOBAL sdnpevnt_readStringEvents(
  TMWSESN *pSession,
  TMWSESN_TX_DATA *pResponse, 
  DNPUTIL_OBJECT_HEADER *pObjHeader, 
  TMWDEFS_CLASS_MASK classMask,
  SDNPEVNT_DESC *pDesc)
{
  SDNPSESN_READ_STATUS status = SDNPSESN_READ_COMPLETE;

  /* Count events in queue to see if we have any that match request */
  if((sdnpevnt_countEvents(pSession, classMask, pDesc, TMWDEFS_FALSE, 1)) > 0)
  {
    SDNPEVNT_STRING *pEvent;
    SDNPSESN *pSDNPSession = (SDNPSESN *)pDesc->pSession;
    TMWTYPES_USHORT maxMsgLength = 0;
    TMWTYPES_USHORT totalEventsInResponse = 0;
    TMWTYPES_USHORT numEventsInObject = 0;
    TMWTYPES_USHORT objHeaderOffset = 0;
    TMWTYPES_UCHAR variation = 0;
    TMWTYPES_UCHAR responseQual = DNPDEFS_QUAL_8BIT_INDEX;
#if SDNPCNFG_USER_MANAGED_EVENTS
    SDNPEVNT_DATA event;
#endif
    TMWTYPES_UCHAR maxStringLength;

    /* Maximum string length is request variation */
    maxStringLength = pDesc->variation;
    if(maxStringLength == 0) maxStringLength = 255;

    /* Validate qualifier and initialize object header */
    if(!_validateQualifier(pObjHeader))
      return(SDNPSESN_READ_FAILED);
 
    if((pSDNPSession->readStatus == SDNPSESN_READ_MORE_DATA)
      && ((pObjHeader->qualifier == DNPDEFS_QUAL_8BIT_LIMITED_QTY)
          ||(pObjHeader->qualifier == DNPDEFS_QUAL_16BIT_LIMITED_QTY)))
    {
      pObjHeader->numberOfPoints = pSDNPSession->readMultiFragNumberOfPoints;
    }

    /* Start adding events */  
#if SDNPCNFG_USER_MANAGED_EVENTS
    pEvent = (SDNPEVNT_STRING *)sdnpevent_getFirstEvent(pDesc, classMask, &event);    
#else
    pEvent = (SDNPEVNT_STRING *)tmwdlist_getFirst(pDesc->pEventList);
#endif

    maxMsgLength = pResponse->maxLength;
#if SDNPDATA_SUPPORT_OBJ120
    if (pSDNPSession->authenticationEnabled && pSDNPSession->preChallengeApplConf)
      /* leave room for challenge object g120v1 at end of fragment */
      maxMsgLength -= 32;
#endif

    while(pEvent != TMWDEFS_NULL)
    {
      TMWTYPES_UCHAR length = 0;

      /* See if this point's class matches request */
      if((pEvent->sdnp.classMask & classMask) != 0)
      {
        /* See if string will be truncated */
        length = pEvent->strLength;
        if(pDesc->truncateStrings && (length > maxStringLength))
          length = maxStringLength;

        /* Make sure point is still in the database and the string 
        * length is the same as the current object 
        */
        if(length != variation)
        {
          /* If we have written any events into response update header with actual
          * number of events written and reset number of events in object.
          */
          if(numEventsInObject > 0)
          {
            sdnpevnt_updateObjectHeader(pResponse, objHeaderOffset, numEventsInObject);
            numEventsInObject = 0;
          }

          /* Variation is string length */
          variation = length;
        }
        
        /* Make sure this event will fit in response */
        if(((pResponse->msgLength + variation + 2) >= maxMsgLength)
          || ((responseQual == DNPDEFS_QUAL_8BIT_INDEX) && (numEventsInObject == 255)))
          break;

        /* See if this is the first point in this object */
        if(numEventsInObject == 0)
        {
          TMWTYPES_USHORT numberOfEvents;

          /* Save offset to object header */
          objHeaderOffset = pResponse->msgLength;

          /* numberOfEvents is the initial value in the response header, but it gets set to the correct value
           * numberOfEvents IS used to determine the qualifier
           */
          numberOfEvents = sdnpevnt_countEvents(pSession, classMask, pDesc, TMWDEFS_FALSE, 256);

          /* Write object header */
          if((status = sdnpevnt_initReadEventResponse(pResponse, 
            pObjHeader, pDesc->group, variation, pDesc->quantity-1, numberOfEvents,
            variation, &responseQual)) != SDNPSESN_READ_COMPLETE)
          {
            return(status);
          }
        }

        /* Write point number */
        if(responseQual == DNPDEFS_QUAL_8BIT_INDEX)
        {
          pResponse->pMsgBuf[pResponse->msgLength++] = (TMWTYPES_UCHAR)pEvent->sdnp.point;
        }
        else
        {
          tmwtarg_store16(&pEvent->sdnp.point, &pResponse->pMsgBuf[pResponse->msgLength]);
          pResponse->msgLength += 2;
        }

        /* Diagnostics */
        pDesc->pStringDiagFunc(pSession, pEvent->sdnp.point, pEvent->strBuf, length);

        /* Store event data into response */
        memcpy(pResponse->pMsgBuf + pResponse->msgLength, pEvent->strBuf, variation);
        pResponse->msgLength = (TMWTYPES_USHORT)(pResponse->msgLength + variation);

        /* Mark event for deletion when acknowledge received */
  #if SDNPCNFG_USER_MANAGED_EVENTS
        if(pSDNPSession->userManagedEvents)
        {
          sdnpdata_umEventSent(pSDNPSession->pDbHandle, pDesc->group, pEvent->sdnp.point);
        }
        else
  #endif
        pEvent->sdnp.eventSent = TMWDEFS_TRUE;

        /* Mark response so we know it contains events */
        pResponse->txFlags |= TMWSESN_TXFLAGS_CONTAINS_EVENTS;

        /* Keep track of how many events have been read */
        totalEventsInResponse += 1;
        numEventsInObject += 1;

        DNPSTAT_SESN_EVENT_SENT(pDesc->pSession, pDesc->group, pEvent->sdnp.point);

        /* Have we read all the events the request asked for */
        if(totalEventsInResponse >= pObjHeader->numberOfPoints)
          break;
      }

#if SDNPCNFG_USER_MANAGED_EVENTS
      pEvent = (SDNPEVNT_STRING *)sdnpevent_getNextEvent(pDesc, classMask, (SDNPEVNT *)pEvent, &event);
#else      
      pEvent = (SDNPEVNT_STRING *)tmwdlist_getNext((TMWDLIST_MEMBER *)pEvent);
#endif
    }

    /* If we have written any events into response update header with actual
     * number of events written
     */
    if(numEventsInObject > 0)
    {
      sdnpevnt_updateObjectHeader(pResponse, objHeaderOffset, numEventsInObject);
      /* update this so caller knows how many events are in response */
      pObjHeader->numberOfPoints = numEventsInObject;
    }

    if(pEvent == TMWDEFS_NULL)
    {
      /* If all of the events were read, set number of points */
      pObjHeader->numberOfPoints = totalEventsInResponse;
    }
    else if(totalEventsInResponse < pObjHeader->numberOfPoints)
    {
      /* If not all events were read return status more data */
      status = SDNPSESN_READ_MORE_DATA;

      /* If read of limited quantity, update this so multifrag response will use correct quantity */
      if((pObjHeader->qualifier == DNPDEFS_QUAL_8BIT_LIMITED_QTY)
        ||(pObjHeader->qualifier == DNPDEFS_QUAL_16BIT_LIMITED_QTY))
         pSDNPSession->readMultiFragNumberOfPoints = pObjHeader->numberOfPoints - totalEventsInResponse;
    }
  }
  else
  {
    /* Set this to zero, so caller knows no events were found */
    pObjHeader->numberOfPoints = 0;
  }

  return(status);
}
#endif

