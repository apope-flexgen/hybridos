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

/* file: sdnpo033.h
 * description: This file is intended for internal SCL use only.
 *  DNP Slave functionality for Object 33 Frozen Analog Input Change events.
 */
#ifndef SDNPO033_DEFINED
#define SDNPO033_DEFINED

#include "tmwscl/utils/tmwdefs.h"
#include "tmwscl/utils/tmwdtime.h"

#include "tmwscl/dnp/sdnpsesn.h"
#include "tmwscl/dnp/sdnpdata.h"
#include "tmwscl/dnp/sdnpevnt.h"

#if SDNPDATA_SUPPORT_OBJ33


#ifdef __cplusplus
extern "C" {
#endif

  /* function: sdnpo033_addEvent
   * purpose: Add a frozen analog input event to event queue
   * arguments:
   *  pSession - pointer to session
   *  point - point number of point that generated the event
   *  value - value of counter
   *  flags - current DNP flags
   *  pTimeStamp - pointer to time of event
   *   This should be gotten by calling sdnputil_getDateTime(). This will
   *   set the invalid field of the time structure properly.
   *    Even if no time is available this SHALL not be TMWDEFS_NULL. You
   *    may choose to set the DNP3 time epoch ie Jan1 1970.
   * returns:
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL sdnpo033_addEvent(
    TMWSESN *pSession, 
    TMWTYPES_USHORT point, 
    TMWTYPES_ANALOG_VALUE *pValue, 
    TMWTYPES_UCHAR flags, 
    TMWDTIME *pTimeStamp);

  /* function: sdnpo033_init
   * purpose: Initialize frozen analog input change events
   * arguments:
   *  pSession - pointer to session
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL sdnpo033_init(
    TMWSESN *pSession);

  /* function: sdnpo033_open
   * purpose: Start frozen analog input change event scanning if configured
   * arguments:
   *  pSession - pointer to session
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL sdnpo033_open(
    TMWSESN *pSession);

  /* function: sdnpo033_close
   * purpose: Close frozen analog input event processing
   * arguments:
   *  pSession - pointer to session
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL sdnpo033_close(
    TMWSESN *pSession);

  /* function: sdnpo033_countEvents
   * purpose: Count the number of frozen analog input change events in
   *  queue
   * arguments:
   *  pSession - pointer to session
   *  classMask - count events in this class or classes
   * returns:
   *  void
   */
  TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpo033_countEvents(
    TMWSESN *pSession, 
    TMWDEFS_CLASS_MASK classMask,
    TMWTYPES_BOOL countAll,
    TMWTYPES_USHORT threshold);

  /* function: sdnpo033_cleanupEvents
   * purpose: Either remove events that have been sent or clear the
   *  sent flags if message was not acknowledged.
   * arguments:
   *  pSession - pointer to session
   *  deleteEvents - remove events if true, else clear sent flag
   * returns:
   *  void
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpo033_cleanupEvents(
    TMWSESN *pSession, 
    TMWTYPES_BOOL deleteEvents);

  /* function: sdnpo033_readObj33
   * purpose: Read frozen analog input change events using specified variation
   * arguments:
   *  pSession - pointer to session
   *  pResponse - response buffer to read into
   *  pObjHeader - current request object header
   * returns:
   *  void
   */
  SDNPSESN_READ_STATUS TMWDEFS_CALLBACK sdnpo033_readObj33(
    TMWSESN *pSession, 
    DNPUTIL_RX_MSG *pRequest,
    TMWSESN_TX_DATA *pResponse, 
    DNPUTIL_OBJECT_HEADER *pObjHeader,
    SDNPSESN_QUAL qualifier);

  /* function: sdnpo033_readObj33v0ByClass
   * purpose: Read frozen analog input change events using variation 0 for a specific 
   *  class
   * arguments:
   *  pSession - pointer to session
   *  pResponse - response buffer to read into
   *  pObjHeader - current request object header
   *  classMask - class of event to read
   * returns:
   *  void
   */
  SDNPSESN_READ_STATUS TMWDEFS_CALLBACK sdnpo033_readObj33v0ByClass(
    TMWSESN *pSession, 
    DNPUTIL_RX_MSG *pRequest,
    TMWSESN_TX_DATA *pResponse, 
    DNPUTIL_OBJECT_HEADER *pObjHeader,
    TMWDEFS_CLASS_MASK classMask);

#ifdef __cplusplus
}
#endif
#endif /* SDNPDATA_SUPPORT_OBJ33 */
#endif /* SDNPO033_DEFINED */
