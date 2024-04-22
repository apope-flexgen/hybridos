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

/* file: WinTLSChannel.cpp
 * description: Implementation of Windows TLS interface using OpenSSL.
 *  Much of this was derived from Triangle MicroWorks' 61850 TLS implementation.
 */
#include "StdAfx.h"
#pragma warning(disable : 4100)

#include <vector>

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#define DEBUG_TLS 0
#if DEBUG_TLS
#define printf TRACE0
#else
#define printf(x)
#endif

#endif /* _DEBUG */

#pragma comment(lib, "Ws2_32.lib")

#include "WinIoTarg/include/WinIoTarg.h"
#include "WinIoTarg/include/WinIoTargDefs.h"
#include "WinIoTarg/WinIoInterface.h"
#include "WinIoTarg/WinTLSChannel.h"

#if TMWTARG_SUPPORT_TLS && TMWCNFG_USE_OPENSSL
#pragma comment(lib, OPENSSL_CRYPTO_LIB)
#pragma comment(lib, OPENSSL_SSL_LIB)

#define WINTLSCCHANNEL_DELAY 50

TMWTYPES_BOOL OpenSSLLoaded = TMWDEFS_FALSE;

// Constructor
WinTLSChannel::WinTLSChannel(const WINTCP_CONFIG& config, TMWTARG_CONFIG* pTmwTargConfig)
    : WinTCPChannel(config, pTmwTargConfig)
{
    // TLS variables
    m_bTLSClose = false;
    m_TLSWriterThreadIsRunning = false;
    m_TLSBufferReadIndex = 0;
    m_TLSBufferWriteIndex = 0;
    m_TLSState = TMW_TLS_IDLE;
    m_TLSRenegotiationCounter = 0;
    m_TLSRenegotiationRequested = false;
    m_TLS_RenegotiationTimeout = false;
    m_sslCtx = TMWDEFS_NULL;
    m_pSSL = TMWDEFS_NULL;
    m_moreSSLData = 0;

    if (config.mode == WINTCP_MODE_SERVER)
        m_renegotiationTimer.Create(WinTLSChannel::RenegotiationTimeout, this, config.nTlsRenegotiationSeconds * 1000);
}

// Destructor
WinTLSChannel::~WinTLSChannel(void)
{
    m_renegotiationTimer.Stop();

    // Close channel if not already closed
    if (isChannelOpen())
        close();

    TLSDestroySSLCtx();
}

// called by OpenSSL to get password for private key files
static int password_cb(char* buf, int num, int rwflag, void* userdata)
{
    char* pPassword = (char*)userdata;
    if (num < (int)strlen(pPassword) + 1)
        return (0);

    strcpy(buf, pPassword);
    return ((int)strlen(pPassword));
}

static AutoCriticalSectionWinTarg tlsInitLock;
// Load the Open SSL dlls and initialize for TLS/SSL
bool WinTLSChannel::TLSLoadSSLLibrary(void)
{
    CriticalSectionLockWinTarg lock(tlsInitLock);
    if (!OpenSSLLoaded)
    {
        /* Global system initialization */
        HINSTANCE hinst = LoadLibrary(_T(OPENSSL_SSL_DLL));
        if (hinst == NULL)
        {
            WinIoTarg_formatErrorMsg(m_errorBuffer, WSAGetLastError());
            LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "TLS, load ssl dll failed\n%19s%s", " ", m_errorBuffer);
            return false;
        }
        hinst = LoadLibrary(_T(OPENSSL_CRYPTO_DLL));
        if (hinst == NULL)
        {
            WinIoTarg_formatErrorMsg(m_errorBuffer, WSAGetLastError());
            LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "TLS, load crypto dll failed\n%19s%s", " ", m_errorBuffer);
            return false;
        }

#if defined(OPENSSL_VERSION_NUMBER) && (OPENSSL_VERSION_NUMBER < 0x1010000fL)
        SSL_library_init(); /* load encryption & hash algorithms for SSL */
        CRYPTO_malloc_init();
        SSL_load_error_strings(); /* load the error strings for good error reporting */
        OpenSSL_add_all_algorithms();
        OpenSSL_add_all_ciphers();
        OpenSSL_add_all_digests();
#else
        OPENSSL_init_ssl(0, NULL);
#endif

        OpenSSLLoaded = TMWDEFS_TRUE;

        LogMessage(TMWDIAG_ID_TARGET, "TLS, Initialized SSL Library");
    }
    return true;
}

// Terminate OpenSSL on exiting entire protocol stack application.
void WinTLSChannel::TLSTerminateSslLibrary(void)
{
    if (OpenSSLLoaded)
    {
        OpenSSLLoaded = false;
#if defined(OPENSSL_VERSION_NUMBER) && (OPENSSL_VERSION_NUMBER < 0x1010000fL)
        CRYPTO_mem_leaks_fp(stderr);
        COMP_zlib_cleanup();
        CONF_modules_unload(1);
        OBJ_cleanup();
        EVP_cleanup();
        CRYPTO_cleanup_all_ex_data();
        ERR_remove_thread_state(NULL);
        ERR_remove_state(NULL);
        ENGINE_cleanup();
        ERR_free_strings();
#else
        // FIPS_mode_set(0); // we can comment this out because OpenSSL 3.0.7 has FIPS disabled by default
        ENGINE_cleanup();
        CONF_modules_unload(1);
        CRYPTO_cleanup_all_ex_data();
        ERR_free_strings();
#endif
    }
}

