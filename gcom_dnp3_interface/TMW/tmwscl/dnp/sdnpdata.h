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

/* file: sdnpdata.h
 * description: This file defines the interface between the TMW DNP3 slave
 *  SCL and the target database. The user should modify the corresponding
 *  implementations in sdnpdata.c as required to access the target database.
 *
 * The DNP3 specification states that point numbers for each object type 
 *  should start at 0 and proceed sequentially to N-1 where N is the number 
 *  of points of that type. Hence the TMW SCL assumes that the point index 
 *  and point number are equivalent and will frequently loop from 0 to N-1 
 *  when processing requests with the 'All Points' qualifier.
 *
 * Although not recommended by the DNP3 specification, it is sometimes 
 *  desirable to 'disable' specific points in the DNP3 point map creating
 *  gaps in the DNP3 point map. This is supported by the TMW SCL by returning
 *  TMWDEFS_NULL from the appropriate sdnpdata_xxxGetPoint subroutine defined
 *  below.
 *
 * Routines are provided below to access data in a DNP3 slave device. A set 
 *  of routines are provided for each data type based on the requirements of
 *  that data type. The following generic descriptions apply to corresponding
 *  data type specific subroutines.
 *
 * sdnpdata_xxxQuantity - This routine returns the number of data points of
 *  of the corresponding type in the specified database. This includes ALL
 *  of the data points in the slave DNP3 database whether they are currently
 *  enabled or disabled (i.e., the quantity is one more than the highest point
 *  number).
 *
 * sdnpdata_xxxGetPoint - Return a pointer to a handle that will be used to
 *  access information and values for the specified point. This routine should
 *  return TMWDEFS_NULL if the specified data point is currently disabled.
 *
 * sdnpdata_xxxRead - For data types that support reading their current value
 *  this subroutine is used to read the current value from the specified data
 *  point.
 *
 * sdnpdata_xxxWrite - For data types that support writing this subroutine
 *  is used to write a new value to the specified data point.
 *
 * sdnpdata_xxxChanged - For data types that support the generation of change
 *  events this subroutine is used to periodically scan for changes. These will
 *  be called if xxxScanPeriod is set to a nonzero value.
 *
 * Several of the routines below return an 8 bit flags value in the memory
 *  location pointed to by the pFlags argument. This value should be an or'd
 *  combination of the appropriate DNPDEFS_DBAS_FLAG_XXX masks defined in
 *  dnpdefs.h. Note that not all flags are valid for all objects/variations.
 *  Valid flags for each object/variation are listed in the header comments
 *  for each function.
 */
#ifndef SDNPDATA_DEFINED
#define SDNPDATA_DEFINED

#include "tmwscl/utils/tmwdefs.h"
#include "tmwscl/utils/tmwtypes.h"
#include "tmwscl/utils/tmwdtime.h"
#include "tmwscl/dnp/dnpcnfg.h"
#include "tmwscl/dnp/dnpdata.h"
#include "tmwscl/dnp/dnpauth.h"

/* Define constants */

/* Beginning in version 3.29.0 LEVEL_TMW and LEVEL_4 are set to 
 * TMWDEFS_FALSE by default.
 */

/* These defines give an idea of what functionality is defined at each 
 * DNP3 Level according to the specification. LEVEL_TMW indicates functionality 
 * beyond Level 4 that WAS compiled in by default in the SDNP SCL before 
 * version 3.29
 * There is additional beyond Level 4 functionality that was compiled out by
 * default.
 * You can set TMW to FALSE to disable some default functionality, or you can 
 * set individual items below to FALSE. If you want to limit functionality to a 
 * Level you should set that define and lower to TRUE and higher Level defines 
 * to FALSE. For example to limit functionality to LEVEL2 you should leave 
 * LEVEL1 and LEVEL2 TRUE and set LEVEL3, LEVEL4 and LEVEL_TMW to FALSE. 
 * You may still choose to set individual items to TRUE or FALSE below. 
 * Some functionality is dependent on other so you may have to experiment. 
 * For example frozen counters depend on binary counters being supported.
 * 
 * If you are a .NET Components customer there is no longer a separate 
 * config/tmwprvt.h file included from tmwcnfg.h which added functionality
 * not compiled in by default for the ANSI C SCL. This simplifies choosing the
 * functionality a device should support. It is important to select the exact 
 * set of features desired, test them thoroughly, and then document them in
 * your Device Profile. 
 */
#define SDNPDATA_CNFG_LEVEL1     TMWDEFS_TRUE
#define SDNPDATA_CNFG_LEVEL2     TMWDEFS_TRUE
#define SDNPDATA_CNFG_LEVEL3     TMWDEFS_TRUE
#ifndef SDNPDATA_CNFG_LEVEL4
#define SDNPDATA_CNFG_LEVEL4     TMWDEFS_TRUE
#endif

/* Note in v3.29.0 File Transfer Object Group 70 was removed from LEVEL_TMW
 * to require this functionality to be intentionally incuded.
 */
#ifndef SDNPDATA_CNFG_LEVEL_TMW
#define SDNPDATA_CNFG_LEVEL_TMW  TMWDEFS_FALSE
#endif

#if SDNPDATA_CNFG_LEVEL4
#define SDNPDATA_CNFG_SUPPORT_FLOAT  TMWCNFG_SUPPORT_FLOAT
#define SDNPDATA_CNFG_SUPPORT_DOUBLE TMWCNFG_SUPPORT_DOUBLE
#else
#define SDNPDATA_CNFG_SUPPORT_FLOAT  TMWDEFS_FALSE
#define SDNPDATA_CNFG_SUPPORT_DOUBLE TMWDEFS_FALSE
#endif

/* Device Attributes */
#ifndef SDNPDATA_SUPPORT_OBJ0
#define SDNPDATA_SUPPORT_OBJ0    TMWDEFS_FALSE
#endif

/* Binary Inputs */
/* If you support for Variation 1, you should also provide support
 * for variation 2. Otherwise if flags are not nominal it won't be
 * possible to send back the flags as required by Technical Bulletin
 * TB 2003-002 Object Flags
 */
#define SDNPDATA_SUPPORT_OBJ1_V1 SDNPDATA_CNFG_LEVEL1
#define SDNPDATA_SUPPORT_OBJ1_V2 SDNPDATA_CNFG_LEVEL1
#define SDNPDATA_SUPPORT_OBJ1 \
  (SDNPDATA_SUPPORT_OBJ1_V1 | \
   SDNPDATA_SUPPORT_OBJ1_V2)

/* Binary Input Events */
#define SDNPDATA_SUPPORT_OBJ2_V1 SDNPDATA_CNFG_LEVEL1
#define SDNPDATA_SUPPORT_OBJ2_V2 SDNPDATA_CNFG_LEVEL1
#define SDNPDATA_SUPPORT_OBJ2_V3 SDNPDATA_CNFG_LEVEL1
#define SDNPDATA_SUPPORT_OBJ2 \
  (SDNPDATA_SUPPORT_OBJ2_V1 | \
   SDNPDATA_SUPPORT_OBJ2_V2 | \
   SDNPDATA_SUPPORT_OBJ2_V3)

/* Double Bit Inputs */
#ifndef SDNPDATA_SUPPORT_OBJ3
#define SDNPDATA_SUPPORT_OBJ3_V1 SDNPDATA_CNFG_LEVEL4
#define SDNPDATA_SUPPORT_OBJ3_V2 SDNPDATA_CNFG_LEVEL4
#define SDNPDATA_SUPPORT_OBJ3 \
  (SDNPDATA_SUPPORT_OBJ3_V1 | \
   SDNPDATA_SUPPORT_OBJ3_V2)
#endif

/* Double bit Input Events */
#ifndef SDNPDATA_SUPPORT_OBJ4
#define SDNPDATA_SUPPORT_OBJ4_V1 SDNPDATA_CNFG_LEVEL4
#define SDNPDATA_SUPPORT_OBJ4_V2 SDNPDATA_CNFG_LEVEL4
#define SDNPDATA_SUPPORT_OBJ4_V3 SDNPDATA_CNFG_LEVEL4
#define SDNPDATA_SUPPORT_OBJ4 \
  (SDNPDATA_SUPPORT_OBJ4_V1 | \
   SDNPDATA_SUPPORT_OBJ4_V2 | \
   SDNPDATA_SUPPORT_OBJ4_V3)
#endif

/* Binary Output Status */
/* If you support for Variation 1, you should also provide support
 * for variation 2. Otherwise if flags are not nominal it won't be
 * possible to send back the flags as required by Technical Bulletin
 * TB 2003-002 Object Flags
 */
#define SDNPDATA_SUPPORT_OBJ10_V1 SDNPDATA_CNFG_LEVEL1
#define SDNPDATA_SUPPORT_OBJ10_WRITE SDNPDATA_CNFG_LEVEL1
#define SDNPDATA_SUPPORT_OBJ10_V2 SDNPDATA_CNFG_LEVEL1
#define SDNPDATA_SUPPORT_OBJ10 \
  (SDNPDATA_SUPPORT_OBJ10_V1 | \
   SDNPDATA_SUPPORT_OBJ10_V2)

/* If Binary Output Event support is not defined on the command line 
 * set the configuration here 
 */
#ifndef SDNPDATA_SUPPORT_OBJ11
/* Binary Output Events */
#define SDNPDATA_SUPPORT_OBJ11_V1 SDNPDATA_CNFG_LEVEL4
#define SDNPDATA_SUPPORT_OBJ11_V2 SDNPDATA_CNFG_LEVEL4
#define SDNPDATA_SUPPORT_OBJ11 \
  (SDNPDATA_SUPPORT_OBJ11_V1 | \
   SDNPDATA_SUPPORT_OBJ11_V2)
#endif

/* Control Relay Output Block */
#define SDNPDATA_SUPPORT_OBJ12_V1 SDNPDATA_CNFG_LEVEL1
#define SDNPDATA_SUPPORT_OBJ12_V2 SDNPDATA_CNFG_LEVEL4
#define SDNPDATA_SUPPORT_OBJ12_V3 SDNPDATA_CNFG_LEVEL4
#define SDNPDATA_SUPPORT_OBJ12 \
  (SDNPDATA_SUPPORT_OBJ12_V1 | \
   SDNPDATA_SUPPORT_OBJ12_V2 | \
   SDNPDATA_SUPPORT_OBJ12_V3)

/* If Binary Output Command Event support is not defined on the command line 
 * set the configuration here 
 */
#ifndef SDNPDATA_SUPPORT_OBJ13
/* Binary Output Command Events */
#define SDNPDATA_SUPPORT_OBJ13_V1 SDNPDATA_CNFG_LEVEL4
#define SDNPDATA_SUPPORT_OBJ13_V2 SDNPDATA_CNFG_LEVEL4
#define SDNPDATA_SUPPORT_OBJ13 \
  (SDNPDATA_SUPPORT_OBJ13_V1 | \
   SDNPDATA_SUPPORT_OBJ13_V2)
#endif

/* Binary Counters */
/* If you support for Variation 5, you should also provide support
 * for variation 1. Otherwise if flags are not nominal it won't be
 * possible to send back the flags as required by Technical Bulletin
 * TB 2003-002 Object Flags.
 * For the same reason if Variation 6 is supported, Variation 2 should be
 * supported.
 */
#define SDNPDATA_SUPPORT_OBJ20_V1 SDNPDATA_CNFG_LEVEL1
#define SDNPDATA_SUPPORT_OBJ20_V2 SDNPDATA_CNFG_LEVEL1
#define SDNPDATA_SUPPORT_OBJ20_V5 SDNPDATA_CNFG_LEVEL1
#define SDNPDATA_SUPPORT_OBJ20_V6 SDNPDATA_CNFG_LEVEL1
#define SDNPDATA_SUPPORT_OBJ20 \
  (SDNPDATA_SUPPORT_OBJ20_V1 | \
   SDNPDATA_SUPPORT_OBJ20_V2 | \
   SDNPDATA_SUPPORT_OBJ20_V5 | \
   SDNPDATA_SUPPORT_OBJ20_V6)

/* Frozen Counters */
/* If you support for Variation 9, you should also provide support
 * for variation 1. Otherwise if flags are not nominal it won't be
 * possible to send back the flags as required by Technical Bulletin
 * TB 2003-002 Object Flags.
 * For the same reason if Variation 10 is supported, Variation 2 should be
 * supported.
 */
#define SDNPDATA_SUPPORT_OBJ21_V1 SDNPDATA_CNFG_LEVEL2
#define SDNPDATA_SUPPORT_OBJ21_V2 SDNPDATA_CNFG_LEVEL2
#define SDNPDATA_SUPPORT_OBJ21_V5 SDNPDATA_CNFG_LEVEL2
#define SDNPDATA_SUPPORT_OBJ21_V6 SDNPDATA_CNFG_LEVEL2
#define SDNPDATA_SUPPORT_OBJ21_V9 SDNPDATA_CNFG_LEVEL2
#define SDNPDATA_SUPPORT_OBJ21_V10 SDNPDATA_CNFG_LEVEL2
#define SDNPDATA_SUPPORT_OBJ21 \
  (SDNPDATA_SUPPORT_OBJ21_V1 | \
   SDNPDATA_SUPPORT_OBJ21_V2 | \
   SDNPDATA_SUPPORT_OBJ21_V5 | \
   SDNPDATA_SUPPORT_OBJ21_V6 | \
   SDNPDATA_SUPPORT_OBJ21_V9 | \
   SDNPDATA_SUPPORT_OBJ21_V10)

/* Binary Counter Events */
#define SDNPDATA_SUPPORT_OBJ22_V1 SDNPDATA_CNFG_LEVEL1
#define SDNPDATA_SUPPORT_OBJ22_V2 SDNPDATA_CNFG_LEVEL1
#define SDNPDATA_SUPPORT_OBJ22_V5 SDNPDATA_CNFG_LEVEL_TMW
#define SDNPDATA_SUPPORT_OBJ22_V6 SDNPDATA_CNFG_LEVEL_TMW
#define SDNPDATA_SUPPORT_OBJ22 \
  (SDNPDATA_SUPPORT_OBJ22_V1 | \
   SDNPDATA_SUPPORT_OBJ22_V2 | \
   SDNPDATA_SUPPORT_OBJ22_V5 | \
   SDNPDATA_SUPPORT_OBJ22_V6)

/* Frozen Counter Events */
#define SDNPDATA_SUPPORT_OBJ23_V1 SDNPDATA_CNFG_LEVEL3
#define SDNPDATA_SUPPORT_OBJ23_V2 SDNPDATA_CNFG_LEVEL3
#define SDNPDATA_SUPPORT_OBJ23_V5 SDNPDATA_CNFG_LEVEL4
#define SDNPDATA_SUPPORT_OBJ23_V6 SDNPDATA_CNFG_LEVEL4
#define SDNPDATA_SUPPORT_OBJ23 \
  (SDNPDATA_SUPPORT_OBJ23_V1 | \
   SDNPDATA_SUPPORT_OBJ23_V2 | \
   SDNPDATA_SUPPORT_OBJ23_V5 | \
   SDNPDATA_SUPPORT_OBJ23_V6)

/* Analog Inputs */
/* If you support for Variation 3, you should also provide support
 * for variation 1. Otherwise if flags are not nominal it won't be
 * possible to send back the flags as required by Technical Bulletin
 * TB 2003-002 Object Flags.
 * For the same reason if Variation 4 is supported, Variation 2 should be
 * supported.
 */
#define SDNPDATA_SUPPORT_OBJ30_V1 SDNPDATA_CNFG_LEVEL1
#define SDNPDATA_SUPPORT_OBJ30_V2 SDNPDATA_CNFG_LEVEL1
#define SDNPDATA_SUPPORT_OBJ30_V3 SDNPDATA_CNFG_LEVEL1
#define SDNPDATA_SUPPORT_OBJ30_V4 SDNPDATA_CNFG_LEVEL1
#define SDNPDATA_SUPPORT_OBJ30_V5 SDNPDATA_CNFG_SUPPORT_FLOAT
#define SDNPDATA_SUPPORT_OBJ30_V6 SDNPDATA_CNFG_SUPPORT_DOUBLE
#define SDNPDATA_SUPPORT_OBJ30 \
  (SDNPDATA_SUPPORT_OBJ30_V1 | \
   SDNPDATA_SUPPORT_OBJ30_V2 | \
   SDNPDATA_SUPPORT_OBJ30_V3 | \
   SDNPDATA_SUPPORT_OBJ30_V4 | \
   SDNPDATA_SUPPORT_OBJ30_V5 | \
   SDNPDATA_SUPPORT_OBJ30_V6)

/* Frozen Analog Inputs */
#ifndef SDNPDATA_SUPPORT_OBJ31
#define SDNPDATA_SUPPORT_OBJ31_V1 TMWDEFS_FALSE
#define SDNPDATA_SUPPORT_OBJ31_V2 TMWDEFS_FALSE
#define SDNPDATA_SUPPORT_OBJ31_V3 TMWDEFS_FALSE
#define SDNPDATA_SUPPORT_OBJ31_V4 TMWDEFS_FALSE
#define SDNPDATA_SUPPORT_OBJ31_V5 TMWDEFS_FALSE
#define SDNPDATA_SUPPORT_OBJ31_V6 TMWDEFS_FALSE
#define SDNPDATA_SUPPORT_OBJ31_V7 TMWDEFS_FALSE
#define SDNPDATA_SUPPORT_OBJ31_V8 TMWDEFS_FALSE
#define SDNPDATA_SUPPORT_OBJ31 \
  (SDNPDATA_SUPPORT_OBJ31_V1 | \
   SDNPDATA_SUPPORT_OBJ31_V2 | \
   SDNPDATA_SUPPORT_OBJ31_V3 | \
   SDNPDATA_SUPPORT_OBJ31_V4 | \
   SDNPDATA_SUPPORT_OBJ31_V5 | \
   SDNPDATA_SUPPORT_OBJ31_V6 | \
   SDNPDATA_SUPPORT_OBJ31_V7 | \
   SDNPDATA_SUPPORT_OBJ31_V8)
#endif

/* Analog Input Events */
#define SDNPDATA_SUPPORT_OBJ32_V1 SDNPDATA_CNFG_LEVEL1
#define SDNPDATA_SUPPORT_OBJ32_V2 SDNPDATA_CNFG_LEVEL1
#define SDNPDATA_SUPPORT_OBJ32_V3 SDNPDATA_CNFG_LEVEL4
#define SDNPDATA_SUPPORT_OBJ32_V4 SDNPDATA_CNFG_LEVEL4
#define SDNPDATA_SUPPORT_OBJ32_V5 SDNPDATA_CNFG_SUPPORT_FLOAT
#define SDNPDATA_SUPPORT_OBJ32_V6 SDNPDATA_CNFG_SUPPORT_DOUBLE
#define SDNPDATA_SUPPORT_OBJ32_V7 SDNPDATA_CNFG_SUPPORT_FLOAT
#define SDNPDATA_SUPPORT_OBJ32_V8 SDNPDATA_CNFG_SUPPORT_DOUBLE
#define SDNPDATA_SUPPORT_OBJ32 \
  (SDNPDATA_SUPPORT_OBJ32_V1 | \
   SDNPDATA_SUPPORT_OBJ32_V2 | \
   SDNPDATA_SUPPORT_OBJ32_V3 | \
   SDNPDATA_SUPPORT_OBJ32_V4 | \
   SDNPDATA_SUPPORT_OBJ32_V5 | \
   SDNPDATA_SUPPORT_OBJ32_V6 | \
   SDNPDATA_SUPPORT_OBJ32_V7 | \
   SDNPDATA_SUPPORT_OBJ32_V8)

/* Frozen Analog Input Events */
#ifndef SDNPDATA_SUPPORT_OBJ33
#define SDNPDATA_SUPPORT_OBJ33_V1 TMWDEFS_FALSE
#define SDNPDATA_SUPPORT_OBJ33_V2 TMWDEFS_FALSE
#define SDNPDATA_SUPPORT_OBJ33_V3 TMWDEFS_FALSE
#define SDNPDATA_SUPPORT_OBJ33_V4 TMWDEFS_FALSE
#define SDNPDATA_SUPPORT_OBJ33_V5 TMWDEFS_FALSE
#define SDNPDATA_SUPPORT_OBJ33_V6 TMWDEFS_FALSE
#define SDNPDATA_SUPPORT_OBJ33_V7 TMWDEFS_FALSE
#define SDNPDATA_SUPPORT_OBJ33_V8 TMWDEFS_FALSE
#define SDNPDATA_SUPPORT_OBJ33 \
  (SDNPDATA_SUPPORT_OBJ33_V1 | \
   SDNPDATA_SUPPORT_OBJ33_V2 | \
   SDNPDATA_SUPPORT_OBJ33_V3 | \
   SDNPDATA_SUPPORT_OBJ33_V4 | \
   SDNPDATA_SUPPORT_OBJ33_V5 | \
   SDNPDATA_SUPPORT_OBJ33_V6 | \
   SDNPDATA_SUPPORT_OBJ33_V7 | \
   SDNPDATA_SUPPORT_OBJ33_V8)
#endif

/* Analog Input Deadband */
#define SDNPDATA_SUPPORT_OBJ34_V1 SDNPDATA_CNFG_LEVEL_TMW
#define SDNPDATA_SUPPORT_OBJ34_V2 SDNPDATA_CNFG_LEVEL_TMW
#define SDNPDATA_SUPPORT_OBJ34_V3 SDNPDATA_CNFG_SUPPORT_FLOAT
#define SDNPDATA_SUPPORT_OBJ34 \
  (SDNPDATA_SUPPORT_OBJ34_V1 | \
   SDNPDATA_SUPPORT_OBJ34_V2 | \
   SDNPDATA_SUPPORT_OBJ34_V3)

/* Analog Output Status */
#define SDNPDATA_SUPPORT_OBJ40_V1 SDNPDATA_CNFG_LEVEL1
#define SDNPDATA_SUPPORT_OBJ40_V2 SDNPDATA_CNFG_LEVEL1
#define SDNPDATA_SUPPORT_OBJ40_V3 SDNPDATA_CNFG_SUPPORT_FLOAT
#define SDNPDATA_SUPPORT_OBJ40_V4 SDNPDATA_CNFG_SUPPORT_DOUBLE
#define SDNPDATA_SUPPORT_OBJ40 \
  (SDNPDATA_SUPPORT_OBJ40_V1 | \
   SDNPDATA_SUPPORT_OBJ40_V2 | \
   SDNPDATA_SUPPORT_OBJ40_V3 | \
   SDNPDATA_SUPPORT_OBJ40_V4)

/* Analog Output Control Block */
#define SDNPDATA_SUPPORT_OBJ41_V1 SDNPDATA_CNFG_LEVEL3
#define SDNPDATA_SUPPORT_OBJ41_V2 SDNPDATA_CNFG_LEVEL1
#define SDNPDATA_SUPPORT_OBJ41_V3 SDNPDATA_CNFG_SUPPORT_FLOAT
#define SDNPDATA_SUPPORT_OBJ41_V4 SDNPDATA_CNFG_SUPPORT_DOUBLE
#define SDNPDATA_SUPPORT_OBJ41 \
  (SDNPDATA_SUPPORT_OBJ41_V1 | \
   SDNPDATA_SUPPORT_OBJ41_V2 | \
   SDNPDATA_SUPPORT_OBJ41_V3 | \
   SDNPDATA_SUPPORT_OBJ41_V4)

/* If Analog Output Event support is not defined on the command line 
 * set the configuration here 
 */
#ifndef SDNPDATA_SUPPORT_OBJ42
/* Analog Output Events */
#define SDNPDATA_SUPPORT_OBJ42_V1 SDNPDATA_CNFG_LEVEL4
#define SDNPDATA_SUPPORT_OBJ42_V2 SDNPDATA_CNFG_LEVEL4
#define SDNPDATA_SUPPORT_OBJ42_V3 SDNPDATA_CNFG_LEVEL4
#define SDNPDATA_SUPPORT_OBJ42_V4 SDNPDATA_CNFG_LEVEL4
#define SDNPDATA_SUPPORT_OBJ42_V5 SDNPDATA_CNFG_LEVEL4
#define SDNPDATA_SUPPORT_OBJ42_V6 SDNPDATA_CNFG_LEVEL4
#define SDNPDATA_SUPPORT_OBJ42_V7 SDNPDATA_CNFG_LEVEL4
#define SDNPDATA_SUPPORT_OBJ42_V8 SDNPDATA_CNFG_LEVEL4
#define SDNPDATA_SUPPORT_OBJ42 \
  (SDNPDATA_SUPPORT_OBJ42_V1 | \
   SDNPDATA_SUPPORT_OBJ42_V2 | \
   SDNPDATA_SUPPORT_OBJ42_V3 | \
   SDNPDATA_SUPPORT_OBJ42_V4 | \
   SDNPDATA_SUPPORT_OBJ42_V5 | \
   SDNPDATA_SUPPORT_OBJ42_V6 | \
   SDNPDATA_SUPPORT_OBJ42_V7 | \
   SDNPDATA_SUPPORT_OBJ42_V8)
#endif

/* If Analog Output Command Event support is not defined on the command line 
 * set the configuration here 
 */
#ifndef SDNPDATA_SUPPORT_OBJ43
/* Analog Output Command Events */
#define SDNPDATA_SUPPORT_OBJ43_V1 SDNPDATA_CNFG_LEVEL4
#define SDNPDATA_SUPPORT_OBJ43_V2 SDNPDATA_CNFG_LEVEL4
#define SDNPDATA_SUPPORT_OBJ43_V3 SDNPDATA_CNFG_LEVEL4
#define SDNPDATA_SUPPORT_OBJ43_V4 SDNPDATA_CNFG_LEVEL4
#define SDNPDATA_SUPPORT_OBJ43_V5 SDNPDATA_CNFG_LEVEL4
#define SDNPDATA_SUPPORT_OBJ43_V6 SDNPDATA_CNFG_LEVEL4
#define SDNPDATA_SUPPORT_OBJ43_V7 SDNPDATA_CNFG_LEVEL4
#define SDNPDATA_SUPPORT_OBJ43_V8 SDNPDATA_CNFG_LEVEL4
#define SDNPDATA_SUPPORT_OBJ43 \
  (SDNPDATA_SUPPORT_OBJ43_V1 | \
   SDNPDATA_SUPPORT_OBJ43_V2 | \
   SDNPDATA_SUPPORT_OBJ43_V3 | \
   SDNPDATA_SUPPORT_OBJ43_V4 | \
   SDNPDATA_SUPPORT_OBJ43_V5 | \
   SDNPDATA_SUPPORT_OBJ43_V6 | \
   SDNPDATA_SUPPORT_OBJ43_V7 | \
   SDNPDATA_SUPPORT_OBJ43_V8)
#endif

/* Time and Date */
#define SDNPDATA_SUPPORT_OBJ50_V1 SDNPDATA_CNFG_LEVEL1
/* Required for LAN Time Sync Procedure */
#define SDNPDATA_SUPPORT_OBJ50_V3 SDNPDATA_CNFG_LEVEL1

#ifndef SDNPDATA_SUPPORT_OBJ50_V4
/* Required for Indexed absolute time */
#define SDNPDATA_SUPPORT_OBJ50_V4 TMWDEFS_FALSE
#endif 

#define SDNPDATA_SUPPORT_OBJ50 \
  (SDNPDATA_SUPPORT_OBJ50_V1 | \
   SDNPDATA_SUPPORT_OBJ50_V3 | \
   SDNPDATA_SUPPORT_OBJ50_V4)

/* File Transfer */
#ifndef SDNPDATA_SUPPORT_OBJ70
/* Note: Starting in version 3.29.0 this is no longer set to LEVEL_TMW by default. 
 * You should only set this to TRUE if you intend to allow file access from an MDNP
 * device across a DNP Association.
 */
#define SDNPDATA_SUPPORT_OBJ70    TMWDEFS_FALSE
#endif

/* Read IIN bit support */
#define SDNPDATA_SUPPORT_OBJ80_READ SDNPDATA_CNFG_LEVEL3

/* If Data Set support is not defined on the command line 
 * set the configuration here 
 */
#ifndef SDNPDATA_SUPPORT_DATASETS

/* Data Set Prototype */
#define SDNPDATA_SUPPORT_OBJ85  TMWDEFS_FALSE

/* Data Set Descriptor */
#define SDNPDATA_SUPPORT_OBJ86_V1 TMWDEFS_FALSE
#define SDNPDATA_SUPPORT_OBJ86_V2 TMWDEFS_FALSE
#define SDNPDATA_SUPPORT_OBJ86_V3 TMWDEFS_FALSE
#define SDNPDATA_SUPPORT_OBJ86 \
  (SDNPDATA_SUPPORT_OBJ86_V1 | \
   SDNPDATA_SUPPORT_OBJ86_V2 | \
   SDNPDATA_SUPPORT_OBJ86_V3)

/* Data Set Present Value */
#define SDNPDATA_SUPPORT_OBJ87  TMWDEFS_FALSE

/* Data Set Snapshot Events */
#define SDNPDATA_SUPPORT_OBJ88  TMWDEFS_FALSE

/* Data Sets in general */
#define SDNPDATA_SUPPORT_DATASETS \
  (SDNPDATA_SUPPORT_OBJ85 | \
   SDNPDATA_SUPPORT_OBJ86 | \
   SDNPDATA_SUPPORT_OBJ87 | \
   SDNPDATA_SUPPORT_OBJ88)
#endif

/* FC 31 Activate Configuration response */
#ifndef SDNPDATA_SUPPORT_OBJ91
#define SDNPDATA_SUPPORT_OBJ91  SDNPDATA_CNFG_LEVEL_TMW
#endif

/* String Data */
#ifndef SDNPDATA_SUPPORT_OBJ110
#define SDNPDATA_SUPPORT_OBJ110 SDNPDATA_CNFG_LEVEL_TMW
#endif

/* String Events */
#ifndef SDNPDATA_SUPPORT_OBJ111
#define SDNPDATA_SUPPORT_OBJ111 SDNPDATA_CNFG_LEVEL_TMW
#endif

/* Virtual Terminal Output */
#ifndef SDNPDATA_SUPPORT_OBJ112
#define SDNPDATA_SUPPORT_OBJ112 SDNPDATA_CNFG_LEVEL_TMW
#endif

/* Virtual Terminal Events */
#ifndef SDNPDATA_SUPPORT_OBJ113
#define SDNPDATA_SUPPORT_OBJ113 SDNPDATA_CNFG_LEVEL_TMW
#endif

/* Extended String Data */
#ifndef SDNPDATA_SUPPORT_OBJ114
#define SDNPDATA_SUPPORT_OBJ114_V1 TMWDEFS_FALSE
#define SDNPDATA_SUPPORT_OBJ114_V2 TMWDEFS_FALSE
#define SDNPDATA_SUPPORT_OBJ114_V3 TMWDEFS_FALSE
#define SDNPDATA_SUPPORT_OBJ114_V4 TMWDEFS_FALSE
#define SDNPDATA_SUPPORT_OBJ114 \
  (SDNPDATA_SUPPORT_OBJ114_V1 | \
   SDNPDATA_SUPPORT_OBJ114_V2 | \
   SDNPDATA_SUPPORT_OBJ114_V3 | \
   SDNPDATA_SUPPORT_OBJ114_V4)
#endif

/* Set this to TMWDEFS_TRUE to reduce stack size requirements to
 * process extended strings. It requires the implementation of
 * sdnpdata_extStrRelease to inform the database that the pointer
 * returned by sdnpdata_extStrGetPtr is no longer in use.
 */
#define SDNPDATA_SUPPORT_OBJ114_MIN_STACK TMWDEFS_TRUE

/* Extended String Events */
#ifndef SDNPDATA_SUPPORT_OBJ115
#define SDNPDATA_SUPPORT_OBJ115_V1 TMWDEFS_FALSE
#define SDNPDATA_SUPPORT_OBJ115_V2 TMWDEFS_FALSE
#define SDNPDATA_SUPPORT_OBJ115_V3 TMWDEFS_FALSE
#define SDNPDATA_SUPPORT_OBJ115_V4 TMWDEFS_FALSE
#define SDNPDATA_SUPPORT_OBJ115 \
  (SDNPDATA_SUPPORT_OBJ115_V1 | \
   SDNPDATA_SUPPORT_OBJ115_V2 | \
   SDNPDATA_SUPPORT_OBJ115_V3 | \
   SDNPDATA_SUPPORT_OBJ115_V4)
#endif

/* If Secure Authentication support is not defined on the command line 
 * set the configuration here 
 * DNPCNFG_SUPPORT_AUTHENTICATION must also be defined appropriately
 * Security Statistics object groups 121 and 122 must be supported 
 * if SAv5 is supported.
 */
#ifndef SDNPDATA_SUPPORT_OBJ120
#define SDNPDATA_SUPPORT_OBJ120 DNPCNFG_SUPPORT_AUTHENTICATION
#endif

/* Secure Authentication User Certificate for Asymmetric Remote Key Update */
#ifndef SDNPDATA_SUPPORT_OBJ120_V8
#define SDNPDATA_SUPPORT_OBJ120_V8 TMWDEFS_FALSE
#endif

/* Set this to TMWDEFS_FALSE to remove code used to support assign class 
 * function code 
 */
#define SDNPDATA_SUPPORT_ASSIGN TMWDEFS_TRUE

/* Set this to TMWDEFS_FALSE to remove code used to support scanning for events 
 * If this is set to TMWDEFS_TRUE, the SCL can be configured to call the database
 * sdnpdata_xxxChanged functions periodically to see if values have changed and
 * if so to call the sdnpxxx_addEvent functions automatically.
 */
#define SDNPDATA_SUPPORT_EVENT_SCAN TMWDEFS_TRUE

/* Set this to TMWDEFS_FALSE to remove code for Identical Unsolicited Response Retries.
 * If this is set to TMWDEFS_TRUE, the SCL can be configured to send Identical 
 * Unsolicited Retries.
 * The DNP3 specification allows for either identical retries with the same events 
 * and sequence number (identical in every octet including IIN bits), or a 
 * regenerated retry which may contain new events,  different IIN bits and a 
 * new sequence number. The master MUST be able to handle both.
 * If compiled in and configured the outstation will send identical retries until 
 * the retries are exceeded or the retry series is cancelled by another read etc.
 */
#ifndef SDNPDATA_SUPPORT_IDENT_UNSOL_RETRY
#define SDNPDATA_SUPPORT_IDENT_UNSOL_RETRY TMWDEFS_FALSE
#endif

/* Set this to TMWDEFS_FALSE to remove code used to support 
 * per point default static variation 
 */
#define SDNPDATA_SUPPORT_STATIC_VAR_POINT TMWDEFS_TRUE

/* Set this to TMWDEFS_FALSE to remove code used to support 
 * per point default event variation 
 */
#define SDNPDATA_SUPPORT_EVENT_VAR_POINT TMWDEFS_TRUE

/* Set this to TMWDEFS_FALSE to remove code used to support 
 * per point event mode (SOE or LAST) 
 */
#ifndef SDNPDATA_SUPPORT_EVENT_MODE_POINT
#define SDNPDATA_SUPPORT_EVENT_MODE_POINT TMWDEFS_TRUE
#endif

/* Set this to TMWDEFS_FALSE to remove code used to support 
 * per point membership in class0
 */
#define SDNPDATA_SUPPORT_CLASS0_POINT TMWDEFS_TRUE

/* Set this to TMWDEFS_FALSE to remove code used to call sdnpdata function when a
 * select is cancelled either because the timer expired or because another command 
 * was received before the operate. Only set this to TMWDEFS_TRUE if your 
 * implementation requires that the database is notified when the select is cancelled.
 */
#ifndef SDNPDATA_SUPPORT_SELECT_CANCEL
#define SDNPDATA_SUPPORT_SELECT_CANCEL TMWDEFS_FALSE
#endif
 
/* 
 * Setting this to TMWDEFS_TRUE will cause the Source Code Library to handle 
 * a master reread of the same file block. 
 * To save memory and processing time, and to disable this resend of file
 * read response, set this to TMWDEFS_FALSE. The Database can still support
 * rereading of the same block.
 */
#define SDNPDATA_SUPPORT_FILE_REREAD TMWDEFS_FALSE

/* Application Layer Spec, Feb 2007, says to keep a copy of the last solicited
 * response that was sent. If a request with the same sequence number is 
 * is received and all of the octets are identical, resend last response, 
 * but do not process the duplicate request.
 * To save memory and processing time, and to disable this resend of response,
 * set this to TMWDEFS_FALSE. Even with this set to TMWDEFS_FALSE, duplicate
 * select and duplicate operate requests will be handled properly as required
 * by the conformance tests.
 */
#define SDNPDATA_KEEP_LAST_RESPONSE  TMWDEFS_FALSE

 /* Private XML format required for save/restore in Test Harness.
  * Can also be used for save/restore of SDNP .NET Library database.
  */
#ifndef SDNPDATA_SUPPORT_XML
#define SDNPDATA_SUPPORT_XML         TMWDEFS_FALSE
#endif

/* Set this parameter to TMWDEFS_TRUE to include support for generating
 * an XML document based on the current state of the DNP3 slave database. 
 * This allows the generation of an XML document which conforms to the new 
 * (version 2.11) DNP3 configuration schema.
 */
#ifndef SDNPDATA_SUPPORT_XML2
#define SDNPDATA_SUPPORT_XML2        TMWDEFS_FALSE
#endif

/* Set this parameter to TMWDEFS_TRUE to include support for per point data
 * called dnpData in the Device Profile schema. This includes values, quality,
 * and timestamps for the individual points in the database. 
 */
#ifndef SDNPDATA_SUPPORT_XML2_DNPDATA
#define SDNPDATA_SUPPORT_XML2_DNPDATA TMWDEFS_FALSE
#endif

 /* Support file transfer read of the above gnerated XML "files" */
#ifndef SDNPDATA_SUPPORT_READ_XML
#define SDNPDATA_SUPPORT_READ_XML     TMWDEFS_FALSE
#endif

/* Do this down here so the support macros as defined in sdnpsesn.h */
#include "tmwscl/dnp/sdnpsesn.h"

/* Define types for functions below */
typedef TMWTYPES_USHORT (*SDNPDATA_QUANTITY_FUNC)(void *);
typedef void * (*SDNPDATA_GET_POINT_FUNC)(void *, TMWTYPES_USHORT);
typedef TMWTYPES_BOOL (*SDNPDATA_IS_IN_CLASS0_FUNC)(void *); 

/* bit values for determining if particular control operation is supported 
 * These are NOT the values that are sent in the DNP message
 */
typedef TMWTYPES_UCHAR SDNPDATA_CROB_CTRL;
#define SDNPDATA_CROB_CTRL_PULSE_ON      0x01
#define SDNPDATA_CROB_CTRL_PULSE_OFF     0x02
#define SDNPDATA_CROB_CTRL_LATCH_ON      0x04
#define SDNPDATA_CROB_CTRL_LATCH_OFF     0x08
#define SDNPDATA_CROB_CTRL_PAIRED_CLOSE  0x10  
#define SDNPDATA_CROB_CTRL_PAIRED_TRIP   0x20  

/* Data set modes, used in sdnpdata_datasetCreatePoint() function */
typedef enum {
  SDNPDATA_DATASET_MODE_WRITE,
  SDNPDATA_DATASET_MODE_SELECT,
  SDNPDATA_DATASET_MODE_OPERATE
} SDNPDATA_DATASET_MODE; 

/* Data Set interface structure used for passing data set element 
 * control values between SCL and target layer database.
 */
typedef struct {
  /* element index in data set for this control value     */
  TMWTYPES_UCHAR        elemIndex;
  /* control value received in select or operate request */
  DNPDATA_DATASET_VALUE data;
} DNPDATA_DATASET_CTRL_VALUE;

/* union of values used for adding events internally in the SCL and
 * to the database if user managed events is configured.
 */
typedef union {
  /* This field is used when adding a counter or frozen counter event
  * from database
  */
  TMWTYPES_ULONG           ulValue;

  /* This field is used when adding a analog input event to database */
  TMWTYPES_ANALOG_VALUE   *analogPtr;

  /* This field is used when adding a string or vterm event to database */
  struct {
    TMWTYPES_UCHAR         length;
    TMWTYPES_UCHAR        *pBuf;
  } stringPtr;

  /* This field is used when adding an extended string event to database */
  struct {
    TMWTYPES_USHORT        length;
    TMWTYPES_UCHAR        *pBuf;
  } extendedStringPtr;

  struct {
    TMWTYPES_UCHAR         numberElems;
    DNPDATA_DATASET_VALUE *pData;
  } dataset;

  struct {
    TMWTYPES_USHORT        assocId;
    TMWTYPES_ULONG         sequenceNumber;
    TMWTYPES_UCHAR         errorCode;
    TMWTYPES_CHAR         *pErrorText;
    TMWTYPES_USHORT        errorTextLength;
  } authError;

  struct {
    TMWTYPES_USHORT        assocId;
    TMWTYPES_ULONG         ulValue;
  } authSecStat;

} SDNPDATA_ADD_EVENT_VALUE;

#ifdef __cplusplus
extern "C" {
#endif

  /* function: sdnpdata_getIIN
   * purpose: Called before each response is sent to allow the user
   *  to specify the current device IIN bits. The SCL will OR in additional
   *  IIN bits maintained by the SCL but will never clear a bit set
   *  in this function.
   * The following bits should be set by the user if appropriate:
   *  DNPDEFS_IIN_TROUBLE     - should be set when an abnormal condition exists,
   *                            such as hardware problems. Only set this if 
   *                            another IIN bit does not indicate this condition.
   *  DNPDEFS_IIN_LOCAL       - should be set if any output point is in the local
   *                            operation mode
   *  DNPDEFS_IIN_BAD_CONFIG  - should be set when a corrupt configuration is 
   *                            detected. Setting this bit is optional.
   * The following bits are managed by the SCL and should generally not 
   * be set by this routine:
   *  DNPDEFS_IIN_RESTART             see NOTE 1
   *  DNPDEFS_IIN_NEED_TIME
   *  DNPDEFS_IIN_CLASS_3
   *  DNPDEFS_IIN_CLASS_2
   *  DNPDEFS_IIN_CLASS_1
   *  DNPDEFS_IIN_ALL_STATIONS
   *  DNPDEFS_IIN_ALREADY_EXECUTING
   *  DNPDEFS_IIN_BUFFER_OVFL         set NOTE 1
   *  DNPDEFS_IIN_OUT_OF_RANGE
   *  DNPDEFS_IIN_OBJECT_UNKNOWN
   *  DNPDEFS_IIN_BAD_FUNCTION
   *  NOTE 1: user does not normally, but may set, IIN_RESTART and IIN_BUFFER_OVFL. 
   *   The SCL will manage the clearing of these two bits when appropriate.
   *  NOTE 2: *pIIN is zero when this function is called. You are free to leave it
   *   as zero or set any bits you choose.
   * arguments:
   *  pSession - pointer to session 
   *  pIIN - pointer in which to store IIN bits
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL sdnpdata_getIIN(
    TMWSESN *pSession,
    TMWTYPES_USHORT *pIIN);

  /* function: sdnpdata_IINQuantity 
   * purpose: Determine how many IIN bits are supported on this session
   * arguments:
   *  pHandle - handle to database returned from sdnpdata_init
   * returns:
   *  16 would indicate no private IIN bits are supported
   *  any number > 16 would indicate how many total IIN bits,
   *   (standard(16) + private bits) are supported.
   */
  TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpdata_IINQuantity(
    void *pHandle);

  /* function: sdnpdata_IINRead 
   * purpose: Read private IIN bit because of read ObjectGroup 80 request
   * arguments:
   *  pHandle - handle to database returned from sdnpdata_init
   *  pointNumber - index of IIN bit to read, 0-15 are standard IIN bits
   * returns:
   *  TMWTYPES_BOOL - TMWTYPES_BOOL if bit is set.
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_IINRead(
    void *pHandle,
    TMWTYPES_USHORT pointNumber);

  /* function: sdnpdata_coldRestart
   * purpose: Perform cold restart. Note that this function can NOT alter the
   *  SCL context and return since it will be called from within the SCL and 
   *  any changes will result in undefined behavior in the SCL. The options
   *  are to perform a hard reset (i.e. reset the program counter, stack, 
   *  etc.) and never return or set a flag to be processed in the user code
   *  after returning from the SCL.
   * arguments:
   *  pSession - pointer to session on which cold restart request was received
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL sdnpdata_coldRestart(
    TMWSESN *pSession);

  /* function: sdnpdata_warmRestart
   * purpose: Perform warm restart. Note that this function can NOT alter the
   *  SCL context and return since it will be called from within the SCL and 
   *  any changes will result in undefined behavior in the SCL. The options
   *  are to perform a hard reset (i.e. reset the program counter, stack, 
   *  etc.) and never return or set a flag to be processed in the user code
   *  after returning from the SCL.
   * arguments:
   *  pSession - pointer to session on which warm restart request was received
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL sdnpdata_warmRestart(
    TMWSESN *pSession);

  /* function: sdnpdata_init
   * purpose: Initialize DNP3 slave database for specified session
   * arguments:
   *  pSession - pointer to session
   *  pUserHandle - User provided handle passed into sdnpsesn_openSession()
   *    This handle could be used to identify the database associated with
   *    a particular session. The SCL does not use this handle internally.
   * returns:
   *  pointer to NON NULL database handle. This handle will be passed into 
   *   other sdnpdata_xxx functions and only needs to be meaningful to the 
   *   those routines. If multiple sessions and therefore databases are opened.
   *   This handle could be used to determine which database (which session)
   *   this function is being called for. The SCL will not use this handle 
   *  The SCL will not use this handle internally.
   *  TMWDEFS_NULL indicates failure.
   */
  void * TMWDEFS_GLOBAL sdnpdata_init(
    TMWSESN *pSession, 
    void *pUserHandle);

  /* function: sdnpdata_close
   * purpose: Close DNP3 slave database
   * arguments:
   *  pHandle - handle to database returned from sdnpdata_init
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL sdnpdata_close(
    void *pHandle);

  /* function: sdnpdata_setTime  
   * purpose: Set the time because a write time request has been
   *  received from the master.  Default behavior is to set the clock.
   *  Target implementations may only want 1 session on a device to set the time. 
   *  Otherwise multiple masters might set the time differently.
   *  There is also a session configuration parameter "respondNeedTime" that
   *  controls whether a session will set the need time IIN bit, requesting a 
   *  time sync from the master.
   * arguments:
   *  pHandle - handle to database returned from sdnpdata_init
   *  pNewTime - pointer to a time structure containing the time sent by the master
   */
  void sdnpdata_setTime(
    void *pHandle,
    TMWDTIME *pNewTime);

  /* function: sdnpdata_unsolEventMask
   * purpose: Indicate slave has received a function code enable unsolicited
   *  or function code disable unsolicited request from the master. 
   * arguments:
   *  pHandle - handle to database returned from sdnpdata_init
   *  unsolEventMask - mask containing three bits indicating which classes are
   *   now enabled for unsolicited responses. 
   *    TMWDEFS_CLASS_MASK_ONE
   *    TMWDEFS_CLASS_MASK_TWO
   *    TMWDEFS_CLASS_MASK_THREE
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL sdnpdata_unsolEventMask(
    void *pHandle, 
    TMWDEFS_CLASS_MASK unsolEventMask);

  /* function: sdnpdata_eventAndStaticRead  
   * purpose: Indicate that a read of events and static data is
   *  beginning or ending. It may be desirable to prevent changes
   *  to the values of static data in the database while a read is
   *  in progress. This can be used to guarantee that events are reported
   *  before the new changed values in the static data. This is not
   *  an issue if events only are read, since another read will be required
   *  to get any remaining events in the queue before the static values are
   *  read. NOTE: Reading static data and events in separate reads is discouraged 
   *  as it is even more likely that the static data would be read before the 
   *  queued events.
   * arguments:
   *  pHandle - handle to database returned from sdnpdata_init 
   *  inProgress - TMWDEFS_TRUE if read is in progress (beginning)
   *               TMWDEFS_FALSE if read is ended
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL sdnpdata_eventAndStaticRead(
    void *pHandle,
    TMWTYPES_BOOL inProgress);

  /* function: sdnpdata_funcCode  
   * purpose: Indicate that a request has been received from the master and that 
   *  the specified function code processing is being started or has completed.
   *  The database may want to accumulate the individual points and data from
   *  a request before acting on them.
   * arguments:
   *  pHandle - handle to database returned from sdnpdata_init 
   *  functionCode - function code being requested. 
   *                 (DNPDEFS_FC_WRITE, DNPDEFS_FC_DIRECT_OP_NOACK etc)
   *  inProgress - TMWDEFS_TRUE if request is in progress (beginning)
   *               TMWDEFS_FALSE if request is ended
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL sdnpdata_funcCode(
    void *pHandle,
    char functionCode,
    TMWTYPES_BOOL inProgress);

  /* Binary Inputs */

  /* function: sdnpdata_binInGetDescription  
   * purpose: Get description of this point
   *  NOTE: this is used only by sdnpxml/sdnpxml2.c if SDNPDATA_SUPPORT_XMLxx is TRUE
   *  code to generate the configuration file or Device Profile for this device. 
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.  
   * returns:
   *  pointer to string describing point
   *  TMWDEFS_NULL if failure
   */
  TMWTYPES_CHAR * TMWDEFS_GLOBAL sdnpdata_binInGetDescription(
    void *pPoint);

  /* function: sdnpdata_binInQuantity
   * purpose: Return the number of binary input data points in the 
   *  specified database.
   * arguments:
   *  pHandle - handle to database returned from sdnpdata_init
   * returns:
   *  The number of binary input data points (see note for
   *  sdnpdata_xxxQuantity at top of this file)
   */
  TMWTYPES_USHORT TMWDEFS_CALLBACK sdnpdata_binInQuantity(
    void *pHandle);

  /* function: sdnpdata_binInGetPoint
   * purpose: Return a handle to a specific data point. This handle is
   *  used by the routines below to read and write values and control
   *  information to this point.
   * arguments:
   *  pHandle - handle to database returned from sdnpdata_init
   *  pointNum - point number to return
   * returns:
   *  Pointer to specified data point or TMWDEFS_NULL if this point
   *   is currently disabled.
   */
  void * TMWDEFS_CALLBACK sdnpdata_binInGetPoint(
    void *pHandle,
    TMWTYPES_USHORT pointNum);

  /* function: sdnpdata_binInDefVariation  
   * purpose: Determine default static variation for this binary input
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   * returns:
   *  default variation for this point
   *  NOTE: this will only be called if the default variation for binary
   *   inputs obj01DefaultVariation is configured as zero
   */
  TMWTYPES_UCHAR TMWDEFS_GLOBAL sdnpdata_binInDefVariation(
    void *pPoint);

  /* function: sdnpdata_binInEventClass
   * purpose: Return the class in which events from this data point
   *  belong.
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   * returns:
   *  Class in which these events will be returned
   */
  TMWDEFS_CLASS_MASK TMWDEFS_GLOBAL sdnpdata_binInEventClass(
    void *pPoint);

  /* function: sdnpdata_binInIsClass0
   * purpose: Should this point be reported in response to an object 60 
   *   variation 1 read request. This allows individual points to be excluded
   *   from a class 0 response but still readable by a specific object group
   *   read request.
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   * returns:
   *  TMWDEFS_TRUE if point should be reported.
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_binInIsClass0(
    void *pPoint);
  
  /* function: sdnpdata_binInEventDefVariation  
   * purpose: Determine default variation for this binary input
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   *  classMask - mask indicating what event class is being requested
   * returns:
   *  default variation for this point
   *  NOTE: this will only be called if the default variation for binary
   *   input change events obj02DefaultVariation is configured as zero
   */
  TMWTYPES_UCHAR TMWDEFS_GLOBAL sdnpdata_binInEventDefVariation(
    void *pPoint,
    TMWDEFS_CLASS_MASK classMask);

  /* function: sdnpdata_binInEventMode 
   * purpose: Determine event mode for this point
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   * returns:
   *  event mode for this point, SOE or LAST
   *  NOTE: this will only be called if the event mode for binary inputs
   *   if binaryInputEventMode is configured as TMWDEFS_EVENT_MODE_PER_POINT
   */
  TMWDEFS_EVENT_MODE TMWDEFS_GLOBAL sdnpdata_binInEventMode(
    void *pPoint);

  /* function: sdnpdata_binInAssignClass
   * purpose: Assign the class in which events from this data point
   *  will belong.
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   *  classMask - new class in which to generate events from this point
   * returns:
   *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_binInAssignClass(
    void *pPoint,
    TMWDEFS_CLASS_MASK classMask);

  /* function: sdnpdata_binInRead
   * purpose: Return the current value of the specified point.
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   *  pFlags - pointer to location to store current DNP3 flags and value.
   *   pFlags contains a status indication and the current state of the point.
   *   The following values (or OR'd combinations) are valid for this type:
   *      DNPDEFS_DBAS_FLAG_OFF_LINE - the point is off-line, and the returned
   *        state of this point may not be correct
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
   *      DNPDEFS_DBAS_FLAG_BINARY_OFF - the current state of the input (Off)
   * returns:
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL sdnpdata_binInRead(
    void *pPoint,
    TMWTYPES_UCHAR *pFlags);

  /* function: sdnpdata_binInChanged
   * purpose: Determine if the specified point has changed and if so
   *  return the new value. This function is used to scan for events 
   *  on each data point. It will be called if SDNPSESN_CONFIG
   *  binaryInputScanPeriod is nonzero.
   *  NOTE: this functionality is compiled out by defining 
   *  SDNPDATA_SUPPORT_OBJ2 FALSE
   *  or SDNPDATA_SUPPORT_EVENT_SCAN FALSE
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   *  pFlags - pointer to location to store current DNP3 flags and value
   *   pFlags contains a status indication and the current state of the point.
   *   The following values (or OR'd combinations) are valid for this type:
   *      DNPDEFS_DBAS_FLAG_OFF_LINE - the point is off-line, and the returned
   *        state of this point may not be correct
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
   *      DNPDEFS_DBAS_FLAG_BINARY_OFF - the current state of the input (Off)
   * returns:
   *  TMWDEFS_TRUE if the point has changed, else TMWDEFS_FALSE
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_binInChanged(
    void *pPoint,
    TMWTYPES_UCHAR *pFlags);

  /* Binary Outputs */

  /* function: sdnpdata_binOutGetDescription  
   * purpose: Get description of this point
   *  NOTE: this is used only by sdnpxml/sdnpxml2.c if SDNPDATA_SUPPORT_XMLxx is TRUE
   *  code to generate the configuration file or Device Profile for this device. 
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.  
   * returns:
   *  pointer to string describing point
   *  TMWDEFS_NULL if failure
   */
  TMWTYPES_CHAR * TMWDEFS_GLOBAL sdnpdata_binOutGetDescription(
    void *pPoint);

  /* function: sdnpdata_binOutQuantity
   * purpose: Return the number of binary output data points in the 
   *  specified database.
   * arguments:
   *  pHandle - handle to database returned from sdnpdata_init
   * returns:
   *  The number of binary output data points (see note for
   *  sdnpdata_xxxQuantity at top of this file)
   */
  TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpdata_binOutQuantity(
    void *pHandle);

  /* function: sdnpdata_binOutGetPoint
   * purpose: Return a handle to a specific data point. This handle is
   *  used by the routines below to read and write values and control
   *  information to this point.
   * arguments:
   *  pHandle - handle to database returned from sdnpdata_init
   *  pointNum - point number to return
   * returns:
   *  Pointer to specified data point or TMWDEFS_NULL if this point
   *   is currently disabled.
   */
  void * TMWDEFS_GLOBAL sdnpdata_binOutGetPoint(
    void *pHandle,
    TMWTYPES_USHORT pointNum);
  
  /* function: sdnpdata_binOutDefVariation  
   * purpose: Determine default static variation for this binary output status
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   * returns:
   *  default variation for this point
   *  NOTE: this will only be called if the default variation for binary
   *   inputs obj10DefaultVariation is configured as zero
   */
  TMWTYPES_UCHAR TMWDEFS_GLOBAL sdnpdata_binOutDefVariation(
    void *pPoint);

  /* function: sdnpdata_binOutEventClass
   * purpose: Return the class in which binary output events from this 
   *  data point belong.
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   * returns:
   *  Class in which these events will be returned
   */
  TMWDEFS_CLASS_MASK TMWDEFS_GLOBAL sdnpdata_binOutEventClass(
    void *pPoint);

  /* function: sdnpdata_binOutIsClass0
   * purpose: Should this point be reported in response to an object 60 
   *   variation 1 read request. This allows individual points to be excluded
   *   from a class 0 response but still readable by a specific object group
   *   read request.
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   * returns:
   *  TMWDEFS_TRUE if point should be reported.
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_binOutIsClass0(
    void *pPoint);

  /* function: sdnpdata_binOutEventDefVariation  
   * purpose: Determine default event variation for this binary output point
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   *  classMask - mask indicating what event class is being requested
   * returns:
   *  default variation for this point
   *  NOTE: this will only be called if the default variation for binary
   *   output events obj11DefaultVariation is configured as zero
   */
  TMWTYPES_UCHAR TMWDEFS_GLOBAL sdnpdata_binOutEventDefVariation(
    void *pPoint,
    TMWDEFS_CLASS_MASK classMask);

  /* function: sdnpdata_binOutEventMode 
   * purpose: Determine event mode for this point
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   * returns:
   *  event mode for this point, SOE or LAST
   *  NOTE: this will only be called if the event mode for binary outputs
   *   if binaryOutputEventMode is configured as TMWDEFS_EVENT_MODE_PER_POINT
   */
  TMWDEFS_EVENT_MODE TMWDEFS_GLOBAL sdnpdata_binOutEventMode(
    void *pPoint);

  /* function: sdnpdata_binOutAssignClass
   * purpose: Assign the class in which binary output events from this 
   *  data point will belong.
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   *  classMask - new class in which to generate events from this point
   * returns:
   *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_binOutAssignClass(
    void *pPoint,
    TMWDEFS_CLASS_MASK classMask);

  /* function: sdnpdata_binOutRead
   * purpose: Return the current value of the specified point.
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   *  pFlags - pointer to location to store current DNP3 flags and value
   *   pFlags contains a status indication and the current state of the point.
   *   The following values (or OR'd combinations) are valid for this type:
   *      DNPDEFS_DBAS_FLAG_OFF_LINE - the point is off-line, and the returned
   *        state of this point may not be correct
   *        NOTE: if the point or device is in local mode the point should be OFF_LINE
   *      DNPDEFS_DBAS_FLAG_ON_LINE - the binary output point has been read
   *        successfully
   *      DNPDEFS_DBAS_FLAG_RESTART - the field device that originated the
   *        data object has been restarted. This device may be the device
   *        reporting this data object.
   *      DNPDEFS_DBAS_FLAG_COMM_LOST - the device reporting this data object
   *        has lost communication with the originator of the data object
   *      DNPDEFS_DBAS_FLAG_REMOTE_FORCED - the state of the binary object
   *        has been forced to its current state at the originating device
   *      DNPDEFS_DBAS_FLAG_LOCAL_FORCED - the state of the binary object
   *        has been forced to its current state at the device reporting
   *        this data object
   *        NOTE: this bit is NOT related to the point being in LOCAL MODE.
   *      DNPDEFS_DBAS_FLAG_BINARY_ON  - the current state of the output (On)
   *      DNPDEFS_DBAS_FLAG_BINARY_OFF - the current state of the output (Off)
   * returns:
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL sdnpdata_binOutRead(
    void *pPoint,
    TMWTYPES_UCHAR *pFlags);

  /* function: sdnpdata_binOutChanged
   * purpose: Determine if the specified point has changed and if so
   *  return the new value. This function is used to scan for events 
   *  on each data point. It will be called if SDNPSESN_CONFIG
   *  binaryOutputScanPeriod is nonzero.
   *  NOTE: this functionality is compiled out by defining 
   *  SDNPDATA_SUPPORT_OBJ11 FALSE  (default)
   *  or SDNPDATA_SUPPORT_EVENT_SCAN FALSE
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   *  pFlags - pointer to location to store current DNP3 flags and value
   *   pFlags contains a status indication and the current state of the point.
   *   The following values (or OR'd combinations) are valid for this type:
   *      DNPDEFS_DBAS_FLAG_OFF_LINE - the point is off-line, and the returned
   *        state of this point may not be correct
   *      DNPDEFS_DBAS_FLAG_ON_LINE - the binary output point has been read
   *        successfully
   *      DNPDEFS_DBAS_FLAG_RESTART - the field device that originated the
   *        data object has been restarted. This device may be the device
   *        reporting this data object.
   *      DNPDEFS_DBAS_FLAG_COMM_LOST - the device reporting this data object
   *        has lost communication with the originator of the data object
   *      DNPDEFS_DBAS_FLAG_REMOTE_FORCED - the state of the binary object
   *        has been forced to its current state at the originating device
   *      DNPDEFS_DBAS_FLAG_LOCAL_FORCED - the state of the binary object
   *        has been forced to its current state at the device reporting
   *        this data object
   *      DNPDEFS_DBAS_FLAG_BINARY_ON  - the current state of the output (On)
   *      DNPDEFS_DBAS_FLAG_BINARY_OFF - the current state of the output (Off)
   * returns:
   *  TMWDEFS_TRUE if the point has changed, else TMWDEFS_FALSE
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_binOutChanged(
    void *pPoint,
    TMWTYPES_UCHAR *pFlags);

  /* function: sdnpdata_binOutWrite 
   * purpose: perform write operation on the specified binary output point and 
   *   return the result. Write to object group 10 variation 1 was received.
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   *  value - Value to be written. Does not contain status.
   *      DNPDEFS_DBAS_FLAG_BINARY_ON  - set the output to (On)
   *      DNPDEFS_DBAS_FLAG_BINARY_OFF - set the output to (Off)
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_binOutWrite(
    void *pPoint,
    TMWTYPES_UCHAR value);

  /* function: sdnpdata_binOutGetControlMask
   * purpose: Determine what control operations this particular point supports.
   *  NOTE: this is used only by sdnpxml/sdnpxml2.c if SDNPDATA_SUPPORT_XMLxx is TRUE
   *  code to generate the configuration file or Device Profile for this device. 
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   * returns:
   *  bitmask indicating what controls are allowed. 
   *   SDNPDATA_CROB_CTRL_PULSE_ON
   *   SDNPDATA_CROB_CTRL_PULSE_OFF
   *   SDNPDATA_CROB_CTRL_LATCH_ON 
   *   SDNPDATA_CROB_CTRL_LATCH_OFF 
   *   SDNPDATA_CROB_CTRL_PAIRED_CLOSE 
   *   SDNPDATA_CROB_CTRL_PAIRED_TRIP
   */
  SDNPDATA_CROB_CTRL TMWDEFS_GLOBAL sdnpdata_binOutGetControlMask(
    void *pPoint);

  /* function: sdnpdata_binOutSelect
   * purpose: perform Select operation on the specified point and return
   *   the result.
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   *  controlCode - the control function to perform. The Control Code
   *   may contained OR'd values of the following values:
   *     DNPDEFS_CROB_CTRL_PULSE_ON - The point(s) is turned on for the
   *      specified onTime (activationPeriod), turned off for the specified
   *       offTime, and leftin the off state.
   *     DNPDEFS_CROB_CTRL_PULSE_OFF - The point(s) is turned off for the
   *      specific offTime (deactivationPeriod), then turned on for the specified 
   *      onTime, and left in the on state.
   *     DNPDEFS_CROB_CTRL_LATCH_ON - The point(s) is latched on
   *     DNPDEFS_CROB_CTRL_LATCH_OFF - The point(s) is latched off
   *     DNPDEFS_CROB_CTRL_QUEUE - place the operation back in the control
   *      queue when complete
   *     DNPDEFS_CROB_CTRL_CLEAR - cancel the currently running operation and
   *      remove queued operations on affected points immediately before
   *      activating this new operation
   *     DNPDEFS_CROB_CTRL_PAIRED_CLOSE - activate Close relay  
   *     DNPDEFS_CROB_CTRL_PAIRED_TRIP - activate Trip relay
   *     DNPDEFS_CROB_CTRL_PAIRED_CLOSE and DNPDEFS_CROB_CTRL_PAIRED_TRIP are
   *      used in systems where a Trip and Close relay pair is used to energize
   *      and de-energize  the field points. Both of these bits can be 0 to
   *      activate the field point select relay only, without activating the
   *      Trip/Close relays. In a system without field point relays, if both
   *      of these bits are 0, then no control operation should be performed.
   *      In a system without Trip/Close relays, both of these bits should
   *      always be 0 to indicate normal digital control operations. It is
   *      invalid for DNPDEFS_CROB_CTRL_PAIRED_TRIP and
   *      DNPDEFS_CROB_CTRL_PAIRED_CLOSE to both be set.
   *  count - the number of times that the control operation should be
   *    performed in succession. If count is 0, do not execute the control.
   *  onTime - amount of time (in ms) the digital output is to be turned on 
   *    (may not apply to all control types)
   *  offTime - amount of time (in ms) the digital output is to be turned off
   *    (may not apply to all control types)
   * returns:
   *  status of CROB operation. Valid values are:
   *    DNPDEFS_CROB_ST_SUCCESS - the command was performed successfully
   *    DNPDEFS_CROB_ST_FORMAT_ERROR - the request was not accepted due
   *      to formatting errors in the request
   *    DNPDEFS_CROB_ST_NOT_SUPPORTED - the request was not accepted because
   *      Control operations are not supported for this point
   *    DNPDEFS_CROB_ST_ALREADY_ACTIVE - the request was not accepted because
   *      the control queue is full or the point is already active
   *    DNPDEFS_CROB_ST_HARDWARE_ERROR - the request was not accepted due to
   *      control hardware problems
   *    DNPDEFS_CROB_ST_LOCAL - the request was not accepted because 
   *      Local/Remote switch is in Local Position
   *    DNPDEFS_CROB_ST_NOT_AUTHORIZED - the request was not accepted because 
   *      of insufficient authorization.
   *    DNPDEFS_CROB_ST_AUTO_INHIBIT - the request was not accepted because it
   *      was prevented or inhibited by a local automation process  
   *    DNPDEFS_CROB_ST_PROC_LIMITED - the request was not accepted because the
   *     device cannot process any more activities than are presently in progress
   *    DNPDEFS_CROB_ST_OUT_OF_RANGE - the request was not accepted because the
   *     value is outside the acceptable range permitted for this point.
   *    DNPDEFS_CROB_ST_DOWNSTRM_LOCAL - the request was not accepted because the
   *     control is being forwarded to a downstream device that is reporting Local.
   *    DNPDEFS_CROB_ST_ALR_COMPLETE - the request was not accepted because the
   *     operation is already complete. For example if the request is to close a
   *     switch and the switch is already closed.
   *    DNPDEFS_CROB_ST_BLOCKED - the request was not accepted because it is
   *     specifically blocked at the outstation.
   *    DNPDEFS_CROB_ST_CANCELLED - the request was not accepted because the
   *     operation was cancelled.
   *    DNPDEFS_CROB_ST_BLOCKED_OM - the request was not accepted because another
   *     master has exclusive rights to operate this point.
   *    DNPDEFS_CROB_ST_DOWNSTRM_FAIL -  the request was not accepted because the
   *     control is being forwarded to a downstream device which cannot be reached
   *     or is otherwise incapable of performing the request.
   *    DNPDEFS_CROB_ST_UNDEFINED - the request not accepted because of some 
   *     other undefined reason
   */
  DNPDEFS_CROB_ST TMWDEFS_GLOBAL sdnpdata_binOutSelect(
    void *pPoint,
    TMWTYPES_UCHAR controlCode,
    TMWTYPES_UCHAR count,
    TMWTYPES_ULONG onTime,
    TMWTYPES_ULONG offTime);
     
  /* function: sdnpdata_binOutCancelSelect
   * purpose: cancel outstanding Select on the specified point
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL sdnpdata_binOutCancelSelect(
    void *pPoint);

  /* function: sdnpdata_binOutOperate
   * purpose: perform Operate operation on the specified point and return
   *   the result. If Object 13 Binary Output Command Events are desired
   *   you may call sdnpo013_addEvent() from this function.
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   *  controlCode - the control function to perform. The Control Code
   *   may contained OR'd values of the following values:
   *     DNPDEFS_CROB_CTRL_PULSE_ON - The point(s) is turned on for the
   *      specified onTime, turned off for the specified offTime, and left
   *      in the off state.
   *     DNPDEFS_CROB_CTRL_PULSE_OFF - The point(s) is turned off for the
   *      specific offTime, then turned on for the specified onTime, and left
   *      in the on state.
   *     DNPDEFS_CROB_CTRL_LATCH_ON - The point(s) is latched on
   *     DNPDEFS_CROB_CTRL_LATCH_OFF - The point(s) is latched off
   *     DNPDEFS_CROB_CTRL_QUEUE - place the operation back in the control
   *      queue when complete. This bit is obsolete, outstations that receive
   *      this should return a status code DNPDEFS_CROB_ST_NOT_SUPPORTED.
   *     DNPDEFS_CROB_CTRL_CLEAR - cancel the currently running operation and
   *      remove queued operations on affected points immediately before
   *      activating this new operation
   *     DNPDEFS_CROB_CTRL_PAIRED_CLOSE - activate Close relay  
   *     DNPDEFS_CROB_CTRL_PAIRED_TRIP - activate Trip relay
   *     DNPDEFS_CROB_CTRL_PAIRED_CLOSE and DNPDEFS_CROB_CTRL_PAIRED_TRIP are
   *      used in systems where a Trip and Close relay pair is used to energize
   *      and de-energize  the field points. Both of these bits can be 0 to
   *      activate the field point select relay only, without activating the
   *      Trip/Close relays. In a system without field point relays, if both
   *      of these bits are 0, then no control operation should be performed.
   *      In a system without Trip/Close relays, both of these bits should
   *      always be 0 to indicate normal digital control operations. It is
   *      invalid for DNPDEFS_CROB_CTRL_PAIRED_TRIP and
   *      DNPDEFS_CROB_CTRL_PAIRED_CLOSE to both be set.
   *  count - the number of times that the control operation should be
   *    performed in succession. If count is 0, do not execute the control.
   *  onTime - amount of time (in ms) the digital output is to be turned on 
   *    (may not apply to all control types)
   *  offTime - amount of time (in ms) the digital output is to be turned off 
   *    (may not apply to all control types)
   * returns:
   *  status of CROB operation. Valid values are:
   *    DNPDEFS_CROB_ST_SUCCESS - the command was performed successfully
   *    DNPDEFS_CROB_ST_TIMEOUT - the request was not accepted because the
   *      Operate message was received after the Arm timer timed out. The
   *      Arm timer is started when a Select operation for the same point
   *      was received.
   *    DNPDEFS_CROB_ST_NO_SELECT - No previously matching select message
   *      (i.e., an Operate message was sent to activate a control point
   *      that was not previously armed with a Select message.)
   *    DNPDEFS_CROB_ST_FORMAT_ERROR - the request was not accepted due
   *      to formatting errors in the request
   *    DNPDEFS_CROB_ST_NOT_SUPPORTED - the request was not accepted because
   *      Control operations are not supported for this point
   *    DNPDEFS_CROB_ST_ALREADY_ACTIVE - the request was not accepted because
   *      the control queue is full or the point is already active
   *    DNPDEFS_CROB_ST_HARDWARE_ERROR - the request was not accepted due to
   *      control hardware problems
   *    DNPDEFS_CROB_ST_LOCAL - the request was not accepted because 
   *      Local/Remote switch is in Local Position
   *    DNPDEFS_CROB_ST_NOT_AUTHORIZED - the request was not accepted because 
   *      of insufficient authorization.
   *    DNPDEFS_CROB_ST_AUTO_INHIBIT - the request was not accepted because it
   *      was prevented or inhibited by a local automation process  
   *    DNPDEFS_CROB_ST_PROC_LIMITED - the request was not accepted because the
   *     device cannot process any more activities than are presently in progress 
   *    DNPDEFS_CROB_ST_OUT_OF_RANGE - the request was not accepted because the 
   *     value is outside the acceptable range permitted for this point.  
   *    DNPDEFS_CROB_ST_DOWNSTRM_LOCAL - the request was not accepted because the 
   *     control is being forwarded to a downstream device that is reporting Local.
   *    DNPDEFS_CROB_ST_ALR_COMPLETE - the request was not accepted because the 
   *     operation is already complete. For example if the request is to close a 
   *     switch and the switch is already closed.
   *    DNPDEFS_CROB_ST_BLOCKED - the request was not accepted because it is 
   *     specifically blocked at the outstation.
   *    DNPDEFS_CROB_ST_CANCELLED - the request was not accepted because the 
   *     operation was cancelled.
   *    DNPDEFS_CROB_ST_BLOCKED_OM - the request was not accepted because another 
   *     master has exclusive rights to operate this point.
   *    DNPDEFS_CROB_ST_DOWNSTRM_FAIL -  the request was not accepted because the 
   *     control is being forwarded to a downstream device which cannot be reached 
   *     or is otherwise incapable of performing the request.
   *    DNPDEFS_CROB_ST_UNDEFINED - the request not accepted because of some 
   *     other undefined reason
   */
  DNPDEFS_CROB_ST TMWDEFS_GLOBAL sdnpdata_binOutOperate(
    void *pPoint,
    TMWTYPES_UCHAR controlCode,
    TMWTYPES_UCHAR count,
    TMWTYPES_ULONG onTime,
    TMWTYPES_ULONG offTime);

  /* function: sdnpdata_binOutSelPatternMask
   * purpose: perform Select Pattern Mask operation on the specified point
   *  and return the result.
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   *  controlCode - the control function to perform. The Control Code
   *   may contained OR'd values of the following values:
   *     DNPDEFS_CROB_CTRL_PULSE_ON - The point(s) is turned on for the
   *      specified onTime, turned off for the specified offTime, and left
   *      in the off state.
   *     DNPDEFS_CROB_CTRL_PULSE_OFF - The point(s) is turned off for the
   *      specific offTime, then turned on for the specified onTime, and left
   *      in the on state.
   *     DNPDEFS_CROB_CTRL_LATCH_ON - The point(s) is latched on
   *     DNPDEFS_CROB_CTRL_LATCH_OFF - The point(s) is latched off
   *     DNPDEFS_CROB_CTRL_CLEAR - cancel the currently running operation and
   *      remove queued operations on affected points immediately before
   *      activating this new operation
   *     DNPDEFS_CROB_CTRL_PAIRED_CLOSE - activate Close relay  
   *     DNPDEFS_CROB_CTRL_PAIRED_TRIP - activate Trip relay
   *     DNPDEFS_CROB_CTRL_PAIRED_CLOSE and DNPDEFS_CROB_CTRL_PAIRED_TRIP are
   *      used in systems where a Trip and Close relay pair is used to energize
   *      and de-energize  the field points. Both of these bits can be 0 to
   *      activate the field point select relay only, without activating the
   *      Trip/Close relays. In a system without field point relays, if both
   *      of these bits are 0, then no control operation should be performed.
   *      In a system without Trip/Close relays, both of these bits should
   *      always be 0 to indicate normal digital control operations. It is
   *      invalid for DNPDEFS_CROB_CTRL_PAIRED_TRIP and
   *      DNPDEFS_CROB_CTRL_PAIRED_CLOSE to both be set.
   *  count - the number of times that the control operation should be
   *    performed in succession. If count is 0, do not execute the control.
   *  activationPeriod - amount of time the digital output is to be turned on 
   *    (may not apply to all control types)
   *  deactivationPeriod - amount of time the digital output is to be turned off
   *    (may not apply to all control types)
   *  firstPointNumber - first point number represented in pattern mask (*pMask).
   *  lastPointNumber - last point number represented in the pattern mask (*pMask).
   *  pMask - pointer to an array of bytes containing individual bits indicating 
   *    which points in the range should have the control applied. If the bit is
   *    set the control operation is applied to the corresponding point. The first 
   *    point is represented by the bit in bit 0 position of the first octet. The 
   *    second point by bit 1 position etc. 
   * returns:
   *  status of CROB operation. Valid values are:
   *    DNPDEFS_CROB_ST_SUCCESS - the command was performed successfully
   *    DNPDEFS_CROB_ST_TIMEOUT - the request was not accepted because the
   *      Operate message was received after the Arm timer timed out. The
   *      Arm timer is started when a Select operation for the same point
   *      was received.
   *    DNPDEFS_CROB_ST_FORMAT_ERROR - the request was not accepted due
   *      to formatting errors in the request
   *    DNPDEFS_CROB_ST_NOT_SUPPORTED - the request was not accepted because
   *      Control operations are not supported for this point
   *    DNPDEFS_CROB_ST_ALREADY_ACTIVE - the request was not accepted because
   *      the control queue is full or the point is already active
   *    DNPDEFS_CROB_ST_HARDWARE_ERROR - the request was not accepted due to
   *      control hardware problems
   *    DNPDEFS_CROB_ST_LOCAL - the request was not accepted because 
   *      Local/Remote switch is in Local Position
   *    DNPDEFS_CROB_ST_NOT_AUTHORIZED - the request was not accepted because 
   *      of insufficient authorization.
   *    DNPDEFS_CROB_ST_AUTO_INHIBIT - the request was not accepted because it
   *      was prevented or inhibited by a local automation process  
   *    DNPDEFS_CROB_ST_PROC_LIMITED - the request was not accepted because the
   *     device cannot process any more activities than are presently in progress 
   *    DNPDEFS_CROB_ST_OUT_OF_RANGE - the request was not accepted because the 
   *     value is outside the acceptable range permitted for this point.  
   *    DNPDEFS_CROB_ST_DOWNSTRM_LOCAL - the request was not accepted because the 
   *     control is being forwarded to a downstream device that is reporting Local.
   *    DNPDEFS_CROB_ST_ALR_COMPLETE - the request was not accepted because the 
   *     operation is already complete. For example if the request is to close a 
   *     switch and the switch is already closed.
   *    DNPDEFS_CROB_ST_BLOCKED - the request was not accepted because it is 
   *     specifically blocked at the outstation.
   *    DNPDEFS_CROB_ST_CANCELLED - the request was not accepted because the 
   *     operation was cancelled.
   *    DNPDEFS_CROB_ST_BLOCKED_OM - the request was not accepted because another 
   *     master has exclusive rights to operate this point.
   *    DNPDEFS_CROB_ST_DOWNSTRM_FAIL -  the request was not accepted because the 
   *     control is being forwarded to a downstream device which cannot be reached 
   *     or is otherwise incapable of performing the request.
   *    DNPDEFS_CROB_ST_UNDEFINED - the request not accepted because of some 
   *     other undefined reason
   */
  DNPDEFS_CROB_ST sdnpdata_binOutSelPatternMask(
    void *pHandle,
    TMWTYPES_UCHAR control, 
    TMWTYPES_UCHAR count, 
    TMWTYPES_ULONG activationPeriod,
    TMWTYPES_ULONG deactivationPeriod, 
    TMWTYPES_USHORT firstPointNumber, 
    TMWTYPES_USHORT lastPointNumber, 
    TMWTYPES_UCHAR *pMask);

  /* function: sdnpdata_binOutOpPatternMask
   * purpose: perform Operate Pattern Mask operation on the specified point
   * and return the result. If Object 13 Binary Output Command Events are desired
   *   you may call sdnpo013_addEvent() from this function.
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   *  controlCode - the control function to perform. The Control Code
   *   may contained OR'd values of the following values:
   *     DNPDEFS_CROB_CTRL_PULSE_ON - The point(s) is turned on for the
   *      specified onTime (activationPeriod), turned off for the specified
   *       offTime, and leftin the off state.
   *     DNPDEFS_CROB_CTRL_PULSE_OFF - The point(s) is turned off for the
   *      specific offTime (deactivationPeriod), then turned on for the specified 
   *      onTime, and left in the on state.
   *     DNPDEFS_CROB_CTRL_LATCH_ON - The point(s) is latched on
   *     DNPDEFS_CROB_CTRL_LATCH_OFF - The point(s) is latched off
   *     DNPDEFS_CROB_CTRL_CLEAR - cancel the currently running operation and
   *      remove queued operations on affected points immediately before
   *      activating this new operation
   *     DNPDEFS_CROB_CTRL_PAIRED_CLOSE - activate Close relay  
   *     DNPDEFS_CROB_CTRL_PAIRED_TRIP - activate Trip relay
   *     DNPDEFS_CROB_CTRL_PAIRED_CLOSE and DNPDEFS_CROB_CTRL_PAIRED_TRIP are
   *      used in systems where a Trip and Close relay pair is used to energize
   *      and de-energize  the field points. Both of these bits can be 0 to
   *      activate the field point select relay only, without activating the
   *      Trip/Close relays. In a system without field point relays, if both
   *      of these bits are 0, then no control operation should be performed.
   *      In a system without Trip/Close relays, both of these bits should
   *      always be 0 to indicate normal digital control operations. It is
   *      invalid for DNPDEFS_CROB_CTRL_PAIRED_TRIP and
   *      DNPDEFS_CROB_CTRL_PAIRED_CLOSE to both be set.
   *  count - the number of times that the control operation should be
   *    performed in succession. If count is 0, do not execute the control.
   *  activationPeriod - amount of time the digital output is to be turned on 
   *    (may not apply to all control types)
   *  deactivationPeriod - amount of time the digital output is to be turned off
   *    (may not apply to all control types)
   *  firstPointNumber - first point number represented in the pattern mask(*pMask).
   *  lastPointNumber - last point number represented in the pattern mask (*pMask).
   *  pMask - pointer to an array of bytes containing individual bits indicating 
   *    which points in the range should have the control applied. If the bit is
   *    set the control operation is applied to the corresponding point. The first 
   *    point is represented by the bit in bit 0 position of the first octet. The 
   *    second point by bit 1 position etc. 
   * returns:
   *  status of CROB operation. Valid values are:
   *    DNPDEFS_CROB_ST_SUCCESS - the command was performed successfully
   *    DNPDEFS_CROB_ST_TIMEOUT - the request was not accepted because the
   *      Operate message was received after the Arm timer timed out. The
   *      Arm timer is started when a Select operation for the same point
   *      was received.
   *    DNPDEFS_CROB_ST_NO_SELECT - No previously matching select message
   *      (i.e., an Operate message was sent to activate a control point
   *      that was not previously armed with a Select message.)
   *    DNPDEFS_CROB_ST_FORMAT_ERROR - the request was not accepted due
   *      to formatting errors in the request
   *    DNPDEFS_CROB_ST_NOT_SUPPORTED - the request was not accepted because
   *      Control operations are not supported for this point
   *    DNPDEFS_CROB_ST_ALREADY_ACTIVE - the request was not accepted because
   *      the control queue is full or the point is already active
   *    DNPDEFS_CROB_ST_HARDWARE_ERROR - the request was not accepted due to
   *      control hardware problems
   *    DNPDEFS_CROB_ST_LOCAL - the request was not accepted because 
   *      Local/Remote switch is in Local Position
   *    DNPDEFS_CROB_ST_NOT_AUTHORIZED - the request was not accepted because 
   *      of insufficient authorization.
   *    DNPDEFS_CROB_ST_AUTO_INHIBIT - the request was not accepted because it
   *      was prevented or inhibited by a local automation process  
   *    DNPDEFS_CROB_ST_PROC_LIMITED - the request was not accepted because the
   *     device cannot process any more activities than are presently in progress 
   *    DNPDEFS_CROB_ST_OUT_OF_RANGE - the request was not accepted because the 
   *     value is outside the acceptable range permitted for this point.  
   *    DNPDEFS_CROB_ST_DOWNSTRM_LOCAL - the request was not accepted because the 
   *     control is being forwarded to a downstream device that is reporting Local.
   *    DNPDEFS_CROB_ST_ALR_COMPLETE - the request was not accepted because the 
   *     operation is already complete. For example if the request is to close a 
   *     switch and the switch is already closed.
   *    DNPDEFS_CROB_ST_BLOCKED - the request was not accepted because it is 
   *     specifically blocked at the outstation.
   *    DNPDEFS_CROB_ST_CANCELLED - the request was not accepted because the 
   *     operation was cancelled.
   *    DNPDEFS_CROB_ST_BLOCKED_OM - the request was not accepted because another 
   *     master has exclusive rights to operate this point.
   *    DNPDEFS_CROB_ST_DOWNSTRM_FAIL -  the request was not accepted because the 
   *     control is being forwarded to a downstream device which cannot be reached 
   *     or is otherwise incapable of performing the request.
   *    DNPDEFS_CROB_ST_UNDEFINED - the request not accepted because of some 
   *     other undefined reason
   */
  DNPDEFS_CROB_ST sdnpdata_binOutOpPatternMask(
    void *pHandle,
    TMWTYPES_UCHAR control, 
    TMWTYPES_UCHAR count, 
    TMWTYPES_ULONG activationPeriod,
    TMWTYPES_ULONG deactivationPeriod, 
    TMWTYPES_USHORT firstPointNumber, 
    TMWTYPES_USHORT lastPointNumber, 
    TMWTYPES_UCHAR *pMask); 
    
  /* function: sdnpdata_binOutCancelPatMask */
  void TMWDEFS_GLOBAL sdnpdata_binOutCancelPatMask(
    void *pHandle,
    TMWTYPES_USHORT firstPointNumber,
    TMWTYPES_USHORT lastPointNumber,
    TMWTYPES_UCHAR *pMask);

  /* function: sdnpdata_binOutCmdEventDefVariation  
   * purpose: Determine default event variation for this binary output command
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   *  classMask - mask indicating what event class is being requested
   * returns:
   *  default variation for this point
   *  NOTE: this will only be called if the default variation for binary
   *   output command obj13DefaultVariation is configured as zero
   */
  TMWTYPES_UCHAR TMWDEFS_GLOBAL sdnpdata_binOutCmdEventDefVariation(
    void *pPoint,
    TMWDEFS_CLASS_MASK classMask);

  /* function: sdnpdata_binOutCmdEventMode 
   * purpose: Determine event mode for this point
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   * returns:
   *  event mode for this point, SOE or LAST
   *  NOTE: this will only be called if the event mode binary output command
   *    if binaryOutCmdEventMode is configured as TMWDEFS_EVENT_MODE_PER_POINT
   */
  TMWDEFS_EVENT_MODE TMWDEFS_GLOBAL sdnpdata_binOutCmdEventMode(
    void *pPoint);

  /* function: sdnpdata_binOutCmdEventClass
   * purpose: Return the class in which binary output command events from 
   *  this data point belong.
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   * returns:
   *  Class in which these events will be returned
   */
  TMWDEFS_CLASS_MASK TMWDEFS_GLOBAL sdnpdata_binOutCmdEventClass(
    void *pPoint);

  /* function: sdnpdata_binOutCmdChanged
   * purpose: Determine if the command status of the specified point has changed 
   *  and if so return the new value. This function is used to scan for events 
   *  on each data point. It will be called if SDNPSESN_CONFIG
   *  binaryOutCmdScanPeriod is nonzero.
   *  NOTE: this functionality is compiled out by defining 
   *  SDNPDATA_SUPPORT_OBJ13 FALSE  (default)
   *  or SDNPDATA_SUPPORT_EVENT_SCAN FALSE
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   *  pStatus - pointer to command status to be filled in, representing the control
   *   requested for the output.
   *   DNPDEFS_DBAS_FLAG_BINARY_ON or DNPDEFS_DBAS_FLAG_BINARY_OFF    
   *   or'ed with status of CROB operation. Valid values are:
   *    DNPDEFS_CROB_ST_SUCCESS - the command was performed successfully
   *    DNPDEFS_CROB_ST_TIMEOUT - the request was not accepted because the
   *      Operate message was received after the Arm timer timed out. The
   *      Arm timer is started when a Select operation for the same point
   *      was received.
   *    DNPDEFS_CROB_ST_NO_SELECT - No previously matching select message
   *      (i.e., an Operate message was sent to activate a control point
   *      that was not previously armed with a Select message.)
   *    DNPDEFS_CROB_ST_FORMAT_ERROR - the request was not accepted due
   *      to formatting errors in the request
   *    DNPDEFS_CROB_ST_NOT_SUPPORTED - the request was not accepted because
   *      Control operations are not supported for this point
   *    DNPDEFS_CROB_ST_ALREADY_ACTIVE - the request was not accepted because
   *      the control queue is full or the point is already active
   *    DNPDEFS_CROB_ST_HARDWARE_ERROR - the request was not accepted due to
   *      control hardware problems
   *    DNPDEFS_CROB_ST_LOCAL - the request was not accepted because 
   *      Local/Remote switch is in Local Position
   *    DNPDEFS_CROB_ST_NOT_AUTHORIZED - the request was not accepted because 
   *      of insufficient authorization.
   *    DNPDEFS_CROB_ST_AUTO_INHIBIT - the request was not accepted because it
   *      was prevented or inhibited by a local automation process  
   *    DNPDEFS_CROB_ST_PROC_LIMITED - the request was not accepted because the
   *     device cannot process any more activities than are presently in progress 
   *    DNPDEFS_CROB_ST_OUT_OF_RANGE - the request was not accepted because the 
   *     value is outside the acceptable range permitted for this point.  
   *    DNPDEFS_CROB_ST_DOWNSTRM_LOCAL - the request was not accepted because the 
   *     control is being forwarded to a downstream device that is reporting Local.
   *    DNPDEFS_CROB_ST_ALR_COMPLETE - the request was not accepted because the 
   *     operation is already complete. For example if the request is to close a 
   *     switch and the switch is already closed.
   *    DNPDEFS_CROB_ST_BLOCKED - the request was not accepted because it is
   *     specifically blocked at the outstation.
   *    DNPDEFS_CROB_ST_CANCELLED - the request was not accepted because the
   *     operation was cancelled.
   *    DNPDEFS_CROB_ST_BLOCKED_OM - the request was not accepted because another
   *     master has exclusive rights to operate this point.
   *    DNPDEFS_CROB_ST_DOWNSTRM_FAIL -  the request was not accepted because the
   *     control is being forwarded to a downstream device which cannot be reached
   *     or is otherwise incapable of performing the request.  
   *    DNPDEFS_CROB_ST_UNDEFINED - the request not accepted because of some 
   *     other undefined reason
   * returns:
   *  TMWDEFS_TRUE if the command status of point has changed, else TMWDEFS_FALSE
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_binOutCmdChanged(
    void *pPoint,
    TMWTYPES_UCHAR *pStatus);

  /* function: sdnpdata_binOutCmdAssignClass
   * purpose: Assign the class in which binary output command events from this 
   *  data point will belong.
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   *  classMask - new class in which to generate events from this point
   * returns:
   *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_binOutCmdAssignClass(
    void *pPoint,
    TMWDEFS_CLASS_MASK classMask);

  /* Binary Counters */

  /* function: sdnpdata_binCntrGetDescription  
   * purpose: Get description of this point
   *  NOTE: this is used only by sdnpxml/sdnpxml2.c if SDNPDATA_SUPPORT_XMLxx is TRUE
   *  code to generate the configuration file or Device Profile for this device. 
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.  
   * returns:
   *  pointer to string describing point
   *  TMWDEFS_NULL if failure
   */
  TMWTYPES_CHAR * TMWDEFS_GLOBAL sdnpdata_binCntrGetDescription(
    void *pPoint);

  /* function: sdnpdata_binCntrQuantity
   * purpose: Return the number of binary counters in the 
   *  specified database.
   * arguments:
   *  pHandle - handle to database returned from sdnpdata_init
   * returns:
   *  The number of binary counters (see note for
   *  sdnpdata_xxxQuantity at top of this file)
   */
  TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpdata_binCntrQuantity(
    void *pHandle);

  /* function: sdnpdata_binCntrGetPoint
   * purpose: Return a handle to a specific data point. This handle is
   *  used by the routines below to read values from this counter and to
   *  assign this counter to an event class.
   * arguments:
   *  pHandle - handle to database returned from sdnpdata_init
   *  pointNum - point number to return
   * returns:
   *  Pointer to specified data point or TMWDEFS_NULL if this point
   *   is currently disabled for reading.
   */
  void * TMWDEFS_GLOBAL sdnpdata_binCntrGetPoint(
    void *pHandle,
    TMWTYPES_USHORT pointNum);
  
  /* function: sdnpdata_binCntrGetFrzPoint
   * purpose: Return a handle to a specific data point. This handle is
   *  used by the routines below to freeze this counter. This is separate
   *  from the binCntrGetPoint() function to allow a running counter to not be 
   *  reported in response to a class 0 read, but still allow the counter to 
   *  be frozen, as required by the Application Layer Spec. 
   * arguments:
   *  pHandle - handle to database returned from sdnpdata_init
   *  pointNum - point number to return handle for
   * returns:
   *  Pointer to specified data point or TMWDEFS_NULL if this point does not 
   *  exist.
   */
  void * TMWDEFS_GLOBAL sdnpdata_binCntrGetFrzPoint(
    void *pHandle,
    TMWTYPES_USHORT pointNum);

  /* function: sdnpdata_binCntrIsClass0
   * purpose: Should this point be reported in response to an object 60 
   *   variation 1 read request. This allows individual points to be excluded
   *   from a class 0 response but still readable by a specific object group
   *   read request.
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   * returns:
   *  TMWDEFS_TRUE if point should be reported.
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_binCntrIsClass0(
    void *pPoint);

  /* function: sdnpdata_binCntrAssignClass
   * purpose: Assign the class in which events from this data point
   *  will belong.
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   *  classMask - new class in which to generate events from this point
   * returns:
   *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_binCntrAssignClass(
    void *pPoint,
    TMWDEFS_CLASS_MASK classMask);

  /* function: sdnpdata_binCntrDefVariation
   * purpose: Determine default static variation for this binary counter
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   * returns:
   *  default variation for this point
   *  NOTE: this will only be called if the default variation for 
   *  binary counter obj20DefaultVariation is configured as 0.
   */
  TMWTYPES_UCHAR TMWDEFS_GLOBAL sdnpdata_binCntrDefVariation(
    void *pPoint);

#if SDNPDATA_SUPPORT_OBJ22
  /* function: sdnpdata_binCntrEventDefVariation
   * purpose: Determine default variation for this counter
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   *  classMask - mask indicating what event class is being requested
   * returns:
   *  default variation for this point
   *  NOTE: this will only be called if the default variation for 
   *  counter change events obj22DefaultVariation is configured as 0.
   */
  TMWTYPES_UCHAR TMWDEFS_GLOBAL sdnpdata_binCntrEventDefVariation(
    void *pPoint,
    TMWDEFS_CLASS_MASK classMask);

  /* function: sdnpdata_binCntrEventClass
 * purpose: Return the class in which events from this data point
 *  belong.
 * arguments:
 *  pPoint - handle to data point returned from 'getPoint' function.
 * returns:
 *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
 */
  TMWDEFS_CLASS_MASK TMWDEFS_GLOBAL sdnpdata_binCntrEventClass(
    void *pPoint);

   /* function: sdnpdata_binCntrEventMode 
   * purpose: Determine event mode for this point
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   * returns:
   *  event mode for this point, SOE or LAST
   *  NOTE: this will only be called if the event mode binary counters
   *    if binaryCounterEventMode is configured as TMWDEFS_EVENT_MODE_PER_POINT
   */
  TMWDEFS_EVENT_MODE TMWDEFS_GLOBAL sdnpdata_binCntrEventMode(
    void *pPoint);
#endif

  /* function: sdnpdata_binCntrRead
   * purpose: Return the current value of the specified point.
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   *  pValue - pointer to location to store current value
   *  pFlags - pointer to location to store current DNP3 flags
   *   pFlags contains a status indication and the current state of the point.
   *   The following values (or OR'd combinations) are valid for this type:
   *      DNPDEFS_DBAS_FLAG_OFF_LINE - the point is off-line, and the returned
   *        state of this point may not be correct
   *      DNPDEFS_DBAS_FLAG_ON_LINE - the binary counter point has been read
   *        successfully
   *      DNPDEFS_DBAS_FLAG_RESTART - the field device that originated the
   *        data object has been restarted. This device may be the device
   *        reporting this data object.
   *      DNPDEFS_DBAS_FLAG_COMM_LOST - the device reporting this data object
   *        has lost communication with the originator of the data object
   *      DNPDEFS_DBAS_FLAG_REMOTE_FORCED - the state of the binary object
   *        has been forced to its current state at the originating device
   *      DNPDEFS_DBAS_FLAG_LOCAL_FORCED - the state of the binary object
   *        has been forced to its current state at the device reporting
   *        this data object
   *      DNPDEFS_DBAS_FLAG_CNTR_ROLLOVER - the accumulated value has exceeded
   *        has exceeded its maximum and rolled over to zero. The counter 
   *        value should be set to 0 upon rollover, and counting is resumed as 
   *        normal. The Rollover bit should be cleared when the counter value 
   *        and roll-over state have been reported.  
   *        NOTE: This maximum value is not necessarily equal to (2^32-1) for 
   *        32 bit counters or (2^16-1) for 16 bit counters. It can be different 
   *        for each counter instance. Technical Bulletin TB-2002-001 Counter 
   *        Objects recommends "slave devices do not set the Rollover flag and 
   *        that host(master) devices ignore the Rollover flag".
   *      DNPDEFS_DBAS_FLAG_DISCONTINUITY - value cannot be compared against 
   *        a prior value to obtain the correct count difference
   * returns:
   *  void 
   */
  void TMWDEFS_GLOBAL sdnpdata_binCntrRead(
    void *pPoint,
    TMWTYPES_ULONG *pValue, 
    TMWTYPES_UCHAR *pFlags);

  /* function: sdnpdata_binCntrChanged
   * purpose: Determine if the specified point has changed and if so
   *  return the new value. This function is used to scan for events on 
   *  each data point. It will be called if SDNPSESN_CONFIG
   *  binaryCounterScanPeriod is nonzero.
   *  NOTE: this functionality is compiled out by defining 
   *  SDNPDATA_SUPPORT_OBJ22 FALSE
   *  or SDNPDATA_SUPPORT_EVENT_SCAN FALSE
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   *  pFlags - pointer to location to store current DNP3 flags.
   *   The following values (or OR'd combinations) are valid for this type:
   *      DNPDEFS_DBAS_FLAG_OFF_LINE - the point is off-line, and the returned
   *        state of this point may not be correct
   *      DNPDEFS_DBAS_FLAG_ON_LINE - the binary counter point has been read
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
   *        has exceeded its maximum and rolled over to zero. The counter 
   *        value should be set to 0 upon rollover, and counting is resumed as 
   *        normal. The Rollover bit should be cleared when the counter value 
   *        and roll-over state have been reported.  
   *        NOTE: This maximum value is not necessarily equal to (2^32-1) for 
   *        32 bit counters or (2^16-1) for 16 bit counters. It can be different 
   *        for each counter instance. Technical Bulletin TB-2002-001 Counter 
   *        Objects recommends "slave devices do not set the Rollover flag and 
   *        that host(master) devices ignore the Rollover flag".
   *      DNPDEFS_DBAS_FLAG_DISCONTINUITY - value cannot be compared against 
   *        a prior value to obtain the correct count difference
   * returns:
   *  TMWDEFS_TRUE if the point has changed, else TMWDEFS_FALSE
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_binCntrChanged(
    void *pPoint,
    TMWTYPES_ULONG *pValue, 
    TMWTYPES_UCHAR *pFlags);

  /* function: sdnpdata_binCntrFreeze
   * purpose:  Freeze the specified counter
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   *  clearAfterFreeze - whether or not counter should be cleared after freeze
   * returns:
   *  TMWDEFS_TRUE if successful
   *  TMWDEFS_FALSE otherwise
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_binCntrFreeze(
    void *pPoint,
    TMWTYPES_BOOL clearAfterFreeze);

  /* function: sdnpdata_binCntrFreezeAtTime
   * purpose:  Freeze the specified counter at time
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   *  timeDateEnum - time-date field schedule interpretation
   *  pFreezeTime - time to perform freeze
   *  freezeInterval - time interval to perfrom periodic freezes (milliseconds)
   * returns:
   *  TMWDEFS_TRUE if successful
   *  TMWDEFS_FALSE otherwise
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_binCntrFreezeAtTime(
    void *pPoint,
    DNPDATA_FREEZE_TIME_DATE_FIELD timeDateEnum,
    TMWDTIME *pFreezeTime,
    TMWTYPES_ULONG freezeInterval);

  /* Frozen Counters */

  /* function: sdnpdata_frznCntrGetDescription
   * purpose: Get description of this point
   *  NOTE: this is used only by sdnpxml/sdnpxml2.c if SDNPDATA_SUPPORT_XMLxx is TRUE
   *  code to generate the configuration file or Device Profile for this device. 
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.  
   * returns:
   *  pointer to string describing point
   *  TMWDEFS_NULL if failure
   */
  TMWTYPES_CHAR * TMWDEFS_GLOBAL sdnpdata_frznCntrGetDescription(
    void *pPoint);

  /* function: sdnpdata_frznCntrQuantity
   * purpose: Return the number of frozen counters in the 
   *  specified database.
   * arguments:
   *  pHandle - handle to database returned from sdnpdata_init
   * returns:
   *  The number of frozen counters (see note for
   *  sdnpdata_xxxQuantity at top of this file)
   */
  TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpdata_frznCntrQuantity(
    void *pHandle);

  /* function: sdnpdata_frznCntrGetPoint
   * purpose: Return a handle to a specific data point. This handle is
   *  used by the routines below to read and write values and control
   *  information to this point.
   * arguments:
   *  pHandle - handle to database returned from sdnpdata_init
   *  pointNum - point number to return
   * returns:
   *  Pointer to specified data point or TMWDEFS_NULL if this point
   *   is currently disabled.
   */
  void * TMWDEFS_GLOBAL sdnpdata_frznCntrGetPoint(
    void *pHandle,
    TMWTYPES_USHORT pointNum);

  /* function: sdnpdata_frznCntrEventClass
   * purpose: Return the class in which events from this data point
   *  belong.
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   * returns:
   *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
   */
  TMWDEFS_CLASS_MASK TMWDEFS_GLOBAL sdnpdata_frznCntrEventClass(
    void *pPoint);

  /* function: sdnpdata_FrznCntrIsClass0
   * purpose: Should this point be reported in response to an object 60 
   *   variation 1 read request. This allows individual points to be excluded
   *   from a class 0 response but still readable by a specific object group
   *   read request.
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   * returns:
   *  TMWDEFS_TRUE if point should be reported.
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_frznCntrIsClass0(
    void *pPoint);

  /* function: sdnpdata_frznCntrAssignClass
   * purpose: Assign the class in which events from this data point
   *  will belong.
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   *  classMask - new class in which to generate events from this point
   * returns:
   *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_frznCntrAssignClass(
    void *pPoint,
    TMWDEFS_CLASS_MASK classMask);

  /* function: sdnpdata_frznCntrDefVariation 
   * purpose: Determine default static variation for this frozen counter
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   * returns:
   *  default variation for this point
   *  NOTE: this will only be called if the default variation for 
   *  frozen counter obj21DefaultVariation is configured as 0.
   */
  TMWTYPES_UCHAR TMWDEFS_GLOBAL sdnpdata_frznCntrDefVariation(
    void *pPoint);

  /* function: sdnpdata_frznCntrEventDefVariation  
   * purpose: Determine default variation for this frozen counter
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   *  classMask - mask indicating what event class is being requested
   * returns:
   *  default variation for this point
   *  NOTE: this will only be called if the default variation for frozen
   *  counter change events obj23DefaultVariation is configured as 0.
   */
  TMWTYPES_UCHAR TMWDEFS_GLOBAL sdnpdata_frznCntrEventDefVariation(
    void *pPoint,
    TMWDEFS_CLASS_MASK classMask);

   /* function: sdnpdata_frznCntrEventMode 
   * purpose: Determine event mode for this point
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   * returns:
   *  event mode for this point, SOE or LAST
   *  NOTE: this will only be called if the event mode frozen counters
   *    if frozenCounterEventMode is configured as TMWDEFS_EVENT_MODE_PER_POINT
   */
  TMWDEFS_EVENT_MODE TMWDEFS_GLOBAL sdnpdata_frznCntrEventMode(
    void *pPoint);
 
  /* function: sdnpdata_frznCntrRead
   * purpose: Return the current value of the specified point.
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   *  pValue - pointer to location to store current value
   *  pFlags - pointer to location to store current DNP3 flags
   *   The following values (or OR'd combinations) are valid for this type:
   *      DNPDEFS_DBAS_FLAG_OFF_LINE - the point is off-line, and the returned
   *        state of this point may not be correct
   *      DNPDEFS_DBAS_FLAG_ON_LINE - the frozen counter point has been read
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
   *        has exceeded its maximum and rolled over to zero. The counter 
   *        value should be set to 0 upon rollover, and counting is resumed as 
   *        normal. The Rollover bit should be cleared when the counter value 
   *        and roll-over state have been reported.  
   *        NOTE: This maximum value is not necessarily equal to (2^32-1) for 
   *        32 bit counters or (2^16-1) for 16 bit counters. It can be different 
   *        for each counter instance. Technical Bulletin TB-2002-001 Counter 
   *        Objects recommends "slave devices do not set the Rollover flag and 
   *        that host(master) devices ignore the Rollover flag".
   *      DNPDEFS_DBAS_FLAG_DISCONTINUITY - value cannot be compared against 
   *        a prior value to obtain the correct count difference
   *  pTimeOfFreeze - pointer to location to store time of freeze if known
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL sdnpdata_frznCntrRead(
    void *pPoint,
    TMWTYPES_ULONG *pValue, 
    TMWTYPES_UCHAR *pFlags, 
    TMWDTIME *pTimeOfFreeze);

  /* function: sdnpdata_frznCntrChanged
   * purpose: Determine if the specified point has changed and if so
   *  return the new value. This function is used to scan for events on 
   *  each data point. It will be called if SDNPSESN_CONFIG
   *  frozenCounterScanPeriod is nonzero.
   *  NOTE: this functionality is compiled out by defining 
   *  SDNPDATA_SUPPORT_OBJ23 FALSE
   *  or SDNPDATA_SUPPORT_EVENT_SCAN FALSE
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   *  pFlags - pointer to location to store current DNP3 flags
   *   The following values (or OR'd combinations) are valid for this type:
   *      DNPDEFS_DBAS_FLAG_OFF_LINE - the point is off-line, and the returned
   *        state of this point may not be correct
   *      DNPDEFS_DBAS_FLAG_ON_LINE - the frozen counter point has been read
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
   *        has exceeded its maximum and rolled over to zero. The counter 
   *        value should be set to 0 upon rollover, and counting is resumed as 
   *        normal. The Rollover bit should be cleared when the counter value 
   *        and roll-over state have been reported.  
   *        NOTE: This maximum value is not necessarily equal to (2^32-1) for 
   *        32 bit counters or (2^16-1) for 16 bit counters. It can be different 
   *        for each counter instance. Technical Bulletin TB-2002-001 Counter 
   *        Objects recommends "slave devices do not set the Rollover flag and 
   *        that host(master) devices ignore the Rollover flag".
   *      DNPDEFS_DBAS_FLAG_DISCONTINUITY - value cannot be compared against 
   *        a prior value to obtain the correct count difference
   * returns:
   *  TMWDEFS_TRUE if the point has changed, else TMWDEFS_FALSE
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_frznCntrChanged(
    void *pPoint,
    TMWTYPES_ULONG *pValue, 
    TMWTYPES_UCHAR *pFlags);

  /* Analog Inputs */

  /* function: sdnpdata_anlgInGetDescription
   * purpose: Get description of this point
   *  NOTE: this is used only by sdnpxml/sdnpxml2.c if SDNPDATA_SUPPORT_XMLxx is TRUE
   *  code to generate the configuration file or Device Profile for this device. 
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.  
   * returns:
   *  pointer to string describing point
   *  TMWDEFS_NULL if failure
   */
  TMWTYPES_CHAR * TMWDEFS_GLOBAL sdnpdata_anlgInGetDescription(
    void *pPoint);

  /* function: sdnpdata_anlgInQuantity
   * purpose: Return the number of analog input data points in the 
   *  specified database.
   * arguments:
   *  pHandle - handle to database returned from sdnpdata_init
   * returns:
   *  The number of analog inputs (see note for
   *  sdnpdata_xxxQuantity at top of this file)
   */
  TMWTYPES_USHORT TMWDEFS_CALLBACK sdnpdata_anlgInQuantity(
    void *pHandle);

  /* function: sdnpdata_anlgInGetPoint
   * purpose: Return a handle to a specific data point. This handle is
   *  used by the routines below to read and write values and control
   *  information to this point.
   * NOTE: This is not the function that will be called for Freeze
   *  This allows the analog input to be disabled for reading, but still
   *  can be frozen as required by TB2018-004
   * arguments:
   *  pHandle - handle to database returned from sdnpdata_init
   *  pointNum - point number to return
   * returns:
   *  Pointer to specified data point or TMWDEFS_NULL if this point
   *   is currently disabled.
   */
  void * TMWDEFS_CALLBACK sdnpdata_anlgInGetPoint(
    void *pHandle,
    TMWTYPES_USHORT pointNum);
   
  /* function: sdnpdata_anlgInGetFreezePoint
   * purpose: Return a handle to a specific data point. This handle is
   *  used by the routines below to freeze the analog input point.
   *  This allows the analog input to be disabled for reading, but still
   *  can be frozen as required by TB2018-004
   * arguments:
   *  pHandle - handle to database returned from sdnpdata_init
   *  pointNum - point number to return
   * returns:
   *  Pointer to specified data point or TMWDEFS_NULL if this point
   *   is currently disabled.
   */
  void * TMWDEFS_CALLBACK sdnpdata_anlgInGetFreezePoint(
    void *pHandle,
    TMWTYPES_USHORT pointNum);

  /* function: sdnpdata_anlgInDefVariation 
   * purpose: Determine default static variation for this analog input
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   * returns:
   *  default variation for this point
   *  NOTE: this will only be called if the default variation for 
   *  analog input obj30DefaultVariation is configured as 0.
   */
  TMWTYPES_UCHAR TMWDEFS_GLOBAL sdnpdata_anlgInDefVariation(
    void *pPoint);

  /* function: sdnpdata_anlgInDbandDefVar 
   * purpose: Determine default static variation for deadband for this analog 
   *  input
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   * returns:
   *  default variation for this point
   *  NOTE: this will only be called if the default variation for 
   *  analog input obj34DefaultVariation is configured as 0.
   */
  TMWTYPES_UCHAR TMWDEFS_GLOBAL sdnpdata_anlgInDbandDefVar(
    void *pPoint);

  /* function: sdnpdata_anlgInEventDefVariation   
   * purpose: Determine default variation for this frozen counter
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   *  classMask - mask indicating what event class is being requested
   * returns:
   *  default variation for this point
   *  NOTE: this will only be called if the default variation for 
   *   analog input change events obj32DefaultVariation is configured as zero
   */
  TMWTYPES_UCHAR TMWDEFS_GLOBAL sdnpdata_anlgInEventDefVariation(
    void *pPoint,
    TMWDEFS_CLASS_MASK classMask);
  
  /* function: sdnpdata_anlgInEventMode 
   * purpose: Determine event mode for this point
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   * returns:
   *  event mode for this point, SOE, LAST, or CURRENT
   *  NOTE: this will only be called if the event mode for analog inputs
   *    if analogInputEventMode is configured as TMWDEFS_EVENT_MODE_PER_POINT
   */
  TMWDEFS_EVENT_MODE TMWDEFS_GLOBAL sdnpdata_anlgInEventMode(
    void *pPoint);
 
  /* function: sdnpdata_anlgInEventClass
   * purpose: Return the class in which events from this data point
   *  belong.
   *  NOTE: if Frozen Analog Inputs are supported, read the description of 
   *   the proper behavior related to Class 0 and Event Class.
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   * returns:
   *  Class in which these events will be returned
   */
  TMWDEFS_CLASS_MASK TMWDEFS_GLOBAL sdnpdata_anlgInEventClass(
    void *pPoint);
  
  /* function: sdnpdata_anlgInIsClass0
   * purpose: Should this point be reported in response to an object 60 
   *   variation 1 read request. This allows individual points to be excluded
   *   from a class 0 response but still readable by a specific object group
   *   read request.
   *  NOTE: if Frozen Analog Inputs are supported, read the description of 
   *   the proper behavior related to Class 0 and Event Class.
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   * returns:
   *  TMWDEFS_TRUE if point should be reported.
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_anlgInIsClass0(
    void *pPoint);

  /* function: sdnpdata_anlgInAssignClass
   * purpose: Assign the class in which events from this data point
   *  will belong.
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   *  classMask - new class in which to generate events from this point
   * returns:
   *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_anlgInAssignClass(
    void *pPoint,
    TMWDEFS_CLASS_MASK classMask);

  /* function: sdnpdata_anlgInRead
   * purpose: Return the current value of the specified point.
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   *  pValue - pointer to location to store current value
   *   function should set pValue->type to indicate data type returned
   *     TMWTYPES_ANALOG_TYPE_DOUBLE
   *     TMWTYPES_ANALOG_TYPE_SFLOAT
   *     TMWTYPES_ANALOG_TYPE_LONG
   *     TMWTYPES_ANALOG_TYPE_SHORT
   *     TMWTYPES_ANALOG_TYPE_SCALED - allows database to return a single 
   *      precision floating point value to support a read of sfloat, AND a 
   *      long value to support a read of a scaled long representation of 
   *      the sfloat. This also allows the database to determine how the 
   *      floating point value would be rounded if an integer value is to
   *      be sent in a response.
   *     TMWTYPES_ANALOG_TYPE_DSCALED - allows database to return a double 
   *      precision floating point value to support a read of double, AND a 
   *      long value to support a read of a scaled long representation of 
   *      the double. This also allows the database to determine how the 
   *      floating point value would be rounded if an integer value is to
   *      be sent in a response.
   *   NOTE:  
   *    If TMWCNFG_SUPPORT_DOUBLE == TRUE returning the value as
   *    TMWTYPES_ANALOG_TYPE_DOUBLE allows for proper reads of DOUBLE FLOAT
   *    SFLOAT, ULONG or USHORT.
   *    If TMWCNFG_SUPPORT_DOUBLE == FALSE and 
   *       TMWCNFG_SUPPORT_SFLOAT == TRUE  
   *    To provide for best precision and OVER_RANGE flag setting:
   *    return the value as TMWTYPES_ANALOG_TYPE_LONG if the value is between
   *    TMWDEFS_LONG_MIN and TMWDEFS_LONG_MAX. 
   *    return the value as TMWTYPES_ANALOG_TYPE_SFLOAT if the value is outside
   *    of that range. While each type is 32 bits in size the long allows 
   *    for 31 bits of precision. Returning a float instead allows greater values,
   *    but only 23 bits of precision. 
   *    If both TMWCNFG_SUPPORT_DOUBLE and TMWCNFG_SUPPORT_SFLOAT == FALSE and
   *    the long value is over or under range, set DNPDEFS_DBAS_FLAG_OVER_RANGE 
   *    bit in pFlags and set lval to TMWDEFS_LONG_MIN or TMWDEFS_LONG_MAX.
   *
   *  pFlags - pointer to location to store current DNP3 flags
   *   The following values (or OR'd combinations) are valid for this type:
   *      DNPDEFS_DBAS_FLAG_OFF_LINE - the point is off-line, and the returned
   *        state of this point may not be correct
   *      DNPDEFS_DBAS_FLAG_ON_LINE - the analog input point has been read
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
   *        NOTE: If both TMWCNFG_SUPPORT_DOUBLE and TMWCNFG_SUPPORT_SFLOAT == 
   *        FALSE and the long value is over or under range, set
   *        DNPDEFS_DBAS_FLAG_OVER_RANGE bit in pFlags and set lval to 
   *        TMWDEFS_LONG_MIN or TMWDEFS_LONG_MAX.
   *      DNPDEFS_DBAS_FLAG_REFERENCE_CHK - the reference signal used to
   *        digitize the signal is not stable, and the resulting digitized
   *        value may not be correct.
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL sdnpdata_anlgInRead(
    void *pPoint,
    TMWTYPES_ANALOG_VALUE *pValue, 
    TMWTYPES_UCHAR *pFlags);

  /* function: sdnpdata_anlgInChanged
   * purpose: Determine if the specified point has changed and if so
   *  return the new value. This function is used to scan for events on
   *  each data point. It will be called if SDNPSESN_CONFIG
   *  analogInputScanPeriod is nonzero.
   *  NOTE: this functionality is compiled out by defining 
   *  SDNPDATA_SUPPORT_OBJ32 FALSE
   *  or SDNPDATA_SUPPORT_EVENT_SCAN FALSE
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   *  pValue - pointer to location to store current value
   *   function should set pValue->type to indicate data type returned
   *     TMWTYPES_ANALOG_TYPE_DOUBLE
   *     TMWTYPES_ANALOG_TYPE_SFLOAT
   *     TMWTYPES_ANALOG_TYPE_LONG
   *     TMWTYPES_ANALOG_TYPE_SHORT
   *     TMWTYPES_ANALOG_TYPE_SCALED - allows database to return a single precision
   *      floating point value to support a read of sfloat, AND a long value
   *      to support a read of a scaled long representation of the sfloat.
   *      This also allows the database to determine how the 
   *      floating point value would be rounded if an integer value is to
   *      be sent in a response.
   *     TMWTYPES_ANALOG_TYPE_DSCALED - allows database to return a double 
   *      precision floating point value to support a read of double, AND a 
   *      long value to support a read of a scaled long representation of 
   *      the double. This also allows the database to determine how the 
   *      floating point value would be rounded if an integer value is to
   *      be sent in a response.
   *   NOTE:  
   *    If TMWCNFG_SUPPORT_DOUBLE == TRUE returning the value as
   *    TMWTYPES_ANALOG_TYPE_DOUBLE allows for proper values for events of type
   *    DOUBLE FLOAT, SFLOAT, ULONG or USHORT.
   *
   *    If TMWCNFG_SUPPORT_DOUBLE == FALSE and 
   *       TMWCNFG_SUPPORT_SFLOAT == TRUE  
   *    To provide for best precision and OVER_RANGE flag setting:
   *    return the value as TMWTYPES_ANALOG_TYPE_LONG if the value is between
   *    TMWDEFS_LONG_MIN and TMWDEFS_LONG_MAX. 
   *    return the value as TMWTYPES_ANALOG_TYPE_SFLOAT if the value is outside
   *    of that range. While each type is 32 bits in size the long allows 
   *    for 31 bits of precision. Returning a float instead allows greater values,
   *    but only 23 bits of precision. 
   *    If both TMWCNFG_SUPPORT_DOUBLE and TMWCNFG_SUPPORT_SFLOAT == FALSE and
   *    the long value is over or under range, set DNPDEFS_DBAS_FLAG_OVER_RANGE 
   *    bit in pFlags and set lval to TMWDEFS_LONG_MIN or TMWDEFS_LONG_MAX.
   *
   *  pFlags - pointer to location to store current DNP3 flags
   *   The following values (or OR'd combinations) are valid for this type:
   *      DNPDEFS_DBAS_FLAG_OFF_LINE - the point is off-line, and the returned
   *        state of this point may not be correct
   *      DNPDEFS_DBAS_FLAG_ON_LINE - the analog input point has been read
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
   *        NOTE: If both TMWCNFG_SUPPORT_DOUBLE and TMWCNFG_SUPPORT_SFLOAT == 
   *        FALSE and the long value is over or under range, set
   *        DNPDEFS_DBAS_FLAG_OVER_RANGE bit in pFlags and set lval to 
   *        TMWDEFS_LONG_MIN or TMWDEFS_LONG_MAX.
   *      DNPDEFS_DBAS_FLAG_REFERENCE_CHK - the reference signal used to
   *        digitize the signal is not stable, and the resulting digitized
   *        value may not be correct.
   * returns:
   *  TMWDEFS_TRUE if the point has changed, else TMWDEFS_FALSE
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_anlgInChanged(
    void *pPoint,
    TMWTYPES_ANALOG_VALUE *pValue, 
    TMWTYPES_UCHAR *pFlags);

  /* function: sdnpdata_anlgInFreeze
   * purpose:  Freeze the specified analog input
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   *  clearAfterFreeze - whether or not analog input should be cleared after freeze
   * returns:
   *  TMWDEFS_TRUE if successful
   *  TMWDEFS_FALSE otherwise
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_anlgInFreeze(
    void *pPoint,
    TMWTYPES_BOOL clearAfterFreeze);

  /* function: sdnpdata_anlgInFreezeAtTime
   * purpose:  Freeze the specified analog input at time
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   *  timeDateEnum - time-date field schedule interpretation
   *  pFreezeTime - time to perform freeze
   *  freezeInterval - time interval to perfrom periodic freezes (milliseconds)
   * returns:
   *  TMWDEFS_TRUE if successful
   *  TMWDEFS_FALSE otherwise
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_anlgInFreezeAtTime(
    void *pPoint,
    DNPDATA_FREEZE_TIME_DATE_FIELD timeDateEnum,
    TMWDTIME *pFreezeTime,
    TMWTYPES_ULONG freezeInterval);
  /* Frozen Analog Inputs */

  /* function: sdnpdata_frznAnlgInGetDescription
   * purpose: Get description of this point
   *  NOTE: this is used only by sdnpxml/sdnpxml2.c if SDNPDATA_SUPPORT_XMLxx is TRUE
   *  code to generate the configuration file or Device Profile for this device. 
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.  
   * returns:
   *  pointer to string describing point
   *  TMWDEFS_NULL if failure
   */
  TMWTYPES_CHAR * TMWDEFS_GLOBAL sdnpdata_frznAnlgInGetDescription(
    void *pPoint);

  /* function: sdnpdata_frznAnlgInQuantity
   * purpose: Return the number of analog input data points in the 
   *  specified database.
   * arguments:
   *  pHandle - handle to database returned from sdnpdata_init
   * returns:
   *  The number of analog inputs (see note for
   *  sdnpdata_xxxQuantity at top of this file)
   */
  TMWTYPES_USHORT TMWDEFS_CALLBACK sdnpdata_frznAnlgInQuantity(
    void *pHandle);

  /* function: sdnpdata_frznAnlgInGetPoint
   * purpose: Return a handle to a specific data point. This handle is
   *  used by the routines below to read and write values and control
   *  information to this point.
   * arguments:
   *  pHandle - handle to database returned from sdnpdata_init
   *  pointNum - point number to return
   * returns:
   *  Pointer to specified data point or TMWDEFS_NULL if this point
   *   is currently disabled.
   */
  void * TMWDEFS_CALLBACK sdnpdata_frznAnlgInGetPoint(
    void *pHandle,
    TMWTYPES_USHORT pointNum);
    
  /* function: sdnpdata_frznAnlgInDefVariation 
   * purpose: Determine default static variation for this analog input
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   * returns:
   *  default variation for this point
   *  NOTE: this will only be called if the default variation for 
   *  analog input obj30DefaultVariation is configured as 0.
   */
  TMWTYPES_UCHAR TMWDEFS_GLOBAL sdnpdata_frznAnlgInDefVariation(
    void *pPoint);

  /* function: sdnpdata_frznAnlgInEventDefVariation   
   * purpose: Determine default variation for this frozen counter
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   *  classMask - mask indicating what event class is being requested
   * returns:
   *  default variation for this point
   *  NOTE: this will only be called if the default variation for 
   *   analog input change events obj32DefaultVariation is configured as zero
   */
  TMWTYPES_UCHAR TMWDEFS_GLOBAL sdnpdata_frznAnlgInEventDefVariation(
    void *pPoint,
    TMWDEFS_CLASS_MASK classMask);
  
  /* function: sdnpdata_frznAnlgInEventMode 
   * purpose: Determine event mode for this point
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   * returns:
   *  event mode for this point, SOE, LAST, or CURRENT
   *  NOTE: this will only be called if the event mode for analog inputs
   *    if analogInputEventMode is configured as TMWDEFS_EVENT_MODE_PER_POINT
   */
  TMWDEFS_EVENT_MODE TMWDEFS_GLOBAL sdnpdata_frznAnlgInEventMode(
    void *pPoint);
 
  /* function: sdnpdata_frznAnlgInEventClass
   * purpose: Return the class in which events from this data point
   *  belong.
   *  NOTE: Each AnlgIn pointNum x may generate either Obj32 or Obj33 event
   *   but not both, as required by TB2018-004. If this function returns an
   *   event class for point x then sdnpdata_anlgInEventClass should return
   *    TMWDEFS_CLASS_MASK_NONE.
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   * returns:
   *  Class in which these events will be returned
   */
  TMWDEFS_CLASS_MASK TMWDEFS_GLOBAL sdnpdata_frznAnlgInEventClass(
    void *pPoint);
  
  /* function: sdnpdata_frznAnlgInIsClass0
   * purpose: Should this point be reported in response to an object 60 
   *   variation 1 read request. This allows individual points to be excluded
   *   from a class 0 response but still readable by a specific object group
   *   read request.
   *  NOTE: Each AnlgIn pointNum x may send Obj30 or Obj31 in response to
   *   a Class0 poll, but not both, as required by TB2018-004. If this function
   *    returns TMWDEFS_TRUE for point x then sdnpdata_anlgInIsClass0 should 
   *    return TMWDEFS_FALSE.
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   * returns:
   *  TMWDEFS_TRUE if point should be reported.
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_frznAnlgInIsClass0(
    void *pPoint);

  /* function: sdnpdata_frznAnlgInAssignClass
   * purpose: Assign the class in which events from this data point
   *  will belong.
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   *  classMask - new class in which to generate events from this point
   * returns:
   *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_frznAnlgInAssignClass(
    void *pPoint,
    TMWDEFS_CLASS_MASK classMask);

  /* function: sdnpdata_frznAnlgInRead
   * purpose: Return the current value of the specified point.
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   *  pValue - pointer to location to store current value
   *   function should set pValue->type to indicate data type returned
   *     TMWTYPES_ANALOG_TYPE_DOUBLE
   *     TMWTYPES_ANALOG_TYPE_SFLOAT
   *     TMWTYPES_ANALOG_TYPE_LONG
   *     TMWTYPES_ANALOG_TYPE_SHORT
   *     TMWTYPES_ANALOG_TYPE_SCALED - allows database to return a single 
   *      precision floating point value to support a read of sfloat, AND a 
   *      long value to support a read of a scaled long representation of 
   *      the sfloat. This also allows the database to determine how the 
   *      floating point value would be rounded if an integer value is to
   *      be sent in a response.
   *     TMWTYPES_ANALOG_TYPE_DSCALED - allows database to return a double 
   *      precision floating point value to support a read of double, AND a 
   *      long value to support a read of a scaled long representation of 
   *      the double. This also allows the database to determine how the 
   *      floating point value would be rounded if an integer value is to
   *      be sent in a response.
   *   NOTE:  
   *    If TMWCNFG_SUPPORT_DOUBLE == TRUE returning the value as
   *    TMWTYPES_ANALOG_TYPE_DOUBLE allows for proper reads of DOUBLE FLOAT
   *    SFLOAT, ULONG or USHORT.
   *    If TMWCNFG_SUPPORT_DOUBLE == FALSE and 
   *       TMWCNFG_SUPPORT_SFLOAT == TRUE  
   *    To provide for best precision and OVER_RANGE flag setting:
   *    return the value as TMWTYPES_ANALOG_TYPE_LONG if the value is between
   *    TMWDEFS_LONG_MIN and TMWDEFS_LONG_MAX. 
   *    return the value as TMWTYPES_ANALOG_TYPE_SFLOAT if the value is outside
   *    of that range. While each type is 32 bits in size the long allows 
   *    for 31 bits of precision. Returning a float instead allows greater values,
   *    but only 23 bits of precision. 
   *    If both TMWCNFG_SUPPORT_DOUBLE and TMWCNFG_SUPPORT_SFLOAT == FALSE and
   *    the long value is over or under range, set DNPDEFS_DBAS_FLAG_OVER_RANGE 
   *    bit in pFlags and set lval to TMWDEFS_LONG_MIN or TMWDEFS_LONG_MAX.
   *
   *  pFlags - pointer to location to store current DNP3 flags
   *   The following values (or OR'd combinations) are valid for this type:
   *      DNPDEFS_DBAS_FLAG_OFF_LINE - the point is off-line, and the returned
   *        state of this point may not be correct
   *      DNPDEFS_DBAS_FLAG_ON_LINE - the analog input point has been read
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
   *        NOTE: If both TMWCNFG_SUPPORT_DOUBLE and TMWCNFG_SUPPORT_SFLOAT == 
   *        FALSE and the long value is over or under range, set
   *        DNPDEFS_DBAS_FLAG_OVER_RANGE bit in pFlags and set lval to 
   *        TMWDEFS_LONG_MIN or TMWDEFS_LONG_MAX.
   *      DNPDEFS_DBAS_FLAG_REFERENCE_CHK - the reference signal used to
   *        digitize the signal is not stable, and the resulting digitized
   *        value may not be correct.
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL sdnpdata_frznAnlgInRead(
    void *pPoint,
    TMWTYPES_ANALOG_VALUE *pValue, 
    TMWTYPES_UCHAR *pFlags,
    TMWDTIME *pTimeOfFreeze);

  /* function: sdnpdata_frznAnlgInChanged
   * purpose: Determine if the specified point has changed and if so
   *  return the new value. This function is used to scan for events on
   *  each data point. It will be called if SDNPSESN_CONFIG
   *  analogInputScanPeriod is nonzero.
   *  NOTE: this functionality is compiled out by defining 
   *  SDNPDATA_SUPPORT_OBJ32 FALSE
   *  or SDNPDATA_SUPPORT_EVENT_SCAN FALSE
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   *  pValue - pointer to location to store current value
   *   function should set pValue->type to indicate data type returned
   *     TMWTYPES_ANALOG_TYPE_DOUBLE
   *     TMWTYPES_ANALOG_TYPE_SFLOAT
   *     TMWTYPES_ANALOG_TYPE_LONG
   *     TMWTYPES_ANALOG_TYPE_SHORT
   *     TMWTYPES_ANALOG_TYPE_SCALED - allows database to return a single precision
   *      floating point value to support a read of sfloat, AND a long value
   *      to support a read of a scaled long representation of the sfloat.
   *      This also allows the database to determine how the 
   *      floating point value would be rounded if an integer value is to
   *      be sent in a response.
   *     TMWTYPES_ANALOG_TYPE_DSCALED - allows database to return a double 
   *      precision floating point value to support a read of double, AND a 
   *      long value to support a read of a scaled long representation of 
   *      the double. This also allows the database to determine how the 
   *      floating point value would be rounded if an integer value is to
   *      be sent in a response.
   *   NOTE:  
   *    If TMWCNFG_SUPPORT_DOUBLE == TRUE returning the value as
   *    TMWTYPES_ANALOG_TYPE_DOUBLE allows for proper values for events of type
   *    DOUBLE FLOAT, SFLOAT, ULONG or USHORT.
   *
   *    If TMWCNFG_SUPPORT_DOUBLE == FALSE and 
   *       TMWCNFG_SUPPORT_SFLOAT == TRUE  
   *    To provide for best precision and OVER_RANGE flag setting:
   *    return the value as TMWTYPES_ANALOG_TYPE_LONG if the value is between
   *    TMWDEFS_LONG_MIN and TMWDEFS_LONG_MAX. 
   *    return the value as TMWTYPES_ANALOG_TYPE_SFLOAT if the value is outside
   *    of that range. While each type is 32 bits in size the long allows 
   *    for 31 bits of precision. Returning a float instead allows greater values,
   *    but only 23 bits of precision. 
   *    If both TMWCNFG_SUPPORT_DOUBLE and TMWCNFG_SUPPORT_SFLOAT == FALSE and
   *    the long value is over or under range, set DNPDEFS_DBAS_FLAG_OVER_RANGE 
   *    bit in pFlags and set lval to TMWDEFS_LONG_MIN or TMWDEFS_LONG_MAX.
   *
   *  pFlags - pointer to location to store current DNP3 flags
   *   The following values (or OR'd combinations) are valid for this type:
   *      DNPDEFS_DBAS_FLAG_OFF_LINE - the point is off-line, and the returned
   *        state of this point may not be correct
   *      DNPDEFS_DBAS_FLAG_ON_LINE - the analog input point has been read
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
   *        NOTE: If both TMWCNFG_SUPPORT_DOUBLE and TMWCNFG_SUPPORT_SFLOAT == 
   *        FALSE and the long value is over or under range, set
   *        DNPDEFS_DBAS_FLAG_OVER_RANGE bit in pFlags and set lval to 
   *        TMWDEFS_LONG_MIN or TMWDEFS_LONG_MAX.
   *      DNPDEFS_DBAS_FLAG_REFERENCE_CHK - the reference signal used to
   *        digitize the signal is not stable, and the resulting digitized
   *        value may not be correct.
   * returns:
   *  TMWDEFS_TRUE if the point has changed, else TMWDEFS_FALSE
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_frznAnlgInChanged(
    void *pPoint,
    TMWTYPES_ANALOG_VALUE *pValue, 
    TMWTYPES_UCHAR *pFlags);

#if SDNPDATA_SUPPORT_OBJ34
  /* Analog Input Deadbands */

  /* function: sdnpdata_anlgInDBandGetDescription
   * purpose: Get description of this point
   *  NOTE: this is used only by sdnpxml/sdnpxml2.c if SDNPDATA_SUPPORT_XMLxx is TRUE
   *  code to generate the configuration file or Device Profile for this device. 
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.  
   * returns:
   *  pointer to string describing point
   *  TMWDEFS_NULL if failure
   */
  TMWTYPES_CHAR * TMWDEFS_GLOBAL sdnpdata_anlgInDBandGetDescription(
    void *pPoint);

  /* function: sdnpdata_anlgInDBandQuantity
   * purpose: Return the number of analog input deadbands in the 
   *  specified database.
   * arguments:
   *  pHandle - handle to database returned from sdnpdata_init
   * returns:
   *  The number of analog input deadbands (see note for
   *  sdnpdata_xxxQuantity at top of this file)
   */
  TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpdata_anlgInDBandQuantity(
    void *pHandle);

  /* function: sdnpdata_anlgInDBandGetPoint
   * purpose: Return a handle to a specific data point. This handle is
   *  used by the routines below to read and write values and control
   *  information to this point.
   * arguments:
   *  pHandle - handle to database returned from sdnpdata_init
   *  pointNum - point number to return
   * returns:
   *  Pointer to specified data point or TMWDEFS_NULL if this point
   *   is currently disabled.
   */
  void * TMWDEFS_GLOBAL sdnpdata_anlgInDBandGetPoint(
    void *pHandle,
    TMWTYPES_USHORT pointNum);

  /* function: sdnpdata_anlgInDBandRead
   * purpose: Return the current value of the specified point. 
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   *  pValue - pointer to location to hold current value
   *   set pValue->type to indicate data type returned.
   *   NOTE:  
   *    If TMWCNFG_SUPPORT_DOUBLE == TRUE returning the value as
   *    pValue->type = TMWTYPES_ANALOG_TYPE_DOUBLE allows for reads of
   *    SFLOAT, ULONG or USHORT.
   *
   *    If TMWCNFG_SUPPORT_DOUBLE == FALSE and 
   *       TMWCNFG_SUPPORT_SFLOAT == TRUE  
   *    To provide for best precision:
   *    return the value as TMWTYPES_ANALOG_TYPE_ULONG if the value is between
   *    0 and TMWDEFS_ULONG_MAX. 
   *    return the value as TMWTYPES_ANALOG_TYPE_SFLOAT if the value is outside
   *    of that range. While each is 32 bits in size the ulong allows 
   *    for 32 bits of precision. Returning a float instead allows greater values,
   *    but only 23 bits of precision.
   *    If both TMWCNFG_SUPPORT_DOUBLE and TMWCNFG_SUPPORT_SFLOAT == FALSE and
   *    the long value exceeds TMWDEFS_ULONG_MAX, return TMWDEFS_ULONG_MAX.
   *   NOTE: Deadband values should be greater than zero. Negative numbers are not
   *    allowed by the specification.
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL sdnpdata_anlgInDBandRead(
    void *pPoint,
    TMWTYPES_ANALOG_VALUE *pValue);

  /* function: sdnpdata_anlgInDBandWrite
   * purpose: Write the specified deadband value
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   *  *pValue - pointer to new deadband value to be stored 
   *   pValue->type indicates storage type  
   *    TMWTYPES_ANALOG_TYPE_SFLOAT
   *    TMWTYPES_ANALOG_TYPE_ULONG
   *    TMWTYPES_ANALOG_TYPE_USHORT 
   * returns:
   *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_anlgInDBandWrite(
    void *pPoint,
    TMWTYPES_ANALOG_VALUE *pValue);
#endif

  /* Analog Outputs */

  /* function: sdnpdata_anlgOutGetDescription
   * purpose: Get description of this point
   *  NOTE: this is used only by sdnpxml/sdnpxml2.c if SDNPDATA_SUPPORT_XMLxx is TRUE
   *  code to generate the configuration file or Device Profile for this device. 
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.  
   * returns:
   *  pointer to string describing point
   *  TMWDEFS_NULL if failure
   */
  TMWTYPES_CHAR * TMWDEFS_GLOBAL sdnpdata_anlgOutGetDescription(
    void *pPoint);

  /* function: sdnpdata_anlgOutQuantity
   * purpose: Return the number of analog output data points in the 
   *  specified database.
   * arguments:
   *  pHandle - handle to database returned from sdnpdata_init
   * returns:
   *  The number of analog outputs data points (see note for
   *  sdnpdata_xxxQuantity at top of this file)
   */
  TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpdata_anlgOutQuantity(
    void *pHandle);

  /* function: sdnpdata_anlgOutGetPoint
   * purpose: Return a handle to a specific data point. This handle is
   *  used by the routines below to read and write values and control
   *  information to this point.
   * arguments:
   *  pHandle - handle to database returned from sdnpdata_init
   *  pointNum - point number to return
   * returns:
   *  Pointer to specified data point or TMWDEFS_NULL if this point
   *   is currently disabled.
   */
  void * TMWDEFS_GLOBAL sdnpdata_anlgOutGetPoint(
    void *pHandle,
    TMWTYPES_USHORT pointNum);

  /* function: sdnpdata_anlgOutDefVariation 
   * purpose: Determine default static variation for this analog output
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   * returns:
   *  default variation for this point
   *  NOTE: this will only be called if the default variation for 
   *  analog output obj40DefaultVariation is configured as 0.
   */
  TMWTYPES_UCHAR TMWDEFS_GLOBAL sdnpdata_anlgOutDefVariation(
    void *pPoint);

  /* function: sdnpdata_anlgOutEventDefVariation   
   * purpose: Determine default event variation for this analog output
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   *  classMask - mask indicating what event class is being requested
   * returns:
   *  default variation for this point
   *  NOTE: this will only be called if the default variation for 
   *   analog input change events obj42DefaultVariation is configured as zero
   */
  TMWTYPES_UCHAR TMWDEFS_GLOBAL sdnpdata_anlgOutEventDefVariation(
    void *pPoint,
    TMWDEFS_CLASS_MASK classMask);

  /* function: sdnpdata_anlgOutEventMode 
   * purpose: Determine event mode for this point
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   * returns:
   *  event mode for this point, SOE or LAST
   *  NOTE: this will only be called if the event mode for analog outputs
   *    if analogOutputEventMode is configured as TMWDEFS_EVENT_MODE_PER_POINT
   */
  TMWDEFS_EVENT_MODE TMWDEFS_GLOBAL sdnpdata_anlgOutEventMode(
    void *pPoint);
 
  /* function: sdnpdata_anlgOutEventClass
   * purpose: Return the class in which output events from this data point
   *  belong.
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   * returns:
   *  Class in which these events will be returned
   */
  TMWDEFS_CLASS_MASK TMWDEFS_GLOBAL sdnpdata_anlgOutEventClass(
    void *pPoint); 
  
  /* function: sdnpdata_anlgOutIsClass0
   * purpose: Should this point be reported in response to an object 60 
   *   variation 1 read request. This allows individual points to be excluded
   *   from a class 0 response but still readable by a specific object group
   *   read request.
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   * returns:
   *  TMWDEFS_TRUE if point should be reported.
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_anlgOutIsClass0(
    void *pPoint);

  /* function: sdnpdata_anlgOutAssignClass
   * purpose: Assign the class in which output events from this data point
   *  will belong.
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   *  classMask - new class in which to generate events from this point
   * returns:
   *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_anlgOutAssignClass(
    void *pPoint,
    TMWDEFS_CLASS_MASK classMask);

  /* function: sdnpdata_anlgOutRead
   * purpose: Return the current value of the specified point.
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   *  pValue - pointer to location to store current value
   *    set pValue->type to indicate data type returned.
   *   NOTE:  
   *    If TMWCNFG_SUPPORT_DOUBLE == TRUE returning the value as
   *    TMWTYPES_ANALOG_TYPE_DOUBLE allows for proper events of type
   *    DOUBLE FLOAT SFLOAT, ULONG or USHORT.
   *
   *    If TMWCNFG_SUPPORT_DOUBLE == FALSE and 
   *       TMWCNFG_SUPPORT_SFLOAT == TRUE  
   *    To provide for best precision and OVER_RANGE flag setting:
   *    return the value as TMWTYPES_ANALOG_TYPE_LONG if the value is between
   *    TMWDEFS_LONG_MIN and TMWDEFS_LONG_MAX. 
   *    return the value as TMWTYPES_ANALOG_TYPE_SFLOAT if the value is outside
   *    of that range. While each is 32 bits in size the long allows 
   *    for 31 bits of precision. Returning a float instead allows greater values,
   *    but only 23 bits of precision. 
   *    If both TMWCNFG_SUPPORT_DOUBLE and TMWCNFG_SUPPORT_SFLOAT == FALSE and
   *    the long value is over or under range, set DNPDEFS_DBAS_FLAG_OVER_RANGE 
   *    bit in pFlags and set lval to TMWDEFS_LONG_MIN or TMWDEFS_LONG_MAX.
   *
   *  pFlags - pointer to location to store current DNP3 flags
   *   The following values (or OR'd combinations) are valid for this type:
   *      DNPDEFS_DBAS_FLAG_OFF_LINE - the point is off-line, and the returned
   *        state of this point may not be correct
   *      DNPDEFS_DBAS_FLAG_ON_LINE - the analog output point has been read
   *        successfully
   *      DNPDEFS_DBAS_FLAG_RESTART - the field device that originated the
   *        data object has been restarted. This device may be the deviced
   *        reporting this data object.
   *      DNPDEFS_DBAS_FLAG_COMM_LOST - the device reporting this data object
   *        has lost communication with the originator of the data object
   *      DNPDEFS_DBAS_FLAG_REMOTE_FORCED - the state of the analog object
   *        has been forced to its current state at the originating device
   *      DNPDEFS_DBAS_FLAG_LOCAL_FORCED - the state of the analog object
   *        has been forced to its current state at the device reporting
   *      DNPDEFS_DBAS_FLAG_OVER_RANGE - the digitized signal or calculation
   *        is greater than the type specified in TMWTYPES_ANALOG_VALUE. If the
   *        SCL determines that the value returned cannot fit in the type 
   *        specified by the object variation read it will set this OVER_RANGE bit.
   *        NOTE: If both TMWCNFG_SUPPORT_DOUBLE and TMWCNFG_SUPPORT_SFLOAT == 
   *        FALSE and the long value is over or under range, set
   *        DNPDEFS_DBAS_FLAG_OVER_RANGE bit in pFlags and set lval to 
   *        TMWDEFS_LONG_MIN or TMWDEFS_LONG_MAX.
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL sdnpdata_anlgOutRead(
    void *pPoint,
    TMWTYPES_ANALOG_VALUE *pValue, 
    TMWTYPES_UCHAR *pFlags);

  /* function: sdnpdata_anlgOutChanged
   * purpose: Determine if the specified point has changed and if so
   *  return the new value. This function is used to scan for events on
   *  each data point. This function is called to determine whether an Object
   *  Group 42 Analog Output Event should be queued. It will be called if 
   *  SDNPSESN_CONFIG analogOutputScanPeriod is nonzero.
   *  NOTE: this functionality is compiled out by defining 
   *  SDNPDATA_SUPPORT_OBJ42 FALSE (default)
   *  or SDNPDATA_SUPPORT_EVENT_SCAN FALSE
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   *  pValue - pointer to location to store current value
   *    set pValue->type to indicate data type returned.
   *   NOTE:  
   *    If TMWCNFG_SUPPORT_DOUBLE == TRUE returning the value as
   *    TMWTYPES_ANALOG_TYPE_DOUBLE allows for proper events of type
   *    DOUBLE FLOAT SFLOAT, ULONG or USHORT.
   *
   *    If TMWCNFG_SUPPORT_DOUBLE == FALSE and 
   *       TMWCNFG_SUPPORT_SFLOAT == TRUE  
   *    To provide for best precision and OVER_RANGE flag setting:
   *    return the value as TMWTYPES_ANALOG_TYPE_LONG if the value is between
   *    TMWDEFS_LONG_MIN and TMWDEFS_LONG_MAX. 
   *    return the value as TMWTYPES_ANALOG_TYPE_SFLOAT if the value is outside
   *    of that range. While each is 32 bits in size the long allows 
   *    for 31 bits of precision. Returning a float instead allows greater values,
   *    but only 23 bits of precision. 
   *    If both TMWCNFG_SUPPORT_DOUBLE and TMWCNFG_SUPPORT_SFLOAT == FALSE and
   *    the long value is over or under range, set DNPDEFS_DBAS_FLAG_OVER_RANGE 
   *    bit in pFlags and set lval to TMWDEFS_LONG_MIN or TMWDEFS_LONG_MAX.
   *
   *  pFlags - pointer to location to store current DNP3 flags
   *   The following values (or OR'd combinations) are valid for this type:
   *      DNPDEFS_DBAS_FLAG_OFF_LINE - the point is off-line, and the returned
   *        state of this point may not be correct
   *      DNPDEFS_DBAS_FLAG_ON_LINE - the analog output point has been read
   *        successfully
   *      DNPDEFS_DBAS_FLAG_RESTART - the field device that originated the
   *        data object has been restarted. This device may be the device
   *        reporting this data object.
   *      DNPDEFS_DBAS_FLAG_COMM_LOST - the device reporting this data object
   *        has lost communication with the originator of the data object
   *      DNPDEFS_DBAS_FLAG_REMOTE_FORCED - the state of the analog object
   *        has been forced to its current state at the originating device
   *      DNPDEFS_DBAS_FLAG_LOCAL_FORCED - the state of the analog object
   *        has been forced to its current state at the device reporting
   *      DNPDEFS_DBAS_FLAG_OVER_RANGE - the digitized signal or calculation
   *        is greater than the type specified in TMWTYPES_ANALOG_VALUE. If the
   *        SCL determines that the value returned cannot fit in the type 
   *        specified by the object variation read it will set this OVER_RANGE bit.
   *        NOTE: If both TMWCNFG_SUPPORT_DOUBLE and TMWCNFG_SUPPORT_SFLOAT == 
   *        FALSE and the long value is over or under range, set
   *        DNPDEFS_DBAS_FLAG_OVER_RANGE bit in pFlags and set lval to 
   *        TMWDEFS_LONG_MIN or TMWDEFS_LONG_MAX.
   * returns:
   *  TMWDEFS_TRUE if the point has changed, else TMWDEFS_FALSE
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_anlgOutChanged(
    void *pPoint,
    TMWTYPES_ANALOG_VALUE *pValue, 
    TMWTYPES_UCHAR *pFlags);

  /* function: sdnpdata_anlgOutSelect
   * purpose: perform Select operation on the specified point and return
   *   the result.
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   *  pValue - pointer to location to containing value
   *   pValue->type will indicate storage type
   *   TMWTYPES_ANALOG_TYPE_LONG 
   *   TMWTYPES_ANALOG_TYPE_SHORT 
   *   TMWTYPES_ANALOG_TYPE_SFLOAT
   *   TMWTYPES_ANALOG_TYPE_DOUBLE
   * returns:
   *  status of Analog Output operation. Valid values are:
   *    DNPDEFS_CTLSTAT_SUCCESS - the command was performed successfully
   *    DNPDEFS_CTLSTAT_FORMAT_ERROR - The request was not accepted due
   *      to formatting errors in the request. This value may also be use
   *      to indicate that the value in the request exceeds the permitted
   *      level (see Table 4-2 in DNP3 Specification Volume 2, Application
   *      Layer.)
   *    DNPDEFS_CTLSTAT_NOT_SUPPORTED - the request was not accepted because
   *      Control operations are not supported for this point
   *    DNPDEFS_CTLSTAT_ALREADY_ACTIVE - the request was not accepted because
   *      the control queue is full or the point is already active
   *    DNPDEFS_CTLSTAT_HARDWARE_ERROR - the request was not accepted due to
   *      control hardware problems
   *    DNPDEFS_CTLSTAT_LOCAL - the request was not accepted because 
   *      Local/Remote switch is in Local Position
   *    DNPDEFS_CTLSTAT_NOT_AUTHORIZED - the request was not accepted because 
   *      of insufficient authorization.
   *    DNPDEFS_CTLSTAT_AUTO_INHIBIT - the request was not accepted because it
   *      was prevented or inhibited by a local automation process  
   *    DNPDEFS_CTLSTAT_PROC_LIMITED - the request was not accepted because the
   *     device cannot process any more activities than are presently in progress 
   *    DNPDEFS_CTLSTAT_OUT_OF_RANGE - the request was not accepted because the 
   *     value is outside the acceptable range permitted for this point.    
   *    DNPDEFS_CTLSTAT_DOWNSTRM_LOCAL - the request was not accepted because the 
   *     control is being forwarded to a downstream device that is reporting Local.
   *    DNPDEFS_CTLSTAT_ALR_COMPLETE - the request was not accepted because the 
   *     operation is already complete. For example if the request is to close a 
   *     switch and the switch is already closed.
   *    DNPDEFS_CTLSTAT_BLOCKED - the request was not accepted because it is 
   *     specifically blocked at the outstation.
   *    DNPDEFS_CTLSTAT_CANCELLED - the request was not accepted because the 
   *     operation was cancelled.
   *    DNPDEFS_CTLSTAT_BLOCKED_OM - the request was not accepted because another 
   *     master has exclusive rights to operate this point.
   *    DNPDEFS_CTLSTAT_DOWNSTRM_FAIL -  the request was not accepted because the 
   *     or control is being forwarded to a downstream device which cannot be reached 
   *     is otherwise incapable of performing the request.
   *    DNPDEFS_CTLSTAT_UNDEFINED - the request not accepted because of some 
   *     other undefined reason
   *
   *   NOTE- if pValue->type TMWTYPES_ANALOG_TYPE_LONG is passed for a point that
   *     only supported 16 bit outputs, you could return 
   *     DNPDEFS_CTLSTAT_OUT_OF_RANGE
   */
  DNPDEFS_CTLSTAT TMWDEFS_GLOBAL sdnpdata_anlgOutSelect(
    void *pPoint,
    TMWTYPES_ANALOG_VALUE *pValue);

  /* function: sdnpdata_anlgOutOperate
   * purpose: perform Operate operation on the specified point and return
   *   the result. If Object 43 Analog Output Command Events are desired
   *   you may call sdnpo043_addEvent() from this function.
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   *  pValue - pointer to location to containing value
   *   pValue->type will indicate storage type
   *   TMWTYPES_ANALOG_TYPE_LONG 
   *   TMWTYPES_ANALOG_TYPE_SHORT 
   *   TMWTYPES_ANALOG_TYPE_SFLOAT
   *   TMWTYPES_ANALOG_TYPE_DOUBLE
   * returns:
   *  status of Analog Output operation. Valid values are:
   *    DNPDEFS_CTLSTAT_SUCCESS - the command was performed successfully
   *    DNPDEFS_CTLSTAT_FORMAT_ERROR - The request was not accepted due
   *      to formatting errors in the request. This value may also be use
   *      to indicate that the value in the request exceeds the permitted
   *      level (see Table 4-2 in DNP3 Specification Volume 2, Application
   *      Layer.)
   *    DNPDEFS_CTLSTAT_NOT_SUPPORTED - the request was not accepted because
   *      Control operations are not supported for this point
   *    DNPDEFS_CTLSTAT_ALREADY_ACTIVE - the request was not accepted because
   *      the control queue is full or the point is already active
   *    DNPDEFS_CTLSTAT_HARDWARE_ERROR - the request was not accepted due to
   *      control hardware problems
   *    DNPDEFS_CTLSTAT_LOCAL - the request was not accepted because 
   *      Local/Remote switch is in Local Position
   *    DNPDEFS_CTLSTAT_NOT_AUTHORIZED - the request was not accepted because 
   *      of insufficient authorization.
   *    DNPDEFS_CTLSTAT_AUTO_INHIBIT - the request was not accepted because it
   *      was prevented or inhibited by a local automation process  
   *    DNPDEFS_CTLSTAT_PROC_LIMITED - the request was not accepted because the
   *     device cannot process any more activities than are presently in progress 
   *    DNPDEFS_CTLSTAT_OUT_OF_RANGE - the request was not accepted because the 
   *     value is outside the acceptable range permitted for this point.  
   *    DNPDEFS_CTLSTAT_DOWNSTRM_LOCAL - the request was not accepted because the 
   *     control is being forwarded to a downstream device that is reporting Local.
   *    DNPDEFS_CTLSTAT_ALR_COMPLETE - the request was not accepted because the 
   *     operation is already complete. For example if the request is to close a 
   *     switch and the switch is already closed.
   *    DNPDEFS_CTLSTAT_BLOCKED - the request was not accepted because it is 
   *     specifically blocked at the outstation.
   *    DNPDEFS_CTLSTAT_CANCELLED - the request was not accepted because the 
   *     operation was cancelled.
   *    DNPDEFS_CTLSTAT_BLOCKED_OM - the request was not accepted because another 
   *     master has exclusive rights to operate this point.
   *    DNPDEFS_CTLSTAT_DOWNSTRM_FAIL -  the request was not accepted because the 
   *     control is being forwarded to a downstream device which cannot be reached 
   *     or is otherwise incapable of performing the request.
   *    DNPDEFS_CTLSTAT_UNDEFINED - the request not accepted because of some 
   *     other undefined reason
   *
   *   NOTE- if pValue->type TMWTYPES_ANALOG_TYPE_LONG is passed for a point that
   *     only supported 16 bit outputs, you could return
   *     DNPDEFS_CTLSTAT_OUT_OF_RANGE
   */
  DNPDEFS_CTLSTAT TMWDEFS_GLOBAL sdnpdata_anlgOutOperate(
    void *pPoint,
    TMWTYPES_ANALOG_VALUE *pValue);

  /* function: sdnpdata_anlgOutCancelSelect
   * purpose: cancel outstanding Select on the specified point
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL sdnpdata_anlgOutCancelSelect(
    void *pPoint);
 
 /* function: sdnpdata_anlgOutEventCmdDefVariation   
   * purpose: Determine default variation for this frozen counter
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   *  classMask - mask indicating what event class is being requested
   * returns:
   *  default variation for this point
   *  NOTE: this will only be called if the default variation for 
   *   analog input change events obj43DefaultVariation is configured as zero
   */
  TMWTYPES_UCHAR TMWDEFS_GLOBAL sdnpdata_anlgOutCmdEventDefVariation(
    void *pPoint,
    TMWDEFS_CLASS_MASK classMask);

  /* function: sdnpdata_anlgOutCmdEventMode 
   * purpose: Determine event mode for this point
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   * returns:
   *  event mode for this point, SOE or LAST
   *  NOTE: this will only be called if the event mode for analog output commands
   *    if analogOutCmdEventMode is configured as TMWDEFS_EVENT_MODE_PER_POINT
   */
  TMWDEFS_EVENT_MODE TMWDEFS_GLOBAL sdnpdata_anlgOutCmdEventMode(
    void *pPoint);
 
  /* function: sdnpdata_anlgOutCmdEventClass
   * purpose: Return the class in which analog output command events for
   *  this data point belong.
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   * returns:
   *  Class in which these events will be returned
   */
  TMWDEFS_CLASS_MASK TMWDEFS_GLOBAL sdnpdata_anlgOutCmdEventClass(
    void *pPoint);

  /* function: sdnpdata_anlgOutCmdAssignClass
   * purpose: Assign the class in which analog output command events for 
   *  this data point will belong.
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   *  classMask - new class in which to generate events from this point
   * returns:
   *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_anlgOutCmdAssignClass(
    void *pPoint,
    TMWDEFS_CLASS_MASK classMask);
 
  /* function: sdnpdata_anlgOutCmdChanged
   * purpose: Determine if the command status for the specified point has 
   *  changed and if so return the new value. This function is used to scan 
   *  for events on each data point. This function is called to determine 
   *  whether an Object Group 43 Analog Output Command Event should be queued. 
   *  It will be called if SDNPSESN_CONFIG analogOutCmdScanPeriod is nonzero.
   *  NOTE: this functionality is compiled out by defining 
   *  SDNPDATA_SUPPORT_OBJ43 FALSE (default)
   *  or SDNPDATA_SUPPORT_EVENT_SCAN FALSE
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   *  pValue - pointer to location to store current value
   *    set pValue->type to indicate data type returned.
   *   NOTE:  
   *    If TMWCNFG_SUPPORT_DOUBLE == TRUE returning the value as
   *    TMWTYPES_ANALOG_TYPE_DOUBLE allows for proper events of type
   *    DOUBLE FLOAT SFLOAT, ULONG or USHORT.
   *
   *    If TMWCNFG_SUPPORT_DOUBLE == FALSE and 
   *       TMWCNFG_SUPPORT_SFLOAT == TRUE  
   *    To provide for best precision and OVER_RANGE flag setting:
   *    return the value as TMWTYPES_ANALOG_TYPE_LONG if the value is between
   *    TMWDEFS_LONG_MIN and TMWDEFS_LONG_MAX. 
   *    return the value as TMWTYPES_ANALOG_TYPE_SFLOAT if the value is outside
   *    of that range. While each is 32 bits in size the long allows 
   *    for 31 bits of precision. Returning a float instead allows greater values,
   *    but only 23 bits of precision. 
   *    If both TMWCNFG_SUPPORT_DOUBLE and TMWCNFG_SUPPORT_SFLOAT == FALSE and
   *    the long value is over or under range, set DNPDEFS_DBAS_FLAG_OVER_RANGE 
   *    bit in pFlags and set lval to TMWDEFS_LONG_MIN or TMWDEFS_LONG_MAX.
   *
   *  pStatus - pointer to location to store current command status
   * returns:
   *  TMWDEFS_TRUE if the point has changed, else TMWDEFS_FALSE
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_anlgOutCmdChanged(
    void *pPoint,
    TMWTYPES_ANALOG_VALUE *pValue, 
    TMWTYPES_UCHAR *pStatus);


 /* Double Bit Inputs */

  /* function: sdnpdata_dblInGetDescription
   * purpose: Get description of this point
   *  NOTE: this is used only by sdnpxml/sdnpxml2.c if SDNPDATA_SUPPORT_XMLxx is TRUE
   *  code to generate the configuration file or Device Profile for this device. 
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.  
   * returns:
   *  pointer to string describing point
   *  TMWDEFS_NULL if failure
   */
  TMWTYPES_CHAR * TMWDEFS_GLOBAL sdnpdata_dblInGetDescription(
    void *pPoint);

  /* function: sdnpdata_dblInQuantity
   * purpose: Return the number of binary input data points in the 
   *  specified database.
   * arguments:
   *  pHandle - handle to database returned from sdnpdata_init
   * returns:
   *  The number of binary input data points (see note for
   *  sdnpdata_xxxQuantity at top of this file)
   */
  TMWTYPES_USHORT TMWDEFS_CALLBACK sdnpdata_dblInQuantity(
    void *pHandle);

  /* function: sdnpdata_dblInGetPoint
   * purpose: Return a handle to a specific data point. This handle is
   *  used by the routines below to read and write values and control
   *  information to this point.
   * arguments:
   *  pHandle - handle to database returned from sdnpdata_init
   *  pointNum - point number to return
   * returns:
   *  Pointer to specified data point or TMWDEFS_NULL if this point
   *   is currently disabled.
   */
  void * TMWDEFS_CALLBACK sdnpdata_dblInGetPoint(
    void *pHandle,
    TMWTYPES_USHORT pointNum);

  /* function: sdnpdata_dblInDefVariation  
   * purpose: Determine default static variation for this binary input
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   * returns:
   *  default variation for this point
   *  NOTE: this will only be called if the default variation for double bit 
   *   inputs obj03DefaultVariation is configured as zero
   */
  TMWTYPES_UCHAR TMWDEFS_GLOBAL sdnpdata_dblInDefVariation(
    void *pPoint);

  /* function: sdnpdata_dblInEventMode 
   * purpose: Determine event mode for this point
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   * returns:
   *  event mode for this point, SOE or LAST
   *  NOTE: this will only be called if the event mode double bit inputs
   *    if doubleInputEventMode is configured as TMWDEFS_EVENT_MODE_PER_POINT
   */
  TMWDEFS_EVENT_MODE TMWDEFS_GLOBAL sdnpdata_dblInEventMode(
    void *pPoint);

  /* function: sdnpdata_dblInEventClass
   * purpose: Return the class in which events from this data point
   *  belong.
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   * returns:
   *  Class in which these events will be returned
   */
  TMWDEFS_CLASS_MASK TMWDEFS_GLOBAL sdnpdata_dblInEventClass(
    void *pPoint);

   /* function: sdnpdata_dblInIsClass0
    * purpose: Should this point be reported in response to an object 60 
    *   variation 1 read request. This allows individual points to be excluded
    *   from a class 0 response but still readable by a specific object group
    *   read request.
    * arguments:
    *  pPoint - handle to data point returned from 'getPoint' function.
    * returns:
    *  TMWDEFS_TRUE if point should be reported.
    */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_dblInIsClass0(
    void *pPoint);

  /* function: sdnpdata_dblInEventDefVariation  
   * purpose: Determine default variation for this frozen counter
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   *  classMask - mask indicating what event class is being requested
   * returns:
   *  default variation for this point
   *  NOTE: this will only be called if the default variation for binary
   *   input change events obj04DefaultVariation is configured as zero
   */
  TMWTYPES_UCHAR TMWDEFS_GLOBAL sdnpdata_dblInEventDefVariation(
    void *pPoint,
    TMWDEFS_CLASS_MASK classMask);

  /* function: sdnpdata_dblInAssignClass
   * purpose: Assign the class in which events from this data point
   *  will belong.
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   *  classMask - new class in which to generate events from this point
   * returns:
   *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_dblInAssignClass(
    void *pPoint,
    TMWDEFS_CLASS_MASK classMask);

  /* function: sdnpdata_dblInRead
   * purpose: Return the current value of the specified point.
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   *  pFlags - pointer to location to store current DNP3 flags and value.
   *   pFlags contains a status indication and the current state of the point.
   *   The following values (or OR'd combinations) are valid for this type:
   *      DNPDEFS_DBAS_FLAG_OFF_LINE - the point is off-line, and the returned
   *        state of this point may not be correct
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
   * returns:
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL sdnpdata_dblInRead(
    void *pPoint,
    TMWTYPES_UCHAR *pFlags);

  /* function: sdnpdata_dblInChanged
   * purpose: Determine if the specified point has changed and if so
   *  return the new value. This function is used to scan for events on 
   *  each data point. It will be called if SDNPSESN_CONFIG 
   *  doubleInputScanPeriod is nonzero.
   *  NOTE: this functionality is compiled out by defining 
   *  SDNPDATA_SUPPORT_OBJ4 FALSE
   *  or SDNPDATA_SUPPORT_EVENT_SCAN FALSE
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   *  pFlags - pointer to location to store current DNP3 flags and value
   *   pFlags contains a status indication and the current state of the point.
   *   The following values (or OR'd combinations) are valid for this type:
   *      DNPDEFS_DBAS_FLAG_OFF_LINE - the point is off-line, and the returned
   *        state of this point may not be correct
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
   *      DNPDEFS_DBAS_FLAG_DOUBLE_INTER - 
   *        the current state of the input (Intermediate -transitioning condition)
   *      DNPDEFS_DBAS_FLAG_DOUBLE_OFF   - 
   *        the current state of the input (Off)
   *      DNPDEFS_DBAS_FLAG_DOUBLE_ON    - 
   *        the current state of the input (On)
   *      DNPDEFS_DBAS_FLAG_DOUBLE_INDET -
   *        the current state of the input (Indeterminate -abnormal or custom 
   *        condition)
   * returns:
   *  TMWDEFS_TRUE if the point has changed, else TMWDEFS_FALSE
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_dblInChanged(
    void *pPoint,
    TMWTYPES_UCHAR *pFlags);

  /* Time and date - indexed abs time and long interval */

  /* function: sdnpdata_indexedTimeGetDescription
   * purpose: Get description of this point
   *  NOTE: this is used only by sdnpxml/sdnpxml2.c if SDNPDATA_SUPPORT_XMLxx is TRUE
   *  code to generate the configuration file or Device Profile for this device.
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   * returns:
   *  pointer to string describing point
   *  TMWDEFS_NULL if failure
   */
  TMWTYPES_CHAR * TMWDEFS_GLOBAL sdnpdata_indexedTimeGetDescription(
    void *pPoint);

  /* function: sdnpdata_indexedTimeQuantity
   * purpose: Return the number of indexed abs time data points in the
   *  specified database.
   * arguments:
   *  pHandle - handle to database returned from sdnpdata_init
   * returns:
   *  The number of indexed abs time data points (see note for
   *  sdnpdata_xxxQuantity at top of this file)
   */
  TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpdata_indexedTimeQuantity(
    void *pHandle);

  /* function: sdnpdata_indexedTimeGetPoint
   * purpose: Return a handle to a specific data point. This handle is
   *  used by the routines below to read and write values and control
   *  information to this point.
   * arguments:
   *  pHandle - handle to database returned from sdnpdata_init
   *  pointNum - point number to return
   * returns:
   *  Pointer to specified data point or TMWDEFS_NULL if this point
   *   is currently disabled.
   */
  void * TMWDEFS_GLOBAL sdnpdata_indexedTimeGetPoint(
    void *pHandle,
    TMWTYPES_USHORT pointNum);

  /* function: sdnpdata_indexedTimeIsClass0
  * purpose: Should this point be reported in response to an object 50
  *   variation 1 read request. This allows individual points to be excluded
  *   from a class 0 response but still readable by a specific object group
  *   read request.
  * arguments:
  *  pPoint - handle to data point returned from 'getPoint' function.
  * returns:
  *  TMWDEFS_TRUE if point should be reported.
  */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_indexedTimeIsClass0(
    void *pPoint);

  /* function: sdnpdata_indexedTimeRead
   * purpose: Return the current value of the specified point.
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   *  pTimeStamp - pointer to the time at which the action shall initially occur.
   *  pIntervalCount - pointer to the interval after the initial occurrence.
   *  pIntervalUnit - pointer to the units of the interval between actions.
   * returns:
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL sdnpdata_indexedTimeRead(
    void *pPoint,
    TMWDTIME *pTimeStamp,
    TMWTYPES_ULONG *pIntervalCount,
    TMWTYPES_BYTE  *pIntervalUnit);

  /* function: sdnpdata_indexedTimeWrite
   * purpose: perform write operation on the specified indexed abs time point and
   *   return the result. Write to object group 50 variation 4 was received.
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   *  pTimeStamp - pointer to the time at which the action shall initially occur.
   *  pIntervalCount - pointer to the interval after the initial occurrence.
   *  pIntervalUnit - pointer to the units of the interval between actions.
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_indexedTimeWrite(
    void *pPoint,
    TMWDTIME pTimeStamp,
    TMWTYPES_ULONG pIntervalCount,
    TMWTYPES_BYTE  pIntervalUnit);

  /* Strings */

  /* function: sdnpdata_strGetDescription
   * purpose: Get description of this point
   *  NOTE: this is used only by sdnpxml/sdnpxml2.c if SDNPDATA_SUPPORT_XMLxx is TRUE
   *  code to generate the configuration file or Device Profile for this device. 
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.  
   * returns:
   *  pointer to string describing point
   *  TMWDEFS_NULL if failure
   */
  TMWTYPES_CHAR * TMWDEFS_GLOBAL sdnpdata_strGetDescription(
    void *pPoint);

  /* function: sdnpdata_strQuantity
   * purpose: Return the number of string data points in the 
   *  specified database.
   * arguments:
   *  pHandle - handle to database returned from sdnpdata_init
   * returns:
   *  The number of string data points (see note for
   *  sdnpdata_xxxQuantity at top of this file)
   */
  TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpdata_strQuantity(
    void *pHandle);

  /* function: sdnpdata_strGetPoint
   * purpose: Return a handle to a specific data point. This handle is
   *  used by the routines below to read and write values and control
   *  information to this point.
   * arguments:
   *  pHandle - handle to database returned from sdnpdata_init
   *  pointNum - point number to return
   * returns:
   *  Pointer to specified data point or TMWDEFS_NULL if this point
   *   is currently disabled.
   */
  void * TMWDEFS_GLOBAL sdnpdata_strGetPoint(
    void *pHandle,
    TMWTYPES_USHORT pointNum);

  /* function: sdnpdata_strEventMode 
   * purpose: Determine event mode for this point
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   * returns:
   *  event mode for this point, SOE or LAST
   *  NOTE: this will only be called if the event mode for octet strings
   *   if stringEventMode is configured as TMWDEFS_EVENT_MODE_PER_POINT
   */
  TMWDEFS_EVENT_MODE TMWDEFS_GLOBAL sdnpdata_strEventMode(
    void *pPoint);

  /* function: sdnpdata_strEventClass
   * purpose: Return the class in which events from this data point
   *  belong.
   * arguments:
   *  pHandle - handle to data point returned from 'getPoint' function.
   * returns:
   *  Class in which these events will be returned
   */
  TMWDEFS_CLASS_MASK TMWDEFS_GLOBAL sdnpdata_strEventClass(
    void *pPoint);

   /* function: sdnpdata_strIsClass0
    * purpose: Should this point be reported in response to an object 60 
    *   variation 1 read request. This allows individual points to be excluded
    *   from a class 0 response but still readable by a specific object group
    *   read request.
    * arguments:
    *  pPoint - handle to data point returned from 'getPoint' function.
    * returns:
    *  TMWDEFS_TRUE if point should be reported.
    */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_strIsClass0(
    void *pPoint);

  /* function: sdnpdata_strAssignClass
   * purpose: Assign the class in which events from this data point
   *  will belong.
   * arguments:
   *  pHandle - handle to data point returned from 'getPoint' function.
   *  classMask - new class in which to generate events from this point
   * returns:
   *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_strAssignClass(
    void *pPoint,
    TMWDEFS_CLASS_MASK classMask);

  /* function: sdnpdata_strRead
   * purpose: Return the current value of the specified point.
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function
   *  maxLength - maximum length of buffer to read.
   *  pBuf - pointer to buffer into which to store string.
   *  pLength - pointer to location into which to store string length.
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL sdnpdata_strRead(
    void *pPoint,
    TMWTYPES_UCHAR maxLength,
    TMWTYPES_UCHAR *pBuf,
    TMWTYPES_UCHAR *pLength);

  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_strWrite(
    void *pPoint,
    TMWTYPES_UCHAR *pBuf,
    TMWTYPES_UCHAR bufLength);

  /* function: sdnpdata_strChanged
   * purpose: Determine if the specified point has changed and if so
   *  return the new value. This function is used to scan for events on
   *  each data point. It will be called if SDNPSESN_CONFIG 
   *  stringScanPeriod is nonzero.
   *  NOTE: this functionality is compiled out by defining 
   *  SDNPDATA_SUPPORT_OBJ111 FALSE
   *  or SDNPDATA_SUPPORT_EVENT_SCAN FALSE
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   *  pFlags - pointer to location to store current DNP3 flags
   *   See description above.
   * returns:
   *  TMWDEFS_TRUE if the point has changed, else TMWDEFS_FALSE
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_strChanged(
    void *pPoint,
    TMWTYPES_UCHAR maxLength,
    TMWTYPES_UCHAR *pBuf,
    TMWTYPES_UCHAR *pLength);

  /* Virtual Terminal */

  /* function: sdnpdata_vtermGetDescription
   * purpose: Get description of this point
   *  NOTE: this is used only by sdnpxml/sdnpxml2.c if SDNPDATA_SUPPORT_XMLxx is TRUE
   *  code to generate the configuration file or Device Profile for this device. 
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.  
   * returns:
   *  pointer to string describing point
   *  TMWDEFS_NULL if failure
   */
  TMWTYPES_CHAR * TMWDEFS_GLOBAL sdnpdata_vtermGetDescription(
    void *pPoint);

  /* function: sdnpdata_vtermQuantity
   * purpose: Return the number of virtual terminals in the 
   *  specified database.
   * arguments:
   *  pHandle - handle to database returned from sdnpdata_init
   * returns:
   *  The number of virtual terminals (see note for
   *  sdnpdata_xxxQuantity at top of this file)
   */
  TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpdata_vtermQuantity(
    void *pHandle);

  /* function: sdnpdata_vtermGetPoint
   * purpose: Return a handle to a specific data point. This handle is
   *  used by the routines below to read and write values and control
   *  information to this point.
   * arguments:
   *  pHandle - handle to database returned from sdnpdata_init
   *  pointNum - point number to return
   * returns:
   *  Pointer to specified data point or TMWDEFS_NULL if this point
   *   is currently disabled.
   */
  void * TMWDEFS_GLOBAL sdnpdata_vtermGetPoint(
    void *pHandle,
    TMWTYPES_USHORT pointNum);

  /* function: sdnpdata_vtermEventMode 
   * purpose: Determine event mode for this point
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   * returns:
   *  event mode for this point, SOE or LAST
   *  NOTE: this will only be called if the event mode for octet strings
   *   if vtermEventMode is configured as TMWDEFS_EVENT_MODE_PER_POINT
   */
  TMWDEFS_EVENT_MODE TMWDEFS_GLOBAL sdnpdata_vtermEventMode(
    void *pPoint);

  /* function: sdnpdata_vtermEventClass
   * purpose: Return the class in which events from this data point
   *  belong.
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   * returns:
   *  Class in which these events will be returned
   */
  TMWDEFS_CLASS_MASK TMWDEFS_GLOBAL sdnpdata_vtermEventClass(
    void *pPoint);

  /* function: sdnpdata_vtermAssignClass
   * purpose: Assign the class in which events from this data point
   *  will belong.
   * arguments:
   *  pHandle - handle to data point returned from 'getPoint' function.
   *  classMask - new class in which to generate events from this point
   * returns:
   *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_vtermAssignClass(
    void *pPoint,
    TMWDEFS_CLASS_MASK classMask);

  /* function: sdnpdata_vtermRead
   * purpose: Return the current value of the specified point.
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function
   *  maxLength - maximum length of buffer to read.
   *  pBuf - pointer to buffer into which to store string.
   *  pLength - pointer to location into which to store string length.
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL sdnpdata_vtermRead(
    void *pPoint,
    TMWTYPES_UCHAR maxLength,
    TMWTYPES_UCHAR *pBuf,
    TMWTYPES_UCHAR *pLength);

  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_vtermWrite(
    void *pPoint,
    TMWTYPES_UCHAR *pBuf,
    TMWTYPES_UCHAR bufLength);

  /* function: sdnpdata_vtermChanged
   * purpose: Determine if the specified point has changed and if so
   *  return the new value. This function is used to scan for events on
   *  each data point. It will be called if SDNPSESN_CONFIG 
   *  virtualTerminalScanPeriod is nonzero.
   *  NOTE: this functionality is compiled out by defining 
   *  SDNPDATA_SUPPORT_OBJ113 FALSE
   *  or SDNPDATA_SUPPORT_EVENT_SCAN FALSE
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   *  maxLength - maximum length of buffer to read.
   *  pBuf - pointer to buffer into which to store string.
   *  pLength - pointer to location into which to store string length.
   * returns:
   *  TMWDEFS_TRUE if the point has changed, else TMWDEFS_FALSE
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_vtermChanged(
    void *pPoint,
    TMWTYPES_UCHAR maxLength,
    TMWTYPES_UCHAR *pBuf,
    TMWTYPES_UCHAR *pLength);

  /* Extended Strings */

  /* function: sdnpdata_extStrGetDescription
   * purpose: Get description of this point
   *  NOTE: this is used only by sdnpxml/sdnpxml2.c if SDNPDATA_SUPPORT_XMLxx is TRUE
   *  code to generate the configuration file or Device Profile for this device. 
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.  
   * returns:
   *  pointer to string describing point
   *  TMWDEFS_NULL if failure
   */
  TMWTYPES_CHAR * TMWDEFS_GLOBAL sdnpdata_extStrGetDescription(
    void *pPoint);

  /* function: sdnpdata_extStrQuantity
   * purpose: Return the number of string data points in the 
   *  specified database.
   * arguments:
   *  pHandle - handle to database returned from sdnpdata_init
   * returns:
   *  The number of string data points (see note for
   *  sdnpdata_xxxQuantity at top of this file)
   */
  TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpdata_extStrQuantity(
    void *pHandle);

  /* function: sdnpdata_extStrGetPoint
   * purpose: Return a handle to a specific data point. This handle is
   *  used by the routines below to read and write values and control
   *  information to this point.
   * arguments:
   *  pHandle - handle to database returned from sdnpdata_init
   *  pointNum - point number to return
   * returns:
   *  Pointer to specified data point or TMWDEFS_NULL if this point
   *   is currently disabled.
   */
  void * TMWDEFS_GLOBAL sdnpdata_extStrGetPoint(
    void *pHandle,
    TMWTYPES_USHORT pointNum);

  /* function: sdnpdata_extStrDefVariation  
   * purpose: Determine default static variation for this extended string
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   * returns:
   *  default variation for this point
   *  NOTE: this will only be called if the default variation for extended
   *   string obj114DefaultVariation is configured as zero
   */
  TMWTYPES_UCHAR TMWDEFS_GLOBAL sdnpdata_extStrDefVariation(
    void *pPoint);

  /* function: sdnpdata_extStrEventDefVariation   
   * purpose: Determine default variation for this extended string
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   *  classMask - mask indicating what event class is being requested
   * returns:
   *  default variation for this point
   *  NOTE: this will only be called if the default variation for 
   *   extended string change events obj115DefaultVariation is configured as zero
   */
  TMWTYPES_UCHAR TMWDEFS_GLOBAL sdnpdata_extStrEventDefVariation(
    void *pPoint,
    TMWDEFS_CLASS_MASK classMask);

  /* function: sdnpdata_extStrEventMode 
   * purpose: Determine event mode for this point
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   * returns:
   *  event mode for this point, SOE or LAST
   *  NOTE: this will only be called if the event mode for octet strings
   *   if stringEventMode is configured as TMWDEFS_EVENT_MODE_PER_POINT
   */
  TMWDEFS_EVENT_MODE TMWDEFS_GLOBAL sdnpdata_extStrEventMode(
    void *pPoint);

  /* function: sdnpdata_extStrEventClass
   * purpose: Return the class in which events from this data point
   *  belong.
   * arguments:
   *  pHandle - handle to data point returned from 'getPoint' function.
   * returns:
   *  Class in which these events will be returned
   */
  TMWDEFS_CLASS_MASK TMWDEFS_GLOBAL sdnpdata_extStrEventClass(
    void *pPoint);

   /* function: sdnpdata_extStrIsClass0
    * purpose: Should this point be reported in response to an object 60 
    *   variation 1 read request. This allows individual points to be excluded
    *   from a class 0 response but still readable by a specific object group
    *   read request.
    * arguments:
    *  pPoint - handle to data point returned from 'getPoint' function.
    * returns:
    *  TMWDEFS_TRUE if point should be reported.
    */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_extStrIsClass0(
    void *pPoint);

  /* function: sdnpdata_extStrAssignClass
   * purpose: Assign the class in which events from this data point
   *  will belong.
   * arguments:
   *  pHandle - handle to data point returned from 'getPoint' function.
   *  classMask - new class in which to generate events from this point
   * returns:
   *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_extStrAssignClass(
    void *pPoint,
    TMWDEFS_CLASS_MASK classMask);

 #if SDNPDATA_SUPPORT_OBJ114_MIN_STACK
  /* function: sdnpdata_extStrGetPtr
   * purpose: Returns a pointer to the extended string value of the specified point.
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function
   *  pLength - pointer to location into which to store extended string's length.
   *  pFlags - pointer to location to store current DNP3 flags
   *   See description above.
   * returns:
   *  TMWTYPES_UCHAR * - pointer to the location of the extended string's value.
   */
  TMWTYPES_UCHAR * TMWDEFS_GLOBAL sdnpdata_extStrGetPtr(
    void *pPoint,
    TMWTYPES_USHORT *pLength,
    TMWTYPES_UCHAR *pFlags);

  /* function: sdnpdata_extStrRelease
   * purpose: Release the pointer that was returned by in sdnpdata_extStrGetPtr 
   *  The database is free to update the extended string value. The
   *  SCL will not attempt to reference that pointer anymore.
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function
   * returns:
   *  TMWDEFS_TRUE - if the database is correctly informed of the pointer's release.
   */
  void TMWDEFS_GLOBAL sdnpdata_extStrRelease(
    void *pPoint);
#else
    /* function: sdnpdata_extStrRead
   * purpose: Return the current value of the specified point.
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function
   *  maxLength - maximum length of buffer to read.
   *  pBuf - pointer to buffer into which to store string.
   *  pLength - pointer to location into which to store string length.
   *  pFlags - pointer to location to store current DNP3 flags
   *   See description above.
   * returns:
   *  void
   */
    void TMWDEFS_GLOBAL sdnpdata_extStrRead(
    void *pPoint,
    TMWTYPES_USHORT maxLength,
    TMWTYPES_UCHAR *pBuf,
    TMWTYPES_USHORT *pLength,
    TMWTYPES_UCHAR *pFlags);
#endif

  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_extStrWrite(
    void *pPoint,
    TMWTYPES_UCHAR *pBuf,
    TMWTYPES_USHORT bufLength);

  /* function: sdnpdata_extStrChanged
   * purpose: Determine if the specified point has changed and if so
   *  return the new value. This function is used to scan for events on
   *  each data point. It will be called if SDNPSESN_CONFIG 
   *  stringScanPeriod is nonzero.
   *  NOTE: this functionality is compiled out by defining 
   *  SDNPDATA_SUPPORT_OBJ115 FALSE
   *  or SDNPDATA_SUPPORT_EVENT_SCAN FALSE
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   *  pFlags - pointer to location to store current DNP3 flags
   *   See description above.
   * returns:
   *  TMWDEFS_TRUE if the point has changed, else TMWDEFS_FALSE
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_extStrChanged(
    void *pPoint,
    TMWTYPES_USHORT maxLength,
    TMWTYPES_UCHAR *pBuf,
    TMWTYPES_USHORT *pLength,
    TMWTYPES_UCHAR *pFlags);

  /* function: sdnpdata_fileEventClass
   * purpose: Return the class in which file events belong.
   * arguments:
   *  pHandle - handle to database returned from sdnpdata_init
   * returns:
   *  Class in which these events will be returned
   */
  TMWDEFS_CLASS_MASK TMWDEFS_GLOBAL sdnpdata_fileEventClass(
    void *pHandle);

  /* function: sdnpdata_fileAssignClass 
   * purpose: Assign the class in which file transfer response events
   *  will belong.
   * arguments:
   *  pHandle - handle to database returned from sdnpdata_init
   *  classMask - new class in which to generate events from this
   * returns:
   *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_fileAssignClass(
    void *pHandle,
    TMWDEFS_CLASS_MASK classMask);

  /* function: sdnpdata_getFileInfo
   * purpose: return information about file or directory
   *  This is used to build a response to a FC 28 Get File Info request.
   *   NOTE: you may want to limit file access to only certain files and/or directories
   *   based on the filename specified by returning DNPDEFS_FILE_CMD_STAT_DENIED or
   *   other error.
   * arguments:
   *  pSession - pointer to session request was received on
   *  pFilename - null terminated name of file to get info from
   *  pType - return type of file
   *    DNPDEFS_FILE_TYPE_SIMPLE  or
   *    DNPDEFS_FILE_TYPE_DIRECTORY
   *  pSize - return size of simple file in bytes 
   *    If this was directory, size is the number of files contained in 
   *    the directory excluding links to itself and parent, according to
   *    DNP3 Specification, Application Layer Draft H 6.7.4
   *  pTimeOfCreation - return file or directory time of creation
   *  pPermissions - return file or directory permissions
   *   see DNPDEFS_FILE_PERMISSIONS in dnpdefs.h
   * returns:
   *  DNPDEFS_FILE_CMD_STAT_SUCCESS if successful
   *  DNPDEFS_FILE_CMD_STAT_ASYNC if the SCL should send a NULL response
   *   and later send the response as an event. The target database can call
   *   sdnpo070_fileCmdComplete(); or the SCL will call sdnpdata_getFileInfo periodically
   *  DNPDEFS_FILE_CMD_STAT_NOT_FOUND If file does not exist
   *  Other Appropriate DNPDEFS_FILE_CMD_STAT values
   *   DNPDEFS_FILE_CMD_STAT_LOST_COMM   
   *   DNPDEFS_FILE_CMD_STAT_MISC        
   */
  DNPDEFS_FILE_CMD_STAT TMWDEFS_GLOBAL sdnpdata_getFileInfo(
    TMWSESN *pSession,
    TMWTYPES_CHAR *pFilename, 
    DNPDEFS_FILE_TYPE *pType,
    TMWTYPES_ULONG *pSize, 
    TMWDTIME *pTimeOfCreation, 
    DNPDEFS_FILE_PERMISSIONS *pPermissions);

  /* function: sdnpdata_readFileInfo
   * purpose: return information about file or subdirectory
   *  This is used to build a response to a FC 1 read request on a directory.
   *  This function will be called multiple times. It should set pLast to
   *  TRUE when the last entry in a directory is being read.
   * arguments:
   *  pSession - pointer to session request was received on
   *  handle - DNP file handle returned from previous call to sdnpdata_openFile
   *  maxNameSize - max length of pFileName to be returned 
   *    If name won't fit, return DNPDEFS_FILE_TFER_STAT_MISC and SCL
   *    will read this file info again on next request.
   *  pFilename - return null terminated name of file or subdirectory entry
   *  pLast - return TMWDEFS_TRUE if this is the last block in this file
   *  pType - return type of file
   *    DNPDEFS_FILE_TYPE_SIMPLE  or
   *    DNPDEFS_FILE_TYPE_DIRECTORY
   *  pSize - return size of simple file in bytes  
   *    If this was directory, size is the number of files contained in 
   *    the directory excluding links to itself and parent, according to
   *    DNP3 Specification, Application Layer Draft H 6.7.4
   *  pTimeOfCreation - return file time of creation
   *  pPermissions - return file permissions
   * returns:
   *  DNPDEFS_FILE_TFER_STAT_SUCCESS if successful
   *  DNPDEFS_FILE_TFER_STAT_ASYNC if the SCL should send a NULL response
   *   and later send the response as an event. The target database can call
   *   sdnpo070_fileCmdComplete(); or the SCL will call sdnpdata_readFileInfo 
   *   periodically. When the SCL is reading an entire directory this will be called
   *   repeatedly. The response will not be sent until either all entries or 
   *   enough to fill a response are read.
   *  Appropriate DNPDEFS_FILE_TFER_STAT values on error.
   *   DNPDEFS_FILE_TFER_STAT_INV_HANDLE  
   *   DNPDEFS_FILE_TFER_STAT_LOST_COMM 
   *   DNPDEFS_FILE_TFER_STAT_MISC        
   */
  DNPDEFS_FILE_TFER_STAT TMWDEFS_GLOBAL sdnpdata_readFileInfo(
    TMWSESN *pSession,
    TMWTYPES_ULONG handle, 
    TMWTYPES_USHORT maxNameSize,
    TMWTYPES_CHAR *pName,
    TMWTYPES_BOOL *pLast,
    DNPDEFS_FILE_TYPE *pType,
    TMWTYPES_ULONG *pSize, 
    TMWDTIME *pTimeOfCreation, 
    DNPDEFS_FILE_PERMISSIONS *pPermissions);

  /* function: sdnpdata_getAuthentication
   * purpose: return authentication key to be used for next file
   *  operation. If authentication is not supported, or  this routine
   *  should set pAuthKey to 0 and return success.
   * arguments:
   *  pSession - pointer to session request was received on
   *  pUsername - username of user requesting authentication
   *  pPassword - password of user requesting authentication
   *  pAuthKey - return authentication key to be used by master in
   *   open or delete request. If unacceptable make pAuthKey point
   *   to authentication key value of zero.
   * returns:
   *  TMWDEFS_TRUE if authentication key has be returned, 
   *  TMWDEFS_FALSE otherwise
   *  NOTE: The DNP specs do not allow a file authentication response to be 
   *   delayed and sent later as an event.
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_getAuthentication(
    TMWSESN *pSession,
    TMWTYPES_CHAR *pUsername, 
    TMWTYPES_CHAR *pPassword, 
    TMWTYPES_ULONG *pAuthKey);

  /* function: sdnpdata_deleteFile
   * purpose: delete the requested file
   *   NOTE: you may want to limit delete access to only certain files and/or directories
   *   based on the filename specified by returning DNPDEFS_FILE_CMD_STAT_DENIED or
   *   other error.
   * arguments:
   *  pSession - pointer to session request was received on
   *  pFilename - null terminated name of file to delete
   *  authKey - authentication key for this request
   * returns:
   *  DNPDEFS_FILE_CMD_STAT_SUCCESS if successful
   *   return DNPDEFS_FILE_CMD_STAT_LOCKED if the specified file is open, 
   *    and do not delete the file.
   *  DNPDEFS_FILE_CMD_STAT_ASYNC if the SCL should send a NULL response
   *   and later send the response as an event. The target database can call
   *   sdnpo070_fileCmdComplete(); or the SCL will call sdnpdata_deleteFile periodically
   *  Other appropriate DNPDEFS_FILE_CMD_STAT values.
   *   DNPDEFS_FILE_CMD_STAT_DENIED    
   *   DNPDEFS_FILE_CMD_STAT_NOT_FOUND   
   *   DNPDEFS_FILE_CMD_STAT_LOST_COMM   
   *   DNPDEFS_FILE_CMD_STAT_MISC        
   */
  DNPDEFS_FILE_CMD_STAT TMWDEFS_GLOBAL sdnpdata_deleteFile(
    TMWSESN *pSession,
    TMWTYPES_CHAR *pFilename, 
    TMWTYPES_ULONG authKey);

  /* function: sdnpdata_openFile
   * purpose: open specified file
   *   NOTE: you may want to limit access to only certain files and/or directories
   *   based on the filename specified by returning DNPDEFS_FILE_CMD_STAT_DENIED or
   *   other error.
   * arguments:
   *  pSession - pointer to session request was received on
   *  pFilename - null terminated name of file to open. This is the entire 
   *   name string as sent from the master.
   *  authKey - authentication key for this request
   *  mode - access mode for file  
   *   DNPDEFS_FILE_MODE_READ     
   *   DNPDEFS_FILE_MODE_WRITE  
   *   DNPDEFS_FILE_MODE_APPEND  
   *  pMaxBlockSize - pointer to proposed maximum block size for this file
   *   this function can modify this to return a reduced maxBlockSize
   *  pPermissions - permissions to give new file if created
   *    NOTE: this data is passed to the database from the SCL. It is NOT
   *          retrieved from the database, it is a pointer to a USHORT
   *          for backward compatibility reasons.
   *  pTimeOfCreation - return time of creation for file
   *  pHandle - return DNP file handle used in DNP g70/v4 and g70/v5 Messages, 
   *   non-zero value if successful. 
   *  pSize - return file size. 
   *   If file was opened for reading, size should indicate the total number 
   *    of octets in the file that was opened. If this is a directory, size
   *    would be the number of octets that would be transferred by the read
   *    requests. This is (the number of files contained in the directory * 20)  
   *    + (sum of the lengths of the names of all of the files in the directory).
   *   If file was opened for writing size SCL will set size to zero.
   *  pType - return type of file
   *   DNPDEFS_FILE_TYPE_DIRECTORY  
   *   DNPDEFS_FILE_TYPE_SIMPLE    
   * returns:
   *  DNPDEFS_FILE_CMD_STAT_SUCCESS if successful
   *  DNPDEFS_FILE_CMD_STAT_ASYNC if the SCL should send a NULL response
   *   and later send the response as an event. The target database can call
   *   sdnpo070_fileCmdComplete(); or the SCL will call sdnpdata_openFile periodically
   *  Other appropriate DNPDEFS_FILE_CMD_STAT values on error or if access is not allowed.
   *   DNPDEFS_FILE_CMD_STAT_DENIED      
   *   DNPDEFS_FILE_CMD_STAT_INV_MODE    
   *   DNPDEFS_FILE_CMD_STAT_NOT_FOUND    
   *   DNPDEFS_FILE_CMD_STAT_TOO_MANY   
   *   DNPDEFS_FILE_CMD_STAT_LOST_COMM   
   *   DNPDEFS_FILE_CMD_STAT_MISC        
   */
  DNPDEFS_FILE_CMD_STAT TMWDEFS_GLOBAL sdnpdata_openFile(
    TMWSESN *pSession,
    TMWTYPES_CHAR *pFilename, 
    TMWTYPES_ULONG authKey,
    DNPDEFS_FILE_MODE mode, 
    TMWTYPES_USHORT *pMaxBlockSize, 
    DNPDEFS_FILE_PERMISSIONS *pPermissions, 
    TMWDTIME *pTimeOfCreation, 
    TMWTYPES_ULONG *pHandle, 
    TMWTYPES_ULONG *pSize,
    DNPDEFS_FILE_TYPE *pType);

  /* function: sdnpdata_closeFile
   * purpose: close existing file
   * arguments:
   *  pSession - pointer to session request was received on
   *  handle - DNP file handle returned from previous call to sdnpdata_openFile
   * returns:
   *  DNPDEFS_FILE_CMD_STAT_SUCCESS if successful
   *  DNPDEFS_FILE_CMD_STAT_ASYNC if the SCL should send a NULL response
   *   and later send the response as an event. The target database can call
   *   sdnpo070_fileCmdComplete(); or the SCL will call sdnpdata_closeFile periodically
   *  Other appropriate DNPDEFS_FILE_CMD_STAT values on error.
   *   DNPDEFS_FILE_CMD_STAT_INV_HANDLE  
   *   DNPDEFS_FILE_CMD_STAT_LOST_COMM   
   *   DNPDEFS_FILE_CMD_STAT_MISC        
   */
  DNPDEFS_FILE_CMD_STAT TMWDEFS_GLOBAL sdnpdata_closeFile(
    TMWSESN *pSession,
    TMWTYPES_ULONG handle);

  /* function: sdnpdata_readFile
   * purpose: read block of data from file.
   * arguments:
   *  pSession - pointer to session request was received on
   *  handle - DNP file handle returned from previous call to sdnpdata_openFile
   *  pLast - return TMWDEFS_TRUE if this is the last block in this file
   *  pBytesRead - return the actual number of bytes read, up to maxBlockSize
   *   specified in sdnpdata_openFile()
   *  pBuf - buffer into which to store data
   * returns:
   *  DNPDEFS_FILE_TFER_STAT_SUCCESS if successful
   *  DNPDEFS_FILE_TFER_STAT_ASYNC if the SCL should send a NULL response
   *   and later send the response as an event. The database code can call
   *   sdnpo070_fileCmdComplete(); or the SCL will call sdnpdata_readFile again
   *  Appropriate DNPDEFS_FILE_TFER_STAT values on error.
   *   DNPDEFS_FILE_TFER_STAT_INV_HANDLE
   *   DNPDEFS_FILE_TFER_STAT_LOST_COMM  
   *   DNPDEFS_FILE_TFER_STAT_NOT_OPEN  
   *   DNPDEFS_FILE_TFER_STAT_HANDLE_EXP 
   *   DNPDEFS_FILE_TFER_STAT_BAD_FILE   
   *   DNPDEFS_FILE_TFER_STAT_MISC    
   */
  DNPDEFS_FILE_TFER_STAT TMWDEFS_GLOBAL sdnpdata_readFile(
    TMWSESN *pSession,
    TMWTYPES_ULONG handle, 
    TMWTYPES_BOOL *pLast,
    TMWTYPES_USHORT *pBytesRead,
    TMWTYPES_UCHAR *pBuf);

  /* function: sdnpdata_confirmFileRead
   * purpose: The last file or directory read has been confirmed by the master. 
   *  The SCL will handle the master rereading the last block that was sent. 
   *  You could use this call to delete data records from a log file once the master 
   *  has the data. This will be called once for each sdnpdata_readFile() call that 
   *  returns SUCCESS, when the appl confirm for that data is received from the 
   *  master. 
   *  For directory reads  sdnpdata_readFileInfo() will be called multiple times for
   *  each response to the master depending on block size and amount of data in a  
   *  file info. The call to sdnpdata_confirmFileRead will be called once for each 
   *  read directory response. This call will acknowledge all sdnpdata_readFileInfo() 
   *  calls up to that time.
   * arguments:
   *  pSession - pointer to session request was received on 
   *  handle - DNP file handle returned from previous call to sdnpdata_openFile
   * returns:
   *  void;
   */ 
  void TMWDEFS_GLOBAL sdnpdata_confirmFileRead(
    TMWSESN *pSession,
    TMWTYPES_ULONG handle);

  /* function: sdnpdata_writeFile
   * purpose: write block of data to file
   * arguments:
   *  pSession - pointer to session request was received on
   *  handle - DNP file handle returned from previous call to sdnpdata_openFile
   *  last - TMWDEFS_TRUE if this is the last block in this file
   *  numBytes - number of bytes to write. This will not exceed maxBlockSize
   *   specified in sdnpdata_openFile()
   *  pBuf - pointer to data buffer to write
   * returns:
   *  DNPDEFS_FILE_TFER_STAT_SUCCESS if successful
   *  DNPDEFS_FILE_TFER_STAT_ASYNC if the SCL should send a NULL response
   *   and later send the response as an event. The target database can call
   *   sdnpo070_fileCmdComplete(); or the SCL will call sdnpdata_writeFile 
   *   periodically
   *  Appropriate DNPDEFS_FILE_TFER_STAT values on error.
   *   DNPDEFS_FILE_TFER_STAT_INV_HANDLE
   *   DNPDEFS_FILE_TFER_STAT_LOST_COMM  
   *   DNPDEFS_FILE_TFER_STAT_NOT_OPEN  
   *   DNPDEFS_FILE_TFER_STAT_HANDLE_EXP 
   *   DNPDEFS_FILE_TFER_STAT_OVERRUN   
   *   DNPDEFS_FILE_TFER_STAT_BAD_FILE   
   *   DNPDEFS_FILE_TFER_STAT_MISC    
   */
  DNPDEFS_FILE_TFER_STAT TMWDEFS_GLOBAL sdnpdata_writeFile(
    TMWSESN *pSession, 
    TMWTYPES_ULONG handle, 
    TMWTYPES_BOOL last,
    TMWTYPES_USHORT numBytes, 
    TMWTYPES_UCHAR *pBuf);

#if SDNPCNFG_USER_MANAGED_EVENTS
  /* The following code is necessary only if User Managed Event
   * queues are implemented, instead of letting the SCL manage events.
   * This might be useful if events needed to be saved in nonVolatile
   * memory, or particular queuing algorithms are desired.
   * #define SDNPCNFG_USER_MANAGED_EVENTS TMWDEFS_TRUE when compiling
   * and set userManagedEvents = TMWDEFS_TRUE in SDNPSESN_CONFIG when the
   * session is opened.
   */
  /* Structure used to retrieve user managed events from database */
  typedef struct  {
    TMWTYPES_USHORT          point;
    TMWDEFS_CLASS_MASK       classMask;
    TMWTYPES_UCHAR           group;
    TMWTYPES_UCHAR           defaultVariation;
    TMWTYPES_UCHAR           flags;
    TMWDTIME                 timeStamp;
    union {
      /* This field is used when getting a counter or frozen counter event
      * from database
      */
      TMWTYPES_ULONG           ulValue;

      /* This field is used when getting an analog input event (objectGroup 32), 
       * an analog output event (objectGroup 42) 
       * or an analog command event (objectGroup 43)
       * from the database 
       */
      TMWTYPES_ANALOG_VALUE    analogValue;

      /* This field is used when getting a string or vterm event from database */
      struct {
        TMWTYPES_UCHAR         length;
        TMWTYPES_UCHAR         buf[DNPDEFS_MAX_STRING_LENGTH+1];
      } stringValue;

#if SDNPDATA_SUPPORT_OBJ115
      /* This field is used when getting a an extended string event from database */
      struct {
        TMWTYPES_USHORT        length;
        TMWTYPES_UCHAR         buf[DNPCNFG_MAX_EXT_STRING_LENGTH+1];
      } extendedStringValue;
#endif

      /* This field is used when getting a Data Set event from database */
      struct {
        TMWTYPES_UCHAR         numberElems;
        DNPDATA_DATASET_VALUE *pDataSet;
      } datasetValue;

      /* This field is used when getting an Authentication Error Event
       * from the database. This event would be on an association other
       * than the association where the error occurred.
       */
      struct {
        TMWTYPES_USHORT        assocId;
        TMWTYPES_ULONG         sequenceNumber;
        TMWTYPES_UCHAR         errorCode;
        TMWTYPES_USHORT        errorTextLength;
        TMWTYPES_UCHAR         errorText[DNPAUTH_MAX_ERROR_TEXT_LENGTH + 1];
      } authError;

      /* This field is used when getting an Authentication Security Statistic 
       * from the database 
       */
      struct {
        TMWTYPES_USHORT        assocId;
        TMWTYPES_ULONG         ulValue;
      } authSecStat;

    } value;
  } SDNPDATA_GET_EVENT;

  /* function: sdnpdata_umEventAdd
   * purpose: add event to user managed event queue in database
   * arguments:
   *  pHandle - handle to database returned from sdnpdata_init
   *  group - object group indicating event type being added
   *   DNPDEFS_OBJ_2_BIN_CHNG_EVENTS  or
   *   DNPDEFS_OBJ_4_DBL_CHNG_EVENTS  or
   *   DNPDEFS_OBJ_11_BIN_OUT_EVENTS  or
   *   DNPDEFS_OBJ_13_BIN_CMD_EVENTS  or 
   *   DNPDEFS_OBJ_22_CNTR_EVENTS     or
   *   DNPDEFS_OBJ_23_FCTR_EVENTS     or
   *   DNPDEFS_OBJ_32_ANA_CHNG_EVENTS or  
   *   DNPDEFS_OBJ_42_ANA_OUT_EVENTS  or
   *   DNPDEFS_OBJ_43_ANA_CMD_EVENTS  or
   *   DNPDEFS_OBJ_111_STRING_EVENTS  or 
   *   DNPDEFS_OBJ_113_VTERM_EVENTS  
   *  point - point number of point that changed
   *  classMask - class that this change event should be reported in. 
   *   TMWDEFS_CLASS_MASK_ONE   or
   *   TMWDEFS_CLASS_MASK_TWO   or
   *   TMWDEFS_CLASS_MASK_THREE
   *  defaultVariation - what variation to use for this event if a read of 
   *   variation 0 is received.
   *   This is used when per point variation is compiled in. 
   *   (SDNPDATA_SUPPORT_EVENT_MODE_POINT)
   *  flags - flags value for this point
   *  pValue - pointer to union containing value for this point
   *   group 2, 4 will not have a value other than flags
   *   group 22, 23 will have value contained in ulValue
   *   group 30 will have value pointed to by analogPtr
   *   group 111 and 113 will have length and string pointed to by stringPtr
   *  pTimeStamp - pointer to time stamp indicating when this point changed
   * returns:
   *  TMWDEFS_TRUE if event was successfully added to queue, 
   *  TMWDEFS_FALSE if this event could not be added, OR another event had to be
   *   deleted in order to add this event. The SCL will set the OVERFLOW IIN bit 
   *   indicating 1 or more events were lost.
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_umEventAdd(
    void *pHandle,
    TMWTYPES_UCHAR group,
    TMWTYPES_USHORT point,
    TMWDEFS_CLASS_MASK classMask,
    TMWTYPES_UCHAR defaultVariation,
    TMWTYPES_UCHAR flags,
    SDNPDATA_ADD_EVENT_VALUE *pValue,
    TMWDTIME *pTimeStamp);
  
  /* function: sdnpdata_umEventNotSentCount
   * purpose: retrieve count of events of this type that have not
   *  been marked as sent, in user managed event queue in database
   * arguments:
   *  pHandle - handle to database returned from sdnpdata_init
   *  group - object group indicating event type
   *   DNPDEFS_OBJ_2_BIN_CHNG_EVENTS  or
   *   DNPDEFS_OBJ_4_DBL_CHNG_EVENTS  or
   *   DNPDEFS_OBJ_11_BIN_OUT_EVENTS  or
   *   DNPDEFS_OBJ_13_BIN_CMD_EVENTS  or 
   *   DNPDEFS_OBJ_22_CNTR_EVENTS     or
   *   DNPDEFS_OBJ_23_FCTR_EVENTS     or
   *   DNPDEFS_OBJ_32_ANA_CHNG_EVENTS or  
   *   DNPDEFS_OBJ_42_ANA_OUT_EVENTS  or
   *   DNPDEFS_OBJ_43_ANA_CMD_EVENTS  or
   *   DNPDEFS_OBJ_111_STRING_EVENTS  or 
   *   DNPDEFS_OBJ_113_VTERM_EVENTS  
   *  classMask - which class to check for events.
   *   TMWDEFS_CLASS_MASK_ONE    or
   *   TMWDEFS_CLASS_MASK_TWO    or
   *   TMWDEFS_CLASS_MASK_THREE  or
   *   TMWDEFS_CLASS_MASK_ALL 
   * returns:
   *  number of events for this group and class in queue THAT HAVE
   *   NOT BEEN MARKED AS SENT
   */
  TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpdata_umEventNotSentCount(
    void *pHandle,
    TMWTYPES_UCHAR group,
    TMWDEFS_CLASS_MASK classMask);

  /* function: sdnpdata_umEventGet
   * purpose: retrieve event from user managed event queue in database
   * arguments:
   *  pHandle - handle to database returned from sdnpdata_init
   *  group - object group indicating event type
   *   DNPDEFS_OBJ_2_BIN_CHNG_EVENTS  or
   *   DNPDEFS_OBJ_4_DBL_CHNG_EVENTS  or
   *   DNPDEFS_OBJ_11_BIN_OUT_EVENTS  or
   *   DNPDEFS_OBJ_13_BIN_CMD_EVENTS  or 
   *   DNPDEFS_OBJ_22_CNTR_EVENTS     or
   *   DNPDEFS_OBJ_23_FCTR_EVENTS     or
   *   DNPDEFS_OBJ_32_ANA_CHNG_EVENTS or  
   *   DNPDEFS_OBJ_42_ANA_OUT_EVENTS  or
   *   DNPDEFS_OBJ_43_ANA_CMD_EVENTS  or
   *   DNPDEFS_OBJ_111_STRING_EVENTS  or 
   *   DNPDEFS_OBJ_113_VTERM_EVENTS  
   *  NOTE: To support a read of both Obj2 and Obj4 events including both of those
   *   groups intermixed in order of occurrence, this function should return the
   *   next Obj2 or Obj4 event that occurred when group argument is either 2 or 4.
   *  classMask - which class to get event for.
   *   TMWDEFS_CLASS_MASK_ONE    or
   *   TMWDEFS_CLASS_MASK_TWO    or
   *   TMWDEFS_CLASS_MASK_THREE  or
   *   TMWDEFS_CLASS_MASK_ALL 
   *  firstEvent - if TMWDEFS_TRUE, return data for first event in queue that
   *   matches group and classMask
   *  pEvent - pointer to structure to hold fields describing this event.
   * returns:
   *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_umEventGet(
    void *pHandle,
    TMWTYPES_UCHAR group,
    TMWDEFS_CLASS_MASK classMask,
    TMWTYPES_BOOL firstEvent,
    SDNPDATA_GET_EVENT *pEvent);

  /* function: sdnpdata_umEventSent
   * purpose: Mark the last event returned by sdnpdata_umEventGet() as
   *  having been sent, but not yet acked by application confirm. 
   *  Event cannot be removed from queue yet.
   * arguments:
   *  pHandle - handle to database returned from sdnpdata_init
   *  group - object group indicating event type
   *   DNPDEFS_OBJ_2_BIN_CHNG_EVENTS  or
   *   DNPDEFS_OBJ_4_DBL_CHNG_EVENTS  or
   *   DNPDEFS_OBJ_22_CNTR_EVENTS     or
   *   DNPDEFS_OBJ_23_FCTR_EVENTS     or
   *   DNPDEFS_OBJ_32_ANA_CHNG_EVENTS or  
   *   DNPDEFS_OBJ_42_ANA_OUT_EVENTS  or
   *   DNPDEFS_OBJ_43_ANA_CMD_EVENTS  or
   *   DNPDEFS_OBJ_111_STRING_EVENTS  or 
   *   DNPDEFS_OBJ_113_VTERM_EVENTS  
   *  point - point number in event 
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL sdnpdata_umEventSent(
    void *pHandle,
    TMWTYPES_UCHAR group,
    TMWTYPES_USHORT point);

  /* function: sdnpdata_umEventNotSent
   * purpose: Mark all events in this user managed queue as not yet sent.
   * arguments:
   *  pHandle - handle to database returned from sdnpdata_init
   *  group - object group indicating event type
   *   DNPDEFS_OBJ_2_BIN_CHNG_EVENTS  or
   *   DNPDEFS_OBJ_4_DBL_CHNG_EVENTS  or
   *   DNPDEFS_OBJ_22_CNTR_EVENTS     or
   *   DNPDEFS_OBJ_23_FCTR_EVENTS     or
   *   DNPDEFS_OBJ_32_ANA_CHNG_EVENTS or  
   *   DNPDEFS_OBJ_42_ANA_OUT_EVENTS  or
   *   DNPDEFS_OBJ_43_ANA_CMD_EVENTS  or
   *   DNPDEFS_OBJ_111_STRING_EVENTS  or 
   *   DNPDEFS_OBJ_113_VTERM_EVENTS  
   * returns:
   *  TMWDEFS_TRUE if specified queue is full
   *  TMWDEFS_FALSE if queue is not full
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_umEventNotSent(
    void *pHandle,
    TMWTYPES_UCHAR group);

  /* function: sdnpdata_umEventRemove
   * purpose: remove all events, that have been marked 
   *  as sent, from this user managed event queue in database
   * arguments:
   *  pHandle - handle to database returned from sdnpdata_init
   *  group - object group indicating event type
   *   DNPDEFS_OBJ_2_BIN_CHNG_EVENTS  or
   *   DNPDEFS_OBJ_4_DBL_CHNG_EVENTS  or
   *   DNPDEFS_OBJ_22_CNTR_EVENTS     or
   *   DNPDEFS_OBJ_23_FCTR_EVENTS     or
   *   DNPDEFS_OBJ_32_ANA_CHNG_EVENTS or  
   *   DNPDEFS_OBJ_42_ANA_OUT_EVENTS  or
   *   DNPDEFS_OBJ_43_ANA_CMD_EVENTS  or
   *   DNPDEFS_OBJ_111_STRING_EVENTS  or 
   *   DNPDEFS_OBJ_113_VTERM_EVENTS  
   * returns:
   *  TMWDEFS_TRUE if specified queue is full
   *  TMWDEFS_FALSE if queue is not full
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_umEventRemove(
    void *pHandle,
    TMWTYPES_UCHAR group);
#endif

#if SDNPDATA_SUPPORT_OBJ0
  /* Device Attributes */

  /* Object Group 0. Variations 0-255 are used to specify particular attributes
   * Index 0 is used for standard attributes defined by DNP User Group in  
   *  Tech Bulletin TB2003-001 and TB2005-002.
   *  Defines DNPDEFS_OBJ0xxx in dnpdefs.h can be used to identify the standard
   *  attributes for Index 0.
   *
   * Indexes other than 0 are used to specify user-specific attribute sets.
   */

  /* function: sdnpdata_deviceAttrQuantity 
   * purpose:  Return quantity of device attribute sets present in this device,
   *  including 1 for the standard DNP set and 1 for each User-specific set. 
   * arguments:
   *  pHandle - handle to database returned from sdnpdata_init
   * returns:
   *  Number of device attribute sets present (see note for
   *  sdnpdata_xxxQuantity at top of this file)
   */
  TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpdata_deviceAttrQuantity(
    void *pHandle);
 
  /* function: sdnpdata_deviceAttrGetPoint
   * purpose: Return a handle to a specific device attribute point or set
   *  of attributes
   * arguments:
   *  pHandle - handle to database returned from sdnpdata_init
   *  pointNum - point number or device attribute set to return a handle to.
   *   0 for standard attributes, other values for user-specific attribute set
   * returns:
   *  Handle for specified point (device attribute set) or TMWDEFS_NULL if this set
   *   is currently disabled.
   */
  void * TMWDEFS_CALLBACK sdnpdata_deviceAttrGetPoint(
    void *pHandle,
    TMWTYPES_USHORT pointNum);
 
  /* function: sdnpdata_deviceAttrNext
   * purpose:  Return next device attribute variation that is supported and the
   *  value of its property field. The SCL will use this when reading all device 
   *  attributes.
   * arguments:
   *  pPoint - handle returned by sdnpdata_deviceAttrGetPoint. It points to a 
   *   set of attributes.
   *  variation - variation of device attribute. 
   *   NOTE: Variation is used as an "index" when referring to device 
   *   attributes.   
   *   if variation == 0, return the first variation supported in this set of 
   *   device attributes.
   *   if variation != 0, return next variation after specified variation,
   *   This function supports gaps in the device attribute variations supported.
   *  pNextVariation - pointer to variation variable to be filled in
   *  pProperty - pointer to property variable to be filled in
   *   0x00 indicates attribute is NOT writable by master
   *   0x01 indicates attribute is writable by master
   * returns:
   *  TMWDEFS_TRUE if there is a next variation.
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_deviceAttrNext(
    void *pPoint,
    TMWTYPES_UCHAR variation,
    TMWTYPES_UCHAR *pNextVariation,
    TMWTYPES_UCHAR *pProperty);

  /* function: sdnpdata_deviceAttrGetVar
   * purpose: Return a handle to a specific device attribute. This handle is
   *  used by the routines below to read and write values to this attribute.
   * arguments:
   *  pPoint - handle to device attribute set returned by 
   *   sdnpdata_deviceAttrGetPoint
   *  point - point number of device attribute to get handle for, 
   *   0 for standard attributes, other values for user-specific attribute set
   *  variation - variation of device attribute within that set.
   * returns:
   *  Pointer to specified device attribute or TMWDEFS_NULL if this attribute
   *   does not exist
   */
  void * TMWDEFS_GLOBAL sdnpdata_deviceAttrGetVar( 
    void *pPoint,
    TMWTYPES_UCHAR variation);

  /* function: sdnpdata_deviceAttrRead
   * purpose: Read specified device attribute
   * arguments:
   *  pAttribute - handle to device attribute returned from 
   *   sdnpdata_deviceAttrGetVar().
   *  pData - pointer to DNPDATA_ATTRIBUTE_VALUE structure to be filled in.
   *   pData->type indicates type of data in the union pData->value
   *     DNPDEFS_ATTRIBUTE_TYPE_VSTR,
   *     DNPDEFS_ATTRIBUTE_TYPE_UINT,
   *     DNPDEFS_ATTRIBUTE_TYPE_INT,
   *     DNPDEFS_ATTRIBUTE_TYPE_FLT,
   *     DNPDEFS_ATTRIBUTE_TYPE_OSTR,
   *     DNPDEFS_ATTRIBUTE_TYPE_BSTR,
   *     DNPDEFS_ATTRIBUTE_TYPE_DNP3TIME
   *   NOTE: Types are specified for the device attributes defined in  
   *    Tech Bulletin TB2003-001 and TB2005-002. For quantities UINT is used.
   *    To indicate support INT (0 or 1) is used. Most of the others are VSTR.
   * returns:
   *  TMWDEFS_TRUE if read was successful 
   *  TMWDEFS_FALSE otherwise
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_deviceAttrRead(
    void *pAttribute,
    DNPDATA_ATTRIBUTE_VALUE *pData);

  /* function: sdnpdata_deviceAttrWrite
   * purpose: Write values to specified device attribute
   *  NOTE: this function should fail if property for this
   *  device attribute indicates it is not writable.
   * arguments:
   *  pAttribute - handle to attribute returned from sdnpdata_deviceAttrGetVar().
   * returns:
   *  TMWDEFS_TRUE if write was successful 
   *  TMWDEFS_FALSE otherwise
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_deviceAttrWrite(
    void *pAttribute,
    DNPDATA_ATTRIBUTE_VALUE *pData);
#endif

  /* DATA SETS  */

  /* function: sdnpdata_datasetGetDescription  
   * purpose: Get description of this point
   *  NOTE: this is used only by sdnpxml/sdnpxml2.c if SDNPDATA_SUPPORT_XMLxx is TRUE
   *  code to generate the configuration file or Device Profile for this device. 
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.  
   * returns:
   *  pointer to string describing point
   *  TMWDEFS_NULL if failure
   */
TMWTYPES_CHAR * TMWDEFS_GLOBAL sdnpdata_datasetGetDescription(
  void *pPoint);

#if SDNPDATA_SUPPORT_OBJ85
  /* Data Set Prototypes */

  /* function: sdnpdata_datasetProtoQuantity
   * purpose: Return the number of Data Set prototypes in the 
   *  specified database.
   * arguments:
   *  pHandle - handle to database returned from sdnpdata_init
   * returns:
   *  The number of Data Set prototypes
   *  NOTE: these must be sequential starting from prototype id
   *   or point index 0.
   */
  TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpdata_datasetProtoQuantity(
    void *pHandle);

  /* function: sdnpdata_datasetProtoGetID
   * purpose: Return a prototype ID or point index for a Data Set prototype 
   *  with this UUID, if one exists in the database.
   *  This point number is used by sdnpdata_datasetProtoGetPoint to prepare
   *  a prototype for reading.
   * arguments:
   *  pHandle - handle to database returned from sdnpdata_init
   *  pUUID - pointer to 16 byte UUID string to be looked up
   *  pPointNum - point number or prototype id to be filled in.
   * returns:
   *  TMWDEFS_TRUE if prototype was found
   *  TMWDEFS_FALSE otherwise
   */
  TMWTYPES_BOOL TMWDEFS_CALLBACK sdnpdata_datasetProtoGetID(
    void *pHandle,
    TMWTYPES_UCHAR *pUUID,
    TMWTYPES_USHORT *pPointNum);

  /* function: sdnpdata_datasetProtoGetPoint
   * purpose: Return a handle to a specific Data Set prototype. This handle is
   *  used by the routine below to read values for this prototype.
   * arguments:
   *  pHandle - handle to database returned from sdnpdata_init
   *  pointNum - point number or prototype id to return a handle to.
   * returns:
   *  Handle for a specified prototype or TMWDEFS_NULL if this prototype
   *   is currently disabled or not found.
   */
  void * TMWDEFS_CALLBACK sdnpdata_datasetProtoGetPoint(
    void *pHandle,
    TMWTYPES_USHORT pointNum);
 
  /* function: sdnpdata_datasetProtoRead
   * purpose: Get a pointer to array of Data Set prototype element
   *   structures to support read of object group 85 variation 1
   *  pPoint - handle to Data Set prototype "data point" returned by 
   *   sdnpdata_datasetProtoGetPoint function.
   *  pNumberElems - number of elements returned for specified Data Set prototype
   *   not counting the mandatory prototype id and UUID, which should not
   *   be contained in the array of elements.
   *  pUUID - pointer to a 16 byte array to be filled in by this function with
   *   a UUID uniquely identifying this prototype.
   * returns:
   *  Pointer to array of Data Set prototype contents structures
   *   DNPDATA_DATASET_DESCR_ELEM.  This points to memory maintained
   *   by the database. It can be in ROM or in RAM. This pointer will need
   *   to be valid until sdnpdata_datasetProtoRelease() is called
   *   NOTE: The array of elements should not contain the mandatory Prototype ID
   *   and UUID. Prototype ID was specified when xxxGetPoint was called and
   *   UUID will be returned by pUUID parameter. If a Namespace and Name 
   *   element are present in the prototype they must be the first and second 
   *   element in this array. If either Namespace or Name is present the other 
   *   must also be present.
   */
  DNPDATA_DATASET_DESCR_ELEM * TMWDEFS_GLOBAL sdnpdata_datasetProtoRead(
    void *pPoint,
    TMWTYPES_UCHAR *pNumberElems,
    TMWTYPES_UCHAR *pUUID);
 
  /* function: sdnpdata_datasetProtoRelease
   * purpose: Release the pointer that was returned by in sdnpdata_datasetProtoRead 
   *  The database is free to deallocate or reuse the memory that was pointed to. The
   *  SCL will not attempt to reference that pointer anymore.
   *  pPoint - handle to Data Set descriptor "data point" returned by 
   *   sdnpdata_datasetDescrGetPoint function. 
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL sdnpdata_datasetProtoRelease(
    void *pPoint);

  /* function: sdnpdata_datasetProtoCreatePoint
   * purpose: Create a Data Set prototype if it does not exist and return a handle 
   *  to it. This handle is used by the routines below to write values for this 
   *  prototype.
   * arguments:
   *  pHandle - handle to database returned from sdnpdata_init
   *  pointNum - point number or prototype id to return a handle to.
   *   This should start after the last index for prototypes already found on the
   *   outstation(slave).
   *  pUUID - pointer to 16 byte string containing UUID to be stored
   * returns:
   *  Handle for a specified prototype or TMWDEFS_NULL if this prototype does
   *   not exist or cannot be created for writing to.
   */
  void * TMWDEFS_GLOBAL sdnpdata_datasetProtoCreatePoint(
    void *pHandle,
    TMWTYPES_USHORT pointNum,
    TMWTYPES_UCHAR *pUUID);

  /* function: sdnpdata_datasetProtoWrite
   * purpose: Write a Data Set prototype element to support write of object 
   *   group 85 variation 1
   *  pPoint - handle to Data Set prototype "data point" returned by 
   *   sdnpdata_datasetProtoCreatePoint function.
   *  index - index of element in Data Set prototype to be written starting
   *   with index 0. The mandatory Data Set prototype ID and UUID will not be
   *   written using this function. They will be written to the database using 
   *   the sdnpdata_datasetProtoCreatePoint() function.
   *  pElem - pointer to structure containing prototype element to be written
   *   NOTE: if pElem->ancillaryValue->type == DNPDATA_VALUE_STRPTR, this is just
   *    a pointer, the string itself must be copied somewhere. 
   * returns: 
   *  TMWDEFS_TRUE if write is successful
   *  TMWDEFS_FALSE if write failed
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_datasetProtoWrite(
    void *pPoint,
    TMWTYPES_UCHAR index,
    DNPDATA_DATASET_DESCR_ELEM *pElem); 
#endif

#if SDNPDATA_SUPPORT_OBJ86
  /* Data Set Descriptors */

  /* function: sdnpdata_datasetDescrQuantity
   * purpose: Return the number of Data Set descriptors in the 
   *  specified database.
   * arguments:
   *  pHandle - handle to database returned from sdnpdata_init
   * returns:
   *  The number of Data Set descriptors (see note for
   *  sdnpdata_xxxQuantity at top of this file)
   */
  TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpdata_datasetDescrQuantity(
    void *pHandle);

  /* function: sdnpdata_datasetDescrGetPoint
   * purpose: Return a handle to a specific data descriptor. This handle is
   *  used by the routines below to read values for this descriptor.
   * arguments:
   *  pHandle - handle to database returned from sdnpdata_init
   *  pointNum - point number or Data Set id to return a handle to.
   * returns:
   *  Handle for a specified data descriptor or TMWDEFS_NULL if this
   *   is currently disabled or not found.
   */
  void * TMWDEFS_CALLBACK sdnpdata_datasetDescrGetPoint(
    void *pHandle,
    TMWTYPES_USHORT pointNum);

  /* function: sdnpdata_datasetDescrReadCont
   * purpose: Get a pointer to array of Data Set descriptor contents
   *   structures to support read of object group 86 variation 1
   *  pPoint - handle to Data Set descriptor "data point" returned by 
   *   sdnpdata_datasetDescrGetPoint function.
   *  pNumberElems - number of elements returned in specified Data Set array.
   * returns:
   *  Pointer to array of Data Set descriptor contents structures
   *   DNPDATA_DATASET_DESCR_ELEM. This points to memory maintained
   *   by the database. It can be in ROM or in RAM. This pointer will need
   *   to be valid until sdnpdata_datasetDescrRel() is called
   *  NOTE: Data Set id is a mandatory element and was specified in
   *  sdnpdata_datasetDescrGetPoint(). It is not returned as part of the 
   *  DNPDATA_DATASET_DESCR_ELEM array. Descriptor name is an optional element, 
   *  but if present, must be the first element in the array returned.
   */
  DNPDATA_DATASET_DESCR_ELEM * TMWDEFS_GLOBAL sdnpdata_datasetDescrReadCont(
    void *pPoint,
    TMWTYPES_UCHAR *pNumberElems);

  /* function: sdnpdata_datasetDescrReadChars
   * purpose: Read characteristics of Data Set descriptor specified
   *   to support read of object group 86 variation 2.
   *  pPoint - handle to Data Set descriptor "data point" returned by 
   *   sdnpdata_datasetDescrGetPoint function.
   *  pValue - pointer to characteristics variable to be filled in by this
   *   function. The following bit definitions may be OR'ed together.
   *    DNPDEFS_DATASET_CHAR_RD  set if data set is readable
   *    DNPDEFS_DATASET_CHAR_WR  set if data set is writable 
   *    DNPDEFS_DATASET_CHAR_ST  set if outstation maintains a static data set
   *      Note: a read of object 87 would fail if this bit is not set.
   *    DNPDEFS_DATASET_CHAR_EV  set if outstation generates a data set event
   *    DNPDEFS_DATASET_CHAR_DF  set if defined by master
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL sdnpdata_datasetDescrReadChars(
    void *pPoint,
    TMWTYPES_UCHAR *pValue);

  /* function: sdnpdata_datasetDescrReadIndex
   * purpose: Get a pointer to array of Data Set descriptor index
   *   structures to support read of object group 86 variation 3
   *  pPoint - handle to Data Set descriptor "data point" returned by 
   *   sdnpdata_datasetDescrGetPoint function.
   *  pNumberElems - number of elements returned in specified Data Set array
   * returns:
   *  Pointer to array of Data Set descriptor index structures
   *   DNPDATA_DATASET_DESCR_INDEX. This points to memory maintained
   *   by the database. It can be in ROM or in RAM. This pointer will need
   *   to be valid until sdnpdata_datasetDescrRelease() is called
   *   NOTE: there should be one element of this array for each data and
   *    control value element in the Data Set descriptor (including data and 
   *    control value elements in the contained prototypes).
   */
  DNPDATA_DATASET_DESCR_INDEX * TMWDEFS_GLOBAL sdnpdata_datasetDescrReadIndex(
    void *pPoint,
    TMWTYPES_UCHAR *pNumberElems);
 
  /* function: sdnpdata_datasetDescrRelease
   * purpose: Release the pointer that was returned by 
   *  sdnpdata_datasetDescrReadCont or sdnpdata_datasetDescrReadIndex. 
   *  The database is free to deallocate or reuse the memory that was pointed to.
   *  The SCL will not attempt to reference that pointer anymore.
   *  pPoint - handle to Data Set descriptor "data point" returned by 
   *   sdnpdata_datasetDescrGetPoint function. 
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL sdnpdata_datasetDescrRelease(
    void *pPoint);

  /* function: sdnpdata_datasetDescrCreatePoint
   * purpose: Create a Data Set descriptor if it does not exist and return a  
   *  handle to it. This handle is used by the routines below to write values 
   *  for this descriptor.
   * arguments:
   *  pHandle - handle to database returned from sdnpdata_init
   *  pointNum - point number or Data Set id to return a handle to.
   *   This should start after the last index for descriptors already found on
   *   the outstation(slave).
   * returns:
   *  Handle for a specified descriptor or TMWDEFS_NULL if this descriptor does
   *   not exist or cannot be created for writing to.
   */
  void * TMWDEFS_GLOBAL sdnpdata_datasetDescrCreatePoint(
    void *pHandle,
    TMWTYPES_USHORT pointNum) ;

  /* function: sdnpdata_datasetDescrWriteCont
   * purpose: Write a Data Set descriptor contents structure to support 
   *   write of object group 86 variation 1
   *  pPoint - handle to Data Set descriptor "data point" returned by 
   *   sdnpdata_datasetDescrCreatePoint function.
   *  index - index of element in Data Set descriptor to be written starting
   *   with index 0. The mandatory Data Set descriptor ID will not be written 
   *   using this function. It will be written to the database using 
   *   the sdnpdata_datasetDescrCreatePoint() function.
   *  pElem - pointer to structure containing data to be written
   *   NOTE: if pElem->ancillaryValue->type == DNPDATA_VALUE_STRPTR, this is just
   *    a pointer, the string itself must be copied somewhere. 
   * returns: 
   *  TMWDEFS_TRUE if write is successful
   *  TMWDEFS_FALSE if write failed
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_datasetDescrWriteCont(
    void *pPoint,
    TMWTYPES_UCHAR index,
    DNPDATA_DATASET_DESCR_ELEM *pElem); 

  /* function: sdnpdata_datasetDescrWriteIndex
   * purpose: Write a Data Set descriptor index structure to support 
   *   write of object group 86 variation 3
   *  pPoint - handle to Data Set descriptor "data point" returned by 
   *   sdnpdata_datasetDescrGetPoint function.
   *  index - index of element in Data Set descriptor to be written starting
   *   with index 0. There will be one of these for each data and control 
   *   value element in the descriptor and in any prototypes contained by it.
   *  pElem - pointer to structure containing data to be written
   * returns:
   *  TMWDEFS_TRUE if write is successful
   *  TMWDEFS_FALSE if write failed
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_datasetDescrWriteIndex(
    void *pPoint,
    TMWTYPES_UCHAR index,
    DNPDATA_DATASET_DESCR_INDEX *pElem); 

  /* function: sdnpdata_datasetAssignClass
   * purpose: Assign the class in which events for this Data Set descriptor
   *  will belong.
   * arguments:
   *  pPoint - handle to Data Set returned from sdnpdata_datasetDescrGetPoint 
   *   function.
   *  classMask - new class in which to generate events from this point
   * returns:
   *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_datasetDescrAssignClass(
    void *pPoint,
    TMWDEFS_CLASS_MASK classMask);

  /* function: sdnpdata_datasetDescrEventClass
   * purpose: Return the class in which events for this Data Set descriptor
   *  belong.
   * arguments:
   *  pPoint - handle to Data Set returned from sdnpdata_datasetDescrGetPoint
   * returns:
   *  Class in which these events (Object Group 88) will be returned
   */
  TMWDEFS_CLASS_MASK TMWDEFS_GLOBAL sdnpdata_datasetDescrEventClass(
    void *pPoint);
#endif

#if SDNPDATA_SUPPORT_OBJ87
  /* Data Set Contents */

  /* function: sdnpdata_datasetQuantity
   * purpose: Return the number of datasets in the specified database.
   * arguments:
   *  pHandle - handle to database returned from sdnpdata_init
   * returns:
   *  The number of datasets (see note for
   *  sdnpdata_xxxQuantity at top of this file)
   */
  TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpdata_datasetQuantity(
    void *pHandle);

  /* function: sdnpdata_datasetGetPoint
   * purpose: Return a handle to a specific Data Set point. This handle is
   *  used by the routines below to read values from this Data Set.
   * arguments:
   *  pHandle - handle to database returned from sdnpdata_init
   *  pointNum - point number or Data Set ID to return a handle to.
   * returns:
   *  Handle for a specified Data Set or TMWDEFS_NULL if this Data Set
   *   is currently disabled or not found.
   */
  void * TMWDEFS_CALLBACK sdnpdata_datasetGetPoint(
    void *pHandle,
    TMWTYPES_USHORT pointNum);

  /* function: sdnpdata_datasetRead
   * purpose: Get a pointer to array of Data Set structures to support read 
   *   of object group 87 variation 1
   *  pPoint - handle to Data Set "data point" returned by 
   *   sdnpdata_datasetGetPoint function.
   *  pNumberElems - number of elements returned in Data Set array
   *  pTimeStamp - location in memory to copy timeStamp for this Data Set to.
   * returns:
   *  Pointer to array of Data Set value structures SDNPDATA_DATASET_VALUE. 
   *   This points to memory maintained by the database. This pointer will need 
   *   to be valid until sdnpdata_datasetRelease() is called.
   *   NOTE: The array of elements should not contain the mandatory Data Set ID
   *   and timeStamp. Data Set ID was specified when xxxGetPoint was called and
   *   timeStamp should be returned by pTimeStamp parameter.
   */
  DNPDATA_DATASET_VALUE * TMWDEFS_GLOBAL sdnpdata_datasetRead(
    void *pPoint,
    TMWTYPES_UCHAR *pNumberElems,
    TMWDTIME *pTimeStamp);
   
  /* function: sdnpdata_datasetRelease
   * purpose: Release the pointer that was returned by sdnpdata_datasetRead().
   *  The database is free to deallocate or reuse the memory that was pointed to. The
   *  SCL will not attempt to reference that pointer anymore.
   *  pPoint - handle to Data Set descriptor "data point" returned by 
   *   sdnpdata_datasetDescrGetPoint function. 
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL sdnpdata_datasetRelease(
    void *pPoint);

  /* function: sdnpdata_datasetCreatePoint
   * purpose: Create a Data Set if it does not exist (if desired) and return a  
   *  handle to it. This handle is used by the routines below to write or control  
   *  values for this Data Set.  
   * arguments:
   *  pHandle - handle to database returned from sdnpdata_init
   *  pointNum - point number or Data Set id to return a handle to.
   *   This should start after the last index for datasets already found on the
   *   outstation(slave).
   *  mode - indicates whether this call is caused by write, select or operate
   *   SDNPDATA_DATASET_MODE_WRITE,
   *   SDNPDATA_DATASET_MODE_SELECT,
   *   SDNPDATA_DATASET_MODE_OPERATE
   *  pTimeStamp - pointer timeStamp to be stored or TMWDEFS_NULL when cancel select is being performed. 
   * returns:
   *  Handle for a specified Data Set or TMWDEFS_NULL if this Data Set does
   *   not exist or cannot be created for writing or controlling.
   */
  void * TMWDEFS_GLOBAL sdnpdata_datasetCreatePoint(
    void *pHandle,
    TMWTYPES_USHORT pointNum,
    SDNPDATA_DATASET_MODE mode,
    TMWDTIME *pTimeStamp);

   /* function: sdnpdata_datasetWrite
    * purpose: Write a Data Set value structure to support 
    *   write of object group 87 variation 1
    *  pPoint - handle to Data Set "data point" returned by 
    *   sdnpdata_datasetCreatePoint function.
    *  index - index of element in Data Set to be written starting
    *   with index 0. The mandatory Data Set ID and timeStamp will not be  
    *   written using this function. They will be written to the database  
    *   using the sdnpdata_datasetCreatePoint() function.
    *  pElem - pointer to structure containing value to be written
    *   NOTE: if pElem->type == DNPDATA_VALUE_STRPTR, this is just
    *    a pointer, the string itself must be copied somewhere. 
    * returns:
    *  TMWDEFS_TRUE if write is successful
    *  TMWDEFS_FALSE if write failed
    */
   TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_datasetWrite(
     void *pPoint,
     TMWTYPES_UCHAR index,
     DNPDATA_DATASET_VALUE *pElem); 
   
   /* function: sdnpdata_datasetSelect
    * purpose: Determine whether operate would succeed for a Data Set to support 
    *   select of object group 87 variation 1
    *  pPoint - handle to Data Set "data point" returned by 
    *   sdnpdata_datasetCreatePoint function.
    *  pCtrlValues - pointer to array of data set control value structures 
    *   indicating element index in this data set and value for that element.
    *  numberCtrlValues - number of control values passed in pCtrlValues array
    * returns:
    *  status of select operation. Valid values are:
    *    DNPDEFS_DATASET_ST_SUCCESS - the operate command would succeed
    *    DNPDEFS_DATASET_ST_FORMAT_ERROR - the request was not accepted due
    *      to formatting errors in the request
    *    DNPDEFS_DATASET_ST_NOT_SUPPORTED - the request was not accepted because
    *      Control operations are not supported for this point
    *    DNPDEFS_DATASET_ST_ALREADY_ACTIVE - the request was not accepted because
    *      the control queue is full or the point is already active
    *    DNPDEFS_DATASET_ST_HARDWARE_ERROR - the request was not accepted due to
    *      control hardware problems
    */
   DNPDEFS_DATASET_ST TMWDEFS_GLOBAL sdnpdata_datasetSelect(
     void *pPoint,
     DNPDATA_DATASET_CTRL_VALUE *pCtrlValues,
     TMWTYPES_UCHAR numberCtrlValues); 

  /* function: sdnpdata_datasetOperate
   * purpose: Issue operate on a Data Set to support operate function code
   *   on object group 87 variation 1
   *  pPoint - handle to Data Set "data point" returned by 
   *   sdnpdata_datasetCreatePoint function.
   *  pCtrlValues - pointer to array of data set control value structures 
   *   indicating element index in this data set and value for that element.
   *  numberCtrlValues - number of control values passed in pCtrlValues array
   * returns:
   *  status of DATASET operation. Valid values are:
   *    DNPDEFS_DATASET_ST_SUCCESS - the command was performed successfully
   *    DNPDEFS_DATASET_ST_FORMAT_ERROR - the request was not accepted due
   *      to formatting errors in the request
   *    DNPDEFS_DATASET_ST_NOT_SUPPORTED - the request was not accepted because
   *      Control operations are not supported for this point
   *    DNPDEFS_DATASET_ST_ALREADY_ACTIVE - the request was not accepted because
   *      the control queue is full or the point is already active
   *    DNPDEFS_DATASET_ST_HARDWARE_ERROR - the request was not accepted due to
   *      control hardware problems
   */
  DNPDEFS_DATASET_ST TMWDEFS_GLOBAL sdnpdata_datasetOperate(
    void *pPoint,
    DNPDATA_DATASET_CTRL_VALUE *pCtrlValues,
    TMWTYPES_UCHAR numberCtrlValues); 
      
  /* function: sdnpdata_datasetCancelSelect
   * purpose: cancel outstanding Select on the specified point
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL sdnpdata_datasetCancelSelect(
    void *pPoint);
#endif
 
  /* function: sdnpdata_activateConfig
   * purpose: Process activate configuration (FC31) command received from master
   *  and provide result status and time delay to be sent back in Object Group 91 
   *  variation 1 response.
   * arguments:
   *  pHandle - handle to database returned from sdnpdata_init
   *  pointNum - point number received in request from master if request contained 
   *    object 110 string. Its meaning is a local matter. A request containing 
   *    object 70 variation 8 does not contain a point number and pointNum will 
   *    be set to zero.
   *  lastString - TMWDEFS_TRUE  - this is the last string in the activate config 
   *    request.   TMWDEFS_FALSE - otherwise.
   *  pString - pointer to string received from master
   *  stringLength - length of string
   *  pDelay - pointer to ULONG that should be filled in to indicate how long
   *   outstation expects to be busy activating the configuration.
   *   Only the last delay returned by this function 
   *  (when lastString == TMWDEFS_TRUE) will be sent back to the master.
   *  pErrorText - pointer to optional error text that can be filled in 
   *  pErrorLength - max length of error text allowed. Should be filled in
   *   to indicate the length of pErrorText returned to SCL
   * returns: 
   *  status of activate config command
   *   Possible status values:
   *    DNPDEFS_ACTCONFIG_SUCCESS    - success
   *    DNPDEFS_ACTCONFIG_REQERROR   - error in the request object
   *    DNPDEFS_ACTCONFIG_DATAERROR  - error in the configuration data
   *    DNPDEFS_ACTCONFIG_ERROR      - any other error  
   *    DNPDEFS_ACTCONFIG_NOTCHECKED - not checked
   */
   TMWTYPES_UCHAR TMWDEFS_GLOBAL sdnpdata_activateConfig(
     void *pHandle,
     TMWTYPES_USHORT pointNum,
     TMWTYPES_BOOL   lastString,
     TMWTYPES_UCHAR *pString,
     TMWTYPES_USHORT stringLength,
     TMWTYPES_ULONG *pDelay,
     TMWTYPES_UCHAR *pErrorText,
     TMWTYPES_UCHAR *pErrorLength); 

  /* function: sdnpdata_authAssignClass 
   * purpose: Set class to which secure authentication g120v7 error events belong
   * arguments: 
   *  pHandle - handle to database returned from sdnpdata_init
   *  classMask - Class in which these events will be returned
   * returns:
   *  TMWDEFS_TRUE if successful
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_authAssignClass(
    void *pHandle,
    TMWDEFS_CLASS_MASK classMask);

  /* function: sdnpdata_authErrorEventClass
   * purpose: Return class in which secure authentication g120v7 error events belong
   * arguments: 
   *  pHandle - handle to database returned from sdnpdata_init
   * returns:
   *  Class in which these events will be returned
   */
  TMWDEFS_CLASS_MASK TMWDEFS_GLOBAL sdnpdata_authErrorEventClass(
    void *pHandle);

  /* function: sdnpdata_authIsCriticalReq
   * purpose: Determine if this message should be considered critical.
   *  The Secure Authentication spec lists function codes that are mandatory to
   *  be considered critical. Other function codes are optional.
   * arguments: 
   *  pHandle - handle to database returned from sdnpdata_init
   *  fc - function code received
   *  pRxMsg - pointer to received message
   *  msgLength - length of received message
   * returns:
   *  TMWDEFS_TRUE if this message is considered critical and should be challenged
   *  TMWDEFS_FALSE otherwise
   * NOTE: Application Confirms are a special case in the specification. To make
   *  Appl Confirms Critical you must call sdnpsesn_authPreChallApplConf() instead 
   *  of returning TMWDEFS_TRUE from this function.
   *  See sdnpsesn.h for more information about sdnpsesn_authPreChallApplConf.
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_authIsCriticalReq(
    void          *pHandle,
    TMWTYPES_UCHAR fc,
    TMWTYPES_UCHAR *pRxMsg,
    TMWTYPES_USHORT msgLength);
   
  /* function: sdnpdata_authGetRole
   * purpose: Return the role for this user.
   *  This function is currently only called by the sample code for 
   *  sdnpdata_authRequestAllowed and not by the SDNP Library itself. 
   *  It is here to make it clear that you should be considering the
   *  configured role for a user when deciding of a request should be allowed.
   * arguments: 
   *  pHandle - handle to database returned from sdnpdata_init
   *  userNumber - user number 
   * returns: The following are defined in the Secure Authentication Specification
   *  DNPAUTH_USER_ROLE_VIEWER   
   *  DNPAUTH_USER_ROLE_OPERATOR  
   *  DNPAUTH_USER_ROLE_ENGINEER     
   *  DNPAUTH_USER_ROLE_INSTALLER   
   *  DNPAUTH_USER_ROLE_SECADM    
   *  DNPAUTH_USER_ROLE_SECAUD  
   *  DNPAUTH_USER_ROLE_RBACMNT
   *  DNPAUTH_USER_ROLE_SINGLEUSER      
   */
  TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpdata_authGetRole(
    void           *pHandle,
    TMWTYPES_USHORT userNumber);

  /* function: sdnpdata_authRequestAllowed
   * purpose: Determine if this user is allowed to perform this request which has
   *  been challenged and the challenge reply from the master was valid. 
   *  NOTE: If not authorized, this event should be logged.
   * arguments: 
   *  pHandle - handle to database returned from sdnpdata_init
   *  userNumber - user number
   *  fc - function code received
   *  pRxMsg - pointer to received message
   *  msgLength - length of received message
   * returns:
   *  TMWDEFS_TRUE if this request is allowed for this user
   *  TMWDEFS_FALSE otherwise
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_authRequestAllowed(
    void           *pHandle,
    TMWTYPES_USHORT userNumber,
    TMWTYPES_UCHAR  fc,
    TMWTYPES_UCHAR *pRxMsg,
    TMWTYPES_USHORT msgLength);

  /* function: sdnpdata_authLogErrorRx
   * purpose: Log error message (Obj120V7) received from master. 
   * arguments:  
   *  pHandle - handle to database returned from sdnpdata_init
   *  userNumber - user number from error message
   *  assocId - Association Id from error message
   *  sequenceNumber - Sequence number from error message
   *  errorCode - error code from error message
   *  pTimeStamp - pointer to time stamp from message
   *  pErrorText - optional error text from message
   *  errorTextLength - length of optional error text
   * returns: 
   *  void
   */ 
  void TMWDEFS_GLOBAL sdnpdata_authLogErrorRx(
    void            *pHandle,
    TMWTYPES_USHORT  userNumber,
    TMWTYPES_USHORT  assocId,
    TMWTYPES_ULONG   sequenceNumber,
    TMWTYPES_UCHAR   errorCode,
    TMWDTIME        *pTimeStamp,
    TMWTYPES_CHAR   *pErrorText,
    TMWTYPES_USHORT  errorTextLength);  
 
  /* function: sdnpdata_authLogErrorTx
   * purpose: Log error for this user number. 
   *  1815-2012 7.5.2.4 says it is recommended that error messages be transmitted on 
   *  an association other than the one on which authentication is performed.
   * arguments:  
   *  pHandle - handle to database returned from sdnpdata_init
   *  userNumber - user number
   *  assocId - association id indicating which master/outstation association
   *  sequenceNumber - Sequence number from error message
   *  errorCode - error identifier
   *  pErrorText - pointer to optional error text
   *  errorTextLength - length of optional error text.
   *  msgSent - indicates if the error message was sent to the master. When count
   *   exceeds max, error message g120v7 will not be sent.
   * returns: 
   *  void
   */ 
  void TMWDEFS_GLOBAL sdnpdata_authLogErrorTx(
    void            *pHandle,
    TMWTYPES_USHORT  userNumber,
    TMWTYPES_USHORT  assocId,
    TMWTYPES_ULONG   sequenceNumber,
    TMWTYPES_UCHAR   errorCode,
    TMWDTIME        *pTimeStamp,
    TMWTYPES_CHAR   *pErrorText,
    TMWTYPES_USHORT  errorTextLength,
    TMWTYPES_BOOL    msgSent); 

#if SDNPCNFG_SUPPORT_SA_VERSION5
  
#if TMWCNFG_SUPPORT_CRYPTO_AESGMAC
  /* function: sdnpdata_authStoreKSQ
   * purpose: The optional MAC algorithm AES-GMAC added in SAv5 requires the 
   *   Key Change Sequence Number(KSQ) be retained over restarts.
   * arguments:  
   *  pHandle - handle to database returned from sdnpdata_init
   *  KSQ - KSQ value to be stored so that it can be retained.
   * returns: 
   *  void
   */ 
  void TMWDEFS_GLOBAL sdnpdata_authStoreKSQ(
    void            *pHandle,
    TMWTYPES_ULONG   KSQ);
     
  /* function: sdnpdata_authGetKSQ
   * purpose: The optional MAC algorithm AES-GMAC added in SAv5 requires the 
   *   Key Change Sequence Number(KSQ) be retained over restarts.
   * arguments:  
   *  pHandle - handle to database returned from sdnpdata_init
   *  pKSQ - pointer to memory to store the returned KSQ
   * returns: 
   *  void
   */ 
  void TMWDEFS_GLOBAL sdnpdata_authGetKSQ(
    void            *pHandle,
    TMWTYPES_ULONG  *pKSQ);
#endif

#if DNPCNFG_SUPPORT_AUTHKEYUPDATE
  /* The following functions are required if Update Keys can be sent over DNP as 
   * specified in DNP Secure Authentication Version 5. Symmetric update algorithms 
   * are required, asymmetric algoritms are optional.
   */

  /* function: sdnpdata_authKeyChgMethodSupport 
   * purpose: Determine if this key change method for sending update keys is supported
   * arguments:  
   *  pHandle - database handle returned from sdnpdata_init 
   *  keyChangeMethod - less than 64 are symmetric
   *    DNPAUTH_KEYCH_SYMAES128_SHA1   = Symmetric AES-128
   *    DNPAUTH_KEYCH_SYMAES256_SHA256 = Symmetric AES-256 
   *    DNPAUTH_KEYCH_SYMAES256_GMAC   = Symmetric AES-256 / AES-GMAC
   *    
   *    DNPAUTH_KEYCH_ASYM_RSA1024_SHA1 = Asymmetric RSA-1024 / DSA / SHA1
   *    DNPAUTH_KEYCH_ASYM_RSA2048_SHA256 = Asymmetric RSA-2048 / DSA / SHA256
   *    DNPAUTH_KEYCH_ASYM_RSA3072_SHA256 = Asymmetric RSA-3072 / DSA / SHA256
   *    DNPAUTH_KEYCH_ASYM_RSA2048_SHA256_GMAC = Asymmetric RSA-2048 / DSA / AES-GMAC
   *    DNPAUTH_KEYCH_ASYM_RSA3072_SHA256_GMAC = Asymmetric RSA-3072 / DSA / AES-GMAC
   *    DNPAUTH_KEYCH_ASYM_RSA1024_RSA_SHA1 = Asymmetric RSA-1024 / RSA / SHA1
   *    DNPAUTH_KEYCH_ASYM_RSA2048_RSA_SHA256 = Asymmetric RSA-2048 / RSA / SHA256
   *    DNPAUTH_KEYCH_ASYM_RSA3072_RSA_SHA256 = Asymmetric RSA-3072 / RSA / SHA256
   *    DNPAUTH_KEYCH_ASYM_RSA2048_RSA_SHA256_GMAC = Asymmetric RSA-2048 / RSA / AES-GMAC
   *    DNPAUTH_KEYCH_ASYM_RSA3072_RSA_SHA256_GMAC = Asymmetric RSA-3072 / RSA / AES-GMAC
   * returns:
   *  TMWDEFS_TRUE if supported
   *  TMWDEFS_FALSE otherwise
   */ 
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_authKeyChgMethodSupport(
    void           *pHandle,
    TMWTYPES_UCHAR  keyChangeMethod);  

  /* function: sdnpdata_authGetOSName 
   * purpose: Get the preconfigured name of this outstation. The master must also 
   *  know this same name to send update keys over DNP.
   * arguments:  
   *  pHandle - database handle returned from sdnpdata_init
   *  userNumber - user number that was returned by {sdnpdata_authUpdateKeyChangeReq
   *  pOSName - pointer to buffer where name should be copied
   *  pOSNameLength -  when called this is the maximum length allowed for 
   *    outstation name, on return this should be set to the length of name returned
   * returns:
   *  TMWDEFS_TRUE if successful
   *  TMWDEFS_FALSE otherwise
   */ 
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_authGetOSName(
    void            *pHandle, 
    TMWTYPES_CHAR   *pOSName,
    TMWTYPES_USHORT *pOSNameLength);
  
  /* function: sdnpdata_authGetUserName 
   * purpose: Get the globally unique name for this user
   *  that was stored by sdnpdata_authUserStatusChngRcvd
   * arguments:  
   *  pHandle - database handle returned from sdnpdata_init
   *  userNumber - user number that was returned by sdnpdata_authUpdateKeyChangeReq
   *  pUserName - pointer to buffer where name should be copied
   *  pUserNameLength -  when called this is the maximum length allowed for 
   *    user name, on return this should be set to the length of name to be filled in
   * returns:
   *  TMWDEFS_TRUE if successful
   *  TMWDEFS_FALSE otherwise
   */ 
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_authGetUserName(
    void            *pHandle, 
    TMWTYPES_USHORT  userNumber,
    TMWTYPES_CHAR   *pUserName,
    TMWTYPES_USHORT *pUserNameLength);
    
  /* function: sdnpdata_authUserCertRcvd 
   *  NOTE: This only needs to be implemented if optional IEC 62351 Certificate sent
   *        in g120v8 for asymmetric key update is supported
   * purpose: Parse the User IEC 62351-8 Certificate data that was received 
   *  in g120V8 message. The certificate will contain the user name, 
   *  operation, user role, and user role expiry interval, status change sequence (SCS) number,
   *  and area of responsibility.
   *  Verify the Area of Responsibility text string in the certificate matches
   *  at least one such string configured for this outstation.
   *  This function should return the user number for this user.
   *  NOTE: The SDNP Library will call a separate function to verify that sequence number in the certificate incremented.
   *  NOTE: The SDNP library will call a separate function to store this certificate
   * arguments:  
   *  pHandle - database handle returned from sdnpdata_init 
   *  keyChangeMethod - 
   *    DNPAUTH_KEYCH_ASYM_RSA1024_SHA1 = Asymmetric RSA-1024 / DSA / SHA1
   *    DNPAUTH_KEYCH_ASYM_RSA2048_SHA256 = Asymmetric RSA-2048 / DSA / SHA256
   *    DNPAUTH_KEYCH_ASYM_RSA3072_SHA256 = Asymmetric RSA-3072 / DSA / SHA256
   *    DNPAUTH_KEYCH_ASYM_RSA2048_SHA256_GMAC = Asymmetric RSA-2048 / DSA / AES-GMAC
   *    DNPAUTH_KEYCH_ASYM_RSA3072_SHA256_GMAC = Asymmetric RSA-3072 / DSA / AES-GMAC
   *    DNPAUTH_KEYCH_ASYM_RSA1024_RSA_SHA1 = Asymmetric RSA-1024 / RSA / SHA1
   *    DNPAUTH_KEYCH_ASYM_RSA2048_RSA_SHA256 = Asymmetric RSA-2048 / RSA / SHA256
   *    DNPAUTH_KEYCH_ASYM_RSA3072_RSA_SHA256 = Asymmetric RSA-3072 / RSA / SHA256
   *    DNPAUTH_KEYCH_ASYM_RSA2048_RSA_SHA256_GMAC = Asymmetric RSA-2048 / RSA / AES-GMAC
   *    DNPAUTH_KEYCH_ASYM_RSA3072_RSA_SHA256_GMAC = Asymmetric RSA-3072 / RSA / AES-GMAC
   *  certType - DNPAUTH_ID_CERTIFICATE or DNPAUTH_ATTRIB_CERTIFICATE
   *  pCertData - pointer to certificate data in message
   *  certDataLength - length of certificate data in message
   *  pStatusChangeSequence - pointer to value of SCS received from the master 
   *   to be filled in by this function.
   *  pOperation - pointer to operation variable to be filled in by this function
   *  pError - pointer to error variable to be filled in by this function if it 
   *   fails for any reason and returns 0 for a user number.
   *   DNPAUTH_ERROR_NONE if success
   *   DNPAUTH_ERROR_UNKNOWN_USER if the user does not exist
   *   DNPAUTH_ERROR_INVALIDSIG if the signature does not validate using configured
   *    Authority Public Key
   *   DNPAUTH_ERROR_INVALIDCERTDATA if the SCS value is not greater than previous SCS
   *    or Area Of Responsibility does not match a configured value or other 
   *    certification data errors.
   * returns:
   *  user number for this user name, or 0 if any failure 
   *   ADD will need to determine a new unused userNumber.
   *   DELETE must still return correct nonzero user number if successful. 
   *   For DELETE, if successful, invalidate the Update Key for this User 
   *               (delete the user).
   *               return 0==failure if the user does not exist.
   */  
  TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpdata_authUserCertRcvd(
    void            *pHandle, 
    TMWTYPES_UCHAR   keyChangeMethod, 
    TMWTYPES_UCHAR   certType, 
    TMWTYPES_UCHAR  *pCertData, 
    TMWTYPES_USHORT  certDataLength,
    TMWTYPES_ULONG  *pStatusChangeSequence,
    TMWTYPES_UCHAR  *pOperation,
    TMWTYPES_UCHAR  *pError);

  /* function: sdnpdata_authUserStatusChgRcvd
   * purpose: Store the User Status Change data that was received in g120V10 message
   *  The library will call this after verifying the certification data received.
   *  This data must be stored and will be retrieved by other sdnpdata_xx functions.
   *  This function should assign an available user number for this user.
   *  NOTE: the role and userRoleExpiryInterval should not take affect until the 
   *        tmwcrypto_commitKey(true) is called!
   * arguments:  
   *  pHandle - database handle returned from sdnpdata_init
   *  pUserName - pointer to globally unique identifier representing a user. 
   *  userNameLength - length of user name
   *  keyChangeMethod - less than 64 are symmetric
   *    DNPAUTH_KEYCH_SYMAES128_SHA1   = Symmetric AES-128
   *    DNPAUTH_KEYCH_SYMAES256_SHA256 = Symmetric AES-256
   *    DNPAUTH_KEYCH_SYMAES256_GMAC   = Symmetric AES-256 / AES-GMAC
   *
   *    DNPAUTH_KEYCH_ASYM_RSA1024_SHA1 = Asymmetric RSA-1024 / DSA / SHA1
   *    DNPAUTH_KEYCH_ASYM_RSA2048_SHA256 = Asymmetric RSA-2048 / DSA / SHA256
   *    DNPAUTH_KEYCH_ASYM_RSA3072_SHA256 = Asymmetric RSA-3072 / DSA / SHA256
   *    DNPAUTH_KEYCH_ASYM_RSA2048_SHA256_GMAC = Asymmetric RSA-2048 / DSA / AES-GMAC
   *    DNPAUTH_KEYCH_ASYM_RSA3072_SHA256_GMAC = Asymmetric RSA-3072 / DSA / AES-GMAC
   *    DNPAUTH_KEYCH_ASYM_RSA1024_RSA_SHA1 = Asymmetric RSA-1024 / RSA / SHA1
   *    DNPAUTH_KEYCH_ASYM_RSA2048_RSA_SHA256 = Asymmetric RSA-2048 / RSA / SHA256
   *    DNPAUTH_KEYCH_ASYM_RSA3072_RSA_SHA256 = Asymmetric RSA-3072 / RSA / SHA256
   *    DNPAUTH_KEYCH_ASYM_RSA2048_RSA_SHA256_GMAC = Asymmetric RSA-2048 / RSA / AES-GMAC
   *    DNPAUTH_KEYCH_ASYM_RSA3072_RSA_SHA256_GMAC = Asymmetric RSA-3072 / RSA / AES-GMAC
   *  operation -  DNPAUTH_USER_STATUS_ADD, DNPAUTH_USER_STATUS_DELETE, 
   *            or DNPAUTH_USER_STATUS_CHANGE
   *   For CHANGE  return 0==failure if the user does not exist.
   *   For DELETE, invalidate the Update Key for this User (delete the user).
   *               return 0==failure if the user does not exist.
   *  role - user role definition, what this user is allowed to do.
   *   DNPAUTH_USER_ROLE_VIEWER     
   *   DNPAUTH_USER_ROLE_OPERATOR      
   *   DNPAUTH_USER_ROLE_ENGINEER
   *   DNPAUTH_USER_ROLE_INSTALLER
   *   DNPAUTH_USER_ROLE_SECADM      
   *   DNPAUTH_USER_ROLE_SECAUD    
   *   DNPAUTH_USER_ROLE_RBACMNT  
   *   DNPAUTH_USER_ROLE_SINGLEUSER
   *  userRoleExpiryInterval - Number of days after receiving the new 
   *   User Update Key in the g120v13 request.
   * returns:
   *  user number for this user name, or 0 if failure
   *  DELETE must still return correct nonzero user number if successful.
   */  
  TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpdata_authUserStatusChgRcvd(
    void            *pHandle,
    TMWTYPES_CHAR   *pUserName,
    TMWTYPES_USHORT  userNameLength,
    TMWTYPES_UCHAR   keyChangeMethod, 
    TMWTYPES_UCHAR   operation, 
    TMWTYPES_USHORT  userRole, 
    TMWTYPES_USHORT  userRoleExpiryInterval);

  /* function: sdnpdata_authUpdateKeyChgReq
   * purpose: Process received Update Key Change Request g120v11
   * arguments: 
   *  pHandle - database handle returned from sdnpdata_init
   *  keyChangeMethod - less than 64 are symmetric
   *    DNPAUTH_KEYCH_SYMAES128_SHA1   = Symmetric AES-128
   *    DNPAUTH_KEYCH_SYMAES256_SHA256 = Symmetric AES-256
   *    DNPAUTH_KEYCH_SYMAES256_GMAC   = Symmetric AES-256 / AES-GMAC
   *
   *    DNPAUTH_KEYCH_ASYM_RSA1024_SHA1 = Asymmetric RSA-1024 / DSA / SHA1
   *    DNPAUTH_KEYCH_ASYM_RSA2048_SHA256 = Asymmetric RSA-2048 / DSA / SHA256
   *    DNPAUTH_KEYCH_ASYM_RSA3072_SHA256 = Asymmetric RSA-3072 / DSA / SHA256
   *    DNPAUTH_KEYCH_ASYM_RSA2048_SHA256_GMAC = Asymmetric RSA-2048 / DSA / AES-GMAC
   *    DNPAUTH_KEYCH_ASYM_RSA3072_SHA256_GMAC = Asymmetric RSA-3072 / DSA / AES-GMAC
   *    DNPAUTH_KEYCH_ASYM_RSA1024_RSA_SHA1 = Asymmetric RSA-1024 / RSA / SHA1
   *    DNPAUTH_KEYCH_ASYM_RSA2048_RSA_SHA256 = Asymmetric RSA-2048 / RSA / SHA256
   *    DNPAUTH_KEYCH_ASYM_RSA3072_RSA_SHA256 = Asymmetric RSA-3072 / RSA / SHA256
   *    DNPAUTH_KEYCH_ASYM_RSA2048_RSA_SHA256_GMAC = Asymmetric RSA-2048 / RSA / AES-GMAC
   *    DNPAUTH_KEYCH_ASYM_RSA3072_RSA_SHA256_GMAC = Asymmetric RSA-3072 / RSA / AES-GMAC
   *  pUserName - pointer to globally unique identifier representing user.
   *  userNameLength - length of user name
   * returns:
   *  user number for this user name (should be same number returned from 
   *  sdnpdata_authUserCertRcvd or sdnpdata_authUserStatusChgRcvd)
   *  or 0 if failure 
   */  
  TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpdata_authUpdateKeyChgReq( 
    void            *pHandle,
    TMWTYPES_UCHAR   keyChangeMethod,
    TMWTYPES_CHAR   *pUserName,
    TMWTYPES_USHORT  userNameLength);

#endif  /* DNPCNFG_SUPPORT_AUTHKEYUPDATE */

  /* function: sdnpdata_authSecStatQuantity
   * purpose: Return the number of security statistics in the 
   *  specified database.
   * arguments:
   *  pHandle - handle to database returned from sdnpdata_init
   * returns:
   *  The number of security statistics (see note for
   *  sdnpdata_xxxQuantity at top of this file)
   */
  TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpdata_authSecStatQuantity(
    void *pHandle);

  /* function: sdnpdata_authSecStatGetPoint
   * purpose: Return a handle to a specific data point. This handle is
   *  used by the routines below to read values from this statistic 
   * arguments:
   *  pHandle - handle to database returned from sdnpdata_init
   *  pointNum - point number to return
   * returns:
   *  Pointer to specified data point or TMWDEFS_NULL if this point
   *   is currently disabled for reading.
   */
  void * TMWDEFS_GLOBAL sdnpdata_authSecStatGetPoint(
    void *pHandle,
    TMWTYPES_USHORT pointNum);
   
  /* function: sdnpdata_authSecStatThreshold 
   * purpose: Retrieve the Security Statistic max or threshold value
   * arguments:  
   *  pHandle - handle to database returned from sdnpdata_init
   *  index - index indicating which statistic
   *   for example DNPAUTH_UNEXPECTED_MSG_INDEX
   */
  TMWTYPES_USHORT TMWDEFS_GLOBAL sdnpdata_authSecStatThreshold(
    void            *pHandle,
    TMWTYPES_USHORT  index);

  /* function: sdnpdata_authSecStatSet 
   * purpose: Set the Security Statistic for this index.
   * arguments:  
   *  pPoint - handle to data point returned from 'getPoint' function.
   *  value - value to set for this statistic
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL sdnpdata_authSecStatSet(
    void            *pPoint,
    TMWTYPES_ULONG   value);

  /* function: sdnpdata_authSecStatEventClass
   * purpose: Retrieve the event class for this point
   * NOTE IEEE 1815-2012 Table 5-2 says this statistic SHALL be in 
   *  class 1, 2,or 3. It must be in an event class.
   * arguments:   
   *  pPoint - handle to data point returned from 'getPoint' function.
   * returns:
   *  Class in which these events will be returned
   */
  TMWDEFS_CLASS_MASK TMWDEFS_GLOBAL sdnpdata_authSecStatEventClass(
    void *pPoint);

  /* function: sdnpdata_authSecStatIsClass0
   * purpose: Should this point be reported in response to an object 60 
   *   variation 1 read request. This allows individual points to be excluded
   *   from a class 0 response but still readable by a specific object group
   *   read request.
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   * returns:
   *  TMWDEFS_TRUE if point should be reported.
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_authSecStatIsClass0(
    void *pPoint);

  /* function: sdnpdata_authSecStatEventDefVariation 
   * purpose: Determine default variation for this security statistic
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   *  classMask - mask indicating what event class is being requested
   * returns:
   *  default variation for this point
   *  NOTE: this will only be called if the default variation for security 
   *   statistic change events obj122DefaultVariation is configured as zero
   */
  TMWTYPES_UCHAR TMWDEFS_GLOBAL sdnpdata_authSecStatEventDefVariation(
    void *pPoint,
    TMWDEFS_CLASS_MASK classMask);

  /* function: sdnpdata_authSecStatEventMode 
   * purpose: Determine event mode for this point
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   * returns:
   *  event mode for this point, SOE or LAST
   *  NOTE: this will only be called if the event mode for security statistics
   *   if securityStatEventMode is configured as TMWDEFS_EVENT_MODE_PER_POINT
   */
  TMWDEFS_EVENT_MODE TMWDEFS_GLOBAL sdnpdata_authSecStatEventMode(
    void *pPoint);

  /* function: sdnpdata_authSecStatAssignClass
   * purpose: Assign the class in which events from this data point
   *  will belong.
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   *  classMask - new class in which to generate events from this point
   * returns:
   *  TMWDEFS_TRUE if successful, else TMWDEFS_FALSE
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_authSecStatAssignClass(
    void *pPoint,
    TMWDEFS_CLASS_MASK classMask);

  /* function: sdnpdata_authSecStatRead
   * purpose: Return the current value of the specified point.
   * arguments:
   *  pPoint - handle to data point returned from 'getPoint' function.
   *  pValue - pointer to location to store current value
   *  pFlags - pointer to location to store current DNP3 flags
   *   pFlags contains a status indication and the current state of the point.
   *   The following values (or OR'd combinations) are valid for this type:
   *      DNPDEFS_DBAS_FLAG_OFF_LINE - the point is off-line, and the returned
   *        state of this point may not be correct
   *      DNPDEFS_DBAS_FLAG_ON_LINE - the binary counter point has been read
   *        successfully
   *      DNPDEFS_DBAS_FLAG_RESTART - the field device that originated the
   *        data object has been restarted. This device may be the device
   *        reporting this data object.
   *      DNPDEFS_DBAS_FLAG_COMM_LOST - the device reporting this data object
   *        has lost communication with the originator of the data object
   *      DNPDEFS_DBAS_FLAG_REMOTE_FORCED - the state of the binary object
   *        has been forced to its current state at the originating device
   *      DNPDEFS_DBAS_FLAG_LOCAL_FORCED - the state of the binary object
   *        has been forced to its current state at the device reporting
   *        this data object
   *      DNPDEFS_DBAS_FLAG_STAT_ROLLOVER - the accumulated value has exceeded
   *        has exceeded its maximum and rolled over to zero. The counter 
   *        value should be set to 0 upon rollover, and counting is resumed as 
   *        normal. The Rollover bit should be cleared when the counter value 
   *        and roll-over state have been reported.
   *      DNPDEFS_DBAS_FLAG_DISCONTINUITY - value cannot be compared against 
   *        a prior value to obtain the correct count difference
   * returns:
   *  void 
   */
  void TMWDEFS_GLOBAL sdnpdata_authSecStatRead(
    void *pPoint,
    TMWTYPES_ULONG *pValue, 
    TMWTYPES_UCHAR *pFlags);
    
  /* function: sdnpdata_authSesnKeyStatCount
   * purpose:  Number of Key Status Requests within Expected Session Key Change 
   *  Interval has been exceeded. Notify a human and if other associations exist,
   *  send an Error Event g120v7 with code 12 Max Session Key Status Requests 
   *  Exceeded on those associations.
   * arguments:   
   *  pHandle - database handle returned from sdnpdata_init 
   *  assocId - association id for status request
   *  userNumber - userNumber from status request
   *  sequenceNumber - key change sequence number from status request
   * returns: 
   */  
  void TMWDEFS_GLOBAL sdnpdata_authSesnKeyStatCount(
    void            *pHandle,  
    TMWTYPES_USHORT  assocId,
    TMWTYPES_USHORT  userNumber,
    TMWTYPES_ULONG   sequenceNumber);

  /* function: sdnpdata_authLogUnexpectedMsg
   * purpose:  log that an unexpected message was received 
   * arguments:   
   *  pHandle - database handle returned from sdnpdata_init 
   *  state - current master state ie SDNPAUTH_STATE_XXX
   *  event - event being processes ie SDNPAUTH_EVT_XX
   *  pRxFragment - pointer to message that was received.
   * returns: 
   */  
  void TMWDEFS_GLOBAL sdnpdata_authLogUnexpectedMsg(
    void            *pHandle, 
    TMWTYPES_UCHAR   state, 
    TMWTYPES_ULONG   event, 
    TMWSESN_RX_DATA *pRxFragment); 

  /* function: sdnpdata_authLogTx
   * purpose:  log the Secure Authentication message that is being sent
   *  to Master. g120v7 says this event should be queued for transmission on other
   *  associations if they exist. (only the most recent shall be buffered)
   * arguments:   
   *  pHandle - database handle returned from sdnpdata_init
   *  variation - variation of message sent (object group 120).
   *  userNumber - user number transmitted if applicable, 0 otherwise.
   *  sequenceNumber - sequence number sent if applicable, 0 otherwise.
   *  pMsgBuf - pointer to message being sent.
   *  msgLength - length of message being sent.
   * returns: 
   *  void
   */ 
  void TMWDEFS_GLOBAL sdnpdata_authLogTx(
    void            *pHandle,
    TMWTYPES_UCHAR   variation,
    TMWTYPES_USHORT  userNumber,
    TMWTYPES_ULONG   sequenceNumber,
    TMWTYPES_UCHAR  *pMsgBuf,
    TMWTYPES_USHORT  msgLength);
 
  /* function: sdnpdata_authLogRx
   * purpose:  log the Secure Authentication message that was received
   *  from Master
   * arguments:   
   *  pHandle - database handle returned from sdnpdata_init
   *  variation - variation of message received (object group 120).
   *  userNumber - user number received
   *  sequenceNumber - sequence number received
   *  pMsgBuf - pointer to message received.
   *  msgLength - length of message received.
   * returns: 
   *  void
   */ 
  void TMWDEFS_GLOBAL sdnpdata_authLogRx(
    void            *pHandle, 
    TMWTYPES_UCHAR   variation,
    TMWTYPES_USHORT  userNumber,
    TMWTYPES_ULONG   sequenceNumber,
    TMWTYPES_UCHAR  *pMsgBuf,
    TMWTYPES_USHORT  msgLength); 

  /* function: sdnpdata_authLogMaxRekeyTCPClose 
   * purpose:  log that Rekeys Due to Authentication Failure statistic 
   *  has exceeded Max Authentication Rekeys and since this is TCP
   *  the connection will be closed.
   * arguments:   
   *  pHandle - database handle returned from sdnpdata_init 
   * returns: 
   */  
  void TMWDEFS_GLOBAL sdnpdata_authLogMaxRekeyTCPClose(
    void            *pHandle);

#endif  /* SDNPCNFG_SUPPORT_SA_VERSION5 */

#if SDNPCNFG_SUPPORT_SA_VERSION2
  /* The following functions only need to be implemented for DNP3 Secure 
   * Authentication Version 2 
   * These function will NOT be called if ONLY SA V5 is supported
   * If support for BOTH Version 2 and Version 5 is required, the 
   * following functions WILL be called.
   * See the SCL User Manual for details of how these relate to 
   * the Version 5 functions in tmwcrypto.h/c
   */

  /* function: sdnpdata_authLogChallTx
   * purpose:  log that a challenge has been sent to the master
   *    NOTE: Only required for SA_VERSION2
   * arguments:  
   *  pHandle - handle to database returned from sdnpdata_init
   *  userNumber - user number sent in challenge (0)
   *  sequenceNumber - Challenge sequence number transmitted
   *  macAlgorithm - MAC Algorithm Id transmitted
   *  reason - Reason challenge was transmitted
   *   Authentication Spec now only defines DNPAUTH_REASON_CRITICAL==1
   * returns: 
   *  void
   */ 
  void TMWDEFS_GLOBAL sdnpdata_authLogChallTx(
    void            *pHandle,
    TMWTYPES_USHORT  userNumber,
    TMWTYPES_ULONG   sequenceNumber,  
    TMWTYPES_UCHAR   macAlgorithm,
    TMWTYPES_UCHAR   reason);
 
  /* function: sdnpdata_authLogChallRplyTx
   * purpose:  log that a challenge reply has been sent to the master
   *    NOTE: Only required for SA_VERSION2
   * arguments:  
   *  pHandle - handle to database returned from sdnpdata_init
   *  userNumber - user number sent in challenge reply
   *  sequenceNumber - Challenge sequence number transmitted
   * returns: 
   *  void
   */ 
  void TMWDEFS_GLOBAL sdnpdata_authLogChallRplyTx(
    void            *pHandle,
    TMWTYPES_USHORT  userNumber,
    TMWTYPES_ULONG   sequenceNumber);

  /* function: sdnpdata_authLogKeyStatusTx
   * purpose:  log that a key status request has been sent to the master
   *    NOTE: Only required for SA_VERSION2
   * arguments:   
   *  pHandle - handle to database returned from sdnpdata_init
   *  userNumber - user number
   *  sequenceNumber - Sequence number transmitted
   *  keyWrapAgorithm - Key Wrap Algorithm ID transmitted
   *  keyStatus - key status transmitted
   *  macAlgorithm - MAC Algorithm Id transmitted
   * returns: 
   *  void
   */ 
  void sdnpdata_authLogKeyStatusTx(
    void            *pHandle, 
    TMWTYPES_USHORT  userNumber,
    TMWTYPES_ULONG   sequenceNumber,
    TMWTYPES_UCHAR   keyWrapAlgorithm,
    TMWTYPES_UCHAR   keyStatus,
    TMWTYPES_UCHAR   macAlgorithm); 
 
  /* function: sdnpdata_authLogAggrTx
   * purpose:  log that an aggressive mode request has been sent
   *    NOTE: Only required for SA_VERSION2
   * arguments:   
   *  pHandle - handle to database returned from sdnpdata_init
   *  userNumber - user number
   *  sequenceNumber - Challenge Sequence number transmitted
   * returns: 
   *  void
   */ 
  void TMWDEFS_GLOBAL sdnpdata_authLogAggrTx(
    void            *pHandle,
    TMWTYPES_USHORT  userNumber,
    TMWTYPES_ULONG   sequenceNumber);

  /* function: sdnpdata_authLogChallRx
   * purpose:  log that a challenge request has been received
   *    NOTE: Only required for SA_VERSION2
   * arguments:   
   *  pHandle - handle to database returned from sdnpdata_init
   *  userNumber - user number
   *  sequenceNumber - Challenge sequence number from message
   *  macAlgorithm - MAC Algorithm ID from message
   *  reason - reason from message
   * returns: 
   *  void
   */ 
  void TMWDEFS_GLOBAL sdnpdata_authLogChallRx(
    void            *pHandle,
    TMWTYPES_USHORT  userNumber,
    TMWTYPES_ULONG   sequenceNumber,  
    TMWTYPES_UCHAR   macAlgorithm,
    TMWTYPES_UCHAR   reason);

  /* function: sdnpdata_authLogChallRplyRx
   * purpose:  log that a challenge reply has been received
   *    NOTE: Only required for SA_VERSION2
   * arguments:   
   *  pHandle - handle to database returned from sdnpdata_init
   *  userNumber - user number from message
   *  sequenceNumber - Sequence number from message
   *  status - TMWDEFS_TRUE if challenge reply was valid
   *   TMWDEFS_FALSE otherwise 
   * returns: 
   *  void
   */ 
  void TMWDEFS_GLOBAL sdnpdata_authLogChallRplyRx(
    void            *pHandle,
    TMWTYPES_USHORT  userNumber,
    TMWTYPES_ULONG   sequenceNumber,  
    TMWTYPES_BOOL    status);

  /* function: sdnpdata_authLogAggrRx
   * purpose:  log that an aggressive mode request has been received
   *    NOTE: Only required for SA_VERSION2
   * arguments:   
   *  pHandle - handle to database returned from sdnpdata_init
   *  userNumber - user number from message
   *  sequenceNumber - Sequence number from message
   *  status - TMWDEFS_TRUE if aggressive mode request was valid
   *   TMWDEFS_FALSE otherwise 
   * returns: 
   *  void
   */ 
  void TMWDEFS_GLOBAL sdnpdata_authLogAggrRx(
    void            *pHandle,
    TMWTYPES_USHORT  userNumber,
    TMWTYPES_ULONG   sequenceNumber,  
    TMWTYPES_BOOL    status);
  
  /* function: sdnpdata_authLogKeyStatRqRx
   * purpose:  log that a key status request has been received
   *    NOTE: Only required for SA_VERSION2
   * arguments:   
   *  pHandle - handle to database returned from sdnpdata_init
   *  userNumber - user number from message
   * returns: 
   *  void
   */ 
  void TMWDEFS_GLOBAL sdnpdata_authLogKeyStatRqRx(
    void            *pHandle,
    TMWTYPES_USHORT  userNumber);

  /* function: sdnpdata_authLogKeyChangeRx
   * purpose:  log that a key change request has been received
   * arguments:   
   *  pHandle - handle to database returned from sdnpdata_init
   *    NOTE: Only required for SA_VERSION2
   *  userNumber - user number from message
   *  sequenceNumber - Key Change Sequence number from message
   *  status - TMWDEFS_TRUE if key change request was valid
   *   TMWDEFS_FALSE otherwise 
   * returns: 
   *  void
   */ 
  void TMWDEFS_GLOBAL sdnpdata_authLogKeyChangeRx(
    void            *pHandle,
    TMWTYPES_USHORT  userNumber,
    TMWTYPES_ULONG   sequenceNumber,  
    TMWTYPES_BOOL    status);
 
  /* function: sdnpdata_authDecryptKeyWrapData
   * purpose: Decrypt the data using the update key provided,
   *   return the decrypted data in pValue setting *pLength to
   *   the length of the returned data
   *    NOTE: Only required for SA_VERSION2, see utils/tmwcrypto.h for SA_VERSION5
   * arguments: 
   *  pHandle - handle to database returned from sdnpdata_init
   *  userNumber - user number
   *  algorithm - Decryption algorithm to use
   *   DNPAUTH_KEYWRAP_AES128 is the only one currently specified.
   *   other values reserved for future use or vendor specific choices 
   *  pEncryptedData - pointer to data to be decrypted
   *  encryptedDataLength - length of data to be decrypted
   *  pPlainValue - where to copy the plain (decrypted) data (including padding bytes 
   *   that may have been added at master).
   *  pPlainLength - when called this is the maximum length allowed for plain data,
   *   on return this should be set to the length of the plain data. This will 
   *   contain any padding bytes that were added at master. This function should
   *   not try to determine what padding might have been added.
   * returns:
   *  TMWDEFS_TRUE of successful
   *  TMWDEFS_FALSE otherwise
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_authDecryptKeyWrapData(
    void            *pHandle,
    TMWTYPES_USHORT  userNumber,
    TMWTYPES_UCHAR   algorithm,
    TMWTYPES_UCHAR  *pEncryptedData, 
    TMWTYPES_USHORT  encryptedValueLength, 
    TMWTYPES_UCHAR  *pPlainValue,
    TMWTYPES_USHORT *pPlainLength);
   
    /* function: sdnpdata_authHMACSupport 
     * purpose:  This function should determine whether the MAC algorithm 
     *   requested by the outstation is supported and return the length of
     *   the MAC data requested
     *    NOTE: Only required for SA_VERSION2, see utils/tmwcrypto.h for
     *          SA_VERSION5
     * arguments: 
     *  HMACAlgorithm - HMAC algorithm    
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
    TMWTYPES_CHAR sdnpdata_authHMACSupport(
      TMWTYPES_UCHAR HMACAlgorithm);

  /* function: sdnpdata_authHMACValue
   * purpose: using the specified algorithm and key calculate the 
   *  Keyed-Hash Message Authentication Code (MAC) value of the 
   *   data provided. 
   *   Copy up to the number of bytes allowed by *pMACValueLength into *pMACValue
   *   and set *pHMACValueLength to the number of bytes copied.
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
   *  pMACValue - pointer to where hashed data should be copied
   *  pMACValueLength - when called this is the maximum length allowed for hashed 
   *   data, on return this should be set to the length of the hashed data.
   * returns:
   *  TMWDEFS_TRUE of successful
   *  TMWDEFS_FALSE otherwise
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_authHMACValue(
    TMWTYPES_UCHAR    algorithm,
    DNPDATA_AUTH_KEY *pKey,
    TMWTYPES_UCHAR   *pData,
    TMWTYPES_ULONG    dataLength,
    TMWTYPES_UCHAR   *pHMACValue,
    TMWTYPES_USHORT  *pHMACValueLength);

  /* function: sdnpdata_authRandomChallengeData
   * purpose:  generate pseudo-random data,
   *  using algorithm specified in Secure Authentication Spec
   *  and FIPS 186-2 Digital Signal Standard 
   *    NOTE: Only required for SA_VERSION2, see utils/tmwcrypto.h for SA_VERSION5
   * arguments: 
   *  pBuf - pointer to where random data should be copied 
   *  minLength - minimum length of data as required by spec
   *  pLength -  when called this is the maximum length allowed for the random,
   *   on return this should be set to the length of the random data.
   * returns:
   *  TMWDEFS_TRUE of successful
   *  TMWDEFS_FALSE otherwise
   */ 
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_authRandomChallengeData( 
    TMWTYPES_UCHAR  *pBuf,
    TMWTYPES_USHORT  minLength,
    TMWTYPES_USHORT *pLength);
#endif

#ifdef __cplusplus
}
#endif

#if SDNPDATA_SUPPORT_XML2
/* The following defines and structures are used for retrieving information to
 * create the device profile for this device 
 */

/* This is used in many places, for names and other strings */
#define SDNPXML_MAXSTRINGLENGTH 255

/* deviceConfig notableAdditions, max number of notes and notable additions */
#define SDNPXML_MAX_NOTABLEADDITIONS 2

/* deviceConfig configuration methods, max number of software types */
#define SDNPXML_MAX_CONFIGSOFTWARE 2

/* deviceConfig configuration methods, max other types */
#define SDNPXML_MAX_CONFIGOTHERMETHODS 2

/* deviceConfig online and offlineXmlFileNames, max number xml filenames and notes */
#define SDNPXML_MAX_XMLFILENAMES 2
 
/* deviceConfig connectionsSupported, max other connection types */
#define SDNPXML_MAX_OTHERCONNECTIONS 2

/* networkConfig ipAddressOfRemoteDevice, max ip addresses of remote devices */
#define SDNPXML_MAX_IPADDRESSES 4


typedef struct {
  /* note is optional null terminated string
   * Empty string will not be output
   */
  TMWTYPES_CHAR note[SDNPXML_MAXSTRINGLENGTH];
  TMWTYPES_BOOL readAccess;    
  TMWTYPES_BOOL writeAccess;   
  
  /* null terminated strings
   * These two strings are required by schema. 
   */ 
  TMWTYPES_CHAR filename[SDNPXML_MAXSTRINGLENGTH];
  TMWTYPES_CHAR description[SDNPXML_MAXSTRINGLENGTH];
} SDNPDATA_XMLFILETYPE;


/* device config structure */
typedef struct {

  /* vendor name, null terminated string */ 
  TMWTYPES_CHAR vendorName[SDNPXML_MAXSTRINGLENGTH];

  /* device name, null terminated string */ 
  TMWTYPES_CHAR deviceName[SDNPXML_MAXSTRINGLENGTH];

  /* hardware version, null terminated string */ 
  TMWTYPES_CHAR hardwareVersion[SDNPXML_MAXSTRINGLENGTH];

  /* software version, null terminated string */ 
  TMWTYPES_CHAR softwareVersion[SDNPXML_MAXSTRINGLENGTH];

  /* Device Profile Document Version Number */
  TMWTYPES_ULONG documentVersionNumber;

  /* Set all that apply to TMWDEFS_TRUE */
  struct {
    TMWTYPES_BOOL none;
    TMWTYPES_BOOL level1;
    TMWTYPES_BOOL level2;
    TMWTYPES_BOOL level3;
    TMWTYPES_BOOL level4;
  } dnpLevelSupported;

  /* Set all that apply to TMWDEFS_TRUE */
  struct {
    /* SCL knows about these
     * selfAddressReservation  
     * dataSets
     * fileTransfer
     * virtualTerminal;
     * FC31ActivateConfiguration
     * secureAuthentication
     */
    TMWTYPES_BOOL mappingToIEC61850ObjectModels;
  } supportedFunctionBlocks;

  /* notableAdditions */
  /* Null terminated strings */
  /* Leave notableAdditions[x].note[0]==0 if it is not to be output. */
  /* Leave notableAdditions[x].notableAddition[0]==0 if it is not to be output. */
  struct {
    TMWTYPES_CHAR note[SDNPXML_MAXSTRINGLENGTH];
    TMWTYPES_CHAR notableAddition[SDNPXML_MAXSTRINGLENGTH];
  } notableAdditions[SDNPXML_MAX_NOTABLEADDITIONS];

  /* configurationMethods */
  /* Set all that apply to TMWDEFS_TRUE 
   * software and other are null terminated strings 
   */
  struct {
    TMWTYPES_BOOL xmlViaFileTransfer;
    TMWTYPES_BOOL xmlViaOtherTransportMechanism;    
    TMWTYPES_BOOL terminal;  

    /* software */
    /* null terminated strings, 
     * Empty strings, where name[0]==0,will not be output 
     * Empty strings, where version[0]==0,will not be output 
     */
    struct {
      TMWTYPES_CHAR name[SDNPXML_MAXSTRINGLENGTH];
      TMWTYPES_CHAR version[SDNPXML_MAXSTRINGLENGTH];
    } software[SDNPXML_MAX_CONFIGSOFTWARE];

    TMWTYPES_BOOL proprietaryFileViaFileTransfer;
    TMWTYPES_BOOL proprietaryFileViaOtherTransportMechanism;    
    TMWTYPES_BOOL direct;    
    TMWTYPES_BOOL factory;    
    TMWTYPES_BOOL protocol;   

    /* other
     * null terminated strings
     * Empty strings, where other[x][0]==0, will not be output 
     */
    TMWTYPES_CHAR other[SDNPXML_MAX_CONFIGOTHERMETHODS][SDNPXML_MAXSTRINGLENGTH]; 
  } configurationMethods;

  /* onlineXmlFileNames 
  * Set all that apply to TMWDEFS_TRUE 
  */
  struct {
    TMWTYPES_BOOL dnpDPReadSupported;
    TMWTYPES_BOOL dnpDPCapReadSupported;    
    TMWTYPES_BOOL dnpDPCfgReadSupported;  

    SDNPDATA_XMLFILETYPE xmlFile[SDNPXML_MAX_XMLFILENAMES];

    /* null terminated string
     * Empty string will not be output 
     */
    TMWTYPES_CHAR note[SDNPXML_MAX_XMLFILENAMES][SDNPXML_MAXSTRINGLENGTH];
  } onlineXmlFileNames;

  /* offlineXmlFileNames  
   * Set all that apply to TMWDEFS_TRUE 
   */
  struct {
    TMWTYPES_BOOL dnpDPReadSupported;
    TMWTYPES_BOOL dnpDPWriteSupported; 
    TMWTYPES_BOOL dnpDPCapReadSupported;    
    TMWTYPES_BOOL dnpDPCapWriteSupported;   
    TMWTYPES_BOOL dnpDPCfgReadSupported;  
    TMWTYPES_BOOL dnpDPCfgWriteSupported;   

    SDNPDATA_XMLFILETYPE xmlFile[SDNPXML_MAX_XMLFILENAMES];

    /* null terminated string
     * Empty string will not be output 
     */
    TMWTYPES_CHAR note[SDNPXML_MAX_XMLFILENAMES][SDNPXML_MAXSTRINGLENGTH];
  } offlineXmlFileNames;

  /* connectionsSupported */
  /* Set all that apply to TMWDEFS_TRUE */
  struct {
    TMWTYPES_BOOL serial;  
    TMWTYPES_BOOL network;  

    /* other is a null terminated string
     * Empty strings where other[x][0]==0 will not be output
     */
    TMWTYPES_CHAR other[SDNPXML_MAX_OTHERCONNECTIONS][SDNPXML_MAXSTRINGLENGTH];
  }connectionsSupported;

} SDNPDATA_XML_DEVICECONFIG;


/* Serial Config */
/* The following enumeration is used to specify what serialConfig  
 * serialParameters value is specified 
 */
typedef enum {
  SDNPDATA_XML_SERIALPARAMAEMPTY,
  SDNPDATA_XML_SERIALPARAMASYNC,
  SDNPDATA_XML_SERIALPARAMOTHER 
} SDNPDATA_XML_SERIALPARAMTYPE;

/* The following enumeration is used to specify what serialConfig 
 * interCharacterTimeOut value is specified 
 */
typedef enum {
  /* not output */
  SDNPDATA_XML_INTERCHAREMTPY,
  /* notChecked */
  SDNPDATA_XML_INTERCHARNOTCHECKED,
  /* noGapPermitted */
  SDNPDATA_XML_INTERCHARNOGAPPERM,
  /* valueBitTimes */
  SDNPDATA_XML_INTERCHARVALUEBIT,
  /* valueMilliseconds */
  SDNPDATA_XML_INTERCHARVALUEMILL,
  /* variable, variable field should be filled in */
  SDNPDATA_XML_INTERCHARVARIABLE
} SDNPDATA_XML_INTERCHARTOTYPE;

/* The following enumeration is used to specify what serialConfig 
 * interCharacterGap value is specified 
 */
typedef enum {
  /* no output */
  SDNPDATA_XML_INTERCHARGAPEMPTY,
  /* none */
  SDNPDATA_XML_INTERCHARGAPNONE,
  /* maximumBitTimes, set value field */
  SDNPDATA_XML_INTERCHARGAPMAXBIT,
  /* maximumMilliseconds, set value field */
  SDNPDATA_XML_INTERCHARGAPMAXMILL
} SDNPDATA_XML_INTERCHARGAPTYPE;


/* Serial Config structure */

/* The following enumeration is used to specify whether flow control option 
 * should be Asserted, DeAsserted or Neither 
 */
typedef enum {
  /* Neither asserted nor deasserted */
  SDNPDATA_XML_ASSERTED_EMPTY,
  /* asserted */
  SDNPDATA_XML_ASSERTED_ASSERTED,
  /* deasserted */
  SDNPDATA_XML_ASSERTED_DEASSERTED
} SDNPDATA_XML_ASSERTEDTYPE;

typedef struct {

  /* serial port name, null terminated string */ 
  TMWTYPES_CHAR portName[SDNPXML_MAXSTRINGLENGTH];

  /* serial parameters */
  struct {
    /* if type == SDNPDATA_XML_SERIALPARAMOTHER, fill other in. */
    SDNPDATA_XML_SERIALPARAMTYPE type;
    TMWTYPES_CHAR other[SDNPXML_MAXSTRINGLENGTH]; 
  } serialParameters; 

  /* baud rate */
  TMWTYPES_ULONG baudRate;

  /* rs232 options */
  struct {
    /* sequence of 0 or more of the following.
     * Leave other.length 0 if other is not to be output.
     */
    TMWTYPES_BOOL none;
    struct {
      TMWTYPES_BOOL assertsRTSBeforeTx;
      TMWTYPES_BOOL assertsDTRBeforeTx;
      TMWTYPES_BOOL assertsRTSBeforeRx;
      TMWTYPES_BOOL assertsDTRBeforeRx;
      TMWTYPES_BOOL alwaysAssertsRTS;
      TMWTYPES_BOOL alwaysAssertsDTR;
      SDNPDATA_XML_ASSERTEDTYPE requiresCTSBeforeTx;
      SDNPDATA_XML_ASSERTEDTYPE requiresDCDBeforeTx;
      SDNPDATA_XML_ASSERTEDTYPE requiresDSRBeforeTx;
      SDNPDATA_XML_ASSERTEDTYPE requiresRIBeforeTx;
      TMWTYPES_BOOL rxInactive;
      SDNPDATA_XML_ASSERTEDTYPE requiresCTSBeforeRx;
      SDNPDATA_XML_ASSERTEDTYPE requiresDCDBeforeRx;
      SDNPDATA_XML_ASSERTEDTYPE requiresDSRBeforeRx;
      SDNPDATA_XML_ASSERTEDTYPE requiresRIBeforeRx;
      TMWTYPES_BOOL alwaysIgnoresCTS;
      TMWTYPES_BOOL alwaysIgnoresDCD;
      TMWTYPES_BOOL alwaysIgnoresDSR;
      TMWTYPES_BOOL alwaysIgnoresRI;
      TMWTYPES_CHAR other[SDNPXML_MAXSTRINGLENGTH];
    }rs232Options;

    /* rs422 options */
    struct {
      /*
       * sequence of 0 or more of the following.
       * Other is a null terminated string. Leave other[0]==0 
       * if other is not to be output.
       */
      TMWTYPES_BOOL requiresIndicationBeforeRx;
      TMWTYPES_BOOL assertsControlBeforeTx;
      TMWTYPES_CHAR other[SDNPXML_MAXSTRINGLENGTH];
    }rs422Options;

    /* 485 options*/
    struct {
      /* sequence of 0 or more of the following.
       * Leave other[0]==0 if other is not to be output.
       */
      TMWTYPES_BOOL requiresRxInactiveBeforeTx;
      TMWTYPES_CHAR other[SDNPXML_MAXSTRINGLENGTH];
    }rs485Options;
  }flowControl; 

  /* SCL knows link status interval
   * linkStatusInterval
   */

  struct {
    /* Choice of one of these */
    /* If supported is set to TMWDEFS_FALSE it will be output as 
     * "<no />" to schema 
     */
    TMWTYPES_BOOL supported;
    TMWTYPES_CHAR yes[SDNPXML_MAXSTRINGLENGTH];
  } supportsCollisionAvoidance;

  struct {
    SDNPDATA_XML_INTERCHARTOTYPE type;
    union {
      TMWTYPES_ULONG value; 
      TMWTYPES_CHAR variable[SDNPXML_MAXSTRINGLENGTH];
    }value;
  }interCharacterTimeout;

  struct {
    SDNPDATA_XML_INTERCHARGAPTYPE type;
    TMWTYPES_ULONG value;
  } interCharacterGap;

} SDNPDATA_XML_SERIALCONFIG;


/* Network Config */
/* The following enumeration is used to specify what networkConfig 
 * typeOfEndPoint value is specified 
 */
typedef enum {
  /* tcpInitiating */
  SDNPDATA_XML_TCPINITIATING,
  /* tcpListening */
  SDNPDATA_XML_TCPLISTENING,
  /* tcpDual */
  SDNPDATA_XML_TCPDUAL,
  /* udpDatagram */
  SDNPDATA_XML_UDPDATAGRAM
} SDNPDATA_XML_ENDPOINTTYPE;

/* The following enumeration is used to specify what networkConfig 
 * tcpConnectionEstablishment value is specified
 */
typedef enum {
  /* Choose this one to cause "<>" empty value */
  SDNPDATA_XML_TCPEMPTY,
  /* allowsAll */
  SDNPDATA_XML_TCPALL,
  /* basedOnIPAddress */
  SDNPDATA_XML_TCPBASEDONIP,
  /* basedOnListOfIPAddresses */
  SDNPDATA_XML_TCPBASEDONLIST,
  /* basedOnWildcardIPAddress */
  SDNPDATA_XML_TCPBASEDONWILD,
  /* basedOnListOfWildcardIPAddresses */
  SDNPDATA_XML_TCPBASEDONWILDLIST,
  /* other */
  /* If this is specified, null terminated other string should be filled in. */
  SDNPDATA_XML_TCPOTHER,
} SDNPDATA_XML_TCPCONNECTTYPE;

/* The following enumeration is used to specify what networkConfig
 * udpPortForResponses value is specified 
 */
typedef enum {
  /* Choose this one to cause "<>" empty value */
  SDNPDATA_XML_UDPEMPTY,
  /* none */
  SDNPDATA_XML_UDPNONE, 
  /* useSourcePortNumber */
  SDNPDATA_XML_UDPUSESOURCE, 
  /* value, set value field to value to be output */
  SDNPDATA_XML_UDPVALUE 
} SDNPDATA_XML_UDPRESPONSETYPE; 

/* The following enumeration is used to specify what networkConfig 
 * timeSynchronization value is specified 
 */
typedef enum {
  /* notSupported */
  SDNPDATA_XML_TIMESYNCNOTSUP,
  /* dnpLANProcedure */
  SDNPDATA_XML_TIMESYNCLAN,
  /* dnpWriteTimeProcedure */
  SDNPDATA_XML_TIMESYNCWRITE,
  /* other, set other field to be output*/
  SDNPDATA_XML_TIMESYNCOTHER
} SDNPDATA_XML_TIMESYNCTYPE;

/* network config structure */
typedef struct {

  /* network port name */
  TMWTYPES_CHAR portName[SDNPXML_MAXSTRINGLENGTH];

  SDNPDATA_XML_ENDPOINTTYPE typeOfEndPoint;

  /* ip address of this device */
  TMWTYPES_CHAR ipAddress[SDNPXML_MAXSTRINGLENGTH];

  /* subnet mask */ 
  TMWTYPES_CHAR subnetMask[SDNPXML_MAXSTRINGLENGTH];

  /* Gateway IP Address */ 
  TMWTYPES_CHAR gatewayIPAddress[SDNPXML_MAXSTRINGLENGTH];

  /* tcpConnectionEstablishement
   * choice, specify type to indicate which one 
   */
  struct {
    SDNPDATA_XML_TCPCONNECTTYPE type;
    TMWTYPES_CHAR other[SDNPXML_MAXSTRINGLENGTH];
  } tcpConnectionEstablishment;

  /* Ip Address of Remote Device, multiple addresses allowed*/ 
  TMWTYPES_CHAR ipAddressOfRemoteDevice[SDNPXML_MAX_IPADDRESSES][SDNPXML_MAXSTRINGLENGTH];

  /* TCP Listen Port */
  struct {
    /* Choice of one of these two  
     * If notApplicable is TMWDEFS_TRUE, "<notApplicable /> will be output
     * otherwise the value specified will be output
     */
    TMWTYPES_BOOL notApplicable;
    TMWTYPES_ULONG value;
  } tcpListenPort;

  /* TCP Listen Port of remote device (when Dual End Point) */
  struct {
    /* Choice of one of these two  
     * If notApplicable is TMWDEFS_TRUE, "<notApplicable /> will be output
     * otherwise the value specified will be output
     */
    TMWTYPES_BOOL notApplicable;
    TMWTYPES_ULONG value;
  } tcpPortOfRemoteDevice;

  /* SCL knows value of TCP Keep Alive Timer  
  * tcpKeepAliveTimer;
  */

  /* Local UDP Port */
  struct {
    /* Choice of one of these two  
     * If letSystemChoose is TMWDEFS_TRUE, "<letSystemChoose /> will be output
     * otherwise the value specified will be output
     */
    TMWTYPES_BOOL letSystemChoose;
    TMWTYPES_ULONG value;
  } localUDPPort;

  /* destinationUDPPort (Masters Only), not used, should be removed. */
  TMWTYPES_ULONG destinationUDPPort;

  /* Destination UDP port for unsolicited null responses */
  struct {
    /* Choice of one of these two  
     * If none is TMWDEFS_TRUE, "<none /> will be output
     * otherwise the value specified will be output
     */
    TMWTYPES_BOOL none;
    TMWTYPES_ULONG value;
  } udpPortForUnsolicitedNullResponses;

  /* Destination UDP port for responses (if UDP only) */
  struct {
    /* Choice 
     * Set type to the correct enum, 
     * if type==SDNPDATA_XML_UDPVALUE, value will be output
     */
    SDNPDATA_XML_UDPRESPONSETYPE type;
    TMWTYPES_ULONG value;
  } udpPortForResponses;

  /* multipleOutstationConnections is a master only value */ 

  /* multipleMasterConnections */
  struct {
    /* If notSupported is TMWDEFS_TRUE only <notSupported /> will be output.
     * If notSupported is TMWDEFS_FALSE, 0 or more of the other three can be 
     *  set to TMWDEFS_TRUE 
     */
    TMWTYPES_BOOL notSupported;
    TMWTYPES_BOOL basedOnIPAddress;
    TMWTYPES_BOOL basedOnIPPortNumber;
    TMWTYPES_BOOL browsingForStaticData;
  } multipleMasterConnections;

  /* timeSynchronization */
  struct {
    /* Choice 
     * Set type to the correct enum, 
     * if type==SDNPDATA_XML_TIMESYNCOTHER, other string will be output
     */
    SDNPDATA_XML_TIMESYNCTYPE type;
    TMWTYPES_CHAR other[SDNPXML_MAXSTRINGLENGTH];
  } timeSynchronization;

} SDNPDATA_XML_NETWORKCONFIG;

/* Outstation Config */
/* The following enumeration is used to specify what outstationConfig 
 * timeSyncRequired value is specified
 */
typedef enum { 
  /* "never" */
  SDNPDATA_XML_TIMESYNCREQNEVER,
  /* "withinSecondsOfIIN14", value */
  SDNPDATA_XML_TIMESYNCREQWITHIN, 
  /* "periodically", value */
  SDNPDATA_XML_TIMESYNCREQPERIOD 
} SDNPDATA_XML_TIMESYNCREQTYPE;  

/* The following enumeration is used to specify what outstationConfig 
 * deviceTroubleBit value is specified 
 */
typedef enum { 
  /* "neverUsed" */
  SDNPDATA_XML_DEVICETROUBLENEVER,
  /* "reasonForSetting", reasonForSetting null terminated string */ 
  SDNPDATA_XML_DEVICETROUBLEREASON,
} SDNPDATA_XML_DEVICETROUBLE;  

/* The following enumeration is used to specify what outstationConfig 
 * eventBufferOverflowBehavior value is specified 
 */
typedef enum { 
  /* "discardOldest" */
  SDNPDATA_XML_EVENT_DISCOLD, 
  /* "discardNewest" */
  SDNPDATA_XML_EVENT_DISCNEW, 
  /* "other", null terminated other[] */
  SDNPDATA_XML_EVENT_OTHER, 
} SDNPDATA_XML_EVENTBUFOVTYPE;  

/* outstation config */
typedef struct {

  /* applicationLayerConfirmTimeout, SCL will determine this */

  /* timeSyncRequired */
  struct{
    SDNPDATA_XML_TIMESYNCREQTYPE type;
    TMWTYPES_ULONG value;
  } timeSyncRequired;

  /* deviceTroubleBit*/
  struct {
    SDNPDATA_XML_DEVICETROUBLE type;
    /* null terminated string */
    TMWTYPES_CHAR reasonForSetting[SDNPXML_MAXSTRINGLENGTH]; 
  } deviceTroubleBit;

  /* fileHandleTimeout, SCL will determine this */ 

  /* eventBufferOverflowBehavior */
  struct {
    SDNPDATA_XML_EVENTBUFOVTYPE type;
    /* null terminated string */
    TMWTYPES_CHAR other[SDNPXML_MAXSTRINGLENGTH];
  } eventBufferOverflowBehavior; 

  /* eventBufferOrganization  
  * This changed from a string to a more complex type in schema 2.09
  * TMWTYPES_CHAR eventBufferOrganization[SDNPXML_MAXSTRINGLENGTH]; 
  * This is perObjectGroup in the SDNP SCL.
  */

  /* sendsMultiFragmentResponses, SCL will determine this */

  /* requestsLastFragmentConfirmation, SCL will determine this */

  /* settingsPreservedThroughDeviceReset 
   * (in 2.08 schema this was renamed ...DeviceRestart, but we will 
   * keep the old name for backward code compatibility)
   */
  struct {
    /* set all that apply to TMWDEFS_TRUE */
    TMWTYPES_BOOL assignClass;
    TMWTYPES_BOOL analogDeadbands;
    TMWTYPES_BOOL dataSetPrototypes;
    TMWTYPES_BOOL dataSetDescriptors;
    TMWTYPES_BOOL FC31ActivateConfiguration;
  } settingsPreservedThroughDeviceReset;

}SDNPDATA_XML_OUTSTATIONCONFIG;

/* Outstation Performance */
/* The following enumeration is used to specify outstationPerformance types that 
 * use schema type timingPerformanceType such as maxTimeBaseDrift, 
 * referenceErrorViaDNP etc 
 */
typedef enum { 
  /* "value" set value */
  SDNPDATA_XML_TIMINGPERFVALUE,
  /* "other", set null terminated string */  
  SDNPDATA_XML_TIMINGPERFOTHER
} SDNPDATA_XML_TIMINGPERFTYPE;  

/* This is used by a number of outstationPerformance parameters */
typedef struct {
  SDNPDATA_XML_TIMINGPERFTYPE type;
  union {
    /* milliseconds, if type is SDNPDATA_XML_TIMINGPERFVALUE */
    TMWTYPES_ULONG value; 
    /* null terminated string, if type is SDNPDATA_XML_TIMINGPERFOTHER */
    TMWTYPES_CHAR other[SDNPXML_MAXSTRINGLENGTH];
  }value;
}SDNPDATA_TIMINGPERFORMANCETYPE; 

/* configuration oustationPerformance */
typedef struct {

  /* Maximum Time Base Drift (milliseconds per minute) */
  SDNPDATA_TIMINGPERFORMANCETYPE maxTimeBaseDrift; 

  /* When Does Outstation Set IIN 1.4? 
   * This can be never, or 1 or more of the other 4 values 
   * If never is TMWDEFS_TRUE it will be output
   * If never is TMWDEFS_FALSE 
   *    If atStartup is TMWDEFS_TRUE it will be output
   *    If the ULONG values are nonzero they will be output.
   */
  struct {
    TMWTYPES_BOOL  never;
    TMWTYPES_BOOL  atStartup; 
    TMWTYPES_ULONG periodically; 
    TMWTYPES_ULONG afterLastTimeSync; 
    TMWTYPES_ULONG whenTimeErrorExceeds; 
  } outstationSetsIIN14;

  /* Maximum Internal Time Reference Error When et Via DNP (ms) */
  SDNPDATA_TIMINGPERFORMANCETYPE referenceErrorViaDNP;

  /* Maximum Delay Measurement (ms) */
  SDNPDATA_TIMINGPERFORMANCETYPE delayMeasurementError;

  /* Maximum Response Time in (ms) */
  SDNPDATA_TIMINGPERFORMANCETYPE responseTime;

  /* Maximum Time From Startup to IIN 1.4 Assertion (ms)  */
  SDNPDATA_TIMINGPERFORMANCETYPE startupToIIN14;

  /* Maximum Event Time-tag Error for Local Binary and Double Bit I/O (ms)  */
  SDNPDATA_TIMINGPERFORMANCETYPE binaryOrDoubleBitEventError;

  /* Maximum Event Time-tag Error for Local I/O Other Than Binary and Double Bit 
   * Data Types (ms) 
   */
  SDNPDATA_TIMINGPERFORMANCETYPE nonBinaryOrDoubleBitEventError; 

} SDNPDATA_XML_OUTSTATIONPERFORM;

/* field config */
typedef struct {
  /* These are all null terminated strings, 
   * Empty strings, where first byte==0, will not be output 
   */
  /* Location Name Or Code */
  TMWTYPES_CHAR outstationLocation[SDNPXML_MAXSTRINGLENGTH]; 
  /* Outstation Field ID Code/Number */
  TMWTYPES_CHAR outstationId[SDNPXML_MAXSTRINGLENGTH]; 
  /* Outstation Name */
  TMWTYPES_CHAR outstationName[SDNPXML_MAXSTRINGLENGTH]; 
  /* Device Serial Number */
  TMWTYPES_CHAR deviceSerialNumber[SDNPXML_MAXSTRINGLENGTH]; 

} SDNPDATA_XML_FIELDCONFIG;

/* Security Config */
/* Number of other critical fragments */
#define SDNPXML_MAX_CRITICALFRAGMENTS 2

/* The following enumeration is used to specify current value
 * for securityConfig TLSCipherSuites
 */
typedef enum { 
  /* "notApplicable" */
  SDNPDATA_XML_TLSNA,
  
  /* "TLSRSAEncryptedAES128" */
  SDNPDATA_XML_TLSRSARAES128,

  /* "TLSRSAEncryptedRC4" */
  SDNPDATA_XML_TLSRSARC4,

  /* "TLSRSAEncrypted3DES" */
  SDNPDATA_XML_TLSRSADES,

  /* "TLSDHSignedDSSEncrypted3DES" */
  SDNPDATA_XML_TLSCHDSSDES,

  /* "TLSDHSignedRSAEncrypted3DES" */
  SDNPDATA_XML_TLSDHRSADES,   

  /* "TLSDHESignedDSSEncrypted3DES" */
  SDNPDATA_XML_TLSDHEDSSDES,

  /* "TLSDHESignedRSAEncrypted3DES" */
  SDNPDATA_XML_TLSDHERSADES,

  /* "TLSDHSignedDSSEncryptedAES128" */
  SDNPDATA_XML_TLSDSSAES128,

  /* "TLSDHSignedDSSEncryptedAES256" */
  SDNPDATA_XML_TLSDSSAES256,

  /* "TLSDHEncryptedAES128" */
  SDNPDATA_XML_TLSAES128, 

  /* "TLSDHEncryptedAES256" */
  SDNPDATA_XML_TLSAES256, 

  /* "other", set null terminated string */  
  SDNPDATA_XML_TLSOTHER

} SDNPDATA_XML_TLSCIPHERTYPE;  

/* The following enumeration is used to specify securityconfig 
 * changeCipherRequestTimeout
 */
typedef enum { 
  /* "notApplicable" */
  SDNPDATA_XML_SECURITYVALUENA,

  /* "value" set value */
  SDNPDATA_XML_SECURITYVALUEVALUE,

  /* "other", set null terminated string */  
  SDNPDATA_XML_SECURITYVALUEOTHER
} SDNPDATA_XML_SECURITYVALUETYPE;  

/* This structure is used for some security config */
typedef  struct {
  SDNPDATA_XML_SECURITYVALUETYPE type;
  union {
    TMWTYPES_ULONG value; 
    /* null terminated string, if type is SDNPDATA_XML_SECURITYVALUEOTHER */
    TMWTYPES_CHAR other[SDNPXML_MAXSTRINGLENGTH];
  }value;
} SDNP_XML_SECURITYVALUE;

/* security config structure */
typedef struct { 

  /* Cipher Suites used with DNP implementations using TLS */
  struct {
    SDNPDATA_XML_TLSCIPHERTYPE type;
    TMWTYPES_CHAR other[SDNPXML_MAXSTRINGLENGTH];
  } TLSCipherSuites; 

  /* Change cipher request timeout */
  SDNP_XML_SECURITYVALUE changeCipherRequestTimeout;

  /* Number of Certificate Authorities supported */
  TMWTYPES_ULONG numberCASupported;

  /* Certificate Revocation check time */
  SDNP_XML_SECURITYVALUE certificateRevocationCheckTime;

  /* Additional critical function codes, set the desired ones to TMWDEFS_TRUE */
  struct {
    TMWTYPES_BOOL FC0;
    TMWTYPES_BOOL FC1;
    TMWTYPES_BOOL FC7;
    TMWTYPES_BOOL FC8;
    TMWTYPES_BOOL FC9;
    TMWTYPES_BOOL FC10;
    TMWTYPES_BOOL FC11;
    TMWTYPES_BOOL FC12;
    TMWTYPES_BOOL FC22;
    TMWTYPES_BOOL FC23;
    TMWTYPES_BOOL FC25;
    TMWTYPES_BOOL FC26;
    TMWTYPES_BOOL FC27;
    TMWTYPES_BOOL FC28;
    TMWTYPES_BOOL FC30;
    TMWTYPES_BOOL FC129;
    TMWTYPES_BOOL FC130;
  } additionalCriticalFCs;
  
  /* Remote Update Key Change,
   * These changed in schema 2.09 from a boolean to a choice of one of each. 
   * The SCL allows whatever the crypto library allows so it could be multiple.
   * The Authority chooses which to actually use.
   */
  struct {
    /* DNPAUTH_KEYCH_SYMAES128_SHA1
     * DNPAUTH_KEYCH_SYMAES256_SHA256
     * DNPAUTH_KEYCH_SYMAES256_GMAC
     */
    TMWTYPES_UCHAR symmetricCrypto;

    /* DNPAUTH_KEYCH_ASYM_RSA1024_SHA1
     * DNPAUTH_KEYCH_ASYM_RSA2048_SHA256
     * DNPAUTH_KEYCH_ASYM_RSA3072_SHA256
     * DNPAUTH_KEYCH_ASYM_RSA2048_SHA256_GMAC
     * DNPAUTH_KEYCH_ASYM_RSA3072_SHA256_GMAC
     * DNPAUTH_KEYCH_ASYM_RSA1024_RSA_SHA1
     * DNPAUTH_KEYCH_ASYM_RSA2048_RSA_SHA256
     * DNPAUTH_KEYCH_ASYM_RSA3072_RSA_SHA256
     * DNPAUTH_KEYCH_ASYM_RSA2048_RSA_SHA256_GMAC
     * DNPAUTH_KEYCH_ASYM_RSA3072_RSA_SHA256_GMAC
     */
    TMWTYPES_UCHAR asymmetricCrypto;
  } remoteUpdateKeyChangeSupported;

  /* Default User Credentials are permitted to expire */
  TMWTYPES_BOOL permitUserCredentialExpiry;

  /* Other critical fragments 
   * Null terminated strings. 
   * Empty strings, ie criticalFragments[x]criticalFragment[0]==0, and note[0]==0,     
   *  will not be output 
   */ 
  struct {
    TMWTYPES_CHAR note[SDNPXML_MAXSTRINGLENGTH];
    TMWTYPES_CHAR criticalFragment[SDNPXML_MAXSTRINGLENGTH];
  } otherCriticalFragments[SDNPXML_MAX_CRITICALFRAGMENTS];

} SDNPDATA_XML_SECURITYCONFIG;

/* Binary Output Group */
/* The following enumeration is used to specify database binary output group max 
 * and minimum pulse values
 */
typedef enum {  
  /* fixed, set fixedPulseValue field */
  SDNPDATA_XML_PULSEVALUEFIXED,
  /* basedOnPointIndex */
  SDNPDATA_XML_PULSEVALUEPERPOINT
} SDNPDATA_XML_PULSEVALUETYPE;  

typedef  struct {
  SDNPDATA_XML_PULSEVALUETYPE type;
  /* set this if type SDNPDATA_XML_PULSEVALUEFIXED */
  TMWTYPES_ULONG fixedPulseValue;  
} SDNP_XML_PULSEVALUE;

/* The following enumeration is used to specify if/when command events are generated
 */
typedef enum { 
  /* "never" */  
  SDNPDATA_XML_COMMANDEVENTNEVER,
  /* "onSuccess" */
  SDNPDATA_XML_COMMANDEVENTSUCCESS,
  /* "allControlAttempts" */  
  SDNPDATA_XML_COMMANDEVENTALL
} SDNPDATA_XML_COMMANDEVENTTYPE;  


typedef struct {
  /* Set type to FIXED, or set value to minimum */
  SDNP_XML_PULSEVALUE minimumPulseTime;

  /* Set type to FIXED, or set value to maximum */
  SDNP_XML_PULSEVALUE maximumPulseTime;

  /* Set this to enum to indicate if binary output command events are generated */
  SDNPDATA_XML_COMMANDEVENTTYPE commandEvents;

} SDNPDATA_XML_BINOUTGROUPCONFIG;


/* bit values for determining XML Device Profile supportedControlOperations 
 * for each point.
 */
typedef TMWTYPES_UCHAR SDNPDATA_XML_SUP;
#define SDNPDATA_XML_SUP_SELOP           0x01
#define SDNPDATA_XML_SUP_DIROP           0x02
#define SDNPDATA_XML_SUP_DIRNOACK        0x04
#define SDNPDATA_XML_SUP_MORE_ONE        0x08
#define SDNPDATA_XML_SUP_SEL_CANCEL      0x01

/* The following enumeration is used to specify database Analog Input group 
 * deadband assignment
 */
typedef enum {  
  /* fixed */
  SDNPDATA_XML_DDBNDASSIGNFIXED,

  /* configurableViaDNP3 */
  SDNPDATA_XML_DDBNDASSIGNDNP3,

  /* configurableViaOtherMeans*/
  SDNPDATA_XML_DDBNDASSIGNOTHERMEANS, 

  /* basedOnPointIndex*/
  SDNPDATA_XML_DDBNDASSIGNPERPOINT, 

  /* other, fill in other string */
  SDNPDATA_XML_DDBNDASSIGNOTHER, 
} SDNPDATA_XML_DDBNDASSIGNTYPE;  

/* The following enumeration is used to specify database analog Input group 
 * deadband algorithm
 */
typedef enum {  
  /* simple */
  SDNPDATA_XML_DDBNDALGOSIMPLE,

  /* integrating */
  SDNPDATA_XML_DDBNDALGOINTEG, 

  /* basedOnPointIndex */
  SDNPDATA_XML_DDBNDALGOPERPOINT, 

  /* other, fill in other string */
  SDNPDATA_XML_DDBNDALGOOTHER, 
} SDNPDATA_XML_DDBNDALGOTYPE;  

/* Analog Input Group */
typedef struct { 
  struct {
    SDNPDATA_XML_DDBNDASSIGNTYPE type;

    /* if type is SDNPDATA_XML_DDBNDASSIGNOTHER
     * set this null terminated string
     */
    TMWTYPES_CHAR other[SDNPXML_MAXSTRINGLENGTH];
  } analogDeadbandAssignments;

  struct {
    SDNPDATA_XML_DDBNDALGOTYPE type;

    /* if type is SDNPDATA_XML_DDBNDALGOOTHER
     * set this null terminated string
     */
    TMWTYPES_CHAR other[SDNPXML_MAXSTRINGLENGTH];
  } analogDeadbandAlgorithm;

} SDNPDATA_XML_ANLGINGROUPCONFIG;

/* The following enumeration is used to specify database counter rollover */
typedef enum {
  /* sixteenBits */
  SDNPDATA_XML_CNTRROLLOVER16,

  /* thirtyTwoBits */
  SDNPDATA_XML_CNTRROLLOVER32,

  /* value */
  SDNPDATA_XML_CNTRROLLOVERVALUE,

  /* other */
  SDNPDATA_XML_CNTRROLLOVEROTHER,

  /* basedOnPointIndex */
  SDNPDATA_XML_CNTRROLLOVERPERPOINT

} SDNPDATA_XML_CNTRROLLOVERTYPE; 

/* The following enumeration is used to specify database counter freeze behavior */
typedef enum {
  /* masterRequest */
  SDNPDATA_XML_CNTRFROZENMASTERREQ,

  /* localFreezeWithoutTimeOfDay */
  SDNPDATA_XML_CNTRFROZENLOCALWOTIME,

  /* localFreezeRequiredTimeOfDay */
  SDNPDATA_XML_CNTRFROZENLOCALTIME,

  /* other */
  SDNPDATA_XML_CNTRFROZENOTHER

} SDNPDATA_XML_CNTRFROZENTYPE; 

/* counter group config */
typedef struct {

  struct {
    SDNPDATA_XML_CNTRROLLOVERTYPE type;
    TMWTYPES_ULONG value;
    TMWTYPES_CHAR other[SDNPXML_MAXSTRINGLENGTH];
  } counterRollOver;

  struct {
    SDNPDATA_XML_CNTRFROZENTYPE type;
    TMWTYPES_CHAR other[SDNPXML_MAXSTRINGLENGTH];
  } countersFrozen ;

} SDNPDATA_XML_COUNTERGROUPCONFIG;

/* The following enumeration is used to specify database file transfer
 * authentication and append mode
*/
typedef enum {  
  /* never */
  SDNPDATA_XML_FILENEVER,
  /* always */
  SDNPDATA_XML_FILEALWAYS,
  /* sometimes, set field sometimes with string to be output */
  SDNPDATA_XML_FILESOMETIMES 
} SDNPDATA_XML_FILETYPE;  

typedef struct {
  SDNPDATA_XML_FILETYPE type;
  /* null terminated string, if type SDNPDATA_XML_FILESOMETIMES */
  TMWTYPES_CHAR sometimes[SDNPXML_MAXSTRINGLENGTH]; 
} SDNPDATA_XML_FILECAPABILITY;

/* Structure to support database sequentialFileTransfer */
typedef struct {

  /* File Authentication */
  SDNPDATA_XML_FILECAPABILITY fileAuthentication;

  /* File Append Mode */
  SDNPDATA_XML_FILECAPABILITY fileAppendMode; 

  /* Permissions Support */
  struct {
    TMWTYPES_BOOL ownerReadAllowed;
    TMWTYPES_BOOL ownerWriteAllowed;
    TMWTYPES_BOOL ownerExecuteAllowed;
    TMWTYPES_BOOL groupReadAllowed;
    TMWTYPES_BOOL groupWriteAllowed;
    TMWTYPES_BOOL groupExecuteAllowed;
    TMWTYPES_BOOL worldReadAllowed;
    TMWTYPES_BOOL worldWriteAllowed;
    TMWTYPES_BOOL worldExecuteAllowed;
  }permissionsSupport;

} SDNPDATA_XML_FILECONFIG;

/* database pointListDefinition used in all data type Group description */
typedef enum {
  /* fixed */
  SDNPDATA_XML_POINT_LISTFIXED,
  /* configurable */
  SDNPDATA_XML_POINT_LISTCONFIGURABLE,
  /* other, set other field */
  SDNPDATA_XML_POINT_LISTOTHER
} SDNPDATA_XML_POINTLISTTYPE;

typedef struct {
  /* specify fixed, configurable or other */
  SDNPDATA_XML_POINTLISTTYPE type; 

  /* null terminated string, if type is SDNPDATA_XML_POINT_LISTOTHER */
  TMWTYPES_CHAR other[SDNPXML_MAXSTRINGLENGTH];
} SDNPDATA_XML_POINTLISTVALUE;

/* point list config structure for all data object groups */
typedef struct { 
  SDNPDATA_XML_POINTLISTVALUE binaryInputGroup;
  SDNPDATA_XML_POINTLISTVALUE doubleBitInputGroup;
  SDNPDATA_XML_POINTLISTVALUE binaryOutputGroup;
  SDNPDATA_XML_POINTLISTVALUE counterGroup;
  SDNPDATA_XML_POINTLISTVALUE analogInputGroup;
  SDNPDATA_XML_POINTLISTVALUE analogOutputGroup;
  SDNPDATA_XML_POINTLISTVALUE sequentialFile; 
  SDNPDATA_XML_POINTLISTVALUE prototypeDefinition;
  SDNPDATA_XML_POINTLISTVALUE datasetDefinition;
  SDNPDATA_XML_POINTLISTVALUE octetStringGroup;
  SDNPDATA_XML_POINTLISTVALUE virtualTermGroup;
} SDNPDATA_XML_POINTLISTCONFIG;


/* Per Binary Input point structure */
typedef struct { 
  /* null terminated strings, empty string will not be output */
  TMWTYPES_CHAR   nameState0[SDNPXML_MAXSTRINGLENGTH];
  TMWTYPES_CHAR   nameState1[SDNPXML_MAXSTRINGLENGTH];

  /* null terminated name of point */
  TMWTYPES_CHAR   name[SDNPXML_MAXSTRINGLENGTH];
} SDNPDATA_XML_BININPOINTCONFIG;

/* Per Double Bit Input point structure */
typedef struct { 
  /* null terminated strings, empty string will not be output */
  TMWTYPES_CHAR   nameState0[SDNPXML_MAXSTRINGLENGTH];
  TMWTYPES_CHAR   nameState1[SDNPXML_MAXSTRINGLENGTH];
  TMWTYPES_CHAR   nameState2[SDNPXML_MAXSTRINGLENGTH];
  TMWTYPES_CHAR   nameState3[SDNPXML_MAXSTRINGLENGTH];

  /* null terminated name of point */
  TMWTYPES_CHAR   name[SDNPXML_MAXSTRINGLENGTH];
} SDNPDATA_XML_DBLINPOINTCONFIG; 

/* Per Binary Output point structure */
typedef struct {
  TMWTYPES_DOUBLE minimumPulseWidth;
  TMWTYPES_DOUBLE maximumPulseWidth;

  /* null terminated strings, empty string will not be output */
  TMWTYPES_CHAR   nameState0[SDNPXML_MAXSTRINGLENGTH];
  TMWTYPES_CHAR   nameState1[SDNPXML_MAXSTRINGLENGTH];

  /* The following two are part of dnpData */
  TMWTYPES_UCHAR  control;
  TMWTYPES_UCHAR  status;

  /* null terminated name of point */
  TMWTYPES_CHAR   name[SDNPXML_MAXSTRINGLENGTH];
} SDNPDATA_XML_BINOUTPOINTCONFIG;

/* Per Counter point structure */
typedef struct { 

  TMWTYPES_ULONG counterRollOver;

  /* name of point */
  /* null terminated name of point */
  TMWTYPES_CHAR   name[SDNPXML_MAXSTRINGLENGTH];
} SDNPDATA_XML_CNTRPOINTCONFIG;

/* Per Analog Input point structure */
typedef struct {
  /* In Jan2010 schema, min and max TransmittedValue were changed to 
   * minInt, minFloat, maxInt and maxFloat TransmittedValue
   * In April2016 schema it became a choice between integer or float.
   * set minInt and maxInt to the same value to use the float values.
   */
  TMWTYPES_LONG   minIntTransmittedValue;
  TMWTYPES_LONG   maxIntTransmittedValue;

  TMWTYPES_DOUBLE minFloatTransmittedValue;
  TMWTYPES_DOUBLE maxFloatTransmittedValue;

  TMWTYPES_DOUBLE scaleOffset;  
  TMWTYPES_DOUBLE scaleFactor;  
  TMWTYPES_DOUBLE resolution; 

  /* null terminated string, empty string will not be output */
  TMWTYPES_CHAR   units[SDNPXML_MAXSTRINGLENGTH];  

  /* null terminated name of point */
  TMWTYPES_CHAR   name[SDNPXML_MAXSTRINGLENGTH];
} SDNPDATA_XML_ANLGINPOINTCONFIG;

/* Per Analog Output point structure */
typedef struct {
  TMWTYPES_DOUBLE minTransmittedValue;
  TMWTYPES_DOUBLE maxTransmittedValue;

  TMWTYPES_DOUBLE scaleOffset;  
  TMWTYPES_DOUBLE scaleFactor;  
  TMWTYPES_DOUBLE resolution;  

  /* null terminated string, empty string will not be output */
  TMWTYPES_CHAR   units[SDNPXML_MAXSTRINGLENGTH];  

  /* null terminated name of point */
  TMWTYPES_CHAR   name[SDNPXML_MAXSTRINGLENGTH];
} SDNPDATA_XML_ANLGOUTPOINTCONFIG;

/* Octet String point structure */
typedef struct {
  TMWTYPES_CHAR name[SDNPXML_MAXSTRINGLENGTH];
} SDNPDATA_XML_STRINGPOINTCONFIG;

/* Virtual Terminal point structure */
typedef struct {
  TMWTYPES_CHAR name[SDNPXML_MAXSTRINGLENGTH];
} SDNPDATA_XML_VTERMPOINTCONFIG;


/* Per File "point" structure, the SCL will call sdnpdata_XmlGetPerFileConfig 
 * to get this information till function returns TMWDEFS_FALSE 
 */
typedef struct {

  /* Null terminated string. 
   * If empty string, ie filename[0]==0 this entire file record will not be output 
   */
  TMWTYPES_CHAR filename[SDNPXML_MAXSTRINGLENGTH];

  TMWDEFS_CLASS_MASK eventClass;
  TMWTYPES_BOOL readAuthenticateRequired;
  TMWTYPES_BOOL writeAuthenticateRequired;
  TMWTYPES_BOOL deleteAuthenticateRequired;

  /* Null terminated string. 
   * Empty string will not be output 
   */
  TMWTYPES_CHAR description[SDNPXML_MAXSTRINGLENGTH]; 

} SDNPDATA_XML_PERFILECONFIG;


#ifdef __cplusplus
extern "C" {
#endif
  
  /* function: sdnpdata_deviceProfileFile
   * purpose: Check to see if this filename is to be used for retrieving the device 
   * profile in xml format if so, is this file authentication key OK? 
   * If this returns TMWDEFS_TRUE, but authentication is not OK
   *   the SCL will send back a denied status to the master.
   * if this returns TMWDEFS_FALSE, then the normal sdnpdata_openFile will be called
   * If this file name is for retrieving the Device Profile from the database itself,
   * ie not generated by the SCL at this time, then this should return TMWDEFS_FALSE
   * arguments:  
   *  pHandle - handle to database returned from sdnpdata_init 
   *  pFileName - file name to be checked
   *  authKey - file authentication key sent from master
   *  pAuthKey - return value indicating if this authentication key was valid
   * returns: 
   *  TMWDEFS_TRUE if this is the name for the SCL to generate the Device Profile 
   *   in XML format.
   *  TMWDEFS_FALSE otherwise
   */ 
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_deviceProfileFile(
    void *pHandle, 
    TMWTYPES_CHAR *pFileName,
    TMWTYPES_ULONG authKey,
    TMWTYPES_BOOL *pAuthKeyOK);

  /* function: sdnpdata_XmlGetDeviceConfig
   * purpose: Get the information needed to generate the XML for the 
   *  configuration/deviceConfig portion for the Device Profile XML output
   * arguments:  
   *  pHandle - handle to database returned from sdnpdata_init
   *  pConfig - pointer to the configuration structure to be filled in.
   * returns: 
   *  TMWDEFS_TRUE if successful
   *  TMWDEFS_FALSE otherwise
   */ 
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_XmlGetDeviceConfig(
    void *pHandle, 
    SDNPDATA_XML_DEVICECONFIG *pConfig);

  /* function: sdnpdata_XmlGetSerialConfig
   * purpose: Get the information needed to generate the XML for the 
   *  configuration/serialConfig portion for the Device Profile XML output
   * arguments:  
   *  pHandle - handle to database returned from sdnpdata_init
   *  pConfig - pointer to the configuration structure to be filled in.
   * returns: 
   *  TMWDEFS_TRUE if successful
   *  TMWDEFS_FALSE otherwise
   */ 
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_XmlGetSerialConfig(
    void *pHandle, 
    SDNPDATA_XML_SERIALCONFIG *pConfig);
  
  /* function: sdnpdata_XmlGetNetworkConfig
   * purpose: Get the information needed to generate the XML for the 
   *  configuration/network portion for the Device Profile XML output
   * arguments:  
   *  pHandle - handle to database returned from sdnpdata_init
   *  pConfig - pointer to the configuration structure to be filled in.
   * returns: 
   *  TMWDEFS_TRUE if successful
   *  TMWDEFS_FALSE otherwise
   */ 
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_XmlGetNetworkConfig(
    void *pHandle, 
    SDNPDATA_XML_NETWORKCONFIG *pConfig);
  
  /* function: sdnpdata_XmlGetOutstationConfig
   * purpose: Get the information needed to generate the XML for the 
   *  configuration/outstationConfig portion for the Device Profile XML output
   * arguments:  
   *  pHandle - handle to database returned from sdnpdata_init
   *  pConfig - pointer to the configuration structure to be filled in.
   * returns: 
   *  TMWDEFS_TRUE if successful
   *  TMWDEFS_FALSE otherwise
   */ 
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_XmlGetOutstationConfig(
    void *pHandle, 
    SDNPDATA_XML_OUTSTATIONCONFIG *pConfig);

  /* function: sdnpdata_XmlGetOutstationPerform
   * purpose: Get the information needed to generate the XML for the 
   *  configuration/outstationPerformance portion for the Device Profile XML output
   * arguments:  
   *  pHandle - handle to database returned from sdnpdata_init
   *  pConfig - pointer to the configuration structure to be filled in.
   * returns: 
   *  TMWDEFS_TRUE if successful
   *  TMWDEFS_FALSE otherwise
   */ 
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_XmlGetOutstationPerform(
    void *pHandle, 
    SDNPDATA_XML_OUTSTATIONPERFORM *pConfig);

  /* function: sdnpdata_XmlGetFieldConfig
   * purpose: Get the information needed to generate the XML for the 
   *  configuration/fieldConfig portion for the Device Profile XML output
   * arguments:  
   *  pHandle - handle to database returned from sdnpdata_init
   *  pConfig - pointer to the configuration structure to be filled in.
   * returns: 
   *  TMWDEFS_TRUE if successful
   *  TMWDEFS_FALSE otherwise
   */ 
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_XmlGetFieldConfig(
    void *pHandle, 
    SDNPDATA_XML_FIELDCONFIG *pConfig);

  /* function: sdnpdata_XmlGetSecurityConfig
   * purpose: Get the information needed to generate the XML for the 
   *  configuration/securityConfig portion for the Device Profile XML output
   * arguments:  
   *  pHandle - handle to database returned from sdnpdata_init
   *  pConfig - pointer to the configuration structure to be filled in.
   * returns: 
   *  TMWDEFS_TRUE if successful
   *  TMWDEFS_FALSE otherwise
   */ 
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_XmlGetSecurityConfig(
    void *pHandle, 
    SDNPDATA_XML_SECURITYCONFIG *pConfig);

  /* function: sdnpdata_XmlGetBinOutGroupConfig
   * purpose: Get the information needed to generate the XML for the 
   *  database binaryOutputGroup portion of the Device Profile XML output
   * arguments:  
   *  pHandle - handle to database returned from sdnpdata_init
   *  pConfig - pointer to the configuration structure to be filled in.
   * returns: 
   *  TMWDEFS_TRUE if successful
   *  TMWDEFS_FALSE otherwise
   */ 
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_XmlGetBinOutGroupConfig(
    void *pHandle, 
    SDNPDATA_XML_BINOUTGROUPCONFIG *pConfig);
  
  /* function: sdnpdata_XmlGetCntrGroupConfig
   * purpose: Get the information needed to generate the XML for the 
   *  database counterGroup portion of the Device Profile XML output
   * arguments:  
   *  pHandle - handle to database returned from sdnpdata_init
   *  pConfig - pointer to the configuration structure to be filled in.
   * returns: 
   *  TMWDEFS_TRUE if successful
   *  TMWDEFS_FALSE otherwise
   */ 
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_XmlGetCntrGroupConfig(
    void *pHandle, 
    SDNPDATA_XML_COUNTERGROUPCONFIG *pConfig);

  /* function: sdnpdata_XmlGetAnlgInGroupConfig
   * purpose: Get the information needed to generate the XML for the 
   *  database Analog Input Group portion of the Device Profile XML output
   * arguments:  
   *  pHandle - handle to database returned from sdnpdata_init
   *  pConfig - pointer to the configuration structure to be filled in.
   * returns: 
   *  TMWDEFS_TRUE if successful
   *  TMWDEFS_FALSE otherwise
   */ 
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_XmlGetAnlgInGroupConfig(
    void *pHandle, 
    SDNPDATA_XML_ANLGINGROUPCONFIG *pConfig);
   
  /* function: sdnpdata_XmlGetFileConfig
   * purpose: Get the information needed to generate the XML for the 
   *  database sequentialFileTransfer portion of the Device Profile XML output
   * arguments:  
   *  pHandle - handle to database returned from sdnpdata_init
   *  pConfig - pointer to the configuration structure to be filled in.
   * returns: 
   *  TMWDEFS_TRUE if successful
   *  TMWDEFS_FALSE otherwise
   */ 
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_XmlGetFileConfig(
    void *pHandle, 
    SDNPDATA_XML_FILECONFIG *pConfig); 
 
  /* function: sdnpdata_XmlGetPointListDefinition
   * purpose: Get the information needed to generate the XML for the 
   *  pointListDefinition portion for each object group of the Device Profile 
   *  XML output
   * arguments:  
   *  pHandle - handle to database returned from sdnpdata_init
   *  pConfig - pointer to the configuration structure to be filled in.
   * returns: 
   *  TMWDEFS_TRUE if successful
   *  TMWDEFS_FALSE otherwise
   */ 
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_XmlGetPointListDefinition(
    void *pHandle, 
    SDNPDATA_XML_POINTLISTCONFIG *pConfig); 
  
  /* function: sdnpdata_XmlGetBinInPointConfig
   * purpose: Get the information needed to generate the XML for the 
   *  database sequentialFileTransfer portion of the Device Profile XML output
   * arguments:  
   *  pPoint - handle returned from sdnpdata_binInGetPoint function
   *  pConfig - pointer to the configuration structure to be filled in.
   * returns: 
   *  TMWDEFS_TRUE if successful
   *  TMWDEFS_FALSE otherwise
   */ 
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_XmlGetBinInPointConfig(
    void *pPoint, 
    SDNPDATA_XML_BININPOINTCONFIG *pConfig);

  /* function: sdnpdata_XmlGetDblInPointConfig
   * purpose: Get the information needed to generate the XML for the 
   *  database sequentialFileTransfer portion of the Device Profile XML output
   * arguments:  
   *  pPoint - handle returned from sdnpdata_dblInGetPoint function
   *  pConfig - pointer to the configuration structure to be filled in.
   * returns: 
   *  TMWDEFS_TRUE if successful
   *  TMWDEFS_FALSE otherwise
   */ 
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_XmlGetDblInPointConfig(
    void *pPoint, 
    SDNPDATA_XML_DBLINPOINTCONFIG *pConfig);

  /* function: sdnpdata_XmlGetBinOutPointConfig
   * purpose: Get the information needed to generate the XML for the 
   *  database sequentialFileTransfer portion of the Device Profile XML output
   * arguments:  
   *  pPoint - handle returned from sdnpdata_binOutGetPoint function
   *  pConfig - pointer to the configuration structure to be filled in.
   * returns: 
   *  TMWDEFS_TRUE if successful
   *  TMWDEFS_FALSE otherwise
   */ 
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_XmlGetBinOutPointConfig(
    void *pPoint, 
    SDNPDATA_XML_BINOUTPOINTCONFIG *pConfig);
  
  /* function: sdnpdata_XmlGetCntrPointConfig
   * purpose: Get the information needed to generate the XML for the 
   *  database sequentialFileTransfer portion of the Device Profile XML output
   * arguments:  
   *  pPoint - handle returned from sdnpdata_binCntrGetPoint function
   *  pConfig - pointer to the configuration structure to be filled in.
   * returns: 
   *  TMWDEFS_TRUE if successful
   *  TMWDEFS_FALSE otherwise
   */ 
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_XmlGetCntrPointConfig(
    void *pPoint, 
    SDNPDATA_XML_CNTRPOINTCONFIG *pConfig);

  /* function: sdnpdata_XmlGetAnlgInPointConfig
   * purpose: Get the information needed to generate the XML for the 
   *  database sequentialFileTransfer portion of the Device Profile XML output
   * arguments:  
   *  pPoint - handle returned from sdnpdata_anlgInGetPoint function
   *  pConfig - pointer to the configuration structure to be filled in.
   * returns: 
   *  TMWDEFS_TRUE if successful
   *  TMWDEFS_FALSE otherwise
   */ 
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_XmlGetAnlgInPointConfig(
    void *pPoint, 
    SDNPDATA_XML_ANLGINPOINTCONFIG *pConfig);

  /* function: sdnpdata_XmlGetAnlgOutPointConfig
   * purpose: Get the information needed to generate the XML for the 
   *  database sequentialFileTransfer portion of the Device Profile XML output
   * arguments:  
   *  pPoint - handle returned from sdnpdata_anlgOutGetPoint function
   *  pConfig - pointer to the configuration structure to be filled in.
   * returns: 
   *  TMWDEFS_TRUE if successful
   *  TMWDEFS_FALSE otherwise
   */ 
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_XmlGetAnlgOutPointConfig(
    void *pPoint, 
    SDNPDATA_XML_ANLGOUTPOINTCONFIG *pConfig);
  
  /* function: sdnpdata_XmlGetStringPointConfig
   * purpose: Get the information needed to generate the XML for the 
   *  database sequentialFileTransfer portion of the Device Profile XML output
   * arguments:  
   *  pPoint - handle returned from sdnpdata_strGetPoint function
   *  pConfig - pointer to the configuration structure to be filled in.
   * returns: 
   *  TMWDEFS_TRUE if successful
   *  TMWDEFS_FALSE otherwise
   */ 
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_XmlGetStringPointConfig(
    void *pPoint, 
    SDNPDATA_XML_STRINGPOINTCONFIG *pConfig);

  /* function: sdnpdata_XmlGetVtermPointConfig
   * purpose: Get the information needed to generate the XML for the 
   *  database sequentialFileTransfer portion of the Device Profile XML output
   * arguments:  
   *  pPoint - handle returned from sdnpdata_vtermGetPoint function
   *  pConfig - pointer to the configuration structure to be filled in.
   * returns: 
   *  TMWDEFS_TRUE if successful
   *  TMWDEFS_FALSE otherwise
   */ 
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_XmlGetVtermPointConfig(
    void *pPoint, 
    SDNPDATA_XML_VTERMPOINTCONFIG *pConfig);

  /* function: sdnpdata_XmlGetPerFileConfig
   * purpose: Get the information needed to generate the XML for the 
   *  database sequentialFileTransfer portion of the Device Profile XML output
   * arguments:  
   *  pPoint - handle returned sdnpdata_init function
   *  index - index 0 to n-1 for all files in database 
   *  pConfig - pointer to the configuration structure to be filled in.
   * returns: 
   *  TMWDEFS_TRUE if successful
   *  TMWDEFS_FALSE otherwise
   */ 
  TMWTYPES_BOOL TMWDEFS_GLOBAL sdnpdata_XmlGetPerFileConfig(
    void *pHandle,
    TMWTYPES_ULONG index, 
    SDNPDATA_XML_PERFILECONFIG *pConfig);

  /* function: sdnpdata_binOutGetSupCtrl
   * purpose: Determine what device profile control operations are supported
   *  for this particular point.
   *  NOTE: this is used only by sdnpxml/sdnpxml2.c if SDNPDATA_SUPPORT_XMLxx is TRUE
   *  code to generate the configuration file or Device Profile for this device.
   * arguments:
   *  pPoint - handle to data point returned from sdnpdata_binOutGetPoint function.
   * returns:
   *  bitmask indicating what operations are supported.
   *   SDNPDATA_XML_SUP_SELOP
   *   SDNPDATA_XML_SUP_DIROP
   *   SDNPDATA_XML_SUP_DIRNOACK
   *   SDNPDATA_XML_SUP_MORE_ONE
   *   SDNPDATA_XML_SUP_SEL_CANCEL
   */
  SDNPDATA_XML_SUP TMWDEFS_GLOBAL sdnpdata_binOutGetSupCtrl(
    void *pPoint);

  /* function: sdnpdata_anlgOutGetSupCtrl
   * purpose: Determine what device profile control operations are supported
   *  for this particular point.
   *  NOTE: this is used only by sdnpxml/sdnpxml2.c if SDNPDATA_SUPPORT_XMLxx is TRUE
   *  code to generate the configuration file or Device Profile for this device.
   * arguments:
   *  pPoint - handle to data point returned from sdnpdata_anlgOutGetPoint function.
   * returns:
   *  bitmask indicating what operations are supported.
   *   SDNPDATA_XML_SUP_SELOP
   *   SDNPDATA_XML_SUP_DIROP
   *   SDNPDATA_XML_SUP_DIRNOACK
   */
  SDNPDATA_XML_SUP TMWDEFS_GLOBAL sdnpdata_anlgOutGetSupCtrl(
    void *pPoint);

#ifdef __cplusplus
}
#endif

#endif /* SDNPDATA_SUPPORT_XML2 */

#endif /* SDNPDATA_DEFINED */
