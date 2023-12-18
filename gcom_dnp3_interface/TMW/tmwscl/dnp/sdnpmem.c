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

/* file: sdnpmem.c
* description: IEC 60870-5-103 slave utilities.
*/
#include "tmwscl/dnp/dnpdiag.h"
#include "tmwscl/dnp/dnpcnfg.h"
#include "tmwscl/dnp/sdnpmem.h"
#include "tmwscl/dnp/dnpmem.h"
#include "tmwscl/dnp/sdnpdata.h"
#include "tmwscl/dnp/sdnpdiag.h"
#include "tmwscl/dnp/sdnpsesn.h"
#include "tmwscl/dnp/sdnpo002.h"
#include "tmwscl/dnp/sdnpo004.h"
#include "tmwscl/dnp/sdnpo011.h"
#include "tmwscl/dnp/sdnpo013.h"
#include "tmwscl/dnp/sdnpo022.h"
#include "tmwscl/dnp/sdnpo023.h"
#include "tmwscl/dnp/sdnpo032.h"
#include "tmwscl/dnp/sdnpo042.h"
#include "tmwscl/dnp/sdnpo043.h"
#include "tmwscl/dnp/sdnpo070.h"
#include "tmwscl/dnp/sdnpo111.h"
#include "tmwscl/dnp/sdnpo113.h"
#if TMWCNFG_USE_SIMULATED_DB
#include "tmwscl/dnp/sdnpsim.h"
#endif
#if SDNPDATA_SUPPORT_OBJ120
#include "tmwscl/dnp/sdnpauth.h"
#endif

/* Memory allocation structures */

typedef struct{
  TMWMEM_HEADER               header;
  SDNPSESN                    databuf;
} SDNPMEM_SDNPSESN;

typedef struct{
  TMWMEM_HEADER               header;
  TMWTYPES_UCHAR              databuf[DNPCNFG_MAX_RX_FRAGMENT_LENGTH];
} SDNPMEM_ASDU_BUFFER;

#if SDNPDATA_SUPPORT_OBJ2
typedef struct{
  TMWMEM_HEADER               header;
  SDNPEVNT_O002_EVENT         databuf;
} SDNPMEM_OBJECT2_EVENT;
#endif

#if SDNPDATA_SUPPORT_OBJ4
typedef struct{
  TMWMEM_HEADER               header;
  SDNPEVNT_O004_EVENT         databuf;
} SDNPMEM_OBJECT4_EVENT;
#endif

#if SDNPDATA_SUPPORT_OBJ11
typedef struct{
  TMWMEM_HEADER               header;
  SDNPEVNT_O011_EVENT         databuf;
} SDNPMEM_OBJECT11_EVENT;
#endif

#if SDNPDATA_SUPPORT_OBJ13
typedef struct{
  TMWMEM_HEADER               header;
  SDNPEVNT_O013_EVENT         databuf;
} SDNPMEM_OBJECT13_EVENT;
#endif

#if SDNPDATA_SUPPORT_OBJ22
typedef struct{
  TMWMEM_HEADER               header;
  SDNPEVNT_O022_EVENT         databuf;
} SDNPMEM_OBJECT22_EVENT;
#endif

#if SDNPDATA_SUPPORT_OBJ23
typedef struct{
  TMWMEM_HEADER               header;
  SDNPEVNT_O023_EVENT         databuf;
} SDNPMEM_OBJECT23_EVENT;
#endif

#if SDNPDATA_SUPPORT_OBJ32
typedef struct{
  TMWMEM_HEADER               header;
  SDNPEVNT_O032_EVENT         databuf;
} SDNPMEM_OBJECT32_EVENT;
#endif

#if SDNPDATA_SUPPORT_OBJ33
typedef struct{
  TMWMEM_HEADER               header;
  SDNPEVNT_O033_EVENT         databuf;
} SDNPMEM_OBJECT33_EVENT;
#endif

#if SDNPDATA_SUPPORT_OBJ42
typedef struct{
  TMWMEM_HEADER               header;
  SDNPEVNT_O042_EVENT         databuf;
} SDNPMEM_OBJECT42_EVENT;
#endif

#if SDNPDATA_SUPPORT_OBJ43
typedef struct{
  TMWMEM_HEADER               header;
  SDNPEVNT_O043_EVENT         databuf;
} SDNPMEM_OBJECT43_EVENT;
#endif

#if SDNPDATA_SUPPORT_OBJ70
typedef struct{
  TMWMEM_HEADER               header;
  SDNPO070_FILE_CONTROL_BLOCK databuf;
} SDNPMEM_OBJECT70_CONTROL;
#endif

#if SDNPDATA_SUPPORT_OBJ88
typedef struct{
  TMWMEM_HEADER               header;
  SDNPEVNT_O088_EVENT         databuf;
} SDNPMEM_OBJECT88_EVENT;
#endif

#if SDNPDATA_SUPPORT_OBJ111
typedef struct{
  TMWMEM_HEADER               header;
  SDNPEVNT_O111_EVENT         databuf;
} SDNPMEM_OBJECT111_EVENT;
#endif

#if SDNPDATA_SUPPORT_OBJ113
typedef struct{
  TMWMEM_HEADER               header;
  SDNPEVNT_O113_EVENT         databuf;
} SDNPMEM_OBJECT113_EVENT;
#endif

#if SDNPDATA_SUPPORT_OBJ115
typedef struct{
  TMWMEM_HEADER               header;
  SDNPEVNT_O115_EVENT         databuf;
} SDNPMEM_OBJECT115_EVENT;
#endif

#if SDNPDATA_SUPPORT_OBJ120
typedef struct{
  TMWMEM_HEADER               header;
  SDNPAUTH_INFO               databuf;
} SDNPMEM_AUTH_INFO;

typedef struct{
  TMWMEM_HEADER               header;
  SDNPEVNT_0120_EVENT         databuf;
} SDNPMEM_OBJECT120_EVENT;

