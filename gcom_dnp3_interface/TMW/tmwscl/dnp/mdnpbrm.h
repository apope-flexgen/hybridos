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

/* file: mdnpbrm.h
 * description: Define methods used to create and send DNP3 requests.
 *
 * The routines in this file are used to generate and send DNP3 requests. In
 * general the user's code will call one of these for each request they want
 * to send to a remote device. The following general notes apply to one or 
 * more of the routines below.
 * 
 * Each of the request functions defined below returns a pointer to a transmit
 * data structure. In general the contents of this structure should not be
 * used, but the pointer itself should be used to reference this request in 
 * the future. Specifically this request pointer can be used to cancel the
 * request via dnpchnl_cancelFragment or in the request's user callback to
 * indentify the specific request that the callback pertains to.
 *
 * The first argument to most of the request functions is a request descriptor
 * of type MDNPBRM_REQ_DESC. This descriptor holds generic information 
 * pertinent to the request. This structure should be initialized by a call 
 * to mdnpbrm_initReqDesc and then modified as required for the current 
 * request before being passed to the request function.
 *
 * Several of the functions accept a pointer to a user supplied transmit data
 * structure, of type DNPCHNL_TX_DATA, called pUserTxData. In most cases this
 * parameter will be set to TMWDEFS_NULL. If pUserTxData is NULL the function
 * will allocate a local transmit data structure, fill it in, send it, and 
 * return a pointer to the newly allocated structure. If pUserTxData is not 
 * NULL this routine will simply add the appropriate object header and data 
 * to the user specified transmit data structure and return without sending
 * it. This allows the user to build a request with multiple objects. In 
 * general pUserTxData should be allocated by a call to dnpchnl_newTxData
 * and initialized by calling mdnpbrm_buildRequestHeader before being passed
 * to a request function multiple times. After that dnpchnl_sendFragment should
 * be called to send it. 
 * DNPCHNL_TX_DATA *pUserTxData  = (DNPCHNL_TX_DATA*)dnpchnl_newTxData(pChannel, pSession, size, pSession->destAddress);
 * if(pUserTxData  != NULL)
 * { 
 *   mdnpbrm_buildRequestHeader(pUserTxData, DNPDEFS_FC_ACTIVATE_CONFIG);
 *   mdnpbrm_activateConfig(&request, pUserTxData, 0, pFileName1, fileName1Length, 70);
 *   mdnpbrm_activateConfig(&request, pUserTxData, 0, pFileName1, fileName2Length, 70);
 *   mdnpbrm_activateConfig(&request, pUserTxData, 0, pString, stringLength, 110);
 *   if(!dnpchnl_sendFragment((TMWSESN_TX_DATA*)pUserTxData))
 *      /. failed ./
 *      dnpchnl_freeTxData((TMWSESN_TX_DATA*)pUserTxData);
 * }
 */

#ifndef MDNPBRM_DEFINED
#define MDNPBRM_DEFINED

#include "tmwscl/utils/tmwdefs.h"
#include "tmwscl/dnp/mdnpsesn.h"
#include "tmwscl/dnp/dnpchnl.h"

#define MDNPBRM_AUTO_OPERATE_PRIORITY  254
#define MDNPBRM_AUTHENTICATE_PRIORITY  251
#define MDNPBRM_DISABLE_UNSOL_PRIORITY 250
#define MDNPBRM_CLEAR_RESTART_PRIORITY 249
#define MDNPBRM_TIMESYNC_PRIORITY      248
#define MDNPBRM_DATASET_XCHNG_PRIORITY 246
#define MDNPBRM_INTEGRITY_PRIORITY     244
#define MDNPBRM_ENABLE_UNSOL_PRIORITY  242


/* Allows adding more DOHs to an assign class request without adding another COH 
 * No COH is added at the beginning of a request to remove from class 0,
 * but for more clarity an equivalent define is being created.
 */
#define MDNPBRM_ADD_NO_NEW_COH TMWDEFS_CLASS_MASK_NOTCLASS0

/* Structure to hold parameters common to several requests. This structure 
 * should be initialized by a call to mdnpbrm_initReqDesc. Then the user 
 * should overwrite the desired fields before calling the desired mdnpbrm
 * request function.
 */
typedef struct MDNPBrmReqDescStruct{
  /* Define where this request should be sent. Only one of pChannel or
   * pSession should be set. The other should be TMWDEFS_NULL. If pChannel
   * is not TMWDEFS_NULL this request will be broadcast to all sessions
   * on that channel. If pSession is not TMWDEFS_NULL this request will
   * be sent to the specified session. Most requests will set pSession.
   */
  TMWCHNL *pChannel;
  TMWSESN *pSession;

  /* If this is a broadcast request specify the destination address to
   * use. This should be one of the following:
   *     DNPDEFS_BROADCAST_ADDR_NOCON, 
   *     DNPDEFS_BROADCAST_ADDR_CON
   *     DNPDEFS_BROADCAST_ADDR_ORIG
   */
  TMWTYPES_USHORT broadcastAddr;

  /* Request description, note that a copy is NOT made of this request so
   * it should be stored in a buffer that remains valid for the life of
   * the request
   */
  TMWTYPES_CHAR *pMsgDescription;

  /* Priority for this request. Priorities above 200 are normally reserved for
   * internal SCL use. An exception to this is when SELECT/OPERATE is performed
   * using separate BRM commands and automatic event processing is enabled for
   * the master session. See Note about this in mdnpbrm_binaryCommand
   * mdnpbrm_patternMask and mdnpbrm_analogCommand below.
   */
  TMWTYPES_UCHAR priority;

  /* Absolute response timeout, maximum amount of time, in milliseconds, 
   * that the SCL will wait for a response to this request before terminating
   * the request. This period starts as soon as the request is submitted.
   */
  TMWTYPES_MILLISECONDS responseTimeout;

  /* User callback to be called when the request, or one step in a multi step
   * request, is complete. Most of the requests generated below consist of a
   * single request/response. In this case the user callback will be called
   * with status DNPCHNL_RESP_STATUS_SUCCESS or one of the failure status 
   * codes when the response is received or the request times out. Some of 
   * the requests require multiple request/response cycles. In this case the
   * user callback will be called for each response with a status of
   * DNPCHNL_RESP_STATUS_INTERMEDIATE.
   */
  void *pUserCallbackParam;
  DNPCHNL_CALLBACK_FUNC pUserCallback;

#if MDNPDATA_SUPPORT_OBJ120
  /* Secure authentication */
  /* Send objectGroup 120 variation 3 and variation 9 agressive mode objects in 
   * request 
   */
  TMWTYPES_BOOL  authAggressiveMode;

  /* User number if aggressive mode is selected or if this request is challenged */
  TMWTYPES_USHORT authUserNumber;
#endif

} MDNPBRM_REQ_DESC;

/* Define 'automatic' processing to be done by the SCL for the various 
 * 'command' requests. If the corresponding bit is set in an 'autoMode' 
 * parameter to one of the control routines then the specified operation 
 * will be performed as required. Specifically, if MDNPBRM_AUTO_MODE_OPERATE
 * is set than an operate request will be performed when a successful
 * select completes. If MDNPBRM_AUTO_MODE_FEEDBACK is set an operate 
 * feedback poll will be performed when an operate completes.
 */
typedef TMWTYPES_UCHAR MDNPBRM_AUTO_MODE;
#define MDNPBRM_AUTO_MODE_NONE      0x00
#define MDNPBRM_AUTO_MODE_OPERATE   0x01
#define MDNPBRM_AUTO_MODE_FEEDBACK  0x02

/* Define structure to hold information for a single Control Relay 
 * Output Block. An array of these structures will be passed to 
 * mdnpbrm_binaryCommand to generate a single request with multiple 
 * CROB objects in it.
 */
typedef struct MDNPBrmCROBInfo {
  TMWTYPES_USHORT pointNumber;

  /*  
   * Control Relay Output Block. 
   *  control - control code
   *    low 4 bits is Op Type  
   *      DNPDEFS_CROB_CTRL_NUL, DNPDEFS_CROB_CTRL_PULSE_ON, DNPDEFS_CROB_CTRL_PULSE_OFF,
   *      DNPDEFS_CROB_CTRL_LATCH_ON, or DNPDEFS_CROB_CTRL_LATCH_OFF
   *    Then the Queue Field which is obsolete
   *    Then the Clear Field DNPDEFS_CROB_CTRL_CLEAR     
   *    Then the two bit Trip-Close field  0, or DNPDEFS_CROB_CTRL_PAIRED_CLOSE or DNPDEFS_CROB_CTRL_PAIRED_TRIP  
   *  count - number of times outstation shall execute the operation
   */ 
  DNPDEFS_CROB_CTRL control; 
  TMWTYPES_ULONG onTime;
  TMWTYPES_ULONG offTime;

/* MDNPDATA_SUPPORT_OBJ12_COUNT must be set to TMWDEFS_TRUE to support the count 
 * field 
 */
#if MDNPDATA_SUPPORT_OBJ12_COUNT
   /*  count - number of times outstation shall execute the operation */
  TMWTYPES_UCHAR count;
#endif

} MDNPBRM_CROB_INFO;

/* Define structure to hold information for a single Analog 
 * Output Block. An array of these structures will be passed to 
 * mdnpbrm_analogCommand to generate a single request with multiple 
 * analog output objects in it.
 */
typedef struct MDNPBrmAnalogInfo {
  TMWTYPES_USHORT pointNumber;
  TMWTYPES_ANALOG_VALUE value; 
} MDNPBRM_ANALOG_INFO;

/* Define structure to hold information for a single indexed absolute time
 * and long interval object. An array of these structures will be passed to
 * mdnpbrm_writeIndexedTime to generate a single request with multiple
 * indexed time objects in it.
 */
typedef struct MDNPBrmIndexedTimeInfo {
  TMWTYPES_USHORT pointNumber;
  TMWDTIME indexedTime; /* UTC Time */
  TMWTYPES_ULONG intervalCount;
  TMWTYPES_BYTE intervalUnits;
} MDNPBRM_INDEXEDTIME_INFO;

/* Define structure to hold information for a single string 
 * Output Block. An array of these structures will be passed to 
 * mdnpbrm_writeStrings to generate a single request with multiple 
 * string objects in it.
 */
typedef struct MDNPBrmStringInfo {
  TMWTYPES_USHORT pointNumber;
  TMWTYPES_CHAR  value[DNPDEFS_MAX_STRING_LENGTH]; 
} MDNPBRM_STRING_INFO;

/* Define structure to hold information for a single extended string 
 * Output Block. An array of these structures will be passed to 
 * mdnpbrm_writeExtStrings to generate a single request with multiple 
 * extended string objects in it.
 */
