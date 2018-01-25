/*
    Defrag Sheduler

    Module name:

        Sheduler.cpp

    Abstract:

        Sheduler utilities module. 
        Contains Sheduler and API functions.

    $Header: /home/CVS_DEFRAG/Defrag/Src/Defrag01/JobManager.cpp,v 1.10 2009/12/29 09:50:16 dimas Exp $
    $Log: JobManager.cpp,v $
    Revision 1.10  2009/12/29 09:50:16  dimas
    Bug with sparsed files fragmentation detection fixed

    Revision 1.9  2009/12/25 13:48:54  dimas
    Debug artefacts removed

    Revision 1.8  2009/12/23 13:01:47  dimas
    1. Full File Name processing implemented.
    2. Top 100 Fragmented functionality for all file list requests implemented.

    Revision 1.7  2009/12/21 17:00:13  dimas
    1. DEFRAG_CMD_GET_MOST_FRAGMENTED and similar requests implemented
    2. Basic Exclude files check added

    Revision 1.6  2009/12/17 10:27:20  dimas
    1. Defragmentation of FAT volumes in release build enabled
    2. Debug function GetDisplayBlockCounts() implemented

    Revision 1.5  2009/12/16 14:13:34  dimas
    DEFRAG_CMD_GET_FILE_INFO request implemented

    Revision 1.4  2009/12/15 16:06:40  dimas
    OP_REMOVE_ALL pseudo operation on settings lists added

    Revision 1.3  2009/12/03 13:01:06  dimas
    Cluster Inspector implemented

    Revision 1.2  2009/12/02 14:42:15  dimas
    App/Service interaction improved

    Revision 1.1  2009/11/24 14:52:31  dimas
    no message

*/

#include "stdafx.h"


void
InsertJob( 
    IN  PDEFRAG_JOB_CONTEXT     Job )
{

    //
    // Insert Job
    //
    EnterCriticalSection( &Global.JobListSync );

    if ( !(Global.AppStatus & APP_STAT_JOB_MANAGER_STOPPED) ) 
    {
        PDEFRAG_JOB_CONTEXT     LastJob = Global.JobList;
        if ( LastJob )
        {
            while ( LastJob->NextJob )
            {
                LastJob = LastJob->NextJob;
            }

            LastJob->NextJob = Job;
            Job->PrevJob = LastJob;
        }
        else
        {
            Global.JobList = Job;
        }

    }

    LeaveCriticalSection( &Global.JobListSync );

} // end of InsertJob


PDEFRAG_JOB_CONTEXT
FindJob( 
    IN  DWORD                   JobId )
{
    PDEFRAG_JOB_CONTEXT     Job = NULL;

    //
    // Find Job
    //
    EnterCriticalSection( &Global.JobListSync );

    Job = Global.JobList;
    while ( Job )
    {
        if (  JobId == Job->Id )
        {
            break;
        }
        Job = Job->NextJob;
    }

    LeaveCriticalSection( &Global.JobListSync );

    return Job;

} // end of FindJob


#ifdef _DEBUG
    #define _DEBUG_JOB
#else
    //#define _DEBUG
    //#define _DEBUG_JOB
    //#define _DEBUG_JOBS
#endif

//
// Delete Job control structures
//
PDEFRAG_JOB_CONTEXT
FreeJob(
    IN  PDEFRAG_JOB_CONTEXT     JobCtx )
{
    PDEFRAG_JOB_CONTEXT     CurrentJob = JobCtx, NextJob = NULL, PrevJob = NULL;



    if ( CurrentJob ) //&& !(CurrentJob->Status |= DEFRAG_JOB_FLAG_DELETED) )
    {
        EnterCriticalSection( &Global.JobListSync );

#ifdef _DEBUG_JOB
        WCHAR       Mess[MAX_PATH];
        swprintf( Mess, L"   FreeJob()  %08X. Update JobList: NextJob %08X  PrevJob %08X", 
            CurrentJob, NextJob, PrevJob );
        OutMessage( Mess );
#endif

        NextJob = CurrentJob->NextJob;
        PrevJob = CurrentJob->PrevJob;

        if ( PrevJob )
        {
            PrevJob->NextJob = NextJob;
        }
        else
        {
            Global.JobList = NextJob;
        }

        if ( NextJob )
        {
            NextJob->PrevJob = PrevJob;
        }

#ifdef _DEBUG_JOB
        swprintf( Mess, L"      LeaveCriticalSection & call DfrgFreeJobCtx()" );
        OutMessage( Mess );
#endif

        LeaveCriticalSection( &Global.JobListSync );

        DfrgFreeJobCtx( JobCtx );

#ifdef _DEBUG_JOB
        swprintf( Mess, L"      Job %08X Freed", CurrentJob );
        OutMessage( Mess );
#endif

    }


    return NextJob;

} // end of FreeJob

//
// Free Job control structures list 
//
void
FreeJobList(
    IN  PDEFRAG_JOB_CONTEXT     JobList )
{
    PDEFRAG_JOB_CONTEXT     CurrentJob = JobList;


#ifdef _DEBUG
    WCHAR       Mess[MAX_PATH];
    swprintf( Mess, L"   FreeJobList %08X", CurrentJob );
    OutMessage( Mess );
#endif

    EnterCriticalSection( &Global.JobListSync );

    while ( CurrentJob )
    {
        CurrentJob = FreeJob( CurrentJob );
    } 

    LeaveCriticalSection( &Global.JobListSync );

} // end of FreeJobList

#ifdef _DEBUG_JOBS
    #undef _DEBUG
    #undef _DEBUG_JOB
    #undef _DEBUG_JOBS
#endif


#ifndef DEFRAG_NATIVE_APP

