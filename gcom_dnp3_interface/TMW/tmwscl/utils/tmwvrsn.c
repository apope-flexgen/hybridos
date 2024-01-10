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
#include "tmwscl/utils/tmwtarg.h"
#include "tmwvrsn.h"

void tmwvrsn_getVersionInfo(
  unsigned int *pMajor,
  unsigned int *pMinor,
  unsigned int *pPatch,
  const char **ppDate)
{
  *pMajor = tmwvrsn_TMWVERSION_MAJOR;
  *pMinor = tmwvrsn_TMWVERSION_MINOR;
  *pPatch = tmwvrsn_TMWVERSION_PATCH;
  *ppDate = tmwvrsn_TMWVERSION_DATE;
}

void tmwvrsn_getVersionTime(
  int *pSec,
  int *pMin,
  int *pHour,
  int *pDay,
  int *pMonth,
  int *pYear)
{
  *pSec = 10;
  *pMin = 0;
  *pHour = 11;
  *pDay = 24;
  *pMonth = 5;
  *pYear = 2022;
}
