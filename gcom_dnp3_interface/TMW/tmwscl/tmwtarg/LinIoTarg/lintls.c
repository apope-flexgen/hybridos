/*****************************************************************************/
/* Triangle MicroWorks, Inc.                         Copyright (c) 1997-2019 */
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

/* file: lintls.c
 * description: Implementation of TLS interface using OpenSSL.
 */
#include "tmwscl/utils/tmwcnfg.h"
#include "tmwtargcnfg.h"
#include "lintcp.h"

#if TMWTARG_SUPPORT_TLS && TMWTARG_SUPPORT_TCP
#if TMWCNFG_USE_OPENSSL
#include <openssl/opensslv.h>
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/conf.h>
#include <openssl/engine.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/x509_vfy.h>

#include "tmwscl/utils/tmwtarg.h"
#include "lintls.h"
#include "tmwtargio.h"
#include "liniodiag.h"


#define RFC_SOCKET_FAILED   -1
#define RFC_SOCKET_SUCCESS   0
#define RFC_SOCKET_BLOCKED   1

#define TLSCCHANNEL_DELAY 50

void TransmitFromTLSWriteBuffer(TCP_IO_CHANNEL *pChannel);
int TLSWrite(TCP_IO_CHANNEL *pChannel, TMWTYPES_UCHAR *pBuf, int numCharsToSend);
int TLSCheckRenegotiationStatus(TCP_IO_CHANNEL *pChannel);
void TLSCheckForRenegotiation(TCP_IO_CHANNEL *pChannel);
int TLSCheckStatus(TCP_IO_CHANNEL *pChannel, int status);
void TLSReceivedPdu(TCP_IO_CHANNEL *pChannel);
int TLSDoHandShake(TCP_IO_CHANNEL *pChannel);

TMWTYPES_BOOL OpenSSLLoaded = TMWDEFS_FALSE;

/* called by OpenSSL to get password for private key files */
static int password_cb(char *buf, int num, int rwflag, void *userdata)
{
  TMWTARG_UNUSED_PARAM(rwflag);
  char *pPassword = (char *)userdata;
  if (num < (int)strlen(pPassword) + 1)
    return(0);

  strcpy(buf, pPassword);
  return((int)strlen(pPassword));
}

static TMWTYPES_BOOL TLSLoadSslLibrary(void) {

  if (!OpenSSLLoaded)
  {
#if defined(OPENSSL_VERSION_NUMBER) && (OPENSSL_VERSION_NUMBER < 0x1010000fL)
    int initStatus;
    initStatus = SSL_library_init(); /* load encryption & hash algorithms for SSL */

    CRYPTO_malloc_init();
    SSL_load_error_strings(); /* load the error strings for good error reporting */
    OpenSSL_add_all_algorithms();
    OpenSSL_add_all_ciphers();
    OpenSSL_add_all_digests();
    LINIODIAG_MSG("TLS, Initialized 1.0.x SSL Library - SSL_library_init returned %d", initStatus);
#else
    OPENSSL_init_ssl(0, NULL);
    LINIODIAG_MSG("TLS, Initialized SSL Library");
#endif
    OpenSSLLoaded = TMWDEFS_TRUE;
  }
  return TMWDEFS_TRUE;
}

/* Terminate OpenSSL on exiting entire protocol stack application. */
void lintls_terminateSslLibrary(void)
{
  if (OpenSSLLoaded)
  {
    OpenSSLLoaded = TMWDEFS_FALSE;
#if defined(OPENSSL_VERSION_NUMBER) && (OPENSSL_VERSION_NUMBER < 0x1010000fL)
    CRYPTO_mem_leaks_fp(stderr);
    COMP_zlib_cleanup();
    CONF_modules_unload(1);
    OBJ_cleanup();
    EVP_cleanup();
    CRYPTO_cleanup_all_ex_data();
    ERR_remove_thread_state(NULL);
    ERR_remove_state(0);
    ENGINE_cleanup();
    ERR_free_strings();
#else
    FIPS_mode_set(0);
    ENGINE_cleanup();
    CONF_modules_unload(1);
    CRYPTO_cleanup_all_ex_data();
    ERR_free_strings();
#endif
  }
}

/* Load Diffie Hellman parameters */
static void TLSLoadDHParams(TCP_IO_CHANNEL *pTcpChannel, SSL_CTX *pCtx)
{
  DH *ret = 0;
  BIO *bio;

  if ((bio = BIO_new_file(pTcpChannel->chnlConfig.dhFileName, "r")) == NULL)
  {
    LINIODIAG_ERRORMSG("TLS(%s), Couldn't open DH file", pTcpChannel->chnlConfig.chnlName);
  }
  else
  {
    LINIODIAG_MSG("TLS(%s), Opened DH file", pTcpChannel->chnlConfig.chnlName);

    ret = PEM_read_bio_DHparams(bio, NULL, NULL, NULL);

    BIO_free(bio);

    if (SSL_CTX_set_tmp_dh(pCtx, ret) < 0)
      LINIODIAG_ERRORMSG("TLS(%s), Couldn't set DH parameters", pTcpChannel->chnlConfig.chnlName);
    else
      LINIODIAG_MSG("TLS(%s), Set DH parameters", pTcpChannel->chnlConfig.chnlName);
  }
}

