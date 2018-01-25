/*
    Defrag Engine

    Module name:

        Defrag01.cpp

    Abstract:

        Defrag Engine main module. 
        Defines the entry point for the console application.

    $Header: /home/CVS_DEFRAG/Defrag/Src/Defrag01/Defrag01.cpp,v 1.2 2009/12/02 14:42:15 dimas Exp $
    $Log: Defrag01.cpp,v $
    Revision 1.2  2009/12/02 14:42:15  dimas
    App/Service interaction improved

    Revision 1.1  2009/11/24 14:52:31  dimas
    no message

*/

#include "stdafx.h"


DEFRAG_GLOBAL     Global;


void
ShowUsageHelp(
    void )
{
    OutMessage( L"" );
    OutMessage( L"               ---- Defrag01 engine usage ----" );
    OutMessage( L"ATTENTION! This is Beta version and you use it on your own risk." );
    OutMessage( L"You can use it alone in command line mode or as service controlled " );
    OutMessage( L"via GUI or this utility itself running in CMD mode as describe below:" );
    OutMessage( L"" );
    OutMessage( L"   Defrag01 {?|-?|/?}            - Show usage" );
    OutMessage( L"" );
    OutMessage( L"   Defrag01 Job[.*] <Volume>     - Do Job:" );
    OutMessage( L"      Job.Analyze                  - Analyze only" );
    OutMessage( L"      Job.Simple                   - Simple defrag" );
    OutMessage( L"      Job.Free                     - Free maximum contiguous space" );
    OutMessage( L"      Job.Name                     - Smart by Name        - not implemented" );
    OutMessage( L"      Job.Size                     - Smart by Size        - not implemented" );
    OutMessage( L"      Job.Access                   - Smart by Last Access - not implemented" );
    OutMessage( L"      Job.Modify                   - Smart by Last Modify - not implemented" );
    OutMessage( L"      Job.Create                   - Smart by Last Modify - not implemented" );
    OutMessage( L"      Job.Together                 - Move folders together- not implemented" );
    OutMessage( L"Comment: If Service is registered it is used to performe job." );
    OutMessage( L"" );
    OutMessage( L"   Defrag01 Service[.*]          - Service control:" );
    OutMessage( L"      Service.Register             - Register defrag service" );
    OutMessage( L"      Service.Start                - (Re)Start defrag service" );
    OutMessage( L"      Service.Stop                 - Stop defrag service" );
    OutMessage( L"      Service.Unregister           - Unregister defrag service" );
    OutMessage( L"      Service                      - Run as service. Do not use this" );
    OutMessage( L"                                     parameter in command line mode" );
    OutMessage( L"" );

} // end of ShowUsageHelp



int
CheckVersion( void )
{
    int                  Result = NO_ERROR;

    OSVERSIONINFO        winVer;
    OSVERSIONINFOEX      winVerEx;
    WCHAR                Mess[MAX_PATH];


    winVer.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    if ( !GetVersionEx( &winVer ) || 
        winVer.dwOSVersionInfoSize != sizeof(OSVERSIONINFO) ) 
    {
        DislpayErrorMessage( L"Error get OS version", -1, ERR_OS );
        return -1;
    }
    else if ( winVer.dwPlatformId != VER_PLATFORM_WIN32_NT ) 
    {
        OutMessage( L"Could not run on Windows 9x/Me platform" );
        return -1;
    }

    memcpy( &Global.OsVersion, &winVer, sizeof(OSVERSIONINFO) );

    swprintf( Mess, _T("\nRun on Windows %d.%d build %d, %s "),
                    Global.OsVersion.dwMajorVersion, Global.OsVersion.dwMinorVersion,
                    Global.OsVersion.dwBuildNumber, Global.OsVersion.szCSDVersion );

    if ( winVer.dwMajorVersion >= 4 ) 
    {
        winVerEx.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

        if ( !GetVersionEx( (OSVERSIONINFO*)&winVerEx ) || 
            winVerEx.dwOSVersionInfoSize != sizeof(OSVERSIONINFOEX) ) 
        {
            //
            // Get Product Type from registry
            //
        }
        else 
        { 
            //
            // WinNT 4.0 SP 6 or above
            //
            _tcscat( Mess, _T("(") );
            if ( winVerEx.wServicePackMajor || winVerEx.wServicePackMinor )
            {
                swprintf( Mess+_tcslen(Mess), _T("SP %d.%d, "),
                       winVerEx.wServicePackMajor, winVerEx.wServicePackMinor );
            }

            if ( winVerEx.wProductType == VER_NT_WORKSTATION ) 
            {
                _tcscat( Mess, _T("Workstation") );
            }
            else 
            {
                swprintf( Mess+_tcslen(Mess), _T("%s"),
                       (winVerEx.wProductType == VER_NT_DOMAIN_CONTROLLER) ? 
                          _T("Domain controller") : _T("Server") );
            }
            _tcscat( Mess, _T(")\n") );
        }
    }

    OutMessage( Mess );

    return Result;

} // end of CheckVersion


