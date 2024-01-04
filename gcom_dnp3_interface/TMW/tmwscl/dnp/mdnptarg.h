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

/* file: mdnptarg.h
 * description: Definitions of MDNP target layer functions
 */
#ifndef MDNPTARG_DEFINED
#define MDNPTARG_DEFINED

#include "tmwtargcnfg.h"

#if TMWTARG_SUPPORT_DNPFILEIO
#include "tmwscl/utils/tmwtarg.h"
#include "tmwscl/dnp/dnpdefs.h"


#ifdef __cplusplus
extern "C" {
#endif

  /* function: mdnptarg_openLocalFile
   * purpose: Open a file on the local file system
   * arguments:
   *  pLocalFileName - name of local file to open
   *  fileMode - simple file(1) or directory(0)
   * returns:
   *  FILE * - NULL on failure, else successful
   */
  FILE *mdnptarg_openLocalFile(
    const TMWTYPES_CHAR *pLocalFileName, 
    DNPDEFS_FILE_MODE fileMode);

  /* function: mdnptarg_closeLocalFile
   * purpose: Close a file on the local file system
   * arguments:
   *  pFileHandle - local file handle returned from mdnptarg_openLocalFile
   * returns:
   *  0 if successful
   */
  TMWTYPES_INT mdnptarg_closeLocalFile(
    FILE *pFileHandle);

  /* function: mdnptarg_getLocalFileInfo
   * purpose: Get size and time of creation for a file on the local file system
   * arguments:
   *  pLocalFileName - name of local file to get info for 
   *  pFileSize - size of file or 0xffffffff if not known
   *  pDateTime - time of creation for file or Jan 1 1970 if not known.
   * returns:
   *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
   */
  TMWTYPES_BOOL mdnptarg_getLocalFileInfo(
    const TMWTYPES_CHAR *pLocalFileName, 
    TMWTYPES_ULONG *pFileSize,
    TMWDTIME *pDateTime); 

  /* function: mdnptarg_readLocalFile
   * purpose: Read data from a local file and place it in a buffer
   * arguments:
   *  pLocalFileHandle - local file handle
   *  pBuf - buffer to fill
   *  bufSize - number of bytes to put in pBuf
   *  pLastBlock - fill this in to indicate last block
   * returns:
   *  TMWTYPES_USHORT - number of bytes read
   */
  TMWTYPES_USHORT mdnptarg_readLocalFile(
    FILE *pFileHandle,
    TMWTYPES_UCHAR *pBuf,
    TMWTYPES_USHORT bufSize,
    TMWTYPES_BOOL *pLastBlock);

  /* function: mdnptarg_storeFileData
   * purpose: Store a file on the local file system
   * arguments:
   *  pFileHandle - local file handle returned from mdnptarg_openLocalFile
   *  nBytesInBlockData - number of bytes in pBlockData
   *  pBlockData - the data for this block or TMWDEFS_NULL if none.
   * returns:
   *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
   */
  TMWTYPES_BOOL mdnptarg_storeFileData(
    FILE *pFileHandle,
    TMWTYPES_USHORT nBytesInBlockData,
    const TMWTYPES_UCHAR *pBlockData);

#ifdef __cplusplus
}
#endif

#endif /* TMWTARG_SUPPORT_DNPFILEIO */

#endif /* MDNPTARG_DEFINED */
