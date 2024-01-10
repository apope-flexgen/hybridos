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

/* file: tmwtimer.h */
#include "tmwscl/utils/tmwtarg.h"
#include "tmwscl/utils/tmwtimer.h"
#include "tmwscl/utils/tmwdefs.h"
#include "tmwscl/utils/tmwdlist.h"

#if !TMWCNFG_MULTIPLE_TIMER_QS
/* A single timer queue exists which requires only a single polled 
 * or system timer.
 */
typedef struct TMWTimerQueue
{

  /* List of SCL timers that are running */
  TMWDLIST list;

  TMWTYPES_UINT timerHighWater;

#if TMWCNFG_SUPPORT_THREADS
  /* Lock for this timer queue, for multithread support */
  TMWDEFS_RESOURCE_LOCK lock;
#endif

  TMWAPPL              *pAppl;

  /* If true the tmwtarg timer is running and will call _timerCallback */
  TMWTYPES_BOOL         timerRunning;
} TMWTIMER_QUEUE;

/* structure containing list of timers we are currently managing */
static TMWTIMER_QUEUE _timerPool;

static TMWTYPES_BOOL tmwtimer_initialized = TMWDEFS_FALSE;

/*
 * MULTITHREADING ISSUES with a single timer queue.
 * To support multithreaded applications several design criteria are
 * important. Always lock the channel before locking the timer queue.
 * If this rule is followed there will be no deadlock issues.
 * Lock the timer queue when adding, deleting or traversing the queue.
 * Lock the channel when calling the timer callback function.
 * Do not process expired timers when starting a timer or cancelling a timer,
 *  or else a second channel may end up running in the context of the first 
 *  channel that is starting or canceling the timer, this is complicated and 
 *  could cause deadlock.
 */
#endif

/* Local functions */

/* function: _checkTimer 
 * purpose: Determine if new timer will expire sooner than old timer
 * arguments:
 *  pTimer - pointer to new timer structure. 
 *    NOTE pTimer->timeout cannot have already expired
 *    therefore it must be greater than now although it could have
 *    wrapped back past zero, so it looks less.
 *  timeToCheck - time to compare this new timer to
 *  now - current time
 * returns:
 *  TMWDEFS_TRUE if new timer would expire sooner 
 *  TMWDEFS_FALSE otherwise
 * NOTE: the algorithm in _checkTimerExpired cannot be used to compare
 *  two timers. One of the assumptions in _checkTimerExpired is that the
 *  comparison to the current time must occur at least every 17 days.
 *  If the two times being compared are more than 17 days apart the 
 *  algorithm did not work for comparing two times to see wich will occur
 *  sooner.
 */
static TMWTYPES_BOOL TMWDEFS_LOCAL _checkTimer(
  TMWTIMER *pTimer,
  TMWTYPES_MILLISECONDS timeToCheck,
  TMWTYPES_MILLISECONDS now)
{ 
 
  /* If timeToCheck is in the past, the new timer must be greater than it, since new timer cannot be negative */
  if((timeToCheck - now) > TMWTIMER_MAX_SOFT_TIMER_DELAY)
  {
    return TMWDEFS_FALSE;
  }

  /* Determine which timer expires farthest from now */ 
  if((timeToCheck - now) > (pTimer->timeout - now))
  {
    return(TMWDEFS_TRUE);
  }

  return(TMWDEFS_FALSE);
}

/* function: _checkTimerExpired */
static TMWTYPES_BOOL TMWDEFS_LOCAL _checkTimerExpired(
  TMWTIMER *pTimer,
  TMWTYPES_MILLISECONDS timeToCheck)
{
  /* Check for timeout, watch for rollover */
  TMWTYPES_MILLISECONDS timeRemaining = pTimer->timeout - timeToCheck;

  /* If a break point is set at pCallback in _timerCallback and a single 31 day timer is 
   * added to an empty timer queue, it could look like the timer has timed out immediately.
   * Checking for this larger value avoids that problem, by allowing 1 day at the break point.
   */
  if(timeRemaining > TMWDEFS_DAYS(32UL))
    return(TMWDEFS_TRUE);

  if(timeRemaining == 0)
    return(TMWDEFS_TRUE);

  return(TMWDEFS_FALSE);
}


/* Global Functions */

/* function: tmwtimer_init */
void TMWDEFS_GLOBAL tmwtimer_init(
  TMWTIMER *pTimer)
{
  pTimer->pCallback = TMWDEFS_NULL;
  pTimer->pCallbackParam = TMWDEFS_NULL;
  pTimer->timeout = 0;
  pTimer->pChannel = TMWDEFS_NULL;
  pTimer->active = TMWDEFS_FALSE;
}

