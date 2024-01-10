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

/* file: tmwsim.h
 * description: This file simulates generic data points that can be used
 *  for both master and slave simulations.
 */
#ifndef TMWSIM_DEFINED
#define TMWSIM_DEFINED

#include "tmwscl/utils/tmwdefs.h"
#include "tmwscl/utils/tmwcnfg.h"
#include "tmwscl/utils/tmwdlist.h"
#include "tmwscl/utils/tmwdtime.h"
#include "tmwscl/utils/tmwtarg.h"

#if TMWCNFG_USE_SIMULATED_DB

#if TMWCNFG_SIM_USE_DOUBLE
/* Always make simulated database use double precision floating point 
 * for its storage type 
 */
#define TMWSIM_DATA_TYPE TMWTYPES_DOUBLE
#define TMWSIM_DATA_MIN TMWDEFS_DOUBLE_MIN
#define TMWSIM_DATA_MAX TMWDEFS_DOUBLE_MAX

#else
/* Don't allow the simulated database to use doubles or sfloat if 
 * not available to SCL
 */
#if TMWCNFG_SUPPORT_DOUBLE     
#define TMWSIM_DATA_TYPE TMWTYPES_DOUBLE
#define TMWSIM_DATA_MIN TMWDEFS_DOUBLE_MIN
#define TMWSIM_DATA_MAX TMWDEFS_DOUBLE_MAX
#elif TMWCNFG_SUPPORT_FLOAT
#define TMWSIM_DATA_TYPE TMWTYPES_SFLOAT
#define TMWSIM_DATA_MIN TMWDEFS_SFLOAT_MIN
#define TMWSIM_DATA_MAX TMWDEFS_SFLOAT_MAX 
#else
#define TMWSIM_DATA_TYPE TMWTYPES_LONG
#define TMWSIM_DATA_MIN TMWDEFS_LONG_MIN
#define TMWSIM_DATA_MAX TMWDEFS_LONG_MAX 
#endif
#endif

#if !TMW_USE_BINARY_TREE 
/* Use a simple linked list that requires no dynamic memory allocation */
#define TMWSIM_TABLE_HEAD  TMWDLIST
#define TMWSIM_TABLE       TMWDLIST
#endif

/* Maximum length for an octet string in the simulated database */
#define TMWSIM_STRING_MAX_LENGTH 255

/* Maximum length for a DNP Extended String in the simulated database
 * This could be set to match the DNP configuration parameter
 * DNPCNFG_MAX_EXT_STRING_LENGTH. DNP object group 114 supports string
 * sizes between 0 and 65535. This constant can be used to limit the 
 * length to a more practical size to reduce memory consumption.
 */
#ifndef TMWSIM_EXT_STRING_MAX_LENGTH
#define TMWSIM_EXT_STRING_MAX_LENGTH 2048
#endif

/* Maximum length of a point description in the simulated database */
#define TMWSIM_MAX_DESC_LENGTH   128

typedef enum TMWSimTypeEnum {
  TMWSIM_TYPE_BINARY = 0,
  TMWSIM_TYPE_DOUBLE_BINARY,
  TMWSIM_TYPE_ANALOG,
  TMWSIM_TYPE_COUNTER,
  TMWSIM_TYPE_INTEGRATED_TOTAL,
  TMWSIM_TYPE_BITSTRING,
  TMWSIM_TYPE_PARAMETER,
  TMWSIM_TYPE_REFERENCE,
  TMWSIM_TYPE_ATTRIBUTE, /* for DNP device attributes */
  TMWSIM_TYPE_LIST,      /* for DNP device attributes */
  TMWSIM_TYPE_STRING,
  TMWSIM_TYPE_EXT_STRING,
  TMWSIM_TYPE_TIME_INDEXED,
} TMWSIM_TYPE;

#define TMWSIM_TESTINGMODE_SUCCESS      0
#define TMWSIM_TESTINGMODE_FAILURE      1
#define TMWSIM_TESTINGMODE_DELAY        2
#define TMWSIM_TESTINGMODE_OPERATEFAIL  3
#define TMWSIM_TESTINGMODE_SELECTFAIL   4
typedef TMWTYPES_UCHAR TMWSIM_TESTINGMODE; 

