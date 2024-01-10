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

/* file: dnpdata.h
 * description: This file defines the interface between the TMW DNP3 SCL and 
 *  the target database.
 */
#ifndef DNPDATA_DEFINED
#define DNPDATA_DEFINED

#include "tmwscl/utils/tmwdefs.h"
#include "tmwscl/utils/tmwtypes.h"
#include "tmwscl/utils/tmwdtime.h"
#include "tmwscl/utils/tmwcrypto.h"
#include "tmwscl/dnp/dnpdefs.h"
#include "tmwscl/dnp/dnpcnfg.h"
#include "tmwscl/dnp/dnpauth.h"

/* Define constants */
 
/* These values are to indicate the type of data stored in a union. 
 * These interface values are not transmitted in a DNP message
 */
typedef enum {
  DNPDATA_VALUE_STRPTR,
  DNPDATA_VALUE_STRARRAY,
  DNPDATA_VALUE_UINT32,
  DNPDATA_VALUE_INT32,
  DNPDATA_VALUE_SFLT,
  DNPDATA_VALUE_DOUBLE,
  DNPDATA_VALUE_TIME
} DNPDATA_VALUE_TYPE;

/* Data Set element interface structure used for passing data set element 
 * values between target layer and SCL 
 */
typedef struct {
  
  /* Indicates which field in the union contains the data */
  DNPDATA_VALUE_TYPE type;

  /* Length of the data in the union. For strings this is obvious. For other
   * types such as DNPDATA_VALUE_UINT32 the length could be 1, 2 or 4 bytes indicating
   * the length of the data that should be sent in a message. 
   */
  TMWTYPES_UCHAR     length;

  /* Union containing the different data types */
  union {
    TMWTYPES_UCHAR    *pStrValue; 
    TMWTYPES_UCHAR     strValue[DNPCNFG_MAX_STRING_ARRAY];
    TMWTYPES_ULONG     uint32Value;
    TMWTYPES_LONG      int32Value;
    TMWTYPES_SFLOAT    sfltValue;
    TMWTYPES_DOUBLE    doubleValue;
    TMWDTIME           timeValue;
  } value;
} DNPDATA_DATASET_VALUE;

/* Data Set Descriptor element interface structure used for passing data set 
 * descriptor elements between target layer and SCL 
 */
typedef struct {
  /*
   * Descriptor Element length in Tech Bulletin 2004-004b is the length of the 
   * ancillary value + 3. It is not a field in this structure since 
   * DNPDATA_DATASET_VALUE contains the length of the ancillary value.
   */

  /* Descriptor element type from specification, such as ID, name, UUID, data element etc. */ 
  DNPDEFS_DATASET_DESCR_CODE   descrElemType;

  /* Data type from specification, such as integer, floating point, string etc */
  DNPDEFS_DATASET_TYPE_CODE    dataTypeCode;

  /* Maximum length of corresponding data in data set */
  TMWTYPES_UCHAR               maxDataLength;
  
  /* Completes the description, depends on descrElemType type */
  DNPDATA_DATASET_VALUE        ancillaryValue;
} DNPDATA_DATASET_DESCR_ELEM;

/* Data set descriptor Point Index Attribute structure used for passing data set 
 * descriptor Point Attribute Index elements (Object Group 86, variation 3) 
 * between target layer and SCL 
 */
typedef struct {
  /* The length of the point type and point index is present
   * in this structure. The required length for the point index 
   * will be determined before it is transmitted, This storage
   * location is large enough to hold a ULONG 
   */
  TMWTYPES_UCHAR               pointType;
  TMWTYPES_ULONG               pointIndex;
} DNPDATA_DATASET_DESCR_INDEX;



/* The following types are used to support Object Group 0 Device Attributes */
/* Device Attribute interface structure */
typedef struct {
  /* Length of the data in the union. */
  TMWTYPES_UCHAR              length;

  /* Indicates which field in the union contains the data */
  DNPDEFS_ATTRIBUTE_DATA_TYPE type;

  /* Union containing the different data types */
  union { 
    TMWTYPES_UCHAR     *pStrValue;    /* DNPDEFS_ATTRIBUTE_TYPE_VSTR  
                                         DNPDEFS_ATTRIBUTE_TYPE_OSTR 
                                         DNPDEFS_ATTRIBUTE_TYPE_BSTR */ 
    TMWTYPES_ULONG      uintValue;    /* DNPDEFS_ATTRIBUTE_TYPE_UINT */
    TMWTYPES_LONG       intValue;     /* DNPDEFS_ATTRIBUTE_TYPE_INT  */
    TMWDTIME            timeValue;    /* DNPDEFS_ATTRIBUTE_TYPE_DNP3TIME */
    union {
      TMWTYPES_SFLOAT   sfltValue;    /* DNPDEFS_ATTRIBUTE_TYPE_FLT length == 4*/
      TMWTYPES_DOUBLE   doubleValue;  /* DNPDEFS_ATTRIBUTE_TYPE_FLT length == 8*/
    } fltValue;
  } value;
} DNPDATA_ATTRIBUTE_VALUE;

/* Structure for DNP Secure Authentication Session Key */
typedef struct dnpauth_key {
  TMWTYPES_UCHAR value[DNPAUTH_MAX_KEY_LENGTH];
  TMWTYPES_UCHAR length;
} DNPDATA_AUTH_KEY;

/*                          Table 4-11 Freezing schedule interpretation
   |---------------------------------------------------------------------------------------------------|
   | Time-date field | Interval field |                Freeze timing                                   |
   |---------------------------------------------------------------------------------------------------|
   | zero            | zero           | Freeze once immediately.                                       |
   |---------------------------------------------------------------------------------------------------|
   | non-zero        | zero           | Freeze once at the specified time.                             |
   |---------------------------------------------------------------------------------------------------|
   |                 |                | Periodically freeze at intervals relative to the beginning of  |
   | zero            | non-zero       | of the current hour. Use the time interval from the interval   |
   |                 |                | field. Coninue freezing forever or until a new function code   |
   |                 |                | FREEZE_AT_TIME(_NR) is received.                               |
   |---------------------------------------------------------------------------------------------------|
   |                 |                | Periodically freeze at intervals relative to the time and date |
   | non-zero        | non-zero       | in the time-date field. Use the time interval from the interval|
   |                 |                | field. Coninue freezing forever or until a new function code   |
   |                 |                | FREEZE_AT_TIME(_NR) is received.                               |
   |---------------------------------------------------------------------------------------------------|

   NOTE - To cancel a freeze request, set the time-date field to all ones

   The follow enum is used to enumerate the type of value passed in the time-date field of freeze at time requests.
*/
typedef enum {
  DNPDATA_FREEZE_TIME_DATE_FIELD_ZERO,
  DNPDATA_FREEZE_TIME_DATE_FIELD_NON_ZERO,
  DNPDATA_FREEZE_TIME_DATE_FIELD_CANCEL
} DNPDATA_FREEZE_TIME_DATE_FIELD; 

#endif /* SDNPDATA_DEFINED */
