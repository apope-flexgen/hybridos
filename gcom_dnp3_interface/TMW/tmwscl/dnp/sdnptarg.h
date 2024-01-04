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

/* file: sdnptarg.h
 * description: Definitions of SDNP target layer functions to support file transfer
 */
#ifndef SDNPTARG_DEFINED
#define SDNPTARG_DEFINED

#include "tmwtargcnfg.h"

#if TMWTARG_SUPPORT_DNPFILEIO
#include "tmwscl/utils/tmwtarg.h"
#include "tmwscl/dnp/dnpdefs.h"

#define  SDNPTARG_FILE_HANDLE 0x12345678 /* SDNP File handle is 32 bits. */

#ifdef __cplusplus
extern "C" {
#endif

/* function: sdnptarg_getFileInfo
   * purpose: return information about file or directory
   *  This is used to build a response to a FC 28 Get File Info request.
   * arguments:
   *  pFilename - null terminated name of file to get info from
   *  pType - return type of file
   *    DNPDEFS_FILE_TYPE_SIMPLE  or
   *    DNPDEFS_FILE_TYPE_DIRECTORY
   *  pSize - return size of simple file in bytes 
   *    If this was directory, size is the number of files contained in 
   *    the directory excluding links to itself and parent, according to
   *    DNP3 Specification, Application Layer Draft H 6.7.4
   *  pTimeOfCreation - return file or directory time of creation
   *  pPermissions - return file or directory permissions
   *   see DNPDEFS_FILE_PERMISSIONS in dnpdefs.h
   * returns:
   *  DNPDEFS_FILE_CMD_STAT_SUCCESS if successful
   *  DNPDEFS_FILE_CMD_STAT_ASYNC if the SCL should send a NULL response
   *   and later send the response as an event. The target database can call
   *   sdnpo070_fileCmdComplete(); or the SCL will call sdnpdata_getFileInfo periodically
   *  DNPDEFS_FILE_CMD_STAT_NOT_FOUND If file does not exist
   *  Other Appropriate DNPDEFS_FILE_CMD_STAT values
   *   DNPDEFS_FILE_CMD_STAT_LOST_COMM   
   *   DNPDEFS_FILE_CMD_STAT_MISC        
   */
  DNPDEFS_FILE_CMD_STAT TMWDEFS_GLOBAL sdnptarg_getFileInfo(
    TMWTYPES_CHAR *pFilename,
    DNPDEFS_FILE_TYPE *pType,
    TMWTYPES_ULONG *pSize,
    TMWDTIME *pTimeOfCreation,
    DNPDEFS_FILE_PERMISSIONS *pPermissions);

  /* function: sdnptarg_readFileInfo
   * purpose: return information about file or subdirectory
   *  This is used to build a response to a FC 1 read request on a directory.
   *  This function will be called multiple times. It should set pLast to
   *  TRUE when the last entry in a directory is being read.
   * arguments:
   *  handle - DNP file handle returned from previous call to sdnpdata_openFile
   *  maxNameSize - max length of pFileName to be returned 
   *    If name won't fit, return DNPDEFS_FILE_TFER_STAT_MISC and SCL
   *    will read this file info again on next request.
   *  pFilename - return null terminated name of file or subdirectory entry
   *  pLast - return TMWDEFS_TRUE if this is the last block in this file
   *  pType - return type of file
   *    DNPDEFS_FILE_TYPE_SIMPLE  or
   *    DNPDEFS_FILE_TYPE_DIRECTORY
   *  pSize - return size of simple file in bytes  
   *    If this was directory, size is the number of files contained in 
   *    the directory excluding links to itself and parent, according to
   *    DNP3 Specification, Application Layer Draft H 6.7.4
   *  pTimeOfCreation - return file time of creation
   *  pPermissions - return file permissions
   * returns:
   *  DNPDEFS_FILE_TFER_STAT_SUCCESS if successful
   *  DNPDEFS_FILE_TFER_STAT_ASYNC if the SCL should send a NULL response
   *   and later send the response as an event. The target database can call
   *   sdnpo070_fileCmdComplete(); or the SCL will call sdnpdata_readFileInfo 
   *   periodically. When the SCL is reading an entire directory this will be called
   *   repeatedly. The response will not be sent until either all entries or 
   *   enough to fill a response are read.
   *  Appropriate DNPDEFS_FILE_TFER_STAT values on error.
   *   DNPDEFS_FILE_TFER_STAT_INV_HANDLE  
   *   DNPDEFS_FILE_TFER_STAT_LOST_COMM 
   *   DNPDEFS_FILE_TFER_STAT_MISC        
   */
  DNPDEFS_FILE_TFER_STAT TMWDEFS_GLOBAL sdnptarg_readFileInfo(
    TMWTYPES_ULONG handle, 
    TMWTYPES_USHORT maxNameSize,
    TMWTYPES_CHAR *pName,
    TMWTYPES_BOOL *pLast,
    DNPDEFS_FILE_TYPE *pType,
    TMWTYPES_ULONG *pSize, 
    TMWDTIME *pTimeOfCreation, 
    DNPDEFS_FILE_PERMISSIONS *pPermissions);

  /* function: sdnptarg_deleteFile
   * purpose: delete the requested file
   * arguments:
   *  pFilename - null terminated name of file to delete
   * returns:
   *  DNPDEFS_FILE_CMD_STAT_SUCCESS if successful
   *   return DNPDEFS_FILE_CMD_STAT_LOCKED if the specified file is open, 
   *    and do not delete the file.
   *  DNPDEFS_FILE_CMD_STAT_ASYNC if the SCL should send a NULL response
   *   and later send the response as an event. The target database can call
   *   sdnpo070_fileCmdComplete(); or the SCL will call sdnpdata_deleteFile periodically
   *  Other appropriate DNPDEFS_FILE_CMD_STAT values.
   *   DNPDEFS_FILE_CMD_STAT_DENIED    
   *   DNPDEFS_FILE_CMD_STAT_NOT_FOUND   
   *   DNPDEFS_FILE_CMD_STAT_LOST_COMM   
   *   DNPDEFS_FILE_CMD_STAT_MISC        
   */
  DNPDEFS_FILE_CMD_STAT TMWDEFS_GLOBAL sdnptarg_deleteFile(
    TMWTYPES_CHAR *pFilename);

  /* function: sdnpdata_openFile
   * purpose: open specified file
   * arguments:
   *  pSession - pointer to session request was received on
   *  pFilename - null terminated name of file to open
   *  authKey - authentication key for this request
   *  mode - access mode for file  
   *   DNPDEFS_FILE_MODE_READ     
   *   DNPDEFS_FILE_MODE_WRITE  
   *   DNPDEFS_FILE_MODE_APPEND  
   *  pMaxBlockSize - pointer to proposed maximum block size for this file
   *   this function can modify this to return a reduced maxBlockSize
   *  pPermissions - permissions to give new file if created
   *    NOTE: this data is passed to the database from the SCL. It is NOT
   *          retrieved from the database, it is a pointer to a USHORT
   *          for backward compatibility reasons.
   *  pTimeOfCreation - return time of creation for file
   *  pHandle - return DNP file handle used in DNP g70/v4 and g70/v5 Messages, 
   *   non-zero value if successful. 
   *  pSize - return file size. 
   *   If file was opened for reading, size should indicate the total number 
   *    of octets in the file that was opened. If this is a directory, size
   *    would be the number of octets that would be transferred by the read
   *    requests. This is (the number of files contained in the directory * 20)  
   *    + (sum of the lengths of the names of all of the files in the directory).
   *   If file was opened for writing size SCL will set size to zero.
   *  pType - return type of file
   *   DNPDEFS_FILE_TYPE_DIRECTORY  
   *   DNPDEFS_FILE_TYPE_SIMPLE    
   * returns:
   *  DNPDEFS_FILE_CMD_STAT_SUCCESS if successful
   *  DNPDEFS_FILE_CMD_STAT_ASYNC if the SCL should send a NULL response
   *   and later send the response as an event. The target database can call
   *   sdnpo070_fileCmdComplete(); or the SCL will call sdnpdata_openFile periodically
   *  Other appropriate DNPDEFS_FILE_CMD_STAT values on error.
   *   DNPDEFS_FILE_CMD_STAT_DENIED      
   *   DNPDEFS_FILE_CMD_STAT_INV_MODE    
   *   DNPDEFS_FILE_CMD_STAT_NOT_FOUND    
   *   DNPDEFS_FILE_CMD_STAT_TOO_MANY   
   *   DNPDEFS_FILE_CMD_STAT_LOST_COMM   
   *   DNPDEFS_FILE_CMD_STAT_MISC        
   */
  DNPDEFS_FILE_CMD_STAT TMWDEFS_GLOBAL sdnptarg_openFile(
    TMWTYPES_CHAR *pFilename, 
    DNPDEFS_FILE_MODE mode, 
    TMWTYPES_USHORT *pMaxBlockSize, 
    DNPDEFS_FILE_PERMISSIONS *pPermissions, 
    TMWDTIME *pTimeOfCreation, 
    TMWTYPES_ULONG *pHandle, 
    TMWTYPES_ULONG *pSize,
    DNPDEFS_FILE_TYPE *pType);

  /* function: sdnptarg_closeFile
   * purpose: close existing file
   * arguments:
   *  handle - DNP file handle returned from previous call to sdnpdata_openFile
   * returns:
   *  DNPDEFS_FILE_CMD_STAT_SUCCESS if successful
   *  DNPDEFS_FILE_CMD_STAT_ASYNC if the SCL should send a NULL response
   *   and later send the response as an event. The target database can call
   *   sdnpo070_fileCmdComplete(); or the SCL will call sdnpdata_closeFile periodically
   *  Other appropriate DNPDEFS_FILE_CMD_STAT values on error.
   *   DNPDEFS_FILE_CMD_STAT_INV_HANDLE  
   *   DNPDEFS_FILE_CMD_STAT_LOST_COMM   
   *   DNPDEFS_FILE_CMD_STAT_MISC        
   */
  DNPDEFS_FILE_CMD_STAT TMWDEFS_GLOBAL sdnptarg_closeFile(
    TMWTYPES_ULONG handle);

  /* function: sdnptarg_readFile
   * purpose: read block of data from file.
   * arguments:
   *  handle - DNP file handle returned from previous call to sdnpdata_openFile
   *  pLast - return TMWDEFS_TRUE if this is the last block in this file
   *  pBytesRead - return the actual number of bytes read, up to maxBlockSize
   *   specified in sdnpdata_openFile()
   *  pBuf - buffer into which to store data
   * returns:
   *  DNPDEFS_FILE_TFER_STAT_SUCCESS if successful
   *  DNPDEFS_FILE_TFER_STAT_ASYNC if the SCL should send a NULL response
   *   and later send the response as an event. The database code can call
   *   sdnpo070_fileCmdComplete(); or the SCL will call sdnpdata_readFile again
   *  Appropriate DNPDEFS_FILE_TFER_STAT values on error.
   *   DNPDEFS_FILE_TFER_STAT_INV_HANDLE
   *   DNPDEFS_FILE_TFER_STAT_LOST_COMM  
   *   DNPDEFS_FILE_TFER_STAT_NOT_OPEN  
   *   DNPDEFS_FILE_TFER_STAT_HANDLE_EXP 
   *   DNPDEFS_FILE_TFER_STAT_BAD_FILE   
   *   DNPDEFS_FILE_TFER_STAT_MISC    
   */
  DNPDEFS_FILE_TFER_STAT TMWDEFS_GLOBAL sdnptarg_readFile(
    TMWTYPES_ULONG handle, 
    TMWTYPES_BOOL *pLast,
    TMWTYPES_USHORT *pBytesRead,
    TMWTYPES_UCHAR *pBuf);

  /* function: sdnptarg_writeFile
   * purpose: write block of data to file
   * arguments:
   *  handle - DNP file handle returned from previous call to sdnpdata_openFile
   *  last - TMWDEFS_TRUE if this is the last block in this file
   *  numBytes - number of bytes to write. This will not exceed maxBlockSize
   *   specified in sdnpdata_openFile()
   *  pBuf - pointer to data buffer to write
   * returns:
   *  DNPDEFS_FILE_TFER_STAT_SUCCESS if successful
   *  DNPDEFS_FILE_TFER_STAT_ASYNC if the SCL should send a NULL response
   *   and later send the response as an event. The target database can call
   *   sdnpo070_fileCmdComplete(); or the SCL will call sdnpdata_writeFile periodically
   *  Appropriate DNPDEFS_FILE_TFER_STAT values on error.
   *   DNPDEFS_FILE_TFER_STAT_INV_HANDLE
   *   DNPDEFS_FILE_TFER_STAT_LOST_COMM  
   *   DNPDEFS_FILE_TFER_STAT_NOT_OPEN  
   *   DNPDEFS_FILE_TFER_STAT_HANDLE_EXP 
   *   DNPDEFS_FILE_TFER_STAT_OVERRUN   
   *   DNPDEFS_FILE_TFER_STAT_BAD_FILE   
   *   DNPDEFS_FILE_TFER_STAT_MISC    
   */
  DNPDEFS_FILE_TFER_STAT TMWDEFS_GLOBAL sdnptarg_writeFile(
    TMWTYPES_ULONG handle, 
    TMWTYPES_BOOL last,
    TMWTYPES_USHORT numBytes, 
    TMWTYPES_UCHAR *pBuf);

#ifdef __cplusplus
}
;
#endif

#endif /* TMWTARG_SUPPORT_FILEIO */

#endif /* SDNPTARG_DEFINED */
