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

/* file: sdnpxml.c
 * description:  TMW private format xml code for writing configuration used by test harness
 */
#include "tmwscl/dnp/dnpdiag.h"
#include "tmwscl/utils/tmwtarg.h"
#include "tmwscl/utils/tmwcnfg.h"
#include "tmwscl/utils/tmwdefs.h"
#include "tmwscl/dnp/sdnpdata.h"
#include "tmwscl/dnp/sdnputil.h"
#include "tmwscl/dnp/sdnpcnfg.h"
#include "tmwscl/dnp/sdnpdiag.h"

#if TMWCNFG_USE_SIMULATED_DB
#include "tmwscl/dnp/sdnpsim.h"

#if SDNPDATA_SUPPORT_XML

/* function: _appendStringLen */
static TMWTYPES_CHAR * TMWDEFS_GLOBAL _appendStringLen(
  TMWTYPES_CHAR  *pStr1, 
  TMWTYPES_CHAR  *pStr2,
  TMWTYPES_ULONG *curLen,
  TMWTYPES_BOOL   bGetLen)
{
  TMWTYPES_CHAR *result;
  TMWTYPES_ULONG length;

  if (bGetLen == TMWDEFS_FALSE)
  {
    result = strcat(pStr1, pStr2);
    return result;
  }
  else
  {
    length = (TMWTYPES_ULONG)strlen(pStr2) + *curLen;
    *curLen = length;
  }
  return TMWDEFS_NULL;
}

static TMWTYPES_CHAR * TMWDEFS_LOCAL _buildHeader(TMWTYPES_CHAR *result, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  result = _appendStringLen(result, "<?xml version=\"1.0\"?>\n", curLen, bGetLen);
  result = _appendStringLen(result, "<tmw:dnpdata\n", curLen, bGetLen);
  result = _appendStringLen(result, " xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n", curLen, bGetLen);
  result = _appendStringLen(result, " xmlns:tmw=\"http://www.TriangleMicroWorks.com/TestHarness/Schemas/dnpdata\">\n", curLen, bGetLen);
  result = _appendStringLen(result, " <device>\n", curLen, bGetLen);
  result = _appendStringLen(result, "  <deviceAttributeGroup>\n", curLen, bGetLen);
  return result;
}

static TMWTYPES_CHAR * TMWDEFS_LOCAL _buildTail(TMWTYPES_CHAR *result, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  result = _appendStringLen(result, "  </authSecStatGroup>\n", curLen, bGetLen);
  result = _appendStringLen(result, " </device>\n", curLen, bGetLen);
  result = _appendStringLen(result, "</tmw:dnpdata>\n", curLen, bGetLen);
  return result;
}

static TMWTYPES_CHAR * TMWDEFS_LOCAL _buildDeviceAttrTail(TMWTYPES_CHAR *result, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  result = _appendStringLen(result, "  </deviceAttributeGroup>\n", curLen, bGetLen);
  result = _appendStringLen(result, "  <binaryInputGroup>\n", curLen, bGetLen);
  return result;
}

static TMWTYPES_CHAR * TMWDEFS_LOCAL _buildBinInTail(TMWTYPES_CHAR *result, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  result = _appendStringLen(result, "  </binaryInputGroup>\n", curLen, bGetLen);
  result = _appendStringLen(result, "  <doubleBitInputGroup>\n", curLen, bGetLen);
  return result;
}

static TMWTYPES_CHAR * TMWDEFS_LOCAL _buildDblBitInTail(TMWTYPES_CHAR *result, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  result = _appendStringLen(result, "  </doubleBitInputGroup>\n", curLen, bGetLen);
  result = _appendStringLen(result, "  <binaryOutputControlGroup>\n", curLen, bGetLen);
  return result;
}

static TMWTYPES_CHAR * TMWDEFS_LOCAL _buildBinOutTail(TMWTYPES_CHAR *result, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  result = _appendStringLen(result, "  </binaryOutputControlGroup>\n", curLen, bGetLen);
  result = _appendStringLen(result, "  <binaryOutputStatusGroup>\n", curLen, bGetLen);
  return result;
}

static TMWTYPES_CHAR * TMWDEFS_LOCAL _buildBinOutStatusTail(TMWTYPES_CHAR *result, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  result = _appendStringLen(result, "  </binaryOutputStatusGroup>\n", curLen, bGetLen);
  result = _appendStringLen(result, "  <binaryCounterGroup>\n", curLen, bGetLen);
  return result;
}

static TMWTYPES_CHAR * TMWDEFS_LOCAL _buildBinCounterTail(TMWTYPES_CHAR *result, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  result = _appendStringLen(result, "   </binaryCounterGroup>\n", curLen, bGetLen);
  result = _appendStringLen(result, "  <frozenCounterGroup>\n", curLen, bGetLen);
  return result;
}

static TMWTYPES_CHAR * TMWDEFS_LOCAL _buildFrozenCounterTail(TMWTYPES_CHAR *result, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  result = _appendStringLen(result, "   </frozenCounterGroup>\n", curLen, bGetLen);
  result = _appendStringLen(result, "  <analogInputGroup>\n", curLen, bGetLen);
  return result;
}

static TMWTYPES_CHAR * TMWDEFS_LOCAL _buildAnalogInTail(TMWTYPES_CHAR *result, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  result = _appendStringLen(result, "  </analogInputGroup>\n", curLen, bGetLen);
  result = _appendStringLen(result, "  <frznAnalogInputGroup>\n", curLen, bGetLen);
  return result;
}

static TMWTYPES_CHAR * TMWDEFS_LOCAL _buildFrznAnalogInTail(TMWTYPES_CHAR *result, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  result = _appendStringLen(result, "  </frznAnalogInputGroup>\n", curLen, bGetLen);
  result = _appendStringLen(result, "  <analogInputDeadbandGroup>\n", curLen, bGetLen);
  return result;
}

static TMWTYPES_CHAR * TMWDEFS_LOCAL _buildAnalogInDbandTail(TMWTYPES_CHAR *result, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  result = _appendStringLen(result, "  </analogInputDeadbandGroup>\n", curLen, bGetLen);
  result = _appendStringLen(result, "  <analogOutputControlGroup>\n", curLen, bGetLen);
  return result;
}

static TMWTYPES_CHAR * TMWDEFS_LOCAL _buildAnalogOutTail(TMWTYPES_CHAR *result, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  result = _appendStringLen(result, "  </analogOutputControlGroup>\n", curLen, bGetLen);
  result = _appendStringLen(result, "  <analogOutputStatusGroup>\n", curLen, bGetLen);
  return result;
}

static TMWTYPES_CHAR * TMWDEFS_LOCAL _buildAnalogOutStatusTail(TMWTYPES_CHAR *result, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  result = _appendStringLen(result, "  </analogOutputStatusGroup>\n", curLen, bGetLen);
  result = _appendStringLen(result, "  <indexedTimeGroup>\n", curLen, bGetLen);
  return result;
}

static TMWTYPES_CHAR * TMWDEFS_LOCAL _buildIndexedTimeTail(TMWTYPES_CHAR *result, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  result = _appendStringLen(result, "  </indexedTimeGroup>\n", curLen, bGetLen);
  result = _appendStringLen(result, "  <dataSetProtoGroup>\n", curLen, bGetLen);
  return result;
}

static TMWTYPES_CHAR * TMWDEFS_LOCAL _buildDatasetProtoTail(TMWTYPES_CHAR *result, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  result = _appendStringLen(result, "  </dataSetProtoGroup>\n", curLen, bGetLen);
  result = _appendStringLen(result, "  <dataSetDescrGroup>\n", curLen, bGetLen);
  return result;
}

static TMWTYPES_CHAR * TMWDEFS_LOCAL _buildDatasetDescrTail(TMWTYPES_CHAR *result, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  result = _appendStringLen(result, "  </dataSetDescrGroup>\n", curLen, bGetLen);
  result = _appendStringLen(result, "  <dataSetDataGroup>\n", curLen, bGetLen);
  return result;
}

static TMWTYPES_CHAR * TMWDEFS_LOCAL _buildDatasetDataTail(TMWTYPES_CHAR *result, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  result = _appendStringLen(result, "  </dataSetDataGroup>\n", curLen, bGetLen);
  result = _appendStringLen(result, "  <stringGroup>\n", curLen, bGetLen);
  return result;
}

static TMWTYPES_CHAR * TMWDEFS_LOCAL _buildStringTail(TMWTYPES_CHAR *result, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  result = _appendStringLen(result, "  </stringGroup>\n", curLen, bGetLen);
  result = _appendStringLen(result, "  <virtualTerminalGroup>\n", curLen, bGetLen);
  return result;
}

static TMWTYPES_CHAR * TMWDEFS_LOCAL _buildVtermTail(TMWTYPES_CHAR *result, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  result = _appendStringLen(result, "  </virtualTerminalGroup>\n", curLen, bGetLen);
  result = _appendStringLen(result, "  <authSecStatGroup>\n", curLen, bGetLen);
  return result;
}

static TMWTYPES_CHAR * TMWDEFS_LOCAL _putBoolField(TMWTYPES_CHAR *result, TMWTYPES_CHAR *prefix, TMWTYPES_CHAR *name, TMWTYPES_BOOL flag, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  TMWTYPES_CHAR buf[128];
  tmwtarg_snprintf(buf, sizeof(buf), "%s<%s>%s</%s>\n", prefix, name, flag ? "true" : "false", name);
  return _appendStringLen(result, buf, curLen, bGetLen);
}

static TMWTYPES_CHAR * TMWDEFS_LOCAL _putClass(TMWTYPES_CHAR *result, TMWDEFS_CLASS_MASK mask, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  result = _putBoolField(result, "    ", "notInClass0", TMWDEFS_TOBOOL(mask, TMWDEFS_CLASS_MASK_NOTCLASS0), curLen, bGetLen);
  result = _putBoolField(result, "    ", "class1Events", TMWDEFS_TOBOOL(mask, TMWDEFS_CLASS_MASK_ONE), curLen, bGetLen);
  result = _putBoolField(result, "    ", "class2Events", TMWDEFS_TOBOOL(mask, TMWDEFS_CLASS_MASK_TWO), curLen, bGetLen);
  result = _putBoolField(result, "    ", "class3Events", TMWDEFS_TOBOOL(mask, TMWDEFS_CLASS_MASK_THREE), curLen, bGetLen);
  return result;
}

static TMWTYPES_CHAR * TMWDEFS_LOCAL _putControl(TMWTYPES_CHAR *result, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  result = _putBoolField(result, "    ", "supportSelectOperate", TMWDEFS_TRUE, curLen, bGetLen);
  result = _putBoolField(result, "    ", "supportDirectOperate", TMWDEFS_TRUE, curLen, bGetLen);
  result = _putBoolField(result, "    ", "supportDirectOperateNoAck", TMWDEFS_TRUE, curLen, bGetLen);
  return result;
}

#if SDNPDATA_SUPPORT_OBJ12
static TMWTYPES_CHAR * TMWDEFS_LOCAL _putBinControl(TMWTYPES_CHAR *result, SDNPDATA_CROB_CTRL mask, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  result = _putBoolField(result, "    ", "supportPulseOn", TMWDEFS_TOBOOL(mask, SDNPDATA_CROB_CTRL_PULSE_ON), curLen, bGetLen);
  result = _putBoolField(result, "    ", "supportPulseOff", TMWDEFS_TOBOOL(mask, SDNPDATA_CROB_CTRL_PULSE_OFF), curLen, bGetLen);
  result = _putBoolField(result, "    ", "supportLatchOn", TMWDEFS_TOBOOL(mask, SDNPDATA_CROB_CTRL_LATCH_ON), curLen, bGetLen);
  result = _putBoolField(result, "    ", "supportLatchOff", TMWDEFS_TOBOOL(mask, SDNPDATA_CROB_CTRL_LATCH_OFF), curLen, bGetLen);
  result = _putBoolField(result, "    ", "supportTripClose", TMWDEFS_TOBOOL(mask, SDNPDATA_CROB_CTRL_PAIRED_CLOSE), curLen, bGetLen);
  return result;
}
#endif

#if SDNPDATA_SUPPORT_OBJ3
static TMWTYPES_CHAR * TMWDEFS_LOCAL _putDblBitValue(TMWTYPES_CHAR *result, TMWTYPES_UCHAR state, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  TMWTYPES_CHAR buf[128];

  switch(state & 0xc0)
  {
    case DNPDEFS_DBAS_FLAG_DOUBLE_INTER:
      tmwtarg_snprintf(buf, sizeof(buf), "%s<state>%s</state>\n", "     ", "intermediate");
      break;
    case DNPDEFS_DBAS_FLAG_DOUBLE_OFF:
      tmwtarg_snprintf(buf, sizeof(buf), "%s<state>%s</state>\n", "     ", "off");
      break;
    case DNPDEFS_DBAS_FLAG_DOUBLE_ON:
      tmwtarg_snprintf(buf, sizeof(buf), "%s<state>%s</state>\n", "     ", "on");
      break;
    case DNPDEFS_DBAS_FLAG_DOUBLE_INDET:
      tmwtarg_snprintf(buf, sizeof(buf), "%s<state>%s</state>\n", "     ", "indeterminate");
      break;
  }

  return _appendStringLen(result, buf, curLen, bGetLen);
}
#endif

#if SDNPDATA_SUPPORT_OBJ0
static void TMWDEFS_LOCAL _toDeviceAttOutData(DNPDATA_ATTRIBUTE_VALUE *pValue, TMWTYPES_UCHAR *pOut, int *pLength)
{
  int length = 0; 
  TMWTYPES_CHAR tempBuf[256];

  switch(pValue->type)
  {
    case DNPDEFS_ATTRIBUTE_TYPE_VSTR:   
      length = pValue->length;
      if(length+1 > *pLength)
        length = (*pLength)-1;  
      if ( length ) {
        memcpy(tempBuf, pValue->value.pStrValue, length); 
      } else {
        tempBuf[0] = NULL;
      }
      break;  
    case DNPDEFS_ATTRIBUTE_TYPE_UINT:    
      length = tmwtarg_snprintf(tempBuf, *pLength, "%u",pValue->value.uintValue);
      break;
    case DNPDEFS_ATTRIBUTE_TYPE_INT:  
      length = tmwtarg_snprintf(tempBuf, *pLength, "%d",pValue->value.intValue); 
      break;
    case DNPDEFS_ATTRIBUTE_TYPE_FLT:  
      if(pValue->length == 4)
        length = tmwtarg_snprintf(tempBuf, *pLength, "%g",pValue->value.fltValue.sfltValue);  
      else
        length = tmwtarg_snprintf(tempBuf, *pLength, "%g",pValue->value.fltValue.doubleValue);  
      break; 
      
    case DNPDEFS_ATTRIBUTE_TYPE_DNP3TIME:
        tmwdiag_time2string(&pValue->value.timeValue, TMWDEFS_TIME_FORMAT_56, tempBuf, sizeof(tempBuf), TMWDEFS_FALSE);
        length = tmwtarg_snprintf(tempBuf, *pLength, "%s", tempBuf); 
        break;

    case DNPDEFS_ATTRIBUTE_TYPE_OSTR:  
    case DNPDEFS_ATTRIBUTE_TYPE_BSTR:  
      {   
        int j;
        int len = 0;
        /* Make sure it fits */
        length = pValue->length;
        if(length+1 > *pLength)
          length = (*pLength)-1;  
        for(j=0; j< length; j++)
          len += tmwtarg_snprintf((tempBuf + len), sizeof(tempBuf)-len, "%02x ", pValue->value.pStrValue[j]);
        length = len;
      }
      break;
    default:
      length = 0; 
      break;
  }  

  *pLength = length;
  if(length > 0)
  {
    memcpy(pOut, tempBuf, length);
    *(pOut+length) = 0;
  }
}

