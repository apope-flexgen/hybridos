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

/* file: sdnpo042.h
 * description: This file is intended for internal SCL use only.
 *   DNP SLAVE functionality for Object 42 Analog Output Change Event
 */
#ifndef SDNPO042_DEFINED
#define SDNPO042_DEFINED

#include "tmwscl/utils/tmwdefs.h"
#include "tmwscl/utils/tmwdtime.h"

#include "tmwscl/dnp/sdnpsesn.h"
#include "tmwscl/dnp/sdnpdata.h"
#include "tmwscl/dnp/sdnpevnt.h"

#if SDNPDATA_SUPPORT_OBJ42

#ifdef __cplusplus
extern "C" {
#endif

  /* function: sdnpo042_addEvent
   * purpose: Add a analog output event to event queue
   * arguments:
   *  pSession - pointer to session
   *  point - point number of point that generated the event
   *  pValue - pointer to value of analog output point
   *  flags - current DNP flags
   *   The following values (or OR'd combinations) are valid for this type:
   *      DNPDEFS_DBAS_FLAG_OFF_LINE - the point is off-line, and the returned
   *        state of this point may not be correct
   *      DNPDEFS_DBAS_FLAG_ON_LINE - the analog output point has been read
   *        successfully
   *      DNPDEFS_DBAS_FLAG_RESTART - the field device that originated the
   *        data object has been restarted. This device may be the device
   *        reporting this data object.
   *      DNPDEFS_DBAS_FLAG_COMM_LOST - the device reporting this data object
   *        has lost communication with the originator of the data object
   *      DNPDEFS_DBAS_FLAG_REMOTE_FORCED - the state of the analog object
   *        has been forced to its current state at the originating device
   *      DNPDEFS_DBAS_FLAG_LOCAL_FORCED - the state of the analog object
   *        has been forced to its current state at the device reporting
   *      DNPDEFS_DBAS_FLAG_OVER_RANGE - the digitized signal or calculation
   *        is greater than the type specified in TMWTYPES_ANALOG_VALUE. If the
   *        SCL determines that the value returned cannot fit in the type 
   *        specified by the object variation read it will set this OVER_RANGE bit.
   *        NOTE: If both TMWCNFG_SUPPORT_DOUBLE and TMWCNFG_SUPPORT_SFLOAT == 
   *        FALSE and the long value is over or under range, set
   *        DNPDEFS_DBAS_FLAG_OVER_RANGE bit in pFlags and set lval to 
   *        TMWDEFS_LONG_MIN or TMWDEFS_LONG_MAX.
   *  pTimeStamp - pointer to time of event
   *   This should be gotten by calling sdnputil_getDateTime(). This will
   *   set the invalid field of the time structure properly.  
   *    Even if no time is available this SHALL not be TMWDEFS_NULL. You
   *    may choose to set the DNP3 time epoch ie Jan1 1970.
   * returns:
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL sdnpo042_addEvent(
    TMWSESN *pSession, 
    TMWTYPES_USHORT point, 
    TMWTYPES_ANALOG_VALUE *pValue, 
    TMWTYPES_UCHAR flags, 
    TMWDTIME *pTimeStamp);

  /* function: sdnpo042_init
   * purpose: Initialize analog output change events
   * arguments:
   *  pSession - pointer to session
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL sdnpo042_init(
    TMWSESN *pSession);

  /* function: sdnpo042_open
   * purpose: Start analog output change event scanning if configured
   * arguments:
   *  pSession - pointer to session
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL sdnpo042_open(
    TMWSESN *pSession);


  /* function: sdnpo042_close
   * purpose: Close analog output event processing
   * arguments:
   *  pSession - pointer to session
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL sdnpo042_close(
    TMWSESN *pSession);

  /* function: sdnpo042_countEvents
   * purpose: Count the number of analog output change events in
   *  queue
   * arguments:
   *  pSession - pointer to session
   *  classMask - count events in this class or classes
   * returns:
   *  void
   */
  TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpo042_countEvents(
    TMWSESN *pSession, 
    TMWDEFS_CLASS_MASK classMask,
    TMWTYPES_BOOL countAll,
    TMWTYPES_USHORT threshold);

  /* function: sdnpo042_cleanupEvents
   * purpose: Either remove events that have been sent or clear the
   *  sent flags if message was not acknowledged.
   * arguments:
   *  pSession - pointer to session
   *  deleteEvents - remove events if true, else clear sent flag
   * returns:
   *  void
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpo042_cleanupEvents(
    TMWSESN *pSession, 
    TMWTYPES_BOOL deleteEvents);

  /* function: sdnpo042_readObj42
   * purpose: Read analog output change events using specified variation
   * arguments:
   *  pSession - pointer to session
   *  pResponse - response buffer to read into
   *  pObjHeader - current request object header
   * returns:
   *  void
   */
  SDNPSESN_READ_STATUS TMWDEFS_CALLBACK sdnpo042_readObj42(
    TMWSESN *pSession, 
    DNPUTIL_RX_MSG *pRequest,
    TMWSESN_TX_DATA *pResponse, 
    DNPUTIL_OBJECT_HEADER *pObjHeader,
    SDNPSESN_QUAL qualifier);
  
  /* function: sdnpo042_readObj42v3ByClass
   * purpose: Read analog output change events using default variation for a 
   *  specific class
   * arguments:
   *  pSession - pointer to session
   *  pResponse - response buffer to read into
   *  pObjHeader - current request object header
   *  classMask - class of event to read
   * returns:
   *  void
   */
  SDNPSESN_READ_STATUS TMWDEFS_CALLBACK sdnpo042_readObj42v0ByClass(
    TMWSESN *pSession, 
    DNPUTIL_RX_MSG *pRequest,
    TMWSESN_TX_DATA *pResponse, 
    DNPUTIL_OBJECT_HEADER *pObjHeader,
    TMWDEFS_CLASS_MASK classMask);

#ifdef __cplusplus
}
#endif
#endif /* SDNPDATA_SUPPORT_OBJ42 */
#endif /* SDNPO042_DEFINED */
