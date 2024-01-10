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

/* file: tmwcnfg.h
 * description: This file specifies configuration parameters that apply to
 *  any TMW source code library.
 */
#ifndef TMWCNFG_DEFINED
#define TMWCNFG_DEFINED
#include "tmwscl/utils/tmwdefs.h"

#if TMW_USE_PRIVATE_DEFINES 
/* Include private definitions for internal TMW applications.
 * SCL customers should not define TMW_USE_PRIVATE_DEFINES
 * or TMW_USE_GATEWAY_DEFINES.
 * 
 * .NET SCL no longer defines TMW_USE_PRIVATE_DEFINES to select functionality
 * beyond the SCL defaults. tmwprvt.h is no longer included or modified to
 * select that extra functionality to match what is compiled in for the 
 * 21 day install demo library.
 * 
 * It is important for you to choose carefully what functionality you want
 * compiled in and provided by your application. 
 *
 * Modify *cnfg.h and *data.h files as described in the xx xxx User Manual.pdf
 * to choose what functionality is supported.
 */
#include "tmwscl/config/tmwprvt.h"
#endif

#if TMW_USE_GATEWAY_DEFINES
 /* Include private definitions for TMW Gateway application. */

#include "tmwscl/config/tmwsdg.h"
#endif

/* The TMW_PRIVATE define is intended only for use by internal TMW applications 
 * and is not required by customer applications.
 */

/* TMWTARG_HAS_VSNPRINTF specifies if the target C runtime library has a
 * vsnprintf function.  If this function exists length checking will be used
 * for tmwtarg_snprintf() function calls.  If not defined the length will not
 * be checked for tmwtarg_snprintf() function calls, note that this may cause 
 * buffer overflows if the resulting length of a sprintf call exceeds the 
 * length of the supplied buffer.
 */
#define TMWCNFG_HAS_VSNPRINTF         TMWDEFS_TRUE

/* TMWCNFG_SUPPORT_DIAG enables the generation of diagnostic information
 * which is passed to the target application by the tmwtarg_putDiagString
 * method defined below. Setting this parameter to TMWDEFS_FALSE will 
 * disable the generation of diagnostic information.
 */
#define TMWCNFG_SUPPORT_DIAG          TMWDEFS_TRUE

/* TMWCNFG_SUPPORT_STATS enables the generation of statistical information
 * which is passed to the target application by the tmwtarg_updateStatistics
 * method defined below. Setting this parameter to TMWDEFS_FALSE line will 
 * disable the generation of statistical information.
 */
#define TMWCNFG_SUPPORT_STATS         TMWDEFS_TRUE

/* Define whether or not single and/or double precision floating point 
 * support should be included. 
 *
 * Setting this parameter to TMWDEFS_FALSE will remove all references 
 * to single precision floating point variables in the TMW SCL. This is
 * required for some data types in DNP and 101, 103 and 104. Setting this
 * to false will remove support for the data types that require single 
 * precision floating point.
 */
#define TMWCNFG_SUPPORT_FLOAT         TMWDEFS_TRUE

/* Setting this parameter to TMWDEFS_FALSE will remove all references 
 * to double precision floating point variables in the TMW SCL.
 * This is required for some data types in DNP. Setting this to false
 * will remove support for the data types that require double 
 * precision floating point. 
 */
#define TMWCNFG_SUPPORT_DOUBLE        TMWDEFS_TRUE

/* The simulated database requires double precision data types to provide
 * complete precision for both single precision floating point and large 
 * long values. Even though both types require 32 bits of storage, single precision
 * only provides for 23 bits a precision with a larger range, while a long allows
 * for 32 bits of precision with a smaller range. 
 * Setting this to TMWDEFS_FALSE will allow the simulated database to be 
 * compiled without using double precision types, but at the expense of data 
 * precision.  If short floats and longs are both required, your database may need
 * to provide better precision than the example simulated database.
 */
#define TMWCNFG_SIM_USE_DOUBLE        TMWDEFS_TRUE

/* The simulated database requires extended strings so support DNP Object group 114.
 * Setting this to TMWDEFS_FALSE will allow the simulated database to be 
 * consume less memory, but will not be able to support DNP Object group 114.
 */
#define TMWCNFG_SIM_SUPPORT_EXT_STRINGS   TMWDEFS_TRUE

/* Define whether or not multiple threads are supported by the TMW SCL. If 
 * this parameter is set to TMWDEFS_FALSE it is assumed that all the TMW SCL 
 * code will run on a single thread (or different threads but not concurrently)
 * and all resource locking will be removed. If this parameter is set to 
 * TMWDEFS_TRUE the TMW SCL will provide resource locking for each channel, 
 * and optionally for the database processing queue as specified by the 
 * TMWCNFG_SUPPORT_ASYNCH_DB parameter. This resource locking is managed 
 * entirely within the SCL and no special support is required by the user.
 * For more information see the section on multi-threaded applications in
 * the TMW SCL user manual.
 */
