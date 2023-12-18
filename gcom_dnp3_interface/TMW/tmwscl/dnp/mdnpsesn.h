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

/* file: mdnpsesn.h
 * description: Master DNP Session Definitions
 */
#ifndef MDNPSESN_DEFINED
#define MDNPSESN_DEFINED

#include "tmwscl/utils/tmwsesn.h"
#include "tmwscl/dnp/mdnpdata.h"

#define MDNPSESN_TX_DATA_BUFFER_MAX 2048

/* Define MDNP Session Configuration Structure */
typedef TMWTYPES_USHORT MDNPSESN_AUTO_REQ;
 
/* The Application Layer Spec Volume 2 Version 2.10 December 2007         
 * specifies some behavior the master should follow. The following allow the SCL
 * to be configured to automatically perform that behavior as well as the       
 * the ability to tailor the behavior as needed. The default values will be set  
 * to perform as spelled out in the application layer specification. Any changes
 * to the default behavior should be carefully considered        
 */

/* Maintain current values, since some applications may save the actual values  */ 
#define MDNPSESN_AUTO_CLEAR_RESTART       0x0001 /* Clear IIN restart bit       */
#define MDNPSESN_AUTO_INTEGRITY_RESTART   0x0002 /* Issue integrity data poll   */
                                                 /* on restart                  */
#define MDNPSESN_AUTO_INTEGRITY_LOCAL     0x0004 /* Issue integrity data poll   */
                                                 /* after local IIN bit was set */
                                                 /* and cleared                 */
#define MDNPSESN_AUTO_INTEGRITY_TIMEOUT   0x0008 /* Issue integrity data poll   */
                                                 /* on timeout                  */
#define MDNPSESN_AUTO_INTEGRITY_OVERFLOW  0x0010 /* Issue integrity data poll   */
                                                 /* on buffer overflow          */
#define MDNPSESN_AUTO_DELAY_MEAS          0x0020 /* Use delay measurement in    */
                                                 /* time sync                   */
#define MDNPSESN_AUTO_TIME_SYNC_SERIAL    0x0040 /* Perform time sync on need   */
                                                 /* time using serial method    */
                                                 /* Define for LAN method below */
#define MDNPSESN_AUTO_TIME_SYNC           MDNPSESN_AUTO_TIME_SYNC_SERIAL
                                                 /* For backward compatibility  */
                                                 /* keep this old define        */
#define MDNPSESN_AUTO_EVENT_POLL          0x0080 /* Issue event data poll when  */
                                                 /* class 1, 2, or 3 IIN bit is */
                                                 /* set                         */
#define MDNPSESN_AUTO_ENABLE_UNSOL        0x0100 /* automatically enable        */
                                                 /* unsolicited events upon     */
                                                 /* remote or master device     */
                                                 /* startup                     */
#define MDNPSESN_AUTO_DISABLE_UNSOL       0x0200 /* automatically disable       */
                                                 /* unsolicited events upon     */
                                                 /* remote device startup, Not  */
                                                 /* necessary, outstation should*/
                                                 /* be disabled at startup      */ 
#define MDNPSESN_AUTO_CONFIRM             0x0400 /* Enable/Disable automatic    */
                                                 /* generation of application   */
                                                 /* layer confirmations         */
#define MDNPSESN_AUTO_TIME_SYNC_LAN       0x0800 /* Perform time sync on need   */
                                                 /* time using LAN method       */
#define MDNPSESN_AUTO_DATASET_RESTART     0x1000 /* Exchange data set prototypes*/
                                                 /* and descriptors with slave  */
                                                 /* when IIN restart bit is set */
                                                 /* or when master restarts.    */
#define MDNPSESN_AUTO_INTEGRITY_ONLINE    0x2000 /* Issue integrity data poll   */
                                                 /* when session becomes        */
                                                 /* "online", meaning connected */
#define MDNPSESN_AUTO_UNSOL_STARTUP       0x4000 /* Master Unsolicited Startup  */
                     /* Behavior, see autoRequestMask comment below for details.*/
 

