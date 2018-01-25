/*
    Defrag interface header

    Module name:

        Api.h

    Abstract:

        Defrag interface header.
        Defines interface and communication constants, types, structures, functions, etc.

    $Header: /home/CVS_DEFRAG/Defrag/Src/Shared/Api.h,v 1.8 2009/12/23 13:01:46 dimas Exp $
    $Log: Api.h,v $
    Revision 1.8  2009/12/23 13:01:46  dimas
    1. Full File Name processing implemented.
    2. Top 100 Fragmented functionality for all file list requests implemented.

    Revision 1.7  2009/12/21 17:00:13  dimas
    1. DEFRAG_CMD_GET_MOST_FRAGMENTED and similar requests implemented
    2. Basic Exclude files check added

    Revision 1.6  2009/12/16 14:13:34  dimas
    DEFRAG_CMD_GET_FILE_INFO request implemented

    Revision 1.5  2009/12/15 16:06:40  dimas
    OP_REMOVE_ALL pseudo operation on settings lists added

    Revision 1.4  2009/12/03 13:01:06  dimas
    Cluster Inspector implemented

    Revision 1.3  2009/12/02 14:42:15  dimas
    App/Service interaction improved

    Revision 1.2  2009/11/30 14:58:47  dimas
    Currently processing file name visualization implimented

    Revision 1.1  2009/11/24 14:51:31  dimas
    no message

*/

#ifndef __DFRG_API_H__
#define __DFRG_API_H__


//#pragma pack( push, 1 )


// ============================================
//
//       General definitions
//
// =============================================

//
// Communication Protocol Version
//
#define     DEFRAG_INTERNAL_PROTOCOL_VERSION        0x00010001

#define     DEFRAG_EXTERNAL_PROTOCOL_VERSION        0x00010001


//
// Global Names
//
#define SERVICE_NAME            L"ADefragService"

//#define DFRG_PORT_NAME          L"\\ADfrgPort"
#define DFRG_PORT_NAME          L"\\GLOBAL??\\ADfrgPort"


//
// Operation modes (For Application Global Structure)
//
#define ShowHelpMode                 -1L

#define DoJobMode                    201

#define RunAsServiceMode             100
#define StartServiceMode             101
#define StopServiceMode              102
#define RegisterServiceMode          103
#define UnregisterServiceMode        104

#define GuiAppId                     201
#define CmdAppId                     202
#define ServiceAppId                 203
#define NativeAppId                  204


// ============================================
//
//       Error codes & debug messages
//
// =============================================

#define DFRG_ERROR_ABORTED                    (-1)
#define DFRG_ERROR_SKIP         DFRG_ERROR_ABORTED

#define DFRG_ERROR_UNKNOWN_FS               (-101)

#define DFRG_ERROR_FS_DATA_CORRUPTED        (-102)

#define DFRG_ERROR_NOT_ENOUGH_SPACE         (-201)

#define DFRG_ERROR_JOB_IN_PROGRESS          (-202)


#define DFRG_ERROR_ABORTED_MESS             L"Aborted by user"

#define DFRG_ERROR_UNKNOWN_FS_MESS          L"Unknown or unsupported filesystem"

#define DFRG_ERROR_FS_DATA_CORRUPTED_MESS   L"FS data corrupted"

#define DFRG_ERROR_NOT_ENOUGH_SPACE_MESS    L"Not enough space for current extent"

#define DFRG_ERROR_JOB_IN_PROGRESS_MESS     L"Job already in progress"

#define DFRG_ERROR_UNKNOWN_MESS             L"Unknown error"


// ==============================================
//
//      API Inteface structures
//
// ==============================================

//
// Special Job Id's
//
#define     EMPTY_JOB_ID        (0)
#define     INVALID_JOB_ID      (-1L)
#define     GLOBAL_JOB_ID       (-2L)