// Initialize SSL context.
static char* TMW_ciphers = "TLSv1:TLSv1.1:TLSv1.2:SSLv3:!SSLv2:!aNULL:!eNULL:!CAMELLIA:!EXPORT40:!EXPORT56:@STRENGTH";
SSL_CTX* WinTLSChannel::TLSInitSSLCtx()
{
    TMWTYPES_CHAR* pCaFileName = TMWDEFS_NULL;
    TMWTYPES_CHAR* pCaPathName = TMWDEFS_NULL;
    bool bFoundValidKey = false;

    /* Create our context*/
    /* This will allow TLS 1.0 1.1 and 1.2 */
#if defined(OPENSSL_VERSION_NUMBER) && (OPENSSL_VERSION_NUMBER < 0x1010000fL)
    m_sslCtx = SSL_CTX_new(SSLv23_method());
#else
    m_sslCtx = SSL_CTX_new(TLS_method());
#endif

    if (m_sslCtx == TMWDEFS_NULL)
    {
        LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "TLS, Couldn't create a new context");
        return TMWDEFS_NULL;
    }

    LogMessage(TMWDIAG_ID_TARGET, "TLS, Created context");

    SSL_CTX_set_options(m_sslCtx, SSL_OP_CIPHER_SERVER_PREFERENCE | SSL_OP_NO_SESSION_RESUMPTION_ON_RENEGOTIATION
                        // | SSL_OP_ALLOW_UNSAFE_LEGACY_RENEGOTIATION
    );

    SSL_CTX_set_default_passwd_cb_userdata(m_sslCtx, NULL);
    SSL_CTX_set_default_passwd_cb(m_sslCtx, password_cb);

    if (TLSLoadPrivateKeyFile(m_sslCtx, m_config.tlsDsaPrivateKeyFile, m_config.tlsDsaPrivateKeyPassPhrase,
                              m_config.tlsDsaCertificateId, TMWDEFS_FALSE /* bIsRsaKey */))
    {
        bFoundValidKey = true;
    }

    if (TLSLoadPrivateKeyFile(m_sslCtx, m_config.tlsRsaPrivateKeyFile, m_config.tlsRsaPrivateKeyPassPhrase,
                              m_config.tlsRsaCertificateId, TMWDEFS_TRUE /* bIsRsaKey */))
    {
        bFoundValidKey = true;
    }

    if (!bFoundValidKey)
    {
        LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "TLS, Can't read DSA or RSA key files");
        SSL_CTX_free(m_sslCtx);
        m_sslCtx = TMWDEFS_NULL;
        return TMWDEFS_NULL;
    }

    if (m_config.caFileName[0] != 0)
        pCaFileName = m_config.caFileName;

    if (m_config.caPathName[0] != 0)
        pCaPathName = m_config.caPathName;

    if (!(SSL_CTX_load_verify_locations(m_sslCtx, pCaFileName, pCaPathName)))
    {
        LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "TLS, Can't read CA list %s", m_config.caFileName);
    }
    else
    {
        LogMessage(TMWDIAG_ID_TARGET, "TLS, Read CA list %s", m_config.caFileName);

        if (pCaFileName != TMWDEFS_NULL)
            SSL_CTX_set_client_CA_list(m_sslCtx, SSL_load_client_CA_file(pCaFileName));

        if (m_config.dhFileName[0] != 0)
            TLSLoadDHParams(m_sslCtx, m_config.dhFileName);

        SSL_CTX_set_verify(m_sslCtx, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, NULL /* TMW_verify_callback */);
        SSL_CTX_set_verify_depth(m_sslCtx, m_config.nCaVerifyDepth);

        if (!SSL_CTX_set_cipher_list(m_sslCtx, TMW_ciphers))
        {
            LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "TLS, Can't set ciphers");
        }

        if (strlen(m_config.caCrlFileName) > 0)
        {
            char* pCaCrlFileName;
            X509_STORE* pCaStore = SSL_CTX_get_cert_store(m_sslCtx);
            X509_LOOKUP* lu = X509_STORE_add_lookup(pCaStore, X509_LOOKUP_file());
            pCaCrlFileName = m_config.caCrlFileName;
            if (0 == X509_load_crl_file(lu, pCaCrlFileName, X509_FILETYPE_PEM))
            {
                LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "TLS, Can't load crl file %s", pCaCrlFileName);
            }
            else
            {
                X509_STORE_set_flags(pCaStore, X509_V_FLAG_CRL_CHECK | X509_V_FLAG_CRL_CHECK_ALL);
            }
        }
        else
        {
            LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "TLS, No crl file was specified");
        }
    }
    return m_sslCtx;
}

// Destroy SSL context
void WinTLSChannel::TLSDestroySSLCtx()
{
    if (m_sslCtx != TMWDEFS_NULL)
        SSL_CTX_free(m_sslCtx);

    m_sslCtx = TMWDEFS_NULL;
}

// Load Diffie Hellman parameters
void WinTLSChannel::TLSLoadDHParams(SSL_CTX* pCtx, char* file)
{
    DH* ret = 0;
    BIO* bio;

    if ((bio = BIO_new_file(file, "r")) == NULL)
    {
        LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "TLS, Couldn't open DH file %s\n", file);
    }
    else
    {
        LogMessage(TMWDIAG_ID_TARGET, "TLS, Opened DH file");

        ret = PEM_read_bio_DHparams(bio, NULL, NULL, NULL);

        BIO_free(bio);

        if (SSL_CTX_set_tmp_dh(pCtx, ret) < 0)
            LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "TLS, Couldn't set DH parameters");
        else
            LogMessage(TMWDIAG_ID_TARGET, "TLS, Set DH parameters");
    }
}

