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

/* file: mdnpsim.h
 * description: Simulates a DNP master database.
 */
#include "tmwscl/dnp/dnpdiag.h"
#include "tmwscl/utils/tmwcnfg.h"

#if TMWCNFG_USE_SIMULATED_DB
#include "tmwscl/utils/tmwtarg.h"
#include "tmwscl/utils/tmwmsim.h"
#include "tmwscl/utils/tmwsim.h"
#include "tmwscl/utils/tmwdiag.h"

#include "tmwscl/dnp/mdnpsim.h"
#include "tmwscl/dnp/mdnpmem.h"


/* Forward Definitions */

#if MDNPDATA_SUPPORT_DATASETS
static void TMWDEFS_LOCAL _initDataSets(MDNPSIM_DATABASE *pDbHandle);
static void TMWDEFS_LOCAL _clearDataSets(MDNPSIM_DATABASE *pDbHandle);
#endif

/* Call update callback if set */
static void _callCallback(
  TMWSIM_POINT *pPoint,
  TMWSIM_EVENT_TYPE type,
  DNPDEFS_OBJ_GROUP_ID objectGroup,
  TMWTYPES_USHORT pointNumber)
{
  MDNPSIM_DATABASE *pDbHandle;
  
  pDbHandle = (MDNPSIM_DATABASE *)pPoint->pDbHandle;
  if(pDbHandle->pUpdateCallback != TMWDEFS_NULL)
  {
    pDbHandle->pUpdateCallback(pDbHandle->pUpdateCallbackParam, type, objectGroup, pointNumber);
  }
}

/* Call update callback when point is removed, if set */
static void _callRemoveCallback(
  MDNPSIM_DATABASE *pDbHandle,
  TMWSIM_EVENT_TYPE type,
  DNPDEFS_OBJ_GROUP_ID objectGroup,
  TMWTYPES_USHORT pointNumber)
{ 
  if(pDbHandle->pUpdateCallback != TMWDEFS_NULL)
  {
    pDbHandle->pUpdateCallback(pDbHandle->pUpdateCallbackParam, type, objectGroup, pointNumber);
  }
}

/* function: mdnpsim_init */
void * TMWDEFS_GLOBAL mdnpsim_init(
  TMWSESN *pSession)
{
  MDNPSIM_DATABASE *pDbHandle;

  pDbHandle = (MDNPSIM_DATABASE *)mdnpmem_alloc(MDNPMEM_SIM_DATABASE_TYPE);
  if(pDbHandle != TMWDEFS_NULL)
  {  
    memset(pDbHandle, 0, sizeof(MDNPSIM_DATABASE));

    pDbHandle->pMDNPSession = (MDNPSESN *)pSession;

    tmwsim_tableCreate(&pDbHandle->binaryInputs);
    tmwsim_tableCreate(&pDbHandle->doubleInputs);
    tmwsim_tableCreate(&pDbHandle->binaryOutputs);
    tmwsim_tableCreate(&pDbHandle->binaryCounters);
    tmwsim_tableCreate(&pDbHandle->frozenCounters);
    tmwsim_tableCreate(&pDbHandle->analogInputs);
    tmwsim_tableCreate(&pDbHandle->frozenAnalogInputs);
    tmwsim_tableCreate(&pDbHandle->analogOutputs);
    tmwsim_tableCreate(&pDbHandle->stringData);
    tmwsim_tableCreate(&pDbHandle->vtermEvents);
    tmwsim_tableCreate(&pDbHandle->deviceAttrs);
    tmwsim_tableCreate(&pDbHandle->authSecStatsFromOS);
    tmwsim_tableCreate(&pDbHandle->authSecStatsFromMaster);
    tmwsim_tableCreate(&pDbHandle->indexedTime);

#if MDNPDATA_SUPPORT_OBJ70
    pDbHandle->fileSimXferContext.pCurrentLocalFileHandle = TMWDEFS_NULL;
    pDbHandle->fileSimXferContext.authKey = 0UL;
#endif

#if MDNPDATA_SUPPORT_DATASETS
   _initDataSets(pDbHandle);
#endif
  
    tmwdlist_initialize(&pDbHandle->authUsers);

    pDbHandle->pUpdateCallback = TMWDEFS_NULL;
    pDbHandle->pUpdateCallbackParam = TMWDEFS_NULL;

    pDbHandle->nextRcvdIsCritical = TMWDEFS_FALSE;

    pDbHandle->restartTime = 0;
    memset(&pDbHandle->readTime, 0, sizeof(pDbHandle->restartTime));
    pDbHandle->readTime.invalid = TMWDEFS_TRUE; 
  }
  return(pDbHandle);
}

/* Return the head of the DB table for a given object group */
static TMWSIM_TABLE_HEAD *_getDbTable(
  MDNPSIM_DATABASE *pDbHandle,
  DNPDEFS_OBJ_GROUP_ID objectGroup)
{
  TMWSIM_TABLE_HEAD *pTableHead = TMWDEFS_NULL;

  switch (objectGroup)
  {
  case DNPDEFS_OBJ_1_BIN_INPUTS:
    pTableHead = &pDbHandle->binaryInputs;
    break;
  case DNPDEFS_OBJ_3_DBL_INPUTS:
    pTableHead = &pDbHandle->doubleInputs;
    break;
  case DNPDEFS_OBJ_10_BIN_OUTS:
    pTableHead = &pDbHandle->binaryOutputs;
    break;
  case DNPDEFS_OBJ_20_RUNNING_CNTRS:
    pTableHead = &pDbHandle->binaryCounters;
    break;
  case DNPDEFS_OBJ_21_FROZEN_CNTRS:
    pTableHead = &pDbHandle->frozenCounters;
    break;
  case DNPDEFS_OBJ_30_ANA_INPUTS:
    pTableHead = &pDbHandle->analogInputs;
    break;
  case DNPDEFS_OBJ_31_FRZN_ANA_INPUTS:
    pTableHead = &pDbHandle->frozenAnalogInputs;
    break;
  case DNPDEFS_OBJ_40_ANA_OUT_STATUSES:
    pTableHead = &pDbHandle->analogOutputs;
    break;
  case DNPDEFS_OBJ_50_TIME_AND_DATE:
    pTableHead = &pDbHandle->indexedTime;
    break;
  case DNPDEFS_OBJ_110_STRING_DATA:
    pTableHead = &pDbHandle->stringData;
    break;
  case DNPDEFS_OBJ_112_VTERM_OUTPUT:
    pTableHead = &pDbHandle->vtermEvents;
    break;
  case DNPDEFS_OBJ_114_EXT_STR_DATA:
    pTableHead = &pDbHandle->stringData; /* Shared with OBJ 110 */
    break;
  case DNPDEFS_OBJ_0_DEVICE_ATTRIBUTES:
    pTableHead = &pDbHandle->deviceAttrs;
    break;
  case DNPDEFS_OBJ_120_AUTHENTICATION:
    pTableHead = &pDbHandle->authSecStatsFromMaster;
    break;
  case DNPDEFS_OBJ_121_AUTHSECSTATS:
    pTableHead = &pDbHandle->authSecStatsFromOS;
    break;
  default:
    break;
  }
  return pTableHead;
}

 static void _clearDB(MDNPSIM_DATABASE *pDbHandle)
 {
  void *pPoint;

  _callRemoveCallback(pDbHandle, TMWSIM_CLEAR_DATABASE, 0, 0);

  while((pPoint = mdnpsim_binaryInputGetPoint(pDbHandle, 0)) != TMWDEFS_NULL)
  {
    mdnpsim_binaryInputDeletePoint(pDbHandle, pPoint);  
  }
  tmwsim_tableDestroy(&pDbHandle->binaryInputs);

  while((pPoint = mdnpsim_doubleInputGetPoint(pDbHandle, 0)) != TMWDEFS_NULL)
  {
    mdnpsim_doubleInputDeletePoint(pDbHandle, pPoint);  
  }
  tmwsim_tableDestroy(&pDbHandle->doubleInputs);

  while((pPoint = mdnpsim_binaryOutputGetPoint(pDbHandle, 0)) != TMWDEFS_NULL)
  {
    mdnpsim_binaryOutputDeletePoint(pDbHandle, pPoint);  
  }
  tmwsim_tableDestroy(&pDbHandle->binaryOutputs);

  while((pPoint = mdnpsim_binaryCounterGetPoint(pDbHandle, 0)) != TMWDEFS_NULL)
  {
    mdnpsim_binaryCounterDeletePoint(pDbHandle, pPoint);  
  }
  tmwsim_tableDestroy(&pDbHandle->binaryCounters);

  while((pPoint = mdnpsim_frozenCounterGetPoint(pDbHandle, 0)) != TMWDEFS_NULL)
  {
    mdnpsim_frozenCounterDeletePoint(pDbHandle, pPoint);  
  }
  tmwsim_tableDestroy(&pDbHandle->frozenCounters);

  while((pPoint = mdnpsim_analogInputGetPoint(pDbHandle, 0)) != TMWDEFS_NULL)
  {
    mdnpsim_analogInputDeletePoint(pDbHandle, pPoint);  
  }
  tmwsim_tableDestroy(&pDbHandle->analogInputs);

  while((pPoint = mdnpsim_frozenAnalogInputGetPoint(pDbHandle, 0)) != TMWDEFS_NULL)
  {
    mdnpsim_frozenAnalogInputDeletePoint(pDbHandle, pPoint);  
  }
  tmwsim_tableDestroy(_getDbTable(pDbHandle, DNPDEFS_OBJ_31_FRZN_ANA_INPUTS));

  while((pPoint = mdnpsim_analogOutputGetPoint(pDbHandle, 0)) != TMWDEFS_NULL)
  {
    mdnpsim_analogOutputDeletePoint(pDbHandle, pPoint);  
  }
  tmwsim_tableDestroy(&pDbHandle->analogOutputs);

  while ((pPoint = mdnpsim_indexedTimeGetPoint(pDbHandle, 0)) != TMWDEFS_NULL)
  {
    mdnpsim_indexedTimeDeletePoint(pDbHandle, pPoint);
  }
  tmwsim_tableDestroy(&pDbHandle->indexedTime); 
#if !MDNPDATA_SUPPORT_OBJ114
  while ((pPoint = mdnpsim_stringGetPoint(pDbHandle, 0)) != TMWDEFS_NULL)
  {
    mdnpsim_stringDeletePoint(pDbHandle, pPoint);
  }
  tmwsim_tableDestroy(&pDbHandle->stringData);
#else
  /* extended strings are in same list as strings */
  while ((pPoint = mdnpsim_stringOrExtGetPoint(pDbHandle, 0)) != TMWDEFS_NULL)
  {
    TMWSIM_POINT *pSimPoint = (TMWSIM_POINT *)pPoint;
    if (pSimPoint->data.string.extString == TMWDEFS_FALSE)
      mdnpsim_stringDeletePoint(pDbHandle, pPoint);
    else
      mdnpsim_extStringDeletePoint(pDbHandle, pPoint);
  }
  tmwsim_tableDestroy(&pDbHandle->stringData);
#endif
  
  while((pPoint = mdnpsim_virtualTerminalGetPoint(pDbHandle, 0)) != TMWDEFS_NULL)
  {
    mdnpsim_virtualTerminalDeletePoint(pDbHandle, pPoint);  
  }
  tmwsim_tableDestroy(&pDbHandle->vtermEvents);

#if MDNPDATA_SUPPORT_OBJ0
  while((pPoint = tmwsim_tableFindPointByIndex(&pDbHandle->deviceAttrs, 0)) != TMWDEFS_NULL)
  {
    TMWSIM_POINT *pVariation;
    TMWSIM_POINT *pAttrPoint = (TMWSIM_POINT *)pPoint;
    while((pVariation = (TMWSIM_POINT*)tmwsim_tableFindPointByIndex(&pAttrPoint->data.list.listHead, 0)) != TMWDEFS_NULL)
    {
      mdnpsim_deviceAttrDeletePoint(pDbHandle, (TMWTYPES_USHORT)tmwsim_getPointNumber(pAttrPoint), (TMWTYPES_UCHAR)tmwsim_getPointNumber(pVariation)); 
    }
    tmwsim_tableDelete(&pDbHandle->deviceAttrs, (TMWTYPES_USHORT)tmwsim_getPointNumber(pAttrPoint));
  }
  tmwsim_tableDestroy(&pDbHandle->deviceAttrs);
#endif

#if MDNPDATA_SUPPORT_OBJ120
  while((pPoint = mdnpsim_authSecStatGetPoint(pDbHandle, TMWDEFS_TRUE, 0)) != TMWDEFS_NULL)
  {
    mdnpsim_authSecStatDeletePoint(pDbHandle, TMWDEFS_TRUE, pPoint);  
  }
  tmwsim_tableDestroy(&pDbHandle->authSecStatsFromOS);

  /* Dont clear Security Statistics generated on master, they must be present if SA is enabled. */
#endif

#if MDNPDATA_SUPPORT_DATASETS
  _clearDataSets(pDbHandle);
#endif
  
  tmwdlist_destroy(&pDbHandle->authUsers, mdnpmem_free);

  pDbHandle->restartTime = 0;
  memset(&pDbHandle->readTime, 0, sizeof(pDbHandle->restartTime));
  pDbHandle->readTime.invalid = TMWDEFS_TRUE; 
}

static void _clearSecStatsDb(MDNPSIM_DATABASE *pDbHandle)
{
#if MDNPDATA_SUPPORT_OBJ120
  void *pPoint;
  while ((pPoint = mdnpsim_authSecStatGetPoint(pDbHandle, TMWDEFS_FALSE, 0)) != TMWDEFS_NULL)
  {
    mdnpsim_authSecStatDeletePoint(pDbHandle, TMWDEFS_FALSE, pPoint);
  }
  tmwsim_tableDestroy(&pDbHandle->authSecStatsFromMaster);
#else
  TMWTARG_UNUSED_PARAM(pDbHandle);
#endif
}

/* function: mdnpsim_close */
void TMWDEFS_GLOBAL mdnpsim_close(
  void *pHandle)
{
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;
  _clearDB(pDbHandle);
  _clearSecStatsDb(pDbHandle);
  mdnpmem_free(pDbHandle);
}

void TMWDEFS_GLOBAL mdnpsim_clear(
  void *pHandle)
{
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;
  _clearDB(pDbHandle);
 
  /* Recreate/initialize the empty table, so points can be added */
  tmwsim_tableCreate(&pDbHandle->binaryInputs);
  tmwsim_tableCreate(&pDbHandle->doubleInputs);
  tmwsim_tableCreate(&pDbHandle->binaryOutputs);
  tmwsim_tableCreate(&pDbHandle->binaryCounters);
  tmwsim_tableCreate(&pDbHandle->frozenCounters);
  tmwsim_tableCreate(&pDbHandle->analogInputs);
  tmwsim_tableCreate(&pDbHandle->frozenAnalogInputs);
  tmwsim_tableCreate(&pDbHandle->analogOutputs);
  tmwsim_tableCreate(&pDbHandle->stringData);
  tmwsim_tableCreate(&pDbHandle->vtermEvents);
  tmwsim_tableCreate(&pDbHandle->deviceAttrs);
  tmwsim_tableCreate(&pDbHandle->authSecStatsFromOS);
  tmwsim_tableCreate(&pDbHandle->indexedTime);
  /* Dont clear Security Statistics generated on master, they must be present if SA is enabled. */

#if MDNPDATA_SUPPORT_DATASETS
  _initDataSets(pDbHandle);
#endif
}

/* Set update callback and parameter */
void mdnpsim_setCallback(
  void *pHandle,
  MDNPSIM_CALLBACK_FUNC pUpdateCallback,
  void *pUpdateCallbackParam)
{
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;
  pDbHandle->pUpdateCallback = pUpdateCallback;
  pDbHandle->pUpdateCallbackParam = pUpdateCallbackParam;
}
 
void mdnpsim_storeReadTime(
  void *pHandle, 
  TMWDTIME *pTimeStamp)
 {
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;
  pDbHandle->readTime = *pTimeStamp;
 }

/* function: mdnpsim_getReadTime */
TMWDTIME *mdnpsim_getReadTime(
  void *pHandle)
{
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;
  return(&pDbHandle->readTime);
}

/* function: mdnpsim_getPointNumber */
TMWTYPES_USHORT TMWDEFS_GLOBAL mdnpsim_getPointNumber(
  void *pPoint)
{
  return((TMWTYPES_USHORT)tmwsim_getPointNumber((TMWSIM_POINT *)pPoint));
}

/* function: mdnpsim_binaryInputAddPoint */
TMWDEFS_GLOBAL void *mdnpsim_binaryInputAddPoint(
  void *pHandle,
  TMWTYPES_USHORT pointNumber)
{
  TMWSIM_POINT *pPoint; 
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;
  pPoint = tmwsim_tableAdd(&pDbHandle->binaryInputs, pointNumber);
  if(pPoint != TMWDEFS_NULL)
  {
    tmwsim_initBinary(pPoint, pHandle, pointNumber);
    pPoint->pSCLHandle = (void*)pDbHandle->pMDNPSession;
    _callCallback(pPoint, TMWSIM_POINT_ADD, DNPDEFS_OBJ_1_BIN_INPUTS, pointNumber);
  }
  return(pPoint);
}

/* function: mdnpsim_binaryInputDeletePoint */
TMWDEFS_GLOBAL void mdnpsim_binaryInputDeletePoint(
  void *pHandle,
  void *pPoint)
{
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;    
  TMWTYPES_USHORT pointNumber = (TMWTYPES_USHORT)tmwsim_getPointNumber((TMWSIM_POINT *)pPoint);
  _callRemoveCallback(pDbHandle, TMWSIM_POINT_DELETE, DNPDEFS_OBJ_1_BIN_INPUTS, pointNumber); 
  tmwsim_tableDelete(&pDbHandle->binaryInputs, pointNumber);
}

/* function: mdnpsim_binaryInputGetPoint */
TMWDEFS_GLOBAL void *mdnpsim_binaryInputGetPoint(
  void *pHandle,
  TMWTYPES_USHORT index)
{
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;  
  /* Index and point number are not the same for mdnp, because there may be gaps */
  return(tmwsim_tableFindPointByIndex(&pDbHandle->binaryInputs, index));
}

/* function: mdnpsim_binaryInputLookupPoint */
TMWDEFS_GLOBAL void *mdnpsim_binaryInputLookupPoint(
  void *pHandle,
  TMWTYPES_USHORT pointNumber)
{
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;
  return(tmwsim_tableFindPoint(&pDbHandle->binaryInputs, pointNumber));
}

/* function: mdnpsim_storeBinaryInput */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpsim_storeBinaryInput(
  void *pHandle,
  TMWTYPES_USHORT pointNumber,
  TMWTYPES_BOOL value,
  TMWTYPES_UCHAR flags,
  TMWDTIME *pTimeStamp)
{
  TMWSIM_POINT *pPoint;

  pPoint = (TMWSIM_POINT *)mdnpsim_binaryInputLookupPoint(pHandle, pointNumber);
  if (pPoint == TMWDEFS_NULL)
  {
    pPoint = (TMWSIM_POINT *)mdnpsim_binaryInputAddPoint(pHandle, pointNumber);
    if(pPoint == TMWDEFS_NULL)
      return(TMWDEFS_FALSE);
  }

  tmwsim_setFlags(pPoint, flags, TMWDEFS_CHANGE_REMOTE_OP);
  tmwsim_setBinaryValue(pPoint, value, TMWDEFS_CHANGE_REMOTE_OP);
  if(pTimeStamp != TMWDEFS_NULL)
    tmwsim_setTimeStamp(pPoint, pTimeStamp);
  else
    tmwdtime_getDateTime((TMWSESN*)pPoint->pSCLHandle, &pPoint->timeStamp); 

  _callCallback(pPoint, TMWSIM_POINT_UPDATE, DNPDEFS_OBJ_1_BIN_INPUTS, pointNumber);  

  return TMWDEFS_TRUE;
}

/* function: mdnpsim_binaryInputGetValue */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpsim_binaryInputGetValue(
  void *pPoint)
{ 
  return(tmwsim_getBinaryValue((TMWSIM_POINT *)pPoint)); 
}

/* function: mdnpsim_doubleInputAddPoint */
TMWDEFS_GLOBAL void *mdnpsim_doubleInputAddPoint(
  void *pHandle,
  TMWTYPES_USHORT pointNumber)
{
  TMWSIM_POINT *pPoint; 
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;
  pPoint = tmwsim_tableAdd(&pDbHandle->doubleInputs, pointNumber);
  if(pPoint != TMWDEFS_NULL)
  {
    tmwsim_initDoubleBinary(pPoint, pHandle, pointNumber);
    pPoint->pSCLHandle = (void*)pDbHandle->pMDNPSession;
    _callCallback(pPoint, TMWSIM_POINT_ADD, DNPDEFS_OBJ_3_DBL_INPUTS, pointNumber);
  }
  return(pPoint);
}

/* function: mdnpsim_doubleInputDeletePoint */
TMWDEFS_GLOBAL void mdnpsim_doubleInputDeletePoint(
  void *pHandle,
  void *pPoint)
{
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;    
  TMWTYPES_USHORT pointNumber = (TMWTYPES_USHORT)tmwsim_getPointNumber((TMWSIM_POINT *)pPoint);
  _callRemoveCallback(pDbHandle, TMWSIM_POINT_DELETE, DNPDEFS_OBJ_3_DBL_INPUTS, pointNumber); 
  tmwsim_tableDelete(&pDbHandle->doubleInputs, pointNumber);
}

/* function: mdnpsim_doubleInputGetPoint */
TMWDEFS_GLOBAL void *mdnpsim_doubleInputGetPoint(
  void *pHandle,
  TMWTYPES_USHORT index)
{
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;  
  /* Index and point number are not the same for mdnp, because there may be gaps */
  return(tmwsim_tableFindPointByIndex(&pDbHandle->doubleInputs, index));
}

/* function: mdnpsim_doubleInputLookupPoint */
TMWDEFS_GLOBAL void *mdnpsim_doubleInputLookupPoint(
  void *pHandle,
  TMWTYPES_USHORT pointNumber)
{
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;
  return(tmwsim_tableFindPoint(&pDbHandle->doubleInputs, pointNumber));
}

/* function: mdnpsim_storeDoubleInput */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpsim_storeDoubleInput(
  void *pHandle,
  TMWTYPES_USHORT pointNumber,
  TMWTYPES_UCHAR flagsAndValue,
  TMWDTIME *pTimeStamp)
{
  TMWSIM_POINT *pPoint;

  /* flags and value are stored in flags */
  pPoint = (TMWSIM_POINT *)mdnpsim_doubleInputLookupPoint(pHandle, pointNumber);
  if(pPoint == TMWDEFS_NULL)
  {
    pPoint = (TMWSIM_POINT *)mdnpsim_doubleInputAddPoint(pHandle, pointNumber);
    if(pPoint == TMWDEFS_NULL)
      return(TMWDEFS_FALSE);
  } 

  tmwsim_setFlags(pPoint, flagsAndValue, TMWDEFS_CHANGE_REMOTE_OP);
    
  if(pTimeStamp != TMWDEFS_NULL)
    tmwsim_setTimeStamp(pPoint, pTimeStamp);
  else
    tmwdtime_getDateTime((TMWSESN*)pPoint->pSCLHandle, &pPoint->timeStamp); 

  _callCallback(pPoint, TMWSIM_POINT_UPDATE, DNPDEFS_OBJ_3_DBL_INPUTS, 
    pointNumber);   

  return TMWDEFS_TRUE;

}

/* function: mdnpsim_doubleInputGetValue */
TMWTYPES_UCHAR TMWDEFS_GLOBAL mdnpsim_doubleInputGetValue(
  void *pPoint)
{
  TMWTYPES_UCHAR flags = tmwsim_getFlags((TMWSIM_POINT *)pPoint);
  return(flags >>6);
}

/* function: mdnpsim_binaryOutputAddPoint */
TMWDEFS_GLOBAL void *mdnpsim_binaryOutputAddPoint(
  void *pHandle,
  TMWTYPES_USHORT pointNumber)
{
  TMWSIM_POINT *pPoint; 
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;
  pPoint = tmwsim_tableAdd(&pDbHandle->binaryOutputs, pointNumber);
  if(pPoint != TMWDEFS_NULL)
  {
    tmwsim_initBinary(pPoint, pHandle, pointNumber);
    pPoint->pSCLHandle = (void*)pDbHandle->pMDNPSession;
    _callCallback(pPoint, TMWSIM_POINT_ADD, DNPDEFS_OBJ_10_BIN_OUTS, pointNumber);
  }
  return(pPoint);
}

/* function: mdnpsim_binaryOutputDeletePoint */
TMWDEFS_GLOBAL void mdnpsim_binaryOutputDeletePoint(
  void *pHandle,
  void *pPoint)
{
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;    
  TMWTYPES_USHORT pointNumber = (TMWTYPES_USHORT)tmwsim_getPointNumber((TMWSIM_POINT *)pPoint);
  _callRemoveCallback(pDbHandle, TMWSIM_POINT_DELETE, DNPDEFS_OBJ_10_BIN_OUTS, pointNumber); 
  tmwsim_tableDelete(&pDbHandle->binaryOutputs, pointNumber);
}

/* function: mdnpsim_binaryOutputGetPoint */
TMWDEFS_GLOBAL void *mdnpsim_binaryOutputGetPoint(
  void *pHandle,
  TMWTYPES_USHORT index)
{
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;  
  /* Index and point number are not the same for mdnp since there can be gaps */
  return(tmwsim_tableFindPointByIndex(&pDbHandle->binaryOutputs, index));
}

/* function: mdnpsim_binaryOutputLookupPoint */
TMWDEFS_GLOBAL void *mdnpsim_binaryOutputLookupPoint(
  void *pHandle,
  TMWTYPES_USHORT pointNumber)
{
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;
  return(tmwsim_tableFindPoint(&pDbHandle->binaryOutputs, pointNumber));
}

/* function: mdnpsim_storeBinaryOutput */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpsim_storeBinaryOutput(
  void *pHandle,
  TMWTYPES_USHORT pointNumber,
  TMWTYPES_BOOL value,
  TMWTYPES_UCHAR flags,
  TMWDTIME *pTimeStamp)
{
  TMWSIM_POINT *pPoint;

  pPoint = (TMWSIM_POINT *)mdnpsim_binaryOutputLookupPoint(pHandle, pointNumber);
  if(pPoint == TMWDEFS_NULL)
  {
    pPoint = (TMWSIM_POINT *)mdnpsim_binaryOutputAddPoint(pHandle, pointNumber);
    if(pPoint == TMWDEFS_NULL)
      return(TMWDEFS_FALSE);
  }
    
  tmwsim_setFlags(pPoint, flags, TMWDEFS_CHANGE_REMOTE_OP);
  tmwsim_setBinaryValue(pPoint, value, TMWDEFS_CHANGE_REMOTE_OP);

  if(pTimeStamp != TMWDEFS_NULL)
    tmwsim_setTimeStamp(pPoint, pTimeStamp);
  else
    tmwdtime_getDateTime((TMWSESN*)pPoint->pSCLHandle, &pPoint->timeStamp); 

  _callCallback(pPoint, TMWSIM_POINT_UPDATE, DNPDEFS_OBJ_10_BIN_OUT_STATUSES, 
    pointNumber);  
     
  return TMWDEFS_TRUE;
}

/* function: mdnpsim_binaryOutputGetValue */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpsim_binaryOutputGetValue(
  void *pPoint)
{
  return(tmwsim_getBinaryValue((TMWSIM_POINT *)pPoint)); 
}

/* function: mdnpsim_binaryCounterAddPoint */
TMWDEFS_GLOBAL void *mdnpsim_binaryCounterAddPoint(
  void *pHandle,
  TMWTYPES_USHORT pointNumber)
{
  TMWSIM_POINT *pPoint; 
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;
  pPoint = tmwsim_tableAdd(&pDbHandle->binaryCounters, pointNumber);
  if(pPoint != TMWDEFS_NULL)
  {
    tmwsim_initCounter(pPoint, pHandle, pointNumber);
    pPoint->pSCLHandle = (void*)pDbHandle->pMDNPSession;
    _callCallback(pPoint, TMWSIM_POINT_ADD, DNPDEFS_OBJ_20_RUNNING_CNTRS, pointNumber);
  }
  return(pPoint);
}

/* function: mdnpsim_binaryCounterDeletePoint */
TMWDEFS_GLOBAL void mdnpsim_binaryCounterDeletePoint(
  void *pHandle,
  void *pPoint)
{
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;    
  TMWTYPES_USHORT pointNumber = (TMWTYPES_USHORT)tmwsim_getPointNumber((TMWSIM_POINT *)pPoint);
  _callRemoveCallback(pDbHandle, TMWSIM_POINT_DELETE, DNPDEFS_OBJ_20_RUNNING_CNTRS, pointNumber); 
  tmwsim_tableDelete(&pDbHandle->binaryCounters, pointNumber);
}

