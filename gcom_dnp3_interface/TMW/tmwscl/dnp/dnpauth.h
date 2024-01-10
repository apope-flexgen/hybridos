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

/* file: dnpauth.h
 * description: This file is intended for internal SCL use only.
 *   DNP Master and slave functionality for Secure Authentication
 */
#ifndef DNPAUTH_DEFINED
#define DNPAUTH_DEFINED

#include "tmwscl/dnp/dnpdefs.h" 
#include "tmwscl/dnp/dnpcnfg.h"

/* Session Key maximum length
 * Minimum required by spec is 16. 
 * Larger values require more memory per user number allowed.
 */
#define DNPAUTH_MAX_KEY_LENGTH          64

 /*  Technical Bulletin TB2016 - 002 limits the challenge data size to 4-64 octets. */
#define DNPAUTH_MIN_CHALLENGE_DATA      4
#define DNPAUTH_MAX_CHALLENGE_DATA      64

/* 1815-2012 Table 7-17 and TB2016-002 limits this to 32 octets. 
 * Previous to 3.29 we used the above define so we will
 * limit what we send to 32, but will still support receiving 64 octets.
 */
#define DNPAUTH_MAX_V11_V12_CHALL_DATA  32

/* The max length is not limited by the spec and could be configured greater than 255 */
#define DNPAUTH_MAX_ERROR_TEXT_LENGTH   255

/* Number of additional bytes required for aggressive mode objects */
#define DNPAUTH_AGGRESSIVE_SIZE         32

/* The size of Object Group 120 Variation 3 Aggressive Mode Request */
#define DNPAUTH_OBJ120V3_SIZE           10

/* At 2007 DNP3 Tech Committee face to face it was decided for the following
 * cases the user number used should represent the "device" or "any user" 
 * and should be == 1
 *  Outstation sending aggressive mode Unsolicited Response
 *  Outstation sending aggressive mode Solicited Response
 *  Master challenging an Unsolicited Response
 *  Master challenging a Solicited Response
 *  Master sending an aggressive mode poll or application confirm 
 */

#define DNPAUTH_DEFAULT_USERNUMBER      1
 
/* Reason for Challenge, as defined in Secure Authentication Spec and    */
/* Object Group 120 Variation 1 Object Library                           */
#define DNPAUTH_REASON_CRITICAL         1
 
/* MAC Algorithms, as defined in Secure Authentication Spec and          */
/* Object Group 120 Variation 1 Object Library                           */ 
/* In version 2 these were called HMACs */
#define DNPAUTH_HMAC_SHA1_4OCTET        1 /* Only for Vers 2  and backward 
                                           * compatibility as per TB2016-002 
                                           */
#define DNPAUTH_HMAC_SHA1_10OCTET       2 
#define DNPAUTH_HMAC_SHA256_8OCTET      3
#define DNPAUTH_HMAC_SHA256_16OCTET     4
#define DNPAUTH_HMAC_SHA1_8OCTET        5 
 
/* In version 5 these are called MACs */
#define DNPAUTH_MAC_SHA1_10OCTET       2 
#define DNPAUTH_MAC_SHA256_8OCTET      3  /* serial */
#define DNPAUTH_MAC_SHA256_16OCTET     4
#define DNPAUTH_MAC_SHA1_8OCTET        5  /* serial */
#define DNPAUTH_MAC_AESGMAC_12OCTET    6

/* Session key status, as defined in Secure Authentication Spec and      */ 
/* Object Group 120 Variation 5 Object Library                           */
#define DNPAUTH_KEY_OK                  1
#define DNPAUTH_KEY_NOTINIT             2
#define DNPAUTH_KEY_COMMFAIL            3
#define DNPAUTH_KEY_AUTHFAIL            4 

/* AES Key Wrap Algorithms, as defined in Secure Authentication Spec and */
/* Object Group 120 Variation 5 Object Library                           */
#define DNPAUTH_KEYWRAP_AES128          1 
#define DNPAUTH_KEYWRAP_AES256          2 

