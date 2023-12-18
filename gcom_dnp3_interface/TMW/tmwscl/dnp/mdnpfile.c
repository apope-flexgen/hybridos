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

/* file: mdnpfile.c
 * description: Implement methods used to copy files.
 */

#include "tmwscl/dnp/dnpdiag.h"
#include "tmwscl/dnp/mdnpdiag.h"
#include "tmwscl/dnp/mdnpbrm.h"
#include "tmwscl/dnp/dnpdefs.h"
#include "tmwscl/dnp/dnpdtime.h"
#include "tmwscl/dnp/dnpchnl.h"
#include "tmwscl/dnp/mdnpsesn.h"
#include "tmwscl/dnp/mdnpfile.h"
#include "tmwscl/dnp/mdnpmem.h"
#include "tmwscl/utils/tmwtarg.h"
#include "tmwscl/dnp/mdnpdata.h"

#if MDNPDATA_SUPPORT_OBJ70
static TMWTYPES_USHORT _mdnpRequestId = 0;
  
/* function: _allocateFileXferContext 
 * purpose:  Allocate file transfer context structure
 * arguments:
 *  pMDNPSession - pointer to master session structure
 * returns:
 *  pointer to file transfer context structure 
 *  TMWDEFS_NULL if structure could not be allocated
 */
static TMWDEFS_LOCAL MDNPFILE_XFER_CONTEXT *_allocateFileXferContext(
  MDNPSESN *pMDNPSession)
{
  MDNPFILE_XFER_CONTEXT *pFileXferContext;

  if(pMDNPSession->pFileXferContext != TMWDEFS_NULL)
  {
    /* File transfer context already in use and 
     * currently only one file transfer context per session is supported 
     */
    return(TMWDEFS_NULL);
  }

  pFileXferContext = (MDNPFILE_XFER_CONTEXT *)mdnpmem_alloc(MDNPMEM_FILE_TRANSFER_TYPE);
  if(pFileXferContext == TMWDEFS_NULL)
  {
    /* Could not allocate callback structure */
    return(TMWDEFS_NULL);
  }

  pFileXferContext->localFileOpen    = TMWDEFS_FALSE;
  pFileXferContext->bLastBlock       = TMWDEFS_FALSE;
  pFileXferContext->useAggressiveMode= TMWDEFS_FALSE;
  pFileXferContext->fileSize         = 0;
  pFileXferContext->readBlockNum     = 0;
  pFileXferContext->writeBlockNum    = 0;
  pFileXferContext->remoteFileHandle = 0;
  pFileXferContext->nextOperation    = MDNPFILE_XFER_STATE_RETURN;
  pFileXferContext->timeOfCreation.leastSignificant = 0;
  pFileXferContext->timeOfCreation.mostSignificant  = 0;

  pMDNPSession->pFileXferContext = pFileXferContext;

  return(pFileXferContext);
}
/* function: _fileXferCB 
 * purpose:  Internal callback function to handle NULL response or to call file
 *  transfer state machine callback function. This will be the internal callback
 *  function for the file transfer txData request. This will be called for the
 *  response received for a file transfer request, either the immediate data
 *  filled response or the NULL response indicating data will follow as polled or
 *  unsolicited response.
 * arguments:
 *   pCallbackParam - pointer to master session
 *   pResponse - pointer to structure containing response received
 * returns:
 *   void
 */
static void TMWDEFS_LOCAL _fileXferCB(
  void                  *pCallbackParam,
  DNPCHNL_RESPONSE_INFO *pResponse)
{
  MDNPSESN *pMDNPSession   = (MDNPSESN *)pCallbackParam;
  
  /* If this is a Null response, 
   * the data containing response should come later as an event 
   */
  if((pResponse->pRxData != TMWDEFS_NULL)
    && (pResponse->pRxData->msgLength == 4))
  {
    if(pResponse->status == DNPCHNL_RESP_STATUS_SUCCESS)
    {
      pResponse->status = DNPCHNL_RESP_STATUS_INTERMEDIATE;
      pResponse->requestStatus = 0;
      pResponse->last   = TMWDEFS_FALSE;
    
      /* Put this request back on the queue, so it will be here when the response
       * is received or this request times out
       */
      tmwdlist_addEntry(&pResponse->pTxData->pChannel->messageQueue,
        (TMWDLIST_MEMBER *)pResponse->pTxData);
      return;
    }
  }

  /* If mdnpfile_fileXferCB was already called directly for this response,
   * just return.
   */
  if(pMDNPSession->pInternalCallback != TMWDEFS_NULL)
    return;

  mdnpfile_fileXferCB((TMWSESN *)pCallbackParam, pResponse);
}

/* function: _initTxData
 * purpose: Initialize the tx data structure
 * arguments:
 *  pTxData
 *  pReqDesc
 *  defultMsgDescription
 * returns:
 *  TMWDEFS_TRUE if successful
 *  TMWDEFS_FALSE otherwise
 */
static TMWTYPES_BOOL TMWDEFS_LOCAL _initTxData(
  DNPCHNL_TX_DATA *pTxData,
  MDNPBRM_REQ_DESC *pReqDesc,
  const char *defaultMsgDescription)
{
  if(pReqDesc->pSession == TMWDEFS_NULL)
  {
    return(TMWDEFS_FALSE);
  }
  else
  {
    /* Validate session */
    if((pReqDesc->pSession->protocol != TMWTYPES_PROTOCOL_DNP)
      || (pReqDesc->pSession->type != TMWTYPES_SESSION_TYPE_MASTER))
    {
      MDNPDIAG_ERROR(pReqDesc->pChannel, pReqDesc->pSession, MDNPDIAG_INV_PROTOCOL);
      return(TMWDEFS_FALSE);
    }

    /* Initialize session */
    pTxData->tmw.destAddress = pTxData->tmw.pSession->destAddress;
  }

  /* Initialize request parameters */
  pTxData->priority = pReqDesc->priority;

#if MDNPDATA_SUPPORT_OBJ120
  pTxData->authUserNumber = pReqDesc->authUserNumber;
  pTxData->authAggressiveMode = pReqDesc->authAggressiveMode;
#endif

  /* Response timeout */
  pTxData->tmw.responseTimeout = pReqDesc->responseTimeout;

  /* Initialize user callback info */
  pTxData->pUserCallback = pReqDesc->pUserCallback;
  pTxData->pUserCallbackParam = pReqDesc->pUserCallbackParam;

  /* Initialize message description */
  if(pReqDesc->pMsgDescription != TMWDEFS_NULL)
    pTxData->tmw.pMsgDescription = pReqDesc->pMsgDescription;
  else
    pTxData->tmw.pMsgDescription = defaultMsgDescription;

  return(TMWDEFS_TRUE);
}
/* function: _initFileTxData
 * purpose: Initialize the tx data structure
 * arguments:
 *  pTxData
 *  pReqDesc
 *  defultMsgDescription
 * returns:
 *  TMWDEFS_TRUE if successful
 *  TMWDEFS_FALSE otherwise
 */
static TMWTYPES_BOOL TMWDEFS_LOCAL _initFileTxData(
  DNPCHNL_TX_DATA *pTxData,
  MDNPBRM_REQ_DESC *pReqDesc)
{
  /* Initialize internal callback info */
  pTxData->pInternalCallback = _fileXferCB; 
  pTxData->pInternalCallbackParam = pReqDesc->pSession; 

  /* Save this so we can match up response when it arrives as an event */
  if(((MDNPSESN *)pReqDesc->pSession)->pFileXferContext != TMWDEFS_NULL)
  {
    ((MDNPSESN *)pReqDesc->pSession)->pFileXferContext->pTxData = (DNPCHNL_TX_DATA *)pTxData;
    ((MDNPSESN *)pReqDesc->pSession)->pFileXferContext->status = DNPCHNL_RESP_STATUS_SUCCESS;
  }
  return(TMWDEFS_TRUE);
}

/* function: _reinitTxData
 * purpose: Reinitialize the tx data structure
 * arguments:
 *  pTxData
 *  msgDescription
 * returns:
 *  void
 */
static void TMWDEFS_LOCAL _reinitTxData(
  DNPCHNL_TX_DATA *pTxData,
  const char *msgDescription)
{
#if TMWCNFG_SUPPORT_DIAG
  pTxData->tmw.pMsgDescription = msgDescription;
#else
  TMWTARG_UNUSED_PARAM(msgDescription);
#endif
  pTxData->tmw.txFlags = 0;
  pTxData->sent = TMWDEFS_FALSE;
}

/* function: _cleanupFileXfer
 * purpose: Deallocate file transfer structures
 * arguments:
 *  pMDNPSession
 *  pTxData
 * returns:
 *  void
 */
static void TMWDEFS_LOCAL _cleanupFileXfer(
  MDNPSESN *pMDNPSession,
  TMWSESN_TX_DATA *pTxData)
{
  if(pMDNPSession->pFileXferContext != TMWDEFS_NULL)
  {
    if(pMDNPSession->pFileXferContext->localFileOpen)
    {
      mdnpdata_closeLocalFile(pMDNPSession->pDbHandle);
    }
    mdnpmem_free(pMDNPSession->pFileXferContext);
    pMDNPSession->pFileXferContext = TMWDEFS_NULL;
  }

  if(pTxData != TMWDEFS_NULL)
  {
    dnpchnl_freeTxData(pTxData);
  }
}

/* function: _fileOpen
 * purpose: Build and send a file open Obj70V3 request.
 * arguments:
 *  pTxData - transmit data structure to use for building and sending request
 * returns:
 *  pointer to txData if request was built and sent to slave
 *  TMWDEFS_NULL otherwise 
 */
