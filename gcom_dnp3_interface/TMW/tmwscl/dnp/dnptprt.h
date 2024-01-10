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

/* file: dnptprt.h
 * description: This file is intended for internal SCL use only.
 *   Implement the DNP3 transport layer.
 */
#ifndef DNPTPRT_DEFINED
#define DNPTPRT_DEFINED

#include "tmwscl/utils/tmwtprt.h"
#include "tmwscl/utils/tmwchnl.h"
#include "tmwscl/utils/tmwcnfg.h"
#include "tmwscl/dnp/dnpcnfg.h"

/* Define configuration parameters supported by this physical layer 
 * implementation.
 */
typedef struct dnptprtConfig {

  /* Currently there is not configuration specifically for the transport layer */
  TMWTYPES_UCHAR unused;

  /* Removed frameSize. It was previously overwritten by DNPCHNL txFrameSize  
   * To configure frame sizes set
   *  DNPLINK_CONFIG txFrameSize and rxFrameSize 
   */

  /* Removed fragmentSize. It was previously overwritten by DNPCHNL rxFragmentSize
   * To configure fragment sizes set 
   *   DNPCHNL_CONFIG rxFragmentSize and txFragmentSize
   */

} DNPTPRT_CONFIG;


/* Prototype for user callback function */
typedef void (*DNPTPRT_TX_CALLBACK_FUNC)(
  void *pCallbackParam,
  TMWSESN_TX_DATA *pTxData);

typedef struct DNPTprtContext {
  /* Generic transport layer context, must be first entry */
  TMWTPRT_CONTEXT tmw;

  /* Specify the maximum number of data bytes in a link layer frame. This
   * value includes the transport header byte. The maximum value for this
   * parameter is 250
   */
  TMWTYPES_USHORT txFrameSize;

#if !DNPCNFG_MULTI_SESSION_REQUESTS
  /* Transmit info */
  TMWTYPES_BOOL txInProgress;
  TMWTYPES_BOOL txFirstFrame;
  TMWTYPES_USHORT txFragmentOffset;
  TMWSESN_TX_DATA *pTxDescriptor;
  TMWSESN_TX_DATA linkTxDescriptor;

#if !TMWCNFG_USE_DYNAMIC_MEMORY || TMWCNFG_ALLOC_ONLY_AT_STARTUP 
  /* Note that this is slightly bigger than necessary.
   * It provides room for the link layer header and CRCs
   */
  TMWTYPES_UCHAR txFrameBuffer[DNPCNFG_MAX_TX_FRAME_LENGTH];
#endif

  /* Receive info */
  TMWSESN *pRxSession;
  TMWTYPES_BOOL rxWaitingForFirst;
  TMWTYPES_USHORT rxFragmentOffset;
  TMWSESN_RX_DATA rxFragment;
  TMWTYPES_UCHAR *pRxFragmentBuffer;

#if !TMWCNFG_USE_DYNAMIC_MEMORY || TMWCNFG_ALLOC_ONLY_AT_STARTUP 
  TMWTYPES_UCHAR rxFragmentBuffer[DNPCNFG_MAX_RX_FRAGMENT_LENGTH];
#endif
#endif

 /* User callback function to be called when fragment is given to transport layer
  * Most implementations will not need to provide this callback function. This
  * allows the fragment to be modified before it is transmitted for test purposes,
  * for example to modify the IIN bits or introduce errors.
  */
  DNPTPRT_TX_CALLBACK_FUNC pUserTxCallback;
  void *pCallbackParam;

} DNPTPRT_CONTEXT;

/* Define structure to hold DNP transport layer specific session
 * information.
 */
typedef struct {
  TMWTYPES_UCHAR rxSequenceNumber;
  TMWTYPES_UCHAR txSequenceNumber;

#if DNPCNFG_MULTI_SESSION_REQUESTS
  /* Transmit info */
  TMWTYPES_BOOL txInProgress;
  TMWTYPES_BOOL txFirstFrame;
  TMWTYPES_USHORT txFragmentOffset;
  TMWSESN_TX_DATA *pTxDescriptor;
  TMWSESN_TX_DATA linkTxDescriptor;

  /* Note that this is slightly bigger than necessary.
   * It provides room for the link layer header and CRCs
   */
  TMWTYPES_UCHAR txFrameBuffer[DNPCNFG_MAX_TX_FRAME_LENGTH];

  /* Receive info */
  TMWSESN *pRxSession;
  TMWTYPES_BOOL rxWaitingForFirst;
  TMWTYPES_USHORT rxFragmentOffset;
  TMWSESN_RX_DATA rxFragment;
  TMWTYPES_UCHAR rxFragmentBuffer[DNPCNFG_MAX_RX_FRAGMENT_LENGTH];
#endif

} DNPTPRT_SESSION_INFO;


#ifdef __cplusplus
extern "C" {
#endif

  /* function: dnptprt_initConfig
   * purpose: Initialize a dnptprt configuration data structure
   * arguments:
   *  pConfig - pointer to dnptprt configuration structure to be
   *   initialized.
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL dnptprt_initConfig(
    DNPTPRT_CONFIG *pConfig);

  /* Initialize a new channel. This method returns a pointer to a context
   * that must be passed into all of the other routines below.
   */
  DNPTPRT_CONTEXT * TMWDEFS_GLOBAL dnptprt_initChannel(
    TMWCHNL *pChannel,
    const DNPTPRT_CONFIG *pConfig);

  /* function: dnptprt_reInitChannel
   * This is called if the txFrameSize of rxFragmentSize is increased for an open channel
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL dnptprt_reInitChannel(
    TMWCHNL *pChannel,
    TMWTYPES_USHORT txFrameSize,
    TMWTYPES_USHORT rxFragmentSize);

  /* function: dnptprt_modifyChannel
   *  DEPRECATED FUNCTION, SHOULD USE dnpchnl_setChannelConfig()
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL dnptprt_modifyChannel(
    TMWCHNL *pChannel,
    const DNPTPRT_CONFIG *pConfig, 
    TMWTYPES_ULONG configMask);

  /* Delete this channel, freeing all allocated memory and releasing
   * resources.
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL dnptprt_deleteChannel(
    TMWCHNL *pChannel);

  TMWDEFS_SCL_API void TMWDEFS_GLOBAL dnptprt_parseFrame(
    void *parseParam,
    TMWSESN *pSession,
    TMWSESN_RX_DATA *pRxFrame);

  /* function: dnptprt_registerCallback  
   * purpose: Allow user to register a callback function to be called when
   *  a fragment is passed to the transport layer for transmission. Most
   *  users will not register such a function, but this could be used  
   *  for test purposes to modify a fragment before it is sent out to 
   *  modify the IIN bits on the slave or introduce errors.
   * arguments:
   *  pChannel
   *  pUserTxCallback
   * returns:
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL dnptprt_registerCallback(
    TMWCHNL *pChannel,
    DNPTPRT_TX_CALLBACK_FUNC pUserTxCallback,
    void                    *pCallbackParam);


#ifdef __cplusplus
}
#endif
#endif /* DNPTPRT_DEFINED */
