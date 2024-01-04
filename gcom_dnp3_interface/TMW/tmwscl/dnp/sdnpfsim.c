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

/* file: sdnpsim.c
 * description: Example file I/O implementation.
 *
 * Simple file transfer target implementation for Windows. This
 * implementation only supports accessing one file at a time.
 * Trying to open a second file will result in a too many files
 * error.
 */

#include "tmwscl/utils/tmwtarg.h"
#include "tmwscl/utils/tmwcnfg.h"
#include "tmwscl/dnp/dnpdiag.h"

/* to get rid of a deprecation warning in VS 2005 .NET */
#if _WIN32
#if defined(_MSC_VER)
#define _STRNICMP _strnicmp
#else
#define _STRNICMP strnicmp
#endif
#else
#define _STRNICMP strncasecmp
#endif

#if TMWCNFG_USE_SIMULATED_DB

#include "tmwscl/utils/tmwtarg.h"
#include "tmwscl/dnp/dnpcnfg.h"
#include "tmwscl/dnp/sdnpcnfg.h"
#include "tmwscl/dnp/sdnpdata.h"
#include "tmwscl/dnp/sdnpsim.h"
#if  SDNPDATA_SUPPORT_OBJ70 || SDNPDATA_SUPPORT_XML2
#include "tmwscl/dnp/sdnpxml2.h"
#include "tmwscl/dnp/sdnpfsim.h"
#include "tmwscl/dnp/sdnpxml.h"
#include "tmwscl/dnp/sdnptarg.h"

#define DEVICE_PROFILE_XML2_FILE "dnpDPCfg.xml"
#define DEVICE_DESCRIPTION_FILE  "DNPDeviceConfiguration.xml"
#define VIRTUAL_FILE_DRIVE       "V:"

#if SDNPDATA_SUPPORT_READ_XML
/* function: _buildInfoResponse */
static TMWTYPES_BOOL _buildInfoResponse(
  TMWSESN                  *pSession,
  const char               *pFilename,
  DNPDEFS_FILE_TYPE        *pType,
  TMWTYPES_ULONG           *pSize,
  TMWDTIME                 *pTimeOfCreation,
  DNPDEFS_FILE_PERMISSIONS *pPermissions)
{
  tmwdtime_getDateTime(pSession, pTimeOfCreation);
  *pType = DNPDEFS_FILE_TYPE_SIMPLE;

#if !SDNPDATA_SUPPORT_XML2 && !SDNPDATA_SUPPORT_XML
  TMWTARG_UNUSED_PARAM(pFilename);
  TMWTARG_UNUSED_PARAM(pSize);
#endif
#if SDNPDATA_SUPPORT_XML2 
  if(strstr(pFilename, DEVICE_PROFILE_XML2_FILE) != 0)
  {
    SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
    pSDNPSession->xml2IndentSize = 0;
    *pSize = sdnpxml2_saveDatabaseGetSize(pSession);
    if(*pSize == 0)
    {
      /* Return error */
      return TMWDEFS_FALSE;
    }
  }
#endif
#if SDNPDATA_SUPPORT_XML
  if(strstr(pFilename, DEVICE_DESCRIPTION_FILE) != 0)
  {
    *pSize = sdnpxml_saveDatabaseStaticGetSize(pSession);
    if(*pSize == 0)
    {
      /* Return error */
      return TMWDEFS_FALSE;
    }
  }
#endif
  *pPermissions = 0x1ff;
  return TMWDEFS_TRUE;
}
#endif

