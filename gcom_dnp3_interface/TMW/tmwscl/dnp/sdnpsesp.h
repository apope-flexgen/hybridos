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

/* file: sdnpsesp.h
 * description: This file is intended for internal SCL use only.
 *   Private slave DNP session support
 */
#ifndef SDSESN_DEFINED
#define SDSESN_DEFINED

#include "tmwscl/dnp/dnputil.h"
#include "tmwscl/dnp/dnpsesn.h"
#include "tmwscl/dnp/dnpcnfg.h"
#include "tmwscl/dnp/sdnpdata.h"
#include "tmwscl/dnp/sdnpcnfg.h"
#include "tmwscl/dnp/sdnpxml.h"
#include "tmwscl/dnp/sdnpxml2.h"
#include "tmwscl/utils/tmwchnl.h"
#include "tmwscl/dnp/dnpbncfg.h"

typedef enum {
  /* the read of this type is complete*/
  SDNPSESN_READ_COMPLETE,
  /* There is more data for this type, but there is no more room 
   * this will probably result in a multifragment response 
   */
  SDNPSESN_READ_MORE_DATA,
  /* There is no more data for this type, but do not put more data
   * in this response, used by secure authentication.
   */
  SDNPSESN_READ_NOMORE_DATA,
  SDNPSESN_READ_NEW_VARIATION,
  SDNPSESN_READ_FAILED
} SDNPSESN_READ_STATUS;

typedef enum {
  SDNPSESN_QUAL_PARSE_ONLY,
  SDNPSESN_QUAL_BUILD_RESPONSE
} SDNPSESN_QUAL;

#if SDNPDATA_SUPPORT_OBJ12_V2
typedef struct {
  TMWTYPES_USHORT pcbStatusOffset;
  TMWTYPES_UCHAR control;
  TMWTYPES_UCHAR count;
  TMWTYPES_ULONG activation;
  TMWTYPES_ULONG deactivation;
} PCB_DATA;
#endif

typedef struct {
  TMWSESN *pSession;
  TMWDEFS_CLASS_MASK classMask;
} UNSOL_DELAY_TIMER;

/* This size is large enough for binary output Obj 12 Var 1 
 * and for analog outputs Obj 41 Variations 1-4 
 * for datasets this needs to be large enough to hold a dataset 
 * object group 87 variation 1 
 */
#ifndef SDNPSESN_SELECT_BUFFER_SIZE
#define SDNPSESN_SELECT_BUFFER_SIZE  (16 + (SDNPCNFG_MAX_CONTROL_REQUESTS * 13))
#endif

#define SDNPSESN_NOREAD   0x00
#define SDNPSESN_OBJ2READ 0x01
#define SDNPSESN_OBJ4READ 0x02
#define SDNPSESN_OBJ2ANDOBJ4READ 0x03