typedef struct MDNPBrmExtStringInfo {
  TMWTYPES_USHORT pointNumber;
  TMWTYPES_CHAR  value[DNPCNFG_MAX_EXT_STRING_LENGTH]; 
} MDNPBRM_EXT_STRING_INFO;

/* Internal structure used to hold callback information for a select, 
 * operate, feedback sequence of requests.
 */
typedef struct {
  TMWSESN *pSession;

  /* original tx data, to be reused for followup requests */
  DNPCHNL_TX_DATA *pTxData;

  TMWTYPES_BOOL operateRequired;
  TMWTYPES_BOOL binFeedbackRequired; /* binary output or counter feedback */
  TMWTYPES_BOOL anlgFeedbackRequired;
  TMWTYPES_MILLISECONDS feedbackDelay;
  TMWTIMER feedbackTimer;

} OperateCallbackData;

/* Data types for exchanging data set prototype and descriptor objects
 * between master and slave (outstation)
 */
typedef enum  {
  MDNPBRM_DATASET_READ_PROTO,
  MDNPBRM_DATASET_DELAY_PROTO,
  MDNPBRM_DATASET_WRITE_PROTO,
  MDNPBRM_DATASET_READ_DESCR,
  MDNPBRM_DATASET_DELAY_DESCR,
  MDNPBRM_DATASET_WRITE_DESCR,
  MDNPBRM_DATASET_OPERATE,
  MDNPBRM_DATASET_COMPLETE
} MDNPBRM_DATASET_OPER;

typedef struct _mdnpDataSetXferContext {
  MDNPBRM_DATASET_OPER  nextOperation; 
  DNPCHNL_TX_DATA      *pTxData;
  TMWTYPES_USHORT       numberProtosDefinedOnSlave;
  TMWTYPES_USHORT       numberDescrsDefinedOnSlave;
  TMWTIMER              delayTimer;
} MDNPBRM_DATSET_XFER_CONTEXT;

