/*
    Defrag Native App (Boot Defrag) main module 

    Module name:

        BootDfrg.cpp

    Abstract:

        Defrag Native App (Boot Defrag) main module.

    $Header: /home/CVS_DEFRAG/Defrag/Src/BootDfrg/BootDfrg.cpp,v 1.3 2009/12/24 15:02:51 dimas Exp $
    $Log: BootDfrg.cpp,v $
    Revision 1.3  2009/12/24 15:02:51  dimas
    Some cosmetic improvements

    Revision 1.2  2009/12/21 17:00:13  dimas
    1. DEFRAG_CMD_GET_MOST_FRAGMENTED and similar requests implemented
    2. Basic Exclude files check added

    Revision 1.1  2009/11/24 14:52:45  dimas
    no message

*/


#include "stdafx.h"

#define     PRESS_KBD_TIMEOUT   10

DEFRAG_GLOBAL      Global;



//
// IO_APC_ROUTINE - Read Kbd (GetInput) Callback routine
//
VOID
KbdInput(
    IN  PVOID               ApcContext,
    IN  PIO_STATUS_BLOCK    IoStatus,
    IN  ULONG               Reserved )
{
    NTSTATUS                Status = STATUS_SUCCESS;
    PDEFRAG_KBD             Kbd = (PDEFRAG_KBD)ApcContext;


#ifdef _DEBUGK
    WCHAR       Mess[MAX_PATH];
    swprintf( Mess, L"KbdInput(). Kbd %d Key pressed. Status %08X read %d bytes = %d", 
        Kbd->Num, IoStatus->Status, IoStatus->Information, sizeof(KEYBOARD_INPUT_DATA) );
    OutMessage( Mess );
#endif

    if ( Global.Kbd[0].ObjToWait && 
        NT_SUCCESS( IoStatus->Status ) )
    {
        Kbd->CurPos.QuadPart += IoStatus->Information;

        if ( sizeof(KEYBOARD_INPUT_DATA) == IoStatus->Information &&
            (Global.KbdBuffer.Flags & KEY_BREAK) )
        {
            Status = NtSetEvent( 
                        Global.Kbd[0].ObjToWait, 
                        NULL );
        }
        else
        {
            Status = ReadKbd( Kbd->Num, IoStatus );
        }
    }

} // end of KbdInput


NTSTATUS
ReadKbd(
    int                     i,
    PIO_STATUS_BLOCK        IoStatus )
{
    NTSTATUS                Status = STATUS_SUCCESS;

    Status = NtReadFile(
                    Global.Kbd[i].Handle,
                    NULL,           // Event,      //   OPTIONAL
                    KbdInput,       // ApcRoutine, //   OPTIONAL
                    &(Global.Kbd[i]), // ApcContext, //   OPTIONAL
                    IoStatus,
                    &(Global.KbdBuffer),
                    sizeof(Global.KbdBuffer),
                    &(Global.Kbd[i].CurPos), // IN PLARGE_INTEGER  ByteOffset  OPTIONAL,
                    NULL );         //IN PULONG  Key  OPTIONAL );

    return Status;

} // end of ReadKbd


