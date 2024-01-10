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

/* file: mdnpo086.c
 * description: DNP Master functionality for Object 86 Data Set Descriptor
 */
#include "tmwscl/dnp/dnpdiag.h"
#include "tmwscl/dnp/mdnpo086.h"
#include "tmwscl/dnp/mdnpbrm.h"
#include "tmwscl/dnp/mdnpdiag.h"
#include "tmwscl/dnp/mdnpcnfg.h"
#include "tmwscl/dnp/dnputil.h"

#include "tmwscl/utils/tmwdb.h"
#include "tmwscl/utils/tmwtarg.h"

#if MDNPDATA_SUPPORT_OBJ86

#if TMWCNFG_SUPPORT_ASYNCH_DB
typedef struct MDNPO086ContDataStruct {
  TMWDB_DATA tmw;
  DNPDATA_DATASET_DESCR_ELEM descr;
  TMWTYPES_UCHAR  elemIndex;
  TMWTYPES_USHORT pointNumber;

  /* Memory to copy string data to, length of data array will be determined 
   * when allocated
   */
  TMWTYPES_UCHAR              data[1];
} MDNPO086_CONTENTS_DATA;

typedef struct MDNPO086CharsDataStruct {
  TMWDB_DATA tmw;
  TMWTYPES_UCHAR  characteristics;
  TMWTYPES_USHORT pointNumber;
} MDNPO086_CHARS_DATA;

typedef struct MDNPO086IndexDataStruct {
  TMWDB_DATA tmw; 
  DNPDATA_DATASET_DESCR_INDEX descr;
  TMWTYPES_UCHAR  elemIndex;
  TMWTYPES_USHORT pointNumber;
} MDNPO086_INDEX_DATA;

/* function: _dbStoreContFunc */
static TMWTYPES_BOOL TMWDEFS_LOCAL _dbStoreContFunc(
  TMWDB_DATA *pData)
{
  MDNPO086_CONTENTS_DATA *pDNPData = (MDNPO086_CONTENTS_DATA *)pData;
  mdnpdata_storeDatasetDescrCont(pData->pDbHandle, pDNPData->pointNumber, pDNPData->elemIndex, &pDNPData->descr); 
  return(TMWDEFS_TRUE);
}

/* function: _dbStoreCharsFunc */
static TMWTYPES_BOOL TMWDEFS_LOCAL _dbStoreCharsFunc(
  TMWDB_DATA *pData)
{
  MDNPO086_CHARS_DATA *pDNPData = (MDNPO086_CHARS_DATA *)pData;
  mdnpdata_storeDatasetDescrChars(pData->pDbHandle, pDNPData->pointNumber, pDNPData->characteristics);
  return(TMWDEFS_TRUE);
}

/* function: _dbStoreIndexFunc */
static TMWTYPES_BOOL TMWDEFS_LOCAL _dbStoreIndexFunc(
  TMWDB_DATA *pData)
{
  MDNPO086_INDEX_DATA *pDNPData = (MDNPO086_INDEX_DATA *)pData;
  mdnpdata_storeDatasetDescrIndex(pData->pDbHandle, pDNPData->pointNumber, pDNPData->elemIndex, &pDNPData->descr); 
  return(TMWDEFS_TRUE);
}

