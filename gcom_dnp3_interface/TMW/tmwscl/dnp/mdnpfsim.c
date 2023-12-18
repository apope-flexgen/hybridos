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

/* file: mdnpfsim.c
 * description: Simulates a DNP master file xfer.
 * Note that this simulation only supports transfer of a single file at a time.
 */
#include "tmwscl/dnp/dnpdiag.h"
#include "tmwscl/utils/tmwcnfg.h"

#if TMWCNFG_USE_SIMULATED_DB
#include "tmwscl/utils/tmwtarg.h"
#include "tmwscl/utils/tmwdlist.h"
#include "tmwscl/utils/tmwmsim.h"
#include "tmwscl/utils/tmwsim.h"

#include "tmwscl/utils/tmwsesn.h"

#include "tmwscl/dnp/mdnpsim.h"
#include "tmwscl/dnp/mdnpfsim.h"
#include "tmwscl/dnp/mdnpmem.h"

#include "tmwscl/dnp/mdnpbrm.h"
#include "tmwscl/dnp/mdnptarg.h"
 

/* function: mdnpfsim_storeAuthKey */
TMWTYPES_BOOL mdnpfsim_storeFileAuthKey(
  void *pHandle,
  TMWTYPES_ULONG authKey)
{
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;
  TMWTARG_UNUSED_PARAM(pHandle);  

  pDbHandle->fileSimXferContext.authKey = authKey;
  return (TMWDEFS_TRUE);
}

/* function: mdnpfsim_storeFileStatus  */
TMWTYPES_BOOL mdnpfsim_storeFileStatus(
  void *pHandle,
  TMWTYPES_ULONG handle,
  TMWTYPES_ULONG fileSize,
  TMWTYPES_USHORT maxBlockSize,
  TMWTYPES_USHORT requestId,
  DNPDEFS_FILE_CMD_STAT status,
  TMWTYPES_USHORT nOptionalChars,
  const TMWTYPES_CHAR *pOptionalChars)
{
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;

  TMWTARG_UNUSED_PARAM( pHandle);   
  TMWTARG_UNUSED_PARAM( requestId); 
  TMWTARG_UNUSED_PARAM( nOptionalChars);
  TMWTARG_UNUSED_PARAM( pOptionalChars);

  pDbHandle->fileSimXferContext.handle = handle; 
  pDbHandle->fileSimXferContext.fileSize = fileSize; 
  pDbHandle->fileSimXferContext.maxBlockSize = maxBlockSize; 
  pDbHandle->fileSimXferContext.cmdStatus = status;  
  
  return (TMWDEFS_TRUE);
}

/* function: mdnpfsim_storeFileData */
TMWTYPES_BOOL mdnpfsim_storeFileData(
  void *pHandle,
  TMWTYPES_ULONG handle,
  TMWTYPES_ULONG blockNumber,
  TMWTYPES_BOOL lastBlockFlag,
  TMWTYPES_USHORT nBytesInBlockData,
  const TMWTYPES_UCHAR *pBlockData)
{
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle; 
 
  pDbHandle->fileSimXferContext.handle = handle; 
  pDbHandle->fileSimXferContext.blockNumber = blockNumber; 
  pDbHandle->fileSimXferContext.lastBlockFlag = lastBlockFlag;  

#if TMWTARG_SUPPORT_DNPFILEIO
  return (mdnptarg_storeFileData(pDbHandle->fileSimXferContext.pCurrentLocalFileHandle, nBytesInBlockData, pBlockData));
#else
  TMWTARG_UNUSED_PARAM(nBytesInBlockData);
  TMWTARG_UNUSED_PARAM(pBlockData);
  return TMWDEFS_FALSE;
#endif
}

