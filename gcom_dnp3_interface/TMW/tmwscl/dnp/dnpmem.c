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
/* file: dnpmem.c
 * description:  Implementation of Master DNP specific memory allocation functions.
 */
#include "tmwscl/dnp/dnpdiag.h"
#include "tmwscl/utils/tmwtarg.h"
#include "tmwscl/dnp/dnpmem.h"
#include "tmwscl/dnp/dnplink.h"
#include "tmwscl/dnp/dnptprt.h"
#include "tmwscl/dnp/dnpchnl.h"

typedef struct {
  TMWMEM_HEADER        header;
  DNPCHNL              databuf;
} DNPMEM_DNPCHNL;

#if TMWCNFG_SUPPORT_DIAG
typedef struct{
  TMWMEM_HEADER        header;
  DNPLINK_FRAME        databuf;
} DNPMEM_LINK_FRAME;
#endif

typedef struct{
  TMWMEM_HEADER        header;
  DNPLINK_SESSION_INFO databuf;
} DNPMEM_LINK_SESSION;

typedef struct{
  TMWMEM_HEADER        header;
  DNPLINK_CONTEXT      databuf;
} DNPMEM_LINK_CONTEXT;

typedef struct{
  TMWMEM_HEADER        header;
  DNPTPRT_SESSION_INFO databuf;
} DNPMEM_TPRT_SESSION;

typedef struct{
  TMWMEM_HEADER        header;
  DNPTPRT_CONTEXT      databuf;
} DNPMEM_TPRT_CONTEXT;

typedef struct{
  TMWMEM_HEADER        header;
  DNPCHNL_TX_DATA      databuf;
} DNPMEM_CHNL_TX_DATA;

static const TMWTYPES_CHAR *_nameTable[DNPMEM_ALLOC_TYPE_MAX] = {
  "DNPCHNL",
#if TMWCNFG_SUPPORT_DIAG
  "DNPLINK_FRAME",
#endif
  "DNPLINK_SESSION_INFO",
  "DNPLINK_CONTEXT",
  "DNPTPRT_SESSION",
  "DNPTPRT_CONTEXT",
  "DNPCHNL_TX_DATA"
};
 
#if !TMWCNFG_USE_DYNAMIC_MEMORY
/* Use static allocated memory instead of dynamic memory */

static DNPMEM_DNPCHNL         dnpmem_chnls[DNPCNFG_NUMALLOC_CHANNELS];
#if TMWCNFG_SUPPORT_DIAG
static DNPMEM_LINK_FRAME      dnpmem_linkFrames[DNPCNFG_NUMALLOC_LINK_FRAMES];
#endif
static DNPMEM_LINK_SESSION    dnpmem_linkSessions[DNPCNFG_NUMALLOC_LINK_SESSIONS];
static DNPMEM_LINK_CONTEXT    dnpmem_linkContexts[DNPCNFG_NUMALLOC_LINK_CONTEXTS];
static DNPMEM_TPRT_SESSION    dnpmem_tprtSessions[DNPCNFG_NUMALLOC_TPRT_SESSIONS];
static DNPMEM_TPRT_CONTEXT    dnpmem_tprtContexts[DNPCNFG_NUMALLOC_TPRT_CONTEXTS];
static DNPMEM_CHNL_TX_DATA    dnpmem_txDatas[DNPCNFG_NUMALLOC_CHNL_TX_DATAS];

#endif

static TMWMEM_POOL_STRUCT _dnpmemAllocTable[DNPMEM_ALLOC_TYPE_MAX];

#if TMWCNFG_USE_DYNAMIC_MEMORY
void TMWDEFS_GLOBAL dnpmem_initConfig(
  DNPMEM_CONFIG *pConfig)
{
  pConfig->numChannels     = DNPCNFG_NUMALLOC_CHANNELS;
  pConfig->numFrames       = DNPCNFG_NUMALLOC_LINK_FRAMES;
  pConfig->numSessions     = DNPCNFG_NUMALLOC_LINK_SESSIONS;
  pConfig->numLinkContexts = DNPCNFG_NUMALLOC_LINK_CONTEXTS;
  pConfig->numTprtSessions = DNPCNFG_NUMALLOC_TPRT_SESSIONS;
  pConfig->numTprtContexts = DNPCNFG_NUMALLOC_TPRT_CONTEXTS;
  pConfig->numTxDatas      = DNPCNFG_NUMALLOC_CHNL_TX_DATAS;
}
#endif

