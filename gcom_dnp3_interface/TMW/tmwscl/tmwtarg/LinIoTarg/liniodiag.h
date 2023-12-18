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

/* file: liniodiag.h
 * description: Implementation of diagnostic routines for Linux target
 */
#ifndef liniodiag_DEFINED
#define liniodiag_DEFINED

#include "tmwscl/utils/tmwdefs.h"
#include "tmwscl/utils/tmwcnfg.h"
#include "tmwscl/utils/tmwdiag.h"
#include "tmwscl/utils/tmwtarg.h"

#if !TMWCNFG_SUPPORT_DIAG
#define LINIODIAG_MSG(...)

#define LINIODIAG_ERRORMSG(...)

#else
 
#define LINIODIAG_MSG(...) \
  liniotarg_diagMsg(__VA_ARGS__)

#define LINIODIAG_ERRORMSG(...) \
  liniotarg_diagErrorMsg(__VA_ARGS__)
#endif
  

void TMWDEFS_GLOBAL liniotarg_diagMsg(const char *format, ...);

void TMWDEFS_GLOBAL liniotarg_diagErrorMsg(const char *format, ...);

#endif /* liniotarg_DEFINED */
