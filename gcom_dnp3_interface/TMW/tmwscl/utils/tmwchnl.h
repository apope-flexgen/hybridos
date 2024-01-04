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

/* file: tmwchnl.h
 * description: Channel related structures and functions 
 *  implementations.
 */
#ifndef TMWCHNL_DEFINED
#define TMWCHNL_DEFINED

#ifndef __midl
#include "tmwscl/utils/tmwcnfg.h"
#include "tmwscl/utils/tmwdefs.h"
#include "tmwscl/utils/tmwdlist.h"
#include "tmwscl/utils/tmwphys.h"
#include "tmwscl/utils/tmwlink.h"
#include "tmwscl/utils/tmwtprt.h"
#include "tmwscl/utils/tmwappl.h"
#include "tmwscl/utils/tmwdtime.h"
#include "tmwtargos.h"  /* TMWDEFS_RESOURCE_LOCK */
#endif

typedef enum TMWCHNL_ERROR_CODE_ENUM
{
  /* Physical layer errors */
  TMWCHNL_ERROR_PHYS_TRANSMIT,            /* Error returned from target transmit routine         */
  TMWCHNL_ERROR_PHYS_CHAR_TIMEOUT,        /* Intercharacter timeout occurred                     */ 
  TMWCHNL_ERROR_PHYS_REMOTE_CLOSE,        /* remote side of channel closed connection            */ 

  /* Link layer errors */
  TMWCHNL_ERROR_LINK_FRAME_LENGTH,        /* Incoming frame too short or exceeded buffer size    */
  TMWCHNL_ERROR_LINK_ADDRESS_UNKNOWN,     /* Received frame was for an unknown link address      */
  TMWCHNL_ERROR_LINK_ILLEGAL_FUNCTION,    /* illegal link function code in received frame        */
  TMWCHNL_ERROR_LINK_INVALID_CHECKSUM,    /* Invalid checksum or CRC                             */
  TMWCHNL_ERROR_LINK_NOT_RESET,           /* Link has not been reset, frame rejected             */
  TMWCHNL_ERROR_LINK_FCB,                 /* Received invalid frame count bit                    */
  TMWCHNL_ERROR_LINK_INVALID_START_CHAR,  /* Did not receive correct starting sync char          */
  TMWCHNL_ERROR_LINK_FRAME_TIMEOUT,       /* Entire frame was not received in specified time     */
  TMWCHNL_ERROR_LINK_CNFM_TIMEOUT,        /* Link Confirm was not received in specified time     */
  TMWCHNL_ERROR_LINK_STATUS_TIMEOUT,      /* Link status response not received in specified time */

                                          /* The following link errors are used by 101/103       */
  TMWCHNL_ERROR_LINK_WRONG_SESN,          /* Response was not from expected session              */
  TMWCHNL_ERROR_LINK_WRONG_REPLY,         /* Received unexpected reply, frame rejected           */
  TMWCHNL_ERROR_LINK_INVALID_2ND_CHAR,    /* Did not receive correct second sync char            */
  TMWCHNL_ERROR_LINK_INVALID_END_CHAR,    /* Did not receive correct ending sync character       */
  TMWCHNL_ERROR_LINK_MISMATCHING_LENGTH,  /* variable length bytes in FT1.2 frame did not match  */
  TMWCHNL_ERROR_LINK_INV_DIR,             /* Received invalid dir bit in control octet           */

                                          /* The following link errors are used by 104           */
  TMWCHNL_ERROR_LINK_NO_CNFM_RECEIVED,    /* Confirm of 104 U-format APDU not received           */
  TMWCHNL_ERROR_LINK_NO_ACK_RECEIVED,     /* Acknowledge of 104 I-format APDU not received       */ 
  TMWCHNL_ERROR_LINK_SEQUENCE_UNKNOWN,    /* Unknown confirming sequence number in received APDU */
  TMWCHNL_ERROR_LINK_OUT_OF_SEQUENCE,     /* received APDU not in sequence with previous APDU    */  /*SPM, not currently used */

  /* Transport layer errors */
  TMWCHNL_ERROR_TPRT_SEQUENCE_ERROR       /* Sequence number error */

} TMWCHNL_ERROR_CODE;