/* Load Private key file */
static TMWTYPES_BOOL TLSLoadPrivateKeyFile(TCP_IO_CHANNEL *pTcpChannel,
                                           SSL_CTX        *pSSLCtx,
                                           char           *pKeyFileName,
                                           char           *pKeyPassPhrase,
                                           char           *pCertFileName,
                                           TMWTYPES_BOOL  isRsaKey)
{
  if ((pKeyFileName == NULL) || (pKeyFileName[0] == 0))
  {
    return(TMWDEFS_FALSE);
  }

  if ((pCertFileName == NULL) || (pCertFileName[0] == 0))
    pCertFileName = pKeyFileName;     /* Try reading both from same file */

  if (!SSL_CTX_use_certificate_file(pSSLCtx, pCertFileName, SSL_FILETYPE_PEM))
  {
    LINIODIAG_ERRORMSG("TLS(%s), Can't read certificate from file", pTcpChannel->chnlConfig.chnlName);
    return(TMWDEFS_FALSE);
  }

  SSL_CTX_set_default_passwd_cb_userdata(pSSLCtx, (void *)pKeyPassPhrase);
  if (isRsaKey)
  {
    if (!SSL_CTX_use_RSAPrivateKey_file(pSSLCtx, pKeyFileName, SSL_FILETYPE_PEM))
    {
      LINIODIAG_ERRORMSG("TLS(%s), Can't read private key from file %s", pTcpChannel->chnlConfig.chnlName, pKeyFileName);
      SSL_CTX_set_default_passwd_cb_userdata(pSSLCtx, (void *)NULL);
      return(TMWDEFS_FALSE);
    }
  }
  else
  {
    if (!SSL_CTX_use_PrivateKey_file(pSSLCtx, pKeyFileName, SSL_FILETYPE_PEM))
    {
      LINIODIAG_ERRORMSG("TLS(%s), Can't read private key file %s", pTcpChannel->chnlConfig.chnlName, pKeyFileName);
      SSL_CTX_set_default_passwd_cb_userdata(pSSLCtx, (void *)NULL);
      return(TMWDEFS_FALSE);
    }
  }

  /* Clear passphrase since it has already been used */
  SSL_CTX_set_default_passwd_cb_userdata(pSSLCtx, (void *)NULL);

  return(TMWDEFS_TRUE);
}

/* Initialize SSL context. */
static char *TMW_ciphers = "TLSv1:TLSv1.1:TLSv1.2:SSLv3:!SSLv2:!aNULL:!eNULL:!CAMELLIA:!EXPORT40:!EXPORT56:@STRENGTH";
SSL_CTX *TLSInitSSLCtx(TCP_IO_CHANNEL *pTcpChannel)
{
  TMWTYPES_CHAR *pCaFileName = TMWDEFS_NULL;
  TMWTYPES_CHAR *pCaPathName = TMWDEFS_NULL;
  TMWTYPES_BOOL bFoundValidKey = TMWDEFS_FALSE;

  /* Create our context*/
  /* This will allow TLS 1.0 1.1 and 1.2 */
#if defined(OPENSSL_VERSION_NUMBER) && (OPENSSL_VERSION_NUMBER < 0x1010000fL)
  SSL_CTX *pSslCtx = SSL_CTX_new(SSLv23_method());
#else
  SSL_CTX *pSslCtx = SSL_CTX_new(TLS_method());
#endif

  if (pSslCtx == TMWDEFS_NULL)
  {
    LINIODIAG_ERRORMSG("TLS(%s), Couldn't create a new context", pTcpChannel->chnlConfig.chnlName);
    return TMWDEFS_NULL;
  }

  LINIODIAG_MSG("TLS(%s), Created context", pTcpChannel->chnlConfig.chnlName);
  
  SSL_CTX_set_options(pSslCtx, SSL_OP_CIPHER_SERVER_PREFERENCE 
                             | SSL_OP_NO_SESSION_RESUMPTION_ON_RENEGOTIATION
                          /* | SSL_OP_ALLOW_UNSAFE_LEGACY_RENEGOTIATION */
                      );

  SSL_CTX_set_default_passwd_cb_userdata(pSslCtx, NULL);
  SSL_CTX_set_default_passwd_cb(pSslCtx, password_cb);

  if (TLSLoadPrivateKeyFile(pTcpChannel, pSslCtx, pTcpChannel->chnlConfig.tlsDsaPrivateKeyFile,
    pTcpChannel->chnlConfig.tlsDsaPrivateKeyPassPhrase, pTcpChannel->chnlConfig.tlsDsaCertificateId, TMWDEFS_FALSE /* bIsRsaKey */))
  {
    bFoundValidKey = TMWDEFS_TRUE;
  }

  if (TLSLoadPrivateKeyFile(pTcpChannel, pSslCtx, pTcpChannel->chnlConfig.tlsRsaPrivateKeyFile,
    pTcpChannel->chnlConfig.tlsRsaPrivateKeyPassPhrase, pTcpChannel->chnlConfig.tlsRsaCertificateId, TMWDEFS_TRUE /* bIsRsaKey */))
  {
    bFoundValidKey = TMWDEFS_TRUE;
  }

  if (!bFoundValidKey)
  {
    LINIODIAG_ERRORMSG("TLS(%s), Can't read DSA or RSA key files", pTcpChannel->chnlConfig.chnlName);
    SSL_CTX_free(pSslCtx);
    return TMWDEFS_NULL;
  }

  if (pTcpChannel->chnlConfig.caFileName[0] != 0)
    pCaFileName = pTcpChannel->chnlConfig.caFileName;

  if (pTcpChannel->chnlConfig.caPathName[0] != 0)
    pCaPathName = pTcpChannel->chnlConfig.caPathName;

  if (!(SSL_CTX_load_verify_locations(pSslCtx, pCaFileName, pCaPathName)))
  {
    LINIODIAG_ERRORMSG("TLS(%s), Can't read CA list %s", pTcpChannel->chnlConfig.chnlName, pTcpChannel->chnlConfig.caFileName);
  }
  else
  {
    LINIODIAG_MSG("TLS(%s), Read CA list %s", pTcpChannel->chnlConfig.chnlName, pTcpChannel->chnlConfig.caFileName);

    if (pCaFileName != TMWDEFS_NULL)
      SSL_CTX_set_client_CA_list(pSslCtx, SSL_load_client_CA_file(pCaFileName));

    if (pTcpChannel->chnlConfig.dhFileName[0] != 0)
      TLSLoadDHParams(pTcpChannel, pSslCtx);

    SSL_CTX_set_verify(pSslCtx, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, NULL /* TMW_verify_callback */);
    SSL_CTX_set_verify_depth(pSslCtx, pTcpChannel->chnlConfig.nCaVerifyDepth);

    if (!SSL_CTX_set_cipher_list(pSslCtx, TMW_ciphers))
    {
      LINIODIAG_ERRORMSG("TLS(%s), Can't set ciphers", pTcpChannel->chnlConfig.chnlName);
    }

    if (strlen(pTcpChannel->chnlConfig.caCrlFileName) > 0) {
      char * pCaCrlFileName;
      X509_STORE *pCaStore = SSL_CTX_get_cert_store(pSslCtx);
      X509_LOOKUP* lu = X509_STORE_add_lookup(pCaStore, X509_LOOKUP_file());
      pCaCrlFileName = pTcpChannel->chnlConfig.caCrlFileName;
      if (0 == X509_load_crl_file(lu, pCaCrlFileName, X509_FILETYPE_PEM))
      {
        LINIODIAG_ERRORMSG("TLS(%s), Can't load crl file %s", pTcpChannel->chnlConfig.chnlName, pCaCrlFileName);
      }
      else
      {
        X509_STORE_set_flags(pCaStore, X509_V_FLAG_CRL_CHECK | X509_V_FLAG_CRL_CHECK_ALL);
      }
    }
    else
    {
      LINIODIAG_ERRORMSG("TLS(%s), No crl file was specified", pTcpChannel->chnlConfig.chnlName);
    }
  }
  return pSslCtx;
}

