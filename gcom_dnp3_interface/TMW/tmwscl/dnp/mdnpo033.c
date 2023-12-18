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

/* file: mdnpo033.c
 * description: DNP Master functionality for Object 33 Frozen Analog Input Events
 */
#include "tmwscl/dnp/dnpdiag.h"
#include "tmwscl/dnp/dnputil.h"
#include "tmwscl/dnp/mdnpo033.h"
#include "tmwscl/dnp/mdnpdata.h"
#include "tmwscl/dnp/dnpdtime.h"

#include "tmwscl/utils/tmwdb.h"
#include "tmwscl/utils/tmwtarg.h"

#if MDNPDATA_SUPPORT_OBJ33
#if TMWCNFG_SUPPORT_ASYNCH_DB
typedef struct MDNPO033DataStruct {
  TMWDB_DATA tmw;
  TMWTYPES_UCHAR flags;
  TMWTYPES_ANALOG_VALUE value;
  TMWTYPES_USHORT pointNumber;
  TMWTYPES_BOOL timeSpecified;
  TMWDTIME timeStamp;
} MDNPO033_DATA;

/* function: _dbStoreFunc */
static TMWTYPES_BOOL TMWDEFS_LOCAL _dbStoreFunc(
  TMWDB_DATA *pData)
{
  MDNPO033_DATA *pDNPData = (MDNPO033_DATA *)pData;
  TMWDTIME *pTimeStamp = (pDNPData->timeSpecified) ? &pDNPData->timeStamp : TMWDEFS_NULL;

  mdnpdata_storeFrozenAnalogInput(pData->pDbHandle, 
    pDNPData->pointNumber, &pDNPData->value, pDNPData->flags, 
    TMWDEFS_TRUE, pTimeStamp);

  return(TMWDEFS_TRUE);
}

/* function: _storeFrozenAnalogInput */
static void TMWDEFS_LOCAL _storeFrozenAnalogInput(
  void *pDbHandle, 
  TMWTYPES_USHORT pointNumber, 
  TMWTYPES_ANALOG_VALUE *pValue,
  TMWTYPES_UCHAR flags,
  TMWDTIME *pTimeStamp)
{
  /* Allocate new binary data structure */
  MDNPO033_DATA *pDNPData = (MDNPO033_DATA *)tmwtarg_alloc(sizeof(MDNPO033_DATA));
  if (pDNPData == TMWDEFS_NULL)
  {
    return;
  }

  /* Initialize data indepdendent parts */
  tmwdb_initData((TMWDB_DATA *)pDNPData, pDbHandle, _dbStoreFunc);

  /* Initialize data dependent parts */
  pDNPData->flags = flags;
  pDNPData->value = *pValue;
  pDNPData->pointNumber = pointNumber;
  pDNPData->timeSpecified = TMWDEFS_FALSE;

  if(pTimeStamp != TMWDEFS_NULL)
  {
    pDNPData->timeSpecified = TMWDEFS_TRUE;
    pDNPData->timeStamp = *pTimeStamp;
  }

  /* Store in database queue */
  if(!tmwdb_addEntry((TMWDB_DATA *)pDNPData))
    tmwtarg_free(pDNPData);
}
#else
/* function: _storeFrozenAnalogInput */
static void TMWDEFS_LOCAL _storeFrozenAnalogInput(
  void *pDbHandle, 
  TMWTYPES_USHORT pointNumber, 
  TMWTYPES_ANALOG_VALUE *pValue,
  TMWTYPES_UCHAR flags,
  TMWDTIME *pTimeStamp)
{
  mdnpdata_storeFrozenAnalogInput(pDbHandle, pointNumber, 
    pValue, flags, TMWDEFS_TRUE, pTimeStamp);
}
#endif /* TMWCNFG_SUPPORT_ASYNCH_DB */
#endif /* MDNPDATA_SUPPORT_OBJ33 */