static TMWSESN_TX_DATA * TMWDEFS_LOCAL _fileOpen(
  TMWSESN_TX_DATA *pTxData,
  TMWTYPES_USHORT requestId,
  DNPDEFS_FILE_MODE mode,
  TMWTYPES_MS_SINCE_70 *pTimeOfCreation,
  TMWTYPES_USHORT permissions,
  TMWTYPES_ULONG authenticationKey,
  TMWTYPES_ULONG fileSize,
  TMWTYPES_USHORT maxBlockSize,
  TMWTYPES_CHAR *pFilename)
{
  TMWTYPES_ULONG   tmpLong;
  TMWTYPES_USHORT  tmpShort;
  DNPCHNL_TX_DATA *pChnlTxData    = (DNPCHNL_TX_DATA *)pTxData;
  MDNPSESN *pMDNPSession          = (MDNPSESN*)pTxData->pSession;
  TMWTYPES_USHORT  filenameLength = (TMWTYPES_USHORT)strlen(pFilename); 

  pMDNPSession->pFileXferContext->maxBlockSize = maxBlockSize;

  /* Build request header */
  mdnpbrm_buildRequestHeader(pChnlTxData, DNPDEFS_FC_OPEN_FILE);

#if MDNPDATA_SUPPORT_OBJ120
  if(pChnlTxData->authAggressiveMode)
  {
    if(mdnpauth_addAggrRequestStart(pTxData, pChnlTxData->authUserNumber) == TMWDEFS_NULL)
    {
      return(TMWDEFS_NULL);
    }
  }
#endif 

  /* Time Object Header */
  if(!mdnpbrm_addObjectHeader(pChnlTxData, DNPDEFS_OBJ_70_FILE_IDENTIFIER, 3, DNPDEFS_QUAL_16BIT_FREE_FORMAT, 0, 1))
  {
    return(TMWDEFS_NULL);
  }

  /* Size */
  tmpShort = (TMWTYPES_USHORT)(26 + filenameLength);
  tmwtarg_store16(&tmpShort, &pTxData->pMsgBuf[pTxData->msgLength]);
  pTxData->msgLength += 2;

  /* Filename Offset */
  tmpShort = 26;
  tmwtarg_store16(&tmpShort, &pTxData->pMsgBuf[pTxData->msgLength]);
  pTxData->msgLength += 2;

  /* Filename Size */
  tmwtarg_store16(&filenameLength, &pTxData->pMsgBuf[pTxData->msgLength]);
  pTxData->msgLength += 2;

  /* Time Of Creation */
  if(mode == DNPDEFS_FILE_MODE_READ)
  {
    tmpLong = 0; 
    tmwtarg_store32(&tmpLong, &pTxData->pMsgBuf[pTxData->msgLength]);
    pTxData->msgLength += 4;

    tmpShort = 0;
    tmwtarg_store16(&tmpShort, &pTxData->pMsgBuf[pTxData->msgLength]);
    pTxData->msgLength += 2;
  }
  else
  { 
    dnpdtime_writeMsSince70(pTxData->pMsgBuf + pTxData->msgLength, pTimeOfCreation);
    pTxData->msgLength += 6;
  }

  /* Permissions */
  tmwtarg_store16(&permissions, &pTxData->pMsgBuf[pTxData->msgLength]);
  pTxData->msgLength += 2;

  /* Authentication Key */
  tmwtarg_store32(&authenticationKey, &pTxData->pMsgBuf[pTxData->msgLength]);
  pTxData->msgLength += 4;

  /* File Size */
  tmwtarg_store32(&fileSize, &pTxData->pMsgBuf[pTxData->msgLength]);
  pTxData->msgLength += 4;

  /* Operational Mode */
  tmwtarg_store16(&mode, &pTxData->pMsgBuf[pTxData->msgLength]);
  pTxData->msgLength += 2;

  /* Maximum Block Size */
  tmwtarg_store16(&maxBlockSize, &pTxData->pMsgBuf[pTxData->msgLength]);
  pTxData->msgLength += 2;

  /* Request ID */
  tmwtarg_store16(&requestId, &pTxData->pMsgBuf[pTxData->msgLength]);
  pTxData->msgLength += 2;

  /* Filename */
  memcpy(&pTxData->pMsgBuf[pTxData->msgLength], pFilename, filenameLength);
  pTxData->msgLength = (TMWTYPES_USHORT)(pTxData->msgLength + filenameLength);

  DNPDIAG_SHOW_FILE_OPEN((TMWSESN*)pMDNPSession, pFilename, filenameLength, fileSize, pTimeOfCreation, permissions, authenticationKey, mode, maxBlockSize, requestId);

  /* Send request */
  if(!dnpchnl_sendFragment(pTxData))
  {
    pTxData = TMWDEFS_NULL; 
  }

  return(pTxData);
}

/* function: _fileRead 
 * purpose: Build and send a file read Obj70V5 request.
 * arguments:
 *  pTxData - transmit data structure to use for building and sending request
 * returns:
 *  pointer to txData if request was built and sent to slave
 *  TMWDEFS_NULL otherwise
 */
static TMWSESN_TX_DATA * TMWDEFS_LOCAL _fileRead(
  TMWSESN_TX_DATA *pTxData,
  TMWTYPES_ULONG block,
  TMWTYPES_ULONG handle)
{
  TMWTYPES_USHORT tmp;
  DNPCHNL_TX_DATA *pDNPTxData = (DNPCHNL_TX_DATA *)pTxData;

  /* Build request header */
  mdnpbrm_buildRequestHeader(pDNPTxData, DNPDEFS_FC_READ);

  /* Add Object Header */
  if(!mdnpbrm_addObjectHeader(pDNPTxData, DNPDEFS_OBJ_70_FILE_IDENTIFIER, 5, DNPDEFS_QUAL_16BIT_FREE_FORMAT, 0, 1))
  {
    return(TMWDEFS_NULL);
  }

  /* Number of bytes in object */
  tmp = 8;
  tmwtarg_store16(&tmp, &pTxData->pMsgBuf[pTxData->msgLength]);
  pTxData->msgLength += 2;

  /* File Handle */
  tmwtarg_store32(&handle, &pTxData->pMsgBuf[pTxData->msgLength]);
  pTxData->msgLength += 4;

  /* Block */
  tmwtarg_store32(&block, &pTxData->pMsgBuf[pTxData->msgLength]);
  pTxData->msgLength += 4;

  /* Send request */
  if(!dnpchnl_sendFragment(pTxData))
  { 
    pTxData = TMWDEFS_NULL; 
  }

  return(pTxData);
}

/* function: _fileClose
 * purpose: Build and send a file close Obj70V4 request.
 * arguments:
 *  pTxData - transmit data structure to use for building and sending request
 * returns:
 *  pointer to txData if request was built and sent to slave
 *  TMWDEFS_NULL otherwise 
 */
static TMWSESN_TX_DATA * TMWDEFS_GLOBAL _fileClose(
  TMWSESN_TX_DATA *pTxData,
  TMWTYPES_USHORT requestId,
  TMWTYPES_ULONG handle)
{
  TMWTYPES_ULONG tmpLong;
  TMWTYPES_USHORT tmpShort;

  /* Build request header */
  mdnpbrm_buildRequestHeader((DNPCHNL_TX_DATA *)pTxData, DNPDEFS_FC_CLOSE_FILE);

#if MDNPDATA_SUPPORT_OBJ120
  {
    DNPCHNL_TX_DATA *pChnlTxData = (DNPCHNL_TX_DATA *)pTxData;
    if(pChnlTxData->authAggressiveMode)
    {
      if(mdnpauth_addAggrRequestStart(pTxData, pChnlTxData->authUserNumber) == TMWDEFS_NULL)
      {
        return(TMWDEFS_NULL);
      }
    }
  }
#endif 

  /* Time Object Header */
  if(!mdnpbrm_addObjectHeader((DNPCHNL_TX_DATA *)pTxData, DNPDEFS_OBJ_70_FILE_IDENTIFIER, 4, DNPDEFS_QUAL_16BIT_FREE_FORMAT, 0, 1))
  {
    return(TMWDEFS_NULL);
  }

  /* Size */
  tmpShort = 13;
  tmwtarg_store16(&tmpShort, &pTxData->pMsgBuf[pTxData->msgLength]);
  pTxData->msgLength += 2;

  /* File Handle */
  tmwtarg_store32(&handle, &pTxData->pMsgBuf[pTxData->msgLength]);
  pTxData->msgLength += 4;

  /* File Size */
  tmpLong = 0;
  tmwtarg_store32(&tmpLong, &pTxData->pMsgBuf[pTxData->msgLength]);
  pTxData->msgLength += 4;

  /* Maximum Block Size */
  tmpShort = 0;
  tmwtarg_store16(&tmpShort, &pTxData->pMsgBuf[pTxData->msgLength]);
  pTxData->msgLength += 2;

  /* Request ID */
  tmwtarg_store16(&requestId, &pTxData->pMsgBuf[pTxData->msgLength]);
  pTxData->msgLength += 2;

  /* Status */
  pTxData->pMsgBuf[pTxData->msgLength++] = 0;

  DNPDIAG_SHOW_FILE_CLOSE(pTxData->pSession, handle, requestId);

  /* Send request */
  if(!dnpchnl_sendFragment(pTxData))
  { 
    pTxData = TMWDEFS_NULL; 
  }

  return(pTxData);
}

/* function: _fileDelete
 * purpose: Build and send a file delete Obj70V3 request.
 * arguments:
 *  pTxData - transmit data structure to use for building and sending request
 * returns:
 *  pointer to txData if request was built and sent to slave
 *  TMWDEFS_NULL otherwise 
 */
