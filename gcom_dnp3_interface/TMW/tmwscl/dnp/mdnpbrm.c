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

/* file: mdnpbrm.c
 * description: Implement methods used to create and send DNP3 requests.
 */
#include "tmwscl/dnp/dnpdiag.h"
#include "tmwscl/dnp/mdnpdiag.h"
#include "tmwscl/dnp/mdnpbrm.h"
#include "tmwscl/dnp/dnpdefs.h"
#include "tmwscl/dnp/dnpdtime.h"
#include "tmwscl/dnp/dnpchnl.h"
#include "tmwscl/dnp/mdnpsesn.h"
#include "tmwscl/dnp/mdnpfile.h"
#include "tmwscl/dnp/mdnpo085.h"
#include "tmwscl/dnp/mdnpo086.h"
#include "tmwscl/dnp/mdnpo087.h"
#include "tmwscl/dnp/mdnpmem.h"
#include "tmwscl/utils/tmwtarg.h"
#if DNPCNFG_SUPPORT_AUTHENTICATION
#include "tmwscl/dnp/mdnpauth.h"
#if MDNPCNFG_SUPPORT_SA_VERSION5
#include "tmwscl/dnp/mdnpsa.h"
#endif
#endif

/* forward declaration */
static TMWSESN_TX_DATA * TMWDEFS_LOCAL _binOutFeedbackPoll(
  DNPCHNL_TX_DATA *pTxData);

static TMWSESN_TX_DATA * TMWDEFS_GLOBAL _anlgOutFeedbackPoll(
  DNPCHNL_TX_DATA *pTxData);

static TMWSESN_TX_DATA * TMWDEFS_GLOBAL _frznCntrFeedbackPoll(
  DNPCHNL_TX_DATA *pTxData);

static TMWSESN_TX_DATA * TMWDEFS_GLOBAL _frznAnlgInFeedbackPoll(
  DNPCHNL_TX_DATA *pTxData);

static TMWSESN_TX_DATA * TMWDEFS_GLOBAL _writeTime(
  DNPCHNL_TX_DATA *pTxData);

static TMWSESN_TX_DATA * TMWDEFS_GLOBAL _writeRecordedTime(
  DNPCHNL_TX_DATA *pTxData,
  TMWTYPES_BOOL broadcast);

static void TMWDEFS_LOCAL _initReqDesc(
  MDNPBRM_REQ_DESC *pReqDesc)
{
  pReqDesc->pChannel = TMWDEFS_NULL;
  pReqDesc->pSession = TMWDEFS_NULL;
  pReqDesc->broadcastAddr = DNPDEFS_BROADCAST_ADDR_CON;
  pReqDesc->priority = 128;
  pReqDesc->responseTimeout = 0;
  pReqDesc->pUserCallback = TMWDEFS_NULL;
  pReqDesc->pUserCallbackParam = TMWDEFS_NULL;
  pReqDesc->pMsgDescription = TMWDEFS_NULL;
}