DWORD 
WINAPI 
JobThread(
    LPVOID Parameter )
{
    DWORD                       Result = NO_ERROR;
    PDEFRAG_JOB_CONTEXT         Job  = (PDEFRAG_JOB_CONTEXT)Parameter;

#ifdef _DEBUG_JOB
    OutMessage( L"" );
    OutMessage( L"JobThread started\n" );
#endif

    __try
    {

        Result = DfrgDoJob( Job, &DfrgSettings );

    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        Result = GetExceptionCode();
        DislpayErrorMessage( L"JobThread() Exception !!!", Result, ERR_OS );
    }

#ifdef _DEBUG_JOB
    OutMessage( L"" );
    OutMessage( L"JobThread terminated\n" );
#endif

    //
    // Remove in progress mark
    //
    EnterCriticalSection( &Job->JobSync );

    if ( Job->JobThread )
    {
        CloseHandle( Job->JobThread );
        Job->JobThread = NULL;
    }

    Job->Status &= ~DEFRAG_JOB_FLAG_INPROGRESS;
    Job->Status |= DEFRAG_JOB_FLAG_PROCESSED;

    LeaveCriticalSection( &Job->JobSync );

    SetShedulerEvent();

    return Result;

} // end of JobThread


int
StartJobThread(
    PDEFRAG_JOB_CONTEXT         Job )
{
    int                         Result = NO_ERROR;
    NTSTATUS                    Status = STATUS_SUCCESS;
    BOOLEAN                     IsInCriticalSection = FALSE;


    if ( !Job || !(Job->Status & DEFRAG_JOB_FLAG_INITIALIZED) )
    {
        return ERROR_INVALID_PARAMETER;
    }

    __try
    {
        //
        // Mark Job in Progress
        //
        EnterCriticalSection( &Job->JobSync );
        IsInCriticalSection = TRUE;

        if ( (Global.AppStatus & APP_STAT_JOB_MANAGER_STOPPED) ) 
        {
            Result = DFRG_ERROR_ABORTED;
            __leave;
        }

        if ( (Job->Status & DEFRAG_JOB_FLAG_INPROGRESS) ||
            (Job->Status & DEFRAG_JOB_FLAG_ERROR) ||
            (Job->Status & DEFRAG_JOB_FLAG_STOPPED) )
        {
            Result = ERROR_INVALID_PARAMETER;
            __leave;
        }

        Job->Status |= DEFRAG_JOB_FLAG_INPROGRESS;

        LeaveCriticalSection( &Job->JobSync );
        IsInCriticalSection = FALSE;

        //
        // Start thread to performe job
        //
        HANDLE JobThreadHandle = CreateThread( 
                                NULL, 
                                0, 
                                JobThread, 
                                Job, 
                                0, 
                                NULL ); 
        if ( JobThreadHandle ) 
        {
            Job->JobThread = JobThreadHandle;
            //CloseHandle( JobThreadHandle );
        }
        else
        {
            Result = ERROR_NOT_ENOUGH_MEMORY;
        }

    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        Result = GetExceptionCode();
        DislpayErrorMessage( L"StartJobThread() Exception !!!", Result, ERR_OS );
    }


    if ( IsInCriticalSection )
    {
        LeaveCriticalSection( &Job->JobSync );
    }

    return Result;

} // end of StartJobThread



