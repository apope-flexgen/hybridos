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

/* file: dnpbncfg.c
 * description: Processes binary configuration files
 */
#include "tmwscl/dnp/dnpdiag.h"
#include "tmwscl/utils/tmwtarg.h"

#include "tmwscl/dnp/dnpdefs.h"
#include "tmwscl/dnp/dnpchnl.h"
#include "tmwscl/dnp/dnpbncfg.h"

#if DNPCNFG_SUPPORT_BINCONFIG

static TMWTYPES_BOOL TMWDEFS_LOCAL dnpbncfg_ReadBinaryConfigHeader(
  FILE *fp, 
  DNPBNCFG_FILEVALUES *fileValues,
  TMWTYPES_USHORT *pXMLVersionIndex);

static TMWTYPES_BOOL TMWDEFS_LOCAL dnpbncfg_ReadBinaryConfigSection1_1(
  FILE *fp,
  TMWTYPES_USHORT xmlVersionIndex, 
  DNPBNCFG_FILEVALUES *fileValues,
  TMWTARG_BINFILE_VALS *pTargBinFileValues);

static TMWTYPES_BOOL TMWDEFS_LOCAL dnpbncfg_ReadBinaryConfigSection1_2(
  FILE *fp,
  TMWTYPES_USHORT xmlVersionIndex, 
  DNPBNCFG_FILEVALUES *fileValues,
  TMWTARG_BINFILE_VALS *pTargBinFileValues);

static TMWTYPES_BOOL TMWDEFS_LOCAL dnpbncfg_ReadBinaryConfigSection1_3(
  FILE *fp,
  TMWTYPES_USHORT xmlVersionIndex, 
  DNPBNCFG_FILEVALUES *fileValues,
  TMWTARG_BINFILE_VALS *pTargBinFileValues);

static TMWTYPES_BOOL TMWDEFS_LOCAL dnpbncfg_ReadBinaryConfigSection1_4(
  FILE *fp,
  TMWTYPES_USHORT xmlVersionIndex, 
  DNPBNCFG_FILEVALUES *fileValues);

static TMWTYPES_BOOL TMWDEFS_LOCAL dnpbncfg_ReadBinaryConfigSection1_5(
  FILE *fp,
  TMWTYPES_USHORT xmlVersionIndex, 
  DNPBNCFG_FILEVALUES *fileValues);

static TMWTYPES_BOOL TMWDEFS_LOCAL dnpbncfg_ReadBinaryConfigSection1_6(
  FILE *fp,
  TMWTYPES_USHORT xmlVersionIndex, 
  DNPBNCFG_FILEVALUES *fileValues);

static TMWTYPES_BOOL TMWDEFS_LOCAL dnpbncfg_ReadBinaryConfigSection1_7(
  FILE *fp,
  TMWTYPES_USHORT xmlVersionIndex, 
  DNPBNCFG_FILEVALUES *fileValues);

static TMWTYPES_BOOL TMWDEFS_LOCAL dnpbncfg_ReadBinaryConfigSection1_8(
  FILE *fp,
  TMWTYPES_USHORT xmlVersionIndex, 
  DNPBNCFG_FILEVALUES *fileValues);

static TMWTYPES_BOOL TMWDEFS_LOCAL dnpbncfg_ReadBinaryConfigSection1_9(
  FILE *fp,
  TMWTYPES_USHORT xmlVersionIndex, 
  DNPBNCFG_FILEVALUES *fileValues);

static TMWTYPES_BOOL TMWDEFS_LOCAL dnpbncfg_ReadBinaryConfigSection1_10(
  FILE *fp,
  TMWTYPES_USHORT xmlVersionIndex, 
  DNPBNCFG_FILEVALUES *fileValues);

static TMWTYPES_BOOL TMWDEFS_LOCAL dnpbncfg_ReadBinaryConfigSection1_11(
  FILE *fp,
  TMWTYPES_USHORT xmlVersionIndex, 
  DNPBNCFG_FILEVALUES *fileValues);

static TMWTYPES_BOOL TMWDEFS_LOCAL dnpbncfg_ReadBinaryConfigSection1_12(
  FILE *fp,
  TMWTYPES_USHORT xmlVersionIndex, 
  DNPBNCFG_FILEVALUES *fileValues);

static TMWTYPES_BOOL TMWDEFS_LOCAL dnpbncfg_ReadBinaryConfigSection1_13(
  FILE *fp,
  TMWTYPES_USHORT xmlVersionIndex, 
  DNPBNCFG_FILEVALUES *fileValues);

static TMWTYPES_USHORT TMWDEFS_LOCAL dnpbncfg_GetXmlVersionIndex(
  DNPBNCFG_FILEVALUES *fileValues);

/* function: dnpbncfg_ReadBinaryConfigHeader */
static TMWTYPES_BOOL TMWDEFS_LOCAL dnpbncfg_ReadBinaryConfigHeader(
  FILE *fp, 
  DNPBNCFG_FILEVALUES *fileValues,
  TMWTYPES_USHORT *pXMLVersionIndex)
{
  size_t charRead;

  TMWTYPES_UCHAR fileType[DNPBNCFG_BIN_CONFIG_HEADER_FIELD_LEN];

  charRead = fread(fileType, 1, DNPBNCFG_BIN_CONFIG_HEADER_FIELD_LEN, fp);

  if (charRead == DNPBNCFG_BIN_CONFIG_HEADER_FIELD_LEN)
    charRead = fread(fileValues->xmlVersion, 1, DNPBNCFG_BIN_CONFIG_HEADER_FIELD_LEN, fp);
  else
    return TMWDEFS_FALSE;

  if (charRead == DNPBNCFG_BIN_CONFIG_HEADER_FIELD_LEN)
    charRead = fread(fileValues->fileVersion, 1, DNPBNCFG_BIN_CONFIG_HEADER_FIELD_LEN, fp);
  else
    return TMWDEFS_FALSE;

  if (charRead == DNPBNCFG_BIN_CONFIG_HEADER_FIELD_LEN)
  {
      *pXMLVersionIndex = dnpbncfg_GetXmlVersionIndex(fileValues);
  }
  else
    return TMWDEFS_FALSE;

  return TMWDEFS_TRUE;
}

static TMWTYPES_USHORT TMWDEFS_LOCAL dnpbncfg_GetXmlVersionIndex(
  DNPBNCFG_FILEVALUES *fileValues)
{
  TMWTYPES_USHORT versionIndex = 0;

  TMWTYPES_CHAR *version = (TMWTYPES_CHAR*)fileValues->xmlVersion;

  if (version[4] == 0 && version[0] == '2' && version[1] == '.')
  {
    if (version[2] == '0' && version[3] == '7')
      versionIndex = 0;
    else if (version[2] == '0' && version[3] == '8')
      versionIndex = 1;
    else if (version[2] == '0' && version[3] == '9')
      versionIndex = 2;
    else if (version[2] == '1' && version[3] == '0')
      versionIndex = 3;
  }

  return versionIndex;
}