/* function: _initTxData */
static TMWTYPES_BOOL TMWDEFS_LOCAL _initTxData(
  DNPCHNL_TX_DATA *pTxData,
  MDNPBRM_REQ_DESC *pReqDesc,
  const char *defaultMsgDescription)
{
  if(pReqDesc->pSession == TMWDEFS_NULL)
  {
    pTxData->tmw.destAddress = pReqDesc->broadcastAddr;
    pTxData->tmw.txFlags |= (TMWSESN_TXFLAGS_NO_RESPONSE | TMWSESN_TXFLAGS_BROADCAST);
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

TMWTYPES_BOOL TMWDEFS_LOCAL _sendFragment(
  DNPCHNL_TX_DATA *pDNPTxData)
{
  MDNPSESN *pMDNPSession;
  TMWTYPES_UCHAR funcCode;
  
  /* Cancel the delay timer that prevents another command from being 
   * sent before the operate 
   */
  pMDNPSession = (MDNPSESN *)pDNPTxData->tmw.pSession;
  funcCode = pDNPTxData->tmw.pMsgBuf[DNPDEFS_AH_INDEX_FUNC_CODE];
  if((funcCode >= DNPDEFS_FC_OPERATE)
    &&(funcCode <= DNPDEFS_FC_DIRECT_OP_NOACK))
  {
    tmwtimer_cancel(&pMDNPSession->selOpDelayTimer);
  }

  /* Send request */
  if(!dnpchnl_sendFragment((TMWSESN_TX_DATA*)pDNPTxData))
  { 
    dnpchnl_freeTxData((TMWSESN_TX_DATA*)pDNPTxData); 
    return(TMWDEFS_FALSE);
  }
  return(TMWDEFS_TRUE);
} 
   
/* function: _opFeedbackDelayTimeout */
static void TMWDEFS_LOCAL _opFeedbackDelayTimeout(
  void *pCallbackParam)
{
  OperateCallbackData *cData = (OperateCallbackData *)pCallbackParam;
  TMWSESN *pSession = (TMWSESN *)cData->pSession;

  /* Modify the txData containing the feedback poll that was blocked on the queue so that it can be sent */
  DNPCHNL_TX_DATA *pDNPTxData = cData->pTxData;

  /* Free callback data since we are done with it */
  mdnpmem_free(cData);

  /* To prevent _operateFeedbackTimeoutResponse callback which is no longer needed */
  pDNPTxData->pInternalCallback = TMWDEFS_NULL;

  /* No longer blocked waiting */
  pDNPTxData->sent = TMWDEFS_FALSE;

  dnpchnl_okToSend(pSession->pChannel);
}

#if MDNPDATA_SUPPORT_DATASETS
/* Queue a request that will not be sent, but will allow a command to timeout or channel to be closed
 * deallocating memory and stopping timer if needed
 */
static TMWTYPES_BOOL TMWDEFS_LOCAL _queueDelayedRequest(
  MDNPBRM_DATSET_XFER_CONTEXT *cData)
{
  MDNPBRM_REQ_DESC reqDesc;
  DNPCHNL_TX_DATA *pDNPTxData = cData->pTxData;

  /* Build request descriptor */
  mdnpbrm_initReqDesc(&reqDesc, pDNPTxData->tmw.pSession);
 
#if TMWCNFG_SUPPORT_DIAG
  pDNPTxData->tmw.pMsgDescription = "Delayed Request";
#endif
  /* Build request header */
  mdnpbrm_buildRequestHeader(pDNPTxData, DNPDEFS_FC_READ); 

  /* Setting sent to true will keep this from being sent while waiting for delay timer to expire */
  pDNPTxData->sent = TMWDEFS_TRUE;

  /* Queue request */
  if (dnpchnl_sendFragment((TMWSESN_TX_DATA *)pDNPTxData))
    return TMWDEFS_TRUE;

  return TMWDEFS_FALSE;
}

static TMWTYPES_BOOL TMWDEFS_LOCAL _writeDatasetProtos(
  MDNPBRM_DATSET_XFER_CONTEXT *cData)
{
  MDNPBRM_REQ_DESC reqDesc;
  DNPCHNL_TX_DATA *pDNPTxData = cData->pTxData;
 
  cData->nextOperation = MDNPBRM_DATASET_READ_DESCR;

  /* Build request descriptor */
  mdnpbrm_initReqDesc(&reqDesc, pDNPTxData->tmw.pSession);

  reqDesc.priority = MDNPBRM_DISABLE_UNSOL_PRIORITY;
#if TMWCNFG_SUPPORT_DIAG
  pDNPTxData->tmw.pMsgDescription = "Automatic Write of Master defined Data Set Prototypes";
#endif
  /* Build request header */
  mdnpbrm_buildRequestHeader(pDNPTxData, DNPDEFS_FC_WRITE);
  if (mdnpo085_writeV1((TMWSESN_TX_DATA *)pDNPTxData, &reqDesc, (TMWSESN_TX_DATA *)pDNPTxData, 0, TMWDEFS_NULL))
  {
    /* Send request */
    if (dnpchnl_sendFragment((TMWSESN_TX_DATA *)pDNPTxData))
      return TMWDEFS_TRUE;
  }
  return TMWDEFS_FALSE;
}

static TMWTYPES_BOOL TMWDEFS_LOCAL _writeDatasetDescrs(
  MDNPBRM_DATSET_XFER_CONTEXT *cData)
{
  MDNPBRM_REQ_DESC reqDesc;
  DNPCHNL_TX_DATA *pDNPTxData = cData->pTxData;

  cData->nextOperation = MDNPBRM_DATASET_COMPLETE;

  /* Build request descriptor */
  mdnpbrm_initReqDesc(&reqDesc, pDNPTxData->tmw.pSession);

  reqDesc.priority = MDNPBRM_DISABLE_UNSOL_PRIORITY;
#if TMWCNFG_SUPPORT_DIAG
  pDNPTxData->tmw.pMsgDescription = "Automatic Write of Master defined Data Set Descriptors";
#endif
  /* Build request header */
  mdnpbrm_buildRequestHeader(pDNPTxData, DNPDEFS_FC_WRITE);

  if (mdnpo086_writeV1((TMWSESN_TX_DATA *)pDNPTxData, &reqDesc, (TMWSESN_TX_DATA *)pDNPTxData, 0, TMWDEFS_NULL))
  {
    /* Send request */
    if (dnpchnl_sendFragment((TMWSESN_TX_DATA *)pDNPTxData))
      return TMWDEFS_TRUE;
  }
  return TMWDEFS_FALSE;
} 

/* function: _datasetDelayFunc */
static void TMWDEFS_LOCAL _datasetDelayFunc(
  void *pCallbackParam)
{
  MDNPBRM_DATSET_XFER_CONTEXT *cData = (MDNPBRM_DATSET_XFER_CONTEXT *)pCallbackParam;
  DNPCHNL_TX_DATA *pDNPTxData = cData->pTxData;
  TMWCHNL *pChannel = pDNPTxData->tmw.pSession->pChannel;

  switch(cData->nextOperation)
  {
  case MDNPBRM_DATASET_WRITE_PROTO:
    /* dont have to wait any longer 
     * remove the temporary request.
     */
    tmwdlist_removeEntry(&pChannel->messageQueue, (TMWDLIST_MEMBER *)pDNPTxData);
    pDNPTxData->sent = TMWDEFS_FALSE;
    _writeDatasetProtos(cData);
    break;

  case MDNPBRM_DATASET_WRITE_DESCR:
    /* dont have to wait any longer
    * remove the temporary request.
    */
    tmwdlist_removeEntry(&pChannel->messageQueue, (TMWDLIST_MEMBER *)pDNPTxData);
    pDNPTxData->sent = TMWDEFS_FALSE;
    _writeDatasetDescrs(cData);
    break;

  default:
    /* If Error we are finished with context */
    mdnpmem_free(cData);
    return;
  } 
}
 
static TMWTYPES_BOOL TMWDEFS_LOCAL _readDatasetDescrs(
  MDNPBRM_DATSET_XFER_CONTEXT *cData)
{
  DNPCHNL_TX_DATA *pDNPTxData;
  MDNPSESN *pMDNPSession;
  TMWTYPES_USHORT quantity;
  TMWTYPES_UCHAR qualifier;

  pDNPTxData = cData->pTxData;
  pMDNPSession = (MDNPSESN *)pDNPTxData->tmw.pSession;

  quantity = mdnpdata_datasetDescrQuantity(pMDNPSession->pDbHandle) -
    mdnpdata_datasetDescrSlaveQty(pMDNPSession->pDbHandle);

  /* If there are descriptors defined on the outstation, read them */
  if (cData->numberDescrsDefinedOnSlave > 0)
  {
    /* Set up next operation based on whether there are prototypes to write to outstation */
    if (quantity > 0)
      cData->nextOperation = MDNPBRM_DATASET_DELAY_DESCR;
    else
      cData->nextOperation = MDNPBRM_DATASET_COMPLETE;

#if TMWCNFG_SUPPORT_DIAG
    pDNPTxData->tmw.pMsgDescription = "Automatic Read of Outstation defined Data Set Descriptors";
#endif
    /* Build request header */
    mdnpbrm_buildRequestHeader(pDNPTxData, DNPDEFS_FC_READ);

    qualifier = DNPDEFS_QUAL_8BIT_START_STOP;
    if (cData->numberDescrsDefinedOnSlave > 256)
      qualifier = DNPDEFS_QUAL_16BIT_START_STOP;

    if (mdnpbrm_addObjectHeader(pDNPTxData, 86, 1, qualifier, 0,
      (TMWTYPES_USHORT)(cData->numberDescrsDefinedOnSlave - 1)))
    {
      /* Send request */
      if (dnpchnl_sendFragment((TMWSESN_TX_DATA *)pDNPTxData))
      {
        return TMWDEFS_FALSE;
      }
    }
  }
  else
  {
    /* If no descriptors defined on Outstation skip reading the descriptors
     * Write descriptors now
     */
    if (_writeDatasetDescrs(cData))
    {
      return TMWDEFS_FALSE;
    }
  }
  return TMWDEFS_TRUE;
}

/* function: _datasetCallbackFunc */
static void TMWDEFS_LOCAL _datasetCallbackFunc(
  void *pCallbackParam,
  DNPCHNL_RESPONSE_INFO *pResponse)
{
  TMWTYPES_BOOL complete = TMWDEFS_TRUE;
  MDNPBRM_DATSET_XFER_CONTEXT *cData = (MDNPBRM_DATSET_XFER_CONTEXT *)pCallbackParam;

  /* First check to make sure the operation succeeded */
  if(pResponse->status == DNPCHNL_RESP_STATUS_SUCCESS)
  {
    TMWTYPES_USHORT quantity;
    TMWTYPES_UCHAR qualifier;
    DNPCHNL_TX_DATA *pDNPTxData = cData->pTxData;
    MDNPSESN *pMDNPSession = (MDNPSESN *)pDNPTxData->tmw.pSession;

    /* Reuse the original txData */
    pDNPTxData->sent = TMWDEFS_FALSE;   
    pDNPTxData->tmw.txFlags = 0;
    pDNPTxData->priority = MDNPBRM_DATASET_XCHNG_PRIORITY;

    switch(cData->nextOperation)
    {
    case MDNPBRM_DATASET_READ_PROTO:
      quantity = mdnpdata_datasetProtoQuantity(pMDNPSession->pDbHandle) - 
        mdnpdata_datasetProtoSlaveQty(pMDNPSession->pDbHandle);
       
      /* If there are prototypes defined on the outstation, read them */
      if (cData->numberProtosDefinedOnSlave > 0)
      {
        /* Set up next operation based on whether there are prototypes to write to outstation */
        if (quantity > 0)
          cData->nextOperation = MDNPBRM_DATASET_DELAY_PROTO;
        else
          cData->nextOperation = MDNPBRM_DATASET_READ_DESCR;

#if TMWCNFG_SUPPORT_DIAG
        pDNPTxData->tmw.pMsgDescription = "Automatic Read of Outstation defined Data Set Protos";
#endif
        /* Build request header */
        mdnpbrm_buildRequestHeader(pDNPTxData, DNPDEFS_FC_READ);

        qualifier = DNPDEFS_QUAL_8BIT_START_STOP;

        if (cData->numberProtosDefinedOnSlave > 256)
         qualifier = DNPDEFS_QUAL_16BIT_START_STOP;

        if (mdnpbrm_addObjectHeader(pDNPTxData, 85, 1, qualifier, 0,
          (TMWTYPES_USHORT)(cData->numberProtosDefinedOnSlave - 1)))
        {
          /* Send request */
          if (dnpchnl_sendFragment((TMWSESN_TX_DATA *)pDNPTxData))
          {
            complete = TMWDEFS_FALSE;
          }
        }
      }
      else
      {
        /* If no prototypes defined on Outstation skip reading the prototypes
         * Write prototypes now if there are any on master
         */
        if (quantity > 0)
        {
          if (_writeDatasetProtos(cData))
          {
            complete = TMWDEFS_FALSE;
          }
        }
        else
        {
          complete = _readDatasetDescrs(cData);
        }
      }
      break;

    case MDNPBRM_DATASET_DELAY_PROTO:
      cData->nextOperation = MDNPBRM_DATASET_WRITE_PROTO;

      /* Start timer to delay so that Prototypes read can be stored in database 
       * This is necessary if async database functionality is being used
       */
      tmwtimer_start(&cData->delayTimer, MDNPCNFG_DATASET_DELAY, 
        pResponse->pSession->pChannel, _datasetDelayFunc, cData);

      /* Need a request on the pChannel->messageQueue so timeouts and closing of channels are handled properly.
       * But we need to wait for prototypes from outstation to be in database to know Ids for master defined descriptors
       */
      if(_queueDelayedRequest(cData))
      {
        complete = TMWDEFS_FALSE;
      }
      break;
     
    case MDNPBRM_DATASET_READ_DESCR:  
      complete = _readDatasetDescrs(cData);
      break;

    case MDNPBRM_DATASET_DELAY_DESCR:
      cData->nextOperation = MDNPBRM_DATASET_WRITE_DESCR;

      /* Start timer to delay so that Descriptors read can be stored in database 
       * This is necessary if async database functionality is being used
       */
      tmwtimer_start(&cData->delayTimer, MDNPCNFG_DATASET_DELAY, 
        pResponse->pSession->pChannel, _datasetDelayFunc, cData);

      /* Need a request on the pChannel->messageQueue so timeouts and closing of channels are handled properly.
       * But we need to wait for descriptors from outstation to be in database to know Ids for master defined descriptors
       */
      if (_queueDelayedRequest(cData))
      {
        complete = TMWDEFS_FALSE;
      }
      break;

    case MDNPBRM_DATASET_OPERATE:
      if(pResponse->status == DNPCHNL_RESP_STATUS_SUCCESS)
      {
        /* Reuse the original txData */
        pDNPTxData = cData->pTxData;
        pDNPTxData->sent = TMWDEFS_FALSE;   
        pDNPTxData->tmw.txFlags = 0;

        /* Set priority high to make sure this is the next thing to go out */
        pDNPTxData->priority = MDNPBRM_AUTO_OPERATE_PRIORITY;
     
        /* Change function code to operate */
        pDNPTxData->tmw.pMsgBuf[DNPDEFS_AH_INDEX_FUNC_CODE] = DNPDEFS_FC_OPERATE;

#if TMWCNFG_SUPPORT_DIAG
        pDNPTxData->tmw.pMsgDescription = "Operate Data Set Request as a result of Select Response";
#endif

        pDNPTxData->pInternalCallback = TMWDEFS_NULL;
        mdnpmem_free(cData);
     
        /* Issue new request */
        if(!dnpchnl_sendFragment((TMWSESN_TX_DATA *)pDNPTxData))
        { 
          dnpchnl_freeTxData((TMWSESN_TX_DATA *)pDNPTxData);
        }

        pResponse->status = DNPCHNL_RESP_STATUS_INTERMEDIATE;
        pResponse->last = TMWDEFS_FALSE;

        complete = TMWDEFS_FALSE;
      }
      else 
      {
        mdnpmem_free(cData);
      }
      break;
    }
  } 
  else if (pResponse->status == DNPCHNL_RESP_STATUS_INTERMEDIATE)
  {
     complete = TMWDEFS_FALSE;
  }

  if(!complete)
  {
    /* This will prevent reused txData from being deallocated */
    pResponse->status = DNPCHNL_RESP_STATUS_INTERMEDIATE;
    pResponse->last = TMWDEFS_FALSE;
  }
  else
  { 
    tmwtimer_cancel(&cData->delayTimer);
    mdnpmem_free(cData);
  }
}
#endif 

/* function: _operateFeedbackTimeoutResponse */
static void TMWDEFS_LOCAL _operateFeedbackTimeoutResponse(
  void *pCallbackParam,
  DNPCHNL_RESPONSE_INFO *pResponse)
{
  if((pResponse->status == DNPCHNL_RESP_STATUS_TIMEOUT)
     ||(pResponse->status == DNPCHNL_RESP_STATUS_CANCELED))
  {
    /* Free callback data since we are done with it */
    OperateCallbackData *cData = (OperateCallbackData *)pCallbackParam;

    tmwtimer_cancel(&cData->feedbackTimer);

    mdnpmem_free(cData);
  }
}

/* function: _selectOperateCallbackFunc */
static void TMWDEFS_LOCAL _selectOperateCallbackFunc(
  void *pCallbackParam,
  DNPCHNL_RESPONSE_INFO *pResponse)
{
  OperateCallbackData *cData = (OperateCallbackData *)pCallbackParam;

  /* First check to make sure the operation succeeded */
  if (pResponse->status == DNPCHNL_RESP_STATUS_SUCCESS)
  {
    DNPCHNL_TX_DATA *pDNPTxData;

    /* Make sure there was a response in case a non NOACK command was incorrectly sent as a broadcast request
     * which would not have a response. In that case we cannot reuse the pTxData.
     */
    if (pResponse->pRxData == TMWDEFS_NULL)
    {
      MDNPDIAG_ERROR(pResponse->pTxData->pChannel, pResponse->pSession, MDNPDIAG_OPER_FEEDBACK);

      mdnpmem_free(cData);
      return;
    }

    /* Reuse the original txData */
    pDNPTxData = cData->pTxData;
    pDNPTxData->sent = TMWDEFS_FALSE;
    pDNPTxData->tmw.txFlags = 0;

    /* Set priority high to make sure this is the next thing to go out */
    pDNPTxData->priority = MDNPBRM_AUTO_OPERATE_PRIORITY;

    /* See if we need to perform an operate */
    if (cData->operateRequired)
    {
      /* Change function code to operate */
      pDNPTxData->tmw.pMsgBuf[DNPDEFS_AH_INDEX_FUNC_CODE] = DNPDEFS_FC_OPERATE;

#if TMWCNFG_SUPPORT_DIAG
      pDNPTxData->tmw.pMsgDescription = "Operate Request as a result of Select Response";
#endif

#if MDNPDATA_SUPPORT_OBJ120
      if (pDNPTxData->authAggressiveMode)
      {
        TMWTYPES_USHORT saveLength = pDNPTxData->tmw.msgLength;
        pDNPTxData->tmw.msgLength = 2;
        mdnpauth_addAggrRequestStart((TMWSESN_TX_DATA *)pDNPTxData, pDNPTxData->authUserNumber);
        pDNPTxData->tmw.msgLength = saveLength - pDNPTxData->authAggrModeObjLength;
      }
#endif 

      if ((cData->binFeedbackRequired) 
        || (cData->anlgFeedbackRequired))
      {
        cData->operateRequired = TMWDEFS_FALSE;
      }
      else
      {
        pDNPTxData->pInternalCallback = TMWDEFS_NULL;
        mdnpmem_free(cData);
      }

      /* Issue operate request */
      _sendFragment(pDNPTxData);

      pResponse->status = DNPCHNL_RESP_STATUS_INTERMEDIATE;
      pResponse->last = TMWDEFS_FALSE;
    }
    else if ((cData->binFeedbackRequired)
      || (cData->anlgFeedbackRequired))
    {
      TMWTYPES_BOOL binFeedbackRequired = cData->binFeedbackRequired;
      TMWTYPES_BOOL anlgFeedbackRequired = cData->anlgFeedbackRequired;

      /* Reuse txData */
      cData->pTxData = pDNPTxData;

      if (cData->feedbackDelay != 0)
      {
        /* If response timeout while waiting for feedback delay, need to clean up */
        pDNPTxData->pInternalCallback = _operateFeedbackTimeoutResponse;

        /* Setting sent to true will keep this from being sent while waiting for feedback delay timer to expire */
        pDNPTxData->sent = TMWDEFS_TRUE;

        tmwtimer_start(&cData->feedbackTimer, cData->feedbackDelay,
          pResponse->pSession->pChannel, _opFeedbackDelayTimeout, cData);
      }
      else
      {
        pDNPTxData->pInternalCallback = TMWDEFS_NULL;
        mdnpmem_free(cData);
      }

      /* Set priority high, but not as high as authentication or select/operate */
      pDNPTxData->priority = 250;

      /* Issue operate feedback poll */
      /* Only does either binOut or anlgOut feedback when multiple request objects are in one request */
      if (binFeedbackRequired)
      {
#if TMWCNFG_SUPPORT_DIAG
        pDNPTxData->tmw.pMsgDescription = "Operate Binary Output Feedback Poll";
#endif
        _binOutFeedbackPoll(pDNPTxData);
      }
      else if (anlgFeedbackRequired)
      {
#if TMWCNFG_SUPPORT_DIAG
        pDNPTxData->tmw.pMsgDescription = "Operate Analog Output Feedback Poll";
#endif
        _anlgOutFeedbackPoll(pDNPTxData);
      }

      pResponse->status = DNPCHNL_RESP_STATUS_INTERMEDIATE;
      pResponse->last = TMWDEFS_FALSE;
    }
  }
  else if(pResponse->status != DNPCHNL_RESP_STATUS_INTERMEDIATE)
  {
    mdnpmem_free(cData);
  }

  return;
}

/* function: _freezeCountersCallbackFunc */
static void TMWDEFS_LOCAL _freezeCountersCallbackFunc(
  void *pCallbackParam,
  DNPCHNL_RESPONSE_INFO *pResponse)
{
  OperateCallbackData *cData = (OperateCallbackData *)pCallbackParam;

  /* First check to make sure the operation succeeded */
  if(pResponse->status == DNPCHNL_RESP_STATUS_SUCCESS)
  {
    DNPCHNL_TX_DATA *pDNPTxData;

    /* Check status code from response */
    if(pResponse->pRxData == TMWDEFS_NULL)
    {
      MDNPDIAG_ERROR(pResponse->pTxData->pChannel, pResponse->pSession, MDNPDIAG_FRZN_FEEDBACK);

      mdnpmem_free(cData);
      return;
    }
   
    /* Reuse the original txData */
    pDNPTxData = cData->pTxData;
    pDNPTxData->sent = TMWDEFS_FALSE;   
    pDNPTxData->tmw.txFlags = 0;
    pDNPTxData->pInternalCallback = TMWDEFS_NULL;
#if TMWCNFG_SUPPORT_DIAG
      pDNPTxData->tmw.pMsgDescription = "Operate Frozen Counter Feedback Poll";
#endif
	
    /* Issue feedback poll */
    _frznCntrFeedbackPoll(pDNPTxData);
  
    /* Free callback data since we are done with it */
    mdnpmem_free(cData);

    /* not finished since read frozen counters is being done now */
    pResponse->status = DNPCHNL_RESP_STATUS_INTERMEDIATE;
    pResponse->last = TMWDEFS_FALSE;
  } 
  else 
  {
    mdnpmem_free(cData);
  }
  return;
}

/* function: _freezeAnalogInputsCallbackFunc */
static void TMWDEFS_LOCAL _freezeAnalogInputsCallbackFunc(
  void *pCallbackParam,
  DNPCHNL_RESPONSE_INFO *pResponse)
{
  OperateCallbackData *cData = (OperateCallbackData *)pCallbackParam;

  /* First check to make sure the operation succeeded */
  if(pResponse->status == DNPCHNL_RESP_STATUS_SUCCESS)
  {
    DNPCHNL_TX_DATA *pDNPTxData;

    /* Check status code from response */
    if(pResponse->pRxData == TMWDEFS_NULL)
    {
      MDNPDIAG_ERROR(pResponse->pTxData->pChannel, pResponse->pSession, MDNPDIAG_FRZN_FEEDBACK);

      mdnpmem_free(cData);
      return;
    }
   
    /* Reuse the original txData */
    pDNPTxData = cData->pTxData;
    pDNPTxData->sent = TMWDEFS_FALSE;   
    pDNPTxData->tmw.txFlags = 0;
    pDNPTxData->pInternalCallback = TMWDEFS_NULL;
#if TMWCNFG_SUPPORT_DIAG
      pDNPTxData->tmw.pMsgDescription = "Operate Frozen Analog Inputs Feedback Poll";
#endif
	
    /* Issue feedback poll */
    _frznAnlgInFeedbackPoll(pDNPTxData);
  
    /* Free callback data since we are done with it */
    mdnpmem_free(cData);

    /* not finished since read frozen counters is being done now */
    pResponse->status = DNPCHNL_RESP_STATUS_INTERMEDIATE;
    pResponse->last = TMWDEFS_FALSE;
  } 
  else 
  {
    mdnpmem_free(cData);
  }
  return;
}

#if MDNPDATA_SUPPORT_OBJ50_V1
/* function: _delayCallbackFunc */
static void TMWDEFS_LOCAL _delayCallbackFunc(
  void *pCallbackParam,
  DNPCHNL_RESPONSE_INFO *pResponse)
{
  TMWSESN *pSession = (TMWSESN *)pCallbackParam;
  MDNPSESN *pMDNPSession = (MDNPSESN *)pSession;

  if(pResponse->status == DNPCHNL_RESP_STATUS_SUCCESS)
  {
    /* Reuse the original txData */
    DNPCHNL_TX_DATA *pDNPTxData = (DNPCHNL_TX_DATA *)pResponse->pTxData;

    pDNPTxData->sent = TMWDEFS_FALSE;   
    pDNPTxData->tmw.txFlags = 0;
    pDNPTxData->pInternalCallback = TMWDEFS_NULL;
#if TMWCNFG_SUPPORT_DIAG
    pDNPTxData->tmw.pMsgDescription  = "Time Synchronization After Delay Measurement";
#endif
	
    /* Set priority high to make sure this goes out */
    pDNPTxData->priority = 253;

    _writeTime(pDNPTxData);

    pResponse->status = DNPCHNL_RESP_STATUS_INTERMEDIATE;
    pResponse->last = TMWDEFS_FALSE;

    /* Clear the need time IIN bit so we don't try to do another time sync */
    pMDNPSession->currentIIN &= ~DNPDEFS_IIN_NEED_TIME;
  }
}
#endif

#if MDNPDATA_SUPPORT_OBJ50_V3
/* function: _recordTimeCallbackFunc */
static void TMWDEFS_LOCAL _recordTimeCallbackFunc(
  void *pCallbackParam,
  DNPCHNL_RESPONSE_INFO *pResponse)
{
  MDNPSESN *pMDNPSession = (MDNPSESN *)pCallbackParam;
  TMWTYPES_BOOL broadcast = TMWDEFS_FALSE;

  if(pResponse->status == DNPCHNL_RESP_STATUS_SUCCESS)
  {
    /* Reuse the original txData */
    DNPCHNL_TX_DATA *pDNPTxData = (DNPCHNL_TX_DATA *)pResponse->pTxData;

    /* If this was a broadcast request allocate another txdata
      This is called from the after transmit callback and is difficult to reuse the original
      */
    if ((pDNPTxData->tmw.txFlags & TMWSESN_TXFLAGS_BROADCAST) != 0)
    {
      DNPCHNL_TX_DATA *pOrigTxData = pDNPTxData;
      pDNPTxData = (DNPCHNL_TX_DATA *)dnpchnl_newTxData(
        pOrigTxData->tmw.pChannel, pOrigTxData->tmw.pSession, 12 + DNPAUTH_AGGRESSIVE_SIZE, 0);
      
#if MDNPDATA_SUPPORT_OBJ120
      pDNPTxData->authUserNumber = pOrigTxData->authUserNumber;
      pDNPTxData->authAggressiveMode = pOrigTxData->authAggressiveMode;
#endif
       
      pDNPTxData->tmw.destAddress = pOrigTxData->tmw.destAddress;

      /* Response timeout */
      pDNPTxData->tmw.responseTimeout = pOrigTxData->tmw.responseTimeout;

      /* Initialize user callback info */
      pDNPTxData->pUserCallback = pOrigTxData->pUserCallback;
      pDNPTxData->pUserCallbackParam = pOrigTxData->pUserCallbackParam;

      broadcast = TMWDEFS_TRUE;
    }

    pDNPTxData->sent = TMWDEFS_FALSE;   
    pDNPTxData->tmw.txFlags = 0;
    pDNPTxData->pInternalCallback = TMWDEFS_NULL;
#if TMWCNFG_SUPPORT_DIAG
    pDNPTxData->tmw.pMsgDescription  = "Write Recorded Time";
#endif
	
    /* Set priority high to make sure this goes out */
    pDNPTxData->priority = MDNPBRM_TIMESYNC_PRIORITY;

    _writeRecordedTime(pDNPTxData, broadcast);

    pResponse->status = DNPCHNL_RESP_STATUS_INTERMEDIATE;
    pResponse->last = TMWDEFS_FALSE;

    /* If Not Broadcast, clear the need time IIN bit so we don't try to do another time sync */
    if(!broadcast)
      pMDNPSession->currentIIN &= ~DNPDEFS_IIN_NEED_TIME;
  }
}
#endif

/* function: mdnpbrm_initBroadcastDesc */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpbrm_initBroadcastDesc(
  MDNPBRM_REQ_DESC *pReqDesc,
  TMWCHNL *pChannel,
  TMWTYPES_USHORT destAddr)
{
  _initReqDesc(pReqDesc);
  pReqDesc->pChannel = pChannel;
  pReqDesc->broadcastAddr = destAddr;

#if MDNPDATA_SUPPORT_OBJ120
  pReqDesc->authAggressiveMode = TMWDEFS_FALSE;
  pReqDesc->authUserNumber = DNPAUTH_DEFAULT_USERNUMBER;
#endif

  return(TMWDEFS_TRUE);
}

/* function: mdnpbrm_initReqDesc */
TMWTYPES_BOOL TMWDEFS_LOCAL mdnpbrm_initReqDesc(
  MDNPBRM_REQ_DESC *pReqDesc,
  TMWSESN *pSession)
{
  MDNPSESN *pMDNPSession = (MDNPSESN *)pSession;

  _initReqDesc(pReqDesc);
  pReqDesc->pSession = pSession;
  pReqDesc->responseTimeout = pMDNPSession->defaultResponseTimeout;
  
#if MDNPDATA_SUPPORT_OBJ120
  pReqDesc->authAggressiveMode = TMWDEFS_FALSE;
  pReqDesc->authUserNumber = DNPAUTH_DEFAULT_USERNUMBER;
#endif

  return(TMWDEFS_TRUE);
}

/* function: mdnpbrm_buildRequestHeader */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpbrm_buildRequestHeader(
  DNPCHNL_TX_DATA *pTxData,
  TMWTYPES_UCHAR funcCode)
{
  pTxData->tmw.pMsgBuf[0] = 0xc0;
  pTxData->tmw.pMsgBuf[1] = funcCode;
  pTxData->tmw.msgLength = 2;

  DNPDIAG_BUILD_MESSAGE(pTxData->tmw.pChannel, pTxData->tmw.pSession, pTxData->tmw.pMsgDescription);
  return(TMWDEFS_TRUE);
}

/* function: mdnpbrm_addObjectHeader */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpbrm_addObjectHeader(
  DNPCHNL_TX_DATA *pTxData,
  TMWTYPES_UCHAR group,
  TMWTYPES_UCHAR variation,
  TMWTYPES_UCHAR qualifier,
  TMWTYPES_USHORT start,
  TMWTYPES_USHORT endOrQty)
{
  /* Make sure we'll fit */
  if((pTxData->tmw.msgLength + 3) > pTxData->tmw.maxLength)
  {
    MDNPDIAG_ERROR(pTxData->tmw.pChannel, pTxData->tmw.pSession, MDNPDIAG_INV_LENGTH);
    return(TMWDEFS_FALSE);
  }

  /* Add group, variation, and qualifier */
  pTxData->tmw.pMsgBuf[pTxData->tmw.msgLength++] = group;
  pTxData->tmw.pMsgBuf[pTxData->tmw.msgLength++] = variation;
  pTxData->tmw.pMsgBuf[pTxData->tmw.msgLength++] = qualifier;
 
  DNPDIAG_SHOW_TX_OBJECT_HDR(pTxData->tmw.pSession, group, variation, qualifier);
 
  /* Based on qualifier add start, stop, quantity, etc. */
  switch(qualifier)
  {
  case DNPDEFS_QUAL_8BIT_START_STOP:
    /* Make sure we'll fit */
    if((pTxData->tmw.msgLength + 2) > pTxData->tmw.maxLength)
    {
      MDNPDIAG_ERROR(pTxData->tmw.pChannel, pTxData->tmw.pSession, MDNPDIAG_INV_LENGTH);
      return(TMWDEFS_FALSE);
    }

    /* Validate start/stop */
    if((start > 255) || (endOrQty > 255) || (start > endOrQty))
    {
      MDNPDIAG_ERROR(pTxData->tmw.pChannel, pTxData->tmw.pSession, MDNPDIAG_INV_STARTSTOP);
      return(TMWDEFS_FALSE);
    }

    /* Store start and end point */
    pTxData->tmw.pMsgBuf[pTxData->tmw.msgLength++] = (TMWTYPES_UCHAR)start;
    pTxData->tmw.pMsgBuf[pTxData->tmw.msgLength++] = (TMWTYPES_UCHAR)endOrQty;
    break;

  case DNPDEFS_QUAL_16BIT_START_STOP:
    /* Make sure we'll fit */
    if((pTxData->tmw.msgLength + 4) > pTxData->tmw.maxLength)
    {
      MDNPDIAG_ERROR(pTxData->tmw.pChannel, pTxData->tmw.pSession, MDNPDIAG_INV_LENGTH);
      return(TMWDEFS_FALSE);
    }

    /* Validate start/stop */
    if(start > endOrQty)
    {
      MDNPDIAG_ERROR(pTxData->tmw.pChannel, pTxData->tmw.pSession, MDNPDIAG_INV_STARTSTOP);
      return(TMWDEFS_FALSE);
    }

    /* Store start and end point */
    tmwtarg_store16(&start, pTxData->tmw.pMsgBuf + pTxData->tmw.msgLength);
    pTxData->tmw.msgLength += 2;

    tmwtarg_store16(&endOrQty, pTxData->tmw.pMsgBuf + pTxData->tmw.msgLength);
    pTxData->tmw.msgLength += 2;
    break;

  case DNPDEFS_QUAL_ALL_POINTS:
    break;

  case DNPDEFS_QUAL_8BIT_LIMITED_QTY:
    /* Make sure we'll fit */
    if((pTxData->tmw.msgLength + 1) > pTxData->tmw.maxLength)
    {
      MDNPDIAG_ERROR(pTxData->tmw.pChannel, pTxData->tmw.pSession, MDNPDIAG_INV_LENGTH);
      return(TMWDEFS_FALSE);
    }

    /* Validate quantity */
    if((endOrQty == 0) || (endOrQty > 255))
    {
      MDNPDIAG_ERROR(pTxData->tmw.pChannel, pTxData->tmw.pSession, MDNPDIAG_INV_QUANTITY);
      return(TMWDEFS_FALSE);
    }

    /* Store quantity */
    *(pTxData->tmw.pMsgBuf + pTxData->tmw.msgLength++) = (TMWTYPES_UCHAR)endOrQty;
    break;

  case DNPDEFS_QUAL_16BIT_LIMITED_QTY:
    /* Make sure we'll fit */
    if((pTxData->tmw.msgLength + 2) > pTxData->tmw.maxLength)
    {
      MDNPDIAG_ERROR(pTxData->tmw.pChannel, pTxData->tmw.pSession, MDNPDIAG_INV_LENGTH);
      return(TMWDEFS_FALSE);
    }

    /* Validate quantity */
    if(endOrQty == 0)
    {
      MDNPDIAG_ERROR(pTxData->tmw.pChannel, pTxData->tmw.pSession, MDNPDIAG_INV_QUANTITY);
      return(TMWDEFS_FALSE);
    }

    /* Store quantity */
    tmwtarg_store16(&endOrQty, pTxData->tmw.pMsgBuf + pTxData->tmw.msgLength);
    pTxData->tmw.msgLength += 2;
    break;

  case DNPDEFS_QUAL_8BIT_INDEX:
  case DNPDEFS_QUAL_16BIT_INDEX_8BITQ:
    /* Make sure we'll fit */
    if((pTxData->tmw.msgLength + 1) > pTxData->tmw.maxLength)
    {
      MDNPDIAG_ERROR(pTxData->tmw.pChannel, pTxData->tmw.pSession, MDNPDIAG_INV_LENGTH);
      return(TMWDEFS_FALSE);
    }

    /* Validate quantity */
    if(endOrQty > 255)
    {
      MDNPDIAG_ERROR(pTxData->tmw.pChannel, pTxData->tmw.pSession, MDNPDIAG_INV_QUANTITY);
      return(TMWDEFS_FALSE);
    }

    *(pTxData->tmw.pMsgBuf + pTxData->tmw.msgLength++) = (TMWTYPES_UCHAR)endOrQty;
    break;

  case DNPDEFS_QUAL_16BIT_INDEX:
    /* Make sure we'll fit */
    if((pTxData->tmw.msgLength + 2) > pTxData->tmw.maxLength)
    {
      MDNPDIAG_ERROR(pTxData->tmw.pChannel, pTxData->tmw.pSession, MDNPDIAG_INV_LENGTH);
      return(TMWDEFS_FALSE);
    }

    tmwtarg_store16(&endOrQty, pTxData->tmw.pMsgBuf + pTxData->tmw.msgLength);
    pTxData->tmw.msgLength += 2;
    break;

  case DNPDEFS_QUAL_8BIT_FREE_FORMAT:
  case DNPDEFS_QUAL_16BIT_FREE_FORMAT:
    /* Make sure we'll fit */
    if((pTxData->tmw.msgLength + 1) > pTxData->tmw.maxLength)
    {
      MDNPDIAG_ERROR(pTxData->tmw.pChannel, pTxData->tmw.pSession, MDNPDIAG_INV_LENGTH);
      return(TMWDEFS_FALSE);
    }

    /* Store quantity */
    pTxData->tmw.pMsgBuf[pTxData->tmw.msgLength++] = (TMWTYPES_UCHAR)endOrQty;
    break;

  default:
    MDNPDIAG_ERROR(pTxData->tmw.pChannel, pTxData->tmw.pSession, MDNPDIAG_INV_QUALIFIER);
    return(TMWDEFS_FALSE);
  }

  return(TMWDEFS_TRUE);
}

/* function: mdnpbrm_addObjectData */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpbrm_addObjectData(
  DNPCHNL_TX_DATA *pTxData,
  TMWTYPES_USHORT len,
  TMWTYPES_UCHAR *pData)
{
  if(pTxData->tmw.msgLength + len > pTxData->tmw.maxLength)
  {
    MDNPDIAG_ERROR(pTxData->tmw.pChannel, pTxData->tmw.pSession, MDNPDIAG_OBJ_LENGTH);
    return(TMWDEFS_FALSE);
  }

  memcpy(pTxData->tmw.pMsgBuf + pTxData->tmw.msgLength, pData, len);
  pTxData->tmw.msgLength = (TMWTYPES_USHORT)(pTxData->tmw.msgLength + len);

  return(TMWDEFS_TRUE);
}

/* function: mdnpbrm_cancelSelOpDelayTimer */
TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpbrm_cancelSelOpDelayTimer(
  TMWSESN *pSession)
{
  return(mdnpsesn_cancelSelOpDelayTimer(pSession));
}

/* function: mdnpbrm_readClass */
TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_readClass(
  MDNPBRM_REQ_DESC *pReqDesc,
  DNPCHNL_TX_DATA *pUserTxData,
  TMWTYPES_UCHAR qualifier,
  TMWTYPES_USHORT maxQuantity,
  TMWTYPES_BOOL class0,
  TMWTYPES_BOOL class1,
  TMWTYPES_BOOL class2,
  TMWTYPES_BOOL class3)
{
  DNPCHNL_TX_DATA *pTxData;
  TMWTYPES_USHORT size;
  TMWTYPES_BOOL status;

  /* Max size is 2 Appl Header + (5 * # of classes selected) Object Header */
  size = 2;
  if(class0 == TMWDEFS_TRUE) size += 5;
  if(class1 == TMWDEFS_TRUE) size += 5;
  if(class2 == TMWDEFS_TRUE) size += 5;
  if(class3 == TMWDEFS_TRUE) size += 5;

  if(pUserTxData == TMWDEFS_NULL)
  {
#if MDNPDATA_SUPPORT_OBJ120
    if(pReqDesc->authAggressiveMode)
      size += DNPAUTH_AGGRESSIVE_SIZE;
#endif

    /* Allocate Transmit Data Structure */
    pTxData = (DNPCHNL_TX_DATA *)dnpchnl_newTxData(
      pReqDesc->pChannel, pReqDesc->pSession, size, 0);

    if(pTxData == TMWDEFS_NULL)
    {
      return(TMWDEFS_NULL);
    }

    /* Initialize Transmit Data Structure from Request Descriptor */
    if(!_initTxData(pTxData, pReqDesc, "Class Data Poll"))
    {
      dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
      return(TMWDEFS_NULL);
    }

    /* Build request header */
    mdnpbrm_buildRequestHeader(pTxData, DNPDEFS_FC_READ);

#if MDNPDATA_SUPPORT_OBJ120
    if(pReqDesc->authAggressiveMode)
    {
      if(mdnpauth_addAggrRequestStart((TMWSESN_TX_DATA *)pTxData, pReqDesc->authUserNumber) == TMWDEFS_NULL)
      {
        dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
        return(TMWDEFS_NULL);
      }
    }
#endif 
  }
  else
  {
    pTxData = pUserTxData;
    if((pUserTxData->tmw.msgLength + size) >= pUserTxData->tmw.maxLength)
    {
      MDNPDIAG_ERROR(pReqDesc->pChannel, pReqDesc->pSession, MDNPDIAG_OBJ_LENGTH);
      pTxData = TMWDEFS_NULL;
    }

    if(pUserTxData->tmw.pMsgBuf[1] != DNPDEFS_FC_READ)
    {
      MDNPDIAG_ERROR(pReqDesc->pChannel, pReqDesc->pSession, MDNPDIAG_FC);
      pTxData = TMWDEFS_NULL;
    }

    if(pTxData == TMWDEFS_NULL)
    {
      dnpchnl_freeTxData((TMWSESN_TX_DATA *)pUserTxData);
      return(TMWDEFS_NULL);
    }
  }

  /* Read requested class in order */
  status = TMWDEFS_TRUE;

  if(status && (class1 == TMWDEFS_TRUE))
    status = mdnpbrm_addObjectHeader(pTxData, DNPDEFS_OBJ_60_CLASS_SCANS, 2, qualifier, 0, maxQuantity);

  if(status && (class2 == TMWDEFS_TRUE))
    status = mdnpbrm_addObjectHeader(pTxData, DNPDEFS_OBJ_60_CLASS_SCANS, 3, qualifier, 0, maxQuantity);

  if(status && (class3 == TMWDEFS_TRUE))
    status = mdnpbrm_addObjectHeader(pTxData, DNPDEFS_OBJ_60_CLASS_SCANS, 4, qualifier, 0, maxQuantity);

  if(status && (class0 == TMWDEFS_TRUE))
    status = mdnpbrm_addObjectHeader(pTxData, DNPDEFS_OBJ_60_CLASS_SCANS, 1, qualifier, 0, maxQuantity);

  if(!status)
  {
    dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
    return(TMWDEFS_NULL);
  }

  /* Send request */
  if(pUserTxData == TMWDEFS_NULL)
  {
    MDNPSESN *pMDNPSession = (MDNPSESN *)pReqDesc->pSession;
    if((pMDNPSession != TMWDEFS_NULL) && (pMDNPSession->unsolRespState == MDNPSESN_UNSOL_STARTUP))
    {
      /* Register callback function so that it can set the unsolRespState when complete */
      ((DNPCHNL_TX_DATA*)pTxData)->pInternalCallback = mdnpsesn_autoIntegrityCallback;
    }

    /* Send request */ 
    if(!_sendFragment(pTxData))
    {
      pTxData = TMWDEFS_NULL;
    }
  }
  return((TMWSESN_TX_DATA *)pTxData);
}

/* function: mdnpbrm_integrityPoll */
TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_integrityPoll(
  MDNPBRM_REQ_DESC *pReqDesc)
{
  TMWSESN_TX_DATA *pTxData;
  TMWTYPES_BOOL msgSpecified = TMWDEFS_TRUE;

  if(pReqDesc->pMsgDescription == TMWDEFS_NULL)
  {
    msgSpecified = TMWDEFS_FALSE;
#if TMWCNFG_SUPPORT_DIAG
    pReqDesc->pMsgDescription = "Integrity Class Poll";
#endif
  } 
 
  pTxData = mdnpbrm_readClass(pReqDesc, TMWDEFS_NULL, DNPDEFS_QUAL_ALL_POINTS, 0,
    TMWDEFS_TRUE, TMWDEFS_TRUE, TMWDEFS_TRUE, TMWDEFS_TRUE);

  if(!msgSpecified)
    pReqDesc->pMsgDescription = TMWDEFS_NULL;
  
  return(pTxData);
}

/* function: mdnpbrm_eventPoll */
TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_eventPoll(
  MDNPBRM_REQ_DESC *pReqDesc)
{
  TMWTYPES_BOOL msgSpecified = TMWDEFS_TRUE;
  TMWSESN_TX_DATA *pTxData;

  if(pReqDesc->pMsgDescription == TMWDEFS_NULL)
  {
    msgSpecified = TMWDEFS_FALSE;
#if TMWCNFG_SUPPORT_DIAG
    pReqDesc->pMsgDescription = "Event Class Poll";
#endif
  }

  pTxData = mdnpbrm_readClass(pReqDesc, TMWDEFS_NULL, DNPDEFS_QUAL_ALL_POINTS, 0,
    TMWDEFS_FALSE, TMWDEFS_TRUE, TMWDEFS_TRUE, TMWDEFS_TRUE);
 
  if(!msgSpecified)
    pReqDesc->pMsgDescription = TMWDEFS_NULL;
  
  return(pTxData);
}

/* function: _binOutFeedbackPoll */
static TMWSESN_TX_DATA * TMWDEFS_LOCAL _binOutFeedbackPoll(
  DNPCHNL_TX_DATA *pTxData)
{
  TMWTYPES_BOOL status;

  /* Build request header */
  mdnpbrm_buildRequestHeader(pTxData, DNPDEFS_FC_READ);

  /* Read class 1, 2, 3 event data and binary output status in that order */
  status = mdnpbrm_addObjectHeader(pTxData, DNPDEFS_OBJ_60_CLASS_SCANS, 2, DNPDEFS_QUAL_ALL_POINTS, 0, 0);
  if(status) status = mdnpbrm_addObjectHeader(pTxData, DNPDEFS_OBJ_60_CLASS_SCANS, 3, DNPDEFS_QUAL_ALL_POINTS, 0, 0);
  if(status) status = mdnpbrm_addObjectHeader(pTxData, DNPDEFS_OBJ_60_CLASS_SCANS, 4, DNPDEFS_QUAL_ALL_POINTS, 0, 0);
  if(status) status = mdnpbrm_addObjectHeader(pTxData, DNPDEFS_OBJ_10_BIN_OUT_STATUSES, 0, DNPDEFS_QUAL_ALL_POINTS, 0, 0);

  if(!status)
  {
    dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
    return(TMWDEFS_NULL);
  }

  /* Send request */
  if(!dnpchnl_sendFragment((TMWSESN_TX_DATA *)pTxData))
  { 
    dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
    return(TMWDEFS_NULL);
  }

  return((TMWSESN_TX_DATA *)pTxData);
}

/* function: mdnpbrm_binOutFeedbackPoll */
TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_binOutFeedbackPoll(
  MDNPBRM_REQ_DESC *pReqDesc)
{
  /* Size is 2 Appl Header + (3 * 5) Object Header */
  DNPCHNL_TX_DATA *pTxData = (DNPCHNL_TX_DATA *)dnpchnl_newTxData(
    pReqDesc->pChannel, pReqDesc->pSession, 17, 0);

  if(pTxData == TMWDEFS_NULL)
  {
    return(TMWDEFS_NULL);
  }

  /* Initialize Transmit Data Structure from Request Descriptor */
  if(!_initTxData(pTxData, pReqDesc, "Operate Binary Output Feedback Poll"))
  {
    dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
    return(TMWDEFS_NULL);
  }

  return(_binOutFeedbackPoll(pTxData));
}

/* function: _anlgOutFeedbackPoll */
static TMWSESN_TX_DATA * TMWDEFS_GLOBAL _anlgOutFeedbackPoll(
  DNPCHNL_TX_DATA *pTxData)
{
  TMWTYPES_BOOL status;

  /* Build request header */
  mdnpbrm_buildRequestHeader(pTxData, DNPDEFS_FC_READ);

  /* Read class 1, 2, 3 event data and analog output status in that order */
  status = mdnpbrm_addObjectHeader(pTxData, DNPDEFS_OBJ_60_CLASS_SCANS, 2, DNPDEFS_QUAL_ALL_POINTS, 0, 0);
  if(status) status = mdnpbrm_addObjectHeader(pTxData, DNPDEFS_OBJ_60_CLASS_SCANS, 3, DNPDEFS_QUAL_ALL_POINTS, 0, 0);
  if(status) status = mdnpbrm_addObjectHeader(pTxData, DNPDEFS_OBJ_60_CLASS_SCANS, 4, DNPDEFS_QUAL_ALL_POINTS, 0, 0);
  if(status) status = mdnpbrm_addObjectHeader(pTxData, DNPDEFS_OBJ_40_ANA_OUT_STATUSES, 0, DNPDEFS_QUAL_ALL_POINTS, 0, 0);

  if(!status)
  {
    dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
    return(TMWDEFS_NULL);
  }

  /* Send request */
  if(!dnpchnl_sendFragment((TMWSESN_TX_DATA *)pTxData))
  { 
    dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
    return(TMWDEFS_NULL);
  }

  return((TMWSESN_TX_DATA *)pTxData);
}

/* function: mdnpbrm_anlgOutFeedbackPoll */
TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_anlgOutFeedbackPoll(
  MDNPBRM_REQ_DESC *pReqDesc)
{
  /* Size is 2 Appl Header + (3 * 5) Object Header */
  DNPCHNL_TX_DATA *pTxData = (DNPCHNL_TX_DATA *)dnpchnl_newTxData(
    pReqDesc->pChannel, pReqDesc->pSession, 17, 0);

  if(pTxData == TMWDEFS_NULL)
  {
    return(TMWDEFS_NULL);
  }

  /* Initialize Transmit Data Structure from Request Descriptor */
  if(!_initTxData(pTxData, pReqDesc, "Operate Analog Output Feedback Poll"))
  {
    dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
    return(TMWDEFS_NULL);
  }

  return(_anlgOutFeedbackPoll(pTxData)); 
}

/* function: _frznCntrFeedbackPoll */
static TMWSESN_TX_DATA * TMWDEFS_GLOBAL _frznCntrFeedbackPoll(
  DNPCHNL_TX_DATA *pTxData)
{
  TMWTYPES_BOOL status;

  /* Build request header */
  mdnpbrm_buildRequestHeader(pTxData, DNPDEFS_FC_READ);

  /* Read class 1, 2, 3 event data and frozen counters in that order */
  status = mdnpbrm_addObjectHeader(pTxData, DNPDEFS_OBJ_60_CLASS_SCANS, 2, DNPDEFS_QUAL_ALL_POINTS, 0, 0);
  if(status) status = mdnpbrm_addObjectHeader(pTxData, DNPDEFS_OBJ_60_CLASS_SCANS, 3, DNPDEFS_QUAL_ALL_POINTS, 0, 0);
  if(status) status = mdnpbrm_addObjectHeader(pTxData, DNPDEFS_OBJ_60_CLASS_SCANS, 4, DNPDEFS_QUAL_ALL_POINTS, 0, 0);
  if(status) status = mdnpbrm_addObjectHeader(pTxData, DNPDEFS_OBJ_21_FROZEN_CNTRS, 0, DNPDEFS_QUAL_ALL_POINTS, 0, 0);
 
  if(!status)
  {
    dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
    return(TMWDEFS_NULL);
  }

  /* Send request */
  if(!dnpchnl_sendFragment((TMWSESN_TX_DATA *)pTxData))
  { 
    dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
    return(TMWDEFS_NULL);
  }

  return((TMWSESN_TX_DATA *)pTxData);
}

/* function: mdnpbrm_frznCntrFeedbackPoll */
TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_frznCntrFeedbackPoll(
  MDNPBRM_REQ_DESC *pReqDesc)
{
  /* Size is 2 Appl Header + (3 * 5) Object Header */
  DNPCHNL_TX_DATA *pTxData = (DNPCHNL_TX_DATA *)dnpchnl_newTxData(
    pReqDesc->pChannel, pReqDesc->pSession, 17, 0);

  if(pTxData == TMWDEFS_NULL)
  {
    return(TMWDEFS_NULL);
  }

  /* Initialize Transmit Data Structure from Request Descriptor */
  if(!_initTxData(pTxData, pReqDesc, "Operate Frozen Counter Feedback Poll"))
  {
    dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
    return(TMWDEFS_NULL);
  }
 
  return(_frznCntrFeedbackPoll(pTxData));
}

/* function: _frznAnlgInFeedbackPoll */
static TMWSESN_TX_DATA * TMWDEFS_GLOBAL _frznAnlgInFeedbackPoll(
  DNPCHNL_TX_DATA *pTxData)
{
  TMWTYPES_BOOL status;

  /* Build request header */
  mdnpbrm_buildRequestHeader(pTxData, DNPDEFS_FC_READ);

  /* Read class 1, 2, 3 event data and frozen counters in that order */
  status = mdnpbrm_addObjectHeader(pTxData, DNPDEFS_OBJ_60_CLASS_SCANS, 2, DNPDEFS_QUAL_ALL_POINTS, 0, 0);
  if(status) status = mdnpbrm_addObjectHeader(pTxData, DNPDEFS_OBJ_60_CLASS_SCANS, 3, DNPDEFS_QUAL_ALL_POINTS, 0, 0);
  if(status) status = mdnpbrm_addObjectHeader(pTxData, DNPDEFS_OBJ_60_CLASS_SCANS, 4, DNPDEFS_QUAL_ALL_POINTS, 0, 0);
  if(status) status = mdnpbrm_addObjectHeader(pTxData, DNPDEFS_OBJ_31_FRZN_ANA_INPUTS, 0, DNPDEFS_QUAL_ALL_POINTS, 0, 0);
 
  if(!status)
  {
    dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
    return(TMWDEFS_NULL);
  }

  /* Send request */
  if(!dnpchnl_sendFragment((TMWSESN_TX_DATA *)pTxData))
  { 
    dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
    return(TMWDEFS_NULL);
  }

  return((TMWSESN_TX_DATA *)pTxData);
}

/* function: mdnpbrm_frznAnlgInFeedbackPoll */
TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_frznAnlgInFeedbackPoll(
  MDNPBRM_REQ_DESC *pReqDesc)
{
  /* Size is 2 Appl Header + (3 * 5) Object Header */
  DNPCHNL_TX_DATA *pTxData = (DNPCHNL_TX_DATA *)dnpchnl_newTxData(
    pReqDesc->pChannel, pReqDesc->pSession, 17, 0);

  if(pTxData == TMWDEFS_NULL)
  {
    return(TMWDEFS_NULL);
  }

  /* Initialize Transmit Data Structure from Request Descriptor */
  if(!_initTxData(pTxData, pReqDesc, "Operate Frozen Analog Input Feedback Poll"))
  {
    dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
    return(TMWDEFS_NULL);
  }
 
  return(_frznAnlgInFeedbackPoll(pTxData));
}

TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_clearIINBit(
  MDNPBRM_REQ_DESC *pReqDesc,
  TMWTYPES_UCHAR index)
{
  DNPCHNL_TX_DATA *pTxData = TMWDEFS_NULL;
  char *pData= TMWDEFS_NULL;
  TMWTYPES_USHORT size = 8; 

#if MDNPDATA_SUPPORT_OBJ120
  if(pReqDesc->authAggressiveMode)
    size += DNPAUTH_AGGRESSIVE_SIZE;
#endif

  /* Size is 2 Appl Header + 5 Object Header + 1 Object Data */ 
  pTxData = (DNPCHNL_TX_DATA *)dnpchnl_newTxData(
    pReqDesc->pChannel, pReqDesc->pSession, size, 0);

  if(pTxData == TMWDEFS_NULL)
  {
    return(TMWDEFS_NULL);
  }
  
  
#if !TMWCNFG_SUPPORT_DIAG
  if(index == DNPDEFS_IIN_RESTART_INDEX)
  {
    pData = "Clear Restart IIN";
  }
  else if(index == DNPDEFS_IIN_NEEDTIME_INDEX)
  {
    pData = "Clear Meed Time IIN";
  }
  else
  {
    pData = "Clear specified IIN bit";
  }
#endif

  /* Initialize Transmit Data Structure from Request Descriptor */
  if(!_initTxData(pTxData, pReqDesc, pData))
  {
    dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
    return(TMWDEFS_NULL);
  }

  /* Build request header */
  mdnpbrm_buildRequestHeader(pTxData, DNPDEFS_FC_WRITE);

#if MDNPDATA_SUPPORT_OBJ120
    if(pReqDesc->authAggressiveMode)
    {
      if(mdnpauth_addAggrRequestStart((TMWSESN_TX_DATA *)pTxData, pReqDesc->authUserNumber) == TMWDEFS_NULL)
      {
        dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
        return(TMWDEFS_NULL);
      }
    }
#endif 

  /* Build object header */
  if(!mdnpbrm_addObjectHeader(pTxData, DNPDEFS_OBJ_80_IIN_BITS, 1, DNPDEFS_QUAL_8BIT_START_STOP, index, index))
  {
    dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
    return(TMWDEFS_NULL);
  }

  /* Add object data */
  pTxData->tmw.pMsgBuf[pTxData->tmw.msgLength++] = 0;
  
  /* Send request */ 
  if(!_sendFragment(pTxData))
  {
    pTxData = TMWDEFS_NULL;
  }
  return((TMWSESN_TX_DATA *)pTxData);
}

/* function: mdnpbrm_clearRestart */
TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_clearRestart(
  MDNPBRM_REQ_DESC *pReqDesc)
{
  return(mdnpbrm_clearIINBit(pReqDesc, DNPDEFS_IIN_RESTART_INDEX));   
}
/* function: mdnpbrm_clearNeedTime */
TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_clearNeedTime(
  MDNPBRM_REQ_DESC *pReqDesc)
{
  return(mdnpbrm_clearIINBit(pReqDesc, DNPDEFS_IIN_NEEDTIME_INDEX));   
}

/* mdnpbrm_coldRestart */
TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_coldRestart(
  MDNPBRM_REQ_DESC *pReqDesc)
{
  DNPCHNL_TX_DATA *pTxData = TMWDEFS_NULL;
  /* Size is 2 Appl Header */
  TMWTYPES_USHORT size = 2; 

#if MDNPDATA_SUPPORT_OBJ120
  if(pReqDesc->authAggressiveMode)
    size += DNPAUTH_AGGRESSIVE_SIZE;
#endif

  pTxData = (DNPCHNL_TX_DATA *)dnpchnl_newTxData(
    pReqDesc->pChannel, pReqDesc->pSession, size, 0);
  
  if(pTxData == TMWDEFS_NULL)
  {
    return(TMWDEFS_NULL);
  }

  /* Initialize Transmit Data Structure from Request Descriptor */
  if(!_initTxData(pTxData, pReqDesc, "Cold Restart"))
  {
    dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
    return(TMWDEFS_NULL);
  }

  /* Build request header */
  mdnpbrm_buildRequestHeader(pTxData, DNPDEFS_FC_COLD_RESTART);

#if MDNPDATA_SUPPORT_OBJ120
    if(pReqDesc->authAggressiveMode)
    {
      if(mdnpauth_addAggrRequestStart((TMWSESN_TX_DATA *)pTxData, pReqDesc->authUserNumber) == TMWDEFS_NULL)
      {
        dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
        return(TMWDEFS_NULL);
      }
    }
#endif 

  /* Send request */ 
  if(!_sendFragment(pTxData))
  {
    pTxData = TMWDEFS_NULL;
  }
  return((TMWSESN_TX_DATA *)pTxData);
}

/* mdnpbrm_warmRestart */
TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_warmRestart(
  MDNPBRM_REQ_DESC *pReqDesc)
{
  DNPCHNL_TX_DATA *pTxData = TMWDEFS_NULL;
  /* Size is 2 Appl Header */
  TMWTYPES_USHORT size = 2; 

#if MDNPDATA_SUPPORT_OBJ120
  if(pReqDesc->authAggressiveMode)
    size += DNPAUTH_AGGRESSIVE_SIZE;
#endif 
  /* allow room for secure authentication aggressive Mode object */  
  pTxData = (DNPCHNL_TX_DATA *)dnpchnl_newTxData(
    pReqDesc->pChannel, pReqDesc->pSession, size, 0);

  if(pTxData == TMWDEFS_NULL)
  {
    return(TMWDEFS_NULL);
  }

  /* Initialize Transmit Data Structure from Request Descriptor */
  if(!_initTxData(pTxData, pReqDesc, "Warm Restart"))
  {
    dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
    return(TMWDEFS_NULL);
  }

  /* Build request header */
  mdnpbrm_buildRequestHeader(pTxData, DNPDEFS_FC_WARM_RESTART);
  
#if MDNPDATA_SUPPORT_OBJ120
    if(pReqDesc->authAggressiveMode)
    {
      if(mdnpauth_addAggrRequestStart((TMWSESN_TX_DATA *)pTxData, pReqDesc->authUserNumber) == TMWDEFS_NULL)
      {
        dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
        return(TMWDEFS_NULL);
      }
    }
#endif 
  /* Send request */ 
  if(!_sendFragment(pTxData))
  {
    pTxData = TMWDEFS_NULL;
  }
  return((TMWSESN_TX_DATA *)pTxData);
}

#if MDNPDATA_SUPPORT_OBJ50_V1
/* function: _delayMeasurement */
static TMWSESN_TX_DATA * TMWDEFS_LOCAL _delayMeasurement(
  MDNPBRM_REQ_DESC *pReqDesc,
  const char *pMsgDescription,
  DNPCHNL_CALLBACK_FUNC pInternalCallback,
  void *pInternalCallbackParam)
{
  DNPCHNL_TX_DATA *pTxData = TMWDEFS_NULL;
  /* use 12 so there is room for delayCallback to call _writeTime */
  TMWTYPES_USHORT size = 12; 

#if MDNPDATA_SUPPORT_OBJ120
  if(pReqDesc->authAggressiveMode)
    size += DNPAUTH_AGGRESSIVE_SIZE;
#endif

  pTxData = (DNPCHNL_TX_DATA *)dnpchnl_newTxData(
    pReqDesc->pChannel, pReqDesc->pSession, size, 0);

  if(pTxData == TMWDEFS_NULL)
  {
    return(TMWDEFS_NULL);
  }
 
  /* Initialize Transmit Data Structure from Request Descriptor */
  if(!_initTxData(pTxData, pReqDesc, pMsgDescription))
  {
    dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
    return(TMWDEFS_NULL);
  }

  /* Build request header */
  mdnpbrm_buildRequestHeader(pTxData, DNPDEFS_FC_DELAY_MEASURE);

  /* Setup internal callback */
  pTxData->pInternalCallback = pInternalCallback;
  pTxData->pInternalCallbackParam = pInternalCallbackParam;

  /* Send request */ 
  if(!_sendFragment(pTxData))
  {
    pTxData = TMWDEFS_NULL;
  }

  return((TMWSESN_TX_DATA *)pTxData);
}

/* function: mdnpbrm_delayMeasurement */
TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_delayMeasurement(
  MDNPBRM_REQ_DESC *pReqDesc)
{
  return(_delayMeasurement(pReqDesc, "Delay Measurement", TMWDEFS_NULL, TMWDEFS_NULL));
}
#endif

#if MDNPDATA_SUPPORT_OBJ50_V1
/* function: mdnpbrm_readTime */
TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_readTime(
  MDNPBRM_REQ_DESC *pReqDesc)
{
  DNPCHNL_TX_DATA *pTxData = TMWDEFS_NULL;
  /* Size is 2 Appl Header + 4 Object Header */ 
  TMWTYPES_USHORT size = 6; 

#if MDNPDATA_SUPPORT_OBJ120
  if(pReqDesc->authAggressiveMode)
    size += DNPAUTH_AGGRESSIVE_SIZE;
#endif

  pTxData = (DNPCHNL_TX_DATA *)dnpchnl_newTxData(
    pReqDesc->pChannel, pReqDesc->pSession, size, 0);
  
  if(pTxData == TMWDEFS_NULL)
  {
    return(TMWDEFS_NULL);
  }

  /* Initialize Transmit Data Structure from Request Descriptor */
  if(!_initTxData(pTxData, pReqDesc, "Read Time"))
  {
    dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
    return(TMWDEFS_NULL);
  }

  /* Build request header */
  mdnpbrm_buildRequestHeader(pTxData, DNPDEFS_FC_READ);

  /* Time Object Header */
  if(!mdnpbrm_addObjectHeader(pTxData, DNPDEFS_OBJ_50_TIME_AND_DATE, 1, DNPDEFS_QUAL_8BIT_LIMITED_QTY, 0, 1))
  {
    dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
    return(TMWDEFS_NULL);
  }

  /* Send request */ 
  if(!_sendFragment(pTxData))
  {
    pTxData = TMWDEFS_NULL;
  }

  return((TMWSESN_TX_DATA *)pTxData);
}
#endif

#if MDNPDATA_SUPPORT_OBJ50_V1

/* function: _writeTime */
static TMWSESN_TX_DATA * TMWDEFS_GLOBAL _writeTime(
  DNPCHNL_TX_DATA *pTxData)
{ 
  /* Build request header */
  mdnpbrm_buildRequestHeader(pTxData, DNPDEFS_FC_WRITE);

#if MDNPDATA_SUPPORT_OBJ120 
  if(pTxData->authAggressiveMode)
  {
    if(mdnpauth_addAggrRequestStart((TMWSESN_TX_DATA *)pTxData, pTxData->authUserNumber) == TMWDEFS_NULL)
    {
      dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
      return(TMWDEFS_NULL);
    }
  }
#endif 

  /* Time Object Header */
  if(!mdnpbrm_addObjectHeader(pTxData, DNPDEFS_OBJ_50_TIME_AND_DATE, 1, DNPDEFS_QUAL_8BIT_LIMITED_QTY, 0, 1))
  {
    dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
    return(TMWDEFS_NULL);
  }

  /* Set 6 time bytes to a known pattern, so the check for duplicate request will work properly */
  pTxData->tmw.pMsgBuf[pTxData->tmw.msgLength++] = 0x11; 
  pTxData->tmw.pMsgBuf[pTxData->tmw.msgLength++] = 0x22; 
  pTxData->tmw.pMsgBuf[pTxData->tmw.msgLength++] = 0x33; 
  pTxData->tmw.pMsgBuf[pTxData->tmw.msgLength++] = 0x44; 
  pTxData->tmw.pMsgBuf[pTxData->tmw.msgLength++] = 0x55; 
  pTxData->tmw.pMsgBuf[pTxData->tmw.msgLength++] = 0x66;
  
  /* Setup to write time before fragment is transmitted */
  pTxData->tmw.txFlags |= TMWSESN_TXFLAGS_STORE_DNP_TIME;

  /* Send request */ 
  if(!_sendFragment(pTxData))
  {
    pTxData = TMWDEFS_NULL;
  }

  return((TMWSESN_TX_DATA *)pTxData);
}

/* function: mdnpbrm_writeTime */
TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_writeTime(
  MDNPBRM_REQ_DESC *pReqDesc)
{ 
  /* Size is 2 Appl Header + 4 Object Header + 6 Object Data     */
  /* allow room for secure authentication aggressive mode object */
  DNPCHNL_TX_DATA *pTxData = (DNPCHNL_TX_DATA *)dnpchnl_newTxData(
    pReqDesc->pChannel, pReqDesc->pSession, 12+DNPAUTH_AGGRESSIVE_SIZE, 0);

  if(pTxData == TMWDEFS_NULL)
  {
    return(TMWDEFS_NULL);
  }

  /* Initialize Transmit Data Structure from Request Descriptor */
  if(!_initTxData(pTxData, pReqDesc, "Time Synchronization"))
  {
    dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
    return(TMWDEFS_NULL);
  }

  return(_writeTime(pTxData));
}
#endif

#if MDNPDATA_SUPPORT_OBJ50_V3
/* function: _recordCurrentTime */
static TMWSESN_TX_DATA * TMWDEFS_LOCAL _recordCurrentTime(
  MDNPBRM_REQ_DESC *pReqDesc,
  const char *pMsgDescription,
  DNPCHNL_CALLBACK_FUNC pInternalCallback,
  void *pInternalCallbackParam)
{
  /* Size of this request is 2 Appl Header  
   * Use 12 so _recordTimeCallbackFunc() has enough room for _writeRecordedTime() 
   * Also allow room for aggressive mode objects
   */
  DNPCHNL_TX_DATA *pTxData = (DNPCHNL_TX_DATA *)dnpchnl_newTxData(
    pReqDesc->pChannel, pReqDesc->pSession, 12+DNPAUTH_AGGRESSIVE_SIZE, 0);
  
  if(pTxData == TMWDEFS_NULL)
  {
    return(TMWDEFS_NULL);
  }

  /* Initialize Transmit Data Structure from Request Descriptor */
  if(!_initTxData(pTxData, pReqDesc, pMsgDescription))
  {
    dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
    return(TMWDEFS_NULL);
  }

  /* Build request header */
  mdnpbrm_buildRequestHeader(pTxData, DNPDEFS_FC_RECORD_CURRENT_TIME);
  
#if MDNPDATA_SUPPORT_OBJ120 
  if(pTxData->authAggressiveMode)
  {
    if(mdnpauth_addAggrRequestStart((TMWSESN_TX_DATA *)pTxData, pTxData->authUserNumber) == TMWDEFS_NULL)
    {
      dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
      return(TMWDEFS_NULL);
    }
  }
#endif 

  /* Setup internal callback */
  pTxData->pInternalCallback = pInternalCallback;
  pTxData->pInternalCallbackParam = pInternalCallbackParam;

  /* Set flag to save time last byte is transmitted */
  pTxData->tmw.txFlags |= TMWSESN_TXFLAGS_SAVE_LAST_BYTE_TIME;

  /* Send request */ 
  if(!_sendFragment(pTxData))
  {
    pTxData = TMWDEFS_NULL;
  }

  return((TMWSESN_TX_DATA *)pTxData);
}

/* function: mdnpbrm_recordCurrentTime */
TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_recordCurrentTime(
  MDNPBRM_REQ_DESC *pReqDesc)
{
  return(_recordCurrentTime(pReqDesc, 
    "Record Current Time", TMWDEFS_NULL, TMWDEFS_NULL));
}
#endif

#if MDNPDATA_SUPPORT_OBJ50_V3
/* function:_writeRecordedTime */
static TMWSESN_TX_DATA * TMWDEFS_GLOBAL _writeRecordedTime(
  DNPCHNL_TX_DATA *pTxData,
  TMWTYPES_BOOL broadcast)
{
  TMWTYPES_MS_SINCE_70 msSince70;
  TMWDTIME *pDateTime;
    
  /* Build request header */
  mdnpbrm_buildRequestHeader(pTxData, DNPDEFS_FC_WRITE);

#if MDNPDATA_SUPPORT_OBJ120 
  if(pTxData->authAggressiveMode)
  {
    if(mdnpauth_addAggrRequestStart((TMWSESN_TX_DATA *)pTxData, pTxData->authUserNumber) == TMWDEFS_NULL)
    {
      dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
      return(TMWDEFS_NULL);
    }
  }
#endif 

  /* Time Object Header */
  if(!mdnpbrm_addObjectHeader(pTxData, DNPDEFS_OBJ_50_TIME_AND_DATE, 3, DNPDEFS_QUAL_8BIT_LIMITED_QTY, 0, 1))
  {
    dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
    return(TMWDEFS_NULL);
  }

  /* If this is a broadcast message get last recorded time from channel, else
   *   get it from session
   */
  if(broadcast)
  {
    pTxData->tmw.txFlags |= (TMWSESN_TXFLAGS_NO_RESPONSE | TMWSESN_TXFLAGS_BROADCAST);
    pDateTime = &pTxData->tmw.pChannel->lastByteTime;
  }
  else
  {
    MDNPSESN *pMDNPSession = (MDNPSESN *)pTxData->tmw.pSession;
    pDateTime = &pMDNPSession->lastByteTime;
  }

  /* Store time of last record current time request */
  dnpdtime_dateTimeToMSSince70(&msSince70, pDateTime);
  dnpdtime_writeMsSince70(pTxData->tmw.pMsgBuf + pTxData->tmw.msgLength, &msSince70);
  pTxData->tmw.msgLength += 6;

  /* Send request */ 
  if(!_sendFragment(pTxData))
  {
    pTxData = TMWDEFS_NULL;
  }

  return((TMWSESN_TX_DATA *)pTxData);
}

/* function: mdnpbrm_writeRecordedTime */
TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_writeRecordedTime(
  MDNPBRM_REQ_DESC *pReqDesc)
{
  DNPCHNL_TX_DATA *pTxData;
  TMWTYPES_BOOL broadcast = TMWDEFS_FALSE;
    
  /* Size is 2 Appl Header + 4 Object Header + 6 Object Data */
  pTxData = (DNPCHNL_TX_DATA *)dnpchnl_newTxData(
    pReqDesc->pChannel, pReqDesc->pSession, 12+DNPAUTH_AGGRESSIVE_SIZE, 0);
  
  if(pTxData == TMWDEFS_NULL)
  {
    return(TMWDEFS_NULL);
  }

  /* Initialize Transmit Data Structure from Request Descriptor */
  if(!_initTxData(pTxData, pReqDesc, "Time Synchronization"))
  {
    dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
    return(TMWDEFS_NULL);
  }

  if(pReqDesc->pSession == TMWDEFS_NULL)
    broadcast = TMWDEFS_TRUE;

  return(_writeRecordedTime(pTxData, broadcast));
}
#endif

#if MDNPDATA_SUPPORT_OBJ50_V1 || MDNPDATA_SUPPORT_OBJ50_V3
/* function: mdnpbrm_timeSync */
TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_timeSync(
  MDNPBRM_REQ_DESC *pReqDesc,
  MDNPBRM_SYNC_TYPE type,
  TMWTYPES_BOOL measureDelay)
{
  switch(type)
  {
#if MDNPDATA_SUPPORT_OBJ50_V1
  case MDNPBRM_SYNC_TYPE_SERIAL:
    if(measureDelay)
    {
      /* broadcast delay measurement makes no sense
       * there would be no response allowed from slave
       */
      if(pReqDesc->pSession == TMWDEFS_NULL)
      {
        MDNPDIAG_ERROR(pReqDesc->pChannel, TMWDEFS_NULL, MDNPDIAG_INV_BROADCAST);
        return(TMWDEFS_NULL);
      }
      return(_delayMeasurement(pReqDesc, 
        "Delay Measurement Preceding Time Synchronization", 
        _delayCallbackFunc, pReqDesc->pSession));
    }
    else
      return(mdnpbrm_writeTime(pReqDesc));
#endif

#if MDNPDATA_SUPPORT_OBJ50_V3
  case MDNPBRM_SYNC_TYPE_LAN: 
    /* broadcast recordTime is suggested in 1815-2012
     * even though no response to broadcast is allowed from outstation
     */
    return(_recordCurrentTime(pReqDesc, 
      "Record Current Time Preceding Time Synchronization", 
      _recordTimeCallbackFunc, pReqDesc->pSession));
#endif
  }

  return(TMWDEFS_NULL);
}
#endif

#if MDNPDATA_SUPPORT_OBJ50_V4
/* function: mdnpbrm_writeTime */
TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_writeIndexedTime(
  MDNPBRM_REQ_DESC *pReqDesc,
  DNPCHNL_TX_DATA *pUserTxData,
  TMWTYPES_UCHAR qualifier,
  TMWTYPES_UCHAR numObjects,
  MDNPBRM_INDEXEDTIME_INFO *pINDEXEDTIMEInfo)
{
  TMWTYPES_MS_SINCE_70 msSince70;
  DNPCHNL_TX_DATA *pTxData;
  int i;

  TMWTYPES_USHORT size = (TMWTYPES_USHORT)(7 + (numObjects * 12));
#if MDNPDATA_SUPPORT_OBJ120
  if (pReqDesc->authAggressiveMode)
    size += DNPAUTH_AGGRESSIVE_SIZE;
#endif

  if ((qualifier != DNPDEFS_QUAL_8BIT_INDEX)
    && (qualifier != DNPDEFS_QUAL_16BIT_INDEX)
    && (qualifier != DNPDEFS_QUAL_8BIT_START_STOP)
    && (qualifier != DNPDEFS_QUAL_16BIT_START_STOP))
  {
    MDNPDIAG_ERROR(pReqDesc->pChannel, pReqDesc->pSession, MDNPDIAG_INV_QUALIFIER);
    return(TMWDEFS_NULL);
  }

  if (pUserTxData == TMWDEFS_NULL)
  {
    if (qualifier == DNPDEFS_QUAL_8BIT_INDEX)
    {
      /* Max size is 2 Appl Header + 5 Object Header + (numObjects * (1 Index + 11 Data)) */
      pTxData = (DNPCHNL_TX_DATA *)dnpchnl_newTxData(
        pReqDesc->pChannel, pReqDesc->pSession, size, 0);
    }
    else
    {
      /* Max size is 2 Appl Header + 5 Object Header + (numObjects * (2 Index + 11 Data))
       * 1 octet per crob larger than above
       */
      size += numObjects;
      pTxData = (DNPCHNL_TX_DATA *)dnpchnl_newTxData(
        pReqDesc->pChannel, pReqDesc->pSession, size, 0);
    }

    if (pTxData == TMWDEFS_NULL)
    {
      return(TMWDEFS_NULL);
    }

    /* Initialize Transmit Data Structure from Request Descriptor */
    if (!_initTxData(pTxData, pReqDesc, "Write Indexed Absolute Time and Long Interval"))
    {
      dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
      return(TMWDEFS_NULL);
    }

    /* Build request header */
    mdnpbrm_buildRequestHeader(pTxData, DNPDEFS_FC_WRITE);

#if MDNPDATA_SUPPORT_OBJ120
    if (pReqDesc->authAggressiveMode)
    {
      if (mdnpauth_addAggrRequestStart((TMWSESN_TX_DATA *)pTxData, pReqDesc->authUserNumber) == TMWDEFS_NULL)
      {
        dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
        return(TMWDEFS_NULL);
      }
    }
#endif 
  }
  else
  {
    pTxData = pUserTxData;
    if (qualifier == DNPDEFS_QUAL_8BIT_INDEX)
    {
      /* Max size is 5 Object Header + (numCROBs * (1 Index + 11 Data)) */
      if (pUserTxData->tmw.msgLength + 5 + (numObjects * 12) >= pUserTxData->tmw.maxLength)
      {
        MDNPDIAG_ERROR(pReqDesc->pChannel, pReqDesc->pSession, MDNPDIAG_OBJ_LENGTH);
        pTxData = TMWDEFS_NULL;
      }
    }
    else
    {
      /* Max size is 5 Object Header + (numCROBs * (2 Index + 11 Data)) */
      if (pUserTxData->tmw.msgLength + 5 + (numObjects * 13) >= pUserTxData->tmw.maxLength)
      {
        MDNPDIAG_ERROR(pReqDesc->pChannel, pReqDesc->pSession, MDNPDIAG_OBJ_LENGTH);
        pTxData = TMWDEFS_NULL;
      }
    }

    if (pUserTxData->tmw.pMsgBuf[1] != DNPDEFS_FC_WRITE)
    {
      MDNPDIAG_ERROR(pReqDesc->pChannel, pReqDesc->pSession, MDNPDIAG_FC);
      pTxData = TMWDEFS_NULL;
    }

    if (pTxData == TMWDEFS_NULL)
    {
      dnpchnl_freeTxData((TMWSESN_TX_DATA *)pUserTxData);
      return(TMWDEFS_NULL);
    }
  }

  /* Add object header */
  if ((qualifier == DNPDEFS_QUAL_8BIT_START_STOP)
    || (qualifier == DNPDEFS_QUAL_16BIT_START_STOP))
  {
    TMWTYPES_USHORT start = pINDEXEDTIMEInfo[0].pointNumber;
    if (!mdnpbrm_addObjectHeader(pTxData, DNPDEFS_OBJ_50_TIME_AND_DATE, 4, qualifier, start, start + numObjects - 1))
    {
      dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
      return(TMWDEFS_NULL);
    }
  }
  else
  {
    if (!mdnpbrm_addObjectHeader(pTxData, DNPDEFS_OBJ_50_TIME_AND_DATE, 4, qualifier, 0, numObjects))
    {
      dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
      return(TMWDEFS_NULL);
    }
  }

  for (i = 0; i < numObjects; i++)
  {
    /* Index */
    if (qualifier == DNPDEFS_QUAL_16BIT_INDEX)
    {
      tmwtarg_store16(&pINDEXEDTIMEInfo[i].pointNumber, pTxData->tmw.pMsgBuf + pTxData->tmw.msgLength);
      pTxData->tmw.msgLength += 2;
    }
    else if (qualifier == DNPDEFS_QUAL_8BIT_INDEX)
    {
      if (pINDEXEDTIMEInfo[i].pointNumber <= 0xff)
      {
        pTxData->tmw.pMsgBuf[pTxData->tmw.msgLength++] = (TMWTYPES_UCHAR)pINDEXEDTIMEInfo[i].pointNumber;
      }
      else
      {
        MDNPDIAG_ERROR(pReqDesc->pChannel, pReqDesc->pSession, MDNPDIAG_INV_QUALIFIER_POINT);
        dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
        return(TMWDEFS_NULL);
      }
    }

    /* Convert to milliseconds since 1970 */
    dnpdtime_dateTimeToMSSince70(&msSince70, &pINDEXEDTIMEInfo[i].indexedTime);

    /* Write into message */
    dnpdtime_writeMsSince70(&pTxData->tmw.pMsgBuf[pTxData->tmw.msgLength], &msSince70);
    pTxData->tmw.msgLength = (TMWTYPES_USHORT)(pTxData->tmw.msgLength + 6);

    /* Get interval count */
    tmwtarg_store32((TMWTYPES_ULONG *)&pINDEXEDTIMEInfo[i].intervalCount, &pTxData->tmw.pMsgBuf[pTxData->tmw.msgLength]);
    pTxData->tmw.msgLength = (TMWTYPES_USHORT)(pTxData->tmw.msgLength + 4);

    /* Get interval unit */
    tmwtarg_store8((TMWTYPES_BYTE *)&pINDEXEDTIMEInfo[i].intervalUnits, &pTxData->tmw.pMsgBuf[pTxData->tmw.msgLength]);
    pTxData->tmw.msgLength = (TMWTYPES_USHORT)(pTxData->tmw.msgLength + 1);

    DNPDIAG_SHOW_INDEXED_TIME(pReqDesc->pSession, pINDEXEDTIMEInfo[i].pointNumber, &pINDEXEDTIMEInfo[i].indexedTime, pINDEXEDTIMEInfo[i].intervalCount, pINDEXEDTIMEInfo[i].intervalUnits, 0);
  }

  /* Send request */
  if (pUserTxData == TMWDEFS_NULL)
  {
    /* Send request */
    if (!_sendFragment(pTxData))
    {
      pTxData = TMWDEFS_NULL;
    }
  }
  return((TMWSESN_TX_DATA *)pTxData);
}
#endif

/* function: mdnpbrm_unsolEnable */
TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_unsolEnable(
  MDNPBRM_REQ_DESC *pReqDesc,
  TMWTYPES_BOOL class1,
  TMWTYPES_BOOL class2,
  TMWTYPES_BOOL class3)
{
  DNPCHNL_TX_DATA *pTxData;
  TMWTYPES_USHORT size;
  TMWTYPES_BOOL status;

  /* Size is 2 Appl Header + (3 * # of classes selected) Object Header */
  size = 2;
  if(class1 == TMWDEFS_TRUE) size += 3;
  if(class2 == TMWDEFS_TRUE) size += 3;
  if(class3 == TMWDEFS_TRUE) size += 3;

#if MDNPDATA_SUPPORT_OBJ120
  if(pReqDesc->authAggressiveMode)
    size += DNPAUTH_AGGRESSIVE_SIZE;
#endif

  /* Allocate Application Transmit Data Structure */
  pTxData = (DNPCHNL_TX_DATA *)dnpchnl_newTxData(
    pReqDesc->pChannel, pReqDesc->pSession, size, 0);
  
  if(pTxData == TMWDEFS_NULL)
  {
    return(TMWDEFS_NULL);
  }

  /* Initialize Transmit Data Structure from Request Descriptor */
  if(!_initTxData(pTxData, pReqDesc, "Enable Unsolicited"))
  {
    dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
    return(TMWDEFS_NULL);
  }

  /* Build request header */
  mdnpbrm_buildRequestHeader(pTxData, DNPDEFS_FC_ENABLE_UNSOL);
  
#if MDNPDATA_SUPPORT_OBJ120
    if(pReqDesc->authAggressiveMode)
    {
      if(mdnpauth_addAggrRequestStart((TMWSESN_TX_DATA *)pTxData, pReqDesc->authUserNumber) == TMWDEFS_NULL)
      {
        /* If this fails, still send it using non AggressiveMode to prevent this from failing in mdnpsesn_openSession and never being sent again. */
        pReqDesc->authAggressiveMode = TMWDEFS_FALSE;
      }
    }
#endif 

  /* Read requested class in order */
  status = TMWDEFS_TRUE;

  if(status && (class1 == TMWDEFS_TRUE))
    status = mdnpbrm_addObjectHeader(pTxData, DNPDEFS_OBJ_60_CLASS_SCANS, 2, DNPDEFS_QUAL_ALL_POINTS, 0, 0);

  if(status && (class2 == TMWDEFS_TRUE))
    status = mdnpbrm_addObjectHeader(pTxData, DNPDEFS_OBJ_60_CLASS_SCANS, 3, DNPDEFS_QUAL_ALL_POINTS, 0, 0);

  if(status && (class3 == TMWDEFS_TRUE))
    status = mdnpbrm_addObjectHeader(pTxData, DNPDEFS_OBJ_60_CLASS_SCANS, 4, DNPDEFS_QUAL_ALL_POINTS, 0, 0);

  if(!status)
  {
    dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
    return(TMWDEFS_NULL);
  }

  /* Send request */ 
  if(!_sendFragment(pTxData))
  {
    pTxData = TMWDEFS_NULL;
  } 
  else
  {
    /* Force state to allow unsolicited responses since the enable may not be sent 
     * automatically by autoRequestMask
     */
    MDNPSESN *pMDNPSession = (MDNPSESN *)pReqDesc->pSession;
    if((pMDNPSession != TMWDEFS_NULL) && (pMDNPSession->unsolRespState == MDNPSESN_UNSOL_STARTUP))
    {
      pMDNPSession->unsolRespState = MDNPSESN_UNSOL_FIRSTUR;
    }
  }

  return((TMWSESN_TX_DATA *)pTxData);
}

/* function: mdnpbrm_unsolDisable */
TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_unsolDisable(
  MDNPBRM_REQ_DESC *pReqDesc,
  TMWTYPES_BOOL class1,
  TMWTYPES_BOOL class2,
  TMWTYPES_BOOL class3)
{
  DNPCHNL_TX_DATA *pTxData;
  TMWTYPES_USHORT size;
  TMWTYPES_BOOL status;

  /* Size is 2 Appl Header + (3 * # of classes selected) Object Header */
  size = 2;
  if(class1 == TMWDEFS_TRUE) size += 3;
  if(class2 == TMWDEFS_TRUE) size += 3;
  if(class3 == TMWDEFS_TRUE) size += 3;

#if MDNPDATA_SUPPORT_OBJ120
  if(pReqDesc->authAggressiveMode)
    size += DNPAUTH_AGGRESSIVE_SIZE;
#endif

  /* Allocate Application Transmit Data Structure */
  pTxData = (DNPCHNL_TX_DATA *)dnpchnl_newTxData(
    pReqDesc->pChannel, pReqDesc->pSession, size, 0);

  if(pTxData == TMWDEFS_NULL)
  {
    return(TMWDEFS_NULL);
  }

  /* Initialize Transmit Data Structure from Request Descriptor */
  if(!_initTxData(pTxData, pReqDesc, "Disable Unsolicited"))
  {
    dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
    return(TMWDEFS_NULL);
  }

  /* Build request header */
  mdnpbrm_buildRequestHeader(pTxData, DNPDEFS_FC_DISABLE_UNSOL);
  
#if MDNPDATA_SUPPORT_OBJ120
    if(pReqDesc->authAggressiveMode)
    {
      if(mdnpauth_addAggrRequestStart((TMWSESN_TX_DATA *)pTxData, pReqDesc->authUserNumber) == TMWDEFS_NULL)
      {
        /* If this fails, still send it using non AggressiveMode to prevent this from failing in mdnpsesn_openSession and never being sent again. */
        pReqDesc->authAggressiveMode = TMWDEFS_FALSE;
      }
    }
#endif 

  /* Read requested class in order */
  status = TMWDEFS_TRUE;

  if(status && (class1 == TMWDEFS_TRUE))
    status = mdnpbrm_addObjectHeader(pTxData, DNPDEFS_OBJ_60_CLASS_SCANS, 2, DNPDEFS_QUAL_ALL_POINTS, 0, 0);

  if(status && (class2 == TMWDEFS_TRUE))
    status = mdnpbrm_addObjectHeader(pTxData, DNPDEFS_OBJ_60_CLASS_SCANS, 3, DNPDEFS_QUAL_ALL_POINTS, 0, 0);

  if(status && (class3 == TMWDEFS_TRUE))
    status = mdnpbrm_addObjectHeader(pTxData, DNPDEFS_OBJ_60_CLASS_SCANS, 4, DNPDEFS_QUAL_ALL_POINTS, 0, 0);

  if(!status)
  {
    dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
    return(TMWDEFS_NULL);
  }

  /* Send request */ 
  if(!_sendFragment(pTxData))
  {
    pTxData = TMWDEFS_NULL;
  }

  return((TMWSESN_TX_DATA *)pTxData);
}

/* function: mdnpbrm_assignClass */
TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_assignClass(
  MDNPBRM_REQ_DESC *pReqDesc,
  DNPCHNL_TX_DATA *pUserTxData,
  TMWDEFS_CLASS_MASK classMask,
  TMWTYPES_UCHAR group,
  TMWTYPES_UCHAR qualifier,
  TMWTYPES_USHORT start,
  TMWTYPES_USHORT stopOrQty,
  TMWTYPES_USHORT *pPoints)
{
  DNPCHNL_TX_DATA *pTxData;
  TMWTYPES_BOOL status;
  TMWTYPES_USHORT size;

  if(pUserTxData == TMWDEFS_NULL)
  {
    /* Size is 2 Appl Header + 3 Class Object Header + 7 Data Object Header 
     * if using 8/16 bit index allow room for points  
     */
    size = 12;
    if(qualifier == DNPDEFS_QUAL_8BIT_INDEX)
      size = (TMWTYPES_USHORT)(size + stopOrQty);
    else if(qualifier == DNPDEFS_QUAL_16BIT_INDEX)
      size = (TMWTYPES_USHORT)(size + stopOrQty *2);
    
#if MDNPDATA_SUPPORT_OBJ120
    if(pReqDesc->authAggressiveMode)
      size += DNPAUTH_AGGRESSIVE_SIZE;
#endif

    pTxData = (DNPCHNL_TX_DATA *)dnpchnl_newTxData(
      pReqDesc->pChannel, pReqDesc->pSession, size, 0);

    if(pTxData == TMWDEFS_NULL)
    {
      return(TMWDEFS_NULL);
    }

    /* Initialize Transmit Data Structure from Request Descriptor */
    if(!_initTxData(pTxData, pReqDesc, "Assign Class"))
    {
      dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
      return(TMWDEFS_NULL);
    }

    /* Build request header */
    mdnpbrm_buildRequestHeader(pTxData, DNPDEFS_FC_ASSIGN_CLASS);
    
#if MDNPDATA_SUPPORT_OBJ120
    if(pReqDesc->authAggressiveMode)
    {
      if(mdnpauth_addAggrRequestStart((TMWSESN_TX_DATA *)pTxData, pReqDesc->authUserNumber) == TMWDEFS_NULL)
      {
        dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
        return(TMWDEFS_NULL);
      }
    }
#endif 
  }
  else
  {
    pTxData = pUserTxData;
    if(pUserTxData->tmw.msgLength + 10 >= pUserTxData->tmw.maxLength)
    {
      MDNPDIAG_ERROR(pReqDesc->pChannel, pReqDesc->pSession, MDNPDIAG_OBJ_LENGTH);
      pTxData = TMWDEFS_NULL;
    }

    if(pUserTxData->tmw.pMsgBuf[1] != DNPDEFS_FC_ASSIGN_CLASS)
    {
      MDNPDIAG_ERROR(pReqDesc->pChannel, pReqDesc->pSession, MDNPDIAG_FC);
      pTxData = TMWDEFS_NULL;
    }

    if(pTxData == TMWDEFS_NULL)
    {
      dnpchnl_freeTxData((TMWSESN_TX_DATA *)pUserTxData);
      return(TMWDEFS_NULL);
    }
  }

  /* Class Object Header */
  switch(classMask)
  {
  case TMWDEFS_CLASS_MASK_ONE:
    status = mdnpbrm_addObjectHeader(pTxData, DNPDEFS_OBJ_60_CLASS_SCANS, 2, DNPDEFS_QUAL_ALL_POINTS, 0, 0);
    break;

  case TMWDEFS_CLASS_MASK_TWO:
    status = mdnpbrm_addObjectHeader(pTxData, DNPDEFS_OBJ_60_CLASS_SCANS, 3, DNPDEFS_QUAL_ALL_POINTS, 0, 0);
    break;

  case TMWDEFS_CLASS_MASK_THREE:
    status = mdnpbrm_addObjectHeader(pTxData, DNPDEFS_OBJ_60_CLASS_SCANS, 4, DNPDEFS_QUAL_ALL_POINTS, 0, 0);
    break;

  case TMWDEFS_CLASS_MASK_NONE:
    status = mdnpbrm_addObjectHeader(pTxData, DNPDEFS_OBJ_60_CLASS_SCANS, 1, DNPDEFS_QUAL_ALL_POINTS, 0, 0);
    break;

  /* This also allows multiple DOHs to be added to a request without adding a new COH */
  case TMWDEFS_CLASS_MASK_NOTCLASS0:
    status = TMWDEFS_TRUE;
    break;

  default:
    dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
    return(TMWDEFS_NULL);
  }

  if(!status)
  {
    dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
    return(TMWDEFS_NULL);
  }

  /* Data Object Header */
  if(!mdnpbrm_addObjectHeader(pTxData, group, 0, qualifier, start, stopOrQty))
  {
    dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
    return(TMWDEFS_NULL);
  }

  if((qualifier == DNPDEFS_QUAL_8BIT_INDEX) || (qualifier == DNPDEFS_QUAL_16BIT_INDEX))
  {
    int i;
    /* Add point numbers */
    for(i = 0; i < stopOrQty; i++)
    {
      if(qualifier == DNPDEFS_QUAL_8BIT_INDEX)
      {
        pTxData->tmw.pMsgBuf[pTxData->tmw.msgLength++] = (TMWTYPES_UCHAR)pPoints[i];
      }
      else
      {
        tmwtarg_store16(&pPoints[i], pTxData->tmw.pMsgBuf + pTxData->tmw.msgLength);
        pTxData->tmw.msgLength += 2;
      }
    }
  }

  /* Send request */
  if(pUserTxData == TMWDEFS_NULL)
  {
    if(!_sendFragment(pTxData))
    {
      pTxData = TMWDEFS_NULL;
    }
  }

  return((TMWSESN_TX_DATA *)pTxData);
}

/* function: mdnpbrm_readGroup */
TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_readGroup(
  MDNPBRM_REQ_DESC *pReqDesc,
  DNPCHNL_TX_DATA *pUserTxData,
  TMWTYPES_UCHAR group,
  TMWTYPES_UCHAR variation,
  TMWTYPES_UCHAR qualifier,
  TMWTYPES_USHORT start,
  TMWTYPES_USHORT stopOrQty)
{
  DNPCHNL_TX_DATA *pTxData;

  if(pUserTxData == TMWDEFS_NULL)
  {
    /* Size is 2 Appl Header + 7 Object Header */
    /* allow room for secure authentication aggressive mode object */
    pTxData = (DNPCHNL_TX_DATA *)dnpchnl_newTxData(
      pReqDesc->pChannel, pReqDesc->pSession, 9+DNPAUTH_AGGRESSIVE_SIZE, 0);

    if(pTxData == TMWDEFS_NULL)
    {
      return(TMWDEFS_NULL);
    }

    /* Initialize Transmit Data Structure from Request Descriptor */
    if(!_initTxData(pTxData, pReqDesc, "Read Group"))
    {
      dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
      return(TMWDEFS_NULL);
    }

    /* Build request header */
    mdnpbrm_buildRequestHeader(pTxData, DNPDEFS_FC_READ);  

#if MDNPDATA_SUPPORT_OBJ120
    if(pReqDesc->authAggressiveMode)
    {
      if(mdnpauth_addAggrRequestStart((TMWSESN_TX_DATA *)pTxData, pReqDesc->authUserNumber) == TMWDEFS_NULL)
      {
        dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
        return(TMWDEFS_NULL);
      }
    }
#endif 
  }
  else
  {
    if(pUserTxData->tmw.pMsgBuf[1] != DNPDEFS_FC_READ)
    {
      MDNPDIAG_ERROR(pReqDesc->pChannel, pReqDesc->pSession, MDNPDIAG_FC);
      dnpchnl_freeTxData((TMWSESN_TX_DATA *)pUserTxData);
      return(TMWDEFS_NULL);
    }

    pTxData = pUserTxData;
  }

  /* Object Header */
  if(!mdnpbrm_addObjectHeader(pTxData, group, variation, qualifier, start, stopOrQty))
  {
    dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
    return(TMWDEFS_NULL);
  }

  /* Send request */
  if(pUserTxData == TMWDEFS_NULL)
  {
    if(!_sendFragment(pTxData))
    {
      pTxData = TMWDEFS_NULL;
    }
  }

  return((TMWSESN_TX_DATA *)pTxData);
}

/* function: mdnpbrm_readPoints */
TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_readPoints(
  MDNPBRM_REQ_DESC *pReqDesc,
  DNPCHNL_TX_DATA *pUserTxData,
  TMWTYPES_UCHAR group, 
  TMWTYPES_UCHAR variation, 
  TMWTYPES_UCHAR qualifier,
  TMWTYPES_USHORT *pPoints,
  TMWTYPES_USHORT numPoints)
{
  DNPCHNL_TX_DATA *pTxData;
  int i;
  
  /* Validate qualifier */
  if((qualifier != DNPDEFS_QUAL_8BIT_INDEX)
    && (qualifier != DNPDEFS_QUAL_16BIT_INDEX))
  {
    if(pUserTxData != TMWDEFS_NULL)
      dnpchnl_freeTxData((TMWSESN_TX_DATA *)pUserTxData);

    MDNPDIAG_ERROR(pReqDesc->pChannel, pReqDesc->pSession, MDNPDIAG_INV_QUALIFIER);
    return(TMWDEFS_NULL);
  }

  if(qualifier == DNPDEFS_QUAL_8BIT_INDEX)
  {
    for(i = 0; i < numPoints; i++)
    {
      if(pPoints[i] > 255)
      {
        if(pUserTxData != TMWDEFS_NULL)
          dnpchnl_freeTxData((TMWSESN_TX_DATA *)pUserTxData);

        MDNPDIAG_ERROR(pReqDesc->pChannel, pReqDesc->pSession, MDNPDIAG_INV_8BIT_POINT);
        return(TMWDEFS_NULL);
      }
    }
  }

  if(pUserTxData == TMWDEFS_NULL)
  {
    TMWTYPES_USHORT size = 0;

#if MDNPDATA_SUPPORT_OBJ120
    if(pReqDesc->authAggressiveMode)
      size += DNPAUTH_AGGRESSIVE_SIZE;
#endif

    if(qualifier == DNPDEFS_QUAL_8BIT_INDEX)
    {
      /* Size is 2 Appl Header + 5 Object Header + numPoints Index */
      size += (7 + numPoints);
      pTxData = (DNPCHNL_TX_DATA *)dnpchnl_newTxData(
        pReqDesc->pChannel, pReqDesc->pSession, size, 0);
    }
    else
    {
      /* Size is 2 Appl Header + 5 Object Header + (numPoints * 2) Index */
      size += (7 + (numPoints * 2));
      pTxData = (DNPCHNL_TX_DATA *)dnpchnl_newTxData(
        pReqDesc->pChannel, pReqDesc->pSession, size, 0);
    }

    if(pTxData == TMWDEFS_NULL)
    {
      return(TMWDEFS_NULL);
    }

    /* Initialize Transmit Data Structure from Request Descriptor */
    if(!_initTxData(pTxData, pReqDesc, "Read Point"))
    {
      dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
      return(TMWDEFS_NULL);
    }

    /* Build request header */
    mdnpbrm_buildRequestHeader(pTxData, DNPDEFS_FC_READ);
    
#if MDNPDATA_SUPPORT_OBJ120
    if(pReqDesc->authAggressiveMode)
    {
      if(mdnpauth_addAggrRequestStart((TMWSESN_TX_DATA *)pTxData, pReqDesc->authUserNumber) == TMWDEFS_NULL)
      {
        dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
        return(TMWDEFS_NULL);
      }
    }
#endif 
  }
  else
  {
    pTxData = pUserTxData;
    if(qualifier == DNPDEFS_QUAL_8BIT_INDEX)
    {
      /* Size is 5 Object Header + numPoints Index */
      if(pUserTxData->tmw.msgLength + 5 + numPoints >= pUserTxData->tmw.maxLength)
      {
        MDNPDIAG_ERROR(pReqDesc->pChannel, pReqDesc->pSession, MDNPDIAG_OBJ_LENGTH);
        pTxData = TMWDEFS_NULL;
      }
    }
    else
    {
      /* Size is 5 Object Header + numPoints * 2 Index */
      if(pUserTxData->tmw.msgLength + 5 + (numPoints * 2) >= pUserTxData->tmw.maxLength)
      {
        MDNPDIAG_ERROR(pReqDesc->pChannel, pReqDesc->pSession, MDNPDIAG_OBJ_LENGTH);
        pTxData = TMWDEFS_NULL;
      }
    }

    if(pUserTxData->tmw.pMsgBuf[1] != DNPDEFS_FC_READ)
    {
      MDNPDIAG_ERROR(pReqDesc->pChannel, pReqDesc->pSession, MDNPDIAG_FC);
      pTxData = TMWDEFS_NULL;
    }

    if(pTxData == TMWDEFS_NULL)
    {
      dnpchnl_freeTxData((TMWSESN_TX_DATA *)pUserTxData);
      return(TMWDEFS_NULL);
    }
  }

  /* Object Header */
  if(!mdnpbrm_addObjectHeader(pTxData, group, variation, qualifier, 0, numPoints))
  {
    dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
    return(TMWDEFS_NULL);
  }

  /* Add point numbers */
  for(i = 0; i < numPoints; i++)
  {
    if(qualifier == DNPDEFS_QUAL_8BIT_INDEX)
    {
      pTxData->tmw.pMsgBuf[pTxData->tmw.msgLength++] = (TMWTYPES_UCHAR)pPoints[i];
    }
    else
    {
      tmwtarg_store16(&pPoints[i], pTxData->tmw.pMsgBuf + pTxData->tmw.msgLength);
      pTxData->tmw.msgLength += 2;
    }
  }

  /* Send request */
  if(pUserTxData == TMWDEFS_NULL)
  {
    if(!_sendFragment(pTxData))
    {
      pTxData = TMWDEFS_NULL;
    }
  }

  return((TMWSESN_TX_DATA *)pTxData);
}

/* function: mdnpbrm_binaryOutWrite */
TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_binaryOutWrite(
  MDNPBRM_REQ_DESC *pReqDesc,
  DNPCHNL_TX_DATA *pUserTxData,
  TMWTYPES_UCHAR qualifier,
  TMWTYPES_USHORT start,
  TMWTYPES_USHORT stop,
  TMWTYPES_UCHAR *pValues)
{  
  DNPCHNL_TX_DATA *pTxData;
  TMWTYPES_UCHAR *ptr;
  int bit;

  if(start > stop)
    return(TMWDEFS_NULL);

  if((qualifier!= DNPDEFS_QUAL_8BIT_START_STOP) && (qualifier != DNPDEFS_QUAL_16BIT_START_STOP))
    return(TMWDEFS_NULL);

  if(pUserTxData == TMWDEFS_NULL)
  { 
    TMWTYPES_USHORT size = 10+((stop-start)/8);
    
#if MDNPDATA_SUPPORT_OBJ120
    if(pReqDesc->authAggressiveMode)
      size += DNPAUTH_AGGRESSIVE_SIZE;
#endif

    pTxData = (DNPCHNL_TX_DATA *)dnpchnl_newTxData(
      pReqDesc->pChannel, pReqDesc->pSession, size, 0);

    if(pTxData == TMWDEFS_NULL)
    {
      return(TMWDEFS_NULL);
    }

    /* Initialize Transmit Data Structure from Request Descriptor */
    if(!_initTxData(pTxData, pReqDesc, "Write Binary Output"))
    {
      dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
      return(TMWDEFS_NULL);
    }

    /* Build request header */
    mdnpbrm_buildRequestHeader(pTxData, DNPDEFS_FC_WRITE);
    
#if MDNPDATA_SUPPORT_OBJ120
    if(pReqDesc->authAggressiveMode)
    {
      if(mdnpauth_addAggrRequestStart((TMWSESN_TX_DATA *)pTxData, pReqDesc->authUserNumber) == TMWDEFS_NULL)
      {
        dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
        return(TMWDEFS_NULL);
      }
    }
#endif 
  }
  else
  {
    if(pUserTxData->tmw.pMsgBuf[1] != DNPDEFS_FC_WRITE)
    {
      MDNPDIAG_ERROR(pReqDesc->pChannel, pReqDesc->pSession, MDNPDIAG_FC);
      dnpchnl_freeTxData((TMWSESN_TX_DATA *)pUserTxData);
      return(TMWDEFS_NULL);
    }

    pTxData = pUserTxData;
  }

  /* Object Header */
  if(!mdnpbrm_addObjectHeader(pTxData, DNPDEFS_OBJ_10_BIN_OUT_STATUSES, 1, qualifier, start, stop))
  {
    dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
    return(TMWDEFS_NULL);
  }

  /* Set correct bits */

  bit = 0;
  ptr = &pTxData->tmw.pMsgBuf[pTxData->tmw.msgLength];
  *ptr = 0; 
  while(start <= stop)
  {
     if(*pValues++ != 0)
        *ptr |= (1<<bit);

     if(++bit > 7)
     {
       bit = 0;
       pTxData->tmw.msgLength++;
       ptr = &pTxData->tmw.pMsgBuf[pTxData->tmw.msgLength];
       *ptr = 0; 
     }
     start++;
  }
  if(bit != 0)
    pTxData->tmw.msgLength++; 

  /* Send request */
  if(pUserTxData == TMWDEFS_NULL)
  {
    /* Send request */ 
    if(!_sendFragment(pTxData))
    {
      pTxData = TMWDEFS_NULL;
    }
  }
  return((TMWSESN_TX_DATA *)pTxData);
}

/* function: mdnpbrm_binaryCommand */
TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_binaryCommand(
  MDNPBRM_REQ_DESC *pReqDesc,
  DNPCHNL_TX_DATA *pUserTxData,
  TMWTYPES_UCHAR funcCode,
  MDNPBRM_AUTO_MODE autoMode,
  TMWTYPES_MILLISECONDS opFeedbackDelay,
  TMWTYPES_UCHAR qualifier,
  TMWTYPES_UCHAR numCROBs,
  MDNPBRM_CROB_INFO *pCROBInfo)
{
  DNPCHNL_TX_DATA *pTxData;
  TMWTYPES_USHORT size;
  TMWTYPES_UCHAR i;
  OperateCallbackData *pCallbackData = TMWDEFS_NULL;

  /* only SELECT, OPERATE, DIRECT_OP and DIRECT_OP_NOACK are allowed */
  if(funcCode < DNPDEFS_FC_SELECT || funcCode > DNPDEFS_FC_DIRECT_OP_NOACK)
  {
    MDNPDIAG_ERROR(pReqDesc->pChannel, pReqDesc->pSession, MDNPDIAG_FC);
    return(TMWDEFS_NULL);
  }

  /* If SELECT ONLY auto feedback after OPERATE does not make sense */
  if((funcCode == DNPDEFS_FC_SELECT) && (autoMode == MDNPBRM_AUTO_MODE_FEEDBACK))
  {
    autoMode = MDNPBRM_AUTO_MODE_NONE;
  }

  /* if not SELECT, AUTO OPERATE makes no sense */
  if((funcCode != DNPDEFS_FC_SELECT)
    &&(autoMode == MDNPBRM_AUTO_MODE_OPERATE))
  {
    autoMode = MDNPBRM_AUTO_MODE_NONE;
  }

  /* Validate qualifier */
  if((qualifier != DNPDEFS_QUAL_8BIT_INDEX)
    && (qualifier != DNPDEFS_QUAL_16BIT_INDEX))
  {
    if(pUserTxData != TMWDEFS_NULL)
      dnpchnl_freeTxData((TMWSESN_TX_DATA *)pUserTxData);

    MDNPDIAG_ERROR(pReqDesc->pChannel, pReqDesc->pSession, MDNPDIAG_INV_QUALIFIER);
    return(TMWDEFS_NULL);
  }

  size = (TMWTYPES_USHORT)(7 + (numCROBs * 12));
#if MDNPDATA_SUPPORT_OBJ120
  if(pReqDesc->authAggressiveMode)
    size += DNPAUTH_AGGRESSIVE_SIZE;
#endif

  if(pUserTxData == TMWDEFS_NULL)
  {
    if(qualifier == DNPDEFS_QUAL_8BIT_INDEX)
    {
      /* Max size is 2 Appl Header + 5 Object Header + (numCROBs * (1 Index + 11 Data)) */
      pTxData = (DNPCHNL_TX_DATA *)dnpchnl_newTxData(
        pReqDesc->pChannel, pReqDesc->pSession, size, 0);
    }
    else
    {
      /* Max size is 2 Appl Header + 5 Object Header + (numCROBs * (2 Index + 11 Data))
       * 1 octet per crob larger than above
       */
      size += numCROBs;
      pTxData = (DNPCHNL_TX_DATA *)dnpchnl_newTxData(
        pReqDesc->pChannel, pReqDesc->pSession, size, 0);
    }

    if(pTxData == TMWDEFS_NULL)
    {
      return(TMWDEFS_NULL);
    }

    /* Initialize Transmit Data Structure from Request Descriptor */
    if(!_initTxData(pTxData, pReqDesc, "Binary Command"))
    {
      dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
      return(TMWDEFS_NULL);
    }

    /* Build request header */
    mdnpbrm_buildRequestHeader(pTxData, funcCode);
    
#if MDNPDATA_SUPPORT_OBJ120
    if(pReqDesc->authAggressiveMode)
    {
      if(mdnpauth_addAggrRequestStart((TMWSESN_TX_DATA *)pTxData, pReqDesc->authUserNumber) == TMWDEFS_NULL)
      {
        dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
        return(TMWDEFS_NULL);
      }
    }
#endif 
  }
  else
  {
    pTxData = pUserTxData;
    if(qualifier == DNPDEFS_QUAL_8BIT_INDEX)
    {
      /* Max size is 5 Object Header + (numCROBs * (1 Index + 11 Data)) */
      if(pUserTxData->tmw.msgLength + 5 + (numCROBs * 12) >= pUserTxData->tmw.maxLength)
      {
        MDNPDIAG_ERROR(pReqDesc->pChannel, pReqDesc->pSession, MDNPDIAG_OBJ_LENGTH);
        pTxData = TMWDEFS_NULL;
      }
    }
    else
    {
      /* Max size is 5 Object Header + (numCROBs * (2 Index + 11 Data)) */
      if(pUserTxData->tmw.msgLength + 5 + (numCROBs * 13) >= pUserTxData->tmw.maxLength)
      {
        MDNPDIAG_ERROR(pReqDesc->pChannel, pReqDesc->pSession, MDNPDIAG_OBJ_LENGTH);
        pTxData = TMWDEFS_NULL;
      }
    }

    if(pUserTxData->tmw.pMsgBuf[1] != funcCode)
    {
      MDNPDIAG_ERROR(pReqDesc->pChannel, pReqDesc->pSession, MDNPDIAG_FC);
      pTxData = TMWDEFS_NULL;
    }

    if(pTxData == TMWDEFS_NULL)
    {
      dnpchnl_freeTxData((TMWSESN_TX_DATA *)pUserTxData);
      return(TMWDEFS_NULL);
    }
  }

  /* Object Header */
  if(!mdnpbrm_addObjectHeader(pTxData, DNPDEFS_OBJ_12_BIN_OUT_CTRLS, 1, qualifier, 0, numCROBs))
  {
    dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
    return(TMWDEFS_NULL);
  }

  for(i = 0; i < numCROBs; i++)
  {
#if MDNPDATA_SUPPORT_OBJ12_COUNT
    DNPDIAG_SHOW_CROB(pReqDesc->pSession, pCROBInfo[i].pointNumber, pCROBInfo[i].control, pCROBInfo[i].count, pCROBInfo[i].onTime, pCROBInfo[i].offTime, 0, 0);
#else
    DNPDIAG_SHOW_CROB(pReqDesc->pSession, pCROBInfo[i].pointNumber, pCROBInfo[i].control, 1, pCROBInfo[i].onTime, pCROBInfo[i].offTime, 0, 0);
#endif

    /* Index */
    if(qualifier == DNPDEFS_QUAL_16BIT_INDEX)
    {
      tmwtarg_store16(&pCROBInfo[i].pointNumber, pTxData->tmw.pMsgBuf + pTxData->tmw.msgLength);
      pTxData->tmw.msgLength += 2;
    }
    else
    {
      if (pCROBInfo[i].pointNumber <= 0xff)
      {
        pTxData->tmw.pMsgBuf[pTxData->tmw.msgLength++] = (TMWTYPES_UCHAR)pCROBInfo[i].pointNumber;
      }
      else
      {
        MDNPDIAG_ERROR(pReqDesc->pChannel, pReqDesc->pSession, MDNPDIAG_INV_QUALIFIER_POINT);
        dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
        return(TMWDEFS_NULL);
      }
    }

    /* Control */
    pTxData->tmw.pMsgBuf[pTxData->tmw.msgLength++] = pCROBInfo[i].control;

    /* Count */
#if MDNPDATA_SUPPORT_OBJ12_COUNT 
    pTxData->tmw.pMsgBuf[pTxData->tmw.msgLength++] = pCROBInfo[i].count;
#else
    pTxData->tmw.pMsgBuf[pTxData->tmw.msgLength++] = 1;
#endif

    /* On time */
    tmwtarg_store32(&pCROBInfo[i].onTime, pTxData->tmw.pMsgBuf + pTxData->tmw.msgLength);
    pTxData->tmw.msgLength += 4;

    /* Off Time */
    tmwtarg_store32(&pCROBInfo[i].offTime, pTxData->tmw.pMsgBuf + pTxData->tmw.msgLength);
    pTxData->tmw.msgLength += 4;

    /* Initialize status */
    pTxData->tmw.pMsgBuf[pTxData->tmw.msgLength++] = 0;
  }

  /* Check funcCode to see if response expected */
  if(funcCode == DNPDEFS_FC_DIRECT_OP_NOACK)
  {
    pTxData->tmw.txFlags |= TMWSESN_TXFLAGS_NO_RESPONSE;

#if MDNPDATA_SUPPORT_OBJ120
    if(!pReqDesc->authAggressiveMode)
    {
      MDNPSESN *pMDNPSession = (MDNPSESN *)pTxData->tmw.pSession;
      if(pMDNPSession->authenticationEnabled)
      {
        /* Need to delay in case this is challenged. */
        pTxData->dnpTxFlags = DNPCHNL_DNPTXFLAGS_AUTH_NOACKDELAY;
      }
    }
#endif
  }
  else if (autoMode != 0)
  {
    if (pTxData->pInternalCallbackParam != TMWDEFS_NULL)
    {
      /* this request has multiple objects, can't add another callback structure */
      mdnpmem_free(pTxData->pInternalCallbackParam);
    }

    pCallbackData = (OperateCallbackData *)mdnpmem_alloc(MDNPMEM_OPER_CALLBACK_TYPE);
    if (pCallbackData == TMWDEFS_NULL)
    {
      dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
      return(TMWDEFS_NULL);
    }

    pCallbackData->pSession = pReqDesc->pSession;
    pCallbackData->pTxData = pTxData;
    tmwtimer_init(&pCallbackData->feedbackTimer);

    pCallbackData->operateRequired = TMWDEFS_FALSE;
    if ((funcCode == DNPDEFS_FC_SELECT)
      && ((autoMode & MDNPBRM_AUTO_MODE_OPERATE) != 0))
    {
      pCallbackData->operateRequired = TMWDEFS_TRUE;
    }

    pCallbackData->binFeedbackRequired = TMWDEFS_FALSE;
    pCallbackData->anlgFeedbackRequired = TMWDEFS_FALSE;
    if ((autoMode & MDNPBRM_AUTO_MODE_FEEDBACK) != 0)
    {
      pCallbackData->binFeedbackRequired = TMWDEFS_TRUE;
    }
   
    pCallbackData->feedbackDelay = opFeedbackDelay;

    pTxData->pInternalCallback = _selectOperateCallbackFunc;
    pTxData->pInternalCallbackParam = pCallbackData;
  }

  /* Send request */
  if(pUserTxData == TMWDEFS_NULL)
  {
    if(!_sendFragment(pTxData))
    {
      if(pCallbackData != TMWDEFS_NULL)
      {
        mdnpmem_free(pCallbackData);
      } 
      pTxData = TMWDEFS_NULL; 
    }
  }

  return((TMWSESN_TX_DATA *)pTxData);
}

/* function: mdnpbrm_sendPatternMask */
TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_patternMask(
  MDNPBRM_REQ_DESC *pReqDesc,
  TMWTYPES_UCHAR funcCode, 
  MDNPBRM_AUTO_MODE autoMode, 
  TMWTYPES_MILLISECONDS opFeedbackDelay,
  TMWTYPES_UCHAR qualifier,
  TMWTYPES_USHORT start,
  TMWTYPES_USHORT stop,
  TMWTYPES_UCHAR control,
  TMWTYPES_UCHAR count,
  TMWTYPES_ULONG activationPeriod,
  TMWTYPES_ULONG deactivationPeriod,
  TMWTYPES_UCHAR *pMask)
{
  DNPCHNL_TX_DATA *pTxData;
  TMWTYPES_USHORT numPoints;
  TMWTYPES_USHORT maskSize;
  TMWTYPES_USHORT length;
  int i;
  OperateCallbackData *pCallbackData = TMWDEFS_NULL;

  /* If SELECT ONLY auto feedback after OPERATE does not make sense */
  if((funcCode == DNPDEFS_FC_SELECT) && (autoMode == MDNPBRM_AUTO_MODE_FEEDBACK))
  {
    autoMode = MDNPBRM_AUTO_MODE_NONE;
  }

  /* if not SELECT, AUTO OPERATE makes no sense */
  if((funcCode != DNPDEFS_FC_SELECT)
    &&(autoMode == MDNPBRM_AUTO_MODE_OPERATE))
  {
    autoMode = MDNPBRM_AUTO_MODE_NONE;
  }
  
  /* Validate qualifier */
  if((qualifier != DNPDEFS_QUAL_8BIT_START_STOP)
    && (qualifier != DNPDEFS_QUAL_16BIT_START_STOP))
  {
    MDNPDIAG_ERROR(pReqDesc->pChannel, pReqDesc->pSession, MDNPDIAG_INV_QUALIFIER);
    return(TMWDEFS_NULL);
  }

  /* Calculate number of bytes in mask */
  numPoints = (TMWTYPES_USHORT)(stop - start + 1);
  maskSize = (TMWTYPES_USHORT)(numPoints / 8);
  if((maskSize * 8) < numPoints)
    maskSize = (TMWTYPES_USHORT)(maskSize + 1);
  
  length = (TMWTYPES_USHORT)(24 + maskSize);

#if MDNPDATA_SUPPORT_OBJ120
  if(pReqDesc->authAggressiveMode) 
    length += DNPAUTH_AGGRESSIVE_SIZE;
#endif

  /* Max size is 2 + PCB size + Pattern Mask size
   * PCB Size = 4 Object Header + 11 Object Data
   * Pattern Mask Size = 7 Object Header + Mask Size 
   */
  pTxData = (DNPCHNL_TX_DATA *)dnpchnl_newTxData(
    pReqDesc->pChannel, pReqDesc->pSession, length, 0);

  if(pTxData == TMWDEFS_NULL)
  {
    return(TMWDEFS_NULL);
  }

  /* Initialize Transmit Data Structure from Request Descriptor */
  if(!_initTxData(pTxData, pReqDesc, "Pattern Mask"))
  {
    dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
    return(TMWDEFS_NULL);
  }

  /* Build request header */
  mdnpbrm_buildRequestHeader(pTxData, funcCode);
  
#if MDNPDATA_SUPPORT_OBJ120
    if(pReqDesc->authAggressiveMode)
    {
      if(mdnpauth_addAggrRequestStart((TMWSESN_TX_DATA *)pTxData, pReqDesc->authUserNumber) == TMWDEFS_NULL)
      {
        dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
        return(TMWDEFS_NULL);
      }
    }
#endif 

  /* Check funcCode to see if response expected */
  if(funcCode == DNPDEFS_FC_DIRECT_OP_NOACK)
  {
    pTxData->tmw.txFlags |= TMWSESN_TXFLAGS_NO_RESPONSE;

#if MDNPDATA_SUPPORT_OBJ120
    if(!pReqDesc->authAggressiveMode)
    {
      MDNPSESN *pMDNPSession = (MDNPSESN *)pTxData->tmw.pSession;
      if(pMDNPSession->authenticationEnabled)
      {
        /* Need to delay in case this is challenged. */
        pTxData->dnpTxFlags = DNPCHNL_DNPTXFLAGS_AUTH_NOACKDELAY;
      }
    }
#endif
  }

  /* Add Pattern Control Block */
  if(!mdnpbrm_addObjectHeader(pTxData, DNPDEFS_OBJ_12_BIN_OUT_CTRLS, 2, DNPDEFS_QUAL_8BIT_LIMITED_QTY, 0, 1))
  {
    dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
    return(TMWDEFS_NULL);
  }

  pTxData->tmw.pMsgBuf[pTxData->tmw.msgLength++] = control;

  pTxData->tmw.pMsgBuf[pTxData->tmw.msgLength++] = count;

  tmwtarg_store32(&activationPeriod, pTxData->tmw.pMsgBuf + pTxData->tmw.msgLength);
  pTxData->tmw.msgLength += 4;

  tmwtarg_store32(&deactivationPeriod, pTxData->tmw.pMsgBuf + pTxData->tmw.msgLength);
  pTxData->tmw.msgLength += 4;

  pTxData->tmw.pMsgBuf[pTxData->tmw.msgLength++] = 0;

  /* Add Pattern Mask */
  if(!mdnpbrm_addObjectHeader(pTxData, DNPDEFS_OBJ_12_BIN_OUT_CTRLS, 3, qualifier, start, stop))
  {
    dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
    return(TMWDEFS_NULL);
  }

  for(i = 0; i < maskSize; i++)
    pTxData->tmw.pMsgBuf[pTxData->tmw.msgLength++] = pMask[i];

  if(autoMode != 0)
  {
    if (pTxData->pInternalCallbackParam != TMWDEFS_NULL)
    {
      /* this request has multiple objects, can't add another callback structure */
      mdnpmem_free(pTxData->pInternalCallbackParam);
    }

    pCallbackData = (OperateCallbackData *)mdnpmem_alloc(MDNPMEM_OPER_CALLBACK_TYPE);
    if(pCallbackData == TMWDEFS_NULL)
    {
      dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
      return(TMWDEFS_NULL);
    }

    pCallbackData->pSession = pReqDesc->pSession;
    pCallbackData->pTxData = pTxData;
    tmwtimer_init(&pCallbackData->feedbackTimer);

    pCallbackData->operateRequired = TMWDEFS_FALSE;
    if((funcCode == DNPDEFS_FC_SELECT)
      && ((autoMode & MDNPBRM_AUTO_MODE_OPERATE) != 0))
    {
      pCallbackData->operateRequired = TMWDEFS_TRUE;
    }

    pCallbackData->binFeedbackRequired = TMWDEFS_FALSE;
    pCallbackData->anlgFeedbackRequired = TMWDEFS_FALSE;
    if ((autoMode & MDNPBRM_AUTO_MODE_FEEDBACK) != 0)
    {
      pCallbackData->binFeedbackRequired = TMWDEFS_TRUE;
    }

    pCallbackData->feedbackDelay = opFeedbackDelay;

    pTxData->pInternalCallback = _selectOperateCallbackFunc;
    pTxData->pInternalCallbackParam = pCallbackData;
  }

  /* Send request */ 
  if(!_sendFragment(pTxData))
  {
    if(pCallbackData != TMWDEFS_NULL)
    {
      mdnpmem_free(pCallbackData);
    } 
    pTxData = TMWDEFS_NULL; 
  }

  return((TMWSESN_TX_DATA *)pTxData);
}

/* function: mdnpbrm_analogCommand */
TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_analogCommand(
  MDNPBRM_REQ_DESC *pReqDesc,
  DNPCHNL_TX_DATA *pUserTxData,
  TMWTYPES_UCHAR funcCode,
  MDNPBRM_AUTO_MODE autoMode,
  TMWTYPES_MILLISECONDS opFeedbackDelay,
  TMWTYPES_UCHAR qualifier,
  TMWTYPES_UCHAR variation,
  TMWTYPES_UCHAR numObjects,
  MDNPBRM_ANALOG_INFO *pAnlgInfo)
{
  DNPCHNL_TX_DATA *pTxData;
  TMWTYPES_USHORT size;
  int datasize[] = {4, 2, 4, 8};
  int dsize;
  int asize;
  int i;
  OperateCallbackData *pCallbackData = TMWDEFS_NULL;

  /* If SELECT ONLY auto feedback after OPERATE does not make sense */
  if((funcCode == DNPDEFS_FC_SELECT) && (autoMode == MDNPBRM_AUTO_MODE_FEEDBACK))
  {
    autoMode = MDNPBRM_AUTO_MODE_NONE;
  }

  /* if not SELECT, AUTO OPERATE makes no sense */
  if((funcCode != DNPDEFS_FC_SELECT)
    &&(autoMode == MDNPBRM_AUTO_MODE_OPERATE) )
  {
    autoMode = MDNPBRM_AUTO_MODE_NONE;
  }

  /* Validate input */
  if((variation < 1) || (variation > 4))
  {
    if(pUserTxData != TMWDEFS_NULL)
      dnpchnl_freeTxData((TMWSESN_TX_DATA *)pUserTxData);

    MDNPDIAG_ERROR(pReqDesc->pChannel, pReqDesc->pSession, MDNPDIAG_INV_VARIATION);
    return(TMWDEFS_NULL);
  }

  dsize = datasize[variation - 1];

  if(qualifier == DNPDEFS_QUAL_8BIT_INDEX) 
    asize = 1;
  else if(qualifier == DNPDEFS_QUAL_16BIT_INDEX)
    asize = 2;
  else
  {
    MDNPDIAG_ERROR(pReqDesc->pChannel, pReqDesc->pSession, MDNPDIAG_INV_QUALIFIER);
    return(TMWDEFS_NULL);
  }
   
  /* Max size is 2 Appl Header + 5 Object Header + 
   * (numObjects * (asize Index + dsize + 1 Data)) 
   */
  size = (TMWTYPES_USHORT)(7 + (numObjects * (asize + dsize + 1))); 
  if(((autoMode & MDNPBRM_AUTO_MODE_FEEDBACK) != 0)
    && (size < 17))
  {
    /* Make sure this is big enough for feedback poll 
     * Size is 2 Appl Header + (3 * 5) Object Header
     */
    size = 17;
  }

#if MDNPDATA_SUPPORT_OBJ120
  if(pReqDesc->authAggressiveMode)
    size += DNPAUTH_AGGRESSIVE_SIZE;
#endif

  if(pUserTxData == TMWDEFS_NULL)
  {
    pTxData = (DNPCHNL_TX_DATA *)dnpchnl_newTxData(pReqDesc->pChannel, pReqDesc->pSession, 
      size, 0);

    if(pTxData == TMWDEFS_NULL)
    {
      return(TMWDEFS_NULL);
    }

    /* Initialize Transmit Data Structure from Request Descriptor */
    if(!_initTxData(pTxData, pReqDesc, "Analog Command"))
    {
      dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
      return(TMWDEFS_NULL);
    }

    /* Build request header */
    mdnpbrm_buildRequestHeader(pTxData, funcCode);
    
#if MDNPDATA_SUPPORT_OBJ120
    if(pReqDesc->authAggressiveMode)
    {
      if(mdnpauth_addAggrRequestStart((TMWSESN_TX_DATA *)pTxData, pReqDesc->authUserNumber) == TMWDEFS_NULL)
      {
        dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
        return(TMWDEFS_NULL);
      }
    }
#endif 
  }
  else
  {
    pTxData = pUserTxData;

    /* Max size is 5 Object Header + 
     * (numObjects * (asize Index + dsize + 1 Data)) 
     */
    if(pUserTxData->tmw.msgLength + size >= pUserTxData->tmw.maxLength)
    {
      MDNPDIAG_ERROR(pReqDesc->pChannel, pReqDesc->pSession, MDNPDIAG_OBJ_LENGTH);
      pTxData = TMWDEFS_NULL;
    }

    if(pUserTxData->tmw.pMsgBuf[1] != funcCode)
    {
      MDNPDIAG_ERROR(pReqDesc->pChannel, pReqDesc->pSession, MDNPDIAG_FC);
      pTxData = TMWDEFS_NULL;
    }
    if(pTxData == TMWDEFS_NULL)
    {
      dnpchnl_freeTxData((TMWSESN_TX_DATA *)pUserTxData);
      return(TMWDEFS_NULL);
    }
  }

  /* Object Header */
  if(!mdnpbrm_addObjectHeader(pTxData, 
    DNPDEFS_OBJ_41_ANA_OUT_CTRLS, variation, qualifier, 0, numObjects))
  {
    dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
    return(TMWDEFS_NULL);
  }

  for(i = 0; i < numObjects; i++)
  {
    TMWTYPES_UCHAR flags = 0;

    DNPDIAG_SHOW_ANALOG_CONTROL(pReqDesc->pSession, pAnlgInfo[i].pointNumber, &pAnlgInfo[i].value, 0, TMWDEFS_FALSE, 0, TMWDEFS_NULL);

    /* Index */
    if(qualifier == DNPDEFS_QUAL_16BIT_INDEX)
    {
      tmwtarg_store16(&pAnlgInfo[i].pointNumber, pTxData->tmw.pMsgBuf + pTxData->tmw.msgLength);
      pTxData->tmw.msgLength += 2;
    }
    else
    {
      if (pAnlgInfo[i].pointNumber <= 0xff)
      {
        pTxData->tmw.pMsgBuf[pTxData->tmw.msgLength++] = (TMWTYPES_UCHAR)pAnlgInfo[i].pointNumber;
      }
      else
      {
        MDNPDIAG_ERROR(pReqDesc->pChannel, pReqDesc->pSession, MDNPDIAG_INV_QUALIFIER_POINT);
        dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
        return(TMWDEFS_NULL);
      }
    }

    /* Value */
    switch(variation)
    {
    case 1:
      {
        TMWTYPES_LONG tmpValue = dnputil_getAnalogValueLong(&pAnlgInfo[i].value, &flags);
        tmwtarg_store32((TMWTYPES_ULONG *)&tmpValue, pTxData->tmw.pMsgBuf + pTxData->tmw.msgLength);
        pTxData->tmw.msgLength += 4;
      }
      break;

    case 2:
      {
        TMWTYPES_SHORT tmpValue = dnputil_getAnalogValueShort(&pAnlgInfo[i].value, &flags);
        tmwtarg_store16((TMWTYPES_USHORT *)&tmpValue, pTxData->tmw.pMsgBuf + pTxData->tmw.msgLength);
        pTxData->tmw.msgLength += 2;
      }
      break;

#if MDNPDATA_SUPPORT_OBJ41_V3
    case 3:
      {
        TMWTYPES_SFLOAT tmpValue = dnputil_getAnalogValueFloat(&pAnlgInfo[i].value, &flags);
        tmwtarg_store32((TMWTYPES_ULONG *)&tmpValue, pTxData->tmw.pMsgBuf + pTxData->tmw.msgLength);
        pTxData->tmw.msgLength += 4;
      }
      break;
#endif

#if MDNPDATA_SUPPORT_OBJ41_V4
    case 4:
      {
        TMWTYPES_DOUBLE dval = dnputil_getAnalogValueDouble(&pAnlgInfo[i].value);
        tmwtarg_store64(&dval, pTxData->tmw.pMsgBuf + pTxData->tmw.msgLength);
        pTxData->tmw.msgLength += 8;
      }
      break;
#endif

    default:
      {
        dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
        return(TMWDEFS_NULL);
      }
      break;
    }

    /* If the data value could not be sent with the variation requested, do not send, return failure */
    if (flags != 0)
    {
      char pointNum[256];
      sprintf(pointNum, "Analog Point %d, Value %g, Variation %d", pAnlgInfo[i].pointNumber, pAnlgInfo[i].value.value.dval, variation);
      MDNPDIAG_ERROR_MSG(pReqDesc->pChannel, pReqDesc->pSession, MDNPDIAG_INV_VARIATION_VALUE, pointNum);
      dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
      return(TMWDEFS_NULL);
    }

    /* Initialize status */
    pTxData->tmw.pMsgBuf[pTxData->tmw.msgLength++] = 0;
  }

  /* Check funcCode to see if response expected */
  if(funcCode == DNPDEFS_FC_DIRECT_OP_NOACK)
  {
    pTxData->tmw.txFlags |= TMWSESN_TXFLAGS_NO_RESPONSE;

#if MDNPDATA_SUPPORT_OBJ120
    if(!pReqDesc->authAggressiveMode)
    {
      MDNPSESN *pMDNPSession = (MDNPSESN *)pTxData->tmw.pSession;
      if(pMDNPSession->authenticationEnabled)
      {
        /* Need to delay in case this is challenged. */
        pTxData->dnpTxFlags = DNPCHNL_DNPTXFLAGS_AUTH_NOACKDELAY;
      }
    }
#endif
  }
  else if(autoMode != 0)
  {
    if (pTxData->pInternalCallbackParam != TMWDEFS_NULL)
    {
      /* this request has multiple objects, can't add another callback structure */
      mdnpmem_free(pTxData->pInternalCallbackParam);
    } 

    pCallbackData = (OperateCallbackData *)mdnpmem_alloc(MDNPMEM_OPER_CALLBACK_TYPE);
    if(pCallbackData == TMWDEFS_NULL)
    {
      dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
      return(TMWDEFS_NULL);
    }

    pCallbackData->pSession = pReqDesc->pSession;
    pCallbackData->pTxData = pTxData;
    tmwtimer_init(&pCallbackData->feedbackTimer);

    pCallbackData->operateRequired = TMWDEFS_FALSE;
    if((funcCode == DNPDEFS_FC_SELECT)
      && ((autoMode & MDNPBRM_AUTO_MODE_OPERATE) != 0))
    {
      pCallbackData->operateRequired = TMWDEFS_TRUE;
    }

    pCallbackData->binFeedbackRequired = TMWDEFS_FALSE;
    pCallbackData->anlgFeedbackRequired = TMWDEFS_FALSE;
    if ((autoMode & MDNPBRM_AUTO_MODE_FEEDBACK) != 0)
    {
      pCallbackData->anlgFeedbackRequired = TMWDEFS_TRUE;
    }

    pCallbackData->feedbackDelay = opFeedbackDelay;

    pTxData->pInternalCallback = _selectOperateCallbackFunc;
    pTxData->pInternalCallbackParam = pCallbackData;
  }

  /* Send request */
  if(pUserTxData == TMWDEFS_NULL)
  {
    /* Send request */ 
    if(!_sendFragment(pTxData))
    {
      if(pCallbackData != TMWDEFS_NULL)
      {
        mdnpmem_free(pCallbackData);
      }  
      pTxData = TMWDEFS_NULL; 
    }
  }

  return((TMWSESN_TX_DATA *)pTxData);
}

/* function: mdnpbrm_writeDeadband */
TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_writeDeadband(
  MDNPBRM_REQ_DESC *pReqDesc,
  DNPCHNL_TX_DATA *pUserTxData,
  TMWTYPES_UCHAR qualifier,
  TMWTYPES_UCHAR variation,
  TMWTYPES_UCHAR numObjects,
  MDNPBRM_ANALOG_INFO *pAnlgInfo)
{
  DNPCHNL_TX_DATA *pTxData;
  int i;

  /* Validate input arguments */
  if((variation < 1) || (variation > 3))
  {
    if(pUserTxData != TMWDEFS_NULL)
      dnpchnl_freeTxData((TMWSESN_TX_DATA *)pUserTxData);

    MDNPDIAG_ERROR(pReqDesc->pChannel, pReqDesc->pSession, MDNPDIAG_INV_VARIATION);
    return(TMWDEFS_NULL);
  }

  if((qualifier != DNPDEFS_QUAL_8BIT_INDEX)
    && (qualifier != DNPDEFS_QUAL_16BIT_INDEX)
    && (qualifier != DNPDEFS_QUAL_8BIT_START_STOP)
    && (qualifier != DNPDEFS_QUAL_16BIT_START_STOP))
  {
    MDNPDIAG_ERROR(pReqDesc->pChannel, pReqDesc->pSession, MDNPDIAG_INV_QUALIFIER);
    return(TMWDEFS_NULL);
  }

  if(pUserTxData == TMWDEFS_NULL)
  {
    /* Max size is 2 Appl Header + 5 Object Header + numObjects * (2 Point Number + 4 Object Data) */
    TMWTYPES_USHORT size = (7 + (numObjects * 6));

#if MDNPDATA_SUPPORT_OBJ120
    if(pReqDesc->authAggressiveMode)
      size += DNPAUTH_AGGRESSIVE_SIZE;
#endif

    pTxData = (DNPCHNL_TX_DATA *)dnpchnl_newTxData(
      pReqDesc->pChannel, pReqDesc->pSession, size, 0);

    if(pTxData == TMWDEFS_NULL)
      return(TMWDEFS_NULL);

    /* Initialize Transmit Data Structure from Request Descriptor */
    if(!_initTxData(pTxData, pReqDesc, "Write Deadband"))
    {
      dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
      return(TMWDEFS_NULL);
    }

    /* Create request header */
    mdnpbrm_buildRequestHeader(pTxData, DNPDEFS_FC_WRITE);

#if MDNPDATA_SUPPORT_OBJ120
    if(pReqDesc->authAggressiveMode)
    {
      if(mdnpauth_addAggrRequestStart((TMWSESN_TX_DATA *)pTxData, pReqDesc->authUserNumber) == TMWDEFS_NULL)
      {
        dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
        return(TMWDEFS_NULL);
      }
    }
#endif 
  }
  else
  {
    pTxData = pUserTxData;

    /* Max size is 5 Object Header + 2 Point Number + numObjs * 4 Object Data */
    if(pUserTxData->tmw.msgLength + (5 + (numObjects * 6)) >= pUserTxData->tmw.maxLength)
    {
      MDNPDIAG_ERROR(pReqDesc->pChannel, pReqDesc->pSession, MDNPDIAG_OBJ_LENGTH);
      pTxData = TMWDEFS_NULL;
    }

    if(pUserTxData->tmw.pMsgBuf[1] != DNPDEFS_FC_WRITE)
    {
      MDNPDIAG_ERROR(pReqDesc->pChannel, pReqDesc->pSession, MDNPDIAG_FC);
      pTxData = TMWDEFS_NULL;
    }

    if(pTxData == TMWDEFS_NULL)
    {
      dnpchnl_freeTxData((TMWSESN_TX_DATA *)pUserTxData);
      return(TMWDEFS_NULL);
    }
  }

  /* Add object header */
  if ((qualifier == DNPDEFS_QUAL_8BIT_START_STOP)
    || (qualifier == DNPDEFS_QUAL_16BIT_START_STOP))
  {
    TMWTYPES_USHORT start = pAnlgInfo[0].pointNumber;
    if(!mdnpbrm_addObjectHeader(pTxData, DNPDEFS_OBJ_34_ANA_INPUT_DBANDS, variation, qualifier, start, start+numObjects-1))
    {
      dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
      return(TMWDEFS_NULL);
    }
  }
  else
  {
    if (!mdnpbrm_addObjectHeader(pTxData, DNPDEFS_OBJ_34_ANA_INPUT_DBANDS, variation, qualifier, 0, numObjects))
    {
      dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
      return(TMWDEFS_NULL);
    }
  }

  for(i = 0; i < numObjects; i++)
  {
    /* Index */
    if(qualifier == DNPDEFS_QUAL_16BIT_INDEX)
    {
      tmwtarg_store16(&pAnlgInfo[i].pointNumber, pTxData->tmw.pMsgBuf + pTxData->tmw.msgLength);
      pTxData->tmw.msgLength += 2;
    }
    else if(qualifier == DNPDEFS_QUAL_8BIT_INDEX)
    {
      pTxData->tmw.pMsgBuf[pTxData->tmw.msgLength++] = (TMWTYPES_UCHAR)pAnlgInfo[i].pointNumber;
    }

    /* Add object data */
    switch(variation)
    {
    case 1:
      {
        TMWTYPES_USHORT tmp = dnputil_getAnlgDBandValueUShort(&pAnlgInfo[i].value);
        tmwtarg_store16(&tmp, &pTxData->tmw.pMsgBuf[pTxData->tmw.msgLength]);
        pTxData->tmw.msgLength += 2;
      }
      break;

    case 2:
      {
        TMWTYPES_ULONG tmp = dnputil_getAnlgDBandValueULong(&pAnlgInfo[i].value);
        tmwtarg_store32(&tmp, &pTxData->tmw.pMsgBuf[pTxData->tmw.msgLength]);
        pTxData->tmw.msgLength += 4;
      }
      break;

#if MDNPDATA_SUPPORT_OBJ34_V3
    case 3:
      {
        TMWTYPES_SFLOAT tmp = dnputil_getAnlgDBandValueFloat(&pAnlgInfo[i].value);
        tmwtarg_store32((TMWTYPES_ULONG *)&tmp, &pTxData->tmw.pMsgBuf[pTxData->tmw.msgLength]);
        pTxData->tmw.msgLength += 4;
      }
      break;
#endif
    default:
      {
        dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
        return(TMWDEFS_NULL);
      }
      break;
    }
  }

  /* Send request */
  if(pUserTxData == TMWDEFS_NULL)
  {
    /* Send request */ 
    if(!_sendFragment(pTxData))
    {
      pTxData = TMWDEFS_NULL;
    }
  }
  return((TMWSESN_TX_DATA *)pTxData);
}

/* function: _freezeObjectGroup */
static TMWSESN_TX_DATA * _freezeObjectGroup(
  MDNPBRM_REQ_DESC *pReqDesc,
  DNPCHNL_TX_DATA *pUserTxData,
  TMWTYPES_UCHAR funcCode,
  TMWTYPES_BOOL noAck,
  TMWTYPES_UCHAR qualifier,
  TMWTYPES_USHORT start,
  TMWTYPES_USHORT stopOrQty,
  TMWTYPES_BOOL feedbackRequested,
  DNPDEFS_OBJ_GROUP_ID objectGroup,
  DNPDATA_FREEZE_TIME_DATE_FIELD timeDateEnum,
  TMWDTIME *pFreezeTime,
  TMWTYPES_ULONG freezeInterval,
  DNPCHNL_CALLBACK_FUNC pCallback)
{
  DNPCHNL_TX_DATA *pTxData;
  TMWTYPES_MS_SINCE_70 msSince70;

  if((qualifier != DNPDEFS_QUAL_8BIT_START_STOP)
    && (qualifier != DNPDEFS_QUAL_16BIT_START_STOP)
    && (qualifier != DNPDEFS_QUAL_ALL_POINTS))
  {
    if(pUserTxData != TMWDEFS_NULL)
      dnpchnl_freeTxData((TMWSESN_TX_DATA *)pUserTxData);

    return(TMWDEFS_NULL);
  }

  if(pUserTxData == TMWDEFS_NULL)
  {
    /* Size is 2 Appl Header + 4 Object Header */
    /* if feedback is requested need 17 */
    TMWTYPES_USHORT size = 17;

    if (funcCode == DNPDEFS_FC_FRZ_TIME)
    {
      size += 14;
    }
#if MDNPDATA_SUPPORT_OBJ120
    if(pReqDesc->authAggressiveMode)
      size += DNPAUTH_AGGRESSIVE_SIZE;
#endif

    pTxData = (DNPCHNL_TX_DATA *)dnpchnl_newTxData(
      pReqDesc->pChannel, pReqDesc->pSession, size, 0);

    if(pTxData == TMWDEFS_NULL)
    {
      return(TMWDEFS_NULL);
    }

    if (objectGroup == DNPDEFS_OBJ_20_RUNNING_CNTRS)
    {
      /* Initialize Transmit Data Structure from Request Descriptor */
      if (!_initTxData(pTxData, pReqDesc, "Freeze Counters"))
      {
        dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
        return(TMWDEFS_NULL);
      }
    }
    else
    {
      /* Initialize Transmit Data Structure from Request Descriptor */
      if (!_initTxData(pTxData, pReqDesc, "Freeze Analog Inputs"))
      {
        dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
        return(TMWDEFS_NULL);
      }
    }

    /* Build request header */
    if(noAck)
    {
      mdnpbrm_buildRequestHeader(pTxData, funcCode + 1);
      pTxData->tmw.txFlags |= TMWSESN_TXFLAGS_NO_RESPONSE;
    }
    else
    {
      mdnpbrm_buildRequestHeader(pTxData, funcCode);
    }
    
#if MDNPDATA_SUPPORT_OBJ120
    if(pReqDesc->authAggressiveMode)
    {
      if(mdnpauth_addAggrRequestStart((TMWSESN_TX_DATA *)pTxData, pReqDesc->authUserNumber) == TMWDEFS_NULL)
      {
        dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
        return(TMWDEFS_NULL);
      }
    }
#endif 
  }
  else
  {
    TMWTYPES_UCHAR fc = funcCode;
    pTxData = pUserTxData;

    if((pUserTxData->tmw.msgLength + 7) >= pUserTxData->tmw.maxLength)
    {
      MDNPDIAG_ERROR(pReqDesc->pChannel, pReqDesc->pSession, MDNPDIAG_OBJ_LENGTH);
      pTxData = TMWDEFS_NULL;
    }
  
    if(noAck)
      fc += 1;

    if(pUserTxData->tmw.pMsgBuf[1] != fc)
    {
      MDNPDIAG_ERROR(pReqDesc->pChannel, pReqDesc->pSession, MDNPDIAG_FC);
      pTxData = TMWDEFS_NULL;
    }

    if(pTxData == TMWDEFS_NULL)
    {
      dnpchnl_freeTxData((TMWSESN_TX_DATA *)pUserTxData);
      return(TMWDEFS_NULL);
    }
  }

  if (funcCode == DNPDEFS_FC_FRZ_TIME)
  {
    /* Time Object Header */
    if(!mdnpbrm_addObjectHeader(pTxData, DNPDEFS_OBJ_50_TIME_AND_DATE, 2, DNPDEFS_QUAL_8BIT_LIMITED_QTY, 0, 1))
    {
      dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
      return(TMWDEFS_NULL);
    }
    /* Store the requested freeze time */
    if (timeDateEnum == DNPDATA_FREEZE_TIME_DATE_FIELD_ZERO)
    {
      msSince70.leastSignificant = 0;
      msSince70.mostSignificant = 0;
    }
    else if (timeDateEnum == DNPDATA_FREEZE_TIME_DATE_FIELD_CANCEL)
    {
      msSince70.leastSignificant = 0xffff;
      msSince70.mostSignificant = 0xffffffff;
    }
    else
    {
      dnpdtime_dateTimeToMSSince70(&msSince70, pFreezeTime);
      DNPDIAG_SHOW_FREEZE(pReqDesc->pSession, pFreezeTime, freezeInterval, 0);
    }
    dnpdtime_writeMsSince70(pTxData->tmw.pMsgBuf + pTxData->tmw.msgLength, &msSince70);
    pTxData->tmw.msgLength += 6;

    tmwtarg_store32(&freezeInterval, pTxData->tmw.pMsgBuf + pTxData->tmw.msgLength);
    pTxData->tmw.msgLength += 4;
  }

  /* Frozen Counter/Analog Object Header */
  if(!mdnpbrm_addObjectHeader(pTxData, objectGroup, 0, qualifier, start, stopOrQty))
  {
    dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
    return(TMWDEFS_NULL);
  }

  if(feedbackRequested)
  {
    OperateCallbackData *pCallbackData;
    if (pTxData->pInternalCallbackParam != TMWDEFS_NULL)
    {
      /* this request has multiple objects, cant add another callback structure */
      mdnpmem_free(pTxData->pInternalCallbackParam);
    }
    pCallbackData = (OperateCallbackData *)mdnpmem_alloc(MDNPMEM_OPER_CALLBACK_TYPE);
    if(pCallbackData == TMWDEFS_NULL)
    {
      dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
      return(TMWDEFS_NULL);
    }

    pCallbackData->pSession = pReqDesc->pSession;
    pCallbackData->pTxData = pTxData;
    tmwtimer_init(&pCallbackData->feedbackTimer);
 
    pCallbackData->operateRequired  = TMWDEFS_FALSE; 
    pCallbackData->binFeedbackRequired = TMWDEFS_TRUE;

    pTxData->pInternalCallback = pCallback;
    pTxData->pInternalCallbackParam = pCallbackData;
  }

  /* Send request */
  if(pUserTxData == TMWDEFS_NULL)
  {
    /* Send request */ 
    if(!_sendFragment(pTxData))
    {
      pTxData = TMWDEFS_NULL;
    }
  }
  return((TMWSESN_TX_DATA *)pTxData);
}

/* function: mdnpbrm_freezeCounters */
TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_freezeCounters(
  MDNPBRM_REQ_DESC *pReqDesc,
  DNPCHNL_TX_DATA *pUserTxData,
  TMWTYPES_BOOL clear,
  TMWTYPES_BOOL noAck,
  TMWTYPES_UCHAR qualifier,
  TMWTYPES_USHORT start,
  TMWTYPES_USHORT stopOrQty,
  TMWTYPES_BOOL feedbackRequested)
{
  TMWTYPES_UCHAR fc = DNPDEFS_FC_FRZ;
  if (clear)
    fc = DNPDEFS_FC_FRZ_CLEAR;

  return (_freezeObjectGroup(pReqDesc, pUserTxData, fc, noAck, qualifier, start, stopOrQty, feedbackRequested,
                             DNPDEFS_OBJ_20_RUNNING_CNTRS, DNPDATA_FREEZE_TIME_DATE_FIELD_ZERO, TMWDEFS_NULL, 0, _freezeCountersCallbackFunc));
}

/* function: mdnpbrm_freezeAnalogInputs */
TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_freezeAnalogInputs(
  MDNPBRM_REQ_DESC *pReqDesc,
  DNPCHNL_TX_DATA *pUserTxData,
  TMWTYPES_BOOL clear,
  TMWTYPES_BOOL noAck,
  TMWTYPES_UCHAR qualifier,
  TMWTYPES_USHORT start,
  TMWTYPES_USHORT stopOrQty,
  TMWTYPES_BOOL feedbackRequested)
{
  TMWTYPES_UCHAR fc = DNPDEFS_FC_FRZ;
  if (clear)
    fc = DNPDEFS_FC_FRZ_CLEAR;

  return (_freezeObjectGroup(pReqDesc, pUserTxData, fc, noAck, qualifier, start, stopOrQty, feedbackRequested,
                             DNPDEFS_OBJ_30_ANA_INPUTS, DNPDATA_FREEZE_TIME_DATE_FIELD_ZERO, TMWDEFS_NULL, 0, _freezeAnalogInputsCallbackFunc));
}

/* function: mdnpbrm_freezeAtTime */
TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_freezeAtTime(
  MDNPBRM_REQ_DESC *pReqDesc,
  DNPCHNL_TX_DATA *pUserTxData,
  TMWTYPES_BOOL noAck,
  DNPDEFS_OBJ_GROUP_ID objectGroup,
  TMWTYPES_UCHAR qualifier,
  DNPDATA_FREEZE_TIME_DATE_FIELD timeDateEnum,
  TMWDTIME *pFreezeTime,
  TMWTYPES_ULONG freezeInterval,
  TMWTYPES_USHORT start,
  TMWTYPES_USHORT stopOrQty,
  TMWTYPES_BOOL feedbackRequested)
{
  return (_freezeObjectGroup(pReqDesc, pUserTxData, DNPDEFS_FC_FRZ_TIME, noAck, qualifier, start, stopOrQty, feedbackRequested,
                             objectGroup, timeDateEnum, pFreezeTime, freezeInterval, _freezeAnalogInputsCallbackFunc));
}

/* function: mdnpbrm_writeString */
TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_writeString(
  MDNPBRM_REQ_DESC *pReqDesc,
  DNPCHNL_TX_DATA *pUserTxData,
  TMWTYPES_USHORT point,
  TMWTYPES_UCHAR *pValue,
  TMWTYPES_UCHAR length)
{
  DNPCHNL_TX_DATA *pTxData;
  TMWTYPES_UCHAR qualifier;
  TMWTYPES_USHORT size;
    
  qualifier = (TMWTYPES_UCHAR)((point > 255) ? DNPDEFS_QUAL_16BIT_INDEX : DNPDEFS_QUAL_8BIT_INDEX);
  size = (TMWTYPES_USHORT)(9 + length);

  if(pUserTxData == TMWDEFS_NULL)
  {
    
#if MDNPDATA_SUPPORT_OBJ120
    if(pReqDesc->authAggressiveMode)
      size += DNPAUTH_AGGRESSIVE_SIZE;
#endif

    /* Size is 2 Appl Header + 5 Object Header + point number + length */
    pTxData = (DNPCHNL_TX_DATA *)dnpchnl_newTxData(
      pReqDesc->pChannel, pReqDesc->pSession, size, 0);

    if(pTxData == TMWDEFS_NULL)
    {
      return(TMWDEFS_NULL);
    } 

    /* Initialize Transmit Data Structure from Request Descriptor */
    if(!_initTxData(pTxData, pReqDesc, "Write String"))
    {
      dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
      return(TMWDEFS_NULL);
    }

    /* Build request header */
    mdnpbrm_buildRequestHeader(pTxData, DNPDEFS_FC_WRITE);

#if MDNPDATA_SUPPORT_OBJ120
    if(pReqDesc->authAggressiveMode)
    {
      if(mdnpauth_addAggrRequestStart((TMWSESN_TX_DATA *)pTxData, pReqDesc->authUserNumber) == TMWDEFS_NULL)
      {
        dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
        return(TMWDEFS_NULL);
      }
    }
#endif 
  }
  else
  {
    pTxData = pUserTxData;
    if((pUserTxData->tmw.msgLength + size) >= pUserTxData->tmw.maxLength)
    {
      MDNPDIAG_ERROR(pReqDesc->pChannel, pReqDesc->pSession, MDNPDIAG_OBJ_LENGTH);
      pTxData = TMWDEFS_NULL;
    }

    if(pUserTxData->tmw.pMsgBuf[1] != DNPDEFS_FC_WRITE)
    {
      MDNPDIAG_ERROR(pReqDesc->pChannel, pReqDesc->pSession, MDNPDIAG_FC);
      pTxData = TMWDEFS_NULL;
    }

    if(pTxData == TMWDEFS_NULL)
    {
      dnpchnl_freeTxData((TMWSESN_TX_DATA *)pUserTxData);
      return(TMWDEFS_NULL);
    }
  }

  /* Time Object Header */
  if(!mdnpbrm_addObjectHeader(pTxData, DNPDEFS_OBJ_110_STRING_DATA, length, qualifier, 0, 1))
  {
    dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
    return(TMWDEFS_NULL);
  }

  /* Index */
  if(qualifier == DNPDEFS_QUAL_16BIT_INDEX)
  {
    tmwtarg_store16(&point, pTxData->tmw.pMsgBuf + pTxData->tmw.msgLength);
    pTxData->tmw.msgLength += 2;
  }
  else
  {
    pTxData->tmw.pMsgBuf[pTxData->tmw.msgLength++] = (TMWTYPES_UCHAR)point;
  }

  /* Copy string into message */
  memcpy(&pTxData->tmw.pMsgBuf[pTxData->tmw.msgLength], pValue, length);
  pTxData->tmw.msgLength = (TMWTYPES_USHORT)(pTxData->tmw.msgLength + length);

  /* Send request */
  if(pUserTxData == TMWDEFS_NULL)
  {
    /* Send request */ 
    if(!_sendFragment(pTxData))
    {
      pTxData = TMWDEFS_NULL;
    }
  }
  return((TMWSESN_TX_DATA *)pTxData);
}

TMWDEFS_SCL_API TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_writeStrings(
  MDNPBRM_REQ_DESC *pReqDesc,
  DNPCHNL_TX_DATA *pUserTxData,
  TMWTYPES_UCHAR qualifier,
  TMWTYPES_UCHAR numObjects,
  MDNPBRM_STRING_INFO *pStringInfo)
{
  DNPCHNL_TX_DATA *pTxData;
  TMWTYPES_USHORT size = 9;
  /* Since the variation specifies the string length, all strings must be the same length */
  TMWTYPES_UCHAR length = (TMWTYPES_UCHAR)strlen(pStringInfo[0].value);
  int i;
    
  size += length * numObjects;

  if((qualifier != DNPDEFS_QUAL_8BIT_INDEX)
    && (qualifier != DNPDEFS_QUAL_16BIT_INDEX)
    && (qualifier != DNPDEFS_QUAL_8BIT_START_STOP)
    && (qualifier != DNPDEFS_QUAL_16BIT_START_STOP))
  {
    MDNPDIAG_ERROR(pReqDesc->pChannel, pReqDesc->pSession, MDNPDIAG_INV_QUALIFIER);
    return(TMWDEFS_NULL);
  }

  if(pUserTxData == TMWDEFS_NULL)
  {
    
#if MDNPDATA_SUPPORT_OBJ120
    if(pReqDesc->authAggressiveMode)
      size += DNPAUTH_AGGRESSIVE_SIZE;
#endif

    /* Size is 2 Appl Header + 5 Object Header + point number + length */
    pTxData = (DNPCHNL_TX_DATA *)dnpchnl_newTxData(
      pReqDesc->pChannel, pReqDesc->pSession, size, 0);

    if(pTxData == TMWDEFS_NULL)
    {
      return(TMWDEFS_NULL);
    } 

    /* Initialize Transmit Data Structure from Request Descriptor */
    if(!_initTxData(pTxData, pReqDesc, "Write String"))
    {
      dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
      return(TMWDEFS_NULL);
    }

    /* Build request header */
    mdnpbrm_buildRequestHeader(pTxData, DNPDEFS_FC_WRITE);

#if MDNPDATA_SUPPORT_OBJ120
    if(pReqDesc->authAggressiveMode)
    {
      if(mdnpauth_addAggrRequestStart((TMWSESN_TX_DATA *)pTxData, pReqDesc->authUserNumber) == TMWDEFS_NULL)
      {
        dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
        return(TMWDEFS_NULL);
      }
    }
#endif 
  }
  else
  {
    pTxData = pUserTxData;
    if((pUserTxData->tmw.msgLength + size) >= pUserTxData->tmw.maxLength)
    {
      MDNPDIAG_ERROR(pReqDesc->pChannel, pReqDesc->pSession, MDNPDIAG_OBJ_LENGTH);
      pTxData = TMWDEFS_NULL;
    }

    if(pUserTxData->tmw.pMsgBuf[1] != DNPDEFS_FC_WRITE)
    {
      MDNPDIAG_ERROR(pReqDesc->pChannel, pReqDesc->pSession, MDNPDIAG_FC);
      pTxData = TMWDEFS_NULL;
    }

    if(pTxData == TMWDEFS_NULL)
    {
      dnpchnl_freeTxData((TMWSESN_TX_DATA *)pUserTxData);
      return(TMWDEFS_NULL);
    }
  }

  /* Add object header */
  if ((qualifier == DNPDEFS_QUAL_8BIT_START_STOP)
    || (qualifier == DNPDEFS_QUAL_16BIT_START_STOP))
  {
    TMWTYPES_USHORT start = pStringInfo[0].pointNumber;
    if(!mdnpbrm_addObjectHeader(pTxData, DNPDEFS_OBJ_110_STRING_DATA, length, qualifier, start, start+numObjects-1))
    {
      dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
      return(TMWDEFS_NULL);
    }
  }
  else
  {
    if (!mdnpbrm_addObjectHeader(pTxData, DNPDEFS_OBJ_110_STRING_DATA, length, qualifier, 0, numObjects))
    {
      dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
      return(TMWDEFS_NULL);
    }
  }

  for (i = 0; i < numObjects; i++)
  {
    /* Index */
    if (qualifier == DNPDEFS_QUAL_16BIT_INDEX)
    {
      tmwtarg_store16(&pStringInfo[i].pointNumber, pTxData->tmw.pMsgBuf + pTxData->tmw.msgLength);
      pTxData->tmw.msgLength += 2;
    }
    else if (qualifier == DNPDEFS_QUAL_8BIT_INDEX)
    {
      pTxData->tmw.pMsgBuf[pTxData->tmw.msgLength++] = (TMWTYPES_UCHAR)pStringInfo[i].pointNumber;
    }

    /* Copy string into message */
    memcpy(&pTxData->tmw.pMsgBuf[pTxData->tmw.msgLength], pStringInfo[i].value, length);
    pTxData->tmw.msgLength = (TMWTYPES_USHORT)(pTxData->tmw.msgLength + length);
  }

  /* Send request */
  if(pUserTxData == TMWDEFS_NULL)
  {
    /* Send request */ 
    if(!_sendFragment(pTxData))
    {
      pTxData = TMWDEFS_NULL;
    }
  }
  return((TMWSESN_TX_DATA *)pTxData);
}

#if MDNPDATA_SUPPORT_OBJ91
/* function: mdnpbrm_activateConfig */
TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_activateConfig(
  MDNPBRM_REQ_DESC *pReqDesc,
  DNPCHNL_TX_DATA *pUserTxData,
  TMWTYPES_USHORT point,
  TMWTYPES_UCHAR *pValue,
  TMWTYPES_USHORT length,
  DNPDEFS_OBJ_GROUP_ID objectGroup)
{
  DNPCHNL_TX_DATA *pTxData;
  TMWTYPES_UCHAR qualifier;
  TMWTYPES_USHORT size;
  TMWTYPES_BOOL addingToExisting = TMWDEFS_FALSE;
    
  /* Size is 2 Appl Header + 5 Object Header + point number + length */
  /* allow room for authentication also */
  size = (TMWTYPES_USHORT)(29 + length);

  if(pUserTxData == TMWDEFS_NULL)
  {
    pTxData = (DNPCHNL_TX_DATA *)dnpchnl_newTxData(
      pReqDesc->pChannel, pReqDesc->pSession, size, 0);

    if(pTxData == TMWDEFS_NULL)
    {
      return(TMWDEFS_NULL);
    }

    /* Initialize Transmit Data Structure from Request Descriptor */
    if(!_initTxData(pTxData, pReqDesc, "Activate Configuration"))
    {
      dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
      return(TMWDEFS_NULL);
    }

    /* Build request header */
    mdnpbrm_buildRequestHeader(pTxData, DNPDEFS_FC_ACTIVATE_CONFIG);
  }
  else
  {
    pTxData = pUserTxData;
    if((pUserTxData->tmw.msgLength + size) >= pUserTxData->tmw.maxLength)
    {
      MDNPDIAG_ERROR(pReqDesc->pChannel, pReqDesc->pSession, MDNPDIAG_OBJ_LENGTH);
      pTxData = TMWDEFS_NULL;
    }

    if(pUserTxData->tmw.pMsgBuf[1] != DNPDEFS_FC_ACTIVATE_CONFIG)
    {
      MDNPDIAG_ERROR(pReqDesc->pChannel, pReqDesc->pSession, MDNPDIAG_FC);
      pTxData = TMWDEFS_NULL;
    }

    /* If either of the above checks failed, pTxData would have been nulled out */
    if(pTxData == TMWDEFS_NULL)
    {
      dnpchnl_freeTxData((TMWSESN_TX_DATA *)pUserTxData);
      return(TMWDEFS_NULL);
    }

    /* See if there is an existing file object this can be added to.  
     * This requires all objects to be file objects in order to use just one header  
     * It would be possible to add this new file object to the last object in the request 
     * if it was a file object, but this is more complicated than needed.
     * Otherwise, a new object header will be added.
     */
    if((objectGroup == 70) && (pUserTxData->tmw.msgLength > 8))
    { 
      if(pTxData->tmw.pMsgBuf[4] == DNPDEFS_QUAL_16BIT_FREE_FORMAT)
      {
        int i;
        TMWTYPES_USHORT nameLength;
        MDNPSESN *pMDNPSession = (MDNPSESN *)pTxData->tmw.pSession;

        /*  verify there is just one file object in request   */
        TMWTYPES_UCHAR quantity = pTxData->tmw.pMsgBuf[5];
        TMWTYPES_USHORT index = 6;
        for(i = 0; i<quantity; i++)
        {
          tmwtarg_get16(pTxData->tmw.pMsgBuf + index, &nameLength); 
          index += nameLength+2;
        } 
        
        /* This is too complicated for some slaves, even though it is implied in spec. 
         */
         if((pMDNPSession->combineActConfigData) 
         && (index == pUserTxData->tmw.msgLength))
           addingToExisting = TMWDEFS_TRUE;
      }
    }
  }

  if(objectGroup == 110)
  {
    if(length > 255)
    {
      dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
      return(TMWDEFS_NULL);
    }

    qualifier = (TMWTYPES_UCHAR)((point > 255) ? DNPDEFS_QUAL_16BIT_INDEX : DNPDEFS_QUAL_8BIT_INDEX);

    /* Object Header,  variation is string length */
    if(!mdnpbrm_addObjectHeader(pTxData, DNPDEFS_OBJ_110_STRING_DATA, (TMWTYPES_UCHAR)length, qualifier, 0, 1))
    {
      dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
      return(TMWDEFS_NULL);
    }

    /* Index */
    if(qualifier == DNPDEFS_QUAL_16BIT_INDEX)
    {
      tmwtarg_store16(&point, pTxData->tmw.pMsgBuf + pTxData->tmw.msgLength);
      pTxData->tmw.msgLength += 2;
    }
    else
    {
      pTxData->tmw.pMsgBuf[pTxData->tmw.msgLength++] = (TMWTYPES_UCHAR)point;
    }

    /* Copy string into message */
    memcpy(&pTxData->tmw.pMsgBuf[pTxData->tmw.msgLength], pValue, length);
    pTxData->tmw.msgLength = (TMWTYPES_USHORT)(pTxData->tmw.msgLength + length);
  }
  else if(objectGroup == 70)
  {
    if(!addingToExisting)
    {
      /* Object Header */
      if(!mdnpbrm_addObjectHeader(pTxData, DNPDEFS_OBJ_70_FILE_IDENTIFIER, 8, DNPDEFS_QUAL_16BIT_FREE_FORMAT, 0, 1))
      {
        dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
        return(TMWDEFS_NULL);
      }
    }
    else
    {
      TMWTYPES_UCHAR quantity = pTxData->tmw.pMsgBuf[5];
      quantity++;
      pTxData->tmw.pMsgBuf[5] = quantity;
    }

    /* length of string */
    tmwtarg_store16(&length, pTxData->tmw.pMsgBuf + pTxData->tmw.msgLength);
    pTxData->tmw.msgLength += 2;

    /* Copy string into message */
    memcpy(&pTxData->tmw.pMsgBuf[pTxData->tmw.msgLength], pValue, length);
    pTxData->tmw.msgLength = (TMWTYPES_USHORT)(pTxData->tmw.msgLength + length);
  }
  else  
  {
    dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
    return(TMWDEFS_NULL);
  }

  /* Send request */
  if(pUserTxData == TMWDEFS_NULL)
  {
    /* Send request */ 
    if(!_sendFragment(pTxData))
    {
      pTxData = TMWDEFS_NULL;
    }
  }
  return((TMWSESN_TX_DATA *)pTxData);
}
#endif

/* function: mdnpbrm_writeVirtualTerminal */
TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_writeVirtualTerminal(
  MDNPBRM_REQ_DESC *pReqDesc,
  DNPCHNL_TX_DATA *pUserTxData,
  TMWTYPES_USHORT point,
  TMWTYPES_UCHAR *pValue,
  TMWTYPES_UCHAR length)
{
  DNPCHNL_TX_DATA *pTxData;
  TMWTYPES_UCHAR qualifier;
  TMWTYPES_USHORT size;
  
  qualifier = (TMWTYPES_UCHAR)((point > 255) ? DNPDEFS_QUAL_16BIT_INDEX : DNPDEFS_QUAL_8BIT_INDEX);

  /* Size is 2 Appl Header + 5 Object Header + point number + length */
  size = (TMWTYPES_USHORT)(9 + length);

  if(pUserTxData == TMWDEFS_NULL)
  {
#if MDNPDATA_SUPPORT_OBJ120
    if(pReqDesc->authAggressiveMode)
      size += DNPAUTH_AGGRESSIVE_SIZE;
#endif

    pTxData = (DNPCHNL_TX_DATA *)dnpchnl_newTxData(
      pReqDesc->pChannel, pReqDesc->pSession, size, 0);

    if(pTxData == TMWDEFS_NULL)
    {
      return(TMWDEFS_NULL);
    }

    /* Initialize Transmit Data Structure from Request Descriptor */
    if(!_initTxData(pTxData, pReqDesc, "Write Virtual Terminal"))
    {
      dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
      return(TMWDEFS_NULL);
    }

    /* Build request header */
    mdnpbrm_buildRequestHeader(pTxData, DNPDEFS_FC_WRITE);
    
#if MDNPDATA_SUPPORT_OBJ120
    if(pReqDesc->authAggressiveMode)
    {
      if(mdnpauth_addAggrRequestStart((TMWSESN_TX_DATA *)pTxData, pReqDesc->authUserNumber) == TMWDEFS_NULL)
      {
        dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
        return(TMWDEFS_NULL);
      }
    }
#endif 

  }
  else
  {
    pTxData = pUserTxData;
    if((pUserTxData->tmw.msgLength + size) >= pUserTxData->tmw.maxLength)
    {
      MDNPDIAG_ERROR(pReqDesc->pChannel, pReqDesc->pSession, MDNPDIAG_OBJ_LENGTH);
      pTxData = TMWDEFS_NULL;
    }

    if(pUserTxData->tmw.pMsgBuf[1] != DNPDEFS_FC_WRITE)
    {
      MDNPDIAG_ERROR(pReqDesc->pChannel, pReqDesc->pSession, MDNPDIAG_FC);
      pTxData = TMWDEFS_NULL;
    }

    if(pTxData == TMWDEFS_NULL)
    {
      dnpchnl_freeTxData((TMWSESN_TX_DATA *)pUserTxData);
      return(TMWDEFS_NULL);
    }
  }

  /* Virtual Terminal Object Header */
  if(!mdnpbrm_addObjectHeader(pTxData, DNPDEFS_OBJ_112_VTERM_OUTPUT, length, qualifier, 0, 1))
  {
    dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
    return(TMWDEFS_NULL);
  }

  /* Index */
  if(qualifier == DNPDEFS_QUAL_16BIT_INDEX)
  {
    tmwtarg_store16(&point, pTxData->tmw.pMsgBuf + pTxData->tmw.msgLength);
    pTxData->tmw.msgLength += 2;
  }
  else
  {
    pTxData->tmw.pMsgBuf[pTxData->tmw.msgLength++] = (TMWTYPES_UCHAR)point;
  }

  /* Copy string into message */
  memcpy(&pTxData->tmw.pMsgBuf[pTxData->tmw.msgLength], pValue, length);
  pTxData->tmw.msgLength = (TMWTYPES_USHORT)(pTxData->tmw.msgLength + length);

  /* Send request */
  if(pUserTxData == TMWDEFS_NULL)
  {
    /* Send request */ 
    if(!_sendFragment(pTxData))
    {
      pTxData = TMWDEFS_NULL;
    }
  }
  return((TMWSESN_TX_DATA *)pTxData);
}

/* function: mdnpbrm_writeExtString */
TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_writeExtString(
  MDNPBRM_REQ_DESC *pReqDesc,
  DNPCHNL_TX_DATA *pUserTxData,
  TMWTYPES_USHORT point,
  TMWTYPES_UCHAR *pValue,
  TMWTYPES_USHORT length)
{
  DNPCHNL_TX_DATA *pTxData;
  TMWTYPES_UCHAR qualifier;
  TMWTYPES_UCHAR variation;
  TMWTYPES_USHORT size;
    
  qualifier = (TMWTYPES_UCHAR)((point > 255) ? DNPDEFS_QUAL_16BIT_INDEX : DNPDEFS_QUAL_8BIT_INDEX);
  variation = (length > 255) ? 2 : 1;
  size = (TMWTYPES_USHORT)(9 + length);

  if(pUserTxData == TMWDEFS_NULL)
  {
    
#if MDNPDATA_SUPPORT_OBJ120
    if(pReqDesc->authAggressiveMode)
      size += DNPAUTH_AGGRESSIVE_SIZE;
#endif

    /* Size is 2 Appl Header + 5 Object Header + point number + length */
    pTxData = (DNPCHNL_TX_DATA *)dnpchnl_newTxData(
      pReqDesc->pChannel, pReqDesc->pSession, size, 0);

    if(pTxData == TMWDEFS_NULL)
    {
      return(TMWDEFS_NULL);
    } 

    /* Initialize Transmit Data Structure from Request Descriptor */
    if(!_initTxData(pTxData, pReqDesc, "Write Extended String"))
    {
      dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
      return(TMWDEFS_NULL);
    }

    /* Build request header */
    mdnpbrm_buildRequestHeader(pTxData, DNPDEFS_FC_WRITE);

#if MDNPDATA_SUPPORT_OBJ120
    if(pReqDesc->authAggressiveMode)
    {
      if(mdnpauth_addAggrRequestStart((TMWSESN_TX_DATA *)pTxData, pReqDesc->authUserNumber) == TMWDEFS_NULL)
      {
        dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
        return(TMWDEFS_NULL);
      }
    }
#endif 
  }
  else
  {
    pTxData = pUserTxData;
    if((pUserTxData->tmw.msgLength + size) >= pUserTxData->tmw.maxLength)
    {
      MDNPDIAG_ERROR(pReqDesc->pChannel, pReqDesc->pSession, MDNPDIAG_OBJ_LENGTH);
      pTxData = TMWDEFS_NULL;
    }

    if(pUserTxData->tmw.pMsgBuf[1] != DNPDEFS_FC_WRITE)
    {
      MDNPDIAG_ERROR(pReqDesc->pChannel, pReqDesc->pSession, MDNPDIAG_FC);
      pTxData = TMWDEFS_NULL;
    }

    if(pTxData == TMWDEFS_NULL)
    {
      dnpchnl_freeTxData((TMWSESN_TX_DATA *)pUserTxData);
      return(TMWDEFS_NULL);
    }
  }

  /* Object Header */
  if(!mdnpbrm_addObjectHeader(pTxData, DNPDEFS_OBJ_114_EXT_STR_DATA, variation, qualifier, 0, 1))
  {
    dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
    return(TMWDEFS_NULL);
  }

  /* Index */
  if(qualifier == DNPDEFS_QUAL_16BIT_INDEX)
  {
    tmwtarg_store16(&point, pTxData->tmw.pMsgBuf + pTxData->tmw.msgLength);
    pTxData->tmw.msgLength += 2;
  }
  else
  {
    pTxData->tmw.pMsgBuf[pTxData->tmw.msgLength++] = (TMWTYPES_UCHAR)point;
  }

  /* Put Length into message */
  if (length >255)
  {
    tmwtarg_store16(&length, pTxData->tmw.pMsgBuf + pTxData->tmw.msgLength);
    pTxData->tmw.msgLength += 2;
  }
  else
  {
    pTxData->tmw.pMsgBuf[pTxData->tmw.msgLength++] = (TMWTYPES_UCHAR)length;
  }

  /* Copy string into message */
  memcpy(&pTxData->tmw.pMsgBuf[pTxData->tmw.msgLength], pValue, length);
  pTxData->tmw.msgLength = (TMWTYPES_USHORT)(pTxData->tmw.msgLength + length);

  /* Send request */
  if(pUserTxData == TMWDEFS_NULL)
  {
    /* Send request */ 
    if(!_sendFragment(pTxData))
    {
      pTxData = TMWDEFS_NULL;
    }
  }
  return((TMWSESN_TX_DATA *)pTxData);
}

TMWDEFS_SCL_API TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_writeExtStrings(
  MDNPBRM_REQ_DESC *pReqDesc,
  DNPCHNL_TX_DATA *pUserTxData,
  TMWTYPES_UCHAR qualifier,
  TMWTYPES_UCHAR variation,
  TMWTYPES_UCHAR numObjects,
  MDNPBRM_EXT_STRING_INFO *pStringInfo)
{
  DNPCHNL_TX_DATA *pTxData;
  TMWTYPES_ULONG size = 9;
  TMWTYPES_USHORT length;
  int i;
    
  if((qualifier != DNPDEFS_QUAL_8BIT_INDEX)
    && (qualifier != DNPDEFS_QUAL_16BIT_INDEX)
    && (qualifier != DNPDEFS_QUAL_8BIT_START_STOP)
    && (qualifier != DNPDEFS_QUAL_16BIT_START_STOP))
  {
    MDNPDIAG_ERROR(pReqDesc->pChannel, pReqDesc->pSession, MDNPDIAG_INV_QUALIFIER);
    return(TMWDEFS_NULL);
  }

  /* Validate input arguments */
  if((variation != 1) && (variation != 2))
  {
    MDNPDIAG_ERROR(pReqDesc->pChannel, pReqDesc->pSession, MDNPDIAG_INV_VARIATION);
    return(TMWDEFS_NULL);
  } 

  /* Calculate the size using string length + the width of the size (based on variation) */
  for (i = 0; i < numObjects; i++)
  {
    size += (TMWTYPES_ULONG)strlen(pStringInfo[i].value) + variation;
  }


  if(pUserTxData == TMWDEFS_NULL)
  {
    
#if MDNPDATA_SUPPORT_OBJ120
    if(pReqDesc->authAggressiveMode)
      size += DNPAUTH_AGGRESSIVE_SIZE;
#endif

    /* USHORT is the limit in the SCL. */
    if (size > 65535)
      return(TMWDEFS_NULL);

    /* Size is 2 Appl Header + 5 Object Header + point number + length */
    pTxData = (DNPCHNL_TX_DATA *)dnpchnl_newTxData(
      pReqDesc->pChannel, pReqDesc->pSession, (TMWTYPES_USHORT)size, 0);

    if(pTxData == TMWDEFS_NULL)
    {
      return(TMWDEFS_NULL);
    } 

    /* Initialize Transmit Data Structure from Request Descriptor */
    if(!_initTxData(pTxData, pReqDesc, "Write Extended String"))
    {
      dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
      return(TMWDEFS_NULL);
    }

    /* Build request header */
    mdnpbrm_buildRequestHeader(pTxData, DNPDEFS_FC_WRITE);

#if MDNPDATA_SUPPORT_OBJ120
    if(pReqDesc->authAggressiveMode)
    {
      if(mdnpauth_addAggrRequestStart((TMWSESN_TX_DATA *)pTxData, pReqDesc->authUserNumber) == TMWDEFS_NULL)
      {
        dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
        return(TMWDEFS_NULL);
      }
    }
#endif 
  }
  else
  {
    pTxData = pUserTxData;
    if((pUserTxData->tmw.msgLength + size) >= pUserTxData->tmw.maxLength)
    {
      MDNPDIAG_ERROR(pReqDesc->pChannel, pReqDesc->pSession, MDNPDIAG_OBJ_LENGTH);
      pTxData = TMWDEFS_NULL;
    }

    if(pUserTxData->tmw.pMsgBuf[1] != DNPDEFS_FC_WRITE)
    {
      MDNPDIAG_ERROR(pReqDesc->pChannel, pReqDesc->pSession, MDNPDIAG_FC);
      pTxData = TMWDEFS_NULL;
    }

    if(pTxData == TMWDEFS_NULL)
    {
      dnpchnl_freeTxData((TMWSESN_TX_DATA *)pUserTxData);
      return(TMWDEFS_NULL);
    }
  }

  /* Add object header */
  if ((qualifier == DNPDEFS_QUAL_8BIT_START_STOP)
    || (qualifier == DNPDEFS_QUAL_16BIT_START_STOP))
  {
    TMWTYPES_USHORT start = pStringInfo[0].pointNumber;
    if(!mdnpbrm_addObjectHeader(pTxData, DNPDEFS_OBJ_114_EXT_STR_DATA, variation, qualifier, start, start+numObjects-1))
    {
      dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
      return(TMWDEFS_NULL);
    }
  }
  else
  {
    if (!mdnpbrm_addObjectHeader(pTxData, DNPDEFS_OBJ_114_EXT_STR_DATA, variation, qualifier, 0, numObjects))
    {
      dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
      return(TMWDEFS_NULL);
    }
  }

  for (i = 0; i < numObjects; i++)
  {
    /* Index */
    if (qualifier == DNPDEFS_QUAL_16BIT_INDEX)
    {
      tmwtarg_store16(&pStringInfo[i].pointNumber, pTxData->tmw.pMsgBuf + pTxData->tmw.msgLength);
      pTxData->tmw.msgLength += 2;
    }
    else if (qualifier == DNPDEFS_QUAL_8BIT_INDEX)
    {
      pTxData->tmw.pMsgBuf[pTxData->tmw.msgLength++] = (TMWTYPES_UCHAR)pStringInfo[i].pointNumber;
    }

    /* Put Length into message */
    length = (TMWTYPES_USHORT)strlen(pStringInfo[i].value);
    if (variation == 2)
    {
      tmwtarg_store16(&length, pTxData->tmw.pMsgBuf + pTxData->tmw.msgLength);
      pTxData->tmw.msgLength += 2;
    }
    else
    {
      if (length > 255)
      {
        dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
        char pointNum[256];
        sprintf(pointNum, "Ext String Point %d, String Length %ld, Variation %d", pStringInfo[i].pointNumber, strlen(pStringInfo[i].value), variation);
        MDNPDIAG_ERROR_MSG(pReqDesc->pChannel, pReqDesc->pSession, MDNPDIAG_INV_VARIATION_VALUE, pointNum);
        return(TMWDEFS_NULL);
      }
      pTxData->tmw.pMsgBuf[pTxData->tmw.msgLength++] = (TMWTYPES_UCHAR)length;
    }

    /* Copy string into message */
    memcpy(&pTxData->tmw.pMsgBuf[pTxData->tmw.msgLength], pStringInfo[i].value, length);
    pTxData->tmw.msgLength = (TMWTYPES_USHORT)(pTxData->tmw.msgLength + length);
  }

  /* Send request */
  if(pUserTxData == TMWDEFS_NULL)
  {
    /* Send request */ 
    if(!_sendFragment(pTxData))
    {
      pTxData = TMWDEFS_NULL;
    }
  }
  return((TMWSESN_TX_DATA *)pTxData);
}

#if MDNPDATA_SUPPORT_OBJ70
/* function: mdnpbrm_fileInfo */
TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_fileInfo(
  MDNPBRM_REQ_DESC *pReqDesc,
  TMWTYPES_USHORT requestId,
  const TMWTYPES_CHAR *pFilename)
{
  return(mdnpfile_fileInfo(pReqDesc, requestId,pFilename));
}

/* function: mdnpbrm_fileAuthentication */
TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_fileAuthentication(
  MDNPBRM_REQ_DESC *pReqDesc,
  TMWTYPES_CHAR *pUsername,
  TMWTYPES_CHAR *pPassword)
{
  return(mdnpfile_fileAuthentication(pReqDesc, pUsername, pPassword));
}

/* function: mdnpbrm_fileOpen */
TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_fileOpen(
  MDNPBRM_REQ_DESC *pReqDesc,
  TMWTYPES_USHORT requestId,
  DNPDEFS_FILE_MODE mode,
  TMWTYPES_USHORT permissions,
  DNPDEFS_FILE_TYPE fileType,
  TMWTYPES_ULONG authenticationKey,
  TMWTYPES_USHORT maxBlockSize,
  const TMWTYPES_CHAR *pFilename)
{
  MDNPSESN *pMDNPSession;
  TMWTYPES_ULONG fileSize; 
  TMWTYPES_MS_SINCE_70 timeOfCreation;

  pMDNPSession = (MDNPSESN *)pReqDesc->pSession;
  pMDNPSession->fileXferType = fileType;

  timeOfCreation.leastSignificant = 0;
  timeOfCreation.mostSignificant = 0;

  /* For writing and appending mdnpbrm_fileOpenForWrite should be called. 
   * However, if this function is called and the size and time of creation are not known 
   * 0xffffffff and 0 ms since epoch should be sent 
   */ 
  if(mode != DNPDEFS_FILE_MODE_READ)
    fileSize = 0xffffffff;
  else
    fileSize = 0;

  return(mdnpfile_fileOpen(pReqDesc, requestId, mode, &timeOfCreation, permissions, authenticationKey, fileSize, maxBlockSize, pFilename));
}

/* function: mdnpbrm_fileOpenForWrite */
TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_fileOpenForWrite(
  MDNPBRM_REQ_DESC *pReqDesc,
  TMWTYPES_USHORT requestId,
  DNPDEFS_FILE_MODE mode,
  TMWTYPES_USHORT permissions,
  DNPDEFS_FILE_TYPE fileType,
  TMWTYPES_ULONG authenticationKey,
  TMWTYPES_USHORT maxBlockSize,
  const TMWTYPES_CHAR *pFilename,
  TMWDTIME *pTimeOfCreation,
  TMWTYPES_ULONG fileSize)
{
  MDNPSESN *pMDNPSession; 
  TMWTYPES_MS_SINCE_70 msSince70;

  pMDNPSession = (MDNPSESN *)pReqDesc->pSession;
  pMDNPSession->fileXferType = fileType;
  
  if(mode == DNPDEFS_FILE_MODE_READ)
  {
    msSince70.leastSignificant = 0;
    msSince70.mostSignificant = 0;
    fileSize = 0;
  }
  else
  {
    dnpdtime_dateTimeToMSSince70(&msSince70, pTimeOfCreation);
  }

  return(mdnpfile_fileOpen(pReqDesc, requestId, mode, &msSince70, permissions, authenticationKey, fileSize, maxBlockSize, pFilename));
}

/* function: mdnpbrm_fileClose */
TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_fileClose(
  MDNPBRM_REQ_DESC *pReqDesc,
  TMWTYPES_USHORT requestId,
  TMWTYPES_ULONG handle)
{
  return(mdnpfile_fileClose( pReqDesc, requestId, handle));
}

/* function: mdnpbrm_fileDelete */
TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_fileDelete(
  MDNPBRM_REQ_DESC *pReqDesc,
  TMWTYPES_USHORT requestId,
  TMWTYPES_ULONG authenticationKey,
  const TMWTYPES_CHAR *pUserName,
  const TMWTYPES_CHAR *pPassword,
  const TMWTYPES_CHAR *pFilename)
{
  return(mdnpfile_fileDelete(pReqDesc, requestId, 
    authenticationKey, pUserName, pPassword, pFilename));
}

/* function: mdnpbrm_fileAbort */
TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_fileAbort(
  MDNPBRM_REQ_DESC *pReqDesc,
  TMWTYPES_USHORT requestId,
  TMWTYPES_ULONG handle)
{
  return(mdnpfile_fileAbort(pReqDesc, requestId, handle));
}

/* function: mdnpbrm_fileRead */
TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_fileRead(
  MDNPBRM_REQ_DESC *pReqDesc,
  TMWTYPES_ULONG block,
  TMWTYPES_ULONG handle)
{
  return(mdnpfile_fileRead( pReqDesc, block, handle));
}

/* function: mdnpbrm_fileWrite */
TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_fileWrite(
  MDNPBRM_REQ_DESC *pReqDesc,
  TMWTYPES_ULONG block,
  TMWTYPES_BOOL last,
  TMWTYPES_ULONG handle,
  const TMWTYPES_UCHAR *pData,
  TMWTYPES_USHORT length)
{
  DNPCHNL *pDNPChannel = (DNPCHNL*)pReqDesc->pSession->pChannel;
  /* See if this will fit in a tx fragment */
  if(length +16 > pDNPChannel->txFragmentSize)
    return TMWDEFS_NULL;

  return(mdnpfile_fileWrite(pReqDesc, block, last, handle, pData, length));
}

/* function: mdnpbrm_copyLocalFileToRemote  */
TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_copyLocalFileToRemote(
  MDNPBRM_REQ_DESC *pReqDesc,
  const TMWTYPES_CHAR *pLocalFileName,
  const TMWTYPES_CHAR *pRemoteFileName,
  const TMWTYPES_CHAR *pUserName,
  const TMWTYPES_CHAR *pPassword,
  const TMWTYPES_USHORT permissions)
{
  return (mdnpfile_copyLocalFileToRemote(pReqDesc, pLocalFileName, pRemoteFileName, pUserName, pPassword, permissions));
}

/* function: mdnpbrm_copyRemoteFileToLocal  */
TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_copyRemoteFileToLocal(
  MDNPBRM_REQ_DESC *pReqDesc,
  const TMWTYPES_CHAR *pLocalFileName,
  const TMWTYPES_CHAR *pRemoteFileName,
  const TMWTYPES_CHAR *pUserName,
  const TMWTYPES_CHAR *pPassword)
{
  return (mdnpfile_copyRemoteFileToLocal(pReqDesc, pLocalFileName, pRemoteFileName, pUserName, pPassword));
}

/* function: mdnpbrm_readRemoteDirectory  */
TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_readRemoteDirectory(
  MDNPBRM_REQ_DESC *pReqDesc,
  const TMWTYPES_CHAR *pRemoteDirName,
  const TMWTYPES_CHAR *pUserName,
  const TMWTYPES_CHAR *pPassword)
{
  return (mdnpfile_readRemoteDirectory(pReqDesc, pRemoteDirName, pUserName, pPassword));
}
#endif

TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_writeDeviceAttribute(
  MDNPBRM_REQ_DESC *pReqDesc,
  DNPCHNL_TX_DATA  *pUserTxData,
  TMWTYPES_USHORT   point,
  TMWTYPES_UCHAR    variation,
  DNPDATA_ATTRIBUTE_VALUE *pData)
{
  DNPCHNL_TX_DATA *pTxData;
  TMWTYPES_UCHAR size;

  /* calculate size required*/
  size = 12 + pData->length;
  if(pUserTxData == TMWDEFS_NULL)
  {
#if MDNPDATA_SUPPORT_OBJ120
    if(pReqDesc->authAggressiveMode)
    {
      size += DNPAUTH_AGGRESSIVE_SIZE;;
    }
#endif

    pTxData = (DNPCHNL_TX_DATA *)dnpchnl_newTxData(
      pReqDesc->pChannel, pReqDesc->pSession, size, 0);

    if(pTxData == TMWDEFS_NULL)
      return(TMWDEFS_NULL);

    /* Initialize Transmit Data Structure from Request Descriptor */
    if(!_initTxData(pTxData, pReqDesc, "Write device attribute"))
    {
      dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
      return(TMWDEFS_NULL);
    }

    /* Create request header */
    mdnpbrm_buildRequestHeader(pTxData, DNPDEFS_FC_WRITE);

#if MDNPDATA_SUPPORT_OBJ120
    if(pReqDesc->authAggressiveMode)
    {
      if(mdnpauth_addAggrRequestStart((TMWSESN_TX_DATA *)pTxData, pReqDesc->authUserNumber) == TMWDEFS_NULL)
      {
        dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
        return(TMWDEFS_NULL);
      }
    }
#endif 
  }
  else
  { 
    /* Is there enough room left in request? */
    if(pUserTxData->tmw.msgLength + size >= pUserTxData->tmw.maxLength)
    {
      MDNPDIAG_ERROR(pReqDesc->pChannel, pReqDesc->pSession, MDNPDIAG_OBJ_LENGTH);
      return(TMWDEFS_NULL);
    }

    if(pUserTxData->tmw.pMsgBuf[1] != DNPDEFS_FC_WRITE)
    {
      MDNPDIAG_ERROR(pReqDesc->pChannel, pReqDesc->pSession, MDNPDIAG_FC);
      return(TMWDEFS_NULL);
    }

    pTxData = pUserTxData;
  }

  /* Add object header */
  if(!mdnpbrm_addObjectHeader(pTxData, DNPDEFS_OBJ_0_DEVICE_ATTRIBUTES, variation, DNPDEFS_QUAL_8BIT_START_STOP, point, point))
  {
    dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
    return(TMWDEFS_NULL);
  }

  /* Store data in message */
  dnputil_putAttrValueInMessage((TMWSESN_TX_DATA *)pTxData, pData); 
 
  /* Send request */
  if(pUserTxData == TMWDEFS_NULL)
  {
    /* Send request */ 
    if(!_sendFragment(pTxData))
    {
      pTxData = TMWDEFS_NULL;
    }
  }
  return((TMWSESN_TX_DATA *)pTxData);
}

#if MDNPDATA_SUPPORT_OBJ85
/* function: mdnpbrm_writeDatasetProto */
TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_writeDatasetProto(
  MDNPBRM_REQ_DESC *pReqDesc,
  DNPCHNL_TX_DATA  *pUserTxData,
  TMWTYPES_UCHAR    numObjects,
  TMWTYPES_USHORT  *pPointNumbers)
{
  TMWSESN_TX_DATA *pTxData;

  if(pUserTxData == TMWDEFS_NULL)
  {
    DNPCHNL *pDNPChannel = (DNPCHNL*)pReqDesc->pSession->pChannel; 
    pTxData = dnpchnl_newTxData(
      pReqDesc->pChannel, pReqDesc->pSession, pDNPChannel->txFragmentSize, 0);

    if(pTxData == TMWDEFS_NULL)
      return(TMWDEFS_NULL);

    /* Initialize Transmit Data Structure from Request Descriptor */
    if(!_initTxData((DNPCHNL_TX_DATA *)pTxData, pReqDesc, "Write Data Set Prototype"))
    {
      dnpchnl_freeTxData(pTxData);
      return(TMWDEFS_NULL);
    }

    mdnpbrm_buildRequestHeader((DNPCHNL_TX_DATA *)pTxData, DNPDEFS_FC_WRITE);
  }
  else
  {
    if(pUserTxData->tmw.pMsgBuf[1] != DNPDEFS_FC_WRITE)
    {
      dnpchnl_freeTxData((TMWSESN_TX_DATA *)pUserTxData);
      MDNPDIAG_ERROR(pReqDesc->pChannel, pReqDesc->pSession, MDNPDIAG_FC);
      return(TMWDEFS_NULL);
    }

    pTxData = (TMWSESN_TX_DATA *)pUserTxData;
  }

  if(!mdnpo085_writeV1(pTxData, pReqDesc, (TMWSESN_TX_DATA *)pUserTxData, numObjects, pPointNumbers))
  {
    dnpchnl_freeTxData(pTxData);
    return(TMWDEFS_NULL);
  }
  else
  {
    return(pTxData);
  }
}
#endif

#if MDNPDATA_SUPPORT_OBJ86
/* function: mdnpbrm_writeDatasetDescr */
TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_writeDatasetDescr(
  MDNPBRM_REQ_DESC *pReqDesc,
  DNPCHNL_TX_DATA  *pUserTxData,
  TMWTYPES_UCHAR    variation,
  TMWTYPES_UCHAR    numObjects,
  TMWTYPES_USHORT  *pPointNumbers)
{
  TMWSESN_TX_DATA *pTxData;
  TMWTYPES_BOOL status;

  /* Validate input arguments */
  if((variation != 1) && (variation != 3))
  {
    MDNPDIAG_ERROR(pReqDesc->pChannel, pReqDesc->pSession, MDNPDIAG_INV_VARIATION);
    return(TMWDEFS_NULL);
  } 

  if(pUserTxData == TMWDEFS_NULL)
  {
    DNPCHNL *pDNPChannel = (DNPCHNL*)pReqDesc->pSession->pChannel;  
    pTxData = dnpchnl_newTxData(
      pReqDesc->pChannel, pReqDesc->pSession, pDNPChannel->txFragmentSize, 0);

    if(pTxData == TMWDEFS_NULL)
      return(TMWDEFS_NULL);

    /* Initialize Transmit Data Structure from Request Descriptor */
    if(!_initTxData((DNPCHNL_TX_DATA *)pTxData, pReqDesc, "Write Data Set Descriptor"))
    {
      dnpchnl_freeTxData(pTxData);
      return(TMWDEFS_NULL);
    }

    mdnpbrm_buildRequestHeader((DNPCHNL_TX_DATA *)pTxData, DNPDEFS_FC_WRITE);
  }
  else
  {

    if(pUserTxData->tmw.pMsgBuf[1] != DNPDEFS_FC_WRITE)
    {
      dnpchnl_freeTxData((TMWSESN_TX_DATA *)pUserTxData);
      MDNPDIAG_ERROR(pReqDesc->pChannel, pReqDesc->pSession, MDNPDIAG_FC);
      return(TMWDEFS_NULL);
    }

    pTxData = (TMWSESN_TX_DATA *)pUserTxData;
  }

  if(variation == 1)
    status = mdnpo086_writeV1(pTxData, pReqDesc, (TMWSESN_TX_DATA *)pUserTxData, numObjects, pPointNumbers);
  else
    status = mdnpo086_writeV3(pTxData, pReqDesc, (TMWSESN_TX_DATA *)pUserTxData, numObjects, pPointNumbers);

  if(status)
    return(pTxData);

  dnpchnl_freeTxData(pTxData);
  return(TMWDEFS_NULL);
 }
#endif

#if MDNPDATA_SUPPORT_OBJ87
/* function: mdnpbrm_writeDataset */
TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_writeDataset(
  MDNPBRM_REQ_DESC *pReqDesc,
  DNPCHNL_TX_DATA  *pUserTxData,
  TMWTYPES_UCHAR    numObjects,
  TMWTYPES_USHORT  *pPointNumbers)
{
  TMWSESN_TX_DATA *pTxData;
  if(pUserTxData == TMWDEFS_NULL)
  {
    DNPCHNL *pDNPChannel = (DNPCHNL*)pReqDesc->pSession->pChannel;  
    pTxData = dnpchnl_newTxData(
      pReqDesc->pChannel, pReqDesc->pSession, pDNPChannel->txFragmentSize, 0);

    if(pTxData == TMWDEFS_NULL)
      return(TMWDEFS_NULL);

    /* Initialize Transmit Data Structure from Request Descriptor */
    if(!_initTxData((DNPCHNL_TX_DATA *)pTxData, pReqDesc, "Write Data Set Present Value"))
    {
      dnpchnl_freeTxData(pTxData);
      return(TMWDEFS_NULL);
    }

    mdnpbrm_buildRequestHeader((DNPCHNL_TX_DATA *)pTxData, DNPDEFS_FC_WRITE);
  }
  else
  {
    
    if(pUserTxData->tmw.pMsgBuf[1] != DNPDEFS_FC_WRITE)
    {
      dnpchnl_freeTxData((TMWSESN_TX_DATA *)pUserTxData);
      MDNPDIAG_ERROR(pReqDesc->pChannel, pReqDesc->pSession, MDNPDIAG_FC);
      return(TMWDEFS_NULL);
    }

    pTxData = (TMWSESN_TX_DATA *)pUserTxData;
  }
 
  return(mdnpo087_writeV1(pTxData, pReqDesc, (TMWSESN_TX_DATA *)pUserTxData, numObjects, pPointNumbers));
}


/* function: mdnpbrm_controlDataset */
TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_controlDataset(
  MDNPBRM_REQ_DESC *pReqDesc,
  TMWTYPES_UCHAR    funcCode,
  MDNPBRM_AUTO_MODE autoMode,
  TMWTYPES_UCHAR    numObjects,
  TMWTYPES_USHORT  *pPointNumbers)
{
  DNPCHNL_TX_DATA *pTxData;
  DNPCHNL *pDNPChannel = (DNPCHNL*)pReqDesc->pSession->pChannel;

  pTxData = (DNPCHNL_TX_DATA *)dnpchnl_newTxData(
    pReqDesc->pChannel, pReqDesc->pSession, pDNPChannel->txFragmentSize, 0);

  if(pTxData == TMWDEFS_NULL)
    return(TMWDEFS_NULL);

  /* Initialize Transmit Data Structure from Request Descriptor */
  if(!_initTxData(pTxData, pReqDesc, "Control Data Set"))
  {
    dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
    return(TMWDEFS_NULL);
  }

  mdnpbrm_buildRequestHeader(pTxData, funcCode);

  /* Check funcCode to see if response expected */
  if(funcCode == DNPDEFS_FC_DIRECT_OP_NOACK)
  {
    pTxData->tmw.txFlags |= TMWSESN_TXFLAGS_NO_RESPONSE;

#if MDNPDATA_SUPPORT_OBJ120
    if(!pReqDesc->authAggressiveMode)
    {
      MDNPSESN *pMDNPSession = (MDNPSESN *)pTxData->tmw.pSession;
      if(pMDNPSession->authenticationEnabled)
      {
        /* Need to delay in case this is challenged. */
        pTxData->dnpTxFlags = DNPCHNL_DNPTXFLAGS_AUTH_NOACKDELAY;
      }
    }
#endif
  }
  else if((funcCode == DNPDEFS_FC_SELECT)
    && ((autoMode & MDNPBRM_AUTO_MODE_OPERATE) != 0))
  {
    MDNPBRM_DATSET_XFER_CONTEXT *pCallbackData; 
    pCallbackData = (MDNPBRM_DATSET_XFER_CONTEXT *)mdnpmem_alloc(MDNPMEM_DATASET_CONTEXT_TYPE);
    if(pCallbackData == TMWDEFS_NULL)
    {
      dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
      return(TMWDEFS_NULL);
    }

    tmwtimer_init(&pCallbackData->delayTimer);
    pCallbackData->pTxData = pTxData;
    pTxData->pInternalCallback = _datasetCallbackFunc;
    pTxData->pInternalCallbackParam = pCallbackData;
    pCallbackData->nextOperation = MDNPBRM_DATASET_OPERATE;
  }
  
  return(mdnpo087_writeV1((TMWSESN_TX_DATA *)pTxData, pReqDesc, TMWDEFS_NULL, numObjects, pPointNumbers));
}

/* function: mdnpbrm_addCntrlDataset */
TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_addControlDataset(
  MDNPBRM_REQ_DESC *pReqDesc,
  DNPCHNL_TX_DATA  *pUserTxData,
  TMWTYPES_UCHAR    funcCode,
  MDNPBRM_AUTO_MODE autoMode,
  TMWTYPES_UCHAR    numObjects,
  TMWTYPES_USHORT  *pPointNumbers)
{
  if(pUserTxData == TMWDEFS_NULL)
  {
    return mdnpbrm_controlDataset(pReqDesc, funcCode, autoMode, numObjects, pPointNumbers);
  }
  else
  {
    /* make sure the function code matches the one in the partially built request */
    if(pUserTxData->tmw.pMsgBuf[1] != funcCode)
    {
      dnpchnl_freeTxData((TMWSESN_TX_DATA *)pUserTxData);
      MDNPDIAG_ERROR(pReqDesc->pChannel, pReqDesc->pSession, MDNPDIAG_FC);
      return(TMWDEFS_NULL);
    }
  }

  return(mdnpo087_writeV1((TMWSESN_TX_DATA *)pUserTxData, pReqDesc, (TMWSESN_TX_DATA *)pUserTxData, numObjects, pPointNumbers));
}
#endif

#if MDNPDATA_SUPPORT_DATASETS
/* function: mdnpbrm_datasetExchange */
TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_datasetExchange(
  MDNPBRM_REQ_DESC *pReqDesc)
{
  DNPCHNL_TX_DATA *pTxData;
  DNPCHNL *pDNPChannel = (DNPCHNL*)pReqDesc->pSession->pChannel;  
  MDNPBRM_DATSET_XFER_CONTEXT *pCallbackData = TMWDEFS_NULL; 

  pCallbackData = (MDNPBRM_DATSET_XFER_CONTEXT *)mdnpmem_alloc(MDNPMEM_DATASET_CONTEXT_TYPE);
  if(pCallbackData == TMWDEFS_NULL)
  {
    return(TMWDEFS_NULL);
  }

  tmwtimer_init(&pCallbackData->delayTimer);

  pTxData = (DNPCHNL_TX_DATA *)dnpchnl_newTxData(
    pReqDesc->pChannel, pReqDesc->pSession, pDNPChannel->txFragmentSize, 0);

  if(pTxData == TMWDEFS_NULL)
  {
    return(TMWDEFS_NULL);
  }

  if(!_initTxData((DNPCHNL_TX_DATA *)pTxData, pReqDesc, TMWDEFS_NULL))
  {
    dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
    return(TMWDEFS_NULL);
  }

  mdnpbrm_buildRequestHeader(pTxData, 1);

  /* Read number of outstation defined prototypes */
  pTxData = (DNPCHNL_TX_DATA*)mdnpbrm_readGroup(pReqDesc, pTxData, 0, DNPDEFS_OBJ0_NUM_OUTSTA_PROTOS, DNPDEFS_QUAL_8BIT_START_STOP, 0, 0); 
  pTxData = (DNPCHNL_TX_DATA*)mdnpbrm_readGroup(pReqDesc, pTxData, 0, DNPDEFS_OBJ0_NUM_OUTSTA_DATASET, DNPDEFS_QUAL_8BIT_START_STOP, 0, 0); 
  if(pTxData != TMWDEFS_NULL)
  {
    pCallbackData->pTxData = pTxData;
    pCallbackData->nextOperation = MDNPBRM_DATASET_READ_PROTO;

    pTxData->pInternalCallback = _datasetCallbackFunc;
    pTxData->pInternalCallbackParam = pCallbackData;

    if(!dnpchnl_sendFragment((TMWSESN_TX_DATA *)pTxData))
    { 
      dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
      return(TMWDEFS_NULL);
    }

  } 
  return((TMWSESN_TX_DATA*)pTxData);
}
#endif

#if MDNPDATA_SUPPORT_OBJ120
/* function: mdnpbrm_authManualSendSessionKey */
TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_authManualSendSessionKey(
  MDNPBRM_REQ_DESC *pReqDesc,
  TMWTYPES_USHORT userNumber)
{
#if MDNPCNFG_SUPPORT_SA_VERSION5
  DNPCHNL_TX_DATA *pTxData;
  DNPCHNL *pDNPChannel = (DNPCHNL*)pReqDesc->pSession->pChannel;
  DNPSESN *pDNPSession = (DNPSESN*)pReqDesc->pSession;

  if (pDNPSession->operateInV2Mode)
    return(TMWDEFS_NULL);

  pTxData = (DNPCHNL_TX_DATA *)dnpchnl_newTxData(
    pReqDesc->pChannel, pReqDesc->pSession, pDNPChannel->txFragmentSize, 0);

  if (pTxData == TMWDEFS_NULL)
    return(TMWDEFS_NULL);

  /* Initialize Transmit Data Structure from Request Descriptor */
  if (!_initTxData(pTxData, pReqDesc, "Secure Authentication Manual Session Key Sequence"))
  {
    dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
    return(TMWDEFS_NULL);
  }
  
  return mdnpsa_manualSendSessionKey((TMWSESN_TX_DATA*)pTxData, userNumber);
#else
  TMWTARG_UNUSED_PARAM(pReqDesc);
  TMWTARG_UNUSED_PARAM(userNumber);
  return TMWDEFS_NULL;
#endif
}

/* function: mdnpbrm_authUserCertificate */
TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_authUserCertificate(
  MDNPBRM_REQ_DESC *pReqDesc,
  void*             userNameDbHandle)
{
#if DNPCNFG_SUPPORT_AUTHKEYUPDATE && MDNPCNFG_SUPPORT_SA_VERSION5
  DNPCHNL_TX_DATA *pTxData;
  DNPCHNL *pDNPChannel = (DNPCHNL*)pReqDesc->pSession->pChannel;  
  DNPSESN *pDNPSession = (DNPSESN*)pReqDesc->pSession;
  
  if(pDNPSession->operateInV2Mode)
    return(TMWDEFS_NULL);
  
  pTxData = (DNPCHNL_TX_DATA *)dnpchnl_newTxData(
    pReqDesc->pChannel, pReqDesc->pSession, pDNPChannel->txFragmentSize, 0);

  if(pTxData == TMWDEFS_NULL)
    return(TMWDEFS_NULL);

  /* Initialize Transmit Data Structure from Request Descriptor */
  if(!_initTxData(pTxData, pReqDesc, "Secure Authentication Certificate"))
  {
    dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
    return(TMWDEFS_NULL);
  }

  pTxData->priority = MDNPBRM_AUTHENTICATE_PRIORITY;
  return mdnpsa_sendUserCertificateV8((TMWSESN_TX_DATA*)pTxData, userNameDbHandle); 
#else
  TMWTARG_UNUSED_PARAM(pReqDesc);
  TMWTARG_UNUSED_PARAM(userNameDbHandle);
  return TMWDEFS_NULL;
#endif
}

/* function: mdnpbrm_authUserStatusChange */
TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_authUserStatusChange(
  MDNPBRM_REQ_DESC *pReqDesc,
  void*             userNameDbHandle,
  TMWTYPES_UCHAR    operation,
  TMWTYPES_BOOL     sendUpdateKey)
{
#if DNPCNFG_SUPPORT_AUTHKEYUPDATE && MDNPCNFG_SUPPORT_SA_VERSION5
  DNPCHNL_TX_DATA *pTxData;
  DNPCHNL *pDNPChannel = (DNPCHNL*)pReqDesc->pSession->pChannel;  
  DNPSESN *pDNPSession = (DNPSESN*)pReqDesc->pSession;
  
  if(pDNPSession->operateInV2Mode)
    return(TMWDEFS_NULL);
  
  pTxData = (DNPCHNL_TX_DATA *)dnpchnl_newTxData(
    pReqDesc->pChannel, pReqDesc->pSession, pDNPChannel->txFragmentSize, 0);

  if(pTxData == TMWDEFS_NULL)
    return(TMWDEFS_NULL);

  /* Initialize Transmit Data Structure from Request Descriptor */
  if(!_initTxData(pTxData, pReqDesc, "Secure Authentication User Status Change"))
  {
    dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
    return(TMWDEFS_NULL);
  }

  /* Delete only sends a g120v10, not the rest of the update sequence */
  if(operation == DNPAUTH_USER_STATUS_DELETE)
    sendUpdateKey = TMWDEFS_FALSE;

  pTxData->priority = MDNPBRM_AUTHENTICATE_PRIORITY;
  return mdnpsa_sendUserStatusChangeMsgV10((TMWSESN_TX_DATA*)pTxData, userNameDbHandle, operation, sendUpdateKey); 
#else
  TMWTARG_UNUSED_PARAM(pReqDesc);
  TMWTARG_UNUSED_PARAM(userNameDbHandle);
  TMWTARG_UNUSED_PARAM(operation);
  TMWTARG_UNUSED_PARAM(sendUpdateKey);
  return TMWDEFS_NULL;
#endif
}
               
/* function: mdnpbrm_authUserUpdateKeyChange */
TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_authUserUpdateKeyChange(
  MDNPBRM_REQ_DESC *pReqDesc,
  void *            userNameDbHandle,
  TMWTYPES_BOOL     sendUpdateKey)
{
#if DNPCNFG_SUPPORT_AUTHKEYUPDATE && MDNPCNFG_SUPPORT_SA_VERSION5
  DNPCHNL_TX_DATA *pTxData;
  DNPCHNL *pDNPChannel = (DNPCHNL*)pReqDesc->pSession->pChannel;  
  DNPSESN *pDNPSession = (DNPSESN*)pReqDesc->pSession;
  
  if(pDNPSession->operateInV2Mode)
    return(TMWDEFS_NULL);

  pTxData = (DNPCHNL_TX_DATA *)dnpchnl_newTxData(
    pReqDesc->pChannel, pReqDesc->pSession, pDNPChannel->txFragmentSize, 0);

  if(pTxData == TMWDEFS_NULL)
    return(TMWDEFS_NULL);
  
  /* Initialize Transmit Data Structure from Request Descriptor */
  if(!_initTxData(pTxData, pReqDesc, "Secure Authentication Update Key Change Request"))
  {
    dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
    return(TMWDEFS_NULL);
  }

  pTxData->priority = MDNPBRM_AUTHENTICATE_PRIORITY;
  return mdnpsa_sendUserKeyChangeMsgV11((TMWSESN_TX_DATA*)pTxData, userNameDbHandle, sendUpdateKey); 
#else
  TMWTARG_UNUSED_PARAM(pReqDesc);
  TMWTARG_UNUSED_PARAM(userNameDbHandle);
  TMWTARG_UNUSED_PARAM(sendUpdateKey);
  return TMWDEFS_NULL;
#endif
}

/* function: mdnpbrm_authUserSymUpdateKeyChange */
/* This is sent after the authority has provided the new update key encrypted along with the random data from the outstation */
TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_authUserSendSymUpdateKey(
  MDNPBRM_REQ_DESC *pReqDesc,
  void *            userNameDbHandle) 
{
#if DNPCNFG_SUPPORT_AUTHKEYUPDATE && MDNPCNFG_SUPPORT_SA_VERSION5
  DNPCHNL_TX_DATA *pTxData;
  DNPCHNL *pDNPChannel = (DNPCHNL*)pReqDesc->pSession->pChannel;  
  DNPSESN *pDNPSession = (DNPSESN*)pReqDesc->pSession;
  
  if(pDNPSession->operateInV2Mode)
    return(TMWDEFS_NULL);

  pTxData = (DNPCHNL_TX_DATA *)dnpchnl_newTxData(
    pReqDesc->pChannel, pReqDesc->pSession, pDNPChannel->txFragmentSize, 0);

  if(pTxData == TMWDEFS_NULL)
    return(TMWDEFS_NULL);
  
  /* Initialize Transmit Data Structure from Request Descriptor */
  if(!_initTxData(pTxData, pReqDesc, "Secure Authentication Send Symmetric Update Key"))
  {
    dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
    return(TMWDEFS_NULL);
  }

  pTxData->priority = MDNPBRM_AUTHENTICATE_PRIORITY;
  return mdnpsa_sendSymUpdKeyV13((TMWSESN_TX_DATA*)pTxData, userNameDbHandle); 
#else
  TMWTARG_UNUSED_PARAM(pReqDesc);
  TMWTARG_UNUSED_PARAM(userNameDbHandle);
  return TMWDEFS_NULL;
#endif
}
 
/* function: mdnpbrm_authUserSendg120v6Test */
/*  For Test Purposes ONLY  Send g120v6 without first sending g120v4 */
TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_authUserSendg120v6Test(
  MDNPBRM_REQ_DESC *pReqDesc,
  TMWTYPES_USHORT userNumber)
{
#if MDNPCNFG_SUPPORT_SA_VERSION5 && TMW_PRIVATE_AUTHTEST 
  DNPCHNL_TX_DATA *pTxData;
  DNPCHNL *pDNPChannel = (DNPCHNL*)pReqDesc->pSession->pChannel;
  DNPSESN *pDNPSession = (DNPSESN*)pReqDesc->pSession;

  if (pDNPSession->operateInV2Mode)
    return(TMWDEFS_NULL);
 
  pTxData = (DNPCHNL_TX_DATA *)dnpchnl_newTxData(
    pReqDesc->pChannel, pReqDesc->pSession, pDNPChannel->txFragmentSize, 0);

  if (pTxData == TMWDEFS_NULL)
    return(TMWDEFS_NULL);

  /* Initialize Transmit Data Structure from Request Descriptor */
  if (!_initTxData(pTxData, pReqDesc, "Secure Authentication Key Change Request"))
  {
    dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
    return(TMWDEFS_NULL);
  } 

  pTxData->priority = MDNPBRM_AUTHENTICATE_PRIORITY;
  return mdnpsa_sendKeyChangeRequestV6((TMWSESN_TX_DATA*)pTxData, userNumber);
#else
  TMWTARG_UNUSED_PARAM(pReqDesc);
  TMWTARG_UNUSED_PARAM(userNumber);
  return TMWDEFS_NULL;
#endif
}

/* function: mdnpbrm_authUserSendg120v15Test For Test Purposes ONLY */
TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpbrm_authUserSendg120v15Test(
  MDNPBRM_REQ_DESC *pReqDesc,
  void *            userNameDbHandle)
{
#if DNPCNFG_SUPPORT_AUTHKEYUPDATE && MDNPCNFG_SUPPORT_SA_VERSION5
  DNPCHNL_TX_DATA *pTxData;
  DNPCHNL *pDNPChannel = (DNPCHNL*)pReqDesc->pSession->pChannel;
  DNPSESN *pDNPSession = (DNPSESN*)pReqDesc->pSession;

  if (pDNPSession->operateInV2Mode)
    return(TMWDEFS_NULL);

  pTxData = (DNPCHNL_TX_DATA *)dnpchnl_newTxData(
    pReqDesc->pChannel, pReqDesc->pSession, pDNPChannel->txFragmentSize, 0);

  if (pTxData == TMWDEFS_NULL)
    return(TMWDEFS_NULL);

  /* Initialize Transmit Data Structure from Request Descriptor */
  if (!_initTxData(pTxData, pReqDesc, "Secure Authentication Send Update Key Change Confirmation"))
  {
    dnpchnl_freeTxData((TMWSESN_TX_DATA *)pTxData);
    return(TMWDEFS_NULL);
  }

  pTxData->priority = MDNPBRM_AUTHENTICATE_PRIORITY;
  return mdnpsa_sendUpdKeyCnfmV15((TMWSESN_TX_DATA*)pTxData, userNameDbHandle);
#else
  TMWTARG_UNUSED_PARAM(pReqDesc);
  TMWTARG_UNUSED_PARAM(userNameDbHandle);
  return TMWDEFS_NULL;
#endif
}

#endif