//
// Job types
//
#define DEFRAG_JOB_ANALIZE_ONLY         0
#define DEFRAG_JOB_SIMPLE_DEFRAG        1
#define DEFRAG_JOB_FREE_SPACE           2
#define DEFRAG_JOB_SMART_BY_NAME        3
#define DEFRAG_JOB_SMART_BY_ACCESS      4
#define DEFRAG_JOB_SMART_BY_MODIFY      5
#define DEFRAG_JOB_SMART_BY_SIZE        6
#define DEFRAG_JOB_SMART_BY_CREATE      7
#define DEFRAG_JOB_FORCE_TOGETHER       8
#define DEFRAG_JOB_MAX                  DEFRAG_JOB_FORCE_TOGETHER

#define DEFRAG_JOB_DEBUG1              (DEFRAG_JOB_MAX+1)

#define DEFRAG_JOB_EMPTY               -1 // JOB_CANCELLED


//
// Direct Job Control commands
//
#define DEFRAG_CMD_GET_JOB_STATUS       1
#define DEFRAG_CMD_INIT_JOB             2
#define DEFRAG_CMD_PAUSE_JOB            3
#define DEFRAG_CMD_RESUME_JOB           4
#define DEFRAG_CMD_STOP_JOB             5
#define DEFRAG_CMD_DELETE_JOB           6
#define DEFRAG_CMD_SET_JOB_OPTIONS      7
#define DEFRAG_CMD_GET_MAP_AND_STATUS   8

#define DEFRAG_CMD_GET_JOB_LIST        21
#define DEFRAG_CMD_GET_JOB_OPTIONS     22

#define DEFRAG_CMD_DEBUG1               9

#define DEFRAG_CMD_ANALIZE_VOLUME      10
#define DEFRAG_CMD_FREE_SPACE          11
#define DEFRAG_CMD_SIMPLE_DEFRAG       12
#define DEFRAG_CMD_SMART_BY_NAME       13
#define DEFRAG_CMD_SMART_BY_ACCESS     14
#define DEFRAG_CMD_SMART_BY_MODIFY     15
#define DEFRAG_CMD_SMART_BY_SIZE       16
#define DEFRAG_CMD_SMART_BY_CREATE     17
#define DEFRAG_CMD_FORCE_TOGETHER      18

//
// Report Info
//
#define DEFRAG_CMD_GET_STATISTIC       30
#define DEFRAG_CMD_GET_CLUSTER_FILES   31
#define DEFRAG_CMD_GET_UNMOV_FILES     32
#define DEFRAG_CMD_GET_SYSTEM_FILES    33
#define DEFRAG_CMD_GET_DIR_FILES       34
#define DEFRAG_CMD_GET_SPEED           35
#define DEFRAG_CMD_GET_MOST_FRAGMENTED 36
#define DEFRAG_CMD_GET_SKIPPED         37
#define DEFRAG_CMD_GET_FILE_INFO       38

//
// Settings
//
#define DEFRAG_CMD_GET_SETTING         50
#define DEFRAG_CMD_SET_SETTING         51



//
// Job Options 
//
#define DEFRAG_JOB_OPTION_CHECK_EXCLUDE     0x00000001
#define DEFRAG_JOB_OPTION_SHOW_PROGRESS     0x00000002
#define DEFRAG_JOB_OPTION_BE_AGRESSIVE      0x00000004

#define DEFRAG_JOB_OPTION_BOOT_DEFRAG       0x00010000


//
// Job State Flags
//
#define DEFRAG_JOB_FLAG_INITIALIZED         0x00000001
#define DEFRAG_JOB_FLAG_ANALIZED            0x00000002
#define DEFRAG_JOB_FLAG_PROCESSED           0x00000004

#define DEFRAG_JOB_FLAG_ERROR               0x00010000

#define DEFRAG_JOB_FLAG_PAUSED              0x01000000
#define DEFRAG_JOB_FLAG_RESUMED             0x02000000
#define DEFRAG_JOB_FLAG_INPROGRESS          0x04000000

#define DEFRAG_JOB_FLAG_STOPPED             0x40000000
#define DEFRAG_JOB_FLAG_DELETED             0x80000000


// ==============================================
//
//          Analisys Structures definitions
//
// ==============================================