/* DNP SECURE AUTHENTICATION Configuration 
 * The following configuration structure may be ignored if Secure Authentication is not supported
 */

#if DNPCNFG_SUPPORT_AUTHENTICATION
#if MDNPCNFG_SUPPORT_SA_VERSION2
/* MDNP Secure Authentication per user configuration structure */
typedef struct MDNPAuthUserConfig {
  /* User Number */
  TMWTYPES_USHORT userNumber;
  void *          userHandle;
} MDNPSESN_AUTH_USERCONFIG;
#endif

/* MDNP Secure Authentication Configuration structure */ 
typedef struct MDNPAuthConfig {

  /* MAC algorithm to sent in challenge requests sent by Master.
   * Outstation will use this algorithm to generate the MAC value in the challenge
   * reply. The Master will then use this same algorithm to generate a MAC to
   * authenticate the challenge reply. The MAC algorithm the Outstation is configured
   * to send in a challenge does NOT have to match the MAC Algorithm the Master 
   * sends in a challenge.
   *  1 DNPAUTH_HMAC_SHA1_4OCTET   (SHA1 truncated to    4 octets   Only for SA V2 and backward compatibility )
   *  2 DNPAUTH_MAC_SHA1_10OCTET   (SHA1 truncated to   10 octets)
   *  3 DNPAUTH_MAC_SHA256_8OCTET  (SHA256 truncated to  8 octets)
   *  4 DNPAUTH_MAC_SHA256_16OCTET (SHA256 truncated to 16 octets)
   *  5 DNPAUTH_MAC_SHA1_8OCTET    (SHA1 truncated to    8 octets)
   *  6 DNPAUTH_MAC_AESGMAC_12OCTET     
   *  6-127 reserved for future, 
   *  128-255 reserved for vendor specific choices 
   */ 
  TMWTYPES_UCHAR   MACAlgorithm;

  /* Number of consecutive application layer timeouts before declaring a 
   * Communications Failure Event. You may choose to set this to the same 
   * value as maxErrorCount in SAv2 or Max Reply Timeout statistic in SAv5, 
   * but this is separately configured and counted.
   */
  TMWTYPES_UCHAR  maxApplTimeoutCount;

  /* length of session keys to be generated and sent to the outstation */
  TMWTYPES_UCHAR   sessionKeyLength;

  /* keyWrapAlgorithm to be used to encrypt and "wrap" session keys is received 
   * from outstation in key status message and is not configurable on master 
   */

  /* How long to wait for any authentication reply 
   * default shall be 2 seconds
   */
  TMWTYPES_ULONG   responseTimeout;

  /* Session key interval in milliseconds.  
   * When time since last key change reaches this, session keys will be updated.
   * For systems that communicate infrequently, 
   * this may be set to zero, using only the maxKeyChangeCount to determine 
   * when to update keys. 
   *  Spec says range should be up to 1 week
   *  default 15 minutes
   */
  TMWTYPES_MILLISECONDS keyChangeInterval;

  /* Session Authentication ASDU count since last key change, When this number 
   *  of authentication ASDUs is transmitted or rcved since last key change, 
   *    Session keys will be updated.
   *  default shall be 1000 authentication ASDUs transmitted or received
   */ 
  TMWTYPES_USHORT  maxKeyChangeCount;
  
  /* Sent in error message. 
   * More meaningful when outstation sends error message to master.
   * Not very useful when sent by master to outstation.
   */
  TMWTYPES_USHORT assocId;

  /* Agressive mode is enabled */
  TMWTYPES_BOOL   aggressiveModeSupport;

  /* Bit mask to indicate which auto requests should be sent using Aggressive Mode
   * Depends on aggressiveModeSupport==TMWDEFS_TRUE and which autoRequestMask bits 
   * are set. This mask uses the same bits that autoRequestMask uses.
   * For example if autoRequestMask has MDNPSESN_AUTO_TIME_SYNC_LAN bit set
   * and autoRequestAggrModeMask has the same bit MDNPSESN_AUTO_TIME_SYNC_LAN set,
   * when a LAN time sync is automatically sent the library will use Aggressive Mode 
   * if it can.
   * NOTE: This will NOT send auto integrity polls, auto application confirm or  
   * auto data set read using Aggressive Mode even if those bits are set.
   * The following values (or OR'd combinations) are valid for this type:
   *  MDNPSESN_AUTO_CLEAR_RESTART = Clear IIN restart bit
   *  MDNPSESN_AUTO_INTEGRITY_RESTART = Not Supported
   *  MDNPSESN_AUTO_INTEGRITY_LOCAL = Not Supported
   *  MDNPSESN_AUTO_INTEGRITY_TIMEOUT = Not Supported
   *  MDNPSESN_AUTO_INTEGRITY_OVERFLOW = Not Supported
   *  MDNPSESN_AUTO_DELAY_MEAS = Not Supported
   *  MDNPSESN_AUTO_TIME_SYNC_SERIAL = Perform time sync on need time using serial method
   *  MDNPSESN_AUTO_EVENT_POLL = Automatic Event Poll when Class when class 1, 2, or 3 IIN bit is set
   *  MDNPSESN_AUTO_ENABLE_UNSOL = automatically enable unsolicited events upon remote or master device startup
   *  MDNPSESN_AUTO_DISABLE_UNSOL = automatically disable unsolicited events upon remote device startup, Not necessary, outstation should be disabled at startup
   *  MDNPSESN_AUTO_CONFIRM = Not Supported
   *  MDNPSESN_AUTO_TIME_SYNC_LAN = Perform time sync on need time using LAN method
   *  MDNPSESN_AUTO_DATASET_RESTART = Not Supported
   *  MDNPSESN_AUTO_INTEGRITY_ONLINE = Not Supported
   *  MDNPSESN_AUTO_UNSOL_STARTUP = Master Unsolicited Startup Behavior
   */
  MDNPSESN_AUTO_REQ autoRequestAggrModeMask;

  /* Version 5 requires ability to disallow SHA1 */
  TMWTYPES_BOOL   disallowSHA1;

  /* Extra diagnostics including plain key data before it is encrypted or after it is decrypted */
  TMWTYPES_BOOL   extraDiags;

  /* extra configuration to assist in testing. */
  TMWTYPES_ULONG        testConfig;

  /* Secure Authentication Version 2 */
  TMWTYPES_BOOL   operateInV2Mode;
  
  /* SA Version 5 says master must either send aggressive mode or delay after 
   * sending direct no ack in case the request is challenged. This is how long 
   * the master will delay before sending next request.
   */
  TMWTYPES_MILLISECONDS directNoAckDelayTime;

#if MDNPCNFG_SUPPORT_SA_VERSION2 
  /* Number of errors messages to be sent before disabling error message transmission, 
   * default shall be 2 
   * range 0-255
   */
  /* This has been replaced with a statistic in Secure Authentication version 4 */
  TMWTYPES_UCHAR  maxErrorCount;

  /* Configuration for each user  
   * Specification says default user number is 1, configure it as first user in array. 
   * Add any other user numbers. For each user number the database must contain an
   * update key.
   */
  MDNPSESN_AUTH_USERCONFIG authUsers[DNPCNFG_AUTHV2_MAX_NUMBER_USERS];
#endif

#if MDNPCNFG_SUPPORT_SA_VERSION5

  /* Length of random challenge data to send in g120v1 challenge request.
   * TB2016-002 says Minimum length==4, Maximum length==64
   */
  TMWTYPES_USHORT  randomChallengeDataLength;

  /* These 5 maximum values are used to determine special actions when the related statistics  
   * counts exceed these values. Reporting thresholds which are used on the outstation are not
   * applicable to the master.
   */
  TMWTYPES_USHORT  maxAuthenticationFailures;
  TMWTYPES_USHORT  maxReplyTimeouts;
  TMWTYPES_USHORT  maxAuthenticationRekeys;
  TMWTYPES_USHORT  maxErrorMessagesSent;
  TMWTYPES_USHORT  maxRekeysDueToRestarts;

  /* Master should treat all responses or unsoliciteds as critical and challenge them */
  TMWTYPES_BOOL   authCriticalResponses;
  TMWTYPES_BOOL   authCriticalUnsoliciteds;
#endif

} MDNPSESN_AUTH_CONFIG;
#endif

