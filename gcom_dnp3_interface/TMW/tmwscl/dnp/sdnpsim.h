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

/* file: sdnpsim.h
 * description: Simulates a DNP slave database.
 *  This file is an example of a simulated DNP slave database interface.
 *  It should NOT be included in the final version of a DNP slave device.
 */
#ifndef SDNPSIM_DEFINED
#define SDNPSIM_DEFINED

#include "tmwscl/utils/tmwcnfg.h"
#if TMWCNFG_USE_SIMULATED_DB

#include "tmwscl/utils/tmwdefs.h"
#include "tmwscl/utils/tmwdtime.h"
#include "tmwscl/utils/tmwsim.h"
#include "tmwscl/dnp/dnpcnfg.h" 
#include "tmwscl/dnp/sdnpsesn.h" 
#include "tmwscl/dnp/sdnpfsim.h" 

#define SDNPSIM_CROB_CTRL SDNPDATA_CROB_CTRL

/* Specify how many points of each data type to simulate */
#define SDNPSIM_NUM_BINARY_INPUTS       0
#define SDNPSIM_NUM_DOUBLE_INPUTS       0
#define SDNPSIM_NUM_BINARY_OUTPUTS      0
#define SDNPSIM_NUM_COUNTERS            0
#define SDNPSIM_NUM_ANALOG_INPUTS       0
#define SDNPSIM_NUM_ANALOG_OUTPUTS      0
#define SDNPSIM_NUM_TIME_INDEXED        0
#define SDNPSIM_NUM_OCTET_STRINGS       0
#define SDNPSIM_NUM_EXT_OCTET_STRINGS   0
#define SDNPSIM_NUM_VIRTUAL_TERMINALS   0

typedef void (*SDNPSIM_CALLBACK_FUNC)(
  void *pCallbackParam, 
  TMWSIM_EVENT_TYPE type,
  DNPDEFS_OBJ_GROUP_ID objectGroup, 
  TMWTYPES_ULONG pointNumber);

/* structure to hold events, 
 * if user managed events are used instead of SCL managed events 
 */
typedef struct {
  /* List Member, must be first entry */
  TMWDLIST_MEMBER      listMember;
  TMWTYPES_BOOL        eventSent;
  TMWTYPES_UCHAR       group;
  TMWTYPES_UCHAR       flags;
  TMWTYPES_UCHAR       defaultVariation;
  TMWDEFS_CLASS_MASK   classMask;
  TMWTYPES_USHORT      point;
  TMWDTIME             timeStamp; 
  union {
    /* This is used for counter and frozen counter events
    */
    TMWTYPES_ULONG         ulValue;

    /* This is used for analog input events */
    TMWTYPES_ANALOG_VALUE  analogValue;

    /* This is used for string and vterm events */
    struct {
      TMWTYPES_UCHAR       length;
      TMWTYPES_UCHAR       buf[DNPDEFS_MAX_STRING_LENGTH+1];
    } stringValue;

#if SDNPDATA_SUPPORT_OBJ115
    /* This is used for extended string events */
    struct {
      TMWTYPES_USHORT      length;
      TMWTYPES_UCHAR       buf[DNPCNFG_MAX_EXT_STRING_LENGTH+1];
    } extendedStringValue;
#endif

    /* This is used for Data Set events */
    struct {
      TMWTYPES_UCHAR       data[DNPCNFG_MAX_TX_FRAGMENT_LENGTH];
      TMWTYPES_UCHAR       numberElems;
    }datasetValue;

    struct {
      TMWTYPES_USHORT      assocId;
      TMWTYPES_ULONG       sequenceNumber;
      TMWTYPES_UCHAR       errorCode;
      TMWTYPES_USHORT      errorTextLength;
      TMWTYPES_CHAR        errorText[DNPAUTH_MAX_ERROR_TEXT_LENGTH +1];
    } authError;

    struct {
      TMWTYPES_USHORT      assocId;
      TMWTYPES_ULONG       ulValue;
    } authSecStat;

  } value;
} SDNPSIM_EVENT;

/* Data Sets */ 
/* Maximum number of Data Set descriptor elements per prototype or descriptor */
#define SDNPSIM_MAX_DESCR_ELEM      DNPCNFG_MAX_DATASET_DESCR_ELEMS

/* Maximum number of Data Set elements per Data Set  
 * Because of contained prototypes, allow more data set elements 
 */
#define SDNPSIM_MAX_DATASET_ELEM    (2*DNPCNFG_MAX_DATASET_DESCR_ELEMS)
#define SDNPSIM_DATASET_STRING_LEN  64

/* Data Set Prototype structure */ 
typedef struct {
  /* must be first field */
  TMWSIM_POINT                      simPoint;

  TMWTYPES_USHORT                   protoId;  /* Point Index */
  TMWTYPES_UCHAR                    uuid[16];

  /* Data Set Prototype was written from master */
  TMWTYPES_BOOL                     fromMaster;  

  TMWTYPES_UCHAR                    numberElems;

  DNPDATA_DATASET_DESCR_ELEM        contents[SDNPSIM_MAX_DESCR_ELEM];
} SDNPSIM_DATASET_PROTO;

/* Data Set Descriptor structure */
typedef struct {
  /* must be first field */
  TMWSIM_POINT                      simPoint;

  TMWTYPES_USHORT                   datasetId;  /* Point Index */

  /* Data Set descriptor was written from master */
  TMWTYPES_BOOL                     fromMaster;
  TMWTYPES_UCHAR                    characteristics;
  TMWTYPES_UCHAR                    numberDescrElems;
  TMWTYPES_UCHAR                    numberIndexElems;
  TMWTYPES_UCHAR                    numberDataElems;
  TMWTYPES_UCHAR                    testingMode;

  DNPDATA_DATASET_DESCR_ELEM        descrContents[SDNPSIM_MAX_DESCR_ELEM];

  /* Because descriptors can contain prototypes, there can be more descriptor 
   * indexes and data elements than descriptor elements 
   */
  DNPDATA_DATASET_DESCR_INDEX       descrIndex[SDNPSIM_MAX_DATASET_ELEM];
  DNPDATA_DATASET_VALUE             dataElem[SDNPSIM_MAX_DATASET_ELEM];

  /* If descriptor data type is OSTR or BSTR display data as hex */
  TMWTYPES_BOOL                     displayValueAsHex[SDNPSIM_MAX_DATASET_ELEM];

} SDNPSIM_DATASET_DESCR_DATA;


/* DNP Secure Authentication Error structure */
typedef struct {

  /* must be first field */
  TMWSIM_POINT                      unused;

  TMWTYPES_USHORT  userNumber;
  TMWTYPES_USHORT  assocId;
  TMWTYPES_ULONG   sequenceNumber;
  TMWTYPES_UCHAR   errorCode;
  TMWDTIME         timeStamp;
  TMWTYPES_CHAR    errorText[128];
  TMWTYPES_USHORT  errorTextLength;

} SDNPSIM_AUTHERROR;

typedef struct {
  /* must be first field */
  TMWSIM_POINT     unused;

  TMWTYPES_USHORT  userRole;
  TMWTYPES_USHORT  userRoleExpiryInterval;  
  TMWTYPES_USHORT  userNumber;  
  TMWTYPES_UCHAR   keyChangeMethod; 
  TMWTYPES_UCHAR   operation; 
  TMWTYPES_CHAR    userName[128]; 
  TMWTYPES_USHORT  userNameLength;  
  TMWTYPES_UCHAR   userPublicKey[128]; 
  TMWTYPES_USHORT  userPublicKeyLength;

} SDNPSIM_AUTHUSER;
  