static TMWTYPES_CHAR * TMWDEFS_LOCAL _buildDeviceAttrPoint(TMWTYPES_CHAR *result, void *pDbHandle, TMWTYPES_USHORT pointNum, TMWTYPES_UCHAR *pVariation, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  TMWTYPES_UCHAR variation;
  TMWTYPES_UCHAR nextVariation;
  TMWTYPES_CHAR buf[256];
  void *pPoint;

  variation = 0;
  nextVariation = 255;
  pPoint = sdnpdata_deviceAttrGetPoint(pDbHandle, pointNum);
  if(pPoint != TMWDEFS_NULL)
  {
    void *pAttribute;
    TMWTYPES_UCHAR property;
    
    variation = *pVariation;

    if(variation == 0)
    {
      result = _appendStringLen(result, "   <deviceAttribute>\n", curLen, bGetLen);
      tmwtarg_snprintf(buf, sizeof(buf), "    <pointNumber>%d</pointNumber>\n", pointNum);
      result = _appendStringLen(result, buf, curLen, bGetLen);
    }

    if(sdnpdata_deviceAttrNext(pPoint, variation, &nextVariation, &property)) 
    { 
      TMWTYPES_CHAR *desc;
      int len;
      DNPDATA_ATTRIBUTE_VALUE data;
      TMWTYPES_UCHAR value[256]; 
      variation = nextVariation;
 
      pAttribute = sdnpdata_deviceAttrGetVar(pPoint, variation); 
      if(pAttribute != TMWDEFS_NULL)
      {
        sdnpdata_deviceAttrRead(pAttribute, &data);

        result = _appendStringLen(result, "    <deviceAttributeElement>\n", curLen, bGetLen);

        tmwtarg_snprintf(buf, sizeof(buf), "     <variation>%d</variation>\n", variation);
        result = _appendStringLen(result, buf, curLen, bGetLen);

        tmwtarg_snprintf(buf, sizeof(buf), "     <property>%d</property>\n", property);
        result = _appendStringLen(result, buf, curLen, bGetLen);

        tmwtarg_snprintf(buf, sizeof(buf), "     <dataType>%d</dataType>\n", data.type);
        result = _appendStringLen(result, buf, curLen, bGetLen);

        tmwtarg_snprintf(buf, sizeof(buf), "     <length>%d</length>\n", data.length);
        result = _appendStringLen(result, buf, curLen, bGetLen);

        desc = tmwsim_getDescription((TMWSIM_POINT*)pAttribute);
        if(desc && (strlen(desc) > 0))
        {
          tmwsim_xmlFormatDesc(desc, buf, sizeof(buf));
          result = _appendStringLen(result, buf, curLen, bGetLen);
        }

        len = 256;
        _toDeviceAttOutData(&data, value, &len);
        if(len > 0)
        {
          tmwtarg_snprintf(buf, sizeof(buf), "     <value>%s</value>\n", value);
          result = _appendStringLen(result, buf, curLen, bGetLen);
        }
        result = _appendStringLen(result, "    </deviceAttributeElement>\n", curLen, bGetLen);
      }
    }
    else 
    {
      nextVariation = 255;

      /* See if there are any more device attribute points */
      pPoint = sdnpdata_deviceAttrGetPoint(pDbHandle, pointNum+1);
      result = _appendStringLen(result, "   </deviceAttribute>\n", curLen, bGetLen);
    }
  }
   
  *pVariation = nextVariation;

  return result;
}
#endif

#if SDNPDATA_SUPPORT_EVENT_MODE_POINT
static TMWTYPES_CHAR * _eventModeString(TMWDEFS_EVENT_MODE reportingMode)
{
  switch(reportingMode)
  { 
    case TMWDEFS_EVENT_MODE_PER_POINT:
      return "basedOnPointIndex";
 
    case TMWDEFS_EVENT_MODE_SOE:
      return "allEvents";
 
    case TMWDEFS_EVENT_MODE_MOST_RECENT:
      return "mostRecentEventTimeValue";
 
    case TMWDEFS_EVENT_MODE_CURRENT:
      return "mostRecentResponseTimeValue";
 
  }
  return "allEvents";
}
#endif

#if SDNPDATA_SUPPORT_OBJ1
static TMWTYPES_CHAR * TMWDEFS_LOCAL _buildBinInPoint(TMWTYPES_CHAR *result, void *pDbHandle, TMWTYPES_USHORT pointNum, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  TMWTYPES_CHAR buf[256];
  void *pPoint;

  pPoint = sdnpdata_binInGetPoint(pDbHandle, pointNum);
  if(pPoint != TMWDEFS_NULL)
  {
    TMWDEFS_CLASS_MASK classMask;
    TMWTYPES_UCHAR flags;
    TMWTYPES_CHAR *desc;

    sdnpdata_binInRead(pPoint, &flags);
    desc = sdnpdata_binInGetDescription(pPoint);
    classMask = sdnpdata_binInEventClass(pPoint);

    result = _appendStringLen(result, "   <binaryInput>\n", curLen, bGetLen);

    tmwtarg_snprintf(buf, sizeof(buf), "    <pointNumber>%d</pointNumber>\n", pointNum);
    result = _appendStringLen(result, buf, curLen, bGetLen);

    if(desc && (strlen(desc) > 0))
    {
      tmwsim_xmlFormatDesc(desc, buf, sizeof(buf));
      result = _appendStringLen(result, buf, curLen, bGetLen);
    }
    
    result = _putClass(result, classMask, curLen, bGetLen);

    result = _appendStringLen(result, "    <flags>\n", curLen, bGetLen);
    result = _putBoolField(result, "     ", "online", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_ON_LINE), curLen, bGetLen);
    result = _putBoolField(result, "     ", "restart", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_RESTART), curLen, bGetLen);
    result = _putBoolField(result, "     ", "commLost", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_COMM_LOST), curLen, bGetLen);
    result = _putBoolField(result, "     ", "remoteForced", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_REMOTE_FORCED), curLen, bGetLen);
    result = _putBoolField(result, "     ", "localForced", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_LOCAL_FORCED), curLen, bGetLen);
    result = _putBoolField(result, "     ", "chatterFilter", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_CHATTER), curLen, bGetLen);
    result = _putBoolField(result, "     ", "state", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_BINARY_ON), curLen, bGetLen);
    result = _appendStringLen(result, "    </flags>\n", curLen, bGetLen);

    /* enabled.  I currently can't get the point if it is not enabled. */ 

#if SDNPDATA_SUPPORT_EVENT_VAR_POINT
    tmwtarg_snprintf(buf, sizeof(buf), "    <defaultEventVariation>%d</defaultEventVariation>\n", sdnpdata_binInEventDefVariation(pPoint, TMWDEFS_CLASS_MASK_ALL));
    result = _appendStringLen(result, buf, curLen, bGetLen);
#endif

#if SDNPDATA_SUPPORT_STATIC_VAR_POINT
    tmwtarg_snprintf(buf, sizeof(buf), "    <defaultStaticVariation>%d</defaultStaticVariation>\n", sdnpdata_binInDefVariation(pPoint));
    result = _appendStringLen(result, buf, curLen, bGetLen);
#endif

#if SDNPDATA_SUPPORT_EVENT_MODE_POINT
    tmwtarg_snprintf(buf, sizeof(buf), "    <eventMode>%s</eventMode>\n", _eventModeString(sdnpdata_binInEventMode(pPoint)));
    result = _appendStringLen(result, buf, curLen, bGetLen);
#endif

/* Add to xml ? PointTime */

    result = _appendStringLen(result, "   </binaryInput>\n", curLen, bGetLen);
  }
  return result;
}
#endif

#if SDNPDATA_SUPPORT_OBJ3
static TMWTYPES_CHAR * TMWDEFS_LOCAL _buildDblBitInPoint(TMWTYPES_CHAR *result, void *pDbHandle, TMWTYPES_USHORT pointNum, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  TMWTYPES_CHAR buf[256];
  void *pPoint;

  pPoint = sdnpdata_dblInGetPoint(pDbHandle, pointNum);
  if(pPoint != TMWDEFS_NULL)
  {
    TMWDEFS_CLASS_MASK classMask;
    TMWTYPES_UCHAR flags;
    TMWTYPES_CHAR *desc;

    sdnpdata_dblInRead(pPoint, &flags);
    desc = sdnpdata_dblInGetDescription(pPoint);
#if SDNPDATA_SUPPORT_OBJ4
    classMask = sdnpdata_dblInEventClass(pPoint);
#else
    classMask = TMWDEFS_CLASS_MASK_NONE;
#endif

    result = _appendStringLen(result, "   <doubleBitInput>\n", curLen, bGetLen);

    tmwtarg_snprintf(buf, sizeof(buf), "    <pointNumber>%d</pointNumber>\n", pointNum);
    result = _appendStringLen(result, buf, curLen, bGetLen);

    if(desc && (strlen(desc) > 0))
    {
      tmwsim_xmlFormatDesc(desc, buf, sizeof(buf));
      result = _appendStringLen(result, buf, curLen, bGetLen);
    }
    
    result = _putClass(result, classMask, curLen, bGetLen);

    result = _appendStringLen(result, "    <flags>\n", curLen, bGetLen);
    result = _putBoolField(result, "     ", "online", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_ON_LINE), curLen, bGetLen);
    result = _putBoolField(result, "     ", "restart", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_RESTART), curLen, bGetLen);
    result = _putBoolField(result, "     ", "commLost", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_COMM_LOST), curLen, bGetLen);
    result = _putBoolField(result, "     ", "remoteForced", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_REMOTE_FORCED), curLen, bGetLen);
    result = _putBoolField(result, "     ", "localForced", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_LOCAL_FORCED), curLen, bGetLen);
    result = _putBoolField(result, "     ", "chatterFilter", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_CHATTER), curLen, bGetLen);
    result = _putDblBitValue(result, flags, curLen,  bGetLen);
    result = _appendStringLen(result, "    </flags>\n", curLen, bGetLen);
 
    /* enabled.  I currently can't get the point if it is not enabled. */ 

#if SDNPDATA_SUPPORT_EVENT_VAR_POINT && SDNPDATA_SUPPORT_OBJ4
    tmwtarg_snprintf(buf, sizeof(buf), "    <defaultEventVariation>%d</defaultEventVariation>\n", sdnpdata_dblInEventDefVariation(pPoint, TMWDEFS_CLASS_MASK_ALL));
    result = _appendStringLen(result, buf, curLen, bGetLen);
#endif

#if SDNPDATA_SUPPORT_STATIC_VAR_POINT
    tmwtarg_snprintf(buf, sizeof(buf), "    <defaultStaticVariation>%d</defaultStaticVariation>\n", sdnpdata_dblInDefVariation(pPoint));
    result = _appendStringLen(result, buf, curLen, bGetLen);
#endif

#if SDNPDATA_SUPPORT_EVENT_MODE_POINT && SDNPDATA_SUPPORT_OBJ4
    tmwtarg_snprintf(buf, sizeof(buf), "    <eventMode>%s</eventMode>\n", _eventModeString(sdnpdata_dblInEventMode(pPoint)));
    result = _appendStringLen(result, buf, curLen, bGetLen);
#endif

    result = _appendStringLen(result, "   </doubleBitInput>\n", curLen, bGetLen);
  }
  return result;
}
#endif

#if SDNPDATA_SUPPORT_OBJ12
static TMWTYPES_CHAR * TMWDEFS_LOCAL _buildBinOutPoint(TMWTYPES_CHAR *result, void *pDbHandle, TMWTYPES_USHORT pointNum, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  TMWTYPES_CHAR buf[256];
  void *pPoint;

  pPoint = sdnpdata_binOutGetPoint(pDbHandle, pointNum);
  if(pPoint != TMWDEFS_NULL)
  {
    TMWDEFS_CLASS_MASK classMask;
    SDNPDATA_CROB_CTRL ctrlMask;
    TMWTYPES_UCHAR flags;
    TMWTYPES_CHAR *desc;

    sdnpdata_binOutRead(pPoint, &flags);
    desc = sdnpdata_binOutGetDescription(pPoint);
    ctrlMask = sdnpdata_binOutGetControlMask(pPoint);

    result = _appendStringLen(result, "   <binaryOutputControl>\n", curLen, bGetLen);

    tmwtarg_snprintf(buf, sizeof(buf), "    <pointNumber>%d</pointNumber>\n", pointNum);
    result = _appendStringLen(result, buf, curLen, bGetLen);

    if(desc && (strlen(desc) > 0))
    {
      tmwsim_xmlFormatDesc(desc, buf, sizeof(buf));
      result = _appendStringLen(result, buf, curLen, bGetLen);
    }
    
    result = _putControl(result, curLen, bGetLen);
    result = _putBinControl(result, ctrlMask, curLen, bGetLen);

#if SDNPDATA_SUPPORT_OBJ13
    classMask = sdnpdata_binOutCmdEventClass(pPoint);
#else
    classMask = TMWDEFS_CLASS_MASK_NONE;
#endif
    
    result = _putClass(result, classMask, curLen, bGetLen);

    result = _appendStringLen(result, "   </binaryOutputControl>\n", curLen, bGetLen);
  }
  return result;
}
#endif

#if SDNPDATA_SUPPORT_OBJ10
static TMWTYPES_CHAR * TMWDEFS_LOCAL _buildBinOutStatusPoint(TMWTYPES_CHAR *result, void *pDbHandle, TMWTYPES_USHORT pointNum, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  TMWTYPES_CHAR buf[256];
  void *pPoint;

  pPoint = sdnpdata_binOutGetPoint(pDbHandle, pointNum);
  if(pPoint != TMWDEFS_NULL)
  {
    TMWTYPES_ULONG assocPoint;
    TMWTYPES_UCHAR flags;
    TMWTYPES_UCHAR classMask;
    TMWTYPES_CHAR *desc;

    sdnpdata_binOutRead(pPoint, &flags);

#if SDNPDATA_SUPPORT_OBJ11
    classMask = sdnpdata_binOutEventClass(pPoint);
#else
    classMask = TMWDEFS_CLASS_MASK_NONE;
#endif
    desc = sdnpdata_binOutGetDescription(pPoint);

    result = _appendStringLen(result, "   <binaryOutputStatus>\n", curLen, bGetLen);

    tmwtarg_snprintf(buf, sizeof(buf), "    <pointNumber>%d</pointNumber>\n", pointNum);
    result = _appendStringLen(result, buf, curLen, bGetLen);

    if(desc && (strlen(desc) > 0))
    {
      tmwsim_xmlFormatDesc(desc, buf, sizeof(buf));
      result = _appendStringLen(result, buf, curLen, bGetLen);
    }
    
    result = _putClass(result, classMask, curLen, bGetLen);

    result = _appendStringLen(result, "    <flags>\n", curLen, bGetLen);
    result = _putBoolField(result, "     ", "online", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_ON_LINE), curLen, bGetLen);
    result = _putBoolField(result, "     ", "restart", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_RESTART), curLen, bGetLen);
    result = _putBoolField(result, "     ", "commLost", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_COMM_LOST), curLen, bGetLen);
    result = _putBoolField(result, "     ", "remoteForced", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_REMOTE_FORCED), curLen, bGetLen);
    result = _putBoolField(result, "     ", "localForced", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_LOCAL_FORCED), curLen, bGetLen);
    result = _putBoolField(result, "     ", "state", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_BINARY_ON), curLen, bGetLen);
    result = _appendStringLen(result, "    </flags>\n", curLen, bGetLen);

    assocPoint = tmwsim_getAssocPoint((TMWSIM_POINT*)pPoint);
    if (assocPoint != TMWDEFS_NO_ASSOC_POINT)
    {
      /* 0 must be written as special value, otherwise old workspaces will look like
       *  all points have assocPoint 0!
       */
      if (assocPoint == 0)
        assocPoint = SDNPXML_ASSOC_ZERO;

      tmwtarg_snprintf(buf, sizeof(buf), "    <assocPoint>%u</assocPoint>\n", assocPoint);
      result = _appendStringLen(result, buf, curLen, bGetLen);
    }

#if SDNPDATA_SUPPORT_EVENT_VAR_POINT && SDNPDATA_SUPPORT_OBJ11
    tmwtarg_snprintf(buf, sizeof(buf), "    <defaultEventVariation>%d</defaultEventVariation>\n", sdnpdata_binOutEventDefVariation(pPoint, TMWDEFS_CLASS_MASK_ALL));
    result = _appendStringLen(result, buf, curLen, bGetLen);
#endif

