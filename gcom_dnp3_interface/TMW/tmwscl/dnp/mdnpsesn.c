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

/* file: mdnpsesn.h
 * description: Implement a DNP Master session
 */
#include "tmwscl/dnp/dnpdiag.h"
#include "tmwscl/utils/tmwtarg.h"

#include "tmwscl/dnp/dnpdefs.h"
#include "tmwscl/dnp/dnpsesn.h"
#include "tmwscl/dnp/dnpdtime.h"
#include "tmwscl/dnp/dnpstat.h"
#include "tmwscl/dnp/dnpbncfg.h"
#include "tmwscl/dnp/dnpdiag.h"

#include "tmwscl/dnp/mdnpbrm.h"
#include "tmwscl/dnp/mdnpdiag.h"
#include "tmwscl/dnp/mdnpsesn.h"
#include "tmwscl/dnp/mdnpdata.h"
#include "tmwscl/dnp/mdnpmem.h"

#include "tmwscl/dnp/mdnpo000.h"
#include "tmwscl/dnp/mdnpo001.h"
#include "tmwscl/dnp/mdnpo002.h"
#include "tmwscl/dnp/mdnpo003.h"
#include "tmwscl/dnp/mdnpo004.h"
#include "tmwscl/dnp/mdnpo010.h"
#include "tmwscl/dnp/mdnpo011.h"
#include "tmwscl/dnp/mdnpo013.h"
#include "tmwscl/dnp/mdnpo012.h"
#include "tmwscl/dnp/mdnpo020.h"
#include "tmwscl/dnp/mdnpo021.h"
#include "tmwscl/dnp/mdnpo022.h"
#include "tmwscl/dnp/mdnpo023.h"
#include "tmwscl/dnp/mdnpo030.h"
#include "tmwscl/dnp/mdnpo031.h"
#include "tmwscl/dnp/mdnpo032.h"
#include "tmwscl/dnp/mdnpo033.h"
#include "tmwscl/dnp/mdnpo034.h"
#include "tmwscl/dnp/mdnpo040.h"
#include "tmwscl/dnp/mdnpo041.h"
#include "tmwscl/dnp/mdnpo042.h"
#include "tmwscl/dnp/mdnpo043.h"
#include "tmwscl/dnp/mdnpo050.h"
#include "tmwscl/dnp/mdnpo051.h"
#include "tmwscl/dnp/mdnpo052.h"
#include "tmwscl/dnp/mdnpo070.h"
#include "tmwscl/dnp/mdnpo080.h"
#include "tmwscl/dnp/mdnpo085.h"
#include "tmwscl/dnp/mdnpo086.h"
#include "tmwscl/dnp/mdnpo087.h"
#include "tmwscl/dnp/mdnpo088.h"
#include "tmwscl/dnp/mdnpo091.h"
#include "tmwscl/dnp/mdnpo110.h"
#include "tmwscl/dnp/mdnpo111.h"
#include "tmwscl/dnp/mdnpo113.h"
#include "tmwscl/dnp/mdnpo114.h"
#include "tmwscl/dnp/mdnpo115.h"
#include "tmwscl/dnp/mdnpo120.h"
#if DNPCNFG_SUPPORT_AUTHENTICATION
#include "tmwscl/dnp/mdnpauth.h"
#if MDNPCNFG_SUPPORT_SA_VERSION5
#include "tmwscl/dnp/mdnpo121.h"
#include "tmwscl/dnp/mdnpo122.h"
#include "tmwscl/dnp/mdnpsa.h"
#endif
#endif

/* Forward Declarations */

static void TMWDEFS_LOCAL _processResponse(
  TMWSESN *pSession, 
  TMWSESN_RX_DATA *pRxFragment);

static void TMWDEFS_LOCAL _processUnsolicited(
  TMWSESN *pSession, 
  TMWSESN_RX_DATA *pRxFragment); 

/* Define a table that specifies which function codes are supported
 * and the processing routine to be used to process them.
 */
typedef void (*MDNPSESN_SUPPORT_FUNC)(
  TMWSESN *pSession,
  TMWSESN_RX_DATA *pRxFragment);

typedef struct MDNPSessionFuncEntryStruct {
  TMWTYPES_UCHAR funcCode;
  MDNPSESN_SUPPORT_FUNC pFunc;
} MDNPSESN_FUNC_ENTRY;

/* Table which defines the supported function codes and the associated
 *  processing functions.
 */
static const MDNPSESN_FUNC_ENTRY mdnpsesn_funcTable[] = {
  {DNPDEFS_FC_RESPONSE,            _processResponse},
  {DNPDEFS_FC_UNSOLICITED,         _processUnsolicited},
  {0,                              TMWDEFS_NULL}
};


/* Table which maps request function code and response group/variation
 *  to an associated processing function.
 */
typedef TMWTYPES_BOOL (*MDNP_READ_RESP_FUNC)(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  DNPUTIL_OBJECT_HEADER *pObjHeader);

typedef struct {
  TMWTYPES_UCHAR reqFC;
  TMWTYPES_UCHAR group;
  TMWTYPES_UCHAR variation;
  TMWTYPES_BOOL allVariations;
  MDNP_READ_RESP_FUNC pReadFunc;
} MDNP_FUNC_ENTRY;

#define MDNP_NOCHECK ((TMWTYPES_UCHAR)-1)

/* request FC                 Grp Var  All Variations Read Func */
static const MDNP_FUNC_ENTRY _mdnpFuncTable[] = {
#if MDNPDATA_SUPPORT_OBJ0
  {DNPDEFS_FC_READ,           0,  0,  TMWDEFS_TRUE,   mdnpo000_readObj0},      /* Device Attributes */
#endif
#if MDNPDATA_SUPPORT_OBJ1_V1
  {DNPDEFS_FC_READ,           1,  1,  TMWDEFS_FALSE,  mdnpo001_readObj1v1},    /* Binary Inputs */
#endif
#if MDNPDATA_SUPPORT_OBJ1_V2
  {DNPDEFS_FC_READ,           1,  2,  TMWDEFS_FALSE,  mdnpo001_readObj1v2},
#endif
#if MDNPDATA_SUPPORT_OBJ2_V1
  {DNPDEFS_FC_READ,           2,  1,  TMWDEFS_FALSE,  mdnpo002_readObj2v1},    /* Binary Input Change Events */
#endif
#if MDNPDATA_SUPPORT_OBJ2_V2
  {DNPDEFS_FC_READ,           2,  2,  TMWDEFS_FALSE,  mdnpo002_readObj2v2},
#endif
#if MDNPDATA_SUPPORT_OBJ2_V3
  {DNPDEFS_FC_READ,           2,  3,  TMWDEFS_FALSE,  mdnpo002_readObj2v3},
#endif
  #if MDNPDATA_SUPPORT_OBJ3_V1
  {DNPDEFS_FC_READ,           3,  1,  TMWDEFS_FALSE,  mdnpo003_readObj3v1},    /* Double Bit Inputs */
#endif
#if MDNPDATA_SUPPORT_OBJ3_V2
  {DNPDEFS_FC_READ,           3,  2,  TMWDEFS_FALSE,  mdnpo003_readObj3v2},
#endif
#if MDNPDATA_SUPPORT_OBJ4_V1
  {DNPDEFS_FC_READ,           4,  1,  TMWDEFS_FALSE,  mdnpo004_readObj4v1},    /* Double Bit Input Change Events */
#endif
#if MDNPDATA_SUPPORT_OBJ4_V2
  {DNPDEFS_FC_READ,           4,  2,  TMWDEFS_FALSE,  mdnpo004_readObj4v2},
#endif
#if MDNPDATA_SUPPORT_OBJ4_V3
  {DNPDEFS_FC_READ,           4,   3, TMWDEFS_FALSE,  mdnpo004_readObj4v3},
#endif
#if MDNPDATA_SUPPORT_OBJ10_V1
  {DNPDEFS_FC_READ,           10,  1, TMWDEFS_FALSE,  mdnpo010_readObj10v1},   /* Binary Outputs */
#endif
#if MDNPDATA_SUPPORT_OBJ10_V2
  {DNPDEFS_FC_READ,           10,  2, TMWDEFS_FALSE,  mdnpo010_readObj10v2},
#endif
#if MDNPDATA_SUPPORT_OBJ11_V1
  {DNPDEFS_FC_READ,           11, 1,  TMWDEFS_FALSE,  mdnpo011_readObj11v1},    /* Binary Output Events */
#endif
#if MDNPDATA_SUPPORT_OBJ11_V2
  {DNPDEFS_FC_READ,           11, 2,  TMWDEFS_FALSE,  mdnpo011_readObj11v2},
#endif
#if MDNPDATA_SUPPORT_OBJ13_V1
  {DNPDEFS_FC_READ,           13, 1,  TMWDEFS_FALSE,  mdnpo013_readObj13v1},    /* Binary Output Command Events */
#endif
#if MDNPDATA_SUPPORT_OBJ13_V2
  {DNPDEFS_FC_READ,           13, 2,  TMWDEFS_FALSE,  mdnpo013_readObj13v2},
#endif
#if MDNPDATA_SUPPORT_OBJ20
  {DNPDEFS_FC_READ,           20,  0,  TMWDEFS_TRUE,   mdnpo020_readObj20},    /* Binary Counters */
#endif
#if MDNPDATA_SUPPORT_OBJ21
  {DNPDEFS_FC_READ,           21,  0,  TMWDEFS_TRUE,   mdnpo021_readObj21},    /* Frozen Counters */
#endif
#if MDNPDATA_SUPPORT_OBJ22
  {DNPDEFS_FC_READ,           22,  0,  TMWDEFS_TRUE,   mdnpo022_readObj22},    /* Binary Counter Change Events */
#endif
#if MDNPDATA_SUPPORT_OBJ23
  {DNPDEFS_FC_READ,           23,  0,  TMWDEFS_TRUE,   mdnpo023_readObj23},    /* Frozen Counter Change Events */
#endif
#if MDNPDATA_SUPPORT_OBJ30
  {DNPDEFS_FC_READ,           30,  0,  TMWDEFS_TRUE,   mdnpo030_readObj30},    /* Analog Inputs */
#endif
#if MDNPDATA_SUPPORT_OBJ31
  {DNPDEFS_FC_READ,           31,  0,  TMWDEFS_TRUE,   mdnpo031_readObj31},    /* Frozen Analog Inputs */
#endif
#if MDNPDATA_SUPPORT_OBJ32
  {DNPDEFS_FC_READ,           32,  0,  TMWDEFS_TRUE,   mdnpo032_readObj32},    /* Analog Change Events */
#endif
#if MDNPDATA_SUPPORT_OBJ33
  {DNPDEFS_FC_READ,           33,  0,  TMWDEFS_TRUE,   mdnpo033_readObj33},    /* Frozen Analog Input Events */
#endif
#if MDNPDATA_SUPPORT_OBJ34_V1
  {DNPDEFS_FC_READ,           34,  1,  TMWDEFS_FALSE,  mdnpo034_readObj34v1},  /* Analog Input Deadbands */
#endif
#if MDNPDATA_SUPPORT_OBJ34_V2
  {DNPDEFS_FC_READ,           34,  2,  TMWDEFS_FALSE,  mdnpo034_readObj34v2},
#endif
#if MDNPDATA_SUPPORT_OBJ34_V3
  {DNPDEFS_FC_READ,           34,  3,  TMWDEFS_FALSE,  mdnpo034_readObj34v3},
#endif
#if MDNPDATA_SUPPORT_OBJ40
  {DNPDEFS_FC_READ,           40,  0,  TMWDEFS_TRUE,   mdnpo040_readObj40},    /* Analog Output Status */
#endif
#if MDNPDATA_SUPPORT_OBJ42
  {DNPDEFS_FC_READ,           42,  0,  TMWDEFS_TRUE,   mdnpo042_readObj42},    /* Analog Output Change Events */
#endif
#if MDNPDATA_SUPPORT_OBJ43
  {DNPDEFS_FC_READ,           43,  0,  TMWDEFS_TRUE,   mdnpo043_readObj43},    /* Analog Output Command Events */
#endif
#if MDNPDATA_SUPPORT_OBJ50
  {DNPDEFS_FC_READ,           50,  1,  TMWDEFS_FALSE,   mdnpo050_readObj50v1},  /* Date and Time */
#if MDNPDATA_SUPPORT_OBJ50_V4
  {DNPDEFS_FC_READ,           50,  4,  TMWDEFS_FALSE,   mdnpo050_readObj50v4},  /* Indexed absolute time */
#endif
#endif
#if MDNPDATA_SUPPORT_OBJ2_V3
  {DNPDEFS_FC_READ,           51,  1,  TMWDEFS_FALSE,   mdnpo051_readObj51v1}, /* Common Time of Occurrence */
  {DNPDEFS_FC_READ,           51,  2,  TMWDEFS_FALSE,   mdnpo051_readObj51v2}, /* Common Time of Occurrence */
#endif
#if MDNPDATA_SUPPORT_OBJ50_V1
  {DNPDEFS_FC_DELAY_MEASURE,  52,  2,  TMWDEFS_FALSE,  mdnpo052_storeObj52v2},
#endif
  {DNPDEFS_FC_COLD_RESTART,   52,  1,  TMWDEFS_FALSE,  mdnpo052_storeObj52v1},
  {DNPDEFS_FC_COLD_RESTART,   52,  2,  TMWDEFS_FALSE,  mdnpo052_storeObj52v2},
  {DNPDEFS_FC_WARM_RESTART,   52,  1,  TMWDEFS_FALSE,  mdnpo052_storeObj52v1},
  {DNPDEFS_FC_WARM_RESTART,   52,  2,  TMWDEFS_FALSE,  mdnpo052_storeObj52v2},
#if MDNPDATA_SUPPORT_OBJ70_V2
  {DNPDEFS_FC_AUTHENTICATE,   70,  2,  TMWDEFS_FALSE,  mdnpo070_RespObj70v2},   /* File Transfer */
#endif
#if MDNPDATA_SUPPORT_OBJ70_V4
  {DNPDEFS_FC_READ,           70,  4,  TMWDEFS_FALSE,  mdnpo070_RespObj70v4},   /* File Transfer */
  {DNPDEFS_FC_OPEN_FILE,      70,  4,  TMWDEFS_FALSE,  mdnpo070_RespObj70v4},   /* File Transfer */
  {DNPDEFS_FC_CLOSE_FILE,     70,  4,  TMWDEFS_FALSE,  mdnpo070_RespObj70v4},   /* File Transfer */
  {DNPDEFS_FC_DELETE_FILE,    70,  4,  TMWDEFS_FALSE,  mdnpo070_RespObj70v4},   /* File Transfer */
  {DNPDEFS_FC_ABORT,          70,  4,  TMWDEFS_FALSE,  mdnpo070_RespObj70v4},   /* File Transfer */
  {DNPDEFS_FC_GET_FILE_INFO,  70,  4,  TMWDEFS_FALSE,  mdnpo070_RespObj70v4},   /* File Transfer */
#endif
#if MDNPDATA_SUPPORT_OBJ70_V5
  {DNPDEFS_FC_READ,           70,  5,  TMWDEFS_FALSE,  mdnpo070_RespObj70v5},   /* File Transfer */
  {DNPDEFS_FC_WRITE,          70,  5,  TMWDEFS_FALSE,  mdnpo070_RespObj70v5},   /* File Transfer */
#endif
#if MDNPDATA_SUPPORT_OBJ70_V6
  {DNPDEFS_FC_READ,           70,  6,  TMWDEFS_FALSE,  mdnpo070_RespObj70v6},   /* File Transfer */
  {DNPDEFS_FC_WRITE,          70,  6,  TMWDEFS_FALSE,  mdnpo070_RespObj70v6},   /* File Transfer */
#endif
#if MDNPDATA_SUPPORT_OBJ70_V7
  {DNPDEFS_FC_GET_FILE_INFO,  70,  7,  TMWDEFS_FALSE,  mdnpo070_RespObj70v7},   /* File Transfer */
  {DNPDEFS_FC_READ,           70,  7,  TMWDEFS_FALSE,  mdnpo070_RespObj70v7},   /* File Transfer */
#endif
#if MDNPDATA_SUPPORT_OBJ80
  {DNPDEFS_FC_READ,           80,  1,  TMWDEFS_FALSE,  mdnpo080_readObj80v1},   /* IIN bits */
#endif
#if MDNPDATA_SUPPORT_OBJ85
  {DNPDEFS_FC_READ,           85,  1,  TMWDEFS_FALSE,  mdnpo085_readObj85v1},   /* Data Set Prototype */
#endif
#if MDNPDATA_SUPPORT_OBJ86_V1
  {DNPDEFS_FC_READ,           86,  1,  TMWDEFS_FALSE,  mdnpo086_readObj86v1},   /* Data Set Descriptor */
#endif
#if MDNPDATA_SUPPORT_OBJ86_V2
  {DNPDEFS_FC_READ,           86,  2,  TMWDEFS_FALSE,  mdnpo086_readObj86v2},   /* Data Set Descriptor */
#endif
#if MDNPDATA_SUPPORT_OBJ86_V3
  {DNPDEFS_FC_READ,           86,  3,  TMWDEFS_FALSE,  mdnpo086_readObj86v3},   /* Data Set Descriptor */
#endif
#if MDNPDATA_SUPPORT_OBJ87
  {DNPDEFS_FC_READ,           87,  1,  TMWDEFS_FALSE,  mdnpo087_readObj87v1},   /* Data Set Present Value */
#endif
#if MDNPDATA_SUPPORT_OBJ88
  {DNPDEFS_FC_READ,           88,  1,  TMWDEFS_FALSE,  mdnpo088_readObj88v1},   /* Data Set Snapshot */
#endif
#if MDNPDATA_SUPPORT_OBJ91
  {DNPDEFS_FC_ACTIVATE_CONFIG,91,  1,  TMWDEFS_FALSE,  mdnpo091_respObj91v1},   /* Activate Configuration Response */
#endif
#if MDNPDATA_SUPPORT_OBJ110
  {DNPDEFS_FC_READ,           110, 1,  TMWDEFS_TRUE,   mdnpo110_readObj110},    /* String Data */
#endif
#if MDNPDATA_SUPPORT_OBJ111
  {DNPDEFS_FC_READ,           111, 1,  TMWDEFS_TRUE,   mdnpo111_readObj111},    /* String Change Events */
#endif
#if MDNPDATA_SUPPORT_OBJ113
  {DNPDEFS_FC_READ,           113, 1,  TMWDEFS_TRUE,   mdnpo113_readObj113},    /* Virtual Terminal Events */
#endif
#if MDNPDATA_SUPPORT_OBJ114
  {DNPDEFS_FC_READ,           114, 1,  TMWDEFS_TRUE,   mdnpo114_readObj114},    /* Extended String Data */
#endif
#if MDNPDATA_SUPPORT_OBJ115
  {DNPDEFS_FC_READ,           115, 0,  TMWDEFS_TRUE,   mdnpo115_readObj115},    /* Extended String Change Events */
#endif
#if MDNPDATA_SUPPORT_OBJ120
  {DNPDEFS_FC_READ,           120, 1,  TMWDEFS_FALSE,   mdnpo120_preChallenge}, /* Secure Authentication "pre-challenge" */
  {DNPDEFS_FC_READ,           120, 7,  TMWDEFS_FALSE,   mdnpo120_readObj120v7}, /* Secure Authentication Error Events */
#if MDNPCNFG_SUPPORT_SA_VERSION5 
  {DNPDEFS_FC_READ,           121, 1,  TMWDEFS_FALSE,   mdnpo121_readObj121v1}, /* Secure Authentication Statistics */
  {DNPDEFS_FC_READ,           122, 0,  TMWDEFS_TRUE,    mdnpo122_readObj122},   /* Secure Authentication Statistics Events */
#endif
#endif
  {0,                         0,   0,  TMWDEFS_FALSE,  TMWDEFS_NULL}            /* Last entry */
};


