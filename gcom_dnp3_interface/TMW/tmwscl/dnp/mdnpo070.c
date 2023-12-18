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

/* file: mdnpo070.c
 * description: DNP Master functionality for Object 70 File Transfer
 */
#include "tmwscl/dnp/dnpdiag.h"
#include "tmwscl/dnp/mdnpdiag.h"
#include "tmwscl/dnp/mdnpo070.h"
#include "tmwscl/dnp/mdnpdata.h"
#include "tmwscl/dnp/mdnpcnfg.h"
#include "tmwscl/dnp/dnpdtime.h"

#include "tmwscl/utils/tmwdb.h"
#include "tmwscl/utils/tmwtarg.h"
 
#if MDNPDATA_SUPPORT_OBJ70

#if MDNPDATA_SUPPORT_OBJ70_V2
/* function: mdnpo070_RespObj70v2 */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpo070_RespObj70v2(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{
  MDNPSESN *pMDNPSession = (MDNPSESN *)pSession;
  TMWTYPES_USHORT bytesInObject;
  TMWTYPES_USHORT usernameOffset;
  TMWTYPES_USHORT usernameSize;
  TMWTYPES_USHORT passwordOffset;
  TMWTYPES_USHORT passwordSize;
  TMWTYPES_ULONG  authKey;

  TMWTARG_UNUSED_PARAM(pSession);
  TMWTARG_UNUSED_PARAM(pObjHeader);

  /* Protect against badly formatted message */
  if(pRxFragment->offset + 14 > pRxFragment->msgLength)
  {
    DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_INVALID_SIZE);
    return(TMWDEFS_FALSE);
  }

  tmwtarg_get16(&pRxFragment->pMsgBuf[pRxFragment->offset], &bytesInObject);
  pRxFragment->offset += 2;
 
  tmwtarg_get16(&pRxFragment->pMsgBuf[pRxFragment->offset], &usernameOffset);
  pRxFragment->offset += 2;

  tmwtarg_get16(&pRxFragment->pMsgBuf[pRxFragment->offset], &usernameSize);
  pRxFragment->offset += 2; 
  
  tmwtarg_get16(&pRxFragment->pMsgBuf[pRxFragment->offset], &passwordOffset);
  pRxFragment->offset += 2;
  
  tmwtarg_get16(&pRxFragment->pMsgBuf[pRxFragment->offset], &passwordSize);
  pRxFragment->offset += 2;
  
  tmwtarg_get32(&pRxFragment->pMsgBuf[pRxFragment->offset], &authKey);
  pRxFragment->offset += 4;

  /* user name and password are always NULL in the response
   * so don't bother providing them 
   */
  pRxFragment->offset = pRxFragment->offset + usernameSize;
  pRxFragment->offset = pRxFragment->offset + passwordSize;

  if(pRxFragment->offset > pRxFragment->msgLength)
  {
    DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_INVALID_SIZE);
    return(TMWDEFS_FALSE);
  }

  MDNPDIAG_SHOW_FILE_AUTH(pSession, authKey);

  /* Call data function here */
  mdnpdata_storeFileAuthKey(pMDNPSession->pDbHandle, authKey);

  if(pMDNPSession->pFileXferContext != TMWDEFS_NULL)
  {
    pMDNPSession->pInternalCallback = mdnpfile_fileXferCB;
  }

  return(TMWDEFS_TRUE);
}
#endif /* MDNPDATA_SUPPORT_OBJ70_V2 */
 
