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

/* file: tmwmsim.c
 * description: Simulates a generic master database.
 */
#include "tmwscl/utils/tmwtarg.h"
#include "tmwscl/utils/tmwcnfg.h"
#include "tmwscl/utils/tmwsim.h"
#include "tmwscl/utils/tmwmsim.h"
#include "tmwscl/utils/tmwmem.h"

#if TMWCNFG_USE_SIMULATED_DB

/* function: tmwmsim_getBinary */
TMWSIM_POINT * TMWDEFS_GLOBAL tmwmsim_getBinary(
  void *pHandle,
  TMWSIM_TABLE_HEAD *pTableHead,
  TMWTYPES_ULONG pointNum, 
  TMWTYPES_BOOL *pValue, 
  TMWTYPES_UCHAR *pFlags)
{
  TMWSIM_POINT *pPoint = tmwsim_tableFindPoint(pTableHead, pointNum);
  if(pPoint == TMWDEFS_NULL)
  {
    pPoint = tmwsim_tableAdd(pTableHead, pointNum);
    if(pPoint != TMWDEFS_NULL)
    {
      tmwsim_initPoint(pPoint, pHandle, pointNum, TMWSIM_TYPE_BINARY);
    }
  }
  if(pPoint != TMWDEFS_NULL)
  {
    *pFlags = tmwsim_getFlags(pPoint);
    *pValue = tmwsim_getBinaryValue(pPoint);
  }

  return(pPoint);
}

/* function: tmwmsim_getBinary */
TMWSIM_POINT * TMWDEFS_GLOBAL tmwmsim_getIntegratedTotal(
  void *pHandle,
  TMWSIM_TABLE_HEAD *pTableHead,
  TMWTYPES_ULONG pointNum, 
  TMWTYPES_LONG *pValue, 
  TMWTYPES_UCHAR *pFlags)
{
  TMWSIM_POINT *pPoint = tmwsim_tableFindPoint(pTableHead, pointNum);
  if(pPoint == TMWDEFS_NULL)
  {
    pPoint = tmwsim_tableAdd(pTableHead, pointNum);
    if(pPoint != TMWDEFS_NULL)
    {
      tmwsim_initPoint(pPoint, pHandle, pointNum, TMWSIM_TYPE_INTEGRATED_TOTAL);
    }
  }
  if(pPoint != TMWDEFS_NULL)
  {
    *pFlags = tmwsim_getFlags(pPoint);
    *pValue = tmwsim_getIntegratedTotalValue(pPoint);
  }

  return(pPoint);
}

/* function: tmwmsim_getBinaryByIndex */
void * TMWDEFS_GLOBAL tmwmsim_getBinaryByIndex(
  TMWSIM_TABLE_HEAD *pTableHead,
  TMWTYPES_USHORT pointIndex, 
  TMWTYPES_BOOL *pValue, 
  TMWTYPES_UCHAR *pFlags)
{
  TMWSIM_POINT *pPoint = tmwsim_tableFindPointByIndex(pTableHead, pointIndex);
  if(pPoint != TMWDEFS_NULL)
  {
    *pFlags = tmwsim_getFlags(pPoint);
    *pValue = tmwsim_getBinaryValue(pPoint);
  }

  return(pPoint);
}

/* function: tmwmsim_setBinary */
void * TMWDEFS_GLOBAL tmwmsim_setBinary(
  void *pHandle,
  TMWSIM_TABLE_HEAD *pTableHead,
  TMWTYPES_ULONG pointNum, 
  TMWTYPES_BOOL value, 
  TMWTYPES_UCHAR flags)
{ 
  TMWSIM_POINT *pPoint = tmwsim_tableFindPoint(pTableHead, pointNum);
  if(pPoint == TMWDEFS_NULL)
  {
    pPoint = tmwsim_tableAdd(pTableHead, pointNum);
    if(pPoint != TMWDEFS_NULL)
    {
      tmwsim_initPoint(pPoint, pHandle, pointNum, TMWSIM_TYPE_BINARY);
    }
  }
  if(pPoint != TMWDEFS_NULL)
  {
    tmwsim_setFlags(pPoint, flags, TMWDEFS_CHANGE_REMOTE_OP);
    tmwsim_setBinaryValue(pPoint, value, TMWDEFS_CHANGE_REMOTE_OP);
  }

  return(pPoint);
}

