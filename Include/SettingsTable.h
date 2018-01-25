/*

    SettingsTable

    Module name:

        SettingsTable.h

    Abstract:

        Contains unified list of Defrag settings. 
        Used by server to define SettingsTable.
        Used by clients to define Settings Id's.

    
    $Header: /home/CVS_DEFRAG/Defrag/Src/Include/SettingsTable.h,v 1.2 2009/12/25 15:58:15 dimas Exp $
    $Log: SettingsTable.h,v $
    Revision 1.2  2009/12/25 15:58:15  dimas
    Priority control implemented

    Revision 1.1  2009/11/24 14:51:52  dimas
    no message


*/



//
// Control settings
//
// Excluded File names list
//
SETTING( ExcludeFilesEnable,     TYPE_DWORD,    NULL, NULL,                     0 )
SETTING( ExcludedFiles,          TYPE_MULTI_SZ, NULL, DfrgSetFilteringList,     NULL )

//
// Popular options page
//
SETTING( BootOptimizationEnable, TYPE_DWORD,    NULL, NULL,                     0 )
SETTING( AutoUpdateEnable,       TYPE_DWORD,    NULL, NULL,                     0 )

//
// Defrag options
//
SETTING( AllowDefragFlash,       TYPE_DWORD,    NULL, NULL,                     0 )
SETTING( AllowDefragRemovable,   TYPE_DWORD,    NULL, NULL,                     0 )
SETTING( ThresholdEnable,        TYPE_DWORD,    NULL, NULL,                     0 )
SETTING( ThresholdSize,          TYPE_DWORD,    NULL, NULL,                     50 )
SETTING( AggressivelyFreeSpaceEnable, TYPE_DWORD, NULL, NULL,                   0 )
SETTING( AdjustCpuPriority,      TYPE_DWORD,    NULL, DfrgSetCpuPriority,       0 )
SETTING( CpuPriority,            TYPE_DWORD,    NULL, DfrgSetCpuPriority,       DfgCpuPriorityNormal )
SETTING( ThrottleEnable,         TYPE_DWORD,    NULL, NULL,                     0 )
SETTING( VssEnable,              TYPE_DWORD,    NULL, NULL,                     0 )
SETTING( VssMode,                TYPE_DWORD,    NULL, NULL,                     DfgVSSModeStop )

//
// Scheduler options
//
SETTING( SchedulerNotStartBatteryEnable, TYPE_DWORD, NULL, NULL,                0 )
SETTING( SchedulerStopBatteryEnable, TYPE_DWORD, NULL, NULL,                    0 )
SETTING( SchedulerWakeRunEnable, TYPE_DWORD,    NULL, NULL,                     0 )
SETTING( SchedulerRunMissedEnable, TYPE_DWORD,  NULL, NULL,                     0 )

//
// Max File size in clusters
//
SETTING( MaxFileSize,            TYPE_DWORD,    NULL, NULL,                     DfrgMaxFileSizeDef )

//
// LogFile path & logging granularity
//
SETTING( LogFilePath,            TYPE_STRING,   NULL, NULL,                     DfrgLogFilePathDef )
SETTING( LoggingDepth,           TYPE_DWORD,    NULL, DfrgSetLoggingDepth,      DfrgLoggingDepthDef )
SETTING( ApplicationLogEnable,   TYPE_DWORD,    NULL, NULL,                     0 )
SETTING( EventLogEnable,         TYPE_DWORD,    NULL, NULL,                     0 )

//
// Scheduled Jobs
//
SETTING( SheduledJobs,           TYPE_JOB,      NULL, DfrgSetJobList,           NULL )
SETTING( SchedulerLastUpdateTime, TYPE_DWORD,   NULL, NULL,                     0 )