//#define _DEBUGS

int
InitDefrag(
    int     argc, 
    WCHAR*  argv[] )
{
    int         Result = NO_ERROR;


__try
{

#ifdef _DEBUG
    OutMessage( L"   InitDefrag" );
#endif

    //
    // Performe initialization
    //
    InitializeCriticalSection( &Global.JobListSync );
    Global.AppStatus &= ~APP_STAT_UNINIT;

    //
    // Get program name
    //
    wcscpy( Global.AppName, argv[0] );

    if ( !wcsrchr( Global.AppName, L'\\' ) )
    {
        if ( GetCurrentDirectory( (DWORD)(ARRAYSIZE(Global.AppName)-wcslen(argv[0])), Global.AppName ) )
        {
            wcscat( Global.AppName, L"\\" );
            wcscat( Global.AppName, argv[0] );
        }
    }

    _wcslwr( Global.AppName );


    //
    // Get version
    //
    Result = CheckVersion();
    if ( NO_ERROR != Result )
    {
        __leave;
    }

    if ( Global.OsVersion.dwMajorVersion < 5 || 
        Global.OsVersion.dwMajorVersion == 5 && Global.OsVersion.dwMinorVersion < 1 )
    {
        OutMessage( L"InitDefrag: Wrong version" );
        Result = ERROR_NOT_SUPPORTED;
        __leave;
    }

    //
    // Initialize subsystems
    //
    if ( RunAsServiceMode == Global.ServiceAction )
    {
        DfrgInitializeSettings( NULL );
    }


    //
    // Set Service Process Privileges
    //
    Result = SetProcessAdminPrivilege();
    if ( NO_ERROR != Result )
    {
        DislpayErrorMessage( L"InitDefrag(). Set Priv Error", Result, ERR_OS );
        __leave;
    }

    Global.AppStatus |= APP_STAT_INIT;

#ifdef _DEBUG
    OutMessage( L"   Defrag Initialized" );
#endif
}
__except(EXCEPTION_EXECUTE_HANDLER)
{
    Result = GetExceptionCode();
    DislpayErrorMessage( L"InitDefrag Exception !!!", Result, ERR_OS );
}

    return Result;

} // end of InitDefrag


void
UninitDefrag( void )
{

#ifdef _DEBUG
    OutMessage( L"   UninitDefrag" );
#endif

    if ( (Global.AppStatus & APP_STAT_UNINIT) )
    {
        return;
    }

    //
    // Delete all jobs
    //
    FreeJobList( Global.JobList );
    Global.JobList = NULL;

    DeleteCriticalSection( &Global.JobListSync );

    //
    // Uninitialize subsystems
    //
    if ( RunAsServiceMode == Global.ServiceAction )
    {
#ifdef _DEBUGS
        OutMessage( L"      UninitializeSettings" );
#endif
        DfrgUninitializeSettings();
    }


    //
    // Close syncronization object
    //
    if ( Global.ObjectToWait )
    {
#ifdef _DEBUGS
        OutMessage( L"      Close syncronization object" );
#endif
        CloseHandle( Global.ObjectToWait );
        Global.ObjectToWait = NULL;
    }

    //if ( RunAsServiceMode != Global.ServiceAction ) 

    Global.AppStatus &= ~APP_STAT_INIT;
    Global.AppStatus |= APP_STAT_UNINIT;

#ifdef _DEBUG
    OutMessage( L"   Defrag Uninitialized" );
#endif

} // end of UninitDefrag