/* Table which maps request function code and response group/variation
 *  to an associated processing function.
 */
typedef DNPCHNL_RESP_STATUS (*MDNP_SEL_OPER_RESP_FUNC)(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRxFragment,
  DNPUTIL_OBJECT_HEADER *pObjHeader);

typedef struct {
  TMWTYPES_UCHAR group;
  TMWTYPES_UCHAR variation;
  TMWTYPES_UCHAR dataLength;                /* length of data for checking bad message */
  MDNP_SEL_OPER_RESP_FUNC pResponseFunc;
} MDNP_RESP_FUNC_ENTRY;

/*  Grp Var Response Func */
static const MDNP_RESP_FUNC_ENTRY _mdnpSelOperFuncTable[] = {
#if MDNPDATA_SUPPORT_OBJ12_V1
  {DNPDEFS_OBJ_12_BIN_OUT_CTRLS,  1, 11, mdnpo012_selOperRespObj12},    
#endif
#if MDNPDATA_SUPPORT_OBJ12_V2
  {DNPDEFS_OBJ_12_BIN_OUT_CTRLS,  2, 11, mdnpo012_selOperRespObj12},
#endif
#if MDNPDATA_SUPPORT_OBJ41_V1
  {DNPDEFS_OBJ_41_ANA_OUT_CTRLS,  1, 5,  mdnpo041_selOperRespObj41v1},    
#endif
#if MDNPDATA_SUPPORT_OBJ41_V2
  {DNPDEFS_OBJ_41_ANA_OUT_CTRLS,  2, 3,  mdnpo041_selOperRespObj41v2},
#endif
#if MDNPDATA_SUPPORT_OBJ41_V3
  {DNPDEFS_OBJ_41_ANA_OUT_CTRLS,  3, 5,  mdnpo041_selOperRespObj41v3},    
#endif
#if MDNPDATA_SUPPORT_OBJ41_V4
  {DNPDEFS_OBJ_41_ANA_OUT_CTRLS,  4, 9,  mdnpo041_selOperRespObj41v4},
#endif
#if MDNPDATA_SUPPORT_OBJ87
  {DNPDEFS_OBJ_87_DATASET_VALUE,  1, MDNP_NOCHECK, mdnpo087_selOperRespObj87v1},
#endif
  {0,   0, 0, TMWDEFS_NULL}            /* Last entry */
};


/* Internal functions */

static TMWTYPES_UCHAR TMWDEFS_LOCAL _getExpectedMsgLength(DNPDEFS_OBJ_GROUP_ID objectGroup, TMWTYPES_UCHAR variation)
{
  switch (objectGroup)
  {
 
  /* object 0 device attributes, verification code in mdnpo000_xxx response function */
  case DNPDEFS_OBJ_1_BIN_INPUTS:
  case DNPDEFS_OBJ_3_DBL_INPUTS:
    /* variation 1 is checked in response function */
    if(variation == 1) return MDNP_NOCHECK;
    if(variation == 2) return 1;
    break;

  case DNPDEFS_OBJ_2_BIN_CHNG_EVENTS:
  case DNPDEFS_OBJ_4_DBL_CHNG_EVENTS:
    if(variation == 1) return 1;
    if(variation == 2) return 7;
    if(variation == 3) return 3;
    break;

  case DNPDEFS_OBJ_10_BIN_OUTS:
    /* variation 1 is checked in response function */
    if(variation == 1) return MDNP_NOCHECK;
    if(variation == 2) return 1;
    break;

  case DNPDEFS_OBJ_11_BIN_OUT_EVENTS:
  case DNPDEFS_OBJ_13_BIN_CMD_EVENTS:
    if(variation == 1) return 1;
    if(variation == 2) return 7; 
    break;
 
  case DNPDEFS_OBJ_20_RUNNING_CNTRS:
    if(variation == 1) return 5;
    if(variation == 2) return 3;
    if(variation == 3) return 5;
    if(variation == 4) return 3;
    if(variation == 5) return 4;
    if(variation == 6) return 2;
    if(variation == 7) return 4;
    if(variation == 8) return 2;
    break;

  case DNPDEFS_OBJ_21_FROZEN_CNTRS:
  case DNPDEFS_OBJ_22_CNTR_EVENTS:
  case DNPDEFS_OBJ_23_FCTR_EVENTS:
    if(variation == 1) return 5;
    if(variation == 2) return 3;
    if(variation == 3) return 5;
    if(variation == 4) return 3;
    if(variation == 5) return 11;
    if(variation == 6) return 9;
    if(variation == 7) return 11;
    if(variation == 8) return 9;
    /* The following for object group 21 only, but others will never get here anyway */
    if(variation == 9) return 4;
    if(variation == 10) return 2;
    if(variation == 11) return 4;
    if(variation == 12) return 2;
    break;

  case DNPDEFS_OBJ_30_ANA_INPUTS:
    if(variation == 1) return 5;
    if(variation == 2) return 3;
    if(variation == 3) return 4;
    if(variation == 4) return 2;
    if(variation == 5) return 5;
    if(variation == 6) return 9;
    break;

  case DNPDEFS_OBJ_31_FRZN_ANA_INPUTS:
    if(variation == 1) return 5;
    if(variation == 2) return 3;
    if(variation == 3) return 11;
    if(variation == 4) return 9;
    if(variation == 5) return 4;
    if(variation == 6) return 2;
    if(variation == 7) return 5;
    if(variation == 8) return 9;
    break;

  case DNPDEFS_OBJ_32_ANA_CHNG_EVENTS:
  case DNPDEFS_OBJ_33_FRZN_ANA_EVENTS:
  case DNPDEFS_OBJ_42_ANA_OUT_EVENTS:
  case DNPDEFS_OBJ_43_ANA_CMD_EVENTS:
    if(variation == 1) return 5;
    if(variation == 2) return 3;
    if(variation == 3) return 11;
    if(variation == 4) return 9;
    if(variation == 5) return 5;
    if(variation == 6) return 9;
    if(variation == 7) return 11;
    if(variation == 8) return 15;
    break;

  case DNPDEFS_OBJ_34_ANA_INPUT_DBANDS:
    if(variation == 1) return 2;
    if(variation == 2) return 4;
    if(variation == 3) return 4;
    break;

  case DNPDEFS_OBJ_40_ANA_OUT_STATUSES:
    if(variation == 1) return 5;
    if(variation == 2) return 3;
    if(variation == 3) return 5;
    if(variation == 4) return 9;
    break;
 
  case DNPDEFS_OBJ_50_TIME_AND_DATE:
    if(variation == 1) return 6;
    if(variation == 4) return 11;
    break;

  case DNPDEFS_OBJ_51_TIME_DATE_CTO:
    if(variation == 1) return 6;
    if(variation == 2) return 6; 
    break;
    
  case DNPDEFS_OBJ_52_TIME_DELAY:
    if(variation == 1) return 2;
    if(variation == 2) return 2; 
    break;

  /* object 70 file transfer, verification code in mdnpo070_xxx  */
  /* object 80, verification code in mdnpo080_readObj80v1 */
  /* objects 85-88 data sets, verification code in mdnpxxx  */ 
  /* object 91, verification code in mdnpo091_respObj91v1  */ 
  /* object 110, verification code in mdnpo110_readObj110  */ 
  /* object 111, verification code in mdnpo111_readObj111  */ 
  /* object 113, verification code in mdnpo113_readObj113  */ 
  /* object 120-122, verification code in response functions  */ 

  default:
    return MDNP_NOCHECK; 
  }
     
  return MDNP_NOCHECK; 
}

/*  validate message size based on header info */
static TMWTYPES_BOOL TMWDEFS_LOCAL _validateMessageSize(
  TMWTYPES_UCHAR objectSize, 
  DNPUTIL_OBJECT_HEADER *pHeader, 
  TMWTYPES_USHORT msgLength)
{
  TMWTYPES_ULONG length;
  int pointIndexLength = 0; 

  if(objectSize == MDNP_NOCHECK)
    return TMWDEFS_TRUE;

  if(pHeader->qualifier == DNPDEFS_QUAL_8BIT_INDEX)
  {
    pointIndexLength = 1;
  }
  else if((pHeader->qualifier == DNPDEFS_QUAL_16BIT_INDEX)
    || (pHeader->qualifier == DNPDEFS_QUAL_16BIT_INDEX_8BITQ))
  {
    pointIndexLength = 2;
  }

  length = pHeader->numberOfPoints *(objectSize + pointIndexLength);

  if(length > msgLength)
  { 
    return TMWDEFS_FALSE;
  }
  else 
    return TMWDEFS_TRUE;
} 

/* function: _autoRequestAlreadyInQueue 
 * purpose: Check to see if there is already an automatically
 *  generated request of this type in the queue.
 * arguments:
 *  pSession - session to check request for
 * returns:
 *  TMWDEFS_TRUE if the same request is in the queue
 *  TMWDEFS_FALSE if not
 */
static TMWTYPES_BOOL TMWDEFS_LOCAL _autoRequestAlreadyInQueue(
  TMWSESN *pSession,
  void *sentinel)
{
  TMWCHNL *pChannel = pSession->pChannel;
  DNPCHNL_TX_DATA *pRequest = TMWDEFS_NULL;

  while((pRequest = (DNPCHNL_TX_DATA *)tmwdlist_getAfter(
    &pChannel->messageQueue, (TMWDLIST_MEMBER *)pRequest)) != TMWDEFS_NULL)
  {
    /* If callback param was set to sentinel value and request is
     * for this session it means this is an auto integrity poll
     */
    if((pRequest->tmw.pSession == pSession)
      && (pRequest->pUserCallbackParam == sentinel))
    {
      return(TMWDEFS_TRUE);
    }
  }
  return(TMWDEFS_FALSE);
}


/* forward reference */
static void TMWDEFS_LOCAL _autoDisableUnsolCallback(
  void *pCallbackParam,
  DNPCHNL_RESPONSE_INFO *pResponse);

static void TMWDEFS_LOCAL _autoDisableUnsolReq(TMWSESN *pSession)
{ 
  if(!_autoRequestAlreadyInQueue(pSession, _autoDisableUnsolReq))
  {
    MDNPBRM_REQ_DESC reqDesc;
    mdnpbrm_initReqDesc(&reqDesc, pSession);
    reqDesc.pUserCallback = _autoDisableUnsolCallback;
    reqDesc.pUserCallbackParam = _autoDisableUnsolReq; /* Sentinel value */
    reqDesc.priority = MDNPBRM_DISABLE_UNSOL_PRIORITY;

#if TMWCNFG_SUPPORT_DIAG
    reqDesc.pMsgDescription = "Disable Unsolicited Due to Master Startup";
#endif
#if MDNPDATA_SUPPORT_OBJ120
    {
      MDNPSESN *pMDNPSession = (MDNPSESN *)pSession;
      if (pMDNPSession->autoRequestAggrModeMask & MDNPSESN_AUTO_UNSOL_STARTUP)
      {
        reqDesc.authAggressiveMode = TMWDEFS_TRUE;
      }
    }
#endif
    mdnpbrm_unsolDisable(&reqDesc, TMWDEFS_TRUE, TMWDEFS_TRUE, TMWDEFS_TRUE);
  }
}

/* function: _unsolCallback */
static void TMWDEFS_LOCAL _autoDisableUnsolCallback(
  void *pCallbackParam,
  DNPCHNL_RESPONSE_INFO *pResponse)
{ 
  TMWTARG_UNUSED_PARAM(pCallbackParam);
  if(pResponse->status == DNPCHNL_RESP_STATUS_TIMEOUT)
  {  
    _autoDisableUnsolReq(pResponse->pSession);
  }
}

