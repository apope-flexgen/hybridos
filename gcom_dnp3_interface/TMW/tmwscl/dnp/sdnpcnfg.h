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

/* file: sdnpcnfg.h
 * description:   slave DNP configuration definitions
 */
#ifndef SDNPCNFG_DEFINED
#define SDNPCNFG_DEFINED

#include "tmwscl/utils/tmwcnfg.h"
#include "tmwscl/dnp/dnpcnfg.h"

  
/* All *_NUMALLOC_* defines are used only to limit the number of structures
 * of that type that can be allocated. A value of TMWDEFS_NO_LIMIT (the default)
 * means there is no limit. To use static or only at startup memory configuration
 * TMWDEFS_NO_LIMIT is not possible. These DO NOT have to depend on the 
 * TMWCNFG_xxx defines, but can be changed here to set desired values.
 */

/* Specify the maximum number of Slave DNP sessions that can be open at 
 * a given time.
 */
#define SDNPCNFG_NUMALLOC_SESNS              TMWCNFG_MAX_SESSIONS

/* Specify the number of temporary ASDU buffers that can be allocated.
 * currently the TMW SCL needs one per SDNP session.
 */
#define SDNPCNFG_NUMALLOC_ASDU_BUFFERS       TMWCNFG_MAX_SESSIONS

/* For each event type specify the maximum number of events that can
 * be queued at once. This number specifies the maximum number of events
 * per data type for the entire system so it should generally be set to
 * the maximum number per session times the number of sessions.
 */
#define SDNPCNFG_NUMALLOC_OBJECT2_EVENTS     TMWCNFG_MAX_EVENTS
#define SDNPCNFG_NUMALLOC_OBJECT4_EVENTS     TMWCNFG_MAX_EVENTS
#define SDNPCNFG_NUMALLOC_OBJECT11_EVENTS    TMWCNFG_MAX_EVENTS
#define SDNPCNFG_NUMALLOC_OBJECT13_EVENTS    TMWCNFG_MAX_EVENTS
#define SDNPCNFG_NUMALLOC_OBJECT22_EVENTS    TMWCNFG_MAX_EVENTS
#define SDNPCNFG_NUMALLOC_OBJECT23_EVENTS    TMWCNFG_MAX_EVENTS
#define SDNPCNFG_NUMALLOC_OBJECT32_EVENTS    TMWCNFG_MAX_EVENTS
#define SDNPCNFG_NUMALLOC_OBJECT33_EVENTS    TMWCNFG_MAX_EVENTS
#define SDNPCNFG_NUMALLOC_OBJECT42_EVENTS    TMWCNFG_MAX_EVENTS
#define SDNPCNFG_NUMALLOC_OBJECT43_EVENTS    TMWCNFG_MAX_EVENTS
#define SDNPCNFG_NUMALLOC_OBJECT88_EVENTS    TMWCNFG_MAX_EVENTS
#define SDNPCNFG_NUMALLOC_OBJECT111_EVENTS   TMWCNFG_MAX_EVENTS
#define SDNPCNFG_NUMALLOC_OBJECT113_EVENTS   TMWCNFG_MAX_EVENTS
#define SDNPCNFG_NUMALLOC_OBJECT115_EVENTS   TMWCNFG_MAX_EVENTS
#define SDNPCNFG_NUMALLOC_OBJECT120_EVENTS   TMWCNFG_MAX_EVENTS
#define SDNPCNFG_NUMALLOC_OBJECT122_EVENTS   TMWCNFG_MAX_EVENTS

/* Maximum number of simultaneous file tranfers that can be in progress 
 * which also determines the maximum number of file transfer events 
 * that can be queued at any time.
 * Currently on one per session is supported.
 */
#define SDNPCNFG_NUMALLOC_OBJECT70_BLOCKS    TMWCNFG_MAX_SESSIONS

/* Determines the maximum number of controls allowed in a single request.
 * This applies to both Binary (CROB) and Analog Control outputs.     
 * It is also possible to permit only a single CROB per request at runtime
 * by opening the SDNP Session with allowMultiCROBRequests set to TMWDEFS_FALSE
 */
#ifndef SDNPCNFG_MAX_CONTROL_REQUESTS
#define SDNPCNFG_MAX_CONTROL_REQUESTS         10
#endif


/* Set this parameter to the desired size of the memory buffer
 * used during generation of a device description XML file.
 * This buffer should be large enough to hold an indivisible block of
 * device description information. 
 */