static TMWSESN_TX_DATA * TMWDEFS_LOCAL _fileDelete(
  TMWSESN_TX_DATA *pTxData,
  TMWTYPES_USHORT requestId,
  TMWTYPES_ULONG authenticationKey,
  const TMWTYPES_CHAR *pFilename)
{ 
  TMWTYPES_ULONG  tmpLong;
  TMWTYPES_USHORT tmpShort; 
  TMWTYPES_USHORT filenameLength = (TMWTYPES_USHORT)strlen(pFilename);
#if MDNPDATA_SUPPORT_OBJ120
  DNPCHNL_TX_DATA *pChnlTxData = (DNPCHNL_TX_DATA *)pTxData;
#endif 

  /* Build request header */
  mdnpbrm_buildRequestHeader((DNPCHNL_TX_DATA *)pTxData, DNPDEFS_FC_DELETE_FILE);

#if MDNPDATA_SUPPORT_OBJ120
  if(pChnlTxData->authAggressiveMode)
  {
    if(mdnpauth_addAggrRequestStart(pTxData, pChnlTxData->authUserNumber) == TMWDEFS_NULL)
    {
      return(TMWDEFS_NULL);
    }
  }
#endif 

  /* Time Object Header */
  if(!mdnpbrm_addObjectHeader((DNPCHNL_TX_DATA *)pTxData, DNPDEFS_OBJ_70_FILE_IDENTIFIER, 3, 
    DNPDEFS_QUAL_16BIT_FREE_FORMAT, 0, 1))
  {
    return(TMWDEFS_NULL);
  }

  /* Size */
  tmpShort = (TMWTYPES_USHORT)(26 + filenameLength);
  tmwtarg_store16(&tmpShort, &pTxData->pMsgBuf[pTxData->msgLength]);
  pTxData->msgLength += 2;

  /* Filename Offset */
  tmpShort = 26;
  tmwtarg_store16(&tmpShort, &pTxData->pMsgBuf[pTxData->msgLength]);
  pTxData->msgLength += 2;

  /* Filename Size */
  tmwtarg_store16(&filenameLength, &pTxData->pMsgBuf[pTxData->msgLength]);
  pTxData->msgLength += 2;

  /* Time Of Creation */
  tmpLong = 0;
  tmwtarg_store32(&tmpLong, &pTxData->pMsgBuf[pTxData->msgLength]);
  pTxData->msgLength += 4;

  tmpShort = 0;
  tmwtarg_store16(&tmpShort, &pTxData->pMsgBuf[pTxData->msgLength]);
  pTxData->msgLength += 2;

  /* Permissions */
  tmwtarg_store16(&tmpShort, &pTxData->pMsgBuf[pTxData->msgLength]);
  pTxData->msgLength += 2;

  /* Authentication Key */
  tmwtarg_store32(&authenticationKey, &pTxData->pMsgBuf[pTxData->msgLength]);
  pTxData->msgLength += 4;

  /* File Size */
  tmpLong = 0;
  tmwtarg_store32(&tmpLong, &pTxData->pMsgBuf[pTxData->msgLength]);
  pTxData->msgLength += 4;

  /* Operational Mode */
  tmpShort = 0;
  tmwtarg_store16(&tmpShort, &pTxData->pMsgBuf[pTxData->msgLength]);
  pTxData->msgLength += 2;

  /* Maximum Block Size */
  tmwtarg_store16(&tmpShort, &pTxData->pMsgBuf[pTxData->msgLength]);
  pTxData->msgLength += 2;

  /* Request ID */
  tmwtarg_store16(&requestId, &pTxData->pMsgBuf[pTxData->msgLength]);
  pTxData->msgLength += 2;

  /* Filename */
  memcpy(&pTxData->pMsgBuf[pTxData->msgLength], pFilename, filenameLength);
  pTxData->msgLength = (TMWTYPES_USHORT)(pTxData->msgLength + filenameLength);

  /* Send request */
  if(!dnpchnl_sendFragment(pTxData))
  { 
    pTxData = TMWDEFS_NULL; 
  }

  return(pTxData);
}

/* function: _fileWrite 
 * purpose: Build and send a file write Obj70V5 request.
 * arguments:
 *  pTxData - transmit data structure to use for building and sending request
 * returns:
 *  pointer to txData if request was built and sent to slave
 *  TMWDEFS_NULL otherwise
 */
static TMWSESN_TX_DATA * TMWDEFS_LOCAL _fileWrite(
  TMWSESN_TX_DATA *pTxData,
  TMWTYPES_ULONG block,
  TMWTYPES_BOOL last,
  TMWTYPES_ULONG handle,
  const TMWTYPES_UCHAR *pData,
  TMWTYPES_USHORT length)
{
#if MDNPDATA_SUPPORT_OBJ120
  DNPCHNL_TX_DATA *pChnlTxData = (DNPCHNL_TX_DATA *)pTxData;
#endif 
  TMWTYPES_USHORT tmp;

  /* Build request header */
  mdnpbrm_buildRequestHeader((DNPCHNL_TX_DATA *)pTxData, DNPDEFS_FC_WRITE);

#if MDNPDATA_SUPPORT_OBJ120
  if(pChnlTxData->authAggressiveMode)
  {
    if(mdnpauth_addAggrRequestStart(pTxData, pChnlTxData->authUserNumber) == TMWDEFS_NULL)
    {
      return(TMWDEFS_NULL);
    }
  }
#endif 

  /* Add Object Header */
  if(!mdnpbrm_addObjectHeader((DNPCHNL_TX_DATA *)pTxData, DNPDEFS_OBJ_70_FILE_IDENTIFIER, 5, 
    DNPDEFS_QUAL_16BIT_FREE_FORMAT, 0, 1))
  {
    return(TMWDEFS_NULL);
  }
  
  DNPDIAG_SHOW_FILE_DATA(pTxData->pSession, handle, block, length, last, TMWDEFS_FALSE, TMWDIAG_ID_TX);

  /* Number of bytes in object */
  tmp = (TMWTYPES_USHORT)(8 + length);
  tmwtarg_store16(&tmp, &pTxData->pMsgBuf[pTxData->msgLength]);
  pTxData->msgLength += 2;

  /* File Handle */
  tmwtarg_store32(&handle, &pTxData->pMsgBuf[pTxData->msgLength]);
  pTxData->msgLength += 4;

  /* Block */
  if(last) block |= 0x80000000L;
  tmwtarg_store32(&block, &pTxData->pMsgBuf[pTxData->msgLength]);
  pTxData->msgLength += 4;

  /* Copy Data, if it was not already copied in by caller */
  if(pData != TMWDEFS_NULL)
    memcpy(pTxData->pMsgBuf + pTxData->msgLength, pData, length);
  pTxData->msgLength = (TMWTYPES_USHORT)(pTxData->msgLength + length);

  /* Send request */
  if(!dnpchnl_sendFragment(pTxData))
  { 
    pTxData = TMWDEFS_NULL; 
  }

  return(pTxData);
}

/* function: _setupAuthentication
 * purpose: Allocate Tx Data structure and build an authentication request, 
 *   but do not send it.
 * arguments:
 * 
 * returns:
 *  pointer to txData if request was built
 *  TMWDEFS_NULL otherwise (file transfer context will be deallocated)
 */
static TMWSESN_TX_DATA * TMWDEFS_LOCAL _setupAuthentication(
  MDNPBRM_REQ_DESC    *pReqDesc,
  const TMWTYPES_CHAR *pUserName, 
  const TMWTYPES_CHAR *pPassword, 
  TMWTYPES_USHORT      nextRequestLength)
{
  TMWSESN_TX_DATA *pTxData;
  TMWTYPES_USHORT  messageLength;
  TMWTYPES_USHORT  usernameLength;
  TMWTYPES_USHORT  passwordLength = 0;

  usernameLength = (TMWTYPES_USHORT)strlen(pUserName);
  if(pPassword != TMWDEFS_NULL)
  {
    passwordLength = (TMWTYPES_USHORT)strlen(pPassword);
  }

  /* Size is 2 Appl Header + 6 Object Header + 12 Fixed Data + username + password */
  messageLength = (TMWTYPES_USHORT)(2 + 6 + 12 + usernameLength + passwordLength);
  if(nextRequestLength > messageLength)
  {
    messageLength = nextRequestLength;
  }

#if MDNPDATA_SUPPORT_OBJ120
  if(pReqDesc->authAggressiveMode)
    messageLength += DNPAUTH_AGGRESSIVE_SIZE;
#endif

  /* Allocate transmit data buffer */
  pTxData = dnpchnl_newTxData(
    pReqDesc->pChannel, pReqDesc->pSession, messageLength, 0);

  if(pTxData != TMWDEFS_NULL)
  { 
    /* Initialize Transmit Data Structure from Request Descriptor */
    if(_initTxData((DNPCHNL_TX_DATA *)pTxData, pReqDesc, "File Authentication Request"))
    {
      _initFileTxData((DNPCHNL_TX_DATA *)pTxData, pReqDesc);
      return(pTxData);
    }
  }

  _cleanupFileXfer((MDNPSESN*)pReqDesc->pSession, pTxData);
  return(TMWDEFS_NULL);
}

/* function: _fileAuthentication 
 * purpose: Build and send a file authentication Obj70V2 request.
 * arguments:
 *  pMDNPSession - pointer to master session
 *  pTxData - transmit data structure to use for building and sending request
 * returns:
 *  pointer to txData if request was built and sent to slave
 *  TMWDEFS_NULL otherwise (file transfer context will be deallocated)
 */
static TMWSESN_TX_DATA * TMWDEFS_LOCAL _fileAuthentication(
  MDNPSESN *pMDNPSession,
  TMWSESN_TX_DATA *pTxData,
  const TMWTYPES_CHAR *pUserName,
  const TMWTYPES_CHAR *pPassword)
{ 
  TMWTYPES_USHORT usernameLength = 0;
  TMWTYPES_USHORT passwordLength = 0;
  TMWTYPES_ULONG authKey;
  TMWTYPES_USHORT tmp;

  usernameLength = (TMWTYPES_USHORT)strlen(pUserName);
  if(pPassword != TMWDEFS_NULL)
  {
    passwordLength = (TMWTYPES_USHORT)strlen(pPassword);
  }

  /* Build request header */
  mdnpbrm_buildRequestHeader((DNPCHNL_TX_DATA *)pTxData, DNPDEFS_FC_AUTHENTICATE);
  
#if MDNPDATA_SUPPORT_OBJ120 
  {
    DNPCHNL_TX_DATA *pChnlTxData = (DNPCHNL_TX_DATA *)pTxData;
    if(pChnlTxData->authAggressiveMode)
    {
      if(mdnpauth_addAggrRequestStart((TMWSESN_TX_DATA *)pTxData, pChnlTxData->authUserNumber) == TMWDEFS_NULL)
      {
        _cleanupFileXfer(pMDNPSession, pTxData);
        return(TMWDEFS_NULL);
      }
    }
  }
#endif 

  /* Time Object Header */
  if(!mdnpbrm_addObjectHeader((DNPCHNL_TX_DATA *)pTxData, DNPDEFS_OBJ_70_FILE_IDENTIFIER, 2, 
    DNPDEFS_QUAL_16BIT_FREE_FORMAT, 0, 1))
  {
    _cleanupFileXfer(pMDNPSession, pTxData);
    return(TMWDEFS_NULL);
  }

  /* Size */
  tmp = (TMWTYPES_USHORT)(12 + usernameLength + passwordLength);
  tmwtarg_store16(&tmp, &pTxData->pMsgBuf[pTxData->msgLength]);
  pTxData->msgLength += 2;

  /* User Name Offset */
  tmp = 12;
  tmwtarg_store16(&tmp, &pTxData->pMsgBuf[pTxData->msgLength]);
  pTxData->msgLength += 2;

  /* User Name Size */
  tmwtarg_store16(&usernameLength, &pTxData->pMsgBuf[pTxData->msgLength]);
  pTxData->msgLength += 2;

  /* Password Offset */
  tmp = (TMWTYPES_USHORT)(12 + usernameLength);
  tmwtarg_store16(&tmp, &pTxData->pMsgBuf[pTxData->msgLength]);
  pTxData->msgLength += 2;

  /* Password Size */
  tmwtarg_store16(&passwordLength, &pTxData->pMsgBuf[pTxData->msgLength]);
  pTxData->msgLength += 2;

  /* Authentication Key */
  authKey = 0;
  tmwtarg_store32(&authKey, &pTxData->pMsgBuf[pTxData->msgLength]);
  pTxData->msgLength += 4;

  /* User Name */
  memcpy(&pTxData->pMsgBuf[pTxData->msgLength], pUserName, usernameLength);
  pTxData->msgLength = (TMWTYPES_USHORT)(pTxData->msgLength + usernameLength);

  /* Password */
  if(pPassword != TMWDEFS_NULL)
  {
    memcpy(&pTxData->pMsgBuf[pTxData->msgLength], pPassword, passwordLength);
    pTxData->msgLength = (TMWTYPES_USHORT)(pTxData->msgLength + passwordLength);
  }

  /* Send request */
  if(!dnpchnl_sendFragment((TMWSESN_TX_DATA *)pTxData))
  { 
    _cleanupFileXfer(pMDNPSession, pTxData);
    pTxData = TMWDEFS_NULL; 
  }

  return((TMWSESN_TX_DATA *)pTxData);
}