typedef struct{
  TMWMEM_HEADER               header;
  SDNPEVNT_O122_EVENT         databuf;
} SDNPMEM_OBJECT122_EVENT;

#if SDNPCNFG_SUPPORT_SA_VERSION5
typedef struct{
  TMWMEM_HEADER               header;
  SDNPAUTH_USER               databuf;
} SDNPMEM_AUTH_USER;
#endif
#endif

#if TMWCNFG_USE_SIMULATED_DB
typedef struct{
  TMWMEM_HEADER               header;
  SDNPSIM_DATABASE            databuf;
} SDNPMEM_SIM_DATABASE;

#if SDNPCNFG_USER_MANAGED_EVENTS
typedef struct{
  TMWMEM_HEADER               header;
  SDNPSIM_EVENT               databuf;
} SDNPMEM_SIM_UMEVENT;
#endif

#if SDNPDATA_SUPPORT_DATASETS
typedef struct{
  TMWMEM_HEADER               header;
  SDNPSIM_DATASET_PROTO       databuf;
} SDNPMEM_SIM_DATASET_PROTO;

typedef struct{
  TMWMEM_HEADER               header;
  SDNPSIM_DATASET_DESCR_DATA  databuf;
} SDNPMEM_SIM_DATASET_DESCR;

#endif

#if SDNPDATA_SUPPORT_OBJ120
typedef struct{
  TMWMEM_HEADER               header;
  SDNPSIM_AUTHERROR           databuf;
} SDNPMEM_SIM_AUTHERROR;

typedef struct{
  TMWMEM_HEADER               header;
  SDNPSIM_AUTHUSER            databuf;
} SDNPMEM_SIM_AUTHUSER;
 
#endif
#endif

#if SDNPDATA_SUPPORT_XML2
typedef struct{
  TMWMEM_HEADER               header;
  union {
    SDNPDATA_XML_DEVICECONFIG      device;
    SDNPDATA_XML_SERIALCONFIG      serial;
    SDNPDATA_XML_NETWORKCONFIG     network;
    SDNPDATA_XML_OUTSTATIONCONFIG  outstation;
    SDNPDATA_XML_OUTSTATIONPERFORM oustationPerform;
    SDNPDATA_XML_SECURITYCONFIG    security;
  } databuf;
} SDNPMEM_XML_CONFIG;
#endif

static const TMWTYPES_CHAR *_nameTable[SDNPMEM_ALLOC_TYPE_MAX] = {
  "SDNPSESN",
  "SDNPMEM_ASDU_BUFFER"
#if SDNPDATA_SUPPORT_OBJ2
  , "SDNPO002_OBJECT_EVENT"
#endif
#if SDNPDATA_SUPPORT_OBJ4
  , "SDNPO004_OBJECT_EVENT"
#endif
#if SDNPDATA_SUPPORT_OBJ11
  , "SDNPO011_OBJECT_EVENT"
#endif
#if SDNPDATA_SUPPORT_OBJ13
  , "SDNPO013_OBJECT_EVENT"
#endif
#if SDNPDATA_SUPPORT_OBJ22
  , "SDNPO022_OBJECT_EVENT"
#endif
#if SDNPDATA_SUPPORT_OBJ23
  , "SDNPO023_OBJECT_EVENT"
#endif
#if SDNPDATA_SUPPORT_OBJ32
  , "SDNPO032_OBJECT_EVENT"
#endif
#if SDNPDATA_SUPPORT_OBJ33
  , "SDNPO033_OBJECT_EVENT"
#endif
#if SDNPDATA_SUPPORT_OBJ42
  , "SDNPO042_OBJECT_EVENT"
#endif
#if SDNPDATA_SUPPORT_OBJ43
  , "SDNPO043_OBJECT_EVENT"
#endif
#if SDNPDATA_SUPPORT_OBJ70
  , "SDNPO070_FILE_CONTROL_BLOCK"
#endif
#if SDNPDATA_SUPPORT_OBJ88
  , "SDNPO088_OBJECT_EVENT"
#endif
#if SDNPDATA_SUPPORT_OBJ111
  , "SDNPO111_OBJECT_EVENT"
#endif
#if SDNPDATA_SUPPORT_OBJ113
  , "SDNPO113_OBJECT_EVENT"
#endif
#if SDNPDATA_SUPPORT_OBJ115
  , "SDNPO115_OBJECT_EVENT"
#endif
#if SDNPDATA_SUPPORT_OBJ120
  ,"SDNPO120_OBJECT_EVENT" 
  ,"SDNPO122_OBJECT_EVENT" 
  ,"SDNPAUTH_INFO" 
#if SDNPCNFG_SUPPORT_SA_VERSION5
  ,"SDNPAUTH_USER" 
#endif
#endif

#if TMWCNFG_USE_SIMULATED_DB
  , "SDNPSIM_DATABASE"
#if SDNPCNFG_USER_MANAGED_EVENTS
  , "SDNPSIM_UMEVENT"
#endif
#if SDNPDATA_SUPPORT_DATASETS
  , "SDNPSIM_DATASET_PROTO" 
  , "SDNPSIM_DATASET_DESCR" 
#endif
#if SDNPDATA_SUPPORT_OBJ120
  , "SDNPSIM_AUTH_ERROR"
  , "SDNPSIM_AUTH_USER"
#endif
#endif
#if SDNPDATA_SUPPORT_XML2
  , "SDNPXML_CONFIG_TYPE"  
#endif
};

#define NUM_STRINGS_PER_DATASET 10
  
#if !TMWCNFG_USE_DYNAMIC_MEMORY
/* Use static allocated array of memory instead of dynamic memory */

