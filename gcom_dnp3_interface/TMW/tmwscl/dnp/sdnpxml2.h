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

/* file: sdnpxml2.h
* description: This file is intended for internal SCL use only.
*  Slave DNP Device Profile schema 2.11 XML support
*/

#ifndef SDNPXML2_DEFINED
#define SDNPXML2_DEFINED

#include "tmwscl/dnp/sdnpcnfg.h"
#include "tmwscl/dnp/sdnpdata.h"

#if SDNPDATA_SUPPORT_XML2
typedef enum SDNPXML2SaveStateEnum {
  SDNPXML2_SAVE_FAILED=0,
  SDNPXML2_SAVE_NOT_DONE,
  SDNPXML2_SAVE_DONE,
  SDNPXML2_SAVE_DOC_HEADER,
  SDNPXML2_SAVE_DOC_TAIL,
  SDNPXML2_SAVE_CONFIG_HEAD,
  SDNPXML2_SAVE_CONFIG,
  SDNPXML2_SAVE_CONFIG_TAIL,
  SDNPXML2_SAVE_DEVICECONFIG,
  SDNPXML2_SAVE_SERIALCONFIG,
  SDNPXML2_SAVE_NETWORKCONFIG,
  SDNPXML2_SAVE_LINKCONFIG,
  SDNPXML2_SAVE_APPLCONFIG,
  SDNPXML2_SAVE_OUTSTATIONCONFIG,
  SDNPXML2_SAVE_UNSOLCONFIG,
  SDNPXML2_SAVE_UNSOLTRIGGERCONFIG,
  SDNPXML2_SAVE_OUTPERFORMCONFIG,
  SDNPXML2_SAVE_FIELDCONFIG,
  SDNPXML2_SAVE_SECURITYCONFIG,
  SDNPXML2_SAVE_BROADCASTCONFIG,
  SDNPXML2_SAVE_DATABASE_HEAD,
  SDNPXML2_SAVE_DATABASE_TAIL,
  SDNPXML2_SAVE_BIN_IN,
  SDNPXML2_SAVE_DBL_IN,
  SDNPXML2_SAVE_BIN_OUT,
  SDNPXML2_SAVE_BIN_COUNTER,
  SDNPXML2_SAVE_ANALOG_IN,
  SDNPXML2_SAVE_ANALOG_OUT,
  SDNPXML2_SAVE_FILE,
  SDNPXML2_SAVE_STRING,
  SDNPXML2_SAVE_VTERM,
  SDNPXML2_SAVE_DATAPOINTS_LIST_HEAD,
  SDNPXML2_SAVE_DATAPOINTS_LIST_TAIL,
  SDNPXML2_SAVE_BIN_IN_POINTS_HEAD,
  SDNPXML2_SAVE_BIN_IN_POINTS,
  SDNPXML2_SAVE_BIN_IN_POINTS_TAIL,
  SDNPXML2_SAVE_DBL_IN_POINTS_HEAD,
  SDNPXML2_SAVE_DBL_IN_POINTS,
  SDNPXML2_SAVE_DBL_IN_POINTS_TAIL,
  SDNPXML2_SAVE_BIN_OUT_POINTS_HEAD,
  SDNPXML2_SAVE_BIN_OUT_POINTS,
  SDNPXML2_SAVE_BIN_OUT_POINTS_TAIL,
  SDNPXML2_SAVE_BIN_COUNTER_POINTS_HEAD,
  SDNPXML2_SAVE_BIN_COUNTER_POINTS,
  SDNPXML2_SAVE_BIN_COUNTER_POINTS_TAIL,
  SDNPXML2_SAVE_ANALOG_IN_POINTS_HEAD,
  SDNPXML2_SAVE_ANALOG_IN_POINTS,
  SDNPXML2_SAVE_ANALOG_IN_POINTS_TAIL,
  SDNPXML2_SAVE_ANALOG_OUT_POINTS_HEAD,
  SDNPXML2_SAVE_ANALOG_OUT_POINTS,
  SDNPXML2_SAVE_ANALOG_OUT_POINTS_TAIL,
  SDNPXML2_SAVE_FILES_HEAD,
  SDNPXML2_SAVE_FILES,
  SDNPXML2_SAVE_FILES_TAIL,
  SDNPXML2_SAVE_STRING_POINTS_HEAD,
  SDNPXML2_SAVE_STRING_POINTS,
  SDNPXML2_SAVE_STRING_POINTS_TAIL,
  SDNPXML2_SAVE_VTERM_POINTS_HEAD,
  SDNPXML2_SAVE_VTERM_POINTS,
  SDNPXML2_SAVE_VTERM_POINTS_TAIL,
  SDNPXML2_SAVE_DATASET_PROTO,
  SDNPXML2_SAVE_DATASET_DESCR,
} SDNPXML2_SAVE_STATE;
 