/* function: mdnpfsim_storeFileDataStatus */
TMWTYPES_BOOL mdnpfsim_storeFileDataStatus(
  void *pHandle,
  TMWTYPES_ULONG handle,
  TMWTYPES_ULONG blockNumber,
  TMWTYPES_BOOL lastBlockFlag,
  DNPDEFS_FILE_TFER_STAT status,
  TMWTYPES_USHORT nOptionalChars,
  const TMWTYPES_CHAR *pOptionalChars)
{
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;

  TMWTARG_UNUSED_PARAM( pHandle);  
  TMWTARG_UNUSED_PARAM( nOptionalChars);
  TMWTARG_UNUSED_PARAM( pOptionalChars);
 
  pDbHandle->fileSimXferContext.handle = handle; 
  pDbHandle->fileSimXferContext.blockNumber = blockNumber; 
  pDbHandle->fileSimXferContext.lastBlockFlag = lastBlockFlag;   
  pDbHandle->fileSimXferContext.tferStatus = status;  

  return (TMWDEFS_TRUE);
}

/* function: mdnpfsim_storeFileInfo */
TMWTYPES_BOOL mdnpfsim_storeFileInfo(
  void *pHandle,
  DNPDEFS_FILE_TYPE fileType,
  TMWTYPES_ULONG  fileSize,
  TMWDTIME *pFileCreationTime,
  DNPDEFS_FILE_PERMISSIONS permissions,
  const TMWTYPES_CHAR *pFileName)
{
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle; 

  TMWTARG_UNUSED_PARAM( pHandle); 
  TMWTARG_UNUSED_PARAM( pFileName);

  pDbHandle->fileSimXferContext.fileType = fileType; 
  pDbHandle->fileSimXferContext.fileSize = fileSize; 
  pDbHandle->fileSimXferContext.fileCreationTime = *pFileCreationTime; 
  pDbHandle->fileSimXferContext.permissions = permissions;   

  return (TMWDEFS_TRUE);
}

/* function: mdnpfsim_getFileAuthKey */
TMWTYPES_ULONG mdnpfsim_getFileAuthKey(
  void *pHandle)
{
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;
  return (pDbHandle->fileSimXferContext.authKey);
}
  
/* function: mdnpfsim_getFileHandle */
TMWTYPES_ULONG mdnpfsim_getFileHandle(
  void *pHandle)
{
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;
  return (pDbHandle->fileSimXferContext.handle);
}

/* function: mdnpfsim_getFileSize */
TMWTYPES_ULONG mdnpfsim_getFileSize(
  void *pHandle)
{
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;
  return (pDbHandle->fileSimXferContext.fileSize);
}

/* function: mdnpfsim_getFileMaxBlockSize */
TMWTYPES_USHORT mdnpfsim_getFileMaxBlockSize(
  void *pHandle)
{
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;
  return (pDbHandle->fileSimXferContext.maxBlockSize);
}

/* function: mdnpfsim_getFileBlockNumber */
TMWTYPES_ULONG mdnpfsim_getFileBlockNumber(
  void *pHandle)
{
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;
  return (pDbHandle->fileSimXferContext.blockNumber);
}

/* function: mdnpfsim_getFileLastBlockFlag */
TMWTYPES_BOOL mdnpfsim_getFileLastBlockFlag(
  void *pHandle)
{
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;
  return (pDbHandle->fileSimXferContext.lastBlockFlag);
}

/* function: mdnpfsim_getFileType */
DNPDEFS_FILE_TYPE mdnpfsim_getFileType(
  void *pHandle)
{
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;
  return (pDbHandle->fileSimXferContext.fileType);
}
 
/* function: mdnpfsim_getFileCreationTime */
TMWTYPES_BOOL mdnpfsim_getFileCreationTime(
  void *pHandle,
  TMWDTIME *pTime)
{
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;
  *pTime = pDbHandle->fileSimXferContext.fileCreationTime;
  return TMWDEFS_TRUE;
}

/* function: mdnpfsim_getFilePermissions */
DNPDEFS_FILE_PERMISSIONS mdnpfsim_getFilePermissions(
  void *pHandle)
{
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;
  return (pDbHandle->fileSimXferContext.permissions);
}
 
/* function: mdnpfsim_getFileCmdStat */
DNPDEFS_FILE_CMD_STAT mdnpfsim_getFileCmdStat(
  void *pHandle)
{
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;
  return (pDbHandle->fileSimXferContext.cmdStatus);
}
 
