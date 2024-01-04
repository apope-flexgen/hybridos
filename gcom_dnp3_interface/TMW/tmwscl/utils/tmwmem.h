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

/* file: tmwmem.h
 * description: Memory allocation/deallocation.
 */
#ifndef TMWMEM_DEFINED
#define TMWMEM_DEFINED

#include "tmwscl/utils/tmwdefs.h"
#include "tmwscl/utils/tmwtypes.h"
#include "tmwscl/utils/tmwcnfg.h"
#include "tmwscl/utils/tmwdlist.h"

/* Defines and structures specifically used by tmwmem.c */
/* to support general tmw structure allocation.         */

/* Defines of maximum buffers of each specific type */

#define TMWMEM_NUMALLOC_APPLS          TMWCNFG_MAX_APPLICATIONS
#define TMWMEM_NUMALLOC_CHNLS          TMWCNFG_MAX_CHANNELS
#define TMWMEM_NUMALLOC_PHYS_CONTEXTS  TMWCNFG_MAX_CHANNELS
#define TMWMEM_NUMALLOC_SIMPOINTS      TMWCNFG_MAX_POINTS
#define TMWMEM_NUMALLOC_SIMSTRINGS     TMWCNFG_MAX_STRINGS
#define TMWMEM_NUMALLOC_SIMEXTSTRINGS  TMWCNFG_MAX_EXT_STRINGS

/* Enums for specific types supported by tmwmem_malloc  */
typedef enum TMWmemType {
  TMWMEM_APPL_TYPE,
  TMWMEM_CHNL_TYPE,
  TMWMEM_PHYS_CONTEXT_TYPE,
#if TMWCNFG_USE_SIMULATED_DB
  TMWMEM_SIM_POINT_TYPE,
  TMWMEM_SIM_STRING_TYPE,
#if  TMWCNFG_SIM_SUPPORT_EXT_STRINGS
  TMWMEM_SIM_EXT_STRING_TYPE,
#endif
#endif
  TMWMEM_MAX_TYPE
} TMWMEM_TYPE;

typedef struct {
  TMWTYPES_UINT numAppls;
  TMWTYPES_UINT numChnls;
  TMWTYPES_UINT numPhysContexts;
#if TMWCNFG_USE_SIMULATED_DB
  TMWTYPES_UINT numSimPoints;
  TMWTYPES_UINT numSimStrings;
#if  TMWCNFG_SIM_SUPPORT_EXT_STRINGS
  TMWTYPES_UINT numSimExtStrings;
#endif
#endif
} TMWMEM_CONFIG;

/* Defines and structures used by all ***_mem.c files  */

/* Hidden header structure for each buffer             */
typedef struct TMWmemHeader
{
  /* List Member, must be first entry */
  TMWDLIST_MEMBER listMember;

  /* Enum value to determine what pool to free this buffer into */
  TMWTYPES_UCHAR   type; 

#if TMWCNFG_MEMORY_ALIGN_NEEDED
  /* If a processor requires long word alignment or if the compiler
   * creates unpacked structures, this will address memory alignment
   * and sizof(TMWMEM_HEADER) to get to the data portion of the 
   * structure.
   */
  TMWTYPES_UCHAR  pad1;
  TMWTYPES_UCHAR  pad2;
  TMWTYPES_UCHAR  pad3;
#endif
} TMWMEM_HEADER;
 
/* structure to hold information about a type specific pool        */
typedef struct TMWmemPoolStruct
{

  /* Enum indicating what type specific pool this describes        */
  TMWTYPES_UCHAR  type;

  /* Number currently allocated */
  TMWTYPES_UINT allocated;

  /* Maximum number of buffers that can be allocated for this type */
  TMWTYPES_UINT max;

  /* Size of a buffer including hidden TMWMEM_HEADER structure     */ 
  TMWTYPES_UINT size;

  /* Pointer to list of free and allocated blocks if STATIC or alloc
   * once at startup 
   */
  TMWDLIST  freeBuffers;
  TMWDLIST  allocatedBuffers;
} TMWMEM_POOL_STRUCT;

/* Macros for manipulating hidden header information in each buffer */
#define TMWMEM_GETBUF(pBuf)      ((TMWTYPES_UCHAR*)pBuf+sizeof(TMWMEM_HEADER))
#define TMWMEM_GETHEADER(pBuf)   (TMWMEM_HEADER *)((TMWTYPES_UCHAR*)pBuf - sizeof(TMWMEM_HEADER))
#define TMWMEM_GETTYPE(pHeader)  (TMWTYPES_UCHAR)pHeader->type

/* get_usage type definition */
typedef TMWTYPES_BOOL (*TMWMEM_GET_USAGE_FUNC)(
  TMWTYPES_UCHAR index,
  const TMWTYPES_CHAR **pName,
  TMWMEM_POOL_STRUCT *pStruct);

typedef void (*TMWMEM_FREE_FUNC)(
  void *pBuf);                               
                                