/* function: mdnpsim_binaryCounterGetPoint */
TMWDEFS_GLOBAL void *mdnpsim_binaryCounterGetPoint(
  void *pHandle,
  TMWTYPES_USHORT index)
{
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;  
  /* Index and point number are not the same for mdnp, because there may be gaps */
  return(tmwsim_tableFindPointByIndex(&pDbHandle->binaryCounters, index));
}

/* function: mdnpsim_binaryCounterLookupPoint */
TMWDEFS_GLOBAL void *mdnpsim_binaryCounterLookupPoint(
  void *pHandle,
  TMWTYPES_USHORT pointNumber)
{
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;
  return(tmwsim_tableFindPoint(&pDbHandle->binaryCounters, pointNumber));
}

/* function: mdnpsim_storeBinaryCounter */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpsim_storeBinaryCounter(
  void *pHandle,
  TMWTYPES_USHORT pointNumber,
  TMWTYPES_ULONG value,
  TMWTYPES_UCHAR flags,
  TMWDTIME *pTimeStamp)
{
  TMWSIM_POINT *pPoint;

  pPoint = (TMWSIM_POINT *)mdnpsim_binaryCounterLookupPoint(pHandle, pointNumber);
  if(pPoint == TMWDEFS_NULL)
  {
    pPoint = (TMWSIM_POINT *)mdnpsim_binaryCounterAddPoint(pHandle, pointNumber);
    if(pPoint == TMWDEFS_NULL)
      return TMWDEFS_FALSE;
  }

  tmwsim_setFlags(pPoint, flags, TMWDEFS_CHANGE_REMOTE_OP);
  tmwsim_setCounterValue(pPoint, value, TMWDEFS_CHANGE_REMOTE_OP);
  if(pTimeStamp != TMWDEFS_NULL)
    tmwsim_setTimeStamp(pPoint, pTimeStamp);
  else
    tmwdtime_getDateTime((TMWSESN*)pPoint->pSCLHandle, &pPoint->timeStamp); 
     
  _callCallback(pPoint, TMWSIM_POINT_UPDATE, DNPDEFS_OBJ_20_RUNNING_CNTRS, 
    pointNumber);
    
  return TMWDEFS_TRUE;
}

/* function: mdnpsim_binaryCounterGetValue */
TMWTYPES_ULONG TMWDEFS_GLOBAL mdnpsim_binaryCounterGetValue(
  void *pPoint) 
{
  return(tmwsim_getCounterValue((TMWSIM_POINT *)pPoint)); 
}

/* function: mdnpsim_frozenCounterAddPoint */
TMWDEFS_GLOBAL void *mdnpsim_frozenCounterAddPoint(
  void *pHandle,
  TMWTYPES_USHORT pointNumber)
{
  TMWSIM_POINT *pPoint; 
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;
  pPoint = tmwsim_tableAdd(&pDbHandle->frozenCounters, pointNumber);
  if(pPoint != TMWDEFS_NULL)
  {
    tmwsim_initCounter(pPoint, pHandle, pointNumber);
    pPoint->pSCLHandle = (void*)pDbHandle->pMDNPSession;
    _callCallback(pPoint, TMWSIM_POINT_ADD, DNPDEFS_OBJ_21_FROZEN_CNTRS, pointNumber);
  }
  return(pPoint);
}

/* function: mdnpsim_frozenCounterDeletePoint */
TMWDEFS_GLOBAL void mdnpsim_frozenCounterDeletePoint(
  void *pHandle,
  void *pPoint)
{
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;    
  TMWTYPES_USHORT pointNumber = (TMWTYPES_USHORT)tmwsim_getPointNumber((TMWSIM_POINT *)pPoint);
  _callRemoveCallback(pDbHandle, TMWSIM_POINT_DELETE, DNPDEFS_OBJ_21_FROZEN_CNTRS, pointNumber); 
  tmwsim_tableDelete(&pDbHandle->frozenCounters, pointNumber);
}

/* function: mdnpsim_frozenCounterGetPoint */
TMWDEFS_GLOBAL void *mdnpsim_frozenCounterGetPoint(
  void *pHandle,
  TMWTYPES_USHORT index)
{
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;  
  /* Index and point number are not the same for mdnp, because there may be gaps */
  return(tmwsim_tableFindPointByIndex(&pDbHandle->frozenCounters, index));
}

/* function: mdnpsim_frozenCounterLookupPoint */
TMWDEFS_GLOBAL void *mdnpsim_frozenCounterLookupPoint(
  void *pHandle,
  TMWTYPES_USHORT pointNumber)
{
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;
  return(tmwsim_tableFindPoint(&pDbHandle->frozenCounters, pointNumber));
}

/* function: mdnpsim_storeFrozenCounter */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpsim_storeFrozenCounter(
  void *pHandle,
  TMWTYPES_USHORT pointNumber,
  TMWTYPES_ULONG value,
  TMWTYPES_UCHAR flags,
  TMWDTIME *pTimeStamp)
{
  TMWSIM_POINT *pPoint;

  pPoint = (TMWSIM_POINT *)mdnpsim_frozenCounterLookupPoint(pHandle, pointNumber);
  if(pPoint == TMWDEFS_NULL)
  {
    pPoint = (TMWSIM_POINT *)mdnpsim_frozenCounterAddPoint(pHandle, pointNumber);
    if(pPoint == TMWDEFS_NULL)
      return TMWDEFS_FALSE;
  } 

  tmwsim_setFlags(pPoint, flags, TMWDEFS_CHANGE_REMOTE_OP);
  tmwsim_setCounterValue(pPoint, value, TMWDEFS_CHANGE_REMOTE_OP);
    
  if(pTimeStamp != TMWDEFS_NULL)
    tmwsim_setTimeStamp(pPoint, pTimeStamp);
  else
    tmwdtime_getDateTime((TMWSESN*)pPoint->pSCLHandle, &pPoint->timeStamp); 
   
  _callCallback(pPoint, TMWSIM_POINT_UPDATE, DNPDEFS_OBJ_21_FROZEN_CNTRS, 
    pointNumber);
    
  return TMWDEFS_TRUE;
}

/* function: mdnpsim_frozenCounterGetValue */
TMWTYPES_ULONG TMWDEFS_GLOBAL mdnpsim_frozenCounterGetValue(
  void *pPoint) 
{
  return(tmwsim_getCounterValue((TMWSIM_POINT *)pPoint)); 
}

/* function: mdnpsim_analogInputAddPoint */
TMWDEFS_GLOBAL void *mdnpsim_analogInputAddPoint(
  void *pHandle,
  TMWTYPES_USHORT pointNumber)
{
  TMWSIM_POINT *pPoint; 
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;
  pPoint = tmwsim_tableAdd(&pDbHandle->analogInputs, pointNumber);
  if(pPoint != TMWDEFS_NULL)
  {
    tmwsim_initAnalog(pPoint, pHandle, pointNumber, TMWSIM_DATA_MIN, TMWSIM_DATA_MAX, 0, 0);
    pPoint->pSCLHandle = (void*)pDbHandle->pMDNPSession;
    _callCallback(pPoint, TMWSIM_POINT_ADD, DNPDEFS_OBJ_30_ANA_INPUTS, pointNumber);
  }
  return(pPoint);
}

/* function: mdnpsim_analogInputDeletePoint */
TMWDEFS_GLOBAL void mdnpsim_analogInputDeletePoint(
  void *pHandle,
  void *pPoint)
{
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;    
  TMWTYPES_USHORT pointNumber = (TMWTYPES_USHORT)tmwsim_getPointNumber((TMWSIM_POINT *)pPoint);
  _callRemoveCallback(pDbHandle, TMWSIM_POINT_DELETE, DNPDEFS_OBJ_30_ANA_INPUTS, pointNumber); 
  tmwsim_tableDelete(&pDbHandle->analogInputs, pointNumber);
}

/* function: mdnpsim_analogInputGetPoint */
TMWDEFS_GLOBAL void *mdnpsim_analogInputGetPoint(
  void *pHandle,
  TMWTYPES_USHORT index)
{ 
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;  
  /* Index and point number are not the same for mdnp, because there may be gaps */
  return(tmwsim_tableFindPointByIndex(&pDbHandle->analogInputs, index));
}

/* function: mdnpsim_analogInputLookupPoint */
TMWDEFS_GLOBAL void *mdnpsim_analogInputLookupPoint(
  void *pHandle,
  TMWTYPES_USHORT pointNumber)
{
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;
  return(tmwsim_tableFindPoint(&pDbHandle->analogInputs, pointNumber));
}

/* function: mdnpsim_storeAnalogInput */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpsim_storeAnalogInput(
  void *pHandle,
  TMWTYPES_USHORT pointNumber,
  TMWTYPES_ANALOG_VALUE *pValue,
  TMWTYPES_UCHAR flags,
  TMWDTIME *pTimeStamp)
{
  TMWSIM_POINT *pPoint;
  
  pPoint = (TMWSIM_POINT *)mdnpsim_analogInputLookupPoint(pHandle, pointNumber);
  if(pPoint == TMWDEFS_NULL)
  {
    pPoint = (TMWSIM_POINT *)mdnpsim_analogInputAddPoint(pHandle, pointNumber);
    if(pPoint == TMWDEFS_NULL)
      return TMWDEFS_FALSE;
  }

#if TMWCNFG_SUPPORT_DOUBLE 
  tmwsim_setFlags(pPoint, flags, TMWDEFS_CHANGE_REMOTE_OP);
  tmwsim_setAnalogValue(pPoint, dnputil_getAnalogValueDouble(pValue), TMWDEFS_CHANGE_REMOTE_OP);
  if(pTimeStamp != TMWDEFS_NULL)
    tmwsim_setTimeStamp(pPoint, pTimeStamp);
  else
    tmwdtime_getDateTime((TMWSESN*)pPoint->pSCLHandle, &pPoint->timeStamp); 

  _callCallback(pPoint, TMWSIM_POINT_UPDATE, DNPDEFS_OBJ_30_ANA_INPUTS, pointNumber);

  return TMWDEFS_TRUE;

#elif TMWCNFG_SUPPORT_FLOAT
  if(pValue->type == TMWTYPES_ANALOG_TYPE_SFLOAT)
  {    
    TMWSIM_DATA_TYPE newValue;
    TMWTYPES_UCHAR tempFlags;
    tmwsim_setFlags(pPoint, flags, TMWDEFS_CHANGE_REMOTE_OP);
    newValue = (TMWSIM_DATA_TYPE)dnputil_getAnalogValueFloat(pValue, &tempFlags);
    tmwsim_setAnalogValue(pPoint, newValue, TMWDEFS_CHANGE_REMOTE_OP);
    return TMWDEFS_TRUE;
  }
  else 
  { 
    TMWSIM_DATA_TYPE newValue;  
    TMWTYPES_UCHAR tempFlags;
    tmwsim_setFlags(pPoint, flags, TMWDEFS_CHANGE_REMOTE_OP);
    newValue = (TMWSIM_DATA_TYPE)dnputil_getAnalogValueLong(pValue, &tempFlags);
    tmwsim_setAnalogValue(pPoint, newValue, TMWDEFS_CHANGE_REMOTE_OP);
    return TMWDEFS_TRUE;
  }
#else
  {
    TMWSIM_DATA_TYPE newValue;
    TMWTYPES_UCHAR tempFlags;
    tmwsim_setFlags(pPoint, flags, TMWDEFS_CHANGE_REMOTE_OP);
    newValue = (TMWSIM_DATA_TYPE)dnputil_getAnalogValueLong(pValue, &tempFlags);
    tmwsim_setAnalogValue(pPoint, newValue, TMWDEFS_CHANGE_REMOTE_OP);
    return TMWDEFS_TRUE;
  }
#endif
}

/* function: mdnpsim_analogInputGetValue */
TMWSIM_DATA_TYPE TMWDEFS_GLOBAL mdnpsim_analogInputGetValue(
  void *pPoint) 
{
  return(tmwsim_getAnalogValue((TMWSIM_POINT*)pPoint));
}

/* function: mdnpsim_storeAnalogInputDeadband */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpsim_storeAnalogInputDeadband(
  void *pHandle,
  TMWTYPES_USHORT pointNumber,
  TMWTYPES_ANALOG_VALUE *pValue)
{
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;
  TMWSIM_POINT *pPoint = tmwsim_tableFindPoint(&pDbHandle->analogInputs, pointNumber);
  if(pPoint == TMWDEFS_NULL)
    return(TMWDEFS_FALSE);

#if TMWCNFG_SUPPORT_DOUBLE
  pPoint->data.analog.deadband = dnputil_getAnlgDBandValueDouble(pValue);
#elif TMWCNFG_SUPPORT_FLOAT
  pPoint->data.analog.deadband = dnputil_getAnlgDBandValueFloat(pValue);
#else
  pPoint->data.analog.deadband = dnputil_getAnlgDBandValueULong(pValue);
#endif
  return(TMWDEFS_TRUE);
}

/* function: mdnpsim_frozenAnalogInputAddPoint */
TMWDEFS_GLOBAL void *mdnpsim_frozenAnalogInputAddPoint(
  void *pHandle,
  TMWTYPES_USHORT pointNumber)
{
  TMWSIM_POINT *pPoint; 
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;
  pPoint = tmwsim_tableAdd(_getDbTable(pDbHandle, DNPDEFS_OBJ_31_FRZN_ANA_INPUTS), pointNumber);
  if(pPoint != TMWDEFS_NULL)
  {
    tmwsim_initAnalog(pPoint, pHandle, pointNumber, 0, 0, 0, 0);
    pPoint->pSCLHandle = (void*)pDbHandle->pMDNPSession;
    _callCallback(pPoint, TMWSIM_POINT_ADD, DNPDEFS_OBJ_31_FRZN_ANA_INPUTS, pointNumber);
  }
  return(pPoint);
}

/* function: mdnpsim_frozenAnalogInputDeletePoint */
TMWDEFS_GLOBAL void mdnpsim_frozenAnalogInputDeletePoint(
  void *pHandle,
  void *pPoint)
{
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;    
  TMWTYPES_USHORT pointNumber = (TMWTYPES_USHORT)tmwsim_getPointNumber((TMWSIM_POINT *)pPoint);
  _callRemoveCallback(pDbHandle, TMWSIM_POINT_DELETE, DNPDEFS_OBJ_31_FRZN_ANA_INPUTS, pointNumber); 
  tmwsim_tableDelete(_getDbTable(pDbHandle, DNPDEFS_OBJ_31_FRZN_ANA_INPUTS), pointNumber);
}

/* function: mdnpsim_analogInputGetPoint */
TMWDEFS_GLOBAL void *mdnpsim_frozenAnalogInputGetPoint(
  void *pHandle,
  TMWTYPES_USHORT index)
{
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;  
  /* Index and point number are not the same for mdnp, because there may be gaps */
  return(tmwsim_tableFindPointByIndex(_getDbTable(pDbHandle, DNPDEFS_OBJ_31_FRZN_ANA_INPUTS), index));
}

/* function: mdnpsim_frozenAnalogInputLookupPoint */
TMWDEFS_GLOBAL void *mdnpsim_frozenAnalogInputLookupPoint(
  void *pHandle,
  TMWTYPES_USHORT pointNumber)
{
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;
  return(tmwsim_tableFindPoint(_getDbTable(pDbHandle, DNPDEFS_OBJ_31_FRZN_ANA_INPUTS), pointNumber));
}

/* function: mdnpsim_storeFrozenAnalogInput */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpsim_storeFrozenAnalogInput(
  void *pHandle,
  TMWTYPES_USHORT pointNumber,
  TMWTYPES_ANALOG_VALUE *pValue,
  TMWTYPES_UCHAR flags,
  TMWDTIME *pTimeStamp)
{
  TMWSIM_POINT *pPoint;

  pPoint = (TMWSIM_POINT *)mdnpsim_frozenAnalogInputLookupPoint(pHandle, pointNumber);
  if(pPoint == TMWDEFS_NULL)
  {
    pPoint = (TMWSIM_POINT *)mdnpsim_frozenAnalogInputAddPoint(pHandle, pointNumber);
    if(pPoint == TMWDEFS_NULL)
      return TMWDEFS_FALSE;
  } 

#if TMWCNFG_SUPPORT_DOUBLE 
  tmwsim_setFlags(pPoint, flags, TMWDEFS_CHANGE_REMOTE_OP);
  tmwsim_setAnalogValue(pPoint, dnputil_getAnalogValueDouble(pValue), TMWDEFS_CHANGE_REMOTE_OP);
  if(pTimeStamp != TMWDEFS_NULL)
    tmwsim_setTimeStamp(pPoint, pTimeStamp);
  else
    tmwdtime_getDateTime((TMWSESN*)pPoint->pSCLHandle, &pPoint->timeStamp); 

  _callCallback(pPoint, TMWSIM_POINT_UPDATE, DNPDEFS_OBJ_31_FRZN_ANA_INPUTS, pointNumber);

  return TMWDEFS_TRUE;

#elif TMWCNFG_SUPPORT_FLOAT
  if(pValue->type == TMWTYPES_ANALOG_TYPE_SFLOAT)
  {    
    TMWSIM_DATA_TYPE newValue;
    TMWTYPES_UCHAR tempFlags;
    tmwsim_setFlags(pPoint, flags, TMWDEFS_CHANGE_REMOTE_OP);
    newValue = (TMWSIM_DATA_TYPE)dnputil_getAnalogValueFloat(pValue, &tempFlags);
    tmwsim_setAnalogValue(pPoint, newValue, TMWDEFS_CHANGE_REMOTE_OP);
    return TMWDEFS_TRUE;
  }
  else 
  { 
    TMWSIM_DATA_TYPE newValue;  
    TMWTYPES_UCHAR tempFlags;
    tmwsim_setFlags(pPoint, flags, TMWDEFS_CHANGE_REMOTE_OP);
    newValue = (TMWSIM_DATA_TYPE)dnputil_getAnalogValueLong(pValue, &tempFlags);
    tmwsim_setAnalogValue(pPoint, newValue, TMWDEFS_CHANGE_REMOTE_OP);
    return TMWDEFS_TRUE;
  }
#else
  {
    TMWSIM_DATA_TYPE newValue;
    TMWTYPES_UCHAR tempFlags;
    tmwsim_setFlags(pPoint, flags, TMWDEFS_CHANGE_REMOTE_OP);
    newValue = (TMWSIM_DATA_TYPE)dnputil_getAnalogValueLong(pValue, &tempFlags);
    tmwsim_setAnalogValue(pPoint, newValue, TMWDEFS_CHANGE_REMOTE_OP);
    return TMWDEFS_TRUE;
  }
#endif
}

/* function: mdnpsim_frozenAnalogInputGetValue */
TMWSIM_DATA_TYPE TMWDEFS_GLOBAL mdnpsim_frozenAnalogInputGetValue(
  void *pPoint) 
{
  return(tmwsim_getAnalogValue((TMWSIM_POINT*)pPoint));
}

/* function: mdnpsim_analogOutputAddPoint */
TMWDEFS_GLOBAL void *mdnpsim_analogOutputAddPoint(
  void *pHandle,
  TMWTYPES_USHORT pointNumber)
{
  TMWSIM_POINT *pPoint; 
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;
  pPoint = tmwsim_tableAdd(&pDbHandle->analogOutputs, pointNumber);
  if(pPoint != TMWDEFS_NULL)
  {
    tmwsim_initAnalog(pPoint, pHandle, pointNumber, TMWSIM_DATA_MIN, TMWSIM_DATA_MAX, 0, 0);
    pPoint->pSCLHandle = (void*)pDbHandle->pMDNPSession;
    _callCallback(pPoint, TMWSIM_POINT_ADD, DNPDEFS_OBJ_40_ANA_OUT_STATUSES, pointNumber);
  }
  return(pPoint);
}

/* function: mdnpsim_analogOutputDeletePoint */
TMWDEFS_GLOBAL void mdnpsim_analogOutputDeletePoint(
  void *pHandle,
  void *pPoint)
{
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;    
  TMWTYPES_USHORT pointNumber = (TMWTYPES_USHORT)tmwsim_getPointNumber((TMWSIM_POINT *)pPoint);
  _callRemoveCallback(pDbHandle, TMWSIM_POINT_DELETE, DNPDEFS_OBJ_40_ANA_OUT_STATUSES, pointNumber); 
  tmwsim_tableDelete(&pDbHandle->analogOutputs, pointNumber);
}

/* function: mdnpsim_analogOutputGetPoint */
TMWDEFS_GLOBAL void *mdnpsim_analogOutputGetPoint(
  void *pHandle,
  TMWTYPES_USHORT index)
{  
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;  
  /* Index and point number are not the same for mdnp, because there may be gaps */
  return(tmwsim_tableFindPointByIndex(&pDbHandle->analogOutputs, index));
}

/* function: mdnpsim_analogOutputLookupPoint */
TMWDEFS_GLOBAL void *mdnpsim_analogOutputLookupPoint(
  void *pHandle,
  TMWTYPES_USHORT pointNumber)
{
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;
  return(tmwsim_tableFindPoint(&pDbHandle->analogOutputs, pointNumber));
}

/* function: mdnpsim_storeAnalogOutput */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpsim_storeAnalogOutput(
  void *pHandle,
  TMWTYPES_USHORT pointNumber,
  TMWTYPES_ANALOG_VALUE *pValue,
  TMWTYPES_UCHAR flags, 
  TMWDTIME *pTimeStamp)
{
  TMWSIM_POINT *pPoint;
  
  pPoint = (TMWSIM_POINT *)mdnpsim_analogOutputLookupPoint(pHandle, pointNumber);
  if(pPoint == TMWDEFS_NULL)
  {
    pPoint = (TMWSIM_POINT *)mdnpsim_analogOutputAddPoint(pHandle, pointNumber);
    if(pPoint == TMWDEFS_NULL)
      return TMWDEFS_FALSE;
  }  

#if TMWCNFG_SUPPORT_DOUBLE 
  tmwsim_setFlags(pPoint, flags, TMWDEFS_CHANGE_REMOTE_OP);
  tmwsim_setAnalogValue(pPoint, dnputil_getAnalogValueDouble(pValue), TMWDEFS_CHANGE_REMOTE_OP);
    
  if(pTimeStamp != TMWDEFS_NULL)
    tmwsim_setTimeStamp(pPoint, pTimeStamp);
  else
    tmwdtime_getDateTime((TMWSESN*)pPoint->pSCLHandle, &pPoint->timeStamp); 

  _callCallback(pPoint, TMWSIM_POINT_UPDATE, DNPDEFS_OBJ_40_ANA_OUT_STATUSES, pointNumber); 
    
  return TMWDEFS_TRUE;

#elif TMWCNFG_SUPPORT_FLOAT
  if(pValue->type == TMWTYPES_ANALOG_TYPE_SFLOAT)
  {
    TMWSIM_DATA_TYPE newValue;
    TMWTYPES_UCHAR tempFlags;
    tmwsim_setFlags(pPoint, flags, TMWDEFS_CHANGE_REMOTE_OP);
    newValue = (TMWSIM_DATA_TYPE)dnputil_getAnalogValueFloat(pValue, &tempFlags);
    tmwsim_setAnalogValue(pPoint, newValue, TMWDEFS_CHANGE_REMOTE_OP);
    return TMWDEFS_TRUE;
  }
  else 
  {
    TMWSIM_DATA_TYPE newValue;
    TMWTYPES_UCHAR tempFlags;
    tmwsim_setFlags(pPoint, flags, TMWDEFS_CHANGE_REMOTE_OP);
    newValue = (TMWSIM_DATA_TYPE)dnputil_getAnalogValueLong(pValue, &tempFlags);
    tmwsim_setAnalogValue(pPoint, newValue, TMWDEFS_CHANGE_REMOTE_OP);
    return TMWDEFS_TRUE;
  }
#else
  {
    TMWSIM_DATA_TYPE newValue;
    TMWTYPES_UCHAR tempFlags;
    tmwsim_setFlags(pPoint, flags, TMWDEFS_CHANGE_REMOTE_OP);
    newValue = dnputil_getAnalogValueLong(pValue, &tempFlags);
    tmwsim_setAnalogValue(pPoint, newValue, TMWDEFS_CHANGE_REMOTE_OP);
    return TMWDEFS_TRUE;
  }
#endif
}

/* function: mdnpsim_analogOutputGetValue */
TMWSIM_DATA_TYPE TMWDEFS_GLOBAL mdnpsim_analogOutputGetValue(
  void *pPoint) 
{
  return(tmwsim_getAnalogValue((TMWSIM_POINT*)pPoint));
}

void mdnpsim_storeRestartTime(
  void *pHandle,
  TMWTYPES_ULONG time)
{
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;
  pDbHandle->restartTime = time;
}

/* function: mdnpsim_getRestartTime */
TMWTYPES_ULONG mdnpsim_getRestartTime(
  void *pHandle)
{
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;
  return(pDbHandle->restartTime);
}

/* function: mdnpsim_indexedTimeAddPoint */
TMWDEFS_GLOBAL void *mdnpsim_indexedTimeAddPoint(
  void *pHandle,
  TMWTYPES_USHORT pointNumber)
{
  TMWSIM_POINT *pPoint;
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;
  pPoint = tmwsim_tableAdd(&pDbHandle->indexedTime, pointNumber);
  if (pPoint != TMWDEFS_NULL)
  {
    tmwsim_initIndexedTime(pPoint, pHandle, pointNumber);
    pPoint->pSCLHandle = (void*)pDbHandle->pMDNPSession;
    _callCallback(pPoint, TMWSIM_POINT_ADD, DNPDEFS_OBJ_50_TIME_AND_DATE, pointNumber);
  }
  return(pPoint);
}

/* function: mdnpsim_indexedTimeDeletePoint */
TMWDEFS_GLOBAL void mdnpsim_indexedTimeDeletePoint(
  void *pHandle,
  void *pPoint)
{
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;
  TMWTYPES_USHORT pointNumber = (TMWTYPES_USHORT)tmwsim_getPointNumber((TMWSIM_POINT *)pPoint);
  _callRemoveCallback(pDbHandle, TMWSIM_POINT_DELETE, DNPDEFS_OBJ_50_TIME_AND_DATE, pointNumber);
  tmwsim_tableDelete(&pDbHandle->indexedTime, pointNumber);
}

/* function: mdnpsim_indexedTimeGetPoint */
TMWDEFS_GLOBAL void *mdnpsim_indexedTimeGetPoint(
  void *pHandle,
  TMWTYPES_USHORT index)
{
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;
  /* Index and point number are not the same for mdnp, because there may be gaps */
  return(tmwsim_tableFindPointByIndex(&pDbHandle->indexedTime, index));
}

/* function: mdnpsim_indexedTimeLookupPoint */
TMWDEFS_GLOBAL void *mdnpsim_indexedTimeLookupPoint(
  void *pHandle,
  TMWTYPES_USHORT pointNumber)
{
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;
  return(tmwsim_tableFindPoint(&pDbHandle->indexedTime, pointNumber));
}

/* function: mdnpsim_storeString */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpsim_storeIndexedTime(
  void *pHandle,
  TMWTYPES_USHORT pointNumber,
  TMWDTIME indexedTime,
  TMWTYPES_ULONG intervalCount,
  TMWTYPES_BYTE intervalUnit)
{
  TMWSIM_POINT *pPoint;

  pPoint = (TMWSIM_POINT *)mdnpsim_indexedTimeLookupPoint(pHandle, pointNumber);
  if (pPoint == TMWDEFS_NULL)
  {
    pPoint = (TMWSIM_POINT *)mdnpsim_indexedTimeAddPoint(pHandle, pointNumber);
    if (pPoint == TMWDEFS_NULL)
      return TMWDEFS_FALSE;
  }

  tmwsim_setIndexedTime(pPoint, indexedTime, intervalCount, intervalUnit);

  tmwdtime_getDateTime((TMWSESN*)pPoint->pSCLHandle, &pPoint->timeStamp);

  _callCallback(pPoint, TMWSIM_POINT_UPDATE, DNPDEFS_OBJ_50_TIME_AND_DATE, pointNumber);

  return TMWDEFS_TRUE;
}

/* function: mdnpsim_stringAddPoint */
TMWDEFS_GLOBAL void *mdnpsim_stringAddPoint(
  void *pHandle,
  TMWTYPES_USHORT pointNumber)
{
  TMWSIM_POINT *pPoint; 
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;
  pPoint = tmwsim_tableAdd(&pDbHandle->stringData, pointNumber);
  if(pPoint != TMWDEFS_NULL)
  {
    tmwsim_initString(pPoint, pHandle, pointNumber);
    pPoint->pSCLHandle = (void*)pDbHandle->pMDNPSession;
    _callCallback(pPoint, TMWSIM_POINT_ADD, DNPDEFS_OBJ_110_STRING_DATA, pointNumber);
  }
  return(pPoint);
}

