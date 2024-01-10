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

/* file: tmwtargcnfg.h
 * description: Defines which target layer features are supported by this target layer.
 */
#include "tmwscl/utils/tmwcnfg.h"

#ifndef TMWTARGCNFG_DEFINED
#define TMWTARGCNFG_DEFINED

/* set this to TMWDEFS_FALSE to remove serial support */
#define TMWTARG_SUPPORT_232 TMWDEFS_TRUE

/* set this to TMWDEFS_FALSE to remove TCP support */
#define TMWTARG_SUPPORT_TCP TMWDEFS_TRUE

/* set this to TMWDEFS_TRUE to enable DNP target layer file support */
#define TMWTARG_SUPPORT_DNPFILEIO TMWDEFS_TRUE

/* set this to TMWDEFS_TRUE to enable I101 and 104 target layer file support */
#define TMWTARG_SUPPORT_14FILEIO  TMWDEFS_TRUE

/* set this to TMWDEFS_FALSE to remove UDP support */
/* UDP is required for DNP, otherwise it can be removed */
#define TMWTARG_SUPPORT_UDP TMWDEFS_TRUE

/* set this to TMWDEFS_FALSE to remove TLS support */
#define TMWTARG_SUPPORT_TLS TMWDEFS_TRUE

/* Set this to TMWDEFS_FALSE to remove channel thread support. 
 * Without the channel thread only polledMode=true is supported
 * and connect requests will occur in the application context.
 */
#define TMWTARG_SUPPORT_CHANNEL_THREAD TMWDEFS_TRUE

/* Set this to TMWDEFS_FALSE if target layer should not support 
 * multi timer implementation. This will allow polled MULTI timers
 * to be used by customer applications.
 */
#define TMWTARG_SUPPORT_MULTI_TIMER TMWDEFS_TRUE


#if TMWCNFG_USE_OPENSSL
#if TMWCNFG_USE_OPENSSL_1_0_2
  #define OPENSSL_CRYPTO_LIB   "libeay32.lib"
  #define OPENSSL_SSL_LIB      "ssleay32.lib"
  #define OPENSSL_CRYPTO_DLL   "libeay32.dll"
  #define OPENSSL_SSL_DLL      "ssleay32.dll"
#else
  #define OPENSSL_CRYPTO_LIB   "libcrypto.lib"
  #define OPENSSL_SSL_LIB      "libssl.lib"
  #if _WIN64
    #define OPENSSL_CRYPTO_DLL "libcrypto-1_1-x64.dll"
    #define OPENSSL_SSL_DLL    "libssl-1_1-x64.dll"
  #else
    #define OPENSSL_CRYPTO_DLL "libcrypto-1_1.dll"
    #define OPENSSL_SSL_DLL    "libssl-1_1.dll"
  #endif
#endif
#endif

#endif /* TMWTARGCNFG_DEFINED */
