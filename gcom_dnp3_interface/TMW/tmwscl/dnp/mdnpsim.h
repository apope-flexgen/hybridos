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
 *  This file is an example of a simulated DNP master database interface.
 *  It should NOT be included in the final version of a DNP master device.
 */
#ifndef MDNPSIM_DEFINED
#define MDNPSIM_DEFINED

#include "tmwscl/utils/tmwcnfg.h"
#if TMWCNFG_USE_SIMULATED_DB

#include "tmwscl/utils/tmwdefs.h"
#include "tmwscl/utils/tmwsim.h"
#include "tmwscl/dnp/mdnpsesn.h"
#include "tmwscl/dnp/mdnpfsim.h"
#include "tmwscl/dnp/mdnpcnfg.h"

/* Define this to include support for generating an XML document based on
 * the current state of the simulated DNP3 master database. A proposal is 
 * being developed that would support the distribution of DNP3 configuration
 * information using XML. This proposal is still under development so
 * this code will probably change as the proposal is refined. The included
 * XML support allows the generation of an XML document which conforms to
 * the latest DNP3 configuration schema.
 */
#define MDNPSIM_SUPPORT_XML TMWDEFS_TRUE

/* Specify callback */
typedef void (*MDNPSIM_CALLBACK_FUNC)(
  void *pCallbackParam, 
  TMWSIM_EVENT_TYPE type,
  DNPDEFS_OBJ_GROUP_ID objectGroup, 
  TMWTYPES_USHORT pointNumber);


/* Data Sets */ 

/* Maximum number of Data Set descriptor elements per prototype or descriptor */
#define MDNPSIM_MAX_DESCR_ELEM DNPCNFG_MAX_DATASET_DESCR_ELEMS

/* Maximum number of Data Set elements per Data Set  
 * Because of contained prototypes, allow more data set elements 
 */
#define MDNPSIM_MAX_DATASET_ELEM (2*DNPCNFG_MAX_DATASET_DESCR_ELEMS)

/* Data Set Prototype structure */ 
typedef struct {
  /* must be first field */
  TMWSIM_POINT                      unused;

  TMWTYPES_USHORT                   protoId;  /* Point Index */
  TMWTYPES_BOOL                     fromSlave;
  TMWTYPES_UCHAR                    uuid[16];

  TMWTYPES_UCHAR                    numberElems;
  DNPDATA_DATASET_DESCR_ELEM        contents[MDNPSIM_MAX_DESCR_ELEM];
} MDNPSIM_DATASET_PROTO;

/* Data Set structure, this combines dataset descriptor and data set value information
 * together. They need to match 1 for 1 and to understand the data the descriptor is required.
 */
typedef struct {
  /* must be first field */
  TMWSIM_POINT                      unused;
  TMWTYPES_USHORT                   datasetId;  /* Point Index */

  /* Dataset timestamp */
  TMWDTIME                          timeStamp;
  TMWTYPES_BOOL                     fromSlave;
  TMWTYPES_UCHAR                    characteristics;
  TMWTYPES_UCHAR                    numberDescrElems;
  TMWTYPES_UCHAR                    numberIndexElems;
  TMWTYPES_UCHAR                    numberDataElems; 
  DNPDATA_DATASET_DESCR_ELEM        descrContents[MDNPSIM_MAX_DESCR_ELEM];

  /* Because descriptors can contain prototypes, there can be more descriptor 
   * indexes and data elements than descriptor elements 
   */
  DNPDATA_DATASET_DESCR_INDEX       descrIndex[MDNPSIM_MAX_DATASET_ELEM];
  DNPDATA_DATASET_VALUE             dataElem[MDNPSIM_MAX_DATASET_ELEM];

  /* If descriptor data type is OSTR or BSTR display data as hex */
  TMWTYPES_BOOL                     displayValueAsHex[MDNPSIM_MAX_DATASET_ELEM];

} MDNPSIM_DATASET_DESCR_DATA;


