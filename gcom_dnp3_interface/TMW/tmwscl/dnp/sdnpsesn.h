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

/* file: sdnpsesn.h
 * description: Defines structures and functions to support slave DNP3 
 *  sessions.
 */
#ifndef SDNPSESN_DEFINED
#define SDNPSESN_DEFINED

#include "tmwscl/utils/tmwdefs.h"
#include "tmwscl/utils/tmwsesn.h"
#include "tmwscl/dnp/sdnpcnfg.h"
#include "tmwscl/dnp/sdnpdata.h"

#define UNSOL_NORETRY TMWDEFS_DAYS(31UL)

/* DNP SECURE AUTHENTICATION Configuration 
 * The following configuration structure may be ignored if Secure Authentication is 
 * not supported
 */

#if SDNPCNFG_SUPPORT_SA_VERSION2
/* Secure Authentication Per user configuration structure */
typedef struct SDNPAuthUserConfig {
  /* User Number */
  TMWTYPES_USHORT userNumber;
} SDNPSESN_AUTH_USERCONFIG;
#endif

/* Secure Authentication Configuration structure */
typedef struct SDNPAuthConfig {
 
  /* Association Id. Sent in error message to master. When error message is sent 
   * to a different master, this identifies which association error occurred on.
   */
  TMWTYPES_USHORT assocId;

  /* MAC algorithm to sent in challenge requests sent by Outstation.
   * Master will use this algorithm to generate the MAC value in the challenge
   * reply. The Outstation will then use this same algorithm to generate a MAC to
   * authenticate the challenge reply. The MAC algorithm the Master is configured
   * to send in a challenge does NOT have to match the MAC Algorithm the Outstation
   * sends in a challenge.
   *  1 DNPAUTH_HMAC_SHA1_4OCTET   (SHA1 truncated to    4 octets   Only for SA V2)
   *  2 DNPAUTH_MAC_SHA1_10OCTET   (SHA1 truncated to   10 octets)
   *  3 DNPAUTH_MAC_SHA256_8OCTET  (SHA256 truncated to  8 octets)
   *  4 DNPAUTH_MAC_SHA256_16OCTET (SHA256 truncated to 16 octets)
   *  5 DNPAUTH_MAC_SHA1_8OCTET    (SHA1 truncated to    8 octets)
   *  6 DNPAUTH_MAC_AESGMAC_12OCTET
   */
  TMWTYPES_UCHAR   MACAlgorithm;
  
  /* Key wrap algorithm to use to decrypt and "unwrap" session keys
   * Deprecated on session, now per user.
   * TMWTYPES_UCHAR   keyWrapAlgorithm;
   */

  /* Number of consecutive application layer timeouts before declaring a 
   * Communications Failure Event. You may choose to set this to the same 
   * value as maxErrorCount in SAv2 or Max Reply Timeout statistic in SAv5, 
   * but this is separately configured and counted.
   */
  TMWTYPES_UCHAR  maxApplTimeoutCount;

  /* How long to wait for any authentication reply 
   * Default shall be 2 seconds 
   */
  TMWTYPES_MILLISECONDS replyTimeout;

  /* Expected session key interval and count. When this amount of time elapses
   * or this quantity of Authentication messages are sent or received, the session 
   * keys for this user will be invalidated.
   * Interval and count should be 2 times the master key change interval and count.
   * For systems that communicate infrequently, keyChangeInterval may be set to  
   * zero, using only the maxKeyChangeCount to determine when keys should be 
   * considered old and should be invalidated.
   *  default 30 minutes and 2000 messages.
   */
  TMWTYPES_MILLISECONDS keyChangeInterval;
  TMWTYPES_USHORT       maxKeyChangeCount;
  
  /* Agressive mode is enabled, allows aggressive mode requests from master */
  TMWTYPES_BOOL        aggressiveModeSupport;

  /* Version 5 requires ability to disallow SHA1 */
  TMWTYPES_BOOL        disallowSHA1;

  /* Extra diagnostics including plain key data before it is encrypted 
   * or after it is decrypted
   */
  TMWTYPES_BOOL        extraDiags;

  /* extra configuration to assist in testing. */
  TMWTYPES_ULONG       testConfig;
  
  /* Secure Authentication Version 2 */ 
  TMWTYPES_BOOL        operateInV2Mode;

  /* These have been added as configuration in addition to modes being 
   * set by functions 
   */
  TMWTYPES_BOOL        authSendAggrResp;
  TMWTYPES_BOOL        authSendAggrUnsol;
  TMWTYPES_BOOL        preChallengeApplConf;

#if SDNPCNFG_SUPPORT_SA_VERSION2 
  /* Number of errors messages to be sent before disabling error message 
   * transmission.
   * default shall be 2 
   * range 0-255
   */
  TMWTYPES_UCHAR       maxErrorCount; 

  /* Additional configuration for each user  
   * Specification says default user number is 1, configure it as first user in 
   * array. Add any other user numbers. For each user number, the database must 
   * contain an update key.
   */
  SDNPSESN_AUTH_USERCONFIG authUsers[DNPCNFG_AUTHV2_MAX_NUMBER_USERS];
#endif

#if SDNPCNFG_SUPPORT_SA_VERSION5 
  /* Length of random challenge data to send in g120v1 challenge and g120v5 
   * key status.
   * TB2016-002 says Minimum length==4, Maximum length==64
   */
  TMWTYPES_USHORT  randomChallengeDataLength;

  /* SAv5 (1815-2012) 7.6.1.4.6 says that if this number of Session Key Status 
   * Requests within the Expected Session Key Change Interval, the outstation 
   * shall alert a human.
   * If a different DNP association is in use, the outstation shall send an Error 
   * (g120v7) event object on that association with the code <12> Max Session Key 
   * Status Requests Exceeded. This value shall be configurable up to a maximum of 
   * 255 or down to 2. The default value shall be 5.
   */
  TMWTYPES_USHORT  maxSessionKeyStatusCount;

  /* These 4 maximum values are in addition to the thresholds for the individual 
   * statistics. The fifth one, maxKeysDueToRestarts, is on master.
   * This is made clear in the Secure Authentication Test Procedures. 
   * The outstation will take special actions when a statistic exceeds these Max Values. 
   */
  TMWTYPES_USHORT  maxAuthenticationFailures;
  TMWTYPES_USHORT  maxReplyTimeouts;
  TMWTYPES_USHORT  maxAuthenticationRekeys;
  TMWTYPES_USHORT  maxErrorMessagesSent;
#endif

} SDNPSESN_AUTH_CONFIG;
  