/* Check Certificate */
TMWTYPES_BOOL CheckCert(TCP_IO_CHANNEL *pTcpChannel, SSL *ssl)
{
  X509 *pPeerCert;
  TMWTYPES_BOOL status;
  TMWTYPES_LONG retValue;
  char buf[512];

  status = TMWDEFS_FALSE;

  retValue = SSL_get_verify_result(ssl);

  pPeerCert = SSL_get_peer_certificate(ssl);
  if (pPeerCert != NULL)
  {
    LINIODIAG_MSG("TLS(%s), Certificate", pTcpChannel->chnlConfig.chnlName);
    X509_NAME_oneline(X509_get_subject_name(pPeerCert), buf, sizeof buf);
    LINIODIAG_MSG("TLS(%s), Subject=%s", pTcpChannel->chnlConfig.chnlName, buf);
    X509_NAME_oneline(X509_get_issuer_name(pPeerCert), buf, sizeof buf);
    LINIODIAG_MSG("TLS(%s), Issuer=%s", pTcpChannel->chnlConfig.chnlName, buf);
  }

  if (retValue == X509_V_OK)
  {
    if (pTcpChannel->chnlConfig.tlsCommonName[0] != 0)
    {
      char commonName[TMWTARG_CRYPTO_TLS_NAME_LEN];
      if (pPeerCert != TMWDEFS_NULL)
      {
        if (X509_NAME_get_text_by_NID(X509_get_subject_name(pPeerCert), NID_commonName, commonName, sizeof(commonName)))
        {
          X509_free(pPeerCert);
          if (strcmp(commonName, pTcpChannel->chnlConfig.tlsCommonName) != 0)
          {
            LINIODIAG_ERRORMSG("TLS(%s), Common name does not match.\nExpecting %s  Received %s",
              pTcpChannel->chnlConfig.chnlName, pTcpChannel->chnlConfig.tlsCommonName, commonName);
          }
          else
          {
            status = TMWDEFS_TRUE;
          }
        }
        else
        {
          LINIODIAG_ERRORMSG("TLS(%s), Can't get common name", pTcpChannel->chnlConfig.chnlName);
          X509_free(pPeerCert);
        }
      }
    }
    else
    {
      status = TMWDEFS_TRUE;
    }
  }
  else
  {
    LINIODIAG_ERRORMSG("TLS(%s), Certificate doesn't verify, verify result %d %s", pTcpChannel->chnlConfig.chnlName, retValue, X509_verify_cert_error_string(retValue));
  }

  return status;
}

/**********************************************************************************\
  Function :		StoreInTLSWriteBuffer
  Description : Store Bytes to be transmitted in circular buffer.
  Return :
  Parameters :
      void	-
  Note : [none]
\**********************************************************************************/
int StoreInTLSWriteBuffer(TCP_IO_CHANNEL *pChannel, const TMWTYPES_UCHAR *pBuf, TMWTYPES_USHORT length)
{
  if (length > 0)
  {
    TMWTARG_LOCK_SECTION(&pChannel->tls.bufferLock);
    int roomLeft = LINTLS_BUFFER_SIZE - pChannel->tls.bufferWriteIndex;

    /* Is there enough room at end of buffer? */
    if (roomLeft >= length)
    {
      memcpy(&pChannel->tls.writeBuffer[pChannel->tls.bufferWriteIndex], pBuf, length);
      pChannel->tls.bufferWriteIndex += length;
      if (pChannel->tls.bufferWriteIndex == LINTLS_BUFFER_SIZE)
        pChannel->tls.bufferWriteIndex = 0;
    }
    else
    {
      int remainingLength;
      memcpy(&pChannel->tls.writeBuffer[pChannel->tls.bufferWriteIndex], pBuf, roomLeft);
      remainingLength = length - roomLeft;
      memcpy(&pChannel->tls.writeBuffer[0], &pBuf[roomLeft], remainingLength);
      pChannel->tls.bufferWriteIndex = remainingLength;
    }
    TMWTARG_UNLOCK_SECTION(&pChannel->tls.bufferLock);
  }
  return length;
}

