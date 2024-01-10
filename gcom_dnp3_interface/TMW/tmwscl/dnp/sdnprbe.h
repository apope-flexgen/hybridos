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

/* file: sdnprbe.h
 * description: This file is intended for internal SCL use only.
 *   Slave DNP Report By Exception support.
 */
#ifndef SDNPRBE_DEFINED
#define SDNPRBE_DEFINED

#include "tmwscl/dnp/sdnpsesn.h"

#ifdef __cplusplus
extern "C" {
#endif

  /* function: sdnprbe_init */
  void TMWDEFS_GLOBAL sdnprbe_init(
    TMWSESN *pSession);

  /* function: sdnprbe_open */
  void TMWDEFS_GLOBAL sdnprbe_open(
    TMWSESN *pSession);

  /* function: sdnprbe_close */
  void TMWDEFS_GLOBAL sdnprbe_close(
    TMWSESN *pSession);

  /* function: sdnprbe_cleanupEvents */
  void TMWDEFS_GLOBAL sdnprbe_cleanupEvents(
    TMWSESN *pSession, 
    TMWTYPES_BOOL deleteEvents);

  /* function: sdnprbe_countEvents 
   *  countAll - if TMWDEFS_TRUE, count all events in all of the event queues
   *             if TMWDEFS_FALSE, count until at least threshold events are found 
   *  threshold -if countAll is TMWDEFS_FALSE, stop counting when threshold is reached.
   *             The most the SDNP library currently cares about is 256 for qualifier. 
   */
  TMWTYPES_ULONG TMWDEFS_GLOBAL sdnprbe_countEvents(
    TMWSESN *pSession,
    TMWDEFS_CLASS_MASK classMask,
    TMWTYPES_BOOL countAll,
    TMWTYPES_USHORT threshold);

  /* function: sdnprbe_updateIINBits */
  void TMWDEFS_GLOBAL sdnprbe_updateIINBits(
    TMWSESN *pSession);

#ifdef __cplusplus
}
#endif
#endif /* SDNPRBE_DEFINED */
