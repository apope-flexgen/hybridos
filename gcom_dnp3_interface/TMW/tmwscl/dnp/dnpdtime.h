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

/* file: dnpdtime.h
 * description: Support for DNP 6 byte time
 */
#ifndef DNPDTIME_DEFINED
#define DNPDTIME_DEFINED

#include "tmwscl/utils/tmwdefs.h"
#include "tmwscl/utils/tmwdtime.h"

#ifdef __cplusplus
extern "C" 
{
#endif

  /* function: dnpdtime_dateTimeToMSSince70
   * purpose: convert standard time to DNP time
   * arguments:
   *  pMsSince70 - DNP time to store results
   *  pDateTime - standard time to convert
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL dnpdtime_dateTimeToMSSince70(
    TMWTYPES_MS_SINCE_70 *pMsSince70, 
    TMWDTIME *pDateTime);

  /* function: dnpdtime_msSince70ToDateTime
   * purpose: convert DNP time to standard time
   * arguments:
   *  pDateTime - standard time to store results
   *  pMsSince70 - DNP time to convert
   * returns:
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL dnpdtime_msSince70ToDateTime(
    TMWDTIME *pDateTime, 
    const TMWTYPES_MS_SINCE_70 *pMsSince70);

  /* function: dnpdtime_writeMsSince70
   * purpose: write DNP time into buffer
   * arguments:
   *  destination - destination buffer for time
   *  pMsSince70 - DNP time to store
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL dnpdtime_writeMsSince70(
    TMWTYPES_UCHAR *destination, 
    const TMWTYPES_MS_SINCE_70 *pMsSince70);

  /* function: dnpdtime_readMsSince70
   * purpose: read DNP time from buffer
   * arguments:
   *  pMsSince70 - DNP time to store into
   *  source - source buffer to read time from
   * returns:
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL dnpdtime_readMsSince70(
    TMWTYPES_MS_SINCE_70 *pMsSince70, 
    const TMWTYPES_UCHAR *source);

  /* function: dnpdtime_addTime
   * purpose: add specified number of milliseconds to DNP time
   * arguments:
   *  pDestMsSince70 - destination DNP time for results
   *  pOperMsSince70 - source DNP time to add to
   *  milliseconds - number of milliseconds to add
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL dnpdtime_addTime(
    TMWTYPES_MS_SINCE_70 *pDestMsSince70, 
    const TMWTYPES_MS_SINCE_70 *pOperMsSince70, 
    TMWTYPES_ULONG milliseconds);

  /* function: dnpdtime_subtractTime
   * purpose: subtract two DNP times
   * arguments:
   *  pDestMsSince70 - destination DNP time for results
   *  pOper1MsSince70 - source DNP time to subtract from
   *  pOper2MsSince70 - DNP time to subtract
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL dnpdtime_subtractTime(
    TMWTYPES_MS_SINCE_70 *pDestMsSince70,
    const TMWTYPES_MS_SINCE_70 *pOper1MsSince70,  
    const TMWTYPES_MS_SINCE_70 *pOper2MsSince70);

#ifdef __cplusplus
}
#endif
#endif /* DNPDTIME_DEFINED */

