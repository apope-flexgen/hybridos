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

/* file: sdnpo115.h
 * description: This file is intended for internal SCL use only.
 *   DNP Slave functionality for Object 115 Extended String Events
 */
#ifndef SDNPO115_DEFINED
#define SDNPO115_DEFINED

#include "tmwscl/dnp/sdnpsesn.h"
#include "tmwscl/dnp/sdnpdata.h"
#include "tmwscl/dnp/sdnpevnt.h"

#if SDNPDATA_SUPPORT_OBJ115

#ifdef __cplusplus
extern "C" {
#endif

  /* function: sdnpo115_addEvent 
   * purpose: add a string event to queue 
   * arguments:
   *  pSession - pointer to session
   *  point - point number of point that generated the event
   *  pBuf - pointer to string
   *  bufLength - length of string
   *  flags - current DNP flags
   *  pTimeStamp - pointer to time of event
   *   This should be gotten by calling sdnputil_getDateTime(). This will
   *   set the invalid field of the time structure properly.  
   *    Even if no time is available this SHALL not be TMWDEFS_NULL. You
   *    may choose to set the DNP3 time epoch ie Jan1 1970.
   * returns:
   *  TMWDEFS_TRUE if successful
   *  TMWDEFS_FALSE otherwise
   */
  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpo115_addEvent(
    TMWSESN *pSession, 
    TMWTYPES_USHORT point, 
    TMWTYPES_UCHAR *pBuf, 
    TMWTYPES_USHORT bufLength,
    TMWTYPES_UCHAR flags,
    TMWDTIME *pTimeStamp);

  /* function: sdnpo115_init 
   * purpose: initialize extended string events
   * arguments:
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL sdnpo115_init(
    TMWSESN *pSession);

  /* function: sdnpo115_open
   * purpose: start extended string event scanning if configured
   * arguments:
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL sdnpo115_open(
    TMWSESN *pSession);

  /* function: sdnpo115_closet 
   * purpose: 
   * arguments:
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL sdnpo115_close(
    TMWSESN *pSession);

  /* function: sdnpo115_countEvents 
   * purpose: counter number of extended string events are
   *  queued that match the requested class
   * arguments:
   * returns:
   *  void
   */
  TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpo115_countEvents(
    TMWSESN *pSession, 
    TMWDEFS_CLASS_MASK classMask,
    TMWTYPES_BOOL countAll,
    TMWTYPES_USHORT threshold);

  /* function: sdnpo115_cleanupEvents 
   * purpose: remove extended string events that have been
   *  successfully transmitted from queue.
   * arguments:
   * returns:
   *  void
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpo115_cleanupEvents(
    TMWSESN *pSession, 
    TMWTYPES_BOOL deleteEvents);

  /* function: sdnpo115_readObj115VarX
   * purpose: read extended string event of specified variation into response
   * arguments:
   * returns:
   *  void
   */
  TMWTYPES_BOOL TMWDEFS_CALLBACK sdnpo115_readObj115VarX(
    TMWSESN *pSession,
    TMWSESN_TX_DATA *pResponse,
    SDNPEVNT *pEvent,
    TMWTYPES_UCHAR variation);

  /* function: sdnpo115_readObj115 
   * purpose: read extended string events into response
   * arguments:
   * returns:
   *  void
   */
  SDNPSESN_READ_STATUS TMWDEFS_CALLBACK sdnpo115_readObj115(
    TMWSESN *pSession, 
    DNPUTIL_RX_MSG *pRequest,
    TMWSESN_TX_DATA *pResponse, 
    DNPUTIL_OBJECT_HEADER *pObjHeader,
    SDNPSESN_QUAL qualifier);

  /* function: sdnpo115_readObj115ByClass
   * purpose: read extended string events into response
   * arguments:
   * returns:
   *  void
   */
  SDNPSESN_READ_STATUS TMWDEFS_CALLBACK sdnpo115_readObj115ByClass(
    TMWSESN *pSession,
    DNPUTIL_RX_MSG *pRequest,
    TMWSESN_TX_DATA *pResponse, 
    DNPUTIL_OBJECT_HEADER *pObjHeader,
    TMWDEFS_CLASS_MASK classMask);


#ifdef __cplusplus
}
#endif
#endif /* SDNPDATA_SUPPORT_OBJ115 */
#endif /* SDNPO115_DEFINED */
