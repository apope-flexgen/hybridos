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

/* file: mdnpo003.c
 * description: DNP Master functionality for Object 3 Double Bit Inputs
 */
#include "tmwscl/dnp/dnpdiag.h"
#include "tmwscl/dnp/mdnpo003.h"
#include "tmwscl/dnp/mdnpdata.h"

#include "tmwscl/utils/tmwdb.h"
#include "tmwscl/utils/tmwtarg.h"

#if MDNPDATA_SUPPORT_OBJ3
#if TMWCNFG_SUPPORT_ASYNCH_DB

/* Structure used to store data for asynch database updates */
typedef struct MDNPO003DataStruct {
  TMWDB_DATA tmw;
  TMWTYPES_UCHAR flags;
  TMWTYPES_USHORT pointNumber;
} MDNPO003_DATA;

/* function: _dbStoreFunc */
static TMWTYPES_BOOL TMWDEFS_LOCAL _dbStoreFunc(
  TMWDB_DATA *pData)
{
  MDNPO003_DATA *pDNPData = (MDNPO003_DATA *)pData;

  mdnpdata_storeDoubleInput(pData->pDbHandle, 
    pDNPData->pointNumber, pDNPData->flags, TMWDEFS_FALSE, TMWDEFS_NULL);

  return(TMWDEFS_TRUE);
}

/* function: _storeDoubleInput */
static void TMWDEFS_LOCAL _storeDoubleInput(
  void *pDbHandle, 
  TMWTYPES_USHORT pointNumber, 
  TMWTYPES_UCHAR flags)
{
  /* Allocate new double bit data structure */
  MDNPO003_DATA *pDNPData = (MDNPO003_DATA *)tmwtarg_alloc(sizeof(MDNPO003_DATA));
  if (pDNPData == TMWDEFS_NULL)
  {
    return;
  }

  /* Initialize data indepdendent parts */
  tmwdb_initData((TMWDB_DATA *)pDNPData, pDbHandle, _dbStoreFunc);

  /* Initialize data dependent parts */
  pDNPData->flags = flags;
  pDNPData->pointNumber = pointNumber;

  /* Store in database queue */
  if(!tmwdb_addEntry((TMWDB_DATA *)pDNPData))
    tmwtarg_free(pDNPData);
}
#else
/* function: _storeDoubleInput */
static void TMWDEFS_LOCAL _storeDoubleInput(
  void *pDbHandle, 
  TMWTYPES_USHORT pointNumber, 
  TMWTYPES_UCHAR flags)
{
  mdnpdata_storeDoubleInput(pDbHandle, pointNumber, 
    flags, TMWDEFS_FALSE, TMWDEFS_NULL);
}
#endif /* TMWCNFG_SUPPORT_ASYNCH_DB */
#endif /* MDNPDATA_SUPPORT_OBJ3 */

#if MDNPDATA_SUPPORT_OBJ3_V1
/* function: mdnpo003_readObj3v1 */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpo003_readObj3v1(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  DNPUTIL_OBJECT_HEADER *pObjHeader)
{
  MDNPSESN *pMDNPSession = (MDNPSESN *)pSession;
  TMWTYPES_USHORT pointNumber;
  TMWTYPES_USHORT index;
  TMWTYPES_UCHAR shift;
  TMWTYPES_UCHAR value;

  index = 0;
  shift = 0;
  while(index < pObjHeader->numberOfPoints)
  {
    dnputil_getPointNumber(pRxFragment, pObjHeader, index, &pointNumber);

    /* Protect against badly formatted message */
    if(pRxFragment->offset >= pRxFragment->msgLength)
    {
      DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_INVALID_SIZE);
      return(TMWDEFS_FALSE);
    }

    /* variation contains 2 bits for each point value, flags are nominal */
    value = (pRxFragment->pMsgBuf[pRxFragment->offset] & (0x03 << shift));
    value = (value << (6 - shift)) | DNPDEFS_DBAS_FLAG_ON_LINE;
    DNPDIAG_SHOW_DOUBLE_INPUT(pSession, pointNumber, value, TMWDEFS_FALSE, TMWDEFS_NULL);

    _storeDoubleInput(pMDNPSession->pDbHandle, pointNumber, value);

    index = (TMWTYPES_USHORT)(index + 1);

    if((pObjHeader->qualifier == DNPDEFS_QUAL_8BIT_INDEX)
      || (pObjHeader->qualifier == DNPDEFS_QUAL_16BIT_INDEX))
    {
      pRxFragment->offset += 1;
    }
    else
    {
      shift = (TMWTYPES_UCHAR)(shift + 2);
      if(shift > 6)
      {
        shift = 0;
        pRxFragment->offset += 1;
      }
    }
  }

  if((pObjHeader->qualifier != DNPDEFS_QUAL_8BIT_INDEX)
    && (pObjHeader->qualifier != DNPDEFS_QUAL_16BIT_INDEX))
  {
    if(shift != 0)
      pRxFragment->offset += 1;
  }

  return(TMWDEFS_TRUE);
}
#endif /* MDNPDATA_SUPPORT_OBJ3_V1 */

#if MDNPDATA_SUPPORT_OBJ3_V2
/* function: mdnpo003_readObj3v2 */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpo003_readObj3v2(
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

    value = pRxFragment->pMsgBuf[pRxFragment->offset++];

    DNPDIAG_SHOW_DOUBLE_INPUT(pSession, pointNumber, value, TMWDEFS_FALSE, TMWDEFS_NULL);

    _storeDoubleInput(pMDNPSession->pDbHandle, pointNumber, value);
  }

  return(TMWDEFS_TRUE);
}
#endif /* MDNPDATA_SUPPORT_OBJ3_V2 */
