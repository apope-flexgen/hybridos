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

/* file: dnplink.h
 * description: DNP link layer implementation
 */
#ifndef DNPLINK_DEFINED
#define DNPLINK_DEFINED

#include "tmwscl/utils/tmwchnl.h"
#include "tmwscl/utils/tmwlink.h"
#include "tmwscl/utils/tmwcnfg.h"
#include "tmwscl/dnp/dnpcnfg.h"
#include "tmwscl/dnp/dnpdefs.h"

typedef enum {
  /* This should be chosen if dnp is used over a serial link with no
   *  IP networking support.
   *  NOTE: If this is selected tmwtarg_transmitUDP
   *        will not be called.
   */
  DNPLINK_NETWORK_NO_IP,

  /*
   * This should be chosen if TCP ONLY with no UDP functionality is
   * being used. TCP ONLY is for backward compatibility or if UDP
   * for broadcast is not feasible. 
   *  NOTE: If this is selected tmwtarg_transmitUDP
   *        will not be called.
   */
  DNPLINK_NETWORK_TCP_ONLY,

  /* This should be chosen if both TCP and UDP is being used. 
   * master devices will send broadcast messages over UDP and slaves
   * (outstations) should be able to receive UDP broadcast messages.
   */
  DNPLINK_NETWORK_TCP_UDP,

  /* This should be chosen if UDP ONLY is being used on a highly 
   * reliable network, or because of cost considerations.
   * All requests and all responses will be sent using UDP datagrams.
   */
  DNPLINK_NETWORK_UDP_ONLY
  
} DNPLINK_NETWORK_TYPE;

/* Define configuration parameters supported by this link layer 
 * implementation.
 */
typedef struct dnplinkConfig {

  /* Receive and transmit link layer frame size 
   * This includes room for link header and CRCs
   * maximum value 292
   */
  TMWTYPES_USHORT        rxFrameSize;
  TMWTYPES_USHORT        txFrameSize;

  /* Maximum amount of time to wait for a complete frame after
   * receiving valid frame sync characters
   */
  TMWTYPES_MILLISECONDS rxFrameTimeout;

  /* When should we ask for link layer confirmations. The 
   * options are:
   *  TMWDEFS_LINKCNFM_NEVER     (not for any frame)
   *  TMWDEFS_LINKCNFM_SOMETIMES (multi-frame fragments)
   *  TMWDEFS_LINKCNFM_ALWAYS    (for all frames)
   */
  TMWDEFS_LINKCNFM confirmMode;

  /* Maximum amount of time to wait for a link level confirm
   * if requested. Even if confirmMode is set to NEVER this will
   * still be used for Link Test Frame and Request Link Status 
   * if they are sent.
   */
  TMWTYPES_MILLISECONDS confirmTimeout;

  /* Maximum number of link layer retries if link layer confirm
   * times out.
   */
  TMWTYPES_UCHAR maxRetries;

  /* This parameter specifies how often a session that is offline will
   * attempt to reestablish communication. This includes attempting to
   * open/reopen a channel and/or issueing request status messages as
   * appropriate for the current configuration.
   */
  TMWTYPES_MILLISECONDS offlinePollPeriod;

  /* Specify networking support as described in DNP3 Specification - 
   * IP Networking. See enums above
   */
  DNPLINK_NETWORK_TYPE networkType;

} DNPLINK_CONFIG;

/* Define bit masks used to specify which configuration parameters
 * should be modified in a call to dnplink_modifyChannel.
 */
#define DNPLINK_CONFIG_RX_FRAME_SIZE      0x00000001
#define DNPLINK_CONFIG_RX_FRAME_TIMEOUT   0x00000002
#define DNPLINK_CONFIG_CONFIRM_MODE       0x00000004
#define DNPLINK_CONFIG_CONFIRM_TIMEOUT    0x00000008
#define DNPLINK_CONFIG_MAX_RETRIES        0x00000010

#define DNPLINK_CRC_SIZE 2
#define DNPLINK_HEADER_SIZE 10
#define DNPLINK_BLOCK_SIZE 18

typedef enum {
  DNP_LINK_STATE_IDLE,
  DNP_LINK_STATE_HEADER,
  DNP_LINK_STATE_USER_DATA,
  DNP_LINK_STATE_DISCARD_DATA
} DNP_LINK_STATE;

typedef void (*DNPLINK_DIAG_HEADER_FUNC)(
  void *pParam,
  TMWTYPES_MILLISECONDS firstByteTime,
  TMWTYPES_UCHAR *buf,
  TMWTYPES_USHORT *pLen);

