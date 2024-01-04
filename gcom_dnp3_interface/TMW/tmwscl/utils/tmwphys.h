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

/* file: tmwphys.h
 * description: Define interface for generic physical layer.
 */
#ifndef TMWPHYS_DEFINED
#define TMWPHYS_DEFINED

#include "tmwscl/utils/tmwcnfg.h"
#include "tmwscl/utils/tmwdefs.h"
#include "tmwscl/utils/tmwtimer.h"


/* Data structure passed from the link layer to the physical layer 
 *  to transmit bytes 
 */
typedef struct TMWPhysTxDescriptor
{
  /* Buffer to transmit */
  TMWTYPES_UCHAR *pTxBuffer;
  TMWTYPES_UCHAR  UDPPort;
  TMWTYPES_USHORT numBytesToTx;

  /* Callbacks */
  void *pCallbackParam;
  TMWTYPES_CALLBACK_FUNC beforeTxCallback;
  TMWTYPES_CALLBACK_FUNC afterTxCallback;
  TMWTYPES_CALLBACK_FUNC failedTxCallback;
} TMWPHYS_TX_DESCRIPTOR;


typedef enum TMWPHYSD_CLOSE_REASON_ENUM
{
  TMWPHYSD_UNKNOWN_CLOSE_REASON,
  TMWPHYSD_LOCAL_CLOSED,
  TMWPHYSD_REMOTE_CLOSED
} TMWPHYSD_CLOSE_REASON;

/* Return value for TMWPHYS_CHECK_ADDR_FUNC */
typedef enum TMWPHYS_ADDRESS_MATCH_TYPE_ENUM
{
  /* The incoming message is for this channel */
  TMWPHYS_ADDRESS_MATCH_SUCCESS=0,
  /* The incoming message may be for this channel,
   * so far the bytes match this protocol,
   * but more bytes are needed to tell if the address matches.
   */
  TMWPHYS_ADDRESS_MATCH_MAYBE,
  /* The incoming message is not for this channel */
  TMWPHYS_ADDRESS_MATCH_FAILED
} TMWPHYS_ADDRESS_MATCH_TYPE;


/* State of physical layer */
typedef enum TMWPHYS_OPENSTATE_ENUM
{
  TMWPHYS_CLOSED,
  TMWPHYS_OPENING,
  TMWPHYS_OPEN
} TMWPHYS_OPENSTATE_TYPE;


/* Functions called by the physical layer to determine how many bytes to 
 *  read and parse bytes. These functions make up the physical layer's
 *  callback into the link layer for receiving messages.
 */
typedef TMWTYPES_USHORT (*TMWPHYS_NEEDED_CHARS_FUNC)(
  void *pCallbackParam);

typedef void (*TMWPHYS_PARSE_FUNC)(
  void *pCallbackParam, 
  TMWTYPES_UCHAR *buf, 
  TMWTYPES_USHORT numBytes,
  TMWTYPES_MILLISECONDS firstByteTime); 

typedef void (*TMWPHYS_USER_PARSE_FUNC)(
  void *pCallbackParam, 
  TMWTYPES_USHORT neededChars,
  TMWTYPES_USHORT *pNumBytes,
  TMWTYPES_UCHAR  *pBuf, 
  TMWTYPES_MILLISECONDS firstByteTime); 

typedef TMWPHYS_ADDRESS_MATCH_TYPE (*TMWPHYS_CHECK_ADDRESS_FUNC)(
  void *pCallbackParam, 
  TMWTYPES_UCHAR *buf, 
  TMWTYPES_USHORT numBytes,
  TMWTYPES_MILLISECONDS firstByteTime); 

typedef void (*TMWPHYS_CHANNEL_FUNC)(
  void *pCallbackParam,
  TMWTYPES_BOOL openOrClose,
  TMWDEFS_TARG_OC_REASON  reason);

/* Define configuration parameters supported by this physical layer 
 * implementation.
 */