static SDNPMEM_SDNPSESN         sdnpmem_sSesns[SDNPCNFG_NUMALLOC_SESNS];
static SDNPMEM_ASDU_BUFFER      sdnpmem_asduBuffers[SDNPCNFG_NUMALLOC_ASDU_BUFFERS];
#if SDNPDATA_SUPPORT_OBJ2
static SDNPMEM_OBJECT2_EVENT    sdnpmem_object2Events[SDNPCNFG_NUMALLOC_OBJECT2_EVENTS];
#endif
#if SDNPDATA_SUPPORT_OBJ4
static SDNPMEM_OBJECT4_EVENT    sdnpmem_object4Events[SDNPCNFG_NUMALLOC_OBJECT4_EVENTS];
#endif
#if SDNPDATA_SUPPORT_OBJ11
static SDNPMEM_OBJECT11_EVENT    sdnpmem_object11Events[SDNPCNFG_NUMALLOC_OBJECT11_EVENTS];
#endif
#if SDNPDATA_SUPPORT_OBJ13
static SDNPMEM_OBJECT13_EVENT    sdnpmem_object13Events[SDNPCNFG_NUMALLOC_OBJECT13_EVENTS];
#endif
#if SDNPDATA_SUPPORT_OBJ22
static SDNPMEM_OBJECT22_EVENT   sdnpmem_object22Events[SDNPCNFG_NUMALLOC_OBJECT22_EVENTS];
#endif
#if SDNPDATA_SUPPORT_OBJ23
static SDNPMEM_OBJECT23_EVENT   sdnpmem_object23Events[SDNPCNFG_NUMALLOC_OBJECT23_EVENTS];
#endif
#if SDNPDATA_SUPPORT_OBJ32
static SDNPMEM_OBJECT32_EVENT   sdnpmem_object32Events[SDNPCNFG_NUMALLOC_OBJECT32_EVENTS];
#endif
#if SDNPDATA_SUPPORT_OBJ33
static SDNPMEM_OBJECT33_EVENT   sdnpmem_object33Events[SDNPCNFG_NUMALLOC_OBJECT33_EVENTS];
#endif
#if SDNPDATA_SUPPORT_OBJ42
static SDNPMEM_OBJECT42_EVENT    sdnpmem_object42Events[SDNPCNFG_NUMALLOC_OBJECT42_EVENTS];
#endif
#if SDNPDATA_SUPPORT_OBJ43
static SDNPMEM_OBJECT43_EVENT    sdnpmem_object43Events[SDNPCNFG_NUMALLOC_OBJECT43_EVENTS];
#endif
#if SDNPDATA_SUPPORT_OBJ70
static SDNPMEM_OBJECT70_CONTROL sdnpmem_object70CtrlBlocks[SDNPCNFG_NUMALLOC_OBJECT70_BLOCKS];
#endif
#if SDNPDATA_SUPPORT_OBJ88
static SDNPMEM_OBJECT88_EVENT   sdnpmem_object88EventBlocks[SDNPCNFG_NUMALLOC_OBJECT88_EVENTS];
#endif
#if SDNPDATA_SUPPORT_OBJ111
static SDNPMEM_OBJECT111_EVENT  sdnpmem_object111Events[SDNPCNFG_NUMALLOC_OBJECT111_EVENTS];
#endif
#if SDNPDATA_SUPPORT_OBJ113
static SDNPMEM_OBJECT113_EVENT  sdnpmem_object113Events[SDNPCNFG_NUMALLOC_OBJECT113_EVENTS];
#endif
#if SDNPDATA_SUPPORT_OBJ115
static SDNPMEM_OBJECT115_EVENT  sdnpmem_object115Events[SDNPCNFG_NUMALLOC_OBJECT115_EVENTS];
#endif
#if SDNPDATA_SUPPORT_OBJ120
static SDNPMEM_OBJECT120_EVENT  sdnpmem_object120Events[SDNPCNFG_NUMALLOC_OBJECT120_EVENTS];
static SDNPMEM_OBJECT122_EVENT  sdnpmem_object122Events[SDNPCNFG_NUMALLOC_OBJECT120_EVENTS];
static SDNPMEM_AUTH_INFO        sdnpmem_authInfos[SDNPCNFG_NUMALLOC_SESNS];
#if SDNPCNFG_SUPPORT_SA_VERSION5
static SDNPMEM_AUTH_USER        sdnpmem_authUsers[SDNPCNFG_NUMALLOC_AUTH_USERS];
#endif
#endif
#if TMWCNFG_USE_SIMULATED_DB
static SDNPMEM_SIM_DATABASE     sdnpmem_simDbases[SDNPCNFG_NUMALLOC_SIM_DATABASES];
#if SDNPCNFG_USER_MANAGED_EVENTS
static SDNPMEM_SIM_UMEVENT      sdnpmem_simUMEvents[SDNPCNFG_NUMALLOC_SIM_UMEVENTS];
#endif
#if SDNPDATA_SUPPORT_DATASETS
static SDNPMEM_SIM_DATASET_PROTO  sdnpmem_simDatasetProtos[SDNPCNFG_NUMALLOC_SIM_DATASETS];
static SDNPMEM_SIM_DATASET_DESCR  sdnpmem_simDatasetDescrs[SDNPCNFG_NUMALLOC_SIM_DATASETS];
#endif
#if SDNPDATA_SUPPORT_OBJ120
static SDNPMEM_SIM_AUTHERROR     sdnpmem_simAuthErrors[(SDNPCNFG_NUMALLOC_AUTH_USERS*10)]; 
static SDNPMEM_SIM_AUTHUSER      sdnpmem_simAuthUsers[SDNPCNFG_NUMALLOC_AUTH_USERS]; 
#endif
 
#endif

#if SDNPDATA_SUPPORT_XML2
static SDNPMEM_XML_CONFIG  sdnpmem_xmlConfigs[SDNPCNFG_NUMALLOC_DEVICE_PROFILES]; 
#endif

#endif

static TMWMEM_POOL_STRUCT _sdnpmemAllocTable[SDNPMEM_ALLOC_TYPE_MAX];

