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

/* file: sdnpsesn.h
 * description: Implement a DNP Slave session
 */
#include "tmwscl/dnp/dnpdiag.h"
#include "tmwscl/dnp/sdnpdiag.h"
#include "tmwscl/utils/tmwtarg.h"

#include "tmwscl/dnp/dnpdefs.h"
#include "tmwscl/dnp/dnpchnl.h"
#include "tmwscl/dnp/dnpbncfg.h"
#include "tmwscl/dnp/dnpdtime.h"

#include "tmwscl/dnp/sdnpmem.h"
#include "tmwscl/dnp/sdnprbe.h"
#include "tmwscl/dnp/sdnpsesn.h"
#include "tmwscl/dnp/sdnpunsl.h"
#include "tmwscl/dnp/sdnpdata.h"
#include "tmwscl/dnp/sdnpunsl.h"
#include "tmwscl/dnp/sdnputil.h"

#include "tmwscl/dnp/sdnpo000.h"
#include "tmwscl/dnp/sdnpo001.h"
#include "tmwscl/dnp/sdnpo002.h"
#include "tmwscl/dnp/sdnpo003.h"
#include "tmwscl/dnp/sdnpo004.h"
#include "tmwscl/dnp/sdnpo010.h"
#include "tmwscl/dnp/sdnpo011.h"
#include "tmwscl/dnp/sdnpo012.h"
#include "tmwscl/dnp/sdnpo013.h"
#include "tmwscl/dnp/sdnpo020.h"
#include "tmwscl/dnp/sdnpo021.h"
#include "tmwscl/dnp/sdnpo022.h"
#include "tmwscl/dnp/sdnpo023.h"
#include "tmwscl/dnp/sdnpo030.h"
#include "tmwscl/dnp/sdnpo031.h"
#include "tmwscl/dnp/sdnpo032.h"
#include "tmwscl/dnp/sdnpo033.h"
#include "tmwscl/dnp/sdnpo034.h"
#include "tmwscl/dnp/sdnpo040.h"
#include "tmwscl/dnp/sdnpo041.h"
#include "tmwscl/dnp/sdnpo042.h"
#include "tmwscl/dnp/sdnpo043.h"
#include "tmwscl/dnp/sdnpo050.h"
#include "tmwscl/dnp/sdnpo060.h"
#include "tmwscl/dnp/sdnpo070.h"
#include "tmwscl/dnp/sdnpo080.h"
#include "tmwscl/dnp/sdnpo085.h"
#include "tmwscl/dnp/sdnpo086.h"
#include "tmwscl/dnp/sdnpo087.h"
#include "tmwscl/dnp/sdnpo088.h"
#include "tmwscl/dnp/sdnpo110.h"
#include "tmwscl/dnp/sdnpo111.h"
#include "tmwscl/dnp/sdnpo112.h"
#include "tmwscl/dnp/sdnpo113.h"
#include "tmwscl/dnp/sdnpo114.h"
#include "tmwscl/dnp/sdnpo115.h"
#include "tmwscl/dnp/sdnpo120.h"
#if DNPCNFG_SUPPORT_AUTHENTICATION
#if SDNPCNFG_SUPPORT_SA_VERSION5
#include "tmwscl/dnp/sdnpsa.h"  
#include "tmwscl/dnp/sdnpo121.h"
#include "tmwscl/dnp/sdnpo122.h"
#endif
#include "tmwscl/dnp/sdnpauth.h"  
#endif

/* Forward declarations */
static void TMWDEFS_LOCAL _processConfirm(
  TMWSESN *pSession,
  TMWSESN_RX_DATA *pRxData);

static void TMWDEFS_LOCAL _processReadRequest(
  TMWSESN *pSession);

static void TMWDEFS_LOCAL _processWriteRequest(
  TMWSESN *pSession);

static void TMWDEFS_LOCAL _processSelectRequest(
  TMWSESN *pSession);

static void TMWDEFS_LOCAL _processOperateRequest(
  TMWSESN *pSession);

static void TMWDEFS_LOCAL _processDirectOperateRequest(
  TMWSESN *pSession);

static void TMWDEFS_LOCAL _processFreezeRequest(
  TMWSESN *pSession);

static void TMWDEFS_LOCAL _processRestartRequest(
  TMWSESN *pSession);

static void TMWDEFS_LOCAL _processEnableUnsolRequest(
  TMWSESN *pSession);

static void TMWDEFS_LOCAL _processDisableUnsolRequest(
  TMWSESN *pSession);

#if SDNPDATA_SUPPORT_ASSIGN
static void TMWDEFS_LOCAL _processAssignClassRequest(
  TMWSESN *pSession);
#endif

static void TMWDEFS_LOCAL _processRecordTimeRequest(
  TMWSESN *pSession);

#if SDNPDATA_SUPPORT_OBJ50_V1
static void TMWDEFS_LOCAL _processDelayRequest(
  TMWSESN *pSession);
#endif

#if SDNPDATA_SUPPORT_OBJ70
static void TMWDEFS_LOCAL _processFileRequest(
  TMWSESN *pSession);
#endif

#if SDNPDATA_SUPPORT_OBJ91
static void TMWDEFS_LOCAL _processActConfigRequest(
  TMWSESN *pSession);
#endif

static void TMWDEFS_CALLBACK _processNextMessage(
  TMWSESN *pSession);

/* Define and initialize a table that specifies which function codes are supported
 * and the processing routine to be used to process them.
 */
typedef void (*SDNPSESN_FC_SUPPORT_FUNC)(
  TMWSESN *pSession);

typedef struct SDNPSessionFuncEntryStruct {
  TMWTYPES_UCHAR funcCode;
  SDNPSESN_FC_SUPPORT_FUNC pFunc;
} SDNPSESN_FC_FUNC_ENTRY;

static const SDNPSESN_FC_FUNC_ENTRY sdnpsesn_funcTable[] = {
  /* DNP Slave functionality */
  {DNPDEFS_FC_READ,                _processReadRequest},
  {DNPDEFS_FC_WRITE,               _processWriteRequest},
  {DNPDEFS_FC_SELECT,              _processSelectRequest},
  {DNPDEFS_FC_OPERATE,             _processOperateRequest},
  {DNPDEFS_FC_DIRECT_OP,           _processDirectOperateRequest},
  {DNPDEFS_FC_DIRECT_OP_NOACK,     _processDirectOperateRequest},
#if SDNPDATA_SUPPORT_OBJ20
  {DNPDEFS_FC_FRZ,                 _processFreezeRequest},
  {DNPDEFS_FC_FRZ_NOACK,           _processFreezeRequest},
  {DNPDEFS_FC_FRZ_CLEAR,           _processFreezeRequest},
  {DNPDEFS_FC_FRZ_CLEAR_NOACK,     _processFreezeRequest},
  {DNPDEFS_FC_FRZ_TIME,            _processFreezeRequest},
  {DNPDEFS_FC_FRZ_TIME_NOACK,      _processFreezeRequest},
#endif
  {DNPDEFS_FC_COLD_RESTART,        _processRestartRequest},
  {DNPDEFS_FC_WARM_RESTART,        _processRestartRequest},
#ifdef TMW_FUTURE
  {DNPDEFS_FC_INIT_DATA_TO_DFLTS,  _processInitDataRequest},
  {DNPDEFS_FC_INIT_APPLICATION,    _processInitAppRequest},
  {DNPDEFS_FC_START_APPLICATION,   _processStartAppRequest},
  {DNPDEFS_FC_STOP_APPLICATION,    _processStopAppRequest},
  {DNPDEFS_FC_SAVE_CONFIGURATION,  _processSaveConfigRequest},
#endif
  {DNPDEFS_FC_ENABLE_UNSOL,        _processEnableUnsolRequest},
  {DNPDEFS_FC_DISABLE_UNSOL,       _processDisableUnsolRequest},
#if SDNPDATA_SUPPORT_ASSIGN
  {DNPDEFS_FC_ASSIGN_CLASS,        _processAssignClassRequest},
#endif
#if SDNPDATA_SUPPORT_OBJ50_V1
  {DNPDEFS_FC_DELAY_MEASURE,       _processDelayRequest},
#endif
  {DNPDEFS_FC_RECORD_CURRENT_TIME, _processRecordTimeRequest},
#if SDNPDATA_SUPPORT_OBJ70
  {DNPDEFS_FC_OPEN_FILE,           _processFileRequest},
  {DNPDEFS_FC_CLOSE_FILE,          _processFileRequest},
  {DNPDEFS_FC_DELETE_FILE,         _processFileRequest},
  {DNPDEFS_FC_GET_FILE_INFO,       _processFileRequest},
  {DNPDEFS_FC_AUTHENTICATE,        _processFileRequest},
  {DNPDEFS_FC_ABORT,               _processFileRequest},
#endif
#if SDNPDATA_SUPPORT_OBJ91
  {DNPDEFS_FC_ACTIVATE_CONFIG,     _processActConfigRequest},
#endif
  {0,                              TMWDEFS_NULL}
};

/* Define a table that assigns a function to handle read requests
 * for each supported object group and variation. This table will
 * be used to dispatch read requests to the appropriate handler
 * based on the objects group and variation.
 */
typedef struct {
  TMWTYPES_UCHAR group;          /* Object group */
  TMWTYPES_UCHAR variation;      /* Object variation */
  TMWTYPES_UCHAR readPass;       /* Which read pass */
                                 /* 0 = events (including class 1, 2, and 3) */
                                 /* 1 = actually reads class 1, 2, and 3 so */
                                 /* it should not appear in this table */
                                 /* 2 = static data (excluding class 0) */
                                 /* 3 = class 0 data poll */
  TMWTYPES_BOOL allVariations;   /* TMWDEFS_TRUE if this function should be */
                                 /* for all variations of this object */
  SDNPSESN_READ_FUNC pReadFunc;  /* Read function */
} SDNPSESN_OBJ_READ_FUNC_ENTRY;

/* Grp Var  Pass  AllVariations   Read Function*/
static const SDNPSESN_OBJ_READ_FUNC_ENTRY _sdnpObjReadFuncTable[] = {
#if SDNPDATA_SUPPORT_OBJ0
  {0,  0,  2,    TMWDEFS_TRUE,  sdnpo000_readObj0},    /* Device Attributes */
#endif
#if SDNPDATA_SUPPORT_OBJ1
  {1,  0,  2,    TMWDEFS_TRUE,  sdnpo001_readObj1},    /* Binary Inputs */
#endif
#if SDNPDATA_SUPPORT_OBJ2
  {2,  0,  0,    TMWDEFS_TRUE,  sdnpo002_readObj2},    /* Binary Input Change Events */
#endif
#if SDNPDATA_SUPPORT_OBJ3
  {3,  0,  2,    TMWDEFS_TRUE,  sdnpo003_readObj3},    /* Double Bit Inputs */
#endif
#if SDNPDATA_SUPPORT_OBJ4
  {4,  0,  0,    TMWDEFS_TRUE,  sdnpo004_readObj4},    /* Double Bit Input Change Events */
#endif
#if SDNPDATA_SUPPORT_OBJ10
  {10, 0,  2,    TMWDEFS_TRUE,  sdnpo010_readObj10},   /* Binary Outputs */
#endif
#if SDNPDATA_SUPPORT_OBJ11
  {11, 0,  0,    TMWDEFS_TRUE,  sdnpo011_readObj11},   /* Binary Output Events */
#endif
#if SDNPDATA_SUPPORT_OBJ13
  {13, 0,  0,    TMWDEFS_TRUE,  sdnpo013_readObj13},   /* Binary Output Command Events */
#endif
#if SDNPDATA_SUPPORT_OBJ20
  {20, 0,  2,    TMWDEFS_TRUE,  sdnpo020_readObj20},   /* Binary Counters */
#endif
#if SDNPDATA_SUPPORT_OBJ21
  {21, 0,  2,    TMWDEFS_TRUE,  sdnpo021_readObj21},   /* Frozen Counters */
#endif
#if SDNPDATA_SUPPORT_OBJ22
  {22, 0,  0,    TMWDEFS_TRUE,  sdnpo022_readObj22},   /* Binary Counter Change Events */
#endif
#if SDNPDATA_SUPPORT_OBJ23
  {23, 0,  0,    TMWDEFS_TRUE,  sdnpo023_readObj23},   /* Frozen Counter Change Events */
#endif
#if SDNPDATA_SUPPORT_OBJ30
  {30, 0,  2,    TMWDEFS_TRUE,  sdnpo030_readObj30},   /* Analog Inputs */
#endif
#if SDNPDATA_SUPPORT_OBJ31
  {31, 0,  2,    TMWDEFS_TRUE,  sdnpo031_readObj31},   /* Frozen Analog Inputs */
#endif
#if SDNPDATA_SUPPORT_OBJ32
  {32, 0,  0,    TMWDEFS_TRUE,  sdnpo032_readObj32},   /* Analog Change Events */
#endif
#if SDNPDATA_SUPPORT_OBJ33
  {33, 0,  0,    TMWDEFS_TRUE,  sdnpo033_readObj33},   /* Frozen Analog Input Change Events */
#endif
#if SDNPDATA_SUPPORT_OBJ34
  {34, 0,  2,    TMWDEFS_TRUE,  sdnpo034_readObj34},   /* Analog Input Deadband */
#endif
#if SDNPDATA_SUPPORT_OBJ40
  {40, 0,  2,    TMWDEFS_TRUE,  sdnpo040_readObj40},   /* Analog Output Status */
#endif
#if SDNPDATA_SUPPORT_OBJ42
  {42, 0,  0,    TMWDEFS_TRUE,  sdnpo042_readObj42},   /* Analog Output Events */
#endif
#if SDNPDATA_SUPPORT_OBJ43
  {43, 0,  0,    TMWDEFS_TRUE,  sdnpo043_readObj43},   /* Analog Output Command Events */
#endif
#if SDNPDATA_SUPPORT_OBJ50_V4
  {50, 0,  2,    TMWDEFS_FALSE,  sdnpo050_readObj50v4}, /* Indexed absolute time */
  {50, 4,  2,    TMWDEFS_FALSE,  sdnpo050_readObj50v4}, /* Indexed absolute time */
#endif
#if SDNPDATA_SUPPORT_OBJ50_V1  
  /* a read of variation 0 returning variation 1 was added in version 3.01.000 
   * it really should be used for variation 4. Leave this for backward compatibility
   * if variation 4 is not supported.
   */
  {50, 0,  2,    TMWDEFS_FALSE,  sdnpo050_readObj50v1}, /* Date and Time */
  {50, 1,  2,    TMWDEFS_FALSE,  sdnpo050_readObj50v1}, /* Date and Time */
#endif
#if SDNPDATA_SUPPORT_OBJ70
  {70, 0,  0,    TMWDEFS_TRUE,   sdnpo070_readObj70},   /* File transfer */
#endif
  {60, 1,  2,    TMWDEFS_FALSE,  sdnpo060_readObj60v1},   /* Class Data */
  {60, 2,  0,    TMWDEFS_FALSE,  sdnpo060_readObj60v2},
  {60, 3,  0,    TMWDEFS_FALSE,  sdnpo060_readObj60v3},
  {60, 4,  0,    TMWDEFS_FALSE,  sdnpo060_readObj60v4},
#if SDNPDATA_SUPPORT_OBJ80_READ
  {80, 1,  2,    TMWDEFS_FALSE,  sdnpo080_readObj80v1},   /* Internal Indications */
#endif
#if SDNPDATA_SUPPORT_OBJ85
  {85, 0,  2,    TMWDEFS_TRUE,   sdnpo085_readObj85},     /* Data Set Prototypes */
#endif
#if SDNPDATA_SUPPORT_OBJ86
  {86, 0,  2,    TMWDEFS_TRUE,   sdnpo086_readObj86},     /* Data Set Descriptors*/
#endif
#if SDNPDATA_SUPPORT_OBJ87
  {87, 0,  2,    TMWDEFS_TRUE,   sdnpo087_readObj87},     /* Data Set Present Value*/
#endif
#if SDNPDATA_SUPPORT_OBJ88
  {88, 0,  0,    TMWDEFS_TRUE,   sdnpo088_readObj88},     /* Data Set Snapshot Events*/
#endif
#if SDNPDATA_SUPPORT_OBJ110
  {110,1,  2,    TMWDEFS_TRUE,   sdnpo110_readObj110},    /* String Data */
#endif
#if SDNPDATA_SUPPORT_OBJ111
  {111,1,  0,    TMWDEFS_TRUE,   sdnpo111_readObj111},    /* String Change Events */
#endif
#if SDNPDATA_SUPPORT_OBJ113
  {113,1,  0,    TMWDEFS_TRUE,   sdnpo113_readObj113},    /* Virtual Terminal Events */
#endif
#if SDNPDATA_SUPPORT_OBJ114
  {114,1,  2,    TMWDEFS_TRUE,   sdnpo114_readObj114},    /* Extended String Data */
#endif
#if SDNPDATA_SUPPORT_OBJ115
  {115,1,  0,    TMWDEFS_TRUE,   sdnpo115_readObj115},    /* Extended String Change Events */
#endif
#if SDNPDATA_SUPPORT_OBJ120
#if SDNPCNFG_SUPPORT_SA_VERSION5 
  {121,1,  2,    TMWDEFS_TRUE,   sdnpo121_readObj121},    /* Security Statistics */
  {122,0,  0,    TMWDEFS_TRUE,   sdnpo122_readObj122},    /* Security Statistics Change Events */
#endif
#endif
  {0,  0,  0,    TMWDEFS_FALSE,  TMWDEFS_NULL}            /* Last entry */
};

/* Define a table that assigns a function to handle write requests.
 */
typedef struct {
  TMWTYPES_UCHAR group;           /* Object group */
  TMWTYPES_UCHAR variation;       /* Object variation */
  TMWTYPES_UCHAR dataLength;      /* length of data for checking bad message */
  TMWTYPES_BOOL allVariations;    /* TMWDEFS_TRUE if this function should be */
                                  /* for all variations of this object */
  SDNPSESN_WRITE_FUNC pWriteFunc; /* Write function */
} SDNPSESN_OBJ_WRITE_FUNC_ENTRY;

#define SDNP_NOCHECK ((TMWTYPES_UCHAR)-1)

/* Grp Var  AllVariations   Write */
static const SDNPSESN_OBJ_WRITE_FUNC_ENTRY _sdnpObjWriteFuncTable[] = { 
#if SDNPDATA_SUPPORT_OBJ0
  {0,  0,  SDNP_NOCHECK,  TMWDEFS_TRUE,   sdnpo000_writeObj00},     /* Device Attribute */
#endif
#if SDNPDATA_SUPPORT_OBJ10_WRITE
  {10,  1, SDNP_NOCHECK,  TMWDEFS_FALSE,  sdnpo010_writeObj10v1},   /* Binary Output */
#endif
#if SDNPDATA_SUPPORT_OBJ34_V1
  {34,  1, 2,             TMWDEFS_FALSE,  sdnpo034_writeObj34v1},   /* Analog Input Deadband */
#endif
#if SDNPDATA_SUPPORT_OBJ34_V2
  {34,  2, 4,             TMWDEFS_FALSE,  sdnpo034_writeObj34v2},
#endif
#if SDNPDATA_SUPPORT_OBJ34_V3
  {34,  3, 4,             TMWDEFS_FALSE,  sdnpo034_writeObj34v3},
#endif
#if SDNPDATA_SUPPORT_OBJ50_V1
  {50,  1, 6,             TMWDEFS_FALSE,  sdnpo050_writeObj50v1},
#endif
#if SDNPDATA_SUPPORT_OBJ50_V3
  {50,  3, 6,             TMWDEFS_FALSE,  sdnpo050_writeObj50v3},
#endif
#if SDNPDATA_SUPPORT_OBJ50_V4
  {50,  4, 11,             TMWDEFS_FALSE,  sdnpo050_writeObj50v4},
#endif
#if SDNPDATA_SUPPORT_OBJ70
  {70,  5, 10,            TMWDEFS_FALSE,  sdnpo070_writeObj70},
#endif
  {80,  1, 1,             TMWDEFS_FALSE,  sdnpo080_writeObj80v1},
#if SDNPDATA_SUPPORT_OBJ85
  {85,  1, SDNP_NOCHECK,  TMWDEFS_FALSE,  sdnpo085_writeObj085v1},
#endif 
#if SDNPDATA_SUPPORT_OBJ86_V1
  {86,  1, SDNP_NOCHECK,  TMWDEFS_FALSE,  sdnpo086_writeObj086v1},
#endif 
#if SDNPDATA_SUPPORT_OBJ86_V3
  {86,  3, SDNP_NOCHECK,  TMWDEFS_FALSE,  sdnpo086_writeObj086v3},
#endif 
#if SDNPDATA_SUPPORT_OBJ87
  {87,  1, SDNP_NOCHECK,  TMWDEFS_FALSE,  sdnpo087_writeObj087v1},
#endif 
#if SDNPDATA_SUPPORT_OBJ110
  {110, 1, SDNP_NOCHECK,  TMWDEFS_TRUE,   sdnpo110_writeObj110},
#endif
#if SDNPDATA_SUPPORT_OBJ112
  {112, 1, SDNP_NOCHECK,  TMWDEFS_TRUE,   sdnpo112_writeObj112},
#endif
#if SDNPDATA_SUPPORT_OBJ114
  {114, 1, SDNP_NOCHECK,  TMWDEFS_TRUE,   sdnpo114_writeObj114},
#endif
  {0,   0, SDNP_NOCHECK,  TMWDEFS_FALSE,  TMWDEFS_NULL}              /* Last entry */
};

/* Define a table that assigns a function to handle select and
 * operate requests.
 */
typedef struct {
  TMWTYPES_UCHAR group;                     /* Object group */
  TMWTYPES_UCHAR variation;                 /* Object variation */
  TMWTYPES_UCHAR dataLength;                /* length of data for checking bad message */
  SDNPSESN_INIT_FUNC pInitFunc;             /* Initialize response */
  SDNPSESN_SELECT_FUNC pSelectFunc;         /* Select function */
  SDNPSESN_OPERATE_FUNC pOperateFunc;       /* Operate function */
} SDNPSESN_OBJ_SELOP_FUNC_ENTRY;

static const SDNPSESN_OBJ_SELOP_FUNC_ENTRY _sdnpObjSelOpFuncTable[] = {
#if SDNPDATA_SUPPORT_OBJ12_V1
  {12,  1,  11,           TMWDEFS_NULL,         sdnpo012_selObj12v1, sdnpo012_opObj12v1},
#endif
#if SDNPDATA_SUPPORT_OBJ12_V2
  {12,  2,  11,           sdnpo012_initObj12v2, sdnpo012_selObj12v2, sdnpo012_opObj12v2},
#endif
#if SDNPDATA_SUPPORT_OBJ12_V3
  {12,  3, SDNP_NOCHECK,  TMWDEFS_NULL,         sdnpo012_selObj12v3, sdnpo012_opObj12v3},
#endif
#if SDNPDATA_SUPPORT_OBJ41_V1
  {41,  1,   5,           TMWDEFS_NULL,         sdnpo041_selObj41v1, sdnpo041_opObj41v1},
#endif
#if SDNPDATA_SUPPORT_OBJ41_V2
  {41,  2,   3,           TMWDEFS_NULL,         sdnpo041_selObj41v2, sdnpo041_opObj41v2},
#endif
#if SDNPDATA_SUPPORT_OBJ41_V3
  {41,  3,   5,           TMWDEFS_NULL,         sdnpo041_selObj41v3, sdnpo041_opObj41v3},
#endif
#if SDNPDATA_SUPPORT_OBJ41_V4
  {41,  4,   9,           TMWDEFS_NULL,         sdnpo041_selObj41v4, sdnpo041_opObj41v4},
#endif
#if SDNPDATA_SUPPORT_OBJ87
  {87,  1, SDNP_NOCHECK,  TMWDEFS_NULL,         sdnpo087_selObj087v1,sdnpo087_opObj087v1},
#endif 
  {0,   0,  0,  TMWDEFS_NULL,         TMWDEFS_NULL,        TMWDEFS_NULL}        /* Last entry */
};

#if SDNPDATA_SUPPORT_SELECT_CANCEL
/* Define a table that assigns a function to handle cancelling a select
 */
typedef struct {
  TMWTYPES_UCHAR group;                     /* Object group */
  SDNPSESN_CANCEL_FUNC pCancelSelectFunc;   /* Cancel Select function */
} SDNPSESN_OBJ_CANSEL_FUNC_ENTRY;

static const SDNPSESN_OBJ_CANSEL_FUNC_ENTRY _sdnpObjCanSelFuncTable[] = {
#if SDNPDATA_SUPPORT_OBJ12
  {12, sdnpo012_cancelSelect},
#endif
#if SDNPDATA_SUPPORT_OBJ41
  {41, sdnpo041_cancelSelect},
#endif
#if SDNPDATA_SUPPORT_OBJ87
  {87, sdnpo087_cancelSelect},
#endif 
  {0, TMWDEFS_NULL}        /* Last entry */
};
#endif

#if SDNPDATA_SUPPORT_OBJ21 || SDNPDATA_SUPPORT_OBJ31
/* Define a table that assigns a function to handle freeze requests.
 */
typedef struct {
  TMWTYPES_UCHAR group;              /* Object group */
  TMWTYPES_UCHAR variation;          /* Object variation */
  SDNPSESN_FREEZE_FUNC pFreezeFunc;  /* Freeze function */
} SDNPSESN_OBJ_FREEZE_FUNC_ENTRY;

/* Grp Var  AllVariations   Write */
static const SDNPSESN_OBJ_FREEZE_FUNC_ENTRY _sdnpObjFreezeFuncTable[] = {
  #if SDNPDATA_SUPPORT_OBJ21
  {20,  0,  sdnp020_freeze},
#endif
#if SDNPDATA_SUPPORT_OBJ31
  {30,  0,  sdnp030_freeze},
#endif
  {0,   0,  TMWDEFS_NULL}      /* Last entry */
};
#endif

#if SDNPDATA_SUPPORT_ASSIGN
/* Define a table that assigns a function to handle assign class
 * requests.
 */
typedef struct {
  TMWTYPES_UCHAR group;                          /* Object group */
  TMWTYPES_UCHAR variation;                      /* Object variation */ 
  SDNPSESN_ASSIGN_CLASS_FUNC pAssignClassFunc;   /* Assign class function */
} SDNPSESN_OBJ_ASSIGN_FUNC_ENTRY;

/* Grp Var  AllVariations   Write */
static const SDNPSESN_OBJ_ASSIGN_FUNC_ENTRY _sdnpObjAssignFuncTable[] = {
#if SDNPDATA_SUPPORT_OBJ1 && SDNPDATA_SUPPORT_OBJ2
  {1,  0,  sdnpo001_assignClass},
#endif
#if SDNPDATA_SUPPORT_OBJ3 && SDNPDATA_SUPPORT_OBJ4
  {3,  0,  sdnpo003_assignClass},
#endif
#if SDNPDATA_SUPPORT_OBJ10 && SDNPDATA_SUPPORT_OBJ11
  {10,  0,  sdnpo010_assignClass},
#endif
#if SDNPDATA_SUPPORT_OBJ12 && SDNPDATA_SUPPORT_OBJ13
  {12,  0,  sdnpo012_assignClass},
#endif
#if SDNPDATA_SUPPORT_OBJ20 && SDNPDATA_SUPPORT_OBJ22
  {20,  0,  sdnpo020_assignClass},
#endif
#if SDNPDATA_SUPPORT_OBJ21 && SDNPDATA_SUPPORT_OBJ23
  {21,  0,  sdnpo021_assignClass},
#endif
#if SDNPDATA_SUPPORT_OBJ30 && SDNPDATA_SUPPORT_OBJ32
  {30,  0,  sdnpo030_assignClass},
#endif
#if SDNPDATA_SUPPORT_OBJ31 && SDNPDATA_SUPPORT_OBJ33
  {31,  0,  sdnpo031_assignClass},
#endif
#if SDNPDATA_SUPPORT_OBJ40 && SDNPDATA_SUPPORT_OBJ42
  {40,  0,  sdnpo040_assignClass},
#endif
#if SDNPDATA_SUPPORT_OBJ41 && SDNPDATA_SUPPORT_OBJ43
  {41,  0,  sdnpo041_assignClass},
#endif
#if SDNPDATA_SUPPORT_OBJ70
  /* application layer spec says obj70v3 but obj70v0 seems logical */
  {70,  0,  sdnpo070_assignClass},
  {70,  3,  sdnpo070_assignClass},
#endif
#if SDNPDATA_SUPPORT_OBJ86 && SDNPDATA_SUPPORT_OBJ88
  {86,  0,  sdnpo086_assignClass},
#endif
#if SDNPDATA_SUPPORT_OBJ110 && SDNPDATA_SUPPORT_OBJ111
  {110, 0,  sdnpo110_assignClass},
#endif
#if SDNPDATA_SUPPORT_OBJ112 && SDNPDATA_SUPPORT_OBJ113
  {112, 0,  sdnpo112_assignClass},
#endif
#if SDNPDATA_SUPPORT_OBJ114 && SDNPDATA_SUPPORT_OBJ115
  {114, 0,  sdnpo114_assignClass},
#endif
#if SDNPDATA_SUPPORT_OBJ120
  {120, 0,  sdnpo120_assignClass},
#endif
#if SDNPCNFG_SUPPORT_SA_VERSION5 
  {121, 0,  sdnpo121_assignClass},
#endif
  {0,   0,  TMWDEFS_NULL}              /* Last entry */
};
#endif

/* Local Functions */
       
/*  validate message size based on header info */
static TMWTYPES_BOOL TMWDEFS_LOCAL _validateMessageSize(
  TMWSESN *pSession,
  TMWTYPES_UCHAR objectSize, 
  DNPUTIL_OBJECT_HEADER *pHeader, 
  TMWTYPES_USHORT msgLength)
{
  TMWTYPES_ULONG length;
  int pointIndexLength = 0;

  if(objectSize == SDNP_NOCHECK)
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
    DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_INVALID_SIZE);
    
    ((SDNPSESN*)pSession)->iin |= DNPDEFS_IIN_OUT_OF_RANGE;
    return TMWDEFS_FALSE;
  }
  else 
    return TMWDEFS_TRUE;
} 