/* Define simulated database context */
typedef struct SDNPSimDatabaseStruct {

  SDNPSESN *pSDNPSession;

  TMWTYPES_USHORT   iinBits;
  TMWTYPES_BOOL     dbLocalMode;

  TMWSIM_TABLE_HEAD binaryInputs; 
  TMWSIM_TABLE_HEAD doubleInputs;
  TMWSIM_TABLE_HEAD binaryOutputs;
  TMWSIM_TABLE_HEAD binaryCounters;
  TMWSIM_TABLE_HEAD frozenCounters;
  TMWSIM_TABLE_HEAD analogInputs;
  TMWSIM_TABLE_HEAD frozenAnalogInputs;
  TMWSIM_TABLE_HEAD analogOutputs;
  TMWSIM_TABLE_HEAD octetStrings;
  TMWSIM_TABLE_HEAD virtualTerminals;
  TMWSIM_TABLE_HEAD deviceAttrs; 
  TMWSIM_TABLE_HEAD authErrors; 
  TMWSIM_TABLE_HEAD authSecStats;
  TMWSIM_TABLE_HEAD indexedTime;
  
  /* User callbacks */
  SDNPSIM_CALLBACK_FUNC pUpdateCallback;
  void *pUpdateCallbackParam;

#if SDNPDATA_SUPPORT_OBJ70 || SDNPDATA_SUPPORT_XML2
  /* File Transfer support */
  SDNPFSIM_DATABASE sdnpFsim;
#endif

  /* Datasets */
  TMWDLIST datasetProtos;
  TMWDLIST datasetDescrDatas; 
  
  /* User managed events */
  TMWTYPES_USHORT eventIndex;
  SDNPSIM_EVENT *pLastEvent;

  /* Binary and Double Bit Binary Input Events are stored in the same queue
   * so that a read of both will return those events intermixed in
   * order of occurrence.
   */
  TMWDLIST binInEvents;
  TMWDLIST binOutEvents; 
  TMWDLIST binCmdEvents; 
  TMWDLIST binCntrEvents;
  TMWDLIST frznCntrEvents;
  TMWDLIST anlgInEvents;
  TMWDLIST frznAnlgInEvents;
  TMWDLIST anlgOutEvents;
  TMWDLIST anlgCmdEvents;
  TMWDLIST datasetEvents;
  TMWDLIST stringEvents;
  TMWDLIST vtermEvents;
  TMWDLIST extendedStringEvents;
  TMWDLIST authErrorEvents;
  TMWDLIST authStatEvents;

  
  /* Secure Authentication */ 
  TMWTYPES_USHORT authNextUserNumber;
  TMWDEFS_CLASS_MASK authClassMask;
  TMWDLIST authUsers;

  /* Manged SCL database handle*/
  void *managedDBhandle;
  
#if SDNPDATA_SUPPORT_XML 
  /* 
   * Current save state of XML device configuration 
   */
  SDNPXML_SAVE_STATE  xmlSaveState;
  TMWTYPES_USHORT     xmlCurSavePointNum;
  TMWTYPES_UCHAR      xmlCurSaveDAVariation; 
#endif

} SDNPSIM_DATABASE;