#ifdef __cplusplus
extern "C" 
{
#endif

  /* routine: tmwmem_initConfig
   * purpose:  INTERNAL memory management init function. 
   *  NOTE: user should call xxxmem_initConfig() (ie mdnpmem_initConfig()
   *  or s101mem_initConfig() etc) to get the number of buffers allowed 
   *  for each type.
   * arguments:
   *  pConfig - pointer to memory configuration structure to be filled in
   * returns:
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwmem_initConfig(
    TMWMEM_CONFIG  *pConfig);

  /* routine: tmwmem_init
   * purpose: INTERNAL memory management init function.
   *  NOTE: user should call xxxmem_initMemory() (ie mdnpmem_initMemory(),
   *  s101mem_initMemory(), etc) to modify the number of buffers allowed 
   *  for each type.
   * arguments:
   *  pConfig - pointer to memory configuration structure to be used
   * returns:
   *  TMWDEFS_TRUE if successful
   *  TMWDEFS_FALSE otherwise
   */ 
  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL tmwmem_init(
    TMWMEM_CONFIG *pConfig);

  /* routine: tmwmem_close
   * purpose: INTERNAL memory management close function. Release staticly held memory
   *  NOTE: called by closeApplication
   * returns:
   *  void
   */ 
  void TMWDEFS_GLOBAL tmwmem_close(void);

  /* function: tmwmem_alloc
   * purpose:  Allocate memory  
   * arguments: 
   *   type - enum value indicating what structure to allocate
   * returns:
   *   TMWDEFS_NULL if allocation failed
   *   void * pointer to allocated memory if successful
   */
  TMWDEFS_SCL_API void * TMWDEFS_GLOBAL tmwmem_alloc(
    TMWMEM_TYPE type);

  /* function: tmwmem_checkLimit
   * purpose: Returns true if a call to tmwmem_alloc would succeed.
   * arguments: 
   *   type - enum value indicating what structure to allocate
   * returns:    
   *   TMWDEFS_TRUE if space is available for another object of this type.
   *   TMWDEFS_FALSE if no space is available.
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL tmwmem_checkLimit(
    TMWMEM_TYPE type);

  /* function: tmwmem_free
   * purpose:  Deallocate memory
   * arguments: 
   *   pBuf - pointer to buffer to be deallocated
   * returns:    
   *   void  
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwmem_free(
    void *pBuf);

  /* function: tmwmem_getUsage
   * purpose:  Determine memory usage for each type of memory
   *     managed by this file.
   * arguments: 
   *   index: index of pool, starting with 0 caller can call
   *     this function repeatedly, while incrementing index. When
   *     index is larger than number of pools, this function
   *     will return TMWDEFS_FALSE
   *   pName: pointer to a char pointer to be filled in
   *   pStruct: pointer to structure to be filled in.
   * returns:    
   *   TMWDEFS_TRUE  if successfully returned usage statistics.
   *   TMWDEFS_FALSE if failure because index is too large.
   */
  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL tmwmem_getUsage(
    TMWTYPES_UCHAR index,
    const TMWTYPES_CHAR **pName,
    TMWMEM_POOL_STRUCT *pStruct);

  /* function: tmwmem_lowAlloc
   * purpose:  Low level routine to allocate memory using a 
   *    TMWMEM_POOL_STRUCT structure passed in from a SCL specific
   *    memory allocation routine.
   * arguments: 
   *   pAllocStruct - pointer to structure containing information
   *    about pool to allocate buffer from.
   * returns:
   *   TMWDEFS_NULL if allocation failed
   *   void * pointer to allocated memory if successful
   */
  TMWDEFS_SCL_API void * TMWDEFS_GLOBAL tmwmem_lowAlloc(
    TMWMEM_POOL_STRUCT *pAllocStruct);

  /* function: tmwmem_checkLimit
   * purpose: Returns true if a call to tmwmem_lowAlloc would succeed.
   * arguments: 
   *   pAllocStruct - pointer to structure containing information
   *    about pool to allocate buffer from.
   * returns:    
   *   TMWDEFS_TRUE if space is available for another object of this type.
   *   TMWDEFS_FALSE if no space is available.
   */
  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL tmwmem_lowCheckLimit(
    TMWMEM_POOL_STRUCT *pAllocStruct);

  /* function: tmwmem_lowFree
   * purpose:  Low level deallocation routine 
   * arguments: 
   *   pAllocStruct - pointer to structure containing information
   *     about pool buffer should be freed back into.
   *   pHeader - pointer to memory that should be deallocated
   * returns:
   *   void  
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwmem_lowFree(
    TMWMEM_POOL_STRUCT *pAllocStruct,
    TMWMEM_HEADER *pHeader);
      
  /* function: tmwmem_lowInit
   * purpose:  internal function to initialize a particular memory pool
   * arguments: 
   *   void
   * returns: 
   *  TMWDEFS_TRUE if successful
   *  TMWDEFS_FALSE otherwise
   */
  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL tmwmem_lowInit(
    TMWMEM_POOL_STRUCT *pAllocStruct,
    TMWTYPES_UCHAR  type,
    TMWTYPES_UINT max,
    TMWTYPES_UINT size,
    TMWTYPES_UCHAR  *pPool);

#ifdef __cplusplus
}
#endif
#endif /* TMWMEM_DEFINED */