/* function: tmwtimer_isActive */
TMWTYPES_BOOL TMWDEFS_GLOBAL tmwtimer_isActive(
  TMWTIMER *pTimer)
{
  return(pTimer->active);
}

#if !TMWCNFG_MULTIPLE_TIMER_QS
/* Default configuration, single timer queue for entire SCL */

/* forward declarations */
static void TMWDEFS_LOCAL _restartSystemTimer(void);

/* function: tmwtimer_initialize() */
void TMWDEFS_GLOBAL tmwtimer_initialize(void)
{ 
  /* Protect against this being called multiple times. */
  if (tmwtimer_initialized)
    return;

  tmwtimer_initialized = TMWDEFS_TRUE;

  /* Lock database queue */
  TMWTARG_LOCK_INIT(&_timerPool.lock);
  TMWTARG_LOCK_SECTION(&_timerPool.lock);

  tmwdlist_initialize(&_timerPool.list);
  _timerPool.timerRunning = TMWDEFS_FALSE;
  _timerPool.timerHighWater = 0;

  /* Unlock database queue */
  TMWTARG_UNLOCK_SECTION(&_timerPool.lock);
}

void TMWDEFS_GLOBAL tmwtimer_applInit(TMWAPPL *pApplContext)
{
  _timerPool.pAppl = pApplContext;
}

void TMWDEFS_GLOBAL tmwtimer_close(void)
{
  TMWTARG_LOCK_DELETE(&_timerPool.lock);
  tmwtimer_initialized = TMWDEFS_FALSE;
}

TMWTYPES_UINT TMWDEFS_GLOBAL tmwtimer_getHighWater(void)
{
  return _timerPool.timerHighWater;
}

static void TMWDEFS_CALLBACK _timerCallback(
  void *pCallbackParam)
{
  TMWTIMER *pLockedTimer;
  TMWTIMER *pTimer;
  TMWTYPES_MILLISECONDS now;
  TMWTARG_UNUSED_PARAM(pCallbackParam);

  /* This prevents channels from being deleted while processing timer callback */
  TMWTARG_LOCK_SECTION(&_timerPool.pAppl->lock);

  /* Lock the timerPool queue to prevent timers from being added or removed */
  TMWTARG_LOCK_SECTION(&_timerPool.lock);
  _timerPool.timerRunning = TMWDEFS_FALSE;

  pTimer = (TMWTIMER *)tmwdlist_getFirst(&_timerPool.list);
  now = tmwtarg_getMSTime();

  /* See if first timer on list has expired */
  while((pTimer != TMWDEFS_NULL) && _checkTimerExpired(pTimer, now))
  {
#if TMWCNFG_SUPPORT_THREADS
    TMWDEFS_RESOURCE_LOCK *pLock = &pTimer->pChannel->lock;
#endif

    /* To prevent deadlock always lock channel before locking timer queue. 
     * Must unlock the timerPool first
     */
    TMWTARG_UNLOCK_SECTION(&_timerPool.lock);
    TMWTARG_LOCK_SECTION(pLock);
    TMWTARG_LOCK_SECTION(&_timerPool.lock);

    /* Now that the timerPool queue is locked, get pointer to first timer on list 
     * again, in case the first timer was deleted while we were not locked
     */
    pLockedTimer = (TMWTIMER *)tmwdlist_getFirst(&_timerPool.list);

    /* Process this timer if it has already timed out */
    if((pLockedTimer == pTimer) && _checkTimerExpired(pTimer, now))
    {
      /* Cancel timer and delete from list */
      pTimer->active = TMWDEFS_FALSE;
      tmwdlist_removeEntry(&_timerPool.list, (TMWDLIST_MEMBER *)pTimer);

      /* Unlock this, so that others can access timer queue */
      TMWTARG_UNLOCK_SECTION(&_timerPool.lock);

      /* Call timer callback */
      if(pTimer->pCallback != TMWDEFS_NULL)
      {
        pTimer->pCallback(pTimer->pCallbackParam); 
      }

      /* Relock the timerPool queue */
      TMWTARG_LOCK_SECTION(&_timerPool.lock);
    }

    /* Release the channel lock */
    TMWTARG_UNLOCK_SECTION(pLock);

    /* Get next timer */
    pTimer = (TMWTIMER *)tmwdlist_getFirst(&_timerPool.list);
  }

  TMWTARG_UNLOCK_SECTION(&_timerPool.lock);

  TMWTARG_UNLOCK_SECTION(&_timerPool.pAppl->lock);
  _restartSystemTimer();
}