/* function: mdnpfile_fileAuthentication */
TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpfile_fileAuthentication(
  void *pReqD,
  const TMWTYPES_CHAR *pUserName,
  const TMWTYPES_CHAR *pPassword)
{
  TMWSESN_TX_DATA       *pTxData;
  MDNPFILE_XFER_CONTEXT *pFileXferContext;  
  MDNPBRM_REQ_DESC      *pReqDesc = (MDNPBRM_REQ_DESC *)pReqD;
  MDNPSESN              *pMDNPSession = (MDNPSESN *)pReqDesc->pSession;

  pFileXferContext = _allocateFileXferContext(pMDNPSession);
  if(pFileXferContext == TMWDEFS_NULL)
  {
    return(TMWDEFS_NULL);
  }

  if(pUserName == TMWDEFS_NULL)
  {
    /* user name is required. */
    _cleanupFileXfer(pMDNPSession, TMWDEFS_NULL);
    return(TMWDEFS_NULL);
  }
  
  pTxData = _setupAuthentication(pReqDesc, pUserName, pPassword, 0);
  if(pTxData == TMWDEFS_NULL)
  {
    /* File transfer context will be deallocated by _setupAuthentication */
    return(TMWDEFS_NULL);
  }

  return(_fileAuthentication(pMDNPSession, pTxData, pUserName, pPassword));    
}

/* function: mdnpfile_fileOpen */
TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpfile_fileOpen(
  void *pReqD,
  TMWTYPES_USHORT requestId,
  DNPDEFS_FILE_MODE mode,
  TMWTYPES_MS_SINCE_70 *pTimeOfCreation,
  TMWTYPES_USHORT permissions,
  TMWTYPES_ULONG authenticationKey,
  TMWTYPES_ULONG fileSize,
  TMWTYPES_USHORT maxBlockSize,
  const TMWTYPES_CHAR *pFilename)
{
  TMWTYPES_USHORT        messageLength;
  TMWSESN_TX_DATA       *pTxData;
  MDNPFILE_XFER_CONTEXT *pFileXferContext;
  MDNPBRM_REQ_DESC      *pReqDesc = (MDNPBRM_REQ_DESC *)pReqD;
  MDNPSESN              *pMDNPSession = (MDNPSESN *)pReqDesc->pSession;
  TMWTYPES_USHORT        filenameLength = (TMWTYPES_USHORT)strlen(pFilename);

  pFileXferContext = _allocateFileXferContext(pMDNPSession);
  if(pFileXferContext == TMWDEFS_NULL)
  {
    return(TMWDEFS_NULL);
  }
  
  /* Message length = 2 Appl Header + 6 Object Header + 26 Fixed Data + filename */
  messageLength = (TMWTYPES_USHORT)(2 + 6 + 26 + filenameLength);

#if MDNPDATA_SUPPORT_OBJ120
  if(pReqDesc->authAggressiveMode)
  {
    messageLength += DNPAUTH_AGGRESSIVE_SIZE;
  }
#endif

  /* Allocate transmit data buffer */
  pTxData = dnpchnl_newTxData(
    pReqDesc->pChannel, pReqDesc->pSession, messageLength, 0);

  if(pTxData == TMWDEFS_NULL)
  {
    _cleanupFileXfer(pMDNPSession, pTxData);
    return(TMWDEFS_NULL);
  }
  
  /* Initialize Transmit Data Structure from Request Descriptor */
  if(!_initTxData((DNPCHNL_TX_DATA *)pTxData, pReqDesc, "File Open Request"))
  {
    _cleanupFileXfer(pMDNPSession, pTxData);
    return(TMWDEFS_NULL);
  } 

  _initFileTxData((DNPCHNL_TX_DATA *)pTxData, pReqDesc);

  if(TMWDEFS_NULL == _fileOpen(pTxData, requestId, mode, pTimeOfCreation, permissions, authenticationKey, fileSize,
    maxBlockSize, (TMWTYPES_CHAR*)pFilename))
  {
    _cleanupFileXfer(pMDNPSession, pTxData);
    return(TMWDEFS_NULL);
  }

  return(pTxData);
}

/* function: mdnpfile_fileClose */
TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpfile_fileClose(
  void *pReqD,
  TMWTYPES_USHORT requestId,
  TMWTYPES_ULONG handle)
{
  TMWTYPES_USHORT        messageLength;
  TMWSESN_TX_DATA       *pTxData;
  MDNPFILE_XFER_CONTEXT *pFileXferContext;
  MDNPBRM_REQ_DESC      *pReqDesc = (MDNPBRM_REQ_DESC *)pReqD;
  MDNPSESN              *pMDNPSession = (MDNPSESN *)pReqDesc->pSession;

  pFileXferContext = _allocateFileXferContext(pMDNPSession);
  if(pFileXferContext == TMWDEFS_NULL)
  {
    return(TMWDEFS_NULL);
  }
 
  /* Message length = 2 Appl Header + 6 Object Header + 13 Fixed Data */
  messageLength = (TMWTYPES_USHORT)(2 + 6 + 13);

#if MDNPDATA_SUPPORT_OBJ120
  if(pReqDesc->authAggressiveMode)
  {
    messageLength += DNPAUTH_AGGRESSIVE_SIZE;;
  }
#endif

  /* Allocate transmit data buffer */
  pTxData = dnpchnl_newTxData(
    pReqDesc->pChannel, pReqDesc->pSession, messageLength, 0);

  if(pTxData == TMWDEFS_NULL)
  {
    _cleanupFileXfer(pMDNPSession, TMWDEFS_NULL);
    return(TMWDEFS_NULL);
  }

  /* Initialize Transmit Data Structure from Request Descriptor */
  if(!_initTxData((DNPCHNL_TX_DATA *)pTxData, pReqDesc, "File Close Request"))
  {
    _cleanupFileXfer(pMDNPSession, pTxData);
    return(TMWDEFS_NULL);
  }

  _initFileTxData((DNPCHNL_TX_DATA *)pTxData, pReqDesc);

  if(TMWDEFS_NULL == _fileClose(pTxData, requestId,handle))
  {
    _cleanupFileXfer(pMDNPSession, pTxData);
    return(TMWDEFS_NULL);
  }

  return(pTxData);
}

/* function: mdnpfile_fileDelete */
TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpfile_fileDelete(
  void *pReqD,
  TMWTYPES_USHORT requestId,
  TMWTYPES_ULONG authenticationKey,
  const TMWTYPES_CHAR *pUserName,
  const TMWTYPES_CHAR *pPassword,
  const TMWTYPES_CHAR *pFilename)
{
  TMWTYPES_USHORT        deleteMessageSize;
  TMWSESN_TX_DATA       *pTxData;
  MDNPFILE_XFER_CONTEXT *pFileXferContext;
  MDNPBRM_REQ_DESC      *pReqDesc = (MDNPBRM_REQ_DESC *)pReqD;
  MDNPSESN              *pMDNPSession = (MDNPSESN *)pReqDesc->pSession;
  TMWTYPES_USHORT        filenameLength = (TMWTYPES_USHORT)strlen(pFilename);

  pFileXferContext = _allocateFileXferContext(pMDNPSession);
  if(pFileXferContext == TMWDEFS_NULL)
  {
    return(TMWDEFS_NULL);
  }

  deleteMessageSize = 2 + 6 + 26 + filenameLength;

#if MDNPDATA_SUPPORT_OBJ120
  if(pReqDesc->authAggressiveMode)
  {
    deleteMessageSize += DNPAUTH_AGGRESSIVE_SIZE;
  }
#endif

  /* If caller specified user name, perform file authentication 
   * and then send the delete request when the key is received back from the slave
   */
  if(pUserName != TMWDEFS_NULL)
  {
    /* Save the filename for after the authorization completes */
    if(filenameLength > DNPCNFG_MAX_FILENAME)
    {
      _cleanupFileXfer(pMDNPSession, TMWDEFS_NULL);
      return(TMWDEFS_NULL);
    }
    memcpy(pMDNPSession->pFileXferContext->remoteFileName, pFilename, filenameLength);
    pMDNPSession->pFileXferContext->remoteFileName[filenameLength] = '\0';

    pTxData = _setupAuthentication(pReqDesc, pUserName, pPassword, deleteMessageSize);
    if(pTxData == TMWDEFS_NULL)
    {
      /* File transfer context will be deallocated by _setupAuthentication */
      return(TMWDEFS_NULL);
    }

    pMDNPSession->fileXferType = DNPDEFS_FILE_TYPE_SIMPLE;

    /* After authorization, send file delete */
    pMDNPSession->pFileXferContext->nextOperation = MDNPFILE_XFER_STATE_DELETE;
    return(_fileAuthentication(pMDNPSession, pTxData, pUserName, pPassword));
  }

  /* If user name was not specified, file authentication request does not need to be sent */

  /* Allocate transmit data buffer */
  pTxData = dnpchnl_newTxData(
    pReqDesc->pChannel, pReqDesc->pSession, deleteMessageSize, 0);

  if(pTxData == TMWDEFS_NULL)
  {
    _cleanupFileXfer(pMDNPSession, TMWDEFS_NULL);
    return(TMWDEFS_NULL);
  }

  /* Initialize Transmit Data Structure from Request Descriptor */
  if(!_initTxData((DNPCHNL_TX_DATA *)pTxData, pReqDesc, "File Delete Request"))
  {
    _cleanupFileXfer(pMDNPSession, pTxData);
    return(TMWDEFS_NULL);
  }

  _initFileTxData((DNPCHNL_TX_DATA *)pTxData, pReqDesc);

  if(TMWDEFS_NULL == _fileDelete(pTxData, requestId, authenticationKey, pFilename))
  {
    _cleanupFileXfer(pMDNPSession, pTxData);
    return(TMWDEFS_NULL);
  }
  return(pTxData);
}

