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

/* file: mdnpdata.h
 * description: This file defines the interface between the Triangle 
 *  MicroWorks, Inc. DNP master source code library and the target database.
 */
#ifndef MDNPDATA_DEFINED
#define MDNPDATA_DEFINED

#include "tmwscl/utils/tmwdefs.h"
#include "tmwscl/utils/tmwdtime.h"
#include "tmwscl/utils/tmwcrypto.h"
#include "tmwscl/dnp/dnpcnfg.h"
#include "tmwscl/dnp/mdnpcnfg.h"
#include "tmwscl/dnp/dnputil.h"
#include "tmwscl/dnp/dnpdata.h"
#include "tmwscl/dnp/dnpauth.h"

/*
 * If you are a.NET Components customer there is no longer a separate
 * config / tmwprvt.h file included from tmwcnfg.h which added functionality
 * not compiled in by default for the ANSI C SCL.This simplifies choosing the
 * functionality a device should support.It is important to select the exact
 * set of features desired, test them thoroughly, and then document them in
 * your Device Profile.
 */

/* This define can be used to restore some functionality that
 * is now compiled out by default in the SCL starting in version 3.29.0.
 * ActivateConfig, Octet String, Virtual Terminal.
 */
#ifndef MDNPDATA_CNFG_LEVEL_TMW
#define MDNPDATA_CNFG_LEVEL_TMW  TMWDEFS_FALSE
#endif

/* Device Attributes */
#ifndef MDNPDATA_SUPPORT_OBJ0
#define MDNPDATA_SUPPORT_OBJ0    TMWDEFS_FALSE
#endif

/* Binary Inputs */
#define MDNPDATA_SUPPORT_OBJ1_V1 TMWDEFS_TRUE
#define MDNPDATA_SUPPORT_OBJ1_V2 TMWDEFS_TRUE
#define MDNPDATA_SUPPORT_OBJ1 \
  (MDNPDATA_SUPPORT_OBJ1_V1 | \
   MDNPDATA_SUPPORT_OBJ1_V2)

/* Binary Input Events */
#define MDNPDATA_SUPPORT_OBJ2_V1 TMWDEFS_TRUE
#define MDNPDATA_SUPPORT_OBJ2_V2 TMWDEFS_TRUE
#define MDNPDATA_SUPPORT_OBJ2_V3 TMWDEFS_TRUE
#define MDNPDATA_SUPPORT_OBJ2 \
  (MDNPDATA_SUPPORT_OBJ2_V1 | \
   MDNPDATA_SUPPORT_OBJ2_V2 | \
   MDNPDATA_SUPPORT_OBJ2_V3)

/* Double Bit Inputs */
#define MDNPDATA_SUPPORT_OBJ3_V1 TMWDEFS_TRUE
#define MDNPDATA_SUPPORT_OBJ3_V2 TMWDEFS_TRUE
#define MDNPDATA_SUPPORT_OBJ3 \
  (MDNPDATA_SUPPORT_OBJ3_V1 | \
   MDNPDATA_SUPPORT_OBJ3_V2)

/* Double Bit Input Events */
#define MDNPDATA_SUPPORT_OBJ4_V1 TMWDEFS_TRUE
#define MDNPDATA_SUPPORT_OBJ4_V2 TMWDEFS_TRUE
#define MDNPDATA_SUPPORT_OBJ4_V3 TMWDEFS_TRUE
#define MDNPDATA_SUPPORT_OBJ4 \
  (MDNPDATA_SUPPORT_OBJ4_V1 | \
   MDNPDATA_SUPPORT_OBJ4_V2 | \
   MDNPDATA_SUPPORT_OBJ4_V3)

/* Binary Output Status */
#define MDNPDATA_SUPPORT_OBJ10_V1 TMWDEFS_TRUE
#define MDNPDATA_SUPPORT_OBJ10_V2 TMWDEFS_TRUE
#define MDNPDATA_SUPPORT_OBJ10 \
  (MDNPDATA_SUPPORT_OBJ10_V1 | \
   MDNPDATA_SUPPORT_OBJ10_V2)

/* If Binary Output Event support is not defined on the command line 
 * set the configuration here 
 */
#ifndef MDNPDATA_SUPPORT_OBJ11
/* Binary Output Events */
#define MDNPDATA_SUPPORT_OBJ11_V1 TMWDEFS_TRUE
#define MDNPDATA_SUPPORT_OBJ11_V2 TMWDEFS_TRUE
#define MDNPDATA_SUPPORT_OBJ11 \
  (MDNPDATA_SUPPORT_OBJ11_V1 | \
   MDNPDATA_SUPPORT_OBJ11_V2)
#endif

/* Control Relay Output Block */
#define MDNPDATA_SUPPORT_OBJ12_V1 TMWDEFS_TRUE
#define MDNPDATA_SUPPORT_OBJ12_V2 TMWDEFS_TRUE
#define MDNPDATA_SUPPORT_OBJ12_V3 TMWDEFS_TRUE
#define MDNPDATA_SUPPORT_OBJ12 \
  (MDNPDATA_SUPPORT_OBJ12_V1 | \
   MDNPDATA_SUPPORT_OBJ12_V2 | \
   MDNPDATA_SUPPORT_OBJ12_V3)

/* Allow mdnpbrm_binaryCommand() to send CROB count value other than 1 
 * When this function was first released it did not allow the caller to 
 * specify count. In order to prevent unitialized values from being used
 * by these early implementations, this define must be changed to TMWDEFS_TRUE
 * and count in MDNPBRM_CROB_INFO set to the desired value.
 */
#ifndef MDNPDATA_SUPPORT_OBJ12_COUNT
#define MDNPDATA_SUPPORT_OBJ12_COUNT TMWDEFS_TRUE
#endif

/* If Binary Output Command Event support is not defined on the command line 
 * set the configuration here 
 */
#ifndef MDNPDATA_SUPPORT_OBJ13
/* Binary Output Command Events */
#define MDNPDATA_SUPPORT_OBJ13_V1 TMWDEFS_TRUE
#define MDNPDATA_SUPPORT_OBJ13_V2 TMWDEFS_TRUE
#define MDNPDATA_SUPPORT_OBJ13 \
  (MDNPDATA_SUPPORT_OBJ13_V1 | \
   MDNPDATA_SUPPORT_OBJ13_V2)
#endif

/* Binary Counters */
#define MDNPDATA_SUPPORT_OBJ20_V1 TMWDEFS_TRUE
#define MDNPDATA_SUPPORT_OBJ20_V2 TMWDEFS_TRUE
#define MDNPDATA_SUPPORT_OBJ20_V5 TMWDEFS_TRUE
#define MDNPDATA_SUPPORT_OBJ20_V6 TMWDEFS_TRUE
#define MDNPDATA_SUPPORT_OBJ20 \
  (MDNPDATA_SUPPORT_OBJ20_V1 | \
   MDNPDATA_SUPPORT_OBJ20_V2 | \
   MDNPDATA_SUPPORT_OBJ20_V5 | \
   MDNPDATA_SUPPORT_OBJ20_V6)

/* Frozen Counters */
#define MDNPDATA_SUPPORT_OBJ21_V1 TMWDEFS_TRUE
#define MDNPDATA_SUPPORT_OBJ21_V2 TMWDEFS_TRUE
#define MDNPDATA_SUPPORT_OBJ21_V5 TMWDEFS_TRUE
#define MDNPDATA_SUPPORT_OBJ21_V6 TMWDEFS_TRUE
#define MDNPDATA_SUPPORT_OBJ21_V9 TMWDEFS_TRUE
#define MDNPDATA_SUPPORT_OBJ21_V10 TMWDEFS_TRUE
#define MDNPDATA_SUPPORT_OBJ21 \
  (MDNPDATA_SUPPORT_OBJ21_V1 | \
   MDNPDATA_SUPPORT_OBJ21_V2 | \
   MDNPDATA_SUPPORT_OBJ21_V5 | \
   MDNPDATA_SUPPORT_OBJ21_V6 | \
   MDNPDATA_SUPPORT_OBJ21_V9 | \
   MDNPDATA_SUPPORT_OBJ21_V10)

/* Binary Counter Events */
#define MDNPDATA_SUPPORT_OBJ22_V1 TMWDEFS_TRUE
#define MDNPDATA_SUPPORT_OBJ22_V2 TMWDEFS_TRUE
#define MDNPDATA_SUPPORT_OBJ22_V5 TMWDEFS_TRUE
#define MDNPDATA_SUPPORT_OBJ22_V6 TMWDEFS_TRUE
#define MDNPDATA_SUPPORT_OBJ22 \
  (MDNPDATA_SUPPORT_OBJ22_V1 | \
   MDNPDATA_SUPPORT_OBJ22_V2 | \
   MDNPDATA_SUPPORT_OBJ22_V5 | \
   MDNPDATA_SUPPORT_OBJ22_V6)

/* Frozen Counter Events */
#define MDNPDATA_SUPPORT_OBJ23_V1 TMWDEFS_TRUE
#define MDNPDATA_SUPPORT_OBJ23_V2 TMWDEFS_TRUE
#define MDNPDATA_SUPPORT_OBJ23_V5 TMWDEFS_TRUE
#define MDNPDATA_SUPPORT_OBJ23_V6 TMWDEFS_TRUE
#define MDNPDATA_SUPPORT_OBJ23 \
  (MDNPDATA_SUPPORT_OBJ23_V1 | \
   MDNPDATA_SUPPORT_OBJ23_V2 | \
   MDNPDATA_SUPPORT_OBJ23_V5 | \
   MDNPDATA_SUPPORT_OBJ23_V6)

/* Analog Inputs */
#define MDNPDATA_SUPPORT_OBJ30_V1 TMWDEFS_TRUE
#define MDNPDATA_SUPPORT_OBJ30_V2 TMWDEFS_TRUE
#define MDNPDATA_SUPPORT_OBJ30_V3 TMWDEFS_TRUE
#define MDNPDATA_SUPPORT_OBJ30_V4 TMWDEFS_TRUE
#define MDNPDATA_SUPPORT_OBJ30_V5 TMWCNFG_SUPPORT_FLOAT
#define MDNPDATA_SUPPORT_OBJ30_V6 TMWCNFG_SUPPORT_DOUBLE
#define MDNPDATA_SUPPORT_OBJ30 \
  (MDNPDATA_SUPPORT_OBJ30_V1 | \
   MDNPDATA_SUPPORT_OBJ30_V2 | \
   MDNPDATA_SUPPORT_OBJ30_V3 | \
   MDNPDATA_SUPPORT_OBJ30_V4 | \
   MDNPDATA_SUPPORT_OBJ30_V5 | \
   MDNPDATA_SUPPORT_OBJ30_V6)

/* Frozen Analog Inputs */
#ifndef MDNPDATA_SUPPORT_OBJ31
#define MDNPDATA_SUPPORT_OBJ31_V1 TMWDEFS_FALSE
#define MDNPDATA_SUPPORT_OBJ31_V2 TMWDEFS_FALSE
#define MDNPDATA_SUPPORT_OBJ31_V3 TMWDEFS_FALSE
#define MDNPDATA_SUPPORT_OBJ31_V4 TMWDEFS_FALSE
#define MDNPDATA_SUPPORT_OBJ31_V5 TMWDEFS_FALSE
#define MDNPDATA_SUPPORT_OBJ31_V6 TMWDEFS_FALSE
#define MDNPDATA_SUPPORT_OBJ31_V7 TMWDEFS_FALSE
#define MDNPDATA_SUPPORT_OBJ31_V8 TMWDEFS_FALSE
#define MDNPDATA_SUPPORT_OBJ31 \
  (MDNPDATA_SUPPORT_OBJ31_V1 | \
   MDNPDATA_SUPPORT_OBJ31_V2 | \
   MDNPDATA_SUPPORT_OBJ31_V3 | \
   MDNPDATA_SUPPORT_OBJ31_V4 | \
   MDNPDATA_SUPPORT_OBJ31_V5 | \
   MDNPDATA_SUPPORT_OBJ31_V6 | \
   MDNPDATA_SUPPORT_OBJ31_V7 | \
   MDNPDATA_SUPPORT_OBJ31_V8)
#endif

/* Analog Input Events */
#define MDNPDATA_SUPPORT_OBJ32_V1 TMWDEFS_TRUE
#define MDNPDATA_SUPPORT_OBJ32_V2 TMWDEFS_TRUE
#define MDNPDATA_SUPPORT_OBJ32_V3 TMWDEFS_TRUE
#define MDNPDATA_SUPPORT_OBJ32_V4 TMWDEFS_TRUE
#define MDNPDATA_SUPPORT_OBJ32_V5 TMWCNFG_SUPPORT_FLOAT
#define MDNPDATA_SUPPORT_OBJ32_V6 TMWCNFG_SUPPORT_DOUBLE
#define MDNPDATA_SUPPORT_OBJ32_V7 TMWCNFG_SUPPORT_FLOAT
#define MDNPDATA_SUPPORT_OBJ32_V8 TMWCNFG_SUPPORT_DOUBLE
#define MDNPDATA_SUPPORT_OBJ32 \
  (MDNPDATA_SUPPORT_OBJ32_V1 | \
   MDNPDATA_SUPPORT_OBJ32_V2 | \
   MDNPDATA_SUPPORT_OBJ32_V3 | \
   MDNPDATA_SUPPORT_OBJ32_V4 | \
   MDNPDATA_SUPPORT_OBJ32_V5 | \
   MDNPDATA_SUPPORT_OBJ32_V6 | \
   MDNPDATA_SUPPORT_OBJ32_V7 | \
   MDNPDATA_SUPPORT_OBJ32_V8)

