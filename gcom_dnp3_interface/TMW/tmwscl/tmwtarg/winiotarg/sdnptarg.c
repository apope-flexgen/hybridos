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

/* file: sdnptarg.c
 * description: Implementation of SDNP target layer functions for Windows
 */

#include "tmwscl/dnp/sdnptarg.h"

#if TMWTARG_SUPPORT_DNPFILEIO

#include <io.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tmwscl/dnp/dnpcnfg.h"


typedef struct SdnpTargFileDesc {
  FILE                    *pCurrentFile;
  struct _finddata_t       findData;
  intptr_t                 findHandle;
  TMWTYPES_ULONG           dnpHandle;
  TMWTYPES_USHORT          blockSize;
  TMWTYPES_BOOL            inUse;
  TMWTYPES_CHAR            findFilename[DNPCNFG_MAX_FILENAME];
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
    pFileDesc->findHandle = 0;
    
  }
}

/* function: _buildInfoResponse */
static TMWTYPES_BOOL _buildInfoResponse(
  struct _finddata_t       *pFileData,
  const char               *pFilename,
  DNPDEFS_FILE_TYPE        *pType,
  TMWTYPES_ULONG           *pSize,
  TMWDTIME                 *pTimeOfCreation,
  DNPDEFS_FILE_PERMISSIONS *pPermissions)
{
  struct tm fileTime;

  /* convert file time of creation to tm structure */
  localtime_s(&fileTime, &pFileData->time_create);

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

  /* See what type of file this is */
  if((pFileData->attrib & _A_SUBDIR) != _A_SUBDIR)
  {
    /* Simple file */
    *pType = DNPDEFS_FILE_TYPE_SIMPLE;
    *pSize = pFileData->size;

    if((pFileData->attrib & _A_RDONLY) == _A_RDONLY)
      *pPermissions = DNPDEFS_WORLD_READ_ALLOWED|DNPDEFS_GROUP_READ_ALLOWED|DNPDEFS_OWNER_READ_ALLOWED;
    else
      *pPermissions = 0x1ff;
  }
  else
  {
    /* Subdirectory, file size should be number of files in directory */
    TMWTYPES_USHORT numFiles = 0;
    struct _finddata_t fileData;
    char filename[DNPCNFG_MAX_FILENAME];
    intptr_t fileHandle;

    STRCPY(filename, DNPCNFG_MAX_FILENAME, pFilename);
    STRCAT(filename, DNPCNFG_MAX_FILENAME, "/*.*");

    /* Count files in 'pFilename\*.*' */
    if((fileHandle = _findfirst(filename, &fileData)) > 0)
    {
      numFiles = 1;

      while(_findnext(fileHandle, &fileData) == 0)
        numFiles += 1;

      _findclose(fileHandle);
    }

    /* exclude links to itself and parent "."  ".." according to Application Layer Draft H 6.74 */
    *pSize = numFiles-2;
    *pType = DNPDEFS_FILE_TYPE_DIRECTORY;
    if((pFileData->attrib & _A_RDONLY) == _A_RDONLY)
      *pPermissions = DNPDEFS_WORLD_READ_ALLOWED|DNPDEFS_GROUP_READ_ALLOWED|DNPDEFS_OWNER_READ_ALLOWED;
    else
      *pPermissions = 0x1ff;
  }
  return TMWDEFS_TRUE;
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
  struct _finddata_t fileData;
  intptr_t fileHandle;

  /* Get file info */
  char tmpFileNameBuf[DNPCNFG_MAX_FILENAME+1];

  STRCPY(tmpFileNameBuf, (DNPCNFG_MAX_FILENAME+1), pFilename);

  if (tmpFileNameBuf[strlen(tmpFileNameBuf)-1] == '/' || tmpFileNameBuf[strlen(tmpFileNameBuf)-1] == '\\')
  { /* if last char is a '/' remove it */
    tmpFileNameBuf[strlen(tmpFileNameBuf)-1] = 0;
  }

  if ((fileHandle = _findfirst(tmpFileNameBuf, &fileData)) > 0)
  {
    /* Close find since we are done with this one */
    _findclose(fileHandle);

    /* Get file info for response */
    _buildInfoResponse(&fileData, tmpFileNameBuf, pType, pSize, pTimeOfCreation, pPermissions);

    return(DNPDEFS_FILE_CMD_STAT_SUCCESS);
  }
  return(DNPDEFS_FILE_CMD_STAT_NOT_FOUND);
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
  char tmpName[DNPCNFG_MAX_FILENAME];
  size_t fileNameLen;

  /* Validate file state against request */
  if (pFileDesc == TMWDEFS_NULL)
  {
    return(DNPDEFS_FILE_TFER_STAT_INV_HANDLE);
  }

  if (pFileDesc->findHandle <= 0)
  {
    return(DNPDEFS_FILE_TFER_STAT_NOT_OPEN);
  }

  STRCPY(tmpName, DNPCNFG_MAX_FILENAME, pFileDesc->findFilename);
  STRCAT(tmpName, DNPCNFG_MAX_FILENAME, "/");
  STRCAT(tmpName, DNPCNFG_MAX_FILENAME, pFileDesc->findData.name);

  _buildInfoResponse(&pFileDesc->findData, tmpName, pType, pSize, pTimeOfCreation, pPermissions);

  fileNameLen = strlen(pFileDesc->findData.name);
  if(fileNameLen >= maxNameSize)
  {
    /* Name won't fit in buffer */
    return(DNPDEFS_FILE_TFER_STAT_MISC);
  }
    
  STRNCPY(pName, maxNameSize, pFileDesc->findData.name, fileNameLen);

  /* NULL terminate name */
  pName[fileNameLen] = 0x00;

  if(_findnext(pFileDesc->findHandle, &pFileDesc->findData) == 0)
  {
    *pLast = TMWDEFS_FALSE;
  }
  else
  {
    *pLast = TMWDEFS_TRUE;
  }
  return(DNPDEFS_FILE_TFER_STAT_SUCCESS);
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

/* function: _directorySize */
static size_t TMWDEFS_LOCAL _directorySize(
  char *pDirName)
{
  struct _finddata_t dirData;
  intptr_t dirHandle;
  size_t length = 0;

  if((dirHandle = _findfirst(pDirName, &dirData)) <= 0)
  {
    return(0);
  }
  /* For a directory, size is the number of octets that would be transferred by 
   * the read requests. This is (the number of files contained in the directory * 20)
   * + (sum of the lengths of the names of all of the files in the directory).
   */

  /* exclude links to itself and parent "."  ".." since they are not sent */
  if (_findnext(dirHandle, &dirData) == 0)
  {
    while (_findnext(dirHandle, &dirData) == 0)
    {
      length += 20;
      length += strlen(dirData.name);
    }
  }
  return(length);
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
      char filename[DNPCNFG_MAX_FILENAME];
      STRCPY(filename, DNPCNFG_MAX_FILENAME, pFilename);
      STRCAT(filename, DNPCNFG_MAX_FILENAME, "/*.*");

      pFileDesc = _allocFileDesc();
      if (pFileDesc)
      {

        if ((pFileDesc->findHandle = _findfirst(filename, &pFileDesc->findData)) <= 0)
        {
          *pHandle = 0;
          return(DNPDEFS_FILE_CMD_STAT_NOT_FOUND);
        }

        /* exclude links to itself and parent "."  ".." according to Application Layer Draft H 6.74 */
        if (_findnext(pFileDesc->findHandle, &pFileDesc->findData) == 0)
        {
          if (_findnext(pFileDesc->findHandle, &pFileDesc->findData) <= 0)
          {
            *pSize = (TMWTYPES_ULONG)_directorySize(filename);

            pFileDesc->blockSize = *pMaxBlockSize;
            *pHandle = pFileDesc->dnpHandle;
            return(DNPDEFS_FILE_CMD_STAT_SUCCESS);
          }
        }
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
    
  pFileDesc = _allocFileDesc();
  if (pFileDesc)
  {
    /* Open file */
    if(fopen_s(&pFileDesc->pCurrentFile, pFilename, pFileMode) == 0)
    {
      /* Initialize file state */
      pFileDesc->blockSize = *pMaxBlockSize;
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
    _freeFileDesc(pFileDesc);
    return(DNPDEFS_FILE_CMD_STAT_NOT_FOUND);
  }

  /* Clear file handle */
  *pHandle = 0;

  /* Return error */
  return (DNPDEFS_FILE_CMD_STAT_TOO_MANY);
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

  if (pFileDesc->findHandle > 0)
  {
    /* Close find */
    _findclose(pFileDesc->findHandle);
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

  handle = _fileno(pFileDesc->pCurrentFile);
    
  /* Read data */
  *pBytesRead = (TMWTYPES_USHORT)_read(handle, pBuf, pFileDesc->blockSize);  

  /* Check for errors */
  if(ferror(pFileDesc->pCurrentFile))
    return(DNPDEFS_FILE_TFER_STAT_BAD_FILE);

  /* Check for end of file */
  *pLast = (TMWTYPES_BOOL)(_eof(handle) ? TMWDEFS_TRUE : TMWDEFS_FALSE);

  /* Return success */
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