/* function: sdnpfsim_initDb */
void TMWDEFS_GLOBAL sdnpfsim_initDb(
  SDNPFSIM_DATABASE *pFDBHandle)
{ 
  pFDBHandle->findHandle = 0;
  pFDBHandle->blockSize = 0;
  pFDBHandle->pCurrentFile = TMWDEFS_NULL;
  pFDBHandle->bInternalDriveInfo = TMWDEFS_FALSE;

  pFDBHandle->fileAuthenticationKey = 0;
  pFDBHandle->filePassword[0] = '\0';
  pFDBHandle->fileUserName[0] = '\0';
  pFDBHandle->fileEventClass = TMWDEFS_CLASS_MASK_THREE;

#if SDNPDATA_SUPPORT_XML
  memset(pFDBHandle->xmlStaticDescriptionBuf,0,SDNPCNFG_XML_SAVE_BUF_SIZE);
  pFDBHandle->xmlStaticDescriptionBufLen = 0;
  pFDBHandle->xmlStaticDescriptionBufIndex = 0;
  pFDBHandle->xmlStaticDescriptionBufOpen = TMWDEFS_FALSE;
  pFDBHandle->xmlStaticDescriptionBufState = SDNPXML_SAVE_NOT_DONE;
#endif

  pFDBHandle->fileTestingMode = TMWSIM_TESTINGMODE_SUCCESS;
  tmwtimer_init(&pFDBHandle->fileTestingTimer); 
}

/* function: sdnpfsim_clearDb */
void TMWDEFS_GLOBAL sdnpfsim_clearDb(
  SDNPFSIM_DATABASE *pFDBHandle)
{
  tmwtimer_cancel(&pFDBHandle->fileTestingTimer); 
}

/* function: sdnpfsim_getFileInfo */
DNPDEFS_FILE_CMD_STAT TMWDEFS_GLOBAL sdnpfsim_getFileInfo(
  SDNPFSIM_DATABASE *pFDBHandle,
  TMWTYPES_CHAR *pFilename,
  DNPDEFS_FILE_TYPE *pType,
  TMWTYPES_ULONG *pSize,
  TMWDTIME *pTimeOfCreation,
  DNPDEFS_FILE_PERMISSIONS *pPermissions)
{
  if(pFDBHandle->fileTestingMode == TMWSIM_TESTINGMODE_FAILURE)
  {
    return(DNPDEFS_FILE_CMD_STAT_LOST_COMM);
  }
  else if(pFDBHandle->fileTestingMode == TMWSIM_TESTINGMODE_DELAY)
  {
    if(pFDBHandle->fileDelayCount++ < 3)
      return(DNPDEFS_FILE_CMD_STAT_ASYNC);

    if(pFDBHandle->fileDelayCount++ >=10)
      pFDBHandle->fileDelayCount = 0;
  }

#if TMWTARG_SUPPORT_DNPFILEIO
  if (strcmp(pFilename,"") == 0)
  { /* return disk drive names */
    return(DNPDEFS_FILE_CMD_STAT_NOT_FOUND);
  }

  /* Get file info */
  return (sdnptarg_getFileInfo(pFilename, pType, pSize, pTimeOfCreation, pPermissions));

#else
  TMWTARG_UNUSED_PARAM(pFDBHandle);
  TMWTARG_UNUSED_PARAM(pFilename);
  TMWTARG_UNUSED_PARAM(pType);
  TMWTARG_UNUSED_PARAM(pSize);
  TMWTARG_UNUSED_PARAM(pTimeOfCreation);
  TMWTARG_UNUSED_PARAM(*pPermissions);
  return(DNPDEFS_FILE_CMD_STAT_NOT_FOUND);
#endif
}
    
/* function: sdnpfsim_fileEventClass */
TMWDEFS_CLASS_MASK TMWDEFS_GLOBAL sdnpfsim_fileEventClass(
  SDNPFSIM_DATABASE *pFDBHandle)
{
  return(pFDBHandle->fileEventClass);
}

/* function: sdnpfsim_setFileEventClass */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpfsim_setFileEventClass(
  SDNPFSIM_DATABASE *pFDBHandle, 
  TMWDEFS_CLASS_MASK eventClass)
{
  pFDBHandle->fileEventClass = eventClass;
  return(TMWDEFS_TRUE);
}