#define TMWCNFG_SUPPORT_THREADS       TMWDEFS_TRUE

/* Define whether or not multiple timer queues, one for each channel are
 * supported. The default configuration for the SCL is to create a single
 * queue for all SCL timers and to require only a single system or polled timer.
 * For some multithreaded architectures it is desirable to create a separate 
 * timer queue for each channel. This allows the timer callback functions to be
 * run in the context of the channel and can reduce resource contention. Setting 
 * this to TMWDEFS_FALSE will not create separate timer queues. Setting this
 * to TMWDEFS_TRUE will create separate queues and require multiple system 
 * timers through a call to tmwtarg_setMultiTimer().
 */
#define TMWCNFG_MULTIPLE_TIMER_QS     TMWDEFS_FALSE

/* Define whether database processing should be performed asynchronous
 * to the rest of the TMW SCL processing in Master SCLs. This parameter is
 * ignored in Slave SCLs. If this parameter is set to TMWDEFS_TRUE database
 * updates will be queued during message parsing. The user can then process
 * the updates from the queue as time permits, possibly on a different 
 * thread. This feature is useful if database updates take a significant
 * amount of time such that processing them while parsing the message would
 * delay further processing of the time dependent SCL code beyond acceptable
 * limits. Note that the use of asynchronous database updates currently
 * requires the use of dynamic memory, hence TMWCNFG_USE_DYNAMIC_MEMORY
 * must be set to TMWDEFS_TRUE.
 */
#ifndef TMWCNFG_SUPPORT_ASYNCH_DB
#define TMWCNFG_SUPPORT_ASYNCH_DB     TMWDEFS_TRUE
#endif

/* If this parameter is set to TMWDEFS_TRUE the TMW SCL will use a simulated
 * database. Define this to TMWDEFS_FALSE to remove all references to the
 * simulated database.
 */
#ifndef TMWCNFG_USE_SIMULATED_DB
#define TMWCNFG_USE_SIMULATED_DB       TMWDEFS_TRUE
#endif

/* TMWCNFG_SUPPORT_RXCALLBACKS controls whether certain optional callback
 * functionality provided for test or processing of received messages by
 * applications outside of the SCL is compiled in.
 * DNPLINK_CONTEXT pRxHeaderCallback
 * TMWLINK_CONTEXT pUserRxFrameCallback
 * TMWTPRT_CONTEXT pUserParseFunc
 * TMWPHYS_CONTEXT pUserParseFunc
 * at .NET level
 * TMWChannel::rxLinkHdrCallbackEvent uses DNPLINK_CONTEXT pRxHeaderCallback 
 *  only registered and implemented by DNP for now
 * TMWChannel::rxLinkCallbackEvent uses TMWLINK_CONTEXT pUserRxFrameCallback 
 *  only registered and implemented by DNP for now
 * TMWChannel::rxTransportCallbackEvent uses TMWTPRT_CONTEXT pUserParseFunc 
 *  only for DNP transport
 * TMWChannel::rxPhysCallbackEvent uses TMWPHYS_CONTEXT pUserParseFunc 
 *  registered and implemented for all channels.
 */
#ifndef TMWCNFG_SUPPORT_RXCALLBACKS
#define TMWCNFG_SUPPORT_RXCALLBACKS TMWDEFS_FALSE
#endif

/* Compile in common Cryptography interface tmwcrypto.h/c */
#ifndef TMWCNFG_SUPPORT_CRYPTO
#define TMWCNFG_SUPPORT_CRYPTO         TMWDEFS_FALSE
#endif

/* Compile in optional Asymmetric (public key) cryptography interface */
#ifndef TMWCNFG_SUPPORT_CRYPTO_ASYM
#define TMWCNFG_SUPPORT_CRYPTO_ASYM    TMWDEFS_FALSE
#endif

/* Compile in optional AES-GMAC Hash Algorithm support  
 * If using OpenSSL this requires a version that contains aes-gmac.c 
 * such as openssl 1.1.1beta1.
 */
#ifndef TMWCNFG_SUPPORT_CRYPTO_AESGMAC
#define TMWCNFG_SUPPORT_CRYPTO_AESGMAC TMWDEFS_FALSE
#endif

/* If you want to support Secure Authentication using the common 
 * cryptography interface (TMWCNFG_SUPPORT_CRYPTO) and you do not use OpenSSL
 * you must modify tmwcrypto.c/h to use another cryptography library
 * To allow a test build as delivered simulated crypto code that builds but
 * does NOT actually perform cryptography is provided.
 * If you implement your own cryptography code, this define Must be set
 * to TMWDEFS_FALSE
 */
