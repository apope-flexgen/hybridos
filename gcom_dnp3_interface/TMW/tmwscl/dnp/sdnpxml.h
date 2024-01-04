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

/* file: sdnpxml.h
 * description: This file is intended for internal SCL use only.
 *  Slave DNP XML support for old proprietary device profile xml format
 *  This is used by the Test Harness for save/restore
 */
#ifndef SDNPXML_DEFINED
#define SDNPXML_DEFINED

#include "tmwscl/dnp/sdnpcnfg.h"
#include "tmwscl/dnp/sdnpsesn.h"

 /* Special value to be written when associated input point is 0.
  * Otherwise all points in workspace without an associated Point
  * would appear to have an associated input point 0.
  */
#define SDNPXML_ASSOC_ZERO ((TMWTYPES_ULONG)-2)

#if SDNPDATA_SUPPORT_XML

typedef enum SDNPXMLSaveStateEnum {
  SDNPXML_SAVE_FAILED=0,
  SDNPXML_SAVE_NOT_DONE,
  SDNPXML_SAVE_DONE,
  SDNPXML_SAVE_CONTINUE,
  SDNPXML_SAVE_HEADER,
  SDNPXML_SAVE_DEVICE_ATTR,
  SDNPXML_SAVE_DEVICE_ATTR_TAIL,
  SDNPXML_SAVE_BIN_IN,
  SDNPXML_SAVE_BIN_IN_TAIL,
  SDNPXML_SAVE_DBL_IN,
  SDNPXML_SAVE_DBL_IN_TAIL,
  SDNPXML_SAVE_BIN_OUT,
  SDNPXML_SAVE_BIN_OUT_TAIL,
  SDNPXML_SAVE_BIN_OUT_STATUS,
  SDNPXML_SAVE_BIN_OUT_STATUS_TAIL,
  SDNPXML_SAVE_BIN_COUNTER,
  SDNPXML_SAVE_BIN_COUNTER_TAIL,
  SDNPXML_SAVE_FROZEN_COUNTER,
  SDNPXML_SAVE_FROZEN_COUNTER_TAIL,
  SDNPXML_SAVE_ANALOG_IN,
  SDNPXML_SAVE_ANALOG_IN_TAIL,
  SDNPXML_SAVE_FRZNANALOG_IN,
  SDNPXML_SAVE_FRZNANALOG_IN_TAIL,
  SDNPXML_SAVE_ANALOG_IN_DB,
  SDNPXML_SAVE_ANALOG_IN_DB_TAIL,
  SDNPXML_SAVE_ANALOG_OUT,
  SDNPXML_SAVE_ANALOG_OUT_TAIL,
  SDNPXML_SAVE_ANALOG_OUT_STATUS,
  SDNPXML_SAVE_ANALOG_OUT_STATUS_TAIL,
  SDNPXML_SAVE_INDEXED_TIME,
  SDNPXML_SAVE_INDEXED_TIME_TAIL,
  SDNPXML_SAVE_DATASET_PROTO,
  SDNPXML_SAVE_DATASET_PROTO_TAIL,
  SDNPXML_SAVE_DATASET_DESCR,
  SDNPXML_SAVE_DATASET_DESCR_TAIL,
  SDNPXML_SAVE_DATASET_DATA,
  SDNPXML_SAVE_DATASET_DATA_TAIL,
  SDNPXML_SAVE_STRING,
  SDNPXML_SAVE_STRING_TAIL,
  SDNPXML_SAVE_VTERM,
  SDNPXML_SAVE_VTERM_TAIL,
  SDNPXML_SAVE_AUTHSECSTAT,
  SDNPXML_SAVE_AUTHSECSTAT_TAIL,
  SDNPXML_SAVE_TAIL,
} SDNPXML_SAVE_STATE;

#ifdef __cplusplus
extern "C" {
#endif

#if SDNPDATA_SUPPORT_XML
  TMWDEFS_SCL_API SDNPXML_SAVE_STATE TMWDEFS_GLOBAL sdnpxml_saveDatabaseStatic(
    TMWSESN *pSession, 
    TMWTYPES_CHAR *result,
    TMWTYPES_ULONG maxLen,
    TMWTYPES_ULONG *bufLen);

  void TMWDEFS_GLOBAL sdnpxml_saveDatabaseStaticInit(
    TMWSESN *pSession);

  TMWTYPES_ULONG TMWDEFS_GLOBAL sdnpxml_saveDatabaseStaticGetSize(
    TMWSESN *pSession);
#endif

#ifdef __cplusplus
}
#endif
#endif /* SDNPDATA_SUPPORT_XML */
#endif /* SDNPXML_DEFINED */
