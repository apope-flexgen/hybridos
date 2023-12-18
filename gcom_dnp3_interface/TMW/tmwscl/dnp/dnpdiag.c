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

/* file: dnpdiag.h
 * description: Generic DNP diagnostics.
 */
#include "tmwscl/dnp/dnpdiag.h"
#include "tmwscl/utils/tmwtarg.h"
#include "tmwscl/utils/tmwcnfg.h"
#include "tmwscl/utils/tmwdefs.h"
#include "tmwscl/utils/tmwdiag.h"
#include "tmwscl/utils/tmwchnl.h"

#include "tmwscl/dnp/dnpdefs.h"
#include "tmwscl/dnp/dnpmem.h"
#include "tmwscl/dnp/dnpdtime.h"
#include "tmwscl/dnp/dnpauth.h"

#if TMWCNFG_SUPPORT_DIAG

/* Define constants used below */
#define MAX_ROW_LENGTH 16
#define DNPLINK_HEADER_LENGTH 10
#define DNPLINK_BLOCK_LENGTH 18

/* Error messages used by both master and slave DNP */
static const DNPDIAG_ERROR_ENTRY dnpDiagErrorMsg[] = {
  {DNPDIAG_INV_LENGTH        ,  "Parsing received object header, invalid message length"},
  {DNPDIAG_FIRST_LAST        ,  "Parsing received object header, first point > last point"},
  {DNPDIAG_INVALID_SIZE      ,  "Invalid message size received"},
  {DNPDIAG_INCR_TIMEOUT      ,  "Incremental timeout with no current fragment"},
  {DNPDIAG_INCR_TIMEOUT_REQ  ,  "Incremental timeout for request "},
  {DNPDIAG_RESP_TIMEOUT      ,  "Response timeout for request "},
  {DNPDIAG_CHANNEL_TIMEOUT   ,  "Channel timeout for request"},
  {DNPDIAG_PROC_FRAGMENT     ,  "Error processing received fragment"},
  {DNPDIAG_BAD_START         ,  "Frame discarded - Bad start characters"},
  {DNPDIAG_UNSUP_FC          ,  "Function code not supported"},
  {DNPDIAG_UNSUP_FC_RCVD     ,  "Link Layer Function code not supported was received"},
  {DNPDIAG_CRC_HEADER        ,  "Frame discarded - Invalid CRC in header"},
  {DNPDIAG_CRC_DATA          ,  "Frame discarded - Invalid CRC in data block"},
  {DNPDIAG_READ_MORE         ,  "Frame discarded - Read more bytes than requested"},
  {DNPDIAG_FRAME_OVERRUN     ,  "Frame discarded - Received Frame exceeds configured frame size"}, 
  {DNPDIAG_TRANSMIT_FAILED   ,  "Transmission failed for request "},
  {DNPDIAG_TOO_MANY_REQUESTS ,  "Too many requests, not inserted in transmit queue"},
  {DNPDIAG_DUPLICATE_REQ     ,  "Duplicate request removed from message queue"},
  {DNPDIAG_REQUEST_CANCELED  ,  "Request canceled "},
  {DNPDIAG_MASTER_SESSION    ,  "Could not find first master session"},
  {DNPDIAG_ALLOC_TX          ,  "Error allocating tx data structure"},
  {DNPDIAG_ALLOC_FRAG        ,  "Error allocating fragment buffer"},
  {DNPDIAG_INVALID_BUF_SIZE  ,  "Invalid buffer size"},
  {DNPDIAG_INACTIVE          ,  "Attempt to send frame to inactive session"},
  {DNPDIAG_WAITING           ,  "Attempt to send frame when waiting for link confirm"},
  {DNPDIAG_SECONDARY_BRDCST  ,  "Secondary frame received with broadcast address"},
  {DNPDIAG_INVALID_FRSIZE    ,  "Invalid frame sizes"},
  {DNPDIAG_LINK_NOT_RESET    ,  "Link not reset"},
  {DNPDIAG_INVALID_FCV       ,  "Invalid FCV"},
  {DNPDIAG_INVALID_FCB       ,  "Invalid frame count bit"},
  {DNPDIAG_BRDCST_DISABLED   ,  "Broadcast disabled, request not processed"},
  {DNPDIAG_FC_DISABLED       ,  "Function code disabled, request not processed"},
  {DNPDIAG_TEST_LINK_BRDCST  ,  "Test link states received with broadcast address"},
  {DNPDIAG_LINK_RESET_BRDCST ,  "Link reset received with broadcast address"},
  {DNPDIAG_LINK_STAT_BRDCST  ,  "Request link status received with broadcast address"},
  {DNPDIAG_UNRECOG_BRDCST    ,  "Unrecognized request received with broadcast address"},
  {DNPDIAG_RCV_TIMEOUT       ,  "Receive frame timeout expired, partial frame discarded"},
  {DNPDIAG_NEEDED_BYTES      ,  "Invalid state in getNeededBytes"},
  {DNPDIAG_PARSE_APPHEADER   ,  "Error parsing application header"},
  {DNPDIAG_PARSE_HEADER      ,  "Error parsing object header"},
  {DNPDIAG_FRAG_RESTART      ,  "Fragment restarted in the middle of receiving a fragment"},
  {DNPDIAG_FRAME_SESSION     ,  "Received frame for different session"},
  {DNPDIAG_TPRT_SEQ_INVALID  ,  "Invalid transport sequence number received"},
  {DNPDIAG_TPRT_SEGDISCARD   ,  "Received transport segment discarded"},
  {DNPDIAG_TPRT_DISCARD      ,  "Fragment received so far discarded"},
  {DNPDIAG_FRAG_SIZE         ,  "Received fragment exceeds configured fragment size"},
  {DNPDIAG_FRAME_SIZE        ,  "Invalid frame size specified"},
	{DNPDIAG_TX_BUFFER         ,  "Unable to allocate tx frame buffer"},
	{DNPDIAG_RX_BUFFER         ,  "Unable to allocate rx fragment buffer"},
	{DNPDIAG_QUANTITY_0        ,  "quantity = 0"},
	{DNPDIAG_QUALIFIER         ,  "Unsupported qualifier"},
  {DNPDIAG_INACTIVE_SESSION  ,  "Received frame for inactive session"},
  {DNPDIAG_READ_PROTO_FAIL   ,  "Failed to read data set prototype"},
  {DNPDIAG_DATASET_VALUE     ,  "Failed to parse data set value"},
  {DNPDIAG_DATASET_PROTO     ,  "Failed to parse data set, missing prototype"},
  {DNPDIAG_DATASET_DESCR     ,  "Failed to parse data set, missing descriptor"},
  {DNPDIAG_DATASET_ELEMS     ,  "Failed to parse data set, too many elements"},
  {DNPDIAG_DATASET_CTRLS     ,  "Failed to parse data set control, too many control values in group"},
  {DNPDIAG_DATASET_TYPE      ,  "Unknown data set value type from database"},
  {DNPDIAG_DATASET_MANDATORY ,  "Missing mandatory element in received data set message"},
  {DNPDIAG_DATASET_FAIL      ,  "Failed to create data set point"},
  {DNPDIAG_ATTRIBUTE_VALUE   ,  "Failed to parse attribute value"},
  {DNPDIAG_AUTH_RXBADMSG     ,  "Secure Authentication, received invalid message"},
  {DNPDIAG_AUTH_RXBADSEQ     ,  "Secure Authentication, received unexpected sequence"},
  {DNPDIAG_AUTH_BADUSER      ,  "Secure Authentication, invalid user number"},
  {DNPDIAG_AUTH_APPLTIMEOUT  ,  "Secure Authentication, application timeout"},
  {DNPDIAG_AUTH_CHALLENGERPLY,  "Secure Authentication, challenge reply MAC Value failure"},
  {DNPDIAG_AUTH_KEYNOTOK     ,  "Secure Authentication, challenge reply or aggressive mode failure, Key Not Valid"},
  {DNPDIAG_AUTH_AGGRNOCHALL  ,  "Secure Authentication, rcvd aggressive mode without a previous challenge"}, 
  {DNPDIAG_AUTH_AGGRRXNOV9   ,  "Secure Authentication, rcvd aggressive mode, cannot find g120v9 aggressive mode MAC object"}, 
  {DNPDIAG_AUTH_AGGRRXFAIL   ,  "Secure Authentication, rcvd aggressive mode object MAC Value failure"}, 
  {DNPDIAG_AUTH_AGGRTXFAIL   ,  "Secure Authentication, can't send aggressive mode message until at least one challenge is received"}, 
  {DNPDIAG_AUTH_MACFAIL      ,  "Secure Authentication, MAC Value generation failure"}, 
  {DNPDIAG_AUTH_MACSUPPORT   ,  "Secure Authentication, MAC Algorithm not supported"}, 
  {DNPDIAG_AUTH_MACDEFAULT   ,  "Secure Authentication, MAC Algorithm not supported by remote device, reverting to default"}, 
  {DNPDIAG_AUTH_KEYWRAPDEFAULT, "Secure Authentication, Key Wrap Algorithm not supported by remote device, reverting to default"}, 
  {DNPDIAG_AUTH_FAIL         ,  "Secure Authentication, Failed, does remote device support Authentication?"}, 
  {DNPDIAG_AUTH_TOO_BIG      ,  "Secure Authentication, Failed, rcvd message is too large"},
  {DNPDIAG_AUTH_FINDUSERNAME ,  "Secure Authentication, can't get user name for this user number"},
  {DNPDIAG_AUTH_USERNAME     ,  "Secure Authentication, Failed, user name does not match"}, 
  {DNPDIAG_AUTH_GETKEYFAIL   ,  "Secure Authentication, get key failed"}, 
  {DNPDIAG_AUTH_BADCHALLENGE ,  "Secure Authentication, Failed, challenge data does not match"},
  {DNPDIAG_AUTH_BADCLENGTH   ,  "Secure Authentication, Failed, challenge data length is invalid"},
  {DNPDIAG_AUTH_BADMAC       ,  "Secure Authentication, Failed, MAC value does not match"}, 
  {DNPDIAG_AUTH_DECRYPT      ,  "Secure Authentication, Failed, decryption failed"},
  {DNPDIAG_AUTH_UPDATE_FAILED,  "Secure Authentication, Failed key update"},
  {DNPDIAG_AUTH_UNKNOWN_USR  ,  "Secure Authentication, Unknown User" },
  {DNPDIAG_AUTH_FORCEDERROR  ,  "Secure Authentication, Forced Error condition for testing"}, 

  {DNPDIAG_ERROR_ENUM_MAX    ,  ""}
};

/* Array to determine if specific error messages are disabled.
 */
static TMWTYPES_UCHAR _errorMsgDisabled[(DNPDIAG_ERROR_ENUM_MAX/8)+1];


/* Local Functions */

/* function: _objectToHdrId
 * purpose: return the appropriate diag header id based on object group
 * arguments:
 *  object - object group id
 * returns:
 *  diagnostic header id
 */
static TMWDIAG_ID TMWDEFS_LOCAL _objectToHdrId(TMWTYPES_UCHAR object)
{
  switch(object)
  {
  case DNPDEFS_OBJ_2_BIN_CHNG_EVENTS:
  case DNPDEFS_OBJ_4_DBL_CHNG_EVENTS:
  case DNPDEFS_OBJ_11_BIN_OUT_EVENTS:  
  case DNPDEFS_OBJ_13_BIN_CMD_EVENTS:  
  case DNPDEFS_OBJ_22_CNTR_EVENTS:
  case DNPDEFS_OBJ_23_FCTR_EVENTS:
  case DNPDEFS_OBJ_32_ANA_CHNG_EVENTS:
  case DNPDEFS_OBJ_42_ANA_OUT_EVENTS:
  case DNPDEFS_OBJ_43_ANA_CMD_EVENTS:
  case DNPDEFS_OBJ_70_FILE_EVENTS:
  case DNPDEFS_OBJ_88_DATASET_EVENTS:
  case DNPDEFS_OBJ_111_STRING_EVENTS:
  case DNPDEFS_OBJ_113_VTERM_EVENTS:
  case DNPDEFS_OBJ_115_EXT_STR_EVENTS:
    return(TMWDIAG_ID_EVENT_HDRS);
   
  case DNPDEFS_OBJ_120_AUTHENTICATION:
    return(TMWDIAG_ID_SECURITY_HDRS);

  case DNPDEFS_OBJ_121_AUTHSECSTATS:
    return(TMWDIAG_ID_SECURITY_HDRS | TMWDIAG_ID_STATIC_HDRS);

  case DNPDEFS_OBJ_122_AUTHSTATEVENTS:
    return(TMWDIAG_ID_SECURITY_HDRS | TMWDIAG_ID_EVENT_HDRS);

  default:
    return(TMWDIAG_ID_STATIC_HDRS);
  }
}

/* function: _fcString
 * purpose: return string representation of function code
 * arguments:
 *  fc - function code
 * returns:
 *  constant string describing function code
 */
static const char * TMWDEFS_LOCAL _fcString(
  TMWTYPES_UCHAR fc)
{
  switch(fc)
  {
  case DNPDEFS_FC_CONFIRM:            return("Application Confirmation");
  case DNPDEFS_FC_READ:               return("Read Request");
  case DNPDEFS_FC_WRITE:              return("Write Request");
  case DNPDEFS_FC_SELECT:             return("Select");
  case DNPDEFS_FC_OPERATE:            return("Operate");
  case DNPDEFS_FC_DIRECT_OP:          return("Direct Operate");
  case DNPDEFS_FC_DIRECT_OP_NOACK:    return("Direct Operate No Ack");
  case DNPDEFS_FC_FRZ:                return("Immediate Freeze");
  case DNPDEFS_FC_FRZ_NOACK:          return("Immediate Freeze No Ack");
  case DNPDEFS_FC_FRZ_CLEAR:          return("Freeze and Clear");
  case DNPDEFS_FC_FRZ_CLEAR_NOACK:    return("Freeze and Clear No Ack");
  case DNPDEFS_FC_FRZ_TIME:           return("Freeze with Time");
  case DNPDEFS_FC_FRZ_TIME_NOACK:     return("Freeze with Time No Ack");
  case DNPDEFS_FC_COLD_RESTART:       return("Cold Restart");
  case DNPDEFS_FC_WARM_RESTART:       return("Warm Restart");
  case DNPDEFS_FC_INIT_DATA_TO_DFLTS: return("Init Data To Defaults");
  case DNPDEFS_FC_INIT_APPLICATION:   return("Initialize Application");
  case DNPDEFS_FC_START_APPLICATION:  return("Start Application");
  case DNPDEFS_FC_STOP_APPLICATION:   return("Stop Application");
  case DNPDEFS_FC_SAVE_CONFIGURATION: return("Save Configuration");
  case DNPDEFS_FC_ENABLE_UNSOL:       return("Enable Unsolicited Messages");
  case DNPDEFS_FC_DISABLE_UNSOL:      return("Disable Unsolicited Messages");
  case DNPDEFS_FC_ASSIGN_CLASS:       return("Assign Class");
  case DNPDEFS_FC_DELAY_MEASURE:      return("Delay Measurement");
  case DNPDEFS_FC_RECORD_CURRENT_TIME: return("Record Current Time");
  case DNPDEFS_FC_OPEN_FILE:          return("Open File");
  case DNPDEFS_FC_CLOSE_FILE:         return("Close File");
  case DNPDEFS_FC_DELETE_FILE:        return("Delete File");
  case DNPDEFS_FC_GET_FILE_INFO:      return("Get File Info");
  case DNPDEFS_FC_AUTHENTICATE:       return("File Authenticate");
  case DNPDEFS_FC_ACTIVATE_CONFIG:    return("Activate Configuration");
  case DNPDEFS_FC_AUTH_REQUEST:       return("Authentication Request");
  case DNPDEFS_FC_AUTH_REQUEST_NOACK: return("Authentication Request No Ack");
  case DNPDEFS_FC_ABORT:              return("Abort");
  case DNPDEFS_FC_RESPONSE:           return("Response");
  case DNPDEFS_FC_UNSOLICITED:        return("Unsolicited");
  case DNPDEFS_FC_AUTH_RESPONSE:      return("Authentication Response");
  case DNPDEFS_FC_AUTH_SAV6_HDR:      return("Authentication SPDU");
  default:                            return("Unknown");
  }
}

/* function: _groupString
 * purpose: return string representation of object group
 * arguments:
 *  group - object group id
 * returns:
 *  constant string describing group
 */
static const char * TMWDEFS_LOCAL _groupString(
  TMWTYPES_UCHAR group)
{
  switch(group)
  {
  case DNPDEFS_OBJ_0_DEVICE_ATTRIBUTES:   return("Device Attribute");
  case DNPDEFS_OBJ_1_BIN_INPUTS:          return("Binary Input");
  case DNPDEFS_OBJ_2_BIN_CHNG_EVENTS:     return("Binary Input Change");
  case DNPDEFS_OBJ_3_DBL_INPUTS:          return("Double Bit Input");
  case DNPDEFS_OBJ_4_DBL_CHNG_EVENTS:     return("Double Bit Input Change");
  case DNPDEFS_OBJ_10_BIN_OUT_STATUSES:   return("Binary Output");
  case DNPDEFS_OBJ_11_BIN_OUT_EVENTS:     return("Binary Output Event");
  case DNPDEFS_OBJ_13_BIN_CMD_EVENTS:     return("Binary Output Command Event");
  case DNPDEFS_OBJ_12_BIN_OUT_CTRLS:      return("Control Relay Output Block");
  case DNPDEFS_OBJ_20_RUNNING_CNTRS:      return("Binary Counter");
  case DNPDEFS_OBJ_21_FROZEN_CNTRS:       return("Frozen Counter");
  case DNPDEFS_OBJ_22_CNTR_EVENTS:        return("Counter Change Event");
  case DNPDEFS_OBJ_23_FCTR_EVENTS:        return("Frozen Counter Event");
  case DNPDEFS_OBJ_30_ANA_INPUTS:         return("Analog Input");
  case DNPDEFS_OBJ_31_FRZN_ANA_INPUTS:    return("Frozen Analog Input");
  case DNPDEFS_OBJ_32_ANA_CHNG_EVENTS:    return("Analog Change Event");
  case DNPDEFS_OBJ_33_FRZN_ANA_EVENTS:    return("Frozen Analog Event");
  case DNPDEFS_OBJ_34_ANA_INPUT_DBANDS:   return("Analog Input Deadband");
  case DNPDEFS_OBJ_40_ANA_OUT_STATUSES:   return("Analog Output");
  case DNPDEFS_OBJ_41_ANA_OUT_CTRLS:      return("Analog Output Block");
  case DNPDEFS_OBJ_42_ANA_OUT_EVENTS:     return("Analog Output Event");
  case DNPDEFS_OBJ_43_ANA_CMD_EVENTS:     return("Analog Output Command Event");
  case DNPDEFS_OBJ_50_TIME_AND_DATE:      return("Time and Date");
  case DNPDEFS_OBJ_51_TIME_DATE_CTO:      return("Common Time of Occurrence");
  case DNPDEFS_OBJ_52_TIME_DELAY:         return("Time Delay");
  case DNPDEFS_OBJ_60_CLASS_SCANS:        return("Class Data");
  case DNPDEFS_OBJ_70_FILE_IDENTIFIER:    return("File Identifier");
  case DNPDEFS_OBJ_80_IIN_BITS:           return("Internal Indications");
  case DNPDEFS_OBJ_85_DATASET_PROTO:      return("Data Set Prototype");
  case DNPDEFS_OBJ_86_DATASET_DESCR:      return("Data Set Descriptor");
  case DNPDEFS_OBJ_87_DATASET_VALUE:      return("Data Set Present Value"); 
  case DNPDEFS_OBJ_88_DATASET_EVENTS:     return("Data Set Snapshot Event"); 
  case DNPDEFS_OBJ_91_ACTIVATE_STATUS:    return("Activate Config Status"); 
  case DNPDEFS_OBJ_110_STRING_DATA:       return("Octet String Object");
  case DNPDEFS_OBJ_111_STRING_EVENTS:     return("Octet String Event Object");
  case DNPDEFS_OBJ_112_VTERM_OUTPUT:      return("Virtual Terminal Output Block");
  case DNPDEFS_OBJ_113_VTERM_EVENTS:      return("Virtual Terminal Event Data");
  case DNPDEFS_OBJ_114_EXT_STR_DATA:      return("Extended Octet String Object");
  case DNPDEFS_OBJ_115_EXT_STR_EVENTS:    return("Extended Octet String Event Object");
  case DNPDEFS_OBJ_120_AUTHENTICATION:    return("Authentication");
  case DNPDEFS_OBJ_121_AUTHSECSTATS:      return("Authentication Security Statistics");
  case DNPDEFS_OBJ_122_AUTHSTATEVENTS:    return("Authentication Security Statistics Event");

  default:                                return("Unknown");
  }
}

/* function: _qualifierString
 * purpose: return string representation of qualifier
 * arguments:
 *  qualifier - DNP qualifier id
 * returns:
 *  constant string describing qualifier
 */
static const char * TMWDEFS_LOCAL _qualifierString(
  TMWTYPES_UCHAR qualifier)
{
  switch(qualifier)
  {
  case DNPDEFS_QUAL_8BIT_START_STOP:    return("8 Bit Start Stop");
  case DNPDEFS_QUAL_16BIT_START_STOP:   return("16 Bit Start Stop");
  case DNPDEFS_QUAL_ALL_POINTS:         return("All Points");
  case DNPDEFS_QUAL_8BIT_LIMITED_QTY:   return("8 Bit Limited Quantity");
  case DNPDEFS_QUAL_16BIT_LIMITED_QTY:  return("16 Bit Limited Quantity");
  case DNPDEFS_QUAL_8BIT_INDEX:         return("8 Bit Index");
  case DNPDEFS_QUAL_16BIT_INDEX:        return("16 Bit Index");
  case DNPDEFS_QUAL_16BIT_INDEX_8BITQ:  return("16 Bit Index, 8 Bit Quantity");
  case DNPDEFS_QUAL_8BIT_FREE_FORMAT:   return("8 Bit Free Format");
  case DNPDEFS_QUAL_16BIT_FREE_FORMAT:  return("16 Bit Free Format");
  default:                              return("Unknown");
  }
}

/* function: _getLinkControlString
 * purpose: return a string representation of the link level
 *  control to display in diagnostics messages.
 * arguments:
 *  ctrl - link level control field
 * returns:
 *  string to display
 */
static const char * TMWDEFS_LOCAL _getLinkControlString(
  TMWTYPES_UCHAR ctrl)
{
  if(ctrl & DNPDEFS_LC_PRM_MASK)
  {
    switch(ctrl & DNPDEFS_LC_FUNCTION_MASK)
    {
    case DNPDEFS_LCF_PRI_LINK_RESET:
      return("Primary Frame - Reset Link States");
    case DNPDEFS_LCF_PRI_RESET_USER:
      return("Primary Frame - Reset User (Obsolete)");
    case DNPDEFS_LCF_PRI_TEST_LINK:
      return("Primary Frame - Test Link States");
    case DNPDEFS_LCF_PRI_CNFRM_DATA:
      return("Primary Frame - Confirmed User Data");
    case DNPDEFS_LCF_PRI_UNCNFRM_DATA:
      return("Primary Frame - Unconfirmed User Data");
    case DNPDEFS_LCF_PRI_REQUEST_STATUS:
      return("Primary Frame - Request Link Status");
    }
  }
  else
  {
    switch(ctrl & DNPDEFS_LC_FUNCTION_MASK)
    {
    case DNPDEFS_LCF_SEC_CONFIRM_ACK:
      return("Secondary Frame - Acknowledge");
    case DNPDEFS_LCF_SEC_CONFIRM_NACK:
      return("Secondary Frame - Negative Acknowledge");
    case DNPDEFS_LCF_SEC_STATUS_OF_LINK:
      return("Secondary Frame - Link Status");
    case DNPDEFS_LCF_SEC_NOT_FUNCTIONING:
      return("Secondary Frame - Not Functioning (Obsolete)");
    case DNPDEFS_LCF_SEC_NOT_USED:
      return("Secondary Frame - Not Supported");
    }
  }

  return("Unknown frame");
}

/* function: _controlOpTypeString
 * purpose: return string representation of a Control Code Operation Type
 * arguments:
 *  control -  
 * returns:
 *  constant string describing control
 */