#define TMWCNFG_USE_SIMULATED_CRYPTO     TMWDEFS_TRUE

/* If you want to support Secure Authentication using the common 
 * cryptography interface (TMWCNFG_SUPPORT_CRYPTO) you must modify
 * tmwcrypto.c/h to provide a database to store users and keys.
 * To allow a test build as delivered simulated crypto database code that stores
 * a small amount of data in memory is provided.
 * When you implement database, this define Must be set to TMWDEFS_FALSE
 */
#define TMWCNFG_USE_SIMULATED_CRYPTO_DB  TMWDEFS_TRUE

/* Use OpenSSL implementation for tmwcrypto interface if Secure Authentication 
 * is supported and/or for TLS if it is supported.
 */
#ifndef TMWCNFG_USE_OPENSSL
#define TMWCNFG_USE_OPENSSL              TMWDEFS_FALSE
#endif
/* Use OpenSSL version 1_0_2x instead of 1.1.1. OpenSSL library introduced a number
 * of API changes in 1.1.1 so this conditional was introduced to accomodate the 
 * changes. If the application is using a 1.0.x OpenSSL library, this define 
 * must be set to TMWDEFS_TRUE.
 */
#define TMWCNFG_USE_OPENSSL_1_0_2        TMWDEFS_FALSE

/* Specify whether dynamic memory allocation is supported. Set this parameter
 * to TMWDEFS_FALSE and specify limits for each data type below to use
 * static compile time memory allocation. Set this to TMWDEFS_TRUE to support 
 * dynamic memory allocation at runtime (tmwtarg_alloc() will be called).
 * Note that the maximum limits specified below for each allocated data 
 * type apply when using any type of memory allocation. Hence it
 * is possible to use dynamic memory allocation and still specify a 
 * maximum limit to the amount of memory that will be used by the TMW SCL.
 */
#define TMWCNFG_USE_DYNAMIC_MEMORY    TMWDEFS_TRUE

/* To support allocation of all of the memory required for each memory pool  
 * at startup, set both TMWCNFG_USE_DYNAMIC_MEMORY and 
 * TMWCNFG_ALLOC_ONLY_AT_STARTUP to TMWDEFS_TRUE. It is also necessary to specify
 * the limit for each type of data below. The SCL will call tmwtarg_alloc() 
 * once for each pool and then will manage the memory internally.
 * To change the number of buffers in each pool at runtime, call protocol specific
 * xxxmem_initMemory functions such as sdnpmem_initMemory() or 
 * m101mem_initMemory() to change the compiled in quantities.
 */
#define TMWCNFG_ALLOC_ONLY_AT_STARTUP TMWDEFS_FALSE

/* Specify if a processor requires long word (4byte) alignment or if the 
 * compiler creates unpacked structures. Setting this to TMWDEFS_TRUE will pad
 * TMWMEM_HEADER to address memory alignment and sizof(TMWMEM_HEADER) issues.
 * If sizeof(TMWMEM_HEADER) == 2, but the data field in tmwmem.h starts at
 * offset 4, (2 bytes were left empty), this should be set to TMWDEFS_TRUE to
 * pad structure TMWMEM_HEADER to 4 bytes.
 */
#define  TMWCNFG_MEMORY_ALIGN_NEEDED  TMWDEFS_FALSE
 
/* Specify maximum number of consecutive calls to tmwtarg_receive() to receive  
 * data when tmwtarg_receive returns non zero. This can be used to prevent 
 * looping in the SCL when a constant stream of data is being received. This 
 * applies in both the poll for input or event driven input models.
 * NOTE: it may take multiple calls to tmwtarg_receive to receive a single frame.
 * This is set to a large number by default, reduce it only as needed.
 */ 
#define TMWCNFG_MAX_APPLRCVS          512
 
/* Specify the maximum number of application contexts that can be in use
 * at once. Typically this value will be 1 per supported protocol unless
 * different threads are being used to manage multiple sessions.
 */
#define TMWCNFG_MAX_APPLICATIONS      TMWDEFS_NO_LIMIT

/* Specify the maximum number of channels that can be open at once.
 */
#define TMWCNFG_MAX_CHANNELS          TMWDEFS_NO_LIMIT

/* Specify the maximum number of sessions that can be open at once.
 */
#define TMWCNFG_MAX_SESSIONS          TMWDEFS_NO_LIMIT

/* Specify the maximum number of sectors that can be open at once.
 * For protocols that do not support sectors this parameter is ignored.
 */
#define TMWCNFG_MAX_SECTORS           TMWDEFS_NO_LIMIT

