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

/* file: sdnpo043.h
 * description: This file is intended for internal SCL use only.
 *   DNP SLAVE functionality for Object 43 Analog Output Command Change Event
 */
#ifndef SDNPO043_DEFINED
#define SDNPO043_DEFINED

#include "tmwscl/utils/tmwdefs.h"
#include "tmwscl/utils/tmwdtime.h"

#include "tmwscl/dnp/sdnpsesn.h"
#include "tmwscl/dnp/sdnpdata.h"
#include "tmwscl/dnp/sdnpevnt.h"

#if SDNPDATA_SUPPORT_OBJ43

#ifdef __cplusplus
extern "C" {
#endif

  /* function: sdnpo043_addEvent
   * purpose: Add an analog output command event to event queue. This indicates
   *  that a command has been attempted on an Analog Output point. This can be called
   *  in sdnpdata_anlgOutOperate() or when an anlg out command is performed locally. To add 
   *  this event when an invalid status was received sdnpo041.c could be modified
   *  to do so.
   * arguments:
   *  pSession - pointer to session
   *  point - point number of point that generated the event
   *  pValue - pointer to commanded value for this analog output point
   *  status - status from processing the command. 
   *    DNPDEFS_CTLSTAT_SUCCESS - the command was performed successfully
   *    DNPDEFS_CTLSTAT_FORMAT_ERROR - The request was not accepted due
   *      to formatting errors in the request. This value may also be use
   *      to indicate that the value in the request exceeds the permitted
   *      level (see Table 4-2 in DNP3 Specification Volume 2, Application
   *      Layer.)
   *    DNPDEFS_CTLSTAT_NO_SELECT - No previously matching select message
   *      (i.e., an Operate message was sent to activate a control point
   *      that was not previously armed with a Select message.)
   *    DNPDEFS_CTLSTAT_FORMAT_ERROR - the request was not accepted due
   *      to formatting errors in the request
   *    DNPDEFS_CTLSTAT_NOT_SUPPORTED - the request was not accepted because
   *      Control operations are not supported for this point
   *    DNPDEFS_CTLSTAT_ALREADY_ACTIVE - the request was not accepted because
   *      the control queue is full or the point is already active
   *    DNPDEFS_CTLSTAT_HARDWARE_ERROR - the request was not accepted due to
   *      control hardware problems
   *    DNPDEFS_CTLSTAT_LOCAL - the request was not accepted because 
   *      Local/Remote switch is in Local Position
   *    DNPDEFS_CTLSTAT_NOT_AUTHORIZED - the request was not accepted because 
   *      of insufficient authorization.
   *    DNPDEFS_CTLSTAT_AUTO_INHIBIT - the request was not accepted because it
   *      was prevented or inhibited by a local automation process  
   *    DNPDEFS_CTLSTAT_PROC_LIMITED - the request was not accepted because the
   *     device cannot process any more activities than are presently in progress 
   *    DNPDEFS_CTLSTAT_OUT_OF_RANGE - the request was not accepted because the 
   *     value is outside the acceptable range permitted for this point.  
   *    DNPDEFS_CTLSTAT_DOWNSTRM_LOCAL - the request was not accepted because the 
   *     control is being forwarded to a downwstream device that is reporting Local.
   *    DNPDEFS_CTLSTAT_ALR_COMPLETE - the request was not accepted because the 
   *     operation is already complete. For example if the request is to close a 
   *     switch and the switch is already closed.
   *    DNPDEFS_CTLSTAT_BLOCKED - the request was not accepted because is specifically
   *     blocked at the outstation.
   *    DNPDEFS_CTLSTAT_CANCELLED - the request was not accepted because the operation
   *     was cancelled.
   *    DNPDEFS_CTLSTAT_BLOCKED_OM - the request was not accepted because another 
   *     master has exclusive rights to operate this point.
   *    DNPDEFS_CTLSTAT_DOWNSTRM_FAIL -  the request was not accepted because the 
   *     control is being forwarded to a downwstream device which cannot be reached or
   *     is otherwise incapable of performing the request.
   *    DNPDEFS_CTLSTAT_UNDEFINED - the request not accepted because of some 
   *     other undefined reason
   *  pTimeStamp - pointer to time of event
   *   This should be gotten by calling sdnputil_getDateTime(). This will
   *   set the invalid field of the time structure properly. 
   *    Even if no time is available this SHALL not be TMWDEFS_NULL. You
   *    may choose to set the DNP3 time epoch ie Jan1 1970.
   * returns:
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL sdnpo043_addEvent(
    TMWSESN *pSession, 
    TMWTYPES_USHORT point, 
    TMWTYPES_ANALOG_VALUE *pValue, 
    TMWTYPES_UCHAR status, 
    TMWDTIME *pTimeStamp);

  /* function: sdnpo043_init
   * purpose: Initialize analog output command change events
   * arguments:
   *  pSession - pointer to session
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL sdnpo043_init(
    TMWSESN *pSession);

  /* function: sdnpo043_open
   * purpose: Start analog output command change event scanning if configured
   * arguments:
   *  pSession - pointer to session
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL sdnpo043_open(
    TMWSESN *pSession);

  /* function: sdnpo043_close
   * purpose: Close analog output command event processing
   * arguments:
   *  pSession - pointer to session
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL sdnpo043_close(
    TMWSESN *pSession);

  /* function: sdnpo043_countEvents
   * purpose: Count the number of analog output command events in
   *  queue
   * arguments:
   *  pSession - pointer to session
   *  classMask - count events in this class or classes
   * returns:
   *  void
   */
  TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpo043_countEvents(
    TMWSESN *pSession, 
    TMWDEFS_CLASS_MASK classMask,
    TMWTYPES_BOOL countAll,
    TMWTYPES_USHORT threshold);

  /* function: sdnpo043_cleanupEvents
   * purpose: Either remove events that have been sent or clear the
   *  sent flags if message was not acknowledged.
   * arguments:
   *  pSession - pointer to session
   *  deleteEvents - remove events if true, else clear sent flag
   * returns:
   *  void
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpo043_cleanupEvents(
    TMWSESN *pSession, 
    TMWTYPES_BOOL deleteEvents);

  /* function: sdnpo043_readObj43
   * purpose: Read analog output command events using specified variation
   * arguments:
   *  pSession - pointer to session
   *  pResponse - response buffer to read into
   *  pObjHeader - current request object header
   * returns:
   *  void
   */
  SDNPSESN_READ_STATUS TMWDEFS_CALLBACK sdnpo043_readObj43(
    TMWSESN *pSession, 
    DNPUTIL_RX_MSG *pRequest,
    TMWSESN_TX_DATA *pResponse, 
    DNPUTIL_OBJECT_HEADER *pObjHeader,
    SDNPSESN_QUAL qualifier);
  
  /* function: sdnpo043_readObj43v3ByClass
   * purpose: Read analog output command events using default variation for a 
   *  specific class
   * arguments:
   *  pSession - pointer to session
   *  pResponse - response buffer to read into
   *  pObjHeader - current request object header
   *  classMask - class of event to read
   * returns:
   *  void
   */
  SDNPSESN_READ_STATUS TMWDEFS_CALLBACK sdnpo043_readObj43v0ByClass(
    TMWSESN *pSession, 
    DNPUTIL_RX_MSG *pRequest,
    TMWSESN_TX_DATA *pResponse, 
    DNPUTIL_OBJECT_HEADER *pObjHeader,
    TMWDEFS_CLASS_MASK classMask);

#ifdef __cplusplus
}
#endif
#endif /* SDNPDATA_SUPPORT_OBJ43 */
#endif /* SDNPO043_DEFINED */
