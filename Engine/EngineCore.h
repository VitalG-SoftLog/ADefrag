/*
    Defrag Engine Core functions header

    Module name:

        EngineCore.h

    Abstract:

        Defrag Engine Core functions & structures header. 

    $Header: /home/CVS_DEFRAG/Defrag/Src/Engine/EngineCore.h,v 1.6 2009/12/24 10:52:20 dimas Exp $
    $Log: EngineCore.h,v $
    Revision 1.6  2009/12/24 10:52:20  dimas
    Check against Exclude Files list implemented

    Revision 1.5  2009/12/23 13:01:47  dimas
    1. Full File Name processing implemented.
    2. Top 100 Fragmented functionality for all file list requests implemented.

    Revision 1.4  2009/12/21 17:00:13  dimas
    1. DEFRAG_CMD_GET_MOST_FRAGMENTED and similar requests implemented
    2. Basic Exclude files check added

    Revision 1.3  2009/11/30 12:48:36  dimas
    1. Smart modes improved.
    2. FAT processing improved.

    Revision 1.2  2009/11/26 15:56:54  dimas
    1. Smart modes improved.
    2. File names support improved.

    Revision 1.1  2009/11/24 14:52:15  dimas
    no message

*/

#ifndef __DFRG_CORE_H__
#define __DFRG_CORE_H__


// =================================================
//
// Constants, Macros & Data
//
// =================================================

//
// Disk/File System structures
// 

#pragma pack( push, 1 )

//
// Common (Disk)
//

enum DISK_TYPE {
  UnknownDisk = 0,
  Fat12Disk   = 1, // 3
  Fat16Disk   = 4, // 4 
  Fat32Disk   = 16,// 8
  NtfsDisk    = 32,
};


// Partition

typedef struct _MBR_SECTOR {
    unsigned char    ActivFlag;     // 80h - active, 0 - not active
    unsigned char    StartHead;
    unsigned char    StartSector;   // ccccccccCcSsssss
    unsigned char    StartCylinder;
    unsigned char    Type;          // 01 - FAT12, 04 - FAT16, 05 - extended, 06 - gaint
    unsigned char    EndHead;
    unsigned char    EndSector;     // ccccccccCcSsssss
    unsigned char    EndCylinder;
    unsigned long    StartLogicalSector;
    unsigned long    TotalSectorsNum;
} MBR_SECTOR, *PMBR_SECTOR;

// Boot

#define NTFS_DRIVE_SIGN     0x202020205346544E // 'NTFS    '
#define FAT_DRIVE_SIGN      0x302E35534F44534D // 'MSDOS5.0'

#define FAT_BOOT_SIGN       0xAA55

typedef struct _BIOS_PARAMETERS_BLOCK {
    BYTE        Reserved[3];
} BIOS_PARAMETERS_BLOCK, *PBIOS_PARAMETERS_BLOCK;

typedef struct _EXTENDED_BIOS_PARAMETERS_BLOCK {
    BYTE        Reserved[3];
} EXTENDED_BIOS_PARAMETERS_BLOCK, *PEXTENDED_BIOS_PARAMETERS_BLOCK;

typedef struct _NTFS_BOOT_SECTOR {
    BYTE                            JmpCode[3];
    ULONGLONG                       Sign;
    BIOS_PARAMETERS_BLOCK           Bios;
    EXTENDED_BIOS_PARAMETERS_BLOCK  BiosEx;
} NTFS_BOOT_SECTOR, *PNTFS_BOOT_SECTOR;