/* function: _cancelSelect */
static void _cancelSelect(SDNPSESN *pSDNPSession)
{
#if SDNPDATA_SUPPORT_SELECT_CANCEL
  DNPUTIL_RX_MSG rxMsg;
  DNPUTIL_RX_MSG *pRxMessage = &rxMsg; 

  pRxMessage->pMsgBuf = pSDNPSession->selectBuffer;
  pRxMessage->msgLength = pSDNPSession->selectBufferLength;
  pRxMessage->offset = 2;
    
  while(pRxMessage->offset < pRxMessage->msgLength)
  {
    int index;
    DNPUTIL_OBJECT_HEADER header;

    /* Parse object header */
    if(!dnputil_parseObjectHeader(pRxMessage, 0, &header))
    {
      break;
    }
 
    index = 0;
    while(_sdnpObjCanSelFuncTable[index].group != 0)
    {
      if(_sdnpObjCanSelFuncTable[index].group == header.group)
      {
        _sdnpObjCanSelFuncTable[index].pCancelSelectFunc(
          (TMWSESN*)pSDNPSession, pRxMessage, &header);
        break;
      }

      index += 1;
    }
  }
#else
  TMWTARG_UNUSED_PARAM(pSDNPSession);
#endif
}

/* function: _transmitConfirm */
static void TMWDEFS_LOCAL _transmitConfirm(
  TMWSESN *pSession, 
  TMWTYPES_UCHAR control,
  TMWTYPES_USHORT destAddress)
{
  /* Allocate and initialize tx data buffer for confirm */
  DNPCHNL_TX_DATA *pTxData = (DNPCHNL_TX_DATA *)dnpchnl_newTxData(
    pSession->pChannel, pSession, 2, destAddress);

  if(pTxData == TMWDEFS_NULL)
  {
    return;
  }

  pTxData->tmw.msgLength = 2;

#if TMWCNFG_SUPPORT_DIAG
  pTxData->tmw.pMsgDescription = "Application Confirmation";
#endif

  /* Fill in application control byte */
  pTxData->tmw.pMsgBuf[0] = (TMWTYPES_UCHAR)(DNPDEFS_AC_FIRST_AND_FINAL
    | (control & DNPDEFS_AC_SEQUENCE_MASK));

  /* Fill in function code */
  pTxData->tmw.pMsgBuf[1] = DNPDEFS_FC_CONFIRM;

  /* Transmit flags, no retry, no response expected */
  pTxData->tmw.txFlags = TMWSESN_TXFLAGS_NO_RESPONSE;

  /* Send the application confirm using normal mechanism, which
   * will now send this even if a request is outstanding.
   */
  dnpchnl_sendFragment((TMWSESN_TX_DATA *)pTxData);
}


/* function: _initializeResponse
 * purpose: initialize an application layer response
 * arguments:
 *  pSession - session to respond to
 *  pResponse - response data
 *  firstFragment - is this the first fragment in the response
 *  finalFragment - is this the final fragment in the response
 * returns:
 *  void
 */
static void TMWDEFS_LOCAL _initializeResponse(
  TMWSESN *pSession,
  TMWSESN_TX_DATA *pResponse,
  TMWTYPES_BOOL firstFragment,
  TMWTYPES_BOOL finalFragment)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  TMWTYPES_UCHAR control;

  /* Application Control */
  control = pSDNPSession->respSequenceNumber;
  pSDNPSession->lastRespSequenceNumber = control;

  if(firstFragment)
    control |= DNPDEFS_AC_FIRST;

  if(finalFragment)
    control |= DNPDEFS_AC_FINAL;

  pResponse->pMsgBuf[0] = control;

  /* Function Code */
  pResponse->pMsgBuf[1] = DNPDEFS_FC_RESPONSE;

  /* IIN bits */
  pResponse->pMsgBuf[2] = (TMWTYPES_UCHAR)((pSDNPSession->iin & 0xff00) >> 8);
  pResponse->pMsgBuf[3] = (TMWTYPES_UCHAR)(pSDNPSession->iin & 0x00ff);

  /* Current message length */
  pResponse->msgLength = 4;

  /* Misc initialization */
  pResponse->txFlags = TMWSESN_TXFLAGS_NO_RESPONSE;

  /* Increment sequence number */
  pSDNPSession->respSequenceNumber =
    sdnputil_nextSequenceNumber(pSDNPSession->respSequenceNumber);
  
#if SDNPDATA_SUPPORT_OBJ120
  if(pSDNPSession->authenticationEnabled && pSDNPSession->authSendAggrResp)
  {  
    /* Leave some room at end for aggressive mode MAC object */
    pResponse->maxLength -= SDNPAUTH_AGGRMODEPAD;
    sdnpauth_addAggrModeStart(pSDNPSession, pResponse); 
  }
#endif 
}

/* function: _sendResponse */
static void TMWDEFS_GLOBAL _sendResponse(
  TMWSESN *pSession,
  TMWTYPES_BOOL noResp,
  DNPUTIL_RX_MSG *pRequest,
  TMWSESN_TX_DATA *pResponse)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  if(pRequest->isBroadcast
    || noResp)
  {
    /* No response was required, deallocate the TX Data */
    dnpchnl_freeTxData(pResponse);

    /* Clear error IIN bits, so these don't get sent in next response */
    pSDNPSession->iin &= ~DNPDEFS_IIN_ERROR_BITS;

    /* See if there is anything waiting to be sent */
    _processNextMessage(pSession);
    return;
  }

  if(pResponse->pMsgBuf[0] & DNPDEFS_AC_CONFIRM)
  {
    pResponse->responseTimeout = pSDNPSession->applConfirmTimeout;
  }
    
  if(!dnpchnl_sendFragment(pResponse))
  {
    dnpchnl_freeTxData(pResponse);
  }
}


/* function: _sendNullResponse */
static TMWTYPES_BOOL TMWDEFS_LOCAL _sendNullResponse(
  TMWSESN *pSession,
  DNPUTIL_RX_MSG *pRequest)
{
  TMWSESN_TX_DATA *pResponse = dnpchnl_newTxData(
    pSession->pChannel, pSession, 4, pRequest->rxAddress);

  if(pResponse == TMWDEFS_NULL)
  {
    return(TMWDEFS_FALSE);
  }

#if TMWCNFG_SUPPORT_DIAG
  pResponse->pMsgDescription = "NULL Response";
#endif

  _initializeResponse(pSession, pResponse, TMWDEFS_TRUE, TMWDEFS_TRUE);
  _sendResponse(pSession, TMWDEFS_FALSE, pRequest, pResponse);

  return(TMWDEFS_TRUE);
}

/* function: _buildReadResponse
 * purpose: build response to a read request
 * arguments:
 *  pSession - session this response will be sent to
 *  firstFragment - is this the first fragment in the response
 * returns:
 *  void
 */
static void TMWDEFS_LOCAL _buildReadResponse(
  TMWSESN *pSession,
  TMWTYPES_BOOL firstFragment)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  DNPCHNL *pDNPChannel = (DNPCHNL *)pSession->pChannel;
  DNPUTIL_RX_MSG *pRequest = &pSDNPSession->lastRcvdRequest;
  DNPUTIL_OBJECT_HEADER header;
  TMWSESN_TX_DATA *pResponse;

  /* Initialize response */
  pResponse = dnpchnl_newTxData((TMWCHNL *)pDNPChannel,
    pSession, pDNPChannel->txFragmentSize, pRequest->rxAddress);

  if(pResponse == TMWDEFS_NULL)
  {
    return;
  }
  _initializeResponse(pSession, pResponse, firstFragment, TMWDEFS_FALSE);

  
#if SDNPDATA_SUPPORT_OBJ120
  if (pSDNPSession->authenticationEnabled && pSDNPSession->preChallengeApplConf)
    /* Leave some room at end for the challenge object */
    pResponse->maxLength -= SDNPAUTH_PRECHALLENGEPAD; 
#endif

#if TMWCNFG_SUPPORT_DIAG
  pResponse->pMsgDescription = "Read Response";
#endif

  /* Read requests are processed in four passes. The first pass reads explicit
   * event requests and figures out which object 60 class 1, 2, or 3 event polls
   * were requested. Next we read any events specified by a class 1, 2, or 3
   * event data poll (as determined in pass one). The third pass reads all
   * explicit static data. Finally we read static data specified by a class 0
   * data poll.
   */
  do
  {
    if(pSDNPSession->readPass == 1)
    {
      /* Read class 1, 2, 3 events */
      pSDNPSession->readStatus = sdnpo060_readEvents(
        pSession, pRequest, pResponse, &pSDNPSession->eventObjectHeader);
    }
    else if(pSDNPSession->readPass == 3)
    {
      /* Read class 0 static data */ 
      pSDNPSession->readStatus = sdnpo060_readStatics(
        pSession, pRequest, pResponse, &pSDNPSession->staticObjectHeader);
    }
    else
    {
      /* Loop through request, processing objects as we go */
      do
      {
        TMWTYPES_BOOL processed;
        TMWTYPES_ULONG offset;
        int tableIndex;

        /* Save offset to current object header in case we don't
         * finish reading the entire object
         */
        offset = pRequest->offset;

        /* Parse object header from message */
        if(!dnputil_parseObjectHeader(pRequest, 0, &header))
        {
          DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_PARSE_HEADER);
          pSDNPSession->readStatus = SDNPSESN_READ_FAILED;
          break;
        }
       
        /* Are we reading this object for the first time */
        if(pSDNPSession->readStatus == SDNPSESN_READ_COMPLETE)
        {
          /* Yes, initialize point index to first point */
          pSDNPSession->readPointIndex = 0;
        }

        /* Call processing function */
        tableIndex = 0;
        processed = TMWDEFS_FALSE;
        while(_sdnpObjReadFuncTable[tableIndex].pReadFunc != TMWDEFS_NULL)
        {
          if((_sdnpObjReadFuncTable[tableIndex].group == header.group)
            && (_sdnpObjReadFuncTable[tableIndex].allVariations
            || (_sdnpObjReadFuncTable[tableIndex].variation == header.variation)))
          {
            SDNPSESN_QUAL qualifier;
            processed = TMWDEFS_TRUE;

            if(_sdnpObjReadFuncTable[tableIndex].readPass == pSDNPSession->readPass)
            {
              /* Diagnostics */
              DNPDIAG_SHOW_OBJECT_HEADER(pSession, &header);
              DNPDIAG_BUILD_MESSAGE(pResponse->pChannel, (TMWSESN*)pSDNPSession, pResponse->pMsgDescription); 

              /* only these qualifiers are allowed for event reads which are done on pass 0 */
              /* Except for Virtual Terminal events which actually allow indexed reads */
              if((pSDNPSession->readPass == 0) 
                && !((header.qualifier == DNPDEFS_QUAL_ALL_POINTS)
                ||   (header.qualifier == DNPDEFS_QUAL_8BIT_LIMITED_QTY)
                ||   (header.qualifier == DNPDEFS_QUAL_16BIT_LIMITED_QTY)
                ||   (header.qualifier == DNPDEFS_QUAL_16BIT_FREE_FORMAT))
                && (header.group != DNPDEFS_OBJ_113_VTERM_EVENTS))
              {
                DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_QUALIFIER);
                pSDNPSession->readStatus = SDNPSESN_READ_FAILED;
                break;
              }

              qualifier = SDNPSESN_QUAL_BUILD_RESPONSE;
#ifdef TMW_SUPPORT_MONITOR
              if(pDNPChannel->tmw.pPhysContext->monitorMode)
                qualifier = SDNPSESN_QUAL_PARSE_ONLY;
#endif
            }
            else
            {
              qualifier = SDNPSESN_QUAL_PARSE_ONLY;
            }

            pSDNPSession->readStatus =
              _sdnpObjReadFuncTable[tableIndex].pReadFunc(
              pSession, pRequest, pResponse, &header, qualifier);

            break;
          }

          tableIndex += 1;
        }

        /* Check read status */
        if(!processed)
        {
          /* Was the object/variation supported */
          pSDNPSession->iin |= DNPDEFS_IIN_OBJECT_UNKNOWN;
        }
        else if(pSDNPSession->readStatus == SDNPSESN_READ_MORE_DATA)
        {
          /* If more data reset offset to beginning of this object
           * header so we will start reading this object type next
           * time through
           */
          pRequest->offset = offset;
        }
      } while((pRequest->offset < pRequest->msgLength)
        && (pSDNPSession->readStatus == SDNPSESN_READ_COMPLETE));
    }

    /* See if we are done with this pass */
    if(pSDNPSession->readStatus == SDNPSESN_READ_COMPLETE)
    {
      /* Yep, reset request offset to point to first object
       *  header in the request and increment read pass
       */
      pRequest->offset = 2;
      pSDNPSession->readPass += 1;
      pSDNPSession->readGroupIndex = 0;
    } 
    else if(pSDNPSession->readStatus == SDNPSESN_READ_FAILED)
    {
      if((pSDNPSession->iin & DNPDEFS_IIN_OBJECT_UNKNOWN) == 0)
      {
        /* If the read failed, and it was not an unknown object/variation, it was because of an invalid point */
        pSDNPSession->iin |= DNPDEFS_IIN_OUT_OF_RANGE;
      }
    }

  } while((pSDNPSession->readPass < 4)
    && (pSDNPSession->readStatus == SDNPSESN_READ_COMPLETE));

  /* Is this the final fragment in the response */
  if(pSDNPSession->readStatus != SDNPSESN_READ_MORE_DATA)
  {
    /* Yep, set the final bit and terminate the read request */
    pSDNPSession->readPending = TMWDEFS_FALSE;
    pResponse->pMsgBuf[0] |= DNPDEFS_AC_FINAL;
    pSDNPSession->pendingRequest = TMWDEFS_FALSE;
    if(pSDNPSession->databaseLocked)
    {  
      sdnpdata_eventAndStaticRead(pSDNPSession->pDbHandle, TMWDEFS_FALSE);
      pSDNPSession->databaseLocked = TMWDEFS_FALSE;
    }
  }
  else
  {
    /* If multifragment responses are not allowed, mark this FINAL */
    /* Or if no more data is allowed to be put in this response 
     * used by secure authentication challenge message.
     */
    if((pSDNPSession->multiFragRespAllowed == TMWDEFS_FALSE)
      ||(pSDNPSession->readStatus == SDNPSESN_READ_NOMORE_DATA)) 
    {
      pSDNPSession->readStatus = SDNPSESN_READ_COMPLETE;
      pSDNPSession->readPending = TMWDEFS_FALSE;
      pSDNPSession->pendingRequest = TMWDEFS_FALSE;
      pResponse->pMsgBuf[0] |= DNPDEFS_AC_FINAL;
      if(pSDNPSession->databaseLocked)
      {  
        sdnpdata_eventAndStaticRead(pSDNPSession->pDbHandle, TMWDEFS_FALSE);
        pSDNPSession->databaseLocked = TMWDEFS_FALSE;
      }
    }
    /* Multi Fragment responses allowed, now see if application confirmations required */
    else if(pSDNPSession->multiFragConfirm)
    {
      pSDNPSession->multiFragWaitingForConfirm = TMWDEFS_TRUE;

      /* Dont call function if it is going to be done for events below. */
      if ((pResponse->txFlags & TMWSESN_TXFLAGS_CONTAINS_EVENTS) == 0)
        sdnpsesn_requestApplConfirm(pSession, pResponse);
    }
  }

  /* See if the response contains events */
  if(pResponse->txFlags & TMWSESN_TXFLAGS_CONTAINS_EVENTS)
  {
    /* Yep, need to request an application confirmation */
    sdnpsesn_requestApplConfirm(pSession, pResponse);

    /* Update IIN bits to reflect new state of queued events */
    sdnprbe_updateIINBits(pSession);
    
    /* If unsolicited responses are enabled, check to see if event queues 
     * are empty. If so, cancel unsolicited delay timer for that class. 
     */
    if(pSDNPSession->unsolEventMask != 0)
    {
      int i;
      sdnpunsl_countUnsolicitedEvents(pSession, TMWDEFS_CLASS_MASK_ALL);
      for(i = 0; i < TMWDEFS_CLASS_MAX; i++)
      {
        if(pSDNPSession->unsolNumPending[i] == 0)
        {
          tmwtimer_cancel(&pSDNPSession->unsolDelayTimer[i]);
        }
      }
    }
  } else if(pSDNPSession->iin & DNPDEFS_IIN_ALL_CLASSES){
    /* If IIN bits say events are queued, check to see if this is still true 
    * this is necessary when the database manages the events 
    */
    sdnprbe_updateIINBits(pSession);
  }  
  else if(pSDNPSession->iin & DNPDEFS_IIN_BUFFER_OVFL)
  {
    /* Need to request an application confirmation, even
     * though no events are contained in the response, this
     * will result in a confirm from the master to clear this
     * IIN bit.
     */
    sdnpsesn_requestApplConfirm(pSession, pResponse);
  }

  /* Send the response */
  _sendResponse(pSession, TMWDEFS_FALSE, pRequest, pResponse);
}
 
/* function: _broadcastEnabledFC */
static TMWTYPES_BOOL TMWDEFS_CALLBACK _broadcastEnabledFC(SDNPSESN *pSDNPSession)
{
  pSDNPSession->iin |= DNPDEFS_IIN_ALL_STATIONS;
  if(pSDNPSession->lastRcvdRequest.rxAddress == DNPDEFS_BROADCAST_ADDR_CON)
  {
    pSDNPSession->allStationsConfirmRequired = TMWDEFS_TRUE;
  }
  else
  {
    pSDNPSession->allStationsConfirmRequired = TMWDEFS_FALSE;
  }

  if(pSDNPSession->lastRcvdRequest.fc < 32)
  {
    TMWTYPES_ULONG temp = (1 << pSDNPSession->lastRcvdRequest.fc);
    if((pSDNPSession->enabledBroadcastFCs & temp) != 0)
      return TMWDEFS_TRUE;
  }
  return TMWDEFS_FALSE;
}

/* function: _enabledFC */
static TMWTYPES_BOOL TMWDEFS_CALLBACK _enabledFC(SDNPSESN *pSDNPSession)
{
  if(pSDNPSession->lastRcvdRequest.fc < 32)
  {
    TMWTYPES_ULONG temp = (1 << pSDNPSession->lastRcvdRequest.fc);
    if((pSDNPSession->enabledFCs & temp) != 0)
      return TMWDEFS_TRUE;
  }
  return TMWDEFS_FALSE;
}

/* function: _processNextMessage
 * purpose: Build next response message.
 * arguments:
 *  pSession - session response will be sent to
 * returns:
 *  void
 */
static void TMWDEFS_CALLBACK _processNextMessage(
  TMWSESN *pSession)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;

  /* Make sure application fragment buffer is available */
  if(pSDNPSession->dnp.pCurrentMessage != TMWDEFS_NULL)
    return;

  /* See if we have a pending request to process */
  if(pSDNPSession->pendingRequest)
  {
    TMWTYPES_BOOL processed = TMWDEFS_FALSE;
    
#if SDNPDATA_KEEP_LAST_RESPONSE
    /* If this request is a duplicate, sequence number in this request matches the 
     * sequence number in the previous request and the request is identical, resend 
     * the last response, but do not process this duplicate request.
     */
    if(pSDNPSession->duplicateRequestRcvd)
    {
      pSDNPSession->duplicateRequestRcvd = TMWDEFS_FALSE;
      pSDNPSession->pendingRequest = TMWDEFS_FALSE;

      /* Resend last response */
      pSDNPSession->pLastResponse->sent = TMWDEFS_FALSE;
      _sendResponse(pSession, TMWDEFS_FALSE, &pSDNPSession->lastRcvdRequest, (TMWSESN_TX_DATA*)pSDNPSession->pLastResponse);

      return;
    }
#endif

    /* If function code disabled, do not process it */
    if((!pSDNPSession->lastRcvdRequest.isBroadcast)
       && (!_enabledFC(pSDNPSession)))
    {
      DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_FC_DISABLED);
    }
    else
    { 
      /* Loop through the function code support table to see if this
       *  function code is supported.
       */
      int index = 0;
      while(sdnpsesn_funcTable[index].pFunc != TMWDEFS_NULL)
      {
        if(pSDNPSession->lastRcvdRequest.fc == sdnpsesn_funcTable[index].funcCode)
        {
          if(pSDNPSession->lastRcvdRequest.isBroadcast
            && !_broadcastEnabledFC(pSDNPSession))
          {
            DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_BRDCST_DISABLED);
            break;
          }

          sdnpdata_funcCode(pSDNPSession->pDbHandle, pSDNPSession->lastRcvdRequest.fc, TMWDEFS_TRUE);

          /* Call the function code specific processing routine */
          sdnpsesn_funcTable[index].pFunc(pSession);

          sdnpdata_funcCode(pSDNPSession->pDbHandle, pSDNPSession->lastRcvdRequest.fc, TMWDEFS_FALSE);
          processed = TMWDEFS_TRUE;
          break;
        }

        index += 1;
      }
    }
    if(!processed)
    {
      /* Function code was not recognized */
      pSDNPSession->pendingRequest = TMWDEFS_FALSE;
      pSDNPSession->iin |= DNPDEFS_IIN_BAD_FUNCTION;
      pSDNPSession->respSequenceNumber = (TMWTYPES_UCHAR)(pSDNPSession->lastRcvdRequest.ac & DNPDEFS_AC_SEQUENCE_MASK);
      _sendNullResponse(pSession, &pSDNPSession->lastRcvdRequest);
    }

    return;
  }

  /* If unsolicited events are allowed */
  if(pSDNPSession->unsolAllowed)
  {
    /* Try sending any unsolicited responses that are pending */
    if(!tmwtimer_isActive(&pSDNPSession->unsolRetryTimer)
      && !pSDNPSession->unsolWaitingForConfirm
      && !pSDNPSession->unsolQueued)
    {
      /* Has initial null response been confirmed */
      if(!pSDNPSession->unsolInitialNullConfirmed)
      {
        /* Nope, try again */
        sdnpunsl_sendNullResponse(pSession, TMWDEFS_TRUE);
      }
      else
      {
        /* Yep, send events */
        sdnpunsl_processUnsolEvents(pSession, TMWDEFS_CLASS_MASK_ALL);
      }
    }
  }
}

/* function: _processFragment */
static TMWTYPES_BOOL TMWDEFS_CALLBACK _processFragment(
  TMWSESN *pSession,
  TMWSESN_RX_DATA *pRxFragment)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  TMWTYPES_UCHAR ac;

#if  SDNPDATA_SUPPORT_OBJ120
  /* If this returns TMWDEFS_NULL, processing of this fragment is complete 
   * (a challenge request might have been sent back to master and
   * a request queued for later processing.
   */ 
  if(pSDNPSession->authenticationEnabled)
  {
    pRxFragment = sdnpauth_processing(pSession, SDNPAUTH_EVT_MSG_RECEIVED, 0, pRxFragment);
    if(pRxFragment == TMWDEFS_NULL)
      return(TMWDEFS_TRUE);
  }
#endif

  /* Clear unsolicited retry timer so we will retry an unsolicited
   * response as soon as we are done processing this request
   */
  if(!pSDNPSession->unsolWaitingForConfirm
    && tmwtimer_isActive(&pSDNPSession->unsolRetryTimer))
  {
    pSDNPSession->unsolNumRetries = 0;
    tmwtimer_cancel(&pSDNPSession->unsolRetryTimer);
  }

  /* Process confirms immediately */
  if(pRxFragment->pMsgBuf[DNPDEFS_AH_INDEX_FUNC_CODE] == DNPDEFS_FC_CONFIRM)
  {
    if(!pRxFragment->isBroadcast
      || _broadcastEnabledFC(pSDNPSession))
    {
      _processConfirm(pSession, pRxFragment);
      return(TMWDEFS_TRUE);
    }
    else
    {  
      DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_BRDCST_DISABLED);
      return(TMWDEFS_FALSE);
    }
  }

  /* If application layer confirm was requested, send one  
   * Master is no longer supposed to request application confirm, but
   * if it does slave should respond with confirm.
   */
  ac = pRxFragment->pMsgBuf[DNPDEFS_AH_INDEX_APPL_CTRL];
  if(ac & DNPDEFS_AC_CONFIRM)
  {
    /* Yes, confirm it */
    _transmitConfirm(pSession, ac, pRxFragment->rxAddress);
  }

  /* Received a new request, abort any existing requests */
  if(pSDNPSession->pendingRequest)
  {
    SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_REQ_PENDING);
    pSDNPSession->pendingRequest = TMWDEFS_FALSE;
    pSDNPSession->readPending = TMWDEFS_FALSE;
    if(pSDNPSession->databaseLocked)
    {  
      sdnpdata_eventAndStaticRead(pSDNPSession->pDbHandle, TMWDEFS_FALSE);
      pSDNPSession->databaseLocked = TMWDEFS_FALSE;
    }
  }

  /* If this session has a solicited response pending, cancel it */
  if((pSDNPSession->dnp.pCurrentMessage != TMWDEFS_NULL)
    && ((pSDNPSession->dnp.pCurrentMessage->pMsgBuf[DNPDEFS_AH_INDEX_APPL_CTRL] & DNPDEFS_AC_UNSOLICITED) == 0))
  {
    SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_CANCEL_FRAGMENT);

    /* dnpchnl_cancelFragment will NULL out pCurrentMessage */
    dnpchnl_cancelFragment(pSDNPSession->dnp.pCurrentMessage);
  }

  /* Parse application header into lastRcvdRequest*/
  sdnputil_parseApplHeader(pRxFragment, pSDNPSession);

  /* If a select is pending and this request is not an operate
   * cancel the select. Allow duplicate selects also.
   */
  if (pSDNPSession->selectBufferLength != 0)
  {
    /* Select is pending */
#if !SDNPCNFG_DISABLE_SELOP_SEQ_CHECK
    if ((pSDNPSession->lastRcvdRequest.fc != DNPDEFS_FC_OPERATE)
      && (pSDNPSession->lastRcvdRequest.fc != DNPDEFS_FC_SELECT))
    {
      /* Request is Not an Operate or Select */
      /* Cancel timer if it is running */
      if (tmwtimer_isActive(&pSDNPSession->selectTimer))
      {
        SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_OPER_SELECT);
        tmwtimer_cancel(&pSDNPSession->selectTimer);
        _cancelSelect(pSDNPSession);
      }
      /* Remove pending select request */
      pSDNPSession->selectBufferLength = 0;
    }
#else
    if ((pSDNPSession->lastRcvdRequest.fc != DNPDEFS_FC_OPERATE)
      && (pSDNPSession->lastRcvdRequest.fc != DNPDEFS_FC_SELECT))
    {
      /* Request is Not an Operate or Select */
      /* If checking is disabled, also allow Read requests between the Select and Operate request
       * Note: this would violate the DNP3 Specification and fail the DNP3 conformance tests,
       * but is tolerant of a master that sends reads between the Select and Operate requests.
       */
      if (!pSDNPSession->disableSelOpSeqCheck || (pSDNPSession->lastRcvdRequest.fc != DNPDEFS_FC_READ))
      {
        /* Cancel timer if it is running */
        if (tmwtimer_isActive(&pSDNPSession->selectTimer))
        {
          SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_OPER_SELECT);
          tmwtimer_cancel(&pSDNPSession->selectTimer);
          _cancelSelect(pSDNPSession);
        }
        /* Remove pending select request */
        pSDNPSession->selectBufferLength = 0;
      }
    }
#endif 
  }

  /* If not done with the previous transmission, delay
   * processing of this request until we can transmit
   * response.
   */
  if(pSDNPSession->dnp.pCurrentMessage != TMWDEFS_NULL)
  {
    SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_CHNL_BUSY);
    return(TMWDEFS_TRUE);
  }

  /* Channel is available, go ahead and build the
   * response.
   */
  _processNextMessage(pSession);

  return(TMWDEFS_TRUE);
}

/* function: _unsolRetryTimeout
 * purpose: Handle retry timeouts (i.e. period to wait before retrying
 *  unsolicited responses)
 * arguments:
 *  pCallbackParam - pointer to user specified callback parameter, points
 *   to session.
 * returns:
 *  void
 */
static void TMWDEFS_CALLBACK _unsolRetryTimeout(
  void *pCallbackParam)
{
  TMWSESN *pSession = (TMWSESN *)pCallbackParam;
  _processNextMessage(pSession);
}

/* function: _selectTimeout
 * purpose:  
 * arguments:
 *  pCallbackParam - pointer to user specified callback parameter, points
 *   to session.
 * returns:
 *  void
 */
static void TMWDEFS_CALLBACK _selectTimeout(
  void *pCallbackParam)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pCallbackParam;  
  _cancelSelect(pSDNPSession);
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
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  TMWTARG_UNUSED_PARAM(status);

  /* We can be waiting for a confirmation from either an unsolicited
   * response or an application response. If neither of these are
   * true, log an error and return
   */
  if((pTxData == TMWDEFS_NULL)
    && !pSDNPSession->unsolWaitingForConfirm)
  {
    SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_APPL_NO_RESP);
    return;
  }

  if(pSDNPSession->dnp.pCurrentMessage == pTxData)
  {
    pSDNPSession->dnp.pCurrentMessage = TMWDEFS_NULL;
  }

#if SDNPDATA_SUPPORT_IDENT_UNSOL_RETRY
  /* Mark any events as NOT SENT.
   * However, if this was the initial null response that timed out, it does not contain events
   * Initial null response timeout was causing problems with solicited file transfer response events.
   */
  if(pSDNPSession->unsolSendIdenticalRetry)
  {
    /* Don't clean up the unsolicited events so we can send an identical retry */
    if(!pSDNPSession->unsolWaitingForConfirm)
    {
      sdnprbe_cleanupEvents(pSession, TMWDEFS_FALSE);
    }
  }
  else
  {
    if((!pSDNPSession->unsolWaitingForConfirm)
      ||(pSDNPSession->unsolWaitingForConfirm && pSDNPSession->unsolResponseContainsEvents))
    {
      sdnprbe_cleanupEvents(pSession, TMWDEFS_FALSE);
    }
  }

  /* Unsolicited timeout will be handled below, to allow for identical retry. */ 