/* Frozen Analog Input Events */
#ifndef MDNPDATA_SUPPORT_OBJ33
#define MDNPDATA_SUPPORT_OBJ33_V1 TMWDEFS_FALSE
#define MDNPDATA_SUPPORT_OBJ33_V2 TMWDEFS_FALSE
#define MDNPDATA_SUPPORT_OBJ33_V3 TMWDEFS_FALSE
#define MDNPDATA_SUPPORT_OBJ33_V4 TMWDEFS_FALSE
#define MDNPDATA_SUPPORT_OBJ33_V5 TMWDEFS_FALSE
#define MDNPDATA_SUPPORT_OBJ33_V6 TMWDEFS_FALSE
#define MDNPDATA_SUPPORT_OBJ33_V7 TMWDEFS_FALSE
#define MDNPDATA_SUPPORT_OBJ33_V8 TMWDEFS_FALSE
#define MDNPDATA_SUPPORT_OBJ33 \
  (MDNPDATA_SUPPORT_OBJ33_V1 | \
   MDNPDATA_SUPPORT_OBJ33_V2 | \
   MDNPDATA_SUPPORT_OBJ33_V3 | \
   MDNPDATA_SUPPORT_OBJ33_V4 | \
   MDNPDATA_SUPPORT_OBJ33_V5 | \
   MDNPDATA_SUPPORT_OBJ33_V6 | \
   MDNPDATA_SUPPORT_OBJ33_V7 | \
   MDNPDATA_SUPPORT_OBJ33_V8)
#endif

/* Analog Input Deadbands */
#define MDNPDATA_SUPPORT_OBJ34_V1 TMWDEFS_TRUE
#define MDNPDATA_SUPPORT_OBJ34_V2 TMWDEFS_TRUE
#define MDNPDATA_SUPPORT_OBJ34_V3 TMWCNFG_SUPPORT_FLOAT
#define MDNPDATA_SUPPORT_OBJ34 \
  (MDNPDATA_SUPPORT_OBJ34_V1 | \
   MDNPDATA_SUPPORT_OBJ34_V2 | \
   MDNPDATA_SUPPORT_OBJ34_V3)

/* Analog Output Status */
#define MDNPDATA_SUPPORT_OBJ40_V1 TMWDEFS_TRUE
#define MDNPDATA_SUPPORT_OBJ40_V2 TMWDEFS_TRUE
#define MDNPDATA_SUPPORT_OBJ40_V3 TMWCNFG_SUPPORT_FLOAT
#define MDNPDATA_SUPPORT_OBJ40_V4 TMWCNFG_SUPPORT_DOUBLE
#define MDNPDATA_SUPPORT_OBJ40 \
  (MDNPDATA_SUPPORT_OBJ40_V1 | \
   MDNPDATA_SUPPORT_OBJ40_V2 | \
   MDNPDATA_SUPPORT_OBJ40_V3 | \
   MDNPDATA_SUPPORT_OBJ40_V4)

/* Analog Output Control Block */
#define MDNPDATA_SUPPORT_OBJ41_V1 TMWDEFS_TRUE
#define MDNPDATA_SUPPORT_OBJ41_V2 TMWDEFS_TRUE
#define MDNPDATA_SUPPORT_OBJ41_V3 TMWCNFG_SUPPORT_FLOAT
#define MDNPDATA_SUPPORT_OBJ41_V4 TMWCNFG_SUPPORT_DOUBLE
#define MDNPDATA_SUPPORT_OBJ41 \
  (MDNPDATA_SUPPORT_OBJ41_V1 | \
   MDNPDATA_SUPPORT_OBJ41_V2 | \
   MDNPDATA_SUPPORT_OBJ41_V3 | \
   MDNPDATA_SUPPORT_OBJ41_V4)

/* If Analog Output Event support is not defined on the command line 
 * set the configuration here 
 */
#ifndef MDNPDATA_SUPPORT_OBJ42
/* Analog Output Events */
#define MDNPDATA_SUPPORT_OBJ42_V1 TMWDEFS_TRUE
#define MDNPDATA_SUPPORT_OBJ42_V2 TMWDEFS_TRUE
#define MDNPDATA_SUPPORT_OBJ42_V3 TMWDEFS_TRUE
#define MDNPDATA_SUPPORT_OBJ42_V4 TMWDEFS_TRUE
#define MDNPDATA_SUPPORT_OBJ42_V5 TMWDEFS_TRUE
#define MDNPDATA_SUPPORT_OBJ42_V6 TMWDEFS_TRUE
#define MDNPDATA_SUPPORT_OBJ42_V7 TMWDEFS_TRUE
#define MDNPDATA_SUPPORT_OBJ42_V8 TMWDEFS_TRUE
#define MDNPDATA_SUPPORT_OBJ42 \
  (MDNPDATA_SUPPORT_OBJ42_V1 | \
   MDNPDATA_SUPPORT_OBJ42_V2 | \
   MDNPDATA_SUPPORT_OBJ42_V3 | \
   MDNPDATA_SUPPORT_OBJ42_V4 | \
   MDNPDATA_SUPPORT_OBJ42_V5 | \
   MDNPDATA_SUPPORT_OBJ42_V6 | \
   MDNPDATA_SUPPORT_OBJ42_V7 | \
   MDNPDATA_SUPPORT_OBJ42_V8)
#endif

/* If Analog Output Command Event support is not defined on the command line 
 * set the configuration here 
 */
#ifndef MDNPDATA_SUPPORT_OBJ43
/* Analog Output Command Events */
#define MDNPDATA_SUPPORT_OBJ43_V1 TMWDEFS_FALSE
#define MDNPDATA_SUPPORT_OBJ43_V2 TMWDEFS_FALSE
#define MDNPDATA_SUPPORT_OBJ43_V3 TMWDEFS_FALSE
#define MDNPDATA_SUPPORT_OBJ43_V4 TMWDEFS_FALSE
#define MDNPDATA_SUPPORT_OBJ43_V5 TMWDEFS_FALSE
#define MDNPDATA_SUPPORT_OBJ43_V6 TMWDEFS_FALSE
#define MDNPDATA_SUPPORT_OBJ43_V7 TMWDEFS_FALSE
#define MDNPDATA_SUPPORT_OBJ43_V8 TMWDEFS_FALSE
#define MDNPDATA_SUPPORT_OBJ43 \
  (MDNPDATA_SUPPORT_OBJ43_V1 | \
   MDNPDATA_SUPPORT_OBJ43_V2 | \
   MDNPDATA_SUPPORT_OBJ43_V3 | \
   MDNPDATA_SUPPORT_OBJ43_V4 | \
   MDNPDATA_SUPPORT_OBJ43_V5 | \
   MDNPDATA_SUPPORT_OBJ43_V6 | \
   MDNPDATA_SUPPORT_OBJ43_V7 | \
   MDNPDATA_SUPPORT_OBJ43_V8)
#endif

/* Time and Date */
#define MDNPDATA_SUPPORT_OBJ50_V1 TMWDEFS_TRUE
#define MDNPDATA_SUPPORT_OBJ50_V3 TMWDEFS_TRUE
#ifndef MDNPDATA_SUPPORT_OBJ50_V4
#define MDNPDATA_SUPPORT_OBJ50_V4 TMWDEFS_FALSE
#endif
#define MDNPDATA_SUPPORT_OBJ50 \
  (MDNPDATA_SUPPORT_OBJ50_V1 | \
   MDNPDATA_SUPPORT_OBJ50_V3 | \
   MDNPDATA_SUPPORT_OBJ50_V4)

/* File Transfer */
#ifndef MDNPDATA_SUPPORT_OBJ70
#define MDNPDATA_SUPPORT_OBJ70_V2 TMWDEFS_FALSE
#define MDNPDATA_SUPPORT_OBJ70_V3 TMWDEFS_FALSE
#define MDNPDATA_SUPPORT_OBJ70_V4 TMWDEFS_FALSE
#define MDNPDATA_SUPPORT_OBJ70_V5 TMWDEFS_FALSE
#define MDNPDATA_SUPPORT_OBJ70_V6 TMWDEFS_FALSE
#define MDNPDATA_SUPPORT_OBJ70_V7 TMWDEFS_FALSE
#define MDNPDATA_SUPPORT_OBJ70 \
  (MDNPDATA_SUPPORT_OBJ70_V2 | \
   MDNPDATA_SUPPORT_OBJ70_V3 | \
   MDNPDATA_SUPPORT_OBJ70_V4 | \
   MDNPDATA_SUPPORT_OBJ70_V5 | \
   MDNPDATA_SUPPORT_OBJ70_V6 | \
   MDNPDATA_SUPPORT_OBJ70_V7)
#endif

/* IIN bits */
#define MDNPDATA_SUPPORT_OBJ80    TMWDEFS_TRUE

/* If Data Set support is not defined on the command line 
 * set the configuration here 
 */
#ifndef MDNPDATA_SUPPORT_DATASETS

/* Data Set Prototypes */  
#define MDNPDATA_SUPPORT_OBJ85    TMWDEFS_FALSE

/* Data Set Descriptors */
#define MDNPDATA_SUPPORT_OBJ86_V1 TMWDEFS_FALSE
#define MDNPDATA_SUPPORT_OBJ86_V2 TMWDEFS_FALSE
#define MDNPDATA_SUPPORT_OBJ86_V3 TMWDEFS_FALSE
#define MDNPDATA_SUPPORT_OBJ86 \
  (MDNPDATA_SUPPORT_OBJ86_V1 | \
   MDNPDATA_SUPPORT_OBJ86_V2 | \
   MDNPDATA_SUPPORT_OBJ86_V3)

/* Data Set Present Value */
#define MDNPDATA_SUPPORT_OBJ87    TMWDEFS_FALSE

/* Data Set Snapshot Events */
#define MDNPDATA_SUPPORT_OBJ88    TMWDEFS_FALSE

/* Data Sets in general */
#define MDNPDATA_SUPPORT_DATASETS \
  (MDNPDATA_SUPPORT_OBJ85 | \
   MDNPDATA_SUPPORT_OBJ86 | \
   MDNPDATA_SUPPORT_OBJ87 | \
   MDNPDATA_SUPPORT_OBJ88)
#endif

/* Activate Configuration Response */
#define MDNPDATA_SUPPORT_OBJ91  MDNPDATA_CNFG_LEVEL_TMW

/* String Data */
#define MDNPDATA_SUPPORT_OBJ110 MDNPDATA_CNFG_LEVEL_TMW

/* String Events */
#define MDNPDATA_SUPPORT_OBJ111 MDNPDATA_CNFG_LEVEL_TMW

/* Virtual Terminal Output */
#define MDNPDATA_SUPPORT_OBJ112 MDNPDATA_CNFG_LEVEL_TMW

/* Virtual Terminal Events */
#define MDNPDATA_SUPPORT_OBJ113 MDNPDATA_CNFG_LEVEL_TMW

/* Extended String Data */
#ifndef MDNPDATA_SUPPORT_OBJ114
#define MDNPDATA_SUPPORT_OBJ114_V1 TMWDEFS_FALSE
#define MDNPDATA_SUPPORT_OBJ114_V2 TMWDEFS_FALSE
#define MDNPDATA_SUPPORT_OBJ114_V3 TMWDEFS_FALSE
#define MDNPDATA_SUPPORT_OBJ114_V4 TMWDEFS_FALSE
#define MDNPDATA_SUPPORT_OBJ114 \
  (MDNPDATA_SUPPORT_OBJ114_V1 | \
   MDNPDATA_SUPPORT_OBJ114_V2 | \
   MDNPDATA_SUPPORT_OBJ114_V3 | \
   MDNPDATA_SUPPORT_OBJ114_V4)
#endif

/* Extended String Events */
#ifndef MDNPDATA_SUPPORT_OBJ115
#define MDNPDATA_SUPPORT_OBJ115_V1 TMWDEFS_FALSE
#define MDNPDATA_SUPPORT_OBJ115_V2 TMWDEFS_FALSE
#define MDNPDATA_SUPPORT_OBJ115_V3 TMWDEFS_FALSE
#define MDNPDATA_SUPPORT_OBJ115_V4 TMWDEFS_FALSE
#define MDNPDATA_SUPPORT_OBJ115 \
  (MDNPDATA_SUPPORT_OBJ115_V1 | \
   MDNPDATA_SUPPORT_OBJ115_V2 | \
   MDNPDATA_SUPPORT_OBJ115_V3 | \
   MDNPDATA_SUPPORT_OBJ115_V4)
#endif

/* If Secure Authentication support is not defined on the command line 
 * set the configuration here. 
 * DNPCNFG_SUPPORT_AUTHENTICATION must also be defined appropriately
 * object group 121 and 122 are supported if object group 120 is supported
 */  
#ifndef MDNPDATA_SUPPORT_OBJ120
#define MDNPDATA_SUPPORT_OBJ120 DNPCNFG_SUPPORT_AUTHENTICATION
/* Secure Authentication Statistics Static and Event Objects are required if SA is supported */
#endif

/* Secure Authentication User Certificate for Asymmetric Remote Key Update */
#ifndef MDNPDATA_SUPPORT_OBJ120_V8
#define MDNPDATA_SUPPORT_OBJ120_V8 TMWDEFS_FALSE
#endif


/* If in Startup state, we should discard and not confirm 
 * unsolicited responses until an integrity poll completes.
 * App Layer Volume 2 Part 2 1.1.2 and Part 3 1.5.  
 * This means even initial unsolicited NULL responses should be discarded.
 * The previous behavior of the SCL was to accept and confirm them. To allow
 * NULL Unsoliciteds to be accepted even before the integrity poll completes 
 * set this define to TMWDEFS_FALSE.
 */
#define MDNPDATA_DISCARD_NULL_UNSOL TMWDEFS_FALSE


/* Secure Authentication Event that occurred */
typedef enum mdnpauth_event
{
  /* Rcvd IIN 2-2 Function Code Not Implemented in response to Read Status g120v4
   * indicating outstation does not support Secure Authentication.
   */
  MDNPAUTH_EV_V4_BAD_FUNCTION, 

  /* Rcvd IIN 2-2 Function Code Not Implemented in response to change user status g120v10
   * indicating outstation does not support Secure Authentication Remote Key Update.
   */
  MDNPAUTH_EV_V10_BAD_FUNCTION 
} MDNPAUTH_EVENT_ENUM;


