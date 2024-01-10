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

/* file: dnpdefs.h
 * description: Generic DNP definitions
 */
#ifndef DNPDEFS_DEFINED
#define DNPDEFS_DEFINED

#include "tmwscl/utils/tmwdefs.h"
#include "tmwscl/utils/tmwtypes.h"

/* link layer definitions */

/* link layer header indices */
#define DNPDEFS_LH_INDEX_FIRST_SYNC    0
#define DNPDEFS_LH_INDEX_SECOND_SYNC   1
#define DNPDEFS_LH_INDEX_LENGTH        2
#define DNPDEFS_LH_INDEX_CONTROL       3
#define DNPDEFS_LH_INDEX_DEST_ADDR     4
#define DNPDEFS_LH_INDEX_SOURCE_ADDR   6
#define DNPDEFS_LH_INDEX_CRC           8

/* bit masks for Data Link Header CONTROL byte */
#define DNPDEFS_LC_DIR_MASK       0x80   /* mask for direction             */
#define DNPDEFS_LC_PRM_MASK       0x40   /* mask for Primary Message       */
#define DNPDEFS_LC_FCB_MASK       0x20   /* mask for Frame Count Bit       */
#define DNPDEFS_LC_FCV_MASK       0x10   /* mask for Frame Count Bit Valid */
#define DNPDEFS_LC_DFC_MASK       0x10   /* Data Flow Control bit          */
#define DNPDEFS_LC_FUNCTION_MASK  0x0f   /* mask for Function Code         */

/* Primary Frame Control Byte function codes */
#define DNPDEFS_LCF_PRI_LINK_RESET         0
#define DNPDEFS_LCF_PRI_RESET_USER         1 /* obsolete */
#define DNPDEFS_LCF_PRI_TEST_LINK          2
#define DNPDEFS_LCF_PRI_CNFRM_DATA         3
#define DNPDEFS_LCF_PRI_UNCNFRM_DATA       4
#define DNPDEFS_LCF_PRI_REQUEST_STATUS     9

/* Secondary Frame Control Byte Function Codes */
#define DNPDEFS_LCF_SEC_CONFIRM_ACK        0
#define DNPDEFS_LCF_SEC_CONFIRM_NACK       1
#define DNPDEFS_LCF_SEC_STATUS_OF_LINK    11
#define DNPDEFS_LCF_SEC_NOT_FUNCTIONING   14
#define DNPDEFS_LCF_SEC_NOT_USED          15

/* link header value definitions */
#define DNPDEFS_SELF_ADDR             0xFFFC /* Address used for self address */
                                             /* support. If enabled slave will*/
                                             /* respond to this address as if */
                                             /* it were the slave address     */
#define DNPDEFS_BROADCAST_ADDR_NOCON  0xFFFD /* Address used to broadcast     */
                                             /* messages to all devices       */
                                             /* no confirmation necessary     */
#define DNPDEFS_BROADCAST_ADDR_CON    0xFFFE /* Address used to broadcast     */
                                             /* messages to all devices       */
                                             /* confirmation is required      */
#define DNPDEFS_BROADCAST_ADDR_ORIG   0xFFFF /* Address used to broadcast     */
                                             /* messages to all devices.      */
                                             /* This address was used by      */
                                             /* the original broadcast        */
                                             /* implementation and is         */
                                             /* included here for backward    */
                                             /* compatibility                 */

#define DNPDEFS_FIRST_SYNC_CHAR         0x05 /* First two bytes that must  */
#define DNPDEFS_SECOND_SYNC_CHAR        0x64 /* be preceed every DNP       */
                                             /* physical message           */

#define DNPDEFS_LINK_HDR_NUM_DATA_BYTES    5 /* Number of bytes in header  */
                                             /* counted toward length      */
                                             /* (start, stop, length, and  */
                                             /* 2 byte CRC not counted)    */
#define DNPDEFS_LINK_HDR_NO_CRC_SIZE       8 /* Number of bytes in header  */
                                             /* including sync chars,      */
                                             /* length, control, dest, and */
                                             /* source, but not CRC)       */
#define DNPDEFS_LINK_HDR_AND_CRC_SIZE     10 /* Number of bytes in header  */
                                             /* including sync chars,      */
                                             /* length, control, dest,     */
                                             /* source, and 2 byte CRC)    */
#define DNPDEFS_FULL_BLOCK_NO_CRC_SIZE    16 /* number of bytes in a full  */
                                             /* block, not including 2     */
                                             /* byte CRC                   */
#define DNPDEFS_FULL_BLOCK_AND_CRC_SIZE   18 /* number of bytes in a full  */
                                             /* block, including 2 byte    */
                                             /* CRC                        */
#define DNPDEFS_MAX_FRAME_SIZE           292 /* Maximum number of bytes in */
                                             /* a frame when length field  */
                                             /* is maximum (255).  This    */
                                             /* includes bytes for all 17  */
                                             /* possible 2-byte CRCs, the  */
                                             /* two sync characters and    */
                                             /* the length byte (in the    */
                                             /* link header).              */

#define DNPDEFS_RECEIVE_FRAME_SIZE  (DNPDEFS_MAX_FRAME_SIZE - (17-1)*2)
                                             /* As frames are received,    */
                                             /* each CRC is transferred to */
                                             /* the buffer.  Then after    */
                                             /* checking the CRC for       */
                                             /* correctness, the CRCs are  */
                                             /* overwritten by subsequent  */
                                             /* data bytes. Therefore, the */
                                             /* received buffer only needs */
                                             /* to leave room for one CRC. */

/* (pseudo) transport layer definitions */

/* Tranport header indices */
#define DNPDEFS_TH_SIZE           1     /* Size of transport header         */

