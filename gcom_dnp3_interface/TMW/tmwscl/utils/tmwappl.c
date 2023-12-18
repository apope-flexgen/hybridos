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

/* file: tmwappl.c
 * description: Application context.
 */
#include "tmwscl/utils/tmwtarg.h"
#include "tmwscl/utils/tmwappl.h"
#include "tmwscl/utils/tmwchnl.h"
#include "tmwscl/utils/tmwmem.h"
 
/* If your environment does not allow initialized data 
 * tmwappl_init() should be called before calling tmwapp_initApplication 
 * or any other functions from the SCL
 */
static TMWTYPES_BOOL tmwappl_initialized = TMWDEFS_FALSE;
static TMWTYPES_ULONG numberApplContexts = 0;

static TMWTYPES_ULONG tmwappl_protocolsInitialized;
static TMWDEFS_RESOURCE_LOCK sclInitlock = TMWDEFS_NULL;

/* function: tmwappl_init */
void TMWDEFS_GLOBAL tmwappl_init(void)
{
  tmwappl_initialized = TMWDEFS_FALSE;
  numberApplContexts = 0;
  sclInitlock = TMWDEFS_NULL;
}

/* function: tmwappl_internalInit */
TMWTYPES_BOOL TMWDEFS_GLOBAL tmwappl_internalInit(void)
{
  if(!tmwappl_initialized)
  {
    tmwappl_initialized = TMWDEFS_TRUE;
    tmwappl_protocolsInitialized = 0;
    return(TMWDEFS_TRUE);
  }
  else
  {
    return(TMWDEFS_FALSE);
  }
}

/* function: tmwappl_initSCL */
TMWTYPES_BOOL TMWDEFS_GLOBAL tmwappl_initSCL(void)
{
  TMWTARG_LOCK_INIT(&sclInitlock);
  if (sclInitlock)
  {
    return(TMWDEFS_TRUE);
  }
  return(TMWDEFS_FALSE);
}

/* function: tmwappl_initApplication */
TMWAPPL * TMWDEFS_GLOBAL tmwappl_initApplication(void)
{
  TMWAPPL *pApplContext;

  /*
   * Do not require calling twmappl_initSCL() prior to calling tmwappl_initApplication so
   * no changes are required to current application, but provide warning to encourage
   * application updates.
   */
#if TMWCNFG_SUPPORT_THREADS
  if (sclInitlock)
    TMWTARG_LOCK_SECTION(&sclInitlock);
  else
    TMWDIAG_ERROR("tmwappl_initSCL() should be called prior to tmwappl_initApplication");
#endif

  if(!tmwappl_initialized)
  {
    /* Initialize memory management module */
    if(!tmwmem_init(TMWDEFS_NULL))
      return(TMWDEFS_NULL);
  }
  if (sclInitlock)
    TMWTARG_UNLOCK_SECTION(&sclInitlock);

  pApplContext = (TMWAPPL *)tmwmem_alloc(TMWMEM_APPL_TYPE);
  if(pApplContext != TMWDEFS_NULL)
  {
    tmwdlist_initialize(&pApplContext->channels);
    TMWTARG_LOCK_INIT(&pApplContext->lock);
    tmwtimer_applInit(pApplContext);
    numberApplContexts++;
  }

  return(pApplContext);
}

/* function: tmwappl_closeApplication */
TMWTYPES_BOOL TMWDEFS_GLOBAL tmwappl_closeApplication(
  TMWAPPL *pApplContext,
  TMWTYPES_BOOL forceClose)
{
  if(tmwdlist_size(&pApplContext->channels) > 0U)
  {
    if(!forceClose)
    {
      return(TMWDEFS_FALSE);
    }
     
#ifdef TODO
    TMWCHNL *pEntry;
    while((pEntry = (TMWCHNL *)tmwdlist_getFirst(&pApplContext->channels)) != TMWDEFS_NULL)
    {
      if(pEntry->pCloseFunc != TMWDEFS_NULL)
        pEntry->pCloseFunc(pEntry);
    }
#endif
  }

  TMWTARG_LOCK_DELETE(&pApplContext->lock);

  tmwmem_free(pApplContext); 
  if(--numberApplContexts == 0)
  {
	/* Tell target layer we are exiting in case it wants to clean up */
	tmwtarg_exit();

    tmwmem_close();
    tmwappl_initialized = TMWDEFS_FALSE;
    if (sclInitlock)
      TMWTARG_LOCK_DELETE(&sclInitlock);
  }
  return(TMWDEFS_TRUE);
}

/* function: tmwappl_getInitialized */
TMWTYPES_BOOL TMWDEFS_GLOBAL tmwappl_getInitialized(
  TMWAPPL_INIT_COMPONENT component)
{
  if(tmwappl_protocolsInitialized & component)
  {
    return(TMWDEFS_TRUE);
  }

  return(TMWDEFS_FALSE);
}

/* function: tmwappl_setInitialized */
void TMWDEFS_GLOBAL tmwappl_setInitialized(
  TMWAPPL_INIT_COMPONENT component)
{
  if(component <= TMWAPPL_INIT_LAST)
  {
    tmwappl_protocolsInitialized |= component;
  }
}

/* function: tmwappl_checkForInput */
void TMWDEFS_GLOBAL tmwappl_checkForInput(
  TMWAPPL *pApplContext)
{
  TMWCHNL *pChannel = TMWDEFS_NULL;

  /* Lock application list of channels */
  TMWTARG_LOCK_SECTION(&pApplContext->lock);

  while((pChannel = (TMWCHNL *)tmwdlist_getAfter(
    &pApplContext->channels, (TMWDLIST_MEMBER *)pChannel)) != TMWDEFS_NULL)
  {
    TMWTYPES_BOOL bProcessInput = TMWDEFS_TRUE;
    int checkForInputLoopCount = 1;

    /* Read from input channel until nothing left if in polled mode */
    if (pChannel->polledMode == TMWDEFS_TRUE)
    {
      /* Lock channel */
      TMWTARG_LOCK_SECTION(&pChannel->lock);

      while(bProcessInput == TMWDEFS_TRUE)
      {
        bProcessInput = pChannel->pPhys->pPhysReceive(pChannel->pPhysContext, 0UL);
        if(++checkForInputLoopCount > TMWCNFG_MAX_APPLRCVS)
        {
          break;
        }
      }

      /* Unlock channel */
      TMWTARG_UNLOCK_SECTION(&pChannel->lock);
    }
  }
  TMWTARG_UNLOCK_SECTION(&pApplContext->lock);
}

/* function: tmwappl_findChannel */
TMWTYPES_BOOL TMWDEFS_GLOBAL tmwappl_findChannel(
  TMWAPPL *pApplContext,
  void *pChannel)
{ 
  TMWCHNL *pChnl = TMWDEFS_NULL;
  TMWTYPES_BOOL found = TMWDEFS_FALSE;

  /* Lock application list of channels */
  TMWTARG_LOCK_SECTION(&pApplContext->lock);

  while((pChnl = (TMWCHNL *)tmwdlist_getAfter(
    &pApplContext->channels, (TMWDLIST_MEMBER *)pChnl)) != TMWDEFS_NULL)
  {
    if((TMWCHNL*)pChannel == pChnl)
    {
      found = TMWDEFS_TRUE;
      break;
    }
  }
  TMWTARG_UNLOCK_SECTION(&pApplContext->lock);
  return found;
}