/* function: mdnpfile_fileAbort */
TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpfile_fileAbort(
  void           *pReqD,
  TMWTYPES_USHORT requestId,
  TMWTYPES_ULONG  handle)
{
  TMWTYPES_BOOL          abortSCLManagedXfer;
  TMWTYPES_USHORT        messageLength;
  TMWTYPES_ULONG         tmpLong;
  TMWTYPES_USHORT        tmp;
  TMWSESN_TX_DATA       *pTxData;
  MDNPFILE_XFER_CONTEXT *pFileXferContext;
  MDNPBRM_REQ_DESC      *pReqDesc = (MDNPBRM_REQ_DESC *)pReqD;
  MDNPSESN              *pMDNPSession = (MDNPSESN *)pReqDesc->pSession;

  if(pMDNPSession->pFileXferContext != TMWDEFS_NULL)
  {
    /* If an SCL managed file transfer is in progress, mark it to be aborted */
    pMDNPSession->pFileXferContext->nextOperation = MDNPFILE_XFER_STATE_ABORT;
    abortSCLManagedXfer = TMWDEFS_TRUE;
  }
  else
  {
    pFileXferContext = _allocateFileXferContext(pMDNPSession);
    if(pFileXferContext == TMWDEFS_NULL)
    {
      return(TMWDEFS_NULL);
    }
    abortSCLManagedXfer = TMWDEFS_FALSE;
  }

  /* Message length = 2 Appl Header + 6 Object Header + 13 Fixed Data */
  messageLength = (TMWTYPES_USHORT)(2 + 6 + 13);

#if MDNPDATA_SUPPORT_OBJ120
  if(pReqDesc->authAggressiveMode)
    messageLength += DNPAUTH_AGGRESSIVE_SIZE;
#endif

  /* Allocate transmit data buffer */
  pTxData = dnpchnl_newTxData(
    pReqDesc->pChannel, pReqDesc->pSession, messageLength, 0);

  if(pTxData == TMWDEFS_NULL)
  {
    return(TMWDEFS_NULL);
  }

  /* Initialize Transmit Data Structure from Request Descriptor */
  if(!_initTxData((DNPCHNL_TX_DATA *)pTxData, pReqDesc, "File Abort Request"))
  {
    _cleanupFileXfer(pMDNPSession, pTxData);
    return(TMWDEFS_NULL);
  }

  if(!abortSCLManagedXfer)
    _initFileTxData((DNPCHNL_TX_DATA *)pTxData, pReqDesc);

  /* Build request header */
  mdnpbrm_buildRequestHeader((DNPCHNL_TX_DATA *)pTxData, DNPDEFS_FC_ABORT);

#if MDNPDATA_SUPPORT_OBJ120
  {
    DNPCHNL_TX_DATA *pChnlTxData = (DNPCHNL_TX_DATA *)pTxData;
    if(pChnlTxData->authAggressiveMode)
    {
      if(mdnpauth_addAggrRequestStart(pTxData, pChnlTxData->authUserNumber) == TMWDEFS_NULL)
      {
        _cleanupFileXfer(pMDNPSession, pTxData);
        return(TMWDEFS_NULL);
      }
    }
  }
#endif 

  /* Time Object Header */
  if(!mdnpbrm_addObjectHeader((DNPCHNL_TX_DATA *)pTxData, DNPDEFS_OBJ_70_FILE_IDENTIFIER, 4, DNPDEFS_QUAL_16BIT_FREE_FORMAT, 0, 1))
  {
    _cleanupFileXfer(pMDNPSession, pTxData);
    return(TMWDEFS_NULL);
  }

  /* Size */
  tmp = 13;
  tmwtarg_store16(&tmp, &pTxData->pMsgBuf[pTxData->msgLength]);
  pTxData->msgLength += 2;

  /* File Handle */
  tmwtarg_store32(&handle, &pTxData->pMsgBuf[pTxData->msgLength]);
  pTxData->msgLength += 4;

  /* File Size */
  tmpLong = 0;
  tmwtarg_store32(&tmpLong, &pTxData->pMsgBuf[pTxData->msgLength]);
  pTxData->msgLength += 4;

  /* Maximum Block Size */
  tmp = 0;
  tmwtarg_store16(&tmp, &pTxData->pMsgBuf[pTxData->msgLength]);
  pTxData->msgLength += 2;

  /* Request ID */
  tmwtarg_store16(&requestId, &pTxData->pMsgBuf[pTxData->msgLength]);
  pTxData->msgLength += 2;

  /* Status */
  pTxData->pMsgBuf[pTxData->msgLength++] = 0;

  /* Send request */
  if(!dnpchnl_sendFragment(pTxData))
  { 
    _cleanupFileXfer(pMDNPSession, pTxData);
    pTxData = TMWDEFS_NULL; 
  }

  return(pTxData);
}

/* function: mdnpfile_fileRead */
TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpfile_fileRead(
  void *pReqD,
  TMWTYPES_ULONG block,
  TMWTYPES_ULONG handle)
{
  TMWSESN_TX_DATA       *pTxData;
  MDNPFILE_XFER_CONTEXT *pFileXferContext;
  MDNPBRM_REQ_DESC      *pReqDesc = (MDNPBRM_REQ_DESC *)pReqD;
  MDNPSESN              *pMDNPSession = (MDNPSESN *)pReqDesc->pSession;

  pFileXferContext = _allocateFileXferContext(pMDNPSession);
  if(pFileXferContext == TMWDEFS_NULL)
  {
    return(TMWDEFS_NULL);
  }

  /* Size is 2 Appl Header + 6 Object Header + 8 Object Data */
  pTxData = dnpchnl_newTxData(
    pReqDesc->pChannel, pReqDesc->pSession, 16, 0);

  if(pTxData == TMWDEFS_NULL)
  {
    _cleanupFileXfer(pMDNPSession, TMWDEFS_NULL);
    return(TMWDEFS_NULL);
  }

  /* Initialize Transmit Data Structure from Request Descriptor */
  if(!_initTxData((DNPCHNL_TX_DATA *)pTxData, pReqDesc, "Read File Request"))
  {
    _cleanupFileXfer(pMDNPSession, pTxData);
    return(TMWDEFS_NULL);
  }

  _initFileTxData((DNPCHNL_TX_DATA *)pTxData, pReqDesc);
  
  pFileXferContext->remoteFileHandle = handle;
  if(TMWDEFS_NULL == _fileRead(pTxData, block, handle))
  {
    _cleanupFileXfer(pMDNPSession, pTxData);
    return(TMWDEFS_NULL);
  }

  return(pTxData);
}

/* function: mdnpfile_fileWrite */
TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpfile_fileWrite(
  void *pReqD,
  TMWTYPES_ULONG block,
  TMWTYPES_BOOL last,
  TMWTYPES_ULONG handle,
  const TMWTYPES_UCHAR *pData,
  TMWTYPES_USHORT length)
{
  TMWSESN_TX_DATA       *pTxData;
  MDNPFILE_XFER_CONTEXT *pFileXferContext;
  MDNPBRM_REQ_DESC      *pReqDesc;
  MDNPSESN              *pMDNPSession;
  TMWTYPES_USHORT        messageLength;
  
  pReqDesc = (MDNPBRM_REQ_DESC *)pReqD;
  pMDNPSession = (MDNPSESN *)pReqDesc->pSession;

  pFileXferContext = _allocateFileXferContext(pMDNPSession);
  if(pFileXferContext == TMWDEFS_NULL)
  {
    return(TMWDEFS_NULL);
  }

  /* Size is 2 Appl Header + 6 Object Header + 8 Object Data + length */
  messageLength = (TMWTYPES_USHORT)(16 + length);

#if MDNPDATA_SUPPORT_OBJ120
  if(pReqDesc->authAggressiveMode)
    messageLength += DNPAUTH_AGGRESSIVE_SIZE;
#endif

  pTxData = dnpchnl_newTxData(
    pReqDesc->pChannel, pReqDesc->pSession, messageLength, 0);

  if(pTxData == TMWDEFS_NULL)
  {
    _cleanupFileXfer(pMDNPSession, pTxData);
    return(TMWDEFS_NULL);
  }

  /* Initialize Transmit Data Structure from Request Descriptor */
  if(!_initTxData((DNPCHNL_TX_DATA *)pTxData, pReqDesc, "Write File Request"))
  {
    _cleanupFileXfer(pMDNPSession, pTxData);
    return(TMWDEFS_NULL);
  }
 
  _initFileTxData((DNPCHNL_TX_DATA *)pTxData, pReqDesc);

  pFileXferContext->remoteFileHandle = handle;
  if(TMWDEFS_NULL == _fileWrite(pTxData, block, last, handle, pData, length))
  {
    _cleanupFileXfer(pMDNPSession, pTxData);
    return(TMWDEFS_NULL);
  }

  return(pTxData);
}