#else
  /* Mark any events as NOT SENT.
   * However, if this was the initial null response that timed out, it does not contain events
   * Initial null response timeout was causing problems with solicited file transfer response events.
   */
  if((!pSDNPSession->unsolWaitingForConfirm)
    ||(pSDNPSession->unsolWaitingForConfirm && pSDNPSession->unsolResponseContainsEvents))
  {
    sdnprbe_cleanupEvents(pSession, TMWDEFS_FALSE);
  }
#endif


  /* If this is a timeout of an unsolicited response  
   * If the unsolicited confirm timeout expired pTxData would be NULL
   * If the UR timed out on the queue before being transmitted pTxData would not be NULL and status would be TIMEOUT
   * If the UR failed to be transmitted, status would be FAILURE and this was already processed in sdnpsesn_failedUnsolTx()
   */
  if((pTxData == TMWDEFS_NULL)
    || (((pTxData->pMsgBuf[0] & DNPDEFS_AC_UNSOLICITED) != 0) && (status == DNPCHNL_RESP_STATUS_TIMEOUT)))
  {
    int index;
    TMWTYPES_MILLISECONDS delay;

    /* if it timed out on the queue */
    pSDNPSession->unsolQueued = TMWDEFS_FALSE;

    /* Yep, no longer waiting for confirm */
    pSDNPSession->unsolWaitingForConfirm = TMWDEFS_FALSE;
#if SDNPDATA_SUPPORT_IDENT_UNSOL_RETRY
    if(!pSDNPSession->unsolSendIdenticalRetry)
      pSDNPSession->unsolResponseContainsEvents = TMWDEFS_FALSE;
#else
    pSDNPSession->unsolResponseContainsEvents = TMWDEFS_FALSE;
#endif

    /* Get retry period based on number of retries */
    delay = pSDNPSession->unsolRetryDelay;
    if(pSDNPSession->unsolInitialNullConfirmed
      && (pSDNPSession->unsolNumRetries++ >= pSDNPSession->unsolMaxRetries))
    {
      if (pSDNPSession->unsolOfflineDelayOnce)
      {
        /* Delay the longer time once and then go back to the normal delay */
        pSDNPSession->unsolNumRetries = 0;
      }
      else
      {
        /* To prevent NumRetries from wrapping past zero, just keep it at MaxRetries */
        pSDNPSession->unsolNumRetries = pSDNPSession->unsolMaxRetries;
      }

      delay = pSDNPSession->unsolOfflineRetryDelay;
      
#if SDNPDATA_SUPPORT_IDENT_UNSOL_RETRY
      if((delay == UNSOL_NORETRY) 
        && (pSDNPSession->pUnsolLastResponse != TMWDEFS_NULL))
      {
        /* If we saved unsolicited response to be sent as an identical
         * retry, mark all of the events as not sent, and deallocate
         * the saved identical retry in xxcleanupEvents.
         */
        if(pSDNPSession->pUnsolLastResponse != TMWDEFS_NULL)
          sdnprbe_cleanupEvents(pSession, TMWDEFS_FALSE);
      }
#endif
    }

    /* cancel any delay timer that was started while waiting for the confirm. We don't want 
     * delay the unsolicited retry waiting for more events since this was already satisfied.
     */
    for(index = 0; index < 3; index++)
      tmwtimer_cancel(&pSDNPSession->unsolDelayTimer[index]); 

    /* Update unsolicited event status */
    sdnpunsl_updateStatus(pSession);

    /* Start unsolicited retry timer */
    tmwtimer_start(
      &pSDNPSession->unsolRetryTimer, delay,
      pSession->pChannel, _unsolRetryTimeout, pSession);

    /* Call idle callback if channel is idle */
    dnpchnl_checkForChannelIdle(pSession->pChannel);
    return;
  }
  
  /* Update unsolicited event status */
  sdnpunsl_updateStatus(pSession);

  /* If a fragment from a multifragment read response timed out.
   * Cancel the entire read request
   */
  if((pSDNPSession->readPending)
    && (pSDNPSession->readStatus == SDNPSESN_READ_MORE_DATA))
  {
    pSDNPSession->readPending = TMWDEFS_FALSE;
    pSDNPSession->pendingRequest = TMWDEFS_FALSE;
    if(pSDNPSession->databaseLocked)
    {  
      sdnpdata_eventAndStaticRead(pSDNPSession->pDbHandle, TMWDEFS_FALSE);
      pSDNPSession->databaseLocked = TMWDEFS_FALSE;
    }
  }
}

/* function: _unsolConfirmTimeout
 * purpose: Handle unsolicited confirm timeouts
 * arguments:
 *  pCallbackParam - pointer to user specified callback parameter, points
 *   to session.
 * returns:
 *  void
 */
static void TMWDEFS_CALLBACK _unsolConfirmTimeout(
  void *pCallbackParam)
{
  TMWSESN *pSession = (TMWSESN *)pCallbackParam;
  SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_UNSOL_TO);

  tmwsesn_setOnline(pSession, TMWDEFS_FALSE);

#if  SDNPDATA_SUPPORT_OBJ120
  {
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  /* Don't get upset about unsolicited null response timing out 
   * this is probably because the authentication keys are being exchanged
   */
  if(pSDNPSession->authenticationEnabled && pSDNPSession->unsolResponseContainsEvents)
    sdnpauth_applTimeout(pSDNPSession);
  }
#endif

  _processError(pSession, TMWDEFS_NULL, DNPCHNL_RESP_STATUS_TIMEOUT);
  
  /* See if there is anything else to transmit */
  _processNextMessage(pSession);
}

/* function _prepareMessage */
static void TMWDEFS_CALLBACK _prepareMessage(
  TMWSESN *pSession,
  TMWSESN_TX_DATA *pTxData)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;

  if(pTxData->pMsgBuf[1] != DNPDEFS_FC_AUTH_RESPONSE)
  { 
    /* Get user specified IIN bits */
    TMWTYPES_USHORT iin = 0;

    sdnpdata_getIIN(pSession, &iin);

    /* If user specified buffer overflow, latch this so that IIN bit
     * is not cleared until application confirm is received from master
     * and there is room in all of the SCL event queues. User can
     * set this bit if there is still an overflow in the database.
     */
    if(iin & DNPDEFS_IIN_BUFFER_OVFL)
    {
      pSDNPSession->iin |= DNPDEFS_IIN_BUFFER_OVFL;
    }

    /* Same for restart. If customer wants to set this,
     * latch it so it will stay set till we receive write obj80v1.
     */
    if(iin & DNPDEFS_IIN_RESTART)
      pSDNPSession->iin |= DNPDEFS_IIN_RESTART;

    /* Add in the SCL bits */
    iin |= pSDNPSession->iin;

    /* Remember this bit has been sent, to insure it gets sent at least once */  
    if(iin & DNPDEFS_IIN_BUFFER_OVFL)
    {
       pSDNPSession->bufferOverFlowSent = TMWDEFS_TRUE;
    }

    /* Store in response */
    pTxData->pMsgBuf[2] = (TMWTYPES_UCHAR)((iin & 0xff00) >> 8);
    pTxData->pMsgBuf[3] = (TMWTYPES_UCHAR)(iin & 0x00ff);

    /* Clear error IIN bits */
    pSDNPSession->iin &= ~DNPDEFS_IIN_ERROR_BITS;

    /* Process all stations IIN bit */
    if(pSDNPSession->allStationsConfirmRequired)
    {
      /* Confirm required */
      sdnpsesn_requestApplConfirm(pSession, pTxData);
    }
    else
    {
      /* Confirm not required, clear bit now */
      pSDNPSession->iin &= ~DNPDEFS_IIN_ALL_STATIONS;
    }
 
    pSDNPSession->dnp.pCurrentMessage = pTxData;
  }

#if  SDNPDATA_SUPPORT_OBJ120
 if((pTxData->txFlags & TMWSESN_TXFLAGS_DNP_AUTH_AGGR) !=0)
  { 
    pTxData->maxLength += SDNPAUTH_AGGRMODEPAD;
    sdnpauth_addAggrModeEnd(pSDNPSession, pTxData);
  }  
  if((pTxData->txFlags & TMWSESN_TXFLAGS_DNP_AUTH_CHALL) !=0)
  {
    sdnpauth_saveLastChallenge(pTxData);
  }
#endif 

}

/* function: _beforeTxCallback
 * purpose: Called by link layer right before the first frame
 *  of the current fragment is transmitted
 * arguments:
 *  pUserData - user specified callback data, points to session
 *  pTxData - response data being transmitted
 * returns:
 *  void
 */
static void TMWDEFS_CALLBACK _beforeTxCallback(
  TMWSESN_TX_DATA *pTxData)
{
#if SDNPDATA_SUPPORT_OBJ50_V1
  if((pTxData->txFlags & TMWSESN_TXFLAGS_STORE_DNP_DELAY) != 0)
  {
    TMWTYPES_UCHAR buf[2];
    TMWSESN *pSession =  pTxData->pSession;
    SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;

    TMWTYPES_USHORT processingDelay = (TMWTYPES_USHORT)
      (tmwtarg_getMSTime() - pSDNPSession->delayMeasurementRxTime);

    /* Use store function into a temporary buffer,
     * that takes care of byte order for this processor .
     * Then when copy is done in link layer it is correct
     */
    tmwtarg_store16(&processingDelay, buf);

    if((pTxData->txFlags & TMWSESN_TXFLAGS_DNP_AUTH_AGGR) == 0)
    {
      pSession->pChannel->pTprt->pTprtUpdateMsg(
        pSession->pChannel->pTprtContext, 8,
        buf, 2);  
    }
    else
    {  
#if SDNPDATA_SUPPORT_OBJ120
      sdnpauth_aggrDelay(pTxData, buf);
#endif
    } 
  }
#else
  TMWTARG_UNUSED_PARAM(pTxData);
#endif
}

/* function: _afterTxCallback
 * purpose: Called by link layer right before the first frame
 *  of the current fragment is transmitted
 * arguments:
 *  pUserData - user specified callback data, points to session
 *  pTxData - response data being transmitted
 * returns:
 *  void
 */
static void TMWDEFS_CALLBACK _afterTxCallback(
  TMWSESN_TX_DATA *pTxData)
{
  TMWSESN *pSession = (TMWSESN *)pTxData->pSession;
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  TMWTYPES_USHORT txFlags = pTxData->txFlags;

  /* If unsolicited response, set state */
  if((pTxData->pMsgBuf[0] & DNPDEFS_AC_UNSOLICITED) != 0)
  {
    pSDNPSession->unsolQueued = TMWDEFS_FALSE;

    /* Set flag that tells session it is waiting for an unsolicited
     * confirmation and go ahead and free transmit data structure
     * since we will never need it again.
     */
    pSDNPSession->unsolWaitingForConfirm = TMWDEFS_TRUE;

    pSDNPSession->unsolResponseContainsEvents = TMWDEFS_FALSE;
    if(txFlags & TMWSESN_TXFLAGS_CONTAINS_EVENTS)
      pSDNPSession->unsolResponseContainsEvents = TMWDEFS_TRUE;

#if SDNPDATA_SUPPORT_IDENT_UNSOL_RETRY
    if(pSDNPSession->unsolSendIdenticalRetry)
    {
      pSDNPSession->pUnsolLastResponse = (DNPCHNL_TX_DATA*)pTxData;
      pSDNPSession->pUnsolLastResponse->referenceCount++;
    }
#endif
    pTxData->txFlags |= TMWSESN_TXFLAGS_NO_RESPONSE;

    /* Start unsolicited confirmation timeout */
    tmwtimer_start(&pSDNPSession->unsolRetryTimer, 
      pSDNPSession->unsolConfirmTimeout,
      pSession->pChannel,
      _unsolConfirmTimeout, pSession);
      TMWSESN_STAT_CALLBACK_FUNC(pSession, TMWSESN_STAT_DNPUNSOL_SENT, TMWDEFS_NULL); 
  } 

#if SDNPDATA_KEEP_LAST_RESPONSE 
  else
  {
    /* If we still have a pointer to another response delete it.
     * If we are resending that last response, however, don't delete it 
     */
    if((pSDNPSession->pLastResponse != TMWDEFS_NULL)
      &&((TMWSESN_TX_DATA*)pSDNPSession->pLastResponse != pTxData))
    {
      dnpchnl_freeTxData((TMWSESN_TX_DATA*)pSDNPSession->pLastResponse); 
    }
    pSDNPSession->pLastResponse = (DNPCHNL_TX_DATA*)pTxData;
    pSDNPSession->pLastResponse->referenceCount++; 
  }
#endif
 
  
  /* Use the txFlags from pTxData, in case it was set above */
  if(pTxData->txFlags & TMWSESN_TXFLAGS_NO_RESPONSE) 
  {
    if(pTxData == pSDNPSession->dnp.pCurrentMessage)
      pSDNPSession->dnp.pCurrentMessage = TMWDEFS_NULL;
  }
 
#if SDNPDATA_SUPPORT_OBJ120 && SDNPCNFG_SUPPORT_SA_VERSION5  
  if(pSDNPSession->authenticationEnabled && (pSDNPSession->dnp.operateInV2Mode == TMWDEFS_FALSE))
    sdnpsa_msgSent(pSession);
#endif

  /* Process cold or warm restart requests */
  if((txFlags & TMWSESN_TXFLAGS_COLD_RESTART) != 0)
  {
    SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_RESTART_COLD);
    pSDNPSession->iin |= DNPDEFS_IIN_RESTART;
    sdnpdata_coldRestart(pSession);
    return;
  }
  else if((txFlags & TMWSESN_TXFLAGS_WARM_RESTART) != 0)
  {
    SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_RESTART_WARM);
    sdnpdata_warmRestart(pSession);
    return;
  }
}

/* function: sdnpsesn_failedUnsolTx */
void TMWDEFS_CALLBACK sdnpsesn_failedUnsolTx(
  TMWSESN *pSession)
{
   /* Unsolicited response, don't start the application confirm timer
    * since we know we did not send the response, but to make the behavior
    * similar, start a timer to wait application confirm timeout plus
    * the unsolicited retry delay time
    */
  TMWTYPES_MILLISECONDS delay;
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;

  pSDNPSession->unsolQueued = TMWDEFS_FALSE;

  /* Get retry period based on number of retries */
  delay = pSDNPSession->unsolRetryDelay;
  if(pSDNPSession->unsolNumRetries++ >= pSDNPSession->unsolMaxRetries)
  {
    /* To prevent NumRetries from wrapping past zero, just keep it at MaxRetries */
    pSDNPSession->unsolNumRetries = pSDNPSession->unsolMaxRetries;
    delay = pSDNPSession->unsolOfflineRetryDelay;

#if SDNPDATA_SUPPORT_IDENT_UNSOL_RETRY
    if((delay == UNSOL_NORETRY) 
      && (pSDNPSession->pUnsolLastResponse != TMWDEFS_NULL))
    {
      /* If we saved unsolicited response to be sent as an identical
        * retry, mark all of the events as not sent, and deallocate
        * the saved identical retry in xxcleanupEvents.
        */
      if(pSDNPSession->pUnsolLastResponse != TMWDEFS_NULL)
        sdnprbe_cleanupEvents(pSession, TMWDEFS_FALSE);
    }
#endif
  }

  /* Start unsolicited retry timer */
  tmwtimer_start(
    &pSDNPSession->unsolRetryTimer, (delay + pSDNPSession->unsolConfirmTimeout),
    pSession->pChannel, _unsolRetryTimeout, pSession);
}

/* function: _failedTxCallback
 * purpose: Called by link layer if transmission failed
 * arguments:
 *  pUserData - user specified callback data, points to session
 *  pTxData - response data being transmitted
 * returns:
 *  void
 */
static void TMWDEFS_CALLBACK _failedTxCallback(
  TMWSESN_TX_DATA *pTxData)
{
  DNPSESN *pDNPSession = (DNPSESN *)pTxData->pSession;

   /* If unsolicited response, don't start the application confirm timer
    * since we know we did not send the response, but to make the behavior
    * similar, start a timer to wait application confirm timeout plus
    * the unsolicited retry delay time
    */
  if((pTxData->pMsgBuf[0] & DNPDEFS_AC_UNSOLICITED) != 0)
  {
    sdnpsesn_failedUnsolTx((TMWSESN*)pDNPSession);
  } 
   
  pDNPSession->pCurrentMessage = TMWDEFS_NULL;

  /* Diagnostics */
  SDNPDIAG_ERROR(pDNPSession->tmw.pChannel, (TMWSESN*)pDNPSession, SDNPDIAG_APPL_TX);

  /* Dont call _processError here, it is called from dnpchnl.c _applError() */
}

/* function: _infoCallback */
static void TMWDEFS_CALLBACK _infoCallback(
  TMWSESN *pSession,
  TMWSCL_INFO sesnInfo)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;

  switch(sesnInfo)
  {
  case TMWSCL_INFO_ONLINE:
    {
      /* Specs say to send initial null unsolicited on restart  
       * Also allow sending of unsolicited when slave comes online
       * since this is the behavior of previous versions of the SCL
       * and is required by some customers.
       */
      if((pSDNPSession->iin & DNPDEFS_IIN_RESTART)
        ||(pSDNPSession->sendUnsolWhenOnline))
      {
        /* Send initial null response */
        if(pSDNPSession->unsolAllowed) 
        {
          /* In the sendUnsolWhenOnline case, make sure a previous UR is not waiting for a confirm */
          if((!pSDNPSession->unsolWaitingForConfirm)
            /* Protect against unsol being queued before link status request retries cause disconnect */
            && (!pSDNPSession->unsolQueued)
            /* if master does not clear the restart iin bit, but has confirmed the initial null UR */
            && (!pSDNPSession->unsolInitialNullConfirmed))
          {
            sdnpunsl_sendNullResponse(pSession, TMWDEFS_TRUE);
          }
          else if(pSDNPSession->sendUnsolWhenOnline)
          {
            /* This will make sure NULL Unsolicited will be sent before any unsolicited event responses. */
            pSDNPSession->unsolInitialNullConfirmed = TMWDEFS_FALSE;
            sdnpunsl_sendNullResponse(pSession, TMWDEFS_TRUE);
          }
        }
      }
    }
    break;
#if  SDNPDATA_SUPPORT_OBJ120
  case TMWSCL_INFO_TIMEOUT:
  case TMWSCL_INFO_CLOSED:
    {
      if(pSDNPSession->authenticationEnabled)
        sdnpauth_applTimeout(pSDNPSession);
    }
    break;
#endif
  default:
    break;
  }
}

/* function: _processConfirm
 * purpose: Process received application layer confirmation
 * arguments:
 *  pSession - session confirmation was received from
 *  pRxFragment - received fragment
 * returns:
 *  void
 */
static void TMWDEFS_LOCAL _processConfirm(
  TMWSESN *pSession,
  TMWSESN_RX_DATA *pRxFragment)
{
  /* Get pointer to application layer session info */
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  TMWCHNL *pChannel = pSession->pChannel;
  TMWTYPES_UCHAR sequenceNumber;
  DNPUTIL_RX_MSG msg;

  /* Parse application header */
  dnputil_parseApplHeader(pRxFragment, &msg, TMWDEFS_FALSE);

  /* Parse application header */
  sequenceNumber = (TMWTYPES_UCHAR)(msg.ac & DNPDEFS_AC_SEQUENCE_MASK);

  /* Process All Stations IIN bit */
  if(pSDNPSession->allStationsConfirmRequired)
  {
    pSDNPSession->iin &= ~DNPDEFS_IIN_ALL_STATIONS;
    pSDNPSession->allStationsConfirmRequired = TMWDEFS_FALSE;
  }

  /* What type of confirmation did we receive */
  if((msg.ac & DNPDEFS_AC_UNSOLICITED) == DNPDEFS_AC_UNSOLICITED)
  {
    /* Unsolicited, make sure we are waiting for one */
    if(pSDNPSession->unsolWaitingForConfirm)
    {
      if(pSDNPSession->lastUnsolSequenceNumber == sequenceNumber)
      {
        /* Cancel retry timer since we have received a valid confirmation */
        tmwtimer_cancel(&pSDNPSession->unsolRetryTimer);
        TMWSESN_STAT_CALLBACK_FUNC(pSession, TMWSESN_STAT_DNPUNSOL_CONFIRM, TMWDEFS_NULL); 

#if SDNPDATA_SUPPORT_OBJ120 
        /* tell authentication that the confirm was received */
        sdnpauth_applConfirm(pSDNPSession);
#endif

        /* Unsolicited Response */
        pSDNPSession->unsolNumRetries = 0;
        pSDNPSession->unsolInitialNullConfirmed = TMWDEFS_TRUE;
        pSDNPSession->unsolWaitingForConfirm = TMWDEFS_FALSE;
        pSDNPSession->unsolResponseContainsEvents = TMWDEFS_FALSE;

        /* Remove confirmed events from event queues */
        sdnprbe_cleanupEvents(pSession, TMWDEFS_TRUE);

        /* Update unsolicted event status */
        sdnpunsl_updateStatus(pSession);
      }
      else
      {
        SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_UNSOL_CONF_SEQ);
      }
    }
    else
    {
      SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_UNSOL_CONF_UNEXP);
    }
  }
  else
  {
#if SDNPDATA_KEEP_LAST_RESPONSE
    /* Receiving an application confirm means that receiving a duplicate of the last request
     * we received no longer should cause a resend of that last response. This prevents problems
     * with multifragment responses resending the first fragment.
     */
    pSDNPSession->notDuplicateEligible = TMWDEFS_TRUE;
#endif

    if(pSDNPSession->dnp.pCurrentMessage != TMWDEFS_NULL)
    {
      if(pSDNPSession->lastRespSequenceNumber == sequenceNumber)
      {
#if SDNPDATA_SUPPORT_OBJ120 
        /* tell authentication that the confirm was received */
        sdnpauth_applConfirm(pSDNPSession);
#endif

        /* Tell transport layer that we are done with this message. Transport layer
         * will only have a pointer to this if it required a link layer confirm
         * that was never received, but instead we receive the application confirm to the
         * message and are therefore finished with it.
         */
        pChannel->pTprt->pTprtCancel(pChannel->pTprtContext, pSDNPSession->dnp.pCurrentMessage);

        /* Application Response */
        tmwdlist_removeEntry(&pChannel->messageQueue, (TMWDLIST_MEMBER *)pSDNPSession->dnp.pCurrentMessage);

        dnpchnl_freeTxData(pSDNPSession->dnp.pCurrentMessage);

        pSDNPSession->dnp.pCurrentMessage = TMWDEFS_NULL;

        pSDNPSession->multiFragWaitingForConfirm = TMWDEFS_FALSE;

        /* Remove confirmed events from event queues */
        sdnprbe_cleanupEvents(pSession, TMWDEFS_TRUE);

        /* Update unsolicted event status */
        sdnpunsl_updateStatus(pSession);
      }
      else
      {
        SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_RESPONSE_SEQ);
      }
    }
    else
    {
      SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_RESPONSE_UNEXP);
    }
  }

  /* See if there is anything waiting to be sent */
  _processNextMessage(pSession);
}

/* function: _processReadRequest
 * purpose: Process read requests
 * arguments:
 *  pSession - session request was received on
 *  pRxFragment - received request data
 * returns:
 *  void
 */
static void TMWDEFS_LOCAL _processReadRequest(
  TMWSESN *pSession)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;

  if(!pSDNPSession->readPending)
  {
    /* Get application control and request sequence number from message */
    pSDNPSession->recvSequenceNumber = (TMWTYPES_UCHAR)
      (pSDNPSession->lastRcvdRequest.ac & DNPDEFS_AC_SEQUENCE_MASK);

    pSDNPSession->respSequenceNumber = pSDNPSession->recvSequenceNumber;

    /* Initialize read response state */
    pSDNPSession->readPass = 0;
    pSDNPSession->readPointIndex = 0;
    pSDNPSession->readGroupIndex = 0;
    pSDNPSession->readMultiFragNumberOfPoints = 0;
    pSDNPSession->readStatus = SDNPSESN_READ_COMPLETE;

    pSDNPSession->eventObjectHeader.group = DNPDEFS_OBJ_60_CLASS_SCANS;
    pSDNPSession->eventObjectHeader.variation = 0;
    pSDNPSession->eventObjectHeader.qualifier = DNPDEFS_QUAL_ALL_POINTS;
    pSDNPSession->eventObjectHeader.firstPointNumber = 0;
    pSDNPSession->eventObjectHeader.lastPointNumber = 0;
    pSDNPSession->eventObjectHeader.numberOfPoints = 0;
    pSDNPSession->readEventClassesRequested = TMWDEFS_CLASS_MASK_NONE;
   
    pSDNPSession->staticObjectHeader.firstPointNumber = 0;
    pSDNPSession->staticObjectHeader.lastPointNumber = 0;
    pSDNPSession->staticObjectHeader.numberOfPoints = 0;
    pSDNPSession->readStaticObjectsRequested = TMWDEFS_FALSE;
    
    pSDNPSession->readPending = TMWDEFS_TRUE;
    pSDNPSession->multiFragWaitingForConfirm = TMWDEFS_FALSE;
    
#if SDNPDATA_SUPPORT_OBJ2 && SDNPDATA_SUPPORT_OBJ4
    pSDNPSession->readObj2AndObj4 = SDNPSESN_NOREAD;
#endif
  }

  /* See if an unsolicited confirm is pending, if so we have to wait until we
    * either receive the confirmation or the message times out to process this
    * read request.
    */
  if(pSDNPSession->unsolWaitingForConfirm)
  {
    return;
  }

#if SDNPDATA_SUPPORT_IDENT_UNSOL_RETRY
    /* If we saved unsolicited response to be sent as an identical
     * retry, mark all of the events as not sent, and deallocate
     * the saved identical retry in xxcleanupEvents.
     */
    if(pSDNPSession->pUnsolLastResponse != TMWDEFS_NULL)
      sdnprbe_cleanupEvents(pSession, TMWDEFS_FALSE);
#endif

  /* Determine whether to set FIR bit */
  if(pSDNPSession->readStatus != SDNPSESN_READ_MORE_DATA)
  {
    /* Build the response (or at least the first fragment of it) */
    _buildReadResponse(pSession, TMWDEFS_TRUE);
  }
  else
  {
    /* check to see if still waiting for an appl confirm
     * this is necessary because when slave sends challenge response it causes _processNextMessage to be called 
     * which would cause the next fragment to be sent without receiving an appl confirm
     */
    if(!pSDNPSession->multiFragWaitingForConfirm)
    {
      /* Build the next fragment of the response */
      _buildReadResponse(pSession, TMWDEFS_FALSE);
    }
  }
}


/* function: _writeSupported
 * purpose: Determine if this write is enabled for broadcast or device specific address
 */
static TMWTYPES_BOOL TMWDEFS_LOCAL _writeEnabled(SDNPSESN *pSDNPSession, DNPUTIL_OBJECT_HEADER *pHeader)
{
  TMWTYPES_ULONG enabledWrites;

  if(pSDNPSession->lastRcvdRequest.isBroadcast)
    enabledWrites = pSDNPSession->enabledBroadcastWrites;
  else
    enabledWrites = pSDNPSession->enabledWrites;

  if(pHeader->group == DNPDEFS_OBJ_50_TIME_AND_DATE)
  {
    if(pHeader->variation == 1)
    { 
      /* write clock */
      if((enabledWrites & SDNPSESN_ENABLE_WRITE_CLOCK) == 0)
        return TMWDEFS_FALSE;
    }
    if(pHeader->variation == 3)
    {
      /* write last recorded time */
      if((enabledWrites & SDNPSESN_ENABLE_WRITE_LRTIME) == 0)
        return TMWDEFS_FALSE;
    }
  }
  else if((pHeader->group == DNPDEFS_OBJ_80_IIN_BITS)
    && (pHeader->numberOfPoints == 1)
    && (pHeader->firstPointNumber == DNPDEFS_IIN_RESTART_INDEX))
  {
    /* write restart IIN bit */
    if((enabledWrites & SDNPSESN_ENABLE_WRITE_CRESTART) == 0)
        return TMWDEFS_FALSE;
  }
  else
  {
    /* any other write */
    if((enabledWrites & SDNPSESN_ENABLE_WRITE_OTHER) == 0)
      return TMWDEFS_FALSE;
  }

  return TMWDEFS_TRUE;
}


/* function: _processWriteRequest
 * purpose: Process write requests
 * arguments:
 *  pSession - session request was received on
 *  pRxFragment - received request data
 * returns:
 *  void
 */
static void TMWDEFS_LOCAL _processWriteRequest(
  TMWSESN *pSession)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  DNPUTIL_RX_MSG *pRxMessage = &pSDNPSession->lastRcvdRequest;
  DNPCHNL *pDNPChannel = (DNPCHNL *)pSession->pChannel;
  TMWSESN_TX_DATA *pResponse;
  int index;
  TMWTYPES_BOOL processed;
  DNPUTIL_OBJECT_HEADER header;

  pResponse = dnpchnl_newTxData((TMWCHNL *)pDNPChannel,
    pSession, pDNPChannel->txFragmentSize, pRxMessage->rxAddress);

  if(pResponse == TMWDEFS_NULL)
  {
    return;
  }

#if TMWCNFG_SUPPORT_DIAG
  pResponse->pMsgDescription = "Write Response";
