/*
    Fat disk analysis module header

    Module name:

        FatUtils.h

    Abstract:

        Fat disk analysis module header.
        Defines constants, types, structures, functions, etc. 
        used for FAT 12/16/32/Ex disk analysis and defragmentation

    $Header: /home/CVS_DEFRAG/Defrag/Src/Engine/FatUtils.h,v 1.1 2009/11/24 15:15:10 dimas Exp $
    $Log: FatUtils.h,v $
    Revision 1.1  2009/11/24 15:15:10  dimas
    Bugs 5.2, 6.*, 15.14 fixed

*/


#ifndef __FAT_UTILS_H__
#define __FAT_UTILS_H__


// =================================================
//
// Constants, Macros & Data
//
// =================================================

//
// Disk/File System structures
// 

#pragma pack( push, 1 )

#ifndef FILE_ATTRIBUTE_VOLUME_ID
    #define FILE_ATTRIBUTE_VOLUME_ID    (0x08)
#endif

#define FILE_ATTRIBUTE_LONG_NAME        (FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_VOLUME_ID)

#define FILE_ATTRIBUTE_LONG_NAME_MASK   (FILE_ATTRIBUTE_LONG_NAME | FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_ARCHIVE)

#define LAST_LONG_ENTRY                 (0x40) 

typedef struct _DOS_FILE_TIME {
    unsigned    sec   : 5;      // 0..29 * 2 = 0, 2, 4, .. 58 seconds
    unsigned    min   : 6;      // 0..59
    unsigned    hour  : 5;      // 0.23
    //unsigned    msec2 : 11;
} DOS_FILE_TIME, *PDOS_FILE_TIME;

typedef struct _DOS_FILE_DATE {
    unsigned    day   : 5;      // 1..31
    unsigned    month : 4;      // 1..12
    unsigned    year  : 7;      // 0..127 + 1980
} DOS_FILE_DATE, *PDOS_FILE_DATE;

typedef struct _FAT_DIR_RECORD {
    BYTE        ShortName[8];   // padded with 0x20, ".ext" not allowed
                                // ShortName[0] == 0xE5 - entry is free 
                                // ShortName[0] == 0x00 - entry is free and no next exist
                                // ShortName[0] == 0x05 - the actual file name character for this byte is 0xE5 (KANJI)
    BYTE        Extention[3];   // padded with 0x20
    BYTE        Attributes;
    BYTE        NtReserved;
    BYTE        TimeTenth;      // tenths of a second, 0..199
    WORD        CreateTime;     // 0 - if not supported
    WORD        CreateDate;     // 0 - if not supported
    WORD        LastAccessDate; // 0 - if not supported
    WORD        FirstClusterHigh;// 0 for FAT12, FAT16
    WORD        WriteTime;
    WORD        WriteDate;
    WORD        FirstClusterLow; 
    DWORD       FileSize;       // 0 for Dirs (ATTR_DIRECTORY set).
                                // The maximum allowed file size on a FAT volume is 0xFFFFFFFF (4,294,967,295).
} FAT_DIR_RECORD, *PFAT_DIR_RECORD;

//
// A set of long entries is always associated with a short entry that they always immediately precede.  
//
typedef struct _FAT_LONG_DIR_RECORD {
    BYTE        Ordinal;    // 1 .. N | LAST_LONG_ENTRY (0x40), counts up from short entry
    WCHAR       Name1[5];
    BYTE        Attributes;
    BYTE        Type;
    BYTE        CheckSum; // CheckSum odf short dir entry followed the set of long
    WCHAR       Name2[6];
    WORD        FirstLusterLow; // = 0
    WCHAR       Name3[2];
} FAT_LONG_DIR_RECORD, *PFAT_LONG_DIR_RECORD;

#define FAT_LONGNAME_ENTRY_LENGTH              13   // Max longname char num in one entry

#pragma pack( pop )



// =================================================
//
// Function prototypes
//
// =================================================
/*
PWCHAR
GetStreamTypeName( 
    NTFS_STREAM_ATTRIBUTE_TYPE  MftStreamType );

int
NtfsCookMftRecord( 
    PDEFRAG_VOLUME_CONTEXT      VolumeCtx,
    PMFT_FILE_RECORD_HEADER     MftRecord );

int
NtfsExploreAttribute( 
    IN  PMFT_ATTRIBUTE          Attribute,
    IN  int                     ListDepth,
    OUT PDEFRAG_FILE_CONTEXT    FileCtx,
    OUT int                     *AttrListsNum );

int
NtfsParseAttributes( 
    IN  PBYTE                   AttributeBuffer,
    IN  DWORD                   BufferLength,
    IN  int                     ListDepth,
    OUT PDEFRAG_FILE_CONTEXT    FileCtx );

int
NtfsGetFileInfoFromMftRec( 
    PDEFRAG_VOLUME_CONTEXT  VolumeCtx,
    PMFT_FILE_RECORD_HEADER MftRecord,
    ULONGLONG               RecordIndex,
    PDEFRAG_FILE_CONTEXT    *OutFileCtx );

int 
NtfsGetClustersMapFromMft( 
    PMFT_NONRESIDENT_ATTRIBUTE  Attribute, 
    PDEFRAG_STREAM_CONTEXT      StreamCtx,
    PDEFRAG_VOLUME_CONTEXT      VolumeCtx );
*/
int
FatGetFileList( 
    PDEFRAG_VOLUME_CONTEXT  VolumeCtx );

#endif // !__FAT_UTILS_H__