#if MDNPDATA_SUPPORT_OBJ70_V4
/* function: mdnpo070_RespObj70v4 */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpo070_RespObj70v4(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{
  MDNPSESN *pMDNPSession = (MDNPSESN *)pSession;
  DNPCHNL *pDNPChannel = (DNPCHNL *)pSession->pChannel;
  const TMWTYPES_CHAR *pOptionalChars = TMWDEFS_NULL;
  TMWTYPES_USHORT bytesInObject;
  TMWTYPES_ULONG  handle;
  TMWTYPES_ULONG  fileSize;
  TMWTYPES_USHORT maxBlockSize;
  TMWTYPES_USHORT requestId;
  DNPDEFS_FILE_CMD_STAT status;
  TMWTYPES_USHORT nOptionalChars;

  TMWTARG_UNUSED_PARAM(pObjHeader);

  /* Protect against badly formatted message */
  if(pRxFragment->offset + 15 > pRxFragment->msgLength)
  {
    DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_INVALID_SIZE);
    return(TMWDEFS_FALSE);
  }

  tmwtarg_get16(&pRxFragment->pMsgBuf[pRxFragment->offset], &bytesInObject);
  pRxFragment->offset += 2;
 
  tmwtarg_get32(&pRxFragment->pMsgBuf[pRxFragment->offset], &handle);
  pRxFragment->offset += 4;

  tmwtarg_get32(&pRxFragment->pMsgBuf[pRxFragment->offset], &fileSize);
  pRxFragment->offset += 4;

  tmwtarg_get16(&pRxFragment->pMsgBuf[pRxFragment->offset], &maxBlockSize);
  pRxFragment->offset += 2;

  tmwtarg_get16(&pRxFragment->pMsgBuf[pRxFragment->offset], &requestId);
  pRxFragment->offset += 2;

  tmwtarg_get8(&pRxFragment->pMsgBuf[pRxFragment->offset], &status);
  pRxFragment->offset += 1;

  nOptionalChars = 0;
  if(bytesInObject > 13)
  {
    nOptionalChars = (TMWTYPES_USHORT)(bytesInObject - 13);
    if(nOptionalChars > 0)
    {
      pOptionalChars = (const TMWTYPES_CHAR *)(&pRxFragment->pMsgBuf[pRxFragment->offset]);
    }
    pRxFragment->offset = pRxFragment->offset + nOptionalChars;
  }

  if(pRxFragment->offset > pRxFragment->msgLength)
  {
    DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_INVALID_SIZE);
    return(TMWDEFS_FALSE);
  }

  DNPDIAG_SHOW_FILE_STATUS(
    pSession,
    handle,
    fileSize,
    maxBlockSize,
    requestId,
    status,
    nOptionalChars,
    pOptionalChars);
 
  /* Call data function here */
  mdnpdata_storeFileStatus(
    pMDNPSession->pDbHandle, 
    handle,
    fileSize,
    maxBlockSize,
    requestId,
    status,
    nOptionalChars,
    pOptionalChars);
  
  if(pMDNPSession->pFileXferContext != TMWDEFS_NULL)
  {
    if(maxBlockSize > pDNPChannel->txFragmentSize)
      maxBlockSize = pDNPChannel->txFragmentSize - 16;

    pMDNPSession->pFileXferContext->remoteFileHandle = handle;
    pMDNPSession->pFileXferContext->fileSize = fileSize;
    pMDNPSession->pFileXferContext->maxBlockSize = maxBlockSize;
    pMDNPSession->pInternalCallback = mdnpfile_fileXferCB;

    /* Validate file state */
    if(status != DNPDEFS_FILE_CMD_STAT_SUCCESS)
      pMDNPSession->pFileXferContext->status = DNPCHNL_RESP_STATUS_FAILURE;
  } 
  else
  { 
    if(status != DNPDEFS_FILE_CMD_STAT_SUCCESS)
      return(TMWDEFS_FALSE);
  }
  
  return(TMWDEFS_TRUE);
}
#endif /* MDNPDATA_SUPPORT_OBJ70_V4 */
 