#endif

  pSDNPSession->recvSequenceNumber = (TMWTYPES_UCHAR)(pRxMessage->ac & DNPDEFS_AC_SEQUENCE_MASK);
  pSDNPSession->respSequenceNumber = pSDNPSession->recvSequenceNumber;

  _initializeResponse(pSession, pResponse, TMWDEFS_TRUE, TMWDEFS_TRUE);

  while(pRxMessage->offset < pRxMessage->msgLength)
  {
    if(!dnputil_parseObjectHeader(pRxMessage, 0, &header))
    {
      DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_PARSE_HEADER);
      pSDNPSession->iin |= DNPDEFS_IIN_OUT_OF_RANGE;
      break;
    }

    /* Diagnostics */
    DNPDIAG_SHOW_OBJECT_HEADER(pSession, &header);
  
    index = 0;
    processed = TMWDEFS_FALSE;
    while(_sdnpObjWriteFuncTable[index].pWriteFunc != TMWDEFS_NULL)
    {
      if((_sdnpObjWriteFuncTable[index].group == header.group)
        && (_sdnpObjWriteFuncTable[index].allVariations
        || (_sdnpObjWriteFuncTable[index].variation == header.variation)))
      {
        processed = TMWDEFS_TRUE;

        /*   validate message size based on header info */
        if(!_validateMessageSize(pSession, _sdnpObjWriteFuncTable[index].dataLength, &header, (TMWTYPES_USHORT)(pRxMessage->msgLength-pRxMessage->offset)))
        { 
          pRxMessage->offset = pRxMessage->msgLength;
          break;
        }
        
        if(!_writeEnabled(pSDNPSession, &header))
        {
          if(pSDNPSession->lastRcvdRequest.isBroadcast)
          {
            DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_BRDCST_DISABLED);
          }
          else
          {
            DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_FC_DISABLED);
          }

          pRxMessage->offset = pRxMessage->msgLength;
          break;
        }

        if(!_sdnpObjWriteFuncTable[index].pWriteFunc(pSession, pRxMessage, pResponse, &header))
        {
          /* Requested object and variation are supported for this function
           * code but something in the message was out of range.
           */
          SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_WRITE);
          pSDNPSession->iin |= DNPDEFS_IIN_OUT_OF_RANGE;
        }

        break;
      }

      index += 1;
    }

    if(!processed)
    {
      /* This object group/variation is not supported for this function code */
      SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_WRITE_OBJVAR);
      pSDNPSession->iin |= DNPDEFS_IIN_OBJECT_UNKNOWN;
      pRxMessage->offset = pRxMessage->msgLength;
    }
  }

  /* Done with this message */
  pSDNPSession->pendingRequest = TMWDEFS_FALSE;

  /* Send response */
  _sendResponse(pSession, TMWDEFS_FALSE, pRxMessage, pResponse);
}

/* function: _processSelectRequest
 * purpose: Process select requests
 * arguments:
 *  pSession - session request was received on
 *  pRxFragment - received request data
 * returns:
 *  void
 */
static void TMWDEFS_LOCAL _processSelectRequest(
  TMWSESN *pSession)
{
  TMWSESN_TX_DATA *pResponse;
  DNPUTIL_OBJECT_HEADER header;
  TMWTYPES_BOOL processed;
  TMWTYPES_BOOL success;
  TMWTYPES_UCHAR status; 
  int index;
  int numberOfPoints;
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  DNPUTIL_RX_MSG *pRxMessage = &pSDNPSession->lastRcvdRequest;
  DNPCHNL *pDNPChannel = (DNPCHNL *)pSession->pChannel;
  
  /* Process sequence number */
  pSDNPSession->recvSequenceNumber = (TMWTYPES_UCHAR)(pRxMessage->ac & DNPDEFS_AC_SEQUENCE_MASK);;
  pSDNPSession->respSequenceNumber = pSDNPSession->recvSequenceNumber;
  
  /* Table 4-1 in Application Layer Spec V2 Part 1 December 2007
   * If a selection is in effect and
   * If this sequence number is the same as previous but the data is different discard new select.
   * If sequence number is not same, copy new select and start select timer
   * If this is a duplicate select with same sequence number, don't
   * restart timer, just send response with status zero.
   * NOTE: when SDNPDATA_KEEP_LAST_RESPONSE is TRUE, duplicate select request will not get here,
   * the last response will be resent before processing request as a select. If that is not
   * defined, and the select is still active, send select response, but do not restart the timer.
   */
  if(tmwtimer_isActive(&pSDNPSession->selectTimer))
  {
    if(pSDNPSession->recvSequenceNumber == pSDNPSession->selectSequenceNumber)
    {
      if((pSDNPSession->selectBufferLength != pRxMessage->msgLength)
      ||(memcmp(pSDNPSession->selectBuffer, pRxMessage->pMsgBuf, pSDNPSession->selectBufferLength) != 0))
      {
        /* sequence number matched but data did not, discard, do not restart timer, do not respond */
        SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_SELECT); 
        pSDNPSession->pendingRequest = TMWDEFS_FALSE;
        return;
      }
      else
      {
        /* If this is a duplicate select with same sequence just send response with status zero */
         /* Initialize response */
        pResponse = dnpchnl_newTxData((TMWCHNL *)pDNPChannel,
          pSession, pDNPChannel->txFragmentSize, pRxMessage->rxAddress);

        if(pResponse == TMWDEFS_NULL)
        {
          return;
        } 

        /* Initialize response */
        _initializeResponse(pSession, pResponse, TMWDEFS_TRUE, TMWDEFS_TRUE);

        /* Copy select message into response */
        memcpy(&pResponse->pMsgBuf[pResponse->msgLength], pRxMessage->pMsgBuf +2, pRxMessage->msgLength -2);
        pResponse->msgLength = (TMWTYPES_USHORT)(pResponse->msgLength + pRxMessage->msgLength-2);
     
        /* Done with this message */
        pSDNPSession->pendingRequest = TMWDEFS_FALSE;

        /* Send response */
        _sendResponse(pSession, TMWDEFS_FALSE, pRxMessage, pResponse);
        return;
      }
    }
    else
    {
      /* There is an old select pending, but this is a new select with a different sequence number 
       * Tell the database to cancel the first select
       */
      _cancelSelect(pSDNPSession);
      pSDNPSession->selectBufferLength = 0;
    }
  }

  /* Initialize processing if required */
  index = 0;
  while(_sdnpObjSelOpFuncTable[index].group != 0)
  {
    if(_sdnpObjSelOpFuncTable[index].pInitFunc != TMWDEFS_NULL)
      _sdnpObjSelOpFuncTable[index].pInitFunc(pSession);

    index += 1;
  }

  /* Initialize response */
  pResponse = dnpchnl_newTxData((TMWCHNL *)pDNPChannel,
    pSession, pDNPChannel->txFragmentSize, pRxMessage->rxAddress);

  if(pResponse == TMWDEFS_NULL)
  {
    return;
  }

#if TMWCNFG_SUPPORT_DIAG
  pResponse->pMsgDescription = "Select Response";
#endif

  _initializeResponse(pSession, pResponse, TMWDEFS_TRUE, TMWDEFS_TRUE);

  /* Process objects */
  success = TMWDEFS_TRUE;
  status = DNPDEFS_CTLSTAT_SUCCESS;
  numberOfPoints = 0;
  while(pRxMessage->offset < pRxMessage->msgLength)
  {
    /* Parse object header */
    if(!dnputil_parseObjectHeader(pRxMessage, 0, &header))
    {
      DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_PARSE_HEADER);
      pSDNPSession->iin |= DNPDEFS_IIN_OUT_OF_RANGE;
      success = TMWDEFS_FALSE;
      break;
    }

    /* Diagnostics */
    DNPDIAG_SHOW_OBJECT_HEADER(pSession, &header);

    index = 0;
    processed = TMWDEFS_FALSE;
    while(_sdnpObjSelOpFuncTable[index].group != 0)
    {
      if((_sdnpObjSelOpFuncTable[index].group == header.group)
        && (_sdnpObjSelOpFuncTable[index].variation == header.variation))
      {
        if(_sdnpObjSelOpFuncTable[index].pSelectFunc != TMWDEFS_NULL)
        {
          processed = TMWDEFS_TRUE;
         
          /*  validate message size based on header info */
          if(!_validateMessageSize(pSession, _sdnpObjSelOpFuncTable[index].dataLength, &header, (TMWTYPES_USHORT)(pRxMessage->msgLength-pRxMessage->offset)))
          { 
            success = TMWDEFS_FALSE;
            SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_SELECT);
            pRxMessage->offset = pRxMessage->msgLength;
            break;
          }

          if(!_sdnpObjSelOpFuncTable[index].pSelectFunc(
            pSession, pRxMessage, pResponse, &header, (TMWTYPES_UCHAR)numberOfPoints, status))
          {
            /* Requested object and variation are supported for this function
             * code but something in the select request failed. 
             * IIN bits were set according to select operate rules
             */
            success = TMWDEFS_FALSE;
            SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_SELECT);
          }
          /* Keep track of the number of points so far, so TOO_MANY_OPS status will be set properly */
          numberOfPoints += header.numberOfPoints;
        }

        break;
      }

      index += 1;
    }

    if(!processed)
    {
      /* This object group/variation is not supported for this function code */
      SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_SELECT_OBJVAR);
      pSDNPSession->iin |= DNPDEFS_IIN_OBJECT_UNKNOWN;
      success = TMWDEFS_FALSE;
      break;
    }
  }

  /* Copy select request so we can compare the operate request against
   * it and confirm they are identical
   */
  if(success)
  {
    if(pRxMessage->msgLength <= SDNPSESN_SELECT_BUFFER_SIZE)
    {
      pSDNPSession->selectSequenceNumber = pSDNPSession->recvSequenceNumber;
      pSDNPSession->selectBufferLength = pRxMessage->msgLength;
      memcpy(pSDNPSession->selectBuffer,
        pRxMessage->pMsgBuf, pRxMessage->msgLength);

      tmwtimer_start(&pSDNPSession->selectTimer,
        pSDNPSession->selectTimeout, pSession->pChannel,        
        _selectTimeout, pSession);
    }
    else
    {
      SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_SELECT);
      pSDNPSession->iin |= DNPDEFS_IIN_OUT_OF_RANGE;
    }
  }

  /* Done with this message */
  pSDNPSession->pendingRequest = TMWDEFS_FALSE;

  /* Send response */
  _sendResponse(pSession, TMWDEFS_FALSE, pRxMessage, pResponse);
}

/* function: _processOperateRequest
 * purpose: Process operate requests
 * arguments:
 *  pSession - session request was received on
 *  pRxFragment - received request data
 * returns:
 *  void
 */
static void TMWDEFS_LOCAL _processOperateRequest(
  TMWSESN *pSession)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  DNPUTIL_RX_MSG *pRxMessage = &pSDNPSession->lastRcvdRequest;
  DNPCHNL *pDNPChannel = (DNPCHNL *)pSession->pChannel;
  TMWSESN_TX_DATA *pResponse;
  DNPUTIL_OBJECT_HEADER header;
  TMWTYPES_UCHAR sequenceNumber;
  TMWTYPES_BOOL processed;
  TMWTYPES_BOOL needToCancelSelect;
  TMWTYPES_UCHAR status;
  int index;
  int numberOfPoints;

  /* Initialize processing if required */
  index = 0;
  while(_sdnpObjSelOpFuncTable[index].group != 0)
  {
    if(_sdnpObjSelOpFuncTable[index].pInitFunc != TMWDEFS_NULL)
      _sdnpObjSelOpFuncTable[index].pInitFunc(pSession);

    index += 1;
  }

  /* Initialize response */
  pResponse = dnpchnl_newTxData((TMWCHNL *)pDNPChannel,
    pSession, pDNPChannel->txFragmentSize, pRxMessage->rxAddress);

  if(pResponse == TMWDEFS_NULL)
  {
    return;
  }

#if TMWCNFG_SUPPORT_DIAG
  pResponse->pMsgDescription = "Operate Response";
#endif

  /* Get sequence number */
  sequenceNumber = (TMWTYPES_UCHAR)(pRxMessage->ac & DNPDEFS_AC_SEQUENCE_MASK);

  /* Validate select */
  status = DNPDEFS_CTLSTAT_SUCCESS;
  needToCancelSelect = TMWDEFS_FALSE;
  if(pSDNPSession->selectBufferLength == 0)
  {
    /* No select pending */
    status = DNPDEFS_CTLSTAT_NO_SELECT;
  }
  #if !SDNPCNFG_DISABLE_SELOP_SEQ_CHECK
  else if(sequenceNumber != sdnputil_nextSequenceNumber(pSDNPSession->selectSequenceNumber))
  {
    /* Sequence number does not match */
    needToCancelSelect = TMWDEFS_TRUE;
    status = DNPDEFS_CTLSTAT_NO_SELECT;
  }
#else
  /* Disable checking of consecutive application sequence numbers between a select and operate request? */
  else if((sequenceNumber != sdnputil_nextSequenceNumber(pSDNPSession->selectSequenceNumber))
    &&(!pSDNPSession->disableSelOpSeqCheck))
  {
    /* Sequence number does not match */
    needToCancelSelect = TMWDEFS_TRUE;
      status = DNPDEFS_CTLSTAT_NO_SELECT;
  }
#endif
  else if(!tmwtimer_isActive(&pSDNPSession->selectTimer))
  {   
    /* If this is a duplicate operate request allow it as specified
     * in TC2000-002. Respond with echoed message, status 0, but don't operate control.
     * NOTE: when SDNPDATA_KEEP_LAST_RESPONSE is TRUE, duplicate request will not get here,
     * the last response will be resent before processing request as an operate. This code
     * must stay here in case that define is set to FALSE.
     */
    if((sequenceNumber == pSDNPSession->recvSequenceNumber)
      &&(pSDNPSession->selectBufferLength == pRxMessage->msgLength)
      &&(memcmp(pSDNPSession->selectBuffer + 2, pRxMessage->pMsgBuf + 2,
         pSDNPSession->selectBufferLength - 2) == 0))
    { 
  
      pSDNPSession->recvSequenceNumber = sequenceNumber;
      pSDNPSession->respSequenceNumber = pSDNPSession->recvSequenceNumber;

      /* Initialize response */
      _initializeResponse(pSession, pResponse, TMWDEFS_TRUE, TMWDEFS_TRUE);

      /* Copy operate message into response */
      memcpy(&pResponse->pMsgBuf[pResponse->msgLength], pRxMessage->pMsgBuf +2, pRxMessage->msgLength -2);
	    pResponse->msgLength = (TMWTYPES_USHORT)(pResponse->msgLength + pRxMessage->msgLength-2);
 
	    /* Done with this message */
      pSDNPSession->pendingRequest = TMWDEFS_FALSE;

      /* Send response */
      _sendResponse(pSession, TMWDEFS_FALSE, pRxMessage, pResponse);
      return;
    }

    /* Select timer has expired */
    status = DNPDEFS_CTLSTAT_TIMEOUT;
  }
  else if((pSDNPSession->selectBufferLength != pRxMessage->msgLength)
    || (memcmp(pSDNPSession->selectBuffer + 2, pRxMessage->pMsgBuf + 2,
       pSDNPSession->selectBufferLength - 2) != 0))
  {
    /* Select and operate requests do not match */
    status = DNPDEFS_CTLSTAT_NO_SELECT;
    needToCancelSelect = TMWDEFS_TRUE;
  }
  
  tmwtimer_cancel(&pSDNPSession->selectTimer);

  if(needToCancelSelect)
  {
    _cancelSelect(pSDNPSession);
  }
  
  /* If valid operate, don't clear pending select length 
   * to allow for reception of duplicate operate request
   * as specified in TB2000-002
   */
  if(status != DNPDEFS_CTLSTAT_SUCCESS)
  {
    /* Clear length, so select will not still be pending. */
    pSDNPSession->selectBufferLength = 0;
  }

  pSDNPSession->recvSequenceNumber = sequenceNumber;
  pSDNPSession->respSequenceNumber = pSDNPSession->recvSequenceNumber;

  /* Initialize response */
  _initializeResponse(pSession, pResponse, TMWDEFS_TRUE, TMWDEFS_TRUE);

  numberOfPoints = 0;

  /* Don't process this message again even from afterTxCallback */
  pSDNPSession->pendingRequest = TMWDEFS_FALSE;

  /* Process operate request */
  while(pRxMessage->offset < pRxMessage->msgLength)
  {
    /* Parse object header */
    if(!dnputil_parseObjectHeader(pRxMessage, 0, &header))
    {
      DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_PARSE_HEADER);
      pSDNPSession->iin |= DNPDEFS_IIN_OUT_OF_RANGE;
      break;
    }

    /* Diagnostics */
    DNPDIAG_SHOW_OBJECT_HEADER(pSession, &header);

    /* Parse object data */
    index = 0;
    processed = TMWDEFS_FALSE;
    while(_sdnpObjSelOpFuncTable[index].group != 0)
    {
      if((_sdnpObjSelOpFuncTable[index].group == header.group)
        && (_sdnpObjSelOpFuncTable[index].variation == header.variation))
      { 
        processed = TMWDEFS_TRUE;

        /*  validate message size based on header info */
        if(!_validateMessageSize(pSession, _sdnpObjSelOpFuncTable[index].dataLength, &header, (TMWTYPES_USHORT)(pRxMessage->msgLength-pRxMessage->offset)))
        { 
          pRxMessage->offset = pRxMessage->msgLength;
          break;
        }

        if(!_sdnpObjSelOpFuncTable[index].pOperateFunc(
          pSession, pRxMessage, pResponse, &header, (TMWTYPES_UCHAR)numberOfPoints, status))
        {   
          /* Requested object and variation are supported for this function
           * code but something in the operate request failed. 
           * IIN bits were set according to select operate rules
           */
          SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_OPERATE);
        }
        /* Keep track of the number of points so far, so TOO_MANY_OPS status will be set properly */
        numberOfPoints += header.numberOfPoints;
        break;
      }

      index += 1;
    }

    if(!processed)
    {
      /* This object group/variation is not supported for this function code */
      SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_OPER_OBJVAR);
      pSDNPSession->iin |= DNPDEFS_IIN_OBJECT_UNKNOWN;
      
      /* Clear length, so this will not still be valid. */
      pSDNPSession->selectBufferLength = 0;
      break;
    }
  }

  /* Send response */
  _sendResponse(pSession, TMWDEFS_FALSE, pRxMessage, pResponse);
}

/* function: _processDirectOperateRequest
 * purpose: Process direct operate requests
 * arguments:
 *  pSession - session request was received on
 *  pRxFragment - received request data
 * returns:
 *  void
 */
static void TMWDEFS_LOCAL _processDirectOperateRequest(
  TMWSESN *pSession)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  DNPUTIL_RX_MSG *pRxMessage = &pSDNPSession->lastRcvdRequest;
  DNPCHNL *pDNPChannel = (DNPCHNL *)pSession->pChannel;
  TMWSESN_TX_DATA *pResponse;
  DNPUTIL_OBJECT_HEADER header;
  TMWTYPES_BOOL processed;
  TMWTYPES_UCHAR status;
  int index;
  int numberOfPoints;
  TMWTYPES_BOOL noResp;

  /* Initialize processing if required */
  index = 0;
  while(_sdnpObjSelOpFuncTable[index].group != 0)
  {
    if(_sdnpObjSelOpFuncTable[index].pInitFunc != TMWDEFS_NULL)
      _sdnpObjSelOpFuncTable[index].pInitFunc(pSession);

    index += 1;
  }

  /* Parse application header */
  pSDNPSession->recvSequenceNumber = (TMWTYPES_UCHAR)(pRxMessage->ac & DNPDEFS_AC_SEQUENCE_MASK);
  pSDNPSession->respSequenceNumber = pSDNPSession->recvSequenceNumber;

  /* Initialize response, it's easiest to generate one even if we will not send it */
  pResponse = dnpchnl_newTxData((TMWCHNL *)pDNPChannel,
    pSession, pDNPChannel->txFragmentSize, pRxMessage->rxAddress);

  if(pResponse == TMWDEFS_NULL)
  {
    return;
  }

  _initializeResponse(pSession, pResponse, TMWDEFS_TRUE, TMWDEFS_TRUE);
#if TMWCNFG_SUPPORT_DIAG
  pResponse->pMsgDescription = "Direct Operate Response";
#endif

  /* Process request one object at a time */
  status = DNPDEFS_CTLSTAT_SUCCESS;
  numberOfPoints = 0;

  /* Don't process this message again even from afterTxCallback */
  pSDNPSession->pendingRequest = TMWDEFS_FALSE;

  while(pRxMessage->offset < pRxMessage->msgLength)
  {
    if(!dnputil_parseObjectHeader(pRxMessage, 0, &header))
    {
      DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_PARSE_HEADER);
      pSDNPSession->iin |= DNPDEFS_IIN_OUT_OF_RANGE;
      break;
    }

    /* Diagnostics */
    DNPDIAG_SHOW_OBJECT_HEADER(pSession, &header);

    index = 0;
    processed = TMWDEFS_FALSE;
    while(_sdnpObjSelOpFuncTable[index].group != 0)
    {
      if((_sdnpObjSelOpFuncTable[index].group == header.group)
        && (_sdnpObjSelOpFuncTable[index].variation == header.variation))
      { 
        processed = TMWDEFS_TRUE;

        /*   validate message size based on header info */
        if(!_validateMessageSize(pSession, _sdnpObjSelOpFuncTable[index].dataLength, &header, (TMWTYPES_USHORT)(pRxMessage->msgLength-pRxMessage->offset)))
        {  
          pRxMessage->offset = pRxMessage->msgLength;
          break;
        }

        if(!_sdnpObjSelOpFuncTable[index].pOperateFunc(
          pSession, pRxMessage, pResponse, &header, (TMWTYPES_UCHAR)numberOfPoints, status))
        {
          /* Requested object and variation are supported for this function
           * code but something in the direct operate request failed. 
           * IIN bits were set according to select operate rules
           */
          SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_DIR_OPERATE);
        }
        /* Keep track of the number of points so far, so TOO_MANY_OPS status will be set properly */
        numberOfPoints += header.numberOfPoints;
        break;
      }

      index += 1;
    }

    if(!processed)
    {
      /* This object group/variation is not supported for this function code */
      SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_DIRECT_OBJVAR);
      pSDNPSession->iin |= DNPDEFS_IIN_OBJECT_UNKNOWN;
      break;
    }
  }

  /* If reply required, send it */
  noResp = (TMWTYPES_BOOL)((pRxMessage->fc == DNPDEFS_FC_DIRECT_OP_NOACK) ? TMWDEFS_TRUE : TMWDEFS_FALSE);

  /* Send response */
  _sendResponse(pSession, noResp, pRxMessage, pResponse);
}

#if SDNPDATA_SUPPORT_OBJ20
/* function: _processFreezeRequest
 * purpose: Process counter freeze requests
 * arguments:
 *  pSession - session request was received on
 *  pRxFragment - received request data
 * returns:
 *  void
 */
static void TMWDEFS_LOCAL _processFreezeRequest(
  TMWSESN *pSession)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  DNPUTIL_RX_MSG *pRxMessage = &pSDNPSession->lastRcvdRequest;
  DNPCHNL *pDNPChannel = (DNPCHNL *)pSession->pChannel;
  TMWSESN_TX_DATA *pResponse;  
  TMWTYPES_BOOL noResp;
#if SDNPDATA_SUPPORT_OBJ21
  TMWTYPES_MS_SINCE_70 msSince70;
  TMWDTIME freezeTime;
  TMWTYPES_ULONG freezeInterval = 0;
  DNPDATA_FREEZE_TIME_DATE_FIELD timeDateEnum = DNPDATA_FREEZE_TIME_DATE_FIELD_ZERO;
#endif

  pSDNPSession->recvSequenceNumber = (TMWTYPES_UCHAR)(pRxMessage->ac & DNPDEFS_AC_SEQUENCE_MASK);
  pSDNPSession->respSequenceNumber = pSDNPSession->recvSequenceNumber;

#if SDNPDATA_SUPPORT_OBJ21
  while(pRxMessage->offset < pRxMessage->msgLength)
  {
    DNPUTIL_OBJECT_HEADER header;
    TMWTYPES_BOOL processed;
    int index;

    if(!dnputil_parseObjectHeader(pRxMessage, 0, &header))
    {
      DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_PARSE_HEADER);
      pSDNPSession->iin |= DNPDEFS_IIN_OUT_OF_RANGE;
      break;
    }

    /* Diagnostics */
    DNPDIAG_SHOW_OBJECT_HEADER(pSession, &header);
      
    if (header.group == DNPDEFS_OBJ_50_TIME_AND_DATE)
    {
      if (header.qualifier != DNPDEFS_QUAL_8BIT_LIMITED_QTY)
      {
        DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_QUALIFIER);
        pSDNPSession->iin |= DNPDEFS_IIN_OUT_OF_RANGE;
        break;
      }

      /* Check the Time-date field for special handling */
      dnpdtime_readMsSince70(&msSince70, &pRxMessage->pMsgBuf[pRxMessage->offset]);      
      pRxMessage->offset += 6;

      tmwtarg_get32(&pRxMessage->pMsgBuf[pRxMessage->offset], &freezeInterval);
      pRxMessage->offset += 4;

      if ((msSince70.leastSignificant == 0) && (msSince70.mostSignificant == 0))
      {
        /* */
        timeDateEnum = DNPDATA_FREEZE_TIME_DATE_FIELD_ZERO;
        tmwdtime_getDateTime(pSession, &freezeTime);
      }
      else if ((msSince70.leastSignificant == 0xffff) && (msSince70.mostSignificant == 0xffffffff))
      {
        timeDateEnum = DNPDATA_FREEZE_TIME_DATE_FIELD_CANCEL;
        tmwdtime_getDateTime(pSession, &freezeTime);
      }
      else
      {
        timeDateEnum = DNPDATA_FREEZE_TIME_DATE_FIELD_NON_ZERO;
        dnpdtime_msSince70ToDateTime(&freezeTime, &msSince70);

        DNPDIAG_SHOW_FREEZE(pSession, &freezeTime, freezeInterval, TMWDIAG_ID_RX);
      }

      continue;
    }

    /* Validate qualifier */
    /* These qualifiers are not specified for level 4 and below and are not supported.
     * by the freeze functions
     */
    if ((header.qualifier == DNPDEFS_QUAL_8BIT_INDEX)
      || (header.qualifier == DNPDEFS_QUAL_16BIT_INDEX)
      || (header.qualifier == DNPDEFS_QUAL_16BIT_INDEX_8BITQ))
    {
      DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_QUALIFIER);
      pSDNPSession->iin |= DNPDEFS_IIN_OUT_OF_RANGE;
      break;
    }

    index = 0;
    processed = TMWDEFS_FALSE;
    while(_sdnpObjFreezeFuncTable[index].pFreezeFunc != TMWDEFS_NULL)
    {
      if((_sdnpObjFreezeFuncTable[index].group == header.group)
        && (_sdnpObjFreezeFuncTable[index].variation == header.variation))
      {
        if(!_sdnpObjFreezeFuncTable[index].pFreezeFunc(pSession, pRxMessage->fc, &header, timeDateEnum, &freezeTime, freezeInterval))
        {
          SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_FREEZE);
          pSDNPSession->iin |= DNPDEFS_IIN_OUT_OF_RANGE;
        }

        processed = TMWDEFS_TRUE;
        break;
      }

      index += 1;
    }

    if(!processed)
    {
      /* This object group/variation is not supported for this function code */
      SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_FREEZE_OBJVAR);
      pSDNPSession->iin |= DNPDEFS_IIN_OBJECT_UNKNOWN;
      break;
    }
  }
#else
  /* According to TB2002-001 Counter Objects
   * A device that does not support Frozen Counters should respond after setting the 
   * “Function Code Not Implemented” indication. Devices that support Frozen Counters 
   * must support both Immediate Freeze and Freeze and Clear commands. 
   */
  SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_FREEZE);
  pSDNPSession->iin |= DNPDEFS_IIN_BAD_FUNCTION;
#endif

  /* If reply required, send it */
  noResp = (TMWTYPES_BOOL)(
    ((pRxMessage->fc == DNPDEFS_FC_FRZ_NOACK) || (pRxMessage->fc == DNPDEFS_FC_FRZ_CLEAR_NOACK)
    || (pRxMessage->fc == DNPDEFS_FC_FRZ_TIME_NOACK)) ? TMWDEFS_TRUE : TMWDEFS_FALSE);

  pResponse = dnpchnl_newTxData((TMWCHNL *)pDNPChannel,
    pSession, pDNPChannel->txFragmentSize, pRxMessage->rxAddress);

  if(pResponse == TMWDEFS_NULL)
  {
    return;
  }

  _initializeResponse(pSession, pResponse, TMWDEFS_TRUE, TMWDEFS_TRUE);
#if TMWCNFG_SUPPORT_DIAG
  pResponse->pMsgDescription = "Freeze Response";
#endif

  /* Done with this message */
  pSDNPSession->pendingRequest = TMWDEFS_FALSE;

  /* Send response */
  _sendResponse(pSession, noResp, pRxMessage, pResponse);
}
#endif

/* function: _processRestartRequest
 * purpose: Process restart requests
 * arguments:
 *  pSession - session request was received on
 *  pRxFragment - received request data
 * returns:
 *  void
 */
static void TMWDEFS_LOCAL _processRestartRequest(
  TMWSESN *pSession)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  DNPUTIL_RX_MSG *pRxMessage = &pSDNPSession->lastRcvdRequest;
  DNPCHNL *pDNPChannel = (DNPCHNL *)pSession->pChannel;
  TMWSESN_TX_DATA *pResponse;

  /* Process sequence number */
  pSDNPSession->recvSequenceNumber = (TMWTYPES_UCHAR)(pRxMessage->ac & DNPDEFS_AC_SEQUENCE_MASK);
  pSDNPSession->respSequenceNumber = pSDNPSession->recvSequenceNumber;

  /* Initialize response */
  pResponse = dnpchnl_newTxData((TMWCHNL *)pDNPChannel,
    pSession, pDNPChannel->txFragmentSize, pRxMessage->rxAddress);

  if(pResponse == TMWDEFS_NULL)
  {
    return;
  }

  _initializeResponse(pSession, pResponse, TMWDEFS_TRUE, TMWDEFS_TRUE);
#if TMWCNFG_SUPPORT_DIAG
  pResponse->pMsgDescription = "Restart Response";
