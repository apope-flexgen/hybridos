/*****************************************************************************/
/* Triangle MicroWorks, Inc.                         Copyright (c) 2008-2019 */
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

/* file: sampleTCP.h
 * description: Definition of Sample TCP/IP target routines
 */
#ifndef sampleTCP_DEFINED
#define sampleTCP_DEFINED

#if TMWTARG_SUPPORT_TCP
#include "tmwscl/utils/tmwtypes.h"
#include "tmwscl/utils/tmwdefs.h"
#include "tmwscl/utils/tmwtarg.h"

typedef enum TcpState
{
  TCP_STATE_INITIALIZED = 1,
  TCP_STATE_CLOSED,
  TCP_STATE_OPENED,
  TCP_STATE_CONNECTING,
  TCP_STATE_LISTENING,
  TCP_STATE_CONNECTED,
} TCP_STATE;


typedef struct TcpIOChannel {
  TMWTARGTCP_CONFIG     chnlConfig;

  TMWDEFS_RESOURCE_LOCK tcpChannelLock;

} TCP_IO_CHANNEL;

/* function: sampleTCP_initChannel */
void * TMWDEFS_GLOBAL sampleTCP_initChannel(
  const void *pUserConfig,
  TMWTARG_CONFIG *pTmwConfig,
  TMWTARG_IO_CHANNEL *pTargIoChannel);

/* function: sampleTCP_deleteChannel */
void TMWDEFS_GLOBAL sampleTCP_deleteChannel(TMWTARG_IO_CHANNEL *pTargIoChannel);

/* function: sampleTCP_openChannel */
TMWTYPES_BOOL TMWDEFS_GLOBAL sampleTCP_openChannel(TMWTARG_IO_CHANNEL *pTargIoChannel);

/* function: sampleTCP_closeChannel */
void TMWDEFS_GLOBAL sampleTCP_closeChannel(TMWTARG_IO_CHANNEL *pTargIoChannel);

/* function: sampleTCP_getTransmitReady */
TMWTYPES_MILLISECONDS TMWDEFS_GLOBAL sampleTCP_getTransmitReady(
  TMWTARG_IO_CHANNEL *pTargIoChannel);

/* function: sampleTCP_receive */
TMWTYPES_USHORT TMWDEFS_GLOBAL sampleTCP_receive(
  TMWTARG_IO_CHANNEL *pTargIoChannel,
  TMWTYPES_UCHAR *pBuff,
  TMWTYPES_USHORT maxBytes,
  TMWTYPES_MILLISECONDS maxTimeout,
  TMWTYPES_BOOL *pInterCharTimeoutOccurred);

/* function: sampleTCP_transmit */
TMWTYPES_BOOL TMWDEFS_GLOBAL sampleTCP_transmit(
  TMWTARG_IO_CHANNEL *pTargIoChannel,
  TMWTYPES_UCHAR *pBuff,
  TMWTYPES_USHORT numBytes);

/* function: sampleTCP_transmitUDP */
TMWTYPES_BOOL TMWDEFS_GLOBAL sampleTCP_transmitUDP(
  TMWTARG_IO_CHANNEL *pTargIoChannel,
  TMWTYPES_UCHAR UDPPort,
  TMWTYPES_UCHAR *pBuff,
  TMWTYPES_USHORT numBytes);

/* function: sampleTCP_checkForInputFunction */
void TMWDEFS_GLOBAL sampleTCP_checkForInputFunction(TMWTARG_IO_CHANNEL *pTargIoChannel,
  TMWTYPES_MILLISECONDS timeout);

/* function: sampleTCP_exit */
void TMWDEFS_GLOBAL sampleTCP_exit(void);

#endif /* #if TMWTARG_SUPPORT_TCP */

#endif /* sampleTCP_DEFINED */
