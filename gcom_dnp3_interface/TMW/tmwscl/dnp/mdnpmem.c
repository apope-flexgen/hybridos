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
/* file: mdnpmem.c
 * description:  Implementation of Master DNP specific memory allocation functions.
 */

#include "tmwscl/dnp/dnpdiag.h"
#include "tmwscl/dnp/mdnpdata.h"
#include "tmwscl/dnp/mdnpsesp.h"
#include "tmwscl/utils/tmwtarg.h"
#include "tmwscl/dnp/mdnpmem.h"
#include "tmwscl/dnp/mdnpbrm.h"
#include "tmwscl/dnp/mdnpdiag.h"
#if TMWCNFG_USE_SIMULATED_DB
#include "tmwscl/dnp/mdnpsim.h"
#endif

typedef struct{
  TMWMEM_HEADER          header;
  MDNPSESN               databuf;
} MDNPMEM_MDNPSESN;

typedef struct{
  TMWMEM_HEADER          header;
  OperateCallbackData    databuf;
} MDNPMEM_OPER_CALLBACK;

typedef struct{
  TMWMEM_HEADER          header;
  MDNPFILE_XFER_CONTEXT  databuf;
} MDNPMEM_FILE_CALLBACK;

typedef struct{
  TMWMEM_HEADER                header;
  MDNPBRM_DATSET_XFER_CONTEXT  databuf;
} MDNPMEM_DATASET_CONTEXT;

#if MDNPDATA_SUPPORT_OBJ120
typedef struct{
  TMWMEM_HEADER          header;
  MDNPAUTH_INFO          databuf;
} MDNPMEM_AUTH_INFO;

#if MDNPCNFG_SUPPORT_SA_VERSION5
typedef struct{
  TMWMEM_HEADER          header;
  MDNPAUTH_USER          databuf;
} MDNPMEM_AUTH_USER;

typedef struct{
  TMWMEM_HEADER          header;
  MDNPAUTH_EVENT         databuf;
} MDNPMEM_AUTH_EVENT;
#endif 
#endif 

#if TMWCNFG_USE_SIMULATED_DB
typedef struct{
  TMWMEM_HEADER          header;
  MDNPSIM_DATABASE       databuf;
} MDNPMEM_SIM_DATABASE;

#if MDNPDATA_SUPPORT_DATASETS
typedef struct{
  TMWMEM_HEADER               header;
  MDNPSIM_DATASET_PROTO       databuf;
} MDNPMEM_SIM_DATASET_PROTO;

typedef struct{
  TMWMEM_HEADER               header;
  MDNPSIM_DATASET_DESCR_DATA  databuf;
} MDNPMEM_SIM_DATASET_DESCR;

#endif

#if MDNPDATA_SUPPORT_OBJ120
typedef struct{
  TMWMEM_HEADER               header;
  MDNPSIM_AUTHUSER            databuf;
} MDNPMEM_SIM_AUTHUSER;
#endif
#endif

static const TMWTYPES_CHAR *_nameTable[MDNPMEM_ALLOC_TYPE_MAX] = {
  "MDNPSESN",
  "OperateCallbackData"

#if MDNPDATA_SUPPORT_OBJ70
  ,"FileCallbackData"
#endif
#if MDNPDATA_SUPPORT_DATASETS
  ,"DatasetXferContext"
#endif
  
#if MDNPDATA_SUPPORT_OBJ120
  ,"MDNPAUTH_INFO" 
#if MDNPCNFG_SUPPORT_SA_VERSION5
  ,"MDNPAUTH_USER" 
  ,"MDNPAUTH_EVENT"
#endif
#endif

#if TMWCNFG_USE_SIMULATED_DB
  , "MDNPSIM_DATABASE"
#if MDNPDATA_SUPPORT_DATASETS
  , "MDNPSIM_DATASET_PROTO" 
  , "MDNPSIM_DATASET_DESCR" 
#endif
#if MDNPDATA_SUPPORT_OBJ120
  , "MDNPSIM_AUTH_USER" 
#endif
#endif
};
 

