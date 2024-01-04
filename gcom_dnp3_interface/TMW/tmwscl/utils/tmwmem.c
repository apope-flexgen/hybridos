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
/* file: tmwmem.c
 * description: Implementation of memory allocation functions.
 */
#include "tmwscl/utils/tmwtarg.h"
#include "tmwscl/utils/tmwmem.h"
#include "tmwscl/utils/tmwappl.h"
#include "tmwscl/utils/tmwchnl.h"
#include "tmwscl/utils/tmwphys.h"
#include "tmwscl/utils/tmwsctr.h"
#include "tmwscl/utils/tmwsesn.h"
#if TMWCNFG_USE_SIMULATED_DB
#include "tmwscl/utils/tmwsim.h"
#endif

typedef struct TMWmemApplication
{
  TMWMEM_HEADER     header;
  TMWAPPL           databuf;
} TMWMEM_TMWAPPL;

typedef struct TMWmemTMWChannel
{
  TMWMEM_HEADER     header;
  TMWCHNL           databuf;
} TMWMEM_TMWCHNL;

typedef struct TMWmemTMWPhysContext
{
  TMWMEM_HEADER     header;
  TMWPHYS_CONTEXT   databuf;
} TMWMEM_TMWPHYS_CONTEXT;

#if TMWCNFG_USE_SIMULATED_DB
typedef struct TMWmemTMWSimPoint
{
  TMWMEM_HEADER     header;
  TMWSIM_POINT      databuf;
} TMWMEM_TMWSIM_POINT;

typedef struct TMWmemTMWSimString
{
  TMWMEM_HEADER     header;
  TMWTYPES_UCHAR    databuf[TMWSIM_STRING_MAX_LENGTH];
} TMWMEM_TMWSIM_STRING;

#if  TMWCNFG_SIM_SUPPORT_EXT_STRINGS
typedef struct TMWmemTMWSimExtString
{
  TMWMEM_HEADER     header;
  /* Provide extra octet for code that Null Terminates */
  TMWTYPES_UCHAR    databuf[TMWSIM_EXT_STRING_MAX_LENGTH+1];
} TMWMEM_TMWSIM_EXT_STRING;
#endif
#endif

static const TMWTYPES_CHAR *_nameTable[TMWMEM_MAX_TYPE] = {
  "TMWAPPL",
  "TMWCHNL",
  "TMWPHYS_CONTEXT",
#if TMWCNFG_USE_SIMULATED_DB
  "TMWSIM_POINT",
  "TMWSIM_STRING",
#if  TMWCNFG_SIM_SUPPORT_EXT_STRINGS
  "TMWSIM_EXTSTRING"
#endif
#endif
};

#if !TMWCNFG_USE_DYNAMIC_MEMORY
/* Use static allocated array of memory instead of dynamic memory */
static TMWMEM_TMWAPPL           tmwmem_appls[TMWMEM_NUMALLOC_APPLS];
static TMWMEM_TMWCHNL           tmwmem_chnls[TMWMEM_NUMALLOC_CHNLS];
static TMWMEM_TMWPHYS_CONTEXT   tmwmem_physContexts[TMWMEM_NUMALLOC_PHYS_CONTEXTS];
#if TMWCNFG_USE_SIMULATED_DB
static TMWMEM_TMWSIM_POINT      tmwmem_simPoints[TMWMEM_NUMALLOC_SIMPOINTS];
static TMWMEM_TMWSIM_STRING     tmwmem_simStrings[TMWMEM_NUMALLOC_SIMSTRINGS];
#if  TMWCNFG_SIM_SUPPORT_EXT_STRINGS
static TMWMEM_TMWSIM_EXT_STRING tmwmem_simExtStrings[TMWMEM_NUMALLOC_SIMEXTSTRINGS];
#endif
#endif

#endif

static TMWMEM_POOL_STRUCT _allocTable[TMWMEM_MAX_TYPE];

typedef struct TMWmemPool
{
#if TMWCNFG_SUPPORT_THREADS
  TMWDEFS_RESOURCE_LOCK lock;
#endif
  TMWTYPES_UCHAR spare;
} TMWMEM_POOL;

#if TMWCNFG_SUPPORT_THREADS
/* memory pool structure */
static TMWMEM_POOL _memoryPool;
#endif

