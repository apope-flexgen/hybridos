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

/* file: sdnpevnt.h
 * description: This file is intended for internal SCL use only.
 *   Base class for all slave DNP3 events types
 */
#ifndef SDNPEVNT_DEFINED
#define SDNPEVNT_DEFINED

#include "tmwscl/utils/tmwdefs.h"
#include "tmwscl/utils/tmwdtime.h"
#include "tmwscl/utils/tmwsesn.h"
#include "tmwscl/utils/tmwdlist.h"

#include "tmwscl/dnp/sdnpdata.h"
#include "tmwscl/dnp/sdnpmem.h"


/* Structure used to store events */
typedef struct SDNPEventStruct {
  /* List Member, must be first entry */
  TMWDLIST_MEMBER listMember;

  TMWSESN *pSession;
  TMWTYPES_USHORT point;
  TMWTYPES_UCHAR group;
  TMWTYPES_UCHAR flags;
  TMWTYPES_UCHAR defaultVariation;
  TMWDEFS_CLASS_MASK classMask;
  TMWDTIME timeStamp;
  TMWTYPES_BOOL getCurrentValue; /* for analog inputs only */
  TMWTYPES_BOOL eventSent;
} SDNPEVNT;

/* Structure used to store binary input events */
typedef struct Object2EventStruct {
  SDNPEVNT sdnp;
} SDNPEVNT_O002_EVENT;

/* Structure used to store double bit input events */
typedef struct Object4EventStruct {
  SDNPEVNT sdnp;
} SDNPEVNT_O004_EVENT;

/* Structure used to store binary output events */
typedef struct Object11EventStruct {
  SDNPEVNT sdnp;
} SDNPEVNT_O011_EVENT;

/* Structure used to store binary output command events */
typedef struct Object13EventStruct {
  SDNPEVNT sdnp;
} SDNPEVNT_O013_EVENT;

/* Structure used to store binary counter events */
typedef struct Object22EventStruct {
  SDNPEVNT sdnp;
  TMWTYPES_ULONG value;
} SDNPEVNT_O022_EVENT;

/* Structure used to store frozen counter events */
typedef struct Object23EventStruct {
  SDNPEVNT sdnp;
  TMWTYPES_ULONG value;
} SDNPEVNT_O023_EVENT;

/* Structure used to store analog input events */
typedef struct Object32EventStruct {
  SDNPEVNT sdnp;
  TMWTYPES_ANALOG_VALUE value;
} SDNPEVNT_O032_EVENT;

/* Structure used to store frozen analog input events  */
typedef struct Object33EventStruct {
  SDNPEVNT sdnp;
  TMWTYPES_ANALOG_VALUE value;
} SDNPEVNT_O033_EVENT;
 
/* Structure used to store analog output events */ 
typedef struct Object42EventStruct {
  SDNPEVNT sdnp;
  TMWTYPES_ANALOG_VALUE value;
} SDNPEVNT_O042_EVENT;

/* Structure used to store analog output command events */
typedef struct Object43EventStruct {
  SDNPEVNT sdnp;
  TMWTYPES_ANALOG_VALUE value;
} SDNPEVNT_O043_EVENT;

/* Structure used to store Data Set snapshot events */
typedef struct Object88EventStruct {
  SDNPEVNT sdnp; 
  TMWTYPES_USHORT txFragmentSize;
  TMWTYPES_USHORT responseSize;
  TMWTYPES_UCHAR  data[SDNPCNFG_MAX_DSEVENT_LENGTH];
} SDNPEVNT_O088_EVENT;

/* Structure used to store string events */
typedef struct SDNPStringEventStruct {
  SDNPEVNT sdnp;
  TMWTYPES_UCHAR strBuf[DNPDEFS_MAX_STRING_LENGTH];
  TMWTYPES_UCHAR strLength;
} SDNPEVNT_STRING;

/* Structure used to store extended string events */
typedef struct SDNPExtStringEventStruct {
  SDNPEVNT sdnp;
  TMWTYPES_UCHAR strBuf[DNPCNFG_MAX_EXT_STRING_LENGTH];
  TMWTYPES_USHORT strLength;
} SDNPEVNT_EXT_STRING;

/* Structure used to store Secure Authentication Error events */
typedef struct Object120EventStruct {
  SDNPEVNT sdnp;  
  TMWTYPES_ULONG   sequenceNumber;
  TMWTYPES_USHORT  assocId;
  TMWTYPES_USHORT  errorTextLength;
  TMWTYPES_UCHAR   errorTextBuf[DNPDEFS_MAX_STRING_LENGTH];
  TMWTYPES_UCHAR   errorCode;
} SDNPEVNT_0120_EVENT;

