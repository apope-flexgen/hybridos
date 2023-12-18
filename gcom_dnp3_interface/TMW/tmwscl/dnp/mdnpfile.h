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

/* file: mdnpfile.h
 * description: DNP master file transfer.
 */
#ifndef MDNPFILE_DEFINED
#define MDNPFILE_DEFINED

#include "tmwscl/dnp/dnpchnl.h"

typedef enum {
  MDNPFILE_XFER_STATE_FAILED=1,
  MDNPFILE_XFER_STATE_RETURN,
  MDNPFILE_XFER_STATE_READ,
  MDNPFILE_XFER_STATE_WRITE,
  MDNPFILE_XFER_STATE_CLOSE,
  MDNPFILE_XFER_STATE_DONE,
  MDNPFILE_XFER_STATE_OPEN_READ,
  MDNPFILE_XFER_STATE_OPEN_WRITE,
  MDNPFILE_XFER_STATE_DELETE,

  /* to allow user to abort in the middle of an SCL managed file transfer */
  MDNPFILE_XFER_STATE_ABORT
} MDNPFILE_XFER_STATE;


typedef struct _MDnpFileXferContext {

  /* Indicates simple file or directory */
  DNPDEFS_FILE_TYPE    fileType;

  TMWTYPES_BOOL        localFileOpen;
  TMWTYPES_BOOL        useAggressiveMode;

  TMWTYPES_ULONG       remoteFileHandle;
  TMWTYPES_CHAR        remoteFileName[DNPCNFG_MAX_FILENAME+1];

  TMWTYPES_ULONG       fileSize;
  TMWTYPES_MS_SINCE_70 timeOfCreation;
  TMWTYPES_USHORT      maxBlockSize;

  TMWTYPES_USHORT      permissions;

  MDNPFILE_XFER_STATE  nextOperation;
  TMWTYPES_BOOL        bLastBlock;
  TMWTYPES_ULONG       readBlockNum;
  TMWTYPES_ULONG       writeBlockNum;

  DNPCHNL_TX_DATA     *pTxData;
  DNPCHNL_RESP_STATUS  status;
} MDNPFILE_XFER_CONTEXT;

#ifdef __cplusplus
extern "C" {
#endif

  /* function: mdnpfile_fileInfo
   * purpose: 
   * arguments:
   * returns:
   *  pointer to transmit data structure if request was built and queued
   *  TMWDEFS_NULL otherwise
   */
  TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpfile_fileInfo(
    void *pReqD,
    TMWTYPES_USHORT requestId,
    const TMWTYPES_CHAR *pFilename);

  /* function: mdnpfile_fileAuthentication 
   * purpose: 
   * arguments:
   * returns:
   *  pointer to transmit data structure if request was built and queued
   *  TMWDEFS_NULL otherwise
   */
  TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpfile_fileAuthentication(
    void *pReqD,
    const TMWTYPES_CHAR *pUsername,
    const TMWTYPES_CHAR *pPassword);

  /* function: mdnpfile_fileOpen 
   * purpose: 
   * arguments:
   * returns:
   *  pointer to transmit data structure if request was built and queued
   *  TMWDEFS_NULL otherwise
   */
  TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpfile_fileOpen(
    void *pReqD,
    TMWTYPES_USHORT requestId,
    DNPDEFS_FILE_MODE mode,
    TMWTYPES_MS_SINCE_70 *pTimeOfCreation,
    TMWTYPES_USHORT permissions,
    TMWTYPES_ULONG authenticationKey,
    TMWTYPES_ULONG fileSize,
    TMWTYPES_USHORT maxBlockSize,
    const TMWTYPES_CHAR *pFilename);

  /* function: mdnpfile_fileClose
   * purpose: 
   * arguments:
   * returns:
   *  pointer to transmit data structure if request was built and queued
   *  TMWDEFS_NULL otherwise
   */
  TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpfile_fileClose(
    void *pReqD,
    TMWTYPES_USHORT requestId,
    TMWTYPES_ULONG handle);

  /* function: mdnpfile_fileDelete 
   * purpose: 
   * arguments:
   * returns:
   *  pointer to transmit data structure if request was built and queued
   *  TMWDEFS_NULL otherwise
   */
  TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpfile_fileDelete(
    void *pReqD,
    TMWTYPES_USHORT requestId,
    TMWTYPES_ULONG authenticationKey,
    const TMWTYPES_CHAR *pUserName,
    const TMWTYPES_CHAR *pPassword,
    const TMWTYPES_CHAR *pFilename);

  /* function: mdnpfile_fileAbort
   * purpose: 
   * arguments:
   * returns:
   *  pointer to transmit data structure if request was built and queued
   *  TMWDEFS_NULL otherwise
   */
  TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpfile_fileAbort(
    void *pReqD,
    TMWTYPES_USHORT requestId,
    TMWTYPES_ULONG handle);

  /* function: mdnpfile_fileRead 
   * purpose: 
   * arguments:
   * returns:
   *  pointer to transmit data structure if request was built and queued
   *  TMWDEFS_NULL otherwise
   */
  TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpfile_fileRead(
    void *pReqD,
    TMWTYPES_ULONG block,
    TMWTYPES_ULONG handle);

  /* function: mdnpfile_fileWrite 
   * purpose: 
   * arguments:
   * returns:
   *  pointer to transmit data structure if request was built and queued
   *  TMWDEFS_NULL otherwise
   */
  TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpfile_fileWrite(
    void *pReqD,
    TMWTYPES_ULONG block,
    TMWTYPES_BOOL last,
    TMWTYPES_ULONG handle,
    const TMWTYPES_UCHAR *pData,
    TMWTYPES_USHORT length);

  /* function: mdnpfile_copyLocalFileToRemote
   * purpose: 
   * arguments:
   * returns:
   *  pointer to transmit data structure if request was built and queued
   *  TMWDEFS_NULL otherwise
   */
  TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpfile_copyLocalFileToRemote(
    void *pReqD,
    const TMWTYPES_CHAR *pLocalFileName,
    const TMWTYPES_CHAR *pRemoteFileName,
    const TMWTYPES_CHAR *pUserName,
    const TMWTYPES_CHAR *pPassword,
    const TMWTYPES_USHORT permissions);

  /* function: mdnpfile_copyRemoteFileToLocal
   * purpose: 
   * arguments:
   * returns:
   *  pointer to transmit data structure if request was built and queued
   *  TMWDEFS_NULL otherwise
   */
  TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpfile_copyRemoteFileToLocal(
    void *pReqD,
    const TMWTYPES_CHAR *pLocalFileName,
    const TMWTYPES_CHAR *pRemoteFileName,
    const TMWTYPES_CHAR *pUserName,
    const TMWTYPES_CHAR *pPassword);
  
  /* function: mdnpfile_copyRemoteFileToLocal  */
  TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpfile_readRemoteDirectory(
    void *pReqD,
    const TMWTYPES_CHAR *pRemoteDirName,
    const TMWTYPES_CHAR *pUserName,
    const TMWTYPES_CHAR *pPassword);

  /* function: mdnpfile_fileXferCB 
   * purpose: File transfer callback function to be called by mdnpsesn.c when
   *  a object 70 response is received, either because it is an immediate response
   *  or a delayed event response after an initial immediate NULL response.
   * arguments:
   *  pSession - pointer to master session
   *  pResponse - pointer to response received from slave
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL mdnpfile_fileXferCB(
    TMWSESN               *pSession,
    DNPCHNL_RESPONSE_INFO *pResponse);

#ifdef __cplusplus
}
#endif
#endif /* MDNPFILE_DEFINED */
