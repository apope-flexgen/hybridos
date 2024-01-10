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

/* file: sdnpo011.h
 * description: This file is intended for internal SCL use only.
 *   DNP SLAVE functionality for Object 11 Binary Output events.
 */
#ifndef SDNPO011_DEFINED
#define SDNPO011_DEFINED

#include "tmwscl/utils/tmwdefs.h"
#include "tmwscl/utils/tmwdtime.h"

#include "tmwscl/dnp/sdnpsesn.h"
#include "tmwscl/dnp/sdnpdata.h"
#include "tmwscl/dnp/sdnpevnt.h"

#if SDNPDATA_SUPPORT_OBJ11

#ifdef __cplusplus
extern "C" {
#endif

  /* function: sdnpo011_addEvent
   * purpose: Add a binary output event to event queue
   * arguments:
   *  pSession - pointer to session
   *  point - point number of point that generated the event
   *  flags - current DNP flags including state of output
   *   The following values (or OR'd combinations) are valid for this type:
   *      DNPDEFS_DBAS_FLAG_OFF_LINE - the point is off-line, and the returned
   *        state of this point may not be correct
   *      DNPDEFS_DBAS_FLAG_ON_LINE - the binary output point has been read
   *        successfully
   *      DNPDEFS_DBAS_FLAG_RESTART - the field device that originated the
   *        data object has been restarted. This device may be the device
   *        reporting this data object.
   *      DNPDEFS_DBAS_FLAG_COMM_LOST - the device reporting this data object
   *        has lost communication with the originator of the data object
   *      DNPDEFS_DBAS_FLAG_REMOTE_FORCED - the state of the binary object
   *        has been forced to its current state at the originating device
   *      DNPDEFS_DBAS_FLAG_LOCAL_FORCED - the state of the binary object
   *        has been forced to its current state at the device reporting
   *        this data object
   *      DNPDEFS_DBAS_FLAG_BINARY_ON  - the current state of the output (On)
   *      DNPDEFS_DBAS_FLAG_BINARY_OFF - the current state of the output (Off)
   *  pTimeStamp - pointer to time of event.
   *   This should be gotten by calling sdnputil_getDateTime(). This will
   *   set the invalid field of the time structure properly.  
   *    Even if no time is available this SHALL not be TMWDEFS_NULL. You
   *    may choose to set the DNP3 time epoch ie Jan1 1970.
   * returns:
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL sdnpo011_addEvent(
    TMWSESN *pSession, 
    TMWTYPES_USHORT point, 
    TMWTYPES_UCHAR flags, 
    TMWDTIME *pTimeStamp);

  /* function: sdnpo011_init
   * purpose: Initialize binary output events
   * arguments:
   *  pSession - pointer to session
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL sdnpo011_init(
    TMWSESN *pSession); 
    
  /* function: sdnpo011_open
   * purpose: Start binary output scanning if configured
   * arguments:
   *  pSession - pointer to session
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL sdnpo011_open(
    TMWSESN *pSession);

  /* function: sdnpo011_close
   * purpose: Close binary output event processing
   * arguments:
   *  pSession - pointer to session
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL sdnpo011_close(
    TMWSESN *pSession);

  /* function: sdnpo011_countEvents
   * purpose: Count the number of binary output events in
   *  queue
   * arguments:
   *  pSession - pointer to session
   *  classMask - count events in this class or classes
   * returns:
   *  void
   */
  TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpo011_countEvents(
    TMWSESN *pSession, 
    TMWDEFS_CLASS_MASK classMask,
    TMWTYPES_BOOL countAll,
    TMWTYPES_USHORT threshold);

  /* function: sdnpo011_cleanupEvents
   * purpose: Either remove events that have been sent or clear the
   *  sent flags if message was not acknowledged.
   * arguments:
   *  pSession - pointer to session
   *  deleteEvents - remove events if true, else clear sent flag
   * returns:
   *  TMWDEFS_TRUE if specified queue is full
   *  TMWDEFS_FALSE if queue is not full
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpo011_cleanupEvents(
    TMWSESN *pSession, 
    TMWTYPES_BOOL deleteEvents);

  /* function: sdnpo011_readObj11
   * purpose: Read binary output events using specified variation
   * arguments:
   *  pSession - pointer to session
   *  pResponse - response buffer to read into
   *  pObjHeader - current request object header
   * returns:
   *   SDNPSESN_READ_COMPLETE  if read succeeded, but there is no more data
   *   SDNPSESN_READ_MORE_DATA if read succeeded, and there is more data
   *   SDNPSESN_READ_FAILED if read failed
   */
  SDNPSESN_READ_STATUS TMWDEFS_CALLBACK sdnpo011_readObj11(
    TMWSESN *pSession, 
    DNPUTIL_RX_MSG *pRequest,
    TMWSESN_TX_DATA *pResponse, 
    DNPUTIL_OBJECT_HEADER *pObjHeader,
    SDNPSESN_QUAL qualifier);
 
  /* function: sdnpo011_readObj11v0ByClass
   * purpose: Read binary change events using default variation for a 
   *  specific class
   * arguments:
   *  pSession - pointer to session
   *  pResponse - response buffer to read into
   *  pObjHeader - current request object header
   *  classMask - class of event to read
   * returns:
   *  void
   */
  SDNPSESN_READ_STATUS TMWDEFS_CALLBACK sdnpo011_readObj11v0ByClass(
    TMWSESN *pSession, 
    DNPUTIL_RX_MSG *pRequest,
    TMWSESN_TX_DATA *pResponse, 
    DNPUTIL_OBJECT_HEADER *pObjHeader, 
    TMWDEFS_CLASS_MASK classMask);

  /* function: sdnpo011_setVariationInfo
   * purpose: Set the variation information
   * arguments:
   *  pDesc - pointer to descriptor
   *  variation - variation
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL sdnpo011_setVariationInfo(
    SDNPEVNT_DESC *pDesc, 
    TMWTYPES_UCHAR variation);

#ifdef __cplusplus
}
#endif
#endif /* SDNPDATA_SUPPORT_OBJ11 */
#endif /* SDNPO011_DEFINED */
