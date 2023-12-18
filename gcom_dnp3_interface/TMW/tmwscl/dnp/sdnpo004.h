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

/* file: sdnpo004.h
 * description: This file is intended for internal SCL use only.
 *   DNP SLAVE functionality for Object 4 Double Bit Input change events.
 */
#ifndef SDNPO004_DEFINED
#define SDNPO004_DEFINED

#include "tmwscl/utils/tmwdefs.h"
#include "tmwscl/utils/tmwdtime.h"

#include "tmwscl/dnp/sdnpsesn.h"
#include "tmwscl/dnp/sdnpdata.h"
#include "tmwscl/dnp/sdnpevnt.h"

#if SDNPDATA_SUPPORT_OBJ4

#ifdef __cplusplus
extern "C" {
#endif

  /* function: sdnpo004_addEvent
   * purpose: Add a double bit input event to event queue
   * arguments:
   *  pSession - pointer to session
   *  point - point number of point that generated the event
   *  flags - current DNP flags including value
   *  pTimeStamp - pointer to time of event.
   *   This should be gotten by calling sdnputil_getDateTime(). This will
   *   set the invalid field of the time structure properly.  
   *    Even if no time is available this SHALL not be TMWDEFS_NULL. You
   *    may choose to set the DNP3 time epoch ie Jan1 1970.
   * returns:
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL sdnpo004_addEvent(
    TMWSESN *pSession, 
    TMWTYPES_USHORT point, 
    TMWTYPES_UCHAR flags, 
    TMWDTIME *pTimeStamp);

  /* function: sdnpo004_init
   * purpose: Initialize double bit input change events
   * arguments:
   *  pSession - pointer to session
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL sdnpo004_init(
    TMWSESN *pSession); 
    
  /* function: sdnpo004_open
   * purpose: Start double bit input change scanning if configured
   * arguments:
   *  pSession - pointer to session
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL sdnpo004_open(
    TMWSESN *pSession);

  /* function: sdnpo004_close
   * purpose: Close double bit input event processing
   * arguments:
   *  pSession - pointer to session
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL sdnpo004_close(
    TMWSESN *pSession);

  /* function: sdnpo004_countEvents
   * purpose: Count the number of double bit input change events in
   *  queue
   * arguments:
   *  pSession - pointer to session
   *  classMask - count events in this class or classes
   * returns:
   *  void
   */
  TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpo004_countEvents(
    TMWSESN *pSession, 
    TMWDEFS_CLASS_MASK classMask,
    TMWTYPES_BOOL countAll,
    TMWTYPES_USHORT threshold);

  /* function: sdnpo004_cleanupEvents
   * purpose: Either remove events that have been sent or clear the
   *  sent flags if message was not acknowledged.
   * arguments:
   *  pSession - pointer to session
   *  deleteEvents - remove events if true, else clear sent flag
   * returns:
   *  TMWDEFS_TRUE if specified queue is full
   *  TMWDEFS_FALSE if queue is not full
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpo004_cleanupEvents(
    TMWSESN *pSession, 
    TMWTYPES_BOOL deleteEvents);

  /* function: sdnpo004_setVariationInfo()
   * purpose: Set variation to proposed
   * arguments:
   *  pDesc - pointer to event descriptor 
   *  variation - proposed variation
   * returns:
   *  void
   */
  void TMWDEFS_CALLBACK sdnpo004_setVariationInfo(
    SDNPEVNT_DESC *pDesc,
    TMWTYPES_UCHAR variation);

  /* function: sdnpo004_readObj4
   * purpose: Read double bit change events using specified variation
   * arguments:
   *  pSession - pointer to session
   *  pResponse - response buffer to read into
   *  pObjHeader - current request object header
   * returns:
   *   SDNPSESN_READ_COMPLETE  if read succeeded, but there is no more data
   *   SDNPSESN_READ_MORE_DATA if read succeeded, and there is more data
   *   SDNPSESN_READ_FAILED if read failed
   */
  SDNPSESN_READ_STATUS TMWDEFS_CALLBACK sdnpo004_readObj4(
    TMWSESN *pSession, 
    DNPUTIL_RX_MSG *pRequest,
    TMWSESN_TX_DATA *pResponse, 
    DNPUTIL_OBJECT_HEADER *pObjHeader,
    SDNPSESN_QUAL qualifier);

#if SDNPDATA_SUPPORT_OBJ4_V3
  /* function: sdnpo004_readObj4Var3
   * purpose: Read double bit change events using variation 3
   *  Called from sdnpevnt_readEvents() to allow special code
   *  to determine if a CTO object is required.
   * arguments:
   *  pEvent - pointer a event pointer
   *    Both an input and an output from this function
   *  pNumEventsInResponse - pointer to number of events in response.
   *    Both an input and an output from this function
   *  pDesc - pointer to event descriptor 
   *  pResponse - response buffer to read into
   *  pObjHeader - current request object header
   *  classMask - mask indicating event class being read
   * returns:
   *  void
   */
  SDNPSESN_READ_STATUS TMWDEFS_CALLBACK sdnpo004_readObj4Var3(
    SDNPEVNT **pEvent,
    TMWTYPES_USHORT *pNumEventsInResponse,
    SDNPEVNT_DESC *pDesc,
    TMWSESN_TX_DATA *pResponse, 
    DNPUTIL_OBJECT_HEADER *pObjHeader, 
    TMWDEFS_CLASS_MASK classMask);
#endif

  /* function: sdnpo004_readObj4v0ByClass
   * purpose: Read double bit change events using default variation for a 
   *  specific class
   * arguments:
   *  pSession - pointer to session
   *  pResponse - response buffer to read into
   *  pObjHeader - current request object header
   *  classMask - class of event to read
   * returns:
   *  void
   */
  SDNPSESN_READ_STATUS TMWDEFS_CALLBACK sdnpo004_readObj4v0ByClass(
    TMWSESN *pSession, 
    DNPUTIL_RX_MSG *pRequest,
    TMWSESN_TX_DATA *pResponse, 
    DNPUTIL_OBJECT_HEADER *pObjHeader, 
    TMWDEFS_CLASS_MASK classMask);

  /* function: sdnpo004_setDesc
   * purpose: Set device descriptor when switching from group 2 to 4
   * arguments:
   *  pDesc - pointer to descriptor 
   * returns:
   *  TNWDEFS_TRUE if 
   */
  void TMWDEFS_CALLBACK sdnpo004_setDesc(
    SDNPEVNT_DESC *pDesc);

#ifdef __cplusplus
}
#endif
#endif /* SDNPDATA_SUPPORT_OBJ4 */
#endif /* SDNPO004_DEFINED */
