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

/* file: sdnpsim.c
 * description: Simulates a DNP slave database.
 */
#include "tmwscl/dnp/dnpdiag.h"
#include "tmwscl/utils/tmwcnfg.h"
#if TMWCNFG_USE_SIMULATED_DB

#include "tmwscl/dnp/sdnpsim.h"
#include "tmwscl/dnp/sdnpfsim.h"
#include "tmwscl/dnp/sdnpmem.h"
#include "tmwscl/dnp/sdnpo120.h"
#include "tmwscl/dnp/sdnputil.h"
#include "tmwscl/dnp/dnpdtime.h"
#include "tmwscl/dnp/sdnpo011.h"
#include "tmwscl/dnp/sdnpo023.h"
#include "tmwscl/dnp/sdnpo033.h"
#include "tmwscl/dnp/sdnpo042.h"
#include "tmwscl/utils/tmwsim.h"
#include "tmwscl/utils/tmwtarg.h"
#include "tmwscl/utils/tmwvrsn.h"


#if SDNPDATA_SUPPORT_OBJ0
static void _createDeviceAttributes(SDNPSIM_DATABASE *pDbHandle);
#endif

#if SDNPDATA_SUPPORT_DATASETS
static void _initDataSets(SDNPSIM_DATABASE *pDbHandle);
static void _clearDataSets(SDNPSIM_DATABASE *pDbHandle);
#endif

#if TMWCNFG_SUPPORT_DIAG && SDNPDATA_SUPPORT_DATASETS
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
#endif

/* Call update callback if set */
static void _callCallback(
  TMWSIM_POINT *pPoint,
  TMWSIM_EVENT_TYPE type,
  DNPDEFS_OBJ_GROUP_ID objectGroup,
  TMWTYPES_USHORT pointNumber)
{
  SDNPSIM_DATABASE *pDbHandle;
  
  pDbHandle = (SDNPSIM_DATABASE *)pPoint->pDbHandle;
  if(pDbHandle->pUpdateCallback != TMWDEFS_NULL)
  {
    pDbHandle->pUpdateCallback(pDbHandle->pUpdateCallbackParam, type, objectGroup, pointNumber);
  }
}

/* Call update callback when point is removed, if set */
static void _callRemoveCallback(
  SDNPSIM_DATABASE *pDbHandle,
  TMWSIM_EVENT_TYPE type,
  DNPDEFS_OBJ_GROUP_ID objectGroup,
  TMWTYPES_USHORT pointNumber)
{ 
  if(pDbHandle != TMWDEFS_NULL && pDbHandle->pUpdateCallback != TMWDEFS_NULL)
  {
    pDbHandle->pUpdateCallback(pDbHandle->pUpdateCallbackParam, type, objectGroup, pointNumber);
  }
}

#if SDNPDATA_SUPPORT_DATASETS
static void TMWDEFS_LOCAL _createDatasetDeviceAttributes(SDNPSIM_DATABASE *pDbHandle)
{
#if SDNPDATA_SUPPORT_OBJ0
  TMWSIM_POINT *pPoint;
  DNPDATA_ATTRIBUTE_VALUE data; 

  data.type = DNPDEFS_ATTRIBUTE_TYPE_UINT;
  data.length = 1;
  data.value.uintValue = 1;   
  pPoint = (TMWSIM_POINT *)sdnpsim_addDeviceAttribute(pDbHandle, 0, DNPDEFS_OBJ0_NUM_MASTER_PROTOS, 0, &data);
  if(pPoint != TMWDEFS_NULL)
    tmwsim_setDescription(pPoint, "Number of Master-Defined Data Set Prototypes"); 

  data.type = DNPDEFS_ATTRIBUTE_TYPE_UINT;
  data.length = 1;
  data.value.uintValue = 1;   
  pPoint = (TMWSIM_POINT *)sdnpsim_addDeviceAttribute(pDbHandle, 0, DNPDEFS_OBJ0_NUM_OUTSTA_PROTOS, 0, &data);
  if(pPoint != TMWDEFS_NULL)
    tmwsim_setDescription(pPoint, "Number of Outstation-Defined Data Set Prototypes"); 

  data.type = DNPDEFS_ATTRIBUTE_TYPE_UINT;
  data.length = 1;
  data.value.uintValue = 1;  
  pPoint = (TMWSIM_POINT *)sdnpsim_addDeviceAttribute(pDbHandle, 0, DNPDEFS_OBJ0_NUM_MASTER_DATASET, 0, &data);
  if(pPoint != TMWDEFS_NULL)
    tmwsim_setDescription(pPoint, "Number of Master-Defined Data Sets");

  data.type = DNPDEFS_ATTRIBUTE_TYPE_UINT;
  data.length = 1;
  data.value.uintValue = 1;
  pPoint = (TMWSIM_POINT *)sdnpsim_addDeviceAttribute(pDbHandle, 0, DNPDEFS_OBJ0_NUM_OUTSTA_DATASET, 0, &data);
  if(pPoint != TMWDEFS_NULL)
    tmwsim_setDescription(pPoint, "Number of Outstation-Defined Data Sets"); 
#else
  TMWTARG_UNUSED_PARAM(pDbHandle);
#endif
}
#endif


/* Delete all of the security statistics from the database.
 * This is done separately from the rest of the database, because these MUST exist if SA is enabled
 */
static void TMWDEFS_LOCAL _clearSecStatsDb(
  void *pHandle)
{
#if SDNPDATA_SUPPORT_OBJ120
  void *pPoint;
  SDNPSIM_DATABASE *pDbHandle;
  TMWTYPES_ULONG quantity;
  TMWTYPES_USHORT i;

  pDbHandle = (SDNPSIM_DATABASE *)pHandle;

  quantity = tmwsim_tableSize(&pDbHandle->authSecStats);
  for (i = 0; i < quantity; i++)
  {
    if ((pPoint = sdnpsim_authSecStatGetPoint(pDbHandle, i)) != TMWDEFS_NULL)
    {
      sdnpsim_deleteAuthSecStat(pDbHandle, i);
    }
  }
#else
  TMWTARG_UNUSED_PARAM(pHandle);
#endif
}

static void TMWDEFS_LOCAL _cancelFreezeTimer(
  TMWSIM_TABLE_HEAD *pTableHead,
  int pointIndex)
{
  TMWSIM_POINT *pPoint = tmwsim_tableFindPointByIndex(pTableHead, (TMWTYPES_USHORT)pointIndex);
  if ((pPoint) && (pPoint->freezeEnabled))
  {
    tmwtimer_cancel(&pPoint->freezeTimer);
  }
}


/* Delete all points in the database */
static void TMWDEFS_LOCAL _clearDb(
  void *pHandle)
{
  int i;
  TMWSIM_POINT *pPoint;
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;

  _callRemoveCallback(pDbHandle, TMWSIM_CLEAR_DATABASE, 0, 0);

  for (i = tmwsim_tableSize(&pDbHandle->binaryInputs) - 1; i >= 0; i--)
    _callRemoveCallback(pDbHandle, TMWSIM_POINT_DELETE, DNPDEFS_OBJ_1_BIN_INPUTS, (TMWTYPES_USHORT)i);
  tmwsim_tableDestroy(&pDbHandle->binaryInputs);

  for (i = tmwsim_tableSize(&pDbHandle->doubleInputs) - 1; i >= 0; i--)
    _callRemoveCallback(pDbHandle, TMWSIM_POINT_DELETE, DNPDEFS_OBJ_3_DBL_INPUTS, (TMWTYPES_USHORT)i);
  tmwsim_tableDestroy(&pDbHandle->doubleInputs);

  for (i = tmwsim_tableSize(&pDbHandle->binaryOutputs) - 1; i >= 0; i--)
    _callRemoveCallback(pDbHandle, TMWSIM_POINT_DELETE, DNPDEFS_OBJ_10_BIN_OUT_STATUSES, (TMWTYPES_USHORT)i);
  tmwsim_tableDestroy(&pDbHandle->binaryOutputs);

  for (i = tmwsim_tableSize(&pDbHandle->binaryCounters) - 1; i >= 0; i--)
  {
    _cancelFreezeTimer(&pDbHandle->binaryCounters, i);
    _callRemoveCallback(pDbHandle, TMWSIM_POINT_DELETE, DNPDEFS_OBJ_20_RUNNING_CNTRS, (TMWTYPES_USHORT)i);
  }
  tmwsim_tableDestroy(&pDbHandle->binaryCounters);

  for (i = tmwsim_tableSize(&pDbHandle->frozenCounters) - 1; i >= 0; i--)
    _callRemoveCallback(pDbHandle, TMWSIM_POINT_DELETE, DNPDEFS_OBJ_21_FROZEN_CNTRS, (TMWTYPES_USHORT)i);
  tmwsim_tableDestroy(&pDbHandle->frozenCounters);

  for (i = tmwsim_tableSize(&pDbHandle->analogInputs) - 1; i >= 0; i--)
  {
    _cancelFreezeTimer(&pDbHandle->analogInputs, i);
    _callRemoveCallback(pDbHandle, TMWSIM_POINT_DELETE, DNPDEFS_OBJ_30_ANA_INPUTS, (TMWTYPES_USHORT)i);
  }
  tmwsim_tableDestroy(&pDbHandle->analogInputs);

  for (i = tmwsim_tableSize(&pDbHandle->frozenAnalogInputs) - 1; i >= 0; i--)
    _callRemoveCallback(pDbHandle, TMWSIM_POINT_DELETE, DNPDEFS_OBJ_31_FRZN_ANA_INPUTS, (TMWTYPES_USHORT)i);
  tmwsim_tableDestroy(&pDbHandle->frozenAnalogInputs);

  for (i = tmwsim_tableSize(&pDbHandle->analogOutputs) - 1; i >= 0; i--)
    _callRemoveCallback(pDbHandle, TMWSIM_POINT_DELETE, DNPDEFS_OBJ_40_ANA_OUT_STATUSES, (TMWTYPES_USHORT)i);
  tmwsim_tableDestroy(&pDbHandle->analogOutputs);

  for (i = tmwsim_tableSize(&pDbHandle->indexedTime) - 1; i >= 0; i--)
    _callRemoveCallback(pDbHandle, TMWSIM_POINT_DELETE, DNPDEFS_OBJ_50_TIME_AND_DATE, (TMWTYPES_USHORT)i);
  tmwsim_tableDestroy(&pDbHandle->indexedTime);

  for (i = tmwsim_tableSize(&pDbHandle->octetStrings) - 1; i >= 0; i--)
  {
    TMWSIM_POINT *pDataPoint = tmwsim_tableFindPoint(&pDbHandle->octetStrings, i);
    if (pDataPoint->data.string.extString)
    {
      _callRemoveCallback(pDbHandle, TMWSIM_POINT_DELETE, DNPDEFS_OBJ_114_EXT_STR_DATA, (TMWTYPES_USHORT)i);
    }
    else
    {
      _callRemoveCallback(pDbHandle, TMWSIM_POINT_DELETE, DNPDEFS_OBJ_110_STRING_DATA, (TMWTYPES_USHORT)i);
    }
  }
  tmwsim_tableDestroy(&pDbHandle->octetStrings);

  for (i = tmwsim_tableSize(&pDbHandle->virtualTerminals) - 1; i >= 0; i--)
    _callRemoveCallback(pDbHandle, TMWSIM_POINT_DELETE, DNPDEFS_OBJ_112_VTERM_OUTPUT, (TMWTYPES_USHORT)i);
  tmwsim_tableDestroy(&pDbHandle->virtualTerminals);

  for (i = tmwsim_tableSize(&pDbHandle->deviceAttrs) - 1; i >= 0; i--)
  {
    if ((pPoint = (TMWSIM_POINT *)tmwsim_tableFindPointByIndex(&pDbHandle->deviceAttrs, (TMWTYPES_USHORT)i)) != TMWDEFS_NULL)
    {
      int j;
      for (j = tmwsim_tableSize(&pPoint->data.list.listHead) - 1; j >= 0; j--)
      {
        TMWSIM_POINT *pVariation;
        if ((pVariation = (TMWSIM_POINT *)tmwsim_tableFindPointByIndex(&pPoint->data.list.listHead, (TMWTYPES_USHORT)j)) != TMWDEFS_NULL)
        {
          _callRemoveCallback(pDbHandle, TMWSIM_POINT_DELETE, DNPDEFS_OBJ_0_DEVICE_ATTRIBUTES,
            ((TMWTYPES_USHORT)(((TMWTYPES_USHORT)tmwsim_getPointNumber(pPoint) << 8) | ((TMWTYPES_UCHAR)tmwsim_getPointNumber(pVariation) & 0xff))));
        }
      }
      tmwsim_tableDestroy(&pPoint->data.list.listHead);
    }
  }
  tmwsim_tableDestroy(&pDbHandle->deviceAttrs);

  tmwdlist_destroy(&pDbHandle->authUsers, sdnpmem_free);

  /* empty user managed event queues */
  tmwdlist_destroy(&pDbHandle->binInEvents, sdnpmem_free);
  tmwdlist_destroy(&pDbHandle->binOutEvents, sdnpmem_free);
  tmwdlist_destroy(&pDbHandle->binCmdEvents, sdnpmem_free);
  tmwdlist_destroy(&pDbHandle->binCntrEvents, sdnpmem_free);
  tmwdlist_destroy(&pDbHandle->frznCntrEvents, sdnpmem_free);
  tmwdlist_destroy(&pDbHandle->anlgInEvents, sdnpmem_free);
  tmwdlist_destroy(&pDbHandle->frznAnlgInEvents, sdnpmem_free);
  tmwdlist_destroy(&pDbHandle->anlgOutEvents, sdnpmem_free);
  tmwdlist_destroy(&pDbHandle->anlgCmdEvents, sdnpmem_free);
  tmwdlist_destroy(&pDbHandle->datasetEvents, sdnpmem_free);
  tmwdlist_destroy(&pDbHandle->stringEvents, sdnpmem_free);
  tmwdlist_destroy(&pDbHandle->vtermEvents, sdnpmem_free);
  tmwdlist_destroy(&pDbHandle->extendedStringEvents, sdnpmem_free);
  tmwdlist_destroy(&pDbHandle->authErrorEvents, sdnpmem_free);
  tmwdlist_destroy(&pDbHandle->authStatEvents, sdnpmem_free);

#if SDNPDATA_SUPPORT_OBJ70
  sdnpfsim_clearDb(&(pDbHandle->sdnpFsim));
#endif

#if SDNPDATA_SUPPORT_OBJ86
  _clearDataSets(pDbHandle);
#endif
}

/* Initialize the SA Security Statistics table */
static void TMWDEFS_LOCAL _initSecStatsDb(
  SDNPSIM_DATABASE *pDbHandle)
{
  tmwsim_tableCreate(&pDbHandle->authSecStats);
}

static void TMWDEFS_LOCAL _deleteSecStatsDb(
  void *pHandle)
{
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;
  tmwsim_tableDestroy(&pDbHandle->authSecStats);
}

/* Build the SA Security Statistics database */
static void TMWDEFS_LOCAL _buildSecStatsDb(
  void *pHandle)
{
  sdnpsim_addSecStats(pHandle);
}

#define LONG_TEST_STRING "Extended string that has more than 255 characters abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ 0123456789 abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ 0123456789 abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ 0123456789 abcdef-255-123456789012345678901234567890End"
/* Build default database */
static void TMWDEFS_LOCAL _buildDb(
  void *pHandle)
{
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;
  TMWTYPES_USHORT i;
  (void)i; // adding this to get rid of the compiler error about an unused variable
#if SDNPDATA_SUPPORT_OBJ50_V4
  TMWTYPES_ULONG count = 9900;
  TMWTYPES_BYTE unit = 5;
  TMWDTIME time;
#endif

  pDbHandle->iinBits = 0; 
  pDbHandle->dbLocalMode = TMWDEFS_FALSE;

  tmwsim_tableCreate(&pDbHandle->binaryInputs);

#if SDNPDATA_SUPPORT_OBJ1 || SDNPDATA_SUPPORT_OBJ2
#if SDNPSIM_NUM_BINARY_INPUTS > 0
  for(i = 0; i < SDNPSIM_NUM_BINARY_INPUTS; i++)
    sdnpsim_addBinaryInput(pDbHandle, TMWDEFS_CLASS_MASK_ONE, 0x01, TMWDEFS_FALSE);
#endif
#endif
  
  tmwsim_tableCreate(&pDbHandle->doubleInputs);

#if SDNPDATA_SUPPORT_OBJ3 || SDNPDATA_SUPPORT_OBJ4
#if SDNPSIM_NUM_DOUBLE_INPUTS > 0
  for(i = 0; i < SDNPSIM_NUM_DOUBLE_INPUTS; i++)
    sdnpsim_addDoubleInput(pDbHandle, TMWDEFS_CLASS_MASK_ONE, (DNPDEFS_DBAS_FLAG_ON_LINE | DNPDEFS_DBAS_FLAG_DOUBLE_OFF));
#endif
#endif

  tmwsim_tableCreate(&pDbHandle->binaryOutputs);

#if SDNPDATA_SUPPORT_OBJ10 || SDNPDATA_SUPPORT_OBJ12
#if SDNPSIM_NUM_BINARY_OUTPUTS > 0
  for(i = 0; i < SDNPSIM_NUM_BINARY_OUTPUTS; i++)
    sdnpsim_addBinaryOutput(pDbHandle, TMWDEFS_CLASS_MASK_NONE, 0x01, TMWDEFS_FALSE, 0xff);
#endif
#endif

  tmwsim_tableCreate(&pDbHandle->binaryCounters);

#if SDNPDATA_SUPPORT_OBJ20 || SDNPDATA_SUPPORT_OBJ21 || SDNPDATA_SUPPORT_OBJ22 || SDNPDATA_SUPPORT_OBJ23
#if SDNPSIM_NUM_COUNTERS > 0
  for(i = 0; i < SDNPSIM_NUM_COUNTERS; i++)
    sdnpsim_addBinaryCounter(pDbHandle, TMWDEFS_CLASS_MASK_THREE, TMWDEFS_CLASS_MASK_THREE, 0x01, 0);
#endif
#endif

  tmwsim_tableCreate(&pDbHandle->frozenCounters);

#if SDNPDATA_SUPPORT_OBJ20 || SDNPDATA_SUPPORT_OBJ21 || SDNPDATA_SUPPORT_OBJ22 || SDNPDATA_SUPPORT_OBJ23
#if SDNPSIM_NUM_COUNTERS > 0
  for(i = 0; i < SDNPSIM_NUM_COUNTERS; i++)
    sdnpsim_addFrozenCounter(pDbHandle, TMWDEFS_CLASS_MASK_THREE, TMWDEFS_CLASS_MASK_THREE, 0x01, 0);
#endif
#endif

  tmwsim_tableCreate(&pDbHandle->analogInputs);

#if SDNPDATA_SUPPORT_OBJ30 || SDNPDATA_SUPPORT_OBJ32
#if SDNPSIM_NUM_ANALOG_INPUTS > 0
  for(i = 0; i < SDNPSIM_NUM_ANALOG_INPUTS; i++)
    sdnpsim_addAnalogInput(pDbHandle, TMWDEFS_CLASS_MASK_TWO, 0x01, 0, 0);
#endif
#endif

  tmwsim_tableCreate(&pDbHandle->frozenAnalogInputs);

#if SDNPDATA_SUPPORT_OBJ31
  /* 
   * TB2018-004 requires for each Analog Input point Either Obj30 or Obj31 response to Class 0 but NOT both 
   * By default in this database Frozen Analog Input Obj31 will NOT be sent in response.
   * If you configure Frozen Analog Input Obj31 point x to be sent in Class 0 response, 
   * you Should configure Analog Input Obj30 point x to NOT be sent in Class 0 response.
   * Similarly, if an Obj30/32 point x is in an event class, the same Obj31/33 point x should 
   * not be in an event class visa versa.
   */
#if SDNPSIM_NUM_ANALOG_INPUTS > 0
  for(i = 0; i < SDNPSIM_NUM_ANALOG_INPUTS; i++)
    sdnpsim_addfrznAnlgInput(pDbHandle, TMWDEFS_CLASS_MASK_NOTCLASS0, 0x01, 0);
#endif
#endif

  tmwsim_tableCreate(&pDbHandle->analogOutputs);

#if SDNPDATA_SUPPORT_OBJ40 || SDNPDATA_SUPPORT_OBJ41
#if SDNPSIM_NUM_ANALOG_OUTPUTS > 0
  for(i = 0; i < SDNPSIM_NUM_ANALOG_OUTPUTS; i++)
    sdnpsim_addAnalogOutput(pDbHandle, TMWDEFS_CLASS_MASK_NONE, 0x01, 0);
#endif
#endif

  tmwsim_tableCreate(&pDbHandle->indexedTime);

#if SDNPDATA_SUPPORT_OBJ50_V4
#if SDNPSIM_NUM_TIME_INDEXED > 0
  sdnputil_getDateTime((TMWSESN*)pDbHandle->pSDNPSession, &time);
  time.invalid = false; /* Don't set a default startTime time to invalid */
  for (i = 0; i < SDNPSIM_NUM_TIME_INDEXED; i++)
    sdnpsim_addIndexedTime(pDbHandle, TMWDEFS_CLASS_MASK_NONE, time, count++, unit++);
#endif
#endif

  tmwsim_tableCreate(&pDbHandle->octetStrings);

#if SDNPDATA_SUPPORT_OBJ110 || SDNPDATA_SUPPORT_OBJ111
#if SDNPSIM_NUM_OCTET_STRINGS > 0
  for (i = 0; i < SDNPSIM_NUM_OCTET_STRINGS; i++)
    sdnpsim_addString(pDbHandle, TMWDEFS_CLASS_MASK_THREE, (TMWTYPES_UCHAR *)"Hello World", 11);
#endif

#endif
#if SDNPDATA_SUPPORT_OBJ114
#if SDNPSIM_NUM_EXT_OCTET_STRINGS > 0
  for(i = 0; i < SDNPSIM_NUM_EXT_OCTET_STRINGS; i++)
    sdnpsim_addExtString(pDbHandle, TMWDEFS_CLASS_MASK_THREE, DNPDEFS_DBAS_FLAG_ON_LINE, (TMWTYPES_UCHAR *)LONG_TEST_STRING, (TMWTYPES_USHORT)strlen(LONG_TEST_STRING));
#endif
#endif
  tmwsim_tableCreate(&pDbHandle->virtualTerminals);

#if SDNPDATA_SUPPORT_OBJ112 || SDNPDATA_SUPPORT_OBJ113
#if SDNPSIM_NUM_VIRTUAL_TERMINALS > 0
  for(i = 0; i < SDNPSIM_NUM_VIRTUAL_TERMINALS; i++)
    sdnpsim_addVirtualTerminal(pDbHandle, TMWDEFS_CLASS_MASK_THREE, (TMWTYPES_UCHAR *)"Hello World", 11);
#endif
#endif

  tmwsim_tableCreate(&pDbHandle->deviceAttrs);

#if SDNPDATA_SUPPORT_OBJ0 
  _createDeviceAttributes(pDbHandle);
#endif

#if SDNPDATA_SUPPORT_OBJ70
  sdnpfsim_initDb(&(pDbHandle->sdnpFsim));
#endif
  
  pDbHandle->authClassMask = TMWDEFS_CLASS_MASK_THREE;

  tmwdlist_initialize(&pDbHandle->authUsers);

  /* Initialize user managed event queues. 
   * These are only used if testing the user managed event interface
   * when the SCL does not manage the event queues.
   */
  tmwdlist_initialize(&pDbHandle->binInEvents);
  tmwdlist_initialize(&pDbHandle->binOutEvents);
  tmwdlist_initialize(&pDbHandle->binCmdEvents);
  tmwdlist_initialize(&pDbHandle->binCntrEvents);
  tmwdlist_initialize(&pDbHandle->frznCntrEvents);
  tmwdlist_initialize(&pDbHandle->anlgInEvents);
  tmwdlist_initialize(&pDbHandle->frznAnlgInEvents);
  tmwdlist_initialize(&pDbHandle->anlgOutEvents);
  tmwdlist_initialize(&pDbHandle->anlgCmdEvents);
  tmwdlist_initialize(&pDbHandle->datasetEvents);
  tmwdlist_initialize(&pDbHandle->stringEvents);
  tmwdlist_initialize(&pDbHandle->vtermEvents);
  tmwdlist_initialize(&pDbHandle->extendedStringEvents);
  tmwdlist_initialize(&pDbHandle->authErrorEvents);
  tmwdlist_initialize(&pDbHandle->authStatEvents);

#if SDNPDATA_SUPPORT_OBJ86
  _initDataSets(pDbHandle);
#endif
}

/* function: sdnpsim_getIIN */
TMWTYPES_USHORT sdnpsim_getIIN(
  TMWSESN *pSession)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pSDNPSession->pDbHandle;

  return(pDbHandle->iinBits);
}

/* function: sdnpsim_setIIN */
void sdnpsim_setIIN(
  TMWSESN *pSession,
  TMWTYPES_USHORT iinBits)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pSDNPSession->pDbHandle; 
  pDbHandle->iinBits = iinBits;
}

/* function: sdnpsim_IINQuantity */
TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpsim_IINQuantity(
  void *pHandle)
{
  TMWTARG_UNUSED_PARAM(pHandle);
  return(32);
}

/* function: sdnpsim_IINRead */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_IINRead(
  void *pHandle,
  TMWTYPES_USHORT pointNumber)
{
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(pointNumber);
  return(TMWDEFS_TRUE);
}

/* function: sdnpsim_setLocalMode */
void sdnpsim_setLocalMode(
    void *pHandle,
    TMWTYPES_BOOL state)
{
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;
  if (state == TMWDEFS_TRUE)
  {
    pDbHandle->dbLocalMode = TMWDEFS_TRUE;
    pDbHandle->iinBits |= DNPDEFS_IIN_LOCAL;
  }
  else
  {
    pDbHandle->dbLocalMode = TMWDEFS_FALSE;
    if(!sdnpsim_determineLocalMode(pHandle))
      pDbHandle->iinBits &= ~DNPDEFS_IIN_LOCAL;
  }
}

/* function: sdnpsim_determineLocalMode */
TMWTYPES_BOOL sdnpsim_determineLocalMode(
  void *pHandle)
{
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;
  TMWTYPES_USHORT pointNum = 0;
  void *pPoint;

  /* If the global flag is set to local for the database, no need to check points */
  if (pDbHandle->dbLocalMode)
  {
    pDbHandle->iinBits |= DNPDEFS_IIN_LOCAL;
    return TMWDEFS_TRUE;
  }

  /* Check all of the output points and see if any are in local mode
   * If so, set IIN1.5
   */
  while ((pPoint = sdnpsim_binOutGetPoint(pHandle, pointNum)) != TMWDEFS_NULL)
  {
    if (tmwsim_getLocal((TMWSIM_POINT *)pPoint))
    {
      pDbHandle->iinBits |= DNPDEFS_IIN_LOCAL;
      return TMWDEFS_TRUE;
    }
    pointNum++;
  }

  pointNum = 0;
  while ((pPoint = sdnpsim_anlgOutGetPoint(pHandle, pointNum)) != TMWDEFS_NULL)
  {
    if (tmwsim_getLocal((TMWSIM_POINT *)pPoint))
    {
      pDbHandle->iinBits |= DNPDEFS_IIN_LOCAL;
      return TMWDEFS_TRUE;
    }
    pointNum++;
  }
  pDbHandle->iinBits &= ~DNPDEFS_IIN_LOCAL;
  return TMWDEFS_FALSE;
}

/* function: _dbLocalMode 
 * Get local mode for entire database, using *pPoint as an argument
 */
static TMWTYPES_BOOL _dbLocalMode(
  void *pPoint)
{
  TMWSIM_POINT *pSimPoint = (TMWSIM_POINT*)pPoint;
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pSimPoint->pDbHandle;

  /* If the device or this point are in localMode clear the ONLINE flag */
  return(pDbHandle->dbLocalMode);
}

/* function: sdnpsim_init */
void * TMWDEFS_GLOBAL sdnpsim_init(
  TMWSESN *pSession)
{
  SDNPSIM_DATABASE *pDbHandle; 

  pDbHandle = (SDNPSIM_DATABASE *)sdnpmem_alloc(SDNPMEM_SIM_DATABASE_TYPE);
  if(pDbHandle != TMWDEFS_NULL)
  {
    pDbHandle->pSDNPSession = (SDNPSESN *)pSession;
    pDbHandle->pUpdateCallback = TMWDEFS_NULL;
    pDbHandle->pUpdateCallbackParam = TMWDEFS_NULL;
    
    /* Start here so in this simulated database we are less likely to 
     * overwrite existing user numbers. This would need to be done better in 
     * a real database.
     */
    pDbHandle->authNextUserNumber = 10;
    
#if TMWCNFG_USE_MANAGED_SCL
    memset(pDbHandle, 0, sizeof(SDNPSIM_DATABASE));
    pDbHandle->pSDNPSession = (SDNPSESN *)pSession;
    sdnpsim_clear(pDbHandle);
    
    /* For now set this here so it is done for managed database
     * It should really be a property that is saved in workspace 
     */
    pDbHandle->authClassMask = TMWDEFS_CLASS_MASK_THREE;

#if SDNPDATA_SUPPORT_OBJ70
    sdnpfsim_initDb(&(pDbHandle->sdnpFsim));
#endif

    _initSecStatsDb(pDbHandle);

#else
    _buildDb(pDbHandle);
    _initSecStatsDb(pDbHandle);
    _buildSecStatsDb(pDbHandle);
#endif
  }

  return(pDbHandle);
}

/* function: sdnpsim_close */
void TMWDEFS_GLOBAL sdnpsim_close(
  void *pHandle)
{
  _clearDb(pHandle);
  _clearSecStatsDb(pHandle);
  _deleteSecStatsDb(pHandle);

  sdnpmem_free(pHandle);
}

/* function: sdnpsim_clear */
void TMWDEFS_GLOBAL sdnpsim_clear(
  void *pHandle)
{  
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;
  _clearDb(pHandle);
  
  /* Recreate/reinit tables, so points can be added */
  tmwsim_tableCreate(&pDbHandle->binaryInputs);
  tmwsim_tableCreate(&pDbHandle->doubleInputs);
  tmwsim_tableCreate(&pDbHandle->binaryOutputs);
  tmwsim_tableCreate(&pDbHandle->binaryCounters);
  tmwsim_tableCreate(&pDbHandle->frozenCounters);
  tmwsim_tableCreate(&pDbHandle->analogInputs);
  tmwsim_tableCreate(&pDbHandle->frozenAnalogInputs);
  tmwsim_tableCreate(&pDbHandle->analogOutputs);
  tmwsim_tableCreate(&pDbHandle->indexedTime);
  tmwsim_tableCreate(&pDbHandle->octetStrings);
  tmwsim_tableCreate(&pDbHandle->virtualTerminals); 
  tmwsim_tableCreate(&pDbHandle->deviceAttrs);

  /* Dont clear AuthSecStats, they must be present if SA is enabled. */

#if SDNPDATA_SUPPORT_DATASETS
  tmwdlist_initialize(&pDbHandle->datasetProtos);
  tmwdlist_initialize(&pDbHandle->datasetDescrDatas); 
#endif
}

/* function: sdnpsim_reset */
void TMWDEFS_GLOBAL sdnpsim_reset(
  void *pHandle)
{
  _clearDb(pHandle);
  _clearSecStatsDb(pHandle);
  _buildDb(pHandle); 
  _buildSecStatsDb(pHandle);
}

void TMWDEFS_GLOBAL sdnpsim_addSecStats(
  void *pHandle)
{
#if SDNPDATA_SUPPORT_OBJ120
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;
  int i;
  if (pDbHandle->pSDNPSession->authenticationEnabled && (pDbHandle->pSDNPSession->dnp.operateInV2Mode == TMWDEFS_FALSE))
  {
    for (i = 0; i < DNPAUTH_NUMBER_STATISTICS; i++)
      sdnpsim_addAuthSecStat(pDbHandle, (TMWTYPES_USHORT)i, TMWDEFS_CLASS_MASK_THREE, 0x01, 0);  /* spec says it SHALL be in an event class */
  }
#else
  TMWTARG_UNUSED_PARAM(pHandle);
#endif
}

/* Set update callback and parameter */
void sdnpsim_setCallback(
  void *pHandle,
  SDNPSIM_CALLBACK_FUNC pUpdateCallback,
  void *pUpdateCallbackParam)
{
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;
  pDbHandle->pUpdateCallback = pUpdateCallback;
  pDbHandle->pUpdateCallbackParam = pUpdateCallbackParam;
}

/* function: sdnpsim_enablePoint */
void TMWDEFS_GLOBAL sdnpsim_enablePoint(
  void *pPoint,
  TMWTYPES_BOOL enabled)
{
  tmwsim_setEnabled((TMWSIM_POINT *)pPoint, enabled);
}

/* function: sdnpsim_isPointEnabled */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_isPointEnabled(
  void *pPoint)
{
  return(tmwsim_getEnabled((TMWSIM_POINT *)pPoint));
}

/* function: sdnpsim_getPointNumber */
TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpsim_getPointNumber(
  void *pPoint)
{
  return((TMWTYPES_USHORT)tmwsim_getPointNumber((TMWSIM_POINT *)pPoint));
}

/* function: sdnpsim_getEventClass */
TMWDEFS_CLASS_MASK TMWDEFS_GLOBAL sdnpsim_getEventClass(
  void *pPoint)
{
  return(tmwsim_getEventClass((TMWSIM_POINT *)pPoint));
}

/* function: sdnpsim_setEventClass */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_setEventClass(
  void *pPoint,
  TMWDEFS_CLASS_MASK classMask)
{
  /* See if this point has been set to fail for test purposes */
  if(tmwsim_getTestingMode((TMWSIM_POINT *)pPoint))
    return(TMWDEFS_FALSE);

  tmwsim_setEventClass((TMWSIM_POINT *)pPoint, classMask);
  return(TMWDEFS_TRUE);
}

/* function: sdnpsim_getCmdEventClass */
TMWDEFS_CLASS_MASK TMWDEFS_GLOBAL sdnpsim_getCmdEventClass(
  void *pPoint)
{
  return(tmwsim_getCmdEventClass((TMWSIM_POINT *)pPoint));
}

/* function: sdnpsim_setCmdEventClass */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_setCmdEventClass(
  void *pPoint,
  TMWDEFS_CLASS_MASK classMask)
{
  /* See if this point has been set to fail for test purposes */
  if(tmwsim_getTestingMode((TMWSIM_POINT *)pPoint))
    return(TMWDEFS_FALSE);

  tmwsim_setCmdEventClass((TMWSIM_POINT *)pPoint, classMask);
  return(TMWDEFS_TRUE);
}

/* function: sdnpsim_inClass0 */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_inClass0(
   void *pPoint)
{
  if((tmwsim_getEventClass((TMWSIM_POINT *)pPoint) & TMWDEFS_CLASS_MASK_NOTCLASS0) == 0)
    return TMWDEFS_TRUE;
  else
    return TMWDEFS_FALSE;
}

/* function: sdnpsim_getEventMode */
TMWDEFS_EVENT_MODE sdnpsim_getEventMode(
  void *pPoint) 
{
  TMWSIM_POINT *pSimPoint = (TMWSIM_POINT *)pPoint;
  return (TMWDEFS_EVENT_MODE)pSimPoint->eventMode;
}

/* function: sdnpsim_setEventMode */
void sdnpsim_setEventMode(
  void *pPoint,
  TMWDEFS_EVENT_MODE mode) 
{
  TMWSIM_POINT *pSimPoint = (TMWSIM_POINT *)pPoint;
  pSimPoint->eventMode = (TMWTYPES_UCHAR)mode;
}

/* function: sdnpsim_getFlags */
TMWTYPES_UCHAR TMWDEFS_GLOBAL sdnpsim_getFlags(
  void *pPoint)
{
  return(tmwsim_getFlags((TMWSIM_POINT *)pPoint));
}

/* function: sdnpsim_addBinaryInput */
void * TMWDEFS_GLOBAL sdnpsim_addBinaryInput(
  void *pHandle,
  TMWDEFS_CLASS_MASK classMask,
  TMWTYPES_UCHAR flags,
  TMWTYPES_BOOL value)
{
  TMWSIM_POINT *pPoint;
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;

  TMWTYPES_USHORT pointNum = (TMWTYPES_USHORT)tmwsim_tableSize(&pDbHandle->binaryInputs);
  pPoint = tmwsim_tableAdd(&pDbHandle->binaryInputs, pointNum);
  if(pPoint != TMWDEFS_NULL)
  {
    tmwsim_initBinary(pPoint, pHandle, pointNum);
    pPoint->pSCLHandle = (void*)pDbHandle->pSDNPSession;
    pPoint->classMask = classMask;
    pPoint->flags = flags;
    pPoint->defaultEventVariation = 3;
    pPoint->defaultStaticVariation = 1;
    pPoint->eventMode = (TMWTYPES_UCHAR)TMWDEFS_EVENT_MODE_SOE;
    pPoint->data.binary.value = value;
    sdnputil_getDateTime((TMWSESN*)pPoint->pSCLHandle, &pPoint->timeStamp); 
  
    _callCallback((TMWSIM_POINT *)pPoint, TMWSIM_POINT_ADD, DNPDEFS_OBJ_1_BIN_INPUTS, 
      (TMWTYPES_USHORT)tmwsim_getPointNumber((TMWSIM_POINT *)pPoint));
  }

  return(pPoint);
}

/* function: sdnpsim_deleteBinaryInput */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_deleteBinaryInput(
  void *pHandle)
{
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;
  TMWTYPES_USHORT pointNumber = (TMWTYPES_USHORT)tmwsim_tableSize(&pDbHandle->binaryInputs);
  if(pointNumber > 0)
  { 
    _callRemoveCallback(pDbHandle, TMWSIM_POINT_DELETE, DNPDEFS_OBJ_1_BIN_INPUTS, (TMWTYPES_USHORT)(pointNumber-1)); 
    return(tmwsim_tableDelete(&pDbHandle->binaryInputs, pointNumber-1));
  }

  return(TMWDEFS_FALSE);
}

/* function: sdnpsim_binInQuantity */
TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpsim_binInQuantity(
  void *pHandle)
{
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;
  return((TMWTYPES_USHORT)tmwsim_tableSize(&pDbHandle->binaryInputs));
}

/* function: sdnpsim_binInGetPoint */
void * TMWDEFS_GLOBAL sdnpsim_binInGetPoint(
  void *pHandle,
  TMWTYPES_USHORT pointNum)
{
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;
  return(tmwsim_tableFindPoint(&pDbHandle->binaryInputs, pointNum));
}

/* function: sdnpsim_binInSetFlags */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_binInSetFlags(
  void *pPoint,
  TMWTYPES_UCHAR flags)
{
  TMWSIM_POINT *pSimPoint = (TMWSIM_POINT *)pPoint;
  tmwsim_setFlags(pSimPoint, flags, TMWDEFS_CHANGE_REMOTE_OP);
  sdnputil_getDateTime((TMWSESN*)pSimPoint->pSCLHandle, &pSimPoint->timeStamp); 

  _callCallback(pSimPoint, TMWSIM_POINT_UPDATE, DNPDEFS_OBJ_1_BIN_INPUTS, 
    (TMWTYPES_USHORT)tmwsim_getPointNumber(pSimPoint));

  return(TMWDEFS_TRUE);
}

/* function: sdnpsim_binInGetEnabledPoint */
void * TMWDEFS_GLOBAL sdnpsim_binInGetEnabledPoint(
  void *pHandle,
  TMWTYPES_USHORT pointNum)
{
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;
  TMWSIM_POINT *pPoint = tmwsim_tableFindPoint(&pDbHandle->binaryInputs, pointNum);
  if((pPoint == TMWDEFS_NULL) || !pPoint->enabled)
    return(TMWDEFS_NULL);

  return(pPoint);
}

/* function: sdnpsim_binInDefVariation */
TMWTYPES_UCHAR TMWDEFS_GLOBAL sdnpsim_binInDefVariation(
  void *pPoint)
{
  return(tmwsim_getDefStaticVariation((TMWSIM_POINT *)pPoint));
}

TMWTYPES_UCHAR sdnpsim_binInEventDefVariation(
  void *pPoint,
  TMWDEFS_CLASS_MASK classMask)
{
  TMWTARG_UNUSED_PARAM(classMask);
  return(tmwsim_getDefEventVariation((TMWSIM_POINT *)pPoint));
}

/* function: sdnpsim_binInRead */
void TMWDEFS_GLOBAL sdnpsim_binInRead(
  void *pPoint,
  TMWTYPES_BOOL *pValue,
  TMWTYPES_UCHAR *pFlags)
{
  *pFlags = tmwsim_getFlags((TMWSIM_POINT *)pPoint);
  *pValue = tmwsim_getBinaryValue((TMWSIM_POINT *)pPoint);
  return;
}

/* function: sdnpsim_binInChanged */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_binInChanged(
  void *pPoint,
  TMWTYPES_BOOL *pValue,
  TMWTYPES_UCHAR *pFlags)
{
  TMWDEFS_CHANGE_REASON reason;
  if(tmwsim_getChanged((TMWSIM_POINT *)pPoint, &reason))
  {
    *pFlags = tmwsim_getFlags((TMWSIM_POINT *)pPoint);
    *pValue = tmwsim_getBinaryValue((TMWSIM_POINT *)pPoint);
    return(TMWDEFS_TRUE);
  }

  return(TMWDEFS_FALSE);
}

/* function: sdnpsim_binInWrite */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_binInWrite(
  void *pPoint,
  TMWTYPES_BOOL value)
{
  TMWSIM_POINT *pSimPoint = (TMWSIM_POINT *)pPoint;
  tmwsim_setBinaryValue(pSimPoint, value, TMWDEFS_CHANGE_LOCAL_OP);
  sdnputil_getDateTime((TMWSESN*)pSimPoint->pSCLHandle, &pSimPoint->timeStamp); 

  _callCallback(pSimPoint, TMWSIM_POINT_UPDATE, DNPDEFS_OBJ_1_BIN_INPUTS, 
    (TMWTYPES_USHORT)tmwsim_getPointNumber(pSimPoint));

  return(TMWDEFS_TRUE);
}

/* function: sdnpsim_addBinaryOutput */
void * TMWDEFS_GLOBAL sdnpsim_addBinaryOutput(
  void *pHandle,
  TMWDEFS_CLASS_MASK classMask,
  TMWTYPES_UCHAR flags,
  TMWTYPES_BOOL value,
  SDNPDATA_CROB_CTRL controlMask)
{
  TMWSIM_POINT *pPoint;
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;

  TMWTYPES_USHORT pointNum = (TMWTYPES_USHORT)tmwsim_tableSize(&pDbHandle->binaryOutputs);
  pPoint = tmwsim_tableAdd(&pDbHandle->binaryOutputs, pointNum);
  if(pPoint != TMWDEFS_NULL)
  {
    tmwsim_initBinary(pPoint, pHandle, pointNum);
    pPoint->pSCLHandle = (void*)pDbHandle->pSDNPSession;
    pPoint->classMask = classMask;
    pPoint->flags = flags;
    pPoint->defaultStaticVariation = 2;
    pPoint->defaultEventVariation = 1;
    pPoint->eventMode = (TMWTYPES_UCHAR)TMWDEFS_EVENT_MODE_SOE;
    pPoint->data.binary.value = value;
    pPoint->data.binary.control = controlMask;
    sdnputil_getDateTime((TMWSESN*)pPoint->pSCLHandle, &pPoint->timeStamp);

    _callCallback((TMWSIM_POINT *)pPoint, TMWSIM_POINT_ADD, DNPDEFS_OBJ_10_BIN_OUT_STATUSES, 
      (TMWTYPES_USHORT)tmwsim_getPointNumber((TMWSIM_POINT *)pPoint));
  }

  return(pPoint);
}

/* function: sdnpsim_deleteBinaryOutput */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_deleteBinaryOutput(
  void *pHandle)
{ 
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;
  TMWTYPES_USHORT pointNumber = (TMWTYPES_USHORT)tmwsim_tableSize(&pDbHandle->binaryOutputs);
  if(pointNumber > 0)
  { 
    _callRemoveCallback(pDbHandle, TMWSIM_POINT_DELETE, DNPDEFS_OBJ_10_BIN_OUT_STATUSES, (TMWTYPES_USHORT)(pointNumber-1));
    return(tmwsim_tableDelete(&pDbHandle->binaryOutputs, pointNumber-1));
  }

  return(TMWDEFS_FALSE);
}

/* function: sdnpsim_binOutQuantity */
TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpsim_binOutQuantity(
  void *pHandle)
{
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;
  return((TMWTYPES_USHORT)tmwsim_tableSize(&pDbHandle->binaryOutputs));
}

/* function: sdnpsim_binOutGetPoint */
void * TMWDEFS_GLOBAL sdnpsim_binOutGetPoint(
  void *pHandle,
  TMWTYPES_USHORT pointNum)
{
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;
  return(tmwsim_tableFindPoint(&pDbHandle->binaryOutputs, pointNum));
}

/* function: sdnpsim_binOutSetFlags */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_binOutSetFlags(
  void *pPoint,
  TMWTYPES_UCHAR flags)
{
  TMWSIM_POINT *pSimPoint = (TMWSIM_POINT *)pPoint;
  tmwsim_setFlags(pSimPoint, flags, TMWDEFS_CHANGE_REMOTE_OP);
  sdnputil_getDateTime((TMWSESN*)pSimPoint->pSCLHandle, &pSimPoint->timeStamp); 

  _callCallback(pSimPoint, TMWSIM_POINT_UPDATE, DNPDEFS_OBJ_10_BIN_OUT_STATUSES, 
    (TMWTYPES_USHORT)tmwsim_getPointNumber(pSimPoint));
   
  return(TMWDEFS_TRUE);
}

/* function: sdnpsim_binOutGetEnabledPoint */
void * TMWDEFS_GLOBAL sdnpsim_binOutGetEnabledPoint(
  void *pHandle,
  TMWTYPES_USHORT pointNum)
{
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;
  TMWSIM_POINT *pPoint = tmwsim_tableFindPoint(&pDbHandle->binaryOutputs, pointNum);
  if((pPoint == TMWDEFS_NULL) || !pPoint->enabled)
    return(TMWDEFS_NULL);

  return(pPoint);
}

#if (SDNPDATA_SUPPORT_OBJ11 || SDNPDATA_SUPPORT_OBJ13)
/* function: sdnpsim_binOutEventDefVariation */
TMWTYPES_UCHAR sdnpsim_binOutEventDefVariation(
  void *pPoint,
  TMWDEFS_CLASS_MASK classMask)
{
  TMWTARG_UNUSED_PARAM(classMask);
  return(tmwsim_getDefEventVariation((TMWSIM_POINT *)pPoint));
}
#endif

#if SDNPDATA_SUPPORT_OBJ11
/* function: sdnpsim_binOutChanged */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_binOutChanged(
  void *pPoint,
  TMWTYPES_BOOL *pValue,
  TMWTYPES_UCHAR *pFlags)
{
  TMWDEFS_CHANGE_REASON reason;
  if(tmwsim_getChanged((TMWSIM_POINT *)pPoint, &reason))
  {
    *pFlags = tmwsim_getFlags((TMWSIM_POINT *)pPoint);
    *pValue = tmwsim_getBinaryValue((TMWSIM_POINT *)pPoint);
    return(TMWDEFS_TRUE);
  }

  return(TMWDEFS_FALSE);
}
#endif

/* function: sdnpsim_binOutSetControlMask */
void sdnpsim_binOutSetControlMask(
  void *pPoint,
  SDNPDATA_CROB_CTRL control)
{
  ((TMWSIM_POINT *)pPoint)->data.binary.control = (TMWTYPES_UCHAR)control;
}

/* function: sdnpsim_binOutGetControl */
SDNPDATA_CROB_CTRL sdnpsim_binOutGetControlMask(
  void *pPoint)
{
  return((SDNPDATA_CROB_CTRL)(((TMWSIM_POINT *)pPoint)->data.binary.control));
}

/* function: sdnpsim_binOutDefVariation */
TMWTYPES_UCHAR TMWDEFS_GLOBAL sdnpsim_binOutDefVariation(
  void *pPoint)
{
  return(tmwsim_getDefStaticVariation((TMWSIM_POINT *)pPoint));
}

/* function: sdnpsim_binOutRead */
void TMWDEFS_GLOBAL sdnpsim_binOutRead(
  void *pPoint,
  TMWTYPES_BOOL *pValue,
  TMWTYPES_UCHAR *pFlags)
{
  /* If the device or this point are in localMode clear the ONLINE flag */
  if(_dbLocalMode(pPoint)
    || tmwsim_getLocal((TMWSIM_POINT *)pPoint))
  {
    /* Clear ONLINE bit */
    *pFlags = tmwsim_getFlags((TMWSIM_POINT *)pPoint) & ~DNPDEFS_DBAS_FLAG_ON_LINE;
  }
  else
  {
    *pFlags = tmwsim_getFlags((TMWSIM_POINT *)pPoint);
  }

  *pValue = tmwsim_getBinaryValue((TMWSIM_POINT *)pPoint);
  return;
}

/* function: sdnpsim_binOutWrite */
/* This is used for binout write from master */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_binOutWrite(
  void *pPoint,
  TMWTYPES_BOOL value)
{
  TMWSIM_POINT *pSimPoint = (TMWSIM_POINT *)pPoint;

  /* See if this point is in localMode */
  if(tmwsim_getLocal((TMWSIM_POINT *)pPoint))
  { 
    return(TMWDEFS_FALSE);
  }

  /* This is used for binout write from master */
  tmwsim_setBinaryValue(pSimPoint, value, TMWDEFS_CHANGE_REMOTE_OP);
  sdnputil_getDateTime((TMWSESN*)pSimPoint->pSCLHandle, &pSimPoint->timeStamp); 

  _callCallback(pSimPoint, TMWSIM_POINT_UPDATE, DNPDEFS_OBJ_10_BIN_OUT_STATUSES, 
    (TMWTYPES_USHORT)tmwsim_getPointNumber(pSimPoint));
    
  return(TMWDEFS_TRUE);
}

/* function: sdnpsim_binOutSetValue */
/* This is used for a local database set value */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_binOutSetValue(
  void *pPoint,
  TMWTYPES_BOOL value)
{
  TMWSIM_POINT *pSimPoint = (TMWSIM_POINT *)pPoint;
   
  tmwsim_setBinaryValue(pSimPoint, value, TMWDEFS_CHANGE_LOCAL_OP);
  sdnputil_getDateTime((TMWSESN*)pSimPoint->pSCLHandle, &pSimPoint->timeStamp); 

  _callCallback(pSimPoint, TMWSIM_POINT_UPDATE, DNPDEFS_OBJ_10_BIN_OUT_STATUSES, 
    (TMWTYPES_USHORT)tmwsim_getPointNumber(pSimPoint));
    
  return(TMWDEFS_TRUE);
}

/* function: sdnpsim_binOutSelect */
DNPDEFS_CROB_ST TMWDEFS_GLOBAL sdnpsim_binOutSelect(
  void *pPoint,
  TMWTYPES_UCHAR control,
  TMWTYPES_UCHAR count,
  TMWTYPES_ULONG onTime,
  TMWTYPES_ULONG offTime)
{
  TMWTYPES_UCHAR mode;
  TMWTYPES_UCHAR supported = ((TMWSIM_POINT *)pPoint)->data.binary.control;

  TMWTARG_UNUSED_PARAM(count);
  TMWTARG_UNUSED_PARAM(onTime);
  TMWTARG_UNUSED_PARAM(offTime);
  
  /* See if this point has been set to fail for test purposes */
  mode = tmwsim_getTestingMode((TMWSIM_POINT *)pPoint);
  if((mode == TMWSIM_TESTINGMODE_FAILURE)
    ||(mode == TMWSIM_TESTINGMODE_SELECTFAIL))
  {
   /* For test purposes, use the flag value as the CROB status code 
     * This way the testHarness user can set the CROB status code 
     * to generate particular response status codes.
     */
    int flags = tmwsim_getFlags((TMWSIM_POINT *)pPoint);
      return((DNPDEFS_CROB_ST)flags);
  }

  /* If the device or this point are in localMode  */
  if (_dbLocalMode(pPoint)
    || tmwsim_getLocal((TMWSIM_POINT *)pPoint))
  { 
    return(DNPDEFS_CROB_ST_LOCAL);
  }

  switch(control & DNPDEFS_CROB_CTRL_PAIRED_OP)
  {
  case DNPDEFS_CROB_CTRL_PAIRED_TRIP:
    if((supported & SDNPDATA_CROB_CTRL_PAIRED_TRIP) == 0)
      return(DNPDEFS_CROB_ST_NOT_SUPPORTED);

    break;

  case DNPDEFS_CROB_CTRL_PAIRED_CLOSE:
    if((supported & SDNPDATA_CROB_CTRL_PAIRED_CLOSE) == 0)
      return(DNPDEFS_CROB_ST_NOT_SUPPORTED);

    break;

  case 0:
    switch(control & DNPDEFS_CROB_CTRL_MASK)
    {
    case DNPDEFS_CROB_CTRL_PULSE_ON:
      if((supported & SDNPDATA_CROB_CTRL_PULSE_ON) == 0)
        return(DNPDEFS_CROB_ST_NOT_SUPPORTED);

      break;

    case DNPDEFS_CROB_CTRL_PULSE_OFF:
      if((supported & SDNPDATA_CROB_CTRL_PULSE_OFF) == 0)
        return(DNPDEFS_CROB_ST_NOT_SUPPORTED);

      break;

    case DNPDEFS_CROB_CTRL_LATCH_ON:
      if((supported & SDNPDATA_CROB_CTRL_LATCH_ON) == 0)
        return(DNPDEFS_CROB_ST_NOT_SUPPORTED);

      break;

    case DNPDEFS_CROB_CTRL_LATCH_OFF:
      if((supported & SDNPDATA_CROB_CTRL_LATCH_OFF) == 0)
        return(DNPDEFS_CROB_ST_NOT_SUPPORTED);

      break;

    default:
      return(DNPDEFS_CROB_ST_NOT_SUPPORTED);
    }

    break;

  default:
    return(DNPDEFS_CROB_ST_NOT_SUPPORTED);
  }

  return(DNPDEFS_CROB_ST_SUCCESS);
}

/* function: sdnpsim_binOutOperate */
DNPDEFS_CROB_ST TMWDEFS_GLOBAL sdnpsim_binOutOperate(
  void *pPoint,
  TMWTYPES_UCHAR control,
  TMWTYPES_UCHAR count,
  TMWTYPES_ULONG onTime,
  TMWTYPES_ULONG offTime)
{
  TMWTYPES_UCHAR mode;
  TMWTYPES_UCHAR flags;
  TMWSIM_POINT *pSimPoint = (TMWSIM_POINT *)pPoint;
  TMWTYPES_UCHAR supported = (pSimPoint)->data.binary.control;
  TMWTYPES_BOOL value = 0;

  TMWTARG_UNUSED_PARAM(count);
  TMWTARG_UNUSED_PARAM(onTime);
  TMWTARG_UNUSED_PARAM(offTime); 
  
  /* See if this point has been set to fail for test purposes */
  mode = tmwsim_getTestingMode(pSimPoint);
  if((mode == TMWSIM_TESTINGMODE_FAILURE)
    ||(mode == TMWSIM_TESTINGMODE_OPERATEFAIL))
  {
    /* For test purposes, use the flag value as the CROB status code 
     * This way the testHarness user can set the CROB status code 
     * to generate particular response status codes.
     */
    flags = tmwsim_getFlags(pSimPoint);
    return((DNPDEFS_CROB_ST)flags);
  }

  /* If the device or this point are in localMode  */
  if (_dbLocalMode(pPoint)
    || tmwsim_getLocal((TMWSIM_POINT *)pPoint))
  { 
    return(DNPDEFS_CROB_ST_LOCAL);
  }

  switch(control & DNPDEFS_CROB_CTRL_PAIRED_OP)
  {
  case DNPDEFS_CROB_CTRL_PAIRED_TRIP:
    if((supported & SDNPDATA_CROB_CTRL_PAIRED_TRIP) == 0)
      return(DNPDEFS_CROB_ST_NOT_SUPPORTED);

    value = 0;
    break;

  case DNPDEFS_CROB_CTRL_PAIRED_CLOSE:
    if((supported & SDNPDATA_CROB_CTRL_PAIRED_CLOSE) == 0)
      return(DNPDEFS_CROB_ST_NOT_SUPPORTED);

    value = 1;
    break;

  case 0:
    switch(control & DNPDEFS_CROB_CTRL_MASK)
    {
    case DNPDEFS_CROB_CTRL_PULSE_ON:
      if((supported & SDNPDATA_CROB_CTRL_PULSE_ON) == 0)
        return(DNPDEFS_CROB_ST_NOT_SUPPORTED);

      value = 1;
      break;

    case DNPDEFS_CROB_CTRL_PULSE_OFF:
      if((supported & SDNPDATA_CROB_CTRL_PULSE_OFF) == 0)
        return(DNPDEFS_CROB_ST_NOT_SUPPORTED);

      value = 0;
      break;

    case DNPDEFS_CROB_CTRL_LATCH_ON:
      if((supported & SDNPDATA_CROB_CTRL_LATCH_ON) == 0)
        return(DNPDEFS_CROB_ST_NOT_SUPPORTED);

      value = 1;
      break;

    case DNPDEFS_CROB_CTRL_LATCH_OFF:
      if((supported & SDNPDATA_CROB_CTRL_LATCH_OFF) == 0)
        return(DNPDEFS_CROB_ST_NOT_SUPPORTED);

      value = 0;
      break;

    default:
      return(DNPDEFS_CROB_ST_NOT_SUPPORTED);
    }

    break;

  default:
    return(DNPDEFS_CROB_ST_NOT_SUPPORTED);
  }

  tmwsim_setBinaryValue(pSimPoint, value, TMWDEFS_CHANGE_REMOTE_OP);
  sdnputil_getDateTime((TMWSESN*)pSimPoint->pSCLHandle, &pSimPoint->timeStamp); 

  if (pSimPoint->assocPointNumber != TMWDEFS_NO_ASSOC_POINT)
  {
    TMWSIM_POINT *binInPtr = (TMWSIM_POINT*)sdnpsim_binInGetPoint(pSimPoint->pDbHandle, (TMWTYPES_USHORT)pSimPoint->assocPointNumber);
    if (binInPtr != TMWDEFS_NULL)
    {
      sdnpsim_binInWrite(binInPtr, value);
    }
  }

  _callCallback(pSimPoint, TMWSIM_POINT_UPDATE, DNPDEFS_OBJ_10_BIN_OUT_STATUSES, 
    (TMWTYPES_USHORT)tmwsim_getPointNumber(pSimPoint));
  
#if SDNPDATA_SUPPORT_OBJ11
  {
    TMWDEFS_CHANGE_REASON reason;
    flags = tmwsim_getFlags(pSimPoint);
    if (value)
      flags |= DNPDEFS_DBAS_FLAG_BINARY_ON;
    else
      flags &= ~DNPDEFS_DBAS_FLAG_BINARY_ON;

    if (tmwsim_getChanged(pSimPoint, &reason))
      sdnpo011_addEvent((TMWSESN*)pSimPoint->pSCLHandle, (TMWTYPES_USHORT)pSimPoint->pointNumber, flags, &pSimPoint->timeStamp);
  }
#endif

  return(DNPDEFS_CROB_ST_SUCCESS);
}

#if SDNPDATA_SUPPORT_OBJ12_V3
/* function: sdnpsim_binOutSelPatternMask */
DNPDEFS_CROB_ST sdnpsim_binOutSelPatternMask(
  void *pHandle,
  TMWTYPES_UCHAR control,
  TMWTYPES_UCHAR count,
  TMWTYPES_ULONG activationPeriod,
  TMWTYPES_ULONG deactivationPeriod,
  TMWTYPES_USHORT firstPointNumber,
  TMWTYPES_USHORT lastPointNumber,
  TMWTYPES_UCHAR *pMask)
{
  TMWTYPES_UCHAR mode;
  TMWTYPES_USHORT maxPointNum = sdnpsim_binOutQuantity(pHandle);
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;
  void *pPoint;

  TMWTARG_UNUSED_PARAM(control);
  TMWTARG_UNUSED_PARAM(count);
  TMWTARG_UNUSED_PARAM(activationPeriod);
  TMWTARG_UNUSED_PARAM(deactivationPeriod);
  TMWTARG_UNUSED_PARAM(pMask);

  /* See if this point has been set to fail for test purposes */
  pPoint = tmwsim_tableFindPoint(&pDbHandle->binaryOutputs, firstPointNumber);
  mode = tmwsim_getTestingMode((TMWSIM_POINT *)pPoint);
  if((mode == TMWSIM_TESTINGMODE_FAILURE)
    ||(mode == TMWSIM_TESTINGMODE_SELECTFAIL))
  {
    /* For test purposes, use the flag value as the CROB status code 
     * This way the testHarness user can set the CROB status code 
     * to generate particular response status codes.
     */
    int flags = tmwsim_getFlags((TMWSIM_POINT *)pPoint);
    return((DNPDEFS_CROB_ST)flags);
  }

  if(firstPointNumber > lastPointNumber)
    return(DNPDEFS_CROB_ST_FORMAT_ERROR);

  if(firstPointNumber > maxPointNum)
    return(DNPDEFS_CROB_ST_NOT_SUPPORTED);

  return(DNPDEFS_CROB_ST_SUCCESS);
}

/* function: sdnpsim_binOutOpPatternMask */
DNPDEFS_CROB_ST sdnpsim_binOutOpPatternMask(
  void *pHandle,
  TMWTYPES_UCHAR control,
  TMWTYPES_UCHAR count,
  TMWTYPES_ULONG activationPeriod,
  TMWTYPES_ULONG deactivationPeriod,
  TMWTYPES_USHORT firstPointNumber,
  TMWTYPES_USHORT lastPointNumber,
  TMWTYPES_UCHAR *pMask)
{
  TMWTYPES_USHORT maxPointNum = sdnpsim_binOutQuantity(pHandle);
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;
  TMWTYPES_USHORT pointNum;
  TMWTYPES_UCHAR shift;
  TMWTYPES_UCHAR mode;
  void *pPoint;

  TMWTARG_UNUSED_PARAM(control);
  TMWTARG_UNUSED_PARAM(count);
  TMWTARG_UNUSED_PARAM(activationPeriod);
  TMWTARG_UNUSED_PARAM(deactivationPeriod);
  TMWTARG_UNUSED_PARAM(pMask);

  /* See if this point has been set to fail for test purposes */
  pPoint = tmwsim_tableFindPoint(&pDbHandle->binaryOutputs, firstPointNumber);
  mode = tmwsim_getTestingMode((TMWSIM_POINT *)pPoint);
  if((mode == TMWSIM_TESTINGMODE_FAILURE)
    ||(mode == TMWSIM_TESTINGMODE_OPERATEFAIL))
  {
    /* For test purposes, use the flag value as the CROB status code 
     * This way the testHarness user can set the CROB status code 
     * to generate particular response status codes.
     */
    int flags = tmwsim_getFlags((TMWSIM_POINT *)pPoint);
    return((DNPDEFS_CROB_ST)flags);
  }

  if(firstPointNumber > lastPointNumber)
    return(DNPDEFS_CROB_ST_FORMAT_ERROR);

  if(firstPointNumber > maxPointNum)
    return(DNPDEFS_CROB_ST_NOT_SUPPORTED);

  shift = 0;
  for(pointNum = firstPointNumber; pointNum <= lastPointNumber; pointNum++)
  {
    if(*pMask & (0x01 << shift))
    {
      TMWSIM_POINT *pSimPoint = (TMWSIM_POINT*)sdnpsim_binOutGetEnabledPoint(pHandle, pointNum);
      if(pSimPoint == TMWDEFS_NULL)
      {
        return(DNPDEFS_CROB_ST_NOT_SUPPORTED);
      }

      sdnpsim_binOutOperate((void*)pSimPoint, control, count, activationPeriod, deactivationPeriod);
      sdnputil_getDateTime((TMWSESN*)pSimPoint->pSCLHandle, &pSimPoint->timeStamp); 
    }

    if(++shift >= 8)
    {
      shift = 0;
      pMask++;
    }
  }

  return(DNPDEFS_CROB_ST_SUCCESS);
}
#endif

/* function: sdnpsim_addBinaryCounter */
void * TMWDEFS_GLOBAL sdnpsim_addBinaryCounter(
  void *pHandle,
  TMWDEFS_CLASS_MASK binClassMask,
  TMWDEFS_CLASS_MASK frznClassMask,
  TMWTYPES_UCHAR flags,
  TMWTYPES_ULONG value)
{
  TMWSIM_POINT *pPoint;
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;
  TMWTYPES_USHORT pointNum = (TMWTYPES_USHORT)tmwsim_tableSize(&pDbHandle->binaryCounters);
  TMWTARG_UNUSED_PARAM(frznClassMask);

  pPoint = tmwsim_tableAdd(&pDbHandle->binaryCounters, pointNum);
  if(pPoint != TMWDEFS_NULL)
  {
    tmwsim_initCounter(pPoint, pHandle, pointNum);

    pPoint->pSCLHandle = (void*)pDbHandle->pSDNPSession;
    pPoint->flags = flags; 
    pPoint->defaultStaticVariation = 5;  /* 32 bit without flag */
    pPoint->defaultEventVariation = 1;   /* 32 bit with flag */ 
    pPoint->eventMode = (TMWTYPES_UCHAR)TMWDEFS_EVENT_MODE_MOST_RECENT;
    pPoint->classMask = binClassMask;
    pPoint->data.counter.value = value;
    sdnputil_getDateTime((TMWSESN*)pPoint->pSCLHandle, &pPoint->timeStamp);

    _callCallback((TMWSIM_POINT *)pPoint, TMWSIM_POINT_ADD, DNPDEFS_OBJ_20_RUNNING_CNTRS, 
      (TMWTYPES_USHORT)tmwsim_getPointNumber((TMWSIM_POINT *)pPoint));
  }

  return(pPoint);
}

/* function: sdnpsim_addFrozenCounter */
void * TMWDEFS_GLOBAL sdnpsim_addFrozenCounter(
  void *pHandle,
  TMWDEFS_CLASS_MASK binClassMask,
  TMWDEFS_CLASS_MASK frznClassMask,
  TMWTYPES_UCHAR flags,
  TMWTYPES_ULONG value)
{
  TMWSIM_POINT *pPoint;
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;
  TMWTYPES_USHORT pointNum = (TMWTYPES_USHORT)tmwsim_tableSize(&pDbHandle->frozenCounters);
  TMWTARG_UNUSED_PARAM(binClassMask);

  pPoint = tmwsim_tableAdd(&pDbHandle->frozenCounters, pointNum);
  if(pPoint != TMWDEFS_NULL)
  {
    tmwsim_initCounter(pPoint, pHandle, pointNum);

    pPoint->pSCLHandle = (void*)pDbHandle->pSDNPSession;
    pPoint->flags = flags; 
    pPoint->defaultStaticVariation = 9; /* 32 bit without flag */
    pPoint->defaultEventVariation = 1;  /* 32 bit with flag */
    pPoint->eventMode = (TMWTYPES_UCHAR)TMWDEFS_EVENT_MODE_SOE;
    pPoint->classMask = frznClassMask;
    pPoint->data.counter.value = value;
    sdnputil_getDateTime((TMWSESN*)pPoint->pSCLHandle, &pPoint->timeStamp);

    _callCallback((TMWSIM_POINT *)pPoint, TMWSIM_POINT_ADD, DNPDEFS_OBJ_21_FROZEN_CNTRS, 
      (TMWTYPES_USHORT)tmwsim_getPointNumber((TMWSIM_POINT *)pPoint));
  }

  return(pPoint);
}

/* function: sdnpsim_deleteBinaryCounter */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_deleteBinaryCounter(
  void *pHandle)
{
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;
  TMWTYPES_USHORT pointNumber = (TMWTYPES_USHORT)tmwsim_tableSize(&pDbHandle->binaryCounters);
  if(pointNumber > 0)
  {
    _cancelFreezeTimer(&pDbHandle->binaryCounters, pointNumber-1);
    _callRemoveCallback(pDbHandle, TMWSIM_POINT_DELETE, DNPDEFS_OBJ_20_RUNNING_CNTRS, (TMWTYPES_USHORT)(pointNumber-1));
    return(tmwsim_tableDelete(&pDbHandle->binaryCounters, pointNumber-1)); 
  }

  return(TMWDEFS_FALSE);
}

/* function: sdnpsim_deleteFrozenCounter */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_deleteFrozenCounter(
  void *pHandle)
{
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;
  TMWTYPES_USHORT pointNumber = (TMWTYPES_USHORT)tmwsim_tableSize(&pDbHandle->frozenCounters);
  if(pointNumber > 0)
  {  
    _callRemoveCallback(pDbHandle, TMWSIM_POINT_DELETE, DNPDEFS_OBJ_21_FROZEN_CNTRS, (TMWTYPES_USHORT)(pointNumber-1));
    return(tmwsim_tableDelete(&pDbHandle->frozenCounters, pointNumber-1));
  }

  return(TMWDEFS_FALSE);
}

/* function: sdnpsim_binCntrQuantity */
TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpsim_binCntrQuantity(
  void *pHandle)
{
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;
  return((TMWTYPES_USHORT)tmwsim_tableSize(&pDbHandle->binaryCounters));
}

/* function: sdnpsim_binaryCounterGetPoint */
void * TMWDEFS_GLOBAL sdnpsim_binaryCounterGetPoint(
  void *pHandle,
  TMWTYPES_USHORT pointNum)
{
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;
  return(tmwsim_tableFindPoint(&pDbHandle->binaryCounters, pointNum));
}

/* function: sdnpsim_frozenCounterGetPoint */
void * TMWDEFS_GLOBAL sdnpsim_frozenCounterGetPoint(
  void *pHandle,
  TMWTYPES_USHORT pointNum)
{
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;
  return(tmwsim_tableFindPoint(&pDbHandle->frozenCounters, pointNum));
}

/* function: sdnpsim_binCntrSetFlags */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_binCntrSetFlags(
  void *pPoint,
  TMWTYPES_UCHAR flags)
{
  TMWSIM_POINT *pSimPoint = (TMWSIM_POINT *)pPoint;
  tmwsim_setFlags(pSimPoint, flags, TMWDEFS_CHANGE_REMOTE_OP);
  sdnputil_getDateTime((TMWSESN*)pSimPoint->pSCLHandle, &pSimPoint->timeStamp);

  _callCallback(pSimPoint, TMWSIM_POINT_UPDATE, DNPDEFS_OBJ_20_RUNNING_CNTRS, 
    (TMWTYPES_USHORT)tmwsim_getPointNumber(pSimPoint));
 
  return(TMWDEFS_TRUE);
}

/* function: sdnpsim_binCntrGetEnabledPoint */
void * TMWDEFS_GLOBAL sdnpsim_binCntrGetEnabledPoint(
  void *pHandle,
  TMWTYPES_USHORT pointNum)
{
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;
  TMWSIM_POINT *pPoint = tmwsim_tableFindPoint(&pDbHandle->binaryCounters, pointNum);
  if((pPoint == TMWDEFS_NULL) || !pPoint->enabled)
    return(TMWDEFS_NULL);

  return(pPoint);
}

/* function: sdnpsim_binCntrDefVariation */
TMWTYPES_UCHAR sdnpsim_binCntrDefVariation(
  void *pPoint)
{
  return(tmwsim_getDefStaticVariation((TMWSIM_POINT *)pPoint));
}

/* function: sdnpsim_binCntrEventDefVariation */
TMWTYPES_UCHAR sdnpsim_binCntrEventDefVariation(
  void *pPoint,
  TMWDEFS_CLASS_MASK classMask)
{
  TMWTARG_UNUSED_PARAM(classMask);
  return(tmwsim_getDefEventVariation((TMWSIM_POINT *)pPoint));
}

/* function: sdnpsim_binCntrRead */
void TMWDEFS_GLOBAL sdnpsim_binCntrRead(
  void *pPoint,
  TMWTYPES_ULONG *pValue,
  TMWTYPES_UCHAR *pFlags)
{
  *pFlags = tmwsim_getFlags((TMWSIM_POINT *)pPoint);
  *pValue = tmwsim_getCounterValue((TMWSIM_POINT *)pPoint);
  return;
}

/* function: sdnpsim_binCntrChanged */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_binCntrChanged(
  void *pPoint,
  TMWTYPES_ULONG *pValue,
  TMWTYPES_UCHAR *pFlags)
{
  TMWDEFS_CHANGE_REASON reason;
  if(tmwsim_getChanged((TMWSIM_POINT *)pPoint, &reason))
  {
    *pFlags = tmwsim_getFlags((TMWSIM_POINT *)pPoint);
    *pValue = tmwsim_getCounterValue((TMWSIM_POINT *)pPoint);
    return(TMWDEFS_TRUE);
  }

  return(TMWDEFS_FALSE);
}

/* function: sdnpsim_binCntrFreeze */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_binCntrFreeze(
  void *pPoint,
  TMWTYPES_BOOL clearAfterFreeze)
{
  SDNPSIM_DATABASE *pDbHandle;
  TMWSIM_POINT *pFrozenCounterPoint;
  TMWTYPES_USHORT pointNumber;
  TMWSIM_POINT *pBinaryCounterPoint = (TMWSIM_POINT *)pPoint;

  pDbHandle = (SDNPSIM_DATABASE *)pBinaryCounterPoint->pDbHandle;
  pointNumber = (TMWTYPES_USHORT)pBinaryCounterPoint->pointNumber;

  pFrozenCounterPoint = (TMWSIM_POINT *)sdnpsim_frozenCounterGetPoint((void *)pDbHandle, pointNumber);

  /* See if this point has been set to fail for test purposes */
  if(tmwsim_getTestingMode(pBinaryCounterPoint) || tmwsim_getTestingMode(pFrozenCounterPoint))
    return(TMWDEFS_FALSE);

  pFrozenCounterPoint->data.counter.value = pBinaryCounterPoint->data.counter.value;
  sdnputil_getDateTime((TMWSESN*)pFrozenCounterPoint->pSCLHandle, &pFrozenCounterPoint->data.counter.timeOfFreeze);
  pFrozenCounterPoint->timeStamp = pFrozenCounterPoint->data.counter.timeOfFreeze;

#if SDNPDATA_SUPPORT_OBJ23
  /* Conformance test version 2.8 test 8.16.2.2.2 indicates if frozen counter event is supported the event should be generated even if the value did not change. */
  sdnpo023_addEvent((TMWSESN*)pFrozenCounterPoint->pSCLHandle, pointNumber, pFrozenCounterPoint->data.counter.value, tmwsim_getFlags(pFrozenCounterPoint), &pFrozenCounterPoint->data.counter.timeOfFreeze);
#endif

  pFrozenCounterPoint->data.counter.frozenValueChanged = TMWDEFS_TRUE;
  _callCallback(pFrozenCounterPoint, TMWSIM_POINT_UPDATE, DNPDEFS_OBJ_21_FROZEN_CNTRS, pointNumber);
   
  if(clearAfterFreeze)
    sdnpsim_binCntrWrite(pBinaryCounterPoint, 0);

  return(TMWDEFS_TRUE);
}

/* Calculates the number of milliseconds to initialize the tmwtimer with based on
 * the Time-date field of the freeze request.
 */
static TMWTYPES_BOOL _calcInitFreezeMs(
  TMWSIM_POINT *pPoint,
  DNPDATA_FREEZE_TIME_DATE_FIELD timeDateEnum,
  TMWDTIME *pFreezeTime,
  TMWTYPES_ULONG freezeInterval,
  TMWTYPES_ULONG *pInitFreezeMs)
{
  TMWDTIME now, frzTime;
  TMWTYPES_MS_SINCE_70 dnpNow, dnpFrzTime;
  TMWTYPES_MS_SINCE_70 firstInterval;

  sdnputil_getDateTime((TMWSESN*)pPoint->pSCLHandle, &now);
  dnpdtime_dateTimeToMSSince70(&dnpNow, &now);
  if (timeDateEnum == DNPDATA_FREEZE_TIME_DATE_FIELD_ZERO)
  {
    /* Periodically freeze at intervals relative to the beginning of the current hour. */
    frzTime = now;
    frzTime.minutes = 0;
    frzTime.mSecsAndSecs = 0;
    while (tmwdtime_compareTime(&frzTime, &now) <= 0)
    {
      tmwdtime_addOffset(&frzTime, freezeInterval);
    }
    dnpdtime_dateTimeToMSSince70(&dnpFrzTime, &frzTime);
  }
  else
  {
    /* Non-zero time, perform the initial freeze at the time specified. */
    if (tmwdtime_compareTime(pFreezeTime, &now) > 0)
    {
      dnpdtime_dateTimeToMSSince70(&dnpFrzTime, pFreezeTime);
    }
    else
    {
      /* If the time specified has elapsed, freeze now. */
      dnpFrzTime = dnpNow;
    }
  }
  dnpdtime_subtractTime(&firstInterval, &dnpFrzTime, &dnpNow);
  if (firstInterval.mostSignificant > 0xffff)
  {
    /* error interval too great. */
    return (TMWDEFS_FALSE);
  }
  *pInitFreezeMs = (firstInterval.mostSignificant << 16) | firstInterval.leastSignificant;
  return (TMWDEFS_TRUE);
}

static void TMWDEFS_CALLBACK _binCtrFreezeCallback(void *pPoint)
{
  TMWSIM_POINT *pBinaryCounterPoint = (TMWSIM_POINT *)pPoint;
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pBinaryCounterPoint->pDbHandle;

  sdnpsim_binCntrFreeze(pPoint, TMWDEFS_FALSE);
  if (pBinaryCounterPoint->freezeInterval)
  {
    tmwtimer_start(&pBinaryCounterPoint->freezeTimer, pBinaryCounterPoint->freezeInterval, pDbHandle->pSDNPSession->dnp.tmw.pChannel, _binCtrFreezeCallback, pPoint);
  }
}

/* function: sdnpsim_binCntrFreezeAtTime */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_binCntrFreezeAtTime(
  void *pPoint,
  DNPDATA_FREEZE_TIME_DATE_FIELD timeDateEnum,
  TMWDTIME *pFreezeTime,
  TMWTYPES_ULONG freezeInterval)
{
  TMWTYPES_ULONG initialFreezeDelay;
  TMWSIM_POINT *pBinaryCounterPoint = (TMWSIM_POINT *)pPoint;
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pBinaryCounterPoint->pDbHandle;

  if (timeDateEnum == DNPDATA_FREEZE_TIME_DATE_FIELD_CANCEL)
  {
    pBinaryCounterPoint->freezeEnabled = TMWDEFS_FALSE;
    tmwtimer_cancel(&pBinaryCounterPoint->freezeTimer);
    return (TMWDEFS_TRUE);
  }
  else 
  {
    if (_calcInitFreezeMs(pBinaryCounterPoint, timeDateEnum, pFreezeTime, freezeInterval, &initialFreezeDelay))
    {
      pBinaryCounterPoint->freezeEnabled = TMWDEFS_TRUE;
      pBinaryCounterPoint->freezeInterval = freezeInterval;
      tmwtimer_start(&pBinaryCounterPoint->freezeTimer, initialFreezeDelay, pDbHandle->pSDNPSession->dnp.tmw.pChannel, _binCtrFreezeCallback, pPoint);
      return (TMWDEFS_TRUE);
    }
  }
  return (TMWDEFS_FALSE);
}

/* function: sdnpsim_binCntrWrite */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_binCntrWrite(
  void *pPoint,
  TMWTYPES_ULONG value)
{
  TMWSIM_POINT *pSimPoint = (TMWSIM_POINT *)pPoint;
  tmwsim_setCounterValue(pSimPoint, value, TMWDEFS_CHANGE_LOCAL_OP);
  sdnputil_getDateTime((TMWSESN*)pSimPoint->pSCLHandle, &pSimPoint->timeStamp);

  _callCallback(pSimPoint, TMWSIM_POINT_UPDATE, DNPDEFS_OBJ_20_RUNNING_CNTRS, 
    (TMWTYPES_USHORT)tmwsim_getPointNumber(pSimPoint));

  return(TMWDEFS_TRUE);
}

/* function: sdnpsim_frznCntrQuantity */
TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpsim_frznCntrQuantity(
  void *pHandle)
{
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;
  return((TMWTYPES_USHORT)tmwsim_tableSize(&pDbHandle->frozenCounters));
}

/* function: sdnpsim_frznCntrSetFlags */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_frznCntrSetFlags(
  void *pPoint,
  TMWTYPES_UCHAR flags)
{
  TMWSIM_POINT *pSimPoint = (TMWSIM_POINT *)pPoint;

  tmwsim_setFlags(pSimPoint, flags, TMWDEFS_CHANGE_REMOTE_OP);
  sdnputil_getDateTime((TMWSESN*)pSimPoint->pSCLHandle, &pSimPoint->timeStamp);

  _callCallback(pSimPoint, TMWSIM_POINT_UPDATE, DNPDEFS_OBJ_21_FROZEN_CNTRS, 
    (TMWTYPES_USHORT)tmwsim_getPointNumber(pSimPoint));

  return(TMWDEFS_TRUE);
}

/* function: sdnpsim_frznCntrGetEnabledPoint */
void * TMWDEFS_GLOBAL sdnpsim_frznCntrGetEnabledPoint(
  void *pHandle,
  TMWTYPES_USHORT pointNum)
{
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;
  TMWSIM_POINT *pPoint = tmwsim_tableFindPoint(&pDbHandle->frozenCounters, pointNum);
  if((pPoint == TMWDEFS_NULL) || !pPoint->enabled)
    return(TMWDEFS_NULL);

  return(pPoint);
}

/* function: sdnpsim_frznCntrGetEventClass */
TMWDEFS_CLASS_MASK TMWDEFS_GLOBAL sdnpsim_frznCntrGetEventClass(
  void *pPoint)
{
  return(((TMWSIM_POINT *)pPoint)->classMask);
}

/* function: sdnpsim_frznCntrSetEventClass */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_frznCntrSetEventClass(
  void *pPoint,
  TMWDEFS_CLASS_MASK classMask)
{
  /* See if this point has been set to fail for test purposes */
  if(tmwsim_getTestingMode((TMWSIM_POINT *)pPoint))
    return(TMWDEFS_FALSE);

  ((TMWSIM_POINT *)pPoint)->classMask = classMask;
  return(TMWDEFS_TRUE);
}

/* function: sdnpsim_frznCntrInClass0 */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_frznCntrInClass0(
   void *pPoint)
{
  if((((TMWSIM_POINT *)pPoint)->classMask & TMWDEFS_CLASS_MASK_NOTCLASS0) == 0)
    return TMWDEFS_TRUE;
  else
    return TMWDEFS_FALSE;
}


/* function: sdnpsim_frznCntrDefVariation */
TMWTYPES_UCHAR sdnpsim_frznCntrDefVariation(
  void *pPoint)
{
  return(tmwsim_getDefStaticVariation((TMWSIM_POINT *)pPoint));
}

/* function: sdnpsim_frznCntrEventDefVariation */
TMWTYPES_UCHAR sdnpsim_frznCntrEventDefVariation(
  void *pPoint,
  TMWDEFS_CLASS_MASK classMask)
{
  TMWTARG_UNUSED_PARAM(classMask);
  return(tmwsim_getDefEventVariation((TMWSIM_POINT *)pPoint));
}

/* function: sdnpsim_frznCntrRead */
void TMWDEFS_GLOBAL sdnpsim_frznCntrRead(
  void *pPoint,
  TMWTYPES_ULONG *pValue,
  TMWTYPES_UCHAR *pFlags,
  TMWDTIME *pTimeOfFreeze)
{
  *pFlags = tmwsim_getFlags((TMWSIM_POINT *)pPoint);
  *pValue = ((TMWSIM_POINT *)pPoint)->data.counter.value;
  *pTimeOfFreeze = ((TMWSIM_POINT *)pPoint)->data.counter.timeOfFreeze;
  return;
}

/* function: sdnpsim_frznCntrChanged */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_frznCntrChanged(
  void *pPoint,
  TMWTYPES_ULONG *pValue,
  TMWTYPES_UCHAR *pFlags)
{
  if(((TMWSIM_POINT *)pPoint)->data.counter.frozenValueChanged)
  {
    *pFlags = tmwsim_getFlags((TMWSIM_POINT *)pPoint);
    *pValue = ((TMWSIM_POINT *)pPoint)->data.counter.value;
    ((TMWSIM_POINT *)pPoint)->data.counter.frozenValueChanged = TMWDEFS_FALSE;
    return(TMWDEFS_TRUE);
  }

  return(TMWDEFS_FALSE);
}

/* function: sdnpsim_addAnalogInput */
void * TMWDEFS_GLOBAL sdnpsim_addAnalogInput(
  void *pHandle,
  TMWDEFS_CLASS_MASK classMask,
  TMWTYPES_UCHAR flags,
  TMWSIM_DATA_TYPE value,
  TMWSIM_DATA_TYPE deadband)
{
 TMWSIM_POINT *pPoint;
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;

  TMWTYPES_USHORT pointNum = (TMWTYPES_USHORT)tmwsim_tableSize(&pDbHandle->analogInputs);
  pPoint = tmwsim_tableAdd(&pDbHandle->analogInputs, pointNum);
  if(pPoint != TMWDEFS_NULL)
  {
    tmwsim_initAnalog(pPoint, pHandle, pointNum, TMWSIM_DATA_MIN, TMWSIM_DATA_MAX, deadband, 0);

    pPoint->pSCLHandle = (void*)pDbHandle->pSDNPSession;
    pPoint->classMask = classMask;
    pPoint->data.analog.value = value;
    pPoint->flags = flags;
    pPoint->defaultStaticVariation = 3;
    pPoint->defaultEventVariation = 1;
    pPoint->eventMode = (TMWTYPES_UCHAR)TMWDEFS_EVENT_MODE_MOST_RECENT;
    pPoint->data.analog.defaultDeadbandVariation = 2;
    sdnputil_getDateTime((TMWSESN*)pPoint->pSCLHandle, &pPoint->timeStamp);

    _callCallback((TMWSIM_POINT *)pPoint, TMWSIM_POINT_ADD, DNPDEFS_OBJ_30_ANA_INPUTS, 
      (TMWTYPES_USHORT)tmwsim_getPointNumber((TMWSIM_POINT *)pPoint));
  }

  return(pPoint);
}  
   
/* function: sdnpsim_deleteAnalogInput */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_deleteAnalogInput(
  void *pHandle)
{
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;
  TMWTYPES_USHORT pointNumber = (TMWTYPES_USHORT)tmwsim_tableSize(&pDbHandle->analogInputs);
  if(pointNumber > 0)
  {
    _cancelFreezeTimer(&pDbHandle->analogInputs, pointNumber-1);
    _callRemoveCallback(pDbHandle, TMWSIM_POINT_DELETE, DNPDEFS_OBJ_30_ANA_INPUTS, 
      (TMWTYPES_USHORT)(pointNumber-1)); 

    return(tmwsim_tableDelete(&pDbHandle->analogInputs, pointNumber-1));
  }

  return(TMWDEFS_FALSE);
}
 
/* function: sdnpsim_anlgInQuantity */
TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpsim_anlgInQuantity(
  void *pHandle)
{
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;
  return((TMWTYPES_USHORT)tmwsim_tableSize(&pDbHandle->analogInputs));
}

/* function: sdnpsim_anlgInGetPoint */
void * TMWDEFS_GLOBAL sdnpsim_anlgInGetPoint(
  void *pHandle,
  TMWTYPES_USHORT pointNum)
{
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;
  return(tmwsim_tableFindPoint(&pDbHandle->analogInputs, pointNum));
}

/* function: sdnpsim_anlgInSetFlags */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_anlgInSetFlags(
  void *pPoint,
  TMWTYPES_UCHAR flags)
{
  TMWSIM_POINT *pSimPoint = (TMWSIM_POINT *)pPoint;

  tmwsim_setFlags(pSimPoint, flags, TMWDEFS_CHANGE_REMOTE_OP);
  sdnputil_getDateTime((TMWSESN*)pSimPoint->pSCLHandle, &pSimPoint->timeStamp);

  _callCallback(pSimPoint, TMWSIM_POINT_UPDATE, DNPDEFS_OBJ_30_ANA_INPUTS, 
    (TMWTYPES_USHORT)tmwsim_getPointNumber(pSimPoint));

  return(TMWDEFS_TRUE);
}

/* function: sdnpsim_anlgInGetEnabledPoint */
void * TMWDEFS_GLOBAL sdnpsim_anlgInGetEnabledPoint(
  void *pHandle,
  TMWTYPES_USHORT pointNum)
{
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;
  TMWSIM_POINT *pPoint = tmwsim_tableFindPoint(&pDbHandle->analogInputs, pointNum);
  if((pPoint == TMWDEFS_NULL) || !pPoint->enabled)
  {
    return(TMWDEFS_NULL);
  }
  return(pPoint);
}

/* function: sdnpsim_anlgInDefVariation */
TMWTYPES_UCHAR sdnpsim_anlgInDefVariation(
  void *pPoint)
{
  return(tmwsim_getDefStaticVariation((TMWSIM_POINT *)pPoint));
}

/* function: sdnpsim_anlgInDbandDefVar */
TMWTYPES_UCHAR sdnpsim_anlgInDbandDefVar(
  void *pPoint)
{
  return(tmwsim_getDefDeadbandVariation((TMWSIM_POINT *)pPoint));
}

/* function: sdnpsim_anlgInEventDefVariation */
TMWTYPES_UCHAR sdnpsim_anlgInEventDefVariation(
  void *pPoint,
  TMWDEFS_CLASS_MASK classMask)
{
  TMWTARG_UNUSED_PARAM(classMask);
  return(tmwsim_getDefEventVariation((TMWSIM_POINT *)pPoint));
}


/* function: sdnpsim_anlgRead */
void TMWDEFS_GLOBAL sdnpsim_anlgRead(
  void *pPoint,
  TMWTYPES_ANALOG_VALUE *pValue,
  TMWTYPES_UCHAR *pFlags)
{
  *pFlags = tmwsim_getFlags((TMWSIM_POINT *)pPoint);

#if TMWCNFG_SUPPORT_DOUBLE 
  pValue->type = TMWTYPES_ANALOG_TYPE_DOUBLE;
  pValue->value.dval = tmwsim_getAnalogValue((TMWSIM_POINT *)pPoint);

  /* sample code for rounding to nearest integer
   * {
   *  TMWTYPES_DOUBLE dbValue;
   *  dbValue = tmwsim_getAnalogValue((TMWSIM_POINT *)pPoint);
   *  if(dbValue > TMWDEFS_LONG_MAX 
   *    || dbValue < TMWDEFS_LONG_MIN)
   *  {
   *    pValue->type = TMWTYPES_ANALOG_TYPE_DOUBLE;
   *    pValue->value.dval = dbValue;
   *  }
   *  else
   *  {
   *    pValue->type = TMWTYPES_ANALOG_TYPE_DSCALED;
   *    pValue->value.dscaled.dval = dbValue;
   *    if(pValue->value.dscaled.dval >= 0)
   *    {
   *      pValue->value.dscaled.lval = (TMWTYPES_LONG)(dbValue + .5);
   *    }
   *    else
   *    {
   *      pValue->value.dscaled.lval = (TMWTYPES_LONG)(dbValue - .5); 
   *    }
   *  }
   * }
   */
#elif TMWCNFG_SUPPORT_FLOAT
  {
   TMWSIM_DATA_TYPE temp;
   temp = tmwsim_getAnalogValue((TMWSIM_POINT *)pPoint);

   /* If this value won't fit in a LONG, put it in an SFLOAT */
   if((temp < (TMWTYPES_SFLOAT)TMWDEFS_LONG_MIN)
     ||(temp > (TMWTYPES_SFLOAT)TMWDEFS_LONG_MAX))
   {
     pValue->type = TMWTYPES_ANALOG_TYPE_SFLOAT;
     /* Make sure it fits in an SFLOAT */
     if(temp <= TMWDEFS_SFLOAT_MIN)
     {
       pValue->value.fval = TMWDEFS_SFLOAT_MIN;
       *pFlags |= DNPDEFS_DBAS_FLAG_OVER_RANGE;
     }
     else if(temp > TMWDEFS_SFLOAT_MAX)
     {
       pValue->value.fval = TMWDEFS_SFLOAT_MAX;
       *pFlags |= DNPDEFS_DBAS_FLAG_OVER_RANGE;
     }
     else
     {
       pValue->value.fval = (TMWTYPES_SFLOAT)temp;
     }
   }
   else
   {
     pValue->type = TMWTYPES_ANALOG_TYPE_SFLOAT; 
     pValue->value.fval = (TMWTYPES_SFLOAT)temp;
   }
  }
  #else
  {
    TMWTYPES_LONG temp;
    pValue->type = TMWTYPES_ANALOG_TYPE_LONG; 
    temp = (TMWTYPES_LONG)tmwsim_getAnalogValue((TMWSIM_POINT *)pPoint);
    pValue->value.lval = temp;
    if(temp < TMWDEFS_LONG_MIN)
    {
      pValue->value.lval = TMWDEFS_LONG_MIN;
      *pFlags |= DNPDEFS_DBAS_FLAG_OVER_RANGE;
    }
    else if(temp > TMWDEFS_LONG_MAX)
    {
      pValue->value.lval = TMWDEFS_LONG_MAX;
      *pFlags |= DNPDEFS_DBAS_FLAG_OVER_RANGE;
    }
  }
#endif
}

/* function: sdnpsim_anlgInRead */
void TMWDEFS_GLOBAL sdnpsim_anlgInRead(
  void *pPoint,
  TMWTYPES_ANALOG_VALUE *pValue,
  TMWTYPES_UCHAR *pFlags)
{
  sdnpsim_anlgRead(pPoint, pValue, pFlags);
}

/* function: sdnpsim_anlgInChanged */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_anlgInChanged(
  void *pPoint,
  TMWTYPES_ANALOG_VALUE *pValue,
  TMWTYPES_UCHAR *pFlags)
{
  TMWDEFS_CHANGE_REASON reason;
  if(tmwsim_getChanged((TMWSIM_POINT *)pPoint, &reason))
  {
    sdnpsim_anlgRead(pPoint, pValue, pFlags);   
    return(TMWDEFS_TRUE);
  }

  return(TMWDEFS_FALSE);
}

/* function: sdnpsim_anlgInFreeze */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_anlgInFreeze(
  void *pPoint,
  TMWTYPES_BOOL clearAfterFreeze)
{
  SDNPSIM_DATABASE *pDbHandle;
  TMWSIM_POINT *pFrozenAnalogInPoint;
  TMWTYPES_USHORT pointNumber;
  TMWSIM_POINT *pAnalogInPoint = (TMWSIM_POINT *)pPoint;

  pDbHandle = (SDNPSIM_DATABASE *)pAnalogInPoint->pDbHandle;
  pointNumber = (TMWTYPES_USHORT)pAnalogInPoint->pointNumber;

  pFrozenAnalogInPoint = (TMWSIM_POINT *)sdnpsim_frznAnlgInGetPoint((void *)pDbHandle, pointNumber);
  /* If frozen analog point does not exist or is disabled, the freeze will fail. */
  if ((pFrozenAnalogInPoint == TMWDEFS_NULL) || !pFrozenAnalogInPoint->enabled)
  {
    return(TMWDEFS_FALSE);
  }

  /* See if this point has been set to fail for test purposes */
  if(tmwsim_getTestingMode(pAnalogInPoint) || tmwsim_getTestingMode(pFrozenAnalogInPoint))
    return(TMWDEFS_FALSE);

  tmwsim_setAnalogValue(pFrozenAnalogInPoint, pAnalogInPoint->data.analog.value, TMWDEFS_CHANGE_LOCAL_OP);
  sdnputil_getDateTime((TMWSESN*)pFrozenAnalogInPoint->pSCLHandle, &pFrozenAnalogInPoint->timeStamp);
   
#if SDNPDATA_SUPPORT_OBJ33 
  {
    TMWTYPES_ANALOG_VALUE anlgValue;
    anlgValue.type = TMWTYPES_ANALOG_TYPE_DOUBLE;
    anlgValue.value.dval = pAnalogInPoint->data.analog.value;
    sdnpo033_addEvent((TMWSESN*)pFrozenAnalogInPoint->pSCLHandle, pointNumber, &anlgValue, tmwsim_getFlags(pFrozenAnalogInPoint), &pFrozenAnalogInPoint->timeStamp);
  }
#endif

  _callCallback(pFrozenAnalogInPoint, TMWSIM_POINT_UPDATE, DNPDEFS_OBJ_31_FRZN_ANA_INPUTS, pointNumber);


  /* IEEE 1815-2012 Section 11.9.1.2 states:
   * With a freeze-and-clear type operation the underlying analog input value is preset to a suitable,
   * possibly non-zero number after its value is copied to the frozen analog variable.
   *
   * For simplicity the TMW simulated database implementation simply sets the value to zero.
   */
  if(clearAfterFreeze)
    sdnpsim_anlgInWrite(pAnalogInPoint, 0);
   
  return(TMWDEFS_TRUE);
}

static void TMWDEFS_CALLBACK _anlgInCtrFreezeCallback(void *pPoint)
{
  TMWSIM_POINT *pAnalogInPoint = (TMWSIM_POINT *)pPoint;
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pAnalogInPoint->pDbHandle;

  sdnpsim_anlgInFreeze(pPoint, TMWDEFS_FALSE);
  if (pAnalogInPoint->freezeInterval)
  {
    tmwtimer_start(&pAnalogInPoint->freezeTimer, pAnalogInPoint->freezeInterval, pDbHandle->pSDNPSession->dnp.tmw.pChannel, _anlgInCtrFreezeCallback, pPoint);
  }
}

/* function: sdnpsim_anlgInFreezeAtTime */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_anlgInFreezeAtTime(
  void *pPoint,
  DNPDATA_FREEZE_TIME_DATE_FIELD timeDateEnum,
  TMWDTIME *pFreezeTime,
  TMWTYPES_ULONG freezeInterval)
{
  TMWTYPES_ULONG initialFreezeDelay;
  TMWSIM_POINT *pAnalogInPoint = (TMWSIM_POINT *)pPoint;
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pAnalogInPoint->pDbHandle;

  if (timeDateEnum == DNPDATA_FREEZE_TIME_DATE_FIELD_CANCEL)
  {
    pAnalogInPoint->freezeEnabled = TMWDEFS_FALSE;
    tmwtimer_cancel(&pAnalogInPoint->freezeTimer);
    return (TMWDEFS_TRUE);
  }
  else 
  {
    if (_calcInitFreezeMs(pAnalogInPoint, timeDateEnum, pFreezeTime, freezeInterval, &initialFreezeDelay))
    {
      pAnalogInPoint->freezeEnabled = TMWDEFS_TRUE;
      pAnalogInPoint->freezeInterval = freezeInterval;
      tmwtimer_start(&pAnalogInPoint->freezeTimer, initialFreezeDelay, pDbHandle->pSDNPSession->dnp.tmw.pChannel, _anlgInCtrFreezeCallback, pPoint);
      return (TMWDEFS_TRUE);
    }
  }
  return (TMWDEFS_FALSE);
}

/* function: sdnpsim_anlgInWrite */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_anlgInWrite(
  void *pPoint,
  TMWSIM_DATA_TYPE value)
{
  TMWSIM_POINT *pSimPoint = (TMWSIM_POINT *)pPoint;
  tmwsim_setAnalogValue(pSimPoint, value, TMWDEFS_CHANGE_LOCAL_OP);
  sdnputil_getDateTime((TMWSESN*)pSimPoint->pSCLHandle, &pSimPoint->timeStamp);

  _callCallback(pSimPoint, TMWSIM_POINT_UPDATE, DNPDEFS_OBJ_30_ANA_INPUTS, 
    (TMWTYPES_USHORT)tmwsim_getPointNumber(pSimPoint));
 
  return(TMWDEFS_TRUE);
}

/* function: sdnpsim_anlgInGetDBand */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_anlgInGetDBand(
  void *pPoint,
  TMWTYPES_ANALOG_VALUE *pDeadbandValue)
{
#if TMWCNFG_SUPPORT_DOUBLE
  pDeadbandValue->type = TMWTYPES_ANALOG_TYPE_DOUBLE;
  pDeadbandValue->value.dval = tmwsim_getAnalogDeadband((TMWSIM_POINT *)pPoint);
#elif TMWCNFG_SUPPORT_FLOAT  
  {
    TMWSIM_DATA_TYPE temp;
    temp = tmwsim_getAnalogDeadband((TMWSIM_POINT *)pPoint);

    /* If this value won't fit in a ULONG, put it in an SFLOAT */
    if(temp > (TMWTYPES_SFLOAT)TMWDEFS_ULONG_MAX)
    {
      pDeadbandValue->type = TMWTYPES_ANALOG_TYPE_SFLOAT;

      /* Make sure it fits in an SFLOAT */
      if(temp > TMWDEFS_SFLOAT_MAX)
      {
        pDeadbandValue->value.fval = TMWDEFS_SFLOAT_MAX;
      }
      else
      {
        pDeadbandValue->value.fval = (TMWTYPES_SFLOAT)temp;
      }
    }
    else
    {
      pDeadbandValue->type = TMWTYPES_ANALOG_TYPE_ULONG; 
      pDeadbandValue->value.ulval = (TMWTYPES_ULONG)temp;
    }
  }
#else
  pDeadbandValue->type = TMWTYPES_ANALOG_TYPE_ULONG;
  pDeadbandValue->value.ulval = (TMWTYPES_ULONG)tmwsim_getAnalogDeadband((TMWSIM_POINT *)pPoint);
#endif
  return(TMWDEFS_TRUE);
}

/* function: sdnpsim_anlgInSetDBand */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_anlgInSetDBand(
  void *pPoint,
  TMWTYPES_ANALOG_VALUE *pDeadbandValue)
{
  TMWSIM_POINT *pSimPoint = (TMWSIM_POINT *)pPoint;

  /* See if this point has been set to fail for test purposes */
  if(tmwsim_getTestingMode(pSimPoint))
    return(TMWDEFS_FALSE);

  switch(pDeadbandValue->type)
  {
#if TMWCNFG_SUPPORT_DOUBLE
  case TMWTYPES_ANALOG_TYPE_DOUBLE:
  case TMWTYPES_ANALOG_TYPE_SFLOAT:
    pSimPoint->data.analog.deadband = dnputil_getAnlgDBandValueDouble(pDeadbandValue);
    break;

#elif TMWCNFG_SUPPORT_FLOAT
  case TMWTYPES_ANALOG_TYPE_SFLOAT:
    ((TMWSIM_POINT *)pPoint)->data.analog.deadband = dnputil_getAnlgDBandValueFloat(pDeadbandValue);
    break;
#endif

  case TMWTYPES_ANALOG_TYPE_ULONG:
    pSimPoint->data.analog.deadband = (TMWSIM_DATA_TYPE)dnputil_getAnlgDBandValueULong(pDeadbandValue);
    break;

  case TMWTYPES_ANALOG_TYPE_USHORT:
    pSimPoint->data.analog.deadband = dnputil_getAnlgDBandValueUShort(pDeadbandValue);
    break;

  default:
    /* Other types are not supported by DNP and are not used. */
    pSimPoint->data.analog.deadband = 0;
    break;
  }

  sdnputil_getDateTime((TMWSESN*)pSimPoint->pSCLHandle, &pSimPoint->timeStamp);
  return(TMWDEFS_TRUE);
}

/* Frozen Analog Inputs */

/* function: sdnpsim_addfrznAnlgInput */
void * TMWDEFS_GLOBAL sdnpsim_addfrznAnlgInput(
  void *pHandle,
  TMWDEFS_CLASS_MASK classMask,
  TMWTYPES_UCHAR flags,
  TMWSIM_DATA_TYPE value)
{
  TMWSIM_POINT *pPoint;
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;
  TMWTYPES_USHORT pointNum = (TMWTYPES_USHORT)tmwsim_tableSize(&pDbHandle->frozenAnalogInputs);

  pPoint = tmwsim_tableAdd(&pDbHandle->frozenAnalogInputs, pointNum);
  if(pPoint != TMWDEFS_NULL)
  {
    tmwsim_initAnalog(pPoint, pHandle, pointNum, TMWSIM_DATA_MIN, TMWSIM_DATA_MAX, 0, 0);

    pPoint->pSCLHandle = (void*)pDbHandle->pSDNPSession;
    pPoint->flags = flags; 
    pPoint->defaultStaticVariation = 5; /* 32 bit without flag */
    pPoint->defaultEventVariation = 1;  /* 32 bit with flag */
    pPoint->eventMode = (TMWTYPES_UCHAR)TMWDEFS_EVENT_MODE_SOE;
    pPoint->classMask = classMask;
    pPoint->data.analog.value = value;
    sdnputil_getDateTime((TMWSESN*)pPoint->pSCLHandle, &pPoint->timeStamp);

    _callCallback((TMWSIM_POINT *)pPoint, TMWSIM_POINT_ADD, DNPDEFS_OBJ_31_FRZN_ANA_INPUTS, 
      (TMWTYPES_USHORT)tmwsim_getPointNumber((TMWSIM_POINT *)pPoint));
  }

  return(pPoint);
}

/* function: sdnpsim_deleteFrznAnlgInput */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_deleteFrznAnlgInput(
  void *pHandle)
{
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;
  TMWTYPES_USHORT pointNumber = sdnpsim_frznAnlgInQuantity(pDbHandle);
  TMWTYPES_BOOL tableDeleted = TMWDEFS_FALSE;
  if(pointNumber > 0)
  { 
    _callRemoveCallback(pDbHandle, TMWSIM_POINT_DELETE, DNPDEFS_OBJ_31_FRZN_ANA_INPUTS, (pointNumber-1)); 
    tableDeleted = tmwsim_tableDelete(&pDbHandle->frozenAnalogInputs, pointNumber-1);
  }
  return(tableDeleted);
}

/* function: sdnpsim_frznAnlgInQuantity */
TMWDEFS_SCL_API TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpsim_frznAnlgInQuantity(
  void *pHandle)
{
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;
  return((TMWTYPES_USHORT)tmwsim_tableSize(&pDbHandle->frozenAnalogInputs));
}

/* function: sdnpsim_frznAnlgInGetPoint */
void * TMWDEFS_GLOBAL sdnpsim_frznAnlgInGetPoint(
  void *pHandle,
  TMWTYPES_USHORT pointNum)
{
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;
  return(tmwsim_tableFindPoint(&pDbHandle->frozenAnalogInputs, pointNum));
}

/* function: sdnpsim_frznAnlgInDefVariation */
TMWTYPES_UCHAR sdnpsim_frznAnlgInDefVariation(
  void *pPoint)
{
  return(tmwsim_getDefStaticVariation((TMWSIM_POINT *)pPoint));
}

TMWTYPES_UCHAR sdnpsim_frznAnlgInEventDefVariation(
  void *pPoint,
  TMWDEFS_CLASS_MASK classMask)
{
  TMWTARG_UNUSED_PARAM(classMask);
  return(tmwsim_getDefEventVariation((TMWSIM_POINT *)pPoint));
}

/* function: sdnpsim_frznAnlgInSetFlags */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_frznAnlgInSetFlags(
  void *pPoint,
  TMWTYPES_UCHAR flags)
{
  TMWSIM_POINT *pSimPoint = (TMWSIM_POINT *)pPoint;

  tmwsim_setFlags(pSimPoint, flags, TMWDEFS_CHANGE_REMOTE_OP);
  sdnputil_getDateTime((TMWSESN*)pSimPoint->pSCLHandle, &pSimPoint->timeStamp);

  _callCallback(pSimPoint, TMWSIM_POINT_UPDATE, DNPDEFS_OBJ_31_FRZN_ANA_INPUTS, 
    (TMWTYPES_USHORT)tmwsim_getPointNumber(pSimPoint));

  return(TMWDEFS_TRUE);
}

/* function: sdnpsim_frznAnlgInGetEnabledPoint */
void * TMWDEFS_GLOBAL sdnpsim_frznAnlgInGetEnabledPoint(
  void *pHandle,
  TMWTYPES_USHORT pointNum)
{
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;
  TMWSIM_POINT *pPoint = tmwsim_tableFindPoint(&pDbHandle->frozenAnalogInputs, pointNum);
  if((pPoint == TMWDEFS_NULL) || !pPoint->enabled)
    return(TMWDEFS_NULL);

  return(pPoint);
}

/* function: sdnpsim_frznAnlgInRead */
void TMWDEFS_GLOBAL sdnpsim_frznAnlgInRead(
  void *pPoint,
  TMWTYPES_ANALOG_VALUE *pValue,
  TMWTYPES_UCHAR *pFlags,
  TMWDTIME *pTimeOfFreeze)
{
  sdnpsim_anlgRead(pPoint, pValue, pFlags);
  *pTimeOfFreeze = ((TMWSIM_POINT *)pPoint)->timeStamp;
  return;
}

/* function: sdnpsim_frznAnlgInChanged */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_frznAnlgInChanged(
  void *pPoint,
  TMWTYPES_ANALOG_VALUE *pValue,
  TMWTYPES_UCHAR *pFlags)
{
  TMWDEFS_CHANGE_REASON reason;
  if (tmwsim_getChanged((TMWSIM_POINT *)pPoint, &reason))
  {
    sdnpsim_anlgRead(pPoint, pValue, pFlags);
    return(TMWDEFS_TRUE);
  }

  return(TMWDEFS_FALSE);
}

/* function: sdnpsim_frznAnlgInWrite */
/*  This is NOT called from the SDNP library but allows value to be restored from a workspace or csv file */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_frznAnlgInWrite(
  void *pPoint,
  TMWSIM_DATA_TYPE value)
{
  TMWSIM_POINT *pSimPoint = (TMWSIM_POINT *)pPoint;
  tmwsim_setAnalogValue(pSimPoint, value, TMWDEFS_CHANGE_LOCAL_OP);
  sdnputil_getDateTime((TMWSESN*)pSimPoint->pSCLHandle, &pSimPoint->timeStamp);

  _callCallback(pSimPoint, TMWSIM_POINT_UPDATE, DNPDEFS_OBJ_31_FRZN_ANA_INPUTS,
    (TMWTYPES_USHORT)tmwsim_getPointNumber(pSimPoint));

  return(TMWDEFS_TRUE);
}

/* function: sdnpsim_frznAnlgInDefEventVariation */
TMWTYPES_UCHAR sdnpsim_frznAnlgInDefEventVariation(
  void *pPoint,
  TMWDEFS_CLASS_MASK classMask)
{
  TMWTARG_UNUSED_PARAM(classMask);
  return(tmwsim_getDefEventVariation((TMWSIM_POINT *)pPoint));
}


/* function: sdnpsim_addAnalogOutput */
void * TMWDEFS_GLOBAL sdnpsim_addAnalogOutput(
  void *pHandle,
  TMWDEFS_CLASS_MASK classMask,
  TMWTYPES_UCHAR flags,
  TMWSIM_DATA_TYPE value)
{
  TMWSIM_POINT *pPoint;
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;

  TMWTYPES_USHORT pointNum = (TMWTYPES_USHORT)tmwsim_tableSize(&pDbHandle->analogOutputs);
  pPoint = tmwsim_tableAdd(&pDbHandle->analogOutputs, pointNum);
  if(pPoint != TMWDEFS_NULL)
  {
    tmwsim_initAnalog(pPoint, pHandle,
      pointNum,
      TMWSIM_DATA_MIN, TMWSIM_DATA_MAX, 0, 0);

    pPoint->pSCLHandle = (void*)pDbHandle->pSDNPSession;
    pPoint->data.analog.value = value;
    pPoint->classMask = classMask;
    pPoint->flags = flags;
    pPoint->defaultStaticVariation = 2;
    pPoint->defaultEventVariation = 2;
    pPoint->eventMode = (TMWTYPES_UCHAR)TMWDEFS_EVENT_MODE_SOE;
    sdnputil_getDateTime((TMWSESN*)pPoint->pSCLHandle, &pPoint->timeStamp);

    _callCallback((TMWSIM_POINT *)pPoint, TMWSIM_POINT_ADD, DNPDEFS_OBJ_40_ANA_OUT_STATUSES, 
      (TMWTYPES_USHORT)tmwsim_getPointNumber((TMWSIM_POINT *)pPoint));
  }

  return(pPoint);
}

/* function: sdnpsim_deleteAnalogOutput */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_deleteAnalogOutput(
  void *pHandle)
{
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;
  TMWTYPES_USHORT pointNumber = (TMWTYPES_USHORT)tmwsim_tableSize(&pDbHandle->analogOutputs);
  if(pointNumber > 0)
  {
    _callRemoveCallback(pDbHandle, TMWSIM_POINT_DELETE, DNPDEFS_OBJ_40_ANA_OUT_STATUSES,  
      (TMWTYPES_USHORT)(pointNumber-1)); 

    return(tmwsim_tableDelete(&pDbHandle->analogOutputs, pointNumber-1));
  }

  return(TMWDEFS_FALSE);
}

/* function: sdnpsim_anlgOutQuantity */
TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpsim_anlgOutQuantity(
  void *pHandle)
{
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;
  return((TMWTYPES_USHORT)tmwsim_tableSize(&pDbHandle->analogOutputs));
}

/* function: sdnpsim_anlgOutGetPoint */
void * TMWDEFS_GLOBAL sdnpsim_anlgOutGetPoint(
  void *pHandle,
  TMWTYPES_USHORT pointNum)
{
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;
  return(tmwsim_tableFindPoint(&pDbHandle->analogOutputs, pointNum));
}

/* function: sdnpsim_anlgOutSetFlags */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_anlgOutSetFlags(
  void *pPoint,
  TMWTYPES_UCHAR flags)
{
  TMWSIM_POINT *pSimPoint = (TMWSIM_POINT *)pPoint;
  tmwsim_setFlags(pSimPoint, flags, TMWDEFS_CHANGE_REMOTE_OP);
  sdnputil_getDateTime((TMWSESN*)pSimPoint->pSCLHandle, &pSimPoint->timeStamp);

  _callCallback(pSimPoint, TMWSIM_POINT_UPDATE, DNPDEFS_OBJ_40_ANA_OUT_STATUSES, 
    (TMWTYPES_USHORT)tmwsim_getPointNumber(pSimPoint));

  return(TMWDEFS_TRUE);
}

/* function: sdnpsim_anlgOutGetEnabledPoint */
void * TMWDEFS_GLOBAL sdnpsim_anlgOutGetEnabledPoint(
  void *pHandle,
  TMWTYPES_USHORT pointNum)
{
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;
  TMWSIM_POINT *pPoint = tmwsim_tableFindPoint(&pDbHandle->analogOutputs, pointNum);
  if((pPoint == TMWDEFS_NULL) || !pPoint->enabled)
    return(TMWDEFS_NULL);

  return(pPoint);
}  

/* function: sdnpsim_anlgOutDefVariation */
TMWTYPES_UCHAR sdnpsim_anlgOutDefVariation(
  void *pPoint)
{
  return(tmwsim_getDefStaticVariation((TMWSIM_POINT *)pPoint));
}

#if (SDNPDATA_SUPPORT_OBJ42 || SDNPDATA_SUPPORT_OBJ43)
/* function: sdnpsim_anlgOutEventDefVariation */
TMWTYPES_UCHAR sdnpsim_anlgOutEventDefVariation(
  void *pPoint,
  TMWDEFS_CLASS_MASK classMask)
{
  TMWTARG_UNUSED_PARAM(classMask);
  return(tmwsim_getDefEventVariation((TMWSIM_POINT *)pPoint));
}
#endif

#if SDNPDATA_SUPPORT_OBJ42
/* function: sdnpsim_anlgOutChanged */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_anlgOutChanged(
  void *pPoint,
  TMWTYPES_ANALOG_VALUE *pValue,
  TMWTYPES_UCHAR *pFlags)
{
  TMWDEFS_CHANGE_REASON reason;
  if (tmwsim_getChanged((TMWSIM_POINT *)pPoint, &reason))
  {
    sdnpsim_anlgRead(pPoint, pValue, pFlags);
    return(TMWDEFS_TRUE);
  }

  return(TMWDEFS_FALSE);
}
#endif

/* function: sdnpsim_anlgOutRead */
void TMWDEFS_GLOBAL sdnpsim_anlgOutRead(
  void *pPoint,
  TMWTYPES_ANALOG_VALUE *pValue,
  TMWTYPES_UCHAR *pFlags)
{
  sdnpsim_anlgRead(pPoint, pValue, pFlags);

  /* If the device or this point are in localMode clear the ONLINE flag */
  if (_dbLocalMode(pPoint)
    || (tmwsim_getLocal((TMWSIM_POINT *)pPoint)))
  {
    /* Clear ONLINE bit */
    *pFlags = *pFlags & ~DNPDEFS_DBAS_FLAG_ON_LINE;
  }
}

/* function: sdnpsim_anlgOutSelect */
DNPDEFS_CTLSTAT TMWDEFS_GLOBAL sdnpsim_anlgOutSelect(
  void *pPoint,
  TMWTYPES_ANALOG_VALUE *pValue)
{
  TMWTYPES_UCHAR mode;
  TMWTARG_UNUSED_PARAM(pValue);
  
  /* See if this point has been set to fail for test purposes */
  mode = tmwsim_getTestingMode((TMWSIM_POINT *)pPoint);
  if((mode == TMWSIM_TESTINGMODE_FAILURE)
    ||(mode == TMWSIM_TESTINGMODE_SELECTFAIL))
  {
    /* For test purposes, use the analog value as the CROB status code 
     * This way the testHarness user can set the CROB status code 
     * to generate particular response status codes.
     */
    TMWSIM_DATA_TYPE value = tmwsim_getAnalogValue((TMWSIM_POINT *)pPoint);
      return((DNPDEFS_CTLSTAT)value);
  }

  /* If the device or this point are in localMode  */
  if(_dbLocalMode(pPoint)
    || tmwsim_getLocal((TMWSIM_POINT *)pPoint))
  { 
    return(DNPDEFS_CROB_ST_LOCAL);
  }

  return(DNPDEFS_CTLSTAT_SUCCESS);
}

/* function: sdnpsim_anlgOutOperate */
DNPDEFS_CTLSTAT TMWDEFS_GLOBAL sdnpsim_anlgOutOperate(
  void *pPoint,
  TMWTYPES_ANALOG_VALUE *pValue)
{
  TMWSIM_POINT *pSimPoint = (TMWSIM_POINT *)pPoint;
#if TMWCNFG_SUPPORT_DOUBLE
  TMWSIM_DATA_TYPE newValue = dnputil_getAnalogValueDouble(pValue);
#elif TMWCNFG_SUPPORT_FLOAT
  TMWTYPES_UCHAR flags; /* Temporary storage */
  TMWSIM_DATA_TYPE newValue = dnputil_getAnalogValueFloat(pValue, &flags);
#else
  TMWTYPES_UCHAR flags; /* Temporary storage */
  TMWSIM_DATA_TYPE newValue = dnputil_getAnalogValueLong(pValue, &flags);
#endif
 
  /* See if this point has been set to fail for test purposes */
  TMWTYPES_UCHAR mode = tmwsim_getTestingMode(pSimPoint);
  if((mode == TMWSIM_TESTINGMODE_FAILURE)
    ||(mode == TMWSIM_TESTINGMODE_OPERATEFAIL))
  {
    /* For test purposes, use the value as the CROB status code 
     * This way the testHarness user can set the CROB status code 
     * to generate particular response status codes.
     */ 
    return((DNPDEFS_CTLSTAT)tmwsim_getAnalogValue(pSimPoint));
  }

  /* If the device or this point are in localMode  */
  if (_dbLocalMode(pPoint)
    || tmwsim_getLocal((TMWSIM_POINT *)pPoint))
  { 
    return(DNPDEFS_CROB_ST_LOCAL);
  }

  tmwsim_setAnalogValue(pSimPoint, newValue, TMWDEFS_CHANGE_REMOTE_OP);
  sdnputil_getDateTime((TMWSESN*)pSimPoint->pSCLHandle, &pSimPoint->timeStamp);

  if (pSimPoint->assocPointNumber != TMWDEFS_NO_ASSOC_POINT)
  {
    TMWSIM_POINT *anlgInPtr = (TMWSIM_POINT*)sdnpsim_anlgInGetPoint(pSimPoint->pDbHandle, (TMWTYPES_USHORT)pSimPoint->assocPointNumber);
    if (anlgInPtr != TMWDEFS_NULL)
    {
      sdnpsim_anlgInWrite(anlgInPtr, newValue);
    }
  }

  _callCallback(pSimPoint, TMWSIM_POINT_UPDATE, DNPDEFS_OBJ_40_ANA_OUT_STATUSES, 
    (TMWTYPES_USHORT)tmwsim_getPointNumber(pSimPoint));
  
#if SDNPDATA_SUPPORT_OBJ42
  {
    TMWDEFS_CHANGE_REASON reason;
    if (tmwsim_getChanged(pSimPoint, &reason))
      sdnpo042_addEvent((TMWSESN*)pSimPoint->pSCLHandle, (TMWTYPES_USHORT)(pSimPoint->pointNumber), pValue, tmwsim_getFlags(pSimPoint), &pSimPoint->timeStamp);
  }
#endif

  return(DNPDEFS_CTLSTAT_SUCCESS);
}

/* function: sdnpsim_anlgOutWrite */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_anlgOutWrite(
  void *pPoint,
  TMWTYPES_ANALOG_VALUE *pValue)
{
  TMWSIM_POINT *pSimPoint = (TMWSIM_POINT *)pPoint;
#if TMWCNFG_SUPPORT_DOUBLE
  TMWSIM_DATA_TYPE newValue = dnputil_getAnalogValueDouble(pValue);
#elif TMWCNFG_SUPPORT_FLOAT
  TMWTYPES_UCHAR flags; /* Temporary storage */
  TMWSIM_DATA_TYPE newValue = dnputil_getAnalogValueFloat(pValue, &flags);
#else
  TMWTYPES_UCHAR flags; /* Temporary storage */
  TMWSIM_DATA_TYPE newValue = dnputil_getAnalogValueLong(pValue, &flags);
#endif

  tmwsim_setAnalogValue(pSimPoint, newValue, TMWDEFS_CHANGE_LOCAL_OP);
  sdnputil_getDateTime((TMWSESN*)pSimPoint->pSCLHandle, &pSimPoint->timeStamp);

  _callCallback(pSimPoint, TMWSIM_POINT_UPDATE, DNPDEFS_OBJ_40_ANA_OUT_STATUSES, 
    (TMWTYPES_USHORT)tmwsim_getPointNumber(pSimPoint));

  return(TMWDEFS_TRUE);
}

/* Double Bit Inputs */
/* function: sdnpsim_addDoubleInput */
void * TMWDEFS_GLOBAL sdnpsim_addDoubleInput(
  void *pHandle,
  TMWDEFS_CLASS_MASK classMask,
  TMWTYPES_UCHAR flagsAndValue)
{
  TMWSIM_POINT *pPoint;
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;

  TMWTYPES_USHORT pointNum = (TMWTYPES_USHORT)tmwsim_tableSize(&pDbHandle->doubleInputs);
  pPoint = tmwsim_tableAdd(&pDbHandle->doubleInputs, pointNum);
  if(pPoint != TMWDEFS_NULL)
  {
    /* Just use flags to hold value and flags, since that is the way it is
     * in the dnp message 
     */
    tmwsim_initDoubleBinary(pPoint, pHandle, pointNum);
    pPoint->pSCLHandle = (void*)pDbHandle->pSDNPSession;
    pPoint->classMask = classMask;
    pPoint->flags = flagsAndValue;
    pPoint->defaultEventVariation = 3;
    pPoint->defaultStaticVariation = 1;
    pPoint->eventMode = (TMWTYPES_UCHAR)TMWDEFS_EVENT_MODE_SOE;
    sdnputil_getDateTime((TMWSESN*)pPoint->pSCLHandle, &pPoint->timeStamp);

    _callCallback((TMWSIM_POINT *)pPoint, TMWSIM_POINT_ADD, DNPDEFS_OBJ_3_DBL_INPUTS, 
      (TMWTYPES_USHORT)tmwsim_getPointNumber((TMWSIM_POINT *)pPoint));
  }

  return(pPoint);
}

/* function: sdnpsim_deleteDoubleInput */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_deleteDoubleInput(
  void *pHandle)
{
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;
  TMWTYPES_USHORT pointNumber = (TMWTYPES_USHORT)tmwsim_tableSize(&pDbHandle->doubleInputs);
  if(pointNumber > 0)
  { 
    _callRemoveCallback(pDbHandle, TMWSIM_POINT_DELETE, DNPDEFS_OBJ_3_DBL_INPUTS,  
      (TMWTYPES_USHORT)(pointNumber-1)); 

    return(tmwsim_tableDelete(&pDbHandle->doubleInputs, pointNumber-1));
  }

  return(TMWDEFS_FALSE);
}

/* function: sdnpsim_dblInQuantity */
TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpsim_dblInQuantity(
  void *pHandle)
{
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;
  return((TMWTYPES_USHORT)tmwsim_tableSize(&pDbHandle->doubleInputs));
}

/* function: sdnpsim_dblInGetPoint */
void * TMWDEFS_GLOBAL sdnpsim_dblInGetPoint(
  void *pHandle,
  TMWTYPES_USHORT pointNum)
{
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;
  return(tmwsim_tableFindPoint(&pDbHandle->doubleInputs, pointNum));
}

/* function: sdnpsim_dblInSetFlags */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_dblInSetFlags(
  void *pPoint,
  TMWTYPES_UCHAR flags)
{
  TMWSIM_POINT *pSimPoint = (TMWSIM_POINT *)pPoint;
  tmwsim_setFlags(pSimPoint, flags, TMWDEFS_CHANGE_REMOTE_OP);
  sdnputil_getDateTime((TMWSESN*)pSimPoint->pSCLHandle, &pSimPoint->timeStamp);

  _callCallback(pSimPoint, TMWSIM_POINT_UPDATE, DNPDEFS_OBJ_3_DBL_INPUTS, 
    (TMWTYPES_USHORT)tmwsim_getPointNumber(pSimPoint));

  return(TMWDEFS_TRUE);
}

/* function: sdnpsim_dblInGetEnabledPoint */
void * TMWDEFS_GLOBAL sdnpsim_dblInGetEnabledPoint(
  void *pHandle,
  TMWTYPES_USHORT pointNum)
{
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;
  TMWSIM_POINT *pPoint = tmwsim_tableFindPoint(&pDbHandle->doubleInputs, pointNum);
  if((pPoint == TMWDEFS_NULL) || !pPoint->enabled)
    return(TMWDEFS_NULL);

  return(pPoint);
}

/* function: sdnpsim_dblInDefVariation */
TMWTYPES_UCHAR TMWDEFS_GLOBAL sdnpsim_dblInDefVariation(
  void *pPoint)
{
  return(tmwsim_getDefStaticVariation((TMWSIM_POINT *)pPoint));
}

TMWTYPES_UCHAR sdnpsim_dblInEventDefVariation(
  void *pPoint,
  TMWDEFS_CLASS_MASK classMask)
{
  TMWTARG_UNUSED_PARAM(classMask);
  return(tmwsim_getDefEventVariation((TMWSIM_POINT *)pPoint));
}

/* function: sdnpsim_dblInRead */
void TMWDEFS_GLOBAL sdnpsim_dblInRead(
  void *pPoint,
  TMWTYPES_UCHAR *pFlags)
{
  *pFlags = tmwsim_getFlags((TMWSIM_POINT *)pPoint);
  return;
}

/* function: sdnpsim_dblInChanged */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_dblInChanged(
  void *pPoint,
  TMWTYPES_UCHAR *pFlags)
{
  TMWDEFS_CHANGE_REASON reason;
  if(tmwsim_getChanged((TMWSIM_POINT *)pPoint, &reason))
  {
    *pFlags = tmwsim_getFlags((TMWSIM_POINT *)pPoint);
    return(TMWDEFS_TRUE);
  }

  return(TMWDEFS_FALSE);
}

/* Time and date - indexed absolute time and long interval */
/* function: sdnpsim_addIndexedTime */
void *TMWDEFS_GLOBAL sdnpsim_addIndexedTime(
  void *pHandle,
  TMWDEFS_CLASS_MASK classMask,
  TMWDTIME timeValue,
  TMWTYPES_ULONG intervalCount,
  TMWTYPES_BYTE  intervalUnit)
{
  TMWSIM_POINT *pPoint;
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;

  TMWTYPES_USHORT pointNum = (TMWTYPES_USHORT)tmwsim_tableSize(&pDbHandle->indexedTime);
  pPoint = tmwsim_tableAdd(&pDbHandle->indexedTime, pointNum);
  if (pPoint != TMWDEFS_NULL)
  {
    tmwsim_initIndexedTime(pPoint, pHandle, pointNum);
    pPoint->pSCLHandle = (void*)pDbHandle->pSDNPSession;
    pPoint->classMask = classMask;

    tmwsim_setIndexedTime(pPoint, timeValue, intervalCount, intervalUnit);

    sdnputil_getDateTime((TMWSESN*)pPoint->pSCLHandle, &pPoint->timeStamp);

    _callCallback((TMWSIM_POINT *)pPoint, TMWSIM_POINT_ADD, DNPDEFS_OBJ_50_TIME_AND_DATE,
      (TMWTYPES_USHORT)tmwsim_getPointNumber((TMWSIM_POINT *)pPoint));
  }

  return(pPoint);
}

/* function: sdnpsim_deleteIndexedTime */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_deleteIndexedTime(
  void *pHandle)
{
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;
  TMWTYPES_USHORT pointNumber = (TMWTYPES_USHORT)tmwsim_tableSize(&pDbHandle->indexedTime);
  if (pointNumber > 0)
  {
    _callRemoveCallback(pDbHandle, TMWSIM_POINT_DELETE, DNPDEFS_OBJ_50_TIME_AND_DATE,
      (TMWTYPES_USHORT)(pointNumber - 1));

    return(tmwsim_tableDelete(&pDbHandle->indexedTime, pointNumber - 1));
  }

  return(TMWDEFS_FALSE);
}

/* function: sdnpsim_indexedTimeQuantity */
TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpsim_indexedTimeQuantity(
  void *pHandle)
{
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;
  return((TMWTYPES_USHORT)tmwsim_tableSize(&pDbHandle->indexedTime));
}

/* function: sdnpsim_indexedTimeGetPoint */
void * TMWDEFS_GLOBAL sdnpsim_indexedTimeGetPoint(
  void *pHandle,
  TMWTYPES_USHORT pointNum)
{
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;
  return(tmwsim_tableFindPoint(&pDbHandle->indexedTime, pointNum));
}

/* function: sdnpsim_indexedTimeGetEnabledPoint */
void * TMWDEFS_GLOBAL sdnpsim_indexedTimeGetEnabledPoint(
  void *pHandle,
  TMWTYPES_USHORT pointNum)
{
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;
  TMWSIM_POINT *pPoint = tmwsim_tableFindPoint(&pDbHandle->indexedTime, pointNum);
  if ((pPoint == TMWDEFS_NULL) || !pPoint->enabled)
    return(TMWDEFS_NULL);

  return(pPoint);
}

/* function: sdnpsim_indexedTimeRead */
TMWDEFS_SCL_API void TMWDEFS_GLOBAL sdnpsim_indexedTimeRead(
  void *pPoint,
  TMWDTIME *pTimeStamp,
  TMWTYPES_ULONG *pIntervalCount,
  TMWTYPES_BYTE  *pIntervalUnit)
{
  tmwsim_getIndexedTimeTime((TMWSIM_POINT *)pPoint, pTimeStamp);
  *pIntervalCount = tmwsim_getIndexedTimeIntervalCount((TMWSIM_POINT *)pPoint);
  *pIntervalUnit = tmwsim_getIndexedTimeIntervalUnit((TMWSIM_POINT *)pPoint);
  return;
}

/* function: sdnpsim_startTimeWrite */
TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_startTimeWrite(
  void *pPoint,
  TMWDTIME timeStamp)
{
  TMWSIM_POINT *pSimPoint = (TMWSIM_POINT *)pPoint;
  tmwsim_setStartTime(pSimPoint, timeStamp);
  sdnputil_getDateTime((TMWSESN*)pSimPoint->pSCLHandle, &pSimPoint->timeStamp);

  _callCallback(pSimPoint, TMWSIM_POINT_UPDATE, DNPDEFS_OBJ_50_TIME_AND_DATE,
    (TMWTYPES_USHORT)tmwsim_getPointNumber(pSimPoint));

  return(TMWDEFS_TRUE);
}

/* function: sdnpsim_intervalCountWrite */
TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_intervalCountWrite(
  void *pPoint,
  TMWTYPES_ULONG intervalCount)
{
  TMWSIM_POINT *pSimPoint = (TMWSIM_POINT *)pPoint;
  tmwsim_setIntervalCount(pSimPoint, intervalCount);
  sdnputil_getDateTime((TMWSESN*)pSimPoint->pSCLHandle, &pSimPoint->timeStamp);

  _callCallback(pSimPoint, TMWSIM_POINT_UPDATE, DNPDEFS_OBJ_50_TIME_AND_DATE,
    (TMWTYPES_USHORT)tmwsim_getPointNumber(pSimPoint));

  return(TMWDEFS_TRUE);
}

/* function: sdnpsim_intervalUnitWrite */
TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_intervalUnitWrite(
  void *pPoint,
  TMWTYPES_BYTE  intervalUnit)
{
  TMWSIM_POINT *pSimPoint = (TMWSIM_POINT *)pPoint;
  tmwsim_setIntervalUnit(pSimPoint, intervalUnit);
  sdnputil_getDateTime((TMWSESN*)pSimPoint->pSCLHandle, &pSimPoint->timeStamp);

  _callCallback(pSimPoint, TMWSIM_POINT_UPDATE, DNPDEFS_OBJ_50_TIME_AND_DATE,
    (TMWTYPES_USHORT)tmwsim_getPointNumber(pSimPoint));

  return(TMWDEFS_TRUE);
}
/* function: sdnpsim_indexedTimeWrite */
TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_indexedTimeWrite(
  void *pPoint,
  TMWDTIME timeStamp,
  TMWTYPES_ULONG intervalCount,
  TMWTYPES_BYTE  intervalUnit)
{
  TMWSIM_POINT *pSimPoint = (TMWSIM_POINT *)pPoint;
  tmwsim_setIndexedTime(pSimPoint, timeStamp, intervalCount, intervalUnit);
  sdnputil_getDateTime((TMWSESN*)pSimPoint->pSCLHandle, &pSimPoint->timeStamp);

  _callCallback(pSimPoint, TMWSIM_POINT_UPDATE, DNPDEFS_OBJ_50_TIME_AND_DATE,
    (TMWTYPES_USHORT)tmwsim_getPointNumber(pSimPoint));

  return(TMWDEFS_TRUE);
}

#if SDNPDATA_SUPPORT_OBJ110
/* function: sdnpsim_addString */
void *TMWDEFS_GLOBAL sdnpsim_addString(
  void *pHandle,
  TMWDEFS_CLASS_MASK classMask,
  TMWTYPES_UCHAR *pValue,
  TMWTYPES_UCHAR length)
{
  TMWSIM_POINT *pPoint;
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;

  TMWTYPES_USHORT pointNum = (TMWTYPES_USHORT)tmwsim_tableSize(&pDbHandle->octetStrings);
  pPoint = tmwsim_tableAdd(&pDbHandle->octetStrings, pointNum);
  if(pPoint != TMWDEFS_NULL)
  {
    tmwsim_initString(pPoint, pHandle, pointNum);
    pPoint->pSCLHandle = (void*)pDbHandle->pSDNPSession;
    tmwsim_setEventClass(pPoint, classMask);
    /* If not value is passed in, use the default value set by tmwsim_initString */
    if (pValue)
    {
      tmwsim_setStringValue(pPoint, pValue, length, TMWDEFS_CHANGE_NONE);
    }
    sdnputil_getDateTime((TMWSESN*)pPoint->pSCLHandle, &pPoint->timeStamp);

    pPoint->eventMode = (TMWTYPES_UCHAR)TMWDEFS_EVENT_MODE_SOE;

    _callCallback((TMWSIM_POINT *)pPoint, TMWSIM_POINT_ADD, DNPDEFS_OBJ_110_STRING_DATA, 
      (TMWTYPES_USHORT)tmwsim_getPointNumber((TMWSIM_POINT *)pPoint));
  }

  return(pPoint);
}

/* function: sdnpsim_deleteString */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_deleteString(
  void *pHandle)
{
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;
  TMWTYPES_USHORT pointNumber = (TMWTYPES_USHORT)tmwsim_tableSize(&pDbHandle->octetStrings);
  if(pointNumber > 0)
  {
    _callRemoveCallback(pDbHandle, TMWSIM_POINT_DELETE, DNPDEFS_OBJ_110_STRING_DATA, 
        (TMWTYPES_USHORT)(pointNumber-1));

    return(tmwsim_tableDelete(&pDbHandle->octetStrings, pointNumber-1));
  }

  return(TMWDEFS_FALSE);
}

/* function: sdnpsim_strQuantity */
TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpsim_strQuantity(
  void *pHandle)
{
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;
  return((TMWTYPES_USHORT)tmwsim_tableSize(&pDbHandle->octetStrings));
}

/* function: sdnpsim_strGetPoint */
void *TMWDEFS_GLOBAL sdnpsim_strGetPoint(
  void *pHandle,
  TMWTYPES_USHORT pointNum)
{
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;
  TMWSIM_POINT *pDataPoint = tmwsim_tableFindPoint(&pDbHandle->octetStrings, pointNum);

  /* Object groups 110 & 114 share a database, ensure this point is not an extended string. */
  if ((pDataPoint) && (pDataPoint->data.string.extString == TMWDEFS_TRUE))
  {
    pDataPoint = TMWDEFS_NULL;
  }
  return(pDataPoint);
}

/* function: sdnpsim_strGetEnabledPoint */
void *TMWDEFS_GLOBAL sdnpsim_strGetEnabledPoint(
  void *pHandle,
  TMWTYPES_USHORT pointNum)
{
  TMWSIM_POINT *pPoint = (TMWSIM_POINT *)sdnpsim_strGetPoint(pHandle, pointNum);
  if((pPoint == TMWDEFS_NULL) || !pPoint->enabled)
    return(TMWDEFS_NULL);

  return(pPoint);
}

/* function: sdnpsim_strRead */
void TMWDEFS_GLOBAL sdnpsim_strRead(
  void *pPoint,
  TMWTYPES_UCHAR maxLength,
  TMWTYPES_UCHAR *pBuf,
  TMWTYPES_UCHAR *pBufLength)
{
  tmwsim_getStringValue((TMWSIM_POINT *)pPoint, maxLength, pBuf, pBufLength);
  return;
}

/* function: sdnpsim_strWrite */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_strWrite(
  void *pPoint,
  TMWTYPES_UCHAR *pBuf,
  TMWTYPES_UCHAR bufLength)
{
  TMWSIM_POINT *pSimPoint = (TMWSIM_POINT *)pPoint;
  tmwsim_setStringValue(pSimPoint, pBuf, bufLength, TMWDEFS_CHANGE_REMOTE_OP);
  sdnputil_getDateTime((TMWSESN*)pSimPoint->pSCLHandle, &pSimPoint->timeStamp);

  _callCallback(pSimPoint, TMWSIM_POINT_UPDATE, DNPDEFS_OBJ_110_STRING_DATA, 
    (TMWTYPES_USHORT)tmwsim_getPointNumber(pSimPoint));

  return(TMWDEFS_TRUE);
}

/* function: sdnpsim_strChanged */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_strChanged(
  void *pPoint,
  TMWTYPES_UCHAR maxLength,
  TMWTYPES_UCHAR *pBuf,
  TMWTYPES_UCHAR *pBufLength)
{
  TMWDEFS_CHANGE_REASON reason;

  if(tmwsim_getChanged((TMWSIM_POINT *)pPoint, &reason))
  {
    tmwsim_getStringValue((TMWSIM_POINT *)pPoint, maxLength, pBuf, pBufLength);
    return(TMWDEFS_TRUE);
  }

  return(TMWDEFS_FALSE);
}
#endif

#if SDNPDATA_SUPPORT_OBJ112
/* function: sdnpsim_addVirtualTerminal */
void * TMWDEFS_GLOBAL sdnpsim_addVirtualTerminal(
  void *pHandle,
  TMWDEFS_CLASS_MASK classMask,
  TMWTYPES_UCHAR *pValue,
  TMWTYPES_UCHAR length)
{
  TMWSIM_POINT *pPoint;
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;

  TMWTYPES_USHORT pointNum = (TMWTYPES_USHORT)tmwsim_tableSize(&pDbHandle->virtualTerminals);
  pPoint = tmwsim_tableAdd(&pDbHandle->virtualTerminals, pointNum);
  if(pPoint != TMWDEFS_NULL)
  {
    tmwsim_initString(pPoint, pHandle, pointNum);
    pPoint->pSCLHandle = (void*)pDbHandle->pSDNPSession;
    tmwsim_setEventClass(pPoint, classMask);
    tmwsim_setStringValue(pPoint, pValue, length, TMWDEFS_CHANGE_NONE);
    pPoint->eventMode = (TMWTYPES_UCHAR)TMWDEFS_EVENT_MODE_SOE;
    pPoint->pCallbackParam = TMWDEFS_NULL;
    sdnputil_getDateTime((TMWSESN*)pPoint->pSCLHandle, &pPoint->timeStamp);

    _callCallback((TMWSIM_POINT *)pPoint, TMWSIM_POINT_ADD, DNPDEFS_OBJ_112_VTERM_OUTPUT, 
      (TMWTYPES_USHORT)tmwsim_getPointNumber((TMWSIM_POINT *)pPoint));
  }

  return(pPoint);
}

/* function: sdnpsim_deleteVirtualTerminal */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_deleteVirtualTerminal(
  void *pHandle)
{
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;
  TMWTYPES_USHORT pointNumber = (TMWTYPES_USHORT)tmwsim_tableSize(&pDbHandle->virtualTerminals);
  if(pointNumber > 0)
  {
    _callRemoveCallback(pDbHandle, TMWSIM_POINT_DELETE, DNPDEFS_OBJ_112_VTERM_OUTPUT, 
      (TMWTYPES_USHORT)(pointNumber-1));

    return(tmwsim_tableDelete(&pDbHandle->virtualTerminals, pointNumber-1));
  }

  return(TMWDEFS_FALSE);
}

/* function: sdnpsim_vtermQuantity */
TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpsim_vtermQuantity(
  void *pHandle)
{
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;
  return((TMWTYPES_USHORT)tmwsim_tableSize(&pDbHandle->virtualTerminals));
}

/* function: sdnpsim_vtermGetPoint */
void * TMWDEFS_GLOBAL sdnpsim_vtermGetPoint(
  void *pHandle,
  TMWTYPES_USHORT pointNum)
{
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;
  return(tmwsim_tableFindPoint(&pDbHandle->virtualTerminals, pointNum));
}

/* function: sdnpsim_vtermGetEnabledPoint */
void * TMWDEFS_GLOBAL sdnpsim_vtermGetEnabledPoint(
  void *pHandle,
  TMWTYPES_USHORT pointNum)
{
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;
  TMWSIM_POINT *pPoint = tmwsim_tableFindPoint(&pDbHandle->virtualTerminals, pointNum);
  if((pPoint == TMWDEFS_NULL) || !pPoint->enabled)
    return(TMWDEFS_NULL);

  return(pPoint);
}

/* function: sdnpsim_vtermRead */
void TMWDEFS_GLOBAL sdnpsim_vtermRead(
  void *pPoint,
  TMWTYPES_UCHAR maxLength,
  TMWTYPES_UCHAR *pBuf,
  TMWTYPES_UCHAR *pBufLength)
{
  tmwsim_getStringValue((TMWSIM_POINT *)pPoint, maxLength, pBuf, pBufLength);
  return;
}

/* function: sdnpsim_vtermWrite */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_vtermWrite(
  void *pPoint,
  TMWTYPES_UCHAR *pBuf,
  TMWTYPES_UCHAR bufLength)
{
  TMWSIM_POINT *pSimPoint = (TMWSIM_POINT *)pPoint;
  tmwsim_setStringValue(pSimPoint, pBuf, bufLength, TMWDEFS_CHANGE_REMOTE_OP);
  sdnputil_getDateTime((TMWSESN*)pSimPoint->pSCLHandle, &pSimPoint->timeStamp);

  _callCallback(pSimPoint, TMWSIM_POINT_UPDATE, DNPDEFS_OBJ_112_VTERM_OUTPUT, 
    (TMWTYPES_USHORT)tmwsim_getPointNumber(pSimPoint));

  return(TMWDEFS_TRUE);
}

/* function: sdnpsim_vtermChanged */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_vtermChanged(
  void *pPoint,
  TMWTYPES_UCHAR maxLength,
  TMWTYPES_UCHAR *pBuf,
  TMWTYPES_UCHAR *pBufLength)
{
  TMWDEFS_CHANGE_REASON reason;

  if(tmwsim_getChanged((TMWSIM_POINT *)pPoint, &reason))
  {
    tmwsim_getStringValue((TMWSIM_POINT *)pPoint, maxLength, pBuf, pBufLength);
    return(TMWDEFS_TRUE);
  }

  return(TMWDEFS_FALSE);
}
#endif

#if SDNPDATA_SUPPORT_OBJ114
/* function: sdnpsim_addExtString */
void *TMWDEFS_GLOBAL sdnpsim_addExtString(
  void *pHandle,
  TMWDEFS_CLASS_MASK classMask,
  TMWTYPES_UCHAR flags,
  TMWTYPES_UCHAR *pValue,
  TMWTYPES_USHORT length)
{
  TMWSIM_POINT *pPoint;
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;

  TMWTYPES_USHORT pointNum = (TMWTYPES_USHORT)tmwsim_tableSize(&pDbHandle->octetStrings);
  pPoint = tmwsim_tableAdd(&pDbHandle->octetStrings, pointNum);
  if(pPoint != TMWDEFS_NULL)
  {
    tmwsim_initExtString(pPoint, pHandle, pointNum);
    pPoint->pSCLHandle = (void*)pDbHandle->pSDNPSession;
    pPoint->flags = flags;
    pPoint->defaultEventVariation = 4;
    pPoint->defaultStaticVariation = 2;
    pPoint->eventMode = (TMWTYPES_UCHAR)TMWDEFS_EVENT_MODE_SOE;
    tmwsim_setEventClass(pPoint, classMask);
    /* If not value is passed in, use the default value set by tmwsim_initExtString */
    if (pValue)
    {
      tmwsim_setExtStringValue(pPoint, pValue, length, TMWDEFS_CHANGE_NONE);
    }
    sdnputil_getDateTime((TMWSESN*)pPoint->pSCLHandle, &pPoint->timeStamp);

    pPoint->eventMode = (TMWTYPES_UCHAR)TMWDEFS_EVENT_MODE_SOE;

    _callCallback((TMWSIM_POINT *)pPoint, TMWSIM_POINT_ADD, DNPDEFS_OBJ_114_EXT_STR_DATA, 
      (TMWTYPES_USHORT)tmwsim_getPointNumber((TMWSIM_POINT *)pPoint));
  }

  return(pPoint);
}

/* function: sdnpsim_deleteExtString */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_deleteExtString(
  void *pHandle)
{
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;
  TMWTYPES_USHORT pointNumber = (TMWTYPES_USHORT)tmwsim_tableSize(&pDbHandle->octetStrings);
  if(pointNumber > 0)
  {
    _callRemoveCallback(pDbHandle, TMWSIM_POINT_DELETE, DNPDEFS_OBJ_114_EXT_STR_DATA, 
        (TMWTYPES_USHORT)(pointNumber-1));

    return(tmwsim_tableDelete(&pDbHandle->octetStrings, pointNumber-1));
  }

  return(TMWDEFS_FALSE);
}

/* function: sdnpsim_extStrQuantity */
TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpsim_extStrQuantity(
  void *pHandle)
{
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;
  return((TMWTYPES_USHORT)tmwsim_tableSize(&pDbHandle->octetStrings));
}

/* function: sdnpsim_extStrGetPoint */
void *TMWDEFS_GLOBAL sdnpsim_extStrGetPoint(
  void *pHandle,
  TMWTYPES_USHORT pointNum)
{
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;
  TMWSIM_POINT *pDataPoint = tmwsim_tableFindPoint(&pDbHandle->octetStrings, pointNum);

  /* Object groups 110 & 114 share a database, ensure this point is an extended string. */
  if ((pDataPoint) && (pDataPoint->data.string.extString == TMWDEFS_FALSE))
  {
    pDataPoint = TMWDEFS_NULL;
  }
  return(pDataPoint);
}

/* function: sdnpsim_extStrDefVariation */
TMWTYPES_UCHAR TMWDEFS_GLOBAL sdnpsim_extStrDefVariation(
  void *pPoint)
{
  return(tmwsim_getDefStaticVariation((TMWSIM_POINT *)pPoint));
}

/* function: sdnpsim_extStrEventDefVariation */
TMWTYPES_UCHAR sdnpsim_extStrEventDefVariation(
  void *pPoint,
  TMWDEFS_CLASS_MASK classMask)
{
  TMWTARG_UNUSED_PARAM(classMask);
  return(tmwsim_getDefEventVariation((TMWSIM_POINT *)pPoint));
}

/* function: sdnpsim_extStrGetEnabledPoint */
void *TMWDEFS_GLOBAL sdnpsim_extStrGetEnabledPoint(
  void *pHandle,
  TMWTYPES_USHORT pointNum)
{
  TMWSIM_POINT *pPoint = (TMWSIM_POINT *)sdnpsim_extStrGetPoint(pHandle, pointNum);
  if((pPoint == TMWDEFS_NULL) || !pPoint->enabled)
    return(TMWDEFS_NULL);

  return(pPoint);
}

/* function: sdnpsim_extStrRead */
void TMWDEFS_GLOBAL sdnpsim_extStrRead(
  void *pPoint,
  TMWTYPES_USHORT maxLength,
  TMWTYPES_UCHAR *pBuf,
  TMWTYPES_USHORT *pBufLength,
  TMWTYPES_UCHAR *pFlags)
{
  *pFlags = tmwsim_getFlags((TMWSIM_POINT *)pPoint);
  tmwsim_getExtStringValue((TMWSIM_POINT *)pPoint, maxLength, pBuf, pBufLength);
  return;
}

TMWTYPES_UCHAR * TMWDEFS_GLOBAL sdnpsim_extStrGetPtr(
  void *pPoint,
  TMWTYPES_USHORT *pBufLength,
  TMWTYPES_UCHAR *pFlags)
{
  *pFlags = tmwsim_getFlags((TMWSIM_POINT *)pPoint);
  return(tmwsim_getExtStringPointer((TMWSIM_POINT *)pPoint, pBufLength));
}

/* function: sdnpsim_extStrWrite */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_extStrWrite(
  void *pPoint,
  TMWTYPES_UCHAR *pBuf,
  TMWTYPES_USHORT bufLength)
{
  TMWSIM_POINT *pSimPoint = (TMWSIM_POINT *)pPoint;
  tmwsim_setExtStringValue(pSimPoint, pBuf, bufLength, TMWDEFS_CHANGE_REMOTE_OP);
  sdnputil_getDateTime((TMWSESN*)pSimPoint->pSCLHandle, &pSimPoint->timeStamp);

  _callCallback(pSimPoint, TMWSIM_POINT_UPDATE, DNPDEFS_OBJ_114_EXT_STR_DATA, 
    (TMWTYPES_USHORT)tmwsim_getPointNumber(pSimPoint));

  return(TMWDEFS_TRUE);
}

/* function: sdnpsim_extStrChanged */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_extStrChanged(
  void *pPoint,
  TMWTYPES_USHORT maxLength,
  TMWTYPES_UCHAR *pBuf,
  TMWTYPES_USHORT *pBufLength,
  TMWTYPES_UCHAR *pFlags)
{
  TMWDEFS_CHANGE_REASON reason;

  if(tmwsim_getChanged((TMWSIM_POINT *)pPoint, &reason))
  {
    *pFlags = tmwsim_getFlags((TMWSIM_POINT *)pPoint);
    tmwsim_getExtStringValue((TMWSIM_POINT *)pPoint, maxLength, pBuf, pBufLength);
    return(TMWDEFS_TRUE);
  }

  return(TMWDEFS_FALSE);
}
#endif

#if SDNPDATA_SUPPORT_OBJ120

/* For sample test application purposes only */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_authConfigUser(
  void             *pHandle,
  TMWTYPES_CHAR    *pUserName,
  TMWTYPES_USHORT   userNameLength,
  TMWTYPES_USHORT   userNumber,
  TMWTYPES_USHORT   userRole,
  TMWTYPES_USHORT   userRoleExpiryInterval)
{
  SDNPSIM_AUTHUSER *pUser = TMWDEFS_NULL;
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;
  TMWTYPES_BOOL notOnList = TMWDEFS_FALSE;

  while ((pUser = (SDNPSIM_AUTHUSER *)tmwdlist_getAfter(
    &pDbHandle->authUsers, (TMWDLIST_MEMBER *)pUser)) != TMWDEFS_NULL)
  {
    if (memcmp(pUser->userName, pUserName, userNameLength) == 0)
    {
      break;
    }
  }

  if (pUser == TMWDEFS_NULL)
  {
    pUser = (SDNPSIM_AUTHUSER *)sdnpmem_alloc(SDNPMEM_SIM_AUTH_USER_TYPE);
    notOnList = TMWDEFS_TRUE;
  }

  if (pUser == TMWDEFS_NULL)
  {
    return TMWDEFS_FALSE;
  }

  pUser->userRole = userRole;
  pUser->userRoleExpiryInterval = userRoleExpiryInterval;
  pUser->userNumber = userNumber;

  if (notOnList)
  {
    memcpy(pUser->userName, pUserName, userNameLength);
    pUser->userNameLength = userNameLength;
    tmwdlist_addEntry(&pDbHandle->authUsers, (TMWDLIST_MEMBER *)pUser);
  }

  return TMWDEFS_TRUE;
}
 
/* function: sdnpsim_authAssignClass */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_authAssignClass(
  void *pHandle,
  TMWDEFS_CLASS_MASK classMask)
{ 
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;
  pDbHandle->authClassMask = classMask;
  return TMWDEFS_TRUE;
}
 
/* function: sdnpsim_authErrorEventClass */
TMWDEFS_CLASS_MASK TMWDEFS_GLOBAL sdnpsim_authErrorEventClass(
  void *pHandle)
{ 
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;
  return(pDbHandle->authClassMask);
}

/* function: sdnpsim_authGetOSName */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_authGetOSName(
  void            *pHandle, 
  TMWTYPES_CHAR   *pOSName,
  TMWTYPES_USHORT *pOSNameLength)
{ 
  TMWTARG_UNUSED_PARAM(pHandle);
  STRCPY(pOSName, *pOSNameLength, "SDNP Outstation");
  *pOSNameLength = (TMWTYPES_USHORT)strlen(pOSName);
  return(TMWDEFS_TRUE);
}

/* function: sdnpsim_authGetUserAsymPubKey */
/* Get public key stored by sdnpsim_authUserStatusChange*/
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_authGetUserAsymPubKey(
  void            *pHandle,
  TMWTYPES_USHORT  userNumber,
  TMWCRYPTO_KEY   *pKey)
{ 
  SDNPSIM_AUTHUSER *pUser = TMWDEFS_NULL;
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;
   
  while((pUser = (SDNPSIM_AUTHUSER *)tmwdlist_getAfter(
      &pDbHandle->authUsers, (TMWDLIST_MEMBER *)pUser)) != TMWDEFS_NULL)
  {
    if(pUser->userNumber == userNumber)  
    {  
      memcpy(pKey->value, pUser->userPublicKey, pUser->userPublicKeyLength);
      pKey->value[pUser->userPublicKeyLength] = 0;
      pKey->length = pUser->userPublicKeyLength; 
      pKey->ivLength = 0;
      return(TMWDEFS_TRUE);
    }
  } 
   return(TMWDEFS_FALSE);
}
    
/* function: sdnpsim_authGetUserName */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_authGetUserName(
  void            *pHandle, 
  TMWTYPES_USHORT  userNumber,
  TMWTYPES_CHAR   *pUserName,
  TMWTYPES_USHORT *pUserNameLength)
{  
  SDNPSIM_AUTHUSER *pUser = TMWDEFS_NULL;
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;

  while((pUser = (SDNPSIM_AUTHUSER *)tmwdlist_getAfter(
      &pDbHandle->authUsers, (TMWDLIST_MEMBER *)pUser)) != TMWDEFS_NULL)
  {
    if(userNumber == pUser->userNumber)
    {
      memcpy(pUserName, pUser->userName, pUser->userNameLength);
      *pUserNameLength = pUser->userNameLength; 
      return TMWDEFS_TRUE;
    }
  }
  return TMWDEFS_FALSE;
}

/* function: sdnpsim_authUserStatusChange */
TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpsim_authUserStatusChg(
  void            *pHandle,
  TMWTYPES_CHAR   *pUserName,
  TMWTYPES_USHORT  userNameLength,
  TMWTYPES_UCHAR   keyChangeMethod, 
  TMWTYPES_UCHAR   operation, 
  TMWTYPES_USHORT  userRole, 
  TMWTYPES_USHORT  userRoleExpiryInterval)
{ 
  SDNPSIM_AUTHUSER *pUser = TMWDEFS_NULL;
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle; 
  TMWTYPES_USHORT userNumber = 0;
  TMWTYPES_BOOL notOnList = TMWDEFS_FALSE;
   
  while((pUser = (SDNPSIM_AUTHUSER *)tmwdlist_getAfter(
      &pDbHandle->authUsers, (TMWDLIST_MEMBER *)pUser)) != TMWDEFS_NULL)
  {
    if(memcmp(pUser->userName, pUserName, userNameLength)==0)
    { 
      break;
    }
  } 
 
  if(pUser == TMWDEFS_NULL)
  {
    /* If user does not exist, create user for operation ADD */
    if (operation == DNPAUTH_USER_STATUS_ADD)
    {
      pUser = (SDNPSIM_AUTHUSER *)sdnpmem_alloc(SDNPMEM_SIM_AUTH_USER_TYPE);
      notOnList = TMWDEFS_TRUE;
    }
  }
 
  if(pUser == TMWDEFS_NULL)
  {
    return(0);
  }
  
  pUser->userRole = userRole;
  pUser->userRoleExpiryInterval = userRoleExpiryInterval;
  pUser->keyChangeMethod = keyChangeMethod;
  pUser->operation = operation;
   
  if(notOnList)
  {
    memcpy(pUser->userName, pUserName, userNameLength);
    pUser->userNameLength = userNameLength;
    if(memcmp("Common", pUserName, userNameLength) == 0)
      pUser->userNumber = 1;
    else
      pUser->userNumber = pDbHandle->authNextUserNumber++;
    tmwdlist_addEntry(&pDbHandle->authUsers, (TMWDLIST_MEMBER *)pUser);
  }

  userNumber = pUser->userNumber;

  if (operation == DNPAUTH_USER_STATUS_DELETE)
  {
    tmwdlist_removeEntry(&pDbHandle->authUsers, (TMWDLIST_MEMBER *)pUser);
    return(userNumber);
  }

  return userNumber;
}

#if SDNPDATA_SUPPORT_OBJ120_V8
/* function: sdnpsim_authUserCertRcvd */
TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpsim_authUserCertRcvd(
  void            *pHandle, 
  TMWTYPES_UCHAR   keyChangeMethod, 
  TMWTYPES_UCHAR   certType, 
  TMWTYPES_UCHAR   *pCertData, 
  TMWTYPES_USHORT  certDataLength)
{   
  SDNPSIM_AUTHUSER *pUser = TMWDEFS_NULL;
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;
  TMWTYPES_BOOL notOnList = TMWDEFS_FALSE;
  TMWTARG_UNUSED_PARAM(certType);

  /* SPM for now just use my own format for certificate*/
  /* ***************************************************/
  TMWTYPES_UCHAR  operation = *pCertData++;
  TMWTYPES_USHORT userRoleExpiryInterval = 100;
  TMWTYPES_USHORT userRole = 32768;
  TMWTYPES_USHORT userNameLength = certDataLength -1;

  while((pUser = (SDNPSIM_AUTHUSER *)tmwdlist_getAfter(
      &pDbHandle->authUsers, (TMWDLIST_MEMBER *)pUser)) != TMWDEFS_NULL)
  {
    if(memcmp(pUser->userName, pCertData, userNameLength)==0)
    { 
      break;
    }
  } 
 
  if(pUser == TMWDEFS_NULL)
  {
    pUser = (SDNPSIM_AUTHUSER *)sdnpmem_alloc(SDNPMEM_SIM_AUTH_USER_TYPE);
    notOnList = TMWDEFS_TRUE;
  }
 
  if(pUser == TMWDEFS_NULL)
  {
    return(0);
  }
  
  pUser->userRole = userRole;
  pUser->userRoleExpiryInterval = userRoleExpiryInterval;
  pUser->keyChangeMethod = keyChangeMethod; 
  pUser->operation = operation;
   
  if(notOnList)
  {
    memcpy(pUser->userName, pCertData, userNameLength);
    pUser->userNameLength = userNameLength;
    pUser->userNumber = pDbHandle->authNextUserNumber++;
    tmwdlist_addEntry(&pDbHandle->authUsers, (TMWDLIST_MEMBER *)pUser);
  }

  return pUser->userNumber;
}
#endif

/* function: sdnpsim_authUpdateKeyChgReqRcvd */ 
TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpsim_authUpdateKeyChgReqRcvd( 
  void            *pHandle,
  TMWTYPES_CHAR   *pUserName,
  TMWTYPES_USHORT  userNameLength)
{  
  SDNPSIM_AUTHUSER *pUser = TMWDEFS_NULL;
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;

  while((pUser = (SDNPSIM_AUTHUSER *)tmwdlist_getAfter(
      &pDbHandle->authUsers, (TMWDLIST_MEMBER *)pUser)) != TMWDEFS_NULL)
  {
    if(memcmp(pUser->userName, pUserName, userNameLength)==0)
    {
      /* found */
      return pUser->userNumber;
    }
  }
  return 0;
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
 
/* function: sdnpsim_addAuthSecStat */
void * TMWDEFS_GLOBAL sdnpsim_addAuthSecStat(
  void *pHandle, 
  TMWTYPES_USHORT index,
  TMWDEFS_CLASS_MASK classMask, 
  TMWTYPES_UCHAR flags,
  TMWTYPES_ULONG value)
{
  TMWSIM_POINT *pPoint;
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;
 
  pPoint = tmwsim_tableAdd(&pDbHandle->authSecStats, index);
  if(pPoint != TMWDEFS_NULL)
  {
    tmwsim_initCounter(pPoint, pHandle, index);

    pPoint->pSCLHandle = (void*)pDbHandle->pSDNPSession;
    pPoint->flags = flags;
    /* In simulated database use same for counter and frozen counter */
    pPoint->defaultStaticVariation = 1;
    pPoint->defaultEventVariation = 1; 
    pPoint->eventMode = (TMWTYPES_UCHAR)TMWDEFS_EVENT_MODE_SOE;
    pPoint->classMask = classMask;
    pPoint->data.counter.value = value; 
    sdnputil_getDateTime((TMWSESN*)pPoint->pSCLHandle, &pPoint->timeStamp);
    
    tmwsim_setDescription(pPoint, (TMWTYPES_CHAR *)_securityStatisticString(index)); 

    _callCallback((TMWSIM_POINT *)pPoint, TMWSIM_POINT_ADD, DNPDEFS_OBJ_121_AUTHSECSTATS, 
      (TMWTYPES_USHORT)tmwsim_getPointNumber((TMWSIM_POINT *)pPoint));
  }

  return(pPoint);
}

/* function: sdnpsim_deleteAuthSecStat */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_deleteAuthSecStat(
  void *pHandle,
  TMWTYPES_USHORT index)
{
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;
  void *pSimPoint = tmwsim_tableFindPoint(&pDbHandle->authSecStats, index);
  if(pSimPoint != TMWDEFS_NULL)
  {   
    _callRemoveCallback(pDbHandle, TMWSIM_POINT_DELETE, DNPDEFS_OBJ_121_AUTHSECSTATS, (TMWTYPES_USHORT)(index));
    return(tmwsim_tableDelete(&pDbHandle->authSecStats, index)); 
  }

  return(TMWDEFS_FALSE);
}

/* function: sdnpsim_authSecStatQuantity */
TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpsim_authSecStatQuantity(
  void *pHandle)
{
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;
  /* If SAv5 is enabled, all security status SHALL be present */
  if(pDbHandle->pSDNPSession->authenticationEnabled && (pDbHandle->pSDNPSession->dnp.operateInV2Mode == TMWDEFS_FALSE))
    return(DNPAUTH_NUMBER_STATISTICS);
  else
    return (0);
}

/* function: sdnpsim_authSecStatGetPoint */
void * TMWDEFS_GLOBAL sdnpsim_authSecStatGetPoint(
  void *pHandle,
  TMWTYPES_USHORT pointNum)
{
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;
  return(tmwsim_tableFindPoint(&pDbHandle->authSecStats, pointNum));
}

/* function: sdnpsim_authSecStatEventDefVariation */
TMWTYPES_UCHAR sdnpsim_authSecStatEventDefVariation(
  void *pPoint, 
  TMWDEFS_CLASS_MASK classMask)
{
  TMWTARG_UNUSED_PARAM(classMask);
  return(tmwsim_getDefEventVariation((TMWSIM_POINT *)pPoint));
}

/* function: sdnpsim_getAuthSecStatThreshold */
TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpsim_getAuthSecStatThreshold(
  void            *pHandle,  
  TMWTYPES_USHORT  index)
{
  TMWTARG_UNUSED_PARAM(pHandle); 
  switch(index)
  {
  case  DNPAUTH_UNEXPECTED_MSG_INDEX:       
    return DNPAUTH_UNEXPECTED_MSG_THRESHOLD;

  case  DNPAUTH_AUTHOR_FAIL_INDEX:       
    return DNPAUTH_AUTHOR_FAIL_THRESHOLD;

  case  DNPAUTH_AUTHENT_FAIL_INDEX:        
    return DNPAUTH_AUTHENT_FAIL_THRESHOLD;

  case  DNPAUTH_REPLY_TIMEOUT_INDEX:        
    return DNPAUTH_REPLY_TIMEOUT_THRESHOLD;

  case  DNPAUTH_REKEY_DUETOFAIL_INDEX:        
    return DNPAUTH_REKEY_THRESHOLD;   

  case  DNPAUTH_TOTAL_MSG_SENT_INDEX:        
    return DNPAUTH_TOTAL_MSG_SENT_THRESHOLD; 

  case  DNPAUTH_TOTAL_MSG_RCVD_INDEX:        
    return DNPAUTH_TOTAL_MSG_RCVD_THRESHOLD;

  case  DNPAUTH_CRIT_MSG_RCVD_INDEX:        
    return DNPAUTH_CRIT_MSG_SENT_THRESHOLD;

  case  DNPAUTH_DISCARDED_MSG_INDEX:        
    return DNPAUTH_CRIT_MSG_RCVD_THRESHOLD;  

  case  DNPAUTH_ERROR_MSG_SENT_INDEX:        
    return DNPAUTH_ERROR_MSG_SENT_THRESHOLD;   

  case  DNPAUTH_ERROR_MSG_RCVD_INDEX:        
    return DNPAUTH_ERROR_MSG_RCVD_THRESHOLD;  

  case  DNPAUTH_SUCCESS_AUTHENT_INDEX:      
    return DNPAUTH_SUCCESS_AUTHENT_THRESHOLD; 

  case  DNPAUTH_SESKEY_CHANGE_INDEX:        
    return DNPAUTH_SESKEY_CHANGE_THRESHOLD;    

  case  DNPAUTH_FAILSESKEY_CHANGE_INDEX:        
    return DNPAUTH_FAILSESKEY_CHANGE_THRESHOLD;    

  case  DNPAUTH_UPDKEY_CHANGE_INDEX:        
    return DNPAUTH_UPDKEY_CHANGE_THRESHOLD;  

  case  DNPAUTH_FAILUPDKEY_CHANGE_INDEX:        
    return DNPAUTH_FAILUPDKEY_CHANGE_THRESHOLD; 

  case  DNPAUTH_REKEY_DUE_RESTART_INDEX:        
    return DNPAUTH_REKEY_DUE_RESTART_THRESHOLD;    
  }
  return(0);
}

/* function: sdnpsim_authSecStatRead */
TMWDEFS_SCL_API void sdnpsim_authSecStatRead(
  void *pPoint, 
  TMWTYPES_ULONG *pValue, 
  TMWTYPES_UCHAR *pFlags)
{
  *pFlags = tmwsim_getFlags((TMWSIM_POINT *)pPoint);
  *pValue = tmwsim_getCounterValue((TMWSIM_POINT *)pPoint);
  return;
}

TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_authSecStatSet(
  void            *pPoint,
  TMWTYPES_ULONG  value)
{ 
  TMWSIM_POINT *pSimPoint = (TMWSIM_POINT *)pPoint;
  tmwsim_setCounterValue(pSimPoint, value, TMWDEFS_CHANGE_LOCAL_OP);
  sdnputil_getDateTime((TMWSESN*)pSimPoint->pSCLHandle, &pSimPoint->timeStamp);

  _callCallback(pSimPoint, TMWSIM_POINT_UPDATE, DNPDEFS_OBJ_121_AUTHSECSTATS, 
    (TMWTYPES_USHORT)tmwsim_getPointNumber(pSimPoint));

  return(TMWDEFS_TRUE);
}
 
#if SDNPCNFG_SUPPORT_SA_VERSION2

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

/* function: sdnpsim_authDecryptKeyWrapData */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_authDecryptKeyWrapData(
  void            *pHandle,
  TMWTYPES_USHORT  userNumber,
  TMWTYPES_UCHAR   algorithm,
  TMWTYPES_UCHAR  *pEncryptedData, 
  TMWTYPES_USHORT  encryptedValueLength, 
  TMWTYPES_UCHAR  *pPlainValue,
  TMWTYPES_USHORT *pPlainLength)
{
  TMWTARG_UNUSED_PARAM(userNumber);
  TMWTARG_UNUSED_PARAM(algorithm);

  #if TMWCNFG_SUPPORT_CRYPTO 
  {
    /*  If TMWCRYPTO interface is supported (it is required for SAv5) use it for SAv2. */
    SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;
    TMWCRYPTO_KEY cryptoKey;
    cryptoKey.ivLength =0;
    cryptoKey.length = 16;
    /* These are the default key values used by the test harness */
    if(userNumber == 1) 
      memcpy(cryptoKey.value, defaultUserKey1, 16);
    else 
      memcpy(cryptoKey.value, defaultUserKeyOther, 16); 

    return tmwcrypto_decryptData(pDbHandle->pSDNPSession->dnp.pCryptoHandle, TMWCRYPTO_ALG_KEYWRAP_AES128,
       &cryptoKey, pEncryptedData, encryptedValueLength, pPlainValue, pPlainLength);
  }
#else
  TMWTARG_UNUSED_PARAM(pHandle);
  /* Just return the encrypted data for simulation purposes */
  memcpy(pPlainValue, pEncryptedData, encryptedValueLength);
  *pPlainLength = encryptedValueLength;
  return TMWDEFS_TRUE;
#endif
}

TMWTYPES_CHAR sdnpsim_authHMACSupport(
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

/* function: sdnpsim_authHMACValue */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_authHMACValue(
  TMWTYPES_UCHAR    algorithm,
  DNPDATA_AUTH_KEY *pKey,
  TMWTYPES_UCHAR   *pData,
  TMWTYPES_ULONG   dataLength,
  TMWTYPES_UCHAR   *pHMACValue,
  TMWTYPES_USHORT  *pHMACValueLength)
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
    length = 8;
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
     &cryptoKey, length, pData, dataLength, pHMACValue, pHMACValueLength);
#else
    memcpy(pHMACValue, pData, length);
    *pHMACValueLength = length;
    return TMWDEFS_TRUE;
#endif
  }

  return TMWDEFS_FALSE;
}

/* function: sdnpsim_authRandomChallengeData */ 
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_authRandomChallengeData(
  TMWTYPES_USHORT  minLength,
  TMWTYPES_UCHAR  *pBuf,
  TMWTYPES_USHORT *pLength)
{
#if TMWCNFG_SUPPORT_CRYPTO 
  /*  If TMWCRYPTO interface is supported (it is required for SAv5) use it for SAv2. */
  return tmwcrypto_getRandomData(TMWDEFS_NULL, minLength, pBuf, pLength); 
#else
  /* Just return some data for simulation purposes */
  memcpy(pBuf, "1234567890123456789012345678901234567890", minLength);
  *pLength = minLength;
  return(TMWDEFS_TRUE);
#endif
}
#endif
#endif

#if TMWCNFG_SUPPORT_DIAG
/* function: _classMaskToString */
static const char * TMWDEFS_LOCAL _classMaskToString(TMWDEFS_CLASS_MASK classMask)
{
  switch(classMask)
  {
  case TMWDEFS_CLASS_MASK_ONE:
    return "1";

  case TMWDEFS_CLASS_MASK_TWO:
    return "2";

  case TMWDEFS_CLASS_MASK_THREE:
    return "3";

  default:
    return "none";
  }
}
/* routine: sdnpsim_showData */
void TMWDEFS_GLOBAL sdnpsim_showData(TMWSESN *pSession)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  TMWTYPES_USHORT pointNum;
  TMWTYPES_USHORT quantity;
  TMWDIAG_ANLZ_ID anlzId;
  TMWTYPES_CHAR buf[1024];
  void *pPoint;
#if SDNPDATA_SUPPORT_OBJ0
  SDNPSIM_DATABASE *pDbHandle;
#endif

  if(pSDNPSession == TMWDEFS_NULL)
    return;
 
  if(tmwdiag_initId(&anlzId, TMWDEFS_NULL, pSession, TMWDEFS_NULL, TMWDIAG_ID_MMI) == TMWDEFS_FALSE)
  {
    return;
  }

  tmwdiag_skipLine(&anlzId);
  tmwdiag_putLine(&anlzId, "Binary Inputs:\n");

  quantity = sdnpsim_binInQuantity(pSDNPSession->pDbHandle);
  for(pointNum = 0; pointNum < quantity; pointNum++)
  {
    if((pPoint = sdnpsim_binInGetPoint(pSDNPSession->pDbHandle, pointNum)) != TMWDEFS_NULL)
    {
      TMWDEFS_CLASS_MASK classMask;
      TMWTYPES_UCHAR flags;

      sdnpdata_binInRead(pPoint, &flags);
      classMask = sdnpsim_getEventClass(pPoint);

      tmwtarg_snprintf(buf, sizeof(buf), "   %06d: value %s, flags 0x%x, class %s\n", 
        pointNum, ((flags & 0x80) ? "1" : "0"), flags & 0x7f, _classMaskToString(classMask)); 

      tmwdiag_putLine(&anlzId, buf);
    }
  }


  tmwdiag_skipLine(&anlzId);
  tmwdiag_putLine(&anlzId, "Binary Outputs:\n");

  quantity = sdnpsim_binOutQuantity(pSDNPSession->pDbHandle);
  for(pointNum = 0; pointNum < quantity; pointNum++)
  {
    if((pPoint = sdnpsim_binOutGetPoint(pSDNPSession->pDbHandle, pointNum)) != TMWDEFS_NULL)
    {
      TMWTYPES_UCHAR flags;

      sdnpdata_binOutRead(pPoint, &flags);

      tmwtarg_snprintf(buf, sizeof(buf), "   %06d: value %s, flags 0x%x\n", 
        pointNum, ((flags & 0x80) ? "1" : "0"), flags & 0x7f); 

      tmwdiag_putLine(&anlzId, buf);
    }
  }

#if SDNPDATA_SUPPORT_OBJ3
  tmwdiag_skipLine(&anlzId);
  tmwdiag_putLine(&anlzId, "Double Bit Inputs:\n");

  quantity = sdnpsim_dblInQuantity(pSDNPSession->pDbHandle);
  for(pointNum = 0; pointNum < quantity; pointNum++)
  {
    if((pPoint = sdnpsim_dblInGetPoint(pSDNPSession->pDbHandle, pointNum)) != TMWDEFS_NULL)
    {
      TMWTYPES_UCHAR value;
      TMWTYPES_UCHAR flags;
      char *valuePtr;

      sdnpsim_dblInRead(pPoint, &flags);

      value = (flags & 0xc0)>>6;
      if(value == 0)
        valuePtr = "intermediate";
      else if(value == 1)
        valuePtr = "off";
      else if(value == 2)
        valuePtr = "on";
      else
        valuePtr = "indeterminate";
       
      tmwtarg_snprintf(buf, sizeof(buf), "   %06d: value %s, flags 0x%x\n", 
          pointNum, valuePtr, flags & 0x3f); 
  
      tmwdiag_putLine(&anlzId, buf);
    }
  }
#endif


#if SDNPDATA_SUPPORT_OBJ20
  tmwdiag_skipLine(&anlzId);
  tmwdiag_putLine(&anlzId, "Binary Counters:\n");

  quantity = sdnpsim_binCntrQuantity(pSDNPSession->pDbHandle);
  for(pointNum = 0; pointNum < quantity; pointNum++)
  {
    if((pPoint = sdnpsim_binCntrGetEnabledPoint(pSDNPSession->pDbHandle, pointNum)) != TMWDEFS_NULL)
    {
      TMWDEFS_CLASS_MASK classMask;
      TMWTYPES_ULONG value;
      TMWTYPES_UCHAR flags;

      sdnpsim_binCntrRead(pPoint, &value, &flags);
      classMask = sdnpsim_getEventClass(pPoint);

      tmwtarg_snprintf(buf, sizeof(buf), "   %06d: value %u, flags 0x%x, class %s\n", 
        pointNum, value, flags, _classMaskToString(classMask)); 

      tmwdiag_putLine(&anlzId, buf);
    }
  }
#endif

#if SDNPDATA_SUPPORT_OBJ21
  tmwdiag_skipLine(&anlzId);
  tmwdiag_putLine(&anlzId, "Frozen Counters:\n");

  quantity = sdnpsim_frznCntrQuantity(pSDNPSession->pDbHandle);
  for(pointNum = 0; pointNum < quantity; pointNum++)
  {
    if((pPoint = sdnpsim_frznCntrGetEnabledPoint(pSDNPSession->pDbHandle, pointNum)) != TMWDEFS_NULL)
    {
      TMWDEFS_CLASS_MASK classMask;
      TMWDTIME timeOfFreeze;
      TMWTYPES_ULONG value;
      TMWTYPES_UCHAR flags;
      TMWTYPES_CHAR timeBuf[64];

      sdnpsim_frznCntrRead(pPoint, &value, &flags, &timeOfFreeze);
      classMask = sdnpsim_getEventClass(pPoint);
      tmwdiag_time2string(&timeOfFreeze, TMWDEFS_TIME_FORMAT_56, timeBuf, sizeof(timeBuf), TMWDEFS_FALSE);

      tmwtarg_snprintf(buf, sizeof(buf), "   %06d: value %u, flags 0x%x, class %s, time of freeze %s\n", 
        pointNum, value, flags, _classMaskToString(classMask), timeBuf); 

      tmwdiag_putLine(&anlzId, buf);
    }
  }
#endif

#if SDNPDATA_SUPPORT_OBJ30
  tmwdiag_skipLine(&anlzId);
  tmwdiag_putLine(&anlzId, "Analog Inputs:\n");

  quantity = sdnpsim_anlgInQuantity(pSDNPSession->pDbHandle);
  for(pointNum = 0; pointNum < quantity; pointNum++)
  {
    if((pPoint = sdnpsim_anlgInGetPoint(pSDNPSession->pDbHandle, pointNum)) != TMWDEFS_NULL)
    {
      TMWDEFS_CLASS_MASK classMask;
      TMWTYPES_ANALOG_VALUE deadband;
      TMWTYPES_ANALOG_VALUE value;
      TMWTYPES_UCHAR flags;

      sdnpsim_anlgInRead(pPoint, &value, &flags);
      classMask = sdnpsim_getEventClass(pPoint);

#if SDNPDATA_SUPPORT_OBJ34
      pPoint = sdnpsim_anlgInGetEnabledPoint(pSDNPSession->pDbHandle, pointNum);
      sdnpsim_anlgInGetDBand(pPoint, &deadband); 
#else
      deadband.type = TMWTYPES_ANALOG_TYPE_ULONG;
      deadband.value.ulval = 0; 
#endif

#if TMWCNFG_SUPPORT_DOUBLE
      tmwtarg_snprintf(buf, sizeof(buf), "   %06d: value %.10g, flags 0x%x, class %s, deadband %.10g\n", 
        pointNum, dnputil_getAnalogValueDouble(&value), flags, _classMaskToString(classMask), 
        dnputil_getAnlgDBandValueDouble(&deadband)); 
#elif TMWCNFG_SUPPORT_FLOAT
      tmwtarg_snprintf(buf, sizeof(buf), "   %06d: value %.10g, flags 0x%x, class %s, deadband %.10g\n", 
        pointNum, dnputil_getAnalogValueFloat(&value, &flags), flags, _classMaskToString(classMask), 
        dnputil_getAnlgDBandValueFloat(&deadband)); 
#else
      tmwtarg_snprintf(buf, sizeof(buf), "   %06d: value %d, flags 0x%x, class %s, deadband %d\n", 
        pointNum, dnputil_getAnalogValueLong(&value, &flags), flags, _classMaskToString(classMask), 
        dnputil_getAnlgDBandValueULong(&deadband)); 
#endif

      tmwdiag_putLine(&anlzId, buf);
    }
  }
#endif

#if SDNPDATA_SUPPORT_OBJ31
  tmwdiag_skipLine(&anlzId);
  tmwdiag_putLine(&anlzId, "Frozen Analog Inputs:\n");

  quantity = sdnpsim_frznAnlgInQuantity(pSDNPSession->pDbHandle);
  for(pointNum = 0; pointNum < quantity; pointNum++)
  {
    if((pPoint = sdnpsim_frznAnlgInGetEnabledPoint(pSDNPSession->pDbHandle, pointNum)) != TMWDEFS_NULL)
    {
      TMWDEFS_CLASS_MASK classMask;
      TMWDTIME timeOfFreeze;
      TMWTYPES_ANALOG_VALUE value;
      TMWTYPES_UCHAR flags;
      TMWTYPES_CHAR timeBuf[64];

      sdnpsim_frznAnlgInRead(pPoint, &value, &flags, &timeOfFreeze);
      classMask = sdnpsim_getEventClass(pPoint);
      tmwdiag_time2string(&timeOfFreeze, TMWDEFS_TIME_FORMAT_56, timeBuf, sizeof(timeBuf), TMWDEFS_FALSE);

#if TMWCNFG_SUPPORT_DOUBLE
      tmwtarg_snprintf(buf, sizeof(buf), "   %06d: value %.10g, flags 0x%x, class %s, time of freeze %s\n", 
        pointNum, dnputil_getAnalogValueDouble(&value), flags, _classMaskToString(classMask), timeBuf);
#elif TMWCNFG_SUPPORT_FLOAT
      tmwtarg_snprintf(buf, sizeof(buf), "   %06d: value %.10g, flags 0x%x, class %s, time of freeze %s\n", 
        pointNum, dnputil_getAnalogValueFloat(&value, &flags), flags, _classMaskToString(classMask), timeBuf);
#else
      tmwtarg_snprintf(buf, sizeof(buf), "   %06d: value %d, flags 0x%x, class %s, time of freeze %s\n", 
        pointNum, dnputil_getAnalogValueLong(&value, &flags), flags, _classMaskToString(classMask), timeBuf);
#endif

      tmwdiag_putLine(&anlzId, buf);
    }
  }
#endif

#if (SDNPDATA_SUPPORT_OBJ40 || SDNPDATA_SUPPORT_OBJ41)
  tmwdiag_skipLine(&anlzId);
  tmwdiag_putLine(&anlzId, "Analog Outputs:\n");

  quantity = sdnpsim_anlgOutQuantity(pSDNPSession->pDbHandle);
  for(pointNum = 0; pointNum < quantity; pointNum++)
  {
    if((pPoint = sdnpsim_anlgOutGetPoint(pSDNPSession->pDbHandle, pointNum)) != TMWDEFS_NULL)
    {
      TMWTYPES_ANALOG_VALUE value;
      TMWTYPES_UCHAR flags;

      sdnpsim_anlgOutRead(pPoint, &value, &flags);
#if TMWCNFG_SUPPORT_DOUBLE
      tmwtarg_snprintf(buf, sizeof(buf), "   %06d: value %.10g, flags 0x%x\n", pointNum, 
        dnputil_getAnalogValueDouble(&value), flags); 
#elif TMWCNFG_SUPPORT_FLOAT
      tmwtarg_snprintf(buf, sizeof(buf), "   %06d: value %.10g, flags 0x%x\n", pointNum, 
        dnputil_getAnalogValueFloat(&value, &flags), flags); 
#else
      tmwtarg_snprintf(buf, sizeof(buf), "   %06d: value %d, flags 0x%x\n", pointNum, 
        dnputil_getAnalogValueLong(&value, &flags), flags); 
#endif

      tmwdiag_putLine(&anlzId, buf);
    }
  }
#endif

#if SDNPDATA_SUPPORT_OBJ50
  tmwdiag_skipLine(&anlzId);
  tmwdiag_putLine(&anlzId, "Indexed Absolute Time:\n");

  quantity = sdnpsim_indexedTimeQuantity(pSDNPSession->pDbHandle);
  for (pointNum = 0; pointNum < quantity; pointNum++)
  {
    if ((pPoint = sdnpsim_indexedTimeGetEnabledPoint(pSDNPSession->pDbHandle, pointNum)) != TMWDEFS_NULL)
    {
      TMWDTIME indexedTime;
      TMWTYPES_ULONG intervalCount;
      TMWTYPES_UCHAR intervalUnit;
      TMWTYPES_CHAR timeBuf[64];

      sdnpsim_indexedTimeRead(pPoint, &indexedTime, &intervalCount, &intervalUnit);
      tmwdiag_time2string(&indexedTime, TMWDEFS_TIME_FORMAT_56, timeBuf, sizeof(timeBuf), TMWDEFS_FALSE);
      
      tmwtarg_snprintf(buf, sizeof(buf), "   %06d: start time %s, interval count %u, interval units %d\n",
        pointNum, timeBuf, intervalCount, intervalUnit);

      tmwdiag_putLine(&anlzId, buf);
    }
  }
#endif

#if SDNPDATA_SUPPORT_OBJ0
  tmwdiag_skipLine(&anlzId);
  tmwdiag_putLine(&anlzId, "Device Attributes:\n"); 
  pPoint = TMWDEFS_NULL;
  pDbHandle = (SDNPSIM_DATABASE *)pSDNPSession->pDbHandle;

  while((pPoint = tmwsim_tableGetNextPoint(
    &pDbHandle->deviceAttrs, (TMWSIM_POINT *)pPoint)) != TMWDEFS_NULL)
  {
    TMWSIM_POINT *pDataPoint = (TMWSIM_POINT *)pPoint;
    TMWSIM_POINT *pAttribute = TMWDEFS_NULL;
    while((pAttribute = tmwsim_tableGetNextPoint(
      &pDataPoint->data.list.listHead, (TMWSIM_POINT *)pAttribute)) != TMWDEFS_NULL)
    {
      DNPDATA_ATTRIBUTE_VALUE attribute;

      sdnpsim_deviceAttrRead(pAttribute, &attribute);

      if(attribute.type == DNPDEFS_ATTRIBUTE_TYPE_VSTR)
      { 
        TMWTYPES_UCHAR tmpBuf[255];
        memcpy(tmpBuf, attribute.value.pStrValue, attribute.length);
        tmpBuf[attribute.length] = '\0';

        tmwtarg_snprintf(buf, sizeof(buf), "   Device Attribute  Point %d, Variation %3d, Property 0x%02x, %s\n",
          tmwsim_getPointNumber((TMWSIM_POINT *)pPoint), tmwsim_getPointNumber(pAttribute), pAttribute->data.attribute.property, tmpBuf);
      }
      else if((attribute.type == DNPDEFS_ATTRIBUTE_TYPE_OSTR)
        ||(attribute.type == DNPDEFS_ATTRIBUTE_TYPE_BSTR)) 
      { 
        int len;
        int j;
        TMWTYPES_CHAR tmpBuf[255];  

        len = 0;
        for(j=0; j<attribute.length; j++)
        {
          if(len > 250)
            break;
          len += tmwtarg_snprintf((tmpBuf + len), sizeof(tmpBuf)-len, "%02x ", attribute.value.pStrValue[j]); 
          }
        tmpBuf[len] = '\0';

        tmwtarg_snprintf(buf, sizeof(buf), "   Device Attribute  Point %d, Variation %3d, Property 0x%02x, %s\n",
          tmwsim_getPointNumber((TMWSIM_POINT *)pPoint), tmwsim_getPointNumber(pAttribute), pAttribute->data.attribute.property, tmpBuf);
      }
      else if(attribute.type == DNPDEFS_ATTRIBUTE_TYPE_UINT)
      { 
        tmwtarg_snprintf(buf, sizeof(buf), "   Device Attribute  Point %d, Variation %3d, Property 0x%02x, %d\n",
          tmwsim_getPointNumber((TMWSIM_POINT *)pPoint), tmwsim_getPointNumber(pAttribute), pAttribute->data.attribute.property, attribute.value.uintValue);
      }
      else if(attribute.type == DNPDEFS_ATTRIBUTE_TYPE_INT)
      { 
        tmwtarg_snprintf(buf, sizeof(buf), "   Device Attribute  Point %d, Variation %3d, Property 0x%02x, %d\n",
          tmwsim_getPointNumber((TMWSIM_POINT *)pPoint), tmwsim_getPointNumber(pAttribute), pAttribute->data.attribute.property, attribute.value.intValue);
      }
      else if(attribute.type == DNPDEFS_ATTRIBUTE_TYPE_FLT)
      { 
        if(attribute.length == 4)
          tmwtarg_snprintf(buf, sizeof(buf), "   Device Attribute  Point %d, Variation %3d, Property 0x%02x, %.10g\n",
            tmwsim_getPointNumber((TMWSIM_POINT *)pPoint), tmwsim_getPointNumber(pAttribute), pAttribute->data.attribute.property, attribute.value.fltValue.sfltValue);
        else
          tmwtarg_snprintf(buf, sizeof(buf), "   Device Attribute  Point %d, Variation %3d, Property 0x%02x, %.10g\n",
            tmwsim_getPointNumber((TMWSIM_POINT *)pPoint), tmwsim_getPointNumber(pAttribute), pAttribute->data.attribute.property, attribute.value.fltValue.doubleValue);
      }
      else if(attribute.type == DNPDEFS_ATTRIBUTE_TYPE_DNP3TIME)
      {
        TMWTYPES_CHAR timeBuf[255];
        tmwdiag_time2string(&attribute.value.timeValue, TMWDEFS_TIME_FORMAT_56, timeBuf, sizeof(timeBuf), TMWDEFS_FALSE);
        tmwtarg_snprintf(buf, sizeof(buf), "   Device Attribute  Point %d, Variation %3d, Property 0x%02x, %s\n",
          tmwsim_getPointNumber((TMWSIM_POINT *)pPoint), tmwsim_getPointNumber(pAttribute), pAttribute->data.attribute.property, timeBuf);
      }
      tmwdiag_putLine(&anlzId, buf);
    }
  }
#endif

#if SDNPDATA_SUPPORT_DATASETS
  tmwdiag_skipLine(&anlzId);
  tmwdiag_putLine(&anlzId, "Data Set Prototypes:\n");

  quantity = sdnpsim_datasetProtoQuantity(pSDNPSession->pDbHandle);
  for(pointNum = 0; pointNum < quantity; pointNum++)
  {
    DNPDATA_DATASET_DESCR_ELEM *pDescrElem;
    TMWTYPES_UCHAR numberElems;
    TMWTYPES_UCHAR strbuf[128];
    if(((pPoint = sdnpsim_datasetProtoGetPoint(pSDNPSession->pDbHandle, pointNum)) != TMWDEFS_NULL)
      && ((pDescrElem = sdnpsim_datasetProtoRead(pPoint, &numberElems, strbuf)) != TMWDEFS_NULL))
    { 
      int i;
      tmwtarg_snprintf(buf, sizeof(buf), "   Data Set Prototype ID %06d\n", pointNum); 
      tmwdiag_putLine(&anlzId, buf); 

      tmwtarg_snprintf(buf, sizeof(buf), "     Element%3d, UUID= ", 1); 
      tmwdiag_putLine(&anlzId, buf);
      dnpdiag_displayOctets(&anlzId, strbuf, 16, TMWDEFS_FALSE);    

      for(i=0; i < numberElems; i++)
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
        if(_isPrintable(strbuf, pDescrElem->ancillaryValue.length))
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
  }

  tmwdiag_skipLine(&anlzId);
  tmwdiag_putLine(&anlzId, "Data Set Descriptors:\n");

  quantity = sdnpsim_datasetDescrQuantity(pSDNPSession->pDbHandle);
  for(pointNum = 0; pointNum < quantity; pointNum++)
  {
    DNPDATA_DATASET_DESCR_ELEM *pDescrElem;
    DNPDATA_DATASET_DESCR_INDEX *pDescrIndex;
    TMWTYPES_UCHAR numberElems;
    TMWTYPES_UCHAR numberIndexElems;
    TMWTYPES_UCHAR strbuf[128];

    if(((pPoint = sdnpsim_datasetDescrGetPoint(pSDNPSession->pDbHandle, pointNum)) != TMWDEFS_NULL)
       && ((pDescrElem = sdnpsim_datasetDescrReadCont(pPoint, &numberElems)) != TMWDEFS_NULL))
    { 
      int i;
      TMWTYPES_UCHAR characteristics;
      sdnpsim_datasetDescrReadChars(pPoint, &characteristics);
      tmwtarg_snprintf(buf, sizeof(buf), "   Data Set Descriptor ID %06d characteristics 0x%0x\n", 
        pointNum, characteristics); 

      tmwdiag_putLine(&anlzId, buf);
      for(i=0; i < numberElems; i++)
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
          dnpdiag_displayOctets(&anlzId, strbuf, 16, TMWDEFS_FALSE);  

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
    }

    if(((pPoint = sdnpsim_datasetDescrGetPoint(pSDNPSession->pDbHandle, pointNum)) != TMWDEFS_NULL)
      && ((pDescrIndex = sdnpsim_datasetDescrReadIndex(pPoint, &numberIndexElems)) != TMWDEFS_NULL))
    { 
      int i;
      for(i=0; i < numberIndexElems; i++)
      {
        tmwtarg_snprintf(buf, sizeof(buf), "     Element%3d, Point Type %3d  Index %d\n", 
          i+1, pDescrIndex->pointType, pDescrIndex->pointIndex); 
        tmwdiag_putLine(&anlzId, buf);
        pDescrIndex++;
      }
    }
  }

  tmwdiag_skipLine(&anlzId);
  tmwdiag_putLine(&anlzId, "Data Sets:\n");

  quantity = sdnpsim_datasetQuantity(pSDNPSession->pDbHandle);
  for(pointNum = 0; pointNum < quantity; pointNum++)
  {
    DNPDATA_DATASET_VALUE *pElem;
    TMWTYPES_UCHAR numberElems;
    TMWDTIME timeStamp;
    if(((pPoint = sdnpsim_datasetGetPoint(pSDNPSession->pDbHandle, pointNum)) != TMWDEFS_NULL)
      && ((pElem = sdnpsim_datasetRead(pPoint, &numberElems, &timeStamp)) != TMWDEFS_NULL))
    { 
      SDNPSIM_DATASET_DESCR_DATA *pDescr;
      int i;
      char strbuf[128];

      pDescr = (SDNPSIM_DATASET_DESCR_DATA*)pPoint;

      tmwtarg_snprintf(buf, sizeof(buf), "   Data Set ID %06d\n", pointNum); 
      tmwdiag_putLine(&anlzId, buf);

      tmwdiag_time2string(&timeStamp, TMWDEFS_TIME_FORMAT_56, strbuf, sizeof(strbuf), TMWDEFS_FALSE);
      tmwtarg_snprintf(buf, sizeof(buf), "     Element%3d, %s\n", 1, strbuf); 
      tmwdiag_putLine(&anlzId, buf);

      for(i=0; i < numberElems; i++)
      {
        TMWTYPES_BOOL indent;
        switch(pElem->type)
        {
        case DNPDATA_VALUE_STRPTR:   
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
          tmwtarg_snprintf(buf, sizeof(buf), "     Element%3d, len=%d  %d\n", 
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
  }

#endif
#if SDNPDATA_SUPPORT_OBJ110
  tmwdiag_skipLine(&anlzId);
  tmwdiag_putLine(&anlzId, "String Data:\n");

  quantity = sdnpsim_strQuantity(pSDNPSession->pDbHandle);
  for(pointNum = 0; pointNum < quantity; pointNum++)
  {
    if((pPoint = sdnpsim_strGetEnabledPoint(pSDNPSession->pDbHandle, pointNum)) != TMWDEFS_NULL)
    {
      TMWDEFS_CLASS_MASK classMask;
      TMWTYPES_UCHAR strBuf[DNPDEFS_MAX_STRING_LENGTH+1];
      TMWTYPES_UCHAR strLength;

      sdnpsim_strRead(pPoint, DNPDEFS_MAX_STRING_LENGTH, strBuf, &strLength);
      strBuf[strLength] = '\0';
      classMask = sdnpsim_getEventClass(pPoint);

      tmwtarg_snprintf(buf, sizeof(buf), "   %06d: value '%s', class %s\n", 
        pointNum, strBuf, _classMaskToString(classMask)); 

      tmwdiag_putLine(&anlzId, buf);
    }
  }
#endif /* SDNPDATA_SUPPORT_OBJ110 */

#if SDNPDATA_SUPPORT_OBJ112
  tmwdiag_skipLine(&anlzId);
  tmwdiag_putLine(&anlzId, "Virtual Terminal:\n");

  quantity = sdnpsim_vtermQuantity(pSDNPSession->pDbHandle);
  for(pointNum = 0; pointNum < quantity; pointNum++)
  {
    if((pPoint = sdnpsim_vtermGetPoint(pSDNPSession->pDbHandle, pointNum)) != TMWDEFS_NULL)
    {
      TMWDEFS_CLASS_MASK classMask;
      TMWTYPES_UCHAR strBuf[DNPDEFS_MAX_STRING_LENGTH+1];
      TMWTYPES_UCHAR strLength;

      sdnpsim_vtermRead(pPoint, DNPDEFS_MAX_STRING_LENGTH, strBuf, &strLength);
      /* Make sure string is NULL terminated */
      strBuf[strLength] = '\0';
      classMask = sdnpsim_getEventClass(pPoint);

      tmwtarg_snprintf(buf, sizeof(buf), "   %06d: value '%s', class %s\n", 
        pointNum, strBuf, _classMaskToString(classMask)); 

      tmwdiag_putLine(&anlzId, buf);
    }
  }
#endif /* SDNPDATA_SUPPORT_OBJ112 */

#if SDNPDATA_SUPPORT_OBJ114
  tmwdiag_skipLine(&anlzId);
  tmwdiag_putLine(&anlzId, "Extended String Data:\n");

  quantity = sdnpsim_extStrQuantity(pSDNPSession->pDbHandle);
  for(pointNum = 0; pointNum < quantity; pointNum++)
  {
    if((pPoint = sdnpsim_extStrGetEnabledPoint(pSDNPSession->pDbHandle, pointNum)) != TMWDEFS_NULL)
    {
      TMWDEFS_CLASS_MASK classMask;
      TMWTYPES_UCHAR strBuf[DNPCNFG_MAX_DIAG_STRING_LENGTH + 8];
      TMWTYPES_USHORT strLength;
      TMWTYPES_UCHAR flags;

      sdnpsim_extStrRead(pPoint, DNPCNFG_MAX_DIAG_STRING_LENGTH+1, strBuf, &strLength, &flags);
      if (strLength > DNPCNFG_MAX_DIAG_STRING_LENGTH)
      {
        strLength = DNPCNFG_MAX_DIAG_STRING_LENGTH;
        strBuf[strLength++] = '.';
        strBuf[strLength++] = '.';
        strBuf[strLength++] = '.';
      }
      strBuf[strLength] = '\0';
      classMask = sdnpsim_getEventClass(pPoint);

      tmwtarg_snprintf(buf, sizeof(buf), "   %06d: value '%s', class %s\n", 
        pointNum, strBuf, _classMaskToString(classMask)); 

      tmwdiag_putLine(&anlzId, buf);
    }
  }
#endif /* SDNPDATA_SUPPORT_OBJ114 */

#if SDNPDATA_SUPPORT_OBJ120
  tmwdiag_skipLine(&anlzId);
  tmwdiag_putLine(&anlzId, "Security Statistics:\n");

  quantity = sdnpsim_authSecStatQuantity(pSDNPSession->pDbHandle);
  for(pointNum = 0; pointNum < quantity; pointNum++)
  {
    if((pPoint = sdnpsim_authSecStatGetPoint(pSDNPSession->pDbHandle, pointNum)) != TMWDEFS_NULL)
    {
      TMWDEFS_CLASS_MASK classMask;
      TMWTYPES_ULONG value;
      TMWTYPES_UCHAR flags;

      sdnpsim_authSecStatRead(pPoint, &value, &flags);
      classMask = sdnpsim_getEventClass(pPoint);

      tmwtarg_snprintf(buf, sizeof(buf), "   %06d: value %u, flags 0x%x, class %s\n", 
        pointNum, value, flags, _classMaskToString(classMask)); 

      tmwdiag_putLine(&anlzId, buf);
    }
  }
#endif
}
#endif

#if SDNPCNFG_USER_MANAGED_EVENTS
/* Hardcode the max event queue size to 10 for now*/
#define TMWSIM_MAX_EVENTS 10

/* function: _getEventQueue */
static TMWDLIST *_getEventQueue(
  SDNPSIM_DATABASE *pDbHandle, 
  TMWTYPES_UCHAR group)
{
  TMWDLIST  *pEventQueue = TMWDEFS_NULL;

  switch(group)
  {
    /* Binary and Double Bit Binary are kept in the same queue
     * so that a read of both will return those events intermixed in
     * order of occurrence
     */
  case DNPDEFS_OBJ_2_BIN_CHNG_EVENTS:
  case DNPDEFS_OBJ_4_DBL_CHNG_EVENTS:
    pEventQueue = &pDbHandle->binInEvents;
    break;

  case DNPDEFS_OBJ_11_BIN_OUT_EVENTS:  
    pEventQueue = &pDbHandle->binOutEvents;
    break;

  case DNPDEFS_OBJ_13_BIN_CMD_EVENTS:  
    pEventQueue = &pDbHandle->binCmdEvents;
    break;

  case DNPDEFS_OBJ_22_CNTR_EVENTS:  
    pEventQueue = &pDbHandle->binCntrEvents;
    break;

  case DNPDEFS_OBJ_23_FCTR_EVENTS:  
    pEventQueue = &pDbHandle->frznCntrEvents;
    break;

  case DNPDEFS_OBJ_32_ANA_CHNG_EVENTS:
    pEventQueue = &pDbHandle->anlgInEvents;
    break;

  case DNPDEFS_OBJ_33_FRZN_ANA_EVENTS:  
    pEventQueue = &pDbHandle->frznAnlgInEvents;
    break;

  case DNPDEFS_OBJ_42_ANA_OUT_EVENTS:  
    pEventQueue = &pDbHandle->anlgOutEvents;
    break;

  case DNPDEFS_OBJ_43_ANA_CMD_EVENTS:  
    pEventQueue = &pDbHandle->anlgCmdEvents;
    break;

  case DNPDEFS_OBJ_88_DATASET_EVENTS:
    pEventQueue = &pDbHandle->datasetEvents;
    break;

  case DNPDEFS_OBJ_111_STRING_EVENTS:
    pEventQueue = &pDbHandle->stringEvents;
    break; 

  case DNPDEFS_OBJ_113_VTERM_EVENTS:
    pEventQueue = &pDbHandle->vtermEvents;
    break;

  case DNPDEFS_OBJ_115_EXT_STR_EVENTS:
    pEventQueue = &pDbHandle->extendedStringEvents;
    break;

  case DNPDEFS_OBJ_120_AUTHENTICATION:
    pEventQueue = &pDbHandle->authErrorEvents;
    break;

  case DNPDEFS_OBJ_122_AUTHSTATEVENTS:
    pEventQueue = &pDbHandle->authStatEvents;
    break;
  }
  return(pEventQueue);
}

static void TMWDEFS_LOCAL _storeDatasetEvent(
  SDNPSIM_EVENT *pEvent, 
  SDNPDATA_ADD_EVENT_VALUE *pData)
{
  int i;
  DNPDATA_DATASET_VALUE *pEventElem = (DNPDATA_DATASET_VALUE *)pEvent->value.datasetValue.data;
  DNPDATA_DATASET_VALUE *pAddElem = (DNPDATA_DATASET_VALUE *)pData->dataset.pData;
  TMWTYPES_UCHAR *pEnd = pEvent->value.datasetValue.data + sizeof(pEvent->value.datasetValue.data);

  for(i=0; i< pData->dataset.numberElems; i++)
  {
    *pEventElem = *pAddElem;
    if(pAddElem->type == DNPDATA_VALUE_STRPTR)
    {
      TMWTYPES_USHORT len = pAddElem->length;
      pEnd -= len;
      memcpy(pEnd, pAddElem->value.pStrValue, len);
      pEventElem->value.pStrValue = pEnd;
    }
    pEventElem++;
    pAddElem++;
  }
  pEvent->value.datasetValue.numberElems = pData->dataset.numberElems;
}

/* function: _countEvents */
TMWTYPES_ULONG TMWDEFS_LOCAL _countEvents(
  void *pHandle, 
  TMWTYPES_UCHAR group)
{
  TMWDLIST         *pEventQueue;
  SDNPSIM_EVENT    *pEvent = TMWDEFS_NULL;
  int               count = 0;
  pEventQueue = _getEventQueue((SDNPSIM_DATABASE *)pHandle, group);
  if (pEventQueue != TMWDEFS_NULL)
  {
    while ((pEvent = (SDNPSIM_EVENT *)tmwdlist_getAfter(
      pEventQueue, (TMWDLIST_MEMBER *)pEvent)) != TMWDEFS_NULL)
    {
      if (pEvent->group & group)
      {
        count++;
      }
    }
  }
  return((TMWTYPES_USHORT)count);
}

/* function: sdnpsim_umEventAdd */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_umEventAdd(
  void *pHandle,
  TMWTYPES_UCHAR group,
  TMWTYPES_USHORT point,
  TMWDEFS_CLASS_MASK classMask,
  TMWTYPES_UCHAR defaultVariation,
  TMWTYPES_UCHAR flags,
  SDNPDATA_ADD_EVENT_VALUE *pValue,
  TMWDTIME *pTimeStamp)
{
  SDNPSIM_EVENT    *pEvent;
  TMWDLIST         *pEventQueue;

  pEventQueue = _getEventQueue((SDNPSIM_DATABASE *)pHandle, group);
  if(pEventQueue == TMWDEFS_NULL)
  {
    return(TMWDEFS_FALSE);
  }

  /* Hardcoded the max size for this simple sample
   * and does not support delete oldest event
   */
  if(tmwdlist_size(pEventQueue) >= TMWSIM_MAX_EVENTS)
  {
    /* Combined binary input and double binary input events in one queue */
    if (group < 5)
    {
      if (_countEvents(pHandle, group) >= TMWSIM_MAX_EVENTS)
        return(TMWDEFS_FALSE);
    }
    else
      return(TMWDEFS_FALSE);
  } 

  pEvent = (SDNPSIM_EVENT *)sdnpmem_alloc(SDNPMEM_SIM_UMEVENT_TYPE);
  if(pEvent == TMWDEFS_NULL)
  {
    return(TMWDEFS_FALSE);
  }
  pEvent->eventSent = TMWDEFS_FALSE;
  pEvent->flags = flags;
  pEvent->classMask = classMask;
  pEvent->defaultVariation = defaultVariation;
  pEvent->point = point;
  pEvent->group = group;
  pEvent->timeStamp = *pTimeStamp;

  switch(group)
  { 
  case DNPDEFS_OBJ_22_CNTR_EVENTS:  
    pEvent->value.ulValue = pValue->ulValue;
    break;

  case DNPDEFS_OBJ_23_FCTR_EVENTS:  
    pEvent->value.ulValue = pValue->ulValue;
    break;

  case DNPDEFS_OBJ_32_ANA_CHNG_EVENTS:
  case DNPDEFS_OBJ_33_FRZN_ANA_EVENTS:
  case DNPDEFS_OBJ_42_ANA_OUT_EVENTS:
  case DNPDEFS_OBJ_43_ANA_CMD_EVENTS:
    pEvent->value.analogValue = *pValue->analogPtr;
    break;

  case DNPDEFS_OBJ_88_DATASET_EVENTS:
    _storeDatasetEvent(pEvent, pValue);
    break;

  case DNPDEFS_OBJ_111_STRING_EVENTS:
  case DNPDEFS_OBJ_113_VTERM_EVENTS:
    pEvent->value.stringValue.length = (TMWTYPES_UCHAR)pValue->stringPtr.length;
    memcpy(pEvent->value.stringValue.buf, pValue->stringPtr.pBuf, 
      pValue->stringPtr.length);
    break; 

#if SDNPDATA_SUPPORT_OBJ115
  case DNPDEFS_OBJ_115_EXT_STR_EVENTS: 
    pEvent->value.extendedStringValue.length = pValue->extendedStringPtr.length;
    memcpy(pEvent->value.extendedStringValue.buf, pValue->extendedStringPtr.pBuf,
      pValue->extendedStringPtr.length);
    break;
#endif

  case DNPDEFS_OBJ_120_AUTHENTICATION:

    /* G120V7 says only the most recent Auth Error event object shall be buffered */
    if ((group == 120) && (tmwdlist_size(pEventQueue) == 1))
    {
      TMWDLIST_MEMBER *pEntry = tmwdlist_removeFirst(pEventQueue);
      sdnpmem_free(pEntry);
    }

    pEvent->value.authError.errorTextLength = pValue->authError.errorTextLength;
    memcpy(pEvent->value.authError.errorText, pValue->authError.pErrorText,
      pValue->authError.errorTextLength);
    pEvent->value.authError.assocId = pValue->authError.assocId;
    pEvent->value.authError.sequenceNumber = pValue->authError.sequenceNumber;
    pEvent->value.authError.errorCode = pValue->authError.errorCode;
    break;

  case DNPDEFS_OBJ_122_AUTHSTATEVENTS:
    pEvent->value.authSecStat.assocId = pValue->authSecStat.assocId;
    pEvent->value.authSecStat.ulValue = pValue->authSecStat.ulValue;
    break;
  }

  tmwdlist_addEntry(pEventQueue, (TMWDLIST_MEMBER *)pEvent);
  return(TMWDEFS_TRUE);
}

/* function: sdnpsim_umEventNotSentCount */
TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpsim_umEventNotSentCount(
  void *pHandle,
  TMWTYPES_UCHAR group,
  TMWDEFS_CLASS_MASK classMask)
{
  TMWDLIST         *pEventQueue;
  SDNPSIM_EVENT    *pEvent = TMWDEFS_NULL;
  int               count = 0;

  pEventQueue = _getEventQueue((SDNPSIM_DATABASE *)pHandle, group);
  if(pEventQueue != TMWDEFS_NULL)
  {
    while((pEvent = (SDNPSIM_EVENT *)tmwdlist_getAfter(
      pEventQueue, (TMWDLIST_MEMBER *)pEvent)) != TMWDEFS_NULL)
    {
      if((pEvent->classMask & classMask) 
        && (!pEvent->eventSent))
      {
        count++;
      }
    }
  }
  return((TMWTYPES_USHORT)count);
}

/* function: sdnpsim_umEventGet */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_umEventGet(
  void *pHandle,
  TMWTYPES_UCHAR group,
  TMWDEFS_CLASS_MASK classMask,
  TMWTYPES_BOOL firstEvent,
  SDNPDATA_GET_EVENT *pEvent)
{
  SDNPSIM_DATABASE *pDbHandle;
  TMWDLIST         *pEventQueue;
  SDNPSIM_EVENT    *pSimEvent = TMWDEFS_NULL;

  pDbHandle = (SDNPSIM_DATABASE *)pHandle;
  if(firstEvent)
    pDbHandle->eventIndex = 0;

  pEventQueue = _getEventQueue(pDbHandle, group);
  if(pEventQueue == TMWDEFS_NULL)
  {
    return(TMWDEFS_FALSE);
  }

  while((pSimEvent = (SDNPSIM_EVENT *)tmwdlist_getEntry(pEventQueue, pDbHandle->eventIndex)) != TMWDEFS_NULL)
  {
    pDbHandle->eventIndex++;
    if(pSimEvent->classMask & classMask)
    { 
      pDbHandle->pLastEvent = pSimEvent;

      pEvent->group            = pSimEvent->group;
      pEvent->classMask        = pSimEvent->classMask;
      pEvent->defaultVariation = pSimEvent->defaultVariation;
      pEvent->flags            = pSimEvent->flags;
      pEvent->point            = pSimEvent->point;
      pEvent->timeStamp        = pSimEvent->timeStamp;
      switch(group)
      { 
      case DNPDEFS_OBJ_22_CNTR_EVENTS:  
        if(pSimEvent != TMWDEFS_NULL)
        {
          pEvent->value.ulValue = pSimEvent->value.ulValue;
        }
        break;

      case DNPDEFS_OBJ_23_FCTR_EVENTS:  
        if(pSimEvent != TMWDEFS_NULL)
        {
          pEvent->value.ulValue = pSimEvent->value.ulValue;
        }
        break;

      case DNPDEFS_OBJ_32_ANA_CHNG_EVENTS:
      case DNPDEFS_OBJ_33_FRZN_ANA_EVENTS:
      case DNPDEFS_OBJ_42_ANA_OUT_EVENTS:
      case DNPDEFS_OBJ_43_ANA_CMD_EVENTS:
        if(pSimEvent != TMWDEFS_NULL)
        {
          pEvent->value.analogValue = pSimEvent->value.analogValue;
        }
        break;   
      
      case DNPDEFS_OBJ_88_DATASET_EVENTS:
        if(pSimEvent != TMWDEFS_NULL)
        {
          pEvent->value.datasetValue.pDataSet = (DNPDATA_DATASET_VALUE *)pSimEvent->value.datasetValue.data;
          pEvent->value.datasetValue.numberElems = pSimEvent->value.datasetValue.numberElems;
         }
        break;

      case DNPDEFS_OBJ_111_STRING_EVENTS:
        if(pSimEvent != TMWDEFS_NULL)
        {
          pEvent->value.stringValue.length = pSimEvent->value.stringValue.length;
          memcpy(pEvent->value.stringValue.buf, pSimEvent->value.stringValue.buf, 
            pSimEvent->value.stringValue.length);
        }
        break; 

      case DNPDEFS_OBJ_113_VTERM_EVENTS:
        if(pSimEvent != TMWDEFS_NULL)
        {
          pEvent->value.stringValue.length = pSimEvent->value.stringValue.length;
          memcpy(pEvent->value.stringValue.buf, pSimEvent->value.stringValue.buf, 
            pSimEvent->value.stringValue.length);
        }
        break;

#if SDNPDATA_SUPPORT_OBJ115
      case DNPDEFS_OBJ_115_EXT_STR_EVENTS:
        if (pSimEvent != TMWDEFS_NULL)
        {
          pEvent->value.extendedStringValue.length = pSimEvent->value.extendedStringValue.length;
          memcpy(pEvent->value.extendedStringValue.buf, pSimEvent->value.extendedStringValue.buf,
            pSimEvent->value.extendedStringValue.length);
        }
        break;
#endif

      case DNPDEFS_OBJ_120_AUTHENTICATION:
        if (pSimEvent != TMWDEFS_NULL)
        {
          pEvent->value.authError.errorTextLength = pSimEvent->value.authError.errorTextLength;
          memcpy(pEvent->value.authError.errorText, pSimEvent->value.authError.errorText,
            pSimEvent->value.authError.errorTextLength);
          pEvent->value.authError.assocId = pSimEvent->value.authError.assocId;
          pEvent->value.authError.sequenceNumber = pSimEvent->value.authError.sequenceNumber;
          pEvent->value.authError.errorCode = pSimEvent->value.authError.errorCode;
        }
        break;

      case DNPDEFS_OBJ_122_AUTHSTATEVENTS:
        if (pSimEvent != TMWDEFS_NULL)
        { 
          pEvent->value.authSecStat.assocId = pSimEvent->value.authSecStat.assocId;
          pEvent->value.authSecStat.ulValue = pSimEvent->value.authSecStat.ulValue;
        }
        break;
      }

      return(TMWDEFS_TRUE);
    }
  }

  return(TMWDEFS_FALSE);
}

/* function: sdnpsim_umEventSent */
void TMWDEFS_GLOBAL sdnpsim_umEventSent(
  void *pHandle,
  TMWTYPES_UCHAR group,
  TMWTYPES_USHORT point)
{
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;
  TMWTARG_UNUSED_PARAM(group);
  TMWTARG_UNUSED_PARAM(point);
  pDbHandle->pLastEvent->eventSent = TMWDEFS_TRUE;
}

TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_umEventNotSent(
  void *pHandle,
  TMWTYPES_UCHAR group)
{
  TMWDLIST         *pEventQueue;
  SDNPSIM_EVENT    *pEvent = TMWDEFS_NULL;

  pEventQueue = _getEventQueue((SDNPSIM_DATABASE *)pHandle, group);
  if(pEventQueue == TMWDEFS_NULL)
  {
    return(TMWDEFS_FALSE);
  }

  pEvent = (SDNPSIM_EVENT *)tmwdlist_getFirst(pEventQueue);
  while(pEvent != TMWDEFS_NULL)
  {
    /* Get next event now in case we delete this one */
    SDNPSIM_EVENT *pNextEvent = (SDNPSIM_EVENT *)tmwdlist_getNext((TMWDLIST_MEMBER *)pEvent);

    if(pEvent->eventSent)
    {
      pEvent->eventSent = TMWDEFS_FALSE;
    }

    pEvent = pNextEvent;
  }


  /* Hardcode the max size */
  if(tmwdlist_size(pEventQueue) >= TMWSIM_MAX_EVENTS)
  {
    return(TMWDEFS_TRUE);
  }

  return(TMWDEFS_FALSE);
}

/* function: sdnpsim_umEventRemove */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_umEventRemove(
  void *pHandle,
  TMWTYPES_UCHAR group)
{
  TMWDLIST         *pEventQueue;
  SDNPSIM_EVENT    *pEvent = TMWDEFS_NULL;

  pEventQueue = _getEventQueue((SDNPSIM_DATABASE *)pHandle, group);
  if(pEventQueue == TMWDEFS_NULL)
  {
    return(TMWDEFS_FALSE);
  }

  pEvent = (SDNPSIM_EVENT *)tmwdlist_getFirst(pEventQueue);
  while(pEvent != TMWDEFS_NULL)
  {
    /* Get next event now in case we delete this one */
    SDNPSIM_EVENT *pNextEvent = (SDNPSIM_EVENT *)tmwdlist_getNext((TMWDLIST_MEMBER *)pEvent);

    if(pEvent->eventSent)
    {
      tmwdlist_removeEntry(pEventQueue, (TMWDLIST_MEMBER *)pEvent);
      sdnpmem_free(pEvent);
    }

    pEvent = pNextEvent;
  }

  /* Hardcode the max size */
  if(tmwdlist_size(pEventQueue) >= TMWSIM_MAX_EVENTS)
  {
    return(TMWDEFS_TRUE);
  }

  /* For now report that this queue is not full */
  return(TMWDEFS_FALSE);
}
#endif

#if SDNPDATA_SUPPORT_OBJ0
/* function: sdnpsim_addDeviceAttribute */
void * TMWDEFS_GLOBAL sdnpsim_addDeviceAttribute(
  void *pHandle, 
  TMWTYPES_USHORT point,
  TMWTYPES_UCHAR  variation,
  TMWTYPES_UCHAR  property,
  DNPDATA_ATTRIBUTE_VALUE *pData)
{
  TMWSIM_POINT *pPoint;
  TMWSIM_POINT *pAttribute = TMWDEFS_NULL;
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;
  TMWTYPES_USHORT pointNum = (TMWTYPES_USHORT)tmwsim_tableSize(&pDbHandle->deviceAttrs);

  while(point >= pointNum)
  { 
    /* Create an entry for each point index (device attribute set index)
     * which allows for a list of attributes for this point
     */
    pPoint = tmwsim_tableAdd(&pDbHandle->deviceAttrs, pointNum);
    if(pPoint == TMWDEFS_NULL)
      return TMWDEFS_NULL;

    tmwsim_initPoint(pPoint, pHandle, pointNum, TMWSIM_TYPE_LIST);
    pPoint->pSCLHandle = (void*)pDbHandle->pSDNPSession;
    sdnputil_getDateTime((TMWSESN*)pPoint->pSCLHandle, &pPoint->timeStamp); 
    pointNum++;
  }
 
  pPoint = tmwsim_tableFindPoint(&pDbHandle->deviceAttrs, point);
  if(pPoint != TMWDEFS_NULL)
  { 
    /* See if this attribute already exists */
    pAttribute = tmwsim_tableFindPoint(&pPoint->data.list.listHead, variation);
    
    if(pAttribute != TMWDEFS_NULL)
      /* already exists */
      return(TMWDEFS_NULL);

    pAttribute = tmwsim_tableAdd(&pPoint->data.list.listHead, variation);
    if(pAttribute == TMWDEFS_NULL)
      return(TMWDEFS_NULL);

    tmwsim_initPoint(pAttribute, pHandle, variation, TMWSIM_TYPE_ATTRIBUTE);
    pPoint->pSCLHandle = (void*)pDbHandle->pSDNPSession;
    sdnputil_getDateTime((TMWSESN*)pPoint->pSCLHandle, &pPoint->timeStamp); 
    
    if((pData->type == DNPDEFS_ATTRIBUTE_TYPE_VSTR)
      || (pData->type == DNPDEFS_ATTRIBUTE_TYPE_OSTR)
      || (pData->type == DNPDEFS_ATTRIBUTE_TYPE_BSTR))
    { 
      pAttribute->data.attribute.pBuf = (TMWTYPES_UCHAR *)tmwmem_alloc(TMWMEM_SIM_STRING_TYPE);
      if(pAttribute->data.attribute.pBuf == TMWDEFS_NULL)
        return(TMWDEFS_NULL);

      /* Truncate string if it is too long */
      if(pData->length > TMWSIM_STRING_MAX_LENGTH)
        pData->length = TMWSIM_STRING_MAX_LENGTH;

      if(pData->length > 0)
         memcpy(pAttribute->data.attribute.pBuf, pData->value.pStrValue, pData->length);
    }
    else
    {  
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
      } 
    }
    pAttribute->data.attribute.property = property;
    pAttribute->data.attribute.length = pData->length;
    pAttribute->data.attribute.type = (TMWTYPES_UCHAR)pData->type;
    pAttribute->data.attribute.point = point;
     
    _callCallback((TMWSIM_POINT *)pPoint, TMWSIM_POINT_ADD, DNPDEFS_OBJ_0_DEVICE_ATTRIBUTES,  
      ((TMWTYPES_USHORT)(((TMWTYPES_USHORT)point << 8)|(variation & 0xff))));
  }

  return(pAttribute);
}

/* function: sdnpsim_deleteDeviceAttr */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_deleteDeviceAttr(
  void *pHandle,
  TMWTYPES_USHORT point,
  TMWTYPES_UCHAR variation)
{
  TMWSIM_POINT *pPoint;
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;

  pPoint = tmwsim_tableFindPoint(&pDbHandle->deviceAttrs, point);
  if(pPoint != TMWDEFS_NULL) 
  {  
    _callRemoveCallback(pDbHandle, TMWSIM_POINT_DELETE, DNPDEFS_OBJ_0_DEVICE_ATTRIBUTES,
      ((TMWTYPES_USHORT)(((TMWTYPES_USHORT)point<<8)|((TMWTYPES_UCHAR)variation&0xff))));

    return(tmwsim_tableDelete(&pPoint->data.list.listHead, variation));
  }
  return(TMWDEFS_FALSE);
}

TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpsim_deviceAttrQuantity(
  void *pHandle)
{
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;
  return((TMWTYPES_USHORT)tmwsim_tableSize(&pDbHandle->deviceAttrs));
}

TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_deviceAttrNext(
  void *pPoint,
  TMWTYPES_UCHAR variation,
  TMWTYPES_UCHAR *pNextVariation,
  TMWTYPES_UCHAR *pProperty)
{
  TMWSIM_POINT *pAttribute;
  TMWSIM_POINT *pDataPoint = (TMWSIM_POINT *)pPoint;
  if(variation == 0)
    pAttribute = tmwsim_tableGetFirstPoint(&pDataPoint->data.list.listHead);
  else
    pAttribute = tmwsim_tableFindPoint(&pDataPoint->data.list.listHead, variation);

  if(pAttribute != TMWDEFS_NULL)
  {  
    if(variation == 0)
    {
      *pNextVariation = (TMWTYPES_UCHAR)pAttribute->pointNumber; 
      *pProperty = pAttribute->data.attribute.property;
      return(TMWDEFS_TRUE);
    }
    pAttribute = tmwsim_tableGetNextPoint(&pDataPoint->data.list.listHead, pAttribute);
    if(pAttribute != TMWDEFS_NULL)
    {
      *pNextVariation = (TMWTYPES_UCHAR)pAttribute->pointNumber; 
      *pProperty = pAttribute->data.attribute.property;
      return(TMWDEFS_TRUE);
    }
  }
  return(TMWDEFS_FALSE);
}

static void _createDeviceAttributes(SDNPSIM_DATABASE *pDbHandle)
{
  TMWSIM_POINT * pPoint;
  DNPDATA_ATTRIBUTE_VALUE data;
  
  data.type = DNPDEFS_ATTRIBUTE_TYPE_VSTR;
  data.value.pStrValue = (TMWTYPES_UCHAR *)"default configuration";
  data.length = (TMWTYPES_UCHAR)strlen((char *)data.value.pStrValue);
  pPoint = (TMWSIM_POINT *)sdnpsim_addDeviceAttribute(pDbHandle, 0, DNPDEFS_OBJ0_CONFIG_ID, 0, &data); 
  if(pPoint != TMWDEFS_NULL)
    tmwsim_setDescription(pPoint, "Configuration ID"); 

  data.type = DNPDEFS_ATTRIBUTE_TYPE_VSTR;
  data.value.pStrValue = (TMWTYPES_UCHAR *)"v4r1b508";
  data.length = (TMWTYPES_UCHAR)strlen((char *)data.value.pStrValue);
  pPoint = (TMWSIM_POINT *)sdnpsim_addDeviceAttribute(pDbHandle, 0, DNPDEFS_OBJ0_CONFIG_VERSION, 0, &data);
  if (pPoint != TMWDEFS_NULL)
    tmwsim_setDescription(pPoint, "Configuration version");  
    
  /* 198 and 199 use the device attribute type DNP3TIME. 
   * This was added in TB2013-004, and some masters may not parse this type.
   */
  data.type = DNPDEFS_ATTRIBUTE_TYPE_DNP3TIME;
  data.length = 6;
  /* 14th Jan 2009, 14:56:23.491 */
  data.value.timeValue.year = 2009;
  data.value.timeValue.month = 1;
  data.value.timeValue.dayOfMonth = 14;
  data.value.timeValue.hour = 14;
  data.value.timeValue.minutes = 56;
  data.value.timeValue.mSecsAndSecs = 23491;
  data.value.timeValue.invalid = TMWDEFS_FALSE;
  pPoint = (TMWSIM_POINT *)sdnpsim_addDeviceAttribute(pDbHandle, 0, DNPDEFS_OBJ0_CONFIG_BLD_DATE, 0, &data);
  if (pPoint != TMWDEFS_NULL)
    tmwsim_setDescription(pPoint, "Configuration build date"); 
  
  data.type = DNPDEFS_ATTRIBUTE_TYPE_DNP3TIME;
  data.length = 6;
  /* 14th Jan 2009, 15:33:12.843 */
  data.value.timeValue.year = 2009;
  data.value.timeValue.month = 1;
  data.value.timeValue.dayOfMonth = 14;
  data.value.timeValue.hour = 15;
  data.value.timeValue.minutes = 33;
  data.value.timeValue.mSecsAndSecs = 12843;
  data.value.timeValue.invalid = TMWDEFS_FALSE;
  pPoint = (TMWSIM_POINT *)sdnpsim_addDeviceAttribute(pDbHandle, 0, DNPDEFS_OBJ0_CONFIG_CHG_DATE, 0, &data);
  if (pPoint != TMWDEFS_NULL)
    tmwsim_setDescription(pPoint, "Configuration last change date"); 

  data.type = DNPDEFS_ATTRIBUTE_TYPE_OSTR;
  data.value.pStrValue = (TMWTYPES_UCHAR *)"2C70";
  data.length = (TMWTYPES_UCHAR)strlen((char *)data.value.pStrValue);
  pPoint = (TMWSIM_POINT *)sdnpsim_addDeviceAttribute(pDbHandle, 0, DNPDEFS_OBJ0_CONFIG_DIGEST, 0, &data);
  if (pPoint != TMWDEFS_NULL)
    tmwsim_setDescription(pPoint, "Configuration digest"); 

  data.type = DNPDEFS_ATTRIBUTE_TYPE_VSTR;
  data.value.pStrValue = (TMWTYPES_UCHAR *)"CCITT CRC16";
  data.length = (TMWTYPES_UCHAR)strlen((char *)data.value.pStrValue);
  pPoint = (TMWSIM_POINT *)sdnpsim_addDeviceAttribute(pDbHandle, 0, DNPDEFS_OBJ0_CONFIG_DIG_ALG, 0, &data);
  if (pPoint != TMWDEFS_NULL)
    tmwsim_setDescription(pPoint, "Configuration digest algorithm"); 

  data.type = DNPDEFS_ATTRIBUTE_TYPE_VSTR;
  data.value.pStrValue = (TMWTYPES_UCHAR *)"ZSUB_3_12AF_0123456789ABCDEF0123456789ABCDEF";
  data.length = (TMWTYPES_UCHAR)strlen((char *)data.value.pStrValue);
  pPoint = (TMWSIM_POINT *)sdnpsim_addDeviceAttribute(pDbHandle, 0, DNPDEFS_OBJ0_MASTER_RID, 0, &data);
  if (pPoint != TMWDEFS_NULL)
    tmwsim_setDescription(pPoint, "Master Resource ID"); 

  data.type = DNPDEFS_ATTRIBUTE_TYPE_FLT;
  data.length = 4;
  data.value.fltValue.sfltValue = (TMWTYPES_SFLOAT)6366191.0; 
  pPoint = (TMWSIM_POINT *)sdnpsim_addDeviceAttribute(pDbHandle, 0, DNPDEFS_OBJ0_DEVLOC_ALTITUDE, 0, &data);
  if (pPoint != TMWDEFS_NULL)
    tmwsim_setDescription(pPoint, "Device location altitude"); 
  
  data.type = DNPDEFS_ATTRIBUTE_TYPE_FLT;
  data.length = 4;
  data.value.fltValue.sfltValue = (TMWTYPES_SFLOAT)3.79621; 
  pPoint = (TMWSIM_POINT *)sdnpsim_addDeviceAttribute(pDbHandle, 0, DNPDEFS_OBJ0_DEVLOC_LONGITUDE, 0, &data);
  if (pPoint != TMWDEFS_NULL)
    tmwsim_setDescription(pPoint, "Device location longitude"); 
  
  data.type = DNPDEFS_ATTRIBUTE_TYPE_FLT;
  data.length = 4;
  data.value.fltValue.sfltValue = (TMWTYPES_SFLOAT)-23.79621; 
  pPoint = (TMWSIM_POINT *)sdnpsim_addDeviceAttribute(pDbHandle, 0, DNPDEFS_OBJ0_DEVLOC_LATITUDE, 0, &data);
  if (pPoint != TMWDEFS_NULL)
    tmwsim_setDescription(pPoint, "Device location latitude"); 

  data.type = DNPDEFS_ATTRIBUTE_TYPE_VSTR;
  data.value.pStrValue = (TMWTYPES_UCHAR *)"PKMO";
  data.length = (TMWTYPES_UCHAR)strlen((char *)data.value.pStrValue);
  pPoint = (TMWSIM_POINT *)sdnpsim_addDeviceAttribute(pDbHandle, 0, DNPDEFS_OBJ0_SECOND_OPER_NAME, 0, &data);
  if (pPoint != TMWDEFS_NULL)
    tmwsim_setDescription(pPoint, "User-assigned secondary operator name"); 

  data.type = DNPDEFS_ATTRIBUTE_TYPE_VSTR;
  data.value.pStrValue = (TMWTYPES_UCHAR *)"PKMO";
  data.length = (TMWTYPES_UCHAR)strlen((char *)data.value.pStrValue);
  pPoint = (TMWSIM_POINT *)sdnpsim_addDeviceAttribute(pDbHandle, 0, DNPDEFS_OBJ0_PRIME_OPER_NAME, 0, &data);
  if (pPoint != TMWDEFS_NULL)
    tmwsim_setDescription(pPoint, "User-assigned primary operator name"); 

  data.type = DNPDEFS_ATTRIBUTE_TYPE_VSTR;
  data.value.pStrValue = (TMWTYPES_UCHAR *)"State Energy Master 1";
  data.length = (TMWTYPES_UCHAR)strlen((char *)data.value.pStrValue);
  pPoint = (TMWSIM_POINT *)sdnpsim_addDeviceAttribute(pDbHandle, 0, DNPDEFS_OBJ0_SYSTEM_NAME, 0, &data);
  if (pPoint != TMWDEFS_NULL)
    tmwsim_setDescription(pPoint, "User-assigned system name"); 

  data.type = DNPDEFS_ATTRIBUTE_TYPE_UINT;
  data.length = 1;
  data.value.uintValue = 5;
  pPoint = (TMWSIM_POINT *)sdnpsim_addDeviceAttribute(pDbHandle, 0, DNPDEFS_OBJ0_SEC_AUTH_VERSION, 0, &data);
  if (pPoint != TMWDEFS_NULL)
    tmwsim_setDescription(pPoint, "Secure authentication Version"); 

  data.type = DNPDEFS_ATTRIBUTE_TYPE_UINT;
  data.length = 1;
  data.value.uintValue = DNPAUTH_NUMBER_STATISTICS;
  pPoint = (TMWSIM_POINT *)sdnpsim_addDeviceAttribute(pDbHandle, 0, DNPDEFS_OBJ0_SEC_STATS_QUANT, 0, &data);
  if (pPoint != TMWDEFS_NULL)
    tmwsim_setDescription(pPoint, "Number of security statistics per association"); 

  data.type = DNPDEFS_ATTRIBUTE_TYPE_VSTR;
  data.length = 0;
  data.value.pStrValue = TMWDEFS_NULL;
  pPoint = (TMWSIM_POINT *)sdnpsim_addDeviceAttribute(pDbHandle, 0, DNPDEFS_OBJ0_USER_SPEC_SETS, 0, &data);
  if (pPoint != TMWDEFS_NULL)
    tmwsim_setDescription(pPoint, "ID of support for user-specific attributes"); 

#if SDNPDATA_SUPPORT_DATASETS
  /* create the device attributes for data sets */
  _createDatasetDeviceAttributes(pDbHandle);
#endif

  data.type = DNPDEFS_ATTRIBUTE_TYPE_UINT;
  data.length = 1;
  data.value.uintValue = SDNPCNFG_MAX_CONTROL_REQUESTS;
  pPoint = (TMWSIM_POINT *)sdnpsim_addDeviceAttribute(pDbHandle, 0, DNPDEFS_OBJ0_MAX_BIN_OUT_PER, 0, &data);
  if (pPoint != TMWDEFS_NULL)
    tmwsim_setDescription(pPoint, "Max number of binary outputs from object group 12 per request"); 

  data.type = DNPDEFS_ATTRIBUTE_TYPE_UINT;
  data.length = 4;
  data.value.uintValue = 100;
  pPoint = (TMWSIM_POINT *)sdnpsim_addDeviceAttribute(pDbHandle, 0, DNPDEFS_OBJ0_LOCAL_TIME_ACCUR, 0, &data);
  if (pPoint != TMWDEFS_NULL)
    tmwsim_setDescription(pPoint, "Local timing accuracy");

  data.type = DNPDEFS_ATTRIBUTE_TYPE_UINT;
  data.length = 4;
  data.value.uintValue = 100;
  pPoint = (TMWSIM_POINT *)sdnpsim_addDeviceAttribute(pDbHandle, 0, DNPDEFS_OBJ0_TIME_ACCURACY_DUR, 0, &data);
  if (pPoint != TMWDEFS_NULL)
    tmwsim_setDescription(pPoint, "Duration of time accuracy (following a time synchronization)");

  data.type = DNPDEFS_ATTRIBUTE_TYPE_INT;
  data.length = 1;
  data.value.intValue = 1;
  pPoint = (TMWSIM_POINT *)sdnpsim_addDeviceAttribute(pDbHandle, 0, DNPDEFS_OBJ0_ANLG_OUT_EV_SUPP, 0, &data);
  if (pPoint != TMWDEFS_NULL)
    tmwsim_setDescription(pPoint, "Analog output events supported");

  data.type = DNPDEFS_ATTRIBUTE_TYPE_UINT;
  data.length = 4;
  data.value.intValue = 0;
  pPoint = (TMWSIM_POINT *)sdnpsim_addDeviceAttribute(pDbHandle, 0, DNPDEFS_OBJ0_MAX_ANLG_OUT_INDEX, 0, &data);
  if (pPoint != TMWDEFS_NULL)
    tmwsim_setDescription(pPoint, "Maximum analog output index");

  data.type = DNPDEFS_ATTRIBUTE_TYPE_UINT;
  data.length = 4;
  data.value.intValue = 0;
  pPoint = (TMWSIM_POINT *)sdnpsim_addDeviceAttribute(pDbHandle, 0, DNPDEFS_OBJ0_NUM_ANLG_OUT_PNTS, 0, &data);
  if (pPoint != TMWDEFS_NULL)
    tmwsim_setDescription(pPoint, "Number of analog output points");

  data.type = DNPDEFS_ATTRIBUTE_TYPE_INT;
  data.length = 1;
  data.value.intValue = 1;
  pPoint = (TMWSIM_POINT *)sdnpsim_addDeviceAttribute(pDbHandle, 0, DNPDEFS_OBJ0_BIN_OUT_EVENT_SUPP, 0, &data);
  if (pPoint != TMWDEFS_NULL)
    tmwsim_setDescription(pPoint, "Binary output events supported.");

  data.type = DNPDEFS_ATTRIBUTE_TYPE_UINT;
  data.length = 4;
  data.value.intValue = 0;
  pPoint = (TMWSIM_POINT *)sdnpsim_addDeviceAttribute(pDbHandle, 0, DNPDEFS_OBJ0_MAX_BIN_OUT_INDEX, 0, &data);
  if (pPoint != TMWDEFS_NULL)
    tmwsim_setDescription(pPoint, "Maximum binary output index");

  data.type = DNPDEFS_ATTRIBUTE_TYPE_UINT;
  data.length = 4;
  data.value.intValue = 0;
  pPoint = (TMWSIM_POINT *)sdnpsim_addDeviceAttribute(pDbHandle, 0, DNPDEFS_OBJ0_NUM_BIN_OUT_PNTS, 0, &data);
  if (pPoint != TMWDEFS_NULL)
    tmwsim_setDescription(pPoint, "Number of binary output points");

  data.type = DNPDEFS_ATTRIBUTE_TYPE_INT;
  data.length = 1;
  data.value.intValue = 1;
  pPoint = (TMWSIM_POINT *)sdnpsim_addDeviceAttribute(pDbHandle, 0, DNPDEFS_OBJ0_FRZN_CNTR_EV_SUPP, 0, &data);
  if (pPoint != TMWDEFS_NULL)
    tmwsim_setDescription(pPoint, "Frozen counter events supported");

  data.type = DNPDEFS_ATTRIBUTE_TYPE_INT;
  data.length = 1;
  data.value.intValue = 1;
  pPoint = (TMWSIM_POINT *)sdnpsim_addDeviceAttribute(pDbHandle, 0, DNPDEFS_OBJ0_FRZN_CNTR_SUPP, 0, &data);
  if (pPoint != TMWDEFS_NULL)
    tmwsim_setDescription(pPoint, "Frozen counts supported");

  data.type = DNPDEFS_ATTRIBUTE_TYPE_INT;
  data.length = 1;
  data.value.intValue = 1;
  pPoint = (TMWSIM_POINT *)sdnpsim_addDeviceAttribute(pDbHandle, 0, DNPDEFS_OBJ0_CNTR_EVENT_SUPP, 0, &data);
  if (pPoint != TMWDEFS_NULL)
    tmwsim_setDescription(pPoint, "Counter events supported");

  data.type = DNPDEFS_ATTRIBUTE_TYPE_UINT;
  data.length = 4;
  data.value.intValue = 0;
  pPoint = (TMWSIM_POINT *)sdnpsim_addDeviceAttribute(pDbHandle, 0, DNPDEFS_OBJ0_MAX_CNTR_INDEX, 0, &data);
  if (pPoint != TMWDEFS_NULL)
    tmwsim_setDescription(pPoint, "Maximum counter index");

  data.type = DNPDEFS_ATTRIBUTE_TYPE_UINT;
  data.length = 4;
  data.value.intValue = 0;
  pPoint = (TMWSIM_POINT *)sdnpsim_addDeviceAttribute(pDbHandle, 0, DNPDEFS_OBJ0_NUM_CNTR_PNTS, 0, &data);
  if (pPoint != TMWDEFS_NULL)
    tmwsim_setDescription(pPoint, "Number of counter points");

  data.type = DNPDEFS_ATTRIBUTE_TYPE_INT;
  data.length = 1;
  data.value.intValue = 0;
  pPoint = (TMWSIM_POINT *)sdnpsim_addDeviceAttribute(pDbHandle, 0, DNPDEFS_OBJ0_FRZN_ANLG_IN_SUPP, 0, &data);
  if (pPoint != TMWDEFS_NULL)
    tmwsim_setDescription(pPoint, "Frozen analog inputs supported");

  data.type = DNPDEFS_ATTRIBUTE_TYPE_INT;
  data.length = 1;
  data.value.intValue = 1;
  pPoint = (TMWSIM_POINT *)sdnpsim_addDeviceAttribute(pDbHandle, 0, DNPDEFS_OBJ0_ANLG_IN_EVENT_SUPP, 0, &data);
  if (pPoint != TMWDEFS_NULL)
    tmwsim_setDescription(pPoint, "Analog input events supported");

  data.type = DNPDEFS_ATTRIBUTE_TYPE_UINT;
  data.length = 4;
  data.value.intValue = 0;
  pPoint = (TMWSIM_POINT *)sdnpsim_addDeviceAttribute(pDbHandle, 0, DNPDEFS_OBJ0_MAX_ANLG_IN_INDEX, 0, &data);
  if (pPoint != TMWDEFS_NULL)
    tmwsim_setDescription(pPoint, "Maximum analog input index");

  data.type = DNPDEFS_ATTRIBUTE_TYPE_UINT;
  data.length = 4;
  data.value.intValue = 0;
  pPoint = (TMWSIM_POINT *)sdnpsim_addDeviceAttribute(pDbHandle, 0, DNPDEFS_OBJ0_NUM_ANLG_IN_PNTS, 0, &data);
  if (pPoint != TMWDEFS_NULL)
    tmwsim_setDescription(pPoint, "Number of analog input points");

  data.type = DNPDEFS_ATTRIBUTE_TYPE_INT;
  data.length = 1;
  data.value.intValue = 1;
  pPoint = (TMWSIM_POINT *)sdnpsim_addDeviceAttribute(pDbHandle, 0, DNPDEFS_OBJ0_DBL_IN_EVENTS_SUPP, 0, &data);
  if (pPoint != TMWDEFS_NULL)
    tmwsim_setDescription(pPoint, "Double-bit binary input events supported");

  data.type = DNPDEFS_ATTRIBUTE_TYPE_UINT;
  data.length = 4;
  data.value.intValue = 0;
  pPoint = (TMWSIM_POINT *)sdnpsim_addDeviceAttribute(pDbHandle, 0, DNPDEFS_OBJ0_MAX_DBL_IN_INDEX, 0, &data);
  if (pPoint != TMWDEFS_NULL)
    tmwsim_setDescription(pPoint, "Maximum double-bit binary input index");

  data.type = DNPDEFS_ATTRIBUTE_TYPE_UINT;
  data.length = 4;
  data.value.intValue = 0;
  pPoint = (TMWSIM_POINT *)sdnpsim_addDeviceAttribute(pDbHandle, 0, DNPDEFS_OBJ0_NUM_DBL_IN_PNTS, 0, &data);
  if (pPoint != TMWDEFS_NULL)
    tmwsim_setDescription(pPoint, "Number of double-bit binary input points");

  data.type = DNPDEFS_ATTRIBUTE_TYPE_INT;
  data.length = 1;
  data.value.intValue = 1;
  pPoint = (TMWSIM_POINT *)sdnpsim_addDeviceAttribute(pDbHandle, 0, DNPDEFS_OBJ0_BIN_IN_EVENTS_SUPP, 0, &data);
  if (pPoint != TMWDEFS_NULL)
    tmwsim_setDescription(pPoint, "Binary input events supported");

  data.type = DNPDEFS_ATTRIBUTE_TYPE_UINT;
  data.length = 4;
  data.value.intValue = 0;
  pPoint = (TMWSIM_POINT *)sdnpsim_addDeviceAttribute(pDbHandle, 0, DNPDEFS_OBJ0_MAX_BIN_IN_INDEX, 0, &data);
  if (pPoint != TMWDEFS_NULL)
    tmwsim_setDescription(pPoint, "Maximum binary input index");

  data.type = DNPDEFS_ATTRIBUTE_TYPE_UINT;
  data.length = 4;
  data.value.intValue = 0;
  pPoint = (TMWSIM_POINT *)sdnpsim_addDeviceAttribute(pDbHandle, 0, DNPDEFS_OBJ0_NUM_BIN_IN_PNTS, 0, &data);
  if (pPoint != TMWDEFS_NULL)
    tmwsim_setDescription(pPoint, "Number of binary input points");

  data.type = DNPDEFS_ATTRIBUTE_TYPE_UINT;
  data.length = 4;
  data.value.intValue = 2048;
  pPoint = (TMWSIM_POINT *)sdnpsim_addDeviceAttribute(pDbHandle, 0, DNPDEFS_OBJ0_MAX_TX_FRAG_SIZE, 1, &data);
  if (pPoint != TMWDEFS_NULL)
    tmwsim_setDescription(pPoint, "Maximum transmit fragment size");

  data.type = DNPDEFS_ATTRIBUTE_TYPE_UINT;
  data.length = 4;
  data.value.intValue = 2048;
  pPoint = (TMWSIM_POINT *)sdnpsim_addDeviceAttribute(pDbHandle, 0, DNPDEFS_OBJ0_MAX_RX_FRAG_SIZE, 0, &data);
  if (pPoint != TMWDEFS_NULL)
    tmwsim_setDescription(pPoint, "Maximum receive fragment size");

  data.type = DNPDEFS_ATTRIBUTE_TYPE_VSTR;
  data.value.pStrValue = (TMWTYPES_UCHAR *)tmwvrsn_TMWVERSION_STRING;
  data.length = (TMWTYPES_UCHAR)strlen((char *)data.value.pStrValue);
  pPoint = (TMWSIM_POINT *)sdnpsim_addDeviceAttribute(pDbHandle, 0, DNPDEFS_OBJ0_SFTW_VERSION_STR, 0, &data);
  if (pPoint != TMWDEFS_NULL)
    tmwsim_setDescription(pPoint, "Device manufacturer�s software version string");

  data.type = DNPDEFS_ATTRIBUTE_TYPE_VSTR;
  data.value.pStrValue = (TMWTYPES_UCHAR *)"hardware version";
  data.length = (TMWTYPES_UCHAR)strlen((char *)data.value.pStrValue);
  pPoint = (TMWSIM_POINT *)sdnpsim_addDeviceAttribute(pDbHandle, 0, DNPDEFS_OBJ0_HDWR_VERSION_STR, 0, &data);
  if (pPoint != TMWDEFS_NULL)
    tmwsim_setDescription(pPoint, "Device manufacturer�s hardware version string");

  /* previously called reserved */
  data.type = DNPDEFS_ATTRIBUTE_TYPE_VSTR;
  data.value.pStrValue = (TMWTYPES_UCHAR *)"TH Solar";
  data.length = (TMWTYPES_UCHAR)strlen((char *)data.value.pStrValue);
  pPoint = (TMWSIM_POINT *)sdnpsim_addDeviceAttribute(pDbHandle, 0, DNPDEFS_OBJ0_OWNER_NAME, 0, &data);
  if (pPoint != TMWDEFS_NULL)
    tmwsim_setDescription(pPoint, "User-assigned owner name");

  data.type = DNPDEFS_ATTRIBUTE_TYPE_VSTR;
  data.value.pStrValue = (TMWTYPES_UCHAR *)"Raleigh NC";
  data.length = (TMWTYPES_UCHAR)strlen((char *)data.value.pStrValue);
  pPoint = (TMWSIM_POINT *)sdnpsim_addDeviceAttribute(pDbHandle, 0, DNPDEFS_OBJ0_USER_LOCATION, 1, &data);
  if (pPoint != TMWDEFS_NULL)
    tmwsim_setDescription(pPoint, "User-assigned location name or code string");

  data.type = DNPDEFS_ATTRIBUTE_TYPE_VSTR;
  data.value.pStrValue = (TMWTYPES_UCHAR *)"2.00.xx";
  data.length = (TMWTYPES_UCHAR)strlen((char *)data.value.pStrValue);
  pPoint = (TMWSIM_POINT *)sdnpsim_addDeviceAttribute(pDbHandle, 0, DNPDEFS_OBJ0_USER_ID_CODE, 1, &data);
  if (pPoint != TMWDEFS_NULL)
    tmwsim_setDescription(pPoint, "User-assigned ID code/number string");

  data.type = DNPDEFS_ATTRIBUTE_TYPE_VSTR;
  data.value.pStrValue = (TMWTYPES_UCHAR *)"test harness";
  data.length = (TMWTYPES_UCHAR)strlen((char *)data.value.pStrValue);
  pPoint = (TMWSIM_POINT *)sdnpsim_addDeviceAttribute(pDbHandle, 0, DNPDEFS_OBJ0_USER_NAME, 1, &data);
  if (pPoint != TMWDEFS_NULL)
    tmwsim_setDescription(pPoint, "User-assigned name string for the outstation");

  data.type = DNPDEFS_ATTRIBUTE_TYPE_VSTR;
  data.value.pStrValue = (TMWTYPES_UCHAR *)"200-333-333-222";
  data.length = (TMWTYPES_UCHAR)strlen((char *)data.value.pStrValue);
  pPoint = (TMWSIM_POINT *)sdnpsim_addDeviceAttribute(pDbHandle, 0, DNPDEFS_OBJ0_SERIAL_NUMBER, 0, &data);
  if (pPoint != TMWDEFS_NULL)
    tmwsim_setDescription(pPoint, "Device serial number string");

  data.type = DNPDEFS_ATTRIBUTE_TYPE_VSTR;
  data.value.pStrValue = (TMWTYPES_UCHAR *)"3:2004";
  data.length = (TMWTYPES_UCHAR)strlen((char *)data.value.pStrValue);
  pPoint = (TMWSIM_POINT *)sdnpsim_addDeviceAttribute(pDbHandle, 0, DNPDEFS_OBJ0_SUBSET_LEVEL, 0, &data);
  if (pPoint != TMWDEFS_NULL)
    tmwsim_setDescription(pPoint, "DNP subset level and conformance");

  data.type = DNPDEFS_ATTRIBUTE_TYPE_VSTR;
  data.value.pStrValue = (TMWTYPES_UCHAR *)"Protocol TestHarness";
  data.length = (TMWTYPES_UCHAR)strlen((char *)data.value.pStrValue);
  pPoint = (TMWSIM_POINT *)sdnpsim_addDeviceAttribute(pDbHandle, 0, DNPDEFS_OBJ0_PRODUCT_NAME, 0, &data);
  if (pPoint != TMWDEFS_NULL)
    tmwsim_setDescription(pPoint, "Device manufacturer�s product name and model");

  /* DNPDEFS_OBJ0_DEVICE_CODE   Not defined in Tech Bulletin */

  data.type = DNPDEFS_ATTRIBUTE_TYPE_VSTR;
  data.value.pStrValue = (TMWTYPES_UCHAR *)"Triangle Microworks";
  data.length = (TMWTYPES_UCHAR)strlen((char *)data.value.pStrValue);
  pPoint = (TMWSIM_POINT *)sdnpsim_addDeviceAttribute(pDbHandle, 0, DNPDEFS_OBJ0_DEVICE_MANU_NAME, 0, &data);
  if (pPoint != TMWDEFS_NULL)
    tmwsim_setDescription(pPoint, "Reserved for device manufacturer�s registered DNP device code");
}

void * TMWDEFS_GLOBAL sdnpsim_deviceAttrGetPoint( 
  void *pHandle,
  TMWTYPES_USHORT point)
{
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle; 
  return(tmwsim_tableFindPoint(&pDbHandle->deviceAttrs, point));
}

void * TMWDEFS_GLOBAL sdnpsim_deviceAttrGetVar( 
  void *pPoint,
  TMWTYPES_UCHAR variation)
{
  TMWSIM_POINT *pDataPoint = (TMWSIM_POINT *)pPoint;
  if(variation == 255)
  {
    /* return any device attribute variation for this point */
    return(tmwsim_tableGetFirstPoint(&pDataPoint->data.list.listHead));
  }
  return(tmwsim_tableFindPoint(&pDataPoint->data.list.listHead, variation));
}

TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_deviceAttrRead(
  void *pAttribute,
  DNPDATA_ATTRIBUTE_VALUE *pData)
{  
  TMWSIM_POINT *pDataPoint = (TMWSIM_POINT *)pAttribute;
  DNPDEFS_ATTRIBUTE_DATA_TYPE type = (DNPDEFS_ATTRIBUTE_DATA_TYPE)pDataPoint->data.attribute.type;

  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pDataPoint->pDbHandle; 
 
  /* index 0 indicates DNP User Group standard attributes
   * the ones in the switch are dynamic and must be calculated 
   */
  if(pDataPoint->data.attribute.point == 0)
  {
    switch(pDataPoint->pointNumber)
    {
#if SDNPDATA_SUPPORT_OBJ120
      case DNPDEFS_OBJ0_SEC_AUTH_VERSION: 
        if(pDbHandle->pSDNPSession->authenticationEnabled)
        {
          if(pDbHandle->pSDNPSession->dnp.operateInV2Mode == TMWDEFS_FALSE)
            pData->value.uintValue = 5; 
          else
            pData->value.uintValue = 2;
        }
        else
        {
          pData->value.uintValue = 0;
        }
        pData->type = DNPDEFS_ATTRIBUTE_TYPE_UINT;
        pData->length = 1;
        return(TMWDEFS_TRUE);
        break;
      case DNPDEFS_OBJ0_SEC_STATS_QUANT:  
        if(pDbHandle->pSDNPSession->authenticationEnabled)
        {
          if(pDbHandle->pSDNPSession->dnp.operateInV2Mode == TMWDEFS_FALSE)
            pData->value.uintValue = DNPAUTH_NUMBER_STATISTICS; 
          else
            pData->value.uintValue = 0;
        }
        else
        {
          pData->value.uintValue = 0;
        }
        pData->type = DNPDEFS_ATTRIBUTE_TYPE_UINT;
        pData->length = 1;
        return(TMWDEFS_TRUE);
        break;
#endif
      case DNPDEFS_OBJ0_USER_SPEC_SETS:
        /* This should contain a vstr with namespace/index pairs */
        /* Currently this can be set by user */
        break; 

#if SDNPDATA_SUPPORT_OBJ85
      case DNPDEFS_OBJ0_NUM_MASTER_PROTOS: 
        pData->value.uintValue = sdnpsim_datasetProtoMasterQty(pDbHandle);
        pData->type = DNPDEFS_ATTRIBUTE_TYPE_UINT;
        pData->length = 4;
        return(TMWDEFS_TRUE);
        break;
      case DNPDEFS_OBJ0_NUM_OUTSTA_PROTOS:
        pData->value.uintValue = tmwdlist_size(&pDbHandle->datasetProtos);
        pData->value.uintValue -= sdnpsim_datasetProtoMasterQty(pDbHandle);
        pData->type = DNPDEFS_ATTRIBUTE_TYPE_UINT;
        pData->length = 4;
        return(TMWDEFS_TRUE);
        break;
#endif
      case DNPDEFS_OBJ0_ANLG_OUT_EV_SUPP:
        pData->value.intValue = 1;
        pData->type = DNPDEFS_ATTRIBUTE_TYPE_INT;
        pData->length = 1;
        return(TMWDEFS_TRUE);
        break;
#if SDNPDATA_SUPPORT_OBJ87
      case DNPDEFS_OBJ0_NUM_MASTER_DATASET:
        pData->value.uintValue = sdnpsim_datasetMasterQty(pDbHandle);
        pData->type = DNPDEFS_ATTRIBUTE_TYPE_UINT;
        pData->length = 4;
        return(TMWDEFS_TRUE);
        break;
      case DNPDEFS_OBJ0_NUM_OUTSTA_DATASET:
        pData->value.uintValue = tmwdlist_size(&pDbHandle->datasetDescrDatas);
        pData->value.uintValue -= sdnpsim_datasetMasterQty(pDbHandle);
        pData->type = DNPDEFS_ATTRIBUTE_TYPE_UINT;
        pData->length = 4;
        return(TMWDEFS_TRUE);
        break;
#endif
      case DNPDEFS_OBJ0_MAX_ANLG_OUT_INDEX: 
        pData->value.uintValue = tmwsim_tableSize(&pDbHandle->analogOutputs);
        if(pData->value.uintValue >0)
          pData->value.uintValue--;
        pData->type = DNPDEFS_ATTRIBUTE_TYPE_UINT;
        pData->length = 4;
        return(TMWDEFS_TRUE);
        break;
      case DNPDEFS_OBJ0_NUM_ANLG_OUT_PNTS:
        pData->value.uintValue = tmwsim_tableSize(&pDbHandle->analogOutputs);
        pData->type = DNPDEFS_ATTRIBUTE_TYPE_UINT;
        pData->length = 4;
        return(TMWDEFS_TRUE);
        break;
      case DNPDEFS_OBJ0_BIN_OUT_EVENT_SUPP:
        pData->value.intValue = 1;
        pData->type = DNPDEFS_ATTRIBUTE_TYPE_INT;
        pData->length = 1;
        return(TMWDEFS_TRUE);
        break;
      case DNPDEFS_OBJ0_MAX_BIN_OUT_INDEX: 
        pData->value.uintValue = tmwsim_tableSize(&pDbHandle->binaryOutputs);
        if(pData->value.uintValue >0)
          pData->value.uintValue--;
        pData->type = DNPDEFS_ATTRIBUTE_TYPE_UINT;
        pData->length = 4;
        return(TMWDEFS_TRUE);
      case DNPDEFS_OBJ0_NUM_BIN_OUT_PNTS:
        pData->value.uintValue = tmwsim_tableSize(&pDbHandle->binaryOutputs);
        pData->type = DNPDEFS_ATTRIBUTE_TYPE_UINT;
        pData->length = 4;
        return(TMWDEFS_TRUE);
      case DNPDEFS_OBJ0_FRZN_CNTR_EV_SUPP: 
      case DNPDEFS_OBJ0_FRZN_CNTR_SUPP:
      case DNPDEFS_OBJ0_CNTR_EVENT_SUPP:
        pData->value.intValue = 1;
        pData->type = DNPDEFS_ATTRIBUTE_TYPE_INT;
        pData->length = 1;
        return(TMWDEFS_TRUE);
        break;
      case DNPDEFS_OBJ0_MAX_CNTR_INDEX:
        pData->value.uintValue = tmwsim_tableSize(&pDbHandle->binaryCounters);
        if(pData->value.uintValue >0)
          pData->value.uintValue--;
        pData->type = DNPDEFS_ATTRIBUTE_TYPE_UINT;
        pData->length = 4;
        return(TMWDEFS_TRUE); 
      case DNPDEFS_OBJ0_NUM_CNTR_PNTS:
        pData->value.uintValue = tmwsim_tableSize(&pDbHandle->binaryCounters);
        pData->type = DNPDEFS_ATTRIBUTE_TYPE_UINT;
        pData->length = 4;
        return(TMWDEFS_TRUE); 
      case DNPDEFS_OBJ0_FRZN_ANLG_IN_SUPP:
        pData->value.intValue = 0;
        pData->type = DNPDEFS_ATTRIBUTE_TYPE_INT;
        pData->length = 1;
        return(TMWDEFS_TRUE);
        break;
      case DNPDEFS_OBJ0_ANLG_IN_EVENT_SUPP:
        pData->value.intValue = 1;
        pData->type = DNPDEFS_ATTRIBUTE_TYPE_INT;
        pData->length = 1;
        return(TMWDEFS_TRUE);
        break;
      case DNPDEFS_OBJ0_MAX_ANLG_IN_INDEX:
        pData->value.uintValue = tmwsim_tableSize(&pDbHandle->analogInputs);
        if(pData->value.uintValue >0)
          pData->value.uintValue--;
        pData->type = DNPDEFS_ATTRIBUTE_TYPE_UINT;
        pData->length = 4;
        return(TMWDEFS_TRUE); 
      case DNPDEFS_OBJ0_NUM_ANLG_IN_PNTS:
        pData->value.uintValue = tmwsim_tableSize(&pDbHandle->analogInputs);
        pData->type = DNPDEFS_ATTRIBUTE_TYPE_UINT;
        pData->length = 4;
        return(TMWDEFS_TRUE); 
      case DNPDEFS_OBJ0_DBL_IN_EVENTS_SUPP:
        pData->value.intValue = 1;
        pData->type = DNPDEFS_ATTRIBUTE_TYPE_INT;
        pData->length = 1;
        return(TMWDEFS_TRUE);
        break;
      case DNPDEFS_OBJ0_MAX_DBL_IN_INDEX:
        pData->value.uintValue = tmwsim_tableSize(&pDbHandle->doubleInputs);
        if(pData->value.uintValue >0)
          pData->value.uintValue--;
        pData->type = DNPDEFS_ATTRIBUTE_TYPE_UINT;
        pData->length = 4;
        return(TMWDEFS_TRUE); 
      case DNPDEFS_OBJ0_NUM_DBL_IN_PNTS:
        pData->value.uintValue = tmwsim_tableSize(&pDbHandle->doubleInputs);
        pData->type = DNPDEFS_ATTRIBUTE_TYPE_UINT;
        pData->length = 4;
        return(TMWDEFS_TRUE); 
      case DNPDEFS_OBJ0_BIN_IN_EVENTS_SUPP:
        pData->value.intValue = 1;
        pData->type = DNPDEFS_ATTRIBUTE_TYPE_INT;
        pData->length = 1;
        return(TMWDEFS_TRUE);
        break;
      case DNPDEFS_OBJ0_MAX_BIN_IN_INDEX:
        pData->value.uintValue = tmwsim_tableSize(&pDbHandle->binaryInputs);
        if(pData->value.uintValue >0)
          pData->value.uintValue--;
        pData->type = DNPDEFS_ATTRIBUTE_TYPE_UINT;
        pData->length = 4;
        return(TMWDEFS_TRUE); 
      case DNPDEFS_OBJ0_NUM_BIN_IN_PNTS:
        pData->value.uintValue = tmwsim_tableSize(&pDbHandle->binaryInputs);
        pData->type = DNPDEFS_ATTRIBUTE_TYPE_UINT;
        pData->length = 4;
        return(TMWDEFS_TRUE); 
    }
  } 
    
  if((type == DNPDEFS_ATTRIBUTE_TYPE_VSTR)
    || (type == DNPDEFS_ATTRIBUTE_TYPE_OSTR)
    || (type == DNPDEFS_ATTRIBUTE_TYPE_BSTR))
  {
    pData->value.pStrValue = pDataPoint->data.attribute.pBuf;
  }
  else
  { 
    switch(pDataPoint->data.attribute.type)
    {
    case DNPDEFS_ATTRIBUTE_TYPE_UINT:
      pData->value.uintValue = (TMWTYPES_ULONG)pDataPoint->data.attribute.value;
      break;
    case DNPDEFS_ATTRIBUTE_TYPE_INT:
      pData->value.intValue = (TMWTYPES_LONG)pDataPoint->data.attribute.value; 
      break;
    case DNPDEFS_ATTRIBUTE_TYPE_FLT:
      if(pDataPoint->data.attribute.length == 4)
        pData->value.fltValue.sfltValue = (TMWTYPES_SFLOAT)pDataPoint->data.attribute.value; 
      else
        pData->value.fltValue.doubleValue = (TMWTYPES_DOUBLE)pDataPoint->data.attribute.value; 
      break;
    case DNPDEFS_ATTRIBUTE_TYPE_DNP3TIME:
      pData->value.timeValue = pDataPoint->data.attribute.timeValue; 
      break;
    } 
  }

  pData->type = (DNPDEFS_ATTRIBUTE_DATA_TYPE)pDataPoint->data.attribute.type;
  pData->length = pDataPoint->data.attribute.length;
  return(TMWDEFS_TRUE);
}

TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_deviceAttrWrite(
  void *pAttribute, 
  DNPDATA_ATTRIBUTE_VALUE *pData)
{
  TMWSIM_POINT *pDataPoint = (TMWSIM_POINT*)pAttribute;
  
  /* These are determined by Secure Authentication Version and cannot be written */
  if((pDataPoint->pointNumber >= DNPDEFS_OBJ0_SEC_AUTH_VERSION)
    && (pDataPoint->pointNumber <= DNPDEFS_OBJ0_SEC_STATS_QUANT))
    return(TMWDEFS_FALSE);

  /* These are determined by simulated database and cannot be set */
  if((pDataPoint->pointNumber >= DNPDEFS_OBJ0_NUM_MASTER_PROTOS)
    && (pDataPoint->pointNumber <= DNPDEFS_OBJ0_NUM_OUTSTA_DATASET))
    return(TMWDEFS_FALSE);

  /* These are determined by simulated database and cannot be set */
  if((pDataPoint->pointNumber >= DNPDEFS_OBJ0_ANLG_OUT_EV_SUPP)
    && (pDataPoint->pointNumber <= DNPDEFS_OBJ0_NUM_BIN_IN_PNTS ))
    return(TMWDEFS_FALSE);

  /* See if this device attribute is writable */
  if(!(pDataPoint->data.attribute.property & 0x01))
    return(TMWDEFS_FALSE);

  /* For now there is only point number 0, but 255 variations 
   * use variation as the lookup.
   */
  if((pData->type == DNPDEFS_ATTRIBUTE_TYPE_VSTR)
    || (pData->type == DNPDEFS_ATTRIBUTE_TYPE_OSTR)
    || (pData->type == DNPDEFS_ATTRIBUTE_TYPE_BSTR))
  { 
    if(pDataPoint->data.attribute.pBuf == TMWDEFS_NULL)
      pDataPoint->data.attribute.pBuf = (TMWTYPES_UCHAR *)tmwmem_alloc(TMWMEM_SIM_STRING_TYPE);
  
    /* Truncate string if it is too long */
    if(pData->length > TMWSIM_STRING_MAX_LENGTH)
      pData->length = TMWSIM_STRING_MAX_LENGTH;

    memcpy(pDataPoint->data.attribute.pBuf, pData->value.pStrValue, pData->length);
  }
  else
  {  
    if(pDataPoint->data.attribute.pBuf != TMWDEFS_NULL) 
    {
      tmwmem_free(pDataPoint->data.attribute.pBuf);
      pDataPoint->data.attribute.pBuf = TMWDEFS_NULL;
    }
    switch(pData->type)
    {
    case DNPDEFS_ATTRIBUTE_TYPE_UINT:
      pDataPoint->data.attribute.value = pData->value.uintValue;
      break;
    case DNPDEFS_ATTRIBUTE_TYPE_INT:
      pDataPoint->data.attribute.value = pData->value.intValue; 
      break;
    case DNPDEFS_ATTRIBUTE_TYPE_FLT:
      if(pData->length == 4)
        pDataPoint->data.attribute.value = pData->value.fltValue.sfltValue;
      else
        pDataPoint->data.attribute.value = pData->value.fltValue.doubleValue;
      break;
    case DNPDEFS_ATTRIBUTE_TYPE_DNP3TIME:
      pDataPoint->data.attribute.timeValue = pData->value.timeValue;
      break;
    } 
  }
  pDataPoint->data.attribute.length = pData->length;
  pDataPoint->data.attribute.type = (TMWTYPES_UCHAR)pData->type; 
  sdnputil_getDateTime((TMWSESN*)pDataPoint->pSCLHandle, &pDataPoint->timeStamp);
  
  _callCallback((TMWSIM_POINT *)pDataPoint, TMWSIM_POINT_UPDATE, DNPDEFS_OBJ_0_DEVICE_ATTRIBUTES,  
    ((TMWTYPES_USHORT)(((TMWTYPES_USHORT) pDataPoint->data.attribute.point << 8)|(pDataPoint->pointNumber & 0xff))));

  return(TMWDEFS_TRUE);
}

/* Not called from SCL */
void TMWDEFS_GLOBAL sdnpsim_deviceAttrSetProperty(
  void *pAttribute,
  TMWTYPES_UCHAR property)
{
  TMWSIM_POINT *pDataPoint = (TMWSIM_POINT*)pAttribute;
  pDataPoint->data.attribute.property = property;
}

/* Not called from SCL */
TMWTYPES_UCHAR TMWDEFS_GLOBAL sdnpsim_deviceAttrGetProperty(
  void *pAttribute)
{
  TMWSIM_POINT *pDataPoint = (TMWSIM_POINT*)pAttribute;
  return(pDataPoint->data.attribute.property);
}
#endif

#if SDNPDATA_SUPPORT_DATASETS
static void _initDataSets(SDNPSIM_DATABASE *pDbHandle)
{
  SDNPSIM_DATASET_PROTO *pProto;
  SDNPSIM_DATASET_DESCR_DATA *pDescr;
  SDNPSIM_DATASET_DESCR_DATA *pDataSet;
  DNPDATA_DATASET_DESCR_ELEM elem;
  DNPDATA_DATASET_DESCR_INDEX indexElem;
  DNPDATA_DATASET_VALUE  value;
  TMWDTIME timeStamp; 
  TMWTYPES_UCHAR index;
  TMWTYPES_UCHAR indexElemIndex;

  tmwdlist_initialize(&pDbHandle->datasetProtos);
  tmwdlist_initialize(&pDbHandle->datasetDescrDatas); 

  /*********************/
  /* Prototypes        */
  /*********************/
  pProto =  (SDNPSIM_DATASET_PROTO *)sdnpsim_datasetProtoCreatePoint(pDbHandle, 0, 
    (TMWTYPES_UCHAR *)"\xc2\xf4\x10\x10\x65\xb3\x11\xd1\xa2\x9f\x00\xaa\x00\xc1\x5a\x82", TMWDEFS_FALSE);
  
  if(pProto == TMWDEFS_NULL)
    return;

  index = 0;

  elem.descrElemType   = DNPDEFS_DATASET_DESCR_NSPC; /* 3 */
  elem.dataTypeCode    = (DNPDEFS_DATASET_TYPE_CODE)0x00;
  elem.maxDataLength   = 0x00;
  elem.ancillaryValue.type = DNPDATA_VALUE_STRPTR;
  elem.ancillaryValue.value.pStrValue = (TMWTYPES_UCHAR *)"Earth Relaying Org";
  elem.ancillaryValue.length = (TMWTYPES_UCHAR)strlen((char *)elem.ancillaryValue.value.pStrValue);
  sdnpsim_datasetProtoWrite(pProto, index++, &elem);

  elem.descrElemType   = DNPDEFS_DATASET_DESCR_NAME; /* 4 */
  elem.dataTypeCode    = (DNPDEFS_DATASET_TYPE_CODE)0x00;
  elem.maxDataLength   = 0x00;
  elem.ancillaryValue.type = DNPDATA_VALUE_STRPTR;
  elem.ancillaryValue.value.pStrValue = (TMWTYPES_UCHAR *)"Feeder Fault";
  elem.ancillaryValue.length = (TMWTYPES_UCHAR)strlen((char *)elem.ancillaryValue.value.pStrValue);
  sdnpsim_datasetProtoWrite(pProto, index++, &elem);

  elem.descrElemType   = DNPDEFS_DATASET_DESCR_DAEL; /* 5*/
  elem.dataTypeCode    = (DNPDEFS_DATASET_TYPE_CODE)0x04;  /* FLT */
  elem.maxDataLength   = 0x08;
  elem.ancillaryValue.type = DNPDATA_VALUE_STRPTR;
  elem.ancillaryValue.value.pStrValue = (TMWTYPES_UCHAR *)"Current Phs A";
  elem.ancillaryValue.length = (TMWTYPES_UCHAR)strlen((char *)elem.ancillaryValue.value.pStrValue);
  sdnpsim_datasetProtoWrite(pProto, index++, &elem);

  elem.descrElemType   = DNPDEFS_DATASET_DESCR_DAEL; /* 5*/
  elem.dataTypeCode    = (DNPDEFS_DATASET_TYPE_CODE)0x04;  /* FLT */
  elem.maxDataLength   = 0x08;
  elem.ancillaryValue.type = DNPDATA_VALUE_STRPTR;
  elem.ancillaryValue.value.pStrValue = (TMWTYPES_UCHAR *)"Current Phs B";
  elem.ancillaryValue.length = (TMWTYPES_UCHAR)strlen((char *)elem.ancillaryValue.value.pStrValue);
  sdnpsim_datasetProtoWrite(pProto, index++, &elem);

  elem.descrElemType   = DNPDEFS_DATASET_DESCR_DAEL; /* 5*/
  elem.dataTypeCode    = (DNPDEFS_DATASET_TYPE_CODE)0x04;  /* FLT */
  elem.maxDataLength   = 0x08;
  elem.ancillaryValue.type = DNPDATA_VALUE_STRPTR;
  elem.ancillaryValue.value.pStrValue = (TMWTYPES_UCHAR *)"Current Phs C";
  elem.ancillaryValue.length = (TMWTYPES_UCHAR)strlen((char *)elem.ancillaryValue.value.pStrValue);
  sdnpsim_datasetProtoWrite(pProto, index++, &elem);

  elem.descrElemType   = DNPDEFS_DATASET_DESCR_DAEL; /* 5*/
  elem.dataTypeCode    = (DNPDEFS_DATASET_TYPE_CODE)0x04;  /* FLT */
  elem.maxDataLength   = 0x08;
  elem.ancillaryValue.type = DNPDATA_VALUE_STRPTR;
  elem.ancillaryValue.value.pStrValue = (TMWTYPES_UCHAR *)"Current Grnd";
  elem.ancillaryValue.length = (TMWTYPES_UCHAR)strlen((char *)elem.ancillaryValue.value.pStrValue);
  sdnpsim_datasetProtoWrite(pProto, index++, &elem);

  elem.descrElemType   = DNPDEFS_DATASET_DESCR_DAEL; /* 5*/
  elem.dataTypeCode    = (DNPDEFS_DATASET_TYPE_CODE)0x06;  /* BSTR */
  elem.maxDataLength   = 0x02;
  elem.ancillaryValue.type = DNPDATA_VALUE_STRPTR;
  elem.ancillaryValue.value.pStrValue = (TMWTYPES_UCHAR *)"Triggers";
  elem.ancillaryValue.length = (TMWTYPES_UCHAR)strlen((char *)elem.ancillaryValue.value.pStrValue);
  sdnpsim_datasetProtoWrite(pProto, index++, &elem);

  elem.descrElemType   = DNPDEFS_DATASET_DESCR_DAEL; /* 5*/
  elem.dataTypeCode    = (DNPDEFS_DATASET_TYPE_CODE)0x01;  /* VSTR */
  elem.maxDataLength   = 0x08;
  elem.ancillaryValue.type = DNPDATA_VALUE_STRPTR;
  elem.ancillaryValue.value.pStrValue = (TMWTYPES_UCHAR *)"Fault Class";
  elem.ancillaryValue.length = (TMWTYPES_UCHAR)strlen((char *)elem.ancillaryValue.value.pStrValue);
  sdnpsim_datasetProtoWrite(pProto, index++, &elem);

  elem.descrElemType   = DNPDEFS_DATASET_DESCR_DAEL; /* 5*/
  elem.dataTypeCode    = (DNPDEFS_DATASET_TYPE_CODE)0x02;  /* UINT */
  elem.maxDataLength   = 0x04;
  elem.ancillaryValue.type = DNPDATA_VALUE_STRPTR;
  elem.ancillaryValue.value.pStrValue = (TMWTYPES_UCHAR *)"Dist";
  elem.ancillaryValue.length = (TMWTYPES_UCHAR)strlen((char *)elem.ancillaryValue.value.pStrValue);
  sdnpsim_datasetProtoWrite(pProto, index++, &elem);

  /********************************/
  /* Now the Data Set descriptors */
  /********************************/
  pDescr =  (SDNPSIM_DATASET_DESCR_DATA *)sdnpsim_datasetDescrCreatePoint(pDbHandle, 0, TMWDEFS_FALSE);
  if(pDescr == TMWDEFS_NULL)
    return;
  
  sdnpsim_datasetDescrAssignClass(pDescr, TMWDEFS_CLASS_MASK_THREE); 
  sdnpsim_datasetDescrSetChars(pDescr, 0x0f);

  index = 0;
  indexElemIndex = 0;

  /* Since the Data Set id is the mandatory first element,
   * don't store it 
   */
  elem.descrElemType   = (DNPDEFS_DATASET_DESCR_CODE)0x04;
  elem.dataTypeCode    = (DNPDEFS_DATASET_TYPE_CODE)0x00;
  elem.maxDataLength   = 0x00;
  elem.ancillaryValue.type = DNPDATA_VALUE_STRPTR;
  elem.ancillaryValue.value.pStrValue = (TMWTYPES_UCHAR *)"Fdr 11-A Fault";
  elem.ancillaryValue.length = (TMWTYPES_UCHAR)strlen((char *)elem.ancillaryValue.value.pStrValue);
  sdnpsim_datasetDescrWriteCont(pDescr, index++, &elem);

  elem.descrElemType   = (DNPDEFS_DATASET_DESCR_CODE)0x05;
  elem.dataTypeCode    = (DNPDEFS_DATASET_TYPE_CODE)0x04;  /* FLT */
  elem.maxDataLength   = 0x08;
  elem.ancillaryValue.type = DNPDATA_VALUE_STRPTR;
  elem.ancillaryValue.value.pStrValue = (TMWTYPES_UCHAR *)"Current Phs A";
  elem.ancillaryValue.length = (TMWTYPES_UCHAR)strlen((char *)elem.ancillaryValue.value.pStrValue);
  sdnpsim_datasetDescrWriteCont(pDescr, index++, &elem);
  indexElem.pointType  = 30;
  indexElem.pointIndex = 4;
  sdnpsim_datasetDescrWriteIndex(pDescr, indexElemIndex++, &indexElem);

  elem.descrElemType   = (DNPDEFS_DATASET_DESCR_CODE)0x05;
  elem.dataTypeCode    = (DNPDEFS_DATASET_TYPE_CODE)0x04;  /* FLT */
  elem.maxDataLength   = 0x08;
  elem.ancillaryValue.type = DNPDATA_VALUE_STRPTR;
  elem.ancillaryValue.value.pStrValue = (TMWTYPES_UCHAR *)"Current Phs B";
  elem.ancillaryValue.length = (TMWTYPES_UCHAR)strlen((char *)elem.ancillaryValue.value.pStrValue);
  sdnpsim_datasetDescrWriteCont(pDescr, index++, &elem);
  indexElem.pointType  = 30;
  indexElem.pointIndex = 5;
  sdnpsim_datasetDescrWriteIndex(pDescr, indexElemIndex++, &indexElem);

  elem.descrElemType   = (DNPDEFS_DATASET_DESCR_CODE)0x05;
  elem.dataTypeCode    = (DNPDEFS_DATASET_TYPE_CODE)0x04;  /* FLT */
  elem.maxDataLength   = 0x08;
  elem.ancillaryValue.type = DNPDATA_VALUE_STRPTR;
  elem.ancillaryValue.value.pStrValue = (TMWTYPES_UCHAR *)"Current Phs C";
  elem.ancillaryValue.length = (TMWTYPES_UCHAR)strlen((char *)elem.ancillaryValue.value.pStrValue);
  sdnpsim_datasetDescrWriteCont(pDescr, index++, &elem);
  indexElem.pointType  = 30;
  indexElem.pointIndex = 6;
  sdnpsim_datasetDescrWriteIndex(pDescr, indexElemIndex++, &indexElem);

  elem.descrElemType   = (DNPDEFS_DATASET_DESCR_CODE)0x05;
  elem.dataTypeCode    = (DNPDEFS_DATASET_TYPE_CODE)0x04;  /* FLT */
  elem.maxDataLength   = 0x08;
  elem.ancillaryValue.type = DNPDATA_VALUE_STRPTR;
  elem.ancillaryValue.value.pStrValue = (TMWTYPES_UCHAR *)"Current Grnd";
  elem.ancillaryValue.length = (TMWTYPES_UCHAR)strlen((char *)elem.ancillaryValue.value.pStrValue);
  sdnpsim_datasetDescrWriteCont(pDescr, index++, &elem);
  indexElem.pointType  = 30;
  indexElem.pointIndex = 7;
  sdnpsim_datasetDescrWriteIndex(pDescr, indexElemIndex++, &indexElem);

  elem.descrElemType   = (DNPDEFS_DATASET_DESCR_CODE)0x05;
  elem.dataTypeCode    = (DNPDEFS_DATASET_TYPE_CODE)0x06;  /* BSTR */
  elem.maxDataLength   = 0x02;
  elem.ancillaryValue.type = DNPDATA_VALUE_STRPTR;
  elem.ancillaryValue.value.pStrValue = (TMWTYPES_UCHAR *)"Triggers";
  elem.ancillaryValue.length = (TMWTYPES_UCHAR)strlen((char *)elem.ancillaryValue.value.pStrValue);
  sdnpsim_datasetDescrWriteCont(pDescr, index++, &elem);
  indexElem.pointType  = 41;
  indexElem.pointIndex = 8;
  sdnpsim_datasetDescrWriteIndex(pDescr, indexElemIndex++, &indexElem);

  elem.descrElemType   = (DNPDEFS_DATASET_DESCR_CODE)0x05;
  elem.dataTypeCode    = (DNPDEFS_DATASET_TYPE_CODE)0x01;  /* VSTR */
  elem.maxDataLength   = 0x08;
  elem.ancillaryValue.type = DNPDATA_VALUE_STRPTR;
  elem.ancillaryValue.value.pStrValue = (TMWTYPES_UCHAR *)"Fault Class";
  elem.ancillaryValue.length = (TMWTYPES_UCHAR)strlen((char *)elem.ancillaryValue.value.pStrValue);
  sdnpsim_datasetDescrWriteCont(pDescr, index++, &elem);
  indexElem.pointType  = 110;
  indexElem.pointIndex = 9;
  sdnpsim_datasetDescrWriteIndex(pDescr, indexElemIndex++, &indexElem);

  elem.descrElemType   = (DNPDEFS_DATASET_DESCR_CODE)0x05;
  elem.dataTypeCode    = (DNPDEFS_DATASET_TYPE_CODE)0x02;  /* UINT */
  elem.maxDataLength   = 0x04;
  elem.ancillaryValue.type = DNPDATA_VALUE_STRPTR;
  elem.ancillaryValue.value.pStrValue = (TMWTYPES_UCHAR *)"Dist";
  elem.ancillaryValue.length = (TMWTYPES_UCHAR)strlen((char *)elem.ancillaryValue.value.pStrValue);
  sdnpsim_datasetDescrWriteCont(pDescr, index++, &elem);
  indexElem.pointType  = 3;
  indexElem.pointIndex = 10;
  sdnpsim_datasetDescrWriteIndex(pDescr, indexElemIndex++, &indexElem);

  /* This is the second descriptor from the tech bulletin, which uses a prototype */
  pDescr =  (SDNPSIM_DATASET_DESCR_DATA *)sdnpsim_datasetDescrCreatePoint(pDbHandle, 1, TMWDEFS_FALSE);
  if(pDescr == TMWDEFS_NULL)
    return;

  sdnpsim_datasetDescrAssignClass(pDescr, TMWDEFS_CLASS_MASK_THREE);
  sdnpsim_datasetDescrSetChars(pDescr, 0x0f);
 
  index = 0;
  indexElemIndex = 0;

  /* Since the Data Set id is the mandatory first element,
   * don't store it 
   */
  elem.descrElemType   = (DNPDEFS_DATASET_DESCR_CODE)0x04;
  elem.dataTypeCode    = (DNPDEFS_DATASET_TYPE_CODE)0x00;
  elem.maxDataLength   = 0x00;
  elem.ancillaryValue.type = DNPDATA_VALUE_STRPTR;
  elem.ancillaryValue.value.pStrValue = (TMWTYPES_UCHAR *)"Fdr 11-A Fault";
  elem.ancillaryValue.length = (TMWTYPES_UCHAR)strlen((char *)elem.ancillaryValue.value.pStrValue);
  sdnpsim_datasetDescrWriteCont(pDescr, index++, &elem);

  elem.descrElemType   = (DNPDEFS_DATASET_DESCR_CODE)0x06;
  elem.dataTypeCode    = (DNPDEFS_DATASET_TYPE_CODE)0x00;  
  elem.maxDataLength   = 0x00;
  elem.ancillaryValue.type = DNPDATA_VALUE_STRPTR;
  /* Fault Data after 16 bytes of UUID */
  /*elem.ancillaryValue.value.pStrValue = (TMWTYPES_UCHAR *)"\xc2\xf4\x10\x10\x65\xb3\x11\xd1\xa2\x9f\x00\xaa\x00\xc1\x5a\x82Fault Data"; */
  elem.ancillaryValue.value.pStrValue = (TMWTYPES_UCHAR *)"\xc2\xf4\x10\x10\x65\xb3\x11\xd1\xa2\x9f\x00\xaa\x00\xc1\x5a\x82\x46\x61\x75\x6c\x74\x20\x44\x61\x74\x61";
  elem.ancillaryValue.length = 26;
  sdnpsim_datasetDescrWriteCont(pDescr, index++, &elem);

  /* This data descriptor was not in the tech bulletin */
  pDescr =  (SDNPSIM_DATASET_DESCR_DATA *)sdnpsim_datasetDescrCreatePoint(pDbHandle, 2, TMWDEFS_FALSE);
  if(pDescr == TMWDEFS_NULL)
    return;

  sdnpsim_datasetDescrAssignClass(pDescr, TMWDEFS_CLASS_MASK_THREE); 
  sdnpsim_datasetDescrSetChars(pDescr, 0x0f);

  index = 0;
  indexElemIndex = 0;

  /* Since the Data Set id is the mandatory first element,
   * don't store it 
   */
  elem.descrElemType   = (DNPDEFS_DATASET_DESCR_CODE)0x04;
  elem.dataTypeCode    = (DNPDEFS_DATASET_TYPE_CODE)0x00;
  elem.maxDataLength   = 0x00;
  elem.ancillaryValue.type = DNPDATA_VALUE_STRPTR;
  elem.ancillaryValue.value.pStrValue = (TMWTYPES_UCHAR *)"Test";
  elem.ancillaryValue.length = (TMWTYPES_UCHAR)strlen((char *)elem.ancillaryValue.value.pStrValue);
  sdnpsim_datasetDescrWriteCont(pDescr, index++, &elem);

  elem.descrElemType   = (DNPDEFS_DATASET_DESCR_CODE)0x05;
  elem.dataTypeCode    = (DNPDEFS_DATASET_TYPE_CODE)0x04; /* FLT */
  elem.maxDataLength   = 0x08;
  elem.ancillaryValue.type = DNPDATA_VALUE_STRPTR;
  elem.ancillaryValue.value.pStrValue = (TMWTYPES_UCHAR *)"Test Phs A";
  elem.ancillaryValue.length = (TMWTYPES_UCHAR)strlen((char *)elem.ancillaryValue.value.pStrValue);
  sdnpsim_datasetDescrWriteCont(pDescr, index++, &elem);
  indexElem.pointType  = 40;
  indexElem.pointIndex = 65537;
  sdnpsim_datasetDescrWriteIndex(pDescr, indexElemIndex++, &indexElem);

  elem.descrElemType   = (DNPDEFS_DATASET_DESCR_CODE)0x05;
  elem.dataTypeCode    = (DNPDEFS_DATASET_TYPE_CODE)0x04; /* FLT */
  elem.maxDataLength   = 0x08;
  elem.ancillaryValue.type = DNPDATA_VALUE_STRPTR;
  elem.ancillaryValue.value.pStrValue = (TMWTYPES_UCHAR *)"Test Phs B";
  elem.ancillaryValue.length = (TMWTYPES_UCHAR)strlen((char *)elem.ancillaryValue.value.pStrValue);
  sdnpsim_datasetDescrWriteCont(pDescr, index++, &elem);
  indexElem.pointType  = 40;
  indexElem.pointIndex = 4000;
  sdnpsim_datasetDescrWriteIndex(pDescr, indexElemIndex++, &indexElem);

  elem.descrElemType   = (DNPDEFS_DATASET_DESCR_CODE)0x05;
  elem.dataTypeCode    = (DNPDEFS_DATASET_TYPE_CODE)0x04; /* FLT */
  elem.maxDataLength   = 0x08;
  elem.ancillaryValue.type = DNPDATA_VALUE_STRPTR;
  elem.ancillaryValue.value.pStrValue = (TMWTYPES_UCHAR *)"Test Phs C";
  elem.ancillaryValue.length = (TMWTYPES_UCHAR)strlen((char *)elem.ancillaryValue.value.pStrValue);
  sdnpsim_datasetDescrWriteCont(pDescr, index++, &elem);
  indexElem.pointType  = 40;
  indexElem.pointIndex = 500;
  sdnpsim_datasetDescrWriteIndex(pDescr, indexElemIndex++, &indexElem);

  /*********************/
  /* Now the Data Sets */
  /*********************/     
  sdnputil_getDateTime((TMWSESN*)pDbHandle->pSDNPSession, &timeStamp); 
  pDataSet =  (SDNPSIM_DATASET_DESCR_DATA *)sdnpsim_datasetCreatePoint(pDbHandle, 0, &timeStamp);
  if(pDataSet == TMWDEFS_NULL)
    return;

  /* Data Set id, and timeStamp are not in elements in simulated data base */ 
  index = 0;

  /* Data elements */
  value.length = 4;
  value.type = DNPDATA_VALUE_SFLT;
  value.value.sfltValue = 100;
  sdnpsim_datasetWrite(pDataSet, index++, &value);
 
  value.length = 4;
  value.type = DNPDATA_VALUE_SFLT;
  value.value.sfltValue = 200;
  sdnpsim_datasetWrite(pDataSet, index++, &value);

  value.length = 4;
  value.type = DNPDATA_VALUE_SFLT;
  value.value.sfltValue = 300;
  sdnpsim_datasetWrite(pDataSet, index++, &value);

  value.length = 4;
  value.type = DNPDATA_VALUE_SFLT;
  value.value.sfltValue = 400;
  sdnpsim_datasetWrite(pDataSet, index++, &value);

  value.length = 2;
  value.type = DNPDATA_VALUE_STRPTR;
  value.value.pStrValue = (TMWTYPES_UCHAR *)"\xb\x0";
  sdnpsim_datasetWrite(pDataSet, index++, &value);

  value.length = 3;
  value.type = DNPDATA_VALUE_STRPTR;
  value.value.pStrValue = (TMWTYPES_UCHAR *)"ACG";
  sdnpsim_datasetWrite(pDataSet, index++, &value);

  value.length = 1;
  value.type = DNPDATA_VALUE_UINT32;
  value.value.uint32Value = 141;
  sdnpsim_datasetWrite(pDataSet, index++, &value);


  /* This data matches Table 11 in Tech Bulletin */
  pDataSet =  (SDNPSIM_DATASET_DESCR_DATA *)sdnpsim_datasetCreatePoint(pDbHandle, 1, &timeStamp);
  if(pDataSet == TMWDEFS_NULL)
    return;

  /* Data Set id, and timeStamp are not in elements in simulated data base */ 
  index = 0;

  /* Data elements */
  value.length = 4;
  value.type = DNPDATA_VALUE_SFLT;
  value.value.sfltValue = (TMWTYPES_SFLOAT)1328.98;
  sdnpsim_datasetWrite(pDataSet, index++, &value);

  value.length = 4;
  value.type = DNPDATA_VALUE_SFLT;
  value.value.sfltValue = (TMWTYPES_SFLOAT)420.17;
  sdnpsim_datasetWrite(pDataSet, index++, &value);

  value.length = 4;
  value.type = DNPDATA_VALUE_SFLT;
  value.value.sfltValue = (TMWTYPES_SFLOAT)1048.43;
  sdnpsim_datasetWrite(pDataSet, index++, &value);

  value.length = 4;
  value.type = DNPDATA_VALUE_SFLT;
  value.value.sfltValue = (TMWTYPES_SFLOAT)342.95;
  sdnpsim_datasetWrite(pDataSet, index++, &value);

  value.length = 1;
  value.type = DNPDATA_VALUE_STRPTR;
  value.value.pStrValue = (TMWTYPES_UCHAR *)"\x0b";
  sdnpsim_datasetWrite(pDataSet, index++, &value);

  value.length = 3;
  value.type = DNPDATA_VALUE_STRPTR;
  value.value.pStrValue = (TMWTYPES_UCHAR *)"ACG";
  sdnpsim_datasetWrite(pDataSet, index++, &value);

  value.length = 1;
  value.type = DNPDATA_VALUE_UINT32;
  value.value.uint32Value = 141;
  sdnpsim_datasetWrite(pDataSet, index++, &value);

  /* a third set of data */ 
  pDataSet =  (SDNPSIM_DATASET_DESCR_DATA *)sdnpsim_datasetCreatePoint(pDbHandle, 2, &timeStamp);
  if(pDataSet == TMWDEFS_NULL)
    return;

  /* Data Set id, and timeStamp are not in elements in simulated data base */ 
  index = 0;

  /* Data elements */
  value.length = 8;
  value.type = DNPDATA_VALUE_DOUBLE;
  value.value.doubleValue = 5.1;
  sdnpsim_datasetWrite(pDataSet, index++, &value);

  value.length = 8;
  value.type = DNPDATA_VALUE_DOUBLE;
  value.value.doubleValue = 6.2;
  sdnpsim_datasetWrite(pDataSet, index++, &value);

  value.length = 8;
  value.type = DNPDATA_VALUE_DOUBLE;
  value.value.doubleValue = 7.3;
  sdnpsim_datasetWrite(pDataSet, index++, &value);
}
 

static void _freedatasetDescrDatas(
  void *pBuf)
{
#if TMWCNFG_USE_DYNAMIC_MEMORY
  int i;
  SDNPSIM_DATASET_DESCR_DATA *pDescr = (SDNPSIM_DATASET_DESCR_DATA *)pBuf;
  for(i=0; i< pDescr->numberDescrElems; i++)
  {
    if(pDescr->descrContents[i].ancillaryValue.type == DNPDATA_VALUE_STRPTR)
    {
      tmwtarg_free(pDescr->descrContents[i].ancillaryValue.value.pStrValue);
    }
  }
#endif
  sdnpmem_free(pBuf);
}

static void _freeDatasetProtos(
   void *pBuf)
{
#if TMWCNFG_USE_DYNAMIC_MEMORY
  int i;
  SDNPSIM_DATASET_PROTO *pProto = (SDNPSIM_DATASET_PROTO *)pBuf;
  for(i=0; i< pProto->numberElems; i++)
  {
    if(pProto->contents[i].ancillaryValue.type == DNPDATA_VALUE_STRPTR)
    {
      tmwtarg_free(pProto->contents[i].ancillaryValue.value.pStrValue);
    }
  }
#endif
  sdnpmem_free(pBuf);
}

static void _clearDataSets(SDNPSIM_DATABASE *pDbHandle)
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
  tmwdlist_destroy(&pDbHandle->datasetDescrDatas, _freedatasetDescrDatas);
  tmwdlist_destroy(&pDbHandle->datasetProtos, _freeDatasetProtos);
}

TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpsim_datasetProtoQuantity(
  void *pHandle)
{
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;
  return((TMWTYPES_USHORT)tmwdlist_size(&pDbHandle->datasetProtos));
}

/* function: sdnpsim_datasetProtoMasterQty */
TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpsim_datasetProtoMasterQty(
  void *pHandle)
{
  SDNPSIM_DATASET_PROTO *pPoint;
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;
  TMWTYPES_USHORT quantity = 0;
 
  pPoint = TMWDEFS_NULL;
  while((pPoint = (SDNPSIM_DATASET_PROTO *)tmwdlist_getAfter(&pDbHandle->datasetProtos, 
    (TMWDLIST_MEMBER *)pPoint)) != TMWDEFS_NULL)
  {
    if(pPoint->fromMaster)
    {
      quantity++;
    }
  }
  return(quantity);
}

/* function: sdnpsim_datasetProtoGetID */
TMWTYPES_BOOL TMWDEFS_CALLBACK sdnpsim_datasetProtoGetID(
  void *pHandle,
  TMWTYPES_UCHAR *pUUID,
  TMWTYPES_USHORT *pPointNum)
{
  SDNPSIM_DATASET_PROTO *pPoint;
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;

  /* See if Data Set Prototype with this UUID exists in database */
  pPoint = TMWDEFS_NULL;
  while((pPoint = (SDNPSIM_DATASET_PROTO *)tmwdlist_getAfter(&pDbHandle->datasetProtos, 
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

void * TMWDEFS_GLOBAL sdnpsim_datasetProtoGetPoint(
  void *pHandle,
  TMWTYPES_USHORT pointNum)
{
  SDNPSIM_DATASET_PROTO *pPoint;
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;

  /* See if Data Set Prototype with this proto id exists in database */
  pPoint = TMWDEFS_NULL;
  while((pPoint = (SDNPSIM_DATASET_PROTO *)tmwdlist_getAfter(&pDbHandle->datasetProtos, 
    (TMWDLIST_MEMBER *)pPoint)) != TMWDEFS_NULL)
  {
    if(pPoint->protoId == pointNum)
       return(pPoint);
  }
  return(TMWDEFS_NULL);
}

DNPDATA_DATASET_DESCR_ELEM * TMWDEFS_GLOBAL sdnpsim_datasetProtoRead(
  void *pPoint,
  TMWTYPES_UCHAR *pNumberElems,
  TMWTYPES_UCHAR *pUUID)
{
  SDNPSIM_DATASET_PROTO *pProto = (SDNPSIM_DATASET_PROTO *)pPoint;
  memcpy(pUUID, pProto->uuid, 16);
  *pNumberElems = pProto->numberElems; 
  return(&pProto->contents[0]);
}

void * TMWDEFS_GLOBAL sdnpsim_datasetProtoCreatePoint(
  void *pHandle,
  TMWTYPES_USHORT pointNum,
  TMWTYPES_UCHAR *pUUID,
  TMWTYPES_BOOL fromMaster)
{
  TMWSIM_POINT *pSimPoint;
  SDNPSIM_DATASET_PROTO *pNewProto;
  SDNPSIM_DATASET_PROTO *pProto = TMWDEFS_NULL;
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;

  /* See if Data Set Prototype with this protoID exists in database */
  while((pProto = (SDNPSIM_DATASET_PROTO *)tmwdlist_getAfter(&pDbHandle->datasetProtos, 
    (TMWDLIST_MEMBER *)pProto)) != TMWDEFS_NULL)
  {
    if(pointNum > pProto->protoId)
    {
      continue;
    } 
    if(pointNum == pProto->protoId)
    {
      return(pProto);
    }
    else 
    {
      /* need to add a new one before this existing prototype*/
      break;
    }
  }
  pNewProto = (SDNPSIM_DATASET_PROTO *)sdnpmem_alloc(SDNPMEM_SIM_DSET_PROTO_TYPE);
  if(pNewProto != TMWDEFS_NULL)
  {
    int j;
    for(j=0; j<SDNPSIM_MAX_DESCR_ELEM; j++)
    {
      /* Init this to uint, so we know there is no string data allocated */
      pNewProto->contents[j].ancillaryValue.type = DNPDATA_VALUE_UINT32;
    }
    pNewProto->protoId = pointNum;
    pNewProto->numberElems = 0; 
    memcpy(pNewProto->uuid, pUUID, 16);
    pNewProto->fromMaster = fromMaster;   
   
    pSimPoint = (TMWSIM_POINT*)pNewProto;

    /* This will initialize simPoint sim point structure to make managed code happy */
    tmwsim_initPoint(pSimPoint, pHandle, pointNum, TMWSIM_TYPE_BINARY); 
    pSimPoint->pSCLHandle = pDbHandle->pSDNPSession;
    
    sdnputil_getDateTime((TMWSESN*)pSimPoint->pSCLHandle, &pSimPoint->timeStamp); 
  }
  else
  {
    return(TMWDEFS_NULL);
  }

  /* if at end of list just add the new one there */
  if(pProto == TMWDEFS_NULL)
  {
    tmwdlist_addEntry(&pDbHandle->datasetProtos,(TMWDLIST_MEMBER *)pNewProto);
  }
  else
  {
    tmwdlist_insertEntryBefore(&pDbHandle->datasetProtos,(TMWDLIST_MEMBER *)pProto, (TMWDLIST_MEMBER *)pNewProto);
 
  } 
  _callCallback((TMWSIM_POINT*)pNewProto, TMWSIM_POINT_ADD, DNPDEFS_OBJ_85_DATASET_PROTO, pointNum);

  return(pNewProto);
}

/* Allow user to delete prototypes even though you must restart the device to continue
 * Renumber the following prototypes 
 */

/* function: sdnpsim_datasetProtoDeletePoint */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_datasetProtoDeletePoint(
  void *pHandle,
  TMWTYPES_USHORT pointNum)
{
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;
  SDNPSIM_DATASET_PROTO *pProto = TMWDEFS_NULL;

  /* See if Data Set Prototype with this protoID exists in database */
  while((pProto = (SDNPSIM_DATASET_PROTO *)tmwdlist_getAfter(&pDbHandle->datasetProtos, 
    (TMWDLIST_MEMBER *)pProto)) != TMWDEFS_NULL)
  {
    if(pointNum == pProto->protoId)
    {
      SDNPSIM_DATASET_PROTO *pDeletedProto = pProto;

      /* renumber the following prototypes, since gaps are not allowed */
      while((pProto = (SDNPSIM_DATASET_PROTO *)tmwdlist_getAfter(&pDbHandle->datasetProtos, 
      (TMWDLIST_MEMBER *)pProto)) != TMWDEFS_NULL)
      {
        pProto->protoId--;
      }

      _callRemoveCallback(pDbHandle, TMWSIM_POINT_DELETE, DNPDEFS_OBJ_85_DATASET_PROTO, 
        (TMWTYPES_USHORT)pointNum);

      tmwdlist_removeEntry(&pDbHandle->datasetProtos, (TMWDLIST_MEMBER*)pDeletedProto);
      
      sdnpmem_free(pDeletedProto);
      return(TMWDEFS_TRUE);
    }  
  }

  return(TMWDEFS_FALSE);
}

TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_datasetProtoWrite(
  void *pPoint,
  TMWTYPES_UCHAR index,
  DNPDATA_DATASET_DESCR_ELEM *pElem)
{
  TMWSIM_POINT *pSimPoint = (TMWSIM_POINT*)pPoint;
  SDNPSIM_DATASET_PROTO *pProto = (SDNPSIM_DATASET_PROTO *)pPoint;
  
  /* Make sure array is big enough */
  if(index >= SDNPSIM_MAX_DESCR_ELEM)
    return(TMWDEFS_FALSE);

#if TMWCNFG_USE_DYNAMIC_MEMORY
  if(pProto->contents[index].ancillaryValue.type == DNPDATA_VALUE_STRPTR) 
  {
    tmwtarg_free(pProto->contents[index].ancillaryValue.value.pStrValue);
  }
#endif

  /* Copy descriptor element into database */
  pProto->contents[index] = *pElem; 

  /* If the value is just pointed to, it needs to be copied into simulated database memory */
  if(pElem->ancillaryValue.type == DNPDATA_VALUE_STRPTR)
  {
    int length = pElem->ancillaryValue.length;
    if(length > DNPCNFG_MAX_STRING_ARRAY)
    {
#if TMWCNFG_USE_DYNAMIC_MEMORY
      pProto->contents[index].ancillaryValue.value.pStrValue = (TMWTYPES_UCHAR*)tmwtarg_alloc(length);
      if (pProto->contents[index].ancillaryValue.value.pStrValue == TMWDEFS_NULL)
      {
        return(TMWDEFS_FALSE);
      }

      memcpy(pProto->contents[index].ancillaryValue.value.pStrValue,
        pElem->ancillaryValue.value.pStrValue, length);
    }
    else
    {
#else
      /* Truncate string to fit in array */
      length = DNPCNFG_MAX_STRING_ARRAY;
#endif
    pProto->contents[index].ancillaryValue.type = DNPDATA_VALUE_STRARRAY;

    memcpy(pProto->contents[index].ancillaryValue.value.strValue,
      pElem->ancillaryValue.value.pStrValue, length);
    }
  }

  sdnputil_getDateTime((TMWSESN*)pSimPoint->pSCLHandle, &pSimPoint->timeStamp); 
  
  if(pProto->numberElems <= index)
  {
    pProto->numberElems = index+1;
    _callCallback((TMWSIM_POINT*)pProto, TMWSIM_POINT_ADD, DNPDEFS_OBJ_85_DATASET_PROTO, (TMWTYPES_USHORT)pProto->protoId);
  }
  else
  {
    _callCallback((TMWSIM_POINT*)pProto, TMWSIM_POINT_UPDATE, DNPDEFS_OBJ_85_DATASET_PROTO, (TMWTYPES_USHORT)pProto->protoId);
  }
    
  return(TMWDEFS_TRUE); 
}

TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpsim_datasetDescrQuantity(
  void *pHandle)
{
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;
  return((TMWTYPES_USHORT)tmwdlist_size(&pDbHandle->datasetDescrDatas));
}

void * TMWDEFS_GLOBAL sdnpsim_datasetDescrCreatePoint(
  void *pHandle,
  TMWTYPES_USHORT pointNum,
  TMWTYPES_BOOL fromMaster)
{
  SDNPSIM_DATASET_DESCR_DATA *pNewDescr;
  SDNPSIM_DATASET_DESCR_DATA *pDescr = TMWDEFS_NULL;
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;

  /* Application layer spec says these 4 device attributes must be supported */
  _createDatasetDeviceAttributes(pDbHandle); 

  /* See if Data Set Descriptor exists in database */
  /* They are in order by index, but there could be point gaps if the user is currently editing
   * Descriptors cannot have point gaps after device is restarted and data sets being used
   */ 
  while((pDescr = (SDNPSIM_DATASET_DESCR_DATA *)tmwdlist_getAfter(&pDbHandle->datasetDescrDatas, 
    (TMWDLIST_MEMBER *)pDescr)) != TMWDEFS_NULL)
  {
    if(pointNum > pDescr->datasetId)
    {
      continue;
    }
    if(pDescr->datasetId == pointNum)
    {
      return(pDescr);
    }  
    else 
    {
      /* need to add a new one before this existing descriptor*/
      break;
    }
  }
  pNewDescr = (SDNPSIM_DATASET_DESCR_DATA *)sdnpmem_alloc(SDNPMEM_SIM_DSET_DESCR_TYPE);
  if(pNewDescr != TMWDEFS_NULL)
  {
    int j;
    for(j=0; j<SDNPSIM_MAX_DESCR_ELEM; j++)
    {
      /* Init this to uint, so we know there is no string data allocated */
      pNewDescr->descrContents[j].ancillaryValue.type = DNPDATA_VALUE_UINT32;
    } 

    for(j=0; j<SDNPSIM_MAX_DATASET_ELEM; j++)
    {
      /* Init the point index attributes */
      pNewDescr->descrIndex[j].pointIndex = 0;
      pNewDescr->descrIndex[j].pointType = 0;

      /* Init this to uint, so we know there is no string data allocated */
      pNewDescr->dataElem[j].type = DNPDATA_VALUE_UINT32;
      pNewDescr->dataElem[j].length = 1;
      pNewDescr->dataElem[j].value.uint32Value = 0;
      pNewDescr->displayValueAsHex[j]= TMWDEFS_FALSE;
    }   

    pNewDescr->simPoint.pDbHandle = pHandle;
    pNewDescr->datasetId = pointNum;
    pNewDescr->numberDescrElems = 0;
    pNewDescr->numberIndexElems = 0;
    pNewDescr->numberDataElems = 0;
    pNewDescr->characteristics = 0; 
    if(fromMaster)
    {
      /* data sets defined by master are readable, writable and static */
      pNewDescr->characteristics |= DNPDEFS_DATASET_CHAR_DF;
      pNewDescr->characteristics |= DNPDEFS_DATASET_CHAR_RD;
      pNewDescr->characteristics |= DNPDEFS_DATASET_CHAR_WR; 
      pNewDescr->characteristics |= DNPDEFS_DATASET_CHAR_ST; 
    }

    pNewDescr->fromMaster = fromMaster;  
    pNewDescr->testingMode = 0;  
    
    /* This will initialize simPoint sim point structure to make managed code happy */
    tmwsim_initPoint((TMWSIM_POINT*)pNewDescr, pHandle, pointNum, TMWSIM_TYPE_BINARY); 
    pNewDescr->simPoint.pSCLHandle = pDbHandle->pSDNPSession;
    sdnputil_getDateTime((TMWSESN*)pDbHandle->pSDNPSession, &pNewDescr->simPoint.timeStamp); 
  }
  else
  {
    return(TMWDEFS_NULL);
  }
  
  /* if at end of list just add the new one there */
  if(pDescr == TMWDEFS_NULL)
  {
    tmwdlist_addEntry(&pDbHandle->datasetDescrDatas,(TMWDLIST_MEMBER *)pNewDescr);
  }
  else
  {
    tmwdlist_insertEntryBefore(&pDbHandle->datasetDescrDatas,(TMWDLIST_MEMBER *)pDescr, (TMWDLIST_MEMBER *)pNewDescr);
  }  
  _callCallback((TMWSIM_POINT*)pNewDescr, TMWSIM_POINT_ADD, DNPDEFS_OBJ_86_DATASET_DESCR, pointNum);
  _callCallback((TMWSIM_POINT*)pNewDescr, TMWSIM_POINT_ADD, DNPDEFS_OBJ_87_DATASET_VALUE, pointNum);

  return(pNewDescr);
}

/* Allow user to delete descriptor even though you must restart the device to continue 
 */

/* function: sdnpsim_datasetDescrDeletePoint */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_datasetDescrDeletePoint(
  void *pHandle,
  TMWTYPES_USHORT pointNum)
{
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;
  SDNPSIM_DATASET_DESCR_DATA *pDescr = TMWDEFS_NULL;

  /* See if Data Set Descriptor with this id exists in database */
  while((pDescr = (SDNPSIM_DATASET_DESCR_DATA *)tmwdlist_getAfter(&pDbHandle->datasetDescrDatas, 
    (TMWDLIST_MEMBER *)pDescr)) != TMWDEFS_NULL)
  {
    if(pDescr->datasetId == pointNum)
    {
      SDNPSIM_DATASET_DESCR_DATA *pDeletedDescr = pDescr;

      /* renumber the following prototypes, since gaps are not allowed */
      while((pDescr = (SDNPSIM_DATASET_DESCR_DATA *)tmwdlist_getAfter(&pDbHandle->datasetDescrDatas, 
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
      
      sdnpmem_free(pDeletedDescr);
      return(TMWDEFS_TRUE);
    }  
  }

  return(TMWDEFS_FALSE);
}

void * TMWDEFS_CALLBACK sdnpsim_datasetDescrGetPoint(
  void *pHandle,
  TMWTYPES_USHORT pointNum)
{
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;
  SDNPSIM_DATASET_DESCR_DATA *pDescr = TMWDEFS_NULL;
  
  /* See if Data Set Descriptor with this id exists in database */
  while((pDescr = (SDNPSIM_DATASET_DESCR_DATA *)tmwdlist_getAfter(&pDbHandle->datasetDescrDatas, 
    (TMWDLIST_MEMBER *)pDescr)) != TMWDEFS_NULL)
  {
    if(pDescr->datasetId == pointNum)
      return pDescr;
  }

  return(TMWDEFS_NULL);
}

TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_datasetDescrGetName(
  void *pHandle,
  TMWTYPES_USHORT pointNumber,
  char *buf)
{ 
  SDNPSIM_DATASET_DESCR_DATA *pPoint = TMWDEFS_NULL;
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;

  /* find Data Set Descriptor in database */ 
  while((pPoint = (SDNPSIM_DATASET_DESCR_DATA *)tmwdlist_getAfter(&pDbHandle->datasetDescrDatas, 
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

DNPDATA_DATASET_DESCR_ELEM * TMWDEFS_GLOBAL sdnpsim_datasetDescrReadCont(
  void *pPoint,
  TMWTYPES_UCHAR *pNumberElems)
{
  SDNPSIM_DATASET_DESCR_DATA *pDescr = (SDNPSIM_DATASET_DESCR_DATA *)pPoint;

  *pNumberElems = pDescr->numberDescrElems; 
  return(&pDescr->descrContents[0]);
}

void TMWDEFS_GLOBAL sdnpsim_datasetDescrReadChars(
  void *pPoint,
  TMWTYPES_UCHAR *pValue)
{
  SDNPSIM_DATASET_DESCR_DATA *pDescr = (SDNPSIM_DATASET_DESCR_DATA *)pPoint;
  *pValue = pDescr->characteristics; 
}

void TMWDEFS_GLOBAL sdnpsim_datasetDescrSetChars(
  void *pPoint,
  TMWTYPES_UCHAR value)
{
  SDNPSIM_DATASET_DESCR_DATA *pDescr = (SDNPSIM_DATASET_DESCR_DATA *)pPoint;
  pDescr->characteristics = value; 
}

DNPDATA_DATASET_DESCR_INDEX * TMWDEFS_GLOBAL sdnpsim_datasetDescrReadIndex(
  void *pPoint,
  TMWTYPES_UCHAR *pNumberElems)
{
  SDNPSIM_DATASET_DESCR_DATA *pDescr = (SDNPSIM_DATASET_DESCR_DATA *)pPoint;

  *pNumberElems = pDescr->numberIndexElems; 
  return(&pDescr->descrIndex[0]);
}

static void TMWDEFS_LOCAL _initDatasetValue(
  SDNPSIM_DATASET_DESCR_DATA *pDescr, 
  DNPDATA_DATASET_DESCR_ELEM *pElem, 
  TMWTYPES_UCHAR index)
{ 
  DNPDATA_DATASET_VALUE *pDataElem;

  /* Don't write past end of array */
  if(index >= SDNPSIM_MAX_DATASET_ELEM)
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
      pDataElem->value.strValue[0]=(TMWTYPES_UCHAR)'O';
      pDataElem->length = 1;
      pDescr->displayValueAsHex[index] = TMWDEFS_TRUE;
      break;
    case DNPDEFS_DATASET_TYPE_BSTR:
      pDataElem->type = DNPDATA_VALUE_STRARRAY;
      pDataElem->value.strValue[0]=(TMWTYPES_UCHAR)'B';
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
  SDNPSIM_DATASET_DESCR_DATA *pDescr, 
  DNPDATA_DATASET_DESCR_ELEM *pElem, 
  TMWTYPES_UCHAR index)
{
  if(pDescr->descrContents[pDescr->numberDescrElems-1].descrElemType == DNPDEFS_DATASET_DESCR_PTYP)
  {
    char *pUUID;
    SDNPSIM_DATASET_PROTO *pProto; 
    SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pDescr->simPoint.pDbHandle;
    
    if(pDescr->descrContents[pDescr->numberDescrElems-1].ancillaryValue.type == DNPDATA_VALUE_STRPTR)
      pUUID = (char*)pDescr->descrContents[pDescr->numberDescrElems-1].ancillaryValue.value.pStrValue;
    else
      pUUID = (char*)pDescr->descrContents[pDescr->numberDescrElems-1].ancillaryValue.value.strValue;

    /* See if Data Set Prototype with this UUID exists in database */
    pProto = TMWDEFS_NULL;
    while((pProto = (SDNPSIM_DATASET_PROTO *)tmwdlist_getAfter(&pDbHandle->datasetProtos, 
      (TMWDLIST_MEMBER *)pProto)) != TMWDEFS_NULL)
    {
      if(!strncmp(pUUID, (char *)pProto->uuid, 16))
      { 
        break;
      }
    }
    if(pProto != TMWDEFS_NULL)
    {
      /* create a data set element for each prototype data element */
      int i;
      for(i=0; i< pProto->numberElems; i++)
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

TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_datasetDescrWriteCont(
  void *pPoint,
  TMWTYPES_UCHAR index,
  DNPDATA_DATASET_DESCR_ELEM *pElem)
{
  TMWSIM_POINT *pSimPoint;
  SDNPSIM_DATASET_DESCR_DATA *pDescr = (SDNPSIM_DATASET_DESCR_DATA *)pPoint;

  /* Make sure array is big enough */
  if(index >= SDNPSIM_MAX_DESCR_ELEM)
    return(TMWDEFS_FALSE);

#if TMWCNFG_USE_DYNAMIC_MEMORY
  /* If memory has been allocated for a previous string, deallocate it */
  if(pDescr->descrContents[index].ancillaryValue.type == DNPDATA_VALUE_STRPTR) 
  {
    tmwtarg_free(pDescr->descrContents[index].ancillaryValue.value.pStrValue);
  }
#endif

  /* Copy descriptor element into database */
  pDescr->descrContents[index] = *pElem; 

  /* If the value is just pointed to, it needs to be copied into simulated database memory */
  if(pElem->ancillaryValue.type == DNPDATA_VALUE_STRPTR)
  {
    int length = pElem->ancillaryValue.length;
    if(length > DNPCNFG_MAX_STRING_ARRAY)
    {
#if TMWCNFG_USE_DYNAMIC_MEMORY
      pDescr->descrContents[index].ancillaryValue.value.pStrValue = (TMWTYPES_UCHAR*)tmwtarg_alloc(length);
      if (pDescr->descrContents[index].ancillaryValue.value.pStrValue == TMWDEFS_NULL)
      {
        return(TMWDEFS_FALSE);
      }

      memcpy(pDescr->descrContents[index].ancillaryValue.value.pStrValue,
        pElem->ancillaryValue.value.pStrValue, length);
    }
    else
    {
#else
      /* Truncate string to fit in array */
      length = DNPCNFG_MAX_STRING_ARRAY;
#endif
      pDescr->descrContents[index].ancillaryValue.type = DNPDATA_VALUE_STRARRAY;

      memcpy(pDescr->descrContents[index].ancillaryValue.value.strValue,
        pElem->ancillaryValue.value.pStrValue, length);
    }
  }
     
  pSimPoint = (TMWSIM_POINT*)pDescr;
  sdnputil_getDateTime((TMWSESN*)pSimPoint->pSCLHandle, &pDescr->simPoint.timeStamp); 

  if(pDescr->numberDescrElems <= index)
  {
    pDescr->numberDescrElems = index+1;

    /* Also add/initialize the data value element(s), based on descriptor element 
       * In case the optional name element incorrectly has type other than NONE, don't add it to data set values
       */
    if(pElem->descrElemType != DNPDEFS_DATASET_DESCR_NAME)
      _initDatasetValues(pDescr, pElem, pDescr->numberDataElems);
    
    _callCallback(pSimPoint, TMWSIM_POINT_ADD, DNPDEFS_OBJ_86_DATASET_DESCR, (TMWTYPES_USHORT)pDescr->datasetId);
  }
  else
  {
    _callCallback(pSimPoint, TMWSIM_POINT_UPDATE, DNPDEFS_OBJ_86_DATASET_DESCR, (TMWTYPES_USHORT)pDescr->datasetId);
  }
  return(TMWDEFS_TRUE);
}

TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_datasetDescrWriteIndex(
  void *pPoint,
  TMWTYPES_UCHAR index,
  DNPDATA_DATASET_DESCR_INDEX *pElem)
{
  SDNPSIM_DATASET_DESCR_DATA *pDescr = (SDNPSIM_DATASET_DESCR_DATA *)pPoint;

  /* Make sure array is big enough */
  if(index >= SDNPSIM_MAX_DATASET_ELEM)
    return(TMWDEFS_FALSE);

  /* Copy descriptor element into database */
  pDescr->descrIndex[index] = *pElem; 

  if(pDescr->numberIndexElems <= index)
  {
    pDescr->numberIndexElems = index+1;
  }
  _callCallback((TMWSIM_POINT*)pDescr, TMWSIM_POINT_UPDATE, DNPDEFS_OBJ_86_DATASET_DESCR, (TMWTYPES_USHORT)pDescr->datasetId);
  return(TMWDEFS_TRUE);
}

TMWDEFS_CLASS_MASK TMWDEFS_GLOBAL sdnpsim_datasetDescrEventClass(
  void *pPoint)
{
  SDNPSIM_DATASET_DESCR_DATA *pDescr = (SDNPSIM_DATASET_DESCR_DATA *)pPoint;
  return(pDescr->simPoint.classMask);
}
 
DNPDATA_DATASET_DESCR_ELEM *sdnpsim_datasetDescrGetExpIndex(void *pPoint, TMWTYPES_UCHAR index)
{
  int i;
  DNPDATA_DATASET_DESCR_ELEM *pDescrElem; 
  SDNPSIM_DATASET_DESCR_DATA *pDescr = (SDNPSIM_DATASET_DESCR_DATA *)pPoint;
  int expandedIndex = 0;
  for(i=0; i<pDescr->numberDescrElems; i++)
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
      /* expand out prototype  */
      TMWTYPES_USHORT pointNumber;
      TMWTYPES_UCHAR *pUUID; 

      if(pDescrElem->ancillaryValue.type == DNPDATA_VALUE_STRPTR)
        pUUID = pDescrElem->ancillaryValue.value.pStrValue;
      else
        pUUID = pDescrElem->ancillaryValue.value.strValue;

      if(sdnpsim_datasetProtoGetID(pDescr->simPoint.pDbHandle, pUUID, &pointNumber))
      {
        int j;
        SDNPSIM_DATASET_PROTO *pProto = (SDNPSIM_DATASET_PROTO *)sdnpsim_datasetProtoGetPoint(pDescr->simPoint.pDbHandle, pointNumber);
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

/* function: sdnpsim_datasetAssignClass */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_datasetDescrAssignClass(
  void *pPoint,
  TMWDEFS_CLASS_MASK classMask)
{
  SDNPSIM_DATASET_DESCR_DATA *pDescr = (SDNPSIM_DATASET_DESCR_DATA *)pPoint;
  pDescr->simPoint.classMask = classMask;
  return(TMWDEFS_TRUE);
}
#endif

#if SDNPDATA_SUPPORT_OBJ87
TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpsim_datasetQuantity(
  void *pHandle)
{
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;
  return((TMWTYPES_USHORT)tmwdlist_size(&pDbHandle->datasetDescrDatas));
}

/* function: sdnpsim_datasetMasterQty */
TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpsim_datasetMasterQty(
  void *pHandle)
{
  SDNPSIM_DATASET_DESCR_DATA *pPoint;
  SDNPSIM_DATABASE *pDbHandle = (SDNPSIM_DATABASE *)pHandle;
  TMWTYPES_USHORT quantity = 0;

  pPoint = TMWDEFS_NULL;
  while((pPoint = (SDNPSIM_DATASET_DESCR_DATA *)tmwdlist_getAfter(&pDbHandle->datasetDescrDatas, 
    (TMWDLIST_MEMBER *)pPoint)) != TMWDEFS_NULL)
  {
    if(pPoint->fromMaster)
    {
      quantity++;
    }
  }
  return(quantity);
}

void * TMWDEFS_GLOBAL sdnpsim_datasetCreatePoint(
  void *pHandle,
  TMWTYPES_USHORT pointNum,
  TMWDTIME *pTimeStamp)
{
  SDNPSIM_DATASET_DESCR_DATA *pData = TMWDEFS_NULL;
  pData = (SDNPSIM_DATASET_DESCR_DATA *)sdnpsim_datasetDescrCreatePoint(pHandle, pointNum, 0); 
  if(pData != TMWDEFS_NULL && pTimeStamp != TMWDEFS_NULL)
  {
    pData->simPoint.timeStamp = *pTimeStamp;
    _callCallback((TMWSIM_POINT*)pData, TMWSIM_POINT_ADD, DNPDEFS_OBJ_87_DATASET_VALUE, (TMWTYPES_USHORT)pData->datasetId);
  } 
  return(pData);
}

void * TMWDEFS_GLOBAL sdnpsim_datasetGetPoint(
  void *pHandle,
  TMWTYPES_USHORT pointNum)
{ 
  return(sdnpsim_datasetDescrGetPoint(pHandle, pointNum));
}

DNPDATA_DATASET_VALUE * TMWDEFS_GLOBAL sdnpsim_datasetRead(
  void *pPoint,
  TMWTYPES_UCHAR *pNumberElems,
  TMWDTIME *pTimeStamp)
{
  SDNPSIM_DATASET_DESCR_DATA *pData = (SDNPSIM_DATASET_DESCR_DATA *)pPoint;
  *pTimeStamp = pData->simPoint.timeStamp;
  *pNumberElems = pData->numberDataElems; 
  return(&pData->dataElem[0]);
}

TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_datasetWrite(
  void *pPoint,
  TMWTYPES_UCHAR index,
  DNPDATA_DATASET_VALUE *pElem)
{
  SDNPSIM_DATASET_DESCR_DATA *pData = (SDNPSIM_DATASET_DESCR_DATA *)pPoint;

  /* Make sure array is big enough */
  if(index >= SDNPSIM_MAX_DATASET_ELEM)
    return(TMWDEFS_FALSE);

#if TMWCNFG_USE_DYNAMIC_MEMORY
  if(pData->dataElem[index].type == DNPDATA_VALUE_STRPTR) 
  {
    tmwtarg_free(pData->dataElem[index].value.pStrValue);
  }
#endif

  /* Copy descriptor element into database */
  pData->dataElem[index] = *pElem; 

  /* If the value is just pointed to, it needs to be copied somewhere */
  if(pElem->type == DNPDATA_VALUE_STRPTR)
  {
    int length = pElem->length;
    if(length > DNPCNFG_MAX_STRING_ARRAY)
    {
#if TMWCNFG_USE_DYNAMIC_MEMORY
      pData->dataElem[index].value.pStrValue = (TMWTYPES_UCHAR*)tmwtarg_alloc(length);
      if (pData->dataElem[index].value.pStrValue == TMWDEFS_NULL)
      {
        return(TMWDEFS_FALSE);
      }

      memcpy(pData->dataElem[index].value.pStrValue,
        pElem->value.pStrValue, length);
    }
    else
    {
#else
      /* Truncate string to fit in array */
      length = DNPCNFG_MAX_STRING_ARRAY;
#endif
      pData->dataElem[index].type = DNPDATA_VALUE_STRARRAY;

      memcpy(pData->dataElem[index].value.strValue,
        pElem->value.pStrValue, length);
    }
  }
    
  if(pData->numberDataElems <= index)
  {
    pData->numberDataElems = index+1;
  }

  sdnputil_getDateTime((TMWSESN*)pData->simPoint.pSCLHandle, &pData->simPoint.timeStamp);

  _callCallback((TMWSIM_POINT*)pData, TMWSIM_POINT_UPDATE, DNPDEFS_OBJ_87_DATASET_VALUE, (TMWTYPES_USHORT)pData->datasetId);
  return(TMWDEFS_TRUE); 
}

DNPDEFS_DATASET_ST TMWDEFS_GLOBAL sdnpsim_datasetSelect(
  void *pPoint,
  DNPDATA_DATASET_CTRL_VALUE *pCtrlValues,
  TMWTYPES_UCHAR numberCtrlValues)
{
  SDNPSIM_DATASET_DESCR_DATA *pData = (SDNPSIM_DATASET_DESCR_DATA *)pPoint;
  TMWTARG_UNUSED_PARAM(pCtrlValues);
  TMWTARG_UNUSED_PARAM(numberCtrlValues);
  if(pData->testingMode == TMWSIM_TESTINGMODE_SELECTFAIL)
    return(DNPDEFS_DATASET_ST_NOT_SUPPORTED);

  return(DNPDEFS_DATASET_ST_SUCCESS); 
}

DNPDEFS_DATASET_ST TMWDEFS_GLOBAL sdnpsim_datasetOperate(
  void *pPoint,
  DNPDATA_DATASET_CTRL_VALUE *pCtrlValues,
  TMWTYPES_UCHAR numberCtrlValues)
{
  int i;
  int index;
  DNPDATA_DATASET_VALUE *pValue;
  SDNPSIM_DATASET_DESCR_DATA *pData = (SDNPSIM_DATASET_DESCR_DATA *)pPoint;

  if(pData->testingMode == TMWSIM_TESTINGMODE_OPERATEFAIL)
    return(DNPDEFS_DATASET_ST_NOT_SUPPORTED);

  for(i = 0; i< numberCtrlValues; i++)
  { 
    pValue = &pCtrlValues[i].data;
    index =   pCtrlValues[i].elemIndex;
    if(index > SDNPSIM_MAX_DATASET_ELEM)
      return(DNPDEFS_DATASET_ST_TOO_MANY_OPS);

#if TMWCNFG_USE_DYNAMIC_MEMORY 
    if(pData->dataElem[index].type == DNPDATA_VALUE_STRPTR) 
    {
      tmwtarg_free(pData->dataElem[index].value.pStrValue);
    }
#endif

    /* Copy descriptor element into database */
    pData->dataElem[index] = *pValue; 

    /* If the value is just pointed to, it needs to be copied somewhere */
    if(pValue->type == DNPDATA_VALUE_STRPTR)
    {
      int length = pValue->length;
      if(length > DNPCNFG_MAX_STRING_ARRAY)
      {
#if TMWCNFG_USE_DYNAMIC_MEMORY
        pData->dataElem[index].value.pStrValue = (TMWTYPES_UCHAR*)tmwtarg_alloc(length);
        if (pData->dataElem[index].value.pStrValue == TMWDEFS_NULL)
        {
          return(DNPDEFS_DATASET_ST_HARDWARE_ERROR);
        }

        memcpy(pData->dataElem[index].value.pStrValue,
          pValue->value.pStrValue, length);
      }
      else
      {
#else
        /* Truncate string to fit in array */
        length = DNPCNFG_MAX_STRING_ARRAY;
#endif
        pData->dataElem[index].type = DNPDATA_VALUE_STRARRAY;

        memcpy(pData->dataElem[index].value.strValue,
          pValue->value.strValue, length);
      }
    }

    if(pData->numberDataElems <= index)
    {
      pData->numberDataElems = (TMWTYPES_UCHAR)index+1;
    }

    /* Advance to next element to set */
    index++;
  }

  _callCallback((TMWSIM_POINT*)pData, TMWSIM_POINT_UPDATE, DNPDEFS_OBJ_87_DATASET_VALUE, (TMWTYPES_USHORT)pData->datasetId);
  return(DNPDEFS_DATASET_ST_SUCCESS); 
}

/* function: sdnpsim_datasetTestingMode */
void TMWDEFS_GLOBAL sdnpsim_datasetTestingMode(
  void *pPoint, 
  TMWTYPES_UCHAR mode)
{
  SDNPSIM_DATASET_DESCR_DATA *pData = (SDNPSIM_DATASET_DESCR_DATA *)pPoint;
  pData->testingMode = mode;
}
#endif /* SDNPDATA_SUPPORT_DATASETS */

#if SDNPDATA_SUPPORT_XML2
/* 
 * Set 1 of these to test some other values which generate different xml choices
 */
#define XMLTEST1 0
#define XMLTEST2 0

TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_XmlGetDeviceConfig(
  void *pHandle, 
  SDNPDATA_XML_DEVICECONFIG *pConfig)
{ 
  TMWTARG_UNUSED_PARAM(pHandle);
  /* deviceFunction==outstation */

  STRCPY(pConfig->vendorName, SDNPXML_MAXSTRINGLENGTH, "Triangle MicroWorks Inc.");
  STRCPY(pConfig->deviceName, SDNPXML_MAXSTRINGLENGTH, "Test Harness");
  STRCPY(pConfig->hardwareVersion, SDNPXML_MAXSTRINGLENGTH, "n/a");
  STRCPY(pConfig->softwareVersion, SDNPXML_MAXSTRINGLENGTH, tmwvrsn_TMWVERSION_STRING);
  
  pConfig->documentVersionNumber = 1;
 
  pConfig->dnpLevelSupported.level1 = TMWDEFS_TRUE;
  pConfig->dnpLevelSupported.level2 = TMWDEFS_TRUE;
  pConfig->dnpLevelSupported.level3 = TMWDEFS_TRUE;
  pConfig->dnpLevelSupported.level4 = TMWDEFS_TRUE;
#if XMLTEST1
  pConfig->dnpLevelSupported.level1 = TMWDEFS_FALSE;
  pConfig->dnpLevelSupported.level4 = TMWDEFS_FALSE;
#endif
    
#if XMLTEST1 
  pConfig->supportedFunctionBlocks.mappingToIEC61850ObjectModels = TMWDEFS_TRUE;

  STRCPY(pConfig->notableAdditions[0].note, SDNPXML_MAXSTRINGLENGTH, "This is a note");
  STRCPY(pConfig->notableAdditions[0].notableAddition, SDNPXML_MAXSTRINGLENGTH, "Notable Addition here"); 
  STRCPY(pConfig->notableAdditions[1].note, SDNPXML_MAXSTRINGLENGTH, "This is another note");
  STRCPY(pConfig->notableAdditions[1].notableAddition, SDNPXML_MAXSTRINGLENGTH, "Another Notable Addition here");
#endif
  
  pConfig->configurationMethods.xmlViaOtherTransportMechanism = TMWDEFS_TRUE; 
  pConfig->configurationMethods.protocol = TMWDEFS_TRUE; 

#if XMLTEST1 
  pConfig->configurationMethods.xmlViaFileTransfer = TMWDEFS_TRUE;  
  pConfig->onlineXmlFileNames.dnpDPReadSupported = TMWDEFS_TRUE; 
#endif

  pConfig->onlineXmlFileNames.dnpDPCfgReadSupported = TMWDEFS_TRUE;  
   
#if XMLTEST1 
  pConfig->onlineXmlFileNames.xmlFile[0].readAccess = TMWDEFS_TRUE;
  STRCPY(pConfig->onlineXmlFileNames.xmlFile[0].filename, SDNPXML_MAXSTRINGLENGTH, "dnpDPCfg.xml");
  STRCPY(pConfig->onlineXmlFileNames.xmlFile[0].description, SDNPXML_MAXSTRINGLENGTH, "Device Profile current values in XML format");

  STRCPY(pConfig->onlineXmlFileNames.xmlFile[0].note, SDNPXML_MAXSTRINGLENGTH, "xmlFileName0 note");
  pConfig->onlineXmlFileNames.xmlFile[0].readAccess = TMWDEFS_TRUE;
  pConfig->onlineXmlFileNames.xmlFile[0].writeAccess = TMWDEFS_TRUE;
  STRCPY(pConfig->onlineXmlFileNames.xmlFile[0].filename, SDNPXML_MAXSTRINGLENGTH, "xmlFileName1");
  STRCPY(pConfig->onlineXmlFileNames.xmlFile[0].description, SDNPXML_MAXSTRINGLENGTH, "xmlFileName1 description");
  
  STRCPY(pConfig->onlineXmlFileNames.xmlFile[1].note, SDNPXML_MAXSTRINGLENGTH, "xmlFileName1 note");
  pConfig->onlineXmlFileNames.xmlFile[1].readAccess = TMWDEFS_TRUE;
  pConfig->onlineXmlFileNames.xmlFile[1].writeAccess = TMWDEFS_TRUE;
  STRCPY(pConfig->onlineXmlFileNames.xmlFile[1].filename, SDNPXML_MAXSTRINGLENGTH, "xmlFileName1");
  STRCPY(pConfig->onlineXmlFileNames.xmlFile[1].description, SDNPXML_MAXSTRINGLENGTH, "xmlFileName1 description");

  STRCPY(pConfig->onlineXmlFileNames.note[0], SDNPXML_MAXSTRINGLENGTH, "Note about xml file here"); 
  STRCPY(pConfig->onlineXmlFileNames.note[1], SDNPXML_MAXSTRINGLENGTH, "Note about another xml file here");  

  pConfig->offlineXmlFileNames.dnpDPCapReadSupported = TMWDEFS_TRUE;  
  pConfig->offlineXmlFileNames.dnpDPCfgWriteSupported = TMWDEFS_TRUE;   

  STRCPY(pConfig->offlineXmlFileNames.xmlFile[0].note, SDNPXML_MAXSTRINGLENGTH, "xmlOfflineFileName0 note");
  pConfig->offlineXmlFileNames.xmlFile[0].readAccess = TMWDEFS_TRUE;
  pConfig->offlineXmlFileNames.xmlFile[0].writeAccess = TMWDEFS_TRUE;
  STRCPY(pConfig->offlineXmlFileNames.xmlFile[0].filename, SDNPXML_MAXSTRINGLENGTH, "xmlOfflineFileName1");
  STRCPY(pConfig->offlineXmlFileNames.xmlFile[0].description, SDNPXML_MAXSTRINGLENGTH, "xmlOfflineFileName1 description");
  
  STRCPY(pConfig->offlineXmlFileNames.xmlFile[1].note, SDNPXML_MAXSTRINGLENGTH, "xmlOfflineFileName1 note");
  pConfig->offlineXmlFileNames.xmlFile[1].readAccess = TMWDEFS_TRUE;
  pConfig->offlineXmlFileNames.xmlFile[1].writeAccess = TMWDEFS_TRUE;
  STRCPY(pConfig->offlineXmlFileNames.xmlFile[1].filename, SDNPXML_MAXSTRINGLENGTH, "xmlOfflineFileName1");
  STRCPY(pConfig->offlineXmlFileNames.xmlFile[1].description, SDNPXML_MAXSTRINGLENGTH, "xmlOfflineFileName1 description");
 
  STRCPY(pConfig->offlineXmlFileNames.note[0], SDNPXML_MAXSTRINGLENGTH, "Note about xml offline file here");  
  STRCPY(pConfig->offlineXmlFileNames.note[1], SDNPXML_MAXSTRINGLENGTH, "Note about another xml offline file here");  
#endif

  pConfig->offlineXmlFileNames.dnpDPReadSupported = TMWDEFS_TRUE;  
  pConfig->offlineXmlFileNames.dnpDPCfgReadSupported = TMWDEFS_TRUE;  

  pConfig->connectionsSupported.serial = TMWDEFS_TRUE;
  pConfig->connectionsSupported.network = TMWDEFS_TRUE;
#if XMLTEST1 
  STRCPY(pConfig->connectionsSupported.other[0], SDNPXML_MAXSTRINGLENGTH, "other method");
  STRCPY(pConfig->connectionsSupported.other[1], SDNPXML_MAXSTRINGLENGTH, "another method");
#endif

  return TMWDEFS_TRUE;
}

TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_XmlGetSerialConfig(
  void *pHandle, 
  SDNPDATA_XML_SERIALCONFIG *pConfig)
{
  TMWTARG_UNUSED_PARAM(pHandle);

  memcpy(pConfig->portName, "COM1", 4);

  pConfig->serialParameters.type = SDNPDATA_XML_SERIALPARAMASYNC;

#if XMLTEST1
  pConfig->serialParameters.type = SDNPDATA_XML_SERIALPARAMAEMPTY;
#endif
#if XMLTEST2
  pConfig->serialParameters.type = SDNPDATA_XML_SERIALPARAMOTHER;
  STRCPY(pConfig->serialParameters.other, SDNPXML_MAXSTRINGLENGTH, "other serial parameter");
#endif

  pConfig->baudRate = 19200;

  pConfig->flowControl.none = TMWDEFS_TRUE;

#if XMLTEST1
  pConfig->flowControl.none = TMWDEFS_FALSE;
  pConfig->flowControl.rs232Options.assertsRTSBeforeTx = TMWDEFS_TRUE;
  pConfig->flowControl.rs232Options.assertsDTRBeforeTx = TMWDEFS_FALSE;
  pConfig->flowControl.rs232Options.requiresCTSBeforeTx = SDNPDATA_XML_ASSERTED_ASSERTED;
  pConfig->flowControl.rs232Options.requiresDCDBeforeTx = SDNPDATA_XML_ASSERTED_DEASSERTED;
  pConfig->flowControl.rs232Options.assertsDTRBeforeRx = TMWDEFS_TRUE;
  pConfig->flowControl.rs422Options.requiresIndicationBeforeRx = TMWDEFS_TRUE;
  pConfig->flowControl.rs485Options.requiresRxInactiveBeforeTx = TMWDEFS_TRUE;
#endif

  pConfig->supportsCollisionAvoidance.supported = TMWDEFS_FALSE;
#if XMLTEST1
  pConfig->supportsCollisionAvoidance.supported = TMWDEFS_TRUE;
  STRCPY(pConfig->supportsCollisionAvoidance.yes, SDNPXML_MAXSTRINGLENGTH, "test value");
#endif

  pConfig->interCharacterTimeout.type = SDNPDATA_XML_INTERCHARNOTCHECKED;
#if XMLTEST1
  pConfig->interCharacterTimeout.type = SDNPDATA_XML_INTERCHARNOGAPPERM; 
#endif
#if XMLTEST2
  pConfig->interCharacterTimeout.type = SDNPDATA_XML_INTERCHARVALUEMILL;
  pConfig->interCharacterTimeout.value.value = 111;
#endif

  pConfig->interCharacterGap.type = SDNPDATA_XML_INTERCHARGAPNONE;
#if XMLTEST1
  pConfig->interCharacterGap.type = SDNPDATA_XML_INTERCHARGAPMAXBIT;
  pConfig->interCharacterGap.value = 22;
#endif
#if XMLTEST2
  pConfig->interCharacterGap.type = SDNPDATA_XML_INTERCHARGAPMAXMILL;
  pConfig->interCharacterGap.value = 33;
#endif

  return TMWDEFS_TRUE;
}

TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_XmlGetNetworkConfig(
  void *pHandle, 
  SDNPDATA_XML_NETWORKCONFIG *pConfig)
{
  TMWTARG_UNUSED_PARAM(pHandle);

  STRCPY(pConfig->portName, SDNPXML_MAXSTRINGLENGTH, "DNP3");

  pConfig->typeOfEndPoint = SDNPDATA_XML_TCPLISTENING;
#if XMLTEST1
  pConfig->typeOfEndPoint = SDNPDATA_XML_TCPDUAL;
#endif

  STRCPY(pConfig->ipAddress, SDNPXML_MAXSTRINGLENGTH, "10.10.10.1");
  STRCPY(pConfig->subnetMask, SDNPXML_MAXSTRINGLENGTH, "255.255.255.255");
  STRCPY(pConfig->gatewayIPAddress, SDNPXML_MAXSTRINGLENGTH, "1.2.3.4");

  pConfig->tcpConnectionEstablishment.type = SDNPDATA_XML_TCPALL;
#if XMLTEST1
  pConfig->tcpConnectionEstablishment.type = SDNPDATA_XML_TCPBASEDONIP;
#endif

  STRCPY(pConfig->ipAddressOfRemoteDevice[0], SDNPXML_MAXSTRINGLENGTH, "*.*.*.*");
#if XMLTEST1
  STRCPY(pConfig->ipAddressOfRemoteDevice[1], SDNPXML_MAXSTRINGLENGTH, "1.2.3.4");
#endif

  pConfig->tcpListenPort.value = 20000;

  pConfig->tcpPortOfRemoteDevice.value = 20000;

  pConfig->localUDPPort.letSystemChoose = TMWDEFS_TRUE;
#if XMLTEST1
  pConfig->localUDPPort.letSystemChoose = TMWDEFS_FALSE;
  pConfig->localUDPPort.value = 20000;
#endif

  pConfig->udpPortForUnsolicitedNullResponses.none = TMWDEFS_FALSE;
  pConfig->udpPortForUnsolicitedNullResponses.value = 20000;
#if XMLTEST1
  pConfig->udpPortForUnsolicitedNullResponses.none = TMWDEFS_TRUE;
#endif

  pConfig->udpPortForResponses.type = SDNPDATA_XML_UDPUSESOURCE;
#if XMLTEST1
  pConfig->udpPortForResponses.type = SDNPDATA_XML_UDPVALUE;
  pConfig->udpPortForResponses.value = 20001;
#endif
#if XMLTEST2
  pConfig->udpPortForResponses.type = SDNPDATA_XML_UDPNONE;
#endif

  pConfig->multipleMasterConnections.basedOnIPAddress = TMWDEFS_TRUE;
#if XMLTEST1
  pConfig->multipleMasterConnections.browsingForStaticData = TMWDEFS_TRUE; 
#endif
#if XMLTEST2
  pConfig->multipleMasterConnections.notSupported = TMWDEFS_TRUE; 
#endif

  pConfig->timeSynchronization.type = SDNPDATA_XML_TIMESYNCLAN;
#if XMLTEST1
  pConfig->timeSynchronization.type = SDNPDATA_XML_TIMESYNCOTHER;
  STRCPY(pConfig->timeSynchronization.other, SDNPXML_MAXSTRINGLENGTH, "other time sync method");
#endif
#if XMLTEST2
  pConfig->timeSynchronization.type = SDNPDATA_XML_TIMESYNCNOTSUP;
#endif

  return TMWDEFS_TRUE;
}

TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_XmlGetOutstationConfig(
  void *pHandle, 
  SDNPDATA_XML_OUTSTATIONCONFIG *pConfig)
{
  TMWTARG_UNUSED_PARAM(pHandle);

  pConfig->timeSyncRequired.type = SDNPDATA_XML_TIMESYNCREQPERIOD;
  pConfig->timeSyncRequired.value = 1800;

#if XMLTEST1
  pConfig->timeSyncRequired.type = SDNPDATA_XML_TIMESYNCREQNEVER;
#endif
  
#if XMLTEST2
  pConfig->timeSyncRequired.type = SDNPDATA_XML_TIMESYNCREQWITHIN;
  pConfig->timeSyncRequired.value = 1000;
#endif

  pConfig->deviceTroubleBit.type = SDNPDATA_XML_DEVICETROUBLENEVER;
#if XMLTEST1
  pConfig->deviceTroubleBit.type = SDNPDATA_XML_DEVICETROUBLEREASON;
  STRCPY(pConfig->deviceTroubleBit.reasonForSetting, SDNPXML_MAXSTRINGLENGTH, "This is the reason we set device trouble bit");   
#endif

  /* fileHandleTimeout, SCL will determine this */ 
   
  pConfig->eventBufferOverflowBehavior.type = SDNPDATA_XML_EVENT_DISCNEW;

#if XMLTEST1
  pConfig->eventBufferOverflowBehavior.type = SDNPDATA_XML_EVENT_OTHER;
  STRCPY(pConfig->eventBufferOverflowBehavior.other, SDNPXML_MAXSTRINGLENGTH, "per Object Group"); 
#endif
#if XMLTEST2
  pConfig->eventBufferOverflowBehavior.type = SDNPDATA_XML_EVENT_DISCOLD;
#endif

  /* 2.09 changed STRCPY(pConfig->eventBufferOrganization, SDNPXML_MAXSTRINGLENGTH, "per Object Group");  */

  /* sendsMultiFragmentResponses, SCL will determine this */

  /* requestsLastFragmentConfirmation, SCL will determine this */

  /* settingsPreservedThroughDeviceReset */
  pConfig->settingsPreservedThroughDeviceReset.assignClass = TMWDEFS_FALSE;
  pConfig->settingsPreservedThroughDeviceReset.analogDeadbands = TMWDEFS_FALSE;
  pConfig->settingsPreservedThroughDeviceReset.dataSetPrototypes = TMWDEFS_FALSE;
  pConfig->settingsPreservedThroughDeviceReset.dataSetDescriptors = TMWDEFS_FALSE;
  pConfig->settingsPreservedThroughDeviceReset.FC31ActivateConfiguration = TMWDEFS_FALSE;
#if XMLTEST1
  pConfig->settingsPreservedThroughDeviceReset.assignClass = TMWDEFS_TRUE;
  pConfig->settingsPreservedThroughDeviceReset.analogDeadbands = TMWDEFS_TRUE;
  pConfig->settingsPreservedThroughDeviceReset.dataSetPrototypes = TMWDEFS_TRUE;
  pConfig->settingsPreservedThroughDeviceReset.dataSetDescriptors = TMWDEFS_TRUE;
  pConfig->settingsPreservedThroughDeviceReset.FC31ActivateConfiguration = TMWDEFS_TRUE;
#endif
#if XMLTEST2
  pConfig->settingsPreservedThroughDeviceReset.assignClass = TMWDEFS_TRUE;
  pConfig->settingsPreservedThroughDeviceReset.analogDeadbands = TMWDEFS_FALSE;
  pConfig->settingsPreservedThroughDeviceReset.dataSetPrototypes = TMWDEFS_TRUE;
  pConfig->settingsPreservedThroughDeviceReset.dataSetDescriptors = TMWDEFS_FALSE;
  pConfig->settingsPreservedThroughDeviceReset.FC31ActivateConfiguration = TMWDEFS_TRUE;
#endif

  /* Supports Configuration Signature, not supported */

  /* Requests Application Confirmation, SCL will determine this */
  
  return TMWDEFS_TRUE;
}   

TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_XmlGetOutstationPerform(
  void *pHandle, 
  SDNPDATA_XML_OUTSTATIONPERFORM *pConfig)
{ 
  TMWTARG_UNUSED_PARAM(pHandle);

  pConfig->maxTimeBaseDrift.type = SDNPDATA_XML_TIMINGPERFOTHER;
  STRCPY(pConfig->maxTimeBaseDrift.value.other, SDNPXML_MAXSTRINGLENGTH, "Operating System Function");

#if XMLTEST1
  pConfig->maxTimeBaseDrift.type = SDNPDATA_XML_TIMINGPERFVALUE;
  pConfig->maxTimeBaseDrift.value.value = 100;
#endif
 
  pConfig->outstationSetsIIN14.atStartup = TMWDEFS_TRUE;
  pConfig->outstationSetsIIN14.periodically = 10800;
#if XMLTEST1
  pConfig->outstationSetsIIN14.never = TMWDEFS_TRUE;
  pConfig->outstationSetsIIN14.atStartup = TMWDEFS_TRUE;
  pConfig->outstationSetsIIN14.periodically = 60000;
  pConfig->outstationSetsIIN14.whenTimeErrorExceeds = 10000;
#endif

  pConfig->referenceErrorViaDNP.type = SDNPDATA_XML_TIMINGPERFOTHER;
  STRCPY(pConfig->referenceErrorViaDNP.value.other, SDNPXML_MAXSTRINGLENGTH, "Operating System Function");
#if XMLTEST1
  pConfig->referenceErrorViaDNP.type = SDNPDATA_XML_TIMINGPERFVALUE; 
  pConfig->referenceErrorViaDNP.value.value = 1000;
#endif
     
  pConfig->delayMeasurementError.type = SDNPDATA_XML_TIMINGPERFOTHER;
  STRCPY(pConfig->delayMeasurementError.value.other, SDNPXML_MAXSTRINGLENGTH, "Operating System Function");
  
  pConfig->responseTime.type = SDNPDATA_XML_TIMINGPERFVALUE;
  pConfig->responseTime.value.value = 500;
   
  pConfig->startupToIIN14.type = SDNPDATA_XML_TIMINGPERFVALUE;
  pConfig->startupToIIN14.value.value = 0;
   
  pConfig->binaryOrDoubleBitEventError.type = SDNPDATA_XML_TIMINGPERFVALUE;
  pConfig->binaryOrDoubleBitEventError.value.value = 200;

  pConfig->nonBinaryOrDoubleBitEventError.type = SDNPDATA_XML_TIMINGPERFVALUE;
  pConfig->nonBinaryOrDoubleBitEventError.value.value = 200; 

  return TMWDEFS_TRUE;
}

TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_XmlGetFieldConfig(
  void *pHandle, 
  SDNPDATA_XML_FIELDCONFIG *pConfig)
{ 
  TMWTARG_UNUSED_PARAM(pHandle);
  STRCPY(pConfig->outstationLocation, SDNPXML_MAXSTRINGLENGTH, "Raleigh");
  STRCPY(pConfig->outstationId, SDNPXML_MAXSTRINGLENGTH, "987654321");
  STRCPY(pConfig->outstationName, SDNPXML_MAXSTRINGLENGTH, "TMW Test Harness");
  STRCPY(pConfig->deviceSerialNumber, SDNPXML_MAXSTRINGLENGTH, "200-333-333-222");
  return TMWDEFS_TRUE;
}

TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_XmlGetSecurityConfig(
  void *pHandle, 
  SDNPDATA_XML_SECURITYCONFIG *pConfig)
{ 
  TMWTARG_UNUSED_PARAM(pHandle);
 
  pConfig->TLSCipherSuites.type = SDNPDATA_XML_TLSNA;
#if XMLTEST1
  pConfig->TLSCipherSuites.type = SDNPDATA_XML_TLSOTHER;
  STRCPY(pConfig->TLSCipherSuites.other, SDNPXML_MAXSTRINGLENGTH, "Some Cipher");
#endif
#if XMLTEST2
  pConfig->TLSCipherSuites.type = SDNPDATA_XML_TLSDHERSADES;
#endif

  pConfig->changeCipherRequestTimeout.type = SDNPDATA_XML_SECURITYVALUENA;
#if XMLTEST1
  pConfig->changeCipherRequestTimeout.type = SDNPDATA_XML_SECURITYVALUEVALUE;
  pConfig->changeCipherRequestTimeout.value.value = 1000;
#endif
#if XMLTEST2
  pConfig->changeCipherRequestTimeout.type = SDNPDATA_XML_SECURITYVALUEOTHER;
  STRCPY(pConfig->changeCipherRequestTimeout.value.other, SDNPXML_MAXSTRINGLENGTH, "change cipher request timeout other");
#endif

  pConfig->numberCASupported = 1;
 
  pConfig->certificateRevocationCheckTime.type = SDNPDATA_XML_SECURITYVALUENA;
#if XMLTEST1
    pConfig->numberCASupported = 32;
    pConfig->certificateRevocationCheckTime.type = SDNPDATA_XML_SECURITYVALUEVALUE;
    pConfig->certificateRevocationCheckTime.value.value = 12345;
#endif
#if XMLTEST2
  pConfig->certificateRevocationCheckTime.type = SDNPDATA_XML_SECURITYVALUEOTHER;
  STRCPY(pConfig->certificateRevocationCheckTime.value.other, SDNPXML_MAXSTRINGLENGTH, "certificateRevocationCheckTime other");
#endif
 
#if XMLTEST1
  pConfig->additionalCriticalFCs.FC0 = TMWDEFS_TRUE;
  pConfig->additionalCriticalFCs.FC7 = TMWDEFS_TRUE;
  pConfig->additionalCriticalFCs.FC8 = TMWDEFS_TRUE;
#endif
  
  /* Schema 2.09 only allows one of each, even though we support all that the crypto library supports */
  pConfig->remoteUpdateKeyChangeSupported.symmetricCrypto = DNPAUTH_KEYCH_SYMAES256_SHA256;
  pConfig->remoteUpdateKeyChangeSupported.asymmetricCrypto = DNPAUTH_KEYCH_ASYM_RSA2048_SHA256; 
     
  /* Can Default User credentials expire? */
  pConfig->permitUserCredentialExpiry = TMWDEFS_TRUE;
  
#if XMLTEST1
  /* values for testing */
  STRCPY(pConfig->otherCriticalFragments[0].note, SDNPXML_MAXSTRINGLENGTH, "example note");
  STRCPY(pConfig->otherCriticalFragments[0].criticalFragment, SDNPXML_MAXSTRINGLENGTH, "example critical fragment");
  STRCPY(pConfig->otherCriticalFragments[1].note, SDNPXML_MAXSTRINGLENGTH, "second example note");
  STRCPY(pConfig->otherCriticalFragments[1].criticalFragment, SDNPXML_MAXSTRINGLENGTH, "example critical fragment");
#endif

  return TMWDEFS_TRUE;
}

TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_XmlGetBinOutGroupConfig(
  void *pHandle, 
  SDNPDATA_XML_BINOUTGROUPCONFIG *pConfig)
{ 
  TMWTARG_UNUSED_PARAM(pHandle);

  pConfig->minimumPulseTime.type = SDNPDATA_XML_PULSEVALUEFIXED;
  pConfig->minimumPulseTime.fixedPulseValue = 1;
  pConfig->maximumPulseTime.type = SDNPDATA_XML_PULSEVALUEFIXED;
  pConfig->maximumPulseTime.fixedPulseValue = 2147483647;
  pConfig->commandEvents = SDNPDATA_XML_COMMANDEVENTNEVER;
#if XMLTEST1
  pConfig->minimumPulseTime.type = SDNPDATA_XML_PULSEVALUEPERPOINT;
  pConfig->maximumPulseTime.type = SDNPDATA_XML_PULSEVALUEPERPOINT;;
  pConfig->commandEvents = SDNPDATA_XML_COMMANDEVENTSUCCESS;
#endif
#if XMLTEST2
  pConfig->commandEvents = SDNPDATA_XML_COMMANDEVENTALL;
#endif 

  return TMWDEFS_TRUE;
}

TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_XmlGetAnlgInGroupConfig(
  void *pHandle, 
  SDNPDATA_XML_ANLGINGROUPCONFIG *pConfig)
{ 
  TMWTARG_UNUSED_PARAM(pHandle);

  pConfig->analogDeadbandAssignments.type = SDNPDATA_XML_DDBNDASSIGNDNP3;
#if XMLTEST1
  pConfig->analogDeadbandAssignments.type = SDNPDATA_XML_DDBNDASSIGNOTHERMEANS;
#endif
#if XMLTEST2
  pConfig->analogDeadbandAssignments.type = SDNPDATA_XML_DDBNDASSIGNOTHER;
  STRCPY(pConfig->analogDeadbandAssignments.other, SDNPXML_MAXSTRINGLENGTH, "deadbandAssignOther");
#endif
 
  pConfig->analogDeadbandAlgorithm.type = SDNPDATA_XML_DDBNDALGOSIMPLE;
#if XMLTEST1 
  pConfig->analogDeadbandAlgorithm.type = SDNPDATA_XML_DDBNDALGOPERPOINT;
#endif
#if XMLTEST2
  pConfig->analogDeadbandAlgorithm.type = SDNPDATA_XML_DDBNDALGOOTHER;
  STRCPY(pConfig->analogDeadbandAlgorithm.other, SDNPXML_MAXSTRINGLENGTH, "deadbandAlgorithmOther");
#endif
  
  return TMWDEFS_TRUE;
}

TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_XmlGetCntrGroupConfig(
  void *pHandle, 
  SDNPDATA_XML_COUNTERGROUPCONFIG *pConfig)
{ 
  TMWTARG_UNUSED_PARAM(pHandle);

  pConfig->counterRollOver.type = SDNPDATA_XML_CNTRROLLOVER32;
#if XMLTEST1
  pConfig->counterRollOver.type = SDNPDATA_XML_CNTRROLLOVERVALUE;
  pConfig->counterRollOver.value = 0xff;
#endif
#if XMLTEST2
  pConfig->counterRollOver.type = SDNPDATA_XML_CNTRROLLOVEROTHER;
  STRCPY(pConfig->counterRollOver.other, SDNPXML_MAXSTRINGLENGTH, "rollover time");
#endif
 
  pConfig->countersFrozen.type = SDNPDATA_XML_CNTRFROZENMASTERREQ;
#if XMLTEST1 
  pConfig->countersFrozen.type = SDNPDATA_XML_CNTRFROZENLOCALWOTIME;
#endif
#if XMLTEST2
  pConfig->countersFrozen.type = SDNPDATA_XML_CNTRFROZENOTHER;
  STRCPY(pConfig->countersFrozen.other, SDNPXML_MAXSTRINGLENGTH, "rollover time");
#endif
  
  return TMWDEFS_TRUE;
}

TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_XmlGetFileConfig(
  void *pHandle, 
  SDNPDATA_XML_FILECONFIG *pConfig)
{
  TMWTARG_UNUSED_PARAM(pHandle);

  pConfig->fileAuthentication.type = SDNPDATA_XML_FILESOMETIMES;
  STRCPY(pConfig->fileAuthentication.sometimes, SDNPXML_MAXSTRINGLENGTH, "if authentication key, name, and password are configured");
  pConfig->fileAppendMode.type = SDNPDATA_XML_FILEALWAYS;
#if XMLTEST1
  pConfig->fileAuthentication.type = SDNPDATA_XML_FILEALWAYS;
  pConfig->fileAppendMode.type = SDNPDATA_XML_FILESOMETIMES;
  STRCPY(pConfig->fileAppendMode.sometimes, SDNPXML_MAXSTRINGLENGTH, "some files allow appending");
#endif
#if XMLTEST2
  pConfig->fileAuthentication.type = SDNPDATA_XML_FILENEVER;
  pConfig->fileAppendMode.type = SDNPDATA_XML_FILENEVER; 
#endif

  /* permissions */
  pConfig->permissionsSupport.ownerReadAllowed = TMWDEFS_TRUE;
	pConfig->permissionsSupport.ownerWriteAllowed = TMWDEFS_TRUE;
	pConfig->permissionsSupport.ownerExecuteAllowed = TMWDEFS_TRUE;
	pConfig->permissionsSupport.groupReadAllowed = TMWDEFS_TRUE;
	pConfig->permissionsSupport.groupWriteAllowed = TMWDEFS_TRUE;
	pConfig->permissionsSupport.groupExecuteAllowed = TMWDEFS_TRUE;
	pConfig->permissionsSupport.worldReadAllowed = TMWDEFS_TRUE;
	pConfig->permissionsSupport.worldWriteAllowed = TMWDEFS_TRUE;
	pConfig->permissionsSupport.worldExecuteAllowed = TMWDEFS_TRUE;
#if XMLTEST1
	  pConfig->permissionsSupport.worldReadAllowed = TMWDEFS_FALSE;
	  pConfig->permissionsSupport.worldWriteAllowed = TMWDEFS_FALSE;
  	pConfig->permissionsSupport.worldExecuteAllowed = TMWDEFS_FALSE;
#endif

  return TMWDEFS_TRUE;
}

TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_XmlGetPointListDefinition(
  void *pHandle, 
  SDNPDATA_XML_POINTLISTCONFIG *pConfig)
{ 
  TMWTARG_UNUSED_PARAM(pHandle);
 
  pConfig->binaryInputGroup.type = SDNPDATA_XML_POINT_LISTCONFIGURABLE;
  pConfig->doubleBitInputGroup.type = SDNPDATA_XML_POINT_LISTCONFIGURABLE;
  pConfig->binaryOutputGroup.type = SDNPDATA_XML_POINT_LISTCONFIGURABLE; 
  pConfig->counterGroup.type = SDNPDATA_XML_POINT_LISTCONFIGURABLE;
  pConfig->analogInputGroup.type = SDNPDATA_XML_POINT_LISTCONFIGURABLE;
  pConfig->analogOutputGroup.type = SDNPDATA_XML_POINT_LISTCONFIGURABLE;
  pConfig->sequentialFile.type = SDNPDATA_XML_POINT_LISTCONFIGURABLE;
  pConfig->prototypeDefinition.type = SDNPDATA_XML_POINT_LISTCONFIGURABLE;
  pConfig->datasetDefinition.type = SDNPDATA_XML_POINT_LISTCONFIGURABLE;
  pConfig->octetStringGroup.type = SDNPDATA_XML_POINT_LISTCONFIGURABLE;
  pConfig->virtualTermGroup.type = SDNPDATA_XML_POINT_LISTCONFIGURABLE;

#if XMLTEST1
  pConfig->binaryInputGroup.type = SDNPDATA_XML_POINT_LISTFIXED;
  pConfig->doubleBitInputGroup.type = SDNPDATA_XML_POINT_LISTFIXED;
  pConfig->binaryOutputGroup.type = SDNPDATA_XML_POINT_LISTOTHER;
  STRCPY(pConfig->binaryOutputGroup.other, SDNPXML_MAXSTRINGLENGTH, "binary output points list other");
  pConfig->octetStringGroup.type = SDNPDATA_XML_POINT_LISTOTHER;
  STRCPY(pConfig->octetStringGroup.other, SDNPXML_MAXSTRINGLENGTH, "octet sting list other");
  pConfig->prototypeDefinition.type = SDNPDATA_XML_POINT_LISTOTHER;
  STRCPY(pConfig->prototypeDefinition.other, SDNPXML_MAXSTRINGLENGTH, "prototypes list other");
  pConfig->datasetDefinition.type = SDNPDATA_XML_POINT_LISTOTHER;
  STRCPY(pConfig->datasetDefinition.other, SDNPXML_MAXSTRINGLENGTH, "datasets list other");
  pConfig->virtualTermGroup.type = SDNPDATA_XML_POINT_LISTOTHER;
  STRCPY(pConfig->virtualTermGroup.other, SDNPXML_MAXSTRINGLENGTH, "virtual term list other");
#endif

  return TMWDEFS_TRUE;
}

TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_XmlGetBinInPointConfig(
  void *pPoint, 
  SDNPDATA_XML_BININPOINTCONFIG *pConfig)
{ 
  TMWSIM_POINT *pSimPoint = (TMWSIM_POINT *)pPoint;
  
  STRCPY(pConfig->nameState0, SDNPXML_MAXSTRINGLENGTH, "off");
  STRCPY(pConfig->nameState1, SDNPXML_MAXSTRINGLENGTH, "on");
  if(pPoint != TMWDEFS_NULL)
    tmwtarg_snprintf(pConfig->name, sizeof(pConfig->name), "binaryInput%d", pSimPoint->pointNumber); 
  return TMWDEFS_TRUE;
}

TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_XmlGetDblInPointConfig(
  void *pPoint, 
  SDNPDATA_XML_DBLINPOINTCONFIG *pConfig)
{ 
  TMWSIM_POINT *pSimPoint = (TMWSIM_POINT *)pPoint;
  
  STRCPY(pConfig->nameState0, SDNPXML_MAXSTRINGLENGTH, "Intermediate");
  STRCPY(pConfig->nameState1, SDNPXML_MAXSTRINGLENGTH, "Off");
  STRCPY(pConfig->nameState2, SDNPXML_MAXSTRINGLENGTH, "On");
  STRCPY(pConfig->nameState3, SDNPXML_MAXSTRINGLENGTH, "Indeterminate");
 
  if(pPoint != TMWDEFS_NULL)
    tmwtarg_snprintf(pConfig->name, sizeof(pConfig->name), "doubleBitInput%d", pSimPoint->pointNumber); 

  return TMWDEFS_TRUE;
}

TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_XmlGetBinOutPointConfig(
  void *pPoint, 
  SDNPDATA_XML_BINOUTPOINTCONFIG *pConfig)
{ 
  TMWSIM_POINT *pSimPoint = (TMWSIM_POINT *)pPoint;
  
  if(pPoint != TMWDEFS_NULL)
    tmwtarg_snprintf(pConfig->name, sizeof(pConfig->name), "binaryOutput%d", pSimPoint->pointNumber); 

  pConfig->minimumPulseWidth = 0;
  pConfig->maximumPulseWidth = 0xffffffff;
  STRCPY(pConfig->nameState0, SDNPXML_MAXSTRINGLENGTH, "off");
  STRCPY(pConfig->nameState1, SDNPXML_MAXSTRINGLENGTH, "on");
#if XMLTEST1
  /* The following two are part of dnpData */
  pConfig->control = 0x03;
  pConfig->status = DNPDEFS_CROB_ST_TIMEOUT; 
#endif
  return TMWDEFS_TRUE;
}

TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_XmlGetCntrPointConfig(
  void *pPoint, 
  SDNPDATA_XML_CNTRPOINTCONFIG *pConfig)
{ 
  TMWSIM_POINT *pSimPoint = (TMWSIM_POINT *)pPoint;
  
  pConfig->counterRollOver = 0xffffffff;

  if(pPoint != TMWDEFS_NULL)
    tmwtarg_snprintf(pConfig->name, sizeof(pConfig->name), "counter%d", pSimPoint->pointNumber);
  return TMWDEFS_TRUE;
}

TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_XmlGetAnlgInPointConfig(
  void *pPoint, 
  SDNPDATA_XML_ANLGINPOINTCONFIG *pConfig)
{ 
  TMWSIM_POINT *pSimPoint = (TMWSIM_POINT *)pPoint;
  
  if(pPoint != TMWDEFS_NULL)
    tmwtarg_snprintf(pConfig->name, sizeof(pConfig->name), "analogInput%d", pSimPoint->pointNumber); 

  /* This is now a choice of integer or float. 
   * Setting both minInt and maxInt to the same value results in the minFloat and maxFloat values being written to the device profile
   */
  pConfig->minIntTransmittedValue = 0;
  pConfig->maxIntTransmittedValue = 0;

#if TMWCNFG_SUPPORT_DOUBLE 
  pConfig->minFloatTransmittedValue = TMWDEFS_DOUBLE_MIN;
  pConfig->maxFloatTransmittedValue = TMWDEFS_DOUBLE_MAX; 
#elif TMWCNFG_SUPPORT_FLOAT
  pConfig->minFloatTransmittedValue = TMWDEFS_SFLOAT_MIN;
  pConfig->maxFloatTransmittedValue = TMWDEFS_SFLOAT_MAX; 
#else
  pConfig->minFloatTransmittedValue = 0;
  pConfig->maxFloatTransmittedValue = 0;
  pConfig->minIntTransmittedValue = TMWDEFS_LONG_MIN;
  pConfig->maxIntTransmittedValue = TMWDEFS_LONG_MAX;
#endif

  pConfig->scaleOffset = 0;
  pConfig->scaleFactor = 0;
  pConfig->resolution = 0;
  STRCPY(pConfig->units, SDNPXML_MAXSTRINGLENGTH, "n/a");

#if XMLTEST1
  pConfig->scaleOffset = 100;
  pConfig->scaleFactor = 2;
  pConfig->resolution = .001;
  STRCPY(pConfig->units, SDNPXML_MAXSTRINGLENGTH, "amps");
#endif
  return TMWDEFS_TRUE;
}

TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_XmlGetAnlgOutPointConfig(
  void *pPoint, 
  SDNPDATA_XML_ANLGOUTPOINTCONFIG *pConfig)
{ 
  TMWSIM_POINT *pSimPoint = (TMWSIM_POINT *)pPoint;
  
  if(pPoint != TMWDEFS_NULL)
    tmwtarg_snprintf(pConfig->name, sizeof(pConfig->name), "analogOutput%d", pSimPoint->pointNumber); 
  
#if TMWCNFG_SUPPORT_DOUBLE 
  pConfig->minTransmittedValue = TMWDEFS_DOUBLE_MIN;
  pConfig->maxTransmittedValue = TMWDEFS_DOUBLE_MAX; 
#elif TMWCNFG_SUPPORT_FLOAT
  pConfig->minTransmittedValue = TMWDEFS_SFLOAT_MIN;
  pConfig->maxTransmittedValue = TMWDEFS_SFLOAT_MAX; 
#else 
  pConfig->minTransmittedValue = 0;
  pConfig->maxTransmittedValue = 0;
#endif 

  pConfig->scaleOffset = 0;
  pConfig->scaleFactor = 0;
  pConfig->resolution = 0;
  STRCPY(pConfig->units, SDNPXML_MAXSTRINGLENGTH, "n/a");

#if XMLTEST1
  pConfig->scaleOffset = 200;
  pConfig->scaleFactor = 4;
  pConfig->resolution = .01;
  STRCPY(pConfig->units, SDNPXML_MAXSTRINGLENGTH, "volts");
#endif
  return TMWDEFS_TRUE;
}

TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_XmlGetStringPointConfig(
  void *pPoint, 
  SDNPDATA_XML_STRINGPOINTCONFIG *pConfig)
{
  TMWSIM_POINT *pSimPoint = (TMWSIM_POINT *)pPoint;
  
  if(pPoint != TMWDEFS_NULL)
    tmwtarg_snprintf(pConfig->name, sizeof(pConfig->name), "octetString%d", pSimPoint->pointNumber); 

  return TMWDEFS_TRUE;
}

TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_XmlGetVtermPointConfig(
  void *pPoint, 
  SDNPDATA_XML_VTERMPOINTCONFIG *pConfig)
{
  TMWSIM_POINT *pSimPoint = (TMWSIM_POINT *)pPoint;
  
  if(pPoint != TMWDEFS_NULL)
    tmwtarg_snprintf(pConfig->name, sizeof(pConfig->name), "virtualTerminal%d", pSimPoint->pointNumber); 

  return TMWDEFS_TRUE;
}

TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_XmlGetPerFileConfig(
  TMWTYPES_ULONG index, 
  SDNPDATA_XML_PERFILECONFIG *pConfig)
{  
  if(index > 0)
    return TMWDEFS_FALSE;

  STRCPY(pConfig->filename, SDNPXML_MAXSTRINGLENGTH, "dnpDPCfg.xml");
  pConfig->eventClass = TMWDEFS_CLASS_MASK_NONE;
  pConfig->readAuthenticateRequired = TMWDEFS_FALSE;
  pConfig->writeAuthenticateRequired = TMWDEFS_FALSE;
  pConfig->deleteAuthenticateRequired = TMWDEFS_FALSE;
  STRCPY(pConfig->description, SDNPXML_MAXSTRINGLENGTH, "DNP3 Device Profile");

#if XMLTEST1
  if(index > 3)
    return TMWDEFS_FALSE;

  tmwtarg_snprintf(pConfig->filename, sizeof(pConfig->filename), "file%d", index);

  pConfig->eventClass = TMWDEFS_CLASS_MASK_ONE;
  pConfig->readAuthenticateRequired = TMWDEFS_TRUE;
  pConfig->writeAuthenticateRequired = TMWDEFS_FALSE;
  pConfig->deleteAuthenticateRequired = TMWDEFS_FALSE;
  STRCPY(pConfig->description, SDNPXML_MAXSTRINGLENGTH, "This is a file in the database");
#endif
   
  return TMWDEFS_TRUE;
}
#endif /* SDNPDATA_SUPPORT_XML2 */

#endif /* TMWCNFG_USE_SIMULATED_DB */
