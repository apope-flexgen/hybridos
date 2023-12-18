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

/* file: tmwsim.c
 * description: Implement generic simulation functionality that can be used
 *  for both master and slave simulations.
 */
#include "tmwscl/utils/tmwcnfg.h"
#include "tmwscl/utils/tmwtarg.h"
#include "tmwscl/utils/tmwsim.h"

#if TMWCNFG_USE_SIMULATED_DB 

#if !TMW_USE_BINARY_TREE
/* This implements a simple linked list table to hold the
 * TMWSIM_POINT structures. These functions could be replace
 * with functions that implement a more efficient sorted
 * tree structure.
 */

/* function: tmwsim_tableCreate */
TMWTYPES_BOOL TMWDEFS_CALLBACK tmwsim_tableCreate(
  TMWSIM_TABLE_HEAD *pTableHead)
{
  tmwdlist_initialize(pTableHead);
  return(TMWDEFS_TRUE);
}

/* function: tmwsim_tableDestroy */
void TMWDEFS_CALLBACK tmwsim_tableDestroy(
  TMWSIM_TABLE_HEAD *pTableHead)
{
  tmwdlist_destroy(pTableHead, (TMWMEM_FREE_FUNC)tmwsim_deletePoint);
} 

/* function: tmwsim_tableAdd */
TMWSIM_POINT * TMWDEFS_CALLBACK tmwsim_tableAdd(
  TMWSIM_TABLE_HEAD *pTableHead,
  TMWTYPES_ULONG pointNum)
{
  TMWSIM_POINT *pPoint;

  /* List must be kept in order, otherwise a range won't work     */
  /* Start at the end of list, since we often add points in order */
  TMWSIM_POINT *pEntry = TMWDEFS_NULL;

  pPoint = tmwsim_newPoint();
  if(pPoint == TMWDEFS_NULL)
    return(TMWDEFS_NULL);

  pEntry = (TMWSIM_POINT *)tmwdlist_getLast(pTableHead); 
  if((pEntry == TMWDEFS_NULL) || (pEntry->pointNumber < pointNum))
  {
    /* Yes, insert at end of queue */
    tmwdlist_addEntry(pTableHead, (TMWDLIST_MEMBER *)pPoint);
    return(pPoint);
  }

  /* Oh well, start at the beginning of list and see where to insert */
  pEntry = TMWDEFS_NULL;
  while((pEntry = (TMWSIM_POINT *)tmwdlist_getAfter(
    pTableHead, (TMWDLIST_MEMBER *)pEntry)) != TMWDEFS_NULL)
  {
    if(pointNum < pEntry->pointNumber)
      break;
  }

  /* See if we are at the end of the queue */
  if(pEntry != TMWDEFS_NULL)
  {
    /* No, insert before next event entry */
    tmwdlist_insertEntryBefore(pTableHead,
      (TMWDLIST_MEMBER *)pEntry, (TMWDLIST_MEMBER *)pPoint);
  }
  else
  {
    /* Yes, insert at end of queue */
    tmwdlist_addEntry(pTableHead, (TMWDLIST_MEMBER *)pPoint);
  }
  return(pPoint);
} 

/* function: tmwsim_tableDelete */
TMWTYPES_BOOL TMWDEFS_CALLBACK tmwsim_tableDelete(
  TMWSIM_TABLE_HEAD *pTableHead,
  TMWTYPES_ULONG pointNum)
{ 
  /* find the point structure in the list */
  void *pPoint = tmwsim_tableFindPoint(pTableHead, pointNum);
  if(pPoint != TMWDEFS_NULL)
  {
    /* remove it from the list and deallocate the memory */
    tmwdlist_removeEntry(pTableHead, (TMWDLIST_MEMBER *)pPoint);
    tmwsim_deletePoint((TMWSIM_POINT *)pPoint);
    return(TMWDEFS_TRUE);
  }
  return(TMWDEFS_FALSE);
}

/* function: tmwsim_tableFindPoint */
TMWSIM_POINT * TMWDEFS_CALLBACK tmwsim_tableFindPoint(
  TMWSIM_TABLE_HEAD *pTableHead,
  TMWTYPES_ULONG pointNum)
{
   TMWSIM_POINT *pPoint = (TMWSIM_POINT *)tmwdlist_getFirst(pTableHead);
   while(pPoint != TMWDEFS_NULL)
   {
     if(pPoint->pointNumber == pointNum)
       return(pPoint);

     pPoint = (TMWSIM_POINT *)tmwdlist_getNext((TMWDLIST_MEMBER *)pPoint);
  }
  return(TMWDEFS_NULL);
} 
  
/* function: tmwsim_tableGetFirstPoint */
TMWSIM_POINT * TMWDEFS_CALLBACK tmwsim_tableGetFirstPoint(
  TMWSIM_TABLE_HEAD *pTableHead)
{
  return((TMWSIM_POINT *)tmwdlist_getFirst(pTableHead));
} 

/* function: tmwsim_tableGetLastPoint */
TMWSIM_POINT * TMWDEFS_CALLBACK tmwsim_tableGetLastPoint(
  TMWSIM_TABLE_HEAD *pTableHead)
{
  return((TMWSIM_POINT *)tmwdlist_getLast(pTableHead));
} 

/* function: tmwsim_tableGetNextPoint */
TMWSIM_POINT * TMWDEFS_CALLBACK tmwsim_tableGetNextPoint(
  TMWSIM_TABLE_HEAD *pTableHead,
  TMWSIM_POINT *pPoint)
{
  /* If caller specified NULL then return the first point. */
  if(pPoint == TMWDEFS_NULL)
  {
    return(tmwsim_tableGetFirstPoint(pTableHead));
  }
  return((TMWSIM_POINT*)tmwdlist_getNext((TMWDLIST_MEMBER *)pPoint)); 
}

/* function: tmwsim_tableFindPointByIndex */
TMWSIM_POINT * TMWDEFS_CALLBACK tmwsim_tableFindPointByIndex(
  TMWSIM_TABLE_HEAD *pTableHead,
  TMWTYPES_USHORT pointIndex)
{
  int i;
  TMWSIM_POINT *pPoint = tmwsim_tableGetFirstPoint(pTableHead);
  for(i=0; i<pointIndex; i++)
  { 
    if(pPoint == TMWDEFS_NULL)
      break;
    pPoint = tmwsim_tableGetNextPoint(pTableHead, pPoint);
  }
  return(pPoint);
}

/* function: tmwsim_tableSize */
TMWTYPES_UINT TMWDEFS_CALLBACK tmwsim_tableSize(
  TMWSIM_TABLE_HEAD *pTableHead)
{
  return(tmwdlist_size(pTableHead));
}
#endif /* !TMW_USE_BINARY_TREE */