#if SDNPDATA_SUPPORT_STATIC_VAR_POINT
    tmwtarg_snprintf(buf, sizeof(buf), "    <defaultStaticVariation>%d</defaultStaticVariation>\n", sdnpdata_binOutDefVariation(pPoint));
    result = _appendStringLen(result, buf, curLen, bGetLen);
#endif

#if SDNPDATA_SUPPORT_EVENT_MODE_POINT && SDNPDATA_SUPPORT_OBJ11
    tmwtarg_snprintf(buf, sizeof(buf), "    <eventMode>%s</eventMode>\n", _eventModeString(sdnpdata_binOutEventMode(pPoint)));
    result = _appendStringLen(result, buf, curLen, bGetLen);
#endif
    result = _appendStringLen(result, "   </binaryOutputStatus>\n", curLen, bGetLen);
  }
  return result;
}
#endif

#if SDNPDATA_SUPPORT_OBJ20
static TMWTYPES_CHAR * TMWDEFS_LOCAL _buildBinCounterPoint(TMWTYPES_CHAR *result, void *pDbHandle, TMWTYPES_USHORT pointNum, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  TMWTYPES_CHAR buf[256];
  void *pPoint;
  pPoint = sdnpdata_binCntrGetPoint(pDbHandle, pointNum);
  if(pPoint != TMWDEFS_NULL)
  {
    TMWDEFS_CLASS_MASK classMask;
    TMWTYPES_ULONG value;
    TMWTYPES_UCHAR flags;
    TMWTYPES_CHAR *desc;

    desc = sdnpdata_binCntrGetDescription(pPoint);
    sdnpdata_binCntrRead(pPoint, &value, &flags);
#if SDNPDATA_SUPPORT_OBJ22
    classMask = sdnpdata_binCntrEventClass(pPoint);
#else
classMask = TMWDEFS_CLASS_MASK_NONE;
#endif

    result = _appendStringLen(result, "   <binaryCounter>\n", curLen, bGetLen);

    tmwtarg_snprintf(buf, sizeof(buf), "    <pointNumber>%d</pointNumber>\n", pointNum);
    result = _appendStringLen(result, buf, curLen, bGetLen);

    if(desc && (strlen(desc) > 0))
    {
      tmwsim_xmlFormatDesc(desc, buf, sizeof(buf));
      result = _appendStringLen(result, buf, curLen, bGetLen);
    }
    
    result = _putClass(result, classMask, curLen, bGetLen);

    result = _appendStringLen(result, "    <flags>\n", curLen, bGetLen);
    result = _putBoolField(result, "     ", "online", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_ON_LINE), curLen, bGetLen);
    result = _putBoolField(result, "     ", "restart", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_RESTART), curLen, bGetLen);
    result = _putBoolField(result, "     ", "commLost", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_COMM_LOST), curLen, bGetLen);
    result = _putBoolField(result, "     ", "remoteForced", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_REMOTE_FORCED), curLen, bGetLen);
    result = _putBoolField(result, "     ", "localForced", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_LOCAL_FORCED), curLen, bGetLen);
    result = _putBoolField(result, "     ", "rollover", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_CNTR_ROLLOVER), curLen, bGetLen);
    result = _putBoolField(result, "     ", "discontinuity", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_REFERENCE_CHK), curLen, bGetLen);
    result = _appendStringLen(result, "    </flags>\n", curLen, bGetLen);

    tmwtarg_snprintf(buf, sizeof(buf), "     <value>%u</value>\n", value);
    result = _appendStringLen(result, buf, curLen, bGetLen);

#if SDNPDATA_SUPPORT_EVENT_VAR_POINT && SDNPDATA_SUPPORT_OBJ22
    tmwtarg_snprintf(buf, sizeof(buf), "    <defaultEventVariation>%d</defaultEventVariation>\n", sdnpdata_binCntrEventDefVariation(pPoint, TMWDEFS_CLASS_MASK_ALL));
    result = _appendStringLen(result, buf, curLen, bGetLen);
#endif

#if SDNPDATA_SUPPORT_STATIC_VAR_POINT
    tmwtarg_snprintf(buf, sizeof(buf), "    <defaultStaticVariation>%d</defaultStaticVariation>\n", sdnpdata_binCntrDefVariation(pPoint));
    result = _appendStringLen(result, buf, curLen, bGetLen);
#endif

#if SDNPDATA_SUPPORT_EVENT_MODE_POINT && SDNPDATA_SUPPORT_OBJ22  
    tmwtarg_snprintf(buf, sizeof(buf), "    <eventMode>%s</eventMode>\n", _eventModeString(sdnpdata_binCntrEventMode(pPoint)));
    result = _appendStringLen(result, buf, curLen, bGetLen);
#endif

    result = _appendStringLen(result, "  </binaryCounter>\n", curLen, bGetLen);
  }
  return result;
}
#endif

#if SDNPDATA_SUPPORT_OBJ21
static TMWTYPES_CHAR * TMWDEFS_LOCAL _buildFrozenCounterPoint(TMWTYPES_CHAR *result, void *pDbHandle, TMWTYPES_USHORT pointNum, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  TMWTYPES_CHAR buf[256];
  void *pPoint;
  pPoint = sdnpdata_frznCntrGetPoint(pDbHandle, pointNum);
  if(pPoint != TMWDEFS_NULL)
  {
    TMWDEFS_CLASS_MASK classMask;
    TMWDTIME timeOfFreeze;
    TMWTYPES_ULONG value;
    TMWTYPES_UCHAR flags;
    TMWTYPES_CHAR *desc;

    desc = sdnpdata_frznCntrGetDescription(pPoint);
    sdnpdata_frznCntrRead(pPoint, &value, &flags, &timeOfFreeze);
#if SDNPDATA_SUPPORT_OBJ23
    classMask = sdnpdata_frznCntrEventClass(pPoint);
#else
    classMask = TMWDEFS_CLASS_MASK_NONE;
#endif

    result = _appendStringLen(result, "   <frozenCounter>\n", curLen, bGetLen);

    tmwtarg_snprintf(buf, sizeof(buf), "    <pointNumber>%d</pointNumber>\n", pointNum);
    result = _appendStringLen(result, buf, curLen, bGetLen);

    if(desc && (strlen(desc) > 0))
    {
      tmwsim_xmlFormatDesc(desc, buf, sizeof(buf));
      result = _appendStringLen(result, buf, curLen, bGetLen);
    }
    
    result = _putClass(result, classMask, curLen, bGetLen);

    result = _appendStringLen(result, "    <flags>\n", curLen, bGetLen);
    result = _putBoolField(result, "     ", "online", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_ON_LINE), curLen, bGetLen);
    result = _putBoolField(result, "     ", "restart", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_RESTART), curLen, bGetLen);
    result = _putBoolField(result, "     ", "commLost", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_COMM_LOST), curLen, bGetLen);
    result = _putBoolField(result, "     ", "remoteForced", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_REMOTE_FORCED), curLen, bGetLen);
    result = _putBoolField(result, "     ", "localForced", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_LOCAL_FORCED), curLen, bGetLen);
    result = _putBoolField(result, "     ", "rollover", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_CNTR_ROLLOVER), curLen, bGetLen);
    result = _putBoolField(result, "     ", "discontinuity", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_REFERENCE_CHK), curLen, bGetLen);
    result = _appendStringLen(result, "    </flags>\n", curLen, bGetLen);

    tmwtarg_snprintf(buf, sizeof(buf), "     <value>%u</value>\n", value);
    result = _appendStringLen(result, buf, curLen, bGetLen);

#if SDNPDATA_SUPPORT_EVENT_VAR_POINT && SDNPDATA_SUPPORT_OBJ23
    tmwtarg_snprintf(buf, sizeof(buf), "    <defaultEventVariation>%d</defaultEventVariation>\n", sdnpdata_frznCntrEventDefVariation(pPoint, TMWDEFS_CLASS_MASK_ALL));
    result = _appendStringLen(result, buf, curLen, bGetLen);
#endif

#if SDNPDATA_SUPPORT_STATIC_VAR_POINT
    tmwtarg_snprintf(buf, sizeof(buf), "    <defaultStaticVariation>%d</defaultStaticVariation>\n", sdnpdata_frznCntrDefVariation(pPoint));
    result = _appendStringLen(result, buf, curLen, bGetLen);
#endif

#if SDNPDATA_SUPPORT_EVENT_MODE_POINT && SDNPDATA_SUPPORT_OBJ23
    tmwtarg_snprintf(buf, sizeof(buf), "    <eventMode>%s</eventMode>\n", _eventModeString(sdnpdata_frznCntrEventMode(pPoint)));
    result = _appendStringLen(result, buf, curLen, bGetLen);
#endif


    result = _appendStringLen(result, "  </frozenCounter>\n", curLen, bGetLen);
  }
  return result;
}
#endif

#if SDNPDATA_SUPPORT_OBJ30
static TMWTYPES_CHAR * TMWDEFS_LOCAL _buildAnalogInPoint(TMWTYPES_CHAR *result, void *pDbHandle, TMWTYPES_USHORT pointNum, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  TMWTYPES_CHAR buf[256];
  void *pPoint;
  pPoint = sdnpdata_anlgInGetPoint(pDbHandle, pointNum);
  if(pPoint != TMWDEFS_NULL)
  {
    TMWDEFS_CLASS_MASK classMask;
    TMWTYPES_ANALOG_VALUE value;
    TMWTYPES_UCHAR flags;
    TMWTYPES_CHAR *desc;

    desc = sdnpdata_anlgInGetDescription(pPoint);
    sdnpdata_anlgInRead(pPoint, &value, &flags);
#if SDNPDATA_SUPPORT_OBJ32
    classMask = sdnpdata_anlgInEventClass(pPoint);
#else
    classMask = TMWDEFS_CLASS_MASK_NONE;
#endif

    result = _appendStringLen(result, "   <analogInput>\n", curLen, bGetLen);

    tmwtarg_snprintf(buf, sizeof(buf), "    <pointNumber>%d</pointNumber>\n", pointNum);
    result = _appendStringLen(result, buf, curLen, bGetLen);

    if(desc && (strlen(desc) > 0))
    {
      tmwsim_xmlFormatDesc(desc, buf, sizeof(buf));
      result = _appendStringLen(result, buf, curLen, bGetLen);
    }
    
    result = _putClass(result, classMask, curLen, bGetLen);

    result = _appendStringLen(result, "    <flags>\n", curLen, bGetLen);
    result = _putBoolField(result, "     ", "online", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_ON_LINE), curLen, bGetLen);
    result = _putBoolField(result, "     ", "restart", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_RESTART), curLen, bGetLen);
    result = _putBoolField(result, "     ", "commLost", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_COMM_LOST), curLen, bGetLen);
    result = _putBoolField(result, "     ", "remoteForced", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_REMOTE_FORCED), curLen, bGetLen);
    result = _putBoolField(result, "     ", "localForced", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_LOCAL_FORCED), curLen, bGetLen);
    result = _putBoolField(result, "     ", "overRange", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_OVER_RANGE), curLen, bGetLen);
    result = _putBoolField(result, "     ", "referenceCheck", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_REFERENCE_CHK), curLen, bGetLen);
    result = _appendStringLen(result, "    </flags>\n", curLen, bGetLen);

#if TMWCNFG_SUPPORT_DOUBLE
    tmwtarg_snprintf(buf, sizeof(buf), "    <value>%g</value>\n", dnputil_getAnalogValueDouble(&value));
#elif TMWCNFG_SUPPORT_SFLOAT
    tmwtarg_snprintf(buf, sizeof(buf), "    <value>%g</value>\n", dnputil_getAnalogValueFloat(&value));
#else
    tmwtarg_snprintf(buf, sizeof(buf), "    <value>%d</value>\n", dnputil_getAnalogValueLong(&value,&flags));
#endif
    result = _appendStringLen(result, buf, curLen, bGetLen);

#if SDNPDATA_SUPPORT_EVENT_VAR_POINT && SDNPDATA_SUPPORT_OBJ32
    tmwtarg_snprintf(buf, sizeof(buf), "    <defaultEventVariation>%d</defaultEventVariation>\n", sdnpdata_anlgInEventDefVariation(pPoint, TMWDEFS_CLASS_MASK_ALL));
    result = _appendStringLen(result, buf, curLen, bGetLen);
#endif

#if SDNPDATA_SUPPORT_STATIC_VAR_POINT
    tmwtarg_snprintf(buf, sizeof(buf), "    <defaultStaticVariation>%d</defaultStaticVariation>\n", sdnpdata_anlgInDefVariation(pPoint));
    result = _appendStringLen(result, buf, curLen, bGetLen);
#endif

#if  SDNPDATA_SUPPORT_EVENT_MODE_POINT && SDNPDATA_SUPPORT_OBJ32
    tmwtarg_snprintf(buf, sizeof(buf), "    <eventMode>%s</eventMode>\n", _eventModeString(sdnpdata_anlgInEventMode(pPoint)));
    result = _appendStringLen(result, buf, curLen, bGetLen);
#endif

    result = _appendStringLen(result, "   </analogInput>\n", curLen, bGetLen);
  }
  return result;
}
#endif

#if SDNPDATA_SUPPORT_OBJ31
static TMWTYPES_CHAR * TMWDEFS_LOCAL _buildFrznAnalogInPoint(TMWTYPES_CHAR *result, void *pDbHandle, TMWTYPES_USHORT pointNum, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  TMWTYPES_CHAR buf[256];
  void *pPoint;
  pPoint = sdnpdata_frznAnlgInGetPoint(pDbHandle, pointNum);
  if (pPoint != TMWDEFS_NULL)
  {
    TMWDEFS_CLASS_MASK classMask;
    TMWTYPES_ANALOG_VALUE value;
    TMWTYPES_UCHAR flags;
    TMWTYPES_CHAR *desc;
    TMWDTIME timeOfFreeze;

    desc = sdnpdata_frznAnlgInGetDescription(pPoint);
    sdnpdata_frznAnlgInRead(pPoint, &value, &flags, &timeOfFreeze);
    classMask = sdnpdata_frznAnlgInEventClass(pPoint);

    result = _appendStringLen(result, "   <frznAnalogInput>\n", curLen, bGetLen);

    tmwtarg_snprintf(buf, sizeof(buf), "    <pointNumber>%d</pointNumber>\n", pointNum);
    result = _appendStringLen(result, buf, curLen, bGetLen);

    if (desc && (strlen(desc) > 0))
    {
      tmwsim_xmlFormatDesc(desc, buf, sizeof(buf));
      result = _appendStringLen(result, buf, curLen, bGetLen);
    }

    result = _putClass(result, classMask, curLen, bGetLen);

    result = _appendStringLen(result, "    <flags>\n", curLen, bGetLen);
    result = _putBoolField(result, "     ", "online", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_ON_LINE), curLen, bGetLen);
    result = _putBoolField(result, "     ", "restart", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_RESTART), curLen, bGetLen);
    result = _putBoolField(result, "     ", "commLost", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_COMM_LOST), curLen, bGetLen);
    result = _putBoolField(result, "     ", "remoteForced", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_REMOTE_FORCED), curLen, bGetLen);
    result = _putBoolField(result, "     ", "localForced", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_LOCAL_FORCED), curLen, bGetLen);
    result = _putBoolField(result, "     ", "overRange", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_OVER_RANGE), curLen, bGetLen);
    result = _putBoolField(result, "     ", "referenceCheck", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_REFERENCE_CHK), curLen, bGetLen);
    result = _appendStringLen(result, "    </flags>\n", curLen, bGetLen);

#if TMWCNFG_SUPPORT_DOUBLE
    tmwtarg_snprintf(buf, sizeof(buf), "    <value>%g</value>\n", dnputil_getAnalogValueDouble(&value));
#elif TMWCNFG_SUPPORT_SFLOAT
    tmwtarg_snprintf(buf, sizeof(buf), "    <value>%g</value>\n", dnputil_getAnalogValueFloat(&value));
#else
    tmwtarg_snprintf(buf, sizeof(buf), "    <value>%d</value>\n", dnputil_getAnalogValueLong(&value, &flags));
#endif
    result = _appendStringLen(result, buf, curLen, bGetLen);

#if SDNPDATA_SUPPORT_EVENT_VAR_POINT
    tmwtarg_snprintf(buf, sizeof(buf), "    <defaultEventVariation>%d</defaultEventVariation>\n", sdnpdata_frznAnlgInEventDefVariation(pPoint, TMWDEFS_CLASS_MASK_ALL));
    result = _appendStringLen(result, buf, curLen, bGetLen);