/* Slave DNP Session Context */
typedef struct SDNPSessionStruct {

  /* Generic dnp Session info, must be first entry */
  DNPSESN dnp;

  /* Parsing Table says send qualifier 5b for activate config response. 
   * For backward compatibility with old implementations send qualifier 7 instead.
   */
  TMWTYPES_BOOL sendObj91RespQual7;

  /* Slave specific configuration */
  TMWTYPES_BOOL multiFragRespAllowed;
  TMWTYPES_BOOL multiFragConfirm;
  
  /* Bit mask indicating which Writes are enabled for broadcast requests if broadcast Write is enabled */ 
  TMWTYPES_UCHAR enabledBroadcastWrites;
  
  /* Bit mask indicating which Writes are enabled for device specific requests if device spec Write is enabled */ 
  TMWTYPES_UCHAR enabledWrites;

  /* Bit mask indicating which Function Codes are enabled for broadcast requests */ 
  TMWTYPES_ULONG enabledBroadcastFCs;

  /* Bit mask indicating which Function Codes are enabled for device specific requests */ 
  TMWTYPES_ULONG enabledFCs;
 
  TMWTYPES_MILLISECONDS applConfirmTimeout;
  TMWTYPES_MILLISECONDS unsolConfirmTimeout;
  TMWTYPES_MILLISECONDS selectTimeout;
  
  TMWTYPES_BOOL respondNeedTime;
  TMWTYPES_MILLISECONDS clockValidPeriod;

  TMWTYPES_USHORT warmRestartDelay;
  TMWTYPES_USHORT coldRestartDelay;
  
  TMWTYPES_BOOL allowMultiCROBRequests;
  TMWTYPES_UCHAR maxControlRequests;

  TMWTYPES_BOOL recordedCurrentTime;
  TMWTYPES_MILLISECONDS recordCurrentTime;

  TMWTYPES_BOOL fileTransferEnabled;
  TMWTYPES_MILLISECONDS fileTransferTimeout;

  TMWTYPES_BOOL unsolAllowed;
  TMWTYPES_BOOL sendUnsolWhenOnline;
  TMWTYPES_BOOL unsolSendIdenticalRetry;

  TMWTYPES_UCHAR unsolMaxEvents[3];
  TMWTYPES_MILLISECONDS unsolMaxDelay[3];

  TMWTYPES_USHORT unsolMaxRetries;
  TMWTYPES_MILLISECONDS unsolRetryDelay;
  TMWTYPES_MILLISECONDS unsolOfflineRetryDelay;
  TMWTYPES_BOOL unsolOfflineDelayOnce;

  /* Default variations */
  TMWTYPES_UCHAR obj01DefaultVariation;
  TMWTYPES_UCHAR obj02DefaultVariation;
  TMWTYPES_UCHAR obj03DefaultVariation;
  TMWTYPES_UCHAR obj04DefaultVariation;
  TMWTYPES_UCHAR obj10DefaultVariation;
  TMWTYPES_UCHAR obj11DefaultVariation;
  TMWTYPES_UCHAR obj13DefaultVariation;
  TMWTYPES_UCHAR obj20DefaultVariation;
  TMWTYPES_UCHAR obj21DefaultVariation;
  TMWTYPES_UCHAR obj22DefaultVariation;
  TMWTYPES_UCHAR obj23DefaultVariation;
  TMWTYPES_UCHAR obj30DefaultVariation;
  TMWTYPES_UCHAR obj31DefaultVariation;
  TMWTYPES_UCHAR obj32DefaultVariation;
  TMWTYPES_UCHAR obj33DefaultVariation;
  TMWTYPES_UCHAR obj34DefaultVariation;
  TMWTYPES_UCHAR obj40DefaultVariation;
  TMWTYPES_UCHAR obj42DefaultVariation;
  TMWTYPES_UCHAR obj43DefaultVariation;
  TMWTYPES_UCHAR obj114DefaultVariation;
  TMWTYPES_UCHAR obj115DefaultVariation;
  TMWTYPES_UCHAR obj122DefaultVariation;

  /* If this is TMWDEFS_TRUE the event with the earliest timeStamp will be 
   * deleted when a new event is added to an event queue that is full.
   */
  TMWTYPES_BOOL deleteOldestEvent;

  /* If this is TMWDEFS_TRUE event queueing and retrieval will be handled outside
   * of the SCL through the sdnpdata_umEventxxx() functions.
   */
  TMWTYPES_BOOL userManagedEvents;

#if SDNPDATA_SUPPORT_STATIC_VAR_POINT || SDNPDATA_SUPPORT_EVENT_VAR_POINT
  /* If this is TMWDEFS_TRUE it allows per point specification of precision of 
   * counter points.
  */
  TMWTYPES_BOOL counterSizeVarPerPoint;

  /* If this is TMWDEFS_TRUE it allows per point specification of precision of 
   * analog input points.
  */
  TMWTYPES_BOOL analogInSizeVarPerPoint;

  /* If this is TMWDEFS_TRUE it allows per point specification of precision of 
   * analog output points.
   */
  TMWTYPES_BOOL analogOutSizeVarPerPoint;
#endif

#if SDNPDATA_SUPPORT_OBJ2
  /* Binary Input Event Configuration */
  TMWTYPES_USHORT binaryInputMaxEvents;
  TMWDEFS_EVENT_MODE binaryInputEventMode;
  TMWTYPES_MILLISECONDS binaryInputScanPeriod;
#endif
  
#if SDNPDATA_SUPPORT_OBJ4
  /* Double Bit Input Event Configuration */
  TMWTYPES_USHORT doubleInputMaxEvents;
  TMWDEFS_EVENT_MODE doubleInputEventMode;
  TMWTYPES_MILLISECONDS doubleInputScanPeriod;
#endif

#if SDNPDATA_SUPPORT_OBJ11
  /* Binary Output Event Configuration */
  TMWTYPES_USHORT binaryOutputMaxEvents;
  TMWDEFS_EVENT_MODE binaryOutputEventMode;
  TMWTYPES_MILLISECONDS binaryOutputScanPeriod;
#endif

#if SDNPDATA_SUPPORT_OBJ13
  /* Binary Output Command Event Configuration */
  TMWTYPES_USHORT binaryOutCmdMaxEvents;
  TMWDEFS_EVENT_MODE binaryOutCmdEventMode;
  TMWTYPES_MILLISECONDS binaryOutCmdScanPeriod;
#endif
  
#if SDNPDATA_SUPPORT_OBJ32
  /* Analog Input Event Configuration */
  TMWTYPES_USHORT analogInputMaxEvents;
  TMWDEFS_EVENT_MODE analogInputEventMode;
  TMWTYPES_MILLISECONDS analogInputScanPeriod;
#endif
  
#if SDNPDATA_SUPPORT_OBJ22
  /* Binary Counter Event Configuration */
  TMWTYPES_USHORT binaryCounterMaxEvents;
  TMWDEFS_EVENT_MODE binaryCounterEventMode;
  TMWTYPES_MILLISECONDS binaryCounterScanPeriod;
#endif
  
#if SDNPDATA_SUPPORT_OBJ23
  /* Frozen Counter Event Configuration */
  TMWTYPES_USHORT frozenCounterMaxEvents;
  TMWDEFS_EVENT_MODE frozenCounterEventMode;
  TMWTYPES_MILLISECONDS frozenCounterScanPeriod;
#endif

#if SDNPDATA_SUPPORT_OBJ33
  /* Frozen Ananlog Input Event Configuration */
  TMWTYPES_USHORT frozenAnalogInMaxEvents;
  TMWDEFS_EVENT_MODE frozenAnalogInEventMode;
  TMWTYPES_MILLISECONDS frozenAnalogInScanPeriod;
#endif

#if SDNPDATA_SUPPORT_OBJ42
  /* Analog Output Event Configuration */
  TMWTYPES_USHORT analogOutputMaxEvents;
  TMWDEFS_EVENT_MODE analogOutputEventMode;
  TMWTYPES_MILLISECONDS analogOutputScanPeriod;
#endif

#if SDNPDATA_SUPPORT_OBJ43
  /* Analog Output Command Event Configuration */
  TMWTYPES_USHORT analogOutCmdMaxEvents;
  TMWDEFS_EVENT_MODE analogOutCmdEventMode;
  TMWTYPES_MILLISECONDS analogOutCmdScanPeriod;
#endif
  
#if SDNPDATA_SUPPORT_OBJ88
  /* Data Set Snapshot Event Configuration */
  TMWTYPES_USHORT datasetMaxEvents;
  TMWDEFS_EVENT_MODE datasetEventMode;
#endif
  
#if SDNPDATA_SUPPORT_OBJ110
  TMWTYPES_BOOL truncateStrings;
#if SDNPDATA_SUPPORT_OBJ111
  /* String Event Configuration */
  TMWTYPES_USHORT stringMaxEvents;
  TMWDEFS_EVENT_MODE stringEventMode;
  TMWTYPES_MILLISECONDS stringScanPeriod;
#endif
#endif
  
#if SDNPDATA_SUPPORT_OBJ113
  /* Virtual Terminal Event Configuration */
  TMWTYPES_USHORT virtualTerminalMaxEvents;
  TMWTYPES_BOOL truncateVirtualTerminalEvents;
  TMWDEFS_EVENT_MODE virtualTerminalEventMode;
  TMWTYPES_MILLISECONDS virtualTerminalScanPeriod;
#endif

#if SDNPDATA_SUPPORT_OBJ114
  TMWTYPES_BOOL truncateExtStrings;
#if SDNPDATA_SUPPORT_OBJ115
  /* Extended String Event Configuration */
  TMWTYPES_USHORT extStringMaxEvents;
  TMWDEFS_EVENT_MODE extStringEventMode;
  TMWTYPES_MILLISECONDS extStringScanPeriod;
#endif
#endif

#if SDNPDATA_SUPPORT_OBJ120
  /* Security Statistics */
  TMWTYPES_USHORT authSecStatMaxEvents; 
  TMWDEFS_EVENT_MODE authSecStatEventMode;
  TMWTYPES_MILLISECONDS authSecStatScanPeriod;
#endif

  TMWTYPES_BOOL sortEventsByTime;
  TMWTYPES_BOOL supportEventsWithTime;

  /* Event buffers */
  TMWTIMER obj2Timer;
  TMWTIMER obj4Timer;
  /* Object 2 and 4 events are sorted in the same queue */
  TMWDLIST obj2_4Events;

  TMWTIMER obj11Timer;
  TMWDLIST obj11Events;

  TMWTIMER obj13Timer;
  TMWDLIST obj13Events;

  TMWTIMER obj22Timer;
  TMWDLIST obj22Events;

  TMWTIMER obj23Timer;
  TMWDLIST obj23Events;

  TMWTIMER obj32Timer;
  TMWDLIST obj32Events;

  TMWTIMER obj33Timer;
  TMWDLIST obj33Events;

  TMWTIMER obj42Timer;
  TMWDLIST obj42Events;

  TMWTIMER obj43Timer;
  TMWDLIST obj43Events;

  TMWDLIST obj88Events;

  TMWTIMER obj111Timer;
  TMWDLIST obj111Events;

  TMWTIMER obj113Timer;
  TMWDLIST obj113Events;

  TMWTIMER obj115Timer;
  TMWDLIST obj115Events;

  /* Authentication error events */
  TMWDLIST obj120Events;
  /* Authentication statistic events */
  TMWDLIST obj122Events;

  /* Clock valid timer */
  TMWTIMER clockValidTimer;

  /* If multi fragment response is waiting for application confirm */
  TMWTYPES_BOOL multiFragWaitingForConfirm;

  /* Unsolicited message parameters */
  TMWTIMER           unsolDelayTimer[TMWDEFS_CLASS_MAX];
  UNSOL_DELAY_TIMER  unsolDelayTimerParam[TMWDEFS_CLASS_MAX];
  TMWTYPES_USHORT    unsolNumPending[TMWDEFS_CLASS_MAX];
  TMWDEFS_CLASS_MASK unsolEventMask;
  TMWTYPES_BOOL      unsolSendByClass;

  TMWTIMER unsolRetryTimer;
  TMWTYPES_BOOL unsolEventsReady;
  TMWTYPES_USHORT unsolNumRetries;
  TMWTYPES_BOOL unsolWaitingForConfirm;
  TMWTYPES_BOOL unsolQueued;
  TMWTYPES_BOOL unsolInitialNullConfirmed;
  TMWTYPES_BOOL unsolResponseContainsEvents;
  DNPCHNL_TX_DATA *pUnsolLastResponse;

  /* Internal Indication bits */
  TMWTYPES_USHORT iin;

  /* If event buffer overflows after some events have been sent to master, 
   * IIN overflow bit must be sent even if we receive a confirm for those events 
   * so that the queue is no longer full. The overflow IIN must be sent at least 
   * once to the master.
   */
  TMWTYPES_BOOL bufferOverFlowSent;

  /* Is an application confirmation required to clear all stations IIN gbit */
  TMWTYPES_BOOL allStationsConfirmRequired;

  /* Database Handle */
  void *pDbHandle;

  /* Last sequence number received from remote device */
  TMWTYPES_UCHAR recvSequenceNumber;

  /* Unsolicited sequence number */
  TMWTYPES_UCHAR unsolSequenceNumber;
  TMWTYPES_UCHAR lastUnsolSequenceNumber;

  /* Sequence number to be sent in response message */
  TMWTYPES_UCHAR respSequenceNumber;

  /* This is ECSN in application layer spec, February 2007 */
  TMWTYPES_UCHAR lastRespSequenceNumber;

  /* Select operate processing */
  TMWTIMER selectTimer;

  /* Buffer to hold select request */
  TMWTYPES_UCHAR selectBuffer[SDNPSESN_SELECT_BUFFER_SIZE];
  TMWTYPES_USHORT selectBufferLength;
  TMWTYPES_UCHAR selectSequenceNumber;

  /* Buffer to hold current request */
  TMWTYPES_BOOL  duplicateRequestRcvd;
  TMWTYPES_BOOL  pendingRequest;
  DNPUTIL_RX_MSG lastRcvdRequest;

  /* State information for managing read requests */
  TMWTYPES_UCHAR readPass;
  TMWTYPES_BOOL readPending;
  TMWTYPES_BOOL databaseLocked;
  TMWTYPES_USHORT readPointIndex;
  /* This also holds variation when reading device attributes */
  TMWTYPES_USHORT readGroupIndex;
  /* This is to keep track of read limited quantity over multiple fragments */
  TMWTYPES_USHORT readMultiFragNumberOfPoints;
  SDNPSESN_READ_STATUS readStatus;

  /* For reading event groups */
  DNPUTIL_OBJECT_HEADER eventObjectHeader;
  TMWDEFS_CLASS_MASK readEventClassesRequested;

#if SDNPDATA_SUPPORT_OBJ2 && SDNPDATA_SUPPORT_OBJ4
  /* If a response contains both Binary Inputs and Double Bit Binary Inputs 
   * they must be sorted together in response. These fields facilitate that.
   */
  TMWTYPES_UCHAR readObj2AndObj4;
  TMWTYPES_UCHAR obj2Variation;
  TMWTYPES_UCHAR obj4Variation;
  TMWTYPES_ULONG obj2Count;
  TMWTYPES_ULONG obj4Count;
#endif

  /* For reading static groups */
  DNPUTIL_OBJECT_HEADER staticObjectHeader;
  TMWTYPES_BOOL readStaticObjectsRequested; 

  /* Groups included in response to read static data request */
  TMWTYPES_UCHAR staticGroups[SDNPCNFG_MAX_NUMBER_STATIC_GROUPS];

#if SDNPDATA_SUPPORT_OBJ50_V1
  TMWTYPES_MILLISECONDS delayMeasurementRxTime;
#endif

#if SDNPDATA_SUPPORT_OBJ12_V2
  PCB_DATA pcbData;
#endif

#if SDNPDATA_SUPPORT_OBJ70
  /* File transfer control block */
  void *pObj70FileCtrl;
#endif
  
#if SDNPDATA_KEEP_LAST_RESPONSE
  /* Application layer spec, Feb 2007, says keep a copy of last solicited response 
   * Don't save last unsolicited response, since it will not be retried/resent, 
   * it will be regenerated if it needs to be sent again.
   */
  DNPCHNL_TX_DATA *pLastResponse; 

  /* For multifragment responses, don't resend a fragment after 
   * an application confirm has been received for any fragment(s).
   */
  TMWTYPES_BOOL notDuplicateEligible;
#endif

#if DNPCNFG_SUPPORT_AUTHENTICATION
  TMWTYPES_BOOL   authenticationEnabled;

  /* If outstation is going to challenge an application confirm, there can be a problem
   * if the master does not expect it. The master may send another request, or if there is
   * a half duplex line, there may be a collision. Setting this to TMWDEFS_TRUE will cause
   * a "preChallenge" G120V1 to be appended to the data the outstation sends with the 
   * APPL CON bit set. The master will then send the application confirm using aggressive
   * mode. This means the outstation will not have to challenge the confirm to verify the
   * master. 
   * This can be left set to TMWDEFS_TRUE, and all responses containing events will be 
   * "pre-challenged" or this can be set to TMWDEFS_TRUE only when a pre-challenge is
   * desired and then set to TMWDEFS_FALSE, so no more pre-challenges will be sent.
   */
  TMWTYPES_BOOL   preChallengeApplConf;

  /* If this is true, send the solicited responses with an aggressive mode object. 
   * Spec says Default User Number 1 should be used.
   */
  TMWTYPES_BOOL  authSendAggrResp;

  /* If this is true, send the event filled unsolicited responses with an aggressive mode object. 
   * This does not send initial null unsolicited aggressive mode.
   * Spec says Default User Number 1 should be used.
   */
  TMWTYPES_BOOL  authSendAggrUnsol;

  /* Pointer to authentication information structure */
  void           *pAuthenticationInfo;
   
#if SDNPCNFG_DISABLE_SELOP_SEQ_CHECK
  /*  Disable checking for consecutive application sequence numbers required between Select and Operate requests.*/
  TMWTYPES_BOOL disableSelOpSeqCheck;
#endif

#if DNPCNFG_MULTI_SESSION_REQUESTS
  /* If allowing simultaneous requests on multiple sessions on a channel */
  TMWTYPES_USHORT lastUnsolTxFragmentLength;
  TMWTYPES_UCHAR  lastUnsolTxFragment[DNPCNFG_MAX_TX_FRAGMENT_LENGTH];
#endif

#endif

#if SDNPDATA_SUPPORT_XML2
  /* 
   * If this is TMWDEFS_TRUE, the per point value, quality and timestamp
   * called dnpData in the Device Profile schema will be generated, in addition
   * to the current configuration values. SDNPDATA_SUPPORT_XML2_DNPDATA must 
   * be defined as TMWDEFS_TRUE also.
   */
  TMWTYPES_BOOL       xml2WriteDnpData;

  /* current indent size, set to zero when doing file transfer */
  TMWTYPES_UCHAR      xml2IndentSize;

  /* 
   * Current save state of XML2 device configuration 
   */
  TMWTYPES_UCHAR         xml2CurIndent;
  TMWTYPES_ULONG         xml2BufferLen;
  TMWTYPES_ULONG         xml2BufferIndex;
  SDNPXML2_SAVE_STATE    xml2BufferState;
  SDNPXML2_SAVE_STATE    xml2SaveState;
  SDNPXML2_SAVE_SUBSTATE xml2SaveSubState;
  TMWTYPES_USHORT        xml2CurSavePointNum;
  TMWTYPES_UCHAR         xml2CurSaveElemNum;

  /* buffer used for building XML output */
  TMWTYPES_CHAR         *pXml2Buffer;
  TMWTYPES_CHAR          xml2Buffer[SDNPCNFG_XML_SAVE_BUF_SIZE];
#endif

} SDNPSESN;