/* Tranport header values */
#define DNPDEFS_TH_FIRST          0x40  /* These are the opposites of the   */
#define DNPDEFS_TH_FINAL          0x80  /* appl layer first and final bits! */
#define DNPDEFS_TH_SEQUENCE_MASK  0x3f  /* used to extract transport seq    */
                                        /* number                           */

/* application layer definitions */

/* application layer header indices */
#define DNPDEFS_AH_INDEX_APPL_CTRL      0
#define DNPDEFS_AH_INDEX_FUNC_CODE      1
#define DNPDEFS_AH_INDEX_REQ_ASDU       2
#define DNPDEFS_AH_INDEX_IIN            2
#define DNPDEFS_AH_INDEX_IIN2           3
#define DNPDEFS_AH_INDEX_RESP_ASDU      4

/* application layer control values */
#define DNPDEFS_AC_FIRST                0x80
#define DNPDEFS_AC_FINAL                0x40
#define DNPDEFS_AC_FIRST_AND_FINAL     (DNPDEFS_AC_FIRST | DNPDEFS_AC_FINAL)
#define DNPDEFS_AC_CONFIRM              0x20
#define DNPDEFS_AC_UNSOLICITED          0x10
#define DNPDEFS_AC_SEQUENCE_MASK        0x0f

/* application layer Function code values */
#define DNPDEFS_FC_CONFIRM              0x00
#define DNPDEFS_FC_READ                 0x01
#define DNPDEFS_FC_WRITE                0x02
#define DNPDEFS_FC_SELECT               0x03
#define DNPDEFS_FC_OPERATE              0x04
#define DNPDEFS_FC_DIRECT_OP            0x05
#define DNPDEFS_FC_DIRECT_OP_NOACK      0x06
#define DNPDEFS_FC_FRZ                  0x07
#define DNPDEFS_FC_FRZ_NOACK            0x08
#define DNPDEFS_FC_FRZ_CLEAR            0x09
#define DNPDEFS_FC_FRZ_CLEAR_NOACK      0x0A
#define DNPDEFS_FC_FRZ_TIME             0x0B
#define DNPDEFS_FC_FRZ_TIME_NOACK       0x0C
#define DNPDEFS_FC_COLD_RESTART         0x0D
#define DNPDEFS_FC_WARM_RESTART         0x0E
#define DNPDEFS_FC_INIT_DATA_TO_DFLTS   0x0F
#define DNPDEFS_FC_INIT_APPLICATION     0x10
#define DNPDEFS_FC_START_APPLICATION    0x11
#define DNPDEFS_FC_STOP_APPLICATION     0x12
#define DNPDEFS_FC_SAVE_CONFIGURATION   0x13
#define DNPDEFS_FC_ENABLE_UNSOL         0x14
#define DNPDEFS_FC_DISABLE_UNSOL        0x15
#define DNPDEFS_FC_ASSIGN_CLASS         0x16
#define DNPDEFS_FC_DELAY_MEASURE        0x17
#define DNPDEFS_FC_RECORD_CURRENT_TIME  0x18
#define DNPDEFS_FC_OPEN_FILE            0x19
#define DNPDEFS_FC_CLOSE_FILE           0x1A
#define DNPDEFS_FC_DELETE_FILE          0x1B
#define DNPDEFS_FC_GET_FILE_INFO        0x1C
#define DNPDEFS_FC_AUTHENTICATE         0x1D
#define DNPDEFS_FC_ABORT                0x1E
#define DNPDEFS_FC_ACTIVATE_CONFIG      0x1F
#define DNPDEFS_FC_AUTH_REQUEST         0x20
#define DNPDEFS_FC_AUTH_REQUEST_NOACK   0x21
#define DNPDEFS_FC_AUTH_SAV6_HDR        0x80
#define DNPDEFS_FC_RESPONSE             0x81
#define DNPDEFS_FC_UNSOLICITED          0x82
#define DNPDEFS_FC_AUTH_RESPONSE        0x83 

/* Internal Indications bit masks */
#define DNPDEFS_IIN_RESTART             0x8000        /* IIN bit 1.7 */
#define DNPDEFS_IIN_TROUBLE             0x4000        /* IIN bit 1.6 */
#define DNPDEFS_IIN_LOCAL               0x2000        /* IIN bit 1.5 */
#define DNPDEFS_IIN_NEED_TIME           0x1000        /* IIN bit 1.4 */
#define DNPDEFS_IIN_CLASS_3             0x0800        /* IIN bit 1.3 */
#define DNPDEFS_IIN_CLASS_2             0x0400        /* IIN bit 1.2 */
#define DNPDEFS_IIN_CLASS_1             0x0200        /* IIN bit 1.1 */
#define DNPDEFS_IIN_ALL_STATIONS        0x0100        /* IIN bit 1.0 */
#define DNPDEFS_IIN_BAD_CONFIG          0x0020        /* IIN bit 2.5 */
#define DNPDEFS_IIN_ALREADY_EXECUTING   0x0010        /* IIN bit 2.4 */
#define DNPDEFS_IIN_BUFFER_OVFL         0x0008        /* IIN bit 2.3 */
#define DNPDEFS_IIN_OUT_OF_RANGE        0x0004        /* called Param Error */
#define DNPDEFS_IIN_PARAM_ERROR         0x0004        /* IIN bit 2.2 */
#define DNPDEFS_IIN_OBJECT_UNKNOWN      0x0002        /* IIN bit 2.1 */
#define DNPDEFS_IIN_BAD_FUNCTION        0x0001        /* IIN bit 2.0 */

#define DNPDEFS_NUM_EVENT_CLASSES 3
#define DNPDEFS_IIN_ALL_CLASSES \
  (DNPDEFS_IIN_CLASS_1 | DNPDEFS_IIN_CLASS_2 | DNPDEFS_IIN_CLASS_3)