typedef struct DNPLinkContext {
  /* Generic link layer info, must be first entry */
  TMWLINK_CONTEXT        tmw;

  /* Link layer configuration */
  TMWTYPES_USHORT        txFrameSize;
  TMWTYPES_USHORT        rxFrameSize;

  TMWTYPES_MILLISECONDS  rxFrameTimeout;

  /* Link layer confirmation support */
  TMWDEFS_LINKCNFM       confirmMode;
  TMWTYPES_MILLISECONDS  confirmTimeout;
  TMWTYPES_UCHAR         maxRetries;
  TMWTIMER               confirmTimer;
  TMWTYPES_USHORT        retryCount;

  DNPLINK_NETWORK_TYPE   networkType;

  /* Transmit info */
  TMWSESN               *pTxSession;
  TMWTYPES_USHORT        txMessageSize;
  TMWSESN_TX_DATA       *pTxDescriptor;
  TMWPHYS_TX_DESCRIPTOR  physTxDescriptor;
#if !TMWCNFG_USE_DYNAMIC_MEMORY || TMWCNFG_ALLOC_ONLY_AT_STARTUP 
  TMWTYPES_UCHAR         txFrameBuffer[DNPCNFG_MAX_TX_FRAME_LENGTH];
#endif

  /* separate descriptor and buffer for sending acks and nacks */
  TMWPHYS_TX_DESCRIPTOR  ackPhysTxDescriptor;
  TMWTYPES_UCHAR         ackTxFrameBuffer[10];

  /* Receive info */
  TMWTIMER               frameTimer;
  TMWTYPES_UCHAR         ctrl;
  TMWSESN               *pRxSession;
  DNP_LINK_STATE         rxState;
  void                  *pRxDiagContext;
  TMWTYPES_UCHAR         rxLength;
  TMWSESN_RX_DATA        rxFrame;
  TMWTYPES_USHORT        rxBlockOffset;
  TMWTYPES_USHORT        rxFrameOffset;
  TMWTYPES_UCHAR         pRxBlock[DNPLINK_BLOCK_SIZE];

#if TMWCNFG_SUPPORT_RXCALLBACKS
  DNPLINK_DIAG_HEADER_FUNC pRxHeaderCallback;
  void                    *pRxHeaderCallbackParam;
#endif

#if !TMWCNFG_USE_DYNAMIC_MEMORY || TMWCNFG_ALLOC_ONLY_AT_STARTUP 
  /* Note that this is to big but I don't have a macro to reliably convert
   * from link frame size to transport frame size.
   */
  TMWTYPES_UCHAR rxFrameBuffer[DNPCNFG_MAX_RX_FRAME_LENGTH];
#endif
} DNPLINK_CONTEXT;


/* DNP Link Level Session Info */
typedef struct {
  /* LinkIsReset in data link layer spec 2.8.2, used to be called localReset
   * If this is false, then the Secondary Station is in state UnReset in table 3.2
   * If this is true, then the Secondary Station is in state Idle 
   */
  TMWTYPES_BOOL linkIsReset;

  /* SecondaryStationIsReset in data link layer spec 2.8.1, used to be called remoteReset */
  TMWTYPES_BOOL secondaryStationIsReset;

  /* A Reset Link States primary frame has been sent, but not yet acked 
   * ResetLinkWait1/2 in the data link layer spec Table 3-1, used to be called resetPending.
   */
  TMWTYPES_BOOL resetLinkWait;

  /* User can call dnplink_linkReset requesting a link reset to be sent. If it cannot
   * be sent immediately, this is set to true, causeing it to be sent when it can be 
   */
  TMWTYPES_BOOL sendUserLinkReset;

  /* NFCB (Next FCB) in data link layer spec 2.8.1, used to be called txFrameCountBit */
  TMWTYPES_BOOL nextFCB;

  /* EFCB (Expected FCB) in data link layer spec 2.8.2, used to be called rxFrameCountBit */
  TMWTYPES_BOOL expectedFCB;

  /* Indicates if last secondary response was ACK or NACK, which will be
   * retransmitted if received FCB is not the expected one.
   */
  TMWTYPES_UCHAR lastSecondaryConfirmFuncCode;

  /* Timer to periodically send link status request */
  TMWTIMER linkStatusTimer;

} DNPLINK_SESSION_INFO;


