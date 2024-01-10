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

/* file: dnputil.h
 * description: This file is intended for internal SCL use only.
 *   Miscellaneous DNP utilities.
 */
#ifndef DNPUTIL_DEFINED
#define DNPUTIL_DEFINED

#include "tmwscl/utils/tmwcnfg.h"
#include "tmwscl/dnp/dnpsesn.h"
#include "tmwscl/dnp/dnpdata.h"

/* Structure to hold info about received message */
typedef struct DNPUtilRxMsgStruct {
  TMWTYPES_UCHAR *pMsgBuf;
  TMWTYPES_ULONG  offset;
  TMWTYPES_USHORT msgLength;
  TMWTYPES_UCHAR ac;
  TMWTYPES_UCHAR fc;
  TMWTYPES_USHORT iin;
  TMWTYPES_BOOL isBroadcast;
  TMWTYPES_USHORT rxAddress;
  TMWTYPES_MILLISECONDS firstByteTime;
  TMWTYPES_MILLISECONDS lastByteTime;
} DNPUTIL_RX_MSG;

/* Structure to hold data from object header */
typedef struct DNPUtilObjectHeaderStruct {
  TMWTYPES_UCHAR group;
  TMWTYPES_UCHAR variation;
  TMWTYPES_UCHAR qualifier;
  TMWTYPES_USHORT firstPointNumber;
  TMWTYPES_USHORT lastPointNumber;
  TMWTYPES_USHORT numberOfPoints;
  TMWTYPES_UCHAR *pObjectHeader;
  TMWTYPES_UCHAR *pObjectData;
} DNPUTIL_OBJECT_HEADER;

