/*****************************************************************************/
/* Triangle MicroWorks, Inc.                         Copyright (c) 2008-2020 */
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

/* file: linTCP.h
 * description: Definition of TCP/IP target routines for Linux
 */
#ifndef linTCP_DEFINED
#define linTCP_DEFINED

#if TMWTARG_SUPPORT_TCP
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#include <netinet/tcp.h>
#include <fcntl.h>
#include <signal.h>
#include <netinet/in.h>
#include <poll.h>
#include "tmwscl/utils/tmwtypes.h"
#include "tmwscl/utils/tmwdefs.h"
#include "tmwscl/utils/tmwtarg.h"

#if TMWTARG_SUPPORT_TLS
#include "lintls.h"
#endif

#define SOCKET_ERROR            -1
#define INVALID_SOCKET          -1

#define LINTCP_BACKLOG          5
#define LINTCP_RECV_FLAGS       0

 /* This indicates max number of received UDP Octets that can be buffered on a channel.
  * This must be >= LINTCP_MAX_UDP_FRAME */
#define LINTCP_UDP_BUFFER_SIZE  3000

 /* Largest received UDP Packet length supported.
  * This also should be <= LINTCP_UDP_BUFFER_SIZE
  */
#define LINTCP_MAX_UDP_FRAME    500

#define MAKE_SOCKET_NON_BLOCKING( s, retval ) \
    { \
        struct sigaction sa; \
        memset( &sa, 0, sizeof(sa )); \
        sa.sa_handler = SIG_IGN; \
        sigaction( SIGPIPE, &sa, (struct sigaction *) NULL ); \
	      retval = fcntl( s, F_SETFL, O_NONBLOCK ); \
    }

#define MAKE_SOCKET_REUSEADDR( s, ret ) \
    { \
        int cmd_arg = 1; \
        ret = setsockopt( s, SOL_SOCKET, SO_REUSEADDR,  \
                (char *) &cmd_arg, sizeof(cmd_arg) ); \
    }

#define MAKE_SOCKET_KEEPALIVE( s, ret ) \
    { \
        int cmd_arg = 1; \
        ret = setsockopt( s, SOL_SOCKET, SO_KEEPALIVE,  \
                (char *) &cmd_arg, sizeof(cmd_arg) ); \
    }

#define MAKE_SOCKET_NO_DELAY( s, ret ) \
    { \
        int cmd_arg = 1; \
        ret = setsockopt( s, SOL_TCP, TCP_NODELAY,  \
                (char *) &cmd_arg, sizeof(cmd_arg) ); \
    }

enum SocketIndex
{
  SOCKET_INDEX_DATA,
  SOCKET_INDEX_UDP,
  SOCKET_INDEX_MAX
};

typedef struct TcpIOChannel {
  TMWTARGTCP_CONFIG     chnlConfig;

  /* Number of milliseconds to wait between TCP connection attempts */
  TMWTYPES_MILLISECONDS connectRetry;

  int                   afInet;
  struct in6_addr       ipAddr;
  TMWTYPES_BOOL         newConnectionRcvd;
  TMWTYPES_BOOL         disconnectOldChnl;
  TMWTYPES_BOOL         clientConnected;
  TMWTYPES_BOOL         disconnectError;

  SOCKET                dataSocket;
  SOCKET                udpSocket;
  int                   pipeFd[2];
  struct pollfd         pollFds[SOCKET_INDEX_MAX];

  void                 *pTcpListener;
  TMWDEFS_RESOURCE_LOCK tcpChannelLock;

#if TMWTARG_SUPPORT_UDP
  TMWTYPES_USHORT       sourceUDPPort;
  struct in6_addr       validUDPAddress;

  struct sockaddr_in6   destUDPaddr;
  int                   destUDPaddrLen;

  struct sockaddr_in6   UDPOnlyBrdcastAddr;
  int                   UDPOnlyBrdcastAddrLen;

  TMWDEFS_RESOURCE_LOCK udpBufferLock;
  TMWTYPES_UCHAR        udpBuffer[LINTCP_UDP_BUFFER_SIZE];
  int                   udpReadIndex;
  int                   udpWriteIndex;
#endif

  /*TLS Support */
#if TMWTARG_SUPPORT_TLS
  TLS_IO_CHANNEL        tls;
#endif

  TMWTYPES_MILLISECONDS time_connect;
  char lastIPAddress[128];

} TCP_IO_CHANNEL;

#define MAX_LISTENING_CHANNELS 16
typedef struct TcpListener {
  SOCKET                     listenSocket;
  TMWTYPES_USHORT            listenPort;
  struct in6_addr            listenAddr;
  int                        numIoChannels;
#if TMWCNFG_SUPPORT_THREADS
  TMWTARG_THREAD_STATE       chanThreadState;
  TMW_ThreadId               chanThreadHandle;
#endif

  TMWTARG_IO_CHANNEL *ioChannelPtrs[MAX_LISTENING_CHANNELS];
  struct TcpListener *pNext;
  
} TCP_LISTENER;

/* function: linTCP_initChannel */
void * TMWDEFS_GLOBAL linTCP_initChannel(
  const void *pUserConfig,
  TMWTARG_CONFIG *pTmwConfig,
  TMWTARG_IO_CHANNEL *pTargIoChannel);

/* function: linTCP_deleteChannel */
void TMWDEFS_GLOBAL linTCP_deleteChannel(TMWTARG_IO_CHANNEL *pTargIoChannel);

/* function: linTCP_openChannel */
TMWTYPES_BOOL TMWDEFS_GLOBAL linTCP_openChannel(TMWTARG_IO_CHANNEL *pTargIoChannel);

/* function: linTCP_closeChannel */
void TMWDEFS_GLOBAL linTCP_closeChannel(TMWTARG_IO_CHANNEL *pTargIoChannel);

/* function: linTCP_getTransmitReady */
TMWTYPES_MILLISECONDS TMWDEFS_GLOBAL linTCP_getTransmitReady(
  TMWTARG_IO_CHANNEL *pTargIoChannel);

/* function: linTCP_receive */
TMWTYPES_USHORT TMWDEFS_GLOBAL linTCP_receive(
  TMWTARG_IO_CHANNEL *pTargIoChannel,
  TMWTYPES_UCHAR *pBuff,
  TMWTYPES_USHORT maxBytes,
  TMWTYPES_MILLISECONDS maxTimeout,
  TMWTYPES_BOOL *pInterCharTimeoutOccurred);

/* function: linTCP_transmit */
TMWTYPES_BOOL TMWDEFS_GLOBAL linTCP_transmit(
  TMWTARG_IO_CHANNEL *pTargIoChannel,
  TMWTYPES_UCHAR *pBuff,
  TMWTYPES_USHORT numBytes);

TMWTYPES_BOOL TMWDEFS_GLOBAL linTCP_transmitUDP(
  TMWTARG_IO_CHANNEL *pTargIoChannel,
  TMWTYPES_UCHAR UDPPort,
  TMWTYPES_UCHAR *pBuff,
  TMWTYPES_USHORT numBytes);

void TMWDEFS_GLOBAL linTCP_checkForInputFunction(TMWTARG_IO_CHANNEL *pTargIoChannel,
  TMWTYPES_MILLISECONDS timeout);

void TMWDEFS_GLOBAL linTCP_exit(void);

#endif /* #if TMWTARG_SUPPORT_TCP */

#endif /* linTCP_DEFINED */
