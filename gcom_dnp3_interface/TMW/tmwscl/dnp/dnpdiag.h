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

/* file: dnpdiag.h
 * description: Generic DNP diagnostics
 */
#ifndef DNPDIAG_DEFINED
#define DNPDIAG_DEFINED

#include "tmwscl/utils/tmwcnfg.h"
#include "tmwscl/utils/tmwtarg.h"
#include "tmwscl/utils/tmwchnl.h"
#include "tmwscl/utils/tmwsesn.h"
#include "tmwscl/utils/tmwdtime.h"

#include "tmwscl/dnp/dnpsesn.h"
#include "tmwscl/dnp/dnpdata.h"
#include "tmwscl/dnp/dnputil.h"
#include "tmwscl/dnp/dnpauth.h"

#define MAX_FRAME_SIZE (292 + 5)

/* This structure is used to hold all of the blocks of a received 
 *  frame so they can be displayed at once. This is require since 
 *  the link layer code throws away the physical layer blocks once 
 *  they have been parsed.
 */
typedef struct DNPLinkFrame {
  TMWCHNL *pChannel;
  TMWSESN *pSession;
  TMWTYPES_USHORT size;
  TMWTYPES_UCHAR buffer[MAX_FRAME_SIZE];
} DNPLINK_FRAME;

/* Structure to be used in error tables */
typedef struct DNPDiagErrorEntry {
  TMWTYPES_USHORT errorNumber;
  const TMWTYPES_CHAR  *pErrorMsg;
} DNPDIAG_ERROR_ENTRY;

/* Define error numbers used by both Master and Slave DNP */
typedef enum {
  DNPDIAG_INV_LENGTH,
  DNPDIAG_FIRST_LAST,
  DNPDIAG_INVALID_SIZE,
  DNPDIAG_INCR_TIMEOUT,
  DNPDIAG_INCR_TIMEOUT_REQ,
  DNPDIAG_RESP_TIMEOUT,
  DNPDIAG_CHANNEL_TIMEOUT,
  DNPDIAG_PROC_FRAGMENT,
  DNPDIAG_BAD_START, 
  DNPDIAG_UNSUP_FC,  
  DNPDIAG_UNSUP_FC_RCVD,
  DNPDIAG_CRC_HEADER,
  DNPDIAG_CRC_DATA,
  DNPDIAG_READ_MORE,
  DNPDIAG_FRAME_OVERRUN,
  DNPDIAG_TRANSMIT_FAILED,
  DNPDIAG_TOO_MANY_REQUESTS, 
  DNPDIAG_DUPLICATE_REQ, 
  DNPDIAG_REQUEST_CANCELED, 
  DNPDIAG_MASTER_SESSION,
  DNPDIAG_ALLOC_TX,        
  DNPDIAG_ALLOC_FRAG,    
  DNPDIAG_INVALID_BUF_SIZE,
  DNPDIAG_INACTIVE,    
  DNPDIAG_WAITING,
  DNPDIAG_SECONDARY_BRDCST, 
  DNPDIAG_INVALID_FRSIZE, 
  DNPDIAG_LINK_NOT_RESET, 
  DNPDIAG_INVALID_FCV,
  DNPDIAG_INVALID_FCB, 
  DNPDIAG_BRDCST_DISABLED,
  DNPDIAG_FC_DISABLED,
  DNPDIAG_TEST_LINK_BRDCST,
  DNPDIAG_LINK_RESET_BRDCST,
  DNPDIAG_LINK_STAT_BRDCST, 
  DNPDIAG_UNRECOG_BRDCST,
  DNPDIAG_RCV_TIMEOUT, 
  DNPDIAG_NEEDED_BYTES, 
  DNPDIAG_PARSE_APPHEADER,
  DNPDIAG_PARSE_HEADER,
  DNPDIAG_FRAG_RESTART,
  DNPDIAG_FRAME_SESSION,
  DNPDIAG_TPRT_SEQ_INVALID,
  DNPDIAG_TPRT_SEGDISCARD,
  DNPDIAG_TPRT_DISCARD,
  DNPDIAG_FRAG_SIZE,
  DNPDIAG_FRAME_SIZE,
  DNPDIAG_TX_BUFFER,
  DNPDIAG_RX_BUFFER,
  DNPDIAG_QUANTITY_0,
  DNPDIAG_QUALIFIER,
  DNPDIAG_INACTIVE_SESSION,
  DNPDIAG_READ_PROTO_FAIL,
  DNPDIAG_DATASET_VALUE,
  DNPDIAG_DATASET_PROTO,
  DNPDIAG_DATASET_DESCR,
  DNPDIAG_DATASET_ELEMS,
  DNPDIAG_DATASET_CTRLS,
  DNPDIAG_DATASET_TYPE,
  DNPDIAG_DATASET_MANDATORY,
  DNPDIAG_DATASET_FAIL,
  DNPDIAG_ATTRIBUTE_VALUE,
  DNPDIAG_AUTH_RXBADMSG,
  DNPDIAG_AUTH_RXBADSEQ,
  DNPDIAG_AUTH_BADUSER,
  DNPDIAG_AUTH_APPLTIMEOUT,
  DNPDIAG_AUTH_CHALLENGERPLY,
  DNPDIAG_AUTH_KEYNOTOK,
  DNPDIAG_AUTH_AGGRNOCHALL,
  DNPDIAG_AUTH_AGGRRXNOV9,
  DNPDIAG_AUTH_AGGRRXFAIL,
  DNPDIAG_AUTH_AGGRTXFAIL,
  DNPDIAG_AUTH_MACFAIL,
  DNPDIAG_AUTH_MACSUPPORT,
  DNPDIAG_AUTH_MACDEFAULT,  
  DNPDIAG_AUTH_KEYWRAPDEFAULT,
  DNPDIAG_AUTH_FAIL,
  DNPDIAG_AUTH_TOO_BIG,
  DNPDIAG_AUTH_FINDUSERNAME,
  DNPDIAG_AUTH_USERNAME,
  DNPDIAG_AUTH_GETKEYFAIL, 
  DNPDIAG_AUTH_BADCHALLENGE,
  DNPDIAG_AUTH_BADCLENGTH,
  DNPDIAG_AUTH_BADMAC,
  DNPDIAG_AUTH_DECRYPT,
  DNPDIAG_AUTH_UPDATE_FAILED,
  DNPDIAG_AUTH_UNKNOWN_USR,
  DNPDIAG_AUTH_FORCEDERROR,

  /* This must be last entry */
  DNPDIAG_ERROR_ENUM_MAX

} DNPDIAG_ERROR_ENUM; 


/* Type of data being displayed by DNPDIAG_AUTH_SHOW_DATA */
#define dataTypeUserPublicKey 1
#define dataTypeCertData 2
#define dataTypeKeyData 3
#define dataTypePlainData 4
#define dataTypeSessKeyData 5
#define dataTypeRcvdChallData 6
#define dataTypeGenChallData 7
#define dataTypeRcvdMAC 8
#define dataTypeGenMAC 9

#if !TMWCNFG_SUPPORT_DIAG

#define DNPDIAG_BUILD_MESSAGE(pChannel, pSession, description) \
  TMWTARG_UNUSED_PARAM(pChannel); TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(description);

#define DNPDIAG_INSERT_QUEUE(pChannel, pSession, description) \
  TMWTARG_UNUSED_PARAM(pChannel); TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(description);

#define DNPDIAG_SHOW_OBJECT_HEADER(pSession, pHdr) \
  TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(pHdr);

#define DNPDIAG_SHOW_TX_OBJECT_HDR(pSession, group, variation, qualifier) \
  TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(group); TMWTARG_UNUSED_PARAM(variation); TMWTARG_UNUSED_PARAM(qualifier);

#define DNPDIAG_SHOW_IIN_VALUE(pSession, pointNumber, value) \
  TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(pointNumber); TMWTARG_UNUSED_PARAM(value);

#define DNPDIAG_SHOW_BINARY_INPUT(pSession, point, flags, isEvent, pTimeStamp) \
  TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(point); TMWTARG_UNUSED_PARAM(flags); TMWTARG_UNUSED_PARAM(isEvent); \
  TMWTARG_UNUSED_PARAM(pTimeStamp);

#define DNPDIAG_SHOW_DOUBLE_INPUT(pSession, point, flags, isEvent, pTimeStamp) \
  TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(point); TMWTARG_UNUSED_PARAM(flags); TMWTARG_UNUSED_PARAM(isEvent); \
  TMWTARG_UNUSED_PARAM(pTimeStamp);

#define DNPDIAG_SHOW_CROB(pSession, point, control, count, onTime, offTime, status, direction) \
  TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(point); TMWTARG_UNUSED_PARAM(control); TMWTARG_UNUSED_PARAM(count); \
  TMWTARG_UNUSED_PARAM(onTime); TMWTARG_UNUSED_PARAM(offTime); \
  TMWTARG_UNUSED_PARAM(status); TMWTARG_UNUSED_PARAM(direction);

#define DNPDIAG_SHOW_BINARY_OUTPUT(pSession, point, flags, isEvent, pTimeStamp) \
  TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(point); TMWTARG_UNUSED_PARAM(flags); TMWTARG_UNUSED_PARAM(isEvent); \
  TMWTARG_UNUSED_PARAM(pTimeStamp);

#define DNPDIAG_SHOW_BIN_OUT_WRITE(pSession, point, state) \
  TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(point); TMWTARG_UNUSED_PARAM(state); 

#define DNPDIAG_SHOW_BINARY_CMD_STATUS(pSession, point, status, pTimeStamp) \
  TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(point); TMWTARG_UNUSED_PARAM(status); TMWTARG_UNUSED_PARAM(pTimeStamp); 

#define DNPDIAG_SHOW_BINARY_COUNTER(pSession, point, value, flags, isEvent, pTimeStamp) \
  TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(point); TMWTARG_UNUSED_PARAM(value); TMWTARG_UNUSED_PARAM(flags); \
  TMWTARG_UNUSED_PARAM(isEvent); TMWTARG_UNUSED_PARAM(pTimeStamp);

#define DNPDIAG_SHOW_FROZEN_COUNTER(pSession, point, value, flags, isEvent, pTimeStamp) \
  TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(point); TMWTARG_UNUSED_PARAM(value); TMWTARG_UNUSED_PARAM(flags); \
  TMWTARG_UNUSED_PARAM(isEvent); TMWTARG_UNUSED_PARAM(pTimeStamp);

#define DNPDIAG_SHOW_ANALOG_INPUT(pSession, point, value, flags, isEvent, pTimeStamp) \
  TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(point); TMWTARG_UNUSED_PARAM(value); TMWTARG_UNUSED_PARAM(flags); \
  TMWTARG_UNUSED_PARAM(isEvent); TMWTARG_UNUSED_PARAM(pTimeStamp);