/* Application layer Function code enabled for broadcast and device specific address
 * bit values. These can be or'ed together in enabledBroadcastFCs and enabledFCs to 
 * indicate which FCs are enabled for broadcast and device specific addresses.
 */
#define SDNPSESN_ENABLE_FC_CONFIRM              (1<<DNPDEFS_FC_CONFIRM) 
#define SDNPSESN_ENABLE_FC_READ                 (1<<DNPDEFS_FC_READ) 
#define SDNPSESN_ENABLE_FC_WRITE                (1<<DNPDEFS_FC_WRITE) 
#define SDNPSESN_ENABLE_FC_SELECT               (1<<DNPDEFS_FC_SELECT) 
#define SDNPSESN_ENABLE_FC_OPERATE              (1<<DNPDEFS_FC_OPERATE)
#define SDNPSESN_ENABLE_FC_DIRECT_OP            (1<<DNPDEFS_FC_DIRECT_OP)
#define SDNPSESN_ENABLE_FC_DIRECT_OP_NOACK      (1<<DNPDEFS_FC_DIRECT_OP_NOACK)
#define SDNPSESN_ENABLE_FC_FRZ                  (1<<DNPDEFS_FC_FRZ)
#define SDNPSESN_ENABLE_FC_FRZ_NOACK            (1<<DNPDEFS_FC_FRZ_NOACK)
#define SDNPSESN_ENABLE_FC_FRZ_CLEAR            (1<<DNPDEFS_FC_FRZ_CLEAR)
#define SDNPSESN_ENABLE_FC_FRZ_CLEAR_NOACK      (1<<DNPDEFS_FC_FRZ_CLEAR_NOACK)
#define SDNPSESN_ENABLE_FC_FRZ_TIME             (1<<DNPDEFS_FC_FRZ_TIME)
#define SDNPSESN_ENABLE_FC_FRZ_TIME_NOACK       (1<<DNPDEFS_FC_FRZ_TIME_NOACK)
#define SDNPSESN_ENABLE_FC_COLD_RESTART         (1<<DNPDEFS_FC_COLD_RESTART)
#define SDNPSESN_ENABLE_FC_WARM_RESTART         (1<<DNPDEFS_FC_WARM_RESTART)
#define SDNPSESN_ENABLE_FC_INIT_DATA            (1<<DNPDEFS_FC_INIT_DATA_TO_DFLTS)
#define SDNPSESN_ENABLE_FC_INIT_APP             (1<<DNPDEFS_FC_INIT_APPLICATION)
#define SDNPSESN_ENABLE_FC_START_APP            (1<<DNPDEFS_FC_START_APPLICATION)
#define SDNPSESN_ENABLE_FC_STOP_APP             (1<<DNPDEFS_FC_STOP_APPLICATION)
#define SDNPSESN_ENABLE_FC_SAVE_CONFIG          (1<<DNPDEFS_FC_SAVE_CONFIGURATION)
#define SDNPSESN_ENABLE_FC_ENABLE_UNSOL         (1<<DNPDEFS_FC_ENABLE_UNSOL)
#define SDNPSESN_ENABLE_FC_DISABLE_UNSOL        (1<<DNPDEFS_FC_DISABLE_UNSOL)
#define SDNPSESN_ENABLE_FC_ASSIGN_CLASS         (1<<DNPDEFS_FC_ASSIGN_CLASS)
#define SDNPSESN_ENABLE_FC_DELAY_MEASURE        (1<<DNPDEFS_FC_DELAY_MEASURE)
#define SDNPSESN_ENABLE_FC_RECORD_CTIME         (1<<DNPDEFS_FC_RECORD_CURRENT_TIME)
#define SDNPSESN_ENABLE_FC_OPEN_FILE            (1<<DNPDEFS_FC_OPEN_FILE)
#define SDNPSESN_ENABLE_FC_CLOSE_FILE           (1<<DNPDEFS_FC_CLOSE_FILE)
#define SDNPSESN_ENABLE_FC_DELETE_FILE          (1<<DNPDEFS_FC_DELETE_FILE)
#define SDNPSESN_ENABLE_FC_GET_FILE_INFO        (1<<DNPDEFS_FC_GET_FILE_INFO)
#define SDNPSESN_ENABLE_FC_AUTHENTICATE         (1<<DNPDEFS_FC_AUTHENTICATE)
#define SDNPSESN_ENABLE_FC_ABORT                (1<<DNPDEFS_FC_ABORT)
#define SDNPSESN_ENABLE_FC_ACTIVATE_CONFIG      (1<<DNPDEFS_FC_ACTIVATE_CONFIG) 

