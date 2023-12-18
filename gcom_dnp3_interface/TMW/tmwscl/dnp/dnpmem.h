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

/* file: dnpmem.h
 * description:   DNP memory allocation routines
 */
#ifndef DNPMEM_DEFINED
#define DNPMEM_DEFINED

#include "tmwscl/utils/tmwcnfg.h"
#include "tmwscl/utils/tmwmem.h"
#include "tmwscl/dnp/dnpcnfg.h"

/* Memory allocation defines */
typedef enum DNPMemAllocType {
  DNPMEM_CHNL_TYPE,
  DNPMEM_LINK_FRAME_TYPE,
  DNPMEM_LINK_SESSION_INFO_TYPE,
  DNPMEM_LINK_CONTEXT_TYPE,
  DNPMEM_TPRT_SESSION_INFO_TYPE,
  DNPMEM_TPRT_CONTEXT_TYPE,
  DNPMEM_CHNL_TX_DATA_TYPE,
  DNPMEM_ALLOC_TYPE_MAX
} DNPMEM_ALLOC_TYPE;

typedef struct {
  /* Specify the maximum number of DNP channels 
   * The TMW SCL needs one for each DNP serial or TCP connection
   */
  TMWTYPES_UINT numChannels;

  /* Specify the number of link layer frames buffers that can be allocated. 
   * The TMW SCL needs one per DNP channel if Diagnostics are compiled in.
   */
  TMWTYPES_UINT numFrames;

  /* Specify the number of link layer session contexts that can be allocated. 
   * The TMW SCL needs one per DNP master or slave session.
   */
  TMWTYPES_UINT numSessions;

  /* Specify the number of link layer contexts that can be allocated. 
   * The TMW SCL needs one per DNP channel.
   */
  TMWTYPES_UINT numLinkContexts;

  /* Specify the number of transport sessions that can be allocated. 
   * The TMW SCL needs one per DNP master or slave session.
   */
  TMWTYPES_UINT numTprtSessions;
  
  /* Specify the number of transport layer contexts that can be allocated. 
   * The TMW SCL needs one per DNP channel.
   */
  TMWTYPES_UINT numTprtContexts;

  /* 1 for each DNP request or response being queued or transmitted
   * recommend at least 1 per session for DNP slave
   * recommend 2 or more per session for DNP master
   *   requires more if multiple requests are to be queued on master.
   *   if requests are sent automatically based on IIN bits, 1 is needed
   *   for each simultaneous request.
   */
  TMWTYPES_UINT numTxDatas;
} DNPMEM_CONFIG;

#ifdef __cplusplus
extern "C" 
{
#endif

  /* routine: dnpmem_initConfig
   * purpose:  INTERNAL function to initialize the memory configuration 
   *  structure indicating how many of each structure type to put in
   *  the memory pool. These will be initialized according 
   *  to the compile time defines.
   *  NOTE: user should call sdnpmem_initConfig() or mdnpmem_initConfig()
   *  to modify the number of buffers allowed for each type.
   * arguments:
   *  pConfig - pointer to memory configuration structure to be filled in
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL dnpmem_initConfig(
    DNPMEM_CONFIG *pConfig);
 
  /* routine: dnpmem_init
   * purpose: INTERNAL memory management init function.
   *  NOTE: user should call sdnpmem_initMemory()or mdnpmem_initMemory()
   *  to modify the number of buffers allowed for each type.
   * arguments:
   *  pConfig - pointer to memory configuration structure to be used
   * returns:
   *  TMWDEFS_TRUE if successful
   *  TMWDEFS_FALSE otherwise
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL dnpmem_init(
    DNPMEM_CONFIG *pConfig);

  /* function: dnpmem_alloc
   * purpose:  Allocate memory  
   * arguments: 
   *  type - enum value indicating what structure to allocate
   * returns:
   *   TMWDEFS_NULL if allocation failed
   *   void * pointer to allocated memory if successful
   */
  void * TMWDEFS_GLOBAL dnpmem_alloc(
    DNPMEM_ALLOC_TYPE type);

  /* function: dnpmem_free
   * purpose:  Deallocate memory
   * arguments: 
   *  pBuf - pointer to buffer to be deallocated
   * returns:    
   *   void  
   */
  void TMWDEFS_GLOBAL dnpmem_free(
    void *pBuf);

  /* function: dnpmem_getUsage
   * purpose:  Determine memory usage for each type of memory
   *    managed by this file.
   * arguments: 
   *  index: index of pool, starting with 0 caller can call
   *    this function repeatedly, while incrementing index. When
   *     index is larger than number of pools, this function
   *     will return TMWDEFS_FALSE
   *  pName: pointer to a char pointer to be filled in
   *  pStruct: pointer to structure to be filled in.
   * returns:    
   *  TMWDEFS_TRUE  if successfully returned usage statistics.
   *  TMWDEFS_FALSE if failure because index is too large.
   */
  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL dnpmem_getUsage(
    TMWTYPES_UCHAR index,
    const TMWTYPES_CHAR **pName,
    TMWMEM_POOL_STRUCT *pStruct);

#ifdef __cplusplus
}
#endif

#endif /* DNPMEM_DEFINED */