/* function: sdnpfsim_setAuthentication */
TMWTYPES_BOOL sdnpfsim_setAuthentication(
  SDNPFSIM_DATABASE *pFDBHandle,
  TMWTYPES_ULONG authenticationKey, 
  TMWTYPES_CHAR *pUserName, 
  TMWTYPES_CHAR *pPassword)
{
  if(pUserName == TMWDEFS_NULL)
  {
    pFDBHandle->fileUserName[0] = '\0';
  } 
  else if(strlen(pUserName) < SDNPSIM_FILE_AUTH_SIZE)
  {    
    STRCPY(pFDBHandle->fileUserName, SDNPSIM_FILE_AUTH_SIZE, pUserName);
  }
  else 
  {
    return(TMWDEFS_FALSE);
  }

  if(pPassword == TMWDEFS_NULL)
  {
    pFDBHandle->filePassword[0] = '\0';
  }
  else if(strlen(pPassword) < SDNPSIM_FILE_AUTH_SIZE)
  {
    STRCPY(pFDBHandle->filePassword, SDNPSIM_FILE_AUTH_SIZE, pPassword);
  }
  else 
  {
    return(TMWDEFS_FALSE);
  }

  pFDBHandle->fileAuthenticationKey = authenticationKey; 

  return(TMWDEFS_TRUE);
}

/* function: sdnpfsim_getAuthenticationData */
TMWTYPES_BOOL sdnpfsim_getAuthenticationData(
  SDNPFSIM_DATABASE *pFDBHandle,
  TMWTYPES_ULONG *pAuthenticationKey, 
  TMWTYPES_CHAR *pUserName, 
  TMWTYPES_USHORT maxUserNameSize,
  TMWTYPES_CHAR *pPassword,
  TMWTYPES_USHORT maxPasswordSize)
{
  STRCPY(pUserName, maxUserNameSize, pFDBHandle->fileUserName);
  STRCPY(pPassword, maxPasswordSize, pFDBHandle->filePassword); 
  *pAuthenticationKey = pFDBHandle->fileAuthenticationKey;
  return(TMWDEFS_TRUE);
}

/* function: sdnpfsim_readFileInfo */
DNPDEFS_FILE_TFER_STAT TMWDEFS_GLOBAL sdnpfsim_readFileInfo(
  TMWSESN *pSession,
  SDNPFSIM_DATABASE *pFDBHandle,
  TMWTYPES_ULONG dnpFileHandle,
  TMWTYPES_USHORT maxNameSize,
  TMWTYPES_CHAR *pName,
  TMWTYPES_BOOL *pLast,
  DNPDEFS_FILE_TYPE *pType,
  TMWTYPES_ULONG *pSize,
  TMWDTIME *pTimeOfCreation,
  DNPDEFS_FILE_PERMISSIONS *pPermissions)
{
  if(pFDBHandle->fileTestingMode == TMWSIM_TESTINGMODE_FAILURE)
  {
    return(DNPDEFS_FILE_CMD_STAT_LOST_COMM);
  }
  else if(pFDBHandle->fileTestingMode == TMWSIM_TESTINGMODE_DELAY)
  {
    if(pFDBHandle->fileDelayCount++ < 3)
      return(DNPDEFS_FILE_CMD_STAT_ASYNC);

    if(pFDBHandle->fileDelayCount++ >=10)
      pFDBHandle->fileDelayCount = 0;
  }

#if SDNPDATA_SUPPORT_READ_XML
  if(pFDBHandle->bInternalDriveInfo == TMWDEFS_TRUE)
  { 
    /* If SCL supports new shema, return that one */
    if(strstr(pFDBHandle->findFilename, DEVICE_PROFILE_XML2_FILE)!=0)
    {
      _buildInfoResponse(pSession, DEVICE_PROFILE_XML2_FILE, pType, pSize, pTimeOfCreation, pPermissions);
      STRCPY(pName, maxNameSize, DEVICE_PROFILE_XML2_FILE);
      STRCPY(pFDBHandle->findFilename, DNPCNFG_MAX_FILENAME, DEVICE_DESCRIPTION_FILE); 
#if SDNPDATA_SUPPORT_XML
      *pLast = TMWDEFS_FALSE;
#else
      *pLast = TMWDEFS_TRUE;
#endif
    } 
    else
    {
      _buildInfoResponse(pSession, DEVICE_DESCRIPTION_FILE, pType, pSize, pTimeOfCreation, pPermissions);
      STRCPY(pName, maxNameSize, DEVICE_DESCRIPTION_FILE); 
      *pLast = TMWDEFS_TRUE;
    }
    return(DNPDEFS_FILE_TFER_STAT_SUCCESS);
  }
#else
  TMWTARG_UNUSED_PARAM(pSession);
#endif

#if TMWTARG_SUPPORT_DNPFILEIO
  return (sdnptarg_readFileInfo(dnpFileHandle, maxNameSize, pName, pLast, pType, pSize, pTimeOfCreation, pPermissions));

#else
  TMWTARG_UNUSED_PARAM(pSession);
  TMWTARG_UNUSED_PARAM(pFDBHandle);
  TMWTARG_UNUSED_PARAM(dnpFileHandle);
  TMWTARG_UNUSED_PARAM(maxNameSize);
  TMWTARG_UNUSED_PARAM(pName);
  TMWTARG_UNUSED_PARAM(pLast);
  TMWTARG_UNUSED_PARAM(pType);
  TMWTARG_UNUSED_PARAM(pSize);
  TMWTARG_UNUSED_PARAM(pTimeOfCreation);
  TMWTARG_UNUSED_PARAM(pPermissions);
  return(DNPDEFS_FILE_TFER_STAT_NOT_OPEN);
#endif
}


