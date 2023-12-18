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

/* file: tmwdlist.h 
 * description: Doubly linked list
 *  (note: current implementation is only a singly linked list because
 *   that is all we need)
 */
#ifndef TMWDLIST_DEFINED
#define TMWDLIST_DEFINED

#include "tmwscl/utils/tmwdefs.h"
#include "tmwscl/utils/tmwtypes.h"

/* Define a data structure that defines a list member. Structures that
 * need to be list members should 'extend' this structure by declaring
 * it as the first member variable. Hence the structure can be cast
 * to a TMWDLIST_MEMBER and vice versa as necessary.
 */
typedef struct TMWDlistMember {
  struct TMWDlistMember *pNext;
  struct TMWDlistMember *pPrev;
  struct TMWDoubleList  *pList;
} TMWDLIST_MEMBER;

/* Define a data structure that defines a list */
typedef struct TMWDoubleList {
  TMWTYPES_UINT size;
  TMWDLIST_MEMBER *pFirst;
  TMWDLIST_MEMBER *pLast;
} TMWDLIST;

/* must be equivalent to TMWMEM_FREE_FUNC */
typedef void (*TMWDLIST_FREE_FUNC)(
  void *pBuf);        

#ifdef __cplusplus
extern "C" 
{
#endif

  /* function: tmwdlist_initialize 
   * purpose: Initialize a list. Must be called for each instance of
   *  a list.
   * arguments:
   *  pList - pointer to TMWDLIST structure
   * returns:
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwdlist_initialize(
    TMWDLIST *pList);

  /* function: tmwdlist_destroy 
   * purpose: Destroy all entries in the list, calling pFreeFunc
   *  to free each entry.
   * arguments:
   *  pList - pointer to TMWDLIST structure
   *  pFreeFunc - function called to free each member of the list
   * returns:
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwdlist_destroy(
    TMWDLIST *pList,
    TMWDLIST_FREE_FUNC pFreeFunc);

  /* function: tmwdlist_size 
   * purpose: Return the size of the list.
   * arguments:
   *  pList - pointer to TMWDLIST structure
   * returns:
   *  number of entries in the list
   */
  TMWDEFS_SCL_API TMWTYPES_UINT TMWDEFS_GLOBAL tmwdlist_size(
    TMWDLIST *pList);

  /* function: tmwdlist_addEntry 
   * purpose: Add the specified entry at the end of the list.
   * arguments:
   *  pList - pointer to TMWDLIST structure
   *  pEntry - entry to add
   * returns:
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwdlist_addEntry(
    TMWDLIST *pList, 
    TMWDLIST_MEMBER *pEntry);

  /* function: tmwdlist_insertEntryAt 
   * purpose: Insert entry at the specified index.
   * arguments:
   *  pList - pointer to TMWDLIST structure
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL tmwdlist_insertEntryAt(
    TMWDLIST *pList, 
    TMWTYPES_UINT index, 
    TMWDLIST_MEMBER *pEntry);

  /* function: tmwdlist_insertEntryBefore 
   * purpose: Insert the specified entry before an existing entry.
   * arguments:
   *  pList - pointer to TMWDLIST structure
   *  pExistingEntry - insert new entry before this one
   *  pNewEntry - entry to insert
   * returns:
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwdlist_insertEntryBefore(
    TMWDLIST *pList, 
    TMWDLIST_MEMBER *pExistingEntry, 
    TMWDLIST_MEMBER *pNewEntry);

  /* function: tmwdlist_insertEntryAfter 
   * purpose: Insert the specified entry after an existing entry.
   * arguments:
   *  pList - pointer to TMWDLIST structure
   *  pExistingEntry - insert new entry after this one
   *  pNewEntry - entry to insert
   * returns:
   *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL tmwdlist_insertEntryAfter(
    TMWDLIST *pList, 
    TMWDLIST_MEMBER *pExistingEntry, 
    TMWDLIST_MEMBER *pNewEntry);

  /* function: tmwdlist_removeEntry 
   * purpose: Remove the specified entry from the list.
   * arguments:
   *  pList - pointer to TMWDLIST structure
   *  pEntry - entry to remove
   * returns:
   *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
   */
  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL tmwdlist_removeEntry(
    TMWDLIST *pList, 
    TMWDLIST_MEMBER *pEntry);

  TMWDLIST_MEMBER * TMWDEFS_GLOBAL tmwdlist_removeFirst(
    TMWDLIST *pList);

  /* function: tmwdlist_getFirst 
   * purpose: Returns the first entry in the list
   * arguments:
   *  pList - pointer to a TMWDLIST structure
   * returns:
   *  pointer to the requested entry or TMWDEFS_NULL
   */
  TMWDEFS_SCL_API TMWDLIST_MEMBER * TMWDEFS_GLOBAL tmwdlist_getFirst(
    TMWDLIST *pList);

  /* function: tmwdlist_getLast 
   * purpose: Returns the last entry in the list.
   * arguments:
   *  pList - pointer to a TMWDLIST structure
   * returns:
   *  pointer to the requested entry or TMWDEFS_NULL
   */
  TMWDEFS_SCL_API TMWDLIST_MEMBER * TMWDEFS_GLOBAL tmwdlist_getLast(
    TMWDLIST *pList);

  /* function: tmwdlist_getEntry 
   * purpose: Get the entry at the specified index.
   * arguments:
   *  pList - pointer to a TMWDLIST structure
   *  index - 0 based index of entry to get
   * returns:
   *  pointer to the requested entry or TMWDEFS_NULL
   */
  TMWDLIST_MEMBER * TMWDEFS_GLOBAL tmwdlist_getEntry(
    TMWDLIST *pList, 
    TMWTYPES_UINT index);

  /* function: tmwdlist_getAfter 
   * purpose: Get the entry in the list immediately after the specified 
   *  entry.
   * NOTE: this will verify that pEntry is on the list.
   * arguments:
   *  pList - pointer to a TMWDLIST structure
   *  pEntry - return the entry after this one
   * returns:
   *  pointer to the requested entry or TMWDEFS_NULL
   */
  TMWDEFS_SCL_API TMWDLIST_MEMBER * TMWDEFS_GLOBAL tmwdlist_getAfter(
    TMWDLIST *pList, 
    TMWDLIST_MEMBER *pEntry);

  /* function: tmwdlist_getNext  
   * purpose: Get the entry immediately after the specified 
   *  entry.
   *  NOTE: this function will not check for NULL pointer or
   *   the presence of this element on the list. A safe function
   *   to call is tmwdlist_getAfter()
   * arguments:
   *  pEntry - return the entry after this one
   * returns:
   *  pointer to the requested entry or TMWDEFS_NULL
   */
  TMWDEFS_SCL_API TMWDLIST_MEMBER * TMWDEFS_GLOBAL tmwdlist_getNext(
    TMWDLIST_MEMBER *pEntry);

  /* function: tmwdlist_containsEntry
   * purpose: Check to see if this pEntry is on this list 
   * arguments:
   *  pList - pointer to a TMWDLIST structure
   *  pEntry - entry to look for in list
   * returns: 
   *  TMWDEFS_TRUE if entry is on list
   *  TMWDEFS_FALSE otherwise
   */
  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL tmwdlist_containsEntry(
    TMWDLIST *pList,
    TMWDLIST_MEMBER *pEntry); 

#ifdef __cplusplus
}
#endif
#endif /* TMWDLIST_DEFINED */