#define MDNPSIM_MAX_CHALLENGE_DATA_LEN 128
#define MDNPSIM_MAX_SYMKEY_LEN 512
#define MDNPSIM_MAX_ASYMKEY_LEN 2048
typedef struct {
  /* must be first field */
  TMWSIM_POINT     unused;

  TMWTYPES_USHORT  userNumber;
  TMWTYPES_ULONG   statusChangeSequenceNumber;

  TMWTYPES_USHORT  userRole;
  TMWTYPES_USHORT  userRoleExpiryInterval;  
  TMWTYPES_UCHAR   keyChangeMethod; 
  TMWTYPES_UCHAR   operation; 
  TMWTYPES_UCHAR   userName[128]; 
  TMWTYPES_USHORT  userNameLength;  
  TMWTYPES_UCHAR   OSChallengeData[MDNPSIM_MAX_CHALLENGE_DATA_LEN]; 
  TMWTYPES_USHORT  OSChallengeLength;  

  TMWTYPES_UCHAR   updateKey[MDNPSIM_MAX_SYMKEY_LEN]; 
  TMWTYPES_USHORT  updateKeyLength;  

  TMWTYPES_UCHAR   certData[MDNPSIM_MAX_ASYMKEY_LEN]; 
  TMWTYPES_USHORT  certDataLength;  

  TMWTYPES_UCHAR   updateKeyData[MDNPSIM_MAX_ASYMKEY_LEN]; 
  TMWTYPES_USHORT  updateKeyDataLength;  

} MDNPSIM_AUTHUSER;
  
 
typedef struct MDNPSimDatabaseStruct {
  TMWSIM_TABLE_HEAD binaryInputs;
  TMWSIM_TABLE_HEAD doubleInputs;
  TMWSIM_TABLE_HEAD binaryOutputs;
  TMWSIM_TABLE_HEAD binaryCounters;
  TMWSIM_TABLE_HEAD frozenCounters;
  TMWSIM_TABLE_HEAD analogInputs;
  TMWSIM_TABLE_HEAD frozenAnalogInputs;
  TMWSIM_TABLE_HEAD analogOutputs;
  TMWSIM_TABLE_HEAD stringData;
  TMWSIM_TABLE_HEAD vtermEvents;
  TMWSIM_TABLE_HEAD deviceAttrs; 
  TMWSIM_TABLE_HEAD authSecStatsFromMaster;
  TMWSIM_TABLE_HEAD authSecStatsFromOS;
  TMWSIM_TABLE_HEAD indexedTime;

  MDNPFSIM_XFER_CONTEXT fileSimXferContext;

  TMWDLIST datasetProtos;
  TMWDLIST datasetDescrDatas;

  /* Number of dataset prototypes that have been read from slave 
   * This is used to determine the point number or data set prototype ids for the
   * prototypes defined by the master
   */
  TMWTYPES_USHORT numberSlavePrototypes;

  /* Number of dataset descriptors that have been read from slave 
   * This is used to determine the point number or data set descriptor ids for the
   * descriptors defined by the master
   */
  TMWTYPES_USHORT numberSlaveDescriptors;

  /* Secure Authentication */  
  TMWDLIST authUsers;

  /* User callbacks */
  MDNPSIM_CALLBACK_FUNC pUpdateCallback;
  void *pUpdateCallbackParam;

  TMWTYPES_BOOL nextRcvdIsCritical;
  
  /* time returned from outstation in response to read time request */
  TMWDTIME  readTime;

  /* time returned from outstation in response to either cold or warm restart request */
  TMWTYPES_ULONG restartTime; 

  /* session that opened this database */
  MDNPSESN *pMDNPSession;

  /* Manged SCL database handle*/
  void *managedDBhandle;

} MDNPSIM_DATABASE;