DWORD 
WINAPI 
DfrgJobManager(
    LPVOID Parameter )
{
    DWORD                       Result = NO_ERROR;
    NTSTATUS                    Status = STATUS_SUCCESS;

    UNICODE_STRING              PortName;
    OBJECT_ATTRIBUTES           ObjectAttributes = { sizeof(OBJECT_ATTRIBUTES), NULL, &PortName, 0, NULL, NULL }; //, OBJ_KERNEL_HANDLE, NULL, NULL };

    //PPORT_CONNECTION_INFO       ConnectionInfo;
    PORT_MESSAGEX2              ClientMessage;

    PORT_SECTION_WRITE          ServerSection = { sizeof(ServerSection), 0, 0, 0, 0, 0 };
    PORT_SECTION_READ           ClientSection = { sizeof(ClientSection), 0, 0 };

    HANDLE                      TerminateThreadEvent = (HANDLE)Parameter;
    HANDLE                      Events[] = { TerminateThreadEvent }; 
    DWORD                       WaitResult;
    DWORD                       RequestCnt = 0;

    DWORD                       Command = 0;
    PDEFRAG_JOB_CONTEXT         Job = NULL;

    PSETTINGS_INFO              Setting;
    DWORD                       DataLength, OutputDataLength = 0;

    BOOLEAN                     IsInCriticalSection = FALSE;

//#ifdef _DEBUG
    WCHAR                       Mess[MAX_PATH];
//#endif


    __try
    {
#ifdef _DEBUG_JOB
        swprintf( Mess, L"Try CreatePort '%ls'", DFRG_PORT_NAME );
        OutMessage( Mess );
#endif

        RtlInitUnicodeString( &PortName, DFRG_PORT_NAME );
        Status = ZwCreatePort( 
                        &Global.ServerPort, 
                        &ObjectAttributes, 
                        PORT_DATA_LENGTH, 
                        sizeof(PORT_MESSAGEX2), 
                        0 );

        if ( Status != STATUS_SUCCESS || !Global.ServerPort ) 
        {
            swprintf( Mess, L"ZwCreatePort '%ls' %08lX", PortName.Buffer, Global.ServerPort );
            Result = DislpayErrorMessage( Mess, Status, ERR_NT_STATUS );
            __leave;
        }
#ifdef _DEBUG_JOB
        else 
        {
            swprintf( Mess, L"ZwCreatePort '%ls' handle = %08lX", PortName.Buffer, Global.ServerPort );
            OutMessage( Mess );
        }
#endif

        while ( TRUE )
        {
            //
            // Wait for connection
            //
            Status = ZwListenPort( Global.ServerPort, (PLPC_MESSAGE)&ClientMessage );

            if ( Status != STATUS_SUCCESS ) 
            {
                swprintf( Mess, L"ZwListenPort '%ls' %08lX", PortName.Buffer, Global.ServerPort );
                Result = DislpayErrorMessage( Mess, Status, ERR_NT_STATUS );
                __leave;
            }
#ifdef _DEBUG_JOB
            else 
            {
                //ConnectionInfo = (PPORT_CONNECTION_INFO)ClientMessage.RawData;
                //*(ConnectionInfo->TextMessage + sizeof(ConnectionInfo->TextMessage)/sizeof(WCHAR) - 1) = L'\0';
                ClientMessage.ConnectionInfo.TextMessage[ARRAYSIZE(ClientMessage.ConnectionInfo.TextMessage)-1] = L'\0';
                FormatLpcMsg( Mess, TRUE, L"ZwListenPort", (PORT_MESSAGE*)&ClientMessage, 0, 0, ClientMessage.ConnectionInfo.TextMessage );
                OutMessage( Mess );
            }
#endif

            if ( DEFRAG_EXTERNAL_PROTOCOL_VERSION != ClientMessage.ConnectionInfo.Version )
            {
                //
                // Refuse connection
                //
                //wcscpy( ClientMessage.WString, L"Connection Refused" );
                //ClientMessage.DataSize = (USHORT)(wcslen(ClientMessage.WString) + 1) * sizeof(WCHAR);
#ifdef _DEBUG_JOB
                swprintf( Mess, L"Wrong IF versions: %08X != %08X. Connection Refused", 
                    DEFRAG_EXTERNAL_PROTOCOL_VERSION, ClientMessage.ConnectionInfo.Version );
                OutMessage( Mess );
#endif
                wcscpy( ClientMessage.ConnectionInfo.TextMessage, L"Connection Refused" );
                ClientMessage.ConnectionInfo.Version = DEFRAG_EXTERNAL_PROTOCOL_VERSION;
                ClientMessage.DataSize = sizeof(PORT_CONNECTION_INFO);

                Status = ZwAcceptConnectPort( 
                                &Global.ClientPort, 
                                (HANDLE)0xababaeba, // sign / sec num / ClientCnt
                                (PLPC_MESSAGE)&ClientMessage, 
                                FALSE, 
                                NULL, 
                                NULL );

                Result = ERROR_NOT_SUPPORTED;
                continue;
            }

            //
            // Prepare buffers.
            // Create Server memory section.
            //
            Global.ServerSectionHandle = CreateFileMapping( 
                                    INVALID_HANDLE_VALUE, 
                                    NULL,
                                    PAGE_READWRITE, 
                                    0, 
                                    SERVER_SECTION_SIZE, 
                                    0 );
            if ( !Global.ServerSectionHandle )
            {
                swprintf( Mess, L"Server could not create FileMapping" );
                DislpayErrorMessage( Mess, Result, ERR_OS );
                Result = ERROR_NOT_ENOUGH_MEMORY;
                __leave;
            }

            Global.ServerBuffer = (PBYTE)MapViewOfFile( 
                                    Global.ServerSectionHandle,
                                    FILE_MAP_WRITE,
                                    0, 0, SERVER_SECTION_SIZE );
            if ( !Global.ServerBuffer )
            {
                OutMessage( L"MapViewOfFile error" );
                Result = ERROR_NOT_ENOUGH_MEMORY;
                __leave;
            }

            Global.BufferLength = SERVER_SECTION_SIZE;

            ServerSection.SectionHandle = Global.ServerSectionHandle;
            ServerSection.SectionOffset = 0;
            ServerSection.ViewSize      = SERVER_SECTION_SIZE;

            //
            // Reply to connection
            //
            //wcscpy( ClientMessage.WString, L"Connection Accepted" );
            //ClientMessage.DataSize = (USHORT)(wcslen(ClientMessage.WString) + 1) * sizeof(WCHAR);
            wcscpy( ClientMessage.ConnectionInfo.TextMessage, L"Connection Accepted" );
            ClientMessage.ConnectionInfo.Version = DEFRAG_EXTERNAL_PROTOCOL_VERSION;
            ClientMessage.DataSize = sizeof(PORT_CONNECTION_INFO);

            Status = ZwAcceptConnectPort( 
                            &Global.ClientPort, 
                            (HANDLE)0xababaeba, // sign / sec num / ClientCnt
                            (PLPC_MESSAGE)&ClientMessage, 
                            TRUE, 
                            &ServerSection, 
                            &ClientSection );

            if ( Status != STATUS_SUCCESS ) 
            {
                swprintf( Mess, L"ZwAcceptConnectPort '%ls' %08lX", PortName.Buffer, Global.ServerPort );
                Result = DislpayErrorMessage( Mess, Status, ERR_NT_STATUS );
                __leave;
            }
#ifdef _DEBUG_JOB
            else 
            {
                swprintf( Mess, _T("ZwAcceptConnectPort. ClientPort %08lX  ClientBuf %p"), 
                    Global.ClientPort, ClientSection.ViewBase );
                OutMessage( Mess );
            }
#endif

            //
            // Set Clien (Input) Buffer addr & length
            //
            Global.ClientBuffer = (PBYTE)ClientSection.ViewBase;


            //
            // Finalyze connection
            //
            Status = ZwCompleteConnectPort( Global.ClientPort );

            if ( Status != STATUS_SUCCESS ) 
            {
                swprintf( Mess, L"ZwAcceptConnectPort '%ls' %08lX", PortName.Buffer, Global.ServerPort );
                Result = DislpayErrorMessage( Mess, Status, ERR_NT_STATUS );
                __leave;
            }


            while ( TRUE ) 
            {
                HANDLE  PortId;

                //
                // Wait for request
                //
                Status = ZwReplyWaitReceivePort( Global.ClientPort, &PortId, 0, (PLPC_MESSAGE)&ClientMessage );

                if ( Status != STATUS_SUCCESS )
                {
                    swprintf( Mess, L"ZwReplyWaitReceivePort '%ls' %08lX", PortName.Buffer, Global.ServerPort );
                    Result = DislpayErrorMessage( Mess, Status, ERR_NT_STATUS );
                    break;
                    //__leave;
                }

#ifdef _DEBUGC
                //ClientMessage.WString[min(ClientMessage.DataSize,PORT_DATA_LENGTH)/sizeof(WCHAR)-1] = 0;
                FormatLpcMsg( Mess, TRUE, L"ZwReplyWaitReceivePort", (PORT_MESSAGE*)&ClientMessage, ClientMessage.Command, ClientMessage.JobId, NULL ); // ClientMessage.WString );
                OutMessage( Mess );
#endif

                if ( (Global.AppStatus & APP_STAT_JOB_MANAGER_STOPPED) ) 
                {
                    Result = DFRG_ERROR_ABORTED;
                    __leave;
                }

                if ( ClientMessage.MessageType == LPC_PORT_CLOSED || 
                     ClientMessage.MessageType == LPC_CLIENT_DIED ) 
                {
#ifdef _DEBUG_JOB
                    OutMessage ( L"\r\nClient port closed or died - break" );
#endif
                    break;
                }

                if ( ClientMessage.MessageType != LPC_REQUEST ) 
                {
                    RequestCnt++;
                    continue;
                }

                //
                // Reply to request
                //
                Result = NO_ERROR;
                Command = ClientMessage.Command;
                Job = NULL;
                OutputDataLength = 0;

                IsInCriticalSection = FALSE;
                EnterCriticalSection( &Global.JobListSync );
                IsInCriticalSection = TRUE;

                if ( DEFRAG_CMD_INIT_JOB != Command &&
                    DEFRAG_CMD_GET_JOB_LIST != Command &&
                    DEFRAG_CMD_GET_SETTING != Command && 
                    DEFRAG_CMD_SET_SETTING != Command )
                {
                    Job = FindJob( ClientMessage.JobId );
                    if ( !Job || !(Job->Status & DEFRAG_JOB_FLAG_INITIALIZED) )
                    {
#ifdef _DEBUG
                        swprintf( Mess, L"   ### Invalid Job ID = %08X ", 
                            ClientMessage.JobId );
                        OutMessage( Mess );
#endif
                        Result = ERROR_INVALID_PARAMETER;
                        ClientMessage.Result = Result;
                    }
                }

                if ( NO_ERROR == Result )
                {
                    switch ( Command )
                    {

                    //
                    // Get current Jobs list
                    //
                    case DEFRAG_CMD_GET_JOB_LIST:

                        {
                        PDEFRAG_JOB_OPTIONS     JobOptions = (PDEFRAG_JOB_OPTIONS)Global.ServerBuffer;
                        DWORD                   JobCnt = 0;

                        EnterCriticalSection( &Global.JobListSync );

                        Job = Global.JobList;

                        while ( Job )
                        {
                            *((PDWORD)Global.ServerBuffer + JobCnt) = Job->Id;
                            JobCnt++;
                            Job = Job->NextJob;
                        }

                        LeaveCriticalSection( &Global.JobListSync );

                        *((PDWORD)Global.ServerBuffer + JobCnt) = INVALID_JOB_ID;

                        OutputDataLength = (JobCnt+1) * sizeof(DWORD);
                        }

                        break;


                    case DEFRAG_CMD_GET_JOB_OPTIONS:

                        {
                        PDEFRAG_JOB_OPTIONS     JobOptions = (PDEFRAG_JOB_OPTIONS)Global.ServerBuffer;

                        EnterCriticalSection( &Global.JobListSync );

                        JobOptions->Id = Job->Id;
                        wcscpy( JobOptions->VolumeName, Job->VolumeCtx.VName );
                        JobOptions->CurrentJobType = !(Job->Status & DEFRAG_JOB_FLAG_ANALIZED) ? DEFRAG_JOB_ANALIZE_ONLY : Job->Type;

                        LeaveCriticalSection( &Global.JobListSync );

                        OutputDataLength = sizeof(DEFRAG_JOB_OPTIONS);
                        }

                        break;

                    //
                    // Settings & Sheduled Jobs
                    //
                    case DEFRAG_CMD_GET_SETTING:

                        Setting = (PSETTINGS_INFO)(&ClientMessage.Setting);

#ifdef _DEBUGW
                        swprintf( Mess, L"   ### GetSetting == %ls (%d), Id %d", 
                            GetOpString(Setting->SettingOp), Setting->SettingOp, Setting->SettingId );
                        OutMessage( Mess );
#endif
                        if ( OP_GET != Setting->SettingOp )
                        {
                            Result = ERROR_INVALID_PARAMETER;
                        }
                        else
                        {
                            DataLength = ClientMessage.OutputDataLength; // Setting->DataLength;

                            Result = DfrgApiSettingOp(
                                            Setting->SettingId,
                                            Setting->SettingOp,
                                            Global.ServerBuffer,
                                            &DataLength );

                            OutputDataLength = DataLength;
                        }

                        break;

                    case DEFRAG_CMD_SET_SETTING:

                        Setting = (PSETTINGS_INFO)(&ClientMessage.Setting);
                        DataLength = ClientMessage.InputDataLength; // Setting->DataLength;
#ifdef _DEBUG
                        swprintf( Mess, L"   ### SetSetting == %ls (%d), Id %d", 
                            GetOpString(Setting->SettingOp), Setting->SettingOp, Setting->SettingId );
                        OutMessage( Mess );
                        if ( OP_ADD == Setting->SettingOp && 1 == Setting->SettingId )
                        {
                            swprintf( Mess, L"      exclude: '%ls'", 
                                Global.ClientBuffer );
                            OutMessage( Mess );
                        }
#endif
                        if ( !Global.ClientBuffer ||
                                OP_SET != Setting->SettingOp && 
                                OP_REMOVE != Setting->SettingOp &&
                                OP_REMOVE_ALL != Setting->SettingOp )
                        {
                            Result = ERROR_INVALID_PARAMETER;
                        }
                        else
                        {
                            if ( OP_REMOVE_ALL == Setting->SettingOp )
                            {
                                Result = DfrgApiSettingOp(
                                                Setting->SettingId,
                                                OP_REMOVE,
                                                NULL,
                                                &DataLength );
                            }
                            else
                            {
                                Result = DfrgApiSettingOp(
                                                Setting->SettingId,
                                                Setting->SettingOp,
                                                Global.ClientBuffer,
                                                &DataLength );
                            }
                        }

                        break;

                    //
                    // General Interactive Job Control
                    //
                    case DEFRAG_CMD_GET_JOB_STATUS:

                        break;

                    case DEFRAG_CMD_GET_MAP_AND_STATUS:
                        {
                        DataLength = min( Global.BufferLength, Job->DisplayMapExSize*sizeof(DFRG_MAP_BLOCK_INFO) );

                        memcpy( Global.ServerBuffer, Job->DisplayMapEx, DataLength );

                        OutputDataLength = DataLength;
                        }

                        break;

                    case DEFRAG_CMD_INIT_JOB:

#ifdef _DEBUG_JOB
                        OutMessage ( L"### Init Job" );
#endif
                        Result = DfrgInitJobCtx( &Job, ClientMessage.DriveName, DEFRAG_JOB_ANALIZE_ONLY );
#ifdef _DEBUG_JOB
                        swprintf( Mess, L"--- DfrgInitJobCtx Result %d ", Result );
                        OutMessage ( Mess );
#endif
                        if ( NO_ERROR == Result )
                        {
                            //
                            // Insert Job
                            //
                            InsertJob( Job );
                        }

                        break;

                    case DEFRAG_CMD_STOP_JOB:


#ifdef _DEBUG_JOB
                        OutMessage ( L"### Stop Job" );
#endif

                        Job->Status |= DEFRAG_JOB_FLAG_STOPPED;

                        SetEvent( Job->ResumeEvent );

                        break;

                    case DEFRAG_CMD_DELETE_JOB:

#ifdef _DEBUG_JOB
                        OutMessage ( L"### Delete Job" );
#endif

                        if ( !(Job->Status & DEFRAG_JOB_FLAG_STOPPED) )
                        {
                            Result = ERROR_INVALID_PARAMETER;
                        }
                        else
                        {
                            FreeJob( Job );
                            Job = NULL;
                        }

                        break;

                    case DEFRAG_CMD_PAUSE_JOB:

#ifdef _DEBUG_JOB
                        OutMessage ( L"### Pause Job" );
#endif

                        Job->Status |= DEFRAG_JOB_FLAG_PAUSED;

                        ResetEvent( Job->ResumeEvent );

                        break;

                    case DEFRAG_CMD_RESUME_JOB:

#ifdef _DEBUG_JOB
                        OutMessage ( L"### Resume Job" );
#endif

                        Job->Status &= ~DEFRAG_JOB_FLAG_PAUSED;

                        SetEvent( Job->ResumeEvent );

                        ClientMessage.Result = Result;

                        break;

                    case DEFRAG_CMD_SET_JOB_OPTIONS:

                        ClientMessage.Result = ERROR_NOT_SUPPORTED;

                        break;

                    //
                    // Analisys
                    //
                    case DEFRAG_CMD_ANALIZE_VOLUME:

#ifdef _DEBUG_JOB
                        OutMessage ( L"### Analize " );
#endif
                        //
                        // Analyze volume
                        //
                        Job->Type = DEFRAG_JOB_ANALIZE_ONLY;
                        Result = StartJobThread( Job );

                        ClientMessage.Result = Result;

                        break;

                    case DEFRAG_CMD_GET_STATISTIC:

                        //if ( !(Job->Status & DEFRAG_JOB_FLAG_ANALIZED) )
                        //{
                        //    Result = ERROR_INVALID_PARAMETER;
                        //}
                        //else
                        {
                            PDFRG_VOLUME_INFO   DfrgVolumeInfo = (PDFRG_VOLUME_INFO)Global.ServerBuffer;

                            memcpy( DfrgVolumeInfo, &(Job->VolumeCtx.NtfsData), sizeof(NTFS_VOLUME_DATA_BUFFER) );

                            memcpy( &(DfrgVolumeInfo->Statistic), &(Job->VolumeCtx.Statistic), sizeof(DEFRAG_VOLUME_STATISTIC) );

                            OutputDataLength = sizeof(DFRG_VOLUME_INFO);
#ifdef _DEBUG
                            static int  Cnt = 0;
                            if ( !(Cnt % 100) ) // || NtfsDisk != Job->VolumeCtx.DiskType 
                            {
                                swprintf( Mess, L"### Get Statistic. (req %d) CurrentFileName '%ls'",
                                    Cnt, DfrgVolumeInfo->Statistic.CurrentFileName );
                                OutMessage( Mess );
                            }
                            Cnt++;
#endif
                        }

                        break;

                    case DEFRAG_CMD_GET_CLUSTER_FILES:

                        if ( !( (Job->Status & DEFRAG_JOB_FLAG_ANALIZED) &&
                                (Job->Status & DEFRAG_JOB_FLAG_STOPPED) ) )
                        {
                            //
                            // Volume should be analized & Job should be stopped/finished
                            //
                            Result = ERROR_INVALID_PARAMETER;
                        }
                        else
                        {
                            PDEFRAG_CINSPECTOR_INFO ClusterInfo = (PDEFRAG_CINSPECTOR_INFO)Global.ServerBuffer;

#ifdef _DEBUG
                            swprintf( Mess, L"### Get ClusterFiles from %I64d [%I64d], count %d ",
                                ClientMessage.ClustersBlock.StartCluster, 
                                ClientMessage.ClustersBlock.ClustersNumber,
                                ClientMessage.OutputDataLength / sizeof( DEFRAG_CINSPECTOR_INFO ) );
                            OutMessage( Mess );
#endif
                            //DataLength = Global.BufferLength;
                            DataLength = min( Global.BufferLength, ClientMessage.OutputDataLength );

                            Result = DfrgClusterInspector( 
                                            Job,
                                            &ClientMessage.ClustersBlock, 
                                            ClusterInfo, 
                                            &DataLength );
#ifdef _DEBUG
                            if ( NO_ERROR == Result )
                            {
                                DFRG_MAP_BLOCK_INFO         DisplayMapBlock;
                                DWORD                       BlockNum;

                                GetDisplayBlockCounts( 
                                            Job, 
                                            ClientMessage.ClustersBlock.StartCluster, 
                                            &DisplayMapBlock, 
                                            &BlockNum );
                                swprintf( Mess, L"   Block[%d] stat: alloc %u, free %u, frag %u, meta %u",
                                    BlockNum, 
                                    (ULONG)DisplayMapBlock.Allocated, (ULONG)DisplayMapBlock.FreeSpace, 
                                    (ULONG)DisplayMapBlock.FragmentedFiles, (ULONG)DisplayMapBlock.Metadata );
                                OutMessage( Mess );

                                for( DWORD k=0; k<DataLength / sizeof( DEFRAG_CINSPECTOR_INFO ); k++ )
                                {
                                    swprintf( Mess, L"   [%d] FileName: '%ls' Vcn %I64d  Lcn %I64d [%I64d]",
                                        k, (ClusterInfo+k)->FileName, 
                                        (ClusterInfo+k)->VCN, (ClusterInfo+k)->StartCluster, (ClusterInfo+k)->NumberOfClusters );
                                    OutMessage( Mess );
                                }
                            }
#endif
                            //OutputDataLength = DataLength;
                            OutputDataLength = min( DataLength, ClientMessage.OutputDataLength );
                        }

                        break;

                    case DEFRAG_CMD_GET_FILE_INFO:

                        if ( !( (Job->Status & DEFRAG_JOB_FLAG_ANALIZED) &&
                                (Job->Status & DEFRAG_JOB_FLAG_STOPPED) ) )
                        {
                            //
                            // Volume should be analized & Job should be stopped/finished
                            //
                            Result = ERROR_INVALID_PARAMETER;
                        }
                        else
                        {
                            PDEFRAG_FILE_INFO   FileInfo = (PDEFRAG_FILE_INFO)Global.ServerBuffer;
                            PWCHAR              FileName = (PWCHAR)Global.ClientBuffer;

                            FileName = wcschr( FileName, L':' );
                            if ( FileName )
                            {
                                FileName++;
                            }
                            else
                            {
                                FileName = (PWCHAR)Global.ClientBuffer;
                            }

                            DataLength = min( Global.BufferLength, ClientMessage.OutputDataLength );

                            Result = DfrgGetFileInfo( 
                                            Job,
                                            FileName, 
                                            FileInfo, 
                                            &DataLength );

                            OutputDataLength = min( DataLength, ClientMessage.OutputDataLength );
                        }

                        break;

                    //
                    // Get file lists
                    //
                    case DEFRAG_CMD_GET_UNMOV_FILES:
                    case DEFRAG_CMD_GET_SYSTEM_FILES:
                    case DEFRAG_CMD_GET_DIR_FILES:

                    case DEFRAG_CMD_GET_MOST_FRAGMENTED:
                    case DEFRAG_CMD_GET_SKIPPED: // Excluded files

                        if ( !( (Job->Status & DEFRAG_JOB_FLAG_ANALIZED) &&
                                (Job->Status & DEFRAG_JOB_FLAG_STOPPED) ) )
                        {
                            //
                            // Volume should be analized & Job should be stopped/finished
                            //
                            Result = ERROR_INVALID_PARAMETER;
                        }
                        else
                        {
                            PDEFRAG_FILE_LIST   FileList = (PDEFRAG_FILE_LIST)Global.ServerBuffer;

                            DataLength = min( Global.BufferLength, ClientMessage.OutputDataLength );

                            Result = DfrgGetMarkedFiles( 
                                            Job,
                                            Command,
                                            FileList, 
                                            &DataLength );

                            OutputDataLength = min( DataLength, ClientMessage.OutputDataLength );
                        }

                        break;

                    //
                    // Evaluate speed
                    //
                    case DEFRAG_CMD_GET_SPEED:

                        Result = ERROR_NOT_SUPPORTED;

                        break;

                    //
                    // Interactive Job Defragmentation
                    //
                    case DEFRAG_CMD_DEBUG1:
                    case DEFRAG_CMD_FREE_SPACE:
                    case DEFRAG_CMD_SIMPLE_DEFRAG:

                    case DEFRAG_CMD_SMART_BY_NAME:
                    case DEFRAG_CMD_SMART_BY_ACCESS:
                    case DEFRAG_CMD_SMART_BY_MODIFY:
                    case DEFRAG_CMD_SMART_BY_SIZE:
                    case DEFRAG_CMD_SMART_BY_CREATE:
                    case DEFRAG_CMD_FORCE_TOGETHER:

                        if ( !(Job->Status & DEFRAG_JOB_FLAG_ANALIZED) )
                        {
                            Result = ERROR_INVALID_PARAMETER;
                        }
                        else if ( NO_ERROR != Job->Result ) 
                        {
                            Result = Job->Result;
                        }
                        else
                        {
                            switch ( Command )
                            {
                            case DEFRAG_CMD_DEBUG1:

                                wcscpy( Mess, L"### DEBUG1 " );

                            default:

                                wcscpy( Mess, L"### UNKNOWN == DEBUG1 " );
                                Job->Type = DEFRAG_JOB_DEBUG1;

                                break;

                            case DEFRAG_CMD_FREE_SPACE:

                                wcscpy( Mess, L"### FREE_SPACE" );
                                Job->Type = DEFRAG_JOB_FREE_SPACE;
                               
                                break;

                            case DEFRAG_CMD_SIMPLE_DEFRAG:

                                wcscpy( Mess, L"### SIMPLE_DEFRAG" );
                                Job->Type = DEFRAG_JOB_SIMPLE_DEFRAG;

                                break;

                            case DEFRAG_CMD_SMART_BY_NAME:

                                wcscpy( Mess, L"### SMART_BY_NAME" );
                                Job->Type = DEFRAG_JOB_SMART_BY_NAME;

                                break;

                            case DEFRAG_CMD_SMART_BY_ACCESS:

                                wcscpy( Mess, L"### SMART_BY_ACCESS" );
                                Job->Type = DEFRAG_JOB_SMART_BY_ACCESS;

                                break;

                            case DEFRAG_CMD_SMART_BY_MODIFY:

                                wcscpy( Mess, L"### SMART_BY_MODIFY" );
                                Job->Type = DEFRAG_JOB_SMART_BY_MODIFY;

                                break;

                            case DEFRAG_CMD_SMART_BY_SIZE:

                                wcscpy( Mess, L"### SMART_BY_SIZE" );
                                Job->Type = DEFRAG_JOB_SMART_BY_SIZE;

                                break;

                            case DEFRAG_CMD_SMART_BY_CREATE:

                                wcscpy( Mess, L"### SMART_BY_CREATE" );
                                Job->Type = DEFRAG_JOB_SMART_BY_CREATE;

                                break;

                            case DEFRAG_CMD_FORCE_TOGETHER:

                                wcscpy( Mess, L"### FORCE_TOGETHER " );
                                Job->Type = DEFRAG_JOB_FORCE_TOGETHER;

                                break;

                            }

#ifdef _DEBUG_JOB
                            OutMessage ( Mess );
#endif
                            Result = StartJobThread( Job );
                        }

                        break;

                    default:
                        //
                        // Unknown command 
                        //
                        Result = ERROR_NOT_SUPPORTED; // ERROR_INVALID_PARAMETER;

                        swprintf( ClientMessage.WString, L"reply #%d", RequestCnt );
                        //ClientMessage.DataSize = (wcslen(ClientMessage.WString) + 1) * sizeof(WCHAR);
                        memcpy( Global.ServerBuffer, ClientMessage.RawData, ClientMessage.DataSize );

                        break;

                    } // switch

                } // if FindJob

                RequestCnt++;

                ClientMessage.Result = Result;
                ClientMessage.OutputDataLength = OutputDataLength;

                if ( Job ) 
                {
                    ClientMessage.JobId     = Job->Id;
                    ClientMessage.JobStatus = Job->Status;

                    //
                    // Mark progress
                    //
                    ClientMessage.JobProgress = 0;
                    if ( Job->ObjectsToProcessNum.QuadPart )
                    {
                        ClientMessage.JobProgress = 
                            (ULONG)((Job->ProcessedObjectsNum.QuadPart * 100) / Job->ObjectsToProcessNum.QuadPart);
                    }
                } // if ( Job


                if ( IsInCriticalSection )
                {
                    LeaveCriticalSection( &Global.JobListSync );
                    IsInCriticalSection = FALSE;
                }

                ClientMessage.ServiceStatus = Global.AppStatus;

                ClientMessage.DataSize = sizeof( ClientMessage.RawData );
#ifdef _DEBUGC
                swprintf( Mess, L"--- Call ZwReplyPort: JobId %08X  Status %d ", ClientMessage.JobId, ClientMessage.Result );
                OutMessage ( Mess );
#endif
                Status = ZwReplyPort( Global.ClientPort, (PLPC_MESSAGE)&ClientMessage );

                if ( Status != STATUS_SUCCESS ) 
                {
                    swprintf( Mess, L"ZwReplyPort '%ls' %08lX", PortName.Buffer, Global.ServerPort );
                    Result = DislpayErrorMessage( Mess, Status, ERR_NT_STATUS );
                    break;
                    //__leave;
                }

                if ( (Global.AppStatus & APP_STAT_JOB_MANAGER_STOPPED) ) 
                {
                    Result = DFRG_ERROR_ABORTED;
                    __leave;
                }

            } // while - wait for request

/*
            DfrgStopAllJobs();

            FreeJobList( Global.JobList );

            Global.JobList = NULL;
*/

            if ( Global.ServerBuffer )
            {
                UnmapViewOfFile( Global.ServerBuffer );
                Global.ServerBuffer = NULL;
            }

            if ( Global.ServerSectionHandle ) 
            {
                CloseHandle( Global.ServerSectionHandle );
                Global.ServerSectionHandle = NULL;
            }

            if ( Global.ClientPort ) 
            { 
                NtClose( Global.ClientPort );
                Global.ClientPort = NULL;
            }

        } // while - wait for connection

    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        Result = GetExceptionCode();
        DislpayErrorMessage( L"DfrgJobManager() Exception !!!", Result, ERR_OS );
    }

#ifdef _DEBUG
    OutMessage( L"DfrgJobManager  Exit" );
#endif


    if ( IsInCriticalSection )
    {
        LeaveCriticalSection( &Global.JobListSync );
        IsInCriticalSection = FALSE;
    }

    Global.AppStatus |= APP_STAT_JOB_MANAGER_STOPPED;

    if ( Global.ServerBuffer )
    {
        UnmapViewOfFile( Global.ServerBuffer );
        Global.ServerBuffer = NULL;
    }

    if ( Global.ServerSectionHandle ) 
    {
        CloseHandle( Global.ServerSectionHandle );
        Global.ServerSectionHandle = NULL;
    }

    if ( Global.ClientPort ) 
    { 
        NtClose( Global.ClientPort );
        Global.ClientPort = NULL;
    }

    if ( Global.ServerPort) 
    { 
        NtClose( Global.ServerPort );
        Global.ServerPort = NULL;
    }

    //
    // Wait for service termination
    //
    while ( TRUE )
    {
#ifdef _DEBUG
        OutMessage( L"DfrgJobManager  Wait" );
#endif
        WaitResult = WaitForMultipleObjects( 
                                    ARRAYSIZE(Events), 
                                    Events, 
                                    FALSE, 
                                    INFINITE );
#ifdef _DEBUG
        swprintf( Mess, L"Got event #%d", WaitResult-WAIT_OBJECT_0 );
        OutMessage( Mess );
#endif
        if ( WaitResult-WAIT_OBJECT_0 == 0 )
        {
            // 
            // Terminate Thread 
            // 
            break;
        }
    }
/*
    if ( Global.ServerBuffer )
    {
        UnmapViewOfFile( Global.ServerBuffer );
        Global.ServerBuffer = NULL;
    }

    if ( Global.ServerSectionHandle ) 
    {
        CloseHandle( Global.ServerSectionHandle );
        Global.ServerSectionHandle = NULL;
    }

    if ( Global.ClientPort ) 
    { 
        NtClose( Global.ClientPort );
        Global.ClientPort = NULL;
    }

    if ( Global.ServerPort) 
    { 
        NtClose( Global.ServerPort );
        Global.ServerPort = NULL;
    }
*/

    return Result;

} // end of DfrgJobManager