// Load Private key file
bool WinTLSChannel::TLSLoadPrivateKeyFile(SSL_CTX* pSSLCtx, char* pKeyFileName, char* pKeyPassPhrase,
                                          char* pCertFileName, TMWTYPES_BOOL isRsaKey)
{
    if ((pKeyFileName == NULL) || (pKeyFileName[0] == 0))
    {
        return (false);
    }

    if ((pCertFileName == NULL) || (pCertFileName[0] == 0))
        pCertFileName = pKeyFileName; /* Try reading both from same file */

    if (!SSL_CTX_use_certificate_file(pSSLCtx, pCertFileName, SSL_FILETYPE_PEM))
    {
        LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "TLS, Can't read certificate from file");
        return (false);
    }

    SSL_CTX_set_default_passwd_cb_userdata(pSSLCtx, (void*)pKeyPassPhrase);
    if (isRsaKey)
    {
        if (!SSL_CTX_use_RSAPrivateKey_file(pSSLCtx, pKeyFileName, SSL_FILETYPE_PEM))
        {
            LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "TLS, Can't read private key from file %s\n",
                       pKeyFileName);
            SSL_CTX_set_default_passwd_cb_userdata(pSSLCtx, (void*)NULL);
            return (false);
        }
    }
    else
    {
        if (!SSL_CTX_use_PrivateKey_file(pSSLCtx, pKeyFileName, SSL_FILETYPE_PEM))
        {
            LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "TLS, Can't read private key file %s\n", pKeyFileName);
            SSL_CTX_set_default_passwd_cb_userdata(pSSLCtx, (void*)NULL);
            return (false);
        }
    }

    /* Clear passphrase since it has already been used */
    SSL_CTX_set_default_passwd_cb_userdata(pSSLCtx, (void*)NULL);

    return (true);
}

// Check Certificate
bool WinTLSChannel::CheckCert(SSL* ssl)
{
    X509* pPeerCert;
    TMWTYPES_BOOL status;
    TMWTYPES_LONG retValue;
    char buf[512];

    status = TMWDEFS_FALSE;

    retValue = SSL_get_verify_result(ssl);

    pPeerCert = SSL_get_peer_certificate(ssl);
    if (pPeerCert != NULL)
    {
        LogMessage(TMWDIAG_ID_TARGET, "TLS, Certificate");
        X509_NAME_oneline(X509_get_subject_name(pPeerCert), buf, sizeof buf);
        LogMessage(TMWDIAG_ID_TARGET, "TLS, Subject=%s", buf);
        X509_NAME_oneline(X509_get_issuer_name(pPeerCert), buf, sizeof buf);
        LogMessage(TMWDIAG_ID_TARGET, "TLS, Issuer=%s", buf);
    }

    if (retValue == X509_V_OK)
    {
        if (m_config.tlsCommonName[0] != 0)
        {
            char commonName[256];
            if (pPeerCert != TMWDEFS_NULL)
            {
                if (X509_NAME_get_text_by_NID(X509_get_subject_name(pPeerCert), NID_commonName, commonName,
                                              sizeof(commonName)))
                {
                    X509_free(pPeerCert);
                    if (strcmp(commonName, m_config.tlsCommonName) != 0)
                    {
                        LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR,
                                   "TLS, Common name does not match.\nExpecting %s  Received %s",
                                   m_config.tlsCommonName, commonName);
                    }
                    else
                    {
                        status = TMWDEFS_TRUE;
                    }
                }
                else
                {
                    LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "TLS, Can't get common name");
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
        LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "TLS, Certificate doesn't verify, verify result %d\n %s",
                   retValue, X509_verify_cert_error_string(retValue));
    }

    return status;
}

/**********************************************************************************\
        Function :			WinTLSChannel::StartTLSWriterThread
        Description : Start TLS Writer thread
        Return :			bool	-
        Parameters :
                        void	-
        Note : [none]
\**********************************************************************************/
bool WinTLSChannel::StartTLSWriterThread(void)
{
    // Start thread writing TLS transmits
    if (m_TLSWriterThreadIsRunning == false)
    {
        unsigned int threadID;
        m_bTLSWriterThreadRun = true;

        m_TLSWriterThreadHandle = (HANDLE)WinIoTarg_startThread(TLSWriterThread, this, &threadID,
                                                                THREAD_PRIORITY_BELOW_NORMAL);
        if (m_TLSWriterThreadHandle == INVALID_HANDLE_VALUE)
            return false;
    }

    return true;
}

/**********************************************************************************\
        Function :			WinTLSChannel::StopTLSWriterThread
        Description : Stop TLS Writer thread
        Return :			void	-
        Parameters :
        Note : [none]
\**********************************************************************************/
void WinTLSChannel::StopTLSWriterThread()
{
    // Stop writer thread
    if (m_TLSWriterThreadIsRunning == true)
    {
        m_bTLSWriterThreadRun = false;

        if (WaitForSingleObject(m_TLSWriterThreadHandle, 5000) == WAIT_TIMEOUT)
        {
            m_TLSWriterThreadIsRunning = false;
            WinIoTarg_endThread(m_TLSWriterThreadHandle);
        }
    }
}

