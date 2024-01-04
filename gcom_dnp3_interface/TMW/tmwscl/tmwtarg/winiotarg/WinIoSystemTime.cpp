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
 * FileName : WinIoSystemTime.cpp
 * Author   : Fred VanEijk
 * Purpose  : Implement the simulated time
 *            
 */

#include "StdAfx.h"
#include "WinIoTarg/WinIoInterface.h"
#include "WinIoTarg/WinIoSystemTime.h"
#include "WinIoTarg/include/WinIoTarg.h"

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <time.h>
//#include <ctype>
#include <cctype>
#include <vector>
#include <algorithm>

#if defined(_DEBUG) && defined(_WIN32)
#define new DEBUG_CLIENTBLOCK
#endif

#if TMW_USE_GATEWAY_DEFINES
#if _MSC_VER >= 1400
const char * strp_weekdays[] = 
    { "sunday", "monday", "tuesday", "wednesday", "thursday", "friday", "saturday"};
const char * strp_monthnames[] = 
    { "january", "february", "march", "april", "may", "june", "july", "august", "september", "october", "november", "december"};

// singleton
// allocate the 1st time we need it
CRegistryManager        *WinIoSystemTime::g_pObjRegistryManager = NULL;
std::vector<std::string>    WinIoSystemTime::m_arrTimeZones;
std::vector<CRegTimeZoneInfo*> WinIoSystemTime::m_arrRegTimeZoneInfo;
bool WinIoSystemTime::_ignoreDST = false;
int  WinIoSystemTime::_timeZoneIndex = 0;


//CArray<CRegTimeZoneInfo*, CRegTimeZoneInfo*> *GetTimeZoneInfoArray()
//{
//  return &WinIoSystemTime::m_arrRegTimeZoneInfo;
//}
//
//
//int WinIoGetUTCTimeZone()
//{
//  return WinIoSystemTime::GetUTCTimeZone();
//}
  
///Global functions///////////////////////////////////////////////
bool TimeZoneComparer(const void *i_TZ1, const void *i_TZ2)
{

  CRegTimeZoneInfo* pTZ1 = (CRegTimeZoneInfo*)i_TZ1;
  CRegTimeZoneInfo* pTZ2 = (CRegTimeZoneInfo*)i_TZ2;

  int biasDifference = pTZ1->m_regTZI.Bias - pTZ2->m_regTZI.Bias;

  if (0 != biasDifference)
    return false; 

  if (_tcscmp(pTZ1->m_szStd, pTZ2->m_szStd) == 0)
    return true;

  return false;

  //int biasDifference = pTZ2->m_regTZI.Bias - pTZ1->m_regTZI.Bias ;

  //if (0 != biasDifference)
  //{
  //  return biasDifference; 
  //}
  //else
  //{
  //  return (_tcscmp(pTZ1->m_szStd, pTZ2->m_szStd));
  //}
}

void WinIoSystemTime::CleanRegManager()
{
#if TMW_USE_GATEWAY_DEFINES
#if _MSC_VER >= 1400
  for (size_t index = 0; index < WinIoSystemTime::m_arrRegTimeZoneInfo.size(); index++)
  {
    CRegTimeZoneInfo* pRegTimeZoneInfo = WinIoSystemTime::m_arrRegTimeZoneInfo[index];
    if (pRegTimeZoneInfo)
    {
      delete pRegTimeZoneInfo;
    }
  }
  if (WinIoSystemTime::g_pObjRegistryManager != NULL)
  {
    delete WinIoSystemTime::g_pObjRegistryManager;
  }
#endif
#endif
}


CRegistryManager *WinIoSystemTime::GetRegistryManager()
{
  if (g_pObjRegistryManager == NULL)
  {
    g_pObjRegistryManager = new CRegistryManager();
    EnumerateTimeZones();
  }
  return g_pObjRegistryManager;
}

void CRegTimeZoneInfo::FillFromRegistry(int i_RegistryIndex, BYTE* i_pValue, std::string &i_strValueName, DWORD i_ValueSize)
{
  i_RegistryIndex = i_RegistryIndex; // suppress unref param warning
  static int cnt =0;

  std::string strValueName(i_strValueName);
  std::transform(strValueName.begin(), strValueName.end(), strValueName.begin(), [](char c) { return (char)std::toupper(c); });

  if (!strValueName.compare(_T("Display")))
  {
    memcpy(m_szDisplay, i_pValue, i_ValueSize);
    cnt++;
  }

  if (!i_strValueName.compare(_T("Dlt")))
    memcpy(m_szDlt, i_pValue, i_ValueSize);

  if (!i_strValueName.compare(_T("Std")))
    memcpy(m_szStd, i_pValue, i_ValueSize);

  if (!i_strValueName.compare(_T("MapID")))
    memcpy(m_szMapID, i_pValue, i_ValueSize);

  if (!i_strValueName.compare(_T("Index")))
    memcpy((BYTE*)&m_iIndex, i_pValue, i_ValueSize);

  if (!i_strValueName.compare(_T("TZI")))
    memcpy((BYTE*)&m_regTZI, i_pValue, i_ValueSize);

}

