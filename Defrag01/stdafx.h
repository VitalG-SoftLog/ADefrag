/*
    Defrag Service header

    Module name:

        stdafx.h

    Abstract:

        Defrag Service main header. 
        Include file for standard system and application include files.

    $Header: /home/CVS_DEFRAG/Defrag/Src/Defrag01/stdafx.h,v 1.2 2009/11/24 15:15:10 dimas Exp $
    $Log: stdafx.h,v $
    Revision 1.2  2009/11/24 15:15:10  dimas
    Bugs 5.2, 6.*, 15.14 fixed

    Revision 1.1  2009/11/24 14:52:31  dimas
    no message

*/

#pragma once

#define DEFRAG_ENGINE_SERVICE

#define DECLSPEC_APP_IMPEX      __declspec(dllexport) 
#define DECLSPEC_ENGINE_IMPEX   __declspec(dllimport) 


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

//#define WIN32_32MS

#include <stdio.h>
#include <tchar.h>
#include <stdlib.h>
//#include <math.h>
//#include <crtdbg.h>

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



//#ifdef MAX_PATH
//    #undef MAX_PATH
//#endif
//#define MAX_PATH    512

#include "NtStruct.h"

#include "Debug.h"
#include "Unicode.h"
#include "SecUtils.h"
#include "SrvStub.h"

typedef struct _DEFRAG_JOB_CONTEXT      *PDEFRAG_JOB_CONTEXT;
typedef struct _DEFRAG_VOLUME_CONTEXT   *PDEFRAG_VOLUME_CONTEXT;
typedef struct _DEFRAG_FILE_CONTEXT     *PDEFRAG_FILE_CONTEXT;
typedef struct _DEFRAG_STREAM_CONTEXT   *PDEFRAG_STREAM_CONTEXT;

#include "BNode.h"
#include "FatUtils.h"
#include "NtfsUtils.h"

#include "Api.h"

#include "EngineApi.h"
#include "EngineCore.h"
//#include "DefragUtils.h"
//#include "HelperUtils.h"
//#include "FileExContext.h"
//#include "Statistic.h"
//#include "DisplayMap.h"

#include "Defrag01.h"
#include "JobManager.h"
#include "Settings.h"
#include "IoUtils.h"
#include "Client.h"
#include "Scheduler.h"

#ifndef ASSERT
    #define ASSERT _ASSERT
#endif

#ifndef KERNEL
    //#define     malloc( _size_ )    LocalAlloc( LPTR, _size_ )
    //#define     free( _ptr_ )       LocalFree( _ptr_ )
    //#define     malloc( _size_ )    VirtualAlloc( NULL, _size_, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE )
    //#define     free( _ptr_ )       VirtualFree( _ptr_, 0, MEM_RELEASE )
    //#define     malloc( _size_ )    HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, _size_ )
    //#define     free( _ptr_ )       HeapFree( GetProcessHeap(), 0, _ptr_ )
#endif