#endif

#if SDNPDATA_SUPPORT_STATIC_VAR_POINT
    tmwtarg_snprintf(buf, sizeof(buf), "    <defaultStaticVariation>%d</defaultStaticVariation>\n", sdnpdata_frznAnlgInDefVariation(pPoint));
    result = _appendStringLen(result, buf, curLen, bGetLen);
#endif

#if SDNPDATA_SUPPORT_EVENT_MODE_POINT
    tmwtarg_snprintf(buf, sizeof(buf), "    <eventMode>%s</eventMode>\n", _eventModeString(sdnpdata_frznAnlgInEventMode(pPoint)));
    result = _appendStringLen(result, buf, curLen, bGetLen);
#endif

    result = _appendStringLen(result, "   </frznAnalogInput>\n", curLen, bGetLen);
  }
  return result;
}
#endif

#if SDNPDATA_SUPPORT_OBJ34
static TMWTYPES_CHAR * TMWDEFS_LOCAL _buildAnalogInDBandPoint(TMWTYPES_CHAR *result, void *pDbHandle, TMWTYPES_USHORT pointNum, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  TMWTYPES_CHAR buf[256];
  void *pPoint;
  pPoint = sdnpdata_anlgInDBandGetPoint(pDbHandle, pointNum);
  if(pPoint != TMWDEFS_NULL)
  {
    TMWTYPES_ANALOG_VALUE value;
    TMWTYPES_CHAR *desc;

    desc = sdnpdata_anlgInDBandGetDescription(pPoint);
    sdnpdata_anlgInDBandRead(pPoint, &value);

    result = _appendStringLen(result, "   <analogInputDeadband>\n", curLen, bGetLen);

    tmwtarg_snprintf(buf, sizeof(buf), "    <pointNumber>%d</pointNumber>\n", pointNum);
    result = _appendStringLen(result, buf, curLen, bGetLen);

    if(desc && (strlen(desc) > 0))
    {
      tmwsim_xmlFormatDesc(desc, buf, sizeof(buf));
      result = _appendStringLen(result, buf, curLen, bGetLen);
    }
    
#if TMWCNFG_SUPPORT_DOUBLE
    tmwtarg_snprintf(buf, sizeof(buf), "    <value>%g</value>\n", dnputil_getAnalogValueDouble(&value));
#elif TMWCNFG_SUPPORT_SFLOAT
    tmwtarg_snprintf(buf, sizeof(buf), "    <value>%g</value>\n", dnputil_getAnalogValueFloat(&value));
#else
    {
    TMWTYPES_UCHAR flags;
    tmwtarg_snprintf(buf, sizeof(buf), "    <value>%d</value>\n", dnputil_getAnalogValueLong(&value,&flags));
    }
#endif
    result = _appendStringLen(result, buf, curLen, bGetLen);

#if SDNPDATA_SUPPORT_STATIC_VAR_POINT
    tmwtarg_snprintf(buf, sizeof(buf), "    <defaultStaticVariation>%d</defaultStaticVariation>\n", sdnpdata_anlgInDbandDefVar(pPoint));
    result = _appendStringLen(result, buf, curLen, bGetLen);
#endif

    result = _appendStringLen(result, "   </analogInputDeadband>\n", curLen, bGetLen);
  }
  return result;
}
#endif

#if SDNPDATA_SUPPORT_OBJ40 || SDNPDATA_SUPPORT_OBJ41
static TMWTYPES_CHAR * TMWDEFS_LOCAL _buildAnalogOutPoint(TMWTYPES_CHAR *result, void *pDbHandle, TMWTYPES_USHORT pointNum, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  TMWTYPES_CHAR buf[256];
  void *pPoint;
  pPoint = sdnpdata_anlgOutGetPoint(pDbHandle, pointNum);
  if(pPoint != TMWDEFS_NULL)
  {
    TMWDEFS_CLASS_MASK classMask;
    TMWTYPES_ANALOG_VALUE value;
    TMWTYPES_UCHAR flags;
    TMWTYPES_CHAR *desc;

    desc = sdnpdata_anlgOutGetDescription(pPoint);
    sdnpdata_anlgOutRead(pPoint, &value, &flags);

    result = _appendStringLen(result, "   <analogOutputControl>\n", curLen, bGetLen);

    tmwtarg_snprintf(buf, sizeof(buf), "    <pointNumber>%d</pointNumber>\n", pointNum);
    result = _appendStringLen(result, buf, curLen, bGetLen);

    if(desc && (strlen(desc) > 0))
    {
      tmwsim_xmlFormatDesc(desc, buf, sizeof(buf));
      result = _appendStringLen(result, buf, curLen, bGetLen);
    }
    
    result = _putControl(result, curLen, bGetLen);

#if SDNPDATA_SUPPORT_OBJ43
    classMask = sdnpdata_anlgOutCmdEventClass(pPoint);
#else
    classMask = TMWDEFS_CLASS_MASK_NONE;
#endif
    
    result = _putClass(result, classMask, curLen, bGetLen);

    result = _appendStringLen(result, "   </analogOutputControl>\n", curLen, bGetLen);
  }
  return result;
}
#endif

#if SDNPDATA_SUPPORT_OBJ40
static TMWTYPES_CHAR * TMWDEFS_LOCAL _buildAnalogOutStatusPoint(TMWTYPES_CHAR *result, void *pDbHandle, TMWTYPES_USHORT pointNum, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  TMWTYPES_CHAR buf[256];
  void *pPoint;
  pPoint = sdnpdata_anlgOutGetPoint(pDbHandle, pointNum);
  if(pPoint != TMWDEFS_NULL)
  {
    TMWTYPES_ULONG assocPoint;
    TMWTYPES_ANALOG_VALUE value;
    TMWTYPES_UCHAR flags;
    TMWTYPES_UCHAR classMask;
    TMWTYPES_CHAR *desc;

    desc = sdnpdata_anlgOutGetDescription(pPoint);
    sdnpdata_anlgOutRead(pPoint, &value, &flags);
#if SDNPDATA_SUPPORT_OBJ42
    classMask = sdnpdata_anlgOutEventClass(pPoint);
#else
    classMask = TMWDEFS_CLASS_MASK_NONE;
#endif

    result = _appendStringLen(result, "   <analogOutputStatus>\n", curLen, bGetLen);

    tmwtarg_snprintf(buf, sizeof(buf), "    <pointNumber>%d</pointNumber>\n", pointNum);
    result = _appendStringLen(result, buf, curLen, bGetLen);

    if(desc && (strlen(desc) > 0))
    {
      tmwsim_xmlFormatDesc(desc, buf, sizeof(buf));
      result = _appendStringLen(result, buf, curLen, bGetLen);
    }

    result = _putClass(result, classMask, curLen, bGetLen);
    
    result = _appendStringLen(result, "    <flags>\n", curLen, bGetLen);
    result = _putBoolField(result, "     ", "online", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_ON_LINE), curLen, bGetLen);
    result = _putBoolField(result, "     ", "restart", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_RESTART), curLen, bGetLen);
    result = _putBoolField(result, "     ", "commLost", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_COMM_LOST), curLen, bGetLen);
    result = _putBoolField(result, "     ", "remoteForced", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_REMOTE_FORCED), curLen, bGetLen);
    result = _putBoolField(result, "     ", "localForced", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_LOCAL_FORCED), curLen, bGetLen);
    result = _putBoolField(result, "     ", "overRange", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_OVER_RANGE), curLen, bGetLen);
    result = _putBoolField(result, "     ", "referenceCheck", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_REFERENCE_CHK), curLen, bGetLen);
    result = _appendStringLen(result, "    </flags>\n", curLen, bGetLen);

    assocPoint = tmwsim_getAssocPoint((TMWSIM_POINT*)pPoint);
    if (assocPoint != TMWDEFS_NO_ASSOC_POINT)
    {
      /* 0 must be written as special value, otherwise old workspaces will look like
       *  all points have assocPoint 0!
       */
      if (assocPoint == 0)
        assocPoint = SDNPXML_ASSOC_ZERO;

      tmwtarg_snprintf(buf, sizeof(buf), "    <assocPoint>%u</assocPoint>\n", assocPoint);
      result = _appendStringLen(result, buf, curLen, bGetLen);
    }

#if TMWCNFG_SUPPORT_DOUBLE
    tmwtarg_snprintf(buf, sizeof(buf), "    <value>%g</value>\n", dnputil_getAnalogValueDouble(&value));
#elif TMWCNFG_SUPPORT_SFLOAT
    tmwtarg_snprintf(buf, sizeof(buf), "    <value>%g</value>\n", dnputil_getAnalogValueFloat(&value));
#else
    tmwtarg_snprintf(buf, sizeof(buf), "    <value>%d</value>\n", dnputil_getAnalogValueLong(&value,&flags));
#endif
    result = _appendStringLen(result, buf, curLen, bGetLen);

#if SDNPDATA_SUPPORT_EVENT_VAR_POINT && SDNPDATA_SUPPORT_OBJ42
    tmwtarg_snprintf(buf, sizeof(buf), "    <defaultEventVariation>%d</defaultEventVariation>\n", sdnpdata_anlgOutEventDefVariation(pPoint, TMWDEFS_CLASS_MASK_ALL));
    result = _appendStringLen(result, buf, curLen, bGetLen);
#endif

#if SDNPDATA_SUPPORT_STATIC_VAR_POINT
    tmwtarg_snprintf(buf, sizeof(buf), "    <defaultStaticVariation>%d</defaultStaticVariation>\n", sdnpdata_anlgOutDefVariation(pPoint));
    result = _appendStringLen(result, buf, curLen, bGetLen);
#endif

#if SDNPDATA_SUPPORT_EVENT_MODE_POINT && SDNPDATA_SUPPORT_OBJ42
    tmwtarg_snprintf(buf, sizeof(buf), "    <eventMode>%s</eventMode>\n", _eventModeString(sdnpdata_anlgOutEventMode(pPoint)));
    result = _appendStringLen(result, buf, curLen, bGetLen);
#endif

    result = _appendStringLen(result, "   </analogOutputStatus>\n", curLen, bGetLen);
  }
  return result;
}
#endif

#if SDNPDATA_SUPPORT_OBJ50_V4
static TMWTYPES_CHAR * TMWDEFS_LOCAL _buildIndexedTimePoint(TMWTYPES_CHAR *result, void *pDbHandle, TMWTYPES_USHORT pointNum, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  TMWTYPES_CHAR buf[256];
  void *pPoint;
  pPoint = sdnpdata_indexedTimeGetPoint(pDbHandle, pointNum);
  if (pPoint != TMWDEFS_NULL)
  {
    TMWDTIME startTime;
    TMWTYPES_ULONG intervalCount;
    TMWTYPES_BYTE intervalUnits;
    TMWTYPES_CHAR *desc;

    desc = sdnpdata_indexedTimeGetDescription(pPoint);
    sdnpdata_indexedTimeRead(pPoint, &startTime, &intervalCount, &intervalUnits);
     
    result = _appendStringLen(result, "   <indexedTime>\n", curLen, bGetLen);

    tmwtarg_snprintf(buf, sizeof(buf), "    <pointNumber>%d</pointNumber>\n", pointNum);
    result = _appendStringLen(result, buf, curLen, bGetLen);

    if (desc && (strlen(desc) > 0))
    {
      tmwsim_xmlFormatDesc(desc, buf, sizeof(buf));
      result = _appendStringLen(result, buf, curLen, bGetLen);
    }

    /* startTime  CCYY-MM-DDThh:mm:ss[Z|(+|-)hh:mm]    2020-05-30T09:30:10.5 */
    tmwtarg_snprintf(buf, sizeof(buf), "    <startTime>%4d-%02d-%02dT%02d:%02d:%02d.%d</startTime>\n", 
      startTime.year, startTime.month, startTime.dayOfMonth, startTime.hour, startTime.minutes, 
      startTime.mSecsAndSecs/1000, startTime.mSecsAndSecs%1000);
    result = _appendStringLen(result, buf, curLen, bGetLen);

    tmwtarg_snprintf(buf, sizeof(buf), "    <intervalCount>%d</intervalCount>\n", intervalCount);
    result = _appendStringLen(result, buf, curLen, bGetLen);

    tmwtarg_snprintf(buf, sizeof(buf), "    <intervalUnits>%d</intervalUnits>\n", intervalUnits);
    result = _appendStringLen(result, buf, curLen, bGetLen);

    result = _putBoolField(result, "    ", "notInClass0", !sdnpdata_indexedTimeIsClass0(pPoint), curLen, bGetLen);

    result = _appendStringLen(result, "   </indexedTime>\n", curLen, bGetLen);
  }
  return result;
}
#endif
		
#if SDNPDATA_SUPPORT_DATASETS
static char * TMWDEFS_LOCAL _toDescrType(TMWTYPES_ULONG type)
{ 
  switch(type)
  {
  case DNPDEFS_DATASET_DESCR_ID: 
    return "id";
   
  case DNPDEFS_DATASET_DESCR_UUID:
    return "uuid";
   
  case DNPDEFS_DATASET_DESCR_NSPC:
    return "nspc";
   
  case DNPDEFS_DATASET_DESCR_NAME:
    return "name";
   
  case DNPDEFS_DATASET_DESCR_DAEL:
    return "dael";
   
  case DNPDEFS_DATASET_DESCR_PTYP:
    return "ptyp";
   
  case DNPDEFS_DATASET_DESCR_CTLV:
    return "ctlv";
   
  case DNPDEFS_DATASET_DESCR_CTLS:
    return "ctls";

  default:
    return "unknown";
  }
}

static char * TMWDEFS_LOCAL _toDataType(TMWTYPES_ULONG type) 
{ 
  switch(type)
  {
  case DNPDEFS_DATASET_TYPE_VSTR: 
    return "vstr";

  case DNPDEFS_DATASET_TYPE_UINT: 
    return "uint"; 

  case DNPDEFS_DATASET_TYPE_INT: 
    return "int";
 
  case DNPDEFS_DATASET_TYPE_FLT: 
    return "flt";

  case DNPDEFS_DATASET_TYPE_OSTR: 
    return "ostr";

  case DNPDEFS_DATASET_TYPE_BSTR: 
    return "bstr";

  case DNPDEFS_DATASET_TYPE_TIME: 
    return "time";

  case DNPDEFS_DATASET_TYPE_UNCD: 
    return "uncd";

  default: 
    return "none";

  }
} 
 