#define DNPDEFS_IIN_ERROR_BITS (DNPDEFS_IIN_TROUBLE | DNPDEFS_IIN_LOCAL | DNPDEFS_IIN_BAD_CONFIG \
  | DNPDEFS_IIN_ALREADY_EXECUTING | DNPDEFS_IIN_OUT_OF_RANGE | DNPDEFS_IIN_OBJECT_UNKNOWN | DNPDEFS_IIN_BAD_FUNCTION)

/* IIN bit indexes, used for Object Group 80 V1 write commands */
#define DNPDEFS_IIN_NEEDTIME_INDEX           4
#define DNPDEFS_IIN_RESTART_INDEX            7

/* Application Layer Object Header definitions */

/* Object Header Indices */
/* application object header indices */
#define DNPDEFS_OH_INDEX_GROUP_NUM      0
#define DNPDEFS_OH_INDEX_VARIATION      1
#define DNPDEFS_OH_INDEX_QUALIFIER      2
#define DNPDEFS_OH_INDEX_RANGE_QTY      3
#define DNPDEFS_OH_INDEX_RANGE_START    3
#define DNPDEFS_OH_INDEX_RANGE_STOP8    4
#define DNPDEFS_OH_INDEX_RANGE_STOP16   5
#define DNPDEFS_OH_INDEX_FREE_SIZE      4
#define DNPDEFS_OH_INDEX_DATA_8_QTY     4
#define DNPDEFS_OH_INDEX_DATA_16_QTY    5  
#define DNPDEFS_OH_INDEX_DATA_8_SS      5
#define DNPDEFS_OH_INDEX_DATA_FREE_QTY  6
#define DNPDEFS_OH_INDEX_DATA_16_SS     7

/* object group numbers */
typedef TMWTYPES_UCHAR DNPDEFS_OBJ_GROUP_ID;
#define DNPDEFS_OBJ_0_DEVICE_ATTRIBUTES  0
#define DNPDEFS_OBJ_1_BIN_INPUTS         1
#define DNPDEFS_OBJ_2_BIN_CHNG_EVENTS    2
#define DNPDEFS_OBJ_3_DBL_INPUTS         3
#define DNPDEFS_OBJ_4_DBL_CHNG_EVENTS    4
#define DNPDEFS_OBJ_10_BIN_OUT_STATUSES 10
#define DNPDEFS_OBJ_10_BIN_OUTS         10
#define DNPDEFS_OBJ_11_BIN_OUT_EVENTS   11
#define DNPDEFS_OBJ_12_BIN_OUT_CTRLS    12
#define DNPDEFS_OBJ_13_BIN_CMD_EVENTS   13
#define DNPDEFS_OBJ_20_RUNNING_CNTRS    20
#define DNPDEFS_OBJ_21_FROZEN_CNTRS     21
#define DNPDEFS_OBJ_22_CNTR_EVENTS      22
#define DNPDEFS_OBJ_23_FCTR_EVENTS      23
#define DNPDEFS_OBJ_30_ANA_INPUTS       30
#define DNPDEFS_OBJ_31_FRZN_ANA_INPUTS  31
#define DNPDEFS_OBJ_32_ANA_CHNG_EVENTS  32
#define DNPDEFS_OBJ_33_FRZN_ANA_EVENTS  33
#define DNPDEFS_OBJ_34_ANA_INPUT_DBANDS 34
#define DNPDEFS_OBJ_40_ANA_OUT_STATUSES 40
#define DNPDEFS_OBJ_41_ANA_OUT_CTRLS    41
#define DNPDEFS_OBJ_42_ANA_OUT_EVENTS   42
#define DNPDEFS_OBJ_43_ANA_CMD_EVENTS   43
#define DNPDEFS_OBJ_50_TIME_AND_DATE    50
#define DNPDEFS_OBJ_51_TIME_DATE_CTO    51
#define DNPDEFS_OBJ_52_TIME_DELAY       52
#define DNPDEFS_OBJ_60_CLASS_SCANS      60
#define DNPDEFS_OBJ_70_FILE_IDENTIFIER  70
#define DNPDEFS_OBJ_70_FILE_EVENTS      70
#define DNPDEFS_OBJ_80_IIN_BITS         80
#define DNPDEFS_OBJ_85_DATASET_PROTO    85
#define DNPDEFS_OBJ_86_DATASET_DESCR    86
#define DNPDEFS_OBJ_87_DATASET_VALUE    87
#define DNPDEFS_OBJ_88_DATASET_EVENTS   88
#define DNPDEFS_OBJ_91_ACTIVATE_STATUS  91
#define DNPDEFS_OBJ_110_STRING_DATA    110
#define DNPDEFS_OBJ_111_STRING_EVENTS  111
#define DNPDEFS_OBJ_112_VTERM_OUTPUT   112
#define DNPDEFS_OBJ_113_VTERM_EVENTS   113
#define DNPDEFS_OBJ_114_EXT_STR_DATA   114
#define DNPDEFS_OBJ_115_EXT_STR_EVENTS 115
#define DNPDEFS_OBJ_120_AUTHENTICATION 120
#define DNPDEFS_OBJ_121_AUTHSECSTATS   121
#define DNPDEFS_OBJ_122_AUTHSTATEVENTS 122

