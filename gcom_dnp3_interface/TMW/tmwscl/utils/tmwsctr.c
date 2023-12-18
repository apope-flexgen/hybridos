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

/* file: tmwsctr.c
 * description: Definition of a generic sector
 */
#include "tmwscl/utils/tmwtarg.h"
#include "tmwscl/utils/tmwsctr.h"

/* function: tmwsctr_openSector */
TMWTYPES_BOOL TMWDEFS_GLOBAL tmwsctr_openSector(
  TMWSCTR *pSector, 
  TMWSESN *pSession,
  TMWSCTR_STAT_CALLBACK pCallback,
  void *pCallbackParam)
{
  pSector->pUserData = TMWDEFS_NULL;
  pSector->pSession = pSession;
  tmwsctr_setStatCallback(pSector, pCallback, pCallbackParam);
  return(TMWDEFS_TRUE);
}

/* function: tmwsctr_setStatCallback */
void TMWDEFS_GLOBAL tmwsctr_setStatCallback(
  TMWSCTR *pSector,
  TMWSCTR_STAT_CALLBACK pCallback,
  void *pCallbackParam)
{
  pSector->pStatCallbackFunc = pCallback;
  pSector->pStatCallbackParam = pCallbackParam;
}

#if TMWCNFG_SUPPORT_STATS
/* function: tmwsctr_callStatCallback */
void TMWDEFS_GLOBAL tmwsctr_callStatCallback(
  TMWSCTR *pSector,
  TMWSCTR_STAT_EVENT eventType,
  void *pEventData)
{
  if(pSector->pStatCallbackFunc != TMWDEFS_NULL)
  {
    pSector->pStatCallbackFunc(
      pSector->pStatCallbackParam, eventType, pEventData);
  }
}
#endif

/* function: tmwsctr_closeSector */
void TMWDEFS_GLOBAL tmwsctr_closeSector(
  TMWSCTR *pSector)
{
  pSector->pUserData = TMWDEFS_NULL;
  TMWTARG_UNUSED_PARAM(pSector);
}

#if TMWCNFG_SUPPORT_DIAG
/* function: tmwsctr_getSectorName */
const TMWTYPES_CHAR * TMWDEFS_GLOBAL tmwsctr_getSectorName(
  TMWSCTR *pSector)
{
  if(pSector != TMWDEFS_NULL)
  {
    return(tmwtarg_getSectorName(pSector));
  }
  return("Name Not Initialized");
}

/* function: tmwsctr_getChannelName */
const TMWTYPES_CHAR * TMWDEFS_GLOBAL tmwsctr_getChannelName(
  TMWSCTR *pSector)
{
  return(tmwsesn_getChannelName(pSector->pSession));
}
#endif

/* function: tmwsctr_setUserDataPtr */
void TMWDEFS_GLOBAL tmwsctr_setUserDataPtr(
  TMWSCTR *pSector, 
  void *pUserData)
{
  pSector->pUserData = pUserData;
}

/* function: tmwsctr_getUserDataPtr */
void * TMWDEFS_GLOBAL tmwsctr_getUserDataPtr(
  TMWSCTR *pSector)
{
  return pSector->pUserData;
}