typedef struct TMWSimBinaryStruct {
  TMWTYPES_BOOL value;
  TMWTYPES_USHORT relativeTime; /* for 103 */
  TMWTYPES_USHORT faultNumber;  /* for 103 */
  TMWTYPES_UCHAR control;
  TMWTYPES_BOOL lastReportedValue;
} TMWSIM_BINARY;

typedef struct TMWSimDoubleBinaryStruct {
  TMWDEFS_DPI_TYPE value;
  TMWTYPES_USHORT relativeTime;
  TMWDEFS_DPI_TYPE lastReportedValue;
} TMWSIM_DOUBLE_BINARY;

typedef struct TMWSIMAnalogStruct {
  TMWSIM_DATA_TYPE value;
  TMWSIM_DATA_TYPE lowLimit;
  TMWSIM_DATA_TYPE highLimit;
  TMWSIM_DATA_TYPE deadband;
  TMWSIM_DATA_TYPE smoothing;
  TMWSIM_DATA_TYPE lastReportedValue;
  TMWTYPES_USHORT relativeTime; /* for 103 */
  TMWTYPES_USHORT faultNumber;  /* for 103 */
  TMWTYPES_ULONG  parameterIOA[4]; /* For 101/4 */
  TMWTYPES_ULONG  pacnaIOA;        /* For 101/4 */
  TMWTYPES_UCHAR  defaultDeadbandVariation; /* For DNP */
} TMWSIM_ANALOG;

typedef struct TMWSIMIntegratedTotalStruct {
  TMWTYPES_LONG value;
} TMWSIM_INTEGRATED_TOTAL;

typedef struct TMWSIMCounterStruct {
  TMWTYPES_ULONG value;
  TMWTYPES_ULONG frozenValue; /* for IEC */
  TMWTYPES_ULONG frozenValueChanged;
  TMWTYPES_ULONG lastReportedValue;
  /* These three were used for DNP when one sim point held both running and frozen counter. */
  TMWDEFS_CLASS_MASK xfrozenClassMask; /* Not currently used */
  TMWTYPES_UCHAR defaultFrznEventVariation; /* Not currently used */
  TMWTYPES_UCHAR defaultFrznStaticVariation; /*  Not currently used */
  TMWDTIME timeOfFreeze;
} TMWSIM_COUNTER;

typedef struct TMWSIMBitstringStruct {
  TMWTYPES_ULONG value;
  TMWTYPES_USHORT miscTime;
} TMWSIM_BITSTRING;

typedef struct TMWSimParameterStruct {
  TMWTYPES_UCHAR kind;
  struct TMWSIMPointStruct *pPoint;
} TMWSIM_PARAMETER;

typedef struct TMWSimReferenceStruct {
  struct TMWSIMPointStruct *pPoint;
} TMWSIM_REFERENCE;

typedef struct TMWSimAttrStruct {
  TMWTYPES_USHORT point;
  TMWSIM_DATA_TYPE value;
  TMWDTIME timeValue;
  TMWTYPES_UCHAR  *pBuf;
  TMWTYPES_UCHAR  length;
  TMWTYPES_UCHAR  type;
  TMWTYPES_UCHAR  property;
} TMWSIM_ATTRIBUTE; 

typedef struct TMWSimListStruct {
  TMWSIM_TABLE_HEAD listHead;
} TMWSIM_LIST;

typedef struct TMWSimStringStruct {
  TMWTYPES_UCHAR *pBuf;
  TMWTYPES_USHORT length;
  TMWTYPES_BOOL   extString;
} TMWSIM_STRING;

typedef struct TMWSimIndexedTimeStruct {
  TMWDTIME timeValue;
  TMWTYPES_ULONG intervalCount;
  TMWTYPES_BYTE  intervalUnit;
} TMWSIM_INDEXEDTIME;

typedef void (*TMWSIM_CALLBACK_FUNC)(
  void *pCallbackParam,
  void *pPoint);