/* function: _restartSystemTimer 
 * purpose: Determine if SCL needs to restart the single system
 *  timer.
 * arguments:
 *  void
 * returns:
 *  void
 */
static void TMWDEFS_LOCAL _restartSystemTimer(void)
{
  TMWTIMER *pTimer;

  TMWTARG_LOCK_SECTION(&_timerPool.lock);
  pTimer = (TMWTIMER *)tmwdlist_getFirst(&_timerPool.list);
  if(pTimer != TMWDEFS_NULL)
  {
    TMWTYPES_MILLISECONDS timeoutValue = pTimer->timeout - tmwtarg_getMSTime();

    /* Make sure that time has not elapsed and the calculation would be negative */
    if(timeoutValue > TMWTIMER_MAX_SOFT_TIMER_DELAY)
    {
      timeoutValue = 0;
    }

    if(_timerPool.timerRunning)
    {
      tmwtarg_cancelTimer();
      tmwtarg_startTimer(timeoutValue, _timerCallback, pTimer);
    }
    else
    {
      /* Start the system timer */
      _timerPool.timerRunning = TMWDEFS_TRUE;
      tmwtarg_startTimer(timeoutValue, _timerCallback, pTimer);
    }
  }

  TMWTARG_UNLOCK_SECTION(&_timerPool.lock);
}

/* function: tmwtimer_start */
void TMWDEFS_GLOBAL tmwtimer_start(
  TMWTIMER *pTimer,
  TMWTYPES_MILLISECONDS msTimeout,
  struct TMWChannelStruct *pChannel,
  TMWTYPES_CALLBACK_FUNC pCallback,
  void *pCallbackParam)
{
  TMWTIMER *aTimer;
  TMWTYPES_MILLISECONDS now;
  TMWTYPES_BOOL restartTimer = TMWDEFS_FALSE;

  /* Channel is required for locking */
  if(pChannel == TMWDEFS_NULL)
    return;

  /* Calling start restarts a timer */
  tmwtimer_cancel(pTimer);
 
  /* If this timer asks for zero milliseconds call it back now */
  if(msTimeout == 0)
  {
    /* Call timer callback */
    if(pCallback != TMWDEFS_NULL)
    {
      pCallback(pCallbackParam);
    }
    return;
  }

  /* Limit max value to 31 days */
  if(msTimeout > TMWTIMER_MAX_SOFT_TIMER_DELAY)
  {
    msTimeout = TMWTIMER_MAX_SOFT_TIMER_DELAY;
  }

  /* Initialize timer */
  pTimer->active = TMWDEFS_TRUE;
  pTimer->pChannel = pChannel;
  pTimer->pCallback = pCallback;
  pTimer->pCallbackParam = pCallbackParam;
  now = tmwtarg_getMSTime();
  pTimer->timeout = now + msTimeout;

  /* Lock timer queue */
  TMWTARG_LOCK_SECTION(&_timerPool.lock);

  /* Sort the timer list based on timeout */
  aTimer = (TMWTIMER *)tmwdlist_getFirst(&_timerPool.list);
  if(aTimer == TMWDEFS_NULL)
  { 
    /* List is empty, add timer */
    tmwdlist_addEntry(&_timerPool.list, (TMWDLIST_MEMBER *)pTimer);
    restartTimer = TMWDEFS_TRUE;
  }
  else if(_checkTimer(pTimer, aTimer->timeout, now))
  {
    tmwdlist_insertEntryBefore(&_timerPool.list, 
      (TMWDLIST_MEMBER *)aTimer, (TMWDLIST_MEMBER *)pTimer);

    restartTimer = TMWDEFS_TRUE;
  }
  else
  {
    /* Figure out where this timer belongs and put it in the list 
     * Do not need to restart the system timer, since this is not 
     * first timer on list
     * aTimer is pointing to first timer on list.
     */    
    while((aTimer = (TMWTIMER *)tmwdlist_getNext((TMWDLIST_MEMBER *)aTimer)) != TMWDEFS_NULL)
    {
      if(_checkTimer(pTimer, aTimer->timeout, now))
      { 
        /* Put timer before this one */
        tmwdlist_insertEntryBefore(&_timerPool.list,
         (TMWDLIST_MEMBER *)aTimer, (TMWDLIST_MEMBER *)pTimer);
        break;
      }
    }

    if(aTimer == TMWDEFS_NULL)
    {
      /* Put timer at end of list */
      tmwdlist_addEntry(&_timerPool.list, (TMWDLIST_MEMBER *)pTimer);
    }

    if(_timerPool.list.size >_timerPool.timerHighWater)
    {
     _timerPool.timerHighWater = _timerPool.list.size;
    }
  }

  /* Unlock timer queue */
  TMWTARG_UNLOCK_SECTION(&_timerPool.lock);

  if(restartTimer)
  {
    _restartSystemTimer();
  }
}