/* The following functions are used whether a simple list
 * or a more complicated tree structure is implemented to
 * organize the TMWSIM_POINT structures 
 */

/* function: tmwsim_newPoint */
TMWSIM_POINT *TMWDEFS_GLOBAL tmwsim_newPoint(void)
{
  return((TMWSIM_POINT *)tmwmem_alloc(TMWMEM_SIM_POINT_TYPE));
}

/* function: tmwsim_deletePoint */
void TMWDEFS_CALLBACK tmwsim_deletePoint(TMWSIM_POINT *pPoint)
{
  if(pPoint->deleteFlexPoint != TMWDEFS_NULL){
    pPoint->deleteFlexPoint(pPoint->flexPointHandle);
  }

  if((pPoint->type == TMWSIM_TYPE_STRING) || (pPoint->type == TMWSIM_TYPE_EXT_STRING))
  {
    tmwmem_free(pPoint->data.string.pBuf);
  }

  else if(pPoint->type == TMWSIM_TYPE_LIST)
  {
    tmwsim_tableDestroy(&pPoint->data.list.listHead);
  }
  else if((pPoint->type == TMWSIM_TYPE_ATTRIBUTE)
    &&(pPoint->data.attribute.pBuf != TMWDEFS_NULL))
  { 
    tmwmem_free(pPoint->data.attribute.pBuf);
  }
  tmwmem_free(pPoint);
}

/* function: tmwsim_setAssocPoint */
void TMWDEFS_CALLBACK tmwsim_setAssocPoint(
  TMWSIM_POINT *pPoint,
  TMWTYPES_ULONG assocPoint)
{
  pPoint->assocPointNumber = assocPoint;
}

/* function: tmwsim_getAssocPoint */
TMWTYPES_ULONG TMWDEFS_CALLBACK tmwsim_getAssocPoint(
  TMWSIM_POINT *pPoint)
{
  return (pPoint->assocPointNumber);
}

/* function: tmwsim_initPoint */
void TMWDEFS_GLOBAL tmwsim_initPoint(
  TMWSIM_POINT *pDataPoint,
  void *pHandle,
  TMWTYPES_ULONG pointNum,
  TMWSIM_TYPE type)
{
  TMWDTIME timeStamp;

  pDataPoint->pDbHandle = pHandle;
  pDataPoint->pSCLHandle = TMWDEFS_NULL;
  pDataPoint->managedPointHandle = TMWDEFS_NULL;
  pDataPoint->flexPointHandle = TMWDEFS_NULL;
  pDataPoint->deleteFlexPoint = TMWDEFS_NULL;
  pDataPoint->type = type;
  pDataPoint->pointNumber = pointNum;
  pDataPoint->assocPointNumber = TMWDEFS_NO_ASSOC_POINT;
  pDataPoint->flags = 0;
  pDataPoint->defaultEventVariation = 0;
  pDataPoint->defaultStaticVariation = 0;
  pDataPoint->bStored = TMWDEFS_FALSE;
  pDataPoint->local = TMWDEFS_FALSE;
  pDataPoint->changed = TMWDEFS_FALSE;
  pDataPoint->enabled = TMWDEFS_TRUE;
  pDataPoint->selectRequired = TMWDEFS_FALSE;
  pDataPoint->testingMode = 0;
  pDataPoint->reason = TMWDEFS_CHANGE_NONE;
  pDataPoint->cmdClassMask = TMWDEFS_CLASS_MASK_NONE;
  pDataPoint->classMask = TMWDEFS_CLASS_MASK_NONE;
  pDataPoint->groupMask = TMWDEFS_GROUP_MASK_GENERAL;
  memset(pDataPoint->description, 0, TMWSIM_MAX_DESC_LENGTH);
  pDataPoint->pCallbackFunc = TMWDEFS_NULL;
  pDataPoint->timeFormat = TMWDEFS_TIME_FORMAT_NONE;
  tmwdtime_getDateTime(TMWDEFS_NULL, &timeStamp);
  tmwsim_setTimeStamp(pDataPoint,&timeStamp);
  pDataPoint->freezeEnabled = TMWDEFS_FALSE;
  pDataPoint->freezeInterval = 0;
  tmwtimer_init(&pDataPoint->freezeTimer);

  switch(type)
  {
  case TMWSIM_TYPE_BINARY:
    pDataPoint->data.binary.value = TMWDEFS_FALSE;
    pDataPoint->data.binary.relativeTime = 0;
    pDataPoint->data.binary.faultNumber = 0;
    pDataPoint->data.binary.lastReportedValue = TMWDEFS_FALSE;
    break;

  case TMWSIM_TYPE_DOUBLE_BINARY:
    pDataPoint->data.doubleBinary.value = TMWDEFS_DPI_OFF;
    pDataPoint->data.doubleBinary.relativeTime = 0;
    pDataPoint->data.doubleBinary.lastReportedValue = TMWDEFS_DPI_OFF;
    break;

  case TMWSIM_TYPE_ANALOG:
    pDataPoint->data.analog.value = 0;
    pDataPoint->data.analog.deadband = 0;
    pDataPoint->data.analog.smoothing = 0;
    pDataPoint->data.analog.lastReportedValue = 0;
    pDataPoint->data.analog.lowLimit = TMWSIM_DATA_MIN;
    pDataPoint->data.analog.highLimit = TMWSIM_DATA_MAX;
    break;

  case TMWSIM_TYPE_COUNTER:
    pDataPoint->data.counter.value = 0;
    pDataPoint->data.counter.frozenValueChanged = TMWDEFS_FALSE;
    pDataPoint->data.counter.lastReportedValue = 0;
    tmwdtime_getDateTime(TMWDEFS_NULL, &pDataPoint->data.counter.timeOfFreeze);
    break;

  case TMWSIM_TYPE_INTEGRATED_TOTAL:
    pDataPoint->data.it.value = 0;
    break;

  case TMWSIM_TYPE_BITSTRING:
    pDataPoint->data.bitstring.value = 0;
    pDataPoint->data.bitstring.miscTime = 0;
    break;

  case TMWSIM_TYPE_PARAMETER:
    break;

  case TMWSIM_TYPE_REFERENCE:
    break;

  case TMWSIM_TYPE_LIST:
    tmwsim_tableCreate(&pDataPoint->data.list.listHead);
    break;

  case TMWSIM_TYPE_ATTRIBUTE:
    pDataPoint->data.attribute.pBuf = TMWDEFS_NULL;
    pDataPoint->data.attribute.length = 0;
    pDataPoint->data.attribute.value = 0;
    break;

  case TMWSIM_TYPE_STRING:
    pDataPoint->data.string.extString = TMWDEFS_FALSE;
    pDataPoint->data.string.pBuf = (TMWTYPES_UCHAR *)tmwmem_alloc(TMWMEM_SIM_STRING_TYPE);
    if(pDataPoint->data.string.pBuf != TMWDEFS_NULL)
      memset(pDataPoint->data.string.pBuf, 0, TMWSIM_STRING_MAX_LENGTH);
    break;

#if TMWCNFG_SIM_SUPPORT_EXT_STRINGS
  case TMWSIM_TYPE_EXT_STRING:
    pDataPoint->data.string.extString = TMWDEFS_TRUE;
    pDataPoint->data.string.pBuf = (TMWTYPES_UCHAR *)tmwmem_alloc(TMWMEM_SIM_EXT_STRING_TYPE);
    if(pDataPoint->data.string.pBuf != TMWDEFS_NULL)
      memset(pDataPoint->data.string.pBuf, 0, TMWSIM_EXT_STRING_MAX_LENGTH);
    break;
#endif

  case TMWSIM_TYPE_TIME_INDEXED:
    pDataPoint->data.indexedTime.intervalCount = 0;
    pDataPoint->data.indexedTime.intervalUnit = 0;
    tmwdtime_getDateTime(TMWDEFS_NULL, &pDataPoint->data.indexedTime.timeValue);
    break;
  }

}