/* function: dnpbncfg_ReadBinaryConfigSection1_1 */
static TMWTYPES_BOOL TMWDEFS_LOCAL dnpbncfg_ReadBinaryConfigSection1_1(
  FILE *fp,
  TMWTYPES_USHORT xmlVersionIndex, 
  DNPBNCFG_FILEVALUES *fileValues,
  TMWTARG_BINFILE_VALS *pTargBinFileValues)
{
  size_t charRead;
  TMWTYPES_ULONG sectionSize;
  TMWTYPES_BOOL readOK;

  TMWTYPES_UCHAR sectionSizeArray[DNPBNCFG_SECTION_SIZE_FIELD_SIZE];
  TMWTYPES_UCHAR field1[DNPBNCFG_CONTROL_SIZE1_1];
  TMWTYPES_UCHAR field2[DNPBNCFG_DEVFUNCTION_SIZE1_1];
  TMWTYPES_UCHAR field3[DNPBNCFG_VENDORNAME_SIZE1_1];
  TMWTYPES_UCHAR field4[DNPBNCFG_DEVICENAME_SIZE1_1];
  TMWTYPES_UCHAR field5[DNPBNCFG_HARDWAREVERSION_SIZE1_1];
  TMWTYPES_UCHAR field6[DNPBNCFG_SOFTWAREVERSION_SIZE1_1];
  TMWTYPES_UCHAR field7[DNPBNCFG_DOCVERSIONNUM_SIZE1_1];
  TMWTYPES_UCHAR field8[DNPBNCFG_DNPLEVELSSUPPORTED_SIZE1_1];
  TMWTYPES_UCHAR field9[DNPBNCFG_SUPPORTEDFUNCTIONBLOCK_SIZE1_1];
  TMWTYPES_UCHAR field10[DNPBNCFG_METHODSTOSETCONFIGPARAMS_SIZE1_1];
  TMWTYPES_UCHAR field11[DNPBNCFG_CONNECTIONSSUPPORTED_SIZE1_1];
  TMWTARG_UNUSED_PARAM(xmlVersionIndex);

  charRead = fread(sectionSizeArray, 1, DNPBNCFG_SECTION_SIZE_FIELD_SIZE, fp);
  charRead = fread(field1, 1, DNPBNCFG_CONTROL_SIZE1_1, fp);
  charRead += fread(field2, 1, DNPBNCFG_DEVFUNCTION_SIZE1_1, fp);
  charRead += fread(field3, 1, DNPBNCFG_VENDORNAME_SIZE1_1, fp);
  charRead += fread(field4, 1, DNPBNCFG_DEVICENAME_SIZE1_1, fp);
  charRead += fread(field5, 1, DNPBNCFG_HARDWAREVERSION_SIZE1_1, fp);
  charRead += fread(field6, 1, DNPBNCFG_SOFTWAREVERSION_SIZE1_1, fp);
  charRead += fread(field7, 1, DNPBNCFG_DOCVERSIONNUM_SIZE1_1, fp);
  charRead += fread(field8, 1, DNPBNCFG_DNPLEVELSSUPPORTED_SIZE1_1, fp);
  charRead += fread(field9, 1, DNPBNCFG_SUPPORTEDFUNCTIONBLOCK_SIZE1_1, fp);
  charRead += fread(field10, 1, DNPBNCFG_METHODSTOSETCONFIGPARAMS_SIZE1_1, fp);
  charRead += fread(field11, 1, DNPBNCFG_CONNECTIONSSUPPORTED_SIZE1_1, fp);

  tmwtarg_get32(sectionSizeArray, &sectionSize);

  readOK = (sectionSize == charRead);

  if(readOK)
  {
    TMWTYPES_ULONG fieldUsed = 0;
    TMWTYPES_ULONG fieldValue = 0;

    tmwtarg_get32(field1, &fieldUsed);
    fileValues->fieldsUsed1_1 = fieldUsed;


    /* section 1.1.1 */
    if(fieldUsed & CRTL_DEVFUNCT1_1_1)
    {
      tmwtarg_get32(field2, &fieldValue);

      if(fieldValue & FUNCTION_MASTER1_1)
        fileValues->isOutStation = TMWDEFS_FALSE;
      else
        fileValues->isOutStation = TMWDEFS_TRUE;
    }

    
    /* save target layer values */
    fieldValue = 0;

    if(pTargBinFileValues != TMWDEFS_NULL)
    {
      /*1.1.13 */
      if(fileValues->fieldsUsed1_1 & CRTL_CONNECTIONSUP1_1_13)
      {
        tmwtarg_get32(field11, &fileValues->connectionsSupported1_1);

        if(fileValues->connectionsSupported1_1 & CONNTYPES_SERIAL1_1)
          pTargBinFileValues->supportsSerialConn = TMWDEFS_TRUE;
        else
          pTargBinFileValues->supportsSerialConn = TMWDEFS_FALSE;

        if(fileValues->connectionsSupported1_1 & CONNTYPES_NETWORK1_1)
          pTargBinFileValues->supportsTCPConn = TMWDEFS_TRUE;
        else
          pTargBinFileValues->supportsTCPConn = TMWDEFS_FALSE;

        pTargBinFileValues->useSupportedComm1_1_13 = TMWDEFS_TRUE;
      }
    }
  }

  return readOK;
}

/* function: dnpbncfg_ReadBinaryConfigSection1_2 */
static TMWTYPES_BOOL TMWDEFS_LOCAL dnpbncfg_ReadBinaryConfigSection1_2(
  FILE *fp,
  TMWTYPES_USHORT xmlVersionIndex, 
  DNPBNCFG_FILEVALUES *fileValues,
  TMWTARG_BINFILE_VALS *pTargBinFileValues)
{
  size_t charRead;
  TMWTYPES_ULONG sectionSize;
  TMWTYPES_BOOL readOK;

  TMWTYPES_UCHAR sectionSizeArray[DNPBNCFG_SECTION_SIZE_FIELD_SIZE];
  TMWTYPES_UCHAR field1[DNPBNCFG_CONTROL_SIZE1_2];
  TMWTYPES_UCHAR field2[DNPBNCFG_SPECIALCONTROL_SIZE1_2];
  TMWTYPES_UCHAR field3[DNPBNCFG_PORTNAME_SIZE1_2+1];
  TMWTYPES_UCHAR field4[DNPBNCFG_BAUDRATE_SIZE1_2];
  TMWTYPES_UCHAR field5[DNPBNCFG_FLOWCONTROL_SIZE1_2];
  TMWTYPES_UCHAR field6[DNPBNCFG_LINKSTATUSINTERVAL_SIZE1_2];
  TMWTYPES_UCHAR field7[DNPBNCFG_MINBACKOFFTIMEFORCOLLISIONAVIODANCE_SIZE1_2];
  TMWTYPES_UCHAR field8[DNPBNCFG_MAXRANDOMBACKOFFTIMEFORCOLLISIONAVIODANCE_SIZE1_2];
  TMWTYPES_UCHAR field9[DNPBNCFG_RECEIVERINTERCHARTIMEOUT_SIZE1_2];
  TMWTYPES_UCHAR field10[DNPBNCFG_INTERCHARMAXGAPSINTX_SIZE1_2];
  TMWTARG_UNUSED_PARAM(xmlVersionIndex);


  charRead = fread(sectionSizeArray, 1, DNPBNCFG_SECTION_SIZE_FIELD_SIZE, fp);
  charRead = fread(field1, 1, DNPBNCFG_CONTROL_SIZE1_2, fp);
  charRead += fread(field2, 1, DNPBNCFG_SPECIALCONTROL_SIZE1_2, fp);
  charRead += fread(field3, 1, DNPBNCFG_PORTNAME_SIZE1_2, fp);
  field3[DNPBNCFG_PORTNAME_SIZE1_2] = 0; /* Null terminate */
  charRead += fread(field4, 1, DNPBNCFG_BAUDRATE_SIZE1_2, fp);
  charRead += fread(field5, 1, DNPBNCFG_FLOWCONTROL_SIZE1_2, fp);
  charRead += fread(field6, 1, DNPBNCFG_LINKSTATUSINTERVAL_SIZE1_2, fp);
  charRead += fread(field7, 1, DNPBNCFG_MINBACKOFFTIMEFORCOLLISIONAVIODANCE_SIZE1_2, fp);
  charRead += fread(field8, 1, DNPBNCFG_MAXRANDOMBACKOFFTIMEFORCOLLISIONAVIODANCE_SIZE1_2, fp);
  charRead += fread(field9, 1, DNPBNCFG_RECEIVERINTERCHARTIMEOUT_SIZE1_2, fp);
  charRead += fread(field10, 1, DNPBNCFG_INTERCHARMAXGAPSINTX_SIZE1_2, fp);

  tmwtarg_get32(sectionSizeArray, &sectionSize);

  readOK = (sectionSize == charRead);

  if(readOK)
  {
    tmwtarg_get32(field1, &fileValues->fieldsUsed1_2);
    tmwtarg_get32(field2, &fileValues->specialControls1_2);
    strncpy(fileValues->portName1_2, (const char *)field3, DNPBNCFG_PORTNAME_SIZE1_2+1);
    tmwtarg_get32(field4, &fileValues->baudRate1_2);
    tmwtarg_get32(field5, &fileValues->flowControl1_2);
    tmwtarg_get32(field6, &fileValues->linkStatusInterval1_2);
    tmwtarg_get32(field7, &fileValues->minBackOffTimeCollAviod1_2);
    tmwtarg_get32(field8, &fileValues->maxRandBackOffTimeCollAviod1_2);
    tmwtarg_get32(field9, &fileValues->receiverInterCharTimeout1_2);
    tmwtarg_get32(field10, &fileValues->interCharMaxGapsInTx1_2);

    /* save target layer values */
    if(pTargBinFileValues != TMWDEFS_NULL)
    {
      /*1.2.1*/
      if(fileValues->fieldsUsed1_2 & CRTL_PORTNAME1_2_1)
      {
        strncpy(pTargBinFileValues->serialPortName, fileValues->portName1_2, DNPBNCFG_PORTNAME_SIZE1_2+1);
        pTargBinFileValues->useSerialPortName1_2_1 = TMWDEFS_TRUE;
      }

      /*1.2.3*/
      if(fileValues->fieldsUsed1_2 & CRTL_BAUDRATE1_2_3)
      {
        sprintf(pTargBinFileValues->baudRate, "%d", fileValues->baudRate1_2);
        pTargBinFileValues->useBuadRate1_2_3 = TMWDEFS_TRUE;
      }
    }
  }

  return readOK;

}

