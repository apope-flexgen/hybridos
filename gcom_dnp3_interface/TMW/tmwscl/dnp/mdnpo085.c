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

/* file: mdnpo085.c
 * description: DNP Master functionality for Object 85 Data Set Prototype
 */
#include "tmwscl/dnp/dnpdiag.h"
#include "tmwscl/dnp/mdnpo085.h"
#include "tmwscl/dnp/mdnpbrm.h"
#include "tmwscl/dnp/mdnpdiag.h"
#include "tmwscl/dnp/mdnpcnfg.h"
#include "tmwscl/dnp/dnputil.h"

#include "tmwscl/utils/tmwdb.h"
#include "tmwscl/utils/tmwtarg.h"

#if MDNPDATA_SUPPORT_OBJ85

#if TMWCNFG_SUPPORT_ASYNCH_DB
typedef struct MDNPO085DataStruct {
  TMWDB_DATA tmw;
  TMWTYPES_UCHAR uuid[16];
  TMWTYPES_USHORT pointNumber;
} MDNPO085_DATA;

typedef struct MDNPO085ElemDataStruct {
  TMWDB_DATA tmw; 
  DNPDATA_DATASET_DESCR_ELEM descr;
  TMWTYPES_UCHAR  elemIndex;
  TMWTYPES_USHORT pointNumber;

  /* Memory to copy string data to, length of data array will be determined 
   * when allocated
   */
  TMWTYPES_UCHAR              data[1];
} MDNPO085_ELEM_DATA;

/* function: _dbStoreFunc */
static TMWTYPES_BOOL TMWDEFS_LOCAL _dbStoreFunc(
  TMWDB_DATA *pData)
{
  MDNPO085_DATA *pDNPData = (MDNPO085_DATA *)pData;
  mdnpdata_storeDatasetProtoUUID(pData->pDbHandle, pDNPData->pointNumber, pDNPData->uuid); 
  return(TMWDEFS_TRUE);
}

/* function: _dbStoreElemFunc */
static TMWTYPES_BOOL TMWDEFS_LOCAL _dbStoreElemFunc(
  TMWDB_DATA *pData)
{
  MDNPO085_ELEM_DATA *pDNPData = (MDNPO085_ELEM_DATA *)pData;
  mdnpdata_storeDatasetProto(pData->pDbHandle, pDNPData->pointNumber, pDNPData->elemIndex, &pDNPData->descr); 
  return(TMWDEFS_TRUE);
}

/* function: _storeDatasetProto */
static void TMWDEFS_LOCAL _storeDatasetProto(
  void *pDbHandle, 
  TMWTYPES_USHORT pointNumber, 
  TMWTYPES_UCHAR *pUUID)
{
  /* Allocate new binary data structure */
  MDNPO085_DATA *pDNPData = (MDNPO085_DATA *)tmwtarg_alloc(sizeof(MDNPO085_DATA));
  if (pDNPData == TMWDEFS_NULL)
  {
    return;
  }

  /* Initialize data independent parts */
  tmwdb_initData((TMWDB_DATA *)pDNPData, pDbHandle, _dbStoreFunc);

  /* Copy data dependent parts */
  memcpy(pDNPData->uuid, pUUID, 16);
  pDNPData->pointNumber = pointNumber;

  /* Store in database queue */
  if(!tmwdb_addEntry((TMWDB_DATA *)pDNPData))
    tmwtarg_free(pDNPData);
}