typedef struct _DOS_BOOT_SECTOR {

    BYTE                JmpCode[3];
    ULONGLONG           Sign;
    WORD                BytesPerSector;
    BYTE                SectorsPerCluster;
    WORD                ReservedSectorsNum;
    BYTE                FatNum;
    WORD                RootEntryCnt;
    WORD                TotalSectorsNum;
    BYTE                Discriptor;
    WORD                SectorsPerFat;
    WORD                SectorsPerTrack;
    WORD                HeadsNum;
    DWORD               HiddenSectorsNum;
    DWORD               TotalSectorsNum32;

    union {
        struct {
            BYTE                DriveNum;
            BYTE                Reserved1;
            BYTE                BootFlag;
            DWORD               VolumeId;
            BYTE                VolumeLabel[11];
            BYTE                FileSysType[8];
            BYTE                Reserved2[448]; // Boot code
        } Fat16;
        struct {
            ULONG               SectorsPerFat32;
            USHORT              ExtFlags;
            USHORT              FSVersion;
            ULONG               RootCluster;
            USHORT              FSInfo;
            USHORT              BakBootSec;
            UCHAR               Reserved[12];

            BYTE                DriveNum;
            BYTE                Reserved1;
            BYTE                BootFlag;
            DWORD               VolumeId;
            BYTE                VolumeLabel[11];
            BYTE                FileSysType[8];
            UCHAR               Reserved2[420]; // Boot code
        } Fat32;
    };

    WORD                FatBootSign;   // 0xAA55

} DOS_BOOT_SECTOR, *PDOS_BOOT_SECTOR;

typedef struct _BOOT_SECTOR {
    union {
        BYTE                Buffer[1024];
        NTFS_BOOT_SECTOR    Ntfs;
        DOS_BOOT_SECTOR     Dos;
    };
} BOOT_SECTOR, *PBOOT_SECTOR;

#pragma pack( pop )



typedef struct _DOS_VOLUME_DATA_BUFFER {
    DWORD           FatOffset;
    DWORD           FatLength;
    DWORD           FatsNumber;
    DWORD           RootDirOffset;
    DWORD           RootDirLength;
    DWORD           DataOffset;
} DOS_VOLUME_DATA_BUFFER, *PDOS_VOLUME_DATA_BUFFER; 


//
// Buffers used while analyzing
//

#define     VIRTUAL_LCN   ((ULONGLONG)-1)

#define     BUFFER_ITEMS_NUM    4096

#define     BITMAP_ITEM_SIZE    (sizeof(BYTE))
#define     BITMAP_BUFFER_SIZE  (sizeof(VOLUME_BITMAP_BUFFER) + (BUFFER_ITEMS_NUM-1)*BITMAP_ITEM_SIZE)

#define     CLUSTER_MAP_ITEM_SIZE   (2*sizeof(LARGE_INTEGER))
#define     CLUSTER_MAP_BUFFER_SIZE (sizeof(RETRIEVAL_POINTERS_BUFFER) + (BUFFER_ITEMS_NUM-1)*CLUSTER_MAP_ITEM_SIZE)

#define     FILE_RECS_BUFFER_SIZE     (4096*64)


//
// Bitmap Gap structure
//
typedef struct _DEFRAG_LCN_EXTENT {
    LARGE_INTEGER               StartLcn;
    LARGE_INTEGER               Length;
} DEFRAG_LCN_EXTENT, *PDEFRAG_LCN_EXTENT;

//
// Files extent structure
//
typedef struct _DEFRAG_FILES_EXTENT {
    //
    // First two fields must be the same as DEFRAG_LCN_EXTENT structure
    //
    LARGE_INTEGER               StartLcn;
    LARGE_INTEGER               Length;

    LARGE_INTEGER               StartVcn;
    PDEFRAG_STREAM_CONTEXT      StreamCtx;
} DEFRAG_FILES_EXTENT, *PDEFRAG_FILES_EXTENT;

//
// ZoneFlags field values
//
#define DEFRAG_ZONE_PROCESSED       0x10000000

//
// Zone structure
//
typedef struct _DEFRAG_ZONE {
    ULONG                           Type;
    ULONG                           ZoneFlags;
    //RETRIEVAL_POINTERS_BUFFER       ClustersMap;
    DWORD                           ExtentCount;
    DEFRAG_LCN_EXTENT               Extents[1];
} DEFRAG_ZONE, *PDEFRAG_ZONE;