/* function: dnpbncfg_ReadBinaryConfigSection1_3 */
static TMWTYPES_BOOL TMWDEFS_LOCAL dnpbncfg_ReadBinaryConfigSection1_3(
  FILE *fp,
  TMWTYPES_USHORT xmlVersionIndex, 
  DNPBNCFG_FILEVALUES *fileValues,
  TMWTARG_BINFILE_VALS *pBinFileTargValues)
{
  TMWTYPES_ULONG i;
  size_t charRead;
  TMWTYPES_ULONG sectionSize;
  TMWTYPES_BOOL readOK;
  TMWTYPES_ULONG numberOfIPAddrs = 0;
    
  
  TMWTYPES_UCHAR sectionSizeArray[DNPBNCFG_SECTION_SIZE_FIELD_SIZE];
  TMWTYPES_UCHAR field1[DNPBNCFG_CONTROL_SIZE1_3];
  TMWTYPES_UCHAR field2[DNPBNCFG_SPECIALCONTROL_SIZE1_3];
  TMWTYPES_UCHAR field3[DNPBNCFG_PORTNAME_SIZE1_3+1];
  TMWTYPES_UCHAR field4[DNPBNCFG_TYPEOFENDPOINT_SIZE1_3];
  TMWTYPES_UCHAR field5[DNPBNCFG_IPOFDEVICE_SIZE1_3+1];
  TMWTYPES_UCHAR field6[DNPBNCFG_SUBNETMASK_SIZE1_3+1];
  TMWTYPES_UCHAR field7[DNPBNCFG_GATEWAYIPADDR_SIZE1_3+1];
  TMWTYPES_UCHAR field8[DNPBNCFG_ACCEPTSTCPCONNSORUDPDARAGRAMS_SIZE1_3];
  TMWTYPES_UCHAR field9[DNPBNCFG_IPADDRSTOACCEPTTCPANDUDPCOUNT_SIZE1_3];
  TMWTYPES_UCHAR field10[DNPBNCFG_MAXNUMIPADDRS1_3][DNPBNCFG_IPADDRTOACCEPTTCPANDUDP_SIZE1_3+1];
  TMWTYPES_UCHAR field11[DNPBNCFG_TCPLISTENPORTNUMBER_SIZE1_3];
  TMWTYPES_UCHAR field12[DNPBNCFG_TCPLISTENPORTNUMBEROFREMOTE_SIZE1_3];
  TMWTYPES_UCHAR field13[DNPBNCFG_TCPKEEPALIVETIMER_SIZE1_3];
  TMWTYPES_UCHAR field14[DNPBNCFG_LOCALUDPPORT_SIZE1_3];
  TMWTYPES_UCHAR field15[DNPBNCFG_DESTUDPPORTFORDNP3REQ_SIZE1_3];
  TMWTYPES_UCHAR field16[DNPBNCFG_DESTUDPPORTFORINITIALUNSOLNULLRESP_SIZE1_3];
  TMWTYPES_UCHAR field17[DNPBNCFG_DESTUDPPORTFORRESPONSES_SIZE1_3];
  TMWTYPES_UCHAR field18[DNPBNCFG_MULTIPLEMASTERCONNECTIONS_SIZE1_3];
  TMWTYPES_UCHAR field19[DNPBNCFG_TIMESYNCHSUPPORT_SIZE1_3];
  TMWTARG_UNUSED_PARAM(xmlVersionIndex);


  charRead = fread(sectionSizeArray, 1, DNPBNCFG_SECTION_SIZE_FIELD_SIZE, fp);
  charRead = fread(field1, 1, DNPBNCFG_CONTROL_SIZE1_3, fp);
  charRead += fread(field2, 1, DNPBNCFG_SPECIALCONTROL_SIZE1_3, fp);
  charRead += fread(field3, 1, DNPBNCFG_PORTNAME_SIZE1_3, fp);
  field3[DNPBNCFG_PORTNAME_SIZE1_3] = 0; /* Null terminate */
  charRead += fread(field4, 1, DNPBNCFG_TYPEOFENDPOINT_SIZE1_3, fp);
  charRead += fread(field5, 1, DNPBNCFG_IPOFDEVICE_SIZE1_3, fp);
  field5[DNPBNCFG_IPOFDEVICE_SIZE1_3] = 0; /* Null terminate */
  charRead += fread(field6, 1, DNPBNCFG_SUBNETMASK_SIZE1_3, fp);
  field6[DNPBNCFG_SUBNETMASK_SIZE1_3] = 0; /* Null terminate */
  charRead += fread(field7, 1, DNPBNCFG_GATEWAYIPADDR_SIZE1_3, fp);
  field7[DNPBNCFG_GATEWAYIPADDR_SIZE1_3] = 0; /* Null terminate */
  charRead += fread(field8, 1, DNPBNCFG_ACCEPTSTCPCONNSORUDPDARAGRAMS_SIZE1_3, fp);
  charRead += fread(field9, 1, DNPBNCFG_IPADDRSTOACCEPTTCPANDUDPCOUNT_SIZE1_3, fp);

  tmwtarg_get32(field9, &numberOfIPAddrs);

  for (i = 0; i < numberOfIPAddrs && i < DNPBNCFG_MAXNUMIPADDRS1_3; ++i)
  {
    charRead += fread(field10[i], 1, DNPBNCFG_IPADDRTOACCEPTTCPANDUDP_SIZE1_3, fp);
    field10[i][DNPBNCFG_IPADDRTOACCEPTTCPANDUDP_SIZE1_3] = 0; /* Null terminate */
  }

  charRead += fread(field11, 1, DNPBNCFG_TCPLISTENPORTNUMBER_SIZE1_3, fp);
  charRead += fread(field12, 1, DNPBNCFG_TCPLISTENPORTNUMBEROFREMOTE_SIZE1_3, fp);
  charRead += fread(field13, 1, DNPBNCFG_TCPKEEPALIVETIMER_SIZE1_3, fp);
  charRead += fread(field14, 1, DNPBNCFG_LOCALUDPPORT_SIZE1_3, fp);
  charRead += fread(field15, 1, DNPBNCFG_DESTUDPPORTFORDNP3REQ_SIZE1_3, fp);
  charRead += fread(field16, 1, DNPBNCFG_DESTUDPPORTFORINITIALUNSOLNULLRESP_SIZE1_3, fp);
  charRead += fread(field17, 1, DNPBNCFG_DESTUDPPORTFORRESPONSES_SIZE1_3, fp);
  charRead += fread(field18, 1, DNPBNCFG_MULTIPLEMASTERCONNECTIONS_SIZE1_3, fp);
  charRead += fread(field19, 1, DNPBNCFG_TIMESYNCHSUPPORT_SIZE1_3, fp);

  tmwtarg_get32(sectionSizeArray, &sectionSize);

  readOK = (sectionSize == charRead);

  if(readOK)
  {
    tmwtarg_get32(field1, &fileValues->fieldsUsed1_3);
    tmwtarg_get32(field2, &fileValues->specialControls1_3);
    strncpy(fileValues->portName1_3, (const char *)field3, DNPBNCFG_PORTNAME_SIZE1_3+1);
    tmwtarg_get32(field4, &fileValues->typeOfEndpoint1_3);
    strncpy(fileValues->ipOfDevice1_3, (const char *)field5, DNPBNCFG_IPOFDEVICE_SIZE1_3+1);
    strncpy(fileValues->subnetMask1_3, (const char *)field6, DNPBNCFG_SUBNETMASK_SIZE1_3+1);
    strncpy(fileValues->gatewayIpAddr1_3, (const char *)field7, DNPBNCFG_GATEWAYIPADDR_SIZE1_3+1);
    tmwtarg_get32(field8, &fileValues->acceptsTcpConnsOrUdp1_3);
    tmwtarg_get32(field9, &fileValues->ipAddrsToAcceptTcpandUdpCount1_3);

    for (i = 0; i < numberOfIPAddrs && i < DNPBNCFG_MAXNUMIPADDRS1_3; ++i)
    {
      strncpy(fileValues->ipAddrToAcceptTcpandUdp1_3[i], (const char *)field10[i], DNPBNCFG_IPADDRTOACCEPTTCPANDUDP_SIZE1_3+1);
    }

    tmwtarg_get32(field11, &fileValues->tcpListenPortNumber1_3);
    tmwtarg_get32(field12, &fileValues->tcpListenPortNumberOfRemote1_3);
    tmwtarg_get32(field13, &fileValues->tcpKeepAliveTimer1_3);
    tmwtarg_get32(field14, &fileValues->localUdpPort1_3);
    tmwtarg_get32(field15, &fileValues->destUdpPortForDnp3req1_3);
    tmwtarg_get32(field16, &fileValues->destUdpPortForInitialUnsolNullResp1_3);
    tmwtarg_get32(field17, &fileValues->destUdpPortForResponses1_3);
    tmwtarg_get32(field18, &fileValues->multiMasterConnections1_3);
    tmwtarg_get32(field19, &fileValues->timeSynchSupport1_3);


    /* save target layer values */
    if(pBinFileTargValues != TMWDEFS_NULL)
    {

      /*1.3.1*/
      if(fileValues->fieldsUsed1_3 & CRTL_PORTNAME1_3_1)
      {
        strncpy(pBinFileTargValues->ipPortName, fileValues->portName1_3, TMWTARG_STR_LEN);
        pBinFileTargValues->useIpPortName1_3_1 = TMWDEFS_TRUE;
      }

      /*1.3.2*/
      if(fileValues->fieldsUsed1_3 & CRTL_TYPEOFENDPOINT1_3_2)
      {
        pBinFileTargValues->endpointIsTcpInitiating = (fileValues->typeOfEndpoint1_3 & ENDPOINTS_TCPINITIATING) ? TMWDEFS_TRUE : TMWDEFS_FALSE;
        pBinFileTargValues->endpointIsTcpListening = (fileValues->typeOfEndpoint1_3 & ENDPOINTS_TCPLISTENING) ? TMWDEFS_TRUE : TMWDEFS_FALSE;
        pBinFileTargValues->endpointIsTcpDual = (fileValues->typeOfEndpoint1_3 & ENDPOINTS_TCPDUAL) ? TMWDEFS_TRUE : TMWDEFS_FALSE;
        pBinFileTargValues->endpointIsUDPDatagram = (fileValues->typeOfEndpoint1_3 & ENDPOINTS_UDPDATAGRAM) ? TMWDEFS_TRUE : TMWDEFS_FALSE;
        pBinFileTargValues->useEndpoint1_3_2 = TMWDEFS_TRUE;
      }


      /*1.3.3*/
      if(fileValues->fieldsUsed1_3 & CRTL_IPOFDEVICE1_3_3)
      {
        strncpy(pBinFileTargValues->ipAddress, fileValues->ipOfDevice1_3, TMWTARG_STR_LEN);
        pBinFileTargValues->useIpAddress1_3_3 = TMWDEFS_TRUE;
      }


      /*1.3.4*/
      if(fileValues->fieldsUsed1_3 & CRTL_SUBNETMASK1_3_4)
      {
        strncpy(pBinFileTargValues->subnetMask, fileValues->subnetMask1_3, TMWTARG_STR_LEN);
        pBinFileTargValues->useSubnetMask1_3_4 = TMWDEFS_TRUE;
      }

      /*1.3.5*/
      if(fileValues->fieldsUsed1_3 & CRTL_GATEWAYIPADDR1_3_5)
      {
        strncpy(pBinFileTargValues->gatewayIp, fileValues->gatewayIpAddr1_3, TMWTARG_STR_LEN);
        pBinFileTargValues->useGateWayIp1_3_5 = TMWDEFS_TRUE;
      }


      /*1.3.7*/
      if(fileValues->fieldsUsed1_3 & CRTL_IPADDRSTOACCEPTTCPANDUDP1_3_7)
      {
        TMWTYPES_UINT maxIpStringLen = 0;

        if(numberOfIPAddrs > 0)
        {
          maxIpStringLen = TMWTARG_STR_LEN / numberOfIPAddrs - 1;  /* minus one accounts for the ';' between addrs and the final '\0' */
          pBinFileTargValues->allowedConnIpList[0] = '\0';
        }

        for (i = 0; i < numberOfIPAddrs && i < DNPBNCFG_MAXNUMIPADDRS1_3; ++i)
        {
          strncat(pBinFileTargValues->allowedConnIpList, fileValues->ipAddrToAcceptTcpandUdp1_3[i], maxIpStringLen);

          if(i < numberOfIPAddrs - 1)
            strcat(pBinFileTargValues->allowedConnIpList, ";");
        }

        pBinFileTargValues->allowedConnIpList[TMWTARG_STR_LEN - 1] = '\0';
        pBinFileTargValues->useAllowedConnIpList1_3_7 = TMWDEFS_TRUE;
      }

        /*1.3.8*/
      if (fileValues->fieldsUsed1_3 & CRTL_TCPLISTENPORTNUMBER1_3_8)
      {
        pBinFileTargValues->tcpListenPort = (TMWTYPES_USHORT)fileValues->tcpListenPortNumber1_3;
        if (!((TMWTYPES_USHORT)fileValues->specialControls1_3 & SPCTRL_NOTAPPLICABLETCPLISTENPORT1_3_8))
          pBinFileTargValues->useTcpListenPort1_3_8 = TMWDEFS_TRUE;
      }

      /*1.3.9*/
      if (fileValues->fieldsUsed1_3 & CRTL_TCPLISTENPORTNUMBEROFREMOTE1_3_9)
      {
        pBinFileTargValues->tcpListenPortOfRemote = (TMWTYPES_USHORT)fileValues->tcpListenPortNumberOfRemote1_3;
        if (!((TMWTYPES_USHORT)fileValues->specialControls1_3 & SPCTRL_NOTAPPLICABLETCPLISTENPORTOFREMOTE1_3_9))
          pBinFileTargValues->useTcpListenPortOfRemote1_3_9 = TMWDEFS_TRUE;
      }

      /*1.3.11*/
      if(fileValues->fieldsUsed1_3 & CRTL_LOCALUDPPORT1_3_11)
      {
        pBinFileTargValues->useLocalUdpPort1_3_11 = TMWDEFS_TRUE;
        if (((TMWTYPES_USHORT)fileValues->specialControls1_3 & SPCTRL_LETSYSTEMCHOOSELOCALUDPPORT1_3_11))
        {
          /* used by masters only*/
          pBinFileTargValues->localUdpPort = TMWTARG_UDP_PORT_ANY;
        }
        else
        {
          pBinFileTargValues->localUdpPort = (TMWTYPES_USHORT)fileValues->localUdpPort1_3;
        }
      }

      /*1.3.12*/
      /* used by masters only */
      if(fileValues->fieldsUsed1_3 & CRTL_DESINTATIONUDPPORTFORDNP3REQ1_3_12)
      {
        pBinFileTargValues->destUdpPort = (TMWTYPES_USHORT)fileValues->destUdpPortForDnp3req1_3;
        pBinFileTargValues->useDestUpdPort1_3_12 = TMWDEFS_TRUE;
      }

      /*1.3.13*/
      /* used by outstations only */
      if(fileValues->fieldsUsed1_3 & CRTL_DESINTATIONUDPPORTFORINITIALUNSOLNULLRESP1_3_13)
      {
        pBinFileTargValues->destUdpPortForUnsol = (TMWTYPES_USHORT)fileValues->destUdpPortForInitialUnsolNullResp1_3;
        if (!((TMWTYPES_USHORT)fileValues->specialControls1_3 & SPCTRL_NONEDESINTATIONUDPPORTFORINITIALUNSOLNULLRESP1_3_13))
          pBinFileTargValues->useDestUdpPortForUnsol1_3_13 = TMWDEFS_TRUE;
      }

      /*1.3.14*/
      /* used by outstations only*/
      if(fileValues->fieldsUsed1_3 & CRTL_DESINTATIONUDPPORTFORRESPONSES1_3_14)
      {
        pBinFileTargValues->useSourcePortNumberForResponses = TMWDEFS_FALSE;

        if ((TMWTYPES_USHORT)fileValues->specialControls1_3 & SPCTRL_NONEDESINTATIONUDPPORTFORRESPONSES1_3_14)
          pBinFileTargValues->useDestUdpPortForResponses1_3_14 = TMWDEFS_FALSE;

        else if ((TMWTYPES_USHORT)fileValues->specialControls1_3 & SPCTRL_USESOURCEPORTNUMBERDESINTATIONUDPPORTFORRESP1_3_14)
        {
          pBinFileTargValues->useDestUdpPortForResponses1_3_14 = TMWDEFS_TRUE;
          pBinFileTargValues->useSourcePortNumberForResponses = TMWDEFS_TRUE;
        }
        else
        {
          pBinFileTargValues->useDestUdpPortForResponses1_3_14 = TMWDEFS_TRUE;
          pBinFileTargValues->destUdpPortForResponses = (TMWTYPES_USHORT)fileValues->destUdpPortForResponses1_3;
        }
      }
    }
  }

  return readOK;
}