/**********************************************************************************\
  Function :		TransmitFromTLSWriteBuffer
  Description : [none]
  Return :			[none]
  Parameters :

  Note : [none]
\**********************************************************************************/
void TransmitFromTLSWriteBuffer(TCP_IO_CHANNEL *pChannel)
{
  int status;
  TMWTYPES_LONG bytesToTransmit = 0;
  TMWTYPES_UCHAR *writePtr = TMWDEFS_NULL;

  TMWTARG_LOCK_SECTION(&pChannel->tls.bufferLock);

  /* See if there are any bytes in the circular buffer to transmit */
  if (pChannel->tls.bufferReadIndex == pChannel->tls.bufferWriteIndex)
  {
    TMWTARG_UNLOCK_SECTION(&pChannel->tls.bufferLock);
    return;
  }

  writePtr = &pChannel->tls.writeBuffer[pChannel->tls.bufferReadIndex];

  if (pChannel->tls.bufferReadIndex < pChannel->tls.bufferWriteIndex)
  {
    bytesToTransmit = pChannel->tls.bufferWriteIndex - pChannel->tls.bufferReadIndex;
    status = TLSWrite(pChannel, writePtr, bytesToTransmit);
    if (status == bytesToTransmit)
    {
      pChannel->tls.bufferReadIndex += bytesToTransmit;
    }
    else /* either failure, blocked, or some bytes transmitted */
    {
      if (status > 0)
      {
        pChannel->tls.bufferReadIndex += status;
      }
    }
  }
  else
  {
    bytesToTransmit = LINTLS_BUFFER_SIZE - pChannel->tls.bufferReadIndex;
    status = TLSWrite(pChannel, writePtr, bytesToTransmit);
    if (status == bytesToTransmit)
    {
      pChannel->tls.bufferReadIndex = 0;
    }
    else /* either failure, blocked, or some bytes transmitted */
    {
      if (status > 0)
      {
        pChannel->tls.bufferReadIndex += status;
      }
    }
  }

  /* wrap back to beginning of buffer */
  if (pChannel->tls.bufferReadIndex == LINTLS_BUFFER_SIZE)
    pChannel->tls.bufferReadIndex = 0;

  TMWTARG_UNLOCK_SECTION(&pChannel->tls.bufferLock);
}


int TLSWrite(TCP_IO_CHANNEL *pTcpChannel, TMWTYPES_UCHAR *pBuf, int numCharsToSend)
{
  switch (TLSCheckRenegotiationStatus(pTcpChannel))
  {
  case RFC_SOCKET_SUCCESS:
    break;
  case RFC_SOCKET_BLOCKED:
    return(0);
  case RFC_SOCKET_FAILED:
  default:
    return(-1);
  }

  if (pTcpChannel->tls.renegotiationRequested)
  {
    /* We block SSL_writes once renegotiation has been requested */
    LINIODIAG_MSG("TLS(%s), TLSWrite renegotiate requested", pTcpChannel->chnlConfig.chnlName);
    return(0);
  }

  int temp = SSL_pending(pTcpChannel->tls.pSSL);
  if (temp)
  {
    LINIODIAG_MSG("TLS(%s), SSL_write blocked by ssl pending", pTcpChannel->chnlConfig.chnlName);
    return 0;
  }

  /* Try to write  */
  int r = SSL_write(pTcpChannel->tls.pSSL, pBuf, numCharsToSend);

  switch (TLSCheckStatus(pTcpChannel, r))
  {
  case RFC_SOCKET_SUCCESS:
    break;
  case RFC_SOCKET_BLOCKED:
    LINIODIAG_MSG("TLS(%s), SSL_write blocked", pTcpChannel->chnlConfig.chnlName);
    r = 0;
    break;
  case RFC_SOCKET_FAILED:
  default:
    LINIODIAG_ERRORMSG("TLS(%s), SSL_write failure", pTcpChannel->chnlConfig.chnlName);
    r = -1;
    break;
  }

  return r;
}

int lintls_read(TCP_IO_CHANNEL *pTcpChannel, TMWTYPES_UCHAR *pBuff, int maxNumChars)
{
  if (pTcpChannel->tls.pSSL == TMWDEFS_NULL)
  {
    LINIODIAG_ERRORMSG("TLS(%s), read, pSSL == NULLL", pTcpChannel->chnlConfig.chnlName);
    return TMWDEFS_FALSE;
  }

  switch (TLSCheckRenegotiationStatus(pTcpChannel))
  {
  case RFC_SOCKET_SUCCESS:
    break;
  case RFC_SOCKET_BLOCKED:
    LINIODIAG_MSG("TLS(%s), SSL lintls_read socket blocked", pTcpChannel->chnlConfig.chnlName);
    return(0);
  case RFC_SOCKET_FAILED:
  default:
    LINIODIAG_ERRORMSG("TLS(%s), SSL lintls_read socket failed", pTcpChannel->chnlConfig.chnlName);
    return(-1);
  }

  int r = SSL_read(pTcpChannel->tls.pSSL, pBuff, maxNumChars);

  switch (TLSCheckStatus(pTcpChannel, r))
  {
  case RFC_SOCKET_SUCCESS:
    break;
  case RFC_SOCKET_BLOCKED:
    // LINIODIAG_MSG("TLS(%s), lintls_read() blocked", pTcpChannel->chnlConfig.chnlName);
    return(0);
  case RFC_SOCKET_FAILED:
  default:
    LINIODIAG_ERRORMSG("TLS(%s), lintls_read() failed", pTcpChannel->chnlConfig.chnlName);
    return(-1);
  }

  // LINIODIAG_MSG("TLS(%s), Receive, data was received", pTcpChannel->chnlConfig.chnlName);


  /* This could be called from higher up the protocol stacks if you wanted to count application layer PDUs
   * This would require each protocol that used TLS to call this function. For now, just count here.
   */
  TLSReceivedPdu(pTcpChannel);

  TLSCheckForRenegotiation(pTcpChannel);

  return r;
}

int printOpenSSLErrors(const char * str, size_t len, void *ui)
{
  TMWTARG_UNUSED_PARAM(len);
  TMWTARG_UNUSED_PARAM(ui);

  LINIODIAG_ERRORMSG("openSSL error, %s\n", str);

  return 0;
}