#define DNPDEFS_OBJ_STR_0_DEVICE_ATTRS      "Device Attributes"
#define DNPDEFS_OBJ_STR_1_BIN_INPUTS        "Binary Inputs"
#define DNPDEFS_OBJ_STR_2_BIN_CHNG_EVENTS   "Binary Change Events"
#define DNPDEFS_OBJ_STR_3_DBL_INPUTS        "Double Inputs"
#define DNPDEFS_OBJ_STR_4_DBL_CHNG_EVENTS   "Double Change Events"
#define DNPDEFS_OBJ_STR_10_BIN_OUT_STATUSES "Binary Output Statuses"
#define DNPDEFS_OBJ_STR_11_BIN_OUT_EVENTS   "Binary Output Events"
#define DNPDEFS_OBJ_STR_12_BIN_OUT_CTRLS    "Binary Output Controls"
#define DNPDEFS_OBJ_STR_13_BIN_CMD_EVENTS   "Binary Output Command Events"
#define DNPDEFS_OBJ_STR_20_RUNNING_CNTRS    "Running Counters"
#define DNPDEFS_OBJ_STR_21_FROZEN_CNTRS     "Frozen Counters"
#define DNPDEFS_OBJ_STR_22_CNTR_EVENTS      "Counter Events"
#define DNPDEFS_OBJ_STR_23_FCTR_EVENTS      "Frozen Counter Events"
#define DNPDEFS_OBJ_STR_30_ANA_INPUTS       "Analog Inputs"
#define DNPDEFS_OBJ_STR_31_FRZN_ANA_INPUTS  "Frozen Analog Inputs"
#define DNPDEFS_OBJ_STR_32_ANA_CHNG_EVENTS  "Analog Change Events"
#define DNPDEFS_OBJ_STR_33_FRZN_ANA_EVENTS  "Frozen Analog Events"
#define DNPDEFS_OBJ_STR_34_ANA_INPUT_DBANDS "Analog Input Dead Bands"
#define DNPDEFS_OBJ_STR_40_ANA_OUT_STATUSES "Analog Output Statuses"
#define DNPDEFS_OBJ_STR_41_ANA_OUT_CTRLS    "Analog Output Controls"
#define DNPDEFS_OBJ_STR_42_ANA_OUT_EVENTS   "Analog Output Events"
#define DNPDEFS_OBJ_STR_43_ANA_CMD_EVENTS   "Analog Output Command Events"
#define DNPDEFS_OBJ_STR_50_TIME_AND_DATE    "Time and Date"
#define DNPDEFS_OBJ_STR_51_TIME_DATE_CTO    "Time Date CTO"
#define DNPDEFS_OBJ_STR_52_TIME_DELAY       "Time Delay"
#define DNPDEFS_OBJ_STR_60_CLASS_SCANS      "Class Scans"
#define DNPDEFS_OBJ_STR_70_FILE_IDENTIFIER  "File Identifier"
#define DNPDEFS_OBJ_STR_70_FILE_EVENTS      "File Events"
#define DNPDEFS_OBJ_STR_80_IIN_BITS         "IIN Bits"
#define DNPDEFS_OBJ_STR_85_DATASET_PROTO    "Data Set Prototypes"
#define DNPDEFS_OBJ_STR_86_DATASET_DESCR    "Data Set Descriptors"
#define DNPDEFS_OBJ_STR_87_DATASET_VALUE    "Data Set Present Values"
#define DNPDEFS_OBJ_STR_88_DATASET_EVENTS   "Data Set Events"
#define DNPDEFS_OBJ_STR_91_ACTIVATE_STATUS  "Activate Config Status"
#define DNPDEFS_OBJ_STR_110_STRING_DATA     "String Data"
#define DNPDEFS_OBJ_STR_111_STRING_EVENTS   "String Events"
#define DNPDEFS_OBJ_STR_112_VTERM_OUTPUT    "Virtual Terminal Output"
#define DNPDEFS_OBJ_STR_113_VTERM_EVENTS    "Virtual Terminal Events"
#define DNPDEFS_OBJ_STR_114_EXT_STR_DATA    "Extended String Data"
#define DNPDEFS_OBJ_STR_115_EXT_STR_EVENTS  "Extended String Events"
#define DNPDEFS_OBJ_STR_120_AUTHENTICATION  "Authentication"
#define DNPDEFS_OBJ_STR_121_AUTHSECSTATS    "Authentication Security Statistics"
#define DNPDEFS_OBJ_STR_122_AUTHSTATEVENTS  "Authentication Security Statistics Events" 

/* object header qualifier values */
#define DNPDEFS_QUAL_8BIT_START_STOP        0x00
#define DNPDEFS_QUAL_16BIT_START_STOP       0x01
#define DNPDEFS_QUAL_ALL_POINTS             0x06
#define DNPDEFS_QUAL_8BIT_LIMITED_QTY       0x07
#define DNPDEFS_QUAL_16BIT_LIMITED_QTY      0x08
#define DNPDEFS_QUAL_8BIT_INDEX             0x17
#define DNPDEFS_QUAL_16BIT_INDEX_8BITQ      0x27
#define DNPDEFS_QUAL_16BIT_INDEX            0x28
#define DNPDEFS_QUAL_8BIT_FREE_FORMAT       0x1B
#define DNPDEFS_QUAL_16BIT_FREE_FORMAT      0x5B

/* status values for object variations that include status */
typedef TMWTYPES_UCHAR DNPDEFS_DBAS_FLAG;
#define DNPDEFS_DBAS_FLAG_OFF_LINE      0x00
#define DNPDEFS_DBAS_FLAG_ON_LINE       0x01
#define DNPDEFS_DBAS_FLAG_RESTART       0x02
#define DNPDEFS_DBAS_FLAG_COMM_LOST     0x04
#define DNPDEFS_DBAS_FLAG_REMOTE_FORCED 0x08
#define DNPDEFS_DBAS_FLAG_LOCAL_FORCED  0x10
#define DNPDEFS_DBAS_FLAG_CHATTER       0x20
#define DNPDEFS_DBAS_FLAG_CNTR_ROLLOVER 0x20
#define DNPDEFS_DBAS_FLAG_STAT_ROLLOVER 0x20
#define DNPDEFS_DBAS_FLAG_OVER_RANGE    0x20
#define DNPDEFS_DBAS_FLAG_OVERFLOW      0x20
#define DNPDEFS_DBAS_FLAG_REFERENCE_CHK 0x40
#define DNPDEFS_DBAS_FLAG_DISCONTINUITY 0x40
#define DNPDEFS_DBAS_FLAG_BINARY_ON     0x80
#define DNPDEFS_DBAS_FLAG_BINARY_OFF    0x00

