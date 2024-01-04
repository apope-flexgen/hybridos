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

/* file: dnpcnfg.h
 * description:   DNP configuration definitions
 */
#ifndef DNPCNFG_DEFINED
#define DNPCNFG_DEFINED

#include "tmwscl/utils/tmwcnfg.h"

/* All *_NUMALLOC_* defines are used only to limit the number of structures 
 * of that type that can be allocated. A value of TMWDEFS_NO_LIMIT (the default) 
 * means there is no limit. To use static or only at startup memory configuration
 * TMWDEFS_NO_LIMIT is not possible. These DO NOT have to depend on the 
 * TMWCNFG_xxx defines, but can be changed here to set desired values.
 */
 
/* Specify the maximum number of DNP channels 
 * The TMW SCL needs one for each DNP serial or TCP connection
 */
#define DNPCNFG_NUMALLOC_CHANNELS             TMWCNFG_MAX_CHANNELS

/* Specify the number of link layer frames buffers that can be allocated. 
 * The TMW SCL needs one per DNP channel if Diagnostics are compiled in.
 */
#define DNPCNFG_NUMALLOC_LINK_FRAMES          TMWCNFG_MAX_CHANNELS

/* Specify the number of link layer contexts that can be allocated. 
 * The TMW SCL needs one per DNP channel.
 */
#define DNPCNFG_NUMALLOC_LINK_CONTEXTS        TMWCNFG_MAX_CHANNELS

/* Specify the number of transport layer contexts that can be allocated. 
 * The TMW SCL needs one per DNP channel.
 */
#define DNPCNFG_NUMALLOC_TPRT_CONTEXTS        TMWCNFG_MAX_CHANNELS

/* Specify the number of link layer session contexts that can be allocated. 
 * The TMW SCL needs one per master or slave session.
 */
#define DNPCNFG_NUMALLOC_LINK_SESSIONS        TMWCNFG_MAX_SESSIONS

/* Specify the number of transport sessions that can be allocated. 
 * The TMW SCL needs one per master or slave session.
 */
#define DNPCNFG_NUMALLOC_TPRT_SESSIONS        TMWCNFG_MAX_SESSIONS

/* Specify the number of transmit data structures that can be allocated. 
 * This is dependent on how may application layer fragments will be buffered 
 * per session. etc.
 *  recommend at least 1 per session for slave
 *   (2 if Identical Unsolicited Response Retries is supported)
 *  recommend 2 or more per session for master
 *   requires more if multiple requests are to be queued on master.
 *   if requests are sent automatically based on IIN bits, 1 is needed
 *   for each simultaneous request.
 */
#define DNPCNFG_NUMALLOC_CHNL_TX_DATAS        TMWCNFG_MAX_APPL_MSGS

/* Define the maximum number of bytes transmitted or received in a 
 * frame. This parameter specifies the actual number of bytes on the
 * wire including all DNP3 header and control bytes. The absolute
 * maximum value for this parameter is 292 bytes which includes 249
 * bytes of application data, 1 byte for the DNP3 transport header,
 * 8 bytes for the DNP3 link header, and 34 bytes of CRC data. A
 * theoretical minimum would be 14 which would only include 1 byte
 * of application data. A realistic minimum would be 28 which would
 * include 15 bytes of application data.
 * 
 * If dynamic memory is supported this parameter is only used as a 
 * default value for the appropriate transport and link layer config-
 * uration parameters. The actual value can be changed by the user to 
 * a value larger or smaller than this value. If dynamic memory alloca-
 * tion is not supported this parameter is used to statically allocate 
 * buffers and as the default for the configuration parameters. In this
 * case the user can change the configured values to smaller values but 
 * not larger values.
 */
#define DNPCNFG_MAX_TX_FRAME_LENGTH 292
#define DNPCNFG_MAX_RX_FRAME_LENGTH 292

/* Define the maximum number of application layer data bytes in a 
 * transport layer fragment. The DNP3 specification recommends a value
 * of 2048 and requires that devices that support a larger fragment
 * size be able to configure it down to no more than 2048 bytes. 
 *
 * Typically slave devices can configure the received fragment size
 * much smaller than 2048 since they will only receive requests and
 * similarly the master can configure a transmit fragment size to
 * a smaller value since it will typically only transmit requests.
 *
 * If dynamic memory is supported this parameter is only used as a 
 * default value for the appropriate transport and link layer config-
 * uration parameters. The actual value can be changed by the user to 
 * a value larger or smaller than this value. If dynamic memory alloca-
 * tion is not supported this parameter is used to statically allocate 
 * buffers and as the default for the configuration parameters. In this
 * case the user can change the configured values to smaller values but 
 * not larger values.
 */
#ifndef DNPCNFG_MAX_TX_FRAGMENT_LENGTH
 /* Maximum allowed value of 65535 */