#define DNPDIAG_SHOW_FROZEN_ANALOG(pSession, point, value, flags, isEvent, pTimeStamp) \
  TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(point); TMWTARG_UNUSED_PARAM(value); TMWTARG_UNUSED_PARAM(flags); \
  TMWTARG_UNUSED_PARAM(isEvent); TMWTARG_UNUSED_PARAM(pTimeStamp);

#define DNPDIAG_SHOW_ANALOG_OUTPUT(pSession, point, value, flags, isEvent, pTimeStamp) \
  TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(point); TMWTARG_UNUSED_PARAM(value); TMWTARG_UNUSED_PARAM(flags); \
  TMWTARG_UNUSED_PARAM(isEvent); TMWTARG_UNUSED_PARAM(pTimeStamp);

#define DNPDIAG_SHOW_ANALOG_CONTROL(pSession, point, value, status, isEvent, direction, pTimeStamp) \
  TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(point); TMWTARG_UNUSED_PARAM(value); TMWTARG_UNUSED_PARAM(status); \
  TMWTARG_UNUSED_PARAM(isEvent); TMWTARG_UNUSED_PARAM(direction); TMWTARG_UNUSED_PARAM(pTimeStamp);

#define DNPDIAG_SHOW_ANALOG_DEADBAND(pSession, point, value, direction) \
  TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(point); TMWTARG_UNUSED_PARAM(value);

#define DNPDIAG_SHOW_TIME_AND_DATE(pSession, pDateTime, pText) \
  TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(pDateTime); TMWTARG_UNUSED_PARAM(pText);

#define DNPDIAG_SHOW_TIME_AND_DATE_SENT(pSession, pDateTime) \
  TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(pDateTime);

#define DNPDIAG_SHOW_INDEXED_TIME(pSession, point, ptimeValue, pIntervalCount, pIntervalUnits, direction) \
  TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(point); TMWTARG_UNUSED_PARAM(ptimeValue); \
    TMWTARG_UNUSED_PARAM(pIntervalCount); TMWTARG_UNUSED_PARAM(pIntervalUnits); TMWTARG_UNUSED_PARAM(direction);

#define DNPDIAG_SHOW_FREEZE(pSession, pDateTime, freezeInterval, direction) \
  TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(pDateTime); TMWTARG_UNUSED_PARAM(freezeInterval); TMWTARG_UNUSED_PARAM(direction);

#define DNPDIAG_SHOW_STRING_DATA(pSession, point, pBuf, bufLength, direction) \
  TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(point); TMWTARG_UNUSED_PARAM(pBuf); \
  TMWTARG_UNUSED_PARAM(bufLength); TMWTARG_UNUSED_PARAM(direction);

#define DNPDIAG_SHOW_STRING_EVENT(pSession, point, pBuf, bufLength) \
  TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(point); TMWTARG_UNUSED_PARAM(pBuf); \
  TMWTARG_UNUSED_PARAM(bufLength);

#define DNPDIAG_SHOW_VTERM_OUTPUT(pSession, point, pBuf, bufLength) \
  TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(point); TMWTARG_UNUSED_PARAM(pBuf); \
  TMWTARG_UNUSED_PARAM(bufLength);

#define DNPDIAG_SHOW_VTERM_EVENT(pSession, point, pBuf, bufLength) \
  TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(point); TMWTARG_UNUSED_PARAM(pBuf); \
  TMWTARG_UNUSED_PARAM(bufLength);

#define DNPDIAG_SHOW_EXT_STRING_DATA(pSession, point, pBuf, bufLength, flags, direction) \
  TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(point); TMWTARG_UNUSED_PARAM(pBuf); \
  TMWTARG_UNUSED_PARAM(bufLength); TMWTARG_UNUSED_PARAM(flags); TMWTARG_UNUSED_PARAM(direction);

#define DNPDIAG_SHOW_EXT_STRING_EVENT(pSession, point, pBuf, bufLength, flags, pTimeStamp) \
  TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(point); TMWTARG_UNUSED_PARAM(pBuf); \
  TMWTARG_UNUSED_PARAM(bufLength); TMWTARG_UNUSED_PARAM(flags); TMWTARG_UNUSED_PARAM(pTimeStamp);

#define DNPDIAG_SHOW_AUTH_SECURITY_STAT(pSession, association, point, value, flags, isEvent, pTimeStamp) \
  TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(association); TMWTARG_UNUSED_PARAM(point); TMWTARG_UNUSED_PARAM(value); \
  TMWTARG_UNUSED_PARAM(flags); TMWTARG_UNUSED_PARAM(isEvent); TMWTARG_UNUSED_PARAM(pTimeStamp);

#define DNPDIAG_SHOW_ACTIVATE_CONFIG(pSession, timeDelay, statusCode, pBuf, bufLength) \
  TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(timeDelay); TMWTARG_UNUSED_PARAM(statusCode); \
  TMWTARG_UNUSED_PARAM(pBuf); TMWTARG_UNUSED_PARAM(bufLength);

#define DNPDIAG_FRAGMENT_SENT(pChannel, pSession, pFragment, numBytes) \
  TMWTARG_UNUSED_PARAM(pChannel); TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(pFragment); \
  TMWTARG_UNUSED_PARAM(numBytes);

#define DNPDIAG_FRAGMENT_RECEIVED(pChannel, pSession, pFragment, numBytes) \
  TMWTARG_UNUSED_PARAM(pChannel); TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(pFragment); TMWTARG_UNUSED_PARAM(numBytes);

#define DNPDIAG_TPRT_FRAME_SENT(pChannel, pSession, pFrame, numBytes) \
  TMWTARG_UNUSED_PARAM(pChannel); TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(pFrame); TMWTARG_UNUSED_PARAM(numBytes);

#define DNPDIAG_TPRT_FRAME_RECEIVED(pChannel, pSession, pFrame, numBytes) \
  TMWTARG_UNUSED_PARAM(pChannel); TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(pFrame); TMWTARG_UNUSED_PARAM(numBytes);

#define DNPDIAG_LINK_FRAME_SENT(pChannel, pSession, pFrame, numBytes, retryCount) \
  TMWTARG_UNUSED_PARAM(pChannel); TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(pFrame); TMWTARG_UNUSED_PARAM(numBytes); TMWTARG_UNUSED_PARAM(retryCount);

#define DNPDIAG_LINK_HEADER_BLOCK_RECEIVED(pChannel, pSession, pBuf, numBytes) \
  TMWTARG_UNUSED_PARAM(pChannel); TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(pBuf); TMWTARG_UNUSED_PARAM(numBytes);

#define DNPDIAG_LINK_DATA_BLOCK_RECEIVED(pContext, pBuf, numBytes) \
  TMWTARG_UNUSED_PARAM(pContext); TMWTARG_UNUSED_PARAM(pBuf); TMWTARG_UNUSED_PARAM(numBytes);

#define DNPDIAG_LINK_FRAME_COMPLETE(pContext, discarded) \
  TMWTARG_UNUSED_PARAM(pContext); TMWTARG_UNUSED_PARAM(discarded);

#define DNPDIAG_LINK_FRAME_RECEIVED(pChannel, pSession, pFrame, numBytes) \
  TMWTARG_UNUSED_PARAM(pChannel); TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(pFrame); TMWTARG_UNUSED_PARAM(numBytes);

#define DNPDIAG_SHOW_DATASET_PROTO(pSession, point, elemIndex, pDescr, direction) \
  TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(point); TMWTARG_UNUSED_PARAM(elemIndex); TMWTARG_UNUSED_PARAM(pDescr); \
  TMWTARG_UNUSED_PARAM(direction);

#define DNPDIAG_SHOW_DATASET_DSCR_CONT(pSession, point, elemIndex, pDescr, direction) \
  TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(point); TMWTARG_UNUSED_PARAM(elemIndex); TMWTARG_UNUSED_PARAM(pDescr);\
  TMWTARG_UNUSED_PARAM(direction);

#define DNPDIAG_SHOW_DATASET_DSCR_CHRS(pSession, point, value, direction)\
  TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(point); TMWTARG_UNUSED_PARAM(value); TMWTARG_UNUSED_PARAM(direction);

#define DNPDIAG_SHOW_DATASET_DSCR_INDX(pSession, point, elemIndex, pDescr, direction) \
  TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(point); TMWTARG_UNUSED_PARAM(elemIndex); TMWTARG_UNUSED_PARAM(pDescr); \
  TMWTARG_UNUSED_PARAM(direction);

#define DNPDIAG_SHOW_DATASET_TIME(pSession, point, pTimeStamp, isEvent, direction) \
  TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(point); TMWTARG_UNUSED_PARAM(pTimeStamp); TMWTARG_UNUSED_PARAM(isEvent); \
  TMWTARG_UNUSED_PARAM(direction);

#define DNPDIAG_SHOW_DATASET(pSession, elemIndex, pElem, fc, isIgnored, isEvent, direction) \
  TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(elemIndex); TMWTARG_UNUSED_PARAM(pElem); TMWTARG_UNUSED_PARAM(fc); \
  TMWTARG_UNUSED_PARAM(isIgnored); TMWTARG_UNUSED_PARAM(isEvent); TMWTARG_UNUSED_PARAM(direction);

#define DNPDIAG_SHOW_DEVICE_ATTRIBUTE(pSession, point, variation, pValue, direction) \
  TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(point); TMWTARG_UNUSED_PARAM(variation); TMWTARG_UNUSED_PARAM(pValue); \
  TMWTARG_UNUSED_PARAM(direction);

#define DNPDIAG_SHOW_DEVICE_ATTR_PROP(pSession, point, variation, property) \
  TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(point); TMWTARG_UNUSED_PARAM(variation); TMWTARG_UNUSED_PARAM(property);  

#define DNPDIAG_SHOW_FILE_OPEN(pSession, filename, fileNameLength, fileSize, pTimeOfCreation, permissions, authKey, mode, maxBlockSize, requestId)\
  TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(filename); TMWTARG_UNUSED_PARAM(fileNameLength); TMWTARG_UNUSED_PARAM(fileSize); \
  TMWTARG_UNUSED_PARAM(pTimeOfCreation);TMWTARG_UNUSED_PARAM(permissions); TMWTARG_UNUSED_PARAM(authKey); TMWTARG_UNUSED_PARAM(mode); \
  TMWTARG_UNUSED_PARAM(maxBlockSize); TMWTARG_UNUSED_PARAM(requestId);

#define DNPDIAG_SHOW_FILE_CLOSE(pSession, handle, requestId)\
  TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(handle); TMWTARG_UNUSED_PARAM(requestId);