#if TMWCNFG_USE_DYNAMIC_MEMORY
void TMWDEFS_GLOBAL tmwmem_initConfig(
  TMWMEM_CONFIG  *pConfig)
{ 
  pConfig->numAppls        = TMWMEM_NUMALLOC_APPLS;
  pConfig->numChnls        = TMWMEM_NUMALLOC_CHNLS;
  pConfig->numPhysContexts = TMWMEM_NUMALLOC_PHYS_CONTEXTS;
#if TMWCNFG_USE_SIMULATED_DB
  pConfig->numSimPoints    = TMWMEM_NUMALLOC_SIMPOINTS;
  pConfig->numSimStrings   = TMWMEM_NUMALLOC_SIMSTRINGS;
#if  TMWCNFG_SIM_SUPPORT_EXT_STRINGS
  pConfig->numSimExtStrings = TMWMEM_NUMALLOC_SIMEXTSTRINGS;
#endif
#endif
}
#endif

/* function: tmwmem_alloc */
void * TMWDEFS_GLOBAL tmwmem_alloc(
  TMWMEM_TYPE type)
{
  if ((type >= TMWMEM_MAX_TYPE)
    || (type < 0))
  {
    return(TMWDEFS_NULL);
  }

  return(tmwmem_lowAlloc(&_allocTable[type]));
}

/* function: tmwmem_checkLimit */
TMWTYPES_BOOL TMWDEFS_GLOBAL tmwmem_checkLimit(
  TMWMEM_TYPE type)
{
  if ((type >= TMWMEM_MAX_TYPE)
    || (type < 0))
  {
    return(TMWDEFS_FALSE);
  }

  return(tmwmem_lowCheckLimit(&_allocTable[type]));
}

/* function: tmwmem_free */
void TMWDEFS_GLOBAL tmwmem_free(
  void *pBuf)
{
  TMWMEM_HEADER *pHeader = TMWMEM_GETHEADER(pBuf);

  if(pHeader->type >= TMWMEM_MAX_TYPE)
  {
    return;
  }

  tmwmem_lowFree(&_allocTable[pHeader->type], pHeader);
}

/* function: tmwmem_getUsage */
TMWTYPES_BOOL TMWDEFS_GLOBAL tmwmem_getUsage(
  TMWTYPES_UCHAR index,
  const TMWTYPES_CHAR **name,
  TMWMEM_POOL_STRUCT *pStruct)
{
  if(index >= TMWMEM_MAX_TYPE)
  {
    return(TMWDEFS_FALSE);
  }

  *name = _nameTable[index];
  *pStruct = _allocTable[index];
  return(TMWDEFS_TRUE);
}

/* Low level routines used by <scl>mem.c files */

/* function: tmwmem_alloc */
void * TMWDEFS_GLOBAL tmwmem_lowAlloc(
  TMWMEM_POOL_STRUCT *pAllocStruct)
{
  TMWMEM_HEADER *pHeader;
  void *pBuf = TMWDEFS_NULL;

  /* Lock memory pool */
  TMWTARG_LOCK_SECTION(&_memoryPool.lock);

#if TMWCNFG_USE_DYNAMIC_MEMORY && !TMWCNFG_ALLOC_ONLY_AT_STARTUP 
 if((pAllocStruct->max == TMWDEFS_NO_LIMIT)
    || (pAllocStruct->allocated < pAllocStruct->max))
  {
    pAllocStruct->allocated++;
    pHeader = (TMWMEM_HEADER *)tmwtarg_alloc(pAllocStruct->size);
    if(pHeader != TMWDEFS_NULL)
    {
      pHeader->type = pAllocStruct->type;
      pBuf = TMWMEM_GETBUF(pHeader);
    }
  }
#else
  pHeader = (TMWMEM_HEADER *)tmwdlist_removeFirst(&pAllocStruct->freeBuffers);
  if(pHeader != TMWDEFS_NULL)
  {
     pAllocStruct->allocated++;
     tmwdlist_addEntry(&pAllocStruct->allocatedBuffers, (TMWDLIST_MEMBER *)pHeader);
     pHeader->type = pAllocStruct->type;
     pBuf = TMWMEM_GETBUF(pHeader);    
  }
#endif

  /* Unlock memory pool */
  TMWTARG_UNLOCK_SECTION(&_memoryPool.lock);

  return(pBuf);
}