/* forward reference */
static void TMWDEFS_LOCAL _autoEnableUnsolCallback(
  void *pCallbackParam,
  DNPCHNL_RESPONSE_INFO *pResponse);

static void TMWDEFS_LOCAL _autoEnableUnsolReq(TMWSESN *pSession, char *msgPtr)
{ 
#if !TMWCNFG_SUPPORT_DIAG
  TMWTARG_UNUSED_PARAM(msgPtr);
#endif
  if(!_autoRequestAlreadyInQueue(pSession, _autoEnableUnsolReq))
  {
    MDNPBRM_REQ_DESC reqDesc;
    MDNPSESN *pMDNPSession = (MDNPSESN *)pSession;

    mdnpbrm_initReqDesc(&reqDesc, pSession);
    reqDesc.pUserCallback = _autoEnableUnsolCallback;
    reqDesc.pUserCallbackParam = _autoEnableUnsolReq; /* Sentinel value */
    reqDesc.priority = MDNPBRM_ENABLE_UNSOL_PRIORITY;
#if TMWCNFG_SUPPORT_DIAG
     reqDesc.pMsgDescription = (TMWTYPES_CHAR*)msgPtr;
#endif 
#if MDNPDATA_SUPPORT_OBJ120
     if (pMDNPSession->autoRequestAggrModeMask & MDNPSESN_AUTO_ENABLE_UNSOL)
     {
       reqDesc.authAggressiveMode = TMWDEFS_TRUE;
     }
#endif
    mdnpbrm_unsolEnable(&reqDesc, 
      (TMWTYPES_BOOL)((pMDNPSession->autoRequestBits & MDNPSESN_CLASS1_AUTO_ENABLE) != 0 ? TMWDEFS_TRUE : TMWDEFS_FALSE), 
      (TMWTYPES_BOOL)((pMDNPSession->autoRequestBits & MDNPSESN_CLASS2_AUTO_ENABLE) != 0 ? TMWDEFS_TRUE : TMWDEFS_FALSE), 
      (TMWTYPES_BOOL)((pMDNPSession->autoRequestBits & MDNPSESN_CLASS3_AUTO_ENABLE) != 0 ? TMWDEFS_TRUE : TMWDEFS_FALSE)); 
  }
}

/* function: _unsolCallback */
static void TMWDEFS_LOCAL _autoEnableUnsolCallback(
  void *pCallbackParam,
  DNPCHNL_RESPONSE_INFO *pResponse)
{ 
  TMWTARG_UNUSED_PARAM(pCallbackParam);

  if(pResponse->status == DNPCHNL_RESP_STATUS_TIMEOUT)
  {
    _autoEnableUnsolReq(pResponse->pSession, (char *)pResponse->pTxData->pMsgDescription);
    return;
  }
}
 
static void TMWDEFS_LOCAL _autoIntegrityReq(TMWSESN *pSession, char *msgPtr)
{ 
#if !TMWCNFG_SUPPORT_DIAG
  TMWTARG_UNUSED_PARAM(msgPtr);
#endif
  if(!_autoRequestAlreadyInQueue(pSession, _autoIntegrityReq))
  {
    MDNPBRM_REQ_DESC reqDesc;
    mdnpbrm_initReqDesc(&reqDesc, pSession);
    reqDesc.pUserCallback = mdnpsesn_autoIntegrityCallback;
    reqDesc.pUserCallbackParam = _autoIntegrityReq; /* Sentinel value */
    reqDesc.priority = MDNPBRM_INTEGRITY_PRIORITY;
   
#if TMWCNFG_SUPPORT_DIAG
    reqDesc.pMsgDescription = (TMWTYPES_CHAR*)msgPtr;
#endif 
    mdnpbrm_readClass(&reqDesc, TMWDEFS_NULL, DNPDEFS_QUAL_ALL_POINTS, 0,
      TMWDEFS_TRUE, TMWDEFS_TRUE, TMWDEFS_TRUE, TMWDEFS_TRUE);
  }
}

/* function: mdnpsesn_autoIntegrityCallback */
void TMWDEFS_GLOBAL mdnpsesn_autoIntegrityCallback(
  void *pCallbackParam,
  DNPCHNL_RESPONSE_INFO *pResponse)
{ 
  TMWTARG_UNUSED_PARAM(pCallbackParam);
  if(pResponse->last) 
  {
    if(pResponse->status == DNPCHNL_RESP_STATUS_TIMEOUT)
    {  
      _autoIntegrityReq(pResponse->pSession, (char *)pResponse->pTxData->pMsgDescription);
    }
    else
    {
      MDNPSESN *pMDNPSession = (MDNPSESN *)pResponse->pTxData->pSession;
      if(pMDNPSession->unsolRespState == MDNPSESN_UNSOL_STARTUP)
        pMDNPSession->unsolRespState = MDNPSESN_UNSOL_FIRSTUR;
    }
  }
}

#if MDNPDATA_SUPPORT_DATASETS
/*forward reference */ 
static void TMWDEFS_LOCAL _autoDatasetXChngCallback(
  void *pCallbackParam,
  DNPCHNL_RESPONSE_INFO *pResponse);

static void TMWDEFS_LOCAL _autoDatasetReq(TMWSESN *pSession, char *msgPtr) 
{
#if !TMWCNFG_SUPPORT_DIAG
  TMWTARG_UNUSED_PARAM(msgPtr);
#endif
  if (!_autoRequestAlreadyInQueue(pSession, _autoDatasetReq))
  {
    /* Build request descriptor */
    MDNPBRM_REQ_DESC reqDesc;
    mdnpbrm_initReqDesc(&reqDesc, pSession);
    reqDesc.responseTimeout = TMWDEFS_MINUTES(10);
    reqDesc.priority = MDNPBRM_DATASET_XCHNG_PRIORITY;

    /* Setup callback to resend this if it fails */
    reqDesc.pUserCallback = _autoDatasetXChngCallback;
    reqDesc.pUserCallbackParam = _autoDatasetReq; /* Sentinel value */
#if TMWCNFG_SUPPORT_DIAG
    reqDesc.pMsgDescription = (TMWTYPES_CHAR*)msgPtr;
#endif
    mdnpbrm_datasetExchange(&reqDesc);
  }
  return;
}

/* function: _autoDatasetXChngCallback */
static void TMWDEFS_LOCAL _autoDatasetXChngCallback(
  void *pCallbackParam,
  DNPCHNL_RESPONSE_INFO *pResponse)
{ 
  TMWTARG_UNUSED_PARAM(pCallbackParam);
  if(pResponse->status == DNPCHNL_RESP_STATUS_TIMEOUT)
  {  
    _autoDatasetReq(pResponse->pSession, (char *)pResponse->pTxData->pMsgDescription);
  }
}
#endif

/* function: _idleCallback */
static TMWTYPES_BOOL TMWDEFS_CALLBACK _idleCallback(
  TMWSESN *pSession)
{
#if MDNPDATA_SUPPORT_OBJ120
   if(mdnpauth_notIdle(pSession))
     return(TMWDEFS_FALSE);
#else
  TMWTARG_UNUSED_PARAM(pSession);
#endif
   return(TMWDEFS_TRUE);
}

/* function: _processIIN 
 * purpose: process the received IIN bits
 * arguments:
 *  pSession - session from which this message originated
 *  reqFC - Function code of request that caused this response.
 * returns:
 *  void
 */
static void TMWDEFS_LOCAL _processIIN(
  TMWSESN *pSession,
  TMWTYPES_UCHAR reqFC)
{
  MDNPSESN *pMDNPSession = (MDNPSESN *)pSession;
  TMWTYPES_USHORT iin = pMDNPSession->currentIIN;
  MDNPBRM_REQ_DESC reqDesc;

  /* Diagnostics */
  MDNPDIAG_SHOW_IIN(pSession, iin);

  /* Call user routine to process IIN bits first */
  mdnpdata_processIIN(pSession, &iin);

  /* Build request descriptor */
  mdnpbrm_initReqDesc(&reqDesc, pSession);
  
  /* Suppress error message if request is already queued */
  pMDNPSession->dnp.suppressError = TMWDEFS_TRUE;

  /* Process restart bit */
  if(iin & DNPDEFS_IIN_RESTART) 
  {        
    /* 
     * Sending a disable unsolicited when the outstation restarts is probably never necessary, 
     * but leave this as we allowed this in earlier releases.
     * If the IIN bits are from a response to a DISABLE UNSOLICITED request, 
     * don't send another one now. 
     */
    if((pMDNPSession->autoRequestMask & MDNPSESN_AUTO_DISABLE_UNSOL)
      && (reqFC != DNPDEFS_FC_DISABLE_UNSOL))
    {
      reqDesc.priority = MDNPBRM_DISABLE_UNSOL_PRIORITY;
#if TMWCNFG_SUPPORT_DIAG
      reqDesc.pMsgDescription = "Disable Unsolicited Due to Restart IIN";
#endif
#if MDNPDATA_SUPPORT_OBJ120
      if (pMDNPSession->autoRequestAggrModeMask & MDNPSESN_AUTO_DISABLE_UNSOL)
      {
        reqDesc.authAggressiveMode = TMWDEFS_TRUE;
      }
#endif
      mdnpbrm_unsolDisable(&reqDesc, TMWDEFS_TRUE, TMWDEFS_TRUE, TMWDEFS_TRUE);
    }

    if(pMDNPSession->autoRequestMask & MDNPSESN_AUTO_CLEAR_RESTART)
    {
      reqDesc.priority = MDNPBRM_CLEAR_RESTART_PRIORITY;
#if TMWCNFG_SUPPORT_DIAG
      reqDesc.pMsgDescription = "Clear Restart Due to Restart IIN";
#endif
#if MDNPDATA_SUPPORT_OBJ120
      if (pMDNPSession->autoRequestAggrModeMask & MDNPSESN_AUTO_CLEAR_RESTART)
      {
        reqDesc.authAggressiveMode = TMWDEFS_TRUE;
      }
#endif
      mdnpbrm_clearRestart(&reqDesc);
    }

#if MDNPDATA_SUPPORT_DATASETS
    if(pMDNPSession->autoRequestMask & MDNPSESN_AUTO_DATASET_RESTART)
    {
#if TMWCNFG_SUPPORT_DIAG  
      _autoDatasetReq(pSession, "Read/Write Data Set Prototypes and Descriptors Due to Restart IIN"); 
#else
      _autoDatasetReq(pSession, TMWDEFS_NULL); 
#endif
    }
#endif

    if(pMDNPSession->autoRequestMask & MDNPSESN_AUTO_INTEGRITY_RESTART)
    {
#if TMWCNFG_SUPPORT_DIAG
    _autoIntegrityReq(pSession, "Integrity Poll Due to Restart IIN");
#else
    _autoIntegrityReq(pSession, TMWDEFS_NULL);
#endif
    }

    if(pMDNPSession->autoRequestMask & MDNPSESN_AUTO_ENABLE_UNSOL)
    {
#if TMWCNFG_SUPPORT_DIAG
    _autoEnableUnsolReq(pSession, "Enable Unsolicited Due to Restart IIN");
#else
    _autoEnableUnsolReq(pSession, TMWDEFS_NULL);
#endif 
    }
  }

  /* Process event buffer overflow bit */
  if((iin & DNPDEFS_IIN_BUFFER_OVFL)
    && (pMDNPSession->autoRequestMask & MDNPSESN_AUTO_INTEGRITY_OVERFLOW))
  {
#if TMWCNFG_SUPPORT_DIAG
    _autoIntegrityReq(pSession, "Integrity Poll Due to Buffer Overflow IIN");
#else
    _autoIntegrityReq(pSession, TMWDEFS_NULL);
#endif
  }

  /* Process local mode bit */
  if(!(iin & DNPDEFS_IIN_LOCAL)
    && (pMDNPSession->previousIIN & DNPDEFS_IIN_LOCAL)
    && (pMDNPSession->autoRequestMask & MDNPSESN_AUTO_INTEGRITY_LOCAL))
  {
#if TMWCNFG_SUPPORT_DIAG
    _autoIntegrityReq(pSession, "Integrity Poll Due to Local IIN");
#else
    _autoIntegrityReq(pSession, TMWDEFS_NULL);
#endif
  }

  /* Process need time bit */
#if MDNPDATA_SUPPORT_OBJ50_V1
  if ((iin & DNPDEFS_IIN_NEED_TIME)
    && (pMDNPSession->autoRequestMask & MDNPSESN_AUTO_TIME_SYNC_SERIAL))
  {
    TMWTYPES_BOOL measureDelay = TMWDEFS_FALSE;
#if TMWCNFG_SUPPORT_DIAG
    reqDesc.pMsgDescription = "Time Synchronization Due to Need Time IIN";
#endif
#if MDNPDATA_SUPPORT_OBJ120
    if (pMDNPSession->autoRequestAggrModeMask & MDNPSESN_AUTO_TIME_SYNC_SERIAL)
    {
      reqDesc.authAggressiveMode = TMWDEFS_TRUE;
    }
#endif
    if((pMDNPSession->autoRequestMask & MDNPSESN_AUTO_DELAY_MEAS) != 0)
    {
      measureDelay = TMWDEFS_TRUE;
#if TMWCNFG_SUPPORT_DIAG
      reqDesc.pMsgDescription = "Delay Measurement Due to Need Time IIN";
#endif
    }

    reqDesc.priority = MDNPBRM_TIMESYNC_PRIORITY;
    mdnpbrm_timeSync(&reqDesc, MDNPBRM_SYNC_TYPE_SERIAL, measureDelay);
  }
#endif

#if MDNPDATA_SUPPORT_OBJ50_V3
  if((iin & DNPDEFS_IIN_NEED_TIME)
    && (pMDNPSession->autoRequestMask & MDNPSESN_AUTO_TIME_SYNC_LAN))
  {
#if TMWCNFG_SUPPORT_DIAG
    reqDesc.pMsgDescription = "Time Synchronization Due to Need Time IIN";
#endif
#if MDNPDATA_SUPPORT_OBJ120
    if (pMDNPSession->autoRequestAggrModeMask & MDNPSESN_AUTO_TIME_SYNC_LAN)
    {
      reqDesc.authAggressiveMode = TMWDEFS_TRUE;
    }
#endif
   
    reqDesc.priority = MDNPBRM_TIMESYNC_PRIORITY;
    mdnpbrm_timeSync(&reqDesc, MDNPBRM_SYNC_TYPE_LAN, TMWDEFS_FALSE);
  }
#endif

  /* Process class 1, 2, and 3 ready bit */
  if((iin & DNPDEFS_IIN_ALL_CLASSES)
    && (pMDNPSession->autoRequestMask & MDNPSESN_AUTO_EVENT_POLL))
  { 
#if TMWCNFG_SUPPORT_DIAG
    reqDesc.pMsgDescription = "Event Poll Due to Class 1, 2, or 3 IIN";
#endif 
#if MDNPDATA_SUPPORT_OBJ120
    if (pMDNPSession->autoRequestAggrModeMask & MDNPSESN_AUTO_EVENT_POLL)
    {
      reqDesc.authAggressiveMode = TMWDEFS_TRUE;
    }
#endif
    reqDesc.priority = MDNPBRM_INTEGRITY_PRIORITY;
    mdnpbrm_readClass(&reqDesc, TMWDEFS_NULL, DNPDEFS_QUAL_ALL_POINTS, 0,
      TMWDEFS_FALSE, TMWDEFS_TRUE, TMWDEFS_TRUE, TMWDEFS_TRUE);
  }

  pMDNPSession->dnp.suppressError = TMWDEFS_FALSE;

  /* Store current IIN for next pass */
  pMDNPSession->previousIIN = iin;

}

