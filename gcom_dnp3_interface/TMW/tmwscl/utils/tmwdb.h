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

/* file: tmwdb.h
 * description: Utilities for managing the TMW database queue
 */
#ifndef TMWDB_DEFINED
#define TMWDB_DEFINED

#include "tmwscl/utils/tmwdefs.h"
#include "tmwscl/utils/tmwcnfg.h"
#include "tmwscl/utils/tmwdlist.h"

#if TMWCNFG_SUPPORT_ASYNCH_DB

struct TMWDBDataStruct;

/* Define function used to store entries in the database queue
 */
typedef TMWTYPES_BOOL (*TMWDB_STORE_FUNC)(
  struct TMWDBDataStruct *pData);

/* Define base structure for data elements in the database
 * queue.
 */
typedef struct TMWDBDataStruct {
  /* List Member, must be first entry */
  TMWDLIST_MEMBER listMember;

  void *pDbHandle;
  TMWDB_STORE_FUNC pStoreFunc;
} TMWDB_DATA;

#ifdef __cplusplus
extern "C" {
#endif

  /* function: tmwdb_init 
   * purpose: Initialize database queue
   *  This should be called once at startup if TMWCNFG_SUPPORT_ASYNCH_DB
   *   is defined TMWDEFS_TRUE on a master SCL implementation.
   * arguments:
   *  maxSize - maximum number of entries to allow in queue.
   * returns:
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwdb_init(
    TMWTYPES_UINT maxSize);

  /* function: tmwdb_destroy 
   * purpose: Destroy the entire database queue
   *  removing all of the entries from the queue. After this is called
   *  the database queue no longer exists.
   * argments:
   *  void
   * returns:
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwdb_destroy(void);

  /* function: tmwdb_setMaxSize 
   * purpose: Set maximum size of database queue
   * arguments:
   *  maxSize - number of entries to allow on the async database queue.
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL tmwdb_setMaxSize(
    TMWTYPES_UINT maxSize);

  /* function: tmwdb_storeEntry
   * purpose: Read the next entry from the database queue
   *  and call the associated store function.
   * arguments:
   *  lockQueue - if set to TMWDEFS_TRUE the database queue will be locked 
   *   and unlocked in this function (if TMWCNFG_SUPPORT_THREADS is defined 
   *   as TMWDEFS_TRUE). If this function is going to be called repeatedly 
   *   from a loop it is much more efficient for the caller to lock the queue 
   *   by calling tmwdb_lockQueue and set lockQueue to TMWDEFS_FALSE. When 
   *   finished looping, tmwdb_unlockQueue must be called.
   * returns:
   *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
   */
  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL tmwdb_storeEntry(
    TMWTYPES_BOOL lockQueue);

  /* function: tmwdb_lockQueue
   * purpose: lock the asynchronous database queue if multiple threads
   *  are supported.
   *  NOTE: make sure to call tmwdb_unlockQueue when finished storing entries.
   * arguments:
   *  none
   * returns:
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwdb_lockQueue(void);

  /* function: tmwdb_unlockQueue
   * purpose: unlock the asynchronous database queue if multiple threads
   *  are supported. This should be called if tmwdb_lockQueue 
   *  and tmwdb_storeEntry(TMWDEFS_FALSE) have been called.
   * arguments:
   *  none
   * returns:
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwdb_unlockQueue(void);

  /* function: tmwdb_getSize 
   * purpose: Return the current size of the database queue
   * arguments:
   *  none
   * returns:
   *  number of entries in the queue
   */
  TMWDEFS_SCL_API TMWTYPES_UINT TMWDEFS_GLOBAL tmwdb_getSize(void);

  /* function: tmwdb_initData
   * purpose: Initialize a database entry
   *  This is called internally by the SCL before putting an
   *  entry on the asynchronous database queue
   * arguments:
   * returns:
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwdb_initData(
    TMWDB_DATA *pData, 
    void *pDbHandle, 
    TMWDB_STORE_FUNC pStoreFunc);

  /* function: tmwdb_addEntry
   * purpose: Add an entry to the database queue
   *  This is called internally by the SCL to add an entry to the queue.
   * arguments:
   * returns:
   *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
   */
  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL tmwdb_addEntry(
    TMWDB_DATA *pData);
  
  /* function: tmwdb_closeDatabase
   * purpose: Removes all queued database updates for specified database
   *  from the async database queue. This is called internally by the SCL
   *  when a master session/sector is closed.
   * arguments:
   *  pDbHandle - database handle
   * returns:
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwdb_closeDatabase(
    void *pDbHandle);

#ifdef __cplusplus
}
#endif
#endif /* TMWCNFG_SUPPORT_ASYNCH_DB */
#endif /* TMWDB_DEFINED */
