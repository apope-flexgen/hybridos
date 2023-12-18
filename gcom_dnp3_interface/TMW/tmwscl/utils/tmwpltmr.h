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

/* file: tmwpltmr.h
 * description: Implement a polled timer. This code is only required
 *  if the platform does not support event timers. To use this code
 *  call the tmwpltmr_startTimer and tmwpltmr_cancelTimer functions
 *  from the tmwtarg_startTimer and tmwtarg_cancelTimer implementation.
 *  Then periodically call the tmwpltmr_checkTimer routine to determine
 *  if the timer has expired.
 */
#ifndef TMWPLTMR_DEFINED
#define TMWPLTMR_DEFINED

#include "tmwscl/utils/tmwcnfg.h"
#include "tmwscl/utils/tmwdefs.h"
#include "tmwscl/utils/tmwtypes.h"
#include "tmwscl/utils/tmwchnl.h"

/* The following allows timeouts to extend to one month (31 days). Because 
 * the millisecond timer rolls over every 48 days, the software timer must 
 * checked (to determine if it has elapsed) at least as often as once every 
 * 17 days. 
 */
#define TMWPLTMR_MAX_SOFT_TIMER_DELAY  TMWDEFS_DAYS(31UL)

#ifdef __cplusplus
extern "C" 
{
#endif

  /* function: tmwpltmr_checkTimer 
   * purpose: Check to see if polled timer has expired. This function should
   *  be called periodically if the tmwtarg timer implementation uses this polled 
   *  timer functionality.
   * arguments:
   *  void
   * returns:
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwpltmr_checkTimer(void);

  /* Internal function, called by default tmwtarg_startTimer() implementation */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwpltmr_startTimer(
    TMWTYPES_MILLISECONDS msTimeout,
    TMWTYPES_CALLBACK_FUNC pCallback, 
    void *pCallbackParam);

  /* Internal function, called by default tmwtarg_cancelTimer() implementation */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwpltmr_cancelTimer(void);

  /* function: tmwpltmr_checkMultiTimer 
   * purpose: Called by application code if MULTI TIMER support but using polled timers.
   *  Check to see if polled timer queue on a channel has expired. This function should
   *  be called periodically if using this functionality.
   * arguments:
   *  pChannel - pointer to channel to check.
   * returns:
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwpltmr_checkMultiTimer(TMWCHNL *pChannel);

#ifdef __cplusplus
}
#endif

#endif /* TMWPLTMR_DEFINED */