#ifdef __cplusplus
extern "C" {
#endif

  /* function: sdnpsim_getIIN */
  TMWTYPES_USHORT sdnpsim_getIIN(
    TMWSESN *pSession);

  /* function: sdnpsim_setIIN */
  void sdnpsim_setIIN(
    TMWSESN *pSession,
    TMWTYPES_USHORT iinBits);

  /* function: sdnpsim_IINQuantity 
   * purpose: Determine how many IIN bits are supported on this session
   * arguments:
   *  pHandle - handle to database returned from sdnpsim_init
   * returns:
   *  16 would indicate no private IIN bits are supported
   *  any number > 16 would indicate how many total IIN bits,
   *   (standard(16) + private bits) are supported.
   */
  TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpsim_IINQuantity(
    void *pHandle);

  /* function: sdnpsim_IINRead 
   * purpose: Read private IIN bit because of read ObjectGroup 80 request
   * arguments:
   *  pHandle - handle to database returned from sdnpsim_init
   *  pointNumber - index of IIN bit to read, 0-15 are standard IIN bits
   * returns:
   *  TMWTYPES_BOOL - TMWTYPES_BOOL if bit is set.
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_IINRead(
    void *pHandle,
    TMWTYPES_USHORT pointNumber);

  /* function: sdnpsim_setLocalMode */
  TMWDEFS_SCL_API void sdnpsim_setLocalMode(
    void *pHandle,
    TMWTYPES_BOOL state);

  /* function: sdnpsim_determineLocalMode 
   * purpose: Determine whether IIN1.5 "Some points are in local mode" should be set
   *  and set it appropriately.
   */
  TMWDEFS_SCL_API TMWTYPES_BOOL sdnpsim_determineLocalMode(
    void *pHandle);

  /* function: sdnpsim_init */
  void * TMWDEFS_GLOBAL sdnpsim_init(
    TMWSESN *pSession);

  /* function: sdnpsim_close */
  void TMWDEFS_GLOBAL sdnpsim_close(
    void *pHandle);

  /* function: sdnpsim_clear */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL sdnpsim_clear(
    void *pHandle);

  /* function: sdnpsim_reset */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL sdnpsim_reset(
    void *pHandle);

  /* Set update callback and parameter */
  TMWDEFS_SCL_API void sdnpsim_setCallback(
    void *pHandle,
    SDNPSIM_CALLBACK_FUNC pUpdateCallback,
    void *pUpdateCallbackParam);
  
  /* Add Security Statistics points as required for Outstation */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL sdnpsim_addSecStats(
    void *pHandle);

  /* function: sdnpsim_enablePoint */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL sdnpsim_enablePoint(
    void *pPoint,
    TMWTYPES_BOOL enabled);

  /* function: sdnpsim_isPointEnabled */
  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_isPointEnabled(
    void *pPoint);

  /* function: sdnpsim_getPointNumber */
  TMWDEFS_SCL_API TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpsim_getPointNumber(
    void *pPoint);

  /* function: sdnpsim_getEventClass */
  TMWDEFS_SCL_API TMWDEFS_CLASS_MASK TMWDEFS_GLOBAL sdnpsim_getEventClass(
    void *pPoint);

  /* function: sdnpsim_setEventClass */
  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_setEventClass(
    void *pPoint, 
    TMWDEFS_CLASS_MASK classMask);

  /* function: sdnpsim_getCmdEventClass */
  TMWDEFS_SCL_API TMWDEFS_CLASS_MASK TMWDEFS_GLOBAL sdnpsim_getCmdEventClass(
    void *pPoint);

  /* function: sdnpsim_setCmdEventClass */
  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_setCmdEventClass(
    void *pPoint, 
    TMWDEFS_CLASS_MASK classMask);
    
  /* function: sdnpsim_inClass0 */
  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_inClass0(
     void *pPoint);

  /* function: sdnpsim_getFlags */
  TMWDEFS_SCL_API TMWTYPES_UCHAR TMWDEFS_GLOBAL sdnpsim_getFlags(
    void *pPoint);

  /* function: sdnpsim_getEventMode */
  TMWDEFS_EVENT_MODE sdnpsim_getEventMode(
    void *pPoint);

  /* function: sdnpsim_setEventMode */
  void sdnpsim_setEventMode(
    void *pPoint,
    TMWDEFS_EVENT_MODE mode);

  /* Binary Inputs */
  TMWDEFS_SCL_API void * TMWDEFS_GLOBAL sdnpsim_addBinaryInput(
    void *pHandle,
    TMWDEFS_CLASS_MASK classMask,
    TMWTYPES_UCHAR flags,
    TMWTYPES_BOOL value);

  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_deleteBinaryInput(
    void *pHandle);

  TMWDEFS_SCL_API TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpsim_binInQuantity(
    void *pHandle);
 
  TMWDEFS_SCL_API void * TMWDEFS_GLOBAL sdnpsim_binInGetPoint(
    void *pHandle,
    TMWTYPES_USHORT pointNum);

  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_binInSetFlags(
    void *pPoint, 
    TMWTYPES_UCHAR flags);

  void * TMWDEFS_GLOBAL sdnpsim_binInGetEnabledPoint(
    void *pHandle,
    TMWTYPES_USHORT pointNum);

  TMWTYPES_UCHAR TMWDEFS_GLOBAL sdnpsim_binInDefVariation(
    void *pPoint);

  TMWTYPES_UCHAR sdnpsim_binInEventDefVariation(
    void *pPoint,
    TMWDEFS_CLASS_MASK classMask);
   
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL sdnpsim_binInRead(
    void *pPoint, 
    TMWTYPES_BOOL *pValue, 
    TMWTYPES_UCHAR *pFlags);

  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_binInChanged(
    void *pPoint, 
    TMWTYPES_BOOL *pValue, 
    TMWTYPES_UCHAR *pFlags);

  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_binInWrite(
    void *pPoint, 
    TMWTYPES_BOOL value);

  /* Binary Outputs */
  TMWDEFS_SCL_API void * TMWDEFS_GLOBAL sdnpsim_addBinaryOutput(
    void *pHandle,
    TMWDEFS_CLASS_MASK classMask,
    TMWTYPES_UCHAR flags,
    TMWTYPES_BOOL value,
    SDNPSIM_CROB_CTRL controlMask);

  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_deleteBinaryOutput(
    void *pHandle);

  TMWDEFS_SCL_API TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpsim_binOutQuantity(
    void *pHandle);

  TMWDEFS_SCL_API void * TMWDEFS_GLOBAL sdnpsim_binOutGetPoint(
    void *pHandle,
    TMWTYPES_USHORT pointNum);

  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_binOutSetFlags(
    void *pPoint, 
    TMWTYPES_UCHAR flags);

  void * TMWDEFS_GLOBAL sdnpsim_binOutGetEnabledPoint(
    void *pHandle,
    TMWTYPES_USHORT pointNum);

  TMWTYPES_UCHAR sdnpsim_binOutEventDefVariation(
    void *pPoint,
    TMWDEFS_CLASS_MASK classMask);

  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_binOutChanged(
    void *pPoint, 
    TMWTYPES_BOOL *pValue, 
    TMWTYPES_UCHAR *pFlags);

  TMWDEFS_SCL_API void sdnpsim_binOutSetControlMask(
    void *pPoint,
    SDNPSIM_CROB_CTRL controlMask);

  TMWDEFS_SCL_API SDNPSIM_CROB_CTRL sdnpsim_binOutGetControlMask(
    void *pPoint);

  TMWTYPES_UCHAR TMWDEFS_GLOBAL sdnpsim_binOutDefVariation(
    void *pPoint);

  TMWDEFS_SCL_API void TMWDEFS_GLOBAL sdnpsim_binOutRead(
    void *pPoint, 
    TMWTYPES_BOOL *pValue, 
    TMWTYPES_UCHAR *pFlags);

  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_binOutWrite(
    void *pPoint, 
    TMWTYPES_BOOL value);

  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_binOutSetValue(
    void *pPoint,
    TMWTYPES_BOOL value);

  DNPDEFS_CROB_ST TMWDEFS_GLOBAL sdnpsim_binOutSelect(
    void *pPoint,
    TMWTYPES_UCHAR controlCode,
    TMWTYPES_UCHAR count,
    TMWTYPES_ULONG onTime,
    TMWTYPES_ULONG offTime);

  DNPDEFS_CROB_ST TMWDEFS_GLOBAL sdnpsim_binOutOperate(
    void *pPoint,
    TMWTYPES_UCHAR controlCode,
    TMWTYPES_UCHAR count,
    TMWTYPES_ULONG onTime,
    TMWTYPES_ULONG offTime);

  DNPDEFS_CROB_ST sdnpsim_binOutSelPatternMask(
    void *pHandle,
    TMWTYPES_UCHAR control, 
    TMWTYPES_UCHAR count, 
    TMWTYPES_ULONG activationPeriod,
    TMWTYPES_ULONG deactivationPeriod, 
    TMWTYPES_USHORT firstPointNumber, 
    TMWTYPES_USHORT lastPointNumber, 
    TMWTYPES_UCHAR *pMask);

  DNPDEFS_CROB_ST sdnpsim_binOutOpPatternMask(
    void *pHandle,
    TMWTYPES_UCHAR control, 
    TMWTYPES_UCHAR count, 
    TMWTYPES_ULONG activationPeriod,
    TMWTYPES_ULONG deactivationPeriod, 
    TMWTYPES_USHORT firstPointNumber, 
    TMWTYPES_USHORT lastPointNumber, 
    TMWTYPES_UCHAR *pMask);

  /* Counters */
  TMWDEFS_SCL_API void * TMWDEFS_GLOBAL sdnpsim_addBinaryCounter(
    void *pHandle,
    TMWDEFS_CLASS_MASK binClassMask,
    TMWDEFS_CLASS_MASK frznClassMask,
    TMWTYPES_UCHAR flags,
    TMWTYPES_ULONG value);

  TMWDEFS_SCL_API void * TMWDEFS_GLOBAL sdnpsim_addFrozenCounter(
    void *pHandle,
    TMWDEFS_CLASS_MASK binClassMask,
    TMWDEFS_CLASS_MASK frznClassMask,
    TMWTYPES_UCHAR flags,
    TMWTYPES_ULONG value);

  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_deleteBinaryCounter(
    void *pHandle);

  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_deleteFrozenCounter(
    void *pHandle);

  TMWDEFS_SCL_API TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpsim_binCntrQuantity(
    void *pHandle);

  TMWDEFS_SCL_API void * TMWDEFS_GLOBAL sdnpsim_binaryCounterGetPoint(
    void *pHandle,
    TMWTYPES_USHORT pointNum);

  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_binCntrSetFlags(
    void *pPoint, 
    TMWTYPES_UCHAR flags);

  void * TMWDEFS_GLOBAL sdnpsim_binCntrGetEnabledPoint(
    void *pHandle,
    TMWTYPES_USHORT pointNum);

  TMWDEFS_SCL_API void TMWDEFS_GLOBAL sdnpsim_binCntrRead(
    void *pPoint, 
    TMWTYPES_ULONG *pValue, 
    TMWTYPES_UCHAR *pFlags);

  TMWTYPES_UCHAR sdnpsim_binCntrDefVariation(
    void *pPoint);

  TMWTYPES_UCHAR sdnpsim_binCntrEventDefVariation(
    void *pPoint,
    TMWDEFS_CLASS_MASK classMask);

  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_binCntrChanged(
    void *pPoint, 
    TMWTYPES_ULONG *pValue, 
    TMWTYPES_UCHAR *pFlags);

  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_binCntrFreeze(
    void *pPoint,
    TMWTYPES_BOOL clearAfterFreeze);

  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_binCntrFreezeAtTime(
    void *pPoint,
    DNPDATA_FREEZE_TIME_DATE_FIELD timeDateEnum,
    TMWDTIME *pFreezeTime,
    TMWTYPES_ULONG freezeInterval);

  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_binCntrWrite(
    void *pPoint, 
    TMWTYPES_ULONG value);

  /* Frozen Counters */
  TMWDEFS_SCL_API void * TMWDEFS_GLOBAL sdnpsim_frozenCounterGetPoint(
    void *pHandle,
    TMWTYPES_USHORT pointNum);

  TMWDEFS_SCL_API TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpsim_frznCntrQuantity(
    void *pHandle);

  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_frznCntrSetFlags(
    void *pPoint, 
    TMWTYPES_UCHAR flags);

  void * TMWDEFS_GLOBAL sdnpsim_frznCntrGetEnabledPoint(
    void *pHandle,
    TMWTYPES_USHORT pointNum);

  TMWDEFS_SCL_API TMWDEFS_CLASS_MASK TMWDEFS_GLOBAL sdnpsim_frznCntrGetEventClass(
    void *pPoint);

  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_frznCntrSetEventClass(
    void *pPoint,
    TMWDEFS_CLASS_MASK classMask);

  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_frznCntrInClass0(
    void *pPoint);

  /* function: sdnpsim_frznCntrDefVariation */
  TMWTYPES_UCHAR sdnpsim_frznCntrDefVariation(
    void *pPoint);

  /* function: sdnpsim_frznCntrEventDefVariation */
  TMWTYPES_UCHAR sdnpsim_frznCntrEventDefVariation(
    void *pPoint,
    TMWDEFS_CLASS_MASK classMask);

  TMWDEFS_SCL_API void TMWDEFS_GLOBAL sdnpsim_frznCntrRead(
    void *pPoint, 
    TMWTYPES_ULONG *pValue, 
    TMWTYPES_UCHAR *pFlags, 
    TMWDTIME *pTimeOfFreeze);

  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_frznCntrChanged(
    void *pPoint, 
    TMWTYPES_ULONG *pValue, 
    TMWTYPES_UCHAR *pFlags);

  /* Analog Inputs */
  TMWDEFS_SCL_API void * TMWDEFS_GLOBAL sdnpsim_addAnalogInput(
    void *pHandle,
    TMWDEFS_CLASS_MASK classMask,
    TMWTYPES_UCHAR flags,
    TMWSIM_DATA_TYPE value,
    TMWSIM_DATA_TYPE deadband);

  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_deleteAnalogInput(
    void *pHandle);

  TMWDEFS_SCL_API TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpsim_anlgInQuantity(
    void *pHandle);

  TMWDEFS_SCL_API void *TMWDEFS_GLOBAL sdnpsim_anlgInGetPoint(
    void *pHandle,
    TMWTYPES_USHORT pointNum);
  
  /* function: sdnpsim_anlgInDefVariation */
  TMWTYPES_UCHAR sdnpsim_anlgInDefVariation(
    void *pPoint);

  /* function: sdnpsim_anlgInDbandDefVar */
  TMWTYPES_UCHAR sdnpsim_anlgInDbandDefVar(
    void *pPoint);

  /* function: sdnpsim_anlgInEventDefVariation */
  TMWTYPES_UCHAR sdnpsim_anlgInEventDefVariation(
    void *pPoint,
    TMWDEFS_CLASS_MASK classMask);
 
  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_anlgInSetFlags(
    void *pPoint, 
    TMWTYPES_UCHAR flags);

  void *TMWDEFS_GLOBAL sdnpsim_anlgInGetEnabledPoint(
    void *pHandle,
    TMWTYPES_USHORT pointNum);

  TMWDEFS_SCL_API void TMWDEFS_GLOBAL sdnpsim_anlgInRead(
    void *pPoint, 
    TMWTYPES_ANALOG_VALUE *pValue, 
    TMWTYPES_UCHAR *pFlags);

  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_anlgInChanged(
    void *pPoint, 
    TMWTYPES_ANALOG_VALUE *pValue, 
    TMWTYPES_UCHAR *pFlags);

  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_anlgInFreeze(
    void *pPoint,
    TMWTYPES_BOOL clearAfterFreeze);

  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_anlgInFreezeAtTime(
    void *pPoint,
    DNPDATA_FREEZE_TIME_DATE_FIELD timeDateEnum,
    TMWDTIME *pFreezeTime,
    TMWTYPES_ULONG freezeInterval);

  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_anlgInWrite(
    void *pPoint, 
    TMWSIM_DATA_TYPE value);

  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_anlgInGetDBand(
    void *pPoint,
    TMWTYPES_ANALOG_VALUE *pDeadbandValue);

  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_anlgInSetDBand(
    void *pPoint,
    TMWTYPES_ANALOG_VALUE *pDeadbandValue);

  /* Frozen Analog Inputs */
  /* function: sdnpsim_addfrznAnlgInput */
  void * TMWDEFS_GLOBAL sdnpsim_addfrznAnlgInput(
    void *pHandle,
    TMWDEFS_CLASS_MASK classMask,
    TMWTYPES_UCHAR flags,
    TMWSIM_DATA_TYPE value);

  /* function: sdnpsim_deleteFrznAnlgInput */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_deleteFrznAnlgInput(
    void *pHandle);

  /* function: sdnpsim_frznAnlgInQuantity */
  TMWDEFS_SCL_API TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpsim_frznAnlgInQuantity(
    void *pHandle);

  /* function: sdnpsim_frznAnlgInGetPoint */
  TMWDEFS_SCL_API void * TMWDEFS_GLOBAL sdnpsim_frznAnlgInGetPoint(
    void *pHandle,
    TMWTYPES_USHORT pointNum);

  /* function: sdnpsim_frznAnlgInDefVariation */
  TMWTYPES_UCHAR sdnpsim_frznAnlgInDefVariation(
    void *pPoint);

  TMWTYPES_UCHAR sdnpsim_frznAnlgInEventDefVariation(
    void *pPoint,
    TMWDEFS_CLASS_MASK classMask);

/* function: sdnpsim_frznAnlgInSetFlags */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_frznAnlgInSetFlags(
    void *pPoint,
    TMWTYPES_UCHAR flags);

  /* function: sdnpsim_frznAnlgInGetEnabledPoint */
  void * TMWDEFS_GLOBAL sdnpsim_frznAnlgInGetEnabledPoint(
    void *pHandle,
    TMWTYPES_USHORT pointNum);

  /* function: sdnpsim_frznAnlgInRead */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL sdnpsim_frznAnlgInRead(
    void *pPoint, 
    TMWTYPES_ANALOG_VALUE *pValue, 
    TMWTYPES_UCHAR *pFlags, 
    TMWDTIME *pTimeOfFreeze);

  /* function: sdnpsim_frznAnlgInChanged */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_frznAnlgInChanged(
    void *pPoint, 
    TMWTYPES_ANALOG_VALUE *pValue, 
    TMWTYPES_UCHAR *pFlags);

  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_frznAnlgInWrite(
    void *pPoint,
    TMWSIM_DATA_TYPE value);

  /* function: sdnpsim_frznAnlgInDefEventVariation */
  TMWTYPES_UCHAR sdnpsim_frznAnlgInDefEventVariation(
    void *pPoint,
    TMWDEFS_CLASS_MASK classMask);


  /* Analog Outputs */
  TMWDEFS_SCL_API void *TMWDEFS_GLOBAL sdnpsim_addAnalogOutput(
    void *pHandle,
    TMWDEFS_CLASS_MASK classMask,
    TMWTYPES_UCHAR flags,
    TMWSIM_DATA_TYPE value);

  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_deleteAnalogOutput(
    void *pHandle);

  TMWDEFS_SCL_API TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpsim_anlgOutQuantity(
    void *pHandle);

  TMWDEFS_SCL_API void * TMWDEFS_GLOBAL sdnpsim_anlgOutGetPoint(
    void *pHandle,
    TMWTYPES_USHORT pointNum);

  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_anlgOutSetFlags(
    void *pPoint, 
    TMWTYPES_UCHAR flags);

  void * TMWDEFS_GLOBAL sdnpsim_anlgOutGetEnabledPoint(
    void *pHandle,
    TMWTYPES_USHORT pointNum);

  /* function: sdnpsim_anlgOutDefVariation */
  TMWTYPES_UCHAR sdnpsim_anlgOutDefVariation(
    void *pPoint);
 
  /* function: sdnpsim_anlgOutEventDefVariation */
  TMWTYPES_UCHAR sdnpsim_anlgOutEventDefVariation(
    void *pPoint,
    TMWDEFS_CLASS_MASK classMask); 

  /* function: sdnpsim_anlgOutChanged */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_anlgOutChanged(
    void *pPoint,
    TMWTYPES_ANALOG_VALUE *pValue,
    TMWTYPES_UCHAR *pFlags);

  TMWDEFS_SCL_API void TMWDEFS_GLOBAL sdnpsim_anlgOutRead(
    void *pPoint, 
    TMWTYPES_ANALOG_VALUE *pValue, 
    TMWTYPES_UCHAR *pFlags);

  DNPDEFS_CROB_ST TMWDEFS_GLOBAL sdnpsim_anlgOutSelect(
    void *pPoint, 
    TMWTYPES_ANALOG_VALUE *pValue);

  DNPDEFS_CROB_ST TMWDEFS_GLOBAL sdnpsim_anlgOutOperate(
    void *pPoint, 
    TMWTYPES_ANALOG_VALUE *pValue);

  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_anlgOutWrite(
    void *pPoint, 
    TMWTYPES_ANALOG_VALUE *pValue);

  /* Double Bit Inputs */
  TMWDEFS_SCL_API void * TMWDEFS_GLOBAL sdnpsim_addDoubleInput(
    void *pHandle,
    TMWDEFS_CLASS_MASK classMask,
    TMWTYPES_UCHAR flagsAndValue);

  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_deleteDoubleInput(
    void *pHandle);

  TMWDEFS_SCL_API TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpsim_dblInQuantity(
    void *pHandle);
 
  TMWDEFS_SCL_API void * TMWDEFS_GLOBAL sdnpsim_dblInGetPoint(
    void *pHandle,
    TMWTYPES_USHORT pointNum);

  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_dblInSetFlags(
    void *pPoint, 
    TMWTYPES_UCHAR flags);

  void * TMWDEFS_GLOBAL sdnpsim_dblInGetEnabledPoint(
    void *pHandle,
    TMWTYPES_USHORT pointNum);

  TMWTYPES_UCHAR TMWDEFS_GLOBAL sdnpsim_dblInDefVariation(
    void *pPoint);

  TMWTYPES_UCHAR sdnpsim_dblInEventDefVariation(
    void *pPoint,
    TMWDEFS_CLASS_MASK classMask);

  TMWDEFS_SCL_API void TMWDEFS_GLOBAL sdnpsim_dblInRead(
    void *pPoint, 
    TMWTYPES_UCHAR *pFlags);

  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_dblInChanged(
    void *pPoint, 
    TMWTYPES_UCHAR *pFlags);

  /* Time and date - indexed absolute time and long interval */
  /* classMask - can only be TMWDEFS_CLASS_MASK_NONE or TMWDEFS_CLASS_MASK_NOTCLASS0 */
  TMWDEFS_SCL_API void * TMWDEFS_GLOBAL sdnpsim_addIndexedTime(
    void *pHandle,
    TMWDEFS_CLASS_MASK classMask,
    TMWDTIME timeValue,
    TMWTYPES_ULONG intervalCount,
    TMWTYPES_BYTE  intervalUnit);

  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_deleteIndexedTime(
    void *pHandle);

  TMWDEFS_SCL_API TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpsim_indexedTimeQuantity(
    void *pHandle);

  void * TMWDEFS_GLOBAL sdnpsim_indexedTimeGetPoint(
    void *pHandle,
    TMWTYPES_USHORT pointNum);

  void * TMWDEFS_GLOBAL sdnpsim_indexedTimeGetEnabledPoint(
    void *pHandle,
    TMWTYPES_USHORT pointNum);

  TMWDEFS_SCL_API void TMWDEFS_GLOBAL sdnpsim_indexedTimeRead(
    void *pPoint,
    TMWDTIME *pTimeStamp,
    TMWTYPES_ULONG *pIntervalCount,
    TMWTYPES_BYTE  *pIntervalUnit);

  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_startTimeWrite(
    void *pPoint,
    TMWDTIME timeStamp);

  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_intervalCountWrite(
    void *pPoint,
    TMWTYPES_ULONG intervalCount);

  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_intervalUnitWrite(
    void *pPoint,
    TMWTYPES_BYTE  intervalUnit);

  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_indexedTimeWrite(
    void *pPoint,
    TMWDTIME timeValue,
    TMWTYPES_ULONG intervalCount,
    TMWTYPES_BYTE  intervalUnit);

  /* Strings */
#if SDNPDATA_SUPPORT_OBJ110
  TMWDEFS_SCL_API void * TMWDEFS_GLOBAL sdnpsim_addString(
    void *pHandle,
    TMWDEFS_CLASS_MASK classMask,
    TMWTYPES_UCHAR *pValue,
    TMWTYPES_UCHAR length);

  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_deleteString(
    void *pHandle);

  TMWDEFS_SCL_API TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpsim_strQuantity(
    void *pHandle);

  TMWDEFS_SCL_API void * TMWDEFS_GLOBAL sdnpsim_strGetPoint(
    void *pHandle, 
    TMWTYPES_USHORT pointNum);

  void * TMWDEFS_GLOBAL sdnpsim_strGetEnabledPoint(
    void *pHandle, 
    TMWTYPES_USHORT pointNum);

  TMWDEFS_SCL_API void TMWDEFS_GLOBAL sdnpsim_strRead(
    void *pPoint, 
    TMWTYPES_UCHAR maxLength, 
    TMWTYPES_UCHAR *pBuf, 
    TMWTYPES_UCHAR *pBufLength);

  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_strWrite(
    void *pPoint, 
    TMWTYPES_UCHAR *pBuf, 
    TMWTYPES_UCHAR bufLength);

  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_strChanged(
    void *pPoint, 
    TMWTYPES_UCHAR maxLength, 
    TMWTYPES_UCHAR *pBuf, 
    TMWTYPES_UCHAR *pBufLength);
#endif

  /* Virtual Terminals */
  TMWDEFS_SCL_API void *TMWDEFS_GLOBAL sdnpsim_addVirtualTerminal(
    void *pHandle,
    TMWDEFS_CLASS_MASK classMask,
    TMWTYPES_UCHAR *pValue,
    TMWTYPES_UCHAR length);

  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_deleteVirtualTerminal(
    void *pHandle);

  TMWDEFS_SCL_API TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpsim_vtermQuantity(
    void *pHandle);
  
  TMWDEFS_SCL_API void * TMWDEFS_GLOBAL sdnpsim_vtermGetPoint(
    void *pHandle, 
    TMWTYPES_USHORT pointNum);

  void * TMWDEFS_GLOBAL sdnpsim_vtermGetEnabledPoint(
    void *pHandle, 
    TMWTYPES_USHORT pointNum);

  TMWDEFS_SCL_API void TMWDEFS_GLOBAL sdnpsim_vtermRead(
    void *pPoint, 
    TMWTYPES_UCHAR maxLength, 
    TMWTYPES_UCHAR *pBuf, 
    TMWTYPES_UCHAR *pBufLength);

  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_vtermWrite(
    void *pPoint, 
    TMWTYPES_UCHAR *pBuf, 
    TMWTYPES_UCHAR bufLength);

  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_vtermChanged(
    void *pPoint, 
    TMWTYPES_UCHAR maxLength, 
    TMWTYPES_UCHAR *pBuf, 
    TMWTYPES_UCHAR *pBufLength);

 /* Extended Strings */
  TMWDEFS_SCL_API void * TMWDEFS_GLOBAL sdnpsim_addExtString(
    void *pHandle,
    TMWDEFS_CLASS_MASK classMask,
    TMWTYPES_UCHAR flags,
    TMWTYPES_UCHAR *pValue,
    TMWTYPES_USHORT length);

  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_deleteExtString(
    void *pHandle);

  TMWDEFS_SCL_API TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpsim_extStrQuantity(
    void *pHandle);

  TMWDEFS_SCL_API void * TMWDEFS_GLOBAL sdnpsim_extStrGetPoint(
    void *pHandle, 
    TMWTYPES_USHORT pointNum);

  TMWTYPES_UCHAR sdnpsim_extStrDefVariation(
    void *pPoint);

  TMWTYPES_UCHAR sdnpsim_extStrEventDefVariation(
    void *pPoint,
    TMWDEFS_CLASS_MASK classMask);

  void * TMWDEFS_GLOBAL sdnpsim_extStrGetEnabledPoint(
    void *pHandle, 
    TMWTYPES_USHORT pointNum);

  TMWDEFS_SCL_API void TMWDEFS_GLOBAL sdnpsim_extStrRead(
    void *pPoint, 
    TMWTYPES_USHORT maxLength, 
    TMWTYPES_UCHAR *pBuf, 
    TMWTYPES_USHORT *pBufLength,
    TMWTYPES_UCHAR *pFlags);

  TMWTYPES_UCHAR * TMWDEFS_GLOBAL sdnpsim_extStrGetPtr(
    void *pPoint,
    TMWTYPES_USHORT *pBufLength,
    TMWTYPES_UCHAR *pFlags);

  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_extStrWrite(
    void *pPoint, 
    TMWTYPES_UCHAR *pBuf, 
    TMWTYPES_USHORT bufLength);

  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_extStrChanged(
    void *pPoint, 
    TMWTYPES_USHORT maxLength, 
    TMWTYPES_UCHAR *pBuf, 
    TMWTYPES_USHORT *pBufLength,
    TMWTYPES_UCHAR *pFlags);

  /* routine: sdnpsim_showData */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL sdnpsim_showData(
    TMWSESN *pSession);

  /* routine: sdnpsim_authConfigUser
   * For sample test application purposes only 
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_authConfigUser(
    void             *pHandle,
    TMWTYPES_CHAR    *pUserName,
    TMWTYPES_USHORT   userNameLength,
    TMWTYPES_USHORT   userNumber,
    TMWTYPES_USHORT   userRole,
    TMWTYPES_USHORT   userRoleExpiryInterval);

  /* function: sdnpsim_authAssignClass */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_authAssignClass(
    void *pHandle,
    TMWDEFS_CLASS_MASK classMask);
   
  /* function: sdnpsim_authErrorEventClass */
  TMWDEFS_CLASS_MASK TMWDEFS_GLOBAL sdnpsim_authErrorEventClass(
    void *pHandle); 

  /* function: sdnpsim_authGetOSName */ 
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_authGetOSName(
    void            *pHandle, 
    TMWTYPES_CHAR   *pOSName,
    TMWTYPES_USHORT *pOSNameLength);

  /* function: sdnpsim_authGetUserName */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_authGetUserName(
    void            *pHandle, 
    TMWTYPES_USHORT  userNumber,
    TMWTYPES_CHAR   *pUserName,
    TMWTYPES_USHORT *pUserNameLength);
 
  /* function: sdnpsim_authUserStatusChange */ 
  TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpsim_authUserStatusChg(
    void            *pHandle,
    TMWTYPES_CHAR   *pUserName,
    TMWTYPES_USHORT  userNameLength,
    TMWTYPES_UCHAR   keyChangeMethod, 
    TMWTYPES_UCHAR   operation, 
    TMWTYPES_USHORT  userRole, 
    TMWTYPES_USHORT  userRoleExpiryInterval);

  /* function: sdnpsim_authUpdateKeyChgReqRcvd */ 
  TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpsim_authUpdateKeyChgReqRcvd( 
    void            *pHandle,
    TMWTYPES_CHAR   *pUserName,
    TMWTYPES_USHORT  userNameLength);

  /* function: sdnpsim_addAuthSecStat */
  void * TMWDEFS_GLOBAL sdnpsim_addAuthSecStat(
    void *pHandle, 
    TMWTYPES_USHORT index,
    TMWDEFS_CLASS_MASK classMask,
    TMWTYPES_UCHAR flags,
    TMWTYPES_ULONG value);

  /* function: sdnpsim_deleteAuthSecStat */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_deleteAuthSecStat(
    void *pHandle,
    TMWTYPES_USHORT index);

  /* function: sdnpsim_authSecStatQuantity */
  TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpsim_authSecStatQuantity(
    void *pHandle);
    
  /* function: sdnpsim_authSecStatGetPoint */
  void * TMWDEFS_GLOBAL sdnpsim_authSecStatGetPoint(
    void *pHandle,
    TMWTYPES_USHORT pointNum);

  /* function: sdnpsim_authSecStatGetValue */
  TMWTYPES_ULONG TMWDEFS_GLOBAL sdnpsim_authSecStatGetValue(
    void            *pHandle,
    TMWTYPES_USHORT  index);
      
  /* function: sdnpsim_authSecStatEventDefVariation */
  TMWTYPES_UCHAR sdnpsim_authSecStatEventDefVariation(
    void *pPoint, 
    TMWDEFS_CLASS_MASK classMask);

  TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpsim_getAuthSecStatThreshold(
    void            *pHandle,  
    TMWTYPES_USHORT  index);

  /* function: sdnpsim_authSecStatRead */
  TMWDEFS_SCL_API void sdnpsim_authSecStatRead(
    void *pPoint, 
    TMWTYPES_ULONG *pValue, 
    TMWTYPES_UCHAR *pFlags);

  /* function: sdnpsim_authSecStatSet */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_authSecStatSet(
    void            *pPoint,
    TMWTYPES_ULONG   value);
  
/* SPM These should be removed when tmwcrypto interface is used */
  /* function: sdnpsim_authDecryptKeyWrapData */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_authDecryptKeyWrapData(
    void            *pHandle,
    TMWTYPES_USHORT  userNumber,
    TMWTYPES_UCHAR   algorithm,
    TMWTYPES_UCHAR  *pEncryptedData, 
    TMWTYPES_USHORT  encryptedValueLength, 
    TMWTYPES_UCHAR  *pPlainValue,
    TMWTYPES_USHORT *pPlainLength);

  /* function: sdnpsim_authHMACSupport */
  TMWTYPES_CHAR sdnpsim_authHMACSupport(
    TMWTYPES_UCHAR HMACAlgorithm);

  /* function: sdnpsim_authHMACValue */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_authHMACValue(
    TMWTYPES_UCHAR    algorithm,
    DNPDATA_AUTH_KEY *pKey,
    TMWTYPES_UCHAR   *pData,
    TMWTYPES_ULONG    dataLength,
    TMWTYPES_UCHAR   *pHMACValue,
    TMWTYPES_USHORT  *pHMACValueLength);

  /* function: sdnpsim_authRandomChallengeData */ 
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_authRandomChallengeData(
    TMWTYPES_USHORT  minLength,
    TMWTYPES_UCHAR  *pBuf,
    TMWTYPES_USHORT *pLength);


#if SDNPCNFG_USER_MANAGED_EVENTS
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_umEventAdd(
    void *pHandle,
    TMWTYPES_UCHAR group,
    TMWTYPES_USHORT point,
    TMWDEFS_CLASS_MASK classMask,
    TMWTYPES_UCHAR defaultVariation,
    TMWTYPES_UCHAR flags,
    SDNPDATA_ADD_EVENT_VALUE *pValue,
    TMWDTIME *pTimeStamp);

  TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpsim_umEventNotSentCount(
    void *pHandle,
    TMWTYPES_UCHAR group,
    TMWDEFS_CLASS_MASK classMask);

  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_umEventGet(
    void *pHandle,
    TMWTYPES_UCHAR group,
    TMWDEFS_CLASS_MASK classMask,
    TMWTYPES_BOOL firstEvent,
    SDNPDATA_GET_EVENT *pEvent);

  void TMWDEFS_GLOBAL sdnpsim_umEventSent(
    void *pHandle,
    TMWTYPES_UCHAR group,
    TMWTYPES_USHORT point);

  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_umEventNotSent(
    void *pHandle,
    TMWTYPES_UCHAR group);

  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_umEventRemove(
    void *pHandle,
    TMWTYPES_UCHAR group);
#endif

#if SDNPDATA_SUPPORT_OBJ0
  TMWDEFS_SCL_API void * TMWDEFS_GLOBAL sdnpsim_addDeviceAttribute(
    void *pHandle, 
    TMWTYPES_USHORT pointNum,
    TMWTYPES_UCHAR variation,
    TMWTYPES_UCHAR property,
    DNPDATA_ATTRIBUTE_VALUE *pData);

  /* function: sdnpsim_deleteDeviceAttr */ 
  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_deleteDeviceAttr(
    void *pHandle,
    TMWTYPES_USHORT point,
    TMWTYPES_UCHAR variation);
 
  TMWDEFS_SCL_API TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpsim_deviceAttrQuantity(
    void *pHandle);

  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_deviceAttrNext(
    void *pPoint,
    TMWTYPES_UCHAR variation,
    TMWTYPES_UCHAR *pNextVariation,
    TMWTYPES_UCHAR *pProperty);

  TMWDEFS_SCL_API void * TMWDEFS_GLOBAL sdnpsim_deviceAttrGetPoint( 
    void *pHandle,
    TMWTYPES_USHORT point);

  TMWDEFS_SCL_API void * TMWDEFS_GLOBAL sdnpsim_deviceAttrGetVar( 
    void *pPoint,
    TMWTYPES_UCHAR variation);

  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_deviceAttrRead(
    void *pAttribute,
    DNPDATA_ATTRIBUTE_VALUE *pData);

  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_deviceAttrWrite(
    void *pAttribute,
    DNPDATA_ATTRIBUTE_VALUE *pData);

  /* Not called from SCL */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL sdnpsim_deviceAttrSetProperty(
    void *pAttribute,
    TMWTYPES_UCHAR property);

  /* Not called from SCL */
  TMWDEFS_SCL_API TMWTYPES_UCHAR TMWDEFS_GLOBAL sdnpsim_deviceAttrGetProperty(
    void *pAttribute);
#endif

#if SDNPDATA_SUPPORT_OBJ85
  TMWDEFS_SCL_API TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpsim_datasetProtoQuantity(
    void *pHandle);

  TMWDEFS_SCL_API TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpsim_datasetProtoMasterQty(
    void *pHandle);

  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_CALLBACK sdnpsim_datasetProtoGetID(
    void *pHandle,
    TMWTYPES_UCHAR *pUUID,
    TMWTYPES_USHORT *pPointNum);

  TMWDEFS_SCL_API void * TMWDEFS_GLOBAL sdnpsim_datasetProtoGetPoint(
    void *pHandle,
    TMWTYPES_USHORT pointNum);

  TMWDEFS_SCL_API DNPDATA_DATASET_DESCR_ELEM * TMWDEFS_GLOBAL sdnpsim_datasetProtoRead(
    void *pPoint,
    TMWTYPES_UCHAR *pNumberElems,
    TMWTYPES_UCHAR *pUUID);

  TMWDEFS_SCL_API void * TMWDEFS_GLOBAL sdnpsim_datasetProtoCreatePoint(
    void *pPoint,
    TMWTYPES_USHORT pointNum,
    TMWTYPES_UCHAR *pUUID,
    TMWTYPES_BOOL fromMaster);

  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_datasetProtoDeletePoint(
    void *pHandle,
    TMWTYPES_USHORT pointNum);

  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_datasetProtoWrite(
    void *pPoint,
    TMWTYPES_UCHAR index,
    DNPDATA_DATASET_DESCR_ELEM *pElem); 
#endif

#if SDNPDATA_SUPPORT_OBJ86
  TMWDEFS_SCL_API TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpsim_datasetDescrQuantity(
    void *pHandle);

  TMWDEFS_SCL_API void * TMWDEFS_GLOBAL sdnpsim_datasetDescrCreatePoint(
    void *pHandle,
    TMWTYPES_USHORT pointNum,
    TMWTYPES_BOOL fromMaster);

  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_datasetDescrDeletePoint(
    void *pHandle,
    TMWTYPES_USHORT pointNum);

  TMWDEFS_SCL_API void * TMWDEFS_GLOBAL sdnpsim_datasetDescrGetPoint(
    void *pHandle,
    TMWTYPES_USHORT pointNum);

  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_datasetDescrGetName(
    void *pHandle,
    TMWTYPES_USHORT pointNumber,
    char *buf);

  TMWDEFS_SCL_API DNPDATA_DATASET_DESCR_ELEM * TMWDEFS_GLOBAL sdnpsim_datasetDescrReadCont(
    void *pPoint,
    TMWTYPES_UCHAR *pNumberElems);

  TMWDEFS_SCL_API DNPDATA_DATASET_DESCR_INDEX * TMWDEFS_GLOBAL sdnpsim_datasetDescrReadIndex(
    void *pPoint,
    TMWTYPES_UCHAR *pNumberElems);

  TMWDEFS_SCL_API void TMWDEFS_GLOBAL sdnpsim_datasetDescrReadChars(
    void *pPoint,
    TMWTYPES_UCHAR *pValue);

  TMWDEFS_SCL_API  void TMWDEFS_GLOBAL sdnpsim_datasetDescrSetChars(
    void *pPoint,
    TMWTYPES_UCHAR value);

  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_datasetDescrWriteCont(
    void *pPoint,
    TMWTYPES_UCHAR index,
    DNPDATA_DATASET_DESCR_ELEM *pElem); 

  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_datasetDescrWriteIndex(
    void *pPoint,
    TMWTYPES_UCHAR index,
    DNPDATA_DATASET_DESCR_INDEX *pElem); 

  TMWDEFS_SCL_API TMWDEFS_CLASS_MASK TMWDEFS_GLOBAL sdnpsim_datasetDescrEventClass(
    void *pPoint);

  /* Get the dataset descriptor element for this index after the descriptor is expanded
   * out to include the elements in any contained prototypes. This gives the descriptor 
   * or prototype element that matches this index in the datatet value array.
   */
  DNPDATA_DATASET_DESCR_ELEM *sdnpsim_datasetDescrGetExpIndex(
    void *pPoint, 
    TMWTYPES_UCHAR index);

  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_datasetDescrAssignClass(
    void *pPoint,
    TMWDEFS_CLASS_MASK classMask);
#endif

#if SDNPDATA_SUPPORT_OBJ87
  TMWDEFS_SCL_API TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpsim_datasetQuantity(
    void *pHandle);

  TMWDEFS_SCL_API TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpsim_datasetMasterQty(
    void *pHandle);

  TMWDEFS_SCL_API void * TMWDEFS_GLOBAL sdnpsim_datasetCreatePoint(
    void *pHandle,
    TMWTYPES_USHORT pointNum,
    TMWDTIME *pTimeStamp);

  TMWDEFS_SCL_API void * TMWDEFS_CALLBACK sdnpsim_datasetGetPoint(
    void *pHandle,
    TMWTYPES_USHORT pointNum);

  TMWDEFS_SCL_API DNPDATA_DATASET_VALUE * TMWDEFS_GLOBAL sdnpsim_datasetRead(
    void *pPoint,
    TMWTYPES_UCHAR *pNumberElems,
    TMWDTIME *pTimeStamp);

  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_datasetWrite(
    void *pPoint,
    TMWTYPES_UCHAR index,
    DNPDATA_DATASET_VALUE *pElem); 

  DNPDEFS_DATASET_ST TMWDEFS_GLOBAL sdnpsim_datasetSelect(
    void *pPoint,
    DNPDATA_DATASET_CTRL_VALUE *pCtrlValues,
    TMWTYPES_UCHAR numberCtrlValues);

  DNPDEFS_DATASET_ST TMWDEFS_GLOBAL sdnpsim_datasetOperate(
    void *pPoint,
    DNPDATA_DATASET_CTRL_VALUE *pCtrlValues,
    TMWTYPES_UCHAR numberCtrlValues);

  /* function: sdnpsim_datasetTestingMode */
  void TMWDEFS_GLOBAL sdnpsim_datasetTestingMode(
    void *pPoint, 
    TMWTYPES_UCHAR mode);
#endif
 
#if SDNPDATA_SUPPORT_XML2
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_XmlGetDeviceConfig(
    void *pHandle, 
    SDNPDATA_XML_DEVICECONFIG *pConfig);
  
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_XmlGetSerialConfig(
    void *pHandle, 
    SDNPDATA_XML_SERIALCONFIG *pConfig);
   
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_XmlGetNetworkConfig(
    void *pHandle, 
    SDNPDATA_XML_NETWORKCONFIG *pConfig);
   
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_XmlGetOutstationConfig(
    void *pHandle, 
    SDNPDATA_XML_OUTSTATIONCONFIG *pConfig);
 
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_XmlGetOutstationPerform(
    void *pHandle, 
    SDNPDATA_XML_OUTSTATIONPERFORM *pConfig);
  
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_XmlGetFieldConfig(
    void *pHandle, 
    SDNPDATA_XML_FIELDCONFIG *pConfig);

  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_XmlGetSecurityConfig(
    void *pHandle, 
    SDNPDATA_XML_SECURITYCONFIG *pConfig);

  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_XmlGetBinOutGroupConfig(
    void *pHandle, 
    SDNPDATA_XML_BINOUTGROUPCONFIG *pConfig);

  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_XmlGetCntrGroupConfig(
    void *pHandle, 
    SDNPDATA_XML_COUNTERGROUPCONFIG *pConfig);

  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_XmlGetAnlgInGroupConfig(
    void *pHandle, 
    SDNPDATA_XML_ANLGINGROUPCONFIG *pConfig);

  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_XmlGetFileConfig(
    void *pHandle, 
    SDNPDATA_XML_FILECONFIG *pConfig);

  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_XmlGetPointListDefinition(
    void *pHandle, 
    SDNPDATA_XML_POINTLISTCONFIG *pConfig);

  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_XmlGetBinInPointConfig(
    void *pPoint, 
    SDNPDATA_XML_BININPOINTCONFIG *pConfig);

  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_XmlGetDblInPointConfig(
    void *pPoint, 
    SDNPDATA_XML_DBLINPOINTCONFIG *pConfig);

  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_XmlGetBinOutPointConfig(
    void *pPoint, 
    SDNPDATA_XML_BINOUTPOINTCONFIG *pConfig);

  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_XmlGetCntrPointConfig(
    void *pPoint, 
    SDNPDATA_XML_CNTRPOINTCONFIG *pConfig);

  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_XmlGetAnlgInPointConfig(
    void *pPoint, 
    SDNPDATA_XML_ANLGINPOINTCONFIG *pConfig);

  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_XmlGetAnlgOutPointConfig(
    void *pPoint, 
    SDNPDATA_XML_ANLGOUTPOINTCONFIG *pConfig);

  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_XmlGetStringPointConfig(
    void *pPoint, 
    SDNPDATA_XML_STRINGPOINTCONFIG *pConfig);

  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_XmlGetVtermPointConfig(
    void *pPoint, 
    SDNPDATA_XML_VTERMPOINTCONFIG *pConfig);

  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsim_XmlGetPerFileConfig(
    TMWTYPES_ULONG index, 
    SDNPDATA_XML_PERFILECONFIG *pConfig);
#endif /* SDNPDATA_SUPPORT_XML2 */

#ifdef __cplusplus
}
#endif
#endif /* TMWCNFG_USE_SIMULATED_DB */
#endif /* SDNPSIM_DEFINED */