/* function: getTZDateTime */
void WinIoSystemTime::getTZDateTime(TMWDTIME *pDateTime)
{
  SYSTEMTIME utcTime;
  SYSTEMTIME tzTime;
  GetSystemTime(&utcTime);


  TIME_ZONE_INFORMATION tzi;

  ZeroMemory(&tzi, sizeof(tzi));
  CRegTimeZoneInfo* pRegTimeZoneInfo = m_arrRegTimeZoneInfo[_timeZoneIndex];

  tzi.Bias            = pRegTimeZoneInfo->m_regTZI.Bias;
  if (_ignoreDST == true)
  {
    tzi.DaylightBias    = 0;
  }
  else
  {
    tzi.DaylightBias    = pRegTimeZoneInfo->m_regTZI.DaylightBias;
  }

  tzi.DaylightDate    = pRegTimeZoneInfo->m_regTZI.DaylightDate;
  tzi.StandardBias    = pRegTimeZoneInfo->m_regTZI.StandardBias;
  tzi.StandardDate    = pRegTimeZoneInfo->m_regTZI.StandardDate;
#if _UNICODE
  _tcscpy(tzi.StandardName, pRegTimeZoneInfo->m_szStd);
  _tcscpy(tzi.DaylightName, pRegTimeZoneInfo->m_szDlt);
#else
  MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pRegTimeZoneInfo->m_szStd , -1, tzi.StandardName, 32);
  MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pRegTimeZoneInfo->m_szDlt , -1, tzi.DaylightName , 32);
#endif
  //LogMessage("getTZDateTime: timezone index = %d\r\n",_timeZoneIndex);

  //LogMessage("getTZDateTime: utcTime = %d %d:%d:%d.%d\r\n", utcTime.wDay, utcTime.wHour, utcTime.wMinute, utcTime.wSecond, utcTime.wMilliseconds);

  if (SystemTimeToTzSpecificLocalTime(&tzi, &utcTime, &tzTime) == 0)
  {
    LogMessage("SystemTimeToTzSpecificLocalTime: convert date time failed (error =0x%08x)\r\n defaulting to local time\r\n", GetLastError());
    WinIoInterface::ProtoAnaLog(TMWDIAG_ID_TARGET,  "SystemTimeToTzSpecificLocalTime: convert date time failed (error =0x%08x)\r\n defaulting to local time\r\n", GetLastError());
    GetLocalTime(&tzTime);
  }

  //LogMessage("getTZDateTime: tzTime = %d %d:%d:%d.%d\r\n", tzTime.wDay, tzTime.wHour, tzTime.wMinute, tzTime.wSecond, tzTime.wMilliseconds);


  pDateTime->year = tzTime.wYear;
  pDateTime->month = (TMWTYPES_UCHAR)tzTime.wMonth;
  pDateTime->dayOfMonth = (TMWTYPES_UCHAR)tzTime.wDay;
  pDateTime->dayOfWeek = (TMWTYPES_UCHAR)tzTime.wDayOfWeek;
  if (tzTime.wDayOfWeek == 0)
  {
    pDateTime->dayOfWeek = 7;
  }
  pDateTime->hour = (TMWTYPES_UCHAR)tzTime.wHour;
  pDateTime->minutes = (TMWTYPES_UCHAR)tzTime.wMinute;
  pDateTime->mSecsAndSecs = tzTime.wSecond*1000 + tzTime.wMilliseconds;
  pDateTime->invalid = false;
  pDateTime->qualifier = TMWDTIME_UNKNOWN;

  if(tzi.DaylightBias != 0)
    pDateTime->dstInEffect = TMWDEFS_TRUE;
  else
    pDateTime->dstInEffect = TMWDEFS_FALSE;
}
#endif
#endif

void WinIoSystemTime::LogMessage(const char   *format, ...)
{

  if (WinIoTargProtoAnaLogFun != WINIOTARG_NULL)
  {
    va_list va;
    va_start(va, format);
    WinIoTargProtoAnaLogFun(NULL, TRACE_MASK_CUSTOM, 0, "", format, va);
    va_end(va);
  }
}

/* function: getDateTime */
void WinIoSystemTime::getDateTime(TMWDTIME *pDateTime)
{

#if TMW_USE_GATEWAY_DEFINES
#if _MSC_VER >= 1400
  if (
    m_timeMode == WINIO_TIME_MODE_SYSTEM_TIME_ZONE
    || m_timeMode == WINIO_TIME_MODE_SYSTEM_TIME_ZONE_NO_SETTIME
    )
  {
    return getTZDateTime(pDateTime);
  }
#endif
#endif

  SYSTEMTIME localTime;
  GetLocalTime(&localTime);
  pDateTime->year = localTime.wYear;
  pDateTime->month = (TMWTYPES_UCHAR)localTime.wMonth;
  pDateTime->dayOfMonth = (TMWTYPES_UCHAR)localTime.wDay;
  pDateTime->dayOfWeek = (TMWTYPES_UCHAR)localTime.wDayOfWeek;
  if (localTime.wDayOfWeek == 0)
  {
    pDateTime->dayOfWeek = 7;
  }
  pDateTime->hour = (TMWTYPES_UCHAR)localTime.wHour;
  pDateTime->minutes = (TMWTYPES_UCHAR)localTime.wMinute;
  pDateTime->mSecsAndSecs = localTime.wSecond*1000 + localTime.wMilliseconds;
  pDateTime->invalid = false;
  pDateTime->qualifier = TMWDTIME_UNKNOWN;
  
  TIME_ZONE_INFORMATION timeZoneInfo;
  if(GetTimeZoneInformation(&timeZoneInfo) == TIME_ZONE_ID_DAYLIGHT)
    pDateTime->dstInEffect = TMWDEFS_TRUE;
  else
    pDateTime->dstInEffect = TMWDEFS_FALSE;
}

