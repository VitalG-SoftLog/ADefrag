/*
    Defrag Service header

    Module name:

        stdafx.h

    Abstract:

        Defrag Service main header. 
        Include file for standard system and application include files.

    $Header: /home/CVS_DEFRAG/Defrag/Src/BootDfrg/stdafx.h,v 1.6 2009/12/24 10:52:20 dimas Exp $
    $Log: stdafx.h,v $
    Revision 1.6  2009/12/24 10:52:20  dimas
    Check against Exclude Files list implemented

    Revision 1.5  2009/11/30 12:48:36  dimas
    1. Smart modes improved.
    2. FAT processing improved.

    Revision 1.4  2009/11/26 15:56:54  dimas
    1. Smart modes improved.
    2. File names support improved.

    Revision 1.3  2009/11/24 17:43:24  roman
    Initial commit

    Revision 1.2  2009/11/24 15:15:10  dimas
    Bugs 5.2, 6.*, 15.14 fixed

    Revision 1.1  2009/11/24 14:52:45  dimas
    no message

*/

#pragma once

#define DEFRAG_NATIVE_APP

#define DECLSPEC_APP_IMPEX      // __declspec(dllexport) 
#define DECLSPEC_ENGINE_IMPEX   // __declspec(dllimport)


// The following macros define the minimum required platform.  The minimum required platform
// is the earliest version of Windows, Internet Explorer etc. that has the necessary features to run 
// your application.  The macros work by enabling all features available on platform versions up to and 
// including the version specified.

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER
    #define WINVER 0x0501           // Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT
    #define _WIN32_WINNT 0x0501     // Change this to the appropriate value to target other versions of Windows.
#endif

//#ifndef _WIN32_WINDOWS            // Specifies that the minimum required platform is Windows 98.
//    #define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
//#endif
//
//#ifndef _WIN32_IE                 // Specifies that the minimum required platform is Internet Explorer 7.0.
//    #define _WIN32_IE 0x0700      // Change this to the appropriate value to target other versions of IE.
//#endif

#define WIN32_LEAN_AND_MEAN         // Exclude rarely-used stuff from Windows headers


#ifndef UNICODE
    #define UNICODE
#endif

#ifndef _UNICODE
    #define _UNICODE
#endif



//#include <ntddk.h>
#include <ntifs.h>

#include <stdio.h>
#include <tchar.h>
#include <stdlib.h>
//#include <crtdbg.h>


#include <winerror.h>

/*
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
//#include <LMCons.h>
//#include <LMaccess.h>
//#include <AclApi.h>
//#include <Sddl.h>
*/

//
// Missing definittions
//
typedef unsigned char   BYTE, *PBYTE;
typedef unsigned short  WORD, *PWORD;
typedef unsigned int    DWORD, *PDWORD;
typedef int             BOOL;

typedef HANDLE          HWND;
typedef HANDLE          HMODULE;


#ifdef MAX_PATH
    #undef MAX_PATH
#endif
#define MAX_PATH    260

#ifndef INVALID_HANDLE_VALUE
    #define INVALID_HANDLE_VALUE ((HANDLE)(LONG_PTR)-1)
    #define INFINITE            0xFFFFFFFF  // Infinite timeout
#endif

#ifndef NO_ERROR
    #define NO_ERROR        0
#endif 

//
// CRT / Windows to Native conversion
//
#ifdef KERNEL
    #define     malloc( _size_ )    RtlAllocateHeap( Global.Heap, 0, _size_ )
    #define     free( _ptr_ )       RtlFreeHeap( Global.Heap, 0, _ptr_ )
    #define     CRITICAL_SECTION    HANDLE // ???
    #define     InitializeCriticalSection( SectionPtr )
    #define     EnterCriticalSection( SectionPtr )
    #define     LeaveCriticalSection( SectionPtr )
    #define     DeleteCriticalSection( SectionPtr )
    #define     WaitForSingleObject( Handle, Timeout )
    #define     CreateEvent( SecAttr, Manual, Init, Name ) NULL
    #define     SetShedulerEvent() 

#endif


//#include "NtStruct.h"

#include "Debug.h"
#include "Unicode.h"

typedef struct _DEFRAG_JOB_CONTEXT      *PDEFRAG_JOB_CONTEXT;
typedef struct _DEFRAG_VOLUME_CONTEXT   *PDEFRAG_VOLUME_CONTEXT;
typedef struct _DEFRAG_FILE_CONTEXT     *PDEFRAG_FILE_CONTEXT;
typedef struct _DEFRAG_STREAM_CONTEXT   *PDEFRAG_STREAM_CONTEXT;

#include "BNode.h"
#include "FatUtils.h"
#include "NtfsUtils.h"

#if defined(USE_LPC6432)
    #define LPC_CLIENT_ID   CLIENT_ID64
    #define LPC_SIZE_T      ULONGLONG
    #define LPC_PVOID       ULONGLONG
    #define LPC_HANDLE      ULONGLONG
#else
    #define LPC_CLIENT_ID   CLIENT_ID
    #define LPC_SIZE_T      SIZE_T
    #define LPC_PVOID       PVOID
    #define LPC_HANDLE      HANDLE
#endif


#include "Api.h"

#include "EngineApi.h"
#include "EngineCore.h"
#include "DefragUtils.h"
#include "DefragSmartUtils.h"
#include "DefragFreeUtils.h"
#include "HelperUtils.h"
#include "FileExContext.h"
#include "FileNames.h"
#include "Statistic.h"
#include "DisplayMap.h"

#include "BootDfrg.h"
#include "JobManager.h"
#include "Settings.h"
#include "IoUtils.h"
//#include "Client.h"