static const char * TMWDEFS_LOCAL _controlOpTypeString(
  TMWTYPES_UCHAR control)
{
  control = control & DNPDEFS_CROB_CTRL_MASK;
  if(control == 0)
    return("NUL");
  if(control == DNPDEFS_CROB_CTRL_PULSE_ON)
    return("PULSE_ON");
  if(control == DNPDEFS_CROB_CTRL_PULSE_OFF)
    return("PULSE_OFF");
  if(control == DNPDEFS_CROB_CTRL_LATCH_ON)
    return("LATCH_ON");
  if(control == DNPDEFS_CROB_CTRL_LATCH_OFF)
    return("LATCH_OFF");

  return("Undefined");
}

/* function: _controlQueueString
 * purpose: return string representation of a Control Code Queue field
 * arguments:
 *  control -  
 * returns:
 *  constant string describing control
 */
static const char * TMWDEFS_LOCAL _controlQueueString(
  TMWTYPES_UCHAR control)
{ 
  if(control & DNPDEFS_CROB_CTRL_QUEUE) 
    return("QUEUE");
    
  return(" ");
}
 
/* function: _controlClearString
 * purpose: return string representation of a Control Code Clear field
 * arguments:
 *  control -  
 * returns:
 *  constant string describing control
 */
static const char * TMWDEFS_LOCAL _controlClearString(
  TMWTYPES_UCHAR control)
{ 
  if(control & DNPDEFS_CROB_CTRL_CLEAR) 
    return("CLEAR");
    
  return(" ");
}

/* function: _controlTCCString
 * purpose: return string representation of a Control Code Trip-Close field
 * arguments:
 *  control -  
 * returns:
 *  constant string describing control
 */
static const char * TMWDEFS_LOCAL _controlTCCString(
  TMWTYPES_UCHAR control)
{ 
  control = control & DNPDEFS_CROB_CTRL_PAIRED_OP;
  if(control == 0) 
    return("NUL");
  if(control == DNPDEFS_CROB_CTRL_PAIRED_CLOSE) 
    return("CLOSE");
  if(control == DNPDEFS_CROB_CTRL_PAIRED_TRIP) 
    return("TRIP");
    
  return("RESERVED");
}
  
/* function: _controlStatusString
 * purpose: return string representation of CROB and Analog Command status
 * arguments: 
 *  status
 * returns:
 *  constant string describing status
 */
static const char * TMWDEFS_LOCAL _controlStatusString(
  TMWTYPES_UCHAR status)
{
  switch(status)
  {
  case DNPDEFS_CTLSTAT_SUCCESS:         return("SUCCESS"); 
  case DNPDEFS_CTLSTAT_TIMEOUT:         return("TIMEOUT"); 
  case DNPDEFS_CTLSTAT_NO_SELECT:       return("NO_SELECT"); 
  case DNPDEFS_CTLSTAT_FORMAT_ERROR:    return("FORMAT_ERROR"); 
  case DNPDEFS_CTLSTAT_NOT_SUPPORTED:   return("NOT_SUPPORTED"); 
  case DNPDEFS_CTLSTAT_ALREADY_ACTIVE:  return("ALREADY_ACTIVE"); 
  case DNPDEFS_CTLSTAT_HARDWARE_ERROR:  return("HARDWARE_ERROR");    
  case DNPDEFS_CTLSTAT_LOCAL:           return("LOCAL"); 
  case DNPDEFS_CTLSTAT_TOO_MANY_OPS:    return("TOO_MANY_OBJS");    
  case DNPDEFS_CTLSTAT_NOT_AUTHORIZED:  return("NOT_AUTHORIZED"); 
  case DNPDEFS_CTLSTAT_AUTO_INHIBIT:    return("AUTOMATION_INHIBIT");    
  case DNPDEFS_CTLSTAT_PROC_LIMITED:    return("PROCESSING_LIMITED"); 
  case DNPDEFS_CTLSTAT_OUT_OF_RANGE:    return("OUT_OF_RANGE");
  case DNPDEFS_CTLSTAT_DOWNSTRM_LOCAL:  return("DOWNSTREAM_LOCAL");   
  case DNPDEFS_CTLSTAT_ALR_COMPLETE:    return("ALREADY_COMPLETE");   
  case DNPDEFS_CTLSTAT_BLOCKED:         return("BLOCKED");   
  case DNPDEFS_CTLSTAT_CANCELLED:       return("CANCELLED");   
  case DNPDEFS_CTLSTAT_BLOCKED_OM:      return("BLOCKED_OTHER_MASTER");   
  case DNPDEFS_CTLSTAT_DOWNSTRM_FAIL:   return("DOWNSTREAM_FAIL");   
  case DNPDEFS_CTLSTAT_RESERVED:        return("RESERVED");  
  case DNPDEFS_CTLSTAT_UNDEFINED:       return("UNDEFINED");   
  default:                              return("RESERVED");   
  }
}

/* function: _isPrintable */
static TMWTYPES_BOOL TMWDEFS_LOCAL _isPrintable(
  TMWTYPES_UCHAR chr)
{
  if((chr<0x20) || (chr>0x7e))
  {
    return(TMWDEFS_FALSE);
  }
  return(TMWDEFS_TRUE);
}



/* function: _doubleBitStateString
 * purpose: return string representation of double bit state
 * arguments: 
 *  flags
 * returns:
 *  constant string describing double bit state
 */
static const char * TMWDEFS_LOCAL _doubleBitStateString(
  TMWTYPES_UCHAR flags)
{
  switch(flags & 0xc0)
  {
  case 0x00:    return("intermediate");
  case 0x40:    return("off");
  case 0x80:    return("on");
  case 0xc0:    return("indeterminate");
  }
  return(" ");
}

/* function: _toPermissionsString
 * purpose: return string representation of file permissions.
 * arguments: 
 */
static void TMWDEFS_LOCAL _toPermissionsString(
  TMWTYPES_CHAR *pStr,
  TMWTYPES_USHORT size,
  TMWTYPES_USHORT permissions)
{
  pStr[0] = '\0';
  if((permissions & DNPDEFS_WORLD_EXECUTE_ALLOWED) == DNPDEFS_WORLD_EXECUTE_ALLOWED)
    STRCPY(pStr, size, " WorldExecute");
  if((permissions & DNPDEFS_WORLD_WRITE_ALLOWED) == DNPDEFS_WORLD_WRITE_ALLOWED)
    STRCPY(pStr, size, " WorldWrite");
  if((permissions & DNPDEFS_WORLD_READ_ALLOWED) == DNPDEFS_WORLD_READ_ALLOWED)
    STRCPY(pStr, size, " WorldRead");
  if((permissions & DNPDEFS_GROUP_EXECUTE_ALLOWED) == DNPDEFS_GROUP_EXECUTE_ALLOWED)
    STRCPY(pStr, size, " GroupExecute");
  if((permissions & DNPDEFS_GROUP_WRITE_ALLOWED) == DNPDEFS_GROUP_WRITE_ALLOWED)
    STRCPY(pStr, size, " GroupWrite");
  if((permissions & DNPDEFS_GROUP_READ_ALLOWED) == DNPDEFS_GROUP_READ_ALLOWED)
    STRCPY(pStr, size, " GroupRead");
  if((permissions & DNPDEFS_OWNER_EXECUTE_ALLOWED) == DNPDEFS_OWNER_EXECUTE_ALLOWED)
    STRCPY(pStr, size, " OwnerExecute");
  if((permissions & DNPDEFS_OWNER_WRITE_ALLOWED) == DNPDEFS_OWNER_WRITE_ALLOWED)
    STRCPY(pStr, size, " OwnerWrite");
  if((permissions & DNPDEFS_OWNER_READ_ALLOWED) == DNPDEFS_OWNER_READ_ALLOWED)
    STRCPY(pStr, size, " OwnerRead");
}

/* function: _hexBytesOut
 * purpose: common function to print out 16 hex bytes per line.
 * arguments: 
 */
static void TMWDEFS_LOCAL _hexBytesOut(TMWDIAG_ANLZ_ID *pAnlzId, const TMWTYPES_UCHAR *pData, const TMWTYPES_ULONG length)
{
  int outLength;
  int index = 0;
  TMWTYPES_BOOL truncated = TMWDEFS_FALSE;
  char buf[128];

  if (length > DNPCNFG_MAX_DIAG_HEX_BYTES)
  {
    outLength = DNPCNFG_MAX_DIAG_HEX_BYTES;
    truncated = TMWDEFS_TRUE;
  }
  else
  {
    outLength = (TMWTYPES_USHORT)length;
  }

  while (index < outLength)
  {
    int i;
    int len;
    int rowLength;
    len = tmwtarg_snprintf(buf, sizeof(buf), "%16s", " ");

    rowLength = outLength - index;
    if (rowLength > MAX_ROW_LENGTH)
      rowLength = MAX_ROW_LENGTH;

    for (i = 0; i < rowLength; i++)
      len += tmwtarg_snprintf(buf + len, sizeof(buf) - len, "%02x ", pData[index++]);

    (void)tmwtarg_snprintf(buf + len, sizeof(buf) - len, "\n");
    tmwdiag_putLine(pAnlzId, buf);
  }

  if (truncated)
  {
    tmwtarg_snprintf(buf, sizeof(buf), "%-11s     Data length %u exceeds %d octets, display truncated \n", " ", length, DNPCNFG_MAX_DIAG_HEX_BYTES);
    tmwdiag_putLine(pAnlzId, buf);
  }
  
  tmwdiag_skipLine(pAnlzId);
}

/* function: _displayApplFragment
 * purpose: display application layer fragment
 * arguments:
 *  prompt - prompt for text
 *  channelName - name of channel
 *  pBuf - header data
 *  numBytes - number of bytes
 * returns:
 *  void
 */
static void TMWDEFS_LOCAL _displayApplFragment(
  TMWCHNL *pChannel,
  TMWSESN *pSession,
  const char *prompt,
  const TMWTYPES_UCHAR *pBuf,
  TMWTYPES_USHORT numBytes,
  TMWTYPES_ULONG direction)
{
  TMWDIAG_ANLZ_ID anlzId;
  TMWTYPES_UCHAR ac;
  TMWTYPES_UCHAR fc;
  char buf[256];

  if (tmwdiag_initId(&anlzId, pChannel, pSession, TMWDEFS_NULL, TMWDIAG_ID_APPL | direction) == TMWDEFS_FALSE)
  {
    return;
  }

  ac = pBuf[0];
  fc = pBuf[1];

  (void)tmwtarg_snprintf(buf, sizeof(buf), "%s %-10s Application Header, %s\n",
    prompt, tmwsesn_getSessionName(pSession), _fcString(fc));

  tmwdiag_skipLine(&anlzId);
  tmwdiag_putLine(&anlzId, buf);

  (void)tmwtarg_snprintf(buf, sizeof(buf), "%16sFIR(%s) FIN(%s) CON(%s) UNS(%s) SEQ# %d\n", " ",
    (ac & 0x80) ? "1" : "0", (ac & 0x40) ? "1" : "0", (ac & 0x20) ? "1" : "0",
    (ac & 0x10) ? "1" : "0", ac & 0x0f);

  tmwdiag_putLine(&anlzId, buf);

  _hexBytesOut(&anlzId, pBuf, numBytes);
}

/* function: _displayTprtFrame
 * purpose: display transport layer frame
 * arguments:
 *  prompt - prompt for text
 *  channelName - name of channel
 *  pBuf - header data
 *  numBytes - number of bytes
 * returns:
 *  void
 */
static void TMWDEFS_LOCAL _displayTprtFrame(
  TMWCHNL *pChannel,
  TMWSESN *pSession,
  const char *prompt,
  const TMWTYPES_UCHAR *pBuff,
  TMWTYPES_USHORT numBytes,
  TMWTYPES_ULONG direction)
{
  TMWDIAG_ANLZ_ID anlzId;
  unsigned char header;
  char buf[256];

  if (tmwdiag_initId(&anlzId, pChannel, pSession, TMWDEFS_NULL, TMWDIAG_ID_TPRT | direction) == TMWDEFS_FALSE)
  {
    return;
  }

  (void)tmwtarg_snprintf(buf, sizeof(buf), "%s %-10s Transport Header\n",
    prompt, tmwsesn_getSessionName(pSession));

  tmwdiag_skipLine(&anlzId);
  tmwdiag_putLine(&anlzId, buf);

  header = pBuff[0];
  (void)tmwtarg_snprintf(buf, sizeof(buf), "%16sFIR(%s) FIN(%s) SEQ# %d\n", " ",
    (header & DNPDEFS_TH_FIRST) ? "1" : "0",
    (header & DNPDEFS_TH_FINAL) ? "1" : "0",
    header & DNPDEFS_TH_SEQUENCE_MASK);

  tmwdiag_putLine(&anlzId, buf);

  _hexBytesOut(&anlzId, pBuff, numBytes);
}

/* function: _displayLinkFrame
 * purpose: display link layer frame
 * arguments:
 *  prompt - prompt for text
 *  channelName - name of channel
 *  pBuf - header data
 *  numBytes - number of bytes
 * returns:
 *  void
 */
static void TMWDEFS_LOCAL _displayLinkFrame(
  TMWCHNL *pChannel,
  TMWSESN *pSession,
  const char *prompt,
  const TMWTYPES_UCHAR *pBuf,
  TMWTYPES_USHORT numBytes,
  TMWTYPES_USHORT retryCount,
  TMWTYPES_BOOL discarded,
  TMWTYPES_ULONG direction)
{
  TMWTYPES_USHORT destAddr;
  TMWTYPES_USHORT srcAddr;
  TMWTYPES_CHAR retryBuf[16];
  TMWTYPES_CHAR discardBuf[16];
  TMWTYPES_CHAR buf[256];
  TMWDIAG_ANLZ_ID anlzId;
  int len;
  int i;

  if (tmwdiag_initId(&anlzId, pChannel, pSession, TMWDEFS_NULL, TMWDIAG_ID_LINK | direction) == TMWDEFS_FALSE)
  {
    return;
  }

  /* Display header information */

  retryBuf[0] = '\0';
  if(retryCount > 0)
    (void)tmwtarg_snprintf(retryBuf, sizeof(retryBuf), "Retry %d", retryCount);

  discardBuf[0] = '\0';
  if(discarded)
    (void)tmwtarg_snprintf(discardBuf, sizeof(discardBuf), "Discarded");

  (void)tmwtarg_snprintf(buf, sizeof(buf), "%s %-10s %s %s %s\n",
    prompt, tmwchnl_getChannelName(pChannel),
    _getLinkControlString(pBuf[DNPDEFS_LH_INDEX_CONTROL]),
    retryBuf, discardBuf);

  tmwdiag_skipLine(&anlzId);
  tmwdiag_putLine(&anlzId, buf);

  /* Display header information */

  tmwtarg_get16(&pBuf[DNPDEFS_LH_INDEX_DEST_ADDR], &destAddr);
  tmwtarg_get16(&pBuf[DNPDEFS_LH_INDEX_SOURCE_ADDR], &srcAddr);

  if((pBuf[DNPDEFS_LH_INDEX_CONTROL] & DNPDEFS_LC_PRM_MASK) == DNPDEFS_LC_PRM_MASK)
  {
    (void)tmwtarg_snprintf(buf, sizeof(buf), "%16sLEN(%d) DIR(%s) PRM(1) FCV(%s) FCB(%s) DEST(%d) SRC(%d)\n",
      " ", pBuf[DNPDEFS_LH_INDEX_LENGTH],
      ((pBuf[DNPDEFS_LH_INDEX_CONTROL] & DNPDEFS_LC_DIR_MASK) == 0) ? "0" : "1",
      ((pBuf[DNPDEFS_LH_INDEX_CONTROL] & DNPDEFS_LC_FCV_MASK) == 0) ? "0" : "1",
      ((pBuf[DNPDEFS_LH_INDEX_CONTROL] & DNPDEFS_LC_FCB_MASK) == 0) ? "0" : "1",
      destAddr, srcAddr);
  }
  else
  {
    (void)tmwtarg_snprintf(buf, sizeof(buf), "%16sLEN(%d) DIR(%s) PRM(0) DFC(%s) DEST(%d) SRC(%d)\n",
      " ", pBuf[DNPDEFS_LH_INDEX_LENGTH],
      ((pBuf[DNPDEFS_LH_INDEX_CONTROL] & DNPDEFS_LC_DIR_MASK) == 0) ? "0" : "1",
      ((pBuf[DNPDEFS_LH_INDEX_CONTROL] & DNPDEFS_LC_DFC_MASK) == 0) ? "0" : "1",
      destAddr, srcAddr);
  }

  tmwdiag_putLine(&anlzId, buf);

  /* Now display the actual bytes */
  len = tmwtarg_snprintf(buf, sizeof(buf), "%16s", " ");
  for(i = 0; i < DNPLINK_HEADER_LENGTH; i++)
    len += tmwtarg_snprintf(buf + len, sizeof(buf) - len, "%02x ", pBuf[i]);

  (void)tmwtarg_snprintf(buf + len, sizeof(buf) - len, "\n");
  tmwdiag_putLine(&anlzId, buf);
   
  _hexBytesOut(&anlzId, pBuf + DNPLINK_HEADER_LENGTH, numBytes - DNPLINK_HEADER_LENGTH);
}

/* Global Functions */

/* routine: dnpdiag_validateErrorTable */
TMWTYPES_BOOL dnpdiag_validateErrorTable(void)
{
  int i;
  for(i=0; i<DNPDIAG_ERROR_ENUM_MAX;i++)
  {
    if(dnpDiagErrorMsg[i].errorNumber != i)
      return(TMWDEFS_FALSE);
  }

  return(TMWDEFS_TRUE);
}

/* routine: dnpdiag_init */
void TMWDEFS_GLOBAL dnpdiag_init()
{
  /* No error messages are disabled by default. */
  memset(_errorMsgDisabled, 0, (DNPDIAG_ERROR_ENUM_MAX/8)+1);
}


/* function: dnpdiag_buildMessage */
void TMWDEFS_GLOBAL dnpdiag_buildMessage(
  TMWCHNL *pChannel,
  TMWSESN *pSession,
  const char *pDescription)
{
  TMWDIAG_ANLZ_ID id;
  char buf[256];
  
#ifdef TMW_SUPPORT_MONITOR
  /* If in analyzer or listen only mode, do not display this */
  if(pChannel->pPhysContext->monitorMode)
    return;
#endif

  if (tmwdiag_initId(&id, pChannel, pSession, TMWDEFS_NULL, TMWDIAG_ID_USER) == TMWDEFS_FALSE)
  {
    return;
  }

  if(pDescription != TMWDEFS_NULL)
  {
    (void)tmwtarg_snprintf(buf, sizeof(buf), "<+++ %-10s Build DNP3 Message: %s\n",
      tmwsesn_getSessionName(pSession), pDescription); 
  }
  else
  {
    (void)tmwtarg_snprintf(buf, sizeof(buf), "<+++ %-10s Build DNP3 Message:\n",
      tmwsesn_getSessionName(pSession)); 
  }
  tmwdiag_skipLine(&id);
  tmwdiag_putLine(&id, buf);
}

/* function: dnpdiag_insertQueue */
void TMWDEFS_GLOBAL dnpdiag_insertQueue(
  TMWCHNL *pChannel,
  TMWSESN *pSession,
  const char *description)
{
  TMWDIAG_ANLZ_ID id;
  char buf[256];

  if (tmwdiag_initId(&id, pChannel, pSession, TMWDEFS_NULL, TMWDIAG_ID_USER) == TMWDEFS_FALSE)
  {
    return;
  }

  (void)tmwtarg_snprintf(buf, sizeof(buf), "<+++ %-10s Insert request in queue: %s\n",
    tmwsesn_getSessionName(pSession), description);

  tmwdiag_skipLine(&id);
  tmwdiag_putLine(&id, buf);
}

/* function: dnpdiag_showObjectHeader */
void TMWDEFS_GLOBAL dnpdiag_showObjectHeader(
  TMWSESN *pSession,
  DNPUTIL_OBJECT_HEADER *pHdr)
{
  TMWDIAG_ANLZ_ID anlzId;
  char buf[128];

  if(pHdr->group == DNPDEFS_OBJ_120_AUTHENTICATION && pHdr->variation == 7)
  {
    if (tmwdiag_initId(&anlzId, TMWDEFS_NULL, pSession, TMWDEFS_NULL, TMWDIAG_ID_EVENT_HDRS | TMWDIAG_ID_RX) == TMWDEFS_FALSE)
    {
      return;
    }
  }
  else
  {
    if(tmwdiag_initId(&anlzId, TMWDEFS_NULL, pSession, TMWDEFS_NULL, _objectToHdrId(pHdr->group) | TMWDIAG_ID_RX) == TMWDEFS_FALSE)
    {
      return;
    }
  }

  (void)tmwtarg_snprintf(buf, sizeof(buf), "%16sRx Object %d(%s), variation %d, qualifier 0x%02x(%s) \n", " ",
    pHdr->group, _groupString(pHdr->group), pHdr->variation, pHdr->qualifier, _qualifierString(pHdr->qualifier));

  tmwdiag_skipLine(&anlzId);
  tmwdiag_putLine(&anlzId, buf);
}

void TMWDEFS_GLOBAL dnpdiag_showTxObjectHdr(
  TMWSESN *pSession, 
  TMWTYPES_UCHAR group,
  TMWTYPES_UCHAR variation,
  TMWTYPES_UCHAR qualifier)
{
  TMWDIAG_ANLZ_ID anlzId;
  char buf[128];

  if(tmwdiag_initId(&anlzId, TMWDEFS_NULL, pSession, TMWDEFS_NULL, _objectToHdrId(group)) == TMWDEFS_FALSE)
  {
    return;
  }

  (void)tmwtarg_snprintf(buf, sizeof(buf), "%16sTx Object %d(%s), variation %d, qualifier 0x%02x(%s) \n", " ",
    group, _groupString(group), variation, qualifier, _qualifierString(qualifier));

  tmwdiag_skipLine(&anlzId);
  tmwdiag_putLine(&anlzId, buf);
}

/* function: dnpdiag_showIINValue
 * purpose: display IIN bits from read object 80
 * arguments:
 *  pSession - session from which this message originated
 *  pointNumber - point number
 *  value - value of IIN bit
 * returns:
 *  void
 */
void TMWDEFS_GLOBAL dnpdiag_showIINValue(
  TMWSESN *pSession,
  TMWTYPES_USHORT pointNumber, 
  TMWTYPES_BOOL value)
{ 
  TMWDIAG_ANLZ_ID anlzId;
  char buf[128];

  if(tmwdiag_initId(&anlzId, TMWDEFS_NULL, pSession, TMWDEFS_NULL, TMWDIAG_ID_STATIC_DATA | TMWDIAG_ID_RX) == TMWDEFS_FALSE)
  {
    return;
  }
  (void)tmwtarg_snprintf(buf, sizeof(buf), "%-21s IIN Bit %06d = 0x%02x\n"," ", pointNumber, value);
  tmwdiag_putLine(&anlzId, buf);
}

/* function: dnpdiag_showBinaryInput */
void TMWDEFS_GLOBAL dnpdiag_showBinaryInput(
  TMWSESN *pSession,
  TMWTYPES_USHORT point,
  TMWTYPES_UCHAR flags,
  TMWTYPES_BOOL isEvent,
  TMWDTIME *pTimeStamp)
{
  TMWDIAG_ANLZ_ID anlzId;
  char timeBuf[64];
  char buf[128];
  TMWDIAG_ID direction = 0;

  if (pSession->type == TMWTYPES_SESSION_TYPE_MASTER)
    direction = TMWDIAG_ID_RX;

  if(tmwdiag_initId(&anlzId, TMWDEFS_NULL, pSession, TMWDEFS_NULL,
    (TMWDIAG_ID)((((isEvent) ? TMWDIAG_ID_EVENT_DATA : TMWDIAG_ID_STATIC_DATA) | direction))) == TMWDEFS_FALSE)
  {
    return;
  }
  if(pTimeStamp != TMWDEFS_NULL)
    tmwdiag_time2string(pTimeStamp, TMWDEFS_TIME_FORMAT_56, timeBuf, sizeof(timeBuf), TMWDEFS_FALSE);
  else
    (void)tmwtarg_snprintf(timeBuf, sizeof(timeBuf), " ");

  (void)tmwtarg_snprintf(buf, sizeof(buf), "%-21s Binary Input %06d = 0x%02x\n", timeBuf, point, flags);
  tmwdiag_putLine(&anlzId, buf);
}

