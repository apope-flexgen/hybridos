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

/* file: sdnptarg.c
 * description: Implementation of SDNP target layer functions for Linux
 */

#include "tmwscl/dnp/sdnptarg.h"

#if TMWTARG_SUPPORT_DNPFILEIO

#include <sys/stat.h>
#include <dirent.h>
#include "tmwscl/dnp/dnpdefs.h"
#include "tmwscl/dnp/dnpcnfg.h"


typedef struct SdnpTargFileDesc {
  FILE                    *pCurrentFile;
  DIR                     *pDir;
  struct dirent           *nextEntry;
  TMWTYPES_ULONG           dnpHandle;
  TMWTYPES_USHORT          blockSize;
  TMWTYPES_BOOL            inUse;
  TMWTYPES_CHAR            fileName[DNPCNFG_MAX_FILENAME];
  DNPDEFS_FILE_TYPE        dnpType;
  TMWTYPES_ULONG           size;
  TMWDTIME                 timeOfCreation;
  DNPDEFS_FILE_PERMISSIONS dnpPermissions;
} SDNPTARG_FILE_DESC;

static SDNPTARG_FILE_DESC fileDescs[DNPCNFG_MAX_FILES_OPEN] = { 0 };

static SDNPTARG_FILE_DESC * _allocFileDesc(void)
{
  TMWTYPES_ULONG descIndex;
  for (descIndex = 0; descIndex < DNPCNFG_MAX_FILES_OPEN; descIndex++)
  {
    if (fileDescs[descIndex].inUse == TMWDEFS_FALSE)
    {
      fileDescs[descIndex].inUse = TMWDEFS_TRUE;
      fileDescs[descIndex].dnpHandle = descIndex + SDNPTARG_FILE_HANDLE;
      return &fileDescs[descIndex];
    }
  }
  return TMWDEFS_NULL;
}

static SDNPTARG_FILE_DESC * _findFileDesc(TMWTYPES_ULONG  dnpHandle)
{
  TMWTYPES_ULONG descIndex;
  for (descIndex = 0; descIndex < DNPCNFG_MAX_FILES_OPEN; descIndex++)
  {
    if ((fileDescs[descIndex].inUse == TMWDEFS_TRUE) &&
      (fileDescs[descIndex].dnpHandle == dnpHandle))
    {
      return &fileDescs[descIndex];
    }
  }
  return TMWDEFS_NULL;
}