/* function _prepareMessage */
static void TMWDEFS_LOCAL _prepareMessage(
  TMWSESN *pSession,
  TMWSESN_TX_DATA *pTxData)
{
  TMWTYPES_UCHAR fc;
  MDNPSESN *pMDNPSession = (MDNPSESN *)pSession;

  /* Mark request as a MASTER request */
  pTxData->txFlags |= TMWSESN_TXFLAGS_MASTER;

  fc = pTxData->pMsgBuf[1];

  /* Don't update sequence number secure authentication messages */
  /* For testing purposes, SendGenericRequest can be used for sending SA messages like g120v2, but we want to get user callbacks */
#if MDNPDATA_SUPPORT_OBJ120
  if(!pMDNPSession->authenticationEnabled
    || ((fc != DNPDEFS_FC_AUTH_REQUEST) 
      &&(fc != DNPDEFS_FC_AUTH_REQUEST_NOACK))
    || (!memcmp(pTxData->pMsgDescription, "Generic Request", 15)))
  {
#endif 
    /* Don't update sequence number for confirmations */
    if(fc != DNPDEFS_FC_CONFIRM)
    {
      /* Update request sequence number */
      pMDNPSession->reqSequenceNumber = (TMWTYPES_UCHAR)
        ((pMDNPSession->reqSequenceNumber + 1) & DNPDEFS_AC_SEQUENCE_MASK);

      pTxData->pMsgBuf[DNPDEFS_AH_INDEX_APPL_CTRL] &= ~DNPDEFS_AC_SEQUENCE_MASK;
      pTxData->pMsgBuf[DNPDEFS_AH_INDEX_APPL_CTRL] |= pMDNPSession->reqSequenceNumber;
    }

    dnputil_setCurrentMessage(pSession, pTxData);

#if MDNPDATA_SUPPORT_OBJ120
  }
  if((pTxData->txFlags & TMWSESN_TXFLAGS_DNP_AUTH_CHALL) !=0)
  {
    mdnpauth_saveLastChallenge(pTxData);
  } 
  else if((pTxData->txFlags & TMWSESN_TXFLAGS_DNP_AUTH_AGGR) !=0)
  {   
    mdnpauth_addAggrRequestEnd(pTxData); 
  } 
#endif 
}

/* function _beforeTxCallback */
static void TMWDEFS_LOCAL _beforeTxCallback(
  TMWSESN_TX_DATA *pTxData)
{
#if MDNPDATA_SUPPORT_OBJ50_V1
  TMWSESN *pSession = (TMWSESN *)pTxData->pSession;
  MDNPSESN *pMDNPSession = (MDNPSESN *)pSession;
  if(pTxData->pMsgBuf[DNPDEFS_AH_INDEX_FUNC_CODE] == DNPDEFS_FC_DELAY_MEASURE)
  {
    /* Store time delay measurement request was transmitted */
    pMDNPSession->delayMeasurementTxTime = tmwtarg_getMSTime();
  }

  if(pTxData->txFlags & TMWSESN_TXFLAGS_STORE_DNP_TIME)
  {
    TMWTYPES_MS_SINCE_70 msSince70;
    TMWTYPES_UCHAR buf[6];
    TMWDTIME dTime;

    /* Get current time */
    tmwdtime_getDateTime(pSession, &dTime); 

    /* Add one way propagation delay */
    tmwdtime_addOffset(&dTime, pMDNPSession->propagationDelay);

    /* Convert to DNP time and store in buffer */
    dnpdtime_dateTimeToMSSince70(&msSince70, &dTime);
    dnpdtime_writeMsSince70(buf, &msSince70);

    if((pTxData->txFlags & TMWSESN_TXFLAGS_DNP_AUTH_AGGR) == 0)
    {
      /* Insert time into message */
      pSession->pChannel->pTprt->pTprtUpdateMsg(
        pSession->pChannel->pTprtContext, 6, buf, 6);
    }
    else
    {  
#if MDNPDATA_SUPPORT_OBJ120
      mdnpauth_aggrTimeSync(pTxData, buf);
#endif
    } 
  }   

#else
  TMWTARG_UNUSED_PARAM(pTxData);
#endif
}

/* function _afterTxCallback */
static void TMWDEFS_LOCAL _afterTxCallback(
  TMWSESN_TX_DATA *pTxData)
{
  TMWSESN *pSession = pTxData->pSession;
  MDNPSESN *pMDNPSession = (MDNPSESN *)pSession;
#if MDNPDATA_SUPPORT_OBJ50_V3
  /* Save time last byte was transmitted if requested */
  if((pTxData->txFlags & TMWSESN_TXFLAGS_SAVE_LAST_BYTE_TIME) != 0)
    tmwdtime_getDateTime(pSession, &pMDNPSession->lastByteTime);
#endif
  
  /* See if a response is expected */
  if(pTxData->txFlags & TMWSESN_TXFLAGS_NO_RESPONSE)
  {
    /* Nope, we are done with this message */
    DNPCHNL_TX_DATA *pDNPTxData = (DNPCHNL_TX_DATA *)pTxData;

    if((pDNPTxData->pInternalCallback != TMWDEFS_NULL)
      || (pDNPTxData->pUserCallback != TMWDEFS_NULL))
    {
      DNPCHNL_RESPONSE_INFO response;

      /* Initialize callback response info */
      response.iin = 0;
      response.last = TMWDEFS_TRUE;
      response.pSession = pSession;
      response.pTxData = pTxData;
      response.pRxData = TMWDEFS_NULL;
      response.status = DNPCHNL_RESP_STATUS_SUCCESS;
      response.requestStatus = 0;

      /* Call internal callback */
      if(pDNPTxData->pInternalCallback != TMWDEFS_NULL)
      {
        pDNPTxData->pInternalCallback(
          pDNPTxData->pInternalCallbackParam, &response);
      }

      if((pTxData->txFlags & TMWSESN_TXFLAGS_DNP_AUTH) == 0)
        /* Call user callback, if specified */
        dnpchnl_userCallback(pDNPTxData->tmw.pChannel, pDNPTxData, &response);
    }
  }
  
#if MDNPDATA_SUPPORT_OBJ120  
#if MDNPCNFG_SUPPORT_SA_VERSION5
  if(pMDNPSession->authenticationEnabled)
    mdnpsa_msgSent(pSession);
#endif
#endif
}

/* function _failedTxCallback */
static void TMWDEFS_LOCAL _failedTxCallback(
  TMWSESN_TX_DATA *pTxData)
{
  TMWTARG_UNUSED_PARAM(pTxData);
}
 

/* function: _infoCallback */
static void TMWDEFS_CALLBACK _infoCallback(
  TMWSESN *pSession,
  TMWSCL_INFO sesnInfo)
{
  MDNPSESN *pMDNPSession = (MDNPSESN *)pSession;
  
#ifdef TMW_SUPPORT_MONITOR
  if(pSession->pChannel->pPhysContext->monitorMode)
    return;
#endif

#if MDNPDATA_SUPPORT_OBJ120
  if(pMDNPSession->authenticationEnabled)
  {
    if(sesnInfo == TMWSCL_INFO_ONLINE)
    { 
      mdnpauth_online(pSession);
    }
    else if(sesnInfo == TMWSCL_INFO_TIMEOUT)
    {
      mdnpauth_applTimeout(pSession);
      mdnpauth_checkForQueuedAuthEvent(pSession);
    } 
  }
#endif
  
  /* don't send this auto request if in the context of dnpsesn_openSession */
  if((!pMDNPSession->openInProgress) && (sesnInfo == TMWSCL_INFO_ONLINE))
  {
    if(pMDNPSession->autoRequestMask & MDNPSESN_AUTO_INTEGRITY_ONLINE)
    {
#if TMWCNFG_SUPPORT_DIAG
      _autoIntegrityReq(pSession, "Integrity Poll Due to going ONLINE");
#else
      _autoIntegrityReq(pSession, TMWDEFS_NULL);
#endif
    }
  }
}

/* function: _selOpDelayTimeout */ 
static void TMWDEFS_CALLBACK _selOpDelayTimeout(
  void *pCallbackParam)
{ 
  TMWSESN *pSession = (TMWSESN *)pCallbackParam;
#if MDNPDATA_SUPPORT_OBJ120 
  mdnpauth_checkForQueuedAuthEvent(pSession);
#endif
  dnpchnl_okToSend(pSession->pChannel);
}
 
/* function: _processSelectOperate 
 * purpose: Process response messages from remote device for select or operate request
 * arguments:
 * returns:
 *  void
 */
static DNPCHNL_RESP_STATUS TMWDEFS_LOCAL _processSelectOperate( 
  TMWSESN *pSession,
  TMWSESN_RX_DATA *pRxFragment)
{
  TMWSESN_TX_DATA *pTxData;
  int i; 
  DNPCHNL_RESP_STATUS status;
  DNPCHNL_RESP_STATUS respStatus;
  TMWTYPES_USHORT objIndex;
  DNPUTIL_OBJECT_HEADER header;
  DNPUTIL_RX_MSG msg;

  status = DNPCHNL_RESP_STATUS_SUCCESS;
  respStatus = DNPCHNL_RESP_STATUS_SUCCESS;
  pTxData = dnputil_getCurrentMessage(pSession);

  /* Confirm that the remote device echoed the
   * request byte for byte in the response
   */

#if MDNPDATA_SUPPORT_OBJ120
  {
    TMWTYPES_USHORT authTxOffset = 0;
    TMWTYPES_USHORT authRxOffset = 2;
    TMWTYPES_USHORT msgLength = pTxData->msgLength;
    DNPCHNL_TX_DATA *pDNPTxData = (DNPCHNL_TX_DATA*)pTxData;

    /* Response must be 2 bytes longer to hold IIN bits */
    /* If request was aggressive mode and response was also aggressive mode the length might match 
     * but the data will not be the same 
     */
    if(pRxFragment->msgLength != (pTxData->msgLength + 2)
      ||(pDNPTxData->authAggressiveMode))
    {
      /* if the length does not match, check to see if this was an aggressive mode request, 
       * which will not have the aggressive mode objects in the response
       */

      /* was the request sent with aggressive mode objects surrounding it */
      if(pDNPTxData->authAggressiveMode)
      {
        /* If authentication aggressive mode objects are in request, remove them.
         * Subtract length of g120v3 and g120v9, and move past g120v3.
         */
        msgLength = (TMWTYPES_USHORT)pTxData->msgLength - (10 + pDNPTxData->authAggrModeObjLength);
        authTxOffset = 10;
      }
      else
      {
        authTxOffset = 0;
        msgLength = pTxData->msgLength;
      }

      /* if the length still does not match, while not likely,
       * check for an aggressive mode object in response and skip it, it has already been processed by secure authentication  
       */
      if(pRxFragment->msgLength != (msgLength + 2)) 
      {
        if((pRxFragment->pMsgBuf[4] == DNPDEFS_OBJ_120_AUTHENTICATION)
          && (pRxFragment->pMsgBuf[5] == 3))
        {
          authRxOffset = 12;  
          /* remove aggressive mode variation 9 from end, so that status check later succeeds */
          pRxFragment->msgLength = (TMWTYPES_USHORT)(pRxFragment->msgLength - mdnpauth_getAggrEndLength(pSession));
        } 
        else
        {
          MDNPDIAG_ERROR(pTxData->pChannel, pTxData->pSession, MDNPDIAG_ECHO);
          /* To prevent this being parsed further in MDNPResponseParser */
          pRxFragment->msgLength = 4;
          pRxFragment->invalidFormat = TMWDEFS_TRUE;
          return(DNPCHNL_RESP_STATUS_MISMATCH);
        }
      }
    }

    /* See if this is an exact echo of the request
     * ignoring AC, FC, and IIN bits. If not, call the specific
     * response function to see if it is just the status that does not match.
     */
    for(i = 2; i < msgLength; i++)
    {
      if(pTxData->pMsgBuf[i+authTxOffset] != pRxFragment->pMsgBuf[i+authRxOffset] )
      {
        status = DNPCHNL_RESP_STATUS_MISMATCH;
        break;
      }
    }
  }
#else
  /* Response must be 2 bytes longer to hold IIN bits */
  if(pRxFragment->msgLength != (pTxData->msgLength + 2))
  {
    MDNPDIAG_ERROR(pTxData->pChannel, pTxData->pSession, MDNPDIAG_ECHO);
    /* To prevent this being parsed further in MDNPResponseParser */
    pRxFragment->msgLength = 4;
    pRxFragment->invalidFormat = TMWDEFS_TRUE;
    return(DNPCHNL_RESP_STATUS_MISMATCH);
  }  

  /* See if this is an exact echo of the request
   * ignoring AC, FC, and IIN bits. If not, call the specific
   * response function to see if it is just the status that does not match.
   */
  for(i = 2; i < pTxData->msgLength; i++)
  {
    if(pTxData->pMsgBuf[i] != pRxFragment->pMsgBuf[i+2] )
    {
      status = DNPCHNL_RESP_STATUS_MISMATCH;
      break;
    }
  }
#endif 

  if(status != DNPCHNL_RESP_STATUS_SUCCESS)
  {
    objIndex = 0;

    /* Parse application header */
    dnputil_parseApplHeader(pRxFragment, &msg, TMWDEFS_FALSE);

    /* Look through message processing objects one at a time */
    while(msg.offset < pRxFragment->msgLength)
    {
      /* Read object header */
      if(!dnputil_parseObjectHeader(&msg, objIndex, &header))
      {
        DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_PARSE_HEADER);
        break;
      }

      /* Diagnostics */
      DNPDIAG_SHOW_OBJECT_HEADER(pSession, &header);

      /* Loop through function lookup table to get the appropriate processing
       * function.
       */
      i=0;
      while(_mdnpSelOperFuncTable[i].group != 0)
      {
        if((_mdnpSelOperFuncTable[i].group == header.group)
          && (_mdnpSelOperFuncTable[i].variation == header.variation))
        {
           /*  validate message size based on header info */
          if(!_validateMessageSize(_mdnpSelOperFuncTable[i].dataLength, &header, 
            (TMWTYPES_USHORT)(pRxFragment->msgLength - msg.offset)))
          {  
            DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_INVALID_SIZE);
            break;
          }

          respStatus = _mdnpSelOperFuncTable[i].pResponseFunc(pSession, &msg, &header);
          break;
        }
        i++;
      }
      if(respStatus != DNPCHNL_RESP_STATUS_SUCCESS)
        return(respStatus);
    }
  }
  else
  {  
    /* if this was a select response, start a timer to delay automatic sending 
     * of other DNP requests, to give user a chance to send an operate request, 
     * when the auto select/operate option is not done.
     */
    if(pTxData->pMsgBuf[DNPDEFS_AH_INDEX_FUNC_CODE] == DNPDEFS_FC_SELECT)
    {
      MDNPSESN *pMDNPSession = (MDNPSESN *)pSession;
      if(pMDNPSession->selOpDelayTime > 0)
      {
        tmwtimer_start(&pMDNPSession->selOpDelayTimer,
          pMDNPSession->selOpDelayTime, pSession->pChannel,
          _selOpDelayTimeout, pSession);
      }
    }
  }

  return(status);
}

/* function: _processError
 * purpose: process transmission errors
 * arguments:
 * returns:
 *  void
 */