/* function: dnpdiag_showDoubleInput */
void TMWDEFS_GLOBAL dnpdiag_showDoubleInput(
  TMWSESN *pSession,
  TMWTYPES_USHORT point,
  TMWTYPES_UCHAR flags,
  TMWTYPES_BOOL isEvent,
  TMWDTIME *pTimeStamp)
{
  TMWDIAG_ANLZ_ID anlzId;
  char timeBuf[64];
  char buf[128];
  TMWDIAG_ID direction = 0;

  if (pSession->type == TMWTYPES_SESSION_TYPE_MASTER)
    direction = TMWDIAG_ID_RX;

  if(tmwdiag_initId(&anlzId, TMWDEFS_NULL, pSession, TMWDEFS_NULL,
    (TMWDIAG_ID)((((isEvent) ? TMWDIAG_ID_EVENT_DATA : TMWDIAG_ID_STATIC_DATA) | direction))) == TMWDEFS_FALSE)
  {
    return;
  }

  if(pTimeStamp != TMWDEFS_NULL)
    tmwdiag_time2string(pTimeStamp, TMWDEFS_TIME_FORMAT_56, timeBuf, sizeof(timeBuf), TMWDEFS_FALSE);
  else
    (void)tmwtarg_snprintf(timeBuf, sizeof(timeBuf), " ");
  
  (void)tmwtarg_snprintf(buf, sizeof(buf), "%-21s Double Bit Input %06d = 0x%02x %s\n", 
    timeBuf, point, flags, _doubleBitStateString(flags));

  tmwdiag_putLine(&anlzId, buf);
}

/* function: dnpdiag_showCROB */
void TMWDEFS_GLOBAL dnpdiag_showCROB(
  TMWSESN *pSession,
  TMWTYPES_USHORT point,
  TMWTYPES_UCHAR control,
  TMWTYPES_UCHAR count,
  TMWTYPES_ULONG onTime,
  TMWTYPES_ULONG offTime,
  TMWTYPES_UCHAR status,
  TMWDIAG_ID direction)
{
  TMWDIAG_ANLZ_ID anlzId;
  char statusBuf[64];
  char buf[128];

  if(tmwdiag_initId(&anlzId, TMWDEFS_NULL, pSession, TMWDEFS_NULL, TMWDIAG_ID_STATIC_DATA | direction) == TMWDEFS_FALSE)
  {
    return;
  }

  /* Don't show status when control is being transmitted. It is required to be zero */
  if (pSession == TMWDEFS_NULL || (pSession->type == TMWTYPES_SESSION_TYPE_MASTER && direction != TMWDIAG_ID_RX))
  {
    statusBuf[0] = '\0';
  }
  else
  {
    (void)tmwtarg_snprintf(statusBuf, sizeof(buf), ", status = %d(%s)", status, _controlStatusString(status));
  }

  (void)tmwtarg_snprintf(buf, sizeof(buf), "%-21s Control Relay Output Block %06d = 0x%02x(%s:%s:%s:%s) \n",
    " ", point, control, _controlOpTypeString(control), _controlQueueString(control), _controlClearString(control), _controlTCCString(control));
  tmwdiag_putLine(&anlzId, buf);

  (void)tmwtarg_snprintf(buf, sizeof(buf), "%-21s    count = %d, on = %lu, off = %lu%s\n",
    " ", count, onTime, offTime, statusBuf);
  tmwdiag_putLine(&anlzId, buf);
}

/* function: dnpdiag_showBinaryOutput */
void TMWDEFS_GLOBAL dnpdiag_showBinaryOutput(
  TMWSESN *pSession,
  TMWTYPES_USHORT point,
  TMWTYPES_UCHAR flags,  
  TMWTYPES_BOOL isEvent,
  TMWDTIME *pTimeStamp)
{
  TMWDIAG_ANLZ_ID anlzId;
  char timeBuf[64];
  char buf[128];
  TMWDIAG_ID direction = 0;

  if (pSession->type == TMWTYPES_SESSION_TYPE_MASTER)
    direction = TMWDIAG_ID_RX;

  if(tmwdiag_initId(&anlzId, TMWDEFS_NULL, pSession, TMWDEFS_NULL,
    (TMWDIAG_ID)((((isEvent) ? TMWDIAG_ID_EVENT_DATA : TMWDIAG_ID_STATIC_DATA) | direction))) == TMWDEFS_FALSE)
  {
    return;
  }

  if(pTimeStamp != TMWDEFS_NULL)
    tmwdiag_time2string(pTimeStamp, TMWDEFS_TIME_FORMAT_56, timeBuf, sizeof(timeBuf), TMWDEFS_FALSE);
  else
    (void)tmwtarg_snprintf(timeBuf, sizeof(timeBuf), " ");

  (void)tmwtarg_snprintf(buf, sizeof(buf), "%-21s Binary Output %06d = 0x%02x\n", timeBuf, point, flags);
  tmwdiag_putLine(&anlzId, buf);
}

/* function: dnpdiag_showBinOutWrite */
void TMWDEFS_GLOBAL dnpdiag_showBinOutWrite(
  TMWSESN *pSession,
  TMWTYPES_USHORT point,
  TMWTYPES_UCHAR state)
{
  TMWDIAG_ANLZ_ID anlzId;
  char buf[128];

  if(tmwdiag_initId(&anlzId, TMWDEFS_NULL, pSession, TMWDEFS_NULL, TMWDIAG_ID_STATIC_DATA | TMWDIAG_ID_RX) == TMWDEFS_FALSE)
  {
    return;
  }

  (void)tmwtarg_snprintf(buf, sizeof(buf), "%-21s Binary Output %06d = 0x%02x\n", " ", point, state);
  tmwdiag_putLine(&anlzId, buf);
}

void TMWDEFS_GLOBAL dnpdiag_showBinaryCmdStatus(
  TMWSESN *pSession,
  TMWTYPES_USHORT point, 
  TMWTYPES_UCHAR status,  
  TMWDTIME *pTimeStamp)
{
  TMWDIAG_ANLZ_ID anlzId;
  char timeBuf[64];
  char buf[128];
  TMWDIAG_ID direction = 0;

  if (pSession->type == TMWTYPES_SESSION_TYPE_MASTER)
    direction = TMWDIAG_ID_RX;

  if(tmwdiag_initId(&anlzId, TMWDEFS_NULL, pSession, TMWDEFS_NULL, TMWDIAG_ID_EVENT_DATA | direction) == TMWDEFS_FALSE)
  {
    return;
  }

  if(pTimeStamp != TMWDEFS_NULL)
    tmwdiag_time2string(pTimeStamp, TMWDEFS_TIME_FORMAT_56, timeBuf, sizeof(timeBuf), TMWDEFS_FALSE);
  else
    (void)tmwtarg_snprintf(timeBuf, sizeof(timeBuf), " ");

  (void)tmwtarg_snprintf(buf, sizeof(buf), "%-21s Binary Output Command Status %06d, CS = %d status = %d(%s)\n", 
    timeBuf, point, ((status & 0x80)?1:0), (status & 0x7f), _controlStatusString((TMWTYPES_UCHAR)(status & 0x7f)));

  tmwdiag_putLine(&anlzId, buf);
}

/* function: dnpdiag_showBinaryCounter */
void TMWDEFS_GLOBAL dnpdiag_showBinaryCounter(
  TMWSESN *pSession,
  TMWTYPES_USHORT point,
  TMWTYPES_ULONG value,
  TMWTYPES_UCHAR flags,
  TMWTYPES_BOOL isEvent,
  TMWDTIME *pTimeStamp)
{
  TMWDIAG_ANLZ_ID anlzId;
  char timeBuf[64];
  char buf[128];
  TMWDIAG_ID direction = 0;

  if (pSession->type == TMWTYPES_SESSION_TYPE_MASTER)
    direction = TMWDIAG_ID_RX;

  if(tmwdiag_initId(&anlzId, TMWDEFS_NULL, pSession, TMWDEFS_NULL,
    (TMWDIAG_ID)((((isEvent) ? TMWDIAG_ID_EVENT_DATA : TMWDIAG_ID_STATIC_DATA) | direction))) == TMWDEFS_FALSE)
  {
    return;
  }

  if(pTimeStamp != TMWDEFS_NULL)
    tmwdiag_time2string(pTimeStamp, TMWDEFS_TIME_FORMAT_56, timeBuf, sizeof(timeBuf), TMWDEFS_FALSE);
  else
    (void)tmwtarg_snprintf(timeBuf, sizeof(timeBuf), " ");

  (void)tmwtarg_snprintf(buf, sizeof(buf), "%-21s Binary Counter %06d = %lu, flags = 0x%02x\n", timeBuf, point, value, flags);
  tmwdiag_putLine(&anlzId, buf);
}

/* function: dnpdiag_showFrozenCounter */
void TMWDEFS_GLOBAL dnpdiag_showFrozenCounter(
  TMWSESN *pSession,
  TMWTYPES_USHORT point,
  TMWTYPES_ULONG value,
  TMWTYPES_UCHAR flags,
  TMWTYPES_BOOL isEvent,
  TMWDTIME *pTimeStamp)
{
  TMWDIAG_ANLZ_ID anlzId;
  char timeBuf[64];
  char buf[128];
  TMWDIAG_ID direction = 0;

  if (pSession->type == TMWTYPES_SESSION_TYPE_MASTER)
    direction = TMWDIAG_ID_RX;

  if(tmwdiag_initId(&anlzId, TMWDEFS_NULL, pSession, TMWDEFS_NULL,
    (TMWDIAG_ID)((((isEvent) ? TMWDIAG_ID_EVENT_DATA : TMWDIAG_ID_STATIC_DATA) | direction))) == TMWDEFS_FALSE)
  {
    return;
  }

  if(pTimeStamp != TMWDEFS_NULL)
    tmwdiag_time2string(pTimeStamp, TMWDEFS_TIME_FORMAT_56, timeBuf, sizeof(timeBuf), TMWDEFS_FALSE);
  else
    (void)tmwtarg_snprintf(timeBuf, sizeof(timeBuf), " ");

  (void)tmwtarg_snprintf(buf, sizeof(buf), "%-21s Frozen Counter %06d = %lu, flags = 0x%02x\n", timeBuf, point, value, flags);
  tmwdiag_putLine(&anlzId, buf);
}

/* function: dnpdiag_showAnalogInput */
void TMWDEFS_GLOBAL dnpdiag_showAnalogInput(
  TMWSESN *pSession,
  TMWTYPES_USHORT point,
  TMWTYPES_ANALOG_VALUE *pValue,
  TMWTYPES_UCHAR flags,
  TMWTYPES_BOOL isEvent,
  TMWDTIME *pTimeStamp)
{
  /* Temporary storage for flags set by getAnalogValue functions
   * we need to display the flag values passed in. We don't know
   * what variation is being used, to know what over flow would be
   */
  TMWTYPES_UCHAR tempFlags=0;
  TMWDIAG_ANLZ_ID anlzId;
  char timeBuf[64];
  char buf[128];
#if TMWCNFG_SUPPORT_DOUBLE
  TMWTYPES_DOUBLE value;
#elif TMWCNFG_SUPPORT_FLOAT
  TMWTYPES_SFLOAT value;
#endif
  TMWDIAG_ID direction = 0;

  if (pSession->type == TMWTYPES_SESSION_TYPE_MASTER)
    direction = TMWDIAG_ID_RX;

  if(tmwdiag_initId(&anlzId, TMWDEFS_NULL, pSession, TMWDEFS_NULL,
    (TMWDIAG_ID)((((isEvent) ? TMWDIAG_ID_EVENT_DATA : TMWDIAG_ID_STATIC_DATA) | direction))) == TMWDEFS_FALSE)
  {
    return;
  }

  if(pTimeStamp != TMWDEFS_NULL)
    tmwdiag_time2string(pTimeStamp, TMWDEFS_TIME_FORMAT_56, timeBuf, sizeof(timeBuf), TMWDEFS_FALSE);
  else
    (void)tmwtarg_snprintf(timeBuf, sizeof(timeBuf), " ");


#if TMWCNFG_SUPPORT_DOUBLE
  if((pValue->type == TMWTYPES_ANALOG_TYPE_LONG)
    ||(pValue->type == TMWTYPES_ANALOG_TYPE_SHORT))
  {
    (void)tmwtarg_snprintf(buf, sizeof(buf), "%-21s Analog Input %06d = %d, flags 0x%02x\n",
      timeBuf, point, dnputil_getAnalogValueLong(pValue, &tempFlags), flags);
  }
  else if(pValue->type == TMWTYPES_ANALOG_TYPE_SFLOAT)
  {
    TMWTYPES_SFLOAT value1 = dnputil_getAnalogValueFloat(pValue, &tempFlags);
    value = (TMWTYPES_DOUBLE)value1;
    (void)tmwtarg_snprintf(buf, sizeof(buf), "%-21s Analog Input %06d = %.10g, flags 0x%02x\n", timeBuf, point, value1, flags);
  }
  else
  {
    value = dnputil_getAnalogValueDouble(pValue);
    (void)tmwtarg_snprintf(buf, sizeof(buf), "%-21s Analog Input %06d = %.10g, flags 0x%02x\n", timeBuf, point, value, flags);
  }

#elif TMWCNFG_SUPPORT_FLOAT
  if((pValue->type == TMWTYPES_ANALOG_TYPE_LONG)
    ||(pValue->type == TMWTYPES_ANALOG_TYPE_SHORT))
  {
    (void)tmwtarg_snprintf(buf, sizeof(buf), "%-21s Analog Input %06d = %d, flags 0x%02x\n",
      timeBuf, point, dnputil_getAnalogValueLong(pValue, &tempFlags), flags);
  }
  else
  {
    value = dnputil_getAnalogValueFloat(pValue, &tempFlags);
    (void)tmwtarg_snprintf(buf, sizeof(buf), "%-21s Analog Input %06d = %.10g, flags 0x%02x\n", timeBuf, point, value, flags);
  }

#else
  tmwtarg_snprintf(buf, sizeof(buf), "%-21s Analog Input %06d = %d, flags 0x%02x\n",
    timeBuf, point, dnputil_getAnalogValueLong(pValue, &tempFlags), flags);
#endif

  tmwdiag_putLine(&anlzId, buf);
}

/* function: dnpdiag_showFrozenAnalog */
void TMWDEFS_GLOBAL dnpdiag_showFrozenAnalogInput(
  TMWSESN *pSession,
  TMWTYPES_USHORT point,
  TMWTYPES_ANALOG_VALUE *pValue,
  TMWTYPES_UCHAR flags,
  TMWTYPES_BOOL isEvent,
  TMWDTIME *pTimeStamp)
{
  TMWTYPES_UCHAR tempFlags=0;
  TMWDIAG_ANLZ_ID anlzId;
  char timeBuf[64];
  char buf[128];
  #if TMWCNFG_SUPPORT_DOUBLE
  TMWTYPES_DOUBLE value;
#elif TMWCNFG_SUPPORT_FLOAT
  TMWTYPES_SFLOAT value;
#endif
  TMWDIAG_ID direction = 0;

  if (pSession->type == TMWTYPES_SESSION_TYPE_MASTER)
    direction = TMWDIAG_ID_RX;

  if(tmwdiag_initId(&anlzId, TMWDEFS_NULL, pSession, TMWDEFS_NULL,
    (TMWDIAG_ID)((((isEvent) ? TMWDIAG_ID_EVENT_DATA : TMWDIAG_ID_STATIC_DATA) | direction))) == TMWDEFS_FALSE)
  {
    return;
  }

  if(pTimeStamp != TMWDEFS_NULL)
    tmwdiag_time2string(pTimeStamp, TMWDEFS_TIME_FORMAT_56, timeBuf, sizeof(timeBuf), TMWDEFS_FALSE);
  else
    (void)tmwtarg_snprintf(timeBuf, sizeof(timeBuf), " ");

#if TMWCNFG_SUPPORT_DOUBLE
  if((pValue->type == TMWTYPES_ANALOG_TYPE_LONG)
    ||(pValue->type == TMWTYPES_ANALOG_TYPE_SHORT))
  {
    (void)tmwtarg_snprintf(buf, sizeof(buf), "%-21s Frozen Analog Input %06d = %d, flags 0x%02x\n",
      timeBuf, point, dnputil_getAnalogValueLong(pValue, &tempFlags), flags);
  }
  else if(pValue->type == TMWTYPES_ANALOG_TYPE_SFLOAT)
  {
    TMWTYPES_SFLOAT value1 = dnputil_getAnalogValueFloat(pValue, &tempFlags);
    value = (TMWTYPES_DOUBLE)value1;
    (void)tmwtarg_snprintf(buf, sizeof(buf), "%-21s Frozen Analog Input %06d = %.10g, flags 0x%02x\n", timeBuf, point, value1, flags);
  }
  else
  {
    value = dnputil_getAnalogValueDouble(pValue);
    (void)tmwtarg_snprintf(buf, sizeof(buf), "%-21s Frozen Analog Input %06d = %.10g, flags 0x%02x\n", timeBuf, point, value, flags);
  }

#elif TMWCNFG_SUPPORT_FLOAT
  if((pValue->type == TMWTYPES_ANALOG_TYPE_LONG)
    ||(pValue->type == TMWTYPES_ANALOG_TYPE_SHORT))
  {
    (void)tmwtarg_snprintf(buf, sizeof(buf), "%-21s Frozen Analog Input %06d = %d, flags 0x%02x\n",
      timeBuf, point, dnputil_getAnalogValueLong(pValue, &tempFlags), flags);
  }
  else
  {
    value = dnputil_getAnalogValueFloat(pValue, &tempFlags);
    (void)tmwtarg_snprintf(buf, sizeof(buf), "%-21s Frozen Analog Input %06d = %.10g, flags 0x%02x\n", timeBuf, point, value, flags);
  }

#else
  tmwtarg_snprintf(buf, sizeof(buf), "%-21s Frozen Analog Input %06d = %d, flags 0x%02x\n",
    timeBuf, point, dnputil_getAnalogValueLong(pValue, &tempFlags), flags);
#endif

  tmwdiag_putLine(&anlzId, buf);
}

/* function: dnpdiag_showAnalogOutput */
void TMWDEFS_GLOBAL dnpdiag_showAnalogOutput(
  TMWSESN *pSession,
  TMWTYPES_USHORT point,
  TMWTYPES_ANALOG_VALUE *pValue,
  TMWTYPES_UCHAR flags,
  TMWTYPES_BOOL isEvent,
  TMWDTIME *pTimeStamp)
{
  /* Temporary storage for flags set by getAnalogValue functions
   * we need to display the flag values passed in. We don't know
   * what variation is being used, to know what over flow would be
   */
  TMWTYPES_UCHAR tempFlags=0;
  TMWDIAG_ANLZ_ID anlzId;
  char timeBuf[64];
  char buf[128];
#if TMWCNFG_SUPPORT_DOUBLE
  TMWTYPES_DOUBLE value;
#elif TMWCNFG_SUPPORT_FLOAT
  TMWTYPES_SFLOAT value;
#endif
  TMWDIAG_ID direction = 0;

  if (pSession->type == TMWTYPES_SESSION_TYPE_MASTER)
    direction = TMWDIAG_ID_RX;

  if(tmwdiag_initId(&anlzId, TMWDEFS_NULL, pSession, TMWDEFS_NULL,
    (TMWDIAG_ID)((((isEvent) ? TMWDIAG_ID_EVENT_DATA : TMWDIAG_ID_STATIC_DATA) | direction))) == TMWDEFS_FALSE)
  {
    return;
  }

  if(pTimeStamp != TMWDEFS_NULL)
    tmwdiag_time2string(pTimeStamp, TMWDEFS_TIME_FORMAT_56, timeBuf, sizeof(timeBuf), TMWDEFS_FALSE);
  else
    (void)tmwtarg_snprintf(timeBuf, sizeof(timeBuf), " ");

#if TMWCNFG_SUPPORT_DOUBLE
  if((pValue->type == TMWTYPES_ANALOG_TYPE_LONG)
    ||(pValue->type == TMWTYPES_ANALOG_TYPE_SHORT))
  {
    (void)tmwtarg_snprintf(buf, sizeof(buf), "%-21s Analog Output %06d = %d, flags 0x%02x\n",
      timeBuf, point, dnputil_getAnalogValueLong(pValue, &tempFlags), flags);
  }
  else
  {
    value = dnputil_getAnalogValueDouble(pValue);
    (void)tmwtarg_snprintf(buf, sizeof(buf), "%-21s Analog Output %06d = %.10g, flags 0x%02x\n", timeBuf, point, value, flags);
  }

#elif TMWCNFG_SUPPORT_FLOAT
  if((pValue->type == TMWTYPES_ANALOG_TYPE_LONG)
    ||(pValue->type == TMWTYPES_ANALOG_TYPE_SHORT))
  {
    (void)tmwtarg_snprintf(buf, sizeof(buf), "%-21s Analog Output %06d = %d, flags 0x%02x\n",
      timeBuf, point, dnputil_getAnalogValueLong(pValue, &tempFlags), flags);
  }
  else
  {
    value = dnputil_getAnalogValueFloat(pValue, &tempFlags);
    (void)tmwtarg_snprintf(buf, sizeof(buf), "%-21s Analog Output %06d = %.10g, flags 0x%02x\n", timeBuf, point, value, flags);
  }

#else
  (void)tmwtarg_snprintf(buf, sizeof(buf), "%-21s Analog Output %06d = %d, flags 0x%02x\n",
    timeBuf, point, dnputil_getAnalogValueLong(pValue, &tempFlags), flags);
#endif

  tmwdiag_putLine(&anlzId, buf);
}

/* function: dnpdiag_showAnalogControl */
void TMWDEFS_GLOBAL dnpdiag_showAnalogControl(
  TMWSESN *pSession,
  TMWTYPES_USHORT point,
  TMWTYPES_ANALOG_VALUE *pValue,
  TMWTYPES_UCHAR status,
  TMWTYPES_BOOL isEvent,
  TMWDIAG_ID direction,
  TMWDTIME *pTimeStamp)
{
  /* Temporary storage for flags set by getAnalogValue functions
   * we need to display the flag values passed in. We don't know
   * what variation is being used, to know what over flow would be
   */
  TMWTYPES_UCHAR tempFlags = 0;
  TMWDIAG_ANLZ_ID anlzId;
  char timeBuf[64];
  char statusBuf[64];
  char buf[128];
#if TMWCNFG_SUPPORT_DOUBLE
  TMWTYPES_DOUBLE value;
#elif TMWCNFG_SUPPORT_FLOAT
  TMWTYPES_SFLOAT value;
#endif

  if (tmwdiag_initId(&anlzId, TMWDEFS_NULL, pSession, TMWDEFS_NULL,
    (TMWDIAG_ID)((((isEvent) ? TMWDIAG_ID_EVENT_DATA : TMWDIAG_ID_STATIC_DATA) | direction))) == TMWDEFS_FALSE)
  {
    return;
  }

  if (pTimeStamp != TMWDEFS_NULL)
    tmwdiag_time2string(pTimeStamp, TMWDEFS_TIME_FORMAT_56, timeBuf, sizeof(timeBuf), TMWDEFS_FALSE);
  else
    (void)tmwtarg_snprintf(timeBuf, sizeof(timeBuf), " ");

  /* Don't show status when control is being transmitted. It is required to be zero */
  if (pSession == TMWDEFS_NULL || (pSession->type == TMWTYPES_SESSION_TYPE_MASTER && direction != TMWDIAG_ID_RX))
  {
    statusBuf[0] = '\0';
  }
  else
  {
    (void)tmwtarg_snprintf(statusBuf, sizeof(buf), ", status = %d(%s)", status, _controlStatusString(status));
  }

#if TMWCNFG_SUPPORT_DOUBLE
  if ((pValue->type == TMWTYPES_ANALOG_TYPE_LONG)
    || (pValue->type == TMWTYPES_ANALOG_TYPE_SHORT))
  {
    (void)tmwtarg_snprintf(buf, sizeof(buf), "%-21s Analog Output %06d = %d%s\n",
      timeBuf, point, dnputil_getAnalogValueLong(pValue, &tempFlags), statusBuf);
  }
  else
  {
    value = dnputil_getAnalogValueDouble(pValue);
      (void)tmwtarg_snprintf(buf, sizeof(buf), "%-21s Analog Output %06d = %.10g%s\n", timeBuf, point, value, statusBuf);
  }

#elif TMWCNFG_SUPPORT_FLOAT
  if ((pValue->type == TMWTYPES_ANALOG_TYPE_LONG)
    || (pValue->type == TMWTYPES_ANALOG_TYPE_SHORT))
  {
    (void)tmwtarg_snprintf(buf, sizeof(buf), "%-21s Analog Output %06d = %d%s\n",
      timeBuf, point, dnputil_getAnalogValueLong(pValue, &tempFlags), statusBuf);
  }
  else
  {
    value = dnputil_getAnalogValueFloat(pValue, &tempFlags);
    (void)tmwtarg_snprintf(buf, sizeof(buf), "%-21s Analog Output %06d = %.10g%s\n", timeBuf, point, value, statusBuf);
  }

#else
  (void)tmwtarg_snprintf(buf, sizeof(buf), "%-21s Analog Output %06d = %d%s\n",
    timeBuf, point, dnputil_getAnalogValueLong(pValue, &tempFlags), statusBuf);
#endif

  tmwdiag_putLine(&anlzId, buf);
}