/* function: dnpbncfg_ReadBinaryConfigSection1_4 */
static TMWTYPES_BOOL TMWDEFS_LOCAL dnpbncfg_ReadBinaryConfigSection1_4(
  FILE *fp,
  TMWTYPES_USHORT xmlVersionIndex, 
  DNPBNCFG_FILEVALUES *fileValues)
{
  size_t charRead;
  TMWTYPES_ULONG sectionSize = 0;
  TMWTYPES_BOOL readOK;

  TMWTYPES_UCHAR sectionSizeArray[DNPBNCFG_SECTION_SIZE_FIELD_SIZE];
  TMWTYPES_UCHAR field1[DNPBNCFG_CONTROL_SIZE1_4];
  TMWTYPES_UCHAR field2[DNPBNCFG_SPECIALCONTROL_SIZE1_4];
  TMWTYPES_UCHAR field3[DNPBNCFG_DATALINKADDRESS_SIZE1_4];
  TMWTYPES_UCHAR field4[DNPBNCFG_DNP3SOURCEADDRVALIDATION_SIZE1_4];
  TMWTYPES_UCHAR field5[DNPBNCFG_DNP3SOURCEADDRS_SIZE1_4];
  TMWTYPES_UCHAR field6[DNPBNCFG_SENDSCONFIRMEDUSERDATAFRAMES_SIZE1_4];
  TMWTYPES_UCHAR field7[DNPBNCFG_DATALINKLAYERCONFIRMTIMEOUT_SIZE1_4];
  TMWTYPES_UCHAR field8[DNPBNCFG_MAXDATALINKRETRIES_SIZE1_4];
  TMWTYPES_UCHAR field9[DNPBNCFG_MAXNUMOCTETSTXINDATALINKFRAME_SIZE1_4];
  TMWTYPES_UCHAR field10[DNPBNCFG_MAXNUMOCTETSRECEIVEDINDATALINKFRAME_SIZE1_4];
  TMWTARG_UNUSED_PARAM(xmlVersionIndex);

  charRead = fread(sectionSizeArray, 1, DNPBNCFG_SECTION_SIZE_FIELD_SIZE, fp);
  charRead = fread(field1, 1, DNPBNCFG_CONTROL_SIZE1_4, fp);
  charRead += fread(field2, 1, DNPBNCFG_SPECIALCONTROL_SIZE1_4, fp);
  charRead += fread(field3, 1, DNPBNCFG_DATALINKADDRESS_SIZE1_4, fp);
  charRead += fread(field4, 1, DNPBNCFG_DNP3SOURCEADDRVALIDATION_SIZE1_4, fp);
  charRead += fread(field5, 1, DNPBNCFG_DNP3SOURCEADDRS_SIZE1_4, fp);
  charRead += fread(field6, 1, DNPBNCFG_SENDSCONFIRMEDUSERDATAFRAMES_SIZE1_4, fp);
  charRead += fread(field7, 1, DNPBNCFG_DATALINKLAYERCONFIRMTIMEOUT_SIZE1_4, fp);
  charRead += fread(field8, 1, DNPBNCFG_MAXDATALINKRETRIES_SIZE1_4, fp);
  charRead += fread(field9, 1, DNPBNCFG_MAXNUMOCTETSTXINDATALINKFRAME_SIZE1_4, fp);
  charRead += fread(field10, 1, DNPBNCFG_MAXNUMOCTETSRECEIVEDINDATALINKFRAME_SIZE1_4, fp);

  tmwtarg_get32(sectionSizeArray, &sectionSize);

  readOK = (sectionSize == charRead);

  if (readOK)
  {
    tmwtarg_get32(field1, &fileValues->fieldsUsed1_4);
    tmwtarg_get32(field2, &fileValues->specialControls1_4);
    tmwtarg_get32(field3, &fileValues->dataLinkAddress1_4);
    tmwtarg_get32(field4, &fileValues->dnp3SourceAddrValidation1_4);
    tmwtarg_get32(field5, &fileValues->dnp3SourceAddrs1_4);
    tmwtarg_get32(field6, &fileValues->sendsConfirmedUserDataFrames1_4);
    tmwtarg_get32(field7, &fileValues->dataLinkLayerConfirmTimeout1_4);
    tmwtarg_get32(field8, &fileValues->maxDataLinkRetries1_4);
    tmwtarg_get32(field9, &fileValues->maxNumOctetsTxDataLinkFrame1_4);
    tmwtarg_get32(field10, &fileValues->maxNumOctetsRxDataLinkFrame1_4);
  }

  return readOK;
}