#if !TMWCNFG_USE_DYNAMIC_MEMORY
/* Use static allocated array of memory instead of dynamic memory */
static MDNPMEM_MDNPSESN           mdnpmem_mSesns[MDNPCNFG_NUMALLOC_SESNS];
static MDNPMEM_OPER_CALLBACK      mdnpmem_operCbackDatas[MDNPCNFG_NUMALLOC_OPERCBACKDATAS];

#if MDNPDATA_SUPPORT_OBJ70
static MDNPMEM_FILE_CALLBACK      mdnpmem_fileCbackDatas[MDNPCNFG_NUMALLOC_FILECBACKDATAS];
#endif

#if MDNPDATA_SUPPORT_DATASETS
static MDNPMEM_DATASET_CONTEXT    mdnpmem_datasetContexts[MDNPCNFG_NUMALLOC_DATASET_CTXS];
#endif

#if MDNPDATA_SUPPORT_OBJ120
static MDNPMEM_AUTH_INFO          mdnpmem_authInfos[MDNPCNFG_NUMALLOC_SESNS];
#if MDNPCNFG_SUPPORT_SA_VERSION5
static MDNPMEM_AUTH_USER          mdnpmem_authUsers[MDNPCNFG_NUMALLOC_AUTH_USERS]; 
static MDNPMEM_AUTH_EVENT         mdnpmem_authEvents[MDNPCNFG_NUMALLOC_AUTH_EVENTS]; 
#endif
#endif

#if TMWCNFG_USE_SIMULATED_DB
static MDNPMEM_SIM_DATABASE       mdnpmem_simDbases[MDNPCNFG_NUMALLOC_SIM_DATABASES];
#if MDNPDATA_SUPPORT_DATASETS
static MDNPMEM_SIM_DATASET_PROTO  mdnpmem_simDatasetProtos[MDNPCNFG_NUMALLOC_SIM_DATASETS];
static MDNPMEM_SIM_DATASET_DESCR  mdnpmem_simDatasetDescrs[MDNPCNFG_NUMALLOC_SIM_DATASETS];
#endif

#if MDNPDATA_SUPPORT_OBJ120
static MDNPMEM_SIM_AUTHUSER      mdnpmem_simAuthUsers[MDNPCNFG_NUMALLOC_AUTH_USERS]; 
#endif
#endif 

#endif

static TMWMEM_POOL_STRUCT _mdnpmemAllocTable[MDNPMEM_ALLOC_TYPE_MAX];

#if TMWCNFG_USE_DYNAMIC_MEMORY
static void TMWDEFS_LOCAL _initConfig(
  MDNPMEM_CONFIG *pConfig)
{
  pConfig->numSessions           = MDNPCNFG_NUMALLOC_SESNS;
  pConfig->numOperCbackDatas     = MDNPCNFG_NUMALLOC_OPERCBACKDATAS;
#if MDNPDATA_SUPPORT_OBJ70
  pConfig->numFileCbackDatas     = MDNPCNFG_NUMALLOC_FILECBACKDATAS;
#endif
#if MDNPDATA_SUPPORT_DATASETS
  pConfig->numDatasetContexts    = MDNPCNFG_NUMALLOC_DATASET_CTXS;
#endif

#if MDNPCNFG_SUPPORT_SA_VERSION5 && MDNPDATA_SUPPORT_OBJ120
  pConfig->numSecureAuthUsers    = MDNPCNFG_NUMALLOC_AUTH_USERS;
  pConfig->numSecureAuthEvents   = MDNPCNFG_NUMALLOC_AUTH_USERS;
#endif

#if TMWCNFG_USE_SIMULATED_DB
  pConfig->numSimDbases          = MDNPCNFG_NUMALLOC_SIM_DATABASES;
#if MDNPDATA_SUPPORT_DATASETS
  pConfig->numSimDatasets        = MDNPCNFG_NUMALLOC_SIM_DATASETS;
#endif
#endif
}

