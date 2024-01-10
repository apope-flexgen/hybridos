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

/* file: tmwscl.h
 * description: Triangle MicroWorks Source Code library defines
 */
#ifndef TMWSCL_DEFINED
#define TMWSCL_DEFINED

typedef enum TMWSCL_INFO_ENUM
{
  TMWSCL_INFO_OPENED,
  TMWSCL_INFO_CLOSED,
  TMWSCL_INFO_ONLINE,                                                                                                                                                                                            
  TMWSCL_INFO_OFFLINE,
  TMWSCL_INFO_STARTDT,
  TMWSCL_INFO_STOPDT,
  TMWSCL_INFO_RESET_CU,
  TMWSCL_INFO_RESET_FCB,
  TMWSCL_INFO_RESET_USER,
  TMWSCL_INFO_TIMEOUT
} TMWSCL_INFO;

#endif /* TMWSCL_DEFINED */