/* function: tmwmem_lowCheckLimit */
TMWTYPES_BOOL TMWDEFS_GLOBAL tmwmem_lowCheckLimit(
  TMWMEM_POOL_STRUCT *pAllocStruct)
{
  if((pAllocStruct->max == TMWDEFS_NO_LIMIT)
	  || (pAllocStruct->allocated < pAllocStruct->max))
  {
    return(TMWDEFS_TRUE);
  }

  return(TMWDEFS_FALSE);
}

/* function: tmwmem_lowFree */
void TMWDEFS_GLOBAL tmwmem_lowFree(
  TMWMEM_POOL_STRUCT *pAllocStruct,
  TMWMEM_HEADER *pHeader)
{

  if(pAllocStruct->allocated == 0U)
  {
    /* log error */
    return;
  }

  /* Lock memory pool */
  TMWTARG_LOCK_SECTION(&_memoryPool.lock);

#if TMWCNFG_USE_DYNAMIC_MEMORY && !TMWCNFG_ALLOC_ONLY_AT_STARTUP 
  tmwtarg_free(pHeader);
  pAllocStruct->allocated--;
#else
  if(tmwdlist_removeEntry(&pAllocStruct->allocatedBuffers, (TMWDLIST_MEMBER *)pHeader))
  {
    pAllocStruct->allocated--;
    tmwdlist_addEntry(&pAllocStruct->freeBuffers, (TMWDLIST_MEMBER *)pHeader);
  }
#endif

  /* Unlock memory pool */
  TMWTARG_UNLOCK_SECTION(&_memoryPool.lock);
}

TMWTYPES_BOOL TMWDEFS_GLOBAL tmwmem_lowInit(
  TMWMEM_POOL_STRUCT *pAllocStruct,
  TMWTYPES_UCHAR  type,
  TMWTYPES_UINT   max,
  TMWTYPES_UINT   size,
  TMWTYPES_UCHAR *pPool)
{ 
#if TMWCNFG_USE_DYNAMIC_MEMORY && !TMWCNFG_ALLOC_ONLY_AT_STARTUP  
  TMWTARG_UNUSED_PARAM(pPool);
#else
  TMWTYPES_UINT i;
  TMWMEM_HEADER *pHeader;
#endif

  pAllocStruct += type;

  pAllocStruct->type = type;
  pAllocStruct->allocated = 0;
  pAllocStruct->max = max;
  pAllocStruct->size = size;

#if TMWCNFG_ALLOC_ONLY_AT_STARTUP
  /* Allocate memory at startup, but then use this pool to allocate
   * and deallocate from.
   */
  pPool = (TMWTYPES_UCHAR *)tmwtarg_alloc(max*size);
  if(pPool == TMWDEFS_NULL)
    return TMWDEFS_FALSE;
#endif

#if !TMWCNFG_USE_DYNAMIC_MEMORY || TMWCNFG_ALLOC_ONLY_AT_STARTUP 
  /* Maintain linked list of free and allocated buffers
   * when using static or allocate only at startup
   */
  tmwdlist_initialize(&pAllocStruct->freeBuffers);
  tmwdlist_initialize(&pAllocStruct->allocatedBuffers);
  for(i=0; i<max; i++)
  {
    pHeader = (TMWMEM_HEADER *)(pPool + (size *i)); 
    tmwdlist_addEntry(&pAllocStruct->freeBuffers, (TMWDLIST_MEMBER *)pHeader);
  }
#endif
  return TMWDEFS_TRUE;
}

void TMWDEFS_GLOBAL tmwmem_close(void)
{
  TMWTARG_LOCK_DELETE(&_memoryPool.lock);
}