#if TMWCNFG_USE_DYNAMIC_MEMORY
static void TMWDEFS_LOCAL _initConfig(
  SDNPMEM_CONFIG *pConfig)
{
  pConfig->numSessions        = SDNPCNFG_NUMALLOC_SESNS;
  pConfig->numASDUBuffers     = SDNPCNFG_NUMALLOC_ASDU_BUFFERS;
#if SDNPDATA_SUPPORT_OBJ2
  pConfig->numObject2Events   = SDNPCNFG_NUMALLOC_OBJECT2_EVENTS;
#endif
#if SDNPDATA_SUPPORT_OBJ4
  pConfig->numObject4Events   = SDNPCNFG_NUMALLOC_OBJECT4_EVENTS;
#endif
#if SDNPDATA_SUPPORT_OBJ11
  pConfig->numObject11Events  = SDNPCNFG_NUMALLOC_OBJECT11_EVENTS;
#endif
#if SDNPDATA_SUPPORT_OBJ13
  pConfig->numObject13Events  = SDNPCNFG_NUMALLOC_OBJECT13_EVENTS;
#endif
#if SDNPDATA_SUPPORT_OBJ22
  pConfig->numObject22Events  = SDNPCNFG_NUMALLOC_OBJECT22_EVENTS;
#endif
#if SDNPDATA_SUPPORT_OBJ23
  pConfig->numObject23Events  = SDNPCNFG_NUMALLOC_OBJECT23_EVENTS;
#endif
#if SDNPDATA_SUPPORT_OBJ32
  pConfig->numObject32Events  = SDNPCNFG_NUMALLOC_OBJECT32_EVENTS;
#endif
#if SDNPDATA_SUPPORT_OBJ33
  pConfig->numObject33Events  = SDNPCNFG_NUMALLOC_OBJECT33_EVENTS;
#endif
#if SDNPDATA_SUPPORT_OBJ42
  pConfig->numObject42Events  = SDNPCNFG_NUMALLOC_OBJECT42_EVENTS;
#endif
#if SDNPDATA_SUPPORT_OBJ43
  pConfig->numObject43Events  = SDNPCNFG_NUMALLOC_OBJECT43_EVENTS;
#endif
#if SDNPDATA_SUPPORT_OBJ70
  pConfig->numObject70Events  = SDNPCNFG_NUMALLOC_OBJECT70_BLOCKS;
#endif
#if SDNPDATA_SUPPORT_OBJ88
  pConfig->numObject88Events  = SDNPCNFG_NUMALLOC_OBJECT88_EVENTS;
#endif
#if SDNPDATA_SUPPORT_OBJ111
  pConfig->numObject111Events = SDNPCNFG_NUMALLOC_OBJECT111_EVENTS;
#endif
#if SDNPDATA_SUPPORT_OBJ113
  pConfig->numObject113Events = SDNPCNFG_NUMALLOC_OBJECT113_EVENTS;
#endif
#if SDNPDATA_SUPPORT_OBJ115
  pConfig->numObject115Events = SDNPCNFG_NUMALLOC_OBJECT115_EVENTS;
#endif
#if SDNPDATA_SUPPORT_OBJ120
  pConfig->numObject120Events  = SDNPCNFG_NUMALLOC_OBJECT120_EVENTS; 
  pConfig->numObject122Events  = SDNPCNFG_NUMALLOC_OBJECT122_EVENTS; 
  pConfig->numSecureAuthUsers  = SDNPCNFG_NUMALLOC_AUTH_USERS; 
#endif 
#if TMWCNFG_USE_SIMULATED_DB
  pConfig->numSimDbases       = SDNPCNFG_NUMALLOC_SIM_DATABASES;
#if SDNPCNFG_USER_MANAGED_EVENTS
  pConfig->numSimUMEvents     = SDNPCNFG_NUMALLOC_SIM_UMEVENTS;
#endif
#if SDNPDATA_SUPPORT_DATASETS
  pConfig->numSimDatasets     = SDNPCNFG_NUMALLOC_SIM_DATASETS;
#endif
#endif
#if SDNPDATA_SUPPORT_XML2 
  pConfig->numDeviceProfiles  = SDNPCNFG_NUMALLOC_DEVICE_PROFILES;
#endif
}

void TMWDEFS_GLOBAL sdnpmem_initConfig(
  SDNPMEM_CONFIG *pConfig,
  DNPMEM_CONFIG  *pDnpConfig,
  TMWMEM_CONFIG  *pTmwConfig)
{
  _initConfig(pConfig);
  dnpmem_initConfig(pDnpConfig);
  tmwmem_initConfig(pTmwConfig);
}

/* function: sdnpmem_initMemory */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpmem_initMemory(
  SDNPMEM_CONFIG *pSdnpConfig,
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

  /* Now for SDNP if not yet done */
  if(!tmwappl_getInitialized(TMWAPPL_INIT_SDNP))
  {
    if(!sdnpmem_init(pSdnpConfig))
      return TMWDEFS_FALSE;

#if TMWCNFG_SUPPORT_DIAG
    sdnpdiag_init();
#endif
    tmwappl_setInitialized(TMWAPPL_INIT_SDNP);
  }
  return TMWDEFS_TRUE;
}
#endif