void TMWDEFS_GLOBAL mdnpmem_initConfig(
  MDNPMEM_CONFIG *pConfig,
  DNPMEM_CONFIG  *pDnpConfig,
  TMWMEM_CONFIG  *pTmwConfig)
{
  _initConfig(pConfig);
  dnpmem_initConfig(pDnpConfig);
  tmwmem_initConfig(pTmwConfig);
}

/* function: mdnpmem_initMemory */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpmem_initMemory(
  MDNPMEM_CONFIG *pMdnpConfig,
  DNPMEM_CONFIG  *pDnpConfig,
  TMWMEM_CONFIG  *pTmwConfig)
{
   /* Initialize memory management and diagnostics for
    * DNP if not yet done 
    */
  if(!tmwmem_init(pTmwConfig))
    return TMWDEFS_FALSE;

  if(!tmwappl_getInitialized(TMWAPPL_INIT_DNP))
  {
    if(!dnpmem_init(pDnpConfig))
      return TMWDEFS_FALSE;

#if TMWCNFG_SUPPORT_DIAG
    dnpdiag_init();
#endif
    tmwappl_setInitialized(TMWAPPL_INIT_DNP);
  }

  /* Now for MDNP if not yet done */
  if(!tmwappl_getInitialized(TMWAPPL_INIT_MDNP))
  {
    if(!mdnpmem_init(pMdnpConfig))
      return TMWDEFS_FALSE;

#if TMWCNFG_SUPPORT_DIAG
    mdnpdiag_init();
#endif
    tmwappl_setInitialized(TMWAPPL_INIT_MDNP);
  }
  return TMWDEFS_TRUE;
}
#endif

