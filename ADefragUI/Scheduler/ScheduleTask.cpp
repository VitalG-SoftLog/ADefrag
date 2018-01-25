/*
    Interface of the CScheduleTask class

    Module name:

    ScheduleTask.cpp

    Abstract:

    Interface of the CScheduleTask class

    $Header: /home/CVS_DEFRAG/Defrag/Src/ADefragUI/Scheduler/ScheduleTask.cpp,v 1.2 2009/11/24 18:02:52 roman Exp $
    $Log: ScheduleTask.cpp,v $
    Revision 1.2  2009/11/24 18:02:52  roman
    Initial commit

    Revision 1.1  2009/11/24 14:57:02  dimas
    no message

*/

#include "stdafx.h"
#include "ScheduleTask.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CScheduleTask 

CScheduleTask::CScheduleTask() :
    m_id( -1 ),
    m_active( TRUE ),
    m_period( Scheduler::e_PeriodOneTime ),
    m_drivesType( Scheduler::e_DrivesAll ),
    m_sequenceType( Scheduler::e_SequenceAuto ),
    m_defragMethod( Scheduler::e_DefragSimple ),
    m_defragMode( Scheduler::e_DefragModeNormal ),
    m_startTime( CTime::GetCurrentTime() ),
    m_dayPeriod( 1 ),
    m_weekDay( Scheduler::e_WeekDayMonday ),
    m_enableShutdown( FALSE ),
    m_lastRunStatus( Scheduler::e_RunStatusNotRunned )
{
}

CScheduleTask::~CScheduleTask()
{
}

void CScheduleTask::GetServerData( PDFRG_SHEDULED_JOB data )
{
    if( !data ) return;
    data->Id = m_id;
    CString idFormat;
    idFormat.Format( _T("%d"), m_id );
    wcscpy( data->Name, idFormat );
    data->ActiveStatus = m_active;
    data->Period = m_period;
    // What to defrag
    data->DrivesType = m_drivesType;
    wcscpy( data->DrivesList, m_drivesList );
    data->Sequence = m_sequenceType;
    // How to defrag
    data->Method = m_defragMethod;
    data->DefragMode = m_defragMode;
    // When to defrag
    data->StartTime = m_startTime.GetTime();
    data->DayPeriod = m_dayPeriod;
    data->WeekDay = m_weekDay;
    data->EnableShutdown = m_enableShutdown;

    // Status
    data->Status = m_lastRunStatus;
    data->LastRunTime = m_lastRun.GetTime();
    data->NextRunTime = m_nextRun.GetTime();
    data->ManualNextRunTime = m_manualNextRunTime.GetTime();
}

void CScheduleTask::SetServerData( const PDFRG_SHEDULED_JOB data )
{
    if( !data ) return;
    m_id = data->Id;
    m_active= data->ActiveStatus;
    m_period = data->Period;
    // What to defrag
    m_drivesType = data->DrivesType;
    m_drivesList = data->DrivesList;
    m_sequenceType = data->Sequence;
    // How to defrag
    m_defragMethod = data->Method;
    m_defragMode = data->DefragMode;
    // When to defrag
    m_startTime = data->StartTime;
    m_dayPeriod = data->DayPeriod;
    m_weekDay = data->WeekDay;
    m_enableShutdown = data->EnableShutdown;

    // Status
    m_lastRunStatus = data->Status;
    m_lastRun = data->LastRunTime;
    m_nextRun = data->NextRunTime;
    m_manualNextRunTime = data->ManualNextRunTime;
}