/* Error message codes, as defined in Secure Authentication Spec and     */
/* Object Group 120 Variation 7 Object Library                           */
#define DNPAUTH_ERROR_NONE              0
#define DNPAUTH_ERROR_AUTHENTFAILED     1  /* Authentication failed */
#define DNPAUTH_ERROR_UNEXPECTEDREPLY   2  /* Only for Vers 2  */
#define DNPAUTH_ERROR_NOREPLY           3  /* Only for Vers 2  */
#define DNPAUTH_ERROR_AGGRESSNOTPERMIT  4
#define DNPAUTH_ERROR_MACNOTPERMIT      5
#define DNPAUTH_ERROR_KEYWRAPNOTPERMIT  6
#define DNPAUTH_ERROR_AUTHORFAILED      7  /* Authorization failed */

/* Update Key Change Method not permitted.  
 * The outstation does not permit the specified key change method on this link.  
 * Mandatory Update Key Change Methods are specified in the supplement to Volume 2.
 */
#define DNPAUTH_ERROR_UPDCHGNOTPERMIT   8

/* Invalid Signature.  
 * The digital signature supplied in a User Status Change, User Certificate
 * or Signed Update Key Change object was invalid.
 */
#define DNPAUTH_ERROR_INVALIDSIG        9

/* Invalid Certification Data.  
 * The Certification Data supplied in a User Status Change object was invalid.
 */
#define DNPAUTH_ERROR_INVALIDCERTDATA  10

/* Unknown User.  
 * The master attempted to change the Update Key of a user without first supplying 
 * a valid User Status Change.
 */
#define DNPAUTH_ERROR_UNKNOWN_USER     11

/* Max Session Key Status Requests Exceeded. The master on a different association 
 * has requested Session Key Status too often and it is possible a denial of service 
 * attack is underway.
 */
#define DNPAUTH_ERROR_MAX_STATUS_REQ   12


/* User Status Change, Key Change Method, defined in 120v10 */
/* Key Transport/Authentication of User Credentials/Authentication of Mastart to OS and OS to master */

/* 1, 2, 64, 65, and 66 were to be used in version 4 and are obsolete */

/* Symmetric AES-128/SHA-1/SHA-1 */
#define DNPAUTH_KEYCH_SYMAES128_SHA1   3

/* Symmetric AES-256/SHA-256/SHA-256 */
#define DNPAUTH_KEYCH_SYMAES256_SHA256 4

/* Symmetric AES-256/AES-GMAC/AES-GMAC */
#define DNPAUTH_KEYCH_SYMAES256_GMAC 5

/* Asymmetric RSA-1024/SHA-1/SHA-1 */
#define DNPAUTH_KEYCH_ASYM_RSA1024_SHA1 67

/* Asymmetric RSA-2048/SHA-256/SHA-256 */
#define DNPAUTH_KEYCH_ASYM_RSA2048_SHA256 68

/* Asymmetric RSA-3072/SHA-256/SHA-256 */
#define DNPAUTH_KEYCH_ASYM_RSA3072_SHA256 69

/* Asymmetric RSA-2048/SHA-256/AES-GMAC */
#define DNPAUTH_KEYCH_ASYM_RSA2048_SHA256_GMAC 70

/* Asymmetric RSA-3072/SHA-256/AES-GMAC */
#define DNPAUTH_KEYCH_ASYM_RSA3072_SHA256_GMAC 71

/* New Key Change Methods using RSA for signing defined in TB2016-002 */

/* Asymmetric RSA-1024/RSA-SHA-1/SHA-1 */
#define DNPAUTH_KEYCH_ASYM_RSA1024_RSA_SHA1 72

/* Asymmetric RSA-2048/RSA-SHA-256/SHA-256 */
#define DNPAUTH_KEYCH_ASYM_RSA2048_RSA_SHA256 73

/* Asymmetric RSA-3072/RSA-SHA-256/SHA-256 */
#define DNPAUTH_KEYCH_ASYM_RSA3072_RSA_SHA256 74

/* Asymmetric RSA-2048/RSA-SHA-256/AES-GMAC */
#define DNPAUTH_KEYCH_ASYM_RSA2048_RSA_SHA256_GMAC 75

/* Asymmetric RSA-3072/RSA-SHA-256/AES-GMAC */
#define DNPAUTH_KEYCH_ASYM_RSA3072_RSA_SHA256_GMAC 76


/* IEC 62351-8 Certificate Type */
/* ID Certificate, does contain User Public Key */
#define DNPAUTH_ID_CERTIFICATE          1
/* Attribute Certificate, does not contain User Public Key */
#define DNPAUTH_ATTRIB_CERTIFICATE      2
 
