/*****************************************************************************/
/* Triangle MicroWorks, Inc.                         Copyright (c) 1997-2022 */
/*****************************************************************************/
/*                            MPP_COMMAND_PRODUCT                            */
/*                                                                           */
/* FILE NAME:    mdnpmem.h                                                   */
/* DESCRIPTION:  Master DNP memory allocation routines                       */
/* PROPERTY OF:  Triangle MicroWorks, Inc. Raleigh, North Carolina USA       */
/*               www.TriangleMicroWorks.com  (919) 870-5101                  */
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
/*****************************************************************************/
#ifndef MDNPMEM_DEFINED
#define MDNPMEM_DEFINED

#include "tmwscl/utils/tmwdefs.h"
#include "tmwscl/utils/tmwmem.h"
#include "tmwscl/dnp/mdnpcnfg.h"
#include "tmwscl/dnp/mdnpdata.h"
#include "tmwscl/dnp/mdnpsesp.h"
#include "tmwscl/dnp/dnpmem.h"


/* Memory allocation defines */
typedef enum MdnpmemAllocType {
  MDNPMEM_MDNPSESN_TYPE,
  MDNPMEM_OPER_CALLBACK_TYPE,

#if MDNPDATA_SUPPORT_OBJ70
  MDNPMEM_FILE_TRANSFER_TYPE,
#endif

#if MDNPDATA_SUPPORT_DATASETS
  MDNPMEM_DATASET_CONTEXT_TYPE,
#endif 
  
#if MDNPDATA_SUPPORT_OBJ120
  MDNPMEM_AUTH_INFO_TYPE,
#if MDNPCNFG_SUPPORT_SA_VERSION5
  MDNPMEM_AUTH_USER_TYPE,
  MDNPMEM_AUTH_EVENT_TYPE,
#endif
#endif

#if TMWCNFG_USE_SIMULATED_DB
  MDNPMEM_SIM_DATABASE_TYPE,
#if MDNPDATA_SUPPORT_DATASETS
  MDNPMEM_SIM_DSET_PROTO_TYPE,
  MDNPMEM_SIM_DSET_DESCR_TYPE,
#endif

#if MDNPDATA_SUPPORT_OBJ120
  MDNPMEM_SIM_AUTH_USER_TYPE,
#endif
#endif

  MDNPMEM_ALLOC_TYPE_MAX
} MDNPMEM_ALLOC_TYPE;

typedef struct {

  /* Specify number of Master DNP Sessions */
  TMWTYPES_UINT numSessions;

  /* Specify number of operate requests that can be in progress. Generally
   * you can only have one request pending per channel so setting this to
   * the maximum number of configured channel is usually correct.
   */
  TMWTYPES_UINT numOperCbackDatas;

  /* Specify number of file transfer requests can be in progress. Currently
   * you can only have one request pending per session so setting this to
   * the maximum number of configured sessions is the max needed.
   */
  TMWTYPES_UINT numFileCbackDatas;
 
  /* Specify number of data set transfer contexts. You can only have one request 
   * pending per session so setting this to the maximum number of configured 
   * sessions is the max needed.
  */
  TMWTYPES_UINT numDatasetContexts;
  
  /* Specify number of  master DNP Secure Authentication User structures 
   * that can be allocated.
   */
  TMWTYPES_UINT numSecureAuthUsers;

  /* Specify number of master DNP Secure Authentication State Machine Event  
   * structures that can be allocated.
   */
  TMWTYPES_UINT numSecureAuthEvents;

  /* Specify the number of master DNP simulated databases in use. The SCL will
   * allocate a new simulated database for each session. These are not
   * needed once your actual database is implemented.
   */
  TMWTYPES_UINT numSimDbases; 

  /* Specify number of simulated master DNP Data Set structures that can be 
   * allocated. These are not needed once your actual database is implemented.
   */
  TMWTYPES_UINT numSimDatasets;

} MDNPMEM_CONFIG;

#ifdef __cplusplus
extern "C" 
{
#endif

  /* routine: mdnpmem_initConfig
   * purpose:  initialize specified memory configuration structure,
   *  indicating the number of buffers of each structure type to 
   *  put in each memory pool. These will be initialized according 
   *  to the compile time defines. The user can change the desired
   *  fields and call mdnpmem_initMemory()
   * arguments:
   *  pConfig - pointer to memory configuration structure to be filled in
   *  pDnpConfig - pointer to memory configuration structure to be filled in
   *  pTmwConfig - pointer to memory configuration structure to be filled in
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL mdnpmem_initConfig(
    MDNPMEM_CONFIG *pConfig,
    DNPMEM_CONFIG  *pDnpConfig,
    TMWMEM_CONFIG  *pTmwConfig);

  /* routine: mdnpmem_initMemory
   * purpose: memory management init function. Can be used
   *  to modify the number of buffers that will be allowed in each
   *  buffer pool. This can only be used when TMWCNFG_USE_DYNAMIC_MEMORY
   *  is set to TMWDEFS_TRUE
   *  NOTE: This should be called before calling tmwappl_initApplication()
   * arguments:
   *  pConfig - pointer to memory configuration structure to be used
   *  pDnpConfig - pointer to memory configuration structure to be used
   *  pTmwConfig - pointer to memory configuration structure to be used
   * returns:
   *  TMWDEFS_TRUE if successful
   *  TMWDEFS_FALSE otherwise
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpmem_initMemory(
    MDNPMEM_CONFIG *pConfig,
    DNPMEM_CONFIG  *pDnpConfig,
    TMWMEM_CONFIG  *pTmwConfig);

  /* routine: mdnpmem_init
   * purpose: INTERNAL memory management init function. 
   *  NOTE: user should call mdnpsesn_initMemory() to modify the number
   *  of buffers allowed for each type.
   * arguments:
   *  pConfig - pointer to memory configuration structure to be used
   * returns:
   *  TMWDEFS_TRUE if successful
   *  TMWDEFS_FALSE otherwise
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpmem_init(
    MDNPMEM_CONFIG *pConfig);

  /* function: mdnpmem_alloc
   * purpose:  Allocate memory  
   * arguments: 
   *  type - enum value indicating what structure to allocate
   * returns:
   *   TMWDEFS_NULL if allocation failed
   *   void * pointer to allocated memory if successful
   */
  void * TMWDEFS_GLOBAL mdnpmem_alloc(
    MDNPMEM_ALLOC_TYPE type);

  /* function: mdnpmem_free
   * purpose:  Deallocate memory
   * arguments: 
   *  pBuf - pointer to buffer to be deallocated
   * returns:    
   *   void  
   */
  void TMWDEFS_GLOBAL mdnpmem_free(
    void *pBuf);

  /* function: mdnpmem_getUsage
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
  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpmem_getUsage(
    TMWTYPES_UCHAR index,
    const TMWTYPES_CHAR **pName,
    TMWMEM_POOL_STRUCT *pStruct);

#ifdef __cplusplus
}
#endif
#endif /* MDNPMEM_DEFINED */

