/*
    Defrag Engine Service main module header

    Module name:

        Defrag01.h

    Abstract:

        Defrag Engine Service main module header.
        Defines service specific constants, types, structures, functions, etc.

    $Header: /home/CVS_DEFRAG/Defrag/Src/Defrag01/Defrag01.h,v 1.2 2009/12/02 14:43:40 dimas Exp $
    $Log: Defrag01.h,v $
    Revision 1.2  2009/12/02 14:43:40  dimas
    App/Service interaction improved

    Revision 1.1  2009/11/24 14:52:31  dimas
    no message

*/

#ifndef __DEFRAG_MAIN_H__
#define __DEFRAG_MAIN_H__



// =================================================
//
// Constants, Macros & Data
//
// =================================================

//
// Operation modes 
//
#define APP_STAT_INIT                           0x00000001
#define APP_STAT_JOB_MANAGER_STARTED            0x00000002
#define APP_STAT_SHEDULER_TASK_MANAGER_ACTIVED  0x00000004

#define APP_STAT_JOB_MANAGER_STOPPED            0x40000000
#define APP_STAT_UNINIT                         0x80000000

//
// Operation flags
//
//#define FLAG_CREATE_NEW_FILE        0x00000001
//#define FLAG_OPEN_EXISTEN_FILE      0x00000002
//#define FLAG_DIRECTORY_FILE         0x00000010
//#define FLAG_DELETE_ON_EXIT         0x00000100
#define FLAG_REPEAT_IN_LOOP         0x00001000


//
// Structures definitions
//
typedef struct _DEFRAG_GLOBAL {
    //
    // Common State
    //
    DWORD                       AppId;
    WCHAR                       AppName[MAX_PATH];
    DWORD                       AppStatus;
    DWORD                       JobAction;
    DWORD                       RetCode;
    OSVERSIONINFO               OsVersion;

    //
    // Defrag State
    //
    CRITICAL_SECTION            JobListSync;
    struct _DEFRAG_JOB_CONTEXT  *JobList;
    DWORD                       Flags;

    //
    // Service State
    //
    DWORD                       ServiceAction;
    SERVICE_STATUS              ServiceStatus;
    SERVICE_STATUS_HANDLE       ServiceStatusHandle;
    HANDLE                      ObjectToWait;
    HANDLE                      TerminateSheduller;
    HANDLE                      SheduleThread;
    HANDLE                      ShedulerTaskManagerThread;
    HANDLE                      ShedulerTaskManagerStopEvent;
    HANDLE                      ShedulerTaskManagerUpdateEvent;

    //
    // Server Communication structures
    //
    HANDLE                      ServerPort;
    HANDLE                      ClientPort; 
    HANDLE                      ServerSectionHandle; 
    PBYTE                       ServerBuffer;
    DWORD                       BufferLength;

    //
    // Client Communication structures
    //
    CRITICAL_SECTION            CommSync;
    HANDLE                      PortHandle;
    HANDLE                      ClientSectionHandle; 
    PBYTE                       ClientBuffer;

} DEFRAG_GLOBAL, *PDEFRAG_GLOBAL;

extern DEFRAG_GLOBAL      Global;


// =================================================
//
// Function prototypes
//
// =================================================

int
InitDefrag(
    int     argc, 
    WCHAR*  argv[] );

void
UninitDefrag( void );


#endif // !__DEFRAG_MAIN_H__

