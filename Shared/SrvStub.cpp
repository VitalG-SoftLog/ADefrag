/*
    Defrag Engine

    Module name:

        SrvStub.cpp

    Abstract:

        Defrag service module. 
        Defines service entry point, callbacks, etc.

    $Header: /home/CVS_DEFRAG/Defrag/Src/Shared/SrvStub.cpp,v 1.4 2009/12/25 13:48:54 dimas Exp $
    $Log: SrvStub.cpp,v $
    Revision 1.4  2009/12/25 13:48:54  dimas
    Debug artefacts removed

    Revision 1.3  2009/12/24 10:52:20  dimas
    Check against Exclude Files list implemented

    Revision 1.2  2009/12/24 01:39:11  roman
    fix errors

    Revision 1.1  2009/11/24 14:51:31  dimas
    no message

*/

#include "stdafx.h"

//
// This headers already included in "stdafx.h"
//
//#include "Api.h"
//#include "..\Defrag01\JobManager.h"
//#include "SrvStub.h"

//============================================================================
//
// Service registration & load/unload functions
//
//============================================================================

int
StopSrv( 
    IN SC_HANDLE SCMgrHandle, 
    IN LPCTSTR ServiceName )
{
    int               err = NO_ERROR;
    SC_HANDLE         SrvHandle = NULL;
    SERVICE_STATUS    serviceStatus;


    SrvHandle = OpenService( 
                    SCMgrHandle, 
                    ServiceName, 
                    SERVICE_ALL_ACCESS );
    if ( SrvHandle == NULL ) // || SrvHandle == INVALID_HANDLE_VALUE ) 
    {
        OutMessage( L"StopSrv OpenService() error" );
        return -2;
    }

    if ( !ControlService( 
                SrvHandle, 
                SERVICE_CONTROL_STOP, 
                &serviceStatus ) ) 
    {
        OutMessage( L"StopSrv ControlService() error" );
        err = -3;
    }

    if ( SrvHandle ) 
    {
        CloseServiceHandle( SrvHandle );
        SrvHandle = NULL;
    }

    return err;

} // end of StopSrv


int
StartSrv( 
    IN SC_HANDLE SCMgrHandle, 
    IN LPCTSTR ServiceName )
{
    int               err = NO_ERROR;
    SC_HANDLE         SrvHandle = NULL;
    TCHAR             *Parameters[] = { L"Service" };


    SrvHandle = OpenService( 
                    SCMgrHandle, 
                    ServiceName, 
                    SERVICE_ALL_ACCESS );
    if ( SrvHandle == NULL ) // || SrvHandle == INVALID_HANDLE_VALUE ) 
    {
        OutMessage( L"StartSrv OpenService() error" );
        return -2;
    }

    if ( !StartService(
                SrvHandle, 
                sizeof(Parameters)/sizeof(TCHAR*), 
                (LPCWSTR*)Parameters ) ) 
    {
        OutMessage( L"StartSrv StartService() error" );
        err = -3;
    }

    if ( SrvHandle ) 
    {
        CloseServiceHandle( SrvHandle );
        SrvHandle = NULL;
    }

    return err;

} // end of StartSrv


int
RemoveSrv( 
    IN SC_HANDLE SCMgrHandle, 
    IN LPCTSTR ServiceName )
{
    int               err = NO_ERROR;
    SC_HANDLE         SrvHandle = NULL;


    SrvHandle = OpenService( 
                    SCMgrHandle, 
                    ServiceName, 
                    SERVICE_ALL_ACCESS );

    if ( SrvHandle == NULL ) // || SrvHandle == INVALID_HANDLE_VALUE ) 
    {
        OutMessage( L"RemoveSrv OpenService() error" );
        return -2;
    }

    if ( !DeleteService( SrvHandle ) ) 
    {
        if ( err == ERROR_SERVICE_MARKED_FOR_DELETE ) 
        {
            OutMessage( L"RemoveSrv DeleteService() marked for delete" );
            err = 0;
        }
        else 
        {
            OutMessage( L"RemoveSrv DeleteService() error" );
            err = -3;
        }
    }

    if ( SrvHandle ) 
    {
        CloseServiceHandle( SrvHandle );
        SrvHandle = NULL;
    }

    return err;

} // end of RemoveSrv