#ifdef __cplusplus
extern "C" {
#endif

  /* function: mdnpsim_init */
  void * TMWDEFS_GLOBAL mdnpsim_init(
    TMWSESN *pSession);

  /* function: mdnpsim_close */
  void TMWDEFS_GLOBAL mdnpsim_close(
    void *pHandle);

  /* function: mdnpsim_clear */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL mdnpsim_clear(
    void *pHandle);

  /* Set update callback and parameter */
  TMWDEFS_SCL_API void mdnpsim_setCallback(
    void *pHandle,
    MDNPSIM_CALLBACK_FUNC pUpdateCallback,
    void *pUpdateCallbackParam);

  /* Store time returned from outstation in response to read time request */
  void mdnpsim_storeReadTime(
    void *pHandle, 
    TMWDTIME *pTimeStamp);

  /* Get time returned from outstation in response to read time request */
  TMWDTIME *mdnpsim_getReadTime(
    void *pHandle);

  /* function: mdnpsim_getPointNumber */
  TMWDEFS_SCL_API TMWTYPES_USHORT TMWDEFS_GLOBAL mdnpsim_getPointNumber(
    void *pPoint);

  /* function: mdnpsim_binaryInputAddPoint */
  TMWDEFS_SCL_API TMWDEFS_GLOBAL void *mdnpsim_binaryInputAddPoint(
    void *pHandle,
    TMWTYPES_USHORT pointNumber);

  /* function: mdnpsim_binaryInputDeletePoint */
  TMWDEFS_SCL_API TMWDEFS_GLOBAL void mdnpsim_binaryInputDeletePoint(
    void *pHandle,
    void *pPoint);

  /* function: mdnpsim_binaryInputGetPoint */
  TMWDEFS_SCL_API TMWDEFS_GLOBAL void *mdnpsim_binaryInputGetPoint(
    void *pHandle,
    TMWTYPES_USHORT index);

  /* function: mdnpsim_binaryInputGetPoint */
  TMWDEFS_SCL_API TMWDEFS_GLOBAL void *mdnpsim_binaryInputLookupPoint(
    void *pHandle,
    TMWTYPES_USHORT pointNumber);

  /* mdnpsim_storeBinaryInput */
  TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpsim_storeBinaryInput(
    void *pHandle, 
    TMWTYPES_USHORT pointNumber, 
    TMWTYPES_BOOL value,
    TMWTYPES_UCHAR flags,
    TMWDTIME *pTimeStamp);

   /* function: mdnpsim_binaryInputGetValue */
  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpsim_binaryInputGetValue(
    void *pPoint);

  /* function: mdnpsim_doubleInputAddPoint */
  TMWDEFS_SCL_API TMWDEFS_GLOBAL void *mdnpsim_doubleInputAddPoint(
    void *pHandle,
    TMWTYPES_USHORT pointNumber);

  /* function: mdnpsim_doubleInputDeletePoint */
  TMWDEFS_SCL_API TMWDEFS_GLOBAL void mdnpsim_doubleInputDeletePoint(
    void *pHandle,
    void *pPoint);

  /* function: mdnpsim_doubleInputGetPoint */
  TMWDEFS_SCL_API TMWDEFS_GLOBAL void *mdnpsim_doubleInputGetPoint(
    void *pHandle,
    TMWTYPES_USHORT index);

  /* function: mdnpsim_doubleInputGetPoint */
  TMWDEFS_SCL_API TMWDEFS_GLOBAL void *mdnpsim_doubleInputLookupPoint(
    void *pHandle,
    TMWTYPES_USHORT pointNumber);

  /* mdnpsim_storeDoubleInput */
  TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpsim_storeDoubleInput(
    void *pHandle, 
    TMWTYPES_USHORT pointNumber, 
    TMWTYPES_UCHAR flagsAndValue,
    TMWDTIME *pTimeStamp);

  /* function: mdnpsim_doubleInputGetValue */
  TMWDEFS_SCL_API TMWTYPES_UCHAR TMWDEFS_GLOBAL mdnpsim_doubleInputGetValue(
    void *pPoint);

  /* function: mdnpsim_binaryOutputAddPoint */
  TMWDEFS_SCL_API TMWDEFS_GLOBAL void *mdnpsim_binaryOutputAddPoint(
    void *pHandle,
    TMWTYPES_USHORT pointNumber);

  /* function: mdnpsim_binaryOutputDeletePoint */
  TMWDEFS_SCL_API TMWDEFS_GLOBAL void mdnpsim_binaryOutputDeletePoint(
    void *pHandle,
    void *pPoint);

  /* function: mdnpsim_binaryOutputGetPoint */
  TMWDEFS_SCL_API TMWDEFS_GLOBAL void *mdnpsim_binaryOutputGetPoint(
    void *pHandle,
    TMWTYPES_USHORT index);

  /* function: mdnpsim_binaryOutputLookupPoint */
  TMWDEFS_SCL_API TMWDEFS_GLOBAL void *mdnpsim_binaryOutputLookupPoint(
    void *pHandle,
    TMWTYPES_USHORT pointNumber);

  /* mdnpsim_storeBinaryOutput */
  TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpsim_storeBinaryOutput(
    void *pHandle, 
    TMWTYPES_USHORT pointNumber, 
    TMWTYPES_BOOL value,
    TMWTYPES_UCHAR flags,
    TMWDTIME *pTimeStamp);

  /* function: mdnpsim_binaryOutputGetValue */
  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpsim_binaryOutputGetValue(
    void *pPoint);

  /* function: mdnpsim_binaryCounterAddPoint */
  TMWDEFS_SCL_API TMWDEFS_GLOBAL void *mdnpsim_binaryCounterAddPoint(
    void *pHandle,
    TMWTYPES_USHORT pointNumber);

  /* function: mdnpsim_binaryCounterDeletePoint */
  TMWDEFS_SCL_API TMWDEFS_GLOBAL void mdnpsim_binaryCounterDeletePoint(
    void *pHandle,
    void *pPoint);

  /* function: mdnpsim_binaryCounterGetPoint */
  TMWDEFS_SCL_API TMWDEFS_GLOBAL void *mdnpsim_binaryCounterGetPoint(
    void *pHandle,
    TMWTYPES_USHORT index);

  /* function: mdnpsim_binaryCounterGetPoint */
  TMWDEFS_SCL_API TMWDEFS_GLOBAL void *mdnpsim_binaryCounterLookupPoint(
    void *pHandle,
    TMWTYPES_USHORT pointNumber);

  /* function: mdnpsim_storeBinaryCounter */
  TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpsim_storeBinaryCounter(
    void *pHandle, 
    TMWTYPES_USHORT pointNumber, 
    TMWTYPES_ULONG value, 
    TMWTYPES_UCHAR flags,
    TMWDTIME *pTimeStamp);
  
  /* function: mdnpsim_binaryCounterGetValue */
  TMWDEFS_SCL_API TMWTYPES_ULONG TMWDEFS_GLOBAL mdnpsim_binaryCounterGetValue(
    void *pPoint);

  /* function: mdnpsim_frozenCounterAddPoint */
  TMWDEFS_SCL_API TMWDEFS_GLOBAL void *mdnpsim_frozenCounterAddPoint(
    void *pHandle,
    TMWTYPES_USHORT pointNumber);

  /* function: mdnpsim_frozenCounterDeletePoint */
  TMWDEFS_SCL_API TMWDEFS_GLOBAL void mdnpsim_frozenCounterDeletePoint(
    void *pHandle,
    void *pPoint);

  /* function: mdnpsim_frozenCounterGetPoint */
  TMWDEFS_SCL_API TMWDEFS_GLOBAL void *mdnpsim_frozenCounterGetPoint(
    void *pHandle,
    TMWTYPES_USHORT index);

  /* function: mdnpsim_frozenCounterGetPoint */
  TMWDEFS_SCL_API TMWDEFS_GLOBAL void *mdnpsim_frozenCounterLookupPoint(
    void *pHandle,
    TMWTYPES_USHORT pointNumber);

  /* function: mdnpsim_storeFrozenCounter */
  TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpsim_storeFrozenCounter(
    void *pHandle, 
    TMWTYPES_USHORT pointNumber, 
    TMWTYPES_ULONG value, 
    TMWTYPES_UCHAR flags,
    TMWDTIME *pTimeStamp);

  
  /* function: mdnpsim_frozenCounterGetValue */
  TMWDEFS_SCL_API TMWTYPES_ULONG TMWDEFS_GLOBAL mdnpsim_frozenCounterGetValue(
    void *pPoint);

  /* function: mdnpsim_analogInputAddPoint */
  TMWDEFS_SCL_API TMWDEFS_GLOBAL void *mdnpsim_analogInputAddPoint(
    void *pHandle,
    TMWTYPES_USHORT pointNumber);

  /* function: mdnpsim_analogInputDeletePoint */
  TMWDEFS_SCL_API TMWDEFS_GLOBAL void mdnpsim_analogInputDeletePoint(
    void *pHandle,
    void *pPoint);

  /* function: mdnpsim_analogInputGetPoint */
  TMWDEFS_SCL_API TMWDEFS_GLOBAL void *mdnpsim_analogInputGetPoint(
    void *pHandle,
    TMWTYPES_USHORT index);

  /* function: mdnpsim_analogInputGetPoint */
  TMWDEFS_SCL_API TMWDEFS_GLOBAL void *mdnpsim_analogInputLookupPoint(
    void *pHandle,
    TMWTYPES_USHORT pointNumber);

  /* mdnpsim_storeAnalogInput */
  TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpsim_storeAnalogInput(
    void *pHandle, 
    TMWTYPES_USHORT 
    pointNumber, 
    TMWTYPES_ANALOG_VALUE *pValue, 
    TMWTYPES_UCHAR flags,
    TMWDTIME *pTimeStamp);
  
  /* mdnpsim_analogInputGetValue */
  TMWDEFS_SCL_API TMWSIM_DATA_TYPE TMWDEFS_GLOBAL mdnpsim_analogInputGetValue(
    void *pDataPoint);

  /* function: mdnpsim_frozenAnalogInputAddPoint */
  TMWDEFS_GLOBAL void *mdnpsim_frozenAnalogInputAddPoint(
    void *pHandle,
    TMWTYPES_USHORT pointNumber);

  /* function: mdnpsim_frozenAnalogInputDeletePoint */
  TMWDEFS_GLOBAL void mdnpsim_frozenAnalogInputDeletePoint(
    void *pHandle,
    void *pPoint);

  /* function: mdnpsim_frozenAnalogInputGetPoint */
  TMWDEFS_GLOBAL void *mdnpsim_frozenAnalogInputGetPoint(
    void *pHandle,
    TMWTYPES_USHORT index);

  /* function: mdnpsim_frozenAnalogInputLookupPoint */
  TMWDEFS_GLOBAL void *mdnpsim_frozenAnalogInputLookupPoint(
    void *pHandle,
    TMWTYPES_USHORT pointNumber);

  /* mdnpsim_storeFrozenAnalogInput */
  TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpsim_storeFrozenAnalogInput(
    void *pHandle, 
    TMWTYPES_USHORT 
    pointNumber, 
    TMWTYPES_ANALOG_VALUE *pValue, 
    TMWTYPES_UCHAR flags,
    TMWDTIME *pTimeStamp);
  
  /* mdnpsim_frozenAnalogInputGetValue */
  TMWDEFS_SCL_API TMWSIM_DATA_TYPE TMWDEFS_GLOBAL mdnpsim_frozenAnalogInputGetValue(
    void *pDataPoint);

  TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpsim_storeAnalogInputDeadband(
    void *pHandle, 
    TMWTYPES_USHORT pointNumber, 
    TMWTYPES_ANALOG_VALUE *pValue);

  /* function: mdnpsim_analogOutputAddPoint */
  TMWDEFS_SCL_API TMWDEFS_GLOBAL void *mdnpsim_analogOutputAddPoint(
    void *pHandle,
    TMWTYPES_USHORT pointNumber);

  /* function: mdnpsim_analogOutputDeletePoint */
  TMWDEFS_SCL_API TMWDEFS_GLOBAL void mdnpsim_analogOutputDeletePoint(
    void *pHandle,
    void *pPoint);

  /* function: mdnpsim_analogOutputGetPoint */
  TMWDEFS_SCL_API TMWDEFS_GLOBAL void *mdnpsim_analogOutputGetPoint(
    void *pHandle,
    TMWTYPES_USHORT index);

  /* function: mdnpsim_analogOutputGetPoint */
  TMWDEFS_SCL_API TMWDEFS_GLOBAL void *mdnpsim_analogOutputLookupPoint(
    void *pHandle,
    TMWTYPES_USHORT pointNumber);

  /* mdnpsim_storeAnalogOutput */
  TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpsim_storeAnalogOutput(
    void *pHandle, 
    TMWTYPES_USHORT pointNumber, 
    TMWTYPES_ANALOG_VALUE *pValue, 
    TMWTYPES_UCHAR flags,
    TMWDTIME *pTimeStamp);

  /* function: mdnpsim_analogOutputGetValue */
  TMWDEFS_SCL_API TMWSIM_DATA_TYPE TMWDEFS_GLOBAL mdnpsim_analogOutputGetValue(
    void *pDataPoint);

  /* Store time returned from outstation in response to cold or warm restart request. */
  void mdnpsim_storeRestartTime(
    void *pHandle,
    TMWTYPES_ULONG time);

  /* Get time returned from outstation in response to cold or warm restart request. */
  TMWTYPES_ULONG mdnpsim_getRestartTime(
    void *pHandle);

  /* function: mdnpsim_indexedTimeAddPoint */
  TMWDEFS_GLOBAL void *mdnpsim_indexedTimeAddPoint(
    void *pHandle,
    TMWTYPES_USHORT pointNumber);

  /* function: mdnpsim_indexedTimeDeletePoint */
  TMWDEFS_GLOBAL void mdnpsim_indexedTimeDeletePoint(
    void *pHandle,
    void *pPoint);

  /* function: mdnpsim_indexedTimeGetPoint */
  TMWDEFS_GLOBAL void *mdnpsim_indexedTimeGetPoint(
    void *pHandle,
    TMWTYPES_USHORT index);

  /* function: mdnpsim_indexedTimeLookupPoint */
  TMWDEFS_GLOBAL void *mdnpsim_indexedTimeLookupPoint(
    void *pHandle,
    TMWTYPES_USHORT pointNumber);

  /* function: mdnpsim_storeString */
  TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpsim_storeIndexedTime(
    void *pHandle,
    TMWTYPES_USHORT pointNumber,
    TMWDTIME indexedTime,
    TMWTYPES_ULONG intervalCount,
    TMWTYPES_BYTE intervalUnit);

  /* function: mdnpsim_stringAddPoint */
  TMWDEFS_SCL_API TMWDEFS_GLOBAL void *mdnpsim_stringAddPoint(
    void *pHandle,
    TMWTYPES_USHORT pointNumber);

  /* function: mdnpsim_stringDeletePoint */
  TMWDEFS_SCL_API TMWDEFS_GLOBAL void mdnpsim_stringDeletePoint(
    void *pHandle,
    void *pPoint);

  /* function: mdnpsim_stringGetPoint */
  TMWDEFS_SCL_API TMWDEFS_GLOBAL void *mdnpsim_stringGetPoint(
    void *pHandle,
    TMWTYPES_USHORT index);

  /* function: mdnpsim_stringGetPoint */
  TMWDEFS_SCL_API TMWDEFS_GLOBAL void *mdnpsim_stringLookupPoint(
    void *pHandle,
    TMWTYPES_USHORT pointNumber);

  /* function: mdnpsim_storeString */
  TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpsim_storeString(
    void *pHandle,
    TMWTYPES_USHORT pointNumber,
    TMWTYPES_UCHAR *pStrBuf,
    TMWTYPES_UCHAR strLen);

  /* function: mdnpsim_virtualTerminalAddPoint */
  TMWDEFS_SCL_API TMWDEFS_GLOBAL void *mdnpsim_virtualTerminalAddPoint(
    void *pHandle,
    TMWTYPES_USHORT pointNumber);

  /* function: mdnpsim_virtualTerminalDeletePoint */
  TMWDEFS_SCL_API TMWDEFS_GLOBAL void mdnpsim_virtualTerminalDeletePoint(
    void *pHandle,
    void *pPoint);

  /* function: mdnpsim_virtualTerminalGetPoint */
  TMWDEFS_SCL_API TMWDEFS_GLOBAL void *mdnpsim_virtualTerminalGetPoint(
    void *pHandle,
    TMWTYPES_USHORT index);

  /* function: mdnpsim_virtualTerminalGetPoint */
  TMWDEFS_SCL_API TMWDEFS_GLOBAL void *mdnpsim_virtualTerminalLookupPoint(
    void *pHandle,
    TMWTYPES_USHORT pointNumber);

  /* function: mdnpsim_storeVirtualTerminal */
  TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpsim_storeVirtualTerminal(
    void *pHandle,
    TMWTYPES_USHORT pointNumber,
    TMWTYPES_UCHAR *pVtermBuf,
    TMWTYPES_UCHAR vtermLen);

  /* function: mdnpsim_extStringAddPoint */
  TMWDEFS_SCL_API TMWDEFS_GLOBAL void *mdnpsim_extStringAddPoint(
    void *pHandle,
    TMWTYPES_USHORT pointNumber);

  /* function: mdnpsim_extStringDeletePoint */
  TMWDEFS_SCL_API TMWDEFS_GLOBAL void mdnpsim_extStringDeletePoint(
    void *pHandle,
    void *pPoint);

  /* function: mdnpsim_extStringGetPoint */
  TMWDEFS_SCL_API TMWDEFS_GLOBAL void *mdnpsim_extStringGetPoint(
    void *pHandle,
    TMWTYPES_USHORT index);

  /* function: mdnpsim_extStringGetPoint */
  TMWDEFS_SCL_API TMWDEFS_GLOBAL void *mdnpsim_extStringLookupPoint(
    void *pHandle,
    TMWTYPES_USHORT pointNumber);

  /* function: mdnpsim_storeExtString */
  TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpsim_storeExtString(
    void *pHandle,
    TMWTYPES_USHORT pointNumber,
    TMWTYPES_UCHAR *pStrBuf,
    TMWTYPES_USHORT strLen,
    TMWTYPES_UCHAR  flags,
    TMWDTIME *pTimeStamp);

  /* function: mdnpsim_stringOrExtGetPoint 
   * Strings and Extended Strings share a list. Get either one 
   * if it exists with that index.
   */
  TMWDEFS_GLOBAL void *mdnpsim_stringOrExtGetPoint(
    void *pHandle,
    TMWTYPES_USHORT index);

  TMWDEFS_SCL_API TMWDEFS_GLOBAL void *mdnpsim_storeDeviceAttribute(
    void *pHandle,
    TMWTYPES_USHORT pointNumber,
    TMWTYPES_UCHAR variation,
    DNPDATA_ATTRIBUTE_VALUE *pData);

  TMWDEFS_SCL_API TMWDEFS_GLOBAL void *mdnpsim_storeDeviceAttrProperty(
    void *pDbHandle, 
    TMWTYPES_USHORT pointNumber,
    TMWTYPES_UCHAR variation,
    TMWTYPES_UCHAR property);
   
  TMWDEFS_GLOBAL void *mdnpsim_deviceAttrGetPoint(
    void            *pHandle,
    TMWTYPES_USHORT  point,
    TMWTYPES_UCHAR   variation);

  TMWDEFS_SCL_API TMWDEFS_GLOBAL void *mdnpsim_deviceAttrGet(
    void *pHandle,
    TMWTYPES_USHORT pointNumber,
    TMWTYPES_UCHAR variation, 
    TMWTYPES_UCHAR *pProperty,
    DNPDATA_ATTRIBUTE_VALUE *pData);

  /* function: mdnpsim_deviceAttrDeletePoint */
  TMWDEFS_SCL_API TMWDEFS_GLOBAL void mdnpsim_deviceAttrDeletePoint(
    void *pHandle,
    TMWTYPES_USHORT pointNumber,
    TMWTYPES_UCHAR variation);

  TMWDEFS_SCL_API TMWTYPES_USHORT TMWDEFS_GLOBAL mdnpsim_datasetProtoQuantity(
    void *pHandle);

  TMWDEFS_SCL_API TMWTYPES_USHORT TMWDEFS_GLOBAL mdnpsim_datasetProtoSlaveQty(
    void *pHandle);

  TMWDEFS_SCL_API void TMWDEFS_GLOBAL mdnpsim_storeDatasetProtoUUID(
    void *pHandle, 
    TMWTYPES_USHORT pointNumber, 
    TMWTYPES_UCHAR *pUUID,
    TMWTYPES_BOOL fromSlave);

  TMWDEFS_SCL_API void TMWDEFS_GLOBAL mdnpsim_storeDatasetProto(
    void *pHandle, 
    TMWTYPES_USHORT pointNumber, 
    TMWTYPES_UCHAR  elemIndex,
    DNPDATA_DATASET_DESCR_ELEM *pDescr);
  
  /* function: mdnpsim_datasetProtoDeletePoint */
  TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpsim_datasetProtoDeletePoint(
    void *pHandle,
    TMWTYPES_USHORT pointNum);

  /* function: mdnpsim_datasetProtoGetID */
  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_CALLBACK mdnpsim_datasetProtoGetID(
    void *pHandle,
    TMWTYPES_UCHAR *pUUID,
    TMWTYPES_USHORT *pPointNum);

  TMWDEFS_SCL_API DNPDATA_DATASET_DESCR_ELEM * TMWDEFS_GLOBAL mdnpsim_datasetProtoGet(
    void *pHandle,
    TMWTYPES_USHORT pointNumber,
    TMWTYPES_UCHAR *pNumberElems,
    TMWTYPES_UCHAR *pUUID);

  void * TMWDEFS_CALLBACK mdnpsim_datasetProtoGetPoint(
    void *pHandle,
    TMWTYPES_USHORT pointNumber);

  TMWDEFS_SCL_API TMWTYPES_USHORT TMWDEFS_GLOBAL mdnpsim_datasetDescrQuantity(
    void *pHandle);

  TMWTYPES_USHORT TMWDEFS_GLOBAL mdnpsim_datasetDescrSlaveQty(
    void *pHandle);

  TMWDEFS_SCL_API void TMWDEFS_GLOBAL mdnpsim_storeDatasetDescrCont(
    void *pHandle, 
    TMWTYPES_USHORT pointNumber, 
    TMWTYPES_UCHAR  elemIndex,
    DNPDATA_DATASET_DESCR_ELEM *pDescr,
    TMWTYPES_BOOL fromSlave);

  /* function: mdnpsim_datasetDescrDeletePoint */
  TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpsim_datasetDescrDeletePoint(
    void *pHandle,
    TMWTYPES_USHORT pointNum);

  TMWDEFS_SCL_API void TMWDEFS_GLOBAL mdnpsim_storeDatasetDescrChars(
    void *pHandle, 
    TMWTYPES_USHORT pointNumber, 
    TMWTYPES_UCHAR  value);

  TMWDEFS_SCL_API void TMWDEFS_GLOBAL mdnpsim_storeDatasetDescrIndex(
    void *pHandle, 
    TMWTYPES_USHORT pointNumber, 
    TMWTYPES_UCHAR  elemIndex,
    DNPDATA_DATASET_DESCR_INDEX *pDescr,
    TMWTYPES_BOOL fromSlave);

  TMWDEFS_SCL_API DNPDATA_DATASET_DESCR_ELEM * TMWDEFS_GLOBAL mdnpsim_datasetDescrGetCont(
    void *pHandle,
    TMWTYPES_USHORT pointNumber,
    TMWTYPES_UCHAR *pNumberElems);

  DNPDATA_DATASET_DESCR_ELEM *mdnpsim_datasetDescrGetExpIndex(
    void *pPoint, 
    TMWTYPES_UCHAR index);

  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpsim_datasetDescrGetChars(
    void *pHandle,
    TMWTYPES_USHORT pointNumber,
    TMWTYPES_UCHAR *pValue);

  TMWDEFS_SCL_API DNPDATA_DATASET_DESCR_INDEX * TMWDEFS_GLOBAL mdnpsim_datasetDescrGetIndex(
    void *pHandle,
    TMWTYPES_USHORT pointNumber,
    TMWTYPES_UCHAR *pNumberElems);
  
  TMWDEFS_SCL_API void * TMWDEFS_CALLBACK mdnpsim_datasetDescrGetPoint(
    void *pHandle,
    TMWTYPES_USHORT pointNumber);

  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpsim_datasetDescrGetName(
    void *pHandle,
    TMWTYPES_USHORT pointNumber,
    char *buf);

  TMWDEFS_SCL_API TMWTYPES_USHORT TMWDEFS_GLOBAL mdnpsim_datasetQuantity(
    void *pHandle);

  TMWDEFS_SCL_API DNPDATA_DATASET_DESCR_INDEX * TMWDEFS_GLOBAL mdnpsim_datasetGetIndex(
    void *pHandle,
    TMWTYPES_USHORT pointNumber,
    TMWTYPES_UCHAR *pNumberElems);
  
  void * TMWDEFS_CALLBACK mdnpsim_datasetGetPoint(
    void *pHandle,
    TMWTYPES_USHORT pointNumber);

  void TMWDEFS_GLOBAL mdnpsim_storeDatasetTime(
    void *pHandle, 
    TMWTYPES_USHORT pointNumber, 
    TMWDTIME *pTimeStamp) ;

  TMWDEFS_SCL_API void TMWDEFS_GLOBAL mdnpsim_datasetCreatePoint(
    void *pHandle, 
    TMWTYPES_USHORT pointNumber, 
    TMWDTIME *pTimeStamp); 

  TMWDEFS_SCL_API void TMWDEFS_GLOBAL mdnpsim_storeDataset(
    void *pHandle, 
    TMWTYPES_USHORT pointNumber, 
    TMWTYPES_UCHAR  elemIndex,
    DNPDATA_DATASET_VALUE *pElem);

  TMWDEFS_SCL_API DNPDATA_DATASET_VALUE * TMWDEFS_GLOBAL mdnpsim_datasetGet(
    void *pHandle,
    TMWTYPES_USHORT pointNumber,
    TMWTYPES_UCHAR *pNumberElems,
    TMWDTIME *pTimeStamp);

  void TMWDEFS_GLOBAL mdnpsim_setNextRcvdIsCritical(
    void *pHandle,
    TMWTYPES_BOOL value);

  TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpsim_getNextRcvdIsCritical(
    void *pHandle);

  /* function: mdnpsim_authGetNewSessionKeys */
  TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpsim_authGetNewSessionKeys(
    void             *pHandle, 
    DNPDATA_AUTH_KEY *pControlSessionKey,
    DNPDATA_AUTH_KEY *pMonitorSessionKey);

  /* function: mdnpsim_authKeyWrapSupport */
  TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpsim_authKeyWrapSupport(
    TMWTYPES_UCHAR keyWrapAlgorithm);

  /* function: mdnpsim_authEncryptKeyWrapData */
  TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpsim_authEncryptKeyWrapData(
    void            *pHandle, 
    TMWTYPES_USHORT  userNumber,
    TMWTYPES_UCHAR   keyWrapAlgorithm,
    TMWTYPES_UCHAR  *pPlainData, 
    TMWTYPES_USHORT  plainDataLength, 
    TMWTYPES_UCHAR  *pEncryptedData,
    TMWTYPES_USHORT *pEncryptedLength);

  /* function: mdnpsim_authHMACSupport */
  TMWTYPES_CHAR mdnpsim_authHMACSupport(
    TMWTYPES_UCHAR HMACAlgorithm);

  /* function: mdnpsim_authHMACValue */
  TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpsim_authHMACValue(
    TMWTYPES_UCHAR    algorithm,
    DNPDATA_AUTH_KEY *pKey,
    TMWTYPES_UCHAR   *pData,
    TMWTYPES_ULONG    dataLength,
    TMWTYPES_UCHAR   *pHMACValue,
    TMWTYPES_USHORT  *pHMACValueLength);

  /* function: mdnpsim_authRandomChallengeData */
  TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpsim_authRandomChallengeData(
    TMWTYPES_UCHAR  *pBuf,
    TMWTYPES_USHORT  minLength,
    TMWTYPES_USHORT *pLength);

  /* function: mdnpsim_authGetOSName */
  TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpsim_authGetOSName(
    void            *pHandle, 
    TMWTYPES_UCHAR  *pOSName,
    TMWTYPES_USHORT *pOSNameLength);

  /* function: mdnpsim_authConfigUser  
   *  Store information received from Authority to be used in g120v10 to
   *   start an update key change sequence. This is configuration, this function is
   *   NOT called by DNP library. It will create a user if one by this name
   *   does not exist, or update an existing user.
   * arguments:
   *  pDbHandle - pointer 
   *  pUserName - pointer to globally unique identifier representing user.
   *  userNameLength - length of user name.
   *  statusChangeSequenceNumber - sequence number from Authority
   *  keyChangeMethod -
   *  userRole -
   *  userRoleExpiryInterval -
   *  pUserPublicKey - if asymmetric method is used
   *  userPublicKeyLength - length of public key if asymmetric method is used.
   *  pCertificationData
   *   containing Status Change Sequence (SCS) number between Authority and Outstation,
   *   A globally unique identifier representing the user,  
   *   Interval, Role and Users public key.
   *   Also this same information digitally signed or encrypted depending on whether
   *   symmetric or asymmetric change method is being used. 
   *  certificationDataLength - length of the certification data
   * returns: 
   *   userNameDbHandle  handle into database to look up information
   *  This data will be accessed by mdnpdata_authGetChangeUserData
   *   mdnpdata_authGetUserPublicKey and mdnpdata_authGetCertData using this handle.
  */
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
    TMWTYPES_USHORT   certDataLength);

  /* Get globally unique user name provided by Authority when
   * it requested a User Status Change 
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpsim_authGetUserName(
    void            *pHandle, 
    void            *userDbHandle,
    TMWTYPES_UCHAR  *pUserName,
    TMWTYPES_USHORT *pUserNameLength);

  /* function: mdnpsim_authGetUserNumber
   *  Get User Number stored by mdnpsim_authConfigUser
   *  This is configuration, this function is NOT called by DNP library.
   */
  TMWTYPES_USHORT TMWDEFS_GLOBAL mdnpsim_authGetUserNumber(
    void            *pHandle, 
    void            *userDbHandle);

  /* Get data provided by Authority when it requested a User Status Change */
  TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpsim_authGetChangeUserData(
    void            *pHandle, 
    void            *userDbHandle,
    TMWTYPES_ULONG  *pStatusChangeSequenceNumber,
    TMWTYPES_UCHAR  *pKeyChangeMethod,
    TMWTYPES_USHORT *pUserRole,
    TMWTYPES_USHORT *pUserRoleExpiryInterval);
 
  TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpsim_authGetCertData(
    void            *pHandle, 
    void            *userDbHandle,
    TMWTYPES_UCHAR  *pCertData,
    TMWTYPES_USHORT *pCertDataLength);
 
  TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpsim_authGetUpdateKeyData(
    void            *pHandle,
    void            *userDbHandle,
    TMWTYPES_UCHAR  *pOSData,
    TMWTYPES_USHORT *pOSDataLength);

  void TMWDEFS_GLOBAL mdnpsim_authStoreUpdKeyChangeReply(
    void            *pHandle, 
    void            *userDbHandle,
    TMWTYPES_USHORT  userNumber);

  TMWTYPES_USHORT TMWDEFS_GLOBAL mdnpsim_authDeleteUser(
    void            *pHandle, 
    void            *userDbHandle);
  
  /* function: mdnpsim_addSecStats */
  /* Add Master Security Statistics points as required for SA */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL mdnpsim_addSecStats(
    void *pHandle);

  /* function: mdnpsim_authSecStatAddPoint */
  TMWDEFS_GLOBAL void *mdnpsim_authSecStatAddPoint(
    void *pHandle,
    TMWTYPES_BOOL fromOS,
    TMWTYPES_USHORT pointNumber);

  /* function: mdnpsim_authSecStatDeletePoint */
  TMWDEFS_GLOBAL void mdnpsim_authSecStatDeletePoint(
    void *pHandle,
    TMWTYPES_BOOL fromOS,
    void *pPoint);

  /* function: mdnpsim_authSecStatGetPoint */
  TMWDEFS_GLOBAL void *mdnpsim_authSecStatGetPoint(
    void *pHandle,
    TMWTYPES_BOOL fromOS,
    TMWTYPES_USHORT index);

  /* function: mdnpsim_authSecStatLookupPoint */
  TMWDEFS_GLOBAL void *mdnpsim_authSecStatLookupPoint(
    void *pHandle,
    TMWTYPES_BOOL fromOS,
    TMWTYPES_USHORT pointNumber);

  /* function: mdnpsim_storeAuthSecStat */
  TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpsim_storeAuthSecStat(
    void *pHandle,
    TMWTYPES_BOOL fromOS,
    TMWTYPES_USHORT assocId,
    TMWTYPES_USHORT pointNumber,
    TMWTYPES_ULONG value,
    TMWTYPES_UCHAR flags,
    TMWDTIME *pTimeStamp);

  /* function: mdnpsim_authSecStatGetValue */
  TMWTYPES_ULONG TMWDEFS_GLOBAL mdnpsim_authSecStatGetValue(
    void *pPoint);

#if TMWCNFG_SUPPORT_DIAG
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL mdnpsim_showData(
    TMWSESN *pSession);
#endif

#if MDNPSIM_SUPPORT_XML
  /* routine: mdnpsim_saveDatabase */
  TMWDEFS_SCL_API TMWTYPES_CHAR * TMWDEFS_GLOBAL mdnpsim_saveDatabase(
    TMWSESN *pSession);
#endif

#ifdef __cplusplus
}
#endif
#endif /* TMWCNFG_USE_SIMULATED_DB */
#endif /* MDNPSIM_DEFINED */
