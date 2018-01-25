/*
    Defrag Service Test

    Module name:

        Client.cpp

    Abstract:

        Defrag Engine Service Test module. 
        Defines Client functions..

    $Header: /home/CVS_DEFRAG/Defrag/Src/Shared/Client.cpp,v 1.6 2009/12/24 10:52:20 dimas Exp $
    $Log: Client.cpp,v $
    Revision 1.6  2009/12/24 10:52:20  dimas
    Check against Exclude Files list implemented

    Revision 1.5  2009/12/21 17:00:13  dimas
    1. DEFRAG_CMD_GET_MOST_FRAGMENTED and similar requests implemented
    2. Basic Exclude files check added

    Revision 1.4  2009/12/16 14:13:34  dimas
    DEFRAG_CMD_GET_FILE_INFO request implemented

    Revision 1.3  2009/12/03 13:01:06  dimas
    Cluster Inspector implemented

    Revision 1.2  2009/12/02 14:42:15  dimas
    App/Service interaction improved

    Revision 1.1  2009/11/24 14:51:31  dimas
    no message

*/

#include "stdafx.h"


typedef
NTSTATUS (NTAPI *_ZwConnectPort_) (
    OUT PHANDLE                     ClientServerPort,
    IN PUNICODE_STRING              ServerPortName,
    IN PSECURITY_QUALITY_OF_SERVICE SecurityQos,
    IN OUT PLPC_THIS_SIDE_MEMORY    ClientSharedMemory OPTIONAL,
    IN OUT PLPC_OTHER_SIDE_MEMORY   ServerSharedMemory OPTIONAL,
    OUT PULONG                      MaximumMessageLength OPTIONAL,
    IN OUT PVOID                    ConnectionInfo OPTIONAL,
    IN OUT PULONG                   ConnectionInfoLength OPTIONAL
);

_ZwConnectPort_ ZwConnectPort;

typedef
VOID 
(NTAPI *_RtlInitUnicodeString_ ) (
    IN OUT PUNICODE_STRING  DestinationString,
    IN PCWSTR  SourceString
    );


_RtlInitUnicodeString_ iRtlInitUnicodeString;

typedef
NTSTATUS (NTAPI *_NtClose_) (
    HANDLE Handle
    );

_NtClose_ iNtClose;

typedef
NTSTATUS 
(NTAPI *_ZwRequestWaitReplyPort_) ( 
    IN HANDLE           ServerPort,
    IN PLPC_MESSAGE     Request,
    OUT PLPC_MESSAGE    Reply
    );

_ZwRequestWaitReplyPort_ ZwRequestWaitReplyPort;


HMODULE hdll;

int 
__stdcall 
loadLibrary(void)
{

    InitializeCriticalSection( &(Global.CommSync) ); 

    hdll = ::LoadLibrary(L"ntdll.dll");

    if ( hdll )
    {
        ZwConnectPort           = NULL;
        iRtlInitUnicodeString   = NULL;
        iNtClose                = NULL;
        ZwRequestWaitReplyPort  = NULL;

        ZwConnectPort = (_ZwConnectPort_)::GetProcAddress(hdll, "ZwConnectPort");
        iRtlInitUnicodeString  = (_RtlInitUnicodeString_)::GetProcAddress(hdll, "RtlInitUnicodeString");
        iNtClose               = (_NtClose_)::GetProcAddress(hdll, "NtClose");
        ZwRequestWaitReplyPort = (_ZwRequestWaitReplyPort_)::GetProcAddress(hdll, "ZwRequestWaitReplyPort");

        if ( ZwConnectPort && iRtlInitUnicodeString && iNtClose && ZwRequestWaitReplyPort ) 
        {
            return 0;
        }
    }

	return 1;
}


void 
__stdcall 
unloadLibrary(void)
{
    if ( hdll )
    {
        ::FreeLibrary( hdll );
        hdll = NULL;
    }

    DeleteCriticalSection( &(Global.CommSync) );
}



#ifdef _DEBUG

WCHAR   *MessTypeStr[] = 
{
    L"LPC_NEW_MESSAGE",
    L"LPC_REQUEST",
    L"LPC_REPLY",
    L"LPC_DATAGRAM",
    L"LPC_LOST_REPLY",
    L"LPC_PORT_CLOSED",
    L"LPC_CLIENT_DIED",
    L"LPC_EXCEPTION",
    L"LPC_DEBUG_EVENT",
    L"LPC_ERROR_EVENT",
    L"LPC_CONNECTION_REQUEST"
};

