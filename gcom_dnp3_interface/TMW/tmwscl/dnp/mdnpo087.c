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

/* file: mdnpo087.c
 * description: DNP Master functionality for Object 87 Data Set Present Value Objects
 */
#include "tmwscl/dnp/dnpdiag.h"
#include "tmwscl/dnp/mdnpo087.h"
#include "tmwscl/dnp/mdnpbrm.h"
#include "tmwscl/dnp/dnpdtime.h"
#include "tmwscl/dnp/mdnpdiag.h"
#include "tmwscl/dnp/mdnpcnfg.h"
#include "tmwscl/dnp/mdnpmem.h"

#include "tmwscl/utils/tmwdb.h"
#include "tmwscl/utils/tmwtarg.h"

#if (MDNPDATA_SUPPORT_OBJ87 || MDNPDATA_SUPPORT_OBJ88)

#if TMWCNFG_SUPPORT_ASYNCH_DB
typedef struct MDNPO087DataStruct {
  TMWDB_DATA tmw;
  TMWDTIME              timeStamp;
  TMWTYPES_USHORT       pointNumber;
  TMWTYPES_BOOL         isEvent;
} MDNPO087_DATA;

typedef struct MDNPO087ElemDataStruct {
  TMWDB_DATA tmw; 
  DNPDATA_DATASET_VALUE elem;
  TMWTYPES_UCHAR        elemIndex;
  TMWTYPES_BOOL         isEvent;
  TMWTYPES_USHORT       pointNumber;

  /* Memory to copy string data to, length of data array will be determined 
   * when allocated
   */
  TMWTYPES_UCHAR              data[1];
} MDNPO087_ELEM_DATA;

/* function: _dbStoreFunc */
static TMWTYPES_BOOL TMWDEFS_LOCAL _dbStoreFunc(
  TMWDB_DATA *pData)
{
  MDNPO087_DATA *pDNPData = (MDNPO087_DATA *)pData;
  mdnpdata_storeDatasetTime(pData->pDbHandle, pDNPData->pointNumber, &pDNPData->timeStamp, pDNPData->isEvent);
  return(TMWDEFS_TRUE);
}

/* function: _dbStoreElemFunc */
static TMWTYPES_BOOL TMWDEFS_LOCAL _dbStoreElemFunc(
  TMWDB_DATA *pData)
{
  MDNPO087_ELEM_DATA *pDNPData = (MDNPO087_ELEM_DATA *)pData;
  mdnpdata_storeDataset(pData->pDbHandle, pDNPData->pointNumber, pDNPData->elemIndex, &pDNPData->elem, pDNPData->isEvent); 
  return(TMWDEFS_TRUE);
}

/* function: _storeDatasetTimestamp */
static void TMWDEFS_LOCAL _storeDatasetTimestamp(
  void *pDbHandle, 
  TMWTYPES_USHORT pointNumber,
  TMWDTIME *pTimeStamp,
  TMWTYPES_BOOL isEvent)
{
  /* Allocate new binary data structure */
  MDNPO087_DATA *pDNPData = (MDNPO087_DATA *)tmwtarg_alloc(sizeof(MDNPO087_DATA));
  if (pDNPData == TMWDEFS_NULL)
  {
    return;
  }

  /* Initialize data independent parts */
  tmwdb_initData((TMWDB_DATA *)pDNPData, pDbHandle, _dbStoreFunc);

  /* Copy data dependent parts */ 
  pDNPData->isEvent = isEvent;
  pDNPData->timeStamp = *pTimeStamp;
  pDNPData->pointNumber = pointNumber;

  /* Store in database queue */
  if(!tmwdb_addEntry((TMWDB_DATA *)pDNPData))
    tmwtarg_free(pDNPData);
}