int 
DfrgStartJobManager( 
    HANDLE *ObjectToWait )
{
    int                         Result = NO_ERROR;


#ifdef _DEBUG
    OutMessage( L"DfrgStartJobManager" );
#endif

    if ( ObjectToWait ) 
    {
        *ObjectToWait = CreateEvent( NULL, FALSE, FALSE, NULL );
        if ( !(*ObjectToWait) )
        {
            return ERROR_NOT_ENOUGH_MEMORY;
        }
    }

    Global.TerminateSheduller = CreateEvent( NULL, FALSE, FALSE, NULL );
    if ( !Global.TerminateSheduller )
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }


    HANDLE SheduleThread = CreateThread( 
                        NULL, 
                        0, 
                        DfrgJobManager, 
                        Global.TerminateSheduller, 
                        0, 
                        NULL ); 
    if ( SheduleThread ) 
    {
        Global.SheduleThread = SheduleThread;
        Global.AppStatus |= APP_STAT_JOB_MANAGER_STARTED;
    }
    else
    {
        Result = ERROR_NOT_ENOUGH_MEMORY;
    }


    return Result;

} // end of DfrgStartJobManager


void
DfrgStopAllJobs( void )
{
    PDEFRAG_JOB_CONTEXT     Job;
    DWORD                   ActiveJobCnt = 0;
    HANDLE                  *Events = NULL;

    OutMessage( L"DfrgStopAllJobs" );

__try
{

    if ( (Global.AppStatus & APP_STAT_JOB_MANAGER_STARTED) )
    {
        //
        // Enumerate threads & set status STOPPED
        //
        EnterCriticalSection( &Global.JobListSync );

        ActiveJobCnt = 0;
        Job = Global.JobList;
        while ( Job )
        {
            Job->Status |= DEFRAG_JOB_FLAG_STOPPED;
            if ( Job->JobThread )
            {
                ActiveJobCnt++;
            }
            Job = Job->NextJob;
        }

        LeaveCriticalSection( &Global.JobListSync );


        //
        // Build active thread array
        //
        Events = (HANDLE*)malloc(ActiveJobCnt*sizeof(HANDLE));
        if ( !Events )
        {
            return;
        }

        if ( ActiveJobCnt )
        {
            EnterCriticalSection( &Global.JobListSync );

            ActiveJobCnt = 0;
            Job = Global.JobList;
            while ( Job )
            {
                Job->Status |= DEFRAG_JOB_FLAG_STOPPED;
                if ( Job->JobThread )
                {
                    Events[ActiveJobCnt] = Job->JobThread;
                    ActiveJobCnt++;
                }
                Job = Job->NextJob;
            }

            LeaveCriticalSection( &Global.JobListSync );

            WaitForMultipleObjects( 
                        ActiveJobCnt, 
                        Events, 
                        TRUE, 
                        INFINITE );

            for( DWORD i=0; i<ActiveJobCnt; i++ )
            {
                CloseHandle( Events[i] );
            }

            free( Events );
        }

        OutMessage( L"   DfrgStopAllJobs. All Job Threads terminated" );

    } // if started
}
__except(EXCEPTION_EXECUTE_HANDLER)
{
    int Result = GetExceptionCode();
    DislpayErrorMessage( L"DfrgStopAllJobs() Exception !!!", Result, ERR_OS );
}

} // end of DfrgStopAllJobs