//
// Disk Map states
//
typedef enum {
    BlockEmpty = 0,
    BlockProcessed,
    BlockBusy,
    BlockFragmented,
    BlockCompressed,
    BlockFolder,
    BlockSystem,
    BlockLocked,
    BlockPaging,
    BlockMft,
    BlockInternal
} BlockState;

//
// Disk Map block state counts
//
typedef USHORT DFRG_COUNT_TYPE;

//
// Files modified before this 
//
#define     DFRG_RECENTLY_MODIFIED_VALUE       0xB0051C88000 // 2  weeks in 100 ns intervals ( 7*2 * 24*60*60 * 10**7) =  12096000000000
#define     DFRG_RARELY_MODIFIED_VALUE        0x8F04272E8000 // 26 weeks in 100 ns intervals ( 7*2 * 24*60*60 * 10**7) = 157248000000000

#define     DFRG_MAP_BLOCKS_MAX_NUM         4000


#pragma pack( push, 1 )

//
// This structure containes counts of fragments (extents) of 
// various types in disk view map block.
// If file's extent belong to several types (i.e. file is system, directory & fragmented)
// all appropriate counts are incremented.
// TODO: FreeSpace, BeingProcessed ???
//
typedef struct _DFRG_MAP_BLOCK_INFO
{
    DFRG_COUNT_TYPE     FragmentedFiles;
    DFRG_COUNT_TYPE     Directories;
    DFRG_COUNT_TYPE     Mft;
    DFRG_COUNT_TYPE     Metadata;
    DFRG_COUNT_TYPE     MftZone;
    DFRG_COUNT_TYPE     PageFile;
    DFRG_COUNT_TYPE     HiberFile;
    DFRG_COUNT_TYPE     FreeSpace;
    DFRG_COUNT_TYPE     LockedFiles;
    DFRG_COUNT_TYPE     BeingProcessed;
    DFRG_COUNT_TYPE     CompressedFiles;
    DFRG_COUNT_TYPE     Allocated;
    DFRG_COUNT_TYPE     LeastModifiedFiles;
    DFRG_COUNT_TYPE     RecentlyModifiedFiles;
} DFRG_MAP_BLOCK_INFO, *PDFRG_MAP_BLOCK_INFO;


typedef float DEFRAG_RATIO_TYPE;