static TMWTYPES_BOOL TMWDEFS_LOCAL _toDatasetOutData(DNPDATA_DATASET_VALUE *pDataSetValue, DNPDEFS_DATASET_TYPE_CODE dataType, TMWTYPES_UCHAR *pOut, int maxLength)
{
  int length;
  TMWTYPES_BOOL hexValues = TMWDEFS_FALSE;
  TMWTYPES_CHAR tempBuf[2048];

  switch(pDataSetValue->type)
  {
    case DNPDATA_VALUE_STRPTR:   
      length = pDataSetValue->length;
      if(dataType == DNPDEFS_DATASET_TYPE_VSTR)
      {
        if(length+1 > maxLength)
          length = maxLength-1;
        memcpy(tempBuf, pDataSetValue->value.pStrValue, length); 
      }
      else
      {  
        int j;
        int len = 0;
        for(j=0; j<length; j++)
        { 
          if(len < (maxLength-4))
          {
          len += tmwtarg_snprintf((tempBuf + len), sizeof(tempBuf)-len, "%02x ", pDataSetValue->value.pStrValue[j]);
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

        memcpy(tempBuf, pDataSetValue->value.strValue, length); 
      }
      else
      {  
        int j;
        int len = 0;
        for(j=0; j<length; j++)
        {
          if(len < (maxLength-4))
          {
            len += tmwtarg_snprintf((tempBuf + len), sizeof(tempBuf)-len, "%02x ", pDataSetValue->value.strValue[j]);
          }
        }
        length = len;
        hexValues = TMWDEFS_TRUE;
      }
      break;    
    case DNPDATA_VALUE_UINT32:    
      length = tmwtarg_snprintf(tempBuf, maxLength, "%u",pDataSetValue->value.uint32Value);
      break;
    case DNPDATA_VALUE_INT32:  
      length = tmwtarg_snprintf(tempBuf, maxLength, "%d",pDataSetValue->value.int32Value); 
      break;
    case DNPDATA_VALUE_SFLT:  
      length = tmwtarg_snprintf(tempBuf, maxLength, "%g",pDataSetValue->value.sfltValue);  
      break;
    case DNPDATA_VALUE_DOUBLE: 
      length = tmwtarg_snprintf(tempBuf, maxLength, "%g",pDataSetValue->value.doubleValue);  
      break;
    case DNPDATA_VALUE_TIME: 
      length = tmwtarg_snprintf(tempBuf, maxLength, "time");  
      break;
    default:
      *pOut = 0;
      length = 0;
      return hexValues;
      break;
  }  
  if(length > 0)
  {
    memcpy(pOut, tempBuf, length);
    *(pOut+length) = 0;
  }

  return hexValues;
}

static TMWTYPES_CHAR * TMWDEFS_LOCAL _buildDatasetProtoPoint(TMWTYPES_CHAR *result, void *pDbHandle, TMWTYPES_USHORT pointNum, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  TMWTYPES_CHAR buf[256];
  void *pPoint;
  pPoint = sdnpdata_datasetProtoGetPoint(pDbHandle, pointNum); 
  if(pPoint != TMWDEFS_NULL)
  {
    TMWTYPES_CHAR *desc;
    DNPDATA_DATASET_DESCR_ELEM *pDescrElems; 
    TMWTYPES_UCHAR uuid[16];
    TMWTYPES_UCHAR name[256];
    int i;
    int len; 
    TMWTYPES_UCHAR numberElems;

    pDescrElems = sdnpdata_datasetProtoRead(pPoint, &numberElems, (TMWTYPES_UCHAR*)&uuid);  
 
    result = _appendStringLen(result, "   <dataSetProtoData>\n", curLen, bGetLen);

    tmwtarg_snprintf(buf, sizeof(buf), "    <pointNumber>%d</pointNumber>\n", pointNum);
    result = _appendStringLen(result, buf, curLen, bGetLen);
    
    desc = sdnpdata_datasetGetDescription(pPoint);
    if(desc && (strlen(desc) > 0))
    {
      tmwsim_xmlFormatDesc(desc, buf, sizeof(buf));
      result = _appendStringLen(result, buf, curLen, bGetLen);
    }

    len = 0;
    for(i=0; i< 16; i++)
      len += tmwtarg_snprintf((char *)name+len, sizeof(name)-len, "%02x ", uuid[i]);
    
    tmwtarg_snprintf(buf, sizeof(buf), "    <uuid>%s</uuid>\n", name);
    result = _appendStringLen(result, buf, curLen, bGetLen);

    for(i=0; i<numberElems; i++ )
    {
      result = _appendStringLen(result, "    <dataSetDescrElement>\n", curLen, bGetLen);

      tmwtarg_snprintf(buf, sizeof(buf), "     <descrElemType>%s</descrElemType>\n", _toDescrType(pDescrElems[i].descrElemType));
      result = _appendStringLen(result, buf, curLen, bGetLen);

      tmwtarg_snprintf(buf, sizeof(buf), "     <dataType>%s</dataType>\n", _toDataType(pDescrElems[i].dataTypeCode));
      result = _appendStringLen(result, buf, curLen, bGetLen);

      tmwtarg_snprintf(buf, sizeof(buf), "     <maxDataLength>%d</maxDataLength>\n", pDescrElems[i].maxDataLength);
      result = _appendStringLen(result, buf, curLen, bGetLen);
      
      _toDatasetOutData(&pDescrElems[i].ancillaryValue, DNPDEFS_DATASET_TYPE_VSTR, name, 256);
      tmwtarg_snprintf(buf, sizeof(buf), "     <value>%s</value>\n", name);

      result = _appendStringLen(result, buf, curLen, bGetLen);

      result = _appendStringLen(result, "    </dataSetDescrElement>\n", curLen, bGetLen); 
    }

    result = _appendStringLen(result, "   </dataSetProtoData>\n", curLen, bGetLen);
  }
  return result;
}

static TMWTYPES_CHAR * TMWDEFS_LOCAL _buildDatasetDescrPoint(TMWTYPES_CHAR *result, void *pDbHandle, TMWTYPES_USHORT pointNum, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  void *pPoint;
  TMWTYPES_CHAR *desc;
  TMWTYPES_CHAR buf[2048];

  pPoint = sdnpdata_datasetDescrGetPoint(pDbHandle, pointNum); 
  if(pPoint != TMWDEFS_NULL)
  {
    TMWDEFS_CLASS_MASK classMask;
    DNPDATA_DATASET_DESCR_ELEM *pDescrElems; 
    DNPDATA_DATASET_DESCR_INDEX *pIndexElems;
    TMWTYPES_UCHAR name[256];
    int i;
    TMWTYPES_UCHAR characteristics; 
    TMWTYPES_UCHAR numberElems; 

    pDescrElems = sdnpdata_datasetDescrReadCont(pPoint, &numberElems);
    classMask = sdnpdata_datasetDescrEventClass(pPoint);

    result = _appendStringLen(result, "   <dataSetDescrData>\n", curLen, bGetLen);

    desc = sdnpdata_datasetGetDescription(pPoint);
    if(desc && (strlen(desc) > 0))
    {
      tmwsim_xmlFormatDesc(desc, buf, sizeof(buf));
      result = _appendStringLen(result, buf, curLen, bGetLen);
    }

    tmwtarg_snprintf(buf, sizeof(buf), "    <pointNumber>%d</pointNumber>\n", pointNum);
    result = _appendStringLen(result, buf, curLen, bGetLen);

    result = _putClass(result, classMask, curLen, bGetLen);

    sdnpdata_datasetDescrReadChars(pPoint, &characteristics);
    tmwtarg_snprintf(buf, sizeof(buf), "    <characteristics>%d</characteristics>\n", characteristics);
    result = _appendStringLen(result, buf, curLen, bGetLen);

    for(i=0; i<numberElems; i++ )
    {
      result = _appendStringLen(result, "    <dataSetDescrElement>\n", curLen, bGetLen);

      tmwtarg_snprintf(buf, sizeof(buf), "     <descrElemType>%s</descrElemType>\n", _toDescrType(pDescrElems[i].descrElemType));
      result = _appendStringLen(result, buf, curLen, bGetLen);

      tmwtarg_snprintf(buf, sizeof(buf), "     <dataType>%s</dataType>\n", _toDataType(pDescrElems[i].dataTypeCode));
      result = _appendStringLen(result, buf, curLen, bGetLen);

      tmwtarg_snprintf(buf, sizeof(buf), "     <maxDataLength>%d</maxDataLength>\n", pDescrElems[i].maxDataLength);
      result = _appendStringLen(result, buf, curLen, bGetLen);
  
      if(pDescrElems[i].descrElemType == DNPDEFS_DATASET_DESCR_UUID
        || pDescrElems[i].descrElemType == DNPDEFS_DATASET_DESCR_PTYP)
      {
        _toDatasetOutData(&pDescrElems[i].ancillaryValue, DNPDEFS_DATASET_TYPE_OSTR, name, 256);
        tmwtarg_snprintf(buf, sizeof(buf), "     <hexValue>%s</hexValue>\n", name);
      }
      else
      {
        _toDatasetOutData(&pDescrElems[i].ancillaryValue, DNPDEFS_DATASET_TYPE_VSTR, name, 256);
        tmwtarg_snprintf(buf, sizeof(buf), "     <value>%s</value>\n", name); 
      }
      result = _appendStringLen(result, buf, curLen, bGetLen);

      result = _appendStringLen(result, "    </dataSetDescrElement>\n", curLen, bGetLen); 
    }
 
    pIndexElems = sdnpdata_datasetDescrReadIndex(pPoint, &numberElems); 

    for(i=0; i<numberElems; i++ )
    {
      result = _appendStringLen(result, "    <dataSetPointIndexAttribute>\n", curLen, bGetLen);

      tmwtarg_snprintf(buf, sizeof(buf), "     <pointType>%d</pointType>\n", pIndexElems[i].pointType);
      result = _appendStringLen(result, buf, curLen, bGetLen);

      tmwtarg_snprintf(buf, sizeof(buf), "     <pointIndex>%d</pointIndex>\n", pIndexElems[i].pointIndex);
      result = _appendStringLen(result, buf, curLen, bGetLen);
 
      result = _appendStringLen(result, "    </dataSetPointIndexAttribute>\n", curLen, bGetLen); 
    }

    result = _appendStringLen(result, "   </dataSetDescrData>\n", curLen, bGetLen);
  }
  return result;
}

static void TMWDEFS_LOCAL _datasetDescrExpanded(void *pDbHandle, TMWTYPES_USHORT pointNum, DNPDEFS_DATASET_TYPE_CODE *pDataTypeArray)
{
  void *pPoint = sdnpdata_datasetDescrGetPoint(pDbHandle, pointNum); 
  if(pPoint != TMWDEFS_NULL)
  {
    int i;
    TMWTYPES_UCHAR numberElems; 
    int expandedIndex = 0;
    DNPDATA_DATASET_DESCR_ELEM *pDescrElem = sdnpdata_datasetDescrReadCont(pPoint, &numberElems);

    for(i=1; i<numberElems; i++)
    {
      if( (pDescrElem->descrElemType == DNPDEFS_DATASET_DESCR_DAEL)
        ||(pDescrElem->descrElemType == DNPDEFS_DATASET_DESCR_CTLS)
        ||(pDescrElem->descrElemType == DNPDEFS_DATASET_DESCR_CTLV))
      {
        pDataTypeArray[expandedIndex++] = pDescrElem->dataTypeCode;
      } 
      else if(pDescrElem->descrElemType == DNPDEFS_DATASET_DESCR_PTYP)
      {
        /* expand out prototype */
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
          for(j=2; j<numberProtoElems; j++)
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

static TMWTYPES_CHAR * TMWDEFS_LOCAL _buildDatasetDataPoint(TMWTYPES_CHAR *result, void *pDbHandle, TMWTYPES_USHORT pointNum, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  TMWTYPES_CHAR buf[2048];
  void *pPoint;
  pPoint = sdnpdata_datasetGetPoint(pDbHandle, pointNum); 
  if(pPoint != TMWDEFS_NULL)
  {
    int i;
    DNPDATA_DATASET_VALUE *pDataElems; 
    DNPDEFS_DATASET_TYPE_CODE dataTypeArray[256];
    TMWTYPES_UCHAR dataBuf[2048]; 
    TMWTYPES_UCHAR numberElems; 
    TMWDTIME timeStamp;

    /* Init this in case _datasetDescrExpanded fails to set array properly */
    memset(dataTypeArray, 0, sizeof(dataTypeArray));
  
    pDataElems = sdnpdata_datasetRead(pPoint, &numberElems, &timeStamp);
    _datasetDescrExpanded(pDbHandle, pointNum, dataTypeArray);

    result = _appendStringLen(result, "   <dataSetData>\n", curLen, bGetLen);

    tmwtarg_snprintf(buf, sizeof(buf), "    <pointNumber>%d</pointNumber>\n", pointNum);
    result = _appendStringLen(result, buf, curLen, bGetLen);

    for(i=0; i<numberElems; i++ )
    {
      result = _appendStringLen(result, "    <dataSetElement>\n", curLen, bGetLen);

      tmwtarg_snprintf(buf, sizeof(buf), "     <length>%d</length>\n", pDataElems[i].length);
      result = _appendStringLen(result, buf, curLen, bGetLen); 

      if(_toDatasetOutData(&pDataElems[i], dataTypeArray[i], dataBuf, 2020))
        tmwtarg_snprintf(buf, sizeof(buf), "     <hexValue>%s</hexValue>\n", dataBuf);
      else
        tmwtarg_snprintf(buf, sizeof(buf), "     <value>%s</value>\n", dataBuf);  
      result = _appendStringLen(result, buf, curLen, bGetLen);
   
      result = _appendStringLen(result, "    </dataSetElement>\n", curLen, bGetLen); 
    }

    result = _appendStringLen(result, "   </dataSetData>\n", curLen, bGetLen);
  }
  return result;
}
#endif


#if SDNPDATA_SUPPORT_OBJ110
static TMWTYPES_CHAR * TMWDEFS_LOCAL _buildStringPoint(TMWTYPES_CHAR *result, void *pDbHandle, TMWTYPES_USHORT pointNum, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  void *pPoint;
  TMWTYPES_BOOL isExtendedString;
  TMWTYPES_CHAR buf[2048];

  isExtendedString = TMWDEFS_FALSE;

  pPoint = sdnpdata_strGetPoint(pDbHandle, pointNum);
  if(pPoint != TMWDEFS_NULL)
  {
    TMWDEFS_CLASS_MASK classMask;
    TMWTYPES_UCHAR strBuf[TMWSIM_STRING_MAX_LENGTH+1];
    TMWTYPES_UCHAR strLength;
    TMWTYPES_CHAR *desc;

    desc = sdnpdata_strGetDescription(pPoint);
    sdnpdata_strRead(pPoint, TMWSIM_STRING_MAX_LENGTH, strBuf, &strLength);

#if SDNPDATA_SUPPORT_OBJ111
    classMask = sdnpdata_strEventClass(pPoint);
#else
    classMask = TMWDEFS_CLASS_MASK_NONE;
#endif

    result = _appendStringLen(result, "   <stringData>\n", curLen, bGetLen);

    tmwtarg_snprintf(buf, sizeof(buf), "    <pointNumber>%d</pointNumber>\n", pointNum);
    result = _appendStringLen(result, buf, curLen, bGetLen);

    if(desc && (strlen(desc) > 0))
    {
      tmwsim_xmlFormatDesc(desc, buf, sizeof(buf));
      result = _appendStringLen(result, buf, curLen, bGetLen);
    }

    result = _putClass(result, classMask, curLen, bGetLen);

    strBuf[strLength] = '\0';
    tmwsim_xmlFormatValue((TMWTYPES_CHAR *)strBuf, strLength, buf, sizeof(buf));
    result = _appendStringLen(result, buf, curLen, bGetLen);

    result = _putBoolField(result, "    ", "extended", isExtendedString, curLen, bGetLen);

#if SDNPDATA_SUPPORT_EVENT_MODE_POINT && SDNPDATA_SUPPORT_OBJ111
    tmwtarg_snprintf(buf, sizeof(buf), "    <eventMode>%s</eventMode>\n", _eventModeString(sdnpdata_strEventMode(pPoint)));
    result = _appendStringLen(result, buf, curLen, bGetLen);
#endif

    result = _appendStringLen(result, "   </stringData>\n", curLen, bGetLen);
  }
#if SDNPDATA_SUPPORT_OBJ114
  else
  { 
      isExtendedString = TMWDEFS_TRUE;
      pPoint = sdnpsim_extStrGetPoint(pDbHandle, pointNum);
      if (pPoint != TMWDEFS_NULL)
      {
        TMWDEFS_CLASS_MASK classMask;
        TMWTYPES_UCHAR flags;
        TMWTYPES_USHORT strLength;
        TMWTYPES_CHAR *desc; 
        TMWTYPES_UCHAR *ptr;
#if !SDNPDATA_SUPPORT_OBJ114_MIN_STACK 
        TMWTYPES_UCHAR strBuf[1024+1];
#endif

        desc = sdnpdata_extStrGetDescription(pPoint);

#if SDNPDATA_SUPPORT_OBJ114_MIN_STACK 
        ptr = sdnpdata_extStrGetPtr(pPoint, &strLength, &flags); 
         
        void TMWDEFS_GLOBAL sdnpdata_extStrRelease(
          void *pPoint);
#else 
        ptr = strBuf;
        sdnpdata_extStrRead(pPoint, 1024, ptr, &strLength, &flags);
#endif

        classMask = sdnpdata_extStrEventClass(pPoint);

        result = _appendStringLen(result, "   <stringData>\n", curLen, bGetLen);

        tmwtarg_snprintf(buf, sizeof(buf), "    <pointNumber>%d</pointNumber>\n", pointNum);
        result = _appendStringLen(result, buf, curLen, bGetLen);

        if (desc && (strlen(desc) > 0))
        {
          tmwsim_xmlFormatDesc(desc, buf, sizeof(buf));
          result = _appendStringLen(result, buf, curLen, bGetLen);
        }

        result = _putClass(result, classMask, curLen, bGetLen);

        ptr[strLength] = '\0';
        tmwsim_xmlFormatValue((TMWTYPES_CHAR *)ptr, strLength, buf, sizeof(buf));
        result = _appendStringLen(result, buf, curLen, bGetLen);

        result = _putBoolField(result, "    ", "extended", isExtendedString, curLen, bGetLen);
        if (isExtendedString)
        {
          result = _appendStringLen(result, "    <flags>\n", curLen, bGetLen);
          result = _putBoolField(result, "     ", "online", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_ON_LINE), curLen, bGetLen);
          result = _putBoolField(result, "     ", "restart", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_RESTART), curLen, bGetLen);
          result = _putBoolField(result, "     ", "commLost", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_COMM_LOST), curLen, bGetLen);
          result = _putBoolField(result, "     ", "remoteForced", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_REMOTE_FORCED), curLen, bGetLen);
          result = _putBoolField(result, "     ", "localForced", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_LOCAL_FORCED), curLen, bGetLen);
          result = _putBoolField(result, "     ", "overflow", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_OVERFLOW), curLen, bGetLen);
          result = _appendStringLen(result, "    </flags>\n", curLen, bGetLen);
        }

#if SDNPDATA_SUPPORT_EVENT_MODE_POINT
        tmwtarg_snprintf(buf, sizeof(buf), "    <eventMode>%s</eventMode>\n", _eventModeString(sdnpdata_extStrEventMode(pPoint)));
        result = _appendStringLen(result, buf, curLen, bGetLen);
#endif

        result = _appendStringLen(result, "   </stringData>\n", curLen, bGetLen);
      }
  }
#endif
  return result;
}
#endif