#ifdef __cplusplus
extern "C" {
#endif

  /* function: dnplink_initConfig
   * purpose: Initialize a dnplink configuration data structure
   * arguments:
   *  pConfig - pointer to dnplink configuration structure to be
   *   initialized.
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL dnplink_initConfig(
    DNPLINK_CONFIG *pConfig);

  /* function: dnplink_initChannel
   * purpose: initialize a new channel.
   * arguments:
   *  pConfig - pointer to dnplink configuration information
   *  pPhys - pointer to physical layer interface to use
   *  pPhysContext - pointer to physical layer context
   * returns:
   *  A pointer to a context that is used to identify this channel
   *  for all future operations.
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL dnplink_initChannel(
    TMWCHNL *pChannel,
    const DNPLINK_CONFIG *pConfig);

  /* function: dnplink_reInitChannel
   * This is called if the txFrameSize of txFrameSize is increased for an open channel
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL dnplink_reInitChannel(
    TMWCHNL *pChannel,
    TMWTYPES_USHORT rxFrameSize,
    TMWTYPES_USHORT txFrameSize);

  /* function: dnplink_modifyChannel
   *  DEPRECATED FUNCTION, SHOULD USE dnpchnl_setChannelConfig()
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL dnplink_modifyChannel(
    TMWCHNL *pChannel,
    const DNPLINK_CONFIG *pConfig, 
    TMWTYPES_ULONG configMask);
  
  /* function: dnplink_startLinkStatusTimer
   * purpose: start link status timer if necessary.
   *  This allows modifying value from zero with no timer running to nonzero.
   * arguments:
   *  pSession - session
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL dnplink_startLinkStatusTimer(
    TMWSESN *pSession);

  /* function: dnplink_deleteChannel
   * purpose: delete this channel, freeing all allocated memory and releasing
   *  resources. This is called by the SCL when closing a channel, it should 
   *  not be called otherwise.
   * arguments:
   *  pChannel - channel
   * returns:
   *  TMWDEFS_TRUE if successful
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL dnplink_deleteChannel(
    TMWCHNL *pChannel);
  
  /* function: dnplink_rechecksum
   * purpose: Recalculate checksums for this message. This will be called by
   *  Secure Authentication code when a time sync with aggressive mode is 
   *  being sent. It will not be called otherwise.
   * arguments: 
   *  pPhysTxDescriptor pointer to transmit descriptor for this message.
   * returns
   *  void
   */
  void TMWDEFS_GLOBAL dnplink_rechecksum(
    TMWPHYS_TX_DESCRIPTOR *pPhysTxDescriptor);

  /* function: dnplink_linkStatus
   * purpose: Force Link layer to send a Link Reset request
   * arguments:
   *  pSession - pointer to session
   * returns
   *  TMWDEFS_TRUE if successful
   */
  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL dnplink_linkReset(
    TMWSESN *pSession);

  /* function: dnplink_linkStatus
   * purpose: Force Link layer to send a Test Link request
   * arguments:
   *  pSession - pointer to session
   * returns
   *  TMWDEFS_TRUE if successful
   */
  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL dnplink_testLinkState(
    TMWSESN *pSession);

  /* function: dnplink_linkStatus
   * purpose: Force Link layer to send a Link Status request
   * arguments:
   *  pSession - pointer to session
   * returns
   *  TMWDEFS_TRUE if successful
   */
  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL dnplink_linkStatus(
    TMWSESN *pSession);
  
  /* function: dnplink_closeAndOpen
   * purpose: Force Link layer to close and reopen the connection
   * arguments:
   *  pSession - pointer to session
   * returns
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL dnplink_closeAndOpen(
    TMWSESN *pSession);
   
  /* function: dnplink_getNeededBytes
   * purpose: return the number of bytes required to finish the
   *  current block
   * arguments:
   *  pCallbackParam - callback data, contains link layer context
   * returns
   *  number of characters to read
   */
  TMWDEFS_SCL_API TMWTYPES_USHORT TMWDEFS_CALLBACK dnplink_getNeededBytes(
    void *pCallbackParam);

  /* function: dnplink_parseBytes
   * purpose: parse incoming data
   * arguments:
   *  pCallbackParam - callback data, contains link layer context
   *  recvBuf - received characters
   *  numBytes - number of bytes
   *  firstByteTime - time that first byte was received. Could be zero
   *   if target layer does not fill this in. In that case it will be calculated
   *   in this function.
   * returns
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_CALLBACK dnplink_parseBytes(
    void *pCallbackParam,
    TMWTYPES_UCHAR *recvBuf,
    TMWTYPES_USHORT numBytes,
    TMWTYPES_MILLISECONDS firstByteTime);

#ifdef __cplusplus
}
#endif
#endif /* DNPLINK_DEFINED */
