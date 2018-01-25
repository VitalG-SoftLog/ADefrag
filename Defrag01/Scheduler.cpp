/*
    Defrag Server Scheduler

    Module name:

        Scheduler.cpp

    Abstract:

        Defrag Server Scheduler module.
        Defines scheduler

    $Header: /home/CVS_DEFRAG/Defrag/Src/Defrag01/Scheduler.cpp,v 1.4 2009/12/25 15:58:14 dimas Exp $
    $Log: Scheduler.cpp,v $
    Revision 1.4  2009/12/25 15:58:14  dimas
    Priority control implemented

    Revision 1.3  2009/12/25 13:48:54  dimas
    Debug artefacts removed

    Revision 1.2  2009/12/16 14:13:34  dimas
    DEFRAG_CMD_GET_FILE_INFO request implemented

    Revision 1.1  2009/11/24 14:52:31  dimas
    no message

*/
    
#include "stdafx.h"
#include "time.h"
#include <map>

#ifdef _DEBUG
    #define _DEBUG_JOB
#else
    //#define _DEBUG_JOB
#endif

HANDLE ShedulerTaskManagerRunTimer = NULL;
#define SCHEDULER_TIME_SPAN         60      // 60 sec
#define SCHEDULER_DAY_SPAN          (3600*24)      // 24 hours

class CContextHolder
{
public:
    CContextHolder(){}
    virtual ~CContextHolder() { FreeContexts(); }

    void FreeContexts( ) { while( !m_contexts.empty() ) { FreeContext( m_contexts.begin()->first ); } }
    void FreeContext( int id ) { if( m_contexts.count(id) > 0 ) { m_contexts.erase( id ); } }

    void Set( int id, PDEFRAG_JOB_CONTEXT context ) { FreeContext( id ); m_contexts[id] = context; }
    PDEFRAG_JOB_CONTEXT Get( int id ) const { return m_contexts.count(id) > 0 ? m_contexts.find(id)->second : NULL; }

protected:
    std::map<int,PDEFRAG_JOB_CONTEXT> m_contexts;
};

CContextHolder g_SchedulerContextHolder;

bool GetAsSystemTime(const __time64_t& date, SYSTEMTIME& timeDest)
{
    struct tm ttm;
    errno_t err = _localtime64_s( &ttm, &date );
    if (err != 0) return false;

    timeDest.wYear = (WORD) (1900 + ttm.tm_year);
    timeDest.wMonth = (WORD) (1 + ttm.tm_mon);
    timeDest.wDayOfWeek = (WORD) ttm.tm_wday;
    timeDest.wDay = (WORD) ttm.tm_mday;
    timeDest.wHour = (WORD) ttm.tm_hour;
    timeDest.wMinute = (WORD) ttm.tm_min;
    timeDest.wSecond = (WORD) ttm.tm_sec;
    timeDest.wMilliseconds = 0;

    return true;
}


void
SetShedulerEvent( void )
{
    if ( Global.ShedulerTaskManagerUpdateEvent ) 
    {
        SetEvent( Global.ShedulerTaskManagerUpdateEvent );
    }
}


BOOL SetSchedulerTime( __time64_t date )
{
    __time64_t timer( date );
    __time64_t curTime( _time64(NULL) );
    if( timer < curTime ) timer = curTime;
    SYSTEMTIME sysTime;

    if( GetAsSystemTime( timer, sysTime ) )
    {
        LARGE_INTEGER fileTime;
        if( SystemTimeToFileTime( &sysTime,(FILETIME*) &fileTime ) )
        {
            LARGE_INTEGER fileUTCTime;
            LocalFileTimeToFileTime((FILETIME*) &fileTime,(FILETIME*) &fileUTCTime);

            if( !::SetWaitableTimer( ShedulerTaskManagerRunTimer, 
                &fileUTCTime, 0, NULL, NULL, TRUE ) )
            {
#ifdef _DEBUG_JOB
                OutMessage( L"   SetWaitableTimer failed" );
#endif
                return FALSE;
            }
            return TRUE;
        }
    }
    return FALSE;
}