/* routine: mdnpmem_init */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpmem_init(
  MDNPMEM_CONFIG *pConfig)
{
#if TMWCNFG_USE_DYNAMIC_MEMORY
  /* dynamic memory allocation supported */
  MDNPMEM_CONFIG  config; 

  /* If caller has not specified memory pool configuration, use the
   * default compile time values 
   */
  if(pConfig == TMWDEFS_NULL)
  {
    pConfig = &config;
    _initConfig(pConfig);
  }

  if(!tmwmem_lowInit(_mdnpmemAllocTable, MDNPMEM_MDNPSESN_TYPE,      pConfig->numSessions,       sizeof(MDNPMEM_MDNPSESN),      TMWDEFS_NULL))
    return TMWDEFS_FALSE;
  if(!tmwmem_lowInit(_mdnpmemAllocTable, MDNPMEM_OPER_CALLBACK_TYPE, pConfig->numOperCbackDatas, sizeof(MDNPMEM_OPER_CALLBACK), TMWDEFS_NULL))
    return TMWDEFS_FALSE;

#if MDNPDATA_SUPPORT_OBJ70
  if(!tmwmem_lowInit(_mdnpmemAllocTable, MDNPMEM_FILE_TRANSFER_TYPE, pConfig->numFileCbackDatas, sizeof(MDNPMEM_FILE_CALLBACK), TMWDEFS_NULL))
    return TMWDEFS_FALSE;
#endif

#if MDNPDATA_SUPPORT_DATASETS
  if(!tmwmem_lowInit(_mdnpmemAllocTable, MDNPMEM_DATASET_CONTEXT_TYPE, pConfig->numDatasetContexts, sizeof(MDNPMEM_DATASET_CONTEXT),TMWDEFS_NULL))
    return TMWDEFS_FALSE;
#endif

#if MDNPDATA_SUPPORT_OBJ120
  if(!tmwmem_lowInit(_mdnpmemAllocTable, MDNPMEM_AUTH_INFO_TYPE,     pConfig->numSessions,        sizeof(MDNPMEM_AUTH_INFO),         TMWDEFS_NULL))
    return TMWDEFS_FALSE;
#if MDNPCNFG_SUPPORT_SA_VERSION5
  if(!tmwmem_lowInit(_mdnpmemAllocTable, MDNPMEM_AUTH_USER_TYPE,     pConfig->numSecureAuthUsers, sizeof(MDNPMEM_AUTH_USER),         TMWDEFS_NULL))
    return TMWDEFS_FALSE;
  if(!tmwmem_lowInit(_mdnpmemAllocTable, MDNPMEM_AUTH_EVENT_TYPE,    pConfig->numSecureAuthEvents,sizeof(MDNPMEM_AUTH_EVENT),        TMWDEFS_NULL))
    return TMWDEFS_FALSE;
#endif
#endif

#if TMWCNFG_USE_SIMULATED_DB
  if(!tmwmem_lowInit(_mdnpmemAllocTable, MDNPMEM_SIM_DATABASE_TYPE,   pConfig->numSimDbases,      sizeof(MDNPMEM_SIM_DATABASE),      TMWDEFS_NULL))
    return TMWDEFS_FALSE;
#if MDNPDATA_SUPPORT_DATASETS
  if(!tmwmem_lowInit(_mdnpmemAllocTable, MDNPMEM_SIM_DSET_PROTO_TYPE, pConfig->numSimDatasets,    sizeof(MDNPMEM_SIM_DATASET_PROTO), TMWDEFS_NULL))
    return TMWDEFS_FALSE;
  if(!tmwmem_lowInit(_mdnpmemAllocTable, MDNPMEM_SIM_DSET_DESCR_TYPE, pConfig->numSimDatasets,    sizeof(MDNPMEM_SIM_DATASET_DESCR), TMWDEFS_NULL))
    return TMWDEFS_FALSE;
#endif
#if MDNPDATA_SUPPORT_OBJ120   
  if(!tmwmem_lowInit(_mdnpmemAllocTable, MDNPMEM_SIM_AUTH_USER_TYPE,  pConfig->numSecureAuthUsers,sizeof(MDNPMEM_SIM_AUTHUSER),       TMWDEFS_NULL))
    return TMWDEFS_FALSE;
#endif
#endif

#else
  /* static memory allocation supported */
  TMWTARG_UNUSED_PARAM(pConfig);
  if(!tmwmem_lowInit(_mdnpmemAllocTable, MDNPMEM_MDNPSESN_TYPE,      MDNPCNFG_NUMALLOC_SESNS,          sizeof(MDNPMEM_MDNPSESN),        (TMWTYPES_UCHAR*)mdnpmem_mSesns))
    return TMWDEFS_FALSE;
  if(!tmwmem_lowInit(_mdnpmemAllocTable, MDNPMEM_OPER_CALLBACK_TYPE, MDNPCNFG_NUMALLOC_OPERCBACKDATAS, sizeof(MDNPMEM_OPER_CALLBACK),   (TMWTYPES_UCHAR*)mdnpmem_operCbackDatas))
    return TMWDEFS_FALSE;

#if MDNPDATA_SUPPORT_OBJ70
  if(!tmwmem_lowInit(_mdnpmemAllocTable, MDNPMEM_FILE_TRANSFER_TYPE, MDNPCNFG_NUMALLOC_FILECBACKDATAS, sizeof(MDNPMEM_FILE_CALLBACK),   (TMWTYPES_UCHAR*)mdnpmem_fileCbackDatas))
    return TMWDEFS_FALSE;
#endif

#if MDNPDATA_SUPPORT_DATASETS
  if(!tmwmem_lowInit(_mdnpmemAllocTable, MDNPMEM_DATASET_CONTEXT_TYPE, MDNPCNFG_NUMALLOC_DATASET_CTXS, sizeof(MDNPMEM_DATASET_CONTEXT), (TMWTYPES_UCHAR*)mdnpmem_datasetContexts))
    return TMWDEFS_FALSE;
#endif
 
#if MDNPDATA_SUPPORT_OBJ120
  if(!tmwmem_lowInit(_mdnpmemAllocTable, MDNPMEM_AUTH_INFO_TYPE,     MDNPCNFG_NUMALLOC_SESNS,          sizeof(MDNPMEM_AUTH_INFO),       (TMWTYPES_UCHAR*)mdnpmem_authInfos))
    return TMWDEFS_FALSE;
#if MDNPCNFG_SUPPORT_SA_VERSION5
  if(!tmwmem_lowInit(_mdnpmemAllocTable, MDNPMEM_AUTH_USER_TYPE,     MDNPCNFG_NUMALLOC_AUTH_USERS,     sizeof(MDNPMEM_AUTH_USER),       (TMWTYPES_UCHAR*)mdnpmem_authUsers))
    return TMWDEFS_FALSE;
  if(!tmwmem_lowInit(_mdnpmemAllocTable, MDNPMEM_AUTH_EVENT_TYPE,    MDNPCNFG_NUMALLOC_AUTH_EVENTS,    sizeof(MDNPMEM_AUTH_EVENT),      (TMWTYPES_UCHAR*)mdnpmem_authEvents))
    return TMWDEFS_FALSE;
#endif
#endif

#if TMWCNFG_USE_SIMULATED_DB
  if(!tmwmem_lowInit(_mdnpmemAllocTable, MDNPMEM_SIM_DATABASE_TYPE,  MDNPCNFG_NUMALLOC_SIM_DATABASES,  sizeof(MDNPMEM_SIM_DATABASE),    (TMWTYPES_UCHAR*)mdnpmem_simDbases))
    return TMWDEFS_FALSE;
#if MDNPDATA_SUPPORT_DATASETS
  if(!tmwmem_lowInit(_mdnpmemAllocTable, MDNPMEM_SIM_DSET_PROTO_TYPE, MDNPCNFG_NUMALLOC_SIM_DATASETS,  sizeof(MDNPMEM_SIM_DATASET_PROTO),(TMWTYPES_UCHAR *)mdnpmem_simDatasetProtos))
    return TMWDEFS_FALSE;
  if(!tmwmem_lowInit(_mdnpmemAllocTable, MDNPMEM_SIM_DSET_DESCR_TYPE, MDNPCNFG_NUMALLOC_SIM_DATASETS,  sizeof(MDNPMEM_SIM_DATASET_DESCR),(TMWTYPES_UCHAR *)mdnpmem_simDatasetDescrs))
    return TMWDEFS_FALSE;
#endif
#if MDNPDATA_SUPPORT_OBJ120   
  if(!tmwmem_lowInit(_mdnpmemAllocTable, MDNPMEM_SIM_AUTH_USER_TYPE,  MDNPCNFG_NUMALLOC_AUTH_USERS,    sizeof(MDNPMEM_SIM_AUTHUSER),     (TMWTYPES_UCHAR *) mdnpmem_simAuthUsers))
    return TMWDEFS_FALSE;
#endif
#endif

#endif
  return TMWDEFS_TRUE;
}