/* function: tmwsim_getPointNumber */
TMWTYPES_ULONG TMWDEFS_GLOBAL tmwsim_getPointNumber(
  TMWSIM_POINT *pDataPoint)
{
  return(pDataPoint->pointNumber);
}

/* function: tmwsim_getEnabled */
TMWTYPES_BOOL TMWDEFS_GLOBAL tmwsim_getEnabled(
  TMWSIM_POINT *pDataPoint)
{
  return(pDataPoint->enabled);
}

/* function: tmwsim_setEnabled */
void TMWDEFS_GLOBAL tmwsim_setEnabled(
  TMWSIM_POINT *pDataPoint, 
  TMWTYPES_BOOL enabled)
{
  pDataPoint->enabled = enabled;
}

/* function: tmwsim_getLocal */
TMWTYPES_BOOL TMWDEFS_GLOBAL tmwsim_getLocal(
  TMWSIM_POINT *pDataPoint)
{
  return(pDataPoint->local);
}

/* function: tmwsim_setLocal */
void TMWDEFS_GLOBAL tmwsim_setLocal(
  TMWSIM_POINT *pDataPoint, 
  TMWTYPES_BOOL state)
{
  pDataPoint->local = state;
}

/* function: tmwsim_getSelectRequired */
TMWTYPES_BOOL TMWDEFS_GLOBAL tmwsim_getSelectRequired(
  TMWSIM_POINT *pDataPoint)
{
  return(pDataPoint->selectRequired);
}

/* function: tmwsim_setSelectRequired */
void TMWDEFS_GLOBAL tmwsim_setSelectRequired(
  TMWSIM_POINT *pDataPoint, 
  TMWTYPES_BOOL selectRequired)
{
  pDataPoint->selectRequired = selectRequired;
}

/* function: tmwsim_getTestingMode */
TMWTYPES_UCHAR TMWDEFS_GLOBAL tmwsim_getTestingMode(
  TMWSIM_POINT *pDataPoint)
{
  return(pDataPoint->testingMode);
}

/* function: tmwsim_setTestingMode */
void TMWDEFS_GLOBAL tmwsim_setTestingMode(
  TMWSIM_POINT *pDataPoint, 
  TMWTYPES_UCHAR mode)
{
  pDataPoint->testingMode = mode;
}  

/* function: tmwsim_getTimeFormat */
TMWDEFS_TIME_FORMAT TMWDEFS_GLOBAL tmwsim_getTimeFormat(
  TMWSIM_POINT *pDataPoint)
{
  return(pDataPoint->timeFormat);
}

/* function: tmwsim_setTimeFormat */
void TMWDEFS_GLOBAL tmwsim_setTimeFormat(
  TMWSIM_POINT *pDataPoint,
  TMWDEFS_TIME_FORMAT timeFormat)
{
  pDataPoint->timeFormat = timeFormat;
}

/* function: tmwsim_getTimeStamp */
void TMWDEFS_GLOBAL tmwsim_getTimeStamp(
  TMWSIM_POINT *pDataPoint,
  TMWDTIME *pTimeStamp)
{
  *pTimeStamp = pDataPoint->timeStamp;
}

/* function: tmwsim_setTimeStamp */
void TMWDEFS_GLOBAL tmwsim_setTimeStamp(
  TMWSIM_POINT *pDataPoint,
  TMWDTIME *pTimeStamp)
{
  pDataPoint->timeStamp = *pTimeStamp;
}

/* function: tmwsim_getFlags */
TMWTYPES_UCHAR TMWDEFS_GLOBAL tmwsim_getFlags(
  TMWSIM_POINT *pDataPoint)
{ 
  return(pDataPoint->flags);
}

/* function: tmwsim_setFlags */
void TMWDEFS_GLOBAL tmwsim_setFlags(
  TMWSIM_POINT *pDataPoint, 
  TMWTYPES_UCHAR flags,
  TMWDEFS_CHANGE_REASON reason)
{
  tmwdtime_getDateTime(pDataPoint->pSCLHandle, &pDataPoint->timeStamp);

  pDataPoint->flags = flags; 
  pDataPoint->changed = TMWDEFS_TRUE;
  pDataPoint->reason = reason;
}

/* function: tmwsim_getReason */
TMWDEFS_CHANGE_REASON TMWDEFS_GLOBAL tmwsim_getReason(
  TMWSIM_POINT *pDataPoint)
{ 
  return(pDataPoint->reason);
}

/* function: tmwsim_setReason */
void TMWDEFS_GLOBAL tmwsim_setReason( 
  TMWSIM_POINT *pDataPoint, 
  TMWDEFS_CHANGE_REASON reason)
{ 
  pDataPoint->reason = reason;
}

/* function: tmwsim_getEventClass */
TMWDEFS_CLASS_MASK TMWDEFS_GLOBAL tmwsim_getEventClass(
  TMWSIM_POINT *pDataPoint)
{
  return(pDataPoint->classMask);
}

/* function: tmwsim_setEventClass */
void TMWDEFS_GLOBAL tmwsim_setEventClass(
  TMWSIM_POINT *pDataPoint, 
  TMWDEFS_CLASS_MASK classMask)
{
  pDataPoint->classMask = classMask;
}