/* function: dnpdiag_showAnalogDeadband
 * purpose: display analog deadband data
 * arguments:
 *  point - point number
 *  value - analog deadband value
 * returns:
 *  void
 */
void TMWDEFS_GLOBAL dnpdiag_showAnalogDeadband(
  TMWSESN *pSession,
  TMWTYPES_USHORT point,
  TMWTYPES_ANALOG_VALUE *pValue,
  TMWDIAG_ID direction)
{
  TMWDIAG_ANLZ_ID anlzId;
  char buf[128];
#if TMWCNFG_SUPPORT_DOUBLE
  TMWTYPES_DOUBLE value;
#elif TMWCNFG_SUPPORT_FLOAT
  TMWTYPES_SFLOAT value;
#endif

  if(tmwdiag_initId(&anlzId, TMWDEFS_NULL, pSession, TMWDEFS_NULL, TMWDIAG_ID_STATIC_DATA | direction) == TMWDEFS_FALSE)
  {
    return;
  }

#if TMWCNFG_SUPPORT_DOUBLE
  if((pValue->type == TMWTYPES_ANALOG_TYPE_LONG)
    ||(pValue->type == TMWTYPES_ANALOG_TYPE_SHORT))
  {
    (void)tmwtarg_snprintf(buf, sizeof(buf), "%-21s Analog Deadband %06d = %d\n",
    " ", point, dnputil_getAnlgDBandValueULong(pValue));
  }
  else
  {
    value = dnputil_getAnlgDBandValueDouble(pValue);
    (void)tmwtarg_snprintf(buf, sizeof(buf), "%-21s Analog Deadband %06d = %.10g\n", " ", point, value);
  }

#elif TMWCNFG_SUPPORT_FLOAT
  if((pValue->type == TMWTYPES_ANALOG_TYPE_LONG)
    ||(pValue->type == TMWTYPES_ANALOG_TYPE_SHORT))
  {
    (void)tmwtarg_snprintf(buf, sizeof(buf), "%-21s Analog Deadband %06d = %u\n",
    " ", point, dnputil_getAnlgDBandValueULong(pValue));
  }
  else
  {
    value = dnputil_getAnlgDBandValueFloat(pValue);
    (void)tmwtarg_snprintf(buf, sizeof(buf), "%-21s Analog Deadband %06d = %.10g\n", " ", point, value);
  }

#else
  (void)tmwtarg_snprintf(buf, sizeof(buf), "%-21s Analog Deadband %06d = %u\n",
    " ", point, dnputil_getAnlgDBandValueULong(pValue));
#endif

  tmwdiag_putLine(&anlzId, buf);
}

/* function: dnpdiag_showTimeAndDate */
void TMWDEFS_GLOBAL dnpdiag_showTimeAndDate(
  TMWSESN *pSession,
  TMWDTIME *pDateTime,
  TMWTYPES_CHAR *pText,
  TMWDIAG_ID direction)
{
  TMWDIAG_ANLZ_ID anlzId;
  char timeBuf[64];
  char buf[128];

  if(tmwdiag_initId(&anlzId, TMWDEFS_NULL, pSession, TMWDEFS_NULL, TMWDIAG_ID_STATIC_DATA | direction) == TMWDEFS_FALSE)
  {
    return;
  }
  tmwdiag_time2string(pDateTime, TMWDEFS_TIME_FORMAT_56, timeBuf, sizeof(timeBuf), TMWDEFS_FALSE);
  if(pText != TMWDEFS_NULL)
  {
    (void)tmwtarg_snprintf(buf, sizeof(buf), "%-15s %-30s %s\n", " ", pText, timeBuf);
  } 
  else
  {
    (void)tmwtarg_snprintf(buf, sizeof(buf), "%-16s Time = %s\n", " ", timeBuf);
  }
  tmwdiag_putLine(&anlzId, buf);
}

/* function: dnpdiag_showFreeze */
void TMWDEFS_GLOBAL dnpdiag_showFreeze(
  TMWSESN *pSession,
  TMWDTIME *pDateTime,
  TMWTYPES_ULONG freezeInterval,
  TMWDIAG_ID direction)
{
  TMWDIAG_ANLZ_ID anlzId;
  char timeBuf[64];
  char buf[128];

  if (tmwdiag_initId(&anlzId, TMWDEFS_NULL, pSession, TMWDEFS_NULL, TMWDIAG_ID_STATIC_DATA | direction) == TMWDEFS_FALSE)
  {
    return;
  }

  tmwdiag_time2string(pDateTime, TMWDEFS_TIME_FORMAT_56, timeBuf, sizeof(timeBuf), TMWDEFS_FALSE); 
  (void)tmwtarg_snprintf(buf, sizeof(buf), "%-15s Freeze Time = %s Freeze Interval %d \n", " ", timeBuf, freezeInterval);
  tmwdiag_putLine(&anlzId, buf);
}

/* function: dnpdiag_showIndexedTime */
void TMWDEFS_GLOBAL dnpdiag_showIndexedTime(
  TMWSESN *pSession,
  TMWTYPES_USHORT point,
  TMWDTIME *pDateTime,
  TMWTYPES_ULONG intervalCount,
  TMWTYPES_BYTE intervalUnits,
  TMWDIAG_ID direction)
{
  TMWDIAG_ANLZ_ID anlzId;
  char timeBuf[64];
  char buf[150];

  if (tmwdiag_initId(&anlzId, TMWDEFS_NULL, pSession, TMWDEFS_NULL, TMWDIAG_ID_STATIC_DATA | direction) == TMWDEFS_FALSE)
  {
    return;
  }
  tmwdiag_time2string(pDateTime, TMWDEFS_TIME_FORMAT_56, timeBuf, sizeof(timeBuf), TMWDEFS_FALSE);
 
  (void)tmwtarg_snprintf(buf, sizeof(buf), "%-21s Indexed Time %06d Start Time = %s Interval Count = %d Interval Units = %d\n", " ", point, timeBuf, intervalCount, intervalUnits);
  tmwdiag_putLine(&anlzId, buf);
}

/* function: dnpdiag_showStringData */
void TMWDEFS_GLOBAL dnpdiag_showStringData(
  TMWSESN *pSession,
  TMWTYPES_USHORT point,
  TMWTYPES_UCHAR *pBuf,
  TMWTYPES_USHORT bufLength,
  TMWDIAG_ID direction)
{
  TMWDIAG_ANLZ_ID anlzId;
  TMWTYPES_UINT offset;
  TMWTYPES_UINT sizeOfBuf;
  TMWTYPES_UINT copyLen = bufLength;
  TMWTYPES_UINT unPrintableBufLength = 0;
  TMWTYPES_BOOL isUnPrintable = TMWDEFS_FALSE;
  TMWTYPES_CHAR buf[DNPCNFG_MAX_DIAG_STRING_LENGTH + 60];  /* Pad 60 bytes for description & NULL terminator */
  TMWTYPES_CHAR unprintableBuf[DNPCNFG_MAX_DIAG_STRING_LENGTH];
  
  if(tmwdiag_initId(&anlzId, TMWDEFS_NULL, pSession, TMWDEFS_NULL, TMWDIAG_ID_STATIC_DATA | direction) == TMWDEFS_FALSE)
  {
    return;
  }
  
  sizeOfBuf = sizeof(buf);
  offset = tmwtarg_snprintf(buf, sizeOfBuf, "%-21s String Data %06d = ", " ", point);
  if (copyLen > DNPCNFG_MAX_DIAG_STRING_LENGTH)
  {
    copyLen = DNPCNFG_MAX_DIAG_STRING_LENGTH;
  }

  unPrintableBufLength += tmwtarg_snprintf(unprintableBuf + unPrintableBufLength, sizeof(unprintableBuf) - unPrintableBufLength, "hex(");

  for (TMWTYPES_UINT i = 0; i < copyLen; i++)
  {
    /* It is possible for string to contain unprintable characters. If so, we will print hex value for the whole string. */
    if (!_isPrintable(pBuf[i]))
    {
      isUnPrintable = TMWDEFS_TRUE;
    }
    unPrintableBufLength += tmwtarg_snprintf(unprintableBuf + unPrintableBufLength, sizeof(unprintableBuf) - unPrintableBufLength, "%02x ", pBuf[i]);
  }

  unPrintableBufLength += tmwtarg_snprintf(unprintableBuf + unPrintableBufLength, sizeof(unprintableBuf) - unPrintableBufLength, ")");

  if (isUnPrintable)
  {
    if (unPrintableBufLength > DNPCNFG_MAX_DIAG_STRING_LENGTH)
    {
      unPrintableBufLength = DNPCNFG_MAX_DIAG_STRING_LENGTH;
      STRNCPY(&buf[offset], (sizeOfBuf - offset), unprintableBuf, unPrintableBufLength);
      offset += unPrintableBufLength;
      /* Buffer has been truncated for printing. */
      if (unPrintableBufLength > DNPCNFG_MAX_DIAG_STRING_LENGTH)
      {
        STRCPY(&buf[offset], (sizeOfBuf - offset), "...");
        offset += 3;
      }
    }
    else
    {
      STRNCPY(&buf[offset], (sizeOfBuf - offset), unprintableBuf, unPrintableBufLength);
      offset += unPrintableBufLength;
    }
  }
  else
  {
    STRNCPY(&buf[offset], (sizeOfBuf - offset), (const char *)pBuf, copyLen);
    offset += copyLen;
    /* Buffer has been truncated for printing. */
    if (copyLen != bufLength)
    {
      STRCPY(&buf[offset], (sizeOfBuf - offset), "...");
      offset += 3;
    }
  }

  STRCPY(&buf[offset], (sizeOfBuf-offset), "\n");

  tmwdiag_putLine(&anlzId, buf);
}

/* function: dnpdiag_showStringEvent */
void TMWDEFS_GLOBAL dnpdiag_showStringEvent(
  TMWSESN *pSession,
  TMWTYPES_USHORT point,
  TMWTYPES_UCHAR *pBuf,
  TMWTYPES_UCHAR bufLength)
{
  TMWDIAG_ANLZ_ID anlzId;
  TMWTYPES_UINT offset;
  TMWTYPES_UINT sizeOfBuf;
  TMWDIAG_ID direction;
  TMWTYPES_UINT copyLen = bufLength;
  TMWTYPES_UINT unPrintableBufLength = 0;
  TMWTYPES_BOOL isUnPrintable = TMWDEFS_FALSE;
  TMWTYPES_CHAR buf[DNPCNFG_MAX_DIAG_STRING_LENGTH + 60];  /* Pad 60 bytes for description & NULL terminator */
  TMWTYPES_CHAR unprintableBuf[DNPCNFG_MAX_DIAG_STRING_LENGTH];

  direction = 0;
  sizeOfBuf = sizeof(buf);

  if (pSession->type == TMWTYPES_SESSION_TYPE_MASTER)
    direction = TMWDIAG_ID_RX;

  if (tmwdiag_initId(&anlzId, TMWDEFS_NULL, pSession, TMWDEFS_NULL, TMWDIAG_ID_EVENT_DATA | direction) == TMWDEFS_FALSE)
  {
    return;
  }

  offset = tmwtarg_snprintf(buf, sizeOfBuf, "%-21s String Event %06d = ", " ", point);
  if (copyLen > DNPCNFG_MAX_DIAG_STRING_LENGTH)
  {
    copyLen = DNPCNFG_MAX_DIAG_STRING_LENGTH;
  }

  unPrintableBufLength += tmwtarg_snprintf(unprintableBuf + unPrintableBufLength, sizeof(unprintableBuf) - unPrintableBufLength, "hex(");

  for (TMWTYPES_UINT i = 0; i < copyLen; i++)
  {
    /* It is possible for string to contain unprintable characters. If so, we will print hex value for the whole string. */
    if (!_isPrintable(pBuf[i]))
    {
      isUnPrintable = TMWDEFS_TRUE;
    }
    unPrintableBufLength += tmwtarg_snprintf(unprintableBuf + unPrintableBufLength, sizeof(unprintableBuf) - unPrintableBufLength, "%02x ", pBuf[i]);
  }

  unPrintableBufLength += tmwtarg_snprintf(unprintableBuf + unPrintableBufLength, sizeof(unprintableBuf) - unPrintableBufLength, ")");

  if (isUnPrintable)
  {
    if (unPrintableBufLength > DNPCNFG_MAX_DIAG_STRING_LENGTH)
    {
      unPrintableBufLength = DNPCNFG_MAX_DIAG_STRING_LENGTH;
      STRNCPY(&buf[offset], (sizeOfBuf - offset), unprintableBuf, unPrintableBufLength);
      offset += unPrintableBufLength;
      /* Buffer has been truncated for printing. */
      if (unPrintableBufLength > DNPCNFG_MAX_DIAG_STRING_LENGTH)
      {
        STRCPY(&buf[offset], (sizeOfBuf - offset), "...");
        offset += 3;
      }
    }
    else
    {
      STRNCPY(&buf[offset], (sizeOfBuf - offset), unprintableBuf, unPrintableBufLength);
      offset += unPrintableBufLength;
    }
  }
  else
  {
    STRNCPY(&buf[offset], (sizeOfBuf - offset), (const char *)pBuf, copyLen);
    offset += copyLen;

    /* Buffer has been truncated for printing. */
    if (copyLen != bufLength)
    {
      STRCPY(&buf[offset], (sizeOfBuf - offset), "...");
      offset += 3;
    }
  }
  STRCPY(&buf[offset], (sizeOfBuf - offset), "\n");

  tmwdiag_putLine(&anlzId, buf);
}

/* function: dnpdiag_showVirtualTerminalOutput */
void TMWDEFS_GLOBAL dnpdiag_showVirtualTerminalOutput(
  TMWSESN *pSession,
  TMWTYPES_USHORT point,
  TMWTYPES_UCHAR *pBuf,
  TMWTYPES_UCHAR bufLength)
{
  TMWDIAG_ANLZ_ID anlzId;
  TMWTYPES_UINT sizeOfBuf;
  char buf[260];
  
  sizeOfBuf = sizeof(buf);

  if(tmwdiag_initId(&anlzId, TMWDEFS_NULL, pSession, TMWDEFS_NULL, TMWDIAG_ID_STATIC_DATA | TMWDIAG_ID_RX) == TMWDEFS_FALSE)
  {
    return;
  }
  (void)tmwtarg_snprintf(buf, sizeOfBuf, "%-21s Virtual Terminal Output %06d =\n", " ", point);
  tmwdiag_putLine(&anlzId, buf);
   
  STRNCPY(buf, sizeOfBuf, (const char *)pBuf, bufLength);   
  STRCPY(&buf[bufLength], (sizeOfBuf-bufLength), "\n");
  tmwdiag_putLine(&anlzId, buf);
   
  (void)tmwtarg_snprintf(buf, sizeof(buf), "\n");
  tmwdiag_putLine(&anlzId, buf);
}

/* function: dnpdiag_showExtStringData
 * NOTE: Diag message will be truncated for extended strings > 255 bytes. This value can be increased
 *       if the entire string is displayed, but stack sizes would likely need to be increased or
 *       memory allocated from the heap. To increase the number of bytes display, increase the size
 *       of extStrBuf to the desired size.
 */
  
void TMWDEFS_GLOBAL dnpdiag_showExtStringData(
  TMWSESN *pSession,
  TMWTYPES_USHORT point,
  TMWTYPES_UCHAR *pBuf,
  TMWTYPES_USHORT bufLength,
  TMWTYPES_UCHAR flags,
  TMWDIAG_ID direction)
{
  TMWDIAG_ANLZ_ID anlzId;
  TMWTYPES_UINT offset;
  TMWTYPES_UINT sizeOfBuf;
  TMWTYPES_UINT copyLen = bufLength;
  TMWTYPES_UINT unPrintableBufLength = 0;
  TMWTYPES_BOOL isUnPrintable = TMWDEFS_FALSE;
  TMWTYPES_CHAR extStrBuf[DNPCNFG_MAX_DIAG_STRING_LENGTH + 70];  /* Pad 70 bytes for description & NULL terminator */
  TMWTYPES_CHAR unPrintableBuf[DNPCNFG_MAX_DIAG_STRING_LENGTH];

  if(tmwdiag_initId(&anlzId, TMWDEFS_NULL, pSession, TMWDEFS_NULL, TMWDIAG_ID_STATIC_DATA | direction) == TMWDEFS_FALSE)
  {
    return;
  }
  
  sizeOfBuf = sizeof(extStrBuf);
  offset = tmwtarg_snprintf(extStrBuf, sizeOfBuf, "%-21s Ext String Data %06d = ", " ", point);
  if (copyLen > DNPCNFG_MAX_DIAG_STRING_LENGTH)
  {
    copyLen = DNPCNFG_MAX_DIAG_STRING_LENGTH;
  }

  unPrintableBufLength += tmwtarg_snprintf(unPrintableBuf + unPrintableBufLength, sizeof(unPrintableBuf) - unPrintableBufLength, "hex(");

  for (TMWTYPES_UINT i = 0; i < copyLen; i++)
  {
    /* It is possible for string to contain unprintable characters. If so, we will print hex value for the whole string. */
    if (!_isPrintable(pBuf[i]))
    {
      isUnPrintable = TMWDEFS_TRUE;
    }
    unPrintableBufLength += tmwtarg_snprintf(unPrintableBuf + unPrintableBufLength, sizeof(unPrintableBuf) - unPrintableBufLength, "%02x ", pBuf[i]);
  }

  unPrintableBufLength += tmwtarg_snprintf(unPrintableBuf + unPrintableBufLength, sizeof(unPrintableBuf) - unPrintableBufLength, ")");

  if (isUnPrintable)
  {
    if (unPrintableBufLength > DNPCNFG_MAX_DIAG_STRING_LENGTH)
    {
      unPrintableBufLength = DNPCNFG_MAX_DIAG_STRING_LENGTH;
      STRNCPY(&extStrBuf[offset], (sizeOfBuf - offset), unPrintableBuf, unPrintableBufLength);
      offset += unPrintableBufLength;
      /* Buffer has been truncated for printing. */
      if (unPrintableBufLength > DNPCNFG_MAX_DIAG_STRING_LENGTH)
      {
        STRCPY(&extStrBuf[offset], (sizeOfBuf - offset), "...");
        offset += 3;
      }
    }
    else
    {
      STRNCPY(&extStrBuf[offset], (sizeOfBuf - offset), unPrintableBuf, unPrintableBufLength);
      offset += unPrintableBufLength;
    }
  }
  else
  {
    STRNCPY(&extStrBuf[offset], (sizeOfBuf - offset), (const char *)pBuf, copyLen);
    offset += copyLen;
  }

  /* Buffer has been truncated for printing. */
  if (copyLen != bufLength)
  {
    STRCPY(&extStrBuf[offset], (sizeOfBuf-offset), "...");
    offset += 3;
  }
  tmwtarg_snprintf(&extStrBuf[offset], (sizeOfBuf-offset), ", flags = 0x%02x\n", flags);

  tmwdiag_putLine(&anlzId, extStrBuf);
}

/* function: dnpdiag_showExtStringEvent */
void TMWDEFS_GLOBAL dnpdiag_showExtStringEvent(
  TMWSESN *pSession,
  TMWTYPES_USHORT point,
  TMWTYPES_UCHAR *pBuf,
  TMWTYPES_USHORT bufLength,
  TMWTYPES_UCHAR flags,
  TMWDTIME *pTimeStamp)
{
  TMWDIAG_ANLZ_ID anlzId;
  TMWTYPES_UINT offset;
  TMWTYPES_UINT sizeOfBuf;
  TMWDIAG_ID direction;
  TMWTYPES_UINT copyLen = bufLength;
  TMWTYPES_UINT unPrintableBufLength = 0;
  TMWTYPES_BOOL isUnPrintable = TMWDEFS_FALSE;
  TMWTYPES_CHAR timeBuf[64];
  TMWTYPES_CHAR extStrBuf[DNPCNFG_MAX_DIAG_STRING_LENGTH + 70];  /* Pad 70 bytes for description & NULL terminator */
  TMWTYPES_CHAR unPrintableBuf[DNPCNFG_MAX_DIAG_STRING_LENGTH];

  direction = 0;
  sizeOfBuf = sizeof(extStrBuf);

  if (pSession->type == TMWTYPES_SESSION_TYPE_MASTER)
    direction = TMWDIAG_ID_RX;

  if (tmwdiag_initId(&anlzId, TMWDEFS_NULL, pSession, TMWDEFS_NULL, TMWDIAG_ID_EVENT_DATA | direction) == TMWDEFS_FALSE)
  {
    return;
  }

  if (pTimeStamp != TMWDEFS_NULL)
    tmwdiag_time2string(pTimeStamp, TMWDEFS_TIME_FORMAT_56, timeBuf, sizeof(timeBuf), TMWDEFS_FALSE);
  else
    (void)tmwtarg_snprintf(timeBuf, sizeof(timeBuf), " ");

  offset = tmwtarg_snprintf(extStrBuf, sizeOfBuf, "%-21s Ext String Event %06d = ", timeBuf, point);
  if (copyLen > DNPCNFG_MAX_DIAG_STRING_LENGTH)
  {
    copyLen = DNPCNFG_MAX_DIAG_STRING_LENGTH;
  }

  unPrintableBufLength += tmwtarg_snprintf(unPrintableBuf + unPrintableBufLength, sizeof(unPrintableBuf) - unPrintableBufLength, "hex(");

  for (TMWTYPES_UINT i = 0; i < copyLen; i++)
  {
    /* It is possible for string to contain unprintable characters. If so, we will print hex value for the whole string. */
    if (!_isPrintable(pBuf[i]))
    {
      isUnPrintable = TMWDEFS_TRUE;
    }
    unPrintableBufLength += tmwtarg_snprintf(unPrintableBuf + unPrintableBufLength, sizeof(unPrintableBuf) - unPrintableBufLength, "%02x ", pBuf[i]);
  }

  unPrintableBufLength += tmwtarg_snprintf(unPrintableBuf + unPrintableBufLength, sizeof(unPrintableBuf) - unPrintableBufLength, ")");

  if (isUnPrintable)
  {
    if (unPrintableBufLength > DNPCNFG_MAX_DIAG_STRING_LENGTH)
    {
      unPrintableBufLength = DNPCNFG_MAX_DIAG_STRING_LENGTH;
      STRNCPY(&extStrBuf[offset], (sizeOfBuf - offset), unPrintableBuf, unPrintableBufLength);
      offset += unPrintableBufLength;
      /* Buffer has been truncated for printing. */
      if (unPrintableBufLength > DNPCNFG_MAX_DIAG_STRING_LENGTH)
      {
        STRCPY(&extStrBuf[offset], (sizeOfBuf - offset), "...");
        offset += 3;
      }
    }
    else
    {
      STRNCPY(&extStrBuf[offset], (sizeOfBuf - offset), unPrintableBuf, unPrintableBufLength);
      offset += unPrintableBufLength;
    }
  }
  else
  {
    STRNCPY(&extStrBuf[offset], (sizeOfBuf - offset), (const char *)pBuf, copyLen);
    offset += copyLen;
  }

  /* Buffer has been truncated for printing. */
  if (copyLen != bufLength)
  {
    STRCPY(&extStrBuf[offset], (sizeOfBuf - offset), "...");
    offset += 3;
  }
  tmwtarg_snprintf(&extStrBuf[offset], (sizeOfBuf - offset), ", flags = 0x%02x\n", flags);

  tmwdiag_putLine(&anlzId, extStrBuf);
}