/* function: _storeDatasetDescrCont */
static void TMWDEFS_LOCAL _storeDatasetDescrCont(
  void *pDbHandle, 
  TMWTYPES_USHORT pointNumber, 
  TMWTYPES_UCHAR  elemIndex,
  DNPDATA_DATASET_DESCR_ELEM *pDescr)
{
  MDNPO086_CONTENTS_DATA *pDNPData;
  TMWTYPES_USHORT bufSize = sizeof(MDNPO086_CONTENTS_DATA) + pDescr->ancillaryValue.length;

  /* Allocate data structure */
  pDNPData = (MDNPO086_CONTENTS_DATA *)tmwtarg_alloc(bufSize);
  if (pDNPData == TMWDEFS_NULL)
  {
    return;
  }

  /* Initialize data independent parts */
  tmwdb_initData((TMWDB_DATA *)pDNPData, pDbHandle, _dbStoreContFunc);

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

/* function: _storeDatasetDescrChars */
static void TMWDEFS_LOCAL _storeDatasetDescrChars(
  void *pDbHandle, 
  TMWTYPES_USHORT pointNumber, 
  TMWTYPES_UCHAR characteristics)
{
  /* Allocate new binary data structure */
  MDNPO086_CHARS_DATA *pDNPData = (MDNPO086_CHARS_DATA *)tmwtarg_alloc(sizeof(MDNPO086_CHARS_DATA));
  if (pDNPData == TMWDEFS_NULL)
  {
    return;
  }

  /* Initialize data independent parts */
  tmwdb_initData((TMWDB_DATA *)pDNPData, pDbHandle, _dbStoreCharsFunc);

  /* Copy data dependent parts */
  pDNPData->characteristics = characteristics;
  pDNPData->pointNumber = pointNumber;

  /* Store in database queue */
  if(!tmwdb_addEntry((TMWDB_DATA *)pDNPData))
    tmwtarg_free(pDNPData);
}

/* function: _storeDatasetDescrIndex */
static void TMWDEFS_LOCAL _storeDatasetDescrIndex(
  void *pDbHandle, 
  TMWTYPES_USHORT pointNumber, 
  TMWTYPES_UCHAR  elemIndex,
  DNPDATA_DATASET_DESCR_INDEX *pDescr)
{
  /* Allocate new binary data structure */
  MDNPO086_INDEX_DATA *pDNPData = (MDNPO086_INDEX_DATA *)tmwtarg_alloc(sizeof(MDNPO086_INDEX_DATA));
  if (pDNPData == TMWDEFS_NULL)
  {
    return;
  }

  /* Initialize data independent parts */
  tmwdb_initData((TMWDB_DATA *)pDNPData, pDbHandle, _dbStoreIndexFunc);

  /* Copy data dependent parts */
  pDNPData->descr = *pDescr;
  pDNPData->elemIndex = elemIndex;
  pDNPData->pointNumber = pointNumber;

  /* Store in database queue */
  if(!tmwdb_addEntry((TMWDB_DATA *)pDNPData))
    tmwtarg_free(pDNPData);
}
#else
/* function: _storeDatasetDescrCont */
static void TMWDEFS_LOCAL _storeDatasetDescrCont(
  void *pDbHandle, 
  TMWTYPES_USHORT pointNumber, 
  TMWTYPES_UCHAR  elemIndex,
  DNPDATA_DATASET_DESCR_ELEM *pDescr)
{ 
  mdnpdata_storeDatasetDescrCont(pDbHandle, pointNumber, elemIndex, pDescr); 
}

/* function: _storeDatasetDescrChars */
static void TMWDEFS_LOCAL _storeDatasetDescrChars(
  void *pDbHandle, 
  TMWTYPES_USHORT pointNumber, 
  TMWTYPES_UCHAR characteristics)
{ 
  mdnpdata_storeDatasetDescrChars(pDbHandle, pointNumber, characteristics);
}
/* function: _storeDatasetDescrIndex */
static void TMWDEFS_LOCAL _storeDatasetDescrIndex(
  void *pDbHandle, 
  TMWTYPES_USHORT pointNumber, 
  TMWTYPES_UCHAR  elemIndex,
  DNPDATA_DATASET_DESCR_INDEX *pDescr)
{ 
  mdnpdata_storeDatasetDescrIndex(pDbHandle, pointNumber, elemIndex, pDescr); 
}
#endif /* TMWCNFG_SUPPORT_ASYNCH_DB */

#if MDNPDATA_SUPPORT_OBJ86_V1
/* function: mdnpo086_readObj86v1 */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpo086_readObj86v1(
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
    int elemIndex;
    TMWTYPES_BOOL invalidLength;
    TMWTYPES_ULONG stopIndex;
    TMWTYPES_ULONG datasetId;
    TMWTYPES_USHORT sizePrefix;
    TMWTYPES_UCHAR length;

    datasetId = 0xffffffff;

    elemIndex = 0;
    stopIndex = 0;
    invalidLength = TMWDEFS_TRUE;

    /* Protect against badly formatted message */
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
     
      /* First element contains mandatory data set id (point index) */
      if(datasetId == 0xffffffff)
      {
        if(descr.descrElemType != DNPDEFS_DATASET_DESCR_ID)
        {
          DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_DATASET_MANDATORY);
          return(TMWDEFS_FALSE);
        }
        datasetId = descr.ancillaryValue.value.uint32Value;
      }
      else
      {
        _storeDatasetDescrCont(pMDNPSession->pDbHandle, (TMWTYPES_USHORT)datasetId, (TMWTYPES_UCHAR)elemIndex, &descr); 
        DNPDIAG_SHOW_DATASET_DSCR_CONT(pSession, (TMWTYPES_USHORT)datasetId, (TMWTYPES_UCHAR)elemIndex, &descr, TMWDIAG_ID_RX);
        elemIndex++;
      }
    } 
  }

  return(TMWDEFS_TRUE);
}