static void _freeFileDesc(SDNPTARG_FILE_DESC *pFileDesc)
{
  if (pFileDesc)
  {
    pFileDesc->inUse = TMWDEFS_FALSE;
    pFileDesc->pCurrentFile = TMWDEFS_NULL;
    pFileDesc->pDir  = TMWDEFS_NULL;
    pFileDesc->nextEntry  = TMWDEFS_NULL;
  }
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

static DNPDEFS_FILE_PERMISSIONS _convertPermissions(mode_t st_mode)
{
  /* Map Linux file permissions to DNP Permissions */
  return  ((st_mode & S_IRWXO) |   /* others to WORLD */
           (st_mode & S_IRWXG) |   /* group */
           (st_mode & S_IRWXU));   /* owner */
}

/* function: _directorySize */
static size_t TMWDEFS_LOCAL _directorySize(
  char *pDirName)
{
  DIR * dirp;
  struct dirent * entry;
  size_t dirSize = 0;

  /* For a directory, size is the number of octets that would be transferred by 
   * the read requests. This is (the number of files contained in the directory * 20)
   * + (sum of the lengths of the names of all of the files in the directory).
   */
  dirp = opendir(pDirName);
  if (dirp)
  {
    while ((entry = readdir(dirp)) != NULL) {
      if (((entry->d_type == DT_REG) || (entry->d_type == DT_DIR)) &&
        strcmp(entry->d_name, ".") && strcmp(entry->d_name, ".."))
      { 
        /* exclude links to itself and parent "."  ".." since they are not sent */
#if 0
        dirSize +=20;
        dirSize += strlen(entry->d_name);
#else
        dirSize++;
#endif
      }
    }
    closedir(dirp);
  }
  return dirSize;
}

static TMWTYPES_BOOL _buildInfoResponse(
  SDNPTARG_FILE_DESC       *pFileDesc,
  TMWTYPES_USHORT           maxNameSize,
  char                     *pFilename,
  DNPDEFS_FILE_TYPE        *pType,
  TMWTYPES_ULONG           *pSize,
  TMWDTIME                 *pTimeOfCreation,
  DNPDEFS_FILE_PERMISSIONS *pPermissions)
{
  struct dirent *entry = pFileDesc->nextEntry;
  char fileName[DNPCNFG_MAX_FILENAME+1];

  if (entry == NULL)
  {
    entry = readdir(pFileDesc->pDir);
  }
  while ((entry != NULL) && (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")))
  {
    entry = readdir(pFileDesc->pDir);
  }

  if (entry != NULL)
  {
    strncpy(fileName, pFileDesc->fileName, DNPCNFG_MAX_FILENAME);
    strncat(fileName, "/", DNPCNFG_MAX_FILENAME);
    strncat(fileName, entry->d_name, DNPCNFG_MAX_FILENAME);
    sdnptarg_getFileInfo(fileName, pType, pSize, pTimeOfCreation, pPermissions);
    strncpy(pFilename, entry->d_name, maxNameSize);
    pFileDesc->nextEntry = readdir(pFileDesc->pDir);
    return TMWDEFS_TRUE;
  }
  return TMWDEFS_FALSE;
}

/* function: sdnptarg_getFileInfo
 * purpose: return information about file or directory
 *  This is used to build a response to a FC 28 Get File Info request.
 */
DNPDEFS_FILE_CMD_STAT TMWDEFS_GLOBAL sdnptarg_getFileInfo(
  TMWTYPES_CHAR *pFilename,
  DNPDEFS_FILE_TYPE *pType,
  TMWTYPES_ULONG *pSize,
  TMWDTIME *pTimeOfCreation,
  DNPDEFS_FILE_PERMISSIONS *pPermissions)
{
  struct stat statbuf;

  if (stat(pFilename, &statbuf) == 0)
  {
    if (S_ISREG(statbuf.st_mode))
    {
      *pType = DNPDEFS_FILE_TYPE_SIMPLE;
      *pSize = (TMWTYPES_ULONG)statbuf.st_size;
      _convertTime(pTimeOfCreation, &statbuf.st_ctim.tv_sec);
      *pPermissions = _convertPermissions(statbuf.st_mode);
      return DNPDEFS_FILE_CMD_STAT_SUCCESS;
    }
    else if (S_ISDIR(statbuf.st_mode))
    {
      *pType = DNPDEFS_FILE_TYPE_DIRECTORY;
      _convertTime(pTimeOfCreation, &statbuf.st_ctim.tv_sec);
      *pPermissions = _convertPermissions(statbuf.st_mode);

      /* The directory size should report the number of files and subdirectories it contains. */
      (*pSize) = 0;
      {
        DIR * dirp;
        struct dirent * entry;

        dirp = opendir(pFilename);
        if (dirp == NULL)
        {
          return DNPDEFS_FILE_CMD_STAT_NOT_FOUND;
        }
        while ((entry = readdir(dirp)) != NULL) {
          if (((entry->d_type == DT_REG) || (entry->d_type == DT_DIR)) &&
              strcmp(entry->d_name, ".") && strcmp(entry->d_name, ".."))
          { /* If the entry is a regular file */
            (*pSize)++;
          }
        }
        closedir(dirp);
      }
      return DNPDEFS_FILE_CMD_STAT_SUCCESS;
    }
  }

  return DNPDEFS_FILE_CMD_STAT_NOT_FOUND;
}

/* function: sdnptarg_readFileInfo
 * purpose: return information about file or subdirectory
 *  This is used to build a response to a FC 1 read request on a directory.
 *  This function will be called multiple times. It should set pLast to
 *  TRUE when the last entry in a directory is being read.
 */
DNPDEFS_FILE_TFER_STAT TMWDEFS_GLOBAL sdnptarg_readFileInfo(
  TMWTYPES_ULONG handle,
  TMWTYPES_USHORT maxNameSize,
  TMWTYPES_CHAR *pName,
  TMWTYPES_BOOL *pLast,
  DNPDEFS_FILE_TYPE *pType,
  TMWTYPES_ULONG *pSize,
  TMWDTIME *pTimeOfCreation,
  DNPDEFS_FILE_PERMISSIONS *pPermissions)
{
  SDNPTARG_FILE_DESC *pFileDesc = _findFileDesc(handle);

  /* Validate file state against request */
  if (pFileDesc == TMWDEFS_NULL)
  {
    return(DNPDEFS_FILE_TFER_STAT_INV_HANDLE);
  }

  if ((pFileDesc->pCurrentFile == TMWDEFS_NULL) &&
      (pFileDesc->pDir == TMWDEFS_NULL))
  {
    return(DNPDEFS_FILE_TFER_STAT_NOT_OPEN);
  }

  _buildInfoResponse(pFileDesc, maxNameSize, pName, pType, pSize, pTimeOfCreation, pPermissions);

  if (pFileDesc->nextEntry == NULL)
  {
    *pLast = TMWDEFS_TRUE;
  }
  else
  {
    *pLast = TMWDEFS_FALSE;
  }

  return DNPDEFS_FILE_TFER_STAT_SUCCESS;
}

/* function: sdnptarg_deleteFile
 * purpose: delete the requested file
 */
DNPDEFS_FILE_CMD_STAT TMWDEFS_GLOBAL sdnptarg_deleteFile(
  TMWTYPES_CHAR *pFilename)
{
  /* Delete requested file */
  if(remove(pFilename))
    return(DNPDEFS_FILE_CMD_STAT_NOT_FOUND);

  /* Return success */
  return(DNPDEFS_FILE_CMD_STAT_SUCCESS);
}

/* function: sdnptarg_openFile
 * purpose: open specified file
 */
DNPDEFS_FILE_CMD_STAT TMWDEFS_GLOBAL sdnptarg_openFile(
  TMWTYPES_CHAR *pFilename,
  DNPDEFS_FILE_MODE mode,
  TMWTYPES_USHORT *pMaxBlockSize,
  DNPDEFS_FILE_PERMISSIONS *pPermissions,
  TMWDTIME *pTimeOfCreation,
  TMWTYPES_ULONG *pHandle,
  TMWTYPES_ULONG *pSize,
  DNPDEFS_FILE_TYPE *pType)
{
  char *pFileMode;
  FILE *pFile;
  SDNPTARG_FILE_DESC *pFileDesc;

  /* If mode is read, see if the requested file is a directory */
  if(mode == DNPDEFS_FILE_MODE_READ)
  {
    DNPDEFS_FILE_CMD_STAT cmdStat = sdnptarg_getFileInfo(pFilename, pType, pSize, pTimeOfCreation, pPermissions);
    if (cmdStat != DNPDEFS_FILE_CMD_STAT_SUCCESS)
    {
      return (cmdStat);
    }
    if(*pType == DNPDEFS_FILE_TYPE_DIRECTORY)
    {
      /* Yep, it's a directory setup a directory read */
      *pSize = (TMWTYPES_ULONG)_directorySize(pFilename);

      pFileDesc = _allocFileDesc();
      if (pFileDesc)
      {
        /* Initialize file state */
        pFileDesc->blockSize = *pMaxBlockSize;
        pFileDesc->pDir = opendir(pFilename);
        pFileDesc->dnpType = DNPDEFS_FILE_TYPE_DIRECTORY;
        pFileDesc->size = *pSize;
        pFileDesc->timeOfCreation = *pTimeOfCreation;
        pFileDesc->dnpPermissions = *pPermissions;
        strncpy(pFileDesc->fileName, pFilename, DNPCNFG_MAX_FILENAME - 1);
        *pHandle = pFileDesc->dnpHandle;

        /* Return success */
        return(DNPDEFS_FILE_CMD_STAT_SUCCESS);
      }

      /* Clear file handle */
      *pHandle = 0;
      return (DNPDEFS_FILE_CMD_STAT_TOO_MANY);
    }
  }

  /* Get file mode */
  switch(mode)
  {
  case DNPDEFS_FILE_MODE_READ:
    pFileMode = "rb";
    break;

  case DNPDEFS_FILE_MODE_WRITE:
    pFileMode = "wb";
    break;

  case DNPDEFS_FILE_MODE_APPEND:
    pFileMode = "ab";
    break;

  default:
    return(DNPDEFS_FILE_CMD_STAT_INV_MODE);
  }
    
  if((pFile = fopen(pFilename, pFileMode)) != TMWDEFS_NULL)
  {
    pFileDesc = _allocFileDesc();
    if (pFileDesc)
    {
      /* Initialize file state */
      pFileDesc->blockSize = *pMaxBlockSize;
      pFileDesc->pCurrentFile = pFile;
      pFileDesc->dnpType = DNPDEFS_FILE_TYPE_SIMPLE;
      pFileDesc->size = *pSize;
      pFileDesc->timeOfCreation = *pTimeOfCreation;
      pFileDesc->dnpPermissions = *pPermissions;
      *pHandle = pFileDesc->dnpHandle;

      /* Return success */
      return(DNPDEFS_FILE_CMD_STAT_SUCCESS);
    }

    /* Clear file handle */
    *pHandle = 0;
    fclose(pFile);
    return (DNPDEFS_FILE_CMD_STAT_TOO_MANY);
  }
  /* Clear file handle */
  *pHandle = 0;

  /* Return error */
  return(DNPDEFS_FILE_CMD_STAT_NOT_FOUND);
}

/* function: sdnptarg_closeFile
 * purpose: close existing file
 */
DNPDEFS_FILE_CMD_STAT TMWDEFS_GLOBAL sdnptarg_closeFile(
  TMWTYPES_ULONG handle)
{
  SDNPTARG_FILE_DESC    *pFileDesc = _findFileDesc(handle);
  DNPDEFS_FILE_CMD_STAT cmdStat    = DNPDEFS_FILE_CMD_STAT_NOT_FOUND;

  /* Validate file handle */
  if (pFileDesc == TMWDEFS_NULL)
    return(DNPDEFS_FILE_CMD_STAT_INV_HANDLE);

  if (pFileDesc->pCurrentFile != TMWDEFS_NULL)
  {
    /* Close file */
    fclose(pFileDesc->pCurrentFile);
    cmdStat = DNPDEFS_FILE_CMD_STAT_SUCCESS;
  }

  if (pFileDesc->pDir != TMWDEFS_NULL)
  {
    /* Close directory */
    closedir(pFileDesc->pDir);
    cmdStat = DNPDEFS_FILE_CMD_STAT_SUCCESS;
  }

  /* Free the file descriptor */
  _freeFileDesc(pFileDesc);

  return cmdStat;
}

/* function: sdnptarg_readFile
 * purpose: read block of data from file.
 */
DNPDEFS_FILE_TFER_STAT TMWDEFS_GLOBAL sdnptarg_readFile(
  TMWTYPES_ULONG handle,
  TMWTYPES_BOOL *pLast,
  TMWTYPES_USHORT *pBytesRead,
  TMWTYPES_UCHAR *pBuf)
{
  SDNPTARG_FILE_DESC *pFileDesc = _findFileDesc(handle);

  /* Validate file state against request */
  if (pFileDesc == TMWDEFS_NULL)
  {
    return(DNPDEFS_FILE_TFER_STAT_INV_HANDLE);
  }

  if (pFileDesc->pCurrentFile == TMWDEFS_NULL)
  {
    return(DNPDEFS_FILE_TFER_STAT_NOT_OPEN);
  }

  clearerr(pFileDesc->pCurrentFile);

  /* Read data */
  *pBytesRead = (TMWTYPES_USHORT)fread(pBuf, 1, pFileDesc->blockSize, pFileDesc->pCurrentFile);

  /* Check for errors */
  if (ferror(pFileDesc->pCurrentFile))
    return(DNPDEFS_FILE_TFER_STAT_BAD_FILE);

  /* Check for end of file */
  *pLast = (TMWTYPES_BOOL)(feof(pFileDesc->pCurrentFile) ? TMWDEFS_TRUE : TMWDEFS_FALSE);
  return DNPDEFS_FILE_TFER_STAT_SUCCESS;
}

/* function: sdnptarg_writeFile
 * purpose: write block of data to file
 */
DNPDEFS_FILE_TFER_STAT TMWDEFS_GLOBAL sdnptarg_writeFile(
  TMWTYPES_ULONG handle,
  TMWTYPES_BOOL last,
  TMWTYPES_USHORT numBytes,
  TMWTYPES_UCHAR *pBuf)
{
  SDNPTARG_FILE_DESC *pFileDesc = _findFileDesc(handle);
  TMWTARG_UNUSED_PARAM(last);

  /* Validate file state against request */
  if (pFileDesc == TMWDEFS_NULL)
  {
    return(DNPDEFS_FILE_TFER_STAT_INV_HANDLE);
  }

  if (pFileDesc->pCurrentFile == TMWDEFS_NULL)
  {
    return(DNPDEFS_FILE_TFER_STAT_NOT_OPEN);
  }

  if (numBytes > pFileDesc->blockSize)
  {
    return(DNPDEFS_FILE_TFER_STAT_OVERRUN);
  }

   /* Write data */
  if(fwrite(pBuf, 1, numBytes, pFileDesc->pCurrentFile) != numBytes)
  {
    return(DNPDEFS_FILE_TFER_STAT_BAD_FILE);
  }

  /* Return success */
  return(DNPDEFS_FILE_TFER_STAT_SUCCESS);
}

#endif /* TMWTARG_SUPPORT_FILEIO */