#if MDNPDATA_SUPPORT_OBJ70_V5
/* function: mdnpo070_RespObj70v5 */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpo070_RespObj70v5(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{
  MDNPSESN *pMDNPSession = (MDNPSESN *)pSession;
  TMWTYPES_USHORT bytesInObject;
  TMWTYPES_ULONG  handle;
  TMWTYPES_ULONG  blockNumber;
  TMWTYPES_BOOL   lastBlockFlag;
  TMWTYPES_UCHAR *pBlockData = TMWDEFS_NULL;
  TMWTYPES_USHORT nBytesInBlockData = 0;

  TMWTARG_UNUSED_PARAM(pSession);
  TMWTARG_UNUSED_PARAM(pObjHeader);
  
  /* Protect against badly formatted message */
  if(pRxFragment->offset + 10 > pRxFragment->msgLength)
  {
    DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_INVALID_SIZE);
    return(TMWDEFS_FALSE);
  }

  tmwtarg_get16(&pRxFragment->pMsgBuf[pRxFragment->offset], &bytesInObject);
  pRxFragment->offset += 2;
 
  tmwtarg_get32(&pRxFragment->pMsgBuf[pRxFragment->offset], &handle);
  pRxFragment->offset += 4;

  tmwtarg_get32(&pRxFragment->pMsgBuf[pRxFragment->offset], &blockNumber);
  pRxFragment->offset += 4;

  /* top bit of blockNumber is last block inidicator */
  lastBlockFlag = (TMWTYPES_BOOL)((blockNumber & 0x80000000) != 0U);
  blockNumber &= 0x7fffffff;

  if(bytesInObject < 8)
    return(TMWDEFS_FALSE);
 
  nBytesInBlockData = (TMWTYPES_USHORT)(bytesInObject - 8);
  if(nBytesInBlockData > 0)
  {
    pBlockData = &pRxFragment->pMsgBuf[pRxFragment->offset];
  }

  if((pRxFragment->offset+nBytesInBlockData) > pRxFragment->msgLength)
  {
    pRxFragment->offset = pRxFragment->msgLength;
    DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_INVALID_SIZE);
    return(TMWDEFS_FALSE);
  }

  DNPDIAG_SHOW_FILE_DATA(pSession, handle, blockNumber, nBytesInBlockData, lastBlockFlag, TMWDEFS_TRUE, TMWDIAG_ID_RX);
   
  /* Call data function here */
  mdnpdata_storeFileData(pMDNPSession->pDbHandle, 
    handle, blockNumber, lastBlockFlag, nBytesInBlockData, pBlockData);   
  
  if(pMDNPSession->pFileXferContext != TMWDEFS_NULL)
  {
    pMDNPSession->pInternalCallback = mdnpfile_fileXferCB;
    pMDNPSession->pFileXferContext->bLastBlock = lastBlockFlag;

    if(pMDNPSession->fileXferType == DNPDEFS_FILE_TYPE_SIMPLE)
    {
      pRxFragment->offset = pRxFragment->offset + nBytesInBlockData;

      /* Validate file handle if doing full file xfer */
      if(handle != pMDNPSession->pFileXferContext->remoteFileHandle)
      {
        pMDNPSession->pFileXferContext->status = DNPCHNL_RESP_STATUS_FAILURE;
        return(TMWDEFS_TRUE);
      }
     
      if((nBytesInBlockData == 0) && (!lastBlockFlag))
      {
        pMDNPSession->pFileXferContext->status = DNPCHNL_RESP_STATUS_FAILURE;
        return(TMWDEFS_TRUE);
      }
    }
    else
    {
      /* Directory read */
      TMWTYPES_ULONG lastByteIndex = pRxFragment->offset + bytesInObject -8;
      while(pRxFragment->offset < lastByteIndex)
      {
        /* loop here reading all variation 7 objects
        */
        TMWTYPES_USHORT fileNameOffset;
        TMWTYPES_USHORT fileNameSize;
        DNPDEFS_FILE_TYPE fileType;
        TMWTYPES_ULONG  fileSize;
        TMWTYPES_MS_SINCE_70 msSince70;
        TMWDTIME fileCreationTime;
        DNPDEFS_FILE_PERMISSIONS permissions;
        TMWTYPES_USHORT requestID;
        const TMWTYPES_CHAR *pFileName = TMWDEFS_NULL;

        /* Protect against badly formatted message */
        if(pRxFragment->offset + 20 > pRxFragment->msgLength)
        {
          DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_INVALID_SIZE);
          return(TMWDEFS_FALSE);
        }

        tmwtarg_get16(&pRxFragment->pMsgBuf[pRxFragment->offset], &fileNameOffset);
        pRxFragment->offset += 2;

        tmwtarg_get16(&pRxFragment->pMsgBuf[pRxFragment->offset], &fileNameSize);
        pRxFragment->offset += 2;

        tmwtarg_get16(&pRxFragment->pMsgBuf[pRxFragment->offset], &fileType);
        pRxFragment->offset += 2;

        tmwtarg_get32(&pRxFragment->pMsgBuf[pRxFragment->offset], &fileSize);
        pRxFragment->offset += 4;

        dnpdtime_readMsSince70(&msSince70, &pRxFragment->pMsgBuf[pRxFragment->offset]);  
        dnpdtime_msSince70ToDateTime(&fileCreationTime, &msSince70);
        pRxFragment->offset += 6;

        tmwtarg_get16(&pRxFragment->pMsgBuf[pRxFragment->offset], &permissions);
        pRxFragment->offset += 2;

        tmwtarg_get16(&pRxFragment->pMsgBuf[pRxFragment->offset], &requestID);
        pRxFragment->offset += 2;
    
        if (fileNameSize > 0)
        {
          pFileName = (const TMWTYPES_CHAR *)(&pRxFragment->pMsgBuf[pRxFragment->offset]);
          pRxFragment->offset = pRxFragment->offset + fileNameSize;
        }
        
        /* Check to see if reading this as an array of variation 7 objects made sense */
        if(((fileType != 0
          && fileType != 1)) 
          ||(pRxFragment->offset > pRxFragment->msgLength))
        {
          /* This response is not formatted correctly, don't try to parse */
          MDNPDIAG_ERROR(pSession->pChannel, pSession, MDNPDIAG_FILE_DIR_READ);
          pMDNPSession->pFileXferContext->status = DNPCHNL_RESP_STATUS_FAILURE;
          return(TMWDEFS_FALSE);
        }
              
        DNPDIAG_SHOW_FILE_INFO(
          pSession,
          fileNameOffset,
          fileNameSize,
          fileType,
          fileSize,
          &fileCreationTime,
          permissions,
          pFileName);

        /* Call data function here */
        mdnpdata_storeFileInfo(
          pMDNPSession->pDbHandle, 
          fileNameOffset,
          fileNameSize,
          fileType,
          fileSize,
          &fileCreationTime,
          permissions,
          pFileName);
      }
    }
  }
  else
  {
    pRxFragment->offset = pRxFragment->offset + nBytesInBlockData;
  }
  return(TMWDEFS_TRUE);
}
#endif /* MDNPDATA_SUPPORT_OBJ70_V5 */
 