/* function: dnpbncfg_ReadBinaryConfigSection1_5 */
static TMWTYPES_BOOL TMWDEFS_LOCAL dnpbncfg_ReadBinaryConfigSection1_5(
  FILE *fp,
  TMWTYPES_USHORT xmlVersionIndex, 
  DNPBNCFG_FILEVALUES *fileValues)
{
  size_t charRead;
  TMWTYPES_ULONG sectionSize;
  TMWTYPES_BOOL readOK;

  TMWTYPES_UCHAR sectionSizeArray[DNPBNCFG_SECTION_SIZE_FIELD_SIZE];
  TMWTYPES_UCHAR field1[DNPBNCFG_CONTROL_SIZE1_5];
  TMWTYPES_UCHAR field2[DNPBNCFG_SPECIALCONTROL_SIZE1_5];
  TMWTYPES_UCHAR field3[DNPBNCFG_MAXTRANSMITTEDFRAGMENTSIZE_SIZE1_5];
  TMWTYPES_UCHAR field4[DNPBNCFG_MAXFILETRANSFERTRANSMITTEDFRAGMENTSIZE_SIZE1_5];
  TMWTYPES_UCHAR field5[DNPBNCFG_MAXRECEIVEDFRAGMENTSIZE_SIZE1_5];
  TMWTYPES_UCHAR field6[DNPBNCFG_MAXOBJECTSINCROBCONTROLREQUEST_SIZE1_5];
  TMWTYPES_UCHAR field7[DNPBNCFG_MAXOBJECTSINANALOGOUTPUTCONTROLREQUEST_SIZE1_5];
  TMWTYPES_UCHAR field8[DNPBNCFG_MAXOBJECTSINDATASETSCONTROLREQUEST_SIZE1_5];
  TMWTYPES_UCHAR field9[DNPBNCFG_SUPPORTSMIXEDOBJECTGROUPSINCONTROLREQUEST_SIZE1_5];
  TMWTARG_UNUSED_PARAM(xmlVersionIndex);

  charRead = fread(sectionSizeArray, 1, DNPBNCFG_SECTION_SIZE_FIELD_SIZE, fp);
  charRead = fread(field1, 1, DNPBNCFG_CONTROL_SIZE1_5, fp);
  charRead += fread(field2, 1, DNPBNCFG_SPECIALCONTROL_SIZE1_5, fp);
  charRead += fread(field3, 1, DNPBNCFG_MAXTRANSMITTEDFRAGMENTSIZE_SIZE1_5, fp);
  charRead += fread(field4, 1, DNPBNCFG_MAXFILETRANSFERTRANSMITTEDFRAGMENTSIZE_SIZE1_5, fp);
  charRead += fread(field5, 1, DNPBNCFG_MAXRECEIVEDFRAGMENTSIZE_SIZE1_5, fp);
  charRead += fread(field6, 1, DNPBNCFG_MAXOBJECTSINCROBCONTROLREQUEST_SIZE1_5, fp);
  charRead += fread(field7, 1, DNPBNCFG_MAXOBJECTSINANALOGOUTPUTCONTROLREQUEST_SIZE1_5, fp);
  charRead += fread(field8, 1, DNPBNCFG_MAXOBJECTSINDATASETSCONTROLREQUEST_SIZE1_5, fp);
  charRead += fread(field9, 1, DNPBNCFG_SUPPORTSMIXEDOBJECTGROUPSINCONTROLREQUEST_SIZE1_5, fp);

  tmwtarg_get32(sectionSizeArray, &sectionSize);

  readOK = (sectionSize == charRead);

  if (readOK)
  {
    tmwtarg_get32(field1, &fileValues->fieldsUsed1_5);
    tmwtarg_get32(field2, &fileValues->specialControls1_5);
    tmwtarg_get32(field3, &fileValues->maxTxFragmentSize1_5);
    tmwtarg_get32(field4, &fileValues->maxFileTransferTxFragSize1_5);
    tmwtarg_get32(field5, &fileValues->maxRxFragmentSize1_5);
    tmwtarg_get32(field6, &fileValues->maxObjsInCROBControlRequest1_5);
    tmwtarg_get32(field7, &fileValues->maxObjsInAnalogOutputCtrlReq1_5);
    tmwtarg_get32(field8, &fileValues->maxObjsInDataSetsCtrlReq1_5);
    tmwtarg_get32(field9, &fileValues->supportsMixedObjGrpsInCtrlReq1_5);
  }

  return readOK;
}

/* function: dnpbncfg_ReadBinaryConfigSection1_6 */
static TMWTYPES_BOOL TMWDEFS_LOCAL dnpbncfg_ReadBinaryConfigSection1_6(
  FILE *fp,
  TMWTYPES_USHORT xmlVersionIndex, 
  DNPBNCFG_FILEVALUES *fileValues)
{
  size_t charRead;
  TMWTYPES_ULONG sectionSize;
  TMWTYPES_BOOL readOK;

  TMWTYPES_UCHAR sectionSizeArray[DNPBNCFG_SECTION_SIZE_FIELD_SIZE];
  TMWTYPES_UCHAR field1[DNPBNCFG_CONTROL_SIZE1_6];
  TMWTYPES_UCHAR field2[DNPBNCFG_SPECIALCONTROL_SIZE1_6];
  TMWTYPES_UCHAR field3[DNPBNCFG_APPLLAYERCOMPLETERESPONSETIMEOUT_SIZE1_6];
  TMWTYPES_UCHAR field4[DNPBNCFG_APPLLAYERFRAGMENTRESPONSETIMEOUT_SIZE1_6];
  TMWTARG_UNUSED_PARAM(xmlVersionIndex);

  charRead = fread(sectionSizeArray, 1, DNPBNCFG_SECTION_SIZE_FIELD_SIZE, fp);
  charRead = fread(field1, 1, DNPBNCFG_CONTROL_SIZE1_6, fp);
  charRead += fread(field2, 1, DNPBNCFG_SPECIALCONTROL_SIZE1_6, fp);
  charRead += fread(field3, 1, DNPBNCFG_APPLLAYERCOMPLETERESPONSETIMEOUT_SIZE1_6, fp);
  charRead += fread(field4, 1, DNPBNCFG_APPLLAYERFRAGMENTRESPONSETIMEOUT_SIZE1_6, fp);

  tmwtarg_get32(sectionSizeArray, &sectionSize);

  readOK = (sectionSize == charRead);
  if (readOK)
  {
    tmwtarg_get32(field1, &fileValues->fieldsUsed1_6);
    tmwtarg_get32(field2, &fileValues->specialControls1_6);
    tmwtarg_get32(field3, &fileValues->applLayerCompleteRespTimeout1_6);
    tmwtarg_get32(field4, &fileValues->applLayerFragRespTimeout1_6);
  }

  return readOK;
}


