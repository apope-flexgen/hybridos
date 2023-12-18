/*****************************************************************************/
/* Triangle MicroWorks, Inc.                        Copyright (c) 1997-2019  */
/*****************************************************************************/
/*                                                                           */
/* This file is the property of:                                             */
/*                                                                           */
/*                       Triangle MicroWorks, Inc.                           */
/*                      Raleigh, North Carolina USA                          */
/*                       www.TriangleMicroWorks.com                          */
/*                          (919) 870 - 6615                                 */
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

/* file: tmwtargp.c
 * description: tmwtarg "private" implementation of routines defined in 
 *  tmwtargp.h. These functions are used by the sample application, but are 
 *  not required to be implemented for the protocol libraries.
 */
#include "tmwscl/utils/tmwtarg.h"
#include "tmwscl/utils/tmwcnfg.h"
#include "tmwscl/utils/tmwpltmr.h"
 
#if TMWCNFG_SUPPORT_DIAG
extern TMWTARGP_PUT_DIAG_STRING_FUNC pPutDiagStringFunc;
/* function: tmwtargp_registerPutDiagStringFunc */
void TMWDEFS_GLOBAL tmwtargp_registerPutDiagStringFunc(
  TMWTARGP_PUT_DIAG_STRING_FUNC pFunc)
{
  pPutDiagStringFunc = pFunc;
}
#endif /* TMWCNFG_SUPPORT_DIAG */

#if !TMWCNFG_MULTIPLE_TIMER_QS 
extern TMWTARGP_START_TIMER_FUNC pStartTimerFunc;
void TMWDEFS_GLOBAL tmwtargp_registerStartTimerFunc(TMWTARGP_START_TIMER_FUNC pFunc)
{
  /* Put your code here */
  TMWTARG_UNUSED_PARAM(pFunc);
}

extern TMWTARGP_CANCEL_TIMER_FUNC pCancelTimerFunc;
void TMWDEFS_GLOBAL tmwtargp_registerCancelTimerFunc(TMWTARGP_CANCEL_TIMER_FUNC pFunc)
{
  /* Put your code here */
  TMWTARG_UNUSED_PARAM(pFunc);
}
#endif