/* User Status Change, Operation, as defined in 120v10 */
/* ADD.  This is a new user not previously known to the outstation.  
 *   The outstation shall record the User Status Information
 */
#define DNPAUTH_USER_STATUS_ADD         1
/* DELETE.  The outstation shall invalidate the existing Update Key associated 
 *   with the User Name.
 */
#define DNPAUTH_USER_STATUS_DELETE      2
/* CHANGE.  The outstation shall update the User Status Information associated
 *   with the User Name.
 */
#define DNPAUTH_USER_STATUS_CHANGE      3


/* User Roles as defined in Secure Authentication Specification Version 5 */
/* Access to Monitor Data. */   
#define DNPAUTH_USER_ROLE_VIEWER        0
/* Access to Monitor Data and Controls */
#define DNPAUTH_USER_ROLE_OPERATOR      1
/* Access to Monitor Data, Read/Write/Delete Data Files, Change Config, and Local Login */    
#define DNPAUTH_USER_ROLE_ENGINEER      2
/* Access to Monitor Data, Read/Write Data Files, Change Config, Firmware, and Local Login */  
#define DNPAUTH_USER_ROLE_INSTALLER     3
/* Access to Change Security Config, Firmware, and Local Login */  
#define DNPAUTH_USER_ROLE_SECADM        4 
/* Access to Monitor Data, Read Files, and Local Login */   
#define DNPAUTH_USER_ROLE_SECAUD        5 
/* Access to Monitor Data, Delete Files, Change Configuration, and Change Security Roles */  
#define DNPAUTH_USER_ROLE_RBACMNT       6 

/* Access to ALL functions. To be used when there is only a single user that requires all permissions. 
 * This was added at March 14 WG C12(1815) meeting.
 */
#define DNPAUTH_USER_ROLE_SINGLEUSER    32768 

/* This is not in the spec, it is used to locally indicate the role expired */
#define DNPAUTH_USER_ROLE_EXPIRED       0xffff


/* Statistic threshold defaults, from Secure Authentication specification*/
#define DNPAUTH_UNEXPECTED_MSG_THRESHOLD       3
#define DNPAUTH_AUTHOR_FAIL_THRESHOLD          5
#define DNPAUTH_AUTHENT_FAIL_THRESHOLD         5
#define DNPAUTH_REPLY_TIMEOUT_THRESHOLD        3
#define DNPAUTH_REKEY_THRESHOLD                3
#define DNPAUTH_TOTAL_MSG_SENT_THRESHOLD     100
#define DNPAUTH_TOTAL_MSG_RCVD_THRESHOLD     100
#define DNPAUTH_CRIT_MSG_SENT_THRESHOLD      100
#define DNPAUTH_CRIT_MSG_RCVD_THRESHOLD      100
#define DNPAUTH_DISCARDED_MSG_THRESHOLD       10
#define DNPAUTH_ERROR_MSG_SENT_THRESHOLD       2
#define DNPAUTH_ERROR_MSG_RCVD_THRESHOLD      10
#define DNPAUTH_SUCCESS_AUTHENT_THRESHOLD    100
#define DNPAUTH_SESKEY_CHANGE_THRESHOLD        5
#define DNPAUTH_FAILSESKEY_CHANGE_THRESHOLD    5
#define DNPAUTH_UPDKEY_CHANGE_THRESHOLD        1
#define DNPAUTH_FAILUPDKEY_CHANGE_THRESHOLD    1
#define DNPAUTH_REKEY_DUE_RESTART_THRESHOLD    3

