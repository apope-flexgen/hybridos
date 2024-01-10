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

/* file: sdnprbe.c
 * description: Slave DNP Report By Exception support.
 */
#include "tmwscl/dnp/dnpdiag.h"
#include "tmwscl/dnp/sdnprbe.h"
#include "tmwscl/dnp/sdnpsesn.h"
#include "tmwscl/dnp/sdnpdata.h"
#include "tmwscl/dnp/sdnpo002.h"
#include "tmwscl/dnp/sdnpo004.h"
#include "tmwscl/dnp/sdnpo011.h"
#include "tmwscl/dnp/sdnpo013.h"
#include "tmwscl/dnp/sdnpo022.h"
#include "tmwscl/dnp/sdnpo023.h"
#include "tmwscl/dnp/sdnpo032.h"
#include "tmwscl/dnp/sdnpo033.h"
#include "tmwscl/dnp/sdnpo042.h"
#include "tmwscl/dnp/sdnpo043.h"
#include "tmwscl/dnp/sdnpo070.h"
#include "tmwscl/dnp/sdnpo088.h"
#include "tmwscl/dnp/sdnpo111.h"
#include "tmwscl/dnp/sdnpo113.h"
#include "tmwscl/dnp/sdnpo115.h"
#if SDNPDATA_SUPPORT_OBJ120
#include "tmwscl/dnp/sdnpo120.h"
#if SDNPCNFG_SUPPORT_SA_VERSION5
#include "tmwscl/dnp/sdnpo122.h"
#endif
#include "tmwscl/dnp/sdnpauth.h"
#endif

/* function: sdnprbe_init */
void TMWDEFS_GLOBAL sdnprbe_init(TMWSESN *pSession)
{
#if SDNPDATA_SUPPORT_OBJ2
  sdnpo002_init(pSession);
#endif
#if SDNPDATA_SUPPORT_OBJ4
  sdnpo004_init(pSession);
#endif
#if SDNPDATA_SUPPORT_OBJ11
  sdnpo011_init(pSession);
#endif
#if SDNPDATA_SUPPORT_OBJ13
  sdnpo013_init(pSession);
#endif
#if SDNPDATA_SUPPORT_OBJ22
  sdnpo022_init(pSession);
#endif
#if SDNPDATA_SUPPORT_OBJ23
  sdnpo023_init(pSession);
#endif
#if SDNPDATA_SUPPORT_OBJ32
  sdnpo032_init(pSession);
#endif
#if SDNPDATA_SUPPORT_OBJ33
  sdnpo033_init(pSession);
#endif
#if SDNPDATA_SUPPORT_OBJ42
  sdnpo042_init(pSession);
#endif
#if SDNPDATA_SUPPORT_OBJ43
  sdnpo043_init(pSession);
#endif
#if SDNPDATA_SUPPORT_OBJ88
  sdnpo088_init(pSession);
#endif
#if SDNPDATA_SUPPORT_OBJ111
  sdnpo111_init(pSession);
#endif
#if SDNPDATA_SUPPORT_OBJ113
  sdnpo113_init(pSession);
#endif
#if SDNPDATA_SUPPORT_OBJ115
  sdnpo115_init(pSession);
#endif
#if SDNPDATA_SUPPORT_OBJ120
  sdnpo120_init(pSession);
#if SDNPCNFG_SUPPORT_SA_VERSION5
  sdnpo122_init(pSession);
#endif
#endif
} 