int TLSCheckStatus(TCP_IO_CHANNEL *pTcpChannel, int status)
{
  if (status <= 0)
  {
    int last_error = SSL_get_error((SSL *)pTcpChannel->tls.pSSL, status);
    switch (last_error)
    {
    case SSL_ERROR_NONE:
      /* Should only happen if status > 0 */
      LINIODIAG_MSG("TLS(%s), CheckStatus: status = (none)", pTcpChannel->chnlConfig.chnlName);
      /* Returns SUCCESS, below */
      break;

    case SSL_ERROR_SSL:
      LINIODIAG_ERRORMSG("TLS(%s), CheckStatus: (Error queue)", pTcpChannel->chnlConfig.chnlName);
      /* SSL library failure */
      pTcpChannel->tls.state = TMW_TLS_FAILED;
      return(RFC_SOCKET_FAILED);

    case SSL_ERROR_WANT_READ:
      // LINIODIAG_MSG("TLS(%s), CheckStatus: (want read)", pTcpChannel->chnlConfig.chnlName);
      return(RFC_SOCKET_BLOCKED);

    case SSL_ERROR_WANT_WRITE:
      LINIODIAG_MSG("TLS(%s), CheckStatus: (want write)", pTcpChannel->chnlConfig.chnlName);
      return(RFC_SOCKET_BLOCKED);

    case SSL_ERROR_WANT_CONNECT:
      LINIODIAG_MSG("TLS(%s), CheckStatus: (want connect)", pTcpChannel->chnlConfig.chnlName);
      return(RFC_SOCKET_BLOCKED);

#if defined(SSL_ERROR_WANT_ACCEPT)
    case SSL_ERROR_WANT_ACCEPT:
      LINIODIAG_MSG("TLS(%s), CheckStatus: (want accept)", pTcpChannel->chnlConfig.chnlName);
      return(RFC_SOCKET_BLOCKED);
#endif

    case SSL_ERROR_WANT_X509_LOOKUP:
      /* Callback function must be called again - retry */
      LINIODIAG_MSG("TLS(%s), CheckStatus: (want lookup)", pTcpChannel->chnlConfig.chnlName);
      return(RFC_SOCKET_BLOCKED);

    case SSL_ERROR_SYSCALL:
      /* Fatal?? error */
      LINIODIAG_ERRORMSG("TLS(%s), CheckStatus: status =  (fatal)", pTcpChannel->chnlConfig.chnlName);
      pTcpChannel->tls.state = TMW_TLS_FAILED;
      return(RFC_SOCKET_FAILED);

    case SSL_ERROR_ZERO_RETURN:
      /* SSL connection closed */
      LINIODIAG_ERRORMSG("TLS(%s), CheckStatus: (zero)", pTcpChannel->chnlConfig.chnlName);
      pTcpChannel->tls.state = TMW_TLS_FAILED;
      return(RFC_SOCKET_FAILED);

    default:
      /* Unknown SSL error */
      LINIODIAG_ERRORMSG("TLS(%s), CheckStatus: (unknown)", pTcpChannel->chnlConfig.chnlName);
      pTcpChannel->tls.state = TMW_TLS_FAILED;
      return(RFC_SOCKET_FAILED);
    }
    return(RFC_SOCKET_SUCCESS);
  }
  else
  {
    /* Success */
    // LINIODIAG_MSG("TLS(%s), CheckStatus: (success)", pTcpChannel->chnlConfig.chnlName);
    pTcpChannel->tls.state = TMW_TLS_READY;
    return(RFC_SOCKET_SUCCESS);
  }
}

int TLSCheckRenegotiationStatus(TCP_IO_CHANNEL *pTcpChannel)
{
  /* If we have an outstanding request for renegotiation */
  if (pTcpChannel->tls.renegotiationRequested)
  {
    if (!SSL_renegotiate_pending(pTcpChannel->tls.pSSL))
    {
      LINIODIAG_MSG("TLS(%s), CheckRenegotiationStatus: second handshake", pTcpChannel->chnlConfig.chnlName);

      /* Here if client responded with Client Hello */
      /* This will be the second ssl_do_handshake, which completes the renegotiation */
      switch (TLSDoHandShake(pTcpChannel))
      {
      case RFC_SOCKET_SUCCESS:
        break;
      case RFC_SOCKET_BLOCKED:
        return(RFC_SOCKET_BLOCKED);
      case RFC_SOCKET_FAILED:
      default:
        return(RFC_SOCKET_FAILED);
      }
    }
  }
  return(RFC_SOCKET_SUCCESS);
}

int TLSDoHandShake(TCP_IO_CHANNEL *pTcpChannel)
{
  int status, result_action;

  /* If we are a client, don't ask for renegotiation at all */
#if defined(OPENSSL_VERSION_NUMBER) && (OPENSSL_VERSION_NUMBER < 0x1010000fL)
  if (!pTcpChannel->tls.pSSL->server)
#else
  if (!SSL_is_server(pTcpChannel->tls.pSSL))
#endif
    return(RFC_SOCKET_SUCCESS);

  LINIODIAG_MSG("TLS(%s), TLSDoHandShake\n", pTcpChannel->chnlConfig.chnlName);

  status = SSL_do_handshake(pTcpChannel->tls.pSSL);

  result_action = TLSCheckStatus(pTcpChannel, status);
  switch (result_action)
  {
  case RFC_SOCKET_SUCCESS:
    switch (pTcpChannel->tls.state)
    {
    case TMW_TLS_READY:
    case TMW_TLS_RENEGOTIATING:
      if (!pTcpChannel->tls.renegotiationRequested)
      {
        /* First call to SSL_do_handshake - just sent Hello Request */
        /* The call to SSL_do_handshake succeeded */
        pTcpChannel->tls.state = TMW_TLS_READY;

        /* Start timer to wait for client to renegotiate */
        pTcpChannel->tls.renegotiationTimeout = tmwtarg_getMSTime() + pTcpChannel->chnlConfig.nTlsRenegotiationMsTimeout;

        /* There is an old bug (1019 and 2146) in OpenSSL that (in violation of the TLS protocol)
           will not allow incoming data between the Hello Request from the server and the
           Client Hello that actually starts the renegotiation process.  To avoid this
           problem, we do not send data from the server side until the renegotiation is
           underway. This also keeps us from requesting another one while this one is in progress. */

        pTcpChannel->tls.renegotiationRequested = 1;
        LINIODIAG_MSG("TLS(%s), Renegotiation requested", pTcpChannel->chnlConfig.chnlName);
      }
      else
      {
        /* Second call to SSL_do_handshake - completed renegotiation */
        pTcpChannel->tls.renegotiationRequested = 0;
        pTcpChannel->tls.renegotiationCounter = 0;
        LINIODIAG_MSG("TLS(%s), Renegotiation completed", pTcpChannel->chnlConfig.chnlName);
      }
      break;
    default:
      break;
    }
    break;
  case RFC_SOCKET_BLOCKED:
    switch (pTcpChannel->tls.state)
    {
    case TMW_TLS_RENEGOTIATING:
      break;
    case TMW_TLS_READY:
    default:
      if (!pTcpChannel->tls.renegotiationRequested)
      {
        /* First call to SSL_do_handshake - trying to Hello Request */
        /* Change state so that we will kee trying the SSL_do_handshake until it succeeds */
        pTcpChannel->tls.state = TMW_TLS_RENEGOTIATING;
      }
      break;
    }
    break;
  case RFC_SOCKET_FAILED:
  default:
    break;
  }
  return(result_action);
}

