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

/* file: sdnpunsl.h
 * description: This file is intended for internal SCL use only.
 *   Slave DNP Unsolicited Response Support
 */
#ifndef SDNPUNSL_DEFINED
#define SDNPUNSL_DEFINED

#include "tmwscl/dnp/sdnpsesn.h"

#ifdef __cplusplus
extern "C" {
#endif

  /* function: sdnpunsl_sendNullResponse */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpunsl_sendNullResponse(
    TMWSESN *pSession,
    TMWTYPES_BOOL initialNullResponse);

  /* function: sdnpunsl_addEvent
   * purpose:
   * arguments:
   * returns:
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL sdnpunsl_addEvent(
    TMWSESN *pSession, 
    TMWDEFS_CLASS_MASK classMask);

  /* function: sdnpunsl_removeEvent 
   * purpose: If event is removed from list and not sent, update the unsol number pending for that class
   * arguments:
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL sdnpunsl_removeEvent(
    SDNPSESN *pSDNPSession,
    SDNPEVNT *pEvent);

  /* function: sdnpunsl_updateStatus
   * purpose:
   * arguments:
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL sdnpunsl_updateStatus(
    TMWSESN *pSession);

  /* function: sdnpunsl_countUnsolicitedEvents
   * purpose:
   * arguments:
   * returns:
   *  void
   */
  TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpunsl_countUnsolicitedEvents(
    TMWSESN *pSession,
    TMWDEFS_CLASS_MASK classMask);

  /* function: sdnpunsl_processUnsolEvents
   * purpose:
   * arguments:
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL sdnpunsl_processUnsolEvents(
    TMWSESN *pSession,
    TMWDEFS_CLASS_MASK classMask);

#ifdef __cplusplus
}
#endif
#endif /* SDNPUNSL_DEFINED */
