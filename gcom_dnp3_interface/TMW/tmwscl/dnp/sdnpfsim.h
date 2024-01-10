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

/* file: sdnpsim.h
 * description: Example file I/O implementation.
 *  This file is an example of a simulated DNP file interface.
 *  It should NOT be included in the final version of a DNP slave device.
 */
#ifndef SDNPFSIM_DEFINED
#define SDNPFSIM_DEFINED

#include "tmwscl/utils/tmwcnfg.h"
#include "tmwscl/utils/tmwdefs.h"
#include "tmwscl/utils/tmwdtime.h"
#include "tmwscl/utils/tmwsim.h"
#include "tmwscl/dnp/sdnpcnfg.h"
#include "tmwscl/dnp/sdnpdata.h"
#include "tmwscl/dnp/sdnpsesn.h"

#if SDNPDATA_SUPPORT_OBJ70 || SDNPDATA_SUPPORT_XML2

#if TMWCNFG_USE_SIMULATED_DB
#define SDNPFSIM_HANDLE 0x12345678
#define SDNPSIM_FILE_AUTH_SIZE          32


/* Define simulated file xfer database context */
typedef struct SDNPFSimDatabaseStruct {
  /* Variables for delaying file commands to cause event responses */
  TMWSIM_TESTINGMODE fileTestingMode;
  TMWTYPES_USHORT    fileDelayCount;
  TMWTIMER           fileTestingTimer;

  intptr_t           findHandle;
  TMWDEFS_CLASS_MASK fileEventClass;

#if TMWTARG_SUPPORT_DNPFILEIO || SDNPDATA_SUPPORT_XML2
  char               findFilename[DNPCNFG_MAX_FILENAME];
#endif

  TMWTYPES_USHORT    blockSize;
  FILE              *pCurrentFile;
  TMWTYPES_BOOL      bInternalDriveInfo;  
  TMWTYPES_CHAR      bInternalFileCount;  

  TMWTYPES_ULONG     fileAuthenticationKey; 
  TMWTYPES_CHAR      fileUserName[SDNPSIM_FILE_AUTH_SIZE];
  TMWTYPES_CHAR      filePassword[SDNPSIM_FILE_AUTH_SIZE];

#if SDNPDATA_SUPPORT_XML 
  TMWTYPES_ULONG      xmlStaticDescriptionBufLen;
  TMWTYPES_ULONG      xmlStaticDescriptionBufIndex;
  TMWTYPES_BOOL       xmlStaticDescriptionBufOpen;
  SDNPXML_SAVE_STATE  xmlStaticDescriptionBufState;

  /* SPM, figure out how to use the one in the scl */
  TMWTYPES_CHAR       xmlStaticDescriptionBuf[SDNPCNFG_XML_SAVE_BUF_SIZE];
#endif
} SDNPFSIM_DATABASE;