/* Attempt to start a renegotiation process */
int TLSRenegotiate(TCP_IO_CHANNEL *pTcpChannel)
{
  LINIODIAG_MSG("TLS(%s), TLSRenegotiate called", pTcpChannel->chnlConfig.chnlName);

  switch (pTcpChannel->tls.state)
  {
  case TMW_TLS_IDLE:
    LINIODIAG_MSG("TLS(%s), TLSRenegotiate blocked", pTcpChannel->chnlConfig.chnlName);
    return(RFC_SOCKET_BLOCKED);

  case TMW_TLS_RENEGOTIATING:
    /* This state is set by TLSDoHandShake the first time through,
       and stays in that state until SSL_do_handshake succeeds, then goes
       to READY.
     */
    return(TLSDoHandShake(pTcpChannel));

  case TMW_TLS_READY:
    break;

  case TMW_TLS_FAILED:
  default:
    LINIODIAG_ERRORMSG("TLS(%s), TLSRenegotiate Failed", pTcpChannel->chnlConfig.chnlName);
    return(RFC_SOCKET_FAILED);
  }

  LINIODIAG_MSG("TLS(%s), TLSRenegotiate SSL_renegotiate", pTcpChannel->chnlConfig.chnlName);

  /* Here if we can start a new renegotiation */
  SSL_renegotiate(pTcpChannel->tls.pSSL);

  pTcpChannel->tls.renegotiationCounter = 0;

  return(TLSDoHandShake(pTcpChannel));
}

void TLSCheckForRenegotiation(TCP_IO_CHANNEL *pTcpChannel)
{
#if defined(OPENSSL_VERSION_NUMBER) && (OPENSSL_VERSION_NUMBER < 0x1010000fL)
  if (!pTcpChannel->tls.pSSL->server)
#else
  if (!SSL_is_server(pTcpChannel->tls.pSSL))
#endif
    return;

  if (pTcpChannel->tls.renegotiationRequested)
    return;     /* Already in progress */

  /* Has renegotiation timer expired? */
  if ((pTcpChannel->chnlConfig.nTlsRenegotiationMsTimeout) && (tmwtarg_getMSTime() > pTcpChannel->tls.renegotiationTimeout))
  {
    LINIODIAG_MSG("TLS(%s), Renegotiate required because of time", pTcpChannel->chnlConfig.chnlName);

    if (TLSRenegotiate(pTcpChannel) == RFC_SOCKET_FAILED)
    {
      LINIODIAG_ERRORMSG("TLS(%s), TLSRenegotiate() failed", pTcpChannel->chnlConfig.chnlName);
    }
    pTcpChannel->tls.renegotiationTimeout = tmwtarg_getMSTime() + pTcpChannel->chnlConfig.nTlsRenegotiationMsTimeout;
  }
}

/* A PDU has been received */
void TLSReceivedPdu(TCP_IO_CHANNEL *pTcpChannel)
{
#if defined(OPENSSL_VERSION_NUMBER) && (OPENSSL_VERSION_NUMBER < 0x1010000fL)
  if (!pTcpChannel->tls.pSSL->server)
#else
  if (!SSL_is_server(pTcpChannel->tls.pSSL))
#endif
    return;

  if (pTcpChannel->tls.renegotiationRequested)
    return;     /* Already in progress */

  /* Is this configured to renegotiate after a certain number of PDUs? */
  if (pTcpChannel->chnlConfig.nTlsRenegotiationCount > 0)
  {
    pTcpChannel->tls.renegotiationCounter++;
    if (pTcpChannel->tls.renegotiationCounter >= pTcpChannel->chnlConfig.nTlsRenegotiationCount)
    {
      LINIODIAG_MSG("TLS(%s), Renegotiate required because of message count", pTcpChannel->chnlConfig.chnlName);

      if (TLSRenegotiate(pTcpChannel) == RFC_SOCKET_FAILED)
      {
        LINIODIAG_ERRORMSG("TLS(%s), TLSRenegotiate() failed", pTcpChannel->chnlConfig.chnlName);
      }
    }
  }
}

/**********************************************************************************\
  Function    : lintls_initChannel
  Description : Perform all channel intialization	required to support a TLS connection
  Return :			bool	-	TMWDEFS_TRUE if successful, TMWDEFS_FALSE otherwise
  Parameters :
      pTcpChannel	-	Pointer to the TCP IO Channel
  Note : [none]
\**********************************************************************************/
TMWTYPES_BOOL lintls_initChannel(TCP_IO_CHANNEL *pTcpChannel)
{
  TMWTARG_LOCK_INIT(&pTcpChannel->tls.bufferLock);
  return TMWDEFS_TRUE;
}