/* function: sdnpfsim_deleteFile */
DNPDEFS_FILE_CMD_STAT TMWDEFS_GLOBAL sdnpfsim_deleteFile(
  SDNPFSIM_DATABASE *pFDBHandle,
  TMWTYPES_CHAR *pFilename,
  TMWTYPES_ULONG authKey)
{
  if(pFDBHandle->fileTestingMode == TMWSIM_TESTINGMODE_FAILURE)
  {
    return(DNPDEFS_FILE_CMD_STAT_MISC);
  }
  else if(pFDBHandle->fileTestingMode == TMWSIM_TESTINGMODE_DELAY)
  {
    if(pFDBHandle->fileDelayCount++ < 3)
      return(DNPDEFS_FILE_CMD_STAT_ASYNC);

    if(pFDBHandle->fileDelayCount++ >=10)
      pFDBHandle->fileDelayCount = 0;
  }

#if TMWTARG_SUPPORT_DNPFILEIO
  if(authKey != pFDBHandle->fileAuthenticationKey)
  {
    return(DNPDEFS_FILE_CMD_STAT_DENIED);
  }

  /* Delete requested file */
  return (sdnptarg_deleteFile(pFilename));
#else
  TMWTARG_UNUSED_PARAM(pFDBHandle);
  TMWTARG_UNUSED_PARAM(pFilename);
  TMWTARG_UNUSED_PARAM(authKey);
  return(DNPDEFS_FILE_TFER_STAT_NOT_OPEN);
#endif
}

/* function: sdnpfsim_getAuthentication */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpfsim_getAuthentication(
  SDNPFSIM_DATABASE *pFDBHandle,
  TMWTYPES_CHAR *pUserName,
  TMWTYPES_CHAR *pPassword,
  TMWTYPES_ULONG *pAuthKey)
{
  if((strcmp(pUserName, pFDBHandle->fileUserName) == 0)
    &&(strcmp(pPassword, pFDBHandle->filePassword) == 0))
  {
    *pAuthKey = pFDBHandle->fileAuthenticationKey;
  }
  else
  {
    /* Return authentication value of zero. */ 
    *pAuthKey = 0;
  }

  return(TMWDEFS_TRUE);
}

/* function: sdnpfsim_deviceProfileFile */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpfsim_deviceProfileFile(
  SDNPFSIM_DATABASE *pFDBHandle,
  TMWTYPES_CHAR *pFileName,
  TMWTYPES_ULONG authKey,
  TMWTYPES_BOOL *pAuthKeyOK)
{
  if(strstr(pFileName, DEVICE_PROFILE_XML2_FILE) != 0)
  {
    if(authKey == pFDBHandle->fileAuthenticationKey)
      *pAuthKeyOK = TMWDEFS_TRUE;
    else
      *pAuthKeyOK = TMWDEFS_FALSE;
    return(TMWDEFS_TRUE);
  }
  else
  {
    return(TMWDEFS_FALSE);
  }
}

#include "tmwscl/dnp/sdnpo070.h"
static void TMWDEFS_CALLBACK _fileTestingTimeout(
  void *pCallbackParam)
{
#if  SDNPDATA_SUPPORT_OBJ70
  sdnpo070_fileCmdComplete((TMWSESN *)pCallbackParam, 0);
#endif
}