#define DNPDIAG_SHOW_FILE_DATA(pSession, handle, block, numBytes, last, isEvent, direction)\
  TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(handle); TMWTARG_UNUSED_PARAM(block); TMWTARG_UNUSED_PARAM(numBytes); \
  TMWTARG_UNUSED_PARAM(last); TMWTARG_UNUSED_PARAM(isEvent); TMWTARG_UNUSED_PARAM(direction);
 
#define DNPDIAG_SHOW_FILE_STRING(pSession, point, pBuf, bufLength, direction) \
  TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(point); TMWTARG_UNUSED_PARAM(pBuf); \
  TMWTARG_UNUSED_PARAM(bufLength); TMWTARG_UNUSED_PARAM(direction);

#define DNPDIAG_SHOW_FILE_STATUS(pSession, handle, fileSize, maxBlockSize, requestId, status, nOptionalChars, pOptionalChars) \
  TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(handle); TMWTARG_UNUSED_PARAM(fileSize); TMWTARG_UNUSED_PARAM(maxBlockSize); \
  TMWTARG_UNUSED_PARAM(requestId); TMWTARG_UNUSED_PARAM(status); TMWTARG_UNUSED_PARAM(nOptionalChars); TMWTARG_UNUSED_PARAM(pOptionalChars);

#define DNPDIAG_SHOW_FILE_DATA_STATUS(pSession, handle, blockNumber, lastBlockFlag, status, nOptionalChars, pOptionalChars) \
  TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(handle); TMWTARG_UNUSED_PARAM(blockNumber); TMWTARG_UNUSED_PARAM(lastBlockFlag); \
  TMWTARG_UNUSED_PARAM(status); TMWTARG_UNUSED_PARAM(nOptionalChars); TMWTARG_UNUSED_PARAM(pOptionalChars);

#define DNPDIAG_SHOW_FILE_INFO(pSession, fileNameOffset, fileNameSize, fileType, fileSize, pTimeOfCreation, permissions, pFileName) \
  TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(fileNameOffset); TMWTARG_UNUSED_PARAM(fileNameSize); TMWTARG_UNUSED_PARAM(fileType); \
  TMWTARG_UNUSED_PARAM(fileSize); TMWTARG_UNUSED_PARAM(pTimeOfCreation); TMWTARG_UNUSED_PARAM(permissions); TMWTARG_UNUSED_PARAM(pFileName);

#define DNPDIAG_SHOW_AUTH_HEADER(pSession, pHdr) \
  TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(pHdr);

#define DNPDIAG_SHOW_AUTHTEST_HEADER(pSession, pHdr) \
  TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(pHdr);

#define DNPDIAG_SHOW_AUTHTEST_FORCE(pSession) \
  TMWTARG_UNUSED_PARAM(pSession);

#define DNPDIAG_SHOW_AUTH_CHALLENGE(pSession, userNumber, sequenceNumber, algorithm, reason, direction) \
  TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(sequenceNumber); TMWTARG_UNUSED_PARAM(userNumber); \
  TMWTARG_UNUSED_PARAM(algorithm); TMWTARG_UNUSED_PARAM(reason); TMWTARG_UNUSED_PARAM(direction);

#define DNPDIAG_SHOW_AUTH_REPLY(pSession, userNumber, sequenceNumber, status, direction) \
  TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(sequenceNumber); TMWTARG_UNUSED_PARAM(userNumber); \
  TMWTARG_UNUSED_PARAM(status); TMWTARG_UNUSED_PARAM(direction);

#define DNPDIAG_SHOW_AUTH_AGGRESSIVE(pSession, userNumber, sequenceNumber, expectedSequence, status, direction) \
  TMWTARG_UNUSED_PARAM(pSession);  TMWTARG_UNUSED_PARAM(userNumber); TMWTARG_UNUSED_PARAM(sequenceNumber);\
  TMWTARG_UNUSED_PARAM(expectedSequence); TMWTARG_UNUSED_PARAM(status); TMWTARG_UNUSED_PARAM(direction);
  
#define DNPDIAG_SHOW_AUTH_KEYSTATREQ(pSession, userNumber, direction) \
  TMWTARG_UNUSED_PARAM(pSession);TMWTARG_UNUSED_PARAM(userNumber); TMWTARG_UNUSED_PARAM(direction);

#define DNPDIAG_SHOW_KEY_STATUS(pSession, userNumber, sequenceNumber, keyWrapAlgorithm, status, macAlgorithm, direction) \
  TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(userNumber); TMWTARG_UNUSED_PARAM(sequenceNumber); \
  TMWTARG_UNUSED_PARAM(keyWrapAlgorithm); TMWTARG_UNUSED_PARAM(status); TMWTARG_UNUSED_PARAM(macAlgorithm); TMWTARG_UNUSED_PARAM(direction);

#define DNPDIAG_SHOW_KEY_STATUS_STATUS(pSession, userNumber, status) \
  TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(userNumber); TMWTARG_UNUSED_PARAM(status);

#define DNPDIAG_SHOW_AUTH_KEYCHANGE(pSession, userNumber, keyLength, sequenceNumber, expectedSequence, status, direction) \
  TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(userNumber); TMWTARG_UNUSED_PARAM(keyLength);\
  TMWTARG_UNUSED_PARAM(sequenceNumber); TMWTARG_UNUSED_PARAM(expectedSequence); TMWTARG_UNUSED_PARAM(status); TMWTARG_UNUSED_PARAM(direction);
 
#define DNPDIAG_MON_AUTH_KEYCHANGE(pSession, userNumber, sequenceNumber) \
  TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(userNumber); TMWTARG_UNUSED_PARAM(sequenceNumber);

#define DNPDIAG_SHOW_AUTH_ERROR(pSession, userNumber, assocId, sequenceNumber, errorCode, pTimeStamp, pErrorText, textLen, isEvent, direction) \
  TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(sequenceNumber); TMWTARG_UNUSED_PARAM(userNumber); \
  TMWTARG_UNUSED_PARAM(assocId); TMWTARG_UNUSED_PARAM(errorCode); TMWTARG_UNUSED_PARAM(pTimeStamp); \
  TMWTARG_UNUSED_PARAM(pErrorText); TMWTARG_UNUSED_PARAM(textLen); TMWTARG_UNUSED_PARAM(isEvent); TMWTARG_UNUSED_PARAM(direction);

#define DNPDIAG_AUTH_SHOW_DATA(pSession, dataType, pData, length, direction) \
  TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(dataType); TMWTARG_UNUSED_PARAM(pData); TMWTARG_UNUSED_PARAM(length); TMWTARG_UNUSED_PARAM(direction);

#define DNPDIAG_AUTH_INITVECTOR(pSession, pData, length, direction) \
  TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(pData); TMWTARG_UNUSED_PARAM(length); TMWTARG_UNUSED_PARAM(direction);

#define DNPDIAG_SHOW_AUTH_USERCERT(pSession, nameLength, pName, keyChangeMethod, certType, direction) \
  TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(nameLength); TMWTARG_UNUSED_PARAM(pName); TMWTARG_UNUSED_PARAM(keyChangeMethod);\
  TMWTARG_UNUSED_PARAM(certType); TMWTARG_UNUSED_PARAM(direction);

#define DNPDIAG_SHOW_AUTH_USERSTATUS(pSession, nameLength, pName, keyChangeMethod, operation, userRole, expires, sequence, lastSequence, direction) \
  TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(nameLength); TMWTARG_UNUSED_PARAM(pName); TMWTARG_UNUSED_PARAM(keyChangeMethod);\
  TMWTARG_UNUSED_PARAM(operation); TMWTARG_UNUSED_PARAM(userRole); TMWTARG_UNUSED_PARAM(expires); TMWTARG_UNUSED_PARAM(sequence);\
  TMWTARG_UNUSED_PARAM(lastSequence); TMWTARG_UNUSED_PARAM(direction);

#define DNPDIAG_SHOW_AUTH_UPDKEYCHGREQ(pSession, nameLength, pName, keyChangeMethod, challengeLength, pChallenge, direction)\
  TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(nameLength); TMWTARG_UNUSED_PARAM(pName); TMWTARG_UNUSED_PARAM(keyChangeMethod);\
  TMWTARG_UNUSED_PARAM(direction);

#define DNPDIAG_SHOW_AUTH_UPDATEKEYCHANGE(pSession, userNumber, sequenceNumber, direction)\
  TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(userNumber); TMWTARG_UNUSED_PARAM(sequenceNumber);\
  TMWTARG_UNUSED_PARAM(direction);

#define DNPDIAG_SHOW_AUTH_UPDATEKEYCHGSIG(pSession, direction)\
  TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(direction);

#define DNPDIAG_SHOW_AUTH_UPDATEKEYCHGCONF(pSession, userNumber, status, direction)\
  TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(userNumber); TMWTARG_UNUSED_PARAM(status); TMWTARG_UNUSED_PARAM(direction);

#define DNPDIAG_SHOW_AUTH_UPDATEKEYCHGRPLY(pSession, userNumber, challengeSequenceNumber, pData, length, direction)\
  TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(userNumber); TMWTARG_UNUSED_PARAM(challengeSequenceNumber);\
  TMWTARG_UNUSED_PARAM(pData); TMWTARG_UNUSED_PARAM(length); TMWTARG_UNUSED_PARAM(direction);

#define DNPDIAG_LINK_ADDRESS_UNKNOWN(pChannel, srcAddress, destAddress) \
  TMWTARG_UNUSED_PARAM(pChannel); TMWTARG_UNUSED_PARAM(srcAddress); TMWTARG_UNUSED_PARAM(destAddress);

#define DNPDIAG_ERROR(pChannel, pSession, errorNumber) \
  TMWTARG_UNUSED_PARAM(pChannel); TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(errorNumber);

#define DNPDIAG_ERROR_MSG(pChannel, pSession, errorNumber, pExtraTextMsg) \
  TMWTARG_UNUSED_PARAM(pChannel); TMWTARG_UNUSED_PARAM(pSession); TMWTARG_UNUSED_PARAM(errorNumber); TMWTARG_UNUSED_PARAM(pExtraTextMsg);

#else

#define DNPDIAG_BUILD_MESSAGE(pChannel, pSession, description) \
  dnpdiag_buildMessage(pChannel, pSession, description)

#define DNPDIAG_INSERT_QUEUE(pChannel, pSession, description) \
  dnpdiag_insertQueue(pChannel, pSession, description)

#define DNPDIAG_SHOW_OBJECT_HEADER(pSession, pHdr) \
  dnpdiag_showObjectHeader(pSession, pHdr)

#define DNPDIAG_SHOW_TX_OBJECT_HDR(pSession, group, variation, qualifier) \
  dnpdiag_showTxObjectHdr(pSession, group, variation, qualifier)

#define DNPDIAG_SHOW_IIN_VALUE(pSession, pointNumber, value) \
  dnpdiag_showIINValue(pSession, pointNumber, value);