/* Specify the maximum number of application layer messages that can be
 * outstanding at one time (i.e. the total number of simultaneous commands
 * outstanding or the number of simultaneous msgs sent from a slave in 104)
 * This parameter can generally be set to 1 per session/sector for slave devices
 * other than IEC 60870-5-104. (SDNP configured for Identical Unsolicited Response 
 * Retries requires 2). For 104 slaves and other master devices this should be set to
 * N * the number of sessions or sectors where N is the maximum number of messages
 * that will be queued at the application layer. 
 */
#define TMWCNFG_MAX_APPL_MSGS         TMWDEFS_NO_LIMIT

/* Specify the maximum number of events that can be queued per data type.
 * Each data type maintains it's own queue of events. This parameter
 * specifies the default maximum length for each queue.
 */
#define TMWCNFG_MAX_EVENTS            TMWDEFS_NO_LIMIT

/* Specify the maximum number of simulated databases at that can be open
 * at one time. Generally there will be one database for each IEC sector 
 * or DNP session. This parameter is ignored if TMWCNFG_USE_SIMULATED_DB
 * is set to TMWDEFS_FALSE.
 */
#define TMWCNFG_MAX_SIM_DATABASES     TMWDEFS_NO_LIMIT

/* Specify the maximum number of simulated database points that can exist
 * at one time. This parameter is ignored if TMWCNFG_USE_SIMULATED_DB
 * is set to TMWDEFS_FALSE.
 */
#define TMWCNFG_MAX_POINTS            TMWDEFS_NO_LIMIT

/* Specify the maximum number of simulated database strings that can exist
 * at one time. This parameter is ignored if TMWCNFG_USE_SIMULATED_DB
 * is set to TMWDEFS_FALSE.
 */
#define TMWCNFG_MAX_STRINGS           TMWDEFS_NO_LIMIT
#define TMWCNFG_MAX_EXT_STRINGS       TMWDEFS_NO_LIMIT

/* Specify the maximum number of bytes that can be buffered at the 
 * physical layer. This parameter is only used if TMWCNFG_USE_DYNAMIC_MEMORY
 * is TMWDEFS_FALSE.
 */
#define TMWCNFG_MAX_RX_BUFFER_LENGTH 265

/* This should only be defined when a .NET Source Code Library has been purchased 
 * in addition to the ANSI C SCL 
 */ 
/* define TMWCNFG_USE_MANAGED_SCL TMWDEFS_TRUE */

/* check to make sure memory related defines do not conflict */
#if !TMWCNFG_USE_DYNAMIC_MEMORY && TMWCNFG_ALLOC_ONLY_AT_STARTUP  
#pragma message("If dynamic memory is not supported, can't use alloc only at startup") 
#endif

#if !TMWCNFG_USE_DYNAMIC_MEMORY && TMWCNFG_SUPPORT_ASYNCH_DB
#pragma message("If dynamic memory is not supported, can't use async database") 
#endif

#if TMWCNFG_ALLOC_ONLY_AT_STARTUP
#if (TMWCNFG_MAX_APPLICATIONS == TMWDEFS_NO_LIMIT \
  || TMWCNFG_MAX_CHANNELS == TMWDEFS_NO_LIMIT \
  || TMWCNFG_MAX_SESSIONS == TMWDEFS_NO_LIMIT \
  || TMWCNFG_MAX_SECTORS == TMWDEFS_NO_LIMIT \
  || TMWCNFG_MAX_APPL_MSGS == TMWDEFS_NO_LIMIT \
  || TMWCNFG_MAX_EVENTS == TMWDEFS_NO_LIMIT)
#pragma message("If TMWCNFG_ALLOC_ONLY_AT_STARTUP, TMWDEFS_NO_LIMIT is not allowed")
#endif 

#if TMWCNFG_USE_SIMULATED_DB
#if (TMWCNFG_MAX_SIM_DATABASES == TMWDEFS_NO_LIMIT \
  || TMWCNFG_MAX_POINTS == TMWDEFS_NO_LIMIT \
  || TMWCNFG_MAX_STRINGS == TMWDEFS_NO_LIMIT)
#pragma message("If TMWCNFG_ALLOC_ONLY_AT_STARTUP, TMWDEFS_NO_LIMIT for SIM_DATABASE quantities is not allowed")
#endif
#endif /* TMWCNFG_USE_SIMULATED_DB */

#endif /* TMWCNFG_ALLOC_ONLY_AT_STARTUP */

#ifndef TMWCNFG_UNUSED_PARAM
/* Specify this to avoid 'unused parameter' warnings
 * Depending on your compiler, this definition may need to be redefined.
 */
#define TMWCNFG_UNUSED_PARAM(x) (void)x
#endif

#endif /* TMWCNFG_DEFINED */