typedef enum SDNPXML2SaveSubStateEnum {
  SDNPXML2_SAVE_DATASETSTART=0,
  SDNPXML2_SAVE_DATASETELEM,
  SDNPXML2_SAVE_DATASETINDEX,
  SDNPXML2_SAVE_DATASETDATA,
  SDNPXML2_SAVE_DATASETEND,
} SDNPXML2_SAVE_SUBSTATE;

#ifdef __cplusplus
extern "C" {
#endif
  
  /* function: sdnpxml2_saveDatabaseToFile 
   * purpose: Generate standard Device Profile in xml format into xml2Buffer 
   *  a block at a time and write it to a file using the sdnpdata_openFile 
   *  and sdnpdata_writeFile interfaces. 
   *  This is intended to be called directly by the outstation/slave application.
   * arguments:
   *  pSession - pointer to session to generate device profile for 
   *  pFileName - name of file to write xml to.
   * returns:  
   *  TMWDEFS_TRUE if successful
   *  TMWDEFS_FALSE otherwise
   */
  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpxml2_saveDatabaseToFile(TMWSESN *pSession, char *pFileName);

  /* function: sdnpxml2_saveDatabaseInit
   * purpose: Initialize session variable related to xml device profile generation.
   *  This should be called before calling sdnpxml2_saveDatabase repeatedly
   *  This may be called directly by the outstation/slave application if 
   *   sdnpxml2_saveDatabaseToFile does not meet your needs.
   * arguments:
   *  pSession - pointer to session 
   * returns: 
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL sdnpxml2_saveDatabaseInit(
    TMWSESN *pSession);

  /* function: sdnpxml2_saveDatabase
   * purpose: Generate standard Device Profile in xml format into buffer provided.
   *  sdnpxml2_saveDatabaseInit() should be called before calling this function 
   *  repeatedly until it returns SDNPXML2_SAVE_FAILED or SDNPXML2_SAVE_DONE.
   *  This may be called directly by the outstation/slave application if 
   *   sdnpxml2_saveDatabaseToFile does not meet your needs.
   * arguments:
   *  pSession - pointer to session
   *  pBuffer - buffer to generate xml into
   *  maxLen - length of buffer provided.
   * returns: 
   *  state of device profile generation
   *    SDNPXML2_SAVE_FAILED if failure
   *    SDNPXML2_SAVE_NOT_DONE if still more to generate
   *    SDNPXML2_SAVE_DONE if done.
   */
  TMWDEFS_SCL_API SDNPXML2_SAVE_STATE TMWDEFS_GLOBAL sdnpxml2_saveDatabase(
    TMWSESN *pSession,
    TMWTYPES_CHAR *pBuffer,
    TMWTYPES_ULONG maxLen);

  /* function: sdnpxml2_saveDatabaseGetSize
   * purpose: Determine the size of the Device Profile xml data that will be generated.
   *  Make sure pSDNPSession->xml2IndentSize is set properly. Normally this is 
   *  set to zero when doing file transfer and 2 when doing a local file write.
   *  This may be called directly by the outstation/slave application
   * arguments:
   *  pSession - pointer to session   
   * returns: 
   *  size of xml data that would be generated.
   */
  TMWDEFS_SCL_API TMWTYPES_ULONG TMWDEFS_GLOBAL sdnpxml2_saveDatabaseGetSize(
    TMWSESN *pSession);

  /* function: sdnpxml2_openDeviceProfile
   * purpose: See if this is an open of a device profile file
   *  THIS IS AN INTERNAL FUNCTION and should not be called by the application
   * arguments:
   *  pSession - pointer to session request was received on
   *  pFilename - null terminated name of file to open
   *  authKey - authentication key for this request
   *  mode - access mode for file  
   *   DNPDEFS_FILE_MODE_READ     
   *   DNPDEFS_FILE_MODE_WRITE  
   *   DNPDEFS_FILE_MODE_APPEND  
   *  pTimeOfCreation - return time of creation for file
   *  pHandle - return file handle, non-zero value if successful. 
   *  pSize - return file size. 
   *   If file was opened for reading, size should indicate the total number 
   *    of octets in the file that was opened. If this is a directory, size
   *    would be the number of octets that would be transferred by the read
   *    requests. This is (the number of files contained in the directory * 20)  
   *    + (sum of the lengths of the names of all of the files in the directory).
   *   If file was opened for writing size SCL will set size to zero.
   *  pType - return type of file
   *   DNPDEFS_FILE_TYPE_SIMPLE    
   * returns:
   *  DNPDEFS_FILE_CMD_STAT_SUCCESS if successful
   *  DNPDEFS_FILE_CMD_STAT_DENIED if not allowed     
   *  DNPDEFS_FILE_CMD_STAT_NOT_FOUND otherwise
   */
  DNPDEFS_FILE_CMD_STAT TMWDEFS_GLOBAL sdnpxml2_openDeviceProfile(
    TMWSESN *pSession,
    TMWTYPES_CHAR *pFilename, 
    TMWTYPES_ULONG authKey,
    DNPDEFS_FILE_MODE mode, 
    TMWDTIME *pTimeOfCreation, 
    TMWTYPES_ULONG *pHandle, 
    TMWTYPES_ULONG *pSize,
    DNPDEFS_FILE_TYPE *pType);
  
  /* function: sdnpxml2_closeDeviceProfile
   * purpose: close device profile
   *  THIS IS AN INTERNAL FUNCTION and should not be called by the application
   * arguments:
   *  pSession - pointer to session request was received on
   *  handle - handle returned from previous call to sdnpxml2_openDeviceProfile
   * returns:
   *  DNPDEFS_FILE_CMD_STAT_SUCCESS if successful
   *  Other appropriate DNPDEFS_FILE_CMD_STAT values on error.
   *   DNPDEFS_FILE_CMD_STAT_INV_HANDLE  
   *   DNPDEFS_FILE_CMD_STAT_LOST_COMM   
   *   DNPDEFS_FILE_CMD_STAT_MISC        
   */
  DNPDEFS_FILE_CMD_STAT TMWDEFS_GLOBAL sdnpxml2_closeDeviceProfile(
    TMWSESN *pSession,
    TMWTYPES_ULONG handle);
     
  /* function: sdnpxml2_readDeviceProfile
   * purpose: read Device Profile into buffer provided, generating the xml as needed.
   *  This should be called in response to a read by the master.
   *  THIS IS AN INTERNAL FUNCTION and should not be called by the application
   * arguments:
   *  pSession - pointer to session request was received on
   *  handle - handle returned from previous call to sdnpxml2_openDeviceProfile
   *  pLast - return value indicating if this was the last block of the read
   *  pBytesRead - return value indicating how many bytes have been read
   *  pBuf - pointer to where to copy the bytes being read
   * returns: 
   *  DNPDEFS_FILE_TFER_STAT_SUCCESS if successful 
   *  DNPDEFS_FILE_TFER_STAT_INV_HANDLE if handle is incorrect  
   *  DNPDEFS_FILE_TFER_STAT_MISC other errors
   */
  DNPDEFS_FILE_TFER_STAT TMWDEFS_GLOBAL sdnpxml2_readDeviceProfile(
    TMWSESN *pSession,
    TMWTYPES_ULONG fileHandle,
    TMWTYPES_BOOL *pLast,
    TMWTYPES_USHORT *pBytesRead,
    TMWTYPES_UCHAR *pBuf);

  
#ifdef __cplusplus
}
#endif
#endif /* SDNPDATA_SUPPORT_XML2 */
#endif