int 
GetInput( 
    void )
{
    NTSTATUS                Status = STATUS_SUCCESS;

    IO_STATUS_BLOCK         IoStatus = { 0 };
    //KEYBOARD_INPUT_DATA     KbdBuffer = { 0 };

    OBJECT_ATTRIBUTES       ObjectAttributes;
    //HANDLE                  Kbd.ObjToWait[MAX_KBD_NUM];
    DWORD                   ObjToWaitCnt = 0;
    int                     i, Choise = -1;
    BOOLEAN                 NeedToWait = FALSE;



    __try
    {
        //
        // Create event
        //
        InitializeObjectAttributes( &ObjectAttributes, NULL, 0, NULL, NULL );

        Status = NtCreateEvent( 
                        &(Global.Kbd[0].ObjToWait),
                        STANDARD_RIGHTS_REQUIRED | SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA,
                        &ObjectAttributes,
                        NotificationEvent, // SynchronizationEvent,
                        FALSE );

        if ( NT_SUCCESS( Status ) )
        {
             ObjToWaitCnt++;
        }
        else
        {
            __leave;
        }


        for ( i=0; i<MAX_KBD_NUM; i++ )
        {
            Global.Kbd[i].Num = i;

            if ( Global.Kbd[i].Handle )
            {
                //Kbd.ObjToWait[i] = Global.Kbd[i].Handle;
                //ObjToWaitCnt++;

                //
                // Flush Kbd buffer
                //
                Status = NtFlushBuffersFile(
                                Global.Kbd[i].Handle,
                                &IoStatus );

                Status = ReadKbd( i, &IoStatus );

                if ( STATUS_PENDING == Status ) 
                {
                    NeedToWait = TRUE;
#ifdef _DEBUGK
                    WCHAR       Mess[MAX_PATH];
                    swprintf( Mess, L"   Read  on Kbd %d is PENDING", i );
                    OutMessage( Mess );
#endif
                    continue;
                }
                else if ( NT_SUCCESS( Status ) )
                {
                    //Kbd.ObjToWait[ObjToWaitCnt] = Global.Kbd[i].Handle;
                    //ObjToWaitCnt++;
                    Global.Kbd[i].CurPos.QuadPart += IoStatus.Information;
#ifdef _DEBUGK
                    WCHAR       Mess[MAX_PATH];
                    swprintf( Mess, L"   Read on Kbd %d SUCCEED, read %d bytes - Key %ls", 
                        i, IoStatus.Information,  (Global.KbdBuffer.Flags & KEY_BREAK) ? L"released":L"pressed" );
                    OutMessage( Mess );
#endif
                    break;
                }
#ifdef _DEBUGK
                else
                {
                    WCHAR       Mess[MAX_PATH];
                    swprintf( Mess, L"GetInput(). Read on Kbd%d error", i );
                    DislpayErrorMessage( Mess, Status, ERR_NT_STATUS );
                    continue;
                }
#endif
            } // if ( Handle

        } // for ( i


        if ( NeedToWait && ObjToWaitCnt )
        {
            Status = NtResetEvent( 
                        Global.Kbd[0].ObjToWait, 
                        NULL ); //&PrevState );

            OutMessage( L"Press any key > " );

            LARGE_INTEGER   Timeout = { 0 };

            Timeout.QuadPart = (LONGLONG)(-(PRESS_KBD_TIMEOUT)) * 1000000 * 10;

            while( TRUE )
            {
            //Status = NtWaitForMultipleObjects(
            //                ObjToWaitCnt,
            //                &(Global.Kbd[0].ObjToWait),
            //                WaitAny,
            //                TRUE,
            //                &Timeout );
            Status = NtWaitForSingleObject(
                            Global.Kbd[0].ObjToWait,
                            TRUE,
                            &Timeout );

            switch ( Status )
            {
            case STATUS_TIMEOUT:

#ifdef _DEBUGK
                OutMessage( L"Timeout. Key not pressed" );
#endif
                __leave;
                break;

            //case STATUS_SUCCESS:
            case STATUS_WAIT_0:
#ifdef _DEBUGK
                WCHAR       Mess[MAX_PATH];
                swprintf( Mess, L"   Key %ls", (Global.KbdBuffer.Flags & KEY_BREAK) ? L"released":L"pressed" );
                OutMessage( Mess );
#endif
                __leave;
                break;

            default:
                //swprintf( Mess, L"Wait terminated. Status %08X", Status ); // 0x000000C0L
                //OutMessage( Mess );
                break;
            } // switch
            } // while
        } // if ( wait
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        Status = STATUS_INVALID_PARAMETER;
        DislpayErrorMessage( L"GetInput. Exception !!!", Status, ERR_OS );
    }


    for ( i=0; i<MAX_KBD_NUM; i++ )
    {
        if ( Global.Kbd[i].Handle )
        {
            Status = NtCancelIoFile( Global.Kbd[i].Handle, &IoStatus );
        }
    }


    for ( i=0; i<MAX_KBD_NUM; i++ )
    {
        if ( Global.Kbd[i].ObjToWait )
        {
            NtClose( Global.Kbd[i].ObjToWait );
            Global.Kbd[i].ObjToWait = NULL;
        }
    }


    return Choise;

} // end of GetInput


int
OutMessage( 
    WCHAR   *UserMess )
//    DWORD   Mode )
{
    WCHAR                   MessBuf[MAX_PATH];
    UNICODE_STRING          Mess = { 0 };

    wcscpy( MessBuf, UserMess );
    wcscat( MessBuf, L"\n" );

    Mess.Buffer = MessBuf;
    Mess.Length = (USHORT)(wcslen( MessBuf ) * sizeof(WCHAR));
    Mess.MaximumLength = Mess.Length + sizeof(WCHAR);

    //
    // Print 
    //
    NtDisplayString( &Mess );


    return STATUS_SUCCESS;

} // end of OutMessage