/* function: mdnpfile_fileInfo */
TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpfile_fileInfo(
  void *pReqD,
  TMWTYPES_USHORT requestId,
  const TMWTYPES_CHAR *pFilename)
{
  TMWTYPES_USHORT        messageLength;
  TMWTYPES_ULONG         tmpLong;
  TMWTYPES_USHORT        tmp;
  TMWSESN_TX_DATA       *pTxData;
  MDNPFILE_XFER_CONTEXT *pFileXferContext;
  MDNPBRM_REQ_DESC      *pReqDesc = (MDNPBRM_REQ_DESC *)pReqD;
  MDNPSESN              *pMDNPSession = (MDNPSESN *)pReqDesc->pSession;
  TMWTYPES_USHORT        filenameLength = (TMWTYPES_USHORT)strlen(pFilename);

  pFileXferContext = _allocateFileXferContext(pMDNPSession);
  if(pFileXferContext == TMWDEFS_NULL)
  {
    return(TMWDEFS_NULL);
  }

  /* Message length = 2 Appl Header + 6 Object Header + 26 Fixed Data + filename */
  messageLength = (TMWTYPES_USHORT)(2 + 6 + 26 + filenameLength);

  /* Allocate transmit data buffer */
  pTxData = dnpchnl_newTxData(
    pReqDesc->pChannel, pReqDesc->pSession, messageLength, 0);

  if(pTxData == TMWDEFS_NULL)
  {
    _cleanupFileXfer(pMDNPSession, pTxData);
    return(TMWDEFS_NULL);
  }

  /* Initialize Transmit Data Structure from Request Descriptor */
  if(!_initTxData((DNPCHNL_TX_DATA *)pTxData, pReqDesc, "File Info Request"))
  {
    _cleanupFileXfer(pMDNPSession, pTxData);
    return(TMWDEFS_NULL);
  }

  _initFileTxData((DNPCHNL_TX_DATA *)pTxData, pReqDesc);

  /* Build request header */
  mdnpbrm_buildRequestHeader((DNPCHNL_TX_DATA *)pTxData, DNPDEFS_FC_GET_FILE_INFO);

  /* Time Object Header */
  if(!mdnpbrm_addObjectHeader((DNPCHNL_TX_DATA *)pTxData, DNPDEFS_OBJ_70_FILE_IDENTIFIER, 7, DNPDEFS_QUAL_16BIT_FREE_FORMAT, 0, 1))
  {
    _cleanupFileXfer(pMDNPSession, pTxData);
    return(TMWDEFS_NULL);
  }

  /* Size */
  tmp = (TMWTYPES_USHORT)(20 + filenameLength);
  tmwtarg_store16(&tmp, &pTxData->pMsgBuf[pTxData->msgLength]);
  pTxData->msgLength += 2;

  /* Filename Offset */
  tmp = 20;
  tmwtarg_store16(&tmp, &pTxData->pMsgBuf[pTxData->msgLength]);
  pTxData->msgLength += 2;

  /* Filename Size */
  tmwtarg_store16(&filenameLength, &pTxData->pMsgBuf[pTxData->msgLength]);
  pTxData->msgLength += 2;

  /* File Type */
  tmp = 0;
  tmwtarg_store16(&tmp, &pTxData->pMsgBuf[pTxData->msgLength]);
  pTxData->msgLength += 2;

  /* File Size */
  tmpLong = 0;
  tmwtarg_store32(&tmpLong, &pTxData->pMsgBuf[pTxData->msgLength]);
  pTxData->msgLength += 4;

  /* Time Of Creation */
  tmwtarg_store32(&tmpLong, &pTxData->pMsgBuf[pTxData->msgLength]);
  pTxData->msgLength += 4;

  tmp = 0;
  tmwtarg_store16(&tmp, &pTxData->pMsgBuf[pTxData->msgLength]);
  pTxData->msgLength += 2;

  /* Permissions */
  tmwtarg_store16(&tmp, &pTxData->pMsgBuf[pTxData->msgLength]);
  pTxData->msgLength += 2;

  /* Request ID */
  tmwtarg_store16(&requestId, &pTxData->pMsgBuf[pTxData->msgLength]);
  pTxData->msgLength += 2;

  /* Filename */
  memcpy(&pTxData->pMsgBuf[pTxData->msgLength], pFilename, filenameLength);
  pTxData->msgLength = (TMWTYPES_USHORT)(pTxData->msgLength + filenameLength);

  /* Send request */
  if(!dnpchnl_sendFragment(pTxData))
  { 
    _cleanupFileXfer(pMDNPSession, pTxData);
    pTxData = TMWDEFS_NULL; 
  }

  return(pTxData);
}

/* function: mdnpfile_copyLocalFileToRemote  */
TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpfile_copyLocalFileToRemote(
  void *pReqD,
  const TMWTYPES_CHAR *pLocalFileName,
  const TMWTYPES_CHAR *pRemoteFileName,
  const TMWTYPES_CHAR *pUserName,
  const TMWTYPES_CHAR *pPassword,
  const TMWTYPES_USHORT permissions)
{
  TMWTYPES_USHORT        messageSize;
  TMWSESN_TX_DATA       *pTxData;
  MDNPFILE_XFER_CONTEXT *pFileXferContext;
  MDNPBRM_REQ_DESC      *pReqDesc;
  MDNPSESN              *pMDNPSession;
  DNPCHNL               *pDNPChannel;
  TMWDTIME               dateTime;

  pReqDesc = (MDNPBRM_REQ_DESC *)pReqD;
  pMDNPSession = (MDNPSESN *)pReqDesc->pSession;
  pDNPChannel = (DNPCHNL *)pReqDesc->pSession->pChannel;

  pFileXferContext = _allocateFileXferContext(pMDNPSession);
  if(pFileXferContext == TMWDEFS_NULL)
  {
    return(TMWDEFS_NULL);
  }

  if(!mdnpdata_openLocalFile(pMDNPSession->pDbHandle, pLocalFileName, DNPDEFS_FILE_MODE_READ))
  {
    _cleanupFileXfer(pMDNPSession, TMWDEFS_NULL);
    return(TMWDEFS_NULL);
  } 

  /* If we can't get size and time, just send 0xffffffff and 0 ms since epoch */
  if(mdnpdata_getLocalFileInfo(pMDNPSession->pDbHandle, pLocalFileName, &pFileXferContext->fileSize, &dateTime))
  { 
    dnpdtime_dateTimeToMSSince70(&pFileXferContext->timeOfCreation, &dateTime); 
  } 
  else
  {
    pFileXferContext->fileSize = 0xffffffff;
    pFileXferContext->timeOfCreation.leastSignificant = 0;
    pFileXferContext->timeOfCreation.mostSignificant = 0;
  }
 
  pFileXferContext->localFileOpen = TMWDEFS_TRUE;
  pMDNPSession->fileXferType = DNPDEFS_FILE_TYPE_SIMPLE; 

  messageSize = pMDNPSession->maxFileBlockSize+16;
#if MDNPDATA_SUPPORT_OBJ120
  if(pReqDesc->authAggressiveMode)
    messageSize += DNPAUTH_AGGRESSIVE_SIZE;
#endif 

  if(messageSize > pDNPChannel->txFragmentSize)
    messageSize = pDNPChannel->txFragmentSize;

  pFileXferContext->maxBlockSize = messageSize-16;
#if MDNPDATA_SUPPORT_OBJ120
  if (pReqDesc->authAggressiveMode)
  {
    pFileXferContext->maxBlockSize -= DNPAUTH_AGGRESSIVE_SIZE;
    pFileXferContext->useAggressiveMode = TMWDEFS_TRUE;
  }
#endif 

  pFileXferContext->permissions = permissions;
  
  /* If there is a user name, perform authorization, otherwise just send open request */
  if(pUserName != TMWDEFS_NULL)
  {
    /* Save the filename for after the authorization completes */
    TMWTYPES_ULONG nameLen = (TMWTYPES_ULONG)strlen(pRemoteFileName);
    if(nameLen > DNPCNFG_MAX_FILENAME)
    {
      _cleanupFileXfer(pMDNPSession, TMWDEFS_NULL);
      return(TMWDEFS_NULL);
    }

    memcpy(pFileXferContext->remoteFileName, pRemoteFileName, nameLen);
    pFileXferContext->remoteFileName[nameLen] = '\0';

    pTxData = _setupAuthentication(pReqDesc, pUserName, pPassword, messageSize);
    if(pTxData == TMWDEFS_NULL)
    {
      /* File transfer context will be deallocated by _setupAuthentication */
      return(TMWDEFS_NULL);
    }
    
    /* After authorization, send file open */
    pFileXferContext->nextOperation = MDNPFILE_XFER_STATE_OPEN_WRITE;

    return(_fileAuthentication(pMDNPSession, pTxData, pUserName, pPassword));   
  }
  else
  {   
    /* Allocate transmit data buffer */
    pTxData = dnpchnl_newTxData(
      pReqDesc->pChannel, pReqDesc->pSession, messageSize, 0);

    if(pTxData == TMWDEFS_NULL)
    {
      _cleanupFileXfer(pMDNPSession, TMWDEFS_NULL);
      return(TMWDEFS_NULL);
    }

    /* Initialize Transmit Data Structure from Request Descriptor */
    if(!_initTxData((DNPCHNL_TX_DATA *)pTxData, pReqDesc, "File Open Request"))
    {
      _cleanupFileXfer(pMDNPSession, pTxData);
      return(TMWDEFS_NULL);
    }

    _initFileTxData((DNPCHNL_TX_DATA *)pTxData, pReqDesc);

    /* after open, send write request */
    pFileXferContext->nextOperation = MDNPFILE_XFER_STATE_WRITE;
    if(TMWDEFS_NULL == _fileOpen(pTxData, _mdnpRequestId++, 
      DNPDEFS_FILE_MODE_WRITE, &pFileXferContext->timeOfCreation, permissions, 0, pFileXferContext->fileSize, 
      pFileXferContext->maxBlockSize, (TMWTYPES_CHAR*)pRemoteFileName))
    {
      _cleanupFileXfer(pMDNPSession, pTxData);
      return(TMWDEFS_NULL);
    }

    return(pTxData);
  }
}

