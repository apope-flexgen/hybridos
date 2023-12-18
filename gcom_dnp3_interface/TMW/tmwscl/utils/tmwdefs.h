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

/* file: tmwdefs.h
 * description: Triangle MicroWorks Source Code definitions.
 */
#ifndef TMWDEFS_DEFINED
#define TMWDEFS_DEFINED


/* Define function modifiers
 *
 * Note that these definitions are default values
 * in order to override these definitions specify them
 * on the compiler command line.  They are specific
 * to the target platform.
 */

#if !defined(TMWDEFS_LOCAL)
  #define TMWDEFS_LOCAL
#endif

#if !defined(TMWDEFS_GLOBAL)
  #define TMWDEFS_GLOBAL
#endif

#if !defined(TMWDEFS_CALLBACK)
  #define TMWDEFS_CALLBACK
#endif

/* Lock for critical sections */
#if !defined(TMWDEFS_RESOURCE_LOCK)
  #define TMWDEFS_RESOURCE_LOCK  void *
#endif
 

#if defined(TMW_SCL_DLL) 
  #if defined(TMWSCL_EXPORTS)
    #define TMWDEFS_SCL_API __declspec(dllexport)
  #else
    #define TMWDEFS_SCL_API __declspec(dllimport)
  #endif 
#else
  #define TMWDEFS_SCL_API
#endif

/* Define NULL */
#ifdef __cplusplus
  #define TMWDEFS_NULL (0)
#else
  #define TMWDEFS_NULL ((void *)0)
#endif

/* True or False */
#define TMWDEFS_TRUE  (0==0)
#define TMWDEFS_FALSE (0==1)

/* Macro to convert to TMWTYPES_BOOL properly if bit is set */
#define TMWDEFS_TOBOOL(mask, bit) ((TMWTYPES_BOOL)(((mask) & (bit))? TMWDEFS_TRUE : TMWDEFS_FALSE))

/* Calculate the max or min of two numbers */
#define TMWDEFS_MAX(x,y) ((x)>=(y)?(x):(y))
#define TMWDEFS_MIN(x,y) ((x)<=(y)?(x):(y))

/* Calculate the length of an array */
#define TMWDEFS_ARRAY_LENGTH(x) (sizeof(x)/sizeof(x[0]))

/* The following strings are used by all TMW products when showing 
 * product information in help, about, or diagnostic displays.   
 */
#define TMWDEFS_COMPANY         "Triangle MicroWorks, Inc."
#define TMWDEFS_COMPANY_1       "Raleigh, North Carolina  USA"     
#define TMWDEFS_COMPANY_2       "Voice:  +1 (919) 870-5101"      
#define TMWDEFS_COMPANY_3       "Fax:  +1 (919) 870-6692"        
#define TMWDEFS_COMPANY_4       "www.TriangleMicroWorks.com"
#define TMWDEFS_COMPANY_ABBREV  "TMW"

/* Ranges for various types defined above */
#define TMWDEFS_BOOL_MIN    TMWDEFS_FALSE
#define TMWDEFS_BOOL_MAX    TMWDEFS_TRUE

#define TMWDEFS_BYTE_MIN    ((TMWTYPES_BYTE)(  0))
#define TMWDEFS_BYTE_MAX    ((TMWTYPES_BYTE)(255))

#define TMWDEFS_CHAR_MIN    ((TMWTYPES_CHAR)(-127-1))
#define TMWDEFS_CHAR_MAX    ((TMWTYPES_CHAR)( 127))

#define TMWDEFS_UCHAR_MIN   ((TMWTYPES_UCHAR)(  0))
#define TMWDEFS_UCHAR_MAX   ((TMWTYPES_UCHAR)(255))

#define TMWDEFS_SHORT_MIN   ((TMWTYPES_SHORT)(-32767L-1L))
#define TMWDEFS_SHORT_MAX   ((TMWTYPES_SHORT)( 32767L))

#define TMWDEFS_USHORT_MIN  ((TMWTYPES_USHORT)(    0))
#define TMWDEFS_USHORT_MAX  ((TMWTYPES_USHORT)(65535UL))

