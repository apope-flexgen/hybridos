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

/* file: sdnpmem.h
 * description:   slave DNP memory allocation routines
 */
#ifndef SDNPMEM_DEFINED
#define SDNPMEM_DEFINED

#include "tmwscl/utils/tmwcnfg.h"
#include "tmwscl/utils/tmwmem.h"
#include "tmwscl/dnp/sdnpdata.h"
#include "tmwscl/dnp/sdnpcnfg.h"
#include "tmwscl/dnp/dnpmem.h"

/* Memory allocation defines */
typedef enum SdnpMemAllocType {
  SDNPMEM_SDNPSESN_TYPE,
  SDNPMEM_ASDU_BUFFER_TYPE,
#if SDNPDATA_SUPPORT_OBJ2
  SDNPMEM_OBJECT2_EVENT_TYPE,
#endif
#if SDNPDATA_SUPPORT_OBJ4
  SDNPMEM_OBJECT4_EVENT_TYPE,
#endif
#if SDNPDATA_SUPPORT_OBJ11
  SDNPMEM_OBJECT11_EVENT_TYPE,
#endif
#if SDNPDATA_SUPPORT_OBJ13
  SDNPMEM_OBJECT13_EVENT_TYPE,
#endif
#if SDNPDATA_SUPPORT_OBJ22
  SDNPMEM_OBJECT22_EVENT_TYPE,
#endif
#if SDNPDATA_SUPPORT_OBJ23
  SDNPMEM_OBJECT23_EVENT_TYPE,
#endif
#if SDNPDATA_SUPPORT_OBJ32
  SDNPMEM_OBJECT32_EVENT_TYPE,
#endif
#if SDNPDATA_SUPPORT_OBJ33
  SDNPMEM_OBJECT33_EVENT_TYPE,
#endif
#if SDNPDATA_SUPPORT_OBJ42
  SDNPMEM_OBJECT42_EVENT_TYPE,
#endif
#if SDNPDATA_SUPPORT_OBJ43
  SDNPMEM_OBJECT43_EVENT_TYPE,
#endif
#if SDNPDATA_SUPPORT_OBJ70
  SDNPMEM_OBJECT70_CTRL_TYPE,
#endif
#if SDNPDATA_SUPPORT_OBJ88
  SDNPMEM_OBJECT88_EVENT_TYPE,
#endif
#if SDNPDATA_SUPPORT_OBJ111
  SDNPMEM_OBJECT111_EVENT_TYPE,
#endif
#if SDNPDATA_SUPPORT_OBJ113
  SDNPMEM_OBJECT113_EVENT_TYPE,
#endif
#if SDNPDATA_SUPPORT_OBJ115
  SDNPMEM_OBJECT115_EVENT_TYPE,
#endif
#if SDNPDATA_SUPPORT_OBJ120
  SDNPMEM_OBJECT120_EVENT_TYPE, 
  SDNPMEM_OBJECT122_EVENT_TYPE, 
  SDNPMEM_AUTH_INFO_TYPE,
#if SDNPCNFG_SUPPORT_SA_VERSION5
  SDNPMEM_AUTH_USER_TYPE,
#endif
#endif
#if TMWCNFG_USE_SIMULATED_DB
  SDNPMEM_SIM_DATABASE_TYPE,
#if SDNPCNFG_USER_MANAGED_EVENTS
  SDNPMEM_SIM_UMEVENT_TYPE,
#endif
#if SDNPDATA_SUPPORT_DATASETS
  SDNPMEM_SIM_DSET_PROTO_TYPE,
  SDNPMEM_SIM_DSET_DESCR_TYPE,
#endif
#if SDNPDATA_SUPPORT_OBJ120
  SDNPMEM_SIM_AUTH_ERROR_TYPE,
  SDNPMEM_SIM_AUTH_USER_TYPE,
#endif
#endif
#if SDNPDATA_SUPPORT_XML2
  SDNPMEM_SIM_XML_CONFIG_TYPE,
#endif

  SDNPMEM_ALLOC_TYPE_MAX
} SDNPMEM_ALLOC_TYPE;