/* function: mdnpfile_copyRemoteFileToLocal  */
TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpfile_copyRemoteFileToLocal(
  void *pReqD,
  const TMWTYPES_CHAR *pLocalFileName,
  const TMWTYPES_CHAR *pRemoteFileName,
  const TMWTYPES_CHAR *pUserName,
  const TMWTYPES_CHAR *pPassword)
{
  TMWTYPES_USHORT        size;
  TMWTYPES_USHORT        maxBlockSize;
  TMWTYPES_ULONG         nameLen;
  TMWSESN_TX_DATA       *pTxData;
  MDNPFILE_XFER_CONTEXT *pFileXferContext;
  MDNPBRM_REQ_DESC      *pReqDesc = (MDNPBRM_REQ_DESC *)pReqD;
  MDNPSESN              *pMDNPSession = (MDNPSESN *)pReqDesc->pSession;
  DNPCHNL               *pDNPChannel = (DNPCHNL *)pReqDesc->pSession->pChannel;

  pFileXferContext = _allocateFileXferContext(pMDNPSession);
  if(pFileXferContext == TMWDEFS_NULL)
  {
    return(TMWDEFS_NULL);
  }

  if(!mdnpdata_openLocalFile(pMDNPSession->pDbHandle, pLocalFileName, DNPDEFS_FILE_MODE_WRITE))
  {
    _cleanupFileXfer(pMDNPSession, TMWDEFS_NULL);
    return(TMWDEFS_NULL);
  }

  pFileXferContext->localFileOpen = TMWDEFS_TRUE;
  pMDNPSession->fileXferType = DNPDEFS_FILE_TYPE_SIMPLE;

  maxBlockSize = pMDNPSession->maxFileBlockSize;
  if(maxBlockSize > (pDNPChannel->rxFragmentSize-18))
    maxBlockSize = pDNPChannel->rxFragmentSize - 18;
   
  pFileXferContext->maxBlockSize = maxBlockSize; 
 
  /* Message length = length needed for open or authentication request
   *  2 Appl Header + 6 Object Header + 26 Fixed Data + filename 
   */
  nameLen = (TMWTYPES_ULONG)strlen(pRemoteFileName);
  size = (TMWTYPES_USHORT)(nameLen + 34);

  /* If there is a user name, perform authorization, otherwise just send open request */
  if(pUserName != TMWDEFS_NULL)
  {
    /* Save the filename for after the authorization completes */
    if(nameLen > DNPCNFG_MAX_FILENAME)
    {
      _cleanupFileXfer(pMDNPSession, TMWDEFS_NULL);
      return(TMWDEFS_NULL);
    }

    memcpy(pFileXferContext->remoteFileName, pRemoteFileName, nameLen);
    pFileXferContext->remoteFileName[nameLen] = '\0';

    pTxData = _setupAuthentication(pReqDesc, pUserName, pPassword, size);
    if(pTxData == TMWDEFS_NULL)
    {
      /* File transfer context will be deallocated by _setupAuthentication */
      return(TMWDEFS_NULL);
    }

    /* After authorization, send file open */
    pFileXferContext->nextOperation = MDNPFILE_XFER_STATE_OPEN_READ;
    return(_fileAuthentication(pMDNPSession, pTxData, pUserName, pPassword));
  }
  else
  {
    TMWTYPES_MS_SINCE_70 timeOfCreation;

#if MDNPDATA_SUPPORT_OBJ120
    if (pReqDesc->authAggressiveMode)
    {
      size += DNPAUTH_AGGRESSIVE_SIZE;
    }
#endif

    /* Allocate transmit data buffer */
    pTxData = dnpchnl_newTxData(
      pReqDesc->pChannel, pReqDesc->pSession, size, 0);

    if(pTxData == TMWDEFS_NULL)
    {
      _cleanupFileXfer(pMDNPSession, TMWDEFS_NULL);
      return(TMWDEFS_NULL);
    }

    /* Initialize Transmit Data Structure from Request Descriptor */
    if(!_initTxData((DNPCHNL_TX_DATA *)pTxData, pReqDesc, "File Open Request"))
    {
      _cleanupFileXfer(pMDNPSession, pTxData);
      return(TMWDEFS_NULL);
    }
  
    _initFileTxData((DNPCHNL_TX_DATA *)pTxData, pReqDesc);
    
    /* after open, send read request */
    pFileXferContext->nextOperation = MDNPFILE_XFER_STATE_READ;    
         
    timeOfCreation.leastSignificant = 0;
    timeOfCreation.mostSignificant = 0;

    if(TMWDEFS_NULL == _fileOpen(pTxData, _mdnpRequestId++, 
      DNPDEFS_FILE_MODE_READ, &timeOfCreation, 0, 0, 0, maxBlockSize, (TMWTYPES_CHAR*)pRemoteFileName))
    {
      _cleanupFileXfer(pMDNPSession, pTxData);
      return(TMWDEFS_NULL);
    }

    return(pTxData);
  }
}

/* function: mdnpfile_readRemoteDirectory  */
TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpfile_readRemoteDirectory(
  void *pReqD,
  const TMWTYPES_CHAR *pRemoteDirName,
  const TMWTYPES_CHAR *pUserName,
  const TMWTYPES_CHAR *pPassword)
{
  TMWTYPES_USHORT        size;
  TMWTYPES_ULONG         nameLen;
  TMWSESN_TX_DATA       *pTxData;
  MDNPFILE_XFER_CONTEXT *pFileXferContext;
  MDNPBRM_REQ_DESC      *pReqDesc = (MDNPBRM_REQ_DESC *)pReqD;
  MDNPSESN              *pMDNPSession = (MDNPSESN *)pReqDesc->pSession;

  pFileXferContext = _allocateFileXferContext(pMDNPSession);
  if(pFileXferContext == TMWDEFS_NULL)
  {
    return(TMWDEFS_NULL);
  }
  
  pMDNPSession->fileXferType = DNPDEFS_FILE_TYPE_DIRECTORY;

  /* Message length = length needed for open or authentication request
   *  2 Appl Header + 6 Object Header + 26 Fixed Data + filename 
   */
  nameLen = (TMWTYPES_ULONG)strlen(pRemoteDirName);
  size = (TMWTYPES_USHORT)(nameLen + 34);

  /* If there is a user name, perform authorization, otherwise just send open request */
  if(pUserName != TMWDEFS_NULL)
  {
    /* Save the filename for after the authorization completes */
    if(nameLen > DNPCNFG_MAX_FILENAME)
    {
      _cleanupFileXfer(pMDNPSession, TMWDEFS_NULL);
      return(TMWDEFS_NULL);
    }

    memcpy(pFileXferContext->remoteFileName, pRemoteDirName, nameLen);
    pFileXferContext->remoteFileName[nameLen] = '\0';

    pTxData = _setupAuthentication(pReqDesc, pUserName, pPassword, size);
    if(pTxData == TMWDEFS_NULL)
    {
      /* File transfer context will be deallocated by _setupAuthentication */
      return(TMWDEFS_NULL);
    }

    /* After authorization, send file open */
    pFileXferContext->nextOperation = MDNPFILE_XFER_STATE_OPEN_READ;
    return(_fileAuthentication(pMDNPSession, pTxData, pUserName, pPassword));    
  }
  else
  {
    DNPCHNL *pDNPChannel;
    TMWTYPES_MS_SINCE_70 timeOfCreation;
    pDNPChannel = (DNPCHNL *)pReqDesc->pSession->pChannel;

#if MDNPDATA_SUPPORT_OBJ120
    if (pReqDesc->authAggressiveMode)
    {
      size += DNPAUTH_AGGRESSIVE_SIZE;
    }
#endif

    /* Allocate transmit data buffer */
    pTxData = dnpchnl_newTxData(
      pReqDesc->pChannel, pReqDesc->pSession, size, 0);

    if(pTxData == TMWDEFS_NULL)
    {
      _cleanupFileXfer(pMDNPSession, TMWDEFS_NULL);
      return(TMWDEFS_NULL);
    }

    /* Initialize Transmit Data Structure from Request Descriptor */
    if(!_initTxData((DNPCHNL_TX_DATA *)pTxData, pReqDesc, "File Open Request"))
    {
      _cleanupFileXfer(pMDNPSession, pTxData);
      return(TMWDEFS_NULL);
    }
    
    _initFileTxData((DNPCHNL_TX_DATA *)pTxData, pReqDesc);

    /* after open, send read request */
    pFileXferContext->nextOperation = MDNPFILE_XFER_STATE_READ;

    size = pMDNPSession->maxFileBlockSize;
    if(size > (pDNPChannel->rxFragmentSize-18))
      size = pDNPChannel->rxFragmentSize - 18;

    timeOfCreation.leastSignificant = 0;
    timeOfCreation.mostSignificant = 0;

    if(TMWDEFS_NULL == _fileOpen(pTxData, _mdnpRequestId++, 
      DNPDEFS_FILE_MODE_READ, &timeOfCreation, 0, 0, 0, size, (TMWTYPES_CHAR*)pRemoteDirName))
    {
      _cleanupFileXfer(pMDNPSession, pTxData);
      return(TMWDEFS_NULL);
    }

    return(pTxData);
  }
}