/* MDNP Session Configuration Structure */
typedef struct MDNPSessionConfigStruct {

  /* Source address (master address) for this session */
  TMWTYPES_USHORT source;

  /* Destination address (outstation address) for this session */
  TMWTYPES_USHORT destination;

  /* How often to send link status requests
   * if no DNP3 frames have been received on this session.
   * In DNP3 IP Networking spec this is called keep-alive interval
   * Enabling keep-alives is REQUIRED when using TCP
   * A value of zero will turn off keep alives.
   */
  TMWTYPES_MILLISECONDS linkStatusPeriod;

  /* Disconnect/reconnect a connection when link status request times out.
   * The spec says to do this when using TCP. However when configuring multiple
   * sessions over a single TCP channel to multiple serial devices you may not  
   * want to disconnect when an individual session does not respond. You may still
   * want to determine whether individual Outstations are responding.
   */
  TMWTYPES_BOOL linkStatusTimeoutDisconnect;

  /* Determine whether the session is to be active or inactive. An inactive 
   * session will not transmit or receive frames.
   */
  TMWTYPES_BOOL active;

  /* Mask used to enable/disable automatic processing of certain requests
   * based on internal events. If changing the default value of this field
   * explicitly set ALL of the bits that are desired, but only those bits.
   * For example:
   *   MDNPSESN_CONFIG config;
   *   mdnpsesn_initConfig(&config);
   *    Explicitly set the desired automatic behavior for this session
   *   config.autoRequestMask = MDNPSESN_AUTO_CLEAR_RESTART 
   *     | MDNPSESN_AUTO_DISABLE_UNSOL 
   *     | MDNPSESN_AUTO_CONFIRM;
   *
   * Note: Setting both the 
   *   MDNPSESN_AUTO_DISABLE_UNSOL and MDNPSESN_AUTO_ENABLE_UNSOL bits
   *   will cause just a DISABLE UNSOLICITED request to be sent followed
   *   by an ENABLE UNSOLICITED request. If  MDNPSESN_AUTO_CLEAR_RESTART
   *   and/or MDNPSESN_AUTO_INTEGRITY_RESTART are set, they will be sent
   *   after the DISABLE UNSOLICITED and before the ENABLE UNSOLICITED 
   *   requests.
   *                                            
   *   Application Layer Spec Part 2 Volume 2 Version 2.10 December 2007  
   *   Section 1.1.2 Master Startup and Part 3, Section 1.5 and Table 1.3  
   *   Master Unsolicited Response State Table says:
   *    Send Disable Unsolicited Request,  
   *    Send an integrity poll and discard any unsolicited 
   *      responses until after the integrity poll completes. 
   *    Send class assignments and analog deadbands if desired.
   *    Enable unsolicited responses
   *   If MDNPSESN_AUTO_UNSOL_STARTUP - An automatic disable unsolicited request 
   *    will be sent to the outstation. Unsolicited responses will be discarded 
   *    and not confirmed until an integrity poll is completed. This is required 
   *    by the application layer spec.
   *    This includes initial unsolicited NULL responses. To process unsolicited 
   *    null responses even before the integrity poll is complete, set 
   *    MDNPDATA_DISCARD_NULL_UNSOL define to TMWDEFS_FALSE at compile time.
   *   The following configuration also controls the automatic exchanges on master 
   *    restart:
   *    If MDNPSESN_AUTO_DATASET_RESTART -An automatic exchange of data set 
   *      prototypes and descriptors will be performed. Setting this causes the 
   *      exchange to take place before an integrity poll and before unsoliciteds 
   *      are enabled. This helps assure that data set descriptors are present 
   *      before events can be received.
   *    If MDNPSESN_AUTO_INTEGRITY_ONLINE -An integrity poll will be performed.
   *    If MDNPSESN_AUTO_ENABLE_UNSOL -An enable unsolicited will be issued using
   *       autoEnableUnsolClass1,2,3 to determine which classes to enable.
   *    
   *  If the application needs to send class assignments, analog deadbands or other
   *    commands before the Integrity Poll or Enable Unsolicited, the application 
   *    can clear MDNPSESN_AUTO_INTEGRITY_ONLINE and/or MDNPSESN_AUTO_ENABLE_UNSOL
   *    and queue the desired sequence of commands in the order and with the 
   *    priority desired.
   * 
   */ 
  MDNPSESN_AUTO_REQ autoRequestMask;

  /* If MDNPSESN_AUTO_ENABLE_UNSOL is set, these three flags will indicate which 
   * event classes should be enabled for unsolicited reporting
   */
  TMWTYPES_BOOL autoEnableUnsolClass1;
  TMWTYPES_BOOL autoEnableUnsolClass2;
  TMWTYPES_BOOL autoEnableUnsolClass3; 

  /* Default absolute response timeout. This is the default value for the 
   * response timeout for any request generated on this session. This value
   * can be overridden on a per command basis by changing the responseTimeout
   * value in the MDNPBRM_REQ_DESC data structure. This value is the absolute
   * maximum amount of time this device will wait for the final response to
   * a request. This time starts as soon as the request is put into the 
   * transmit queue.
   */
  TMWTYPES_MILLISECONDS defaultResponseTimeout;

  /* Amount of time to delay after receiving a response to a select request
   * before sending any request other than an operate. If MDNPBRM_AUTO_MODE 
   * operate is used, the mdnp library will not send a request before the 
   * operate request. If however, a user application sends a select request
   * without using MDNPBRM_AUTO_MODE operate and autoRequestMask is non zero, 
   * or Secure Authentication is enabled, the mdnp library could send a request 
   * before the user application called the brm function to send the operate request. 
   * Setting this to a nonzero value will delay the automatic sending of requests
   * by this master library for up to that many milliseconds.
   * A value of zero means no delay will be enforced by the library.
   * Call mdnpbrm_cancelSelOpDelayTimer() to cancel the delay timer early.
   */
  TMWTYPES_MILLISECONDS selOpDelayTime;
  
  /* Number of times a read request is allowed to timeout before the session
   * is considered offline. This is normally set to 0.
   * Setting this to a value larger than 0 can result in failure to recognize stale 
   * values (i.e., data can be stale by n*polling interval)
   */
  TMWTYPES_UCHAR  readTimeoutsAllowed;

  /* Diagnostic mask */
  TMWTYPES_ULONG  sesnDiagMask;

#if MDNPDATA_SUPPORT_OBJ91
  /* If multiple file objects are sent in Activate Config request combine
   * them to use just a single object header. Some outstations may not support
   * this even though allowed by the spec.
   */
  TMWTYPES_BOOL   combineActConfigData;
#endif
  
#if MDNPDATA_SUPPORT_OBJ70
  /* Maximimum block size to use during file transfer when
   * mdnpbrm_copyRemoteFileToLocal or mdnpbrm_copyLocalFileToRemote
   * functions are used. This must be less than max tx or rx fragment 
   * size minus 16. Max size can be specified in mdnpbrm_fileOpen if
   * application is managing file transfer block by block.
   */
  TMWTYPES_USHORT maxFileBlockSize;
#endif

  /* User registered statistics callback function and parameter */
  TMWSESN_STAT_CALLBACK pStatCallback;
  void *pStatCallbackParam;

#if MDNPDATA_SUPPORT_OBJ120 
  TMWTYPES_BOOL        authenticationEnabled;
  MDNPSESN_AUTH_CONFIG authConfig;
#endif
   
} MDNPSESN_CONFIG;

