/*****************************************************************************/
/* Triangle MicroWorks, Inc.                         Copyright (c) 2008-2019 */
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


/* file: sampleiodiag.c
* description: Implementation of sample diagnostic routines
*/
#include "sampleiodiag.h"


#if TMWCNFG_SUPPORT_DIAG
void TMWDEFS_GLOBAL sampleiotarg_diagMsg(const char *format, ...)  
{
  TMWDIAG_ANLZ_ID anlzId;
  TMWTYPES_INT bufLen;
  TMWTYPES_INT remLen;
  char buf[256];
 
  if (tmwdiag_initId(&anlzId, TMWDEFS_NULL, TMWDEFS_NULL, TMWDEFS_NULL, TMWDIAG_ID_TARGET) == TMWDEFS_FALSE)
  {
    return;
  }

  bufLen = tmwtarg_snprintf(buf, sizeof(buf), "### ");

  /* Ensure there is enough room for the \n */
  remLen = sizeof(buf) - bufLen - 1; 

  va_list va;
  va_start(va, format);
#if TMWCNFG_HAS_VSNPRINTF
  (void)vsnprintf(buf+bufLen, remLen, format, va);
#else
  (void)vsprintf(buf+bufLen, format, va);
#endif
  va_end(va);

  strcat(buf, "\n");

  tmwdiag_putLine(&anlzId, buf);
}

void TMWDEFS_GLOBAL sampleiotarg_diagErrorMsg(const char *format, ...)  
{
  TMWDIAG_ANLZ_ID anlzId;
  TMWTYPES_INT bufLen;
  TMWTYPES_INT remLen;
  char buf[256];
  
  if (tmwdiag_initId(&anlzId, TMWDEFS_NULL, TMWDEFS_NULL, TMWDEFS_NULL, TMWDIAG_ID_TARGET | TMWDIAG_ID_ERROR) == TMWDEFS_FALSE)
  {
    return;
  }
 
  bufLen = tmwtarg_snprintf(buf, sizeof(buf), "### **** ");

  /* Ensure there is enough room for the  ****\n */
  remLen = sizeof(buf) - bufLen - 6; 

  va_list va;
  va_start(va, format);
#if TMWCNFG_HAS_VSNPRINTF
  (void)vsnprintf(buf+bufLen, remLen, format, va);
#else
  (void)vsprintf(buf+bufLen, format, va);
#endif
  va_end(va);

  strcat(buf, " ****\n");

  tmwdiag_putLine(&anlzId, buf);
}
#endif