/* function: mdnpfile_fileXferCB */
void TMWDEFS_GLOBAL mdnpfile_fileXferCB(
  TMWSESN               *pSession,
  DNPCHNL_RESPONSE_INFO *pResponse)
{ 
  MDNPFILE_XFER_CONTEXT *pFileXferContext;
  MDNPFILE_XFER_STATE    nextOperation;
  DNPCHNL_TX_DATA       *pDnpChnlTxData;
  DNPCHNL_RESP_STATUS    status;
  TMWTYPES_MILLISECONDS  timeSent;
  TMWTYPES_BOOL          last         = TMWDEFS_FALSE;
  TMWSESN_TX_DATA       *pTxData      = TMWDEFS_NULL;
  MDNPSESN              *pMDNPSession = (MDNPSESN *)pSession;

  /* if no file transfer context, there is nothing to do */
  if(pMDNPSession->pFileXferContext == TMWDEFS_NULL)
  {
    return;
  }

  pFileXferContext = pMDNPSession->pFileXferContext;
  nextOperation    = pFileXferContext->nextOperation;

  /* Get pointer to original request, whether this is an immediate
   * response or an event response that came later
   */
  pDnpChnlTxData   = pFileXferContext->pTxData;
  timeSent         = pDnpChnlTxData->tmw.timeSent;

  /* Determine if response was a failure, or timed out etc */
  status = pFileXferContext->status;
  if(pResponse->status > DNPCHNL_RESP_STATUS_INTERMEDIATE)
  {
    status = pResponse->status;
  } 
  if(status > DNPCHNL_RESP_STATUS_INTERMEDIATE)
  {
    nextOperation = MDNPFILE_XFER_STATE_FAILED;
  }

  /* This is a delayed response either due to an event read or
   * an unsolicited response 
   */
  if(pDnpChnlTxData != (DNPCHNL_TX_DATA *)pResponse->pTxData)
  {
    /* Since this request was not removed from the request queue 
     * by mdnpsesn.c _processResponse, remove it now. We may 
     * reuse it below to send another request.
     */
    tmwdlist_removeEntry(&pDnpChnlTxData->tmw.pChannel->messageQueue, (TMWDLIST_MEMBER *)pDnpChnlTxData);

    if (dnputil_getCurrentMessage(pSession) == (TMWSESN_TX_DATA *)pDnpChnlTxData)
    {
      dnputil_setCurrentMessage(pSession, TMWDEFS_NULL);
      tmwtimer_cancel(&pDnpChnlTxData->tmw.pChannel->incrementalTimer);
    }
  }

  
#if MDNPDATA_SUPPORT_OBJ120  
    mdnpauth_checkForQueuedAuthEvent(pSession);
#endif

  switch(nextOperation)
  {
    /* mdnpbrm_fileAbort was called during SCL managed file transfer */
  case MDNPFILE_XFER_STATE_ABORT:
    status = DNPCHNL_RESP_STATUS_FAILURE;
    last = TMWDEFS_TRUE;
    break;
  case MDNPFILE_XFER_STATE_RETURN: 
    last = TMWDEFS_TRUE;
    break;
  case MDNPFILE_XFER_STATE_OPEN_READ:
  { 
    TMWTYPES_ULONG authenticationKey;
    TMWTYPES_MS_SINCE_70 timeOfCreation;

    authenticationKey = mdnpdata_getFileAuthKey(pMDNPSession->pDbHandle);

    _reinitTxData(pDnpChnlTxData, "Open File Request");

    timeOfCreation.leastSignificant = 0;
    timeOfCreation.mostSignificant = 0;

    pTxData = _fileOpen((TMWSESN_TX_DATA *)pDnpChnlTxData, _mdnpRequestId++, 
      DNPDEFS_FILE_MODE_READ, &timeOfCreation, 0, authenticationKey, 0, pFileXferContext->maxBlockSize, 
      pFileXferContext->remoteFileName);
    
    if(pTxData != TMWDEFS_NULL)
    {
      status = DNPCHNL_RESP_STATUS_INTERMEDIATE;
      pFileXferContext->nextOperation = MDNPFILE_XFER_STATE_READ;
    }
    else
    {
      status = DNPCHNL_RESP_STATUS_FAILURE;
    }
    break;
  }
  case MDNPFILE_XFER_STATE_OPEN_WRITE:
  {   
    TMWTYPES_ULONG authenticationKey;
    authenticationKey = mdnpdata_getFileAuthKey(pMDNPSession->pDbHandle); 

    _reinitTxData(pDnpChnlTxData, "Open File Request");

    pTxData = _fileOpen((TMWSESN_TX_DATA *)pDnpChnlTxData, _mdnpRequestId++, 
      DNPDEFS_FILE_MODE_WRITE, &pFileXferContext->timeOfCreation, pFileXferContext->permissions, authenticationKey, 
      pFileXferContext->fileSize, pFileXferContext->maxBlockSize, pFileXferContext->remoteFileName);
    
    if(pTxData != TMWDEFS_NULL)
    {
      status = DNPCHNL_RESP_STATUS_INTERMEDIATE;
      pFileXferContext->nextOperation = MDNPFILE_XFER_STATE_WRITE;
    }
    else
    {
      status = DNPCHNL_RESP_STATUS_FAILURE;
    }
    break;
  }
  case MDNPFILE_XFER_STATE_READ:
  {   
    if((pFileXferContext->fileSize != 0)
      &&(pFileXferContext->bLastBlock == TMWDEFS_FALSE))
    {
      /* Read remote file */
      _reinitTxData(pDnpChnlTxData, "Read File Request");

      pTxData = _fileRead((TMWSESN_TX_DATA *)pDnpChnlTxData, 
        pFileXferContext->readBlockNum, pFileXferContext->remoteFileHandle);
      if(pTxData != TMWDEFS_NULL)
      {
        status = DNPCHNL_RESP_STATUS_INTERMEDIATE;
        pFileXferContext->nextOperation = MDNPFILE_XFER_STATE_READ;
      }
      else
      {
        status = DNPCHNL_RESP_STATUS_FAILURE;
      }

      pFileXferContext->readBlockNum++;
    }
    else
    {
      _reinitTxData(pDnpChnlTxData, "Close File Request");

      pTxData = _fileClose((TMWSESN_TX_DATA *)pDnpChnlTxData, _mdnpRequestId++, 
        pFileXferContext->remoteFileHandle);
      if(pTxData != TMWDEFS_NULL)
      {
        status = DNPCHNL_RESP_STATUS_INTERMEDIATE;
        pFileXferContext->nextOperation = MDNPFILE_XFER_STATE_DONE;
      }
      else
      {
        status = DNPCHNL_RESP_STATUS_FAILURE;
      }
    }
    break;
  }
  case MDNPFILE_XFER_STATE_WRITE:
  { 
    TMWTYPES_USHORT writeDataLen;

    /* data goes 16 bytes into tx buffer,
     * there should always be enough room, but make sure 
     */
    if(pFileXferContext->maxBlockSize > (pDnpChnlTxData->tmw.maxLength - 16))
      pFileXferContext->maxBlockSize = pDnpChnlTxData->tmw.maxLength - 16;

    /* Read local file data */
    writeDataLen = mdnpdata_readLocalFile(
      pMDNPSession->pDbHandle,
      &pDnpChnlTxData->tmw.pMsgBuf[16],
      pFileXferContext->maxBlockSize,
      &pFileXferContext->bLastBlock
      );

    if(writeDataLen > 0)
    {
      /* Write local file data to remote */
      _reinitTxData(pDnpChnlTxData, "Write File Request");
#if MDNPDATA_SUPPORT_OBJ120
      if (pFileXferContext->useAggressiveMode)
      {
        pDnpChnlTxData->authAggressiveMode = TMWDEFS_TRUE;
      }
#endif

      pTxData = _fileWrite((TMWSESN_TX_DATA *)pDnpChnlTxData, 
        pFileXferContext->writeBlockNum, pFileXferContext->bLastBlock, 
        pFileXferContext->remoteFileHandle,TMWDEFS_NULL,writeDataLen);
      if(pTxData != TMWDEFS_NULL)
      {
        status = DNPCHNL_RESP_STATUS_INTERMEDIATE; 
        
        pFileXferContext->nextOperation = MDNPFILE_XFER_STATE_WRITE;
        if(pFileXferContext->bLastBlock == TMWDEFS_TRUE)
        {
          pFileXferContext->nextOperation = MDNPFILE_XFER_STATE_CLOSE;
        }
        pFileXferContext->writeBlockNum++;
      }
      else
      {
        status = DNPCHNL_RESP_STATUS_FAILURE;
      }
    }
    break;
  }
  case MDNPFILE_XFER_STATE_CLOSE:
    /* Close remote file */
    _reinitTxData(pDnpChnlTxData, "Close File Request");

    pTxData = _fileClose((TMWSESN_TX_DATA *)pDnpChnlTxData, _mdnpRequestId++, 
      pFileXferContext->remoteFileHandle);
    if(pTxData != TMWDEFS_NULL)
    {
      status = DNPCHNL_RESP_STATUS_INTERMEDIATE;
      pFileXferContext->nextOperation = MDNPFILE_XFER_STATE_DONE;
    }
    else
    {
      status = DNPCHNL_RESP_STATUS_FAILURE;
    }
    break;

  case MDNPFILE_XFER_STATE_DONE:
    last = TMWDEFS_TRUE;
    /* local file will be closed by _cleanupFileXfer */
    break;

  case MDNPFILE_XFER_STATE_FAILED:
    last = TMWDEFS_TRUE;
    break;

  case MDNPFILE_XFER_STATE_DELETE:
  {
    TMWTYPES_ULONG authenticationKey;
    authenticationKey = mdnpdata_getFileAuthKey(pMDNPSession->pDbHandle);

    _reinitTxData(pDnpChnlTxData, "Delete File Request"); 

    /* Send delete request to slave */
    pTxData = _fileDelete((TMWSESN_TX_DATA *)pDnpChnlTxData, _mdnpRequestId++, 
      authenticationKey, pFileXferContext->remoteFileName);  
    if(pTxData != TMWDEFS_NULL)
    {
      status = DNPCHNL_RESP_STATUS_INTERMEDIATE;
      pFileXferContext->nextOperation = MDNPFILE_XFER_STATE_RETURN;
    }
    else
    {
      status = DNPCHNL_RESP_STATUS_FAILURE;
    }

    break;
  }
  
  default:
    break;
  }
  
  /* If we have not allocated another request, we must be finished with file transfer
   * free transfer context here, so user can call next request from callback function 
   */
  if(pTxData == TMWDEFS_NULL)
  {
    _cleanupFileXfer(pMDNPSession, TMWDEFS_NULL);
  }

  if(pDnpChnlTxData != (DNPCHNL_TX_DATA *)pResponse->pTxData)
  {
    /* This was a delayed response for the file request 
     * it came back as the result of an event read or unsolicited response
     */
    if(pDnpChnlTxData->pUserCallback != TMWDEFS_NULL)
    { 
      DNPCHNL_RESPONSE_INFO response;

      /* Initialize user callback info */
      response.iin = pMDNPSession->currentIIN;
      response.pSession = pSession;
      response.pTxData = (TMWSESN_TX_DATA *)pDnpChnlTxData;
      response.pRxData = pResponse->pRxData;
      response.responseTime = tmwtarg_getMSTime() - timeSent;
      response.status = status;
      response.requestStatus = 0;
      response.last = last;

      dnpchnl_userCallback(pDnpChnlTxData->tmw.pChannel, pDnpChnlTxData, &response);

      /* Remove this request if we are not sending another request, ie
       * this is not an intermediate response 
       */
      if(status != DNPCHNL_RESP_STATUS_INTERMEDIATE)
      {
        dnpchnl_freeTxData((TMWSESN_TX_DATA *)pDnpChnlTxData);
      }
    }
  }
  else
  {
    /* If this was an immediate response, return these statuses */
    pResponse->status = status;
    pResponse->last = last;
  }
}

#endif /* MDNPDATA_SUPPORT_OBJ70 */