static void TMWDEFS_CALLBACK _processError(
  TMWSESN *pSession,
  TMWSESN_TX_DATA *pTxData,
  DNPCHNL_RESP_STATUS status)
{
  MDNPSESN *pMDNPSession = (MDNPSESN *)pSession;
  TMWTARG_UNUSED_PARAM(pTxData);

  if(status == DNPCHNL_RESP_STATUS_TIMEOUT) 
  {  
    if(pMDNPSession->autoRequestMask & MDNPSESN_AUTO_INTEGRITY_TIMEOUT)
    {
#if TMWCNFG_SUPPORT_DIAG
    _autoIntegrityReq(pSession, "Integrity Poll Due to Timeout");
#else
    _autoIntegrityReq(pSession, TMWDEFS_NULL);
#endif 
    }
  }
}

/* function: _processMessage */
static DNPCHNL_RESP_STATUS TMWDEFS_LOCAL _processMessage(
  TMWSESN *pSession, 
  TMWSESN_RX_DATA *pRxFragment, 
  TMWTYPES_UCHAR reqFC)
{
  DNPUTIL_OBJECT_HEADER header;
  TMWTYPES_USHORT objIndex = 0;
  DNPUTIL_RX_MSG msg;

  DNPCHNL_RESP_STATUS status = DNPCHNL_RESP_STATUS_SUCCESS;

  /* Make sure function code is one that we process */
  if( (reqFC == DNPDEFS_FC_SELECT)
    ||(reqFC == DNPDEFS_FC_OPERATE)
    ||(reqFC == DNPDEFS_FC_DIRECT_OP))
  {
    return(_processSelectOperate(pSession, pRxFragment));
  }
  else if( (reqFC != DNPDEFS_FC_READ)
        && (reqFC != DNPDEFS_FC_DELAY_MEASURE)
        && (reqFC != DNPDEFS_FC_COLD_RESTART)
        && (reqFC != DNPDEFS_FC_WARM_RESTART)
        && (reqFC != DNPDEFS_FC_AUTHENTICATE)
        && (reqFC != DNPDEFS_FC_OPEN_FILE)
        && (reqFC != DNPDEFS_FC_CLOSE_FILE)
        && (reqFC != DNPDEFS_FC_DELETE_FILE)
        && (reqFC != DNPDEFS_FC_GET_FILE_INFO)
        && (reqFC != DNPDEFS_FC_ABORT)
        && (reqFC != DNPDEFS_FC_WRITE)
        && (reqFC != DNPDEFS_FC_ACTIVATE_CONFIG)
    )
  {
    return(DNPCHNL_RESP_STATUS_SUCCESS);
  }

  /* Parse application header */
  dnputil_parseApplHeader(pRxFragment, &msg, TMWDEFS_FALSE);

  /* Look through message processing objects one at a time */
  while(msg.offset < pRxFragment->msgLength)
  {
    TMWTYPES_BOOL processed = TMWDEFS_FALSE;
    TMWTYPES_BOOL found = TMWDEFS_FALSE;
    TMWTYPES_USHORT savedOffset = (TMWTYPES_USHORT)msg.offset;
    TMWTYPES_USHORT i;

    /* Read object header */
    if(!dnputil_parseObjectHeader(&msg, objIndex, &header))
    {
      status = DNPCHNL_RESP_STATUS_FAILURE;
      DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_PARSE_HEADER);
      /* To prevent this being parsed further in MDNPResponseParser */
      pRxFragment->msgLength = savedOffset;
      pRxFragment->invalidFormat = TMWDEFS_TRUE;
      break;
    }

#if MDNPDATA_SUPPORT_OBJ120
    /* Skip aggressive mode objects, they have already been processed by secure authentication */
    if(header.group == DNPDEFS_OBJ_120_AUTHENTICATION)
    {
      if(header.variation == 3)
      {
        msg.offset += 6;
        continue;
      }
      else if(header.variation == 9)
      {
        msg.offset = pRxFragment->msgLength;
        continue;
      }
    }
#endif

    /* Diagnostics */
    DNPDIAG_SHOW_OBJECT_HEADER(pSession, &header);

    /* Loop through function lookup table to get the appropriate processing
     * function.
     */
    i = 0;
    while(_mdnpFuncTable[i].reqFC != 0)
    {
      if((_mdnpFuncTable[i].reqFC == reqFC)
        && (_mdnpFuncTable[i].group == header.group)
        && (_mdnpFuncTable[i].allVariations
        || (_mdnpFuncTable[i].variation == header.variation)))
      {
        if(_mdnpFuncTable[i].pReadFunc != TMWDEFS_NULL)
        {
          found = TMWDEFS_TRUE;

          /* validate message size based on header info */ 
          if(!_validateMessageSize(_getExpectedMsgLength(header.group, header.variation), 
            &header, (TMWTYPES_USHORT)(msg.msgLength-msg.offset)))
          {  
            DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_INVALID_SIZE);
            break;
          }

          processed = _mdnpFuncTable[i].pReadFunc(pSession, &msg, &header);
          break;
        }
      }

      i++;
    }

    /* See if we successfully processed response */
    if(!processed)
    {
      status = DNPCHNL_RESP_STATUS_FAILURE;
       
      if(found)
        DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_PROC_FRAGMENT);
      else
        MDNPDIAG_ERROR(pSession->pChannel, pSession, MDNPDIAG_OBJVAR);

      /* To prevent this being parsed further in MDNPResponseParser */
      pRxFragment->msgLength = savedOffset;
      pRxFragment->invalidFormat = TMWDEFS_TRUE;
      break;
    }
    objIndex += 1;
  } 

  return(status);
}

#ifdef TMW_SUPPORT_MONITOR
/* function: _processAnalyzerMessage */
static void TMWDEFS_LOCAL _processAnalyzerMessage(
  TMWSESN *pSession, 
  TMWSESN_RX_DATA *pRxFragment)
{
  DNPUTIL_OBJECT_HEADER header;
  TMWTYPES_USHORT objIndex = 0;
  DNPUTIL_RX_MSG msg;
 
  /* Parse application header */
  dnputil_parseApplHeader(pRxFragment, &msg, TMWDEFS_FALSE);

  /* Look through message processing objects one at a time */
  while(msg.offset < pRxFragment->msgLength)
  {
    TMWTYPES_BOOL processed = TMWDEFS_FALSE;
    TMWTYPES_USHORT i;

    /* Read object header */
    if(!dnputil_parseObjectHeader(&msg, objIndex, &header))
    {
      DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_PARSE_HEADER);
      break;
    }

    /* Diagnostics */
    DNPDIAG_SHOW_OBJECT_HEADER(pSession, &header);

    /* Loop through function lookup table to get the appropriate processing
     * function.
     */
    i = 0;
    while(_mdnpFuncTable[i].reqFC != 0)
    {
      if((_mdnpFuncTable[i].group == header.group)
        && (_mdnpFuncTable[i].allVariations
        || (_mdnpFuncTable[i].variation == header.variation)))
      {
        if(_mdnpFuncTable[i].pReadFunc != TMWDEFS_NULL)
        {
          processed = _mdnpFuncTable[i].pReadFunc(pSession, &msg, &header);
          break;
        }
      }

      i++;
    }

    /* See if we successfully processed response */
    if(!processed)
    {
      break;
    }
    objIndex += 1;
  }
}
#endif

/* function: _transmitConfirm */
static void TMWDEFS_LOCAL _transmitConfirm(
  TMWSESN *pSession, 
  TMWTYPES_UCHAR control,
  TMWTYPES_USHORT destAddress)
{ 
  /* Allocate and initialize tx data buffer for confirm */
  DNPCHNL_TX_DATA *pTxData = (DNPCHNL_TX_DATA *)dnpchnl_newTxData(
    pSession->pChannel, pSession, 40, destAddress);

  if(pTxData == TMWDEFS_NULL)
  {
    return;
  }

  /* Make this high priority so it goes to front of request queue */
  pTxData->priority = 255;
  pTxData->tmw.responseTimeout = ((MDNPSESN *)(pSession))->defaultResponseTimeout;

  pTxData->tmw.msgLength = 2;

#if TMWCNFG_SUPPORT_DIAG
  pTxData->tmw.pMsgDescription = "Application Confirmation";
#endif

  /* Fill in application control byte */
  pTxData->tmw.pMsgBuf[0] = (TMWTYPES_UCHAR)(DNPDEFS_AC_FIRST_AND_FINAL
    | (control & (DNPDEFS_AC_SEQUENCE_MASK | DNPDEFS_AC_UNSOLICITED)));

  /* Fill in function code */
  pTxData->tmw.pMsgBuf[1] = DNPDEFS_FC_CONFIRM;

  /* Transmit flags, no retry, no response expected */
  pTxData->tmw.txFlags = (TMWSESN_TXFLAGS_NO_RESPONSE | TMWSESN_TXFLAGS_MASTER);

#if MDNPDATA_SUPPORT_OBJ120
  if(((MDNPSESN *)(pSession))->sendAggrModeConfirm)
  {
    mdnpauth_addAggrRequestStart((TMWSESN_TX_DATA*)pTxData, 1);
    mdnpauth_addAggrRequestEnd((TMWSESN_TX_DATA*)pTxData);
    ((MDNPSESN *)(pSession))->sendAggrModeConfirm = TMWDEFS_FALSE;
    
    /* this got messed with in addAggrRequestEnd */
    pTxData->tmw.pMsgBuf[0] = (TMWTYPES_UCHAR)(DNPDEFS_AC_FIRST_AND_FINAL
      | (control & (DNPDEFS_AC_SEQUENCE_MASK | DNPDEFS_AC_UNSOLICITED)));
  }
#endif

  /* Send the application confirm using normal mechanism, which
   * will now send this even if a request is outstanding.
   */
  dnpchnl_sendFragment((TMWSESN_TX_DATA *)pTxData);
}

/* function: _processResponse 
 * purpose: Process response messages from remote device
 * arguments:
 * returns:
 *  void
 */
static void TMWDEFS_LOCAL _processResponse(
  TMWSESN *pSession, 
  TMWSESN_RX_DATA *pRxFragment)
{
  MDNPSESN *pMDNPSession;
  TMWCHNL *pChannel;
  TMWSESN_TX_DATA *pTxData;
  DNPCHNL_TX_DATA *pDNPTxData;
  TMWTYPES_UCHAR reqFC = 0;
  TMWTYPES_UCHAR ac;

  /* valid messages are at least 4 bytes long */
  if(pRxFragment->msgLength < 4)
    return;

  pMDNPSession = (MDNPSESN *)pSession;
  pChannel = pSession->pChannel;
  pTxData = dnputil_getCurrentMessage(pSession);

  pDNPTxData = (DNPCHNL_TX_DATA *)pTxData;

  /* Get application control byte from response */
  ac = pRxFragment->pMsgBuf[DNPDEFS_AH_INDEX_APPL_CTRL];

  /* Get IIN bits from response */
  pMDNPSession->currentIIN = (TMWTYPES_USHORT)
    ((pRxFragment->pMsgBuf[DNPDEFS_AH_INDEX_IIN] << 8)
    | pRxFragment->pMsgBuf[DNPDEFS_AH_INDEX_IIN2]);

  /* If we have an outstanding request, process the response */
  /* If there are multiple sessions and request on one session may have timed out.
   * Check to see if this is the current request is on the session
   * that received the response
   */
  if((pTxData != TMWDEFS_NULL)
    && (pTxData->pSession == pSession))
  {
    TMWTYPES_UCHAR sequenceNumber;
    DNPCHNL_RESPONSE_INFO response; 

    /* Diagnostics */
    MDNPDIAG_PROCESS_RESPONSE(pSession, pTxData->pMsgDescription);

    sequenceNumber = (TMWTYPES_UCHAR) (ac &0xf);

    if(pMDNPSession->reqSequenceNumber != sequenceNumber)
    {
      MDNPDIAG_ERROR(pSession->pChannel, pSession, MDNPDIAG_BAD_APPL_SEQ);
      return;
    }

#if MDNPDATA_SUPPORT_OBJ120 
    /* Tell authentication an expected response was received */
    mdnpauth_applResponse(pSession);
#endif

    /* Get the request function code */
    reqFC = pTxData->pMsgBuf[DNPDEFS_AH_INDEX_FUNC_CODE];

    /* Process response based on request function code */
    response.status = _processMessage(pSession, pRxFragment, reqFC);
    if(response.status == DNPCHNL_RESP_STATUS_SUCCESS) 
    {
      if((ac & DNPDEFS_AC_FINAL) == 0)
        response.status = DNPCHNL_RESP_STATUS_INTERMEDIATE;
    
      /* If IIN bits indicate a failure, change status */
      if(pMDNPSession->currentIIN  & 0x7)
        response.status = DNPCHNL_RESP_STATUS_IIN; 
    }
    response.requestStatus = 0;
    
    /* If this is the final fragment in the response, cleanup request
     * It works better to check status intermediate because failures also
     * cause txData to be deallocated below
     */
    if(response.status != DNPCHNL_RESP_STATUS_INTERMEDIATE)
    { 
      dnputil_setCurrentMessage(pSession, TMWDEFS_NULL);

      if(reqFC == DNPDEFS_FC_READ)
      {
        pMDNPSession->dnp.readFailedCount = 0;
      }

      /* Tell transport layer that we are done with this request. Transport layer
       * will only have a pointer to this if it required a link layer confirm
       * that was never received, but instead we receive the response to the
       * request and are therefore finished with it.
       */
      pChannel->pTprt->pTprtCancel(pChannel->pTprtContext, pTxData);

      tmwdlist_removeEntry(&pChannel->messageQueue, (TMWDLIST_MEMBER *)pTxData);
    }
    
    /* If application layer confirm was requested, send one */
    if((response.status != DNPCHNL_RESP_STATUS_FAILURE)
      && (ac & DNPDEFS_AC_CONFIRM)
      && (pMDNPSession->autoRequestMask & MDNPSESN_AUTO_CONFIRM))
    {
      /* Yes, confirm it */
      _transmitConfirm(pSession, ac, pRxFragment->rxAddress);
    }

    /* Initialize user callback info */
    response.iin = pMDNPSession->currentIIN;
    response.pSession = pSession;
    response.pTxData = pTxData;
    response.pRxData = pRxFragment;
    response.responseTime = tmwtarg_getMSTime() - pTxData->timeSent;
    response.last = (TMWTYPES_BOOL)((response.status != DNPCHNL_RESP_STATUS_INTERMEDIATE) ? TMWDEFS_TRUE : TMWDEFS_FALSE);

#if MDNPDATA_SUPPORT_OBJ70
    /* Currently, session internal callback is only used by file transfer */
    if(pMDNPSession->pInternalCallback)
    {
      /* Prevent this from being deallocated if it gets reused and there is a transmit error */
      pDNPTxData->referenceCount++; 
      pMDNPSession->pInternalCallback(pSession, &response);
      pDNPTxData->referenceCount--; 
    }
#endif

    if(pDNPTxData->pInternalCallback != TMWDEFS_NULL)
    {
      pDNPTxData->pInternalCallback(pDNPTxData->pInternalCallbackParam, &response);
    }

    /* call user callback if there is one for this request */
    dnpchnl_userCallback(pChannel, pDNPTxData, &response); 
 
    /* Call statistics function to determine if this is a failure and a function is registered */
    DNPSTAT_SESN_CHECK_REQUEST_FAILED(pSession, response.status);

#if MDNPDATA_SUPPORT_OBJ70
    pMDNPSession->pInternalCallback = TMWDEFS_NULL;
#endif

    /* Deallocate txData if we are finished with it */
    if(response.status != DNPCHNL_RESP_STATUS_INTERMEDIATE)
    {
      if(response.status != DNPCHNL_RESP_STATUS_CANCELED)
      {
        dnpchnl_freeTxData(pTxData);
      }
      else
      {
        dnpchnl_cancelFragment(pTxData);
      }
    }
  }
#ifdef TMW_SUPPORT_MONITOR
  /* If this channel is in passive listen only analyzer mode
   * no request would have been sent, therefore pTxData would be
   * NULL, call an analyzer function to process message and show IIN bits. 
   */
  else if(pChannel->pPhysContext->monitorMode)
  {
    _processAnalyzerMessage(pSession, pRxFragment); 
     
    /* Diagnostics */
    MDNPDIAG_SHOW_IIN(pSession, pMDNPSession->currentIIN);
    return;
  }
#endif

  if(ac & DNPDEFS_AC_FINAL)
  {
    /* Process IIN bits */
    _processIIN(pSession, reqFC);
  }
  else
  {
    /* Process event buffer overflow bit */
    /* In multiple fragment cases, overflow bit will be set in the first fragment, and will be cleared once confirmation is received.    */
    /* It is better to do an integrity poll here since the last fragment will not have an overflow bit set. This will not cause multiple */
    /* integrity polls being queued since _autoIntegrityReq checks to see if there is an integrity poll outstanding.                     */
    if ((pMDNPSession->currentIIN & DNPDEFS_IIN_BUFFER_OVFL)
      && (pMDNPSession->autoRequestMask & MDNPSESN_AUTO_INTEGRITY_OVERFLOW))
    {
#if TMWCNFG_SUPPORT_DIAG
      _autoIntegrityReq(pSession, "Integrity Poll Due to Buffer Overflow IIN");
#else
      _autoIntegrityReq(pSession, TMWDEFS_NULL);
#endif
    }

    /* SCL will not process IIN bits, but will show them and call user function */
    pMDNPSession->reqSequenceNumber = (TMWTYPES_UCHAR)((pMDNPSession->reqSequenceNumber + 1) & 0x0f);

    /* Diagnostics */
    MDNPDIAG_SHOW_IIN(pSession, pMDNPSession->currentIIN);

    /* Call the user routine to process IIN bits */
    mdnpdata_processIIN(pSession, &pMDNPSession->currentIIN);
  }
}