/* DEPRECATED SHOULD USE mdnpsesn_getSessionConfig and 
 *  mdnpsesn_setSessionConfig
 */
#define MDNPSESN_CONFIG_SOURCE        0x00000001L
#define MDNPSESN_CONFIG_DESTINATION   0x00000002L
#define MDNPSESN_CONFIG_ACTIVE        0x00000004L
#define MDNPSESN_CONFIG_AUTO          0x00000008L
#define MDNPSESN_CONFIG_RESP_TIMEOUT  0x00000010L

/* Define support for an unsolicited response callback. This callback
 *  function is specified per session and is called whenever the 
 *  session receives an unsolicited response.
 */
typedef struct MDNPSessionUnsolRespStruct {
  TMWSESN *pSession;
  TMWSESN_RX_DATA *pRxData;
  DNPCHNL_RESP_STATUS status;
  TMWTYPES_USHORT iin;
} MDNPSESN_UNSOL_RESP_INFO;

/* Define the unsolicited callback method */
typedef void (*MDNPSESN_UNSOL_CALLBACK_FUNC)(
  void *pCallbackParam,
  MDNPSESN_UNSOL_RESP_INFO *pResponse);


typedef struct MDNPSessionSARespStruct {
  TMWSESN *pSession;
  TMWSESN_RX_DATA *pRxData;
  TMWTYPES_UCHAR status;
  TMWTYPES_USHORT iin;
} MDNPSESN_SA_RESP_INFO;