void 
__stdcall
FormatLpcMsg( 
    WCHAR           *buf, 
    BOOLEAN         ServerSide,
    WCHAR           *prefix, 
    PORT_MESSAGE    *msg, 
    DWORD           ReqRes,
    DWORD           JobId,
    WCHAR           *postfix )
{

   if ( !buf || !msg ) return;

   swprintf( buf, L"%ls %ls Size[%d:%d]  Type %ls (%02d) :: %ls %d  JobID %08X ", //(virtOff %04d) ", // Client %08lX:%08lX 
                  ServerSide ? L"-->" : L"<<-",
                  prefix ? prefix : L"",
                  (int)msg->DataSize, (int)msg->MessageSize, 
                  (msg->MessageType >= LPC_NEW_MESSAGE && msg->MessageType <= LPC_CONNECTION_REQUEST) ? 
                    MessTypeStr[msg->MessageType]:L"Unknown", msg->MessageType,
                  //msg->MessageId,
                  ServerSide ? L"request" : L"result",
                  ReqRes,
                  JobId );
                  //msg->VirtualRangesOffset );
                  //msg->ClientId.UniqueProcess, msg->ClientId.UniqueThread );

   if ( postfix ) {
      wcscat( buf, L"  '" );
      wcscat( buf, postfix );
      wcscat( buf, L"'" );
   }


}

#endif // _DEBUG


int 
__stdcall
OpenConnection( void )
{
    int                             Result = ERROR_INVALID_PARAMETER;
    NTSTATUS                        Status = STATUS_SUCCESS;

    // port
    UNICODE_STRING                  PortName;
    SECURITY_QUALITY_OF_SERVICE     Sqos = { sizeof(Sqos), SecurityImpersonation, TRUE, TRUE };

    PORT_CONNECTION_INFO            ConnectionInfo;
    DWORD                           MaxMessageLength, DataLength;

    // data sections
    PORT_SECTION_WRITE              ClientSection = { sizeof(ClientSection), 0, 0, 0, 0, 0 };
    PORT_SECTION_READ               ServerSection = { sizeof(ServerSection), 0, 0 };

    BOOLEAN                         IsInCriticalSection = FALSE;


    WCHAR                           Mess[MAX_PATH];


__try
{

    EnterCriticalSection( &(Global.CommSync) );
    IsInCriticalSection = TRUE;

#ifdef _DEBUG
    swprintf( Mess, L"Init Port Name '%ls'", DFRG_PORT_NAME );
    OutMessage( Mess );
#endif

    //
    // Init port name
    //
    iRtlInitUnicodeString ( &PortName, DFRG_PORT_NAME );

    //
    // Prepare buffers.
    // Create Client memory section.
    //
    Global.ClientSectionHandle = CreateFileMapping( 
                            INVALID_HANDLE_VALUE, 
                            NULL, 
                            PAGE_READWRITE, 
                            0, 
                            CLIENT_SECTION_SIZE, 
                            NULL );

    if ( Global.ClientSectionHandle == NULL ) 
    {
        swprintf( Mess, L"Client could not create FileMapping" );
        Result = DislpayErrorMessage( Mess, Result, ERR_OS );
        __leave;
    }

    Global.ClientBuffer = (PBYTE)MapViewOfFile( 
                            Global.ClientSectionHandle,
                            FILE_MAP_WRITE,
                            0, 0, SERVER_SECTION_SIZE );

    if ( !Global.ClientBuffer )
    {
        OutMessage( L"MapViewOfFile error" );
        Result = ERROR_NOT_ENOUGH_MEMORY;
        __leave;
    }

    ClientSection.SectionHandle = Global.ClientSectionHandle;
    ClientSection.SectionOffset = 0;
    ClientSection.ViewSize      = CLIENT_SECTION_SIZE;


    //
    // Open port
    //
#ifdef _DEBUG
    swprintf( Mess, L"Try ConnectPort '%ls'", PortName.Buffer );
    OutMessage( Mess );
#endif

    Global.PortHandle = NULL;

    ConnectionInfo.Version = DEFRAG_EXTERNAL_PROTOCOL_VERSION;
    wcscpy( ConnectionInfo.TextMessage, L"Connection Request" );

    DataLength = sizeof(PORT_CONNECTION_INFO);

    Status = ZwConnectPort( 
                    &Global.PortHandle, 
                    &PortName, 
                    &Sqos, 
                    &ClientSection, 
                    &ServerSection, 
                    &MaxMessageLength, 
                    &ConnectionInfo, 
                    &DataLength );

    ConnectionInfo.TextMessage[ARRAYSIZE(ConnectionInfo.TextMessage)-1] = L'\0';

    if ( Status != STATUS_SUCCESS ) 
    {
        swprintf( Mess, _T("ZwConnectPort %ls : %08X, '%ls'. "), 
            PortName.Buffer, Global.PortHandle, ConnectionInfo.TextMessage );
        Result = DislpayErrorMessage( Mess, Result, ERR_NT_STATUS );
        __leave;
    }
#ifdef _DEBUG
    else 
    {
        ConnectionInfo.TextMessage[ARRAYSIZE(ConnectionInfo.TextMessage)-1] = L'\0';
        swprintf( Mess,  _T("Connect Port %08lX. Max msg size= %d  reply data: [%d] '%ls', SrvBuf %p"), 
                         Global.PortHandle, MaxMessageLength, DataLength, ConnectionInfo.TextMessage, ServerSection.ViewBase );
        OutMessage( Mess );
    }
#endif

    if ( DEFRAG_EXTERNAL_PROTOCOL_VERSION != ConnectionInfo.Version )
    {
#ifdef _DEBUG
        swprintf( Mess, L"Wrong IF versions: %08X != %08X. Connection Refused", 
            DEFRAG_EXTERNAL_PROTOCOL_VERSION, ConnectionInfo.Version );
        OutMessage( Mess );
#endif
        Result = ERROR_NOT_SUPPORTED;
        __leave;
    }

    //
    // Set Server (Output) Buffer addr & length
    //
    Global.ServerBuffer = (PBYTE)ServerSection.ViewBase;
    Global.BufferLength = (ULONG)ServerSection.ViewSize;

    Result = NO_ERROR;

}
__except(EXCEPTION_EXECUTE_HANDLER)
{
    Result = GetExceptionCode();
    DislpayErrorMessage( L"OpenConnection Exception !!!", Result, ERR_OS );
}


    if ( IsInCriticalSection )
    {
        LeaveCriticalSection( &(Global.CommSync) );
    }


    return Result;

} // end of OpenConnection