typedef struct {

  /* Specify number of sdnp sessions that can be allocated */
  TMWTYPES_UINT numSessions;
  
  /* Specify number of temporary ASDU buffers that can be allocated.
   * currently the TMW SCL needs one per session.
   */
  TMWTYPES_UINT numASDUBuffers;

  /* Specify number of events of each of these types that can be allocated/queued */
  TMWTYPES_UINT numObject2Events;
  TMWTYPES_UINT numObject4Events;
  TMWTYPES_UINT numObject11Events;
  TMWTYPES_UINT numObject13Events;
  TMWTYPES_UINT numObject22Events;
  TMWTYPES_UINT numObject23Events;
  TMWTYPES_UINT numObject32Events;
  TMWTYPES_UINT numObject33Events;
  TMWTYPES_UINT numObject42Events;
  TMWTYPES_UINT numObject43Events;
  TMWTYPES_UINT numObject70Events;
  TMWTYPES_UINT numObject88Events;
  TMWTYPES_UINT numObject111Events;
  TMWTYPES_UINT numObject113Events; 
  TMWTYPES_UINT numObject115Events;

  /* Specify number of Secure Authentication G120v7 error events that can be allocated */
  TMWTYPES_UINT numObject120Events; 
  /* Specify number of Secure Authentication G122 statistic events that can be allocated */
  TMWTYPES_UINT numObject122Events; 

  /* Specify number of Slave DNP Secure Authentication User structures 
   * that can be allocated.
   */
  TMWTYPES_UINT numSecureAuthUsers; 
   
  /* Specify number of simulated Slave DNP databases that can be allocated.
   * The SCL will allocate a new simulated database for each session.  
   * These are not needed once your actual database is implemented.
   */
  TMWTYPES_UINT numSimDbases;
  /* Specify number of simulated Slave DNP database event structures that can
   * be allocated. These are only necessary for testing the database managed
   * event queues implemented in the simulated database.
   */
  TMWTYPES_UINT numSimUMEvents;
  /* Specify number of simulated Slave DNP Data Set structures that can be 
   * allocated. These are not needed once your actual database is implemented.
   */
  TMWTYPES_UINT numSimDatasets;

  /* Specify the number of buffers needed for generating device profiles in xml format.
   * This would only need to be more than 1 if multiple sessions are supported and they
   * each need to be able to generate a device profile simultaneously.
   * Only applicable if SDNPDATA_SUPPORT_XML2 is defined.
   */
  TMWTYPES_UINT numDeviceProfiles;
} SDNPMEM_CONFIG;


#ifdef __cplusplus
extern "C" {
#endif

  /* routine: sdnpmem_initConfig
   * purpose:  initialize specified memory configuration structure,
   *  indicating the number of buffers of each structure type to 
   *  put in each memory pool. These will be initialized according 
   *  to the compile time defines. The user can change the desired
   *  fields and call sdnpmem_initMemory()
   * arguments:
   *  pConfig - pointer to memory configuration structure to be filled in
   *  pDnpConfig - pointer to memory configuration structure to be filled in
   *  pTmwConfig - pointer to memory configuration structure to be filled in
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL sdnpmem_initConfig(
    SDNPMEM_CONFIG *pConfig,
    DNPMEM_CONFIG  *pDnpConfig,
    TMWMEM_CONFIG  *pTmwConfig);

  /* routine: sdnpmem_initMemory
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
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpmem_initMemory(
    SDNPMEM_CONFIG *pConfig,
    DNPMEM_CONFIG  *pDnpConfig,
    TMWMEM_CONFIG  *pTmwConfig);

  /* routine: sdnpmem_init
   * purpose: INTERNAL memory management init function.
   *  NOTE: user should call sdnpmem_initMemory() to modify the number
   *  of buffers allowed for each type.
   * arguments:
   *  pConfig - pointer to memory configuration structure to be used
   * returns:
   *  TMWDEFS_TRUE if successful
   *  TMWDEFS_FALSE otherwise
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpmem_init(
    SDNPMEM_CONFIG *pConfig);

  /* function: sdnpmem_alloc
   * purpose:  Allocate memory  
   * arguments: 
   *  type - enum value indicating what structure to allocate
   * returns:
   *   TMWDEFS_NULL if allocation failed
   *   void * pointer to allocated memory if successful
   */
  void * TMWDEFS_GLOBAL sdnpmem_alloc(
    SDNPMEM_ALLOC_TYPE type);

  /* function: sdnpmem_checkLimit
   * purpose: Returns true if a call to tmwmem_alloc would succeed.
   * arguments: 
   *   type - enum value indicating what structure to allocate
   * returns:    
   *   TMWDEFS_TRUE if space is available for another object of this type.
   *   TMWDEFS_FALSE if no space is available.
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpmem_checkLimit(
    SDNPMEM_ALLOC_TYPE type);

  /* function: sdnpmem_free
   * purpose:  Deallocate memory
   * arguments: 
   *  pBuf - pointer to buffer to be deallocated
   * returns:    
   *   void  
   */
  void TMWDEFS_CALLBACK sdnpmem_free(
    void *pBuf);

  /* function: sdnpmem_getUsage
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
  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpmem_getUsage(
    TMWTYPES_UCHAR index,
    const TMWTYPES_CHAR **pName,
    TMWMEM_POOL_STRUCT *pStruct);

#ifdef __cplusplus
}
#endif
#endif /* SDNPMEM_DEFINED */
