/*****************************************************************************/
/* Triangle MicroWorks, Inc.                         Copyright (c) 2008-2011 */
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

/* file: twmtargos.h
 * description: Common definitions used by the SCL that are OS dependent.
 */
#ifndef tmwtargos_DEFINED
#define tmwtargos_DEFINED


#include "tmwscl/utils/tmwtypes.h"
#include "tmwtargcnfg.h"

#if TMWCNFG_SUPPORT_THREADS
 /* Type for the handle that the OS passes back for the created thread */
#define TMW_ThreadId            void *

 /* Declaration for the entry point of a thread */
#define TMW_ThreadDecl          void *

/* The type definition of the entry point of a thread */
/* In most cases it will be the same as TMW_ThreadDecl */
#define TMW_ThreadPtr           void *

/* The type definition the OS expects to pass as an argument when starting a thread */
#define TMW_ThreadArg           void *

/* A Macro is used here instead of a function call because of Windows __stdcall calling convention */
/* Put your code here */
#define TMW_ThreadCreate(pId, pFunc, pFuncArg, priority, stackSize) TMWTARG_UNUSED_PARAM(pFunc)

/* If anything needs to be done when exiting a thread as it does in Linux deallocating the stack memory */
#define TMW_ThreadDetach(pThreadId) TMWTARG_UNUSED_PARAM(pThreadId)
 
#if defined TMWDEFS_RESOURCE_LOCK
#undef TMWDEFS_RESOURCE_LOCK
#endif
/* Put your code here */
#define TMWDEFS_RESOURCE_LOCK  void *
#endif

/* Type definition of the baud rate configuration parameter in the TMWTARG232_CONFIG structure.
 * The examples provided use a string representation for the baud rate, if an enum better suits
 * the target environment, it can be modified here.
 */
/* Put your code here, if needed */
typedef TMWTYPES_CHAR TMWTARG_TYPE_BAUDRATE[10];

#endif /* tmwtargos_DEFINED */
