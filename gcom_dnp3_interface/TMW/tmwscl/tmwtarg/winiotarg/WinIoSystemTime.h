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
/*
 * FileName : WinIoSystemTime.h
 * Author   : Fred VanEijk
 * Purpose  : Implementation of system time (i.e. uses the PC Clock)
 *            
 */

#pragma once
#include <vector>
#include "tmwscl/utils/tmwtarg.h"
#include "WinIoTarg/WinIoBaseTime.h"

#if _MSC_VER >= 1400
#define MAX_SIZE    255

class CRegistryManager
{
public:
  CRegistryManager();
  virtual ~CRegistryManager();

  //operations.
public:
  LONG OpenRegistryKey(HKEY hKey,
    LPCTSTR lpSubKey,
    DWORD ulOptions = 0,
    REGSAM samDesired = KEY_READ,
    PHKEY phkResult=NULL
    );

  LONG EnumerateSubKeys(HKEY hKey, std::vector<std::string> &o_subkeyArray);
  LONG EnumerateKeyValueNames(HKEY hKey, std::vector<std::string> &o_keyValueNamesArray);
  LONG QueryReistryValue( HKEY hKey,
    LPCTSTR lpValueName,
    LPDWORD lpReserved,
    LPDWORD lpType,
    LPBYTE lpData,
    LPDWORD lpcbData
    );

};

struct regTZI
{
  long Bias;
  long StandardBias;
  long DaylightBias;
  SYSTEMTIME StandardDate; 
  SYSTEMTIME DaylightDate;
};

class CRegTimeZoneInfo
{

public:

  TCHAR tcName[MAX_SIZE];
  TCHAR m_szDisplay[MAX_SIZE];
  TCHAR m_szDlt[MAX_SIZE];
  TCHAR m_szStd[MAX_SIZE];
  TCHAR m_szMapID[MAX_SIZE];
  DWORD m_iIndex;
  DWORD ActiveTimeBias;
  regTZI m_regTZI;

public:

  void FillFromRegistry(int i_RegistryIndex, BYTE* i_pValue, std::string &i_strValueName, DWORD i_ValueSize);
};
#endif

class WinIoSystemTime : public WinIoBaseTime
{
public:
  WinIoSystemTime(WINIO_TIME_MODE timeMode)
  {
    m_timeMode = timeMode;
  }
  
  virtual void LogMessage(const char   *format, ...);

  /* function: getDateTime
   * purpose: return the current date and time
   * arguments:
   *  pDateTime - storage to return date and time into
   * returns:
   *  void
   */
  virtual void getDateTime(TMWDTIME *pDateTime);

  /* function: getUTCDateTime
   * purpose: return the current Coordinated Universal Time(UTC) time
   * arguments:
   *  pDateTime - storage to return date and time into
   * returns:
   *  void
   */
  static void getUTCDateTime(TMWDTIME *pDateTime);

  /* Set clock to specified time. Future calls to
   * getDateTime will return a clock based on this time.
   */
  virtual TMWTYPES_BOOL setDateTime(const TMWDTIME *pNewDateTime);

  /* Set clock to specified time. Future calls to
   * getUTCDateTime will return a clock based on this time.
   */
  static TMWTYPES_BOOL setUTCDateTime(const TMWDTIME *pNewDateTime);
  
#if _MSC_VER >= 1400
  static CRegistryManager *GetRegistryManager();
  static void CleanRegManager();

  static std::vector<std::string>    m_arrTimeZones;
  static std::vector<CRegTimeZoneInfo*> m_arrRegTimeZoneInfo;

  static int EnumerateTimeZones();
  static int SpecificLocalTimeToSystemTime(SYSTEMTIME* i_stLocal, SYSTEMTIME* o_stUniversal);
  static bool GetCurrentTimeZone(char *curTimeZone);
  static bool GetTimeZone(int index, char *returnedTimeZone);
  static CRegTimeZoneInfo *GetTimeZoneStruct(int index);
  static int GetUTCTimeZone(const char *utcTZName);
  static int GetTimeZoneIndex(const char *tzName);
  static void SetTimeZone(int timeZone, bool ignoreDST)
  {
    _timeZoneIndex = timeZone;
    _ignoreDST = ignoreDST;
  }

  /// <summary> Gets specific time from UTC time. </summary>
  ///
  /// <param name="utcTime">        [in,out] If non-null, the UTC time. </param>
  /// <param name="specificTime">   [in,out] If non-null, the specific time. </param>
  /// <param name="timeZoneIndex">  Zero-based index of the time zone. </param>
  /// <param name="ignoreDST">      true to ignore day light savings time changes. </param>

  static void GetSpecificTimeFromUtcTime(TMWDTIME *utcTime, TMWDTIME *specificTime, int timeZoneIndex, bool ignoreDST);


  static char * strptime(const char *s, const char *format, struct tm *tm, int *millisecs);

  static CRegistryManager        *g_pObjRegistryManager;

protected:

  /* function: getTZDateTime
   * purpose: return the current date and time for a time zone
   * arguments:
   *  pDateTime - storage to return date and time into
   *  timeZoneIndex - the time zone
   *  ignoreDST - whether to adjust for daylight savings
   * returns:
   *  void
   * note:
   *  if the conversion fails the Local time is returned
   */
  void getTZDateTime(TMWDTIME *pDateTime);

  static bool _ignoreDST;
  static int  _timeZoneIndex;

private:
  static bool strp_atoi(const char * & s, int & result, int low, int high, int offset);
  static int GetFullTimeZoneInfoFromRegistry();
  static int SortTimeZoneList();
#endif
};

//WINIOTARG_API CArray<CRegTimeZoneInfo*, CRegTimeZoneInfo*> *GetTimeZoneInfoArray();
//WINIOTARG_API int WinIoGetUTCTimeZone();
