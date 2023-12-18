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

/* file: tmwtypes.h
 * description: Triangle MicroWorks Source Code definitions.
 */
#ifndef TMWTYPES_DEFINED
#define TMWTYPES_DEFINED

#ifndef _WIN32
/* If this file with standard integer types is available use it */
#include <stdint.h>
#endif

#include "tmwscl/utils/tmwdefs.h"
#include "tmwscl/utils/tmwcnfg.h"

/* Type Definitions */

#ifdef __cplusplus
typedef bool           TMWTYPES_BOOL;   /*  false to  true                 */
#else
typedef unsigned char  TMWTYPES_BOOL;   /*  TMWDEFS_FALSE to  TMWDEFS_TRUE */
#endif

typedef unsigned char  TMWTYPES_BYTE;   /*              0 to           255 */
typedef          char  TMWTYPES_CHAR;   /*           -128 to           127 */
typedef unsigned char  TMWTYPES_UCHAR;  /*              0 to           255 */

/* short must be 16 bits and long must be 32 bits. 
 * Use stdint.h types if they are available.
 */
#ifdef _WIN32
typedef          short TMWTYPES_SHORT;  /*        -32,768 to        32,767 */
typedef unsigned short TMWTYPES_USHORT; /*              0 to        65,535 */
typedef          int TMWTYPES_LONG;    /* -2,147,483,648 to 2,147,483,647 */
typedef unsigned int TMWTYPES_ULONG;   /*              0 to 4,294,967,295 */

/* Note: The following 2 types are not used by the SCL and can be removed  */
/*       if there is no compiler support for 64-bit types.                 */
typedef      long long TMWTYPES_INT64;  /* -9,223,372,036,854,775,808 to   */
                                        /* 9,223,372,036,854,775,807       */
typedef unsigned long long
                       TMWTYPES_UINT64; /* 0 to 18,446,744,073,709,551,615 */
#else
typedef        int16_t TMWTYPES_SHORT;  /*        -32,768 to        32,767 */
typedef       uint16_t TMWTYPES_USHORT; /*              0 to        65,535 */
typedef        int32_t TMWTYPES_LONG;   /* -2,147,483,648 to 2,147,483,647 */
typedef       uint32_t TMWTYPES_ULONG;  /*              0 to 4,294,967,295 */

/* Note: The following 2 types are not used by the SCL and can be removed  */
/*       if there is no compiler support for 64-bit types.                 */
typedef        int64_t TMWTYPES_INT64;  /* -9,223,372,036,854,775,808 to   */
                                        /* 9,223,372,036,854,775,807       */
typedef       uint64_t TMWTYPES_UINT64; /* 0 to 18,446,744,073,709,551,615 */
#endif

typedef          int   TMWTYPES_INT;    /* machine dependant               */
typedef unsigned int   TMWTYPES_UINT;   /* machine dependant               */
typedef          float TMWTYPES_SFLOAT; /*  -3.4 * 10(38) to +3.4 * 10(38) */
                                        /*     32-bit short floating point */
                                        /*     number -- IEEE Standard 754 */
                                        /*       fraction = UI23[1..23]    */
                                        /*       exponent = UI8 [24..31]   */
                                        /*       sign     = BS1[32]        */
typedef         double TMWTYPES_DOUBLE; /* 2.2250738585072014 10(-308) to  */
                                        /*     1.7976931348623158 10(+308) */
                                        /*  64-bit double precision number */
                                        /*     number -- IEEE Standard 754 */
                                        /*       fraction = UI52[1..52]    */
                                        /*       exponent = UI11 [53..63]  */
                                        /*       sign     = BS1[64]        */

/* This enumeration type is used to identify the protocol and mode of
 * operation (master or slave) when multiple Triangle MicroWorks,
 * Inc. Source Code Libraries are combined.  A specific example of such
 * a combination is the Triangle MicroWorks, Inc. Gateway Source Code
 * Library and Executable.  This product uses multiple Source Code
 * Libraries to create a data concentrator and/or protocol translator.
 */
typedef enum TMWTYPES_PROTOCOL_ENUM
{
  TMWTYPES_PROTOCOL_101 = 0, 
  TMWTYPES_PROTOCOL_102, 
  TMWTYPES_PROTOCOL_103, 
  TMWTYPES_PROTOCOL_104, 
  TMWTYPES_PROTOCOL_DNP,
  TMWTYPES_PROTOCOL_MB,
  TMWTYPES_PROTOCOL_I61850,
  TMWTYPES_PROTOCOL_NUM_PROTOCOLS
} TMWTYPES_PROTOCOL;

/* Specify whether this is a master or slave
 */