/* If FC Write is supported for broadcast, further specify which writes
 * as indicated in Device Profile for broadcast support
 */
#define SDNPSESN_ENABLE_WRITE_CLOCK             0x01 
#define SDNPSESN_ENABLE_WRITE_LRTIME            0x02
#define SDNPSESN_ENABLE_WRITE_CRESTART          0x04 
#define SDNPSESN_ENABLE_WRITE_OTHER             0x08 

/* Slave DNP Session Configuration Info */
typedef struct SDNPSessionConfigStruct {

  /* Source address (outstation address)for this session */
  TMWTYPES_USHORT source;

  /* Destination address (master address) for this session
   * If validateSourceAddress is TMWDEFS_TRUE, this will be the
   * address to compare the masters source address to.
   * If validateSourceAddress is TMWDEFS_FALSE, this address
   * will be used as the address to send unsolicited responses.
   * All other responses will be sent to the source address received
   * from the master
   */
  TMWTYPES_USHORT destination;

  /* How often to send link status requests
   * if no DNP3 frames have been received on this session.
   * In DNP3 IP Networking spec this is called keep-alive interval
   * Enabling keep-alives is REQUIRED when using TCP
   * A value of zero will turn off keep alives.
   */
  TMWTYPES_MILLISECONDS linkStatusPeriod;

  /* Disconnect/reconnect a connection when link status request times out.
   * The spec says to do this when using TCP, however when configuring multiple
   * sessions over a single TCP channel you may not want this behavior when a
   * single session times out. This is probably more likely when configuring
   * a master session.
   */
  TMWTYPES_BOOL linkStatusTimeoutDisconnect;

  /* Specify whether or not to validate source address in received
   * frames. DNP3 frames contain both a source address field and a
   * destination address field. If TMWDEFS_FALSE the TMW DNP3 SCL does not
   * validate the source address and frames whose destination address 
   * matches a configured slave session will be accepted.
   * Setting this to TMWDEFS_TRUE requires both source and destination
   * addresses to match a local slave session before the frame is accepted.
   */
  TMWTYPES_BOOL validateSourceAddress;

  /* Specify whether or not to enable self address functionality on this
   * this slave device as specified by DNP Technical Bulletin 2003-003 
   * Self-Address Reservation. Slave will respond to address 0xfffc as
   * though it received a request for its configured address. It will 
   * respond with its own address so the master can automatically 
   * discover the slave address
   */
  TMWTYPES_BOOL enableSelfAddress;

  /* Is this session 'active'. Inactive sessions will not transmit
   * frames or process received frames.
   */
  TMWTYPES_BOOL active;

  /* Specify whether application is allowed to send multi fragment responses.
   */
  TMWTYPES_BOOL multiFragRespAllowed;

  /* Specify whether application layer confirmations will be requested
   * for non-final fragments of a multi fragment response. Setting this to
   * TMWDEFS_FALSE would now be considered NON-COMPLIANT. Application layer
   * confirmations are always requested for responses that contain events.
   */
  TMWTYPES_BOOL multiFragConfirm;
  
  /* Parsing Table says send qualifier 5b for activate config response. 
   * For backward compatibility with old implementations send qualifier 7 instead.
   */
  TMWTYPES_BOOL sendObj91RespQual7;

  /* Specifies whether this device will set the Need Time IIN bit
   * in response to this session at startup and after the clock valid
   * period has elapsed. If this bit is set the master will respond
   * with a time synchronization request. Typically this parameter 
   * should be true for one session for each slave device. Set this
   * parameter to TMWDEFS_FALSE if report by exception is not supported 
   * or there is no reason this device needs to be synchronized from
   * the master.
   */
  TMWTYPES_BOOL respondNeedTime;

  /* Specifies how long the local clock will remain valid after receiving
   * a time synchronization. (or after sdnpsesn_restartClockValidTime is called)
   */
  TMWTYPES_MILLISECONDS clockValidPeriod; 
  
  /* Application confirm timeout specifies how long the slave DNP device will
   * wait for an application layer confirmation from the master for a 
   * SOLICITED response. 
   */
  TMWTYPES_MILLISECONDS applConfirmTimeout;

  /* Unsolicited confirm timeout specifies how long the slave DNP device will
   * wait for an application layer confirmation from the master for an 
   * UNSOLICITED response. This in combination with unsolRetryDelay or
   * unsolOfflineRetryDelay will determine how frequently an unsolicited response 
   * will be resent.
   */
  TMWTYPES_MILLISECONDS unsolConfirmTimeout;

  /* selectTimeout specifies the maximum amount of time that a select
   * will remain valid before the corresponding operate is received. If
   * an operate request is received after this period has elapsed since
   * the previous select the select will not be valid and the operate
   * request will fail.
   */
  TMWTYPES_MILLISECONDS selectTimeout;

  /* The time, in ms, a master must wait after receiving a response to a
   * warm restart request. This value is encoded in a time delay fine object
   * in the response which limits it to 65535 ms.
   */
  TMWTYPES_USHORT warmRestartDelay;

  /* The time, in ms, a master must wait after receiving a response to a
   * cold restart request. This value is encoded in a time delay fine object
   * in the response which limits it to 65535 ms.
   */
  TMWTYPES_USHORT coldRestartDelay;

  /* Determines whether multiple Control Relay Output Block objects are
   * allowed in a single request. The DNP3 specification allows this and
   * assumes that each operation will be performed in serial. Some devices
   * may want to limit CROB objects to one per request to simplify processing.
   *  NOTE: If this is set to TMWDEFS_FALSE, Analog Control Objects will not
   *  be allowed in the same request as the single CROB object.
   */
  TMWTYPES_BOOL allowMultiCROBRequests;

  /* Determines the maximum number of controls allowed in a single request.
   * This applies to both Binary (CROB) and Analog Control outputs. This must 
   * be less than SDNPCNFG_MAX_CONTROL_REQUESTS. If allowMultiCROBRequests
   * is set to TMWDEFS_FALSE this won't matter for CROBs.
   */
  TMWTYPES_UCHAR maxControlRequests;
  
  /* Bit mask indicating which Writes are enabled for broadcast requests if
   * broadcast Write is enabled
   */ 
  TMWTYPES_UCHAR enabledBroadcastWrites;

  /* Bit mask indicating which Writes are enabled for device specific requests,
   * if device specific Write is enabled
   */ 
  TMWTYPES_UCHAR enabledWrites;

  /* Bit mask indicating which Function Codes are enabled for broadcast requests */
  TMWTYPES_ULONG enabledBroadcastFCs;

  /* Bit mask indicating which Function Codes are enabled for 
   * device specific requests
   */
  TMWTYPES_ULONG enabledFCs;

  /* If set to TMWDEFS_FALSE, reject all file transfer requests
   * even if file transfer code is compiled in.
   */
  TMWTYPES_BOOL fileTransferEnabled;

  /* If a file transfer operation is in progress (i.e. a file is open) this
   * specifies the maximum amount of time, in ms, allowed between operations.
   * If no file transfer operations occur in this period the file will be
   * closed and future file transfer operations on this file will fail.
   */
  TMWTYPES_MILLISECONDS fileTransferTimeout;

  /* Determines whether unsolicited responses are allowed. If unsolAllowed
   * is set to TMWDEFS_FALSE no unsolicited responses will be generated and
   * requests to enable or disable unsolicited responses will fail.
   */
  TMWTYPES_BOOL unsolAllowed;
  
  /* Determines whether unsolicited null responses will be sent when session
   * comes "online" (meaning connected). Specs say send initial unsolicited 
   * null response on restart.
   * Previous versions of SCL would also send unsolicited null response when
   * a session reconnected. Add this configuration to allow user to maintain
   * that behavior by setting this to TMWDEFS_TRUE if desired.
   */
  TMWTYPES_BOOL sendUnsolWhenOnline;
   
  /* Determines whether to try to send identical unsolicited retries. 
   * If set to TMWDEFS_FALSE, regenerated unsolicited retries will be sent.
   * The DNP Specification allows for either identical retries with the
   * same events and sequence number or regenerated retries which may 
   * contain additional new events, different IIN bits, etc, with a 
   * different sequence number.
   * If set to TMWDEFS_TRUE, identical retries will be sent until confirmed by the 
   * master or until the retry sequence is completed. Set unsolOfflineRetryDelay
   * to UNSOL_NORETRY to limit identical or regenerated retries to unsolMaxRetries 
   */
  TMWTYPES_BOOL unsolSendIdenticalRetry;

  /* Specify the initial/new state of the unsolicited event mask. This mask
   * is used to determine which event class(es) will generate unsolicited 
   * responses. According to the DNP specification, unsolicited responses 
   * should be disabled until an 'Enable Unsolicited Response' request is 
   * received from the master. Hence this value should generally be
   * TMWDEFS_CLASS_MASK_NONE, but some masters do not generate the 'Enable
   * Unsolicited Response' message, in which case they must be enabled here.
   */
  TMWDEFS_CLASS_MASK unsolClassMask;

  /* Determines whether an initial Null unsolicited response is sent.
   * According to the DNP Specification if unsolicited responses are supported
   * "unsolAllowed==true" then an initial null UR shall be sent until an 
   * application confirm is received from the master. Some masters do not handle 
   * the initial UR Null response but do support URs.
   * Setting this to true will disable the sending of the initial Null UR.  
   */
  TMWTYPES_BOOL unsolDontSendInitialNull;

  /* If unsolicited responses are enabled, unsolClassXMaxEvents specifies 
   * the maximum number of events in the corresponding class to be allowed 
   * before an unsolicited response will be generated. 
   */
  TMWTYPES_UCHAR unsolClass1MaxEvents;
  TMWTYPES_UCHAR unsolClass2MaxEvents;
  TMWTYPES_UCHAR unsolClass3MaxEvents;

  /* If unsolicited responses are enabled, unsolClassXMaxDelay specifies 
   * the maximum amount of time in milliseconds after an event in the
   * corresponding class is received before an unsolicited response will
   * be generated.
   */
  TMWTYPES_MILLISECONDS unsolClass1MaxDelay;
  TMWTYPES_MILLISECONDS unsolClass2MaxDelay;
  TMWTYPES_MILLISECONDS unsolClass3MaxDelay;

  /* Specify the maximum number of unsolicited retries before changing to
   * the 'offline' retry period described below. This parameter allows you
   * to specify up to 65535 retries. If you want an infinite number of
   * retries at the same time period, set unsolOfflineRetryDelay to the 
   * same value as unsolRetryDelay.
   */
  TMWTYPES_USHORT unsolMaxRetries;

  /* Specifies the time, in milliseconds, to delay after an unsolicited confirm
   * timeout before retrying the unsolicited response.
   */
  TMWTYPES_MILLISECONDS unsolRetryDelay;

  /* Specifies the time, in milliseconds, to delay after an unsolicited
   * timeout before retrying the unsolicited response after unsolMaxRetries
   * have been attempted. To disable retries after unsolMaxRetries set this
   * value to UNSOL_NORETRY or TMWDEFS_DAYS(31) which means this unsolicited 
   * will not be retried. A new unsolicited response may be generated when
   * conditions warrant.
   */
  TMWTYPES_MILLISECONDS unsolOfflineRetryDelay;

  /* Specifies whether to continue using unsolOfflineRetryDelay value or to
   * use it for one time period and then revert back to unsolRetryDelay till
   * unsolMaxRetries is once again exceeded.
   */
  TMWTYPES_BOOL unsolOfflineDelayOnce;

  /* Default variations
   * Specifies the variation that will be used for unsolicited responses
   * and in response to an integrity poll or a read requesting variation 0.
   * 
   * If any of these configuration variables is set to zero a function
   * sdnpdata_xxxDefVariation will be called for each point of that 
   * object group to determine the default variation for that point.
   */

  /* If this is TMWDEFS_TRUE, only unsolicited events in the class whose timer
   * timer expires or count is exceeded will be sent. Otherwise unsolicited
   * events in all classes will be sent.
   */
  TMWTYPES_BOOL       unsolSendByClass;

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
  
  /* Object groups included in response to read static data request 
   * "empty" entries should be set to 0. The entries do not have to 
   * be in the first x entries, ie {1,0,21,40,0,0,0,0...} is valid.
   * and would indicate Binary Inputs, Frozen Counters and Analog Output 
   * Statuses should be included in a response to a read static data request.
   */
  TMWTYPES_UCHAR staticGroups[SDNPCNFG_MAX_NUMBER_STATIC_GROUPS];

  /* If this is TMWDEFS_TRUE event queueing and retrieval will be handled outside
   * of the SCL through the sdnpdata_umEventxxx() functions. When this option is
   * used, the user provided code must implement max queue sizes, event modes, 
   * whether to delete the oldest or newest event on overflow, etc.
   * SDNPCNFG_USER_MANAGED_EVENTS must be set to TMWDEFS_TRUE at compile time.
   */
  TMWTYPES_BOOL userManagedEvents;

  /* If this is TMWDEFS_TRUE events will be sorted by timestamp when
   * xxx_addEvent is called. This was the desired behavior in IEEE1815-2012
   * This is no longer the correct behavior. The new version of 1815-202x(1?)
   * Tables 5-4 and 5-5 clearly show the order events are queued is the order
   * they should be sent in responses, NOT by timestamp.
   */
  TMWTYPES_BOOL sortEventsByTime;

  /* If this is TMWDEFS_TRUE timestamps are provided for DNP3 events.
   * If this is set to TMWDEFS_FALSE event variations without time will be sent 
   * for events even if with time variations are requested. 
   */
  TMWTYPES_BOOL supportEventsWithTime;

  /* If this is TMWDEFS_TRUE the event with the earliest timeStamp will be 
   * deleted when a new event is added to an event queue that is full.
   */
  TMWTYPES_BOOL deleteOldestEvent;

  /* Diagnostic mask */
  TMWTYPES_ULONG sesnDiagMask;
  
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
  /* Frozen Counter Event Configuration */
  TMWTYPES_USHORT frozenAnalogInMaxEvents;
  TMWDEFS_EVENT_MODE frozenAnalogInEventMode;
  TMWTYPES_MILLISECONDS frozenAnalogInScanPeriod;
#endif

#if SDNPDATA_SUPPORT_OBJ88
  /* Data Set Snapshot Event Configuration */
  TMWTYPES_USHORT datasetMaxEvents;
  TMWDEFS_EVENT_MODE datasetEventMode;
#endif
  
#if SDNPDATA_SUPPORT_OBJ110
  /* Set this to TRUE to truncate string to length specified by read variation,
   * which is the normally desired behavior. Setting this to false may send
   * a longer string than the variation specified. ie reading group 110 
   * variation 10 would either truncate all strings to at most 10 bytes,
   * or if this parameter is FALSE would allow a longer string to be returned.
   * This behavior would be more like reading variation 0.
   */
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

  /* See description of truncateStrings */
  TMWTYPES_BOOL truncateVirtualTerminalEvents;
  TMWDEFS_EVENT_MODE virtualTerminalEventMode;
  TMWTYPES_MILLISECONDS virtualTerminalScanPeriod;
#endif

#if SDNPDATA_SUPPORT_OBJ115
  /* Extended String Event Configuration */
  TMWTYPES_USHORT extStringMaxEvents;
  TMWDEFS_EVENT_MODE extStringEventMode;
  TMWTYPES_MILLISECONDS extStringScanPeriod;
#endif

  /* User registered statistics callback function and parameter */
  TMWSESN_STAT_CALLBACK pStatCallback;
  void *pStatCallbackParam;

#if SDNPDATA_SUPPORT_OBJ120 
  TMWTYPES_BOOL         authenticationEnabled;
  TMWTYPES_USHORT       authSecStatMaxEvents; 
  TMWDEFS_EVENT_MODE    authSecStatEventMode;
  SDNPSESN_AUTH_CONFIG  authConfig;
#endif

#if SDNPDATA_SUPPORT_XML2
  /* 
   * If this is TMWDEFS_TRUE, the per point value, quality and timestamp
   * called dnpData in the Device Profile schema will be generated, in addition
   * to the current configuration values. SDNPDATA_SUPPORT_XML2_DNPDATA must 
   * be defined as TMWDEFS_TRUE also.
   */
  TMWTYPES_BOOL       xml2WriteDnpData;
#endif

#if SDNPCNFG_DISABLE_SELOP_SEQ_CHECK
  /* Disable checking for consecutive application sequence numbers required 
   * between Select and Operate requests.
   * This will allow only read requests between the select and operate requests  
   * NOTE: This SDNP device will fail the DNP3 Conformance Tests if configured 
   * this way.
   */
  TMWTYPES_BOOL disableSelOpSeqCheck;
#endif

#if SDNPDATA_SUPPORT_STATIC_VAR_POINT || SDNPDATA_SUPPORT_EVENT_VAR_POINT
  /* If this is TMWDEFS_TRUE allow per point specification of precision of counter
   * and frozen counter points and events.
   * This allows the database to indicate a point value should be sent as a 
   * 16 or 32 bit variation even when read as the other variation as allowed in 
   * the new 1815-202x.
   * For example a read from the master of variation 1 32-bit could now result 
   * in the response using variation 2 to indicate only 16 bit counter precision
   * (or the reverse) is to be used for that point. This functionality is an 
   * extension to the ability to specify per point variation when variation zero
   * is requested from the master using the same database interface functionality.
   * obj20DefaultVariation, obj21DefaultVariation, obj22DefaultVariation, and/or
   * obj23DefaultVariation must each also be set to zero to enable this for running
   * counter, frozen counter and associated event groups.
   */
  TMWTYPES_BOOL counterSizeVarPerPoint;

  /* If this is TMWDEFS_TRUE allow per point specification of precision of analog 
   * and frozen analog input points and events.
   * This allows the database to indicate a point value should be sent as a 16 or
   * 32 bit variation even when read as the other variation as allowed in the 
   * new 1815-202x.
   * For example a read from the master of variation 1 32-bit could now result 
   * in the response using variation 2 to indicate 16 bit analog precision 
   * (or the reverse) is to be used for that point. This also applies to the 
   * variations for Single and Double precision if supported. This functionality
   * is an extension to the ability to specify per point variation when variation
   * zero is requested from the master using the same database interface functionality. 
   * obj30DefaultVariation, obj31DefaultVariation, obj32DefaultVariation, and/or 
   * obj33DefaultVariation must each also be set to zero to enable this for  
   * analog input, frozen analog input and associated event groups.
   */
  TMWTYPES_BOOL analogInSizeVarPerPoint;

  /* If this is TMWDEFS_TRUE allow per point specification of precision of 
   * analog output points and events.
   * This allows the database to indicate a point value should be sent as a 16 
   * or 32 bit variation even when read as the other variation as allowed in 
   * the new 1815-202x.
   * For example a read from the master of variation 1 32-bit could now result 
   * in the response using variation 2 to indicate 16 bit analog precision 
   * (or the reverse) is to be used for that point. This also applies to the 
   * variations for Single and Double precision if supported. This functionality
   * is an extension to the ability to specify per point variation when variation
   * zero is requested from the master using the same database interface functionality. 
   * obj40DefaultVariation, obj42DefaultVariation, and/or obj43DefaultVariation must
   * each also be set to zero to enable this for analog output status and associated
   * event groups.
   */
  TMWTYPES_BOOL analogOutSizeVarPerPoint;
#endif

} SDNPSESN_CONFIG;