/* function: mdnpmem_alloc */
void * TMWDEFS_GLOBAL mdnpmem_alloc(
  MDNPMEM_ALLOC_TYPE type)
{
  if ((type >= MDNPMEM_ALLOC_TYPE_MAX)
    || (type < 0))
  {
    return(TMWDEFS_NULL);
  }

  return(tmwmem_lowAlloc(&_mdnpmemAllocTable[type]));
}

/* function: mdnpmem_free */
void TMWDEFS_GLOBAL mdnpmem_free(
  void *pBuf)
{    
  TMWMEM_HEADER *pHeader = TMWMEM_GETHEADER(pBuf);
  TMWTYPES_UCHAR   type = TMWMEM_GETTYPE(pHeader);

  if(type >= MDNPMEM_ALLOC_TYPE_MAX)
  {
    return;
  }

  tmwmem_lowFree(&_mdnpmemAllocTable[type], pHeader);
}

/* function: mdnpmem_getUsage */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpmem_getUsage(
  TMWTYPES_UCHAR index,
  const TMWTYPES_CHAR **pName,
  TMWMEM_POOL_STRUCT *pStruct)
{
  if(index >= MDNPMEM_ALLOC_TYPE_MAX)
  {
    return(TMWDEFS_FALSE);
  }

  *pName = _nameTable[index];
  *pStruct = _mdnpmemAllocTable[index];
  return(TMWDEFS_TRUE);
}