/* function: tmwsim_getCmdEventClass */
TMWDEFS_CLASS_MASK TMWDEFS_GLOBAL tmwsim_getCmdEventClass(
  TMWSIM_POINT *pDataPoint)
{
  return(pDataPoint->cmdClassMask);
}

/* function: tmwsim_setCmdEventClass */
void TMWDEFS_GLOBAL tmwsim_setCmdEventClass(
  TMWSIM_POINT *pDataPoint, 
  TMWDEFS_CLASS_MASK classMask)
{
  pDataPoint->cmdClassMask = classMask;
}

/* function: tmwsim_getDefEventVariation */
TMWTYPES_UCHAR TMWDEFS_GLOBAL tmwsim_getDefEventVariation(
  TMWSIM_POINT *pDataPoint)
{
  return(pDataPoint->defaultEventVariation);
}

/* function: tmwsim_setDefEventVariation */
void TMWDEFS_GLOBAL tmwsim_setDefEventVariation(
  TMWSIM_POINT *pDataPoint, 
  TMWTYPES_UCHAR defaultVariation)
{
  pDataPoint->defaultEventVariation = defaultVariation;
}
  
/* function: tmwsim_getDefStaticVariation */
TMWTYPES_UCHAR TMWDEFS_GLOBAL tmwsim_getDefStaticVariation(
  TMWSIM_POINT *pDataPoint)
{
  return(pDataPoint->defaultStaticVariation);
}

/* function: tmwsim_setDefStaticVariation */
void TMWDEFS_GLOBAL tmwsim_setDefStaticVariation(
  TMWSIM_POINT *pDataPoint, 
  TMWTYPES_UCHAR defaultVariation)
{
  pDataPoint->defaultStaticVariation = defaultVariation;
}

/* function: tmwsim_getGroupMask */
TMWDEFS_GROUP_MASK TMWDEFS_GLOBAL tmwsim_getGroupMask(
  TMWSIM_POINT *pDataPoint)
{
  return(pDataPoint->groupMask);
}

/* function: tmwsim_setGroupMask */
void TMWDEFS_GLOBAL tmwsim_setGroupMask(
  TMWSIM_POINT *pDataPoint, 
  TMWDEFS_GROUP_MASK groupMask)
{
  pDataPoint->groupMask = groupMask;
}

/* function: tmwsim_getChanged */
TMWTYPES_BOOL TMWDEFS_GLOBAL tmwsim_getChanged(
  TMWSIM_POINT *pDataPoint, 
  TMWDEFS_CHANGE_REASON *pReason)
{
  if(pDataPoint->changed)
  {
    *pReason = pDataPoint->reason;
    pDataPoint->changed = TMWDEFS_FALSE;
    return(TMWDEFS_TRUE);
  }

  return(TMWDEFS_FALSE);
}

/* function: tmwsim_setDescription */
void TMWDEFS_GLOBAL tmwsim_setDescription(
  TMWSIM_POINT *pDataPoint, 
  TMWTYPES_CHAR *pDesc)
{
  memset(pDataPoint->description, 0, TMWSIM_MAX_DESC_LENGTH);
  if(pDesc != TMWDEFS_NULL)
  {
    STRCPY(pDataPoint->description, TMWSIM_MAX_DESC_LENGTH, pDesc);
  }
}

/* function: tmwsim_getDescription */
TMWTYPES_CHAR * TMWDEFS_GLOBAL tmwsim_getDescription(
  TMWSIM_POINT *pDataPoint)
{
  return pDataPoint->description;
}

/* function: tmwsim_initIntegratedTotal */
void TMWDEFS_GLOBAL tmwsim_initIntegratedTotal(
  TMWSIM_POINT *pDataPoint, 
  void *pHandle,
  TMWTYPES_ULONG pointNum)
{
  tmwsim_initPoint(pDataPoint, pHandle, pointNum, TMWSIM_TYPE_BINARY);
}

/* function: tmwsim_getIntegratedTotalValue */
TMWTYPES_LONG TMWDEFS_GLOBAL tmwsim_getIntegratedTotalValue(
  TMWSIM_POINT *pDataPoint)
{
  return(pDataPoint->data.it.value);
}

/* function: tmwsim_setIntegratedTotalValue */
TMWTYPES_BOOL TMWDEFS_GLOBAL tmwsim_setIntegratedTotalValue(
  TMWSIM_POINT *pDataPoint, 
  TMWTYPES_LONG value, 
  TMWDEFS_CHANGE_REASON reason)
{
  TMWDTIME timeStamp;
  tmwdtime_getDateTime(TMWDEFS_NULL, &timeStamp);
  tmwsim_setTimeStamp(pDataPoint,&timeStamp);

  pDataPoint->data.it.value = value;
  pDataPoint->changed = TMWDEFS_TRUE;
  pDataPoint->reason = reason;

  if(pDataPoint->pCallbackFunc) 
  {
    pDataPoint->pCallbackFunc(pDataPoint->pCallbackParam, pDataPoint);
  }

  return(TMWDEFS_TRUE);
}

/* function: tmwsim_initBinary */
void TMWDEFS_GLOBAL tmwsim_initBinary(
  TMWSIM_POINT *pDataPoint, 
  void *pHandle,
  TMWTYPES_ULONG pointNum)
{
  tmwsim_initPoint(pDataPoint, pHandle, pointNum, TMWSIM_TYPE_BINARY);
}

/* function: tmwsim_getBinaryValue */
TMWTYPES_BOOL TMWDEFS_GLOBAL tmwsim_getBinaryValue(
  TMWSIM_POINT *pDataPoint)
{
  return(pDataPoint->data.binary.value);
}

/* function: tmwsim_setBinaryValue */
TMWTYPES_BOOL TMWDEFS_GLOBAL tmwsim_setBinaryValue(
  TMWSIM_POINT *pDataPoint, 
  TMWTYPES_BOOL value, 
  TMWDEFS_CHANGE_REASON reason)
{
  TMWDTIME timeStamp;
  tmwdtime_getDateTime(TMWDEFS_NULL, &timeStamp);
  tmwsim_setTimeStamp(pDataPoint,&timeStamp);

  pDataPoint->data.binary.value = value;
  pDataPoint->reason = reason;
  if (value != pDataPoint->data.binary.lastReportedValue)
  {
    pDataPoint->changed = TMWDEFS_TRUE;
    pDataPoint->data.binary.lastReportedValue = value;
  }

  if(pDataPoint->pCallbackFunc) 
  {
    pDataPoint->pCallbackFunc(pDataPoint->pCallbackParam, pDataPoint);
  }

  return(TMWDEFS_TRUE);
}