#define TMWDEFS_LONG_MIN    ((TMWTYPES_LONG)(-2147483647L-1L))
#define TMWDEFS_LONG_MAX    ((TMWTYPES_LONG)( 2147483647L))

#define TMWDEFS_ULONG_MIN   ((TMWTYPES_ULONG)(         0))
#define TMWDEFS_ULONG_MAX   ((TMWTYPES_ULONG)(4294967295UL))

#define TMWDEFS_SFLOAT_SMALLEST ((TMWTYPES_SFLOAT)( 1.175494351E-38))
#define TMWDEFS_SFLOAT_MIN      ((TMWTYPES_SFLOAT)(-3.402823466E+38))
#define TMWDEFS_SFLOAT_MAX      ((TMWTYPES_SFLOAT)( 3.402823466E+38))

#define TMWDEFS_DOUBLE_SMALLEST ((TMWTYPES_DOUBLE)( 2.2250738585072014E-308))
#define TMWDEFS_DOUBLE_MIN      ((TMWTYPES_DOUBLE)(-1.7976931348623158E+308))
#define TMWDEFS_DOUBLE_MAX      ((TMWTYPES_DOUBLE)( 1.7976931348623158E+308))

#define TMWDEFS_SECONDS(s)  ((s)*1000UL)
#define TMWDEFS_MINUTES(m)  ((m)*60UL*1000UL)
#define TMWDEFS_HOURS(h)    ((h)*60UL*60UL*1000UL)
#define TMWDEFS_DAYS(d)     ((d)*24UL*60UL*60UL*1000UL)

/* The following definitions are used to indicate the "class" of either
 * data types, or data points.  Note that "class" has different meanings
 * for different protocols, and not all of the following definitions are
 * used for every protocol.
 *
 * For example, in DNP 3, the following are used by the API to assign an
 * "event" class to individual points.  Only change event data exists in
 * "event" classes.  It is unusual, but there is nothing in DNP 3 that
 * prevent an event data point from existing in more than one event class.
 * For this reason, the following definitions use separate bits rather
 * than enumeration; the values can be or'd together to combine classes.
 *
 * In IEC 870-5, class refers to data types, not points; i.e., all points
 * of a given data type must be in the same class.  Only class 1 and class
 * 2 are used, and a data type may exists in either one, but not both.
 */
typedef unsigned char TMWDEFS_CLASS_MASK;

#define TMWDEFS_CLASS_MASK_NONE          0x00
#define TMWDEFS_CLASS_MASK_ONE           0x01
#define TMWDEFS_CLASS_MASK_TWO           0x02
#define TMWDEFS_CLASS_MASK_THREE         0x04

/* This means all three event classes */
#define TMWDEFS_CLASS_MASK_ALL           0x07

/* There are 3 event classes */
#define TMWDEFS_CLASS_MAX                   3

/* The following definition applies only to DNP. It indicates that a point 
 * should not be included in a class 0 static data response even though 
 * other points in that data type are included. This point can still be read
 * using the object group for that specific data type (ie not 60)
 */
#define TMWDEFS_CLASS_MASK_NOTCLASS0     0x08

/* The following definition is used to indicate that there is currently
 * no associated input point assigned for an output point in the database 
 */
#define TMWDEFS_NO_ASSOC_POINT        0xffffffffL

/* Define group masks */
typedef unsigned long TMWDEFS_GROUP_MASK;

#define TMWDEFS_GROUP_MASK_GENERAL    0x00000001L
#define TMWDEFS_GROUP_MASK_1          0x00000002L
#define TMWDEFS_GROUP_MASK_2          0x00000004L
#define TMWDEFS_GROUP_MASK_3          0x00000008L
#define TMWDEFS_GROUP_MASK_4          0x00000010L
#define TMWDEFS_GROUP_MASK_5          0x00000020L
#define TMWDEFS_GROUP_MASK_6          0x00000040L
#define TMWDEFS_GROUP_MASK_7          0x00000080L
#define TMWDEFS_GROUP_MASK_8          0x00000100L
#define TMWDEFS_GROUP_MASK_9          0x00000200L
#define TMWDEFS_GROUP_MASK_10         0x00000400L
#define TMWDEFS_GROUP_MASK_11         0x00000800L
#define TMWDEFS_GROUP_MASK_12         0x00001000L
#define TMWDEFS_GROUP_MASK_13         0x00002000L
#define TMWDEFS_GROUP_MASK_14         0x00004000L
#define TMWDEFS_GROUP_MASK_15         0x00008000L
#define TMWDEFS_GROUP_MASK_16         0x00010000L

