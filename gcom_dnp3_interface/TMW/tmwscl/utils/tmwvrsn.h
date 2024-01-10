/*****************************************************************************/
/* Triangle MicroWorks, Inc.                         Copyright (c) 1997-2021 */
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
#ifndef tmwvrsn_TMWVERSION
#define tmwvrsn_TMWVERSION

#define tmwvrsn_TMWVERSION_MAJOR 3
#define tmwvrsn_TMWVERSION_MINOR 30
#define tmwvrsn_TMWVERSION_PATCH 0
#define tmwvrsn_TMWVERSION_BUILD 855
#define tmwvrsn_TMWVERSION_STRING "3.30.0000"
#define tmwvrsn_MANAGED_SCL_VERSION_STRING "3.30.0000.0000"

#define tmwvrsn_TMWVERSION_DATE "Tue May 24 11:00:10 2022"

#define tmwvrsn_TMWVERSION_DATE_SEC 10
#define tmwvrsn_TMWVERSION_DATE_MIN 0
#define tmwvrsn_TMWVERSION_DATE_HOUR 11
#define tmwvrsn_TMWVERSION_DATE_DAY 24
#define tmwvrsn_TMWVERSION_DATE_MONTH 5
#define tmwvrsn_TMWVERSION_DATE_YEAR 2022

#ifdef __cplusplus
extern "C" {
#endif

extern void tmwvrsn_getVersionInfo(
  unsigned int *pMajor,
  unsigned int *pMinor,
  unsigned int *pPatch,
  const char **ppDate);

extern void tmwvrsn_getVersionTime(
  int *pSec,
  int *pMin,
  int *pHour,
  int *pDay,
  int *pMonth,
  int *pYear);

#ifdef __cplusplus
}
#endif
#endif
