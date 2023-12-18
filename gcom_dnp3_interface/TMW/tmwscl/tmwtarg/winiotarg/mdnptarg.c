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

/* file: mdnptarg.c
 * description: Implementation of MDNP target layer functions for Windows
 */

#include "tmwscl/dnp/mdnptarg.h"

#if TMWTARG_SUPPORT_DNPFILEIO

#include <io.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
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
    return (NULL);
  }
  
#if _MSC_VER >= 1400
  fopen_s(&pFileHandle, pLocalFileName, pFileMode); 
#else
  pFileHandle = fopen(pLocalFileName, pFileMode); 
#endif
  return pFileHandle;
}

/* function: mdnptarg_closeLocalFile */
TMWTYPES_INT mdnptarg_closeLocalFile(
  FILE *pFileHandle)
{
  return fclose(pFileHandle);
}

/* function: mdnptarg_getLocalFileInfo */
TMWTYPES_BOOL mdnptarg_getLocalFileInfo(
  const TMWTYPES_CHAR *pLocalFileName,
  TMWTYPES_ULONG *pFileSize,
  TMWDTIME *pTimeOfCreation)
{
#if _MSC_VER >= 1400
  struct _finddata_t fileData;
  intptr_t fileHandle;
  if((fileHandle = _findfirst(pLocalFileName, &fileData)) > 0)
  {
    struct tm fileTime;

    /* Close find since we are done with this one */
    _findclose(fileHandle);
 
    /* convert file time of creation to tm structure */
    localtime_s(&fileTime, &fileData.time_create);


    /* convert tm structure to TMWDTIME structure */
    pTimeOfCreation->mSecsAndSecs = (TMWTYPES_USHORT)(fileTime.tm_sec * 1000);
    pTimeOfCreation->minutes      = (TMWTYPES_UCHAR)fileTime.tm_min;
    pTimeOfCreation->hour         = (TMWTYPES_UCHAR)fileTime.tm_hour;
    pTimeOfCreation->dayOfMonth   = (TMWTYPES_UCHAR)fileTime.tm_mday;
    pTimeOfCreation->month        = (TMWTYPES_UCHAR)(fileTime.tm_mon+1);
    pTimeOfCreation->year         = (TMWTYPES_USHORT)(fileTime.tm_year+1900);
    pTimeOfCreation->dstInEffect  = (TMWTYPES_BOOL)((fileTime.tm_isdst == 0) ? TMWDEFS_FALSE : TMWDEFS_TRUE);

    /* Monday through Friday is equivalent but Sunday = 0 */
    /* in tm structure but = 7 in TMWDTIME structure      */
    if(fileTime.tm_wday == 0)
      pTimeOfCreation->dayOfWeek = 7;
    else
      pTimeOfCreation->dayOfWeek = (TMWTYPES_UCHAR)fileTime.tm_wday;

    *pFileSize = fileData.size;
    return(TMWDEFS_TRUE);   
  }  
#endif
  return (TMWDEFS_FALSE);
}

/* function: mdnptarg_readLocalFile */
TMWTYPES_USHORT mdnptarg_readLocalFile(
  FILE *pFileHandle,
  TMWTYPES_UCHAR *pBuf,
  TMWTYPES_USHORT bufSize,
  TMWTYPES_BOOL *pLastBlock)
{
  int handle;
  TMWTYPES_USHORT readDataLen;
  if(pFileHandle == TMWDEFS_NULL)
  {
    return(0);
  }

  handle = _fileno(pFileHandle);
  readDataLen = (TMWTYPES_USHORT)_read(handle, pBuf, bufSize);

  /* Check for errors */
  if(ferror(pFileHandle))
  {
    return(0);
  }

  /* Check for end of file */
  *pLastBlock = (TMWTYPES_BOOL)(_eof(handle) ? TMWDEFS_TRUE : TMWDEFS_FALSE);
 
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