/* function: tmwtimer_cancel */
void TMWDEFS_GLOBAL tmwtimer_cancel(
  TMWTIMER *pTimer)
{
  /* Lock timer queue */
  TMWTARG_LOCK_SECTION(&_timerPool.lock);

  if(pTimer->active)
  {
    /* Set it to inactive */
    pTimer->active = TMWDEFS_FALSE;

    tmwdlist_removeEntry(&_timerPool.list, (TMWDLIST_MEMBER *)pTimer);

    if(tmwdlist_size(&_timerPool.list) == 0)
    {
      tmwtarg_cancelTimer();
      _timerPool.timerRunning = TMWDEFS_FALSE;
    }
  }

  /* Unlock timer queue */
  TMWTARG_UNLOCK_SECTION(&_timerPool.lock);
}

#else
/* TMWCNFG_MULTIPLE_TIMER_QS is TMWDEFS_TRUE 
 * indicating a separate timer queue for each channel will be supported.
 */

/* forward declarations */
static void TMWDEFS_LOCAL _restartSystemTimer(
  TMWCHNL *pChannel);

void TMWDEFS_GLOBAL tmwtimer_applInit(TMWAPPL *pApplContext)
{ 
  TMWTARG_UNUSED_PARAM(pApplContext);
}

/* function: _timerCallback 
 * purpose: Callback function called when multiple timer
 *  queues (1 per channel) are supported
 * arguments:
 *  pCallbackParam - pointer to channel.
 *  void
 * returns:
 *  void
 */
static void TMWDEFS_CALLBACK _timerCallback(
  void *pCallbackParam)
{
  TMWTIMER *pTimer;
  TMWTYPES_MILLISECONDS now;
  TMWCHNL *pChannel = (TMWCHNL *)pCallbackParam;

  TMWTARG_LOCK_SECTION(&pChannel->lock);
  pChannel->timerQueue.timerRunning = TMWDEFS_FALSE;

  pTimer = (TMWTIMER *)tmwdlist_getFirst(&pChannel->timerQueue.list);
  now = tmwtarg_getMSTime();

  /* See if first timer on list has expired */
  while((pTimer != TMWDEFS_NULL) && _checkTimerExpired(pTimer, now))
  {
    /* Cancel timer and delete from list */
    pTimer->active = TMWDEFS_FALSE;
    tmwdlist_removeEntry(&pChannel->timerQueue.list, (TMWDLIST_MEMBER *)pTimer);

    /* Call timer callback */
    if(pTimer->pCallback != TMWDEFS_NULL)
    {
      pTimer->pCallback(pTimer->pCallbackParam);
    }

    /* Get next timer */
    pTimer = (TMWTIMER *)tmwdlist_getFirst(&pChannel->timerQueue.list);
  }

  _restartSystemTimer(pChannel);

  TMWTARG_UNLOCK_SECTION(&pChannel->lock);
}

/* function: _restartSystemTimer 
 * purpose: Determine if SCL needs to restart a system timer for
 *  the timer queue contained by this channel.
 * arguments:
 *  pChannel - pointer to channel
 * returns:
 *  void
 */
static void TMWDEFS_LOCAL _restartSystemTimer(
  TMWCHNL *pChannel)
{
  TMWTIMER *pTimer;

  pTimer = (TMWTIMER *)tmwdlist_getFirst(&pChannel->timerQueue.list);
  if(pTimer != TMWDEFS_NULL)
  {
    TMWTYPES_MILLISECONDS timeoutValue = pTimer->timeout - tmwtarg_getMSTime();

    /* Make sure that time has not already elapsed and the calculation is negative */
    if((timeoutValue > TMWTIMER_MAX_SOFT_TIMER_DELAY) || (timeoutValue == 0))
    {
      /* Since a timeout value of zero, will cancel the timer, set it to a minimal value of 1 */
      timeoutValue = 1;
    }

    /* Reset the MultiTimer with the new timeout value */
    pChannel->timerQueue.timerRunning = TMWDEFS_TRUE;
    if (tmwtarg_setMultiTimer(pChannel, timeoutValue))
    {
      TMWDIAG_CHNL_ERROR(pChannel, "tmwtarg_setMultiTimer returned failure")
    }
  }
}