/* function: mdnpsim_stringDeletePoint */
TMWDEFS_GLOBAL void mdnpsim_stringDeletePoint(
  void *pHandle,
  void *pPoint)
{
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;    
  TMWTYPES_USHORT pointNumber = (TMWTYPES_USHORT)tmwsim_getPointNumber((TMWSIM_POINT *)pPoint);
  _callRemoveCallback(pDbHandle, TMWSIM_POINT_DELETE, DNPDEFS_OBJ_110_STRING_DATA, pointNumber); 
  tmwsim_tableDelete(&pDbHandle->stringData, pointNumber);
}

/* function: mdnpsim_stringGetPoint */
TMWDEFS_GLOBAL void *mdnpsim_stringGetPoint(
  void *pHandle,
  TMWTYPES_USHORT index)
{
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;  
  /* Index and point number are not the same for mdnp, because there may be gaps */
  TMWSIM_POINT *pDataPoint = tmwsim_tableFindPointByIndex(&pDbHandle->stringData, index);

  /* Object groups 110 & 114 share a database, ensure this point is not an extended string. */
  if ((pDataPoint) && (pDataPoint->data.string.extString == TMWDEFS_TRUE))
  {
    pDataPoint = TMWDEFS_NULL;
  }
  return(pDataPoint);
}

/* function: mdnpsim_stringLookupPoint */
TMWDEFS_GLOBAL void *mdnpsim_stringLookupPoint(
  void *pHandle,
  TMWTYPES_USHORT pointNumber)
{
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;
  TMWSIM_POINT *pDataPoint = tmwsim_tableFindPoint(&pDbHandle->stringData, pointNumber);

  /* Object groups 110 & 114 share a database, ensure this point is not an extended string. */
  if ((pDataPoint) && (pDataPoint->data.string.extString == TMWDEFS_TRUE))
  {
    pDataPoint = TMWDEFS_NULL;
  }
  return(pDataPoint);
}

/* function: mdnpsim_storeString */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpsim_storeString(
  void *pHandle,
  TMWTYPES_USHORT pointNumber,
  TMWTYPES_UCHAR *pStrBuf,
  TMWTYPES_UCHAR strLen)
{
  TMWSIM_POINT *pPoint;

  pPoint = (TMWSIM_POINT *)mdnpsim_stringLookupPoint(pHandle, pointNumber);
  if(pPoint == TMWDEFS_NULL)
  {
    pPoint = (TMWSIM_POINT *)mdnpsim_stringAddPoint(pHandle, pointNumber);
    if(pPoint == TMWDEFS_NULL)
      return TMWDEFS_FALSE;
  } 

  tmwsim_setStringValue(pPoint, pStrBuf, strLen, TMWDEFS_CHANGE_REMOTE_OP);
  _callCallback(pPoint, TMWSIM_POINT_UPDATE, DNPDEFS_OBJ_110_STRING_DATA, pointNumber);
    
  return TMWDEFS_TRUE;
}

/* function: mdnpsim_virtualTerminalAddPoint */
TMWDEFS_GLOBAL void *mdnpsim_virtualTerminalAddPoint(
  void *pHandle,
  TMWTYPES_USHORT pointNumber)
{
  TMWSIM_POINT *pPoint; 
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;
  pPoint = tmwsim_tableAdd(&pDbHandle->vtermEvents, pointNumber);
  if(pPoint != TMWDEFS_NULL)
  {
    tmwsim_initString(pPoint, pHandle, pointNumber);
    pPoint->pSCLHandle = (void*)pDbHandle->pMDNPSession;
    _callCallback(pPoint, TMWSIM_POINT_ADD, DNPDEFS_OBJ_112_VTERM_OUTPUT, pointNumber);
  }
  return(pPoint);
}

/* function: mdnpsim_virtualTerminalDeletePoint */
TMWDEFS_GLOBAL void mdnpsim_virtualTerminalDeletePoint(
  void *pHandle,
  void *pPoint)
{
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;    
  TMWTYPES_USHORT pointNumber = (TMWTYPES_USHORT)tmwsim_getPointNumber((TMWSIM_POINT *)pPoint);
  _callRemoveCallback(pDbHandle, TMWSIM_POINT_DELETE, DNPDEFS_OBJ_112_VTERM_OUTPUT, pointNumber); 
  tmwsim_tableDelete(&pDbHandle->vtermEvents, pointNumber);
}

/* function: mdnpsim_virtualTerminalGetPoint */
TMWDEFS_GLOBAL void *mdnpsim_virtualTerminalGetPoint(
  void *pHandle,
  TMWTYPES_USHORT index)
{
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;  
  /* Index and point number are not the same for mdnp, because there may be gaps */
  return(tmwsim_tableFindPointByIndex(&pDbHandle->vtermEvents, index));
}

/* function: mdnpsim_virtualTerminalLookupPoint */
TMWDEFS_GLOBAL void *mdnpsim_virtualTerminalLookupPoint(
  void *pHandle,
  TMWTYPES_USHORT pointNumber)
{
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle; 
  return(tmwsim_tableFindPoint(&pDbHandle->vtermEvents, pointNumber));
}

/* function: mdnpsim_storeVirtualTerminal */
TMWDEFS_GLOBAL TMWTYPES_BOOL mdnpsim_storeVirtualTerminal(
  void *pHandle,
  TMWTYPES_USHORT pointNumber,
  TMWTYPES_UCHAR *pVtermBuf,
  TMWTYPES_UCHAR vtermLen)
{
  TMWSIM_POINT *pPoint;

  pPoint = (TMWSIM_POINT *)mdnpsim_virtualTerminalLookupPoint(pHandle, pointNumber);
  if(pPoint == TMWDEFS_NULL)
  {
    pPoint = (TMWSIM_POINT *)mdnpsim_virtualTerminalAddPoint(pHandle, pointNumber);
    if(pPoint == TMWDEFS_NULL)
      return TMWDEFS_FALSE;
  }

  tmwsim_setStringValue(pPoint, pVtermBuf, vtermLen, TMWDEFS_CHANGE_REMOTE_OP);
   
  _callCallback(pPoint, TMWSIM_POINT_UPDATE, DNPDEFS_OBJ_112_VTERM_OUTPUT, pointNumber); 

  return TMWDEFS_TRUE;
}

#if MDNPDATA_SUPPORT_OBJ114
/* function: mdnpsim_extStringAddPoint */
TMWDEFS_GLOBAL void *mdnpsim_extStringAddPoint(
  void *pHandle,
  TMWTYPES_USHORT pointNumber)
{
  TMWSIM_POINT *pPoint; 
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;
  pPoint = tmwsim_tableAdd(&pDbHandle->stringData, pointNumber);
  if(pPoint != TMWDEFS_NULL)
  {
    tmwsim_initExtString(pPoint, pHandle, pointNumber);
    pPoint->pSCLHandle = (void*)pDbHandle->pMDNPSession;
    _callCallback(pPoint, TMWSIM_POINT_ADD, DNPDEFS_OBJ_114_EXT_STR_DATA, pointNumber);
  }
  return(pPoint);
}

/* function: mdnpsim_extStringDeletePoint */
TMWDEFS_GLOBAL void mdnpsim_extStringDeletePoint(
  void *pHandle,
  void *pPoint)
{
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;    
  TMWTYPES_USHORT pointNumber = (TMWTYPES_USHORT)tmwsim_getPointNumber((TMWSIM_POINT *)pPoint);
  _callRemoveCallback(pDbHandle, TMWSIM_POINT_DELETE, DNPDEFS_OBJ_114_EXT_STR_DATA, pointNumber); 
  tmwsim_tableDelete(&pDbHandle->stringData, pointNumber);
}

/* function: mdnpsim_extStringGetPoint */
TMWDEFS_GLOBAL void *mdnpsim_extStringGetPoint(
  void *pHandle,
  TMWTYPES_USHORT index)
{
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;  
  /* Index and point number are not the same for mdnp, because there may be gaps */
  TMWSIM_POINT *pDataPoint = tmwsim_tableFindPointByIndex(&pDbHandle->stringData, index);

  /* Object groups 110 & 114 share a database, ensure this point is an extended string. */
  if ((pDataPoint) && (pDataPoint->data.string.extString == TMWDEFS_FALSE))
  {
    pDataPoint = TMWDEFS_NULL;
  }
  return(pDataPoint);
}

/* function: mdnpsim_extStringLookupPoint */
TMWDEFS_GLOBAL void *mdnpsim_extStringLookupPoint(
  void *pHandle,
  TMWTYPES_USHORT pointNumber)
{
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;
  TMWSIM_POINT *pDataPoint = tmwsim_tableFindPoint(&pDbHandle->stringData, pointNumber);

  /* Object groups 110 & 114 share a database, ensure this point is an extended string. */
  if ((pDataPoint) && (pDataPoint->data.string.extString == TMWDEFS_FALSE))
  {
    pDataPoint = TMWDEFS_NULL;
  }
  return(pDataPoint);
}

/* function: mdnpsim_storeExtString */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpsim_storeExtString(
  void *pHandle,
  TMWTYPES_USHORT pointNumber,
  TMWTYPES_UCHAR *pStrBuf,
  TMWTYPES_USHORT strLen,
  TMWTYPES_UCHAR  flags,
  TMWDTIME *pTimeStamp)
{
  TMWSIM_POINT *pPoint;

  pPoint = (TMWSIM_POINT *)mdnpsim_extStringLookupPoint(pHandle, pointNumber);
  if(pPoint == TMWDEFS_NULL)
  {
    pPoint = (TMWSIM_POINT *)mdnpsim_extStringAddPoint(pHandle, pointNumber);
    if(pPoint == TMWDEFS_NULL)
      return TMWDEFS_FALSE;
  } 

  tmwsim_setExtStringValue(pPoint, pStrBuf, strLen, TMWDEFS_CHANGE_REMOTE_OP);
  tmwsim_setFlags(pPoint, flags, TMWDEFS_CHANGE_REMOTE_OP);
  if(pTimeStamp != TMWDEFS_NULL)
    tmwsim_setTimeStamp(pPoint, pTimeStamp);
  else
    tmwdtime_getDateTime((TMWSESN*)pPoint->pSCLHandle, &pPoint->timeStamp); 

  _callCallback(pPoint, TMWSIM_POINT_UPDATE, DNPDEFS_OBJ_114_EXT_STR_DATA, pointNumber);
    
  return TMWDEFS_TRUE;
}

/* function: mdnpsim_stringOrExtGetPoint
 * Strings and Extended Strings share a list. Get either one if it exists with
 * that index.
 */
TMWDEFS_GLOBAL void *mdnpsim_stringOrExtGetPoint(
  void *pHandle,
  TMWTYPES_USHORT index)
{
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;
  /* Index and point number are not the same for mdnp, because there may be gaps */
  void *pPoint = tmwsim_tableFindPointByIndex(&pDbHandle->stringData, index);
  return(pPoint);
}
#endif

#if MDNPDATA_SUPPORT_OBJ0
TMWDEFS_GLOBAL void *mdnpsim_storeDeviceAttribute(
  void *pHandle,
  TMWTYPES_USHORT point,
  TMWTYPES_UCHAR variation, 
  DNPDATA_ATTRIBUTE_VALUE *pData)
{
  TMWSIM_POINT *pPoint;  
  TMWSIM_POINT *pAttribute = TMWDEFS_NULL;
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;

  pPoint = tmwsim_tableFindPoint(&pDbHandle->deviceAttrs, point);
  if(pPoint == TMWDEFS_NULL)
  {   
    /* Create an entry for this point index (device attribute set index)
     * which allows for a list of attributes for this point
     */
    pPoint = tmwsim_tableAdd(&pDbHandle->deviceAttrs, point);
    tmwsim_initPoint(pPoint, pHandle, point, TMWSIM_TYPE_LIST);
    pPoint->pSCLHandle = (void*)pDbHandle->pMDNPSession;
  }
  if(pPoint != TMWDEFS_NULL)
  {
    /* See if this attribute already exists */
    pAttribute = tmwsim_tableFindPoint(&pPoint->data.list.listHead, variation);

    if(pAttribute == TMWDEFS_NULL)
    {
      pAttribute = tmwsim_tableAdd(&pPoint->data.list.listHead, variation);
      if(pAttribute == TMWDEFS_NULL)
        return TMWDEFS_NULL;

      tmwsim_initPoint(pAttribute, pHandle, variation, TMWSIM_TYPE_ATTRIBUTE);
      pAttribute->pSCLHandle = (void*)pDbHandle->pMDNPSession;
      pAttribute->data.attribute.property = 0;
      pAttribute->data.attribute.point = point;
      
      _callCallback((TMWSIM_POINT *)pAttribute, TMWSIM_POINT_ADD, DNPDEFS_OBJ_0_DEVICE_ATTRIBUTES,  
        ((TMWTYPES_USHORT)(((TMWTYPES_USHORT)point << 8)|(variation & 0xff))));
    }

    if(pAttribute != TMWDEFS_NULL)
    {  
   
      if((pData->type == DNPDEFS_ATTRIBUTE_TYPE_VSTR)
        || (pData->type == DNPDEFS_ATTRIBUTE_TYPE_OSTR)
        || (pData->type == DNPDEFS_ATTRIBUTE_TYPE_BSTR))
      { 
        /* If this already existed in the database, but was not a string
         * (since variation 254 was read first, which just returned property)
         * convert it to a string point.
         */
        if(pAttribute->data.attribute.pBuf == TMWDEFS_NULL)
        {
          pAttribute->data.attribute.pBuf = (TMWTYPES_UCHAR *)tmwmem_alloc(TMWMEM_SIM_STRING_TYPE);
        }

#if (TMWSIM_STRING_MAX_LENGTH != 255)
        /* if MAX is 255 then the UCHAR length can't be greater than that and it causes a warning */
        /* Truncate string if it is too long */
        if(pData->length > TMWSIM_STRING_MAX_LENGTH)
          pData->length = TMWSIM_STRING_MAX_LENGTH;
#endif

        memcpy(pAttribute->data.attribute.pBuf, pData->value.pStrValue, pData->length);
      }
      else
      {     
        if(pAttribute->data.attribute.pBuf != TMWDEFS_NULL)
        {
          tmwmem_free(pAttribute->data.attribute.pBuf);
          pAttribute->data.attribute.pBuf = TMWDEFS_NULL;
        }
        switch(pData->type)
        {
          case DNPDEFS_ATTRIBUTE_TYPE_UINT:
            pAttribute->data.attribute.value = pData->value.uintValue;
            break;
          case DNPDEFS_ATTRIBUTE_TYPE_INT:
            pAttribute->data.attribute.value = pData->value.intValue; 
            break;
          case DNPDEFS_ATTRIBUTE_TYPE_FLT:
            if(pData->length == 4)
              pAttribute->data.attribute.value = pData->value.fltValue.sfltValue; 
            else
              pAttribute->data.attribute.value = pData->value.fltValue.doubleValue; 
            break;
          case DNPDEFS_ATTRIBUTE_TYPE_DNP3TIME:
            pAttribute->data.attribute.timeValue = pData->value.timeValue; 
            break;  
            
            /* The following are just to get rid of warnings */
          case DNPDEFS_ATTRIBUTE_TYPE_VSTR:
          case DNPDEFS_ATTRIBUTE_TYPE_OSTR:
          case DNPDEFS_ATTRIBUTE_TYPE_BSTR:
          case DNPDEFS_ATTRIBUTE_TYPE_LIST:
          case DNPDEFS_ATTRIBUTE_TYPE_EXLIST:
          default:
            return TMWDEFS_NULL;
            break;
        } 
      }
      pAttribute->data.attribute.length = pData->length;
      pAttribute->data.attribute.type = (TMWTYPES_UCHAR)pData->type;
      
     _callCallback(pPoint, TMWSIM_POINT_UPDATE, DNPDEFS_OBJ_0_DEVICE_ATTRIBUTES,
       ((TMWTYPES_USHORT)(((TMWTYPES_USHORT)point << 8)|(variation & 0xff))));
    }
  }
  return pAttribute;
} 

TMWDEFS_GLOBAL void *mdnpsim_storeDeviceAttrProperty(
  void *pHandle,
  TMWTYPES_USHORT point,
  TMWTYPES_UCHAR variation,
  TMWTYPES_UCHAR property)
{
  TMWSIM_POINT *pPoint;  
  TMWSIM_POINT *pAttribute = TMWDEFS_NULL;
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;

  pPoint = tmwsim_tableFindPoint(&pDbHandle->deviceAttrs, point);
  if(pPoint == TMWDEFS_NULL)
  {   
    /* Create an entry for this point index (device attribute set index)
     * which allows for a list of attributes for this point
     */
    pPoint = tmwsim_tableAdd(&pDbHandle->deviceAttrs, point);
    tmwsim_initPoint(pPoint, pHandle, point, TMWSIM_TYPE_LIST);
    pPoint->pSCLHandle = (void*)pDbHandle->pMDNPSession;
  }
  if(pPoint != TMWDEFS_NULL)
  {
    /* See if this attribute already exists */
    pAttribute = tmwsim_tableFindPoint(&pPoint->data.list.listHead, variation);

    if(pAttribute == TMWDEFS_NULL)
    {
      pAttribute = tmwsim_tableAdd(&pPoint->data.list.listHead, variation);
      if(pAttribute == TMWDEFS_NULL)
        return TMWDEFS_NULL;
      tmwsim_initPoint(pAttribute, pHandle, variation, TMWSIM_TYPE_ATTRIBUTE);
      pAttribute->pSCLHandle = (void*)pDbHandle->pMDNPSession;
      pAttribute->data.attribute.type = DNPDEFS_ATTRIBUTE_TYPE_UINT;
      pAttribute->data.attribute.point = point;
    }

    if(pAttribute != TMWDEFS_NULL)
    {
      /* Store property in flags field */
      pAttribute->data.attribute.property = property;
    }
  }
  return pAttribute;
}


TMWDEFS_GLOBAL void *mdnpsim_deviceAttrGetPoint(
  void            *pHandle,
  TMWTYPES_USHORT  point,
  TMWTYPES_UCHAR   variation)
{ 
  TMWSIM_POINT *pPoint;  
  TMWSIM_POINT *pAttribute = TMWDEFS_NULL;
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;

  pPoint = tmwsim_tableFindPoint(&pDbHandle->deviceAttrs, point);
  if(pPoint != TMWDEFS_NULL)
  {   
    /* See if this attribute already exists */
    pAttribute = tmwsim_tableFindPoint(&pPoint->data.list.listHead, variation);
  }
  return pAttribute;
}

TMWDEFS_GLOBAL void *mdnpsim_deviceAttrGet(
  void            *pHandle,
  TMWTYPES_USHORT  point,
  TMWTYPES_UCHAR   variation, 
  TMWTYPES_UCHAR  *pProperty,
  DNPDATA_ATTRIBUTE_VALUE *pData)
{
  TMWSIM_POINT *pPoint;  
  TMWSIM_POINT *pAttribute = TMWDEFS_NULL;
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;

  pPoint = tmwsim_tableFindPoint(&pDbHandle->deviceAttrs, point);
  if(pPoint != TMWDEFS_NULL)
  {   
    /* See if this attribute already exists */
    pAttribute = tmwsim_tableFindPoint(&pPoint->data.list.listHead, variation);
    if(pAttribute != TMWDEFS_NULL)
    {
      *pProperty = pAttribute->data.attribute.property;
      pData->length = pAttribute->data.attribute.length;
      pData->type = (DNPDEFS_ATTRIBUTE_DATA_TYPE)pAttribute->data.attribute.type;
    
      if((pData->type == DNPDEFS_ATTRIBUTE_TYPE_VSTR)
        || (pData->type == DNPDEFS_ATTRIBUTE_TYPE_OSTR)
        || (pData->type == DNPDEFS_ATTRIBUTE_TYPE_BSTR))
      { 
        pData->value.pStrValue = pAttribute->data.attribute.pBuf;
      }
      else
      {  
        switch(pData->type)
        {
          case DNPDEFS_ATTRIBUTE_TYPE_UINT:
            pData->value.uintValue = (TMWTYPES_ULONG)pAttribute->data.attribute.value;
            break;
          case DNPDEFS_ATTRIBUTE_TYPE_INT:
            pData->value.intValue = (TMWTYPES_LONG)pAttribute->data.attribute.value; 
            break;
          case DNPDEFS_ATTRIBUTE_TYPE_FLT:
            if(pData->length == 4)
              pData->value.fltValue.sfltValue = (TMWTYPES_SFLOAT)pAttribute->data.attribute.value; 
            else
              pData->value.fltValue.doubleValue = pAttribute->data.attribute.value; 
            break;
          case DNPDEFS_ATTRIBUTE_TYPE_DNP3TIME:
            pData->value.timeValue = pAttribute->data.attribute.timeValue; 
            break;

            /* The following are just to get rid of warnings */
          case DNPDEFS_ATTRIBUTE_TYPE_VSTR:
          case DNPDEFS_ATTRIBUTE_TYPE_OSTR:
          case DNPDEFS_ATTRIBUTE_TYPE_BSTR:
          case DNPDEFS_ATTRIBUTE_TYPE_LIST:
          case DNPDEFS_ATTRIBUTE_TYPE_EXLIST:
          default:
            return TMWDEFS_NULL;
            break;
        } 
      }  
    }
  }
  return(pAttribute);
}

/* function: mdnpsim_deviceAttrDeletePoint */
TMWDEFS_GLOBAL void mdnpsim_deviceAttrDeletePoint(
  void *pHandle,
  TMWTYPES_USHORT point,
  TMWTYPES_UCHAR variation)
{
  TMWSIM_POINT *pPoint;  
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;
  pPoint = tmwsim_tableFindPoint(&pDbHandle->deviceAttrs, point);
  if(pPoint != TMWDEFS_NULL)
  {   
    /* put the variation in the bottom 8 bits and the point number in the next 16 bits */
    _callRemoveCallback(pDbHandle, TMWSIM_POINT_DELETE, DNPDEFS_OBJ_0_DEVICE_ATTRIBUTES, (TMWTYPES_USHORT)((point<<8) | variation)); 
    tmwsim_tableDelete(&pPoint->data.list.listHead, variation);
  }
}
#endif
#if MDNPDATA_SUPPORT_DATASETS
/* function: _freeDatasetDescrs */
static void TMWDEFS_LOCAL _freeDatasetDescrs(
  void *pBuf)
{
#if TMWCNFG_USE_DYNAMIC_MEMORY
  MDNPSIM_DATASET_DESCR_DATA *pDescr = (MDNPSIM_DATASET_DESCR_DATA *)pBuf;
  int i;
  for(i=0; i< pDescr->numberDescrElems; i++)
  {
    if(pDescr->descrContents[i].ancillaryValue.type == DNPDATA_VALUE_STRPTR)
    {
      tmwtarg_free(pDescr->descrContents[i].ancillaryValue.value.pStrValue);
    }
  }
  for(i=0; i< pDescr->numberDataElems; i++)
  {
    if(pDescr->dataElem[i].type == DNPDATA_VALUE_STRPTR)
    {
      tmwtarg_free(pDescr->dataElem[i].value.pStrValue);
    }
  }
#endif
  mdnpmem_free(pBuf);
}

/* function: _freeDatasetProtos */
static void TMWDEFS_LOCAL _freeDatasetProtos(
  void *pBuf)
{
#if TMWCNFG_USE_DYNAMIC_MEMORY
  int i;
  MDNPSIM_DATASET_PROTO *pProto = (MDNPSIM_DATASET_PROTO *)pBuf;
  for(i=0; i< pProto->numberElems; i++)
  {
    if(pProto->contents[i].ancillaryValue.type == DNPDATA_VALUE_STRPTR)
    {
      tmwtarg_free(pProto->contents[i].ancillaryValue.value.pStrValue);
    }
  }
#endif
  mdnpmem_free(pBuf);
}

/* function: _initDataSets */
static void TMWDEFS_LOCAL _initDataSets(MDNPSIM_DATABASE *pDbHandle)
{
  tmwdlist_initialize(&pDbHandle->datasetProtos);
  tmwdlist_initialize(&pDbHandle->datasetDescrDatas);
  pDbHandle->numberSlavePrototypes = 0;
  pDbHandle->numberSlaveDescriptors = 0;
}

/* function: _clearDataSets */
static void TMWDEFS_LOCAL _clearDataSets(MDNPSIM_DATABASE *pDbHandle)
{  
  int i;
  int quantity = tmwdlist_size(&pDbHandle->datasetDescrDatas);
  for(i=quantity-1; i>=0; i--)
  {
    _callRemoveCallback(pDbHandle, TMWSIM_POINT_DELETE, DNPDEFS_OBJ_87_DATASET_VALUE, (TMWTYPES_USHORT)i); 
    _callRemoveCallback(pDbHandle, TMWSIM_POINT_DELETE, DNPDEFS_OBJ_86_DATASET_DESCR, (TMWTYPES_USHORT)i); 
  }

  quantity = tmwdlist_size(&pDbHandle->datasetProtos);
  for(i=quantity-1; i>=0; i--)
  {
    _callRemoveCallback(pDbHandle, TMWSIM_POINT_DELETE, DNPDEFS_OBJ_85_DATASET_PROTO, (TMWTYPES_USHORT)i); 
  }
  tmwdlist_destroy(&pDbHandle->datasetDescrDatas, _freeDatasetDescrs);
  tmwdlist_destroy(&pDbHandle->datasetProtos, _freeDatasetProtos);
  pDbHandle->numberSlavePrototypes = 0;
  pDbHandle->numberSlaveDescriptors = 0;
}

/* function: _isPrintable */
static TMWTYPES_BOOL TMWDEFS_LOCAL _isPrintable(
  TMWTYPES_UCHAR *pBuf,
  TMWTYPES_USHORT length)
{
  int i;
  for(i=0; i<length; i++)
  {   
    if(((*(pBuf+i))<0x20) || ((*(pBuf+i))>0x7e))
    {
      return(TMWDEFS_FALSE);
    }
  }
  return(TMWDEFS_TRUE);
}
static TMWTYPES_BOOL TMWDEFS_LOCAL _isDatasetElemPrintable(DNPDATA_DATASET_VALUE *pDataSetValue)
{
  switch(pDataSetValue->type)
  {
    case DNPDATA_VALUE_STRPTR:   
      if(_isPrintable((TMWTYPES_UCHAR *)pDataSetValue->value.pStrValue, pDataSetValue->length))
        return TMWDEFS_TRUE;
      break;
    case DNPDATA_VALUE_STRARRAY: 
      if(_isPrintable((TMWTYPES_UCHAR *)pDataSetValue->value.strValue, pDataSetValue->length))
        return TMWDEFS_TRUE;
      break;
  }
  return TMWDEFS_FALSE;
}

#endif

#if MDNPDATA_SUPPORT_OBJ85

/* See if Data Set Prototype exists in database, and if not where to insert it*/
void _protoExists(
  MDNPSIM_DATABASE *pDbHandle, 
  TMWTYPES_BOOL fromSlave, 
  TMWTYPES_USHORT pointNumber,
  MDNPSIM_DATASET_PROTO **pDescr, 
  TMWDLIST_MEMBER **pExisting)
{
  MDNPSIM_DATASET_PROTO *pPoint = TMWDEFS_NULL;

  *pDescr = TMWDEFS_NULL;
  *pExisting = TMWDEFS_NULL;

  while((pPoint = (MDNPSIM_DATASET_PROTO *)tmwdlist_getAfter(&pDbHandle->datasetProtos, 
      (TMWDLIST_MEMBER *)pPoint)) != TMWDEFS_NULL)
  {
    if(pPoint->fromSlave == fromSlave)
    {
      if(pPoint->protoId == pointNumber)
      {
        /* already exists */
        /* for now just overwrite it */ 
        *pDescr = pPoint;
        break;
      }
      else if(pPoint->protoId > pointNumber)
      {
        /* insert new descriptor before this one */
        *pExisting =(TMWDLIST_MEMBER *)pPoint;
        break;
      }
    } 
    else if(fromSlave)
    {
      /* insert new descriptor before this one */
      *pExisting = (TMWDLIST_MEMBER *)pPoint;
      break;
    }
  }
}

/* function: mdnpsim_datasetProtoQuantity */
TMWTYPES_USHORT TMWDEFS_GLOBAL mdnpsim_datasetProtoQuantity(
  void *pHandle)
{
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;
  return((TMWTYPES_USHORT)tmwdlist_size(&pDbHandle->datasetProtos));
}

/* function: mdnpsim_datasetProtoSlaveQty */
TMWTYPES_USHORT TMWDEFS_GLOBAL mdnpsim_datasetProtoSlaveQty(
  void *pHandle)
{
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;
  return(pDbHandle->numberSlavePrototypes);
}