#endif

  /* Store object header (group, variation, qualifier, and range) */
  pResponse->pMsgBuf[pResponse->msgLength++] = DNPDEFS_OBJ_52_TIME_DELAY;
  pResponse->pMsgBuf[pResponse->msgLength++] = 2;
  pResponse->pMsgBuf[pResponse->msgLength++] = DNPDEFS_QUAL_8BIT_LIMITED_QTY;
  pResponse->pMsgBuf[pResponse->msgLength++] = 1;

  if(pRxMessage->fc == DNPDEFS_FC_COLD_RESTART)
  {
    tmwtarg_store16(&pSDNPSession->coldRestartDelay, pResponse->pMsgBuf + pResponse->msgLength);
    pResponse->msgLength += 2;

    pResponse->txFlags |= TMWSESN_TXFLAGS_COLD_RESTART;
  }
  else
  {
    tmwtarg_store16(&pSDNPSession->warmRestartDelay, pResponse->pMsgBuf + pResponse->msgLength);
    pResponse->msgLength += 2;

    pResponse->txFlags |= TMWSESN_TXFLAGS_WARM_RESTART;
  }

  /* No response expected */
  pResponse->txFlags |= TMWSESN_TXFLAGS_NO_RESPONSE;

  /* Done with this message */
  pSDNPSession->pendingRequest = TMWDEFS_FALSE;

  /* Send response */
  _sendResponse(pSession, TMWDEFS_FALSE, pRxMessage, pResponse);
}

/* function: _processEnableUnsolRequest
 * purpose: Process enable unsolicited requests
 * arguments:
 *  pSession - session request was received on
 *  pRxFragment - received request data
 * returns:
 *  void
 */
static void TMWDEFS_LOCAL _processEnableUnsolRequest(
  TMWSESN *pSession)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  DNPUTIL_RX_MSG *pRxMessage = &pSDNPSession->lastRcvdRequest;
  DNPCHNL *pDNPChannel = (DNPCHNL *)pSession->pChannel;
  TMWSESN_TX_DATA *pResponse;

  pSDNPSession->recvSequenceNumber = (TMWTYPES_UCHAR)(pRxMessage->ac & DNPDEFS_AC_SEQUENCE_MASK);
  pSDNPSession->respSequenceNumber = pSDNPSession->recvSequenceNumber;

  pResponse = dnpchnl_newTxData((TMWCHNL *)pDNPChannel,
    pSession, pDNPChannel->txFragmentSize, pRxMessage->rxAddress);

  if(pResponse == TMWDEFS_NULL)
  {
    return;
  }

  _initializeResponse(pSession, pResponse, TMWDEFS_TRUE, TMWDEFS_TRUE);
#if TMWCNFG_SUPPORT_DIAG
  pResponse->pMsgDescription = "Enable Unsolicited Response";
#endif

  if(pSDNPSession->unsolAllowed)
  {
    DNPUTIL_OBJECT_HEADER header;

    while(pRxMessage->offset < pRxMessage->msgLength)
    {
      if(!dnputil_parseObjectHeader(pRxMessage, 0, &header))
      {
        DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_PARSE_HEADER);
        pSDNPSession->iin |= DNPDEFS_IIN_OUT_OF_RANGE;
        break;
      }

      if(header.group != DNPDEFS_OBJ_60_CLASS_SCANS)
      {
        SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_ENABLE_UNSOL);
        pSDNPSession->iin |= DNPDEFS_IIN_OBJECT_UNKNOWN;
        break;
      }

      /* Diagnostics */
      DNPDIAG_SHOW_OBJECT_HEADER(pSession, &header);

      /* Enable unsolicited responses for requested class */
      switch(header.variation)
      {
      case 2:
        pSDNPSession->unsolEventMask |= TMWDEFS_CLASS_MASK_ONE;
        break;

      case 3:
        pSDNPSession->unsolEventMask |= TMWDEFS_CLASS_MASK_TWO;
        break;

      case 4:
        pSDNPSession->unsolEventMask |= TMWDEFS_CLASS_MASK_THREE;
        break;

      default:
        SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_ENABLE_UNSOL_VAR);
        pSDNPSession->iin |= DNPDEFS_IIN_OUT_OF_RANGE;
        break;
      }
    }

    sdnpdata_unsolEventMask(pSDNPSession->pDbHandle, pSDNPSession->unsolEventMask);
  }
  else
  {
    SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_ENABLE_UNSOL_NA);
    pSDNPSession->iin |= DNPDEFS_IIN_BAD_FUNCTION;
  }

  /* Done with this message */
  pSDNPSession->pendingRequest = TMWDEFS_FALSE;

  /* Send response to unsolicited enable request */
  _sendResponse(pSession, TMWDEFS_FALSE, pRxMessage, pResponse);

  /* Update status of unsolicited events */
  sdnpunsl_updateStatus(pSession);

  /* If unsolicited events are ready send them */
  sdnpunsl_processUnsolEvents(pSession, TMWDEFS_CLASS_MASK_ALL);
}

/* function: _processDisableUnsolRequest
 * purpose: Process disable unsolicited requests
 * arguments:
 *  pSession - session request was received on
 *  pRxFragment - received request data
 * returns:
 *  void
 */
static void TMWDEFS_LOCAL _processDisableUnsolRequest(
  TMWSESN *pSession)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  DNPUTIL_RX_MSG *pRxMessage = &pSDNPSession->lastRcvdRequest;
  DNPCHNL *pDNPChannel = (DNPCHNL *)pSession->pChannel;
  TMWSESN_TX_DATA *pResponse;

  pSDNPSession->recvSequenceNumber = (TMWTYPES_UCHAR)(pRxMessage->ac & DNPDEFS_AC_SEQUENCE_MASK);
  pSDNPSession->respSequenceNumber = pSDNPSession->recvSequenceNumber;

  pResponse = dnpchnl_newTxData((TMWCHNL *)pDNPChannel,
    pSession, pDNPChannel->txFragmentSize, pRxMessage->rxAddress);

  if(pResponse == TMWDEFS_NULL)
  {
    return;
  }

  _initializeResponse(pSession, pResponse, TMWDEFS_TRUE, TMWDEFS_TRUE);
#if TMWCNFG_SUPPORT_DIAG
  pResponse->pMsgDescription = "Disable Unsolicited Response";
#endif

  if(pSDNPSession->unsolAllowed)
  {
    DNPUTIL_OBJECT_HEADER header;

    while(pRxMessage->offset < pRxMessage->msgLength)
    {
      if(!dnputil_parseObjectHeader(pRxMessage, 0, &header))
      {
        DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_PARSE_HEADER);
        pSDNPSession->iin |= DNPDEFS_IIN_OUT_OF_RANGE;
        break;
      }

      if(header.group != DNPDEFS_OBJ_60_CLASS_SCANS)
      {
        SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_ENABLE_UNSOL);
        pSDNPSession->iin |= DNPDEFS_IIN_OBJECT_UNKNOWN;
        break;
      }

      /* Diagnostics */
      DNPDIAG_SHOW_OBJECT_HEADER(pSession, &header);

      /* Disable unsolicited responses for requested class */
      switch(header.variation)
      {
      case 2:
        pSDNPSession->unsolEventMask &= ~TMWDEFS_CLASS_MASK_ONE;
        break;

      case 3:
        pSDNPSession->unsolEventMask &= ~TMWDEFS_CLASS_MASK_TWO;
        break;

      case 4:
        pSDNPSession->unsolEventMask &= ~TMWDEFS_CLASS_MASK_THREE;
        break;

      default:
        SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_ENABLE_UNSOL_VAR);
        pSDNPSession->iin |= DNPDEFS_IIN_OUT_OF_RANGE;
        break;
      }
    }

    sdnpdata_unsolEventMask(pSDNPSession->pDbHandle, pSDNPSession->unsolEventMask);
  }
  else
  {
    SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_ENABLE_UNSOL_NA);
    pSDNPSession->iin |= DNPDEFS_IIN_BAD_FUNCTION;
  }

  /* Done with this message */
  pSDNPSession->pendingRequest = TMWDEFS_FALSE;

  /* Send response */
  _sendResponse(pSession, TMWDEFS_FALSE, pRxMessage, pResponse);
}

#if SDNPDATA_SUPPORT_ASSIGN
/* function: _processAssignClassRequest
 * purpose: Process assign class requests
 * arguments:
 *  pSession - session request was received on
 *  pRxFragment - received request data
 * returns:
 *  void
 */
static void TMWDEFS_LOCAL _processAssignClassRequest(
  TMWSESN *pSession)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  DNPUTIL_RX_MSG *pRxMessage = &pSDNPSession->lastRcvdRequest;
  DNPCHNL *pDNPChannel = (DNPCHNL *)pSession->pChannel;
  TMWSESN_TX_DATA *pResponse;
  DNPUTIL_OBJECT_HEADER header;
  TMWDEFS_CLASS_MASK classMask;
  TMWTYPES_BOOL processed;
  int index;

  /* Process sequence number */
  pSDNPSession->recvSequenceNumber = (TMWTYPES_UCHAR)(pRxMessage->ac & DNPDEFS_AC_SEQUENCE_MASK);
  pSDNPSession->respSequenceNumber = pSDNPSession->recvSequenceNumber;

  /* Initialize response */
  pResponse = dnpchnl_newTxData((TMWCHNL *)pDNPChannel,
    pSession, pDNPChannel->txFragmentSize, pRxMessage->rxAddress);

  if(pResponse == TMWDEFS_NULL)
  {
    return;
  }

  _initializeResponse(pSession, pResponse, TMWDEFS_TRUE, TMWDEFS_TRUE);
#if TMWCNFG_SUPPORT_DIAG
  pResponse->pMsgDescription = "Assign Class Response";
#endif

  /* Accumulate class mask */
  classMask = TMWDEFS_CLASS_MASK_NOTCLASS0;
  while(pRxMessage->offset < pRxMessage->msgLength)
  {
    /* Parse object header */
    if(!dnputil_parseObjectHeader(pRxMessage, 0, &header))
    {
      DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_PARSE_HEADER);
      pSDNPSession->iin |= DNPDEFS_IIN_OUT_OF_RANGE;
      break;
    }

    /* Diagnostics */
    DNPDIAG_SHOW_OBJECT_HEADER(pSession, &header);

    if(header.group == DNPDEFS_OBJ_60_CLASS_SCANS)
    {
      switch(header.variation)
      {
      case 1:
        classMask = TMWDEFS_CLASS_MASK_NONE;
        break;

      case 2:
        classMask = TMWDEFS_CLASS_MASK_ONE;
        break;

      case 3:
        classMask = TMWDEFS_CLASS_MASK_TWO;
        break;

      case 4:
        classMask = TMWDEFS_CLASS_MASK_THREE;
        break;

      default:
        SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_ASSIGN_OBJVAR);
        pSDNPSession->iin |= DNPDEFS_IIN_OUT_OF_RANGE;
        break;
      }
    }
    else
    {
      index = 0;
      processed = TMWDEFS_FALSE;
      while(_sdnpObjAssignFuncTable[index].pAssignClassFunc != TMWDEFS_NULL)
      {
        if((_sdnpObjAssignFuncTable[index].group == header.group)
          && (_sdnpObjAssignFuncTable[index].variation == header.variation))
        {
          processed = TMWDEFS_TRUE;

          /*   validate message size based on header info */
          if(!_validateMessageSize(pSession, 0, &header, (TMWTYPES_USHORT)(pRxMessage->msgLength-pRxMessage->offset)))
          { 
            pRxMessage->offset = pRxMessage->msgLength;
            break;
          }
          if(!_sdnpObjAssignFuncTable[index].pAssignClassFunc(
              pSession, pRxMessage, &header, classMask))
          {
            SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_ASSIGN_CLASS);
            pSDNPSession->iin |= DNPDEFS_IIN_OUT_OF_RANGE;
          }
          break;
        }

        index += 1;
      }

      if(!processed)
      {
        SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_ASSIGN_OBJVAR);
        pSDNPSession->iin |= DNPDEFS_IIN_OBJECT_UNKNOWN;
        break;
      }
    }
  }

  /* Done with this message */
  pSDNPSession->pendingRequest = TMWDEFS_FALSE;

  /* Send response */
  _sendResponse(pSession, TMWDEFS_FALSE, pRxMessage, pResponse);
}
#endif

/* function: _processRecordTimeRequest
 * purpose: Process record current time requests
 * arguments:
 *  pSession - session request was received on
 *  pRxFragment - received request data
 * returns:
 *  void
 */
static void TMWDEFS_LOCAL _processRecordTimeRequest(
  TMWSESN *pSession)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  DNPUTIL_RX_MSG *pRxMessage = &pSDNPSession->lastRcvdRequest;
  DNPCHNL *pDNPChannel = (DNPCHNL *)pSession->pChannel;
  TMWSESN_TX_DATA *pResponse;

  /* Validate request */
  if(pRxMessage->offset != pRxMessage->msgLength)
  {
    SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_TIME_NO_OBJECT);
    return;
  }

  /* Process sequence number */
  pSDNPSession->recvSequenceNumber = (TMWTYPES_UCHAR)(pRxMessage->ac & DNPDEFS_AC_SEQUENCE_MASK);
  pSDNPSession->respSequenceNumber = pSDNPSession->recvSequenceNumber;

  /* Initialize response */
  pResponse = dnpchnl_newTxData((TMWCHNL *)pDNPChannel,
    pSession, pDNPChannel->txFragmentSize, pRxMessage->rxAddress);

  if(pResponse == TMWDEFS_NULL)
  {
    return;
  }

  _initializeResponse(pSession, pResponse, TMWDEFS_TRUE, TMWDEFS_TRUE);
#if TMWCNFG_SUPPORT_DIAG
  pResponse->pMsgDescription = "Record Current Time Response";
#endif

  /* Store time for this session */
  pSDNPSession->recordCurrentTime = pRxMessage->lastByteTime;
  pSDNPSession->recordedCurrentTime = TMWDEFS_TRUE;  

  /* Done with this message */
  pSDNPSession->pendingRequest = TMWDEFS_FALSE;

  /* Send response */
  _sendResponse(pSession, TMWDEFS_FALSE, pRxMessage, pResponse);
}

#if SDNPDATA_SUPPORT_OBJ50_V1
/* function: _processDelayRequest
 * purpose: Process assign class requests
 * arguments:
 *  pSession - session request was received on
 *  pRxFragment - received request data
 * returns:
 *  void
 */
static void TMWDEFS_LOCAL _processDelayRequest(
  TMWSESN *pSession)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  DNPUTIL_RX_MSG *pRxMessage = &pSDNPSession->lastRcvdRequest;
  DNPCHNL *pDNPChannel = (DNPCHNL *)pSession->pChannel;
  TMWSESN_TX_DATA *pResponse;

  /* Process sequence number */
  pSDNPSession->recvSequenceNumber = (TMWTYPES_UCHAR)(pRxMessage->ac & DNPDEFS_AC_SEQUENCE_MASK);
  pSDNPSession->respSequenceNumber = pSDNPSession->recvSequenceNumber;

  /* Initialize response */
  pResponse = dnpchnl_newTxData((TMWCHNL *)pDNPChannel,
    pSession, pDNPChannel->txFragmentSize, pRxMessage->rxAddress);

  if(pResponse == TMWDEFS_NULL)
  {
    return;
  }

  _initializeResponse(pSession, pResponse, TMWDEFS_TRUE, TMWDEFS_TRUE);
#if TMWCNFG_SUPPORT_DIAG
  pResponse->pMsgDescription = "Delay Measurement Response";
#endif

  /* Store object header (group, variation, qualifier, and range) */
  pResponse->pMsgBuf[pResponse->msgLength++] = DNPDEFS_OBJ_52_TIME_DELAY;
  pResponse->pMsgBuf[pResponse->msgLength++] = 2;
  pResponse->pMsgBuf[pResponse->msgLength++] = DNPDEFS_QUAL_8BIT_LIMITED_QTY;
  pResponse->pMsgBuf[pResponse->msgLength++] = 1;

  /* Leave room for object data (2 byte delay) */
  pResponse->pMsgBuf[pResponse->msgLength++] = 0x12;
  pResponse->pMsgBuf[pResponse->msgLength++] = 0x34;

  /* Set flag to tell before transmit callback to store delay */
  pSDNPSession->delayMeasurementRxTime = pRxMessage->firstByteTime;
  pResponse->txFlags |= TMWSESN_TXFLAGS_STORE_DNP_DELAY;

  /* Done with this message */
  pSDNPSession->pendingRequest = TMWDEFS_FALSE;

  /* Send response */
  _sendResponse(pSession, TMWDEFS_FALSE, pRxMessage, pResponse);
}
#endif

#if SDNPDATA_SUPPORT_OBJ70
/* function: _processFileRequest
 * purpose: Process file transfer requests
 * arguments:
 *  pSession - session request was received on
 *  pRxFragment - received request data
 * returns:
 *  void
 */
static void TMWDEFS_LOCAL _processFileRequest(
  TMWSESN *pSession)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  DNPUTIL_RX_MSG *pRxMessage = &pSDNPSession->lastRcvdRequest;
  DNPCHNL *pDNPChannel = (DNPCHNL *)pSession->pChannel;
  DNPUTIL_OBJECT_HEADER header;
  TMWSESN_TX_DATA *pResponse;

  pResponse = dnpchnl_newTxData((TMWCHNL *)pDNPChannel,
    pSession, pDNPChannel->txFragmentSize, pRxMessage->rxAddress);

  if(pResponse == TMWDEFS_NULL)
  {
    return;
  }

  pSDNPSession->recvSequenceNumber = (TMWTYPES_UCHAR)(pRxMessage->ac & DNPDEFS_AC_SEQUENCE_MASK);
  pSDNPSession->respSequenceNumber = pSDNPSession->recvSequenceNumber;
  _initializeResponse(pSession, pResponse, TMWDEFS_TRUE, TMWDEFS_TRUE);
#if TMWCNFG_SUPPORT_DIAG
  pResponse->pMsgDescription = "File Response";
#endif

  while(pRxMessage->offset < pRxMessage->msgLength)
  {
    if(!dnputil_parseObjectHeader(pRxMessage, 0, &header))
    {
      DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_PARSE_HEADER);
      pSDNPSession->iin |= DNPDEFS_IIN_OUT_OF_RANGE;
      break;
    }

    /* File transfer only uses 16 bit free format qualifier */
    if(header.qualifier != DNPDEFS_QUAL_16BIT_FREE_FORMAT)
    {
      SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_INV_QUAL_FILE);
      pSDNPSession->iin |= DNPDEFS_IIN_OUT_OF_RANGE;
      break;
    }

    /* Diagnostics */
    DNPDIAG_SHOW_OBJECT_HEADER(pSession, &header);

    /* Process request */
    if(!sdnpo070_processRequest(pSession, pRxMessage, pResponse, &header))
    {
      pSDNPSession->iin |= DNPDEFS_IIN_OBJECT_UNKNOWN;
      break;
    }
  }

  /* See if the response contains events */
  if(pResponse->txFlags & TMWSESN_TXFLAGS_CONTAINS_EVENTS)
  {
    /* Yep, need to request an application confirmation */
    sdnpsesn_requestApplConfirm(pSession, pResponse);

    /* Update IIN bits to reflect new state of queued events */
    sdnprbe_updateIINBits(pSession);
  }

  /* Done with this message */
  pSDNPSession->pendingRequest = TMWDEFS_FALSE;

  /* Send response */
  _sendResponse(pSession, TMWDEFS_FALSE, pRxMessage, pResponse);
}
#endif

#if SDNPDATA_SUPPORT_OBJ91
/* function: _processActConfigRequest
 * purpose: Process Activate Configuration requests
 * arguments:
 *  pSession - session request was received on 
 * returns:
 *  void
 */
static void TMWDEFS_LOCAL _processActConfigRequest(
  TMWSESN *pSession)
{
  TMWTYPES_UCHAR  numberRequests;
  TMWTYPES_UCHAR  status;
  TMWTYPES_UCHAR  errorLength;
  TMWTYPES_BOOL   lastString;
  TMWTYPES_USHORT lengthOrDelayIndex;
  TMWTYPES_USHORT length;
  TMWTYPES_ULONG  delay;
  DNPUTIL_OBJECT_HEADER header;
  TMWSESN_TX_DATA *pResponse;
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  DNPUTIL_RX_MSG *pRxMessage = &pSDNPSession->lastRcvdRequest;
  DNPCHNL *pDNPChannel = (DNPCHNL *)pSession->pChannel;

  /* Process sequence number */
  pSDNPSession->recvSequenceNumber = (TMWTYPES_UCHAR)(pRxMessage->ac & DNPDEFS_AC_SEQUENCE_MASK);
  pSDNPSession->respSequenceNumber = pSDNPSession->recvSequenceNumber;
  
  /* Initialize response */
  pResponse = dnpchnl_newTxData((TMWCHNL *)pDNPChannel,
    pSession, pDNPChannel->txFragmentSize, pRxMessage->rxAddress);

  if(pResponse == TMWDEFS_NULL)
  {
    return;
  }

  _initializeResponse(pSession, pResponse, TMWDEFS_TRUE, TMWDEFS_TRUE);
#if TMWCNFG_SUPPORT_DIAG
  pResponse->pMsgDescription = "Activate Config Response";
#endif
  
  pSDNPSession->pendingRequest = TMWDEFS_FALSE;

  /* Store object header (group, variation, qualifier, and range) */
  pResponse->pMsgBuf[pResponse->msgLength++] = DNPDEFS_OBJ_91_ACTIVATE_STATUS;
  pResponse->pMsgBuf[pResponse->msgLength++] = 1;

  /* Previous versions sent this response using qualifier 7. 
   * The parsing codes specification says to use qualifier 5b. 
   * However since some masters will not be able to parse 5b till they are upgraded,
   * allow this outstation to send qualifier 7 for compatibility purposes 
   */
  if(!pSDNPSession->sendObj91RespQual7)
  {
    pResponse->pMsgBuf[pResponse->msgLength++] = DNPDEFS_QUAL_16BIT_FREE_FORMAT;
    pResponse->pMsgBuf[pResponse->msgLength++] = 1;

    /* 
     * initialize length to zero
     * save index so we can store length, delay and quantity after they are retrieved from database 
     */
    length = 0;
    lengthOrDelayIndex = pResponse->msgLength;
    tmwtarg_store16(&length, &pResponse->pMsgBuf[pResponse->msgLength]);
    pResponse->msgLength += 2;
  }
  else
  {
    pResponse->pMsgBuf[pResponse->msgLength++] = DNPDEFS_QUAL_8BIT_LIMITED_QTY;
    pResponse->pMsgBuf[pResponse->msgLength++] = 1;

    /* save index so we can store delay and quantity after they are retrieved from database 
     */
    lengthOrDelayIndex = pResponse->msgLength;
  }
  
  /* set time delay to zero in case there is an error below */
  delay = 0;
  tmwtarg_store32(&delay, &pResponse->pMsgBuf[pResponse->msgLength]);
  pResponse->msgLength += 4;

  /* set number to zero */
  pResponse->pMsgBuf[pResponse->msgLength++] = 0; 
 
  numberRequests = 0;
  errorLength = 0;
  status = 0;
  lastString = TMWDEFS_FALSE;
  while(pRxMessage->offset < pRxMessage->msgLength)
  {
    /* Parse object header */
    if(!dnputil_parseObjectHeader(pRxMessage, 0, &header))
    {
      DNPDIAG_ERROR(pSession->pChannel, pSession, DNPDIAG_PARSE_HEADER);
      pSDNPSession->iin |= DNPDEFS_IIN_OUT_OF_RANGE;
      break;
    }

    /* Diagnostics */
    DNPDIAG_SHOW_OBJECT_HEADER(pSession, &header);

    if(header.group == DNPDEFS_OBJ_110_STRING_DATA)
    {
      TMWTYPES_USHORT point;
      TMWTYPES_USHORT i;
      for(i = 0; i < header.numberOfPoints; i++)
      {
        /* Get next point number from request */
        dnputil_getPointNumber(pRxMessage, &header, i, &point); 

        /* variation zero would be zero length string. Also 
         * make sure the message is long enough to contain the string 
         */
        if((header.variation == 0) 
          ||((pRxMessage->offset+header.variation) > pRxMessage->msgLength))
        {
          SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_PARSE_ERROR);
          pSDNPSession->iin |= DNPDEFS_IIN_OUT_OF_RANGE; 
          if(!pSDNPSession->sendObj91RespQual7)
          {
            /* Put length in response */
            length = pResponse->msgLength - lengthOrDelayIndex -2;
            tmwtarg_store16(&length, pResponse->pMsgBuf + lengthOrDelayIndex);
          }
          _sendResponse(pSession, TMWDEFS_FALSE, pRxMessage, pResponse);
          return;
        }

        /* Diagnostics */
        DNPDIAG_SHOW_STRING_DATA(pSession, point, (pRxMessage->pMsgBuf + pRxMessage->offset), 
        header.variation, TMWDIAG_ID_RX);
      
        /* Determine if there are more strings or filenames */
        if((pRxMessage->offset+header.variation) >= pRxMessage->msgLength)
          lastString = TMWDEFS_TRUE;

        /* determine how may bytes are left for optional error message */
        if((pResponse->maxLength - pResponse->msgLength) > 255)
          errorLength = 255;
        else
          errorLength = (TMWTYPES_UCHAR)(pResponse->maxLength - pResponse->msgLength); 

        status = sdnpdata_activateConfig(pSDNPSession->pDbHandle, point, lastString, 
          (pRxMessage->pMsgBuf + pRxMessage->offset), header.variation, &delay,
          &pResponse->pMsgBuf[pResponse->msgLength+2], &errorLength);

        if(status == 0)
          errorLength = 0;

        pRxMessage->offset =(TMWTYPES_USHORT)(pRxMessage->offset + header.variation);

        pResponse->pMsgBuf[pResponse->msgLength++] = errorLength +1;
        pResponse->pMsgBuf[pResponse->msgLength++] = status;  
        /* Optional error message was copied in above */
        pResponse->msgLength = pResponse->msgLength + errorLength;

        numberRequests++;
      }
    }
    else if((header.group == DNPDEFS_OBJ_70_FILE_IDENTIFIER)
      &&(header.variation == 8))
    { 
      TMWTYPES_USHORT i;
      for(i = 0; i < header.numberOfPoints; i++)
      {
        TMWTYPES_USHORT stringLength;

        /* Number of bytes in object */
        tmwtarg_get16(pRxMessage->pMsgBuf + pRxMessage->offset, &stringLength);
        pRxMessage->offset += 2;
      
        /* make sure the message is long enough to contain the string */
        if((stringLength == 0) 
          ||((pRxMessage->offset+stringLength) > pRxMessage->msgLength))
        {
          SDNPDIAG_ERROR(pSession->pChannel, pSession, SDNPDIAG_PARSE_ERROR);
          pSDNPSession->iin |= DNPDEFS_IIN_OUT_OF_RANGE;
          if(!pSDNPSession->sendObj91RespQual7)
          {
            /* Put length in response */
            length = pResponse->msgLength - lengthOrDelayIndex -2;
            tmwtarg_store16(&length, pResponse->pMsgBuf + lengthOrDelayIndex);
          } 
          _sendResponse(pSession, TMWDEFS_FALSE, pRxMessage, pResponse);
          return;
        }

        /* Diagnostics */
        DNPDIAG_SHOW_FILE_STRING(pSession, 0, (pRxMessage->pMsgBuf + pRxMessage->offset), 
          stringLength, TMWDIAG_ID_RX);
      
        /* Determine if there are more strings or filenames */
        if((pRxMessage->offset+stringLength) == pRxMessage->msgLength)
          lastString = TMWDEFS_TRUE;

        /* determine how may bytes are left for optional error message */
        if((pResponse->maxLength - pResponse->msgLength) > 255)
          errorLength = 255;
        else
          errorLength = (TMWTYPES_UCHAR)(pResponse->maxLength - pResponse->msgLength); 

        status = sdnpdata_activateConfig(pSDNPSession->pDbHandle, 0, lastString, 
          (pRxMessage->pMsgBuf + pRxMessage->offset), stringLength, &delay,
          &pResponse->pMsgBuf[pResponse->msgLength+2], &errorLength);

        pRxMessage->offset =(TMWTYPES_USHORT)(pRxMessage->offset + stringLength);
        if(status == 0)
          errorLength = 0;

        pResponse->pMsgBuf[pResponse->msgLength++] = errorLength +1;
        pResponse->pMsgBuf[pResponse->msgLength++] = status;  
        /* Optional error message was copied in above */
        pResponse->msgLength = pResponse->msgLength + errorLength;

        numberRequests++;
      } 
    }
    else /* not expected goup */
    {
      pSDNPSession->iin |= DNPDEFS_IIN_OUT_OF_RANGE;
    } 
  }

  if(!pSDNPSession->sendObj91RespQual7)
  {
    length = pResponse->msgLength - lengthOrDelayIndex -2;

    /* Put length in response */
    tmwtarg_store16(&length, pResponse->pMsgBuf + lengthOrDelayIndex);
    lengthOrDelayIndex += 2;
  } 

  /* Put delay in response */
  tmwtarg_store32(&delay, pResponse->pMsgBuf + lengthOrDelayIndex);
  lengthOrDelayIndex += 4;

  /* Put quantity in response */
  pResponse->pMsgBuf[lengthOrDelayIndex] = numberRequests;
 
  /* No response expected */
  pResponse->txFlags |= TMWSESN_TXFLAGS_NO_RESPONSE;

  /* Send response */
  _sendResponse(pSession, TMWDEFS_FALSE, pRxMessage, pResponse);
}
#endif

/* function: _idleCallback */
static TMWTYPES_BOOL TMWDEFS_CALLBACK _idleCallback(
  TMWSESN *pSession)
{
   if(((SDNPSESN *)pSession)->unsolWaitingForConfirm)
     return(TMWDEFS_FALSE);
  
   return(TMWDEFS_TRUE);
}

/* Global Functions */

