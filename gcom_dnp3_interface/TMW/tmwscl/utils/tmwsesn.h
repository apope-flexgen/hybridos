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
/* application, check your licegnse agreement to ensure it allows use on the  */
/* product in question.  Contact Triangle MicroWorks for information about   */
/* extending the number of products that may use this source code library or */
/* obtaining the newest revision.                                            */
/*                                                                           */
/*****************************************************************************/

/* file: tmwsesn.h
 * description: Definition of a generic session
 */
#ifndef TMWSESN_DEFINED
#define TMWSESN_DEFINED
#ifndef __midl
#include "tmwscl/utils/tmwcnfg.h"
#include "tmwscl/utils/tmwdefs.h"
#include "tmwscl/utils/tmwdlist.h"
#include "tmwscl/utils/tmwtimer.h"
#endif

/* Define session information passed to the user */
typedef enum TMWSESN_STAT_EVENT_ENUM
{
  TMWSESN_STAT_ERROR,
  TMWSESN_STAT_ONLINE,
  TMWSESN_STAT_OFFLINE,
  TMWSESN_STAT_ASDU_SENT,
  TMWSESN_STAT_ASDU_RECEIVED,

  /* Currently only used by SDNP 
   * Indicates an event queue has overflowed and an event has been discarded
   * pEventData is a pointer to a TMWTYPES_UCHAR indicating what object group 
   * lost the event.
   */
  TMWSESN_STAT_EVENT_OVERFLOW,
  
  /* Currently only used by SDNP 
   * Indicates an event is being sent.
   * pEventData is a pointer to a TMWSESN_STAT_DNPEVENT indicating object group and point
   * number of event that is being sent. 
   */
  TMWSESN_STAT_DNPEVENT_SENT,

  /* Currently only used by SDNP 
   * Indicates an event has been confirmed by master
   * pEventData is a pointer to a TMWSESN_STAT_DNPEVENT indicating object group and point
   * number of event that was acked. 
   */  
   TMWSESN_STAT_DNPEVENT_CONFIRM,
 
  /* Currently only used by SDNP 
   * Indicates an unsolicited delay timer is being started
   * pEventData is a pointer to a TMWSESN_STAT_DNPUNSOLTIMER indicating event class and timer value
   * of delay timer being started. 
   */
  TMWSESN_STAT_DNPUNSOL_TIMER_START,

  TMWSESN_STAT_DNPUNSOL_SENT,
  TMWSESN_STAT_DNPUNSOL_CONFIRM,

  /* Used only by DNP
   * Indicates a link status request frame was received.
   */
  TMWSESN_STAT_DNPLINKSTATUSREQ_RECEIVED,

  /* Used only by DNP
   * Indicates a link status frame was received.
   */
  TMWSESN_STAT_DNPLINKSTATUS_RECEIVED,

  /* 
   * Indicates a request sent by a master has timed out before completing. 
   */
  TMWSESN_STAT_REQUEST_TIMEOUT,

  /* 
   * Indicates a request sent by a master received a failure response back from the outstation/slave.
   */
  TMWSESN_STAT_REQUEST_FAILED,

  /* Only used by I870, received message for an unknown sector 
   * This allows support for automatic open of sector 
   */
  TMWSESN_STAT_UNKNOWN_SECTOR,

  /* Only used by modbus, indicates exception response sent back to master */
  TMWSESN_STAT_EXCEPTION_RESP,

  /* Only used by modbus, indicates no response was sent back to master */
  TMWSESN_STAT_NO_RESPONSE,

  /* Currently only used by DNP3 Secure Authentication */
  /* Secure Authentication Message sent */
  TMWSESN_STAT_AUTH_SENT,
  /* Secure Authentication Message received */
  TMWSESN_STAT_AUTH_RCVD,
  /* Secure Authentication Response Timeout */
  TMWSESN_STAT_AUTH_RESPTIMEOUT, 
  /* Secure Authentication Key Change timer or count, time to send or receive new session keys */
  TMWSESN_STAT_AUTH_KEYCHANGE 

} TMWSESN_STAT_EVENT;


#ifndef __midl

typedef struct { 
  /* Object Group for event */
  TMWTYPES_UCHAR group;

  /* Point Number or index of point in event */
  TMWTYPES_ULONG point;
} TMWSESN_STAT_DNPEVENT;

typedef struct { 
  /* Class mask for this delay timer being started */
  TMWDEFS_CLASS_MASK eventClass;

  /* Value of delay timer in milliseconds */
  TMWTYPES_MILLISECONDS delay;
} TMWSESN_STAT_DNPUNSOLTIMER;

typedef void (*TMWSESN_STAT_CALLBACK)(
  /* pStatCallbackParam parameter provided when session was opened */
  void *pCallbackParam, 

  /* Enum indicating what statistics event */
  TMWSESN_STAT_EVENT eventType, 

  /* Extra information, specific to the particular TMWSESN_STAT_EVENT enum */
  void *pEventData);