/* function: sdnpfsim_openFile */
DNPDEFS_FILE_CMD_STAT TMWDEFS_GLOBAL sdnpfsim_openFile(
  TMWSESN *pSession,
  SDNPFSIM_DATABASE *pFDBHandle,
  TMWTYPES_CHAR *pFilename,
  TMWTYPES_ULONG authKey,
  DNPDEFS_FILE_MODE mode,
  TMWTYPES_USHORT *pMaxBlockSize,
  DNPDEFS_FILE_PERMISSIONS *pPermissions,
  TMWDTIME *pTimeOfCreation,
  TMWTYPES_ULONG *pFileHandle,
  TMWTYPES_ULONG *pSize,
  DNPDEFS_FILE_TYPE *pType)
{
  /* If file already open return error */
  if ((pFDBHandle->pCurrentFile != TMWDEFS_NULL) || (pFDBHandle->findHandle > 0))
  {
    return(DNPDEFS_FILE_CMD_STAT_TOO_MANY);
  }

  if (authKey != pFDBHandle->fileAuthenticationKey)
  {
    return(DNPDEFS_FILE_CMD_STAT_DENIED);
  }

  if (pFDBHandle->fileTestingMode == TMWSIM_TESTINGMODE_FAILURE)
  {
    return(DNPDEFS_FILE_CMD_STAT_MISC);
  }

  /*.. removing this tests file read delay */
  else if (pFDBHandle->fileTestingMode == TMWSIM_TESTINGMODE_DELAY)
  {
    if (pFDBHandle->fileDelayCount++ < 1)
    {
      /* Start a timer, to have database call sdnpo070_fileCmdComplete */
      tmwtimer_start(&pFDBHandle->fileTestingTimer,
        200,
        pSession->pChannel,
        _fileTestingTimeout,
        pSession);

      return(DNPDEFS_FILE_CMD_STAT_ASYNC);
    }
    if (pFDBHandle->fileDelayCount++ < 2)
    {
      /* Let SCL retry open request periodically */
      return(DNPDEFS_FILE_CMD_STAT_ASYNC);
    }
    if (pFDBHandle->fileDelayCount++ >= 10)
      pFDBHandle->fileDelayCount = 0;
  }

#if SDNPDATA_SUPPORT_READ_XML
  /* if this is a request for the old XML device configuration file
   */
  if (strstr(pFilename, DEVICE_DESCRIPTION_FILE) != 0)
  {
#if SDNPDATA_SUPPORT_XML
    /* can only read the device description */
    if (mode != DNPDEFS_FILE_MODE_READ)
    {
      return(DNPDEFS_FILE_CMD_STAT_INV_MODE);
    }
    pFDBHandle->blockSize = *pMaxBlockSize;
    *pSize = sdnpxml_saveDatabaseStaticGetSize(pSession);
    if (*pSize == 0)
    {
      /* Return error */
      return(DNPDEFS_FILE_CMD_STAT_NOT_FOUND);
    }
    sdnpxml_saveDatabaseStaticInit(pSession);
    pFDBHandle->xmlStaticDescriptionBufOpen = TMWDEFS_TRUE;

    *pFileHandle = SDNPFSIM_HANDLE;
    *pType = DNPDEFS_FILE_TYPE_SIMPLE;
    /* Return success */
    return(DNPDEFS_FILE_CMD_STAT_SUCCESS);
#else
    *pSize = 0;
    return(DNPDEFS_FILE_CMD_STAT_NOT_FOUND);
#endif
  }
  else if (_STRNICMP(VIRTUAL_FILE_DRIVE, pFilename, strlen(VIRTUAL_FILE_DRIVE)) == 0
    && ((strlen(pFilename) == strlen(VIRTUAL_FILE_DRIVE))
      || (pFilename[strlen(pFilename) - 1] == '/')
      || (pFilename[strlen(pFilename) - 1] == '\\')))
  {
    /* open directory of the virtual drive */
    if (mode != DNPDEFS_FILE_MODE_READ)
    {
      /* can only read the directory */
      return(DNPDEFS_FILE_CMD_STAT_INV_MODE);
    }
    else
    {
      pFDBHandle->bInternalDriveInfo = TMWDEFS_TRUE;
      pFDBHandle->bInternalFileCount = 0;
#if SDNPDATA_SUPPORT_XML2
      /* If standard schema is supported, show it first */
      STRCPY(pFDBHandle->findFilename, DNPCNFG_MAX_FILENAME, DEVICE_PROFILE_XML2_FILE);
#else
      /* If just this schema is supported, show it */
      STRCPY(pFDBHandle->findFilename, DNPCNFG_MAX_FILENAME, DEVICE_DESCRIPTION_FILE);
#endif
      *pType = DNPDEFS_FILE_TYPE_DIRECTORY;
      *pFileHandle = SDNPFSIM_HANDLE;
      tmwdtime_getDateTime(pSession, pTimeOfCreation);
      *pPermissions = 0x1ff;

      *pSize = 0;

#if SDNPDATA_SUPPORT_XML
      /* xml file on the virtual drive, size is
       * 20 + length of file name
       */
      *pSize += 20 + (TMWTYPES_ULONG)strlen(DEVICE_DESCRIPTION_FILE);
#endif

#if SDNPDATA_SUPPORT_XML2 
      /* xml2 file on the virtual drive, size is
       * 20 + length of file name
       */
      *pSize += 20 + (TMWTYPES_ULONG)strlen(DEVICE_PROFILE_XML2_FILE);
#endif
    }
    /* Return success */
    return(DNPDEFS_FILE_CMD_STAT_SUCCESS);
  }
#endif
#if TMWTARG_SUPPORT_DNPFILEIO
  /* regular file processing */
  return(sdnptarg_openFile(pFilename, mode, pMaxBlockSize, pPermissions, pTimeOfCreation, pFileHandle, pSize, pType));

#else
  TMWTARG_UNUSED_PARAM(pSession);
  TMWTARG_UNUSED_PARAM(pFDBHandle);
  TMWTARG_UNUSED_PARAM(pFilename);
  TMWTARG_UNUSED_PARAM(authKey);
  TMWTARG_UNUSED_PARAM(mode);
  TMWTARG_UNUSED_PARAM(pMaxBlockSize);
  TMWTARG_UNUSED_PARAM(pPermissions);
  TMWTARG_UNUSED_PARAM(pTimeOfCreation);
  TMWTARG_UNUSED_PARAM(*pFileHandle);
  TMWTARG_UNUSED_PARAM(pSize);
  TMWTARG_UNUSED_PARAM(pType);
  return(DNPDEFS_FILE_CMD_STAT_NOT_FOUND);
#endif
}