/* function: mdnpo086_writeV1 */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpo086_writeV1( 
  TMWSESN_TX_DATA  *pTxData,
  MDNPBRM_REQ_DESC *pReqDesc,
  TMWSESN_TX_DATA  *pUserTxData,
  TMWTYPES_UCHAR    numObjects,
  TMWTYPES_USHORT  *pPointNumbers)
{
  DNPDATA_DATASET_DESCR_ELEM *pDescrElem;
  MDNPSESN *pMDNPSession; 
  int i;
  TMWTYPES_USHORT sizePrefixIndex;
  TMWTYPES_USHORT dataLength;
  TMWTYPES_USHORT pointNumber;
  TMWTYPES_USHORT quantityIndex;
  TMWTYPES_UCHAR numberElems;

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

  if(!mdnpbrm_addObjectHeader((DNPCHNL_TX_DATA *)pTxData, DNPDEFS_OBJ_86_DATASET_DESCR, 1, 
    DNPDEFS_QUAL_16BIT_FREE_FORMAT, 0, numObjects))
  { 
    return(TMWDEFS_FALSE);
  }

  quantityIndex = pTxData->msgLength-1;

  for(i=0; i<numObjects; i++)
  {
    int elemIndex;
    TMWTYPES_UCHAR length;

    pMDNPSession = (MDNPSESN *)pReqDesc->pSession; 

    /* Returns pointer to Data set Descriptor in ROM or RAM and number of elements in descriptor */
    pDescrElem = mdnpdata_datasetDescrGetCont(pMDNPSession->pDbHandle, pointNumber, &numberElems);
    if(pDescrElem == TMWDEFS_NULL)
    {
      if((numObjects != 0xff) || (i == 0))
      {
        /* Can't get the specified descriptors to send
         * or there are no descriptors 
         */

        return(TMWDEFS_FALSE);
      }
      else 
      {
        /* all descriptors have been sent */
        break;
      }
    }

    /* Is there enough room? */
    length = dnputil_lengthRequired(pointNumber);
    if((pTxData->msgLength + length + 6) > pTxData->maxLength)
    {
      mdnpdata_datasetDescrRelease(pMDNPSession->pDbHandle);
      return(TMWDEFS_FALSE);
    }

    /* save pointer for size prefix */
    sizePrefixIndex = pTxData->msgLength;
    pTxData->msgLength += 2; 

    /* Put mandatory data set id in first */
    pTxData->pMsgBuf[pTxData->msgLength++] = length+3;
    pTxData->pMsgBuf[pTxData->msgLength++] = DNPDEFS_DATASET_DESCR_ID;
    pTxData->pMsgBuf[pTxData->msgLength++] = DNPDEFS_DATASET_TYPE_NONE;
    pTxData->pMsgBuf[pTxData->msgLength++] = 0; /* Max length */
    dnputil_putIntInMessage(&pTxData->pMsgBuf[pTxData->msgLength], pointNumber, length);
    pTxData->msgLength = (TMWTYPES_USHORT)(pTxData->msgLength + length);

    for(elemIndex=0; elemIndex<numberElems; elemIndex++)
    {
      /* Is there enough room? */
      if((pTxData->msgLength + pDescrElem->ancillaryValue.length + 4) > pTxData->maxLength)
      {
        mdnpdata_datasetDescrRelease(pMDNPSession->pDbHandle);
        return(TMWDEFS_FALSE);
      }

      pTxData->pMsgBuf[pTxData->msgLength++] = pDescrElem->ancillaryValue.length +3;
      pTxData->pMsgBuf[pTxData->msgLength++] = (TMWTYPES_UCHAR)pDescrElem->descrElemType;
      pTxData->pMsgBuf[pTxData->msgLength++] = (TMWTYPES_UCHAR)pDescrElem->dataTypeCode;
      pTxData->pMsgBuf[pTxData->msgLength++] = pDescrElem->maxDataLength;

      dnputil_putValueInMessage(pTxData->pMsgBuf, &pTxData->msgLength, &pDescrElem->ancillaryValue);

      /* Diagnostics */
      DNPDIAG_SHOW_DATASET_DSCR_CONT(pReqDesc->pSession, pointNumber, (TMWTYPES_UCHAR)elemIndex, pDescrElem, 0);

      pDescrElem++;
    }

    mdnpdata_datasetDescrRelease(pMDNPSession->pDbHandle);

    dataLength = pTxData->msgLength - (sizePrefixIndex +2);
    tmwtarg_store16(&dataLength, &pTxData->pMsgBuf[sizePrefixIndex]);

    if(pPointNumbers != TMWDEFS_NULL)
      pointNumber = *pPointNumbers++;
    else
      pointNumber++;
  }  

  /* Update number of objects written */
  pTxData->pMsgBuf[quantityIndex] = (TMWTYPES_UCHAR)i;

  if(pUserTxData != TMWDEFS_NULL)
    return(TMWDEFS_TRUE);

  /* Send request */
  if(!dnpchnl_sendFragment(pTxData))
  { 
    return(TMWDEFS_FALSE);
  }

  return(TMWDEFS_TRUE);
} 
 