typedef struct TMWSIMPointStruct {
  /* List Member, must be first entry */
  TMWDLIST_MEMBER listMember;

  void *pSCLHandle;
  void *pDbHandle;
  void *managedPointHandle;
  void *flexPointHandle;
  void (*deleteFlexPoint)(void *);

  TMWSIM_TYPE type;
  TMWTYPES_ULONG pointNumber;
  TMWTYPES_ULONG assocPointNumber;       /* For DNP */
  TMWTYPES_UCHAR flags;
  TMWTYPES_UCHAR defaultEventVariation;  /* For DNP, 101/104 uses this for Transmission Mode */
  TMWTYPES_UCHAR defaultStaticVariation; /* For DNP */
  TMWTYPES_UCHAR eventMode;              /* For DNP */
  TMWTYPES_BOOL local;                   /* For DNP, point is in local mode */
  TMWTYPES_BOOL bStored;
  TMWTYPES_BOOL changed;
  TMWTYPES_BOOL enabled;
  TMWTYPES_BOOL selectRequired;
  TMWTYPES_UCHAR testingMode;
  TMWDEFS_CHANGE_REASON reason;
  TMWDEFS_CLASS_MASK classMask;
  TMWDEFS_CLASS_MASK cmdClassMask; /* For DNP command events */
  TMWDEFS_GROUP_MASK groupMask;
  TMWTYPES_CHAR description[TMWSIM_MAX_DESC_LENGTH];
  TMWSIM_CALLBACK_FUNC pCallbackFunc;
  TMWDEFS_TIME_FORMAT timeFormat;
  TMWDTIME timeStamp;
  TMWTYPES_BOOL freezeEnabled;
  TMWTIMER freezeTimer;
  TMWTYPES_ULONG freezeInterval;
  void *pCallbackParam;
  union {
    TMWSIM_BINARY binary;
    TMWSIM_DOUBLE_BINARY doubleBinary;
    TMWSIM_ANALOG analog;
    TMWSIM_COUNTER counter;
    TMWSIM_INTEGRATED_TOTAL it;
    TMWSIM_BITSTRING bitstring;
    TMWSIM_PARAMETER parameter;
    TMWSIM_REFERENCE reference;
    TMWSIM_LIST      list;
    TMWSIM_ATTRIBUTE attribute;
    TMWSIM_STRING string;
    TMWSIM_INDEXEDTIME indexedTime;
  } data;
} TMWSIM_POINT;

/* Specify callback */
typedef enum {
  TMWSIM_POINT_ADD = 0,
  TMWSIM_POINT_UPDATE,
  TMWSIM_POINT_DELETE,
  TMWSIM_CLEAR_DATABASE
} TMWSIM_EVENT_TYPE;