/* function: sdnpsesn_initConfig */
void TMWDEFS_GLOBAL sdnpsesn_initConfig(
  SDNPSESN_CONFIG *pConfig)
{
  int i;
  
  pConfig->source = 4;
  pConfig->destination = 3;

  pConfig->active = TMWDEFS_TRUE;

  pConfig->linkStatusPeriod = 0;
  pConfig->linkStatusTimeoutDisconnect = TMWDEFS_TRUE;
  pConfig->validateSourceAddress = TMWDEFS_FALSE;
  pConfig->enableSelfAddress = TMWDEFS_FALSE;
  pConfig->multiFragRespAllowed = TMWDEFS_TRUE;
  pConfig->multiFragConfirm = TMWDEFS_TRUE;
  pConfig->fileTransferEnabled = TMWDEFS_TRUE;
  pConfig->sendObj91RespQual7 = TMWDEFS_FALSE;

  pConfig->applConfirmTimeout = TMWDEFS_SECONDS(10);
  pConfig->unsolConfirmTimeout = TMWDEFS_SECONDS(10);
  pConfig->selectTimeout = TMWDEFS_SECONDS(5);

  pConfig->respondNeedTime = TMWDEFS_TRUE;
  pConfig->clockValidPeriod = TMWDEFS_MINUTES(30);

  pConfig->warmRestartDelay = (TMWTYPES_USHORT)TMWDEFS_SECONDS(2);
  pConfig->coldRestartDelay = (TMWTYPES_USHORT)TMWDEFS_SECONDS(5);

  pConfig->allowMultiCROBRequests = TMWDEFS_TRUE;
  pConfig->maxControlRequests = SDNPCNFG_MAX_CONTROL_REQUESTS;
  pConfig->enabledBroadcastWrites = 0xf;
  pConfig->enabledBroadcastFCs = 0xffffffff;
  pConfig->enabledFCs = 0xffffffff;
  pConfig->enabledWrites = 0xf;

  pConfig->unsolAllowed = TMWDEFS_TRUE;
  pConfig->unsolDontSendInitialNull = TMWDEFS_FALSE;
  pConfig->sendUnsolWhenOnline = TMWDEFS_FALSE;
  pConfig->unsolSendIdenticalRetry = TMWDEFS_FALSE;
  pConfig->unsolClassMask = TMWDEFS_CLASS_MASK_NONE;
  pConfig->unsolSendByClass = TMWDEFS_FALSE;
  pConfig->unsolMaxRetries = 3;
  pConfig->unsolRetryDelay = TMWDEFS_SECONDS(5);
  pConfig->unsolOfflineRetryDelay = TMWDEFS_SECONDS(30); 
  pConfig->unsolOfflineDelayOnce = TMWDEFS_FALSE;

  pConfig->unsolClass1MaxDelay = TMWDEFS_SECONDS(5);
  pConfig->unsolClass2MaxDelay = TMWDEFS_SECONDS(5);
  pConfig->unsolClass3MaxDelay = TMWDEFS_SECONDS(5);

  pConfig->unsolClass1MaxEvents = 5;
  pConfig->unsolClass2MaxEvents = 5;
  pConfig->unsolClass3MaxEvents = 5;
  pConfig->userManagedEvents = TMWDEFS_FALSE;
  pConfig->sortEventsByTime = TMWDEFS_FALSE;
  pConfig->supportEventsWithTime = TMWDEFS_TRUE;
  pConfig->sesnDiagMask = TMWDIAG_ID_DEF_MASK;

#if SDNPCNFG_DISABLE_SELOP_SEQ_CHECK
  pConfig->disableSelOpSeqCheck = TMWDEFS_FALSE;
#endif

  /* Default variations */
  pConfig->obj01DefaultVariation  = 0;  /* Binary Input w/o status */
  pConfig->obj02DefaultVariation  = 0;  /* Binary Input Change with Rel Time */
  pConfig->obj03DefaultVariation  = 1;  /* Double Bit Input w/o status */
  pConfig->obj04DefaultVariation  = 3;  /* Double Bit Input Change with Rel Time */
  pConfig->obj10DefaultVariation  = 2;  /* Binary Output status */
  pConfig->obj11DefaultVariation  = 1;  /* Binary Output Events w/o time*/
  pConfig->obj13DefaultVariation  = 1;  /* Binary Output Command Events w/o time */
  pConfig->obj20DefaultVariation  = 0;  /* 32 Bit Counter w/o flag */
  pConfig->obj21DefaultVariation  = 0;  /* 32 Bit Frozen counter w/o flag */
  pConfig->obj22DefaultVariation  = 0;  /* 32 Bit Counter Change Event w/o time */
  pConfig->obj23DefaultVariation  = 0;  /* 32 Bit Frozen Counter Change Event w/o time */ 
  pConfig->obj30DefaultVariation  = 0;  /* 32 Bit Analog Input w/o flag */
  pConfig->obj31DefaultVariation  = 0;  /* 32 Bit Frozen Analog Input w/o flag */
  pConfig->obj32DefaultVariation  = 0;  /* 32 Bit Analog Input Change Event w/o time */
  pConfig->obj33DefaultVariation  = 0;  /* 32 Bit Frozen Analog Input Change Event w/o time */
  pConfig->obj34DefaultVariation  = 0;  /* 32 Bit Analog Deadband */
  pConfig->obj40DefaultVariation  = 0;  /* 16 Bit Analog Output Status, (32 Bit var 1 not in subset level 2 */
  pConfig->obj42DefaultVariation  = 0;  /* 16 Bit Analog Output Event w/o time  */
  pConfig->obj43DefaultVariation  = 0;  /* 16 Bit Analog Command Event w/o time */
  pConfig->obj114DefaultVariation = 2;  /* Present Value 0 to 65535 Octets w/o flag */
  pConfig->obj115DefaultVariation = 4;  /* Present Value 0 to 65535 Octets */
  pConfig->obj122DefaultVariation = 1;  /* Security Statistic w/o time */

  /* Default object groups included in a response to read static data request */
  i=0;
#if SDNPDATA_SUPPORT_OBJ1
  pConfig->staticGroups[i++] = DNPDEFS_OBJ_1_BIN_INPUTS;
#endif
#if SDNPDATA_SUPPORT_OBJ3
  pConfig->staticGroups[i++] = DNPDEFS_OBJ_3_DBL_INPUTS;
#endif
#if SDNPDATA_SUPPORT_OBJ10
  pConfig->staticGroups[i++] = DNPDEFS_OBJ_10_BIN_OUT_STATUSES;
#endif
#if SDNPDATA_SUPPORT_OBJ20
  pConfig->staticGroups[i++] = DNPDEFS_OBJ_20_RUNNING_CNTRS;
#endif
#if SDNPDATA_SUPPORT_OBJ21
  pConfig->staticGroups[i++] = DNPDEFS_OBJ_21_FROZEN_CNTRS;
#endif
#if SDNPDATA_SUPPORT_OBJ30
  pConfig->staticGroups[i++] = DNPDEFS_OBJ_30_ANA_INPUTS;
#endif
#if SDNPDATA_SUPPORT_OBJ31
  /* an individual analog input point should either be sent in response to a class0 point as either an object 30 or 31. 
   * This functionality is provided by calls to sdnpdata_anlgInIsClass0 and sdnpdata_frznAnlgInIsClass0 
   */
  pConfig->staticGroups[i++] = DNPDEFS_OBJ_31_FRZN_ANA_INPUTS;
#endif
#if SDNPDATA_SUPPORT_OBJ40
  pConfig->staticGroups[i++] = DNPDEFS_OBJ_40_ANA_OUT_STATUSES;
#endif
#if SDNPDATA_SUPPORT_OBJ120
  /* As specified in table 5-1 in IEEE 1815-2012 */
  pConfig->staticGroups[i++] = DNPDEFS_OBJ_121_AUTHSECSTATS;
#endif
  

  /* Set rest of the entries to zero for empty */
  for(; i < SDNPCNFG_MAX_NUMBER_STATIC_GROUPS; i++)
  {
    pConfig->staticGroups[i] = 0;
  }

  /* Which event to delete from a full queue */
  pConfig->deleteOldestEvent = TMWDEFS_FALSE;

#if SDNPDATA_SUPPORT_OBJ2
  /* Binary Input Event Configuration */
  pConfig->binaryInputMaxEvents = 100;
  pConfig->binaryInputScanPeriod = 0;
  pConfig->binaryInputEventMode = TMWDEFS_EVENT_MODE_SOE;
#endif

#if SDNPDATA_SUPPORT_OBJ4
  /* Double Bit Input Event Configuration */
  pConfig->doubleInputMaxEvents = 100;
  pConfig->doubleInputScanPeriod = 0;
  pConfig->doubleInputEventMode = TMWDEFS_EVENT_MODE_SOE;
#endif

#if SDNPDATA_SUPPORT_OBJ11
  /* Binary Output Event Configuration */
  pConfig->binaryOutputMaxEvents = 100;
  pConfig->binaryOutputScanPeriod = 0;
  pConfig->binaryOutputEventMode = TMWDEFS_EVENT_MODE_SOE;
#endif

#if SDNPDATA_SUPPORT_OBJ13
  /* Binary Output Command Event Configuration */
  pConfig->binaryOutCmdMaxEvents = 100;
  pConfig->binaryOutCmdScanPeriod = 0;
  pConfig->binaryOutCmdEventMode = TMWDEFS_EVENT_MODE_SOE;
#endif

#if SDNPDATA_SUPPORT_OBJ22
  /* Binary Counter Event Configuration */ 
  pConfig->binaryCounterMaxEvents = 30;
  pConfig->binaryCounterScanPeriod = 0;
  pConfig->binaryCounterEventMode = TMWDEFS_EVENT_MODE_MOST_RECENT;
#endif

#if SDNPDATA_SUPPORT_OBJ23
  /* Frozen Counter Event Configuration */
  pConfig->frozenCounterMaxEvents = 30;
  pConfig->frozenCounterScanPeriod = 0;
  pConfig->frozenCounterEventMode = TMWDEFS_EVENT_MODE_SOE;
#endif

#if SDNPDATA_SUPPORT_OBJ32
  /* Analog Input Event Configuration */
  pConfig->analogInputMaxEvents = 30;
  pConfig->analogInputScanPeriod = 0;
  pConfig->analogInputEventMode = TMWDEFS_EVENT_MODE_MOST_RECENT;
#endif

#if SDNPDATA_SUPPORT_OBJ33
  /* Frozen Analog Input Event Configuration */
  pConfig->frozenAnalogInMaxEvents = 30;
  pConfig->frozenAnalogInScanPeriod = 0;
  pConfig->frozenAnalogInEventMode = TMWDEFS_EVENT_MODE_SOE;
#endif

#if SDNPDATA_SUPPORT_OBJ42
  /* Analog Output Event Configuration */
  pConfig->analogOutputMaxEvents = 100;
  pConfig->analogOutputScanPeriod = 0;
  pConfig->analogOutputEventMode = TMWDEFS_EVENT_MODE_SOE;
#endif

#if SDNPDATA_SUPPORT_OBJ43
  /* Analog Output Command Event Configuration */
  pConfig->analogOutCmdMaxEvents = 100;
  pConfig->analogOutCmdScanPeriod = 0;
  pConfig->analogOutCmdEventMode = TMWDEFS_EVENT_MODE_SOE;
#endif

#if SDNPDATA_SUPPORT_OBJ88
  /* Data Set Snapshot Event Configuration */
  pConfig->datasetMaxEvents = 30;
  pConfig->datasetEventMode = TMWDEFS_EVENT_MODE_SOE;
#endif
  
#if SDNPDATA_SUPPORT_OBJ110
  pConfig->truncateStrings = TMWDEFS_TRUE;
#if SDNPDATA_SUPPORT_OBJ111
  /* String Event Configuration */
  pConfig->stringMaxEvents = 10;
  pConfig->stringScanPeriod = 0;
  pConfig->stringEventMode = TMWDEFS_EVENT_MODE_SOE;
#endif
#endif

#if SDNPDATA_SUPPORT_OBJ113
  /* Virtual Terminal Event Configuration */
  pConfig->virtualTerminalMaxEvents = 5;
  pConfig->virtualTerminalScanPeriod = 0;
  pConfig->truncateVirtualTerminalEvents = TMWDEFS_TRUE;
  pConfig->virtualTerminalEventMode = TMWDEFS_EVENT_MODE_SOE;
#endif

#if SDNPDATA_SUPPORT_OBJ115
  /* String Event Configuration */
  pConfig->extStringMaxEvents = 10;
  pConfig->extStringScanPeriod = 0;
  pConfig->extStringEventMode = TMWDEFS_EVENT_MODE_SOE;
#endif

#if SDNPDATA_SUPPORT_OBJ70
  /* File Transfer Configuration */
  pConfig->fileTransferTimeout = TMWDEFS_SECONDS(60);
#endif

  /* User provided statistics callback function */
  pConfig->pStatCallback = TMWDEFS_NULL;
  pConfig->pStatCallbackParam = TMWDEFS_NULL;
  
#if SDNPDATA_SUPPORT_OBJ120 
  pConfig->authenticationEnabled = TMWDEFS_FALSE;
  pConfig->authSecStatMaxEvents  = 10; 
  pConfig->authSecStatEventMode  = TMWDEFS_EVENT_MODE_SOE;
  sdnpauth_initConfig(&pConfig->authConfig);
#endif

#if SDNPDATA_SUPPORT_XML2
  pConfig->xml2WriteDnpData = TMWDEFS_TRUE;
#endif

#if SDNPDATA_SUPPORT_STATIC_VAR_POINT || SDNPDATA_SUPPORT_EVENT_VAR_POINT
  pConfig->counterSizeVarPerPoint   = TMWDEFS_TRUE;
  pConfig->analogInSizeVarPerPoint  = TMWDEFS_TRUE;
  pConfig->analogOutSizeVarPerPoint = TMWDEFS_TRUE;
#endif
}

#if DNPCNFG_SUPPORT_BINCONFIG
/* function: sdnpsesn_initConfigUsingBinary */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsesn_applyBinaryFileValues(
    char * pFileName,
    DNPCHNL_CONFIG *pDNPConfig,
    DNPLINK_CONFIG *pLinkConfig, 
    void *pIoConfig,
    SDNPSESN_CONFIG *pSesnConfig)
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
  binFileTargValues.sessionIsOutstation = TMWDEFS_TRUE;


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
    success = sdnpsesn_getBinFileSessionValues(pSesnConfig, &binFileValues, isChannelSerial, xmlVersionIndex);

  if(success)
    success = dnpchnl_getBinFileChannelValues(pDNPConfig, pLinkConfig, &binFileValues, TMWDEFS_TRUE);


  return success;
}
#endif

#if DNPCNFG_SUPPORT_AUTHENTICATION && DNPCNFG_MULTI_SESSION_REQUESTS
void TMWDEFS_CALLBACK _saveLastUnsolSent(TMWSESN *pSession, TMWSESN_TX_DATA *pTxData)
{
  SDNPSESN *pSDNPSession = (SDNPSESN*)pSession;
  memcpy(pSDNPSession->lastUnsolTxFragment, pTxData->pMsgBuf, pTxData->msgLength);
  pSDNPSession->lastUnsolTxFragmentLength = pTxData->msgLength;
}
#endif

/* function: sdnpsesn_openSession */
TMWSESN * TMWDEFS_GLOBAL sdnpsesn_openSession(
  TMWCHNL *pChannel,
  const SDNPSESN_CONFIG *pConfig,
  void *pUserHandle)
{
  SDNPSESN *pSDNPSession;
  TMWSESN  *pSession;
  int i;

  if(!tmwappl_getInitialized(TMWAPPL_INIT_SDNP))
  {
    if(!sdnpmem_init(TMWDEFS_NULL))
      return(TMWDEFS_NULL);

#if TMWCNFG_SUPPORT_DIAG
    sdnpdiag_init();
#endif
    tmwappl_setInitialized(TMWAPPL_INIT_SDNP);
  }

  /* Allocate space for session context */
  pSDNPSession = (SDNPSESN *)sdnpmem_alloc(SDNPMEM_SDNPSESN_TYPE);

  if(pSDNPSession == TMWDEFS_NULL)
  {
    return(TMWDEFS_NULL);
  }

  pSDNPSession->dnp.pBeforeTxCallback = _beforeTxCallback;
  pSDNPSession->dnp.pAfterTxCallback = _afterTxCallback;
  pSDNPSession->dnp.pFailedTxCallback = _failedTxCallback;
  pSDNPSession->dnp.pPrepareMessage = _prepareMessage;
  pSDNPSession->dnp.pAbortMessage = _processError;
  pSDNPSession->dnp.pNextMessage = _processNextMessage;
  pSDNPSession->dnp.pProcessInfoFunc = _infoCallback;
  pSDNPSession->dnp.pProcessFragmentFunc = _processFragment;
  pSDNPSession->dnp.pIdleFunc = _idleCallback;

  pSDNPSession->dnp.pUserHandle = pUserHandle;

  /* Configuration */
  pSession = (TMWSESN*)pSDNPSession;

  pSDNPSession->allStationsConfirmRequired = TMWDEFS_FALSE;

#if SDNPDATA_SUPPORT_OBJ120 
  /* These two must be set properly before sdnpdata_init is called to determine if SA Statistics are required */
  pSDNPSession->authenticationEnabled = pConfig->authenticationEnabled;
  pSDNPSession->dnp.operateInV2Mode = pConfig->authConfig.operateInV2Mode;
#if DNPCNFG_MULTI_SESSION_REQUESTS
  pSDNPSession->dnp.pSaveLastUnsolSentFunc = _saveLastUnsolSent;
#endif
  pSDNPSession->pAuthenticationInfo = TMWDEFS_NULL;
  pSDNPSession->authSendAggrResp = TMWDEFS_FALSE;
  pSDNPSession->authSendAggrUnsol = TMWDEFS_FALSE;
  pSDNPSession->preChallengeApplConf = TMWDEFS_FALSE;
  pSDNPSession->dnp.pCheckData = sdnpauth_OKToSend;
#else
  pSDNPSession->dnp.pCheckData = TMWDEFS_NULL;
#endif

#if SDNPDATA_SUPPORT_OBJ2 && SDNPDATA_SUPPORT_OBJ4
  pSDNPSession->obj2Count = 0;
  pSDNPSession->obj4Count = 0;
#endif

  /* Initialize timers */
  tmwtimer_init(&pSDNPSession->selectTimer);
  tmwtimer_init(&pSDNPSession->unsolRetryTimer);
  tmwtimer_init(&pSDNPSession->clockValidTimer);

  /* This is used in sdnpdata_init if it calls sdnputil_getDateTime */
  pSDNPSession->clockValidPeriod  = pConfig->clockValidPeriod;

 /* Initialize slave database */
  pSDNPSession->pDbHandle = sdnpdata_init(pSession, pUserHandle);
  if(pSDNPSession->pDbHandle == TMWDEFS_NULL)
  {
    /* Log error */
    sdnpmem_free(pSDNPSession);
    return(TMWDEFS_NULL);
  }

  pSession->pChannel = TMWDEFS_NULL;
  pSession->pLinkSession = TMWDEFS_NULL;
  if(!sdnpsesn_setSessionConfig(pSession, pConfig))
  {
    sdnpmem_free(pSDNPSession);
    return(TMWDEFS_NULL);
  }

  /* Initialize unsolicited event processing */
  pSDNPSession->unsolNumRetries = 0;
  pSDNPSession->unsolEventsReady = TMWDEFS_FALSE;
  pSDNPSession->unsolWaitingForConfirm = TMWDEFS_FALSE;
  pSDNPSession->unsolQueued = TMWDEFS_FALSE;
  pSDNPSession->unsolInitialNullConfirmed = TMWDEFS_FALSE;

  /* If unsolicited is not supported set Initial Null Confirmed to true
   * OR
   * If special configuration to disable the required sending of
   * an initial null UR to make some masters happy.
   */
  if (!pSDNPSession->unsolAllowed || pConfig->unsolDontSendInitialNull)
    pSDNPSession->unsolInitialNullConfirmed = TMWDEFS_TRUE;

  pSDNPSession->pUnsolLastResponse = TMWDEFS_NULL;

  for (i = 0; i < TMWDEFS_CLASS_MAX; i++)
  {
    pSDNPSession->unsolNumPending[i] = 0;
    tmwtimer_init(&pSDNPSession->unsolDelayTimer[i]);
    pSDNPSession->unsolDelayTimerParam[i].pSession = pSession;
  }
  pSDNPSession->unsolDelayTimerParam[0].classMask = TMWDEFS_CLASS_MASK_ONE;
  pSDNPSession->unsolDelayTimerParam[1].classMask = TMWDEFS_CLASS_MASK_TWO;
  pSDNPSession->unsolDelayTimerParam[2].classMask = TMWDEFS_CLASS_MASK_THREE;

  pSDNPSession->recvSequenceNumber = 0;
  pSDNPSession->respSequenceNumber = 0;
  pSDNPSession->unsolSequenceNumber = 0;

  pSDNPSession->selectBufferLength = 0;
  pSDNPSession->selectSequenceNumber = 0;

  /* Current response message outstanding */
  pSDNPSession->dnp.pCurrentMessage = TMWDEFS_NULL;

#if SDNPDATA_KEEP_LAST_RESPONSE 
  pSDNPSession->pLastResponse = TMWDEFS_NULL;
#endif

  /* Initialize IIN */
  pSDNPSession->iin = DNPDEFS_IIN_RESTART;
  if (pSDNPSession->respondNeedTime)
    pSDNPSession->iin |= DNPDEFS_IIN_NEED_TIME;

  pSDNPSession->bufferOverFlowSent = TMWDEFS_FALSE;

  pSDNPSession->recordedCurrentTime = TMWDEFS_FALSE;

#if SDNPDATA_SUPPORT_OBJ70
  pSDNPSession->pObj70FileCtrl = TMWDEFS_NULL;
#endif

  /* Initialize pending read request buffer */
  pSDNPSession->readPending = TMWDEFS_FALSE;
  pSDNPSession->databaseLocked = TMWDEFS_FALSE;
  pSDNPSession->duplicateRequestRcvd = TMWDEFS_FALSE;
  pSDNPSession->pendingRequest = TMWDEFS_FALSE;
  pSDNPSession->lastRcvdRequest.pMsgBuf = (TMWTYPES_UCHAR *)sdnpmem_alloc(SDNPMEM_ASDU_BUFFER_TYPE);
  if(pSDNPSession->lastRcvdRequest.pMsgBuf == TMWDEFS_NULL)
  {
    sdnpmem_free(pSDNPSession);
    return(TMWDEFS_NULL);
  }

#if DNPCNFG_SUPPORT_AUTHENTICATION
  if (pConfig->authenticationEnabled)
  {
    pSDNPSession->dnp.saveLastTxFragment = TMWDEFS_TRUE;
  }
  else
  {
    pSDNPSession->dnp.saveLastTxFragment = TMWDEFS_FALSE;
  }

#if DNPCNFG_MULTI_SESSION_REQUESTS
  pSDNPSession->dnp.lastTxFragmentLength = 0;
  pSDNPSession->lastUnsolTxFragmentLength = 0;
#endif
#endif

  /* Initialize report by exception processing */
  sdnprbe_init(pSession);

  /* Lock channel */
  TMWTARG_LOCK_SECTION(&pChannel->lock);
   
  /* Initialize generic DNP session */
  dnpsesn_openSession(pChannel, pSession, pConfig->pStatCallback, pConfig->pStatCallbackParam,
    TMWTYPES_SESSION_TYPE_SLAVE, pConfig->linkStatusPeriod, pConfig->validateSourceAddress,
    pConfig->enableSelfAddress);

#if SDNPDATA_SUPPORT_EVENT_SCAN
  /* Start report by exception processing */
  sdnprbe_open(pSession);
#endif

  /* Unlock channel */
  TMWTARG_UNLOCK_SECTION(&pChannel->lock);

  return(pSession);
}

/* function: sdnpsesn_modifySession */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsesn_modifySession(
  TMWSESN *pSession,
  const SDNPSESN_CONFIG *pConfig,
  TMWTYPES_ULONG configMask)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;

  if((configMask & SDNPSESN_CONFIG_SOURCE) != 0)
  {
    pSession->srcAddress = pConfig->source;
  }

  if((configMask & SDNPSESN_CONFIG_DESTINATION) != 0)
  {
    pSession->destAddress = pConfig->destination;
  }

  if((configMask & SDNPSESN_CONFIG_UNSOL_ALLOWED) != 0)
  {
    pSDNPSession->unsolAllowed = pConfig->unsolAllowed;
  }

  if((configMask & SDNPSESN_CONFIG_UNSOL_MAX_EVENTS) != 0)
  {
    pSDNPSession->unsolMaxEvents[0] = pConfig->unsolClass1MaxEvents;
    pSDNPSession->unsolMaxEvents[1] = pConfig->unsolClass2MaxEvents;
    pSDNPSession->unsolMaxEvents[2] = pConfig->unsolClass3MaxEvents;
  }

  if((configMask & SDNPSESN_CONFIG_SELECT_TIMEOUT) != 0)
  {
    pSDNPSession->selectTimeout = pConfig->selectTimeout;
  }

  if((configMask & SDNPSESN_CONFIG_UNSOL_MAX_DELAY) != 0)
  {
    pSDNPSession->unsolMaxDelay[0] = pConfig->unsolClass1MaxDelay;
    pSDNPSession->unsolMaxDelay[1] = pConfig->unsolClass2MaxDelay;
    pSDNPSession->unsolMaxDelay[2] = pConfig->unsolClass3MaxDelay;
  }

  if((configMask & SDNPSESN_CONFIG_ACTIVE) != 0)
  {
    pSession->active = pConfig->active;
  } 
  
  if((configMask & SDNPSESN_CONFIG_CNFM_TIMEOUT) != 0)
  {
    pSDNPSession->applConfirmTimeout = pConfig->applConfirmTimeout;
  }


  if((configMask & SDNPSESN_CONFIG_MULTI_FRAG) != 0)
  {
    pSDNPSession->multiFragRespAllowed = pConfig->multiFragRespAllowed;
    pSDNPSession->multiFragConfirm = pConfig->multiFragConfirm;
  }

  if((configMask & SDNPSESN_CONFIG_UNSOL_MASK) != 0)
  {
    pSDNPSession->unsolEventMask = pConfig->unsolClassMask;
  }

  if((configMask & SDNPSESN_CONFIG_UNSOL_RETRIES) != 0)
  {
    pSDNPSession->unsolMaxRetries = pConfig->unsolMaxRetries;
  }

  if((configMask & SDNPSESN_CONFIG_UNSOL_DELAY) != 0)
  {
    pSDNPSession->unsolRetryDelay = pConfig->unsolRetryDelay;
  }

  if((configMask & SDNPSESN_CONFIG_UNSOL_OFFDELAY) != 0)
  {
    pSDNPSession->unsolOfflineRetryDelay = pConfig->unsolOfflineRetryDelay;
  }
  
  if((configMask & SDNPSESN_CONFIG_STATIC_GROUPS) != 0)
  {  
    int i;
    for(i=0; i<SDNPCNFG_MAX_NUMBER_STATIC_GROUPS; i++)
    {
      pSDNPSession->staticGroups[i] = pConfig->staticGroups[i];
    }
  }

  return(TMWDEFS_TRUE);
}