/* Statistics index numbers, from Secure Authentication specification */
/* Unexpected Messages */
#define DNPAUTH_UNEXPECTED_MSG_INDEX               0
/* Authorization Failures */
#define DNPAUTH_AUTHOR_FAIL_INDEX                  1
/* Authentication Failures */
#define DNPAUTH_AUTHENT_FAIL_INDEX                 2
/* Reply Timeouts */
#define DNPAUTH_REPLY_TIMEOUT_INDEX                3
/* Rekeys Due to Authentication Failure */
#define DNPAUTH_REKEY_DUETOFAIL_INDEX              4
/* Total Messages Sent  */
#define DNPAUTH_TOTAL_MSG_SENT_INDEX               5
/* Total Messages Received */
#define DNPAUTH_TOTAL_MSG_RCVD_INDEX               6
/* Critical Messages Sent  */
#define DNPAUTH_CRIT_MSG_SENT_INDEX                7
/* Critical Messages Received */
#define DNPAUTH_CRIT_MSG_RCVD_INDEX                8
/* Discarded Messages */
#define DNPAUTH_DISCARDED_MSG_INDEX                9
/* Error Messages Sent */
#define DNPAUTH_ERROR_MSG_SENT_INDEX              10
/* Error Messages Rxed */
#define DNPAUTH_ERROR_MSG_RCVD_INDEX              11
/* Successful Authentications */
#define DNPAUTH_SUCCESS_AUTHENT_INDEX             12
/* Session Key Changes */
#define DNPAUTH_SESKEY_CHANGE_INDEX               13
/* Failed Session Key Changes */
#define DNPAUTH_FAILSESKEY_CHANGE_INDEX           14
/* Update Key Changes */
#define DNPAUTH_UPDKEY_CHANGE_INDEX               15
/* Failed Update Key Changes */
#define DNPAUTH_FAILUPDKEY_CHANGE_INDEX           16
/* Rekeys Due to Restarts */
#define DNPAUTH_REKEY_DUE_RESTART_INDEX           17

#define DNPAUTH_NUMBER_STATISTICS                 18


/* These values indicate the authentication status of key status g120v5 
 * when received on master. They are passed to dnpdiag.c for diagnostic
 * purposes only
 */
#define DNPAUTH_V5_AUTHENTICATED         0
#define DNPAUTH_V5_NOTAUTHENTICATED      1 
#define DNPAUTH_V5_USERNOTFOUND          2 
#define DNPAUTH_V5_BADFORMAT             0xf 

/* Received g120v2 failed authentication */
#define DNPAUTH_V2_NOTAUTHENTICATED      10
/* Received g120v9 failed authentication */
#define DNPAUTH_V9_NOTAUTHENTICATED      11


/* Structure to hold data from object header */
typedef struct DNPAuthObjectHeaderStruct {
  TMWTYPES_UCHAR group;
  TMWTYPES_UCHAR variation;
  TMWTYPES_UCHAR qualifier;
} DNPAUTH_OBJECT_HEADER;