/**********************************************************************************\
        Function :			WinTLSChannel::TLSWriterThread
        Description :  TLS Writer thread
        Return :			unsigned int __stdcall	-
        Parameters :
                        void *pParam	-
        Note : [none]
\**********************************************************************************/
unsigned int __stdcall WinTLSChannel::TLSWriterThread(void* pParam)
{
    WinTLSChannel* pTCPChannel = (WinTLSChannel*)pParam;

    {
        CriticalSectionLockWinTarg lock(pTCPChannel->getWriteLock());
        pTCPChannel->m_TLSWriterThreadIsRunning = true;
    }

    while (pTCPChannel->m_bTLSWriterThreadRun)
    {
        pTCPChannel->TransmitFromTLSWriteBuffer();
        Sleep(20);
    }

    if (pTCPChannel->m_TLSWriterThreadHandle != INVALID_HANDLE_VALUE)
    {
        CloseHandle(pTCPChannel->m_TLSWriterThreadHandle);
        pTCPChannel->m_TLSWriterThreadHandle = INVALID_HANDLE_VALUE;
    }

    pTCPChannel->m_TLSWriterThreadIsRunning = false;
    return 1;
}

/**********************************************************************************\
        Function :			WinTLSChannel::StoreInTLSWriteBuffer
        Description : Store Bytes to be transmitted in circular buffer.
        Return :
        Parameters :
                        void	-
        Note : [none]
\**********************************************************************************/
int WinTLSChannel::StoreInTLSWriteBuffer(const TMWTYPES_UCHAR* pBuf, TMWTYPES_USHORT length)
{
    if (length > 0)
    {
        CriticalSectionLockWinTarg lock(getWriteLock());
        int roomLeft = WINTCP_TLS_BUFFER_SIZE - m_TLSBufferWriteIndex;

        // Is there enough room at end of buffer?
        if (roomLeft >= length)
        {
            memcpy(&m_TLSWriteBuffer[m_TLSBufferWriteIndex], pBuf, length);
            m_TLSBufferWriteIndex += length;
            if (m_TLSBufferWriteIndex == WINTCP_TLS_BUFFER_SIZE)
                m_TLSBufferWriteIndex = 0;
        }
        else
        {
            int remainingLength;
            memcpy(&m_TLSWriteBuffer[m_TLSBufferWriteIndex], pBuf, roomLeft);
            remainingLength = length - roomLeft;
            memcpy(&m_TLSWriteBuffer[0], &pBuf[roomLeft], remainingLength);
            m_TLSBufferWriteIndex = remainingLength;
        }
    }
    return length;
}

/**********************************************************************************\
        Function :			WinTLSChannel::transmitFromTLSWriteBuffer
        Description : [none]
        Return :			TMWTYPES_BOOL	-
        Parameters :
                
        Note : [none]
\**********************************************************************************/
void WinTLSChannel::TransmitFromTLSWriteBuffer()
{
    int status;
    TMWTYPES_LONG bytesToTransmit = 0;
    TMWTYPES_UCHAR* writePtr = WINIOTARG_NULL;

    CriticalSectionLockWinTarg lock(getWriteLock());

    // See if there are any bytes in the circular buffer to transmit
    if (m_TLSBufferReadIndex == m_TLSBufferWriteIndex)
        return;

    writePtr = &m_TLSWriteBuffer[m_TLSBufferReadIndex];

    if (m_TLSBufferReadIndex < m_TLSBufferWriteIndex)
    {
        bytesToTransmit = m_TLSBufferWriteIndex - m_TLSBufferReadIndex;
        status = TLSWrite(writePtr, bytesToTransmit);
        if (status == bytesToTransmit)
        {
            m_TLSBufferReadIndex += bytesToTransmit;
        }
        else  // either failure, blocked, or some bytes transmitted
        {
            if (status > 0)
            {
                m_TLSBufferReadIndex += status;
            }
            else if (status == 0)
            {
                Sleep(50);
            }
        }
    }
    else
    {
        bytesToTransmit = WINTCP_TLS_BUFFER_SIZE - m_TLSBufferReadIndex;
        status = TLSWrite(writePtr, bytesToTransmit);
        if (status == bytesToTransmit)
        {
            m_TLSBufferReadIndex = 0;
        }
        else  // either failure, blocked, or some bytes transmitted
        {
            if (status > 0)
            {
                m_TLSBufferReadIndex += status;
            }
            else if (status == 0)
            {
                Sleep(50);
            }
        }
    }

    // wrap back to beginning of buffer
    if (m_TLSBufferReadIndex == WINTCP_TLS_BUFFER_SIZE)
        m_TLSBufferReadIndex = 0;
}

int WinTLSChannel::TLSWrite(TMWTYPES_UCHAR* pBuf, int numCharsToSend)
{
    printf("TLS, TLSWrite\n");

    switch (TLSCheckRenegotiationStatus())
    {
        case RFC_SOCKET_SUCCESS:
            break;
        case RFC_SOCKET_BLOCKED:
            return (0);
        case RFC_SOCKET_FAILED:
        default:
            return (-1);
    }

    if (m_TLSRenegotiationRequested)
    {
        // We block SSL_writes once renegotiation has been requested
        LogMessage(TMWDIAG_ID_TARGET, "TLS, TLSWrite renegotiate requested");
        printf("TLS, TLSWrite renegotiate requested");
        return (0);
    }

    int temp = SSL_pending(m_pSSL);
    if (temp)
    {
        LogMessage(TMWDIAG_ID_TARGET, "TLS, SSL_write blocked by ssl pending");
        printf("TLS, SSL_write blocked by ssl pending");
        return 0;
    }

    // Try to write
    int r = SSL_write(m_pSSL, pBuf, numCharsToSend);

    switch (TLSCheckStatus(r))
    {
        case RFC_SOCKET_SUCCESS:
            break;
        case RFC_SOCKET_BLOCKED:
            LogMessage(TMWDIAG_ID_TARGET, "TLS, SSL_write blocked");
            printf("TLS, SSL_write blocked");
            r = 0;
            break;
        case RFC_SOCKET_FAILED:
        default:
            LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "TLS, SSL_write failure");
            printf("TLS, SSL_write failure");
            r = -1;
            break;
    }

    return r;
}