/* Define data structure used to pass information to be 
 * transmitted.
 */
/* These first bits are used by i870 only */
#define TMWSESN_TXFLAGS_STORE_16BIT_TIME        0x0001
#define TMWSESN_TXFLAGS_STORE_56BIT_TIME        0x0002 
#define TMWSESN_TXFLAGS_IECDUPALLOWED           0x0004
#define TMWSESN_TXFLAGS_IEC_AUTH_AGG            0x0008 

/* reuse these bits, since DNP does not use the above defines */
#define TMWSESN_TXFLAGS_STORE_DNP_TIME          0x0001
#define TMWSESN_TXFLAGS_STORE_DNP_DELAY         0x0002
#define TMWSESN_TXFLAGS_DNP_AUTH                0x0004
#define TMWSESN_TXFLAGS_DNP_AUTH_AGGR           0x0008
#define TMWSESN_TXFLAGS_DNP_AUTH_CHALL          0x0010

#define TMWSESN_TXFLAGS_NO_RESPONSE             0x0020
#define TMWSESN_TXFLAGS_CONTAINS_EVENTS         0x0040
#define TMWSESN_TXFLAGS_LINK_COMMAND            0x0080
#define TMWSESN_TXFLAGS_WARM_RESTART            0x0100
#define TMWSESN_TXFLAGS_COLD_RESTART            0x0200
#define TMWSESN_TXFLAGS_MASTER                  0x0400
#define TMWSESN_TXFLAGS_SAVE_LAST_BYTE_TIME     0x0800
#define TMWSESN_TXFLAGS_BROADCAST               0x1000
#define TMWSESN_TXFLAGS_UNSOL                   0x2000
#define TMWSESN_TXFLAGS_CANCEL                  0x8000 

typedef struct TMWSessionTxDataStruct {
  /* List Member, must be first entry */
  TMWDLIST_MEMBER listMember;

  /* Pointer to channel to transmit to for broadcast messages */
  struct TMWChannelStruct *pChannel;

  /* Pointer to session to tranmsit to */
  struct TMWSessionStruct *pSession;

  /* Pointer to sector to transmit to */
  struct TMWSectorStruct *pSector; 

  /* To help prevent this structure from being used if it has been deallocated.
   * Especially needed at the user interface on a cancel request.
   */
  TMWTYPES_USHORT structureValid;

  /* Message description */
  const TMWTYPES_CHAR *pMsgDescription;

  /* Message buffer */
  TMWTYPES_UCHAR *pMsgBuf;
  TMWTYPES_USHORT msgLength;
  TMWTYPES_USHORT maxLength;
  TMWTYPES_USHORT transactId;

  /* Transmit flags */
  TMWTYPES_USHORT txFlags;

  /* Destination address */
  TMWTYPES_USHORT destAddress;

  /* Response timeout support */
  TMWTYPES_MILLISECONDS responseTimeout;
  TMWTIMER responseTimer;
 
  /* Time request was sent */
  TMWTYPES_MILLISECONDS timeSent;

  /* Callbacks */
  void *pCallbackData;
  void (*pBeforeTxCallback)(void *pUserData, struct TMWSessionTxDataStruct *pTxData);
  void (*pAfterTxCallback)(void *pUserData, struct TMWSessionTxDataStruct *pTxData);
  void (*pFailedTxCallback)(void *pUserData, struct TMWSessionTxDataStruct *pTxData);

  TMWTYPES_UCHAR spduType;

} TMWSESN_TX_DATA;

typedef struct TMWSessionRxDataStruct {

  /* Pointer to session received from */
  struct TMWSessionStruct *pSession;

  /* Message buffer */
  TMWTYPES_UCHAR *pMsgBuf;
  TMWTYPES_USHORT msgLength;
  TMWTYPES_USHORT maxLength;

  /* Time first byte was received */
  TMWTYPES_MILLISECONDS firstByteTime;
  TMWTYPES_MILLISECONDS lastByteTime;

  /* Was this message a broadcast message */
  TMWTYPES_BOOL isBroadcast;

  /* Received address, currently only used for broadcast messages */
  TMWTYPES_USHORT rxAddress;

  TMWTYPES_BOOL invalidFormat;

} TMWSESN_RX_DATA;