/* function: tmwsim_setBinaryTime */
TMWTYPES_BOOL TMWDEFS_GLOBAL tmwsim_setBinaryTime(
  TMWSIM_POINT *pDataPoint, 
  TMWTYPES_USHORT relativeTime)
{
  pDataPoint->data.binary.relativeTime = relativeTime;
  return(TMWDEFS_TRUE);
}

/* function: tmwsim_setBinaryFault */
TMWTYPES_BOOL TMWDEFS_GLOBAL tmwsim_setBinaryFault(
  TMWSIM_POINT *pDataPoint, 
  TMWTYPES_USHORT faultNumber)
{
  pDataPoint->data.binary.faultNumber = faultNumber;
  return(TMWDEFS_TRUE);
}

/* function: tmwsim_initDoubleBinary */
void TMWDEFS_GLOBAL tmwsim_initDoubleBinary(
  TMWSIM_POINT *pDataPoint, 
  void *pHandle,
  TMWTYPES_ULONG pointNum)
{
  tmwsim_initPoint(pDataPoint, pHandle, pointNum, TMWSIM_TYPE_DOUBLE_BINARY);
}

/* function: tmwsim_getDoubleBinaryValue */
TMWDEFS_DPI_TYPE TMWDEFS_GLOBAL tmwsim_getDoubleBinaryValue(
  TMWSIM_POINT *pDataPoint)
{
  return(pDataPoint->data.doubleBinary.value);
}

/* function: tmwsim_setDoubleBinaryValue */
TMWTYPES_BOOL TMWDEFS_GLOBAL tmwsim_setDoubleBinaryValue(
  TMWSIM_POINT *pDataPoint, 
  TMWDEFS_DPI_TYPE value, 
  TMWDEFS_CHANGE_REASON reason)
{
  TMWDTIME timeStamp;
  tmwdtime_getDateTime(TMWDEFS_NULL, &timeStamp);
  tmwsim_setTimeStamp(pDataPoint,&timeStamp);

  pDataPoint->data.doubleBinary.value = value;
  pDataPoint->reason = reason;

  if (value != pDataPoint->data.doubleBinary.lastReportedValue)
  {
    pDataPoint->changed = TMWDEFS_TRUE;
    pDataPoint->data.doubleBinary.lastReportedValue = value;
  }

  if(pDataPoint->pCallbackFunc) 
  {
    pDataPoint->pCallbackFunc(pDataPoint->pCallbackParam, pDataPoint);
  }

  return(TMWDEFS_TRUE);
}

/* function: tmwsim_initCounter */
void TMWDEFS_GLOBAL tmwsim_initCounter(
  TMWSIM_POINT *pDataPoint, 
  void *pHandle,
  TMWTYPES_ULONG pointNum)
{
  tmwsim_initPoint(pDataPoint, pHandle, pointNum, TMWSIM_TYPE_COUNTER);
}

/* function: tmwsim_getCounterValue */
TMWTYPES_ULONG TMWDEFS_GLOBAL tmwsim_getCounterValue(
  TMWSIM_POINT *pDataPoint)
{
  return(pDataPoint->data.counter.value);
}

/* function: tmwsim_getFrozenCounterValue */
TMWTYPES_ULONG TMWDEFS_GLOBAL tmwsim_getFrozenCounterValue(
  TMWSIM_POINT *pDataPoint)
{
  return(pDataPoint->data.counter.frozenValue);
}

/* function: tmwsim_setCounterValue */
TMWTYPES_BOOL TMWDEFS_GLOBAL tmwsim_setCounterValue(
  TMWSIM_POINT *pDataPoint, 
  TMWTYPES_ULONG value, 
  TMWDEFS_CHANGE_REASON reason)
{
  TMWDTIME timeStamp;
  tmwdtime_getDateTime(TMWDEFS_NULL, &timeStamp);
  tmwsim_setTimeStamp(pDataPoint,&timeStamp);

  pDataPoint->data.counter.value = value;
  pDataPoint->reason = reason;

  if (value != pDataPoint->data.counter.lastReportedValue)
  {
    pDataPoint->changed = TMWDEFS_TRUE;
    pDataPoint->data.counter.lastReportedValue = value;
  }

  if(pDataPoint->pCallbackFunc) 
  {
    pDataPoint->pCallbackFunc(pDataPoint->pCallbackParam, pDataPoint);
  }

  return(TMWDEFS_TRUE);
}

/* function: tmwsim_getDefFrznStaticVar */
TMWTYPES_UCHAR TMWDEFS_GLOBAL xtmwsim_getDefFrznStaticVar(
  TMWSIM_POINT *pDataPoint)
{
  return(pDataPoint->data.counter.defaultFrznStaticVariation);
}

/* function: tmwsim_setDefFrznStaticVar */
void TMWDEFS_GLOBAL xtmwsim_setDefFrznStaticVar(
  TMWSIM_POINT *pDataPoint, 
  TMWTYPES_UCHAR defaultVariation)
{
  pDataPoint->data.counter.defaultFrznStaticVariation = defaultVariation;
}

/* function: tmwsim_getDefFrznEventVar */
TMWTYPES_UCHAR TMWDEFS_GLOBAL xtmwsim_getDefFrznEventVar(
  TMWSIM_POINT *pDataPoint)
{
  return(pDataPoint->data.counter.defaultFrznEventVariation);
}

/* function: tmwsim_setDefFrznEventVar */
void TMWDEFS_GLOBAL xtmwsim_setDefFrznEventVar(
  TMWSIM_POINT *pDataPoint, 
  TMWTYPES_UCHAR defaultVariation)
{
  pDataPoint->data.counter.defaultFrznEventVariation = defaultVariation;
}

/* function: tmwsim_freezeCounter */
TMWTYPES_BOOL TMWDEFS_GLOBAL tmwsim_freezeCounter(
  TMWSIM_POINT *pDataPoint)
{
  pDataPoint->data.counter.frozenValue = pDataPoint->data.counter.value;
  tmwdtime_getDateTime(TMWDEFS_NULL, &pDataPoint->data.counter.timeOfFreeze);
  pDataPoint->data.counter.frozenValueChanged = TMWDEFS_TRUE;
  return(TMWDEFS_TRUE);
}

/* function: tmwsim_initAnalog */
void TMWDEFS_GLOBAL tmwsim_initAnalog(
  TMWSIM_POINT *pDataPoint, 
  void *pHandle,
  TMWTYPES_ULONG pointNum,
  TMWSIM_DATA_TYPE lowLimit,
  TMWSIM_DATA_TYPE highLimit,
  TMWSIM_DATA_TYPE deadband,
  TMWSIM_DATA_TYPE smoothing)
{
  tmwsim_initPoint(pDataPoint, pHandle, pointNum, TMWSIM_TYPE_ANALOG);
  pDataPoint->data.analog.lowLimit = lowLimit;
  pDataPoint->data.analog.highLimit = highLimit;
  pDataPoint->data.analog.deadband = deadband;
  pDataPoint->data.analog.smoothing = smoothing;
}