#define DNPDIAG_SHOW_BINARY_INPUT(pSession, point, flags, isEvent, pTimeStamp) \
  dnpdiag_showBinaryInput(pSession, point, flags, isEvent, pTimeStamp)

#define DNPDIAG_SHOW_DOUBLE_INPUT(pSession, point, flags, isEvent, pTimeStamp) \
  dnpdiag_showDoubleInput(pSession, point, flags, isEvent, pTimeStamp)

#define DNPDIAG_SHOW_CROB(pSession, point, control, count, onTime, offTime, status, direction) \
  dnpdiag_showCROB(pSession, point, control, count, onTime, offTime, status, direction)

#define DNPDIAG_SHOW_BINARY_OUTPUT(pSession, point, flags, isEvent, pTimeStamp) \
  dnpdiag_showBinaryOutput(pSession, point, flags, isEvent, pTimeStamp)

#define DNPDIAG_SHOW_BIN_OUT_WRITE(pSession, point, state) \
  dnpdiag_showBinOutWrite(pSession, point, state)

#define DNPDIAG_SHOW_BINARY_CMD_STATUS(pSession, point, status, pTimeStamp) \
  dnpdiag_showBinaryCmdStatus(pSession, point, status, pTimeStamp)

#define DNPDIAG_SHOW_BINARY_COUNTER(pSession, point, value, flags, isEvent, pTimeStamp) \
  dnpdiag_showBinaryCounter(pSession, point, value, flags, isEvent, pTimeStamp)

#define DNPDIAG_SHOW_FROZEN_COUNTER(pSession, point, value, flags, isEvent, pTimeStamp) \
  dnpdiag_showFrozenCounter(pSession, point, value, flags, isEvent, pTimeStamp)

#define DNPDIAG_SHOW_ANALOG_INPUT(pSession, point, value, flags, isEvent, pTimeStamp) \
  dnpdiag_showAnalogInput(pSession, point, value, flags, isEvent, pTimeStamp)

#define DNPDIAG_SHOW_FROZEN_ANALOG(pSession, point, value, flags, isEvent, pTimeStamp) \
  dnpdiag_showFrozenAnalogInput(pSession, point, value, flags, isEvent, pTimeStamp)

#define DNPDIAG_SHOW_ANALOG_OUTPUT(pSession, point, value, flags, isEvent, pTimeStamp) \
  dnpdiag_showAnalogOutput(pSession, point, value, flags, isEvent, pTimeStamp)

#define DNPDIAG_SHOW_ANALOG_CONTROL(pSession, point, value, status, isEvent, direction, pTimeStamp) \
  dnpdiag_showAnalogControl(pSession, point, value, status, isEvent, direction, pTimeStamp)

#define DNPDIAG_SHOW_ANALOG_DEADBAND(pSession, point, value, direction) \
  dnpdiag_showAnalogDeadband(pSession, point, value, direction)
 
#define DNPDIAG_SHOW_TIME_AND_DATE(pSession, pDateTime, pText) \
  dnpdiag_showTimeAndDate(pSession, pDateTime, pText, TMWDIAG_ID_RX)

#define DNPDIAG_SHOW_TIME_AND_DATE_SENT(pSession, pDateTime) \
  dnpdiag_showTimeAndDate(pSession, pDateTime, TMWDEFS_NULL, 0)

#define DNPDIAG_SHOW_INDEXED_TIME(pSession, point, ptimeValue, pIntervalCount, pIntervalUnits, direction) \
  dnpdiag_showIndexedTime(pSession, point, ptimeValue, pIntervalCount, pIntervalUnits, direction)

#define DNPDIAG_SHOW_FREEZE(pSession, pDateTime, freezeInterval, direction) \
  dnpdiag_showFreeze(pSession, pDateTime, freezeInterval, direction)

#define DNPDIAG_SHOW_STRING_DATA(pSession, point, pBuf, bufLength, direction) \
  dnpdiag_showStringData(pSession, point, pBuf, bufLength, direction)

#define DNPDIAG_SHOW_STRING_EVENT(pSession, point, pBuf, bufLength) \
  dnpdiag_showStringEvent(pSession, point, pBuf, bufLength)

#define DNPDIAG_SHOW_VTERM_OUTPUT(pSession, point, pBuf, bufLength) \
  dnpdiag_showVirtualTerminalOutput(pSession, point, pBuf, bufLength)

#define DNPDIAG_SHOW_VTERM_EVENT(pSession, point, pBuf, bufLength) \
  dnpdiag_showVirtualTerminalEvent(pSession, point, pBuf, bufLength)

#define DNPDIAG_SHOW_EXT_STRING_DATA(pSession, point, pBuf, bufLength, flags, direction) \
  dnpdiag_showExtStringData(pSession, point, pBuf, bufLength, flags, direction)

#define DNPDIAG_SHOW_EXT_STRING_EVENT(pSession, point, pBuf, bufLength, flags, pTimeStamp) \
  dnpdiag_showExtStringEvent(pSession, point, pBuf, bufLength, flags, pTimeStamp)

#define DNPDIAG_SHOW_AUTH_SECURITY_STAT(pSession, association, point, value, flags, isEvent, pTimeStamp) \
  dnpdiag_showAuthSecStat(pSession, association, point, value, flags, isEvent, pTimeStamp)

#define DNPDIAG_SHOW_ACTIVATE_CONFIG(pSession, timeDelay, statusCode, pBuf, bufLength) \
  dnpdiag_showActivateConfig(pSession, timeDelay, statusCode, pBuf, bufLength)

#define DNPDIAG_FRAGMENT_SENT(pChannel, pSession, pFragment, numBytes) \
  dnpdiag_fragmentSent(pChannel, pSession, pFragment, numBytes)

#define DNPDIAG_FRAGMENT_RECEIVED(pChannel, pSession, pFragment, numBytes) \
  dnpdiag_fragmentReceived(pChannel, pSession, pFragment, numBytes)

#define DNPDIAG_TPRT_FRAME_SENT(pChannel, pSession, pFrame, numBytes) \
  dnpdiag_tprtFrameSent(pChannel, pSession, pFrame, numBytes)

#define DNPDIAG_TPRT_FRAME_RECEIVED(pChannel, pSession, pFrame, numBytes) \
  dnpdiag_tprtFrameReceived(pChannel, pSession, pFrame, numBytes)

#define DNPDIAG_LINK_FRAME_SENT(pChannel, pSession, pFrame, numBytes, retryCount) \
  dnpdiag_linkFrameSent(pChannel, pSession, pFrame, numBytes, retryCount)

#define DNPDIAG_LINK_HEADER_BLOCK_RECEIVED(pChannel, pSession, pBuf, numBytes) \
  dnpdiag_linkHeaderBlockReceived(pChannel, pSession, pBuf, numBytes)

#define DNPDIAG_LINK_DATA_BLOCK_RECEIVED(pContext, pBuf, numBytes) \
  dnpdiag_linkDataBlockReceived(pContext, pBuf, numBytes)

#define DNPDIAG_LINK_FRAME_COMPLETE(pContext, discarded) \
  dnpdiag_linkFrameComplete(pContext, discarded)

#define DNPDIAG_LINK_FRAME_RECEIVED(pChannel, pSession, pFrame, numBytes) \
  dnpdiag_linkFrameReceived(pChannel, pSession, pFrame, numBytes)

#define DNPDIAG_SHOW_DATASET_PROTO(pSession, point, elemIndex, pDescr, direction) \
  dnpdiag_showDatasetProto(pSession, point, elemIndex, pDescr, direction)

#define DNPDIAG_SHOW_DATASET_DSCR_CONT(pSession, point, elemIndex, pDescr, direction) \
  dnpdiag_showDatasetDscrCont(pSession, point, elemIndex, pDescr, direction)

#define DNPDIAG_SHOW_DATASET_DSCR_CHRS(pSession, point, value, direction)\
  dnpdiag_showDatasetDscrChrs(pSession, point, value, direction)

#define DNPDIAG_SHOW_DATASET_DSCR_INDX(pSession, point, elemIndex, pDescr, direction) \
  dnpdiag_showDatasetDscrIndx(pSession, point, elemIndex, pDescr, direction)

#define DNPDIAG_SHOW_DATASET_TIME(pSession, point, pTimeStamp, isEvent, direction) \
  dnpdiag_showDatasetTime(pSession, point, pTimeStamp, isEvent, direction);

#define DNPDIAG_SHOW_DATASET(pSession, elemIndex, pElem, fc, isIgnored, isEvent, direction) \
  dnpdiag_showDataset(pSession, elemIndex, pElem, fc, isIgnored, isEvent, direction)

#define DNPDIAG_SHOW_DEVICE_ATTRIBUTE(pSession, point, variation, pValue, direction) \
  dnpdiag_showDeviceAttribute(pSession, point, variation, pValue, direction)

#define DNPDIAG_SHOW_DEVICE_ATTR_PROP(pSession, point, variation, property) \
  dnpdiag_showDeviceAttrProperty(pSession, point, variation, property) 

#define DNPDIAG_SHOW_FILE_OPEN(pSession, filename, fileNameLength, fileSize, pTimeOfCreation, permissions, authKey, mode, maxBlockSize, requestId)\
  dnpdiag_showFileOpen(pSession, filename, fileNameLength, fileSize, pTimeOfCreation, permissions, authKey, mode, maxBlockSize, requestId)

#define DNPDIAG_SHOW_FILE_CLOSE(pSession, handle, requestId)\
  dnpdiag_showFileClose(pSession, handle, requestId)

#define DNPDIAG_SHOW_FILE_DATA(pSession, handle, block, numBytes, last, isEvent, direction)\
  dnpdiag_showFileData(pSession, handle, block, numBytes, last, isEvent, direction)

#define DNPDIAG_SHOW_FILE_STRING(pSession, point, pBuf, bufLength, direction) \
  dnpdiag_showFileString(pSession, point, pBuf, bufLength, direction)

#define DNPDIAG_SHOW_FILE_STATUS(pSession, handle, fileSize, maxBlockSize, requestId, status, nOptionalChars, pOptionalChars) \
  dnpdiag_showFileStatus(pSession, handle, fileSize, maxBlockSize, requestId, status, nOptionalChars, pOptionalChars)

#define DNPDIAG_SHOW_FILE_DATA_STATUS(pSession, handle, blockNumber, lastBlockFlag, status, nOptionalChars, pOptionalChars) \
  dnpdiag_showFileDataStatus(pSession, handle, blockNumber, lastBlockFlag, status, nOptionalChars, pOptionalChars)

#define DNPDIAG_SHOW_FILE_INFO(pSession, fileNameOffset, fileNameSize, fileType, fileSize, pTimeOfCreation, permissions, pFileName) \
  dnpdiag_showFileInfo(pSession, fileNameOffset, fileNameSize, fileType, fileSize, pTimeOfCreation, permissions, pFileName)