/* TMW Session */
typedef struct TMWSessionStruct {
  /* List Member, must be first entry */
  TMWDLIST_MEMBER listMember;

  /* Session protocol and type (master, slave, or peer) */
  TMWTYPES_PROTOCOL protocol;
  TMWTYPES_SESSION_TYPE type;

  /* Pointer to channel this session belongs to */
  struct TMWChannelStruct *pChannel;
  
  /* Pointer to link layer specific info */
  void *pLinkSession;

  /* Pointer to transport layer specific info */
  void *pTprtSession;

  /* Statistics Callback */
  void *pStatCallbackParam;
  TMWSESN_STAT_CALLBACK pStatCallbackFunc;

  /* Is session currently active */
  TMWTYPES_BOOL active;

  /* Is session currently online */
  TMWTYPES_UCHAR online;

  /* Misc parameters */
  TMWTYPES_USHORT srcAddress;
  TMWTYPES_USHORT destAddress;
  TMWTYPES_USHORT linkAddress;
  
  /* User Data*/
  void *pUserData;

  /* Diagnostic mask */
  TMWTYPES_ULONG sesnDiagMask;
} TMWSESN;


#if TMWCNFG_SUPPORT_STATS

#define TMWSESN_STAT_CALLBACK_FUNC(pSession, eventType, pEventData) \
  tmwsesn_callStatCallback(pSession, eventType, pEventData)
#else

#define TMWSESN_STAT_CALLBACK_FUNC(pSession, eventType, pEventData) \
  TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(eventType); TMWTARG_UNUSED_PARAM(pEventData);
#endif

#ifdef __cplusplus
extern "C" {
#endif

  /* function: tmwsesn_openSession */
  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL tmwsesn_openSession(
    struct TMWChannelStruct *pChannel,
    TMWSESN *pSession, 
    TMWSESN_STAT_CALLBACK pCallback,
    void *pCallbackParam,
    TMWTYPES_PROTOCOL protocol, 
    TMWTYPES_SESSION_TYPE type);

  /* function: tmwsesn_closeSession */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwsesn_closeSession(
    TMWSESN *pSession);

  /* function: tmwsesn_setUserDataPtr
   * purpose: Tell the session to store this pointer to user data
   *   This can be used by the target application code for easier lookup
   *   into data related to the SCL session
   * arguments:
   *  pSession - session to set
   *  pUserData - pointer to user data. Will not be used by SCL
   * returns:
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwsesn_setUserDataPtr(
    TMWSESN *pSession, void *pUserData);

  /* function: tmwsesn_getUserDataPtr
   * purpose: Ask the session for the pointer that was stored by tmwsesn_setUserDataPtr
   * arguments:
   *  pSession - session to get from 
   * returns:
   *  pUserData - pointer to user data.
   */
  TMWDEFS_SCL_API void * TMWDEFS_GLOBAL tmwsesn_getUserDataPtr(
    TMWSESN *pSession);

  /* function: tmwsesn_setStatCallback
   * purpose:
   * arguments:
   * returns:
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwsesn_setStatCallback(
    TMWSESN *pSession,
    TMWSESN_STAT_CALLBACK pCallback,
    void *pCallbackParam);
   
  /* function: tmwsesn_callStatCallback
   * purpose:
   * arguments:
   * returns:
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwsesn_callStatCallback(
    TMWSESN *pSession,
    TMWSESN_STAT_EVENT eventType,
    void *pEventData);

  /* function: tmwsesn_getSessionName
   * purpose: returns the name of this session
   * arguments:
   *  pSession - pointer to Session structure
   * returns:
   *  pointer to a string which contains the name of this
   *  session.
   */
  TMWDEFS_SCL_API const TMWTYPES_CHAR * TMWDEFS_GLOBAL tmwsesn_getSessionName(
    TMWSESN *pSession);

  /* function: tmwsesn_getChannelName
   * purpose: returns the name of the channel this session is on
   * arguments:
   *  pSession - pointer to Session structure
   * returns:
   *  pointer to a string which contains the name of the
   *  channel.
   */
  TMWDEFS_SCL_API const TMWTYPES_CHAR * TMWDEFS_GLOBAL tmwsesn_getChannelName(
    TMWSESN *pSession);

  /* function: tmwsesn_setOnline */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwsesn_setOnline(
    TMWSESN *pSession,
    TMWTYPES_BOOL online);

  /* function: tmwsesn_newTxData */
  TMWSESN_TX_DATA * TMWDEFS_GLOBAL tmwsesn_newTxData(
    TMWTYPES_UCHAR *pBuf,
    TMWTYPES_USHORT bufLen);
 
  /* function: tmwsesn_initTxData */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwsesn_initTxData(
    TMWSESN_TX_DATA *pTxData, 
    TMWTYPES_UCHAR *pBuf, 
    TMWTYPES_USHORT maxBufLen);

  /* function: tmwsesn_freeTxData */
  void TMWDEFS_GLOBAL tmwsesn_freeTxData(
    TMWSESN_TX_DATA *pTxData);

#ifdef __cplusplus
}
#endif

#endif /* __midl */

#endif /* TMWSESN_DEFINED */