/* function: tmwsim_getAnalogValue */
TMWSIM_DATA_TYPE TMWDEFS_GLOBAL tmwsim_getAnalogValue(
  TMWSIM_POINT *pDataPoint)
{
  return(pDataPoint->data.analog.value);
}

/* function: tmwsim_getAnalogValue */
TMWSIM_DATA_TYPE TMWDEFS_GLOBAL tmwsim_getAnalogDeadband(
  TMWSIM_POINT *pDataPoint)
{
  return(pDataPoint->data.analog.deadband);
}

/* function: tmwsim_setAnalogValue */
TMWTYPES_BOOL TMWDEFS_GLOBAL tmwsim_setAnalogValue(
  TMWSIM_POINT *pDataPoint, 
  TMWSIM_DATA_TYPE value, 
  TMWDEFS_CHANGE_REASON reason)
{
  TMWDTIME timeStamp;
  TMWSIM_DATA_TYPE lastReported;

  tmwdtime_getDateTime(TMWDEFS_NULL, &timeStamp);
  tmwsim_setTimeStamp(pDataPoint,&timeStamp);

  lastReported = pDataPoint->data.analog.lastReportedValue;

  pDataPoint->data.analog.value = value;
  pDataPoint->reason = reason;

  if(value != lastReported)
  {
    if((value < pDataPoint->data.analog.lowLimit)
      || (value > pDataPoint->data.analog.highLimit)
      || ((value - lastReported) > pDataPoint->data.analog.deadband)
      || ((lastReported - value) > pDataPoint->data.analog.deadband))
    {
      pDataPoint->changed = TMWDEFS_TRUE;
      pDataPoint->data.analog.lastReportedValue = value;
    }
  }

  if(pDataPoint->pCallbackFunc) 
  {
    pDataPoint->pCallbackFunc(pDataPoint->pCallbackParam, pDataPoint);
  }

  return(TMWDEFS_TRUE);
}   

/* function: tmwsim_getDefDeadbandVariation */
TMWTYPES_UCHAR TMWDEFS_GLOBAL tmwsim_getDefDeadbandVariation(
  TMWSIM_POINT *pDataPoint)
{
  return(pDataPoint->data.analog.defaultDeadbandVariation);
}

/* function: tmwsim_setDefDeadbandVariation */
void TMWDEFS_GLOBAL tmwsim_setDefDeadbandVariation(
  TMWSIM_POINT *pDataPoint, 
  TMWTYPES_UCHAR defaultVariation)
{
  pDataPoint->data.analog.defaultDeadbandVariation = defaultVariation;
}

/* Counter Data Points */
void TMWDEFS_GLOBAL tmwsim_initBitstring(
  TMWSIM_POINT *pDataPoint, 
  void *pHandle,
  TMWTYPES_ULONG pointNum)
{
  tmwsim_initPoint(pDataPoint, pHandle, pointNum, TMWSIM_TYPE_BITSTRING);
}

TMWTYPES_ULONG TMWDEFS_GLOBAL tmwsim_getBitstringValue(
  TMWSIM_POINT *pDataPoint)
{
  return(pDataPoint->data.bitstring.value);
}

TMWTYPES_BOOL TMWDEFS_GLOBAL tmwsim_setBitstringValue(
  TMWSIM_POINT *pDataPoint, 
  TMWTYPES_ULONG value, 
  TMWDEFS_CHANGE_REASON reason)
{
  TMWDTIME timeStamp;
  tmwdtime_getDateTime(TMWDEFS_NULL, &timeStamp);
  tmwsim_setTimeStamp(pDataPoint,&timeStamp);

  pDataPoint->data.bitstring.value = value;
  pDataPoint->changed = TMWDEFS_TRUE;
  pDataPoint->reason = reason;

  if(pDataPoint->pCallbackFunc) 
  {
    pDataPoint->pCallbackFunc(pDataPoint->pCallbackParam, pDataPoint);
  }

  return(TMWDEFS_TRUE);
}

TMWTYPES_USHORT TMWDEFS_GLOBAL tmwsim_getBitstringTime(
  TMWSIM_POINT *pDataPoint)
{
  return(pDataPoint->data.bitstring.miscTime);
}

TMWTYPES_BOOL TMWDEFS_GLOBAL tmwsim_setBitstringTime(
  TMWSIM_POINT *pDataPoint, 
  TMWTYPES_USHORT value)
{
  pDataPoint->data.bitstring.miscTime = value;
  return(TMWDEFS_TRUE);
}

/* function: tmwsim_initParameter */
void TMWDEFS_GLOBAL tmwsim_initParameter(
  TMWSIM_POINT *pDataPoint,
  void *pHandle,
  TMWTYPES_ULONG pointNum,
  TMWSIM_POINT *pRefDataPoint)
{
  tmwsim_initPoint(pDataPoint, pHandle, pointNum, TMWSIM_TYPE_PARAMETER);
  pDataPoint->data.parameter.pPoint = pRefDataPoint;
}

/* function: tmwsim_initReference */
void TMWDEFS_GLOBAL tmwsim_initReference(
  TMWSIM_POINT *pDataPoint,
  void *pHandle,
  TMWTYPES_ULONG pointNum,
  TMWSIM_POINT *pRefDataPoint)
{
  tmwsim_initPoint(pDataPoint, pHandle, pointNum, TMWSIM_TYPE_REFERENCE);
  pDataPoint->data.reference.pPoint = pRefDataPoint;
}

/* Time and date - indexed abs time and long interval Points */
/* function: tmwsim_initIndexedTime */
void TMWDEFS_GLOBAL tmwsim_initIndexedTime(
  TMWSIM_POINT *pDataPoint,
  void *pHandle,
  TMWTYPES_ULONG pointNum)
{
  tmwsim_initPoint(pDataPoint, pHandle, pointNum, TMWSIM_TYPE_TIME_INDEXED);
  /* Get system date and time */
  tmwdtime_getDateTime((TMWSESN*)pDataPoint->pSCLHandle, &pDataPoint->data.indexedTime.timeValue);
  pDataPoint->data.indexedTime.intervalCount = 0;
  pDataPoint->data.indexedTime.intervalUnit = 0;
}