#define DNPDIAG_SHOW_AUTH_HEADER(pSession, pHdr) \
  dnpdiag_showAuthHeader(pSession, pHdr)

#define DNPDIAG_SHOW_AUTHTEST_HEADER(pSession, pHdr) \
  dnpdiag_showAuthTestHeader(pSession, pHdr)

#define DNPDIAG_SHOW_AUTHTEST_FORCE(pSession) \
  dnpdiag_showAuthTestForce(pSession)

#define DNPDIAG_SHOW_AUTH_CHALLENGE(pSession, userNumber, sequenceNumber, algorithm, reason, direction) \
  dnpdiag_showAuthChallenge(pSession, userNumber, sequenceNumber, algorithm, reason, direction)

#define DNPDIAG_SHOW_AUTH_REPLY(pSession, userNumber, sequenceNumber, status, direction) \
  dnpdiag_showAuthReply(pSession, userNumber, sequenceNumber, status, direction)

#define DNPDIAG_SHOW_AUTH_AGGRESSIVE(pSession, userNumber, sequenceNumber, expectedSequence, status, direction) \
  dnpdiag_showAuthAggressive(pSession, userNumber, sequenceNumber, expectedSequence, status, direction)

#define DNPDIAG_SHOW_AUTH_KEYSTATREQ(pSession, userNumber, direction) \
  dnpdiag_showAuthKeyStatReq(pSession, userNumber, direction)

#define DNPDIAG_SHOW_KEY_STATUS(pSession, userNumber, sequenceNumber, keyWrapAlgorithm, status, macAlgorithm, direction) \
  dnpdiag_showAuthKeyStatus(pSession, userNumber, sequenceNumber, keyWrapAlgorithm, status, macAlgorithm, direction)

#define DNPDIAG_SHOW_KEY_STATUS_STATUS(pSession, userNumber, status) \
  dnpdiag_showAuthKeyStatusStatus(pSession, userNumber, status)

#define DNPDIAG_SHOW_AUTH_KEYCHANGE(pSession, userNumber, keyLength, sequenceNumber, expectedSequence, status, direction) \
  dnpdiag_showAuthKeyChange(pSession, userNumber, keyLength, sequenceNumber, expectedSequence, status, direction)

#define DNPDIAG_MON_AUTH_KEYCHANGE(pSession, userNumber, sequenceNumber) \
  dnpdiag_monitorAuthKeyChange(pSession, userNumber, sequenceNumber)

#define DNPDIAG_SHOW_AUTH_ERROR(pSession, userNumber, assocId, sequenceNumber, errorCode, pTimeStamp, pErrorText, textLen, isEvent, direction) \
  dnpdiag_showAuthError(pSession, userNumber, assocId, sequenceNumber, errorCode, pTimeStamp, pErrorText, textLen, isEvent, direction)

#define DNPDIAG_AUTH_SHOW_DATA(pSession, dataType, pData, length, direction) \
  dnpdiag_authShowData(pSession, dataType, pData, length, direction);

#define DNPDIAG_AUTH_INITVECTOR(pSession, pData, length, direction) \
  dnpdiag_authInitVector(pSession, pData, length, direction);

#define DNPDIAG_SHOW_AUTH_USERCERT(pSession, nameLength, pName, keyChangeMethod, certType, direction) \
  dnpdiag_authUserCert(pSession, nameLength, pName, keyChangeMethod, certType, direction); 

#define DNPDIAG_SHOW_AUTH_USERSTATUS(pSession, nameLength, pName, keyChangeMethod, operation, userRole, expires, sequence, lastSequence, direction) \
  dnpdiag_authUserStatus(pSession, nameLength, pName, keyChangeMethod, operation, userRole, expires, sequence, lastSequence, direction); 

#define DNPDIAG_SHOW_AUTH_UPDKEYCHGREQ(pSession, nameLength, pName, keyChangeMethod, challengeLength, pChallenge, direction)\
  dnpdiag_authUpdKeyChgReq(pSession, nameLength, pName, keyChangeMethod, challengeLength, pChallenge, direction); 

#define DNPDIAG_SHOW_AUTH_UPDATEKEYCHANGE(pSession, userNumber, sequenceNumber, direction)\
  dnpdiag_authUpdKeyChange(pSession, userNumber, sequenceNumber, direction); 

#define DNPDIAG_SHOW_AUTH_UPDATEKEYCHGSIG(pSession, direction)\
  dnpdiag_authUpdKeyChgSig(pSession, direction); 

#define DNPDIAG_SHOW_AUTH_UPDATEKEYCHGCONF(pSession, userNumber, status, direction)\
  dnpdiag_authUpdKeyChgConf(pSession, userNumber, status, direction); 

#define DNPDIAG_SHOW_AUTH_UPDATEKEYCHGRPLY(pSession, userNumber, challengeSequenceNumber, pData, length, direction)\
  dnpdiag_authUpdKeyChgRply(pSession, userNumber, challengeSequenceNumber, pData, length, direction); 

#define DNPDIAG_LINK_ADDRESS_UNKNOWN(pChannel, srcAddress, destAddress) \
  dnpdiag_linkAddressUnknown(pChannel, srcAddress, destAddress)

#define DNPDIAG_ERROR(pChannel, pSession, errorNumber) \
  dnpdiag_errorMsg(pChannel, pSession, errorNumber, TMWDEFS_NULL)

#define DNPDIAG_ERROR_MSG(pChannel, pSession, errorNumber, pExtraTextMsg) \
  dnpdiag_errorMsg(pChannel, pSession, errorNumber, pExtraTextMsg)