/* Define group masks to be used for reserved or private use QOI values 
 * Just support 5 for now, to leave bits for future use.
 */
#define TMWDEFS_GROUP_MASK_PRIVATE_1  0x00020000L
#define TMWDEFS_GROUP_MASK_PRIVATE_2  0x00040000L
#define TMWDEFS_GROUP_MASK_PRIVATE_3  0x00080000L
#define TMWDEFS_GROUP_MASK_PRIVATE_4  0x00100000L
#define TMWDEFS_GROUP_MASK_PRIVATE_5  0x00200000L 
 
/* Define "group masks" for cyclic and background data */
#define TMWDEFS_GROUP_MASK_CYCLIC     0x10000000L
#define TMWDEFS_GROUP_MASK_BACKGROUND 0x20000000L
#define TMWDEFS_GROUP_MASK_ANY        0xffffffffL

/* Define for Single Point Information value */
#define TMWDEFS_SPI_MASK (0x1)

/* Define enum for Double Point Information values */
typedef enum TMWDEFS_DPI_TYPE_ENUM 
{
  TMWDEFS_DPI_INTERMEDIATE,
  TMWDEFS_DPI_OFF,
  TMWDEFS_DPI_ON,
  TMWDEFS_DPI_INDETERMINATE
} TMWDEFS_DPI_TYPE;

#define TMWDEFS_DPI_MASK  (0x3)

/* For protocols that support breaking an application layer message into
 * multiple link layer frames (e.g., DNP3), this enumeration is used to
 * specify the desired operation for a specific communication session.
 */
typedef enum TMWDEFS_LINKCNFM_ENUM
{
  TMWDEFS_LINKCNFM_NEVER,         /* never, not for any frame              */
  TMWDEFS_LINKCNFM_SOMETIMES,     /* only for multiframe message fragments */
  TMWDEFS_LINKCNFM_ALWAYS         /* always, for all frames                */
} TMWDEFS_LINKCNFM;

/* For protocols that support either balanced or unbalanced operation 
 * (e.g., IEC 60870-5 protocols), this enumeration is used to specify the 
 * desired mode for a specific communication channel.
 */
typedef enum TMWDEFS_LINK_MODE_ENUM
{
  TMWDEFS_LINK_MODE_BALANCED,     /* link uses balanced transmission       */
  TMWDEFS_LINK_MODE_UNBALANCED    /* link uses unbalanced transmission     */
} TMWDEFS_LINK_MODE;

typedef enum TMWDEFS_EVENT_MODE_ENUM
{
  TMWDEFS_EVENT_MODE_SOE,         /* Sequence of Events, return all events */
  TMWDEFS_EVENT_MODE_MOST_RECENT, /* Most Recent, only most recent event   */
  TMWDEFS_EVENT_MODE_CURRENT,     /* For DNP Analog Inputs only, current value   */
  TMWDEFS_EVENT_MODE_PER_POINT    /* For DNP only, allow configuration per point */
} TMWDEFS_EVENT_MODE;

/***************************************************************************/
/* The following values are passed to initialization functions to indicate */
/* a reason, or "cause-of-initialization."                                 */
/***************************************************************************/

