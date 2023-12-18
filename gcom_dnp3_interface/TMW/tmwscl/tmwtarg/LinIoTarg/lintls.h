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

/* file: lintls.h
 * description: Definition of TLS target routines
 */
#ifndef LINTLS_H
#define LINTLS_H

#include "tmwscl/utils/tmwcnfg.h"
#include "tmwscl/utils/tmwtypes.h"
#include "tmwscl/utils/tmwdefs.h"
#include "tmwscl/utils/tmwtarg.h"

#if TMWTARG_SUPPORT_TLS
#if TMWCNFG_USE_OPENSSL
#include "openssl/ssl.h"
#endif

#define LINTLS_BUFFER_SIZE  4096

typedef enum { 
               TMW_TLS_IDLE,
               TMW_TLS_READY,
               TMW_TLS_RENEGOTIATING,
               TMW_TLS_FAILED
}
TMW_TLS_State;

typedef struct TlsIOChannel {
  SSL_CTX              *pSslCtx;
  SSL                  *pSSL;

  /* TLS Write buffer to allow for delayed writes due to renegotiation */
  TMWDEFS_RESOURCE_LOCK bufferLock;
  TMWTYPES_UCHAR        writeBuffer[LINTLS_BUFFER_SIZE];
  int                   bufferWriteIndex;
  int                   bufferReadIndex;
  TMWTYPES_BOOL         renegotiationRequested;
  TMWTYPES_MILLISECONDS renegotiationTimeout;
  int                   renegotiationCounter;
  TMW_TLS_State         state;
} TLS_IO_CHANNEL;

/* Channel Operation Function Delcarations */
struct TcpIOChannel;
typedef struct TcpIOChannel TCP_IO_CHANNEL;

/* function: tmwtls_open */
TMWTYPES_BOOL lintls_initChannel(TCP_IO_CHANNEL *pTcpChannel);
TMWTYPES_BOOL lintls_open(TCP_IO_CHANNEL *pTcpChannel);
void lintls_close(TCP_IO_CHANNEL *pTcpChannel);
void lintls_delete(TCP_IO_CHANNEL *pTcpChannel);
void lintls_terminateSslLibrary(void);

TMWTYPES_BOOL lintls_listen(TCP_IO_CHANNEL *pTcpChannel, SOCKET acceptSocket);
TMWTYPES_BOOL lintls_connect(TCP_IO_CHANNEL *pTcpChannel, SOCKET tempCommSocket);

int lintls_transmit(TCP_IO_CHANNEL *pTcpChannel, const TMWTYPES_UCHAR *pBufferToSend, TMWTYPES_USHORT numCharsToSend);
int lintls_read(TCP_IO_CHANNEL *pTcpChannel, TMWTYPES_UCHAR *pBuff, int maxNumChars);

TMWTYPES_BOOL lintls_checkChannel(TCP_IO_CHANNEL *pTcpChannel);
#endif
#endif /* LINTLS_H */
