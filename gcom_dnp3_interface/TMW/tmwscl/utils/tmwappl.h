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

/* file: tmwappl.h
 * description: Channel related structures and functions 
 *  implementations.
 */
#ifndef TMWAPPL_DEFINED
#define TMWAPPL_DEFINED

#include "tmwscl/utils/tmwcnfg.h"
#include "tmwscl/utils/tmwdefs.h"
#include "tmwscl/utils/tmwdlist.h"
#include "tmwtargos.h"  /* TMWDEFS_RESOURCE_LOCK */

/* Used to determine if protocol specific initialization function 
 * has been called
 */
typedef TMWTYPES_ULONG  TMWAPPL_INIT_COMPONENT;
#define TMWAPPL_INIT_DNP          0x00000001
#define TMWAPPL_INIT_MDNP         0x00000002
#define TMWAPPL_INIT_SDNP         0x00000004
#define TMWAPPL_INIT_MB           0x00000008
#define TMWAPPL_INIT_MMB          0x00000010
#define TMWAPPL_INIT_SMB          0x00000020
#define TMWAPPL_INIT_I870         0x00000040
#define TMWAPPL_INIT_FT12         0x00000080
#define TMWAPPL_INIT_101          0x00000100
#define TMWAPPL_INIT_M101         0x00000200
#define TMWAPPL_INIT_S101         0x00000400
#define TMWAPPL_INIT_102          0x00000800
#define TMWAPPL_INIT_M102         0x00001000
#define TMWAPPL_INIT_S102         0x00002000
#define TMWAPPL_INIT_103          0x00004000
#define TMWAPPL_INIT_M103         0x00008000
#define TMWAPPL_INIT_S103         0x00010000
#define TMWAPPL_INIT_104          0x00020000
#define TMWAPPL_INIT_M104         0x00040000
#define TMWAPPL_INIT_S104         0x00080000
#define TMWAPPL_INIT_61850        0x00100000
#define TMWAPPL_INIT_M61850       0x00200000
#define TMWAPPL_INIT_S61850       0x00400000
#define TMWAPPL_INIT_LAST         TMWAPPL_INIT_S61850

/* Application layer context
 */
typedef struct TMWApplicationStruct {

  /* List of channels managed by this application context */
  TMWDLIST channels;
  
#if TMWCNFG_SUPPORT_THREADS
  TMWDEFS_RESOURCE_LOCK lock;
#endif

} TMWAPPL;

#ifdef __cplusplus
extern "C" {
#endif

  /* function: tmwappl_init 
   * purpose: Initialize SCL if initialized data is not provided.
   *  If your environment does not allow initialized data 
   *  this must be called before calling tmwappl_initSCL and tmwapp_initApplication 
   *  or any other functions from the SCL
   * arguments:
   *  void
   * returns:
   *  void
   */
  void TMWDEFS_GLOBAL tmwappl_init(void);

  /* function: tmwappl_internalInit 
   * purpose: INTERNAL FUNCTION called from within SCL
   * arguments:
   *  void
   * returns:
   *  TMWDEFS_TRUE if this is the the first time this is called
   *  TMWDEFS_FALSE if this has already been called.
   */
  TMWTYPES_BOOL TMWDEFS_GLOBAL tmwappl_internalInit(void);

  /* function: tmwappl_initSCL
  * purpose: Initialize the SCL for a multithreaded environment when multiple 
  *  application contexts are required.
  *  NOTE This function must be called prior to any calls to tmwappl_initApplication 
  *  in a multi-threaded environment with multiple application contexts. The call 
  *  is optional when TMWCNFG_SUPPORT_THREADS is not defined.
  * arguments:
  *  void
  * returns:
  *  TMWDEFS_TRUE if the SCL has initialized correctly
  *  TMWDEFS_FALSE The SCL initializaiton failed.
  */
  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL tmwappl_initSCL(void);

  /* function: tmwappl_initApplication 
   * purpose: Initialize a new application context
   *  NOTE to support multiple application contexts you need to support multiple 
   *  timer queues. define TMWCNFG_MULTIPLE_TIMER_QS
   * arguments:
   *  void
   * returns:
   *  pointer to new application context or TMWDEFS_NULL
   */
  TMWDEFS_SCL_API TMWAPPL * TMWDEFS_GLOBAL tmwappl_initApplication(void);

  /* function: tmwappl_closeApplication  
   * purpose: Close an application context
   * arguments:
   *  pApplContext - pointer to application context
   *  forceClose - TMWDEFS_TRUE to force context to close any open channels
   *   NOTE: This is not currently implemented
   * returns:
   *  TMWDEFS_TRUE if successful
   *  TMWDEFS_FALSE otherwise 
   *   NOTE: close will fail if there are open channels for this appl 
   *         context and forceClose if TMWDEFS_FALSE.
   */
  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL tmwappl_closeApplication(
    TMWAPPL *pApplContext,
    TMWTYPES_BOOL forceClose);

  /* function: tmwappl_getInitialized 
   * purpose: Determine if a specific protocol has been marked initialized. This
   *  will be called by the protocol specific portions of the SCL. It is not
   *  intended to be called by application code.
   * arguments:
   *   component - indicate protocol component
   * returns:
   *  TMWDEFS_TRUE if protocol/type has already been marked initialized
   *  TMWDEFS_FALSE if protocol/type had not been previously initialized
   */
  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL tmwappl_getInitialized(
    TMWAPPL_INIT_COMPONENT component); 
    
  /* function: tmwappl_setInitialized 
   * purpose: Mark the specific protocol initialized.
   * arguments:
   *   component - indicate protocol component
   * returns:
   *   void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwappl_setInitialized(
    TMWAPPL_INIT_COMPONENT component);

  /* function: tmwappl_checkForInput 
   * purpose: Check for input on any of the specified application context's
   *  input channels.
   * arguments:
   *  pApplContext - application context returned by tmwappl_initApplication
   * returns:
   *  void
   */
  TMWDEFS_SCL_API void TMWDEFS_GLOBAL tmwappl_checkForInput(
    TMWAPPL *pApplContext);

  /* function: tmwappl_findChannel
   * purpose: See if this channel is on the list of application channels. This
   *  can be used to check if a channel was deleted from a target layer callback
   *  function.
   * arguments:
   *  pApplContext - application context returned by tmwappl_initApplication
   *  pChannel - channel context to find
   * returns:
   * TMWDEFS_TRUE if channel is found on list
   */
  TMWDEFS_SCL_API TMWTYPES_BOOL TMWDEFS_GLOBAL tmwappl_findChannel(
    TMWAPPL *pApplContext,
    void *pChannel);

#ifdef __cplusplus
}
#endif
#endif /* TMWAPPL_DEFINED */