/* function: dnpdiag_showActivateConfig */
void TMWDEFS_GLOBAL dnpdiag_showActivateConfig(
  TMWSESN *pSession,
  TMWTYPES_ULONG timeDelay,
  TMWTYPES_USHORT statusCode,
  TMWTYPES_UCHAR *pBuf,
  TMWTYPES_UCHAR bufLength)
{
  TMWDIAG_ANLZ_ID anlzId;
  TMWDIAG_ID direction;
  char buf[260];
  char buf1[260];

  direction = 0;
  if(pSession->type == TMWTYPES_SESSION_TYPE_MASTER)
    direction = TMWDIAG_ID_RX;

  if(tmwdiag_initId(&anlzId, TMWDEFS_NULL, pSession, TMWDEFS_NULL, TMWDIAG_ID_STATIC_DATA | direction) == TMWDEFS_FALSE)
  {
    return;
  }
  (void)tmwtarg_snprintf(buf, sizeof(buf), "%-21s Activate Configuration TimeDelay = %d, Status = %d\n", " ", timeDelay, statusCode);
  tmwdiag_putLine(&anlzId, buf);

  if(bufLength > 0)
  {
    if(tmwdiag_initId(&anlzId, TMWDEFS_NULL, pSession, TMWDEFS_NULL, TMWDIAG_ID_STATIC_DATA | direction) == TMWDEFS_FALSE)
    {
      return;
    }
    memcpy(buf1, pBuf, bufLength);
    buf1[bufLength] = '\0'; 
   (void)tmwtarg_snprintf(buf, sizeof(buf), "%-21s    %s\n", " ", buf1);
    tmwdiag_putLine(&anlzId, buf);
  }
}

/* function: dnpdiag_showVirtualTerminalEvent */
void TMWDEFS_GLOBAL dnpdiag_showVirtualTerminalEvent(
  TMWSESN *pSession,
  TMWTYPES_USHORT point,
  TMWTYPES_UCHAR *pBuf,
  TMWTYPES_UCHAR bufLength)
{
  TMWDIAG_ANLZ_ID anlzId;
  char buf[260];
  TMWDIAG_ID direction = 0;

  if(pSession->type == TMWTYPES_SESSION_TYPE_MASTER)
    direction = TMWDIAG_ID_RX;

  if(tmwdiag_initId(&anlzId, TMWDEFS_NULL, pSession, TMWDEFS_NULL, TMWDIAG_ID_EVENT_DATA | direction) == TMWDEFS_FALSE)
  {
    return;
  }
  (void)tmwtarg_snprintf(buf, sizeof(buf), "%-21s Virtual Terminal Event %06d =\n", " ", point);
  tmwdiag_putLine(&anlzId, buf);

  /*tmwdiag_initId(&anlzId, TMWDEFS_NULL, pSession, TMWDEFS_NULL, TMWDIAG_ID_EVENT_DATA | direction);*/
  memcpy(buf, pBuf, bufLength);
  buf[bufLength] = '\0';
  tmwdiag_putLine(&anlzId, buf);

  /*tmwdiag_initId(&anlzId, TMWDEFS_NULL, pSession, TMWDEFS_NULL, TMWDIAG_ID_EVENT_DATA | direction);*/
  (void)tmwtarg_snprintf(buf, sizeof(buf), "\n");
  tmwdiag_putLine(&anlzId, buf);
}

/* function: dnpdiag_fragmentSent */
void TMWDEFS_GLOBAL dnpdiag_fragmentSent(
  TMWCHNL *pChannel,
  TMWSESN *pSession,
  const TMWTYPES_UCHAR *pFragment,
  TMWTYPES_USHORT numBytes)
{
  _displayApplFragment(pChannel, pSession, "<===", pFragment, numBytes, 0);
}

/* function: dnpdiag_fragmentReceived */
void TMWDEFS_GLOBAL dnpdiag_fragmentReceived(
  TMWCHNL *pChannel,
  TMWSESN *pSession,
  const TMWTYPES_UCHAR *pFragment,
  TMWTYPES_USHORT numBytes)
{
  _displayApplFragment(pChannel, pSession, "===>", pFragment, numBytes, TMWDIAG_ID_RX);
}

/* function: dnpdiag_tprtFrameSent */
void TMWDEFS_GLOBAL dnpdiag_tprtFrameSent(
  TMWCHNL *pChannel,
  TMWSESN *pSession,
  const TMWTYPES_UCHAR *pFrame,
  TMWTYPES_USHORT numBytes)
{
  _displayTprtFrame(pChannel, pSession, "<~~~", pFrame, numBytes, 0);
}

/* function: dnpdiag_tprtFrameReceived */
void TMWDEFS_GLOBAL dnpdiag_tprtFrameReceived(
  TMWCHNL *pChannel,
  TMWSESN *pSession,
  const TMWTYPES_UCHAR *pFrame,
  TMWTYPES_USHORT numBytes)
{
  _displayTprtFrame(pChannel, pSession, "~~~>", pFrame, numBytes, TMWDIAG_ID_RX);
}

/* function: dnpdiag_linkFrameSent */
void TMWDEFS_GLOBAL dnpdiag_linkFrameSent(
  TMWCHNL *pChannel,
  TMWSESN *pSession,
  const TMWTYPES_UCHAR *pFrame,
  TMWTYPES_USHORT numBytes,
  TMWTYPES_USHORT retryCount)
{
  _displayLinkFrame(pChannel, pSession, "<---", pFrame, numBytes, retryCount, TMWDEFS_FALSE, 0);
}

/* function: dnpdiag_linkHeaderBlockReceived */
void * TMWDEFS_GLOBAL dnpdiag_linkHeaderBlockReceived(
  TMWCHNL *pChannel,
  TMWSESN *pSession,
  const TMWTYPES_UCHAR *pBuf,
  TMWTYPES_USHORT numBytes)
{
  DNPLINK_FRAME *pFrame = (DNPLINK_FRAME *)dnpmem_alloc(DNPMEM_LINK_FRAME_TYPE);
  if(pFrame != TMWDEFS_NULL)
  {
    memcpy(pFrame->buffer, pBuf, numBytes);
    pFrame->pChannel = pChannel;
    pFrame->pSession = pSession;
    pFrame->size = numBytes;
  }
  return(pFrame);
}

/* function: dnpdiag_linkDataBlockReceived */
void TMWDEFS_GLOBAL dnpdiag_linkDataBlockReceived(
  void *pContext,
  const TMWTYPES_UCHAR *pBuf,
  TMWTYPES_USHORT numBytes)
{
  DNPLINK_FRAME *pFrame = (DNPLINK_FRAME *)pContext;
  if(pFrame != TMWDEFS_NULL)
  {
    memcpy(pFrame->buffer + pFrame->size, pBuf, numBytes);
    pFrame->size = (TMWTYPES_USHORT)(pFrame->size + numBytes);
  }
}

/* function: dnpdiag_linkFrameComplete */
void TMWDEFS_GLOBAL dnpdiag_linkFrameComplete(
  void *pContext,
  TMWTYPES_BOOL discarded)
{
  DNPLINK_FRAME *pFrame = (DNPLINK_FRAME *)pContext;
  if(pFrame != TMWDEFS_NULL)
  {
    _displayLinkFrame(pFrame->pChannel, pFrame->pSession, "--->", pFrame->buffer, pFrame->size, 0, discarded, TMWDIAG_ID_RX);
    dnpmem_free(pFrame);
  }
}

/* function: dnpdiag_linkFrameReceived */
void TMWDEFS_GLOBAL dnpdiag_linkFrameReceived(
  TMWCHNL *pChannel,
  TMWSESN *pSession,
  const TMWTYPES_UCHAR *pFrame,
  TMWTYPES_USHORT numBytes)
{
  _displayLinkFrame(pChannel, pSession, "--->", pFrame, numBytes, 0, TMWDEFS_FALSE, TMWDIAG_ID_RX);
}


#define MAX_ROW_LENGTH 16
 

void dnpdiag_displayOctets(
  TMWDIAG_ANLZ_ID *pAnlzId,
  TMWTYPES_UCHAR *pBuf,
  TMWTYPES_USHORT length,
  TMWTYPES_BOOL   indent)
{
  int index = 0;
  char buf[128];

  while(index < length)
  {
    int i;
    int len;
    int rowLength;

    if(indent)
      len = tmwtarg_snprintf(buf, sizeof(buf), "%-26s", " ");
    else
      len = 0;

    rowLength = length - index;
    if(rowLength > MAX_ROW_LENGTH)
      rowLength = MAX_ROW_LENGTH;

    for(i = 0; i < rowLength; i++)
      len += tmwtarg_snprintf(buf + len, sizeof(buf) - len, "%02x ", pBuf[index++]);

    (void)tmwtarg_snprintf(buf + len, sizeof(buf) - len, "\n");
    if(indent)
      tmwdiag_putLine(pAnlzId, buf);
    else
      tmwtarg_putDiagString(pAnlzId, buf);
  }
}

const char * TMWDEFS_LOCAL dnpdiag_descrCodeToString(
  DNPDEFS_DATASET_DESCR_CODE code)
{
  switch(code)
  {
  case DNPDEFS_DATASET_DESCR_ID:        return("ID  ");
  case DNPDEFS_DATASET_DESCR_UUID:      return("UUID");
  case DNPDEFS_DATASET_DESCR_NSPC:      return("NSPC");
  case DNPDEFS_DATASET_DESCR_NAME:      return("NAME");
  case DNPDEFS_DATASET_DESCR_DAEL:      return("DAEL");
  case DNPDEFS_DATASET_DESCR_PTYP:      return("PTYP");
  case DNPDEFS_DATASET_DESCR_CTLS:      return("CTLS");
  case DNPDEFS_DATASET_DESCR_CTLV:      return("CTLV");
  default:                              return("UNDEF");
  }
}

const char * TMWDEFS_LOCAL dnpdiag_datasetTypeToString(
  DNPDEFS_DATASET_TYPE_CODE code)
{
  switch(code)
  {
  case DNPDEFS_DATASET_TYPE_NONE:     return("NONE");
  case DNPDEFS_DATASET_TYPE_VSTR:     return("VSTR");
  case DNPDEFS_DATASET_TYPE_UINT:     return("UINT");
  case DNPDEFS_DATASET_TYPE_INT:      return("INT ");
  case DNPDEFS_DATASET_TYPE_FLT:      return("FLT ");
  case DNPDEFS_DATASET_TYPE_OSTR:     return("OSTR");
  case DNPDEFS_DATASET_TYPE_BSTR:     return("BSTR");
  case DNPDEFS_DATASET_TYPE_TIME:     return("TIME");
  case DNPDEFS_DATASET_TYPE_UNCD:     return("UNCD");
  default:                            return("UNDEF");
  }
}

static void _displayProtoOrDescr TMWDEFS_LOCAL(
  TMWSESN *pSession,
  TMWTYPES_UCHAR elemIndex,
  DNPDATA_DATASET_DESCR_ELEM *pDescr,
  TMWDIAG_ID direction)
{  
  TMWDIAG_ANLZ_ID anlzId;
  TMWTYPES_UCHAR length;
  char strbuf[256];
  char buf[260];

  if(tmwdiag_initId(&anlzId, TMWDEFS_NULL, pSession, TMWDEFS_NULL, TMWDIAG_ID_STATIC_DATA |direction) == TMWDEFS_FALSE)
  {
    return;
  }
  (void)tmwtarg_snprintf(buf, sizeof(buf), "%-21s Element %d, Descr Type=%s Data Type=%s Max=%d\n",
                       " ", elemIndex+1, dnpdiag_descrCodeToString(pDescr->descrElemType), dnpdiag_datasetTypeToString(pDescr->dataTypeCode), pDescr->maxDataLength);

  tmwdiag_putLine(&anlzId, buf);

  length = pDescr->ancillaryValue.length;
  if(pDescr->ancillaryValue.type == DNPDATA_VALUE_STRPTR)
  {
    memcpy(strbuf, pDescr->ancillaryValue.value.pStrValue, length);
  }
  else if(pDescr->ancillaryValue.type == DNPDATA_VALUE_STRARRAY)
  {
    memcpy(strbuf, pDescr->ancillaryValue.value.strValue, length);
  }
  strbuf[length] = 0;

  switch(pDescr->descrElemType)
  {
  case DNPDEFS_DATASET_DESCR_ID:
    (void)tmwtarg_snprintf(buf, sizeof(buf), "%-24s ID,           Ancillary Value =%d\n", " ", pDescr->ancillaryValue.value.uint32Value);
    tmwdiag_putLine(&anlzId, buf);
    break;
  case DNPDEFS_DATASET_DESCR_NSPC:
    (void)tmwtarg_snprintf(buf, sizeof(buf), "%-24s Namespace,    Ancillary Value =%s\n", " ", strbuf);
    tmwdiag_putLine(&anlzId, buf);
    break;
  case DNPDEFS_DATASET_DESCR_NAME:
    (void)tmwtarg_snprintf(buf, sizeof(buf), "%-24s Name,         Ancillary Value =%s\n", " ", strbuf);
    tmwdiag_putLine(&anlzId, buf);
    break;
  case DNPDEFS_DATASET_DESCR_DAEL:
    (void)tmwtarg_snprintf(buf, sizeof(buf), "%-24s Data Element, Ancillary Value =%s\n", " ", strbuf);
    tmwdiag_putLine(&anlzId, buf);
    break;
  case DNPDEFS_DATASET_DESCR_CTLV:
    (void)tmwtarg_snprintf(buf, sizeof(buf), "%-24s Control Value Element, Ancillary Value =%s\n", " ", strbuf);
    tmwdiag_putLine(&anlzId, buf);
    break;
  case DNPDEFS_DATASET_DESCR_CTLS:
    (void)tmwtarg_snprintf(buf, sizeof(buf), "%-24s Control Data Element, Ancillary Value =%s\n", " ", strbuf);
    tmwdiag_putLine(&anlzId, buf);
    break;
  case DNPDEFS_DATASET_DESCR_UUID:
    (void)tmwtarg_snprintf(buf, sizeof(buf), "%-24s UUID,         Ancillary Value =\n", " ");
    tmwdiag_putLine(&anlzId, buf);
    dnpdiag_displayOctets(&anlzId, (TMWTYPES_UCHAR *)strbuf, length, TMWDEFS_TRUE);    
    break;
  case DNPDEFS_DATASET_DESCR_PTYP: 
    (void)tmwtarg_snprintf(buf, sizeof(buf), "%-24s Prototype,    Ancillary Value =\n", " ");
    tmwdiag_putLine(&anlzId, buf);
    dnpdiag_displayOctets(&anlzId, (TMWTYPES_UCHAR *)strbuf, 16, TMWDEFS_TRUE);

    /* Prototype may contain optional visible string name after 16 octet UUID */
    if(length > 16)
    {
      (void)tmwtarg_snprintf(buf, sizeof(buf), "%-24s  Optional Name = %s\n", " ", &strbuf[16]);
      tmwdiag_putLine(&anlzId, buf);
    }
    break;
  case DNPDEFS_DATASET_DESCR_UNKNOWN:
    break;
  }
}

/* function: dnpdiag_showDatasetProto */
void TMWDEFS_GLOBAL dnpdiag_showDatasetProto(
  TMWSESN *pSession,
  TMWTYPES_USHORT point,
  TMWTYPES_UCHAR elemIndex,
  DNPDATA_DATASET_DESCR_ELEM *pDescr,
  TMWDIAG_ID direction)
{
  TMWDIAG_ANLZ_ID anlzId;
  char buf[64];

  if(tmwdiag_initId(&anlzId, TMWDEFS_NULL, pSession, TMWDEFS_NULL, TMWDIAG_ID_STATIC_DATA | direction) == TMWDEFS_FALSE)
  {
    return;
  }
  if(elemIndex == 0)
  {
    (void)tmwtarg_snprintf(buf, sizeof(buf), "%-18s Data Set Prototype, Prototype Id =%06d, \n", " ", point);
    tmwdiag_putLine(&anlzId, buf);
  }
 
  _displayProtoOrDescr(pSession, elemIndex, pDescr, direction);
}

/* function: dnpdiag_showDatasetDscrCont */
void TMWDEFS_GLOBAL dnpdiag_showDatasetDscrCont(
  TMWSESN *pSession,
  TMWTYPES_USHORT point,
  TMWTYPES_UCHAR elemIndex,
  DNPDATA_DATASET_DESCR_ELEM *pDescr,
  TMWDIAG_ID direction)
{
  TMWDIAG_ANLZ_ID anlzId;
  char buf[64];

  if(tmwdiag_initId(&anlzId, TMWDEFS_NULL, pSession, TMWDEFS_NULL, TMWDIAG_ID_STATIC_DATA | direction) == TMWDEFS_FALSE)
  {
    return;
  }
  if(elemIndex == 0)
  {
    (void)tmwtarg_snprintf(buf, sizeof(buf), "%-18s Data Set Descriptor, Data Set Id =%06d, \n", " ", point);
    tmwdiag_putLine(&anlzId, buf);
  }

  _displayProtoOrDescr(pSession, elemIndex, pDescr, direction);
}

/* function: dnpdiag_showDatasetDscrChrs */
void TMWDEFS_GLOBAL dnpdiag_showDatasetDscrChrs(
  TMWSESN *pSession,
  TMWTYPES_USHORT point,
  TMWTYPES_UCHAR value,
  TMWDIAG_ID direction)
{
  TMWDIAG_ANLZ_ID anlzId;
  char buf[260];

  if(tmwdiag_initId(&anlzId, TMWDEFS_NULL, pSession, TMWDEFS_NULL, TMWDIAG_ID_STATIC_DATA | direction) == TMWDEFS_FALSE)
  {
    return;
  }
  (void)tmwtarg_snprintf(buf, sizeof(buf), "%-18s Data Set Descriptor Data Set Id =%06d, Characteristics=0x%02x\n",
    " ", point, value);

  tmwdiag_putLine(&anlzId, buf);
}

/* function: dnpdiag_showDatasetDscrIndx */
void TMWDEFS_GLOBAL dnpdiag_showDatasetDscrIndx(
  TMWSESN *pSession,
  TMWTYPES_USHORT point,
  TMWTYPES_UCHAR elemIndex,
  DNPDATA_DATASET_DESCR_INDEX *pDescr,
  TMWDIAG_ID direction)
{
  TMWDIAG_ANLZ_ID anlzId;
  char buf[260];

  if(tmwdiag_initId(&anlzId, TMWDEFS_NULL, pSession, TMWDEFS_NULL, TMWDIAG_ID_STATIC_DATA | direction) == TMWDEFS_FALSE)
  {
    return;
  }

  if(elemIndex == 0)
  {
    (void)tmwtarg_snprintf(buf, sizeof(buf), "%-18s Data Set Descriptor, Data Set Id =%06d, \n",
      " ", point);
    tmwdiag_putLine(&anlzId, buf);
  }
  (void)tmwtarg_snprintf(buf, sizeof(buf), "%-21s Element %d, %-32s index=%d\n",
    " ", elemIndex+1, _groupString(pDescr->pointType), pDescr->pointIndex);

  tmwdiag_putLine(&anlzId, buf);
}

/* function: dnpdiag_showDataset */
void TMWDEFS_GLOBAL dnpdiag_showDatasetTime(
  TMWSESN *pSession,
  TMWTYPES_USHORT point,
  TMWDTIME *pTimeStamp,
  TMWTYPES_BOOL isEvent,
  TMWDIAG_ID direction)
{
  TMWDIAG_ANLZ_ID anlzId;
  char timeBuf[64];
  char buf[260];

  if(tmwdiag_initId(&anlzId, TMWDEFS_NULL, pSession, TMWDEFS_NULL,
    (TMWDIAG_ID)((((isEvent) ? TMWDIAG_ID_EVENT_DATA : TMWDIAG_ID_STATIC_DATA) | direction))) == TMWDEFS_FALSE)
  {
    return;
  }

  tmwtarg_snprintf(buf, sizeof(buf), "%-18s Data Set Element 0,  Data Set Id =%06d \n",
      " ", point);
  tmwdiag_putLine(&anlzId, buf);

  tmwdiag_time2string(pTimeStamp, TMWDEFS_TIME_FORMAT_56, timeBuf, sizeof(timeBuf), TMWDEFS_FALSE);
  (void)tmwtarg_snprintf(buf, sizeof(buf), "%-21s Element   1,  %s\n", " ", timeBuf);
  tmwdiag_putLine(&anlzId, buf);
}

/* function: dnpdiag_showDataset */
void TMWDEFS_GLOBAL dnpdiag_showDataset(
  TMWSESN *pSession,
  TMWTYPES_UCHAR elemIndex,
  DNPDATA_DATASET_VALUE *pElem,
  TMWTYPES_UCHAR fc,
  TMWTYPES_BOOL isIgnored, 
  TMWTYPES_BOOL isEvent,
  TMWDIAG_ID direction)
{
  int len = 0;
  TMWTYPES_BOOL indent;
  TMWDIAG_ANLZ_ID anlzId;
  char strbuf[256];
  char buf[260];

  if(tmwdiag_initId(&anlzId, TMWDEFS_NULL, pSession, TMWDEFS_NULL,
    (TMWDIAG_ID)((((isEvent) ? TMWDIAG_ID_EVENT_DATA : TMWDIAG_ID_STATIC_DATA) | direction))) == TMWDEFS_FALSE)
  {
    return;
  }

  if(!isIgnored)
  {
   len = tmwtarg_snprintf(buf, sizeof(buf), "%-21s Element %3d,  ", " ", elemIndex+2);
  }
  else
  {
    if(fc == DNPDEFS_FC_WRITE)
      len = tmwtarg_snprintf(buf, sizeof(buf), "%-21s Element %3d,  (Control Element, Ignored on Write)\n%-50s", " ", elemIndex+2, " ");
    else if(fc == DNPDEFS_FC_OPERATE)
      len = tmwtarg_snprintf(buf, sizeof(buf), "%-21s Element %3d,  (Data Element, Ignored on Control)\n%-50s", " ", elemIndex+2, " ");
    else if(fc == DNPDEFS_FC_READ)
      len = tmwtarg_snprintf(buf, sizeof(buf), "%-21s Element %3d,  (Control Element, Ignored on Read Response)\n%-50s", " ", elemIndex+2, " ");
  }
  switch(pElem->type)
  {
  case DNPDATA_VALUE_STRPTR:  
  {
    int i;
    for(i=0; i<pElem->length; i++)
    {
      if(!_isPrintable(*(pElem->value.pStrValue+i)))
      {  
        indent = TMWDEFS_FALSE;
        if(pElem->length >= MAX_ROW_LENGTH)
        {
          (void)tmwtarg_snprintf(buf + len, sizeof(buf) - len, "\n");
          indent = TMWDEFS_TRUE;
        }
        tmwdiag_putLine(&anlzId, buf);
        dnpdiag_displayOctets(&anlzId, pElem->value.pStrValue, pElem->length, indent);
        return;
      }
    }
    memcpy(strbuf, pElem->value.pStrValue, pElem->length);
    strbuf[pElem->length] = 0;
    len += tmwtarg_snprintf(buf + len, sizeof(buf) - len, "%s", strbuf);
    break;
  }
  case DNPDATA_VALUE_STRARRAY: 
  {
    int i;
    for(i=0; i<pElem->length; i++)
    {
      if(!_isPrintable(pElem->value.strValue[i]))
      {  
        indent = TMWDEFS_FALSE;
        if(pElem->length >= MAX_ROW_LENGTH)
        {
          (void)tmwtarg_snprintf(buf + len, sizeof(buf) - len, "\n");
          indent = TMWDEFS_TRUE;
        }
        tmwdiag_putLine(&anlzId, buf);
        dnpdiag_displayOctets(&anlzId, pElem->value.strValue, pElem->length, indent);
        return;
      }
    }
    memcpy(strbuf, pElem->value.strValue, pElem->length);
    strbuf[pElem->length] = 0;
    len += tmwtarg_snprintf(buf + len, sizeof(buf) - len, "%s", strbuf);
    break;
  }
  case DNPDATA_VALUE_UINT32:    
    len += tmwtarg_snprintf(buf + len, sizeof(buf) - len, "%d", pElem->value.uint32Value);
    break;
  case DNPDATA_VALUE_INT32:   
    len += tmwtarg_snprintf(buf + len, sizeof(buf) - len, "%d", pElem->value.int32Value);
    break;
#if TMWCNFG_SUPPORT_FLOAT
  case DNPDATA_VALUE_SFLT:  
    len += tmwtarg_snprintf(buf + len, sizeof(buf) - len, "%g", pElem->value.sfltValue);
    break;
#endif
#if TMWCNFG_SUPPORT_DOUBLE
  case DNPDATA_VALUE_DOUBLE:
    len += tmwtarg_snprintf(buf + len, sizeof(buf) - len, "%g", pElem->value.doubleValue);
    break;
#endif
  case DNPDATA_VALUE_TIME: 
    tmwdiag_time2string(&pElem->value.timeValue, TMWDEFS_TIME_FORMAT_56, strbuf, sizeof(strbuf), TMWDEFS_FALSE);
    len += tmwtarg_snprintf(buf + len, sizeof(buf) - len, "%s", strbuf);
    break;
  }

  (void)tmwtarg_snprintf(buf + len, sizeof(buf) - len, "\n");
  tmwdiag_putLine(&anlzId, buf);
}