/* function: sdnpsesn_getSessionConfig */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsesn_getSessionConfig(
  TMWSESN *pSession,
  SDNPSESN_CONFIG *pConfig)
{
  int i;
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;

  pConfig->active              = pSession->active;
  pConfig->source              = pSession->srcAddress;
  pConfig->destination         = pSession->destAddress;
  pConfig->pStatCallback       = pSession->pStatCallbackFunc;
  pConfig->pStatCallbackParam  = pSession->pStatCallbackParam;

  pConfig->linkStatusPeriod            = pSDNPSession->dnp.linkStatusPeriod;
  pConfig->linkStatusTimeoutDisconnect = pSDNPSession->dnp.linkStatusTimeoutDisconnect;

  pConfig->enableSelfAddress       = pSDNPSession->dnp.enableSelfAddress;
  pConfig->validateSourceAddress   = pSDNPSession->dnp.validateSourceAddress;

  pConfig->multiFragRespAllowed    = pSDNPSession->multiFragRespAllowed;
  pConfig->multiFragConfirm        = pSDNPSession->multiFragConfirm;
  pConfig->fileTransferEnabled     = pSDNPSession->fileTransferEnabled;
  pConfig->sendObj91RespQual7      = pSDNPSession->sendObj91RespQual7;
  pConfig->applConfirmTimeout      = pSDNPSession->applConfirmTimeout;
  pConfig->unsolConfirmTimeout     = pSDNPSession->unsolConfirmTimeout;
  pConfig->selectTimeout           = pSDNPSession->selectTimeout;
  pConfig->respondNeedTime         = pSDNPSession->respondNeedTime;
  pConfig->clockValidPeriod        = pSDNPSession->clockValidPeriod;
  pConfig->warmRestartDelay        = pSDNPSession->warmRestartDelay;
  pConfig->coldRestartDelay        = pSDNPSession->coldRestartDelay;
  pConfig->allowMultiCROBRequests  = pSDNPSession->allowMultiCROBRequests;
  pConfig->maxControlRequests      = pSDNPSession->maxControlRequests;
  pConfig->enabledBroadcastWrites  = pSDNPSession->enabledBroadcastWrites;
  pConfig->enabledBroadcastFCs     = pSDNPSession->enabledBroadcastFCs;
  pConfig->enabledFCs              = pSDNPSession->enabledFCs;
  pConfig->enabledWrites           = pSDNPSession->enabledWrites;
  pConfig->fileTransferTimeout     = pSDNPSession->fileTransferTimeout;
  pConfig->unsolAllowed            = pSDNPSession->unsolAllowed;
  pConfig->sendUnsolWhenOnline     = pSDNPSession->sendUnsolWhenOnline;
  pConfig->unsolSendIdenticalRetry = pSDNPSession->unsolSendIdenticalRetry;
  pConfig->unsolClassMask          = pSDNPSession->unsolEventMask;
  pConfig->unsolSendByClass        = pSDNPSession->unsolSendByClass;
  pConfig->unsolMaxRetries         = pSDNPSession->unsolMaxRetries;
  pConfig->unsolRetryDelay         = pSDNPSession->unsolRetryDelay;
  pConfig->unsolOfflineRetryDelay  = pSDNPSession->unsolOfflineRetryDelay;
  pConfig->unsolOfflineDelayOnce   = pSDNPSession->unsolOfflineDelayOnce;
  pConfig->unsolClass1MaxDelay     = pSDNPSession->unsolMaxDelay[0];
  pConfig->unsolClass2MaxDelay     = pSDNPSession->unsolMaxDelay[1];
  pConfig->unsolClass3MaxDelay     = pSDNPSession->unsolMaxDelay[2];
  pConfig->unsolClass1MaxEvents    = pSDNPSession->unsolMaxEvents[0];
  pConfig->unsolClass2MaxEvents    = pSDNPSession->unsolMaxEvents[1];
  pConfig->unsolClass3MaxEvents    = pSDNPSession->unsolMaxEvents[2];
  pConfig->userManagedEvents       = pSDNPSession->userManagedEvents;
  pConfig->sortEventsByTime        = pSDNPSession->sortEventsByTime;
  pConfig->supportEventsWithTime   = pSDNPSession->supportEventsWithTime;
  pConfig->sesnDiagMask            = pSDNPSession->dnp.tmw.sesnDiagMask;

#if SDNPCNFG_DISABLE_SELOP_SEQ_CHECK
  pConfig->disableSelOpSeqCheck   = pSDNPSession->disableSelOpSeqCheck;
#endif

  pConfig->obj01DefaultVariation   = pSDNPSession->obj01DefaultVariation;
  pConfig->obj02DefaultVariation   = pSDNPSession->obj02DefaultVariation;
  pConfig->obj03DefaultVariation   = pSDNPSession->obj03DefaultVariation;
  pConfig->obj04DefaultVariation   = pSDNPSession->obj04DefaultVariation;
  pConfig->obj10DefaultVariation   = pSDNPSession->obj10DefaultVariation;
  pConfig->obj11DefaultVariation   = pSDNPSession->obj11DefaultVariation;
  pConfig->obj13DefaultVariation   = pSDNPSession->obj13DefaultVariation;
  pConfig->obj20DefaultVariation   = pSDNPSession->obj20DefaultVariation;
  pConfig->obj21DefaultVariation   = pSDNPSession->obj21DefaultVariation;
  pConfig->obj22DefaultVariation   = pSDNPSession->obj22DefaultVariation;
  pConfig->obj23DefaultVariation   = pSDNPSession->obj23DefaultVariation;
  pConfig->obj30DefaultVariation   = pSDNPSession->obj30DefaultVariation;
  pConfig->obj31DefaultVariation   = pSDNPSession->obj31DefaultVariation;
  pConfig->obj32DefaultVariation   = pSDNPSession->obj32DefaultVariation;
  pConfig->obj33DefaultVariation   = pSDNPSession->obj33DefaultVariation;
  pConfig->obj34DefaultVariation   = pSDNPSession->obj34DefaultVariation;
  pConfig->obj40DefaultVariation   = pSDNPSession->obj40DefaultVariation;
  pConfig->obj42DefaultVariation   = pSDNPSession->obj42DefaultVariation;
  pConfig->obj43DefaultVariation   = pSDNPSession->obj43DefaultVariation;
  pConfig->obj114DefaultVariation  = pSDNPSession->obj114DefaultVariation;
  pConfig->obj115DefaultVariation  = pSDNPSession->obj115DefaultVariation;
  pConfig->obj122DefaultVariation  = pSDNPSession->obj122DefaultVariation;

  for(i=0; i<SDNPCNFG_MAX_NUMBER_STATIC_GROUPS; i++)
  {
    pConfig->staticGroups[i] = pSDNPSession->staticGroups[i];
  }

  pConfig->deleteOldestEvent       = pSDNPSession->deleteOldestEvent;
  
#if SDNPDATA_SUPPORT_OBJ2
  pConfig->binaryInputMaxEvents    = pSDNPSession->binaryInputMaxEvents;
  pConfig->binaryInputScanPeriod   = pSDNPSession->binaryInputScanPeriod;
  pConfig->binaryInputEventMode    = pSDNPSession->binaryInputEventMode;
#endif
  
#if SDNPDATA_SUPPORT_OBJ4
  pConfig->doubleInputMaxEvents    = pSDNPSession->doubleInputMaxEvents;
  pConfig->doubleInputScanPeriod   = pSDNPSession->doubleInputScanPeriod;
  pConfig->doubleInputEventMode    = pSDNPSession->doubleInputEventMode;
#endif

#if SDNPDATA_SUPPORT_OBJ11
  pConfig->binaryOutputMaxEvents   = pSDNPSession->binaryOutputMaxEvents;
  pConfig->binaryOutputScanPeriod  = pSDNPSession->binaryOutputScanPeriod;
  pConfig->binaryOutputEventMode   = pSDNPSession->binaryOutputEventMode;
#endif

#if SDNPDATA_SUPPORT_OBJ13
  pConfig->binaryOutCmdMaxEvents   = pSDNPSession->binaryOutCmdMaxEvents;
  pConfig->binaryOutCmdScanPeriod  = pSDNPSession->binaryOutCmdScanPeriod;
  pConfig->binaryOutCmdEventMode   = pSDNPSession->binaryOutCmdEventMode;
#endif
  
#if SDNPDATA_SUPPORT_OBJ22
  pConfig->binaryCounterMaxEvents  = pSDNPSession->binaryCounterMaxEvents;
  pConfig->binaryCounterScanPeriod = pSDNPSession->binaryCounterScanPeriod;
  pConfig->binaryCounterEventMode  = pSDNPSession->binaryCounterEventMode;
#endif
  
#if SDNPDATA_SUPPORT_OBJ23
  pConfig->frozenCounterMaxEvents  = pSDNPSession->frozenCounterMaxEvents;
  pConfig->frozenCounterScanPeriod = pSDNPSession->frozenCounterScanPeriod;
  pConfig->frozenCounterEventMode  = pSDNPSession->frozenCounterEventMode;
#endif

#if SDNPDATA_SUPPORT_OBJ32
  pConfig->analogInputMaxEvents    = pSDNPSession->analogInputMaxEvents;
  pConfig->analogInputScanPeriod   = pSDNPSession->analogInputScanPeriod;
  pConfig->analogInputEventMode    = pSDNPSession->analogInputEventMode;
#endif

#if SDNPDATA_SUPPORT_OBJ33
  pConfig->frozenAnalogInMaxEvents    = pSDNPSession->frozenAnalogInMaxEvents;
  pConfig->frozenAnalogInScanPeriod   = pSDNPSession->frozenAnalogInScanPeriod;
  pConfig->frozenAnalogInEventMode    = pSDNPSession->frozenAnalogInEventMode;
#endif

#if SDNPDATA_SUPPORT_OBJ42
  pConfig->analogOutputMaxEvents   = pSDNPSession->analogOutputMaxEvents;
  pConfig->analogOutputScanPeriod  = pSDNPSession->analogOutputScanPeriod;
  pConfig->analogOutputEventMode   = pSDNPSession->analogOutputEventMode;
#endif

#if SDNPDATA_SUPPORT_OBJ43
  pConfig->analogOutCmdMaxEvents   = pSDNPSession->analogOutCmdMaxEvents;
  pConfig->analogOutCmdScanPeriod  = pSDNPSession->analogOutCmdScanPeriod;
  pConfig->analogOutCmdEventMode   = pSDNPSession->analogOutCmdEventMode;
#endif
  
#if SDNPDATA_SUPPORT_OBJ88
  pConfig->datasetMaxEvents        = pSDNPSession->datasetMaxEvents; 
  pConfig->datasetEventMode        = pSDNPSession->datasetEventMode;
#endif
  
#if SDNPDATA_SUPPORT_OBJ110
  pConfig->truncateStrings         = pSDNPSession->truncateStrings;
#if SDNPDATA_SUPPORT_OBJ111
  pConfig->stringMaxEvents         = pSDNPSession->stringMaxEvents;
  pConfig->stringScanPeriod        = pSDNPSession->stringScanPeriod;
  pConfig->stringEventMode         = pSDNPSession->stringEventMode;
#endif
#endif
  
#if SDNPDATA_SUPPORT_OBJ113
  pConfig->virtualTerminalMaxEvents      = pSDNPSession->virtualTerminalMaxEvents;
  pConfig->truncateVirtualTerminalEvents = pSDNPSession->truncateVirtualTerminalEvents;
  pConfig->virtualTerminalScanPeriod     = pSDNPSession->virtualTerminalScanPeriod;
  pConfig->virtualTerminalEventMode      = pSDNPSession->virtualTerminalEventMode;
#endif

#if SDNPDATA_SUPPORT_OBJ115
  pConfig->extStringMaxEvents         = pSDNPSession->extStringMaxEvents;
  pConfig->extStringScanPeriod        = pSDNPSession->extStringScanPeriod;
  pConfig->extStringEventMode         = pSDNPSession->extStringEventMode;
#endif

#if SDNPDATA_SUPPORT_OBJ120 
  pConfig->authenticationEnabled = pSDNPSession->authenticationEnabled;
  pConfig->authSecStatMaxEvents = pSDNPSession->authSecStatMaxEvents; 
  pConfig->authSecStatEventMode = pSDNPSession->authSecStatEventMode;

  sdnpauth_getConfig(pSDNPSession, &pConfig->authConfig);  
#endif

#if SDNPDATA_SUPPORT_XML2
  pConfig->xml2WriteDnpData = pSDNPSession->xml2WriteDnpData;
#endif

#if SDNPDATA_SUPPORT_STATIC_VAR_POINT || SDNPDATA_SUPPORT_EVENT_VAR_POINT
  pConfig->counterSizeVarPerPoint   = pSDNPSession->counterSizeVarPerPoint;
  pConfig->analogInSizeVarPerPoint  = pSDNPSession->analogInSizeVarPerPoint;
  pConfig->analogOutSizeVarPerPoint = pSDNPSession->analogOutSizeVarPerPoint;
#endif

  return(TMWDEFS_TRUE);
}

/* function: sdnpsesn_setSessionConfig */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsesn_setSessionConfig(
  TMWSESN *pSession,
  const SDNPSESN_CONFIG *pConfig)
{
  int i;
  SDNPSESN *pSDNPSession = (SDNPSESN*)pSession;
  
  pSession->active             = pConfig->active;
  pSession->srcAddress         = pConfig->source;
  pSession->destAddress        = pConfig->destination;
  pSession->pStatCallbackFunc  = pConfig->pStatCallback;
  pSession->pStatCallbackParam = pConfig->pStatCallbackParam;
  
  pSDNPSession->dnp.linkStatusTimeoutDisconnect = pConfig->linkStatusTimeoutDisconnect;
  pSDNPSession->dnp.linkStatusPeriod            = pConfig->linkStatusPeriod;
  if(pSDNPSession->dnp.linkStatusPeriod != 0)
    dnplink_startLinkStatusTimer(pSession);

  pSDNPSession->dnp.enableSelfAddress       = pConfig->enableSelfAddress;
  pSDNPSession->dnp.validateSourceAddress   = pConfig->validateSourceAddress;

  pSDNPSession->multiFragRespAllowed   = pConfig->multiFragRespAllowed;
  pSDNPSession->multiFragConfirm       = pConfig->multiFragConfirm;
  pSDNPSession->fileTransferEnabled    = pConfig->fileTransferEnabled;
  pSDNPSession->sendObj91RespQual7     = pConfig->sendObj91RespQual7;
  pSDNPSession->applConfirmTimeout     = pConfig->applConfirmTimeout;
  pSDNPSession->unsolConfirmTimeout    = pConfig->unsolConfirmTimeout;
  pSDNPSession->selectTimeout          = pConfig->selectTimeout;
  pSDNPSession->respondNeedTime        = pConfig->respondNeedTime;
  pSDNPSession->clockValidPeriod       = pConfig->clockValidPeriod;
  pSDNPSession->warmRestartDelay       = pConfig->warmRestartDelay;
  pSDNPSession->coldRestartDelay       = pConfig->coldRestartDelay;
  pSDNPSession->allowMultiCROBRequests = pConfig->allowMultiCROBRequests;
  if(pConfig->maxControlRequests > SDNPCNFG_MAX_CONTROL_REQUESTS)
    pSDNPSession->maxControlRequests = SDNPCNFG_MAX_CONTROL_REQUESTS;
  else
    pSDNPSession->maxControlRequests   = pConfig->maxControlRequests;
  pSDNPSession->enabledBroadcastFCs    = pConfig->enabledBroadcastFCs;
  pSDNPSession->enabledBroadcastWrites = pConfig->enabledBroadcastWrites;
  pSDNPSession->enabledFCs             = pConfig->enabledFCs;
  pSDNPSession->enabledWrites          = pConfig->enabledWrites;
  pSDNPSession->fileTransferTimeout    = pConfig->fileTransferTimeout;
  pSDNPSession->unsolAllowed           = pConfig->unsolAllowed;
  pSDNPSession->sendUnsolWhenOnline    = pConfig->sendUnsolWhenOnline;
  pSDNPSession->unsolSendIdenticalRetry= pConfig->unsolSendIdenticalRetry;

  pSDNPSession->unsolEventMask         = pConfig->unsolClassMask;
  pSDNPSession->unsolSendByClass       = pConfig->unsolSendByClass;
  pSDNPSession->unsolMaxRetries        = pConfig->unsolMaxRetries;
  pSDNPSession->unsolRetryDelay        = pConfig->unsolRetryDelay;
  pSDNPSession->unsolOfflineRetryDelay = pConfig->unsolOfflineRetryDelay;
  pSDNPSession->unsolOfflineDelayOnce  = pConfig->unsolOfflineDelayOnce;
  pSDNPSession->unsolMaxDelay[0]       = pConfig->unsolClass1MaxDelay;
  pSDNPSession->unsolMaxDelay[1]       = pConfig->unsolClass2MaxDelay;
  pSDNPSession->unsolMaxDelay[2]       = pConfig->unsolClass3MaxDelay;
  pSDNPSession->unsolMaxEvents[0]      = pConfig->unsolClass1MaxEvents;
  pSDNPSession->unsolMaxEvents[1]      = pConfig->unsolClass2MaxEvents;
  pSDNPSession->unsolMaxEvents[2]      = pConfig->unsolClass3MaxEvents;
  pSDNPSession->userManagedEvents      = pConfig->userManagedEvents;
  pSDNPSession->sortEventsByTime       = pConfig->sortEventsByTime;
  pSDNPSession->supportEventsWithTime  = pConfig->supportEventsWithTime;
  pSDNPSession->dnp.tmw.sesnDiagMask   = pConfig->sesnDiagMask;

#if SDNPCNFG_DISABLE_SELOP_SEQ_CHECK
  pSDNPSession->disableSelOpSeqCheck   = pConfig->disableSelOpSeqCheck;
#endif

  pSDNPSession->obj01DefaultVariation  = pConfig->obj01DefaultVariation;
  pSDNPSession->obj02DefaultVariation  = pConfig->obj02DefaultVariation;
  pSDNPSession->obj03DefaultVariation  = pConfig->obj03DefaultVariation;
  pSDNPSession->obj04DefaultVariation  = pConfig->obj04DefaultVariation;
  pSDNPSession->obj10DefaultVariation  = pConfig->obj10DefaultVariation;
  pSDNPSession->obj11DefaultVariation  = pConfig->obj11DefaultVariation;
  pSDNPSession->obj13DefaultVariation  = pConfig->obj13DefaultVariation;
  pSDNPSession->obj20DefaultVariation  = pConfig->obj20DefaultVariation;
  pSDNPSession->obj21DefaultVariation  = pConfig->obj21DefaultVariation;
  pSDNPSession->obj22DefaultVariation  = pConfig->obj22DefaultVariation;
  pSDNPSession->obj23DefaultVariation  = pConfig->obj23DefaultVariation;
  pSDNPSession->obj30DefaultVariation  = pConfig->obj30DefaultVariation;
  pSDNPSession->obj31DefaultVariation  = pConfig->obj31DefaultVariation;
  pSDNPSession->obj32DefaultVariation  = pConfig->obj32DefaultVariation;
  pSDNPSession->obj33DefaultVariation  = pConfig->obj33DefaultVariation;
  pSDNPSession->obj34DefaultVariation  = pConfig->obj34DefaultVariation;
  pSDNPSession->obj40DefaultVariation  = pConfig->obj40DefaultVariation;
  pSDNPSession->obj42DefaultVariation  = pConfig->obj42DefaultVariation;
  pSDNPSession->obj43DefaultVariation  = pConfig->obj43DefaultVariation;
  pSDNPSession->obj114DefaultVariation = pConfig->obj114DefaultVariation;
  pSDNPSession->obj115DefaultVariation = pConfig->obj115DefaultVariation;
  pSDNPSession->obj122DefaultVariation = pConfig->obj122DefaultVariation;

#if SDNPDATA_SUPPORT_XML2
  pSDNPSession->xml2WriteDnpData = pConfig->xml2WriteDnpData;
#endif

#if SDNPDATA_SUPPORT_STATIC_VAR_POINT || SDNPDATA_SUPPORT_EVENT_VAR_POINT
  pSDNPSession->counterSizeVarPerPoint   = pConfig->counterSizeVarPerPoint;
  pSDNPSession->analogInSizeVarPerPoint  = pConfig->analogInSizeVarPerPoint;
  pSDNPSession->analogOutSizeVarPerPoint = pConfig->analogOutSizeVarPerPoint;
#endif

  for(i=0; i<SDNPCNFG_MAX_NUMBER_STATIC_GROUPS; i++)
  {
    pSDNPSession->staticGroups[i] = pConfig->staticGroups[i];
  }

  /* which event to delete on overflow */
  pSDNPSession->deleteOldestEvent = pConfig->deleteOldestEvent;

#if SDNPDATA_SUPPORT_OBJ2
  /* Binary Input Event Configuration */
  pSDNPSession->binaryInputMaxEvents  = pConfig->binaryInputMaxEvents;
  pSDNPSession->binaryInputScanPeriod = pConfig->binaryInputScanPeriod;
  pSDNPSession->binaryInputEventMode  = pConfig->binaryInputEventMode;
#endif

#if SDNPDATA_SUPPORT_OBJ4
  /* Double Bit Input Event Configuration */
  pSDNPSession->doubleInputMaxEvents  = pConfig->doubleInputMaxEvents;
  pSDNPSession->doubleInputScanPeriod = pConfig->doubleInputScanPeriod;
  pSDNPSession->doubleInputEventMode  = pConfig->doubleInputEventMode;
#endif

#if SDNPDATA_SUPPORT_OBJ11
  /* Binary Output Event Configuration */
  pSDNPSession->binaryOutputMaxEvents  = pConfig->binaryOutputMaxEvents;
  pSDNPSession->binaryOutputScanPeriod = pConfig->binaryOutputScanPeriod;
  pSDNPSession->binaryOutputEventMode  = pConfig->binaryOutputEventMode;
#endif

#if SDNPDATA_SUPPORT_OBJ13
  /* Binary Output Command Event Configuration */
  pSDNPSession->binaryOutCmdMaxEvents  = pConfig->binaryOutCmdMaxEvents;
  pSDNPSession->binaryOutCmdScanPeriod = pConfig->binaryOutCmdScanPeriod;
  pSDNPSession->binaryOutCmdEventMode  = pConfig->binaryOutCmdEventMode;
#endif

#if SDNPDATA_SUPPORT_OBJ22
  /* Binary Counter Event Configuration */
  pSDNPSession->binaryCounterMaxEvents  = pConfig->binaryCounterMaxEvents;
  pSDNPSession->binaryCounterScanPeriod = pConfig->binaryCounterScanPeriod;
  pSDNPSession->binaryCounterEventMode  = pConfig->binaryCounterEventMode;
#endif

#if SDNPDATA_SUPPORT_OBJ23
  /* Frozen Counter Event Configuration */
  pSDNPSession->frozenCounterMaxEvents  = pConfig->frozenCounterMaxEvents;
  pSDNPSession->frozenCounterScanPeriod = pConfig->frozenCounterScanPeriod;
  pSDNPSession->frozenCounterEventMode  = pConfig->frozenCounterEventMode;
#endif

#if SDNPDATA_SUPPORT_OBJ32
  /* Analog Input Event Configuration */
  pSDNPSession->analogInputMaxEvents  = pConfig->analogInputMaxEvents;
  pSDNPSession->analogInputScanPeriod = pConfig->analogInputScanPeriod;
  pSDNPSession->analogInputEventMode  = pConfig->analogInputEventMode;
#endif

#if SDNPDATA_SUPPORT_OBJ33
  /* Frozen Analog Input Event Configuration */
  pSDNPSession->frozenAnalogInMaxEvents  = pConfig->frozenAnalogInMaxEvents;
  pSDNPSession->frozenAnalogInScanPeriod = pConfig->frozenAnalogInScanPeriod;
  pSDNPSession->frozenAnalogInEventMode  = pConfig->frozenAnalogInEventMode;
#endif

#if SDNPDATA_SUPPORT_OBJ42
  /* Analog Output Event Configuration */
  pSDNPSession->analogOutputMaxEvents  = pConfig->analogOutputMaxEvents;
  pSDNPSession->analogOutputScanPeriod = pConfig->analogOutputScanPeriod;
  pSDNPSession->analogOutputEventMode  = pConfig->analogOutputEventMode;
#endif

#if SDNPDATA_SUPPORT_OBJ43
  /* Analog Output Command Event Configuration */
  pSDNPSession->analogOutCmdMaxEvents  = pConfig->analogOutCmdMaxEvents;
  pSDNPSession->analogOutCmdScanPeriod = pConfig->analogOutCmdScanPeriod;
  pSDNPSession->analogOutCmdEventMode  = pConfig->analogOutCmdEventMode;
#endif

#if SDNPDATA_SUPPORT_OBJ88
  pSDNPSession->datasetMaxEvents        = pConfig->datasetMaxEvents; 
  pSDNPSession->datasetEventMode        = pConfig->datasetEventMode;
#endif
  
#if SDNPDATA_SUPPORT_OBJ110
  pSDNPSession->truncateStrings = pConfig->truncateStrings;
#if SDNPDATA_SUPPORT_OBJ111
  /* String Event Configuration */
  pSDNPSession->stringMaxEvents = pConfig->stringMaxEvents;
  pSDNPSession->stringScanPeriod = pConfig->stringScanPeriod;
  pSDNPSession->stringEventMode = pConfig->stringEventMode;
#endif
#endif

#if SDNPDATA_SUPPORT_OBJ113
  /* Virtual Terminal Event Configuration */
  pSDNPSession->virtualTerminalMaxEvents = pConfig->virtualTerminalMaxEvents;
  pSDNPSession->truncateVirtualTerminalEvents = pConfig->truncateVirtualTerminalEvents;
  pSDNPSession->virtualTerminalScanPeriod = pConfig->virtualTerminalScanPeriod;
  pSDNPSession->virtualTerminalEventMode = pConfig->virtualTerminalEventMode;
#endif

#if SDNPDATA_SUPPORT_OBJ115
  /* Extended String Event Configuration */
  pSDNPSession->extStringMaxEvents = pConfig->extStringMaxEvents;
  pSDNPSession->extStringScanPeriod = pConfig->extStringScanPeriod;
  pSDNPSession->extStringEventMode = pConfig->extStringEventMode;
#endif

#if SDNPDATA_SUPPORT_OBJ120 
  pSDNPSession->authSecStatMaxEvents  = pConfig->authSecStatMaxEvents; 
  pSDNPSession->authSecStatEventMode  = pConfig->authSecStatEventMode;
  if(pConfig->authenticationEnabled)
  {
    /* If authentication was already enabled (for a modify) */
    if (pSDNPSession->pAuthenticationInfo != TMWDEFS_NULL)
    {
      /* If the version was changed from 2 to 5 or 5 to 2 close authentication */
      if (pSDNPSession->dnp.operateInV2Mode != pConfig->authConfig.operateInV2Mode)
      {
        sdnpauth_close(pSDNPSession);
        pSDNPSession->pAuthenticationInfo = TMWDEFS_NULL;
        pSDNPSession->authenticationEnabled = TMWDEFS_FALSE;
      }
    }

    pSDNPSession->dnp.operateInV2Mode = pConfig->authConfig.operateInV2Mode;
    if(pSDNPSession->pAuthenticationInfo == TMWDEFS_NULL)
    {
      pSDNPSession->pAuthenticationInfo = sdnpauth_init(pSDNPSession, &pConfig->authConfig); 
      if(pSDNPSession->pAuthenticationInfo == TMWDEFS_NULL)
      {
        return(TMWDEFS_FALSE);
      }
    }
    sdnpauth_setConfig(pSDNPSession, &pConfig->authConfig); 
  }
  else
  {
    /* If secure authentication was enabled, clean up. */
    if (pSDNPSession->pAuthenticationInfo != TMWDEFS_NULL)
    {
      sdnpauth_close(pSDNPSession);
      pSDNPSession->pAuthenticationInfo = TMWDEFS_NULL;
    }
  }
  pSDNPSession->authenticationEnabled = pConfig->authenticationEnabled;
#endif

  /* If this is a modify */
  if(pSession->pChannel != TMWDEFS_NULL)
  {
#if SDNPDATA_SUPPORT_OBJ120
    if(pConfig->authenticationEnabled)
      pSDNPSession->dnp.saveLastTxFragment = TMWDEFS_TRUE;
    else
      pSDNPSession->dnp.saveLastTxFragment = TMWDEFS_FALSE;
#endif

#if SDNPDATA_SUPPORT_EVENT_SCAN
    /* restart report by exception processing to use new scan values */
    TMWTARG_LOCK_SECTION(&pSession->pChannel->lock);
    sdnprbe_open(pSession);
    TMWTARG_UNLOCK_SECTION(&pSession->pChannel->lock);
#endif
  }

  return(TMWDEFS_TRUE);
}

/* function: sdnpsesn_closeSession */
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsesn_closeSession(
  TMWSESN *pSession)
{
  int i;
#if TMWCNFG_SUPPORT_THREADS
  TMWDEFS_RESOURCE_LOCK *pLock;
#endif
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;

  /* Check for NULL since this would be a common error */
  if(pSession == TMWDEFS_NULL)
  {
    return(TMWDEFS_FALSE);
  }

#if TMWCNFG_SUPPORT_THREADS
  /* Lock channel */
  pLock = &pSession->pChannel->lock;
  TMWTARG_LOCK_SECTION(pLock);
#endif

  /* remove any outstanding message from the request queue */
  dnpchnl_deleteFragments(pSession);
   
#if SDNPDATA_KEEP_LAST_RESPONSE 
  if(pSDNPSession->pLastResponse != TMWDEFS_NULL)
  {
    dnpchnl_freeTxData((TMWSESN_TX_DATA*)pSDNPSession->pLastResponse); 
  }
#endif
  
#if SDNPDATA_SUPPORT_IDENT_UNSOL_RETRY
  if(pSDNPSession->pUnsolLastResponse != TMWDEFS_NULL)
  {
    dnpchnl_freeTxData((TMWSESN_TX_DATA *)pSDNPSession->pUnsolLastResponse);
    pSDNPSession->pUnsolLastResponse = TMWDEFS_NULL;
  }
#endif

  /* Cancel report by exception processing for this session */
  sdnprbe_close(pSession);

  /* Cancel any pending unsolicited events */
  tmwtimer_cancel(&pSDNPSession->unsolRetryTimer);

  for(i = 0; i < TMWDEFS_CLASS_MAX; i++)
    tmwtimer_cancel(&pSDNPSession->unsolDelayTimer[i]);

  /* Clock valid timer */
  tmwtimer_cancel(&pSDNPSession->clockValidTimer);

  /* Select timer */
  tmwtimer_cancel(&pSDNPSession->selectTimer);

#if SDNPDATA_SUPPORT_OBJ70
  if(pSDNPSession->pObj70FileCtrl != TMWDEFS_NULL)
  {
    SDNPO070_FILE_CONTROL_BLOCK *pObj70FileCtrl = 
      (SDNPO070_FILE_CONTROL_BLOCK *)pSDNPSession->pObj70FileCtrl;

    tmwtimer_cancel(&pObj70FileCtrl->fileTransferTimer);
    tmwtimer_cancel(&pObj70FileCtrl->retryTimer);
    sdnpmem_free(pObj70FileCtrl);
  }
#endif

#if SDNPDATA_SUPPORT_OBJ120
  if(pSDNPSession->authenticationEnabled)
  {
    sdnpauth_close(pSDNPSession); 
    pSDNPSession->authenticationEnabled = TMWDEFS_FALSE;
  }
#endif

  /* Close database */
  sdnpdata_close(pSDNPSession->pDbHandle);

  /* Close generic DNP session */
  dnpsesn_closeSession(pSession);
  
  /* Free memory */
  sdnpmem_free(pSDNPSession->lastRcvdRequest.pMsgBuf);
  sdnpmem_free(pSDNPSession);

  /* Unlock channel */
  TMWTARG_UNLOCK_SECTION(pLock);

  return(TMWDEFS_TRUE);
}

SDNPSESN_READ_FUNC TMWDEFS_GLOBAL sdnpsesn_getStaticReadFunc(
  TMWTYPES_UCHAR group,
  TMWTYPES_UCHAR variation)
{
  int index = 0;

  while(_sdnpObjReadFuncTable[index].pReadFunc != TMWDEFS_NULL)
  {
    if((_sdnpObjReadFuncTable[index].group == group)
      && (_sdnpObjReadFuncTable[index].allVariations
      || (_sdnpObjReadFuncTable[index].variation == variation)))
    {
      return(_sdnpObjReadFuncTable[index].pReadFunc);
    }
    index += 1;
  }
  return(TMWDEFS_NULL);
} 

/* function: sdnpsesn_requestApplConfirm */
void TMWDEFS_GLOBAL sdnpsesn_requestApplConfirm(
  TMWSESN *pSession,
  TMWSESN_TX_DATA *pResponse)
{
#if SDNPDATA_SUPPORT_OBJ120
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  if (pSDNPSession->authenticationEnabled && pSDNPSession->preChallengeApplConf)
  {
    /* Set max length back for the challenge object */
    pResponse->maxLength += SDNPAUTH_PRECHALLENGEPAD;
    sdnpauth_addPreChallengeV1(pSDNPSession, pResponse);
  }
#else
  TMWTARG_UNUSED_PARAM(pSession);
#endif
  pResponse->pMsgBuf[0] |= DNPDEFS_AC_CONFIRM;
  pResponse->txFlags &= ~TMWSESN_TXFLAGS_NO_RESPONSE;
}