/* function: mdnpsim_storeDatasetProtoUUID */
void TMWDEFS_GLOBAL mdnpsim_storeDatasetProtoUUID(
  void *pHandle, 
  TMWTYPES_USHORT pointNumber, 
  TMWTYPES_UCHAR *pUUID,
  TMWTYPES_BOOL  fromSlave)
{
  MDNPSIM_DATASET_PROTO *pProto;
  TMWDLIST_MEMBER *pExisting;
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;

  /* See if Data Set Prototype exists in database, and if not where to insert it*/
  _protoExists(pDbHandle, fromSlave, pointNumber, &pProto, &pExisting);

  if(pProto == TMWDEFS_NULL)
  {
    pProto = (MDNPSIM_DATASET_PROTO *)mdnpmem_alloc(MDNPMEM_SIM_DSET_PROTO_TYPE);
    if(pProto != TMWDEFS_NULL)
    {   
      int j;
      pProto->protoId = pointNumber;    
      pProto->numberElems = 0;
      
      /* This will initialize unused sim point structure to make managed code happy */
      tmwsim_initPoint((TMWSIM_POINT*)pProto, pHandle, pointNumber, TMWSIM_TYPE_BINARY);  
      ((TMWSIM_POINT*)pProto)->pSCLHandle = (void*)pDbHandle->pMDNPSession;

      for(j=0; j<MDNPSIM_MAX_DESCR_ELEM; j++)
      {
        /* Init this to uint, so we know there is no string data allocated */
        pProto->contents[j].ancillaryValue.type = DNPDATA_VALUE_UINT32;
      }

      /* Add this in order, prototypes read from the slave before the ones defined on the master */
      if(pExisting != TMWDEFS_NULL)
        tmwdlist_insertEntryBefore(&pDbHandle->datasetProtos, pExisting,(TMWDLIST_MEMBER *)pProto);
      else
        tmwdlist_addEntry(&pDbHandle->datasetProtos,(TMWDLIST_MEMBER *)pProto);

      if(fromSlave)
      {
        MDNPSIM_DATASET_PROTO *pTemp = pProto;
        pDbHandle->numberSlavePrototypes++;
        /* loop through rest of dataset prototypes and increment index for all defined by master */
        while((pTemp = (MDNPSIM_DATASET_PROTO *)tmwdlist_getAfter(&pDbHandle->datasetProtos, 
          (TMWDLIST_MEMBER *)pTemp)) != TMWDEFS_NULL)
        {
          if(!pTemp->fromSlave)
            pTemp->protoId++;
        }
      }
      
      _callCallback((TMWSIM_POINT*)pProto, TMWSIM_POINT_ADD, DNPDEFS_OBJ_85_DATASET_PROTO, pointNumber);
    }
  }

  if(pProto != TMWDEFS_NULL)
  {
    memcpy(pProto->uuid, pUUID, 16);
    pProto->fromSlave = fromSlave;
  }
}

/* function: mdnpsim_storeDatasetProto */
void TMWDEFS_GLOBAL mdnpsim_storeDatasetProto(
  void *pHandle, 
  TMWTYPES_USHORT pointNumber, 
  TMWTYPES_UCHAR  elemIndex,
  DNPDATA_DATASET_DESCR_ELEM *pElem)
{
  MDNPSIM_DATASET_PROTO *pProto;
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;

  /* Make sure array is big enough */
  if(elemIndex >= MDNPSIM_MAX_DESCR_ELEM)
    return;

  /* See if Data Set Descriptor exists in database */
  pProto = TMWDEFS_NULL;
  while((pProto = (MDNPSIM_DATASET_PROTO *)tmwdlist_getAfter(&pDbHandle->datasetProtos, 
        (TMWDLIST_MEMBER *)pProto)) != TMWDEFS_NULL)
  {
    if(pProto->protoId == pointNumber)
    {
      /* found */ 
      break;
    }
  }

  if(pProto != TMWDEFS_NULL)
  {
#if TMWCNFG_USE_DYNAMIC_MEMORY
    if(pProto->contents[elemIndex].ancillaryValue.type == DNPDATA_VALUE_STRPTR) 
    {
      tmwtarg_free(pProto->contents[elemIndex].ancillaryValue.value.pStrValue);
    }
#endif

    /* Copy descriptor element into database */
    pProto->contents[elemIndex] = *pElem; 

    /* If the value is just pointed to, it needs to be copied into simulated database memory */
    if(pElem->ancillaryValue.type == DNPDATA_VALUE_STRPTR)
    {
      int length = pElem->ancillaryValue.length;
      if(length > DNPCNFG_MAX_STRING_ARRAY)
      {
#if TMWCNFG_USE_DYNAMIC_MEMORY
        pProto->contents[elemIndex].ancillaryValue.value.pStrValue = (TMWTYPES_UCHAR*)tmwtarg_alloc(length);
        if (pProto->contents[elemIndex].ancillaryValue.value.pStrValue == TMWDEFS_NULL)
        {
          return;
        }

        memcpy(pProto->contents[elemIndex].ancillaryValue.value.pStrValue,
          pElem->ancillaryValue.value.pStrValue, length);
      }
      else
      {
#else
        /* Truncate string to fit in array */
        length = DNPCNFG_MAX_STRING_ARRAY;
#endif
        pProto->contents[elemIndex].ancillaryValue.type = DNPDATA_VALUE_STRARRAY;

        memcpy(pProto->contents[elemIndex].ancillaryValue.value.strValue,
          pElem->ancillaryValue.value.pStrValue, length);
      }
    }

    if(pProto->numberElems <= elemIndex)
    {
      pProto->numberElems = elemIndex+1;
    }
      
    _callCallback((TMWSIM_POINT*)pProto, TMWSIM_POINT_UPDATE, DNPDEFS_OBJ_85_DATASET_PROTO, pointNumber);
  }
}

/* Allow user to delete prototypes even though you must restart the device to continue
 * Renumber the following prototypes 
 */

/* function: mdnpsim_datasetProtoDeletePoint */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpsim_datasetProtoDeletePoint(
  void *pHandle,
  TMWTYPES_USHORT pointNum)
{
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;
  MDNPSIM_DATASET_PROTO *pProto = TMWDEFS_NULL;

  /* See if Data Set Prototype with this protoID exists in database */
  while((pProto = (MDNPSIM_DATASET_PROTO *)tmwdlist_getAfter(&pDbHandle->datasetProtos, 
    (TMWDLIST_MEMBER *)pProto)) != TMWDEFS_NULL)
  {
    if(pointNum == pProto->protoId)
    {
      MDNPSIM_DATASET_PROTO *pDeletedProto = pProto;

      /* renumber the following prototypes, since gaps are not allowed */
      while((pProto = (MDNPSIM_DATASET_PROTO *)tmwdlist_getAfter(&pDbHandle->datasetProtos, 
      (TMWDLIST_MEMBER *)pProto)) != TMWDEFS_NULL)
      {
        pProto->protoId--;
      }

      _callRemoveCallback(pDbHandle, TMWSIM_POINT_DELETE, DNPDEFS_OBJ_85_DATASET_PROTO, 
        (TMWTYPES_USHORT)pointNum);

      tmwdlist_removeEntry(&pDbHandle->datasetProtos, (TMWDLIST_MEMBER*)pDeletedProto);
      
      mdnpmem_free(pDeletedProto);
      return(TMWDEFS_TRUE);
    }  
  }
  return(TMWDEFS_FALSE);
}

/* function: mdnpsim_datasetProtoGetID */
TMWTYPES_BOOL TMWDEFS_CALLBACK mdnpsim_datasetProtoGetID(
  void *pHandle,
  TMWTYPES_UCHAR *pUUID,
  TMWTYPES_USHORT *pPointNum)
{
  MDNPSIM_DATASET_PROTO *pPoint;
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;

  /* See if Data Set Prototype with this UUID exists in database */
  pPoint = TMWDEFS_NULL;
  while((pPoint = (MDNPSIM_DATASET_PROTO *)tmwdlist_getAfter(&pDbHandle->datasetProtos, 
    (TMWDLIST_MEMBER *)pPoint)) != TMWDEFS_NULL)
  {
    if(!strncmp((char *)pUUID, (char *)pPoint->uuid, 16))
    {
      *pPointNum = pPoint->protoId;
      return(TMWDEFS_TRUE);
    }
  }
  return(TMWDEFS_FALSE);
}

/* function: mdnpsim_datasetProtoGet */
DNPDATA_DATASET_DESCR_ELEM * TMWDEFS_GLOBAL mdnpsim_datasetProtoGet(
  void *pHandle,
  TMWTYPES_USHORT pointNumber,
  TMWTYPES_UCHAR *pNumberElems,
  TMWTYPES_UCHAR *pUUID)
{ 
  MDNPSIM_DATASET_PROTO *pPoint;
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;

  /* find Data Set Prototype in database */
  pPoint = TMWDEFS_NULL;
  while((pPoint = (MDNPSIM_DATASET_PROTO *)tmwdlist_getAfter(&pDbHandle->datasetProtos, 
    (TMWDLIST_MEMBER *)pPoint)) != TMWDEFS_NULL)
  {
    if(pPoint->protoId == pointNumber)
    {
      memcpy(pUUID, pPoint->uuid, 16);
      *pNumberElems = pPoint->numberElems;
      return(&pPoint->contents[0]);
    }
  }
  return(TMWDEFS_NULL);
}

void * TMWDEFS_CALLBACK mdnpsim_datasetProtoGetPoint(
  void *pHandle,
  TMWTYPES_USHORT pointNumber)
{
  MDNPSIM_DATASET_PROTO *pPoint = TMWDEFS_NULL;
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;

  /* find Data Set Prototype in database */ 
  while((pPoint = (MDNPSIM_DATASET_PROTO *)tmwdlist_getAfter(&pDbHandle->datasetProtos, 
    (TMWDLIST_MEMBER *)pPoint)) != TMWDEFS_NULL)
  {
     if(pPoint->protoId == pointNumber)
    { 
      return(pPoint);
    }
  }
    return(TMWDEFS_NULL);
}
#endif

#if MDNPDATA_SUPPORT_OBJ86
/* function: mdnpsim_datasetDescrQuantity */
TMWTYPES_USHORT TMWDEFS_GLOBAL mdnpsim_datasetDescrQuantity(
  void *pHandle)
{
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;
  return((TMWTYPES_USHORT)tmwdlist_size(&pDbHandle->datasetDescrDatas));
}

/* function: mdnpsim_datasetDescrSlaveQty */
TMWTYPES_USHORT TMWDEFS_GLOBAL mdnpsim_datasetDescrSlaveQty(
  void *pHandle)
{
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;
  return(pDbHandle->numberSlaveDescriptors);
}

/* See if Data Set Descriptor exists in database, and if not where to insert it*/
static void _descrExists(
  MDNPSIM_DATABASE *pDbHandle, 
  TMWTYPES_BOOL fromSlave, 
  TMWTYPES_USHORT pointNumber,
  MDNPSIM_DATASET_DESCR_DATA **pDescr, 
  TMWDLIST_MEMBER **pExisting)
{
  MDNPSIM_DATASET_DESCR_DATA *pPoint = TMWDEFS_NULL;

  *pDescr = TMWDEFS_NULL;
  *pExisting = TMWDEFS_NULL;

  while((pPoint = (MDNPSIM_DATASET_DESCR_DATA *)tmwdlist_getAfter(&pDbHandle->datasetDescrDatas, 
      (TMWDLIST_MEMBER *)pPoint)) != TMWDEFS_NULL)
  {
    if(pPoint->fromSlave == fromSlave)
    {
      if(pPoint->datasetId == pointNumber)
      {
        /* already exists */
        /* for now just overwrite it */ 
        *pDescr = pPoint;
        break;
      }
      else if(pPoint->datasetId > pointNumber)
      {
        /* insert new descriptor before this one */
        *pExisting =(TMWDLIST_MEMBER *)pPoint;
        break;
      }
    } 
    else if(fromSlave)
    {
      /* insert new descriptor before this one */
      *pExisting = (TMWDLIST_MEMBER *)pPoint;
      break;
    }
  }
}

static void TMWDEFS_LOCAL _initDatasetValue(
  MDNPSIM_DATASET_DESCR_DATA *pDescr, 
  DNPDATA_DATASET_DESCR_ELEM *pElem, 
  TMWTYPES_UCHAR index)
{
  DNPDATA_DATASET_VALUE *pDataElem;

  /* Don't write past end of array */
  if(index >= MDNPSIM_MAX_DATASET_ELEM)
    return;
  
  pDataElem = &pDescr->dataElem[index];
  switch (pElem->dataTypeCode)
  {
    case DNPDEFS_DATASET_TYPE_VSTR:
      pDataElem->type = DNPDATA_VALUE_STRARRAY;
      pDataElem->value.strValue[0]=(TMWTYPES_UCHAR)'V';
      pDataElem->length = 1;
      break;
    case DNPDEFS_DATASET_TYPE_UINT:
      pDataElem->type = DNPDATA_VALUE_UINT32;
      pDataElem->value.uint32Value = 0;
      break;
    case DNPDEFS_DATASET_TYPE_INT:
      pDataElem->type = DNPDATA_VALUE_INT32;
      pDataElem->value.int32Value = 0;
      break;
    case DNPDEFS_DATASET_TYPE_FLT:
      pDataElem->type = DNPDATA_VALUE_SFLT;
      pDataElem->value.sfltValue = 0;
      pDataElem->length = 4;
      break;
    case DNPDEFS_DATASET_TYPE_OSTR:
      pDataElem->type = DNPDATA_VALUE_STRARRAY;
      pDataElem->value.strValue[0]=(TMWTYPES_UCHAR)0;
      pDataElem->length = 1;
      pDescr->displayValueAsHex[index] = TMWDEFS_TRUE;
      break;
    case DNPDEFS_DATASET_TYPE_BSTR:
      pDataElem->type = DNPDATA_VALUE_STRARRAY;
      pDataElem->value.strValue[0]=(TMWTYPES_UCHAR)0;
      pDataElem->length = 1;
      pDescr->displayValueAsHex[index] = TMWDEFS_TRUE;
      break;
    case DNPDEFS_DATASET_TYPE_TIME:
      pDataElem->type = DNPDATA_VALUE_TIME;
      tmwtarg_getDateTime(&pDataElem->value.timeValue);
      pDataElem->length = 6;
      break;
    default:
      return;
      break;
  }
  pDescr->numberDataElems++;
}

static void TMWDEFS_LOCAL _initDatasetValues(
  MDNPSIM_DATASET_DESCR_DATA *pDescr, 
  DNPDATA_DATASET_DESCR_ELEM *pElem, 
  TMWTYPES_UCHAR index)
{
  if(pDescr->descrContents[pDescr->numberDescrElems-1].descrElemType == DNPDEFS_DATASET_DESCR_PTYP)
  {
    char *pUUID;
    MDNPSIM_DATASET_PROTO *pProto; 
    MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pDescr->unused.pDbHandle;
    
    if(pDescr->descrContents[pDescr->numberDescrElems-1].ancillaryValue.type == DNPDATA_VALUE_STRPTR)
      pUUID = (char*)pDescr->descrContents[pDescr->numberDescrElems-1].ancillaryValue.value.pStrValue;
    else
      pUUID = (char*)pDescr->descrContents[pDescr->numberDescrElems-1].ancillaryValue.value.strValue;

    /* See if Data Set Prototype with this UUID exists in database */
    pProto = TMWDEFS_NULL;
    while((pProto = (MDNPSIM_DATASET_PROTO *)tmwdlist_getAfter(&pDbHandle->datasetProtos, 
      (TMWDLIST_MEMBER *)pProto)) != TMWDEFS_NULL)
    {
      if(!strncmp(pUUID, (char *)pProto->uuid, 16))
      { 
        break;
      }
    }
    if(pProto != TMWDEFS_NULL)
    {
      /*  create a data set element for each prototype data element */
      int i;
      for(i=0; i < pProto->numberElems; i++)
      {
        pElem = &pProto->contents[i];

        /* for each of these in the prototype, there needs to be a value element */
        if((pElem->descrElemType == DNPDEFS_DATASET_DESCR_DAEL)
          || (pElem->descrElemType == DNPDEFS_DATASET_DESCR_CTLV)
          || (pElem->descrElemType == DNPDEFS_DATASET_DESCR_CTLS))
        {
          _initDatasetValue(pDescr, pElem, index);
          index++;
        }
      }
    }
  }
  else
  {
    _initDatasetValue(pDescr, pElem, index); 
  }
}

void _createDatasetDescr(
  MDNPSIM_DATABASE *pDbHandle, 
  TMWTYPES_BOOL fromSlave, 
  TMWTYPES_USHORT pointNumber,  
  TMWDLIST_MEMBER *pExisting,
  MDNPSIM_DATASET_DESCR_DATA **pRetDescr)
{ 
  MDNPSIM_DATASET_DESCR_DATA *pDescr = (MDNPSIM_DATASET_DESCR_DATA *)mdnpmem_alloc(MDNPMEM_SIM_DSET_DESCR_TYPE);
  if(pDescr != TMWDEFS_NULL)
  {
    int j;
    pDescr->datasetId = pointNumber;
    
    /* This will initialize unused sim point structure to make managed code happy */
    tmwsim_initPoint((TMWSIM_POINT*)pDescr, (void *)pDbHandle, pointNumber, TMWSIM_TYPE_BINARY);
    ((TMWSIM_POINT*)pDescr)->pSCLHandle = (void*)pDbHandle->pMDNPSession;

    for(j=0; j<MDNPSIM_MAX_DESCR_ELEM; j++)
    {
      /* Init this to uint, so we know there is no string data allocated */
      pDescr->descrContents[j].ancillaryValue.type = DNPDATA_VALUE_UINT32;
    }  
    for(j=0; j<MDNPSIM_MAX_DATASET_ELEM; j++)
    {
      /* init value in dataset to zero */
      pDescr->dataElem[j].type = DNPDATA_VALUE_UINT32;
      pDescr->dataElem[j].length = 1;
      pDescr->dataElem[j].value.uint32Value = 0;
      pDescr->displayValueAsHex[j] = TMWDEFS_FALSE;
      pDescr->descrIndex[j].pointType = 0;
      pDescr->descrIndex[j].pointIndex = 0;
    }   

    /* Add this in order, descriptors read from the slave before the ones defined on the master */
    if(pExisting != TMWDEFS_NULL)
      tmwdlist_insertEntryBefore(&pDbHandle->datasetDescrDatas, pExisting,(TMWDLIST_MEMBER *)pDescr);
    else
      tmwdlist_addEntry(&pDbHandle->datasetDescrDatas,(TMWDLIST_MEMBER *)pDescr);

    pDescr->unused.pDbHandle = pDbHandle;
    pDescr->numberIndexElems = 0;
    pDescr->numberDescrElems = 0; 
    pDescr->numberDataElems = 0;
    pDescr->characteristics = 0;
    tmwtarg_getDateTime(&pDescr->timeStamp); 
    pDescr->fromSlave = fromSlave;
    if(fromSlave)
    {
      MDNPSIM_DATASET_DESCR_DATA *pTemp = pDescr;
      pDbHandle->numberSlaveDescriptors++;

      /* loop through rest of dataset descriptors and increment index for all defined by master */
      while((pTemp = (MDNPSIM_DATASET_DESCR_DATA *)tmwdlist_getAfter(&pDbHandle->datasetDescrDatas, 
        (TMWDLIST_MEMBER *)pTemp)) != TMWDEFS_NULL)
      {
        if(!pTemp->fromSlave)
        {
          pTemp->datasetId++;
        }
      }
    }
    _callCallback((TMWSIM_POINT*)pDescr, TMWSIM_POINT_ADD, DNPDEFS_OBJ_86_DATASET_DESCR, pointNumber);
    _callCallback((TMWSIM_POINT*)pDescr, TMWSIM_POINT_ADD, DNPDEFS_OBJ_87_DATASET_VALUE, pointNumber);
  }
  *pRetDescr = pDescr;
}

void TMWDEFS_GLOBAL mdnpsim_storeDatasetDescrCont(
  void *pHandle, 
  TMWTYPES_USHORT pointNumber, 
  TMWTYPES_UCHAR  elemIndex,
  DNPDATA_DATASET_DESCR_ELEM *pElem,
  TMWTYPES_BOOL fromSlave)
{
  MDNPSIM_DATASET_DESCR_DATA *pDescr;
  TMWDLIST_MEMBER *pExisting;
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;

  /* Make sure array is big enough */
  if(elemIndex >= MDNPSIM_MAX_DESCR_ELEM)
    return;

  /* See if Data Set Descriptor exists in database, and if not where to insert it*/
  _descrExists(pDbHandle, fromSlave, pointNumber, &pDescr, &pExisting);

  if(pDescr == TMWDEFS_NULL)
  {
    _createDatasetDescr(pDbHandle, fromSlave, pointNumber, pExisting, &pDescr);
  }

  if(pDescr != TMWDEFS_NULL)
  {

#if TMWCNFG_USE_DYNAMIC_MEMORY
    if(pDescr->descrContents[elemIndex].ancillaryValue.type == DNPDATA_VALUE_STRPTR) 
    {
      tmwtarg_free(pDescr->descrContents[elemIndex].ancillaryValue.value.pStrValue);
    }
#endif

    /* Copy descriptor element into database */
    pDescr->descrContents[elemIndex] = *pElem;

    /* If the value is just pointed to, it needs to be copied into simulated database memory */
    if(pElem->ancillaryValue.type == DNPDATA_VALUE_STRPTR)
    {
      int length = pElem->ancillaryValue.length;
      if(length > DNPCNFG_MAX_STRING_ARRAY)
      {
#if TMWCNFG_USE_DYNAMIC_MEMORY
        pDescr->descrContents[elemIndex].ancillaryValue.value.pStrValue = (TMWTYPES_UCHAR*)tmwtarg_alloc(length);
        if (pDescr->descrContents[elemIndex].ancillaryValue.value.pStrValue == TMWDEFS_NULL)
        {
          return;
        }


        memcpy(pDescr->descrContents[elemIndex].ancillaryValue.value.pStrValue,
          pElem->ancillaryValue.value.pStrValue, length);
      }
      else
      {
#else
        /* Truncate string to fit in array */
        length = DNPCNFG_MAX_STRING_ARRAY;
#endif
        pDescr->descrContents[elemIndex].ancillaryValue.type = DNPDATA_VALUE_STRARRAY;

        memcpy(pDescr->descrContents[elemIndex].ancillaryValue.value.strValue,
          pElem->ancillaryValue.value.pStrValue, length);
      }
    }

    if(pDescr->numberDescrElems <= elemIndex)
    {
      pDescr->numberDescrElems = elemIndex+1;

      /* Also add/initialize the data value element(s), based on descriptor element 
       * In case the optional name element incorrectly has type other than NONE, don't add it to data set values
       */
      if(pElem->descrElemType != DNPDEFS_DATASET_DESCR_NAME)
        _initDatasetValues(pDescr, pElem, pDescr->numberDataElems);
    }
    _callCallback((TMWSIM_POINT*)pDescr, TMWSIM_POINT_UPDATE, DNPDEFS_OBJ_86_DATASET_DESCR, pointNumber);
  }
}

/* Allow user to delete descriptor even though you must restart the device to continue 
 */

/* function: mdnpsim_datasetDescrDeletePoint */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpsim_datasetDescrDeletePoint(
  void *pHandle,
  TMWTYPES_USHORT pointNum)
{
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;
  MDNPSIM_DATASET_DESCR_DATA *pDescr = TMWDEFS_NULL;

  /* See if Data Set Descriptor with this id exists in database */
  while((pDescr = (MDNPSIM_DATASET_DESCR_DATA *)tmwdlist_getAfter(&pDbHandle->datasetDescrDatas, 
    (TMWDLIST_MEMBER *)pDescr)) != TMWDEFS_NULL)
  {
    if(pDescr->datasetId == pointNum)
    {
      MDNPSIM_DATASET_DESCR_DATA *pDeletedDescr = pDescr;

      /* renumber the following prototypes, since gaps are not allowed */
      while((pDescr = (MDNPSIM_DATASET_DESCR_DATA *)tmwdlist_getAfter(&pDbHandle->datasetDescrDatas, 
      (TMWDLIST_MEMBER *)pDescr)) != TMWDEFS_NULL)
      {
        pDescr->datasetId--;
      }

      _callRemoveCallback(pDbHandle, TMWSIM_POINT_DELETE, DNPDEFS_OBJ_86_DATASET_DESCR, 
        (TMWTYPES_USHORT)(pointNum));

      /* This is also the data set present value point */
      _callRemoveCallback(pDbHandle, TMWSIM_POINT_DELETE, DNPDEFS_OBJ_87_DATASET_VALUE, 
        (TMWTYPES_USHORT)(pointNum));

      tmwdlist_removeEntry(&pDbHandle->datasetDescrDatas, (TMWDLIST_MEMBER *)pDeletedDescr);
      
      mdnpmem_free(pDeletedDescr);
      return(TMWDEFS_TRUE);
    }  
  }

  return(TMWDEFS_FALSE);
}

void TMWDEFS_GLOBAL mdnpsim_storeDatasetDescrChars(
  void *pHandle, 
  TMWTYPES_USHORT pointNumber, 
  TMWTYPES_UCHAR  value)
{
  MDNPSIM_DATASET_DESCR_DATA *pDescr;
  TMWDLIST_MEMBER *pExisting;
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;

  TMWTYPES_BOOL fromSlave = ((value & DNPDEFS_DATASET_CHAR_DF) == 0) ? TMWDEFS_TRUE : TMWDEFS_FALSE;

  /* See if Data Set Descriptor exists in database, and if not where to insert it*/
  _descrExists(pDbHandle, fromSlave, pointNumber, &pDescr, &pExisting);

  if(pDescr == TMWDEFS_NULL)
  {
    _createDatasetDescr(pDbHandle, fromSlave, pointNumber, pExisting, &pDescr);
  }

  if(pDescr != TMWDEFS_NULL)
  {
    pDescr->characteristics = value;
    _callCallback((TMWSIM_POINT*)pDescr, TMWSIM_POINT_UPDATE, DNPDEFS_OBJ_86_DATASET_DESCR, pointNumber);
  }
}

void TMWDEFS_GLOBAL mdnpsim_storeDatasetDescrIndex(
  void *pHandle, 
  TMWTYPES_USHORT pointNumber, 
  TMWTYPES_UCHAR  elemIndex,
  DNPDATA_DATASET_DESCR_INDEX *pIndex,
  TMWTYPES_BOOL fromSlave)
{
  MDNPSIM_DATASET_DESCR_DATA *pDescr;
  TMWDLIST_MEMBER *pExisting;
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;

  /* See if Data Set Descriptor exists in database, and if not where to insert it*/
  _descrExists(pDbHandle, fromSlave, pointNumber, &pDescr, &pExisting);

  /* Make sure array is big enough */
  if(elemIndex >= MDNPSIM_MAX_DATASET_ELEM)
    return;

  /* See if Data Set Descriptor exists in database */
  pDescr = TMWDEFS_NULL;
  while((pDescr = (MDNPSIM_DATASET_DESCR_DATA *)tmwdlist_getAfter(&pDbHandle->datasetDescrDatas, 
    (TMWDLIST_MEMBER *)pDescr)) != TMWDEFS_NULL)
  {    
    if(pDescr->datasetId == pointNumber)
    {
      /* already exists */
      /* for now just overwrite it */ 
      break;
    }
  }

  if(pDescr == TMWDEFS_NULL)
  {
    _createDatasetDescr(pDbHandle, fromSlave, pointNumber, pExisting, &pDescr);
  }

  if(pDescr != TMWDEFS_NULL)
  {
    /* Copy descriptor element into database */
    pDescr->descrIndex[elemIndex] = *pIndex; 
    
    if(pDescr->numberIndexElems <= elemIndex)
    {
      pDescr->numberIndexElems = elemIndex+1;
    }
    else
    {
      _callCallback((TMWSIM_POINT*)pDescr, TMWSIM_POINT_UPDATE, DNPDEFS_OBJ_86_DATASET_DESCR, pointNumber);
    }
  }
}

DNPDATA_DATASET_DESCR_ELEM * TMWDEFS_GLOBAL mdnpsim_datasetDescrGetCont(
  void *pHandle,
  TMWTYPES_USHORT pointNumber,
  TMWTYPES_UCHAR *pNumberElems)
{ 
  MDNPSIM_DATASET_DESCR_DATA *pPoint;
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;

  /* find Data Set Descriptor in database */
  pPoint = TMWDEFS_NULL;
  while((pPoint = (MDNPSIM_DATASET_DESCR_DATA *)tmwdlist_getAfter(&pDbHandle->datasetDescrDatas, 
    (TMWDLIST_MEMBER *)pPoint)) != TMWDEFS_NULL)
  {
    if(pPoint->datasetId == pointNumber)
    {
      *pNumberElems = pPoint->numberDescrElems;
      return(&pPoint->descrContents[0]);
    }
  }
  return(TMWDEFS_NULL);
}