//
// Defrag Context structures
//
typedef struct _DEFRAG_VOLUME_CONTEXT {

    //
    // General info
    //
    WCHAR                       VName[MAX_PATH];
    ULONG                       DiskType;
    HANDLE                      VHandle;

    //
    // Used as common holdef for volume data
    //
    NTFS_VOLUME_DATA_BUFFER     NtfsData;

    //
    // Volume files statistic
    //
    DEFRAG_VOLUME_STATISTIC     Statistic;

    //
    // Bitmap and free space info
    //
    LARGE_INTEGER               BitMapLength;
    PBYTE                       BitMapBuffer;
    BTR_TREE                    GapsTree;

    //
    // NTFS Specific info
    //
    PBYTE                       FileRecordsBuffer;
    PBYTE                       ClustersMapBuffer;
    LARGE_INTEGER               ValidRecordsNum;
    struct _DEFRAG_FILE_CONTEXT *MftFileCtx;

    //
    // Fat Specific info
    //
    DOS_VOLUME_DATA_BUFFER      DosData;
    PBYTE                       FatBuffer;
    struct _DEFRAG_FILE_CONTEXT *RootDirCtx;

    //
    // Volume files info
    //
    BTR_TREE                    DirsTree;

    //
    // Volume files info
    //
    BTR_TREE                    FilesTree;

    //
    // Used extents info
    //
    BTR_TREE                    ExtentsTree;

    //
    // Upper context info
    //
    struct _DEFRAG_JOB_CONTEXT  *JobCtx;

} DEFRAG_VOLUME_CONTEXT, *PDEFRAG_VOLUME_CONTEXT;


typedef struct _DEFRAG_STREAM_CONTEXT {
    //
    // Stream info
    //
    NTFS_STREAM_ATTRIBUTE_TYPE      MftStreamType;
    //USHORT                          AttributeId;
    DWORD                           NameLength;
    WCHAR                           *StreamName;
    LARGE_INTEGER                   DataSize;
    PRETRIEVAL_POINTERS_BUFFER      ClustersMap;
    struct _DEFRAG_STREAM_CONTEXT   *NextStream;

    //
    // Upper context info
    //
    struct _DEFRAG_FILE_CONTEXT     *FileCtx;

} DEFRAG_STREAM_CONTEXT, *PDEFRAG_STREAM_CONTEXT;


typedef struct _DEFRAG_FILE_CONTEXT {
    //
    // File common info
    //
    HANDLE                      FHandle;
    FILE_ID                     FId;
    WCHAR                       *FName;
    DWORD                       DefragFlags;    // DEFRAG_FILE_*
    DWORD                       DefragFlags2;   // DEFRAG_FILE2_*

    //
    // File attributes info
    //
    DWORD                       Attributes;
    LARGE_INTEGER               CreationTime;
    LARGE_INTEGER               ChangeTime;
    LARGE_INTEGER               LastAccessTime;
    LARGE_INTEGER               TotalClustersNum;
    LARGE_INTEGER               TotalFragmentsNum;

    //
    // Directory links
    //
    FILE_ID                     ParentDirId;
    struct _DEFRAG_FILE_CONTEXT *ParentDir;

    //
    // Streams info
    //
    //DWORD                       StreamNum;
    HANDLE                      StreamHandle;
    PDEFRAG_STREAM_CONTEXT      DefaultStream;
    PDEFRAG_STREAM_CONTEXT      FirstStream;

    //
    // Upper context info
    //
    PDEFRAG_VOLUME_CONTEXT      VolumeCtx;

} DEFRAG_FILE_CONTEXT, *PDEFRAG_FILE_CONTEXT;


//
// Min/Max file size (in clusters)
//
#define     FILE_MIN_SIZE                 1 // 1 cluster = 0x1000  // 4Kb
#define     FILE_MAX_SIZE          0x100000 // 0x100000000  // 4Gb
#define     FILE_MAX_FRAGMENT        0x4000 // 0x4000000  //64Mb

