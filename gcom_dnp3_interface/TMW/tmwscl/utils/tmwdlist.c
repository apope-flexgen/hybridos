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

/* file: tmwdlist.c
 * description: Double linked list
 */
#include "tmwscl/utils/tmwdefs.h"
#include "tmwscl/utils/tmwtarg.h"
#include "tmwscl/utils/tmwdlist.h"
#include "tmwscl/utils/tmwdiag.h"
 
/* function: tmwdlist_initialize */
void TMWDEFS_GLOBAL tmwdlist_initialize(
  TMWDLIST *pList)
{
  pList->size = 0;
  pList->pFirst = TMWDEFS_NULL;
  pList->pLast = TMWDEFS_NULL;
}

/* function: tmwdlist_destroy */
void TMWDEFS_GLOBAL tmwdlist_destroy(
  TMWDLIST *pList,
  TMWDLIST_FREE_FUNC pFreeFunc)
{
  TMWDLIST_MEMBER *pEntry;
  while((pEntry = tmwdlist_getEntry(pList, 0U)) != TMWDEFS_NULL)
  {
    if (tmwdlist_removeEntry(pList, pEntry))
    {
      if(pFreeFunc != TMWDEFS_NULL)
      {
        pFreeFunc(pEntry);  
      }
    }
  }
}

/* function: tmwdlist_size */
TMWTYPES_UINT TMWDEFS_GLOBAL tmwdlist_size(TMWDLIST *pList)
{
  return(pList->size);
}

/* function: tmwdlist_addEntry */
void TMWDEFS_GLOBAL tmwdlist_addEntry(
  TMWDLIST *pList, 
  TMWDLIST_MEMBER *pEntry)
{
  ASSERT(pEntry != TMWDEFS_NULL);

  if(pList->pFirst == TMWDEFS_NULL)
  {
    pEntry->pNext = TMWDEFS_NULL;
    pEntry->pPrev = TMWDEFS_NULL;
    pList->pFirst = pList->pLast = pEntry;
  }
  else
  {
    pEntry->pNext = TMWDEFS_NULL;
    pEntry->pPrev = pList->pLast;
    pList->pLast->pNext = pEntry;
    pList->pLast = pEntry;
  }

  pEntry->pList = pList;
  pList->size += 1U;
}

/* function: tmwdlist_insertEntryAt */
void TMWDEFS_GLOBAL tmwdlist_insertEntryAt(
  TMWDLIST *pList, 
  TMWTYPES_UINT index, 
  TMWDLIST_MEMBER *pEntry)
{
  if(index == 0U)
  {
    pEntry->pNext = pList->pFirst;
    pEntry->pPrev = TMWDEFS_NULL;

    if(pList->pLast == TMWDEFS_NULL)
    {
      pList->pLast = pEntry;
    }
    else
    {
      pList->pFirst->pPrev = pEntry;
    }

    pList->pFirst = pEntry;

    pEntry->pList = pList;
    pList->size += 1U;
  }
  else if(index >= pList->size)
  {
    tmwdlist_addEntry(pList, pEntry);
  }
  else
  {
    TMWDLIST_MEMBER *pMember = pList->pFirst;
    TMWTYPES_UINT i;

    for(i = 0U; i < (index - 1U) && (pMember != TMWDEFS_NULL); i++)
    {
      pMember = pMember->pNext;
    }

    tmwdlist_insertEntryAfter(pList, pMember, pEntry);
  }
}

/* function: tmwdlist_insertEntryBefore */
void TMWDEFS_GLOBAL tmwdlist_insertEntryBefore(
  TMWDLIST *pList, 
  TMWDLIST_MEMBER *pExistingEntry, 
  TMWDLIST_MEMBER *pNewEntry)
{
  ASSERT(pExistingEntry != TMWDEFS_NULL);
  ASSERT(pNewEntry != TMWDEFS_NULL);

  if(pExistingEntry == pList->pFirst)
  {
    pNewEntry->pNext = pExistingEntry;
    pNewEntry->pPrev = TMWDEFS_NULL;
    pExistingEntry->pPrev = pNewEntry;

    pNewEntry->pList = pList;
    pList->pFirst = pNewEntry;
    pList->size += 1U;
  }
  else
  {
    /* Make sure item is on list */
    if (pExistingEntry->pList == pList)
    {
      TMWDLIST_MEMBER *pPrevious = pExistingEntry->pPrev;
      if (pPrevious != TMWDEFS_NULL)
      {
        pNewEntry->pNext = pExistingEntry;
        pNewEntry->pPrev = pPrevious;
        pPrevious->pNext = pNewEntry;
        pExistingEntry->pPrev = pNewEntry;

        pNewEntry->pList = pList;
        pList->size += 1U;
      }
    }
  }
}