/* function: tmwtimer_initMultiTimer() */
void TMWDEFS_GLOBAL tmwtimer_initMultiTimer(
  struct TMWChannelStruct *pChannel)
{
  /* If multiple timer support, initialize per channel timer queue */
  tmwdlist_initialize(&pChannel->timerQueue.list);
  pChannel->pMultiTimerCallback = _timerCallback;
  pChannel->timerQueue.timerRunning = TMWDEFS_FALSE;
  if (tmwtarg_initMultiTimer(pChannel))
  {
    TMWDIAG_CHNL_ERROR(pChannel, "tmwtarg_initMultiTimer returned failure")
  }
}

/* function: tmwtimer_start */
void TMWDEFS_GLOBAL tmwtimer_start(
  TMWTIMER *pTimer,
  TMWTYPES_MILLISECONDS msTimeout,
  struct TMWChannelStruct *pChannel,
  TMWTYPES_CALLBACK_FUNC pCallback,
  void *pCallbackParam)
{
  TMWTIMER *aTimer;
  TMWTYPES_MILLISECONDS now;
  TMWTYPES_BOOL restartTimer = TMWDEFS_FALSE;

  /* Channel is required for locking */
  if(pChannel == TMWDEFS_NULL)
    return;

  /* Calling start restarts a timer */
  tmwtimer_cancel(pTimer);
 
  /* If this timer asks for zero milliseconds call it back now */
  if(msTimeout == 0)
  {
    /* Call timer callback */
    if(pCallback != TMWDEFS_NULL)
    {
      pCallback(pCallbackParam);
    }
    return;
  }

  /* Limit max value to 31 days */
  if(msTimeout > TMWTIMER_MAX_SOFT_TIMER_DELAY)
  {
    msTimeout = TMWTIMER_MAX_SOFT_TIMER_DELAY;
  }

  /* Initialize timer */
  pTimer->active = TMWDEFS_TRUE;
  pTimer->pChannel = pChannel;
  pTimer->pCallback = pCallback;
  pTimer->pCallbackParam = pCallbackParam;
  now = tmwtarg_getMSTime();
  pTimer->timeout = now + msTimeout;

  /* Sort the timer list based on timeout */
  aTimer = (TMWTIMER *)tmwdlist_getFirst(&pChannel->timerQueue.list);
  if(aTimer == TMWDEFS_NULL)
  { 
    /* List is empty, add timer */
    tmwdlist_addEntry(&pChannel->timerQueue.list, (TMWDLIST_MEMBER *)pTimer);
    restartTimer = TMWDEFS_TRUE;
  }
  else if(_checkTimer(pTimer, aTimer->timeout, now))
  {
    tmwdlist_insertEntryBefore(&pChannel->timerQueue.list, 
      (TMWDLIST_MEMBER *)aTimer, (TMWDLIST_MEMBER *)pTimer);

    restartTimer = TMWDEFS_TRUE;
  }
  else
  {
    TMWTIMER *prevTimer;
    /* Figure out where this timer belongs and put it in the list 
     * Do not need to restart the system timer, since this is not 
     * first timer on list
     * aTimer is pointing to first timer on list.
     */
    prevTimer = aTimer;
    while ((aTimer = (TMWTIMER *)tmwdlist_getNext((TMWDLIST_MEMBER *)aTimer)) != TMWDEFS_NULL)
    {
      if(_checkTimer(pTimer, aTimer->timeout, now))
      { 
        /* Put timer before this one (after prevTimer) */
        tmwdlist_insertEntryAfter(&pChannel->timerQueue.list,
          (TMWDLIST_MEMBER *)prevTimer, (TMWDLIST_MEMBER *)pTimer);
        break;
      }
      prevTimer = aTimer;
    }

    if(aTimer == TMWDEFS_NULL)
    {
      /* Put timer at end of list */
      tmwdlist_addEntry(&pChannel->timerQueue.list, (TMWDLIST_MEMBER *)pTimer);
    } 
  }

  if(restartTimer)
  {
    _restartSystemTimer(pChannel);
  }
}

/* function: tmwtimer_cancel */
void TMWDEFS_GLOBAL tmwtimer_cancel(
  TMWTIMER *pTimer)
{
  if(pTimer->active)
  {
    TMWTIMER_QUEUE *pTimerQueue = &pTimer->pChannel->timerQueue;
    /* Set it to inactive */
    pTimer->active = TMWDEFS_FALSE;
    tmwdlist_removeEntry(&pTimerQueue->list, (TMWDLIST_MEMBER *)pTimer);

    if(tmwdlist_size(&pTimerQueue->list) == 0) 
    {
      pTimer->pChannel->timerQueue.timerRunning = TMWDEFS_FALSE;
      tmwtarg_setMultiTimer(pTimer->pChannel, 0);
    }
  }
}

#endif