#if SDNPDATA_SUPPORT_EVENT_SCAN
/* function: sdnprbe_open */
void TMWDEFS_GLOBAL sdnprbe_open(TMWSESN *pSession)
{
#if SDNPDATA_SUPPORT_OBJ2
  sdnpo002_open(pSession);
#endif
#if SDNPDATA_SUPPORT_OBJ4
  sdnpo004_open(pSession);
#endif
#if SDNPDATA_SUPPORT_OBJ11
  sdnpo011_open(pSession);
#endif
#if SDNPDATA_SUPPORT_OBJ13
  sdnpo013_open(pSession);
#endif
#if SDNPDATA_SUPPORT_OBJ22
  sdnpo022_open(pSession);
#endif
#if SDNPDATA_SUPPORT_OBJ23
  sdnpo023_open(pSession);
#endif
#if SDNPDATA_SUPPORT_OBJ32
  sdnpo032_open(pSession);
#endif
#if SDNPDATA_SUPPORT_OBJ33
  sdnpo033_open(pSession);
#endif
#if SDNPDATA_SUPPORT_OBJ42
  sdnpo042_open(pSession);
#endif
#if SDNPDATA_SUPPORT_OBJ43
  sdnpo043_open(pSession);
#endif
  /* Nothing to do for Obj70 */
  /* Nothing to do for Obj88 */
#if SDNPDATA_SUPPORT_OBJ111
  sdnpo111_open(pSession);
#endif
#if SDNPDATA_SUPPORT_OBJ113
  sdnpo113_open(pSession);
#endif
#if SDNPDATA_SUPPORT_OBJ115
  sdnpo115_open(pSession);
#endif
  /* There is no sdnpo120_open() */
}
#endif

/* function: sdnprbe_close */
void TMWDEFS_GLOBAL sdnprbe_close(TMWSESN *pSession)
{
#if SDNPDATA_SUPPORT_OBJ2
  sdnpo002_close(pSession);
#endif
#if SDNPDATA_SUPPORT_OBJ4
  sdnpo004_close(pSession);
#endif
#if SDNPDATA_SUPPORT_OBJ11
  sdnpo011_close(pSession);
#endif
#if SDNPDATA_SUPPORT_OBJ13
  sdnpo013_close(pSession);
#endif
#if SDNPDATA_SUPPORT_OBJ22
  sdnpo022_close(pSession);
#endif
#if SDNPDATA_SUPPORT_OBJ23
  sdnpo023_close(pSession);
#endif
#if SDNPDATA_SUPPORT_OBJ32
  sdnpo032_close(pSession);
#endif
#if SDNPDATA_SUPPORT_OBJ33
  sdnpo033_close(pSession);
#endif
#if SDNPDATA_SUPPORT_OBJ42
  sdnpo042_close(pSession);
#endif
#if SDNPDATA_SUPPORT_OBJ43
  sdnpo043_close(pSession);
#endif
#if SDNPDATA_SUPPORT_OBJ88
  sdnpo088_close(pSession);
#endif
#if SDNPDATA_SUPPORT_OBJ111
  sdnpo111_close(pSession);
#endif
#if SDNPDATA_SUPPORT_OBJ113
  sdnpo113_close(pSession);
#endif
#if SDNPDATA_SUPPORT_OBJ115
  sdnpo115_close(pSession);
#endif
#if SDNPDATA_SUPPORT_OBJ120
  sdnpo120_close(pSession);
#if SDNPCNFG_SUPPORT_SA_VERSION5
  sdnpo122_close(pSession);
#endif
#endif
}

/* function: sdnprbe_cleanupEvents */
void TMWDEFS_GLOBAL sdnprbe_cleanupEvents(TMWSESN *pSession, TMWTYPES_BOOL deleteEvents)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  TMWTYPES_BOOL queueFull = TMWDEFS_FALSE;

#if SDNPDATA_SUPPORT_IDENT_UNSOL_RETRY
  if(pSDNPSession->pUnsolLastResponse != TMWDEFS_NULL)
  {
    dnpchnl_freeTxData((TMWSESN_TX_DATA *)pSDNPSession->pUnsolLastResponse);
    pSDNPSession->pUnsolLastResponse = TMWDEFS_NULL;
  }
#endif

#if SDNPDATA_SUPPORT_OBJ2
  queueFull |= sdnpo002_cleanupEvents(pSession, deleteEvents);
#endif
#if SDNPDATA_SUPPORT_OBJ4
  queueFull |= sdnpo004_cleanupEvents(pSession, deleteEvents);
#endif
#if SDNPDATA_SUPPORT_OBJ11
  queueFull |= sdnpo011_cleanupEvents(pSession, deleteEvents);
#endif
#if SDNPDATA_SUPPORT_OBJ13
  queueFull |= sdnpo013_cleanupEvents(pSession, deleteEvents);