int
InstallSrv( 
    IN SC_HANDLE    SCMgrHandle, 
    IN LPCTSTR      ServiceName, 
    IN LPCTSTR      ServicePath )
{
    int             err = NO_ERROR;
    SC_HANDLE       SrvHandle = NULL;


    SrvHandle = CreateService( 
                    SCMgrHandle,                // SCMgrHandle database
                    ServiceName,                // name of service
                    ServiceName,                // name to display
                    0,                          // desired access
                    SERVICE_WIN32_OWN_PROCESS,  // service type
                    SERVICE_AUTO_START, //SERVICE_DEMAND_START,       // start type
                    SERVICE_ERROR_NORMAL,       // error control type
                    ServicePath,                // service's binary
                    NULL,                       // no load ordering group
                    NULL,                       // no tag identifier
                    NULL, //L"+\0\0",           // dependencies
                    NULL,                       // LocalSystem account
                    NULL );                     // no password

    if ( SrvHandle ) // && SrvHandle != INVALID_HANDLE_VALUE ) 
    {
        CloseServiceHandle( SrvHandle );
        SrvHandle = NULL;
    }
    else
    {
        OutMessage( L"InstallSrv CreateService() error" );
        err = -1;
    }

    return err;

} // end of InstallSrv


int            
SelfRegister( 
    TCHAR *ExePath )
{
    int             err = -1; 
    SC_HANDLE       SCMgrHandle = NULL;
    TCHAR           SrvPath[MAX_PATH];

    //
    // Open Service Control Manager
    //
    SCMgrHandle = OpenSCManager( 
                            NULL,                     // machine (NULL == local)
                            NULL,                     // database (NULL == default)
                            SC_MANAGER_ALL_ACCESS );  // access required

    if ( SCMgrHandle == NULL ) 
    {
        OutMessage( L"SelfRegister error" );
        return -1;
    }

    //
    // Remove existing
    //
    SelfUnregister();

    //
    // Register ourself
    //
    _tcscpy( SrvPath, ExePath );
    _tcscat( SrvPath, _T(" Service") );

    err = InstallSrv( 
        SCMgrHandle, 
        SERVICE_NAME, 
        SrvPath );

    if ( SCMgrHandle ) 
    {
        CloseServiceHandle( SCMgrHandle );
        SCMgrHandle = NULL;
    }

    return err;

} // end of SelfRegister


int
SelfUnregister( 
    void )
{
    int            err = NO_ERROR;
    SC_HANDLE      SCMgrHandle = NULL;

    //
    // Open Service Control Manager
    //
    SCMgrHandle = OpenSCManager( 
                            NULL,                     // machine (NULL == local)
                            NULL,                     // database (NULL == default)
                            SC_MANAGER_ALL_ACCESS );  // access required

    if ( SCMgrHandle == NULL ) 
    {
        OutMessage( L"SelfUnregister error" );
        return -1;
    }

    //
    // Stop and remove ourself
    //
    StopSrv( SCMgrHandle, SERVICE_NAME );

    err = RemoveSrv( SCMgrHandle, SERVICE_NAME );

    if ( SCMgrHandle ) 
    {
        CloseServiceHandle( SCMgrHandle );
        SCMgrHandle = NULL;
    }

    return err;

} // end of SelfUnregister


int
SelfStartService( 
    void )
{
    int            err = NO_ERROR;
    SC_HANDLE      SCMgrHandle = NULL;

    //
    // Open Service Control Manager
    //
    SCMgrHandle = OpenSCManager( 
                            NULL,                     // machine (NULL == local)
                            NULL,                     // database (NULL == default)
                            SC_MANAGER_ALL_ACCESS );  // access required

    if ( SCMgrHandle == NULL ) 
    {
        OutMessage( L"SelfStart error" );
        return -1;
    }

    //
    // Restart
    //
    err = StopSrv( SCMgrHandle, SERVICE_NAME );
    if ( !err )
    {
        Sleep( 5000 );
    }

    err = StartSrv( SCMgrHandle, SERVICE_NAME );

    if ( SCMgrHandle ) 
    {
        CloseServiceHandle( SCMgrHandle );
        SCMgrHandle = NULL;
    }

    return err;

} // end of SelfStartService


int
SelfStopService( 
    void )
{
    int            err = NO_ERROR;
    SC_HANDLE      SCMgrHandle;

    //
    // Open Service Control Manager
    //
    SCMgrHandle = OpenSCManager( 
                            NULL,                     // machine (NULL == local)
                            NULL,                     // database (NULL == default)
                            SC_MANAGER_ALL_ACCESS );  // access required

    if ( SCMgrHandle == NULL ) 
    {
        OutMessage( L"SelfStop error" );
        return -1;
    }

    StopSrv( SCMgrHandle, SERVICE_NAME );

    if ( SCMgrHandle ) 
    {
        CloseServiceHandle( SCMgrHandle );
        SCMgrHandle = NULL;
    }

    return err;


} // end of SelfStopService

