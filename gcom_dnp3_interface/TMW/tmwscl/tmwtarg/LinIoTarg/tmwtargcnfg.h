/*****************************************************************************/
/* Triangle MicroWorks, Inc.                         Copyright (c) 2008-2011 */
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

/* set this to TMWDEFS_TRUE to enable TCP support */
#define TMWTARG_SUPPORT_TCP TMWDEFS_TRUE

/* set this to TMWDEFS_TRUE to enable DNP target layer file support */
#define TMWTARG_SUPPORT_DNPFILEIO TMWDEFS_TRUE

/* set this to TMWDEFS_TRUE to enable I101 and 104 target layer file support */
#define TMWTARG_SUPPORT_14FILEIO  TMWDEFS_TRUE

/* set this to TMWDEFS_TRUE to enable UDP support */
/* UDP is required for DNP, otherwise can be set to TMWDEFS_FALSE */
#ifndef TMWTARG_SUPPORT_UDP
#define TMWTARG_SUPPORT_UDP TMWDEFS_TRUE
#endif
/* set this to TMWDEFS_FALSE to remove TLS support */
#ifndef TMWTARG_SUPPORT_TLS
#define TMWTARG_SUPPORT_TLS TMWDEFS_FALSE
#endif

#if TMWTARG_SUPPORT_TLS && !TMWCNFG_USE_OPENSSL
  #error TMWCNFG_USE_OPENSSL Must be defined to support TLS.
#endif

/* set this to TMWDEFS_FALSE to remove POLL support.    */
/* The Linux target layer uses this as its default      */
/* setting. With this disabled, the Linux target layer  */
/* will use select which isn't as efficient and limits  */
/* socket file descriptor numbers to FD_SETSIZE. This   */
/* option is defined here since select is more widely   */
/* supported and can be used as an aid when porting     */
/* this target layer to an OS that only supports select.*/
#define TMWTARG_SUPPORT_POLL TMWDEFS_TRUE

#endif /* TMWTARGCNFG_DEFINED */