#define DNPDEFS_DBAS_FLAG_DOUBLE_INTER  0x00
#define DNPDEFS_DBAS_FLAG_DOUBLE_OFF    0x40
#define DNPDEFS_DBAS_FLAG_DOUBLE_ON     0x80
#define DNPDEFS_DBAS_FLAG_DOUBLE_INDET  0xC0

/* Size, in bytes, of largest point.  Used to check for end of buffer      */
/* before each point instead of before each byte.  This number consists of */
/* the largest object size (15) and the largest object header size (11).   */
#define DNPDEFS_BIGGEST_POINT           26   

/* Control Relay Output Block (CROB) (object 12) definitions */

/* indices into CROB structure */
#define DNPDEFS_CROB_INDEX_CONTROL      0x00  /* index for Control Code  */
#define DNPDEFS_CROB_INDEX_COUNT        0x01  /* index for count         */
#define DNPDEFS_CROB_INDEX_ON_TIME      0x02  /* index for on time       */
#define DNPDEFS_CROB_INDEX_OFF_TIME     0x06  /* index for off time      */
#define DNPDEFS_CROB_INDEX_STATUS       0x0A  /* index for status field  */

/* values for control code of CROB */
typedef TMWTYPES_UCHAR DNPDEFS_CROB_CTRL;
#define DNPDEFS_CROB_CTRL_NUL           0x00
#define DNPDEFS_CROB_CTRL_PULSE_ON      0x01
#define DNPDEFS_CROB_CTRL_PULSE_OFF     0x02
#define DNPDEFS_CROB_CTRL_LATCH_ON      0x03
#define DNPDEFS_CROB_CTRL_LATCH_OFF     0x04
#define DNPDEFS_CROB_CTRL_MASK          0x0F  /* a mask */
#define DNPDEFS_CROB_CTRL_QUEUE         0x10  
#define DNPDEFS_CROB_CTRL_CLEAR         0x20 
#define DNPDEFS_CROB_CTRL_PAIRED_CLOSE  0x40  
#define DNPDEFS_CROB_CTRL_PAIRED_TRIP   0x80  
#define DNPDEFS_CROB_CTRL_PAIRED_OP     0xC0  /* a mask */

/* status values returned in CROB responses. 
 * These are the same as the DNPDEFS_CTLSTAT_XXX defines,
 * but are kept for backward compatibility.
 */
typedef TMWTYPES_UCHAR DNPDEFS_CROB_ST;
#define DNPDEFS_CROB_ST_SUCCESS            0 
#define DNPDEFS_CROB_ST_TIMEOUT            1
#define DNPDEFS_CROB_ST_NO_SELECT          2
#define DNPDEFS_CROB_ST_FORMAT_ERROR       3
#define DNPDEFS_CROB_ST_NOT_SUPPORTED      4
#define DNPDEFS_CROB_ST_ALREADY_ACTIVE     5
#define DNPDEFS_CROB_ST_HARDWARE_ERROR     6
#define DNPDEFS_CROB_ST_LOCAL              7
#define DNPDEFS_CROB_ST_TOO_MANY_OPS       8
#define DNPDEFS_CROB_ST_NOT_AUTHORIZED     9
#define DNPDEFS_CROB_ST_AUTO_INHIBIT      10
#define DNPDEFS_CROB_ST_PROC_LIMITED      11
#define DNPDEFS_CROB_ST_OUT_OF_RANGE      12
#define DNPDEFS_CROB_ST_DOWNSTRM_LOCAL    13
#define DNPDEFS_CROB_ST_ALR_COMPLETE      14
#define DNPDEFS_CROB_ST_BLOCKED           15
#define DNPDEFS_CROB_ST_CANCELLED         16
#define DNPDEFS_CROB_ST_BLOCKED_OM        17
#define DNPDEFS_CROB_ST_DOWNSTRM_FAIL     18 
#define DNPDEFS_CROB_ST_NON_PARTICIP     126 /* old name */
#define DNPDEFS_CROB_ST_RESERVED         126 /* renamed by TB2014-002 */
#define DNPDEFS_CROB_ST_UNDEFINED        127

/* Commanded state returned in Object Group 13 Binary Output command event */
#define DNPDEFS_CROB_COMMANDED_STATE_OFF 0x00
#define DNPDEFS_CROB_COMMANDED_STATE_ON  0x80

/* status values returned in CROB,   
 * analog output and output command responses. 
 */ 
typedef TMWTYPES_UCHAR DNPDEFS_CTLSTAT;
#define DNPDEFS_CTLSTAT_SUCCESS            0 
#define DNPDEFS_CTLSTAT_TIMEOUT            1
#define DNPDEFS_CTLSTAT_NO_SELECT          2
#define DNPDEFS_CTLSTAT_FORMAT_ERROR       3
#define DNPDEFS_CTLSTAT_NOT_SUPPORTED      4
#define DNPDEFS_CTLSTAT_ALREADY_ACTIVE     5
#define DNPDEFS_CTLSTAT_HARDWARE_ERROR     6
#define DNPDEFS_CTLSTAT_LOCAL              7
#define DNPDEFS_CTLSTAT_TOO_MANY_OPS       8
#define DNPDEFS_CTLSTAT_NOT_AUTHORIZED     9
#define DNPDEFS_CTLSTAT_AUTO_INHIBIT      10
#define DNPDEFS_CTLSTAT_PROC_LIMITED      11
#define DNPDEFS_CTLSTAT_OUT_OF_RANGE      12
#define DNPDEFS_CTLSTAT_DOWNSTRM_LOCAL    13
#define DNPDEFS_CTLSTAT_ALR_COMPLETE      14
#define DNPDEFS_CTLSTAT_BLOCKED           15
#define DNPDEFS_CTLSTAT_CANCELLED         16
#define DNPDEFS_CTLSTAT_BLOCKED_OM        17
#define DNPDEFS_CTLSTAT_DOWNSTRM_FAIL     18 
#define DNPDEFS_CTLSTAT_NON_PARTICIP     126 /* old name */
#define DNPDEFS_CTLSTAT_RESERVED         126 /* renamed by TB2014-002 */
#define DNPDEFS_CTLSTAT_UNDEFINED        127