/* routine: sdnpmem_init */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpmem_init(
  SDNPMEM_CONFIG *pConfig)
{
#if TMWCNFG_USE_DYNAMIC_MEMORY
  /* dynamic memory allocation supported */
  SDNPMEM_CONFIG  config; 

  /* If caller has not specified memory pool configuration, use the
   * default compile time values 
   */
  if(pConfig == TMWDEFS_NULL)
  {
    pConfig = &config;
    _initConfig(pConfig);
  }

  if(!tmwmem_lowInit(_sdnpmemAllocTable, SDNPMEM_SDNPSESN_TYPE,        pConfig->numSessions,        sizeof(SDNPMEM_SDNPSESN),        TMWDEFS_NULL))
    return TMWDEFS_FALSE;
  if(!tmwmem_lowInit(_sdnpmemAllocTable, SDNPMEM_ASDU_BUFFER_TYPE,     pConfig->numASDUBuffers,     sizeof(SDNPMEM_ASDU_BUFFER),     TMWDEFS_NULL))
    return TMWDEFS_FALSE;
#if SDNPDATA_SUPPORT_OBJ2
  if(!tmwmem_lowInit(_sdnpmemAllocTable, SDNPMEM_OBJECT2_EVENT_TYPE,   pConfig->numObject2Events,   sizeof(SDNPMEM_OBJECT2_EVENT),   TMWDEFS_NULL))
    return TMWDEFS_FALSE;
#endif
  #if SDNPDATA_SUPPORT_OBJ4
  if(!tmwmem_lowInit(_sdnpmemAllocTable, SDNPMEM_OBJECT4_EVENT_TYPE,   pConfig->numObject4Events,   sizeof(SDNPMEM_OBJECT4_EVENT),   TMWDEFS_NULL))
    return TMWDEFS_FALSE;
#endif
#if SDNPDATA_SUPPORT_OBJ11
  if(!tmwmem_lowInit(_sdnpmemAllocTable, SDNPMEM_OBJECT11_EVENT_TYPE,  pConfig->numObject11Events,  sizeof(SDNPMEM_OBJECT11_EVENT),  TMWDEFS_NULL))
    return TMWDEFS_FALSE;
#endif
#if SDNPDATA_SUPPORT_OBJ13
  if(!tmwmem_lowInit(_sdnpmemAllocTable, SDNPMEM_OBJECT13_EVENT_TYPE,  pConfig->numObject13Events,  sizeof(SDNPMEM_OBJECT13_EVENT),  TMWDEFS_NULL))
    return TMWDEFS_FALSE;
#endif
#if SDNPDATA_SUPPORT_OBJ22
  if(!tmwmem_lowInit(_sdnpmemAllocTable, SDNPMEM_OBJECT22_EVENT_TYPE,  pConfig->numObject22Events,  sizeof(SDNPMEM_OBJECT22_EVENT),  TMWDEFS_NULL))
    return TMWDEFS_FALSE;
#endif
#if SDNPDATA_SUPPORT_OBJ23
  if(!tmwmem_lowInit(_sdnpmemAllocTable, SDNPMEM_OBJECT23_EVENT_TYPE,  pConfig->numObject23Events,  sizeof(SDNPMEM_OBJECT23_EVENT),  TMWDEFS_NULL))
    return TMWDEFS_FALSE;
#endif
#if SDNPDATA_SUPPORT_OBJ32
  if(!tmwmem_lowInit(_sdnpmemAllocTable, SDNPMEM_OBJECT32_EVENT_TYPE,  pConfig->numObject32Events,  sizeof(SDNPMEM_OBJECT32_EVENT),  TMWDEFS_NULL))
    return TMWDEFS_FALSE;
#endif
  #if SDNPDATA_SUPPORT_OBJ33
  if(!tmwmem_lowInit(_sdnpmemAllocTable, SDNPMEM_OBJECT33_EVENT_TYPE,  pConfig->numObject33Events,  sizeof(SDNPMEM_OBJECT33_EVENT),  TMWDEFS_NULL))
    return TMWDEFS_FALSE;
#endif
#if SDNPDATA_SUPPORT_OBJ42
  if(!tmwmem_lowInit(_sdnpmemAllocTable, SDNPMEM_OBJECT42_EVENT_TYPE,  pConfig->numObject42Events,  sizeof(SDNPMEM_OBJECT42_EVENT),  TMWDEFS_NULL))
    return TMWDEFS_FALSE;
#endif
#if SDNPDATA_SUPPORT_OBJ43
  if(!tmwmem_lowInit(_sdnpmemAllocTable, SDNPMEM_OBJECT43_EVENT_TYPE,  pConfig->numObject43Events,  sizeof(SDNPMEM_OBJECT43_EVENT),  TMWDEFS_NULL))
    return TMWDEFS_FALSE;
#endif
#if SDNPDATA_SUPPORT_OBJ70
  if(!tmwmem_lowInit(_sdnpmemAllocTable, SDNPMEM_OBJECT70_CTRL_TYPE,   pConfig->numObject70Events,  sizeof(SDNPMEM_OBJECT70_CONTROL),TMWDEFS_NULL))
    return TMWDEFS_FALSE;
#endif
#if SDNPDATA_SUPPORT_OBJ88
  if(!tmwmem_lowInit(_sdnpmemAllocTable, SDNPMEM_OBJECT88_EVENT_TYPE,  pConfig->numObject88Events,  sizeof(SDNPMEM_OBJECT88_EVENT),  TMWDEFS_NULL))
    return TMWDEFS_FALSE;
#endif
#if SDNPDATA_SUPPORT_OBJ111
  if(!tmwmem_lowInit(_sdnpmemAllocTable, SDNPMEM_OBJECT111_EVENT_TYPE, pConfig->numObject111Events, sizeof(SDNPMEM_OBJECT111_EVENT), TMWDEFS_NULL))
    return TMWDEFS_FALSE;
#endif
#if SDNPDATA_SUPPORT_OBJ113
  if(!tmwmem_lowInit(_sdnpmemAllocTable, SDNPMEM_OBJECT113_EVENT_TYPE, pConfig->numObject113Events, sizeof(SDNPMEM_OBJECT113_EVENT), TMWDEFS_NULL))
    return TMWDEFS_FALSE;
#endif
#if SDNPDATA_SUPPORT_OBJ115
  if(!tmwmem_lowInit(_sdnpmemAllocTable, SDNPMEM_OBJECT115_EVENT_TYPE, pConfig->numObject115Events, sizeof(SDNPMEM_OBJECT115_EVENT), TMWDEFS_NULL))
    return TMWDEFS_FALSE;
#endif
#if SDNPDATA_SUPPORT_OBJ120   
  if(!tmwmem_lowInit(_sdnpmemAllocTable, SDNPMEM_OBJECT120_EVENT_TYPE, pConfig->numObject120Events, sizeof(SDNPMEM_OBJECT120_EVENT),TMWDEFS_NULL))
    return TMWDEFS_FALSE;
  if(!tmwmem_lowInit(_sdnpmemAllocTable, SDNPMEM_OBJECT122_EVENT_TYPE, pConfig->numObject122Events, sizeof(SDNPMEM_OBJECT122_EVENT),TMWDEFS_NULL))
    return TMWDEFS_FALSE;
  if(!tmwmem_lowInit(_sdnpmemAllocTable, SDNPMEM_AUTH_INFO_TYPE,       pConfig->numSessions,        sizeof(SDNPMEM_AUTH_INFO),       TMWDEFS_NULL))
    return TMWDEFS_FALSE;
#if SDNPCNFG_SUPPORT_SA_VERSION5
  if(!tmwmem_lowInit(_sdnpmemAllocTable, SDNPMEM_AUTH_USER_TYPE,       pConfig->numSecureAuthUsers, sizeof(SDNPMEM_AUTH_USER),       TMWDEFS_NULL))
    return TMWDEFS_FALSE;
#endif
#endif
#if TMWCNFG_USE_SIMULATED_DB
  if(!tmwmem_lowInit(_sdnpmemAllocTable, SDNPMEM_SIM_DATABASE_TYPE,    pConfig->numSimDbases,       sizeof(SDNPMEM_SIM_DATABASE),    TMWDEFS_NULL))
    return TMWDEFS_FALSE;
#if SDNPCNFG_USER_MANAGED_EVENTS
  if(!tmwmem_lowInit(_sdnpmemAllocTable, SDNPMEM_SIM_UMEVENT_TYPE,     pConfig->numSimUMEvents,     sizeof(SDNPMEM_SIM_UMEVENT),     TMWDEFS_NULL))
    return TMWDEFS_FALSE;
#endif
#if SDNPDATA_SUPPORT_DATASETS
  if(!tmwmem_lowInit(_sdnpmemAllocTable, SDNPMEM_SIM_DSET_PROTO_TYPE,  pConfig->numSimDatasets,    sizeof(SDNPMEM_SIM_DATASET_PROTO), TMWDEFS_NULL))
    return TMWDEFS_FALSE;
  if(!tmwmem_lowInit(_sdnpmemAllocTable, SDNPMEM_SIM_DSET_DESCR_TYPE,  pConfig->numSimDatasets,    sizeof(SDNPMEM_SIM_DATASET_DESCR), TMWDEFS_NULL))
    return TMWDEFS_FALSE;
#endif
#if SDNPDATA_SUPPORT_OBJ120    
  if(!tmwmem_lowInit(_sdnpmemAllocTable, SDNPMEM_SIM_AUTH_ERROR_TYPE,(pConfig->numSecureAuthUsers*10),sizeof(SDNPMEM_SIM_AUTHERROR),  TMWDEFS_NULL))
    return TMWDEFS_FALSE; 
  if(!tmwmem_lowInit(_sdnpmemAllocTable, SDNPMEM_SIM_AUTH_USER_TYPE,  pConfig->numSecureAuthUsers,    sizeof(SDNPMEM_SIM_AUTHUSER),   TMWDEFS_NULL))
    return TMWDEFS_FALSE;
#endif
#endif
#if SDNPDATA_SUPPORT_XML2
  if(!tmwmem_lowInit(_sdnpmemAllocTable, SDNPMEM_SIM_XML_CONFIG_TYPE,      pConfig->numDeviceProfiles,  sizeof(SDNPMEM_XML_CONFIG),       TMWDEFS_NULL)) 
    return TMWDEFS_FALSE;
#endif
#else

  TMWTARG_UNUSED_PARAM(pConfig);
  if(!tmwmem_lowInit(_sdnpmemAllocTable, SDNPMEM_SDNPSESN_TYPE,        SDNPCNFG_NUMALLOC_SESNS,            sizeof(SDNPMEM_SDNPSESN),        (TMWTYPES_UCHAR *)sdnpmem_sSesns))
    return TMWDEFS_FALSE;
  if(!tmwmem_lowInit(_sdnpmemAllocTable, SDNPMEM_ASDU_BUFFER_TYPE,     SDNPCNFG_NUMALLOC_ASDU_BUFFERS,     sizeof(SDNPMEM_ASDU_BUFFER),     (TMWTYPES_UCHAR *)sdnpmem_asduBuffers))
    return TMWDEFS_FALSE;
#if SDNPDATA_SUPPORT_OBJ2
  if(!tmwmem_lowInit(_sdnpmemAllocTable, SDNPMEM_OBJECT2_EVENT_TYPE,   SDNPCNFG_NUMALLOC_OBJECT2_EVENTS,    sizeof(SDNPMEM_OBJECT2_EVENT),  (TMWTYPES_UCHAR *)sdnpmem_object2Events))
    return TMWDEFS_FALSE;
#endif
#if SDNPDATA_SUPPORT_OBJ4
  if(!tmwmem_lowInit(_sdnpmemAllocTable, SDNPMEM_OBJECT4_EVENT_TYPE,   SDNPCNFG_NUMALLOC_OBJECT4_EVENTS,   sizeof(SDNPMEM_OBJECT4_EVENT),   (TMWTYPES_UCHAR *)sdnpmem_object4Events))
    return TMWDEFS_FALSE;
#endif
#if SDNPDATA_SUPPORT_OBJ11
  if(!tmwmem_lowInit(_sdnpmemAllocTable, SDNPMEM_OBJECT11_EVENT_TYPE,  SDNPCNFG_NUMALLOC_OBJECT11_EVENTS,  sizeof(SDNPMEM_OBJECT11_EVENT),  (TMWTYPES_UCHAR *)sdnpmem_object11Events))
    return TMWDEFS_FALSE;
#endif
#if SDNPDATA_SUPPORT_OBJ13
  if(!tmwmem_lowInit(_sdnpmemAllocTable, SDNPMEM_OBJECT13_EVENT_TYPE,  SDNPCNFG_NUMALLOC_OBJECT13_EVENTS,  sizeof(SDNPMEM_OBJECT13_EVENT),  (TMWTYPES_UCHAR *)sdnpmem_object13Events))
    return TMWDEFS_FALSE;
#endif
#if SDNPDATA_SUPPORT_OBJ22
  if(!tmwmem_lowInit(_sdnpmemAllocTable, SDNPMEM_OBJECT22_EVENT_TYPE,  SDNPCNFG_NUMALLOC_OBJECT22_EVENTS,  sizeof(SDNPMEM_OBJECT22_EVENT),  (TMWTYPES_UCHAR *)sdnpmem_object22Events))
    return TMWDEFS_FALSE;
#endif
#if SDNPDATA_SUPPORT_OBJ23
  if(!tmwmem_lowInit(_sdnpmemAllocTable, SDNPMEM_OBJECT23_EVENT_TYPE,  SDNPCNFG_NUMALLOC_OBJECT23_EVENTS,  sizeof(SDNPMEM_OBJECT23_EVENT),  (TMWTYPES_UCHAR *)sdnpmem_object23Events))
    return TMWDEFS_FALSE;
#endif
#if SDNPDATA_SUPPORT_OBJ32
  if(!tmwmem_lowInit(_sdnpmemAllocTable, SDNPMEM_OBJECT32_EVENT_TYPE,  SDNPCNFG_NUMALLOC_OBJECT32_EVENTS,  sizeof(SDNPMEM_OBJECT32_EVENT),  (TMWTYPES_UCHAR *)sdnpmem_object32Events))
    return TMWDEFS_FALSE;
#endif
#if SDNPDATA_SUPPORT_OBJ33
  if(!tmwmem_lowInit(_sdnpmemAllocTable, SDNPMEM_OBJECT33_EVENT_TYPE,  SDNPCNFG_NUMALLOC_OBJECT33_EVENTS,  sizeof(SDNPMEM_OBJECT33_EVENT),  (TMWTYPES_UCHAR *)sdnpmem_object33Events))
    return TMWDEFS_FALSE;
#endif
#if SDNPDATA_SUPPORT_OBJ42
  if(!tmwmem_lowInit(_sdnpmemAllocTable, SDNPMEM_OBJECT42_EVENT_TYPE,  SDNPCNFG_NUMALLOC_OBJECT42_EVENTS,  sizeof(SDNPMEM_OBJECT42_EVENT),  (TMWTYPES_UCHAR *)sdnpmem_object42Events))
    return TMWDEFS_FALSE;
#endif
#if SDNPDATA_SUPPORT_OBJ43
  if(!tmwmem_lowInit(_sdnpmemAllocTable, SDNPMEM_OBJECT43_EVENT_TYPE,  SDNPCNFG_NUMALLOC_OBJECT43_EVENTS,  sizeof(SDNPMEM_OBJECT43_EVENT),  (TMWTYPES_UCHAR *)sdnpmem_object43Events))
    return TMWDEFS_FALSE;
#endif
#if SDNPDATA_SUPPORT_OBJ70
  if(!tmwmem_lowInit(_sdnpmemAllocTable, SDNPMEM_OBJECT70_CTRL_TYPE,   SDNPCNFG_NUMALLOC_OBJECT70_BLOCKS,  sizeof(SDNPMEM_OBJECT70_CONTROL),(TMWTYPES_UCHAR *)sdnpmem_object70CtrlBlocks))
    return TMWDEFS_FALSE;
#endif
#if SDNPDATA_SUPPORT_OBJ88
  if(!tmwmem_lowInit(_sdnpmemAllocTable, SDNPMEM_OBJECT88_EVENT_TYPE,  SDNPCNFG_NUMALLOC_OBJECT88_EVENTS,  sizeof(SDNPMEM_OBJECT88_EVENT),  (TMWTYPES_UCHAR *)sdnpmem_object88EventBlocks))
    return TMWDEFS_FALSE;
#endif
#if SDNPDATA_SUPPORT_OBJ111
  if(!tmwmem_lowInit(_sdnpmemAllocTable, SDNPMEM_OBJECT111_EVENT_TYPE, SDNPCNFG_NUMALLOC_OBJECT111_EVENTS, sizeof(SDNPMEM_OBJECT111_EVENT), (TMWTYPES_UCHAR *)sdnpmem_object111Events))
    return TMWDEFS_FALSE;
#endif
#if SDNPDATA_SUPPORT_OBJ113
  if(!tmwmem_lowInit(_sdnpmemAllocTable, SDNPMEM_OBJECT113_EVENT_TYPE, SDNPCNFG_NUMALLOC_OBJECT113_EVENTS, sizeof(SDNPMEM_OBJECT113_EVENT), (TMWTYPES_UCHAR *)sdnpmem_object113Events))
    return TMWDEFS_FALSE;
#endif
#if SDNPDATA_SUPPORT_OBJ115
  if(!tmwmem_lowInit(_sdnpmemAllocTable, SDNPMEM_OBJECT115_EVENT_TYPE, SDNPCNFG_NUMALLOC_OBJECT115_EVENTS, sizeof(SDNPMEM_OBJECT115_EVENT), (TMWTYPES_UCHAR *)sdnpmem_object115Events))
    return TMWDEFS_FALSE;
#endif
#if SDNPDATA_SUPPORT_OBJ120
  if(!tmwmem_lowInit(_sdnpmemAllocTable, SDNPMEM_OBJECT120_EVENT_TYPE, SDNPCNFG_NUMALLOC_OBJECT120_EVENTS, sizeof(SDNPMEM_OBJECT120_EVENT), (TMWTYPES_UCHAR*)sdnpmem_object120Events))
    return TMWDEFS_FALSE;
  if(!tmwmem_lowInit(_sdnpmemAllocTable, SDNPMEM_OBJECT122_EVENT_TYPE, SDNPCNFG_NUMALLOC_OBJECT122_EVENTS, sizeof(SDNPMEM_OBJECT122_EVENT), (TMWTYPES_UCHAR*)sdnpmem_object122Events))
    return TMWDEFS_FALSE;
  if(!tmwmem_lowInit(_sdnpmemAllocTable, SDNPMEM_AUTH_INFO_TYPE,       SDNPCNFG_NUMALLOC_SESNS,            sizeof(SDNPMEM_AUTH_INFO),       (TMWTYPES_UCHAR*)sdnpmem_authInfos))
    return TMWDEFS_FALSE;
#if SDNPCNFG_SUPPORT_SA_VERSION5
  if(!tmwmem_lowInit(_sdnpmemAllocTable, SDNPMEM_AUTH_USER_TYPE,       SDNPCNFG_NUMALLOC_AUTH_USERS,       sizeof(SDNPMEM_AUTH_USER),       (TMWTYPES_UCHAR*)sdnpmem_authUsers))
    return TMWDEFS_FALSE;
#endif
#endif
#if TMWCNFG_USE_SIMULATED_DB
  if(!tmwmem_lowInit(_sdnpmemAllocTable, SDNPMEM_SIM_DATABASE_TYPE,    SDNPCNFG_NUMALLOC_SIM_DATABASES,    sizeof(SDNPMEM_SIM_DATABASE),    (TMWTYPES_UCHAR *)sdnpmem_simDbases))
    return TMWDEFS_FALSE;
#if SDNPCNFG_USER_MANAGED_EVENTS
  if(!tmwmem_lowInit(_sdnpmemAllocTable, SDNPMEM_SIM_UMEVENT_TYPE,     SDNPCNFG_NUMALLOC_SIM_UMEVENTS,     sizeof(SDNPMEM_SIM_UMEVENT),     (TMWTYPES_UCHAR *)sdnpmem_simUMEvents))
    return TMWDEFS_FALSE;
#endif
#if SDNPDATA_SUPPORT_DATASETS
  if(!tmwmem_lowInit(_sdnpmemAllocTable, SDNPMEM_SIM_DSET_PROTO_TYPE,   SDNPCNFG_NUMALLOC_SIM_DATASETS,    sizeof(SDNPMEM_SIM_DATASET_PROTO), (TMWTYPES_UCHAR *)sdnpmem_simDatasetProtos))
    return TMWDEFS_FALSE;
  if(!tmwmem_lowInit(_sdnpmemAllocTable, SDNPMEM_SIM_DSET_DESCR_TYPE,   SDNPCNFG_NUMALLOC_SIM_DATASETS,    sizeof(SDNPMEM_SIM_DATASET_DESCR), (TMWTYPES_UCHAR *)sdnpmem_simDatasetDescrs))
    return TMWDEFS_FALSE;
#endif
#if SDNPDATA_SUPPORT_OBJ120
  if(!tmwmem_lowInit(_sdnpmemAllocTable, SDNPMEM_SIM_AUTH_ERROR_TYPE,  (SDNPCNFG_NUMALLOC_AUTH_USERS*10),  sizeof(SDNPMEM_SIM_AUTHERROR),   (TMWTYPES_UCHAR*)sdnpmem_simAuthErrors))
    return TMWDEFS_FALSE;
  if(!tmwmem_lowInit(_sdnpmemAllocTable, SDNPMEM_SIM_AUTH_USER_TYPE,    SDNPCNFG_NUMALLOC_AUTH_USERS,      sizeof(SDNPMEM_SIM_AUTHUSER),  (TMWTYPES_UCHAR*)sdnpmem_simAuthUsers))
    return TMWDEFS_FALSE;
#endif
#endif
#if SDNPDATA_SUPPORT_XML2 
  if(!tmwmem_lowInit(_sdnpmemAllocTable, SDNPMEM_SIM_XML_CONFIG_TYPE,   SDNPCNFG_NUMALLOC_DEVICE_PROFILES,  sizeof(SDNPMEM_XML_CONFIG),      (TMWTYPES_UCHAR*)sdnpmem_xmlConfigs))
    return TMWDEFS_FALSE;
#endif
#endif

  return TMWDEFS_TRUE;
}