/* function: _processUnsolicited 
 * purpose: Process usolicited response messages from remote device
 * arguments:
 * returns:
 *  void
 */
static void TMWDEFS_LOCAL _processUnsolicited(
  TMWSESN *pSession, 
  TMWSESN_RX_DATA *pRxFragment)
{
  MDNPSESN *pMDNPSession = (MDNPSESN *)pSession;
  TMWTYPES_UCHAR ac;
  DNPCHNL_RESP_STATUS status;

  /* Get message control and IIN info */
  ac = pRxFragment->pMsgBuf[DNPDEFS_AH_INDEX_APPL_CTRL];

  pMDNPSession->currentIIN = (TMWTYPES_USHORT)
    ((pRxFragment->pMsgBuf[DNPDEFS_AH_INDEX_IIN] << 8)
    | pRxFragment->pMsgBuf[DNPDEFS_AH_INDEX_IIN2]);

#ifdef TMW_SUPPORT_MONITOR  
  if(!pSession->pChannel->pPhysContext->monitorMode)
#endif

  /* If in Startup state, we need to discard and not confirm 
   * unsolicited responses. App Layer Volume 2 Part 2 1.1.2
   * and Part 3 1.5.
   * To allow initial NULL unsolicited when in STARTUP state, this define
   * can be set to TMWDEFS_FALSE
   */
#if !MDNPDATA_DISCARD_NULL_UNSOL
  if(pRxFragment->msgLength > 4)
#endif
    if(pMDNPSession->unsolRespState == MDNPSESN_UNSOL_STARTUP)
    {
      MDNPDIAG_ERROR(pSession->pChannel, pSession, MDNPDIAG_UNSOL_STARTUP);
      return;
    }

  /* Unsolicited response is processed as a read response */
  status = _processMessage((TMWSESN *)pSession, pRxFragment, DNPDEFS_FC_READ);

#ifdef TMW_SUPPORT_MONITOR
  /* If this channel is in passive listen only analyzer mode 
   * call an analyzer function to process message and show IIN bits. 
   */
  if(pSession->pChannel->pPhysContext->monitorMode)
  { 
    /* Diagnostics */
    MDNPDIAG_SHOW_IIN(pSession, pMDNPSession->currentIIN);
    return;
  }
#endif
 
  /* See if confirm requested */
  if((status != DNPCHNL_RESP_STATUS_FAILURE)
    && (ac & DNPDEFS_AC_CONFIRM)
    && (pMDNPSession->autoRequestMask & MDNPSESN_AUTO_CONFIRM))
  {
    _transmitConfirm(pSession, ac, pRxFragment->rxAddress);
  }

#if MDNPDATA_SUPPORT_OBJ70
  /* Currently, session internal callback is only used by file transfer      */
  /* File transfer, to support NULL response and then a later event response */
  if(pMDNPSession->pInternalCallback)
  {
    /* Initialize user callback info */
    DNPCHNL_RESPONSE_INFO response; 
    response.pSession = pSession;
    response.pTxData  = TMWDEFS_NULL;
    response.pRxData  = pRxFragment;
    response.status   = status;
    response.requestStatus = 0;
    pMDNPSession->pInternalCallback(pSession, &response);

    pMDNPSession->pInternalCallback = TMWDEFS_NULL;
  }
#endif

  if(pMDNPSession->pUnsolUserCallback != TMWDEFS_NULL)
  {
    MDNPSESN_UNSOL_RESP_INFO response;
    response.iin = pMDNPSession->currentIIN;
    response.pRxData = pRxFragment;
    response.pSession = pSession;
    response.status = status;
    pMDNPSession->pUnsolUserCallback(pMDNPSession->pUnsolUserCallbackParam, &response);
  }

  /* Process IIN bits */
  _processIIN((TMWSESN *)pSession, 0);
}
 
/* External functions */

/* function: mdnpsesn_initConfig */
void TMWDEFS_GLOBAL mdnpsesn_initConfig(
  MDNPSESN_CONFIG *pConfig)
{
  pConfig->source = 3;
  pConfig->destination = 4;

  pConfig->active = TMWDEFS_TRUE;

  pConfig->autoRequestMask =
    MDNPSESN_AUTO_CLEAR_RESTART
    | MDNPSESN_AUTO_INTEGRITY_RESTART
    | MDNPSESN_AUTO_INTEGRITY_ONLINE
    | MDNPSESN_AUTO_INTEGRITY_LOCAL
    | MDNPSESN_AUTO_INTEGRITY_OVERFLOW
    | MDNPSESN_AUTO_TIME_SYNC
    | MDNPSESN_AUTO_EVENT_POLL
    | MDNPSESN_AUTO_UNSOL_STARTUP
    | MDNPSESN_AUTO_DISABLE_UNSOL
    /*| MDNPSESN_AUTO_DATASET_RESTART*/
    | MDNPSESN_AUTO_CONFIRM;

  pConfig->autoEnableUnsolClass1 = TMWDEFS_TRUE;
  pConfig->autoEnableUnsolClass2 = TMWDEFS_TRUE;
  pConfig->autoEnableUnsolClass3 = TMWDEFS_TRUE; 

  pConfig->linkStatusPeriod = 0;
  pConfig->linkStatusTimeoutDisconnect = TMWDEFS_TRUE;
  pConfig->defaultResponseTimeout = TMWDEFS_SECONDS(30);
  pConfig->selOpDelayTime = 0;

  pConfig->readTimeoutsAllowed = 0;
  pConfig->sesnDiagMask = TMWDIAG_ID_DEF_MASK;

#if MDNPDATA_SUPPORT_OBJ91
  pConfig->combineActConfigData = TMWDEFS_TRUE;
#endif

#if MDNPDATA_SUPPORT_OBJ70
  pConfig->maxFileBlockSize = MDNPCNFG_FILE_BLOCK_SIZE;
#endif

  /* User provided statistics callback function */
  pConfig->pStatCallback = TMWDEFS_NULL; 
  pConfig->pStatCallbackParam = TMWDEFS_NULL;
  
#if MDNPDATA_SUPPORT_OBJ120 
  pConfig->authenticationEnabled = TMWDEFS_FALSE;
  mdnpauth_initConfig(&pConfig->authConfig);
#endif
}

#if DNPCNFG_SUPPORT_BINCONFIG
/* function: mdnpsesn_initConfigUsingBinary */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpsesn_applyBinaryFileValues(
    char * pFileName,
    DNPCHNL_CONFIG *pDNPConfig,
    DNPLINK_CONFIG *pLinkConfig, 
    void *pIoConfig,
    MDNPSESN_CONFIG *pSesnConfig)
{
  FILE *configfp;
  TMWTYPES_BOOL isChannelSerial;
  TMWTYPES_BOOL success;
  TMWTYPES_USHORT xmlVersionIndex;
  DNPBNCFG_FILEVALUES binFileValues;
  TMWTARG_BINFILE_VALS binFileTargValues;
  
  configfp = NULL;
  xmlVersionIndex = 0;
  isChannelSerial = TMWDEFS_FALSE;
  success = TMWDEFS_FALSE;

  tmwtarg_initBinFileValues(&binFileTargValues);
  binFileTargValues.sessionIsOutstation = TMWDEFS_FALSE;

#if defined(_MSC_VER) && !defined(_WIN32_WCE)
#if _MSC_VER >= 1400
  fopen_s(&configfp, pFileName, "rb"); 
#else
  configfp = fopen(pFileName, "rb");
#endif
#endif

  if(configfp != NULL)
  {
    success = dnpbncfg_ReadBinaryConfigFile(configfp, &binFileValues, &binFileTargValues, &xmlVersionIndex);

    fclose(configfp);
  }
  
  if(success)
    success = tmwtarg_applyBinFileTargValues(pIoConfig, &binFileTargValues, &isChannelSerial);

  if(success)
    success = mdnpsesn_getBinFileSessionValues(pSesnConfig, &binFileValues, isChannelSerial);

  if(success)
    success = dnpchnl_getBinFileChannelValues(pDNPConfig, pLinkConfig, &binFileValues, TMWDEFS_FALSE);

  return success;
}
#endif
 
/* function: _okToSend */
static  TMWTYPES_BOOL TMWDEFS_LOCAL _okToSend(
    TMWSESN *pSession)
{
  MDNPSESN *pMDNPSession = (MDNPSESN *)pSession;
  if(tmwtimer_isActive(&pMDNPSession->selOpDelayTimer))
  {
    return(TMWDEFS_FALSE);
  }
#if MDNPDATA_SUPPORT_OBJ120 
  return (mdnpauth_OKToSend(pSession));
#else
  return(TMWDEFS_TRUE);
#endif
}

/* function: mdnpsesn_openSession */
TMWSESN * TMWDEFS_GLOBAL mdnpsesn_openSession(
  TMWCHNL *pChannel,
  const MDNPSESN_CONFIG *pConfig,
  void *pUserHandle)
{
  TMWSESN *pSession; 
  MDNPSESN *pMDNPSession;

  if(pChannel == TMWDEFS_NULL)
    return(TMWDEFS_NULL);
  
  if(!tmwappl_getInitialized(TMWAPPL_INIT_MDNP))
  {
    if(!mdnpmem_init(TMWDEFS_NULL))
      return(TMWDEFS_NULL);

#if TMWCNFG_SUPPORT_DIAG
    mdnpdiag_init();
#endif
    tmwappl_setInitialized(TMWAPPL_INIT_MDNP);
  }

  /* Allocate space for session context */
  pMDNPSession = (MDNPSESN *)mdnpmem_alloc(MDNPMEM_MDNPSESN_TYPE);
  if(pMDNPSession == TMWDEFS_NULL)
  {
    return(TMWDEFS_NULL);
  }

  pMDNPSession->dnp.pBeforeTxCallback = _beforeTxCallback;
  pMDNPSession->dnp.pAfterTxCallback = _afterTxCallback;
  pMDNPSession->dnp.pFailedTxCallback = _failedTxCallback;
  pMDNPSession->dnp.pPrepareMessage = _prepareMessage;
  pMDNPSession->dnp.pAbortMessage = _processError;
  pMDNPSession->dnp.pNextMessage = TMWDEFS_NULL;
  pMDNPSession->dnp.pProcessInfoFunc = _infoCallback;
  pMDNPSession->dnp.pProcessFragmentFunc = mdnpsesn_processFragment;
  pMDNPSession->dnp.pIdleFunc = _idleCallback;
  pMDNPSession->dnp.pUserHandle = pUserHandle;
  pMDNPSession->dnp.pCheckData = _okToSend;
#if MDNPDATA_SUPPORT_OBJ120 
  pMDNPSession->pSAUserCallback = TMWDEFS_NULL;
  pMDNPSession->authenticationEnabled = pConfig->authenticationEnabled;
  pMDNPSession->dnp.operateInV2Mode = pConfig->authConfig.operateInV2Mode;
#if DNPCNFG_MULTI_SESSION_REQUESTS
  pMDNPSession->dnp.pSaveLastUnsolSentFunc = TMWDEFS_NULL;
#endif
  pMDNPSession->pAuthenticationInfo = TMWDEFS_NULL;
  pMDNPSession->sendAggrModeConfirm = TMWDEFS_FALSE;
  pMDNPSession->autoRequestAggrModeMask = pConfig->authConfig.autoRequestAggrModeMask;
#endif

  pSession = (TMWSESN*)pMDNPSession;
  pSession->pChannel = pChannel;
  pSession->pLinkSession = TMWDEFS_NULL;

  /* Initialize master database */
  pMDNPSession->pDbHandle = mdnpdata_init(pSession, pUserHandle);
  if(pMDNPSession->pDbHandle == TMWDEFS_NULL)
  {
    /* Log error */
    mdnpmem_free(pMDNPSession);
    return(TMWDEFS_NULL);
  }

  /* Configuration */
  if(!mdnpsesn_setSessionConfig(pSession, pConfig))
  {
    mdnpmem_free(pMDNPSession);
    return(TMWDEFS_NULL);
  }

  tmwtimer_init(&pMDNPSession->selOpDelayTimer);

  /* Initialize state info */
  pMDNPSession->currentIIN = 0;
  pMDNPSession->previousIIN = 0;
  pMDNPSession->reqSequenceNumber = 0;
  pMDNPSession->unsolRespState = MDNPSESN_UNSOL_IDLE;
  pMDNPSession->pUnsolUserCallback = TMWDEFS_NULL;
  /* A new CTO must be received in each fragment that contains an obj2 or obj4 with relative time.
   * TB2018-001 says fragment shall be discarded and not confirmed.
   * Use invalid year 0 to indicate obj51 has not been received yet.
   */
  pMDNPSession->lastCTOReceived.year = 0;
#if MDNPDATA_SUPPORT_OBJ70
  pMDNPSession->pFileXferContext = TMWDEFS_NULL;
  pMDNPSession->pInternalCallback = TMWDEFS_NULL;
#endif

#if MDNPDATA_SUPPORT_OBJ50_V1
  /* Initialize propagation delay and time sync info */
  pMDNPSession->propagationDelay = 0;
#endif


  /* This #if MULTI is required to prevent overwriting pChannel->pCurrentMesssage */
#if DNPCNFG_MULTI_SESSION_REQUESTS 
  dnputil_setCurrentMessage(pSession, TMWDEFS_NULL);
#endif

  pMDNPSession->openInProgress = TMWDEFS_TRUE;

  /* Lock channel */
  TMWTARG_LOCK_SECTION(&pChannel->lock);

  /* Initialize generic DNP session */
  dnpsesn_openSession(pChannel, pSession, pConfig->pStatCallback, pConfig->pStatCallbackParam,
    TMWTYPES_SESSION_TYPE_MASTER, pConfig->linkStatusPeriod, TMWDEFS_TRUE, TMWDEFS_FALSE);

  pMDNPSession->openInProgress = TMWDEFS_FALSE;
  pMDNPSession->dnp.readTimeoutsAllowed = pConfig->readTimeoutsAllowed;
  
  /* Unlock channel */
  TMWTARG_UNLOCK_SECTION(&pChannel->lock);
  
#ifdef TMW_SUPPORT_MONITOR
  if(pChannel->pPhysContext->monitorMode)
    return(pSession);
#endif

  /* When master starts up it should send unsolicited disable and discard unsoliciteds 
   * until an integrity poll completes 
   */
  if(pMDNPSession->autoRequestMask & MDNPSESN_AUTO_UNSOL_STARTUP)
  {
    pMDNPSession->unsolRespState = MDNPSESN_UNSOL_STARTUP; 
    _autoDisableUnsolReq(pSession);
  }

#if MDNPDATA_SUPPORT_DATASETS
  /* If datasets need to be exchanged they should be done before an integrity poll
   * in case there are dataset events 
   */
  if(pMDNPSession->autoRequestMask & MDNPSESN_AUTO_DATASET_RESTART)
  {
    _autoDatasetReq(pSession, "Read/Write Data Set Protypes and Descriptors Due to Master Startup");
  }
#endif
  
  if(pMDNPSession->autoRequestMask & MDNPSESN_AUTO_INTEGRITY_RESTART)
  {
#if TMWCNFG_SUPPORT_DIAG
    _autoIntegrityReq(pSession, "Integrity Poll Due to Master Restart");
#else
    _autoIntegrityReq(pSession, TMWDEFS_NULL);
#endif
  }

  /* Unsolicited responses should not be enabled until after an integrity poll 
   * MDNPSESN_AUTO_INTEGRITY_ONLINE will handle this properly. 
   */
  if(pMDNPSession->autoRequestMask & MDNPSESN_AUTO_ENABLE_UNSOL)
  {
#if TMWCNFG_SUPPORT_DIAG
    _autoEnableUnsolReq(pSession, "Enable Unsolicited Due to Master Startup");
#else
    _autoEnableUnsolReq(pSession, TMWDEFS_NULL);
#endif
  } 

  return(pSession);
}

