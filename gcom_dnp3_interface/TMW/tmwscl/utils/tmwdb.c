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

/* file: tmwdb.c
 * description: Utilities for managing the TMW database queue
 */
#include "tmwscl/utils/tmwtarg.h"
#include "tmwscl/utils/tmwcnfg.h"
#include "tmwscl/utils/tmwdb.h"
#include "tmwscl/utils/tmwdlist.h"

#if TMWCNFG_SUPPORT_ASYNCH_DB

typedef struct TMWDbQueue
{
  TMWDLIST data;
  TMWTYPES_UINT maxSize;
  TMWTYPES_BOOL overLimit;
#if TMWCNFG_SUPPORT_THREADS
  TMWDEFS_RESOURCE_LOCK lock;
#endif
} TMWDB_QUEUE;

static TMWDB_QUEUE _tmwdbQueue;

/* function: tmwdb_init */
void TMWDEFS_GLOBAL tmwdb_init(
  TMWTYPES_UINT maxSize)
{
  /* Lock database queue */
  TMWTARG_LOCK_INIT(&_tmwdbQueue.lock);
  TMWTARG_LOCK_SECTION(&_tmwdbQueue.lock);

  _tmwdbQueue.maxSize = maxSize;
  _tmwdbQueue.overLimit = TMWDEFS_FALSE;
  tmwdlist_initialize(&_tmwdbQueue.data);

  /* Unlock database queue */
  TMWTARG_UNLOCK_SECTION(&_tmwdbQueue.lock);
}

/* function: tmwdb_destroy */
void TMWDEFS_GLOBAL tmwdb_destroy(void)
{
  void *pEntry;

  /* Lock database queue */
  TMWTARG_LOCK_SECTION(&_tmwdbQueue.lock);

  while ((pEntry = tmwdlist_getFirst(&_tmwdbQueue.data)) != TMWDEFS_NULL)
  {
    tmwdlist_removeEntry(&_tmwdbQueue.data, (TMWDLIST_MEMBER *)pEntry);
    tmwtarg_free(pEntry);
  }

  /* Unlock database queue */
  TMWTARG_UNLOCK_SECTION(&_tmwdbQueue.lock);
  TMWTARG_LOCK_DELETE(&_tmwdbQueue.lock);
}

/* function: tmwdb_setMaxSize */
void TMWDEFS_GLOBAL tmwdb_setMaxSize(
  TMWTYPES_UINT maxSize)
{
  /* Lock database queue */
  TMWTARG_LOCK_SECTION(&_tmwdbQueue.lock);

  _tmwdbQueue.maxSize = maxSize;

  /* Unlock database queue */
  TMWTARG_UNLOCK_SECTION(&_tmwdbQueue.lock);
}

/* function: tmwdb_initData */
void TMWDEFS_GLOBAL tmwdb_initData(
  TMWDB_DATA *pData,
  void *pDbHandle,
  TMWDB_STORE_FUNC pStoreFunc)
{
  pData->pDbHandle = pDbHandle;
  pData->pStoreFunc = pStoreFunc;
  return;
}

/* function: tmwdb_addEntry */
TMWTYPES_BOOL TMWDEFS_GLOBAL tmwdb_addEntry(
  TMWDB_DATA *pData)
{
  TMWTYPES_BOOL status = TMWDEFS_FALSE;

  /* Lock database queue */
  TMWTARG_LOCK_SECTION(&_tmwdbQueue.lock);

  if (tmwdlist_size(&_tmwdbQueue.data) < _tmwdbQueue.maxSize)
  {
    _tmwdbQueue.overLimit = TMWDEFS_FALSE;
    tmwdlist_addEntry(&_tmwdbQueue.data, (TMWDLIST_MEMBER *)pData);
    status = TMWDEFS_TRUE;
  }
  else
  {
    if (!_tmwdbQueue.overLimit)
    {
      TMWDIAG_ERROR("Asynchronous database queue exceeded limit");
    }

    _tmwdbQueue.overLimit = TMWDEFS_TRUE;
    status = TMWDEFS_FALSE;
  }

  /* Unlock database queue */
  TMWTARG_UNLOCK_SECTION(&_tmwdbQueue.lock);

  return(status);
}

/* function: tmwdb_storeEntry */
TMWTYPES_BOOL TMWDEFS_GLOBAL tmwdb_storeEntry(
  TMWTYPES_BOOL lockQueue)
{
  TMWTYPES_BOOL status = TMWDEFS_FALSE;
  TMWDB_DATA *pData;

#if TMWCNFG_SUPPORT_THREADS
  if (lockQueue)
  {
    /* Lock database queue */
    TMWTARG_LOCK_SECTION(&_tmwdbQueue.lock);
  }
#else
  TMWTARG_UNUSED_PARAM(lockQueue);
#endif

  pData = (TMWDB_DATA *)tmwdlist_getFirst(&_tmwdbQueue.data);
  if(pData != TMWDEFS_NULL)
  {
    if (tmwdlist_removeEntry(&_tmwdbQueue.data, (TMWDLIST_MEMBER *)pData))
    {
      /* The queue needs to remain locked to prevent the database being closed 
       * after the pData was removed but before the store function is executed 
       */
      status = pData->pStoreFunc(pData); 
      tmwtarg_free(pData); 
    }
    else
    {
      TMWDIAG_ERROR("tmwdb_storeEntry: Error Removing Item from DataBase Queue");
    }
  }

#if TMWCNFG_SUPPORT_THREADS
  if (lockQueue)
  {
    /* Unlock database queue */
    TMWTARG_UNLOCK_SECTION(&_tmwdbQueue.lock);
  }
#endif

  return(status);
}

/* function: tmwdb_lockQueue */
TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwdb_lockQueue(void)
{
  TMWTARG_LOCK_SECTION(&_tmwdbQueue.lock);
}

/* function: tmwdb_unlockQueue */
TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwdb_unlockQueue(void)
{
  TMWTARG_UNLOCK_SECTION(&_tmwdbQueue.lock);
}

/* function: tmwdb_getSize */
TMWTYPES_UINT TMWDEFS_GLOBAL tmwdb_getSize(void)
{
  TMWTYPES_UINT size;
  size = tmwdlist_size(&_tmwdbQueue.data);
  return(size);
}

/* function: tmwdb_closeDatabase */
void TMWDEFS_GLOBAL tmwdb_closeDatabase(
  void *pDbHandle)
{
  TMWDB_DATA *pData = TMWDEFS_NULL;

  /* Lock database queue */
  TMWTARG_LOCK_SECTION(&_tmwdbQueue.lock);

  pData = (TMWDB_DATA *)tmwdlist_getFirst(&_tmwdbQueue.data);
  while (pData != TMWDEFS_NULL)
  {
    TMWDB_DATA *pNextData = (TMWDB_DATA *)
      tmwdlist_getAfter(&_tmwdbQueue.data, (TMWDLIST_MEMBER *)pData);

    if (pData->pDbHandle == pDbHandle)
    {
      if (tmwdlist_removeEntry(&_tmwdbQueue.data, (TMWDLIST_MEMBER *)pData))
      {
        tmwtarg_free(pData);
      }
      else
      {
        TMWDIAG_ERROR("tmwdb_closeDatabase: Error Removing Item from DataBase Queue");
      }
    }

    pData = pNextData;
  }

  /* Unlock database queue */
  TMWTARG_UNLOCK_SECTION(&_tmwdbQueue.lock);
}
#endif /* TMWCNFG_SUPPORT_ASYNCH_DB */