/* function: tmwsim_getIndexedTimeTime */
void TMWDEFS_GLOBAL tmwsim_getIndexedTimeTime(
  TMWSIM_POINT *pDataPoint,
  TMWDTIME *pTimeStamp)
{
  *pTimeStamp = pDataPoint->data.indexedTime.timeValue;
}

/* function: tmwsim_getIndexedTimeIntervalCount */
TMWTYPES_ULONG TMWDEFS_GLOBAL tmwsim_getIndexedTimeIntervalCount(
  TMWSIM_POINT *pDataPoint)
{
  return(pDataPoint->data.indexedTime.intervalCount);
}

/* function: tmwsim_getIndexedTimeIntervalUnit */
TMWTYPES_BYTE TMWDEFS_GLOBAL tmwsim_getIndexedTimeIntervalUnit(
  TMWSIM_POINT *pDataPoint)
{
  return(pDataPoint->data.indexedTime.intervalUnit);
}

/* function: tmwsim_setStartTime */
TMWTYPES_BOOL TMWDEFS_GLOBAL tmwsim_setStartTime(
  TMWSIM_POINT *pDataPoint,
  TMWDTIME timeStamp)
{
  pDataPoint->data.indexedTime.timeValue = timeStamp;

  if (pDataPoint->pCallbackFunc)
  {
    pDataPoint->pCallbackFunc(pDataPoint->pCallbackParam, pDataPoint);
  }

  return(TMWDEFS_TRUE);
}

/* function: tmwsim_setIntervalCount */
TMWTYPES_BOOL TMWDEFS_GLOBAL tmwsim_setIntervalCount(
  TMWSIM_POINT *pDataPoint,
  TMWTYPES_ULONG intervalCount)
{
  pDataPoint->data.indexedTime.intervalCount = intervalCount;

  if (pDataPoint->pCallbackFunc)
  {
    pDataPoint->pCallbackFunc(pDataPoint->pCallbackParam, pDataPoint);
  }

  return(TMWDEFS_TRUE);
}

/* function: tmwsim_setIntervalUnit */
TMWTYPES_BOOL TMWDEFS_GLOBAL tmwsim_setIntervalUnit(
  TMWSIM_POINT *pDataPoint,
  TMWTYPES_BYTE  intervalUnit)
{
  pDataPoint->data.indexedTime.intervalUnit = intervalUnit;

  if (pDataPoint->pCallbackFunc)
  {
    pDataPoint->pCallbackFunc(pDataPoint->pCallbackParam, pDataPoint);
  }

  return(TMWDEFS_TRUE);
}

/* function: tmwsim_setIndexedTime */
TMWTYPES_BOOL TMWDEFS_GLOBAL tmwsim_setIndexedTime(
  TMWSIM_POINT *pDataPoint,
  TMWDTIME timeValue,
  TMWTYPES_ULONG intervalCount,
  TMWTYPES_BYTE  intervalUnit)
{
  pDataPoint->data.indexedTime.timeValue = timeValue;
  pDataPoint->data.indexedTime.intervalCount = intervalCount;
  pDataPoint->data.indexedTime.intervalUnit = intervalUnit;
  
  if (pDataPoint->pCallbackFunc)
  {
    pDataPoint->pCallbackFunc(pDataPoint->pCallbackParam, pDataPoint);
  }

  return(TMWDEFS_TRUE);
}
 
/* String Data Points */
void TMWDEFS_GLOBAL tmwsim_initString(
  TMWSIM_POINT *pDataPoint, 
  void *pHandle,
  TMWTYPES_ULONG pointNum)
{
  tmwsim_initPoint(pDataPoint, pHandle, pointNum, TMWSIM_TYPE_STRING);
  if(pDataPoint->data.string.pBuf != TMWDEFS_NULL)
  {
    STRCPY((TMWTYPES_CHAR *)pDataPoint->data.string.pBuf, 2, " ");
    pDataPoint->data.string.length = (TMWTYPES_USHORT)(strlen((char *)pDataPoint->data.string.pBuf));
  }
}

TMWTYPES_BOOL TMWDEFS_GLOBAL tmwsim_getStringValue(
  TMWSIM_POINT *pDataPoint,
  TMWTYPES_UCHAR maxLength,
  TMWTYPES_UCHAR *pBuf,
  TMWTYPES_UCHAR *pBufLength)
{
  TMWTYPES_UCHAR length = (TMWTYPES_UCHAR)
    ((pDataPoint->data.string.length < maxLength) ?
    pDataPoint->data.string.length : maxLength);

  memcpy(pBuf, pDataPoint->data.string.pBuf, length);
  *pBufLength = length;

  return(TMWDEFS_TRUE);
}

TMWTYPES_BOOL TMWDEFS_GLOBAL tmwsim_setStringValue(
  TMWSIM_POINT *pDataPoint, 
  TMWTYPES_UCHAR *pBuf, 
  TMWTYPES_UCHAR bufLength,
  TMWDEFS_CHANGE_REASON reason)
{
  TMWDTIME timeStamp;
  tmwdtime_getDateTime(TMWDEFS_NULL, &timeStamp);
  tmwsim_setTimeStamp(pDataPoint, &timeStamp);

  if (bufLength > TMWSIM_STRING_MAX_LENGTH)
    bufLength = TMWSIM_STRING_MAX_LENGTH;

  memcpy(pDataPoint->data.string.pBuf, pBuf, bufLength);
  pDataPoint->data.string.length = bufLength;
  
  if(reason != TMWDEFS_CHANGE_NONE)
  {
    pDataPoint->changed = TMWDEFS_TRUE;
    pDataPoint->reason = reason;

    if(pDataPoint->pCallbackFunc) 
    {
      pDataPoint->pCallbackFunc(pDataPoint->pCallbackParam, pDataPoint);
    }
  }

  return(TMWDEFS_TRUE);
}

#if TMWCNFG_SIM_SUPPORT_EXT_STRINGS
/* String Data Points */
void TMWDEFS_GLOBAL tmwsim_initExtString(
  TMWSIM_POINT *pDataPoint,
  void *pHandle,
  TMWTYPES_ULONG pointNum)
{
  tmwsim_initPoint(pDataPoint, pHandle, pointNum, TMWSIM_TYPE_EXT_STRING);
  if (pDataPoint->data.string.pBuf != TMWDEFS_NULL)
  {
    STRCPY((TMWTYPES_CHAR *)pDataPoint->data.string.pBuf, 2, " ");
    pDataPoint->data.string.length = (TMWTYPES_USHORT)(strlen((char *)pDataPoint->data.string.pBuf));
  }
}