/* DEPRECATED SHOULD USE sdnpsesn_getSessionConfig and 
 *  sdnpsesn_setSessionConfig
 */
#define SDNPSESN_CONFIG_SOURCE            0x00000001
#define SDNPSESN_CONFIG_DESTINATION       0x00000002
#define SDNPSESN_CONFIG_UNSOL_ALLOWED     0x00000004
#define SDNPSESN_CONFIG_UNSOL_MAX_EVENTS  0x00000008
#define SDNPSESN_CONFIG_SELECT_TIMEOUT    0x00000010
#define SDNPSESN_CONFIG_UNSOL_MAX_DELAY   0x00000020
#define SDNPSESN_CONFIG_ACTIVE            0x00000040
#define SDNPSESN_CONFIG_CNFM_TIMEOUT      0x00000080
#define SDNPSESN_CONFIG_MULTI_FRAG        0x00000100
#define SDNPSESN_CONFIG_UNSOL_MASK        0x00000200
#define SDNPSESN_CONFIG_UNSOL_RETRIES     0x00000400
#define SDNPSESN_CONFIG_UNSOL_DELAY       0x00000800
#define SDNPSESN_CONFIG_UNSOL_OFFDELAY    0x00001000
#define SDNPSESN_CONFIG_STATIC_GROUPS     0x00002000