#if SDNPDATA_SUPPORT_OBJ112
static TMWTYPES_CHAR * TMWDEFS_LOCAL _buildVtermPoint(TMWTYPES_CHAR *result, void *pDbHandle, TMWTYPES_USHORT pointNum, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  TMWTYPES_CHAR buf[320];
  void *pPoint;
  pPoint = sdnpdata_vtermGetPoint(pDbHandle, pointNum);
  if(pPoint != TMWDEFS_NULL)
  {
    TMWDEFS_CLASS_MASK classMask;
    TMWTYPES_UCHAR strBuf[255];
    TMWTYPES_UCHAR strLength;
    TMWTYPES_CHAR *desc;

    desc = sdnpdata_vtermGetDescription(pPoint);
    sdnpdata_vtermRead(pPoint, sizeof(strBuf), strBuf, &strLength);
#if SDNPDATA_SUPPORT_OBJ113
    classMask = classMask = sdnpdata_vtermEventClass(pPoint);
#else
    classMask = TMWDEFS_CLASS_MASK_NONE;
#endif

    result = _appendStringLen(result, "   <virtualTerminal>\n", curLen, bGetLen);

    tmwtarg_snprintf(buf, sizeof(buf), "    <pointNumber>%d</pointNumber>\n", pointNum);
    result = _appendStringLen(result, buf, curLen, bGetLen);

    if(desc && (strlen(desc) > 0))
    {
      tmwsim_xmlFormatDesc(desc, buf, sizeof(buf));
      result = _appendStringLen(result, buf, curLen, bGetLen);
    }
    
    result = _putClass(result, classMask, curLen, bGetLen);

    strBuf[strLength] = '\0';
    tmwtarg_snprintf(buf, sizeof(buf), "    <value>%s</value>\n", strBuf);
    result = _appendStringLen(result, buf, curLen, bGetLen);

    result = _appendStringLen(result, "   </virtualTerminal>\n", curLen, bGetLen);
  }
  return result;
}
#endif

#if SDNPDATA_SUPPORT_OBJ120 && SDNPCNFG_SUPPORT_SA_VERSION5
static TMWTYPES_CHAR * TMWDEFS_LOCAL _buildAuthSecStatPoint(TMWTYPES_CHAR *result, void *pDbHandle, TMWTYPES_USHORT pointNum, TMWTYPES_ULONG *curLen, TMWTYPES_BOOL bGetLen)
{
  TMWTYPES_CHAR buf[256];
  void *pPoint;
  pPoint = sdnpdata_authSecStatGetPoint(pDbHandle, pointNum);
  if(pPoint != TMWDEFS_NULL)
  {
    TMWDEFS_CLASS_MASK classMask;
    TMWTYPES_ULONG value;
    TMWTYPES_UCHAR flags;
    TMWTYPES_CHAR *desc = TMWDEFS_NULL;

    /* desc = sdnpdata_authSecStatGetDescription(pPoint); */
    sdnpdata_authSecStatRead(pPoint, &value, &flags);
    classMask = sdnpdata_authSecStatEventClass(pPoint);

    result = _appendStringLen(result, "   <authSecStat>\n", curLen, bGetLen);

    tmwtarg_snprintf(buf, sizeof(buf), "    <pointNumber>%d</pointNumber>\n", pointNum);
    result = _appendStringLen(result, buf, curLen, bGetLen);
    
    result = _appendStringLen(result, "    <fromOS>true</fromOS>\n", curLen, bGetLen); 

    if(desc && (strlen(desc) > 0))
    {
      tmwsim_xmlFormatDesc(desc, buf, sizeof(buf));
      result = _appendStringLen(result, buf, curLen, bGetLen);
    }
    
    result = _putClass(result, classMask, curLen, bGetLen);

    result = _appendStringLen(result, "    <flags>\n", curLen, bGetLen);
    result = _putBoolField(result, "     ", "online", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_ON_LINE), curLen, bGetLen);
    result = _putBoolField(result, "     ", "restart", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_RESTART), curLen, bGetLen);
    result = _putBoolField(result, "     ", "commLost", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_COMM_LOST), curLen, bGetLen);
    result = _putBoolField(result, "     ", "remoteForced", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_REMOTE_FORCED), curLen, bGetLen);
    result = _putBoolField(result, "     ", "localForced", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_LOCAL_FORCED), curLen, bGetLen);
    result = _putBoolField(result, "     ", "rollover", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_CNTR_ROLLOVER), curLen, bGetLen);
    result = _putBoolField(result, "     ", "discontinuity", TMWDEFS_TOBOOL(flags, DNPDEFS_DBAS_FLAG_REFERENCE_CHK), curLen, bGetLen);
    result = _appendStringLen(result, "    </flags>\n", curLen, bGetLen);

    tmwtarg_snprintf(buf, sizeof(buf), "     <value>%u</value>\n", value);
    result = _appendStringLen(result, buf, curLen, bGetLen);

#if SDNPDATA_SUPPORT_EVENT_VAR_POINT
    tmwtarg_snprintf(buf, sizeof(buf), "    <defaultEventVariation>%d</defaultEventVariation>\n", sdnpdata_authSecStatEventDefVariation(pPoint, TMWDEFS_CLASS_MASK_ALL));
    result = _appendStringLen(result, buf, curLen, bGetLen);
#endif

    /*tmwtarg_snprintf(buf, sizeof(buf), "    <defaultStaticVariation>%d</defaultStaticVariation>\n", sdnpdata_authSecStatDefVariation(pPoint));
     *result = _appendStringLen(result, buf, curLen, bGetLen);
     */

#if SDNPDATA_SUPPORT_EVENT_MODE_POINT
    tmwtarg_snprintf(buf, sizeof(buf), "    <eventMode>%s</eventMode>\n", _eventModeString(sdnpdata_authSecStatEventMode(pPoint)));
    result = _appendStringLen(result, buf, curLen, bGetLen);
#endif

    result = _appendStringLen(result, "  </authSecStat>\n", curLen, bGetLen);
  }
  return result;
}
#endif