#if MDNPDATA_SUPPORT_OBJ33_V1
/* function: mdnpo033_readObj33v1 */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpo033_readObj33v1(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{
  MDNPSESN *pMDNPSession = (MDNPSESN *)pSession;
  TMWTYPES_ANALOG_VALUE anlgValue;
  TMWTYPES_USHORT pointNumber;
  TMWTYPES_LONG value;
  TMWTYPES_UCHAR flags;
  TMWTYPES_USHORT index;

  for(index = 0; index < pObjHeader->numberOfPoints; index++)
  {
    dnputil_getPointNumber(pRxFragment, pObjHeader, index, &pointNumber);

    flags = pRxFragment->pMsgBuf[pRxFragment->offset++];

    tmwtarg_get32(&pRxFragment->pMsgBuf[pRxFragment->offset], (TMWTYPES_ULONG *)&value);
    pRxFragment->offset += 4;

    anlgValue.type = TMWTYPES_ANALOG_TYPE_LONG;
    anlgValue.value.lval = value;

    DNPDIAG_SHOW_FROZEN_ANALOG(pSession, pointNumber, &anlgValue, flags, TMWDEFS_TRUE, TMWDEFS_NULL);
    _storeFrozenAnalogInput(pMDNPSession->pDbHandle, pointNumber, &anlgValue, flags, TMWDEFS_NULL);
  }

  return(TMWDEFS_TRUE);
}
#endif /* MDNPDATA_SUPPORT_OBJ33_V1 */

#if MDNPDATA_SUPPORT_OBJ33_V2
/* function: mdnpo033_readObj33v2 */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpo033_readObj33v2(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{
  MDNPSESN *pMDNPSession = (MDNPSESN *)pSession;
  TMWTYPES_ANALOG_VALUE anlgValue;
  TMWTYPES_USHORT pointNumber;
  TMWTYPES_SHORT value;
  TMWTYPES_UCHAR flags;
  TMWTYPES_USHORT index;

  for(index = 0; index < pObjHeader->numberOfPoints; index++)
  {
    dnputil_getPointNumber(pRxFragment, pObjHeader, index, &pointNumber);

    flags = pRxFragment->pMsgBuf[pRxFragment->offset++];

    tmwtarg_get16(&pRxFragment->pMsgBuf[pRxFragment->offset], (TMWTYPES_USHORT *)&value);
    pRxFragment->offset += 2;

    anlgValue.type = TMWTYPES_ANALOG_TYPE_SHORT;
    anlgValue.value.sval = value;

    DNPDIAG_SHOW_FROZEN_ANALOG(pSession, pointNumber, &anlgValue, flags, TMWDEFS_TRUE, TMWDEFS_NULL);
    _storeFrozenAnalogInput(pMDNPSession->pDbHandle, pointNumber, &anlgValue, flags, TMWDEFS_NULL);
  }

  return(TMWDEFS_TRUE);
}
#endif /* MDNPDATA_SUPPORT_OBJ33_V2 */

#if MDNPDATA_SUPPORT_OBJ33_V3
/* function: mdnpo033_readObj33v3 */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpo033_readObj33v3(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{
  MDNPSESN *pMDNPSession = (MDNPSESN *)pSession;
  TMWTYPES_ANALOG_VALUE anlgValue;
  TMWTYPES_MS_SINCE_70 msSince70;
  TMWTYPES_USHORT pointNumber;
  TMWTYPES_LONG value;
  TMWTYPES_UCHAR flags;
  TMWDTIME dateTime;
  TMWTYPES_USHORT index;

  for(index = 0; index < pObjHeader->numberOfPoints; index++)
  {
    dnputil_getPointNumber(pRxFragment, pObjHeader, index, &pointNumber);

    flags = pRxFragment->pMsgBuf[pRxFragment->offset++];

    tmwtarg_get32(&pRxFragment->pMsgBuf[pRxFragment->offset], (TMWTYPES_ULONG *)&value);
    pRxFragment->offset += 4;

    anlgValue.type = TMWTYPES_ANALOG_TYPE_LONG;
    anlgValue.value.lval = value;

    dnpdtime_readMsSince70(&msSince70, &pRxFragment->pMsgBuf[pRxFragment->offset]);  
    dnpdtime_msSince70ToDateTime(&dateTime, &msSince70);
    pRxFragment->offset += 6;

    DNPDIAG_SHOW_FROZEN_ANALOG(pSession, pointNumber, &anlgValue, flags, TMWDEFS_TRUE, &dateTime);
    _storeFrozenAnalogInput(pMDNPSession->pDbHandle, pointNumber, &anlgValue, flags, &dateTime);
  }

  return(TMWDEFS_TRUE);
}
#endif /* MDNPDATA_SUPPORT_OBJ33_V3 */

#if MDNPDATA_SUPPORT_OBJ33_V4
/* function: mdnpo033_readObj33v4 */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpo033_readObj33v4(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{
  MDNPSESN *pMDNPSession = (MDNPSESN *)pSession;
  TMWTYPES_ANALOG_VALUE anlgValue;
  TMWTYPES_MS_SINCE_70 msSince70;
  TMWTYPES_USHORT pointNumber;
  TMWTYPES_SHORT value;
  TMWTYPES_UCHAR flags;
  TMWDTIME dateTime;
  TMWTYPES_USHORT index;

  for(index = 0; index < pObjHeader->numberOfPoints; index++)
  {
    dnputil_getPointNumber(pRxFragment, pObjHeader, index, &pointNumber);

    flags = pRxFragment->pMsgBuf[pRxFragment->offset++];

    tmwtarg_get16(&pRxFragment->pMsgBuf[pRxFragment->offset], (TMWTYPES_USHORT *)&value);
    pRxFragment->offset += 2;

    anlgValue.type = TMWTYPES_ANALOG_TYPE_SHORT;
    anlgValue.value.sval = value;

    dnpdtime_readMsSince70(&msSince70, &pRxFragment->pMsgBuf[pRxFragment->offset]);  
    dnpdtime_msSince70ToDateTime(&dateTime, &msSince70);
    pRxFragment->offset += 6;

    DNPDIAG_SHOW_FROZEN_ANALOG(pSession, pointNumber, &anlgValue, flags, TMWDEFS_TRUE, &dateTime);
    _storeFrozenAnalogInput(pMDNPSession->pDbHandle, pointNumber, &anlgValue, flags, &dateTime);
  }

  return(TMWDEFS_TRUE);
}
#endif /* MDNPDATA_SUPPORT_OBJ33_V4 */

#if MDNPDATA_SUPPORT_OBJ33_V5
/* function: mdnpo033_readObj33v5 */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpo033_readObj33v5(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{
  MDNPSESN *pMDNPSession = (MDNPSESN *)pSession;
  TMWTYPES_ANALOG_VALUE anlgValue;
  TMWTYPES_USHORT pointNumber;
  TMWTYPES_SFLOAT value;
  TMWTYPES_UCHAR flags;
  TMWTYPES_USHORT index;

  for(index = 0; index < pObjHeader->numberOfPoints; index++)
  {
    dnputil_getPointNumber(pRxFragment, pObjHeader, index, &pointNumber);

    flags = pRxFragment->pMsgBuf[pRxFragment->offset++];

    tmwtarg_getSFloat(&pRxFragment->pMsgBuf[pRxFragment->offset], &value);
    pRxFragment->offset += 4;

    anlgValue.type = TMWTYPES_ANALOG_TYPE_SFLOAT;
    anlgValue.value.fval = value;

    DNPDIAG_SHOW_FROZEN_ANALOG(pSession, pointNumber, &anlgValue, flags, TMWDEFS_FALSE, TMWDEFS_NULL);
    _storeFrozenAnalogInput(pMDNPSession->pDbHandle, pointNumber, &anlgValue, flags, TMWDEFS_NULL);
  }

  return(TMWDEFS_TRUE);
}
#endif /* MDNPDATA_SUPPORT_OBJ33_V5 */

#if MDNPDATA_SUPPORT_OBJ33_V6
/* function: mdnpo033_readObj33v6 */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpo033_readObj33v6(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{
  MDNPSESN *pMDNPSession = (MDNPSESN *)pSession;
  TMWTYPES_ANALOG_VALUE anlgValue;
  TMWTYPES_USHORT pointNumber;
  TMWTYPES_DOUBLE value;
  TMWTYPES_UCHAR flags;
  TMWTYPES_USHORT index;

  for(index = 0; index < pObjHeader->numberOfPoints; index++)
  {
    dnputil_getPointNumber(pRxFragment, pObjHeader, index, &pointNumber);

    flags = pRxFragment->pMsgBuf[pRxFragment->offset++];

    tmwtarg_get64(&pRxFragment->pMsgBuf[pRxFragment->offset], &value);
    pRxFragment->offset += 8;

    anlgValue.type = TMWTYPES_ANALOG_TYPE_DOUBLE;
    anlgValue.value.dval = value;

    DNPDIAG_SHOW_FROZEN_ANALOG(pSession, pointNumber, &anlgValue, flags, TMWDEFS_FALSE, TMWDEFS_NULL);
    _storeFrozenAnalogInput(pMDNPSession->pDbHandle, pointNumber, &anlgValue, flags, TMWDEFS_NULL);
  }

  return(TMWDEFS_TRUE);
}
#endif /* MDNPDATA_SUPPORT_OBJ33_V6 */

#if MDNPDATA_SUPPORT_OBJ33_V7
/* function: mdnpo033_readObj33v7 */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpo033_readObj33v7(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{
  MDNPSESN *pMDNPSession = (MDNPSESN *)pSession;
  TMWTYPES_ANALOG_VALUE anlgValue;
  TMWTYPES_MS_SINCE_70 msSince70;
  TMWTYPES_USHORT pointNumber;
  TMWTYPES_SFLOAT value;
  TMWTYPES_UCHAR flags;
  TMWDTIME dateTime;
  TMWTYPES_USHORT index;

  for(index = 0; index < pObjHeader->numberOfPoints; index++)
  {
    dnputil_getPointNumber(pRxFragment, pObjHeader, index, &pointNumber);

    flags = pRxFragment->pMsgBuf[pRxFragment->offset++];

    tmwtarg_getSFloat(&pRxFragment->pMsgBuf[pRxFragment->offset], &value);
    pRxFragment->offset += 4;

    anlgValue.type = TMWTYPES_ANALOG_TYPE_SFLOAT;
    anlgValue.value.fval = value;

    dnpdtime_readMsSince70(&msSince70, &pRxFragment->pMsgBuf[pRxFragment->offset]);  
    dnpdtime_msSince70ToDateTime(&dateTime, &msSince70);
    pRxFragment->offset += 6;

    DNPDIAG_SHOW_FROZEN_ANALOG(pSession, pointNumber, &anlgValue, flags, TMWDEFS_FALSE, &dateTime);
    _storeFrozenAnalogInput(pMDNPSession->pDbHandle, pointNumber, &anlgValue, flags, &dateTime);
  }

  return(TMWDEFS_TRUE);
}
#endif /* MDNPDATA_SUPPORT_OBJ33_V7 */

#if MDNPDATA_SUPPORT_OBJ33_V8
/* function: mdnpo033_readObj33v8 */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpo033_readObj33v8(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{
  MDNPSESN *pMDNPSession = (MDNPSESN *)pSession;
  TMWTYPES_ANALOG_VALUE anlgValue;
  TMWTYPES_MS_SINCE_70 msSince70;
  TMWTYPES_USHORT pointNumber;
  TMWTYPES_DOUBLE value;
  TMWTYPES_UCHAR flags;
  TMWDTIME dateTime;
  TMWTYPES_USHORT index;

  for(index = 0; index < pObjHeader->numberOfPoints; index++)
  {
    dnputil_getPointNumber(pRxFragment, pObjHeader, index, &pointNumber);

    flags = pRxFragment->pMsgBuf[pRxFragment->offset++];

    tmwtarg_get64(&pRxFragment->pMsgBuf[pRxFragment->offset], &value);
    pRxFragment->offset += 8;

    anlgValue.type = TMWTYPES_ANALOG_TYPE_DOUBLE;
    anlgValue.value.dval = value;

    dnpdtime_readMsSince70(&msSince70, &pRxFragment->pMsgBuf[pRxFragment->offset]);  
    dnpdtime_msSince70ToDateTime(&dateTime, &msSince70);
    pRxFragment->offset += 6;

    DNPDIAG_SHOW_FROZEN_ANALOG(pSession, pointNumber, &anlgValue, flags, TMWDEFS_FALSE, &dateTime);
    _storeFrozenAnalogInput(pMDNPSession->pDbHandle, pointNumber, &anlgValue, flags, &dateTime);
  }

  return(TMWDEFS_TRUE);
}
#endif /* MDNPDATA_SUPPORT_OBJ33_V8 */

#if MDNPDATA_SUPPORT_OBJ33
/* function: mdnpo033_readObj33 */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpo033_readObj33(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{
  TMWTYPES_BOOL status = TMWDEFS_FALSE;
  switch(pObjHeader->variation)
  {
#if MDNPDATA_SUPPORT_OBJ33_V1
    case 1:
      status = mdnpo033_readObj33v1(pSession, pRxFragment, pObjHeader);
      break;
#endif
#if MDNPDATA_SUPPORT_OBJ33_V2
    case 2:
      status = mdnpo033_readObj33v2(pSession, pRxFragment, pObjHeader);
      break;
#endif
#if MDNPDATA_SUPPORT_OBJ33_V3
    case 3:
      status = mdnpo033_readObj33v3(pSession, pRxFragment, pObjHeader);
      break;
#endif
#if MDNPDATA_SUPPORT_OBJ33_V4
    case 4:
      status = mdnpo033_readObj33v4(pSession, pRxFragment, pObjHeader);
      break;
#endif
#if MDNPDATA_SUPPORT_OBJ33_V5
    case 5:
      status = mdnpo033_readObj33v5(pSession, pRxFragment, pObjHeader);
      break;
#endif
#if MDNPDATA_SUPPORT_OBJ33_V6
    case 6:
      status = mdnpo033_readObj33v6(pSession, pRxFragment, pObjHeader);
      break;
#endif
#if MDNPDATA_SUPPORT_OBJ33_V7
    case 7:
      status = mdnpo033_readObj33v7(pSession, pRxFragment, pObjHeader);
      break;
#endif
#if MDNPDATA_SUPPORT_OBJ33_V8
    case 8:
      status = mdnpo033_readObj33v8(pSession, pRxFragment, pObjHeader);
      break;
#endif
  }
  return(status);
}
#endif