TMWTYPES_BOOL TMWDEFS_GLOBAL tmwsim_getExtStringValue(
  TMWSIM_POINT *pDataPoint,
  TMWTYPES_USHORT maxLength,
  TMWTYPES_UCHAR *pBuf,
  TMWTYPES_USHORT *pBufLength)
{
  TMWTYPES_USHORT length = (TMWTYPES_USHORT)
    ((pDataPoint->data.string.length < maxLength) ?
      pDataPoint->data.string.length : maxLength);

  memcpy(pBuf,pDataPoint->data.string.pBuf, length);
  *pBufLength = length;

  return(TMWDEFS_TRUE);
}

TMWTYPES_UCHAR * TMWDEFS_GLOBAL tmwsim_getExtStringPointer(
  TMWSIM_POINT *pDataPoint,
  TMWTYPES_USHORT *pBufLength)
{
  *pBufLength = pDataPoint->data.string.length;
  return(pDataPoint->data.string.pBuf);
}

TMWTYPES_BOOL TMWDEFS_GLOBAL tmwsim_setExtStringValue(
  TMWSIM_POINT *pDataPoint,
  TMWTYPES_UCHAR *pBuf,
  TMWTYPES_USHORT bufLength,
  TMWDEFS_CHANGE_REASON reason)
{
  TMWDTIME timeStamp;
  tmwdtime_getDateTime(TMWDEFS_NULL, &timeStamp);
  tmwsim_setTimeStamp(pDataPoint, &timeStamp);

  if (bufLength > TMWSIM_EXT_STRING_MAX_LENGTH)
    bufLength = TMWSIM_EXT_STRING_MAX_LENGTH;

  memcpy(pDataPoint->data.string.pBuf, pBuf, bufLength);
  pDataPoint->data.string.length = bufLength;

  if (reason != TMWDEFS_CHANGE_NONE)
  {
    pDataPoint->changed = TMWDEFS_TRUE;
    pDataPoint->reason = reason;

    if (pDataPoint->pCallbackFunc)
    {
      pDataPoint->pCallbackFunc(pDataPoint->pCallbackParam, pDataPoint);
    }
  }

  return(TMWDEFS_TRUE);
}
#endif

/* function: tmwsim_setCallbackFunc */
void TMWDEFS_GLOBAL tmwsim_setCallbackFunc(
  TMWSIM_POINT *pDataPoint,
  TMWSIM_CALLBACK_FUNC pCallbackFunc,
  void *pCallbackParam)
{
  pDataPoint->pCallbackFunc = pCallbackFunc;
  pDataPoint->pCallbackParam = pCallbackParam;
}

static TMWTYPES_BOOL _hasXmlSyntax(TMWTYPES_CHAR *origString, TMWTYPES_CHAR *escString, size_t maxEscLen)
{
  /* The characters &, <, & > cannot be used in their literal form so they must be escaped. */
  if ((strchr(origString, '&')) || (strchr(origString, '<')) || (strchr(origString, '>')))
  {
    size_t ctr;
    size_t escStrLen = 0;

    /* Ensure the string is always null terminated. */
    memset(escString, 0, maxEscLen);
    for (ctr = 0; ctr < strlen(origString); ctr++)
    {
      if (origString[ctr] == '&')
      {
        strcat(escString, "&amp;");
        escStrLen += 5;
      }
      else if (origString[ctr] == '<')
      {
        strcat(escString, "&lt;");
        escStrLen += 4;
      }
      else if (origString[ctr] == '>')
      {
        strcat(escString, "&gt;");
        escStrLen += 4;
      }
      else
      {
        escString[escStrLen] = origString[ctr];
        escStrLen++;
      }

      /* Ensure the expanded description string does not exceed its buffer. */
      if ((escStrLen + 5) > maxEscLen)
        break;
    }
    return TMWDEFS_TRUE;
  }
  return TMWDEFS_FALSE;
}

static TMWTYPES_BOOL _hasXmlSyntaxOctetString(TMWTYPES_CHAR *origString, TMWTYPES_USHORT strLength, TMWTYPES_CHAR *escString, size_t maxEscLen)
{
  /* The string will be converted to hex if there is an unprintable character.              */
  TMWTYPES_USHORT ctr;
  TMWTYPES_UINT escStrLen = 0;
  TMWTYPES_BOOL result = TMWDEFS_FALSE;

  /* Ensure the string is always null terminated. */
  memset(escString, 0, maxEscLen);
  escStrLen += tmwtarg_snprintf(escString + escStrLen, (TMWTYPES_UINT)maxEscLen - escStrLen, "hex(");
  for (ctr = 0; ctr < strLength; ctr++)
  {
    /* check for ASCII control character and ASCII extended character which is ">0x7e" */
    if ((origString[ctr] < 0x20) || (origString[ctr] > 0x7e))
    {
      result = TMWDEFS_TRUE;
    }

    sprintf(&escString[escStrLen], "%02x ", (TMWTYPES_UCHAR)origString[ctr]);
    escStrLen += 3;

    /* Ensure the expanded description string does not exceed its buffer.
     * This will truncate the string without indicating that.
     */
    if ((escStrLen + 4) > maxEscLen)
      break;
  }
  escStrLen += tmwtarg_snprintf(escString + escStrLen, (TMWTYPES_UINT)maxEscLen - escStrLen, ")");
  return result;
}

void tmwsim_xmlFormatDesc(TMWTYPES_CHAR *desc, TMWTYPES_CHAR *buf, size_t bufSize)
{
  /* Guard against buffer overrun by allocating worst case,      */
  /* description contains all "&" that must escaped with "&amp;" */
  TMWTYPES_CHAR escDesc[TMWSIM_MAX_DESC_LENGTH * 5 + 1];

  if (_hasXmlSyntax(desc, escDesc, sizeof(escDesc)))
  {
    desc = escDesc;
  }
  tmwtarg_snprintf(buf, (TMWTYPES_UINT)bufSize, "    <description>%s</description>\n", desc);
}

void tmwsim_xmlFormatValue(TMWTYPES_CHAR *value, TMWTYPES_USHORT strLength, TMWTYPES_CHAR *buf, size_t bufSize)
{
  /* Use a large buffer that should handle most strings that will not */
  /* require an excessively large stack to escape the value string.   */
  TMWTYPES_CHAR escValue[2048 + 1];

  if (_hasXmlSyntaxOctetString(value, strLength, escValue, sizeof(escValue)))
  {
    value = escValue;
  }
  else if (_hasXmlSyntax(value, escValue, sizeof(escValue)))
  {
    value = escValue;
  }

  if (strlen(value) > (bufSize - 20))
    value = "String was too large for workspace";

  tmwtarg_snprintf(buf, (TMWTYPES_UINT)bufSize, "    <value>%s</value>\n", value);
}

#endif /* TMWCNFG_USE_SIMULATED_DB */
