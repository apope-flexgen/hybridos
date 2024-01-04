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
 * description: Slave DNP Device Profile XML generation
 */
#include "tmwscl/dnp/dnpdiag.h"
#include "tmwscl/utils/tmwtarg.h"
#include "tmwscl/utils/tmwcnfg.h"
#include "tmwscl/utils/tmwdefs.h"
#include "tmwscl/dnp/sdnpdata.h"
#include "tmwscl/dnp/sdnputil.h"
#include "tmwscl/dnp/sdnpcnfg.h"
#include "tmwscl/dnp/sdnpxml2.h"
#include "tmwscl/dnp/sdnpo070.h"
#include "tmwscl/dnp/sdnpdiag.h"
#if DNPCNFG_SUPPORT_AUTHENTICATION
#include "tmwscl/dnp/sdnpauth.h"
#endif

#if SDNPDATA_SUPPORT_XML2

#define SDNPXML_KEYWRAP_PER_USER (TMWTYPES_UCHAR)-1

/* function: _appendString */
static void TMWDEFS_GLOBAL _appendString(
  SDNPSESN *pSDNPSession,
  TMWTYPES_CHAR  *pStr2,
  TMWTYPES_ULONG *curLen,
  TMWTYPES_BOOL   bGetLen)
{
  TMWTYPES_UINT length;
  TMWTYPES_INT i;
  length = *curLen;

  if (bGetLen == TMWDEFS_FALSE)
  {
    for(i=0; i<pSDNPSession->xml2CurIndent;i++)
    {
      int j;
      for(j=0; j<pSDNPSession->xml2IndentSize; j++)
        pSDNPSession->pXml2Buffer[length++] = (TMWTYPES_CHAR)0x20;
    }
    /* null terminate after indent was added */
    pSDNPSession->pXml2Buffer[length] = (TMWTYPES_CHAR)0;

    strcat(pSDNPSession->pXml2Buffer, pStr2);
  }
  else
  {
    length += (pSDNPSession->xml2CurIndent*pSDNPSession->xml2IndentSize);
  }
  *curLen = length + (TMWTYPES_ULONG)strlen(pStr2);
}

/* _valueString */
static void _valueString(SDNPSESN *pSDNPSession, TMWTYPES_CHAR *pName, TMWTYPES_CHAR *pValue, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  TMWTYPES_CHAR buf[512];
  if((pValue != TMWDEFS_NULL) && (*pValue != 0))
  {
    tmwtarg_snprintf(buf, sizeof(buf), "<%s>%s</%s>\n", pName, pValue, pName);
    _appendString(pSDNPSession, buf, curLen, bGetLen);
  }
}

/* _currentValueString */
static void _currentValueString(SDNPSESN *pSDNPSession, TMWTYPES_CHAR *pName, const TMWTYPES_CHAR *pValue, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  TMWTYPES_CHAR buf[300];

  if(pName != TMWDEFS_NULL)
  {
    tmwtarg_snprintf(buf, sizeof(buf), "<%s>\n", pName);
    _appendString(pSDNPSession, buf, curLen, bGetLen);
  }

  tmwtarg_snprintf(buf, sizeof(buf), "<currentValue>%s</currentValue>\n", pValue);
  pSDNPSession->xml2CurIndent++;
  _appendString(pSDNPSession, buf, curLen, bGetLen);
  pSDNPSession->xml2CurIndent--;

  if(pName != TMWDEFS_NULL)
  {
    tmwtarg_snprintf(buf, sizeof(buf), "</%s>\n", pName);
    _appendString(pSDNPSession, buf, curLen, bGetLen);
  }
}

/* _currentValueNameString */
static void _currentValueNameString(SDNPSESN *pSDNPSession, TMWTYPES_CHAR *pName, TMWTYPES_CHAR *pValueName, TMWTYPES_CHAR *pValue, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  TMWTYPES_CHAR buf[512];

  if(pName != TMWDEFS_NULL)
  {
    tmwtarg_snprintf(buf, sizeof(buf), "<%s>\n", pName);
    _appendString(pSDNPSession, buf, curLen, bGetLen);
  }

  pSDNPSession->xml2CurIndent++;
  _appendString(pSDNPSession, "<currentValue>\n", curLen, bGetLen);

  tmwtarg_snprintf(buf, sizeof(buf), "<%s>%s</%s>\n", pValueName, pValue, pValueName);
  pSDNPSession->xml2CurIndent++;
  _appendString(pSDNPSession, buf, curLen, bGetLen);

  pSDNPSession->xml2CurIndent--;
  _appendString(pSDNPSession, "</currentValue>\n", curLen, bGetLen);

  if(pName != TMWDEFS_NULL)
  {
    tmwtarg_snprintf(buf, sizeof(buf), "</%s>\n", pName);
    pSDNPSession->xml2CurIndent--;
    _appendString(pSDNPSession, buf, curLen, bGetLen);
  }
}

/* _currentValueValueString */
static void _currentValueValueString(SDNPSESN *pSDNPSession, TMWTYPES_CHAR *pName, TMWTYPES_CHAR *pValue, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  _currentValueNameString(pSDNPSession, pName, "value", pValue, curLen, bGetLen);
}

/* _currentValueNameDecimalString */
static void _currentValueNameDecimalString(SDNPSESN *pSDNPSession, TMWTYPES_CHAR *pName, TMWTYPES_CHAR *pValueName, TMWTYPES_ULONG value, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  TMWTYPES_CHAR buf[512];

  if(pName != TMWDEFS_NULL)
  {
    tmwtarg_snprintf(buf, sizeof(buf), "<%s>\n", pName);
    _appendString(pSDNPSession, buf, curLen, bGetLen);
  }

  pSDNPSession->xml2CurIndent++;
  _appendString(pSDNPSession, "<currentValue>\n", curLen, bGetLen);

  tmwtarg_snprintf(buf, sizeof(buf), "<%s>%d</%s>\n", pValueName, value, pValueName);
  pSDNPSession->xml2CurIndent++;
  _appendString(pSDNPSession, buf, curLen, bGetLen);

    pSDNPSession->xml2CurIndent--;
  _appendString(pSDNPSession, "</currentValue>\n", curLen, bGetLen);

  if(pName != TMWDEFS_NULL)
  {
    tmwtarg_snprintf(buf, sizeof(buf), "</%s>\n", pName);
    pSDNPSession->xml2CurIndent--;
    _appendString(pSDNPSession, buf, curLen, bGetLen);
  }
}

#if SDNPDATA_SUPPORT_OBJ10
#if TMWCNFG_SUPPORT_DOUBLE
/* _currentValueNameFloatString */
static void _currentValueNameFloatString(SDNPSESN *pSDNPSession, TMWTYPES_CHAR *pName, TMWTYPES_CHAR *pValueName, TMWTYPES_DOUBLE value, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  TMWTYPES_CHAR buf[512];

  if(pName != TMWDEFS_NULL)
  {
    tmwtarg_snprintf(buf, sizeof(buf), "<%s>\n", pName);
    _appendString(pSDNPSession, buf, curLen, bGetLen);
  }

  pSDNPSession->xml2CurIndent++;
  _appendString(pSDNPSession, "<currentValue>\n", curLen, bGetLen);

  tmwtarg_snprintf(buf, sizeof(buf), "<%s>%g</%s>\n", pValueName, value, pValueName);
  pSDNPSession->xml2CurIndent++;
  _appendString(pSDNPSession, buf, curLen, bGetLen);

    pSDNPSession->xml2CurIndent--;
  _appendString(pSDNPSession, "</currentValue>\n", curLen, bGetLen);

  if(pName != TMWDEFS_NULL)
  {
    tmwtarg_snprintf(buf, sizeof(buf), "</%s>\n", pName);
    pSDNPSession->xml2CurIndent--;
    _appendString(pSDNPSession, buf, curLen, bGetLen);
  }
}
#endif
#endif

/* _currentValueDecimalString */
static void _currentValueDecimalString(SDNPSESN *pSDNPSession, TMWTYPES_CHAR *pName, TMWTYPES_ULONG value, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  _currentValueNameDecimalString(pSDNPSession, pName, "value", value, curLen, bGetLen);
}

/* _valueOther */
static void _valueOther(SDNPSESN *pSDNPSession, TMWTYPES_CHAR *pValue, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  TMWTYPES_CHAR buf[300];

  pSDNPSession->xml2CurIndent++;
  _appendString(pSDNPSession, "<other>\n", curLen, bGetLen);

  tmwtarg_snprintf(buf, sizeof(buf), "<explanation>%s</explanation>\n", pValue);
  pSDNPSession->xml2CurIndent++;
  _appendString(pSDNPSession, buf, curLen, bGetLen);

  pSDNPSession->xml2CurIndent--;
  _appendString(pSDNPSession, "</other>\n", curLen, bGetLen);
  pSDNPSession->xml2CurIndent--;
}

/* _currentValueOther */
static void _currentValueOther(SDNPSESN *pSDNPSession, TMWTYPES_CHAR *pName, TMWTYPES_CHAR *pValue, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  TMWTYPES_CHAR buf[300];

  if(pName != TMWDEFS_NULL)
  {
    tmwtarg_snprintf(buf, sizeof(buf), "<%s>\n", pName);;
    _appendString(pSDNPSession, buf, curLen, bGetLen);
  }

  pSDNPSession->xml2CurIndent++;
  _appendString(pSDNPSession, "<currentValue>\n", curLen, bGetLen);

  _valueOther(pSDNPSession, pValue, curLen, bGetLen);

  _appendString(pSDNPSession, "</currentValue>\n", curLen, bGetLen);

  if(pName != TMWDEFS_NULL)
  {
    tmwtarg_snprintf(buf, sizeof(buf), "</%s>\n", pName);;
    pSDNPSession->xml2CurIndent--;
    _appendString(pSDNPSession, buf, curLen, bGetLen);
  }
}

/* _currentValueCustom */
static void _currentValueCustom(SDNPSESN *pSDNPSession, TMWTYPES_CHAR *pName, TMWTYPES_CHAR *pValue, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  TMWTYPES_CHAR buf[300];

  pSDNPSession->xml2CurIndent++;
  _appendString(pSDNPSession, "<currentValue>\n", curLen, bGetLen);

  tmwtarg_snprintf(buf, sizeof(buf), "<%s>\n", pName);;
  pSDNPSession->xml2CurIndent++;
  _appendString(pSDNPSession, buf, curLen, bGetLen);

  tmwtarg_snprintf(buf, sizeof(buf), "<explanation>%s</explanation>\n", pValue);
  pSDNPSession->xml2CurIndent++;
  _appendString(pSDNPSession, buf, curLen, bGetLen);

  tmwtarg_snprintf(buf, sizeof(buf), "</%s>\n", pName);;
  pSDNPSession->xml2CurIndent--;
  _appendString(pSDNPSession, buf, curLen, bGetLen);

  pSDNPSession->xml2CurIndent--;
  _appendString(pSDNPSession, "</currentValue>\n", curLen, bGetLen);
  pSDNPSession->xml2CurIndent--;
}

/* _currentValueConfigCustom, both notConfigurableCustomType and configurableCustomeType */
static void _currentValueConfigCustom(SDNPSESN *pSDNPSession, TMWTYPES_CHAR *pName, TMWTYPES_CHAR *pValue, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  TMWTYPES_CHAR buf[300];

  tmwtarg_snprintf(buf, sizeof(buf), "<%s>\n", pName);;
  pSDNPSession->xml2CurIndent++;
  _appendString(pSDNPSession, buf, curLen, bGetLen);

  pSDNPSession->xml2CurIndent++;
  _appendString(pSDNPSession, "<currentValue>\n", curLen, bGetLen);

  pSDNPSession->xml2CurIndent++;
  _appendString(pSDNPSession, "<other>\n", curLen, bGetLen);

  tmwtarg_snprintf(buf, sizeof(buf), "<description>%s</description>\n", pValue);
  pSDNPSession->xml2CurIndent++;
  _appendString(pSDNPSession, buf, curLen, bGetLen);

  pSDNPSession->xml2CurIndent--;
  _appendString(pSDNPSession, "</other>\n", curLen, bGetLen);
  pSDNPSession->xml2CurIndent--;

  pSDNPSession->xml2CurIndent--;
  _appendString(pSDNPSession, "</currentValue>\n", curLen, bGetLen);
  pSDNPSession->xml2CurIndent--;

  tmwtarg_snprintf(buf, sizeof(buf), "</%s>\n", pName);;
  pSDNPSession->xml2CurIndent--;
  _appendString(pSDNPSession, buf, curLen, bGetLen);

}

/* function: _boolString */
static void _boolString(SDNPSESN *pSDNPSession, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen, TMWTYPES_CHAR *pName, TMWTYPES_BOOL value)
{
  TMWTYPES_CHAR buf[512];
  tmwtarg_snprintf(buf, sizeof(buf), "<%s>%s</%s>\n", pName, value ? "true" : "false", pName);
   _appendString(pSDNPSession, buf, curLen, bGetLen);
}

/* function: _assertedString */
static void _assertedString(SDNPSESN *pSDNPSession, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen, TMWTYPES_CHAR *pName, SDNPDATA_XML_ASSERTEDTYPE value)
{
  TMWTYPES_CHAR buf[256];
  if(value == SDNPDATA_XML_ASSERTED_ASSERTED)
    tmwtarg_snprintf(buf, sizeof(buf), "<%s><%s /></%s>\n", pName, "asserted", pName);
  else if(value == SDNPDATA_XML_ASSERTED_DEASSERTED)
    tmwtarg_snprintf(buf, sizeof(buf), "<%s><%s /></%s>\n", pName, "deasserted", pName);
  else
    return;

  _appendString(pSDNPSession, buf, curLen, bGetLen);
}

/* function: _emptyElementString */
static void _emptyElementString(SDNPSESN *pSDNPSession, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen, TMWTYPES_CHAR *pName, TMWTYPES_BOOL value)
{
  TMWTYPES_CHAR buf[256];
  if(value)
  {
    tmwtarg_snprintf(buf, sizeof(buf), "<%s />\n", pName);
    _appendString(pSDNPSession, buf, curLen, bGetLen);
  }
}

/* function: _classMaskString */
static TMWTYPES_CHAR * _classMaskString(TMWDEFS_CLASS_MASK classMask)
{
 if((classMask & TMWDEFS_CLASS_MASK_ONE) != 0)
   return "one";
 if((classMask & TMWDEFS_CLASS_MASK_TWO) != 0)
      return "two";
 if((classMask & TMWDEFS_CLASS_MASK_THREE) != 0)
      return "three";
  return "none";
}

/* function: _linkConfirmModeString */
static TMWTYPES_CHAR * _linkConfirmModeString(TMWDEFS_LINKCNFM mode)
{
  if(mode == TMWDEFS_LINKCNFM_NEVER)
    return "<never/>";
  else if(mode == TMWDEFS_LINKCNFM_SOMETIMES)
    return "<sometimes />";

  return "<always />";
}

/* function: _class0ResponseString */
static TMWTYPES_CHAR * _class0ResponseString(TMWTYPES_BOOL mode)
{
  if(mode == TMWDEFS_FALSE)
    return "never";
  else
    return "always";
}

#if !SDNPDATA_SUPPORT_CLASS0_POINT
/* function: _class0ResponseValue */
static TMWTYPES_CHAR * _class0ResponseValue(TMWTYPES_BOOL mode)
{
  if(mode == TMWDEFS_FALSE)
    return "<never />";
  else
    return "<always />";
}
#endif

/* function: _yesNoString */
static TMWTYPES_CHAR * _yesNoString(TMWTYPES_BOOL value)
{
  if(value)
    return "<yes />";

  return "<no />";
}

/* function: _enabledString */
static TMWTYPES_CHAR * _enabledString(TMWTYPES_BOOL value)
{
  if(value)
    return "<enabled />";

  return "<disabled />";
}

/* networkConfig typeOfEndPoint */
static TMWTYPES_CHAR * _typeOfEndPointString(SDNPDATA_XML_ENDPOINTTYPE type )
{
  switch (type)
  {
    case SDNPDATA_XML_TCPINITIATING:
      return "<tcpInitiating />";

    case SDNPDATA_XML_TCPLISTENING:
      return "<tcpListening />";

    case SDNPDATA_XML_TCPDUAL:
      return "<tcpDual />";

    case SDNPDATA_XML_UDPDATAGRAM:
      return "<udpDatagram />";

    default:
      break;

  }
  return "";
}

/* networkConfig tcpConnectionEstablishment */
static TMWTYPES_CHAR * _tcpConnectionEstabString(SDNPDATA_XML_TCPCONNECTTYPE type )
{
  switch (type)
  {
    case SDNPDATA_XML_TCPALL:
      return "<allowsAll />";

    case SDNPDATA_XML_TCPBASEDONIP:
      return "<basedOnIPAddress />";

    case SDNPDATA_XML_TCPBASEDONLIST:
      return "<basedOnListOfIPAddresses />";

    case SDNPDATA_XML_TCPBASEDONWILD:
      return "<basedOnWildcardIPAddress />";

    case SDNPDATA_XML_TCPBASEDONWILDLIST:
      return "<basedOnListOfWildcardIPAddresses />";

    default:
      break;
  }
  return "";
}

/* networkConfig timeSynchronization */
static TMWTYPES_CHAR * _timeSynchronizationString(SDNPDATA_XML_TIMESYNCTYPE type )
{
  switch (type)
  {
    case SDNPDATA_XML_TIMESYNCNOTSUP:
      return "<notSupported />";

    case SDNPDATA_XML_TIMESYNCLAN:
      return "<dnpLANProcedure />";

    case SDNPDATA_XML_TIMESYNCWRITE:
      return "<dnpWriteTimeProcedure />";

    default:
      break;
  }
  return "";
}

/* securityConfig TLSCipherSuites */
static TMWTYPES_CHAR * _tlsCipherString(SDNPDATA_XML_TLSCIPHERTYPE type)
{
  switch (type)
  {
    case SDNPDATA_XML_TLSNA:
      return "<notApplicable />";

    case SDNPDATA_XML_TLSRSARAES128:
      return "<TLSRSAEncryptedAES128 />";

    case SDNPDATA_XML_TLSRSARC4:
      return "<TLSRSAEncryptedRC4 />";

    case SDNPDATA_XML_TLSRSADES:
      return "<TLSRSAEncrypted3DES />";

    case SDNPDATA_XML_TLSCHDSSDES:
      return "<TLSDHSignedDSSEncrypted3DES />";

    case SDNPDATA_XML_TLSDHRSADES:
      return "<TLSDHSignedRSAEncrypted3DES />";

    case SDNPDATA_XML_TLSDHEDSSDES:
      return "<TLSDHESignedDSSEncrypted3DES />";

    case SDNPDATA_XML_TLSDHERSADES:
      return "<TLSDHESignedRSAEncrypted3DES />";

    case SDNPDATA_XML_TLSDSSAES128:
      return "<TLSDHSignedDSSEncryptedAES128 />";

    case SDNPDATA_XML_TLSDSSAES256:
      return "<TLSDHSignedDSSEncryptedAES256 />";

    case SDNPDATA_XML_TLSAES128:
      return "<TLSDHEncryptedAES128 />";

    case SDNPDATA_XML_TLSAES256:
      return "<TLSDHEncryptedAES256 />";

    default:
      break;
  }
  return "";
}

/* outstationConfig eventBufferOverflowBehavior */
static TMWTYPES_CHAR * _eventOverflowBehaviorString(SDNPDATA_XML_EVENTBUFOVTYPE type )
{
  switch (type)
  {
    case SDNPDATA_XML_EVENT_DISCOLD:
      return "<discardOldest />";

    case SDNPDATA_XML_EVENT_DISCNEW:
      return "<discardNewest />";

    default:
      break;
  }
  return "";
}

#if SDNPDATA_SUPPORT_OBJ22 || SDNPDATA_SUPPORT_OBJ32
/* _anlgReportingModeString */
static TMWTYPES_CHAR * _anlgReportingModeString(TMWDEFS_EVENT_MODE reportingMode)
{
  switch(reportingMode)
  {
  case TMWDEFS_EVENT_MODE_PER_POINT:
      return "<basedOnPointIndex />";

    case TMWDEFS_EVENT_MODE_SOE:
      return "<allEvents />";

    case TMWDEFS_EVENT_MODE_MOST_RECENT:
      return "<mostRecentEventTimeValue />";

    case TMWDEFS_EVENT_MODE_CURRENT:
      return "<mostRecentResponseTimeValue />";

    default:
      break;
  }
  return "<allEvents />";
}
#endif

/* _reportingModeString */
static TMWTYPES_CHAR * _reportingModeString(TMWDEFS_EVENT_MODE reportingMode)
{
  switch(reportingMode)
  {
    case TMWDEFS_EVENT_MODE_PER_POINT:
      return "<basedOnPointIndex />";  /* not suppored by schema */

    case TMWDEFS_EVENT_MODE_SOE:
      return "<allEvents />";

    case TMWDEFS_EVENT_MODE_MOST_RECENT:
      return "<mostRecent />";

    default:
      break;
  }
  return "<allEvents />";
}

#if SDNPDATA_SUPPORT_OBJ20
/* _cntrRolloverString */
static TMWTYPES_CHAR * _cntrRolloverString(SDNPDATA_XML_CNTRROLLOVERTYPE value)
{
  switch(value)
  {
    case SDNPDATA_XML_CNTRROLLOVER16:
      return "<sixteenBits />";

    case SDNPDATA_XML_CNTRROLLOVER32:
      return "<thirtyTwoBits />";

    default:
      break;
  }
  return "<basedOnPointIndex />";
}

/* _cntrFrozenString */
static TMWTYPES_CHAR * _cntrFrozenString(SDNPDATA_XML_CNTRFROZENTYPE value)
{
  switch(value)
  {
    case SDNPDATA_XML_CNTRFROZENLOCALWOTIME:
      return "<localFreezeWithoutTimeOfDay />";

    case SDNPDATA_XML_CNTRFROZENLOCALTIME:
      return "<localFreezeRequiredTimeOfDay />";

    default:
      break;
  }
  return "<masterRequest />";
}
#endif

/* _deadbandAssignString */
static TMWTYPES_CHAR * _deadbandAssignString(SDNPDATA_XML_DDBNDASSIGNTYPE value)
{
  switch(value)
  {
    case SDNPDATA_XML_DDBNDASSIGNFIXED:
      return "<fixed />";

    case SDNPDATA_XML_DDBNDASSIGNDNP3:
      return "<configurableViaDNP3 />";

    case SDNPDATA_XML_DDBNDASSIGNOTHERMEANS:
      return "<configurableViaOtherMeans />";

    case SDNPDATA_XML_DDBNDASSIGNPERPOINT:
      return "<basedOnPointIndex />";

    default:
      break;
  }
  return "  ";
}

/* _deadbandAlgorithmString */
static TMWTYPES_CHAR * _deadbandAlgorithmString(SDNPDATA_XML_DDBNDALGOTYPE value)
{
  switch(value)
  {
    case SDNPDATA_XML_DDBNDALGOSIMPLE:
      return "<simple />";

    case SDNPDATA_XML_DDBNDALGOINTEG:
      return "<integrating />";

    case SDNPDATA_XML_DDBNDALGOPERPOINT:
      return "<basedOnPointIndex />";

    default:
      break;
  }
  return "  ";
}

#if SDNPDATA_SUPPORT_XML2_DNPDATA
#if SDNPDATA_SUPPORT_OBJ1 || SDNPDATA_SUPPORT_OBJ10
/* _binStateString */
static TMWTYPES_CHAR * _binStateString(TMWTYPES_UCHAR flags)
{
  if(flags & DNPDEFS_DBAS_FLAG_BINARY_ON)
    return "true";
  else
    return "false";
}
#endif

#if SDNPDATA_SUPPORT_OBJ10
/* _toControlStatus */
static TMWTYPES_CHAR * _toControlStatus(TMWTYPES_UCHAR status)
{
  switch(status)
  {
    case DNPDEFS_CROB_ST_SUCCESS:
      return "SUCCESS";

    case DNPDEFS_CROB_ST_TIMEOUT:
      return "TIMEOUT";

    case DNPDEFS_CROB_ST_NO_SELECT:
      return "FORMAT_ERROR";

    case DNPDEFS_CROB_ST_FORMAT_ERROR:
      return "SUCCESS";

    case DNPDEFS_CROB_ST_NOT_SUPPORTED:
      return "NOT_SUPPORTED";

    case DNPDEFS_CROB_ST_ALREADY_ACTIVE:
      return "ALREADY_ACTIVE";

    case DNPDEFS_CROB_ST_HARDWARE_ERROR:
      return "HARDWARE_ERROR";

    case DNPDEFS_CROB_ST_LOCAL:
      return "LOCAL";

    case DNPDEFS_CROB_ST_TOO_MANY_OPS:
      return "TOO_MANY_OBJS";

    case DNPDEFS_CROB_ST_NOT_AUTHORIZED:
      return "NOT_AUTHORIZED";

    case DNPDEFS_CROB_ST_AUTO_INHIBIT:
      return "AUTOMATION_INHIBIT";

    case DNPDEFS_CROB_ST_PROC_LIMITED:
      return "PROCESSING_LIMITED";

    case DNPDEFS_CROB_ST_OUT_OF_RANGE:
      return "OUT_OF_RANGE";

    /* These new status codes from TB2014-002 were not added to schema */

   /* case DNPDEFS_CROB_ST_DOWNSTRM_LOCAL:
      return "DOWNSTREAM_LOCAL";

    case DNPDEFS_CROB_ST_ALR_COMPLETE:
      return "ALREADY_COMPLETE";

    case DNPDEFS_CROB_ST_BLOCKED:
      return "BLOCKED";

    case DNPDEFS_CROB_ST_CANCELLED:
      return "CANCELLED";

    case DNPDEFS_CROB_ST_BLOCKED_OM:
      return "BLOCKED_OTHER_MASTER";

    case DNPDEFS_CROB_ST_DOWNSTRM_FAIL:
      return "DOWNSTREAM_FAIL";

    case DNPDEFS_CROB_ST_UNDEFINED:
      return "UNDEFINED";*/

    default:
      break;
  }
  return " ";
}
#endif
#endif

/* _class0Response */
static TMWTYPES_BOOL _class0Response(SDNPSESN *pSDNPSession, DNPDEFS_OBJ_GROUP_ID objectGroup)
{
  int i;
  for(i=0; i<SDNPCNFG_MAX_NUMBER_STATIC_GROUPS; i++)
  {
    if(pSDNPSession->staticGroups[i] == objectGroup)
    {
      return TMWDEFS_TRUE;
    }
  }
  return TMWDEFS_FALSE;
}

/* _variationGroupString */
static TMWTYPES_CHAR * _variationGroupString(TMWTYPES_UCHAR value)
{
  switch(value)
  {
    case 0:
      return "<basedOnPointIndex/>";

    case 1:
      return "<one/>";

    case 2:
      return "<two/>";

    case 3:
      return "<three/>";

    case 4:
      return "<four/>";

    case 5:
      return "<five/>";

    case 6:
      return "<six/>";

    case 7:
      return "<seven/>";

    case 8:
      return "<eight/>";

    case 9:
      return "<nine/>";

    case 10:
      return "<ten/>";

    default:
      return " ";
  }
}

#if SDNPDATA_SUPPORT_STATIC_VAR_POINT || SDNPDATA_SUPPORT_EVENT_VAR_POINT
/* _variationString */
static TMWTYPES_CHAR * _variationString(TMWTYPES_UCHAR value)
{
  switch(value)
  {
    case 1:
      return "<one/>";

    case 2:
      return "<two/>";

    case 3:
      return "<three/>";

    case 4:
      return "<four/>";

    case 5:
      return "<five/>";

    case 6:
      return "<six/>";

    case 7:
      return "<seven/>";

    case 8:
      return "<eight/>";

    case 9:
      return "<nine/>";

    case 10:
      return "<ten/>";

    default:
      return " ";
  }
}
#endif

static void  _pointListDefinition(SDNPSESN *pSDNPSession, DNPDEFS_OBJ_GROUP_ID objectGroup, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  TMWTYPES_CHAR *pName;
  TMWTYPES_CHAR buf[100];

  /* In Jan2010 schema there are no current values for point list definition, only capabilities */
  /* Still true in April2016 */

  pName = "pointListDefinition";

  switch (objectGroup)
  {
  case DNPDEFS_OBJ_70_FILE_IDENTIFIER:
    pName = "fileListDefinition";
    break;
  case DNPDEFS_OBJ_85_DATASET_PROTO:
    pName = "prototypeDefinition";
    break;
  case DNPDEFS_OBJ_86_DATASET_DESCR:
    pName = "descriptorDefinition";
    break;

    default:
      break;
  }

  tmwtarg_snprintf(buf, sizeof(buf), "<%s />\n", pName);;
  _appendString(pSDNPSession, buf, curLen, bGetLen);
}

/* outstationPerformance _timingPerformanceType */
static void _timingPerformanceType(SDNPSESN *pSDNPSession, TMWTYPES_CHAR *pName, SDNPDATA_TIMINGPERFORMANCETYPE *pValue, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  if(pValue->type == SDNPDATA_XML_TIMINGPERFVALUE)
  {
    _currentValueDecimalString(pSDNPSession, pName, pValue->value.value, curLen, bGetLen);
  }

  else if(pValue->type == SDNPDATA_XML_TIMINGPERFOTHER)
  {
    _currentValueConfigCustom(pSDNPSession, pName,  pValue->value.other, curLen, bGetLen);
  }
}

/*  deviceConfig xmlFile */
static void _xmlFileString(SDNPSESN *pSDNPSession, SDNPDATA_XMLFILETYPE *pValue, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  /* note
     readAccess
     writeAccess
     filename
     description
   */

  if(pValue->filename[0] != 0)
  {
    _appendString(pSDNPSession, "<xmlFile>\n", curLen, bGetLen);

    pSDNPSession->xml2CurIndent++;
     _valueString(pSDNPSession, "note", pValue->note, curLen, bGetLen);
    _boolString(pSDNPSession, curLen, bGetLen, "readAccess", pValue->readAccess);
    _boolString(pSDNPSession, curLen, bGetLen, "writeAccess", pValue->writeAccess);
     _valueString(pSDNPSession, "filename", pValue->filename, curLen, bGetLen);
     _valueString(pSDNPSession, "description", pValue->description, curLen, bGetLen);

    pSDNPSession->xml2CurIndent--;
    _appendString(pSDNPSession, "</xmlFile>\n", curLen, bGetLen);
  }
}