#ifdef __cplusplus
extern "C" {
#endif
  
  /* routine: dnpdiag_init
   * purpose: internal diagnostic init function
   * arguments:
   *  void
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL dnpdiag_init(void);
 
  /* routine: dnpdiag_validateErrorTable
   * purpose: Called only to verify if error message table is correct.
   *  This is intended for test purposes only.
   * arguments:
   *  void
   * returns:
   *  TMWDEFS_TRUE if formatted correctly
   *  TMWDEFS_FALSE if there is an error in the table.
   */
  TMWTYPES_BOOL dnpdiag_validateErrorTable(void);

  /* function: dnpdiag_buildMessage
   * purpose:
   * arguments:
   *  pChannel - channel from which this message originated
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL dnpdiag_buildMessage(
    TMWCHNL *pChannel,
    TMWSESN *pSession, 
    const char *description);

  /* function: dnpdiag_insertQueue
   * purpose:
   * arguments:
   *  pChannel - channel from which this message originated
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL dnpdiag_insertQueue(
    TMWCHNL *pChannel,
    TMWSESN *pSession, 
    const char *description);

  /* function: dnpdiag_showObjectHeader
   * purpose: display object header info being received
   * arguments:
   *  pSession - session from which this message originated
   *  pHdr - pointer to DNP Object Header to display
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL dnpdiag_showObjectHeader(
    TMWSESN *pSession,
    DNPUTIL_OBJECT_HEADER *pHdr);

  /* function: dnpdiag_showTxObjectHdr
   * purpose: display object header info being transmitted
   * arguments:
   *  pSession - session from which this message originated 
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL dnpdiag_showTxObjectHdr(
    TMWSESN *pSession, 
    TMWTYPES_UCHAR group,
    TMWTYPES_UCHAR variation,
    TMWTYPES_UCHAR qualifier); 

  /* function: dnpdiag_showIINValue
   * purpose: display IIN bits from read object 80
   * arguments:
   *  pSession - session from which this message originated
   *  pointNumber - point number
   *  value - value of IIN bit
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL dnpdiag_showIINValue(
    TMWSESN *pSession,
    TMWTYPES_USHORT pointNumber, 
    TMWTYPES_BOOL value);

  /* function: dnpdiag_showBinaryInput
   * purpose: display binary input data
   * arguments:
   *  pSession - session from which this message originated
   *  point - point number
   *  flags - binary input with status flags
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL dnpdiag_showBinaryInput(
    TMWSESN *pSession,
    TMWTYPES_USHORT point, 
    TMWTYPES_UCHAR flags,  
    TMWTYPES_BOOL isEvent,
    TMWDTIME *pTimeStamp);

   /* function: dnpdiag_showDoubleInput
   * purpose: display double bit input data
   * arguments:
   *  pSession - session from which this message originated
   *  point - point number
   *  flags - binary input with status flags
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL dnpdiag_showDoubleInput(
    TMWSESN *pSession,
    TMWTYPES_USHORT point, 
    TMWTYPES_UCHAR flags,  
    TMWTYPES_BOOL isEvent,
    TMWDTIME *pTimeStamp);

  /* function: dnpdiag_showCROB
   * purpose: display control relay output block
   * arguments:
   *  pSession - session from which this message originated
   *  point - point number
   *  control - control code
   *  count - count
   *  onTime - signal activation period
   *  offTime - signal deactivation period
   *  status - status code
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL dnpdiag_showCROB(
    TMWSESN *pSession,
    TMWTYPES_USHORT point,
    TMWTYPES_UCHAR control,
    TMWTYPES_UCHAR count,
    TMWTYPES_ULONG onTime,
    TMWTYPES_ULONG offTime,
    TMWTYPES_UCHAR status,
    TMWDIAG_ID direction);

  /* function: dnpdiag_showBinaryOutput
   * purpose: display binary output data
   * arguments:
   *  pSession - session from which this message originated
   *  point - point number
   *  flags - binary output with status flags
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL dnpdiag_showBinaryOutput(
    TMWSESN *pSession,
    TMWTYPES_USHORT point, 
    TMWTYPES_UCHAR flags,  
    TMWTYPES_BOOL isEvent,
    TMWDTIME *pTimeStamp);

  /* function: dnpdiag_showBinOutWrite  
  * purpose: display binary output data from write request
  * arguments:
  *  pSession - session from which this message originated
  *  point - point number
  *  state - binary output state
  * returns:
  *  void
  */
  void TMWDEFS_GLOBAL dnpdiag_showBinOutWrite(
    TMWSESN *pSession,
    TMWTYPES_USHORT point,
    TMWTYPES_UCHAR state);

  /* function: dnpdiag_showBinaryCmdStatus
   * purpose: display binary output command status event
   * arguments:
   *  pSession - session from which this message originated
   *  point - point number
   *  status - command status 
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL dnpdiag_showBinaryCmdStatus(
    TMWSESN *pSession,
    TMWTYPES_USHORT point, 
    TMWTYPES_UCHAR status,  
    TMWDTIME *pTimeStamp);

  /* function: dnpdiag_showBinaryCounter
   * purpose: display binary counter data
   * arguments:
   *  pSession - session from which this message originated
   *  point - point number
   *  value - binary counter value
   *  flags - status flags
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL dnpdiag_showBinaryCounter(
    TMWSESN *pSession,
    TMWTYPES_USHORT point, 
    TMWTYPES_ULONG value, 
    TMWTYPES_UCHAR flags,
    TMWTYPES_BOOL isEvent,
    TMWDTIME *pTimeStamp);

  /* function: dnpdiag_showFrozenCounter
   * purpose: display frozen counter data
   * arguments:
   *  pSession - session from which this message originated
   *  point - point number
   *  value - frozen counter value
   *  flags - status flags
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL dnpdiag_showFrozenCounter(
    TMWSESN *pSession,
    TMWTYPES_USHORT point, 
    TMWTYPES_ULONG value, 
    TMWTYPES_UCHAR flags, 
    TMWTYPES_BOOL isEvent,
    TMWDTIME *pTimeStamp);

  /* function: dnpdiag_showAnalogInput
   * purpose: display analog input data
   * arguments:
   *  pSession - session from which this message originated
   *  point - point number
   *  value - analog input value
   *  flags - status flags
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL dnpdiag_showAnalogInput(
    TMWSESN *pSession,
    TMWTYPES_USHORT point, 
    TMWTYPES_ANALOG_VALUE *pValue, 
    TMWTYPES_UCHAR flags,
    TMWTYPES_BOOL isEvent,
    TMWDTIME *pTimeStamp);

  /* function: dnpdiag_showFrozenAnalogInput
   * purpose: display frozen analog input data
   * arguments:
   *  pSession - session from which this message originated
   *  point - point number
   *  value - frozen analog value
   *  flags - status flags
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL dnpdiag_showFrozenAnalogInput(
    TMWSESN *pSession,
    TMWTYPES_USHORT point, 
    TMWTYPES_ANALOG_VALUE *pValue, 
    TMWTYPES_UCHAR flags, 
    TMWTYPES_BOOL isEvent,
    TMWDTIME *pTimeStamp);

  /* function: dnpdiag_showAnalogOutput
   * purpose: display analog output data
   * arguments:
   *  pSession - session from which this message originated
   *  point - point number
   *  value - analog output value
   *  flags - status flags
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL dnpdiag_showAnalogOutput(
    TMWSESN *pSession,
    TMWTYPES_USHORT point, 
    TMWTYPES_ANALOG_VALUE *pValue, 
    TMWTYPES_UCHAR flags,
    TMWTYPES_BOOL isEvent,
    TMWDTIME *pTimeStamp);

    /* function: dnpdiag_showAnalogOutput
   * purpose: display analog output data
   * arguments:
   *  pSession - session from which this message originated
   *  point - point number
   *  value - analog output value
   *  status - status code
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL dnpdiag_showAnalogControl(
    TMWSESN *pSession,
    TMWTYPES_USHORT point, 
    TMWTYPES_ANALOG_VALUE *pValue, 
    TMWTYPES_UCHAR status,
    TMWTYPES_BOOL isEvent, 
    TMWDIAG_ID direction,
    TMWDTIME *pTimeStamp);

  /* function: dnpdiag_showAnalogDeadband
   * purpose: display analog deadband data
   * arguments:
   *  pSession - session from which this message originated
   *  point - point number
   *  value - analog deadband value
   *  direction - indicate if this was received or is being sent
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL dnpdiag_showAnalogDeadband(
    TMWSESN *pSession,
    TMWTYPES_USHORT point, 
    TMWTYPES_ANALOG_VALUE *pValue,
    TMWDIAG_ID direction);

  /* function: dnpdiag_showTimeAndDate
   * purpose: display time and date
   * arguments:
   *  pSession - session from which this message originated
   *  pDateTime - time and date to display
   *  direction - if Rxd or Txd
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL dnpdiag_showTimeAndDate(
    TMWSESN *pSession,
    TMWDTIME *pDateTime,
    TMWTYPES_CHAR *pText,
    TMWDIAG_ID direction);

  /* function: dnpdiag_showIndexedTime
   * purpose: display indexed absolute time
   * arguments:
   *  pSession - session from which this message originated
   *  pDateTime - start time
   *  intervalCount - interval count
   *  intervalUnits - interval units
   *  direction - if Rxd or Txd
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL dnpdiag_showIndexedTime(
    TMWSESN *pSession,
    TMWTYPES_USHORT point,
    TMWDTIME *pDateTime,
    TMWTYPES_ULONG intervalCount,
    TMWTYPES_BYTE intervalUnits,
    TMWDIAG_ID direction);

  /* function: dnpdiag_showFreeze
   * purpose: display freeze data
   * arguments:
   *  pSession - session from which this message originated
   *  pDateTime - freeze time
   *  freezeInterval - interval units
   *  direction - if Rxd or Txd
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL dnpdiag_showFreeze(
    TMWSESN *pSession,
    TMWDTIME *pDateTime,
    TMWTYPES_ULONG freezeInterval,
    TMWDIAG_ID direction);

  /* function: dnpdiag_showStringData
    * purpose: display string data
    * arguments:
    *  pSession - session from which this message originated
    *  point - point number
    *  pBuf - buffer containing string
    *  bufLength - length of string
    * returns:
    *  void
    */
  void TMWDEFS_GLOBAL dnpdiag_showStringData(
    TMWSESN *pSession,
    TMWTYPES_USHORT point,
    TMWTYPES_UCHAR *pBuf,
    TMWTYPES_USHORT bufLength,
    TMWDIAG_ID direction);

  /* function: dnpdiag_showStringEvent
    * purpose: display string event
    * arguments:
    *  pSession - session from which this message originated
    *  point - point number
    *  pBuf - buffer containing string
    *  bufLength - length of string
    * returns:
    *  void
    */
  void TMWDEFS_GLOBAL dnpdiag_showStringEvent(
    TMWSESN *pSession,
    TMWTYPES_USHORT point,
    TMWTYPES_UCHAR *pBuf,
    TMWTYPES_UCHAR bufLength);

  /* function: dnpdiag_showVirtualTerminalOutput
    * purpose: display virtual terminal output
    * arguments:
    *  pSession - session from which this message originated
    *  point - point number
    *  pBuf - buffer containing virtual terminal string
    *  bufLength - length of string
    * returns:
    *  void
    */
  void TMWDEFS_GLOBAL dnpdiag_showVirtualTerminalOutput(
    TMWSESN *pSession,
    TMWTYPES_USHORT point,
    TMWTYPES_UCHAR *pBuf,
    TMWTYPES_UCHAR bufLength);

  /* function: dnpdiag_showVirtualTerminalEvent
   * purpose: display virtual terminal event
   * arguments:
   *  pSession - session from which this message originated
   *  point - point number
   *  pBuf - buffer containing virtual terminal string
   *  bufLength - length of string
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL dnpdiag_showVirtualTerminalEvent(
    TMWSESN *pSession,
    TMWTYPES_USHORT point,
    TMWTYPES_UCHAR *pBuf,
    TMWTYPES_UCHAR bufLength);

  /* function: dnpdiag_showExtStringData
   * purpose: display extended string data
   * arguments:
   *  pSession - session from which this message originated
   *  point - point number
   *  pBuf - buffer containing string
   *  bufLength - length of string
   *  flags - status flags
   *  direction - indicate if this was received or is being sent
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL dnpdiag_showExtStringData(
    TMWSESN *pSession,
    TMWTYPES_USHORT point,
    TMWTYPES_UCHAR *pBuf,
    TMWTYPES_USHORT bufLength,
    TMWTYPES_UCHAR flags,
    TMWDIAG_ID direction);

  /* function: dnpdiag_showExtStringEvent
   * purpose: display extended string event
   * arguments:
   *  pSession - session from which this message originated
   *  point - point number
   *  pBuf - buffer containing string
   *  bufLength - length of string
   *  flags - status flags
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL dnpdiag_showExtStringEvent(
    TMWSESN *pSession,
    TMWTYPES_USHORT point,
    TMWTYPES_UCHAR *pBuf,
    TMWTYPES_USHORT bufLength,
    TMWTYPES_UCHAR flags,
    TMWDTIME *pTimeStamp);

  /* function: dnpdiag_showAuthSecStat
   * purpose: display security statistic data
   * arguments:
   *  pSession - session from which this message originated
   *  association - association this applies to
   *  point - point number
   *  value - security statistic value
   *  flags - status flags
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL dnpdiag_showAuthSecStat(
    TMWSESN *pSession,
    TMWTYPES_USHORT association, 
    TMWTYPES_USHORT point, 
    TMWTYPES_ULONG value, 
    TMWTYPES_UCHAR flags,
    TMWTYPES_BOOL isEvent,
    TMWDTIME *pTimeStamp);

  /* function: dnpdiag_showActivateConfig
   * purpose: display Activate Configuration Response
   * arguments:
   *  pSession - session from which this message originated
   *  timeDelay - time delay
   *  statusCode - status code
   *  pBuf - buffer containing status code information
   *  bufLength - length of string
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL dnpdiag_showActivateConfig(
    TMWSESN *pSession,
    TMWTYPES_ULONG timeDelay,
    TMWTYPES_USHORT statusCode,
    TMWTYPES_UCHAR *pBuf,
    TMWTYPES_UCHAR bufLength);

  /* function: dnpdiag_fragmentSent
   * purpose: display transmitted application layer fragment
   * arguments:
   *  pChannel - channel from which this message originated
   *  pSession - session from which this message originated
   *  pFragment - pointer to fragment buffer
   *  numBytes - number of bytes in fragment
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL dnpdiag_fragmentSent(
    TMWCHNL *pChannel,
    TMWSESN *pSession, 
    const TMWTYPES_UCHAR *pFragment, 
    TMWTYPES_USHORT numBytes);

  /* function: dnpdiag_fragmentReceived
   * purpose: display received application layer fragment
   * arguments:
   *  pChannel - channel from which this message originated
   *  pSession - session from which this message originated
   *  pFragment - pointer to fragment buffer
   *  numBytes - number of bytes in fragment
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL dnpdiag_fragmentReceived(
    TMWCHNL *pChannel,
    TMWSESN *pSession, 
    const TMWTYPES_UCHAR *pFragment, 
    TMWTYPES_USHORT numBytes);

  /* function: dnpdiag_tprtFrameSent
   * purpose: display transmitted transport layer frame
   * arguments:
   *  pChannel - channel from which this message originated
   *  pSession - session from which this message originated
   *  pFrame - pointer to frame buffer
   *  numBytes - number of bytes in message
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL dnpdiag_tprtFrameSent(
    TMWCHNL *pChannel,
    TMWSESN *pSession, 
    const TMWTYPES_UCHAR *pFrame, 
    TMWTYPES_USHORT numBytes);

  /* function: dnpdiag_tprtFrameReceived
   * purpose: display received transport layer frame
   * arguments:
   *  pChannel - channel from which this message originated
   *  pSession - session from which this message originated
   *  pFrame - pointer to frame buffer
   *  numBytes - number of bytes in frame
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL dnpdiag_tprtFrameReceived(
    TMWCHNL *pChannel,
    TMWSESN *pSession, 
    const TMWTYPES_UCHAR *pFrame, 
    TMWTYPES_USHORT numBytes);

  /* function: dnpdiag_linkFrameSent
   * purpose: display link level frame
   * arguments:
   *  pChannel - channel from which this message originated
   *  pSession - session from which this message originated
   *  pFrame - pointer to frame buffer
   *  numBytes - number of bytes in frame
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL dnpdiag_linkFrameSent(
    TMWCHNL *pChannel,
    TMWSESN *pSession, 
    const TMWTYPES_UCHAR *pFrame, 
    TMWTYPES_USHORT numBytes,
    TMWTYPES_USHORT retryCount);

  /* function: dnpdiag_linkHeaderBlockReceived
   * purpose: display header block received from this channel
   * arguments:
   *  pChannel - channel from which this message originated
   *  pSession - session from which this message originated
   *  pBuf - pointer to header
   *  numBytes - number of bytes in header
   * returns:
   *  pointer to frame context
   */
  void * TMWDEFS_GLOBAL dnpdiag_linkHeaderBlockReceived(
    TMWCHNL *pChannel,
    TMWSESN *pSession, 
    const TMWTYPES_UCHAR *pBuf, 
    TMWTYPES_USHORT numBytes);

  /* function: dnpdiag_linkDataBlockReceived
   * purpose: display a data block received from this channel
   * arguments:
   *  pContext - frame context returned from 
   *   dnpdiag_linkHeaderBlockReceived
   *  pBuf - pointer to block of data
   *  numBytes - number of bytes in data block
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL dnpdiag_linkDataBlockReceived(
    void *pContext,
    const TMWTYPES_UCHAR *pBuf, 
    TMWTYPES_USHORT numBytes);

  /* function: dnpdiag_linkFrameComplete
   * purpose: display link level frame
   * arguments:
   *  pContext - frame context returned from 
   *   dnpdiag_linkHeaderBlockReceived
   *  discarded - TRUE if frame was discarded because of some error
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL dnpdiag_linkFrameComplete(
    void *pContext,
    TMWTYPES_BOOL discarded);

  /* function: dnpdiag_linkFrameReceived
   * purpose: display link level frame
   * arguments:
   *  pChannel - channel from which this message originated
   *  pSession - session from which this message originated
   *  pFrame - pointer to frame buffer
   *  numBytes - number of bytes in message
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL dnpdiag_linkFrameReceived(
    TMWCHNL *pChannel,
    TMWSESN *pSession, 
    const TMWTYPES_UCHAR *pFrame, 
    TMWTYPES_USHORT numBytes);

  /* function: dnpdiag_showDatasetProto
  * purpose: display  
  * arguments:
  *  pSession - session from which this message originated
  *  point - point number 
  * returns:
  *  void
  */
  void TMWDEFS_GLOBAL dnpdiag_showDatasetProto(
    TMWSESN *pSession,
    TMWTYPES_USHORT point,
    TMWTYPES_UCHAR elemIndex,
    DNPDATA_DATASET_DESCR_ELEM *pDescr,
    TMWDIAG_ID direction);

  /* function: dnpdiag_showDataDescrCont
  * purpose: display  
  * arguments:
  *  pSession - session from which this message originated
  *  point - point number 
  * returns:
  *  void
  */
  void TMWDEFS_GLOBAL dnpdiag_showDatasetDscrCont(
    TMWSESN *pSession,
    TMWTYPES_USHORT point,
    TMWTYPES_UCHAR elemIndex,
    DNPDATA_DATASET_DESCR_ELEM *pDescr,
    TMWDIAG_ID direction);

  /* function: dnpdiag_showDataDescrChars
  * purpose: display  
  * arguments:
  *  pSession - session from which this message originated
  *  point - point number 
  * returns:
  *  void
  */
  void TMWDEFS_GLOBAL dnpdiag_showDatasetDscrChrs(
    TMWSESN *pSession,
    TMWTYPES_USHORT point,
    TMWTYPES_UCHAR value,
    TMWDIAG_ID direction);

  /* function: dnpdiag_showDataDescrIndex
  * purpose: display  
  * arguments:
  *  pSession - session from which this message originated
  *  point - point number 
  * returns:
  *  void
  */
  void TMWDEFS_GLOBAL dnpdiag_showDatasetDscrIndx(
    TMWSESN *pSession,
    TMWTYPES_USHORT point,
    TMWTYPES_UCHAR elemIndex,
    DNPDATA_DATASET_DESCR_INDEX *pDescr,
    TMWDIAG_ID direction);

  /* function: dnpdiag_showDatasetTime
   * purpose: display  
   * arguments:
   *  pSession - session from which this message originated
   *  point - point number 
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL dnpdiag_showDatasetTime(
    TMWSESN *pSession,
    TMWTYPES_USHORT point,
    TMWDTIME *pTimeStamp,
    TMWTYPES_BOOL isEvent,
    TMWDIAG_ID direction);

  /* function: dnpdiag_showDataset
   * purpose: display  
   * arguments:
   *  pSession - session from which this message originated
   *  elemIndex
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL dnpdiag_showDataset(
    TMWSESN *pSession,
    TMWTYPES_UCHAR elemIndex,
    DNPDATA_DATASET_VALUE *pElem,
    TMWTYPES_UCHAR fc,
    TMWTYPES_BOOL isIgnored, 
    TMWTYPES_BOOL isEvent,
    TMWDIAG_ID direction);
  
  /* function: dnpdiag_showDeviceAttribute
   * purpose: display  
   * arguments:
   *  pSession - session from which this message originated
   *  elemIndex
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL dnpdiag_showDeviceAttribute(
    TMWSESN *pSession,
    TMWTYPES_USHORT point,
    TMWTYPES_UCHAR variation,
    DNPDATA_ATTRIBUTE_VALUE *pValue,
    TMWDIAG_ID direction);

  /* function: dnpdiag_showDeviceAttrProperty
   * purpose: display  
   * arguments:
   *  pSession - session from which this message originated
   *  elemIndex
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL dnpdiag_showDeviceAttrProperty(
    TMWSESN *pSession,
    TMWTYPES_USHORT point,
    TMWTYPES_UCHAR variation,
    TMWTYPES_UCHAR property);
 
  /* function: dnpdiag_showFileOpen
   * purpose: display  
   * arguments:
   *  pSession - session from which this message originated
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL dnpdiag_showFileOpen(
    TMWSESN *pSession,
    TMWTYPES_CHAR *pFileName,
    TMWTYPES_USHORT fileNameLength,
    TMWTYPES_ULONG fileSize,
    TMWTYPES_MS_SINCE_70 *pTimeOfCreation,
    TMWTYPES_USHORT permissions,
    TMWTYPES_ULONG authKey,
    TMWTYPES_USHORT mode,
    TMWTYPES_USHORT maxBlockSize,
    TMWTYPES_USHORT requestId);

  /* function: dnpdiag_showFileClose
   * purpose: display  
   * arguments:
   *  pSession - session from which this message originated
   *  handle - file handle
   *  requestId - id of request submitted
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL dnpdiag_showFileClose(
    TMWSESN *pSession,
    TMWTYPES_ULONG handle,
    TMWTYPES_USHORT requestId);

  /* function: dnpdiag_showFileData
   * purpose: display  
   * arguments:
   *  pSession - session from which this message originated
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL dnpdiag_showFileData(
    TMWSESN *pSession,
    TMWTYPES_ULONG handle,
    TMWTYPES_ULONG blockNumber,
    TMWTYPES_USHORT numBytes,
    TMWTYPES_BOOL last,
    TMWTYPES_BOOL isEvent,
    TMWDIAG_ID direction);
   
  /* function: dnpdiag_showFileString
   * purpose: display  
   * arguments:
   *  pSession - session from which this message originated
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL dnpdiag_showFileString(
    TMWSESN *pSession,
    TMWTYPES_USHORT point,
    TMWTYPES_UCHAR *pBuf,
    TMWTYPES_USHORT bufLength,
    TMWDIAG_ID direction);

  /* function: dnpdiag_showFileStatus
   * purpose: Display file status
   * arguments:
   *  handle - file handle
   *  reqFC - request function code
   *  size - file size if open for read (NULL otherwise)
   *  maxBlockSize - maximum block size outstation will return
   *  requestId - id of request submitted
   *  status - status/error condition
   *  nOptionalChars - number of bytes in pOptionalChars
   *  pOptionalChars - Optional ASCII characters
   * returns:
   *  void
   */
  void dnpdiag_showFileStatus(
    TMWSESN *pSession, 
    TMWTYPES_ULONG handle,
    TMWTYPES_ULONG fileSize,
    TMWTYPES_USHORT maxBlockSize,
    TMWTYPES_USHORT requestId,
    DNPDEFS_FILE_CMD_STAT status,
    TMWTYPES_USHORT nOptionalChars,
    const TMWTYPES_CHAR *pOptionalChars);

  /* function: dnpdiag_showFileDataStatus
   * purpose: Display file data status
   * arguments:
   *  handle - file handle
   *  blockNumber - current file block number
   *  lastBlockFlag - TRUE if last block
   *  status - status/error condition
   *  nOptionalChars - number of bytes in pOptionalChars
   *  pOptionalChars - Optional ASCII characters
   * returns:
   *  void
   */
  void dnpdiag_showFileDataStatus(
    TMWSESN *pSession, 
    TMWTYPES_ULONG handle,
    TMWTYPES_ULONG blockNumber,
    TMWTYPES_BOOL lastBlockFlag,
    DNPDEFS_FILE_TFER_STAT status,
    TMWTYPES_USHORT nOptionalChars,
    const TMWTYPES_CHAR *pOptionalChars);
  
  /* function: dnpdiag_showFileInfo
   * purpose: Display file info
   * arguments:
   *  fileNameOffset - offset to file name
   *  fileNameSize - length of pFileName
   *  fileType - simple file(1) or directory(0)
   *  fileSize - number of bytes in file
   *  pTimeOfCreation - creation time of the file
   *  permissions - permissions of file
   *  pFileName - the file name
   * returns:
   *  void
   */
  void dnpdiag_showFileInfo(
    TMWSESN *pSession, 
    TMWTYPES_USHORT fileNameOffset,
    TMWTYPES_USHORT fileNameSize,
    DNPDEFS_FILE_TYPE fileType,
    TMWTYPES_ULONG  fileSize,
    const TMWDTIME *pTimeOfCreation,
    DNPDEFS_FILE_PERMISSIONS permissions,
    const TMWTYPES_CHAR *pFileName);

  /* function: dnpdiag_showAuthHeader
   * purpose: display object header info
   * arguments:
   *  pSession - session from which this message originated
   *  pHdr - pointer to DNP Object Header to display
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL dnpdiag_showAuthHeader(
    TMWSESN *pSession,
    DNPAUTH_OBJECT_HEADER *pHdr); 

  /* function: dnpdiag_showAuthTestHeader
   * purpose: display object header info
   * arguments:
   *  pSession - session from which this message originated
   *  pHdr - pointer to DNP Object Header to display
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL dnpdiag_showAuthTestHeader(
    TMWSESN *pSession,
    DNPAUTH_OBJECT_HEADER *pHdr); 
    
  /* function: dnpdiag_showAuthTestHeader
   * purpose: display 
   * arguments:
   *  pSession - session from which this message originated 
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL dnpdiag_showAuthTestForce(
    TMWSESN *pSession); 

  /* function: dnpdiag_showAuthChallenge
   * purpose: display  
   * arguments:
   *  pSession - session from which this message originated
   *  elemIndex
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL dnpdiag_showAuthChallenge(
    TMWSESN *pSession,
    TMWTYPES_USHORT userNumber,
    TMWTYPES_ULONG sequenceNumber,
    TMWTYPES_UCHAR algorithm,
    TMWTYPES_UCHAR reason,
    TMWDIAG_ID direction);
 
  /* function: dnpdiag_showAuthReply
   * purpose: display  
   * arguments:
   *  pSession - session from which this message originated
   *  elemIndex
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL dnpdiag_showAuthReply(
    TMWSESN *pSession,
    TMWTYPES_USHORT userNumber,
    TMWTYPES_ULONG sequenceNumber,
    TMWTYPES_BOOL status,
    TMWDIAG_ID direction);

  /* function: dnpdiag_showAuthAggressive
   * purpose: display  
   * arguments:
   *  pSession - session from which this message originated
   *  elemIndex
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL dnpdiag_showAuthAggressive(
    TMWSESN *pSession,
    TMWTYPES_USHORT userNumber,
    TMWTYPES_ULONG sequenceNumber,
    TMWTYPES_ULONG expectedSequence,
    TMWTYPES_BOOL status,
    TMWDIAG_ID direction);

  /* function: dnpdiag_showAuthKeyStatReq
   * purpose: Display  
   * arguments: 
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL dnpdiag_showAuthKeyStatReq(
    TMWSESN *pSession,
    TMWTYPES_USHORT userNumber,
    TMWDIAG_ID direction);

  /* function: dnpdiag_showAuthKeyStatus
   * purpose: Display  
   * arguments: 
   * returns:
   *  void
   */
  void dnpdiag_showAuthKeyStatus(
    TMWSESN *pSession,  
    TMWTYPES_USHORT userNumber,
    TMWTYPES_ULONG sequenceNumber,
    TMWTYPES_UCHAR keyWrapAlgorithm,
    TMWTYPES_UCHAR status,
    TMWTYPES_UCHAR macAlgorithm,
    TMWDIAG_ID direction); 
    
  /* function: dnpdiag_showAuthKeyStatusStatus
   * purpose: Display  
   * arguments: 
   * returns:
   *  void
   */
  void dnpdiag_showAuthKeyStatusStatus(
    TMWSESN *pSession,  
    TMWTYPES_USHORT userNumber,
    TMWTYPES_UCHAR status);
 
  /* function: dnpdiag_showAuthKeyChange
   * purpose: Display  
   * arguments: 
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL dnpdiag_showAuthKeyChange(
    TMWSESN *pSession,
    TMWTYPES_USHORT userNumber,
    TMWTYPES_USHORT keyLength,
    TMWTYPES_ULONG sequenceNumber,
    TMWTYPES_ULONG expectedSequence,
    TMWTYPES_BOOL status,
    TMWDIAG_ID direction);
    
  /* function: dnpdiag_monitorAuthKeyChange
   * purpose: Display  
   * arguments: 
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL dnpdiag_monitorAuthKeyChange(
    TMWSESN *pSession,
    TMWTYPES_USHORT userNumber, 
    TMWTYPES_ULONG sequenceNumber);

  /* function: dnpdiag_showAuthError
   * purpose: display  
   * arguments:
   *  pSession - session from which this message originated
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL dnpdiag_showAuthError(
    TMWSESN *pSession,
    TMWTYPES_USHORT userNumber,
    TMWTYPES_USHORT assocId,
    TMWTYPES_ULONG sequenceNumber,
    TMWTYPES_UCHAR errorCode,
    TMWDTIME *pTimeStamp,
    TMWTYPES_CHAR *pErrorText,
    TMWTYPES_USHORT textLen,
    TMWTYPES_BOOL isEvent,
    TMWDIAG_ID direction);
 
  /* function: dnpdiag_authShowData
  * purpose: display
  * arguments:
  *  pSession - session from which this message originated
  * returns:
  *  void
  */
  void TMWDEFS_GLOBAL dnpdiag_authShowData(
    TMWSESN *pSession,
    TMWTYPES_UCHAR dataType,
    TMWTYPES_UCHAR *pData,
    TMWTYPES_ULONG length,
    TMWDIAG_ID direction);

  /* function: dnpdiag_authInitVector
   * purpose: display  
   * arguments:
   *  pSession - session from which this message originated
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL dnpdiag_authInitVector(
    TMWSESN *pSession,
    TMWTYPES_UCHAR *pData,
    TMWTYPES_USHORT length,
    TMWDIAG_ID direction);

  /* function: dnpdiag_authUserCert
   * purpose: display  
   * arguments:
   *  pSession - session from which this message originated
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL dnpdiag_authUserCert(
    TMWSESN *pSession,
    TMWTYPES_USHORT nameLength,
    TMWTYPES_UCHAR *pName,
    TMWTYPES_UCHAR keyChangeMethod,
    TMWTYPES_UCHAR certType, 
    TMWDIAG_ID direction);

  /* function: dnpdiag_authUserStatus
   * purpose: display  
   * arguments:
   *  pSession - session from which this message originated
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL dnpdiag_authUserStatus(
    TMWSESN *pSession,
    TMWTYPES_USHORT nameLength,
    TMWTYPES_UCHAR *pName,
    TMWTYPES_UCHAR  keyChangeMethod,
    TMWTYPES_UCHAR  operation,
    TMWTYPES_USHORT userRole,
    TMWTYPES_USHORT expires,
    TMWTYPES_ULONG  sequence,
    TMWTYPES_ULONG  lastSequence,
    TMWDIAG_ID direction);

 /* function: dnpdiag_authUpdKeyChgReq
   * purpose: display  
   * arguments:
   *  pSession - session from which this message originated
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL dnpdiag_authUpdKeyChgReq(
    TMWSESN *pSession,
    TMWTYPES_USHORT nameLength,
    TMWTYPES_UCHAR *pName,
    TMWTYPES_UCHAR  keyChangeMethod,
    TMWTYPES_USHORT challengeLength,
    TMWTYPES_UCHAR *pChallenge,
    TMWDIAG_ID direction);

 /* function: dnpdiag_authUpdKeyChange
   * purpose: display  
   * arguments:
   *  pSession - session from which this message originated
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL dnpdiag_authUpdKeyChange(
    TMWSESN *pSession,
    TMWTYPES_USHORT userNumber,
    TMWTYPES_ULONG sequenceNumber,
    TMWDIAG_ID direction);
  
 /* function: dnpdiag_authUpdKeyChgSig
   * purpose: display  
   * arguments:
   *  pSession - session from which this message originated
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL dnpdiag_authUpdKeyChgSig(
    TMWSESN *pSession,
    TMWDIAG_ID direction);

 /* function: dnpdiag_authUpdKeyChgConf
   * purpose: display  
   * arguments:
   *  pSession - session from which this message originated 
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL dnpdiag_authUpdKeyChgConf(
    TMWSESN *pSession,
    TMWTYPES_USHORT userNumber,
    TMWTYPES_BOOL status,
    TMWDIAG_ID direction);

 /* function: dnpdiag_authUpdKeyChgRply
   * purpose: display  
   * arguments:
   *  pSession - session from which this message originated
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL dnpdiag_authUpdKeyChgRply(
    TMWSESN *pSession,
    TMWTYPES_USHORT userNumber, 
    TMWTYPES_ULONG sequenceNumber, 
    TMWTYPES_UCHAR *pData, 
    TMWTYPES_USHORT length,
    TMWDIAG_ID direction);

  /* function: dnpdiag_linkAddressUnknown
   * purpose: Display a message indicating received link address was not for
   *  a currently configured session
   * arguments:
   *  pChannel - channel from which this message originated
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL dnpdiag_linkAddressUnknown(
    TMWCHNL *pChannel,
    TMWTYPES_USHORT srcAddress,
    TMWTYPES_USHORT destAddress);

  /* function: dnpdiag_error
   * purpose: Display error message
   * arguments:
   *  pChannel - channel from which this message originated
   *  pSession - session from which this message originated
   *  errorNumber - enum indicating what error message to display
   *  pExtraTextMsg - pointer to additional text to display with error msg
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL dnpdiag_errorMsg(
    TMWCHNL *pChannel,
    TMWSESN *pSession, 
    DNPDIAG_ERROR_ENUM errorNumber,
    TMWTYPES_CHAR *pExtraTextMsg);

  /* function: dnpdiag_errorMsgEnable
   * purpose: Enable/Disable specific error message output
   * arguments:
   *  errorNumber - enum indicating what error message
   *  enabled - TMWDEFS_TRUE if error message should be enabled
   *            TMWDEFS_FALSE if error message should be disabled
   * returns:
   *  void
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL dnpdiag_errorMsgEnable(
    DNPDIAG_ERROR_ENUM errorNumber,
    TMWTYPES_BOOL enabled);

  /* Function for converting Data Set descriptor code to string */
  const char * TMWDEFS_LOCAL dnpdiag_descrCodeToString(
    DNPDEFS_DATASET_DESCR_CODE code);

  /* Function for converting Data Set descriptor data type to string */
  const char * TMWDEFS_LOCAL dnpdiag_datasetTypeToString(
    DNPDEFS_DATASET_TYPE_CODE code);

  /* Function for converting security statistic index to string */
  const char * TMWDEFS_GLOBAL dnpdiag_securityStatisticString(
    TMWTYPES_USHORT index);
  
  void dnpdiag_displayOctets(
    TMWDIAG_ANLZ_ID *pAnlzId,
    TMWTYPES_UCHAR *pBuf,
    TMWTYPES_USHORT length,
    TMWTYPES_BOOL   indent);

#ifdef __cplusplus
}
#endif
#endif /* TMWCNFG_SUPPORT_DIAG */
#endif /* DNPDIAG_DEFINED */