//
// Close Connection
//
void 
__stdcall
CloseConnection( void )
{

    BOOLEAN IsInCriticalSection = FALSE;


    EnterCriticalSection( &(Global.CommSync) );
    IsInCriticalSection = TRUE;

    //
    // Delete mapped section
    //
    if ( Global.ClientBuffer )
    {
        UnmapViewOfFile( Global.ClientBuffer );
        Global.ClientBuffer = NULL;
    }

    if ( Global.ClientSectionHandle ) 
    {
        CloseHandle( Global.ClientSectionHandle );
        Global.ClientSectionHandle = NULL;
    }

    //
    // Close Connection Port Handle
    //
#ifdef _DEBUG
    OutMessage( L"Close Port" );
#endif

    if ( Global.PortHandle ) 
    { 
        iNtClose( Global.PortHandle );
        Global.PortHandle = NULL;
    }

    if ( IsInCriticalSection )
    {
        LeaveCriticalSection( &(Global.CommSync) );
    }

}

int 
__stdcall
SendCommandEx( 
    IN  OUT DWORD           *JobId, 
    IN      DWORD           Command,
    IN      PVOID           ControlData,
    IN      PVOID           InputData,
    IN      ULONG           InputDataLength,
    IN  OUT PVOID           OutputData,
    IN  OUT ULONG           *OutputDataLength,
    OUT     DWORD           *JobStatus,
    OUT     DWORD           *JobProgress )
{
    int                     Result = ERROR_INVALID_PARAMETER;
    NTSTATUS                Status = STATUS_SUCCESS;

    PORT_MESSAGEX2          Reply, Request;
    PSETTINGS_INFO          Setting = NULL;
    PCLUSTERS_INFO          Inspect = NULL;

    WCHAR                   Mess[MAX_PATH];
#ifdef _DEBUG
    static DWORD            MsgCnt = 0;
#endif


    BOOLEAN IsInCriticalSection = FALSE;

    __try
    {
        EnterCriticalSection( &(Global.CommSync) );
        IsInCriticalSection = TRUE;

        if ( !Command )
        {
            OutMessage ( L"SendCommandEx(). Wrong command" );
            __leave;
        }

        //
        // Init request
        //
        memset( &Request, 0, sizeof(Request) );
        memset( &Reply, 0, sizeof(Reply) );

        Request.MessageType = LPC_NEW_MESSAGE;
        Request.MessageSize = sizeof(Request);
        Request.VirtualRangesOffset = 0;

        Request.Command = Command;

        Request.JobId = *JobId;

        switch ( Command )
        {
        case DEFRAG_CMD_INIT_JOB:

            Request.JobId = EMPTY_JOB_ID;

            wcscpy( Request.DriveName, (WCHAR*)ControlData );

            break;

        case DEFRAG_CMD_GET_SETTING:
        case DEFRAG_CMD_SET_SETTING:

            Request.JobId = GLOBAL_JOB_ID;

            Setting = (PSETTINGS_INFO)ControlData;

            Request.Setting.SettingId = Setting->SettingId;
            Request.Setting.SettingOp = Setting->SettingOp;
            //Request.Setting.DataLength = Setting->DataLength;

            break;

        case DEFRAG_CMD_GET_CLUSTER_FILES:

            Inspect = (PCLUSTERS_INFO)ControlData;

#ifdef _DEBUG
            swprintf( Mess, L"GET_CLUSTER_FILES %I64d [%I64d]",
                Inspect->StartCluster, Inspect->ClustersNumber );
            OutMessage ( Mess );
#endif
            Request.ClustersBlock.StartCluster      = Inspect->StartCluster;
            Request.ClustersBlock.ClustersNumber    = Inspect->ClustersNumber;

            break;

        default:

            break;
        }

        Request.InputDataLength = InputDataLength;

        if ( OutputDataLength )
        {
            Request.OutputDataLength = *OutputDataLength;
        }

        //
        // Copy input data
        //
        if ( InputDataLength && InputData )
        {
            if ( InputDataLength > CLIENT_SECTION_SIZE )
            {
                Result = ERROR_BUFFER_OVERFLOW;
                OutMessage ( L"SendCommandEx(). Too long input data" );
                __leave;
            }

            memcpy( Global.ClientBuffer, InputData, InputDataLength );
        }

        Request.DataSize = sizeof( Request.RawData );

        Reply.MessageType = LPC_PORT_CLOSED;


        //
        // Send message & wait reply
        //
        Status = ZwRequestWaitReplyPort( 
                    Global.PortHandle, 
                    (PORT_MESSAGE*)&Request, 
                    (PORT_MESSAGE*)&Reply );

        if ( Status != STATUS_SUCCESS ) 
        {
            swprintf( Mess, L"ZwRequestWaitReplyPort" );
            Result = DislpayErrorMessage( Mess, Status, ERR_NT_STATUS );
            __leave;
        }

        if ( Reply.MessageType == LPC_PORT_CLOSED || 
             Reply.MessageType == LPC_CLIENT_DIED )
        {
#ifdef _DEBUG
            OutMessage ( L"\r\nServer port closed or died - break" );
#endif
            Result = DFRG_ERROR_ABORTED;
            __leave;
        }


        if ( Reply.MessageType != LPC_REPLY ) 
        {
            __leave;
        }

#ifdef _DEBUGW
        FormatLpcMsg( Mess, FALSE, L"", (PORT_MESSAGE*)&Reply, Reply.Result, Reply.JobId, NULL );
        OutMessage( Mess );
#endif

        Result = Reply.Result;

        if ( DEFRAG_CMD_INIT_JOB == Command )
        {
            *JobId = Reply.JobId;
        }

        if ( JobStatus )
        {
            *JobStatus = Reply.JobStatus;
        }

        if ( JobProgress )
        {
            *JobProgress = Reply.JobProgress;
        }
        
        if ( OutputDataLength )
        {
            *OutputDataLength = Reply.OutputDataLength;
        }
        
        //
        // Copy output data
        //
        if ( OutputDataLength && *OutputDataLength && OutputData )
        {
            if ( Reply.OutputDataLength > *OutputDataLength )
            {
                Result = ERROR_BUFFER_OVERFLOW; // ERROR_NOT_ENOUGH_MEMORY;
            }
            else
            {
                memcpy( OutputData, Global.ServerBuffer, Reply.OutputDataLength );
            }
        }

    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        Result = GetExceptionCode();
        DislpayErrorMessage( L"SendCommandEx Exception !!!", Result, ERR_OS );
    }

    if ( IsInCriticalSection )
    {
        LeaveCriticalSection( &(Global.CommSync) );
    }


    return Result;

} // end of SendCommandEx