typedef enum TMWDEFS_COI_ENUM
{
  TMWDEFS_COI_POWER_ON,     /* Initialization is occurring because of a    */
                            /* power cycle, or a "cold restart" command    */
                            /* requested via the protocol.  All operations */
                            /* should be initialized, including database   */
                            /* initialization or any dynamic memory        */
                            /* allocation.                                 */
  TMWDEFS_COI_WARM_RESTART  /* Initialization is occurring where,          */
                            /* typically, only the protocol is to be       */
                            /* re-initialized.  A typical originator of    */
                            /* this kind of restart is a remote request    */
                            /* via the protocol.  For this kind of         */
                            /* restart, not all operations need to be      */
                            /* initialization; for example, the database   */
                            /* may not need to be re-initialized, and      */
                            /* dynamic memory may not need to be           */
                            /* re-allocated.                               */
} TMWDEFS_COI;


/* This definition allows an unlimited number of buffers of a certain type */
/* to be allocated.                                                        */
#define TMWDEFS_NO_LIMIT 0U

/* Used to return the reason a point changed.
 */
typedef enum TMWDefsChangeReason {
  TMWDEFS_CHANGE_NONE,
  TMWDEFS_CHANGE_LOCAL_OP,
  TMWDEFS_CHANGE_REMOTE_OP,
  TMWDEFS_CHANGE_SPONTANEOUS
} TMWDEFS_CHANGE_REASON;

/* Used to report status of an operation
 */
typedef enum TMWDefsCommandState {
  TMWDEFS_CMD_STATE_IDLE,
  TMWDEFS_CMD_STATE_SELECTING,
  TMWDEFS_CMD_STATE_EXECUTING,
  TMWDEFS_CMD_STATE_MONITORING
} TMWDEFS_COMMAND_STATE;

typedef enum TMWDefsCommandStatus {
  TMWDEFS_CMD_STAT_SELECTING,
  TMWDEFS_CMD_STAT_EXECUTING,
  TMWDEFS_CMD_STAT_MONITORING,
  TMWDEFS_CMD_STAT_FAILED,
  TMWDEFS_CMD_STAT_SUCCESS
} TMWDEFS_COMMAND_STATUS;

/* Possible modes for clock synchronization */
typedef enum TMWDefsClockSyncMode {
  TMWDEFS_CLOCK_SYNC_DELAY_ACQ,    /* Perform delay acquisition followed by load delay
                                      followed by clock sync */
  TMWDEFS_CLOCK_SYNC_LOAD_DELAY,   /* Perform load delay followed by clock sync */
  TMWDEFS_CLOCK_SYNC_ONLY          /* Perform clock synchronization only */
} TMWDEFS_CLOCK_SYNC_MODE;

/* Possible time formats */
typedef enum TMWDefsTimeFormat {
  TMWDEFS_TIME_FORMAT_NONE,
  TMWDEFS_TIME_FORMAT_24,
  TMWDEFS_TIME_FORMAT_32,
  TMWDEFS_TIME_FORMAT_40,
  TMWDEFS_TIME_FORMAT_56,
  TMWDEFS_TIME_FORMAT_XML,
  TMWDEFS_TIME_FORMAT_LOG,
  TMWDEFS_TIME_FORMAT_UNKNOWN     /* Only used in 101/104 slave configuration */
} TMWDEFS_TIME_FORMAT;

/* The following are reason codes that should be provided by the I/O target 
 * implementation when it calls the channel callback function
 */
typedef enum TMWDefsTargOpenCloseReason{
  TMWDEFS_TARG_OC_SUCCESS,        /* connection opened successfully             */
  TMWDEFS_TARG_OC_FAILURE,        /* connection closed because of a             */
                                  /* communication failure, including           */
                                  /* connection closed by remote end            */
  TMWDEFS_TARG_OC_NEW_CONNECTION, /* connection closed because a new connection */
                                  /* request was received                       */
  TMWDEFS_TARG_OC_LOCAL_CLOSED,   /* connection closed because local end        */
                                  /* requested it                               */
  TMWDEFS_TARG_OC_REMOTE_CLOSED,  /* connection closed because remote end       */
                                  /* requested it                               */
  TMWDEFS_TARG_OC_RESET           /* connection closed because it has been      */
                                  /* reset locally                              */
} TMWDEFS_TARG_OC_REASON;

/* Define this for backward compatibility */
#define TMWDEFS_TARG_OC_SUCCES TMWDEFS_TARG_OC_SUCCESS

#endif /* TMWDEFS_DEFINED */