/* function: mdnpfsim_getFileTferStat */
DNPDEFS_FILE_TFER_STAT mdnpfsim_getFileTferStat(
  void *pHandle)
{
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;
  return (pDbHandle->fileSimXferContext.tferStatus);
}

/* function: mdnpfsim_openLocalFile */
TMWTYPES_BOOL mdnpfsim_openLocalFile(
  void *pHandle,
  const TMWTYPES_CHAR *pLocalFileName, 
  DNPDEFS_FILE_MODE fileMode)
{
#if TMWTARG_SUPPORT_DNPFILEIO
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;
 
  /* If local file is still open, close it before opening another */
  if(pDbHandle->fileSimXferContext.pCurrentLocalFileHandle != TMWDEFS_NULL)
  { 
    mdnptarg_closeLocalFile(pDbHandle->fileSimXferContext.pCurrentLocalFileHandle);
    pDbHandle->fileSimXferContext.pCurrentLocalFileHandle = TMWDEFS_NULL;
    pDbHandle->fileSimXferContext.authKey = 0UL;
  }
  
  pDbHandle->fileSimXferContext.pCurrentLocalFileHandle = mdnptarg_openLocalFile(pLocalFileName, fileMode);
  if(pDbHandle->fileSimXferContext.pCurrentLocalFileHandle != TMWDEFS_NULL)
  {
    return(TMWDEFS_TRUE);
  }
#else
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(pLocalFileName);
  TMWTARG_UNUSED_PARAM(fileMode);
#endif

  return(TMWDEFS_FALSE);
}

/* function: mdnpfsim_closeLocalFile */
TMWTYPES_BOOL mdnpfsim_closeLocalFile(
  void *pHandle)
{
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;
  if(pDbHandle->fileSimXferContext.pCurrentLocalFileHandle == TMWDEFS_NULL)
  {
    return(TMWDEFS_FALSE);
  }

#if TMWTARG_SUPPORT_DNPFILEIO
  mdnptarg_closeLocalFile(pDbHandle->fileSimXferContext.pCurrentLocalFileHandle);
#endif
  pDbHandle->fileSimXferContext.pCurrentLocalFileHandle = TMWDEFS_NULL;
  pDbHandle->fileSimXferContext.authKey = 0UL;

  return(TMWDEFS_TRUE);
}

TMWTYPES_BOOL mdnpfsim_getLocalFileInfo(
  void *pHandle,
  const TMWTYPES_CHAR *pLocalFileName, 
  TMWTYPES_ULONG *pFileSize, 
  TMWDTIME *pTimeOfCreation)
{
#if TMWTARG_SUPPORT_DNPFILEIO
  TMWTARG_UNUSED_PARAM(pHandle);
  return (mdnptarg_getLocalFileInfo(pLocalFileName, pFileSize, pTimeOfCreation));
#else 
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(pLocalFileName);
  TMWTARG_UNUSED_PARAM(pFileSize);
  TMWTARG_UNUSED_PARAM(pTimeOfCreation);
  return(TMWDEFS_FALSE);
#endif
}

/* function: mdnpfsim_readLocalFile */
TMWTYPES_USHORT mdnpfsim_readLocalFile(
    void *pHandle,
    TMWTYPES_UCHAR *pBuf,
    TMWTYPES_USHORT  bufSize,
    TMWTYPES_BOOL *pLastBlock)
{
#if TMWTARG_SUPPORT_DNPFILEIO
  MDNPSIM_DATABASE *pDbHandle = (MDNPSIM_DATABASE *)pHandle;
  return (mdnptarg_readLocalFile(pDbHandle->fileSimXferContext.pCurrentLocalFileHandle, pBuf, bufSize, pLastBlock));
#else
  TMWTARG_UNUSED_PARAM(pHandle);
  TMWTARG_UNUSED_PARAM(pBuf);
  TMWTARG_UNUSED_PARAM(bufSize);
  TMWTARG_UNUSED_PARAM(pLastBlock);
  return(0);
#endif
}

#endif /* TMWCNFG_SUPPORT_SIMULATED_DB */