/* function: getUTCDateTime */
void WinIoSystemTime::getUTCDateTime(TMWDTIME *pDateTime)
{
  SYSTEMTIME utcTime;
  GetSystemTime(&utcTime);
  pDateTime->year = utcTime.wYear;
  pDateTime->month = (TMWTYPES_UCHAR)utcTime.wMonth;
  pDateTime->dayOfMonth = (TMWTYPES_UCHAR)utcTime.wDay;
  pDateTime->dayOfWeek = (TMWTYPES_UCHAR)utcTime.wDayOfWeek;
  if (utcTime.wDayOfWeek == 0)
  {
    pDateTime->dayOfWeek = 7;
  }
  pDateTime->hour = (TMWTYPES_UCHAR)utcTime.wHour;
  pDateTime->minutes = (TMWTYPES_UCHAR)utcTime.wMinute;
  pDateTime->mSecsAndSecs = utcTime.wSecond*1000 + utcTime.wMilliseconds;
  pDateTime->invalid = false;
  pDateTime->qualifier = TMWDTIME_UNKNOWN;
  pDateTime->dstInEffect = TMWDEFS_FALSE;
}

/* function: setDateTime */
TMWTYPES_BOOL WinIoSystemTime::setDateTime(const TMWDTIME *pNewDateTime)
{
  if (
    m_timeMode == WINIO_TIME_MODE_SYSTEM
    || m_timeMode == WINIO_TIME_MODE_SYSTEM_TIME_ZONE
    )
  {
    SYSTEMTIME localTime;
  
    localTime.wYear = pNewDateTime->year;
    localTime.wMonth = pNewDateTime->month;
    localTime.wDay = pNewDateTime->dayOfMonth;
    localTime.wDayOfWeek = pNewDateTime->dayOfWeek;
    if (pNewDateTime->dayOfWeek == 7)
    {
      localTime.wDayOfWeek = 0;
    }
    localTime.wHour = pNewDateTime->hour;
    localTime.wMinute = pNewDateTime->minutes;
    localTime.wSecond = (pNewDateTime->mSecsAndSecs*60)/60000;
    localTime.wMilliseconds = pNewDateTime->mSecsAndSecs - (localTime.wSecond * 1000);

    BOOL bStatus = SetLocalTime(&localTime);
    if (bStatus == FALSE)
    {
      WinIoInterface::ProtoAnaLog(TMWDIAG_ID_TARGET,  "SYSTIME: set date time failed (error =0x%08x)\r\n", GetLastError());
      WINIO_ASSERT(FALSE);
      return false;
    }
  }
  return true;
}

/* function: setUTCDateTime */
TMWTYPES_BOOL WinIoSystemTime::setUTCDateTime(const TMWDTIME *pNewDateTime)
{
  SYSTEMTIME localTime;

  localTime.wYear = pNewDateTime->year;
  localTime.wMonth = pNewDateTime->month;
  localTime.wDay = pNewDateTime->dayOfMonth;
  localTime.wDayOfWeek = pNewDateTime->dayOfWeek;
  if (pNewDateTime->dayOfWeek == 7)
  {
    localTime.wDayOfWeek = 0;
  }
  localTime.wHour = pNewDateTime->hour;
  localTime.wMinute = pNewDateTime->minutes;
  localTime.wSecond = (pNewDateTime->mSecsAndSecs*60)/60000;
  localTime.wMilliseconds = pNewDateTime->mSecsAndSecs - (localTime.wSecond * 1000);

  BOOL bStatus = SetSystemTime(&localTime);
  if (bStatus == FALSE)
  {
    WinIoInterface::ProtoAnaLog(TMWDIAG_ID_TARGET,  "SYSTIME: set date time failed (error =0x%08x)\r\n", GetLastError());
    WINIO_ASSERT(FALSE);
    return false;
  }

  return true;
}


#if TMW_USE_GATEWAY_DEFINES

#if _MSC_VER >= 1400

/*
1. Open the registry key : "SOFTWARE\Microsoft\Windows NT\CurrentVersion\Time Zones
2. Read all sub keys
3. Fill all the TimeZone Structures.
*/
int WinIoSystemTime::EnumerateTimeZones()
{

    //1.Open the registry key "SOFTWARE\Microsoft\Windows NT\CurrentVersion\Time Zones"
    TCHAR szTimeZoneKey[] = _T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Time Zones");

    //CTimeZoneConverterApp* pApp = (CTimeZoneConverterApp*)AfxGetApp();
    //CRegistryManager* pRegistryManager = (CRegistryManager*)(&(pApp->m_objRegistryManager));

    HKEY hTimeZones;
    GetRegistryManager()->OpenRegistryKey(HKEY_LOCAL_MACHINE, szTimeZoneKey, 0, KEY_READ, &hTimeZones);

    //2.Read all sub keys
    GetRegistryManager()->EnumerateSubKeys(hTimeZones, m_arrTimeZones);

    //3. Fill all the TimeZone Structures.
    GetFullTimeZoneInfoFromRegistry();

    SortTimeZoneList();

    return 0;
}