int
ParseCommandString(
    int     argc, 
    WCHAR*  argv[] )
{
    int         Result = NO_ERROR;
    WCHAR       ParamString[MAX_PATH];
    int         i;

    WCHAR       DriveName[MAX_PATH] = L"\\??\\C:";
    int         Mode = DEFRAG_JOB_DEBUG1;


    if ( argc == 1 ) 
    {
        Global.JobAction = ShowHelpMode;
        Result = NO_ERROR;

#ifdef _DEBUG
        Global.JobAction = DoJobMode;
        Result = DfrgInitJobCtx( &(Global.JobList), DriveName, Mode );
#endif

        return Result;
    }

    for ( i=1; i<argc; i++ ) 
    {
        if ( argv[i][0] == L'?' )
        {
            Global.JobAction = ShowHelpMode;
            Result = NO_ERROR;
            break;
        }

        if ( argv[i][0] == L'/' || argv[i][0] == L'-' &&
            argv[i][1] ==  L'?' )
        {
            Global.JobAction = ShowHelpMode;
            Result = NO_ERROR;
            break;
        }

        //if ( argv[i][0] != L'/' && argv[i][0] != L'-' )
        //{
        //    ZeroMemory( Global.FileName, sizeof(Global.FileName) );
        //    wcsncpy( Global.FileName, argv[i], ARRAYSIZE(Global.FileName)-1 );
        //    continue;
        //}

        wcscpy( ParamString, argv[i] );
        _wcsupr( ParamString );

        //
        // Defrag modes
        //
        if ( wcsstr( ParamString, L"JOB" ) ) 
        {
            Global.JobAction = DoJobMode;

            WCHAR *SwitchPtr = wcschr( ParamString, L'.' );
            if ( SwitchPtr ) 
            {
                SwitchPtr++;
                if ( !wcscmp( SwitchPtr, L"ANALYZE" ) )
                {
                    Mode = DEFRAG_JOB_ANALIZE_ONLY;
                }
                else if ( !wcscmp( SwitchPtr, L"SIMPLE" ) )
                {
                    Mode = DEFRAG_JOB_SIMPLE_DEFRAG;
                }
                else if ( !wcscmp( SwitchPtr, L"FREE" ) )
                {
                    Mode = DEFRAG_JOB_FREE_SPACE;
                }
                else if ( !wcscmp( SwitchPtr, L"NAME" ) )
                {
                    Mode = DEFRAG_JOB_SMART_BY_NAME;
                }
                else if ( !wcscmp( SwitchPtr, L"ACCESS" ) )
                {
                    Mode = DEFRAG_JOB_SMART_BY_ACCESS;
                }
                else if ( !wcscmp( SwitchPtr, L"MODIFY" ) )
                {
                    Mode = DEFRAG_JOB_SMART_BY_MODIFY;
                }
                else if ( !wcscmp( SwitchPtr, L"SIZE" ) )
                {
                    Mode = DEFRAG_JOB_SMART_BY_SIZE;
                }
                else if ( !wcscmp( SwitchPtr, L"CREATE" ) )
                {
                    Mode = DEFRAG_JOB_SMART_BY_CREATE;
                }
                else if ( !wcscmp( SwitchPtr, L"TOGETHER" ) )
                {
                    Mode = DEFRAG_JOB_FORCE_TOGETHER;
                }
            }

            if ( argc > i + 1 )
            {
                wcscpy( DriveName, argv[i+1] );
                _wcsupr( DriveName );
            }

            Result = DfrgInitJobCtx( &Global.JobList, DriveName, Mode );
            //if ( NO_ERROR != Result )
            break;
        }

        //
        // Service modes
        //
        else if ( wcsstr( ParamString, L"SERVICE" ) ) 
        {
            WCHAR *SwitchPtr = wcschr( ParamString, L'.' );
            if ( !SwitchPtr ) 
            {
                Global.ServiceAction = RunAsServiceMode;
                Result = NO_ERROR;
                break;
            }

            SwitchPtr++;

            if ( !wcscmp( SwitchPtr, L"START" ) )
            {
                Global.ServiceAction = StartServiceMode;
                Result = NO_ERROR;
                break;
            }
            else if ( !wcscmp( SwitchPtr, L"STOP" ) )
            {
                Global.ServiceAction = StopServiceMode;
                Result = NO_ERROR;
                break;
            }
            else if ( !wcscmp( SwitchPtr, L"REGISTER" ) )
            {
                Global.ServiceAction = RegisterServiceMode;
                Result = NO_ERROR;
                break;
            }
            else if ( !wcscmp( SwitchPtr, L"UNREGISTER" ) )
            {
                Global.ServiceAction = UnregisterServiceMode;
                Result = NO_ERROR;
                break;
            }
            break;
        }

    }

    return Result;

} // end of ParseCommandString