int StopAndDeleteJob( PDEFRAG_JOB_CONTEXT context )
{
    context->Status |= DEFRAG_JOB_FLAG_STOPPED;
    SetEvent( context->ResumeEvent );
    FreeJob( context );
    return 0;
}
//
// Update all tasks list and return task for next run
//
int UpdateTaskList( DFRG_SHEDULED_JOB** jobToRun, BOOL* isJobInProgress )
{
#ifdef _DEBUG_JOB
    OutMessage( L"   ***UpdateTaskList: " );
#endif
    if( !jobToRun ) return ERROR_INVALID_PARAMETER;
    int Result = NO_ERROR;
    *jobToRun = NULL;
    if( isJobInProgress ) *isJobInProgress = FALSE;
    for( PSHEDULED_JOB_ITEM JobItem = DfrgSettings.SheduledJobs; JobItem != NULL; JobItem = JobItem->NextJob )
    {
        DFRG_SHEDULED_JOB& job = JobItem->Job;
        if( !job.ActiveStatus ) continue; // skip not active task
        if( isJobInProgress && g_SchedulerContextHolder.Get( job.Id ) != NULL ) *isJobInProgress = TRUE;
        if( job.Status == Scheduler::e_RunStatusInProgress )
        {
            PDEFRAG_JOB_CONTEXT context = g_SchedulerContextHolder.Get( job.Id );
            if( context && (context->Status & DEFRAG_JOB_FLAG_INPROGRESS) == 0 )
            {
                if( (context->Status & DEFRAG_JOB_FLAG_PROCESSED) != 0 )
                    job.Status = Scheduler::e_RunStatusCompleted;
                else if( (context->Status & DEFRAG_JOB_FLAG_ERROR) != 0 )
                    job.Status = Scheduler::e_RunStatusSkipped;
                else if( (context->Status & DEFRAG_JOB_FLAG_STOPPED) != 0 )
                    job.Status = Scheduler::e_RunStatusCancelled;
                else if( (context->Status & DEFRAG_JOB_FLAG_PAUSED) != 0 )
                    job.Status = Scheduler::e_RunStatusPaused;
                if( (context->Status & DEFRAG_JOB_FLAG_PAUSED) == 0 ) // if not Paused
                {
                    // Stop and Delete Active Job
#ifdef _DEBUG_JOB
                    OutMessage( L"   ***UpdateTaskList: Stop and Delete Completed Job" );
#endif
                    StopAndDeleteJob( context );
                    g_SchedulerContextHolder.FreeContext( job.Id );
                    //job.Status = Scheduler::e_RunStatusCompleted;
                }
            }
            else
            {
                continue;
            }
        }
        if( job.Status == Scheduler::e_RunStatusPaused ) continue;
        //if( job.Status == Scheduler::e_RunStatusCancelled ) continue;
        if( job.Status == Scheduler::e_RunStatusCompleted && job.Period == Scheduler::e_PeriodOneTime
            && job.ManualNextRunTime == 0 ) continue;

        __time64_t curTime( _time64(NULL) );

        switch( job.Period )
        {
        case Scheduler::e_PeriodOneTime:
            {
                job.NextRunTime = __max( job.StartTime, curTime );
            }
            break;

        case Scheduler::e_PeriodDay:
            {
                job.NextRunTime = __max( job.StartTime, job.LastRunTime + job.DayPeriod * SCHEDULER_DAY_SPAN );
                job.NextRunTime = __max( job.NextRunTime, curTime );
            }
            break;

        case Scheduler::e_PeriodWeek:
            {
                __time64_t djustedStartTime = job.StartTime;
                __time64_t djustedLastRunTime = job.LastRunTime + SCHEDULER_DAY_SPAN;
                SYSTEMTIME sysTime;
                while( GetAsSystemTime( djustedStartTime, sysTime ) ) // adjust StartTime
                {
                    if( job.WeekDay == Scheduler::e_WeekDaySunday && sysTime.wDayOfWeek == 0 ) break;
                    if( job.WeekDay + 1 == sysTime.wDayOfWeek ) break;
                    djustedStartTime += SCHEDULER_DAY_SPAN;
                }
                while( GetAsSystemTime( djustedLastRunTime, sysTime ) ) // adjust LastRunTime
                {
                    if( job.WeekDay == Scheduler::e_WeekDaySunday && sysTime.wDayOfWeek == 0 ) break;
                    if( job.WeekDay + 1 == sysTime.wDayOfWeek ) break;
                    djustedLastRunTime += SCHEDULER_DAY_SPAN;
                }
                job.NextRunTime = __max( djustedStartTime, djustedLastRunTime );
                job.NextRunTime = __max( job.NextRunTime, curTime );
            }
            break;

        case Scheduler::e_PeriodSceenSaver:
            {
                // TODO
            }
            break;
        }
        if( job.LastRunTime + SCHEDULER_TIME_SPAN >= job.NextRunTime ) job.NextRunTime +=SCHEDULER_TIME_SPAN;
        if( job.ManualNextRunTime != 0 && job.NextRunTime > job.ManualNextRunTime ) job.NextRunTime = job.ManualNextRunTime;
        if( job.DefragMode == Scheduler::e_DefragModeBoot ) continue;
        if( !*jobToRun ) *jobToRun = &job;
        if( (*jobToRun)->NextRunTime > job.NextRunTime ) *jobToRun = &job;
    }
    DfrgSettings.SchedulerLastUpdateTime = (DWORD)_time64( NULL );
    return Result;
} // end of UpdateTaskList