/* function: mdnpsesn_modifySession */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpsesn_modifySession(
  TMWSESN *pSession,
  const MDNPSESN_CONFIG *pConfig,
  TMWTYPES_ULONG configMask)
{
  MDNPSESN *pMDNPSession = (MDNPSESN *)pSession;

  if((configMask & MDNPSESN_CONFIG_SOURCE) != 0)
  {
    pSession->srcAddress = pConfig->source;
  }

  if((configMask & MDNPSESN_CONFIG_DESTINATION) != 0)
  {
    pSession->destAddress = pConfig->destination;
  }

  if((configMask & MDNPSESN_CONFIG_RESP_TIMEOUT) != 0)
  {
    pMDNPSession->defaultResponseTimeout = pConfig->defaultResponseTimeout;
  }

  if((configMask & MDNPSESN_CONFIG_ACTIVE) != 0)
  {
    pSession->active = pConfig->active;
  }

  if((configMask & MDNPSESN_CONFIG_AUTO) != 0)
  {
    pMDNPSession->autoRequestMask = pConfig->autoRequestMask;
  }

  return(TMWDEFS_TRUE);
}

/* function: mdnpsesn_getSessionConfig */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpsesn_getSessionConfig(
  TMWSESN *pSession,
  MDNPSESN_CONFIG *pConfig)
{
  MDNPSESN *pMDNPSession = (MDNPSESN*)pSession;
  DNPSESN *pDNPSession = (DNPSESN *)pSession;
  
  pConfig->active              = pSession->active;
  pConfig->source              = pSession->srcAddress;
  pConfig->destination         = pSession->destAddress;
  pConfig->pStatCallback       = pSession->pStatCallbackFunc;
  pConfig->pStatCallbackParam  = pSession->pStatCallbackParam;

  pConfig->autoRequestMask     = pMDNPSession->autoRequestMask; 
   
  if(pMDNPSession->autoRequestBits &= MDNPSESN_CLASS1_AUTO_ENABLE)
    pConfig->autoEnableUnsolClass1 = TMWDEFS_TRUE;
  else
    pConfig->autoEnableUnsolClass1 = TMWDEFS_FALSE;

  if(pMDNPSession->autoRequestBits &= MDNPSESN_CLASS2_AUTO_ENABLE)
    pConfig->autoEnableUnsolClass2 = TMWDEFS_TRUE;
  else
    pConfig->autoEnableUnsolClass2 = TMWDEFS_FALSE;

  if(pMDNPSession->autoRequestBits &= MDNPSESN_CLASS3_AUTO_ENABLE)
    pConfig->autoEnableUnsolClass3 = TMWDEFS_TRUE;
  else
    pConfig->autoEnableUnsolClass3 = TMWDEFS_FALSE;

  pConfig->defaultResponseTimeout = pMDNPSession->defaultResponseTimeout;
  pConfig->selOpDelayTime         = pMDNPSession->selOpDelayTime;

  pConfig->linkStatusPeriod           = pDNPSession->linkStatusPeriod;
  pConfig->linkStatusTimeoutDisconnect= pDNPSession->linkStatusTimeoutDisconnect;

  pConfig->readTimeoutsAllowed    = pDNPSession->readTimeoutsAllowed;
  pConfig->sesnDiagMask           = pMDNPSession->dnp.tmw.sesnDiagMask;

#if MDNPDATA_SUPPORT_OBJ91
  pConfig->combineActConfigData   = pMDNPSession->combineActConfigData;
#endif
#if MDNPDATA_SUPPORT_OBJ70
  pConfig->maxFileBlockSize       = pMDNPSession->maxFileBlockSize;
#endif

#if MDNPDATA_SUPPORT_OBJ120 
  pConfig->authenticationEnabled = pMDNPSession->authenticationEnabled;
  mdnpauth_getConfig(pSession, &pConfig->authConfig);  
#endif

  return(TMWDEFS_TRUE);
}

/* function: mdnpsesn_setSessionConfig */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpsesn_setSessionConfig(
  TMWSESN *pSession,
  const MDNPSESN_CONFIG *pConfig)
{
  MDNPSESN *pMDNPSession = (MDNPSESN*)pSession;
  DNPSESN *pDNPSession = (DNPSESN *)pSession;
  
  pSession->active             = pConfig->active;
  pSession->srcAddress         = pConfig->source;
  pSession->destAddress        = pConfig->destination;
  pSession->pStatCallbackFunc  = pConfig->pStatCallback;
  pSession->pStatCallbackParam = pConfig->pStatCallbackParam;

  pMDNPSession->autoRequestMask = pConfig->autoRequestMask;
  pMDNPSession->autoRequestBits = 0;
  if(pConfig->autoEnableUnsolClass1)
    pMDNPSession->autoRequestBits = MDNPSESN_CLASS1_AUTO_ENABLE;
  if(pConfig->autoEnableUnsolClass2)
    pMDNPSession->autoRequestBits |= MDNPSESN_CLASS2_AUTO_ENABLE;
  if(pConfig->autoEnableUnsolClass3)
    pMDNPSession->autoRequestBits |= MDNPSESN_CLASS3_AUTO_ENABLE;
   
  pMDNPSession->defaultResponseTimeout = pConfig->defaultResponseTimeout;
  pMDNPSession->selOpDelayTime         = pConfig->selOpDelayTime;
  
  pDNPSession->linkStatusTimeoutDisconnect = pConfig->linkStatusTimeoutDisconnect;
  pDNPSession->linkStatusPeriod = pConfig->linkStatusPeriod;
  if(pDNPSession->linkStatusPeriod != 0)
    dnplink_startLinkStatusTimer(pSession);

  pDNPSession->readTimeoutsAllowed = pConfig->readTimeoutsAllowed;
  pMDNPSession->dnp.tmw.sesnDiagMask = pConfig->sesnDiagMask;

#if MDNPDATA_SUPPORT_OBJ91
  pMDNPSession->combineActConfigData = pConfig->combineActConfigData;
#endif
#if MDNPDATA_SUPPORT_OBJ70
  pMDNPSession->maxFileBlockSize     = pConfig->maxFileBlockSize;
#endif

#if MDNPDATA_SUPPORT_OBJ120 
  {
  DNPCHNL *pDNPChannel = (DNPCHNL *)pSession->pChannel;
  if(pConfig->authenticationEnabled)
  {
    /* If authentication was already enabled (for a modify) */
    if (pMDNPSession->pAuthenticationInfo != TMWDEFS_NULL)
    {
      /* If the version was changed from 2 to 5 or 5 to 2 close authentication */
      if (pMDNPSession->dnp.operateInV2Mode != pConfig->authConfig.operateInV2Mode)
      {
        mdnpauth_close(pSession);
        pMDNPSession->pAuthenticationInfo = TMWDEFS_NULL;
        pMDNPSession->authenticationEnabled = TMWDEFS_FALSE;
      }
    }

    pMDNPSession->dnp.operateInV2Mode = pConfig->authConfig.operateInV2Mode;
    if (pMDNPSession->pAuthenticationInfo == TMWDEFS_NULL)
    {
      pMDNPSession->dnp.saveLastTxFragment = TMWDEFS_TRUE;
#if DNPCNFG_SUPPORT_AUTHENTICATION && DNPCNFG_MULTI_SESSION_REQUESTS
      pMDNPSession->dnp.lastTxFragmentLength = 0;
#endif
      pMDNPSession->pAuthenticationInfo = mdnpauth_init(pSession, &pConfig->authConfig); 
      if(pMDNPSession->pAuthenticationInfo == TMWDEFS_NULL)
      {
        return(TMWDEFS_FALSE);
      }
    }

    mdnpauth_setConfig(pSession, &pConfig->authConfig);  
    pDNPChannel->directNoAckDelayTime = pConfig->authConfig.directNoAckDelayTime;
  }
  else
  {
    pMDNPSession->dnp.saveLastTxFragment = TMWDEFS_FALSE;
    /* If secure authentication was enabled, clean up. */
    if (pMDNPSession->pAuthenticationInfo != TMWDEFS_NULL)
    {
      mdnpauth_close(pSession);
      pMDNPSession->pAuthenticationInfo = TMWDEFS_NULL;
    }
  }
  pMDNPSession->authenticationEnabled = pConfig->authenticationEnabled;
  }
#endif

  return(TMWDEFS_TRUE);
}
 
/* function: mdnpsesn_closeSession */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpsesn_closeSession(
  TMWSESN *pSession)
{
  TMWCHNL *pChannel;
  MDNPSESN *pMDNPSession = (MDNPSESN *)pSession;

  /* Check for NULL since this would be a common error */
  if(pSession == TMWDEFS_NULL)
  {
    return(TMWDEFS_FALSE);
  }
  
  pChannel = (TMWCHNL *)pSession->pChannel;

  /* Lock channel */
  TMWTARG_LOCK_SECTION(&pChannel->lock);


  /* remove any outstanding requests from the channel */
  dnpchnl_deleteFragments(pSession);

#if MDNPDATA_SUPPORT_OBJ120  
  if(pMDNPSession->authenticationEnabled)
  {
    pMDNPSession->authenticationEnabled = TMWDEFS_FALSE;
    mdnpauth_close(pSession);   
    pMDNPSession->pAuthenticationInfo = TMWDEFS_NULL;
  }
#endif
  
  tmwtimer_cancel(&pMDNPSession->selOpDelayTimer);

  /* Close generic DNP session */
  dnpsesn_closeSession(pSession);

  /* Close database */
  mdnpdata_close(pMDNPSession->pDbHandle);

#if MDNPDATA_SUPPORT_OBJ70
  if(pMDNPSession->pFileXferContext != TMWDEFS_NULL)
  {
    mdnpmem_free(pMDNPSession->pFileXferContext);
  }
#endif
  
  /* Free memory */
  mdnpmem_free(pMDNPSession);

  /* Unlock channel */
  TMWTARG_UNLOCK_SECTION(&pChannel->lock);

  return(TMWDEFS_TRUE);
}

/* function: mdnpsesn_processFragment */
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpsesn_processFragment(
  TMWSESN *pSession,
  TMWSESN_RX_DATA *pRxFragment)
{
  MDNPSESN *pMDNPSession = (MDNPSESN*)pSession;
  TMWTYPES_UCHAR funcCode;
  int index;
 
  /* Protect against badly formatted message */
  if (pRxFragment->msgLength < 4)
  {
    DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_INVALID_SIZE);
    return(TMWDEFS_FALSE);
  }
  
#if MDNPDATA_SUPPORT_OBJ120 
   if(pMDNPSession->authenticationEnabled)
   {
     pRxFragment = mdnpauth_processing(pSession, MDNPAUTH_EVT_MSG_RECEIVED, 0, pRxFragment); 
     if(pRxFragment == TMWDEFS_NULL)
       return TMWDEFS_TRUE;
   }
#endif

   /* reset this to verify CTO received in this fragment */
   ((MDNPSESN *)pSession)->lastCTOReceived.year = 0;

  /* Get function code from message header */
  funcCode = pRxFragment->pMsgBuf[DNPDEFS_AH_INDEX_FUNC_CODE];

  /* Loop through the function code support table to see if this
   *  function code is supported.
   */
  index = 0;
  while(mdnpsesn_funcTable[index].pFunc != TMWDEFS_NULL)
  {
    if(funcCode == mdnpsesn_funcTable[index].funcCode)
    {
      if (funcCode == DNPDEFS_FC_RESPONSE)
      {
        TMWCHNL *pChannel = pSession->pChannel;

        TMWSESN_TX_DATA *pCurrentMessage = dnputil_getCurrentMessage(pSession);
        if ((pCurrentMessage != TMWDEFS_NULL)
          && (pCurrentMessage->pSession == pSession))
        { 
          /* Only cancel the incremental timer if this is the response to the last request sent on this session.
           * Dont let delayed responses on this or other sessions on this channel cancel the incremental timer
           * running for the most recent request.
           */
          TMWTYPES_UCHAR sequenceNumber = (TMWTYPES_UCHAR)(pRxFragment->pMsgBuf[DNPDEFS_AH_INDEX_APPL_CTRL] & 0xf);
          if (pMDNPSession->reqSequenceNumber == sequenceNumber)
          {
            tmwtimer_cancel(&pChannel->incrementalTimer);
          }
        }
      }

      /* Call the function code specific processing routine */
      mdnpsesn_funcTable[index].pFunc(pSession, pRxFragment);
      
#if MDNPDATA_SUPPORT_OBJ120 
      if (dnputil_getCurrentMessage(pSession) == TMWDEFS_NULL)
        mdnpauth_checkForQueuedAuthEvent(pSession);
#endif

      return(TMWDEFS_TRUE);
    }

    index += 1;
  } 
        
  DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_UNSUP_FC);
  return(TMWDEFS_FALSE);
}

/* function: mdnpsesn_sendConfirmation */
void TMWDEFS_GLOBAL mdnpsesn_sendConfirmation(
  TMWSESN *pSession, 
  TMWTYPES_UCHAR seqNumber,
  TMWTYPES_BOOL unsol)
{
  if(unsol)
  {
    seqNumber = (TMWTYPES_UCHAR)
      ((seqNumber & DNPDEFS_AC_SEQUENCE_MASK) | DNPDEFS_AC_UNSOLICITED);
  }

  _transmitConfirm(pSession, seqNumber, pSession->destAddress);
}

/* function: mdnpsesn_getSequenceNumber */
TMWTYPES_UCHAR TMWDEFS_GLOBAL mdnpsesn_getSequenceNumber(
  TMWSESN *pSession)
{
  MDNPSESN *pMDNPSession = (MDNPSESN *)pSession;
  return(pMDNPSession->reqSequenceNumber);
}