/* Structure used to store security statistic events */
typedef struct Object122EventStruct {
  SDNPEVNT sdnp;
  TMWTYPES_USHORT assocId;
  TMWTYPES_ULONG value;
} SDNPEVNT_O122_EVENT;

/* Structure used to store string events */
typedef SDNPEVNT_STRING SDNPEVNT_O111_EVENT;

/* Structure used to store virtual terminal events */
typedef SDNPEVNT_STRING SDNPEVNT_O113_EVENT;

/* Structure used to store string events */
typedef SDNPEVNT_EXT_STRING SDNPEVNT_O115_EVENT;

typedef union {
    SDNPEVNT common;
    /* 2, 4, 11, and 12 do not have any extra data */
    SDNPEVNT_O022_EVENT obj22;
    SDNPEVNT_O023_EVENT obj23;
    SDNPEVNT_O032_EVENT obj32;
    SDNPEVNT_O033_EVENT obj33;
    SDNPEVNT_O042_EVENT obj42;
    SDNPEVNT_O043_EVENT obj43;
#if SDNPDATA_SUPPORT_OBJ88
    SDNPEVNT_O088_EVENT obj88;
#endif
    SDNPEVNT_O111_EVENT obj111;
    SDNPEVNT_O113_EVENT obj113;
#if SDNPDATA_SUPPORT_OBJ115
    SDNPEVNT_O115_EVENT obj115;
#endif
#if DNPCNFG_SUPPORT_AUTHENTICATION
    SDNPEVNT_0120_EVENT obj120;
#if DNPCNFG_SUPPORT_SA_VERSION5
    SDNPEVNT_O122_EVENT obj122;
#endif
#endif
} SDNPEVNT_DATA;

typedef void (*SDNPEVNT_READ_INTO_RESP)(
  TMWSESN *pSession, 
  TMWSESN_TX_DATA *pResponse, 
  SDNPEVNT *pEvent);

typedef TMWTYPES_BOOL (*SDNPEVNT_CHANGED_FUNC)(
  TMWSESN *pSession, 
  void *pPoint,
  TMWTYPES_USHORT pointNum,
  TMWDTIME *pTimeStamp);

typedef void (*SDNPEVNT_STRING_DIAG_FUNC)(
  TMWSESN *pSession,
  TMWTYPES_USHORT pointNum,
  TMWTYPES_UCHAR *pBuf,
  TMWTYPES_UCHAR length);

typedef void *(*SDNPEVNT_GET_POINT_AND_CLASS)(
  TMWSESN *pSession,
  TMWTYPES_USHORT pointNumber,
  TMWDEFS_CLASS_MASK *pClassMask);

typedef void *(*SDNPEVNT_GET_POINT_AND_DEFAULT)(
  TMWSESN *pSession,
  TMWTYPES_USHORT pointNumber,
  TMWDEFS_CLASS_MASK classMask,
  TMWTYPES_UCHAR *pDefaultVariation);

typedef TMWDEFS_EVENT_MODE (*SDNPEVNT_GET_POINT_AND_MODE)(
  TMWSESN *pSession,
  TMWTYPES_USHORT pointNumber);

struct SDNPEventDesc;
typedef void (*SDNPEVNT_SET_VAR_INFO_FUNC)(
  struct SDNPEventDesc *pDesc, 
  TMWTYPES_UCHAR variation);

typedef struct SDNPEventDesc {
  TMWSESN *pSession;
  TMWTYPES_UCHAR readVariation;
  TMWTYPES_UCHAR group;
  TMWTYPES_UCHAR variation;
  TMWTYPES_USHORT sizeInBytes;
  TMWTYPES_USHORT maxEvents;
  TMWTYPES_BOOL truncateStrings;
  TMWDLIST *pEventList;
  TMWDEFS_EVENT_MODE eventMode;
  SDNPMEM_ALLOC_TYPE eventMemType;
  TMWTYPES_USHORT quantity;  
  SDNPEVNT_READ_INTO_RESP pReadIntoRespFunc;
  SDNPEVNT_STRING_DIAG_FUNC pStringDiagFunc;
#if SDNPDATA_SUPPORT_EVENT_SCAN
  SDNPEVNT_GET_POINT_AND_CLASS pGetPointAndClass;
  SDNPEVNT_CHANGED_FUNC pChangedFunc;
#endif
#if SDNPDATA_SUPPORT_EVENT_VAR_POINT
  SDNPEVNT_GET_POINT_AND_DEFAULT pGetPointAndDefault;
  SDNPEVNT_SET_VAR_INFO_FUNC pSetVariationInfo;
#endif
#if SDNPDATA_SUPPORT_EVENT_MODE_POINT
  SDNPEVNT_GET_POINT_AND_MODE pGetPointAndEventMode;
#endif
} SDNPEVNT_DESC;