/* function: tmwdlist_insertEntryAfter */
TMWTYPES_BOOL TMWDEFS_GLOBAL tmwdlist_insertEntryAfter(
  TMWDLIST *pList, 
  TMWDLIST_MEMBER *pExistingEntry, 
  TMWDLIST_MEMBER *pNewEntry)
{
  ASSERT(pExistingEntry != TMWDEFS_NULL);
  ASSERT(pNewEntry != TMWDEFS_NULL);

  if (pExistingEntry && pNewEntry)
  {
    if (pExistingEntry->pList == pList)
    {
      pNewEntry->pNext = pExistingEntry->pNext;
      pNewEntry->pPrev = pExistingEntry;

      if (pExistingEntry == pList->pLast)
      {
        pList->pLast = pNewEntry;
      }
      else
      {
        pExistingEntry->pNext->pPrev = pNewEntry;
      }

      pExistingEntry->pNext = pNewEntry;

      pNewEntry->pList = pList;
      pList->size += 1U;
      return TMWDEFS_TRUE;
    }
  }

  return TMWDEFS_FALSE;
}

/* function: tmwdlist_removeEntry */
TMWTYPES_BOOL TMWDEFS_GLOBAL tmwdlist_removeEntry(
  TMWDLIST *pList, 
  TMWDLIST_MEMBER *pEntry)
{
  ASSERT(pEntry != TMWDEFS_NULL);

  /* Make sure list is not empty */
  if(pList->pFirst == TMWDEFS_NULL)
    return(TMWDEFS_FALSE);

  if(pEntry == pList->pFirst)
  {
    pList->pFirst = pEntry->pNext;
    if(pEntry == pList->pLast)
    {
      pList->pLast = TMWDEFS_NULL;
    }
    else
    {
      pEntry->pNext->pPrev = TMWDEFS_NULL;
    }
  }
  else
  {
    /* Make sure the item is on the list */
    if (pEntry->pList == pList)
    {
      TMWDLIST_MEMBER *pPrevious = pEntry->pPrev;
      if (pEntry == pList->pLast)
      {
        pPrevious->pNext = TMWDEFS_NULL;
        pList->pLast = pPrevious;
      }
      else
      {
        pEntry->pNext->pPrev = pPrevious;
        pPrevious->pNext = pEntry->pNext;
      }
    }
    else
    {
      return TMWDEFS_FALSE;
    }
  }

  pEntry->pNext = TMWDEFS_NULL;
  pEntry->pPrev = TMWDEFS_NULL;

  pEntry->pList = TMWDEFS_NULL;
  pList->size -= 1U;
  return(TMWDEFS_TRUE);
}

TMWDLIST_MEMBER * TMWDEFS_GLOBAL tmwdlist_removeFirst(
  TMWDLIST *pList)
{  
  TMWDLIST_MEMBER *pEntry = pList->pFirst;
  if(pEntry != TMWDEFS_NULL)
  {
    if(pEntry == pList->pLast)
    {
      pList->pLast  = TMWDEFS_NULL;
      pList->pFirst = TMWDEFS_NULL;
    }
    else
    {
      pEntry->pNext->pPrev = TMWDEFS_NULL;
      pList->pFirst = pEntry->pNext;
    }

    pEntry->pNext = TMWDEFS_NULL;
    pEntry->pPrev = TMWDEFS_NULL;
    pEntry->pList = TMWDEFS_NULL;
    pList->size -= 1U;
  }
  return(pEntry);
}


/* function: tmwdlist_getFirst */
TMWDLIST_MEMBER * TMWDEFS_GLOBAL tmwdlist_getFirst(
  TMWDLIST *pList)
{
  return(pList->pFirst);
}

/* function: tmwdlist_getLast */
TMWDLIST_MEMBER * TMWDEFS_GLOBAL tmwdlist_getLast(
  TMWDLIST *pList)
{
  return(pList->pLast);
}

/* function: tmwdlist_getEntry */
TMWDLIST_MEMBER * TMWDEFS_GLOBAL tmwdlist_getEntry(
  TMWDLIST *pList, 
  TMWTYPES_UINT index)
{
  TMWTYPES_UINT i;
  TMWDLIST_MEMBER *pMember = pList->pFirst;

  if(pMember != TMWDEFS_NULL)
  {
    for(i = 0; (i < index) && (pMember != TMWDEFS_NULL); i++)
    {
      pMember = pMember->pNext;
    }
  }

  return(pMember);
}

/* function: tmwdlist_getAfter */
TMWDLIST_MEMBER * TMWDEFS_GLOBAL tmwdlist_getAfter(
  TMWDLIST *pList, 
  TMWDLIST_MEMBER *pEntry)
{
  if(pEntry == TMWDEFS_NULL)
  {
    return(pList->pFirst);
  }
  else
  {
    if(pEntry->pList != pList)
    {
      return(TMWDEFS_NULL);
    }

    return(pEntry->pNext);
  }
}

/* function: tmwdlist_getNext */
TMWDLIST_MEMBER * TMWDEFS_GLOBAL tmwdlist_getNext(
  TMWDLIST_MEMBER *pEntry)
{
  /* No error checking, caller better be sure, or call tmwdlist_getAfter instead */
  return(pEntry->pNext);
}

/* function: tmwdlist_containsEntry */
TMWTYPES_BOOL TMWDEFS_GLOBAL tmwdlist_containsEntry(
  TMWDLIST *pList,
  TMWDLIST_MEMBER *pEntry)
{
  /* Make sure the item is on the list */
  if (pEntry->pList == pList)
    return TMWDEFS_TRUE;
  else
    return TMWDEFS_FALSE;
}