int 
DislpayErrorMessage( 
    const WCHAR *UserMess, 
    int         UserErr, 
    int         ErrorType,
    HWND        Parent,
    BOOLEAN     Suppress )
{
    WCHAR       Mess[MAX_PATH];


    if ( !Suppress )
    {
        swprintf( Mess, L"%ls. NtStat %08X", UserMess, UserErr );
        OutMessage( Mess );
    }

    return UserErr;

} // end of DislpayErrorMessage


NTSTATUS
DfrgBOpenKbd(
    void )
{
    NTSTATUS            Status = STATUS_INVALID_PARAMETER;

    OBJECT_ATTRIBUTES   ObjectAttributes;
    IO_STATUS_BLOCK     IoStatus;
    UNICODE_STRING      UniName = { 0 };
    int                 i;

#ifdef _DEBUG
    WCHAR       Mess[MAX_PATH];
#endif

    __try
    {
        //
        // Open all Keyboards
        //
        // TODO: do NOT use this hardcoded name(s). On XP and later, you are more then 
        //   likely opening the TS keyboard redirector driver. You should use 
        //   IoGetDeviceInterfaces and enumerate the keyboard device interface and read 
        //   from all the keyboards (there can be quite a few of them). The actual 
        //   keyboard guid is in ntddkbd.h (also see the kbdclass sources for it's 
        //   usage).
        //
        UniInitUnicodeString( &UniName, L"\\Device\\KeyboardClass0" );

        for ( i=0; i<MAX_KBD_NUM; i++ )
        {
            Global.Kbd[i].Handle = NULL;
            Global.Kbd[i].ObjToWait = NULL;
        }

        for ( i=0; i<MAX_KBD_NUM; i++ )
        {
            UniName.Buffer[UniName.Length/sizeof(WCHAR)-1] = L'0' + i;

            InitializeObjectAttributes( &ObjectAttributes, &UniName, OBJ_CASE_INSENSITIVE, NULL, NULL );

            Status = NtCreateFile(
                         &(Global.Kbd[i].Handle),
                         GENERIC_READ | GENERIC_WRITE | FILE_READ_ATTRIBUTES, // | SYNCHRONIZE, //  | FILE_READ_ATTRIBUTES | READ_CONTROL | SYNCHRONIZE, //GENERIC_READ | READ_CONTROL | SYNCHRONIZE,
                         &ObjectAttributes,
                         &IoStatus,
                         NULL,                          // allocation size
                         FILE_ATTRIBUTE_NORMAL,
                         0, // FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                         FILE_OPEN,
                         FILE_DIRECTORY_FILE, // FILE_DIRECTORY_FILE, //0, //FILE_SYNCHRONOUS_IO_NONALERT,  // | FILE_DIRECTORY_FILE
                         NULL,
                         0 );

            if ( !NT_SUCCESS( Status ) )
            {
#ifdef _DEBUG
                swprintf( Mess, L"DfrgOpenKbd '%s' NtCreateFile error", UniName.Buffer );
                DislpayErrorMessage( Mess, Status, ERR_NT_STATUS );
#endif
                if ( STATUS_OBJECT_NAME_NOT_FOUND == Status ) // = 34, STATUS_ACCESS_DENIED = 22
                {
                    // igloz: "if" clause commented out to allow usage on machines with no keyboard
                    //if ( i > 0 )
                    //{
                        Status = STATUS_SUCCESS;
                    //}
                    __leave;
                }
            }
#ifdef _DEBUG
            else
            {
                swprintf( Mess, L"DfrgOpenKbd '%s' OK - Handle %08X", UniName.Buffer, Global.Kbd[i].Handle );
                OutMessage( Mess );
            } // if Success
#endif
        } // for ( kbd[i]
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        Status = STATUS_INVALID_PARAMETER;
        DislpayErrorMessage( L"DfrgOpenKbd. Exception !!!", Status, ERR_OS );
    }

    UniFreeUnicodeString( &UniName );

    return Status;

} // end of DfrgBOpenKbd