#endif /* MDNPDATA_SUPPORT_OBJ86_V1 */

#if MDNPDATA_SUPPORT_OBJ86_V2
/* function: mdnpo086_readObj86v2 */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpo086_readObj86v2(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{
  MDNPSESN *pMDNPSession = (MDNPSESN *)pSession;
  TMWTYPES_USHORT pointNumber;
  TMWTYPES_USHORT index;
  TMWTYPES_UCHAR value;

  for(index = 0; index < pObjHeader->numberOfPoints; index++)
  {
    dnputil_getPointNumber(pRxFragment, pObjHeader, index, &pointNumber);

    /* Protect against badly formatted message */
    if(pRxFragment->offset >= pRxFragment->msgLength)
    {
      DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_INVALID_SIZE);
      return(TMWDEFS_FALSE);
    }

    value = pRxFragment->pMsgBuf[pRxFragment->offset++];

    _storeDatasetDescrChars(pMDNPSession->pDbHandle, pointNumber, value);

    DNPDIAG_SHOW_DATASET_DSCR_CHRS(pSession, pointNumber, value, TMWDIAG_ID_RX);
  }
  return(TMWDEFS_TRUE);
}
#endif /* MDNPDATA_SUPPORT_OBJ86_V2 */

#if MDNPDATA_SUPPORT_OBJ86_V3
/* function: mdnpo086_readObj86v3 */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpo086_readObj86v3(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{
  MDNPSESN *pMDNPSession;
  TMWTYPES_USHORT descrIndex;
  DNPDATA_DATASET_DESCR_INDEX descr;

  if(pObjHeader->qualifier != DNPDEFS_QUAL_16BIT_FREE_FORMAT)
    return(TMWDEFS_FALSE);

  pMDNPSession = (MDNPSESN *)pSession;

  for(descrIndex = 0; descrIndex < pObjHeader->numberOfPoints; descrIndex++)
  {
    int elemIndex;
    TMWTYPES_BOOL invalidLength;
    TMWTYPES_ULONG  datasetId;
    TMWTYPES_ULONG stopIndex;
    TMWTYPES_USHORT sizePrefix;
    TMWTYPES_UCHAR length;

    elemIndex = 0;
    stopIndex = 0;
    length = 0;

    /* Protect against badly formatted message */
    invalidLength = TMWDEFS_TRUE;
    if((pRxFragment->offset+2) <= pRxFragment->msgLength)
    {  
      tmwtarg_get16(&pRxFragment->pMsgBuf[pRxFragment->offset], &sizePrefix);
      pRxFragment->offset += 2;

      stopIndex = pRxFragment->offset + sizePrefix;

      /* Protect against badly formatted message */
      if(stopIndex <= pRxFragment->msgLength)
      { 
        /* Get length of data set id 1, 2 or 4*/
        length = pRxFragment->pMsgBuf[pRxFragment->offset++]; 

        /* Protect against badly formatted message */
        if(pRxFragment->offset+length <= stopIndex)
        {
          invalidLength = TMWDEFS_FALSE;
        }
      }
    }

    /* Protect against badly formatted message */
    if(invalidLength)
    {
      DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_INVALID_SIZE);
      return(TMWDEFS_FALSE);
    }

    datasetId = dnputil_getUintFromMessage(&pRxFragment->pMsgBuf[pRxFragment->offset], length);
    pRxFragment->offset = pRxFragment->offset + length;

    /* Now get the individual Point Types and Point Indexes */
    while(pRxFragment->offset < stopIndex)
    {
      length = pRxFragment->pMsgBuf[pRxFragment->offset++]; 

      /* Protect against badly formatted message */
      if((length == 0)
        || ((pRxFragment->offset+length) > stopIndex))
      {
        DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_INVALID_SIZE);
        return(TMWDEFS_FALSE);
      }

      descr.pointType = pRxFragment->pMsgBuf[pRxFragment->offset++];  

      length--;
      descr.pointIndex = dnputil_getUintFromMessage(&pRxFragment->pMsgBuf[pRxFragment->offset], length);
      pRxFragment->offset = pRxFragment->offset + length;

      _storeDatasetDescrIndex(pMDNPSession->pDbHandle, (TMWTYPES_USHORT)datasetId, (TMWTYPES_UCHAR)elemIndex, &descr); 
     
      DNPDIAG_SHOW_DATASET_DSCR_INDX(pSession, (TMWTYPES_USHORT)datasetId, (TMWTYPES_UCHAR)elemIndex, &descr, TMWDIAG_ID_RX);
  
      elemIndex++;
    }
  }

  return(TMWDEFS_TRUE);
}