typedef enum TMWCHNL_STAT_EVENT_ENUM
{
  /* pEventData points to TMWCHNL_STAT_ERROR_TYPE */
  TMWCHNL_STAT_ERROR,   

  /* pEventData is TMWDEFS_NULL for these two */
  TMWCHNL_STAT_OPEN,
  TMWCHNL_STAT_CLOSED,

  /* pEventData points to a TMWTYPES_USHORT containing the quantity of bytes */
  TMWCHNL_STAT_BYTES_SENT,
  TMWCHNL_STAT_BYTES_RECEIVED,

  /* pEventData is TMWDEFS_NULL for these four */
  TMWCHNL_STAT_FRAME_SENT,
  TMWCHNL_STAT_FRAME_RECEIVED,
  TMWCHNL_STAT_FRAGMENT_SENT,
  TMWCHNL_STAT_FRAGMENT_RECEIVED,

  /* For I870 104 Protocol only
   * pEventData points to a TMWTYPES_ULONG containing the elapsedTime in milliseconds 
   */
  TMWCHNL_STAT_T1_TIME_ELAPSED,

  /* pEventData is TMWDEFS_NULL for the following */

  /* For I870 104 Protocol only
   * indicates this channel is the active connection 
   */
  TMWCHNL_STAT_ACTIVE,
  
  /* For I870 104 Protocol only
   * indicates this channel has received a STARTDT act
   */
  TMWCHNL_STAT_STARTDT_RCVD,

  /* For I870 104 Protocol only
   * indicates this channel has received a STOPDT act
   */
  TMWCHNL_STAT_STOPDT_RCVD,

  /* For I870 104 Protocol only
   * indicates this channel has received a STARTDT con
   */
  TMWCHNL_STAT_STARTDT_CON_RCVD,

  /* For I870 104 Protocol only
   * indicates this channel has received a STOPDT con
   */
  TMWCHNL_STAT_STOPDT_CON_RCVD,

  /* For I870 Protocols only 
   * pEventData points to TMWCHNL_STAT_UNKNOWN_SESN_TYPE 
   */
  TMWCHNL_STAT_UNKNOWN_SESSION  

} TMWCHNL_STAT_EVENT;

typedef enum TMWCHNL_TYPE_ENUM
{
  /* Channel Types */
  TMWCHNL_TYPE_DNP,
  TMWCHNL_TYPE_I12,
  TMWCHNL_TYPE_I104,
  TMWCHNL_TYPE_I104_RDCY,
  TMWCHNL_TYPE_MB,
} TMWCHNL_TYPE;

#ifndef __midl
typedef struct { 
  TMWTYPES_SESSION_TYPE sessionType;
  TMWTYPES_USHORT linkAddress;
  TMWTYPES_USHORT destAddress; /* for DNP */
} TMWCHNL_STAT_UNKNOWN_SESN_TYPE;

typedef struct { 
  TMWCHNL_ERROR_CODE errorCode;/* Should be first for backward compatibility */
  TMWSESN *pSession;           /* TMWDEFS_NULL if session is not known       */
} TMWCHNL_STAT_ERROR_TYPE;

typedef void (*TMWCHNL_STAT_CALLBACK)(
  void *pCallbackParam, 
  TMWCHNL_STAT_EVENT eventType, 
  void *pEventData);

typedef void (*TMWCHNL_IDLE_CALLBACK)(
  void *pCallbackParam);

typedef void* (*TMWCHNL_LINK_RDCY_CALLBACK)(
  void *pRdcyGroupChannel);

/* Define the auto open session callback function.
 * pCallbackParam - parameter user provided when channel was opened
 * srcAddress - the source address as required for xxxsesn_openSession. 
 *   This was the destination address in the received message.
 * destAddress - the destination address as required for xxxsesn_openSession
 *   This was the source address in the received message.
 * type - indicates whether master or slave session should to be opened to
 *   process this message.
 *
 *  Example dnp callback function to be called when a message has been 
 *  received for a session that is not open. This function will attempt  
 *  to open a master or slave session using the received addresses.
 *
 *   static void _AutoOpenSessionCallback(
 *     void *pCallbackParam,
 *     TMWTYPES_USHORT srcAddress,
 *     TMWTYPES_USHORT destAddress,
 *     TMWTYPES_SESSION_TYPE type)
 *   {
 *     TMWCHNL *pChannel = (TMWCHNL*)pCallbackParam;
 * 
 *     // Verify this requires a master session to be opened  
 *     if(type == TMWTYPES_SESSION_TYPE_MASTER)
 *     {
 *       MDNPSESN_CONFIG config;
 *       mdnpsesn_initConfig(&config);
 *
 *       // The src and dest addresses are as required for calling mdnpsesn_openSession()
 *       // ie the srcAddress was the destination address in the received message 
 *       config.source = srcAddress;
 *       config.destination = destAddress;
 *
 *       pSession = mdnpsesn_openSession(pChannel, &config, TMWDEFS_NULL);
 *       if(pSession == TMWDEFS_NULL)
 *       {
 *         // Failure
 *         return;
 *       }
 *     } else if(type == TMWTYPES_SESSION_TYPE_SLAVE)
 *     {
 *       SDNPSESN_CONFIG config;
 *       sdnpsesn_initConfig(&config);
 *
 *       // The src and dest addresses are as required for calling sdnpsesn_openSession()
 *       // ie the srcAddress was the destination address in the received message 
 *       config.source = srcAddress;
 *       config.destination = destAddress;
 *
 *       pSession = sdnpsesn_openSession(pChannel, &config, TMWDEFS_NULL);
 *       if(pSession == TMWDEFS_NULL)
 *       {
 *         // Failure
 *         return;
 *       } 
 *     }
 * 
 *     // Success
 *     return; 
 *   }
 */