DNPDATA_DATASET_DESCR_ELEM *mdnpsim_datasetDescrGetExpIndex(void *pPoint, TMWTYPES_UCHAR index)
{
  int i;
  DNPDATA_DATASET_DESCR_ELEM *pDescrElem; 
  MDNPSIM_DATASET_DESCR_DATA *pDescr = (MDNPSIM_DATASET_DESCR_DATA *)pPoint;
  int expandedIndex = 0;
  for(i=1; i<pDescr->numberDescrElems; i++)
  {
    pDescrElem = &pDescr->descrContents[i];
    if( (pDescrElem->descrElemType == DNPDEFS_DATASET_DESCR_DAEL)
      ||(pDescrElem->descrElemType == DNPDEFS_DATASET_DESCR_CTLS)
      ||(pDescrElem->descrElemType == DNPDEFS_DATASET_DESCR_CTLV))
    {
      expandedIndex++;
      if(expandedIndex == index)
      { 
        return pDescrElem; 
      }
    } 
    else if(pDescrElem->descrElemType == DNPDEFS_DATASET_DESCR_PTYP)
    {
      /*  expand out prototype */
      TMWTYPES_USHORT pointNumber;
      TMWTYPES_UCHAR *pUUID; 

      if(pDescrElem->ancillaryValue.type == DNPDATA_VALUE_STRPTR)
        pUUID = pDescrElem->ancillaryValue.value.pStrValue;
      else
        pUUID = pDescrElem->ancillaryValue.value.strValue;

      if(mdnpsim_datasetProtoGetID(pDescr->unused.pDbHandle, pUUID, &pointNumber))
      {
        int j;
        MDNPSIM_DATASET_PROTO *pProto = 
          (MDNPSIM_DATASET_PROTO *)mdnpsim_datasetProtoGetPoint(pDescr->unused.pDbHandle, pointNumber);
        for(j=2; j< pProto->numberElems; j++)
        {
          pDescrElem = &pProto->contents[j];
          if((pDescrElem->descrElemType == DNPDEFS_DATASET_DESCR_DAEL)
            || (pDescrElem->descrElemType == DNPDEFS_DATASET_DESCR_CTLV)
            || (pDescrElem->descrElemType == DNPDEFS_DATASET_DESCR_CTLS))
          {
            expandedIndex++;
            if(expandedIndex == index)
            {
              return pDescrElem;
            }
          }
        }
      }     
    }
  } 

  return TMWDEFS_NULL;
}

TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpsim_datasetDescrGetChars(
  void *pHandle,
  TMWTYPES_USHORT pointNumber,
  TMWTYPES_UCHAR *pValue)
{
  MDNPSIM_DATASET_DESCR_DATA *pPoint;
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;

  /* find Data Set Descriptor in database */
  pPoint = TMWDEFS_NULL;
  while((pPoint = (MDNPSIM_DATASET_DESCR_DATA *)tmwdlist_getAfter(&pDbHandle->datasetDescrDatas, 
    (TMWDLIST_MEMBER *)pPoint)) != TMWDEFS_NULL)
  {
     if(pPoint->datasetId == pointNumber)
    {
      *pValue = pPoint->characteristics; 
      return(TMWDEFS_TRUE);
    }
  }
  return(TMWDEFS_FALSE);
}

DNPDATA_DATASET_DESCR_INDEX * TMWDEFS_GLOBAL mdnpsim_datasetDescrGetIndex(
  void *pHandle,
  TMWTYPES_USHORT pointNumber,
  TMWTYPES_UCHAR *pNumberElems)
{ 
  MDNPSIM_DATASET_DESCR_DATA *pPoint;
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;

  /* find Data Set Descriptor in database */
  pPoint = TMWDEFS_NULL;
  while((pPoint = (MDNPSIM_DATASET_DESCR_DATA *)tmwdlist_getAfter(&pDbHandle->datasetDescrDatas, 
    (TMWDLIST_MEMBER *)pPoint)) != TMWDEFS_NULL)
  {
     if(pPoint->datasetId == pointNumber)
    {
      *pNumberElems = pPoint->numberIndexElems;
      return(&pPoint->descrIndex[0]);
    }
  }
  return(TMWDEFS_NULL);
}

void * TMWDEFS_CALLBACK mdnpsim_datasetDescrGetPoint(
  void *pHandle,
  TMWTYPES_USHORT pointNumber)
{
  MDNPSIM_DATASET_DESCR_DATA *pPoint = TMWDEFS_NULL;
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;

  /* find Data Set Descriptor in database */ 
  while((pPoint = (MDNPSIM_DATASET_DESCR_DATA *)tmwdlist_getAfter(&pDbHandle->datasetDescrDatas, 
    (TMWDLIST_MEMBER *)pPoint)) != TMWDEFS_NULL)
  {
     if(pPoint->datasetId == pointNumber)
    { 
      return(pPoint);
    }
  }
  return(TMWDEFS_NULL);
}

TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpsim_datasetDescrGetName(
  void *pHandle,
  TMWTYPES_USHORT pointNumber,
  char *buf)
{ 
  MDNPSIM_DATASET_DESCR_DATA *pPoint = TMWDEFS_NULL;
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;

  /* find Data Set Descriptor in database */ 
  while((pPoint = (MDNPSIM_DATASET_DESCR_DATA *)tmwdlist_getAfter(&pDbHandle->datasetDescrDatas, 
    (TMWDLIST_MEMBER *)pPoint)) != TMWDEFS_NULL)
  {
    if(pPoint->datasetId == pointNumber)
    { 
      if(pPoint->descrContents[0].descrElemType == DNPDEFS_DATASET_DESCR_NAME)
      {
        if(pPoint->descrContents[0].ancillaryValue.type == DNPDATA_VALUE_STRPTR)
          memcpy(buf, pPoint->descrContents[0].ancillaryValue.value.pStrValue, pPoint->descrContents[0].ancillaryValue.length);
        else
          memcpy(buf, pPoint->descrContents[0].ancillaryValue.value.strValue, pPoint->descrContents[0].ancillaryValue.length); 
        buf[pPoint->descrContents[0].ancillaryValue.length] = 0;
        return TMWDEFS_TRUE;
      }
    }
  }
  return TMWDEFS_FALSE;
}
#endif

#if MDNPDATA_SUPPORT_OBJ87

void * TMWDEFS_CALLBACK mdnpsim_datasetGetPoint(
  void *pHandle,
  TMWTYPES_USHORT pointNumber) 
{
  MDNPSIM_DATASET_DESCR_DATA *pPoint = TMWDEFS_NULL;
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;

  /* find Data Set Descriptor in database */ 
  while((pPoint = (MDNPSIM_DATASET_DESCR_DATA *)tmwdlist_getAfter(&pDbHandle->datasetDescrDatas, 
    (TMWDLIST_MEMBER *)pPoint)) != TMWDEFS_NULL)
  {
     if(pPoint->datasetId == pointNumber)
    { 
      return(pPoint); 
    }
  }
    return(TMWDEFS_NULL);
}

void TMWDEFS_GLOBAL mdnpsim_storeDatasetTime(
  void *pHandle, 
  TMWTYPES_USHORT pointNumber, 
  TMWDTIME *pTimeStamp)
{
  MDNPSIM_DATASET_DESCR_DATA *pPoint;
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;

  /* See if Data Set exists in database */
  pPoint = TMWDEFS_NULL;
  while((pPoint = (MDNPSIM_DATASET_DESCR_DATA *)tmwdlist_getAfter(&pDbHandle->datasetDescrDatas, 
    (TMWDLIST_MEMBER *)pPoint)) != TMWDEFS_NULL)
  {
    if(pPoint->datasetId == pointNumber)
    {
      pPoint->timeStamp = *pTimeStamp;
     _callCallback((TMWSIM_POINT*)pPoint, TMWSIM_POINT_UPDATE, DNPDEFS_OBJ_87_DATASET_VALUE, pointNumber);
      break;
    }
  }
}

void TMWDEFS_GLOBAL mdnpsim_storeDataset(
  void *pHandle, 
  TMWTYPES_USHORT pointNumber, 
  TMWTYPES_UCHAR  elemIndex,
  DNPDATA_DATASET_VALUE *pElem)
{
  MDNPSIM_DATASET_DESCR_DATA *pPoint;
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;

  /* Make sure array is big enough */
  if(elemIndex >= MDNPSIM_MAX_DATASET_ELEM)
    return;

  /* See if Data Set Descriptor exists in database */
  pPoint = TMWDEFS_NULL;
  while((pPoint = (MDNPSIM_DATASET_DESCR_DATA *)tmwdlist_getAfter(&pDbHandle->datasetDescrDatas, 
    (TMWDLIST_MEMBER *)pPoint)) != TMWDEFS_NULL)
  {
    if(pPoint->datasetId == pointNumber)
    {
      /* found */ 
      break;
    }
  }

  if(pPoint != TMWDEFS_NULL)
  {

#if TMWCNFG_USE_DYNAMIC_MEMORY
    if(pPoint->dataElem[elemIndex].type == DNPDATA_VALUE_STRPTR) 
    {
      tmwtarg_free(pPoint->dataElem[elemIndex].value.pStrValue);
    }
#endif

    /* Copy descriptor element into database */
    pPoint->dataElem[elemIndex] = *pElem; 
    
    /* If the value is just pointed to, it needs to be copied into simulated database memory */
    if(pElem->type == DNPDATA_VALUE_STRPTR)
    {
      int length = pElem->length;
      if(length > DNPCNFG_MAX_STRING_ARRAY)
      {
#if TMWCNFG_USE_DYNAMIC_MEMORY
        pPoint->dataElem[elemIndex].value.pStrValue = (TMWTYPES_UCHAR*)tmwtarg_alloc(length);
        if (pPoint->dataElem[elemIndex].value.pStrValue == TMWDEFS_NULL)
        {
          return;
        }

        memcpy(pPoint->dataElem[elemIndex].value.pStrValue,
          pElem->value.pStrValue, length);

        if(pPoint->numberDataElems <= elemIndex)
        {
          pPoint->numberDataElems = elemIndex+1;
        } 
      }
      else
      {
#else
        /* Truncate string to fit in array */
        length = DNPCNFG_MAX_STRING_ARRAY;
#endif
        pPoint->dataElem[elemIndex].type = DNPDATA_VALUE_STRARRAY;

        memcpy(pPoint->dataElem[elemIndex].value.strValue,
        pElem->value.pStrValue, length);
      }
    }

    if(pPoint->numberDataElems <= elemIndex)
    {
      pPoint->numberDataElems = elemIndex+1;
    }
     _callCallback((TMWSIM_POINT*)pPoint, TMWSIM_POINT_UPDATE, DNPDEFS_OBJ_87_DATASET_VALUE, pointNumber);
  }
}

DNPDATA_DATASET_VALUE * TMWDEFS_GLOBAL mdnpsim_datasetGet(
  void *pHandle,
  TMWTYPES_USHORT pointNumber,
  TMWTYPES_UCHAR *pNumberElems,
  TMWDTIME *pTimeStamp)
{ 
  MDNPSIM_DATASET_DESCR_DATA *pPoint; 
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;

  /* find Data Set Descriptor in database */
  pPoint = TMWDEFS_NULL;
  while((pPoint = (MDNPSIM_DATASET_DESCR_DATA *)tmwdlist_getAfter(&pDbHandle->datasetDescrDatas, 
    (TMWDLIST_MEMBER *)pPoint)) != TMWDEFS_NULL)
  {
    if(pPoint->datasetId == pointNumber)
    { 
      *pTimeStamp = pPoint->timeStamp;
      *pNumberElems = pPoint->numberDataElems;
      return(&pPoint->dataElem[0]);
    }
  }
  return(TMWDEFS_NULL);
}
#endif

#if MDNPDATA_SUPPORT_OBJ120
#if MDNPCNFG_SUPPORT_SA_VERSION2
/* function: mdnpsim_authGetNewSessionKeys */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpsim_authGetNewSessionKeys(
  void             *pHandle, 
  DNPDATA_AUTH_KEY *pControlSessionKey,
  DNPDATA_AUTH_KEY *pMonitorSessionKey)
{ 
 
#if TMWCNFG_SUPPORT_CRYPTO 
  {
    /*  If TMWCRYPTO interface is supported (it is required for SAv5) use it for SAv2. */
    MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;
    TMWCRYPTO_KEY cryptoKey;
    if(tmwcrypto_generateNewKey(pDbHandle->pMDNPSession->dnp.pCryptoHandle, TMWCRYPTO_USER_CONTROL_SESSION_KEY, 16, &cryptoKey))
    {
      memcpy(pControlSessionKey->value, cryptoKey.value, 16);
      pControlSessionKey->length = 16;
    }

    if(tmwcrypto_generateNewKey(pDbHandle->pMDNPSession->dnp.pCryptoHandle, TMWCRYPTO_USER_CONTROL_SESSION_KEY, 16, &cryptoKey))
    {
      memcpy(pMonitorSessionKey->value, cryptoKey.value, 16);
      pMonitorSessionKey->length = 16;
    }
    return TMWDEFS_TRUE;
  }
 #else
  TMWTARG_UNUSED_PARAM(pHandle);
  memcpy(pControlSessionKey->value, "user1 controlkey", 16);
  pControlSessionKey->length = 16; 
  memcpy(pMonitorSessionKey->value, "user1 monitorkey", 16);
  pMonitorSessionKey->length = 16;
  return(TMWDEFS_TRUE);
#endif
}

/* function: mdnpsim_authKeyWrapSupport */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpsim_authKeyWrapSupport(
  TMWTYPES_UCHAR keyWrapAlgorithm)
{
  if(keyWrapAlgorithm == DNPAUTH_KEYWRAP_AES128) 
    return TMWDEFS_TRUE;
  else
    return TMWDEFS_FALSE; 
}

#if TMWCNFG_SUPPORT_CRYPTO
/* These are the default user keys the test harness uses for test purposes */
static TMWTYPES_UCHAR defaultUserKey1[] = {
  0x49, 0xC8, 0x7D, 0x5D, 0x90, 0x21, 0x7A, 0xAF, 
  0xEC, 0x80, 0x74, 0xeb, 0x71, 0x52, 0xfd, 0xb5
};
static TMWTYPES_UCHAR defaultUserKeyOther[] = {
  0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 
  0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff
}; 
#endif

/* function: mdnpsim_authEncryptKeyWrapData */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpsim_authEncryptKeyWrapData(
  void            *pHandle, 
  TMWTYPES_USHORT  userNumber,
  TMWTYPES_UCHAR   keyWrapAlgorithm,
  TMWTYPES_UCHAR  *pPlainData, 
  TMWTYPES_USHORT  plainDataLength, 
  TMWTYPES_UCHAR  *pEncryptedData,
  TMWTYPES_USHORT *pEncryptedLength)
{
  TMWTARG_UNUSED_PARAM(userNumber); 
  TMWTARG_UNUSED_PARAM(keyWrapAlgorithm); 
   
#if TMWCNFG_SUPPORT_CRYPTO 
  {
    /*  If TMWCRYPTO interface is supported (it is required for SAv5) use it for SAv2. */
    MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;
    TMWCRYPTO_KEY cryptoKey;
    cryptoKey.ivLength =0;
    cryptoKey.length = 16;
    /* These are the default key values used by the test harness */
    if(userNumber == 1) 
      memcpy(cryptoKey.value, defaultUserKey1, 16);
    else 
      memcpy(cryptoKey.value, defaultUserKeyOther, 16); 

    return tmwcrypto_encryptData(pDbHandle->pMDNPSession->dnp.pCryptoHandle, TMWCRYPTO_ALG_KEYWRAP_AES128,
       &cryptoKey, pPlainData, plainDataLength, pEncryptedData, pEncryptedLength);
  }
#else
  TMWTARG_UNUSED_PARAM(pHandle);
  /* Just return data, don't encrypt it for simulation purposes */
  memcpy(pEncryptedData, pPlainData, plainDataLength);
  *pEncryptedLength = plainDataLength;
  return TMWDEFS_TRUE; 
#endif
}

/* function: mdnpsim_authHMACSupport */
TMWTYPES_CHAR mdnpsim_authHMACSupport(
  TMWTYPES_UCHAR HMACAlgorithm)
{
  if(HMACAlgorithm == DNPAUTH_HMAC_SHA1_4OCTET)
  {
    return(4);
  }
  else if(HMACAlgorithm == DNPAUTH_MAC_SHA256_8OCTET) 
  {
    return(8);
  }
  else if(HMACAlgorithm == DNPAUTH_MAC_SHA1_8OCTET)
  {
    return(8);
  }
  else if(HMACAlgorithm == DNPAUTH_MAC_SHA1_10OCTET)
  {
    return(10);
  }
  else if(HMACAlgorithm == DNPAUTH_MAC_SHA256_16OCTET) 
  {
    return(16);
  }
  return(0);
}

/* function: mdnpsim_authMACValue */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpsim_authHMACValue(
  TMWTYPES_UCHAR    algorithm,
  DNPDATA_AUTH_KEY *pKey,
  TMWTYPES_UCHAR   *pData,
  TMWTYPES_ULONG   dataLength,
  TMWTYPES_UCHAR   *pMACValue,
  TMWTYPES_USHORT  *pMACValueLength)
{ 
  TMWTYPES_USHORT length = 0;
  TMWTARG_UNUSED_PARAM(pKey); 
  TMWTARG_UNUSED_PARAM(dataLength); 
  /* Just copy first x bytes of data for simulation purposes */
  if(algorithm == DNPAUTH_HMAC_SHA1_4OCTET)
  {
    length = 4;
  }
  else if(algorithm == DNPAUTH_MAC_SHA256_8OCTET)
  {
    length = 8;
  }
  else if(algorithm == DNPAUTH_MAC_SHA1_8OCTET)
  {
    length = 10;
  } 
  else if(algorithm == DNPAUTH_MAC_SHA1_10OCTET)
  {
    length = 10;
  } 
  else if(algorithm == DNPAUTH_MAC_SHA256_16OCTET)
  {
    length = 16;
  } 

  if(length > 0)
  {
#if TMWCNFG_SUPPORT_CRYPTO 
    /*  If TMWCRYPTO interface is supported (it is required for SAv5) use it for SAv2. */
    TMWTYPES_UCHAR crypto_algorithm;
    TMWCRYPTO_KEY cryptoKey; 

    cryptoKey.ivLength = 0;
    memcpy(cryptoKey.value, pKey->value, pKey->length);
    cryptoKey.length = pKey->length;

    crypto_algorithm = TMWCRYPTO_ALG_MAC_SHA1;
    if(algorithm == DNPAUTH_HMAC_SHA256_8OCTET 
      || algorithm == DNPAUTH_HMAC_SHA256_16OCTET)
      crypto_algorithm = TMWCRYPTO_ALG_MAC_SHA256;
 
    return tmwcrypto_MACValue(TMWDEFS_NULL, crypto_algorithm,
     &cryptoKey, length, pData, dataLength, pMACValue, pMACValueLength);
#else
    memcpy(pMACValue, pData, length);
    *pMACValueLength = length;
    return TMWDEFS_TRUE;
#endif
  }

  return TMWDEFS_FALSE;
}

/* function: mdnpsim_authRandomChallengeData */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpsim_authRandomChallengeData(
  TMWTYPES_UCHAR  *pBuf,
  TMWTYPES_USHORT  minLength,
  TMWTYPES_USHORT *pLength)
{
#if TMWCNFG_SUPPORT_CRYPTO 
  /*  If TMWCRYPTO interface is supported (it is required for SAv5) use it for SAv2. */
  return tmwcrypto_getRandomData(TMWDEFS_NULL, minLength, pBuf, pLength); 
#else
  /* return minLength bytes of random data */ 
  memcpy(pBuf, "123456789012345678901234567890123456789012345678901234567890", minLength); 
  *pLength = minLength;
  return(TMWDEFS_TRUE);
#endif
}
#endif

TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpsim_authGetOSName(
  void            *pHandle, 
  TMWTYPES_UCHAR  *pOSName,
  TMWTYPES_USHORT *pOSNameLength)
{ 
  TMWTARG_UNUSED_PARAM(pHandle);
  STRCPY((TMWTYPES_CHAR*)pOSName, *pOSNameLength, "SDNP Outstation");
  *pOSNameLength = (TMWTYPES_USHORT)strlen((TMWTYPES_CHAR*)pOSName);
  return(TMWDEFS_TRUE);
}
 
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpsim_authGetUserName(
  void            *pHandle, 
  void            *userDbHandle,
  TMWTYPES_UCHAR  *pUserName,
  TMWTYPES_USHORT *pUserNameLength)
{
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;
  MDNPSIM_AUTHUSER *pUser = TMWDEFS_NULL; 

  while((pUser = (MDNPSIM_AUTHUSER *)tmwdlist_getAfter(
      &pDbHandle->authUsers, (TMWDLIST_MEMBER *)pUser)) != TMWDEFS_NULL)
  {
    if(userDbHandle == pUser)
    { 
      memcpy(pUserName, pUser->userName, pUser->userNameLength);
      *pUserNameLength = pUser->userNameLength; 
      return TMWDEFS_TRUE;
    }
  }
  return TMWDEFS_FALSE;
}

TMWTYPES_USHORT TMWDEFS_GLOBAL mdnpsim_authGetUserNumber(
  void            *pHandle, 
  void            *userDbHandle)
{
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;
  MDNPSIM_AUTHUSER *pUser = TMWDEFS_NULL; 

  while((pUser = (MDNPSIM_AUTHUSER *)tmwdlist_getAfter(
      &pDbHandle->authUsers, (TMWDLIST_MEMBER *)pUser)) != TMWDEFS_NULL)
  {
    if(userDbHandle == pUser)
    {  
      return(pUser->userNumber);
    }
  }
  return 0;
}

void * TMWDEFS_GLOBAL mdnpsim_authConfigUser( 
  void             *pHandle,
  TMWTYPES_CHAR    *pUserName,
  TMWTYPES_USHORT   userNameLength,
  TMWTYPES_ULONG    statusChangeSequenceNumber,
  TMWTYPES_UCHAR    keyChangeMethod, 
  TMWTYPES_UCHAR    operation, 
  TMWTYPES_USHORT   userRole, 
  TMWTYPES_USHORT   userRoleExpiryInterval,
  TMWTYPES_UCHAR   *pUpdateKey,
  TMWTYPES_USHORT   updateKeyLength,
  TMWTYPES_UCHAR   *pCertData,
  TMWTYPES_USHORT   certDataLength)
{  
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;
  MDNPSIM_AUTHUSER *pUser = TMWDEFS_NULL;
  TMWTYPES_BOOL alreadyOnList = TMWDEFS_FALSE;

  while((pUser = (MDNPSIM_AUTHUSER *)tmwdlist_getAfter(
      &pDbHandle->authUsers, (TMWDLIST_MEMBER *)pUser)) != TMWDEFS_NULL)
  {
    if(memcmp(pUser->userName, pUserName, userNameLength)==0)
    {
      /* found */
      alreadyOnList = TMWDEFS_TRUE;
      break;
    }
  } 

  if(pUser == TMWDEFS_NULL)
    pUser = (MDNPSIM_AUTHUSER *)mdnpmem_alloc(MDNPMEM_SIM_AUTH_USER_TYPE);

  if(pUser == TMWDEFS_NULL)
  {
    return(TMWDEFS_NULL);
  }
   
  pUser->userNumber = 0;
  pUser->userRole = userRole;
  pUser->userRoleExpiryInterval = userRoleExpiryInterval;
  pUser->keyChangeMethod = keyChangeMethod;
  pUser->statusChangeSequenceNumber = statusChangeSequenceNumber;

  memcpy(pUser->userName, pUserName, userNameLength);
  pUser->userNameLength = userNameLength;

  pUser->operation = operation; 
  
  /* Update Key may have been provided by authority */
  memcpy(pUser->updateKey, pUpdateKey, updateKeyLength);
  pUser->updateKeyLength = updateKeyLength; 
  
  /* Certification data should have been provided by authority */
  memcpy(pUser->certData, pCertData, certDataLength);
  pUser->certDataLength = certDataLength; 

  if(!alreadyOnList)
    tmwdlist_addEntry(&pDbHandle->authUsers, (TMWDLIST_MEMBER *)pUser);

  return pUser;
}

TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpsim_authGetChangeUserData(
  void            *pHandle, 
  void            *userDbHandle,
  TMWTYPES_ULONG  *pStatusChangeSequenceNumber,
  TMWTYPES_UCHAR  *pKeyChangeMethod,
  TMWTYPES_USHORT *pUserRole,
  TMWTYPES_USHORT *pUserRoleExpiryInterval)
{
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;
  MDNPSIM_AUTHUSER *pUser = TMWDEFS_NULL;

  while((pUser = (MDNPSIM_AUTHUSER *)tmwdlist_getAfter(
      &pDbHandle->authUsers, (TMWDLIST_MEMBER *)pUser)) != TMWDEFS_NULL)
  {
    if(userDbHandle == pUser)
    { 
      *pStatusChangeSequenceNumber = pUser->statusChangeSequenceNumber;
      *pKeyChangeMethod = pUser->keyChangeMethod;
      *pUserRole = pUser->userRole; 
      *pUserRoleExpiryInterval = pUser->userRoleExpiryInterval;
      return TMWDEFS_TRUE;
    }
  }
  return TMWDEFS_FALSE;
}
 
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpsim_authGetCertData(
  void            *pHandle, 
  void            *userDbHandle,
  TMWTYPES_UCHAR  *pCertData,
  TMWTYPES_USHORT *pCertDataLength)
{ 
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;
  MDNPSIM_AUTHUSER *pUser = TMWDEFS_NULL;
  while((pUser = (MDNPSIM_AUTHUSER *)tmwdlist_getAfter(
      &pDbHandle->authUsers, (TMWDLIST_MEMBER *)pUser)) != TMWDEFS_NULL)
  {
    if(userDbHandle == pUser)
    { 
      memcpy((char *)pCertData, pUser->certData, pUser->certDataLength);
     *pCertDataLength = pUser->certDataLength; 
      return TMWDEFS_TRUE;
    }
  }
  return TMWDEFS_FALSE;
}

TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpsim_authGetUpdateKeyData(
  void            *pHandle,
  void            *userNameDbHandle,
  TMWTYPES_UCHAR  *pOSData,
  TMWTYPES_USHORT *pOSDataLength)
{
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(userNameDbHandle);
  TMWTARG_UNUSED_PARAM(pOSData);
  TMWTARG_UNUSED_PARAM(pOSDataLength);
  return TMWDEFS_FALSE;
}

void TMWDEFS_GLOBAL mdnpsim_authStoreUpdKeyChangeReply(
  void            *pHandle, 
  void            *userDbHandle,
  TMWTYPES_USHORT  userNumber)
{ 
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;
  MDNPSIM_AUTHUSER *pUser = TMWDEFS_NULL;

  while((pUser = (MDNPSIM_AUTHUSER *)tmwdlist_getAfter(
      &pDbHandle->authUsers, (TMWDLIST_MEMBER *)pUser)) != TMWDEFS_NULL)
  {
    if(pUser == userDbHandle)
    {
      pUser->userNumber = userNumber;
 
#if TMWCNFG_SUPPORT_CRYPTO 
      {
      void* longUserNumber = (void*)userNumber;

      /* Put this user and update key in tmwcrypto simulated database so encryption can find it */ 
      tmwcrypto_setKeyData(pDbHandle->pMDNPSession->dnp.pCryptoHandle, TMWCRYPTO_USER_UPDATE_KEY, longUserNumber, pUser->updateKey, pUser->updateKeyLength);
      }
#endif
      return;
    }
  } 
} 

/* function: mdnpsim_authDeleteUser */
TMWTYPES_USHORT TMWDEFS_GLOBAL mdnpsim_authDeleteUser(
  void            *pHandle, 
  void            *userDbHandle)
{
  TMWTYPES_USHORT  userNumber;
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;
  MDNPSIM_AUTHUSER *pUser = TMWDEFS_NULL;

  while((pUser = (MDNPSIM_AUTHUSER *)tmwdlist_getAfter(
      &pDbHandle->authUsers, (TMWDLIST_MEMBER *)pUser)) != TMWDEFS_NULL)
  {
    if(pUser == userDbHandle)
    {
      userNumber = pUser->userNumber;
      tmwdlist_removeEntry(&pDbHandle->authUsers, (TMWDLIST_MEMBER*)pUser);
      return userNumber;
    }
  } 
  return 0;
}

/* function: mdnpsim_setNextRcvdIsCritical */
void TMWDEFS_GLOBAL mdnpsim_setNextRcvdIsCritical(
  void *pHandle,
  TMWTYPES_BOOL value)
{
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;
  pDbHandle->nextRcvdIsCritical = value;
}

/* function: mdnpsim_getNextRcvdIsCritical */
TMWTYPES_BOOL  TMWDEFS_GLOBAL mdnpsim_getNextRcvdIsCritical(
  void *pHandle)
{
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;
  return(pDbHandle->nextRcvdIsCritical);
}