int
QuerySrv( 
    OUT LPSERVICE_STATUS    ServiceStatus )
{
    int                 err = NO_ERROR;
    SC_HANDLE           SCMgrHandle = NULL;
    SC_HANDLE           SrvHandle = NULL;


    //
    // Open Service Control Manager
    //
    SCMgrHandle = OpenSCManager( 
                            NULL,                     // machine (NULL == local)
                            NULL,                     // database (NULL == default)
                            SC_MANAGER_ALL_ACCESS );  // access required

    if ( SCMgrHandle == NULL ) 
    {
        OutMessage( L"QuerySrv OpenSCManager error" );
        return -1;
    }
    else
    {
        SrvHandle = OpenService( 
                        SCMgrHandle, 
                        SERVICE_NAME, 
                        SERVICE_ALL_ACCESS );

        if ( SrvHandle == NULL ) // || SrvHandle == INVALID_HANDLE_VALUE ) 
        {
            OutMessage( L"QuerySrv OpenService() error" );
            return -2;
        }
        else
        {
            if ( !QueryServiceStatus( SrvHandle, ServiceStatus ) || 
                SERVICE_RUNNING != ServiceStatus->dwCurrentState )
            {
                OutMessage( L"QuerySrv. Service not ready" );
                err = -3;
            }
        }
    }

    if ( SrvHandle ) 
    {
        CloseServiceHandle( SrvHandle );
        SrvHandle = NULL;
    }

    if ( SCMgrHandle ) 
    {
        CloseServiceHandle( SCMgrHandle );
        SCMgrHandle = NULL;
    }

    return err;

} // end of QuerySrv


DWORD 
StartServiceEngine( 
    HANDLE *ObjectToWait )
{
    DWORD  specificError = NO_ERROR;

    specificError = (DWORD)DfrgStartJobManager( ObjectToWait );

    StartSchedulerTaskManager();

    return specificError;

} // end of StartServiceEngine


void         
StopServiceEngine( 
    HANDLE ObjectToWait )
{

    StopSchedulerTaskManager();

    DfrgStopJobManager( ObjectToWait );

    return;

} // end of StopServiceEngine


//============================================================================
//
// Service standard functions
//
//============================================================================