//#define _DEBUG_TREE


#ifdef _DEBUG_TREE

void
PrintTree( 
    PBTR_TREE                   TestTree )
{
    int                         Result = NO_ERROR;

    PDEFRAG_FILE_CONTEXT        FileCtx = NULL;
    PDEFRAG_FILE_CONTEXT        FileCtxL = NULL;
    PDEFRAG_FILE_CONTEXT        FileCtxR = NULL;
    PBTR_NODE                   FileNode;
    LARGE_INTEGER               Cnt = { 0 };


    WCHAR                       Mess[MAX_PATH*2];


    swprintf( Mess, L"Print Test Files Tree [%I64d]",
        TestTree->NodeCnt.QuadPart );
    OutMessage( Mess );

    if ( TestTree->Left )
    {
        FileCtxL = (PDEFRAG_FILE_CONTEXT)TestTree->Left->Body; 
    }

    if ( TestTree->Right )
    {
        FileCtxR = (PDEFRAG_FILE_CONTEXT)TestTree->Right->Body; 
    }

    if ( TestTree->Root )
    {
        FileCtx = (PDEFRAG_FILE_CONTEXT)TestTree->Root->Body; 
    }

    swprintf( Mess, L"  Head %I64d   Left %I64d   Right %I64d ",
        FileCtx  ? FileCtx->TotalClustersNum.QuadPart: (ULONGLONG)(-1), 
        FileCtxL ? FileCtxL->TotalClustersNum.QuadPart:(ULONGLONG)(-1), 
        FileCtxR ? FileCtxR->TotalClustersNum.QuadPart:(ULONGLONG)(-1) );
    OutMessage( Mess );
    

    //
    // Iterate through file list and process files
    //
    FileNode = FindFirstNode( TestTree, LEFT_TO_RIGHT );
    while ( FileNode )
    {
        Cnt.QuadPart++;
        if ( Cnt.QuadPart > TestTree->NodeCnt.QuadPart )
        {
            break;
        }

        FileCtx = (PDEFRAG_FILE_CONTEXT)FileNode->Body; 

        swprintf( Mess, L"  Node %I64d   Size %I64d ",
            Cnt.QuadPart, FileCtx->TotalClustersNum.QuadPart );
        OutMessage( Mess );
        

        FileNode = FindNextNode( TestTree, LEFT_TO_RIGHT );
    }

    OutMessage( L"" );
    GetInput();

} // end of PrintTree

LONGLONG
CompareFileCtx( 
    PVOID                   InFileCtx1,
    PVOID                   InFileCtx2,
    ULONG                   SortMode )
{
    PDEFRAG_FILE_CONTEXT    FileCtx1 = (PDEFRAG_FILE_CONTEXT)InFileCtx1;
    PDEFRAG_FILE_CONTEXT    FileCtx2 = (PDEFRAG_FILE_CONTEXT)InFileCtx2;

        return (LONGLONG)(FileCtx1->TotalClustersNum.QuadPart - FileCtx2->TotalClustersNum.QuadPart);
}

#endif


int 
_cdecl
_tmain(
    int     argc, 
    WCHAR*  argv[] )
{
    int                     Result = ERROR_INVALID_PARAMETER;
    SERVICE_TABLE_ENTRY     DispatchTable[] =
                               { { SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION)ServiceMain },
                                 { NULL, NULL } };
    int                     LoopCnt = 0;

#ifdef _DEBUG
    WCHAR                   Mess[MAX_PATH*2] = L"";
#endif

