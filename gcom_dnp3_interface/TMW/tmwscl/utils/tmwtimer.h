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

/* file: tmwtimer.h
 * description:
 */
#ifndef TMWTIMER_DEFINED
#define TMWTIMER_DEFINED

#include "tmwscl/utils/tmwcnfg.h"
#include "tmwscl/utils/tmwdefs.h"
#include "tmwscl/utils/tmwdlist.h"
#include "tmwscl/utils/tmwappl.h"

/* The following allows timeouts to extend to one month (31 days).  
 * The millisecond timer actually rolls over every 49 days, but
 * 31 is the maximum because of the math calculations in this file.
 */
#define TMWTIMER_MAX_SOFT_TIMER_DELAY  TMWDEFS_DAYS(31UL)

/* forward declaration */
struct TMWChannelStruct;

/* Timer structure */
typedef struct TMWTimer
{
  /* List Member, must be first entry */
  TMWDLIST_MEMBER listMember;

  /* If TRUE this timer is running */
  TMWTYPES_BOOL active;

  /* Time that this timer will expire */
  TMWTYPES_MILLISECONDS timeout;

  /* Channel that owns this timer, for multithreaded locking purposes */
  struct TMWChannelStruct *pChannel;

  /* Function to be called when this timer expires. 
   * If TMWDEFS_NULL, no function will be called back
   */
  TMWTYPES_CALLBACK_FUNC pCallback;

  /* Parameter to be passed to timer expiration callback function */
  void *pCallbackParam;
} TMWTIMER;


#if TMWCNFG_MULTIPLE_TIMER_QS

/* Multiple timer queues exist. This is useful if a multithreaded architecture
 * exists with a single channel per thread and the ability to process timers
 * for a specific channel only when the timer callback function is called is 
 * required. This will require a separate system timer per thread (channel). 
 * In this case the standard polled timer implementation cannot be used since 
 * it only supports a single timer.
 */
typedef struct TMWTimerQueue
{

  /* List of SCL timers that are running for this channel */
  TMWDLIST              list;

  /* No need for a separate lock, the channel lock will be sufficient */

  /* If true the tmwtarg timer is running and will call _timerCallback */
  TMWTYPES_BOOL         timerRunning;
} TMWTIMER_QUEUE;
#endif


#ifdef __cplusplus
extern "C"
{
#endif

#if !TMWCNFG_MULTIPLE_TIMER_QS
  /* function: tmwtimer_initialize 
   * purpose: initialize the timer code. This should be called once by
   *  by customer target application, before any timers are started.
   *  This function is used if the default implementation using a 
   *  single timer queue for all channels is used. This is the case
   *  if the polled timer tmwpltmr.c is used. 
   * arguments:
   *   void
   * returns:
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwtimer_initialize(void);
  /* function: tmwtimer_close
   * purpose: closes the timer. This should be called once by
   * customer target application at shutdown
  */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwtimer_close(void);

  
  /* Internal function called by tmwappl code */
  void TMWDEFS_GLOBAL tmwtimer_applInit(TMWAPPL *pApplContext);

  /* Get the maximum number of timer elements that were ever in the timer queue. 
   * For performance analysis
   */
  TMWDEFS_SCL_API TMWTYPES_UINT TMWDEFS_GLOBAL tmwtimer_getHighWater(void);

#else

  /* Internal function, called by SCL */
  /* function: tmwtimer_initMultiTimer 
   * purpose: Initialize the per channel timer queue for the specified channel.
   *  This will be called by the SCL when a channel is opened if 
   *  TMWCNFG_MULTIPLE_TIMER_QS which creates a separate timer queue per channel 
   *  is set to TMWDEFS_TRUE.
   * arguments:
   *   pChannel - pointer to the channel.
   * returns:
   *   void
   */
  void TMWDEFS_GLOBAL tmwtimer_initMultiTimer(
    struct TMWChannelStruct *pChannel);
    
  /* Internal function called by tmwappl code */
  void TMWDEFS_GLOBAL tmwtimer_applInit(TMWAPPL *pApplContext);

#endif

  /* Internal function, called by SCL */
  /* function: tmwtimer_init 
   * purpose: initialize a timer structure
   *   This will set the timer to not active. This must be
   *   called once on each TMWTIMER structure before it is started.
   *   for the first time.
   * arguments:
   *  pTimer - pointer to caller provided timer structure
   * returns:
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwtimer_init(
    TMWTIMER *pTimer);

  /* Internal function, called by SCL */
  /* function: tmwtimer_isActive 
   * purpose: check to see if this timer is active (running)
   * arguments:
   *  pTimer - pointer to caller provided timer structure
   * returns:
   *  void
   */
  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL tmwtimer_isActive(
    TMWTIMER *pTimer);

  /* Internal function, called by SCL */
  /* function: tmwtimer_start
   * purpose: Start a timer.
   * arguments:
   *  pTimer - pointer to caller provided timer structure
   *  msTimeout - how many milliseconds before timeout
   *   NOTE: maximum of TMWDEFS_DAYS(31UL) (2678400000) (0x9fa52400), 
   *   exceeding this may cause timer to expire immediately.
   *  pChannel - channel that this timer applies to (for multithreading support)
   *  pCallback - callback function to call when this timer expire
   *  pCallbackParam -  parameter for callback function
   * returns:
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwtimer_start(
    TMWTIMER *pTimer,
    TMWTYPES_MILLISECONDS msTimeout,
    struct TMWChannelStruct *pChannel,
    TMWTYPES_CALLBACK_FUNC pCallback,
    void *pCallbackParam);

  /* Internal function, called by SCL */
  /* function: tmwtimer_cancel 
   * purpose: cancel a timer.
   * arguments:
   *  pTimer - pointer to caller provided timer structure
   * returns:
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwtimer_cancel(
    TMWTIMER *pTimer);


#ifdef __cplusplus
}
#endif
#endif /* TMWTIMER_DEFINED */