int WinIoSystemTime::GetFullTimeZoneInfoFromRegistry() 
{
    TCHAR szParentTimeZoneKey[] = _T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Time Zones");

    //CTimeZoneConverterApp* pApp = (CTimeZoneConverterApp*)AfxGetApp();
    //CRegistryManager* pRegistryManager = (CRegistryManager*)(&(pApp->m_objRegistryManager));

    //Prepare key
    std::string szChildTimeZoneKey;

    char buf[256];
    for (size_t cnt = 0; cnt < m_arrTimeZones.size(); cnt++)
    {
      snprintf(buf, 256, "%s\\%s", szParentTimeZoneKey, m_arrTimeZones[cnt].c_str());
        //szChildTimeZoneKey.Format( _T("%s\\%s"), szParentTimeZoneKey, (LPCTSTR)m_arrTimeZones[cnt]);
      szChildTimeZoneKey = buf;

        //open child key
        HKEY hTimeZoneKey;
        GetRegistryManager()->OpenRegistryKey(HKEY_LOCAL_MACHINE, szChildTimeZoneKey.c_str(), 0, KEY_READ, &hTimeZoneKey);

        //enumerate all value names
        std::vector<std::string> keyValueNamesArray;
        GetRegistryManager()->EnumerateKeyValueNames(hTimeZoneKey, keyValueNamesArray);

        DWORD valueType, valueSize;
        BYTE value[512];
        
        CRegTimeZoneInfo* pobjRegTimeZoneInfo = new CRegTimeZoneInfo;
        ZeroMemory(pobjRegTimeZoneInfo, sizeof(CRegTimeZoneInfo));
        ZeroMemory((BYTE*)&pobjRegTimeZoneInfo->m_regTZI , sizeof(regTZI));

        for (size_t valname_cnt = 0; valname_cnt < keyValueNamesArray.size(); valname_cnt++)
        {
          std::string strValueName(keyValueNamesArray[valname_cnt]);
          std::transform(strValueName.begin(), strValueName.end(), strValueName.begin(), [](char c) { return (char)std::toupper(c); });

            if (!strValueName.compare(_T("TZI")))
            {
                valueSize = sizeof(regTZI);
            }
            else
            {
                valueSize = 512;
            }

             //now get the values and fill the structures
            GetRegistryManager()->QueryReistryValue(hTimeZoneKey, keyValueNamesArray[valname_cnt].c_str(), NULL,
                                        &valueType, value, &valueSize);
                

            pobjRegTimeZoneInfo->FillFromRegistry((int)valname_cnt, value, keyValueNamesArray[valname_cnt], valueSize);

            
        }

        m_arrRegTimeZoneInfo.push_back(pobjRegTimeZoneInfo);

    }

    return 0;
}

int WinIoSystemTime::SortTimeZoneList()
{
  std::sort(m_arrRegTimeZoneInfo.begin(), m_arrRegTimeZoneInfo.end(), TimeZoneComparer);
    //qsort(m_arrRegTimeZoneInfo.GetData(), m_arrRegTimeZoneInfo.GetSize(), sizeof(void*), TimeZoneComparer);
   
    return 0;
}

bool WinIoSystemTime::GetCurrentTimeZone(char *curTimeZone)
{
  TIME_ZONE_INFORMATION tzi;
  GetTimeZoneInformation(&tzi);

#if _UNICODE
  for ( int index = 0; index < m_arrRegTimeZoneInfo.GetSize(); index++ )
  {
    CRegTimeZoneInfo* pRegTimeZoneInfo = m_arrRegTimeZoneInfo[index];
    if ( 
      pRegTimeZoneInfo->m_regTZI.Bias == tzi.Bias 
      && pRegTimeZoneInfo->m_regTZI.StandardBias == tzi.StandardBias 
      && (_tcscmp(pRegTimeZoneInfo->m_szStd, tzi.StandardName) == 0)
       )
    {
      USES_CONVERSION; 
      strcpy(curTimeZone, T2A(tzi.StandardName));
      return true;
    }
  }
#else
  TCHAR standardName[512];
  WideCharToMultiByte( CP_ACP, 0, tzi.StandardName, -1,standardName, 256, NULL, NULL );

  //TCHAR dayLightName[512];
  //WideCharToMultiByte( CP_ACP, 0, tzi.DaylightName, -1,dayLightName, 256, NULL, NULL );

  for (size_t index = 0; index < m_arrRegTimeZoneInfo.size(); index++ )
  {
    CRegTimeZoneInfo* pRegTimeZoneInfo = m_arrRegTimeZoneInfo[index];
    if ( 
      pRegTimeZoneInfo->m_regTZI.Bias == tzi.Bias 
      && pRegTimeZoneInfo->m_regTZI.StandardBias == tzi.StandardBias 
      && (_tcscmp(pRegTimeZoneInfo->m_szStd, standardName) == 0)
       )
    {
      strcpy(curTimeZone,standardName);
      return true;
    }
  }
#endif

  strcpy(curTimeZone,"");
  return false;
}