#ifdef __cplusplus
extern "C" {
#endif

  /* function: mdnpdata_processIIN
   * purpose: Process Internal Indication Bits received from remote device.
   *  This routine gives the user the chance to process the IIN bits before
   *  the SCL processing. The user can clear any IIN bits they do not want
   *  the SCL to process.
   * arguments:
   *  pSession - pointer to session
   *  pIIN - pointer to IIN bits
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL mdnpdata_processIIN(
    TMWSESN *pSession,
    TMWTYPES_USHORT *pIIN);

  /* function: mdnpdata_storeIIN 
   * purpose: Store Internal Indication Bit received from remote device
   *  in response to read of object 80 variation 1. This would typically be
   *  used to read private IIN bits outside the range of 0-15, but could  
   *  also used for the standard IIN bits. 
   * arguments:
   *  pHandle - database handle returned from mdnpdata_init
   *  pointNumber - index of IIN bit to be stored
   *  value - value of IIN bit to be stored.
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL mdnpdata_storeIIN(
    void *pHandle,
    TMWTYPES_USHORT pointNumber,
    TMWTYPES_BOOL value);

  /* function: mdnpdata_init
   * purpose: Initialize DNP3 Master database on specified session
   * arguments:
   *  pSession - pointer to session on which to create database
   *  pUserHandle - user specified database handle passed to session open
   * returns:
   *  pointer to database handle for future database calls
   */
  void * TMWDEFS_GLOBAL mdnpdata_init(
    TMWSESN *pSession, 
    void *pUserHandle);

  /* function: mdnpdata_close
   * purpose: Close target database. After this call the database
   *  handle will be invalid.
   * arguments:
   *  pHandle - database handle returned from mdnpdata_init
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL mdnpdata_close(
    void *pHandle);

  /* function: mdnpdata_storeReadTime
   * purpose: Store time read from slave returned in Obj 50 Variation 1
   * arguments:
   *  pHandle - database handle returned from mdnpdata_init 
   *  pTimeStamp - pointer to time stamp returned in Obj 50 Variation 1
   * returns:
   *  void
   */
   void TMWDEFS_GLOBAL mdnpdata_storeReadTime(
    void *pHandle, 
    TMWDTIME *pTimeStamp);

  /* function: mdnpdata_storeBinaryInput
   * purpose: Store binary input value
   * arguments:
   *  pHandle - database handle returned from mdnpdata_init
   *  pointNumber - point number of point to store
   *  flags - DNP3 flags (including value) to store. Flag values are OR'd
   *    combinations of the following:
   *      DNPDEFS_DBAS_FLAG_ON_LINE - the binary input point has been read
   *        successfully
   *      DNPDEFS_DBAS_FLAG_RESTART - the field device that originated the
   *        data object has been restarted. This device may be the deviced
   *        reporting this data object.
   *      DNPDEFS_DBAS_FLAG_COMM_LOST - the device reporting this data object
   *        has lost communication with the originator of the data object
   *      DNPDEFS_DBAS_FLAG_REMOTE_FORCED - the state of the binary object
   *        has been forced to its current state at the originating device
   *      DNPDEFS_DBAS_FLAG_LOCAL_FORCED - the state of the binary object
   *        has been forced to its current state at the device reporting
   *        this data object
   *      DNPDEFS_DBAS_FLAG_CHATTER - the binary input point has been filtered
   *        in order to remove unneeded transitions in the state of the input
   *      DNPDEFS_DBAS_FLAG_BINARY_ON  - the current state of the input (On)
   *  isEvent - TMWDEFS_TRUE if this update was caused by a change event
   *  pTimeStamp - pointer to time stamp reported with this change event
   *   or TMWDEFS_NULL if no time was reported. Will always be TMWDEFS_NULL
   *   if isEvent is TMWDEFS_FALSE.
   * returns:
   *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpdata_storeBinaryInput(
    void *pHandle, 
    TMWTYPES_USHORT pointNumber, 
    TMWTYPES_UCHAR flags,
    TMWTYPES_BOOL isEvent,
    TMWDTIME *pTimeStamp);
  
  /* function: mdnpdata_storeDoubleInput
   * purpose: Store double bit input value
   * arguments:
   *  pHandle - database handle returned from mdnpdata_init
   *  pointNumber - point number of point to store
   *  flags - DNP3 flags (including value) to store. Flag values are OR'd
   *    combinations of the following:
   *      DNPDEFS_DBAS_FLAG_ON_LINE - the binary input point has been read
   *        successfully
   *      DNPDEFS_DBAS_FLAG_RESTART - the field device that originated the
   *        data object has been restarted. This device may be the deviced
   *        reporting this data object.
   *      DNPDEFS_DBAS_FLAG_COMM_LOST - the device reporting this data object
   *        has lost communication with the originator of the data object
   *      DNPDEFS_DBAS_FLAG_REMOTE_FORCED - the state of the binary object
   *        has been forced to its current state at the originating device
   *      DNPDEFS_DBAS_FLAG_LOCAL_FORCED - the state of the binary object
   *        has been forced to its current state at the device reporting
   *        this data object
   *      DNPDEFS_DBAS_FLAG_CHATTER - the binary input point has been filtered
   *        in order to remove unneeded transitions in the state of the input
   *      DNPDEFS_DBAS_FLAG_DOUBLE_INTER  
   *        the current state of the input (Intermediate -transitioning condition)
   *      DNPDEFS_DBAS_FLAG_DOUBLE_OFF    
   *        the current state of the input (Off)
   *      DNPDEFS_DBAS_FLAG_DOUBLE_ON     
   *        the current state of the input (On)
   *      DNPDEFS_DBAS_FLAG_DOUBLE_INDET  
   *        the current state of the input (Indeterminate -abnormal or custom 
   *        condition)
   *  isEvent - TMWDEFS_TRUE if this update was caused by a change event
   *  pTimeStamp - pointer to time stamp reported with this change event
   *   or TMWDEFS_NULL if no time was reported. Will always be TMWDEFS_NULL
   *   if isEvent is TMWDEFS_FALSE.
   *  timeQualifier - specifies how the timestamp was determined if 
   *   pTimeStamp is not TMWDEFS_NULL
   * returns:
   *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpdata_storeDoubleInput(
    void *pHandle, 
    TMWTYPES_USHORT pointNumber, 
    TMWTYPES_UCHAR flags,
    TMWTYPES_BOOL isEvent,
    TMWDTIME *pTimeStamp);

  /* function: mdnpdata_storeBinaryOutput
   * purpose: Store binary output value
   * arguments:
   *  pHandle - database handle returned from mdnpdata_init
   *  pointNumber - point number of point to store
   *  flags - DNP3 flags (including value) to store. Flag values are OR'd
   *    combinations of the following:
   *      DNPDEFS_DBAS_FLAG_ON_LINE - the binary input point has been read
   *        successfully
   *      DNPDEFS_DBAS_FLAG_RESTART - the field device that originated the
   *        data object has been restarted. This device may be the deviced
   *        reporting this data object.
   *      DNPDEFS_DBAS_FLAG_COMM_LOST - the device reporting this data object
   *        has lost communication with the originator of the data object
   *      DNPDEFS_DBAS_FLAG_REMOTE_FORCED - the state of the binary object
   *        has been forced to its current state at the originating device
   *      DNPDEFS_DBAS_FLAG_LOCAL_FORCED - the state of the binary object
   *        has been forced to its current state at the device reporting
   *        this data object
   *      DNPDEFS_DBAS_FLAG_BINARY_ON  - the current state of the input (On)
   *  isEvent - TMWDEFS_TRUE if this update was caused by a change event
   *  pTimeStamp - pointer to time stamp reported with this change event
   *   or TMWDEFS_NULL if no time was reported. Will always be TMWDEFS_NULL
   *   if isEvent is TMWDEFS_FALSE.
   * returns:
   *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpdata_storeBinaryOutput(
    void *pHandle, 
    TMWTYPES_USHORT pointNumber, 
    TMWTYPES_UCHAR flags,
    TMWTYPES_BOOL isEvent,
    TMWDTIME *pTimeStamp);

  /* function: mdnpdata_storeBinaryCmdStatus
   * purpose: Store binary command status returned from slave
   *  NOTE: this will NOT be called in the case that an binary output request 
   *   succeeded and all statuses are zero. Applications should register a 
   *   brm request callback function to receive the status of the actual request.
   *   This function is intended to be called to indicate the point that had the
   *   failed status. This function would also be called if a g13 event was 
   *   received from the outstation.
   * arguments:
   *  pHandle - database handle returned from mdnpdata_init
   *  pointNumber - point number of point to store
   *  status - status returned from slave. Top bit(0x80) is Commanded State.
   *  isEvent - TMWDEFS_TRUE if this update was caused by a change event
   *  pTimeStamp - pointer to time stamp reported with this change event
   *   or TMWDEFS_NULL if no time was reported. Will always be TMWDEFS_NULL
   *   if isEvent is TMWDEFS_FALSE.
   * returns:
   *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpdata_storeBinaryCmdStatus(
    void *pHandle,
    TMWTYPES_USHORT pointNumber,
    TMWTYPES_UCHAR status,
    TMWTYPES_BOOL isEvent,
    TMWDTIME *pTimeStamp);

  /* function: mdnpdata_storeBinaryCounter
   * purpose: Store binary counter value
   * arguments:
   *  pHandle - database handle returned from mdnpdata_init
   *  pointNumber - point number of point to store
   *  value - new counter value
   *  flags - DNP3 flags to store. Flag values are OR'd
   *    combinations of the following:
   *      DNPDEFS_DBAS_FLAG_ON_LINE - the binary input point has been read
   *        successfully
   *      DNPDEFS_DBAS_FLAG_RESTART - the field device that originated the
   *        data object has been restarted. This device may be the deviced
   *        reporting this data object.
   *      DNPDEFS_DBAS_FLAG_COMM_LOST - the device reporting this data object
   *        has lost communication with the originator of the data object
   *      DNPDEFS_DBAS_FLAG_REMOTE_FORCED - the state of the binary object
   *        has been forced to its current state at the originating device
   *      DNPDEFS_DBAS_FLAG_LOCAL_FORCED - the state of the binary object
   *        has been forced to its current state at the device reporting
   *        this data object
   *      DNPDEFS_DBAS_FLAG_CNTR_ROLLOVER - the accumulated value has exceeded
   *        has exceeded its maximum and rolled over to zero. 
   *        NOTE: This maximum value is not necessarily equal to (2^32-1) for 
   *        32 bit counters or (2^16-1) for 16 bit counters. It can be different 
   *        for each counter instance. Technical Bulletin TB-2002-001 Counter 
   *        Objects recommends "slave devices do not set the Rollover flag and 
   *        that host(Master) devices ignore the Rollover flag".
   *  isEvent - TMWDEFS_TRUE if this update was caused by a change event
   *  pTimeStamp - pointer to time stamp reported with this change event
   *   or TMWDEFS_NULL if no time was reported. Will always be TMWDEFS_NULL
   *   if isEvent is TMWDEFS_FALSE.
   * returns:
   *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpdata_storeBinaryCounter(
    void *pHandle, 
    TMWTYPES_USHORT pointNumber, 
    TMWTYPES_ULONG value, 
    TMWTYPES_UCHAR flags,
    TMWTYPES_BOOL isEvent,
    TMWDTIME *pTimeStamp);

  /* function: mdnpdata_storeFrozenCounter
   * purpose: Store frozen counter value
   * arguments:
   *  pHandle - database handle returned from mdnpdata_init
   *  pointNumber - point number of point to store
   *  value - new counter value
   *  flags - DNP3 flags to store. Flag values are OR'd
   *    combinations of the following:
   *   The following values (or OR'd combinations) are valid for this type:
   *      DNPDEFS_DBAS_FLAG_ON_LINE - the binary input point has been read
   *        successfully
   *      DNPDEFS_DBAS_FLAG_RESTART - the field device that originated the
   *        data object has been restarted. This device may be the deviced
   *        reporting this data object.
   *      DNPDEFS_DBAS_FLAG_COMM_LOST - the device reporting this data object
   *        has lost communication with the originator of the data object
   *      DNPDEFS_DBAS_FLAG_REMOTE_FORCED - the state of the binary object
   *        has been forced to its current state at the originating device
   *      DNPDEFS_DBAS_FLAG_LOCAL_FORCED - the state of the binary object
   *        has been forced to its current state at the device reporting
   *        this data object
   *      DNPDEFS_DBAS_FLAG_CNTR_ROLLOVER - the accumulated value has exceeded
   *        has exceeded its maximum and rolled over to zero. 
   *        NOTE: This maximum value is not necessarily equal to (2^32-1) for 
   *        32 bit counters or (2^16-1) for 16 bit counters. It can be different 
   *        for each counter instance. Technical Bulletin TB-2002-001 Counter 
   *        Objects recommends "slave devices do not set the Rollover flag and 
   *        that host(Master) devices ignore the Rollover flag".
   *  isEvent - TMWDEFS_TRUE if this update was caused by a change event
   *  pTimeStamp - pointer to time stamp reported with this message
   *   or TMWDEFS_NULL if no time was reported. Note that pTimeStamp
   *   can be non null for frozen counters in which case the time will
   *   be the reported time of the last freeze for this counter.
   * returns:
   *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpdata_storeFrozenCounter(
    void *pHandle, 
    TMWTYPES_USHORT pointNumber, 
    TMWTYPES_ULONG value, 
    TMWTYPES_UCHAR flags,
    TMWTYPES_BOOL isEvent,
    TMWDTIME *pTimeStamp);

  /* function: mdnpdata_storeAnalogInput
   * purpose: Store analog input value
   * arguments:
   *  pHandle - database handle returned from mdnpdata_init
   *  pointNumber - point number of point to store
   *  value - new value
   *  flags - DNP3 flags to store. Flag values are OR'd
   *    combinations of the following:
   *      DNPDEFS_DBAS_FLAG_ON_LINE - the binary input point has been read
   *        successfully
   *      DNPDEFS_DBAS_FLAG_RESTART - the field device that originated the
   *        data object has been restarted. This device may be the deviced
   *        reporting this data object.
   *      DNPDEFS_DBAS_FLAG_COMM_LOST - the device reporting this data object
   *        has lost communication with the originator of the data object
   *      DNPDEFS_DBAS_FLAG_REMOTE_FORCED - the state of the binary object
   *        has been forced to its current state at the originating device
   *      DNPDEFS_DBAS_FLAG_LOCAL_FORCED - the state of the binary object
   *        has been forced to its current state at the device reporting
   *        this data object
   *      DNPDEFS_DBAS_FLAG_OVER_RANGE - the digitized signal or calculation
   *        is greater than the type specified in TMWTYPES_ANALOG_VALUE. If the
   *        SCL determines that the value returned cannot fit in the type 
   *        specified by the object variation read it will set this OVER_RANGE bit.
   *      DNPDEFS_DBAS_FLAG_REFERENCE_CHK - the reference signal used to
   *        digitize the signal is not stable, and the resulting digitized
   *        value may not be correct.
   *  isEvent - TMWDEFS_TRUE if this update was caused by a change event
   *  pTimeStamp - pointer to time stamp reported with this change event
   *   or TMWDEFS_NULL if no time was reported. Will always be TMWDEFS_NULL
   *   if isEvent is TMWDEFS_FALSE.
   * returns:
   *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpdata_storeAnalogInput(
    void *pHandle, 
    TMWTYPES_USHORT pointNumber, 
    TMWTYPES_ANALOG_VALUE *pValue, 
    TMWTYPES_UCHAR flags,
    TMWTYPES_BOOL isEvent,
    TMWDTIME *pTimeStamp);

    /* function: mdnpdata_storeFrozenAnalogInput
   * purpose: Store frozen analog input value
   * arguments:
   *  pHandle - database handle returned from mdnpdata_init
   *  pointNumber - point number of point to store
   *  value - new value
   *  flags - DNP3 flags to store. Flag values are OR'd
   *    combinations of the following:
   *      DNPDEFS_DBAS_FLAG_ON_LINE - the binary input point has been read
   *        successfully
   *      DNPDEFS_DBAS_FLAG_RESTART - the field device that originated the
   *        data object has been restarted. This device may be the deviced
   *        reporting this data object.
   *      DNPDEFS_DBAS_FLAG_COMM_LOST - the device reporting this data object
   *        has lost communication with the originator of the data object
   *      DNPDEFS_DBAS_FLAG_REMOTE_FORCED - the state of the binary object
   *        has been forced to its current state at the originating device
   *      DNPDEFS_DBAS_FLAG_LOCAL_FORCED - the state of the binary object
   *        has been forced to its current state at the device reporting
   *        this data object
   *      DNPDEFS_DBAS_FLAG_OVER_RANGE - the digitized signal or calculation
   *        is greater than the type specified in TMWTYPES_ANALOG_VALUE. If the
   *        SCL determines that the value returned cannot fit in the type 
   *        specified by the object variation read it will set this OVER_RANGE bit.
   *      DNPDEFS_DBAS_FLAG_REFERENCE_CHK - the reference signal used to
   *        digitize the signal is not stable, and the resulting digitized
   *        value may not be correct.
   *  isEvent - TMWDEFS_TRUE if this update was caused by a change event
   *  pTimeStamp - pointer to time stamp reported with this change event
   *   or TMWDEFS_NULL if no time was reported. Will always be TMWDEFS_NULL
   *   if isEvent is TMWDEFS_FALSE.
   * returns:
   *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpdata_storeFrozenAnalogInput(
    void *pHandle, 
    TMWTYPES_USHORT pointNumber, 
    TMWTYPES_ANALOG_VALUE *pValue, 
    TMWTYPES_UCHAR flags,
    TMWTYPES_BOOL isEvent,
    TMWDTIME *pTimeStamp);

  /* function: mdnpdata_storeAnalogInputDeadband
   * purpose: Store analog input deadband
   * arguments:
   *  pHandle - database handle returned from mdnpdata_init
   *  pointNumber - point number of point to store
   *  value - new value
   * returns:
   *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpdata_storeAnalogInputDeadband(
    void *pHandle, 
    TMWTYPES_USHORT pointNumber, 
    TMWTYPES_ANALOG_VALUE *pValue);

  /* function: mdnpdata_storeAnalogOutput
   * purpose: Store analog output value
   * arguments:
   *  pHandle - database handle returned from mdnpdata_init
   *  pointNumber - point number of point to store
   *  value - new value
   *  flags - DNP3 flags to store. Flag values are OR'd
   *    combinations of the following:
   *      DNPDEFS_DBAS_FLAG_ON_LINE - the binary input point has been read
   *        successfully
   *      DNPDEFS_DBAS_FLAG_RESTART - the field device that originated the
   *        data object has been restarted. This device may be the deviced
   *        reporting this data object.
   *      DNPDEFS_DBAS_FLAG_COMM_LOST - the device reporting this data object
   *        has lost communication with the originator of the data object
   *      DNPDEFS_DBAS_FLAG_REMOTE_FORCED - the state of the binary object
   *        has been forced to its current state at the originating device
   * returns:
   *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpdata_storeAnalogOutput(
    void *pHandle, 
    TMWTYPES_USHORT pointNumber, 
    TMWTYPES_ANALOG_VALUE *pValue, 
    TMWTYPES_UCHAR flags,
    TMWTYPES_BOOL isEvent,
    TMWDTIME *pTimeStamp);

  /* function: mdnpdata_storeAnalogCmdStatus
   * purpose: Store analog output command status returned from slave
   *  NOTE: this will NOT be called in the case that an analog output request 
   *   succeeded and all statuses are zero. Applications should register a 
   *   brm request callback function to receive the status of the actual request.
   *   This function is intended to be called to indicate the point that had the
   *   failed status. This function would also be called if a g43 event was 
   *   received from the outstation.
   * arguments:
   *  pHandle - database handle returned from mdnpdata_init
   *  pointNumber - point number of point to store
   *  pValue - value returned from slave
   *  status - status of command returned from slave
   *  isEvent - TMWDEFS_TRUE if this update was caused by a change event
   *  pTimeStamp - pointer to time stamp reported with this change event
   *   or TMWDEFS_NULL if no time was reported. Will always be TMWDEFS_NULL
   *   if isEvent is TMWDEFS_FALSE.
   * returns:
   *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpdata_storeAnalogCmdStatus(
    void *pHandle, 
    TMWTYPES_USHORT pointNumber, 
    TMWTYPES_ANALOG_VALUE *pValue, 
    TMWTYPES_UCHAR status,
    TMWTYPES_BOOL isEvent,
    TMWDTIME *pTimeStamp);  

  /* function: mdnpdata_storeRestartTime
   * purpose: Store time value from restart response containing
   *  object 52 var 1 or 2
   * arguments:
   *  pHandle - database handle returned from mdnpdata_init
   *  time - time in milliseconds   
   * returns:
   *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpdata_storeRestartTime(
    void *pHandle,
    TMWTYPES_ULONG time);

  /* function: mdnpdata_storeBinaryOutput
   * purpose: Store binary output value
   * arguments:
   *  pHandle - database handle returned from mdnpdata_init
   *  pointNumber - point number of point to store
   *  indexedTime - indexed absolute time
   *  intervalCount - interval count
   *  intervalUnit - interval unit
   * returns:
   *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpdata_storeIndexedTime(
    void *pHandle,
    TMWTYPES_USHORT pointNumber,
    TMWDTIME indexedTime,
    TMWTYPES_ULONG intervalCount,
    TMWTYPES_BYTE intervalUnit
    );

  /* function: mdnpdata_storeString
   * purpose: Store string value
   * arguments:
   *  pHandle - database handle returned from mdnpdata_init
   *  pointNumber - point number of point to store
   *  pStrBuf - pointer to buffer with string value
   *  strLen - number of bytes in value
   * returns:
   *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpdata_storeString(
    void *pHandle,
    TMWTYPES_USHORT pointNumber,
    TMWTYPES_UCHAR *pStrBuf,
    TMWTYPES_UCHAR strLen);

 /* function: mdnpdata_storeExtString
   * purpose: Store extended string value
   * arguments:
   *  pHandle - database handle returned from mdnpdata_init
   *  pointNumber - point number of point to store
   *  pStrBuf - pointer to buffer with string value
   *  strLen - number of bytes in value
   *  flags - DNP3 flags to store. Flag values are OR'd
   *    combinations of the following:
   *      DNPDEFS_DBAS_FLAG_ON_LINE - the binary input point has been read
   *        successfully
   *      DNPDEFS_DBAS_FLAG_RESTART - the field device that originated the
   *        data object has been restarted. This device may be the deviced
   *        reporting this data object.
   *      DNPDEFS_DBAS_FLAG_COMM_LOST - the device reporting this data object
   *        has lost communication with the originator of the data object
   *      DNPDEFS_DBAS_FLAG_REMOTE_FORCED - the state of the binary object
   *        has been forced to its current state at the originating device
   *  isEvent - TMWDEFS_TRUE if this update was caused by a change event
   *  pTimeStamp - pointer to time stamp reported with this change event
   *   or TMWDEFS_NULL if no time was reported. Will always be TMWDEFS_NULL
   *   if isEvent is TMWDEFS_FALSE.
   * returns:
   *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpdata_storeExtString(
    void *pHandle,
    TMWTYPES_USHORT pointNumber,
    TMWTYPES_UCHAR *pStrBuf,
    TMWTYPES_USHORT strLen,
    TMWTYPES_UCHAR flags,
    TMWTYPES_BOOL isEvent,
    TMWDTIME *pTimeStamp);

  /* function: mdnpdata_storeActiveConfig
   * purpose: Store Active Configuration Response
   * arguments:
   *  pHandle - database handle returned from mdnpdata_init
   *  index - index since multiple statuses may be returned.
   *  timeDelay - how long to delay
   *  statusCode - status code 
   *  pStrBuf - pointer to error string
   *  strLen - number of bytes in error string
   * returns:
   *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpdata_storeActiveConfig(
    void *pHandle,
    TMWTYPES_UCHAR index,
    TMWTYPES_ULONG timeDelay,
    TMWTYPES_UCHAR statusCode,
    TMWTYPES_UCHAR *pStrBuf,
    TMWTYPES_UCHAR strLen);

  /* function: mdnpdata_storeVirtualTerminal
   * purpose: Store virtual terminal value
   * arguments:
   *  pHandle - database handle returned from mdnpdata_init
   *  pointNumber - point number of point to store
   *  pVtermBuf - pointer to buffer with virtual terminal value
   *  vtermLen - number of bytes in value
   * returns:
   *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpdata_storeVirtualTerminal(
    void *pHandle,
    TMWTYPES_USHORT pointNumber,
    TMWTYPES_UCHAR *pVtermBuf,
    TMWTYPES_UCHAR vtermLen);

  /* function: mdnpdata_storeAuthKey
   * purpose: Store file authentication key
   * arguments:
   *  pHandle - database handle returned from mdnpdata_init
   *  authKey - authentication key returned from slave
   * returns:
   *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
   */
  TMWTYPES_BOOL mdnpdata_storeFileAuthKey(
    void *pHandle,
    TMWTYPES_ULONG authKey);

  /* function: mdnpdata_storeFileStatus
   * purpose: Store file status
   * arguments:
   *  pHandle - database handle returned from mdnpdata_init
   *  handle - file handle
   *  size - file size if open for read (NULL otherwise)
   *  maxBlockSize - maximum block size outstation will return
   *  requestId - id of request submitted
   *  status - status/error condition
   *  nOptionalChars - number of bytes in pOptionalChars
   *  pOptionalChars - Optional ASCII characters or TMWDEFS_NULL if none.
   * returns:
   *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
   */
  TMWTYPES_BOOL mdnpdata_storeFileStatus(
    void *pHandle,
    TMWTYPES_ULONG handle,
    TMWTYPES_ULONG fileSize,
    TMWTYPES_USHORT maxBlockSize,
    TMWTYPES_USHORT requestId,
    DNPDEFS_FILE_CMD_STAT status,
    TMWTYPES_USHORT nOptionalChars,
    const TMWTYPES_CHAR *pOptionalChars);

  /* function: mdnpdata_storeFileData
   * purpose: Store file data
   * arguments:
   *  pHandle - database handle returned from mdnpdata_init
   *  handle - file handle
   *  blockNumber - current file block number
   *  lastBlockFlag - TRUE if last block
   *  nBytesInBlockData - number of bytes in pBlockData
   *  pBlockData - the data for this block or TMWDEFS_NULL if none.
   * returns:
   *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
   */
  TMWTYPES_BOOL mdnpdata_storeFileData(
    void *pHandle,
    TMWTYPES_ULONG handle,
    TMWTYPES_ULONG blockNumber,
    TMWTYPES_BOOL lastBlockFlag,
    TMWTYPES_USHORT nBytesInBlockData,
    const TMWTYPES_UCHAR *pBlockData);

  /* function: mdnpdata_storeFileDataStatus
   * purpose: Store file data status
   * arguments:
   *  pHandle - database handle returned from mdnpdata_init
   *  handle - file handle
   *  blockNumber - current file block number
   *  lastBlockFlag - TRUE if last block
   *  status - status/error condition
   *  nOptionalChars - number of bytes in pOptionalChars
   *  pOptionalChars - Optional ASCII characters or TMWDEFS_NULL if none.
   * returns:
   *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
   */
  TMWTYPES_BOOL mdnpdata_storeFileDataStatus(
    void *pHandle,
    TMWTYPES_ULONG handle,
    TMWTYPES_ULONG blockNumber,
    TMWTYPES_BOOL lastBlockFlag,
    DNPDEFS_FILE_TFER_STAT status,
    TMWTYPES_USHORT nOptionalChars,
    const TMWTYPES_CHAR *pOptionalChars);

  /* function: mdnpdata_storeFileInfo
   * purpose: Store file info for each file/directory returned
   * arguments:
   *  pHandle - database handle returned from mdnpdata_init
   *  fileNameOffset - offset to file name in message. Probably no value.
   *  fileNameSize - length of pFileName
   *  fileType - simple file(1) or directory(0)
   *  fileSize - number of bytes in file
   *  pFileCreationTime - creation time of the file
   *  permissions - permissions of file
   *  pFileName - pointer to the file name or TMWDEFS_NULL if none.
   * returns:
   *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
   */
  TMWTYPES_BOOL mdnpdata_storeFileInfo(
    void *pHandle,
    TMWTYPES_USHORT fileNameOffset,
    TMWTYPES_USHORT fileNameSize,
    DNPDEFS_FILE_TYPE fileType,
    TMWTYPES_ULONG  fileSize,
    TMWDTIME *pFileCreationTime,
    DNPDEFS_FILE_PERMISSIONS permissions,
    const TMWTYPES_CHAR *pFileName);

  /* function: mdnpdata_getFileAuthKey
   * purpose: Return the authentication key provided by the slave
   * arguments:
   *  pHandle - database handle returned from mdnpdata_init
   * returns:
   *  TMWTYPES_ULONG the authentication key value
   */
  TMWTYPES_ULONG mdnpdata_getFileAuthKey(
    void *pHandle);

  /* function: mdnpdata_openLocalFile
   * purpose: Open a file on the local file system
   * arguments:
   *  pHandle - database handle returned from mdnpdata_init
   *  pLocalFileName - name of local file to open, null terminated string.
   *  fileMode - simple file(1) or directory(0)
   * returns:
   *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
   */
  TMWTYPES_BOOL mdnpdata_openLocalFile(
    void *pHandle,
    const TMWTYPES_CHAR *pLocalFileName, 
    DNPDEFS_FILE_MODE fileMode);

  /* function: mdnpdata_closeLocalFile
   * purpose: Close a file on the local file system
   * arguments:
   *  pHandle - database handle returned from mdnpdata_init
   * returns:
   *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
   */
  TMWTYPES_BOOL mdnpdata_closeLocalFile(
    void *pHandle);

  /* function: mdnpdata_getLocalFileInfo
   * purpose: Get size and time of creation for a file on the local file system
   * arguments:
   *  pHandle - database handle returned from mdnpdata_init
   *  pLocalFileName - name of local file to get info for, null terminated string.
   *  pFileSize - size of file or 0xffffffff if not known
   *  pDateTime - time of creation for file or Jan 1 1970 if not known.
   * returns:
   *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
   */
  TMWTYPES_BOOL mdnpdata_getLocalFileInfo(
    void *pHandle,
    const TMWTYPES_CHAR *pLocalFileName, 
    TMWTYPES_ULONG *pFileSize,
    TMWDTIME *pDateTime); 

  /* function: mdnpdata_readLocalFile
   * purpose: Read data from a local file and place it in a buffer
   * arguments:
   *  pHandle - database handle returned from mdnpdata_init
   *  pLocalFileHandle - local file handle
   *  pBuf - buffer to fill
   *  bufSize - number of bytes to put in pBuf
   *  pLastBlock - fill this in to indicate last block
   * returns:
   *  TMWTYPES_USHORT - number of bytes read
   */
  TMWTYPES_USHORT mdnpdata_readLocalFile(
    void *pHandle,
    TMWTYPES_UCHAR *pBuf,
    TMWTYPES_USHORT bufSize,
    TMWTYPES_BOOL *pLastBlock);

  /* function: mdnpdata_storeDeviceAttribute
   * purpose:  Store particular device attribute value.
   * arguments: 
   *  pHandle - database handle returned from mdnpdata_init
   *  point - point index of device attribute to store
   *   0 for standard attributes, other values for user-specific attribute set
   *  variation - variation of attribute, used like an index to determine which
   *   attribute.
   *  pData - pointer to structure contaning attribute value to store
   * returns: 
   *  void
   */
  void TMWDEFS_GLOBAL mdnpdata_storeDeviceAttribute(
    void *pHandle,
    TMWTYPES_USHORT point,
    TMWTYPES_UCHAR variation,
    DNPDATA_ATTRIBUTE_VALUE *pData);

  /* function: mdnpdata_storeDeviceAttrProperty
   * purpose:  Store property value for a particular device attribute.
   * arguments: 
   *  pHandle - database handle returned from mdnpdata_init
   *  point - point index of device attribute property to store
   *   0 for standard attributes, other values for user-specific attribute set
   *  variation - variation of attribute, used like an index to determine which
   *   attribute.
   *  property - property of device attribute
   *   0x00 indicates attribute is NOT writable by master
   *   0x01 indicates attribute is writable by master
   * returns: 
   *  void
   */
  void TMWDEFS_GLOBAL mdnpdata_storeDeviceAttrProperty(
    void *pDbHandle, 
    TMWTYPES_USHORT point,
    TMWTYPES_UCHAR variation,
    TMWTYPES_UCHAR property);

  /* function: mdnpdata_datasetProtoQuantity   
   * purpose: Get quantity of Data Set Prototypes in the database.
   * arguments:
   *  pHandle - database handle returned from mdnpdata_init
   * returns:
   *  Quantity of prototypes in database
   */
  TMWTYPES_USHORT mdnpdata_datasetProtoQuantity(
    void *pHandle);

  /* function: mdnpdata_datasetProtoSlaveQty
   * purpose: Get quantity of Data Set Prototypes in the database that
   *  were read from the slave. Prototype ids must start with 0 and be 
   *  contiguous, however the prototypes defined by the outstation or slave 
   *  are numbered 0 to n-1. The ones defined by the master should be numbered 
   *  n to x-1. Where n is the number of prototypes defined by the outstation, 
   *  and x is the number of prototypes defined by the master and outstation 
   *  combined.
   * arguments:
   *  pHandle - database handle returned from mdnpdata_init
   * returns:
   *  Quantity of prototypes read from the slave
   *  NOTE: this would be the number of different prototypes written to the
   *   database by mdnpdata_storeDatasetProtoUUID()
   */
  TMWTYPES_USHORT mdnpdata_datasetProtoSlaveQty(
    void *pHandle);

  /* function: mdnpdata_storeDatasetProtoUUID
   * purpose: Create an entry for a Data Set prototype if it does not
   *  exist and store this Universally Unique Identifier (UUID).
   * arguments:
   *  pHandle - database handle returned from mdnpdata_init
   *  pointNumber - point number or prototype id of Data Set prototype to store
   *  pUUID - pointer to 16 octet UUID.
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL mdnpdata_storeDatasetProtoUUID(
    void *pHandle, 
    TMWTYPES_USHORT pointNumber, 
    TMWTYPES_UCHAR *pUUID);

  /* function: mdnpdata_storeDatasetProto
   * purpose: Store Data Set prototype element data.
   * arguments:
   *  pHandle - database handle returned from mdnpdata_init
   *  pointNumber - point number or prototype id of prototype to store
   *  elemIndex - index of prototype element to store starting with zero
   *   for the first element after mandatory prototype ID and UUID. If a
   *   Namespace and Name are present these will be elemIndex 0 and 1.
   *  pDescr - pointer to descriptor element structure
   *   NOTE: if pDescr->ancillaryValue->type == DNPDATA_VALUE_STRPTR, this is just
   *    a pointer, the string itself must be copied somewhere. 
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL mdnpdata_storeDatasetProto(
    void *pHandle, 
    TMWTYPES_USHORT pointNumber, 
    TMWTYPES_UCHAR  elemIndex,
    DNPDATA_DATASET_DESCR_ELEM *pDescr);

  /* function: mdnpdata_datasetProtoGet
   * purpose: Get a prototype from database for this prototype id. 
   * arguments:
   *  pHandle - database handle returned from mdnpdata_init
   *  pointNumber - point number or prototype id of prototype to get
   *  pNumberElems - number of elements returned for specified Data Set Prototype
   *   not counting the mandatory prototype id and UUID, which should not
   *   be contained in the array of elements.
   *  pUUID - pointer to a 16 byte array to be filled in by this function with
   *   a UUID uniquely identifying this prototype.
   * returns:
   *  Pointer to array of Data Set prototype contents structures
   *   DNPDATA_DATASET_DESCR_ELEM.  This points to memory maintained
   *   by the database. It can be in ROM or in RAM. This pointer will need
   *   to be valid until mdnpdata_datasetProtoRelease() is called.
   *   NOTE: The array of elements should not contain the mandatory Prototype ID
   *   and UUID. Prototype ID is specified as a parameter and UUID will be 
   *   returned by pUUID parameter.
   *   If a Namespace and Name element are present in the prototype they
   *   must be the first and second element in this array.  If either Namespace 
   *   or Name is present the other must also be present.
   *  TMWDEFS_NULL if failed to get pointer to array.
   */
  DNPDATA_DATASET_DESCR_ELEM * TMWDEFS_GLOBAL mdnpdata_datasetProtoGet(
    void *pHandle,
    TMWTYPES_USHORT pointNumber,
    TMWTYPES_UCHAR *pNumberElems,
    TMWTYPES_UCHAR *pUUID);
 
  /* function: mdnpdata_datasetProtoRelease
   * purpose: Release the pointer that was returned by in mdnpdata_datasetProtoGet 
   *  The database is free to deallocate or reuse the memory that was pointed to. 
   *  The SCL will not attempt to reference that pointer anymore.
   *  pHandle - database handle returned from mdnpdata_init 
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL mdnpdata_datasetProtoRelease(
     void *pHandle);

   /* function: mdnpdata_datasetProtoGetID
    * purpose: Get a prototype ID (point index) for a Data Set prototype 
    *  with this UUID, if one exists in the database.
    * arguments:
    *  pHandle - handle to database returned from mdnpdata_init
    *  pUUID - pointer to 16 byte UUID string to be looked up
    *  pPointNumber - point number or prototype id to be filled in.
    * returns:
    *  TMWDEFS_TRUE if prototype was found
    *  TMWDEFS_FALSE otherwise
    */
  TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpdata_datasetProtoGetID(
    void *pHandle,
    TMWTYPES_UCHAR  *pUUID,
    TMWTYPES_USHORT *pPointNumber);

  /* function: mdnpdata_datasetDescrQuantity 
   * purpose: Get quantity of Data Set Descriptors in the database.
   * arguments:
   *  pHandle - database handle returned from mdnpdata_init
   * returns:
   *  Quantity of Descriptors in database
   */
  TMWTYPES_USHORT mdnpdata_datasetDescrQuantity(
    void *pHandle);

  /* function: mdnpdata_datasetDescrSlaveQty
   * purpose: Get quantity of Data Set Descriptors in the database that
   *  were read from the slave. Descriptor ids must start with 0 and be 
   *  contiguous, however the descriptors defined by the outstation or slave 
   *  are numbered 0 to n-1. The ones defined by the master should be numbered 
   *  n to x-1. Where n is the number of descriptors defined by the outstation, 
   *  and x is the number of descriptors defined by the master and outstation 
   *  combined.
   * arguments:
   *  pHandle - database handle returned from mdnpdata_init
   * returns:
   *  Quantity of descriptors read from the slave
   *  NOTE: this would be the number of different descriptors written to the 
   *   database by mdnpdata_storeDatasetDescrCont()
   */
  TMWTYPES_USHORT mdnpdata_datasetDescrSlaveQty(
    void *pHandle);

  /* function: mdnpdata_storeDatasetDescrCont
   * purpose: Store a Data Set descriptor contents structure to support 
   *   read of object group 86 variation 1
   * arguments:
   *  pHandle - database handle returned from mdnpdata_init
   *  pointNumber - point number of point to store 
   *  elemIndex - index of element in Data Set descriptor to be stored starting
   *   with index 0. The mandatory Data Set descriptor ID will not be stored as 
   *   a descriptor element. It will be passed in the pointNumber parameter.
   *  pDescr - pointer to structure containing data to be written
   *   NOTE: if pDescr->ancillaryValue->type == DNPDATA_VALUE_STRPTR, this is just
   *    a pointer, the string itself must be copied somewhere. 
   * returns: 
   *  void
   */
  void TMWDEFS_GLOBAL mdnpdata_storeDatasetDescrCont(
    void *pHandle, 
    TMWTYPES_USHORT pointNumber, 
    TMWTYPES_UCHAR  elemIndex,
    DNPDATA_DATASET_DESCR_ELEM *pDescr);
 
  /* function: mdnpdata_storeDatasetDescrChars
   * purpose: Store a Data Set descriptor characteristics from a  
   *   read of object group 86 variation 2
   * arguments:
   *  pHandle - database handle returned from mdnpdata_init
   *  pointNumber - point number of Data Set descriptor to store 
   *  value - characteristics value
   *   DNPDEFS_DATASET_CHAR_RD 
   *   DNPDEFS_DATASET_CHAR_WR  
   *   DNPDEFS_DATASET_CHAR_ST 
   *   DNPDEFS_DATASET_CHAR_EV  
   *   DNPDEFS_DATASET_CHAR_DF  
   * returns: 
   *  void
   */
  void TMWDEFS_GLOBAL mdnpdata_storeDatasetDescrChars(
    void *pHandle, 
    TMWTYPES_USHORT pointNumber, 
    TMWTYPES_UCHAR  value);
  
  /* function: mdnpdata_storeDatasetDescrIndex
   * purpose: Store a Data Set descriptor index structure to support 
   *   read of object group 86 variation 3
   * arguments:
   *  pHandle - database handle returned from mdnpdata_init
   *  pointNumber - point number of point to store 
   *  elemIndex - index of element in Data Set descriptor index to be stored
   *   starting with index 0. There will be one of these for each data and 
   *   control value element in the descriptor and in any prototypes contained 
   *   by it.
   *  pDescr - pointer to structure containing data to be written
   * returns: 
   *  void
   */
  void TMWDEFS_GLOBAL mdnpdata_storeDatasetDescrIndex(
    void *pHandle, 
    TMWTYPES_USHORT pointNumber, 
    TMWTYPES_UCHAR  elemIndex,
    DNPDATA_DATASET_DESCR_INDEX *pDescr);

  /* function: mdnpdata_datasetDescrGetCont
   * purpose: Get a pointer to array of Data Set descriptor contents
   *   structures to support write of object group 86 variation 1
   * arguments:
   *  pHandle - database handle returned from mdnpdata_init
   *  pointNumber - descriptor id or point number of descriptor to get
   *  pNumberElems - pointer to number of elements returned in specified descriptor 
   *   array.
   * returns:
   *  Pointer to array of data set descriptor contents structures
   *   DNPDATA_DATASET_DESCR_ELEM. This points to memory maintained
   *   by the database. It can be in ROM or in RAM. This pointer will need
   *   to be valid until mdnpdata_datasetDescrRelease() is called.
   *  NOTE: Data Set id is a mandatory element and is specified as a parameter of 
   *  this function. It is not returned as part of the DNPDATA_DATASET_DESCR_ELEM 
   *  array. Descriptor name is an optional element, but if present, must be the 
   *  first element in the array returned.
   *  TMWDEFS_NULL if failed to get pointer to array.
   */
  DNPDATA_DATASET_DESCR_ELEM * TMWDEFS_GLOBAL mdnpdata_datasetDescrGetCont(
    void *pHandle,
    TMWTYPES_USHORT pointNumber,
    TMWTYPES_UCHAR *pNumberElems);

  /* The protocol does not support a write of object group 86 variation 2 
   * characteristics, so there is no function to get the data from the database.
   */

  /* function: mdnpdata_datasetDescrGetIndex
   * purpose: Get a pointer to array of Data Set descriptor index
   *   structures to support write of object group 86 variation 3
   * arguments:
   *  pHandle - database handle returned from mdnpdata_init
   *  pointNumber - descriptor id or point number of descriptor to get
   *  pNumberElems - number of elements returned in specified Data Set array
   * returns:
   *  Pointer to array of Data Set descriptor index structures
   *   DNPDATA_DATASET_DESCR_INDEX. This points to memory maintained
   *   by the database. It can be in ROM or in RAM. This pointer will need
   *   to be valid until mdnpdata_datasetDescrRelease() is called.
   *   NOTE: there should be one element of this array for each data and control
   *    value element in the Data Set descriptor (including data and control 
   *    value elements in the contained prototypes).
   *  TMWDEFS_NULL if failed to get pointer to array.
   */
  DNPDATA_DATASET_DESCR_INDEX * TMWDEFS_GLOBAL mdnpdata_datasetDescrGetIndex(
    void *pHandle,
    TMWTYPES_USHORT pointNumber,
    TMWTYPES_UCHAR *pNumberElems);
   
  /* function: mdnpdata_datasetDescrRelease
   * purpose: Release the pointer that was returned by mdnpdata_datasetDescrGetCont
   *  or mdnpdata_datasetDescrGetIndex 
   *  The database is free to deallocate or reuse the memory that was pointed to. The
   *  SCL will not attempt to reference that pointer anymore.
   * arguments:
   *  pHandle - database handle returned from mdnpdata_init 
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL mdnpdata_datasetDescrRelease(
     void *pHandle);

  /* function: mdnpdata_storeDatasetTime
   * purpose: Store a Data Set timestamp received in either Object Group 87 
   *  Data Set present value, or Object Group 88 Data Set event response
   * arguments:
   *  pHandle - database handle returned from mdnpdata_init
   *  pointNumber - point number (Data Set ID) of Data Set to store 
   *  pTimeStamp - pointer to timestamp received
   *  isEvent - TMWDEFS_TRUE if this update was caused by a change event
   * returns: 
   *  void
   */
  void TMWDEFS_GLOBAL mdnpdata_storeDatasetTime(
    void *pHandle, 
    TMWTYPES_USHORT pointNumber, 
    TMWDTIME *pTimeStamp,
    TMWTYPES_BOOL isEvent);

  /* function: mdnpdata_storeDataset
   * purpose: Store Data Set value received in either Object Group 87 
   *  Data Set present value, or Object Group 88 Data Set event response
   * arguments:
   *  pHandle - database handle returned from mdnpdata_init
   *  pointNumber - point number (datataset ID) of Data Set to store 
   *  elemIndex - index of Data Set element to be stored starting
   *   with index 0 indicating the first element after the mandatory 
   *   Data Set ID and timeStamp.
   *  pElem - pointer to structure containing data to be written 
   *   NOTE: if pElem->type == DNPDATA_VALUE_STRPTR, this is just
   *    a pointer, the string itself must be copied somewhere. 
   *  isEvent - TMWDEFS_TRUE if this update was caused by a change event
   * returns: 
   *  void
   */
  void TMWDEFS_GLOBAL mdnpdata_storeDataset(
    void *pHandle, 
    TMWTYPES_USHORT pointNumber, 
    TMWTYPES_UCHAR  elemIndex,
    DNPDATA_DATASET_VALUE *pElem,
    TMWTYPES_BOOL isEvent);

  /* function: mdnpdata_datasetGet
   * purpose: Get a pointer to array of Data Set value
   *   structures to support write of object group 87 variation 1
   * arguments:
   *  pHandle - database handle returned from mdnpdata_init
   *  pointNumber - point number (datataset ID) of Data Set to get 
   *  pNumberElems - number of elements returned in specified Data Set array
   *  pTimeStamp - pointer to mandatory timestamp 
   * returns:
   *  Pointer to array of Data Set value structures
   *   DNPDATA_DATASET_VALUE. This points to memory maintained
   *   by the database. This pointer will need to be valid until 
   *   mdnpdata_datasetDescrRelease() is called.
   *   NOTE: The array of elements should not contain the mandatory Data Set ID
   *   and timeStamp.
   *  TMWDEFS_NULL if failed to get pointer to array.
   */
  DNPDATA_DATASET_VALUE * TMWDEFS_GLOBAL mdnpdata_datasetGet(
    void *pHandle,
    TMWTYPES_USHORT pointNumber,
    TMWTYPES_UCHAR *pNumberElems,
    TMWDTIME *pTimeStamp);
  
  /* function: mdnpdata_datasetRelease
   * purpose: Release the pointer that was returned by mdnpdata_datasetGet
   *  The database is free to deallocate or reuse the memory that was pointed to.
   *  The SCL will not attempt to reference that pointer anymore.
   * arguments:
   *  pHandle - database handle returned from mdnpdata_init 
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL mdnpdata_datasetRelease(
     void *pHandle);

  /* function: mdnpdata_authIsCriticalReq 
   * purpose: Determine if this message should be considered critical. 
   *  NOTE: The Specification lists requests that are required to be critical.
   *   Although allowed, no responses are required by the spec to be critical.
   *   It is acceptable and normal to always return TMWDEFS_FALSE from this function
   *   Only consider a response critical if it is necessary that you do so.
   * arguments:  
   *  pHandle - database handle returned from mdnpdata_init
   *  pRxMsg - pointer to received message
   *  msgLength = length of received message
   *  pUserNumber - user number to be filled in if this is a critical message
   *   Authentication Spec says to use Default User Number (1) for challenging 
   *   any response from outstation.
   * returns:
   *  TMWDEFS_TRUE if this is a critical message
   *  TMWDEFS_FALSE otherwise
   */
  TMWTYPES_BOOL mdnpdata_authIsCriticalReq(
    void            *pHandle, 
    TMWTYPES_UCHAR  *pRxMsg,
    TMWTYPES_USHORT  msgLength,
    TMWTYPES_USHORT *pUserNumber);

#if MDNPCNFG_SUPPORT_SA_VERSION5 

#if DNPCNFG_SUPPORT_AUTHKEYUPDATE
  /* function: mdnpdata_authGetOSName
   * purpose:  Get the name of the outstation this master is connected to. 
   *  Spec says this must be preconfigured on both the master and outstation.
   *  This is used when Secure Authentication Version 5 or later Update Key Change 
   *  symmetric and asymmetric methods are used.
   * arguments:    
   *  pHandle - handle to database returned from mdnpdata_init
   *  pOSName - pointer to location where Outstation name should be copied
   *  pOSNameLength - when called this is the maximum length allowed for 
   *    the name, on return this should be set to the length of the name.
   * returns: 
   *  TMWDEFS_TRUE if successful
   *  TMWDEFS_FALSE otherwise
   */ 
  TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpdata_authGetOSName(
    void            *pHandle,
    TMWTYPES_UCHAR  *pOSName,
    TMWTYPES_USHORT *pOSNameLength);

  /* function: mdnpdata_authGetUserName
   * purpose:  Get globally unique user name provided by Authority from the database
   * arguments:    
   *  pHandle - handle to database returned from mdnpdata_init 
   *  userNameDbHandle - handle for looking up the data for a particular user name.
   *    This handle is meaningful to the database and is intended to be more 
   *    convenient than the full user name. This handle was determined when the 
   *    user name and other data was added to the database. The actual User 
   *    Number will not be known on the master when remote Update Key 
   *    distribution is used until the outstation assigns one and informs 
   *    the master.
   *  pUserName - pointer to location where user name should be copied
   *  pUserNameLength - when called this is the maximum length allowed for 
   *    the name, on return this should be set to the length of the name.
   * returns: 
   *  TMWDEFS_TRUE if successful
   *  TMWDEFS_FALSE otherwise
   */ 
  TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpdata_authGetUserName(
    void            *pHandle, 
    void            *userNameDbHandle,
    TMWTYPES_UCHAR  *pUserName,
    TMWTYPES_USHORT *pUserNameLength);

  /* This is called when an x.509 user certificate is used */ 
  /* function: mdnpdata_authGetKeyChangeMethod
   * purpose:  Determine what key change method to use. The specification says
   *  this shall be pre-configured at the master and only one method shall be active
   *  between a particular master and outstation. Therefore you may want to always 
   *  return the same method and not allow different ones per request.
   * arguments:    
   *  pHandle - handle to database returned from mdnpdata_init
   *  userNameDbHandle - handle for looking up the data for a particular user name.
   *    This handle is meaningful to the database and is intended to be more 
   *    convenient than the full user name. This handle was determined when the 
   *    user name and other data was added to the database. The actual User 
   *    Number will not be known on the master when remote Update Key 
   *    distribution is used until the outstation assigns one and informs 
   *    the master. 
   *  pKeyChangeMethod - pointer to key change method to be filled in
   * returns: 
   *  TMWDEFS_TRUE if successful
   *  TMWDEFS_FALSE otherwise
   */ 
  TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpdata_authGetKeyChangeMethod(
    void            *pHandle, 
    void            *userNameDbHandle,
    TMWTYPES_UCHAR  *pKeyChangeMethod);

  /* function: mdnpdata_authGetChangeUserData
   * purpose:  Get g120v10 data to send to the outstation. ALL of this data was
   *  provided by the Authority and stored before this key change procedure was
   *  initiated.
   * arguments:    
   *  pHandle - handle to database returned from mdnpdata_init
   *  userNameDbHandle - handle for looking up the data for a particular user name.
   *    This handle is meaningful to the database and is intended to be more 
   *    convenient than the full user name. This handle was determined when the 
   *    user name and other data was added to the database. The actual User 
   *    Number will not be known on the master when remote Update Key 
   *    distribution is used until the outstation assigns one and informs 
   *    the master.
   *  pStatusChangeSequenceNumber - to be filled in 
   *  pKeyChangeMethod - pointer to key change method to be filled in. The specification says
   *    this shall be pre-configured at the master and only one method shall be active
   *    between a particular master and outstation. Therefore you may want to always 
   *    return the same method and not allow different ones per request.
   *  pUserRole - to be filled in. 
   *   DNPAUTH_USER_ROLE_VIEWER     
   *   DNPAUTH_USER_ROLE_OPERATOR      
   *   DNPAUTH_USER_ROLE_ENGINEER
   *   DNPAUTH_USER_ROLE_INSTALLER  
   *   DNPAUTH_USER_ROLE_SECADM      
   *   DNPAUTH_USER_ROLE_SECAUD    
   *   DNPAUTH_USER_ROLE_RBACMNT  
   *   DNPAUTH_USER_ROLE_SINGLEUSER
   *  pUserExpiryInterval - to be filled in. Indicates number of days after receiving the 
   *    User Status Change object that the outstation shall consider the user role to be expired
   * returns: 
   *  TMWDEFS_TRUE if successful
   *  TMWDEFS_FALSE otherwise
   */ 
  TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpdata_authGetChangeUserData(
    void            *pHandle, 
    void            *userNameDbHandle,
    TMWTYPES_ULONG  *pStatusChangeSequenceNumber,
    TMWTYPES_UCHAR  *pKeyChangeMethod,
    TMWTYPES_USHORT *pUserRole,
    TMWTYPES_USHORT *pUserRoleExpiryInterval);
 
  /* function: mdnpdata_authGetCertData
   * purpose:  Get the certification data for the user specified by this userHandle
   *  to be sent in this g120v10 User Status Change message. This certification data
   *  will be provided by the AUTHORITY to certify that the other data in the g120v10
   *   message is correct. 
   *  If symmetric key method, the pre-shared (between AUTHORITY and outstation) 
   *    Authority Certification Key will be used by the AUTHORITY to certify this data.
   *  If asymmetric key method, the AUTHORITY will use the Authority Private Key to sign
   *    this data.  Outstation will need to be configured with the Authority Public Key.
   * arguments:  
   *  pHandle - handle to database returned from mdnpdata_init  
   *  userNameDbHandle - handle for looking up the data for a particular user name.
   *    This handle is meaningful to the database and is intended to be more 
   *    convenient than the full user name. This handle was determined when the 
   *    user name and other data was added to the database. The actual User 
   *    Number will not be known on the master when remote Update Key 
   *    distribution is used until the outstation assigns one and informs 
   *    the master.
   * pCertData - pointer to buffer to be filled in with certification data.
   *   NOTE: This certification data will contain an operation, 
   *   Status Change Sequence (SCS) number between the Authority and Outstation,
   *   Operation, User Role, Role Expiry Interval, A globally unique name representing the user,
   *   and if asymmetric a Users public key.
   * pCertDataLength - pointer to the maximum certification length allowed and should be
   *  filled in on return with the actual length of the certification data. 
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpdata_authGetCertData(
    void            *pHandle,
    void            *userNameDbHandle,
    TMWTYPES_UCHAR  *pCertData,
    TMWTYPES_USHORT *pCertDataLength);

  /* function: mdnpdata_authStoreUpdKeyChangeReply
   * purpose:  Store the user number from the g120v12 Update Key Change Reply message 
   *   from the outstation.
   * arguments:  
   *  pHandle - handle to database returned from mdnpdata_init  
   *  userNameDbHandle - handle for looking up the data for a particular user name.
   *    This handle is meaningful to the database and is intended to be more 
   *    convenient than the full user name. This handle was determined when the 
   *    user name and other data was added to the database. The actual User 
   *    Number will not be known on the master when remote Update Key 
   *    distribution is used until the outstation assigns one and informs 
   *    the master.
   *  userNumber - user number that has been selected by the outstation
   *   to be used for this user and it's update key.
   */ 
  void TMWDEFS_GLOBAL mdnpdata_authStoreUpdKeyChangeReply(
    void            *pHandle,
    void            *userNameDbHandle,
    TMWTYPES_USHORT  userNumber,
    TMWTYPES_UCHAR  *pOSData,
    TMWTYPES_USHORT  OSDataLength);

  /* function: mdnpdata_authGetUpdateKeyData
   * purpose:  Get the encrypted data generated by the Authority to be sent to the outstation in a g120v13 message
 
   * arguments:  
   *  pHandle - handle to database returned from mdnpdata_init  
   *  userNameDbHandle - handle for looking up the data for a particular user name.
   *    This handle is meaningful to the database and is intended to be more 
   *    convenient than the full user name. This handle was determined when the 
   *    user name and other data was added to the database. The actual User 
   *    Number will not be known on the master when remote Update Key 
   *    distribution is used until the outstation assigns one and informs 
   *    the master. 
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpdata_authGetUpdateKeyData(
    void            *pHandle,
    void            *userNameDbHandle,
    TMWTYPES_UCHAR  *pOSData,
    TMWTYPES_USHORT *pOSDataLength);

  /* function: mdnpdata_authDeleteUser
   * purpose:  Delete this auth user from the database. A command has been sent to
   *  the outstation to delete this user and a response indicating success has been
   *  received back from the outstation.
   * arguments:  
   *  pHandle - handle to database returned from mdnpdata_init  
   *  userNameDbHandle - handle for looking up the data for a particular user name.
   *    This handle is meaningful to the database and is intended to be more 
   *    convenient than the full user name. This handle was determined when the 
   *    user name and other data was added to the database. The actual User 
   *    Number will not be known on the master when remote Update Key 
   *    distribution is used until the outstation assigns one and informs 
   *    the master.
   *  userNumber - user number for this handle if one is known. 
   *               zero otherwise.
   */
  TMWTYPES_USHORT TMWDEFS_GLOBAL mdnpdata_authDeleteUser(
    void            *pHandle,
    void            *userNameDbHandle);
#endif

  /* function: mdnpdata_authStoreSecStat 
   * purpose: Store master generated statistic value
   * arguments:
   *  pHandle - handle to database returned from mdnpdata_init
   *  index - index indicating which statistic
   *   for example DNPAUTH_UNEXPECTED_MSG_INDEX
   *  value - value of statistic
   */
  void TMWDEFS_GLOBAL mdnpdata_authStoreSecStat(
    void *pHandle,
    TMWTYPES_USHORT index,
    TMWTYPES_ULONG value);

  /* function: mdnpdata_authStoreOSSecStat 
   * purpose:  Store outstation generated statistic value that was
   *  received from the outstation.
   * arguments:
   *  pHandle - handle to database returned from mdnpdata_init
   *  pointNumber - point Number recveived. Can be converted to index 
   *   indicating which statistic, depending on which association this
   *   was received on. 
   *  value - value of statistic 
   *  flags - DNP3 flags to store. Flag value is OR'd
   *    combinations of the following:
   *      DNPDEFS_DBAS_FLAG_ON_LINE - the binary input point has been read
   *        successfully
   *      DNPDEFS_DBAS_FLAG_RESTART - the field device that originated the
   *        data object has been restarted. This device may be the deviced
   *        reporting this data object.
   *      DNPDEFS_DBAS_FLAG_COMM_LOST - the device reporting this data object
   *        has lost communication with the originator of the data object
   *      DNPDEFS_DBAS_FLAG_REMOTE_FORCED - the state of the binary object
   *        has been forced to its current state at the originating device
   *      DNPDEFS_DBAS_FLAG_LOCAL_FORCED - the state of the binary object
   *        has been forced to its current state at the device reporting
   *        this data object
   *      DNPDEFS_DBAS_FLAG_CNTR_ROLLOVER - the accumulated value has exceeded
   *        has exceeded its maximum and rolled over to zero. 
   *      DNPDEFS_DBAS_FLAG_DISCONTINUITY - value cannot be compared against 
   *        a prior value to obtain the correct count difference
   *  isEvent - TMWDEFS_TRUE if this update was caused by a change event
   *  pTimeStamp - pointer to time stamp reported with this change event
   *   or TMWDEFS_NULL if no time was reported. Will always be TMWDEFS_NULL
   *   if isEvent is TMWDEFS_FALSE.
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpdata_authStoreOSSecStat(
    void *pHandle,
    TMWTYPES_USHORT association,
    TMWTYPES_USHORT pointNumber,
    TMWTYPES_ULONG value,
    TMWTYPES_UCHAR flags,
    TMWTYPES_BOOL isEvent,
    TMWDTIME *pTimeStamp);
  
  /* function: mdnpdata_authLogMaxRekeyTCPClose 
   * purpose:  log that Rekeys Due to Authentication Failure statistic 
   *  has exceeded Max Authentication Rekeys and since this is TCP
   *  the connection will be closed.
   * arguments:   
   *  pHandle - database handle returned from mdnpdata_init 
   * returns: 
   */  
  void TMWDEFS_GLOBAL mdnpdata_authLogMaxRekeyTCPClose(
    void            *pHandle);

  /* function: mdnpdata_authLogUnexpectedMsg
   * purpose:  log that an unexpected message was received as indicated
   *  in SA Spec V4 Table 14
   * arguments:   
   *  pHandle - database handle returned from mdnpdata_init 
   *  state - current master state ie MDNPAUTH_STATE_XXX
   *  event - event being processes ie MDNPAUTH_EVT_XX
   *  pRxFragment - pointer to message that was received.
   * returns: 
   */  
  void TMWDEFS_GLOBAL mdnpdata_authLogUnexpectedMsg(
    void            *pHandle, 
    TMWTYPES_UCHAR   state, 
    TMWTYPES_ULONG   event, 
    TMWSESN_RX_DATA *pRxFragment);

  /* function: mdnpdata_authLogFailedUpdateKey
   * purpose:  log that a Update Key Change failed as indicated in SA Spec V4
   *  Table 14
   * arguments:   
   *  pHandle - database handle returned from mdnpdata_init 
   *  state - current master state ie MDNPAUTH_STATE_XXX
   *  event - event being processes ie MDNPAUTH_EVT_XX
   * returns: 
   */ 
  void TMWDEFS_GLOBAL mdnpdata_authLogFailedUpdateKey(
    void            *pHandle, 
    TMWTYPES_UCHAR   state, 
    TMWTYPES_ULONG   event);

#endif

  /* function: mdnpdata_authLogTx
   * purpose:  log the Secure Authentication message that is sent to 
   *  Outstation.
   *  mdnpdata_authLogErrorTx will be called for error messages (var 7)
   * arguments:   
   *  pHandle - database handle returned from mdnpdata_init
   *  variation - variation of message sent (object group 120).
   *  userNumber - user number transmitted if applicable, 0 otherwise.
   *  sequenceNumber - sequence number sent if applicable, 0 otherwise.
   *  pMsgBuf - pointer to message being sent.
   *  msgLength - length of message being sent.
   * returns: 
   *  void
   */ 
  void TMWDEFS_GLOBAL mdnpdata_authLogTx(
    void            *pHandle, 
    TMWTYPES_UCHAR   variation,
    TMWTYPES_USHORT  userNumber,
    TMWTYPES_ULONG   sequenceNumber,
    TMWTYPES_UCHAR  *pMsgBuf,
    TMWTYPES_USHORT  msgLength); 

  /* function: mdnpdata_authLogRx
   * purpose:  log the Secure Authentication message that was received 
   *  from Outstation
   *  mdnpdata_authLogErrorRx will be called for error messages (var 7)
   * arguments:   
   *  pHandle - database handle returned from mdnpdata_init
   *  variation - variation of message received (object group 120).
   *  userNumber - user number received
   *  sequenceNumber - sequence number received
   *  pMsgBuf - pointer to message received.
   *  msgLength - length of message received.
   * returns: 
   *  void
   */ 
  void TMWDEFS_GLOBAL mdnpdata_authLogRx(
    void            *pHandle, 
    TMWTYPES_UCHAR   variation,
    TMWTYPES_USHORT  userNumber,
    TMWTYPES_ULONG   sequenceNumber,
    TMWTYPES_UCHAR  *pMsgBuf,
    TMWTYPES_USHORT  msgLength); 
 
  /* function: mdnpdata_authLogErrorTx
   * purpose:  log an error condition occurred for a particular user number and 
   *  whether an error messsage is being sent to the outstation. It will not be 
   *  sent if the max error count has been exceeded.
   *  mdnpdata_authLogTx will not be called for error messages.
   * arguments:   
   *  pHandle - database handle returned from mdnpdata_init
   *  userNumber - user number transmitted
   *  assocId - Association Id identifying which master/outstation association 
   *  sequenceNumber - challenge sequence number transmitted
   *  errorCode - error code from error message
   *  pTimeStamp - pointer to time stamp transmitted
   *  pMsgBuf - pointer to error message being sent
   *  msgLength - length of msg
   *  msgSent - indicates if an error message was sent to the outstation. When count
   *   exceeds max, error message will not be sent.
   * returns: 
   *  void
   */ 
  void TMWDEFS_GLOBAL mdnpdata_authLogErrorTx(
    void           *pHandle, 
    TMWTYPES_USHORT userNumber,
    TMWTYPES_USHORT assocId,
    TMWTYPES_ULONG  sequenceNumber,
    TMWTYPES_UCHAR  errorCode,
    TMWDTIME       *pTimeStamp,
    TMWTYPES_CHAR  *pMsgBuf,
    TMWTYPES_USHORT msgLength,
    TMWTYPES_BOOL   msgSent);
       
  /* function: mdnpdata_authLogErrorRx
   * purpose:  store g120v7 error data received from an outstation
   *  mdnpdata_authLogRx will not be called for error messages.
   * arguments:    
   *  pHandle - handle to database returned from mdnpdata_init
   *  userNumber - user number from message
   *  assocId - association id indicating which master/outstation association
   *  sequenceNumber - Sequence number from error message
   *  errorCode - error identifier
   *  pTimeStamp - time stamp from message
   *  pMsgBuf - pointer to error message received
   *  msgLength - length of message being received
   * returns: 
   */ 
  void TMWDEFS_GLOBAL mdnpdata_authLogErrorRx(
    void            *pHandle, 
    TMWTYPES_USHORT  userNumber,
    TMWTYPES_USHORT  assocId,
    TMWTYPES_ULONG   sequenceNumber,
    TMWTYPES_UCHAR   errorCode,
    TMWDTIME        *pTimeStamp,
    TMWTYPES_UCHAR  *pMsgBuf,
    TMWTYPES_USHORT  msgLength);

  /* function: mdnpdata_authEvent
   * purpose: Indicates to database that a specific Secure Authentication
   *  event has occurred.
   * arguments:    
   *  pHandle - handle to database returned from mdnpdata_init
   *  userNumber - user number that was involved if appropriate, 
   *               0 if not user specific.
   *  event - event that occurred.
   *          MDNPAUTH_EV_V4_BAD_FUNCTION-master received IIN2-0 Function Code Not 
   *            Implemented in response to Read Status g120v4 indicating 
   *            outstation does not support Secure Authentication.
   *          MDNPAUTH_EV_V10_BAD_FUNCTION-master received IIN 2-2 Function Code Not
   *            Implemented in response to change user status g120v10 indicating
   *            outstation does not support Secure Authentication Remote Key Update.
   *            
   * returns: 
   *  void
   */
  void TMWDEFS_GLOBAL mdnpdata_authEvent(
    void                  *pHandle, 
    TMWTYPES_USHORT       userNumber,
    MDNPAUTH_EVENT_ENUM   event);

#if MDNPCNFG_SUPPORT_SA_VERSION2
  /* The following functions only need to be implemented for DNP3 Secure Authentication Version 2 
   * These function will NOT be called if ONLY SA V5 is supported
   * If support for BOTH Version 2 and Version 5 is required, the following functions WILL be called.
   * See the SCL User Manual for details of how these relate to the Version 5 functions in tmwcrypto.h/c
   */

  /* function: mdnpdata_authLogKeyStatRqTx
   * purpose:  log that a key status request is being sent to the outstation
   *    NOTE: Only required for SA_VERSION2
   * arguments:   
   *  pHandle - database handle returned from mdnpdata_init
   *  userNumber - user number transmitted
   * returns: 
   *  void
   */ 
  void TMWDEFS_GLOBAL mdnpdata_authLogKeyStatRqTx(
    void            *pHandle, 
    TMWTYPES_USHORT  userNumber); 

  /* function: mdnpdata_authLogKeyChange
   * purpose:  log that a key change request is being sent to the outstation
   *    NOTE: Only required for SA_VERSION2
   * arguments:   
   *  pHandle - database handle returned from mdnpdata_init
   *  userNumber - user number transmitted
   *  sequenceNumber - key change sequence number transmitted
   * returns: 
   *  void
   */ 
  void TMWDEFS_GLOBAL mdnpdata_authLogKeyChangeTx(
    void            *pHandle, 
    TMWTYPES_USHORT  userNumber,
    TMWTYPES_ULONG   sequenceNumber);
  
  /* function: mdnpdata_authLogAggrTx
   * purpose:  log that a aggressive mode request is being sent to the outstation
   *    NOTE: Only required for SA_VERSION2
   * arguments:   
   *  pHandle - database handle returned from mdnpdata_init
   *  userNumber - user number transmitted
   *  sequenceNumber - challenge sequence number transmitted
   * returns: 
   *  void
   */ 
  void TMWDEFS_GLOBAL mdnpdata_authLogAggrTx(
    void            *pHandle, 
    TMWTYPES_USHORT  userNumber,
    TMWTYPES_ULONG   sequenceNumber);

  /* function: mdnpdata_authLogChallRplyTx
   * purpose:  log that a challenge reply is being sent to the outstation
   *    NOTE: Only required for SA_VERSION2
   * arguments:   
   *  pHandle - database handle returned from mdnpdata_init
   *  userNumber - user number transmitted
   *  sequenceNumber - challenge sequence number transmitted
   * returns: 
   *  void
   */ 
  void TMWDEFS_GLOBAL mdnpdata_authLogChallRplyTx(
    void            *pHandle, 
    TMWTYPES_USHORT  userNumber,
    TMWTYPES_ULONG   sequenceNumber);
  
  /* function: mdnpdata_authLogKeyStatusRx
   * purpose:  log that a key status request has been received from an outstation 
   *    NOTE: Only required for SA_VERSION2
   * arguments:   
   *  pHandle - database handle returned from mdnpdata_init
   *  userNumber - user number from message
   *  sequenceNumber -  Sequence number from message
   *  keyWrapAlgorithm - Key Wrap Algorithm from message
   *  keyStatus - Key Status from message
   *  macAlgorithm - MAC Algorithm number from message
   * returns: 
   */ 
  void TMWDEFS_GLOBAL mdnpdata_authLogKeyStatusRx(
    void            *pHandle, 
    TMWTYPES_USHORT  userNumber,
    TMWTYPES_ULONG   sequenceNumber,
    TMWTYPES_UCHAR   keyWrapAlgorithm,
    TMWTYPES_UCHAR   keyStatus,
    TMWTYPES_UCHAR   macAlgorithm); 

   /* function: mdnpdata_authLogChallRx
   * purpose:  log that a challenge request has been received from an outstation
   *    NOTE: Only required for SA_VERSION2
   * arguments:  
   *  pHandle - database handle returned from mdnpdata_init
   *  userNumber - user number from message
   *  sequenceNumber -  Challenge Sequence Number from message 
   *  macAlgorithm - MAC Algorithm number from message
   *  reasonForChallenge - reason for challenge. Authentication Spec currently
   *   only defines CRITICAL==1.
   * returns: 
   */ 
  void TMWDEFS_GLOBAL mdnpdata_authLogChallRx(
    void            *pHandle, 
    TMWTYPES_USHORT  userNumber,
    TMWTYPES_ULONG   sequenceNumber,
    TMWTYPES_UCHAR   macAlgorithm,
    TMWTYPES_UCHAR   reasonForChallenge); 
  
   /* function: mdnpdata_authLogChallRplyRx
   * purpose:  log that a challenge reply has been received from an outstation
   *    NOTE: Only required for SA_VERSION2
   * arguments:   
   *  pHandle - database handle returned from mdnpdata_init
   *  userNumber - user number from message
   *  sequenceNumber -  Challenge Sequence Number from message 
   *  status - TMWDEFS_TRUE if this message was authenticated properly
   *   TMWDEFS_FALSE otherwise
   * returns: 
   */ 
  void TMWDEFS_GLOBAL mdnpdata_authLogChallRplyRx(
    void            *pHandle, 
    TMWTYPES_USHORT  userNumber,
    TMWTYPES_ULONG   sequenceNumber,
    TMWTYPES_BOOL    status);
       
   /* function: mdnpdata_authLogAggrRx
   * purpose:  log that an aggressive mode response has been received from an 
   *    NOTE: Only required for SA_VERSION2
   *  outstation 
   * arguments:   
   *  pHandle - database handle returned from mdnpdata_init
   *  userNumber - user number from message
   *  sequenceNumber -  Challenge Sequence Number from message 
   *  status - TMWDEFS_TRUE if this message was authenticated properly
   *   TMWDEFS_FALSE otherwise 
   * returns: 
   */ 
   void TMWDEFS_GLOBAL mdnpdata_authLogAggrRx(
    void            *pHandle, 
    TMWTYPES_USHORT  userNumber,
    TMWTYPES_ULONG   sequenceNumber,
    TMWTYPES_BOOL    status);

  /* function: mdnpdata_authStoreErrorRx
   * purpose:  store g120v7 error data received from an outstation
   *    NOTE: Only required for SA_VERSION2
   * arguments:    
   *  pHandle - handle to database returned from mdnpdata_init
   *  userNumber - user number from message
   *  assocId - association id indicating which master/outstation association
   *  sequenceNumber - Sequence number from error message
   *  errorCode - error identifier
   *  pTimeStamp - time stamp from message
   *  pErrorText - pointer to optional error text
   *  errorTextLength - length of optional error text.
   * returns: 
   */ 
  void TMWDEFS_GLOBAL mdnpdata_authStoreErrorRx(
    void            *pHandle, 
    TMWTYPES_USHORT  userNumber,
    TMWTYPES_USHORT  assocId,
    TMWTYPES_ULONG   sequenceNumber,
    TMWTYPES_UCHAR   errorCode,
    TMWDTIME        *pTimeStamp,
    TMWTYPES_CHAR   *pErrorText,
    TMWTYPES_USHORT  errorTextLength); 

  /* function: mdnpdata_authGetNewSessionKeys 
   * purpose:  This function should create new session keys to be used for 
   *  authentication. These keys are sent to the outstation in a key change 
   *  request. These keys will be changed frequently, every 15 minutes or 
   *  so by default. These keys should random and generated, not configured.
   *    NOTE: Only required for SA_VERSION2, see utils/tmwcrypto.h for SA_VERSION5
   * arguments:
   *  pHandle - database handle returned from mdnpdata_init
   *  pControlSessionKey - pointer to control direction session key structure 
   *   to be filled in
   *  pMonitorSessionKey - pointer to monitor direction session key structure 
   *   to be filled in
   * returns:
   *  TMWDEFS_TRUE if successful
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL mdnpdata_authGetNewSessionKeys(
    void             *pHandle,
    DNPDATA_AUTH_KEY *pControlSessionKey,
    DNPDATA_AUTH_KEY *pMonitorSessionKey);

  /* function: mdnpdata_authKeyWrapSupport 
   * purpose:  This function should determine whether the key wrap algorithm 
   *   requested by the outstation is supported 
   *    NOTE: Only required for SA_VERSION2, see utils/tmwcrypto.h for SA_VERSION5
   * arguments: 
   *  keyWrapAlgorithm - key wrap algorithm
   *   DNPAUTH_KEYWRAP_AES128 is the only one currently specified.
   * returns:
   *  TMWDEFS_TRUE if supported
   *  TMWDEFS_FALSE otherwise
   */
  TMWTYPES_BOOL mdnpdata_authKeyWrapSupport(
    TMWTYPES_UCHAR  keyWrapAlgorithm);
  
  /* function: mdnpdata_authEncryptKeyWrapData 
   * purpose:  Encrypt the key data using the update key for the indicated 
   *   session database. Data should be padded as required by encryption
   *   algorithm. Return the encrypted data in pValue setting *pLength to
   *   the length of the returned data
   *   Care should be take to protect the security of the Update Key. 
   *   Encrypting it and/or limiting who can get or set this key is very 
   *   important.
   *    NOTE: Only required for SA_VERSION2, see utils/tmwcrypto.h for SA_VERSION5
   * arguments:
   *  pHandle - database handle returned from mdnpdata_init
   *  userNumber - user number
   *  algorithm - Encryption algorithm to use
   *   DNPAUTH_KEYWRAP_AES128 is the only one currently specified.
   *   other values reserved for future use or vendor specific choices 
   *   NOTE: The AES Key Wrap Algorithm specified in RFC3394 is not the same 
   *    thing as AES encryption. The key wrap algorithm actually will call 
   *    the AES encryption function multiple times depending on the length
   *    the key data to be encrypted.
   *  pPlainValue - pointer to data to be encrypted
   *  plainDataLength - length of data to be encrypted, this will NOT be padded 
   *   out to 8 bytes. The AES128 key wrap algorithm expects multiples of 8 bytes.
   *    It does not matter what byte values are used for padding.
   *  pEncryptedData - where to copy the encrypted data
   *  pEncryptedLength - when called this is the maximum length allowed for 
   *    ecnrypted data, on return this should be set to the length of the 
   *    encrypted data.
   * returns:
   *  TMWDEFS_TRUE if successful
   *  TMWDEFS_FALSE otherwise
   */
  TMWTYPES_BOOL mdnpdata_authEncryptKeyWrapData(
    void            *pHandle, 
    TMWTYPES_USHORT  userNumber,
    TMWTYPES_UCHAR   keyWrapAlgorithm,
    TMWTYPES_UCHAR  *pPlainData, 
    TMWTYPES_USHORT  plainDataLength, 
    TMWTYPES_UCHAR  *pEncryptedData,
    TMWTYPES_USHORT *pEncryptedLength);

  /* function: mdnpdata_authHMACSupport 
   * purpose:  This function should determine whether the MAC algorithm 
   *   requested by the outstation is supported and return the length of
   *   the MAC data requested by this algorithm
   *    NOTE: Only required for SA_VERSION2, see utils/tmwcrypto.h for SA_VERSION5
   * arguments: 
   *  HHMACAlgorithm - HMAC algorithm
   *   DNPAUTH_HMAC_SHA1_4OCTET  Only for SA V2.
   *   DNPAUTH_HMAC_SHA1_8OCTET
   *   DNPAUTH_HMAC_SHA1_10OCTET
   *   DNPAUTH_HMAC_SHA256_8OCTET     
   *   DNPAUTH_HMAC_SHA256_16OCTET 
   *   other values reserved for future use or vendor specific choices
   * returns: 
   *  length of data to be generated if algorithm is supported.
   *  0 if algorithm is not supported
   */
  TMWTYPES_CHAR mdnpdata_authHMACSupport(
    TMWTYPES_UCHAR HMACAlgorithm);

  /* function: mdnpdata_authHMACValue 
   * purpose: Using the specified algorithm and key calculate the MAC value.
   * Copy up to the number of bytes allowed by *pMACValueLength into *pMACValue
   * and set *pHMACValueLength to the number of bytes copied.
   *    NOTE: Only required for SA_VERSION2, see utils/tmwcrypto.h for SA_VERSION5
   * arguments:
   *  algorithm - algorithm to use for creating hash value
   *   DNPAUTH_HMAC_SHA1_4OCTET  Only for SA V2.
   *   DNPAUTH_HMAC_SHA1_8OCTET
   *   DNPAUTH_HMAC_SHA1_10OCTET
   *   DNPAUTH_HMAC_SHA256_8OCTET     
   *   DNPAUTH_HMAC_SHA256_16OCTET 
   *   other values reserved for future use or vendor specific choices
   *  pKey - key to use 
   *  pData - pointer to data to hash
   *  dataLength - length of data to hash
   *  pHMACValue - pointer where hashed data should be copied
   *  pHMACValueLength - when called this is the maximum length allowed for 
   *   hashed data, on return this should be set to the length of the hashed data.
   * returns:
   *  TMWDEFS_TRUE if successful
   *  TMWDEFS_FALSE otherwise
   */
  TMWTYPES_BOOL mdnpdata_authHMACValue(
    TMWTYPES_UCHAR    algorithm,
    DNPDATA_AUTH_KEY *pKey,
    TMWTYPES_UCHAR   *pData,
    TMWTYPES_ULONG    dataLength,
    TMWTYPES_UCHAR   *pHMACValue,
    TMWTYPES_USHORT  *pHMACValueLength);

  /* function: mdnpdata_authRandomChallengeData
   * purpose:  generate pseudo-random data, at least 4 bytes long, using algorithm 
   *   specified in FIPS 186-2 Digital Signal Standard 
   *    NOTE: Only required for SA_VERSION2, see utils/tmwcrypto.h for SA_VERSION5
   * arguments:  
   *  pBuf - pointer to where random data should be copied 
   *  minLength - minimum length of data as required by spec
   *  pLength -  when called this is the maximum length allowed for the random data,
   *   on return this should be set to the length of the random data.
   * returns:
   *  TMWDEFS_TRUE of successful
   *  TMWDEFS_FALSE otherwise 
   */ 
  TMWTYPES_BOOL TMWDEFS_LOCAL mdnpdata_authRandomChallengeData(
    TMWTYPES_UCHAR  *pBuf,
    TMWTYPES_USHORT  minLength,
    TMWTYPES_USHORT *pLength);

#endif
   
#ifdef __cplusplus
}
#endif
#endif /* MDNPDATA_DEFINED */