/* function: _daString
* purpose: return string representation of device attribute variation
* arguments:
*  variation - variation of device attribute
* returns:
*  constant string describing device attribute variation
*/
static const char * TMWDEFS_LOCAL _daString(
  TMWTYPES_UCHAR variation)
{
  switch(variation)
  {
  case 0:                                     return("User-specific");
  case DNPDEFS_OBJ0_CONFIG_ID:                return("Configuration ID");
  case DNPDEFS_OBJ0_CONFIG_VERSION:           return("Configuration version");
  case DNPDEFS_OBJ0_CONFIG_BLD_DATE:          return("Configuration build date");
  case DNPDEFS_OBJ0_CONFIG_CHG_DATE:          return("Configuration last change date");
  case DNPDEFS_OBJ0_CONFIG_SIGNATURE:         return("Configuration digest");
  case DNPDEFS_OBJ0_CONFIG_SIG_ALG:           return("Configuration digest algorithm");
  case DNPDEFS_OBJ0_MASTER_RID:               return("Master Resource ID");
  case DNPDEFS_OBJ0_DEVLOC_ALTITUDE:          return("Device location altitude");
  case DNPDEFS_OBJ0_DEVLOC_LONGITUDE:         return("Device location longitude");
  case DNPDEFS_OBJ0_DEVLOC_LATITUDE:          return("Device location latitude");
  case DNPDEFS_OBJ0_SECOND_OPER_NAME:         return("User-assigned secondary operator name");
  case DNPDEFS_OBJ0_PRIME_OPER_NAME:          return("User-assigned primary operator name");
  case DNPDEFS_OBJ0_SYSTEM_NAME:              return("User-assigned system name");
  case DNPDEFS_OBJ0_SEC_AUTH_VERSION:         return("Secure authentication Version");
  case DNPDEFS_OBJ0_SEC_STATS_QUANT:          return("Number of security statistics per association");
  case DNPDEFS_OBJ0_USER_SPEC_SETS:           return("ID of support for user-specific attributes");
  case DNPDEFS_OBJ0_NUM_MASTER_PROTOS:        return("Number of Master-Defined Data Set Prototypes");
  case DNPDEFS_OBJ0_NUM_OUTSTA_PROTOS:        return("Number of Outstation-Defined Data Set Prototypes");
  case DNPDEFS_OBJ0_NUM_MASTER_DATASET:       return("Number of Master-Defined Data Sets");
  case DNPDEFS_OBJ0_NUM_OUTSTA_DATASET:       return("Number of Outstation-Defined Data Sets");
  case DNPDEFS_OBJ0_MAX_BIN_OUT_PER:          return("Max number of binary outputs from object group 12 per request");
  case DNPDEFS_OBJ0_LOCAL_TIME_ACCUR:         return("Local timing accuracy");
  case DNPDEFS_OBJ0_TIME_ACCURACY_DUR:        return("Duration of time accuracy (following a time synchronization)");
  case DNPDEFS_OBJ0_ANLG_OUT_EV_SUPP:         return("Analog output events supported");
  case DNPDEFS_OBJ0_MAX_ANLG_OUT_INDEX:       return("Maximum analog output index");
  case DNPDEFS_OBJ0_NUM_ANLG_OUT_PNTS:        return("Number of analog output points");
  case DNPDEFS_OBJ0_BIN_OUT_EVENT_SUPP:       return("Binary output events supported.");
  case DNPDEFS_OBJ0_MAX_BIN_OUT_INDEX:        return("Maximum binary output index");
  case DNPDEFS_OBJ0_NUM_BIN_OUT_PNTS:         return("Number of binary output points");
  case DNPDEFS_OBJ0_FRZN_CNTR_EV_SUPP:        return("Frozen counter events supported");
  case DNPDEFS_OBJ0_FRZN_CNTR_SUPP:           return("Frozen counts supported");
  case DNPDEFS_OBJ0_CNTR_EVENT_SUPP:          return("Counter events supported");
  case DNPDEFS_OBJ0_MAX_CNTR_INDEX:           return("Maximum counter index");
  case DNPDEFS_OBJ0_NUM_CNTR_PNTS:            return("Number of counter points");
  case DNPDEFS_OBJ0_FRZN_ANLG_IN_SUPP:        return("Frozen analog inputs supported");
  case DNPDEFS_OBJ0_ANLG_IN_EVENT_SUPP:       return("Analog input events supported");
  case DNPDEFS_OBJ0_MAX_ANLG_IN_INDEX:        return("Maximum analog input index");
  case DNPDEFS_OBJ0_NUM_ANLG_IN_PNTS:         return("Number of analog input points");
  case DNPDEFS_OBJ0_DBL_IN_EVENTS_SUPP:       return("Double-bit binary input events supported");
  case DNPDEFS_OBJ0_MAX_DBL_IN_INDEX:         return("Maximum double-bit binary input index");
  case DNPDEFS_OBJ0_NUM_DBL_IN_PNTS:          return("Number of double-bit binary input points");
  case DNPDEFS_OBJ0_BIN_IN_EVENTS_SUPP:       return("Binary input events supported");
  case DNPDEFS_OBJ0_MAX_BIN_IN_INDEX:         return("Maximum binary input index");
  case DNPDEFS_OBJ0_NUM_BIN_IN_PNTS:          return("Number of binary input points");
  case DNPDEFS_OBJ0_MAX_TX_FRAG_SIZE:         return("Maximum transmit fragment size");
  case DNPDEFS_OBJ0_MAX_RX_FRAG_SIZE:         return("Maximum receive fragment size");
  case DNPDEFS_OBJ0_SFTW_VERSION_STR:         return("Device manufacturer’s software version string");
  case DNPDEFS_OBJ0_HDWR_VERSION_STR:         return("Device manufacturer’s hardware version string");
  case DNPDEFS_OBJ0_OWNER_NAME:               return("User-assigned owner name");
  case DNPDEFS_OBJ0_USER_LOCATION:            return("User-assigned location name or code string");
  case DNPDEFS_OBJ0_USER_ID_CODE:             return("User-assigned ID code/number string");
  case DNPDEFS_OBJ0_USER_NAME:                return("User-assigned name string for the outstation");
  case DNPDEFS_OBJ0_SERIAL_NUMBER:            return("Device serial number string");
  case DNPDEFS_OBJ0_SUBSET_LEVEL:             return("DNP subset level and conformance");
  case DNPDEFS_OBJ0_PRODUCT_NAME:             return("Device manufacturer’s product name and model");
  case DNPDEFS_OBJ0_DEVICE_CODE:              return("Reserved for device manufacturer’s registered DNP device code");
  case DNPDEFS_OBJ0_DEVICE_MANU_NAME:         return("Device manufacturer’s name string");
  case DNPDEFS_OBJ0_ATTRIBUTE_NAMES:          return("Reserved for retrieval of attribute names");
  case DNPDEFS_OBJ0_ALL_ATTRIBUTES:           return("Special variation for requesting return of all attributes");
  case DNPDEFS_OBJ0_LIST_ATTRIBUTES:          return("Special variation for requesting list of attributes");

  default:
    return("Private variation");
  }
}

/* function: _attrCodeString
 * purpose: return string representation of  
 * arguments:
 *  code
 * returns:
 *  constant string describing  
 */
static const char * TMWDEFS_LOCAL _attrCodeString(
  DNPDEFS_ATTRIBUTE_DATA_TYPE code)
{
  switch(code)
  {
  case DNPDEFS_ATTRIBUTE_TYPE_VSTR:     return("Visible Characters");
  case DNPDEFS_ATTRIBUTE_TYPE_UINT:     return("Unsigned Integer");
  case DNPDEFS_ATTRIBUTE_TYPE_INT:      return("Signed Integer");
  case DNPDEFS_ATTRIBUTE_TYPE_FLT:      return("Floating-point");
  case DNPDEFS_ATTRIBUTE_TYPE_OSTR:     return("Octet String");
  case DNPDEFS_ATTRIBUTE_TYPE_BSTR:     return("Bit String"); 
  case DNPDEFS_ATTRIBUTE_TYPE_DNP3TIME: return("DNP3 Time"); 

  default:
    return("Unknown");
  }
}

/* function: dnpdiag_showDeviceAttribute */
void TMWDEFS_GLOBAL dnpdiag_showDeviceAttribute(
  TMWSESN *pSession,
  TMWTYPES_USHORT point,
  TMWTYPES_UCHAR variation,
  DNPDATA_ATTRIBUTE_VALUE *pValue,
  TMWDIAG_ID direction)
{
  int len;
  TMWTYPES_BOOL indent;
  TMWTYPES_UCHAR displayVariation;
  TMWDIAG_ANLZ_ID anlzId;
  char strbuf[256];
  char buf[260];

  if(tmwdiag_initId(&anlzId, TMWDEFS_NULL, pSession, TMWDEFS_NULL, TMWDIAG_ID_STATIC_DATA | direction) == TMWDEFS_FALSE)
  {
    return;
  }

  if(point == 0)
    displayVariation = variation;
  else
    displayVariation = 0;

  len = tmwtarg_snprintf(buf, sizeof(buf), "%-21s Device Attribute  Point %d, Variation %d=%s\n", 
    " ", point, variation, _daString(displayVariation));

  tmwdiag_putLine(&anlzId, buf);

  len = tmwtarg_snprintf(buf, sizeof(buf), "%-21s     code %d=%-19s value ", 
    " ", pValue->type, _attrCodeString(pValue->type)); 

  switch(pValue->type)
  {
  case DNPDEFS_ATTRIBUTE_TYPE_VSTR:   
  {
    memcpy(strbuf, pValue->value.pStrValue, pValue->length);
    strbuf[pValue->length] = 0;
    len += tmwtarg_snprintf(buf + len, sizeof(buf) - len, "%s", strbuf);
    break;
  }
  case DNPDEFS_ATTRIBUTE_TYPE_OSTR:  
  case DNPDEFS_ATTRIBUTE_TYPE_BSTR:  
  {
    indent = TMWDEFS_FALSE;
    if(pValue->length >= MAX_ROW_LENGTH)
    {
      (void)tmwtarg_snprintf(buf + len, sizeof(buf) - len, "\n");
      indent = TMWDEFS_TRUE;
    }
    tmwdiag_putLine(&anlzId, buf);
    dnpdiag_displayOctets(&anlzId, pValue->value.pStrValue, pValue->length, indent);
    return;
    break;
  }
  case DNPDEFS_ATTRIBUTE_TYPE_UINT:    
    len += tmwtarg_snprintf(buf + len, sizeof(buf) - len, "%d", pValue->value.uintValue);
    break;
  case DNPDEFS_ATTRIBUTE_TYPE_INT:   
    len += tmwtarg_snprintf(buf + len, sizeof(buf) - len, "%d", pValue->value.intValue);
    break;
  case DNPDEFS_ATTRIBUTE_TYPE_FLT:  
    if(pValue->length == 4)
    {
#if TMWCNFG_SUPPORT_DOUBLE
      len += tmwtarg_snprintf(buf + len, sizeof(buf) - len, "%g", pValue->value.fltValue.sfltValue);
#endif
    }
    else
    {
#if TMWCNFG_SUPPORT_FLOAT
      len += tmwtarg_snprintf(buf + len, sizeof(buf) - len, "%g", pValue->value.fltValue.doubleValue);
#endif
    }
    break;

  case DNPDEFS_ATTRIBUTE_TYPE_DNP3TIME:     
    tmwdiag_time2string(&pValue->value.timeValue, TMWDEFS_TIME_FORMAT_56, strbuf, sizeof(strbuf), TMWDEFS_FALSE); 
    len += tmwtarg_snprintf(buf + len, sizeof(buf) - len, "%s", strbuf);
    break;
    
  case DNPDEFS_ATTRIBUTE_TYPE_LIST:
  case DNPDEFS_ATTRIBUTE_TYPE_EXLIST:  
    break;
  }

  (void)tmwtarg_snprintf(buf + len, sizeof(buf) - len, "\n");
  tmwdiag_putLine(&anlzId, buf);
}

/* function: dnpdiag_showDeviceAttrProperty */
void TMWDEFS_GLOBAL dnpdiag_showDeviceAttrProperty(
  TMWSESN *pSession,
  TMWTYPES_USHORT point,
  TMWTYPES_UCHAR variation,
  TMWTYPES_UCHAR property)
{
  TMWTYPES_UCHAR displayVariation;
  TMWDIAG_ANLZ_ID anlzId;
  char buf[260];
  TMWDIAG_ID direction = 0;

  if(pSession->type == TMWTYPES_SESSION_TYPE_MASTER)
    direction = TMWDIAG_ID_RX;

  if(tmwdiag_initId(&anlzId, TMWDEFS_NULL, pSession, TMWDEFS_NULL, TMWDIAG_ID_STATIC_DATA | direction) == TMWDEFS_FALSE)
  {
    return;
  }

  if(point == 0)
    displayVariation = variation;
  else
    displayVariation = 0;
  tmwtarg_snprintf(buf, sizeof(buf), "%-21s Device Attribute Property %d Point %d Variation %d=%s \n", 
    " ", property, point, variation, _daString(displayVariation));

  tmwdiag_putLine(&anlzId, buf);
}

/* function:  _cmdModeString
 * purpose: return string representation of open mode code
 * arguments:
 *  mode - mode code
 * returns:
 *  constant string describing mode code
 */
static const char * TMWDEFS_LOCAL _cmdModeString(
  DNPDEFS_FILE_MODE mode)
{
  switch(mode)
  {    
  case DNPDEFS_FILE_MODE_READ:        return("Read");
  case DNPDEFS_FILE_MODE_WRITE:       return("Write");
  case DNPDEFS_FILE_MODE_APPEND:      return("Append"); 
  default:                            return("Unknown");
  }
}

/* function: dnpdiag_showFileOpen
 * purpose: display  
 * arguments:
 *  pSession - session from which this message originated
 * returns:
 *  void
 */
void TMWDEFS_GLOBAL dnpdiag_showFileOpen(
  TMWSESN *pSession,
  TMWTYPES_CHAR *pFileName,
  TMWTYPES_USHORT fileNameLength,
  TMWTYPES_ULONG fileSize,
  TMWTYPES_MS_SINCE_70 *pTimeOfCreation,
  TMWTYPES_USHORT permissions,
  TMWTYPES_ULONG authKey,
  TMWTYPES_USHORT mode,
  TMWTYPES_USHORT maxBlockSize,
  TMWTYPES_USHORT requestId)
{ 
  TMWDIAG_ANLZ_ID anlzId;
  char tempBuf[128];
  char buf[260]; 
  TMWDIAG_ID direction = 0;

  if(pSession->type != TMWTYPES_SESSION_TYPE_MASTER)
    direction = TMWDIAG_ID_RX;

  if(tmwdiag_initId(&anlzId, TMWDEFS_NULL, pSession, TMWDEFS_NULL, TMWDIAG_ID_STATIC_DATA | direction) == TMWDEFS_FALSE)
  {
    return;
  }
 
  if (fileNameLength >= 128)
  {
    fileNameLength = 127;
  }
  STRNCPY(tempBuf, 128, pFileName, fileNameLength);
  tempBuf[fileNameLength] = 0x00; /* null terminate it */

  tmwtarg_snprintf(buf, sizeof(buf), "%-21s File Open, Filename %s, permissions = 0x%0x, authKey = 0x%0x\n",
    " ", tempBuf, permissions, authKey);  

  tmwdiag_putLine(&anlzId, buf); 
  
  tmwtarg_snprintf(buf, sizeof(buf), "%-21s mode = %s, requested block size = %u, requestId = %d\n",
    " ",  _cmdModeString(mode), maxBlockSize, requestId);  

  tmwdiag_putLine(&anlzId, buf);

  if(pTimeOfCreation != TMWDEFS_NULL)
  { 
    TMWDTIME timeOfCreation;
    dnpdtime_msSince70ToDateTime(&timeOfCreation, pTimeOfCreation);
    tmwdiag_time2string(&timeOfCreation, TMWDEFS_TIME_FORMAT_56, tempBuf, sizeof(tempBuf), TMWDEFS_FALSE);
  }
  else
    (void)tmwtarg_snprintf(tempBuf, sizeof(tempBuf), "unknown");

  (void)tmwtarg_snprintf(buf, sizeof(buf), "%-21s creation time %s,  file size %u\n", " ", tempBuf, fileSize);
  tmwdiag_putLine(&anlzId, buf);
}

/* function: dnpdiag_showFileClose
 * purpose: display  
 * arguments:
 *  pSession - session from which this message originated
 *  handle - file handle
 *  requestId - id of request submitted
 * returns:
 *  void
 */
void TMWDEFS_GLOBAL dnpdiag_showFileClose(
  TMWSESN *pSession,
  TMWTYPES_ULONG handle,
  TMWTYPES_USHORT requestId)
{
  TMWDIAG_ANLZ_ID anlzId;
  char buf[260];
  TMWDIAG_ID direction = 0;

  if(pSession->type != TMWTYPES_SESSION_TYPE_MASTER)
    direction = TMWDIAG_ID_RX;

  if(tmwdiag_initId(&anlzId, TMWDEFS_NULL, pSession, TMWDEFS_NULL, TMWDIAG_ID_STATIC_DATA | direction) == TMWDEFS_FALSE)
  {
    return;
  }
 
  tmwtarg_snprintf(buf, sizeof(buf), "%-21s File Close File(0x%0x), requestId = %d\n", 
      " ", handle, requestId);
  
  tmwdiag_putLine(&anlzId, buf);
}

void TMWDEFS_GLOBAL dnpdiag_showFileData(
  TMWSESN *pSession,
  TMWTYPES_ULONG handle,
  TMWTYPES_ULONG blockNumber,
  TMWTYPES_USHORT numBytes,
  TMWTYPES_BOOL last,
  TMWTYPES_BOOL isEvent,
  TMWDIAG_ID direction)
{
  TMWDIAG_ANLZ_ID anlzId;
  char buf[260];

  if(tmwdiag_initId(&anlzId, TMWDEFS_NULL, pSession, TMWDEFS_NULL, (TMWDIAG_ID)(((isEvent) ? TMWDIAG_ID_EVENT_DATA : TMWDIAG_ID_STATIC_DATA) | direction)) == TMWDEFS_FALSE)
  {
    return;
  }

  if(direction & TMWDIAG_ID_RX)
  {
    tmwtarg_snprintf(buf, sizeof(buf), "%-21s File Data Receive, File(0x%0x),  blockNumber = %d,  number bytes = %d,  lastBlockFlag = %d\n", 
      " ", handle, blockNumber, numBytes, last);
  }
  else
  {
    tmwtarg_snprintf(buf, sizeof(buf), "%-21s File Data Send, File(0x%0x),  blockNumber = %d,  number bytes = %d,  lastBlockFlag = %d\n", 
      " ", handle, blockNumber, numBytes, last);
  }
  tmwdiag_putLine(&anlzId, buf);
}

/* function: dnpdiag_showFileString */
void TMWDEFS_GLOBAL dnpdiag_showFileString(
  TMWSESN *pSession,
  TMWTYPES_USHORT point,
  TMWTYPES_UCHAR *pBuf,
  TMWTYPES_USHORT bufLength,
  TMWDIAG_ID direction)
{
  TMWDIAG_ANLZ_ID anlzId;
  TMWTYPES_UINT sizeOfBuf;
  TMWTYPES_UINT offset;
  char buf[320];

  sizeOfBuf = sizeof(buf);

  if(tmwdiag_initId(&anlzId, TMWDEFS_NULL, pSession, TMWDEFS_NULL, TMWDIAG_ID_STATIC_DATA | direction) == TMWDEFS_FALSE)
  {
    return;
  }

  offset = tmwtarg_snprintf(buf, sizeOfBuf, "%-21s File String %06d = ", " ", point);
  if(bufLength >= (sizeOfBuf-offset))
    bufLength = (TMWTYPES_USHORT)(sizeOfBuf-offset-4);

  STRNCPY(&buf[offset], (sizeOfBuf-offset), (const char *)pBuf, bufLength); 
  offset += bufLength;
  STRCPY(&buf[offset], (sizeOfBuf-offset), "\n");

  tmwdiag_putLine(&anlzId, buf);
}

/* function:  _cmdStatusString
 * purpose: return string representation of status code
 * arguments:
 *  status - status code
 * returns:
 *  constant string describing status code
 */
static const char * TMWDEFS_LOCAL _cmdStatusString(
  DNPDEFS_FILE_CMD_STAT status)
{
  switch(status)
  {    
  case DNPDEFS_FILE_CMD_STAT_SUCCESS:        return("Success");
  case DNPDEFS_FILE_CMD_STAT_DENIED:         return("Permission Denied");
  case DNPDEFS_FILE_CMD_STAT_INV_MODE:       return("Invalid Mode");
  case DNPDEFS_FILE_CMD_STAT_NOT_FOUND:      return("File Not Found");
  case DNPDEFS_FILE_CMD_STAT_LOCKED:         return("File Locked");
  case DNPDEFS_FILE_CMD_STAT_TOO_MANY:       return("Open File Count Exceeded");
  case DNPDEFS_FILE_CMD_STAT_INV_HANDLE:     return("File Handle Invalid"); 
  case DNPDEFS_FILE_CMD_STAT_INV_BLOCK:      return("Block Size Not Supported");
  case DNPDEFS_FILE_CMD_STAT_LOST_COMM:      return("Lost Communications");
  case DNPDEFS_FILE_CMD_STAT_CANT_ABORT:     return("Can't Abort"); 
  case DNPDEFS_FILE_CMD_STAT_MISC:           return("Other Error"); 
  default:                                   return("Unknown");
  }
}

/* function: dnpdiag_showFileStatus */
void dnpdiag_showFileStatus(
  TMWSESN *pSession, 
  TMWTYPES_ULONG handle,
  TMWTYPES_ULONG fileSize,
  TMWTYPES_USHORT maxBlockSize,
  TMWTYPES_USHORT requestId,
  DNPDEFS_FILE_CMD_STAT status,
  TMWTYPES_USHORT nOptionalChars,
  const TMWTYPES_CHAR *pOptionalChars)
{
  TMWDIAG_ANLZ_ID anlzId;
  char buf[128];
  TMWDIAG_ID direction = 0;

  if(pSession->type == TMWTYPES_SESSION_TYPE_MASTER)
    direction = TMWDIAG_ID_RX;

  TMWTARG_UNUSED_PARAM( nOptionalChars);
  TMWTARG_UNUSED_PARAM( pOptionalChars);

  if(tmwdiag_initId(&anlzId, TMWDEFS_NULL, pSession, TMWDEFS_NULL, TMWDIAG_ID_STATIC_DATA | direction) == TMWDEFS_FALSE)
  {
    return;
  }

  (void)tmwtarg_snprintf(buf, sizeof(buf), "%-21s File(0x%08x) status from Outstation requestId = %d\n", " ", handle , requestId);
  tmwdiag_putLine(&anlzId, buf); 
  
  (void)tmwtarg_snprintf(buf, sizeof(buf), "%-21s status = %s, file size = %u, max block size = %u\n", " ", _cmdStatusString(status), fileSize, maxBlockSize);
  tmwdiag_putLine(&anlzId, buf);
}