int WinTLSChannel::TLSRead(SSL* ssl, SOCKET commSocket, TMWTYPES_UCHAR* pBuff, int maxNumChars)
{
    switch (TLSCheckRenegotiationStatus())
    {
        case RFC_SOCKET_SUCCESS:
            break;
        case RFC_SOCKET_BLOCKED:
            printf("TLS, SSL TLSRead socket blocked\n");
            return (0);
        case RFC_SOCKET_FAILED:
        default:
            printf("TLS, SSL TLSRead socket failed\n");
            return (-1);
    }

    int r = SSL_read(ssl, pBuff, maxNumChars);

    switch (TLSCheckStatus(r))
    {
        case RFC_SOCKET_SUCCESS:
            break;
        case RFC_SOCKET_BLOCKED:
            printf("TLS, TLSRead() blocked\n");
            return (0);
        case RFC_SOCKET_FAILED:
        default:
            printf("TLS, TLSRead() failed");
            return (-1);
    }

    LogMessage(TMWDIAG_ID_TARGET, "TLS, Receive, data was received\n");

    /* This could be called from higher up the protocol stacks if you wanted to count application layer PDUs
     * This would require each protocol that used TLS to call this function. For now, just count here.
     */
    TLSReceivedPdu();

    TLSCheckForRenegotiation();

    return r;
}

int printOpenSSLErrors(const char* str, size_t len, void* ui)
{
    WinTLSChannel* pWinTLSChannel = (WinTLSChannel*)ui;

    pWinTLSChannel->LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "openSSL error, %s\n", str);

    return 0;
}

int WinTLSChannel::TLSCheckStatus(int status)
{
    if (status <= 0)
    {
        printf("TLS, CheckStatus: status <=0");

        int last_error = SSL_get_error((SSL*)m_pSSL, status);
        switch (last_error)
        {
            case SSL_ERROR_NONE:
                /* Should only happen if status > 0 */
                printf(" (none) ");
                /* Returns SUCCESS, below */
                break;

            case SSL_ERROR_SSL:
                printf("Error queue:\n");
                /* SSL library failure */
                m_TLSState = TMW_TLS_FAILED;
                return (RFC_SOCKET_FAILED);

            case SSL_ERROR_WANT_READ:
                printf(" (want read)\n");
                return (RFC_SOCKET_BLOCKED);

            case SSL_ERROR_WANT_WRITE:
                printf(" (want write)\n");
                return (RFC_SOCKET_BLOCKED);

            case SSL_ERROR_WANT_CONNECT:
                printf(" (want connect)\n");
                return (RFC_SOCKET_BLOCKED);

#if defined(SSL_ERROR_WANT_ACCEPT)
            case SSL_ERROR_WANT_ACCEPT:
                printf(" (want accept)\n");
                return (RFC_SOCKET_BLOCKED);
#endif

            case SSL_ERROR_WANT_X509_LOOKUP:
                /* Callback function must be called again - retry */
                printf(" (want lookup)\n");
                return (RFC_SOCKET_BLOCKED);

            case SSL_ERROR_SYSCALL:
                /* Fatal?? error */
                printf(" (fatal)\n");
                m_TLSState = TMW_TLS_FAILED;
                return (RFC_SOCKET_FAILED);

            case SSL_ERROR_ZERO_RETURN:
                /* SSL connection closed */
                printf(" (zero)\n");
                m_TLSState = TMW_TLS_FAILED;
                return (RFC_SOCKET_FAILED);

            default:
                /* Unknown SSL error */
                printf(" (unknown)\n");
                m_TLSState = TMW_TLS_FAILED;
                return (RFC_SOCKET_FAILED);
        }
        return (RFC_SOCKET_SUCCESS);
    }
    else
    {
        /* Success */
        printf("TLSCheckStatus: success\n");
        m_TLSState = TMW_TLS_READY;
        return (RFC_SOCKET_SUCCESS);
    }
}

int WinTLSChannel::TLSCheckRenegotiationStatus()
{
    // If we have an outstanding request for renegotiation
    if (m_TLSRenegotiationRequested)
    {
        if (!SSL_renegotiate_pending((SSL*)m_pSSL))
        {
            printf("TLS, CheckRenegotiationStatus: second handshake\n");

            /* Here if client responded with Client Hello */
            /* This will be the second ssl_do_handshake, which completes the renegotiation */
            switch (TLSDoHandShake())
            {
                case RFC_SOCKET_SUCCESS:
                    break;
                case RFC_SOCKET_BLOCKED:
                    return (RFC_SOCKET_BLOCKED);
                case RFC_SOCKET_FAILED:
                default:
                    return (RFC_SOCKET_FAILED);
            }
        }
    }
    return (RFC_SOCKET_SUCCESS);
}