#ifdef __cplusplus
extern "C" {
#endif

  /* function: sdnpfsim_initDb
   * purpose:  
   * arguments: 
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL sdnpfsim_initDb(
    SDNPFSIM_DATABASE *pFDBHandle);
    
  /* function: sdnpfsim_clearDb
   * purpose:  
   * arguments: 
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL sdnpfsim_clearDb(
    SDNPFSIM_DATABASE *pFDBHandle);
 
  /* function: sdnpfsim_fileEventClass
   * purpose: Return the class in which file events belong.
   * arguments:
   *  pFDBHandle - pointer to FSIM database structure
   * returns:
   *  Class in which these events will be returned
   */
  TMWDEFS_CLASS_MASK TMWDEFS_GLOBAL sdnpfsim_fileEventClass(
    SDNPFSIM_DATABASE *pFDBHandle);

  /* function: sdnpfsim_setFileEventClass 
   * purpose: Assign the class in which file transfer response events
   *  will belong.
   * arguments:
   *  pFDBHandle - pointer to FSIM database structure
   *  classMask - new class in which to generate events from this point
   * returns:
   *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
   */
  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpfsim_setFileEventClass(
    SDNPFSIM_DATABASE *pFDBHandle, 
    TMWDEFS_CLASS_MASK eventClass);
 
  /* function: sdnpfsim_setAuthentication
   * purpose: enable/disable file authentication. If authenticationKey 
   *  is zero, authentication will not be required. If authenticationKey
   *  is nonzero, username and password will be required to get authentication 
   *  key. This key is used for file open and delete.
   * arguments: 
   *  pFDBHandle - pointer to SDNPFSIM database structure
   *  authenticationKey - non-zero value to enable file authentication
   *  pUserName - pointer to user name required for authentication request
   *  pPassword - pointer to password required for authentication request
   * returns:
   *  TMWDEFS_TRUE if successful
   */
  TMWDEFS_SCL_API TMWTYPES_BOOL sdnpfsim_setAuthentication(
    SDNPFSIM_DATABASE *pFDBHandle,
    TMWTYPES_ULONG authenticationKey, 
    TMWTYPES_CHAR *pUserName, 
    TMWTYPES_CHAR *pPassword);

  /* function: sdnpfsim_getAuthenticationData
   * purpose: Get File transfer authentication data
   * arguments: 
   *  pFDBHandle - pointer to SDNPFSIM database structure
   *  PAuthenticationKey - pointer to location to copy file authentication value
   *  pUserName - pointer to location to copy user name required for 
   *    authentication request
   *  pPassword - pointer to location to copy password required for 
   *    authentication request
   * returns:
   *  TMWDEFS_TRUE if successful
   */ 
  TMWDEFS_SCL_API TMWTYPES_BOOL sdnpfsim_getAuthenticationData(
    SDNPFSIM_DATABASE *pFDBHandle,
    TMWTYPES_ULONG *pAthenticationKey, 
    TMWTYPES_CHAR *pUserName, 
    TMWTYPES_USHORT maxUserNameSize,
    TMWTYPES_CHAR *pPassword,
    TMWTYPES_USHORT maxPasswordSize);

  /* function: sdnpfsim_getFileInfo
   * purpose: return information about file or directory
   *  This is used to build a response to a FC 28 Get File Info request.
   * arguments:
   *  pFDBHandle - pointer to SDNPFSIM database structure
   *  pFilename - null terminated name of file to get info from
   *  pType - return type of file
   *    DNPDEFS_FILE_TYPE_SIMPLE  or
   *    DNPDEFS_FILE_TYPE_DIRECTORY
   *  pSize - return size of simple file in bytes 
   *          or directory in number of files and subdirectories,
   *          excluding links to itself and parent
   *  pTimeOfCreation - return file or directory time of creation
   *  pPermissions - return file or directory permissions
   *   see DNPDEFS_FILE_PERMISSIONS in dnpdefs.h
   * returns:
   *  Valid DNPDEFS_FILE_CMD_STAT status
   *  DNPDEFS_FILE_CMD_STAT_SUCCESS if successful
   *  DNPDEFS_FILE_CMD_STAT_NOT_FOUND If file does not exist 
   */
  DNPDEFS_FILE_CMD_STAT TMWDEFS_GLOBAL sdnpfsim_getFileInfo(
    SDNPFSIM_DATABASE *pFDBHandle,
    TMWTYPES_CHAR *pFilename, 
    DNPDEFS_FILE_TYPE *pType,
    TMWTYPES_ULONG *pSize, 
    TMWDTIME *pTimeOfCreation, 
    DNPDEFS_FILE_PERMISSIONS *pPermissions);

  /* function: sdnpfsim_getFileInfo
   * purpose: return information about file or directory
   *  This is used to build a response to a FC 1 read request on a directory.
   *  This function will be called multiple times. It should set pLast to
   *  TRUE when the last entry in a directory is being read.
   * arguments:
   *  pSession - pointer to session request was received on
   *  pFDBHandle - pointer to SDNPFSIM database structure
   *  dnpFileHandle - 32-bit DNP file handle
   *  pFilename - null terminated name of file to get info from
   *  pLast - return TMWDEFS_TRUE if this is the last block in this file
   *  pType - return type of file
   *    DNPDEFS_FILE_TYPE_SIMPLE  or
   *    DNPDEFS_FILE_TYPE_DIRECTORY
   *  pSize - return size of simple file in bytes 
   *          or directory in number of files and subdirectories,
   *          excluding links to itself and parent
   *  pTimeOfCreation - return file time of creation
   *  pPermissions - return file permissions
   * returns:
   *  Valid DNPDEFS_FILE_CMD_STAT status
   */
  DNPDEFS_FILE_TFER_STAT TMWDEFS_GLOBAL sdnpfsim_readFileInfo(
    TMWSESN *pSession,
    SDNPFSIM_DATABASE *pFDBHandle,
    TMWTYPES_ULONG dnpFileHandle,
    TMWTYPES_USHORT maxNameSize,
    TMWTYPES_CHAR *pName,
    TMWTYPES_BOOL *pLast,
    DNPDEFS_FILE_TYPE *pType,
    TMWTYPES_ULONG *pSize, 
    TMWDTIME *pTimeOfCreation, 
    DNPDEFS_FILE_PERMISSIONS *pPermissions);

  /* function: sdnpfsim_getAuthentication
   * purpose: return authentication key to be used for next file
   *  operation. If authentication is not supported this routine
   *  should set pAuthKey to 0 and return success.
   * arguments:
   *  pFDBHandle - pointer to SDNPFSIM database structure
   *  pUsername - username of user requesting authentication
   *  pPassword - password of user requesting authentication
   *  pAuthKey - return authentication key
   * returns:
   *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpfsim_getAuthentication(
    SDNPFSIM_DATABASE *pFDBHandle,
    TMWTYPES_CHAR *pUsername, 
    TMWTYPES_CHAR *pPassword, 
    TMWTYPES_ULONG *pAuthKey);
   
  /* function: sdnpfsim_deviceProfileFile
   * purpose: check to see if name indicates the SCL should generate the Device Profile 
   *  in XML format 
   * arguments:
   *  pFDBHandle - pointer to SDNPFSIM database structure 
   *  pFileName - pointer to file name to be checked 
   *  authKey - authentication key to be validated
   * returns:
   *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpfsim_deviceProfileFile(
    SDNPFSIM_DATABASE *pFDBHandle,
    TMWTYPES_CHAR *pFileName,
    TMWTYPES_ULONG authKey,
    TMWTYPES_BOOL *pAuthKeyOK);

  /* function: sdnpfsim_deleteFile
   * purpose: delete the requested file
   * arguments:
   *  pFDBHandle - pointer to SDNPFSIM database structure
   *  pFilename - null terminated name of file to delete
   *  authKey - authentication key for this request
   * returns:
   *  Valid DNPDEFS_FILE_CMD_STAT status
   */
  DNPDEFS_FILE_CMD_STAT TMWDEFS_GLOBAL sdnpfsim_deleteFile(
    SDNPFSIM_DATABASE *pFDBHandle,
    TMWTYPES_CHAR *pFilename, 
    TMWTYPES_ULONG authKey);

  /* function: sdnpfsim_openFile
   * purpose: open specified file
   * arguments:
   *  pSession - pointer to session request was received on
   *  pFDBHandle - pointer to SDNPFSIM database structure
   *  pFilename - null terminated name of file to open
   *  authKey - authentication key for this request
   *  mode - access mode for file (read, write, append)
   *  pMaxBlockSize - pointer to proposed maximum block size for this file
   *   this function can modify this to return a reduced maxBlockSize
   *  pPermissions - permissions to give new file if created
   *  pTimeOfCreation - return time of creation for file
   *  pFileHandle - return file handle, non-zero value if successful
   *  pSize - return file size
   * returns:
   *  Valid DNPDEFS_FILE_CMD_STAT status
   */
  DNPDEFS_FILE_CMD_STAT TMWDEFS_GLOBAL sdnpfsim_openFile(
    TMWSESN *pSession,
    SDNPFSIM_DATABASE *pFDBHandle,
    TMWTYPES_CHAR *pFilename, 
    TMWTYPES_ULONG authKey,
    DNPDEFS_FILE_MODE mode, 
    TMWTYPES_USHORT *pMaxBlockSize, 
    DNPDEFS_FILE_PERMISSIONS *pPermissions, 
    TMWDTIME *pTimeOfCreation, 
    TMWTYPES_ULONG *pFileHandle, 
    TMWTYPES_ULONG *pSize,
    DNPDEFS_FILE_TYPE *pType);

  /* function: sdnpfsim_closeFile
   * purpose: close existing file
   * arguments:
   *  pSession - pointer to session request was received on
   *  pFDBHandle - pointer to SDNPFSIM database structure
   *  fileHandle - handle returned from previous call to sdnpfsim_openFile
   * returns:
   *  Valid DNPDEFS_FILE_CMD_STAT status
   */
  DNPDEFS_FILE_CMD_STAT TMWDEFS_GLOBAL sdnpfsim_closeFile(
    TMWSESN *pSession,
    SDNPFSIM_DATABASE *pFDBHandle,
    TMWTYPES_ULONG fileHandle);

  /* function: sdnpfsim_readFile
   * purpose: read block of data from file.
   * arguments:
   *  pSession - pointer to session request was received on
   *  pFDBHandle - pointer to SDNPFSIM database structure
   *  fileHandle - handle returned from previous call to sdnpfsim_openFile
   *  pLast - return TMWDEFS_TRUE if this is the last block in this file
   *  pBytesRead - return the actual number of bytes read, up to maxBlockSize
   *   specified in sdnpfsim_openFile()
   *  pBuf - buffer into which to store data
   * returns:
   *  Valid DNPDEFS_FILE_TFER_STAT status
   */
  DNPDEFS_FILE_TFER_STAT TMWDEFS_GLOBAL sdnpfsim_readFile(
    TMWSESN *pSession,
    SDNPFSIM_DATABASE *pFDBHandle,
    TMWTYPES_ULONG fileHandle,
    TMWTYPES_BOOL *pLast,
    TMWTYPES_USHORT *pBytesRead,
    TMWTYPES_UCHAR *pBuf);

  /* function: sdnpfsim_writeFile
   * purpose: write block of data to file
   * arguments:
   *  pFDBHandle - pointer to SDNPFSIM database structure
   *  fileHandle - handle returned from previous call to sdnpfsim_openFile
   *  last - TMWDEFS_TRUE if this is the last block in this file
   *  numBytes - number of bytes to write. This will not exceed maxBlockSize
   *   specified in sdnpfsim_openFile()
   *  pBuf - pointer to data buffer to write
   * returns:
   *  Valid DNPDEFS_FILE_TFER_STAT status
   */
  DNPDEFS_FILE_TFER_STAT TMWDEFS_GLOBAL sdnpfsim_writeFile(
    SDNPFSIM_DATABASE *pFDBHandle, 
    TMWTYPES_ULONG fileHandle, 
    TMWTYPES_BOOL last,
    TMWTYPES_USHORT numBytes, 
    TMWTYPES_UCHAR *pBuf);

  /* function: sdnpfsim_setFileTestingMode */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL sdnpfsim_setFileTestingMode(
    SDNPFSIM_DATABASE *pFDBHandle,
    TMWSIM_TESTINGMODE testingMode);

#ifdef __cplusplus
}
#endif
#endif

#endif /* #if TMWCNFG_USE_SIMULATED_DB */

#endif /* SDNPFSIM_DEFINED */