typedef struct _DEFRAG_VOLUME_STATISTIC 
{
    //
    // Statistic info (while and after analize)
    //
    LARGE_INTEGER               FilesNum;
    LARGE_INTEGER               FilesSize;

    LARGE_INTEGER               DirsNum;
    LARGE_INTEGER               DirsSize;

    LARGE_INTEGER               RarelyModifiedNum;
    LARGE_INTEGER               RarelyModifiedSize;

    LARGE_INTEGER               OccasionalyModifiedNum;
    LARGE_INTEGER               OccasionalyModifiedSize;

    LARGE_INTEGER               RecentlyModifiedNum;
    LARGE_INTEGER               RecentlyModifiedSize;

    LARGE_INTEGER               TextFilesNum;
    LARGE_INTEGER               TextFilesSize;

    LARGE_INTEGER               GraphicFilesNum;
    LARGE_INTEGER               GraphicFilesSize;

    LARGE_INTEGER               ProgramFilesNum;
    LARGE_INTEGER               ProgramFilesSize;

    LARGE_INTEGER               VideoFilesNum;
    LARGE_INTEGER               VideoFilesSize;

    LARGE_INTEGER               MusicFilesNum;
    LARGE_INTEGER               MusicFilesSize;

    LARGE_INTEGER               TmpFilesNum;
    LARGE_INTEGER               TmpFilesSize;

    //
    // Defragmentation statistic info (while and after defragment)
    //
    LARGE_INTEGER               UnmovableFilesNum;
    LARGE_INTEGER               UnmovableFilesSize;

    LARGE_INTEGER               DefragmentedFilesNum;
    LARGE_INTEGER               DefragmentedFilesSize;

    //
    // Files fragmentation 
    //
    LARGE_INTEGER               FragmentedFilesNum;
    LARGE_INTEGER               FragmentedFilesSize;
    LARGE_INTEGER               FragmentedFilesExcessNum;
    DEFRAG_RATIO_TYPE           FragmentedFilesFragmentationRatio;

    //
    // Boot Files fragmentation 
    //
    LARGE_INTEGER               BootFilesNum;
    LARGE_INTEGER               BootFilesSize;
    LARGE_INTEGER               BootFilesExcessNum;
    DEFRAG_RATIO_TYPE           BootFilesFragmentationRatio;

    //
    // Page file fragmentation 
    //
    LARGE_INTEGER               PagefileFilesNum;
    LARGE_INTEGER               PagefileFilesSize;
    LARGE_INTEGER               PagefileFilesExcessNum;
    DEFRAG_RATIO_TYPE           PagefileFilesFragmentationRatio;

    //
    // Mft fragmentation 
    //
    LARGE_INTEGER               MftNum;
    LARGE_INTEGER               MftSize;
    LARGE_INTEGER               MftExcessNum;
    DEFRAG_RATIO_TYPE           MftFragmentationRatio;

    //
    // Metadata fragmentation 
    //
    LARGE_INTEGER               MetadataFilesNum;
    LARGE_INTEGER               MetadataFilesSize;
    LARGE_INTEGER               MetadataFilesExcessNum;
    DEFRAG_RATIO_TYPE           MetadataFilesFragmentationRatio;

    //
    // Free space fragmentation 
    //
    LARGE_INTEGER               GapsNum;
    LARGE_INTEGER               FreeSpaceSize;
    LARGE_INTEGER               FreeSpaceInsideMftSize;
    LARGE_INTEGER               MaxFreeSpaceGap;
    DEFRAG_RATIO_TYPE           FreeSpaceFragmentationRatio;

    //
    // Current info
    //
    WCHAR                       CurrentFileName[MAX_PATH+1];

} DEFRAG_VOLUME_STATISTIC, *PDEFRAG_VOLUME_STATISTIC;


typedef struct _DFRG_VOLUME_INFO
{
    //
    // NTFS_VOLUME_DATA_BUFFER. Should be the first block in this structure.
    //
    LARGE_INTEGER       VolumeSerialNumber;
    LARGE_INTEGER       NumberSectors;
    LARGE_INTEGER       TotalClusters;
    LARGE_INTEGER       FreeClusters;
    LARGE_INTEGER       TotalReserved;
    DWORD               BytesPerSector;
    DWORD               BytesPerCluster;
    DWORD               BytesPerFileRecordSegment;
    DWORD               ClustersPerFileRecordSegment;
    LARGE_INTEGER       MftValidDataLength;
    LARGE_INTEGER       MftStartLcn;
    LARGE_INTEGER       Mft2StartLcn;
    LARGE_INTEGER       MftZoneStart;
    LARGE_INTEGER       MftZoneEnd;

    //
    // Volume files statistic
    //
    DEFRAG_VOLUME_STATISTIC     Statistic;

} DFRG_VOLUME_INFO, *PDFRG_VOLUME_INFO;

// ==============================================
//
//          Interactive Jobs
//
// ==============================================

typedef struct _DEFRAG_JOB_OPTIONS 
{
    DWORD                   Id;
    WCHAR                   VolumeName[MAX_PATH];
    DWORD                   CurrentJobType;
} DEFRAG_JOB_OPTIONS, *PDEFRAG_JOB_OPTIONS;

// ==============================================
//
//          Cluster Inspector
//
// ==============================================

typedef struct _DEFRAG_CINSPECTOR_INFO
{
    ULONGLONG   StartCluster;       // LCN
    ULONGLONG   NumberOfClusters;   // number of clusters from start cluster
    ULONGLONG   VCN;                // 
    WCHAR       FileName[MAX_PATH];
} DEFRAG_CINSPECTOR_INFO, *PDEFRAG_CINSPECTOR_INFO;

// ==============================================
//
//          File Info 
//
// ==============================================