typedef struct tmwphysConfig {

  /* Minimum time, in milliseconds, after receiving a character before we
   *  will attempt to transmit a character on this channel. This is generally
   *  useful when using a modem or some other communication device that
   *  requires a minimum time between receive and transmit.
   */
  TMWTYPES_USHORT firstCharWait;

  /* Receive buffer size. A single buffer of this size is allocated per channel  
   *  at startup and reused as long as this channel is active. The physical
   *  layer transmits directly from the buffer passed down from the link
   *  layer so no transmit buffer size is required.
   */
  TMWTYPES_USHORT rxBufferSize;
  
  /* Is channel currently configured to be active or not
   * active will tell target layer try to connect 
   * !active will be configured but will not try to connect
   */
  TMWTYPES_BOOL active;

  /* Add support for sniffer or protocol analyzer */
  TMWTYPES_BOOL monitorMode;
  
  /* Call user rx phys callback with received data even when "disconnected"
   * To support Modems using tmwtarg_transmit 
   */
  TMWTYPES_BOOL rcvModemData;

} TMWPHYS_CONFIG;

/* Define bit masks used to specify which configuration parameters
 *  should be modified in a call to tmwphys_modifyChannel. Parameters
 *  that do not have bitmasks can not currently be modified at runtime.
 */
#define TMWPHYS_CONFIG_FIRST_CHAR_WAIT      0x00000001UL

/* Internal context used by tmwphys implementation. This structure should
 *  never be read or modified outside the physical layer implementation.
 */
typedef struct TMWPhysContext {

  /* Low level IO parameters */
  void *pIOContext;

  /* Pointer to the channel */
  struct TMWChannelStruct *pChannel;

  /* Is channel currently set to be active or "configured but disconnected" */
  TMWTYPES_BOOL active;

  /* Configuration */
  TMWTYPES_USHORT firstCharWait;
  TMWTYPES_USHORT rxBufferSize;

  /* Physical level context */
  TMWPHYS_OPENSTATE_TYPE channelOpen;
  TMWTIMER firstCharWaitTimer;
  TMWTIMER transmitReadyTimer;
  TMWTYPES_UCHAR *pReceiveBuffer;
  const TMWPHYS_TX_DESCRIPTOR *pTxDescriptor;

  /* Current receive callbacks */
  void *pCallbackParam;
  TMWPHYS_PARSE_FUNC pParseFunc;
  TMWPHYS_CHANNEL_FUNC pChannelFunc;
  TMWPHYS_NEEDED_CHARS_FUNC pNeededCharsFunc;
  TMWPHYS_CHECK_ADDRESS_FUNC pCheckAddressFunc;

#if !TMWCNFG_USE_DYNAMIC_MEMORY || TMWCNFG_ALLOC_ONLY_AT_STARTUP 
  TMWTYPES_UCHAR buffer[TMWCNFG_MAX_RX_BUFFER_LENGTH];
#endif

  TMWTYPES_BOOL monitorMode;
  TMWTYPES_BOOL rcvModemData;
  
#if TMWCNFG_SUPPORT_RXCALLBACKS
  /* callback into user provided parse function.
   * For special test purposes only.
   * Customers will not need to implement this.
   */ 
  TMWPHYS_USER_PARSE_FUNC pUserParseFunc;
  void                   *pUserParseParam;
#endif

} TMWPHYS_CONTEXT;

/* Now define the actual physical layer interface that will be used
 *  by the link layer to call into the physical layer. Each physical
 *  layer implementation must provide an instance of this interface.
 */
typedef TMWTYPES_BOOL (*TMWPHYS_OPEN_CHANNEL_FUNC)(
  TMWPHYS_CONTEXT *pContext);

typedef TMWTYPES_BOOL (*TMWPHYS_CLOSE_CHANNEL_FUNC)(
  TMWPHYS_CONTEXT *pContext, 
  TMWDEFS_TARG_OC_REASON reason);

typedef void (*TMWPHYS_SET_CALLBACKS_FUNC)(
  TMWPHYS_CONTEXT *pContext, 
  TMWPHYS_CHANNEL_FUNC pChannelFunc,
  TMWPHYS_NEEDED_CHARS_FUNC pNeededCharsFunc, 
  TMWPHYS_PARSE_FUNC pParseFunc, 
  TMWPHYS_CHECK_ADDRESS_FUNC pCheckAddressFunc,
  void *pCallbackParam);

typedef TMWTYPES_BOOL (*TMWPHYS_TRANSMIT_FUNC)(
  TMWPHYS_CONTEXT *pContext, 
  const TMWPHYS_TX_DESCRIPTOR *pTxDescriptor);