/* function: sdnpfsim_closeFile */
DNPDEFS_FILE_CMD_STAT TMWDEFS_GLOBAL sdnpfsim_closeFile(
  TMWSESN *pSession,
  SDNPFSIM_DATABASE *pFDBHandle,
  TMWTYPES_ULONG fileHandle)
{
  /* check for fileTestingMode after close is done */
  if(pFDBHandle->fileTestingMode == TMWSIM_TESTINGMODE_DELAY)
  {
    if(pFDBHandle->fileDelayCount++ < 1)
    {
      /* Start a timer, to have database call sdnpo070_fileCmdComplete */
      tmwtimer_start(&pFDBHandle->fileTestingTimer, 
        200,
        pSession->pChannel,
        _fileTestingTimeout,
        pSession); 

      return(DNPDEFS_FILE_CMD_STAT_ASYNC);
    }
    if(pFDBHandle->fileDelayCount++ < 2)
    {
      /* Let SCL retry close request periodically */
      return(DNPDEFS_FILE_CMD_STAT_ASYNC);
    }
    if(pFDBHandle->fileDelayCount++ >=10)
      pFDBHandle->fileDelayCount = 0;
  }

#if SDNPDATA_SUPPORT_READ_XML
#if SDNPDATA_SUPPORT_XML
  if(pFDBHandle->xmlStaticDescriptionBufOpen == TMWDEFS_TRUE)
  {
    /* Validate file state */
    if(fileHandle != SDNPFSIM_HANDLE)
      return(DNPDEFS_FILE_CMD_STAT_INV_HANDLE);
    
    memset(pFDBHandle->xmlStaticDescriptionBuf,0,SDNPCNFG_XML_SAVE_BUF_SIZE);
    pFDBHandle->xmlStaticDescriptionBufLen = 0;
    pFDBHandle->xmlStaticDescriptionBufIndex = 0;
    pFDBHandle->xmlStaticDescriptionBufState = SDNPXML_SAVE_NOT_DONE;
    pFDBHandle->xmlStaticDescriptionBufOpen = TMWDEFS_FALSE;

    /* Return success */
    return(DNPDEFS_FILE_CMD_STAT_SUCCESS);
  }
#endif
  if (pFDBHandle->bInternalDriveInfo)
  { /* close for directory */
    pFDBHandle->bInternalDriveInfo = TMWDEFS_FALSE;
    return(DNPDEFS_FILE_CMD_STAT_SUCCESS);
  }
#endif

#if TMWTARG_SUPPORT_DNPFILEIO
  /* For test purposes only */
  if(pFDBHandle->fileTestingMode == TMWSIM_TESTINGMODE_FAILURE)
  {
    sdnptarg_closeFile(fileHandle);
    return(DNPDEFS_FILE_CMD_STAT_MISC);
  }
  return (sdnptarg_closeFile(fileHandle));
#else
  TMWTARG_UNUSED_PARAM(pSession);
  TMWTARG_UNUSED_PARAM(pFDBHandle);
  TMWTARG_UNUSED_PARAM(fileHandle);
  return(DNPDEFS_FILE_CMD_STAT_NOT_FOUND);
#endif
}
/* function: sdnpfsim_readFile */
DNPDEFS_FILE_TFER_STAT TMWDEFS_GLOBAL sdnpfsim_readFile(
  TMWSESN *pSession,
  SDNPFSIM_DATABASE *pFDBHandle,
  TMWTYPES_ULONG fileHandle,
  TMWTYPES_BOOL *pLast,
  TMWTYPES_USHORT *pBytesRead,
  TMWTYPES_UCHAR *pBuf)
{
  if(pFDBHandle->fileTestingMode == TMWSIM_TESTINGMODE_FAILURE)
  {
    return(DNPDEFS_FILE_TFER_STAT_MISC);
  }
  else if(pFDBHandle->fileTestingMode == TMWSIM_TESTINGMODE_DELAY)
  {
    if(pFDBHandle->fileDelayCount++ < 3)
      return(DNPDEFS_FILE_TFER_STAT_ASYNC);

    if(pFDBHandle->fileDelayCount++ >=10)
      pFDBHandle->fileDelayCount = 0;
  }
  
#if SDNPDATA_SUPPORT_READ_XML && SDNPDATA_SUPPORT_XML
  /* If this pointer is not NULL XML description "file" has been opened
   * and xml description has been generated.
   */ 
  if(pFDBHandle->xmlStaticDescriptionBufOpen == TMWDEFS_TRUE)
  {
    TMWTYPES_UINT memCpyLen;

    /* Validate file state against request */
    if(fileHandle != SDNPFSIM_HANDLE)
      return(DNPDEFS_FILE_TFER_STAT_INV_HANDLE);

    do 
    {
      if (pFDBHandle->xmlStaticDescriptionBufLen == 0)
      { 
        pFDBHandle->xmlStaticDescriptionBufState = sdnpxml_saveDatabaseStatic(pSession, pFDBHandle->xmlStaticDescriptionBuf, SDNPCNFG_XML_SAVE_BUF_SIZE, &pFDBHandle->xmlStaticDescriptionBufLen);  
        if(pFDBHandle->xmlStaticDescriptionBufState == SDNPXML_SAVE_FAILED)
          return(DNPDEFS_FILE_TFER_STAT_MISC);
      }

      if((pFDBHandle->xmlStaticDescriptionBufIndex + pFDBHandle->blockSize) <= pFDBHandle->xmlStaticDescriptionBufLen)
      {
        memCpyLen = pFDBHandle->blockSize;
        *pLast = TMWDEFS_FALSE;
      }
      else
      {
        memCpyLen = pFDBHandle->xmlStaticDescriptionBufLen - pFDBHandle->xmlStaticDescriptionBufIndex;
        if (pFDBHandle->xmlStaticDescriptionBufState == SDNPXML_SAVE_DONE)
        {
          *pLast = TMWDEFS_TRUE;
        }
        else
        {
          *pLast = TMWDEFS_FALSE;
        }
      }
      if (memCpyLen == 0)
      {
        pFDBHandle->xmlStaticDescriptionBufIndex = 0;
        pFDBHandle->xmlStaticDescriptionBufLen = 0;
        memset(pFDBHandle->xmlStaticDescriptionBuf,0,SDNPCNFG_XML_SAVE_BUF_SIZE);
      }
    }
    while(memCpyLen == 0);

    memcpy(pBuf, &pFDBHandle->xmlStaticDescriptionBuf[pFDBHandle->xmlStaticDescriptionBufIndex], memCpyLen);
    pFDBHandle->xmlStaticDescriptionBufIndex += memCpyLen;
    if (memCpyLen < pFDBHandle->blockSize)
    {
      pFDBHandle->xmlStaticDescriptionBufIndex = 0;
      pFDBHandle->xmlStaticDescriptionBufLen = 0;
      memset(pFDBHandle->xmlStaticDescriptionBuf,0,SDNPCNFG_XML_SAVE_BUF_SIZE);
    }
    *pBytesRead = (TMWTYPES_USHORT)memCpyLen;

    return(DNPDEFS_FILE_TFER_STAT_SUCCESS);
  }
#else
  TMWTARG_UNUSED_PARAM(pSession);
#endif

#if TMWTARG_SUPPORT_DNPFILEIO
  return (sdnptarg_readFile(fileHandle, pLast, pBytesRead, pBuf));
#else
  TMWTARG_UNUSED_PARAM(pSession);
  TMWTARG_UNUSED_PARAM(pFDBHandle);
  TMWTARG_UNUSED_PARAM(fileHandle);
  TMWTARG_UNUSED_PARAM(pLast);
  TMWTARG_UNUSED_PARAM(pBytesRead);
  TMWTARG_UNUSED_PARAM(pBuf);
  return(DNPDEFS_FILE_TFER_STAT_NOT_OPEN);
#endif
}