typedef struct _DEFRAG_EXTENT_INFO
{
    LARGE_INTEGER               StartLcn;
    LARGE_INTEGER               Length;
    LARGE_INTEGER               StartVcn;
} DEFRAG_EXTENT_INFO, *PDEFRAG_EXTENT_INFO;

typedef struct _DEFRAG_STREAM_INFO
{
    WCHAR                       StreamName[MAX_PATH];
    ULONG                       StreamType;             // StreamCtx->MftStreamType
    //WCHAR                       StreamTypeName[30];   // NTFS_STREAM_ATTRIBUTE_NAME[StreamType];

    ULONG                       ExtentsNum;
    DEFRAG_EXTENT_INFO          Extent[1];
} DEFRAG_STREAM_INFO, *PDEFRAG_STREAM_INFO;

//
// DefragFlags field values
//
#define DEFRAG_FILE_EXCLUDED            0x00000001
#define DEFRAG_FILE_UNMOVABLE           0x00000002
#define DEFRAG_FILE_LOCKED              0x00000004
#define DEFRAG_FILE_ROOT_DIR            0x00000008

#define DEFRAG_FILE_META                0x00000010
#define DEFRAG_FILE_HIBER               0x00000020
#define DEFRAG_FILE_MFT                 0x00000040
#define DEFRAG_FILE_PAGING              0x00000080

#define DEFRAG_FILE_DIRECTORY           0x00000100
#define DEFRAG_FILE_SYSTEM              0x00000200
#define DEFRAG_FILE_READONLY            0x00000400
#define DEFRAG_FILE_TEMPORARY           0x00000800

#define DEFRAG_FILE_ENCRYPTED           0x00001000
#define DEFRAG_FILE_COMPRESSED          0x00002000
#define DEFRAG_FILE_SPARSED             0x00004000
#define DEFRAG_FILE_OFFLINE             0x00008000

#define DEFRAG_FILE_FRAGMENTED          0x00010000
#define DEFRAG_FILE_STREAMED            0x00020000

#define DEFRAG_FILE_BOOT                0x00100000

#define DEFRAG_FILE_RECENTLY_MODIFIED   0x01000000
#define DEFRAG_FILE_RARELY_MODIFIED     0x02000000
#define DEFRAG_FILE_HOT                 0x04000000 // Created after Job start

#define DEFRAG_FILE_PROCESSED           0x10000000
#define DEFRAG_FILE_MOVEABILITY_CHECKED 0x20000000


#define DEFRAG_FILE2_TEXT_DOC           0x00000001
#define DEFRAG_FILE2_EXEC               0x00000002
#define DEFRAG_FILE2_GRAPHIC            0x00000004
#define DEFRAG_FILE2_VIDEO              0x00000008

#define DEFRAG_FILE2_MUSIC              0x00000010

#define DEFRAG_FILE2_TYPE_MASK          0x000000FF

#define DEFRAG_FILE2_SHORT_NAME         0x00010000
#define DEFRAG_FILE2_LONG_NAME          0x00020000
#define DEFRAG_FILE2_FULL_NAME          0x00040000


typedef struct _DEFRAG_FILE_INFO
{
    WCHAR                       FileName[MAX_PATH];
    ULONGLONG                   FileId;

    LARGE_INTEGER               CreationTime;
    LARGE_INTEGER               LastAccessTime;
    LARGE_INTEGER               ChangeTime;
    //LARGE_INTEGER               LastWriteTime;
    ULONG                       Attributes;

    ULONG                       DefragFlags;    // DEFRAG_FILE_*
    ULONG                       DefragFlags2;   // DEFRAG_FILE2_*
    ULONG                       StreamsNum;
    DEFRAG_STREAM_INFO          Stream[1];
} DEFRAG_FILE_INFO, *PDEFRAG_FILE_INFO;


#define DEFRAG_MAX_FILES_IN_LIST 100 