#ifdef __cplusplus
extern "C" {
#endif
   
  /* function: dnpauth_keyWraptoTMWCryptoAlgo
   * purpose: Determine what TMWCRYPTO algorithm define to use based on the key wrap algorithm from the spec.
   * arguments:  
   *  algorithm - 
   *   DNPAUTH_KEYWRAP_AES128
   *   DNPAUTH_KEYWRAP_AES256 
   * returns: TMWCRYPTO define indicating which algorithm to use
   */ 
  TMWTYPES_ULONG TMWDEFS_GLOBAL dnpauth_keyWraptoTMWCryptoAlgo(TMWTYPES_UCHAR algorithm);
  
  /* function: dnpauth_MACtoTMWCryptoAlgo
   * purpose: Determine what TMWCRYPTO algorithm define to use based on the MAC algorithm from the spec.
   * arguments:  
   *  algorithm - 
   *   DNPAUTH_HMAC_SHA1_4OCTET Only for SA V2.
   *   DNPAUTH_MAC_SHA1_8OCTET
   *   DNPAUTH_MAC_SHA1_10OCTET
   *   DNPAUTH_MAC_SHA256_8OCTET
   *   DNPAUTH_MAC_SHA256_16OCTET
   * returns: TMWCRYPTO define indicating which algorithm to use
   */ 
  TMWTYPES_ULONG TMWDEFS_GLOBAL dnpauth_MACtoTMWCryptoAlgo(TMWTYPES_UCHAR algorithm);
 
  /* function: dnpauth_MACtoTMWCryptoAlgo
   * purpose: Determine what length MAC to use based on the MAC algorithm from the spec. 
   * arguments:  
   *  algorithm - 
   *   DNPAUTH_HMAC_SHA1_4OCTET Only for SA V2.
   *   DNPAUTH_MAC_SHA1_8OCTET
   *   DNPAUTH_MAC_SHA1_10OCTET
   *   DNPAUTH_MAC_SHA256_8OCTET
   *   DNPAUTH_MAC_SHA256_16OCTET
   * returns: length in octets
   */ 
  TMWTYPES_UCHAR TMWDEFS_GLOBAL dnpauth_MACtoLength(TMWTYPES_UCHAR algorithm);

  /* function: dnpauth_methodChallengeLength
   * purpose: Determine the number of octets to use in the g120v11 and g120v12 
   *   for the specified keyChangeMethod from the spec.
   * arguments:  
   *  keyChangeMethod - 
   *   DNPAUTH_KEYCH_SYMAES128_SHA1
   *   DNPAUTH_KEYCH_SYMAES256_SHA256
   *   DNPAUTH_KEYCH_SYMAES256_GMAC
   *   DNPAUTH_KEYCH_ASYM_RSA1024_SHA1
   *   DNPAUTH_KEYCH_ASYM_RSA2048_SHA256
   *   DNPAUTH_KEYCH_ASYM_RSA3072_SHA256
   *   DNPAUTH_KEYCH_ASYM_RSA2048_SHA256_GMAC
   *   DNPAUTH_KEYCH_ASYM_RSA3072_SHA256_GMAC
   *   DNPAUTH_KEYCH_ASYM_RSA1024_RSA_SHA1
   *   DNPAUTH_KEYCH_ASYM_RSA2048_RSA_SHA256
   *   DNPAUTH_KEYCH_ASYM_RSA3072_RSA_SHA256
   *   DNPAUTH_KEYCH_ASYM_RSA2048_RSA_SHA256_GMAC
   *   DNPAUTH_KEYCH_ASYM_RSA3072_RSA_SHA256_GMAC
   * returns: length in octets
   */  
 TMWTYPES_UCHAR TMWDEFS_GLOBAL dnpauth_methodChallengeLength(TMWTYPES_UCHAR keyChangeMethod);
 
  /* function: dnpauth_getUpdateKeyLength
   * purpose: Determine the number of octets to use in the update key for the specified 
   *  keyChangeMethod from the spec.
   * arguments:
   *  keyChangeMethod -
   *   DNPAUTH_KEYCH_SYMAES128_SHA1
   *   DNPAUTH_KEYCH_SYMAES256_SHA256
   *   DNPAUTH_KEYCH_SYMAES256_GMAC
   *   DNPAUTH_KEYCH_ASYM_RSA1024_SHA1
   *   DNPAUTH_KEYCH_ASYM_RSA2048_SHA256
   *   DNPAUTH_KEYCH_ASYM_RSA3072_SHA256
   *   DNPAUTH_KEYCH_ASYM_RSA2048_SHA256_GMAC
   *   DNPAUTH_KEYCH_ASYM_RSA3072_SHA256_GMAC
   *   DNPAUTH_KEYCH_ASYM_RSA1024_RSA_SHA1
   *   DNPAUTH_KEYCH_ASYM_RSA2048_RSA_SHA256
   *   DNPAUTH_KEYCH_ASYM_RSA3072_RSA_SHA256
   *   DNPAUTH_KEYCH_ASYM_RSA2048_RSA_SHA256_GMAC
   *   DNPAUTH_KEYCH_ASYM_RSA3072_RSA_SHA256_GMAC
   * returns: length in octets
   */  
  TMWTYPES_UCHAR TMWDEFS_GLOBAL dnpauth_getUpdateKeyLength(TMWTYPES_UCHAR keyChangeMethod);
  
  /* function: dnpauth_methodToTMWMacAlgo
   * purpose: determine what TMWCRYPTO MAC algorithm for g120v15 to use for specified keyChangeMethod 
   * arguments:  
   *  keyChangeMethod -
   *   DNPAUTH_KEYCH_SYMAES128_SHA1
   *   DNPAUTH_KEYCH_SYMAES256_SHA256
   *   DNPAUTH_KEYCH_SYMAES256_GMAC
   *   DNPAUTH_KEYCH_ASYM_RSA1024_SHA1
   *   DNPAUTH_KEYCH_ASYM_RSA2048_SHA256
   *   DNPAUTH_KEYCH_ASYM_RSA3072_SHA256
   *   DNPAUTH_KEYCH_ASYM_RSA2048_SHA256_GMAC
   *   DNPAUTH_KEYCH_ASYM_RSA3072_SHA256_GMAC
   *   DNPAUTH_KEYCH_ASYM_RSA1024_RSA_SHA1
   *   DNPAUTH_KEYCH_ASYM_RSA2048_RSA_SHA256
   *   DNPAUTH_KEYCH_ASYM_RSA3072_RSA_SHA256
   *   DNPAUTH_KEYCH_ASYM_RSA2048_RSA_SHA256_GMAC
   *   DNPAUTH_KEYCH_ASYM_RSA3072_RSA_SHA256_GMAC
   * returns: TMWCRYPTO define indicating which algorithm to use
   */  
  TMWTYPES_ULONG TMWDEFS_GLOBAL dnpauth_methodToTMWMacAlgo(TMWTYPES_UCHAR keyChangeMethod);

  /* function: dnpauth_methodToKeyTransAlgorithm
   * purpose: determine what key transport algorithm to use for specified keyChangeMethod 
   * arguments:  
   *  keyChangeMethod - 
   *   DNPAUTH_KEYCH_SYMAES128_SHA1
   *   DNPAUTH_KEYCH_SYMAES256_SHA256
   *   DNPAUTH_KEYCH_SYMAES256_GMAC
   *   DNPAUTH_KEYCH_ASYM_RSA1024_SHA1
   *   DNPAUTH_KEYCH_ASYM_RSA2048_SHA256
   *   DNPAUTH_KEYCH_ASYM_RSA3072_SHA256
   *   DNPAUTH_KEYCH_ASYM_RSA2048_SHA256_GMAC
   *   DNPAUTH_KEYCH_ASYM_RSA3072_SHA256_GMAC
   *   DNPAUTH_KEYCH_ASYM_RSA1024_RSA_SHA1
   *   DNPAUTH_KEYCH_ASYM_RSA2048_RSA_SHA256
   *   DNPAUTH_KEYCH_ASYM_RSA3072_RSA_SHA256
   *   DNPAUTH_KEYCH_ASYM_RSA2048_RSA_SHA256_GMAC
   *   DNPAUTH_KEYCH_ASYM_RSA3072_RSA_SHA256_GMAC
   * returns: TMWCRYPTO define indicating which algorithm to use
   */  
  TMWTYPES_ULONG TMWDEFS_GLOBAL dnpauth_methodToKeyTransAlgorithm(TMWTYPES_UCHAR keyChangeMethod);

  /* function: dnpauth_methodToSignatureAlgorithm
   * purpose: determine what signature algorithm to use for specified keyChangeMethod 
   *  This only applies to the asymmetric methods
   * arguments:  
   *  keyChangeMethod -
   *   DNPAUTH_KEYCH_ASYM_RSA1024_SHA1
   *   DNPAUTH_KEYCH_ASYM_RSA2048_SHA256
   *   DNPAUTH_KEYCH_ASYM_RSA3072_SHA256
   *   DNPAUTH_KEYCH_ASYM_RSA2048_SHA256_GMAC
   *   DNPAUTH_KEYCH_ASYM_RSA3072_SHA256_GMAC
   *   DNPAUTH_KEYCH_ASYM_RSA1024_RSA_SHA1
   *   DNPAUTH_KEYCH_ASYM_RSA2048_RSA_SHA256
   *   DNPAUTH_KEYCH_ASYM_RSA3072_RSA_SHA256
   *   DNPAUTH_KEYCH_ASYM_RSA2048_RSA_SHA256_GMAC
   *   DNPAUTH_KEYCH_ASYM_RSA3072_RSA_SHA256_GMAC
   * returns: TMWCRYPTO define indicating which algorithm to use
   */  
TMWTYPES_ULONG TMWDEFS_GLOBAL dnpauth_methodToSignatureAlgorithm(TMWTYPES_UCHAR keyChangeMethod);

  /* function: dnpauth_getAuthSecStatDefaultThreshold
   * purpose: get default threshold value 
   *  This only applies to the asymmetric methods
   * arguments:  
   *  index - threshold index
   * returns: threshold value
   */  
TMWTYPES_USHORT TMWDEFS_GLOBAL dnpauth_getAuthSecStatDefaultThreshold(TMWTYPES_USHORT index);

#ifdef __cplusplus
}
#endif
#endif /* DNPAUTH_DEFINED */