/* function: _securityStatisticString */
static const char * TMWDEFS_LOCAL _securityStatisticString(
  TMWTYPES_USHORT index)
{
  switch(index)
  {
  case DNPAUTH_UNEXPECTED_MSG_INDEX:    return("Unexpected Messages");
  case DNPAUTH_AUTHOR_FAIL_INDEX:       return("Authorization Failures");
  case DNPAUTH_AUTHENT_FAIL_INDEX:      return("Authentication Failures");
  case DNPAUTH_REPLY_TIMEOUT_INDEX:     return("Reply Timeouts ");
  case DNPAUTH_REKEY_DUETOFAIL_INDEX:   return("Rekeys Due to Authentication Failure");
  case DNPAUTH_TOTAL_MSG_SENT_INDEX:    return("Total Messages Sent");
  case DNPAUTH_TOTAL_MSG_RCVD_INDEX:    return("Total Messages Received");
  case DNPAUTH_CRIT_MSG_SENT_INDEX:     return("Critical Messages Sent");
  case DNPAUTH_CRIT_MSG_RCVD_INDEX:     return("Critical Messages Received");
  case DNPAUTH_DISCARDED_MSG_INDEX:     return("Discarded Messages");
  case DNPAUTH_ERROR_MSG_SENT_INDEX:    return("Error Messages Sent");
  case DNPAUTH_ERROR_MSG_RCVD_INDEX:    return("Error Messages Rcvd");
  case DNPAUTH_SUCCESS_AUTHENT_INDEX:   return("Successful Authentications");
  case DNPAUTH_SESKEY_CHANGE_INDEX:     return("Session Key Changes");
  case DNPAUTH_FAILSESKEY_CHANGE_INDEX: return("Failed Session Key Changes");
  case DNPAUTH_UPDKEY_CHANGE_INDEX:     return("Update Key Changes");     
  case DNPAUTH_FAILUPDKEY_CHANGE_INDEX: return("Failed Update Key Changes");  
  case DNPAUTH_REKEY_DUE_RESTART_INDEX: return("Rekeys Due to Restarts");

  default:                              return("Unknown");     
  }
}

/* function: mdnpsim_addSecStats */
/* Add Master Security Statistics points as required for SA */
TMWDEFS_SCL_API void TMWDEFS_GLOBAL mdnpsim_addSecStats(
  void *pHandle)
{
#if MDNPDATA_SUPPORT_OBJ120
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;
  int i;
  if (pDbHandle->pMDNPSession->authenticationEnabled && (pDbHandle->pMDNPSession->dnp.operateInV2Mode == TMWDEFS_FALSE))
  {
    for (i = 0; i < DNPAUTH_NUMBER_STATISTICS; i++)
    {
      mdnpsim_authSecStatAddPoint(pDbHandle, TMWDEFS_FALSE, (TMWTYPES_USHORT)i);
    }
  }
#else
  TMWTARG_UNUSED_PARAM(pHandle);
#endif
}

/* function: mdnpsim_authSecStatAddPoint */
TMWDEFS_GLOBAL void *mdnpsim_authSecStatAddPoint(
  void *pHandle,
  TMWTYPES_BOOL fromOS,
  TMWTYPES_USHORT pointNumber)
{
  TMWSIM_POINT *pPoint; 
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle; 
  /* Use group 122 for statistics from the outstation and 121 for master stats even though 
   * stats from OS come as both 121 and 122
   */
  if(fromOS)
  {
    pPoint = tmwsim_tableAdd(&pDbHandle->authSecStatsFromOS, pointNumber);
    if(pPoint != TMWDEFS_NULL)
    {
      tmwsim_initCounter(pPoint, pHandle, pointNumber); 
      pPoint->pSCLHandle = (void*)pDbHandle->pMDNPSession;
      tmwsim_setDescription(pPoint, (TMWTYPES_CHAR *)_securityStatisticString(pointNumber));
      _callCallback(pPoint, TMWSIM_POINT_ADD, DNPDEFS_OBJ_122_AUTHSTATEVENTS, pointNumber);
    }
  }
  else
  { 
    pPoint = tmwsim_tableAdd(&pDbHandle->authSecStatsFromMaster, pointNumber);
    if(pPoint != TMWDEFS_NULL)
    {
      tmwsim_initCounter(pPoint, pHandle, pointNumber); 
      pPoint->pSCLHandle = (void*)pDbHandle->pMDNPSession;
      tmwsim_setDescription(pPoint, (TMWTYPES_CHAR *)_securityStatisticString(pointNumber));
      _callCallback(pPoint, TMWSIM_POINT_ADD, DNPDEFS_OBJ_121_AUTHSECSTATS, pointNumber);
    }
  }
  return(pPoint);
}

/* function: mdnpsim_authSecStatDeletePoint */
TMWDEFS_GLOBAL void mdnpsim_authSecStatDeletePoint(
  void *pHandle,
  TMWTYPES_BOOL fromOS,
  void *pPoint)
{
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;    
  TMWTYPES_USHORT pointNumber = (TMWTYPES_USHORT)tmwsim_getPointNumber((TMWSIM_POINT *)pPoint); 
  /* Use group 122 for statistics from the outstation and 121 for master stats even though 
   * stats from OS come as both 121 and 122
   */
  if(fromOS)
  {
    _callRemoveCallback(pDbHandle, TMWSIM_POINT_DELETE, DNPDEFS_OBJ_122_AUTHSTATEVENTS, pointNumber);
    tmwsim_tableDelete(&pDbHandle->authSecStatsFromOS, pointNumber);
  }
  else
  {
    _callRemoveCallback(pDbHandle, TMWSIM_POINT_DELETE, DNPDEFS_OBJ_121_AUTHSECSTATS, pointNumber);
    tmwsim_tableDelete(&pDbHandle->authSecStatsFromMaster, pointNumber);
  }
}

/* function: mdnpsim_authSecStatGetPoint */
TMWDEFS_GLOBAL void *mdnpsim_authSecStatGetPoint(
  void *pHandle,
  TMWTYPES_BOOL fromOS,
  TMWTYPES_USHORT index)
{
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;  
  /* Index and point number are not the same for mdnp, because there may be gaps */
  if(fromOS)
    return(tmwsim_tableFindPointByIndex(&pDbHandle->authSecStatsFromOS, index));
  else
    return(tmwsim_tableFindPointByIndex(&pDbHandle->authSecStatsFromMaster, index));
}

/* function: mdnpsim_authSecStatLookupPoint */
TMWDEFS_GLOBAL void *mdnpsim_authSecStatLookupPoint(
  void *pHandle,
  TMWTYPES_BOOL fromOS,
  TMWTYPES_USHORT pointNumber)
{
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;
  if(fromOS)
    return(tmwsim_tableFindPoint(&pDbHandle->authSecStatsFromOS, pointNumber));
  else
    return(tmwsim_tableFindPoint(&pDbHandle->authSecStatsFromMaster, pointNumber));

}

/* function: mdnpsim_storeAuthSecStat */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpsim_storeAuthSecStat(
  void *pHandle,
  TMWTYPES_BOOL fromOS,
  TMWTYPES_USHORT assocId,
  TMWTYPES_USHORT pointNumber,
  TMWTYPES_ULONG value,
  TMWTYPES_UCHAR flags,
  TMWDTIME *pTimeStamp)
{
  TMWSIM_POINT *pPoint;
  TMWTARG_UNUSED_PARAM(assocId);

  pPoint = (TMWSIM_POINT *)mdnpsim_authSecStatLookupPoint(pHandle, fromOS, pointNumber);
  if(pPoint == TMWDEFS_NULL)
  {
    pPoint = (TMWSIM_POINT *)mdnpsim_authSecStatAddPoint(pHandle, fromOS, pointNumber);
    if(pPoint == TMWDEFS_NULL)
      return TMWDEFS_FALSE;
  }
  /* Use group 122 for statistics from the outstation and 121 for master stats even though 
   * stats from OS come as both 121 and 122
   */
  if(fromOS)
  {
    tmwsim_setFlags(pPoint, flags, TMWDEFS_CHANGE_REMOTE_OP);
    tmwsim_setCounterValue(pPoint, value, TMWDEFS_CHANGE_REMOTE_OP);
    if(pTimeStamp != TMWDEFS_NULL)
      tmwsim_setTimeStamp(pPoint, pTimeStamp);
    else
      tmwdtime_getDateTime((TMWSESN*)pPoint->pSCLHandle, &pPoint->timeStamp); 
    _callCallback(pPoint, TMWSIM_POINT_UPDATE, DNPDEFS_OBJ_122_AUTHSTATEVENTS, pointNumber);
      
    return TMWDEFS_TRUE;
  }
  else
  {
    tmwsim_setFlags(pPoint, flags, TMWDEFS_CHANGE_REMOTE_OP);
    tmwsim_setCounterValue(pPoint, value, TMWDEFS_CHANGE_REMOTE_OP);
    tmwdtime_getDateTime((TMWSESN*)pPoint->pSCLHandle, &pPoint->timeStamp);
    _callCallback(pPoint, TMWSIM_POINT_UPDATE, DNPDEFS_OBJ_121_AUTHSECSTATS, pointNumber);
       
    return TMWDEFS_TRUE;
  }
}

/* function: mdnpsim_authSecStatGetValue */
TMWTYPES_ULONG TMWDEFS_GLOBAL mdnpsim_authSecStatGetValue(
  void *pPoint) 
{
  return(tmwsim_getCounterValue((TMWSIM_POINT *)pPoint)); 
}
#endif /* MDNPDATA_SUPPORT_OBJ120 */

#if TMWCNFG_SUPPORT_DIAG
/* function: mdnpsim_showData */
void TMWDEFS_GLOBAL mdnpsim_showData(
  TMWSESN *pSession)
{
  MDNPSESN *pMDNPSession = (MDNPSESN *)pSession;
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pMDNPSession->pDbHandle;
  TMWSIM_POINT *pDataPoint;
#if MDNPDATA_SUPPORT_DATASETS
  MDNPSIM_DATASET_PROTO *pProto;
  MDNPSIM_DATASET_DESCR_DATA *pDescr;
#endif
  TMWDIAG_ANLZ_ID anlzId;
  char buf[1024];

  if (tmwdiag_initId(&anlzId, TMWDEFS_NULL, pSession, TMWDEFS_NULL, TMWDIAG_ID_MMI) == TMWDEFS_FALSE)
  {
    return;
  }

  tmwdiag_skipLine(&anlzId);
  tmwdiag_putLine(&anlzId, "Binary Inputs:\n");

  pDataPoint = TMWDEFS_NULL;
  while((pDataPoint = tmwsim_tableGetNextPoint(
    &pDbHandle->binaryInputs, pDataPoint)) != TMWDEFS_NULL)
  {
    TMWTYPES_UCHAR flags = tmwsim_getFlags(pDataPoint);

    flags &= ~DNPDEFS_DBAS_FLAG_BINARY_ON;
    if(tmwsim_getBinaryValue(pDataPoint))
      flags |= DNPDEFS_DBAS_FLAG_BINARY_ON;

    tmwtarg_snprintf(buf, sizeof(buf), "%20sBinary Input Point %06d = 0x%02x\n", " ",
      tmwsim_getPointNumber(pDataPoint), flags);

    tmwdiag_putLine(&anlzId, buf);
  }

  tmwdiag_skipLine(&anlzId);
  tmwdiag_putLine(&anlzId, "Binary Outputs:\n");
  pDataPoint = TMWDEFS_NULL;
  while((pDataPoint = tmwsim_tableGetNextPoint(
    &pDbHandle->binaryOutputs, pDataPoint)) != TMWDEFS_NULL)
  {
    TMWTYPES_UCHAR flags = tmwsim_getFlags(pDataPoint);

    flags &= ~DNPDEFS_DBAS_FLAG_BINARY_ON;
    if(tmwsim_getBinaryValue(pDataPoint))
      flags |= DNPDEFS_DBAS_FLAG_BINARY_ON;

    tmwtarg_snprintf(buf, sizeof(buf), "%20sBinary Output Point %06d = 0x%02x\n", " ",
      tmwsim_getPointNumber(pDataPoint), flags);

    tmwdiag_putLine(&anlzId, buf);
  }

  tmwdiag_skipLine(&anlzId);
  tmwdiag_putLine(&anlzId, "Binary Counters:\n");
  pDataPoint = TMWDEFS_NULL;
  while((pDataPoint = tmwsim_tableGetNextPoint(
    &pDbHandle->binaryCounters, pDataPoint)) != TMWDEFS_NULL)
  {
    tmwtarg_snprintf(buf, sizeof(buf), "%20sBinary Counter Point %06d = %u, flags = 0x%02x\n", " ",
      tmwsim_getPointNumber(pDataPoint), tmwsim_getCounterValue(pDataPoint),
      tmwsim_getFlags(pDataPoint));

    tmwdiag_putLine(&anlzId, buf);
  }

  tmwdiag_skipLine(&anlzId);
  tmwdiag_putLine(&anlzId, "Frozen Counters:\n");
  pDataPoint = TMWDEFS_NULL;
  while((pDataPoint = tmwsim_tableGetNextPoint(
    &pDbHandle->frozenCounters, pDataPoint)) != TMWDEFS_NULL)
  {
    tmwtarg_snprintf(buf, sizeof(buf), "%20sFrozen Counter Point %06d = %u, flags = 0x%02x\n", " ",
      tmwsim_getPointNumber(pDataPoint), tmwsim_getCounterValue(pDataPoint),
      tmwsim_getFlags(pDataPoint));

    tmwdiag_putLine(&anlzId, buf);
  }

  tmwdiag_skipLine(&anlzId);
  tmwdiag_putLine(&anlzId, "Analog Inputs:\n");
  pDataPoint = TMWDEFS_NULL;
  while((pDataPoint = tmwsim_tableGetNextPoint(
    &pDbHandle->analogInputs, pDataPoint)) != TMWDEFS_NULL)
  {
    TMWSIM_DATA_TYPE value = tmwsim_getAnalogValue(pDataPoint); 

    tmwtarg_snprintf(buf, sizeof(buf), "%20sAnalog Input Point %06d = %.10g, flags 0x%02x deadband %.10g\n", " ",
        tmwsim_getPointNumber(pDataPoint), value,
        tmwsim_getFlags(pDataPoint), pDataPoint->data.analog.deadband);
   
    tmwdiag_putLine(&anlzId, buf);
  }

  tmwdiag_skipLine(&anlzId);
  tmwdiag_putLine(&anlzId, "Frozen Analog Inputs:\n");
  pDataPoint = TMWDEFS_NULL;
  while((pDataPoint = tmwsim_tableGetNextPoint(
    &pDbHandle->frozenAnalogInputs, pDataPoint)) != TMWDEFS_NULL)
  {
    TMWSIM_DATA_TYPE value = tmwsim_getAnalogValue(pDataPoint); 

    tmwtarg_snprintf(buf, sizeof(buf), "%20sFrozen Analog Input Point %06d = %.10g, flags 0x%02x\n", " ",
        tmwsim_getPointNumber(pDataPoint), value,
        tmwsim_getFlags(pDataPoint));
   
    tmwdiag_putLine(&anlzId, buf);
  }

  tmwdiag_skipLine(&anlzId);
  tmwdiag_putLine(&anlzId, "Analog Outputs:\n");
  pDataPoint = TMWDEFS_NULL;
  while((pDataPoint = tmwsim_tableGetNextPoint(
    &pDbHandle->analogOutputs, pDataPoint)) != TMWDEFS_NULL)
  {
    TMWSIM_DATA_TYPE value = tmwsim_getAnalogValue(pDataPoint); 

    tmwtarg_snprintf(buf, sizeof(buf), "%20sAnalog Output Point %06d = %.10g, flags 0x%02x\n", " ",
      tmwsim_getPointNumber(pDataPoint), value,
      tmwsim_getFlags(pDataPoint));
    
    tmwdiag_putLine(&anlzId, buf);
  }

  tmwdiag_skipLine(&anlzId);
  tmwdiag_putLine(&anlzId, "Indexed Absolute Time:\n");
  pDataPoint = TMWDEFS_NULL;
  while ((pDataPoint = tmwsim_tableGetNextPoint(
    &pDbHandle->indexedTime, pDataPoint)) != TMWDEFS_NULL)
  {
    TMWDTIME indexedTime;
    TMWTYPES_ULONG intervalCount = tmwsim_getIndexedTimeIntervalCount(pDataPoint);
    TMWTYPES_UCHAR intervalUnit = tmwsim_getIndexedTimeIntervalUnit(pDataPoint);
    TMWTYPES_CHAR timeBuf[64];

    tmwsim_getIndexedTimeTime(pDataPoint, &indexedTime);
    tmwdiag_time2string(&indexedTime, TMWDEFS_TIME_FORMAT_56, timeBuf, sizeof(timeBuf), TMWDEFS_FALSE);

    tmwtarg_snprintf(buf, sizeof(buf), "   %06d: start time %s, interval count %u, interval units %d\n",
      tmwsim_getPointNumber(pDataPoint), timeBuf, intervalCount, intervalUnit);

    tmwdiag_putLine(&anlzId, buf);
  }

#if MDNPDATA_SUPPORT_OBJ0
  tmwdiag_skipLine(&anlzId);
  tmwdiag_putLine(&anlzId, "Device Attributes:\n");

  pDataPoint = TMWDEFS_NULL;
  while((pDataPoint = tmwsim_tableGetNextPoint(
    &pDbHandle->deviceAttrs, pDataPoint)) != TMWDEFS_NULL)
  {
    TMWSIM_POINT *pAttribute = TMWDEFS_NULL;
    while((pAttribute = tmwsim_tableGetNextPoint(
      &pDataPoint->data.list.listHead, pAttribute)) != TMWDEFS_NULL)
    {
      if(pAttribute->data.attribute.type == DNPDEFS_ATTRIBUTE_TYPE_VSTR)
      { 
        TMWTYPES_UCHAR tmpBuf[255];
        memcpy(tmpBuf, pAttribute->data.attribute.pBuf, pAttribute->data.attribute.length);
        tmpBuf[pAttribute->data.attribute.length] = '\0';

        tmwtarg_snprintf(buf, sizeof(buf), "   Device Attribute  Point %d, Variation %3d, Property 0x%02x, %s\n",
          tmwsim_getPointNumber(pDataPoint), tmwsim_getPointNumber(pAttribute), pAttribute->data.attribute.property, tmpBuf);
      }
      else if((pAttribute->data.attribute.type == DNPDEFS_ATTRIBUTE_TYPE_OSTR)
        ||(pAttribute->data.attribute.type == DNPDEFS_ATTRIBUTE_TYPE_BSTR)) 
      { 
        int len;
        int j; 
        TMWTYPES_CHAR tmpBuf[255];

        len = 0;
        for(j=0; j<pAttribute->data.attribute.length; j++)
        {
          if(len > 250)
            break;
          len += tmwtarg_snprintf((tmpBuf + len), sizeof(tmpBuf)-len, "%02x ", pAttribute->data.attribute.pBuf[j]); 
        }
        tmpBuf[len] = '\0';
        tmwtarg_snprintf(buf, sizeof(buf), "   Device Attribute  Point %d, Variation %3d, Property 0x%02x, %s\n",
          tmwsim_getPointNumber(pDataPoint), tmwsim_getPointNumber(pAttribute), pAttribute->data.attribute.property, tmpBuf);
      } 
      else if(pAttribute->data.attribute.type == DNPDEFS_ATTRIBUTE_TYPE_DNP3TIME)
      { 
        TMWTYPES_CHAR timeBuf[255];
        tmwdiag_time2string(&pAttribute->data.attribute.timeValue, TMWDEFS_TIME_FORMAT_56, timeBuf, sizeof(timeBuf), TMWDEFS_FALSE);
        tmwtarg_snprintf(buf, sizeof(buf), "   Device Attribute  Point %d, Variation %3d, Property 0x%02x, %s\n",
          tmwsim_getPointNumber(pDataPoint), tmwsim_getPointNumber(pAttribute), pAttribute->data.attribute.property, timeBuf);
      }
      else
      { 
        tmwtarg_snprintf(buf, sizeof(buf), "   Device Attribute  Point %d, Variation %3d, Property 0x%02x, %.10g\n",
          tmwsim_getPointNumber(pDataPoint), tmwsim_getPointNumber(pAttribute), pAttribute->data.attribute.property, pAttribute->data.attribute.value);
      }
      tmwdiag_putLine(&anlzId, buf);
    }
  }
#endif

#if MDNPDATA_SUPPORT_DATASETS
  tmwdiag_skipLine(&anlzId);
  tmwdiag_putLine(&anlzId, "Data Set Prototypes:\n");

  pProto = TMWDEFS_NULL;
  while((pProto = (MDNPSIM_DATASET_PROTO *)tmwdlist_getAfter(&pDbHandle->datasetProtos, 
    (TMWDLIST_MEMBER *)pProto)) != TMWDEFS_NULL)
  {
    DNPDATA_DATASET_DESCR_ELEM *pDescrElem;
    char strbuf[128];
    int i; 
    
    if(pProto->fromSlave)
    {
      tmwtarg_snprintf(buf, sizeof(buf), "   Data Set Prototype ID %06d, Outstation Defined\n", pProto->protoId); 
    }
    else
    {
      tmwtarg_snprintf(buf, sizeof(buf), "   Data Set Prototype ID %06d, Master Defined\n", pProto->protoId); 
    }
    tmwdiag_putLine(&anlzId, buf); 

    tmwtarg_snprintf(buf, sizeof(buf), "     Element%3d, UUID= ", 1); 
    tmwdiag_putLine(&anlzId, buf);
    dnpdiag_displayOctets(&anlzId, pProto->uuid, 16, TMWDEFS_FALSE);   
    pDescrElem = pProto->contents;
    for(i=0; i < pProto->numberElems; i++)
    { 
      /* Need to Null terminate strings. */
      if(pDescrElem->ancillaryValue.type == DNPDATA_VALUE_STRPTR)
      { 
        memcpy(strbuf, pDescrElem->ancillaryValue.value.pStrValue, pDescrElem->ancillaryValue.length);
      }
      else if(pDescrElem->ancillaryValue.type == DNPDATA_VALUE_STRARRAY)
      { 
        memcpy(strbuf, pDescrElem->ancillaryValue.value.strValue, pDescrElem->ancillaryValue.length);
      }   
      strbuf[pDescrElem->ancillaryValue.length] = 0;

      /* See if all of the octets in the string are printable characters */
      if(_isPrintable((TMWTYPES_UCHAR *)strbuf, pDescrElem->ancillaryValue.length))
      {
        tmwtarg_snprintf(buf, sizeof(buf), "     Element%3d, %-7s %-7s maxlen=%d, %s\n", 
          i+2, dnpdiag_descrCodeToString(pDescrElem->descrElemType), dnpdiag_datasetTypeToString(pDescrElem->dataTypeCode),
          pDescrElem->maxDataLength, strbuf); 

        tmwdiag_putLine(&anlzId, buf);
      }
      else
      {
      }
      pDescrElem++;
    }
  }

  tmwdiag_skipLine(&anlzId);
  tmwdiag_putLine(&anlzId, "Data Set Descriptors:\n");

  pDescr = TMWDEFS_NULL;
  while((pDescr = (MDNPSIM_DATASET_DESCR_DATA *)tmwdlist_getAfter(&pDbHandle->datasetDescrDatas, 
    (TMWDLIST_MEMBER *)pDescr)) != TMWDEFS_NULL)
  {
    DNPDATA_DATASET_DESCR_ELEM *pDescrElem;
    DNPDATA_DATASET_DESCR_INDEX *pDescrIndex;
    char strbuf[128];
    int i;
    
    if(pDescr->fromSlave)
    {
      tmwtarg_snprintf(buf, sizeof(buf), "   Data Set Descriptor ID %06d, Outstation Defined, characteristics 0x%0x\n", 
        pDescr->datasetId, pDescr->characteristics); 
    }
    else
    {
      tmwtarg_snprintf(buf, sizeof(buf), "   Data Set Descriptor ID %06d, Master Defined\n", pDescr->datasetId); 
    }
    tmwdiag_putLine(&anlzId, buf);

    pDescrElem = pDescr->descrContents;
    for(i=0; i < pDescr->numberDescrElems; i++)
    {
      /* Need to Null terminate strings. */
      if(pDescrElem->ancillaryValue.type == DNPDATA_VALUE_STRPTR)
        memcpy(strbuf, pDescrElem->ancillaryValue.value.pStrValue, pDescrElem->ancillaryValue.length);
      else if(pDescrElem->ancillaryValue.type == DNPDATA_VALUE_STRARRAY)
        memcpy(strbuf, pDescrElem->ancillaryValue.value.strValue, pDescrElem->ancillaryValue.length);

      strbuf[pDescrElem->ancillaryValue.length] = 0;

      if((pDescrElem->descrElemType == DNPDEFS_DATASET_DESCR_UUID)
        || (pDescrElem->descrElemType == DNPDEFS_DATASET_DESCR_PTYP))
      { 
        tmwtarg_snprintf(buf, sizeof(buf), "     Element%3d, %-7s %-7s maxlen=%d, UUID= ", 
          i+1, dnpdiag_descrCodeToString(pDescrElem->descrElemType), dnpdiag_datasetTypeToString(pDescrElem->dataTypeCode),
          pDescrElem->maxDataLength); 
        tmwdiag_putLine(&anlzId, buf);
        dnpdiag_displayOctets(&anlzId, (TMWTYPES_UCHAR *)strbuf, 16, TMWDEFS_FALSE);
        
        /* Prototype may contain optional visible string name after 16 octet UUID */
        if(pDescrElem->ancillaryValue.length > 16)
        {
          tmwtarg_snprintf(buf, sizeof(buf), "%42s Optional Name= %s\n", " ",&strbuf[16]);  
          tmwdiag_putLine(&anlzId, buf);
        }  
      }
      else
      {
        tmwtarg_snprintf(buf, sizeof(buf), "     Element%3d, %-7s %-7s maxlen=%d, %s\n", 
          i+1, dnpdiag_descrCodeToString(pDescrElem->descrElemType), dnpdiag_datasetTypeToString(pDescrElem->dataTypeCode),
          pDescrElem->maxDataLength, strbuf); 
        tmwdiag_putLine(&anlzId, buf);
      }
      pDescrElem++;
    }

    pDescrIndex = pDescr->descrIndex;
    for(i=0; i < pDescr->numberIndexElems; i++)
    {
      tmwtarg_snprintf(buf, sizeof(buf), "     Element%3d, Point Type %3d  Index %d\n", 
        i+1, pDescrIndex->pointType, pDescrIndex->pointIndex); 
        tmwdiag_putLine(&anlzId, buf);
        pDescrIndex++;
    }
  }

  tmwdiag_skipLine(&anlzId);
  tmwdiag_putLine(&anlzId, "Data Sets:\n");

  pDescr = TMWDEFS_NULL;
  while((pDescr = (MDNPSIM_DATASET_DESCR_DATA *)tmwdlist_getAfter(&pDbHandle->datasetDescrDatas, 
    (TMWDLIST_MEMBER *)pDescr)) != TMWDEFS_NULL)
  {
    DNPDATA_DATASET_VALUE *pElem;
    int i;
    char strbuf[128];

    if(pDescr->fromSlave)   
      tmwtarg_snprintf(buf, sizeof(buf), "   Data Set ID %06d, Outstation Defined\n", pDescr->datasetId); 
    else
    {
     tmwtarg_snprintf(buf, sizeof(buf), "   Data Set ID %06d, Master Defined\n",pDescr->datasetId); 
    }
    tmwdiag_putLine(&anlzId, buf);

    /* If empty, don't try to display the time */
    if(pDescr->numberDataElems > 0)
    { 
      tmwdiag_time2string(&pDescr->timeStamp, TMWDEFS_TIME_FORMAT_56, strbuf, sizeof(strbuf), TMWDEFS_FALSE);
      tmwtarg_snprintf(buf, sizeof(buf), "     Element%3d, %s\n", 1, strbuf); 
      tmwdiag_putLine(&anlzId, buf);
    }

    pElem = pDescr->dataElem;
    for(i=0; i < pDescr->numberDataElems; i++)
    {
      TMWTYPES_BOOL indent;
      switch(pElem->type)
      {
      case DNPDATA_VALUE_STRPTR:   
      {
        /* See if all of the octets in the string are printable characters */
        if(pDescr->displayValueAsHex[i])
        {   
          if(pElem->length >= 16)
          {
            (void)tmwtarg_snprintf(buf, sizeof(buf), "     Element%3d, len=%d  \n",
              i+2, pElem->length);
            indent = TMWDEFS_TRUE;
          }
          else
          {
            (void)tmwtarg_snprintf(buf, sizeof(buf), "     Element%3d, len=%d  ",
              i+2, pElem->length);
            indent = TMWDEFS_FALSE;
          }
          tmwdiag_putLine(&anlzId, buf);
          dnpdiag_displayOctets(&anlzId, pElem->value.pStrValue, pElem->length, indent);
          pElem++;
          continue;
        }

        /* All printable characters, copy them, null terminate them and display them */
        memcpy(strbuf, pElem->value.pStrValue, pElem->length);
        strbuf[pElem->length] = 0;
        tmwtarg_snprintf(buf, sizeof(buf), "     Element%3d, len=%d  %s\n",
          i+2, pElem->length, strbuf);

        break; 
      }
      case DNPDATA_VALUE_STRARRAY:  
      {
        if(pDescr->displayValueAsHex[i])
        {   
          if(pElem->length >= 16)
          {
            (void)tmwtarg_snprintf(buf, sizeof(buf), "     Element%3d, len=%d  \n",
              i+2, pElem->length);
            indent = TMWDEFS_TRUE;
          }
          else
          {
            (void)tmwtarg_snprintf(buf, sizeof(buf), "     Element%3d, len=%d  ",
              i+2, pElem->length);
            indent = TMWDEFS_FALSE;
          }
          tmwdiag_putLine(&anlzId, buf);
          dnpdiag_displayOctets(&anlzId, pElem->value.strValue, pElem->length, indent);

          pElem++;
          continue;
        }

        /* All printable characters, copy them, null terminate them and display them */
        memcpy(strbuf, pElem->value.strValue, pElem->length);
        strbuf[pElem->length] = 0;
        tmwtarg_snprintf(buf, sizeof(buf), "     Element%3d, len=%d  %s\n",
          i+2, pElem->length, strbuf);

        break; 
      }
      case DNPDATA_VALUE_UINT32:    
        tmwtarg_snprintf(buf, sizeof(buf), "     Element%3d, len=%d  %u\n", 
          i+2, pElem->length, pElem->value.uint32Value);
        break;
      case DNPDATA_VALUE_INT32:  
        tmwtarg_snprintf(buf, sizeof(buf), "     Element%3d, len=%d  %d\n", 
          i+2, pElem->length, pElem->value.int32Value);
        break;
      case DNPDATA_VALUE_SFLT:  
        tmwtarg_snprintf(buf, sizeof(buf), "     Element%3d, len=%d  %g\n", 
          i+2, pElem->length, pElem->value.sfltValue);
        break;
      case DNPDATA_VALUE_DOUBLE:
        tmwtarg_snprintf(buf, sizeof(buf), "     Element%3d, len=%d  %g\n", 
          i+2, pElem->length, pElem->value.doubleValue);
        break;
      case DNPDATA_VALUE_TIME: 
        tmwdiag_time2string(&pElem->value.timeValue, TMWDEFS_TIME_FORMAT_56, strbuf, sizeof(strbuf), TMWDEFS_FALSE);
        tmwtarg_snprintf(buf, sizeof(buf), "     Element%3d, len=%d  %s\n", 
          i+2, pElem->length, strbuf);
        break;
      }
      tmwdiag_putLine(&anlzId, buf);
      pElem++;
    }
  }
#endif

  tmwdiag_skipLine(&anlzId);
  tmwdiag_putLine(&anlzId, "String Data:\n");
  pDataPoint = TMWDEFS_NULL;
  while((pDataPoint = tmwsim_tableGetNextPoint(
    &pDbHandle->stringData, pDataPoint)) != TMWDEFS_NULL)
  {
    if (pDataPoint->data.string.extString == TMWDEFS_FALSE)
    {
      TMWTYPES_UCHAR tmpBuf[DNPDEFS_MAX_STRING_LENGTH+1];
      TMWTYPES_UCHAR length;

      tmwsim_getStringValue(pDataPoint, DNPDEFS_MAX_STRING_LENGTH, tmpBuf, &length);
      tmpBuf[length] = '\0';


      tmwtarg_snprintf(buf, sizeof(buf), "%20sString Data %06d = %s\n", " ",
        tmwsim_getPointNumber(pDataPoint), tmpBuf);

      tmwdiag_putLine(&anlzId, buf);
    }
  }

  tmwdiag_skipLine(&anlzId);
  tmwdiag_putLine(&anlzId, "Virtual Terminal Events:\n");
  pDataPoint = TMWDEFS_NULL;
  while((pDataPoint = tmwsim_tableGetNextPoint(
    &pDbHandle->vtermEvents, pDataPoint)) != TMWDEFS_NULL)
  {
    TMWTYPES_UCHAR tmpBuf[DNPDEFS_MAX_STRING_LENGTH+1];
    TMWTYPES_UCHAR length;

    tmwsim_getStringValue(pDataPoint, DNPDEFS_MAX_STRING_LENGTH, tmpBuf, &length);
    tmpBuf[length] = '\0';

    tmwtarg_snprintf(buf, sizeof(buf), "%20sVirtual Terminal Output %06d = %s\n", " ",
      tmwsim_getPointNumber(pDataPoint), tmpBuf);

    tmwdiag_putLine(&anlzId, buf);
  }

#if MDNPDATA_SUPPORT_OBJ114
  tmwdiag_skipLine(&anlzId);
  tmwdiag_putLine(&anlzId, "Extended String Data:\n");
  pDataPoint = TMWDEFS_NULL;
  while((pDataPoint = tmwsim_tableGetNextPoint(
    &pDbHandle->stringData, pDataPoint)) != TMWDEFS_NULL)
  {
    if (pDataPoint->data.string.extString == TMWDEFS_TRUE)
    {
      TMWTYPES_UCHAR strBuf[DNPCNFG_MAX_DIAG_STRING_LENGTH +8];
      TMWTYPES_USHORT length;

      tmwsim_getExtStringValue(pDataPoint, DNPCNFG_MAX_DIAG_STRING_LENGTH+1, strBuf, &length);
      if (length > DNPCNFG_MAX_DIAG_STRING_LENGTH)
      {
        length = DNPCNFG_MAX_DIAG_STRING_LENGTH;
        strBuf[length++] = '.';
        strBuf[length++] = '.';
        strBuf[length++] = '.';
      }
      strBuf[length] = '\0';

      tmwtarg_snprintf(buf, sizeof(buf), "%20sString Data %06d = %s\n", " ",
        tmwsim_getPointNumber(pDataPoint), strBuf);

      tmwdiag_putLine(&anlzId, buf);
    }
  }
#endif

  tmwdiag_skipLine(&anlzId);
  tmwdiag_putLine(&anlzId, "Security Statistics:\n");
  pDataPoint = TMWDEFS_NULL; while((pDataPoint = tmwsim_tableGetNextPoint(
    &pDbHandle->authSecStatsFromMaster, pDataPoint)) != TMWDEFS_NULL)
  {
    tmwtarg_snprintf(buf, sizeof(buf), "%20sMaster Security Statistic Point %06d = %u, flags = 0x%02x\n", " ",
      tmwsim_getPointNumber(pDataPoint), tmwsim_getCounterValue(pDataPoint),
      tmwsim_getFlags(pDataPoint));

    tmwdiag_putLine(&anlzId, buf);
  }
  pDataPoint = TMWDEFS_NULL;
  while((pDataPoint = tmwsim_tableGetNextPoint(
    &pDbHandle->authSecStatsFromOS, pDataPoint)) != TMWDEFS_NULL)
  {
    tmwtarg_snprintf(buf, sizeof(buf), "%20sOutstation Security Statistic Point %06d = %u, flags = 0x%02x\n", " ",
      tmwsim_getPointNumber(pDataPoint), tmwsim_getCounterValue(pDataPoint),
      tmwsim_getFlags(pDataPoint));

    tmwdiag_putLine(&anlzId, buf);
  }
}
#endif /* TMWCNFG_SUPPORT_DIAG */