/* function: mdnpo086_writeV3 */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpo086_writeV3( 
  TMWSESN_TX_DATA *pTxData,
  MDNPBRM_REQ_DESC *pReqDesc,
  TMWSESN_TX_DATA  *pUserTxData,
  TMWTYPES_UCHAR    numObjects,
  TMWTYPES_USHORT  *pPointNumbers)
{
  DNPDATA_DATASET_DESCR_INDEX *pDescrElem;
  MDNPSESN *pMDNPSession; 
  int i, j;
  TMWTYPES_USHORT sizePrefixIndex;
  TMWTYPES_USHORT length;
  TMWTYPES_UCHAR  numberElems;
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

  if(!mdnpbrm_addObjectHeader((DNPCHNL_TX_DATA *)pTxData, DNPDEFS_OBJ_86_DATASET_DESCR, 3, 
    DNPDEFS_QUAL_16BIT_FREE_FORMAT, 0, numObjects))
  {
    return(TMWDEFS_FALSE);
  }

  quantityIndex = pTxData->msgLength-1;

  for(i=0; i<numObjects; i++)
  {
    TMWTYPES_UCHAR indexLength;

    /* Returns pointer to Data set Descriptor in ROM or RAM and number of elements in descriptor */
    pDescrElem = mdnpdata_datasetDescrGetIndex(pMDNPSession->pDbHandle, pointNumber, &numberElems);
    if(pDescrElem == TMWDEFS_NULL)
    {
      if((numObjects != 0xff) || (i == 0))
      {
        /* Can't get the specified descriptors to send
         * or there are no descriptors 
         */
        return(TMWDEFS_FALSE);
      }
      else 
      {
        /* all descriptors have been sent */
        break;
      }
    } 

    /* Is there enough room? */
    indexLength = dnputil_lengthRequired(pointNumber);
    if((pTxData->msgLength + indexLength + 3) > pTxData->maxLength)
    {
      mdnpdata_datasetDescrRelease(pMDNPSession->pDbHandle);
      return(TMWDEFS_FALSE);
    }

    /* save pointer for size prefix */
    sizePrefixIndex = pTxData->msgLength;
    pTxData->msgLength += 2; 

    pTxData->pMsgBuf[pTxData->msgLength++] = indexLength;
    dnputil_putIntInMessage(&pTxData->pMsgBuf[pTxData->msgLength], pointNumber, indexLength);
    pTxData->msgLength = (TMWTYPES_USHORT)(pTxData->msgLength + indexLength);

    for(j=0; j<numberElems; j++)
    {
      indexLength = dnputil_lengthRequired(pDescrElem->pointIndex);

      /* Is there enough room? */
      if((pTxData->msgLength + indexLength + 2) > pTxData->maxLength)
      {
        mdnpdata_datasetDescrRelease(pMDNPSession->pDbHandle);
        return(TMWDEFS_FALSE);
      }
      pTxData->pMsgBuf[pTxData->msgLength++] = indexLength +1;
      pTxData->pMsgBuf[pTxData->msgLength++] = pDescrElem->pointType;

      dnputil_putIntInMessage(&pTxData->pMsgBuf[pTxData->msgLength], pDescrElem->pointIndex, (TMWTYPES_UCHAR)indexLength);
      pTxData->msgLength = (TMWTYPES_USHORT)(pTxData->msgLength + indexLength);
   
      pDescrElem++;
    }

    mdnpdata_datasetDescrRelease(pMDNPSession->pDbHandle);

    length = pTxData->msgLength - (sizePrefixIndex +2);
    tmwtarg_store16(&length, &pTxData->pMsgBuf[sizePrefixIndex]);

    if(pPointNumbers != TMWDEFS_NULL)
      pointNumber = *pPointNumbers++;
    else
      pointNumber++;
  }  

  /* Update number of objects written */
  pTxData->pMsgBuf[quantityIndex] = (TMWTYPES_UCHAR)i;

  if(pUserTxData != TMWDEFS_NULL)
    return(TMWDEFS_TRUE);

  /* Send request */
  if(!dnpchnl_sendFragment(pTxData))
  { 
    return(TMWDEFS_FALSE);
  }

  return(TMWDEFS_TRUE);
} 

#endif /* MDNPDATA_SUPPORT_OBJ86_V3 */

#endif