typedef TMWTYPES_BOOL (*TMWPHYS_RECEIVE_FUNC)(
  TMWPHYS_CONTEXT *pContext, 
  TMWTYPES_MILLISECONDS maxTimeout);

typedef struct TMWPhysInterface {
  TMWPHYS_OPEN_CHANNEL_FUNC pPhysOpenChannel;
  TMWPHYS_CLOSE_CHANNEL_FUNC pPhysCloseChannel;
  TMWPHYS_SET_CALLBACKS_FUNC pPhysSetCallbacks;
  TMWPHYS_TRANSMIT_FUNC pPhysTransmit;
  TMWPHYS_RECEIVE_FUNC pPhysReceive;
} TMWPHYS_INTERFACE;

struct TMWTargConfigStruct;

#ifdef __cplusplus
extern "C" {
#endif

  /* function: tmwphys_initConfig
   * purpose: Initialize a tmwphys configuration data structure
   * arguments:
   *  pConfig - pointer to tmwphys configuration structure to be
   *   initialized.
   * returns:
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwphys_initConfig(
    TMWPHYS_CONFIG *pConfig);

  /* function: tmwphys_initChannel
   * purpose: initialize a new channel.
   * arguments:
   *  pConfig - pointer to tmwphys configuration information
   *  pIOConfig - IO configuration information which will be passed 
   *   to low level IO routines in tmwtarg.h.
   *  pTmwTargConfig - TMW specific IO configuration information 
   *   which will be passed to low level IO routines in tmwtarg.h. 
   * returns:
   *  A pointer to a context that is used to identify this channel
   *   for all future operations.
   */
  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL tmwphys_initChannel(
    struct TMWChannelStruct *pChannel,
    const TMWPHYS_CONFIG *pConfig, 
    const void *pIOConfig,
    struct TMWTargConfigStruct *pTmwTargConfig);

  /* function: tmwphys_modifyChannel
   * purpose: Modify an existing channel
   * arguments:
   *  pContext - context returned from tmwphys_initChannel
   *  pConfig - pointer to tmwphys configuration information
   *  configMask - mask which specifies which parameters in pConfig
   *   should be modified.
   *  pIOConfig - IO configuration information which will be passed 
   *   to low level IO routines in tmwtarg.h.
   * returns:
   *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
   */
  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL tmwphys_modifyChannel(
    struct TMWChannelStruct *pChannel,
    const TMWPHYS_CONFIG *pConfig,
    TMWTYPES_ULONG configMask);
      
  /* function: tmwphys_getChannelConfig
   * purpose: Get current configuration
   * arguments:
   *  pChannel - pointer to channel structure
   *  pConfig - pointer to tmwphys configuration information 
   * returns:
   *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
   */
  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL tmwphys_getChannelConfig(
    struct TMWChannelStruct *pChannel,
    TMWPHYS_CONFIG *pPhysConfig);

  /* function: tmwphys_setChannelConfig
   * purpose: Set current configuration
   * arguments:
   *  pChannel - pointer to channel structure
   *  pConfig - pointer to tmwphys configuration information 
   * returns:
   *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
   */
  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL tmwphys_setChannelConfig(
    struct TMWChannelStruct *pChannel,
    const TMWPHYS_CONFIG *pPhysConfig);

  /* function: tmwphys_modifyPhys
   * purpose: Modify an existing channels target layer configuration
   * arguments:
   *  pChannel - channel pointer returned by tmwphys_initChannel
   *  pIOConfig - IO configuration information which will be passed 
   *   to low level IO routines in tmwtarg.h. 
   *  pTmwTargConfig - TMW specific IO configuration information 
   *   which will be passed to low level IO routines in tmwtarg.h.
   * returns:
   *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
   */
  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL tmwphys_modifyPhys(
    struct TMWChannelStruct *pChannel,
    const void *pIOConfig,
    struct TMWTargConfigStruct *pTmwTargConfig);

  /* function: tmwphys_deleteChannel
   * purpose: delete this channel, freeing all allocated memory and releasing
   *  resources.
   * arguments:
   *  pChannel - context returned from tmwphys_initChannel
   * returns:
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwphys_deleteChannel(
    struct TMWChannelStruct *pChannel);

#ifdef __cplusplus
}
#endif
#endif /* TMWPHYS_DEFINED */