/* routine: dnpmem_init */
TMWTYPES_BOOL TMWDEFS_GLOBAL dnpmem_init(
  DNPMEM_CONFIG *pConfig)
{
#if TMWCNFG_USE_DYNAMIC_MEMORY
  /* dynamic memory allocation supported */
  
  DNPMEM_CONFIG  config; 

  /* If caller has not specified memory pool configuration, use the
   * default compile time values 
   */
  if(pConfig == TMWDEFS_NULL)
  {
    pConfig = &config;
    dnpmem_initConfig(pConfig);
  }

  if(!tmwmem_lowInit(_dnpmemAllocTable, DNPMEM_CHNL_TYPE,              pConfig->numChannels,     sizeof(DNPMEM_DNPCHNL),      TMWDEFS_NULL))
    return TMWDEFS_FALSE;
#if TMWCNFG_SUPPORT_DIAG
  if(!tmwmem_lowInit(_dnpmemAllocTable, DNPMEM_LINK_FRAME_TYPE,        pConfig->numFrames,       sizeof(DNPMEM_LINK_FRAME),   TMWDEFS_NULL))
    return TMWDEFS_FALSE;
#endif
  if(!tmwmem_lowInit(_dnpmemAllocTable, DNPMEM_LINK_SESSION_INFO_TYPE, pConfig->numSessions,     sizeof(DNPMEM_LINK_SESSION), TMWDEFS_NULL))
    return TMWDEFS_FALSE;
  if(!tmwmem_lowInit(_dnpmemAllocTable, DNPMEM_LINK_CONTEXT_TYPE,      pConfig->numLinkContexts, sizeof(DNPMEM_LINK_CONTEXT), TMWDEFS_NULL))
    return TMWDEFS_FALSE;
  if(!tmwmem_lowInit(_dnpmemAllocTable, DNPMEM_TPRT_SESSION_INFO_TYPE, pConfig->numTprtSessions, sizeof(DNPMEM_TPRT_SESSION), TMWDEFS_NULL))
    return TMWDEFS_FALSE;
  if(!tmwmem_lowInit(_dnpmemAllocTable, DNPMEM_TPRT_CONTEXT_TYPE,      pConfig->numTprtContexts, sizeof(DNPMEM_TPRT_CONTEXT), TMWDEFS_NULL))
    return TMWDEFS_FALSE;
  if(!tmwmem_lowInit(_dnpmemAllocTable, DNPMEM_CHNL_TX_DATA_TYPE,      pConfig->numTxDatas,      sizeof(DNPMEM_CHNL_TX_DATA), TMWDEFS_NULL))
    return TMWDEFS_FALSE;

#else

  /* static memory allocation supported */
  TMWTARG_UNUSED_PARAM(pConfig);  
  if(!tmwmem_lowInit(_dnpmemAllocTable, DNPMEM_CHNL_TYPE,              DNPCNFG_NUMALLOC_CHANNELS,      sizeof(DNPMEM_DNPCHNL),      (TMWTYPES_UCHAR *)dnpmem_chnls))
    return TMWDEFS_FALSE;
#if TMWCNFG_SUPPORT_DIAG
  if(!tmwmem_lowInit(_dnpmemAllocTable, DNPMEM_LINK_FRAME_TYPE,        DNPCNFG_NUMALLOC_LINK_FRAMES,   sizeof(DNPMEM_LINK_FRAME),   (TMWTYPES_UCHAR *)dnpmem_linkFrames))
    return TMWDEFS_FALSE;
#endif
  if(!tmwmem_lowInit(_dnpmemAllocTable, DNPMEM_LINK_SESSION_INFO_TYPE, DNPCNFG_NUMALLOC_LINK_SESSIONS, sizeof(DNPMEM_LINK_SESSION), (TMWTYPES_UCHAR *)dnpmem_linkSessions))
    return TMWDEFS_FALSE;
  if(!tmwmem_lowInit(_dnpmemAllocTable, DNPMEM_LINK_CONTEXT_TYPE,      DNPCNFG_NUMALLOC_LINK_CONTEXTS, sizeof(DNPMEM_LINK_CONTEXT), (TMWTYPES_UCHAR *)dnpmem_linkContexts))
    return TMWDEFS_FALSE;
  if(!tmwmem_lowInit(_dnpmemAllocTable, DNPMEM_TPRT_SESSION_INFO_TYPE, DNPCNFG_NUMALLOC_TPRT_SESSIONS, sizeof(DNPMEM_TPRT_SESSION), (TMWTYPES_UCHAR *)dnpmem_tprtSessions))
    return TMWDEFS_FALSE;
  if(!tmwmem_lowInit(_dnpmemAllocTable, DNPMEM_TPRT_CONTEXT_TYPE,      DNPCNFG_NUMALLOC_TPRT_CONTEXTS, sizeof(DNPMEM_TPRT_CONTEXT), (TMWTYPES_UCHAR *)dnpmem_tprtContexts))
    return TMWDEFS_FALSE;
  if(!tmwmem_lowInit(_dnpmemAllocTable, DNPMEM_CHNL_TX_DATA_TYPE,      DNPCNFG_NUMALLOC_CHNL_TX_DATAS, sizeof(DNPMEM_CHNL_TX_DATA), (TMWTYPES_UCHAR *)dnpmem_txDatas))
    return TMWDEFS_FALSE;

#endif
  return TMWDEFS_TRUE;
}

/* function: dnpmem_alloc */
void * TMWDEFS_GLOBAL dnpmem_alloc(
  DNPMEM_ALLOC_TYPE type)
{
  if((type >= DNPMEM_ALLOC_TYPE_MAX)
    || (type < 0))
  {
    return(TMWDEFS_NULL);
  }

  return(tmwmem_lowAlloc(&_dnpmemAllocTable[type]));
}

/* function: dnpmem_free */
void TMWDEFS_GLOBAL dnpmem_free(
  void *pBuf)
{    
  TMWMEM_HEADER *pHeader = TMWMEM_GETHEADER(pBuf);
  TMWTYPES_UCHAR   type = TMWMEM_GETTYPE(pHeader);

  if(type >= DNPMEM_ALLOC_TYPE_MAX)
  {
    return;
  }

  tmwmem_lowFree(&_dnpmemAllocTable[type], pHeader);
}

/* function: dnpmem_getUsage */
TMWTYPES_BOOL TMWDEFS_GLOBAL dnpmem_getUsage(
  TMWTYPES_UCHAR index,
  const TMWTYPES_CHAR **pName,
  TMWMEM_POOL_STRUCT *pStruct)
{
  if(index >= DNPMEM_ALLOC_TYPE_MAX)
  {
    return(TMWDEFS_FALSE);
  }

  *pName = _nameTable[index];
  *pStruct = _dnpmemAllocTable[index];
  return(TMWDEFS_TRUE);
}

