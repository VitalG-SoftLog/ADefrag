/*
    Defrag Server Settings header

    Module name:

        Settings.h

    Abstract:

        Defrag Server Settings header.
        Defines settings internal constants, types, structures, functions, etc.

    $Header: /home/CVS_DEFRAG/Defrag/Src/Defrag01/Settings.h,v 1.2 2009/12/21 17:00:13 dimas Exp $
    $Log: Settings.h,v $
    Revision 1.2  2009/12/21 17:00:13  dimas
    1. DEFRAG_CMD_GET_MOST_FRAGMENTED and similar requests implemented
    2. Basic Exclude files check added

    Revision 1.1  2009/11/24 14:52:31  dimas
    no message

*/

#ifndef __DEFRAG_SETTINGS_H__
#define __DEFRAG_SETTINGS_H__



// =================================================
//
// Constants, Macros & Data
//
// =================================================

//#define SERVICES_SUBKEY     L"HKEY_LOCAL_MACHINE\\System\\CurrentControlSet\\Services\\"
#define SERVICES_SUBKEY     L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\"

#define PARAMETERS_SUBKEY   L"\\Parameters"

#define JOBS_SUBKEY         L"\\Jobs"


//
// Valid data types enumerator (mainly derives from valid Registry data types)
//
typedef enum _SETTING_TYPE
{
    TYPE_DWORD,
    TYPE_STRING,
    TYPE_MULTI_SZ,
    //TYPE_BINARY,
    TYPE_JOB,
    TYPE_MAX_ID
} SETTING_TYPE, *PSETTING_TYPE;

//
// Registry Save / Read methods for defined data types.
//
typedef ULONG (__stdcall READWRITE_SETTING)(
    PWCHAR  SettingName,
    PVOID   SettingAddress);

typedef READWRITE_SETTING *PREADWRITE_SETTING;

READWRITE_SETTING DfrgRegReadList;
READWRITE_SETTING DfrgRegWriteList;

READWRITE_SETTING DfrgRegReadDword;
READWRITE_SETTING DfrgRegWriteDword;

READWRITE_SETTING DfrgRegReadString;
READWRITE_SETTING DfrgRegWriteString;

//
// SetRoution serves for data validation.
//
typedef int (__stdcall *PSET_ROUTINE)(
    IN SETTING_ID   SettingId,
    IN PVOID        Address);

//
// Setting definition structure.
//
typedef struct _SETTING_META_INFO
{
    ULONG           SettingId;
    PWCHAR          Name;
    SETTING_TYPE    Type;
    PVOID           Address;
    PVOID           Context;
    PSET_ROUTINE    SetRoutine;
    PVOID           DefaultValue;
} SETTING_META_INFO, *PSETTING_META_INFO;


//
// Set(Add) / Remove / Get  methods for defined data types.
// Operations enumerator is defined in DfrgPrivate.h as it used by usermode API
//
typedef ULONG (__stdcall TYPE_METHOD)(
    PVOID     SettingAddress,
    PVOID     SettingBuffer,
    PULONG    SettingBufferSize);

typedef TYPE_METHOD *PTYPE_METHOD;

TYPE_METHOD DfrgSetDword;
TYPE_METHOD DfrgGetDword;

TYPE_METHOD DfrgSetString;
TYPE_METHOD DfrgGetString;

TYPE_METHOD DfrgAddItemsToList;
TYPE_METHOD DfrgRemoveItemsFromList;
TYPE_METHOD DfrgGetList;


//
// This entry combine all methods for given data type.
//
typedef struct _TYPE_VTABLE_ENTRY
{
    PTYPE_METHOD        Methods[OP_MAX_ID];
    PREADWRITE_SETTING  ReadSetting;            // Reads setting from registry
    PREADWRITE_SETTING  WriteSetting;           // Saves setting to registry
} TYPE_VTABLE_ENTRY, *PTYPE_VTABLE_ENTRY;



typedef struct _SHEDULED_JOB_ITEM
{
    DFRG_SHEDULED_JOB               Job;
    struct _SHEDULED_JOB_ITEM       *NextJob;
    struct _SHEDULED_JOB_ITEM       *PrevJob;

} SHEDULED_JOB_ITEM, *PSHEDULED_JOB_ITEM;


//
// Defrag Settings structure
//
typedef struct _DFRG_SETTINGS 
{
    CRITICAL_SECTION       SyncObject;

    //
    // Defrag control settings
    //
    // Max File size in clusters
    //
    DWORD                   MaxFileSize;

    //
    // Excluded File names list
    //
    DWORD                   ExcludeFilesEnable; // TRUE/FALSE
    UNICODE_STRING          ExcludedFiles;

    //
    // Popular options page
    //
    DWORD                   BootOptimizationEnable; // TRUE/FALSE
    DWORD                   AutoUpdateEnable; // TRUE/FALSE

    //
    // Defrag options
    //
    DWORD                   AllowDefragFlash; // TRUE/FALSE
    DWORD                   AllowDefragRemovable; // TRUE/FALSE
    DWORD                   ThresholdEnable; // TRUE/FALSE
    DWORD                   ThresholdSize; // 1..100
    DWORD                   AggressivelyFreeSpaceEnable; // TRUE/FALSE
    DWORD                   AdjustCpuPriority; // TRUE/FALSE
    DWORD                   CpuPriority; // 0 - DfgCpuPriorityNormal, 1 - DfgCpuPriorityIdle, 2 - DfgCpuPriorityTurbo
    DWORD                   ThrottleEnable; // TRUE/FALSE
    DWORD                   VssEnable; // TRUE/FALSE
    DWORD                   VssMode; // 0 - DfgVSSModeStop, 1 - DfgVSSModeCompatible, 2 - DfgVSSModeNormal

    //
    // Scheduler options
    //
    DWORD                   SchedulerNotStartBatteryEnable; // TRUE/FALSE
    DWORD                   SchedulerStopBatteryEnable; // TRUE/FALSE
    DWORD                   SchedulerWakeRunEnable; // TRUE/FALSE
    DWORD                   SchedulerRunMissedEnable; // TRUE/FALSE

    //
    // LogFile path & logging granularity
    //
    DWORD                   LoggingDepth;
    UNICODE_STRING          LogFilePath;
    DWORD                   ApplicationLogEnable; // TRUE/FALSE
    DWORD                   EventLogEnable; // TRUE/FALSE

    //
    // List of scheduled jobs
    //
    PSHEDULED_JOB_ITEM      SheduledJobs;
    DWORD                   SchedulerLastUpdateTime;

} DFRG_SETTINGS, *PDFRG_SETTINGS;

extern DFRG_SETTINGS DfrgSettings;


// =================================================
//
// Function prototypes
//
// =================================================

void
DfrgInitializeSettings(
    WCHAR   *RegistryPath);

void
DfrgUninitializeSettings(VOID);

VOID
DfrgApiReadSettings(VOID);

int              
DfrgApiSettingOp(
    IN     ULONG        SettingId,
    IN     ULONG        OpId,
    IN OUT PVOID        SettingBuffer,
    IN OUT PULONG       SettingBufferSize );

PWCHAR 
GetOpString( 
    ULONG   OpId );

/*
VOID 
LockSettings( 
    PVOID      Resource );

VOID 
UnlockSettings(
    PVOID      Resource );
*/

#define     LockSettings( Resource )    EnterCriticalSection( Resource )
#define     UnlockSettings( Resource )  LeaveCriticalSection( Resource )

#endif __DEFRAG_SETTINGS_H__
