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

/* file: mdnpo000.c
 * description: DNP Master functionality for Object 0 Device Attributes
 */
#include "tmwscl/dnp/dnpdiag.h"
#include "tmwscl/dnp/mdnpo000.h"
#include "tmwscl/dnp/mdnpdata.h"
#include "tmwscl/dnp/mdnpcnfg.h"
#include "tmwscl/dnp/mdnpbrm.h"

#include "tmwscl/utils/tmwdb.h"
#include "tmwscl/utils/tmwtarg.h"

#if MDNPDATA_SUPPORT_OBJ0
#if TMWCNFG_SUPPORT_ASYNCH_DB

/* Structure used to store data for asynch database updates */
typedef struct MDNPO000DataStruct {
  TMWDB_DATA                  tmw;
  TMWTYPES_USHORT             pointNumber;
  DNPDATA_ATTRIBUTE_VALUE     value;
  TMWTYPES_UCHAR              variation;

  /* Memory to copy string data to, length of data array will be determined 
   * when allocated
   */
  TMWTYPES_UCHAR              data[1];
} MDNPO000_DATA;
 
typedef struct MDNPO000DataPropStruct {
  TMWDB_DATA      tmw;
  TMWTYPES_USHORT pointNumber;
  TMWTYPES_UCHAR  property;
  TMWTYPES_UCHAR  variation;
} MDNPO000_DATA_PROP;

/* function: _dbStoreFunc */
static TMWTYPES_BOOL TMWDEFS_LOCAL _dbStoreFunc(
  TMWDB_DATA *pData)
{
  MDNPO000_DATA *pDNPData = (MDNPO000_DATA *)pData;

  mdnpdata_storeDeviceAttribute(pData->pDbHandle, pDNPData->pointNumber,
    pDNPData->variation, &pDNPData->value);

  return(TMWDEFS_TRUE);
}

/* function: _storeDeviceAttribute */
static TMWTYPES_BOOL TMWDEFS_LOCAL _storeDeviceAttribute(
  void *pDbHandle, 
  TMWTYPES_USHORT pointNumber,
  TMWTYPES_UCHAR variation,
  DNPDATA_ATTRIBUTE_VALUE *pData)
{
  MDNPO000_DATA *pDNPData;
  TMWTYPES_USHORT bufSize = sizeof(MDNPO000_DATA) + pData->length;

  /* Allocate  data structure */
  pDNPData = (MDNPO000_DATA *)tmwtarg_alloc(bufSize);
  if (pDNPData == TMWDEFS_NULL)
  {
    return(TMWDEFS_FALSE);
  }

  /* Initialize data independent parts */
  tmwdb_initData((TMWDB_DATA *)pDNPData, pDbHandle, _dbStoreFunc);

  /* Initialize data dependent parts */
  pDNPData->pointNumber = pointNumber;
  pDNPData->variation = variation;
  pDNPData->value = *pData;

  /* If pointer to string, copy string itself and point to that string */
  if( (pData->type == DNPDEFS_ATTRIBUTE_TYPE_VSTR)
    ||(pData->type == DNPDEFS_ATTRIBUTE_TYPE_OSTR)
    ||(pData->type == DNPDEFS_ATTRIBUTE_TYPE_BSTR))
  {
    memcpy(pDNPData->data, pData->value.pStrValue, pData->length);
    pDNPData->value.value.pStrValue = pDNPData->data;
  }
  /* Store in database queue */
  if(!tmwdb_addEntry((TMWDB_DATA *)pDNPData))
    tmwtarg_free(pDNPData);

  return(TMWDEFS_TRUE);
}

/* function: _dbStorePropertyFunc */
static TMWTYPES_BOOL TMWDEFS_LOCAL _dbStorePropertyFunc(
  TMWDB_DATA *pData)
{
  MDNPO000_DATA_PROP *pDNPData = (MDNPO000_DATA_PROP *)pData;

  mdnpdata_storeDeviceAttrProperty(pData->pDbHandle, pDNPData->pointNumber, pDNPData->variation, pDNPData->property);

  return(TMWDEFS_TRUE);
}

/* function: _storeDeviceAttrProperty */
static void TMWDEFS_LOCAL _storeDeviceAttrProperty(
  void *pDbHandle, 
  TMWTYPES_USHORT pointNumber,
  TMWTYPES_UCHAR variation,
  TMWTYPES_UCHAR property)
{
  /* Allocate new binary data structure */
  MDNPO000_DATA_PROP *pDNPData = (MDNPO000_DATA_PROP *)tmwtarg_alloc(sizeof(MDNPO000_DATA_PROP));
  if (pDNPData == TMWDEFS_NULL)
  {
    return;
  }

  /* Initialize data indepdendent parts */
  tmwdb_initData((TMWDB_DATA *)pDNPData, pDbHandle, _dbStorePropertyFunc);

  /* Initialize data dependent parts */
  pDNPData->pointNumber = pointNumber;
  pDNPData->property = property;
  pDNPData->variation = variation; 

  /* Store in database queue */
  if(!tmwdb_addEntry((TMWDB_DATA *)pDNPData))
    tmwtarg_free(pDNPData);
}
#else