/* function: dnpdiag_showFileInfo */
void dnpdiag_showFileInfo(
  TMWSESN *pSession, 
  TMWTYPES_USHORT fileNameOffset,
  TMWTYPES_USHORT fileNameSize,
  DNPDEFS_FILE_TYPE fileType,
  TMWTYPES_ULONG  fileSize,
  const TMWDTIME *pTimeOfCreation,
  DNPDEFS_FILE_PERMISSIONS permissions,
  const TMWTYPES_CHAR *pFileName)
{
  TMWDIAG_ANLZ_ID anlzId; 
  char buf[256];
  char tempBuf[128];
  TMWDIAG_ID direction = 0;

  if(pSession->type == TMWTYPES_SESSION_TYPE_MASTER)
    direction = TMWDIAG_ID_RX;

  TMWTARG_UNUSED_PARAM( fileNameOffset);

  if (fileNameSize >= 128)
  {
    fileNameSize = 127;
  }

  STRNCPY(tempBuf, 128, pFileName, fileNameSize);
  tempBuf[fileNameSize] = 0x00; /* null terminate it */

  if(tmwdiag_initId(&anlzId, TMWDEFS_NULL, pSession, TMWDEFS_NULL, TMWDIAG_ID_STATIC_DATA | direction) == TMWDEFS_FALSE)
  {
    return;
  }

  (void)tmwtarg_snprintf(buf, sizeof(buf), "%-21s File info from Outstation fileName = %s\n", " ", tempBuf);
  tmwdiag_putLine(&anlzId, buf);
  
  if(pTimeOfCreation != TMWDEFS_NULL)
    tmwdiag_time2string(pTimeOfCreation, TMWDEFS_TIME_FORMAT_56, tempBuf, sizeof(tempBuf), TMWDEFS_FALSE);
  else
    (void)tmwtarg_snprintf(tempBuf, sizeof(tempBuf), "unknown");

  (void)tmwtarg_snprintf(buf, sizeof(buf), "%-21s creation time %s,  file size %u\n", " ", tempBuf, fileSize);
  tmwdiag_putLine(&anlzId, buf);
 
  _toPermissionsString(tempBuf, 128, permissions);
  (void)tmwtarg_snprintf(buf, sizeof(buf), "%-21s permissions %s\n", " ", tempBuf);
  tmwdiag_putLine(&anlzId, buf);

  (void)tmwtarg_snprintf(buf, sizeof(buf), "%-21s file type '%s'\n", " ", (fileType == DNPDEFS_FILE_TYPE_DIRECTORY) ? "directory" : "simple");
  tmwdiag_putLine(&anlzId, buf);
}


/* function:  _tferStatusString
 * purpose: return string representation of status code
 * arguments:
 *  status - status code
 * returns:
 *  constant string describing status code
 */
static const char * TMWDEFS_LOCAL _tferStatusString(
  DNPDEFS_FILE_TFER_STAT status)
{
  switch(status)
  {    
  case DNPDEFS_FILE_TFER_STAT_SUCCESS:       return("Success");
  case DNPDEFS_FILE_TFER_STAT_INV_HANDLE:    return("File Handle Invalid");
  case DNPDEFS_FILE_TFER_STAT_LOST_COMM:     return("Lost Communications");
  case DNPDEFS_FILE_TFER_STAT_NOT_OPEN:      return("No Previous Matching Open");
  case DNPDEFS_FILE_TFER_STAT_HANDLE_EXP:    return("Handle Has Expired");
  case DNPDEFS_FILE_TFER_STAT_OVERRUN:       return("Buffer Overrun");
  case DNPDEFS_FILE_TFER_STAT_BAD_FILE:      return("File Unusable");
  case DNPDEFS_FILE_TFER_STAT_BAD_BLOCK:     return("Block Number Out of Sequence");
  case DNPDEFS_FILE_TFER_STAT_MISC:          return("Other Error");
  default:                                   return("Unknown");
  }
}

/* function: dnpdiag_showFileDataStatus */
void dnpdiag_showFileDataStatus(
  TMWSESN *pSession, 
  TMWTYPES_ULONG handle,
  TMWTYPES_ULONG blockNumber,
  TMWTYPES_BOOL lastBlockFlag,
  DNPDEFS_FILE_TFER_STAT status,
  TMWTYPES_USHORT nOptionalChars,
  const TMWTYPES_CHAR *pOptionalChars)
{
  TMWDIAG_ANLZ_ID anlzId;
  char buf[256];
  TMWDIAG_ID direction = 0;

  if(pSession->type == TMWTYPES_SESSION_TYPE_MASTER)
    direction = TMWDIAG_ID_RX;

  TMWTARG_UNUSED_PARAM( nOptionalChars);
  TMWTARG_UNUSED_PARAM( pOptionalChars);

  if(tmwdiag_initId(&anlzId, TMWDEFS_NULL, pSession, TMWDEFS_NULL, TMWDIAG_ID_STATIC_DATA | direction) == TMWDEFS_FALSE)
  {
    return;
  }
  
  (void)tmwtarg_snprintf(buf, sizeof(buf), "%-21s File(0x%08x) data status from Outstation blockNumber = %d, lastBlockFlag = %d\n", " ", handle , blockNumber, lastBlockFlag);
  tmwdiag_putLine(&anlzId, buf);
   
  (void)tmwtarg_snprintf(buf, sizeof(buf), "%-21s status = %s\n", " " ,_tferStatusString(status));
  tmwdiag_putLine(&anlzId, buf);
}
 
static const char * TMWDEFS_LOCAL _authReason(TMWTYPES_UCHAR reason)
{
  switch(reason)
  {
    case DNPAUTH_REASON_CRITICAL:
      return "CRITICAL";
    default:
      return "undefined";
  }
} 

static const char * TMWDEFS_LOCAL _authKeyWrapAlgorithm(TMWTYPES_UCHAR algorithm)
{
  switch(algorithm)
  { 
    case DNPAUTH_KEYWRAP_AES128:
      return "AES 128"; 
    case DNPAUTH_KEYWRAP_AES256:
      return "AES 256"; 
    default:
      return "undefined";
  }
}

static const char * TMWDEFS_LOCAL _authMACAlgorithm(TMWTYPES_UCHAR algorithm)
{
  switch(algorithm)
  {
    case 0:
      return "none"; 
    case DNPAUTH_HMAC_SHA1_4OCTET:
      return "SHA1 4 OCTET"; 
    case DNPAUTH_MAC_SHA1_8OCTET:
      return "SHA1 8 OCTET"; 
    case DNPAUTH_MAC_SHA1_10OCTET:
      return "SHA1 10 OCTET"; 
    case DNPAUTH_MAC_SHA256_8OCTET:
      return "SHA256 8 OCTET";
    case DNPAUTH_MAC_SHA256_16OCTET:
      return "SHA256 16 OCTET";
    case DNPAUTH_MAC_AESGMAC_12OCTET:
      return "AES-GMAC 12 OCTET";
    default:
      return "undefined";
  }
}

static const char * TMWDEFS_LOCAL _authKeyStatus(TMWTYPES_UCHAR status)
{
  switch(status)
  {
    case DNPAUTH_KEY_OK:
      return "OK";
    case DNPAUTH_KEY_NOTINIT:
      return "NOTINIT";
    case DNPAUTH_KEY_COMMFAIL:
      return "COMMFAIL";
    case DNPAUTH_KEY_AUTHFAIL:
      return "AUTHFAIL";
    default:
      return "undefined";
  }
}

static const char * TMWDEFS_LOCAL _authV5Status(TMWTYPES_UCHAR status)
{
  switch(status)
  {
    case DNPAUTH_V5_AUTHENTICATED:
      return "AUTHENTICATED"; 
    case DNPAUTH_V5_NOTAUTHENTICATED:
      return "NOT AUTHENTICATED";
    case DNPAUTH_V5_USERNOTFOUND:
      return "USER NUMBER NOT FOUND";
    default:
      return "undefined";
  }
} 

static const char * TMWDEFS_LOCAL _authError(TMWTYPES_UCHAR error)
{
  switch(error)
  {
    case DNPAUTH_ERROR_AUTHENTFAILED:
      return "AUTHENTICATION FAILED";
    case DNPAUTH_ERROR_UNEXPECTEDREPLY:
      return "UNEXPECTED REPLY";
    case DNPAUTH_ERROR_NOREPLY:
      return "NO REPLY"; 
    case DNPAUTH_ERROR_AGGRESSNOTPERMIT:
      return "AGGRESSIVE NOT SUPPORTED";
    case DNPAUTH_ERROR_MACNOTPERMIT:
      return "MAC Algorithm NOT SUPPORTED";
    case DNPAUTH_ERROR_KEYWRAPNOTPERMIT:
      return "KEY WRAP Algorithm NOT SUPPORTED"; 
    case DNPAUTH_ERROR_AUTHORFAILED:
      return "AUTHORIZATION FAILED";
    case DNPAUTH_ERROR_UPDCHGNOTPERMIT:
      return "UPDATE KEY CHANGE METHOD NOT PERMITTED";
    case DNPAUTH_ERROR_INVALIDSIG:
      return "INVALID SIGNATURE";
    case DNPAUTH_ERROR_INVALIDCERTDATA:
      return "INVALID CERTIFICATION DATA";
    case DNPAUTH_ERROR_UNKNOWN_USER:
      return "UNKNOWN USER"; 
    case DNPAUTH_ERROR_MAX_STATUS_REQ:
      return "MAX SESSION KEY REQUEST EXCEEDED"; 
 
    default:
      return "undefined";
  }
} 
 
/* function: _authOperationString
 * purpose: return string representation of  
 * arguments: 
 * returns:
 *  constant string describing  
 */
static const char * TMWDEFS_LOCAL _authOperationString(
  TMWTYPES_UCHAR operation)
{
  switch(operation)
  {
    case DNPAUTH_USER_STATUS_ADD:      return("Add");
    case DNPAUTH_USER_STATUS_DELETE:   return("Delete"); 
    case DNPAUTH_USER_STATUS_CHANGE:   return("Change"); 
    default:                           return("Unknown");
  }
}

/* function: _authMethodString
 * purpose: return string representation of  
 * arguments: 
 * returns:
 *  constant string describing  
 */
static const char * TMWDEFS_LOCAL _authMethodString(
  TMWTYPES_UCHAR keyChangeMethod)
{
  switch(keyChangeMethod)
  { 
    case DNPAUTH_KEYCH_SYMAES128_SHA1:           return("Symmetric AES-128/SHA-1-HMAC");
    case DNPAUTH_KEYCH_SYMAES256_SHA256:         return("Symmetric AES-256/SHA-256-HMAC");
    case DNPAUTH_KEYCH_SYMAES256_GMAC:           return("Symmetric AES-256/AES-GMAC"); 
    case DNPAUTH_KEYCH_ASYM_RSA1024_SHA1:        return("Asymmetric RS-1024 DSA SHA-1/SHA-1-HMAC"); 
    case DNPAUTH_KEYCH_ASYM_RSA2048_SHA256:      return("Asymmetric RSA-2048/DSA SHA-256/SHA-256-HMAC");
    case DNPAUTH_KEYCH_ASYM_RSA3072_SHA256:      return("Asymmetric RSA-3072/DSA SHA-256/SHA-256-HMAC");
    case DNPAUTH_KEYCH_ASYM_RSA2048_SHA256_GMAC: return("Asymmetric RSA-2048/DSA SHA-256/AES-GMAC");
    case DNPAUTH_KEYCH_ASYM_RSA3072_SHA256_GMAC: return("Asymmetric RSA-3072/DSA SHA-256/AES-GMAC");
    case DNPAUTH_KEYCH_ASYM_RSA1024_RSA_SHA1:        return("Asymmetric RS-1024/RSA SHA-1/SHA-1-HMAC");
    case DNPAUTH_KEYCH_ASYM_RSA2048_RSA_SHA256:      return("Asymmetric RSA-2048/RSA SHA-256/SHA-256-HMAC");
    case DNPAUTH_KEYCH_ASYM_RSA3072_RSA_SHA256:      return("Asymmetric RSA-3072/RSA SHA-256/SHA-256-HMAC");
    case DNPAUTH_KEYCH_ASYM_RSA2048_RSA_SHA256_GMAC: return("Asymmetric RSA-2048/RSA SHA-256/AES-GMAC");
    case DNPAUTH_KEYCH_ASYM_RSA3072_RSA_SHA256_GMAC: return("Asymmetric RSA-3072/RSA SHA-256/AES-GMAC");

    default:                                     return("Unknown");
  }
}

/* function: dnpdiag_showAuthHeader */
void TMWDEFS_GLOBAL dnpdiag_showAuthHeader(
  TMWSESN *pSession,
  DNPAUTH_OBJECT_HEADER *pHdr)
{
  TMWDIAG_ANLZ_ID anlzId;
  char buf[128];

  if(tmwdiag_initId(&anlzId, TMWDEFS_NULL, pSession, TMWDEFS_NULL, TMWDIAG_ID_SECURITY_HDRS | TMWDIAG_ID_RX) == TMWDEFS_FALSE)
  {
    return;
  }

  (void)tmwtarg_snprintf(buf, sizeof(buf), "%-16sRx Object %d(%s), variation %d, qualifier 0x%02x(%s) \n", " ",
    pHdr->group, _groupString(pHdr->group), pHdr->variation, pHdr->qualifier, _qualifierString(pHdr->qualifier));

  tmwdiag_skipLine(&anlzId);
  tmwdiag_putLine(&anlzId, buf);
}
 
/* function: dnpdiag_showAuthTestHeader */
void TMWDEFS_GLOBAL dnpdiag_showAuthTestHeader(
  TMWSESN *pSession,
  DNPAUTH_OBJECT_HEADER *pHdr)
{
  TMWDIAG_ANLZ_ID anlzId;
  char buf[128];

  if(tmwdiag_initId(&anlzId, TMWDEFS_NULL, pSession, TMWDEFS_NULL, TMWDIAG_ID_SECURITY_HDRS | TMWDIAG_ID_RX) == TMWDEFS_FALSE)
  {
    return;
  }

  (void)tmwtarg_snprintf(buf, sizeof(buf), "%-16sdnpauthtest Rx Object %d(%s), variation %d, qualifier 0x%02x(%s) \n", " ",
    pHdr->group, _groupString(pHdr->group), pHdr->variation, pHdr->qualifier, _qualifierString(pHdr->qualifier));

  tmwdiag_skipLine(&anlzId);
  tmwdiag_putLine(&anlzId, buf);
}

/* function: dnpdiag_showAuthTestForce */
void TMWDEFS_GLOBAL dnpdiag_showAuthTestForce(
  TMWSESN *pSession)
{
  TMWDIAG_ANLZ_ID anlzId;
  char buf[128];

  if(tmwdiag_initId(&anlzId, TMWDEFS_NULL, pSession, TMWDEFS_NULL, TMWDIAG_ID_SECURITY_HDRS) == TMWDEFS_FALSE)
  {
    return;
  }

  (void)tmwtarg_snprintf(buf, sizeof(buf), "%-16s%s \n", " ",
    "Secure Authentication, Forced Error condition for testing");

  tmwdiag_skipLine(&anlzId);
  tmwdiag_putLine(&anlzId, buf);
}

 /* function: dnpdiag_showAuthChallenge */
void TMWDEFS_GLOBAL dnpdiag_showAuthChallenge(
  TMWSESN *pSession,
  TMWTYPES_USHORT userNumber,
  TMWTYPES_ULONG sequenceNumber,
  TMWTYPES_UCHAR algorithm,
  TMWTYPES_UCHAR reason,
  TMWDIAG_ID direction)
{
  TMWDIAG_ANLZ_ID anlzId;
  char buf[260];

  if(tmwdiag_initId(&anlzId, TMWDEFS_NULL, pSession, TMWDEFS_NULL, TMWDIAG_ID_SECURITY_DATA | direction) == TMWDEFS_FALSE)
  {
    return;
  }
   
  tmwtarg_snprintf(buf, sizeof(buf), "%-16s%s Authentication Challenge, User %d, Sequence(CSQ) %d, MAC Algorithm %d-%s, Reason %d-%s\n", 
    " ", (TMWDIAG_ID_RX & direction) ? "Rx":"Tx", userNumber, sequenceNumber, algorithm, _authMACAlgorithm(algorithm), reason, _authReason(reason));
 
  tmwdiag_putLine(&anlzId, buf);
}

/* function: dnpdiag_showAuthReply */
void TMWDEFS_GLOBAL dnpdiag_showAuthReply(
  TMWSESN *pSession,
  TMWTYPES_USHORT userNumber,
  TMWTYPES_ULONG sequenceNumber,
  TMWTYPES_BOOL status,
  TMWDIAG_ID direction)
{
  TMWDIAG_ANLZ_ID anlzId;
  char buf[260];
 
  if(tmwdiag_initId(&anlzId, TMWDEFS_NULL, pSession, TMWDEFS_NULL, TMWDIAG_ID_SECURITY_DATA | direction) == TMWDEFS_FALSE)
  {
    return;
  }

  if(TMWDIAG_ID_RX & direction)
  {
    tmwtarg_snprintf(buf, sizeof(buf), "%-16sRx Authentication Reply, User %d, Sequence(CSQ) %d %s\n", 
      " ",userNumber, sequenceNumber, (status)? "SUCCESS":"FAILURE");
  }
  else
  {
    tmwtarg_snprintf(buf, sizeof(buf), "%-16sTx Authentication Reply, User %d, Sequence(CSQ) %d\n", 
      " ", userNumber, sequenceNumber);
  }
  tmwdiag_putLine(&anlzId, buf);
}

/* function: dnpdiag_showAuthAggressive */
void TMWDEFS_GLOBAL dnpdiag_showAuthAggressive(
  TMWSESN *pSession,
  TMWTYPES_USHORT userNumber,
  TMWTYPES_ULONG sequenceNumber,
  TMWTYPES_ULONG expectedSequence,
  TMWTYPES_BOOL status,
  TMWDIAG_ID direction)
{
  TMWDIAG_ANLZ_ID anlzId;
  char buf[260];
 
  if(tmwdiag_initId(&anlzId, TMWDEFS_NULL, pSession, TMWDEFS_NULL, TMWDIAG_ID_SECURITY_DATA | direction) == TMWDEFS_FALSE)
  {
    return;
  }
  
  if(TMWDIAG_ID_RX & direction)
  { 
    /* For SA Version 4, quit displaying what I think the sequence number should be, it varies depending on order. */
    if(expectedSequence != 0)
    {
      tmwtarg_snprintf(buf, sizeof(buf), "%-16sRx Authentication Aggressive Mode Request, User %d, Sequence(CSQ) %d, Expected Sequence %d %s\n", 
        " ", userNumber, sequenceNumber, expectedSequence, (status)? "SUCCESS":"FAILURE");
    }
    else
    {
      tmwtarg_snprintf(buf, sizeof(buf), "%-16sRx Authentication Aggressive Mode Request, User %d, Sequence(CSQ) %d, %s\n", 
        " ", userNumber, sequenceNumber, (status)? "SUCCESS":"FAILURE");
    }
  }
  else
  { 
    tmwtarg_snprintf(buf, sizeof(buf), "%-16sTx Authentication Aggressive Mode Request, User %d, Sequence(CSQ) %d\n", 
      " ", userNumber, sequenceNumber);
  }
  tmwdiag_putLine(&anlzId, buf);
}

void TMWDEFS_GLOBAL dnpdiag_showAuthKeyStatReq(
  TMWSESN *pSession,
  TMWTYPES_USHORT userNumber,   
  TMWDIAG_ID direction)
{ 
  TMWDIAG_ANLZ_ID anlzId;
  char buf[260];

  if(tmwdiag_initId(&anlzId, TMWDEFS_NULL, pSession, TMWDEFS_NULL, TMWDIAG_ID_SECURITY_DATA | direction) == TMWDEFS_FALSE)
  {
    return;
  }
 
  tmwtarg_snprintf(buf, sizeof(buf), "%-16s%s Authentication Key Status Request, User %d\n", 
    " ",  (TMWDIAG_ID_RX & direction) ? "Rx":"Tx", userNumber);

  tmwdiag_putLine(&anlzId, buf);
}

void dnpdiag_showAuthKeyStatus(
  TMWSESN *pSession,  
  TMWTYPES_USHORT userNumber,
  TMWTYPES_ULONG sequenceNumber,
  TMWTYPES_UCHAR keyWrapAlgorithm,
  TMWTYPES_UCHAR status,   
  TMWTYPES_UCHAR macAlgorithm,
  TMWDIAG_ID direction)
{
  TMWDIAG_ANLZ_ID anlzId;
  char buf[260];

  if(tmwdiag_initId(&anlzId, TMWDEFS_NULL, pSession, TMWDEFS_NULL, TMWDIAG_ID_SECURITY_DATA | direction) == TMWDEFS_FALSE)
  {
    return;
  }
 
  tmwtarg_snprintf(buf, sizeof(buf), "%-16s%s Authentication Key Status, User %d, Sequence(KSQ) %d, Key Wrap Algorithm %d-%s, MAC Algorithm %d-%s, Status %d-%s\n", 
    " ",  (TMWDIAG_ID_RX & direction) ? "Rx":"Tx", userNumber, sequenceNumber, keyWrapAlgorithm, _authKeyWrapAlgorithm(keyWrapAlgorithm), macAlgorithm, _authMACAlgorithm(macAlgorithm), status, _authKeyStatus(status));

  tmwdiag_putLine(&anlzId, buf);
}

void dnpdiag_showAuthKeyStatusStatus(
  TMWSESN *pSession,  
  TMWTYPES_USHORT userNumber,
  TMWTYPES_UCHAR status)
{
  TMWDIAG_ANLZ_ID anlzId;
  char buf[260];

  if(tmwdiag_initId(&anlzId, TMWDEFS_NULL, pSession, TMWDEFS_NULL, TMWDIAG_ID_SECURITY_DATA | TMWDIAG_ID_RX) == TMWDEFS_FALSE)
  {
    return;
  }
      
  tmwtarg_snprintf(buf, sizeof(buf), "%-16s%s Authentication Key Status, User %d, %s  \n", 
    " ",  "Rx", userNumber, _authV5Status(status));

  tmwdiag_putLine(&anlzId, buf);
}

void TMWDEFS_GLOBAL dnpdiag_showAuthKeyChange(
  TMWSESN *pSession,
  TMWTYPES_USHORT userNumber, 
  TMWTYPES_USHORT keyLength,
  TMWTYPES_ULONG sequenceNumber,
  TMWTYPES_ULONG expectedSequence,
  TMWTYPES_BOOL status,
  TMWDIAG_ID direction)
{ 
  TMWDIAG_ANLZ_ID anlzId;
  char buf[260];

  if(tmwdiag_initId(&anlzId, TMWDEFS_NULL, pSession, TMWDEFS_NULL, TMWDIAG_ID_SECURITY_DATA | direction) == TMWDEFS_FALSE)
  {
    return;
  }
 
  if(TMWDIAG_ID_RX & direction)
  { 
    tmwtarg_snprintf(buf, sizeof(buf), "%-16sRx Authentication Key Change Request, User %d, Key Length %d, Sequence(KSQ) %d,  Expected %d %s\n", 
      " ", userNumber, keyLength, sequenceNumber, expectedSequence, (status)? "SUCCESS":"FAILURE");
  }
  else
  { 
    tmwtarg_snprintf(buf, sizeof(buf), "%-16sTx Authentication Key Change Request, User %d, Key Length %d, Sequence(KSQ) %d\n", 
      " ", userNumber, keyLength, sequenceNumber);
  }
  tmwdiag_putLine(&anlzId, buf);
}