/* routine: sdnpxml_saveDatabaseStatic */
SDNPXML_SAVE_STATE TMWDEFS_GLOBAL sdnpxml_saveDatabaseStatic(TMWSESN *pSession, TMWTYPES_CHAR *result, TMWTYPES_ULONG maxLen, TMWTYPES_ULONG *bufLen)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  SDNPSIM_DATABASE *pSimDatabase = (SDNPSIM_DATABASE *)pSDNPSession->pDbHandle;
  TMWTYPES_BOOL loopControl;
  TMWTYPES_ULONG curLen;
  TMWTYPES_USHORT numPoints;
  TMWTYPES_USHORT pointNum;
   
  curLen=0;

  /* Loop till a break is reached */
  loopControl = TMWDEFS_TRUE;
  while(loopControl)
  {
    switch (pSimDatabase->xmlSaveState)
    {
    case SDNPXML_SAVE_HEADER:
      _buildHeader(result, &curLen, TMWDEFS_TRUE);
      if (curLen>=maxLen)
      {
        return SDNPXML_SAVE_NOT_DONE;
      }
      result = _buildHeader(result, &curLen, TMWDEFS_FALSE);
      *bufLen = curLen;
      pSimDatabase->xmlSaveState = SDNPXML_SAVE_DEVICE_ATTR;
      continue;
      break;
      
    case SDNPXML_SAVE_DEVICE_ATTR:
#if SDNPDATA_SUPPORT_OBJ0
      numPoints= sdnpdata_deviceAttrQuantity(pSDNPSession->pDbHandle); 
      for(pointNum = pSimDatabase->xmlCurSavePointNum; pointNum < numPoints; pointNum++)
      {
        TMWTYPES_UCHAR variation = pSimDatabase->xmlCurSaveDAVariation;
        do
        { 
          _buildDeviceAttrPoint(result,pSDNPSession->pDbHandle, pointNum, &variation, &curLen, TMWDEFS_TRUE);
          if (curLen>=maxLen)
          {
            return SDNPXML_SAVE_NOT_DONE;
          }
          /* variation gets advanced in _buildDeviceAttrPoint, set it back to read and store that variation */
          variation = pSimDatabase->xmlCurSaveDAVariation;
          result = _buildDeviceAttrPoint(result,pSDNPSession->pDbHandle, pointNum, &variation, &curLen, TMWDEFS_FALSE);
          *bufLen = curLen;
          pSimDatabase->xmlCurSavePointNum = pointNum;
          pSimDatabase->xmlCurSaveDAVariation = variation;
        } while (variation != 255);
        
        pSimDatabase->xmlCurSavePointNum = pointNum+1;
        pSimDatabase->xmlCurSaveDAVariation = 0;
      }
      *bufLen = curLen;
#endif
      pSimDatabase->xmlSaveState = SDNPXML_SAVE_DEVICE_ATTR_TAIL;
      pSimDatabase->xmlCurSavePointNum = 0;
      pSimDatabase->xmlCurSaveDAVariation = 0;
      continue;
      break;

    case SDNPXML_SAVE_DEVICE_ATTR_TAIL:
      _buildDeviceAttrTail(result, &curLen, TMWDEFS_TRUE);
      if (curLen>=maxLen)
      {
        return SDNPXML_SAVE_NOT_DONE;
      }
      result = _buildDeviceAttrTail(result, &curLen, TMWDEFS_FALSE);
      *bufLen = curLen;
      pSimDatabase->xmlSaveState = SDNPXML_SAVE_BIN_IN;
      continue;
      break;

    case SDNPXML_SAVE_BIN_IN:
#if SDNPDATA_SUPPORT_OBJ1
      numPoints= sdnpdata_binInQuantity(pSDNPSession->pDbHandle);
      for(pointNum = pSimDatabase->xmlCurSavePointNum; pointNum < numPoints; pointNum++)
      {
        _buildBinInPoint(result,pSDNPSession->pDbHandle,pointNum, &curLen, TMWDEFS_TRUE);
        if (curLen>=maxLen)
        {
          return SDNPXML_SAVE_NOT_DONE;
        }
        result = _buildBinInPoint(result,pSDNPSession->pDbHandle,pointNum, &curLen, TMWDEFS_FALSE);
        *bufLen = curLen;
        pSimDatabase->xmlCurSavePointNum = pointNum+1;
      }
      *bufLen = curLen;
#endif
      pSimDatabase->xmlSaveState = SDNPXML_SAVE_BIN_IN_TAIL;
      pSimDatabase->xmlCurSavePointNum = 0;
      continue;
      break;

    case SDNPXML_SAVE_BIN_IN_TAIL:
      _buildBinInTail(result, &curLen, TMWDEFS_TRUE);
      if (curLen>=maxLen)
      {
        return SDNPXML_SAVE_NOT_DONE;
      }
      result = _buildBinInTail(result, &curLen, TMWDEFS_FALSE);
      *bufLen = curLen;
      pSimDatabase->xmlSaveState = SDNPXML_SAVE_DBL_IN;
      continue;
      break;
      
    case SDNPXML_SAVE_DBL_IN:
#if SDNPDATA_SUPPORT_OBJ3
      numPoints= sdnpdata_dblInQuantity(pSDNPSession->pDbHandle);
      for(pointNum = pSimDatabase->xmlCurSavePointNum; pointNum < numPoints; pointNum++)
      {
        _buildDblBitInPoint(result,pSDNPSession->pDbHandle,pointNum, &curLen, TMWDEFS_TRUE);
        if (curLen>=maxLen)
        {
          return SDNPXML_SAVE_NOT_DONE;
        }
        result = _buildDblBitInPoint(result,pSDNPSession->pDbHandle,pointNum, &curLen, TMWDEFS_FALSE);
        *bufLen = curLen;
        pSimDatabase->xmlCurSavePointNum = pointNum+1;
      }
      *bufLen = curLen;
#endif
      pSimDatabase->xmlSaveState = SDNPXML_SAVE_DBL_IN_TAIL;
      pSimDatabase->xmlCurSavePointNum = 0;
      continue;
      break;

    case SDNPXML_SAVE_DBL_IN_TAIL:
      _buildDblBitInTail(result, &curLen, TMWDEFS_TRUE);
      if (curLen>=maxLen)
      {
        return SDNPXML_SAVE_NOT_DONE;
      }
      result = _buildDblBitInTail(result, &curLen, TMWDEFS_FALSE);
      *bufLen = curLen;
      pSimDatabase->xmlSaveState = SDNPXML_SAVE_BIN_OUT;
      continue;
      break;

    case SDNPXML_SAVE_BIN_OUT:
#if SDNPDATA_SUPPORT_OBJ12
      numPoints= sdnpdata_binOutQuantity(pSDNPSession->pDbHandle);
      for(pointNum = pSimDatabase->xmlCurSavePointNum; pointNum < numPoints; pointNum++)
      {
        _buildBinOutPoint(result,pSDNPSession->pDbHandle,pointNum, &curLen, TMWDEFS_TRUE);
        if (curLen>=maxLen)
        {
          return SDNPXML_SAVE_NOT_DONE;
        }
        result = _buildBinOutPoint(result,pSDNPSession->pDbHandle,pointNum, &curLen, TMWDEFS_FALSE);
        *bufLen = curLen;
        pSimDatabase->xmlCurSavePointNum = pointNum+1;
      }
      *bufLen = curLen;
#endif
      pSimDatabase->xmlSaveState = SDNPXML_SAVE_BIN_OUT_TAIL;
      pSimDatabase->xmlCurSavePointNum = 0;
      continue;
      break;

    case SDNPXML_SAVE_BIN_OUT_TAIL:
      _buildBinOutTail(result, &curLen, TMWDEFS_TRUE);
      if (curLen>=maxLen)
      {
        return SDNPXML_SAVE_NOT_DONE;
      }
      result = _buildBinOutTail(result, &curLen, TMWDEFS_FALSE);
      *bufLen = curLen;
      pSimDatabase->xmlSaveState = SDNPXML_SAVE_BIN_OUT_STATUS;
      continue;
      break;

    case SDNPXML_SAVE_BIN_OUT_STATUS:
#if SDNPDATA_SUPPORT_OBJ10
      numPoints= sdnpdata_binOutQuantity(pSDNPSession->pDbHandle);
      for(pointNum = pSimDatabase->xmlCurSavePointNum; pointNum < numPoints; pointNum++)
      {
        _buildBinOutStatusPoint(result,pSDNPSession->pDbHandle,pointNum, &curLen, TMWDEFS_TRUE);
        if (curLen>=maxLen)
        {
          return SDNPXML_SAVE_NOT_DONE;
        }
        result = _buildBinOutStatusPoint(result,pSDNPSession->pDbHandle,pointNum, &curLen, TMWDEFS_FALSE);
        *bufLen = curLen;
        pSimDatabase->xmlCurSavePointNum = pointNum+1;
      }
      *bufLen = curLen;
#endif
      pSimDatabase->xmlSaveState = SDNPXML_SAVE_BIN_OUT_STATUS_TAIL;
      pSimDatabase->xmlCurSavePointNum = 0;
      continue;
      break;

    case SDNPXML_SAVE_BIN_OUT_STATUS_TAIL:
      _buildBinOutStatusTail(result, &curLen, TMWDEFS_TRUE);
      if (curLen>=maxLen)
      {
        return SDNPXML_SAVE_NOT_DONE;
      }
      result = _buildBinOutStatusTail(result, &curLen, TMWDEFS_FALSE);
      *bufLen = curLen;
      pSimDatabase->xmlSaveState = SDNPXML_SAVE_BIN_COUNTER;
      continue;
      break;

    case SDNPXML_SAVE_BIN_COUNTER:
#if SDNPDATA_SUPPORT_OBJ20
      numPoints= sdnpdata_binCntrQuantity(pSDNPSession->pDbHandle);
      for(pointNum = pSimDatabase->xmlCurSavePointNum; pointNum < numPoints; pointNum++)
      {
        _buildBinCounterPoint(result,pSDNPSession->pDbHandle,pointNum, &curLen, TMWDEFS_TRUE);
        if (curLen>=maxLen)
        {
          return SDNPXML_SAVE_NOT_DONE;
        }
        result = _buildBinCounterPoint(result,pSDNPSession->pDbHandle,pointNum, &curLen, TMWDEFS_FALSE);
        *bufLen = curLen;
        pSimDatabase->xmlCurSavePointNum = pointNum+1;
      }
      *bufLen = curLen;
#endif
      pSimDatabase->xmlSaveState = SDNPXML_SAVE_BIN_COUNTER_TAIL;
      pSimDatabase->xmlCurSavePointNum = 0;
      continue;
      break;

    case SDNPXML_SAVE_BIN_COUNTER_TAIL:
      _buildBinCounterTail(result, &curLen, TMWDEFS_TRUE);
      if (curLen>=maxLen)
      {
        return SDNPXML_SAVE_NOT_DONE;
      }
      result = _buildBinCounterTail(result, &curLen, TMWDEFS_FALSE);
      *bufLen = curLen;
      pSimDatabase->xmlSaveState = SDNPXML_SAVE_FROZEN_COUNTER;
      continue;
      break;

    case SDNPXML_SAVE_FROZEN_COUNTER:
#if SDNPDATA_SUPPORT_OBJ21
      numPoints= sdnpdata_binCntrQuantity(pSDNPSession->pDbHandle);
      for(pointNum = pSimDatabase->xmlCurSavePointNum; pointNum < numPoints; pointNum++)
      {
        _buildFrozenCounterPoint(result,pSDNPSession->pDbHandle,pointNum, &curLen, TMWDEFS_TRUE);
        if (curLen>=maxLen)
        {
          return SDNPXML_SAVE_NOT_DONE;
        }
        result = _buildFrozenCounterPoint(result,pSDNPSession->pDbHandle,pointNum, &curLen, TMWDEFS_FALSE);
        *bufLen = curLen;
        pSimDatabase->xmlCurSavePointNum = pointNum+1;
      }
      *bufLen = curLen;
#endif
      pSimDatabase->xmlSaveState = SDNPXML_SAVE_FROZEN_COUNTER_TAIL;
      pSimDatabase->xmlCurSavePointNum = 0;
      continue;
      break;

    case SDNPXML_SAVE_FROZEN_COUNTER_TAIL:
      _buildFrozenCounterTail(result, &curLen, TMWDEFS_TRUE);
      if (curLen>=maxLen)
      {
        return SDNPXML_SAVE_NOT_DONE;
      }
      result = _buildFrozenCounterTail(result, &curLen, TMWDEFS_FALSE);
      *bufLen = curLen;
      pSimDatabase->xmlSaveState = SDNPXML_SAVE_ANALOG_IN;
      continue;
      break;

    case SDNPXML_SAVE_ANALOG_IN:
#if SDNPDATA_SUPPORT_OBJ30
      numPoints= sdnpdata_anlgInQuantity(pSDNPSession->pDbHandle);
      for(pointNum = pSimDatabase->xmlCurSavePointNum; pointNum < numPoints; pointNum++)
      {
        _buildAnalogInPoint(result,pSDNPSession->pDbHandle,pointNum, &curLen, TMWDEFS_TRUE);
        if (curLen>=maxLen)
        {
          return SDNPXML_SAVE_NOT_DONE;
        }
        result = _buildAnalogInPoint(result,pSDNPSession->pDbHandle,pointNum, &curLen, TMWDEFS_FALSE);
        *bufLen = curLen;
        pSimDatabase->xmlCurSavePointNum = pointNum+1;
      }
      *bufLen = curLen;
#endif
      pSimDatabase->xmlSaveState = SDNPXML_SAVE_ANALOG_IN_TAIL;
      pSimDatabase->xmlCurSavePointNum = 0;
      continue;
      break;

    case SDNPXML_SAVE_ANALOG_IN_TAIL:
      _buildAnalogInTail(result, &curLen, TMWDEFS_TRUE);
      if (curLen>=maxLen)
      {
        return SDNPXML_SAVE_NOT_DONE;
      }
      result = _buildAnalogInTail(result, &curLen, TMWDEFS_FALSE);
      *bufLen = curLen;
      pSimDatabase->xmlSaveState = SDNPXML_SAVE_FRZNANALOG_IN;
      continue;
      break;

    case SDNPXML_SAVE_FRZNANALOG_IN:
#if SDNPDATA_SUPPORT_OBJ31
      numPoints = sdnpdata_frznAnlgInQuantity(pSDNPSession->pDbHandle);
      for (pointNum = pSimDatabase->xmlCurSavePointNum; pointNum < numPoints; pointNum++)
      {
        _buildFrznAnalogInPoint(result, pSDNPSession->pDbHandle, pointNum, &curLen, TMWDEFS_TRUE);
        if (curLen >= maxLen)
        {
          return SDNPXML_SAVE_NOT_DONE;
        }
        result = _buildFrznAnalogInPoint(result, pSDNPSession->pDbHandle, pointNum, &curLen, TMWDEFS_FALSE);
        *bufLen = curLen;
        pSimDatabase->xmlCurSavePointNum = pointNum + 1;
      }
      *bufLen = curLen;
#endif
      pSimDatabase->xmlSaveState = SDNPXML_SAVE_FRZNANALOG_IN_TAIL;
      pSimDatabase->xmlCurSavePointNum = 0;
      continue;
      break;

    case SDNPXML_SAVE_FRZNANALOG_IN_TAIL:
      _buildFrznAnalogInTail(result, &curLen, TMWDEFS_TRUE);
      if (curLen >= maxLen)
      {
        return SDNPXML_SAVE_NOT_DONE;
      }
      result = _buildFrznAnalogInTail(result, &curLen, TMWDEFS_FALSE);
      *bufLen = curLen;
      pSimDatabase->xmlSaveState = SDNPXML_SAVE_ANALOG_IN_DB;
      continue;
      break;

    case SDNPXML_SAVE_ANALOG_IN_DB:
#if SDNPDATA_SUPPORT_OBJ34
      numPoints= sdnpdata_anlgInQuantity(pSDNPSession->pDbHandle);
      for(pointNum = pSimDatabase->xmlCurSavePointNum; pointNum < numPoints; pointNum++)
      {
        _buildAnalogInDBandPoint(result,pSDNPSession->pDbHandle,pointNum, &curLen, TMWDEFS_TRUE);
        if (curLen>=maxLen)
        {
          return SDNPXML_SAVE_NOT_DONE;
        }
        result = _buildAnalogInDBandPoint(result,pSDNPSession->pDbHandle,pointNum, &curLen, TMWDEFS_FALSE);
        *bufLen = curLen;
        pSimDatabase->xmlCurSavePointNum = pointNum+1;
      }
      *bufLen = curLen;
#endif
      pSimDatabase->xmlSaveState = SDNPXML_SAVE_ANALOG_IN_DB_TAIL;
      pSimDatabase->xmlCurSavePointNum = 0;
      continue;
      break;

    case SDNPXML_SAVE_ANALOG_IN_DB_TAIL:
      _buildAnalogInDbandTail(result, &curLen, TMWDEFS_TRUE);
      if (curLen>=maxLen)
      {
        return SDNPXML_SAVE_NOT_DONE;
      }
      result = _buildAnalogInDbandTail(result, &curLen, TMWDEFS_FALSE);
      *bufLen = curLen;
      pSimDatabase->xmlSaveState = SDNPXML_SAVE_ANALOG_OUT;
      continue;
      break;

    case SDNPXML_SAVE_ANALOG_OUT:
#if SDNPDATA_SUPPORT_OBJ40 || SDNPDATA_SUPPORT_OBJ41
      numPoints= sdnpdata_anlgOutQuantity(pSDNPSession->pDbHandle);
      for(pointNum = pSimDatabase->xmlCurSavePointNum; pointNum < numPoints; pointNum++)
      {
        _buildAnalogOutPoint(result,pSDNPSession->pDbHandle,pointNum, &curLen, TMWDEFS_TRUE);
        if (curLen>=maxLen)
        {
          return SDNPXML_SAVE_NOT_DONE;
        }
        result = _buildAnalogOutPoint(result,pSDNPSession->pDbHandle,pointNum, &curLen, TMWDEFS_FALSE);
        *bufLen = curLen;
        pSimDatabase->xmlCurSavePointNum = pointNum+1;
      }
      *bufLen = curLen;
#endif
      pSimDatabase->xmlSaveState = SDNPXML_SAVE_ANALOG_OUT_TAIL;
      pSimDatabase->xmlCurSavePointNum = 0;
      continue;
      break;

    case SDNPXML_SAVE_ANALOG_OUT_TAIL:
      _buildAnalogOutTail(result, &curLen, TMWDEFS_TRUE);
      if (curLen>=maxLen)
      {
        return SDNPXML_SAVE_NOT_DONE;
      }
      result = _buildAnalogOutTail(result, &curLen, TMWDEFS_FALSE);
      *bufLen = curLen;
      pSimDatabase->xmlSaveState = SDNPXML_SAVE_ANALOG_OUT_STATUS;
      continue;
      break;

    case SDNPXML_SAVE_ANALOG_OUT_STATUS:
#if SDNPDATA_SUPPORT_OBJ40
      numPoints= sdnpdata_anlgOutQuantity(pSDNPSession->pDbHandle);
      for(pointNum = pSimDatabase->xmlCurSavePointNum; pointNum < numPoints; pointNum++)
      {
        _buildAnalogOutStatusPoint(result,pSDNPSession->pDbHandle,pointNum, &curLen, TMWDEFS_TRUE);
        if (curLen>=maxLen)
        {
          return SDNPXML_SAVE_NOT_DONE;
        }
        result = _buildAnalogOutStatusPoint(result,pSDNPSession->pDbHandle,pointNum, &curLen, TMWDEFS_FALSE);
        *bufLen = curLen;
        pSimDatabase->xmlCurSavePointNum = pointNum+1;
      }
      *bufLen = curLen;
#endif
      pSimDatabase->xmlSaveState = SDNPXML_SAVE_ANALOG_OUT_STATUS_TAIL;
      pSimDatabase->xmlCurSavePointNum = 0;
      continue;
      break;

    case SDNPXML_SAVE_ANALOG_OUT_STATUS_TAIL:
      _buildAnalogOutStatusTail(result, &curLen, TMWDEFS_TRUE);
      if (curLen>=maxLen)
      {
        return SDNPXML_SAVE_NOT_DONE;
      }
      result = _buildAnalogOutStatusTail(result, &curLen, TMWDEFS_FALSE);
      *bufLen = curLen;
      pSimDatabase->xmlSaveState = SDNPXML_SAVE_INDEXED_TIME;
      continue;
      break;

    case SDNPXML_SAVE_INDEXED_TIME:
#if SDNPDATA_SUPPORT_OBJ50_V4
      numPoints = sdnpdata_indexedTimeQuantity(pSDNPSession->pDbHandle);
      for (pointNum = pSimDatabase->xmlCurSavePointNum; pointNum < numPoints; pointNum++)
      {
        _buildIndexedTimePoint(result, pSDNPSession->pDbHandle, pointNum, &curLen, TMWDEFS_TRUE);
        if (curLen >= maxLen)
        {
          return SDNPXML_SAVE_NOT_DONE;
        }
        result = _buildIndexedTimePoint(result, pSDNPSession->pDbHandle, pointNum, &curLen, TMWDEFS_FALSE);
        *bufLen = curLen;
        pSimDatabase->xmlCurSavePointNum = pointNum + 1;
      }
      *bufLen = curLen;
#endif
      pSimDatabase->xmlSaveState = SDNPXML_SAVE_INDEXED_TIME_TAIL;
      pSimDatabase->xmlCurSavePointNum = 0;
      continue;
      break;

    case SDNPXML_SAVE_INDEXED_TIME_TAIL:
      _buildIndexedTimeTail(result, &curLen, TMWDEFS_TRUE);
      if (curLen >= maxLen)
      {
        return SDNPXML_SAVE_NOT_DONE;
      }
      result = _buildIndexedTimeTail(result, &curLen, TMWDEFS_FALSE);
      *bufLen = curLen;
      pSimDatabase->xmlSaveState = SDNPXML_SAVE_DATASET_PROTO;
      continue;
      break;
         
    case SDNPXML_SAVE_DATASET_PROTO:
#if SDNPDATA_SUPPORT_DATASETS
      numPoints= sdnpdata_datasetProtoQuantity(pSDNPSession->pDbHandle);
      for(pointNum = pSimDatabase->xmlCurSavePointNum; pointNum < numPoints; pointNum++)
      {
        _buildDatasetProtoPoint(result,pSDNPSession->pDbHandle,pointNum, &curLen, TMWDEFS_TRUE);
        if (curLen>=maxLen)
        {
          /* If this data set descriptor is too big to fit in the buffer, skip it */
          if(*bufLen == 0)
          {
            SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_XML_SIZE_ERROR);
            curLen = *bufLen;
            continue;
          }
          return SDNPXML_SAVE_NOT_DONE;
        }
        result = _buildDatasetProtoPoint(result,pSDNPSession->pDbHandle,pointNum, &curLen, TMWDEFS_FALSE);
        *bufLen = curLen;
        pSimDatabase->xmlCurSavePointNum = pointNum+1;
      }
      *bufLen = curLen;
#endif
      pSimDatabase->xmlSaveState = SDNPXML_SAVE_DATASET_PROTO_TAIL;
      pSimDatabase->xmlCurSavePointNum = 0;
      continue;
      break;

    case SDNPXML_SAVE_DATASET_PROTO_TAIL:
      _buildDatasetProtoTail(result, &curLen, TMWDEFS_TRUE);
      if (curLen>=maxLen)
      {
        return SDNPXML_SAVE_NOT_DONE;
      }
      result = _buildDatasetProtoTail(result, &curLen, TMWDEFS_FALSE);
      *bufLen = curLen;
      pSimDatabase->xmlSaveState = SDNPXML_SAVE_DATASET_DESCR;
      continue;
      break; 

    case SDNPXML_SAVE_DATASET_DESCR:
#if SDNPDATA_SUPPORT_DATASETS
      numPoints= sdnpdata_datasetDescrQuantity(pSDNPSession->pDbHandle);
      for(pointNum = pSimDatabase->xmlCurSavePointNum; pointNum < numPoints; pointNum++)
      {
        _buildDatasetDescrPoint(result,pSDNPSession->pDbHandle,pointNum, &curLen, TMWDEFS_TRUE);
        if (curLen>=maxLen)
        {
          /* If this data set descriptor is too big to fit in the buffer, skip it */
          if(*bufLen == 0)
          {
            SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_XML_SIZE_ERROR);
            curLen = *bufLen;
            continue;
          }
          return SDNPXML_SAVE_NOT_DONE;
        }
        result = _buildDatasetDescrPoint(result,pSDNPSession->pDbHandle,pointNum, &curLen, TMWDEFS_FALSE);
        *bufLen = curLen;
        pSimDatabase->xmlCurSavePointNum = pointNum+1;
      }
      *bufLen = curLen;