#if MDNPDATA_SUPPORT_OBJ70_V6
/* function: mdnpo070_RespObj70v6 */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpo070_RespObj70v6(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{
  MDNPSESN *pMDNPSession = (MDNPSESN *)pSession;
  const TMWTYPES_CHAR *pOptionalChars = TMWDEFS_NULL;
  TMWTYPES_USHORT bytesInObject;
  TMWTYPES_ULONG  handle;
  TMWTYPES_ULONG  blockNumber;
  TMWTYPES_BOOL   lastBlockFlag;
  TMWTYPES_USHORT nOptionalChars;
  DNPDEFS_FILE_TFER_STAT  status;

  TMWTARG_UNUSED_PARAM(pObjHeader);

  /* Protect against badly formatted message */
  if(pRxFragment->offset + 11 > pRxFragment->msgLength)
  {
    DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_INVALID_SIZE);
    return(TMWDEFS_FALSE);
  }

  tmwtarg_get16(&pRxFragment->pMsgBuf[pRxFragment->offset], &bytesInObject);
  pRxFragment->offset += 2;
 
  tmwtarg_get32(&pRxFragment->pMsgBuf[pRxFragment->offset], &handle);
  pRxFragment->offset += 4;

  tmwtarg_get32(&pRxFragment->pMsgBuf[pRxFragment->offset], &blockNumber);
  pRxFragment->offset += 4;

  tmwtarg_get8(&pRxFragment->pMsgBuf[pRxFragment->offset], &status);
  pRxFragment->offset += 1;

  /* top bit of blockNumber is last block indicator */
  lastBlockFlag = (TMWTYPES_BOOL)((blockNumber & 0x80000000) != 0);
  blockNumber &= 0x7fffffff;
  
  if(bytesInObject < 9)
    return(TMWDEFS_FALSE);

  nOptionalChars = (TMWTYPES_USHORT)(bytesInObject - 9);
  if(nOptionalChars > 0)
  {
    pOptionalChars = (const TMWTYPES_CHAR *)(&pRxFragment->pMsgBuf[pRxFragment->offset]);
  }
  pRxFragment->offset = pRxFragment->offset + nOptionalChars;

  if(pRxFragment->offset > pRxFragment->msgLength)
  {
    DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_INVALID_SIZE);
    return(TMWDEFS_FALSE);
  }

  DNPDIAG_SHOW_FILE_DATA_STATUS(
    pSession,
    handle,
    blockNumber,
    lastBlockFlag,
    status,
    nOptionalChars,
    pOptionalChars);

  /* Call data function here */
  mdnpdata_storeFileDataStatus(
    pMDNPSession->pDbHandle, 
    handle,
    blockNumber,
    lastBlockFlag,
    status,
    nOptionalChars,
    pOptionalChars);

  if(pMDNPSession->pFileXferContext != TMWDEFS_NULL)
  {
    pMDNPSession->pInternalCallback = mdnpfile_fileXferCB;

    pMDNPSession->pFileXferContext->bLastBlock = lastBlockFlag;

    if((status != DNPDEFS_FILE_TFER_STAT_SUCCESS)
      ||(handle != pMDNPSession->pFileXferContext->remoteFileHandle))
    {
      pMDNPSession->pFileXferContext->status = DNPCHNL_RESP_STATUS_FAILURE;
    }
  }
  
  return(TMWDEFS_TRUE);
}
#endif /* MDNPDATA_SUPPORT_OBJ70_V6 */
 