#if MDNPSIM_SUPPORT_XML
static TMWTYPES_CHAR * TMWDEFS_LOCAL _putBoolField(
  TMWTYPES_CHAR *result, 
  TMWTYPES_CHAR *prefix, 
  TMWTYPES_CHAR *name, 
  TMWTYPES_BOOL flag)
{
  TMWTYPES_CHAR buf[128];
  tmwtarg_snprintf(buf, sizeof(buf), "%s<%s>%s</%s>\n", prefix, name, flag ? "true" : "false", name);
  return tmwtarg_appendString(result, buf);
}

#if MDNPDATA_SUPPORT_OBJ3
static TMWTYPES_CHAR * TMWDEFS_LOCAL _putDblBitValue(TMWTYPES_CHAR *result, TMWTYPES_UCHAR state)
{
  TMWTYPES_CHAR buf[128];

  switch(state & 0xc0)
  {
    case DNPDEFS_DBAS_FLAG_DOUBLE_INTER:
      tmwtarg_snprintf(buf, sizeof(buf), "%s<state>%s</state>\n", "     ", "intermediate");
      break;
    case DNPDEFS_DBAS_FLAG_DOUBLE_OFF:
      tmwtarg_snprintf(buf, sizeof(buf), "%s<state>%s</state>\n", "     ", "off");
      break;
    case DNPDEFS_DBAS_FLAG_DOUBLE_ON:
      tmwtarg_snprintf(buf, sizeof(buf), "%s<state>%s</state>\n", "     ", "on");
      break;
    case DNPDEFS_DBAS_FLAG_DOUBLE_INDET:
      tmwtarg_snprintf(buf, sizeof(buf), "%s<state>%s</state>\n", "     ", "indeterminate");
      break;
  }

  return tmwtarg_appendString(result, buf);
}
#endif

#if MDNPDATA_SUPPORT_DATASETS
static char * TMWDEFS_LOCAL _toDescrType(TMWTYPES_ULONG type)
{ 
  switch(type)
  {
  case DNPDEFS_DATASET_DESCR_ID: 
    return "id";
    break;
   
  case DNPDEFS_DATASET_DESCR_UUID:
    return "uuid";
    break;
   
  case DNPDEFS_DATASET_DESCR_NSPC:
    return "nspc";
    break;
   
  case DNPDEFS_DATASET_DESCR_NAME:
    return "name";
    break;
   
  case DNPDEFS_DATASET_DESCR_DAEL:
    return "dael";
    break;
   
  case DNPDEFS_DATASET_DESCR_PTYP:
    return "ptyp";
    break;
   
  case DNPDEFS_DATASET_DESCR_CTLV:
    return "ctlv";
    break;
   
  case DNPDEFS_DATASET_DESCR_CTLS:
    return "ctls";
    break;

  default:
    return "unknown";
    break;
  }
}

static char * TMWDEFS_LOCAL _toDataType(TMWTYPES_ULONG type) 
{ 
  switch(type)
  {
  case DNPDEFS_DATASET_TYPE_VSTR: 
    return "vstr";

  case DNPDEFS_DATASET_TYPE_UINT: 
    return "uint"; 

  case DNPDEFS_DATASET_TYPE_INT: 
    return "int";
 
  case DNPDEFS_DATASET_TYPE_FLT: 
    return "flt";
  
  case DNPDEFS_DATASET_TYPE_OSTR: 
    return "ostr";
 
  case DNPDEFS_DATASET_TYPE_BSTR: 
    return "bstr";
 
  case DNPDEFS_DATASET_TYPE_TIME: 
    return "time";
 
  case DNPDEFS_DATASET_TYPE_UNCD: 
    return "uncd";
 
  default: 
    return "none";
 
  }
} 

static void TMWDEFS_LOCAL _toDatasetOutData(DNPDATA_DATASET_VALUE *pDataSetValue, TMWTYPES_BOOL displayValueAsHex, TMWTYPES_UCHAR *pOut, int maxLength)
{
  int length;
  TMWTYPES_CHAR tempBuf[256];

  switch(pDataSetValue->type)
  {
    case DNPDATA_VALUE_STRPTR:   
      length = pDataSetValue->length;
      if(!displayValueAsHex)
      {
        if(length+1 > maxLength)
          length = maxLength-1;
        memcpy(tempBuf, pDataSetValue->value.pStrValue, length); 
      } 
      else
      {  
        int j;
        int len = 0;
        for(j=0; j<length; j++)
        { 
          if(len < (maxLength-4))
          {
          len += tmwtarg_snprintf((tempBuf + len), sizeof(tempBuf)-len, "%02x ", pDataSetValue->value.pStrValue[j]);
          }
        }
        length = len;
      }
      break; 
    case DNPDATA_VALUE_STRARRAY: 
      length = pDataSetValue->length;
      if(!displayValueAsHex)
      {
        if(length+1 > maxLength)
          length = maxLength-1;

        memcpy(tempBuf, pDataSetValue->value.strValue, length); 
      }
      else
      {  
        int j;
        int len = 0;
        for(j=0; j<length; j++)
        {
          if(len < (maxLength-4))
          {
            len += tmwtarg_snprintf((tempBuf + len), sizeof(tempBuf)-len, "%02x ", pDataSetValue->value.strValue[j]);
          }
        }
        length = len;
      }
      break;    
    case DNPDATA_VALUE_UINT32:    
      length = tmwtarg_snprintf(tempBuf, maxLength, "%u",pDataSetValue->value.uint32Value);
      break;
    case DNPDATA_VALUE_INT32:  
      length = tmwtarg_snprintf(tempBuf, maxLength, "%d",pDataSetValue->value.int32Value); 
      break;
    case DNPDATA_VALUE_SFLT:  
      length = tmwtarg_snprintf(tempBuf, maxLength, "%g",pDataSetValue->value.sfltValue);  
      break;
    case DNPDATA_VALUE_DOUBLE: 
      length = tmwtarg_snprintf(tempBuf, maxLength, "%g",pDataSetValue->value.doubleValue);  
      break;
    case DNPDATA_VALUE_TIME: 
      {
        char timeBuf[64];
        tmwdiag_time2string(&pDataSetValue->value.timeValue, TMWDEFS_TIME_FORMAT_56, timeBuf, sizeof(timeBuf), TMWDEFS_FALSE);
        length = tmwtarg_snprintf(tempBuf, maxLength, timeBuf);
      }
      break;
    default:
      *pOut = 0;
      return;
      break;
  }  
 
  memcpy(pOut, tempBuf, length);
  *(pOut+length) = 0;
}
#endif

#if SDNPDATA_SUPPORT_OBJ0
static void TMWDEFS_LOCAL _toDeviceAttOutData(TMWSIM_ATTRIBUTE *pAttribute, TMWTYPES_UCHAR *pOut, int *pLength)
{
  int length = 0; 
  TMWTYPES_CHAR tempBuf[256];

  switch(pAttribute->type)
  {
    case DNPDEFS_ATTRIBUTE_TYPE_VSTR:   
      length = pAttribute->length;
      if(length+1 > *pLength)
        length = (*pLength)-1;  
      memcpy(tempBuf, pAttribute->pBuf, length); 
      break;  
    case DNPDEFS_ATTRIBUTE_TYPE_UINT:   
    case DNPDEFS_ATTRIBUTE_TYPE_INT:    
    case DNPDEFS_ATTRIBUTE_TYPE_FLT: 
      length = tmwtarg_snprintf(tempBuf, *pLength, "%g",pAttribute->value);  
      break; 
    case DNPDEFS_ATTRIBUTE_TYPE_DNP3TIME:
      tmwdiag_time2string(&pAttribute->timeValue, TMWDEFS_TIME_FORMAT_56, tempBuf, sizeof(tempBuf), TMWDEFS_FALSE);
      length = tmwtarg_snprintf(tempBuf, *pLength, "%s", tempBuf);  
      break;
    case DNPDEFS_ATTRIBUTE_TYPE_OSTR:  
    case DNPDEFS_ATTRIBUTE_TYPE_BSTR:  
      {   
        int j;
        int len = 0;
        /* Make sure it fits */
        length = pAttribute->length;
        if(length+1 > *pLength)
          length = (*pLength)-1;  
        for(j=0; j< length; j++)
          len += tmwtarg_snprintf((tempBuf + len), sizeof(tempBuf)-len, "%02x ", pAttribute->pBuf[j]);
        length = len;
      }
      break;
    default:
      length = 0; 
      break;
  }  

  *pLength = length;
  if(length > 0)
  {
    memcpy(pOut, tempBuf, length);
    *(pOut+length) = 0;
  }
}
#endif

#define LOCAL_BUF_SIZE 2048
/* routine: mdnpsim_saveDatabase */
TMWTYPES_CHAR * TMWDEFS_GLOBAL mdnpsim_saveDatabase(TMWSESN *pSession)
{
  MDNPSESN *pMDNPSession = (MDNPSESN *)pSession;
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pMDNPSession->pDbHandle;
  TMWTYPES_CHAR *result = TMWDEFS_NULL;
  TMWTYPES_CHAR buf[LOCAL_BUF_SIZE];
  TMWSIM_POINT *pPoint;

  if(pMDNPSession == TMWDEFS_NULL)
    return(TMWDEFS_NULL);

  result = tmwtarg_appendString(result, "<?xml version=\"1.0\"?>\n");
  result = tmwtarg_appendString(result, "<tmw:dnpdata\n");
  result = tmwtarg_appendString(result, " xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n");
  result = tmwtarg_appendString(result, " xmlns:tmw=\"http://www.TriangleMicroWorks.com/TestHarness/Schemas/dnpdata\">\n");
  result = tmwtarg_appendString(result, " <device>\n");

  result = tmwtarg_appendString(result, "  <deviceAttributeGroup>\n");


#if SDNPDATA_SUPPORT_OBJ0
  pPoint = TMWDEFS_NULL;
  while((pPoint = tmwsim_tableGetNextPoint(
    &pDbHandle->deviceAttrs, pPoint)) != TMWDEFS_NULL)
  {
    TMWSIM_POINT *pAttribute = TMWDEFS_NULL;

    result = tmwtarg_appendString(result, "   <deviceAttribute>\n");

    tmwtarg_snprintf(buf, sizeof(buf), "    <pointNumber>%d</pointNumber>\n", tmwsim_getPointNumber(pPoint));
    result = tmwtarg_appendString(result, buf);

    while((pAttribute = tmwsim_tableGetNextPoint(
      &pPoint->data.list.listHead, pAttribute)) != TMWDEFS_NULL)
    { 
      TMWTYPES_CHAR *desc;
      int len;
      TMWTYPES_UCHAR value[256]; 
   
      result = tmwtarg_appendString(result, "    <deviceAttributeElement>\n");

      tmwtarg_snprintf(buf, sizeof(buf), "     <variation>%d</variation>\n", tmwsim_getPointNumber(pAttribute));
      result = tmwtarg_appendString(result, buf);

      tmwtarg_snprintf(buf, sizeof(buf), "     <property>%d</property>\n", pAttribute->data.attribute.property);
      result = tmwtarg_appendString(result, buf);

      tmwtarg_snprintf(buf, sizeof(buf), "     <dataType>%d</dataType>\n", pAttribute->data.attribute.type);
      result = tmwtarg_appendString(result, buf);

      tmwtarg_snprintf(buf, sizeof(buf), "     <length>%d</length>\n", pAttribute->data.attribute.length);
      result = tmwtarg_appendString(result, buf);
      
      desc = tmwsim_getDescription((TMWSIM_POINT*)pAttribute);
      if(desc && (strlen(desc) > 0))
      {
        tmwsim_xmlFormatDesc(desc, buf, sizeof(buf));
        result = tmwtarg_appendString(result, buf);
      }

      len = 256;
      _toDeviceAttOutData(&pAttribute->data.attribute, value, &len);
      tmwtarg_snprintf(buf, sizeof(buf), "     <value>%s</value>\n", value);
      result = tmwtarg_appendString(result, buf);
      
      result = tmwtarg_appendString(result, "    </deviceAttributeElement>\n");
    }

    result = tmwtarg_appendString(result, "   </deviceAttribute>\n");
  }
#endif
  result = tmwtarg_appendString(result, "  </deviceAttributeGroup>\n");

  result = tmwtarg_appendString(result, "  <binaryInputGroup>\n");

  pPoint = TMWDEFS_NULL;
  while((pPoint = tmwsim_tableGetNextPoint(&pDbHandle->binaryInputs, pPoint)) != TMWDEFS_NULL)
  {
    TMWTYPES_ULONG pointNum = tmwsim_getPointNumber(pPoint);
    TMWTYPES_CHAR *desc = tmwsim_getDescription(pPoint);
    TMWTYPES_UCHAR flags = tmwsim_getFlags(pPoint);

    flags &= ~DNPDEFS_DBAS_FLAG_BINARY_ON;
    if(tmwsim_getBinaryValue(pPoint))
      flags |= DNPDEFS_DBAS_FLAG_BINARY_ON;

    result = tmwtarg_appendString(result, "   <binaryInput>\n");

    tmwtarg_snprintf(buf, sizeof(buf), "    <pointNumber>%d</pointNumber>\n", pointNum);
    result = tmwtarg_appendString(result, buf);

    if(desc && (strlen(desc) > 0))
    {
      tmwsim_xmlFormatDesc(desc, buf, sizeof(buf));
      result = tmwtarg_appendString(result, buf);
    }
    
    result = tmwtarg_appendString(result, "    <flags>\n");
    result = _putBoolField(result, "     ", "online", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_ON_LINE));
    result = _putBoolField(result, "     ", "restart", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_RESTART));
    result = _putBoolField(result, "     ", "commLost", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_COMM_LOST));
    result = _putBoolField(result, "     ", "remoteForced", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_REMOTE_FORCED));
    result = _putBoolField(result, "     ", "localForced", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_LOCAL_FORCED));
    result = _putBoolField(result, "     ", "chatterFilter", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_CHATTER));
    result = _putBoolField(result, "     ", "state", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_BINARY_ON));
    result = tmwtarg_appendString(result, "    </flags>\n");

    result = tmwtarg_appendString(result, "   </binaryInput>\n");
  }
  result = tmwtarg_appendString(result, "  </binaryInputGroup>\n");

  result = tmwtarg_appendString(result, "  <doubleBitInputGroup>\n");
  pPoint = TMWDEFS_NULL;
  while((pPoint = tmwsim_tableGetNextPoint(&pDbHandle->doubleInputs, pPoint)) != TMWDEFS_NULL)
  {
    TMWTYPES_ULONG pointNum = tmwsim_getPointNumber(pPoint);
    TMWTYPES_CHAR *desc = tmwsim_getDescription(pPoint);
    TMWTYPES_UCHAR flags = tmwsim_getFlags(pPoint);

    flags &= ~DNPDEFS_DBAS_FLAG_DOUBLE_ON;
    if(tmwsim_getDoubleBinaryValue(pPoint))
      flags |= DNPDEFS_DBAS_FLAG_DOUBLE_ON;

    result = tmwtarg_appendString(result, "   <doubleBitInput>\n");

    tmwtarg_snprintf(buf, sizeof(buf), "    <pointNumber>%d</pointNumber>\n", pointNum);
    result = tmwtarg_appendString(result, buf);

    if(desc && (strlen(desc) > 0))
    {
      tmwsim_xmlFormatDesc(desc, buf, sizeof(buf));
      result = tmwtarg_appendString(result, buf);
    }
    
    result = tmwtarg_appendString(result, "    <flags>\n");
    result = _putBoolField(result, "     ", "online", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_ON_LINE));
    result = _putBoolField(result, "     ", "restart", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_RESTART));
    result = _putBoolField(result, "     ", "commLost", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_COMM_LOST));
    result = _putBoolField(result, "     ", "remoteForced", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_REMOTE_FORCED));
    result = _putBoolField(result, "     ", "localForced", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_LOCAL_FORCED));
    result = _putBoolField(result, "     ", "chatterFilter", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_CHATTER));
    result = _putDblBitValue(result, flags);
    result = tmwtarg_appendString(result, "    </flags>\n");

    result = tmwtarg_appendString(result, "   </doubleBitInput>\n");
  }
  result = tmwtarg_appendString(result, "  </doubleBitInputGroup>\n");

  result = tmwtarg_appendString(result, "  <binaryOutputControlGroup>\n");
  pPoint = TMWDEFS_NULL;
  while((pPoint = tmwsim_tableGetNextPoint(&pDbHandle->binaryOutputs, pPoint)) != TMWDEFS_NULL)
  {
    TMWTYPES_ULONG pointNum = tmwsim_getPointNumber(pPoint);
    TMWTYPES_CHAR *desc = tmwsim_getDescription(pPoint);

    result = tmwtarg_appendString(result, "   <binaryOutputControl>\n");

    tmwtarg_snprintf(buf, sizeof(buf), "    <pointNumber>%d</pointNumber>\n", pointNum);
    result = tmwtarg_appendString(result, buf);

    if(desc && (strlen(desc) > 0))
    {
      tmwsim_xmlFormatDesc(desc, buf, sizeof(buf));
      result = tmwtarg_appendString(result, buf);
    }
    
    result = tmwtarg_appendString(result, "   </binaryOutputControl>\n");
  }
  result = tmwtarg_appendString(result, "  </binaryOutputControlGroup>\n");

  result = tmwtarg_appendString(result, "  <binaryOutputStatusGroup>\n");
  pPoint = TMWDEFS_NULL;
  while((pPoint = tmwsim_tableGetNextPoint(&pDbHandle->binaryOutputs, pPoint)) != TMWDEFS_NULL)
  {
    TMWTYPES_ULONG pointNum = tmwsim_getPointNumber(pPoint);
    TMWTYPES_CHAR *desc = tmwsim_getDescription(pPoint);
    TMWTYPES_UCHAR flags = tmwsim_getFlags(pPoint);

    result = tmwtarg_appendString(result, "   <binaryOutputStatus>\n");

    tmwtarg_snprintf(buf, sizeof(buf), "    <pointNumber>%d</pointNumber>\n", pointNum);
    result = tmwtarg_appendString(result, buf);

    if(desc && (strlen(desc) > 0))
    {
      tmwsim_xmlFormatDesc(desc, buf, sizeof(buf));
      result = tmwtarg_appendString(result, buf);
    }
    
    result = tmwtarg_appendString(result, "    <flags>\n");
    result = _putBoolField(result, "     ", "online", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_ON_LINE));
    result = _putBoolField(result, "     ", "restart", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_RESTART));
    result = _putBoolField(result, "     ", "commLost", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_COMM_LOST));
    result = _putBoolField(result, "     ", "remoteForced", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_REMOTE_FORCED));
    result = _putBoolField(result, "     ", "localForced", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_LOCAL_FORCED));
    result = _putBoolField(result, "     ", "state", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_BINARY_ON));
    result = tmwtarg_appendString(result, "    </flags>\n");

    result = tmwtarg_appendString(result, "   </binaryOutputStatus>\n");
  }
  result = tmwtarg_appendString(result, "  </binaryOutputStatusGroup>\n");

  result = tmwtarg_appendString(result, "  <binaryCounterGroup>\n");
  pPoint = TMWDEFS_NULL;
  while((pPoint = tmwsim_tableGetNextPoint(&pDbHandle->binaryCounters, pPoint)) != TMWDEFS_NULL)
  {
    TMWTYPES_ULONG pointNum = tmwsim_getPointNumber(pPoint);
    TMWTYPES_CHAR *desc = tmwsim_getDescription(pPoint);
    TMWTYPES_ULONG value = tmwsim_getCounterValue(pPoint);
    TMWTYPES_UCHAR flags = tmwsim_getFlags(pPoint);

    result = tmwtarg_appendString(result, "   <binaryCounter>\n");

    tmwtarg_snprintf(buf, sizeof(buf), "    <pointNumber>%d</pointNumber>\n", pointNum);
    result = tmwtarg_appendString(result, buf);

    if(desc && (strlen(desc) > 0))
    {
      tmwsim_xmlFormatDesc(desc, buf, sizeof(buf));
      result = tmwtarg_appendString(result, buf);
    }
    
    result = tmwtarg_appendString(result, "    <flags>\n");
    result = _putBoolField(result, "     ", "online", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_ON_LINE));
    result = _putBoolField(result, "     ", "restart", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_RESTART));
    result = _putBoolField(result, "     ", "commLost", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_COMM_LOST));
    result = _putBoolField(result, "     ", "remoteForced", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_REMOTE_FORCED));
    result = _putBoolField(result, "     ", "localForced", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_LOCAL_FORCED));
    result = tmwtarg_appendString(result, "    </flags>\n");

    tmwtarg_snprintf(buf, sizeof(buf), "     <value>%u</value>\n", value);
    result = tmwtarg_appendString(result, buf);

    result = tmwtarg_appendString(result, "  </binaryCounter>\n");
  }
  result = tmwtarg_appendString(result, "   </binaryCounterGroup>\n");

  result = tmwtarg_appendString(result, "  <frozenCounterGroup>\n");
  pPoint = TMWDEFS_NULL;
  while((pPoint = tmwsim_tableGetNextPoint(&pDbHandle->frozenCounters, pPoint)) != TMWDEFS_NULL)
  {
    TMWTYPES_ULONG pointNum = tmwsim_getPointNumber(pPoint);
    TMWTYPES_CHAR *desc = tmwsim_getDescription(pPoint);
    TMWTYPES_ULONG value = tmwsim_getCounterValue(pPoint);
    TMWTYPES_UCHAR flags = tmwsim_getFlags(pPoint);

    result = tmwtarg_appendString(result, "   <frozenCounter>\n");

    tmwtarg_snprintf(buf, sizeof(buf), "    <pointNumber>%d</pointNumber>\n", pointNum);
    result = tmwtarg_appendString(result, buf);

    if(desc && (strlen(desc) > 0))
    {
      tmwsim_xmlFormatDesc(desc, buf, sizeof(buf));
      result = tmwtarg_appendString(result, buf);
    }
    
    result = tmwtarg_appendString(result, "    <flags>\n");
    result = _putBoolField(result, "     ", "online", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_ON_LINE));
    result = _putBoolField(result, "     ", "restart", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_RESTART));
    result = _putBoolField(result, "     ", "commLost", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_COMM_LOST));
    result = _putBoolField(result, "     ", "remoteForced", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_REMOTE_FORCED));
    result = _putBoolField(result, "     ", "localForced", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_LOCAL_FORCED));
    result = tmwtarg_appendString(result, "    </flags>\n");

    tmwtarg_snprintf(buf, sizeof(buf), "     <value>%u</value>\n", value);
    result = tmwtarg_appendString(result, buf);

    result = tmwtarg_appendString(result, "  </frozenCounter>\n");
  }
  result = tmwtarg_appendString(result, "   </frozenCounterGroup>\n");

  result = tmwtarg_appendString(result, "  <analogInputGroup>\n");
  pPoint = TMWDEFS_NULL;
  while((pPoint = tmwsim_tableGetNextPoint(&pDbHandle->analogInputs, pPoint)) != TMWDEFS_NULL)
  {
    TMWTYPES_ULONG pointNum = tmwsim_getPointNumber(pPoint);
    TMWTYPES_CHAR *desc = tmwsim_getDescription(pPoint);
    TMWSIM_DATA_TYPE value = tmwsim_getAnalogValue(pPoint); 
    TMWTYPES_UCHAR flags = tmwsim_getFlags(pPoint);

    result = tmwtarg_appendString(result, "   <analogInput>\n");

    tmwtarg_snprintf(buf, sizeof(buf), "    <pointNumber>%d</pointNumber>\n", pointNum);
    result = tmwtarg_appendString(result, buf);

    if(desc && (strlen(desc) > 0))
    {
      tmwsim_xmlFormatDesc(desc, buf, sizeof(buf));
      result = tmwtarg_appendString(result, buf);
    }
    
    result = tmwtarg_appendString(result, "    <flags>\n");
    result = _putBoolField(result, "     ", "online", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_ON_LINE));
    result = _putBoolField(result, "     ", "restart", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_RESTART));
    result = _putBoolField(result, "     ", "commLost", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_COMM_LOST));
    result = _putBoolField(result, "     ", "remoteForced", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_REMOTE_FORCED));
    result = _putBoolField(result, "     ", "localForced", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_LOCAL_FORCED));
    result = tmwtarg_appendString(result, "    </flags>\n");

    tmwtarg_snprintf(buf, sizeof(buf), "    <value>%g</value>\n", value);
    result = tmwtarg_appendString(result, buf);

    result = tmwtarg_appendString(result, "   </analogInput>\n");
  }
  result = tmwtarg_appendString(result, "  </analogInputGroup>\n");

  result = tmwtarg_appendString(result, "  <analogInputDeadbandGroup>\n");
  pPoint = TMWDEFS_NULL;
  while((pPoint = tmwsim_tableGetNextPoint(&pDbHandle->analogInputs, pPoint)) != TMWDEFS_NULL)
  {
    TMWTYPES_ULONG pointNum = tmwsim_getPointNumber(pPoint);
    TMWTYPES_CHAR *desc = tmwsim_getDescription(pPoint);

    TMWSIM_DATA_TYPE value = tmwsim_getAnalogDeadband(pPoint); 

    result = tmwtarg_appendString(result, "   <analogInputDeadband>\n");

    tmwtarg_snprintf(buf, sizeof(buf), "    <pointNumber>%d</pointNumber>\n", pointNum);
    result = tmwtarg_appendString(result, buf);

    if(desc && (strlen(desc) > 0))
    {
      tmwsim_xmlFormatDesc(desc, buf, sizeof(buf));
      result = tmwtarg_appendString(result, buf);
    }
    
    tmwtarg_snprintf(buf, sizeof(buf), "    <value>%g</value>\n", value);
    result = tmwtarg_appendString(result, buf);

    result = tmwtarg_appendString(result, "   </analogInputDeadband>\n");
  }
  result = tmwtarg_appendString(result, "  </analogInputDeadbandGroup>\n");

  result = tmwtarg_appendString(result, "  <frznAnalogInputGroup>\n");
  pPoint = TMWDEFS_NULL;
  while((pPoint = tmwsim_tableGetNextPoint(&pDbHandle->frozenAnalogInputs, pPoint)) != TMWDEFS_NULL)
  {
    TMWTYPES_ULONG pointNum = tmwsim_getPointNumber(pPoint);
    TMWTYPES_CHAR *desc = tmwsim_getDescription(pPoint);
    TMWSIM_DATA_TYPE value = tmwsim_getAnalogValue(pPoint); 
    TMWTYPES_UCHAR flags = tmwsim_getFlags(pPoint);

    result = tmwtarg_appendString(result, "   <frznAnalogInput>\n");

    tmwtarg_snprintf(buf, sizeof(buf), "    <pointNumber>%d</pointNumber>\n", pointNum);
    result = tmwtarg_appendString(result, buf);

    if(desc && (strlen(desc) > 0))
    {
      tmwsim_xmlFormatDesc(desc, buf, sizeof(buf));
      result = tmwtarg_appendString(result, buf);
    }
    
    result = tmwtarg_appendString(result, "    <flags>\n");
    result = _putBoolField(result, "     ", "online", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_ON_LINE));
    result = _putBoolField(result, "     ", "restart", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_RESTART));
    result = _putBoolField(result, "     ", "commLost", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_COMM_LOST));
    result = _putBoolField(result, "     ", "remoteForced", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_REMOTE_FORCED));
    result = _putBoolField(result, "     ", "localForced", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_LOCAL_FORCED));
    result = tmwtarg_appendString(result, "    </flags>\n");

    tmwtarg_snprintf(buf, sizeof(buf), "    <value>%g</value>\n", value);
    result = tmwtarg_appendString(result, buf);

    result = tmwtarg_appendString(result, "   </frznAnalogInput>\n");
  }
  result = tmwtarg_appendString(result, "  </frznAnalogInputGroup>\n");

  result = tmwtarg_appendString(result, "  <analogOutputControlGroup>\n");
  pPoint = TMWDEFS_NULL;
  while((pPoint = tmwsim_tableGetNextPoint(&pDbHandle->analogOutputs, pPoint)) != TMWDEFS_NULL)
  {
    TMWTYPES_ULONG pointNum = tmwsim_getPointNumber(pPoint);
    TMWTYPES_CHAR *desc = tmwsim_getDescription(pPoint);

    result = tmwtarg_appendString(result, "   <analogOutputControl>\n");

    tmwtarg_snprintf(buf, sizeof(buf), "    <pointNumber>%d</pointNumber>\n", pointNum);
    result = tmwtarg_appendString(result, buf);

    if(desc && (strlen(desc) > 0))
    {
      tmwsim_xmlFormatDesc(desc, buf, sizeof(buf));
      result = tmwtarg_appendString(result, buf);
    }
    
    result = tmwtarg_appendString(result, "   </analogOutputControl>\n");
  }
  result = tmwtarg_appendString(result, "  </analogOutputControlGroup>\n");

  result = tmwtarg_appendString(result, "  <analogOutputStatusGroup>\n");
  pPoint = TMWDEFS_NULL;
  while((pPoint = tmwsim_tableGetNextPoint(&pDbHandle->analogOutputs, pPoint)) != TMWDEFS_NULL)
  {
    TMWTYPES_ULONG pointNum = tmwsim_getPointNumber(pPoint);
    TMWTYPES_CHAR *desc = tmwsim_getDescription(pPoint);

    TMWSIM_DATA_TYPE value = tmwsim_getAnalogValue(pPoint); 
    TMWTYPES_UCHAR flags = tmwsim_getFlags(pPoint);

    result = tmwtarg_appendString(result, "   <analogOutputStatus>\n");

    tmwtarg_snprintf(buf, sizeof(buf), "    <pointNumber>%d</pointNumber>\n", pointNum);
    result = tmwtarg_appendString(result, buf);

    if(desc && (strlen(desc) > 0))
    {
      tmwsim_xmlFormatDesc(desc, buf, sizeof(buf));
      result = tmwtarg_appendString(result, buf);
    }
    
    result = tmwtarg_appendString(result, "    <flags>\n");
    result = _putBoolField(result, "     ", "online", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_ON_LINE));
    result = _putBoolField(result, "     ", "restart", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_RESTART));
    result = _putBoolField(result, "     ", "commLost", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_COMM_LOST));
    result = _putBoolField(result, "     ", "remoteForced", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_REMOTE_FORCED));
    result = _putBoolField(result, "     ", "localForced", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_LOCAL_FORCED));
    result = tmwtarg_appendString(result, "    </flags>\n");

    tmwtarg_snprintf(buf, sizeof(buf), "    <value>%g</value>\n", value);
    result = tmwtarg_appendString(result, buf);

    result = tmwtarg_appendString(result, "   </analogOutputStatus>\n");
  }
  result = tmwtarg_appendString(result, "  </analogOutputStatusGroup>\n");
 
  result = tmwtarg_appendString(result, "  <indexedTimeGroup>\n");
  pPoint = TMWDEFS_NULL;
  while ((pPoint = tmwsim_tableGetNextPoint(&pDbHandle->indexedTime, pPoint)) != TMWDEFS_NULL)
  {
    TMWTYPES_ULONG pointNum = tmwsim_getPointNumber(pPoint);
    TMWDTIME startTime;
    tmwsim_getIndexedTimeTime(pPoint, &startTime);
    TMWTYPES_ULONG intervalCount = tmwsim_getIndexedTimeIntervalCount(pPoint);
    TMWTYPES_BYTE intervalUnits = tmwsim_getIndexedTimeIntervalUnit(pPoint);

    result = tmwtarg_appendString(result, "   <indexedTime>\n");

    tmwtarg_snprintf(buf, sizeof(buf), "    <pointNumber>%d</pointNumber>\n", pointNum);
    result = tmwtarg_appendString(result, buf);

    /* startTime  CCYY-MM-DDThh:mm:ss[Z|(+|-)hh:mm]    2020-05-30T09:30:10.5 */
    tmwtarg_snprintf(buf, sizeof(buf), "    <startTime>%4d-%02d-%02dT%02d:%02d:%02d.%d</startTime>\n",
      startTime.year, startTime.month, startTime.dayOfMonth, startTime.hour, startTime.minutes,
      startTime.mSecsAndSecs / 1000, startTime.mSecsAndSecs % 1000);
    result = tmwtarg_appendString(result, buf);

    tmwtarg_snprintf(buf, sizeof(buf), "    <intervalCount>%d</intervalCount>\n", intervalCount);
    result = tmwtarg_appendString(result, buf);

    tmwtarg_snprintf(buf, sizeof(buf), "    <intervalUnits>%d</intervalUnits>\n", intervalUnits);
    result = tmwtarg_appendString(result, buf);

    result = tmwtarg_appendString(result, "   </indexedTime>\n");
  }
  result = tmwtarg_appendString(result, "  </indexedTimeGroup>\n");