/* function: dnpbncfg_ReadBinaryConfigSection1_7 */
static TMWTYPES_BOOL TMWDEFS_LOCAL dnpbncfg_ReadBinaryConfigSection1_7(
  FILE *fp,
  TMWTYPES_USHORT xmlVersionIndex, 
  DNPBNCFG_FILEVALUES *fileValues)
{
  size_t charRead;
  TMWTYPES_ULONG sectionSize;
  TMWTYPES_BOOL readOK;

  TMWTYPES_UCHAR sectionSizeArray[DNPBNCFG_SECTION_SIZE_FIELD_SIZE];
  TMWTYPES_UCHAR field1[DNPBNCFG_CONTROL_SIZE1_7];
  TMWTYPES_UCHAR field2[DNPBNCFG_SPECIALCONTROL_SIZE1_7];
  TMWTYPES_UCHAR field3[DNPBNCFG_APPLICATIONLAYERCONFIRMTIMEOUT_SIZE1_7];
  TMWTYPES_UCHAR field4[DNPBNCFG_TIMESYNCREQUIRED_SIZE1_7];
  TMWTYPES_UCHAR field5[DNPBNCFG_FILEHANDLETIMEOUT_SIZE1_7];
  TMWTYPES_UCHAR field6[DNPBNCFG_EVENTBUFFEROVERFLOWBEHAVIOR_SIZE1_7];
  TMWTARG_UNUSED_PARAM(xmlVersionIndex);

  charRead = fread(sectionSizeArray, 1, DNPBNCFG_SECTION_SIZE_FIELD_SIZE, fp);
  charRead = fread(field1, 1, DNPBNCFG_CONTROL_SIZE1_7, fp);
  charRead += fread(field2, 1, DNPBNCFG_SPECIALCONTROL_SIZE1_7, fp);
  charRead += fread(field3, 1, DNPBNCFG_APPLICATIONLAYERCONFIRMTIMEOUT_SIZE1_7, fp);
  charRead += fread(field4, 1, DNPBNCFG_TIMESYNCREQUIRED_SIZE1_7, fp);
  charRead += fread(field5, 1, DNPBNCFG_FILEHANDLETIMEOUT_SIZE1_7, fp);
  charRead += fread(field6, 1, DNPBNCFG_EVENTBUFFEROVERFLOWBEHAVIOR_SIZE1_7, fp);

  tmwtarg_get32(sectionSizeArray, &sectionSize);

  readOK = (sectionSize == charRead);

  if (readOK)
  {
    tmwtarg_get32(field1, &fileValues->fieldsUsed1_7);
    tmwtarg_get32(field2, &fileValues->specialControls1_7);
    tmwtarg_get32(field3, &fileValues->applLayerConfirmTimeout1_7);
    tmwtarg_get32(field4, &fileValues->timeSyncRequired1_7);
    tmwtarg_get32(field5, &fileValues->fileHandleTimeout1_7);
    tmwtarg_get32(field6, &fileValues->eventBufOverflowBehavior1_7);
  }

  return readOK;
}


/* function: dnpbncfg_ReadBinaryConfigSection1_8 */
static TMWTYPES_BOOL TMWDEFS_LOCAL dnpbncfg_ReadBinaryConfigSection1_8(
  FILE *fp,
  TMWTYPES_USHORT xmlVersionIndex, 
  DNPBNCFG_FILEVALUES *fileValues)
{
  size_t charRead;
  TMWTYPES_ULONG sectionSize;
  TMWTYPES_BOOL readOK;

  TMWTYPES_UCHAR sectionSizeArray[DNPBNCFG_SECTION_SIZE_FIELD_SIZE];
  TMWTYPES_UCHAR field1[DNPBNCFG_CONTROL_SIZE1_8];
  TMWTYPES_UCHAR field2[DNPBNCFG_SPECIALCONTROL_SIZE1_8];
  TMWTYPES_UCHAR field3[DNPBNCFG_MASTERDATALINKADDRESS_SIZE1_8];
  TMWTYPES_UCHAR field4[DNPBNCFG_UNSOLICITEDRESPONSECONFIRMATIONTIMEOUT_SIZE1_8];
  TMWTYPES_UCHAR field5[DNPBNCFG_MAXUNSOLICITEDRETRIES_SIZE1_8];
  TMWTARG_UNUSED_PARAM(xmlVersionIndex);

  charRead = fread(sectionSizeArray, 1, DNPBNCFG_SECTION_SIZE_FIELD_SIZE, fp);
  charRead = fread(field1, 1, DNPBNCFG_CONTROL_SIZE1_8, fp);
  charRead += fread(field2, 1, DNPBNCFG_SPECIALCONTROL_SIZE1_8, fp);
  charRead += fread(field3, 1, DNPBNCFG_MASTERDATALINKADDRESS_SIZE1_8, fp);
  charRead += fread(field4, 1, DNPBNCFG_UNSOLICITEDRESPONSECONFIRMATIONTIMEOUT_SIZE1_8, fp);
  charRead += fread(field5, 1, DNPBNCFG_MAXUNSOLICITEDRETRIES_SIZE1_8, fp);

  tmwtarg_get32(sectionSizeArray, &sectionSize);

  readOK = (sectionSize == charRead);

  if (readOK)
  {
    tmwtarg_get32(field1, &fileValues->fieldsUsed1_8);
    tmwtarg_get32(field2, &fileValues->specialControls1_8);
    tmwtarg_get32(field3, &fileValues->masterDataLinkAddr1_8);
    tmwtarg_get32(field4, &fileValues->unsolRespConfirmTimeout1_8);
    tmwtarg_get32(field5, &fileValues->maxUnsolicitedRetries1_8);
  }

  return readOK;
}

/* function: dnpbncfg_ReadBinaryConfigSection1_9 */
static TMWTYPES_BOOL TMWDEFS_LOCAL dnpbncfg_ReadBinaryConfigSection1_9(
  FILE *fp,
  TMWTYPES_USHORT xmlVersionIndex, 
  DNPBNCFG_FILEVALUES *fileValues)
{
  size_t charRead;
  TMWTYPES_ULONG sectionSize;
  TMWTYPES_BOOL readOK;

  TMWTYPES_UCHAR sectionSizeArray[DNPBNCFG_SECTION_SIZE_FIELD_SIZE];
  TMWTYPES_UCHAR field1[DNPBNCFG_CONTROL_SIZE1_9];
  TMWTYPES_UCHAR field2[DNPBNCFG_SPECIALCONTROL_SIZE1_9];
  TMWTYPES_UCHAR field3[DNPBNCFG_NUMBEROFCLASSONEEVENTS_SIZE1_9];
  TMWTYPES_UCHAR field4[DNPBNCFG_NUMBEROFCLASSTWOEVENTS_SIZE1_9];
  TMWTYPES_UCHAR field5[DNPBNCFG_NUMBEROFCLASSTHREEEVENTS_SIZE1_9];
  TMWTYPES_UCHAR field6[DNPBNCFG_HOLDTIMEAFTERCLASSONEEVENT_SIZE1_9];
  TMWTYPES_UCHAR field7[DNPBNCFG_HOLDTIMEAFTERCLASSTWOEVENT_SIZE1_9];
  TMWTYPES_UCHAR field8[DNPBNCFG_HOLDTIMEAFTERCLASSTHREEEVENT_SIZE1_9];
  TMWTARG_UNUSED_PARAM(xmlVersionIndex);

  charRead = fread(sectionSizeArray, 1, DNPBNCFG_SECTION_SIZE_FIELD_SIZE, fp);
  charRead = fread(field1, 1, DNPBNCFG_CONTROL_SIZE1_9, fp);
  charRead += fread(field2, 1, DNPBNCFG_SPECIALCONTROL_SIZE1_9, fp);
  charRead += fread(field3, 1, DNPBNCFG_NUMBEROFCLASSONEEVENTS_SIZE1_9, fp);
  charRead += fread(field4, 1, DNPBNCFG_NUMBEROFCLASSTWOEVENTS_SIZE1_9, fp);
  charRead += fread(field5, 1, DNPBNCFG_NUMBEROFCLASSTHREEEVENTS_SIZE1_9, fp);
  charRead += fread(field6, 1, DNPBNCFG_HOLDTIMEAFTERCLASSONEEVENT_SIZE1_9, fp);
  charRead += fread(field7, 1, DNPBNCFG_HOLDTIMEAFTERCLASSTWOEVENT_SIZE1_9, fp);
  charRead += fread(field8, 1, DNPBNCFG_HOLDTIMEAFTERCLASSTHREEEVENT_SIZE1_9, fp);

  tmwtarg_get32(sectionSizeArray, &sectionSize);

  readOK = (sectionSize == charRead);

  if (readOK)
  {
    tmwtarg_get32(field1, &fileValues->fieldsUsed1_9);
    tmwtarg_get32(field2, &fileValues->specialControls1_9);
    tmwtarg_get32(field3, &fileValues->numClassOneEvents1_9);
    tmwtarg_get32(field4, &fileValues->numClassTwoEvents1_9);
    tmwtarg_get32(field5, &fileValues->numClassThreeEvents1_9);
    tmwtarg_get32(field6, &fileValues->holdTimeAfterClassOneEvent1_9);
    tmwtarg_get32(field7, &fileValues->holdTimeAfterClassTwoEvent1_9);
    tmwtarg_get32(field8, &fileValues->holdTimeAfterClassThreeEvent1_9);
  }

  return readOK;
}