/* function: _storeDatasetElem */
static void TMWDEFS_LOCAL _storeDatasetElem(
  void *pDbHandle, 
  TMWTYPES_USHORT pointNumber,
  TMWTYPES_UCHAR  elemIndex,
  DNPDATA_DATASET_VALUE *pElem,
  TMWTYPES_BOOL isEvent)
{ 
  MDNPO087_ELEM_DATA *pDNPData;
  TMWTYPES_USHORT bufSize = sizeof(MDNPO087_ELEM_DATA) + pElem->length;

  /* Allocate data structure */
  pDNPData = (MDNPO087_ELEM_DATA *)tmwtarg_alloc(bufSize);
  if (pDNPData == TMWDEFS_NULL)
  {
    return;
  }
 
  /* Initialize data independent parts */
  tmwdb_initData((TMWDB_DATA *)pDNPData, pDbHandle, _dbStoreElemFunc);

  /* Copy data dependent parts */
  pDNPData->elem = *pElem;

  /* If pointer to string, copy string itself and point to that string */
  if(pElem->type == DNPDATA_VALUE_STRPTR)
  { 
    memcpy(pDNPData->data, pElem->value.pStrValue, pElem->length);
    pDNPData->elem.value.pStrValue = pDNPData->data;
  }

  pDNPData->elemIndex = elemIndex;
  pDNPData->isEvent = isEvent;
  pDNPData->pointNumber = pointNumber;

  /* Store in database queue */
  if(!tmwdb_addEntry((TMWDB_DATA *)pDNPData))
    tmwtarg_free(pDNPData);
}
#else

/* function: _storeDatasetTimestamp */
static void TMWDEFS_LOCAL _storeDatasetTimestamp(
  void *pDbHandle, 
  TMWTYPES_USHORT pointNumber,
  TMWDTIME *pTimeStamp,
  TMWTYPES_BOOL isEvent)
{ 
  mdnpdata_storeDatasetTime(pDbHandle, pointNumber, pTimeStamp, isEvent);  
}

/* function: _storeDatasetElem */
static void TMWDEFS_LOCAL _storeDatasetElem(
  void *pDbHandle, 
  TMWTYPES_USHORT pointNumber,
  TMWTYPES_UCHAR  elemIndex,
  DNPDATA_DATASET_VALUE *pElem,
  TMWTYPES_BOOL isEvent)
{ 
  mdnpdata_storeDataset(pDbHandle, pointNumber, elemIndex, pElem, isEvent);
}
#endif /* TMWCNFG_SUPPORT_ASYNCH_DB */

static TMWTYPES_UCHAR _datasetDescrRead(
  TMWSESN *pSession, 
  TMWTYPES_USHORT datasetId,
  TMWTYPES_UCHAR  maxNumber,
  DNPDEFS_DATASET_TYPE_CODE *pDataTypes,
  DNPDEFS_DATASET_DESCR_CODE *pElemTypes)
{
  TMWTYPES_UCHAR elemIndex;
  TMWTYPES_UCHAR numberDescrElems;
  TMWTYPES_UCHAR descrIndex;
  DNPDATA_DATASET_DESCR_ELEM *pDescrElem;
  MDNPSESN *pMDNPSession = (MDNPSESN *)pSession; 

  elemIndex = 0;
  pDescrElem = mdnpdata_datasetDescrGetCont(pMDNPSession->pDbHandle, (TMWTYPES_USHORT)datasetId, &numberDescrElems);
  if(pDescrElem == TMWDEFS_NULL)
    return(0);

  for(descrIndex=0; descrIndex < numberDescrElems; descrIndex++)
  { 
    if(elemIndex >= maxNumber)
      break;
     
    pElemTypes[elemIndex] = pDescrElem->descrElemType;
    if((pDescrElem->descrElemType == DNPDEFS_DATASET_DESCR_DAEL)
      ||(pDescrElem->descrElemType == DNPDEFS_DATASET_DESCR_CTLS)
      ||(pDescrElem->descrElemType == DNPDEFS_DATASET_DESCR_CTLV))
    {
      pDataTypes[elemIndex++] = pDescrElem->dataTypeCode;
    }
    else if(pDescrElem->descrElemType == DNPDEFS_DATASET_DESCR_PTYP)
    {
      /* get prototype information */
      TMWTYPES_USHORT pointNum;
      TMWTYPES_UCHAR *pStrValue;

      if(pDescrElem->ancillaryValue.type == DNPDATA_VALUE_STRPTR)
        pStrValue = pDescrElem->ancillaryValue.value.pStrValue;
      else if(pDescrElem->ancillaryValue.type == DNPDATA_VALUE_STRARRAY)
        pStrValue = pDescrElem->ancillaryValue.value.strValue;
      else
        break;

      if(mdnpdata_datasetProtoGetID(pMDNPSession->pDbHandle, pStrValue, &pointNum)) 
      {
        DNPDATA_DATASET_DESCR_ELEM *pProtoElem;
        TMWTYPES_UCHAR temp[16];
        TMWTYPES_UCHAR numberElems;
        pProtoElem = mdnpdata_datasetProtoGet(pMDNPSession->pDbHandle, pointNum, &numberElems, temp);
        if(pProtoElem != TMWDEFS_NULL)
        {
          int i;
          for(i=0; i<numberElems; i++)
          {      
            pElemTypes[elemIndex] = pProtoElem->descrElemType;
            if((pProtoElem->descrElemType == DNPDEFS_DATASET_DESCR_DAEL)
              ||(pProtoElem->descrElemType == DNPDEFS_DATASET_DESCR_CTLS)
              ||(pProtoElem->descrElemType == DNPDEFS_DATASET_DESCR_CTLV))
            {
              pDataTypes[elemIndex++] = pProtoElem->dataTypeCode;
            }
            pProtoElem++;
          }
          mdnpdata_datasetProtoRelease(pMDNPSession->pDbHandle);
        }
        else
        {  
          break;
        }
      }
      else
      {
        DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_READ_PROTO_FAIL);
        break;
      }
    }
    pDescrElem++;
  }

  mdnpdata_datasetDescrRelease(pMDNPSession->pDbHandle);
  return(elemIndex);
}