typedef SDNPSESN_READ_STATUS (*SDNPSESN_READ_FUNC)(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRequest,
  TMWSESN_TX_DATA *pResponse, 
  DNPUTIL_OBJECT_HEADER *pObjHeader,
  SDNPSESN_QUAL qualifier);

typedef SDNPSESN_READ_STATUS (*SDNPSESN_READ_EVENTS_FUNC)(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRequest,
  TMWSESN_TX_DATA *pResponse, 
  DNPUTIL_OBJECT_HEADER *pObjHeader, 
  TMWDEFS_CLASS_MASK classMask);

typedef TMWTYPES_BOOL (*SDNPSESN_WRITE_FUNC)(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  TMWSESN_TX_DATA *pResponse, 
  DNPUTIL_OBJECT_HEADER *pObjHeader);

typedef void (*SDNPSESN_INIT_FUNC)(
  TMWSESN *pSession);

typedef TMWTYPES_BOOL (*SDNPSESN_SELECT_FUNC)(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  TMWSESN_TX_DATA *pResponse, 
  DNPUTIL_OBJECT_HEADER *pObjHeader, 
  TMWTYPES_UCHAR numberOfPoints,
  TMWTYPES_UCHAR status);

typedef TMWTYPES_BOOL (*SDNPSESN_OPERATE_FUNC)(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  TMWSESN_TX_DATA *pResponse, 
  DNPUTIL_OBJECT_HEADER *pObjHeader, 
  TMWTYPES_UCHAR numberOfPoints,
  TMWTYPES_UCHAR status);

