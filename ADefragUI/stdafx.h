/*
    Defrag GUI header

    Module name:

        stdafx.h

    Abstract:

        Defrag GUI main header. 
        Include file for standard system and application include files.

    $Header: /home/CVS_DEFRAG/Defrag/Src/ADefragUI/stdafx.h,v 1.3 2009/12/03 15:36:55 dimas Exp $
    $Log: stdafx.h,v $
    Revision 1.3  2009/12/03 15:36:55  dimas
    CVS headers included

*/


#pragma once

#define DEFRAG_GUI_APP

#ifndef _SECURE_ATL
#define _SECURE_ATL 1
#endif

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // Exclude rarely-used stuff from Windows headers
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit

// turns off MFC's hiding of some common and often safely ignored warning messages
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions

#include <afxole.h>         // MFC OLE classes
#include <afxodlgs.h>       // MFC OLE dialog classes

#include <afxdisp.h>        // MFC Automation classes



#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // MFC support for Internet Explorer 4 Common Controls
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>             // MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxcontrolbars.h>     // MFC support for ribbons and control bars
#include <afxcview.h>

#include <memory>
#include <vector>

#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif

#include <windows.h>
#include <winsvc.h>
#include <winternl.h>
#include <Winioctl.h>

//
// To avoid conflict with winternl
//
#define _NTDEF_
typedef LONG NTSTATUS, *PNTSTATUS;

#include <ntsecapi.h>

#include <lm.h>

#include "NtStruct.h"

#include "Debug.h"
#include "Api.h"
#include "Client.h"
#include "ColorManager\ColorManager.h"
#include "globalDefs.h"
#include <GdiPlus.h>
#include <afxhtml.h>
using namespace Gdiplus;

typedef struct _DEFRAG_GUI_JOB_CTX {

    DWORD                       Type;
    WCHAR                       DiskName[20];

    struct _DEFRAG_GUI_JOB_CTX  *NextJob;

} DEFRAG_GUI_JOB_CTX, *PDEFRAG_GUI_JOB_CTX;



//
// Structures definitions
//
typedef struct _DEFRAG_GUI_GLOBAL {

    PDEFRAG_GUI_JOB_CTX         JobList;

    //
    // Application / Job type
    //
    DWORD                       AppId;

    //
    // Communication structures
    //
    CRITICAL_SECTION            CommSync;
    //
    // Server side
    //
    PBYTE                       ServerBuffer;
    DWORD                       BufferLength;
    //
    // Client side
    //
    HANDLE                      PortHandle;
    HANDLE                      ClientSectionHandle; 
    PBYTE                       ClientBuffer;
    //
    //
    //    
    CSSSColorManager            ColorManager;


} DEFRAG_GUI_GLOBAL, *PDEFRAG_GUI_GLOBAL;

extern DEFRAG_GUI_GLOBAL    Global;