typedef enum TMWTYPES_SESSION_TYPE_ENUM
{
  TMWTYPES_SESSION_TYPE_MASTER = 0,
  TMWTYPES_SESSION_TYPE_SLAVE,
  TMWTYPES_SESSION_TYPE_MONITOR,
  TMWTYPES_SESSION_TYPE_NUM_TYPES
} TMWTYPES_SESSION_TYPE;

/* The following definitions are used to provide easy representation of 
 * time in units of milliseconds. Using TMWTYPES_ULONG to store milliseconds
 * allows a range of approximately 48 days.
 */
typedef TMWTYPES_ULONG TMWTYPES_MILLISECONDS;

/* This structure is used primarily by DNP Source Code Libraries to 
 * specify the number of milliseconds since January 1, 1970.  Target 
 * Application specific functions must convert between native date/time 
 * structures and this structure.  Utility conversion routines are 
 * provided in TMWdtime.c and DNPdtime.c.
 */
typedef struct TMWTypesMsSince70
{
  TMWTYPES_ULONG  mostSignificant;
  TMWTYPES_USHORT leastSignificant;
} TMWTYPES_MS_SINCE_70;


#if TMW_PRIVATE
#include "tmwscl/utils/tmwdtime.h"
#endif

/* Define data structure used to hold analog data point values 
 * CURRENTLY ONLY USED BY DNP 
 */
typedef enum TMWTYPES_ANALOG_TYPE_ENUM
{
  TMWTYPES_ANALOG_TYPE_SHORT = 0,
  TMWTYPES_ANALOG_TYPE_USHORT,
  TMWTYPES_ANALOG_TYPE_LONG,
  TMWTYPES_ANALOG_TYPE_ULONG,
  TMWTYPES_ANALOG_TYPE_CHAR,
  TMWTYPES_ANALOG_TYPE_UCHAR
#if TMWCNFG_SUPPORT_FLOAT
  ,TMWTYPES_ANALOG_TYPE_SFLOAT
  ,TMWTYPES_ANALOG_TYPE_SCALED
#endif
#if TMWCNFG_SUPPORT_DOUBLE
  ,TMWTYPES_ANALOG_TYPE_DOUBLE
  ,TMWTYPES_ANALOG_TYPE_DSCALED
#endif
#if TMW_PRIVATE
  ,TMWTYPES_ANALOG_TYPE_STRING
  ,TMWTYPES_ANALOG_TYPE_TIME
#endif
} TMWTYPES_ANALOG_TYPE;

/* This allows for a scaled integer representation of floating point value
 * as well as the floating point value itself. This also allows the database 
 * to determine how the floating point value would be rounded if an integer
 * value is to be sent in a response. fval will be used if a floating point
 * value is required, lval will be used if an integer value is required.
 */
typedef struct TMWScaledFloat {
  TMWTYPES_SFLOAT fval;
  TMWTYPES_LONG   lval;
} TMWTYPES_SCALED_FLOAT;

/* This allows for a scaled integer representation of floating point value
 * as well as the floating point value itself. This also allows the database 
 * to determine how the floating point value would be rounded if an integer
 * value is to be sent in a response. dval will be used if a floating point
 * value is required, lval will be used if an integer value is required.
 */
typedef struct TMWScaledDouble {
  TMWTYPES_DOUBLE dval;
  TMWTYPES_LONG   lval;
} TMWTYPES_SCALED_DOUBLE;

typedef struct TMWAnalogValue {
  TMWTYPES_ANALOG_TYPE type;
  union _valueUnion
  {
    TMWTYPES_SHORT  sval;
    TMWTYPES_USHORT usval;
    TMWTYPES_LONG   lval;
    TMWTYPES_ULONG  ulval;
    TMWTYPES_CHAR   cval;
    TMWTYPES_UCHAR  ucval;
#if TMWCNFG_SUPPORT_FLOAT 
    TMWTYPES_SFLOAT fval;
    TMWTYPES_SCALED_FLOAT scaled;
#endif
#if TMWCNFG_SUPPORT_DOUBLE
    TMWTYPES_DOUBLE dval;
    TMWTYPES_SCALED_DOUBLE dscaled;
#endif
#if TMW_PRIVATE
    TMWTYPES_UCHAR *pString;
    TMWDTIME timeVal;
#endif
  } value;

#if TMW_PRIVATE
  /*
   * The following members are used by an external application
   * to store overflow and underflow state for a type conversion
   */
  TMWTYPES_BOOL m_bOverFlow;
  TMWTYPES_BOOL m_bUnderFlow;
#endif /* TMW_PRIVATE */

} TMWTYPES_ANALOG_VALUE;

/* Define a generic callback function used throughout TMW code
 */
typedef void (*TMWTYPES_CALLBACK_FUNC)(void *pCallbackParam);

#endif /* TMWTYPES_DEFINED */