typedef struct _DEFRAG_FILE_LIST
{
    WCHAR                       FileName[MAX_PATH];
    ULONG                       DefragFlags;    // DEFRAG_FILE_*
    ULONG                       DefragFlags2;   // DEFRAG_FILE2_*
    ULONGLONG                   ExtentsNum;
} DEFRAG_FILE_LIST, *PDEFRAG_FILE_LIST;



#pragma pack( pop )

// ==============================================
//
//          Settings definitions
//
// ==============================================

//
// Default LogFile path
//
#define     DfrgLogFilePathDef   L"C:\\adefrag_log.txt"

//
// Default Max File size in clusters
//
#define     DfrgMaxFileSizeDef  0x10000000

//
// Defragmentation CPU priority
//
#define     DfgCpuPriorityNormal    0
#define     DfgCpuPriorityIdle      1
#define     DfgCpuPriorityTurbo     2

//
// Defragmentation VSS mode
//
#define     DfgVSSModeStop          0
#define     DfgVSSModeCompatible    1
#define     DfgVSSModeNormal        2

//
// Settings
//
#define SETTING(Name, Type, Context, SetRoutine, DefaultValue)                \
    Name##Id,

//
// Enum of Settings identifiers
//
typedef enum _SETTING_ID
{
    #include "SettingsTable.h"
    SettingMaxId
} SETTING_ID, *PSETTING_ID;

//
// Settings Operations enumerator 
//
typedef enum _OP_ID
{
    OP_SET,             // set setting
    OP_ADD = OP_SET,    // add entry to list
    OP_REMOVE,          // remove entry from list
    OP_GET,             // get setting
    OP_MAX_ID,
    OP_REMOVE_ALL       // pseudo operation - remove ALL entries from list
} OP_ID, *POP_ID;

//
// Logging settings
//
typedef enum {
    DfrgLoggingDepthNo = 0,
    DfrgLoggingDepthError,
    DfrgLoggingDepthDef = DfrgLoggingDepthError,
    DfrgLoggingDepthWarning,
    DfrgLoggingDepthInfo,
    DfrgLoggingDepthDebug,
    DfrgLoggingDepthMax = DfrgLoggingDepthDebug,
} DfrgLoggingDepthType;


// ==============================================
//
//          Sheduled Jobs definitions
//
// ==============================================

namespace Scheduler
{

typedef enum {
    e_PeriodOneTime = 0,
    e_PeriodDay,
    e_PeriodWeek,
    e_PeriodSceenSaver
} EPeriodType;

typedef enum {
    e_DrivesAll,
    e_DrivesSelected
} EDrivesType;

typedef enum {
    e_SequenceOneByOne,
    e_SequenceAtOneTime,
    e_SequenceAuto
} ESequenceType;

typedef enum {
    e_DefragSimple,
    e_DefragFreeSpace,
    e_DefragByName,
    e_DefragByAccess,
    e_DefragByModification,
    e_DefragByCreation,
    e_DefragBySize,
    e_DefragForce
} EScheduleDefragMethod;

typedef enum {
    e_DefragModeNormal,
    e_DefragModeSmart,
    e_DefragModeBoot
} EScheduleDefragMode;

typedef enum {
    e_RunStatusNotRunned,
    e_RunStatusInProgress,
    e_RunStatusCompleted,
    e_RunStatusSkipped,
    e_RunStatusCancelled,
    e_RunStatusDeferred,
    e_RunStatusPaused,
} EScheduleRunStatus;

typedef enum {
    e_WeekDayMonday,
    e_WeekDayTuesday,
    e_WeekDayWednesday,
    e_WeekDayThursday,
    e_WeekDayFriday,
    e_WeekDaySaturday,
    e_WeekDaySunday
} EScheduleWeekDay;

typedef struct _DEFRAG_JOB_CONTEXT      *PDEFRAG_JOB_CONTEXT;

}; // namespace Scheduler


#pragma pack( push, 1 )