/* function: mdnpsesn_setSequenceNumber */
void TMWDEFS_GLOBAL mdnpsesn_setSequenceNumber(
  TMWSESN *pSession, 
  TMWTYPES_UCHAR seqNumber)
{
  MDNPSESN *pMDNPSession = (MDNPSESN *)pSession;
  pMDNPSession->reqSequenceNumber = (TMWTYPES_UCHAR)(seqNumber & 0x0f);
}

/* function: mdnpsesn_setUnsolUserCallback */
void TMWDEFS_GLOBAL mdnpsesn_setUnsolUserCallback(
  TMWSESN *pSession,
  MDNPSESN_UNSOL_CALLBACK_FUNC pCallback,
  void *pCallbackParam)
{
  MDNPSESN *pMDNPSession = (MDNPSESN *)pSession;
  pMDNPSession->pUnsolUserCallback = pCallback;
  pMDNPSession->pUnsolUserCallbackParam = pCallbackParam;
}

/* function: mdnpsesn_cancelSelOpDelayTimer */
TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpsesn_cancelSelOpDelayTimer(
  TMWSESN *pSession)
{ 
  MDNPSESN *pMDNPSession = (MDNPSESN *)pSession;
  if(tmwtimer_isActive(&pMDNPSession->selOpDelayTimer))
  {
    tmwtimer_cancel(&pMDNPSession->selOpDelayTimer);
    _selOpDelayTimeout(pSession);
  }
  return TMWDEFS_TRUE;
}

#if MDNPCNFG_SUPPORT_SA_VERSION5
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpsesn_addAuthUser( 
  TMWSESN *pSession,
  TMWTYPES_USHORT userNumber)
{
  MDNPSESN *pMDNPSession = (MDNPSESN*)pSession;
#if TMWCNFG_SUPPORT_THREADS
  TMWDEFS_RESOURCE_LOCK *pLock = &pSession->pChannel->lock;
#endif
  TMWTYPES_BOOL status = TMWDEFS_FALSE;

  TMWTARG_LOCK_SECTION(pLock);
  if(pMDNPSession->pAuthenticationInfo != TMWDEFS_NULL)
  {
    status = mdnpsa_addUser(pMDNPSession->pAuthenticationInfo, userNumber);
  } 
    
  TMWTARG_UNLOCK_SECTION(pLock);
  return status;
}
 
TMWTYPES_USHORT TMWDEFS_GLOBAL mdnpsesn_getAuthUser( 
  TMWSESN *pSession,
  TMWTYPES_USHORT index)
{
  MDNPSESN *pMDNPSession = (MDNPSESN*)pSession;
  if(pMDNPSession->pAuthenticationInfo != TMWDEFS_NULL)
    return mdnpsa_getUser(pMDNPSession->pAuthenticationInfo, index);
  else
    return 0;
}
TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpsesn_removeAuthUser( 
  TMWSESN *pSession,
  TMWTYPES_USHORT userNumber)
{
  MDNPSESN *pMDNPSession = (MDNPSESN*)pSession;
#if TMWCNFG_SUPPORT_THREADS
  TMWDEFS_RESOURCE_LOCK *pLock = &pSession->pChannel->lock;
#endif
  TMWTYPES_BOOL status = TMWDEFS_FALSE;

  TMWTARG_LOCK_SECTION(pLock);
  if(pMDNPSession->pAuthenticationInfo != TMWDEFS_NULL)
  {
    status = mdnpsa_removeUser(pMDNPSession->pAuthenticationInfo, userNumber); 
  }  
    
  TMWTARG_UNLOCK_SECTION(pLock);
  return status;
}

/* function: mdnpsesn_setSAUserCallback */
void TMWDEFS_GLOBAL mdnpsesn_setSAUserCallback(
  TMWSESN *pSession,
  MDNPSESN_SA_CALLBACK_FUNC pCallback,
  void *pCallbackParam)
{
  MDNPSESN *pMDNPSession = (MDNPSESN *)pSession;
  pMDNPSession->pSAUserCallback = pCallback;
  pMDNPSession->pSAUserCallbackParam = pCallbackParam;
}

/* function: mdnpsesn_getAuthCSQ */
TMWTYPES_ULONG TMWDEFS_GLOBAL mdnpsesn_getAuthCSQ(
  TMWSESN *pSession)
{
  MDNPSESN *pMDNPSession = (MDNPSESN *)pSession;
  if(pMDNPSession->pAuthenticationInfo != TMWDEFS_NULL)
    return pMDNPSession->pAuthenticationInfo->challengeSequenceNumber;

  return 0;
}
 
/* function: mdnpsesn_setAuthCSQ */
void TMWDEFS_GLOBAL mdnpsesn_setAuthCSQ(
  TMWSESN *pSession,
  TMWTYPES_ULONG csq)
{
  MDNPSESN *pMDNPSession = (MDNPSESN *)pSession;
  if (pMDNPSession->pAuthenticationInfo != TMWDEFS_NULL)
    pMDNPSession->pAuthenticationInfo->challengeSequenceNumber = csq;
}

/* function: mdnpsesn_getAuthAggrCSQ */
TMWTYPES_ULONG TMWDEFS_GLOBAL mdnpsesn_getAuthAggrCSQ(
  TMWSESN *pSession)
{
  MDNPSESN *pMDNPSession = (MDNPSESN *)pSession;
  if (pMDNPSession->pAuthenticationInfo != TMWDEFS_NULL)
    return pMDNPSession->pAuthenticationInfo->aggressiveModeTxSequence;

  return 0;
}

/* function: mdnpsesn_setAuthAggrCSQ */
void TMWDEFS_GLOBAL mdnpsesn_setAuthAggrCSQ(
  TMWSESN *pSession,
  TMWTYPES_ULONG csq)
{
  MDNPSESN *pMDNPSession = (MDNPSESN *)pSession;
  if (pMDNPSession->pAuthenticationInfo != TMWDEFS_NULL)
    pMDNPSession->pAuthenticationInfo->aggressiveModeTxSequence = csq;
}
#endif

#if DNPCNFG_SUPPORT_BINCONFIG
TMWTYPES_BOOL mdnpsesn_getBinFileSessionValues(
   MDNPSESN_CONFIG *pSesnConfig,
   DNPBNCFG_FILEVALUES *pBinFileValues,
   TMWTYPES_BOOL isChannelSerial)
{

  if(pBinFileValues->isOutStation)
  {
    /* if the bin file is for an outstation, then just configure to talk to it.*/
    /* Destination address (outstation address) for this session */
    if(pBinFileValues->fieldsUsed1_4 & CRTL_DATALINKADDRESS1_4_1)
      pSesnConfig->destination = (TMWTYPES_USHORT)pBinFileValues->dataLinkAddress1_4;

    if(pBinFileValues->fieldsUsed1_4 & CRTL_SOURCEADDRSEXPECTEDWITHVAL1_4_3)
      pSesnConfig->source = (TMWTYPES_USHORT)pBinFileValues->dnp3SourceAddrs1_4;
     
#if MDNPDATA_SUPPORT_OBJ120
    /* section 1.12.1 */
    if(pBinFileValues->fieldsUsed1_12 & CRTL_SECUREAUTHENTICATIONSUPPORTED1_12_1)
    {
      if(pBinFileValues->specialControls1_12 & SPCTRL_NOSECUREAUTHENTICATIONSUPPORT1_12_1)
        pSesnConfig->authenticationEnabled = TMWDEFS_FALSE;
      else
     {
       pSesnConfig->authenticationEnabled = TMWDEFS_TRUE;
       
       if(pBinFileValues->versionSecureAuthSupported1_12 == 2)
         pSesnConfig->authConfig.operateInV2Mode = TMWDEFS_TRUE;
      }
    }
    
    if (pSesnConfig->authenticationEnabled == TMWDEFS_TRUE)
    {
      /* section 1.12.4 */
      if(pBinFileValues->fieldsUsed1_12 & CRTL_ACCEPTSAGGRESSIVEMODE1_12_4)
      {
        if(pBinFileValues->specialControls1_12 & SPCTRL_YESACCEPTSAGGRESSIVEMODE1_12_4)
          pSesnConfig->authConfig.aggressiveModeSupport = TMWDEFS_TRUE;
        else
          pSesnConfig->authConfig.aggressiveModeSupport = TMWDEFS_FALSE;
      }

      /* section 1.12.6 */
      if(pBinFileValues->fieldsUsed1_12 & CRTL_SESSIONKEYCHANGEINTERVAL1_12_6)
      {
        if(pBinFileValues->specialControls1_12 & SPCTRL_DISABLEDSESSIONKEYCHANGEINTERVAL1_12_6)
          pSesnConfig->authConfig.keyChangeInterval = 0;
        else
          /* master should be half of outstation value */
          pSesnConfig->authConfig.keyChangeInterval = pBinFileValues->sessionKeyChangeInterval1_12/2;
      }

      /* section 1.12.7 */
      if(pBinFileValues->fieldsUsed1_12 & CRTL_SESSIONKEYCHANGEMESSAGECOUNT1_12_7)
        /* master should be half of outstation value */
        pSesnConfig->authConfig.maxKeyChangeCount = (TMWTYPES_USHORT)(TMWTYPES_USHORT)pBinFileValues->sessionKeyChangeMessageCnt1_12/2;
    }
  #endif
  }
  else
  {
    /*if the bin file is for a master, then configure as much as possible from values.*/

    /*1.2.5*/
    if(isChannelSerial && (pBinFileValues->fieldsUsed1_2 & CRTL_LINKSTATUSINT1_2_5))
      pSesnConfig->linkStatusPeriod = pBinFileValues->linkStatusInterval1_2;
    
    /*1.3.10*/
    if(!isChannelSerial && (pBinFileValues->fieldsUsed1_3 & CRTL_TCPKEEPALIVETIMER1_3_10))
      pSesnConfig->linkStatusPeriod = pBinFileValues->tcpKeepAliveTimer1_3;

    /*1.4.1*/
    if(pBinFileValues->fieldsUsed1_4 & CRTL_DATALINKADDRESS1_4_1)
      pSesnConfig->source = (TMWTYPES_USHORT)pBinFileValues->dataLinkAddress1_4;

    /*1.4.3*/
    if(pBinFileValues->fieldsUsed1_4 & CRTL_DATALINKADDRESS1_4_1)
      pSesnConfig->destination = (TMWTYPES_USHORT)pBinFileValues->dnp3SourceAddrs1_4;

    /*1.6.1*/
    if(pBinFileValues->fieldsUsed1_6 & CRTL_APPLLAYERCOMPLETERESPONSETIMEOUT1_6_1)
      pSesnConfig->defaultResponseTimeout = pBinFileValues->applLayerCompleteRespTimeout1_6;

#if MDNPDATA_SUPPORT_OBJ120
    /*1.12.1*/
    if(pBinFileValues->fieldsUsed1_12 & CRTL_SECUREAUTHENTICATIONSUPPORTED1_12_1)
    {
      if(pBinFileValues->specialControls1_12 & SPCTRL_NOSECUREAUTHENTICATIONSUPPORT1_12_1)
        pSesnConfig->authenticationEnabled = TMWDEFS_FALSE;
      else
      {
       pSesnConfig->authenticationEnabled = TMWDEFS_TRUE;
       
       if(pBinFileValues->versionSecureAuthSupported1_12 == 2)
         pSesnConfig->authConfig.operateInV2Mode = TMWDEFS_TRUE;
      }
    }

    if (pSesnConfig->authenticationEnabled == TMWDEFS_TRUE)
    {
      /* section 1.12.3 */
      if(pBinFileValues->fieldsUsed1_12 & CRTL_SECURITYRESPONSETIMEOUT1_12_3)
        pSesnConfig->authConfig.responseTimeout = pBinFileValues->securityResponseTimeout1_12;


      /* section 1.12.4 */
      if(pBinFileValues->fieldsUsed1_12 & CRTL_ACCEPTSAGGRESSIVEMODE1_12_4)
      {
        if(pBinFileValues->specialControls1_12 & SPCTRL_YESACCEPTSAGGRESSIVEMODE1_12_4)
          pSesnConfig->authConfig.aggressiveModeSupport = TMWDEFS_TRUE;
        else
          pSesnConfig->authConfig.aggressiveModeSupport = TMWDEFS_FALSE;
      }

      /* section 1.12.6 */
      if(pBinFileValues->fieldsUsed1_12 & CRTL_SESSIONKEYCHANGEINTERVAL1_12_6)
      {
        if(pBinFileValues->specialControls1_12 & SPCTRL_DISABLEDSESSIONKEYCHANGEINTERVAL1_12_6)
          pSesnConfig->authConfig.keyChangeInterval = 0;
        else
          pSesnConfig->authConfig.keyChangeInterval = pBinFileValues->sessionKeyChangeInterval1_12;

      }

      /* section 1.12.7 */
      if(pBinFileValues->fieldsUsed1_12 & CRTL_SESSIONKEYCHANGEMESSAGECOUNT1_12_7)
        pSesnConfig->authConfig.maxKeyChangeCount = (TMWTYPES_USHORT)pBinFileValues->sessionKeyChangeMessageCnt1_12;

      /* section 1.12.8 */
#if MDNPCNFG_SUPPORT_SA_VERSION2
      if(pBinFileValues->fieldsUsed1_12 & CRTL_MAXERRORCOUNT1_12_8)
        pSesnConfig->authConfig.maxErrorCount  = (TMWTYPES_UCHAR)pBinFileValues->maxErrorCount1_12;
#endif
#if MDNPCNFG_SUPPORT_SA_VERSION5
      if(pBinFileValues->fieldsUsed1_12 & CRTL_MAXERRORCOUNT1_12_8)
        pSesnConfig->authConfig.maxErrorMessagesSent = (TMWTYPES_USHORT)pBinFileValues->maxErrorCount1_12;
#endif

      /* section 1.12.9 */
      if(pBinFileValues->fieldsUsed1_12 & CRTL_HMACALGORITHMREQUESTED1_12_9)
      {
        if(pBinFileValues->hmacAlgorithmRequested1_12 & MACALGORITHMS_SHA1TRUNCATED4)
          pSesnConfig->authConfig.MACAlgorithm = DNPAUTH_HMAC_SHA1_4OCTET;
        else if(pBinFileValues->hmacAlgorithmRequested1_12 & MACALGORITHMS_SHA1TRUNCATED8)
          pSesnConfig->authConfig.MACAlgorithm = DNPAUTH_MAC_SHA1_8OCTET;
        else if(pBinFileValues->hmacAlgorithmRequested1_12 & MACALGORITHMS_SHA1TRUNCATED10)
          pSesnConfig->authConfig.MACAlgorithm = DNPAUTH_MAC_SHA1_10OCTET;
        else if(pBinFileValues->hmacAlgorithmRequested1_12 & MACALGORITHMS_SHA256TRUNCATED8)
          pSesnConfig->authConfig.MACAlgorithm = DNPAUTH_MAC_SHA256_8OCTET;
        else if(pBinFileValues->hmacAlgorithmRequested1_12 & MACALGORITHMS_SHA256TRUNCATED16)
          pSesnConfig->authConfig.MACAlgorithm = DNPAUTH_MAC_SHA256_16OCTET;
        else if(pBinFileValues->hmacAlgorithmRequested1_12 & MACALGORITHMS_AESGMAC)
          pSesnConfig->authConfig.MACAlgorithm = DNPAUTH_MAC_AESGMAC_12OCTET;
        /*else
          pSesnConfig->authConfig.MACAlgorithm = MACALGORITHMS_OTHER; */
      }
    }
  
#endif

  }

  return TMWDEFS_TRUE;
}
#endif
