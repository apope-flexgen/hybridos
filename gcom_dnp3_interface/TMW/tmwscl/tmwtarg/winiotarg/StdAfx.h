// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#pragma once

#define __STR2__(x) #x
#define __STR1__(x) __STR2__(x)
#define __LOC__ __FILE__ "(" __STR1__(__LINE__) ") : Note: "

#if _MSC_VER >= 1300
#define WINVER 0x0600
#endif

// Insert your headers here
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#define _WIN32_DCOM

#define __IPHLPAPI_H__      // Prevent inclusion of the problematic ntddndis.h

#define _AFXDLL 1
#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC Automation classes
#include <afxdtctl.h>		    // MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			    // MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#define _CRTDBG_MAP_ALLOC  
#include <stdlib.h>  
#include <crtdbg.h>  

#ifdef _DEBUG
#define DEBUG_CLIENTBLOCK   new( _CLIENT_BLOCK, __FILE__, __LINE__)
#else
#define DEBUG_CLIENTBLOCK
#endif // _DEBUG


#include <process.h>

#include <time.h>
#include <winsock2.h>  // socket API
#include <WS2tcpip.h>

#include <sys\timeb.h>

	
/* To support building when UNICODE is not available */
#ifndef USES_CONVERSION 
#define USES_CONVERSION
#define T2A(text)     text 
#define A2T(text)     text 
#endif


// TODO: reference additional headers your program requires here

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