/* function: _storeDeviceAttribute */
static void TMWDEFS_LOCAL _storeDeviceAttribute(
  void *pDbHandle, 
  TMWTYPES_USHORT pointNumber,
  TMWTYPES_UCHAR variation,
  DNPDATA_ATTRIBUTE_VALUE *pData)
{
  mdnpdata_storeDeviceAttribute(pDbHandle, pointNumber, variation, pData);
}

/* function: _storeDeviceAttrProperty */
static void TMWDEFS_LOCAL _storeDeviceAttrProperty(
  void *pDbHandle, 
  TMWTYPES_USHORT pointNumber,
  TMWTYPES_UCHAR variation,
  TMWTYPES_UCHAR property)
{
  mdnpdata_storeDeviceAttrProperty(pDbHandle, pointNumber, variation, property);
}

#endif /* TMWCNFG_SUPPORT_ASYNCH_DB */ 
 
/* function: mdnpo000_readObj0 */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpo000_readObj0(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{
  TMWTYPES_USHORT pointNumber;
  TMWTYPES_USHORT index;
  MDNPSESN *pMDNPSession = (MDNPSESN *)pSession;

  index = 0;
  while(index < pObjHeader->numberOfPoints)
  {
    dnputil_getPointNumber(pRxFragment, pObjHeader, index, &pointNumber);
    
    if(pObjHeader->variation < 255)
    { 
      DNPDATA_ATTRIBUTE_VALUE value;
      DNPCHNL_TX_DATA *pTxData;
      MDNPBRM_DATSET_XFER_CONTEXT *pContextPtr;

      if(!dnputil_getAttrValueFromMessage(pRxFragment, &value))
      { 
        return(TMWDEFS_FALSE);
      }

      DNPDIAG_SHOW_DEVICE_ATTRIBUTE(pSession, pointNumber, pObjHeader->variation, &value, TMWDIAG_ID_RX);

      _storeDeviceAttribute(pMDNPSession->pDbHandle, pointNumber, pObjHeader->variation, &value);
      
      pTxData = (DNPCHNL_TX_DATA *)dnputil_getCurrentMessage(pSession);
      if((pTxData != TMWDEFS_NULL)
        &&(pTxData->pInternalCallbackParam != TMWDEFS_NULL))
      {
        pContextPtr = (MDNPBRM_DATSET_XFER_CONTEXT *)pTxData->pInternalCallbackParam;
        if(pObjHeader->variation == DNPDEFS_OBJ0_NUM_OUTSTA_PROTOS)
          pContextPtr->numberProtosDefinedOnSlave = (TMWTYPES_USHORT)value.value.uintValue;
        else if(pObjHeader->variation == DNPDEFS_OBJ0_NUM_OUTSTA_DATASET)
          pContextPtr->numberDescrsDefinedOnSlave = (TMWTYPES_USHORT)value.value.uintValue;
      } 
    }
    else /* Variation 255 */
    {   
      TMWTYPES_ULONG stopIndex;
      TMWTYPES_USHORT length;
      TMWTYPES_UCHAR  variation;
      TMWTYPES_UCHAR  property; 
      DNPDEFS_ATTRIBUTE_DATA_TYPE type;

      /* Must be at least 2 bytes */
      if(pRxFragment->offset+1 < pRxFragment->msgLength)
      { 
        if(pObjHeader->qualifier == DNPDEFS_QUAL_16BIT_FREE_FORMAT)
        {
          /* Need to allow for original version of variation 255 */
          tmwtarg_get16(&pRxFragment->pMsgBuf[pRxFragment->offset], &length);
          pRxFragment->offset += 2;
        }
        else
        {
          type = (DNPDEFS_ATTRIBUTE_DATA_TYPE)pRxFragment->pMsgBuf[pRxFragment->offset++]; 
          length = pRxFragment->pMsgBuf[pRxFragment->offset++]; 
    
          if(type == DNPDEFS_ATTRIBUTE_TYPE_EXLIST)
          {
            length += 256;
          }
        }
        stopIndex = pRxFragment->offset + length;
        /* Protect against badly formatted message */
        if(stopIndex > pRxFragment->msgLength)
        {
          DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_INVALID_SIZE);
          return(TMWDEFS_FALSE);
        }
      }
      else
      {
        stopIndex = pRxFragment->msgLength;
      }

      /* Must be at least 2 bytes */
      while(pRxFragment->offset+1 < stopIndex)
      {
        variation = pRxFragment->pMsgBuf[pRxFragment->offset++]; 
        property  = pRxFragment->pMsgBuf[pRxFragment->offset++]; 

        DNPDIAG_SHOW_DEVICE_ATTR_PROP(pSession, pointNumber, variation, property);

        _storeDeviceAttrProperty(pMDNPSession->pDbHandle, pointNumber, variation, property);
      }
    }
    index++;
  }
  return(TMWDEFS_TRUE);
} 
#endif