#ifdef __cplusplus
extern "C" {
#endif
  
  /* These functions involve creating, adding deleting from
   * the database table entity, whether it be a simple linked
   * list, or a more efficient sorted tree.
   */

  /* function: tmwsim_tableCreate
   * purpose:
   * arguments:
   * returns:
   *  void
   */
  TMWTYPES_BOOL TMWDEFS_CALLBACK tmwsim_tableCreate(
    TMWSIM_TABLE_HEAD *pTableHead); 

  /* function: tmwsim_tableDestroy
   * purpose:
   * arguments:
   * returns:
   *  void
   */
  void TMWDEFS_CALLBACK tmwsim_tableDestroy(
    TMWSIM_TABLE_HEAD *pTableHead);
  
  /* function: tmwsim_tableAdd
   * purpose:
   * arguments:
   * returns:
   *  void
   */
  TMWSIM_POINT * TMWDEFS_CALLBACK tmwsim_tableAdd(
    TMWSIM_TABLE_HEAD *pTableHead,
    TMWTYPES_ULONG pointNum);

  /* function: tmwsim_tableDelete
   * purpose:
   * arguments:
   * returns:
   *  void
   */
  TMWTYPES_BOOL TMWDEFS_CALLBACK tmwsim_tableDelete(
    TMWSIM_TABLE_HEAD *pTableHead,
    TMWTYPES_ULONG pointNum);

  /* function: tmwchnl_deleteChannel
   * purpose:
   * arguments:
   * returns:
   *  void
   */
  TMWSIM_POINT * TMWDEFS_CALLBACK tmwsim_tableGetFirstPoint(
    TMWSIM_TABLE_HEAD *pTableHead);

  /* function: tmwsim_tableGetLastPoint
   * purpose:
   * arguments:
   * returns:
   *  void
   */
  TMWSIM_POINT * TMWDEFS_CALLBACK tmwsim_tableGetLastPoint(
    TMWSIM_TABLE_HEAD *pTableHead);

  /* function: tmwsim_tableGetNextPoint
   * purpose:
   * arguments:
   * returns:
   *  void
   */
  TMWSIM_POINT * TMWDEFS_CALLBACK tmwsim_tableGetNextPoint(
    TMWSIM_TABLE_HEAD *pTableHead,
    TMWSIM_POINT *pPoint);

  /* function: tmwsim_tableFindPoint
   * purpose:
   * arguments:
   * returns:
   *  void
   */
  TMWSIM_POINT * TMWDEFS_CALLBACK tmwsim_tableFindPoint(
    TMWSIM_TABLE_HEAD *pTableHead,
    TMWTYPES_ULONG pointNum);

  /* function: tmwsim_tableFindPointByIndex
   * purpose:
   * arguments:
   * returns:
   *  void
   */
  TMWSIM_POINT * TMWDEFS_CALLBACK tmwsim_tableFindPointByIndex(
    TMWSIM_TABLE_HEAD *pTableHead,
    TMWTYPES_USHORT pointIndex);

  /* function: tmwsim_tableSize
   * purpose:
   * arguments:
   * returns:
   *  void
   */
  TMWTYPES_UINT TMWDEFS_CALLBACK tmwsim_tableSize(
    TMWSIM_TABLE_HEAD *pTableHead);

  /* Generic (type independent) functions */

  /* function: tmwsim_newPoint
   * purpose:
   * arguments:
   * returns:
   *  pointer to new point structure if successful 
   *  TMWDEFS_NULL if failure
   */
  TMWDEFS_SCL_API TMWSIM_POINT * TMWDEFS_GLOBAL tmwsim_newPoint(
    void);

  /* function: tmwsim_deletePoint
   * purpose:
   * arguments:
   * returns:
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_CALLBACK tmwsim_deletePoint(
    TMWSIM_POINT *pPoint);

  /* function: tmwsim_initPoint
   * purpose:
   * arguments:
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL tmwsim_initPoint(
    TMWSIM_POINT *pDataPoint,
    void *pHandle,
    TMWTYPES_ULONG pointNum,
    TMWSIM_TYPE type);

  TMWTYPES_ULONG TMWDEFS_GLOBAL tmwsim_getPointNumber(
    TMWSIM_POINT *pDataPoint);

  TMWTYPES_BOOL TMWDEFS_GLOBAL tmwsim_getEnabled(
    TMWSIM_POINT *pDataPoint);

  void TMWDEFS_GLOBAL tmwsim_setEnabled(
    TMWSIM_POINT *pDataPoint,
    TMWTYPES_BOOL enabled);

  /* function: tmwsim_getLocal */
  TMWTYPES_BOOL TMWDEFS_GLOBAL tmwsim_getLocal(
    TMWSIM_POINT *pDataPoint);

  /* function: tmwsim_setLocal */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwsim_setLocal(
    TMWSIM_POINT *pDataPoint, 
    TMWTYPES_BOOL state);
 
  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL tmwsim_getSelectRequired(
    TMWSIM_POINT *pDataPoint);

  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwsim_setSelectRequired(
    TMWSIM_POINT *pDataPoint,
    TMWTYPES_BOOL selectRequired);

  /* function: tmwsim_setAssocPoint */
  TMWDEFS_SCL_API void TMWDEFS_CALLBACK tmwsim_setAssocPoint(
    TMWSIM_POINT *pPoint,
    TMWTYPES_ULONG assocPoint);

  /* function: tmwsim_getAssocPoint */
  TMWDEFS_SCL_API TMWTYPES_ULONG TMWDEFS_CALLBACK tmwsim_getAssocPoint(
    TMWSIM_POINT *pPoint);

  /* function: tmwsim_getTestingMode */
  TMWTYPES_UCHAR TMWDEFS_GLOBAL tmwsim_getTestingMode(
    TMWSIM_POINT *pDataPoint);

  /* function: tmwsim_setTestingMode */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwsim_setTestingMode(
    TMWSIM_POINT *pDataPoint, 
    TMWTYPES_UCHAR mode);  

  /* function: tmwsim_getTimeFormat */
  TMWDEFS_TIME_FORMAT TMWDEFS_GLOBAL tmwsim_getTimeFormat(
    TMWSIM_POINT *pDataPoint);

  /* function: tmwsim_setTimeFormat */
  void TMWDEFS_GLOBAL tmwsim_setTimeFormat(
    TMWSIM_POINT *pDataPoint,
    TMWDEFS_TIME_FORMAT timeFormat);

  /* function: tmwsim_getTimeStamp */
  void TMWDEFS_GLOBAL tmwsim_getTimeStamp(
    TMWSIM_POINT *pDataPoint,
    TMWDTIME *pTimeStamp);
 
  /* function: tmwsim_setTimeStamp */
  void TMWDEFS_GLOBAL tmwsim_setTimeStamp(
    TMWSIM_POINT *pDataPoint,
    TMWDTIME *pTimeStamp);
    
  /* function: tmwsim_getDefEventVariation */
  TMWTYPES_UCHAR TMWDEFS_GLOBAL tmwsim_getDefEventVariation(
    TMWSIM_POINT *pDataPoint);

  /* function: tmwsim_setDefEventVariation */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwsim_setDefEventVariation(
    TMWSIM_POINT *pDataPoint, 
    TMWTYPES_UCHAR defaultVariation); 
     
  /* function: tmwsim_getDefStaticVariation */
  TMWTYPES_UCHAR TMWDEFS_GLOBAL tmwsim_getDefStaticVariation(
    TMWSIM_POINT *pDataPoint);

  /* function: tmwsim_setDefStaticVariation */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwsim_setDefStaticVariation(
    TMWSIM_POINT *pDataPoint, 
    TMWTYPES_UCHAR defaultVariation); 

  TMWDEFS_SCL_API TMWTYPES_UCHAR TMWDEFS_GLOBAL tmwsim_getFlags(
    TMWSIM_POINT *pDataPoint);

  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwsim_setFlags(
    TMWSIM_POINT *pDataPoint, 
    TMWTYPES_UCHAR flags, 
    TMWDEFS_CHANGE_REASON reason);
  
  /* function: tmwsim_getReason */
  TMWDEFS_CHANGE_REASON TMWDEFS_GLOBAL tmwsim_getReason(
    TMWSIM_POINT *pDataPoint);

  /* function: tmwsim_setReason */
  void TMWDEFS_GLOBAL tmwsim_setReason( 
    TMWSIM_POINT *pDataPoint, 
    TMWDEFS_CHANGE_REASON reason);

  TMWDEFS_CLASS_MASK TMWDEFS_GLOBAL tmwsim_getEventClass(
    TMWSIM_POINT *pDataPoint);

  void TMWDEFS_GLOBAL tmwsim_setEventClass(
    TMWSIM_POINT *pDataPoint, 
    TMWDEFS_CLASS_MASK classMask);

  TMWDEFS_CLASS_MASK TMWDEFS_GLOBAL tmwsim_getCmdEventClass(
    TMWSIM_POINT *pDataPoint);

  void TMWDEFS_GLOBAL tmwsim_setCmdEventClass(
    TMWSIM_POINT *pDataPoint, 
    TMWDEFS_CLASS_MASK classMask);

  TMWDEFS_SCL_API TMWDEFS_GROUP_MASK TMWDEFS_GLOBAL tmwsim_getGroupMask(
    TMWSIM_POINT *pDataPoint);

  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwsim_setGroupMask(
    TMWSIM_POINT *pDataPoint, 
    TMWDEFS_GROUP_MASK groupMask);

  TMWTYPES_BOOL TMWDEFS_GLOBAL tmwsim_getChanged(
    TMWSIM_POINT *pDataPoint, 
    TMWDEFS_CHANGE_REASON *pReason);

  /* function: tmwsim_setDescription */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwsim_setDescription(
    TMWSIM_POINT *pDataPoint, 
    TMWTYPES_CHAR *pDesc);

  /* function: tmwsim_getDescription */
  TMWDEFS_SCL_API TMWTYPES_CHAR * TMWDEFS_GLOBAL tmwsim_getDescription(
    TMWSIM_POINT *pDataPoint);

  /* Integrated total Points */
  void TMWDEFS_GLOBAL tmwsim_initIntegratedTotal(
    TMWSIM_POINT *pDataPoint, 
    void *pHandle,
    TMWTYPES_ULONG pointNum); /* pointNum has both rec addr and ioa in it*/

  TMWDEFS_SCL_API TMWTYPES_LONG TMWDEFS_GLOBAL tmwsim_getIntegratedTotalValue(
    TMWSIM_POINT *pDataPoint);

  TMWTYPES_BOOL TMWDEFS_GLOBAL tmwsim_setIntegratedTotalValue(
    TMWSIM_POINT *pDataPoint, 
    TMWTYPES_LONG value, 
    TMWDEFS_CHANGE_REASON reason);

  /* Binary Data Points */
  void TMWDEFS_GLOBAL tmwsim_initBinary(
    TMWSIM_POINT *pDataPoint, 
    void *pHandle,
    TMWTYPES_ULONG pointNum);

  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL tmwsim_getBinaryValue(
    TMWSIM_POINT *pDataPoint);

  TMWTYPES_BOOL TMWDEFS_GLOBAL tmwsim_setBinaryValue(
    TMWSIM_POINT *pDataPoint, 
    TMWTYPES_BOOL value, 
    TMWDEFS_CHANGE_REASON reason);

  TMWTYPES_BOOL TMWDEFS_GLOBAL tmwsim_setBinaryTime(
    TMWSIM_POINT *pDataPoint, 
    TMWTYPES_USHORT relativeTime);

  TMWTYPES_BOOL TMWDEFS_GLOBAL tmwsim_setBinaryFault(
    TMWSIM_POINT *pDataPoint, 
    TMWTYPES_USHORT faultNumber);

  /* function: tmwsim_initDoubleBinary */
  void TMWDEFS_GLOBAL tmwsim_initDoubleBinary(
    TMWSIM_POINT *pDataPoint, 
    void *pHandle,
    TMWTYPES_ULONG pointNum);

  /* function: tmwsim_getDoubleBinaryValue */
  TMWDEFS_DPI_TYPE TMWDEFS_GLOBAL tmwsim_getDoubleBinaryValue(
    TMWSIM_POINT *pDataPoint);

  /* function: tmwsim_setDoubleBinaryValue */
  TMWTYPES_BOOL TMWDEFS_GLOBAL tmwsim_setDoubleBinaryValue(
    TMWSIM_POINT *pDataPoint, 
    TMWDEFS_DPI_TYPE value, 
    TMWDEFS_CHANGE_REASON reason);

  /* Counter Data Points */
  void TMWDEFS_GLOBAL tmwsim_initCounter(
    TMWSIM_POINT *pDataPoint, 
    void *pHandle,
    TMWTYPES_ULONG pointNum);

  TMWDEFS_SCL_API TMWTYPES_ULONG TMWDEFS_GLOBAL tmwsim_getCounterValue(
    TMWSIM_POINT *pDataPoint);

  TMWTYPES_ULONG TMWDEFS_GLOBAL tmwsim_getFrozenCounterValue(
    TMWSIM_POINT *pDataPoint);

  TMWTYPES_BOOL TMWDEFS_GLOBAL tmwsim_setCounterValue(
    TMWSIM_POINT *pDataPoint, 
    TMWTYPES_ULONG value, 
    TMWDEFS_CHANGE_REASON reason);
 
  /* function: tmwsim_getDefFrznStaticVar */
  TMWTYPES_UCHAR TMWDEFS_GLOBAL tmwsim_getDefFrznStaticVar(
    TMWSIM_POINT *pDataPoint);

  /* function: tmwsim_setDefFrznStaticVar */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwsim_setDefFrznStaticVar(
    TMWSIM_POINT *pDataPoint, 
    TMWTYPES_UCHAR defaultVariation); 

  /* function: tmwsim_getDefFrznEventVar */
  TMWTYPES_UCHAR TMWDEFS_GLOBAL tmwsim_getDefFrznEventVar(
    TMWSIM_POINT *pDataPoint);

  /* function: tmwsim_setDefFrznEventVar */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwsim_setDefFrznEventVar(
    TMWSIM_POINT *pDataPoint, 
    TMWTYPES_UCHAR defaultVariation); 

  TMWTYPES_BOOL TMWDEFS_GLOBAL tmwsim_freezeCounter(
    TMWSIM_POINT *pDataPoint);

  /* Analog Data Points */
  void TMWDEFS_GLOBAL tmwsim_initAnalog(
    TMWSIM_POINT *pDataPoint, 
    void *pHandle,
    TMWTYPES_ULONG pointNum,
    TMWSIM_DATA_TYPE lowLimit,
    TMWSIM_DATA_TYPE highLimit,
    TMWSIM_DATA_TYPE deadband,
    TMWSIM_DATA_TYPE smoothing);

  TMWDEFS_SCL_API TMWSIM_DATA_TYPE TMWDEFS_GLOBAL tmwsim_getAnalogValue(
    TMWSIM_POINT *pDataPoint);

  TMWSIM_DATA_TYPE TMWDEFS_GLOBAL tmwsim_getAnalogDeadband(
    TMWSIM_POINT *pDataPoint);

  TMWTYPES_BOOL TMWDEFS_GLOBAL tmwsim_setAnalogValue(
    TMWSIM_POINT *pDataPoint, 
    TMWSIM_DATA_TYPE value, 
    TMWDEFS_CHANGE_REASON reason);

  /* function: tmwsim_getDefDeadbandVariation */
  TMWTYPES_UCHAR TMWDEFS_GLOBAL tmwsim_getDefDeadbandVariation(
    TMWSIM_POINT *pDataPoint);

  /* function: tmwsim_setDefDeadbandVariation */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwsim_setDefDeadbandVariation(
    TMWSIM_POINT *pDataPoint, 
    TMWTYPES_UCHAR defaultVariation); 
     
  /* Counter Data Points */
  void TMWDEFS_GLOBAL tmwsim_initBitstring(
    TMWSIM_POINT *pDataPoint, 
    void *pHandle,
    TMWTYPES_ULONG pointNum);

  TMWTYPES_ULONG TMWDEFS_GLOBAL tmwsim_getBitstringValue(
    TMWSIM_POINT *pDataPoint);

  TMWTYPES_BOOL TMWDEFS_GLOBAL tmwsim_setBitstringValue(
    TMWSIM_POINT *pDataPoint, 
    TMWTYPES_ULONG value, 
    TMWDEFS_CHANGE_REASON reason);

  TMWTYPES_USHORT TMWDEFS_GLOBAL tmwsim_getBitstringTime(
    TMWSIM_POINT *pDataPoint);

  TMWTYPES_BOOL TMWDEFS_GLOBAL tmwsim_setBitstringTime(
    TMWSIM_POINT *pDataPoint, 
    TMWTYPES_USHORT value); 

  /* Parameter Data Points */
  void TMWDEFS_GLOBAL tmwsim_initParameter(
    TMWSIM_POINT *pDataPoint,
    void *pHandle,
    TMWTYPES_ULONG pointNum,
    TMWSIM_POINT *pRefDataPoint);

 /* Reference Data Points */
  void TMWDEFS_GLOBAL tmwsim_initReference(
    TMWSIM_POINT *pDataPoint,
    void *pHandle,
    TMWTYPES_ULONG pointNum,
    TMWSIM_POINT *pRefDataPoint);

  /* Time and date - indexed abs time and long interval */
  /* function: tmwsim_initIndexedTime */
  void TMWDEFS_GLOBAL tmwsim_initIndexedTime(
    TMWSIM_POINT *pDataPoint,
    void *pHandle,
    TMWTYPES_ULONG pointNum);

  /* function: tmwsim_getIndexedTimeTime */
  void TMWDEFS_GLOBAL tmwsim_getIndexedTimeTime(
    TMWSIM_POINT *pDataPoint,
    TMWDTIME *pTimeStamp);

  /* function: tmwsim_getIndexedTimeIntervalCount */
  TMWTYPES_ULONG TMWDEFS_GLOBAL tmwsim_getIndexedTimeIntervalCount(
    TMWSIM_POINT *pDataPoint);

  /* function: tmwsim_getIndexedTimeIntervalUnit */
  TMWTYPES_BYTE TMWDEFS_GLOBAL tmwsim_getIndexedTimeIntervalUnit(
    TMWSIM_POINT *pDataPoint);

  /* function: tmwsim_setStartTime */
  TMWTYPES_BOOL TMWDEFS_GLOBAL tmwsim_setStartTime(
    TMWSIM_POINT *pDataPoint,
    TMWDTIME timeStamp);

  /* function: tmwsim_setIntervalCount */
  TMWTYPES_BOOL TMWDEFS_GLOBAL tmwsim_setIntervalCount(
    TMWSIM_POINT *pDataPoint,
    TMWTYPES_ULONG intervalCount);

  /* function: tmwsim_setIntervalUnit */
  TMWTYPES_BOOL TMWDEFS_GLOBAL tmwsim_setIntervalUnit(
    TMWSIM_POINT *pDataPoint,
    TMWTYPES_BYTE  intervalUnit);

  /* function: tmwsim_setIndexedTime */
  TMWTYPES_BOOL TMWDEFS_GLOBAL tmwsim_setIndexedTime(
    TMWSIM_POINT *pDataPoint,
    TMWDTIME timeStamp,
    TMWTYPES_ULONG intervalCount,
    TMWTYPES_BYTE  intervalUnit);

  /* String Data Points */
  void TMWDEFS_GLOBAL tmwsim_initString(
    TMWSIM_POINT *pDataPoint, 
    void *pHandle,
    TMWTYPES_ULONG pointNum);

  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL tmwsim_getStringValue(
    TMWSIM_POINT *pDataPoint,
    TMWTYPES_UCHAR maxLength,
    TMWTYPES_UCHAR *pBuf,
    TMWTYPES_UCHAR *pBufLength);

  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL tmwsim_setStringValue(
    TMWSIM_POINT *pDataPoint, 
    TMWTYPES_UCHAR *pBuf, 
    TMWTYPES_UCHAR bufLength,
    TMWDEFS_CHANGE_REASON reason);

#if TMWCNFG_SIM_SUPPORT_EXT_STRINGS
  /* Extended String Data Points */
  void TMWDEFS_GLOBAL tmwsim_initExtString(
    TMWSIM_POINT *pDataPoint,
    void *pHandle,
    TMWTYPES_ULONG pointNum);

  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL tmwsim_getExtStringValue(
    TMWSIM_POINT *pDataPoint,
    TMWTYPES_USHORT maxLength,
    TMWTYPES_UCHAR *pBuf,
    TMWTYPES_USHORT *pBufLength);

  TMWTYPES_UCHAR * TMWDEFS_GLOBAL tmwsim_getExtStringPointer(
    TMWSIM_POINT *pDataPoint,
    TMWTYPES_USHORT *pBufLength);

  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL tmwsim_setExtStringValue(
    TMWSIM_POINT *pDataPoint,
    TMWTYPES_UCHAR *pBuf,
    TMWTYPES_USHORT bufLength,
    TMWDEFS_CHANGE_REASON reason);
#endif

  /* function: tmwsim_setCallbackFunc */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwsim_setCallbackFunc(
    TMWSIM_POINT *pDataPoint,
    TMWSIM_CALLBACK_FUNC pCallbackFunc,
    void *pCallbackParam);

  /* function: tmwsim_xmlFormatDesc */
  void tmwsim_xmlFormatDesc(
    TMWTYPES_CHAR *desc,
    TMWTYPES_CHAR *buf,
    size_t bufSize);

  /* function: tmwsim_xmlFormatValue */
  void tmwsim_xmlFormatValue(
    TMWTYPES_CHAR *value,
    TMWTYPES_USHORT strLength,
    TMWTYPES_CHAR *buf,
    size_t bufSize);

#ifdef __cplusplus
}
#endif
#endif
#endif /* TMWSIM_DEFINED */