bool WinIoSystemTime::GetTimeZone(int _index, char *returnedTimeZone)
{
  for (size_t index = 0; index < m_arrRegTimeZoneInfo.size(); index++ )
  {
    CRegTimeZoneInfo* pRegTimeZoneInfo = m_arrRegTimeZoneInfo[index];
    if (_index == (int)index)
    {
      USES_CONVERSION; 
      strcpy(returnedTimeZone,T2A(pRegTimeZoneInfo->m_szDisplay));
      return true;
    }
  }


  strcpy(returnedTimeZone,"");
  return false;
}

CRegTimeZoneInfo *WinIoSystemTime::GetTimeZoneStruct(int _index)
{
  for (size_t index = 0; index < m_arrRegTimeZoneInfo.size(); index++ )
  {
    CRegTimeZoneInfo* pRegTimeZoneInfo = m_arrRegTimeZoneInfo[index];
    if ((DWORD)_index == pRegTimeZoneInfo->m_iIndex)
    {
      return pRegTimeZoneInfo;
    }
  }

  return NULL;
}

int WinIoSystemTime::GetUTCTimeZone(const char *utcTZName)
{
  //USES_CONVERSION;
  //  TCHAR utcName[512];
  //  WCHAR *utc = A2W(utcTZName);

  //  WideCharToMultiByte( CP_ACP, 0, utc, -1, utcName, 256, NULL, NULL );

    //TCHAR utcDLT[512];
    //WideCharToMultiByte( CP_ACP, 0, utc, -1,utcDLT, 256, NULL, NULL );

    for (size_t index = 0; index < m_arrRegTimeZoneInfo.size(); index++ )
    {
        CRegTimeZoneInfo* pRegTimeZoneInfo = m_arrRegTimeZoneInfo[index];
#if _UNICODE
	    USES_CONVERSION;
        if  (_tcscmp(pRegTimeZoneInfo->m_szDisplay, A2T(utcTZName)) == 0)
#else
        if  (_tcscmp(pRegTimeZoneInfo->m_szDisplay, utcTZName) == 0)
#endif
        {
            return (int)index;
        }
    }

    for (size_t index = 0; index < m_arrRegTimeZoneInfo.size(); index++ )
    {
      CRegTimeZoneInfo* pRegTimeZoneInfo = m_arrRegTimeZoneInfo[index];
      if  (
           _tcsncmp(pRegTimeZoneInfo->m_szDisplay, _T("(UTC)"), 5) == 0
        || _tcsncmp(pRegTimeZoneInfo->m_szDisplay, _T("(GMT)"), 5) == 0
        )
      {
        return (int)index;
      }
    }

    return 0;
}

int WinIoSystemTime::GetTimeZoneIndex(const char *tzName)
{
  size_t index = 0;
  for ( index = 0; index < m_arrRegTimeZoneInfo.size(); index++ )
  {
    CRegTimeZoneInfo* pRegTimeZoneInfo = m_arrRegTimeZoneInfo[index];
#if _UNICODE
    USES_CONVERSION;
    if  (_tcscmp(pRegTimeZoneInfo->m_szDisplay, A2T(tzName)) == 0)
#else
    if  (_tcscmp(pRegTimeZoneInfo->m_szDisplay, tzName) == 0)
#endif
    {
      return (int)index;
    }
  }
  return -99;
}


void WinIoSystemTime::GetSpecificTimeFromUtcTime(TMWDTIME *utcTmwDtimeTime, TMWDTIME *specificTime, int timeZoneIndex, bool ignoreDST)
{
  SYSTEMTIME utcTime;
  SYSTEMTIME tzTime;
  utcTime.wYear         = utcTmwDtimeTime->year;
  utcTime.wMonth        = utcTmwDtimeTime->month;
  utcTime.wDayOfWeek    = utcTmwDtimeTime->dayOfWeek%7;
  utcTime.wDay          = utcTmwDtimeTime->dayOfMonth;
  utcTime.wHour         = utcTmwDtimeTime->hour;
  utcTime.wMinute       = utcTmwDtimeTime->minutes;
  utcTime.wSecond       = utcTmwDtimeTime->mSecsAndSecs/1000;
  utcTime.wMilliseconds = utcTmwDtimeTime->mSecsAndSecs%1000;

  TIME_ZONE_INFORMATION tzi;

  ZeroMemory(&tzi, sizeof(tzi));
  CRegTimeZoneInfo* pRegTimeZoneInfo = m_arrRegTimeZoneInfo[timeZoneIndex];
  if (pRegTimeZoneInfo == NULL)
  {
    WinIoInterface::ProtoAnaLog(TMWDIAG_ID_TARGET,  "GetSpecificTimeFromUtcTime: convert date time failed (error =0x%08x)\r\n defaulting to local time\r\n", GetLastError());
    GetLocalTime(&tzTime);
  }
  else
  {

    tzi.Bias = pRegTimeZoneInfo->m_regTZI.Bias;
    if (ignoreDST == true)
    {
      tzi.DaylightBias = 0;
    }
    else
    {
      tzi.DaylightBias = pRegTimeZoneInfo->m_regTZI.DaylightBias;
    }

    tzi.DaylightDate = pRegTimeZoneInfo->m_regTZI.DaylightDate;
    tzi.StandardBias = pRegTimeZoneInfo->m_regTZI.StandardBias;
    tzi.StandardDate = pRegTimeZoneInfo->m_regTZI.StandardDate;

#if _UNICODE
    _tcscpy(tzi.StandardName, pRegTimeZoneInfo->m_szStd);
    _tcscpy(tzi.DaylightName, pRegTimeZoneInfo->m_szDlt);
#else
    MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pRegTimeZoneInfo->m_szStd, -1, tzi.StandardName, 32);
    MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pRegTimeZoneInfo->m_szDlt, -1, tzi.DaylightName, 32);