typedef struct _DFRG_SHEDULED_JOB
{
    WCHAR                           Name[10];
    int                             Id; // -1 is possible
    BOOL                            ActiveStatus;
    Scheduler::EPeriodType          Period;
    // What to defrag
    Scheduler::EDrivesType          DrivesType;
    WCHAR                           DrivesList[40*4];
    Scheduler::ESequenceType        Sequence;
    // How to defrag
    Scheduler::EScheduleDefragMethod Method;
    Scheduler::EScheduleDefragMode  DefragMode;
    // When to defrag
    __time64_t                      StartTime;
    int                             DayPeriod;
    Scheduler::EScheduleWeekDay     WeekDay;
    BOOL                            EnableShutdown;

    // Status
    Scheduler::EScheduleRunStatus   Status;
    __time64_t                      LastRunTime;
    __time64_t                      NextRunTime;
    __time64_t                      ManualNextRunTime;

    //Scheduler::CContextHolder       JobContext;

    //
    // Min / Max File size in clusters
    //
    //LARGE_INTEGER                   MinFileSize;
    //LARGE_INTEGER                   MaxFileSize;

    //
    // Max File Extents number
    //
    //LARGE_INTEGER                   MaxFileExtent;
} DFRG_SHEDULED_JOB, *PDFRG_SHEDULED_JOB;

#pragma pack( pop )


// ==============================================
//
//      Communication structures
//
// ==============================================

//#pragma pack( push, 1 )


#define SERVER_SECTION_SIZE             0x20000

#define CLIENT_SECTION_SIZE             0x20000

#define PORT_DATA_LENGTH                MAX_PATH


//
// Connection establishe structure
//
typedef struct _PORT_CONNECTION_INFO {
    WCHAR       TextMessage[40];
    DWORD       Version;
} PORT_CONNECTION_INFO, *PPORT_CONNECTION_INFO;

//
// Settings get/set control data
//
typedef struct _SETTINGS_INFO {
    DWORD       SettingId;
    DWORD       SettingOp; // OP_ID
} SETTINGS_INFO, *PSETTINGS_INFO;

//
// Cluster Inspector control data
//
typedef struct _CLUSTERS_INFO {
    LONGLONG    StartCluster;
    LONGLONG    ClustersNumber;
} CLUSTERS_INFO, *PCLUSTERS_INFO;


typedef struct _PORT_MESSAGEX2 {

    //
    // PORT_MESSAGE struct
    //
    union {
        struct {
            USHORT      DataSize;
            USHORT      MessageSize;
        };
        ULONG Length;
    };

    union {
        struct {
            USHORT      MessageType;
            USHORT      VirtualRangesOffset;    // DataInfoOffset;
        };
        ULONG ZeroInit;
    };

    union {
        LPC_CLIENT_ID   ClientId;
        double          DoNotUseThisField;      // Force quadword alignment
    };

    ULONG               MessageId;

    //ULONG               SectionSize;
    union {
        LPC_SIZE_T      ClientViewSize;          // Only valid on LPC_CONNECTION_REQUEST message
        ULONG           CallbackId;              // Only valid on LPC_REQUEST message
    };

    //
    // Data
    //
    union {
        //
        // Raw (Debug) data, strings
        //
        UCHAR           RawData[PORT_DATA_LENGTH];
        WCHAR           WString[PORT_DATA_LENGTH/2];

        //
        // Connection Info
        //
        PORT_CONNECTION_INFO    ConnectionInfo;

        struct {

            IN  OUT DWORD       JobId;

            //
            // Input / Request
            //
            IN      DWORD       Command; // DEFRAG_CMD_*
            IN      DWORD       InputDataLength;

            //
            // Light waight control data (IN)
            //
            union {
                WCHAR           DriveName[20];
                SETTINGS_INFO   Setting;
                CLUSTERS_INFO   ClustersBlock;
            };

            //
            // Output / Reply
            //
            OUT     int         Result;  // DFRG_ERROR_*, ERROR_*
            OUT     DWORD       ServiceStatus;
            OUT     DWORD       JobStatus;
            OUT     DWORD       JobProgress;
            IN  OUT DWORD       OutputDataLength;
        };
    };
} PORT_MESSAGEX2, *PPORT_MESSAGEX2;

//#pragma pack( pop )

#endif // !__DFRG_API_H__