/* function: _storeDatasetProtoElem */
static void TMWDEFS_LOCAL _storeDatasetProtoElem(
  void *pDbHandle, 
  TMWTYPES_USHORT pointNumber, 
  TMWTYPES_UCHAR  elemIndex,
  DNPDATA_DATASET_DESCR_ELEM *pDescr)
{
  MDNPO085_ELEM_DATA *pDNPData;
  TMWTYPES_USHORT bufSize = sizeof(MDNPO085_ELEM_DATA) + pDescr->ancillaryValue.length;

  /* Allocate data structure */
  pDNPData = (MDNPO085_ELEM_DATA *)tmwtarg_alloc(bufSize);
  if (pDNPData == TMWDEFS_NULL)
  {
    return;
  }

  /* Initialize data independent parts */
  tmwdb_initData((TMWDB_DATA *)pDNPData, pDbHandle, _dbStoreElemFunc);

  /* Copy data dependent parts */
  pDNPData->descr = *pDescr;

  /* If pointer to string, copy string itself and point to that string */
  if(pDescr->ancillaryValue.type == DNPDATA_VALUE_STRPTR)
  { 
    memcpy(pDNPData->data, pDescr->ancillaryValue.value.pStrValue, pDescr->ancillaryValue.length);
    pDNPData->descr.ancillaryValue.value.pStrValue = pDNPData->data;
  }

  pDNPData->elemIndex = elemIndex;
  pDNPData->pointNumber = pointNumber;

  /* Store in database queue */
  if(!tmwdb_addEntry((TMWDB_DATA *)pDNPData))
    tmwtarg_free(pDNPData);
}
#else
/* function: _storeDatasetProto */
static void TMWDEFS_LOCAL _storeDatasetProto(
  void *pDbHandle, 
  TMWTYPES_USHORT pointNumber, 
  TMWTYPES_UCHAR *pUUID)
{ 
  mdnpdata_storeDatasetProtoUUID(pDbHandle, pointNumber, pUUID); 
}

/* function: _storeDatasetProtoElem */
static void TMWDEFS_LOCAL _storeDatasetProtoElem(
  void *pDbHandle, 
  TMWTYPES_USHORT pointNumber, 
  TMWTYPES_UCHAR  elemIndex,
  DNPDATA_DATASET_DESCR_ELEM *pDescr)
{ 
  mdnpdata_storeDatasetProto(pDbHandle, pointNumber, elemIndex, pDescr); 
}
#endif /* TMWCNFG_SUPPORT_ASYNCH_DB */
 
/* function: mdnpo085_readObj85v1 */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpo085_readObj85v1(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{
  MDNPSESN *pMDNPSession;
  TMWTYPES_USHORT descrIndex;
  DNPDATA_DATASET_DESCR_ELEM descr;
  
  if(pObjHeader->qualifier != DNPDEFS_QUAL_16BIT_FREE_FORMAT)
    return(TMWDEFS_FALSE);

  pMDNPSession = (MDNPSESN *)pSession;

  for(descrIndex = 0; descrIndex < pObjHeader->numberOfPoints; descrIndex++)
  {
    TMWTYPES_BOOL invalidLength;
    int elemIndex;
    int diagIndex;
    TMWTYPES_ULONG protoId;
    TMWTYPES_ULONG stopIndex;
    TMWTYPES_USHORT sizePrefix;
    TMWTYPES_UCHAR length;

    elemIndex = 0;
    diagIndex = 1;
    protoId = 0;
    stopIndex = 0;

    /* Protect against badly formatted message */
    invalidLength = TMWDEFS_TRUE;
    if((pRxFragment->offset+2) <= pRxFragment->msgLength)
    { 
      tmwtarg_get16(&pRxFragment->pMsgBuf[pRxFragment->offset], &sizePrefix);
      pRxFragment->offset += 2;

      stopIndex = pRxFragment->offset + sizePrefix;
      if(stopIndex <= pRxFragment->msgLength)
      {
        invalidLength = TMWDEFS_FALSE;
      }
    }

    /* Protect against badly formatted message */
    if(invalidLength)
    {
      DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_INVALID_SIZE);
      return(TMWDEFS_FALSE);
    }

    /* Make sure at least length byte is there */
    while(pRxFragment->offset < stopIndex)
    {
      length = pRxFragment->pMsgBuf[pRxFragment->offset++]; 
      
      /* Protect against badly formatted message */
      if((length < 3) 
        || ((pRxFragment->offset+length) > stopIndex))
      {
        DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_INVALID_SIZE);
        return(TMWDEFS_FALSE);
      }

      descr.descrElemType = (DNPDEFS_DATASET_DESCR_CODE)pRxFragment->pMsgBuf[pRxFragment->offset++];  
      descr.dataTypeCode = (DNPDEFS_DATASET_TYPE_CODE)pRxFragment->pMsgBuf[pRxFragment->offset++]; 
      descr.maxDataLength = pRxFragment->pMsgBuf[pRxFragment->offset++]; 

      length = length - 3;
      dnputil_getAncValueFromMessage(pRxFragment, descr.descrElemType, length, &descr.ancillaryValue);
    
      if(elemIndex == 0)
      {
        /* First element contains mandatory prototype id (point index) */
        if(descr.descrElemType != DNPDEFS_DATASET_DESCR_ID)
        {
          DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_DATASET_MANDATORY);
          return(TMWDEFS_FALSE);
        }
        protoId = descr.ancillaryValue.value.uint32Value;
      } 
      else if(elemIndex == 1)
      {
        /* Second element contains mandatory UUID */
        if(descr.descrElemType != DNPDEFS_DATASET_DESCR_UUID)
        {
          DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_DATASET_MANDATORY);
          return(TMWDEFS_FALSE);
        }
        _storeDatasetProto(pMDNPSession->pDbHandle, (TMWTYPES_USHORT)protoId, descr.ancillaryValue.value.pStrValue); 
        DNPDIAG_SHOW_DATASET_PROTO(pSession, (TMWTYPES_USHORT)protoId, 0, &descr, TMWDIAG_ID_RX);
      }
      else
      {
        _storeDatasetProtoElem(pMDNPSession->pDbHandle, (TMWTYPES_USHORT)protoId, (TMWTYPES_UCHAR)(elemIndex-2), &descr); 
        DNPDIAG_SHOW_DATASET_PROTO(pSession, (TMWTYPES_USHORT)protoId, (TMWTYPES_UCHAR)diagIndex, &descr, TMWDIAG_ID_RX);
        diagIndex++;
      }
      elemIndex++;
    } 
  }

  return(TMWDEFS_TRUE);
}