void TMWDEFS_GLOBAL dnpdiag_monitorAuthKeyChange(
  TMWSESN *pSession,
  TMWTYPES_USHORT userNumber, 
  TMWTYPES_ULONG sequenceNumber)
{ 
  TMWDIAG_ANLZ_ID anlzId;
  char buf[260];

  if(tmwdiag_initId(&anlzId, TMWDEFS_NULL, pSession, TMWDEFS_NULL, TMWDIAG_ID_SECURITY_DATA | TMWDIAG_ID_RX) == TMWDEFS_FALSE)
  {
    return;
  }
 
  tmwtarg_snprintf(buf, sizeof(buf), "%-16sRx Authentication Key Change Request, User %d, Sequence(KSQ) %d\n", 
    " ", userNumber, sequenceNumber);

  tmwdiag_putLine(&anlzId, buf);
}

/* function: dnpdiag_showAuthError  */
void TMWDEFS_GLOBAL dnpdiag_showAuthError(
  TMWSESN *pSession,
  TMWTYPES_USHORT userNumber,
  TMWTYPES_USHORT assocId,
  TMWTYPES_ULONG sequenceNumber,
  TMWTYPES_UCHAR errorCode,
  TMWDTIME *pTimeStamp,
  TMWTYPES_CHAR *pErrorText,
  TMWTYPES_USHORT textLen,
  TMWTYPES_BOOL isEvent,
  TMWDIAG_ID direction)
{
  TMWDIAG_ANLZ_ID anlzId;
  char buf[256];
  char timeBuf[64];
  char errorBuf[64];

  if(isEvent)
  {
    if(tmwdiag_initId(&anlzId, TMWDEFS_NULL, pSession, TMWDEFS_NULL, 
      TMWDIAG_ID_EVENT_DATA | TMWDIAG_ID_SECURITY_DATA | direction) == TMWDEFS_FALSE)
    {
      return;
    }
  }
  else
  {
    if(tmwdiag_initId(&anlzId, TMWDEFS_NULL, pSession, TMWDEFS_NULL, 
      TMWDIAG_ID_SECURITY_DATA | direction) == TMWDEFS_FALSE)
    {
      return;
    }
  }
 
  tmwdiag_time2string(pTimeStamp, TMWDEFS_TIME_FORMAT_56, timeBuf, sizeof(timeBuf), TMWDEFS_FALSE);

  if(textLen > 0)
  {
    if(textLen >= sizeof(errorBuf))
      textLen = sizeof(errorBuf)-1;

    memcpy(errorBuf, pErrorText, textLen);
    errorBuf[textLen]=0;

    tmwtarg_snprintf(buf, sizeof(buf), "%-16s%s Authentication Error, User %d, Assoc Id %d, Sequence %d, Error Code %d-%s, %s\n", 
      timeBuf,  (TMWDIAG_ID_RX & direction) ? "Rx":"Tx", userNumber, assocId, sequenceNumber, errorCode, _authError(errorCode), errorBuf);
  }
  else
  {
    tmwtarg_snprintf(buf, sizeof(buf), "%-16s%s Authentication Error, User %d, Assoc Id %d, Sequence %d, Error Code %d-%s\n", 
      timeBuf,  (TMWDIAG_ID_RX & direction) ? "Rx":"Tx", userNumber, assocId, sequenceNumber, errorCode, _authError(errorCode));
  }
  tmwdiag_putLine(&anlzId, buf);
}

void TMWDEFS_GLOBAL dnpdiag_authShowData(
  TMWSESN *pSession,
  TMWTYPES_UCHAR dataType,
  TMWTYPES_UCHAR *pData,
  TMWTYPES_ULONG length,
  TMWDIAG_ID direction)
{
  TMWDIAG_ANLZ_ID anlzId;
  char buf[128];

  if (tmwdiag_initId(&anlzId, TMWDEFS_NULL, pSession, TMWDEFS_NULL, TMWDIAG_ID_SECURITY_DATA | direction) == TMWDEFS_FALSE)
  {
    return;
  }

  switch (dataType)
  {
  case dataTypeUserPublicKey:
    tmwtarg_snprintf(buf, sizeof(buf), "%-10s      Authentication, User Public Key, Length = %d\n", " ", length);
    break;
  case dataTypeCertData:
    tmwtarg_snprintf(buf, sizeof(buf), "%-10s      Authentication, Cert Data, Length = %d\n", " ", length);
    break;
  case dataTypeRcvdMAC:
    tmwtarg_snprintf(buf, sizeof(buf), "%-10s      Authentication, Received MAC Data, Length = %d\n", " ", length);
    break;
  case dataTypeGenMAC:
    tmwtarg_snprintf(buf, sizeof(buf), "%-10s      Authentication, Generated MAC Data, Length = %d\n", " ", length);
    break;
  case dataTypeKeyData:
    if (direction & TMWDIAG_ID_RX)
      tmwtarg_snprintf(buf, sizeof(buf), "%-10s      Authentication, Received Encrypted Key Data, Length = %d\n", " ", length);
    else
      tmwtarg_snprintf(buf, sizeof(buf), "%-10s      Authentication, Generated Encrypted Key Data, Length = %d\n", " ", length);
    break;
  case dataTypeRcvdChallData:
    tmwtarg_snprintf(buf, sizeof(buf), "%-10s      Authentication, Received Challenge Data, Length = %d\n", " ", length);
    break;
  case dataTypeGenChallData:
    tmwtarg_snprintf(buf, sizeof(buf), "%-10s      Authentication, Generated Challenge Data, Length = %d\n", " ", length);
    break;
  case dataTypePlainData:
    tmwtarg_snprintf(buf, sizeof(buf), "%-10s      Authentication, Plain Text, Length = %d\n", " ", length);
    break;
  case dataTypeSessKeyData:
    tmwtarg_snprintf(buf, sizeof(buf), "%-10s      Authentication, Session Key Change plain text, Length = %d\n", " ", length);
    break;
  default:
    tmwtarg_snprintf(buf, sizeof(buf), "%-10s      Authentication, other Data, Length = %d\n", " ", length);
  }
  tmwdiag_putLine(&anlzId, buf);

  if (dataType == dataTypeSessKeyData)
  {
    int index = 0;
    /* This displays the 2 byte key length on the first line
    * 16 byte keys on the next two lines 32 byte on 4 lines
    * and the data from the Key Status message on the last line
    */
    int len = tmwtarg_snprintf(buf, sizeof(buf), "%16s", " ");
    int i;
    for (i = 0; i < 2; i++)
      len += tmwtarg_snprintf(buf + len, sizeof(buf) - len, "%02x ", pData[index++]);
    (void)tmwtarg_snprintf(buf + len, sizeof(buf) - len, "\n");
    tmwdiag_putLine(&anlzId, buf);

    length -= 2;
    pData = pData + 2;
  }

  _hexBytesOut(&anlzId, pData, length);
}

void TMWDEFS_GLOBAL dnpdiag_authInitVector(
  TMWSESN *pSession,
  TMWTYPES_UCHAR *pData,
  TMWTYPES_USHORT length,
  TMWDIAG_ID direction) 
{
  TMWDIAG_ANLZ_ID anlzId;
  char buf[128];

  if(tmwdiag_initId(&anlzId, TMWDEFS_NULL, pSession, TMWDEFS_NULL, TMWDIAG_ID_SECURITY_DATA | direction) == TMWDEFS_FALSE)
  {
    return;
  }
  
  tmwtarg_snprintf(buf, sizeof(buf), "%-10s      Authentication, initialization vector, Length = %d\n", " ", length);
  tmwdiag_putLine(&anlzId, buf);

  _hexBytesOut(&anlzId, pData, length);
}

void TMWDEFS_GLOBAL dnpdiag_authUserCert(
  TMWSESN *pSession,
  TMWTYPES_USHORT nameLength,
  TMWTYPES_UCHAR *pName,
  TMWTYPES_UCHAR keyChangeMethod,
  TMWTYPES_UCHAR certType, 
  TMWDIAG_ID direction)
{ 
  TMWDIAG_ANLZ_ID anlzId;
  char buf[256];
  char nameBuf[64];

  if(tmwdiag_initId(&anlzId, TMWDEFS_NULL, pSession, TMWDEFS_NULL, TMWDIAG_ID_SECURITY_DATA | direction) == TMWDEFS_FALSE)
  {
    return;
  }
  
  tmwtarg_snprintf(buf, sizeof(buf), "%-10s      Authentication, User Certificate\n", " ");
  tmwdiag_putLine(&anlzId, buf);
  
  if(nameLength > 63)
    nameLength = 63;

  memcpy(nameBuf, pName, nameLength);
  nameBuf[nameLength]=0;

  tmwtarg_snprintf(buf, sizeof(buf), "%16s User Name %s, Key Change Method %d %s, Cert Type %d\n", 
    " ", nameBuf, keyChangeMethod, _authMethodString(keyChangeMethod), certType);
  
  tmwdiag_putLine(&anlzId, buf);
}


void TMWDEFS_GLOBAL dnpdiag_authUserStatus(
  TMWSESN *pSession,
  TMWTYPES_USHORT nameLength,
  TMWTYPES_UCHAR *pName,
  TMWTYPES_UCHAR keyChangeMethod,
  TMWTYPES_UCHAR operation,
  TMWTYPES_USHORT userRole,
  TMWTYPES_USHORT expires,
  TMWTYPES_ULONG  sequence,
  TMWTYPES_ULONG  lastSequence,
  TMWDIAG_ID direction)
{ 
  TMWDIAG_ANLZ_ID anlzId;
  char buf[256];
  char nameBuf[64];

  if(tmwdiag_initId(&anlzId, TMWDEFS_NULL, pSession, TMWDEFS_NULL, TMWDIAG_ID_SECURITY_DATA | direction) == TMWDEFS_FALSE)
  {
    return;
  }
  
  tmwtarg_snprintf(buf, sizeof(buf), "%-10s      Authentication, User Status Change\n", " ");
  tmwdiag_putLine(&anlzId, buf);
  
  if(nameLength > 63)
    nameLength = 63;

  memcpy(nameBuf, pName, nameLength);
  nameBuf[nameLength]=0;

  if(direction == TMWDIAG_ID_RX)
    tmwtarg_snprintf(buf, sizeof(buf), "%16s User Name %s, Key Change Method %d-%s, Operation %d-%s, Role %d, Expiry %d, Sequence(SCS) %d, last rcvd Sequence(SCS) %d\n", 
      " ", nameBuf, keyChangeMethod, _authMethodString(keyChangeMethod), operation, _authOperationString(operation), userRole, expires, sequence, lastSequence);
  else
    tmwtarg_snprintf(buf, sizeof(buf), "%16s User Name %s, Key Change Method %d-%s, Operation %d-%s, Role %d, Expiry %d, Sequence(SCS) %d\n", 
      " ", nameBuf, keyChangeMethod, _authMethodString(keyChangeMethod), operation, _authOperationString(operation), userRole, expires, sequence);

  tmwdiag_putLine(&anlzId, buf);
}

void TMWDEFS_GLOBAL dnpdiag_authUpdKeyChgReq(
  TMWSESN *pSession,
  TMWTYPES_USHORT nameLength,
  TMWTYPES_UCHAR *pName,
  TMWTYPES_UCHAR keyChangeMethod,
  TMWTYPES_USHORT challengeLength,
  TMWTYPES_UCHAR *pChallenge,
  TMWDIAG_ID direction)
{ 
  TMWDIAG_ANLZ_ID anlzId;
  char buf[128];
  char nameBuf[64];
  
  if(tmwdiag_initId(&anlzId, TMWDEFS_NULL, pSession, TMWDEFS_NULL, TMWDIAG_ID_SECURITY_DATA | direction) == TMWDEFS_FALSE)
  {
    return;
  }
  
  tmwtarg_snprintf(buf, sizeof(buf), "%-10s      Authentication, Update Key Change Request\n", " ");
  tmwdiag_putLine(&anlzId, buf);
  
  if(nameLength > 63)
    nameLength = 63;

  memcpy(nameBuf, pName, nameLength);
  nameBuf[nameLength]=0;

  tmwtarg_snprintf(buf, sizeof(buf), "%16s User Name %s, keyChangeMethod %d-%s\n", 
      " ", nameBuf, keyChangeMethod, _authMethodString(keyChangeMethod));
  tmwdiag_putLine(&anlzId, buf);

  if (direction & TMWDIAG_ID_RX)
    tmwtarg_snprintf(buf, sizeof(buf), "%-10s      Received Challenge Data, Length = %d\n", " ", challengeLength);
  else
    tmwtarg_snprintf(buf, sizeof(buf), "%-10s      Generated Challenge Data, Length = %d\n", " ", challengeLength);

  tmwdiag_putLine(&anlzId, buf);

  _hexBytesOut(&anlzId, pChallenge, challengeLength);
}

void TMWDEFS_GLOBAL dnpdiag_authUpdKeyChange(
  TMWSESN *pSession,
  TMWTYPES_USHORT userNumber,
  TMWTYPES_ULONG sequenceNumber,
  TMWDIAG_ID direction)
{
  TMWDIAG_ANLZ_ID anlzId;
  char buf[128];

  if(tmwdiag_initId(&anlzId, TMWDEFS_NULL, pSession, TMWDEFS_NULL, TMWDIAG_ID_SECURITY_DATA | direction) == TMWDEFS_FALSE)
  {
    return;
  }
  
  tmwtarg_snprintf(buf, sizeof(buf), "%-10s      Authentication, Update Key Change\n", " ");
  tmwdiag_putLine(&anlzId, buf);
   
  tmwtarg_snprintf(buf, sizeof(buf), "%16s User %d, Sequence(KSQ) %d \n", 
      " ", userNumber, sequenceNumber);
  tmwdiag_putLine(&anlzId, buf);
}

void TMWDEFS_GLOBAL dnpdiag_authUpdKeyChgSig(
  TMWSESN *pSession,
  TMWDIAG_ID direction)
{
  TMWDIAG_ANLZ_ID anlzId;
  char buf[128];

  if(tmwdiag_initId(&anlzId, TMWDEFS_NULL, pSession, TMWDEFS_NULL, TMWDIAG_ID_SECURITY_DATA | direction) == TMWDEFS_FALSE)
  {
    return;
  }

  tmwtarg_snprintf(buf, sizeof(buf), "%-10s      Authentication, Update Key Change Signature\n", " ");
  tmwdiag_putLine(&anlzId, buf);
}

void TMWDEFS_GLOBAL dnpdiag_authUpdKeyChgConf(
  TMWSESN *pSession,
  TMWTYPES_USHORT userNumber,
  TMWTYPES_BOOL status,
  TMWDIAG_ID direction) 
{
  TMWDIAG_ANLZ_ID anlzId;
  char buf[128];

  if(tmwdiag_initId(&anlzId, TMWDEFS_NULL, pSession, TMWDEFS_NULL, TMWDIAG_ID_SECURITY_DATA | direction) == TMWDEFS_FALSE)
  {
    return;
  }
   
  if(direction & TMWDIAG_ID_RX)
  { 
    tmwtarg_snprintf(buf, sizeof(buf), "%-10s      Authentication, Receive Update Key Change Confirmation, User %d, Status %s\n", " ", userNumber, (status)?"SUCCESS":"FAILURE");
  }
  else
  {
    tmwtarg_snprintf(buf, sizeof(buf), "%-10s      Authentication, Send Update Key Change Confirmation, User %d\n", " ", userNumber);
  }
  tmwdiag_putLine(&anlzId, buf);
}

void TMWDEFS_GLOBAL dnpdiag_authUpdKeyChgRply(
  TMWSESN *pSession,   
  TMWTYPES_USHORT userNumber, 
  TMWTYPES_ULONG sequenceNumber, 
  TMWTYPES_UCHAR *pData, 
  TMWTYPES_USHORT length,
  TMWDIAG_ID direction) 
{
  TMWDIAG_ANLZ_ID anlzId;
  char buf[128];

  TMWTARG_UNUSED_PARAM(pData); 
  TMWTARG_UNUSED_PARAM(length);

  if(tmwdiag_initId(&anlzId, TMWDEFS_NULL, pSession, TMWDEFS_NULL, TMWDIAG_ID_SECURITY_DATA | direction) == TMWDEFS_FALSE)
  {
    return;
  }

  tmwtarg_snprintf(buf, sizeof(buf), "%-10s      Authentication, Update Key Change Reply\n", " ");
  tmwdiag_putLine(&anlzId, buf);  

  tmwtarg_snprintf(buf, sizeof(buf), "%16s User %d, Sequence(KSQ) %d \n", 
      " ", userNumber, sequenceNumber);
  tmwdiag_putLine(&anlzId, buf);

  if (direction & TMWDIAG_ID_RX) 
    tmwtarg_snprintf(buf, sizeof(buf), "%-10s      Received Challenge Data, Length = %d\n", " ", length);
  else
    tmwtarg_snprintf(buf, sizeof(buf), "%-10s      Generated Challenge Data, Length = %d\n", " ", length);

  tmwdiag_putLine(&anlzId, buf);

  _hexBytesOut(&anlzId, pData, length);
}

/* function: dnpdiag_showAuthSecStat */
void TMWDEFS_GLOBAL dnpdiag_showAuthSecStat(
  TMWSESN *pSession,
  TMWTYPES_USHORT assocId,
  TMWTYPES_USHORT point,
  TMWTYPES_ULONG value,
  TMWTYPES_UCHAR flags,
  TMWTYPES_BOOL isEvent,
  TMWDTIME *pTimeStamp)
{
  TMWDIAG_ANLZ_ID anlzId;
  char timeBuf[64];
  char buf[128];
  TMWDIAG_ID direction = 0;
  TMWTARG_UNUSED_PARAM(assocId);

  if(pSession->type == TMWTYPES_SESSION_TYPE_MASTER)
    direction = TMWDIAG_ID_RX;

  if(tmwdiag_initId(&anlzId, TMWDEFS_NULL, pSession, TMWDEFS_NULL,
    (TMWDIAG_ID)((((isEvent) ? TMWDIAG_ID_EVENT_DATA : TMWDIAG_ID_STATIC_DATA) | direction))) == TMWDEFS_FALSE)
  {
    return;
  }

  if(pTimeStamp != TMWDEFS_NULL)
    tmwdiag_time2string(pTimeStamp, TMWDEFS_TIME_FORMAT_56, timeBuf, sizeof(timeBuf), TMWDEFS_FALSE);
  else
    (void)tmwtarg_snprintf(timeBuf, sizeof(timeBuf), " ");

  (void)tmwtarg_snprintf(buf, sizeof(buf), "%-21s Security Statistic %06d %s = %lu, flags = 0x%02x\n", timeBuf, point, dnpdiag_securityStatisticString(point), value, flags);
  tmwdiag_putLine(&anlzId, buf);
}

/* function: dnpdiag_linkAddressUnknown */
void TMWDEFS_GLOBAL dnpdiag_linkAddressUnknown(
  TMWCHNL *pChannel,
  TMWTYPES_USHORT srcAddress,
  TMWTYPES_USHORT destAddress)
{
  char buf[256];
  TMWTARG_UNUSED_PARAM(pChannel);
  (void)tmwtarg_snprintf(buf, sizeof(buf), "Frame rejected - Link address unknown, src address %d dest address %d", srcAddress, destAddress);
  TMWDIAG_CHNL_ERROR(pChannel, buf);
}

/* function: dnpdiag_errorMsgEnable */
TMWTYPES_BOOL TMWDEFS_GLOBAL dnpdiag_errorMsgEnable(
  DNPDIAG_ERROR_ENUM errorNumber,
  TMWTYPES_BOOL value)
{
  if((errorNumber > DNPDIAG_ERROR_ENUM_MAX)
    || (errorNumber < 0))
  {
    return(TMWDEFS_FALSE);
  }

  if(value)
    _errorMsgDisabled[errorNumber/8] &=  ~(1 <<(errorNumber%8));
  else
    _errorMsgDisabled[errorNumber/8] |=  (1 <<(errorNumber%8));

  return(TMWDEFS_TRUE);
}

/* function: _errorMsgDisabledFunc */
static TMWTYPES_BOOL TMWDEFS_LOCAL _errorMsgDisabledFunc(
   DNPDIAG_ERROR_ENUM errorNumber)
{
  int value;

  if ((errorNumber > DNPDIAG_ERROR_ENUM_MAX)
    || (errorNumber < 0))
  {
    return(TMWDEFS_TRUE);
  }

  value = _errorMsgDisabled[errorNumber/8] & (1 <<(errorNumber%8));
  if(value == 0)
    return(TMWDEFS_FALSE);
  else
    return(TMWDEFS_TRUE);
}

/* function: dnpdiag_errorMsg */
void TMWDEFS_GLOBAL dnpdiag_errorMsg(
  TMWCHNL *pChannel,
  TMWSESN *pSession,
  DNPDIAG_ERROR_ENUM errorNumber,
  TMWTYPES_CHAR *pExtraTextMsg)
{
  const char *pName;
  TMWDIAG_ANLZ_ID anlzId;
  char buf[256];

#ifdef TMW_SUPPORT_MONITOR
  /* If in analyzer or listen only mode, do not display this error */
  if(pChannel != TMWDEFS_NULL && pChannel->pPhysContext->monitorMode)
#endif
  if(_errorMsgDisabledFunc(errorNumber))
    return;
 
  if(tmwdiag_initId(&anlzId, pChannel, pSession, TMWDEFS_NULL, TMWDIAG_ID_APPL | TMWDIAG_ID_ERROR) == TMWDEFS_FALSE)
  {
    return;
  }

  if(pSession!= TMWDEFS_NULL)
  {
    pName = tmwsesn_getSessionName(pSession);
  }
  else
  {
    pName = tmwchnl_getChannelName(pChannel);
  }

  if(pName == (const char *)0)
    pName = " ";

  if(pExtraTextMsg == TMWDEFS_NULL)
    (void)tmwtarg_snprintf(buf, sizeof(buf), "**** %s   %s ****\n", pName, dnpDiagErrorMsg[errorNumber].pErrorMsg);
  else
    (void)tmwtarg_snprintf(buf, sizeof(buf), "**** %s   %s  %s****\n", pName, dnpDiagErrorMsg[errorNumber].pErrorMsg, pExtraTextMsg);

  tmwdiag_skipLine(&anlzId);
  tmwdiag_putLine(&anlzId, buf);
}

/* function: dnpdiag_securityStatisticString
 * purpose: return string representation of security Statistic index
 * arguments:
 *  index - statistic index
 * returns:
 *  constant string describing security statistic
 */
const char * TMWDEFS_GLOBAL dnpdiag_securityStatisticString(
  TMWTYPES_USHORT index)
{
  switch(index)
  {
  case DNPAUTH_UNEXPECTED_MSG_INDEX:    return("Unexpected Messages");
  case DNPAUTH_AUTHOR_FAIL_INDEX:       return("Authorization Failures");
  case DNPAUTH_AUTHENT_FAIL_INDEX:      return("Authentication Failures");
  case DNPAUTH_REPLY_TIMEOUT_INDEX:     return("Reply Timeouts ");
  case DNPAUTH_REKEY_DUETOFAIL_INDEX:   return("Rekeys Due to Authentication Failure");
  case DNPAUTH_TOTAL_MSG_SENT_INDEX:    return("Total Messages Sent");
  case DNPAUTH_TOTAL_MSG_RCVD_INDEX:    return("Total Messages Received");
  case DNPAUTH_CRIT_MSG_SENT_INDEX:     return("Critical Messages Sent");
  case DNPAUTH_CRIT_MSG_RCVD_INDEX:     return("Critical Messages Received");
  case DNPAUTH_DISCARDED_MSG_INDEX:     return("Discarded Messages");
  case DNPAUTH_ERROR_MSG_SENT_INDEX:    return("Error Messages Sent");
  case DNPAUTH_ERROR_MSG_RCVD_INDEX:    return("Error Messages Rcvd");
  case DNPAUTH_SUCCESS_AUTHENT_INDEX:   return("Successful Authentications");
  case DNPAUTH_SESKEY_CHANGE_INDEX:     return("Session Key Changes");
  case DNPAUTH_FAILSESKEY_CHANGE_INDEX: return("Failed Session Key Changes");
  case DNPAUTH_UPDKEY_CHANGE_INDEX:     return("Update Key Changes");     
  case DNPAUTH_FAILUPDKEY_CHANGE_INDEX: return("Failed Update Key Changes");  
  case DNPAUTH_REKEY_DUE_RESTART_INDEX: return("Rekeys Due to Restarts");

  default:                              return("Unknown");     
  }
}


#endif /* TMWCNFG_SUPPORT_DIAG */