/* function: dnpbncfg_ReadBinaryConfigSection1_10 */
static TMWTYPES_BOOL TMWDEFS_LOCAL dnpbncfg_ReadBinaryConfigSection1_10(
  FILE *fp,
  TMWTYPES_USHORT xmlVersionIndex, 
  DNPBNCFG_FILEVALUES *fileValues)
{
  size_t charRead;
  TMWTYPES_ULONG sectionSize;
  TMWTYPES_BOOL readOK;

  TMWTYPES_UCHAR sectionSizeArray[DNPBNCFG_SECTION_SIZE_FIELD_SIZE];
  TMWTYPES_UCHAR field1[DNPBNCFG_CONTROL_SIZE1_10];
  TMWTYPES_UCHAR field2[DNPBNCFG_SPECIALCONTROL_SIZE1_10];
  TMWTYPES_UCHAR field3[DNPBNCFG_OUTSTATIONSETSIIN14_SIZE1_10];
  TMWTYPES_UCHAR field4[DNPBNCFG_AFTERLASTTIMESYNCOUTSTATIONSETSIIN14_SIZE1_10];
  TMWTYPES_UCHAR field5[DNPBNCFG_WHENTIMEERROREXCEEDSOUTSTATIONSETSIIN14_SIZE1_10];
  TMWTARG_UNUSED_PARAM(xmlVersionIndex);

  charRead = fread(sectionSizeArray, 1, DNPBNCFG_SECTION_SIZE_FIELD_SIZE, fp);
  charRead = fread(field1, 1, DNPBNCFG_CONTROL_SIZE1_10, fp);
  charRead += fread(field2, 1, DNPBNCFG_SPECIALCONTROL_SIZE1_10, fp);
  charRead += fread(field3, 1, DNPBNCFG_OUTSTATIONSETSIIN14_SIZE1_10, fp);
  charRead += fread(field4, 1, DNPBNCFG_AFTERLASTTIMESYNCOUTSTATIONSETSIIN14_SIZE1_10, fp);
  charRead += fread(field5, 1, DNPBNCFG_WHENTIMEERROREXCEEDSOUTSTATIONSETSIIN14_SIZE1_10, fp);

  tmwtarg_get32(sectionSizeArray, &sectionSize);

  readOK = (sectionSize == charRead);

  if (readOK)
  {
    tmwtarg_get32(field1, &fileValues->fieldsUsed1_10);
    tmwtarg_get32(field2, &fileValues->specialControls1_10);
    tmwtarg_get32(field3, &fileValues->outStationSetsIIN14_1_10);
    tmwtarg_get32(field4, &fileValues->afterLastTimeSyncOutstationSetsIIN14_1_10);
    tmwtarg_get32(field5, &fileValues->whenTimeErrExceedsOutstationSetsIIN14_1_10);
  }

  return readOK;
}


/* function: dnpbncfg_readBinaryConfigSection1_11 */
static TMWTYPES_BOOL TMWDEFS_LOCAL dnpbncfg_ReadBinaryConfigSection1_11(
  FILE *fp,
  TMWTYPES_USHORT xmlVersionIndex, 
  DNPBNCFG_FILEVALUES *fileValues)
{
  size_t charRead;
  TMWTYPES_ULONG sectionSize;
  TMWTYPES_BOOL readOK;

  TMWTYPES_UCHAR sectionSizeArray[DNPBNCFG_SECTION_SIZE_FIELD_SIZE];
  TMWTYPES_UCHAR field1[DNPBNCFG_CONTROL_SIZE1_11];
  TMWTYPES_UCHAR field2[DNPBNCFG_OUTSTATIONLOCATION_SIZE1_11+1];
  TMWTYPES_UCHAR field3[DNPBNCFG_OUTSTATIONID_SIZE1_11+1];
  TMWTYPES_UCHAR field4[DNPBNCFG_OUTSTATIONNAME_SIZE1_11+1];
  TMWTARG_UNUSED_PARAM(xmlVersionIndex);

  charRead = fread(sectionSizeArray, 1, DNPBNCFG_SECTION_SIZE_FIELD_SIZE, fp);
  charRead = fread(field1, 1, DNPBNCFG_CONTROL_SIZE1_11, fp);
  charRead += fread(field2, 1, DNPBNCFG_OUTSTATIONLOCATION_SIZE1_11, fp);
  field2[DNPBNCFG_OUTSTATIONLOCATION_SIZE1_11] = 0; /* Null terminate */
  charRead += fread(field3, 1, DNPBNCFG_OUTSTATIONID_SIZE1_11, fp);
  field3[DNPBNCFG_OUTSTATIONID_SIZE1_11] = 0; /* Null terminate */
  charRead += fread(field4, 1, DNPBNCFG_OUTSTATIONNAME_SIZE1_11, fp);
  field4[DNPBNCFG_OUTSTATIONNAME_SIZE1_11] = 0; /* Null terminate */

  tmwtarg_get32(sectionSizeArray, &sectionSize);

  readOK = (sectionSize == charRead);

  if (readOK)
  {
    tmwtarg_get32(field1, &fileValues->fieldsUsed1_11);

    strncpy(fileValues->outstationLocation1_11, (const char *)field2, DNPBNCFG_OUTSTATIONLOCATION_SIZE1_11+1);
    strncpy(fileValues->outstationId1_11, (const char *)field3, DNPBNCFG_OUTSTATIONID_SIZE1_11+1);
    strncpy(fileValues->outstationName1_11, (const char *)field4, DNPBNCFG_OUTSTATIONNAME_SIZE1_11+1);
  }

  return readOK;
}