/* function: tmwmsim_getDoubleBinary */
void * TMWDEFS_GLOBAL tmwmsim_getDoubleBinary(
  void *pHandle,
  TMWSIM_TABLE_HEAD *pTableHead, 
  TMWTYPES_ULONG pointNum, 
  TMWDEFS_DPI_TYPE *pValue, 
  TMWTYPES_UCHAR *pFlags)
{
  TMWSIM_POINT *pPoint = tmwsim_tableFindPoint(pTableHead, pointNum);
  if(pPoint == TMWDEFS_NULL)
  {
    pPoint = tmwsim_tableAdd(pTableHead, pointNum);
    if(pPoint != TMWDEFS_NULL)
    {
      tmwsim_initPoint(pPoint, pHandle, pointNum, TMWSIM_TYPE_DOUBLE_BINARY);
    }
  } 

  if(pPoint != TMWDEFS_NULL)
  {
    *pFlags = tmwsim_getFlags(pPoint);
    *pValue = tmwsim_getDoubleBinaryValue(pPoint);
  }

  return(pPoint);
}

/* function: tmwmsim_getCounter */
TMWSIM_POINT * TMWDEFS_GLOBAL tmwmsim_getCounter(
  void *pHandle,
  TMWSIM_TABLE_HEAD *pTableHead,
  TMWTYPES_ULONG pointNum, 
  TMWTYPES_ULONG *pValue, 
  TMWTYPES_UCHAR *pFlags)
{
  TMWSIM_POINT *pPoint = tmwsim_tableFindPoint(pTableHead, pointNum);
  if(pPoint == TMWDEFS_NULL)
  {
    pPoint = tmwsim_tableAdd(pTableHead, pointNum);
    if(pPoint != TMWDEFS_NULL)
    {
      tmwsim_initPoint(pPoint, pHandle, pointNum, TMWSIM_TYPE_COUNTER);
    }
  }
  if(pPoint != TMWDEFS_NULL)
  {
    *pFlags = tmwsim_getFlags(pPoint);
    *pValue = tmwsim_getCounterValue(pPoint);
  }

  return(pPoint);
}

/* function: tmwmsim_getCounterByIndex */
void * TMWDEFS_GLOBAL tmwmsim_getCounterByIndex(
  TMWSIM_TABLE_HEAD *pTableHead, 
  TMWTYPES_USHORT pointIndex, 
  TMWTYPES_ULONG *pValue, 
  TMWTYPES_UCHAR *pFlags)
{
  TMWSIM_POINT *pPoint = tmwsim_tableFindPointByIndex(pTableHead, pointIndex);
  if(pPoint != TMWDEFS_NULL)
  {
    *pFlags = tmwsim_getFlags(pPoint);
    *pValue = tmwsim_getCounterValue(pPoint);
  }

  return(pPoint);
}

/* function: tmwmsim_getAnalog */
TMWSIM_POINT * TMWDEFS_GLOBAL tmwmsim_getAnalog(
  void *pHandle,
  TMWSIM_TABLE_HEAD *pTableHead, 
  TMWTYPES_ULONG pointNum, 
  TMWSIM_DATA_TYPE *pValue, 
  TMWTYPES_UCHAR *pFlags)
{
  TMWSIM_POINT *pPoint = tmwsim_tableFindPoint(pTableHead, pointNum);
  if(pPoint == TMWDEFS_NULL)
  {
    pPoint = tmwsim_tableAdd(pTableHead, pointNum);
    if(pPoint != TMWDEFS_NULL)
    {
      tmwsim_initPoint(pPoint, pHandle, pointNum, TMWSIM_TYPE_ANALOG);
    }
  }

  if(pPoint != TMWDEFS_NULL)
  {
    *pFlags = tmwsim_getFlags(pPoint);
    *pValue = tmwsim_getAnalogValue(pPoint);
  }

  return(pPoint);
}

/* function: tmwmsim_getAnalogByIndex */
void * TMWDEFS_GLOBAL tmwmsim_getAnalogByIndex(
  TMWSIM_TABLE_HEAD *pTableHead, 
  TMWTYPES_USHORT pointIndex, 
  TMWSIM_DATA_TYPE *pValue, 
  TMWTYPES_UCHAR *pFlags)
{
  TMWSIM_POINT *pPoint = tmwsim_tableFindPointByIndex(pTableHead, pointIndex);
  if(pPoint != TMWDEFS_NULL)
  {
    *pFlags = tmwsim_getFlags(pPoint);
    *pValue = tmwsim_getAnalogValue(pPoint);
  }

  return(pPoint);
}

/* function: tmwmsim_setAnalog */
void * TMWDEFS_GLOBAL tmwmsim_setAnalog(
  void *pHandle,
  TMWSIM_TABLE_HEAD *pTableHead, 
  TMWTYPES_ULONG pointNum, 
  TMWSIM_DATA_TYPE value, 
  TMWTYPES_UCHAR flags)
{ 
  TMWSIM_POINT *pPoint = tmwsim_tableFindPoint(pTableHead, pointNum);
  if(pPoint == TMWDEFS_NULL)
  {
    pPoint = tmwsim_tableAdd(pTableHead, pointNum);
    if(pPoint != TMWDEFS_NULL)
    {
      tmwsim_initPoint(pPoint, pHandle, pointNum, TMWSIM_TYPE_ANALOG);
    }
  }

  if(pPoint != TMWDEFS_NULL)
  {
    tmwsim_setFlags(pPoint, flags, TMWDEFS_CHANGE_REMOTE_OP);
    tmwsim_setAnalogValue(pPoint, value, TMWDEFS_CHANGE_REMOTE_OP);
  }

  return(pPoint);
}