void 
WINAPI
ServiceMain( 
    DWORD   argc, 
    LPTSTR  *argv )
{

    DWORD       specificError = NO_ERROR;
    HANDLE      WorkThread = NULL;

#ifdef _DEBUG
    OutMessage( L"SrvMain() report start pending" );
#endif

    Global.ServiceStatus.dwServiceType              = SERVICE_WIN32;
    Global.ServiceStatus.dwCurrentState             = SERVICE_START_PENDING;
    Global.ServiceStatus.dwControlsAccepted         = 0; //SERVICE_ACCEPT_STOP;
    Global.ServiceStatus.dwWin32ExitCode            = EXIT_SUCCESS;
    Global.ServiceStatus.dwServiceSpecificExitCode  = NO_ERROR;
    Global.ServiceStatus.dwCheckPoint               = 0;
    Global.ServiceStatus.dwWaitHint                 = 10000;

    Global.ServiceStatusHandle = RegisterServiceCtrlHandlerEx( 
                                     SERVICE_NAME,
                                     ServiceCtrlHandler, 
                                     NULL ); 

    if ( Global.ServiceStatusHandle == NULL ) 
    {
       return;
    }

    if ( !SetServiceStatus( Global.ServiceStatusHandle, &Global.ServiceStatus ) ) 
    {
    }

    //
    // Performe initialization
    //
    //specificError = InitDefrag( argc, argv );
    if ( NO_ERROR == specificError ) 
    {
        specificError = StartServiceEngine( &Global.ObjectToWait );
    }

    if ( NO_ERROR == specificError ) 
    {

        //
        // Initialization complete - report running status.
        //
        Global.ServiceStatus.dwControlsAccepted   |= SERVICE_ACCEPT_STOP;
        Global.ServiceStatus.dwCurrentState       = SERVICE_RUNNING;
        Global.ServiceStatus.dwCheckPoint         = 0;
        Global.ServiceStatus.dwWaitHint           = 0;

        if ( !SetServiceStatus( Global.ServiceStatusHandle, &Global.ServiceStatus ) ) 
        {
        }

        //
        // Wait while service functional budy does its work
        //
        if ( Global.ObjectToWait )
        {
#ifdef _DEBUG
            OutMessage( L"SrvMain() Let's wait" );
#endif
            WaitForSingleObject( Global.ObjectToWait, INFINITE );
        }

        ////CloseHandle( Global.ObjectToWait );
        ////Global.ObjectToWait = NULL;
        //UninitDefrag();

#ifdef _DEBUG
        OutMessage( L"SrvMain() wait finished" );
#endif

        Global.ServiceStatus.dwWin32ExitCode            = EXIT_SUCCESS;
        Global.ServiceStatus.dwServiceSpecificExitCode  = NO_ERROR;
        Global.ServiceStatus.dwControlsAccepted         &= ~(SERVICE_ACCEPT_STOP);
        Global.ServiceStatus.dwCurrentState             = SERVICE_STOPPED;
        Global.ServiceStatus.dwCheckPoint               = 0;
        Global.ServiceStatus.dwWaitHint                 = 0;

#ifdef _DEBUG
        OutMessage( L"SrvMain() report STOPPED" );
#endif

        if ( !SetServiceStatus( Global.ServiceStatusHandle, &Global.ServiceStatus ) ) 
        {
        }
    }
    else
    {
        Global.ServiceStatus.dwWin32ExitCode            = EXIT_FAILURE;
        Global.ServiceStatus.dwServiceSpecificExitCode  = specificError;
        Global.ServiceStatus.dwControlsAccepted         &= ~(SERVICE_ACCEPT_STOP);
        Global.ServiceStatus.dwCurrentState             = SERVICE_STOPPED;
        Global.ServiceStatus.dwCheckPoint               = 0;
        Global.ServiceStatus.dwWaitHint                 = 0;

#ifdef _DEBUG
        OutMessage( L"SrvMain() error - report STOPPED" );
#endif

        if ( !SetServiceStatus( Global.ServiceStatusHandle, &Global.ServiceStatus ) ) 
        {
        }
    }

#ifdef _DEBUG
    OutMessage( L"SrvMain() exit" );
#endif

} // end of ServiceMain


DWORD 
WINAPI
ServiceCtrlHandler( 
    DWORD   Opcode, 
    DWORD   dwEventType,
    LPVOID  lpEventData, 
    LPVOID  lpContext )
{
    DWORD   RetCode = NO_ERROR; 

    switch( Opcode ) 
    {

    case SERVICE_CONTROL_STOP:

        if ( Global.ServiceStatus.dwCurrentState != SERVICE_STOP_PENDING &&
            Global.ServiceStatus.dwCurrentState != SERVICE_STOPPED &&
            Global.ServiceStatus.dwCurrentState != SERVICE_START_PENDING ) 
        {

            Global.ServiceStatus.dwWin32ExitCode            = EXIT_SUCCESS;
            Global.ServiceStatus.dwServiceSpecificExitCode  = NO_ERROR;
            Global.ServiceStatus.dwControlsAccepted         &= ~(SERVICE_ACCEPT_STOP);
            Global.ServiceStatus.dwCurrentState             = SERVICE_STOP_PENDING;
            Global.ServiceStatus.dwCheckPoint               = 0;
            Global.ServiceStatus.dwWaitHint                 = 10000;

#ifdef _DEBUG
            OutMessage( L"SrvCtrl() report STOP_PENDING" );
#endif
            if ( !SetServiceStatus( Global.ServiceStatusHandle, &Global.ServiceStatus ) ) 
            {
            }

#ifdef _DEBUG
            OutMessage( L"SrvCtrl() StopServiceEngine" );
#endif

            StopServiceEngine( Global.ObjectToWait );

#ifdef _DEBUG
            OutMessage( L"SrvCtrl() exit" );
#endif

            return RetCode;
        }
        else
        {
            RetCode = ERROR_INVALID_PARAMETER;
        }
        break;

    case SERVICE_CONTROL_PAUSE:
    case SERVICE_CONTROL_CONTINUE:
    default:

        RetCode = ERROR_CALL_NOT_IMPLEMENTED;
        break;

    case SERVICE_CONTROL_INTERROGATE: 

        break;

    }

    if ( !SetServiceStatus( Global.ServiceStatusHandle, &Global.ServiceStatus ) ) 
    {
    }

    return RetCode;

} // end of ServiceCtrlHandler