/* Include slave DNP3 'private' structures and functions */
#include "tmwscl/dnp/sdnpsesp.h"

#ifdef __cplusplus
extern "C" {
#endif

  /* function: sdnpsesn_initConfig
   * purpose: Initialize DNP3 slaver session configuration data structure.
   *  This routine should be called to initialize all the members of the
   *  data structure. Then the user should modify the data members they
   *  need in user code. Then pass the resulting structure to 
   *  sdnpsesn_openSession.
   * arguments:
   *  pConfig - pointer to configuration data structure to initialize
   * returns:
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL sdnpsesn_initConfig(
    SDNPSESN_CONFIG *pConfig);

  /* function: sdnpsesn_openSession
   * purpose: Open and DNP3 slave session
   * arguments:
   *  pChannel - channel to open session on
   *  pConfig - DNP3 slave configuration data structure
   *  pUserHandle - handle passed to session database initialization routine
   * returns:
   *  Pointer to new session or TMWDEFS_NULL.
   */
  TMWDEFS_SCL_API TMWSESN * TMWDEFS_GLOBAL sdnpsesn_openSession(
    TMWCHNL *pChannel,
    const SDNPSESN_CONFIG *pConfig, 
    void *pUserHandle);

  /* function: sdnpsesn_applyBinaryFileValues
   purpose: Initialize DNP3 channel configuration data structures and session
   *  configuration data structures using the values in a binary configuration
   *  file. The structures are first initialized to default values just like they
   *  are by dnpchnl_initConfig() and sdnpsesn_initConfig().  The values found in the
   *  binary configuration file are then applied over the default values.
   *  The user should call this routing to initialize these data structures
   *  and then modify the desired fields before calling dnpchnl_openChannel
   *  to actually open the desired channel and sdnpsesn_openSession to open a session
   *  on the channel.
   * arguments:
   *  pFileName - full path to the DNP3 binary configuration file
   *  pDNPConfig - pointer to DNP channel configuration, note that some
   *   parameters in this structure will override values in the transport,
   *   link, and physical layer configurations.
   *  pLinkConfig - pointer to link layer configuration
   *  pIOConfig - pointer to target layer configuration
   *  pSesnConfig - pointer to configuration session configuration
   * returns:
   *  TMWDEFS_TRUE if successful
   */ 
  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsesn_applyBinaryFileValues (
    char * pFileName, 
    DNPCHNL_CONFIG *pDNPConfig,
    DNPLINK_CONFIG *pLinkConfig, 
    void *pIOConfig,
    SDNPSESN_CONFIG *pSesnConfig);

  /* function: sdnpsesn_getSessionConfig
   * purpose:  Get current configuration from a currently open session
   * arguments:
   *  pSession - session to get configuration from
   *  pConfig - dnp slave configuration data structure to be filled in
   * returns:
   *  TMWDEFS_TRUE if successful
   */
  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsesn_getSessionConfig(
    TMWSESN *pSession,
    SDNPSESN_CONFIG *pConfig);

  /* function: sdnpsesn_setSessionConfig
    * purpose: Modify a currently open session
   *  NOTE: normally sdnpsesn_getSessionConfig() will be called
   *   to get the current config, some values will be changed 
   *   and this function will be called to set the values.
   * arguments:
   *  pSession - session to modify
   *  pConfig - dnp slave configuration data structure
   * returns:
   *  TMWDEFS_TRUE if successful
   */
  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsesn_setSessionConfig(
    TMWSESN *pSession,
    const SDNPSESN_CONFIG *pConfig);
 
  /* function: sdnpsesn_modifySession 
   *  DEPRECATED FUNCTION, SHOULD USE sdnpsesn_setSessionConfig()
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsesn_modifySession(
    TMWSESN *pSession,
    const SDNPSESN_CONFIG *pConfig, 
    TMWTYPES_ULONG configMask);

  /* function: sdnpsesn_closeSession
   * purpose: Close a currently open session
   * arguments:
   *  pSession - session to close
   * returns:
   *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
   */
  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsesn_closeSession(
    TMWSESN *pSession);

  /* function: sdnpsesn_restartClockValidTime 
   * purpose: Restart the clock valid time. An application could call this if it
   *   had set the time using an external mechanism and wanted the SCL to consider
   *   the time to be valid for the configured clockValidPeriod.
   * arguments:
   *  pSession - pointer to session returned by sdnpsesn_openSession
   * returns:
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL sdnpsesn_restartClockValidTime(
    TMWSESN *pSession);

  /* function: sdnpsesn_addAuthUser
   * purpose: Add a Secure Authentication User
   * arguments:
   *  pSession - session 
   *  userNumber - user number for this user
   * returns:
   *  TMWDEFS_TRUE if successfull, else TMWDEFS_FALSE
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsesn_addAuthUser( 
    TMWSESN *pSession,
    TMWTYPES_USHORT userNumber); 

  /* function: sdnpsesn_getAuthUser
   * purpose: Get the Secure Authentication User Number from SCL for 
   *  specified index.
   * arguments:
   *  pSession - session 
   *  index - index of user to return user number for. 
   * returns:  
   *  userNumber or 0 if not found
   */
  TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpsesn_getAuthUser( 
    TMWSESN *pSession,
    TMWTYPES_USHORT index);

  /* function: sdnpsesn_removeAuthUser
   * purpose: Remove a Secure Authentication User from SCL
   * arguments:
   *  pSession - session 
   *  userNumber - user number to remove
   * returns:
   *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsesn_removeAuthUser( 
    TMWSESN *pSession,
    TMWTYPES_USHORT userNumber);

  /* function: sdnpsesn_authSendAggrResp
   * purpose: Specify whether SOLICITED responses sent to the master should have 
   *  the aggressive mode objects added. Only the default user number (1) is 
   *  used as required by the specification.
   * arguments:   
   *  pSession - pointer to session structure returned by sdnpsesn_openSession()
   *  sendAggressive - TMWDEFS_TRUE turns on aggressive mode sending
   *                   TMWDEFS_FALSE turns off aggressive mode sending
   * returns:   
   *  void
   */  
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL sdnpsesn_authSendAggrResp( 
    TMWSESN *pSession,
    TMWTYPES_BOOL sendAggressive); 
 
  /* function: sdnpsesn_authSendAggrUnsol
   * purpose: Specify whether UNSOLICITED responses sent to the master should have 
   *  the aggressive mode objects added. Only the default user number (1) is 
   *  used as required by the specification.
   * arguments:   
   *  pSession - pointer to session structure returned by sdnpsesn_openSession()
   *  sendAggressive - TMWDEFS_TRUE turns on aggressive mode sending
   *                   TMWDEFS_FALSE turns off aggressive mode sending
   * returns:   
   *  void
   */  
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL sdnpsesn_authSendAggrUnsol( 
    TMWSESN *pSession,
    TMWTYPES_BOOL sendAggressive); 

  /* function: sdnpsesn_authPreChallApplConf
   * purpose: Send a "preChallenge" in responses requesting application confirms.
   *  This includes responses containing events and ALL cases where an application
   *  confirm is requested including nonfinal fragments of a multifragment response.
   *  If outstation is going to challenge an application confirm, there can be a 
   *  problem if the master does not expect it. The master may send another 
   *  request, or if there is
   *  a half duplex line, there may be a collision. Setting this to TMWDEFS_TRUE 
   *  will cause a "preChallenge" G120V1 to be appended to the data the outstation 
   *  sends with the APPL CON bit set. The master should then send the application 
   *  confirm using aggressive mode. This means the outstation will not have to 
   *  challenge the confirm to verify the master. 
   *  This can be left set to TMWDEFS_TRUE, and ALL responses requesting confirms
   *  will be "pre-challenged" or this can be set to TMWDEFS_TRUE only when a 
   *  pre-challenge is desired and then set to TMWDEFS_FALSE, so no more 
   *  pre-challenges will be sent.
   * arguments:   
   *  pSession - pointer to session structure returned by sdnpsesn_openSession()
   *  preChallenge - TMWDEFS_TRUE if g120v1 is to be appended to responses 
   *                    containing request for application confirm
   *                 TMWDEFS_FALSE to not append g120v1 object to responses
   * returns:    
   *  void
   */  
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL sdnpsesn_authPreChallApplConf( 
    TMWSESN *pSession,
    TMWTYPES_BOOL preChallenge);

  /* function: sdnpsesn_requestApplConfirm
   * purpose: Function to do some common processing to request an application 
   *  confirm and also check to see if prechallenge is needed in response.
   *  pSession - pointer to session structure returned by sdnpsesn_openSession()
   *  pResponse - pointer to response being built. 
   * returns:
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL sdnpsesn_requestApplConfirm(
    TMWSESN *pSession,
    TMWSESN_TX_DATA *pResponse);

  /* function: sdnpsesn_failedUnsolTx
   * purpose: Called if transmission of unsolicited response failed.
   *  This includes if the connection is not open.
   * arguments:
   *  pSession - pointer to session structure returned by sdnpsesn_openSession()
   * returns:
   *  void
   */
  void TMWDEFS_CALLBACK sdnpsesn_failedUnsolTx(
    TMWSESN *pSession);


#ifdef __cplusplus
}
#endif
#endif /* SDNPSESN_DEFINED */