#define     WAIT_EVENT_NUM      2


int 
__stdcall
WaitJobFinished( 
    DWORD       *JobId,
    DWORD       *JobStatus,
    DWORD       *JobProgress )
{
    int                             Result = ERROR_INVALID_PARAMETER;
    NTSTATUS                        Status = STATUS_SUCCESS;

    // syncro
    HANDLE                          Hevnt[WAIT_EVENT_NUM];
    DWORD                           WaitResult, EventsNum = 0;
    HANDLE                          TestTimer = NULL;
    DWORD                           TimerPeriod;
    LARGE_INTEGER                   TimerStart;

    DWORD                           Command = 0;

    WCHAR                           Mess[MAX_PATH];


    __try
    {
        //
        // Start timer
        //
        ZeroMemory( Hevnt, sizeof(HANDLE)*WAIT_EVENT_NUM );

        TestTimer = CreateWaitableTimer( NULL, FALSE, NULL );
        if ( !TestTimer ) 
        {
            swprintf( Mess, L"CreateWaitableTimer error" );
            DislpayErrorMessage( Mess, Result, ERR_OS );
            __leave;
        }

        TimerStart.QuadPart = -20000000; // 10000000 = 1 sec
        TimerPeriod = 2000;              // ms, 1000 = 1 sec

        if ( !SetWaitableTimer( TestTimer, &TimerStart, TimerPeriod, NULL, NULL, 0 ) ) 
        {
            swprintf( Mess, L"SetWaitableTimer error" );
            DislpayErrorMessage( Mess, Result, ERR_OS );
            __leave;
        }
        else 
        {
            Hevnt[EventsNum] = TestTimer;
            EventsNum++;
        }


        while( TRUE ) 
        {
            WaitResult = MsgWaitForMultipleObjects( 
                                EventsNum, 
                                Hevnt, 
                                FALSE, 
                                INFINITE, 
                                QS_POSTMESSAGE );

            if ( WaitResult-WAIT_OBJECT_0 < EventsNum ) 
            {
                *Mess = 0;

                if ( WaitResult-WAIT_OBJECT_0 == 0 ) 
                { // timer

#ifdef _DEBUGW
                    SYSTEMTIME  st1;
                    GetLocalTime( &st1 );
                    swprintf( Mess, _T("Tik %02d.%02d: "), st1.wMinute, st1.wSecond );
                    OutMessage( Mess );
#endif
                }

                Command = DEFRAG_CMD_GET_JOB_STATUS;
                Result = SendCommandEx( 
                                JobId, 
                                Command, 
                                NULL, 
                                NULL, 
                                NULL, 
                                NULL, 
                                NULL, 
                                JobStatus, 
                                JobProgress );
                if ( NO_ERROR != Result || 
                    !((*JobStatus) & DEFRAG_JOB_FLAG_INPROGRESS) ||
                    ((*JobStatus) & DEFRAG_JOB_FLAG_STOPPED) )
                {
                    __leave;
                }
            }
            else if ( WaitResult-WAIT_OBJECT_0 == EventsNum ) // message
            { 
                MSG       Msg;

                PeekMessage( &Msg, (HWND)(-1), WM_QUIT, WM_QUIT, PM_REMOVE );
                if ( Msg.message == WM_QUIT ) 
                {
#ifdef _DEBUG
                    OutMessage ( L"\r\nQuit message- break" );
#endif
                    break;
                }
            } // if - event switch
#ifdef _DEBUG
            else 
            {
                OutMessage ( L"Unknown event" );
            }
#endif
        }  // while
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        Result = GetExceptionCode();
        DislpayErrorMessage( L"WaitJobFinished Exception !!!", Result, ERR_OS );
    }

    if ( TestTimer ) 
    {
        CancelWaitableTimer( TestTimer );
        CloseHandle( TestTimer );
        TestTimer = NULL;
    }

    return Result;

} // end of WaitJobFinished



