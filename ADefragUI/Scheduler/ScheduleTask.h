/*
    Interface of the CScheduleTask class

    Module name:

    ScheduleTask.h

    Abstract:

    Interface of the CScheduleTask class

    $Header: /home/CVS_DEFRAG/Defrag/Src/ADefragUI/Scheduler/ScheduleTask.h,v 1.2 2009/11/24 18:02:52 roman Exp $
    $Log: ScheduleTask.h,v $
    Revision 1.2  2009/11/24 18:02:52  roman
    Initial commit

    Revision 1.1  2009/11/24 14:57:02  dimas
    no message

*/

#ifndef __SCHEDULETASK_H__
#define __SCHEDULETASK_H__

#pragma once

#define TASK_DRIVES_DELIMETER       _T(";")

/////////////////////////////////////////////////////////////////////////////
// CScheduleTask 

class CScheduleTask
{
// Construction
public:
    CScheduleTask();
    virtual ~CScheduleTask();

    int GetID() const { return m_id; }
    void SetID( int id ) { m_id = id; }

    BOOL IsActive() const { return m_active; }
    void SetActive( BOOL active ) { m_active = active; }

    //enum EPeriodType { e_PeriodOneTime, e_PeriodDay, e_PeriodWeek, e_PeriodSceenSaver };
    Scheduler::EPeriodType GetPeriod() const { return m_period; }
    void SetPeriod( Scheduler::EPeriodType period ) { m_period = period; }

    //enum EDrivesType { e_DrivesAll, e_DrivesSelected };
    Scheduler::EDrivesType GetDrivesType() const { return m_drivesType; }
    void SetDrivesType( Scheduler::EDrivesType type ) { m_drivesType = type; }

    //enum ESequenceType { e_SequenceOneByOne, e_SequenceAtOneTime, e_SequenceAuto };
    Scheduler::ESequenceType GetSequenceType() const { return m_sequenceType; }
    void SetSequenceType( Scheduler::ESequenceType type ) { m_sequenceType = type; }

    CString GetDrivesList() const { return m_drivesList; }
    void SetDrivesList( LPCTSTR list ) { m_drivesList = list; }

    //enum EScheduleDefragMethod { e_DefragSimple, e_DefragFreeSpace, e_DefragByName, e_DefragByAccess, eDefragByModification, e_DefragByCreation, e_DefragBySize, e_DefragForce };
    Scheduler::EScheduleDefragMethod GetDefragMethod() const { return m_defragMethod; }
    void SetDefragMethod( Scheduler::EScheduleDefragMethod method ) { m_defragMethod = method; }

    //enum EScheduleDefragMode { e_DefragModeNormal, e_DefragModeSmart, e_DefragModeBoot };
    Scheduler::EScheduleDefragMode GetDefragMode() const { return m_defragMode; }
    void SetDefragMode( Scheduler::EScheduleDefragMode mode ) { m_defragMode = mode; }

    CTime GetStartDateTime() const { return m_startTime; }
    void SetStartDateTime( CTime& time ) { m_startTime = time; }

    int GetDayPeriod() const { return m_dayPeriod; }
    void SetDayPeriod( int period ) { m_dayPeriod = period; }

    //enum EScheduleWeekDay { e_WeekDayMonday, e_WeekDayTuesday, e_WeekDayWednesday, e_WeekDayThursday, e_WeekDayFriday, e_WeekDaySaturday, e_WeekDaySunday };
    Scheduler::EScheduleWeekDay GetWeekDay() const { return m_weekDay; }
    void SetWeekDay( Scheduler::EScheduleWeekDay day ) { m_weekDay = day; }

    BOOL GetEnableShutdown() const { return m_enableShutdown; }
    void SetEnableShutdown( BOOL enable ) { m_enableShutdown = enable; }

    //enum EScheduleRunStatus { e_RunStatusNotRunned, e_RunStatusInProgress, e_RunStatusCompleted, e_RunStatusSkipped, e_RunStatusCancelled, e_RunStatusDeferred };

    //////////////////////////////////////////////////////////////////////////
    // Server data function
    void GetServerData( PDFRG_SHEDULED_JOB data );
    void SetServerData( const PDFRG_SHEDULED_JOB data );

    //////////////////////////////////////////////////////////////////////////
    static CString GetMethodDescription( Scheduler::EScheduleDefragMethod defragMethod );
    static CString GetWeekDayDescription( Scheduler::EScheduleWeekDay weekDay );
    //////////////////////////////////////////////////////////////////////////
    CString GetActionDescription();
    CString GetTypeDescription();
    CTime GetLastRunTime();
    CString GetLastRunStatus();
    Scheduler::EScheduleRunStatus GetTaskStatus() const { return m_lastRunStatus; }
    CTime GetNextRunTime();
    void SetManualNextRunTime( CTime& time ) { m_manualNextRunTime = time; }

protected:
    BOOL m_active;
    Scheduler::EPeriodType m_period;
    Scheduler::EDrivesType m_drivesType;
    Scheduler::ESequenceType m_sequenceType;
    CString m_drivesList;
    Scheduler::EScheduleDefragMethod m_defragMethod;
    Scheduler::EScheduleDefragMode m_defragMode;
    CTime m_startTime;
    int m_dayPeriod;
    Scheduler::EScheduleWeekDay m_weekDay;
    BOOL m_enableShutdown;
    // Service specific data
    int m_id;
    CTime m_lastRun;
    Scheduler::EScheduleRunStatus m_lastRunStatus;
    CTime m_nextRun;
    CTime m_manualNextRunTime;
};

typedef std::vector<CScheduleTask>  CScheduleTaskList;

#endif // __SCHEDULETASK_H__