#endif
#if SDNPDATA_SUPPORT_OBJ22
  queueFull |= sdnpo022_cleanupEvents(pSession, deleteEvents);
#endif
#if SDNPDATA_SUPPORT_OBJ23
  queueFull |= sdnpo023_cleanupEvents(pSession, deleteEvents);
#endif
#if SDNPDATA_SUPPORT_OBJ32
  queueFull |= sdnpo032_cleanupEvents(pSession, deleteEvents);
#endif
#if SDNPDATA_SUPPORT_OBJ33
  queueFull |= sdnpo033_cleanupEvents(pSession, deleteEvents);
#endif
#if SDNPDATA_SUPPORT_OBJ42
  queueFull |= sdnpo042_cleanupEvents(pSession, deleteEvents);
#endif
#if SDNPDATA_SUPPORT_OBJ43
  queueFull |= sdnpo043_cleanupEvents(pSession, deleteEvents);
#endif
#if SDNPDATA_SUPPORT_OBJ70
  queueFull |= sdnpo070_cleanupEvents(pSession, deleteEvents);
#endif
#if SDNPDATA_SUPPORT_OBJ88
  queueFull |= sdnpo088_cleanupEvents(pSession, deleteEvents);
#endif
#if SDNPDATA_SUPPORT_OBJ111
  queueFull |= sdnpo111_cleanupEvents(pSession, deleteEvents);
#endif
#if SDNPDATA_SUPPORT_OBJ113
  queueFull |= sdnpo113_cleanupEvents(pSession, deleteEvents);
#endif
#if SDNPDATA_SUPPORT_OBJ115
  queueFull |= sdnpo115_cleanupEvents(pSession, deleteEvents);
#endif
#if SDNPDATA_SUPPORT_OBJ120
  queueFull |= sdnpo120_cleanupEvents(pSession, deleteEvents);
#if SDNPCNFG_SUPPORT_SA_VERSION5
  queueFull |= sdnpo122_cleanupEvents(pSession, deleteEvents);
#endif
#endif

  /* Update class 1, 2, 3 data available IIN bits */
  sdnprbe_updateIINBits(pSession);

  /* See if we can clear the buffer overflow bit */
  if(deleteEvents
    && pSDNPSession->bufferOverFlowSent
    && !queueFull)
  {
    pSDNPSession->iin &= ~DNPDEFS_IIN_BUFFER_OVFL;

    pSDNPSession->bufferOverFlowSent = TMWDEFS_FALSE;
  }
}