void         
DfrgStopJobManager( 
    HANDLE ObjectToWait )
{

    OutMessage( L"DfrgStopJobManager" );

    Global.AppStatus |= APP_STAT_JOB_MANAGER_STOPPED;

    //
    // Stop all jobs and free resources
    //
    DfrgStopAllJobs();


    //
    // Terminate all (communication) threads
    //
    if ( Global.TerminateSheduller )
    {
        SetEvent( Global.TerminateSheduller );

        //
        // Free communication resources
        //
        if ( Global.ServerPort )
        {
            NtClose( Global.ServerPort );
            Global.ServerPort = NULL;
        }

        if ( Global.SheduleThread )
        {
            TerminateThread( Global.SheduleThread, 0 );

            WaitForSingleObject( Global.SheduleThread, INFINITE );
            CloseHandle( Global.SheduleThread );
            Global.SheduleThread = NULL;

            OutMessage( L"   DfrgStopJobManager. SheduleThread terminated" );
        }

        CloseHandle( Global.TerminateSheduller );
        Global.TerminateSheduller = NULL;
    }

    //
    // Signal event to terminate main thread
    //
    if ( ObjectToWait )
    {
       SetEvent( ObjectToWait );
    }

} // end of DfrgStopJobManager


#endif // DEFRAG_NATIVE_APP