NTSTATUS
DfrgBInit(
    PSTARTUP_ARGUMENT   Argument ) // treat as PVOID
{
    NTSTATUS                Status = STATUS_INVALID_PARAMETER; 

    //PUNICODE_STRING         CommandLine = NULL;
    PWCHAR                  StringBuffer = NULL, ArgPtr = NULL;
    RTL_HEAP_PARAMETERS     HeapParameters;

    __try
    {
        //
        // Initialize some heap
        //
        memset( &HeapParameters, 0, sizeof( RTL_HEAP_PARAMETERS  ));
        HeapParameters.Length = sizeof( RTL_HEAP_PARAMETERS  );

        Global.Heap = RtlCreateHeap( 
                        HEAP_GROWABLE, 
                        NULL, 
                        0x100000, 
                        0x1000, 
                        NULL, 
                        &HeapParameters );
        if ( !Global.Heap )
        {
            DislpayErrorMessage( L"DfrgBInit. RtlCreateHeap() error", STATUS_SUCCESS, ERR_NT_STATUS );
            __leave;
        }

        //
        // Init User Input
        //
        Status = DfrgBOpenKbd();
        //if ( !NT_SUCCESS( Status ) )
        //{
        //    __leave;
        //}

#ifdef _DEBUG
        //
        // Print the command line argument
        //

        //CommandLine = &Argument->Environment->CommandLine;
        //ArgPtr = CommandLine->Buffer;

        PPEB Peb = (PPEB)Argument; 
        RtlNormalizeProcessParams( Peb->ProcessParameters ); 

        ArgPtr = Peb->ProcessParameters->CommandLine.Buffer; 
        //while( *ArgPtr != L' ' ) 
        //{
        //    ArgPtr++;
        //}
        //ArgPtr++;

        //
        // Allocate string
        //
        StringBuffer = (PWCHAR) malloc( 256 );
        if ( !StringBuffer )
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            __leave;
        }

        //
        // Format string
        //
        swprintf( StringBuffer, L"\nBootDfrg %ls", ArgPtr ? ArgPtr : L"NULL" );
        OutMessage( StringBuffer );
#endif

        InitializeCriticalSection( &Global.JobListSync );

        //
        // Initialize settings
        //
        DfrgInitializeSettings( NULL );

        //
        // Set Service Process Privileges
        //
        //Result = SetProcessAdminPrivilege();
        //if ( NO_ERROR != Result )
        //{
        //    DislpayErrorMessage( L"InitDefrag(). Set Priv Error", Result, ERR_OS );
        //    __leave;
        //}

    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        Status = STATUS_INVALID_PARAMETER;
        DislpayErrorMessage( L"DfrgOpenKbd. Exception !!!", Status, ERR_OS );
    }


    //
    // Free heap
    //
    if ( StringBuffer )
    {
        free( StringBuffer );
    }


    return Status;

} // end of DfrgBInit