int WinTLSChannel::TLSDoHandShake()
{
    int status, result_action;

    // If we are a client, don't ask for renegotiation at all
#if defined(OPENSSL_VERSION_NUMBER) && (OPENSSL_VERSION_NUMBER < 0x1010000fL)
    if (!m_pSSL->server)
#else
    if (!SSL_is_server(m_pSSL))
#endif
        return (RFC_SOCKET_SUCCESS);

    printf("TCP TLS: TLSDoHandShake\n");

    status = SSL_do_handshake(m_pSSL);

    result_action = TLSCheckStatus(status);
    switch (result_action)
    {
        case RFC_SOCKET_SUCCESS:
            switch (m_TLSState)
            {
                case TMW_TLS_READY:
                case TMW_TLS_RENEGOTIATING:
                    if (!m_TLSRenegotiationRequested)
                    {
                        /* First call to SSL_do_handshake - just sent Hello Request */
                        /* The call to SSL_do_handshake succeeded */
                        m_TLSState = TMW_TLS_READY;

                        /* Start timer to wait for client to renegotiate */
                        // MMSd_time_stamp( &pTlsSecurityData->renegotiation_request_timeout );

                        /* There is an old bug (1019 and 2146) in OpenSSL that (in violation of the TLS protocol)
                           will not allow incoming data between the Hello Request from the server and the
                           Client Hello that actually starts the renegotiation process.  To avoid this
                           problem, we do not send data from the server side until the renegotiation is
                           underway. This also keeps us from requesting another one while this one is in progress. */

                        m_TLSRenegotiationRequested = 1;
                        LogMessage(TMWDIAG_ID_TARGET, "TLS, Renegotiation requested\n");
                        printf("TLS, Renegotiation requested\n");
                    }
                    else
                    {
                        /* Second call to SSL_do_handshake - completed renegotiation */
                        m_TLSRenegotiationRequested = 0;
                        m_TLSRenegotiationCounter = 0;
                        LogMessage(TMWDIAG_ID_TARGET, "TLS, Renegotiation completed\n");
                        printf("TLS, Renegotiation completed\n");
                    }
                    break;
                default:
                    break;
            }
            break;
        case RFC_SOCKET_BLOCKED:
            switch (m_TLSState)
            {
                case TMW_TLS_RENEGOTIATING:
                    break;
                case TMW_TLS_READY:
                default:
                    if (!m_TLSRenegotiationRequested)
                    {
                        /* First call to SSL_do_handshake - trying to Hello Request */
                        /* Change state so that we will kee trying the SSL_do_handshake until it succeeds */
                        m_TLSState = TMW_TLS_RENEGOTIATING;
                    }
                    break;
            }
            break;
        case RFC_SOCKET_FAILED:
        default:
            break;
    }
    return (result_action);
}

/* Attempt to start a renegotiation process */
int WinTLSChannel::TLSRenegotiate()
{
    printf("TLS: TLSRenegotiate called\n");

    switch (m_TLSState)
    {
        case TMW_TLS_IDLE:
            printf("TLS: TLSRenegotiate blocked\n");
            return (RFC_SOCKET_BLOCKED);

        case TMW_TLS_RENEGOTIATING:
            /* This state is set by TLSDoHandShake the first time through,
               and stays in that state until SSL_do_handshake succeeds, then goes
               to READY.
             */
            return (TLSDoHandShake());

        case TMW_TLS_READY:
            break;

        case TMW_TLS_FAILED:
        default:
            printf("TCP TLS: TLSRenegotiate Failed\n");
            return (RFC_SOCKET_FAILED);
    }

    printf("TCP TLS: TLSRenegotiate SSL_renegotiate\n");

    // Here if we can start a new renegotiation
    SSL_renegotiate(m_pSSL);

    m_TLSRenegotiationCounter = 0;

    return (TLSDoHandShake());
}

void WinTLSChannel::RenegotiationTimeout(void* pObject)
{
    WinTLSChannel* pWinTLSChannel = (WinTLSChannel*)pObject;
    pWinTLSChannel->m_TLS_RenegotiationTimeout = true;
}

void WinTLSChannel::TLSCheckForRenegotiation()
{
#if defined(OPENSSL_VERSION_NUMBER) && (OPENSSL_VERSION_NUMBER < 0x1010000fL)
    if (!m_pSSL->server)
#else
    if (!SSL_is_server(m_pSSL))
#endif
        return;

    if (m_TLSRenegotiationRequested)
        return; /* Already in progress */

    // Has renegotiation timer expired?
    if (m_TLS_RenegotiationTimeout)
    {
        LogMessage(TMWDIAG_ID_TARGET, "TLS, Renegotiate required because of time");

        if (TLSRenegotiate() == RFC_SOCKET_FAILED)
        {
            LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "TLS, TLSRenegotiate() failed");
        }
        m_TLS_RenegotiationTimeout = false;
        m_renegotiationTimer.StartTimeOut();
    }
}

/* A PDU has been received */
void WinTLSChannel::TLSReceivedPdu()
{
#if defined(OPENSSL_VERSION_NUMBER) && (OPENSSL_VERSION_NUMBER < 0x1010000fL)
    if (!m_pSSL->server)
#else
    if (!SSL_is_server(m_pSSL))
#endif
        return;

    if (m_TLSRenegotiationRequested)
        return; /* Already in progress */

    // Is this configured to renegotiate after a certain number of PDUs?
    if (m_config.nTlsRenegotiationCount > 0)
    {
        m_TLSRenegotiationCounter++;
        if (m_TLSRenegotiationCounter >= m_config.nTlsRenegotiationCount)
        {
            LogMessage(TMWDIAG_ID_TARGET, "TLS, Renegotiate required because of message count");
            printf("TLS, Renegotiate required because of message count");

            if (TLSRenegotiate() == RFC_SOCKET_FAILED)
            {
                LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "TLS, TLSRenegotiate() failed");
                printf("TLS, TLSRenegotiate() failed");
            }
        }
    }
}