/* function: mdnpo087_readObj87v1 */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpo087_readObj87v1(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{
  MDNPSESN *pMDNPSession;
  TMWTYPES_USHORT pointIndex;
  TMWTYPES_BOOL isEvent;
  
  if(pObjHeader->qualifier != DNPDEFS_QUAL_16BIT_FREE_FORMAT)
    return(TMWDEFS_FALSE);

  pMDNPSession = (MDNPSESN *)pSession;

  if(pObjHeader->group == DNPDEFS_OBJ_88_DATASET_EVENTS)
    isEvent = TMWDEFS_TRUE;
  else
    isEvent = TMWDEFS_FALSE;

  for(pointIndex = 0; pointIndex < pObjHeader->numberOfPoints; pointIndex++)
  {
    int             elemIndex;
    TMWTYPES_ULONG  datasetId;
    TMWTYPES_ULONG  stopIndex;
    TMWTYPES_USHORT sizePrefix;
    TMWTYPES_USHORT numberDescrElems;    
    DNPDATA_DATASET_VALUE elem;
    DNPDEFS_DATASET_DESCR_CODE elemTypes[DNPCNFG_MAX_DATASET_DESCR_ELEMS];
    DNPDEFS_DATASET_TYPE_CODE  dataTypes[DNPCNFG_MAX_DATASET_DESCR_ELEMS];

    /* Protect against badly formatted message */
    if((pRxFragment->offset+2) > pRxFragment->msgLength)
    {
      DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_INVALID_SIZE);
      return(TMWDEFS_FALSE);
    }

    tmwtarg_get16(&pRxFragment->pMsgBuf[pRxFragment->offset], &sizePrefix);
    pRxFragment->offset += 2;

    stopIndex = pRxFragment->offset + sizePrefix;

    /* Protect against badly formatted message */
    if(stopIndex > pRxFragment->msgLength)
    {
      DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_INVALID_SIZE);
      return(TMWDEFS_FALSE);
    } 

    /* Get Mandatory Data Set Id from message */
    if(!dnputil_getValueFromMessage(pRxFragment->pMsgBuf, &pRxFragment->offset, stopIndex, DNPDEFS_DATASET_TYPE_UINT, &elem))
    {    
      return(TMWDEFS_FALSE);
    }

    datasetId = elem.value.uint32Value;

    /* Get Time from message */ 
    if(!dnputil_getValueFromMessage(pRxFragment->pMsgBuf, &pRxFragment->offset, stopIndex, DNPDEFS_DATASET_TYPE_TIME, &elem))
    {
      return(TMWDEFS_FALSE);
    }

    /* Store data set id and time separately.
     * Tech Bulletin shows data set id and timestamp as first two elements.
     * Since data set ID is also the index and timeStamp is required, calling them out at the
     * interface makes things more clear for user.
     */
    _storeDatasetTimestamp(pMDNPSession->pDbHandle, (TMWTYPES_USHORT)datasetId, &elem.value.timeValue, isEvent);
    DNPDIAG_SHOW_DATASET_TIME(pSession, (TMWTYPES_USHORT)datasetId, &elem.value.timeValue, isEvent, TMWDIAG_ID_RX); 

    /* Read the data set descriptor information, including any prototypes required and
     * store the type information on the stack.
     */
    memset(elemTypes, 0, sizeof(elemTypes));
    numberDescrElems = _datasetDescrRead(pSession, (TMWTYPES_USHORT)datasetId, DNPCNFG_MAX_DATASET_DESCR_ELEMS, dataTypes, elemTypes);
    if(numberDescrElems == 0)
    {
      /* Descriptor is not found here on this master */
      DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_DATASET_DESCR);
    }

    elemIndex = 0;
    /* Now get the individual elements from the received data set */
    while(pRxFragment->offset < stopIndex)
    {  
      DNPDEFS_DATASET_TYPE_CODE typeCode;
      if(elemIndex < numberDescrElems)
        typeCode = dataTypes[elemIndex];
      else
        typeCode = DNPDEFS_DATASET_TYPE_NONE;
      
      if(!dnputil_getValueFromMessage(pRxFragment->pMsgBuf, &pRxFragment->offset, stopIndex, typeCode, &elem))
      {
        return(TMWDEFS_FALSE);
      }

      /* Control status and control value elements are ignored by master on read and unsolicited responses */
      if((elemTypes[elemIndex] != DNPDEFS_DATASET_DESCR_CTLS)
        &&(elemTypes[elemIndex] != DNPDEFS_DATASET_DESCR_CTLV))
      {
        _storeDatasetElem(pMDNPSession->pDbHandle, (TMWTYPES_USHORT)datasetId, (TMWTYPES_UCHAR)elemIndex, &elem, isEvent); 
        DNPDIAG_SHOW_DATASET(pSession, (TMWTYPES_UCHAR)elemIndex, &elem, DNPDEFS_FC_READ, TMWDEFS_FALSE, isEvent, TMWDIAG_ID_RX);
      }
      else
      {
        DNPDIAG_SHOW_DATASET(pSession, (TMWTYPES_UCHAR)elemIndex, &elem, DNPDEFS_FC_READ, TMWDEFS_TRUE, isEvent, TMWDIAG_ID_RX);
      }

      if(elemIndex > DNPCNFG_MAX_DATASET_DESCR_ELEMS)
      {
        DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_DATASET_ELEMS);
        return(TMWDEFS_FALSE);
      }
      elemIndex++;
    } 
  }
  return(TMWDEFS_TRUE);
}
#endif