typedef void (*SDNPSESN_CANCEL_FUNC)(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  DNPUTIL_OBJECT_HEADER *pObjHeader);

typedef TMWTYPES_BOOL (*SDNPSESN_FREEZE_FUNC)(
  TMWSESN *pSession,
  TMWTYPES_UCHAR functionCode, 
  DNPUTIL_OBJECT_HEADER *pObjHeader,
  DNPDATA_FREEZE_TIME_DATE_FIELD timeDateEnum,
  TMWDTIME *pFreezeTime,
  TMWTYPES_ULONG freezeInterval);

typedef TMWTYPES_BOOL (*SDNPSESN_ASSIGN_CLASS_FUNC)(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  DNPUTIL_OBJECT_HEADER *pObjHeader, 
  TMWTYPES_UCHAR newClass);



#ifdef __cplusplus
extern "C" {
#endif

  /* function: sdnpsesn_getStaticReadFunc
   * purpose: Get a pointer to the function to process this 
   *  object group variation
   * arguments:
   *  group - object group
   *  variation - variation
   * returns: 
   *  Pointer to function or TMWDEFS_NULL if function is not in table.
   */
   SDNPSESN_READ_FUNC TMWDEFS_GLOBAL sdnpsesn_getStaticReadFunc(
     TMWTYPES_UCHAR group,
     TMWTYPES_UCHAR variation);

   /* function: sdnpsesn_getBinFileSessionValues
   * purpose: Read session values from a struct holding values
   * read from a binary dnp config file.
   * arguments:
   *  pSesnConfig - pointer to session config
   *  pBinFileValues - struct holding values from binary config file
   *  isChannelSerial - bool, is the channel to which the values are being applied serial
   *  xmlVersionIndex - index representing the device profile version of the source of the values (0=Jan2010; 1=July2012, 2=Nov2013, 3=Nov2014)
   * returns: 
   *  true if successfully copied values.
   */
   TMWTYPES_BOOL sdnpsesn_getBinFileSessionValues(
     SDNPSESN_CONFIG *pSesnConfig,
     DNPBNCFG_FILEVALUES *pBinFileValues,
     TMWTYPES_BOOL isChannelSerial,
     TMWTYPES_USHORT xmlVersionIndex);

   /* function: sdnpsesn_SetBroadcastFunctionCodes1_13
   * purpose: Sets the broadcast function codes as set in a binary config file.
   * arguments:
   *  pBinFileValues - pointer to struct of values read from a binary config file.
   *  enabledBroadcastFCs - flags for which broadcast FCs are enabled.
   *  enabledBroadcastWrites - extra flags for the Write function.
   * returns: 
   *  void
   */
   void  sdnpsesn_SetBroadcastFunctionCodes1_13(
  DNPBNCFG_FILEVALUES *pBinFileValues,
  TMWTYPES_ULONG *enabledBroadcastFCs,
  TMWTYPES_UCHAR *enabledBroadcastWrites);

#ifdef __cplusplus
}
#endif
#endif /* SDNPSESN_DEFINED */
