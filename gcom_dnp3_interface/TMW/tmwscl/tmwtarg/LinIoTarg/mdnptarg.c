/*****************************************************************************/
/* Triangle MicroWorks, Inc.                         Copyright (c) 2008-2011 */
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

/* file: mdnptarg.c
 * description: Implementation of MDNP target layer functions for Linux
 */

#include "tmwscl/dnp/mdnptarg.h"

#if TMWTARG_SUPPORT_DNPFILEIO

#include <sys/stat.h>
#include "tmwscl/dnp/dnpcnfg.h"

/* function: mdnptarg_openLocalFile */
FILE *mdnptarg_openLocalFile(
  const TMWTYPES_CHAR *pLocalFileName,
  DNPDEFS_FILE_MODE fileMode)
{
  FILE *pFileHandle;
  char *pFileMode = "rb";

  /* Open local file */
  switch(fileMode)
  {
  case DNPDEFS_FILE_MODE_READ:
    pFileMode = "rb";
    break;
  case DNPDEFS_FILE_MODE_WRITE:
    pFileMode = "wb";
    break;
  default:
    return (TMWDEFS_FALSE);
  }
  
  pFileHandle = fopen(pLocalFileName, pFileMode); 

  return pFileHandle;
}

/* function: mdnptarg_closeLocalFile */
TMWTYPES_INT mdnptarg_closeLocalFile(
  FILE *pFileHandle)
{
  return fclose(pFileHandle);
}

static void _convertTime(TMWDTIME *pTmwDTime, const time_t *pTime_t)
{
  struct tm *pFileTime = localtime(pTime_t);

  /* convert tm structure to TMWDTIME structure */
  pTmwDTime->mSecsAndSecs = (TMWTYPES_USHORT)(pFileTime->tm_sec * 1000);
  pTmwDTime->minutes = (TMWTYPES_UCHAR)pFileTime->tm_min;
  pTmwDTime->hour = (TMWTYPES_UCHAR)pFileTime->tm_hour;
  pTmwDTime->dayOfMonth = (TMWTYPES_UCHAR)pFileTime->tm_mday;
  pTmwDTime->month = (TMWTYPES_UCHAR)(pFileTime->tm_mon + 1);
  pTmwDTime->year = (TMWTYPES_USHORT)(pFileTime->tm_year + 1900);
  pTmwDTime->dstInEffect = (TMWTYPES_BOOL)((pFileTime->tm_isdst == 0) ? TMWDEFS_FALSE : TMWDEFS_TRUE);

  /* Monday through Friday is equivalent but Sunday = 0 */
  /* in tm structure but = 7 in TMWDTIME structure      */
  if (pFileTime->tm_wday == 0)
    pTmwDTime->dayOfWeek = 7;
  else
    pTmwDTime->dayOfWeek = (TMWTYPES_UCHAR)pFileTime->tm_wday;
}

/* function: mdnptarg_getLocalFileInfo */
TMWTYPES_BOOL mdnptarg_getLocalFileInfo(
  const TMWTYPES_CHAR *pLocalFileName,
  TMWTYPES_ULONG *pFileSize,
  TMWDTIME *pTimeOfCreation)
{
  struct stat statbuf;
  if (stat(pLocalFileName, &statbuf) == 0)
  {
    *pFileSize = (TMWTYPES_ULONG)statbuf.st_size;
    _convertTime(pTimeOfCreation, &statbuf.st_ctim.tv_sec);
    return (TMWDEFS_TRUE);
  }
  return (TMWDEFS_FALSE);
}

/* function: mdnptarg_readLocalFile */
TMWTYPES_USHORT mdnptarg_readLocalFile(
  FILE *pFileHandle,
  TMWTYPES_UCHAR *pBuf,
  TMWTYPES_USHORT bufSize,
  TMWTYPES_BOOL *pLastBlock)
{
  TMWTYPES_USHORT readDataLen;
  if(pFileHandle == TMWDEFS_NULL)
  {
    return(0);
  }

  clearerr(pFileHandle);

  /* Read data */
  readDataLen = (TMWTYPES_USHORT)fread(pBuf, 1, bufSize, pFileHandle);

  /* Check for errors */
  if (ferror(pFileHandle))
    return(0);

  /* Check for end of file */
  *pLastBlock = (TMWTYPES_BOOL)(feof(pFileHandle) ? TMWDEFS_TRUE : TMWDEFS_FALSE);
 
  return(readDataLen);
}

/* function: mdnptarg_storeFileData */
TMWTYPES_BOOL mdnptarg_storeFileData(
  FILE *pFileHandle,
  TMWTYPES_USHORT nBytesInBlockData,
  const TMWTYPES_UCHAR *pBlockData)
{
  if (pFileHandle == TMWDEFS_NULL)
  {
    return(TMWDEFS_FALSE);
  }

  /* Check for errors */
  if (ferror(pFileHandle))
  {
    return(TMWDEFS_FALSE);
  }

  if (nBytesInBlockData > 0)
  {
    fwrite(pBlockData, 1, nBytesInBlockData, pFileHandle);
    fflush(pFileHandle);
  }
  return (TMWDEFS_TRUE);
}

#endif /* TMWTARG_SUPPORT_FILEIO */