NTSTATUS
DfrgBUninit(
    void )
{
    NTSTATUS            Status = STATUS_INVALID_PARAMETER;


    __try
    {
        //
        // Delete all jobs
        //
        DfrgFreeJobCtx( Global.JobList );
        Global.JobList = NULL;

        DeleteCriticalSection( &Global.JobListSync );


        //
        // Uninitialize settings
        //
        DfrgUninitializeSettings();

        //
        // Close Kbd
        //
        for( int i=0; i<MAX_KBD_NUM; i++ )
        {
            if ( Global.Kbd[i].Handle )
            {
                NtClose( Global.Kbd[i].Handle );
                Global.Kbd[i].Handle = NULL;
            }
        }

        //
        // Destroy heap
        //
        if ( Global.Heap )
        {
            RtlDestroyHeap( Global.Heap );
            Global.Heap = NULL;
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        Status = STATUS_INVALID_PARAMETER;
        DislpayErrorMessage( L"DfrgBUninit. Exception !!!", Status, ERR_OS );
    }


    return Status;

} // end of DfrgBUninit

//
// Main() function (Native App entry point)
//
//NTSTATUS 
//__cdecl 
//main(
//    int     argc, 
//    char    *argv[], 
//    char    *envp[], 
//    ULONG   DebugParameter OPTIONAL )
void 
NtProcessStartup( 
    PSTARTUP_ARGUMENT   Argument ) // treat as PVOID
{
    NTSTATUS            Status = STATUS_INVALID_PARAMETER;
    int                 Result = 0; // NO_ERROR;

    PDEFRAG_JOB_CONTEXT CurrentJob = NULL;
    WCHAR               DriveName[MAX_PATH] = L"\\??\\C:";

    WCHAR               Mess[MAX_PATH];


    __try
    {
#ifdef _DEBUG
        OutMessage( L"\nBootDfrg Started" );
        Result = GetInput();
#endif

        Status = DfrgBInit( Argument );
        if ( !NT_SUCCESS( Status ) )
        {
            DislpayErrorMessage( L"\nBootDfrg Init error", Status, ERR_NT_STATUS );
            __leave;
        }

        PSHEDULED_JOB_ITEM           SheduledJob = DfrgSettings.SheduledJobs;

#ifdef _DEBUG
        swprintf( Mess, L"Job List %08X\n", SheduledJob );
        OutMessage( Mess );
#endif

        while ( SheduledJob )
        {
            PDFRG_SHEDULED_JOB      JobToRun = &(SheduledJob->Job);

            swprintf( Mess, L"Find Job %08X\n", JobToRun->Id  );
            OutMessage( Mess );

            if ( JobToRun->DefragMode == Scheduler::e_DefragModeBoot )
            {
                swprintf( Mess, L"Boot Job. Drive '%ls' Method %08X - ", 
                    JobToRun->DrivesList, JobToRun->Method );

                //
                // Init Job
                //
                int Mode = DEFRAG_JOB_ANALIZE_ONLY;

                switch( JobToRun->Method )
                {
                case Scheduler::e_DefragSimple:

                    Mode = DEFRAG_JOB_SIMPLE_DEFRAG;

                    wcscat( Mess, L"SIMPLE_DEFRAG" ); 

                    break;

                case Scheduler::e_DefragFreeSpace:

                    Mode = DEFRAG_JOB_FREE_SPACE;

                    wcscat( Mess, L"FREE_SPACE" ); 

                    break;

                case Scheduler::e_DefragByName:

                    Mode = DEFRAG_JOB_SMART_BY_NAME;

                    wcscat( Mess, L"SMART_BY_NAME" ); 

                    break;

                case Scheduler::e_DefragByAccess:

                    Mode = DEFRAG_JOB_SMART_BY_ACCESS;

                    wcscat( Mess, L"SMART_BY_ACCESS" ); 

                    break;

                case Scheduler::e_DefragByModification:

                    Mode = DEFRAG_JOB_SMART_BY_MODIFY;

                    wcscat( Mess, L"SMART_BY_MODIFY" ); 

                    break;

                case Scheduler::e_DefragByCreation:

                    Mode = DEFRAG_JOB_SMART_BY_CREATE;

                    wcscat( Mess, L"SMART_BY_CREATE" ); 

                    break;

                case Scheduler::e_DefragBySize:

                    Mode = DEFRAG_JOB_SMART_BY_SIZE;

                    wcscat( Mess, L"SMART_BY_SIZE" ); 

                    break;

                case Scheduler::e_DefragForce:

                    Mode = DEFRAG_JOB_FORCE_TOGETHER;

                    wcscat( Mess, L"FORCE_TOGETHER" ); 

                    break;

                default:

                    wcscat( Mess, L"UNKNOWN" ); 

                    break;

                }

                OutMessage( Mess );
#ifdef _DEBUG
                Result = GetInput();
#endif

                if( JobToRun->DrivesType == Scheduler::e_DrivesSelected )
                {
                    DriveName[4] = JobToRun->DrivesList[0];
                }
                else
                {
                    DriveName[4] = L'C';
                }

                Result = DfrgInitJobCtx( &Global.JobList, DriveName, Mode );
                if ( NO_ERROR != Result )
                {
                    DislpayErrorMessage( L"DfrgInitJobCtx error", Status, ERR_NT_STATUS );
                    __leave;
                }

                //
                // Run Job
                //
                CurrentJob = Global.JobList;

                CurrentJob->Options &= ~DEFRAG_JOB_OPTION_SHOW_PROGRESS;
                CurrentJob->Options |= DEFRAG_JOB_OPTION_BOOT_DEFRAG;

                Result = DfrgDoJob( CurrentJob, &DfrgSettings );

                //
                // Output result information
                //
                swprintf( Mess, L"\nBootDfrg. Job on drive '%ls' done. Status: %d (0x%08X)\n", 
                    JobToRun->DrivesList, Status, Status );
                OutMessage( Mess );
                OutMessage( L"" );
#ifdef _DEBUG
                Result = GetInput();
#endif
                //if ( NO_ERROR != Result )
                //{
                //    DislpayErrorMessage( L"DfrgDoJob error", Status, ERR_NT_STATUS );
                //    __leave;
                //}

                //
                // Delete job
                //
                DfrgFreeJobCtx( Global.JobList );
                Global.JobList = NULL;

                ULONG       BufSize = sizeof(DFRG_SHEDULED_JOB);
                DfrgApiSettingOp( SheduledJobsId, OP_REMOVE, JobToRun, &BufSize );

            } // if ( BootJob
#ifdef _DEBUG
            else
            {
                swprintf( Mess, L"   Not boot Job\n" );
                OutMessage( Mess );
            }
#endif

            SheduledJob = SheduledJob->NextJob;

        } // while ( sheduled jobs

    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        Status = STATUS_INVALID_PARAMETER;
        DislpayErrorMessage( L"NtProcessStartup. Exception !!!", Status, ERR_OS );
    }


    DfrgBUninit();

    //
    // Terminate app
    //
    NtTerminateProcess( NtCurrentProcess(), 0 );
    //return 0;

} // end of NtProcessStartup (main)
