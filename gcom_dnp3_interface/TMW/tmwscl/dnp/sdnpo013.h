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

/* file: sdnpo013.h
 * description: This file is intended for internal SCL use only.
 *   DNP SLAVE functionality for Object 13 Binary Output Command events.
 */
#ifndef SDNPO013_DEFINED
#define SDNPO013_DEFINED

#include "tmwscl/utils/tmwdefs.h"
#include "tmwscl/utils/tmwdtime.h"

#include "tmwscl/dnp/sdnpsesn.h"
#include "tmwscl/dnp/sdnpdata.h"
#include "tmwscl/dnp/sdnpevnt.h"

#if SDNPDATA_SUPPORT_OBJ13

#ifdef __cplusplus
extern "C" {
#endif

  /* function: sdnpo013_addEvent
   * purpose: Add a binary output command event to event queue. This indicates
   *  that a command has been attempted on a Binary Output point. This can be called
   *  in sdnpdata_binOutOperate() or when a bin out command is performed locally. To add 
   *  this event when an invalid status was received sdnpo012.c could be modified
   *  to do so.
   * arguments:
   *  pSession - pointer to session
   *  point - point number of point that generated the event
   *  status - status from processing the command. 
   *    DNPDEFS_CROB_ST_SUCCESS - the command was performed successfully
   *    DNPDEFS_CROB_ST_FORMAT_ERROR - The request was not accepted due
   *      to formatting errors in the request. This value may also be use
   *      to indicate that the value in the request exceeds the permitted
   *      level (see Table 4-2 in DNP3 Specification Volume 2, Application
   *      Layer.)
   *    DNPDEFS_CROB_ST_NOT_SUPPORTED - the request was not accepted because
   *      Control operations are not supported for this point
   *    DNPDEFS_CROB_ST_ALREADY_ACTIVE - the request was not accepted because
   *      the control queue is full or the point is already active
   *    DNPDEFS_CROB_ST_HARDWARE_ERROR - the request was not accepted due to
   *      control hardware problems
   *    DNPDEFS_CROB_ST_LOCAL - the request was not accepted because 
   *      Local/Remote switch is in Local Position
   *    DNPDEFS_CROB_ST_NOT_AUTHORIZED - the request was not accepted because 
   *      of insufficient authorization.
   *    DNPDEFS_CROB_ST_AUTO_INHIBIT - the request was not accepted because it
   *      was prevented or inhibited by a local automation process  
   *    DNPDEFS_CROB_ST_PROC_LIMITED - the request was not accepted because the
   *     device cannot process any more activities than are presently in progress 
   *    DNPDEFS_CROB_ST_OUT_OF_RANGE - the request was not accepted because the 
   *     value is outside the acceptable range permitted for this point.  
   *    DNPDEFS_CROB_ST_DOWNSTRM_LOCAL - the request was not accepted because the 
   *     control is being forwarded to a downwstream device that is reporting Local.
   *    DNPDEFS_CROB_ST_ALR_COMPLETE - the request was not accepted because the 
   *     operation is already complete. For example if the request is to close a 
   *     switch and the switch is already closed.
   *    DNPDEFS_CROB_ST_BLOCKED - the request was not accepted because is specifically
   *     blocked at the outstation.
   *    DNPDEFS_CROB_ST_CANCELLED - the request was not accepted because the operation
   *     was cancelled.
   *    DNPDEFS_CROB_ST_BLOCKED_OM - the request was not accepted because another 
   *     master has exclusive rights to operate this point.
   *    DNPDEFS_CROB_ST_DOWNSTRM_FAIL -  the request was not accepted because the 
   *     control is being forwarded to a downwstream device which cannot be reached or
   *     is otherwise incapable of performing the request.
   *    DNPDEFS_CROB_ST_UNDEFINED - the request not accepted because of some 
   *     other undefined reason
   *   Bit 7 of status contains the commanded state representing the control 
   *   requested for the output. 
   *    DNPDEFS_CROB_COMMANDED_STATE_OFF - Latch Off/Trip/NULL
   *    DNPDEFS_CROB_COMMANDED_STATE_ON  - Latch On/Close or the commanded state
   *     is unknown.
   *  pTimeStamp - pointer to time of event.
   *   This should be gotten by calling sdnputil_getDateTime(). This will
   *   set the invalid field of the time structure properly.
   *    Even if no time is available this SHALL not be TMWDEFS_NULL. You
   *    may choose to set the DNP3 time epoch ie Jan1 1970.
   * returns:
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL sdnpo013_addEvent(
    TMWSESN *pSession, 
    TMWTYPES_USHORT point, 
    TMWTYPES_UCHAR status, 
    TMWDTIME *pTimeStamp);

  /* function: sdnpo013_init
   * purpose: Initialize binary output command events
   * arguments:
   *  pSession - pointer to session
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL sdnpo013_init(
    TMWSESN *pSession); 
    
  /* function: sdnpo013_open
   * purpose: Start binary output scanning if configured
   * arguments:
   *  pSession - pointer to session
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL sdnpo013_open(
    TMWSESN *pSession);

  /* function: sdnpo013_close
   * purpose: Close binary output command event processing
   * arguments:
   *  pSession - pointer to session
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL sdnpo013_close(
    TMWSESN *pSession);

  /* function: sdnpo013_countEvents
   * purpose: Count the number of binary output command events in
   *  queue
   * arguments:
   *  pSession - pointer to session
   *  classMask - count events in this class or classes
   * returns:
   *  void
   */
  TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpo013_countEvents(
    TMWSESN *pSession, 
    TMWDEFS_CLASS_MASK classMask,
    TMWTYPES_BOOL countAll,
    TMWTYPES_USHORT threshold);

  /* function: sdnpo013_cleanupEvents
   * purpose: Either remove events that have been sent or clear the
   *  sent flags if message was not acknowledged.
   * arguments:
   *  pSession - pointer to session
   *  deleteEvents - remove events if true, else clear sent flag
   * returns:
   *  TMWDEFS_TRUE if specified queue is full
   *  TMWDEFS_FALSE if queue is not full
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpo013_cleanupEvents(
    TMWSESN *pSession, 
    TMWTYPES_BOOL deleteEvents);

  /* function: sdnpo013_readObj13
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
  SDNPSESN_READ_STATUS TMWDEFS_CALLBACK sdnpo013_readObj13(
    TMWSESN *pSession, 
    DNPUTIL_RX_MSG *pRequest,
    TMWSESN_TX_DATA *pResponse, 
    DNPUTIL_OBJECT_HEADER *pObjHeader,
    SDNPSESN_QUAL qualifier);
 
  /* function: sdnpo013_readObj13v0ByClass
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
  SDNPSESN_READ_STATUS TMWDEFS_CALLBACK sdnpo013_readObj13v0ByClass(
    TMWSESN *pSession, 
    DNPUTIL_RX_MSG *pRequest,
    TMWSESN_TX_DATA *pResponse, 
    DNPUTIL_OBJECT_HEADER *pObjHeader, 
    TMWDEFS_CLASS_MASK classMask);

  /* function: sdnpo013_setVariationInfo
   * purpose: Set the variation information
   * arguments:
   *  pDesc - pointer to descriptor
   *  variation - variation
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL sdnpo013_setVariationInfo(
    SDNPEVNT_DESC *pDesc, 
    TMWTYPES_UCHAR variation);

#ifdef __cplusplus
}
#endif
#endif /* SDNPDATA_SUPPORT_OBJ13 */
#endif /* SDNPO013_DEFINED */