/* The following definitions are not used by the SCL. They are currently 
 * only used by the SCADA Data Gateway.
 */
typedef TMWTYPES_UCHAR DNPDEFS_CTRL_MASK;
#define DNPDEFS_CTRL_MASK_NONE         0x00 /* No control operations are   */
                                            /* allowed for a point         */
#define DNPDEFS_CTRL_MASK_NO_SELECT    0x01 /* Allows single-pass control  */
                                            /* operations (allows          */
                                            /* operations that were not    */
                                            /* previously selected as part */
                                            /* of a SBE or 2-pass          */
                                            /* operation)                  */
#define DNPDEFS_CTRL_MASK_USE_DEFAULT  0x02 /* Allows no specification of  */
                                            /* operation.  This allows     */
                                            /* pre-defined, fixed, or      */
                                            /* default operations to take  */
                                            /* place.  It can be used with */
                                            /* binary controls (OBJ12)     */
#define DNPDEFS_CTRL_MASK_PULSE        0x04 /* Allows pulse for            */
                                            /* binary controls (OBJ12)     */
#define DNPDEFS_CTRL_MASK_LATCH        0x08 /* Allows latch for            */
                                            /* binary controls (OBJ12)     */
#define DNPDEFS_CTRL_MASK_PAIRED_CLOSE 0x10 /* Allows paired close   */
                                            /* for binary controls (OBJ12) */
#define DNPDEFS_CTRL_MASK_PAIRED_TRIP  0x20 /* Allows paired trip command */
                                            /* qualifier                   */
#define DNPDEFS_CTRL_MASK_NON_STANDARD 0x40 /* Allows non-standard command */
                                            /* qualifier                   */

/* File-ID (object 70) definitions */
typedef TMWTYPES_USHORT DNPDEFS_FILE_TYPE;
#define DNPDEFS_FILE_TYPE_DIRECTORY 0x00
#define DNPDEFS_FILE_TYPE_SIMPLE    0x01

typedef TMWTYPES_USHORT DNPDEFS_FILE_MODE;
#define DNPDEFS_FILE_MODE_NULL    0x00
#define DNPDEFS_FILE_MODE_READ    0x01
#define DNPDEFS_FILE_MODE_WRITE   0x02
#define DNPDEFS_FILE_MODE_APPEND  0x03

typedef TMWTYPES_USHORT DNPDEFS_FILE_PERMISSIONS;
#define DNPDEFS_WORLD_EXECUTE_ALLOWED     0x0001
#define DNPDEFS_WORLD_WRITE_ALLOWED       0x0002
#define DNPDEFS_WORLD_READ_ALLOWED        0x0004
#define DNPDEFS_GROUP_EXECUTE_ALLOWED     0x0008
#define DNPDEFS_GROUP_WRITE_ALLOWED       0x0010
#define DNPDEFS_GROUP_READ_ALLOWED        0x0020
#define DNPDEFS_OWNER_EXECUTE_ALLOWED     0x0040
#define DNPDEFS_OWNER_WRITE_ALLOWED       0x0080
#define DNPDEFS_OWNER_READ_ALLOWED        0x0100

typedef TMWTYPES_UCHAR DNPDEFS_FILE_CMD_STAT;
#define DNPDEFS_FILE_CMD_STAT_SUCCESS     0x00
#define DNPDEFS_FILE_CMD_STAT_DENIED      0x01
#define DNPDEFS_FILE_CMD_STAT_INV_MODE    0x02
#define DNPDEFS_FILE_CMD_STAT_NOT_FOUND   0x03
#define DNPDEFS_FILE_CMD_STAT_LOCKED      0x04
#define DNPDEFS_FILE_CMD_STAT_TOO_MANY    0x05
#define DNPDEFS_FILE_CMD_STAT_INV_HANDLE  0x06
#define DNPDEFS_FILE_CMD_STAT_INV_BLOCK   0x07
#define DNPDEFS_FILE_CMD_STAT_LOST_COMM   0x08
#define DNPDEFS_FILE_CMD_STAT_CANT_ABORT  0x09
#define DNPDEFS_FILE_CMD_STAT_MISC        0xff

/* The following is not a standard status code defined 
 * in the DNP3 documents. It is used by sdnpdata_xxx() 
 * to indicate to the SCL that it should send a NULL
 * response and when this command completes the SCL will 
 * send the response as an event
 */
#define DNPDEFS_FILE_CMD_STAT_ASYNC       0xfe

typedef TMWTYPES_UCHAR DNPDEFS_FILE_TFER_STAT;
#define DNPDEFS_FILE_TFER_STAT_SUCCESS    0x00
#define DNPDEFS_FILE_TFER_STAT_INV_HANDLE 0x06
#define DNPDEFS_FILE_TFER_STAT_LOST_COMM  0x08
#define DNPDEFS_FILE_TFER_STAT_NOT_OPEN   0x10
#define DNPDEFS_FILE_TFER_STAT_HANDLE_EXP 0x11
#define DNPDEFS_FILE_TFER_STAT_OVERRUN    0x12
#define DNPDEFS_FILE_TFER_STAT_BAD_FILE   0x13
#define DNPDEFS_FILE_TFER_STAT_BAD_BLOCK  0x14
#define DNPDEFS_FILE_TFER_STAT_MISC       0xff

