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

/* file: tmwmsim.h
 * description: Simulates a master database. More specifically simulates 
 *  data points in a sparsly populated doubly linked list (i.e. point
 *  numbers are random and may contain gaps). Points are added to the
 *  database whenever a data point is requested that does not already
 *  exist.
 */
#ifndef TMWMSIM_DEFINED
#define TMWMSIM_DEFINED

#include "tmwscl/utils/tmwdefs.h"
#include "tmwscl/utils/tmwcnfg.h"
#include "tmwscl/utils/tmwsim.h"
#include "tmwscl/utils/tmwdlist.h"

#if TMWCNFG_USE_SIMULATED_DB

#ifdef __cplusplus
extern "C" {
#endif
  
  TMWDEFS_SCL_API TMWSIM_POINT * TMWDEFS_GLOBAL tmwmsim_getBinary(
    void *pHandle,
    TMWSIM_TABLE_HEAD *pTableHead,
    TMWTYPES_ULONG pointNum, 
    TMWTYPES_BOOL *pValue, 
    TMWTYPES_UCHAR *pFlags);
  
  /* function: tmwmsim_getIntegratedTotal */
  TMWSIM_POINT * TMWDEFS_GLOBAL tmwmsim_getIntegratedTotal(
    void *pHandle,
    TMWSIM_TABLE_HEAD *pTableHead,
    TMWTYPES_ULONG pointNum, 
    TMWTYPES_LONG *pValue, 
    TMWTYPES_UCHAR *pFlags);

  /* function: tmwmsim_tableGetBinaryByIndex */
  TMWDEFS_SCL_API void * TMWDEFS_GLOBAL tmwmsim_getBinaryByIndex(
    TMWSIM_TABLE_HEAD *pTableHead,
    TMWTYPES_USHORT pointIndex, 
    TMWTYPES_BOOL *pValue, 
    TMWTYPES_UCHAR *pFlags);

  TMWDEFS_SCL_API void * TMWDEFS_GLOBAL tmwmsim_setBinary(
    void *pHandle,
    TMWSIM_TABLE_HEAD *pTableHead, 
    TMWTYPES_ULONG pointNum, 
    TMWTYPES_BOOL value, 
    TMWTYPES_UCHAR flags);

  TMWDEFS_SCL_API void * TMWDEFS_GLOBAL tmwmsim_getDoubleBinary(
    void *pHandle,
    TMWSIM_TABLE_HEAD *pTableHead, 
    TMWTYPES_ULONG pointNum, 
    TMWDEFS_DPI_TYPE *pValue, 
    TMWTYPES_UCHAR *pFlags);

  TMWDEFS_SCL_API TMWSIM_POINT * TMWDEFS_GLOBAL tmwmsim_getCounter(
    void *pHandle,
    TMWSIM_TABLE_HEAD *pTableHead,
    TMWTYPES_ULONG pointNum, 
    TMWTYPES_ULONG *pValue, 
    TMWTYPES_UCHAR *pFlags);

  TMWDEFS_SCL_API void * TMWDEFS_GLOBAL tmwmsim_getCounterByIndex(
    TMWSIM_TABLE_HEAD *pTableHead, 
    TMWTYPES_USHORT pointIndex, 
    TMWTYPES_ULONG *pValue, 
    TMWTYPES_UCHAR *pFlags);

  TMWDEFS_SCL_API TMWSIM_POINT * TMWDEFS_GLOBAL tmwmsim_getAnalog(
    void *pHandle,
    TMWSIM_TABLE_HEAD *pTableHead, 
    TMWTYPES_ULONG pointNum, 
    TMWSIM_DATA_TYPE *pValue, 
    TMWTYPES_UCHAR *pFlags);

  TMWDEFS_SCL_API void * TMWDEFS_GLOBAL tmwmsim_getAnalogByIndex(
    TMWSIM_TABLE_HEAD *pTableHead, 
    TMWTYPES_USHORT pointIndex, 
    TMWSIM_DATA_TYPE *pValue, 
    TMWTYPES_UCHAR *pFlags);

  void * TMWDEFS_GLOBAL tmwmsim_setAnalog(
    void *pHandle,
    TMWSIM_TABLE_HEAD *pTableHead, 
    TMWTYPES_ULONG pointNum, 
    TMWSIM_DATA_TYPE value, 
    TMWTYPES_UCHAR flags);

  TMWDEFS_SCL_API void * TMWDEFS_GLOBAL tmwmsim_getBitstring(
    void *pHandle,
    TMWSIM_TABLE_HEAD *pTableHead, 
    TMWTYPES_ULONG pointNum, 
    TMWTYPES_ULONG *pValue, 
    TMWTYPES_UCHAR *pFlags,
    TMWTYPES_USHORT *pMiscTime);

  void * TMWDEFS_GLOBAL tmwmsim_setBitstring(
    void *pHandle,
    TMWSIM_TABLE_HEAD *pTableHead, 
    TMWTYPES_ULONG pointNum, 
    TMWTYPES_ULONG value, 
    TMWTYPES_UCHAR flags,
    TMWTYPES_USHORT miscTime);

  TMWDEFS_SCL_API TMWSIM_POINT * TMWDEFS_GLOBAL tmwmsim_getString(
    void *pHandle,
    TMWSIM_TABLE_HEAD *pTableHead,
    TMWTYPES_ULONG pointNum,
    TMWTYPES_UCHAR maxLen,
    TMWTYPES_UCHAR *pStrBuf,
    TMWTYPES_UCHAR *pStrLen);

  TMWDEFS_SCL_API void * TMWDEFS_GLOBAL tmwmsim_getStringByIndex(
    TMWSIM_TABLE_HEAD *pTableHead,
    TMWTYPES_USHORT pointIndex,
    TMWTYPES_UCHAR maxLen,
    TMWTYPES_UCHAR *pStrBuf,
    TMWTYPES_UCHAR *pStrLen);

#ifdef __cplusplus
}
#endif
#endif
#endif /* TMWMSIM_DEFINED */