/* function: tmwmsim_getBitstring */
void * TMWDEFS_GLOBAL tmwmsim_getBitstring(
  void *pHandle,
  TMWSIM_TABLE_HEAD *pTableHead, 
  TMWTYPES_ULONG pointNum, 
  TMWTYPES_ULONG *pValue, 
  TMWTYPES_UCHAR *pFlags,
  TMWTYPES_USHORT *pMiscTime)
{
  TMWSIM_POINT *pPoint = tmwsim_tableFindPoint(pTableHead, pointNum);
  if(pPoint == TMWDEFS_NULL)
  {
    pPoint = tmwsim_tableAdd(pTableHead, pointNum);
    if(pPoint != TMWDEFS_NULL)
    {
      tmwsim_initPoint(pPoint, pHandle, pointNum, TMWSIM_TYPE_BITSTRING);
    }
  }

  if(pPoint != TMWDEFS_NULL)
  {
    *pFlags = tmwsim_getFlags(pPoint);
    *pValue = tmwsim_getBitstringValue(pPoint);
    if(pMiscTime != TMWDEFS_NULL)
    {
      *pMiscTime = tmwsim_getBitstringTime(pPoint);
    }
  }

  return(pPoint);
}

/* function: tmwmsim_setBitstring */
void * TMWDEFS_GLOBAL tmwmsim_setBitstring(
  void *pHandle,
  TMWSIM_TABLE_HEAD *pTableHead, 
  TMWTYPES_ULONG pointNum, 
  TMWTYPES_ULONG value, 
  TMWTYPES_UCHAR flags,
  TMWTYPES_USHORT miscTime)
{
  TMWSIM_POINT *pPoint = tmwsim_tableFindPoint(pTableHead, pointNum);
  if(pPoint == TMWDEFS_NULL)
  {
    pPoint = tmwsim_tableAdd(pTableHead, pointNum);
    if(pPoint != TMWDEFS_NULL)
    {
      tmwsim_initPoint(pPoint, pHandle, pointNum, TMWSIM_TYPE_BITSTRING);
    }
  }

  if(pPoint != TMWDEFS_NULL)
  {
    tmwsim_setFlags(pPoint, flags, TMWDEFS_CHANGE_REMOTE_OP);
    tmwsim_setBitstringValue(pPoint, value, TMWDEFS_CHANGE_REMOTE_OP);
    tmwsim_setBitstringTime(pPoint, miscTime);
  }

  return(pPoint);
}

/* function: tmwmsim_getString */
TMWSIM_POINT * TMWDEFS_GLOBAL tmwmsim_getString(
  void *pHandle,
  TMWSIM_TABLE_HEAD *pTableHead,
  TMWTYPES_ULONG pointNum,
  TMWTYPES_UCHAR maxLen,
  TMWTYPES_UCHAR *pStrBuf,
  TMWTYPES_UCHAR *pStrLen)
{
  TMWSIM_POINT *pPoint = tmwsim_tableFindPoint(pTableHead, pointNum);
  if(pPoint == TMWDEFS_NULL)
  {
    pPoint = tmwsim_tableAdd(pTableHead, pointNum);
    if(pPoint != TMWDEFS_NULL)
    {
      tmwsim_initPoint(pPoint, pHandle, pointNum, TMWSIM_TYPE_STRING);
    }
  }
 
  if(pPoint != TMWDEFS_NULL)
  {
    tmwsim_getStringValue(pPoint, maxLen, pStrBuf, pStrLen);
  }

  return(pPoint);
}

/* function: tmwmsim_getStringByIndex */
void * TMWDEFS_GLOBAL tmwmsim_getStringByIndex(
  TMWSIM_TABLE_HEAD *pTableHead,
  TMWTYPES_USHORT pointIndex,
  TMWTYPES_UCHAR maxLen,
  TMWTYPES_UCHAR *pStrBuf,
  TMWTYPES_UCHAR *pStrLen)
{
  TMWSIM_POINT *pPoint = tmwsim_tableFindPointByIndex(pTableHead, pointIndex);
  if(pPoint != TMWDEFS_NULL)
  {
    tmwsim_getStringValue(pPoint, maxLen, pStrBuf, pStrLen);
  }

  return(pPoint);
}
#endif /* TMWCNFG_USE_SIMULATED_DB */