typedef void (*TMWCHNL_AUTO_OPEN_FUNC)(
  void *pCallbackParam,
  TMWTYPES_USHORT srcAddress,
  TMWTYPES_USHORT destAddress,
  TMWTYPES_SESSION_TYPE type);

/*  Structure that holds internal information required
 *  to control a channel. A pointer to this is returned
 *  when a channel is opened and must be passed into 
 *  functions to update or close the channel.
 */
typedef struct TMWChannelStruct {
  /* List Member, must be first entry */
  TMWDLIST_MEMBER listMember;
  TMWCHNL_TYPE chanType;

  /* Pointer to application context */
  struct TMWApplicationStruct *pApplContext;

  /* Pointer to physical layer interface and context */
  const TMWPHYS_INTERFACE *pPhys;
  TMWPHYS_CONTEXT *pPhysContext;

  /* Pointer to link layer interface and context */
  const TMWLINK_INTERFACE *pLink;
  TMWLINK_CONTEXT *pLinkContext;

  /* Pointer to transport layer interface and context */
  const TMWTPRT_INTERFACE *pTprt;
  TMWTPRT_CONTEXT *pTprtContext;

  /* Statistics Callback */
  void *pStatCallbackParam;
  TMWCHNL_STAT_CALLBACK pStatCallbackFunc;

  /* Channel Idle Callback */
  void *pIdleCallbackParam;
  TMWCHNL_IDLE_CALLBACK pIdleCallbackFunc;  
  
  /* routine to call when a message has been received for a session that is
   * not currently open. This user provided function can determine whether or
   * not to open the session and allow the received message to be processed.
   */
  void                   *pAutoOpenCallbackParam;
  TMWCHNL_AUTO_OPEN_FUNC  pAutoOpenCallback;

  /* Lock channel for multi threaded applications */
#if TMWCNFG_SUPPORT_THREADS
  TMWTYPES_BOOL         lockOwner;
  TMWDEFS_RESOURCE_LOCK lock;
#endif

  /* Queue of request messages */
  TMWDLIST messageQueue;

  /* Number of request messages that can be queued */
  TMWTYPES_USHORT maxQueueSize; 
  TMWTYPES_USHORT numberQueued; 

  /* Current message outstanding on this channel */
  TMWSESN_TX_DATA *pCurrentMessage;
  TMWSESN_TX_DATA *pUserCallbackMsg;

  /* Timers */ 
  TMWTYPES_MILLISECONDS incrementalTimeout;
  TMWTIMER incrementalTimer;

  /* Time last byte of last message was transmitted */
  TMWDTIME lastByteTime;

  /* Channel in polled receive data mode, not event driven */
  TMWTYPES_BOOL polledMode;

  /* Channel in UDP Only mode.
   * This provides support for protocols other than DNP to send and receive over UDP.
   * NOTE: This is not part of the specification for those protocols.
   * DNP does not depend on this parameter as it is done as part of the protocol.
   */
  TMWTYPES_BOOL udpOnly;

  /* User Data*/
  void *pUserData;

#if TMWCNFG_MULTIPLE_TIMER_QS
  TMWTIMER_QUEUE timerQueue;
  void *pMultiTimerHandle;
  TMWTYPES_CALLBACK_FUNC pMultiTimerCallback;
#endif

  /* Link Redundancy Online status callback */
  TMWCHNL_LINK_RDCY_CALLBACK pLinkRdcyOnlineCallback;

  /* Diagnostic mask */
  TMWTYPES_ULONG chnlDiagMask;
} TMWCHNL;

#if TMWCNFG_SUPPORT_STATS

#define TMWCHNL_STAT_CALLBACK_FUNC(pChannel, eventType, pEventData) \
  tmwchnl_callStatCallback(pChannel, eventType, pEventData)