typedef struct _DEFRAG_JOB_CONTEXT {
    //
    // General info
    //
    DWORD                   Id;
    DWORD                   Type;       // DEFRAG_JOB_*
    DWORD                   Options;    // DEFRAG_JOB_OPTION_*

    DWORD                   Status;     // DEFRAG_JOB_FLAG_*
    int                     Result;

    //
    // Actual Settings 
    //
    PDFRG_SETTINGS          Settings;

    //LARGE_INTEGER           MinFileSize;
    //LARGE_INTEGER           MaxFileSize;
    //LARGE_INTEGER           MaxFileExtent;

    BTR_TREE_SORT_MODE      SortMode;

    //
    // Progress info
    //
    __time64_t              StartTime;
    LARGE_INTEGER           ObjectsToProcessNum;
    LARGE_INTEGER           ProcessedObjectsNum;

    //
    // Display info
    //
    struct _DFRG_MAP_BLOCK_INFO    *DisplayMapEx;
    DWORD                           DisplayMapExSize;
    DWORD                           ClastersInBlock;
    //DEFRAG_FILE_CONTEXT             CurrentFile;
    UNICODE_STRING                  CurrentFileName;

    //
    // Internal structures
    //
    HANDLE                  ResumeEvent;
    CRITICAL_SECTION        JobSync;
    HANDLE                  JobThread;

    DEFRAG_VOLUME_CONTEXT   VolumeCtx;

    //
    // Jobs are joined in list
    //
    struct _DEFRAG_JOB_CONTEXT  *PrevJob;
    struct _DEFRAG_JOB_CONTEXT  *NextJob;

} DEFRAG_JOB_CONTEXT, *PDEFRAG_JOB_CONTEXT;

// =================================================
//
// Function prototypes
//
// =================================================

BOOLEAN
ShouldContinue( 
    IN  PDEFRAG_JOB_CONTEXT     JobCtx );
/*
BOOLEAN
IsFileExcluded( 
    PDEFRAG_JOB_CONTEXT         JobCtx, 
    PDEFRAG_FILE_CONTEXT        FileCtx );

void
DfrgResolveFileParent( 
    PDEFRAG_VOLUME_CONTEXT      VolumeCtx,
    PDEFRAG_FILE_CONTEXT        FileCtx );

int
DfrgResolveFileTree( 
    PDEFRAG_VOLUME_CONTEXT      VolumeCtx );

int
DfrgBuildFileName( 
    PDEFRAG_JOB_CONTEXT         JobCtx, 
    PDEFRAG_FILE_CONTEXT        FileCtx,
    PUNICODE_STRING             UniName );

int
DfrgGetFileName(
    PDEFRAG_JOB_CONTEXT         JobCtx, 
    PDEFRAG_FILE_CONTEXT        FileCtx,
    PUNICODE_STRING             UniName,
    PULONG                      Flags = NULL );
*/
BOOLEAN
DfrgMarkupFile( 
    PDEFRAG_JOB_CONTEXT         JobCtx, 
    PDEFRAG_FILE_CONTEXT        FileCtx );

int
DfrgInitVolumeCtx( 
    PDEFRAG_VOLUME_CONTEXT      VolumeCtx );

void
DfrgReleaseVolumeCtx( 
    IN  PDEFRAG_VOLUME_CONTEXT  VolumeCtx );
int
DfrgGetFileList( 
    PDEFRAG_VOLUME_CONTEXT      VolumeCtx );

BOOLEAN
DfrgAddFileToLists( 
    PDEFRAG_VOLUME_CONTEXT      VolumeCtx,
    PDEFRAG_FILE_CONTEXT        FileCtx );

int
DfrgAnalyzeVolume( 
    IN  PDEFRAG_VOLUME_CONTEXT  VolumeCtx );

int
DfrgProcessVolume(
    IN  PDEFRAG_VOLUME_CONTEXT  VolumeCtx );


#endif // !__DFRG_CORE_H__