//////////////////////////////////////////////////////////////////////////
int RunScheduleTask(DFRG_SHEDULED_JOB* jobToRun, BOOL waitFinish )
{
    if( !jobToRun ) return ERROR_INVALID_PARAMETER;

#ifdef _DEBUG_JOB
    OutMessage( L"   RunScheduleTask: " );
    OutMessage( jobToRun->Name );
#endif

    int mode = DEFRAG_JOB_ANALIZE_ONLY;
    switch( jobToRun->Method )
    {
    case Scheduler::e_DefragSimple:
        mode = DEFRAG_JOB_SIMPLE_DEFRAG;
        break;
    case Scheduler::e_DefragFreeSpace:
        mode = DEFRAG_JOB_FREE_SPACE;
        break;
    case Scheduler::e_DefragByName:
        mode = DEFRAG_JOB_SMART_BY_NAME;
        break;
    case Scheduler::e_DefragByAccess:
        mode = DEFRAG_JOB_SMART_BY_ACCESS;
        break;
    case Scheduler::e_DefragByModification:
        mode = DEFRAG_JOB_SMART_BY_MODIFY;
        break;
    case Scheduler::e_DefragByCreation:
        mode = DEFRAG_JOB_SMART_BY_CREATE;
        break;
    case Scheduler::e_DefragBySize:
        mode = DEFRAG_JOB_SMART_BY_SIZE;
        break;
    case Scheduler::e_DefragForce:
        mode = DEFRAG_JOB_FORCE_TOGETHER;
        break;
    }

    PDEFRAG_JOB_CONTEXT context = NULL;
    WCHAR drive[3] = L"C:";
    if( jobToRun->DrivesType == Scheduler::e_DrivesSelected )
    {
        drive[0] = jobToRun->DrivesList[0];
    }

    int Result = DfrgInitJobCtx( &context, drive, mode );

#ifdef _DEBUG_JOB
    WCHAR                       Mess[MAX_PATH];
    swprintf( Mess, L"--- InitJob Result %d ", Result );
    OutMessage ( Mess );
#endif

    jobToRun->LastRunTime = _time64(0);
    jobToRun->ManualNextRunTime = 0;
    if ( NO_ERROR == Result )
    {
        g_SchedulerContextHolder.Set( jobToRun->Id, context );
        //
        // Insert Job
        //
        InsertJob( context );
        if( NO_ERROR == StartJobThread( context ) )
        {
            jobToRun->Status = Scheduler::e_RunStatusInProgress;
        }
        else
        {
#ifdef _DEBUG_JOB
            OutMessage( L"!!!Failed to start job." );
#endif
            jobToRun->Status = Scheduler::e_RunStatusDeferred;
            StopAndDeleteJob( context );
            g_SchedulerContextHolder.FreeContext( jobToRun->Id );
        }
    }
    else
    {
        jobToRun->Status = Scheduler::e_RunStatusDeferred;
    }

    return Result;
}

//////////////////////////////////////////////////////////////////////////
// Thread for manage Schedule tasks
DWORD 
WINAPI 
ShedulerTaskManager(
    LPVOID Parameter )
{
    DWORD               Result = NO_ERROR;
    HANDLE              hEvents[] = 
    {
        ShedulerTaskManagerRunTimer,  // run task
        Global.ShedulerTaskManagerUpdateEvent,  // update tasks
        Global.ShedulerTaskManagerStopEvent,   // stop scheduler
    };

    enum{ e_RunEventReady, e_UpdateEventReady, e_StopEventReady };

    DWORD               dwResult = e_UpdateEventReady;
    DFRG_SHEDULED_JOB*  jobToRun = NULL;


#ifdef _DEBUG_JOB
    OutMessage( L"   ShedulerTaskManager started" );
#endif
    do
    {
        BOOL isJobInProgress = FALSE;

        if ( (Result = UpdateTaskList(&jobToRun, &isJobInProgress)) != NO_ERROR )
        {
#ifdef _DEBUG_JOB
            OutMessage( L"   ShedulerTaskManager failed to update task list" );
#endif
            break;
        }

        //
        // TODO: Check SchedulerRunMissedEnable
        //
        if ( jobToRun ) 
        {
            SetSchedulerTime( jobToRun->NextRunTime );
        }

        DWORD   waitTime = INFINITE;

        if ( isJobInProgress ) 
        {
            waitTime = 5000;
#ifdef _DEBUG_JOB
            OutMessage( L"   ShedulerTaskManager detect active task and set timer to 5 sec" );
#endif
        }

        dwResult = WaitForMultipleObjectsEx( sizeof(hEvents)/sizeof(hEvents[0]), hEvents, FALSE, waitTime, TRUE );

        ::CancelWaitableTimer( ShedulerTaskManagerRunTimer );

        switch( dwResult )
        {
        case e_RunEventReady:
            {
                // Run task
#ifdef _DEBUG_JOB
                OutMessage( L"   ShedulerTaskManager. Run Task" );
#endif
                RunScheduleTask( jobToRun, FALSE );
            }
            break;
        case e_UpdateEventReady:
            {
#ifdef _DEBUG_JOB
                OutMessage( L"   ==>ShedulerTaskManager. Update Task List" );
#endif
                //ResetEvent( Global.ShedulerTaskManagerUpdateEvent );
                // will be updated automatically by next iteration
            }
            break;
        case WAIT_TIMEOUT:
            {
#ifdef _DEBUG_JOB
                OutMessage( L"   ==>ShedulerTaskManager. Job in Progress -> Update Task List" );
#endif
            }
            break;
        }
    } while( dwResult != e_StopEventReady );

#ifdef _DEBUG_JOB
    OutMessage( L"   ShedulerTaskManager finished." );
#endif
    
    return Result;

} // end of ShedulerTaskManager