/* The following is not a standard status code defined 
 * in the DNP3 documents. It is used by sdnpdata_xxx() 
 * to indicate to the SCL that it should send a NULL
 * response and when this command completes the SCL will 
 * send the response as an event
 */
#define DNPDEFS_FILE_TFER_STAT_ASYNC      0xfe

/* Data Set Definitions */

typedef enum {
  DNPDEFS_DATASET_DESCR_UNKNOWN = 0,
  DNPDEFS_DATASET_DESCR_ID = 1,
  DNPDEFS_DATASET_DESCR_UUID,
  DNPDEFS_DATASET_DESCR_NSPC,
  DNPDEFS_DATASET_DESCR_NAME,
  DNPDEFS_DATASET_DESCR_DAEL,
  DNPDEFS_DATASET_DESCR_PTYP,
  DNPDEFS_DATASET_DESCR_CTLV,
  DNPDEFS_DATASET_DESCR_CTLS

} DNPDEFS_DATASET_DESCR_CODE;

typedef enum {
  DNPDEFS_DATASET_TYPE_NONE = 0,
  DNPDEFS_DATASET_TYPE_VSTR,
  DNPDEFS_DATASET_TYPE_UINT,
  DNPDEFS_DATASET_TYPE_INT,
  DNPDEFS_DATASET_TYPE_FLT,
  DNPDEFS_DATASET_TYPE_OSTR,
  DNPDEFS_DATASET_TYPE_BSTR,
  DNPDEFS_DATASET_TYPE_TIME,
  DNPDEFS_DATASET_TYPE_UNCD

} DNPDEFS_DATASET_TYPE_CODE;

/* Data Set descriptor characteristics bits      */ 
#define DNPDEFS_DATASET_CHAR_RD 0x01   /* set if data set is readable        */
#define DNPDEFS_DATASET_CHAR_WR 0x02   /* set if data set is writable        */
#define DNPDEFS_DATASET_CHAR_ST 0x04   /* set if outstation maintains a      */
                                       /*   static data set                  */
#define DNPDEFS_DATASET_CHAR_EV 0x08   /* set if outstation generates a data */
                                       /*   set event                        */
#define DNPDEFS_DATASET_CHAR_DF 0x10   /* set if defined in master, cleared  */
                                       /*   if defined by outstation         */


/* status values returned in dataset control responses  */
typedef TMWTYPES_UCHAR DNPDEFS_DATASET_ST;
#define DNPDEFS_DATASET_ST_SUCCESS            0 
#define DNPDEFS_DATASET_ST_TIMEOUT            1
#define DNPDEFS_DATASET_ST_NO_SELECT          2
#define DNPDEFS_DATASET_ST_FORMAT_ERROR       3
#define DNPDEFS_DATASET_ST_NOT_SUPPORTED      4
#define DNPDEFS_DATASET_ST_ALREADY_ACTIVE     5
#define DNPDEFS_DATASET_ST_HARDWARE_ERROR     6
#define DNPDEFS_DATASET_ST_LOCAL              7
#define DNPDEFS_DATASET_ST_TOO_MANY_OPS       8
#define DNPDEFS_DATASET_ST_NOT_AUTHORIZED     9
#define DNPDEFS_DATASET_ST_AUTO_INHIBIT      10
#define DNPDEFS_DATASET_ST_PROC_LIMITED      11
#define DNPDEFS_DATASET_ST_OUT_OF_RANGE      12
#define DNPDEFS_DATASET_ST_DOWNSTRM_LOCAL    13
#define DNPDEFS_DATASET_ST_ALR_COMPLETE      14
#define DNPDEFS_DATASET_ST_BLOCKED           15
#define DNPDEFS_DATASET_ST_CANCELLED         16
#define DNPDEFS_DATASET_ST_BLOCKED_OM        17
#define DNPDEFS_DATASET_ST_DOWNSTRM_FAIL     18 
#define DNPDEFS_DATASET_ST_RESERVED         126 /* renamed by TB2014-002 */
#define DNPDEFS_DATASET_ST_UNDEFINED        127


/* DNP3 limits an Object Group 110 string to 255 octets, don't modify this */
#define DNPDEFS_MAX_STRING_LENGTH                 255

/* DNP3 Object Group 114 supports strings between 0 and 65535 octets */
#define DNPDEFS_MAX_EXT_STRING_LENGTH             65535

/* Object Group 0 Device Attribute defines */

/* Device Attribute data type codes */
typedef enum { 
  DNPDEFS_ATTRIBUTE_TYPE_VSTR =1,
  DNPDEFS_ATTRIBUTE_TYPE_UINT,
  DNPDEFS_ATTRIBUTE_TYPE_INT,
  DNPDEFS_ATTRIBUTE_TYPE_FLT,
  DNPDEFS_ATTRIBUTE_TYPE_OSTR,
  DNPDEFS_ATTRIBUTE_TYPE_BSTR, 
  DNPDEFS_ATTRIBUTE_TYPE_DNP3TIME,

  /* The following two types are used for formatting a response, but are not 
   * types that can be chosen for an individual attribute 
   */
  DNPDEFS_ATTRIBUTE_TYPE_LIST = 254,   /* List of UINT8 - BSTR8 pairs         */
  DNPDEFS_ATTRIBUTE_TYPE_EXLIST        /* Extended list. Object length is 256 */
                                       /*   plus the value in length octet    */
} DNPDEFS_ATTRIBUTE_DATA_TYPE;