/* function: sdnprbe_countEvents */
TMWTYPES_ULONG TMWDEFS_GLOBAL sdnprbe_countEvents(
  TMWSESN *pSession,
  TMWDEFS_CLASS_MASK classMask,
  TMWTYPES_BOOL countAll,
  TMWTYPES_USHORT threshold)
{
  TMWTYPES_ULONG numEvents = 0;

#if SDNPDATA_SUPPORT_OBJ2
  numEvents = (numEvents + sdnpo002_countEvents(pSession, classMask, countAll, threshold));
  if(!countAll && (numEvents >= threshold))
    return(numEvents);
#endif
#if SDNPDATA_SUPPORT_OBJ4
  numEvents = (numEvents + sdnpo004_countEvents(pSession, classMask, countAll, threshold));
  if (!countAll && (numEvents >= threshold))
    return(numEvents);
#endif
#if SDNPDATA_SUPPORT_OBJ11
  numEvents = (numEvents + sdnpo011_countEvents(pSession, classMask, countAll, threshold));
  if (!countAll && (numEvents >= threshold))
    return(numEvents);
#endif
#if SDNPDATA_SUPPORT_OBJ13
  numEvents = (numEvents + sdnpo013_countEvents(pSession, classMask, countAll, threshold));
  if (!countAll && (numEvents >= threshold))
    return(numEvents);
#endif
#if SDNPDATA_SUPPORT_OBJ22
  numEvents = (numEvents + sdnpo022_countEvents(pSession, classMask, countAll, threshold));
  if (!countAll && (numEvents >= threshold))
    return(numEvents);
#endif
#if SDNPDATA_SUPPORT_OBJ23
  numEvents = (numEvents + sdnpo023_countEvents(pSession, classMask, countAll, threshold));
  if (!countAll && (numEvents >= threshold))
    return(numEvents);
#endif
#if SDNPDATA_SUPPORT_OBJ32
  numEvents = (numEvents + sdnpo032_countEvents(pSession, classMask, countAll, threshold));
  if (!countAll && (numEvents >= threshold))
    return(numEvents);
#endif
#if SDNPDATA_SUPPORT_OBJ33
  numEvents = (numEvents + sdnpo033_countEvents(pSession, classMask, countAll, threshold));
  if (!countAll && (numEvents >= threshold))
    return(numEvents);
#endif
#if SDNPDATA_SUPPORT_OBJ42
  numEvents = (numEvents + sdnpo042_countEvents(pSession, classMask, countAll, threshold));
  if (!countAll && (numEvents >= threshold))
    return(numEvents);
#endif
#if SDNPDATA_SUPPORT_OBJ43
  numEvents = (numEvents + sdnpo043_countEvents(pSession, classMask, countAll, threshold));
  if (!countAll && (numEvents >= threshold))
    return(numEvents);
#endif
#if SDNPDATA_SUPPORT_OBJ70
  numEvents = (numEvents + sdnpo070_countEvents(pSession, classMask, countAll));
  if (!countAll && (numEvents >= threshold))
    return(numEvents);
#endif
#if SDNPDATA_SUPPORT_OBJ88
  numEvents = (numEvents + sdnpo088_countEvents(pSession, classMask, countAll, threshold));
  if (!countAll && (numEvents >= threshold))
    return(numEvents);
#endif
#if SDNPDATA_SUPPORT_OBJ111
  numEvents = (numEvents + sdnpo111_countEvents(pSession, classMask, countAll, threshold));
  if (!countAll && (numEvents >= threshold))
    return(numEvents);
#endif
#if SDNPDATA_SUPPORT_OBJ113
  numEvents = (numEvents + sdnpo113_countEvents(pSession, classMask, countAll, threshold));
  if (!countAll && (numEvents >= threshold))
    return(numEvents);
#endif
#if SDNPDATA_SUPPORT_OBJ115
  numEvents = (numEvents + sdnpo115_countEvents(pSession, classMask, countAll, threshold));
  if (!countAll && (numEvents >= threshold))
    return(numEvents);
#endif
#if SDNPDATA_SUPPORT_OBJ120
  numEvents = (numEvents + sdnpo120_countEvents(pSession, classMask, countAll, threshold));
  if (!countAll && (numEvents >= threshold))
    return(numEvents);
#if SDNPCNFG_SUPPORT_SA_VERSION5
  numEvents = (numEvents + sdnpo122_countEvents(pSession, classMask, countAll, threshold));
  if (!countAll && (numEvents > threshold))
    return(numEvents);
#endif
#endif

  return(numEvents);
}

/* function: sdnprbe_updateIINBits */
void TMWDEFS_GLOBAL sdnprbe_updateIINBits(TMWSESN *pSession)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;

  pSDNPSession->iin &= ~DNPDEFS_IIN_CLASS_1;
  if(sdnprbe_countEvents(pSession, TMWDEFS_CLASS_MASK_ONE, TMWDEFS_FALSE, 1) > 0)
    pSDNPSession->iin |= DNPDEFS_IIN_CLASS_1;

  pSDNPSession->iin &= ~DNPDEFS_IIN_CLASS_2;
  if(sdnprbe_countEvents(pSession, TMWDEFS_CLASS_MASK_TWO, TMWDEFS_FALSE, 1) > 0)
    pSDNPSession->iin |= DNPDEFS_IIN_CLASS_2;

  pSDNPSession->iin &= ~DNPDEFS_IIN_CLASS_3;
  if(sdnprbe_countEvents(pSession, TMWDEFS_CLASS_MASK_THREE, TMWDEFS_FALSE, 1) > 0)
    pSDNPSession->iin |= DNPDEFS_IIN_CLASS_3;
}
