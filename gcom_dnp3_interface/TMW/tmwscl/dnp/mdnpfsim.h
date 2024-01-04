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

/* file: mdnpfsim.h
 * description: Simulates a DNP master file transfer.
 *  This file is an example of a simulated DNP master database interface.
 *  It should NOT be included in the final version of a DNP master device.
 */
#ifndef MDNPFSIM_DEFINED
#define MDNPFSIM_DEFINED

#include "tmwscl/utils/tmwcnfg.h"
#if TMWCNFG_USE_SIMULATED_DB

#include "tmwscl/utils/tmwdefs.h"
#include "tmwscl/dnp/mdnpsesn.h"
#include "tmwscl/dnp/mdnpcnfg.h"

#if defined(_MSC_VER) && !defined(_WIN32_WCE)
#include <io.h>
#include <time.h>
#include <string.h>
#include <stdlib.h> 
#endif

#include <stdio.h>

typedef struct _MDnpFileSimXferContext {
  TMWTYPES_ULONG           authKey;
  TMWTYPES_ULONG           handle; 
  TMWTYPES_ULONG           fileSize;
  TMWTYPES_USHORT          maxBlockSize; 
  TMWTYPES_ULONG           blockNumber;
  TMWTYPES_BOOL            lastBlockFlag;
  DNPDEFS_FILE_TYPE        fileType;
  TMWDTIME                 fileCreationTime;
  DNPDEFS_FILE_PERMISSIONS permissions;
  DNPDEFS_FILE_CMD_STAT    cmdStatus; 
  DNPDEFS_FILE_TFER_STAT   tferStatus;

  FILE *pCurrentLocalFileHandle; /* the users local file handle */

} MDNPFSIM_XFER_CONTEXT;


#ifdef __cplusplus
extern "C" {
#endif

/* function: mdnpfsim_storeAuthKey */
TMWTYPES_BOOL mdnpfsim_storeFileAuthKey(
  void *pHandle,
  TMWTYPES_ULONG authKey);

/* function: mdnpfsim_storeFileStatus  */
TMWTYPES_BOOL mdnpfsim_storeFileStatus(
  void *pHandle,
  TMWTYPES_ULONG handle,
  TMWTYPES_ULONG fileSize,
  TMWTYPES_USHORT maxBlockSize,
  TMWTYPES_USHORT requestId,
  DNPDEFS_FILE_CMD_STAT status,
  TMWTYPES_USHORT nOptionalChars,
  const TMWTYPES_CHAR *pOptionalChars);

/* function: mdnpfsim_storeFileData */
TMWTYPES_BOOL mdnpfsim_storeFileData(
  void *pHandle,
  TMWTYPES_ULONG handle,
  TMWTYPES_ULONG blockNumber,
  TMWTYPES_BOOL lastBlockFlag,
  TMWTYPES_USHORT nBytesInBlockData,
  const TMWTYPES_UCHAR *pBlockData);

/* function: mdnpfsim_storeFileDataStatus */
TMWTYPES_BOOL mdnpfsim_storeFileDataStatus(
  void *pHandle,
  TMWTYPES_ULONG handle,
  TMWTYPES_ULONG blockNumber,
  TMWTYPES_BOOL lastBlockFlag,
  DNPDEFS_FILE_TFER_STAT status,
  TMWTYPES_USHORT nOptionalChars,
  const TMWTYPES_CHAR *pOptionalChars);

/* function: mdnpfsim_storeFileInfo */
TMWTYPES_BOOL mdnpfsim_storeFileInfo(
  void *pHandle,
  DNPDEFS_FILE_TYPE fileType,
  TMWTYPES_ULONG  fileSize,
  TMWDTIME *pFileCreationTime,
  DNPDEFS_FILE_PERMISSIONS permissions,
  const TMWTYPES_CHAR *pFileName);

/* function: mdnpfsim_getFileAuthKey */
TMWTYPES_ULONG mdnpfsim_getFileAuthKey(
  void *pHandle);

/* function: mdnpfsim_getFileHandle */
TMWTYPES_ULONG mdnpfsim_getFileHandle(
  void *pHandle);

/* function: mdnpfsim_getFileSize */
TMWTYPES_ULONG mdnpfsim_getFileSize(
  void *pHandle);

/* function: mdnpfsim_getFileMaxBlockSize */
TMWTYPES_USHORT mdnpfsim_getFileMaxBlockSize(
  void *pHandle);

/* function: mdnpfsim_getFileBlockNumber */
TMWTYPES_ULONG mdnpfsim_getFileBlockNumber(
  void *pHandle);
 
/* function: mdnpfsim_getFileLastBlockFlag */
TMWTYPES_BOOL mdnpfsim_getFileLastBlockFlag(
  void *pHandle);

/* function: mdnpfsim_getFileType */
DNPDEFS_FILE_TYPE mdnpfsim_getFileType(
  void *pHandle);
 
/* function: mdnpfsim_getFileCreationTime */
TMWTYPES_BOOL mdnpfsim_getFileCreationTime(
  void *pHandle,
  TMWDTIME *pTime);

/* function: mdnpfsim_getFilePermissions */
DNPDEFS_FILE_PERMISSIONS mdnpfsim_getFilePermissions(
  void *pHandle);
 
/* function: mdnpfsim_getFileCmdStat */
DNPDEFS_FILE_CMD_STAT mdnpfsim_getFileCmdStat(
  void *pHandle);
 
/* function: mdnpfsim_getFileTferStat */
DNPDEFS_FILE_TFER_STAT mdnpfsim_getFileTferStat(
  void *pHandle);

/* function: mdnpfsim_openLocalFile */
TMWTYPES_BOOL mdnpfsim_openLocalFile(
  void *pHandle,
  const TMWTYPES_CHAR *pLocalFileName, 
  DNPDEFS_FILE_MODE fileMode);

/* function: mdnpfsim_closeLocalFile */
TMWTYPES_BOOL mdnpfsim_closeLocalFile(
  void *pHandle);

TMWTYPES_BOOL mdnpfsim_getLocalFileInfo(
  void *pHandle,
  const TMWTYPES_CHAR *pLocalFileName, 
  TMWTYPES_ULONG *pFileSize, 
  TMWDTIME *pTimeOfCreation);

/* function: mdnpfsim_readLocalFile */
TMWTYPES_USHORT mdnpfsim_readLocalFile(
    void *pHandle,
    TMWTYPES_UCHAR *pBuf,
    TMWTYPES_USHORT  bufSize,
    TMWTYPES_BOOL *pLastBlock);

#ifdef __cplusplus
}
#endif
#endif /* TMWCNFG_USE_SIMULATED_DB */
#endif /* MDNPFSIM_DEFINED */