#else

#define TMWCHNL_STAT_CALLBACK_FUNC(pChannel, eventType, pEventData) \
  TMWTARG_UNUSED_PARAM(pChannel); TMWTARG_UNUSED_PARAM(eventType); TMWTARG_UNUSED_PARAM(pEventData);
#endif


#ifdef __cplusplus
extern "C" {
#endif

  /* function: tmwchnl_initChannel 
   * purpose:
   * arguments:
   * returns:
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwchnl_initChannel(
    TMWAPPL *pApplContext,
    TMWCHNL *pChannel,
    TMWCHNL_STAT_CALLBACK pStatCallback,
    void *pStatCallbackParam,
    TMWCHNL_IDLE_CALLBACK pIdleCallback,
    void *pIdleCallbackParam,
    TMWCHNL_AUTO_OPEN_FUNC pAutoOpenCallback,
    void *pAutoOpenCallbackParam,
    TMWTYPES_BOOL lockOwner,
    TMWCHNL_TYPE chanType,
    TMWTYPES_ULONG chnlDiagMask);

  /* function: tmwchnl_deleteChannel
   * purpose:
   * arguments:
   * returns:
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwchnl_deleteChannel(
    TMWCHNL *pChannel);

  /* function: tmwchnl_getChannelName
   * purpose: returns the name of this channel
   * arguments:
   *  pChannel - pointer to channel structure
   * returns:
   *  pointer to a string which contains the name of this
   *  channel.
   */
  TMWDEFS_SCL_API const TMWTYPES_CHAR * TMWDEFS_GLOBAL tmwchnl_getChannelName(
    TMWCHNL *pChannel);
  
  /* function: tmwchnl_setUserDataPtr
   * purpose: Tell the channel to store this pointer to user data
   *   This can be used by the target application code for easier lookup
   *   into data related to the SCL channel
   * arguments:
   *  pChannel - channel to set
   *  pUserData - pointer to user data. Will not be used by SCL
   * returns:
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwchnl_setUserDataPtr(
    TMWCHNL *pChannel, void *pUserData);

  /* function: tmwchnl_getUserDataPtr
   * purpose: Ask the channel for the pointer that was stored by tmwchnl_setUserDataPtr
   * arguments:
   *  pChannel - channel to get data from 
   * returns:
   *  pUserData - pointer to user data.
   */
  TMWDEFS_SCL_API void * TMWDEFS_GLOBAL tmwchnl_getUserDataPtr(
    TMWCHNL *pChannel);

  /* function: tmwchnl_setStatCallback
   * purpose: Register a callback function to called by the SCL
   *  when an statistic event occurs on a channel
   * arguments: 
   *  pChannel - pointer to channel structure
   *  pCallback - pointer to statistics callback function
   *  pCallbackParam - parameter to be passed to callback function
   * returns:
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwchnl_setStatCallback(
    TMWCHNL *pChannel,
    TMWCHNL_STAT_CALLBACK pCallback,
    void *pCallbackParam);

  /* function: tmwchnl_callStatCallback
   * purpose: Call the statistics callback function when a statistic
   *  event occurs.
   * arguments: 
   *  pChannel - pointer to channel structure
   *  eventType - statistic event that occurred
   *  pEventData - pointer to event specific data
   * returns:
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwchnl_callStatCallback(
    TMWCHNL *pChannel,
    TMWCHNL_STAT_EVENT eventType,
    void *pEventData);

  /* function: tmwchnl_setIdleCallback
   * purpose: Register a callback function to be called by the SCL
   *  when a channel becomes idle. (No more data to send or response expected)
   * arguments:
   *  pChannel - pointer to channel structure
   *  pCallback - pointer to idle callback function
   *  pCallbackParam - parameter to be passed to callback function
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL tmwchnl_setIdleCallback(
    TMWCHNL *pChannel,
    TMWCHNL_IDLE_CALLBACK pCallback,
    void *pCallbackParam);

  /* function: tmwchnl_lockChannel
   * purpose: Lock a channel for multi threaded applications
   * arguments:
   *  pChannel - channel to lock
   * returns:
   *  void
   */
  void tmwchnl_lockChannel(
    TMWCHNL *pChannel);

  /* function: tmwchnl_lockChannel
   * purpose: Unlock channel for multi threaded applications
   * arguments:
   *  pChannel - channel to lock
   * returns:
   *  void
   */
  void tmwchnl_unlockChannel(
    TMWCHNL *pChannel);

#ifdef __cplusplus
}
#endif

#endif /* __midl */

#endif /* TMWCHNL_DEFINED */