#if DNPCNFG_SUPPORT_BINCONFIG
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsesn_getBinFileSessionValues(
   SDNPSESN_CONFIG *pSesnConfig,
   DNPBNCFG_FILEVALUES *pBinFileValues,
   TMWTYPES_BOOL isChannelSerial,
   TMWTYPES_USHORT xmlVersionIndex)
{

  /* section 1.2.5 */
  if(isChannelSerial && (pBinFileValues->fieldsUsed1_2 & CRTL_LINKSTATUSINT1_2_5))
    pSesnConfig->linkStatusPeriod = pBinFileValues->linkStatusInterval1_2;
  
  
  /* section 1.3.10 */
  if(!isChannelSerial && (pBinFileValues->fieldsUsed1_3 & CRTL_TCPKEEPALIVETIMER1_3_10))
    pSesnConfig->linkStatusPeriod = pBinFileValues->tcpKeepAliveTimer1_3;

  /* section 1.4.1 */
  if(pBinFileValues->fieldsUsed1_4 & CRTL_DATALINKADDRESS1_4_1)
    pSesnConfig->source = (TMWTYPES_USHORT)pBinFileValues->dataLinkAddress1_4;

  /* section 1.4.2 */
  if(pBinFileValues->fieldsUsed1_4 & CRTL_DNP3SOURCEADDRVALIDATION1_4_2)
  {
    if((pBinFileValues->dnp3SourceAddrValidation1_4 & SOURCEADDRVAL_NEVER) == SOURCEADDRVAL_NEVER)
      pSesnConfig->validateSourceAddress = TMWDEFS_FALSE;
    else 
      pSesnConfig->validateSourceAddress = TMWDEFS_TRUE;
  }

  /* section 1.4.3 */
  if(pBinFileValues->fieldsUsed1_4 & CRTL_SOURCEADDRSEXPECTEDWITHVAL1_4_3)
    pSesnConfig->destination = (TMWTYPES_USHORT)pBinFileValues->dnp3SourceAddrs1_4;


  /* section 1.4.4 */
  if(pBinFileValues->fieldsUsed1_4 & CRTL_SELFADDRUSINGFFFC1_4_4)
  {
    if(pBinFileValues->specialControls1_4 & SPCTRL_SUPPORTSSELFADDRUSINGFFFC1_4_4)
      pSesnConfig->enableSelfAddress = TMWDEFS_TRUE;
    else
      pSesnConfig->enableSelfAddress = TMWDEFS_FALSE;
  }


  /* section 1.5.5 */
  if(pBinFileValues->fieldsUsed1_5 & CRTL_MAXOBJECTSINCROBCONTROLREQUEST1_5_5)
  {
    if(pBinFileValues->maxObjsInCROBControlRequest1_5 == 0)
      pSesnConfig->allowMultiCROBRequests = TMWDEFS_FALSE;
    else
    {
      pSesnConfig->allowMultiCROBRequests = TMWDEFS_TRUE;
      pSesnConfig->maxControlRequests = (TMWTYPES_UCHAR)pBinFileValues->maxObjsInCROBControlRequest1_5;
    }
  }

  /* section 1.7.1 */
  if(pBinFileValues->fieldsUsed1_7 & CRTL_APPLICATIONLAYERCONFIRMTIMEOUT1_7_1)
  {
    if(pBinFileValues->specialControls1_7 & SPCTRL_NONEAPPLICATIONLAYERCONFIRMTIMEOUT1_7_1)
      pSesnConfig->applConfirmTimeout = 0;
    else
    {
      if(!(pBinFileValues->specialControls1_7 & SPCTRL_VARIABLEAPPLICATIONLAYERCONFIRMTIMEOUT1_7_1))
      {
        pSesnConfig->applConfirmTimeout = pBinFileValues->applLayerConfirmTimeout1_7;
      }
    }
  }

  /* section 1.7.2 */
  if(pBinFileValues->fieldsUsed1_7 & CRTL_TIMESYNCREQUIRED1_7_2)
  {
    if(pBinFileValues->timeSyncRequired1_7 & TIMESYNCREQUIREDOPTIONS_NEVER)
      pSesnConfig->respondNeedTime = TMWDEFS_FALSE;
    else
      pSesnConfig->respondNeedTime = TMWDEFS_TRUE;
  }

  /* section 1.7.4 */
  if(pBinFileValues->fieldsUsed1_7 & CRTL_FILEHANDLETIMEOUT1_7_4)
  {
    if(!(pBinFileValues->specialControls1_7 & SPCTRL_NOTAPPLICABLEFILEHANDLETIMEOUT1_7_4) &&
        !(pBinFileValues->specialControls1_7 & SPCTRL_VARIABLEFILEHANDLETIMEOUT1_7_4))
    {
      pSesnConfig->fileTransferTimeout = pBinFileValues->fileHandleTimeout1_7;
    }
  }

  /* section 1.7.5 */
  if(pBinFileValues->fieldsUsed1_7 & CRTL_EVENTBUFFEROVERFLOWBEHAVIOR1_7_5)
  {
    if(pBinFileValues->eventBufOverflowBehavior1_7 & EVENTBUFFEROVERFLOWBEHAVIOROPTIONS_DISCARDOLDEST)
      pSesnConfig->deleteOldestEvent = TMWDEFS_TRUE;
    else 
      pSesnConfig->deleteOldestEvent = TMWDEFS_FALSE;
  }

  /* section 1.7.7 */
  if(pBinFileValues->fieldsUsed1_7 & CRTL_SENDSMULTIFRAGMENTRESPONSES1_7_7)
  {
    if(pBinFileValues->specialControls1_7 & SPCTRL_YESSENDSMULTIFRAGMENTRESPONSES1_7_7)
      pSesnConfig->multiFragRespAllowed = TMWDEFS_TRUE;
    else
      pSesnConfig->multiFragRespAllowed = TMWDEFS_FALSE;
  }

  /* section 1.7.11 */
  if (xmlVersionIndex >= 2)  /* Nov2013 or newer only */
  {
    if(pBinFileValues->fieldsUsed1_7 & CRTL_REQUESTSAPPLICATIONCONFIRMATION1_7_11)
    {
      if(pBinFileValues->specialControls1_7 & SPCTRL_YESNONFINALFRAGMENTS1_7_11)
        pSesnConfig->multiFragConfirm = TMWDEFS_TRUE;
      else
        pSesnConfig->multiFragConfirm = TMWDEFS_FALSE;
    }
  }


  /* section 1.8.1 */
  if(pBinFileValues->fieldsUsed1_8 & CRTL_SUPPORTSUNSOLICITEDREPORTING1_8_1)
  {
    if(pBinFileValues->specialControls1_8 & SPCTRL_ONSUPPORTSUNSOLICITEDREPORTING1_8_1)
      pSesnConfig->unsolAllowed = TMWDEFS_TRUE;
    else
      pSesnConfig->unsolAllowed = TMWDEFS_FALSE;
  }

  /* section 1.8.3 */
  if(pBinFileValues->fieldsUsed1_8 & CRTL_UNSOLICITEDRESPONSECONFIRMATIONTIMEOUT1_8_3)
    pSesnConfig->unsolConfirmTimeout = pBinFileValues->unsolRespConfirmTimeout1_8;


  /* section 1.8.4 */
  if(pBinFileValues->fieldsUsed1_8 & CRTL_MAXUNSOLICITEDRETRIES1_8_4)
  {
    if(pBinFileValues->specialControls1_8 & SPCTRL_NONEMAXUNSOLICITEDRETRIES1_8_4)
      pSesnConfig->unsolMaxRetries = 0;
    else if(pBinFileValues->specialControls1_8 & SPCTRL_INFINITEMAXUNSOLICITEDRETRIES1_8_4)
    {
      pSesnConfig->unsolMaxRetries = TMWDEFS_USHORT_MAX;
      pSesnConfig->unsolOfflineRetryDelay = pSesnConfig->unsolRetryDelay;
    }
    else
      pSesnConfig->unsolMaxRetries = (TMWTYPES_USHORT)pBinFileValues->maxUnsolicitedRetries1_8;
  }


  /* section 1.9.1 */
  if(pBinFileValues->fieldsUsed1_9 & CRTL_NUMBEROFCLASSONEEVENTS1_9_1)
  {
    if(pBinFileValues->specialControls1_9 & SPCTRL_CLASSONENOTUSEDTOTRIGGERUNSOLRESPS1_9_1_AND_1_9_5)
      pSesnConfig->unsolClassMask &= ~((TMWDEFS_CLASS_MASK)TMWDEFS_CLASS_MASK_ONE);
    else
    {
      pSesnConfig->unsolClassMask |= (TMWDEFS_CLASS_MASK)TMWDEFS_CLASS_MASK_ONE;
      pSesnConfig->unsolClass1MaxEvents = (TMWTYPES_UCHAR)pBinFileValues->numClassOneEvents1_9;
    }
  }

  /* section 1.9.2 */
  if(pBinFileValues->fieldsUsed1_9 & CRTL_NUMBEROFCLASSTWOEVENTS1_9_2)
  {
    if(pBinFileValues->specialControls1_9 & SPCTRL_CLASSTWONOTUSEDTOTRIGGERUNSOLRESPS1_9_2_AND_1_9_6)
      pSesnConfig->unsolClassMask &= ~((TMWDEFS_CLASS_MASK)TMWDEFS_CLASS_MASK_TWO);
    else
    {
      pSesnConfig->unsolClassMask |= (TMWDEFS_CLASS_MASK)TMWDEFS_CLASS_MASK_TWO;
      pSesnConfig->unsolClass2MaxEvents = (TMWTYPES_UCHAR)pBinFileValues->numClassTwoEvents1_9;
    }
  }

  /* section 1.9.3 */
  if(pBinFileValues->fieldsUsed1_9 & CRTL_NUMBEROFCLASSTHREEEVENTS1_9_3)
  {
    if(pBinFileValues->specialControls1_9 & SPCTRL_CLASSTHREENOTUSEDTOTRIGGERUNSOLRESPS1_9_3_AND_1_9_7)
      pSesnConfig->unsolClassMask &= ~((TMWDEFS_CLASS_MASK)TMWDEFS_CLASS_MASK_THREE);
    else
    {
      pSesnConfig->unsolClassMask |= (TMWDEFS_CLASS_MASK)TMWDEFS_CLASS_MASK_THREE;
      pSesnConfig->unsolClass3MaxEvents = (TMWTYPES_UCHAR)pBinFileValues->numClassThreeEvents1_9;
    }
  }

  /* section 1.9.5 */
  if(pBinFileValues->fieldsUsed1_9 & CRTL_HOLDTIMEAFTERCLASSONEEVENT1_9_5)
    if(!(pBinFileValues->specialControls1_9 & SPCTRL_CLASSONENOTUSEDTOTRIGGERUNSOLRESPS1_9_1_AND_1_9_5))
      pSesnConfig->unsolClass1MaxDelay = pBinFileValues->holdTimeAfterClassOneEvent1_9;

  /* section 1.9.6 */
  if(pBinFileValues->fieldsUsed1_9 & CRTL_HOLDTIMEAFTERCLASSTWOEVENT1_9_6)
    if(!(pBinFileValues->specialControls1_9 & SPCTRL_CLASSTWONOTUSEDTOTRIGGERUNSOLRESPS1_9_2_AND_1_9_6))
      pSesnConfig->unsolClass2MaxDelay = pBinFileValues->holdTimeAfterClassTwoEvent1_9;

  /* section 1.9.7 */
  if(pBinFileValues->fieldsUsed1_9 & CRTL_HOLDTIMEAFTERCLASSTHREEEVENT1_9_7)
    if(!(pBinFileValues->specialControls1_9 & SPCTRL_CLASSTHREENOTUSEDTOTRIGGERUNSOLRESPS1_9_3_AND_1_9_7))
      pSesnConfig->unsolClass3MaxDelay = pBinFileValues->holdTimeAfterClassThreeEvent1_9;

  /* section 1.10.2 */
  if(pBinFileValues->fieldsUsed1_10 & CRTL_OUTSTATIONSETSIIN14_1_10_2)
    if(pBinFileValues->specialControls1_10 & SPCTRL_AFTERLASTTIMESYNCOUTSTATIONSETSIIN1_10_2)
      pSesnConfig->clockValidPeriod = pBinFileValues->afterLastTimeSyncOutstationSetsIIN14_1_10;
    else if(pBinFileValues->specialControls1_10 & SPCTRL_PERIODICALLYOUTSTATIONSETSIIN1_10_2)
      pSesnConfig->clockValidPeriod = pBinFileValues->outStationSetsIIN14_1_10;

#if SDNPDATA_SUPPORT_OBJ120
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

  /* section 1.12.3 */
  if(pBinFileValues->fieldsUsed1_12 & CRTL_SECURITYRESPONSETIMEOUT1_12_3)
    pSesnConfig->authConfig.replyTimeout = pBinFileValues->securityResponseTimeout1_12;


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
#if SDNPCNFG_SUPPORT_SA_VERSION2
  if(pBinFileValues->fieldsUsed1_12 & CRTL_MAXERRORCOUNT1_12_8)
    pSesnConfig->authConfig.maxErrorCount  = (TMWTYPES_UCHAR)pBinFileValues->maxErrorCount1_12;
#endif
#if SDNPCNFG_SUPPORT_SA_VERSION5
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

  /* section 1.12.10 */
  /*if(pBinFileValues->fieldsUsed1_12 & CRTL_KEYWRAPALGORITHM1_12_10)
  {
    if(pBinFileValues->keyWrapAlgorithm1_12 & KEYWRAPALGORITHMS_AES128)
      pSesnConfig->authConfig.keyWrapAlgorithm = DNPAUTH_KEYWRAP_AES128;
    else if(pBinFileValues->keyWrapAlgorithm1_12 & KEYWRAPALGORITHMS_AES256)
      pSesnConfig->authConfig.keyWrapAlgorithm = DNPAUTH_KEYWRAP_AES256;
  }*/

  

  /* section 1.12.15 */
  if(pBinFileValues->fieldsUsed1_12 & CRTL_ADDITIONALCRITICALFCS1_12_15)
  {
    /* not currently implemented */
  }


  /* section 1.12.17 */
  if (xmlVersionIndex >= 2)  /* Nov2013 or newer only */
  {
    if(pBinFileValues->fieldsUsed1_12 & CRTL_REMOTEUPDATEKEYCHANGE1_12_17)
    {
      /* not currently implemented */
    }
  }

  /* section 1.13 */
  if(pBinFileValues->fieldsUsed1_13 & CRTL_BROADCASTFUNCTIONALITYSUPPORTED)
  {
    if(pBinFileValues->specialControls1_13 & SPCRTL_ENABLEDBROADCASTFUNCT)
    {
      sdnpsesn_SetBroadcastFunctionCodes1_13(pBinFileValues, &(pSesnConfig->enabledBroadcastFCs), &(pSesnConfig->enabledBroadcastWrites));
    }
      
    else
    {
      pSesnConfig->enabledBroadcastFCs = 0x0;
      pSesnConfig->enabledBroadcastWrites = 0x0;
    }
  }

#endif

  return TMWDEFS_TRUE;

}

void TMWDEFS_GLOBAL sdnpsesn_SetBroadcastFunctionCodes1_13(
  DNPBNCFG_FILEVALUES *pBinFileValues,
  TMWTYPES_ULONG *enabledBroadcastFCs,
  TMWTYPES_UCHAR *enabledBroadcastWrites)
{
  if(pBinFileValues->fieldsUsed1_13 & CRTL_BROADCASTFUNCTIONALITYFC2SUPPORTED)
  {
    *enabledBroadcastFCs |=  SDNPSESN_ENABLE_FC_WRITE;

    if(pBinFileValues->specialControls1_13 & SPCRTL_ENABLEDBROADCASTFUNCTFC2WRITECLOCK)
      *enabledBroadcastWrites |=  SDNPSESN_ENABLE_WRITE_CLOCK;
    else
      *enabledBroadcastWrites &= ~SDNPSESN_ENABLE_WRITE_CLOCK;

    if(pBinFileValues->specialControls1_13 & SPCRTL_ENABLEDBROADCASTFUNCTFC2WRITELASTRECTIME)
      *enabledBroadcastWrites |=  SDNPSESN_ENABLE_WRITE_LRTIME;
    else
      *enabledBroadcastWrites &= ~SDNPSESN_ENABLE_WRITE_LRTIME;

    if(pBinFileValues->specialControls1_13 & SPCRTL_ENABLEDBROADCASTFUNCTFC2CLEARRESTART)
      *enabledBroadcastWrites |=  SDNPSESN_ENABLE_WRITE_CRESTART;
    else
      *enabledBroadcastWrites &= ~SDNPSESN_ENABLE_WRITE_CRESTART;

    if(pBinFileValues->specialControls1_13 & SPCRTL_ENABLEDBROADCASTFUNCTFC2WRITEOTHER)
      *enabledBroadcastWrites |=  SDNPSESN_ENABLE_WRITE_OTHER;
    else
      *enabledBroadcastWrites &= ~SDNPSESN_ENABLE_WRITE_OTHER;
  }
  else
    *enabledBroadcastFCs &= ~SDNPSESN_ENABLE_FC_WRITE;

  if(pBinFileValues->fieldsUsed1_13 & CRTL_BROADCASTFUNCTIONALITYFC5SUPPORTED)
  {
    if(pBinFileValues->specialControls1_13 & SPCRTL_ENABLEDBROADCASTFUNCTFC5)
      *enabledBroadcastFCs |=  SDNPSESN_ENABLE_FC_OPERATE;
    else
      *enabledBroadcastFCs &= ~SDNPSESN_ENABLE_FC_OPERATE;
  }

  if(pBinFileValues->fieldsUsed1_13 & CRTL_BROADCASTFUNCTIONALITYFC6SUPPORTED)
  {
    if(pBinFileValues->specialControls1_13 & SPCRTL_ENABLEDBROADCASTFUNCTFC6)
      *enabledBroadcastFCs |=  SDNPSESN_ENABLE_FC_DIRECT_OP_NOACK;
    else
      *enabledBroadcastFCs &= ~SDNPSESN_ENABLE_FC_DIRECT_OP_NOACK;
  }

  if(pBinFileValues->fieldsUsed1_13 & CRTL_BROADCASTFUNCTIONALITYFC7SUPPORTED)
  {
    if(pBinFileValues->specialControls1_13 & SPCRTL_ENABLEDBROADCASTFUNCTFC7)
      *enabledBroadcastFCs |=  SDNPSESN_ENABLE_FC_FRZ;
    else
      *enabledBroadcastFCs &= ~SDNPSESN_ENABLE_FC_FRZ;
  }

  if(pBinFileValues->fieldsUsed1_13 & CRTL_BROADCASTFUNCTIONALITYFC8SUPPORTED)
  {
    if(pBinFileValues->specialControls1_13 & SPCRTL_ENABLEDBROADCASTFUNCTFC8)
      *enabledBroadcastFCs |=  SDNPSESN_ENABLE_FC_FRZ_NOACK;
    else
      *enabledBroadcastFCs &= ~SDNPSESN_ENABLE_FC_FRZ_NOACK;
  }

  if(pBinFileValues->fieldsUsed1_13 & CRTL_BROADCASTFUNCTIONALITYFC9SUPPORTED)
  {
    if(pBinFileValues->specialControls1_13 & SPCRTL_ENABLEDBROADCASTFUNCTFC9)
      *enabledBroadcastFCs |=  SDNPSESN_ENABLE_FC_FRZ_CLEAR;
    else
      *enabledBroadcastFCs &= ~SDNPSESN_ENABLE_FC_FRZ_CLEAR;
  }

  if(pBinFileValues->fieldsUsed1_13 & CRTL_BROADCASTFUNCTIONALITYFC10SUPPORTED)
  {
    if(pBinFileValues->specialControls1_13 & SPCRTL_ENABLEDBROADCASTFUNCTFC10)
      *enabledBroadcastFCs |=  SDNPSESN_ENABLE_FC_FRZ_CLEAR_NOACK;
    else
      *enabledBroadcastFCs &= ~SDNPSESN_ENABLE_FC_FRZ_CLEAR_NOACK;
  }

  if(pBinFileValues->fieldsUsed1_13 & CRTL_BROADCASTFUNCTIONALITYFC11SUPPORTED)
  {
    if(pBinFileValues->specialControls1_13 & SPCRTL_ENABLEDBROADCASTFUNCTFC11)
    *enabledBroadcastFCs |=  SDNPSESN_ENABLE_FC_FRZ_TIME;
  else
    *enabledBroadcastFCs &= ~SDNPSESN_ENABLE_FC_FRZ_TIME;
  }

  if(pBinFileValues->fieldsUsed1_13 & CRTL_BROADCASTFUNCTIONALITYFC12SUPPORTED)
  {
    if(pBinFileValues->specialControls1_13 & SPCRTL_ENABLEDBROADCASTFUNCTFC12)
      *enabledBroadcastFCs |=  SDNPSESN_ENABLE_FC_FRZ_TIME_NOACK;
    else
      *enabledBroadcastFCs &= ~SDNPSESN_ENABLE_FC_FRZ_TIME_NOACK;
  }

  if(pBinFileValues->fieldsUsed1_13 & CRTL_BROADCASTFUNCTIONALITYFC13SUPPORTED)
  {
    if(pBinFileValues->specialControls1_13 & SPCRTL_ENABLEDBROADCASTFUNCTFC13)
      *enabledBroadcastFCs |=  SDNPSESN_ENABLE_FC_COLD_RESTART;
    else
      *enabledBroadcastFCs &= ~SDNPSESN_ENABLE_FC_COLD_RESTART;
  }

  if(pBinFileValues->fieldsUsed1_13 & CRTL_BROADCASTFUNCTIONALITYFC14SUPPORTED)
  {
    if(pBinFileValues->specialControls1_13 & SPCRTL_ENABLEDBROADCASTFUNCTFC14)
      *enabledBroadcastFCs |=  SDNPSESN_ENABLE_FC_WARM_RESTART;
    else
      *enabledBroadcastFCs &= ~SDNPSESN_ENABLE_FC_WARM_RESTART;
  }

  if(pBinFileValues->fieldsUsed1_13 & CRTL_BROADCASTFUNCTIONALITYFC15SUPPORTED)
  {
    if(pBinFileValues->specialControls1_13 & SPCRTL_ENABLEDBROADCASTFUNCTFC15)
      *enabledBroadcastFCs |=  SDNPSESN_ENABLE_FC_INIT_DATA;
    else
      *enabledBroadcastFCs &= ~SDNPSESN_ENABLE_FC_INIT_DATA;
  }

  if(pBinFileValues->fieldsUsed1_13 & CRTL_BROADCASTFUNCTIONALITYFC16SUPPORTED)
  {
    if(pBinFileValues->specialControls1_13 & SPCRTL_ENABLEDBROADCASTFUNCTFC16)
      *enabledBroadcastFCs |=  SDNPSESN_ENABLE_FC_INIT_APP;
    else
      *enabledBroadcastFCs &= ~SDNPSESN_ENABLE_FC_INIT_APP;
  }

  if(pBinFileValues->fieldsUsed1_13 & CRTL_BROADCASTFUNCTIONALITYFC17SUPPORTED)
  {
    if(pBinFileValues->specialControls1_13 & SPCRTL_ENABLEDBROADCASTFUNCTFC17)
      *enabledBroadcastFCs |=  SDNPSESN_ENABLE_FC_START_APP;
    else
      *enabledBroadcastFCs &= ~SDNPSESN_ENABLE_FC_START_APP;
  }

  if(pBinFileValues->fieldsUsed1_13 & CRTL_BROADCASTFUNCTIONALITYFC18SUPPORTED)
  {
    if(pBinFileValues->specialControls1_13 & SPCRTL_ENABLEDBROADCASTFUNCTFC18)
      *enabledBroadcastFCs |=  SDNPSESN_ENABLE_FC_STOP_APP;
    else
      *enabledBroadcastFCs &= ~SDNPSESN_ENABLE_FC_STOP_APP;
  }

  if(pBinFileValues->fieldsUsed1_13 & CRTL_BROADCASTFUNCTIONALITYFC19SUPPORTED)
  {
    if(pBinFileValues->specialControls1_13 & SPCRTL_ENABLEDBROADCASTFUNCTFC19)
      *enabledBroadcastFCs |=  SDNPSESN_ENABLE_FC_SAVE_CONFIG;
    else
      *enabledBroadcastFCs &= ~SDNPSESN_ENABLE_FC_SAVE_CONFIG;
  }

  if(pBinFileValues->fieldsUsed1_13 & CRTL_BROADCASTFUNCTIONALITYFC20SUPPORTED)
  {
    if(pBinFileValues->specialControls1_13 & SPCRTL_ENABLEDBROADCASTFUNCTFC20ENABLEUNSOLBYEVENTCLASS)
      *enabledBroadcastFCs |=  SDNPSESN_ENABLE_FC_ENABLE_UNSOL;
    else
      *enabledBroadcastFCs &= ~SDNPSESN_ENABLE_FC_ENABLE_UNSOL;
  }

  if(pBinFileValues->fieldsUsed1_13 & CRTL_BROADCASTFUNCTIONALITYFC21SUPPORTED)
  {
    if(pBinFileValues->specialControls1_13 & SPCRTL_ENABLEDBROADCASTFUNCTFC21DISABLEUNSOLBYEVENTCLASS)
      *enabledBroadcastFCs |=  SDNPSESN_ENABLE_FC_DISABLE_UNSOL;
    else
      *enabledBroadcastFCs &= ~SDNPSESN_ENABLE_FC_DISABLE_UNSOL;
  }

  if(pBinFileValues->fieldsUsed1_13 & CRTL_BROADCASTFUNCTIONALITYFC22SUPPORTED)
  {
    if(pBinFileValues->specialControls1_13 & SPCRTL_ENABLEDBROADCASTFUNCTFC22)
      *enabledBroadcastFCs |=  SDNPSESN_ENABLE_FC_ASSIGN_CLASS;
    else
      *enabledBroadcastFCs &= ~SDNPSESN_ENABLE_FC_ASSIGN_CLASS;
  }

  if(pBinFileValues->fieldsUsed1_13 & CRTL_BROADCASTFUNCTIONALITYFC24SUPPORTED)
  {
    if(pBinFileValues->specialControls1_13 & SPCRTL_ENABLEDBROADCASTFUNCTFC24)
      *enabledBroadcastFCs |=  SDNPSESN_ENABLE_FC_RECORD_CTIME;
    else
      *enabledBroadcastFCs &= ~SDNPSESN_ENABLE_FC_RECORD_CTIME;
  }

  if(pBinFileValues->fieldsUsed1_13 & CRTL_BROADCASTFUNCTIONALITYFC31SUPPORTED)
  {
    if(pBinFileValues->specialControls1_13 & SPCRTL_ENABLEDBROADCASTFUNCTFC31)
      *enabledBroadcastFCs |=  SDNPSESN_ENABLE_FC_ACTIVATE_CONFIG;
    else
      *enabledBroadcastFCs &= ~SDNPSESN_ENABLE_FC_ACTIVATE_CONFIG;
  }
}
#endif


#if DNPCNFG_SUPPORT_AUTHENTICATION
#if SDNPCNFG_SUPPORT_SA_VERSION5
TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsesn_addAuthUser( 
  TMWSESN *pSession,
  TMWTYPES_USHORT userNumber)
{
  SDNPSESN *pSDNPSession = (SDNPSESN*)pSession;
  TMWTYPES_BOOL status = TMWDEFS_FALSE;

  TMWTARG_LOCK_SECTION(&pSession->pChannel->lock);
  if((pSDNPSession->pAuthenticationInfo != TMWDEFS_NULL) && (pSDNPSession->dnp.operateInV2Mode == TMWDEFS_FALSE))
  {
    status = sdnpsa_addUser((SDNPAUTH_INFO*)pSDNPSession->pAuthenticationInfo, userNumber);
  } 
    
  TMWTARG_UNLOCK_SECTION(&pSession->pChannel->lock);
  return status;
}
 
TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpsesn_getAuthUser( 
  TMWSESN *pSession,
  TMWTYPES_USHORT index)
{
  SDNPSESN *pSDNPSession = (SDNPSESN*)pSession;
  if((pSDNPSession->pAuthenticationInfo != TMWDEFS_NULL) && (pSDNPSession->dnp.operateInV2Mode == TMWDEFS_FALSE))
    return sdnpsa_getUser((SDNPAUTH_INFO*)pSDNPSession->pAuthenticationInfo, index);
  else
    return 0;
}

TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpsesn_removeAuthUser( 
  TMWSESN *pSession,
  TMWTYPES_USHORT userNumber)
{
  SDNPSESN *pSDNPSession = (SDNPSESN*)pSession;
  if((pSDNPSession->pAuthenticationInfo != TMWDEFS_NULL) && (pSDNPSession->dnp.operateInV2Mode == TMWDEFS_FALSE))
    return sdnpsa_removeUser((SDNPAUTH_INFO*)pSDNPSession->pAuthenticationInfo, userNumber);
  else
    return TMWDEFS_FALSE;
}
#endif

void TMWDEFS_GLOBAL sdnpsesn_authSendAggrResp( 
  TMWSESN *pSession,
  TMWTYPES_BOOL sendAggressive)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  pSDNPSession->authSendAggrResp = sendAggressive; 
}

void TMWDEFS_GLOBAL sdnpsesn_authSendAggrUnsol( 
  TMWSESN *pSession,
  TMWTYPES_BOOL sendAggressive)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  pSDNPSession->authSendAggrUnsol = sendAggressive; 
}

void TMWDEFS_GLOBAL sdnpsesn_authPreChallApplConf( 
  TMWSESN *pSession,
  TMWTYPES_BOOL preChallenge)
{
  SDNPSESN *pSDNPSession = (SDNPSESN *)pSession;
  pSDNPSession->preChallengeApplConf = preChallenge;
}
#endif

void TMWDEFS_GLOBAL sdnpsesn_restartClockValidTime(TMWSESN *pSession)
{
#if SDNPDATA_SUPPORT_OBJ50
  sdnpo050_restartClockValidTime(pSession);
#else
  TMWTARG_UNUSED_PARAM(pSession);
#endif
}