#define DNPCNFG_MAX_TX_FRAGMENT_LENGTH 2048
#endif
#ifndef DNPCNFG_MAX_RX_FRAGMENT_LENGTH
/* Maximum allowed value of 65535 */
#define DNPCNFG_MAX_RX_FRAGMENT_LENGTH 2048
#endif

/* Define maximum number of hex bytes to display in diagnostics output */
#define DNPCNFG_MAX_DIAG_HEX_BYTES 4000

/* Define maximum number of bytes in a filename 
 * This is used for file transfer if Object 70 is supported
 */
#define DNPCNFG_MAX_FILENAME 256

/* Define maximum number of files that can be opened simultaniously 
 * This is used for file transfer if Object 70 is supported
 */
#define DNPCNFG_MAX_FILES_OPEN 5

/* Define maximum number of bytes in an extended string 
 * This is used if Object 114 is supported. Object group 114 supports string
 * sizes between 0 and 65535. Since the maximum string length that can be 
 * transmitted is limited by the fragment size, this parameter can be used
 * to save significant memory resources.
 */
#ifndef DNPCNFG_MAX_EXT_STRING_LENGTH
#define DNPCNFG_MAX_EXT_STRING_LENGTH 2048
#endif

/* Define maximum number of bytes displayed by the diagnostic functions for string 
 * based objects. This includes object groups 110, 112, & 114. Setting this value 
 * above 255 ensures that only the extended strings will be truncated. 
 */
#define DNPCNFG_MAX_DIAG_STRING_LENGTH 255

/* Indicates whether or not functions used by Datasets and Device Attributes
 * should be compiled into the Master and Slave SCLs. Setting to TMWDEFS_FALSE
 * will save some code space.
 */
#define DNPCNFG_SUPPORT_DATASETS_ATTRS   TMWDEFS_TRUE

/* When the SCL parses a Data Set object 87 or 88 it will use a corresponding
 * Data Set descriptor and any contained prototypes to convert the data to its
 * proper type. The data types will be read onto the stack in the SCL. This 
 * define controls how much memory is set aside to contain these. It requires
 * 1 byte for each of these
 */
#ifndef DNPCNFG_MAX_DATASET_DESCR_ELEMS
#define DNPCNFG_MAX_DATASET_DESCR_ELEMS 32
#endif

/* Define maximum number of control values in a dataset control group
 * (number of values per control status byte). This is used to determine how much
 * room to allow on the stack for accumulating values before sending control to
 * database. Each one is sizeof(DNPDATA_DATASET_CTRL_VALUE) which depends on other
 * defines, but by default is about 32 bytes.
 */
#ifndef DNPCNFG_MAX_DATASET_CTRLS
#define DNPCNFG_MAX_DATASET_CTRLS 8
#endif

/* This will determine how large the string array is in the DNPDATA_DATASET_VALUE
 * structure. Since value is a union, making this larger than TMWDTIME (16 bytes)
 * makes the union larger. If strings are too large to fit in the array they can
 * be passed using a pointer, but then the strings must be copied somewhere.
 */
#define DNPCNFG_MAX_STRING_ARRAY  16

/* Set this parameter to TMWDEFS_TRUE to include support for 
 * configuring using binary version of device profile current values
 * Setting this to TMWDEFS_FALSE will save code space if
 * this feature is not being supported
 */
#ifndef DNPCNFG_SUPPORT_BINCONFIG
#define DNPCNFG_SUPPORT_BINCONFIG        TMWDEFS_FALSE
#endif

/* This will include code required for DNP3 Secure Authentication
 * Setting this to TMWDEFS_FALSE will save code and data space if secure
 * authentication is not being supported
 */
#ifndef DNPCNFG_SUPPORT_AUTHENTICATION
#define DNPCNFG_SUPPORT_AUTHENTICATION   TMWDEFS_FALSE
#endif

/* Specify maximum number of secure authentication user numbers supported 
 * per SAv2 session. 
 * SA Version 2 allocates an array of this many users per M/SDNP Session so 
 * this is a hard limit per session.
 * The defines for SAv2 and SAv5 are allowed to be different (if both versions
 * are supported).
 */
#define DNPCNFG_AUTHV2_MAX_NUMBER_USERS  10 

/* Specify maximum number of secure authentication user numbers supported
 * per SAv5 session.
 * Since SA Version 5 uses a linked list of allocated user structures this 
 * is not actually a hard limit per session. The number of users is limited by 
 * memory allocation, including whether static or limited size memory pools
 * are configured. This define IS by default used to limit the number of  
 * SA State Machine events MDNPAUTH_MAX_EVENTS per MDNP session, generally 
 * 1 per user, with at least 3 for a single user.
 * This define is also used to set the default values for the 
 * M/SDNPCNFG_NUMALLOC_AUTH_USERS for which only apply if limited size memory
 * pools are configured. 
 * The defines for SAv2 and SAv5 are allowed to be different (if both versions 
 * are supported).
 */
#define DNPCNFG_AUTH_MAX_NUMBER_USERS    64 