TMWTYPES_BOOL TMWDEFS_GLOBAL tmwmem_init(
  TMWMEM_CONFIG *pConfig)
{
  if(!tmwappl_internalInit())
    return TMWDEFS_FALSE;

  TMWTARG_LOCK_INIT(&_memoryPool.lock);

#if TMWCNFG_USE_DYNAMIC_MEMORY 
  {
  TMWMEM_CONFIG  config; 
  /* If caller has not specified memory pool configuration, use the
   * default compile time values 
   */
  if(pConfig == TMWDEFS_NULL)
  {
    pConfig = &config;
    tmwmem_initConfig(pConfig);
  }

  if(!tmwmem_lowInit(_allocTable, TMWMEM_APPL_TYPE,         pConfig->numAppls,        sizeof(TMWMEM_TMWAPPL),          TMWDEFS_NULL))
    return TMWDEFS_FALSE;
  if(!tmwmem_lowInit(_allocTable, TMWMEM_CHNL_TYPE,         pConfig->numChnls,        sizeof(TMWMEM_TMWCHNL),          TMWDEFS_NULL))
    return TMWDEFS_FALSE;
  if(!tmwmem_lowInit(_allocTable, TMWMEM_PHYS_CONTEXT_TYPE, pConfig->numPhysContexts, sizeof(TMWMEM_TMWPHYS_CONTEXT),  TMWDEFS_NULL))
    return TMWDEFS_FALSE;
#if TMWCNFG_USE_SIMULATED_DB
  if(!tmwmem_lowInit(_allocTable, TMWMEM_SIM_POINT_TYPE,    pConfig->numSimPoints,    sizeof(TMWMEM_TMWSIM_POINT),     TMWDEFS_NULL))
    return TMWDEFS_FALSE;
  if(!tmwmem_lowInit(_allocTable, TMWMEM_SIM_STRING_TYPE,   pConfig->numSimStrings,   sizeof(TMWMEM_TMWSIM_STRING),    TMWDEFS_NULL))
    return TMWDEFS_FALSE;
#if  TMWCNFG_SIM_SUPPORT_EXT_STRINGS
  if(!tmwmem_lowInit(_allocTable, TMWMEM_SIM_EXT_STRING_TYPE, pConfig->numSimExtStrings, sizeof(TMWMEM_TMWSIM_EXT_STRING), TMWDEFS_NULL))
    return TMWDEFS_FALSE;
#endif
#endif
  }
#else
  /* static memory allocation supported */
  TMWTARG_UNUSED_PARAM(pConfig);

  if(!tmwmem_lowInit(_allocTable, TMWMEM_APPL_TYPE,         TMWMEM_NUMALLOC_APPLS,        sizeof(TMWMEM_TMWAPPL),          (TMWTYPES_UCHAR*)tmwmem_appls))
    return TMWDEFS_FALSE;
  if(!tmwmem_lowInit(_allocTable, TMWMEM_CHNL_TYPE,         TMWMEM_NUMALLOC_CHNLS,        sizeof(TMWMEM_TMWCHNL),          (TMWTYPES_UCHAR*)tmwmem_chnls))
    return TMWDEFS_FALSE;
  if(!tmwmem_lowInit(_allocTable, TMWMEM_PHYS_CONTEXT_TYPE, TMWMEM_NUMALLOC_PHYS_CONTEXTS,sizeof(TMWMEM_TMWPHYS_CONTEXT),  (TMWTYPES_UCHAR*)tmwmem_physContexts))
    return TMWDEFS_FALSE;
#if TMWCNFG_USE_SIMULATED_DB
  if(!tmwmem_lowInit(_allocTable, TMWMEM_SIM_POINT_TYPE,    TMWMEM_NUMALLOC_SIMPOINTS,    sizeof(TMWMEM_TMWSIM_POINT),     (TMWTYPES_UCHAR*)tmwmem_simPoints))
    return TMWDEFS_FALSE;
  if(!tmwmem_lowInit(_allocTable, TMWMEM_SIM_STRING_TYPE,   TMWMEM_NUMALLOC_SIMSTRINGS,   sizeof(TMWMEM_TMWSIM_STRING),    (TMWTYPES_UCHAR*)tmwmem_simStrings))
    return TMWDEFS_FALSE;
#if  TMWCNFG_SIM_SUPPORT_EXT_STRINGS
  if(!tmwmem_lowInit(_allocTable, TMWMEM_SIM_EXT_STRING_TYPE, TMWMEM_NUMALLOC_SIMEXTSTRINGS, sizeof(TMWMEM_TMWSIM_EXT_STRING), (TMWTYPES_UCHAR*)tmwmem_simExtStrings))
    return TMWDEFS_FALSE;
#endif
#endif
#endif
  return TMWDEFS_TRUE;
}