/* function: mdnpo085_writeV1 */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpo085_writeV1( 
  TMWSESN_TX_DATA  *pTxData,
  MDNPBRM_REQ_DESC *pReqDesc,
  TMWSESN_TX_DATA  *pUserTxData,
  TMWTYPES_UCHAR    numObjects,
  TMWTYPES_USHORT  *pPointNumbers)
{
  DNPDATA_DATASET_DESCR_ELEM *pDescrElem;
  MDNPSESN *pMDNPSession; 
  TMWTYPES_UCHAR uuid[16];
  TMWTYPES_USHORT sizePrefixIndex;
  TMWTYPES_USHORT dataLength;
  TMWTYPES_USHORT pointNumber; 
  TMWTYPES_USHORT quantityIndex;
  TMWTYPES_UCHAR  numberElems;
  int i;
  int diagIndex = 1;

  pMDNPSession = (MDNPSESN *)pReqDesc->pSession; 

  /* If point numbers are specified by user, use those.
   * If not specified, start with index of first prototype
   * that was defined here on the master. 
   */
  if(pPointNumbers != TMWDEFS_NULL)
  {
    pointNumber = *pPointNumbers++;
  }
  else
  {
    /* Master indexes start after slave prototypes */
    pointNumber = mdnpdata_datasetProtoSlaveQty(pMDNPSession->pDbHandle);
    numObjects = 0xff;
  }

  if(!mdnpbrm_addObjectHeader((DNPCHNL_TX_DATA *)pTxData, DNPDEFS_OBJ_85_DATASET_PROTO, 1, 
    DNPDEFS_QUAL_16BIT_FREE_FORMAT, 0, 1))
  {
    return(TMWDEFS_FALSE);
  }

  quantityIndex = pTxData->msgLength-1;

  for(i=0; i<numObjects; i++)
  {  
    int j;
    TMWTYPES_UCHAR length;

    /* Returns pointer to Data set Descriptor in ROM or RAM and number of elements in descriptor */
    pDescrElem = mdnpdata_datasetProtoGet(pMDNPSession->pDbHandle, pointNumber, &numberElems, uuid);
    if(pDescrElem == TMWDEFS_NULL)
    {
      if((numObjects != 0xff) || (i == 0))
      {
        /* Can't get the specified prototypes to send
         * or there are no prototypes 
         */
        return(TMWDEFS_FALSE);
      }
      else 
      {
        /* all prototypes have been sent */
        break;
      }
    }

    length = dnputil_lengthRequired(pointNumber);
    if((pTxData->msgLength + length + 23) > pTxData->maxLength)
    {
      mdnpdata_datasetProtoRelease(pMDNPSession->pDbHandle);
      MDNPDIAG_ERROR(pReqDesc->pChannel, pReqDesc->pSession, MDNPDIAG_OBJ_LENGTH);
      return(TMWDEFS_FALSE);
    }

    /* save pointer for size prefix */
    sizePrefixIndex = pTxData->msgLength;
    pTxData->msgLength += 2; 

    /* Put mandatory prototype id in message first */
    pTxData->pMsgBuf[pTxData->msgLength++] = length+3;
    pTxData->pMsgBuf[pTxData->msgLength++] = DNPDEFS_DATASET_DESCR_ID;
    pTxData->pMsgBuf[pTxData->msgLength++] = DNPDEFS_DATASET_TYPE_NONE;
    pTxData->pMsgBuf[pTxData->msgLength++] = 0; /* Max length */
    dnputil_putIntInMessage(&pTxData->pMsgBuf[pTxData->msgLength], pointNumber, length);
    pTxData->msgLength = (TMWTYPES_USHORT)(pTxData->msgLength + length);

    /* Put mandatory UUID in message next */
    pTxData->pMsgBuf[pTxData->msgLength++] = 19;
    pTxData->pMsgBuf[pTxData->msgLength++] = DNPDEFS_DATASET_DESCR_UUID;
    pTxData->pMsgBuf[pTxData->msgLength++] = DNPDEFS_DATASET_TYPE_NONE;
    pTxData->pMsgBuf[pTxData->msgLength++] = 0; /* Max length */
    memcpy(&pTxData->pMsgBuf[pTxData->msgLength], uuid, 16);
    pTxData->msgLength = (TMWTYPES_USHORT)(pTxData->msgLength + 16);

#if TMWCNFG_SUPPORT_DIAG
    { /* Diagnostics */
      DNPDATA_DATASET_DESCR_ELEM  elem;
      elem.descrElemType = DNPDEFS_DATASET_DESCR_UUID;
      elem.dataTypeCode  = DNPDEFS_DATASET_TYPE_NONE;
      elem.maxDataLength = 0; 
      elem.ancillaryValue.type = DNPDATA_VALUE_STRPTR;
      elem.ancillaryValue.length = 16; 
      elem.ancillaryValue.value.pStrValue = uuid;
      DNPDIAG_SHOW_DATASET_PROTO(pReqDesc->pSession, pointNumber, 0, &elem, 0);
    }
#endif

    for(j=0; j<numberElems; j++)
    {
      /* Make sure this fits in fragment */
      if((pTxData->msgLength + pDescrElem->ancillaryValue.length + 4) > pTxData->maxLength)
      {
        mdnpdata_datasetProtoRelease(pMDNPSession->pDbHandle);
        MDNPDIAG_ERROR(pReqDesc->pChannel, pReqDesc->pSession, MDNPDIAG_OBJ_LENGTH);
        return(TMWDEFS_FALSE);
      }

      pTxData->pMsgBuf[pTxData->msgLength++] = pDescrElem->ancillaryValue.length +3;
      pTxData->pMsgBuf[pTxData->msgLength++] = (TMWTYPES_UCHAR)pDescrElem->descrElemType;
      pTxData->pMsgBuf[pTxData->msgLength++] = (TMWTYPES_UCHAR)pDescrElem->dataTypeCode;
      pTxData->pMsgBuf[pTxData->msgLength++] = pDescrElem->maxDataLength;

      dnputil_putValueInMessage(pTxData->pMsgBuf, &pTxData->msgLength, &pDescrElem->ancillaryValue);

      /* Diagnostics */
      DNPDIAG_SHOW_DATASET_PROTO(pReqDesc->pSession, pointNumber, (TMWTYPES_UCHAR)diagIndex, pDescrElem, 0);
      diagIndex++;
      pDescrElem++;
    }

    mdnpdata_datasetProtoRelease(pMDNPSession->pDbHandle);

    dataLength = pTxData->msgLength - (sizePrefixIndex +2);
    tmwtarg_store16(&dataLength, &pTxData->pMsgBuf[sizePrefixIndex]);

    if(pPointNumbers != TMWDEFS_NULL)
      pointNumber = *pPointNumbers++;
    else
      pointNumber++;
  }  

  /* Update number of prototypes in message */
  pTxData->pMsgBuf[quantityIndex] = (TMWTYPES_UCHAR)i;

  /* If user tx data was passed in, don't send it, let user send it */
  if(pUserTxData != TMWDEFS_NULL)
    return(TMWDEFS_TRUE);

  /* Send request */
  if(!dnpchnl_sendFragment(pTxData))
  { 
    return(TMWDEFS_FALSE);
  }
 
  return(TMWDEFS_TRUE);
} 
 
#endif
