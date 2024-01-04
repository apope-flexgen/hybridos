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

/* file: WinTLSChannel.h
 * description: Implementation of Windows TLS interface using OpenSSL.
 *  Much of this was derived from TMWs 61850 TLS implementation.
 */
#ifndef WinTLSChannel_DEFINED
#define WinTLSChannel_DEFINED

#include "tmwscl/utils/tmwdefs.h"
#include "tmwscl/utils/tmwtypes.h"
#include "tmwscl/utils/tmwphys.h"
#include "tmwscl/utils/tmwtarg.h"
#include "WinIoTarg/WinTCPChannel.h" 
#include "WinIoTarg/WinIoTargTimer.h" 

#if TMWCNFG_USE_OPENSSL
#include "openssl/ssl.h"
#include <openssl/err.h>
#include <openssl/conf.h>
#include <openssl/engine.h>

#define WINTCP_TLS_BUFFER_SIZE     4096

typedef enum { 
               TMW_TLS_IDLE,
               TMW_TLS_READY,
               TMW_TLS_RENEGOTIATING,
               TMW_TLS_FAILED
}
TMW_TLS_State;

#define RFC_SOCKET_FAILED   -1
#define RFC_SOCKET_SUCCESS   0
#define RFC_SOCKET_BLOCKED   1

class WinTLSChannel : public WinTCPChannel
{
friend class WinTCPListener;

public:
  // Constructor
  WinTLSChannel() {};
  WinTLSChannel(const WINTCP_CONFIG &config, TMWTARG_CONFIG *pTmwTargConfig);
  
  // Desctructor
  ~WinTLSChannel(void);

  static void     TLSTerminateSslLibrary(void);

  // This should be called to indicate that a PDU has been received.
  // This allows renegotiation to occur after a configured number of PDUs.
  void TLSReceivedPdu();
 
private:
  
  bool     TLSLoadSSLLibrary(void);
  
  SSL_CTX *TLSInitSSLCtx();

  void     TLSDestroySSLCtx();

  void     TLSLoadDHParams(SSL_CTX *pCtx, char *file);

  bool     TLSLoadPrivateKeyFile(SSL_CTX      *pSSLCtx, 
                                 char         *pKeyFileName, 
                                 char         *pKeyPassPhrase, 
                                 char         *pCertFileName, 
                                 TMWTYPES_BOOL isRsaKey);

  bool     CheckCert(SSL *ssl);

  virtual bool TLSOpen();
  virtual void TLSClose();

  virtual bool TLSConnect(SOCKET tempCommSocket);
  virtual bool TLSListen(SOCKET acceptSocket);

  void Renegotiate();

  // Transmit numBytes bytes on this channel
  virtual int TLSTransmit(
    const TMWTYPES_UCHAR *buf, 
    TMWTYPES_USHORT numBytes);

  // Receive bytes from socket
  virtual int TLSLowReceive(
    TMWTYPES_UCHAR *buff, 
    TMWTYPES_ULONG maxBytes);

  AutoCriticalSectionWinTarg m_writeLock;
  AutoCriticalSectionWinTarg &getWriteLock()
  {
    return m_writeLock;
  }

  // Has TLSClose been called 
  bool m_bTLSClose;

  // Should TLS writer thread run
  bool m_bTLSWriterThreadRun;

  // Is TLS writer thread currently running
  bool m_TLSWriterThreadIsRunning;
 
  HANDLE m_TLSWriterThreadHandle;

  // TLS Write buffer to allow for delayed writes due to renegotiation
  TMWTYPES_UCHAR m_TLSWriteBuffer[WINTCP_TLS_BUFFER_SIZE];
  int            m_TLSBufferWriteIndex;
  int            m_TLSBufferReadIndex;

  bool StartTLSWriterThread(void);
  void StopTLSWriterThread();

  // TLS Writer thread entry point
  static unsigned int __stdcall TLSWriterThread(void *pParam);

  int StoreInTLSWriteBuffer(const TMWTYPES_UCHAR *pBuf, TMWTYPES_USHORT length);

  void TransmitFromTLSWriteBuffer();

  int TLSWrite(TMWTYPES_UCHAR *pBuf, int numCharsToSend);
  
  int TLSRead(SSL *ssl, SOCKET commSocket, 
                TMWTYPES_UCHAR *pBuff, int maxNumChars);

  int  TLSCheckStatus(int status);
  int  TLSCheckRenegotiationStatus( );
  int  TLSDoHandShake( );
  int  TLSRenegotiate( );
  void TLSCheckForRenegotiation();
  static void RenegotiationTimeout(void *pObject);

  SSL_CTX           *m_sslCtx;
  SSL               *m_pSSL;                              /* OpenSSL 'connection' */ 
  int                m_moreSSLData;                       /* not all data was read from socket */
  TMW_TLS_State      m_TLSState;                          /* TLS state machine */
  int                m_TLSRenegotiationCounter;           /* Counts TPDUs up to nTlsRenegotiationCount */
  bool               m_TLSRenegotiationRequested;         /* True if Server_Hello is pending */

  bool              m_TLS_RenegotiationTimeout;
  WinIoTargTimer    m_renegotiationTimer;

public: // Interface
  static WinTLSChannel *Create(
    const void *pConfig, 
    TMWTARG_CONFIG *pTmwTargConfig);

};
#endif

#endif // WinTLSChannel_DEFINED 