#endif

    if (SystemTimeToTzSpecificLocalTime(&tzi, &utcTime, &tzTime) == 0)
    {
      WinIoInterface::ProtoAnaLog(TMWDIAG_ID_TARGET, "GetSpecificTimeFromUtcTime: convert date time failed (error =0x%08x)\r\n defaulting to local time\r\n", GetLastError());
      GetLocalTime(&tzTime);
    }
  }

  specificTime->year = tzTime.wYear;
  specificTime->month = (TMWTYPES_UCHAR)tzTime.wMonth;
  specificTime->dayOfMonth = (TMWTYPES_UCHAR)tzTime.wDay;
  specificTime->dayOfWeek = (TMWTYPES_UCHAR)tzTime.wDayOfWeek;
  if (tzTime.wDayOfWeek == 0)
  {
    specificTime->dayOfWeek = 7;
  }
  specificTime->hour = (TMWTYPES_UCHAR)tzTime.wHour;
  specificTime->minutes = (TMWTYPES_UCHAR)tzTime.wMinute;
  specificTime->mSecsAndSecs = tzTime.wSecond*1000 + tzTime.wMilliseconds;
  specificTime->invalid = utcTmwDtimeTime->invalid;
  specificTime->qualifier = utcTmwDtimeTime->qualifier;
  specificTime->tis = utcTmwDtimeTime->tis;
  specificTime->energyTariff = utcTmwDtimeTime->energyTariff;
  specificTime->powerTariff = utcTmwDtimeTime->powerTariff;
  specificTime->genuineTime = utcTmwDtimeTime->genuineTime;
  specificTime->pSession = utcTmwDtimeTime->pSession;

  if (tzi.DaylightBias != 0)
    specificTime->dstInEffect = TMWDEFS_TRUE;
  else
    specificTime->dstInEffect = TMWDEFS_FALSE;
  
}
#endif

#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CRegistryManager::CRegistryManager()
{

}



CRegistryManager::~CRegistryManager()
{

}



LONG CRegistryManager::OpenRegistryKey(HKEY hKey, LPCTSTR lpSubKey, DWORD ulOptions,
  REGSAM samDesired, PHKEY phkResult)
{

  return RegOpenKeyEx(hKey, lpSubKey, ulOptions, samDesired, phkResult);
}



LONG CRegistryManager::EnumerateSubKeys(HKEY hKey, std::vector<std::string> &o_subkeyArray)
{

  TCHAR    achKey[MAX_KEY_LENGTH];   // buffer for subkey name
  DWORD    cbName;                   // size of name string 
  TCHAR    achClass[MAX_PATH] = TEXT("");  // buffer for class name 
  DWORD    cchClassName = MAX_PATH;  // size of class string 
  DWORD    cSubKeys=0;               // number of subkeys 
  DWORD    cbMaxSubKey;              // longest subkey size 
  DWORD    cchMaxClass;              // longest class string 
  DWORD    cValues;              // number of values for key 
  DWORD    cchMaxValue;          // longest value name 
  DWORD    cbMaxValueData;       // longest value data 
  DWORD    cbSecurityDescriptor; // size of security descriptor 
  FILETIME ftLastWriteTime;      // last write time 

  DWORD i, retCode; 

  //TCHAR  achValue[MAX_VALUE_NAME]; 
  //DWORD cchValue = MAX_VALUE_NAME; 

  // Get the class name and the value count. 
  retCode = RegQueryInfoKey(
    hKey,                    // key handle 
    achClass,                // buffer for class name 
    &cchClassName,           // size of class string 
    NULL,                    // reserved 
    &cSubKeys,               // number of subkeys 
    &cbMaxSubKey,            // longest subkey size 
    &cchMaxClass,            // longest class string 
    &cValues,                // number of values for this key 
    &cchMaxValue,            // longest value name 
    &cbMaxValueData,         // longest value data 
    &cbSecurityDescriptor,   // security descriptor 
    &ftLastWriteTime);       // last write time 

  // Enumerate the subkeys, until RegEnumKeyEx fails.

  if (cSubKeys)
  {
    //printf( "\nNumber of subkeys: %d\n", cSubKeys);

    for (i=0; i<cSubKeys; i++) 
    { 
      cbName = MAX_KEY_LENGTH;
      retCode = RegEnumKeyEx(hKey, i,
        achKey, 
        &cbName, 
        NULL, 
        NULL, 
        NULL, 
        &ftLastWriteTime); 
      if (retCode == ERROR_SUCCESS) 
      {
        //_tprintf(TEXT("(%d) %s\n"), i+1, achKey);
        o_subkeyArray.push_back(achKey);

      }
    }
  } 

  return 0;
}