/* function: Create */
WinTLSChannel* WinTLSChannel::Create(const void* pConfig, TMWTARG_CONFIG* pTmwTargConfig)
{
    WINTCP_CONFIG* pTCPConfig = (WINTCP_CONFIG*)pConfig;
    WinTLSChannel* pChannel = new WinTLSChannel(*pTCPConfig, pTmwTargConfig);
    pChannel->setChannelCallback(pTmwTargConfig->pChannelCallback, pTmwTargConfig->pCallbackParam);
    pChannel->m_pChannel = pTmwTargConfig->pChannel;

#if TMWTARG_SUPPORT_CHANNEL_THREAD
    if ((pTCPConfig->mode == WINTCP_MODE_CLIENT) || !pChannel->m_config.polledMode)
    {
        if (pTCPConfig->mode == WINTCP_MODE_CLIENT)
        {
            pChannel->m_chanThreadCheckMask = CT_CHECK_CONNECT;
        }
        if (!pChannel->m_config.polledMode)
        {
            pChannel->m_chanThreadCheckMask |= CT_CHECK_INPUT;
        }
        pChannel->ChannelThreadStart();
    }
#endif

    return pChannel;
}

/**********************************************************************************\
        Function :			WinTLSChannel::open
        Description : [none]
        Return :			bool	-
        Parameters :
                        void	-
        Note : [none]
\**********************************************************************************/
bool WinTLSChannel::TLSOpen(void)
{
    LogMessage(TMWDIAG_ID_TARGET, "TLS, open");

    m_bTLSClose = false;
    if (m_config.useTLS)
    {
        if (!TLSLoadSSLLibrary())
            return false;

        if (m_sslCtx == TMWDEFS_NULL)
        {
            if (TLSInitSSLCtx() == TMWDEFS_NULL)
                return false;
        }
    }
    return (true);
}

/**********************************************************************************\
        Function :			WinTLSChannel::TLSClose
        Description : [none]
        Return :			void	-
        Parameters :
                        void	-
        Note : [none]
\**********************************************************************************/
void WinTLSChannel::TLSClose(void)
{
    m_bTLSClose = true;
    m_renegotiationTimer.Stop();

    StopTLSWriterThread();

    if (m_pSSL != TMWDEFS_NULL)
    {
        int r = SSL_shutdown(m_pSSL);
        if (!r)
        {
            /* If we called SSL_shutdown() first then
               we always get return value of '0'. In
               this case, try again, but first send a
               TCP FIN to trigger the other side's
               close_notify*/
            if (m_clientSocket != INVALID_SOCKET)
                shutdown(m_clientSocket, 1);
            if (m_serverSocket != INVALID_SOCKET)
                shutdown(m_serverSocket, 1);

            r = SSL_shutdown(m_pSSL);
        }

        SSL_free(m_pSSL);
        m_pSSL = TMWDEFS_NULL;
    }
}

/**********************************************************************************\
        Function :			WinTLSChannel::TLSListen
        Description :
        Return :
        Parameters :
        Note : [none]
\**********************************************************************************/
bool WinTLSChannel::TLSListen(SOCKET acceptSocket)
{
    if (m_config.useTLS)
    {
        BIO* sslbio;
        sslbio = BIO_new_socket((int)acceptSocket, BIO_NOCLOSE);

        SSL* pSSL = SSL_new(m_sslCtx);
        if (pSSL == TMWDEFS_NULL)
        {
            LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "TCP LISTENER: SSL, SSL_new failed");
            return false;
        }

        SSL_set_bio(pSSL, sslbio, sslbio);

        if (!SSL_set_cipher_list(pSSL, TMW_ciphers))
        {
            printf("Can't set cyphers");
        }

        int retCode;
        int r = 0;
        int loopCount = 0;
        int loopMax = m_config.tlsHandshakeMsTimeout / WINTLSCCHANNEL_DELAY;
        while ((r <= 0) && !m_bTLSClose)
        {
            /* Channel has been closed */
            if (m_bTLSClose)
            {
                LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "TLS, accept error, TLS Closed");
                SSL_free(pSSL);
                return false;
            }
            r = SSL_accept(pSSL);
            if (r <= 0)
            {
                retCode = SSL_get_error(pSSL, r);
                if (retCode != SSL_ERROR_WANT_READ)
                {
                    char* pErrorString;
                    ERR_print_errors_cb(&printOpenSSLErrors, this);

                    long verify_result = SSL_get_verify_result(pSSL);
                    if (verify_result != X509_V_OK)
                        pErrorString = (char*)X509_verify_cert_error_string(verify_result);

                    SSL_free(pSSL);
                    LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "TCP LISTENER: SSL accept error, %d, %d\n", r,
                               retCode);
                    return false;
                }
                if (loopCount++ > loopMax)
                {
                    SSL_free(pSSL);
                    LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "TLS, accept error, handshake failure");
                    return false;
                }
                Sleep(WINTLSCCHANNEL_DELAY);
            }
        }

        if (CheckCert(pSSL))
        {
            if (m_config.nTlsRenegotiationSeconds > 0)
                m_renegotiationTimer.StartTimeOut();

            if (!StartTLSWriterThread())
            {
                SSL_free(pSSL);
                LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "TLS, TLSListen failed to start TLS writer thread");
                return (false);
            }
        }
        else
        {
            SSL_free(pSSL);
            LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "TLS, check cert failed");
            return false;
        }
        m_pSSL = pSSL;
    }

    m_TLSState = TMW_TLS_READY;
    return true;
}