/* function: dnpbncfg_ReadBinaryConfigSection1_12 */
static TMWTYPES_BOOL TMWDEFS_LOCAL dnpbncfg_ReadBinaryConfigSection1_12(
  FILE *fp,
  TMWTYPES_USHORT xmlVersionIndex, 
  DNPBNCFG_FILEVALUES *fileValues)
{
  size_t charRead;
  TMWTYPES_ULONG sectionSize;
  TMWTYPES_BOOL readOK;

  TMWTYPES_UCHAR sectionSizeArray[DNPBNCFG_SECTION_SIZE_FIELD_SIZE];
  TMWTYPES_UCHAR field1[DNPBNCFG_CONTROL_SIZE1_12];
  TMWTYPES_UCHAR field2[DNPBNCFG_SPECIALCONTROL_SIZE1_12];
  TMWTYPES_UCHAR field3[DNPBNCFG_VERSIONOFSECUREAUTHENTICATIONSUPPORTED_SIZE1_12];
  TMWTYPES_UCHAR field4[DNPBNCFG_MAXNUMBERUSERS_SIZE1_12];
  TMWTYPES_UCHAR field5[DNPBNCFG_SECURITYRESPONSETIMEOUT_SIZE1_12];
  TMWTYPES_UCHAR field6[DNPBNCFG_SESSIONKEYCHANGEINTERVAL_SIZE1_12];
  TMWTYPES_UCHAR field7[DNPBNCFG_SESSIONKEYCHANGEMESSAGECOUNT_SIZE1_12];
  TMWTYPES_UCHAR field8[DNPBNCFG_MAXERRORCOUNT_SIZE1_12];
  TMWTYPES_UCHAR field9[DNPBNCFG_HMACALGORITHMREQUESTED_SIZE1_12];
  TMWTYPES_UCHAR field10[DNPBNCFG_KEYWRAPALGORITHM_SIZE1_12];
  TMWTYPES_UCHAR field11[DNPBNCFG_ADDITIONALCRITICALFCS_SIZE1_12];
  TMWTYPES_UCHAR field12[DNPBNCFG_REMOTEKEYUPDATECHANGE_SIZE1_12];

  charRead = fread(sectionSizeArray, 1, DNPBNCFG_SECTION_SIZE_FIELD_SIZE, fp);
  charRead = fread(field1, 1, DNPBNCFG_CONTROL_SIZE1_12, fp);
  charRead += fread(field2, 1, DNPBNCFG_SPECIALCONTROL_SIZE1_12, fp);
  charRead += fread(field3, 1, DNPBNCFG_VERSIONOFSECUREAUTHENTICATIONSUPPORTED_SIZE1_12, fp);
  charRead += fread(field4, 1, DNPBNCFG_MAXNUMBERUSERS_SIZE1_12, fp);
  charRead += fread(field5, 1, DNPBNCFG_SECURITYRESPONSETIMEOUT_SIZE1_12, fp);
  charRead += fread(field6, 1, DNPBNCFG_SESSIONKEYCHANGEINTERVAL_SIZE1_12, fp);
  charRead += fread(field7, 1, DNPBNCFG_SESSIONKEYCHANGEMESSAGECOUNT_SIZE1_12, fp);
  charRead += fread(field8, 1, DNPBNCFG_MAXERRORCOUNT_SIZE1_12, fp);
  charRead += fread(field9, 1, DNPBNCFG_HMACALGORITHMREQUESTED_SIZE1_12, fp);
  charRead += fread(field10, 1, DNPBNCFG_KEYWRAPALGORITHM_SIZE1_12, fp);
  charRead += fread(field11, 1, DNPBNCFG_ADDITIONALCRITICALFCS_SIZE1_12, fp);

  if (xmlVersionIndex >= 2)
    charRead += fread(field12, 1, DNPBNCFG_REMOTEKEYUPDATECHANGE_SIZE1_12, fp);    /* only read if Nov2013 device profile or newer */

  tmwtarg_get32(sectionSizeArray, &sectionSize);

  readOK = (sectionSize == charRead);

  if(readOK)
  {
    tmwtarg_get32(field1, &fileValues->fieldsUsed1_12);
    tmwtarg_get32(field2, &fileValues->specialControls1_12);
    tmwtarg_get32(field3, &fileValues->versionSecureAuthSupported1_12);
    tmwtarg_get32(field4, &fileValues->maxNumberUsers1_12);
    tmwtarg_get32(field5, &fileValues->securityResponseTimeout1_12);
    tmwtarg_get32(field6, &fileValues->sessionKeyChangeInterval1_12);
    tmwtarg_get32(field7, &fileValues->sessionKeyChangeMessageCnt1_12);
    tmwtarg_get32(field8, &fileValues->maxErrorCount1_12);
    tmwtarg_get32(field9, &fileValues->hmacAlgorithmRequested1_12);
    tmwtarg_get32(field10, &fileValues->keyWrapAlgorithm1_12);
    tmwtarg_get32(field11, &fileValues->additionalCriticalFcs1_12);
    if (xmlVersionIndex >= 2)
      tmwtarg_get32(field11, &fileValues->remoteKeyUpdateChange1_12);
  }


  return readOK;
}

/* function: dnpbncfg_ReadBinaryConfigSection1_13 */
static TMWTYPES_BOOL TMWDEFS_LOCAL dnpbncfg_ReadBinaryConfigSection1_13(
  FILE *fp,
  TMWTYPES_USHORT xmlVersionIndex, 
  DNPBNCFG_FILEVALUES *fileValues)
{
  size_t charRead;
  TMWTYPES_ULONG sectionSize;
  TMWTYPES_BOOL readOK;

  TMWTYPES_UCHAR sectionSizeArray[DNPBNCFG_SECTION_SIZE_FIELD_SIZE];
  TMWTYPES_UCHAR field1[DNPBNCFG_CONTROL_SIZE1_13];
  TMWTYPES_UCHAR field2[DNPBNCFG_SPECIALCONTROL_SIZE1_13];
  TMWTARG_UNUSED_PARAM(xmlVersionIndex);

  charRead = fread(sectionSizeArray, 1, DNPBNCFG_SECTION_SIZE_FIELD_SIZE, fp);
  charRead = fread(field1, 1, DNPBNCFG_CONTROL_SIZE1_13, fp);
  charRead += fread(field2, 1, DNPBNCFG_SPECIALCONTROL_SIZE1_13, fp);

  tmwtarg_get32(sectionSizeArray, &sectionSize);

  readOK = (sectionSize == charRead);

  if(readOK)
  {
    tmwtarg_get32(field1, &fileValues->fieldsUsed1_13);
    tmwtarg_get32(field2, &fileValues->specialControls1_13);
  }

  return readOK;
}





TMWTYPES_BOOL TMWDEFS_GLOBAL dnpbncfg_ReadBinaryConfigFile(
  FILE *fp, 
  DNPBNCFG_FILEVALUES *pBinFileValues,
  TMWTARG_BINFILE_VALS *pBinFileTargValues,
  TMWTYPES_USHORT *pXMLVersionIndex)
{
  TMWTYPES_BOOL success = TMWDEFS_FALSE;

  /* init control variables for available target values */
  pBinFileTargValues->useSupportedComm1_1_13 = TMWDEFS_FALSE;
  pBinFileTargValues->useSerialPortName1_2_1 = TMWDEFS_FALSE;
  pBinFileTargValues->useBuadRate1_2_3 = TMWDEFS_FALSE;
  pBinFileTargValues->useIpPortName1_3_1 = TMWDEFS_FALSE;
  pBinFileTargValues->useEndpoint1_3_2 = TMWDEFS_FALSE;
  pBinFileTargValues->useIpAddress1_3_3 = TMWDEFS_FALSE;
  pBinFileTargValues->useSubnetMask1_3_4 = TMWDEFS_FALSE;
  pBinFileTargValues->useGateWayIp1_3_5 = TMWDEFS_FALSE;
  pBinFileTargValues->useAllowedConnIpList1_3_7 = TMWDEFS_FALSE;
  pBinFileTargValues->useTcpListenPort1_3_8 = TMWDEFS_FALSE;
  pBinFileTargValues->useTcpListenPortOfRemote1_3_9 = TMWDEFS_FALSE;
  pBinFileTargValues->useLocalUdpPort1_3_11 = TMWDEFS_FALSE;
  pBinFileTargValues->useDestUpdPort1_3_12 = TMWDEFS_FALSE;
  pBinFileTargValues->useDestUdpPortForUnsol1_3_13 = TMWDEFS_FALSE;
  pBinFileTargValues->useDestUdpPortForResponses1_3_14 = TMWDEFS_FALSE;


  if (fp != NULL)
  {
    success = dnpbncfg_ReadBinaryConfigHeader(fp, pBinFileValues, pXMLVersionIndex);

    if (success)
      success = dnpbncfg_ReadBinaryConfigSection1_1(fp, *pXMLVersionIndex, pBinFileValues, pBinFileTargValues);

    if (success)
      success = dnpbncfg_ReadBinaryConfigSection1_2(fp, *pXMLVersionIndex, pBinFileValues, pBinFileTargValues);

    if (success)
      success = dnpbncfg_ReadBinaryConfigSection1_3(fp, *pXMLVersionIndex, pBinFileValues, pBinFileTargValues);

    if (success)
      success = dnpbncfg_ReadBinaryConfigSection1_4(fp, *pXMLVersionIndex, pBinFileValues);

    if (success)
      success = dnpbncfg_ReadBinaryConfigSection1_5(fp, *pXMLVersionIndex, pBinFileValues);

    if (success)
      success = dnpbncfg_ReadBinaryConfigSection1_6(fp, *pXMLVersionIndex, pBinFileValues);

    if (success)
      success = dnpbncfg_ReadBinaryConfigSection1_7(fp, *pXMLVersionIndex, pBinFileValues);

    if (success)
      success = dnpbncfg_ReadBinaryConfigSection1_8(fp, *pXMLVersionIndex, pBinFileValues);

    if (success)
      success = dnpbncfg_ReadBinaryConfigSection1_9(fp, *pXMLVersionIndex, pBinFileValues);

    if (success)
      success = dnpbncfg_ReadBinaryConfigSection1_10(fp, *pXMLVersionIndex, pBinFileValues);

    if (success)
      success = dnpbncfg_ReadBinaryConfigSection1_11(fp, *pXMLVersionIndex, pBinFileValues);

    if (success)
      success = dnpbncfg_ReadBinaryConfigSection1_12(fp, *pXMLVersionIndex, pBinFileValues);

    if (success)
      success = dnpbncfg_ReadBinaryConfigSection1_13(fp, *pXMLVersionIndex, pBinFileValues);

    fclose(fp);

    if(success)
      pBinFileTargValues->binFileIsOutstation = pBinFileValues->isOutStation;

  }

  return success;
}

#endif