//
// Start Scheduler Thread, which will be manage the all scheduled tasks
//
int StartSchedulerTaskManager()
{
    int Result = NO_ERROR;

#ifdef _DEBUG_JOB
    OutMessage( L"StartSchedulerTaskManager" );
#endif

    Global.ShedulerTaskManagerUpdateEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
    Global.ShedulerTaskManagerStopEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
    // create waitable timer   
    ShedulerTaskManagerRunTimer = CreateWaitableTimer( NULL, FALSE, NULL );
    // create working thread
    HANDLE ShedulerTaskManagerThread = CreateThread( 
        NULL, 
        0, 
        ShedulerTaskManager, 
        NULL, // parameter
        0, 
        NULL ); 

    if ( ShedulerTaskManagerThread ) 
    {
        Global.ShedulerTaskManagerThread = ShedulerTaskManagerThread;
        Global.AppStatus |= APP_STAT_SHEDULER_TASK_MANAGER_ACTIVED;
#ifdef _DEBUG_JOB
        OutMessage( L"StartSchedulerTaskManager: ShedulerTaskManager thread started" );
#endif
    }
    else
    {
#ifdef _DEBUG_JOB
        OutMessage( L"StartSchedulerTaskManager: Failed to start ShedulerTaskManager thread" );
#endif
        if( ShedulerTaskManagerRunTimer )
        {
            CloseHandle( ShedulerTaskManagerRunTimer );
            ShedulerTaskManagerRunTimer = NULL;
        }
        if( Global.ShedulerTaskManagerStopEvent )
        {
            CloseHandle( Global.ShedulerTaskManagerStopEvent );
            Global.ShedulerTaskManagerStopEvent = NULL;
        }
        Result = ERROR_NOT_ENOUGH_MEMORY;
    }

    return Result;
} // end of StartSchedulerTaskManager

//
// StopSchedulerTaskManager
//
void StopSchedulerTaskManager()
{
#ifdef _DEBUG_JOB
    OutMessage( L"StopSchedulerTaskManager started" );
#endif
    Global.AppStatus &= ~APP_STAT_SHEDULER_TASK_MANAGER_ACTIVED;
    if ( Global.ShedulerTaskManagerThread )
    {
        if( Global.ShedulerTaskManagerStopEvent )   // try to stop in normal way
        {
            SetEvent( Global.ShedulerTaskManagerStopEvent );
        }
        if( WAIT_TIMEOUT == WaitForSingleObject( Global.ShedulerTaskManagerThread, 1000 ) ) // wait for correct closing
        {
            TerminateThread( Global.ShedulerTaskManagerThread, 0 ); // close thread by terminal mode
#ifdef _DEBUG_JOB
            OutMessage( L"   StopSchedulerTaskManager. ShedulerTaskManagerThread terminated" );
#endif
        }

        WaitForSingleObject( Global.ShedulerTaskManagerThread, INFINITE );

        // Clearing
        CloseHandle( Global.ShedulerTaskManagerThread );
        Global.ShedulerTaskManagerThread = NULL;

        if( ShedulerTaskManagerRunTimer )
        {
            CloseHandle( ShedulerTaskManagerRunTimer );
            ShedulerTaskManagerRunTimer = NULL;
        }
        if( Global.ShedulerTaskManagerStopEvent )
        {
            CloseHandle( Global.ShedulerTaskManagerStopEvent );
            Global.ShedulerTaskManagerStopEvent = NULL;
        }
        if( Global.ShedulerTaskManagerUpdateEvent )
        {
            CloseHandle( Global.ShedulerTaskManagerUpdateEvent );
            Global.ShedulerTaskManagerUpdateEvent = NULL;
        }

#ifdef _DEBUG_JOB
        OutMessage( L"   StopSchedulerTaskManager finished" );
#endif
    }
} // end of StopSchedulerTaskManager