/* Define the SA callback method */
typedef void(*MDNPSESN_SA_CALLBACK_FUNC)(
  void *pCallbackParam,
  MDNPSESN_SA_RESP_INFO *pResponse);


/* Include master DNP3 'private' structures and functions */
#include "tmwscl/dnp/mdnpsesp.h"

#ifdef __cplusplus
extern "C" {
#endif

  /* function: mdnpsesn_initConfig
   * purpose: Initialize DNP3 master session configuration data structure.
   *  This routine should be called to initialize all the members of the
   *  data structure. Then the user should modify the data members they
   *  need in user code. Then pass the resulting structure to 
   *  mdnpsesn_openSession.
   * arguments:
   *  pConfig - pointer to configuration data structure to initialize
   * returns:
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL mdnpsesn_initConfig(
    MDNPSESN_CONFIG *pConfig);


  /* function: mdnpsesn_applyBinaryFileValues
   purpose: Applies values found in a binary configuration file to DNP3 channel 
   *  configuration data structures and session configuration data structures. 
   *  The structures must first be initialized to default values using dnpchnl_initConfig()
   *  and mdnpsesn_initConfig().  After the binary file values are applied call dnpchnl_openChannel
   *  to actually open the desired channel and mdnpsesn_openSession to open a session
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
  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpsesn_applyBinaryFileValues (
    char * pFileName,
    DNPCHNL_CONFIG *pDNPConfig,
    DNPLINK_CONFIG *pLinkConfig, 
    void *pIOConfig,
    MDNPSESN_CONFIG *pSesnConfig);

  /* function: mdnpsesn_openSession
   * purpose: Open and DNP3 master session
   * arguments:
   *  pChannel - channel to open session on
   *  pConfig - DNP3 master configuration data structure
   *  pUserHandle - handle passed to session database initialization routine
   * returns:
   *  Pointer to new session or TMWDEFS_NULL.
   */
  TMWDEFS_SCL_API TMWSESN * TMWDEFS_GLOBAL mdnpsesn_openSession(
    TMWCHNL *pChannel,
    const MDNPSESN_CONFIG *pConfig, 
    void *pUserHandle);

  /* function: mdnpsesn_getSessionConfig  
   * purpose:  Get current configuration from an open session
   * arguments:
   *  pSession - session to get configuration from
   *  pConfig - dnp master configuration data structure to be filled in
   * returns:
   *  TMWDEFS_TRUE if successful
   */
  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpsesn_getSessionConfig(
    TMWSESN *pSession,
    MDNPSESN_CONFIG *pConfig);

  /* function: mdnpsesn_setSessionConfig 
   * purpose: Modify a currently open session
   *  NOTE: normally mdnpsesn_getSessionConfig() will be called
   *   to get the current config, some values will be changed 
   *   and this function will be called to set the values.
   * arguments:
   *  pSession - session to modify
   *  pConfig - dnp master configuration data structure
   * returns:
   *  TMWDEFS_TRUE if successful
   */
  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpsesn_setSessionConfig(
    TMWSESN *pSession,
    const MDNPSESN_CONFIG *pConfig);
 
  /* function: mdnpsesn_modifySession 
   *  DEPRECATED FUNCTION, SHOULD USE mdnpsesn_setSessionConfig()
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpsesn_modifySession(
    TMWSESN *pSession,
    const MDNPSESN_CONFIG *pConfig, 
    TMWTYPES_ULONG configMask);
  
  /* function: mdnpsesn_closeSession
   * purpose: Close a currently open session
   * arguments:
   *  pSession - session to close
   * returns:
   *  TMWDEFS_TRUE if successfull, else TMWDEFS_FALSE
   */
  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpsesn_closeSession(
    TMWSESN *pSession);

  /* function: mdnpsesn_sendConfirmation
   * purpose: Send and application layer confirmation to the specified
   *  session. Note that the TMW SCL application layer generally performs
   *  this action automatically but this feature can be disabled by changing
   *  the session's autoRequestMask configuration parameter.
   * arguments:
   *  pSession - session to send confirmation to
   *  seqNumber - sequence number to put in message
   *  unsol - TMWDEFS_TRUE if this is a confirmation to an unsolicited 
   *   response
   * returns:
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL mdnpsesn_sendConfirmation(
    TMWSESN *pSession, 
    TMWTYPES_UCHAR seqNumber,
    TMWTYPES_BOOL unsol);

  /* function: mdnpsesn_getSequenceNumber
   * purpose: Retrieve the current response sequence number. In general
   *  the TMW SCL handles sequence number internally but this function is
   *  provided for the user to access the current application sequence
   *  nunmber.
   * arguments:
   *  pSession - session to get the sequence number from
   * returns:
   *  the current application layer sequence number
   */
  TMWDEFS_SCL_API TMWTYPES_UCHAR TMWDEFS_GLOBAL mdnpsesn_getSequenceNumber(
    TMWSESN *pSession);

  /* function: mdnpsesn_setSequenceNumber
   * purpose: Set the current response sequence number. In general the 
   *  TMW SCL handles sequence number internally but this function is
   *  provided for the user to access the current application sequence
   *  nunmber.
   * arguments:
   *  pSession - session to set the sequence number on
   *  seqNumber - the new sequence number
   * returns:
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL mdnpsesn_setSequenceNumber(
    TMWSESN *pSession, 
    TMWTYPES_UCHAR seqNumber);

  /* function: mdnpsesn_setUnsolUserCallback
   * purpose: Specify a user callback to be invoked whenever an unsolicited
   *  response is received from the remote device.
   * arguments:
   *  pSession - session to add callback to
   *  pCallback - pointer to user callback function
   *  pCallbackParam - pointer to user supplied callback parameter
   * returns:
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL mdnpsesn_setUnsolUserCallback(
    TMWSESN *pSession,
    MDNPSESN_UNSOL_CALLBACK_FUNC pCallback,
    void *pCallbackParam);
  
  /* function:mdnpsesn_cancelSelOpDelayTimer
   * purpose:
   *  If the application is going to be delayed sending the OPERATE command (for 
   *  example waiting for human input) you can configure a nonzero mdnp session  
   *  selOpDelayTime to delay the sending of other commands until the OPERATE is sent
   *  or the timeout is reached. This function will cancel that timer if it is running so
   *  that other commands can be sent.
   * arguments:
   *  pSession - pointer to a master DNP session
   * returns:
   *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE 
   */
  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpsesn_cancelSelOpDelayTimer(
    TMWSESN *pSession);

  /* Secure Authentication code */
   
  /* Secure Authentication Version 5 code */

  /* function: mdnpsesn_addAuthUser
   * purpose: Add a Secure Authentication User
   * arguments:
   *  pSession - session 
   *  userNumber - user number for this user
   * returns:
   *  TMWDEFS_TRUE if successfull, else TMWDEFS_FALSE
   */
  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpsesn_addAuthUser(
    TMWSESN *pSession,
    TMWTYPES_USHORT userNumber); 

  /* function: mdnpsesn_getAuthUser
   * purpose: Get the Secure Authentication User Number from SCL for specified index.
   * arguments:
   *  pSession - session 
   *  index - index of user to return user number for. 
   * returns:  
   *  userNumber or 0 if not found
   */
  TMWDEFS_SCL_API TMWTYPES_USHORT TMWDEFS_GLOBAL mdnpsesn_getAuthUser(
    TMWSESN *pSession,
    TMWTYPES_USHORT index);

  /* function: mdnpsesn_removeAuthUser
   * purpose: Remove a Secure Authentication User from SCL
   * arguments:
   *  pSession - session 
   *  userNumber - user number to remove
   * returns:
   *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
   */
  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpsesn_removeAuthUser(
    TMWSESN *pSession,
    TMWTYPES_USHORT userNumber);

  /* function: mdnpsesn_setSAUnsolCallback
   * purpose: Specify a user callback to be invoked whenever an SA
   *  response is received from the remote device.
   * arguments:
   *  pSession - session to add callback to
   *  pCallback - pointer to user callback function
   *  pCallbackParam - pointer to user supplied callback parameter
   * returns:
   *  void
  */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL mdnpsesn_setSAUserCallback(
    TMWSESN *pSession,
    MDNPSESN_SA_CALLBACK_FUNC pCallback,
    void *pCallbackParam);

  /* function: mdnpsesn_getAuthCSQ
   * purpose: Get the current SA Challenge Sequence number sent in a challenge g120v1 request by the master.
   * It will be incremented before the next challenge is sent.
   * The TMW SCL manages the sequence number internally but this function is 
   * provided for the user to access the current sequence number for test purposes.
   */
  TMWDEFS_SCL_API TMWTYPES_ULONG TMWDEFS_GLOBAL mdnpsesn_getAuthCSQ(
    TMWSESN *pSession);

  /* function: mdnpsesn_setAuthCSQ 
   * purpose:  Set the current SA Challenge Sequence number sent in a challenge g120v1 request by the master.
   * It will be incremented before the next challenge is sent.
   * The TMW SCL manages the sequence number internally but this function is 
   * provided for the user to access the current sequence number for test purposes.
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL mdnpsesn_setAuthCSQ(
    TMWSESN *pSession,
    TMWTYPES_ULONG csq);

  /* function: mdnpsesn_getAuthAggrCSQ
   * purpose: Get the current SA Challenge Sequence Number sent in an aggressive mode request by the master. 
   * It will be incremented before the next aggressive mode is sent.
   * The TMW SCL manages the sequence number internally but this function is 
   * provided for the user to access the current sequence number for test purposes.
  */
  TMWDEFS_SCL_API TMWTYPES_ULONG TMWDEFS_GLOBAL mdnpsesn_getAuthAggrCSQ(
    TMWSESN *pSession);

  /* function: mdnpsesn_setAuthAggrCSQ
   * purpose:  Set the current SA Challenge Sequence Number sent in an aggressive mode request by the master. 
   * It will be incremented before the next aggressive mode request is sent.
   * The TMW SCL manages the sequence number internally but this function is 
   * provided for the user to access the current sequence number for test purposes.
  */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL mdnpsesn_setAuthAggrCSQ(
    TMWSESN *pSession,
    TMWTYPES_ULONG csq);


#ifdef __cplusplus
}
#endif
#endif /* MDNPSESN_DEFINED */