CString CScheduleTask::GetMethodDescription( Scheduler::EScheduleDefragMethod defragMethod )
{
    CString description;
    description.LoadString(IDS_DEFRAG_UNKNOWN);
    switch( defragMethod )
    {
    case Scheduler::e_DefragSimple: description.LoadString(IDS_DEFRAG_SIMPLE); break;
    case Scheduler::e_DefragFreeSpace: description.LoadString(IDS_DEFRAG_FREE_SPACE); break;
    case Scheduler::e_DefragByName: description.LoadString(IDS_DEFRAG_BY_NAME); break;
    case Scheduler::e_DefragByAccess: description.LoadString(IDS_DEFRAG_BY_ACCESS); break;
    case Scheduler::e_DefragByModification: description.LoadString(IDS_DEFRAG_BY_MODIFY); break;
    case Scheduler::e_DefragByCreation: description.LoadString(IDS_DEFRAG_BY_CTEATE); break;
    case Scheduler::e_DefragBySize: description.LoadString(IDS_DEFRAG_BY_SIZE); break;
    case Scheduler::e_DefragForce: description.LoadString(IDS_DEFRAG_BY_FORCE); break;
    }
    return description;
}

CString CScheduleTask::GetWeekDayDescription( Scheduler::EScheduleWeekDay weekDay )
{
    CString description;
    description.LoadString(IDS_WEEKDAY_UNKNOWN);
    switch( weekDay )
    {
    case Scheduler::e_WeekDayMonday: description.LoadString(IDS_WEEKDAY_MONDAY); break;
    case Scheduler::e_WeekDayTuesday: description.LoadString(IDS_WEEKDAY_TUESDAY); break;
    case Scheduler::e_WeekDayWednesday: description.LoadString(IDS_WEEKDAY_WEDNESDAY); break;
    case Scheduler::e_WeekDayThursday: description.LoadString(IDS_WEEKDAY_THURSDAY); break;
    case Scheduler::e_WeekDayFriday: description.LoadString(IDS_WEEKDAY_FRIDAY); break;
    case Scheduler::e_WeekDaySaturday: description.LoadString(IDS_WEEKDAY_SATURDAY); break;
    case Scheduler::e_WeekDaySunday: description.LoadString(IDS_WEEKDAY_SUNDAY); break;
    }
    return description;
}

CString CScheduleTask::GetActionDescription()
{
    CString description;
    if( m_drivesType == Scheduler::e_DrivesAll )
        description.Format( IDS_SCHEDULE_ACTION_ALLDISKS_FORMAT, GetMethodDescription( m_defragMethod ) );
    else
        description.Format( IDS_SCHEDULE_ACTION_SELDISKS_FORMAT, GetMethodDescription( m_defragMethod ), m_drivesList );
    return description;
}

CString CScheduleTask::GetTypeDescription()
{
    CString description;
    switch( m_period )
    {
    case Scheduler::e_PeriodOneTime: description.LoadString(IDS_PERIOD_ONETIME); break;
    case Scheduler::e_PeriodDay: 
        if( m_dayPeriod == 1 ) description.LoadString( IDS_PERIOD_DAILY );
        else description.Format( IDS_PERIOD_DAYS, m_dayPeriod );
        break;
    case Scheduler::e_PeriodWeek: description.Format( IDS_PERIOD_WEEKLY, GetWeekDayDescription( m_weekDay ) ); break;
    case Scheduler::e_PeriodSceenSaver: description.LoadString( IDS_PERIOD_SCREENSAVER );
    }
    return description;
}

CTime CScheduleTask::GetLastRunTime()
{
    return m_lastRun;
}

CString CScheduleTask::GetLastRunStatus()
{
    CString description;
    switch( m_lastRunStatus )
    {
    case Scheduler::e_RunStatusNotRunned: description.LoadString(IDS_RUNSTATUS_PROJECTED); break;
    case Scheduler::e_RunStatusInProgress: description.LoadString(IDS_RUNSTATUS_INPROGRESS); break;
    case Scheduler::e_RunStatusCompleted: description.LoadString(IDS_RUNSTATUS_COMPLETED); break;
    case Scheduler::e_RunStatusSkipped: description.LoadString(IDS_RUNSTATUS_SKIPPED); break;
    case Scheduler::e_RunStatusCancelled: description.LoadString(IDS_RUNSTATUS_CANCELLED); break;
    case Scheduler::e_RunStatusDeferred: description.LoadString(IDS_RUNSTATUS_DEFERRED); break;
    }
    return description;
}

CTime CScheduleTask::GetNextRunTime()
{
    return m_nextRun;
}