#if MDNPDATA_SUPPORT_OBJ70_V7
/* function: mdnpo070_RespObj70v7 */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpo070_RespObj70v7(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{
  MDNPSESN *pMDNPSession = (MDNPSESN *)pSession;
  const TMWTYPES_CHAR *pFileName = TMWDEFS_NULL;
  TMWTYPES_USHORT bytesInObject;
  TMWTYPES_USHORT fileNameOffset;
  TMWTYPES_USHORT fileNameSize;
  DNPDEFS_FILE_TYPE fileType;
  TMWTYPES_ULONG  fileSize;
  TMWTYPES_MS_SINCE_70 msSince70;
  TMWDTIME fileCreationTime;
  DNPDEFS_FILE_PERMISSIONS permissions;
  TMWTYPES_USHORT requestID;

  TMWTARG_UNUSED_PARAM(pObjHeader);

  /* Protect against badly formatted message */
  if(pRxFragment->offset + 22 > pRxFragment->msgLength)
  {
    DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_INVALID_SIZE);
    return(TMWDEFS_FALSE);
  }

  tmwtarg_get16(&pRxFragment->pMsgBuf[pRxFragment->offset], &bytesInObject);
  pRxFragment->offset += 2;
 
  tmwtarg_get16(&pRxFragment->pMsgBuf[pRxFragment->offset], &fileNameOffset);
  pRxFragment->offset += 2;

  tmwtarg_get16(&pRxFragment->pMsgBuf[pRxFragment->offset], &fileNameSize);
  pRxFragment->offset += 2;

  tmwtarg_get16(&pRxFragment->pMsgBuf[pRxFragment->offset], &fileType);
  pRxFragment->offset += 2;

  tmwtarg_get32(&pRxFragment->pMsgBuf[pRxFragment->offset], &fileSize);
  pRxFragment->offset += 4;

  dnpdtime_readMsSince70(&msSince70, &pRxFragment->pMsgBuf[pRxFragment->offset]);  
  dnpdtime_msSince70ToDateTime(&fileCreationTime, &msSince70);
  pRxFragment->offset += 6;

  tmwtarg_get16(&pRxFragment->pMsgBuf[pRxFragment->offset], &permissions);
  pRxFragment->offset += 2;

  tmwtarg_get16(&pRxFragment->pMsgBuf[pRxFragment->offset], &requestID);
  pRxFragment->offset += 2;

  if (fileNameSize > 0)
  {
    pFileName = (const TMWTYPES_CHAR *)(&pRxFragment->pMsgBuf[pRxFragment->offset]);
    pRxFragment->offset = pRxFragment->offset + fileNameSize;
  }
   
  if(pRxFragment->offset > pRxFragment->msgLength)
  {
    DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_INVALID_SIZE);
    return(TMWDEFS_FALSE);
  }

  DNPDIAG_SHOW_FILE_INFO(
    pSession,
    fileNameOffset,
    fileNameSize,
    fileType,
    fileSize,
    &fileCreationTime,
    permissions,
    pFileName);

  /* Call data function here */
  mdnpdata_storeFileInfo(
    pMDNPSession->pDbHandle, 
    fileNameOffset,
    fileNameSize,
    fileType,
    fileSize,
    &fileCreationTime,
    permissions,
    pFileName);  
  
  if(pMDNPSession->pFileXferContext != TMWDEFS_NULL)
  {
    pMDNPSession->pInternalCallback = mdnpfile_fileXferCB;
  }

  return(TMWDEFS_TRUE);
}
#endif /* MDNPDATA_SUPPORT_OBJ70_V7 */
 
#endif /* MDNPDATA_SUPPORT_OBJ70 */