#if MDNPDATA_SUPPORT_OBJ87
/* If multiple request objects are put in a request and this dataset write can't be done, make sure to deallocate the callback structure if there one*/
static void TMWDEFS_LOCAL _freeTxData(
  TMWSESN_TX_DATA  *pTxData)
{
  DNPCHNL_TX_DATA *pDNPTxData = (DNPCHNL_TX_DATA*)pTxData;
  if (pDNPTxData->pInternalCallbackParam != TMWDEFS_NULL)
  {
    mdnpmem_free(pDNPTxData->pInternalCallbackParam);
  }

  dnpchnl_freeTxData(pTxData);
}

/* function: mdnpo087_writeV1 */
TMWSESN_TX_DATA * TMWDEFS_GLOBAL mdnpo087_writeV1( 
  TMWSESN_TX_DATA  *pTxData,
  MDNPBRM_REQ_DESC *pReqDesc,
  TMWSESN_TX_DATA  *pUserTxData,
  TMWTYPES_UCHAR    numObjects,
  TMWTYPES_USHORT  *pPointNumbers)
{
  DNPDATA_DATASET_VALUE *pElem;
  MDNPSESN *pMDNPSession; 
  int i, j;
  TMWTYPES_USHORT sizePrefixIndex;
  TMWTYPES_UCHAR length;
  TMWTYPES_UCHAR numberElems;
  TMWTYPES_USHORT pointNumber; 
  TMWTYPES_USHORT quantityIndex;

  pMDNPSession = (MDNPSESN *)pReqDesc->pSession; 

  /* If point numbers are specified by user, use those.
   * If not specified, start with index of first descriptor
   * that was defined here on the master. 
   */
  if(pPointNumbers != TMWDEFS_NULL)
  {
    pointNumber = *pPointNumbers++;
  }
  else
  {  
    /* Master indexes start after slave descriptors and datasets */
    pointNumber = mdnpdata_datasetDescrSlaveQty(pMDNPSession->pDbHandle);
    numObjects = 0xff;
  }

  if(!mdnpbrm_addObjectHeader((DNPCHNL_TX_DATA *)pTxData, DNPDEFS_OBJ_87_DATASET_VALUE, 1, 
    DNPDEFS_QUAL_16BIT_FREE_FORMAT, 0, numObjects))
  {
    _freeTxData(pTxData);
    return(TMWDEFS_NULL);
  }

  quantityIndex = pTxData->msgLength-1;

  for(i=0; i<numObjects; i++)
  {
    TMWTYPES_USHORT indexLength;
    TMWTYPES_MS_SINCE_70 msSince70;
    TMWDTIME timeStamp;

    /* Returns pointer to Data Set in ROM or RAM and number of elements in descriptor */
    pElem = mdnpdata_datasetGet(pMDNPSession->pDbHandle, pointNumber, &numberElems, &timeStamp);
    if(pElem == TMWDEFS_NULL)
    {
      if((numObjects != 0xff) || (i == 0))
      {
        /* Can't get the specified datasets to send
         * or there are no datasets 
         */
        _freeTxData(pTxData);
        return(TMWDEFS_NULL);
      }
      else 
      {
        /* all data sets have been sent */
        break;
      }
    }

    /* Is there enough room? */
    length = dnputil_lengthRequired(pointNumber);
    if((pTxData->msgLength + length + 10) > pTxData->maxLength)
    {
      mdnpdata_datasetRelease(pMDNPSession->pDbHandle);
      _freeTxData(pTxData);
      return(TMWDEFS_NULL);
    }

    /* save pointer for size prefix */
    sizePrefixIndex = pTxData->msgLength;
    pTxData->msgLength += 2; 

    /* First put data set id in message */
    pTxData->pMsgBuf[pTxData->msgLength++] = length;
    dnputil_putIntInMessage(&pTxData->pMsgBuf[pTxData->msgLength], pointNumber, length);
    pTxData->msgLength = (TMWTYPES_USHORT)(pTxData->msgLength + length);

    /* Then put time stamp in message */
    pTxData->pMsgBuf[pTxData->msgLength++] = 6;

    dnpdtime_dateTimeToMSSince70(&msSince70, &timeStamp);
    dnpdtime_writeMsSince70(&pTxData->pMsgBuf[pTxData->msgLength], &msSince70);
    pTxData->msgLength += 6;
    DNPDIAG_SHOW_DATASET_TIME(pReqDesc->pSession, pointNumber, &timeStamp, TMWDEFS_FALSE, 0);
    
    for(j=0; j<numberElems; j++)
    {
      /* Is there enough room? */ 
      if((pTxData->msgLength + pElem->length + 1) > pTxData->maxLength)
      {
        mdnpdata_datasetRelease(pMDNPSession->pDbHandle);
        _freeTxData(pTxData);
        return(TMWDEFS_NULL);
      }

      pTxData->pMsgBuf[pTxData->msgLength++] = pElem->length;

      dnputil_putValueInMessage(pTxData->pMsgBuf, &pTxData->msgLength, pElem);

      /* Diagnostics */
      DNPDIAG_SHOW_DATASET(pReqDesc->pSession, (TMWTYPES_UCHAR)j, pElem, DNPDEFS_FC_WRITE, TMWDEFS_FALSE, TMWDEFS_FALSE, 0); 

      pElem++;
    }

    mdnpdata_datasetRelease(pMDNPSession->pDbHandle);

    indexLength = pTxData->msgLength - (sizePrefixIndex +2);
    tmwtarg_store16(&indexLength, &pTxData->pMsgBuf[sizePrefixIndex]);

    if(pPointNumbers != TMWDEFS_NULL)
      pointNumber = *pPointNumbers++;
    else
      pointNumber++;
  }  

  /* Update number of objects written */
  pTxData->pMsgBuf[quantityIndex] = (TMWTYPES_UCHAR)i;

  if(pUserTxData != TMWDEFS_NULL)
    return(pTxData);

  /* Send request */
  if(!dnpchnl_sendFragment(pTxData))
  {
    _freeTxData(pTxData);
    pTxData = TMWDEFS_NULL; 
  }

  return(pTxData);
} 

/* function: mdnpo087_selOperRespObj87v1 */
DNPCHNL_RESP_STATUS TMWDEFS_GLOBAL mdnpo087_selOperRespObj87v1(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{
  TMWTARG_UNUSED_PARAM(pSession);
  TMWTARG_UNUSED_PARAM(pObjHeader);
  /* Parse entire message checking to see of status code is non zero 
   *  return(DNPCHNL_RESP_STATUS_MISMATCH);
   */

  /* For now just say control failed, since this will only be called 
   * if response did not match request byte for byte 
   */
  pRxFragment->offset = pRxFragment->msgLength;
  return(DNPCHNL_RESP_STATUS_FAILURE);
} 

#endif