/* function: sdnpfsim_writeFile */
DNPDEFS_FILE_TFER_STAT TMWDEFS_GLOBAL sdnpfsim_writeFile(
  SDNPFSIM_DATABASE *pFDBHandle,
  TMWTYPES_ULONG fileHandle,
  TMWTYPES_BOOL last,
  TMWTYPES_USHORT numBytes,
  TMWTYPES_UCHAR *pBuf)
{
  if(pFDBHandle->fileTestingMode == TMWSIM_TESTINGMODE_FAILURE)
  {
    return(DNPDEFS_FILE_TFER_STAT_MISC);
  }
  else if(pFDBHandle->fileTestingMode == TMWSIM_TESTINGMODE_DELAY)
  {
    if(pFDBHandle->fileDelayCount++ < 3)
      return(DNPDEFS_FILE_TFER_STAT_ASYNC);

    if(pFDBHandle->fileDelayCount++ >=10)
      pFDBHandle->fileDelayCount = 0;
  }

#if TMWTARG_SUPPORT_DNPFILEIO
  /* Call target write to perform write operation */
  return(sdnptarg_writeFile(fileHandle, last, numBytes, pBuf));
#else
  TMWTARG_UNUSED_PARAM(pFDBHandle);
  TMWTARG_UNUSED_PARAM(fileHandle);
  TMWTARG_UNUSED_PARAM(last);
  TMWTARG_UNUSED_PARAM(numBytes);
  TMWTARG_UNUSED_PARAM(pBuf);
  return(DNPDEFS_FILE_TFER_STAT_NOT_OPEN);
#endif
}

/* function: sdnpfsim_setFileTestingMode */
void TMWDEFS_GLOBAL sdnpfsim_setFileTestingMode(
  SDNPFSIM_DATABASE *pFDBHandle,
  TMWSIM_TESTINGMODE testingMode)
{
  pFDBHandle->fileTestingMode = testingMode;
  pFDBHandle->fileDelayCount = 0;
}
#endif

#endif /* #if TMWCNFG_USE_SIMULATED_DB */