LONG CRegistryManager:: EnumerateKeyValueNames(HKEY hKey, std::vector<std::string> &o_keyValueNamesArray)
{
  // Enumerate the key values.
  // TCHAR    achKey[MAX_KEY_LENGTH];   // buffer for subkey name
  // DWORD    cbName;                   // size of name string 
  TCHAR    achClass[MAX_PATH] = TEXT("");  // buffer for class name 
  DWORD    cchClassName = MAX_PATH;  // size of class string 
  DWORD    cSubKeys=0;               // number of subkeys 
  DWORD    cbMaxSubKey;              // longest subkey size 
  DWORD    cchMaxClass;              // longest class string 
  DWORD    cValues;              // number of values for key 
  DWORD    cchMaxValue;          // longest value name 
  DWORD    cbMaxValueData;       // longest value data 
  DWORD    cbSecurityDescriptor; // size of security descriptor 
  FILETIME ftLastWriteTime;      // last write time 

  DWORD i, retCode; 

  TCHAR  achValue[MAX_VALUE_NAME]; 
  DWORD cchValue = MAX_VALUE_NAME; 

  // Get the class name and the value count. 
  retCode = RegQueryInfoKey(
    hKey,                    // key handle 
    achClass,                // buffer for class name 
    &cchClassName,           // size of class string 
    NULL,                    // reserved 
    &cSubKeys,               // number of subkeys 
    &cbMaxSubKey,            // longest subkey size 
    &cchMaxClass,            // longest class string 
    &cValues,                // number of values for this key 
    &cchMaxValue,            // longest value name 
    &cbMaxValueData,         // longest value data 
    &cbSecurityDescriptor,   // security descriptor 
    &ftLastWriteTime);       // last write time 


  if (cValues) 
  {
    //printf( "\nNumber of values: %d\n", cValues);

    for (i=0, retCode=ERROR_SUCCESS; i<cValues; i++) 
    { 
      cchValue = MAX_VALUE_NAME; 
      achValue[0] = '\0'; 
      retCode = RegEnumValue(hKey, i, 
        achValue, 
        &cchValue, 
        NULL, 
        NULL,
        NULL,
        NULL);

      if (retCode == ERROR_SUCCESS ) 
      { 
        o_keyValueNamesArray.push_back(achValue);
      } 
    }
  }

  return 0;
}


LONG CRegistryManager::QueryReistryValue( HKEY hKey,LPCTSTR lpValueName,LPDWORD lpReserved,
  LPDWORD lpType,LPBYTE lpData,LPDWORD lpcbData)
{
  return RegQueryValueEx(hKey, lpValueName, lpReserved, lpType, lpData, lpcbData);
}


bool WinIoSystemTime::strp_atoi(const char * & s, int & result, int low, int high, int offset)
{
  bool worked = false;
  char * end;
  unsigned long num = strtoul(s, & end, 10);
  if (num >= (unsigned long)low && num <= (unsigned long)high)
  {
    result = (int)(num + offset);
    s = end;
    worked = true;
  }
  return worked;
}

