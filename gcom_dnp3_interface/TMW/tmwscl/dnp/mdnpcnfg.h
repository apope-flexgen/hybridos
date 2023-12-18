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
 * description:   DNP master configuration definitions
 */
#ifndef MDNPCNFG_DEFINED
#define MDNPCNFG_DEFINED

#include "tmwscl/utils/tmwcnfg.h"

/* All *_NUMALLOC_* defines are used only to limit the number of structures of that type
 * that can be allocated. A value of TMWDEFS_NO_LIMIT (the default) means there is no limit
 * To use static or only at startup memory configuration TMWDEFS_NO_LIMIT is not possible
 * This DO NOT have to depend on the TMWCNFG_xxx defines, but can be changed here to set
 * desired values.
 */

/* Specify the maximum number of Master DNP sessions that can be open */
#define MDNPCNFG_NUMALLOC_SESNS              TMWCNFG_MAX_SESSIONS

/* Specify how many operate requests can be in progress. Generally
 * you can only have one request pending per channel so setting this to
 * the maximum number of configured channel is usually correct.
 */
#define MDNPCNFG_NUMALLOC_OPERCBACKDATAS     TMWCNFG_MAX_CHANNELS

/* Specify how many file transfer requests can be in progress. Currently
 * you can only have one request pending per session so setting this to
 * the maximum number of configured sessions is usually correct.
 */
#define MDNPCNFG_NUMALLOC_FILECBACKDATAS     TMWCNFG_MAX_SESSIONS

/* Specify how many automatic data set exchanges can be in progress. 
 * You can only have one request pending per session so setting this to
 * the maximum number of configured sessions is usually correct.
 */
#define MDNPCNFG_NUMALLOC_DATASET_CTXS       TMWCNFG_MAX_SESSIONS

/* Specify how much time between reading data set prototype from slave and writing
 * master defined data set prototypes and between reading data set descriptor from 
 * slave and writing master defined data set descriptors when doing automatic 
 * exchange of data set information when master or slave restarts. This delay is
 * necessary if async database is processing is implemented on the master to allow
 * the information to be stored in the database before being read by SCL.
 * This value is in milliseconds.
 */
#define MDNPCNFG_DATASET_DELAY               3000

/* Support DNP Secure Authentication Specification Version 2 */
#define MDNPCNFG_SUPPORT_SA_VERSION2         DNPCNFG_SUPPORT_SA_VERSION2

/* Support DNP Secure Authentication Specification Version 5
 * This is an optional component. 
 * DNPCNFG_SUPPORT_AUTHENTICATION must be TMWDEFS_TRUE. 
 * TMWCNFG_SUPPORT_CRYPTO must be TMWDEFS_TRUE
 */
#define MDNPCNFG_SUPPORT_SA_VERSION5         DNPCNFG_SUPPORT_SA_VERSION5

/* Specify the total number of master DNP Secure Authentication Version 5 User structures
 * for all sessions in the entire system that can be allocated when static or 
 * dynamic memory with limited quantities is used.
 */
#define MDNPCNFG_NUMALLOC_AUTH_USERS         (DNPCNFG_AUTH_MAX_NUMBER_USERS * TMWCNFG_MAX_SESSIONS)

/* Specify number of SA Internal State Machine events that can be allocated
 * for all sessions in the entire system when static or 
 * dynamic memory with limited quantities is used.
 * Generally 1 per user with at least 3 if only a single user is allowed
 */
#define MDNPCNFG_NUMALLOC_AUTH_EVENTS        (MDNPCNFG_NUMALLOC_AUTH_USERS + 2)


#if TMWCNFG_USE_SIMULATED_DB
/* Specify the number of master DNP simulated databases in use. The TMW SCL
 * will allocate a new simulated database for each mdnp session.
 * These are not needed once an actual database is implemented.
 */
#define MDNPCNFG_NUMALLOC_SIM_DATABASES      TMWCNFG_MAX_SIM_DATABASES

/* Specify the number of master DNP simulated database Data Set structures that 
 * can be allocated. These are only needed to simulate Data Set points.
 * These are not needed once an actual database is implemented.
 */
#define MDNPCNFG_NUMALLOC_SIM_DATASETS       (TMWCNFG_MAX_SESSIONS*10)

#endif

/* Specify the maximum block size for file transfer, this is just a value
 * for mdnpsesn_initConfig. Values 16 to 18 bytes less than the tx and rx fragment 
 * sizes can be used. 
 */
#define MDNPCNFG_FILE_BLOCK_SIZE 1024

#endif /* MDNPCNFG_DEFINED */