#ifdef __cplusplus
extern "C" {
#endif

  /* function: dnputil_getCurrentMessage */
  TMWSESN_TX_DATA * TMWDEFS_GLOBAL dnputil_getCurrentMessage(
    TMWSESN *pSession);

  /* function: dnputil_setCurrentMessage */
  void TMWDEFS_GLOBAL dnputil_setCurrentMessage(
    TMWSESN *pSession, TMWSESN_TX_DATA *pMsg);

  /* function: dnputil_linkFrameSizeToTprt */
  TMWTYPES_USHORT TMWDEFS_GLOBAL dnputil_linkFrameSizeToTprt(
    TMWTYPES_USHORT linkFrameSize);

  /* function: dnputil_validateMsgLen */
  TMWTYPES_BOOL TMWDEFS_GLOBAL dnputil_validateMsgLen(
    TMWSESN *pSession, 
    DNPUTIL_RX_MSG *pRxFragment, 
    TMWTYPES_USHORT length);

  /* function: dnputil_parsApplHeader */
  TMWTYPES_BOOL TMWDEFS_GLOBAL dnputil_parseApplHeader(
    TMWSESN_RX_DATA *pRxFragment,
    DNPUTIL_RX_MSG *pMsg,
    TMWTYPES_BOOL copyMsgBuf);

  /* function: dnputil_parseObjectHeader */
  TMWTYPES_BOOL TMWDEFS_GLOBAL dnputil_parseObjectHeader(
    DNPUTIL_RX_MSG *pMsg,
    TMWTYPES_USHORT objectIndex, 
    DNPUTIL_OBJECT_HEADER *pHdr);

  /* function: dnputil_getPointNumber */
  TMWTYPES_BOOL TMWDEFS_GLOBAL dnputil_getPointNumber(
    DNPUTIL_RX_MSG *pRxFragment,
    DNPUTIL_OBJECT_HEADER *pHdr, 
    TMWTYPES_USHORT index, 
    TMWTYPES_USHORT *pPointNumber);

  /* function: dnputil_writePointNumber */
  void TMWDEFS_GLOBAL dnputil_writePointNumber(
    TMWSESN_TX_DATA *pResponse,
    TMWTYPES_USHORT objectHeaderOffset,
    TMWTYPES_USHORT pointNumber);

  /* function: dnputil_getAnalogValueShort */
  TMWTYPES_SHORT dnputil_getAnalogValueShort(
    TMWTYPES_ANALOG_VALUE *pValue,
    TMWTYPES_UCHAR *pFlags);

  /* function: dnputil_getAnalogValueLong */
  TMWTYPES_LONG dnputil_getAnalogValueLong(
    TMWTYPES_ANALOG_VALUE *pValue,
    TMWTYPES_UCHAR *pFlags);

#if TMWCNFG_SUPPORT_FLOAT
  /* function: dnputil_getAnalogValueFloat */
  TMWTYPES_SFLOAT dnputil_getAnalogValueFloat(
    TMWTYPES_ANALOG_VALUE *pValue,
    TMWTYPES_UCHAR *pFlags);
#endif

#if TMWCNFG_SUPPORT_DOUBLE
  /* function: dnputil_getAnalogValueDouble */
  TMWDEFS_SCL_API TMWTYPES_DOUBLE dnputil_getAnalogValueDouble(
    TMWTYPES_ANALOG_VALUE *pValue);
#endif

/* function: dnputil_getAnlgDBandValueUShort */
TMWTYPES_USHORT dnputil_getAnlgDBandValueUShort(
  TMWTYPES_ANALOG_VALUE *pValue);

/* function: dnputil_getAnlgDBandValueULong */
TMWTYPES_ULONG dnputil_getAnlgDBandValueULong(
  TMWTYPES_ANALOG_VALUE *pValue);

#if TMWCNFG_SUPPORT_FLOAT
/* function: dnputil_getAnlgDBandValueFloat */
TMWTYPES_SFLOAT dnputil_getAnlgDBandValueFloat(
  TMWTYPES_ANALOG_VALUE *pValue);
#endif

#if TMWCNFG_SUPPORT_DOUBLE
/* function: dnputil_getAnlgDBandValueDouble */
TMWTYPES_DOUBLE dnputil_getAnlgDBandValueDouble(
  TMWTYPES_ANALOG_VALUE *pValue);
#endif

TMWTYPES_UCHAR TMWDEFS_GLOBAL dnputil_lengthRequired(
  TMWTYPES_ULONG value);

void TMWDEFS_GLOBAL dnputil_putIntInMessage(
  TMWTYPES_UCHAR *ptr, 
  TMWTYPES_ULONG value, 
  TMWTYPES_UCHAR length);

TMWTYPES_LONG TMWDEFS_GLOBAL dnputil_getIntFromMessage(
  TMWTYPES_UCHAR *ptr,
  TMWTYPES_UCHAR length);

TMWTYPES_ULONG TMWDEFS_GLOBAL dnputil_getUintFromMessage(
  TMWTYPES_UCHAR *ptr,
  TMWTYPES_UCHAR length);

void TMWDEFS_GLOBAL dnputil_putValueInMessage(
  TMWTYPES_UCHAR         *pMsgData,
  TMWTYPES_USHORT        *pMsgLength,
  DNPDATA_DATASET_VALUE  *pValue);

TMWTYPES_BOOL TMWDEFS_GLOBAL dnputil_getValueFromMessage(
  TMWTYPES_UCHAR            *pMsgBuf,
  TMWTYPES_ULONG            *pOffset,
  TMWTYPES_ULONG             maxOffset,
  DNPDEFS_DATASET_TYPE_CODE  type,
  DNPDATA_DATASET_VALUE     *pValue);

void TMWDEFS_GLOBAL dnputil_getAncValueFromMessage(
  DNPUTIL_RX_MSG            *pRxFragment,
  DNPDEFS_DATASET_DESCR_CODE code,
  TMWTYPES_UCHAR             length,
  DNPDATA_DATASET_VALUE     *pValue);

void TMWDEFS_GLOBAL dnputil_putAttrValueInMessage(
  TMWSESN_TX_DATA         *pTxData,
  DNPDATA_ATTRIBUTE_VALUE *pValue);
 
TMWTYPES_BOOL TMWDEFS_GLOBAL dnputil_getAttrValueFromMessage(
  DNPUTIL_RX_MSG            *pRxFragment,
  DNPDATA_ATTRIBUTE_VALUE    *pValue);

#ifdef __cplusplus
}
#endif
#endif /* SDNPUTIL_DEFINED */