#if MDNPDATA_SUPPORT_DATASETS
  result = tmwtarg_appendString(result, "  <dataSetProtoGroup>\n");
  pPoint = TMWDEFS_NULL;
  while((pPoint = (TMWSIM_POINT*)tmwdlist_getAfter(&pDbHandle->datasetProtos, 
      (TMWDLIST_MEMBER *)pPoint)) != TMWDEFS_NULL)
  {
    int i;
    int len; 
    TMWTYPES_UCHAR name[256];
    MDNPSIM_DATASET_PROTO *pProto = (MDNPSIM_DATASET_PROTO *)pPoint;
    DNPDATA_DATASET_DESCR_ELEM *pDescrElems = pProto->contents;
    TMWTYPES_CHAR *desc = tmwsim_getDescription(pPoint);

    result = tmwtarg_appendString(result, "   <dataSetProtoData>\n");

    tmwtarg_snprintf(buf, sizeof(buf), "    <pointNumber>%d</pointNumber>\n", pProto->protoId);
    result = tmwtarg_appendString(result, buf);  
    
    if(desc && (strlen(desc) > 0))
    {
      tmwsim_xmlFormatDesc(desc, buf, sizeof(buf));
      result = tmwtarg_appendString(result, buf);
    }

    len = 0;
    for(i=0; i< 16; i++)
      len += tmwtarg_snprintf((char *)name+len, sizeof(name)-len, "%02x ", pProto->uuid[i]);
    
    tmwtarg_snprintf(buf, sizeof(buf), "    <uuid>%s</uuid>\n", name);
    result = tmwtarg_appendString(result, buf);

   /* if(desc && (strlen(desc) > 0))
    {
      tmwsim_xmlFormatDesc(desc, buf, sizeof(buf));
      result = tmwtarg_appendString(result, buf);
    }*/
    for(i=0; i<pProto->numberElems; i++ )
    {
      result = tmwtarg_appendString(result, "    <dataSetDescrElement>\n");

      tmwtarg_snprintf(buf, sizeof(buf), "     <descrElemType>%s</descrElemType>\n", _toDescrType(pDescrElems[i].descrElemType));
      result = tmwtarg_appendString(result, buf);

      tmwtarg_snprintf(buf, sizeof(buf), "     <dataType>%s</dataType>\n", _toDataType(pDescrElems[i].dataTypeCode));
      result = tmwtarg_appendString(result, buf);

      tmwtarg_snprintf(buf, sizeof(buf), "     <maxDataLength>%d</maxDataLength>\n", pDescrElems[i].maxDataLength);
      result = tmwtarg_appendString(result, buf);
      
      _toDatasetOutData(&pDescrElems[i].ancillaryValue, TMWDEFS_FALSE, name, 256);
      tmwtarg_snprintf(buf, sizeof(buf), "     <value>%s</value>\n", name);

      result = tmwtarg_appendString(result, buf);

      result = tmwtarg_appendString(result, "    </dataSetDescrElement>\n"); 
    }

    result = tmwtarg_appendString(result, "   </dataSetProtoData>\n");
  }
  result = tmwtarg_appendString(result, "  </dataSetProtoGroup>\n");

  result = tmwtarg_appendString(result, "  <dataSetDescrGroup>\n");
  pPoint = TMWDEFS_NULL;
  while((pPoint = (TMWSIM_POINT*)tmwdlist_getAfter(&pDbHandle->datasetDescrDatas, 
      (TMWDLIST_MEMBER *)pPoint)) != TMWDEFS_NULL)
  {
    int i;
    TMWTYPES_UCHAR name[256];
    MDNPSIM_DATASET_DESCR_DATA *pDescr = (MDNPSIM_DATASET_DESCR_DATA *)pPoint;
    DNPDATA_DATASET_DESCR_ELEM *pDescrElems = pDescr->descrContents;
    DNPDATA_DATASET_DESCR_INDEX *pIndexElems = pDescr->descrIndex; 
    TMWTYPES_CHAR *desc = tmwsim_getDescription(pPoint);

    result = tmwtarg_appendString(result, "   <dataSetDescrData>\n");

    tmwtarg_snprintf(buf, sizeof(buf), "    <pointNumber>%d</pointNumber>\n", pDescr->datasetId);
    result = tmwtarg_appendString(result, buf);
      
    if(desc && (strlen(desc) > 0))
    {
      tmwsim_xmlFormatDesc(desc, buf, sizeof(buf));
      result = tmwtarg_appendString(result, buf);
    }
 
    tmwtarg_snprintf(buf, sizeof(buf), "    <characteristics>%d</characteristics>\n", pDescr->characteristics);
    result = tmwtarg_appendString(result, buf);

    /* Descriptor elements */
    for(i=0; i<pDescr->numberDescrElems; i++ )
    {
      result = tmwtarg_appendString(result, "    <dataSetDescrElement>\n");

      tmwtarg_snprintf(buf, sizeof(buf), "     <descrElemType>%s</descrElemType>\n", _toDescrType(pDescrElems[i].descrElemType));
      result = tmwtarg_appendString(result, buf);

      tmwtarg_snprintf(buf, sizeof(buf), "     <dataType>%s</dataType>\n", _toDataType(pDescrElems[i].dataTypeCode));
      result = tmwtarg_appendString(result, buf);

      tmwtarg_snprintf(buf, sizeof(buf), "     <maxDataLength>%d</maxDataLength>\n", pDescrElems[i].maxDataLength);
      result = tmwtarg_appendString(result, buf);
       
      if(pDescrElems[i].descrElemType == DNPDEFS_DATASET_DESCR_UUID
        || pDescrElems[i].descrElemType == DNPDEFS_DATASET_DESCR_PTYP)
      {
        _toDatasetOutData(&pDescrElems[i].ancillaryValue, TMWDEFS_TRUE, name, 256);
        tmwtarg_snprintf(buf, sizeof(buf), "     <hexValue>%s</hexValue>\n", name);
      }
      else
      {
        _toDatasetOutData(&pDescrElems[i].ancillaryValue, TMWDEFS_FALSE, name, 256);
        tmwtarg_snprintf(buf, sizeof(buf), "     <value>%s</value>\n", name); 
      }

      result = tmwtarg_appendString(result, buf);

      result = tmwtarg_appendString(result, "    </dataSetDescrElement>\n"); 

    }

    /* Now the Point Index Attribute Elements */
    for(i=0; i<pDescr->numberIndexElems; i++ )
    {
      result = tmwtarg_appendString(result, "    <dataSetPointIndexAttribute>\n");

      tmwtarg_snprintf(buf, sizeof(buf), "     <pointType>%d</pointType>\n", pIndexElems[i].pointType);
      result = tmwtarg_appendString(result, buf);

      tmwtarg_snprintf(buf, sizeof(buf), "     <pointIndex>%d</pointIndex>\n", pIndexElems[i].pointIndex);
      result = tmwtarg_appendString(result, buf);
 
      result = tmwtarg_appendString(result, "    </dataSetPointIndexAttribute>\n"); 
    }

    result = tmwtarg_appendString(result, "   </dataSetDescrData>\n");
  }
  result = tmwtarg_appendString(result, "  </dataSetDescrGroup>\n");


  result = tmwtarg_appendString(result, "  <dataSetDataGroup>\n");
  pPoint = TMWDEFS_NULL;
  while((pPoint = (TMWSIM_POINT*)tmwdlist_getAfter(&pDbHandle->datasetDescrDatas, 
      (TMWDLIST_MEMBER *)pPoint)) != TMWDEFS_NULL)
  {
    int i;
    TMWTYPES_UCHAR dataBuf[256]; 
    MDNPSIM_DATASET_DESCR_DATA *pDescr = (MDNPSIM_DATASET_DESCR_DATA *)pPoint;
    DNPDATA_DATASET_VALUE *pDataElems = pDescr->dataElem;

    result = tmwtarg_appendString(result, "   <dataSetData>\n");

    tmwtarg_snprintf(buf, sizeof(buf), "    <pointNumber>%d</pointNumber>\n", pDescr->datasetId);
    result = tmwtarg_appendString(result, buf);

    for(i=0; i<pDescr->numberDataElems; i++ )
    {
      result = tmwtarg_appendString(result, "    <dataSetElement>\n");

      tmwtarg_snprintf(buf, sizeof(buf), "     <length>%d</length>\n", pDataElems[i].length);
      result = tmwtarg_appendString(result, buf);
  
      if(!_isDatasetElemPrintable(&pDataElems[i]))
      {
        _toDatasetOutData(&pDataElems[i], TMWDEFS_TRUE, dataBuf, 256);
        tmwtarg_snprintf(buf, sizeof(buf), "     <hexValue>%s</hexValue>\n", dataBuf);
      } 
      else
      {
        _toDatasetOutData(&pDataElems[i], TMWDEFS_FALSE, dataBuf, 256);
        tmwtarg_snprintf(buf, sizeof(buf), "     <value>%s</value>\n", dataBuf); 
      }
      result = tmwtarg_appendString(result, buf);

      result = tmwtarg_appendString(result, "    </dataSetElement>\n"); 
    }

    result = tmwtarg_appendString(result, "   </dataSetData>\n");
  }
  result = tmwtarg_appendString(result, "  </dataSetDataGroup>\n");
#endif

  result = tmwtarg_appendString(result, "  <stringGroup>\n");
  pPoint = TMWDEFS_NULL;
  while ((pPoint = tmwsim_tableGetNextPoint(&pDbHandle->stringData, pPoint)) != TMWDEFS_NULL)
  {
    TMWTYPES_ULONG pointNum = tmwsim_getPointNumber(pPoint);
    TMWTYPES_CHAR *desc = tmwsim_getDescription(pPoint);

    result = tmwtarg_appendString(result, "   <stringData>\n");

    tmwtarg_snprintf(buf, sizeof(buf), "    <pointNumber>%d</pointNumber>\n", pointNum);
    result = tmwtarg_appendString(result, buf);

    if (desc && (strlen(desc) > 0))
    {
      tmwsim_xmlFormatDesc(desc, buf, sizeof(buf));
      result = tmwtarg_appendString(result, buf);
    }
#if TMWCNFG_USE_DYNAMIC_MEMORY
    {
      /* Saving database in proprietary format should only occur in TH or .NET with Dynamic memory
       * Don't allocate these large buffers on the stack.
       * (tmwtarg_appendString() uses DYNAMIC MEMORY anyway!)
       */
      TMWTYPES_UCHAR *pBuf;
      TMWTYPES_UCHAR *pStrBuf;
      TMWTYPES_USHORT strLength;
      /* Max string length + XML tags */
      pBuf = (TMWTYPES_UCHAR *)tmwtarg_alloc(DNPCNFG_MAX_EXT_STRING_LENGTH + 30);
      if (pBuf != TMWDEFS_NULL)
      {
        pStrBuf = (TMWTYPES_UCHAR *)tmwtarg_alloc(DNPCNFG_MAX_EXT_STRING_LENGTH + 1);
        if (pStrBuf != TMWDEFS_NULL)
        {
          tmwsim_getExtStringValue(pPoint, DNPCNFG_MAX_EXT_STRING_LENGTH, pStrBuf, &strLength);

          pStrBuf[strLength] = '\0';
          tmwsim_xmlFormatValue((TMWTYPES_CHAR *)pStrBuf, strLength, (TMWTYPES_CHAR *)pBuf, DNPCNFG_MAX_EXT_STRING_LENGTH);
          result = tmwtarg_appendString(result, (TMWTYPES_CHAR *)pBuf);
          tmwtarg_free(pStrBuf);
        }
        else
        {
          tmwtarg_free(pBuf);
          return(TMWDEFS_NULL);
        }
        tmwtarg_free(pBuf);
      }
      else
      {
        return(TMWDEFS_NULL);
      }
    }
#endif

    result = _putBoolField(result, "     ", "extended", pPoint->data.string.extString);

    result = tmwtarg_appendString(result, "   </stringData>\n");
  }
  result = tmwtarg_appendString(result, "  </stringGroup>\n");

  result = tmwtarg_appendString(result, "  <virtualTerminalGroup>\n");
  pPoint = TMWDEFS_NULL;
  while((pPoint = tmwsim_tableGetNextPoint(&pDbHandle->vtermEvents, pPoint)) != TMWDEFS_NULL)
  {
    TMWTYPES_ULONG pointNum = tmwsim_getPointNumber(pPoint);
    TMWTYPES_CHAR *desc = tmwsim_getDescription(pPoint);
    TMWTYPES_UCHAR strBuf[DNPDEFS_MAX_STRING_LENGTH+1];
    TMWTYPES_UCHAR strLength;

    tmwsim_getStringValue(pPoint, DNPDEFS_MAX_STRING_LENGTH, strBuf, &strLength);

    result = tmwtarg_appendString(result, "   <virtualTerminal>\n");

    tmwtarg_snprintf(buf, sizeof(buf), "    <pointNumber>%d</pointNumber>\n", pointNum);
    result = tmwtarg_appendString(result, buf);

    if(desc && (strlen(desc) > 0))
    {
      tmwsim_xmlFormatDesc(desc, buf, sizeof(buf));
      result = tmwtarg_appendString(result, buf);
    }
    
    strBuf[strLength] = '\0';
    tmwsim_xmlFormatValue((TMWTYPES_CHAR *)strBuf, strLength, buf, sizeof(buf));
    result = tmwtarg_appendString(result, buf);

    result = tmwtarg_appendString(result, "   </virtualTerminal>\n");
  }
  result = tmwtarg_appendString(result, "  </virtualTerminalGroup>\n");

  result = tmwtarg_appendString(result, "  <authSecStatGroup>\n");
  pPoint = TMWDEFS_NULL;
  while((pPoint = tmwsim_tableGetNextPoint(&pDbHandle->authSecStatsFromMaster, pPoint)) != TMWDEFS_NULL)
  {
    TMWTYPES_ULONG pointNum = tmwsim_getPointNumber(pPoint);
    TMWTYPES_CHAR *desc = tmwsim_getDescription(pPoint);
    TMWTYPES_ULONG value = tmwsim_getCounterValue(pPoint);
    TMWTYPES_UCHAR flags = tmwsim_getFlags(pPoint);

    result = tmwtarg_appendString(result, "   <authSecStat>\n");

    tmwtarg_snprintf(buf, sizeof(buf), "    <pointNumber>%d</pointNumber>\n", pointNum);
    result = tmwtarg_appendString(result, buf);

    result = tmwtarg_appendString(result, "    <fromOS>false</fromOS>\n");

    if(desc && (strlen(desc) > 0))
    {
      tmwsim_xmlFormatDesc(desc, buf, sizeof(buf));
      result = tmwtarg_appendString(result, buf);
    }
    
    result = tmwtarg_appendString(result, "    <flags>\n");
    result = _putBoolField(result, "     ", "online", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_ON_LINE));
    result = _putBoolField(result, "     ", "restart", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_RESTART));
    result = _putBoolField(result, "     ", "commLost", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_COMM_LOST));
    result = _putBoolField(result, "     ", "remoteForced", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_REMOTE_FORCED));
    result = _putBoolField(result, "     ", "localForced", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_LOCAL_FORCED));
    result = tmwtarg_appendString(result, "    </flags>\n");

    tmwtarg_snprintf(buf, sizeof(buf), "     <value>%u</value>\n", value);
    result = tmwtarg_appendString(result, buf);

    result = tmwtarg_appendString(result, "  </authSecStat>\n");
  } 

  pPoint = TMWDEFS_NULL;
  while((pPoint = tmwsim_tableGetNextPoint(&pDbHandle->authSecStatsFromOS, pPoint)) != TMWDEFS_NULL)
  {
    TMWTYPES_ULONG pointNum = tmwsim_getPointNumber(pPoint);
    TMWTYPES_CHAR *desc = tmwsim_getDescription(pPoint);
    TMWTYPES_ULONG value = tmwsim_getCounterValue(pPoint);
    TMWTYPES_UCHAR flags = tmwsim_getFlags(pPoint);

    result = tmwtarg_appendString(result, "   <authSecStat>\n");

    tmwtarg_snprintf(buf, sizeof(buf), "    <pointNumber>%d</pointNumber>\n", pointNum);
    result = tmwtarg_appendString(result, buf);

    result = tmwtarg_appendString(result, "    <fromOS>true</fromOS>\n");

    if(desc && (strlen(desc) > 0))
    {
      tmwsim_xmlFormatDesc(desc, buf, sizeof(buf));
      result = tmwtarg_appendString(result, buf);
    }
    
    result = tmwtarg_appendString(result, "    <flags>\n");
    result = _putBoolField(result, "     ", "online", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_ON_LINE));
    result = _putBoolField(result, "     ", "restart", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_RESTART));
    result = _putBoolField(result, "     ", "commLost", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_COMM_LOST));
    result = _putBoolField(result, "     ", "remoteForced", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_REMOTE_FORCED));
    result = _putBoolField(result, "     ", "localForced", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_LOCAL_FORCED));
    result = tmwtarg_appendString(result, "    </flags>\n");

    tmwtarg_snprintf(buf, sizeof(buf), "     <value>%u</value>\n", value);
    result = tmwtarg_appendString(result, buf);

    result = tmwtarg_appendString(result, "  </authSecStat>\n");
  }
  result = tmwtarg_appendString(result, "   </authSecStatGroup>\n");

  result = tmwtarg_appendString(result, " </device>\n");
  result = tmwtarg_appendString(result, "</tmw:dnpdata>\n");
  return(result);
}

#endif /* MDNPSIM_SUPPORT_XML */
#endif /* TMWCNFG_SUPPORT_SIMULATED_DB */