/* Support DNP Secure Authentication Specification Version 2 
 * DNPCNFG_SUPPORT_AUTHENTICATION must be TMWDEFS_TRUE
*/
#ifndef DNPCNFG_SUPPORT_SA_VERSION2
#define DNPCNFG_SUPPORT_SA_VERSION2      TMWDEFS_FALSE
#endif

/* Support DNP Secure Authentication Specification Version 5
 * This is an optional component of the Source Code Library
 * DNPCNFG_SUPPORT_AUTHENTICATION must be TMWDEFS_TRUE. 
 * TMWCNFG_SUPPORT_CRYPTO must be TMWDEFS_TRUE
*/
#ifndef DNPCNFG_SUPPORT_SA_VERSION5
#define DNPCNFG_SUPPORT_SA_VERSION5      TMWDEFS_FALSE
#endif

/* Secure Authentication Version 5 configuration */

/* This will include code for DNP3 Secure Authentication Remote changing of 
 * User Update Keys over DNP. This optional feature was added in Version 5
 * of the DNP SA Specification 
 */
#ifndef DNPCNFG_SUPPORT_AUTHKEYUPDATE
#define DNPCNFG_SUPPORT_AUTHKEYUPDATE    TMWDEFS_FALSE
#endif

/* This will include code for optional Asymmetric Key Change Methods.
 * If Authentication Remote Key Update is supported, support for
 * Symmetric Key Change Methods is required. 
 */
#define DNPCNFG_SUPPORT_AUTHKEYUPDASYM   TMWCNFG_SUPPORT_CRYPTO_ASYM

/* Secure Authentication maximum name lengths */
#define DNPCNFG_AUTH_MAX_USERNAME_LENGTH 256
#define DNPCNFG_AUTH_MAX_OSNAME_LENGTH   256

/* Allow master to send requests to multiple sessions on a single channel  
 * without waiting for responses from other sessions on that channel. 
 * Only a single outstanding request per session (association) as required 
 * by the DNP Specification.
 */
#define DNPCNFG_MULTI_SESSION_REQUESTS   TMWDEFS_FALSE

/* The following defines determine whether particular statistics are compiled in  
 * All of these default to TMWDEFS_TRUE if statistics are compiled in
 * Change individual defines to TMWDEFS_FALSE to remove individual statistics 
 */

/* Session Statistics */
/* support TMWSESN_STAT_REQUEST_TIMEOUT                                 */
#define DNPCNFG_SUPPORT_SSTAT_TIMEOUT         TMWCNFG_SUPPORT_STATS

/* support TMWSESN_STAT_REQUEST_FAILED                                  */
#define DNPCNFG_SUPPORT_SSTAT_FAILED          TMWCNFG_SUPPORT_STATS

/* support TMWSESN_STAT_ASDU_SENT and TMWSESN_STAT_ASDU_RECEIVED        */
#define DNPCNFG_SUPPORT_SSTAT_ASDUS           TMWCNFG_SUPPORT_STATS

/* support TMWSESN_STAT_DNPEVENT_SENT and TMWSESN_STAT_DNPEVENT_CONFIRM */
#define DNPCNFG_SUPPORT_SSTAT_EVENT           TMWCNFG_SUPPORT_STATS

/* support TMWSESN_STAT_DNPUNSOL_TIMER_START                            */
#define DNPCNFG_SUPPORT_SSTAT_UNSOL_TIMER     TMWCNFG_SUPPORT_STATS

/* support TMWSESN_STAT_EVENT_OVERFLOW                                  */
#define DNPCNFG_SUPPORT_SSTAT_OVERFLOW        TMWCNFG_SUPPORT_STATS

/* support TMWSESN_STAT_AUTHxxx                                         */
#define DNPCNFG_SUPPORT_SSTAT_AUTH            TMWCNFG_SUPPORT_STATS 

/* Channel Statistics */
/* support TMWCHNL_STAT_ERROR                                           */
#define DNPCNFG_SUPPORT_CSTAT_ERRORS          TMWCNFG_SUPPORT_STATS

/* support TMWCHNL_STAT_FRAME_SENT and TMWCHNL_STAT_FRAME_RECEIVED      */
#define DNPCNFG_SUPPORT_CSTAT_FRAMES          TMWCNFG_SUPPORT_STATS
 
/* support TMWCHNL_STAT_FRAGMENT_SENT and TMWCHNL_STAT_FRAGMENT_RECEIVED*/
#define DNPCNFG_SUPPORT_CSTAT_FRAGS           TMWCNFG_SUPPORT_STATS


#if DNPCNFG_SUPPORT_AUTHENTICATION && !DNPCNFG_SUPPORT_SA_VERSION2 && !DNPCNFG_SUPPORT_SA_VERSION5
#pragma message("If DNPCNFG_SUPPORT_AUTHENTICATION is supported either DNPCNFG_SUPPORT_SA_VERSION2 or VERSION5 must be selected") 
#endif

#endif /* DNPCNFG_DEFINED */