/* function: sdnpmem_alloc */
void * TMWDEFS_GLOBAL sdnpmem_alloc(
  SDNPMEM_ALLOC_TYPE type)
{
  if ((type >= SDNPMEM_ALLOC_TYPE_MAX)
    || (type < 0))
  {
    return(TMWDEFS_NULL);
  }

  return(tmwmem_lowAlloc(&_sdnpmemAllocTable[type]));
}

/* function: sdnpmem_checkLimit */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpmem_checkLimit(
  SDNPMEM_ALLOC_TYPE type)
{
  if ((type >= SDNPMEM_ALLOC_TYPE_MAX)
    || (type < 0))
  {
    return(TMWDEFS_FALSE);
  }

  return(tmwmem_lowCheckLimit(&_sdnpmemAllocTable[type]));
}

/* function: sdnpmem_free */
void TMWDEFS_CALLBACK sdnpmem_free(
  void *pBuf)
{    
  TMWMEM_HEADER *pHeader = TMWMEM_GETHEADER(pBuf);
  TMWTYPES_UCHAR  type = TMWMEM_GETTYPE(pHeader);

  if(type >= SDNPMEM_ALLOC_TYPE_MAX)
  {
    return;
  }

  tmwmem_lowFree(&_sdnpmemAllocTable[type], pHeader);
}

TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpmem_getUsage(
  TMWTYPES_UCHAR index,
  const TMWTYPES_CHAR **name,
  TMWMEM_POOL_STRUCT *pStruct)
{
  if(index >= SDNPMEM_ALLOC_TYPE_MAX)
  {
    return(TMWDEFS_FALSE);
  }

  *name = _nameTable[index];
  *pStruct = _sdnpmemAllocTable[index];
  return(TMWDEFS_TRUE);
}


 