/**********************************************************************************\
  Function : lintls_open
  Description : Open a TLS channel
  Return :			bool	-	TMWDEFS_TRUE if successful, TMWDEFS_FALSE otherwise
  Parameters :
      pTcpChannel	-	Pointer to the TCP IO Channel
  Note : [none]
\**********************************************************************************/
TMWTYPES_BOOL lintls_open(TCP_IO_CHANNEL *pTcpChannel)
{
  LINIODIAG_MSG("TLS(%s), open", pTcpChannel->chnlConfig.chnlName);

  if (pTcpChannel->chnlConfig.useTLS)
  {
    if (!TLSLoadSslLibrary())
      return TMWDEFS_FALSE;

    if (pTcpChannel->tls.pSslCtx == TMWDEFS_NULL)
    {
      pTcpChannel->tls.pSslCtx = TLSInitSSLCtx(pTcpChannel);
      if (pTcpChannel->tls.pSslCtx == TMWDEFS_NULL)
        return TMWDEFS_FALSE;
    }
  }
  return TMWDEFS_TRUE;
}

/**********************************************************************************\
  Function :		lintls_close
  Description : Close a TLS channel
  Return :			void	-
  Parameters :
      pTcpChannel	-	Pointer to the TCP IO Channel
  Note : [none]
\**********************************************************************************/
void lintls_close(TCP_IO_CHANNEL *pTcpChannel)
{
  LINIODIAG_MSG("TLS(%s), close", pTcpChannel->chnlConfig.chnlName);

  if (pTcpChannel->tls.pSSL != TMWDEFS_NULL)
  {
    int r = SSL_shutdown(pTcpChannel->tls.pSSL);
    if (!r) {
      /* If we called SSL_shutdown() first then
         we always get return value of '0'. In
         this case, try again, but first send a
         TCP FIN to trigger the other side's
         close_notify*/
      if (pTcpChannel->dataSocket != INVALID_SOCKET)
        shutdown(pTcpChannel->dataSocket, 1);

      r = SSL_shutdown(pTcpChannel->tls.pSSL);
    }

    /* Set buffer to empty so TLSWrite is not called after the pSSL is freed  */
    TMWTARG_LOCK_SECTION(&pTcpChannel->tls.bufferLock);
    pTcpChannel->tls.bufferReadIndex = pTcpChannel->tls.bufferWriteIndex;
    TMWTARG_UNLOCK_SECTION(&pTcpChannel->tls.bufferLock);

    SSL_free(pTcpChannel->tls.pSSL);
    pTcpChannel->tls.pSSL = TMWDEFS_NULL;
  }
}

/**********************************************************************************\
  Function :		lintls_delete
  Description : Delete a TLS channel
  Return :			void	-
  Parameters :
      pTcpChannel	-	Pointer to the TCP IO Channel
  Note : [none]
\**********************************************************************************/
void lintls_delete(TCP_IO_CHANNEL *pTcpChannel)
{
  LINIODIAG_MSG("TLS(%s), delete", pTcpChannel->chnlConfig.chnlName);

  /* Free the SSL context */
  if (pTcpChannel->tls.pSslCtx != TMWDEFS_NULL)
  {
    SSL_CTX_free(pTcpChannel->tls.pSslCtx);
    pTcpChannel->tls.pSslCtx = TMWDEFS_NULL;
  }

  TMWTARG_LOCK_DELETE(&pTcpChannel->tls.bufferLock);
}

/**********************************************************************************\
  Function    :	lintls_listen
  Description : Listen on a TLS channel
  Return      :	bool	-	TMWDEFS_TRUE if connection established, TMWDEFS_FALSE otherwise
  Parameters :
      pTcpChannel	-	Pointer to the TCP IO Channel
  Note : [none]
\**********************************************************************************/
TMWTYPES_BOOL lintls_listen(TCP_IO_CHANNEL *pTcpChannel, SOCKET acceptSocket)
{
  if (pTcpChannel->chnlConfig.useTLS)
  {
    BIO     *sslbio;
    sslbio = BIO_new_socket((int)acceptSocket, BIO_NOCLOSE);
    if (pTcpChannel->tls.pSSL == TMWDEFS_NULL)
    {
      pTcpChannel->tls.pSSL = SSL_new(pTcpChannel->tls.pSslCtx);
    }
    if (pTcpChannel->tls.pSSL == TMWDEFS_NULL)
    {
      LINIODIAG_ERRORMSG("TLS(%s), SSL_new failed", pTcpChannel->chnlConfig.chnlName);
      return TMWDEFS_FALSE;
    }

    SSL_set_bio(pTcpChannel->tls.pSSL, sslbio, sslbio);

    if (!SSL_set_cipher_list(pTcpChannel->tls.pSSL, TMW_ciphers))
    {
      LINIODIAG_ERRORMSG("TLS(%s), Can't set cyphers", pTcpChannel->chnlConfig.chnlName);
    }

    int retCode;
    int r = 0;
    int loopCount = 0;
    int loopMax = pTcpChannel->chnlConfig.tlsHandshakeMsTimeout / TLSCCHANNEL_DELAY;
    while (r <= 0)
    {
      r = SSL_accept(pTcpChannel->tls.pSSL);
      if (r <= 0)
      {
        retCode = SSL_get_error(pTcpChannel->tls.pSSL, r);
        if (retCode != SSL_ERROR_WANT_READ)
        {
          ERR_print_errors_cb(&printOpenSSLErrors, pTcpChannel);

          long verify_result = SSL_get_verify_result(pTcpChannel->tls.pSSL);
          if (verify_result != X509_V_OK)
          {
            char *pErrorString;
            pErrorString = (char *)X509_verify_cert_error_string(verify_result);
            LINIODIAG_ERRORMSG("TCP LISTENER: verify error, %d, %s", verify_result, pErrorString);
          }

          SSL_free(pTcpChannel->tls.pSSL);
          pTcpChannel->tls.pSSL = TMWDEFS_NULL;

          LINIODIAG_ERRORMSG("TCP LISTENER: SSL accept error, %d, %d\n", r, retCode);
          return TMWDEFS_FALSE;
        }
        if (loopCount++ > loopMax)
        {
          LINIODIAG_ERRORMSG("TLS(%s), accept error, handshake failure", pTcpChannel->chnlConfig.chnlName);
          SSL_free(pTcpChannel->tls.pSSL);
          pTcpChannel->tls.pSSL = TMWDEFS_NULL;
          return TMWDEFS_FALSE;
        }
        tmwtarg_sleep(TLSCCHANNEL_DELAY);
      }
    }

    if (CheckCert(pTcpChannel, pTcpChannel->tls.pSSL))
    {
      if (pTcpChannel->chnlConfig.nTlsRenegotiationSeconds > 0)
        pTcpChannel->tls.renegotiationTimeout = tmwtarg_getMSTime() + pTcpChannel->chnlConfig.nTlsRenegotiationMsTimeout;
    }
    else
    {
      LINIODIAG_ERRORMSG("TLS(%s), check cert failed", pTcpChannel->chnlConfig.chnlName);
      SSL_free(pTcpChannel->tls.pSSL);
      pTcpChannel->tls.pSSL = TMWDEFS_NULL;
      return TMWDEFS_FALSE;
    }
  }

  pTcpChannel->tls.state = TMW_TLS_READY;
  return TMWDEFS_TRUE;
}