/**********************************************************************************\
        Function :			WinTLSChannel::TLSConnect
        Description : Attempt to connect to a remote server
        Return :			bool	-
        Parameters :
                        void	-
        Note : [none]
\**********************************************************************************/
bool WinTLSChannel::TLSConnect(SOCKET tempCommSocket)
{
    bool status = true;
    LogMessage(TMWDIAG_ID_TARGET, "TLS, Connect");

    /* Connect the SSL socket */
    SSL* pSSL = SSL_new(m_sslCtx);
    if (pSSL == TMWDEFS_NULL)
    {
        LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "TLS, connect error, ssl == NULL");
        return (false);
    }

    BIO* sslbio;
    sslbio = BIO_new_socket((int)tempCommSocket, BIO_NOCLOSE);
    SSL_set_bio(pSSL, sslbio, sslbio);

    if (!SSL_set_cipher_list(pSSL, TMW_ciphers))
    {
        printf("Can't set cyphers");
    }

    int retCode;
    int r = 0;
    int loopCount = 0;
    int loopMax = m_config.tlsHandshakeMsTimeout / WINTLSCCHANNEL_DELAY;
    while ((r <= 0) && !m_bTLSClose)
    {
        /* Channel has been closed */
        if (m_bTLSClose)
        {
            LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "TLS, connect error, TLS Closed");
            status = false;
            break;
        }

        r = SSL_connect(pSSL);
        if (r <= 0)
        {
            retCode = SSL_get_error(pSSL, r);
            if (retCode != 2)
            {
                LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "TLS, connect error, %d, %d", r, retCode);
                status = false;
                break;
            }
            Sleep(WINTLSCCHANNEL_DELAY);
            if (loopCount++ > loopMax)
            {
                LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "TLS, connect error, handshake failure");
                status = false;
                break;
            }
        }
    }

    if (status)
    {
        if (!CheckCert(pSSL))
        {
            LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "TLS, connect error, certification failure");
            status = false;
        }
    }
    if (status)
    {
        if (!StartTLSWriterThread())
        {
            LogMessage(TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR, "TLS, failed to start TLS writer thread");
            status = false;
        }
    }

    if (!status)
    {
        SSL_free(pSSL);
        return (false);
    }

    /* Return success */
    LogMessage(TMWDIAG_ID_TARGET, "TLS, Connect success");
    m_pSSL = pSSL;
    m_TLSState = TMW_TLS_READY;
    return (true);
}

// Transmit bytes to this TLS connection
int WinTLSChannel::TLSTransmit(const TMWTYPES_UCHAR* pBufferToSend, TMWTYPES_USHORT numCharsToSend)
{
    LogMessage(TMWDIAG_ID_TARGET, "TLS, transmit store %d bytes", numCharsToSend);
    return StoreInTLSWriteBuffer(pBufferToSend, numCharsToSend);
}

/**********************************************************************************\
        Function :			WinTLSChannel::TLSLowReceive
        Description : Low level receive method
        Return :			TMWTYPES_USHORT	-
        Parameters :
                          TMWTYPES_UCHAR *pBuff	-
                          TMWTYPES_ULONG maxNumChars	-
        Note : [none]
\**********************************************************************************/
int WinTLSChannel::TLSLowReceive(TMWTYPES_UCHAR* pBuff, TMWTYPES_ULONG maxNumChars)
{
    int numReceived = 0;
    fd_set readfds;

    if (m_moreSSLData)
    {
        numReceived = (TMWTYPES_USHORT)TLSRead(m_pSSL, m_commSocket, pBuff, maxNumChars);
        m_moreSSLData = SSL_pending(m_pSSL);
        return numReceived;
    }

    FD_ZERO(&readfds);
#pragma warning(disable : 4127)
    FD_SET(m_commSocket, &readfds);
#pragma warning(default : 4127)
    struct timeval timeout = { 0, 0 };

    // Call select with timeout of 0 to see if data is available
    if (select(0, &readfds, WINIOTARG_NULL, WINIOTARG_NULL, &timeout) == SOCKET_ERROR)
    {
        if (m_pChannelCallback != WINIOTARG_NULL)
            m_pChannelCallback(m_pCallbackParam, false, TMWDEFS_TARG_OC_FAILURE);
        else
            close();
    }
    else if (FD_ISSET(m_commSocket, &readfds))
    {
        int returnValue;
        returnValue = TLSRead(m_pSSL, m_commSocket, pBuff, maxNumChars);
        m_moreSSLData = SSL_pending(m_pSSL);

        if (returnValue == SOCKET_ERROR)
        {
            if (m_pChannelCallback != WINIOTARG_NULL)
                m_pChannelCallback(m_pCallbackParam, false, TMWDEFS_TARG_OC_FAILURE);

            else
                close();
        }
        else
        {
            numReceived = (TMWTYPES_USHORT)returnValue;
        }
    }

    return (numReceived);
}

#endif