/* function: _buildDocHeader */
static void TMWDEFS_LOCAL _buildDocHeader(SDNPSESN *pSDNPSession, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  _appendString(pSDNPSession, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n", curLen, bGetLen);
  _appendString(pSDNPSession, "<?xml-stylesheet type=\"text/xsl\" href=\"DNP3DeviceProfileApril2016.xslt\" media=\"screen\"?>\n", curLen, bGetLen);
  _appendString(pSDNPSession, "<DNP3DeviceProfileDocument schemaVersion=\"2.11.00\" xmlns=\"http://www.dnp3.org/DNP3/DeviceProfile/April2016\"\n", curLen, bGetLen);
  _appendString(pSDNPSession, "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n", curLen, bGetLen);
  _appendString(pSDNPSession, "xsi:schemaLocation=\"http://www.dnp3.org/DNP3/DeviceProfile/April2016 DNP3DeviceProfileApril2016.xsd\">\n", curLen, bGetLen);
  pSDNPSession->xml2CurIndent++;

  /* Added documentType in schema 2.11 */
  _appendString(pSDNPSession, "<documentHeader>\n", curLen, bGetLen);
  pSDNPSession->xml2CurIndent++;
  _appendString(pSDNPSession, "<documentDescription>Generated by Outstation</documentDescription>\n", curLen, bGetLen);
  _appendString(pSDNPSession, "<documentType>Configuration</documentType>\n", curLen, bGetLen);
  pSDNPSession->xml2CurIndent--;
  _appendString(pSDNPSession, "</documentHeader>\n", curLen, bGetLen);

  _appendString(pSDNPSession, "<referenceDevice>\n", curLen, bGetLen);
}

/* function: _buildDocTail */
static void TMWDEFS_LOCAL _buildDocTail(SDNPSESN *pSDNPSession, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  pSDNPSession->xml2CurIndent--;
  _appendString(pSDNPSession, "</referenceDevice>\n", curLen, bGetLen);
  pSDNPSession->xml2CurIndent--;
  _appendString(pSDNPSession, "</DNP3DeviceProfileDocument>\n", curLen, bGetLen);
}

/* function: _buildConfigurationHeader */
static void TMWDEFS_LOCAL _buildConfigurationHeader(SDNPSESN *pSDNPSession, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  pSDNPSession->xml2CurIndent++;
  _appendString(pSDNPSession, "<configuration>\n", curLen, bGetLen);
}

/* function: _buildConfigurationTail */
static void TMWDEFS_LOCAL _buildConfigurationTail(SDNPSESN *pSDNPSession, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  pSDNPSession->xml2CurIndent--;
  _appendString(pSDNPSession, "</configuration>\n", curLen, bGetLen);
}

/* function: _buildDatabaseHeader */
static void TMWDEFS_LOCAL _buildDatabaseHeader(SDNPSESN *pSDNPSession, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  _appendString(pSDNPSession, "<database>\n", curLen, bGetLen);
}

/* function: _buildDatabaseTail */
static void TMWDEFS_LOCAL _buildDatabaseTail(SDNPSESN *pSDNPSession, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  _appendString(pSDNPSession, "</database>\n", curLen, bGetLen);
}

/* function: _buildDataPointsListHeader */
static void TMWDEFS_LOCAL _buildDataPointsListHeader(SDNPSESN *pSDNPSession, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  _appendString(pSDNPSession, "<dataPointsList>\n", curLen, bGetLen);
}

/* function: _buildDataPointsListTail */
static void TMWDEFS_LOCAL _buildDataPointsListTail(SDNPSESN *pSDNPSession, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  _appendString(pSDNPSession, "</dataPointsList>\n", curLen, bGetLen);
}

/* function: _putInputPointType */
static void _putInputPointType(SDNPSESN *pSDNPSession, TMWDEFS_CLASS_MASK classMask, TMWTYPES_BOOL class0, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  _valueString(pSDNPSession, "changeEventClass", _classMaskString(classMask), curLen, bGetLen);
  _valueString(pSDNPSession, "includedInClass0Response", _class0ResponseString(class0), curLen, bGetLen);
}

/* function: _putOutputPointType */
static void _putOutputPointType(SDNPSESN *pSDNPSession, TMWDEFS_CLASS_MASK eventClassMask, TMWDEFS_CLASS_MASK cmdClassMask, TMWTYPES_BOOL class0, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  _valueString(pSDNPSession, "changeEventClass", _classMaskString(eventClassMask), curLen, bGetLen);
  _valueString(pSDNPSession, "commandEventClass", _classMaskString(cmdClassMask), curLen, bGetLen);
  _valueString(pSDNPSession, "includedInClass0Response", _class0ResponseString(class0), curLen, bGetLen);
 }

#if SDNPDATA_SUPPORT_OBJ1
/* function: _buildBinInPoint */
static void TMWDEFS_LOCAL _buildBinInPoint(SDNPSESN *pSDNPSession, void *pDbHandle, TMWTYPES_USHORT pointNum, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  void *pPoint;
  SDNPDATA_XML_BININPOINTCONFIG *pConfig;
  SDNPDATA_XML_BININPOINTCONFIG config;
  TMWTYPES_CHAR buf[128];

  pPoint = sdnpdata_binInGetPoint(pDbHandle, pointNum);
  if(pPoint != TMWDEFS_NULL)
  {
    TMWTYPES_CHAR *pDesc;
    TMWDEFS_CLASS_MASK classMask;
    TMWTYPES_BOOL pointInClass0;
    TMWTYPES_UCHAR flags;

    sdnpdata_binInRead(pPoint, &flags);
    pDesc = sdnpdata_binInGetDescription(pPoint);
    classMask = sdnpdata_binInEventClass(pPoint);

    pSDNPSession->xml2CurIndent++;
    _appendString(pSDNPSession, "<binaryInput>\n", curLen, bGetLen);

    tmwtarg_snprintf(buf, sizeof(buf), "<index>%d</index>\n", pointNum);
    pSDNPSession->xml2CurIndent++;
    _appendString(pSDNPSession, buf, curLen, bGetLen);

    memset(&config, 0, sizeof(config));
    if(sdnpdata_XmlGetBinInPointConfig(pPoint, &config))
    {
      pConfig = &config;

      /* name */
      _valueString(pSDNPSession, "name", pConfig->name, curLen, bGetLen);
    }
    else
      pConfig = TMWDEFS_NULL;

    /* description */
    _valueString(pSDNPSession, "description", pDesc, curLen, bGetLen);

    /* inputPointType contains changeEventClass and includedInClass0Response */
#if SDNPDATA_SUPPORT_CLASS0_POINT
    pointInClass0 = TMWDEFS_FALSE;
    if((_class0Response(pSDNPSession, DNPDEFS_OBJ_1_BIN_INPUTS)) &&  sdnpdata_binInIsClass0(pPoint))
    {
      pointInClass0 = TMWDEFS_TRUE;
    }
    _putInputPointType(pSDNPSession, classMask, pointInClass0, curLen, bGetLen);
#else
    _putInputPointType(pSDNPSession, classMask, _class0Response(pSDNPSession, DNPDEFS_OBJ_1_BIN_INPUTS), curLen, bGetLen);  
#endif

#if SDNPDATA_SUPPORT_STATIC_VAR_POINT
    if(pSDNPSession->obj01DefaultVariation == 0) 
    {
      _valueString(pSDNPSession, "defaultStaticVariation", _variationString(sdnpdata_binInDefVariation(pPoint)), curLen, bGetLen);
    }
    else
    {
      _valueString(pSDNPSession, "defaultStaticVariation",  _variationGroupString(pSDNPSession->obj01DefaultVariation), curLen, bGetLen);
    }
#endif

#if SDNPDATA_SUPPORT_OBJ2
#if SDNPDATA_SUPPORT_EVENT_VAR_POINT
    if(pSDNPSession->obj02DefaultVariation == 0)
    {
      _valueString(pSDNPSession, "defaultEventVariation", _variationString(sdnpdata_binInEventDefVariation(pPoint, TMWDEFS_CLASS_MASK_ALL)), curLen, bGetLen);
    }
    else
    {
      _valueString(pSDNPSession, "defaultEventVariation",  _variationGroupString(pSDNPSession->obj02DefaultVariation), curLen, bGetLen);
    }
#endif
#endif

    if(pConfig != TMWDEFS_NULL)
    {
      _valueString(pSDNPSession, "nameState0", pConfig->nameState0, curLen, bGetLen);
      _valueString(pSDNPSession, "nameState1", pConfig->nameState1, curLen, bGetLen);
    }

#if SDNPDATA_SUPPORT_XML2_DNPDATA
    if(pSDNPSession->xml2WriteDnpData)
    {
      _appendString(pSDNPSession, "<dnpData>\n", curLen, bGetLen);

       pSDNPSession->xml2CurIndent++;
      _valueString(pSDNPSession, "state", _binStateString(flags), curLen, bGetLen);

      tmwtarg_snprintf(buf, sizeof(buf), "<quality>%d</quality>\n", flags&0x7f);
      _appendString(pSDNPSession, buf, curLen, bGetLen);

      /* NotSupported timestamp */

      pSDNPSession->xml2CurIndent--;
      _appendString(pSDNPSession, "</dnpData>\n", curLen, bGetLen);
    }
#endif

    pSDNPSession->xml2CurIndent--;
    _appendString(pSDNPSession, "</binaryInput>\n", curLen, bGetLen);
    pSDNPSession->xml2CurIndent--;
  }
}
#endif

#if SDNPDATA_SUPPORT_OBJ3
/* function: _buildDblBitInPoint */
static void TMWDEFS_LOCAL _buildDblBitInPoint(SDNPSESN *pSDNPSession, void *pDbHandle, TMWTYPES_USHORT pointNum, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  void *pPoint;
  SDNPDATA_XML_DBLINPOINTCONFIG *pConfig;
  SDNPDATA_XML_DBLINPOINTCONFIG config;
  TMWTYPES_CHAR buf[128];

  pPoint = sdnpdata_dblInGetPoint(pDbHandle, pointNum);
  if(pPoint != TMWDEFS_NULL)
  {
    TMWTYPES_CHAR *pDesc;
    TMWDEFS_CLASS_MASK classMask;
    TMWTYPES_BOOL pointInClass0;
    TMWTYPES_UCHAR flags;

    sdnpdata_dblInRead(pPoint, &flags);
    pDesc = sdnpdata_dblInGetDescription(pPoint);
#if SDNPDATA_SUPPORT_OBJ4
    classMask = sdnpdata_dblInEventClass(pPoint);
#else
    classMask = TMWDEFS_CLASS_MASK_NONE;
#endif

    pSDNPSession->xml2CurIndent++;
    _appendString(pSDNPSession, "<doubleBitInput>\n", curLen, bGetLen);

    pSDNPSession->xml2CurIndent++;
    tmwtarg_snprintf(buf, sizeof(buf), "<index>%d</index>\n", pointNum);
    _appendString(pSDNPSession, buf, curLen, bGetLen);

    memset(&config, 0, sizeof(config));
    if(sdnpdata_XmlGetDblInPointConfig(pPoint, &config))
    {
      pConfig = &config;

      /* name */
      _valueString(pSDNPSession, "name", pConfig->name, curLen, bGetLen);
    }
    else
      pConfig = TMWDEFS_NULL;

    /* description */
    _valueString(pSDNPSession, "description", pDesc, curLen, bGetLen);

    /* inputPointType contains changeEventClass and includedInClass0Response */
#if SDNPDATA_SUPPORT_CLASS0_POINT
    pointInClass0 = TMWDEFS_FALSE;
    if((_class0Response(pSDNPSession, DNPDEFS_OBJ_3_DBL_INPUTS)) && sdnpdata_dblInIsClass0(pPoint))
    {
      pointInClass0 = TMWDEFS_TRUE;
    }
    _putInputPointType(pSDNPSession, classMask, pointInClass0, curLen, bGetLen);
#else
    _putInputPointType(pSDNPSession, classMask, _class0Response(pSDNPSession, DNPDEFS_OBJ_3_DBL_INPUTS), curLen, bGetLen);
#endif

#if SDNPDATA_SUPPORT_STATIC_VAR_POINT
    if(pSDNPSession->obj03DefaultVariation == 0)
    {
      _valueString(pSDNPSession, "defaultStaticVariation", _variationString(sdnpdata_dblInDefVariation(pPoint)), curLen, bGetLen);
    }
    else
    {
      _valueString(pSDNPSession, "defaultStaticVariation", _variationGroupString(pSDNPSession->obj03DefaultVariation), curLen, bGetLen);
    }
#endif
#if SDNPDATA_SUPPORT_OBJ4 && SDNPDATA_SUPPORT_EVENT_VAR_POINT
    if(pSDNPSession->obj04DefaultVariation == 0) 
    { 
      _valueString(pSDNPSession, "defaultEventVariation", _variationString(sdnpdata_dblInEventDefVariation(pPoint, TMWDEFS_CLASS_MASK_ALL)), curLen, bGetLen);
    }
    else
    {
      _valueString(pSDNPSession, "defaultEventVariation", _variationGroupString(pSDNPSession->obj04DefaultVariation), curLen, bGetLen);
    }
#endif

    if(pConfig != TMWDEFS_NULL)
    {
      _valueString(pSDNPSession, "nameState0", pConfig->nameState0, curLen, bGetLen);
      _valueString(pSDNPSession, "nameState1", pConfig->nameState1, curLen, bGetLen);
      _valueString(pSDNPSession, "nameState2", pConfig->nameState2, curLen, bGetLen);
      _valueString(pSDNPSession, "nameState3", pConfig->nameState3, curLen, bGetLen);
    }

#if SDNPDATA_SUPPORT_XML2_DNPDATA
    if(pSDNPSession->xml2WriteDnpData)
    {
      _appendString(pSDNPSession, "<dnpData>\n", curLen, bGetLen);

      tmwtarg_snprintf(buf, sizeof(buf), "<state>%d</state>\n", (flags&0xc0)>>6);
      pSDNPSession->xml2CurIndent++;
      _appendString(pSDNPSession, buf, curLen, bGetLen);

      tmwtarg_snprintf(buf, sizeof(buf), "<quality>%d</quality>\n", flags&0x3f);
      _appendString(pSDNPSession, buf, curLen, bGetLen);

      /* NotSupported timestamp */

      pSDNPSession->xml2CurIndent--;
      _appendString(pSDNPSession, "</dnpData>\n", curLen, bGetLen);
    }
#endif

    pSDNPSession->xml2CurIndent--;
    _appendString(pSDNPSession, "</doubleBitInput>\n", curLen, bGetLen);
    pSDNPSession->xml2CurIndent--;
  }
}
#endif

#if SDNPDATA_SUPPORT_OBJ10
/* function: _buildBinOutPoint */
static void TMWDEFS_LOCAL _buildBinOutPoint(SDNPSESN *pSDNPSession, void *pDbHandle, TMWTYPES_USHORT pointNum, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  void *pPoint;
  SDNPDATA_XML_BINOUTPOINTCONFIG *pConfig;
  SDNPDATA_XML_BINOUTPOINTCONFIG config;
  TMWTYPES_CHAR buf[128];

  pPoint = sdnpdata_binOutGetPoint(pDbHandle, pointNum);
  if(pPoint != TMWDEFS_NULL)
  {
    TMWTYPES_CHAR *pDesc;
    TMWDEFS_CLASS_MASK classMask1;
    TMWDEFS_CLASS_MASK classMask2;
    SDNPDATA_CROB_CTRL ctrlMask;
    SDNPDATA_XML_SUP   supportMask;
    TMWTYPES_BOOL pointInClass0;
    TMWTYPES_UCHAR flags;

    sdnpdata_binOutRead(pPoint, &flags);
    pDesc = sdnpdata_binOutGetDescription(pPoint);
    ctrlMask = sdnpdata_binOutGetControlMask(pPoint);
    supportMask = sdnpdata_binOutGetSupCtrl(pPoint);

#if SDNPDATA_SUPPORT_OBJ11
    classMask1 = sdnpdata_binOutEventClass(pPoint);
#else
    classMask1 = TMWDEFS_CLASS_MASK_NONE;
#endif
#if SDNPDATA_SUPPORT_OBJ13
    classMask2 = sdnpdata_binOutCmdEventClass(pPoint);
#else
    classMask2 = TMWDEFS_CLASS_MASK_NONE;
#endif

    pSDNPSession->xml2CurIndent++;
    _appendString(pSDNPSession, "<binaryOutput>\n", curLen, bGetLen);

    tmwtarg_snprintf(buf, sizeof(buf), "<index>%d</index>\n", pointNum);
    pSDNPSession->xml2CurIndent++;
    _appendString(pSDNPSession, buf, curLen, bGetLen);

    memset(&config, 0, sizeof(config));
    if(sdnpdata_XmlGetBinOutPointConfig(pPoint, &config))
    {
      pConfig = &config;

      /* name */
      _valueString(pSDNPSession, "name", pConfig->name, curLen, bGetLen);
    }
    else
      pConfig = TMWDEFS_NULL;

    /* description */
    _valueString(pSDNPSession, "description", pDesc, curLen, bGetLen);

    /* outputPointType contains changeEventClass, commandEventClass, includedInClass0Response*/
#if SDNPDATA_SUPPORT_CLASS0_POINT
    pointInClass0 = TMWDEFS_FALSE;
    if((_class0Response(pSDNPSession, DNPDEFS_OBJ_10_BIN_OUT_STATUSES)) &&  sdnpdata_binOutIsClass0(pPoint))
    {
      pointInClass0 = TMWDEFS_TRUE;
    }
    _putOutputPointType(pSDNPSession, classMask1, classMask2, pointInClass0, curLen, bGetLen);  
#else
    _putOutputPointType(pSDNPSession, classMask1, classMask2, _class0Response(pSDNPSession, DNPDEFS_OBJ_10_BIN_OUT_STATUSES), curLen, bGetLen);  
#endif

#if SDNPDATA_SUPPORT_STATIC_VAR_POINT
    if(pSDNPSession->obj10DefaultVariation == 0) 
    {
      _valueString(pSDNPSession, "defaultStaticVariation", _variationString(sdnpdata_binOutDefVariation(pPoint)), curLen, bGetLen);
    }
    else
    {
      _valueString(pSDNPSession, "defaultStaticVariation",  _variationGroupString(pSDNPSession->obj10DefaultVariation), curLen, bGetLen);
    }
#endif

#if SDNPDATA_SUPPORT_EVENT_VAR_POINT
#if SDNPDATA_SUPPORT_OBJ11
    if(pSDNPSession->obj11DefaultVariation == 0)
    {
      _valueString(pSDNPSession, "defaultChangeEventVariation",  _variationString(sdnpdata_binOutEventDefVariation(pPoint, TMWDEFS_CLASS_MASK_ALL)), curLen, bGetLen);
    }
    else
    {
      _valueString(pSDNPSession, "defaultChangeEventVariation",  _variationGroupString(pSDNPSession->obj11DefaultVariation), curLen, bGetLen);
    }
#endif

#if SDNPDATA_SUPPORT_OBJ13
    if(pSDNPSession->obj13DefaultVariation == 0)
    {
      _valueString(pSDNPSession, "defaultCommandEventVariation", _variationString(sdnpdata_binOutCmdEventDefVariation(pPoint, TMWDEFS_CLASS_MASK_ALL)), curLen, bGetLen);
    }
    else
    {
      _valueString(pSDNPSession, "defaultCommandEventVariation",  _variationGroupString(pSDNPSession->obj13DefaultVariation), curLen, bGetLen);
    }
#endif
#endif

    if(pConfig != TMWDEFS_NULL)
    {
      tmwtarg_snprintf(buf, sizeof(buf), "<minimumPulseWidth>%g</minimumPulseWidth>\n", pConfig->minimumPulseWidth);
      _appendString(pSDNPSession, buf, curLen, bGetLen);

      tmwtarg_snprintf(buf, sizeof(buf), "<maximumPulseWidth>%g</maximumPulseWidth>\n", pConfig->maximumPulseWidth);
      _appendString(pSDNPSession, buf, curLen, bGetLen);
    }

    _appendString(pSDNPSession, "<supportedControlOperations>\n", curLen, bGetLen);
    pSDNPSession->xml2CurIndent++;

    _emptyElementString(pSDNPSession, curLen, bGetLen, "supportSelectOperate", TMWDEFS_TOBOOL(supportMask, SDNPDATA_XML_SUP_SELOP));
    _emptyElementString(pSDNPSession, curLen, bGetLen, "supportDirectOperate", TMWDEFS_TOBOOL(supportMask, SDNPDATA_XML_SUP_DIROP));
    _emptyElementString(pSDNPSession, curLen, bGetLen, "supportDirectOperateNoAck", TMWDEFS_TOBOOL(supportMask, SDNPDATA_XML_SUP_DIRNOACK));

    _emptyElementString(pSDNPSession, curLen, bGetLen, "supportPulseOn", TMWDEFS_TOBOOL(ctrlMask, SDNPDATA_CROB_CTRL_PULSE_ON));
    _emptyElementString(pSDNPSession, curLen, bGetLen, "supportPulseOff", TMWDEFS_TOBOOL(ctrlMask, SDNPDATA_CROB_CTRL_PULSE_OFF));
    _emptyElementString(pSDNPSession, curLen, bGetLen, "supportLatchOn", TMWDEFS_TOBOOL(ctrlMask, SDNPDATA_CROB_CTRL_LATCH_ON));
    _emptyElementString(pSDNPSession, curLen, bGetLen, "supportLatchOff", TMWDEFS_TOBOOL(ctrlMask, SDNPDATA_CROB_CTRL_LATCH_OFF));
    _emptyElementString(pSDNPSession, curLen, bGetLen, "supportTrip", TMWDEFS_TOBOOL(ctrlMask, SDNPDATA_CROB_CTRL_PAIRED_TRIP));
    _emptyElementString(pSDNPSession, curLen, bGetLen, "supportClose", TMWDEFS_TOBOOL(ctrlMask, SDNPDATA_CROB_CTRL_PAIRED_CLOSE));

    _emptyElementString(pSDNPSession, curLen, bGetLen, "countGreaterThanOne", TMWDEFS_TOBOOL(supportMask, SDNPDATA_XML_SUP_MORE_ONE));
    _emptyElementString(pSDNPSession, curLen, bGetLen, "cancelCurrentOperation", TMWDEFS_TOBOOL(supportMask, SDNPDATA_XML_SUP_SEL_CANCEL));

    pSDNPSession->xml2CurIndent--;
    _appendString(pSDNPSession, "</supportedControlOperations>\n", curLen, bGetLen);

    tmwtarg_snprintf(buf, sizeof(buf), "<maxTimeSelectOperate>%d</maxTimeSelectOperate>\n", pSDNPSession->selectTimeout);
    _appendString(pSDNPSession, buf, curLen, bGetLen);

    if(pConfig != TMWDEFS_NULL)
    {
      _valueString(pSDNPSession, "nameState0", pConfig->nameState0, curLen, bGetLen);
      _valueString(pSDNPSession, "nameState1", pConfig->nameState1, curLen, bGetLen);
    }

#if SDNPDATA_SUPPORT_XML2_DNPDATA
    if(pSDNPSession->xml2WriteDnpData)
    {
      _appendString(pSDNPSession, "<dnpData>\n", curLen, bGetLen);

      pSDNPSession->xml2CurIndent++;
      _valueString(pSDNPSession, "state",_binStateString(flags), curLen, bGetLen);

      tmwtarg_snprintf(buf, sizeof(buf), "<quality>%d</quality>\n", flags&0x3f);
      _appendString(pSDNPSession, buf, curLen, bGetLen);

      /* NotSupported timestamp */

      /* control */
      if (pConfig != TMWDEFS_NULL)
      {
        tmwtarg_snprintf(buf, sizeof(buf), "<control>%d</control>\n", pConfig->control);
        _appendString(pSDNPSession, buf, curLen, bGetLen);

        /* status */
        _valueString(pSDNPSession, "status", _toControlStatus(pConfig->status), curLen, bGetLen);
      }

      pSDNPSession->xml2CurIndent--;
      _appendString(pSDNPSession, "</dnpData>\n", curLen, bGetLen);
    }
#endif

    pSDNPSession->xml2CurIndent--;
    _appendString(pSDNPSession, "</binaryOutput>\n", curLen, bGetLen);
    pSDNPSession->xml2CurIndent--;
  }
}
#endif

#if SDNPDATA_SUPPORT_OBJ20
/* function: _buildCounterPoint */
static void TMWDEFS_LOCAL _buildCounterPoint(SDNPSESN *pSDNPSession, void *pDbHandle, TMWTYPES_USHORT pointNum, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  void *pPoint;
#if SDNPDATA_SUPPORT_OBJ21
  void *pFrznPoint;
#endif
  TMWTYPES_CHAR buf[128];

  pPoint = sdnpdata_binCntrGetPoint(pDbHandle, pointNum);
  if(pPoint != TMWDEFS_NULL)
  {
    SDNPDATA_XML_CNTRPOINTCONFIG *pConfig;
    TMWTYPES_CHAR *pDesc;
    TMWTYPES_BOOL pointInClass0;
    TMWDEFS_CLASS_MASK classMask;
    TMWTYPES_UCHAR flags;
    TMWTYPES_ULONG value;
    SDNPDATA_XML_CNTRPOINTCONFIG config;

    pDesc = sdnpdata_binCntrGetDescription(pPoint);
    sdnpdata_binCntrRead(pPoint, &value, &flags);
#if SDNPDATA_SUPPORT_OBJ22
    classMask = sdnpdata_binCntrEventClass(pPoint);
#else
    classMask = TMWDEFS_CLASS_MASK_NONE;
#endif

    pSDNPSession->xml2CurIndent++;
    _appendString(pSDNPSession, "<counter>\n", curLen, bGetLen);

    tmwtarg_snprintf(buf, sizeof(buf), "<index>%d</index>\n", pointNum);
    pSDNPSession->xml2CurIndent++;
    _appendString(pSDNPSession, buf, curLen, bGetLen);

    memset(&config, 0, sizeof(config));
    if(sdnpdata_XmlGetCntrPointConfig(pPoint, &config))
    {
      pConfig = &config;

      /* name */
      _valueString(pSDNPSession, "name", pConfig->name, curLen, bGetLen);
    }
    else
      pConfig = TMWDEFS_NULL;

    /* description */
    _valueString(pSDNPSession, "description", pDesc, curLen, bGetLen);   

#if SDNPDATA_SUPPORT_STATIC_VAR_POINT
    if(pSDNPSession->obj20DefaultVariation == 0)
    {
      _valueString(pSDNPSession, "defaultCounterStaticVariation", _variationString(sdnpdata_binCntrDefVariation(pPoint)), curLen, bGetLen); 
    }
#endif
#if SDNPDATA_SUPPORT_OBJ22
#if SDNPDATA_SUPPORT_EVENT_VAR_POINT
    if(pSDNPSession->obj22DefaultVariation == 0)
    {
      _valueString(pSDNPSession, "defaultCounterEventVariation", _variationString(sdnpdata_binCntrEventDefVariation(pPoint, TMWDEFS_CLASS_MASK_ALL)), curLen, bGetLen);
    }
#endif
#endif

#if SDNPDATA_SUPPORT_CLASS0_POINT
    pointInClass0 = TMWDEFS_FALSE;
    if((_class0Response(pSDNPSession, DNPDEFS_OBJ_20_RUNNING_CNTRS)) &&  sdnpdata_binCntrIsClass0(pPoint))
    {
      pointInClass0 = TMWDEFS_TRUE;
    }
    _valueString(pSDNPSession, "countersIncludedInClass0", _class0ResponseString(pointInClass0), curLen, bGetLen);
#else
    _valueString(pSDNPSession, "countersIncludedInClass0", _class0ResponseString(_class0Response(pSDNPSession, DNPDEFS_OBJ_20_RUNNING_CNTRS)), curLen, bGetLen);
#endif

    _valueString(pSDNPSession, "counterEventClass", _classMaskString(classMask), curLen, bGetLen);

#if SDNPDATA_SUPPORT_EVENT_MODE_POINT && SDNPDATA_SUPPORT_OBJ22
    /* schema 2.11 added counterEventReportMode */
    if (pSDNPSession->binaryCounterEventMode == TMWDEFS_EVENT_MODE_PER_POINT)
    {
      _valueString(pSDNPSession, "counterEventReportMode", _anlgReportingModeString(sdnpdata_binCntrEventMode(pPoint)), curLen, bGetLen);
    }
#endif

#if SDNPDATA_SUPPORT_OBJ21
    pFrznPoint = sdnpdata_frznCntrGetPoint(pDbHandle, pointNum);
    _boolString(pSDNPSession, curLen, bGetLen, "frozenCounterExists", (pFrznPoint != TMWDEFS_NULL) ? TMWDEFS_TRUE:TMWDEFS_FALSE);

    if(pFrznPoint != TMWDEFS_NULL)
    {
#if SDNPDATA_SUPPORT_STATIC_VAR_POINT
      if(pSDNPSession->obj21DefaultVariation == 0)
      {
        _valueString(pSDNPSession, "defaultFrozenCounterStaticVariation", _variationString(sdnpdata_frznCntrDefVariation(pFrznPoint)), curLen, bGetLen);
      }
      else
      {
        _valueString(pSDNPSession, "defaultFrozenCounterStaticVariation",  _variationGroupString(pSDNPSession->obj21DefaultVariation), curLen, bGetLen);
      }
#endif

#if SDNPDATA_SUPPORT_OBJ23
#if SDNPDATA_SUPPORT_EVENT_VAR_POINT
      if(pSDNPSession->obj23DefaultVariation == 0)
      {
        _valueString(pSDNPSession, "defaultFrozenCounterEventVariation", _variationString(sdnpdata_frznCntrEventDefVariation(pFrznPoint, TMWDEFS_CLASS_MASK_ALL)), curLen, bGetLen);
      }
      else
      {
        _valueString(pSDNPSession, "defaultFrozenCounterEventVariation",  _variationGroupString(pSDNPSession->obj23DefaultVariation), curLen, bGetLen);
      }
#endif
#endif

#if SDNPDATA_SUPPORT_CLASS0_POINT
      pointInClass0 = TMWDEFS_FALSE;
      if((_class0Response(pSDNPSession, DNPDEFS_OBJ_21_FROZEN_CNTRS)) &&  sdnpdata_frznCntrIsClass0(pFrznPoint))
      {
        pointInClass0 = TMWDEFS_TRUE;
      }
      _valueString(pSDNPSession, "frozenCountersIncludedInClass0", _class0ResponseString(pointInClass0), curLen, bGetLen);
#else
      _valueString(pSDNPSession, "frozenCountersIncludedInClass0", _class0ResponseString(_class0Response(pSDNPSession, DNPDEFS_OBJ_21_FROZEN_CNTRS)), curLen, bGetLen);
#endif

#if SDNPDATA_SUPPORT_OBJ23
      _valueString(pSDNPSession, "frozenCounterEventClass", _classMaskString(classMask), curLen, bGetLen);
#if SDNPDATA_SUPPORT_EVENT_MODE_POINT
      /* schema 2.11 added frozenCounterEventReportMode */
      if (pSDNPSession->frozenCounterEventMode == TMWDEFS_EVENT_MODE_PER_POINT)
      {
        _valueString(pSDNPSession, "frozenCounterEventReportMode", _anlgReportingModeString(sdnpdata_frznCntrEventMode(pFrznPoint)), curLen, bGetLen);
      }
#endif
#endif

    }
#endif

    if (pConfig != TMWDEFS_NULL)
    {
      tmwtarg_snprintf(buf, sizeof(buf), "<counterRollOver>%u</counterRollOver>\n", pConfig->counterRollOver);
      _appendString(pSDNPSession, buf, curLen, bGetLen);
    }

#if SDNPDATA_SUPPORT_OBJ23
    /* schema 2.11 added reportCounterValueChangeEvents */
    if (sdnpdata_frznCntrEventClass(pFrznPoint))
    {
      _valueString(pSDNPSession, "reportCounterValueChangeEvents", "<yes />", curLen, bGetLen);
    }
#endif
#if SDNPDATA_SUPPORT_XML2_DNPDATA
    if(pSDNPSession->xml2WriteDnpData)
    {
#if SDNPDATA_SUPPORT_OBJ21
      TMWDTIME timeOfFreeze;
#endif
      _appendString(pSDNPSession, "<dnpData>\n", curLen, bGetLen);

      tmwtarg_snprintf(buf, sizeof(buf), "<runningCounterValue>%d</runningCounterValue>\n", value);
      pSDNPSession->xml2CurIndent++;
      _appendString(pSDNPSession, buf, curLen, bGetLen);

      tmwtarg_snprintf(buf, sizeof(buf), "<runningCounterQuality>%d</runningCounterQuality>\n", flags);
      _appendString(pSDNPSession, buf, curLen, bGetLen);

      /* NotSupported runningCounterTimestamp */

#if SDNPDATA_SUPPORT_OBJ21
      sdnpdata_frznCntrRead(pFrznPoint, &value, &flags, &timeOfFreeze);

      tmwtarg_snprintf(buf, sizeof(buf), "<frozenCounterValue>%d</frozenCounterValue>\n", value);
      _appendString(pSDNPSession, buf, curLen, bGetLen);

      tmwtarg_snprintf(buf, sizeof(buf), "<frozenCounterQuality>%d</frozenCounterQuality>\n", flags);
      _appendString(pSDNPSession, buf, curLen, bGetLen);

      /* NotSupported frozenCounterTimestamp */
#endif

      pSDNPSession->xml2CurIndent--;
      _appendString(pSDNPSession, "</dnpData>\n", curLen, bGetLen);
    }
#endif

    pSDNPSession->xml2CurIndent--;
    _appendString(pSDNPSession, "</counter>\n", curLen, bGetLen);
    pSDNPSession->xml2CurIndent--;
  }
}
#endif

#if SDNPDATA_SUPPORT_OBJ30
/* function: _buildAnalogInPoint */
static void TMWDEFS_LOCAL _buildAnalogInPoint(SDNPSESN *pSDNPSession, void *pDbHandle, TMWTYPES_USHORT pointNum, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  void *pPoint;
  SDNPDATA_XML_ANLGINPOINTCONFIG *pConfig;
  SDNPDATA_XML_ANLGINPOINTCONFIG config;
  TMWTYPES_CHAR buf[128];

  pPoint = sdnpdata_anlgInGetPoint(pDbHandle, pointNum);
  if(pPoint != TMWDEFS_NULL)
  {
    TMWTYPES_CHAR *pDesc;
    TMWDEFS_CLASS_MASK classMask;
    TMWTYPES_ANALOG_VALUE value;
    TMWTYPES_UCHAR flags;
    TMWTYPES_BOOL pointInClass0;

    pDesc = sdnpdata_anlgInGetDescription(pPoint);
    sdnpdata_anlgInRead(pPoint, &value, &flags);
#if SDNPDATA_SUPPORT_OBJ32
    classMask = sdnpdata_anlgInEventClass(pPoint);
#else
    classMask = TMWDEFS_CLASS_MASK_NONE;
#endif

    pSDNPSession->xml2CurIndent++;
    _appendString(pSDNPSession, "<analogInput>\n", curLen, bGetLen);

    tmwtarg_snprintf(buf, sizeof(buf), "<index>%d</index>\n", pointNum);
    pSDNPSession->xml2CurIndent++;
    _appendString(pSDNPSession, buf, curLen, bGetLen);

    memset(&config, 0, sizeof(config));
    if(sdnpdata_XmlGetAnlgInPointConfig(pPoint, &config))
    {
      pConfig = &config;

      /* name */
      _valueString(pSDNPSession, "name", pConfig->name, curLen, bGetLen);
    }
    else
      pConfig = TMWDEFS_NULL;

    /* description */
    _valueString(pSDNPSession, "description", pDesc, curLen, bGetLen);

    /* inputPointType contains changeEventClass and includedInClass0Response */
#if SDNPDATA_SUPPORT_CLASS0_POINT
    pointInClass0 = TMWDEFS_FALSE;
    if((_class0Response(pSDNPSession, DNPDEFS_OBJ_30_ANA_INPUTS)) &&  sdnpdata_anlgInIsClass0(pPoint))
    {
      pointInClass0 = TMWDEFS_TRUE;
    }
    _putInputPointType(pSDNPSession, classMask, pointInClass0, curLen, bGetLen);  
#else
    _putInputPointType(pSDNPSession, classMask, _class0Response(pSDNPSession, DNPDEFS_OBJ_30_ANA_INPUTS), curLen, bGetLen);  
#endif

#if SDNPDATA_SUPPORT_STATIC_VAR_POINT
    if(pSDNPSession->obj30DefaultVariation == 0)
    {
      _valueString(pSDNPSession, "defaultStaticVariation", _variationString(sdnpdata_anlgInDefVariation(pPoint)), curLen, bGetLen);    
    }
    else
    {
      _valueString(pSDNPSession, "defaultStaticVariation",  _variationGroupString(pSDNPSession->obj30DefaultVariation), curLen, bGetLen);
    }
#endif

#if SDNPDATA_SUPPORT_EVENT_VAR_POINT && SDNPDATA_SUPPORT_OBJ32
    if(pSDNPSession->obj32DefaultVariation == 0)
    {
      _valueString(pSDNPSession, "defaultEventVariation", _variationString(sdnpdata_anlgInEventDefVariation(pPoint, TMWDEFS_CLASS_MASK_ALL)), curLen, bGetLen);
    }
    else
    {
      _valueString(pSDNPSession, "defaultEventVariation",  _variationGroupString(pSDNPSession->obj32DefaultVariation), curLen, bGetLen);
    }

    /* NotSupported analogEventReportingMode analogIncludedInClass0 analogDeadbandAssignments analogDeadbandAlgorithm
     * frozenAnalogExists frozenAnalogDefaultStaticVariation frozenAnalogDefaultEventVariation frozenAnalogEventReportingMode frozenAnalogIncludedInClass0 frozenAnalogEventClass"
    */

#endif
    if(pConfig != TMWDEFS_NULL)
    {
      /* changed to choice of integer or float in schema 2.11.
       * If min and max integer are the same use the float.
       */
      if (pConfig->minIntTransmittedValue != pConfig->maxIntTransmittedValue)
      {
        tmwtarg_snprintf(buf, sizeof(buf), "<minIntegerTransmittedValue>%d</minIntegerTransmittedValue>\n", pConfig->minIntTransmittedValue);
        _appendString(pSDNPSession, buf, curLen, bGetLen);

        tmwtarg_snprintf(buf, sizeof(buf), "<maxIntegerTransmittedValue>%d</maxIntegerTransmittedValue>\n", pConfig->maxIntTransmittedValue);
        _appendString(pSDNPSession, buf, curLen, bGetLen);
      }
      else
      {
        tmwtarg_snprintf(buf, sizeof(buf), "<minFloatTransmittedValue>%g</minFloatTransmittedValue>\n", pConfig->minFloatTransmittedValue);
        _appendString(pSDNPSession, buf, curLen, bGetLen);

        tmwtarg_snprintf(buf, sizeof(buf), "<maxFloatTransmittedValue>%g</maxFloatTransmittedValue>\n", pConfig->maxFloatTransmittedValue);
        _appendString(pSDNPSession, buf, curLen, bGetLen);
      }

      tmwtarg_snprintf(buf, sizeof(buf), "<scaleOffset>%g</scaleOffset>\n", pConfig->scaleOffset);
      _appendString(pSDNPSession, buf, curLen, bGetLen);

      tmwtarg_snprintf(buf, sizeof(buf), "<scaleFactor>%g</scaleFactor>\n", pConfig->scaleFactor);
      _appendString(pSDNPSession, buf, curLen, bGetLen);

      tmwtarg_snprintf(buf, sizeof(buf), "<resolution>%g</resolution>\n", pConfig->resolution);
      _appendString(pSDNPSession, buf, curLen, bGetLen);

      _valueString(pSDNPSession, "units", pConfig->units, curLen, bGetLen);
    }

#if SDNPDATA_SUPPORT_XML2_DNPDATA
    if(pSDNPSession->xml2WriteDnpData)
    {
#if TMWCNFG_SUPPORT_DOUBLE
      TMWTYPES_DOUBLE dvalue;
      dvalue = dnputil_getAnalogValueDouble(&value);
#elif TMWCNFG_SUPPORT_FLOAT
      TMWTYPES_SFLOAT fvalue;
      fvalue = dnputil_getAnalogValueFloat(&value, &flags);
#else
      TMWTYPES_ULONG ulvalue;
      ulvalue = dnputil_getAnalogValueLong(&value, &flags);
#endif
      _appendString(pSDNPSession, "<dnpData>\n", curLen, bGetLen);

#if TMWCNFG_SUPPORT_DOUBLE
      tmwtarg_snprintf(buf, sizeof(buf), "<value>%g</value>\n", dvalue);
#elif TMWCNFG_SUPPORT_FLOAT
      tmwtarg_snprintf(buf, sizeof(buf), "<value>%g</value>\n", fvalue);
#else
      tmwtarg_snprintf(buf, sizeof(buf), "<value>%g</value>\n", ulvalue);
#endif

      pSDNPSession->xml2CurIndent++;
      _appendString(pSDNPSession, buf, curLen, bGetLen);

      tmwtarg_snprintf(buf, sizeof(buf), "<quality>%d</quality>\n", flags&0x3f);
      _appendString(pSDNPSession, buf, curLen, bGetLen);

      /* NotSupported timestamp */
      /* NotSupported deadband frozenAnalogValue frozenAnalogQuality frozenAnalogTimestamp */

      pSDNPSession->xml2CurIndent--;
      _appendString(pSDNPSession, "</dnpData>\n", curLen, bGetLen);
    }
#endif

    pSDNPSession->xml2CurIndent--;
    _appendString(pSDNPSession, "</analogInput>\n", curLen, bGetLen);
    pSDNPSession->xml2CurIndent--;
  }
}
#endif

#if SDNPDATA_SUPPORT_OBJ40
/* function: _buildAnalogOutPoint */
static void TMWDEFS_LOCAL _buildAnalogOutPoint(SDNPSESN *pSDNPSession, void *pDbHandle, TMWTYPES_USHORT pointNum, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  void *pPoint;
  SDNPDATA_XML_ANLGOUTPOINTCONFIG *pConfig;
  SDNPDATA_XML_ANLGOUTPOINTCONFIG config;
  TMWTYPES_CHAR buf[128];

  pPoint = sdnpdata_anlgOutGetPoint(pDbHandle, pointNum);
  if (pPoint != TMWDEFS_NULL)
  {
    TMWTYPES_CHAR *pDesc;
    TMWDEFS_CLASS_MASK classMask1;
    TMWDEFS_CLASS_MASK classMask2;
    TMWTYPES_ANALOG_VALUE value;
    TMWTYPES_BOOL pointInClass0;
    TMWTYPES_UCHAR flags;
    SDNPDATA_XML_SUP supportMask;

    pDesc = sdnpdata_anlgOutGetDescription(pPoint);
    sdnpdata_anlgOutRead(pPoint, &value, &flags);
#if SDNPDATA_SUPPORT_OBJ42
    classMask1 = sdnpdata_anlgOutEventClass(pPoint);
#else
    classMask1 = TMWDEFS_CLASS_MASK_NONE;
#endif
#if SDNPDATA_SUPPORT_OBJ43
    classMask2 = sdnpdata_anlgOutCmdEventClass(pPoint);
#else
    classMask2 = TMWDEFS_CLASS_MASK_NONE;
#endif
    supportMask = sdnpdata_anlgOutGetSupCtrl(pPoint);

    pSDNPSession->xml2CurIndent++;
    _appendString(pSDNPSession, "<analogOutput>\n", curLen, bGetLen);

    tmwtarg_snprintf(buf, sizeof(buf), "<index>%d</index>\n", pointNum);
    pSDNPSession->xml2CurIndent++;
    _appendString(pSDNPSession, buf, curLen, bGetLen);

    memset(&config, 0, sizeof(config));
    if (sdnpdata_XmlGetAnlgOutPointConfig(pPoint, &config))
    {
      pConfig = &config;

      /* name */
      _valueString(pSDNPSession, "name", pConfig->name, curLen, bGetLen);
    }
    else
      pConfig = TMWDEFS_NULL;

    /* description */
    _valueString(pSDNPSession, "description", pDesc, curLen, bGetLen);

    /* outputPointType contains changeEventClass, commandEventClass, includedInClass0Response*/
#if SDNPDATA_SUPPORT_CLASS0_POINT
    pointInClass0 = TMWDEFS_FALSE;
    if ((_class0Response(pSDNPSession, DNPDEFS_OBJ_40_ANA_OUT_STATUSES)) && sdnpdata_anlgOutIsClass0(pPoint))
    {
      pointInClass0 = TMWDEFS_TRUE;
    }
    _putOutputPointType(pSDNPSession, classMask1, classMask2, pointInClass0, curLen, bGetLen);
#else
    _putOutputPointType(pSDNPSession, classMask1, classMask2, _class0Response(pSDNPSession, DNPDEFS_OBJ_40_ANA_OUT_STATUSES), curLen, bGetLen);
#endif

#if SDNPDATA_SUPPORT_STATIC_VAR_POINT
    if (pSDNPSession->obj40DefaultVariation == 0)
    {
      _valueString(pSDNPSession, "defaultStaticVariation", _variationString(sdnpdata_anlgOutDefVariation(pPoint)), curLen, bGetLen);
    }
    else
    {
      _valueString(pSDNPSession, "defaultStaticVariation", _variationGroupString(pSDNPSession->obj40DefaultVariation), curLen, bGetLen);
    }
#endif

#if SDNPDATA_SUPPORT_EVENT_VAR_POINT
#if SDNPDATA_SUPPORT_OBJ42 
    if(pSDNPSession->obj42DefaultVariation == 0)
    {
      _valueString(pSDNPSession, "defaultChangeEventVariation", _variationString(sdnpdata_anlgOutEventDefVariation(pPoint, TMWDEFS_CLASS_MASK_ALL)), curLen, bGetLen);
    }
    else
    {
      _valueString(pSDNPSession, "defaultChangeEventVariation",   _variationGroupString(pSDNPSession->obj42DefaultVariation), curLen, bGetLen);
    }
#endif

#if SDNPDATA_SUPPORT_OBJ43
    if(pSDNPSession->obj43DefaultVariation == 0)
    {
      _valueString(pSDNPSession, "defaultCommandEventVariation", _variationString(sdnpdata_anlgOutCmdEventDefVariation(pPoint, TMWDEFS_CLASS_MASK_ALL)), curLen, bGetLen);
    }
    else
    {
      _valueString(pSDNPSession, "defaultCommandEventVariation",   _variationGroupString(pSDNPSession->obj43DefaultVariation), curLen, bGetLen);
    }
#endif
#endif

    tmwtarg_snprintf(buf, sizeof(buf), "<maxTimeSelectOperate>%d</maxTimeSelectOperate>\n", pSDNPSession->selectTimeout);
    _appendString(pSDNPSession, buf, curLen, bGetLen);

    _appendString(pSDNPSession, "<supportedControlOperations>\n", curLen, bGetLen);
    pSDNPSession->xml2CurIndent++;

    _emptyElementString(pSDNPSession, curLen, bGetLen, "supportSelectOperate", TMWDEFS_TOBOOL(supportMask, SDNPDATA_XML_SUP_SELOP));
    _emptyElementString(pSDNPSession, curLen, bGetLen, "supportDirectOperate", TMWDEFS_TOBOOL(supportMask, SDNPDATA_XML_SUP_DIROP));
    _emptyElementString(pSDNPSession, curLen, bGetLen, "supportDirectOperateNoAck", TMWDEFS_TOBOOL(supportMask, SDNPDATA_XML_SUP_DIRNOACK));

    pSDNPSession->xml2CurIndent--;
    _appendString(pSDNPSession, "</supportedControlOperations>\n", curLen, bGetLen);

    if (pConfig != TMWDEFS_NULL)
    {
      tmwtarg_snprintf(buf, sizeof(buf), "<minTransmittedValue>%g</minTransmittedValue>\n", pConfig->minTransmittedValue);
      _appendString(pSDNPSession, buf, curLen, bGetLen);

      tmwtarg_snprintf(buf, sizeof(buf), "<maxTransmittedValue>%g</maxTransmittedValue>\n", pConfig->maxTransmittedValue);
      _appendString(pSDNPSession, buf, curLen, bGetLen);

      tmwtarg_snprintf(buf, sizeof(buf), "<scaleOffset>%g</scaleOffset>\n", pConfig->scaleOffset);
      _appendString(pSDNPSession, buf, curLen, bGetLen);

      tmwtarg_snprintf(buf, sizeof(buf), "<scaleFactor>%g</scaleFactor>\n", pConfig->scaleFactor);
      _appendString(pSDNPSession, buf, curLen, bGetLen);

      tmwtarg_snprintf(buf, sizeof(buf), "<resolution>%g</resolution>\n", pConfig->resolution);
      _appendString(pSDNPSession, buf, curLen, bGetLen);

      _valueString(pSDNPSession, "units", pConfig->units, curLen, bGetLen);
    }

#if SDNPDATA_SUPPORT_XML2_DNPDATA
    if(pSDNPSession->xml2WriteDnpData)
    {
#if TMWCNFG_SUPPORT_DOUBLE
      TMWTYPES_DOUBLE dvalue;
      dvalue = dnputil_getAnalogValueDouble(&value);
#elif TMWCNFG_SUPPORT_FLOAT
      TMWTYPES_SFLOAT fvalue;
      fvalue = dnputil_getAnalogValueFloat(&value, &flags);
#else
      TMWTYPES_ULONG ulvalue;
      ulvalue = dnputil_getAnalogValueLong(&value, &flags);
#endif
      _appendString(pSDNPSession, "<dnpData>\n", curLen, bGetLen);

#if TMWCNFG_SUPPORT_DOUBLE
      tmwtarg_snprintf(buf, sizeof(buf), "<value>%g</value>\n", dvalue);
#elif TMWCNFG_SUPPORT_FLOAT
      tmwtarg_snprintf(buf, sizeof(buf), "<value>%g</value>\n", fvalue);
#else
      tmwtarg_snprintf(buf, sizeof(buf), "<value>%g</value>\n", ulvalue);
#endif

      pSDNPSession->xml2CurIndent++;
      _appendString(pSDNPSession, buf, curLen, bGetLen);

      tmwtarg_snprintf(buf, sizeof(buf), "<quality>%d</quality>\n", flags&0x3f);
      _appendString(pSDNPSession, buf, curLen, bGetLen);

      /* NotSupported timestamp */
      /* NotSupported control */
      /* NotSupported status */

      pSDNPSession->xml2CurIndent--;
      _appendString(pSDNPSession, "</dnpData>\n", curLen, bGetLen);
    }
#endif

    pSDNPSession->xml2CurIndent--;
    _appendString(pSDNPSession, "</analogOutput>\n", curLen, bGetLen);
    pSDNPSession->xml2CurIndent--;
  }
}
#endif

#if SDNPDATA_SUPPORT_OBJ70
/* function: _buildFilePoint */
static TMWTYPES_BOOL TMWDEFS_LOCAL _buildFilePoint(SDNPSESN *pSDNPSession, TMWTYPES_USHORT pointNum, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  SDNPDATA_XML_PERFILECONFIG config;

  memset(&config, 0, sizeof(config));
  if(sdnpdata_XmlGetPerFileConfig(pSDNPSession->pDbHandle, pointNum, &config))
  {
    /* files */
    if(config.filename[0] !=0)
    {
      pSDNPSession->xml2CurIndent++;
      _appendString(pSDNPSession, "<sequentialFile>\n", curLen, bGetLen);

      pSDNPSession->xml2CurIndent++;
      _valueString(pSDNPSession, "fileName", config.filename, curLen, bGetLen);

      _valueString(pSDNPSession, "eventClass", _classMaskString(config.eventClass), curLen, bGetLen);

      _boolString(pSDNPSession, curLen, bGetLen, "readAuthenticateRequired",  config.readAuthenticateRequired);
      _boolString(pSDNPSession, curLen, bGetLen, "writeAuthenticateRequired",  config.writeAuthenticateRequired);
      _boolString(pSDNPSession, curLen, bGetLen, "deleteAuthenticateRequired",  config.deleteAuthenticateRequired);

      _valueString(pSDNPSession, "description", config.description, curLen, bGetLen);

      pSDNPSession->xml2CurIndent--;
      _appendString(pSDNPSession, "</sequentialFile>\n", curLen, bGetLen);
      pSDNPSession->xml2CurIndent--;
    }
    return TMWDEFS_TRUE;
  }
  else
  {
    return TMWDEFS_FALSE;
  }
}
#endif

#if SDNPDATA_SUPPORT_DATASETS
/* function: _toDescrType */
static char * TMWDEFS_LOCAL _toDescrType(TMWTYPES_ULONG type)
{
  switch(type)
  {
  case DNPDEFS_DATASET_DESCR_ID:
    return "id";
    break;

  case DNPDEFS_DATASET_DESCR_UUID:
    return "uuid";
    break;

  case DNPDEFS_DATASET_DESCR_NSPC:
    return "nspc";
    break;

  case DNPDEFS_DATASET_DESCR_NAME:
    return "name";
    break;

  case DNPDEFS_DATASET_DESCR_DAEL:
    return "dael";
    break;

  case DNPDEFS_DATASET_DESCR_PTYP:
    return "ptyp";
    break;

  case DNPDEFS_DATASET_DESCR_CTLV:
    return "ctlv";
    break;

  case DNPDEFS_DATASET_DESCR_CTLS:
    return "ctls";
    break;

  default:
    return "unknown";
    break;
  }
}

/* function: _toDataType */
static char * TMWDEFS_LOCAL _toDataType(TMWTYPES_ULONG type)
{
  switch(type)
  {
  case DNPDEFS_DATASET_TYPE_VSTR:
    return "vstr";
    break;
  case DNPDEFS_DATASET_TYPE_UINT:
    return "uint";
    break;
  case DNPDEFS_DATASET_TYPE_INT:
    return "int";
    break;
  case DNPDEFS_DATASET_TYPE_FLT:
    return "flt";
    break;
  case DNPDEFS_DATASET_TYPE_OSTR:
    return "ostr";
    break;
  case DNPDEFS_DATASET_TYPE_BSTR:
    return "bstr";
    break;
  case DNPDEFS_DATASET_TYPE_TIME:
    return "time";
    break;
  case DNPDEFS_DATASET_TYPE_UNCD:
    return "uncd";
    break;
  default:
    return "none";
    break;
  }
}

/* function: _toDatasetOutData */
static TMWTYPES_BOOL TMWDEFS_LOCAL _toDatasetOutData(DNPDATA_DATASET_VALUE *pDataSetValue, DNPDEFS_DATASET_TYPE_CODE dataType, TMWTYPES_UCHAR *pOut, int maxLength)
{
  int length = 0;
  TMWTYPES_BOOL hexValues = TMWDEFS_FALSE;
  TMWTYPES_CHAR buf[256];

  /* null terminate empty string in case of failure */
  *pOut = 0;

  switch(pDataSetValue->type)
  {
    case DNPDATA_VALUE_STRPTR:
      length = pDataSetValue->length;
      if(dataType == DNPDEFS_DATASET_TYPE_VSTR)
      {
        if(length+1 > maxLength)
          length = maxLength-1;
        memcpy(buf, pDataSetValue->value.pStrValue, length);
      }
      else
      {
        int j;
        int len = 0;
        for(j=0; j<length; j++)
        {
          if(len < (maxLength-4))
          {
          len += tmwtarg_snprintf((buf + len), sizeof(buf)-len, "%02x ", pDataSetValue->value.pStrValue[j]);
          }
        }
        length = len;
        hexValues = TMWDEFS_TRUE;
      }
      break;
    case DNPDATA_VALUE_STRARRAY:
      length = pDataSetValue->length;
      if(dataType == DNPDEFS_DATASET_TYPE_VSTR)
      {
        if(length+1 > maxLength)
          length = maxLength-1;

        memcpy(buf, pDataSetValue->value.strValue, length);
      }
      else
      {
        int j;
        int len = 0;
        for(j=0; j<length; j++)
        {
          if(len < (maxLength-4))
          {
            len += tmwtarg_snprintf((buf + len), sizeof(buf)-len, "%02x ", pDataSetValue->value.strValue[j]);
          }
        }
        length = len;
        hexValues = TMWDEFS_TRUE;
      }
      break;
    case DNPDATA_VALUE_UINT32:
      length = tmwtarg_snprintf(buf, maxLength, "%d",pDataSetValue->value.uint32Value);
      break;
    case DNPDATA_VALUE_INT32:
      length = tmwtarg_snprintf(buf, maxLength, "%d",pDataSetValue->value.int32Value);
      break;
    case DNPDATA_VALUE_SFLT:
      length = tmwtarg_snprintf(buf, maxLength, "%g",pDataSetValue->value.sfltValue);
      break;
    case DNPDATA_VALUE_DOUBLE:
      length = tmwtarg_snprintf(buf, maxLength, "%g",pDataSetValue->value.doubleValue);
      break;
    case DNPDATA_VALUE_TIME:
      {
        char timeBuf[64];
        tmwdiag_time2string(&pDataSetValue->value.timeValue, TMWDEFS_TIME_FORMAT_56, timeBuf, sizeof(timeBuf), TMWDEFS_FALSE);
        length = tmwtarg_snprintf(buf, maxLength, timeBuf);
      }
      break;
    default:
      break;
  }
  if(length > 0)
  {
    memcpy(pOut, buf, length);
    *(pOut+length) = 0;
  }

  return hexValues;
}

/* function: _buildDatasetProtoStart */
static void TMWDEFS_LOCAL _buildDatasetProtoStart(SDNPSESN *pSDNPSession, void *pProtoPoint, TMWTYPES_UCHAR *pUUID, TMWTYPES_USHORT pointNum, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  int i;
  int len;
  TMWTYPES_CHAR *pDesc;
  TMWTYPES_CHAR buf[256];

  pSDNPSession->xml2CurIndent++;
  _appendString(pSDNPSession, "<datasetPrototype>\n", curLen, bGetLen);

  pSDNPSession->xml2CurIndent++;
  _appendString(pSDNPSession, "<configuration>\n", curLen, bGetLen);
  pSDNPSession->xml2CurIndent++;

  /* prototypeDefinition */
  _pointListDefinition(pSDNPSession, DNPDEFS_OBJ_85_DATASET_PROTO, curLen, bGetLen);

  pSDNPSession->xml2CurIndent--;
  _appendString(pSDNPSession, "</configuration>\n", curLen, bGetLen);

  _appendString(pSDNPSession, "<prototype>\n", curLen, bGetLen);

  tmwtarg_snprintf(buf, sizeof(buf), "<id>%d</id>\n", pointNum);
  pSDNPSession->xml2CurIndent++;
  _appendString(pSDNPSession, buf, curLen, bGetLen);

  len = 0;
  for(i=0; i< 16; i++)
    len += tmwtarg_snprintf((char *)buf+len, sizeof(buf)-len, "%02x ", pUUID[i]);

  _valueString(pSDNPSession, "uuid", (TMWTYPES_CHAR*)buf, curLen, bGetLen);

  /* description, mandatory*/
  pDesc = sdnpdata_datasetGetDescription(pProtoPoint);
  if((pDesc == TMWDEFS_NULL) || (*pDesc == 0))
    _valueString(pSDNPSession, "description", "description", curLen, bGetLen);
  else
    _valueString(pSDNPSession, "description", pDesc, curLen, bGetLen);
}

/* function: _buildDatasetProtoElem */
static void TMWDEFS_LOCAL _buildDatasetProtoElem(SDNPSESN *pSDNPSession, DNPDATA_DATASET_DESCR_ELEM *pDescrElems, TMWTYPES_USHORT elemNum, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  TMWTYPES_CHAR buf[128];
  TMWTYPES_UCHAR name[256];

  /* namespace if present must be first element. If it is, second element should be name */
  /* if namespace is first element display it */
  if(elemNum == 0)
  {
    if(pDescrElems[0].descrElemType == DNPDEFS_DATASET_DESCR_NSPC)
    {
      _toDatasetOutData(&pDescrElems[0].ancillaryValue, DNPDEFS_DATASET_TYPE_VSTR, name, 256);
      _valueString(pSDNPSession, "namespace", (TMWTYPES_CHAR*)name, curLen, bGetLen);
      return;
    }
  }
  else if(elemNum == 1)
  {
    /* if second element is name */
    if(pDescrElems[1].descrElemType == DNPDEFS_DATASET_DESCR_NAME)
    {
      _toDatasetOutData(&pDescrElems[1].ancillaryValue, DNPDEFS_DATASET_TYPE_VSTR, name, 256);
      _valueString(pSDNPSession, "name", (TMWTYPES_CHAR*)name, curLen, bGetLen);
      return;
    }
  }
  _appendString(pSDNPSession,"<dataElement>\n", curLen, bGetLen);

  pSDNPSession->xml2CurIndent++;
  _valueString(pSDNPSession, "elemTypeCode", _toDescrType(pDescrElems[elemNum].descrElemType), curLen, bGetLen);

  _valueString(pSDNPSession, "dataType", _toDataType(pDescrElems[elemNum].dataTypeCode), curLen, bGetLen);

  tmwtarg_snprintf(buf, sizeof(buf), "<maxDataLength>%d</maxDataLength>\n", pDescrElems[elemNum].maxDataLength);
  _appendString(pSDNPSession, buf, curLen, bGetLen);

  _toDatasetOutData(&pDescrElems[elemNum].ancillaryValue, DNPDEFS_DATASET_TYPE_VSTR, name, 256);

  _valueString(pSDNPSession, "elementName", (TMWTYPES_CHAR*)name, curLen, bGetLen);

  pSDNPSession->xml2CurIndent--;
  _appendString(pSDNPSession, "</dataElement>\n", curLen, bGetLen);
}

/* function: _buildDatasetProtoEnd */
static void TMWDEFS_LOCAL _buildDatasetProtoEnd(SDNPSESN *pSDNPSession, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  pSDNPSession->xml2CurIndent--;
  _appendString(pSDNPSession, "</prototype>\n", curLen, bGetLen);

  pSDNPSession->xml2CurIndent--;
  _appendString(pSDNPSession, "</datasetPrototype>\n", curLen, bGetLen);
  pSDNPSession->xml2CurIndent--;
}

/* function: _datasetDescrExpanded */
static void TMWDEFS_LOCAL _datasetDescrExpanded(void *pDbHandle, TMWTYPES_USHORT pointNum, DNPDEFS_DATASET_TYPE_CODE *pDataTypeArray)
{
  void *pPoint = sdnpdata_datasetDescrGetPoint(pDbHandle, pointNum);
  if(pPoint != TMWDEFS_NULL)
  {
    int i;
    TMWTYPES_UCHAR numberElems;
    int expandedIndex = 0;
    DNPDATA_DATASET_DESCR_ELEM *pDescrElem = sdnpdata_datasetDescrReadCont(pPoint, &numberElems);

    for(i=0; i<numberElems; i++)
    {
      if( (pDescrElem->descrElemType == DNPDEFS_DATASET_DESCR_DAEL)
        ||(pDescrElem->descrElemType == DNPDEFS_DATASET_DESCR_CTLS)
        ||(pDescrElem->descrElemType == DNPDEFS_DATASET_DESCR_CTLV))
      {
        pDataTypeArray[expandedIndex++] = pDescrElem->dataTypeCode;
      }
      else if(pDescrElem->descrElemType == DNPDEFS_DATASET_DESCR_PTYP)
      {
        /* expand out prototype  */
        TMWTYPES_USHORT protoPointNumber;
        TMWTYPES_UCHAR *pUUID;

        if(pDescrElem->ancillaryValue.type == DNPDATA_VALUE_STRPTR)
          pUUID = pDescrElem->ancillaryValue.value.pStrValue;
        else
          pUUID = pDescrElem->ancillaryValue.value.strValue;

        if(sdnpdata_datasetProtoGetID(pDbHandle, pUUID, &protoPointNumber))
        {
          int j;
          TMWTYPES_UCHAR numberProtoElems;
          DNPDATA_DATASET_DESCR_ELEM *pProtoElem;
          TMWTYPES_UCHAR tempUUID[16];
          void *pProto = sdnpdata_datasetProtoGetPoint(pDbHandle, protoPointNumber);
          pProtoElem = sdnpdata_datasetProtoRead(pProto, &numberProtoElems, tempUUID);
          for(j=0; j<numberProtoElems; j++)
          {
            if((pProtoElem->descrElemType == DNPDEFS_DATASET_DESCR_DAEL)
              || (pProtoElem->descrElemType == DNPDEFS_DATASET_DESCR_CTLV)
              || (pProtoElem->descrElemType == DNPDEFS_DATASET_DESCR_CTLS))
            {
              pDataTypeArray[expandedIndex++] = pProtoElem->dataTypeCode;
            }
            pProtoElem++;
          }
        }
      }
      pDescrElem++;
    }
  }
}

/* function: _buildDatasetDescrStart */
static void TMWDEFS_LOCAL _buildDatasetDescrStart(SDNPSESN *pSDNPSession, void *pDescrPoint, TMWTYPES_USHORT pointNum, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  TMWTYPES_CHAR *pDesc;
  TMWTYPES_CHAR buf[256];

  pSDNPSession->xml2CurIndent++;
  _appendString(pSDNPSession, "<datasetDescriptor>\n", curLen, bGetLen);
  pSDNPSession->xml2CurIndent++;
  _appendString(pSDNPSession, "<configuration>\n", curLen, bGetLen);

  _pointListDefinition(pSDNPSession, DNPDEFS_OBJ_86_DATASET_DESCR, curLen, bGetLen);

  _appendString(pSDNPSession, "</configuration>\n", curLen, bGetLen);
  /*
    description
  */

  _appendString(pSDNPSession, "<descriptor>\n", curLen, bGetLen);

  tmwtarg_snprintf(buf, sizeof(buf), "<id>%d</id>\n", pointNum);
  pSDNPSession->xml2CurIndent++;
  _appendString(pSDNPSession, buf, curLen, bGetLen);

  /* description, mandatory*/
  pDesc = sdnpdata_datasetGetDescription(pDescrPoint);
  if((pDesc == TMWDEFS_NULL) || (*pDesc == 0))
    _valueString(pSDNPSession, "description", "description", curLen, bGetLen);
  else
    _valueString(pSDNPSession, "description", pDesc, curLen, bGetLen);

  /* properties is only a capability */
  /* changeEventClass is only a capability */
  /* class0ResponseMode is only a capability */

  _appendString(pSDNPSession, "<datasetElements>\n", curLen, bGetLen);

  pSDNPSession->xml2CurIndent++;
}

/* function: _buildDatasetDescrElem */
static void TMWDEFS_LOCAL _buildDatasetDescrElem(SDNPSESN *pSDNPSession, DNPDATA_DATASET_DESCR_ELEM *pDescrElems, TMWTYPES_UCHAR elemNum, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  TMWTYPES_UCHAR name[256];
  TMWTYPES_CHAR buf[256];

  /* name is optional, but would be the first element if present */
  if((elemNum == 0) && (pDescrElems[0].descrElemType == DNPDEFS_DATASET_DESCR_NAME))
  {
    _toDatasetOutData(&pDescrElems[0].ancillaryValue, DNPDEFS_DATASET_TYPE_OSTR, name, 256);
    _valueString(pSDNPSession, "name", (TMWTYPES_CHAR*)name, curLen, bGetLen);
    return;
  }

  _appendString(pSDNPSession, "<dataSetElement>\n", curLen, bGetLen);

  /* We do not have a description per element, other than the name. */

  if(pDescrElems[elemNum].descrElemType == DNPDEFS_DATASET_DESCR_PTYP)
  {
    pSDNPSession->xml2CurIndent++;
    _appendString(pSDNPSession, "<prototypeElement>\n", curLen, bGetLen);

    pSDNPSession->xml2CurIndent++;
    _toDatasetOutData(&pDescrElems[elemNum].ancillaryValue, DNPDEFS_DATASET_TYPE_OSTR, name, 256);
    tmwtarg_snprintf(buf, sizeof(buf), "<uuid>%s</uuid>\n", name);
    _appendString(pSDNPSession, buf, curLen, bGetLen);

    pSDNPSession->xml2CurIndent--;
    _appendString(pSDNPSession, "</prototypeElement>\n", curLen, bGetLen);
  }
  else /*dataElement */
  {
    pSDNPSession->xml2CurIndent++;
    _appendString(pSDNPSession, "<dataElement>\n", curLen, bGetLen);

    pSDNPSession->xml2CurIndent++;
    _valueString(pSDNPSession, "elemTypeCode", _toDescrType(pDescrElems[elemNum].descrElemType), curLen, bGetLen);

    _valueString(pSDNPSession, "dataType", _toDataType(pDescrElems[elemNum].dataTypeCode), curLen, bGetLen);

    tmwtarg_snprintf(buf, sizeof(buf), "<maxDataLength>%d</maxDataLength>\n", pDescrElems[elemNum].maxDataLength);
    _appendString(pSDNPSession, buf, curLen, bGetLen);

    if(pDescrElems[elemNum].descrElemType == DNPDEFS_DATASET_DESCR_UUID
      || pDescrElems[elemNum].descrElemType == DNPDEFS_DATASET_DESCR_PTYP)
    {
      _toDatasetOutData(&pDescrElems[elemNum].ancillaryValue, DNPDEFS_DATASET_TYPE_OSTR, name, 256);
      tmwtarg_snprintf(buf, sizeof(buf), "<elementName>%s</elementName>\n", name);
    }
    else
    {
      _toDatasetOutData(&pDescrElems[elemNum].ancillaryValue, DNPDEFS_DATASET_TYPE_VSTR, name, 256);
      tmwtarg_snprintf(buf, sizeof(buf), "<elementName>%s</elementName>\n", name);
    }
    pSDNPSession->xml2CurIndent++;
    _appendString(pSDNPSession, buf, curLen, bGetLen);

    pSDNPSession->xml2CurIndent--;
    _appendString(pSDNPSession, "</dataElement>\n", curLen, bGetLen);
    pSDNPSession->xml2CurIndent--;
  }

  pSDNPSession->xml2CurIndent--;
  _appendString(pSDNPSession, "</dataSetElement>\n", curLen, bGetLen);

}

/* function: _buildDatasetDescrIndex */
static void TMWDEFS_LOCAL _buildDatasetDescrIndex(SDNPSESN *pSDNPSession, void *pDbHandle, TMWTYPES_USHORT pointNum, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  int i;
  void *pPoint;
  DNPDATA_DATASET_DESCR_INDEX *pIndexElems;
  TMWTYPES_UCHAR numberElems;
  TMWTYPES_CHAR buf[256];

  pPoint = sdnpdata_datasetDescrGetPoint(pDbHandle, pointNum);
  if(pPoint == TMWDEFS_NULL)
    return;

  pSDNPSession->xml2CurIndent--;
  _appendString(pSDNPSession, "</datasetElements>\n", curLen, bGetLen);

  pIndexElems = sdnpdata_datasetDescrReadIndex(pPoint, &numberElems);
  for(i=0; i<numberElems; i++ )
  {
    pSDNPSession->xml2CurIndent++;
    _appendString(pSDNPSession, "<pointIndexAttribute>\n", curLen, bGetLen);

    tmwtarg_snprintf(buf, sizeof(buf), "<groupNumber>%d</groupNumber>\n", pIndexElems[i].pointType);
    pSDNPSession->xml2CurIndent++;
    _appendString(pSDNPSession, buf, curLen, bGetLen);

    tmwtarg_snprintf(buf, sizeof(buf), "<pointIndex>%d</pointIndex>\n", pIndexElems[i].pointIndex);
    _appendString(pSDNPSession, buf, curLen, bGetLen);

    pSDNPSession->xml2CurIndent--;
    _appendString(pSDNPSession, "</pointIndexAttribute>\n", curLen, bGetLen);
    pSDNPSession->xml2CurIndent--;
  }

  pSDNPSession->xml2CurIndent--;
  _appendString(pSDNPSession, "</descriptor>\n", curLen, bGetLen);
}

  /* function: _buildDatasetDescrData */
static void TMWDEFS_LOCAL _buildDatasetDescrData(SDNPSESN *pSDNPSession, void *pDbHandle, TMWTYPES_USHORT pointNum, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  void *pPoint;

  pPoint = sdnpdata_datasetDescrGetPoint(pDbHandle, pointNum);
  if(pPoint == TMWDEFS_NULL)
    return;

#if SDNPDATA_SUPPORT_XML2_DNPDATA
  _appendString(pSDNPSession, "<datasetPoint>\n", curLen, bGetLen);
  pPoint = sdnpdata_datasetGetPoint(pDbHandle, pointNum);
  if(pPoint != TMWDEFS_NULL)
  {
    int i;
    DNPDATA_DATASET_VALUE *pDataElems;
    DNPDEFS_DATASET_TYPE_CODE dataTypeArray[256];
    TMWTYPES_UCHAR numberElems;
    TMWDTIME timeStamp;

    /* Init this in case _datasetDescrExpanded fails to set array properly */
    memset(dataTypeArray, 0, sizeof(dataTypeArray));

    pDataElems = sdnpdata_datasetRead(pPoint, &numberElems, &timeStamp);
    _datasetDescrExpanded(pDbHandle, pointNum, dataTypeArray);
    for(i=0; i<numberElems; i++ )
    {
      if(pSDNPSession->xml2WriteDnpData)
      {
        pSDNPSession->xml2CurIndent++;
        _appendString(pSDNPSession, "<dnpData>\n", curLen, bGetLen);

        pSDNPSession->xml2CurIndent++;
        /*TBD This does not actually show current value.  */
        _valueString(pSDNPSession, _toDataType(dataTypeArray[i]), "0", curLen, bGetLen);
        pSDNPSession->xml2CurIndent--;

        _appendString(pSDNPSession, "</dnpData>\n", curLen, bGetLen);
        pSDNPSession->xml2CurIndent--;
      }
    }
  }
  _appendString(pSDNPSession, "</datasetPoint>\n", curLen, bGetLen);
#endif

  pSDNPSession->xml2CurIndent--;
  _appendString(pSDNPSession, "</datasetDescriptor>\n", curLen, bGetLen);
  pSDNPSession->xml2CurIndent--;
}
#endif



#if SDNPDATA_SUPPORT_OBJ110
/* function: _buildOctetStringPoint */
static void TMWDEFS_LOCAL _buildOctetStringPoint(SDNPSESN *pSDNPSession, void *pDbHandle, TMWTYPES_USHORT pointNum, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{ void *pPoint;
  TMWTYPES_BOOL extendedString;
  TMWTYPES_UCHAR strLength;
  TMWTYPES_CHAR buf[128];
  TMWTYPES_UCHAR strBuf[DNPDEFS_MAX_STRING_LENGTH+1];

  extendedString = TMWDEFS_FALSE;
  pPoint = sdnpdata_strGetPoint(pDbHandle, pointNum);
#if SDNPDATA_SUPPORT_OBJ114
  if (pPoint == TMWDEFS_NULL)
  {
    pPoint = sdnpdata_extStrGetPoint(pDbHandle, pointNum);
    extendedString = TMWDEFS_TRUE;
  }
#endif
  if(pPoint != TMWDEFS_NULL)
  {
    TMWTYPES_CHAR *pDesc;
    TMWDEFS_CLASS_MASK classMask;
    TMWTYPES_BOOL pointInClass0;
    SDNPDATA_XML_STRINGPOINTCONFIG config;

    pDesc = sdnpdata_strGetDescription(pPoint);
    sdnpdata_strRead(pPoint, DNPDEFS_MAX_STRING_LENGTH, strBuf, &strLength);

#if SDNPDATA_SUPPORT_OBJ111
    classMask = sdnpdata_strEventClass(pPoint);
#else
    classMask = TMWDEFS_CLASS_MASK_NONE;
#endif

    pSDNPSession->xml2CurIndent++;
    _appendString(pSDNPSession, "<octetString>\n", curLen, bGetLen);

    tmwtarg_snprintf(buf, sizeof(buf), "<index>%d</index>\n", pointNum);
    pSDNPSession->xml2CurIndent++;
    _appendString(pSDNPSession, buf, curLen, bGetLen);

    /* name */
    memset(&config, 0, sizeof(config));
    if(sdnpdata_XmlGetStringPointConfig(pPoint, &config))
    {
      /* name */
      _valueString(pSDNPSession, "name", config.name, curLen, bGetLen);
    }

    _valueString(pSDNPSession, "description", pDesc, curLen, bGetLen);

    /* inputPointType contains changeEventClass and includedInClass0Response */
#if SDNPDATA_SUPPORT_CLASS0_POINT
    pointInClass0 = TMWDEFS_FALSE;
    if((_class0Response(pSDNPSession, DNPDEFS_OBJ_110_STRING_DATA)) && sdnpdata_strIsClass0(pPoint))
    {
      pointInClass0 = TMWDEFS_TRUE;
    }
    _putInputPointType(pSDNPSession, classMask, pointInClass0, curLen, bGetLen);
#else
    _putInputPointType(pSDNPSession, classMask, _class0Response(pSDNPSession, DNPDEFS_OBJ_110_STRING_DATA), curLen, bGetLen);
#endif

#if SDNPDATA_SUPPORT_OBJ114
    if(extendedString)
      _valueString(pSDNPSession, "groupNumberUsed", (TMWTYPES_CHAR *)"g114", curLen, bGetLen);
    else
      _valueString(pSDNPSession, "groupNumberUsed", (TMWTYPES_CHAR *)"g110", curLen, bGetLen);
#else
    /* New in schema 2.10, corrected in 2.11 */
    _valueString(pSDNPSession, "groupNumberUsed", (TMWTYPES_CHAR *)"g110", curLen, bGetLen);
#endif

#if SDNPDATA_SUPPORT_XML2_DNPDATA
    if(pSDNPSession->xml2WriteDnpData)
    {
      _appendString(pSDNPSession, "<dnpData>\n", curLen, bGetLen);

      /* Null terminate string */
      strBuf[strLength]='\0';

      pSDNPSession->xml2CurIndent++;
      _valueString(pSDNPSession, "value", (TMWTYPES_CHAR *)strBuf, curLen, bGetLen);

      pSDNPSession->xml2CurIndent--;
      _appendString(pSDNPSession, "</dnpData>\n", curLen, bGetLen);
    }
#endif

    pSDNPSession->xml2CurIndent--;
    _appendString(pSDNPSession, "</octetString>\n", curLen, bGetLen);
    pSDNPSession->xml2CurIndent--;
  }
}
#endif

#if SDNPDATA_SUPPORT_OBJ112
static void TMWDEFS_LOCAL _buildVirtualTerminalPoint(SDNPSESN *pSDNPSession, void *pDbHandle, TMWTYPES_USHORT pointNum, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  void *pPoint;
  TMWTYPES_UCHAR strLength;
  SDNPDATA_XML_VTERMPOINTCONFIG config;
  TMWTYPES_CHAR buf[128];
  TMWTYPES_UCHAR strBuf[256];

  pPoint = sdnpdata_vtermGetPoint(pDbHandle, pointNum);
  if(pPoint != TMWDEFS_NULL)
  {
    TMWTYPES_CHAR *pDesc;
    TMWDEFS_CLASS_MASK classMask;

    pDesc = sdnpdata_vtermGetDescription(pPoint);
    sdnpdata_vtermRead(pPoint, (sizeof(strBuf)-1), strBuf, &strLength);
#if SDNPDATA_SUPPORT_OBJ113
    classMask = classMask = sdnpdata_vtermEventClass(pPoint);
#else
    classMask = TMWDEFS_CLASS_MASK_NONE;
#endif

    pSDNPSession->xml2CurIndent++;
    _appendString(pSDNPSession, "<virtualTerminal>\n", curLen, bGetLen);

    tmwtarg_snprintf(buf, sizeof(buf), "<index>%d</index>\n", pointNum);
    pSDNPSession->xml2CurIndent++;
    _appendString(pSDNPSession, buf, curLen, bGetLen);

    /* name */
    memset(&config, 0, sizeof(config));
    if(sdnpdata_XmlGetVtermPointConfig(pPoint, &config))
    {
      /* name */
      _valueString(pSDNPSession, "name", config.name, curLen, bGetLen);
    }

    _valueString(pSDNPSession, "description", pDesc, curLen, bGetLen);

    _valueString(pSDNPSession, "changeEventClass", _classMaskString(classMask), curLen, bGetLen);

    pSDNPSession->xml2CurIndent--;
    _appendString(pSDNPSession, "</virtualTerminal>\n", curLen, bGetLen);
    pSDNPSession->xml2CurIndent--;
  }
}
#endif

#if SDNPDATA_SUPPORT_OBJ120
static char * TMWDEFS_LOCAL _toMACString(TMWTYPES_UCHAR type)
{
  switch(type)
  {
  case DNPAUTH_HMAC_SHA1_4OCTET:
    return "<sha1Truncated4/>";

  case DNPAUTH_MAC_SHA1_8OCTET:
    return "<other><explanation>sha1Truncated8 not in schema</explanation></other>";

  case DNPAUTH_MAC_SHA1_10OCTET:
    return "<sha1Truncated10/>";

  case DNPAUTH_MAC_SHA256_8OCTET:
    return "<sha256Truncated8/>";

  case DNPAUTH_MAC_SHA256_16OCTET:
    return "<sha256Truncated16/>";

  case DNPAUTH_MAC_AESGMAC_12OCTET:
    return "<aesGMAC/>";

  default:
    break;

  }
  return "<other><explanation>private</explanation></other>";
}
static char * TMWDEFS_LOCAL _toKeyWrapString(TMWTYPES_UCHAR type)
{
  switch(type)
  {
  case DNPAUTH_KEYWRAP_AES128:
    return "<aes128/>";

  case DNPAUTH_KEYWRAP_AES256:
    return "<aes256/>";

  case SDNPXML_KEYWRAP_PER_USER:
    return "<other><explanation>This algorithm is Per User</explanation></other>";

  default:
    return "<other><explanation>private</explanation></other>";
  }
}

static char * TMWDEFS_LOCAL _toSymMethodString(TMWTYPES_UCHAR type)
{
  switch(type)
  {
  case DNPAUTH_KEYCH_SYMAES128_SHA1:
    return "<aes-128KeyWrapWithSHA-1-HMAC/>";

  case DNPAUTH_KEYCH_SYMAES256_SHA256:
    /* schema 2.09 incorrectly states this as aes-128, it was changed in schema 2.10 */
    return "<aes-256KeyWrapWithSHA-256-HMAC/>";

  case DNPAUTH_KEYCH_SYMAES256_GMAC:
    /* schema 2.09 incorrectly states this as aes-128, it was changed in schema 2.10 */
    return "<aes-256KeyWrapWithAES-GMAC/>";

  default:
    return TMWDEFS_NULL;
  }
}

static char * TMWDEFS_LOCAL _toAsymMethodString(TMWTYPES_UCHAR type)
{
  switch(type)
  {
  case DNPAUTH_KEYCH_ASYM_RSA1024_SHA1:
    return "<RSAES-OAEP-1024WithSHA-1-HMAC/>";

  case DNPAUTH_KEYCH_ASYM_RSA2048_SHA256:
    return "<RSAES-OAEP-2048WithSHA-256-HMAC/>";

  case DNPAUTH_KEYCH_ASYM_RSA3072_SHA256:
    return "<RSAES-OAEP-3072WithSHA-256-HMAC/>";

  case DNPAUTH_KEYCH_ASYM_RSA2048_SHA256_GMAC:
    return "<RSAES-OAEP-2048WithAES-GMAC/>";

  case DNPAUTH_KEYCH_ASYM_RSA3072_SHA256_GMAC:
    return "<RSAES-OAEP-3072WithAES-GMAC/>";

    /* These were added in TB2016-002, but are not yet in device profile */
#if 0
  case DNPAUTH_KEYCH_ASYM_RSA1024_RSA_SHA1:
    return "<SAES-OAEP-1024WithRSA-SHA-1-HMAC/>";

  case DNPAUTH_KEYCH_ASYM_RSA2048_RSA_SHA256:
    return "<RSAES-OAEP-2048WithRSA-SHA-256-HMAC/>";

  case DNPAUTH_KEYCH_ASYM_RSA3072_RSA_SHA256:
    return "<RSAES-OAEP-3072WithRSA-SHA-256-HMAC/>";

  case DNPAUTH_KEYCH_ASYM_RSA2048_RSA_SHA256_GMAC:
    return "<RSAES-OAEP-2048WithRSA-AES-GMAC/>";

  case DNPAUTH_KEYCH_ASYM_RSA3072_RSA_SHA256_GMAC:
    return "<RSAES-OAEP-3072WithRSA-AES-GMAC/>";
#endif

  default:
    return TMWDEFS_NULL;
  }
}
#endif

/* referenceDevice/configuration/deviceConfig */
static void TMWDEFS_LOCAL _buildDeviceConfig(SDNPSESN *pSDNPSession, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  int i;
  SDNPDATA_XML_DEVICECONFIG *pDeviceConfig;

  pDeviceConfig = (SDNPDATA_XML_DEVICECONFIG *)sdnpmem_alloc(SDNPMEM_SIM_XML_CONFIG_TYPE);
  if(pDeviceConfig == TMWDEFS_NULL)
    return;

  memset(pDeviceConfig, 0, sizeof(SDNPDATA_XML_DEVICECONFIG));

  pSDNPSession->xml2CurIndent++;
  _appendString(pSDNPSession, "<deviceConfig>\n", curLen, bGetLen);

	/*  deviceFunction
      vendorName
      deviceName
      hardwareVersion
      softwareVersion
      documentVersionNumber
      dnpLevelSupported
      supportedFunctionBlocks
      notableAdditions
      configurationMethods
      onlineXmlFileNames
      offlineXmlFileNames
      connectionsSupported
  */

  if(sdnpdata_XmlGetDeviceConfig(pSDNPSession->pDbHandle, pDeviceConfig))
  {
    pSDNPSession->xml2CurIndent++;
    _currentValueString(pSDNPSession, "deviceFunction",  "<outstation />", curLen, bGetLen);

    _currentValueValueString(pSDNPSession, "vendorName", pDeviceConfig->vendorName, curLen, bGetLen);
    _currentValueValueString(pSDNPSession, "deviceName", pDeviceConfig->deviceName, curLen, bGetLen);
    _currentValueValueString(pSDNPSession, "hardwareVersion", pDeviceConfig->hardwareVersion, curLen, bGetLen);
    _currentValueValueString(pSDNPSession, "softwareVersion", pDeviceConfig->softwareVersion, curLen, bGetLen);
    _currentValueDecimalString(pSDNPSession, "documentVersionNumber", pDeviceConfig->documentVersionNumber, curLen, bGetLen);


    /* outstation containing a sequence of none level1..level4 */
    _appendString(pSDNPSession, "<dnpLevelSupported>\n", curLen, bGetLen);
    pSDNPSession->xml2CurIndent++;
    _appendString(pSDNPSession, "<currentValue>\n", curLen, bGetLen);
    pSDNPSession->xml2CurIndent++;
    _appendString(pSDNPSession, "<outStation>\n", curLen, bGetLen);
    pSDNPSession->xml2CurIndent++;
    _emptyElementString(pSDNPSession, curLen, bGetLen, "none", pDeviceConfig->dnpLevelSupported.none);
    _emptyElementString(pSDNPSession, curLen, bGetLen, "level1", pDeviceConfig->dnpLevelSupported.level1);
    _emptyElementString(pSDNPSession, curLen, bGetLen, "level2", pDeviceConfig->dnpLevelSupported.level2);
    _emptyElementString(pSDNPSession, curLen, bGetLen, "level3", pDeviceConfig->dnpLevelSupported.level3);
    _emptyElementString(pSDNPSession, curLen, bGetLen, "level4", pDeviceConfig->dnpLevelSupported.level4);
    pSDNPSession->xml2CurIndent--;
    _appendString(pSDNPSession, "</outStation>\n", curLen, bGetLen);
    pSDNPSession->xml2CurIndent--;
    _appendString(pSDNPSession, "</currentValue>\n", curLen, bGetLen);
    pSDNPSession->xml2CurIndent--;
    _appendString(pSDNPSession, "</dnpLevelSupported>\n", curLen, bGetLen);


    _appendString(pSDNPSession, "<supportedFunctionBlocks>\n", curLen, bGetLen);
    pSDNPSession->xml2CurIndent++;
    _appendString(pSDNPSession, "<currentValue>\n", curLen, bGetLen);
    pSDNPSession->xml2CurIndent++;


    _emptyElementString(pSDNPSession, curLen, bGetLen, "selfAddressReservation", pSDNPSession->dnp.enableSelfAddress);
#if SDNPDATA_SUPPORT_DATASETS
    _emptyElementString(pSDNPSession, curLen, bGetLen, "dataSets", TMWDEFS_TRUE);
#endif
#if SDNPDATA_SUPPORT_OBJ70
    _emptyElementString(pSDNPSession, curLen, bGetLen, "fileTransfer", TMWDEFS_TRUE);
#endif
#if SDNPDATA_SUPPORT_OBJ112
    _emptyElementString(pSDNPSession, curLen, bGetLen, "virtualTerminal", TMWDEFS_TRUE);
#endif
    _emptyElementString(pSDNPSession, curLen, bGetLen, "mappingToIEC61850ObjectModels", pDeviceConfig->supportedFunctionBlocks.mappingToIEC61850ObjectModels);
#if SDNPDATA_SUPPORT_OBJ91
    _emptyElementString(pSDNPSession, curLen, bGetLen, "FC31ActivateConfiguration", TMWDEFS_TRUE);
#endif
#if SDNPDATA_SUPPORT_OBJ120
    _emptyElementString(pSDNPSession, curLen, bGetLen, "secureAuthentication", TMWDEFS_TRUE);
#endif
    pSDNPSession->xml2CurIndent--;
    _appendString(pSDNPSession, "</currentValue>\n", curLen, bGetLen);
    pSDNPSession->xml2CurIndent--;
    _appendString(pSDNPSession, "</supportedFunctionBlocks>\n", curLen, bGetLen);


    _appendString(pSDNPSession, "<notableAdditions>\n", curLen, bGetLen);
    pSDNPSession->xml2CurIndent++;
    _appendString(pSDNPSession, "<currentValue>\n", curLen, bGetLen);
    pSDNPSession->xml2CurIndent++;
    for(i=0; i<SDNPXML_MAX_NOTABLEADDITIONS; i++)
    {
      _valueString(pSDNPSession, "note", pDeviceConfig->notableAdditions[i].note, curLen, bGetLen);
    }
    for(i=0; i<SDNPXML_MAX_NOTABLEADDITIONS; i++)
    {
      _valueString(pSDNPSession, "notableAddition", pDeviceConfig->notableAdditions[i].notableAddition, curLen, bGetLen);
    }
    pSDNPSession->xml2CurIndent--;
    _appendString(pSDNPSession, "</currentValue>\n", curLen, bGetLen);
    pSDNPSession->xml2CurIndent--;
    _appendString(pSDNPSession, "</notableAdditions>\n", curLen, bGetLen);


    _appendString(pSDNPSession, "<configurationMethods>\n", curLen, bGetLen);
    pSDNPSession->xml2CurIndent++;
    _appendString(pSDNPSession, "<currentValue>\n", curLen, bGetLen);
    pSDNPSession->xml2CurIndent++;
    _emptyElementString(pSDNPSession, curLen, bGetLen, "xmlViaFileTransfer", pDeviceConfig->configurationMethods.xmlViaFileTransfer);
    _emptyElementString(pSDNPSession, curLen, bGetLen, "xmlViaOtherTransportMechanism", pDeviceConfig->configurationMethods.xmlViaOtherTransportMechanism);
    _emptyElementString(pSDNPSession, curLen, bGetLen, "terminal", pDeviceConfig->configurationMethods.terminal);
    /* "software" */
    _emptyElementString(pSDNPSession, curLen, bGetLen, "proprietaryFileViaFileTransfer", pDeviceConfig->configurationMethods.proprietaryFileViaFileTransfer);
    _emptyElementString(pSDNPSession, curLen, bGetLen, "proprietaryFileViaOtherTransportMechanism", pDeviceConfig->configurationMethods.proprietaryFileViaOtherTransportMechanism);
    _emptyElementString(pSDNPSession, curLen, bGetLen, "direct", pDeviceConfig->configurationMethods.direct);
    _emptyElementString(pSDNPSession, curLen, bGetLen, "factory", pDeviceConfig->configurationMethods.factory);
    _emptyElementString(pSDNPSession, curLen, bGetLen, "protocol", pDeviceConfig->configurationMethods.protocol);
    /* other */
    pSDNPSession->xml2CurIndent--;
    _appendString(pSDNPSession, "</currentValue>\n", curLen, bGetLen);
    pSDNPSession->xml2CurIndent--;
    _appendString(pSDNPSession, "</configurationMethods>\n", curLen, bGetLen);

    _appendString(pSDNPSession, "<onlineXmlFileNames>\n", curLen, bGetLen);
    pSDNPSession->xml2CurIndent++;
    _appendString(pSDNPSession, "<currentValue>\n", curLen, bGetLen);
    pSDNPSession->xml2CurIndent++;
    _emptyElementString(pSDNPSession, curLen, bGetLen, "dnpDPReadSupported", pDeviceConfig->onlineXmlFileNames.dnpDPReadSupported);
    _emptyElementString(pSDNPSession, curLen, bGetLen, "dnpDPCapReadSupported", pDeviceConfig->onlineXmlFileNames.dnpDPReadSupported);
    _emptyElementString(pSDNPSession, curLen, bGetLen, "dnpDPCfgReadSupported", pDeviceConfig->onlineXmlFileNames.dnpDPCfgReadSupported);

    for(i=0; i<SDNPXML_MAX_XMLFILENAMES; i++)
    {
      _xmlFileString(pSDNPSession, &pDeviceConfig->onlineXmlFileNames.xmlFile[i], curLen, bGetLen);
    }
    for(i=0; i<SDNPXML_MAX_XMLFILENAMES; i++)
    {
      _valueString(pSDNPSession, "note", pDeviceConfig->onlineXmlFileNames.note[i], curLen, bGetLen);
    }

    pSDNPSession->xml2CurIndent--;
    _appendString(pSDNPSession, "</currentValue>\n", curLen, bGetLen);
    pSDNPSession->xml2CurIndent--;
    _appendString(pSDNPSession, "</onlineXmlFileNames>\n", curLen, bGetLen);

    _appendString(pSDNPSession, "<offlineXmlFileNames>\n", curLen, bGetLen);
    pSDNPSession->xml2CurIndent++;
    _appendString(pSDNPSession, "<currentValue>\n", curLen, bGetLen);
    pSDNPSession->xml2CurIndent++;
    _emptyElementString(pSDNPSession, curLen, bGetLen, "dnpDPReadSupported", pDeviceConfig->offlineXmlFileNames.dnpDPReadSupported);
    _emptyElementString(pSDNPSession, curLen, bGetLen, "dnpDPWriteSupported", pDeviceConfig->offlineXmlFileNames.dnpDPWriteSupported);
    _emptyElementString(pSDNPSession, curLen, bGetLen, "dnpDPCapReadSupported", pDeviceConfig->offlineXmlFileNames.dnpDPCapReadSupported);
    _emptyElementString(pSDNPSession, curLen, bGetLen, "dnpDPCapWriteSupported", pDeviceConfig->offlineXmlFileNames.dnpDPCapWriteSupported);
    _emptyElementString(pSDNPSession, curLen, bGetLen, "dnpDPCfgReadSupported", pDeviceConfig->offlineXmlFileNames.dnpDPCfgReadSupported);
    _emptyElementString(pSDNPSession, curLen, bGetLen, "dnpDPCfgWriteSupported", pDeviceConfig->offlineXmlFileNames.dnpDPCfgWriteSupported);

    for(i=0; i<SDNPXML_MAX_XMLFILENAMES; i++)
    {
      _xmlFileString(pSDNPSession, &pDeviceConfig->offlineXmlFileNames.xmlFile[i], curLen, bGetLen);
    }
    for(i=0; i<SDNPXML_MAX_XMLFILENAMES; i++)
    {
      _valueString(pSDNPSession, "note", pDeviceConfig->offlineXmlFileNames.note[i], curLen, bGetLen);
    }

    pSDNPSession->xml2CurIndent--;
    _appendString(pSDNPSession, "</currentValue>\n", curLen, bGetLen);
    pSDNPSession->xml2CurIndent--;
    _appendString(pSDNPSession, "</offlineXmlFileNames>\n", curLen, bGetLen);

    _appendString(pSDNPSession, "<connectionsSupported>\n", curLen, bGetLen);
    pSDNPSession->xml2CurIndent++;
    _appendString(pSDNPSession, "<currentValue>\n", curLen, bGetLen);
    pSDNPSession->xml2CurIndent++;
    _emptyElementString(pSDNPSession, curLen, bGetLen, "serial", pDeviceConfig->connectionsSupported.serial);
    _emptyElementString(pSDNPSession, curLen, bGetLen, "network", pDeviceConfig->connectionsSupported.network);
    pSDNPSession->xml2CurIndent--;
    for(i=0; i<SDNPXML_MAX_OTHERCONNECTIONS; i++)
    {
      if(pDeviceConfig->connectionsSupported.other[i][0] != 0)
        _valueOther(pSDNPSession, pDeviceConfig->connectionsSupported.other[i], curLen, bGetLen);
    }
    _appendString(pSDNPSession, "</currentValue>\n", curLen, bGetLen);
    pSDNPSession->xml2CurIndent--;
    _appendString(pSDNPSession, "</connectionsSupported>\n", curLen, bGetLen);
  }

  /* NotSupported conformanceTesting */

  pSDNPSession->xml2CurIndent--;
  _appendString(pSDNPSession, "</deviceConfig>\n", curLen, bGetLen);

  sdnpmem_free(pDeviceConfig);
}

/* referenceDevice/configuration/serialConfig */
static void TMWDEFS_LOCAL _buildSerialConfig(SDNPSESN *pSDNPSession, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  SDNPDATA_XML_SERIALCONFIG *pSerialConfig;

  /* portName
     serialParameters
     baudRate
     flowControl
     linkStatusInterval
     supportsCollisionAvoidance
     interCharacterTimeout
     interCharacterGap
   */

  pSerialConfig = (SDNPDATA_XML_SERIALCONFIG *)sdnpmem_alloc(SDNPMEM_SIM_XML_CONFIG_TYPE);
  if(pSerialConfig == TMWDEFS_NULL)
    return;

  _appendString(pSDNPSession, "<serialConfig>\n", curLen, bGetLen);

  memset(pSerialConfig, 0, sizeof(SDNPDATA_XML_SERIALCONFIG));
  if(sdnpdata_XmlGetSerialConfig(pSDNPSession->pDbHandle, pSerialConfig))
  {
    pSDNPSession->xml2CurIndent++;
    _currentValueValueString(pSDNPSession, "portName", pSerialConfig->portName, curLen, bGetLen);

    if(pSerialConfig->serialParameters.type == SDNPDATA_XML_SERIALPARAMASYNC)
    {
      _currentValueString(pSDNPSession, "serialParameters", "<asynchronous />", curLen, bGetLen);
    }
    else if(pSerialConfig->serialParameters.type == SDNPDATA_XML_SERIALPARAMOTHER)
    {
      _currentValueOther(pSDNPSession, "serialParameters", pSerialConfig->serialParameters.other, curLen, bGetLen);
    }

   _currentValueDecimalString(pSDNPSession, "baudRate", pSerialConfig->baudRate, curLen, bGetLen);

   _appendString(pSDNPSession, "<flowControl>\n", curLen, bGetLen);
    pSDNPSession->xml2CurIndent++;
   _appendString(pSDNPSession, "<currentValue>\n", curLen, bGetLen);
   if(pSerialConfig->flowControl.none)
   {
     pSDNPSession->xml2CurIndent++;
     _appendString(pSDNPSession, "<none />\n", curLen, bGetLen);
     _appendString(pSDNPSession, "<rs232Options />\n", curLen, bGetLen);
     _appendString(pSDNPSession, "<rs422Options />\n", curLen, bGetLen);
     _appendString(pSDNPSession, "<rs485Options />\n", curLen, bGetLen);
   }
   else
   {
     _appendString(pSDNPSession, "<rs232Options>\n", curLen, bGetLen);

      pSDNPSession->xml2CurIndent++;
     _emptyElementString(pSDNPSession, curLen, bGetLen, "assertsRTSBeforeTx", pSerialConfig->flowControl.rs232Options.assertsRTSBeforeTx);
     _emptyElementString(pSDNPSession, curLen, bGetLen, "assertsDTRBeforeTx", pSerialConfig->flowControl.rs232Options.assertsDTRBeforeTx);
     _emptyElementString(pSDNPSession, curLen, bGetLen, "assertsRTSBeforeRx", pSerialConfig->flowControl.rs232Options.assertsRTSBeforeRx);
     _emptyElementString(pSDNPSession, curLen, bGetLen, "assertsDTRBeforeRx", pSerialConfig->flowControl.rs232Options.assertsDTRBeforeRx);
     _emptyElementString(pSDNPSession, curLen, bGetLen, "alwaysAssertsRTS", pSerialConfig->flowControl.rs232Options.alwaysAssertsRTS);
     _emptyElementString(pSDNPSession, curLen, bGetLen, "alwaysAssertsDTR", pSerialConfig->flowControl.rs232Options.alwaysAssertsDTR);
     _assertedString(pSDNPSession, curLen, bGetLen, "requiresCTSBeforeTx", pSerialConfig->flowControl.rs232Options.requiresCTSBeforeTx);
     _assertedString(pSDNPSession, curLen, bGetLen, "requiresDCDBeforeTx", pSerialConfig->flowControl.rs232Options.requiresDCDBeforeTx);
     _assertedString(pSDNPSession, curLen, bGetLen, "requiresDSRBeforeTx", pSerialConfig->flowControl.rs232Options.requiresDSRBeforeTx);
     _assertedString(pSDNPSession, curLen, bGetLen, "requiresRIBeforeTx", pSerialConfig->flowControl.rs232Options.requiresRIBeforeTx);
     _emptyElementString(pSDNPSession, curLen, bGetLen, "rxInactive", pSerialConfig->flowControl.rs232Options.rxInactive);
     _assertedString(pSDNPSession, curLen, bGetLen, "requiresCTSBeforeRx", pSerialConfig->flowControl.rs232Options.requiresCTSBeforeRx);
     _assertedString(pSDNPSession, curLen, bGetLen, "requiresDCDBeforeRx", pSerialConfig->flowControl.rs232Options.requiresDCDBeforeRx);
     _assertedString(pSDNPSession, curLen, bGetLen, "requiresDSRBeforeRx", pSerialConfig->flowControl.rs232Options.requiresDSRBeforeRx);
     _assertedString(pSDNPSession, curLen, bGetLen, "requiresRIBeforeRx", pSerialConfig->flowControl.rs232Options.requiresRIBeforeRx);
     _emptyElementString(pSDNPSession, curLen, bGetLen, "alwaysIgnoresCTS", pSerialConfig->flowControl.rs232Options.alwaysIgnoresCTS);
     _emptyElementString(pSDNPSession, curLen, bGetLen, "alwaysIgnoresDCD", pSerialConfig->flowControl.rs232Options.alwaysIgnoresDCD);
     _emptyElementString(pSDNPSession, curLen, bGetLen, "alwaysIgnoresDSR", pSerialConfig->flowControl.rs232Options.alwaysIgnoresDSR);
     _emptyElementString(pSDNPSession, curLen, bGetLen, "alwaysIgnoresRI", pSerialConfig->flowControl.rs232Options.alwaysIgnoresRI);

     if(pSerialConfig->flowControl.rs232Options.other[0] != 0)
     {
       _valueOther(pSDNPSession, pSerialConfig->flowControl.rs232Options.other, curLen, bGetLen);
     }

      pSDNPSession->xml2CurIndent--;
     _appendString(pSDNPSession, "</rs232Options>\n", curLen, bGetLen);

     _appendString(pSDNPSession, "<rs422Options>\n", curLen, bGetLen);
     _emptyElementString(pSDNPSession, curLen, bGetLen, "requiresIndicationBeforeRx", pSerialConfig->flowControl.rs422Options.requiresIndicationBeforeRx);
     _emptyElementString(pSDNPSession, curLen, bGetLen, "assertsControlBeforeTx", pSerialConfig->flowControl.rs422Options.assertsControlBeforeTx);
     if(pSerialConfig->flowControl.rs422Options.other[0] != 0)
     {
       _valueOther(pSDNPSession, pSerialConfig->flowControl.rs422Options.other, curLen, bGetLen);
     }
     _appendString(pSDNPSession, "</rs422Options>\n", curLen, bGetLen);

     _appendString(pSDNPSession, "<rs485Options>\n", curLen, bGetLen);
     _emptyElementString(pSDNPSession, curLen, bGetLen, "requiresRxInactiveBeforeTx", pSerialConfig->flowControl.rs485Options.requiresRxInactiveBeforeTx);
     if(pSerialConfig->flowControl.rs485Options.other[0] != 0)
     {
       _valueOther(pSDNPSession, pSerialConfig->flowControl.rs485Options.other, curLen, bGetLen);
     }
     _appendString(pSDNPSession, "</rs485Options>\n", curLen, bGetLen);
   }

   pSDNPSession->xml2CurIndent--;
   _appendString(pSDNPSession, "</currentValue>\n", curLen, bGetLen);
   pSDNPSession->xml2CurIndent--;
   _appendString(pSDNPSession, "</flowControl>\n", curLen, bGetLen);

    /* linkStatusInterval */
   if(pSDNPSession->dnp.linkStatusPeriod == 0)
   {
     _currentValueString(pSDNPSession, "linkStatusInterval", "<notSupported />", curLen, bGetLen);
   }
   else
   {
     _currentValueDecimalString(pSDNPSession, "linkStatusInterval", pSDNPSession->dnp.linkStatusPeriod, curLen, bGetLen);
   }


   /* supports collision avoidance */
   if(!pSerialConfig->supportsCollisionAvoidance.supported)
   {
     pSDNPSession->xml2CurIndent++;
    _currentValueString(pSDNPSession, "supportsCollisionAvoidance", "<no />", curLen, bGetLen);
     pSDNPSession->xml2CurIndent--;
   }
   else
   {
     _appendString(pSDNPSession, "<supportsCollisionAvoidance>\n", curLen, bGetLen);
     _currentValueCustom(pSDNPSession, "yes", pSerialConfig->supportsCollisionAvoidance.yes, curLen, bGetLen);
     _appendString(pSDNPSession, "</supportsCollisionAvoidance>\n", curLen, bGetLen);
   }


   /* interCharacterTimeout */
   if(pSerialConfig->interCharacterTimeout.type == SDNPDATA_XML_INTERCHARNOTCHECKED)
   {
     _currentValueString(pSDNPSession, "interCharacterTimeout", "<notChecked />", curLen, bGetLen);
   }
   else if(pSerialConfig->interCharacterTimeout.type == SDNPDATA_XML_INTERCHARNOGAPPERM)
   {
     _currentValueString(pSDNPSession, "interCharacterTimeout", "<noGapPermitted />", curLen, bGetLen);
   }
   else if(pSerialConfig->interCharacterTimeout.type == SDNPDATA_XML_INTERCHARVALUEBIT)
   {
     _currentValueNameDecimalString(pSDNPSession, "interCharacterTimeout", "valueBitTimes", pSerialConfig->interCharacterTimeout.value.value, curLen, bGetLen);
   }
   else if(pSerialConfig->interCharacterTimeout.type == SDNPDATA_XML_INTERCHARVALUEMILL)
   {
     _currentValueNameDecimalString(pSDNPSession, "interCharacterTimeout", "valueMilliseconds", pSerialConfig->interCharacterTimeout.value.value, curLen, bGetLen);
   }
   else if(pSerialConfig->interCharacterTimeout.type == SDNPDATA_XML_INTERCHARVARIABLE)
   {
     _appendString(pSDNPSession, "<interCharacterTimeout>\n", curLen, bGetLen);
     _currentValueOther(pSDNPSession, "variable", pSerialConfig->interCharacterTimeout.value.variable, curLen, bGetLen);
     pSDNPSession->xml2CurIndent--;
     _appendString(pSDNPSession, "</interCharacterTimeout>\n", curLen, bGetLen);
   }


   /* interCharacterGap */
   if(pSerialConfig->interCharacterGap.type == SDNPDATA_XML_INTERCHARGAPNONE)
   {
     _currentValueString(pSDNPSession, "interCharacterGap",  "<none />", curLen, bGetLen);
   }
   else if(pSerialConfig->interCharacterGap.type == SDNPDATA_XML_INTERCHARGAPMAXBIT)
   {
     _currentValueNameDecimalString(pSDNPSession, "interCharacterGap", "maximumBitTimes", pSerialConfig->interCharacterGap.value, curLen, bGetLen);
   }
   else if(pSerialConfig->interCharacterGap.type == SDNPDATA_XML_INTERCHARGAPMAXMILL)
   {
     _currentValueNameDecimalString(pSDNPSession, "interCharacterGap", "maximumMilliseconds", pSerialConfig->interCharacterGap.value, curLen, bGetLen);
   }
  }

  pSDNPSession->xml2CurIndent--;
  _appendString(pSDNPSession, "</serialConfig>\n", curLen, bGetLen);

  sdnpmem_free(pSerialConfig);
}

/* referenceDevice/configuration/networkConfig */
static void TMWDEFS_LOCAL _buildNetworkConfig(SDNPSESN *pSDNPSession, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  int i;
  SDNPDATA_XML_NETWORKCONFIG *pNetworkConfig;

  /*
    portName
    typeOfEndPoint
    ipAddress
    subnetMask
    gatewayIPAddress
    tcpConnectionEstablishment
    ipAddressOfRemoteDevice
    tcpListenPort
    tcpPortOfRemoteDevice
    tcpKeepAliveTimer
    localUDPPort
    destinationUDPPort (Master only)
    udpPortForUnsolicitedNullResponses
    udpPortForResponses
    multipleOutstationConnections
    multipleMasterConnections
    timeSynchronization
  */

  pNetworkConfig = (SDNPDATA_XML_NETWORKCONFIG *)sdnpmem_alloc(SDNPMEM_SIM_XML_CONFIG_TYPE);
  if(pNetworkConfig == TMWDEFS_NULL)
    return;

  _appendString(pSDNPSession, "<networkConfig>\n", curLen, bGetLen);

  memset(pNetworkConfig, 0, sizeof(SDNPDATA_XML_NETWORKCONFIG));
  if(sdnpdata_XmlGetNetworkConfig(pSDNPSession->pDbHandle, pNetworkConfig))
  {
    /* portName */
    pSDNPSession->xml2CurIndent++;
    _currentValueValueString(pSDNPSession, "portName", pNetworkConfig->portName, curLen, bGetLen);

    /* typeOfEndPoint */
    _currentValueString(pSDNPSession, "typeOfEndPoint", _typeOfEndPointString(pNetworkConfig->typeOfEndPoint), curLen, bGetLen);

    /* ipAddress */
    _currentValueNameString(pSDNPSession, "ipAddress", "address", pNetworkConfig->ipAddress, curLen, bGetLen);

    /* subnetMask */
    _currentValueNameString(pSDNPSession, "subnetMask", "mask", pNetworkConfig->subnetMask, curLen, bGetLen);

    /* gatewayIPAddress */
    _currentValueNameString(pSDNPSession, "gatewayIPAddress", "address", pNetworkConfig->gatewayIPAddress, curLen, bGetLen);


    /* tcpConnectionEstablishment */
    if(pNetworkConfig->tcpConnectionEstablishment.type == SDNPDATA_XML_TCPOTHER)
    {
      _currentValueOther(pSDNPSession, "tcpConnectionEstablishment", pNetworkConfig->tcpConnectionEstablishment.other, curLen, bGetLen);
    }
    else
    {
      _currentValueString(pSDNPSession, "tcpConnectionEstablishment", _tcpConnectionEstabString(pNetworkConfig->tcpConnectionEstablishment.type), curLen, bGetLen);
    }


    /* ipAddressOfRemoteDevice */
    _appendString(pSDNPSession, "<ipAddressOfRemoteDevice>\n", curLen, bGetLen);
    pSDNPSession->xml2CurIndent++;
    _appendString(pSDNPSession, "<currentValue>\n", curLen, bGetLen);
    pSDNPSession->xml2CurIndent++;
    for(i=0; i<SDNPXML_MAX_IPADDRESSES;i++)
    {
      _valueString(pSDNPSession, "address", pNetworkConfig->ipAddressOfRemoteDevice[i], curLen, bGetLen);
    }
    pSDNPSession->xml2CurIndent--;
    _appendString(pSDNPSession, "</currentValue>\n", curLen, bGetLen);
    pSDNPSession->xml2CurIndent--;
    _appendString(pSDNPSession, "</ipAddressOfRemoteDevice>\n", curLen, bGetLen);

    /* tcpListenPort */
    if(pNetworkConfig->tcpListenPort.notApplicable)
    {
      _currentValueString(pSDNPSession, "tcpListenPort", "<notApplicable />", curLen, bGetLen);
    }
    else
    {
      _currentValueDecimalString(pSDNPSession, "tcpListenPort", pNetworkConfig->tcpListenPort.value, curLen, bGetLen);
    }

    /* tcpPortOfRemoteDevice */
    if(pNetworkConfig->tcpPortOfRemoteDevice.notApplicable)
    {
      _currentValueString(pSDNPSession, "tcpPortOfRemoteDevice", "<notApplicable />", curLen, bGetLen);
    }
    else
    {
      _currentValueDecimalString(pSDNPSession, "tcpPortOfRemoteDevice", pNetworkConfig->tcpPortOfRemoteDevice.value, curLen, bGetLen);
    }

    /* tcpKeepAliveTimer */
    _currentValueDecimalString(pSDNPSession, "tcpKeepAliveTimer", pSDNPSession->dnp.linkStatusPeriod, curLen, bGetLen);


    /* localUDPPort */
    if(pNetworkConfig->localUDPPort.letSystemChoose)
    {
      _currentValueString(pSDNPSession, "localUDPPort",  "<letSystemChoose />", curLen, bGetLen);
    }
    else
    {
      _currentValueDecimalString(pSDNPSession, "localUDPPort", pNetworkConfig->localUDPPort.value, curLen, bGetLen);
    }

    /* destinationUDPPort (Masters Only) */
    /*_currentValueDecimalString(pSDNPSession, "destinationUDPPort", pNetworkConfig->destinationUDPPort, curLen, bGetLen);   */

    /* udpPortForUnsolicitedNullResponses */
    if(pNetworkConfig->udpPortForUnsolicitedNullResponses.none)
    {
      _currentValueString(pSDNPSession, "udpPortForUnsolicitedNullResponses",  "<none />", curLen, bGetLen);
    }
    else
    {
      _currentValueDecimalString(pSDNPSession, "udpPortForUnsolicitedNullResponses", pNetworkConfig->udpPortForUnsolicitedNullResponses.value, curLen, bGetLen);
    }

    /* udpPortForResponses */
    if(pNetworkConfig->udpPortForResponses.type == SDNPDATA_XML_UDPNONE)
    {
      _currentValueString(pSDNPSession, "udpPortForResponses",  "<none />", curLen, bGetLen);
    }
    else if(pNetworkConfig->udpPortForResponses.type == SDNPDATA_XML_UDPUSESOURCE)
    {
      _currentValueString(pSDNPSession, "udpPortForResponses",  "<useSourcePortNumber />", curLen, bGetLen);
    }
    else if(pNetworkConfig->udpPortForResponses.type == SDNPDATA_XML_UDPVALUE)
    {
      _currentValueDecimalString(pSDNPSession, "udpPortForResponses", pNetworkConfig->udpPortForResponses.value, curLen, bGetLen);
    }

    /* multipleOutstationConnections is a master only value */

    /* multipleMasterConnections */
    if(pNetworkConfig->multipleMasterConnections.notSupported)
    {
      _currentValueString(pSDNPSession, "multipleMasterConnections",  "<notSupported />", curLen, bGetLen);
    }
    else
    {
      _appendString(pSDNPSession, "<multipleMasterConnections>\n", curLen, bGetLen);
      pSDNPSession->xml2CurIndent++;
      _appendString(pSDNPSession, "<currentValue>\n", curLen, bGetLen);
      pSDNPSession->xml2CurIndent++;
      _emptyElementString(pSDNPSession, curLen, bGetLen, "basedOnIPAddress", pNetworkConfig->multipleMasterConnections.basedOnIPAddress);
      _emptyElementString(pSDNPSession, curLen, bGetLen, "basedOnIPPortNumber", pNetworkConfig->multipleMasterConnections.basedOnIPPortNumber);
      _emptyElementString(pSDNPSession, curLen, bGetLen, "browsingForStaticData", pNetworkConfig->multipleMasterConnections.browsingForStaticData);

      pSDNPSession->xml2CurIndent--;
      _appendString(pSDNPSession, "</currentValue>\n", curLen, bGetLen);
      pSDNPSession->xml2CurIndent--;
      _appendString(pSDNPSession, "</multipleMasterConnections>\n", curLen, bGetLen);
    }


    /* timeSynchronization */
    if(pNetworkConfig->timeSynchronization.type == SDNPDATA_XML_TIMESYNCOTHER)
    {
      _currentValueOther(pSDNPSession, "timeSynchronization", pNetworkConfig->timeSynchronization.other, curLen, bGetLen);
    }
    else
    {
      _currentValueString(pSDNPSession, "timeSynchronization", _timeSynchronizationString(pNetworkConfig->timeSynchronization.type), curLen, bGetLen);
    }
  }

  pSDNPSession->xml2CurIndent--;
  _appendString(pSDNPSession, "</networkConfig>\n", curLen, bGetLen);

  sdnpmem_free(pNetworkConfig);
}

/* referenceDevice/configuration/linkConfig */
static void TMWDEFS_LOCAL _buildLinkConfig(SDNPSESN *pSDNPSession, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  DNPLINK_CONTEXT *pLinkContext = (DNPLINK_CONTEXT*)pSDNPSession->dnp.tmw.pChannel->pLinkContext;

  _appendString(pSDNPSession, "<linkConfig>\n", curLen, bGetLen);
  pSDNPSession->xml2CurIndent++;

  /*
    dataLinkAddress
    sourceAddressValidation
    expectedSourceAddress
    selfAddressSupport
    sendsConfirmedUserDataFrames
    linkLayerConfirmTimeout
    maxDataLinkRetries
    maxTransmittedFrameSize
    maxReceivedFrameSize
  */

  _currentValueDecimalString(pSDNPSession, "dataLinkAddress", pSDNPSession->dnp.tmw.srcAddress, curLen, bGetLen);

  if(pSDNPSession->dnp.validateSourceAddress)
    _currentValueString(pSDNPSession, "sourceAddressValidation", "<alwaysSingleAddress />", curLen, bGetLen);
  else
    _currentValueString(pSDNPSession, "sourceAddressValidation", "<never />", curLen, bGetLen);

  _currentValueDecimalString(pSDNPSession, "expectedSourceAddress", pSDNPSession->dnp.tmw.destAddress, curLen, bGetLen);

  _currentValueString(pSDNPSession, "selfAddressSupport", _yesNoString(pSDNPSession->dnp.enableSelfAddress), curLen, bGetLen);

  if(pLinkContext->confirmMode == TMWDEFS_LINKCNFM_SOMETIMES)
  {
    _appendString(pSDNPSession, "<sendsConfirmedUserDataFrames>\n", curLen, bGetLen);
    _currentValueCustom(pSDNPSession, "sometimes", "Only for multiframe message fragment", curLen, bGetLen);
    _appendString(pSDNPSession, "</sendsConfirmedUserDataFrames>\n", curLen, bGetLen);
  }
  else
  _currentValueString(pSDNPSession, "sendsConfirmedUserDataFrames", _linkConfirmModeString(pLinkContext->confirmMode), curLen, bGetLen);

  _currentValueDecimalString(pSDNPSession, "linkLayerConfirmTimeout", pLinkContext->confirmTimeout, curLen, bGetLen);

  _currentValueDecimalString(pSDNPSession, "maxDataLinkRetries", pLinkContext->maxRetries, curLen, bGetLen);

  _currentValueDecimalString(pSDNPSession, "maxTransmittedFrameSize", pLinkContext->txFrameSize, curLen, bGetLen);

  _currentValueDecimalString(pSDNPSession, "maxReceivedFrameSize", pLinkContext->rxFrameSize, curLen, bGetLen);

  pSDNPSession->xml2CurIndent--;
  _appendString(pSDNPSession, "</linkConfig>\n", curLen, bGetLen);
}

/* referenceDevice/configuration/applConfig */
static void TMWDEFS_LOCAL _buildApplConfig(SDNPSESN *pSDNPSession, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  DNPCHNL *pDNPChannel = (DNPCHNL*)pSDNPSession->dnp.tmw.pChannel;

  _appendString(pSDNPSession, "<applConfig>\n", curLen, bGetLen);
  pSDNPSession->xml2CurIndent++;

  /*
    maxTransmittedFragmentSize
    maxFileTransferTransmittedFragmentSize
    maxReceivedFragmentSize
    fragmentTimeout
    maxObjectsInCROBControlRequest
    maxObjectsInAnalogOutputControlRequest
    maxObjectsInDataSetsControlRequest
    supportsMixedObjectGroupsInControlRequest
    controlStatusCodesSupported (2.09)
  */

  _currentValueDecimalString(pSDNPSession, "maxTransmittedFragmentSize", pDNPChannel->txFragmentSize, curLen, bGetLen);

  _currentValueDecimalString(pSDNPSession, "maxFileTransferTransmittedFragmentSize", pDNPChannel->txFragmentSize, curLen, bGetLen);

  _currentValueDecimalString(pSDNPSession, "maxReceivedFragmentSize", pDNPChannel->rxFragmentSize, curLen, bGetLen);

  /* No fragmentTimeout, there is a frame timeout */

  _currentValueDecimalString(pSDNPSession, "maxObjectsInCROBControlRequest", pSDNPSession->maxControlRequests, curLen, bGetLen);

  _currentValueDecimalString(pSDNPSession, "maxObjectsInAnalogOutputControlRequest", pSDNPSession->maxControlRequests, curLen, bGetLen);

  _currentValueDecimalString(pSDNPSession, "maxObjectsInDataSetsControlRequest", DNPCNFG_MAX_DATASET_CTRLS, curLen, bGetLen);

  _currentValueString(pSDNPSession, "supportsMixedObjectGroupsInControlRequest", _yesNoString(pSDNPSession->allowMultiCROBRequests), curLen, bGetLen);

  /* New in schema 2.09 controlStatusCodesSupported, has no currentValues section */

  pSDNPSession->xml2CurIndent--;
  _appendString(pSDNPSession, "</applConfig>\n", curLen, bGetLen);
}

/* referenceDevice/configuration/outstationConfig */
static void TMWDEFS_LOCAL _buildOutstationConfig(SDNPSESN *pSDNPSession, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  SDNPDATA_XML_OUTSTATIONCONFIG *pOutstationConfig;

  pOutstationConfig = (SDNPDATA_XML_OUTSTATIONCONFIG *)sdnpmem_alloc(SDNPMEM_SIM_XML_CONFIG_TYPE);
  if(pOutstationConfig == TMWDEFS_NULL)
    return;

  memset(pOutstationConfig, 0, sizeof(SDNPDATA_XML_OUTSTATIONCONFIG));

  _appendString(pSDNPSession, "<outstationConfig>\n", curLen, bGetLen);

  if(sdnpdata_XmlGetOutstationConfig(pSDNPSession->pDbHandle, pOutstationConfig))
  {
    pSDNPSession->xml2CurIndent++;
    /*
      applicationLayerConfirmTimeout
      timeSyncRequired
      deviceTroubleBit
      fileHandleTimeout
      eventBufferOverflowBehavior
      eventBufferOrganization  changed in schema version 2.09
      sendsMultiFragmentResponses
      requestsLastFragmentConfirmation
      settingsPreservedThroughDeviceRestart (was settingsPreservedThroughDeviceReset, changed to this in schema 2.08)
      configurationSignatureSupported (2.09)
      requestsApplicationConfirmation (2.09)
      supportsDNP3ClockManagement (2.11)
    */

    /* applicationLayerConfirmTimeout */
    _currentValueDecimalString(pSDNPSession, "applicationLayerConfirmTimeout", pSDNPSession->applConfirmTimeout, curLen, bGetLen);

    /* timeSyncRequired */
    _appendString(pSDNPSession, "<timeSyncRequired>\n", curLen, bGetLen);
    if(pOutstationConfig->timeSyncRequired.type == SDNPDATA_XML_TIMESYNCREQWITHIN)
    {
      _currentValueNameDecimalString(pSDNPSession, TMWDEFS_NULL, "withinSecondsOfIIN14", pOutstationConfig->timeSyncRequired.value, curLen, bGetLen);
    }
    else if(pOutstationConfig->timeSyncRequired.type == SDNPDATA_XML_TIMESYNCREQPERIOD)
    {
      _currentValueNameDecimalString(pSDNPSession, TMWDEFS_NULL, "periodically", pOutstationConfig->timeSyncRequired.value, curLen, bGetLen);
    }
    else
    {
      _currentValueString(pSDNPSession, TMWDEFS_NULL, "<never />", curLen, bGetLen);
    }
    pSDNPSession->xml2CurIndent--;
    _appendString(pSDNPSession, "</timeSyncRequired>\n", curLen, bGetLen);


    /* deviceTroubleBit */
    if(pOutstationConfig->deviceTroubleBit.type == SDNPDATA_XML_DEVICETROUBLENEVER)
    {
      _currentValueString(pSDNPSession, "deviceTroubleBit", "<neverUsed />", curLen, bGetLen);
    }
    else if(pOutstationConfig->deviceTroubleBit.type == SDNPDATA_XML_DEVICETROUBLEREASON)
    {
      _appendString(pSDNPSession, "<deviceTroubleBit>\n", curLen, bGetLen);
      _currentValueCustom(pSDNPSession, "reasonForSetting", pOutstationConfig->deviceTroubleBit.reasonForSetting, curLen, bGetLen);
      _appendString(pSDNPSession, "</deviceTroubleBit>\n", curLen, bGetLen);
    }


    /* fileHandleTimeout */
#if SDNPDATA_SUPPORT_OBJ70
    _currentValueDecimalString(pSDNPSession, "fileHandleTimeout", pSDNPSession->fileTransferTimeout, curLen, bGetLen);
#else
    _currentValueString(pSDNPSession, "fileHandleTimeout", "<notApplicable />\n", curLen, bGetLen);
#endif


    /* eventBufferOverflowBehavior */
    _appendString(pSDNPSession, "<eventBufferOverflowBehavior>\n", curLen, bGetLen);
    if(pOutstationConfig->eventBufferOverflowBehavior.type == SDNPDATA_XML_EVENT_OTHER)
    {
       _currentValueCustom(pSDNPSession, "other", pOutstationConfig->eventBufferOverflowBehavior.other, curLen, bGetLen);
    }
    else
    {
      _currentValueString(pSDNPSession, TMWDEFS_NULL, _eventOverflowBehaviorString(pOutstationConfig->eventBufferOverflowBehavior.type), curLen, bGetLen);
    }
    _appendString(pSDNPSession, "</eventBufferOverflowBehavior>\n", curLen, bGetLen);

    /* eventBufferOrganization */
    /* was _currentValueString(pSDNPSession, "eventBufferOrganization", pOutstationConfig->eventBufferOrganization, curLen, bGetLen);  */
    _currentValueString(pSDNPSession, "eventBufferOrganization", "<perObjectGroup />", curLen, bGetLen);

    /* sendsMultiFragmentResponses */
    _currentValueString(pSDNPSession, "sendsMultiFragmentResponses", _yesNoString(pSDNPSession->multiFragRespAllowed), curLen, bGetLen);

    /* requestsLastFragmentConfirmation */
    _appendString(pSDNPSession, "<requestsLastFragmentConfirmation>\n", curLen, bGetLen);
    _currentValueCustom(pSDNPSession, "sometimes", "When fragment contains events", curLen, bGetLen);
    _appendString(pSDNPSession, "</requestsLastFragmentConfirmation>\n", curLen, bGetLen);


    /* settingsPreservedThroughDeviceRestart */
    _appendString(pSDNPSession, "<settingsPreservedThroughDeviceRestart>\n", curLen, bGetLen);
    pSDNPSession->xml2CurIndent++;
    _appendString(pSDNPSession, "<currentValue>\n", curLen, bGetLen);
    pSDNPSession->xml2CurIndent++;
    _emptyElementString(pSDNPSession, curLen, bGetLen, "assignClass", pOutstationConfig->settingsPreservedThroughDeviceReset.assignClass);
    _emptyElementString(pSDNPSession, curLen, bGetLen, "analogDeadbands", pOutstationConfig->settingsPreservedThroughDeviceReset.analogDeadbands);
    _emptyElementString(pSDNPSession, curLen, bGetLen, "dataSetPrototypes", pOutstationConfig->settingsPreservedThroughDeviceReset.dataSetPrototypes);
    _emptyElementString(pSDNPSession, curLen, bGetLen, "dataSetDescriptors", pOutstationConfig->settingsPreservedThroughDeviceReset.dataSetDescriptors);
    _emptyElementString(pSDNPSession, curLen, bGetLen, "FC31ActivateConfiguration", pOutstationConfig->settingsPreservedThroughDeviceReset.FC31ActivateConfiguration);
    pSDNPSession->xml2CurIndent--;
    _appendString(pSDNPSession, "</currentValue>\n", curLen, bGetLen);
    pSDNPSession->xml2CurIndent--;
    _appendString(pSDNPSession, "</settingsPreservedThroughDeviceRestart>\n", curLen, bGetLen);

    /* configurationSignatureSupported */
    _currentValueString(pSDNPSession, "configurationSignatureSupported", "<notSupported />", curLen, bGetLen);

    /* requestsApplicationConfirmation */
    _appendString(pSDNPSession, "<requestsApplicationConfirmation>\n", curLen, bGetLen);
	  pSDNPSession->xml2CurIndent++;
	  _appendString(pSDNPSession, "<currentValue>\n", curLen, bGetLen);
	  pSDNPSession->xml2CurIndent++;

   _valueString(pSDNPSession, "eventResponses",  _yesNoString(TMWDEFS_TRUE), curLen, bGetLen);
   _valueString(pSDNPSession, "nonFinalFragments",  _yesNoString(pSDNPSession->multiFragConfirm), curLen, bGetLen);

   pSDNPSession->xml2CurIndent--;
   _appendString(pSDNPSession, "</currentValue>\n", curLen, bGetLen);
   pSDNPSession->xml2CurIndent--;
   _appendString(pSDNPSession, "</requestsApplicationConfirmation>\n", curLen, bGetLen);

   /* supportsDNP3ClockManagement */
   _currentValueString(pSDNPSession, "supportsDNP3ClockManagement", _yesNoString(TMWDEFS_TRUE), curLen, bGetLen);
  }

  pSDNPSession->xml2CurIndent--;
  _appendString(pSDNPSession, "</outstationConfig>\n", curLen, bGetLen);

  sdnpmem_free(pOutstationConfig);
}

/* referenceDevice/configuration/unsolicitedConfig */
static void TMWDEFS_LOCAL _buildUnsolicitedConfig(SDNPSESN *pSDNPSession, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  _appendString(pSDNPSession, "<unsolicitedConfig>\n", curLen, bGetLen);
  pSDNPSession->xml2CurIndent++;

  /*
    supportsUnsolicitedReporting
    masterDataLinkAddress
    unsolicitedResponseConfirmationTimeout"
    maxUnsolicitedRetries
  */

  _currentValueString(pSDNPSession, "supportsUnsolicitedReporting", (pSDNPSession->unsolAllowed ? "<on />" : "<off />"), curLen, bGetLen);
  _currentValueDecimalString(pSDNPSession, "masterDataLinkAddress", pSDNPSession->dnp.tmw.destAddress, curLen, bGetLen);
  _currentValueDecimalString(pSDNPSession, "unsolicitedResponseConfirmationTimeout", pSDNPSession->unsolConfirmTimeout, curLen, bGetLen);

  /* Determine whether to indicate infinite, or the value of max. unsolMaxRetries is number of retries before switching to offline retry value */
  if(pSDNPSession->unsolOfflineRetryDelay != TMWDEFS_DAYS(31))
    _currentValueString(pSDNPSession, "maxUnsolicitedRetries",  "<infinite />", curLen, bGetLen);
  else
    _currentValueDecimalString(pSDNPSession, "maxUnsolicitedRetries", pSDNPSession->unsolMaxRetries, curLen, bGetLen);

  pSDNPSession->xml2CurIndent--;
  _appendString(pSDNPSession, "</unsolicitedConfig>\n", curLen, bGetLen);

}

/* referenceDevice/configuration/unsolicitedResponseTriggerConditions */
static void TMWDEFS_LOCAL _buildUnsolTriggerConfig(SDNPSESN *pSDNPSession, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{

  _appendString(pSDNPSession, "<unsolicitedResponseTriggerConditions>\n", curLen, bGetLen);
  pSDNPSession->xml2CurIndent++;

  /*
    numberOfClassOneEvents
    numberOfClassTwoEvents
    numberOfClassThreeEvents
    totalNumberOfClassEvents
    holdTimeAfterClassOneEvent
    holdTimeAfterClassTwoEvent
    holdTimeAfterClassThreeEvent
    holdTimeAfterAnyEvent
    retriggerHoldTimer
    otherTriggerConditions
  */

  _currentValueDecimalString(pSDNPSession, "numberOfClassOneEvents", pSDNPSession->unsolMaxEvents[0], curLen, bGetLen);
  _currentValueDecimalString(pSDNPSession, "numberOfClassTwoEvents", pSDNPSession->unsolMaxEvents[1], curLen, bGetLen);
  _currentValueDecimalString(pSDNPSession, "numberOfClassThreeEvents", pSDNPSession->unsolMaxEvents[2], curLen, bGetLen);
  /* totalNumberOfClassEvents */
  _currentValueDecimalString(pSDNPSession, "holdTimeAfterClassOneEvent", pSDNPSession->unsolMaxDelay[0], curLen, bGetLen);
  _currentValueDecimalString(pSDNPSession, "holdTimeAfterClassTwoEvent", pSDNPSession->unsolMaxDelay[1], curLen, bGetLen);
  _currentValueDecimalString(pSDNPSession, "holdTimeAfterClassThreeEvent", pSDNPSession->unsolMaxDelay[2], curLen, bGetLen);
  /* holdTimeAfterAnyEvent */
  _currentValueString(pSDNPSession, "retriggerHoldTimer", "<notRetriggeredForEachNewEvent />", curLen, bGetLen);

  /* otherTriggerConditions */

  pSDNPSession->xml2CurIndent--;
  _appendString(pSDNPSession, "</unsolicitedResponseTriggerConditions>\n", curLen, bGetLen);
}

/* referenceDevice/configuration/outstationPerformance */
static void TMWDEFS_LOCAL _buildOutperformConfig(SDNPSESN *pSDNPSession, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  SDNPDATA_XML_OUTSTATIONPERFORM *pOutstationPerformConfig;

  pOutstationPerformConfig = (SDNPDATA_XML_OUTSTATIONPERFORM *)sdnpmem_alloc(SDNPMEM_SIM_XML_CONFIG_TYPE);
  if(pOutstationPerformConfig == TMWDEFS_NULL)
    return;

  memset(pOutstationPerformConfig, 0, sizeof(SDNPDATA_XML_OUTSTATIONPERFORM));

  _appendString(pSDNPSession, "<outstationPerformance>\n", curLen, bGetLen);

  if(sdnpdata_XmlGetOutstationPerform(pSDNPSession->pDbHandle, pOutstationPerformConfig))
  {
    pSDNPSession->xml2CurIndent++;
    /*
      maxTimeBaseDrift
      outstationSetsIIN14"
      referenceErrorViaDNP
      delayMeasurementError
      responseTime
      startupToIIN14
      binaryOrDoubleBitEventError
      nonBinaryOrDoubleBitEventError
    */

    _timingPerformanceType(pSDNPSession, "maxTimeBaseDrift", &pOutstationPerformConfig->maxTimeBaseDrift, curLen, bGetLen);

    if(pOutstationPerformConfig->outstationSetsIIN14.never)
    {
      _currentValueString(pSDNPSession, "outstationSetsIIN14",  "<never />", curLen, bGetLen);
    }
    else
    {
      TMWTYPES_CHAR buf[128];

      _appendString(pSDNPSession, "<outstationSetsIIN14>\n", curLen, bGetLen);
      pSDNPSession->xml2CurIndent++;
      _appendString(pSDNPSession, "<currentValue>\n", curLen, bGetLen);
      pSDNPSession->xml2CurIndent++;
      _emptyElementString(pSDNPSession, curLen, bGetLen, "atStartup", pOutstationPerformConfig->outstationSetsIIN14.atStartup);

      if(pOutstationPerformConfig->outstationSetsIIN14.periodically != 0)
      {
        tmwtarg_snprintf(buf, sizeof(buf), "<periodically>%d</periodically>\n", pOutstationPerformConfig->outstationSetsIIN14.periodically);
        _appendString(pSDNPSession, buf, curLen, bGetLen);
      }
      if(pOutstationPerformConfig->outstationSetsIIN14.afterLastTimeSync != 0)
      {
        tmwtarg_snprintf(buf, sizeof(buf), "<afterLastTimeSync>%d</afterLastTimeSync>\n", pOutstationPerformConfig->outstationSetsIIN14.afterLastTimeSync);
        _appendString(pSDNPSession, buf, curLen, bGetLen);
      }
      if(pOutstationPerformConfig->outstationSetsIIN14.whenTimeErrorExceeds != 0)
      {
        tmwtarg_snprintf(buf, sizeof(buf), "<whenTimeErrorExceeds>%d</whenTimeErrorExceeds>\n", pOutstationPerformConfig->outstationSetsIIN14.whenTimeErrorExceeds);
        _appendString(pSDNPSession, buf, curLen, bGetLen);
      }

      pSDNPSession->xml2CurIndent--;
      _appendString(pSDNPSession, "</currentValue>\n", curLen, bGetLen);
      pSDNPSession->xml2CurIndent--;
      _appendString(pSDNPSession, "</outstationSetsIIN14>\n", curLen, bGetLen);
    }
    _timingPerformanceType(pSDNPSession, "referenceErrorViaDNP", &pOutstationPerformConfig->referenceErrorViaDNP, curLen, bGetLen);
    _timingPerformanceType(pSDNPSession, "delayMeasurementError", &pOutstationPerformConfig->delayMeasurementError, curLen, bGetLen);
    _timingPerformanceType(pSDNPSession, "responseTime", &pOutstationPerformConfig->responseTime, curLen, bGetLen);
    _timingPerformanceType(pSDNPSession, "startupToIIN14", &pOutstationPerformConfig->startupToIIN14, curLen, bGetLen);
    _timingPerformanceType(pSDNPSession, "binaryOrDoubleBitEventError", &pOutstationPerformConfig->binaryOrDoubleBitEventError, curLen, bGetLen);
    _timingPerformanceType(pSDNPSession, "nonBinaryOrDoubleBitEventError", &pOutstationPerformConfig->nonBinaryOrDoubleBitEventError, curLen, bGetLen);
  }

  pSDNPSession->xml2CurIndent--;
  _appendString(pSDNPSession, "</outstationPerformance>\n", curLen, bGetLen);

  sdnpmem_free(pOutstationPerformConfig);
}

/* referenceDevice/configuration/fieldConfig */
static void TMWDEFS_LOCAL _buildFieldConfig(SDNPSESN *pSDNPSession, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  SDNPDATA_XML_FIELDCONFIG fieldConfig;

  _appendString(pSDNPSession, "<fieldConfig>\n", curLen, bGetLen);
  pSDNPSession->xml2CurIndent++;

  memset(&fieldConfig, 0, sizeof(fieldConfig));
  if(sdnpdata_XmlGetFieldConfig(pSDNPSession->pDbHandle, &fieldConfig))
  {
   /*
      outstationLocation
      outstationId
      outstationName
      deviceSerialNumber
    */

    _currentValueString(pSDNPSession, "outstationLocation", fieldConfig.outstationLocation, curLen, bGetLen);
    _currentValueString(pSDNPSession, "outstationId", fieldConfig.outstationId, curLen, bGetLen);
    _currentValueString(pSDNPSession, "outstationName", fieldConfig.outstationName, curLen, bGetLen);
    _currentValueString(pSDNPSession, "deviceSerialNumber", fieldConfig.deviceSerialNumber, curLen, bGetLen);

    /* NotSupported secondaryOperatorName primaryOperatorName systemName ownerName */
  }

  pSDNPSession->xml2CurIndent--;
  _appendString(pSDNPSession, "</fieldConfig>\n", curLen, bGetLen);
}

/* referenceDevice/configuration/securityConfig */
static void TMWDEFS_LOCAL _buildSecurityConfig(SDNPSESN *pSDNPSession, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  SDNPDATA_XML_SECURITYCONFIG *pSecurityConfig;
#if SDNPDATA_SUPPORT_OBJ120
  SDNPAUTH_INFO *pAuthInfo = TMWDEFS_NULL;
  int i; 
  TMWTYPES_ULONG maxErrorCount = 0;
  TMWTYPES_ULONG maxUsers = DNPCNFG_AUTH_MAX_NUMBER_USERS;
  TMWTYPES_ULONG version = 5;
#endif
  
  pSecurityConfig = (SDNPDATA_XML_SECURITYCONFIG *)sdnpmem_alloc(SDNPMEM_SIM_XML_CONFIG_TYPE);
  if(pSecurityConfig == TMWDEFS_NULL)
    return;

  memset(pSecurityConfig, 0, sizeof(SDNPDATA_XML_SECURITYCONFIG));

  _appendString(pSDNPSession, "<securityConfig>\n", curLen, bGetLen);
  pSDNPSession->xml2CurIndent++;

  /*
  secureAuthenticationSupported
  maxNumberUsers
  securityResponseTimeout
  acceptsAggressiveMode
  issuesAggressiveMode
  sessionKeyChangeInterval
  sessionKeyChangeMessageCount
  maxErrorCount
  macAlgorithmRequested
  keyWrapAlgorithm
  TLSCipherSuites
  changeCipherRequestTimeout
  numberCASupported
  certificateRevocationCheckTime
  additionalCriticalFCs
  otherCriticalFragments
  remoteUpdateKeyChangeSupported
  permitUserCredentialExpiry (2.09)
  secureAuthenticationEnabled (2.11)
  challengeDataLength (2.11)
  maxStatisticCounts (2.11)
  */

#if SDNPDATA_SUPPORT_OBJ120
    pAuthInfo = (SDNPAUTH_INFO*)pSDNPSession->pAuthenticationInfo;
    /* version */
#if SDNPCNFG_SUPPORT_SA_VERSION5
    if (pAuthInfo != TMWDEFS_NULL)
      maxErrorCount = pAuthInfo->maxErrorMessagesSent;
#endif

#if SDNPCNFG_SUPPORT_SA_VERSION2
    if (pSDNPSession->dnp.operateInV2Mode)
    {
      version = 2;
      maxUsers = DNPCNFG_AUTHV2_MAX_NUMBER_USERS;
      if (pAuthInfo != TMWDEFS_NULL)
        maxErrorCount = pAuthInfo->maxErrorCount;
    }
#endif
    _currentValueNameDecimalString(pSDNPSession, "secureAuthenticationSupported", "version", version, curLen, bGetLen);
    _currentValueNameDecimalString(pSDNPSession, "maxNumberUsers", "maxUsers", maxUsers, curLen, bGetLen);
    if (pAuthInfo != TMWDEFS_NULL)
    {
      _currentValueDecimalString(pSDNPSession, "securityResponseTimeout", pAuthInfo->replyTimeout, curLen, bGetLen);
      _currentValueString(pSDNPSession, "acceptsAggressiveMode", _yesNoString(pAuthInfo->aggressiveModeSupport), curLen, bGetLen);
      _currentValueString(pSDNPSession, "issuesAggressiveMode", _yesNoString(pAuthInfo->aggressiveModeSupport), curLen, bGetLen);
      _currentValueDecimalString(pSDNPSession, "sessionKeyChangeInterval", (pAuthInfo->keyChangeInterval / 1000), curLen, bGetLen);
      _currentValueDecimalString(pSDNPSession, "sessionKeyChangeMessageCount", pAuthInfo->maxKeyChangeCount, curLen, bGetLen);

      _currentValueDecimalString(pSDNPSession, "maxErrorCount", maxErrorCount, curLen, bGetLen);

      _currentValueString(pSDNPSession, "macAlgorithmRequested", _toMACString(pAuthInfo->MACAlgorithm), curLen, bGetLen);
    }
#if SDNPCNFG_SUPPORT_SA_VERSION2
    if(pSDNPSession->dnp.operateInV2Mode)
      _currentValueString(pSDNPSession, "keyWrapAlgorithm",  _toKeyWrapString(DNPAUTH_KEYWRAP_AES128), curLen, bGetLen);
    else
      _currentValueString(pSDNPSession, "keyWrapAlgorithm",  _toKeyWrapString((TMWTYPES_UCHAR)SDNPXML_KEYWRAP_PER_USER), curLen, bGetLen);
#else
    _currentValueString(pSDNPSession, "keyWrapAlgorithm",  _toKeyWrapString((TMWTYPES_UCHAR)SDNPXML_KEYWRAP_PER_USER), curLen, bGetLen);
#endif
#endif
  if(sdnpdata_XmlGetSecurityConfig(pSDNPSession->pDbHandle, pSecurityConfig))
  {

    /* TLSCipherSuites */
    if(pSecurityConfig->TLSCipherSuites.type == SDNPDATA_XML_TLSOTHER)
    {
      _currentValueOther(pSDNPSession, "TLSCipherSuites", pSecurityConfig->TLSCipherSuites.other, curLen, bGetLen);
    }
    else
    {
      _currentValueString(pSDNPSession, "TLSCipherSuites", _tlsCipherString(pSecurityConfig->TLSCipherSuites.type), curLen, bGetLen);
    }

    /* changeCipherRequestTimeout */
    if(pSecurityConfig->changeCipherRequestTimeout.type == SDNPDATA_XML_SECURITYVALUENA)
    {
      /* "notApplicable" */
      _currentValueString(pSDNPSession, "changeCipherRequestTimeout", "<notApplicable />", curLen, bGetLen);
    }
    else if(pSecurityConfig->changeCipherRequestTimeout.type == SDNPDATA_XML_SECURITYVALUEVALUE)
    {
      /* value */
      _currentValueDecimalString(pSDNPSession, "changeCipherRequestTimeout", pSecurityConfig->changeCipherRequestTimeout.value.value, curLen, bGetLen);
    }
    else if(pSecurityConfig->changeCipherRequestTimeout.type == SDNPDATA_XML_SECURITYVALUEOTHER)
    {
      /* other */
      _currentValueOther(pSDNPSession, "changeCipherRequestTimeout", pSecurityConfig->changeCipherRequestTimeout.value.other, curLen, bGetLen);
    }

    /* numberCASupported */
    _currentValueDecimalString(pSDNPSession, "numberCASupported", pSecurityConfig->numberCASupported, curLen, bGetLen);


    /* certificateRevocationCheckTime */
    if(pSecurityConfig->certificateRevocationCheckTime.type == SDNPDATA_XML_SECURITYVALUENA)
    {
      /* "notApplicable" */
      _currentValueString(pSDNPSession, "certificateRevocationCheckTime", "<notApplicable />", curLen, bGetLen);
    }
    else if(pSecurityConfig->certificateRevocationCheckTime.type == SDNPDATA_XML_SECURITYVALUEVALUE)
    {
      /* value */
      _currentValueDecimalString(pSDNPSession, "certificateRevocationCheckTime", pSecurityConfig->certificateRevocationCheckTime.value.value, curLen, bGetLen);
    }
    else if(pSecurityConfig->certificateRevocationCheckTime.type == SDNPDATA_XML_SECURITYVALUEOTHER)
    {
      /* other */
      _currentValueOther(pSDNPSession, "certificateRevocationCheckTime", pSecurityConfig->certificateRevocationCheckTime.value.other, curLen, bGetLen);
    }

#if SDNPDATA_SUPPORT_OBJ120
    /* additionalCriticalFCs*/
    _appendString(pSDNPSession, "<additionalCriticalFCs>\n", curLen, bGetLen);
    pSDNPSession->xml2CurIndent++;
    _appendString(pSDNPSession, "<currentValue>\n", curLen, bGetLen);
    pSDNPSession->xml2CurIndent++;
    _emptyElementString(pSDNPSession, curLen, bGetLen, "FC0", pSecurityConfig->additionalCriticalFCs.FC0);
    _emptyElementString(pSDNPSession, curLen, bGetLen, "FC1", pSecurityConfig->additionalCriticalFCs.FC1);
    _emptyElementString(pSDNPSession, curLen, bGetLen, "FC7", pSecurityConfig->additionalCriticalFCs.FC7);
    _emptyElementString(pSDNPSession, curLen, bGetLen, "FC8", pSecurityConfig->additionalCriticalFCs.FC8);
    _emptyElementString(pSDNPSession, curLen, bGetLen, "FC9", pSecurityConfig->additionalCriticalFCs.FC9);
    _emptyElementString(pSDNPSession, curLen, bGetLen, "FC10", pSecurityConfig->additionalCriticalFCs.FC10);
    _emptyElementString(pSDNPSession, curLen, bGetLen, "FC11", pSecurityConfig->additionalCriticalFCs.FC11);
    _emptyElementString(pSDNPSession, curLen, bGetLen, "FC12", pSecurityConfig->additionalCriticalFCs.FC12);
    _emptyElementString(pSDNPSession, curLen, bGetLen, "FC22", pSecurityConfig->additionalCriticalFCs.FC22);
    _emptyElementString(pSDNPSession, curLen, bGetLen, "FC23", pSecurityConfig->additionalCriticalFCs.FC23);
    _emptyElementString(pSDNPSession, curLen, bGetLen, "FC25", pSecurityConfig->additionalCriticalFCs.FC25);
    _emptyElementString(pSDNPSession, curLen, bGetLen, "FC26", pSecurityConfig->additionalCriticalFCs.FC26);
    _emptyElementString(pSDNPSession, curLen, bGetLen, "FC27", pSecurityConfig->additionalCriticalFCs.FC27);
    _emptyElementString(pSDNPSession, curLen, bGetLen, "FC28", pSecurityConfig->additionalCriticalFCs.FC28);
    _emptyElementString(pSDNPSession, curLen, bGetLen, "FC30", pSecurityConfig->additionalCriticalFCs.FC30);
    _emptyElementString(pSDNPSession, curLen, bGetLen, "FC129", pSecurityConfig->additionalCriticalFCs.FC129);
    _emptyElementString(pSDNPSession, curLen, bGetLen, "FC130", pSecurityConfig->additionalCriticalFCs.FC130);

    pSDNPSession->xml2CurIndent--;
    _appendString(pSDNPSession, "</currentValue>\n", curLen, bGetLen);
    pSDNPSession->xml2CurIndent--;
    _appendString(pSDNPSession, "</additionalCriticalFCs>\n", curLen, bGetLen);


    /* otherCriticalFragments */
    _appendString(pSDNPSession, "<otherCriticalFragments>\n", curLen, bGetLen);
    pSDNPSession->xml2CurIndent++;
    _appendString(pSDNPSession, "<currentValue>\n", curLen, bGetLen);
    pSDNPSession->xml2CurIndent++;
    for(i=0; i<SDNPXML_MAX_CRITICALFRAGMENTS; i++)
    {
      _valueString(pSDNPSession, "note", pSecurityConfig->otherCriticalFragments[i].note, curLen, bGetLen);
    }
    for(i=0; i<SDNPXML_MAX_CRITICALFRAGMENTS; i++)
    {
      _valueString(pSDNPSession, "criticalFragments", pSecurityConfig->otherCriticalFragments[i].criticalFragment, curLen, bGetLen);
    }
    pSDNPSession->xml2CurIndent--;
    _appendString(pSDNPSession, "</currentValue>\n", curLen, bGetLen);
    pSDNPSession->xml2CurIndent--;
    _appendString(pSDNPSession, "</otherCriticalFragments>\n", curLen, bGetLen);

    /* remoteUpdateKeyChangeSupported*/
    _appendString(pSDNPSession, "<remoteUpdateKeyChangeSupported>\n", curLen, bGetLen);
    pSDNPSession->xml2CurIndent++;
    _appendString(pSDNPSession, "<currentValue>\n", curLen, bGetLen);
    pSDNPSession->xml2CurIndent++;

    /* bySymmetricCryptography changed in 2.09 */
    _valueString(pSDNPSession, "bySymmetricCryptography",  _toSymMethodString(pSecurityConfig->remoteUpdateKeyChangeSupported.symmetricCrypto), curLen, bGetLen);

    /* byAsymmetricCryptography changed in 2.09 */
    _valueString(pSDNPSession, "byAsymmetricCryptography",  _toAsymMethodString(pSecurityConfig->remoteUpdateKeyChangeSupported.asymmetricCrypto), curLen, bGetLen);

    pSDNPSession->xml2CurIndent--;
    _appendString(pSDNPSession, "</currentValue>\n", curLen, bGetLen);
    pSDNPSession->xml2CurIndent--;
    _appendString(pSDNPSession, "</remoteUpdateKeyChangeSupported>\n", curLen, bGetLen);

    /* permitUserCredentialExpiry new in  2.09 */
    _currentValueString(pSDNPSession, "permitUserCredentialExpiry", _yesNoString(pSecurityConfig->permitUserCredentialExpiry), curLen, bGetLen);

    /* secureAuthenticationEnabled new in  2.11 */
    _currentValueString(pSDNPSession, "secureAuthenticationEnabled", (pSDNPSession->authenticationEnabled ? "<on />" : "<off />"), curLen, bGetLen);

#if SDNPCNFG_SUPPORT_SA_VERSION5
    if (pAuthInfo != TMWDEFS_NULL)
    {
      /* challengeDataLength new in  2.11 */
      _currentValueDecimalString(pSDNPSession, "challengeDataLength", pAuthInfo->randomChallengeDataLength, curLen, bGetLen);

      /* maxStatisticCounts new in  2.11 */;
      _appendString(pSDNPSession, "<maxStatisticCounts>\n", curLen, bGetLen);

      pSDNPSession->xml2CurIndent++;
      _currentValueDecimalString(pSDNPSession, "maxAuthenticationFailures", pAuthInfo->maxAuthenticationFailures, curLen, bGetLen);
      _currentValueDecimalString(pSDNPSession, "maxReplyTimeouts", pAuthInfo->maxReplyTimeouts, curLen, bGetLen);
      _currentValueDecimalString(pSDNPSession, "maxAuthenticationRekeys", pAuthInfo->maxAuthenticationRekeys, curLen, bGetLen);
      _currentValueDecimalString(pSDNPSession, "maxErrorMessagesSent", pAuthInfo->maxErrorMessagesSent, curLen, bGetLen);
      /* maxKeysDueToRestarts is on master only */

      pSDNPSession->xml2CurIndent--;

      _appendString(pSDNPSession, "</maxStatisticCounts>\n", curLen, bGetLen);
    }

#else
    /* challengeDataLength new in  2.11 */
    _currentValueDecimalString(pSDNPSession, "challengeDataLength", 4, curLen, bGetLen);
#endif

#endif
  }

  pSDNPSession->xml2CurIndent--;
  _appendString(pSDNPSession, "</securityConfig>\n", curLen, bGetLen);

  sdnpmem_free(pSecurityConfig);
}

/* referenceDevice/configuration/broadcastConfig */
static void TMWDEFS_LOCAL _buildBroadcastConfig(SDNPSESN *pSDNPSession, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  TMWTYPES_CHAR *pEnabledString;

  _appendString(pSDNPSession, "<broadcastConfig>\n", curLen, bGetLen);
  pSDNPSession->xml2CurIndent++;

  _currentValueString(pSDNPSession, "broadcastFunctionalitySupported", _enabledString(pSDNPSession->enabledBroadcastFCs != 0), curLen, bGetLen);

  _appendString(pSDNPSession, "<broadcastFunctionalityFC2Supported>\n", curLen, bGetLen);
  pSDNPSession->xml2CurIndent++;

  pEnabledString = _enabledString((pSDNPSession->enabledBroadcastFCs & SDNPSESN_ENABLE_FC_WRITE)
      && (pSDNPSession->enabledBroadcastWrites & SDNPSESN_ENABLE_WRITE_CLOCK));
  _currentValueString(pSDNPSession, "writeClock", pEnabledString, curLen, bGetLen);

  pEnabledString = _enabledString((pSDNPSession->enabledBroadcastFCs & SDNPSESN_ENABLE_FC_WRITE)
      && (pSDNPSession->enabledBroadcastWrites & SDNPSESN_ENABLE_WRITE_LRTIME));
  _currentValueString(pSDNPSession, "writeLastRecordedTime", pEnabledString, curLen, bGetLen);

  pEnabledString = _enabledString((pSDNPSession->enabledBroadcastFCs & SDNPSESN_ENABLE_FC_WRITE)
      && (pSDNPSession->enabledBroadcastWrites & SDNPSESN_ENABLE_WRITE_CRESTART));
  _currentValueString(pSDNPSession, "clearRestart", pEnabledString, curLen, bGetLen);

  pEnabledString = _enabledString((pSDNPSession->enabledBroadcastFCs & SDNPSESN_ENABLE_FC_WRITE)
      && (pSDNPSession->enabledBroadcastWrites & SDNPSESN_ENABLE_WRITE_OTHER));
  _currentValueString(pSDNPSession, "writeAnyOther", pEnabledString, curLen, bGetLen);

  pSDNPSession->xml2CurIndent--;
  _appendString(pSDNPSession, "</broadcastFunctionalityFC2Supported>\n", curLen, bGetLen);

  _currentValueString(pSDNPSession, "broadcastFunctionalityFC5Supported",  _enabledString((pSDNPSession->enabledBroadcastFCs & SDNPSESN_ENABLE_FC_DIRECT_OP) != 0), curLen, bGetLen);
  _currentValueString(pSDNPSession, "broadcastFunctionalityFC6Supported",  _enabledString((pSDNPSession->enabledBroadcastFCs & SDNPSESN_ENABLE_FC_DIRECT_OP_NOACK) != 0), curLen, bGetLen);
  _currentValueString(pSDNPSession, "broadcastFunctionalityFC7Supported",  _enabledString((pSDNPSession->enabledBroadcastFCs & SDNPSESN_ENABLE_FC_FRZ) != 0), curLen, bGetLen);
  _currentValueString(pSDNPSession, "broadcastFunctionalityFC8Supported",  _enabledString((pSDNPSession->enabledBroadcastFCs & SDNPSESN_ENABLE_FC_FRZ_NOACK) != 0), curLen, bGetLen);
  _currentValueString(pSDNPSession, "broadcastFunctionalityFC9Supported",  _enabledString((pSDNPSession->enabledBroadcastFCs & SDNPSESN_ENABLE_FC_FRZ_CLEAR) != 0), curLen, bGetLen);
  _currentValueString(pSDNPSession, "broadcastFunctionalityFC10Supported", _enabledString((pSDNPSession->enabledBroadcastFCs & SDNPSESN_ENABLE_FC_FRZ_CLEAR_NOACK) != 0), curLen, bGetLen);
  _currentValueString(pSDNPSession, "broadcastFunctionalityFC11Supported", _enabledString((pSDNPSession->enabledBroadcastFCs & SDNPSESN_ENABLE_FC_FRZ_TIME) != 0), curLen, bGetLen);
  _currentValueString(pSDNPSession, "broadcastFunctionalityFC12Supported", _enabledString((pSDNPSession->enabledBroadcastFCs & SDNPSESN_ENABLE_FC_FRZ_TIME_NOACK) != 0), curLen, bGetLen);
  _currentValueString(pSDNPSession, "broadcastFunctionalityFC13Supported", _enabledString((pSDNPSession->enabledBroadcastFCs & SDNPSESN_ENABLE_FC_COLD_RESTART) != 0), curLen, bGetLen);
  _currentValueString(pSDNPSession, "broadcastFunctionalityFC14Supported", _enabledString((pSDNPSession->enabledBroadcastFCs & SDNPSESN_ENABLE_FC_WARM_RESTART) != 0), curLen, bGetLen);
  _currentValueString(pSDNPSession, "broadcastFunctionalityFC15Supported", _enabledString((pSDNPSession->enabledBroadcastFCs & SDNPSESN_ENABLE_FC_INIT_DATA) != 0), curLen, bGetLen);
  _currentValueString(pSDNPSession, "broadcastFunctionalityFC16Supported", _enabledString((pSDNPSession->enabledBroadcastFCs & SDNPSESN_ENABLE_FC_INIT_APP) != 0), curLen, bGetLen);
  _currentValueString(pSDNPSession, "broadcastFunctionalityFC17Supported", _enabledString((pSDNPSession->enabledBroadcastFCs & SDNPSESN_ENABLE_FC_START_APP) != 0), curLen, bGetLen);
  _currentValueString(pSDNPSession, "broadcastFunctionalityFC18Supported", _enabledString((pSDNPSession->enabledBroadcastFCs & SDNPSESN_ENABLE_FC_STOP_APP) != 0), curLen, bGetLen);
  _currentValueString(pSDNPSession, "broadcastFunctionalityFC19Supported", _enabledString((pSDNPSession->enabledBroadcastFCs & SDNPSESN_ENABLE_FC_SAVE_CONFIG) != 0), curLen, bGetLen);

  pEnabledString = _enabledString((pSDNPSession->enabledBroadcastFCs & SDNPSESN_ENABLE_FC_ENABLE_UNSOL) != 0);
  _appendString(pSDNPSession, "<broadcastFunctionalityFC20Supported>\n", curLen, bGetLen);
  pSDNPSession->xml2CurIndent++;
  _currentValueString(pSDNPSession, "byEventClass", pEnabledString, curLen, bGetLen);
  _currentValueString(pSDNPSession, "byAnyOther", "<disabled />", curLen, bGetLen);
  pSDNPSession->xml2CurIndent--;
  _appendString(pSDNPSession, "</broadcastFunctionalityFC20Supported>\n", curLen, bGetLen);

  pEnabledString = _enabledString((pSDNPSession->enabledBroadcastFCs & SDNPSESN_ENABLE_FC_DISABLE_UNSOL) != 0);
  _appendString(pSDNPSession, "<broadcastFunctionalityFC21Supported>\n", curLen, bGetLen);
  pSDNPSession->xml2CurIndent++;
  _currentValueString(pSDNPSession, "byEventClass", pEnabledString, curLen, bGetLen);
  _currentValueString(pSDNPSession, "byAnyOther", "<disabled />", curLen, bGetLen);
  pSDNPSession->xml2CurIndent--;
  _appendString(pSDNPSession, "</broadcastFunctionalityFC21Supported>\n", curLen, bGetLen);

  _currentValueString(pSDNPSession, "broadcastFunctionalityFC22Supported", _enabledString((pSDNPSession->enabledBroadcastFCs & SDNPSESN_ENABLE_FC_ASSIGN_CLASS) != 0), curLen, bGetLen);
  _currentValueString(pSDNPSession, "broadcastFunctionalityFC24Supported", _enabledString((pSDNPSession->enabledBroadcastFCs & SDNPSESN_ENABLE_FC_RECORD_CTIME) != 0), curLen, bGetLen);
  _currentValueString(pSDNPSession, "broadcastFunctionalityFC31Supported", _enabledString((pSDNPSession->enabledBroadcastFCs & SDNPSESN_ENABLE_FC_ACTIVATE_CONFIG) != 0), curLen, bGetLen);

  pSDNPSession->xml2CurIndent--;
  _appendString(pSDNPSession, "</broadcastConfig>\n", curLen, bGetLen);
}

#if SDNPDATA_SUPPORT_OBJ1
static void _buildBinaryInputGroup(SDNPSESN *pSDNPSession, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  pSDNPSession->xml2CurIndent++;
  _appendString(pSDNPSession, "<binaryInputGroup>\n", curLen, bGetLen);
  pSDNPSession->xml2CurIndent++;
  _appendString(pSDNPSession, "<configuration>\n", curLen, bGetLen);
  pSDNPSession->xml2CurIndent++;

   /*
     defaultStaticVariation
     defaultEventVariation
     eventReportingMode
     class0ResponseMode
     eventBufferPerObjectGroup (2.09)
   */

  _currentValueString(pSDNPSession, "defaultStaticVariation",  _variationGroupString(pSDNPSession->obj01DefaultVariation), curLen, bGetLen);
#if SDNPDATA_SUPPORT_OBJ2
  _currentValueString(pSDNPSession, "defaultEventVariation",  _variationGroupString(pSDNPSession->obj02DefaultVariation), curLen, bGetLen);
  _currentValueString(pSDNPSession, "eventReportingMode",  _reportingModeString(pSDNPSession->binaryInputEventMode), curLen, bGetLen);
#endif

#if SDNPDATA_SUPPORT_CLASS0_POINT
  _currentValueString(pSDNPSession, "class0ResponseMode", "<basedOnPointIndex />", curLen, bGetLen);
#else
  _currentValueString(pSDNPSession, "class0ResponseMode", _class0ResponseValue(_class0Response(pSDNPSession, DNPDEFS_OBJ_1_BIN_INPUTS)), curLen, bGetLen);
#endif

#if SDNPDATA_SUPPORT_OBJ2
  /* New in schema 2.09 */
  _currentValueNameDecimalString(pSDNPSession, "eventBufferPerObjectGroup", "numEvents", pSDNPSession->binaryInputMaxEvents, curLen, bGetLen);
#endif

  pSDNPSession->xml2CurIndent--;
  _appendString(pSDNPSession, "</configuration>\n", curLen, bGetLen);

  pSDNPSession->xml2CurIndent--;
  _appendString(pSDNPSession, "</binaryInputGroup>\n", curLen, bGetLen);
  pSDNPSession->xml2CurIndent--;
}

static void _buildBinaryInputPoints(SDNPSESN *pSDNPSession, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  TMWTYPES_USHORT numPoints;

  pSDNPSession->xml2CurIndent++;
  _appendString(pSDNPSession, "<binaryInputPoints>\n", curLen, bGetLen);
  pSDNPSession->xml2CurIndent++;
  _appendString(pSDNPSession, "<configuration>\n", curLen, bGetLen);
  pSDNPSession->xml2CurIndent++;

  /* pointListDefinition */
  _pointListDefinition(pSDNPSession, DNPDEFS_OBJ_1_BIN_INPUTS, curLen, bGetLen);

  pSDNPSession->xml2CurIndent--;
  _appendString(pSDNPSession, "</configuration>\n", curLen, bGetLen);

  numPoints = sdnpdata_binInQuantity(pSDNPSession->pDbHandle);
  if(numPoints>0)
  {
    _appendString(pSDNPSession, "<dataPoints>\n", curLen, bGetLen);
  }
}

static void _buildBinaryInputPointsTail(SDNPSESN *pSDNPSession, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{

  if(sdnpdata_binInQuantity(pSDNPSession->pDbHandle)>0)
  {
    _appendString(pSDNPSession, "</dataPoints>\n", curLen, bGetLen);
  }

  pSDNPSession->xml2CurIndent--;
  _appendString(pSDNPSession, "</binaryInputPoints>\n", curLen, bGetLen);
  pSDNPSession->xml2CurIndent--;
}
#endif /* SDNPDATA_SUPPORT_OBJ1 */

#if SDNPDATA_SUPPORT_OBJ3
static void _buildDoubleBitInputGroup(SDNPSESN *pSDNPSession, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  pSDNPSession->xml2CurIndent++;
  _appendString(pSDNPSession, "<doubleBitInputGroup>\n", curLen, bGetLen);
  pSDNPSession->xml2CurIndent++;
  _appendString(pSDNPSession, "<configuration>\n", curLen, bGetLen);
  pSDNPSession->xml2CurIndent++;

   /* defaultStaticVariation
      defaultEventVariation
      eventReportingMode
      class0ResponseMode
      eventBufferPerObjectGroup (2.09)
    */

  _currentValueString(pSDNPSession, "defaultStaticVariation", _variationGroupString(pSDNPSession->obj03DefaultVariation), curLen, bGetLen);
#if SDNPDATA_SUPPORT_OBJ4
  _currentValueString(pSDNPSession, "defaultEventVariation", _variationGroupString(pSDNPSession->obj04DefaultVariation), curLen, bGetLen);
  _currentValueString(pSDNPSession, "eventReportingMode", _reportingModeString(pSDNPSession->doubleInputEventMode), curLen, bGetLen);
#endif

#if SDNPDATA_SUPPORT_CLASS0_POINT
  _currentValueString(pSDNPSession, "class0ResponseMode", "<basedOnPointIndex />", curLen, bGetLen);
#else
  _currentValueString(pSDNPSession, "class0ResponseMode", _class0ResponseValue(_class0Response(pSDNPSession, DNPDEFS_OBJ_3_DBL_INPUTS)), curLen, bGetLen);
#endif

#if SDNPDATA_SUPPORT_OBJ4
  /* New in schema 2.09 */
  _currentValueNameDecimalString(pSDNPSession, "eventBufferPerObjectGroup", "numEvents", pSDNPSession->doubleInputMaxEvents, curLen, bGetLen);
#endif

  pSDNPSession->xml2CurIndent--;
  _appendString(pSDNPSession, "</configuration>\n", curLen, bGetLen);

  pSDNPSession->xml2CurIndent--;
  _appendString(pSDNPSession, "</doubleBitInputGroup>\n", curLen, bGetLen);
  pSDNPSession->xml2CurIndent--;
}

static void _buildDoubleBitInputPoints(SDNPSESN *pSDNPSession, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  pSDNPSession->xml2CurIndent++;
  _appendString(pSDNPSession, "<doubleBitInputPoints>\n", curLen, bGetLen);
  pSDNPSession->xml2CurIndent++;
  _appendString(pSDNPSession, "<configuration>\n", curLen, bGetLen);
  pSDNPSession->xml2CurIndent++;

  /* pointListDefinition */
  _pointListDefinition(pSDNPSession, DNPDEFS_OBJ_3_DBL_INPUTS, curLen, bGetLen);

  pSDNPSession->xml2CurIndent--;
  _appendString(pSDNPSession, "</configuration>\n", curLen, bGetLen);

  if(sdnpdata_dblInQuantity(pSDNPSession->pDbHandle)>0)
  {
    _appendString(pSDNPSession, "<dataPoints>\n", curLen, bGetLen);
  }
}

static void _buildDoubleBitInputPointsTail(SDNPSESN *pSDNPSession, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  if(sdnpdata_dblInQuantity(pSDNPSession->pDbHandle)>0)
  {
    _appendString(pSDNPSession, "</dataPoints>\n", curLen, bGetLen);
  }

  pSDNPSession->xml2CurIndent--;
  _appendString(pSDNPSession, "</doubleBitInputPoints>\n", curLen, bGetLen);
  pSDNPSession->xml2CurIndent--;
}
#endif /* SDNPDATA_SUPPORT_OBJ3 */

#if SDNPDATA_SUPPORT_OBJ10
static void _buildBinaryOutputGroup(SDNPSESN *pSDNPSession, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  SDNPDATA_XML_BINOUTGROUPCONFIG binOutGroupConfig;

  pSDNPSession->xml2CurIndent++;
  _appendString(pSDNPSession, "<binaryOutputGroup>\n", curLen, bGetLen);
  pSDNPSession->xml2CurIndent++;
  _appendString(pSDNPSession, "<configuration>\n", curLen, bGetLen);
  pSDNPSession->xml2CurIndent++;

  /*
    minimumPulseTime
    maximumPulseTime
    class0ResponseMode
    outputCommandEventObjects
    defaultEventVariation
    defaultCommandEventVariation
    changeEventReportingMode
    commandEventReportingMode
    maxTimeBetweenSelectAndOperate
    eventBufferPerObjectGroup (2.09)
    commandEventBufferPerObjectGroup (2.09)
  */

  memset(&binOutGroupConfig, 0, sizeof(binOutGroupConfig));
  if(sdnpdata_XmlGetBinOutGroupConfig(pSDNPSession->pDbHandle, &binOutGroupConfig))
  {
    /* minimumPulseTime */
    if(binOutGroupConfig.minimumPulseTime.type == SDNPDATA_XML_PULSEVALUEPERPOINT)
    {
      _currentValueString(pSDNPSession, "minimumPulseTime",  "<basedOnPointIndex />", curLen, bGetLen);
    }
    else
    {
#if TMWCNFG_SUPPORT_DOUBLE
      _currentValueNameFloatString(pSDNPSession, "minimumPulseTime", "fixed", binOutGroupConfig.minimumPulseTime.fixedPulseValue, curLen, bGetLen);
#else
      _currentValueNameDecimalString(pSDNPSession, "minimumPulseTime", "fixed", binOutGroupConfig.minimumPulseTime.fixedPulseValue, curLen, bGetLen);
#endif
    }

    /* maximumPulseTime */
    if(binOutGroupConfig.maximumPulseTime.type == SDNPDATA_XML_PULSEVALUEPERPOINT)
    {
      _currentValueString(pSDNPSession, "maximumPulseTime",  "<basedOnPointIndex />", curLen, bGetLen);
    }
    else
    {
#if TMWCNFG_SUPPORT_DOUBLE
      _currentValueNameFloatString(pSDNPSession, "maximumPulseTime", "fixed", binOutGroupConfig.maximumPulseTime.fixedPulseValue, curLen, bGetLen);
#else
      _currentValueNameDecimalString(pSDNPSession, "maximumPulseTime", "fixed", binOutGroupConfig.maximumPulseTime.fixedPulseValue, curLen, bGetLen);
#endif
    }
  }

#if SDNPDATA_SUPPORT_CLASS0_POINT
  _currentValueString(pSDNPSession, "class0ResponseMode", "<basedOnPointIndex />", curLen, bGetLen);
#else
  _currentValueString(pSDNPSession, "class0ResponseMode", _class0ResponseValue(_class0Response(pSDNPSession, DNPDEFS_OBJ_10_BIN_OUT_STATUSES)), curLen, bGetLen);
#endif

#if SDNPDATA_SUPPORT_OBJ13
  if(binOutGroupConfig.commandEvents == SDNPDATA_XML_COMMANDEVENTSUCCESS)
  {
    _currentValueString(pSDNPSession, "outputCommandEventObjects",  "<onSuccess />", curLen, bGetLen);
  }
  else if(binOutGroupConfig.commandEvents == SDNPDATA_XML_COMMANDEVENTALL)
  {
    _currentValueString(pSDNPSession, "outputCommandEventObjects",  "<allControlAttempts />", curLen, bGetLen);
  }
  else
#endif
  _currentValueString(pSDNPSession, "outputCommandEventObjects",  "<never />", curLen, bGetLen);

  _currentValueString(pSDNPSession, "defaultStaticVariation",  _variationGroupString(pSDNPSession->obj10DefaultVariation), curLen, bGetLen);
#if SDNPDATA_SUPPORT_OBJ11
  _currentValueString(pSDNPSession, "defaultEventVariation",  _variationGroupString(pSDNPSession->obj11DefaultVariation), curLen, bGetLen);
#endif
#if SDNPDATA_SUPPORT_OBJ13
  _currentValueString(pSDNPSession, "defaultCommandEventVariation",  _variationGroupString(pSDNPSession->obj13DefaultVariation), curLen, bGetLen);
#endif

#if SDNPDATA_SUPPORT_OBJ11
  _currentValueString(pSDNPSession, "changeEventReportingMode",  _reportingModeString(pSDNPSession->binaryOutputEventMode), curLen, bGetLen);
#endif

#if SDNPDATA_SUPPORT_OBJ13
  _currentValueString(pSDNPSession, "commandEventReportingMode",  _reportingModeString(pSDNPSession->binaryOutCmdEventMode), curLen, bGetLen);
#endif

  _currentValueDecimalString(pSDNPSession, "maxTimeBetweenSelectAndOperate", pSDNPSession->selectTimeout, curLen, bGetLen);

#if SDNPDATA_SUPPORT_OBJ11
  /* New in schema 2.09 */
  _currentValueNameDecimalString(pSDNPSession, "eventBufferPerObjectGroup", "numEvents", pSDNPSession->binaryOutputMaxEvents, curLen, bGetLen);
#endif

#if SDNPDATA_SUPPORT_OBJ13
  /* New in schema 2.09 */
  _currentValueNameDecimalString(pSDNPSession, "commandEventBufferPerObjectGroup", "numEvents", pSDNPSession->binaryOutCmdMaxEvents, curLen, bGetLen);
#endif

  pSDNPSession->xml2CurIndent--;
  _appendString(pSDNPSession, "</configuration>\n", curLen, bGetLen);

  pSDNPSession->xml2CurIndent--;
  _appendString(pSDNPSession, "</binaryOutputGroup>\n", curLen, bGetLen);
  pSDNPSession->xml2CurIndent--;
}

static void _buildBinaryOutputPoints(SDNPSESN *pSDNPSession, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  pSDNPSession->xml2CurIndent++;
  _appendString(pSDNPSession, "<binaryOutputPoints>\n", curLen, bGetLen);
  pSDNPSession->xml2CurIndent++;
  _appendString(pSDNPSession, "<configuration>\n", curLen, bGetLen);
  pSDNPSession->xml2CurIndent++;

  /* pointListDefinition */
  _pointListDefinition(pSDNPSession, DNPDEFS_OBJ_10_BIN_OUT_STATUSES, curLen, bGetLen);

  pSDNPSession->xml2CurIndent--;
  _appendString(pSDNPSession, "</configuration>\n", curLen, bGetLen);

  if(sdnpdata_binOutQuantity(pSDNPSession->pDbHandle)>0)
  {
    _appendString(pSDNPSession, "<dataPoints>\n", curLen, bGetLen);
  }
}

static void _buildBinaryOutputPointsTail(SDNPSESN *pSDNPSession, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  if(sdnpdata_binOutQuantity(pSDNPSession->pDbHandle)>0)
  {
    _appendString(pSDNPSession, "</dataPoints>\n", curLen, bGetLen);
  }

  pSDNPSession->xml2CurIndent--;
  _appendString(pSDNPSession, "</binaryOutputPoints>\n", curLen, bGetLen);
  pSDNPSession->xml2CurIndent--;
}
#endif /* SDNPDATA_SUPPORT_OBJ10 */

#if SDNPDATA_SUPPORT_OBJ20
static void _buildCounterGroup(SDNPSESN *pSDNPSession, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  SDNPDATA_XML_COUNTERGROUPCONFIG cntrGroupConfig;

  pSDNPSession->xml2CurIndent++;
  _appendString(pSDNPSession, "<counterGroup>\n", curLen, bGetLen);
  pSDNPSession->xml2CurIndent++;
  _appendString(pSDNPSession, "<configuration>\n", curLen, bGetLen);
  pSDNPSession->xml2CurIndent++;

  /*
    defaultCounterStaticVariation
    defaultCounterEventVariation
    counterClass0ResponseMode
    counterEventReportingMode
    defaultFrozenCounterStaticVariation
    defaultFrozenCounterEventVariation
    frozenCounterClass0ResponseMode
    frozenCounterEventReportingMode
    counterRollOver
    countersFrozen
    eventBufferPerObjectGroup (2.09)
    frozenEventBufferPerObjectGroup (2.09)
    reportValueChangeCounterEvents (2.10)
  */

  _currentValueString(pSDNPSession, "defaultCounterStaticVariation",  _variationGroupString(pSDNPSession->obj20DefaultVariation), curLen, bGetLen);
#if SDNPDATA_SUPPORT_OBJ22
  _currentValueString(pSDNPSession, "defaultCounterEventVariation",  _variationGroupString(pSDNPSession->obj22DefaultVariation), curLen, bGetLen);
#endif
#if SDNPDATA_SUPPORT_CLASS0_POINT
  _currentValueString(pSDNPSession, "counterClass0ResponseMode",   "<basedOnPointIndex />", curLen, bGetLen);
#else
  _currentValueString(pSDNPSession, "counterClass0ResponseMode",   _class0ResponseValue(_class0Response(pSDNPSession, DNPDEFS_OBJ_20_RUNNING_CNTRS)), curLen, bGetLen);
#endif
#if SDNPDATA_SUPPORT_OBJ22
  _currentValueString(pSDNPSession, "counterEventReportingMode",  _anlgReportingModeString(pSDNPSession->binaryCounterEventMode), curLen, bGetLen);
#endif

#if SDNPDATA_SUPPORT_OBJ21
  _currentValueString(pSDNPSession, "defaultFrozenCounterStaticVariation",  _variationGroupString(pSDNPSession->obj21DefaultVariation), curLen, bGetLen);
#if SDNPDATA_SUPPORT_OBJ23
  _currentValueString(pSDNPSession, "defaultFrozenCounterEventVariation",  _variationGroupString(pSDNPSession->obj23DefaultVariation), curLen, bGetLen);
#endif
#if SDNPDATA_SUPPORT_CLASS0_POINT
  _currentValueString(pSDNPSession, "frozenCounterClass0ResponseMode",   "<basedOnPointIndex />", curLen, bGetLen);
#else
  _currentValueString(pSDNPSession, "frozenCounterClass0ResponseMode",  _class0ResponseValue( _class0Response(pSDNPSession, DNPDEFS_OBJ_21_FROZEN_CNTRS)), curLen, bGetLen);
#endif
#endif

#if SDNPDATA_SUPPORT_OBJ23
  _currentValueString(pSDNPSession, "frozenCounterEventReportingMode",  _reportingModeString(pSDNPSession->frozenCounterEventMode), curLen, bGetLen);
#endif

  memset(&cntrGroupConfig, 0, sizeof(cntrGroupConfig));
  if(sdnpdata_XmlGetCntrGroupConfig(pSDNPSession->pDbHandle, &cntrGroupConfig))
  {
    if(cntrGroupConfig.counterRollOver.type == SDNPDATA_XML_CNTRROLLOVERVALUE)
    {
      _currentValueDecimalString(pSDNPSession, "counterRollOver", cntrGroupConfig.counterRollOver.value, curLen, bGetLen);
    }
    else if(cntrGroupConfig.counterRollOver.type == SDNPDATA_XML_CNTRROLLOVEROTHER)
    {
      _currentValueCustom(pSDNPSession, "counterRollOver", cntrGroupConfig.counterRollOver.other, curLen, bGetLen);
    }
    else
    {
      _currentValueString(pSDNPSession, "counterRollOver", _cntrRolloverString(cntrGroupConfig.counterRollOver.type), curLen, bGetLen);
    }

    if(cntrGroupConfig.countersFrozen.type == SDNPDATA_XML_CNTRFROZENOTHER)
    {
      _currentValueOther(pSDNPSession, "countersFrozen", cntrGroupConfig.countersFrozen.other, curLen, bGetLen);
    }
    else
    {
      _currentValueString(pSDNPSession, "countersFrozen", _cntrFrozenString(cntrGroupConfig.countersFrozen.type), curLen, bGetLen);
    }
  }

#if SDNPDATA_SUPPORT_OBJ22
  /* New in schema 2.09 */
  _currentValueNameDecimalString(pSDNPSession, "eventBufferPerObjectGroup", "numEvents", pSDNPSession->binaryCounterMaxEvents, curLen, bGetLen);
#endif

#if SDNPDATA_SUPPORT_OBJ23
  /* New in schema 2.09 */
  _currentValueNameDecimalString(pSDNPSession, "frozenEventBufferPerObjectGroup", "numEvents", pSDNPSession->frozenCounterMaxEvents, curLen, bGetLen);
#endif

  /* New in schema 2.10 reportValueChangeCounterEvents */
  _currentValueString(pSDNPSession, "reportValueChangeCounterEvents", "<no />", curLen, bGetLen);

  pSDNPSession->xml2CurIndent--;
  _appendString(pSDNPSession, "</configuration>\n", curLen, bGetLen);

  pSDNPSession->xml2CurIndent--;
  _appendString(pSDNPSession, "</counterGroup>\n", curLen, bGetLen);
  pSDNPSession->xml2CurIndent--;
}

static void _buildCounterPoints(SDNPSESN *pSDNPSession, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  pSDNPSession->xml2CurIndent++;
  _appendString(pSDNPSession, "<counterPoints>\n", curLen, bGetLen);
  pSDNPSession->xml2CurIndent++;
  _appendString(pSDNPSession, "<configuration>\n", curLen, bGetLen);
  pSDNPSession->xml2CurIndent++;

  /* pointListDefinition */
  _pointListDefinition(pSDNPSession, DNPDEFS_OBJ_20_RUNNING_CNTRS, curLen, bGetLen);

  pSDNPSession->xml2CurIndent--;
  _appendString(pSDNPSession, "</configuration>\n", curLen, bGetLen);

  if(sdnpdata_binCntrQuantity(pSDNPSession->pDbHandle)>0)
  {
    _appendString(pSDNPSession, "<dataPoints>\n", curLen, bGetLen);
  }
}

static void _buildCounterPointsTail(SDNPSESN *pSDNPSession, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  if(sdnpdata_binCntrQuantity(pSDNPSession->pDbHandle)>0)
  {
    _appendString(pSDNPSession, "</dataPoints>\n", curLen, bGetLen);
  }

  pSDNPSession->xml2CurIndent--;
  _appendString(pSDNPSession, "</counterPoints>\n", curLen, bGetLen);
  pSDNPSession->xml2CurIndent--;
}
#endif /* SDNPDATA_SUPPORT_OBJ20 */

#if SDNPDATA_SUPPORT_OBJ30
static void _buildAnalogInputGroup(SDNPSESN *pSDNPSession, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  SDNPDATA_XML_ANLGINGROUPCONFIG *pConfig;
  SDNPDATA_XML_ANLGINGROUPCONFIG config;

  pSDNPSession->xml2CurIndent++;
  _appendString(pSDNPSession, "<analogInputGroup>\n", curLen, bGetLen);
  pSDNPSession->xml2CurIndent++;
  _appendString(pSDNPSession, "<configuration>\n", curLen, bGetLen);
  pSDNPSession->xml2CurIndent++;
 /*
    defaultStaticVariation
    defaultEventVariation
    analogEventReportingMode
    class0ResponseMode
    analogDeadbandAssignments
    analogDeadbandAlgorithm
    eventBufferPerObjectGroup (2.09)
    frozenEventBufferPerObjectGroup (2.09)
    */

  _currentValueString(pSDNPSession, "defaultStaticVariation",  _variationGroupString(pSDNPSession->obj30DefaultVariation), curLen, bGetLen);
#if SDNPDATA_SUPPORT_OBJ32
  _currentValueString(pSDNPSession, "defaultEventVariation",  _variationGroupString(pSDNPSession->obj32DefaultVariation), curLen, bGetLen);
  _currentValueString(pSDNPSession, "analogEventReportingMode",  _anlgReportingModeString(pSDNPSession->analogInputEventMode), curLen, bGetLen);
#endif

#if SDNPDATA_SUPPORT_CLASS0_POINT
  _currentValueString(pSDNPSession, "analogInputClass0ResponseMode",   "<basedOnPointIndex />", curLen, bGetLen);
#else
  _currentValueString(pSDNPSession, "analogInputClass0ResponseMode",   _class0ResponseValue(_class0Response(pSDNPSession, DNPDEFS_OBJ_30_ANA_INPUTS)), curLen, bGetLen);
#endif

  memset(&config, 0, sizeof(config));
  if(sdnpdata_XmlGetAnlgInGroupConfig(pSDNPSession->pDbHandle, &config))
  {
    pConfig = &config;
    /* analogDeadbandAssignments */
    if(pConfig->analogDeadbandAssignments.type == SDNPDATA_XML_DDBNDASSIGNOTHER)
    {
      _appendString(pSDNPSession, "<analogDeadbandAssignments>\n", curLen, bGetLen);
      _currentValueCustom(pSDNPSession, "other", pConfig->analogDeadbandAssignments.other, curLen, bGetLen);
      pSDNPSession->xml2CurIndent--;
      _appendString(pSDNPSession, "</analogDeadbandAssignments>\n", curLen, bGetLen);
    }
    else
    {
      _currentValueString(pSDNPSession, "analogDeadbandAssignments", _deadbandAssignString(pConfig->analogDeadbandAssignments.type), curLen, bGetLen);
    }

    /* analogDeadbandAlgorithm */
    if(pConfig->analogDeadbandAlgorithm.type == SDNPDATA_XML_DDBNDALGOOTHER)
    {
      _appendString(pSDNPSession, "<analogDeadbandAlgorithm>\n", curLen, bGetLen);
      _currentValueCustom(pSDNPSession, "other", pConfig->analogDeadbandAlgorithm.other, curLen, bGetLen);
      pSDNPSession->xml2CurIndent--;
      _appendString(pSDNPSession, "</analogDeadbandAlgorithm>\n", curLen, bGetLen);
    }
    else
    {
      _currentValueString(pSDNPSession, "analogDeadbandAlgorithm", _deadbandAlgorithmString(pConfig->analogDeadbandAlgorithm.type), curLen, bGetLen);
    }

#if SDNPDATA_SUPPORT_OBJ31
    _currentValueString(pSDNPSession, "defaultFrozenAnalogInputStaticVariation", _variationGroupString(pSDNPSession->obj31DefaultVariation), curLen, bGetLen);
#if SDNPDATA_SUPPORT_OBJ33
    _currentValueString(pSDNPSession, "defaultFrozenAnalogInputEventVariation", _variationGroupString(pSDNPSession->obj33DefaultVariation), curLen, bGetLen);
#endif
#if SDNPDATA_SUPPORT_CLASS0_POINT
    _currentValueString(pSDNPSession, "frozenAnalogInputClass0ResponseMode", "<basedOnPointIndex />", curLen, bGetLen);
#else
    _currentValueString(pSDNPSession, "frozenAnalogInputClass0ResponseMode", _class0ResponseValue(_class0Response(pSDNPSession, DNPDEFS_OBJ_33_ANA_INPUTS)), curLen, bGetLen);
#endif
#if SDNPDATA_SUPPORT_OBJ33
    _currentValueString(pSDNPSession, "frozenAnalogEventReportingMode", _reportingModeString(pSDNPSession->frozenAnalogInEventMode), curLen, bGetLen);
#endif
#endif

  }

#if SDNPDATA_SUPPORT_OBJ32
  /* New in schema 2.09 */
  _currentValueNameDecimalString(pSDNPSession, "eventBufferPerObjectGroup", "numEvents", pSDNPSession->analogInputMaxEvents, curLen, bGetLen);
#endif

  /* New in schema 2.09 */
#if SDNPDATA_SUPPORT_OBJ33
  _currentValueNameDecimalString(pSDNPSession, "frozenEventBufferPerObjectGroup", "numEvents", pSDNPSession->frozenAnalogInMaxEvents, curLen, bGetLen);
#endif

  pSDNPSession->xml2CurIndent--;
  _appendString(pSDNPSession, "</configuration>\n", curLen, bGetLen);

  pSDNPSession->xml2CurIndent--;
  _appendString(pSDNPSession, "</analogInputGroup>\n", curLen, bGetLen);
  pSDNPSession->xml2CurIndent--;
}

static void _buildAnalogInputPoints(SDNPSESN *pSDNPSession, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  pSDNPSession->xml2CurIndent++;
  _appendString(pSDNPSession, "<analogInputPoints>\n", curLen, bGetLen);
  pSDNPSession->xml2CurIndent++;
  _appendString(pSDNPSession, "<configuration>\n", curLen, bGetLen);
  pSDNPSession->xml2CurIndent++;

  /* pointListDefinition */
  _pointListDefinition(pSDNPSession, DNPDEFS_OBJ_30_ANA_INPUTS, curLen, bGetLen);

  pSDNPSession->xml2CurIndent--;
  _appendString(pSDNPSession, "</configuration>\n", curLen, bGetLen);

  if(sdnpdata_anlgInQuantity(pSDNPSession->pDbHandle)>0)
  {
    _appendString(pSDNPSession, "<dataPoints>\n", curLen, bGetLen);
  }
}

static void _buildAnalogInputPointsTail(SDNPSESN *pSDNPSession, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  if(sdnpdata_anlgInQuantity(pSDNPSession->pDbHandle)>0)
  {
    _appendString(pSDNPSession, "</dataPoints>\n", curLen, bGetLen);
  }

  pSDNPSession->xml2CurIndent--;
  _appendString(pSDNPSession, "</analogInputPoints>\n", curLen, bGetLen);
  pSDNPSession->xml2CurIndent--;
}
#endif /* SDNPDATA_SUPPORT_OBJ30 */

#if SDNPDATA_SUPPORT_OBJ40
static void _buildAnalogOutputGroup(SDNPSESN *pSDNPSession, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  pSDNPSession->xml2CurIndent++;
  _appendString(pSDNPSession, "<analogOutputGroup>\n", curLen, bGetLen);
  pSDNPSession->xml2CurIndent++;
  _appendString(pSDNPSession, "<configuration>\n", curLen, bGetLen);
  pSDNPSession->xml2CurIndent++;

  /*
    defaultStaticVariation
    class0ResponseMode
    outputCommandEventObjects
    defaultChangeEventVariation
    defaultCommandEventVariation
    changeEventReportingMode
    commandEventReportingMode
    maxTimeBetweenSelectAndOperate
    eventBufferPerObjectGroup (2.09)
    commandEventBufferPerObjectGroup (2.09)
    */

  _currentValueString(pSDNPSession, "defaultStaticVariation",  _variationGroupString(pSDNPSession->obj40DefaultVariation), curLen, bGetLen);
#if SDNPDATA_SUPPORT_CLASS0_POINT
  _currentValueString(pSDNPSession, "class0ResponseMode", "<basedOnPointIndex />", curLen, bGetLen);
#else
  _currentValueString(pSDNPSession, "class0ResponseMode", _class0ResponseValue( _class0Response(pSDNPSession, DNPDEFS_OBJ_40_ANA_OUT_STATUSES)), curLen, bGetLen);
#endif
  _currentValueString(pSDNPSession, "outputCommandEventObjects",  "<onSuccess />", curLen, bGetLen);
#if SDNPDATA_SUPPORT_OBJ42
  _currentValueString(pSDNPSession, "defaultChangeEventVariation",   _variationGroupString(pSDNPSession->obj42DefaultVariation), curLen, bGetLen);
#endif
#if SDNPDATA_SUPPORT_OBJ43
  _currentValueString(pSDNPSession, "defaultCommandEventVariation",   _variationGroupString(pSDNPSession->obj43DefaultVariation), curLen, bGetLen);
#endif

#if SDNPDATA_SUPPORT_OBJ42
  _currentValueString(pSDNPSession, "changeEventReportingMode",  _reportingModeString(pSDNPSession->analogOutputEventMode), curLen, bGetLen);
#endif
#if SDNPDATA_SUPPORT_OBJ43
  _currentValueString(pSDNPSession, "commandEventReportingMode",  _reportingModeString(pSDNPSession->analogOutCmdEventMode), curLen, bGetLen);
#endif

  /* maxTimeBetweenSelectAndOperate */
  _currentValueDecimalString(pSDNPSession, "maxTimeBetweenSelectAndOperate", pSDNPSession->selectTimeout, curLen, bGetLen);

#if SDNPDATA_SUPPORT_OBJ42
  /* New in schema 2.09 */
  _currentValueNameDecimalString(pSDNPSession, "eventBufferPerObjectGroup", "numEvents", pSDNPSession->analogOutputMaxEvents, curLen, bGetLen);
#endif

#if SDNPDATA_SUPPORT_OBJ43
  /* New in schema 2.09 */
  _currentValueNameDecimalString(pSDNPSession, "commandEventBufferPerObjectGroup", "numEvents", pSDNPSession->analogOutCmdMaxEvents, curLen, bGetLen);
#endif

  pSDNPSession->xml2CurIndent--;
  _appendString(pSDNPSession, "</configuration>\n", curLen, bGetLen);

  pSDNPSession->xml2CurIndent--;
  _appendString(pSDNPSession, "</analogOutputGroup>\n", curLen, bGetLen);
  pSDNPSession->xml2CurIndent--;
}

static void _buildAnalogOutputPoints(SDNPSESN *pSDNPSession, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  pSDNPSession->xml2CurIndent++;
  _appendString(pSDNPSession, "<analogOutputPoints>\n", curLen, bGetLen);
  pSDNPSession->xml2CurIndent++;
  _appendString(pSDNPSession, "<configuration>\n", curLen, bGetLen);
  pSDNPSession->xml2CurIndent++;

  /* pointListDefinition */
  _pointListDefinition(pSDNPSession, DNPDEFS_OBJ_40_ANA_OUT_STATUSES, curLen, bGetLen);

  pSDNPSession->xml2CurIndent--;
  _appendString(pSDNPSession, "</configuration>\n", curLen, bGetLen);

  if(sdnpdata_anlgOutQuantity(pSDNPSession->pDbHandle)>0)
  {
    _appendString(pSDNPSession, "<dataPoints>\n", curLen, bGetLen);
  }
}

static void _buildAnalogOutputPointsTail(SDNPSESN *pSDNPSession, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  if(sdnpdata_anlgOutQuantity(pSDNPSession->pDbHandle)>0)
  {
    _appendString(pSDNPSession, "</dataPoints>\n", curLen, bGetLen);
  }

  pSDNPSession->xml2CurIndent--;
  _appendString(pSDNPSession, "</analogOutputPoints>\n", curLen, bGetLen);
  pSDNPSession->xml2CurIndent--;
}
#endif /* SDNPDATA_SUPPORT_OBJ40 */

#if SDNPDATA_SUPPORT_OBJ70
static void _buildSequentialFileTransfer(SDNPSESN *pSDNPSession, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  SDNPDATA_XML_FILECONFIG config;

  pSDNPSession->xml2CurIndent++;
  _appendString(pSDNPSession, "<sequentialFileTransfer>\n", curLen, bGetLen);
  pSDNPSession->xml2CurIndent++;
  _appendString(pSDNPSession, "<configuration>\n", curLen, bGetLen);

  pSDNPSession->xml2CurIndent++;

  /*
    fileTransferSupported  removed in 2.11
    fileAuthentication
    fileAppendMode
    permissionsSupport
    multipleBlocksInFragment
    maxOpenFiles
  */

#if SDNPDATA_SUPPORT_OBJ70
  memset(&config, 0, sizeof(config));
  if(sdnpdata_XmlGetFileConfig(pSDNPSession->pDbHandle, &config))
  {
    /* fileAuthentication */
    if(config.fileAuthentication.type == SDNPDATA_XML_FILESOMETIMES)
    {
      _appendString(pSDNPSession, "<fileAuthentication>\n", curLen, bGetLen);
      _currentValueCustom(pSDNPSession, "sometimes", config.fileAuthentication.sometimes, curLen, bGetLen);
      _appendString(pSDNPSession, "</fileAuthentication>\n", curLen, bGetLen);
    }
    else
    {
      if(config.fileAuthentication.type == SDNPDATA_XML_FILENEVER)
      {
        _currentValueString(pSDNPSession, "fileAuthentication", "<never />", curLen, bGetLen);
      }
      else if(config.fileAuthentication.type == SDNPDATA_XML_FILEALWAYS)
      {
        _currentValueString(pSDNPSession, "fileAuthentication", "<always />", curLen, bGetLen);
      }
    }

    /* fileAppendMode */
    if(config.fileAppendMode.type == SDNPDATA_XML_FILESOMETIMES)
    {
      _appendString(pSDNPSession, "<fileAppendMode>\n", curLen, bGetLen);
      _currentValueCustom(pSDNPSession, "sometimes", config.fileAppendMode.sometimes, curLen, bGetLen);
      _appendString(pSDNPSession, "</fileAppendMode>\n", curLen, bGetLen);
    }
    else
    {
      if(config.fileAppendMode.type == SDNPDATA_XML_FILENEVER)
      {
        _currentValueString(pSDNPSession, "fileAppendMode", "<never />", curLen, bGetLen);
      }
      else if(config.fileAppendMode.type == SDNPDATA_XML_FILEALWAYS)
      {
        _currentValueString(pSDNPSession, "fileAppendMode", "<always />", curLen, bGetLen);
      }
    }

    /* permissionsSupport */
    _appendString(pSDNPSession, "<permissionsSupport>\n", curLen, bGetLen);
    pSDNPSession->xml2CurIndent++;

    _appendString(pSDNPSession, "<currentValue>\n", curLen, bGetLen);
    pSDNPSession->xml2CurIndent++;

    _emptyElementString(pSDNPSession, curLen, bGetLen, "ownerReadAllowed", config.permissionsSupport.ownerReadAllowed);
    _emptyElementString(pSDNPSession, curLen, bGetLen, "ownerWriteAllowed", config.permissionsSupport.ownerWriteAllowed);
    _emptyElementString(pSDNPSession, curLen, bGetLen, "ownerExecuteAllowed", config.permissionsSupport.ownerExecuteAllowed);
    _emptyElementString(pSDNPSession, curLen, bGetLen, "groupReadAllowed", config.permissionsSupport.groupReadAllowed);
    _emptyElementString(pSDNPSession, curLen, bGetLen, "groupWriteAllowed", config.permissionsSupport.groupWriteAllowed);
    _emptyElementString(pSDNPSession, curLen, bGetLen, "groupExecuteAllowed", config.permissionsSupport.groupExecuteAllowed);
    _emptyElementString(pSDNPSession, curLen, bGetLen, "worldReadAllowed", config.permissionsSupport.worldReadAllowed);
    _emptyElementString(pSDNPSession, curLen, bGetLen, "worldWriteAllowed", config.permissionsSupport.worldWriteAllowed);
    _emptyElementString(pSDNPSession, curLen, bGetLen, "worldExecuteAllowed", config.permissionsSupport.worldExecuteAllowed);

    pSDNPSession->xml2CurIndent--;
    _appendString(pSDNPSession, "</currentValue>\n", curLen, bGetLen);

    pSDNPSession->xml2CurIndent--;
    _appendString(pSDNPSession, "</permissionsSupport>\n", curLen, bGetLen);

    /* multipleBlocksInFragment */
    _currentValueString(pSDNPSession, "multipleBlocksInFragment", _yesNoString(TMWDEFS_FALSE), curLen, bGetLen);

    /* maxOpenFiles */
    _currentValueDecimalString(pSDNPSession, "maxOpenFiles", 1, curLen, bGetLen);
  }
  #endif

  pSDNPSession->xml2CurIndent--;
  _appendString(pSDNPSession, "</configuration>\n", curLen, bGetLen);

  _appendString(pSDNPSession, "</sequentialFileTransfer>\n", curLen, bGetLen);
  pSDNPSession->xml2CurIndent--;
}

static void _buildSequentialFiles(SDNPSESN *pSDNPSession, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  pSDNPSession->xml2CurIndent++;
  _appendString(pSDNPSession, "<sequentialFiles>\n", curLen, bGetLen);
  pSDNPSession->xml2CurIndent++;
  _appendString(pSDNPSession, "<configuration>\n", curLen, bGetLen);

  pSDNPSession->xml2CurIndent++;

  /* fileListDefinition */
  _pointListDefinition(pSDNPSession, DNPDEFS_OBJ_70_FILE_IDENTIFIER, curLen, bGetLen);

  pSDNPSession->xml2CurIndent--;
  _appendString(pSDNPSession, "</configuration>\n", curLen, bGetLen);

  _appendString(pSDNPSession, "<files>\n", curLen, bGetLen);
}

static void _buildSequentialFilesTail(SDNPSESN *pSDNPSession, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  _appendString(pSDNPSession, "</files>\n", curLen, bGetLen);
  pSDNPSession->xml2CurIndent--;
  _appendString(pSDNPSession, "</sequentialFiles>\n", curLen, bGetLen);
  pSDNPSession->xml2CurIndent--;
}
#endif /* SDNPDATA_SUPPORT_OBJ70 */

#if SDNPDATA_SUPPORT_OBJ110
static void _buildOctetStringGroup(SDNPSESN *pSDNPSession, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  pSDNPSession->xml2CurIndent++;
  _appendString(pSDNPSession, "<octetStringGroup>\n", curLen, bGetLen);
  pSDNPSession->xml2CurIndent++;
  _appendString(pSDNPSession, "<configuration>\n", curLen, bGetLen);
  pSDNPSession->xml2CurIndent++;

  /*
   * eventReportingMode
   * class0ResponseMode
   * eventBufferPerObjectGroup (2.09)
   * objectGroupSelection (2.10)
  */

#if SDNPDATA_SUPPORT_OBJ111
  _currentValueString(pSDNPSession, "eventReportingMode", _reportingModeString(pSDNPSession->stringEventMode), curLen, bGetLen);
#endif

#if SDNPDATA_SUPPORT_CLASS0_POINT
  _currentValueString(pSDNPSession, "class0ResponseMode", "<basedOnPointIndex />", curLen, bGetLen);
#else
  _currentValueString(pSDNPSession, "class0ResponseMode", _class0ResponseValue(_class0Response(pSDNPSession, DNPDEFS_OBJ_110_STRING_DATA)), curLen, bGetLen);
#endif

#if SDNPDATA_SUPPORT_OBJ111
  /* New in schema 2.09 */
  _currentValueNameDecimalString(pSDNPSession, "eventBufferPerObjectGroup", "numEvents", pSDNPSession->stringMaxEvents, curLen, bGetLen);

#if SDNPDATA_SUPPORT_OBJ114
  /* New in schema 2.10 */
  _currentValueString(pSDNPSession, "objectGroupSelection", "<basedOnPointIndex />", curLen, bGetLen);
#else
   /* New in schema 2.10 */
  _currentValueString(pSDNPSession, "objectGroupSelection", "<fixedGroup110 />", curLen, bGetLen);
#endif
#endif

  pSDNPSession->xml2CurIndent--;
  _appendString(pSDNPSession, "</configuration>\n", curLen, bGetLen);

  pSDNPSession->xml2CurIndent--;
  _appendString(pSDNPSession, "</octetStringGroup>\n", curLen, bGetLen);
  pSDNPSession->xml2CurIndent--;
}

static void _buildOctetStringPoints(SDNPSESN *pSDNPSession, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  pSDNPSession->xml2CurIndent++;
  _appendString(pSDNPSession, "<octetStringPoints>\n", curLen, bGetLen);
  pSDNPSession->xml2CurIndent++;
  _appendString(pSDNPSession, "<configuration>\n", curLen, bGetLen);
  pSDNPSession->xml2CurIndent++;

  /* pointListDefinition */
  _pointListDefinition(pSDNPSession, DNPDEFS_OBJ_110_STRING_DATA, curLen, bGetLen);

  pSDNPSession->xml2CurIndent--;
  _appendString(pSDNPSession, "</configuration>\n", curLen, bGetLen);

  if(sdnpdata_strQuantity(pSDNPSession->pDbHandle)>0)
  {
    _appendString(pSDNPSession, "<dataPoints>\n", curLen, bGetLen);
  }
}

static void _buildOctetStringPointsTail(SDNPSESN *pSDNPSession, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  if(sdnpdata_strQuantity(pSDNPSession->pDbHandle)>0)
  {
    _appendString(pSDNPSession, "</dataPoints>\n", curLen, bGetLen);
  }

  pSDNPSession->xml2CurIndent--;
  _appendString(pSDNPSession, "</octetStringPoints>\n", curLen, bGetLen);
  pSDNPSession->xml2CurIndent--;
}
#endif /* SDNPDATA_SUPPORT_OBJ110 */

#if SDNPDATA_SUPPORT_OBJ112
static void _buildVirtualTerminalGroup(SDNPSESN *pSDNPSession, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  pSDNPSession->xml2CurIndent++;
  _appendString(pSDNPSession, "<virtualTerminalGroup>\n", curLen, bGetLen);
  pSDNPSession->xml2CurIndent++;
  _appendString(pSDNPSession, "<configuration>\n", curLen, bGetLen);
  pSDNPSession->xml2CurIndent++;

  /*
   * eventBufferPerObjectGroup (2.09)
   */
#if SDNPDATA_SUPPORT_OBJ113
  /* New in schema 2.09 */
  _currentValueNameDecimalString(pSDNPSession, "eventBufferPerObjectGroup", "numEvents", pSDNPSession->virtualTerminalMaxEvents, curLen, bGetLen);
#endif

  pSDNPSession->xml2CurIndent--;
  _appendString(pSDNPSession, "</configuration>\n", curLen, bGetLen);

  pSDNPSession->xml2CurIndent--;
  _appendString(pSDNPSession, "</virtualTerminalGroup>\n", curLen, bGetLen);
  pSDNPSession->xml2CurIndent--;
}

static void _buildVirtualTerminalPoints(SDNPSESN *pSDNPSession, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  pSDNPSession->xml2CurIndent++;
  _appendString(pSDNPSession, "<virtualTerminalPoints>\n", curLen, bGetLen);
  pSDNPSession->xml2CurIndent++;
  _appendString(pSDNPSession, "<configuration>\n", curLen, bGetLen);
  pSDNPSession->xml2CurIndent++;


  /* pointListDefinition */
  _pointListDefinition(pSDNPSession, DNPDEFS_OBJ_112_VTERM_OUTPUT, curLen, bGetLen);

  pSDNPSession->xml2CurIndent--;
  _appendString(pSDNPSession, "</configuration>\n", curLen, bGetLen);

  if(sdnpdata_vtermQuantity(pSDNPSession->pDbHandle)>0)
  {
    _appendString(pSDNPSession, "<dataPoints>\n", curLen, bGetLen);
  }
}

static void _buildVirtualTerminalPointsTail(SDNPSESN *pSDNPSession, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  if(sdnpdata_vtermQuantity(pSDNPSession->pDbHandle)>0)
  {
    _appendString(pSDNPSession, "</dataPoints>\n", curLen, bGetLen);
  }

  pSDNPSession->xml2CurIndent--;
  _appendString(pSDNPSession, "</virtualTerminalPoints>\n", curLen, bGetLen);
  pSDNPSession->xml2CurIndent--;
}
#endif /* SDNPDATA_SUPPORT_OBJ112 */

#if SDNPDATA_SUPPORT_DATASETS

/* routine: _buildDatasetProtoGetLen */
void _buildDatasetProtoGetLen(SDNPSESN *pSDNPSession, TMWTYPES_ULONG *pCurLen)
{
  TMWTYPES_USHORT pointNum;
  TMWTYPES_USHORT numPoints;

  numPoints = sdnpdata_datasetProtoQuantity(pSDNPSession->pDbHandle);
  for(pointNum = pSDNPSession->xml2CurSavePointNum; pointNum < numPoints; pointNum++)
  {
    TMWTYPES_UCHAR elemNum;
    TMWTYPES_UCHAR  numElems;
    DNPDATA_DATASET_DESCR_ELEM *pDescrElems;
    TMWTYPES_UCHAR uuid[16];

    void *pProtoPoint = sdnpdata_datasetProtoGetPoint(pSDNPSession->pDbHandle, pointNum);
    if(pProtoPoint == TMWDEFS_NULL)
      continue;

    pDescrElems = sdnpdata_datasetProtoRead(pProtoPoint, &numElems, (TMWTYPES_UCHAR*)&uuid);
    _buildDatasetProtoStart(pSDNPSession, pProtoPoint, uuid, pointNum, pCurLen, TMWDEFS_TRUE);

    for(elemNum = 0; elemNum < numElems; elemNum++)
    {
      _buildDatasetProtoElem(pSDNPSession, pDescrElems, elemNum, pCurLen, TMWDEFS_TRUE);
    }

    _buildDatasetProtoEnd(pSDNPSession, pCurLen, TMWDEFS_TRUE);
  }
}

/* function: _buildDatasetProto */
void _buildDatasetProto(SDNPSESN *pSDNPSession, TMWTYPES_ULONG *pCurLen, TMWTYPES_ULONG *pTestLen, TMWTYPES_ULONG maxLen)
{
  TMWTYPES_UCHAR  saveIndent;
  TMWTYPES_USHORT pointNum;
  TMWTYPES_USHORT numPoints;

  numPoints = sdnpdata_datasetProtoQuantity(pSDNPSession->pDbHandle);

  for(pointNum = pSDNPSession->xml2CurSavePointNum; pointNum < numPoints; pointNum++)
  {
    TMWTYPES_UCHAR  numElems;
    DNPDATA_DATASET_DESCR_ELEM *pDescrElems;
    TMWTYPES_UCHAR uuid[16];

    void *pProtoPoint = sdnpdata_datasetProtoGetPoint(pSDNPSession->pDbHandle, pointNum);
    if(pProtoPoint == TMWDEFS_NULL)
      continue;

    pDescrElems = sdnpdata_datasetProtoRead(pProtoPoint, &numElems, (TMWTYPES_UCHAR*)&uuid);
    if(pSDNPSession->xml2SaveSubState == SDNPXML2_SAVE_DATASETSTART)
    {
      saveIndent = pSDNPSession->xml2CurIndent;
      _buildDatasetProtoStart(pSDNPSession, pProtoPoint, uuid, pointNum, pTestLen, TMWDEFS_TRUE);
      pSDNPSession->xml2CurIndent = saveIndent;
      if (*pTestLen<maxLen)
      {
        _buildDatasetProtoStart(pSDNPSession, pProtoPoint, uuid, pointNum, pCurLen, TMWDEFS_FALSE);
        pSDNPSession->xml2SaveSubState = SDNPXML2_SAVE_DATASETELEM;
      }
      else
        break;
    }

    if(pSDNPSession->xml2SaveSubState == SDNPXML2_SAVE_DATASETELEM)
    {
      TMWTYPES_UCHAR elemNum;
      for(elemNum = pSDNPSession->xml2CurSaveElemNum; elemNum < numElems; elemNum++)
      {
        saveIndent = pSDNPSession->xml2CurIndent;
        _buildDatasetProtoElem(pSDNPSession, pDescrElems, elemNum, pTestLen, TMWDEFS_TRUE);
        pSDNPSession->xml2CurIndent = saveIndent;
        if (*pTestLen<maxLen)
        {
          _buildDatasetProtoElem(pSDNPSession, pDescrElems, elemNum, pCurLen, TMWDEFS_FALSE);
          pSDNPSession->xml2CurSaveElemNum++;
        }
        else
          break;
      }
      if (*pTestLen>=maxLen)
        break;

      pSDNPSession->xml2SaveSubState = SDNPXML2_SAVE_DATASETEND;
      pSDNPSession->xml2CurSaveElemNum = 0;
    }

    if(pSDNPSession->xml2SaveSubState == SDNPXML2_SAVE_DATASETEND)
    {
      saveIndent = pSDNPSession->xml2CurIndent;
      _buildDatasetProtoEnd(pSDNPSession, pTestLen, TMWDEFS_TRUE);
      pSDNPSession->xml2CurIndent = saveIndent;
      if (*pTestLen<maxLen)
      {
        _buildDatasetProtoEnd(pSDNPSession, pCurLen, TMWDEFS_FALSE);
        pSDNPSession->xml2SaveSubState = SDNPXML2_SAVE_DATASETSTART;
      }
      else
        break;
    }
  }
}

/* routine: _buildDatasetDescrGetLen */
void _buildDatasetDescrGetLen(SDNPSESN *pSDNPSession, TMWTYPES_ULONG *pCurLen)
{
  TMWTYPES_USHORT pointNum;
  TMWTYPES_USHORT numPoints = sdnpdata_datasetDescrQuantity(pSDNPSession->pDbHandle);

  for(pointNum = 0; pointNum < numPoints; pointNum++)
  {
    DNPDATA_DATASET_DESCR_ELEM *pDescrElems;
    TMWTYPES_UCHAR elemNum;
    TMWTYPES_UCHAR numElems;

    void *pDescrPoint = sdnpdata_datasetDescrGetPoint(pSDNPSession->pDbHandle, pointNum);
    if(pDescrPoint == TMWDEFS_NULL)
      continue;

    _buildDatasetDescrStart(pSDNPSession, pDescrPoint, pointNum, pCurLen, TMWDEFS_TRUE);

    pDescrElems = sdnpdata_datasetDescrReadCont(pDescrPoint, &numElems);
    for(elemNum = 0; elemNum < numElems; elemNum++)
    {
      _buildDatasetDescrElem(pSDNPSession, pDescrElems, elemNum, pCurLen, TMWDEFS_TRUE);
    }

    _buildDatasetDescrIndex(pSDNPSession,pSDNPSession->pDbHandle, pointNum, pCurLen, TMWDEFS_TRUE);

    _buildDatasetDescrData(pSDNPSession,pSDNPSession->pDbHandle, pointNum, pCurLen, TMWDEFS_TRUE);
  }
}

/* routine: _buildDatasetDescr */
void _buildDatasetDescr(SDNPSESN *pSDNPSession, TMWTYPES_ULONG *pCurLen, TMWTYPES_ULONG *pTestLen, TMWTYPES_ULONG maxLen)
{
  TMWTYPES_UCHAR  saveIndent;
  TMWTYPES_USHORT  pointNum;
  TMWTYPES_USHORT numPoints = sdnpdata_datasetDescrQuantity(pSDNPSession->pDbHandle);

  for(pointNum = pSDNPSession->xml2CurSavePointNum; pointNum < numPoints; pointNum++)
  {
    void *pDescrPoint = sdnpdata_datasetDescrGetPoint(pSDNPSession->pDbHandle, pointNum);
    if(pDescrPoint == TMWDEFS_NULL)
      continue;

    if(pSDNPSession->xml2SaveSubState == SDNPXML2_SAVE_DATASETSTART)
    {
      saveIndent = pSDNPSession->xml2CurIndent;
      _buildDatasetDescrStart(pSDNPSession, pDescrPoint, pointNum, pTestLen, TMWDEFS_TRUE);
      pSDNPSession->xml2CurIndent = saveIndent;
      if (*pTestLen<maxLen)
      {
        _buildDatasetDescrStart(pSDNPSession, pDescrPoint, pointNum, pCurLen, TMWDEFS_FALSE);
        pSDNPSession->xml2SaveSubState = SDNPXML2_SAVE_DATASETELEM;
      }
      else
        break;
    }

    if(pSDNPSession->xml2SaveSubState == SDNPXML2_SAVE_DATASETELEM)
    {
      DNPDATA_DATASET_DESCR_ELEM *pDescrElems;
      TMWTYPES_UCHAR elemNum;
      TMWTYPES_UCHAR numElems;
      pDescrElems = sdnpdata_datasetDescrReadCont(pDescrPoint, &numElems);
      for(elemNum = pSDNPSession->xml2CurSaveElemNum; elemNum < numElems; elemNum++)
      {
        saveIndent = pSDNPSession->xml2CurIndent;
        _buildDatasetDescrElem(pSDNPSession, pDescrElems, elemNum, pTestLen, TMWDEFS_TRUE);
        pSDNPSession->xml2CurIndent = saveIndent;
        if (*pTestLen<maxLen)
        {
          _buildDatasetDescrElem(pSDNPSession, pDescrElems, elemNum, pCurLen, TMWDEFS_FALSE);
          pSDNPSession->xml2CurSaveElemNum++;
        }
        else
          break;
      }
      if (*pTestLen>=maxLen)
        break;

      pSDNPSession->xml2SaveSubState = SDNPXML2_SAVE_DATASETINDEX;
      pSDNPSession->xml2CurSaveElemNum = 0;
    }

    if(pSDNPSession->xml2SaveSubState == SDNPXML2_SAVE_DATASETINDEX)
    {
      saveIndent = pSDNPSession->xml2CurIndent;
      _buildDatasetDescrIndex(pSDNPSession,pSDNPSession->pDbHandle, pointNum, pTestLen, TMWDEFS_TRUE);
      pSDNPSession->xml2CurIndent = saveIndent;
      if (*pTestLen<maxLen)
      {
        _buildDatasetDescrIndex(pSDNPSession,pSDNPSession->pDbHandle, pointNum, pCurLen, TMWDEFS_FALSE);
        pSDNPSession->xml2SaveSubState = SDNPXML2_SAVE_DATASETDATA;
      }
      else
        break;
    }
    if(pSDNPSession->xml2SaveSubState == SDNPXML2_SAVE_DATASETDATA)
    {
      saveIndent = pSDNPSession->xml2CurIndent;
      _buildDatasetDescrData(pSDNPSession,pSDNPSession->pDbHandle, pointNum, pTestLen, TMWDEFS_TRUE);
      pSDNPSession->xml2CurIndent = saveIndent;
      if (*pTestLen<maxLen)
      {
        _buildDatasetDescrData(pSDNPSession,pSDNPSession->pDbHandle, pointNum, pCurLen, TMWDEFS_FALSE);
        pSDNPSession->xml2CurSavePointNum = pointNum+1;
        pSDNPSession->xml2SaveSubState = SDNPXML2_SAVE_DATASETSTART;
      }
      else
        break;
    }
  }
}
#endif  /* SDNPDATA_SUPPORT_DATASETS */

/* routine: sdnpxml2_saveDatabase */
SDNPXML2_SAVE_STATE TMWDEFS_GLOBAL sdnpxml2_saveDatabase(TMWSESN *pSession, TMWTYPES_CHAR *pBuffer, TMWTYPES_ULONG maxLen)
{
  SDNPSESN *pSDNPSession;
  SDNPXML2_SAVE_STATE prevState;
  SDNPXML2_SAVE_SUBSTATE prevSubState;
  TMWTYPES_UCHAR  saveIndent;
  TMWTYPES_USHORT numPoints;
  TMWTYPES_USHORT pointNum;
  TMWTYPES_USHORT prevPointNum;
  TMWTYPES_UCHAR  prevElemNum;
  TMWTYPES_BOOL   loopControl = TMWDEFS_TRUE;
  TMWTYPES_ULONG  curLen=0;
  TMWTYPES_ULONG  testLen=0;

  pSDNPSession = (SDNPSESN *)pSession;
  pSDNPSession->pXml2Buffer = pBuffer;

  if(pSDNPSession == TMWDEFS_NULL)
    return(SDNPXML2_SAVE_FAILED);

  prevState = pSDNPSession->xml2SaveState;
  prevPointNum = pSDNPSession->xml2CurSavePointNum;
  prevElemNum = pSDNPSession->xml2CurSaveElemNum;
  prevSubState = pSDNPSession->xml2SaveSubState;

  /* Loop till a break is reached */
  while(loopControl)
  {
    saveIndent = pSDNPSession->xml2CurIndent;
    switch (pSDNPSession->xml2SaveState)
    {
    case SDNPXML2_SAVE_DOC_HEADER:
      _buildDocHeader(pSDNPSession, &testLen, TMWDEFS_TRUE);
      pSDNPSession->xml2CurIndent = saveIndent;
      if (testLen>=maxLen)
      {
        break;
      }
      _buildDocHeader(pSDNPSession, &curLen, TMWDEFS_FALSE);
      pSDNPSession->xml2SaveState = SDNPXML2_SAVE_CONFIG_HEAD;
      continue;
      break;

    case SDNPXML2_SAVE_CONFIG_HEAD:
      _buildConfigurationHeader(pSDNPSession, &testLen, TMWDEFS_TRUE);
      pSDNPSession->xml2CurIndent = saveIndent;
      if (testLen>=maxLen)
      {
        break;
      }
      _buildConfigurationHeader(pSDNPSession, &curLen, TMWDEFS_FALSE);
      pSDNPSession->xml2SaveState = SDNPXML2_SAVE_CONFIG;
      continue;
      break;

    case SDNPXML2_SAVE_CONFIG:
      _buildDeviceConfig(pSDNPSession, &testLen, TMWDEFS_TRUE);
      pSDNPSession->xml2CurIndent = saveIndent;
      if (testLen>=maxLen)
      {
        break;
      }
      _buildDeviceConfig(pSDNPSession, &curLen, TMWDEFS_FALSE);
      pSDNPSession->xml2SaveState = SDNPXML2_SAVE_SERIALCONFIG;
      continue;
      break;

    case SDNPXML2_SAVE_SERIALCONFIG:
      _buildSerialConfig(pSDNPSession, &testLen, TMWDEFS_TRUE);
      pSDNPSession->xml2CurIndent = saveIndent;
      if (testLen>=maxLen)
      {
        break;
      }
      _buildSerialConfig(pSDNPSession, &curLen, TMWDEFS_FALSE);
      pSDNPSession->xml2SaveState = SDNPXML2_SAVE_NETWORKCONFIG;
      continue;
      break;

    case SDNPXML2_SAVE_NETWORKCONFIG:
      _buildNetworkConfig(pSDNPSession, &testLen, TMWDEFS_TRUE);
      pSDNPSession->xml2CurIndent = saveIndent;
      if (testLen>=maxLen)
      {
        break;
      }
      _buildNetworkConfig(pSDNPSession, &curLen, TMWDEFS_FALSE);
      pSDNPSession->xml2SaveState = SDNPXML2_SAVE_LINKCONFIG;
      continue;
      break;

    case SDNPXML2_SAVE_LINKCONFIG:
      _buildLinkConfig(pSDNPSession, &testLen, TMWDEFS_TRUE);
      pSDNPSession->xml2CurIndent = saveIndent;
      if (testLen>=maxLen)
      {
        break;
      }
      _buildLinkConfig(pSDNPSession, &curLen, TMWDEFS_FALSE);
      pSDNPSession->xml2SaveState = SDNPXML2_SAVE_APPLCONFIG;
      continue;
      break;

    case SDNPXML2_SAVE_APPLCONFIG:
     _buildApplConfig(pSDNPSession, &testLen, TMWDEFS_TRUE);
      pSDNPSession->xml2CurIndent = saveIndent;
      if (testLen>=maxLen)
      {
        break;
      }
      _buildApplConfig(pSDNPSession, &curLen, TMWDEFS_FALSE);
      pSDNPSession->xml2SaveState = SDNPXML2_SAVE_OUTSTATIONCONFIG;
      continue;
      break;

      /* skip masterConfig */

    case SDNPXML2_SAVE_OUTSTATIONCONFIG:
      _buildOutstationConfig(pSDNPSession, &testLen, TMWDEFS_TRUE);
      pSDNPSession->xml2CurIndent = saveIndent;
      if (testLen>=maxLen)
      {
        break;
      }
      _buildOutstationConfig(pSDNPSession, &curLen, TMWDEFS_FALSE);
      pSDNPSession->xml2SaveState = SDNPXML2_SAVE_UNSOLCONFIG;
      continue;
      break;

    case SDNPXML2_SAVE_UNSOLCONFIG:
      _buildUnsolicitedConfig(pSDNPSession, &testLen, TMWDEFS_TRUE);
      pSDNPSession->xml2CurIndent = saveIndent;
      if (testLen>=maxLen)
      {
        break;
      }
      _buildUnsolicitedConfig(pSDNPSession, &curLen, TMWDEFS_FALSE);
      pSDNPSession->xml2SaveState = SDNPXML2_SAVE_UNSOLTRIGGERCONFIG;
      continue;
      break;

    case SDNPXML2_SAVE_UNSOLTRIGGERCONFIG:
      _buildUnsolTriggerConfig(pSDNPSession, &testLen, TMWDEFS_TRUE);
      pSDNPSession->xml2CurIndent = saveIndent;
      if (testLen>=maxLen)
      {
        break;
      }
      _buildUnsolTriggerConfig(pSDNPSession, &curLen, TMWDEFS_FALSE);
      pSDNPSession->xml2SaveState = SDNPXML2_SAVE_OUTPERFORMCONFIG;
      continue;
      break;

    case SDNPXML2_SAVE_OUTPERFORMCONFIG:
      _buildOutperformConfig(pSDNPSession, &testLen, TMWDEFS_TRUE);
      pSDNPSession->xml2CurIndent = saveIndent;
      if (testLen>=maxLen)
      {
        break;
      }
      _buildOutperformConfig(pSDNPSession, &curLen, TMWDEFS_FALSE);
      pSDNPSession->xml2SaveState = SDNPXML2_SAVE_FIELDCONFIG;
      continue;
      break;

    case SDNPXML2_SAVE_FIELDCONFIG:
      _buildFieldConfig(pSDNPSession, &testLen, TMWDEFS_TRUE);
      pSDNPSession->xml2CurIndent = saveIndent;
      if (testLen>=maxLen)
      {
        break;
      }
      _buildFieldConfig(pSDNPSession, &curLen, TMWDEFS_FALSE);
      pSDNPSession->xml2SaveState = SDNPXML2_SAVE_SECURITYCONFIG;
      continue;
      break;

    case SDNPXML2_SAVE_SECURITYCONFIG:
      _buildSecurityConfig(pSDNPSession, &testLen, TMWDEFS_TRUE);
      pSDNPSession->xml2CurIndent = saveIndent;
      if (testLen>=maxLen)
      {
        break;
      }
      _buildSecurityConfig(pSDNPSession, &curLen, TMWDEFS_FALSE);
      pSDNPSession->xml2SaveState = SDNPXML2_SAVE_BROADCASTCONFIG;
      continue;
      break;

    case SDNPXML2_SAVE_BROADCASTCONFIG:
      _buildBroadcastConfig(pSDNPSession, &testLen, TMWDEFS_TRUE);
      pSDNPSession->xml2CurIndent = saveIndent;
      if (testLen>=maxLen)
      {
        break;
      }
      _buildBroadcastConfig(pSDNPSession, &curLen, TMWDEFS_FALSE);
      pSDNPSession->xml2SaveState = SDNPXML2_SAVE_CONFIG_TAIL;
      continue;
      break;

    case SDNPXML2_SAVE_CONFIG_TAIL:
      _buildConfigurationTail(pSDNPSession, &testLen, TMWDEFS_TRUE);
      pSDNPSession->xml2CurIndent = saveIndent;
      if (testLen>=maxLen)
      {
        break;
      }
      _buildConfigurationTail(pSDNPSession, &curLen, TMWDEFS_FALSE);
      pSDNPSession->xml2SaveState = SDNPXML2_SAVE_DATABASE_HEAD;
      continue;
      break;

    case SDNPXML2_SAVE_DATABASE_HEAD:
      _buildDatabaseHeader(pSDNPSession,  &testLen, TMWDEFS_TRUE);
      pSDNPSession->xml2CurIndent = saveIndent;
      if (testLen>=maxLen)
      {
        break;
      }
      _buildDatabaseHeader(pSDNPSession, &curLen, TMWDEFS_FALSE);
      pSDNPSession->xml2SaveState = SDNPXML2_SAVE_BIN_IN;
      continue;
      break;

    case SDNPXML2_SAVE_BIN_IN:
#if SDNPDATA_SUPPORT_OBJ1
      _buildBinaryInputGroup(pSDNPSession, &testLen, TMWDEFS_TRUE);
      pSDNPSession->xml2CurIndent = saveIndent;
      if (testLen>=maxLen)
      {
        break;
      }
      _buildBinaryInputGroup(pSDNPSession, &curLen, TMWDEFS_FALSE);

#endif
      pSDNPSession->xml2SaveState = SDNPXML2_SAVE_DBL_IN;
      continue;
      break;

    case SDNPXML2_SAVE_DBL_IN:
#if SDNPDATA_SUPPORT_OBJ3
      _buildDoubleBitInputGroup(pSDNPSession, &testLen, TMWDEFS_TRUE);
      pSDNPSession->xml2CurIndent = saveIndent;
      if (testLen>=maxLen)
      {
        break;
      }
      _buildDoubleBitInputGroup(pSDNPSession, &curLen, TMWDEFS_FALSE);
      /* For safety, always set it back to 2 */
      pSDNPSession->xml2CurIndent = 2;

#endif
      pSDNPSession->xml2SaveState = SDNPXML2_SAVE_BIN_OUT;
      continue;
      break;

    case SDNPXML2_SAVE_BIN_OUT:
#if SDNPDATA_SUPPORT_OBJ10
      _buildBinaryOutputGroup(pSDNPSession, &testLen, TMWDEFS_TRUE);
      pSDNPSession->xml2CurIndent = saveIndent;
      if (testLen>=maxLen)
      {
        break;
      }
      _buildBinaryOutputGroup(pSDNPSession, &curLen, TMWDEFS_FALSE);

#endif
      pSDNPSession->xml2SaveState = SDNPXML2_SAVE_BIN_COUNTER;
      continue;
      break;

    case SDNPXML2_SAVE_BIN_COUNTER:
#if SDNPDATA_SUPPORT_OBJ20
      _buildCounterGroup(pSDNPSession, &testLen, TMWDEFS_TRUE);
      pSDNPSession->xml2CurIndent = saveIndent;
      if (testLen>=maxLen)
      {
        break;
      }
      _buildCounterGroup(pSDNPSession, &curLen, TMWDEFS_FALSE);

#endif
      pSDNPSession->xml2SaveState = SDNPXML2_SAVE_ANALOG_IN;
      continue;
      break;

    case SDNPXML2_SAVE_ANALOG_IN:
#if SDNPDATA_SUPPORT_OBJ30
      _buildAnalogInputGroup(pSDNPSession, &testLen, TMWDEFS_TRUE);
      pSDNPSession->xml2CurIndent = saveIndent;
      if (testLen>=maxLen)
      {
        break;
      }
      _buildAnalogInputGroup(pSDNPSession, &curLen, TMWDEFS_FALSE);

#endif
      pSDNPSession->xml2SaveState = SDNPXML2_SAVE_ANALOG_OUT;
      continue;
      break;

 case SDNPXML2_SAVE_ANALOG_OUT:
#if SDNPDATA_SUPPORT_OBJ40
      _buildAnalogOutputGroup(pSDNPSession, &testLen, TMWDEFS_TRUE);
      pSDNPSession->xml2CurIndent = saveIndent;
      if (testLen>=maxLen)
      {
        break;
      }
      _buildAnalogOutputGroup(pSDNPSession, &curLen, TMWDEFS_FALSE);

#endif
      pSDNPSession->xml2SaveState = SDNPXML2_SAVE_FILE;
      continue;
      break;

 case SDNPXML2_SAVE_FILE:
#if SDNPDATA_SUPPORT_OBJ70
      _buildSequentialFileTransfer(pSDNPSession, &testLen, TMWDEFS_TRUE);

      pSDNPSession->xml2CurIndent = saveIndent;
      if (testLen>=maxLen)
      {
        break;
      }
      _buildSequentialFileTransfer(pSDNPSession, &curLen, TMWDEFS_FALSE);
#endif
      pSDNPSession->xml2SaveState = SDNPXML2_SAVE_STRING;
      continue;
      break;

 case SDNPXML2_SAVE_STRING:
#if SDNPDATA_SUPPORT_OBJ110
      _buildOctetStringGroup(pSDNPSession, &testLen, TMWDEFS_TRUE);
      pSDNPSession->xml2CurIndent = saveIndent;
      if (testLen>=maxLen)
      {
        break;
      }
      _buildOctetStringGroup(pSDNPSession, &curLen, TMWDEFS_FALSE);

#endif
      pSDNPSession->xml2SaveState = SDNPXML2_SAVE_VTERM;
      continue;
      break;

 case SDNPXML2_SAVE_VTERM:
#if SDNPDATA_SUPPORT_OBJ112
      _buildVirtualTerminalGroup(pSDNPSession, &testLen, TMWDEFS_TRUE);
      pSDNPSession->xml2CurIndent = saveIndent;
      if (testLen>=maxLen)
      {
        break;
      }
      _buildVirtualTerminalGroup(pSDNPSession, &curLen, TMWDEFS_FALSE);

#endif
      pSDNPSession->xml2SaveState = SDNPXML2_SAVE_DATABASE_TAIL;
      continue;
      break;

    case SDNPXML2_SAVE_DATABASE_TAIL:
      _buildDatabaseTail(pSDNPSession, &testLen, TMWDEFS_TRUE);
      pSDNPSession->xml2CurIndent = saveIndent;
      if (testLen>=maxLen)
      {
        break;
      }
      _buildDatabaseTail(pSDNPSession, &curLen, TMWDEFS_FALSE);
      pSDNPSession->xml2SaveState = SDNPXML2_SAVE_DATAPOINTS_LIST_HEAD;
      continue;
      break;

    case SDNPXML2_SAVE_DATAPOINTS_LIST_HEAD:
      _buildDataPointsListHeader(pSDNPSession,  &testLen, TMWDEFS_TRUE);
      pSDNPSession->xml2CurIndent = saveIndent;
      if (testLen>=maxLen)
      {
        break;
      }
      _buildDataPointsListHeader(pSDNPSession, &curLen, TMWDEFS_FALSE);
      pSDNPSession->xml2SaveState = SDNPXML2_SAVE_BIN_IN_POINTS_HEAD;
      continue;
      break;

    case SDNPXML2_SAVE_BIN_IN_POINTS_HEAD:
#if SDNPDATA_SUPPORT_OBJ1
      _buildBinaryInputPoints(pSDNPSession, &testLen, TMWDEFS_TRUE);
      pSDNPSession->xml2CurIndent = saveIndent;
      if (testLen>=maxLen)
      {
        break;
      }
      _buildBinaryInputPoints(pSDNPSession, &curLen, TMWDEFS_FALSE);

#endif
      pSDNPSession->xml2SaveState = SDNPXML2_SAVE_BIN_IN_POINTS;
      continue;
      break;

    case SDNPXML2_SAVE_BIN_IN_POINTS:
#if SDNPDATA_SUPPORT_OBJ1
      numPoints = sdnpdata_binInQuantity(pSDNPSession->pDbHandle);
      for(pointNum = pSDNPSession->xml2CurSavePointNum; pointNum < numPoints; pointNum++)
      {
        saveIndent = pSDNPSession->xml2CurIndent;
        _buildBinInPoint(pSDNPSession, pSDNPSession->pDbHandle, pointNum, &testLen, TMWDEFS_TRUE);
        pSDNPSession->xml2CurIndent = saveIndent;
        if (testLen>=maxLen)
        {
          break;
        }
        _buildBinInPoint(pSDNPSession, pSDNPSession->pDbHandle, pointNum, &curLen, TMWDEFS_FALSE);
        pSDNPSession->xml2CurSavePointNum = pointNum+1;
      }
      if (testLen>=maxLen)
      {
        break;
      }
      pSDNPSession->xml2CurSavePointNum = 0;
#endif
      pSDNPSession->xml2SaveState = SDNPXML2_SAVE_BIN_IN_POINTS_TAIL;
      continue;
      break;

    case SDNPXML2_SAVE_BIN_IN_POINTS_TAIL:
#if SDNPDATA_SUPPORT_OBJ1
      _buildBinaryInputPointsTail(pSDNPSession, &testLen, TMWDEFS_TRUE);
      pSDNPSession->xml2CurIndent = saveIndent;
      if (testLen>=maxLen)
      {
        break;
      }
      _buildBinaryInputPointsTail(pSDNPSession, &curLen, TMWDEFS_FALSE);
      /* For safety, always set it back to 2 */
      pSDNPSession->xml2CurIndent = 2;
#endif
      pSDNPSession->xml2SaveState = SDNPXML2_SAVE_DBL_IN_POINTS_HEAD;
      continue;
      break;

    case SDNPXML2_SAVE_DBL_IN_POINTS_HEAD:
#if SDNPDATA_SUPPORT_OBJ3
      _buildDoubleBitInputPoints(pSDNPSession, &testLen, TMWDEFS_TRUE);
      pSDNPSession->xml2CurIndent = saveIndent;
      if (testLen>=maxLen)
      {
        break;
      }
      _buildDoubleBitInputPoints(pSDNPSession, &curLen, TMWDEFS_FALSE);

#endif
      pSDNPSession->xml2SaveState = SDNPXML2_SAVE_DBL_IN_POINTS;
      continue;
      break;

    case SDNPXML2_SAVE_DBL_IN_POINTS:
#if SDNPDATA_SUPPORT_OBJ3
      numPoints = sdnpdata_dblInQuantity(pSDNPSession->pDbHandle);
      for(pointNum = pSDNPSession->xml2CurSavePointNum; pointNum < numPoints; pointNum++)
      {
        saveIndent = pSDNPSession->xml2CurIndent;
        _buildDblBitInPoint(pSDNPSession, pSDNPSession->pDbHandle, pointNum, &testLen, TMWDEFS_TRUE);
        pSDNPSession->xml2CurIndent = saveIndent;
        if (testLen>=maxLen)
        {
          break;
        }
        _buildDblBitInPoint(pSDNPSession, pSDNPSession->pDbHandle, pointNum, &curLen, TMWDEFS_FALSE);

        pSDNPSession->xml2CurSavePointNum = pointNum+1;
      }
      if (testLen>=maxLen)
      {
        break;
      }
      pSDNPSession->xml2CurSavePointNum = 0;
#endif
      pSDNPSession->xml2SaveState = SDNPXML2_SAVE_DBL_IN_POINTS_TAIL;
      continue;
      break;

    case SDNPXML2_SAVE_DBL_IN_POINTS_TAIL:
#if SDNPDATA_SUPPORT_OBJ3
      _buildDoubleBitInputPointsTail(pSDNPSession, &testLen, TMWDEFS_TRUE);
      pSDNPSession->xml2CurIndent = saveIndent;
      if (testLen>=maxLen)
      {
        break;
      }
      _buildDoubleBitInputPointsTail(pSDNPSession, &curLen, TMWDEFS_FALSE);
      /* For safety, always set it back to 2 */
      pSDNPSession->xml2CurIndent = 2;

#endif
      pSDNPSession->xml2SaveState = SDNPXML2_SAVE_BIN_OUT_POINTS_HEAD;
      continue;
      break;

    case SDNPXML2_SAVE_BIN_OUT_POINTS_HEAD:
#if SDNPDATA_SUPPORT_OBJ10
      _buildBinaryOutputPoints(pSDNPSession, &testLen, TMWDEFS_TRUE);
      pSDNPSession->xml2CurIndent = saveIndent;
      if (testLen>=maxLen)
      {
        break;
      }
      _buildBinaryOutputPoints(pSDNPSession, &curLen, TMWDEFS_FALSE);

#endif
      pSDNPSession->xml2SaveState = SDNPXML2_SAVE_BIN_OUT_POINTS;
      continue;
      break;

    case SDNPXML2_SAVE_BIN_OUT_POINTS:
#if SDNPDATA_SUPPORT_OBJ10
      numPoints = sdnpdata_binOutQuantity(pSDNPSession->pDbHandle);
      for(pointNum = pSDNPSession->xml2CurSavePointNum; pointNum < numPoints; pointNum++)
      {
        saveIndent = pSDNPSession->xml2CurIndent;
        _buildBinOutPoint(pSDNPSession,pSDNPSession->pDbHandle, pointNum, &testLen, TMWDEFS_TRUE);
        pSDNPSession->xml2CurIndent = saveIndent;
        if (testLen>=maxLen)
        {
          break;
        }
        _buildBinOutPoint(pSDNPSession, pSDNPSession->pDbHandle, pointNum, &curLen, TMWDEFS_FALSE);

        pSDNPSession->xml2CurSavePointNum = pointNum+1;
      }
      if (testLen>=maxLen)
      {
        break;
      }
      pSDNPSession->xml2CurSavePointNum = 0;
#endif
      pSDNPSession->xml2SaveState = SDNPXML2_SAVE_BIN_OUT_POINTS_TAIL;
      continue;
      break;

    case SDNPXML2_SAVE_BIN_OUT_POINTS_TAIL:
#if SDNPDATA_SUPPORT_OBJ10
      _buildBinaryOutputPointsTail(pSDNPSession, &testLen, TMWDEFS_TRUE);
      pSDNPSession->xml2CurIndent = saveIndent;
      if (testLen>=maxLen)
      {
        break;
      }
      _buildBinaryOutputPointsTail(pSDNPSession, &curLen, TMWDEFS_FALSE);
      /* For safety, always set it back to 2 */
      pSDNPSession->xml2CurIndent = 2;
#endif
      pSDNPSession->xml2SaveState = SDNPXML2_SAVE_BIN_COUNTER_POINTS_HEAD;
      continue;
      break;

    case SDNPXML2_SAVE_BIN_COUNTER_POINTS_HEAD:
#if SDNPDATA_SUPPORT_OBJ20
      _buildCounterPoints(pSDNPSession, &testLen, TMWDEFS_TRUE);
      pSDNPSession->xml2CurIndent = saveIndent;
      if (testLen>=maxLen)
      {
        break;
      }
      _buildCounterPoints(pSDNPSession, &curLen, TMWDEFS_FALSE);

#endif
      pSDNPSession->xml2SaveState = SDNPXML2_SAVE_BIN_COUNTER_POINTS;
      continue;
      break;

    case SDNPXML2_SAVE_BIN_COUNTER_POINTS:
#if SDNPDATA_SUPPORT_OBJ20
      numPoints = sdnpdata_binCntrQuantity(pSDNPSession->pDbHandle);
      for(pointNum = pSDNPSession->xml2CurSavePointNum; pointNum < numPoints; pointNum++)
      {
        saveIndent = pSDNPSession->xml2CurIndent;
        _buildCounterPoint(pSDNPSession,pSDNPSession->pDbHandle, pointNum, &testLen, TMWDEFS_TRUE);
        pSDNPSession->xml2CurIndent = saveIndent;
        if (testLen>=maxLen)
        {
          break;
        }
        _buildCounterPoint(pSDNPSession,pSDNPSession->pDbHandle, pointNum, &curLen, TMWDEFS_FALSE);

        pSDNPSession->xml2CurSavePointNum = pointNum+1;
      }
      if (testLen>=maxLen)
      {
        break;
      }
      pSDNPSession->xml2CurSavePointNum = 0;
#endif
      pSDNPSession->xml2SaveState = SDNPXML2_SAVE_BIN_COUNTER_POINTS_TAIL;
      continue;
      break;

    case SDNPXML2_SAVE_BIN_COUNTER_POINTS_TAIL:
#if SDNPDATA_SUPPORT_OBJ20
      _buildCounterPointsTail(pSDNPSession, &testLen, TMWDEFS_TRUE);
      pSDNPSession->xml2CurIndent = saveIndent;
      if (testLen>=maxLen)
      {
        break;
      }
      _buildCounterPointsTail(pSDNPSession, &curLen, TMWDEFS_FALSE);
      /* For safety, always set it back to 2 */
      pSDNPSession->xml2CurIndent = 2;
#endif
      pSDNPSession->xml2SaveState = SDNPXML2_SAVE_ANALOG_IN_POINTS_HEAD;
      continue;
      break;

    case SDNPXML2_SAVE_ANALOG_IN_POINTS_HEAD:
#if SDNPDATA_SUPPORT_OBJ30
      _buildAnalogInputPoints(pSDNPSession, &testLen, TMWDEFS_TRUE);
      pSDNPSession->xml2CurIndent = saveIndent;
      if (testLen>=maxLen)
      {
        break;
      }
      _buildAnalogInputPoints(pSDNPSession, &curLen, TMWDEFS_FALSE);

#endif
      pSDNPSession->xml2SaveState = SDNPXML2_SAVE_ANALOG_IN_POINTS;
      continue;
      break;

    case SDNPXML2_SAVE_ANALOG_IN_POINTS:
#if SDNPDATA_SUPPORT_OBJ30
      numPoints = sdnpdata_anlgInQuantity(pSDNPSession->pDbHandle);
      for(pointNum = pSDNPSession->xml2CurSavePointNum; pointNum < numPoints; pointNum++)
      {
        saveIndent = pSDNPSession->xml2CurIndent;
        _buildAnalogInPoint(pSDNPSession,pSDNPSession->pDbHandle, pointNum, &testLen, TMWDEFS_TRUE);
        pSDNPSession->xml2CurIndent = saveIndent;
        if (testLen>=maxLen)
        {
          break;
        }
        _buildAnalogInPoint(pSDNPSession,pSDNPSession->pDbHandle, pointNum, &curLen, TMWDEFS_FALSE);

        pSDNPSession->xml2CurSavePointNum = pointNum+1;
      }
      if (testLen>=maxLen)
      {
        break;
      }
      pSDNPSession->xml2CurSavePointNum = 0;
#endif
      pSDNPSession->xml2SaveState = SDNPXML2_SAVE_ANALOG_IN_POINTS_TAIL;
      continue;
      break;

    case SDNPXML2_SAVE_ANALOG_IN_POINTS_TAIL:
#if SDNPDATA_SUPPORT_OBJ30
      _buildAnalogInputPointsTail(pSDNPSession, &testLen, TMWDEFS_TRUE);
      pSDNPSession->xml2CurIndent = saveIndent;
      if (testLen>=maxLen)
      {
        break;
      }
      _buildAnalogInputPointsTail(pSDNPSession, &curLen, TMWDEFS_FALSE);
      /* For safety, always set it back to 2 */
      pSDNPSession->xml2CurIndent = 2;
#endif
      pSDNPSession->xml2SaveState = SDNPXML2_SAVE_ANALOG_OUT_POINTS_HEAD;
      continue;
      break;

 case SDNPXML2_SAVE_ANALOG_OUT_POINTS_HEAD:
#if SDNPDATA_SUPPORT_OBJ40
      _buildAnalogOutputPoints(pSDNPSession, &testLen, TMWDEFS_TRUE);
      pSDNPSession->xml2CurIndent = saveIndent;
      if (testLen>=maxLen)
      {
        break;
      }
      _buildAnalogOutputPoints(pSDNPSession, &curLen, TMWDEFS_FALSE);

#endif
      pSDNPSession->xml2SaveState = SDNPXML2_SAVE_ANALOG_OUT_POINTS;
      continue;
      break;

    case SDNPXML2_SAVE_ANALOG_OUT_POINTS:
#if SDNPDATA_SUPPORT_OBJ40
      numPoints = sdnpdata_anlgOutQuantity(pSDNPSession->pDbHandle);
      for(pointNum = pSDNPSession->xml2CurSavePointNum; pointNum < numPoints; pointNum++)
      {
        saveIndent = pSDNPSession->xml2CurIndent;
        _buildAnalogOutPoint(pSDNPSession,pSDNPSession->pDbHandle, pointNum, &testLen, TMWDEFS_TRUE);
        pSDNPSession->xml2CurIndent = saveIndent;
        if (testLen>=maxLen)
        {
          break;
        }
        _buildAnalogOutPoint(pSDNPSession,pSDNPSession->pDbHandle, pointNum, &curLen, TMWDEFS_FALSE);

        pSDNPSession->xml2CurSavePointNum = pointNum+1;
      }
      if (testLen>=maxLen)
      {
        break;
      }
      pSDNPSession->xml2CurSavePointNum = 0;
#endif
      pSDNPSession->xml2SaveState = SDNPXML2_SAVE_ANALOG_OUT_POINTS_TAIL;
      continue;
      break;

    case SDNPXML2_SAVE_ANALOG_OUT_POINTS_TAIL:
#if SDNPDATA_SUPPORT_OBJ40
      _buildAnalogOutputPointsTail(pSDNPSession, &testLen, TMWDEFS_TRUE);
      pSDNPSession->xml2CurIndent = saveIndent;
      if (testLen>=maxLen)
      {
        break;
      }
      _buildAnalogOutputPointsTail(pSDNPSession, &curLen, TMWDEFS_FALSE);
      /* For safety, always set it back to 2 */
      pSDNPSession->xml2CurIndent = 2;
#endif
      pSDNPSession->xml2SaveState = SDNPXML2_SAVE_FILES_HEAD;
      continue;
      break;

 case SDNPXML2_SAVE_FILES_HEAD:
#if SDNPDATA_SUPPORT_OBJ70
      _buildSequentialFiles(pSDNPSession, &testLen, TMWDEFS_TRUE);

      pSDNPSession->xml2CurIndent = saveIndent;
      if (testLen>=maxLen)
      {
        break;
      }
      _buildSequentialFiles(pSDNPSession, &curLen, TMWDEFS_FALSE);
#endif
      pSDNPSession->xml2SaveState = SDNPXML2_SAVE_FILES;
      continue;
      break;

    case SDNPXML2_SAVE_FILES:
#if SDNPDATA_SUPPORT_OBJ70
      pointNum = pSDNPSession->xml2CurSavePointNum;
      saveIndent = pSDNPSession->xml2CurIndent;
      while(_buildFilePoint(pSDNPSession, pointNum, &testLen, TMWDEFS_TRUE))
      {
        pSDNPSession->xml2CurIndent = saveIndent;
        if (testLen>=maxLen)
        {
          break;
        }
        _buildFilePoint(pSDNPSession, pointNum, &curLen, TMWDEFS_FALSE);

        saveIndent = pSDNPSession->xml2CurIndent;
        pointNum++;
        pSDNPSession->xml2CurSavePointNum = pointNum;
      }
      if (testLen>=maxLen)
      {
        break;
      }
      pSDNPSession->xml2CurSavePointNum = 0;
#endif
      pSDNPSession->xml2SaveState = SDNPXML2_SAVE_FILES_TAIL;
      continue;
      break;

    case SDNPXML2_SAVE_FILES_TAIL:
#if SDNPDATA_SUPPORT_OBJ70
      _buildSequentialFilesTail(pSDNPSession, &testLen, TMWDEFS_TRUE);
      pSDNPSession->xml2CurIndent = saveIndent;
      if (testLen>=maxLen)
      {
        break;
      }
      _buildSequentialFilesTail(pSDNPSession, &curLen, TMWDEFS_FALSE);
      /* For safety, always set it back to 2 */
      pSDNPSession->xml2CurIndent = 2;
#endif
      pSDNPSession->xml2SaveState = SDNPXML2_SAVE_STRING_POINTS_HEAD;
      continue;
      break;

 case SDNPXML2_SAVE_STRING_POINTS_HEAD:
#if SDNPDATA_SUPPORT_OBJ110
      _buildOctetStringPoints(pSDNPSession, &testLen, TMWDEFS_TRUE);
      pSDNPSession->xml2CurIndent = saveIndent;
      if (testLen>=maxLen)
      {
        break;
      }
      _buildOctetStringPoints(pSDNPSession, &curLen, TMWDEFS_FALSE);

#endif
      pSDNPSession->xml2SaveState = SDNPXML2_SAVE_STRING_POINTS;
      continue;
      break;

    case SDNPXML2_SAVE_STRING_POINTS:
#if SDNPDATA_SUPPORT_OBJ110
      numPoints = sdnpdata_strQuantity(pSDNPSession->pDbHandle);
      for(pointNum = pSDNPSession->xml2CurSavePointNum; pointNum < numPoints; pointNum++)
      {
        saveIndent = pSDNPSession->xml2CurIndent;
        _buildOctetStringPoint(pSDNPSession,pSDNPSession->pDbHandle, pointNum,  &testLen, TMWDEFS_TRUE);
        pSDNPSession->xml2CurIndent = saveIndent;
        if (testLen>=maxLen)
        {
          break;
        }
        _buildOctetStringPoint(pSDNPSession,pSDNPSession->pDbHandle, pointNum,  &curLen, TMWDEFS_FALSE);

        pSDNPSession->xml2CurSavePointNum = pointNum+1;
      }
      if (testLen>=maxLen)
      {
        break;
      }
      pSDNPSession->xml2CurSavePointNum = 0;
#endif
      pSDNPSession->xml2SaveState = SDNPXML2_SAVE_STRING_POINTS_TAIL;
      continue;
      break;

    case SDNPXML2_SAVE_STRING_POINTS_TAIL:
#if SDNPDATA_SUPPORT_OBJ110
      _buildOctetStringPointsTail(pSDNPSession, &testLen, TMWDEFS_TRUE);
      pSDNPSession->xml2CurIndent = saveIndent;
      if (testLen>=maxLen)
      {
        break;
      }
      _buildOctetStringPointsTail(pSDNPSession, &curLen, TMWDEFS_FALSE);
      /* For safety, always set it back to 2 */
      pSDNPSession->xml2CurIndent = 2;
#endif
      pSDNPSession->xml2SaveState = SDNPXML2_SAVE_VTERM_POINTS_HEAD;
      continue;
      break;

 case SDNPXML2_SAVE_VTERM_POINTS_HEAD:
#if SDNPDATA_SUPPORT_OBJ112
      _buildVirtualTerminalPoints(pSDNPSession, &testLen, TMWDEFS_TRUE);
      pSDNPSession->xml2CurIndent = saveIndent;
      if (testLen>=maxLen)
      {
        break;
      }
      _buildVirtualTerminalPoints(pSDNPSession, &curLen, TMWDEFS_FALSE);

#endif
      pSDNPSession->xml2SaveState = SDNPXML2_SAVE_VTERM_POINTS;
      continue;
      break;

    case SDNPXML2_SAVE_VTERM_POINTS:
#if SDNPDATA_SUPPORT_OBJ112
      numPoints = sdnpdata_vtermQuantity(pSDNPSession->pDbHandle);
      for(pointNum = pSDNPSession->xml2CurSavePointNum; pointNum < numPoints; pointNum++)
      {
        saveIndent = pSDNPSession->xml2CurIndent;
        _buildVirtualTerminalPoint(pSDNPSession,pSDNPSession->pDbHandle, pointNum, &testLen, TMWDEFS_TRUE);
        pSDNPSession->xml2CurIndent = saveIndent;
        if (testLen>=maxLen)
        {
          break;
        }
       _buildVirtualTerminalPoint(pSDNPSession,pSDNPSession->pDbHandle, pointNum, &curLen, TMWDEFS_FALSE);

        pSDNPSession->xml2CurSavePointNum = pointNum+1;
      }
      if (testLen>=maxLen)
      {
        break;
      }
      pSDNPSession->xml2CurSavePointNum = 0;
#endif
      pSDNPSession->xml2SaveState = SDNPXML2_SAVE_VTERM_POINTS_TAIL;
      continue;
      break;

    case SDNPXML2_SAVE_VTERM_POINTS_TAIL:
#if SDNPDATA_SUPPORT_OBJ112
      _buildVirtualTerminalPointsTail(pSDNPSession, &testLen, TMWDEFS_TRUE);
      pSDNPSession->xml2CurIndent = saveIndent;
      if (testLen>=maxLen)
      {
        break;
      }
      _buildVirtualTerminalPointsTail(pSDNPSession, &curLen, TMWDEFS_FALSE);
      /* For safety, always set it back to 2 */
      pSDNPSession->xml2CurIndent = 2;
#endif
      pSDNPSession->xml2SaveState = SDNPXML2_SAVE_DATASET_PROTO;
      continue;
      break;

    case SDNPXML2_SAVE_DATASET_PROTO:
#if SDNPDATA_SUPPORT_DATASETS
      _buildDatasetProto(pSDNPSession, &curLen, &testLen, maxLen);
      if(testLen>=maxLen)
        break;
#endif
      pSDNPSession->xml2SaveState = SDNPXML2_SAVE_DATASET_DESCR;
      pSDNPSession->xml2CurSavePointNum = 0;
      continue;
      break;

    case SDNPXML2_SAVE_DATASET_DESCR:
#if SDNPDATA_SUPPORT_DATASETS
      _buildDatasetDescr(pSDNPSession, &curLen, &testLen, maxLen);
      if(testLen>=maxLen)
        break;
#endif
      pSDNPSession->xml2SaveState = SDNPXML2_SAVE_DATAPOINTS_LIST_TAIL;
      pSDNPSession->xml2CurSavePointNum = 0;
      continue;

    case SDNPXML2_SAVE_DATAPOINTS_LIST_TAIL:
      _buildDataPointsListTail(pSDNPSession,  &testLen, TMWDEFS_TRUE);
      pSDNPSession->xml2CurIndent = saveIndent;
      if (testLen>=maxLen)
      {
        break;
      }
      _buildDataPointsListTail(pSDNPSession, &curLen, TMWDEFS_FALSE);
      pSDNPSession->xml2SaveState = SDNPXML2_SAVE_DOC_TAIL;
      continue;
      break;

    case SDNPXML2_SAVE_DOC_TAIL:
      _buildDocTail(pSDNPSession, &testLen, TMWDEFS_TRUE);
      pSDNPSession->xml2CurIndent = saveIndent;
      if (testLen>=maxLen)
      {
        break;
      }
      _buildDocTail(pSDNPSession, &curLen, TMWDEFS_FALSE);

      pSDNPSession->xml2BufferLen = curLen;
      return SDNPXML2_SAVE_DONE;
      break;

    default:
      return SDNPXML2_SAVE_FAILED;
    }

    pSDNPSession->xml2BufferLen = curLen;
    if (testLen >= maxLen)
    {
      if((prevState == pSDNPSession->xml2SaveState)
        &&(prevSubState == pSDNPSession->xml2SaveSubState)
        &&(prevPointNum == pSDNPSession->xml2CurSavePointNum)
        &&(prevElemNum == pSDNPSession->xml2CurSaveElemNum))
      {
        /* since this single object won't fit by itself, return failure */
        SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_XML_SIZE_ERROR);
        return SDNPXML2_SAVE_FAILED;
      }
      return SDNPXML2_SAVE_NOT_DONE;
    }
  }

  return SDNPXML2_SAVE_DONE;
}

TMWTYPES_ULONG TMWDEFS_GLOBAL sdnpxml2_saveDatabaseGetSize(TMWSESN *pSession)
{
  SDNPSESN *pSDNPSession;
  TMWTYPES_ULONG curLen;
  TMWTYPES_USHORT numPoints;
  TMWTYPES_USHORT pointNum;

  pSDNPSession = (SDNPSESN *)pSession;
  pSDNPSession->xml2CurIndent = 0;
  pSDNPSession->xml2CurSavePointNum = 0;
  pSDNPSession->xml2CurSaveElemNum = 0;
  curLen=0;

  _buildDocHeader(pSDNPSession, &curLen, TMWDEFS_TRUE);

  _buildConfigurationHeader(pSDNPSession, &curLen, TMWDEFS_TRUE);

  _buildDeviceConfig(pSDNPSession, &curLen, TMWDEFS_TRUE);

  _buildSerialConfig(pSDNPSession, &curLen, TMWDEFS_TRUE);

  _buildNetworkConfig(pSDNPSession, &curLen, TMWDEFS_TRUE);

  _buildLinkConfig(pSDNPSession, &curLen, TMWDEFS_TRUE);

  _buildApplConfig(pSDNPSession, &curLen, TMWDEFS_TRUE);

  _buildOutstationConfig(pSDNPSession, &curLen, TMWDEFS_TRUE);

  _buildUnsolicitedConfig(pSDNPSession, &curLen, TMWDEFS_TRUE);

  _buildUnsolTriggerConfig(pSDNPSession, &curLen, TMWDEFS_TRUE);

  _buildOutperformConfig(pSDNPSession, &curLen, TMWDEFS_TRUE);

  _buildFieldConfig(pSDNPSession, &curLen, TMWDEFS_TRUE);

  _buildSecurityConfig(pSDNPSession, &curLen, TMWDEFS_TRUE);

  _buildBroadcastConfig(pSDNPSession, &curLen, TMWDEFS_TRUE);

  _buildConfigurationTail(pSDNPSession, &curLen, TMWDEFS_TRUE);

  _buildDatabaseHeader(pSDNPSession,  &curLen, TMWDEFS_TRUE);

#if SDNPDATA_SUPPORT_OBJ1
  _buildBinaryInputGroup(pSDNPSession, &curLen, TMWDEFS_TRUE);
#endif

#if SDNPDATA_SUPPORT_OBJ3
  _buildDoubleBitInputGroup(pSDNPSession, &curLen, TMWDEFS_TRUE);
#endif

#if SDNPDATA_SUPPORT_OBJ10
  _buildBinaryOutputGroup(pSDNPSession, &curLen, TMWDEFS_TRUE);
#endif

#if SDNPDATA_SUPPORT_OBJ20
  _buildCounterGroup(pSDNPSession, &curLen, TMWDEFS_TRUE);
#endif

#if SDNPDATA_SUPPORT_OBJ30
  _buildAnalogInputGroup(pSDNPSession, &curLen, TMWDEFS_TRUE);
#endif

#if SDNPDATA_SUPPORT_OBJ40
  _buildAnalogOutputGroup(pSDNPSession, &curLen, TMWDEFS_TRUE);
#endif

#if SDNPDATA_SUPPORT_OBJ70
  _buildSequentialFileTransfer(pSDNPSession, &curLen, TMWDEFS_TRUE);
#endif

#if SDNPDATA_SUPPORT_OBJ110
  _buildOctetStringGroup(pSDNPSession, &curLen, TMWDEFS_TRUE);
#endif

#if SDNPDATA_SUPPORT_OBJ112
  _buildVirtualTerminalGroup(pSDNPSession, &curLen, TMWDEFS_TRUE);
#endif

  _buildDatabaseTail(pSDNPSession, &curLen, TMWDEFS_TRUE);

  _buildDataPointsListHeader(pSDNPSession, &curLen, TMWDEFS_TRUE);

#if SDNPDATA_SUPPORT_OBJ1
  _buildBinaryInputPoints(pSDNPSession, &curLen, TMWDEFS_TRUE);

  numPoints = sdnpdata_binInQuantity(pSDNPSession->pDbHandle);
  for(pointNum = 0; pointNum < numPoints; pointNum++)
  {
    _buildBinInPoint(pSDNPSession,  pSDNPSession->pDbHandle, pointNum, &curLen, TMWDEFS_TRUE);
  }
  _buildBinaryInputPointsTail(pSDNPSession, &curLen, TMWDEFS_TRUE);
#endif

#if SDNPDATA_SUPPORT_OBJ3
  _buildDoubleBitInputPoints(pSDNPSession, &curLen, TMWDEFS_TRUE);

  numPoints = sdnpdata_dblInQuantity(pSDNPSession->pDbHandle);
  for(pointNum = 0; pointNum < numPoints; pointNum++)
  {
    _buildDblBitInPoint(pSDNPSession, pSDNPSession->pDbHandle, pointNum, &curLen, TMWDEFS_TRUE);
  }
  _buildDoubleBitInputPoints(pSDNPSession, &curLen, TMWDEFS_TRUE);
#endif

#if SDNPDATA_SUPPORT_OBJ10
  _buildBinaryOutputPoints(pSDNPSession, &curLen, TMWDEFS_TRUE);
  numPoints = sdnpdata_binOutQuantity(pSDNPSession->pDbHandle);
  for(pointNum = 0; pointNum < numPoints; pointNum++)
  {
    _buildBinOutPoint(pSDNPSession,pSDNPSession->pDbHandle, pointNum, &curLen, TMWDEFS_TRUE);
  }
  _buildBinaryOutputPointsTail(pSDNPSession, &curLen, TMWDEFS_TRUE);
#endif

#if SDNPDATA_SUPPORT_OBJ20
  _buildCounterPoints(pSDNPSession, &curLen, TMWDEFS_TRUE);
  numPoints = sdnpdata_binCntrQuantity(pSDNPSession->pDbHandle);
  for(pointNum = 0; pointNum < numPoints; pointNum++)
  {
    _buildCounterPoint(pSDNPSession, pSDNPSession->pDbHandle, pointNum, &curLen, TMWDEFS_TRUE);
  }
  _buildCounterPointsTail(pSDNPSession, &curLen, TMWDEFS_TRUE);
#endif

#if SDNPDATA_SUPPORT_OBJ30
  _buildAnalogInputPoints(pSDNPSession, &curLen, TMWDEFS_TRUE);

  numPoints = sdnpdata_anlgInQuantity(pSDNPSession->pDbHandle);
  for(pointNum = 0; pointNum < numPoints; pointNum++)
  {
    _buildAnalogInPoint(pSDNPSession,  pSDNPSession->pDbHandle, pointNum, &curLen, TMWDEFS_TRUE);
  }
  _buildAnalogInputPointsTail(pSDNPSession, &curLen, TMWDEFS_TRUE);
#endif

#if SDNPDATA_SUPPORT_OBJ40
  _buildAnalogOutputPoints(pSDNPSession, &curLen, TMWDEFS_TRUE);
  numPoints = sdnpdata_anlgOutQuantity(pSDNPSession->pDbHandle);
  for(pointNum = 0; pointNum < numPoints; pointNum++)
  {
    _buildAnalogOutPoint(pSDNPSession, pSDNPSession->pDbHandle, pointNum, &curLen, TMWDEFS_TRUE);
  }
  _buildAnalogOutputPointsTail(pSDNPSession, &curLen, TMWDEFS_TRUE);
#endif

#if SDNPDATA_SUPPORT_OBJ70
  _buildSequentialFiles(pSDNPSession, &curLen, TMWDEFS_TRUE);

  pointNum = 0;
  while(_buildFilePoint(pSDNPSession, pointNum, &curLen, TMWDEFS_TRUE))
  {
    pointNum++;
  }

  _buildSequentialFilesTail(pSDNPSession, &curLen, TMWDEFS_TRUE);
#endif

#if SDNPDATA_SUPPORT_OBJ110
  _buildOctetStringPoints(pSDNPSession, &curLen, TMWDEFS_TRUE);

  numPoints = sdnpdata_strQuantity(pSDNPSession->pDbHandle);
  for(pointNum = 0; pointNum < numPoints; pointNum++)
  {
    _buildOctetStringPoint(pSDNPSession, pSDNPSession->pDbHandle, pointNum, &curLen, TMWDEFS_TRUE);
  }

  _buildOctetStringPointsTail(pSDNPSession, &curLen, TMWDEFS_TRUE);
#endif

#if SDNPDATA_SUPPORT_OBJ112
  _buildVirtualTerminalPoints(pSDNPSession, &curLen, TMWDEFS_TRUE);

  numPoints = sdnpdata_vtermQuantity(pSDNPSession->pDbHandle);
  for(pointNum = 0; pointNum < numPoints; pointNum++)
  {
    _buildVirtualTerminalPoint(pSDNPSession, pSDNPSession->pDbHandle, pointNum, &curLen, TMWDEFS_TRUE);
  }

  _buildVirtualTerminalPointsTail(pSDNPSession, &curLen, TMWDEFS_TRUE);
#endif

#if SDNPDATA_SUPPORT_DATASETS
  _buildDatasetProtoGetLen(pSDNPSession, &curLen);
  _buildDatasetDescrGetLen(pSDNPSession, &curLen);
#endif

  _buildDataPointsListTail(pSDNPSession, &curLen, TMWDEFS_TRUE);

  _buildDocTail(pSDNPSession, &curLen, TMWDEFS_TRUE);

  return(curLen);
}

void TMWDEFS_GLOBAL sdnpxml2_saveDatabaseInit(TMWSESN *pSession)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  pSDNPSession->xml2IndentSize = 0;
  pSDNPSession->xml2CurIndent = 0;
  pSDNPSession->xml2BufferLen = 0;
  pSDNPSession->xml2BufferIndex = 0;
  pSDNPSession->xml2CurSavePointNum = 0;
  pSDNPSession->xml2CurSaveElemNum = 0;
  pSDNPSession->xml2SaveState = SDNPXML2_SAVE_DOC_HEADER;
  pSDNPSession->xml2SaveSubState = SDNPXML2_SAVE_DATASETSTART;
}

#if SDNPDATA_SUPPORT_OBJ70 && SDNPDATA_SUPPORT_READ_XML
/* function: sdnpxml2_openDeviceProfile */
DNPDEFS_FILE_CMD_STAT TMWDEFS_GLOBAL sdnpxml2_openDeviceProfile(
  TMWSESN *pSession,
  TMWTYPES_CHAR *pFilename,
  TMWTYPES_ULONG authKey,
  DNPDEFS_FILE_MODE mode,
  TMWDTIME *pTimeOfCreation,
  TMWTYPES_ULONG *pFileHandle,
  TMWTYPES_ULONG *pSize,
  DNPDEFS_FILE_TYPE *pType)
{
  TMWTYPES_BOOL authKeyOK;

  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  SDNPO070_FILE_CONTROL_BLOCK *pObj70FileCtrl = (SDNPO070_FILE_CONTROL_BLOCK *)pSDNPSession->pObj70FileCtrl;

  if(sdnpdata_deviceProfileFile(pSDNPSession->pDbHandle, pFilename, authKey, &authKeyOK))
  {
    if(!authKeyOK)
    {
      return(DNPDEFS_FILE_CMD_STAT_DENIED);
    }

    /* can only read the device profile */
    if (mode != DNPDEFS_FILE_MODE_READ)
    {
      return(DNPDEFS_FILE_CMD_STAT_INV_MODE);
    }

    pSDNPSession->xml2IndentSize = 0;
    *pSize = sdnpxml2_saveDatabaseGetSize(pSession);
    if(*pSize == 0)
    {
      /* Return error */
      return(DNPDEFS_FILE_CMD_STAT_NOT_FOUND);
    }
    sdnpxml2_saveDatabaseInit(pSession);
    pObj70FileCtrl->xml2DeviceProfileOpen = TMWDEFS_TRUE;

    /* do a better job of choosing handle */
    pObj70FileCtrl->handle+=333;
    *pFileHandle = pObj70FileCtrl->handle;

    *pType = DNPDEFS_FILE_TYPE_SIMPLE;

    tmwdtime_getDateTime(pSession, pTimeOfCreation);

    /* Return success */
    return(DNPDEFS_FILE_CMD_STAT_SUCCESS);
  }

  /* Return error */
  return(DNPDEFS_FILE_CMD_STAT_NOT_FOUND);
}

/* function: sdnpxml2_readDeviceProfile */
DNPDEFS_FILE_TFER_STAT TMWDEFS_GLOBAL sdnpxml2_readDeviceProfile(
  TMWSESN *pSession,
  TMWTYPES_ULONG fileHandle,
  TMWTYPES_BOOL *pLast,
  TMWTYPES_USHORT *pBytesRead,
  TMWTYPES_UCHAR *pBuf)
{
  TMWTYPES_UINT memCpyLen;
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  SDNPO070_FILE_CONTROL_BLOCK *pObj70FileCtrl = (SDNPO070_FILE_CONTROL_BLOCK *)pSDNPSession->pObj70FileCtrl;

  /* Validate file state against request */
  if(fileHandle != pObj70FileCtrl->handle)
    return(DNPDEFS_FILE_TFER_STAT_INV_HANDLE);

  do
  {
    if (pSDNPSession->xml2BufferLen == 0)
    {
      /* For File Transfer read of Device Profile, set indent size to zero */
      pSDNPSession->xml2IndentSize = 0;
      pSDNPSession->xml2BufferState = sdnpxml2_saveDatabase(pSession, pSDNPSession->xml2Buffer, SDNPCNFG_XML_SAVE_BUF_SIZE);
      if(pSDNPSession->xml2BufferState == SDNPXML2_SAVE_FAILED)
        return(DNPDEFS_FILE_TFER_STAT_MISC);
    }

    if((pSDNPSession->xml2BufferIndex + pObj70FileCtrl->maxBlockSize) <= pSDNPSession->xml2BufferLen)
    {
      memCpyLen = pObj70FileCtrl->maxBlockSize;
      *pLast = TMWDEFS_FALSE;
    }
    else
    {
      memCpyLen = pSDNPSession->xml2BufferLen - pSDNPSession->xml2BufferIndex;
      if (pSDNPSession->xml2BufferState == SDNPXML2_SAVE_DONE)
      {
        *pLast = TMWDEFS_TRUE;
      }
      else
      {
        *pLast = TMWDEFS_FALSE;
      }
    }
    if (memCpyLen == 0)
    {
      pSDNPSession->xml2BufferIndex = 0;
      pSDNPSession->xml2BufferLen = 0;
      memset(pSDNPSession->xml2Buffer,0,SDNPCNFG_XML_SAVE_BUF_SIZE);
    }
  }
  while(memCpyLen == 0);

  memcpy(pBuf, &pSDNPSession->xml2Buffer[pSDNPSession->xml2BufferIndex], memCpyLen);
  pSDNPSession->xml2BufferIndex += memCpyLen;
  if (memCpyLen < pObj70FileCtrl->maxBlockSize)
  {
    pSDNPSession->xml2BufferIndex = 0;
    pSDNPSession->xml2BufferLen = 0;
    memset(pSDNPSession->xml2Buffer,0,SDNPCNFG_XML_SAVE_BUF_SIZE);
  }
  *pBytesRead = (TMWTYPES_USHORT)memCpyLen;

  /* Return success */
  return(DNPDEFS_FILE_TFER_STAT_SUCCESS);
}

/* function: sdnpxml2_closeDeviceProfile */
DNPDEFS_FILE_CMD_STAT TMWDEFS_GLOBAL sdnpxml2_closeDeviceProfile(
  TMWSESN *pSession,
  TMWTYPES_ULONG fileHandle)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  SDNPO070_FILE_CONTROL_BLOCK *pObj70FileCtrl = (SDNPO070_FILE_CONTROL_BLOCK *)pSDNPSession->pObj70FileCtrl;

  /* Validate file state */
  if(fileHandle != pObj70FileCtrl->handle)
    return(DNPDEFS_FILE_CMD_STAT_INV_HANDLE);

  memset(pSDNPSession->xml2Buffer,0,SDNPCNFG_XML_SAVE_BUF_SIZE);
  pSDNPSession->xml2BufferLen = 0;
  pSDNPSession->xml2BufferIndex = 0;
  pSDNPSession->xml2BufferState = SDNPXML2_SAVE_NOT_DONE;
  pObj70FileCtrl->xml2DeviceProfileOpen = TMWDEFS_FALSE;

  /* Return success */
  return(DNPDEFS_FILE_CMD_STAT_SUCCESS);
}
#endif

TMWTYPES_BOOL sdnpxml2_saveDatabaseToFile(TMWSESN *pSession, char *pFileName)
{
  SDNPSESN* pSDNPSession;
  SDNPXML2_SAVE_STATE state;
  TMWTYPES_BOOL status;
  TMWDTIME timeOfCreation;
  TMWTYPES_ULONG handle;
  TMWTYPES_ULONG size;
  DNPDEFS_FILE_TYPE type;
  TMWTYPES_USHORT blockSize;
  TMWTYPES_USHORT permissions;

  permissions = 0x1ff;
  blockSize = SDNPCNFG_XML_SAVE_BUF_SIZE;
  /* A number of these arguments we don't care about, but this allows us to reuse the sdnpdata_openFile interface */
  if(DNPDEFS_FILE_CMD_STAT_SUCCESS != sdnpdata_openFile(pSession, pFileName, 0, DNPDEFS_FILE_MODE_WRITE, &blockSize, &permissions, &timeOfCreation, &handle, &size, &type))
    return TMWDEFS_FALSE;

  pSDNPSession = (SDNPSESN *)pSession;

  /* when writing to a file set indent size to 2 */
  sdnpxml2_saveDatabaseInit(pSession);
  pSDNPSession->xml2IndentSize = 2;

  status = TMWDEFS_TRUE;
  /* Loop till a break is reached */
  while(status)
  {
    state = sdnpxml2_saveDatabase(pSession, pSDNPSession->xml2Buffer, SDNPCNFG_XML_SAVE_BUF_SIZE);
    if(state == SDNPXML2_SAVE_FAILED)
    {
      /* Failed to create device profile */
      status = TMWDEFS_FALSE;
      break;
    }

    else if(state == SDNPXML2_SAVE_NOT_DONE)
    {
      /* Write this to a file, buffer contains a null terminated string. */
      if(DNPDEFS_FILE_TFER_STAT_SUCCESS != sdnpdata_writeFile(pSession, handle, TMWDEFS_FALSE, (TMWTYPES_USHORT)strlen(pSDNPSession->xml2Buffer), (TMWTYPES_UCHAR *)pSDNPSession->xml2Buffer))
      {
        status = TMWDEFS_FALSE;
        break;
      }
    }
    else /* SDNPXML2_SAVE_DONE */
    {
      /* Write this to a file, buffer contains a null terminated string. */
      if(DNPDEFS_FILE_TFER_STAT_SUCCESS != sdnpdata_writeFile(pSession, handle, TMWDEFS_TRUE, (TMWTYPES_USHORT)strlen(pSDNPSession->xml2Buffer), (TMWTYPES_UCHAR *)pSDNPSession->xml2Buffer))
      {
        status = TMWDEFS_FALSE;
      }

      /* Writing device profile is complete */
      break;
    }
  }

  sdnpdata_closeFile(pSession, handle);
  return status;
}

#endif /* SDNPDATA_SUPPORT_XML2 */