/**********************************************************************************\
  Function :		lintls_connect
  Description : Attempt to connect to a remote server
  Return :			bool	-	TMWDEFS_TRUE if connection established, TMWDEFS_FALSE otherwise
  Parameters :
      pTcpChannel	-	Pointer to the TCP IO Channel
      tempCommSocket - file descriptor of socket
  Note : [none]
\**********************************************************************************/
TMWTYPES_BOOL lintls_connect(TCP_IO_CHANNEL *pTcpChannel, SOCKET tempCommSocket)
{
  TMWTYPES_BOOL status = TMWDEFS_TRUE;
  LINIODIAG_MSG("TLS(%s), Connect", pTcpChannel->chnlConfig.chnlName);

  /* Connect the SSL socket */
  pTcpChannel->tls.pSSL = SSL_new(pTcpChannel->tls.pSslCtx);
  if (pTcpChannel->tls.pSSL == TMWDEFS_NULL)
  {
    LINIODIAG_ERRORMSG("TLS(%s), connect error, ssl == NULL", pTcpChannel->chnlConfig.chnlName);
    return(TMWDEFS_FALSE);
  }

  BIO     *sslbio;
  sslbio = BIO_new_socket((int)tempCommSocket, BIO_NOCLOSE);
  SSL_set_bio(pTcpChannel->tls.pSSL, sslbio, sslbio);

  if (!SSL_set_cipher_list(pTcpChannel->tls.pSSL, TMW_ciphers))
  {
    LINIODIAG_ERRORMSG("TLS(%s), Can't set cyphers", pTcpChannel->chnlConfig.chnlName);
  }


  int retCode;
  int r = 0;
  int loopCount = 0;
  int loopMax = pTcpChannel->chnlConfig.tlsHandshakeMsTimeout / TLSCCHANNEL_DELAY;
  while (r <= 0)
  {
    /* If this gets nulled out somewhere else */
    if (pTcpChannel->tls.pSSL == TMWDEFS_NULL)
    {
      LINIODIAG_ERRORMSG("TLS(%s), connect error, pSSL == NULL", pTcpChannel->chnlConfig.chnlName);
      status = TMWDEFS_FALSE;
      break;
    }

    r = SSL_connect(pTcpChannel->tls.pSSL);
    if (r <= 0)
    {
      retCode = SSL_get_error(pTcpChannel->tls.pSSL, r);
      if (retCode != 2)
      {
        LINIODIAG_ERRORMSG("TLS(%s), connect error, %d, %d", pTcpChannel->chnlConfig.chnlName, r, retCode);
        status = TMWDEFS_FALSE;
        break;
      }
      tmwtarg_sleep(TLSCCHANNEL_DELAY);
      if (loopCount++ > loopMax)
      {
        LINIODIAG_ERRORMSG("TLS(%s), connect error, handshake failure", pTcpChannel->chnlConfig.chnlName);
        status = TMWDEFS_FALSE;
        break;
      }
    }
  }

  if (status)
  {
    if (!CheckCert(pTcpChannel, pTcpChannel->tls.pSSL))
    {
      LINIODIAG_ERRORMSG("TLS(%s), connect error, certification failure", pTcpChannel->chnlConfig.chnlName);
      status = TMWDEFS_FALSE;
    }
  }

  if (!status)
  {
    SSL_free(pTcpChannel->tls.pSSL);
    pTcpChannel->tls.pSSL = TMWDEFS_NULL;
    return(TMWDEFS_FALSE);
  }

  /* Return success */
  LINIODIAG_MSG("TLS(%s), Connect success", pTcpChannel->chnlConfig.chnlName);
  pTcpChannel->tls.state = TMW_TLS_READY;
  return(TMWDEFS_TRUE);
}

/* Transmit bytes to this TLS connection  */
int lintls_transmit(TCP_IO_CHANNEL *pTcpChannel, const TMWTYPES_UCHAR *pBufferToSend, TMWTYPES_USHORT numCharsToSend)
{
  // LINIODIAG_MSG("TLS(%s), transmit store %d bytes", pTcpChannel->chnlConfig.chnlName, numCharsToSend);
  return StoreInTLSWriteBuffer(pTcpChannel, pBufferToSend, numCharsToSend);
}

/**********************************************************************************\
  Function :		lintls_checkChannel
  Description : Service the TLS channel to see if data can be written from buffer
  Return :			bool	-	TMWDEFS_TRUE
  Parameters :
      pTcpChannel	-	Pointer to the TCP IO Channel
  Note : [none]
\**********************************************************************************/
TMWTYPES_BOOL lintls_checkChannel(TCP_IO_CHANNEL *pTcpChannel)
{
  if (pTcpChannel->tls.pSSL == TMWDEFS_NULL)
  {
    LINIODIAG_ERRORMSG("TLS(%s), checkChannel, pSSL == NULLL", pTcpChannel->chnlConfig.chnlName);
    return TMWDEFS_TRUE;
  }

  TransmitFromTLSWriteBuffer(pTcpChannel);
  return TMWDEFS_TRUE;
}

#else
#error TMWCNFG_USE_OPENSSL Must be defined to support TLS.
#endif /* TMWCNFG_USE_OPENSSL */
#endif /* TMWTARG_SUPPORT_TLS */