#ifdef __cplusplus
extern "C" {
#endif

  /* function: sdnpevnt_init */
  void TMWDEFS_GLOBAL sdnpevnt_init(
    TMWTIMER *pTimer,
    TMWDLIST *pEventList);

  /* function: sdnpevnt_close */
  void TMWDEFS_GLOBAL sdnpevnt_close(
    TMWDLIST *pEventList);

  /* function: sdnpevnt_scanForChanges */
  void TMWDEFS_GLOBAL sdnpevnt_scanForChanges(
    TMWSESN *pSession,
    SDNPEVNT_DESC *pDesc);

  /* function: sdnpevnt_addEvent */
  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpevnt_addEvent(
    TMWSESN *pSession, 
    TMWTYPES_USHORT point,
    TMWTYPES_UCHAR flags, 
    TMWDEFS_CLASS_MASK classMask,
    TMWDTIME *pTimeStamp,
    SDNPEVNT_DESC *pDesc,
    SDNPDATA_ADD_EVENT_VALUE *pValue);
  
  /* function: sdnpevnt_updateEvents */
  void TMWDEFS_GLOBAL sdnpevnt_updateEvents(
    TMWSESN *pSession,
    TMWDEFS_CLASS_MASK eventClass);

  /* function: sdnpevnt_countEvents */
  TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpevnt_countEvents(
    TMWSESN *pSession, 
    TMWDEFS_CLASS_MASK classMask,
    SDNPEVNT_DESC *pDesc,
    TMWTYPES_BOOL countAll,
    TMWTYPES_USHORT threshold);

  /* function: sdnpevnt_cleanupEvents() */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpevnt_cleanupEvents(
    TMWTYPES_BOOL deleteEvents,
    SDNPEVNT_DESC *pDesc);

  /* function: sdnpevnt_initReadEventResponse */
  SDNPSESN_READ_STATUS TMWDEFS_GLOBAL sdnpevnt_initReadEventResponse(
    TMWSESN_TX_DATA *pResponse, 
    DNPUTIL_OBJECT_HEADER *pObjHeader, 
    TMWTYPES_UCHAR group, 
    TMWTYPES_UCHAR variation,
    TMWTYPES_USHORT maxPointNumber,
    TMWTYPES_USHORT numberOfEvents,
    TMWTYPES_USHORT dataSizeInBytes,
    TMWTYPES_UCHAR *pResponseQual);

  /* function: sdnpevnt_updateObjectHeader */
  void TMWDEFS_GLOBAL sdnpevnt_updateObjectHeader(
    TMWSESN_TX_DATA *pResponse,
    TMWTYPES_USHORT objectHeaderOffset,
    TMWTYPES_USHORT numEvents);

  /* function: sdnpevnt_readEvents */
  SDNPSESN_READ_STATUS TMWDEFS_GLOBAL sdnpevnt_readEvents(
    TMWSESN *pSession,
    TMWSESN_TX_DATA *pResponse, 
    DNPUTIL_OBJECT_HEADER *pObjHeader, 
    TMWDEFS_CLASS_MASK classMask,
    SDNPEVNT_DESC *pDesc);

  /* function: sdnpevnt_readStringEvents */
  SDNPSESN_READ_STATUS TMWDEFS_GLOBAL sdnpevnt_readStringEvents(
    TMWSESN *pSession,
    TMWSESN_TX_DATA *pResponse, 
    DNPUTIL_OBJECT_HEADER *pObjHeader, 
    TMWDEFS_CLASS_MASK classMask,
    SDNPEVNT_DESC *pDesc);

  /* function: sdnpevnt_readVtermEvents */
  SDNPSESN_READ_STATUS TMWDEFS_GLOBAL sdnpevnt_readVtermEvents(
    TMWSESN *pSession,
    DNPUTIL_RX_MSG *pRequest,
    TMWSESN_TX_DATA *pResponse,
    DNPUTIL_OBJECT_HEADER *pObjHeader,
    SDNPEVNT_DESC *pDesc);

  SDNPEVNT* TMWDEFS_GLOBAL sdnpevent_getNextEvent(
    SDNPEVNT_DESC *pDesc, 
    TMWDEFS_CLASS_MASK classMask,
    SDNPEVNT *pEvent, 
    SDNPEVNT_DATA *pEventData);

#ifdef __cplusplus
}
#endif
#endif /* SDNPEVNT_DEFINED */