char * WinIoSystemTime::strptime(const char *s, const char *format, struct tm *tm, int *millisecs)
{
  bool working = true;
  while (working && *format && *s)
  {
    switch (*format)
    {
    case '%':
      {
        ++format;
        switch (*format)
        {
        case 'a':
        case 'A': // weekday name
          tm->tm_wday = -1;
          working = false;
          for (int i = 0; i < 7; ++ i)
          {
            size_t len = strlen(strp_weekdays[i]);
            if (!_strnicmp(strp_weekdays[i], s, len))
            {
              tm->tm_wday = i;
              s += len;
              working = true;
              break;
            }
            else if (!_strnicmp(strp_weekdays[i], s, 3))
            {
              tm->tm_wday = i;
              s += 3;
              working = true;
              break;
            }
          }
          break;
        case 'b':
        case 'B':
        case 'h': // month name
          tm->tm_mon = -1;
          working = false;
          for (int i = 0; i < 12; ++ i)
          {
            size_t len = strlen(strp_monthnames[i]);
            if (!_strnicmp(strp_monthnames[i], s, len))
            {
              tm->tm_mon = i;
              s += len;
              working = true;
              break;
            }
            else if (!_strnicmp(strp_monthnames[i], s, 3))
            {
              tm->tm_mon = i;
              s += 3;
              working = true;
              break;
            }
          }
          break;
        case 'd':
        case 'e': // day of month number
          working = strp_atoi(s, tm->tm_mday, 1, 31, 0);
          break;
        case 'D': // %m/%d/%y
          {
            const char * s_save = s;
            working = strp_atoi(s, tm->tm_mon, 1, 12, -1);
            if (working && *s == '/')
            {
              ++ s;
              working = strp_atoi(s, tm->tm_mday, 1, 31, 0);
              if (working && *s == '/')
              {
                ++ s;
                working = strp_atoi(s, tm->tm_year, 0, 99, 0);
                if (working && tm->tm_year < 69)
                  tm->tm_year += 100;
              }
            }
            if (!working)
              s = s_save;
          }
          break;
        case 'H': // hour
          working = strp_atoi(s, tm->tm_hour, 0, 23, 0);
          break;
        case 'I': // hour 12-hour clock
          working = strp_atoi(s, tm->tm_hour, 1, 12, 0);
          break;
        case 'j': // day number of year
          working = strp_atoi(s, tm->tm_yday, 1, 366, -1);
          break;
        case 'm': // month number
          working = strp_atoi(s, tm->tm_mon, 1, 12, -1);
          break;
        case 'M': // minute
          working = strp_atoi(s, tm->tm_min, 0, 59, 0);
          break;
        case 'n': // arbitrary whitespace
        case 't':
          while (isspace((int)*s)) 
            ++s;
          break;
        case 'p': // am / pm
          if (!_strnicmp(s, "am", 2))
          { // the hour will be 1 -> 12 maps to 12 am, 1 am .. 11 am, 12 noon 12 pm .. 11 pm
            if (tm->tm_hour == 12) // 12 am == 00 hours
              tm->tm_hour = 0;
          }
          else if (!_strnicmp(s, "pm", 2))
          {
            if (tm->tm_hour < 12) // 12 pm == 12 hours
              tm->tm_hour += 12; // 1 pm -> 13 hours, 11 pm -> 23 hours
          }
          else
            working = false;
          break;
        case 'r': // 12 hour clock %I:%M:%S %p
          {
            const char * s_save = s;
            working = strp_atoi(s, tm->tm_hour, 1, 12, 0);
            if (working && *s == ':')
            {
              ++ s;
              working = strp_atoi(s, tm->tm_min, 0, 59, 0);
              if (working && *s == ':')
              {
                ++ s;
                working = strp_atoi(s, tm->tm_sec, 0, 60, 0);
                if (working && isspace((int)*s))
                {
                  ++ s;
                  while (isspace((int)*s)) 
                    ++s;
                  if (!_strnicmp(s, "am", 2))
                  { // the hour will be 1 -> 12 maps to 12 am, 1 am .. 11 am, 12 noon 12 pm .. 11 pm
                    if (tm->tm_hour == 12) // 12 am == 00 hours
                      tm->tm_hour = 0;
                  }
                  else if (!_strnicmp(s, "pm", 2))
                  {
                    if (tm->tm_hour < 12) // 12 pm == 12 hours
                      tm->tm_hour += 12; // 1 pm -> 13 hours, 11 pm -> 23 hours
                  }
                  else
                    working = false;
                }
              }
            }
            if (!working)
              s = s_save;
          }
          break;
        case 'R': // %H:%M
          {
            const char * s_save = s;
            working = strp_atoi(s, tm->tm_hour, 0, 23, 0);
            if (working && *s == ':')
            {
              ++ s;
              working = strp_atoi(s, tm->tm_min, 0, 59, 0);
            }
            if (!working)
              s = s_save;
          }
          break;
        case 'S': // seconds
          working = strp_atoi(s, tm->tm_sec, 0, 60, 0);
          break;
        case 'U': // milli seconds 
          working = strp_atoi(s, *millisecs, 0, 999, 0);
          break;
        case 'T': // %H:%M:%S
          {
            const char * s_save = s;
            working = strp_atoi(s, tm->tm_hour, 0, 23, 0);
            if (working && *s == ':')
            {
              ++ s;
              working = strp_atoi(s, tm->tm_min, 0, 59, 0);
              if (working && *s == ':')
              {
                ++ s;
                working = strp_atoi(s, tm->tm_sec, 0, 60, 0);
              }
            }
            if (!working)
              s = s_save;
          }
          break;
        case 'w': // weekday number 0->6 sunday->saturday
          working = strp_atoi(s, tm->tm_wday, 0, 6, 0);
          break;
        case 'Y': // year
          working = strp_atoi(s, tm->tm_year, 1900, 65535, -1900);
          break;
        case 'y': // 2-digit year
          working = strp_atoi(s, tm->tm_year, 0, 99, 0);
          if (working && tm->tm_year < 69)
            tm->tm_year += 100;
          break;
        case '%': // escaped
          if (*s != '%')
            working = false;
          ++s;
          break;
        default:
          working = false;
        }
      }
      break;
    case ' ':
    case '\t':
    case '\r':
    case '\n':
    case '\f':
    case '\v':
      // zero or more whitespaces:
      while (isspace((int)*s))
        ++ s;
      break;
    default:
      // match character
      if (*s != *format)
        working = false;
      else
        ++s;
      break;
    }
    ++format;
  }
  return (working?(char *)s:0);
}
#endif // TMW_USE_GATEWAY_DEFINES