#ifdef __cplusplus
extern "C" {
#endif

  /* function: mdnpbrm_initBroadcastDesc
   * purpose: Initialize a request descriptor for a broadcast
   *  request.
   * arguments:
   *  pReqDesc - request descriptor to initialize
   *  pChannel - channel to broadcast this request on
   *  destAddr - destination address to use in request. 
   *    Must be one of: 
   *     DNPDEFS_BROADCAST_ADDR_NOCON, 
   *     DNPDEFS_BROADCAST_ADDR_CON
   *     DNPDEFS_BROADCAST_ADDR_ORIG
   * returns:
   *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
   */
  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpbrm_initBroadcastDesc(
    MDNPBRM_REQ_DESC *pReqDesc,
    TMWCHNL *pChannel,
    TMWTYPES_USHORT destAddr);

  /* function: mdnpbrm_initReqDesc
   * purpose: Initialize a request descriptor
   * arguments:
   *  pReqDesc - request descriptor to initialize
   *  pSession - session this request will be sent to
   * returns:
   *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
   */
  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpbrm_initReqDesc(
    MDNPBRM_REQ_DESC *pReqDesc,
    TMWSESN *pSession);

  /* function: mdnpbrm_buildRequestHeader
   * purpose: Initialize request application header
   * arguments:
   *  pTxData - tranmsit data structure to initialize
   *  funcCode - function code for request
   * returns:
   *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
   */
  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpbrm_buildRequestHeader(
    DNPCHNL_TX_DATA *pTxData, 
    TMWTYPES_UCHAR funcCode);

  /* function: mdnpbrm_addObjectHeader
   * purpose: Add an object header to the specified message
   * arguments:
   *  pTxData - transmit data structure to add object header to
   *  group - object group
   *  variation - object variation
   *  qualifier - address qualifier
   *  start - start address if qualifier is 8 or 16 bit start/stop
   *  stopOrQty - stop address if qualifier is 8 or 16 bit start/stop
   *   or quantity if qualifier is 8 or 16 bit limited or indexed.
   * returns:
   *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
   */
  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpbrm_addObjectHeader(
    DNPCHNL_TX_DATA *pTxData,
    TMWTYPES_UCHAR group, 
    TMWTYPES_UCHAR variation, 
    TMWTYPES_UCHAR qualifier, 
    TMWTYPES_USHORT start, 
    TMWTYPES_USHORT stopOrQty);

  /* function: mdnpbrm_addObjectData
   * purpose: Add an object data to the specified message
   * arguments:
   *  pTxData - transmit data structure to add data to
   *  len - number of bytes to add
   *  pData - pointer to binary data to add to request
   * returns:
   *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
   */
  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpbrm_addObjectData(
    DNPCHNL_TX_DATA *pTxData,
    TMWTYPES_USHORT len,
    TMWTYPES_UCHAR *pData);

  /* function: mdnpbrm_cancelSelOpDelayTimer
   * purpose:
   *  If the application is going to be delayed sending the OPERATE command (for 
   *  example waiting for human input) you can configure a nonzero mdnp session  
   *  selOpDelayTime to delay the sending of other commands until the OPERATE is sent
   *  or the timeout is reached. This function will cancel that timer if it is running.
   * arguments:
   *  pSession - pointer to a master DNP session
   * returns:
   *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE 
   */
  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpbrm_cancelSelOpDelayTimer(
    TMWSESN *pSession);

  /* function: mdnpbrm_readClass
   * purpose: Issue an read class request to the specified session
   * arguments:
   *  pReqDesc - request descriptor containing generic parameters for
   *   this request
   *  pUserTxData - pointer to user transmit data structure see description
   *   above
   *  qualifier - address qualifier to use, must be all or 8/16 bit
   *   limited quantity.
   *  maxQuantity - quantity to request if qualifier is 8/16 bit limited
   *   quantity.
   *  class0 - TMWDEFS_TRUE to request class 0 (static data)
   *  class1 - TMWDEFS_TRUE to request class 1 event data
   *  class2 - TMWDEFS_TRUE to request class 2 event data
   *  class3 - TMWDEFS_TRUE to request class 3 event data
   * returns:
   *  Pointer to request transmit data structure or TMWDEFS_NULL
   */
  TMWDEFS_SCL_API TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_readClass(
    MDNPBRM_REQ_DESC *pReqDesc,
    DNPCHNL_TX_DATA *pUserTxData,
    TMWTYPES_UCHAR qualifier, 
    TMWTYPES_USHORT maxQuantity,
    TMWTYPES_BOOL class0, 
    TMWTYPES_BOOL class1, 
    TMWTYPES_BOOL class2, 
    TMWTYPES_BOOL class3);

  /* function: mdnpbrm_integrityPoll
   * purpose: Issue an integrity data poll to the specified session
   * arguments:
   *  pReqDesc - request descriptor containing generic parameters for
   *   this request
   * returns:
   *  Pointer to request transmit data structure or TMWDEFS_NULL
   */
  TMWDEFS_SCL_API TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_integrityPoll(
    MDNPBRM_REQ_DESC *pReqDesc);

  /* function: mdnpbrm_eventPoll
   * purpose: Issue an event data poll to the specified session
   * arguments:
   *  pReqDesc - request descriptor containing generic parameters for
   *   this request
   * returns:
   *  Pointer to request transmit data structure or TMWDEFS_NULL
   */
  TMWDEFS_SCL_API TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_eventPoll(
    MDNPBRM_REQ_DESC *pReqDesc);

  /* function: mdnpbrm_binOutFeedbackPoll
   * purpose: Issue an operate feedback request to the specified session.
   *  This will send a read for class 1 2 and 3 events and
   *   binary input status values.
   * arguments:
   *  pReqDesc - request descriptor containing generic parameters for
   *   this request
   * returns:
   *  Pointer to request transmit data structure or TMWDEFS_NULL
   */
  TMWDEFS_SCL_API TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_binOutFeedbackPoll(
    MDNPBRM_REQ_DESC *pReqDesc); 
    
  /* function: mdnpbrm_anlgOutFeedbackPoll
   * purpose: Issue an operate feedback request to the specified session.
   *  This will send a read for class 1 2 and 3 events and
   *  analog input status values.
   * arguments:
   *  pReqDesc - request descriptor containing generic parameters for
   *   this request 
   * returns:
   *  Pointer to request transmit data structure or TMWDEFS_NULL
   */
  TMWDEFS_SCL_API TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_anlgOutFeedbackPoll(
    MDNPBRM_REQ_DESC *pReqDesc);

  /* function: mdnpbrm_frznCntrFeedbackPoll
   * purpose: Issue an operate feedback request to the specified session.
   *  This will send a read for class 1 2 and 3 events,
   *  and frozen counter values
   * arguments:
   *  pReqDesc - request descriptor containing generic parameters for
   *   this request 
   * returns:
   *  Pointer to request transmit data structure or TMWDEFS_NULL
   */
  TMWDEFS_SCL_API TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_frznCntrFeedbackPoll(
    MDNPBRM_REQ_DESC *pReqDesc);

  /* function: mdnpbrm_frznAnlgInFeedbackPoll
   * purpose: Issue an operate feedback request to the specified session.
   *  This will send a read for class 1 2 and 3 events,
   *  and frozen analog input values
   * arguments:
   *  pReqDesc - request descriptor containing generic parameters for
   *   this request 
   * returns:
   *  Pointer to request transmit data structure or TMWDEFS_NULL
   */
  TMWDEFS_SCL_API TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_frznAnlgInFeedbackPoll(
    MDNPBRM_REQ_DESC *pReqDesc);

  /* function: mdnpbrm_clearRestart
   * purpose: Issue a clear restart request to the specified session
   * arguments:
   *  pReqDesc - request descriptor containing generic parameters for
   *   this request
   * returns:
   *  Pointer to request transmit data structure or TMWDEFS_NULL
   */
  TMWDEFS_SCL_API TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_clearRestart(
    MDNPBRM_REQ_DESC *pReqDesc);

  /* function: mdnpbrm_clearNeedTime
   * purpose: Issue a clear Need Time IIN bit request to the specified session
   * arguments:
   *  pReqDesc - request descriptor containing generic parameters for
   *   this request
   * returns:
   *  Pointer to request transmit data structure or TMWDEFS_NULL
   */
  TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_clearNeedTime(
    MDNPBRM_REQ_DESC *pReqDesc);

  /* function: mdnpbrm_clearIINBit
   * purpose: Issue a clear specified IIN bit write request
   * arguments:
   *  pReqDesc - request descriptor containing generic parameters for
   *   this request
   *  index - index of the IIN bit to clear. 
   *    DNPDEFS_IIN_NEEDTIME_INDEX  4
   *    DNPDEFS_IIN_RESTART_INDEX   7
   * returns:
   *  Pointer to request transmit data structure or TMWDEFS_NULL
   */
  TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_clearIINBit(
    MDNPBRM_REQ_DESC *pReqDesc,
    TMWTYPES_UCHAR index);

  /* function: mdnpbrm_coldRestart
   * purpose: Issue a cold restart request to the specified session
   * arguments:
   *  pReqDesc - request descriptor containing generic parameters for
   *   this request
   * returns:
   *  Pointer to request transmit data structure or TMWDEFS_NULL
   */
  TMWDEFS_SCL_API TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_coldRestart(
    MDNPBRM_REQ_DESC *pReqDesc);

  /* function: mdnpbrm_warmRestart
   * purpose: Issue a warm restart request to the specified session
   * arguments:
   *  pReqDesc - request descriptor containing generic parameters for
   *   this request
   * returns:
   *  Pointer to request transmit data structure or TMWDEFS_NULL
   */
  TMWDEFS_SCL_API TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_warmRestart(
    MDNPBRM_REQ_DESC *pReqDesc);

#if MDNPDATA_SUPPORT_OBJ50_V1
  /* function: mdnpbrm_delayMeasurement
   * purpose: Issue an delay measurement request to the specified session.
   *  This is an optional step in a time synchronization, typically only
   *  used for serial channels.
   * arguments:
   *  pReqDesc - request descriptor containing generic parameters for
   *   this request
   * returns:
   *  Pointer to request transmit data structure or TMWDEFS_NULL
   */
  TMWDEFS_SCL_API TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_delayMeasurement(
    MDNPBRM_REQ_DESC *pReqDesc);

  /* function: mdnpbrm_readTime
   * purpose: Issue an read time request to the specified session
   * arguments:
   *  pReqDesc - request descriptor containing generic parameters for
   *   this request
   * returns:
   *  Pointer to request transmit data structure or TMWDEFS_NULL
   */
  TMWDEFS_SCL_API TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_readTime(
    MDNPBRM_REQ_DESC *pReqDesc);

  /* function: mdnpbrm_writeTime
   * purpose: Issue an write time request to the specified session
   *  This sets the time on the remote device, this request is typically 
   *  only used for serial channels.
   * arguments:
   *  pReqDesc - request descriptor containing generic parameters for
   *   this request
   * returns:
   *  Pointer to request transmit data structure or TMWDEFS_NULL
   */
  TMWDEFS_SCL_API TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_writeTime(
    MDNPBRM_REQ_DESC *pReqDesc);
#endif

#if MDNPDATA_SUPPORT_OBJ50_V3
  /* function: mdnpbrm_recordCurrentTime
   * purpose: Issue an record current time request to the specified session.
   *  This is generally the first step in a time synchronization on a LAN
   *  channel.
   * arguments:
   *  pReqDesc - request descriptor containing generic parameters for
   *   this request
   * returns:
   *  Pointer to request transmit data structure or TMWDEFS_NULL
   */
  TMWDEFS_SCL_API TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_recordCurrentTime(
    MDNPBRM_REQ_DESC *pReqDesc);

  /* function: mdnpbrm_writeRecordedTime
   * purpose: Issue an write time request to the specified session
   *  This is generally the second step in a time synchronization on a LAN
   *  channel.
   * arguments:
   *  pReqDesc - request descriptor containing generic parameters for
   *   this request
   * returns:
   *  Pointer to request transmit data structure or TMWDEFS_NULL
   */
  TMWDEFS_SCL_API TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_writeRecordedTime(
    MDNPBRM_REQ_DESC *pReqDesc);
#endif

#if MDNPDATA_SUPPORT_OBJ50_V1 || MDNPDATA_SUPPORT_OBJ50_V3
  /* Define enum which specifies the type of time sync required */
  typedef enum {
    MDNPBRM_SYNC_TYPE_SERIAL,
    MDNPBRM_SYNC_TYPE_LAN
  } MDNPBRM_SYNC_TYPE;

  /* function: mdnpbrm_timeSync 
   * purpose: Synchronize time on a remote DNP3 device. DNP3 supports two
   *  mechanisms for time synchronization, one for standard serial channels
   *  and another for LAN applications. Each of these mechanisms may require
   *  multiple request/response operations. This routine will perform all
   *  the operations required to perform a time synchronization as specified.
   * arguments:
   *  pReqDesc - request descriptor containing generic parameters for
   *   this request.
   *  type - type of time synchronization to perform, serial or lan.
   *  measureDelay - TMWDEFS_TRUE to perform a delay measurement before
   *   the write time request.
   * returns:
   *  Pointer to request transmit data structure or TMWDEFS_NULL
   */
  TMWDEFS_SCL_API TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_timeSync(
    MDNPBRM_REQ_DESC *pReqDesc,
    MDNPBRM_SYNC_TYPE type,
    TMWTYPES_BOOL measureDelay);
#endif

#if MDNPDATA_SUPPORT_OBJ50_V4
  /* function: mdnpbrm_writeIndexedTime
   * purpose: Issue an write indexed absoluted time request to the specified session
   *  It is used to specify the initial time of an action and the time between 
   *  subsequent, regular repetitions of the actions or activities.
   * arguments:
   *  pReqDesc - request descriptor containing generic parameters for
   *   this request
   * returns:
   *  Pointer to request transmit data structure or TMWDEFS_NULL
   */
  TMWDEFS_SCL_API TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_writeIndexedTime(
    MDNPBRM_REQ_DESC *pReqDesc,
    DNPCHNL_TX_DATA *pUserTxData,
    TMWTYPES_UCHAR qualifier,
    TMWTYPES_UCHAR numIndexedTime,
    MDNPBRM_INDEXEDTIME_INFO *pINDEXEDTIMEInfo);
#endif

  /* function: mdnpbrm_unsolEnable
   * purpose: Issue an unsolicited enable request to the specified 
   *  session
   * arguments:
   *  pReqDesc - request descriptor containing generic parameters for
   *   this request
   *  class1 - TMWDEFS_TRUE to enable unsolicited responses for class 1 events
   *  class2 - TMWDEFS_TRUE to enable unsolicited responses for class 2 events
   *  class3 - TMWDEFS_TRUE to enable unsolicited responses for class 3 events
   * returns:
   *  Pointer to request transmit data structure or TMWDEFS_NULL
   */
  TMWDEFS_SCL_API TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_unsolEnable(
    MDNPBRM_REQ_DESC *pReqDesc,
    TMWTYPES_BOOL class1, 
    TMWTYPES_BOOL class2, 
    TMWTYPES_BOOL class3);

  /* function: mdnpbrm_unsolDisable
   * purpose: Issue an unsolicited disable request to the specified 
   *  session
   * arguments:
   *  pReqDesc - request descriptor containing generic parameters for
   *   this request
   *  class1 - TMWDEFS_TRUE to disable unsolicited responses for class 1 events
   *  class2 - TMWDEFS_TRUE to disable unsolicited responses for class 2 events
   *  class3 - TMWDEFS_TRUE to disable unsolicited responses for class 3 events
   * returns:
   *  Pointer to request transmit data structure or TMWDEFS_NULL
   */
  TMWDEFS_SCL_API TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_unsolDisable(
    MDNPBRM_REQ_DESC *pReqDesc,
    TMWTYPES_BOOL class1, 
    TMWTYPES_BOOL class2, 
    TMWTYPES_BOOL class3);

  /* function: mdnpbrm_assignClass
   * purpose: Issue an assign class request to the specified session
   * arguments:
   *  pReqDesc - request descriptor containing generic parameters for
   *   this request
   *  pUserTxData - pointer to user transmit data structure see description
   *   above
   *  group - object group to assign class
   *  classMask - class to assign group to. This puts a Class Object Header (COH)
   *   in a request and all Data Object Headers (DOH) immediately following that 
   *   are assigned to that class.
   *  TMWDEFS_CLASS_MASK_ONE   - assign to event class 1
   *  TMWDEFS_CLASS_MASK_TWO   - assign to event class 2
   *  TMWDEFS_CLASS_MASK_THREE - assign to event class 3
   *  TMWDEFS_CLASS_MASK_NONE  - assign to no event class
   *  TMWDEFS_CLASS_MASK_NOTCLASS0 - remove from static class 0. This adds a "NULL"
   *   COH which indicates NOTCLASS0 and if used must be the first in the request. 
   *  MDNPBRM_ADD_NO_NEW_COH   - To add more DOHs/groups to this request using the existing
   *   last COH to specify the class for this group.
   *  qualifier - address qualfier, must be all or 8/16 bit start/stop
   *   or 8/16 bit limited quantity or 8/16 bit indexed
   *  start - start if qualifier is 8/16 bit start/stop
   *  stopOrQty - stop if qualifier is 8/16 bit start/stop or quantity
   *   if qualifier is 8/16 bit limited quantity
   *  pPoints - pointer to array of point numbers to read if 8/16 bit indexed
   * returns:
   *  Pointer to request transmit data structure or TMWDEFS_NULL
   */
  TMWDEFS_SCL_API TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_assignClass(
    MDNPBRM_REQ_DESC *pReqDesc,
    DNPCHNL_TX_DATA *pUserTxData,
    TMWDEFS_CLASS_MASK classMask, 
    TMWTYPES_UCHAR group, 
    TMWTYPES_UCHAR qualifier, 
    TMWTYPES_USHORT start, 
    TMWTYPES_USHORT stopOrQty,
    TMWTYPES_USHORT *pPoints);

  /* function: mdnpbrm_readGroup
   * purpose: Issue a read group request to the specified session
   * arguments:
   *  pReqDesc - request descriptor containing generic parameters for
   *   this request
   *  pUserTxData - pointer to user transmit data structure see description
   *   above
   *  group - object group for read request
   *  variation - object variation to read. The type specified by this is 
   *   specific to the object group being read.
   *  qualifier - address qualifier, must be all, 8/16 bit limited quantity,
   *   or 8/16 bit start/stop
   *  start - start point if qualifier is start/stop
   *  stopOrQty - stop point if qualifier is start/stop or quantity if
   *   qualifier is limited points.
   * returns:
   *  Pointer to request transmit data structure or TMWDEFS_NULL
   */
  TMWDEFS_SCL_API TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_readGroup(
    MDNPBRM_REQ_DESC *pReqDesc,
    DNPCHNL_TX_DATA *pUserTxData,
    TMWTYPES_UCHAR group, 
    TMWTYPES_UCHAR variation, 
    TMWTYPES_UCHAR qualifier,
    TMWTYPES_USHORT start,
    TMWTYPES_USHORT stopOrQty);

  /* function: mdnpbrm_readPoints
   * purpose: Issue an read one or more specific data points request to the
   *  specified session. Note that this request uses indexed addressing which
   *  is not required by any of the DNP3 subset level definitions and is not
   *  supported by a significant percentage of DNP3 slave devices.
   * arguments:
   *  pReqDesc - request descriptor containing generic parameters for
   *   this request
   *  pUserTxData - pointer to user transmit data structure see description
   *   above
   *  group - object group for read request
   *  variation - object variation to read
   *  qualifier - address qualifier, must be 8/16 bit indexed
   *  DNPDEFS_QUAL_8BIT_INDEX or DNPDEFS_QUAL_16BIT_INDEX
   *  pPoints - pointer to array of point numbers to read
   *  numPoints - number of points to read
   * returns:
   *  Pointer to request transmit data structure or TMWDEFS_NULL
   */
  TMWDEFS_SCL_API TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_readPoints(
    MDNPBRM_REQ_DESC *pReqDesc,
    DNPCHNL_TX_DATA *pUserTxData,
    TMWTYPES_UCHAR group, 
    TMWTYPES_UCHAR variation, 
    TMWTYPES_UCHAR qualifier,
    TMWTYPES_USHORT *pPoints,
    TMWTYPES_USHORT numPoints);

  /* function: mdnpbrm_binaryOutWrite
   * purpose: Issue a binary output write command to the specified session
   * arguments:
   *  pReqDesc - request descriptor containing generic parameters for
   *   this request
   *  pUserTxData - pointer to user transmit data structure see description
   *   above
   *  qualifier - address qualifier, must be 8 or 16 bit startstop
   *    DNPDEFS_QUAL_8BIT_START_STOP or DNPDEFS_QUAL_16BIT_START_STOP
   *  start - starting point index
   *  stop - ending point index
   *  pValues - pointer to array of values, 0 for off, 1 for on
   *   Status bits are not sent in a write request.
   * returns:
   *  Pointer to request transmit data structure or TMWDEFS_NULL
   */
  TMWDEFS_SCL_API TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_binaryOutWrite(
    MDNPBRM_REQ_DESC *pReqDesc,
    DNPCHNL_TX_DATA *pUserTxData,
    TMWTYPES_UCHAR qualifier,
    TMWTYPES_USHORT start,
    TMWTYPES_USHORT stop,
    TMWTYPES_UCHAR *pValues);

  /* function: mdnpbrm_binaryCommand
   * purpose: Issue a binary command to the specified session
   * arguments:
   *  pReqDesc - request descriptor containing generic parameters for
   *   this request
   *  pUserTxData - pointer to user transmit data structure see description
   *   above
   *  funcCode - function code, must be DNPDEFS_FC_SELECT, DNPDEFS_FC_OPERATE,
   *   DNPDEFS_FC_DIRECT_OP, or DNPDEFS_FC_DIRECT_OP_NOACK
   *  autoMode - bit mask to specify which actions to perform internally within 
   *   the SCL. See definition of MDNPBRM_AUTO_MODE above.
   *  opFeedbackDelay - specify a delay, in milliseconds, after receiving 
   *   the response to an operate or direct operate request before issueing
   *   an operate feedback poll using the autoMode parameter.
   *  qualifier - address qualifier, must be 8 or 16 bit indexed
   *   DNPDEFS_QUAL_8BIT_INDEX or DNPDEFS_QUAL_16BIT_INDEX
   *  numCROBs - number of Control Relay Output Blocks in this request
   *  pCROBInfo - data for each Control Relay Output Block
   * returns:
   *  Pointer to request transmit data structure or TMWDEFS_NULL
   *
   * NOTE: If you are doing a SELECT/OPERATE but choose not to use the autoMode
   *  mask to do this, but instead are going to call this brm function to send 
   *  the SELECT and then call this function again to send the OPERATE, you will
   *  need to increase the pReqDesc->priority of the OPERATE to 255 to 
   *  guarantee it gets sent before any automatic processing of events is performed. 
   *  (Automatic processing of events is enabled by default when the mdnp session 
   *  was opened). For example, if in the response to the SELECT the restart IIN bit  
   *  is set and the session was set up to automatically send a clear restart
   *  command, you will want to make sure the OPERATE gets sent before the clear 
   *  restart, otherwise the operate will fail.
   */
  TMWDEFS_SCL_API TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_binaryCommand(
    MDNPBRM_REQ_DESC *pReqDesc,
    DNPCHNL_TX_DATA *pUserTxData,
    TMWTYPES_UCHAR funcCode,
    MDNPBRM_AUTO_MODE autoMode,
    TMWTYPES_MILLISECONDS opFeedbackDelay,
    TMWTYPES_UCHAR qualifier,
    TMWTYPES_UCHAR numCROBs,
    MDNPBRM_CROB_INFO *pCROBInfo);

  /* function: mdnpbrm_patternMask
   * purpose: Send a pattern mask to remote device
   * arguments:
   *  pReqDesc - request descriptor containing generic parameters for
   *   this request
   *  funcCode - function code, DNPDEFS_FC_SELECT, DNPDEFS_FC_OPERATE,
   *   DNPDEFS_FC_DIRECT_OP, or DNPDEFS_FC_DIRECT_OP_NOACK
   *  autoMode - bit mask to specify which actions to perform internally within 
   *   the SCL. See definition of MDNPBRM_AUTO_MODE above.
   *  opFeedbackDelay - specify a delay, in milliseconds, after receiving 
   *   the response to an operate or direct operate request before issueing
   *   an operate feedback poll using the autoMode parameter.
   *  qualifier - address qualifier, must be 8 or 16 bit start/stop
   *    DNPDEFS_QUAL_8BIT_START_STOP or DNPDEFS_QUAL_16BIT_START_STOP
   *  start - first point number for mask
   *  stop - last point number for mask
   *  control - control code
   *    low 4 bits is Op Type  
   *      DNPDEFS_CROB_CTRL_NUL, DNPDEFS_CROB_CTRL_PULSE_ON, DNPDEFS_CROB_CTRL_PULSE_OFF,
   *      DNPDEFS_CROB_CTRL_LATCH_ON, or DNPDEFS_CROB_CTRL_LATCH_OFF
   *    Then the Queue Field which is obsolete
   *    Then the Clear Field DNPDEFS_CROB_CTRL_CLEAR     
   *    Then the two bit Trip-Close field  0, or DNPDEFS_CROB_CTRL_PAIRED_CLOSE or DNPDEFS_CROB_CTRL_PAIRED_TRIP  
   *  count - number of times outstation shall execute the operation
   *  activationPeriod - pulse activation time
   *  deactivationPeriod - pulse deactivation time
   *  pMask - bitmask indicating which points in the range specified by start->stop are to be controlled.
   *   bit value of 1 means that point should be controlled.
   *   Example: funcCode=DNPDEFS_FC_DIRECT_OP, start==2, stop==17, 
   *              control=DNPDEFS_CROB_CTRL_LATCH_ON, *pMask=0x81,0x10; 
   *            sends a latch on to points 2, 9 and 14    
   * returns:
   *  Pointer to request transmit data structure or TMWDEFS_NULL   
   *
   * NOTE: If you are doing a SELECT/OPERATE but choose not to use the autoMode
   *  mask to do this, but instead are going to call this brm function to send 
   *  the SELECT and then call this function again to send the OPERATE, you will
   *  need to increase the pReqDesc->priority of the OPERATE to 255 to 
   *  guarantee it gets sent before any automatic processing of events is performed.
   *  (Automatic processing of events is enabled by default when the mdnp session 
   *  was opened). For example, if in the response to the SELECT the restart IIN bit  
   *  is set and the session was set up to automatically send a clear restart
   *  command, you will want to make sure the OPERATE gets sent before the clear 
   *  restart, otherwise the operate will fail.
   */
  TMWDEFS_SCL_API TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_patternMask(
    MDNPBRM_REQ_DESC *pReqDesc,
    TMWTYPES_UCHAR funcCode, 
    MDNPBRM_AUTO_MODE autoMode, 
    TMWTYPES_MILLISECONDS opFeedbackDelay,
    TMWTYPES_UCHAR qualifier,
    TMWTYPES_USHORT start,
    TMWTYPES_USHORT stop,
    TMWTYPES_UCHAR control,
    TMWTYPES_UCHAR count,
    TMWTYPES_ULONG activationPeriod,
    TMWTYPES_ULONG deactivationPeriod,
    TMWTYPES_UCHAR *pMask);

  /* function: mdnpbrm_analogCommand
   * purpose: Issue an analog command containing object group 41 to the
   *   specified session
   * arguments:
   *  pReqDesc - request descriptor containing generic parameters for
   *   this request
   *  pUserTxData - pointer to user transmit data structure see description
   *   above
   *  funcCode - function code, must be DNPDEFS_FC_SELECT, DNPDEFS_FC_OPERATE,
   *   DNPDEFS_FC_DIRECT_OP, or DNPDEFS_FC_DIRECT_OP_NOACK
   *  autoMode - bit mask to specify which actions to perform internally within 
   *   the SCL. See definition of MDNPBRM_AUTO_MODE above.
   *  opFeedbackDelay - specify a delay, in milliseconds, after receiving 
   *   the response to an operate or direct operate request before issueing
   *   an operate feedback poll using the autoMode parameter.
   *  qualifier - address qualifier, must be 8 or 16 bit indexed
   *   DNPDEFS_QUAL_8BIT_INDEX or DNPDEFS_QUAL_16BIT_INDEX
   *  variation - object variation to use in request, must be 1, 2, 3 or 4
   *   This will indicate what type value will be sent to the outstation.
   *   1 - 32 bit                           (this was added in subset level 3)
   *   2 - 16 bit                           (this is in subset level 1)
   *   3 - Single Precision Floating Point  (this was added in subset level 4)
   *   4 - Double Precision Floating Point  (this is beyond subset level 4 )
   *  numObjects - number of Analog Output Blocks in this request
   *  pAnlgInfo - data for each Analog Output Block
   * returns:
   *  Pointer to request transmit data structure or TMWDEFS_NULL
   *
   * NOTE: If you are doing a SELECT/OPERATE but choose not to use the autoMode
   *  mask to do this, but instead are going to call this brm function to send 
   *  the SELECT and then call this function again to send the OPERATE, you will
   *  need to increase the pReqDesc->priority of the OPERATE to 255 to 
   *  guarantee it gets sent before any automatic processing of events is performed. 
   *  (Automatic processing of events is enabled by default when the mdnp session 
   *  was opened). For example, if in the response to the SELECT the restart IIN bit  
   *  is set and the session was set up to automatically send a clear restart
   *  command, you will want to make sure the OPERATE gets sent before the clear 
   *  restart, otherwise the operate will fail.
   */
  TMWDEFS_SCL_API TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_analogCommand(
    MDNPBRM_REQ_DESC *pReqDesc,
    DNPCHNL_TX_DATA *pUserTxData,
    TMWTYPES_UCHAR funcCode, 
    MDNPBRM_AUTO_MODE autoMode, 
    TMWTYPES_MILLISECONDS opFeedbackDelay,
    TMWTYPES_UCHAR qualifier, 
    TMWTYPES_UCHAR variation,
    TMWTYPES_UCHAR numObjects,
    MDNPBRM_ANALOG_INFO *pAnlgInfo);

  /* function: mdnpbrm_writeDeadband
   * purpose: Issue a write request to Object 34 (Analog Input Deadbands)
   * arguments:
   *  pReqDesc - request descriptor containing generic parameters for
   *   this request
   *  pUserTxData - pointer to user transmit data structure see description
   *   above
   *  qualifier - address qualifier, must be 8/16 bit indexed or 8/16 bit start/stop
   *  variation - object variation to use in request
   *   1 - 16 bit                           (this was added in subset level 4)
   *   2 - 32 bit                           (this was added in subset level 4)
   *   3 - Single Precision Floating Point  (this was added in subset level 4)
   *  numObjects - number of deadbands in this request
   *  pAnlgInfo - data for each deadband
   * returns:
   *  Pointer to request transmit data structure or TMWDEFS_NULL
   */
  TMWDEFS_SCL_API TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_writeDeadband(
    MDNPBRM_REQ_DESC *pReqDesc,
    DNPCHNL_TX_DATA *pUserTxData,
    TMWTYPES_UCHAR qualifier,
    TMWTYPES_UCHAR variation,
    TMWTYPES_UCHAR numObjects,
    MDNPBRM_ANALOG_INFO *pAnlgInfo);

  /* function: mdnpbrm_freezeCounters
   * purpose: Issue an freeze counters request to the specified session
   * arguments:
   *  pReqDesc - request descriptor containing generic parameters for
   *   this request
   *  pUserTxData - pointer to user transmit data structure see description
   *   above
   *  clear - clear counters after freeze
   *  noAck - don't ask for application layer response
   *  qualifier - address qualifier, must be all,
   *   or 8/16 bit start/stop
   *  start - start point if qualifier is 8/16 bit start/stop
   *  stopOrQty - stop point if qualifier is 8/16 bit start/top or quantity
   *   if qualifier is 8/16 bit limited quantity
   *  feedbackRequested - if true automatically read frozen counters when 
   *   freeze command is complete. 
   *   NOTE: if device supports frozen counter events this should be set to 
   *   false to prevent reading all of the frozen counters in addition to
   *   receiving the frozen counter events.
   * returns:
   *  Pointer to request transmit data structure or TMWDEFS_NULL
   */
  TMWDEFS_SCL_API TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_freezeCounters(
    MDNPBRM_REQ_DESC *pReqDesc,
    DNPCHNL_TX_DATA *pUserTxData,
    TMWTYPES_BOOL clear, 
    TMWTYPES_BOOL noAck, 
    TMWTYPES_UCHAR qualifier, 
    TMWTYPES_USHORT start, 
    TMWTYPES_USHORT stopOrQty,
    TMWTYPES_BOOL feedbackRequested);

    /* function: mdnpbrm_freezeAnalogInputs
   * purpose: Issue an freeze analog inputs request to the specified session
   * arguments:
   *  pReqDesc - request descriptor containing generic parameters for
   *   this request
   *  pUserTxData - pointer to user transmit data structure see description
   *   above
   *  clear - clear analog inputs after freeze
   *  noAck - don't ask for application layer response
   *  qualifier - address qualifier, must be all,
   *   or 8/16 bit start/stop
   *  start - start point if qualifier is 8/16 bit start/stop
   *  stopOrQty - stop point if qualifier is 8/16 bit start/top or quantity
   *   if qualifier is 8/16 bit limited quantity
   *  feedbackRequested - if true automatically read frozen analog inputs when 
   *   freeze command is complete. 
   *   NOTE: if device supports frozen analog input events this should be set to 
   *   false to prevent reading all of the frozen analog inputs in addition to
   *   receiving the frozen analog input events.
   * returns:
   *  Pointer to request transmit data structure or TMWDEFS_NULL
   */
  TMWDEFS_SCL_API TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_freezeAnalogInputs(
    MDNPBRM_REQ_DESC *pReqDesc,
    DNPCHNL_TX_DATA *pUserTxData,
    TMWTYPES_BOOL clear,
    TMWTYPES_BOOL noAck, 
    TMWTYPES_UCHAR qualifier, 
    TMWTYPES_USHORT start, 
    TMWTYPES_USHORT stopOrQty,
    TMWTYPES_BOOL feedbackRequested);

 /* function: mdnpbrm_freezeAtTime
   * purpose: Issue an freeze at time request for the object group to the specified session
   * arguments:
   *  pReqDesc - request descriptor containing generic parameters for
   *   this request
   *  pUserTxData - pointer to user transmit data structure see description
   *   above
   *  noAck - don't ask for application layer response
   *  objectGroup (20 counters or 30 analog inputs)
   *  qualifier - address qualifier, must be all,
   *   or 8/16 bit start/stop
   *  timeDateEnum - time-date field schedule interpretation
   *  pFreezeTime - time in UTC to perform freeze
   *  freezeInterval - time interval to perfrom periodic freezes (milliseconds)
   *  start - start point if qualifier is 8/16 bit start/stop
   *  stopOrQty - stop point if qualifier is 8/16 bit start/top or quantity
   *   if qualifier is 8/16 bit limited quantity
   *  feedbackRequested - if true automatically read the object group when 
   *   freeze command is complete. 
   *   NOTE: if device supports events this should be set to 
   *   false to prevent reading all of the object group's data in addition to
   *   receiving its events.
   * returns:
   *  Pointer to request transmit data structure or TMWDEFS_NULL
   */
  TMWDEFS_SCL_API TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_freezeAtTime(
    MDNPBRM_REQ_DESC *pReqDesc,
    DNPCHNL_TX_DATA *pUserTxData,
    TMWTYPES_BOOL noAck,
    DNPDEFS_OBJ_GROUP_ID objectGroup,
    TMWTYPES_UCHAR qualifier,
    DNPDATA_FREEZE_TIME_DATE_FIELD timeDateEnum,
    TMWDTIME *pFreezeTime,
    TMWTYPES_ULONG freezeInterval,
    TMWTYPES_USHORT start, 
    TMWTYPES_USHORT stopOrQty,
    TMWTYPES_BOOL feedbackRequested);

  /* function: mdnpbrm_writeString
   * purpose: Write a string object to the specified point
   * arguments:
   *  pReqDesc - request descriptor containing generic parameters for
   *   this request
   *  pUserTxData - pointer to user transmit data structure see description
   *   above
   *  point - point number of string to write
   *  pValue - pointer to string data to write
   *  length - length of string to write
   * returns:
   *  Pointer to request transmit data structure or TMWDEFS_NULL
   */
  TMWDEFS_SCL_API TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_writeString(
    MDNPBRM_REQ_DESC *pReqDesc,
    DNPCHNL_TX_DATA *pUserTxData,
    TMWTYPES_USHORT point,
    TMWTYPES_UCHAR *pValue,
    TMWTYPES_UCHAR length);

  /* function: mdnpbrm_writeStrings
   * purpose: Write a string objects
   * arguments:
   *  pReqDesc - request descriptor containing generic parameters for
   *   this request
   *  pUserTxData - pointer to user transmit data structure see description
   *   above
   *  qualifier - address qualifier, must be 8 or 16 bit indexed
   *  numObjects - number of strings in this request
   *  pStringInfo - data for each string
   * returns:
   *  Pointer to request transmit data structure or TMWDEFS_NULL
   */
  TMWDEFS_SCL_API TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_writeStrings(
    MDNPBRM_REQ_DESC *pReqDesc,
    DNPCHNL_TX_DATA *pUserTxData,
    TMWTYPES_UCHAR qualifier, 
    TMWTYPES_UCHAR numObjects,
    MDNPBRM_STRING_INFO *pStringInfo);

  /* function: mdnpbrm_activateConfig
   * purpose: Send an Activate Configuration request
   * arguments:
   *  pReqDesc - request descriptor containing generic parameters for
   *   this request
   *  pUserTxData - pointer to user transmit data structure see description
   *   above. If multiple names are being sent using object group 70, they will
   *   be combined using a single object header if the first object is object 70
   *   and all objects added after that if they are object 70, until an object 110 
   *   string object is added. After that each object will have a separate object
   *   header. If the outstation does not support multiple file names with 1 object
   *   header, you can set combineActConfigData = TMWDEFS_FALSE.
   *  point - point number of string to write
   *  pValue - pointer to string data to write
   *  length - length of string to write
   *    if objectGroup 110 is specified this has a max value of 255
   *    if objectGroup 70 is specified this max would be limited by
   *      tx fragment size.
   *  objectGroup - 70 to use file object in request
   *                110 to use string object in request
   * returns:
   *  Pointer to request transmit data structure or TMWDEFS_NULL
   */
  TMWDEFS_SCL_API TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_activateConfig(
    MDNPBRM_REQ_DESC *pReqDesc,
    DNPCHNL_TX_DATA *pUserTxData,
    TMWTYPES_USHORT point,
    TMWTYPES_UCHAR *pValue,
    TMWTYPES_USHORT length,
    DNPDEFS_OBJ_GROUP_ID objectGroup);

  /* function: mdnpbrm_writeVirtualTerminal
   * purpose: Write a string object to the specified point
   * arguments:
   *  pReqDesc - request descriptor containing generic parameters for
   *   this request
   *  pUserTxData - pointer to user transmit data structure see description
   *   above
   *  point - point number of virtual terminal to write to
   *  pValue - pointer to string data to write
   *  length - length of string to write
   * returns:
   *  Pointer to request transmit data structure or TMWDEFS_NULL
   */
  TMWDEFS_SCL_API TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_writeVirtualTerminal(
    MDNPBRM_REQ_DESC *pReqDesc,
    DNPCHNL_TX_DATA *pUserTxData,
    TMWTYPES_USHORT point,
    TMWTYPES_UCHAR *pValue,
    TMWTYPES_UCHAR length);

  /* function: mdnpbrm_writeExtString
   * purpose: Write an extended string object to the specified point
   * arguments:
   *  pReqDesc - request descriptor containing generic parameters for
   *   this request
   *  pUserTxData - pointer to user transmit data structure see description
   *   above
   *  point - point number of string to write
   *  pValue - pointer to string data to write
   *  length - length of string to write
   * returns:
   *  Pointer to request transmit data structure or TMWDEFS_NULL
   */
  TMWDEFS_SCL_API TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_writeExtString(
    MDNPBRM_REQ_DESC *pReqDesc,
    DNPCHNL_TX_DATA *pUserTxData,
    TMWTYPES_USHORT point,
    TMWTYPES_UCHAR *pValue,
    TMWTYPES_USHORT length);

    /* function: mdnpbrm_writeExtStrings
   * purpose: Write extended string objects
   * arguments:
   *  pReqDesc - request descriptor containing generic parameters for
   *   this request
   *  pUserTxData - pointer to user transmit data structure see description
   *   above
   *  qualifier - address qualifier, must be 8 or 16 bit indexed
   *  variation - object variation to use in request, must be 1 or 2
   *  numObjects - number of strings in this request
   *  pStringInfo - data for each string
   * returns:
   *  Pointer to request transmit data structure or TMWDEFS_NULL
   */
  TMWDEFS_SCL_API TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_writeExtStrings(
    MDNPBRM_REQ_DESC *pReqDesc,
    DNPCHNL_TX_DATA *pUserTxData,
    TMWTYPES_UCHAR qualifier, 
    TMWTYPES_UCHAR variation, 
    TMWTYPES_UCHAR numObjects,
    MDNPBRM_EXT_STRING_INFO *pStringInfo);

  /* function: mdnpbrm_fileInfo
   * purpose: - Issue a file information request to remote device
   * arguments:
   *  pReqDesc - request descriptor containing generic parameters for
   *   this request
   *  requestId - request id to be sent in request
   *  pFilename - pointer to name of file to retrieve information about.
   * returns:
   *  Pointer to request transmit data structure or TMWDEFS_NULL
   */
  TMWDEFS_SCL_API TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_fileInfo(
    MDNPBRM_REQ_DESC *pReqDesc,
    TMWTYPES_USHORT requestId,
    const TMWTYPES_CHAR *pFilename);

  /* function: mdnpbrm_fileAuthentication
   * purpose: Issue a file authentication request to a remote device
   *  asking for an authentication key.
   * arguments:
   *  pReqDesc - request descriptor containing generic parameters for
   *   this request
   *  pUserName - pointer to Null Terminated user name to be sent 
   *  pPassword - pointer to Null Terminated password to be sent
   * returns:
   *  Pointer to request transmit data structure or TMWDEFS_NULL
   */
  TMWDEFS_SCL_API TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_fileAuthentication(
    MDNPBRM_REQ_DESC *pReqDesc,
    TMWTYPES_CHAR *pUsername,
    TMWTYPES_CHAR *pPassword);

  /* function: mdnpbrm_fileOpen
   * purpose: Send a request to open a file on the remote device. If file
   *  authentication is required the already acquired file authentication 
   *  key may be passed to this function. 
   * arguments:
   *  pReqDesc - request descriptor containing generic parameters for
   *   this request
   *  requestId - request id to be sent in request
   *  mode - mode for file open indicating open for reading, writing, or appending
   *  permissions - permissions value to send to outstation, only used if mode == DNPDEFS_FILE_MODE_WRITE 
   *                these bits may be OR'ed together
   *                DNPDEFS_WORLD_EXECUTE_ALLOWED     0x0001
   *                DNPDEFS_WORLD_WRITE_ALLOWED       0x0002
   *                DNPDEFS_WORLD_READ_ALLOWED        0x0004
   *                DNPDEFS_GROUP_EXECUTE_ALLOWED     0x0008
   *                DNPDEFS_GROUP_WRITE_ALLOWED       0x0010
   *                DNPDEFS_GROUP_READ_ALLOWED        0x0020
   *                DNPDEFS_OWNER_EXECUTE_ALLOWED     0x0040
   *                DNPDEFS_OWNER_WRITE_ALLOWED       0x0080
   *                DNPDEFS_OWNER_READ_ALLOWED        0x0100
   *  fileType  DNPDEFS_FILE_TYPE_DIRECTORY if opening a directory for reading
   *            DNPDEFS_FILE_TYPE_SIMPLE if opening a file for reading or writing
   *  authentication key - key to be sent in open request. Should be 0 or have 
   *   been acquired by mdnpbrm_fileAuthentication
   *  maxBlockSize - maximum block size to be read or written for this file
   *  pFileName - pointer to file or directory to open
   * returns:
   *  Pointer to request transmit data structure or TMWDEFS_NULL
   */
  TMWDEFS_SCL_API TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_fileOpen(
    MDNPBRM_REQ_DESC *pReqDesc,
    TMWTYPES_USHORT requestId,
    DNPDEFS_FILE_MODE mode,
    TMWTYPES_USHORT permissions,
    DNPDEFS_FILE_TYPE fileType,
    TMWTYPES_ULONG authenticationKey,
    TMWTYPES_USHORT maxBlockSize,
    const TMWTYPES_CHAR *pFilename);

  /* function: mdnpbrm_fileOpenForWrite
   * purpose: Send a request to open a file for writing or appending on the remote device. If file
   *  authentication is required the already acquired file authentication 
   *  key may be passed to this function. This function was added for writing and appending because
   *  Time of Creation and File Size should be sent with meaningful values in those cases. This function can
   *  be used for opening for reading, in which case Time of Creation should be Jan 1 1970 and file size should 
   *  be zero  
   * arguments:
   *  pReqDesc - request descriptor containing generic parameters for
   *   this request
   *  requestId - request id to be sent in request
   *  mode - mode for file open indicating open for reading, writing, or appending
   *  permissions - permissions value to send to outstation, only used if mode == DNPDEFS_FILE_MODE_WRITE 
   *                these bits may be OR'ed together
   *                DNPDEFS_WORLD_EXECUTE_ALLOWED     0x0001
   *                DNPDEFS_WORLD_WRITE_ALLOWED       0x0002
   *                DNPDEFS_WORLD_READ_ALLOWED        0x0004
   *                DNPDEFS_GROUP_EXECUTE_ALLOWED     0x0008
   *                DNPDEFS_GROUP_WRITE_ALLOWED       0x0010
   *                DNPDEFS_GROUP_READ_ALLOWED        0x0020
   *                DNPDEFS_OWNER_EXECUTE_ALLOWED     0x0040
   *                DNPDEFS_OWNER_WRITE_ALLOWED       0x0080
   *                DNPDEFS_OWNER_READ_ALLOWED        0x0100
   *  fileType  DNPDEFS_FILE_TYPE_DIRECTORY if opening a directory for reading
   *            DNPDEFS_FILE_TYPE_SIMPLE if opening a file for reading or writing
   *  authentication key - key to be sent in open request. Should be 0 or have 
   *   been acquired by mdnpbrm_fileAuthentication
   *  maxBlockSize - maximum block size to be read or written for this file
   *  pFileName - pointer to file or directory to open
   *  timeOfCreation - time of creation for local file on master.
   *  fileSize - size of local file on master if writing or appending, use 0xffffffff if not known, and use 0 if reading
   * returns:
   *  Pointer to request transmit data structure or TMWDEFS_NULL
   */
  TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_fileOpenForWrite(
    MDNPBRM_REQ_DESC *pReqDesc,
    TMWTYPES_USHORT requestId,
    DNPDEFS_FILE_MODE mode,
    TMWTYPES_USHORT permissions,
    DNPDEFS_FILE_TYPE fileType,
    TMWTYPES_ULONG authenticationKey,
    TMWTYPES_USHORT maxBlockSize,
    const TMWTYPES_CHAR *pFilename,
    TMWDTIME *pTimeOfCreation,
    TMWTYPES_ULONG fileSize);

  /* function: mdnpbrm_fileClose
   * purpose: Send a request to close a file on the remote device
   * arguments:
   *  pReqDesc - request descriptor containing generic parameters for
   *   this request
   *  requestId - request id to be sent in request.
   *  handle - handle that was returned from remote device when file was opened
   * returns:
   *  Pointer to request transmit data structure or TMWDEFS_NULL
   */
  TMWDEFS_SCL_API TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_fileClose(
    MDNPBRM_REQ_DESC *pReqDesc,
    TMWTYPES_USHORT requestId,
    TMWTYPES_ULONG handle);

  /* function: mdnpbrm_fileDelete
   * purpose: send a request to delete a file on the remote device
   * arguments:
   *  pReqDesc - request descriptor containing generic parameters for
   *   this request
   *  requestId - request id to be sent in request.
   *  authenticationKey - authentication key to be sent if pUserName is 
   *   TMWDEFS_NULL. Could have been acquired by mdnpbrm_fileAuthentication 
   *   or could be zero if not required.
   *  pUserName - pointer to Null Terminated user name to be sent to get 
   *   authentication key. This should be set to TMWDEFS_NULL if authentication 
   *   request is not required to be performed by this function.
   *  pPassword - pointer to Null Terminated password to be sent to get 
   *   authentication key
   *  pFileName - name of file to be deleted on remote device
   * returns:
   *  Pointer to request transmit data structure or TMWDEFS_NULL
   */
  TMWDEFS_SCL_API TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_fileDelete(
    MDNPBRM_REQ_DESC *pReqDesc,
    TMWTYPES_USHORT requestId,
    TMWTYPES_ULONG authenticationKey,
    const TMWTYPES_CHAR *pUserName,
    const TMWTYPES_CHAR *pPassword,
    const TMWTYPES_CHAR *pFilename);

  /* function: mdnpbrm_fileAbort
   * purpose: 
   * arguments:
   *  pReqDesc - request descriptor containing generic parameters for
   *   this request
   * returns:
   *  Pointer to request transmit data structure or TMWDEFS_NULL
   */
  TMWDEFS_SCL_API TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_fileAbort(
    MDNPBRM_REQ_DESC *pReqDesc,
    TMWTYPES_USHORT requestId,
    TMWTYPES_ULONG handle);

  /* function: mdnpbrm_fileRead
   * purpose: Send a single file read block request to the remote device.
   *  The remote file should have been previously opened for writing.
   *  When a successful read response is received from the device the 
   *  mdnpdata_storeFileData function will be called.
   * arguments:
   *  pReqDesc - request descriptor containing generic parameters for
   *   this request
   *  block - block number, should be sequential on each call to this
   *   function beginning with zero after file is opened.
   *  handle - handle that was returned from remote device when file was opened
   * returns:
   *  Pointer to request transmit data structure or TMWDEFS_NULL
   */
  TMWDEFS_SCL_API TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_fileRead(
    MDNPBRM_REQ_DESC *pReqDesc,
    TMWTYPES_ULONG block,
    TMWTYPES_ULONG handle);

  /* function: mdnpbrm_fileWrite
   * purpose: Send a single file write block request to the remote device.
   *  The remote file should have previously been opened for writing.
   * arguments:
   *  pReqDesc - request descriptor containing generic parameters for
   *   this request
   *  block - block number, should be sequential on each call to this
   *   function beginning with zero after file is opened.
   *  last - TMWDEFS_FALSE unless this is the final write to this file
   *   then it should be TMWDEFS_TRUE
   *  handle - handle that was returned from remote device when file was opened
   *  pData - pointer to data to be written to file
   *  length - length of data to be written to file
   * returns:
   *  Pointer to request transmit data structure or TMWDEFS_NULL
   */
  TMWDEFS_SCL_API TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_fileWrite(
    MDNPBRM_REQ_DESC *pReqDesc,
    TMWTYPES_ULONG block,
    TMWTYPES_BOOL last,
    TMWTYPES_ULONG handle,
    const TMWTYPES_UCHAR *pData,
    TMWTYPES_USHORT length);

  /* function: mdnpbrm_copyLocalFileToRemote
   * purpose: copy a file from local to remote device
   *  This will perform the authentication request if required, open
   *  the local file specified, send the file open request to the remote
   *  device, send the sequential block read requests and then close the 
   *  file on the remote device as well as the local file.
   * arguments:
   *  pReqDesc - request descriptor containing generic parameters for
   *   this request
   *  pLocalFileName - name of the local file
   *  pRemoteFileName - name of the remote file
   *  pUserName - the user name for file authentication request.
   *   TMWDEFS_NULL if no authentication to be performed
   *  pPassword - the password for file authentication request.
   *   TMWDEFS_NULL if not required for authentication request.
   *  permissions - permissions value to send to outstation 
   *                these bits may be OR'ed together
   *                DNPDEFS_WORLD_EXECUTE_ALLOWED     0x0001
   *                DNPDEFS_WORLD_WRITE_ALLOWED       0x0002
   *                DNPDEFS_WORLD_READ_ALLOWED        0x0004
   *                DNPDEFS_GROUP_EXECUTE_ALLOWED     0x0008
   *                DNPDEFS_GROUP_WRITE_ALLOWED       0x0010
   *                DNPDEFS_GROUP_READ_ALLOWED        0x0020
   *                DNPDEFS_OWNER_EXECUTE_ALLOWED     0x0040
   *                DNPDEFS_OWNER_WRITE_ALLOWED       0x0080
   *                DNPDEFS_OWNER_READ_ALLOWED        0x0100
   * returns:
   *  Pointer to request transmit data structure or TMWDEFS_NULL
   */
  TMWDEFS_SCL_API TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_copyLocalFileToRemote(
    MDNPBRM_REQ_DESC *pReqDesc,
    const TMWTYPES_CHAR *pLocalFileName,
    const TMWTYPES_CHAR *pRemoteFileName,
    const TMWTYPES_CHAR *pUserName,
    const TMWTYPES_CHAR *pPassword,
    const TMWTYPES_USHORT permissions);

  /* function: mdnpbrm_copyRemoteFileToLocal
   * purpose: copy a file from remote to local device.
   *  This will perform the authentication request if required, open
   *  the local file specified, send the file open request to the remote
   *  device, send the sequential block write requests and then close the 
   *  file on the remote device as well as the local file.
   * arguments:
   *  pReqDesc - request descriptor containing generic parameters for
   *   this request
   *  pLocalFileName - pointer to NULL terminated name of the local file
   *  pRemoteFileName - pointer to NULL terminated name of the remote file
   *  pUserName - pointer to the NULL terminated user name for the file
   *   authentication request. 
   *   TMWDEFS_NULL if no authentication to be performed
   *  pPassword - pointer to the NULL terminated password for the file
   *   authentication request.
   *   TMWDEFS_NULL if not required for authentication request.
   * returns:
   *  Pointer to request transmit data structure or TMWDEFS_NULL
   */
  TMWDEFS_SCL_API TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_copyRemoteFileToLocal(
    MDNPBRM_REQ_DESC *pReqDesc,
    const TMWTYPES_CHAR *pLocalFileName,
    const TMWTYPES_CHAR *pRemoteFileName,
    const TMWTYPES_CHAR *pUserName,
    const TMWTYPES_CHAR *pPassword);

  /* function: mdnpbrm_readRemoteDirectory
   * purpose: Read a directory from a remote device 
   * arguments:
   *  pReqDesc - request descriptor containing generic parameters for
   *   this request 
   *  pRemoteDirName - pointer to NULL terminated name of the remote directory  
   *  pUserName - pointer to the NULL terminated user name for the file
   *   authentication request. 
   *   TMWDEFS_NULL if no authentication to be performed
   *  pPassword - pointer to the NULL terminated password for the file
   *   authentication request.
   *   TMWDEFS_NULL if not required for authentication request.
   * returns:
   *  Pointer to request transmit data structure or TMWDEFS_NULL
   */
  TMWDEFS_SCL_API TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_readRemoteDirectory(
    MDNPBRM_REQ_DESC *pReqDesc,
    const TMWTYPES_CHAR *pRemoteDirName,
    const TMWTYPES_CHAR *pUserName,
    const TMWTYPES_CHAR *pPassword);

  /* function: mdnpbrm_writeDeviceAttribute
   * purpose: Issue a device attribute write request using Object 0 variation as
   *  specified. 
   * arguments:
   *  pReqDesc - request descriptor containing generic parameters for this request
   *  pUserTxData - pointer to user transmit data structure, see description above
   *  point - point index of device attribute to write to. Use 0 for standard 
   *   attribute set. Use indexes other than 0 for user-specific attribute sets.
   *  variation - variation of device attribute to write to 
   *   NOTE: device attributes use point number and variation to specify a specific 
   *   attribute.   Defines DNPDEFS_OBJ0xxx in dnpdefs.h can be used to identify 
   *   the standard attribute variations for Index 0.
   *  pData - pointer to DNPDATA_ATTRIBUTE_VALUE structure specifying value to be 
   *   written.
   * returns:
   *  Pointer to request transmit data structure or TMWDEFS_NULL
   */
  TMWDEFS_SCL_API TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_writeDeviceAttribute(
    MDNPBRM_REQ_DESC *pReqDesc,
    DNPCHNL_TX_DATA *pUserTxData,
    TMWTYPES_USHORT   point,
    TMWTYPES_UCHAR    variation,
    DNPDATA_ATTRIBUTE_VALUE *pData);

  /* function: mdnpbrm_writeDatasetProto
   * purpose: Issue a data set prototype write request using Object 85 variation 1
   * arguments:
   *  pReqDesc - request descriptor containing generic parameters for this request
   *  pUserTxData - pointer to user transmit data structure, see description above
   *  numObjects - number of prototypes to write
   *  pPointNumbers - if TMWDEFS_NULL, start with first prototype defined on 
   *    master and send numObjects prototypes. If not TMWDEFS_NULL this points 
   *    to an array of point numbers for prototypes to be written.
   *  NOTE: contents of prototype(s) will be retrieved by 
   *    mdnpdata_datasetProtoGet() and not passed as a parameter to this function.
   * returns:
   *  Pointer to request transmit data structure or TMWDEFS_NULL
   */
  TMWDEFS_SCL_API TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_writeDatasetProto(
    MDNPBRM_REQ_DESC *pReqDesc,
    DNPCHNL_TX_DATA  *pUserTxData,
    TMWTYPES_UCHAR    numObjects,
    TMWTYPES_USHORT  *pPointNumbers);

  /* function: mdnpbrm_writeDatasetDescr
   * purpose: Issue a write data set descriptor request using Object 86  
   *   variation as specified in parameter
   * arguments:
   *  pReqDesc - request descriptor containing generic parameters for this request
   *  pUserTxData - pointer to user transmit data structure, see description above
   *  variation - descriptor variation to use when writing descriptors 
   *   (1 Contents or 3 Point Index Attributes)
   *  numObjects - number of descriptors to write
   *  pPointNumbers - if TMWDEFS_NULL, start with first descriptor defined on 
   *    master and send numObjects descriptors. If not TMWDEFS_NULL this points
   *    to an array of point numbers for descriptors to be written.
   *  NOTE: contents of descriptor(s) will be retrieved by 
   *    mdnpdata_datasetDescrGetCont() or mdnpdata_datasetDescrGetIndex() and not 
   *    passed as a parameter to this function.
   * returns:
   *  Pointer to request transmit data structure or TMWDEFS_NULL
   */
  TMWDEFS_SCL_API TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_writeDatasetDescr(
    MDNPBRM_REQ_DESC *pReqDesc,
    DNPCHNL_TX_DATA  *pUserTxData,
    TMWTYPES_UCHAR    variation,
    TMWTYPES_UCHAR    numObjects,
    TMWTYPES_USHORT  *pPointNumbers);

  /* function: mdnpbrm_writeDataset
   * purpose: Issue a write data set request using Object 87 variation 1
   * arguments:
   *  pReqDesc - request descriptor containing generic parameters for this request
   *  pUserTxData - pointer to user transmit data structure, see description above
   *  numObjects - number of data sets to write
   *  pPointNumbers - if TMWDEFS_NULL, start with data set id (index) 0
   *    and send numObjects data sets. If not TMWDEFS_NULL this points to an array
   *    of point numbers for data sets to be written.
   *  NOTE: contents of data set(s) will be retrieved by mdnpdata_datasetGet() and not 
   *    passed as a parameter to this function.
   * returns:
   *  Pointer to request transmit data structure or TMWDEFS_NULL
   */
  TMWDEFS_SCL_API TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_writeDataset(
    MDNPBRM_REQ_DESC *pReqDesc,
    DNPCHNL_TX_DATA  *pUserTxData,
    TMWTYPES_UCHAR    numObjects,
    TMWTYPES_USHORT  *pPointNumbers);
  
  /* function: mdnpbrm_controlDataset
   * purpose: Issue a control data set request using Object 87 variation 1
   * arguments:
   *  pReqDesc - request descriptor containing generic parameters for this request
   *  funcCode - function code, must be DNPDEFS_FC_SELECT, DNPDEFS_FC_OPERATE, or
   *   DNPDEFS_FC_DIRECT_OP, or DNPDEFS_FC_DIRECT_OP_NOACK
   *  autoMode - bit mask to specify which actions to perform internally within 
   *   the SCL. MDNPBRM_AUTO_MODE_NONE or MDNPBRM_AUTO_MODE_OPERATE.
   *  numObjects - number of data sets to send (currently only 1 supported)
   *  pPointNumbers - if TMWDEFS_NULL, start with data set id (index) 0
   *    and send numObjects data sets. If not TMWDEFS_NULL this points to an array
   *    of point numbers for data sets to be written.
   *  NOTE: contents of data set(s) will be retrieved by mdnpdata_datasetGet() and not 
   *    passed as a parameter to this function.
   * returns:
   *  Pointer to request transmit data structure or TMWDEFS_NULL
   * NOTE: If you are doing a SELECT/OPERATE but choose not to use the autoMode
   *  mask to do this, but instead are going to call this brm function to send 
   *  the SELECT and then call this function again to send the OPERATE, you will
   *  need to increase the pReqDesc->priority of the OPERATE to 255 to 
   *  guarantee it gets sent before any automatic processing of events is performed. 
   *  (Automatic processing of events is enabled by default when the mdnp session 
   *  was opened). For example, if in the response to the SELECT the restart IIN bit  
   *  is set and the session was set up to automatically send a clear restart
   *  command, you will want to make sure the OPERATE gets sent before the clear 
   *  restart, otherwise the operate will fail.
   */
  TMWDEFS_SCL_API TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_controlDataset(
    MDNPBRM_REQ_DESC *pReqDesc,
    TMWTYPES_UCHAR    funcCode, 
    MDNPBRM_AUTO_MODE autoMode,
    TMWTYPES_UCHAR    numObjects,
    TMWTYPES_USHORT  *pPointNumbers);

  /* function: mdnpbrm_addControlDataset
   * purpose: Issue a control data set request using Object 87 variation 1
   *  NOTE: This function is the equivalent to mdnpbrm_controlDataset except 
   *  the parameter pUserData has been added to allow multiple control objects 
   *  to be sent in the same request.
   * arguments:
   *  pReqDesc - request descriptor containing generic parameters for this request
   *  pUserTxData - pointer to user transmit data structure, see description above
   *  funcCode - function code, must be DNPDEFS_FC_SELECT, DNPDEFS_FC_OPERATE, or
   *   DNPDEFS_FC_DIRECT_OP
   *  autoMode - bit mask to specify which actions to perform internally within 
   *   the SCL. MDNPBRM_AUTO_MODE_NONE or MDNPBRM_AUTO_MODE_OPERATE.
   *  numObjects - number of data sets to send (currently only 1 supported)
   *  pPointNumbers - if TMWDEFS_NULL, start with data set id (index) 0
   *    and send numObjects data sets. If not TMWDEFS_NULL this points to an array
   *    of point numbers for data sets to be written.
   *  NOTE: contents of data set(s) will be retrieved by mdnpdata_datasetGet() and not 
   *    passed as a parameter to this function.
   * returns:
   *  Pointer to request transmit data structure or TMWDEFS_NULL
   * NOTE: If you are doing a SELECT/OPERATE but choose not to use the autoMode
   *  mask to do this, but instead are going to call this brm function to send 
   *  the SELECT and then call this function again to send the OPERATE, you will
   *  need to increase the pReqDesc->priority of the OPERATE to 255 to 
   *  guarantee it gets sent before any automatic processing of events is performed. 
   *  (Automatic processing of events is enabled by default when the mdnp session 
   *  was opened). For example, if in the response to the SELECT the restart IIN bit  
   *  is set and the session was set up to automatically send a clear restart
   *  command, you will want to make sure the OPERATE gets sent before the clear 
   *  restart, otherwise the operate will fail.
   */
  TMWDEFS_SCL_API TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_addControlDataset(
    MDNPBRM_REQ_DESC *pReqDesc,
    DNPCHNL_TX_DATA  *pUserTxData,
    TMWTYPES_UCHAR    funcCode, 
    MDNPBRM_AUTO_MODE autoMode,
    TMWTYPES_UCHAR    numObjects,
    TMWTYPES_USHORT  *pPointNumbers);

  /* function: mdnpbrm_datasetExchange 
   * purpose: Begin exchange of data set prototype and descriptor objects with 
   *   outstation(slave). This consists of a read of two device attributes 
   *   indicating the number of prototypes and dataset descriptors defined
   *   on the outstation. The master will then send a request to read the 
   *   prototypes defined on the slave, followed by a write of the prototypes
   *   defined on the master. The master will then send a request to read the
   *   descriptors defined on the slave, followed by a write of the descriptors
   *   defined on the master.
   * arguments:
   *  pReqDesc - request descriptor containing generic parameters for this request
   * returns:
   *  Pointer to request transmit data structure or TMWDEFS_NULL
   */
  TMWDEFS_SCL_API TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_datasetExchange(
    MDNPBRM_REQ_DESC *pReqDesc);

  /* function: mdnpbrm_authManualSendSessionKey
  * purpose: Initiate a Secure Authentication Session Key Change sequence
  * arguments:
  *  pReqDesc - request descriptor containing generic parameters for this request
  *  userNumber - User on which to perform the Session Key Change operation.
  *     A Session Key Change is done automatically when needed. This can be used
  *     for testing purposes. You may want to set MDNPSESN_AUTH_CONFIG.testConfig 
  *     to prevent automatic sending.
  * returns:
  *  Pointer to request transmit data structure or TMWDEFS_NULL
  */
  TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_authManualSendSessionKey(
    MDNPBRM_REQ_DESC *pReqDesc,
    TMWTYPES_USHORT userNumber);

  /* function: mdnpbrm_authUserCertificate 
   * purpose: Initiate Secure Authentication Version 5 User Cerificate (instead of 
   *  g120v10 Status Change) Sequence, starting by sending a g120v8 User Certificate
   *  request to the outstation. This uses a IEC/TS 62351-8 Certificate instead of 
   *  a DNP specific set of data.
   * arguments:
   *  pReqDesc - request descriptor containing generic parameters for this request
   *  userNameDbHandle - handle for looking up user data received from the Authority
   *   that was stored in the database and will be retrieved by the SCL.  
   *   This includes a IEC/TS 62351-8 Certificate which contains the user's public key
   *   or an Attribute Certificate which does not.
   *   This data will be accessed by mdnpdata_authGetUserName, mdnpdata_authGetChangeUserData
   *    tmwcrypto_getCertificate using this handle.
   *   This handle will also be passed to mdnpdata_authStoreUpdKeyChangeReply
   *  operation is in certificate  
   *  sendUpdateKey - 
   *   TMWDEFS_TRUE if User Status Change(g120v8) should be followed by 
   *     Update Key Change Request (g120v11) and rest of key change sequence 
   *   TMWDEFS_FALSE if Only User Certificate should be sent to Outstation.
   * returns:
   *  Pointer to request transmit data structure or TMWDEFS_NULL
   */
  TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_authUserCertificate(
    MDNPBRM_REQ_DESC *pReqDesc,
    void*             userNameDbHandle);

  /* function: mdnpbrm_authUserStatusChange 
   * purpose: Initiate Secure Authentication Version 5 User Status Change Sequence
   *  starting by sending a g120v10 User Status Change request to the outstation. 
   * arguments:
   *  pReqDesc - request descriptor containing generic parameters for this request
   *  userNameDbHandle - handle for looking up user data received from the Authority
   *   that was stored in the database and will be retrieved by the SCL. This includes 
   *   Status Change Sequence (SCS) number between Authority and Outstation,
   *   A globally unique identifier (name) representing the user,  
   *   Interval, Role and Users public key.
   *   Also this same information digitally signed or encrypted depending on whether
   *   symmetric or asymmetric change method is being used. This handle
   *   will be passed to mdnpdata_authGetUserName, mdnpdata_authGetChangeUserData
   *   tmwcrypto_getKey(USER_ASYM_PUB_KEY), mdnpdata_authGetCertData, and 
   *   mdnpdata_authStoreUpdKeyChangeReply 
   *  operation - operation to perform  
   *   DNPAUTH_USER_STATUS_ADD         
   *   DNPAUTH_USER_STATUS_DELETE
   *   DNPAUTH_USER_STATUS_CHANGE
   *  sendUpdateKey - 
   *   TMWDEFS_TRUE if User Status Change(g120v10) should be followed by 
   *     Update Key Change Request (g120v11) and rest of key change sequence 
   *   TMWDEFS_FALSE if Only User Status Change should be sent to Outstation.
   * returns:
   *  Pointer to request transmit data structure or TMWDEFS_NULL
   */
  TMWDEFS_SCL_API TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_authUserStatusChange(
    MDNPBRM_REQ_DESC *pReqDesc,
    void             *userNameDbHandle,
    TMWTYPES_UCHAR    operation,
    TMWTYPES_BOOL     sendUpdateKey);

  /* function: mdnpbrm_authUserUpdateKeyChange 
   * purpose: Initiate Secure Authentication Version 5 User Update Key Change
   *  or Authentication/Confirm Sequence to verify the existing User Update Key,
   *  starting by sending a g120v11 Update Key Change Request to the outstation. 
   * arguments:
   *  pReqDesc - request descriptor containing generic parameters for this request 
   *  userNameDbHandle - handle for looking up user data received from the Authority
   *   see description above.
   *  sendUpdateKey - 
   *   TMWDEFS_TRUE if a new User Update Key should be sent to the Outstation
   *   TMWDEFS_FALSE if the existing User Update Key should be verified on the Outstation.
   * returns:
   *  Pointer to request transmit data structure or TMWDEFS_NULL
   */
  TMWDEFS_SCL_API TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_authUserUpdateKeyChange(
    MDNPBRM_REQ_DESC *pReqDesc,
    void *            userNameDbHandle,
    TMWTYPES_BOOL     sendUpdateKey);

  /* function: mdnpbrm_authUserSendSymUpdateKey 
   * purpose: Send Secure Authentication Version 5 Symmetric User Update Key 
   *  encrypted by the Authority (along with other data)
   *  in a g120v13 Update Key Change to the outstation. 
   * arguments:
   *  pReqDesc - request descriptor containing generic parameters for this request 
   *  userNameDbHandle - handle for looking up ecnrypted user data received from the 
   *  Authority  
   * returns:
   *  Pointer to request transmit data structure or TMWDEFS_NULL
   */
  TMWDEFS_SCL_API TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_authUserSendSymUpdateKey(
    MDNPBRM_REQ_DESC *pReqDesc,
    void *            userNameDbHandle);

  /* function: mdnpbrm_authUserSendg120v6Test
   * purpose: For Test Purposes Only. Send just a Secure Authentication g120v6
   *  without first sending a g120v4 as specified by 1815-2012
   * arguments:
   *  pReqDesc - request descriptor containing generic parameters for this request
   *  userNumber - User on which to send the g120v6 Session Key Change request.
   * returns:
   *  Pointer to request transmit data structure or TMWDEFS_NULL
   */
  TMWDEFS_SCL_API TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_authUserSendg120v6Test(
    MDNPBRM_REQ_DESC *pReqDesc,
    TMWTYPES_USHORT userNumber);

  /* function: mdnpbrm_authUserSendg120v15Test
   * purpose: For Test Purposes Only. Send just a Secure Authentication 
   *  User Update Key Confirm a g120v15 to the Outstation. NOTE: This MUST 
   *  only be sent after receiving a g120v12 from the Outstation indicating 
   *  a User Number. The g120v15 request contains NO User Number so OS must 
   *  know what User Number this relates to.
   * arguments:
   *  pReqDesc - request descriptor containing generic parameters for this request
   *  userNameDbHandle - handle for looking up ecnrypted user data received from the
   *  Authority
   * returns:
   *  Pointer to request transmit data structure or TMWDEFS_NULL
   */
  TMWDEFS_SCL_API TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_authUserSendg120v15Test(
    MDNPBRM_REQ_DESC *pReqDesc,
    void *            userNameDbHandle);

#ifdef __cplusplus
}
#endif
#endif /* MDNPBRM_DEFINED */