#endif
      pSimDatabase->xmlSaveState = SDNPXML_SAVE_DATASET_DESCR_TAIL;
      pSimDatabase->xmlCurSavePointNum = 0;
      continue;
      break;

    case SDNPXML_SAVE_DATASET_DESCR_TAIL:
      _buildDatasetDescrTail(result, &curLen, TMWDEFS_TRUE);
      if (curLen>=maxLen)
      {
        return SDNPXML_SAVE_NOT_DONE;
      }
      result = _buildDatasetDescrTail(result, &curLen, TMWDEFS_FALSE);
      *bufLen = curLen;
      pSimDatabase->xmlSaveState = SDNPXML_SAVE_DATASET_DATA;
      continue;
      break; 

    case SDNPXML_SAVE_DATASET_DATA:
#if SDNPDATA_SUPPORT_DATASETS
      numPoints= sdnpdata_datasetQuantity(pSDNPSession->pDbHandle);
      for(pointNum = pSimDatabase->xmlCurSavePointNum; pointNum < numPoints; pointNum++)
      {
        _buildDatasetDataPoint(result,pSDNPSession->pDbHandle,pointNum, &curLen, TMWDEFS_TRUE);
        if (curLen>=maxLen)
        {
          /* If this data set descriptor is too big to fit in the buffer, skip it */
          if(*bufLen == 0)
          {
            SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_XML_SIZE_ERROR);
            curLen = *bufLen;
            continue;
          }
          return SDNPXML_SAVE_NOT_DONE;
        }
        result = _buildDatasetDataPoint(result,pSDNPSession->pDbHandle,pointNum, &curLen, TMWDEFS_FALSE);
        *bufLen = curLen;
        pSimDatabase->xmlCurSavePointNum = pointNum+1;
      }
      *bufLen = curLen;
#endif
      pSimDatabase->xmlSaveState = SDNPXML_SAVE_DATASET_DATA_TAIL;
      pSimDatabase->xmlCurSavePointNum = 0;
      continue;
      break;

    case SDNPXML_SAVE_DATASET_DATA_TAIL:
      _buildDatasetDataTail(result, &curLen, TMWDEFS_TRUE);
      if (curLen>=maxLen)
      {
        return SDNPXML_SAVE_NOT_DONE;
      }
      result = _buildDatasetDataTail(result, &curLen, TMWDEFS_FALSE);
      *bufLen = curLen;
      pSimDatabase->xmlSaveState = SDNPXML_SAVE_STRING;
      continue;
      break; 

    case SDNPXML_SAVE_STRING:
#if SDNPDATA_SUPPORT_OBJ110
      numPoints= sdnpdata_strQuantity(pSDNPSession->pDbHandle);
      for(pointNum = pSimDatabase->xmlCurSavePointNum; pointNum < numPoints; pointNum++)
      {
        _buildStringPoint(result,pSDNPSession->pDbHandle,pointNum, &curLen, TMWDEFS_TRUE);
        if (curLen>=maxLen)
        {
          return SDNPXML_SAVE_NOT_DONE;
        }
        result = _buildStringPoint(result,pSDNPSession->pDbHandle,pointNum, &curLen, TMWDEFS_FALSE);
        *bufLen = curLen;
        pSimDatabase->xmlCurSavePointNum = pointNum+1;
      }
      *bufLen = curLen;
#endif
      pSimDatabase->xmlSaveState = SDNPXML_SAVE_STRING_TAIL;
      pSimDatabase->xmlCurSavePointNum = 0;
      continue;
      break;

    case SDNPXML_SAVE_STRING_TAIL:
      _buildStringTail(result, &curLen, TMWDEFS_TRUE);
      if (curLen>=maxLen)
      {
        return SDNPXML_SAVE_NOT_DONE;
      }
      result = _buildStringTail(result, &curLen, TMWDEFS_FALSE);
      *bufLen = curLen;
      pSimDatabase->xmlSaveState = SDNPXML_SAVE_VTERM;
      continue;
      break;

    case SDNPXML_SAVE_VTERM:
#if SDNPDATA_SUPPORT_OBJ112
      numPoints= sdnpdata_vtermQuantity(pSDNPSession->pDbHandle);
      for(pointNum = pSimDatabase->xmlCurSavePointNum; pointNum < numPoints; pointNum++)
      {
        _buildVtermPoint(result,pSDNPSession->pDbHandle,pointNum, &curLen, TMWDEFS_TRUE);
        if (curLen>=maxLen)
        {
          return SDNPXML_SAVE_NOT_DONE;
        }
        result = _buildVtermPoint(result,pSDNPSession->pDbHandle,pointNum, &curLen, TMWDEFS_FALSE);
        *bufLen = curLen;
        pSimDatabase->xmlCurSavePointNum = pointNum+1;
      }
      *bufLen = curLen;
#endif
      pSimDatabase->xmlSaveState = SDNPXML_SAVE_VTERM_TAIL;
      pSimDatabase->xmlCurSavePointNum = 0;
      continue;
      break;

    case SDNPXML_SAVE_VTERM_TAIL:
      _buildVtermTail(result, &curLen, TMWDEFS_TRUE);
      if (curLen>=maxLen)
      {
        return SDNPXML_SAVE_NOT_DONE;
      }
      result = _buildVtermTail(result, &curLen, TMWDEFS_FALSE);
      *bufLen = curLen;
#if SDNPCNFG_SUPPORT_SA_VERSION5
      pSimDatabase->xmlSaveState = SDNPXML_SAVE_AUTHSECSTAT;
#else
      pSimDatabase->xmlSaveState = SDNPXML_SAVE_TAIL;
#endif
      continue;
      break;

#if SDNPCNFG_SUPPORT_SA_VERSION5
    case SDNPXML_SAVE_AUTHSECSTAT:
#if SDNPDATA_SUPPORT_OBJ120
      numPoints= sdnpdata_authSecStatQuantity(pSDNPSession->pDbHandle);
      for(pointNum = pSimDatabase->xmlCurSavePointNum; pointNum < numPoints; pointNum++)
      {
        _buildAuthSecStatPoint(result,pSDNPSession->pDbHandle,pointNum, &curLen, TMWDEFS_TRUE);
        if (curLen>=maxLen)
        {
          return SDNPXML_SAVE_NOT_DONE;
        }
        result = _buildAuthSecStatPoint(result,pSDNPSession->pDbHandle,pointNum, &curLen, TMWDEFS_FALSE);
        *bufLen = curLen;
        pSimDatabase->xmlCurSavePointNum = pointNum+1;
      }
      *bufLen = curLen;
#endif
      pSimDatabase->xmlSaveState = SDNPXML_SAVE_TAIL;
      pSimDatabase->xmlCurSavePointNum = 0;
      continue;
      break;
#endif

    case SDNPXML_SAVE_TAIL:
      _buildTail(result, &curLen, TMWDEFS_TRUE);
      if (curLen>=maxLen)
      {
        return SDNPXML_SAVE_NOT_DONE;
      }
      result = _buildTail(result, &curLen, TMWDEFS_FALSE);
      *bufLen = curLen;
      return SDNPXML_SAVE_DONE;
      break;
    }
  }
  return(SDNPXML_SAVE_DONE);
}

TMWTYPES_ULONG TMWDEFS_GLOBAL sdnpxml_saveDatabaseStaticGetSize(TMWSESN *pSession)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  TMWTYPES_BOOL bGetLen = TMWDEFS_TRUE;
  TMWTYPES_ULONG curLen=0;
  TMWTYPES_USHORT numPoints;
  TMWTYPES_USHORT pointNum;
  TMWTYPES_CHAR *result=TMWDEFS_NULL;

  if(pSDNPSession == TMWDEFS_NULL)
    return(SDNPXML_SAVE_FAILED);

  _buildHeader(result, &curLen, bGetLen);
  
#if SDNPDATA_SUPPORT_OBJ0
  numPoints= sdnpdata_deviceAttrQuantity(pSDNPSession->pDbHandle);
  for(pointNum = 0; pointNum < numPoints; pointNum++)
  {
    TMWTYPES_UCHAR variation = 0;
    do
    {
      _buildDeviceAttrPoint(result,pSDNPSession->pDbHandle, pointNum, &variation, &curLen, bGetLen);
    } while (variation != 255);
  }
#endif
  _buildDeviceAttrTail(result, &curLen, bGetLen);

#if SDNPDATA_SUPPORT_OBJ1
  numPoints= sdnpdata_binInQuantity(pSDNPSession->pDbHandle);
  for(pointNum = 0; pointNum < numPoints; pointNum++)
  {
    _buildBinInPoint(result,pSDNPSession->pDbHandle,pointNum, &curLen, bGetLen);
  }
#endif
  _buildBinInTail(result, &curLen, bGetLen);

#if SDNPDATA_SUPPORT_OBJ3
  numPoints= sdnpdata_dblInQuantity(pSDNPSession->pDbHandle);
  for(pointNum = 0; pointNum < numPoints; pointNum++)
  {
    _buildDblBitInPoint(result,pSDNPSession->pDbHandle,pointNum, &curLen, bGetLen);
  }
#endif
  _buildDblBitInTail(result, &curLen, bGetLen);

#if SDNPDATA_SUPPORT_OBJ12
  numPoints= sdnpdata_binOutQuantity(pSDNPSession->pDbHandle);
  for(pointNum = 0; pointNum < numPoints; pointNum++)
  {
    result = _buildBinOutPoint(result,pSDNPSession->pDbHandle,pointNum, &curLen, bGetLen);
  }
#endif
  _buildBinOutTail(result, &curLen, bGetLen);
  
#if SDNPDATA_SUPPORT_OBJ10
  numPoints= sdnpdata_binOutQuantity(pSDNPSession->pDbHandle);
  for(pointNum = 0; pointNum < numPoints; pointNum++)
  {
    _buildBinOutStatusPoint(result,pSDNPSession->pDbHandle,pointNum, &curLen, bGetLen);
  }
#endif
  _buildBinOutStatusTail(result, &curLen, bGetLen);
  
#if SDNPDATA_SUPPORT_OBJ20
  numPoints= sdnpdata_binCntrQuantity(pSDNPSession->pDbHandle);
  for(pointNum = 0; pointNum < numPoints; pointNum++)
  {
    _buildBinCounterPoint(result,pSDNPSession->pDbHandle,pointNum, &curLen, bGetLen);
  }
#endif
  _buildBinCounterTail(result, &curLen, bGetLen);
  
#if SDNPDATA_SUPPORT_OBJ21
  numPoints= sdnpdata_binCntrQuantity(pSDNPSession->pDbHandle);
  for(pointNum = 0; pointNum < numPoints; pointNum++)
  {
    _buildFrozenCounterPoint(result,pSDNPSession->pDbHandle,pointNum, &curLen, bGetLen);
  }
#endif
  _buildFrozenCounterTail(result, &curLen, bGetLen);
  
#if SDNPDATA_SUPPORT_OBJ30
  numPoints= sdnpdata_anlgInQuantity(pSDNPSession->pDbHandle);
  for(pointNum = 0; pointNum < numPoints; pointNum++)
  {
    _buildAnalogInPoint(result,pSDNPSession->pDbHandle,pointNum, &curLen, bGetLen);
  }
#endif
  _buildAnalogInTail(result, &curLen, bGetLen);

#if SDNPDATA_SUPPORT_OBJ31
  numPoints = sdnpdata_frznAnlgInQuantity(pSDNPSession->pDbHandle);
  for (pointNum = 0; pointNum < numPoints; pointNum++)
  {
    _buildFrznAnalogInPoint(result, pSDNPSession->pDbHandle, pointNum, &curLen, bGetLen);
  }
#endif
  _buildFrznAnalogInTail(result, &curLen, bGetLen);
  
#if SDNPDATA_SUPPORT_OBJ34
  numPoints= sdnpdata_anlgInQuantity(pSDNPSession->pDbHandle);
  for(pointNum = 0; pointNum < numPoints; pointNum++)
  {
    _buildAnalogInDBandPoint(result,pSDNPSession->pDbHandle,pointNum, &curLen, bGetLen);
  }
#endif
  _buildAnalogInDbandTail(result, &curLen, bGetLen);
  
#if SDNPDATA_SUPPORT_OBJ40 || SDNPDATA_SUPPORT_OBJ41
  numPoints= sdnpdata_anlgOutQuantity(pSDNPSession->pDbHandle);
  for(pointNum = 0; pointNum < numPoints; pointNum++)
  {
    _buildAnalogOutPoint(result,pSDNPSession->pDbHandle,pointNum, &curLen, bGetLen);
  }
#endif
  _buildAnalogOutTail(result, &curLen, bGetLen);
  
#if SDNPDATA_SUPPORT_OBJ40
  numPoints= sdnpdata_anlgOutQuantity(pSDNPSession->pDbHandle);
  for(pointNum = 0; pointNum < numPoints; pointNum++)
  {
    _buildAnalogOutStatusPoint(result,pSDNPSession->pDbHandle,pointNum, &curLen, bGetLen);
  }
#endif
  _buildAnalogOutStatusTail(result, &curLen, bGetLen);

#if SDNPDATA_SUPPORT_DATASETS
  numPoints= sdnpdata_datasetProtoQuantity(pSDNPSession->pDbHandle);
  for(pointNum = 0; pointNum < numPoints; pointNum++)
  {
    _buildDatasetProtoPoint(result,pSDNPSession->pDbHandle,pointNum, &curLen, bGetLen);
  } 
#endif
  _buildDatasetProtoTail(result, &curLen, bGetLen);

#if SDNPDATA_SUPPORT_DATASETS
  numPoints= sdnpdata_datasetDescrQuantity(pSDNPSession->pDbHandle);
  for(pointNum = 0; pointNum < numPoints; pointNum++)
  {
    _buildDatasetDescrPoint(result,pSDNPSession->pDbHandle,pointNum, &curLen, bGetLen);
  } 
#endif
  _buildDatasetDescrTail(result, &curLen, bGetLen);
  
#if SDNPDATA_SUPPORT_DATASETS
  numPoints= sdnpdata_datasetQuantity(pSDNPSession->pDbHandle);
  for(pointNum = 0; pointNum < numPoints; pointNum++)
  {
    _buildDatasetDataPoint(result,pSDNPSession->pDbHandle,pointNum, &curLen, bGetLen);
  } 
#endif
  _buildDatasetDataTail(result, &curLen, bGetLen);

#if SDNPDATA_SUPPORT_OBJ110
  numPoints= sdnpdata_strQuantity(pSDNPSession->pDbHandle);
  for(pointNum = 0; pointNum < numPoints; pointNum++)
  {
    _buildStringPoint(result,pSDNPSession->pDbHandle,pointNum, &curLen, bGetLen);
  }
#endif
  _buildStringTail(result, &curLen, bGetLen);
  
#if SDNPDATA_SUPPORT_OBJ112
  numPoints= sdnpdata_vtermQuantity(pSDNPSession->pDbHandle);
  for(pointNum = 0; pointNum < numPoints; pointNum++)
  {
    _buildVtermPoint(result,pSDNPSession->pDbHandle,pointNum, &curLen, bGetLen);
  }
#endif
  _buildTail(result, &curLen, TMWDEFS_TRUE);

  return(curLen);
}

void TMWDEFS_GLOBAL sdnpxml_saveDatabaseStaticInit(TMWSESN *pSession)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  SDNPSIM_DATABASE *pSimDatabase = (SDNPSIM_DATABASE *)pSDNPSession->pDbHandle;

  pSimDatabase->xmlCurSavePointNum = 0;
  pSimDatabase->xmlCurSaveDAVariation = 0;
  pSimDatabase->xmlSaveState = SDNPXML_SAVE_HEADER;
}

#endif /* SDNPCNFG_SUPPORT_XML */
#endif /* TMWCNFG_USE_SIMULATED_DB */