/* Object Group 0 Device Attribute Variation Definitions */
#define DNPDEFS_OBJ0_CONFIG_ID          196
#define DNPDEFS_OBJ0_CONFIG_VERSION     197
#define DNPDEFS_OBJ0_CONFIG_BLD_DATE    198
#define DNPDEFS_OBJ0_CONFIG_CHG_DATE    199
#define DNPDEFS_OBJ0_CONFIG_SIGNATURE   200 /* Renamed in TB2013-0004b, left for backward compatibility */
#define DNPDEFS_OBJ0_CONFIG_DIGEST      200
#define DNPDEFS_OBJ0_CONFIG_SIG_ALG     201 /* Renamed in TB2013-0004b, left for backward compatibility */
#define DNPDEFS_OBJ0_CONFIG_DIG_ALG     201
#define DNPDEFS_OBJ0_MASTER_RID         202
#define DNPDEFS_OBJ0_DEVLOC_ALTITUDE    203
#define DNPDEFS_OBJ0_DEVLOC_LONGITUDE   204
#define DNPDEFS_OBJ0_DEVLOC_LATITUDE    205
#define DNPDEFS_OBJ0_SECOND_OPER_NAME   206
#define DNPDEFS_OBJ0_PRIME_OPER_NAME    207
#define DNPDEFS_OBJ0_SYSTEM_NAME        208
#define DNPDEFS_OBJ0_SEC_AUTH_VERSION   209
#define DNPDEFS_OBJ0_SEC_STATS_QUANT    210
#define DNPDEFS_OBJ0_USER_SPEC_SETS     211
#define DNPDEFS_OBJ0_NUM_MASTER_PROTOS  212
#define DNPDEFS_OBJ0_NUM_OUTSTA_PROTOS  213
#define DNPDEFS_OBJ0_NUM_MASTER_DATASET 214
#define DNPDEFS_OBJ0_NUM_OUTSTA_DATASET 215
#define DNPDEFS_OBJ0_MAX_BIN_OUT_PER    216
#define DNPDEFS_OBJ0_LOCAL_TIME_ACCUR   217
#define DNPDEFS_OBJ0_TIME_ACCURACY_DUR  218
#define DNPDEFS_OBJ0_ANLG_OUT_EV_SUPP   219
#define DNPDEFS_OBJ0_MAX_ANLG_OUT_INDEX 220
#define DNPDEFS_OBJ0_NUM_ANLG_OUT_PNTS  221
#define DNPDEFS_OBJ0_BIN_OUT_EVENT_SUPP 222
#define DNPDEFS_OBJ0_MAX_BIN_OUT_INDEX  223
#define DNPDEFS_OBJ0_NUM_BIN_OUT_PNTS   224  
#define DNPDEFS_OBJ0_FRZN_CNTR_EV_SUPP  225
#define DNPDEFS_OBJ0_FRZN_CNTR_SUPP     226 
#define DNPDEFS_OBJ0_CNTR_EVENT_SUPP    227
#define DNPDEFS_OBJ0_MAX_CNTR_INDEX     228  
#define DNPDEFS_OBJ0_NUM_CNTR_PNTS      229 
#define DNPDEFS_OBJ0_FRZN_ANLG_IN_SUPP  230
#define DNPDEFS_OBJ0_ANLG_IN_EVENT_SUPP 231
#define DNPDEFS_OBJ0_MAX_ANLG_IN_INDEX  232
#define DNPDEFS_OBJ0_NUM_ANLG_IN_PNTS   233
#define DNPDEFS_OBJ0_DBL_IN_EVENTS_SUPP 234
#define DNPDEFS_OBJ0_MAX_DBL_IN_INDEX   235
#define DNPDEFS_OBJ0_NUM_DBL_IN_PNTS    236
#define DNPDEFS_OBJ0_BIN_IN_EVENTS_SUPP 237
#define DNPDEFS_OBJ0_MAX_BIN_IN_INDEX   238
#define DNPDEFS_OBJ0_NUM_BIN_IN_PNTS    239
#define DNPDEFS_OBJ0_MAX_TX_FRAG_SIZE   240
#define DNPDEFS_OBJ0_MAX_RX_FRAG_SIZE   241
#define DNPDEFS_OBJ0_SFTW_VERSION_STR   242
#define DNPDEFS_OBJ0_HDWR_VERSION_STR   243
#define DNPDEFS_OBJ0_OWNER_NAME         244 /* previously called reserved */
#define DNPDEFS_OBJ0_USER_LOCATION      245       
#define DNPDEFS_OBJ0_USER_ID_CODE       246
#define DNPDEFS_OBJ0_USER_NAME          247        
#define DNPDEFS_OBJ0_SERIAL_NUMBER      248    
#define DNPDEFS_OBJ0_SUBSET_LEVEL       249 
#define DNPDEFS_OBJ0_PRODUCT_NAME       250
#define DNPDEFS_OBJ0_DEVICE_CODE        251
#define DNPDEFS_OBJ0_DEVICE_MANU_NAME   252
#define DNPDEFS_OBJ0_ATTRIBUTE_NAMES    253
#define DNPDEFS_OBJ0_ALL_ATTRIBUTES     254
#define DNPDEFS_OBJ0_LIST_ATTRIBUTES    255         

/* Status codes sent from outstation in Object Group 91 Variation 1
 * Status of Requested Operation Activate Configuration (FC31)
 */
#define DNPDEFS_ACTCONFIG_SUCCESS    0
#define DNPDEFS_ACTCONFIG_REQERROR   1
#define DNPDEFS_ACTCONFIG_DATAERROR  2
#define DNPDEFS_ACTCONFIG_ERROR      3
#define DNPDEFS_ACTCONFIG_NOTCHECKED 4

#endif /* DNPDEFS_DEFINED */

