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
 */
#include "tmwscl/utils/tmwtarg.h"
#include "tmwscl/utils/tmwpltmr.h"

#if !TMWCNFG_MULTIPLE_TIMER_QS
/* If a separate timer queue per channel (per thread)
 * is required, this polled timer implementation can 
 * not be used.
 */

/* Define a data structure to hold timer info */
typedef struct TMWPollTimer
{
  TMWTYPES_BOOL isActive;
  TMWTYPES_MILLISECONDS timeout;
  TMWTYPES_CALLBACK_FUNC pCallback;
  void *pCallbackParam;
} TMWPLTMR;

/* Declare a single instance of a timer */
static TMWPLTMR theTimer;

/* function: tmwpltmr_startTimer */
void TMWDEFS_GLOBAL tmwpltmr_startTimer(
  TMWTYPES_MILLISECONDS msTimeout,
  TMWTYPES_CALLBACK_FUNC pCallback, 
  void *pCallbackParam)
{
  theTimer.pCallback = pCallback;
  theTimer.pCallbackParam = pCallbackParam;
  theTimer.timeout = tmwtarg_getMSTime() + msTimeout;
  theTimer.isActive = TMWDEFS_TRUE;
}

/* function: tmwpltmr_cancelTimer */
void TMWDEFS_GLOBAL tmwpltmr_cancelTimer(void)
{
  theTimer.isActive = TMWDEFS_FALSE;
}

/* function: tmwpltmr_checkTimer */
void TMWDEFS_GLOBAL tmwpltmr_checkTimer(void)
{
  if(theTimer.isActive)
  {
    TMWTYPES_ULONG remainingTime = theTimer.timeout - tmwtarg_getMSTime();

    if(remainingTime > TMWPLTMR_MAX_SOFT_TIMER_DELAY)
    {
      remainingTime = 0UL;
    }

    if(remainingTime == 0UL)
    {
      theTimer.isActive = TMWDEFS_FALSE;
      theTimer.pCallback(theTimer.pCallbackParam);
    }
  }
}

#else

/* function: tmwpltmr_checkMuiltiTimer */
void TMWDEFS_GLOBAL tmwpltmr_checkMultiTimer(TMWCHNL *pChannel)
{
  if (pChannel->pMultiTimerCallback)
  {
    pChannel->pMultiTimerCallback(pChannel);
  }
}
#endif
