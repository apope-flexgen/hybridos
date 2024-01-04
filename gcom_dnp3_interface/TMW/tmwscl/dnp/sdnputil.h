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

/* file: sdnputil.h
 * description: This file is intended for internal SCL use only.
 *   Miscellaneous DNP Slave utilities
 */
#ifndef SDNPUTIL_DEFINED
#define SDNPUTIL_DEFINED

#include "tmwscl/dnp/sdnpsesn.h"
#include "tmwscl/dnp/sdnpdata.h"
#include "tmwscl/dnp/sdnpevnt.h"

struct SDNPUtilStaticDesc;

typedef SDNPSESN_READ_STATUS (*SDNPUTIL_READ_INTO_RESP)(
  TMWSESN *pSession, 
  TMWSESN_TX_DATA *pResponse, 
  TMWTYPES_USHORT messageIndex,
  TMWTYPES_USHORT pointNum, 
  void *pPoint,
  struct SDNPUtilStaticDesc *pDesc);

typedef struct SDNPUtilStaticDesc {
  TMWTYPES_UCHAR group;
  TMWTYPES_UCHAR readVariation;
  TMWTYPES_UCHAR variation;
  TMWTYPES_UCHAR sizeInBytes;
  
  /* true if value fits in less than an octet for this variation ie objs 1, 3, 10 */
  TMWTYPES_BOOL  readBits;  
  TMWTYPES_BOOL  alreadySwitchedVariation;
  SDNPDATA_QUANTITY_FUNC pQuantityFunc;
  SDNPDATA_GET_POINT_FUNC pGetPointFunc;
  SDNPDATA_IS_IN_CLASS0_FUNC pIsInClass0Func;
  SDNPUTIL_READ_INTO_RESP pReadIntoRespFunc;
} SDNPUTIL_STATIC_DESC;

#ifdef __cplusplus
extern "C" {
#endif

  /* function: sdnputil_nextSequenceNumber 
   * purpose: return the next valid application sequence number
   * arguments:
   *  sequenceNumber - current sequence number
   * returns:
   *  next valid sequence number
   */
  TMWTYPES_UCHAR TMWDEFS_GLOBAL sdnputil_nextSequenceNumber(
    TMWTYPES_UCHAR sequenceNumber);

  /* function: sdnputil_validateQualifier */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnputil_validateQualifier(
    DNPUTIL_OBJECT_HEADER *pObjHeader,
    TMWTYPES_USHORT numPoints);

  /* function: sdnputil_initReadStaticResponse */
  SDNPSESN_READ_STATUS TMWDEFS_GLOBAL sdnputil_initReadStaticResponse(
    TMWSESN_TX_DATA *pResponse, 
    DNPUTIL_OBJECT_HEADER *pObjHeader, 
    TMWTYPES_UCHAR group, 
    TMWTYPES_UCHAR variation,
    TMWTYPES_USHORT startPoint,
    TMWTYPES_USHORT dataSizeInBytes);
 
  /* function: sdnputil_readObj2or4Var3 */
  SDNPSESN_READ_STATUS TMWDEFS_CALLBACK sdnputil_readObj2or4Var3(
    SDNPEVNT **pEventPtr,
    TMWTYPES_USHORT *pNumEventsInResponse,
    SDNPEVNT_DESC *pDesc,
    TMWSESN_TX_DATA *pResponse,
    DNPUTIL_OBJECT_HEADER *pObjHeader,
    TMWDEFS_CLASS_MASK classMask);

  /* function: sdnputil_getObj2Or4Event 
   * purpose: get event to be sent from object 2 and object 4 queue.
   *  If BOTH object 2 and 4 are being read, they must be intermixed
   *  by order of occurrence in the response. If only 1 of these two
   *  object groups is being read, only events for that group should 
   *  be returned.
   * arguments: 
   * returns: 
   */
  void TMWDEFS_CALLBACK sdnputil_getObj2Or4Event(
    SDNPEVNT_DESC *pDesc,
    SDNPEVNT **pEventPtr, 
    TMWDEFS_CLASS_MASK classMask);
 
  /* function: sdnputil_checkForObjRead 
   * purpose: look ahead in request message to see if it contains a read of the
   *  specified object group
   * arguments: 
   * returns: 
   *  TMWDEFS_TRUE if found, and pHeader filled in.
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnputil_checkForObjRead(
    DNPUTIL_RX_MSG *pRequest,
    DNPDEFS_OBJ_GROUP_ID objectGroup,
    DNPUTIL_OBJECT_HEADER *pHeader);

  /* function: sdnputil_updateObjectHeader */
  void TMWDEFS_GLOBAL sdnputil_updateObjectHeader(
    TMWSESN_TX_DATA *pResponse,
    TMWTYPES_USHORT objectHeaderOffset,
    TMWTYPES_USHORT lastPointNumber,
    TMWTYPES_USHORT numberOfPoints);

  /* function: sdnputil_advanceToNextObjHeader */
   SDNPSESN_READ_STATUS TMWDEFS_GLOBAL sdnputil_advanceToNextObjHeader(    
    DNPUTIL_RX_MSG *pRequest, 
    DNPUTIL_OBJECT_HEADER *pObjHdr);

  /* function: sdnputil_readStatic */
  SDNPSESN_READ_STATUS TMWDEFS_GLOBAL sdnputil_readStatic(
    TMWSESN *pSession,
    DNPUTIL_RX_MSG *pRequest,
    TMWSESN_TX_DATA *pResponse, 
    DNPUTIL_OBJECT_HEADER *pObjHeader,
    SDNPSESN_QUAL qualifier,
    SDNPUTIL_STATIC_DESC *pDesc);

  /* function: sdnputil_updateIINEvents */
  void TMWDEFS_GLOBAL sdnputil_updateIINEvents(
    TMWSESN *pSession,
    TMWDEFS_CLASS_MASK classMask);

  /* function: sdnputil_validateSelOpQualifier
   * purpose: Validate qualifier for select or operate request
   * arguments:
   *  pSession - session request was received on
   *  pResponse - pointer to response message info
   *  pObjHeader - object header we are processing
   * returns:
   *  TMWDEFS_TRUE if qualifier is valid
   *  TMWDEFS_FALSE otherwise
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnputil_validateSelOpQualifier(
    TMWSESN *pSession,
    TMWSESN_TX_DATA *pResponse, 
    DNPUTIL_OBJECT_HEADER *pObjHeader);

  /* function: sdnputil_updateIINAfterSelOp */
  void TMWDEFS_GLOBAL sdnputil_updateIINAfterSelOp(
    TMWSESN *pSession, 
    TMWTYPES_UCHAR status);

  /* function: sdnputil_getDateTime */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL sdnputil_getDateTime(
    TMWSESN *pSession,
    TMWDTIME *pDateTime);

  /* function: sdnputil_parseApplHeader */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnputil_parseApplHeader(
    TMWSESN_RX_DATA *pRxFragment,
    SDNPSESN *pSDNPSession);

  /* function: sdnputil_validateAssignQualifier */
  TMWTYPES_BOOL sdnputil_validateAssignQualifier(
    DNPUTIL_OBJECT_HEADER *pObjHeader,
    TMWTYPES_USHORT numPoints);

#ifdef __cplusplus
}
#endif
#endif /* SDNPUTIL_DEFINED */