#define SDNPCNFG_XML_SAVE_BUF_SIZE            4096

/* Size of configuration array indicating what groups should be included
 * by a slave session in response to a read static data request.
 */
#define SDNPCNFG_MAX_NUMBER_STATIC_GROUPS     16

/* Sequential File Transfer Configuration using Object 70 Variation 2-7 
 * Maximum block size for file transfer reads and writes. This will be
 * limited by maximum rx and tx fragment size but can be set smaller to 
 * save memory.
 */
#define SDNPCNFG_MAX_BLOCK_SIZE               DNPCNFG_MAX_RX_FRAGMENT_LENGTH

/* Maximum number of directory entries that will be buffered before 
 * sending response to a read directory request
 */
#define SDNPCNFG_OBJ70_DIR_SIZE               32    

/* Number of milliseconds before retrying a sdnpdata_xxx call if that function
 * returned "ASYNC" indicating response should be sent later as an event. 
 * NOTE: parameter below is used for configuring retry of directory read call.
 */
#define SDNPCNFG_OBJ70_RETRY_TIME             100

/* Number of milliseconds before retrying a sdnpdata_readFileInfo call if that 
 * function returned "ASYNC" indicating response should be sent later as 
 * an event. This parameter is used for read directory versus all other 
 * sdnpdata_xxx calls for file transfer. 
 */
#define SDNPCNFG_OBJ70_DIR_RETRY_TIME         100   

/* Maximum length of Data Set Event buffer. Since it is required that a data set 
 * fits inside of a Transmit Fragment this is normally set the same. However, if 
 * you are limiting the size of Data Sets you could reduce this to reduce the size
 * of the data set event structure (SDNPEVNT_O088_EVENT), possibly saving memory
 * depending on your memory allocation scheme.
 */
#define SDNPCNFG_MAX_DSEVENT_LENGTH           DNPCNFG_MAX_TX_FRAGMENT_LENGTH

/* Set this to TMWDEFS_TRUE if you want to implement database code to perform
 * event queue management instead of having the SCL do this for you. This is 
 * useful if events need to be kept in Non-volatile memory, or special queueing
 * algorithms are desired. If this is left at TMWDEFS_FALSE, the SCL will allocate
 * memory and completely manage the event queues.
 */
#define SDNPCNFG_USER_MANAGED_EVENTS          TMWDEFS_FALSE

/* Specify the number of slave DNP simulated databases that can be allocated. 
 * The TMW SCL will allocate a new simulated database for each sdnp session.
 * These are not needed once an actual database is implemented.
 */
#define SDNPCNFG_NUMALLOC_SIM_DATABASES       TMWCNFG_MAX_SIM_DATABASES

/* Specify the number of slave DNP simulated database user managed events.
 * This is only necessary for testing the simulated database implementation of
 * database event queue management.
 */
#define SDNPCNFG_NUMALLOC_SIM_UMEVENTS        100

/* Specify the number of slave DNP simulated database Data Sets structures that 
 * can be allocated. These are only needed to simulate Data Set points.
 * These are not needed once an actual database is implemented.
 */
#define SDNPCNFG_NUMALLOC_SIM_DATASETS        (TMWCNFG_MAX_SESSIONS*10)

/* Support DNP Secure Authentication Specification Version 2 */
#define SDNPCNFG_SUPPORT_SA_VERSION2          DNPCNFG_SUPPORT_SA_VERSION2

/* Support DNP Secure Authentication Specification Version 5
 * This is an optional component
 */
#define SDNPCNFG_SUPPORT_SA_VERSION5          DNPCNFG_SUPPORT_SA_VERSION5

/* Specify the total number of slave DNP Secure Authentication Version 5 Users
 * for all sessions in the entire system that can be allocated when static or 
 * dynamic memory with limited quantities is used.
 */    
#define SDNPCNFG_NUMALLOC_AUTH_USERS          (DNPCNFG_AUTH_MAX_NUMBER_USERS * SDNPCNFG_NUMALLOC_SESNS)

/* Specify the number of buffers needed for generating a device profile in xml 
 * format. This would never need to be more than 1 per slave session.
 * This would only need to be more than 1 if multiple sessions are supported 
 * and they each need to be able to generate a device profile simultaneously.
 * Only applicable if SDNPDATA_SUPPORT_XML2 is defined.
 */
#define SDNPCNFG_NUMALLOC_DEVICE_PROFILES     TMWCNFG_MAX_SESSIONS

#endif /* SDNPCNFG_DEFINED */