__try
{
    Global.AppStatus |= APP_STAT_UNINIT;

    Global.AppId = CmdAppId;


#ifdef _DEBUG_TREE

#define     NODE_TO_TEST_NUM        21
    //
    // Test tree
    //

    BTR_TREE                    TestTree = { 0 };
    BTR_TREE_METHODS            Methods;
    PDEFRAG_FILE_CONTEXT        FileCtx;
    DEFRAG_FILE_CONTEXT         FileCtxD = { 0 };
    PBTR_NODE                   Node = NULL;

    int                         FilesNum = NODE_TO_TEST_NUM;
    //int                         FilesLength[NODE_TO_TEST_NUM] = {  0,  0,  1,  1,  2,  2,  3,  3,  4,  4, 5, 
    //                                                              16, 16, 17, 17, 18, 18, 19, 19, 20, 20 };
    int                         FilesLength[NODE_TO_TEST_NUM] = {  0, 20,  0, 20,  1, 19,  1, 19,  2, 18, 2, 
                                                                  18,  3, 17,  3, 17,  4, 16,  4, 16,  5 };

    //
    // Init files tree
    //
    Methods.FreeBody        = NULL;
    Methods.CompareItems    = CompareFileCtx;
    Methods.CopyBody        = NULL;
    Methods.PrintBody       = NULL;

    InitTree( 
        &TestTree, 
        &Methods, 
        FilesKeySize );


    for ( int i=0; i<FilesNum; i++ )
    {
        FileCtx = (PDEFRAG_FILE_CONTEXT) malloc( sizeof(DEFRAG_FILE_CONTEXT) );
        memset( FileCtx, 0, sizeof(DEFRAG_FILE_CONTEXT) );
        int FileLength = (FilesNum / 2) + ((-1) + 2*(i%2))*(FilesNum/2 - i/4);
        FileCtx->TotalClustersNum.QuadPart = FilesLength[i];

        swprintf( Mess, L"  Add Node %d   %d == %d   ",
            i, FilesLength[i], FileLength );
        OutMessage( Mess );

        AddNode( &TestTree, FileCtx );
    }

    PrintTree( &TestTree );

    FileCtxD.TotalClustersNum.QuadPart = 16;

    while ( Node = FindNode( &TestTree, &FileCtxD, _FIND_LE_ ) )
    {
        FileCtx = (PDEFRAG_FILE_CONTEXT) Node->Body;

        swprintf( Mess, L"  Search Node %I64d   Delete Node %I64d   ",
            FileCtxD.TotalClustersNum.QuadPart, FileCtx->TotalClustersNum.QuadPart);
        OutMessage( Mess );

        RemoveNode( &TestTree, Node );
        FreeNode( &TestTree, Node );

        FindFirstNode( &TestTree, RIGHT_TO_LEFT );
        FindFirstNode( &TestTree, LEFT_TO_RIGHT );
        PrintTree( &TestTree );

    }


    FileCtxD.TotalClustersNum.QuadPart = FilesNum / 2;

    do
    {
        Node = FindNode( &TestTree, &FileCtxD, _FIND_LE_ );
        if ( !Node )
            break;

        FileCtx = (PDEFRAG_FILE_CONTEXT) Node->Body;

        swprintf( Mess, L"  Search Node %I64d   Delete Node %I64d   ",
            FileCtxD.TotalClustersNum.QuadPart, FileCtx->TotalClustersNum.QuadPart);
        OutMessage( Mess );

        RemoveNode( &TestTree, Node );
        FreeNode( &TestTree, Node );

        FindFirstNode( &TestTree, RIGHT_TO_LEFT );
        FindFirstNode( &TestTree, LEFT_TO_RIGHT );
        PrintTree( &TestTree );

    } while ( TestTree.NodeCnt.QuadPart );

    do
    {
        Node = FindNode( &TestTree, &FileCtxD, _FIND_GE_ );
        if ( !Node )
            break;

        FileCtx = (PDEFRAG_FILE_CONTEXT) Node->Body;

        swprintf( Mess, L"  Search Node %I64d   Delete Node %I64d   ",
            FileCtxD.TotalClustersNum.QuadPart, FileCtx->TotalClustersNum.QuadPart);
        OutMessage( Mess );

        RemoveNode( &TestTree, Node );
        FreeNode( &TestTree, Node );

        FindFirstNode( &TestTree, RIGHT_TO_LEFT );
        FindFirstNode( &TestTree, LEFT_TO_RIGHT );
        PrintTree( &TestTree );

    } while ( TestTree.NodeCnt.QuadPart );

    if ( FilesNum )
    {
        __leave;
    }
#endif

    //
    // Parse input parameters
    //
    Result = ParseCommandString( argc, argv );
    if ( NO_ERROR != Result )
    {
        if ( ERROR_INVALID_PARAMETER == Result )
        {
            OutMessage( L"\nInvalid parameter in Command String" );
            ShowUsageHelp();
        }
        __leave;
    }

    //
    // Usage help output only 
    //
    if ( ShowHelpMode == Global.JobAction )
    {
        ShowUsageHelp();
        __leave;
    }

    if ( RunAsServiceMode == Global.ServiceAction )
    {
        Global.AppId = ServiceAppId;
    }

    //
    // Performe initialization
    //
    Result = InitDefrag( argc, argv );
    if ( NO_ERROR != Result ) 
    {
        OutMessage( L"Defrag init error" );
        __leave;
    }

#ifdef _DEBUGSW
    else
    {
        __leave;
    }
#endif


    if ( !Global.ServiceAction ) //|| RunAsServiceMode != Global.ServiceAction )
    {
#ifdef _DEBUG
        //
        // Test service presence and if find send job to it
        //
        SERVICE_STATUS    ServiceStatus;

        Result = QuerySrv( &ServiceStatus );
        if ( NO_ERROR != Result  )
        {
            //Result = SelfStartService();
        }

        if ( NO_ERROR == Result )
        {
            Sleep( 2000 );
            RunServiceJob();
            __leave;
        }
        else
        {
            OutMessage( L"Connect to server error\n" );
        }

        //
        // Process command line modes
        //
        if ( (Global.Flags & FLAG_REPEAT_IN_LOOP) )
        {
            OutMessage( L"\nPress <Esc> to exit" );
        }

        PDEFRAG_JOB_CONTEXT     CurrentJob = Global.JobList;

        do 
        {
            switch( Global.JobAction )
            {
            case DoJobMode:

                if ( NO_ERROR != DfrgDoJob( CurrentJob, &DfrgSettings ) )
                {
                }

                CurrentJob = CurrentJob->NextJob;

                break;

            default:
                break;
            }

            if ( !(Global.Flags & FLAG_REPEAT_IN_LOOP) ||
                GetInput() < 0 )
            {
                break;
            }

            LoopCnt++;

        } while ( TRUE );
#endif // _DEBUG
    }
    else
    {
        //
        // Process service modes
        //
        switch ( Global.ServiceAction )
        {
        case RunAsServiceMode:

            if ( !StartServiceCtrlDispatcher( DispatchTable) ) 
            {
                OutMessage( L"StartServiceCtrlDispatcher error" );
            }
            //DislpayErrorMessage( L"main() returned from service", Result, ERR_OS );
            OutMessage( L"main() returned from service" );
            break;

        case StartServiceMode:

            if ( SelfStartService() ) 
            {
               OutMessage( L"Start Service error" );
            }
            else 
            {
               OutMessage( L"Service (re)started" );
            }
            break;

        case StopServiceMode:

            if ( SelfStopService() )
            {
               OutMessage( L"Stop Service error" );
            }
            else 
            {
               OutMessage( L"Service stopped" );
            }
            break;

        case RegisterServiceMode:

            if ( SelfRegister( Global.AppName ) )
            {
               OutMessage( L"Register Service error" );
            }
            else 
            {
               OutMessage( L"Service registered" );
            }
            break;

        case UnregisterServiceMode:

            if ( SelfUnregister() )
            {
               OutMessage( L"Unregister Service error" );
            }
            else 
            {
               OutMessage( L"Service unregistered" );
            }
            break;

        default:
            break;
        }
        __leave;
    }

}
__except(EXCEPTION_EXECUTE_HANDLER)
{
    //OutMessage( _T("main() Exception !!!") );
    //Result = ERROR_INVALID_PARAMETER;
    Result = GetExceptionCode();
    DislpayErrorMessage( L"main() Exception !!!", Result, ERR_OS );
}

    //OutMessage( L"main() finally" );

    //
    // Clean up
    //
    //if ( RunAsServiceMode != Global.ServiceAction )
        UninitDefrag();

    //OutMessage( L"main() exit" );

    if ( NO_ERROR == Result )
    {
        return EXIT_SUCCESS;
    }
    else
    {
        return Result; //EXIT_FAILURE; // Global.RetCode = Result;
    }

} // end of main