//#ifndef DEFRAG_GUI_APP
//#if (defined DEFRAG_ENGINE_SERVICE) && (defined _DEBUG)
#if !(defined DEFRAG_GUI_APP) && !(defined DEFRAG_NATIVE_APP) && (defined _DEBUG)
//
// Use Service functionality in Command line mode
//

int 
__stdcall
RunServiceJob( void )
{
    int                             Result = ERROR_INVALID_PARAMETER;
    NTSTATUS                        Status = STATUS_SUCCESS;

    DWORD                           Command = 0, JobId = 0;
    DWORD                           JobStatus, JobProgress;

    PDFRG_SHEDULED_JOB              Job = NULL;

#ifdef _DEBUG
    WCHAR                           Mess[MAX_PATH];
#endif


__try
{

    //
    // Load ntdll.dll
    //
    if ( loadLibrary() ) 
    {
        OutMessage( L"Load library error!" );
        __leave;
    }

    //
    // Open connection
    //
    Result = OpenConnection();
    if ( NO_ERROR != Result )
    {
        __leave;
    }
#ifdef _DEBUGSW
    else 
    {
        __leave;
    }
#endif

    //
    // Test connection
    //
    //Command = 0;
    ////Result = SendCommand( &JobId, Command, Global.JobList->DiskName, &JobStatus, NULL );
    //Result = SendCommand( &JobId, Command, Global.JobList->VolumeCtx.VName, &JobStatus, NULL );
    //if ( NO_ERROR != Result )
    //{
    //    __leave;
    //}

#define TEST_SETTINGS
#ifdef TEST_SETTINGS
    //
    // Test settings
    //
    SETTINGS_INFO   Setting;
    ULONG           OutputLength;

    //
    // Get Current MaxFileSize
    //
    Setting.SettingId = MaxFileSizeId;
    Setting.SettingOp = OP_GET;
    //Setting.DataLength = sizeof( DfrgSettings.MaxFileSize );
    OutputLength = sizeof( DfrgSettings.MaxFileSize );

    JobId = GLOBAL_JOB_ID;
    Command = DEFRAG_CMD_GET_SETTING;

    Result = SendCommandEx( 
                &JobId, 
                Command, 
                &Setting, 
                NULL, 
                NULL, 
                &DfrgSettings.MaxFileSize, 
                &OutputLength,
                NULL, 
                NULL );
                //&JobStatus, 
                //&JobProgress );

    //DfrgSettings.MaxFileSize = *((DWORD*)Global.ServerBuffer);

#ifdef _DEBUG
    swprintf( Mess, L" Get MaxFileSize  res %d, value %d", Result, DfrgSettings.MaxFileSize );
    OutMessage( Mess );
#endif

    //
    // Set MaxFileSize
    //
    Setting.SettingId = MaxFileSizeId;
    Setting.SettingOp = OP_SET;
    //Setting.DataLength = sizeof( DfrgSettings.MaxFileSize );
    //*((DWORD*)Global.ClientBuffer) = DfrgMaxFileSizeDef/2;

    DWORD       FileSize = 
        (DfrgSettings.MaxFileSize == DfrgMaxFileSizeDef) ? DfrgMaxFileSizeDef/2 : DfrgMaxFileSizeDef;

    JobId = GLOBAL_JOB_ID;
    Command = DEFRAG_CMD_SET_SETTING;

    //Result = SendCommand( &JobId, Command, &Setting, &JobStatus, &JobProgress );
    Result = SendCommandEx( 
                &JobId, 
                Command, 
                &Setting, 
                &FileSize, 
                sizeof( FileSize ), 
                NULL, 
                NULL, 
                NULL, 
                NULL );

#ifdef _DEBUG
    swprintf( Mess, L" Set MaxFileSize  res %d, value %d", Result, FileSize );
    OutMessage( Mess );
#endif

    //
    // Get New MaxFileSize
    //
    Setting.SettingId = MaxFileSizeId;
    Setting.SettingOp = OP_GET;
    //Setting.DataLength = sizeof( DfrgSettings.MaxFileSize );

    JobId = GLOBAL_JOB_ID;
    Command = DEFRAG_CMD_GET_SETTING;
    OutputLength = sizeof( DfrgSettings.MaxFileSize );

    Result = SendCommandEx( 
                &JobId, 
                Command, 
                &Setting, 
                NULL, 
                NULL, 
                &DfrgSettings.MaxFileSize, 
                &OutputLength, 
                NULL, 
                NULL );

    //DfrgSettings.MaxFileSize = *((DWORD*)Global.ServerBuffer);

#ifdef _DEBUG
    swprintf( Mess, L" Get MaxFileSize  res %d, value %d == %d", 
        Result, DfrgSettings.MaxFileSize, FileSize );
    OutMessage( Mess );
    GetInput();
#endif

#endif // TEST_SETTINGS


//#define TEST_SHEDULED_JOBS
#ifdef TEST_SHEDULED_JOBS
    //
    // Test Sheduled Jobs
    //
    //PDFRG_SHEDULED_JOB       Job = (PDFRG_SHEDULED_JOB)Global.ClientBuffer;
    Job = (PDFRG_SHEDULED_JOB)malloc( Global.BufferLength );

    //
    // Get Sheduled Jobs
    //
    Setting.SettingId = SheduledJobsId;
    Setting.SettingOp = OP_GET;
    //Setting.DataLength = Global.BufferLength;

    JobId = GLOBAL_JOB_ID;
    Command = DEFRAG_CMD_GET_SETTING;
    OutputLength = Global.BufferLength;

    Result = SendCommandEx( 
                &JobId, 
                Command, 
                &Setting, 
                NULL, 
                NULL, 
                Job, 
                &OutputLength, 
                NULL, 
                NULL );

    //Job = (PDFRG_SHEDULED_JOB)Global.ServerBuffer;

#ifdef _DEBUG
    //swprintf( Mess, L" Get %d Jobs:  '%ls', '%ls', res %d", 
    //    Setting.DataLength / sizeof(DFRG_SHEDULED_JOB), (Job+0)->Name, (Job+1)->Name, Result );
    swprintf( Mess, L" Get %d Jobs (res %d):", 
        OutputLength / sizeof(DFRG_SHEDULED_JOB), Result );
    if ( !Result )
    {
        for( ULONG i=0; i<OutputLength / sizeof(DFRG_SHEDULED_JOB); i++ )
        {
            swprintf( Mess+wcslen(Mess), L" '%ls',", (Job+i)->Name );
        }
    }
    OutMessage( Mess );
#endif

    //
    // Add / Update 3 Sheduled Jobs
    //
    //Job = (PDFRG_SHEDULED_JOB)Global.ClientBuffer;

    wcscpy( (Job+0)->Name, L"Job 0" );
    wcscpy( (Job+1)->Name, L"Job 1" );
    wcscpy( (Job+2)->Name, L"Job 2" );

    Setting.SettingId = SheduledJobsId;
    Setting.SettingOp = OP_ADD;
    //Setting.DataLength = sizeof(DFRG_SHEDULED_JOB) * 3;

    JobId = GLOBAL_JOB_ID;
    Command = DEFRAG_CMD_SET_SETTING;
    OutputLength = sizeof(DFRG_SHEDULED_JOB) * 3;

    Result = SendCommandEx( 
                &JobId, 
                Command, 
                &Setting, 
                Job, 
                sizeof(DFRG_SHEDULED_JOB) * 3, 
                NULL, 
                NULL, 
                NULL, 
                NULL );

#ifdef _DEBUG
    swprintf( Mess, L" Set %d Jobs  '%ls'..'%ls', res %d", 
        OutputLength / sizeof(DFRG_SHEDULED_JOB), (Job+0)->Name, (Job+2)->Name, Result );
        //Setting.DataLength / sizeof(DFRG_SHEDULED_JOB), (Job+0)->Name, (Job+2)->Name, Result );
    OutMessage( Mess );
#endif

    //
    // Delete Sheduled Job #1
    //
    //Job = (PDFRG_SHEDULED_JOB)Global.ClientBuffer;

    wcscpy( (Job+0)->Name, L"Job 1" );

    Setting.SettingId = SheduledJobsId;
    Setting.SettingOp = OP_REMOVE;
    //Setting.DataLength = sizeof(DFRG_SHEDULED_JOB);

    JobId = GLOBAL_JOB_ID;
    Command = DEFRAG_CMD_SET_SETTING;

    Result = SendCommandEx(
                &JobId, 
                Command, 
                &Setting, 
                Job, 
                sizeof(DFRG_SHEDULED_JOB), 
                NULL, 
                NULL, 
                NULL, 
                NULL );

#ifdef _DEBUG
    swprintf( Mess, L" Delete Job  '%ls', res %d", (Job+0)->Name, Result );
    OutMessage( Mess );
#endif

    //
    // Get Sheduled Jobs
    //
    Setting.SettingId = SheduledJobsId;
    Setting.SettingOp = OP_GET;
    //Setting.DataLength = Global.BufferLength;

    JobId = GLOBAL_JOB_ID;
    Command = DEFRAG_CMD_GET_SETTING;
    OutputLength = Global.BufferLength;

    Result = SendCommandEx(
                &JobId, 
                Command, 
                &Setting, 
                NULL, 
                NULL, 
                Job, 
                &OutputLength, 
                NULL, 
                NULL );

    //Job = (PDFRG_SHEDULED_JOB)Global.ServerBuffer;

#ifdef _DEBUG
    swprintf( Mess, L" Get %d Jobs:  '%ls', '%ls', res %d", 
        OutputLength / sizeof(DFRG_SHEDULED_JOB), (Job+0)->Name, (Job+1)->Name, Result );
        //Setting.DataLength / sizeof(DFRG_SHEDULED_JOB), (Job+0)->Name, (Job+1)->Name, Result );
    OutMessage( Mess );
    GetInput();
#endif

#endif // TEST_SHEDULED_JOBS



//#define TEST_DEFRAG_JOB
#ifdef TEST_DEFRAG_JOB
    //
    // Test Interactive Jobs.
    //

    //
    // Send DfrgInitJobCtx command to Service.
    //
    Command = DEFRAG_CMD_INIT_JOB;
    //Result = SendCommand( &JobId, Command, Global.JobList->DiskName, &JobStatus, &JobProgress );
    Result = SendCommand( &JobId, Command, Global.JobList->VolumeCtx.VName, &JobStatus, &JobProgress );
    if ( NO_ERROR != Result )
    {
        __leave;
    }

    //
    // Send AnalizeVolume command to Service
    //
    Command = DEFRAG_CMD_ANALIZE_VOLUME;
    Result = SendCommand( &JobId, Command, NULL, &JobStatus, &JobProgress );
    if ( NO_ERROR != Result )
    {
        __leave;
    }

    //
    // Wait Analize finished
    //
    Result = WaitJobFinished( &JobId, &JobStatus, &JobProgress );
    if ( NO_ERROR != Result )
    {
        __leave;
    }

    //
    // Get Volume info
    //
    GetInput();

    Command = DEFRAG_CMD_GET_STATISTIC;
    Result = SendCommand( &JobId, Command, &Setting, &JobStatus, &JobProgress );

    PDFRG_VOLUME_INFO VolumeInfo = (PDFRG_VOLUME_INFO)Global.ServerBuffer;

#ifdef _DEBUG
    swprintf( Mess, L" Get Volume info, res %d :", Result );
    OutMessage( Mess );
    swprintf( Mess, L"    files %I64d, dirs %I64d, unmove %I64d, fragmented %I64d, maxgap %I64d,  ", 
        VolumeInfo->Statistic.FilesNum, VolumeInfo->Statistic.DirsNum, VolumeInfo->Statistic.UnmovableFilesNum, 
        VolumeInfo->Statistic.FragmentedFilesNum, VolumeInfo->Statistic.MaxFreeSpaceGap );
    OutMessage( Mess );
    GetInput();
#endif

    if ( NO_ERROR != Result )
    {
        __leave;
    }

/*
    //
    // Send Defrag command to Service
    //
    if ( DEFRAG_JOB_ANALIZE_ONLY != Global.JobList->Type )
    {
        switch ( Global.JobList->Type )
        {

        case DEFRAG_JOB_SIMPLE_DEFRAG:

            Command = DEFRAG_CMD_SIMPLE_DEFRAG;
            break;

        case DEFRAG_JOB_DEBUG1:

            Command = DEFRAG_CMD_DEBUG1;
            break;

        case DEFRAG_JOB_FREE_SPACE:

            Command = DEFRAG_CMD_FREE_SPACE;
            break;

        case DEFRAG_JOB_ANALIZE_ONLY:
        default:

            Command = DEFRAG_CMD_STOP_JOB;
            break;
        }

        Result = SendCommand( &JobId, Command, NULL, &JobStatus, &JobProgress );
        if ( NO_ERROR != Result )
        {
            __leave;
        }

        Result = WaitJobFinished( &JobId, &JobStatus );
        if ( NO_ERROR != Result )
        {
            __leave;
        }
    }
*/
    //
    // Send StopJob command to Service
    //
    Command = DEFRAG_CMD_STOP_JOB;
    Result = SendCommand( &JobId, Command, NULL, &JobStatus, &JobProgress );
    if ( NO_ERROR != Result )
    {
        __leave;
    }

    //
    // Send DeleteJob command to Service
    //
    Command = DEFRAG_CMD_DELETE_JOB;
    Result = SendCommand( &JobId, Command, NULL, &JobStatus, &JobProgress );
    if ( NO_ERROR != Result )
    {
        __leave;
    }
#endif // TEST_DEFRAG_JOB


#define TEST_FILE_INFO
#ifdef TEST_FILE_INFO
    //
    // Test ClusterInspector & 
    //

    //
    // Send DfrgInitJobCtx command to Service.
    //
    Command = DEFRAG_CMD_INIT_JOB;
    JobId = GLOBAL_JOB_ID;
    OutputLength = Global.BufferLength;

    Result = SendCommandEx(
                &JobId, 
                Command, 
                Global.JobList->VolumeCtx.VName, 
                NULL, 
                NULL, 
                Job, 
                &OutputLength, 
                NULL, 
                NULL );
    if ( NO_ERROR != Result )
    {
        __leave;
    }

    //
    // Send AnalizeVolume command to Service
    //
    Command = DEFRAG_CMD_ANALIZE_VOLUME;
    Result = SendCommandEx( &JobId, Command, NULL, NULL, NULL, NULL, NULL, NULL, NULL );
    if ( NO_ERROR != Result )
    {
        __leave;
    }

    //
    // Wait Analize finished
    //
    Result = WaitJobFinished( &JobId, &JobStatus, &JobProgress );
    if ( NO_ERROR != Result )
    {
        __leave;
    }

    //
    // Send StopJob command to Service
    //
    Command = DEFRAG_CMD_STOP_JOB;
    Result = SendCommandEx( &JobId, Command, NULL, NULL, NULL, NULL, NULL, &JobStatus, &JobProgress );
    if ( NO_ERROR != Result )
    {
        __leave;
    }

    //
    // Get Files list
    //
    do
    {
        OutMessage( L"\r\nTest Files list" );
        if ( -1 == GetInput() )
        {
            break;
        }

        Command = DEFRAG_CMD_GET_MOST_FRAGMENTED;
        OutputLength = Global.BufferLength;
        Result = SendCommandEx(
                    &JobId, 
                    Command, 
                    NULL, 
                    NULL, 
                    NULL, 
                    NULL, 
                    &OutputLength, 
                    NULL, 
                    NULL );

#ifdef _DEBUG
        swprintf( Mess, L" Get File list [%d], err %d", 
            OutputLength/sizeof(DEFRAG_FILE_LIST), Result );
        OutMessage( Mess );
#endif

        if ( NO_ERROR != Result )
        {
            continue;
        }

#ifdef _DEBUG
        PDEFRAG_FILE_LIST   FileList = (PDEFRAG_FILE_LIST)Global.ServerBuffer;

        for ( DWORD i=0; i<OutputLength/sizeof(DEFRAG_FILE_LIST); i++, FileList++ )
        {
            swprintf( Mess, L"   '%ls' : [%d] %08X %08X ", 
                FileList->FileName, FileList->ExtentsNum, FileList->DefragFlags, FileList->DefragFlags2 );
            OutMessage( Mess );
        }
#endif
    } while ( TRUE );

    //
    // Get File info
    //
    do
    {
        OutMessage( L"\r\nTest Files Info" );
        if ( -1 == GetInput() )
        {
            break;
        }

        WCHAR   FileName[] = L"C:\\Exchange\\isetup-5.1.13.exe";
        OutputLength = Global.BufferLength;

        Command = DEFRAG_CMD_GET_FILE_INFO;
        Result = SendCommandEx(
                    &JobId, 
                    Command, 
                    NULL, 
                    FileName, 
                    (ULONG)(wcslen(FileName)+1)*sizeof(WCHAR), 
                    NULL, 
                    &OutputLength, 
                    NULL, 
                    NULL );

        PDEFRAG_FILE_INFO FileInfo = (PDEFRAG_FILE_INFO)Global.ServerBuffer;

#ifdef _DEBUG
        swprintf( Mess, L" Get File '%ls' info. StreamsNum %d. Result %d", 
            FileInfo->FileName, FileInfo->StreamsNum, Result );
        OutMessage( Mess );
#endif

        if ( NO_ERROR != Result )
        {
            continue;
        }

#ifdef _DEBUG
        for ( DWORD i=0; i<FileInfo->StreamsNum; i++ )
        {
            swprintf( Mess, L"   Stream %d  name '%ls' Extents %d  ",
                i, 
                FileInfo->Stream[i].StreamName ? FileInfo->Stream[i].StreamName : L"", 
                FileInfo->Stream[i].ExtentsNum );
            OutMessage( Mess );

            for ( DWORD j=0; j<FileInfo->Stream[i].ExtentsNum; j++ )
            {
                swprintf( Mess, L"      Extent %d  Ext[0]: Vcn %I64d  Lcn %I64d [%I64d]",
                    j,
                    FileInfo->Stream[i].Extent[j].StartVcn.QuadPart, 
                    FileInfo->Stream[i].Extent[j].StartLcn.QuadPart, 
                    FileInfo->Stream[i].Extent[j].Length.QuadPart );
                OutMessage( Mess );
            }
        }
#endif
    } while ( TRUE );

    //
    // Send DeleteJob command to Service
    //
    Command = DEFRAG_CMD_DELETE_JOB;
    Result = SendCommandEx( &JobId, Command, NULL, NULL, NULL, NULL, NULL, &JobStatus, &JobProgress );
    if ( NO_ERROR != Result )
    {
        __leave;
    }


#endif // TEST_FILE_INFO




}
__except(EXCEPTION_EXECUTE_HANDLER)
{
    Result = GetExceptionCode();
    DislpayErrorMessage( L"RunServiceJob Exception !!!", Result, ERR_OS );
}

    //
    // Close Connection
    //
    CloseConnection();

    //
    // Unload ntdll.dll
    //
    unloadLibrary();

    if ( Job )
    {
        free( Job );
    }


    return Result;

} // end of RunServiceJob


//#endif // DEFRAG_GUI_APP
#endif // DEFRAG_GUI_APP
