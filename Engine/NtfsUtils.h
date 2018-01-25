/*
    Ntfs disk analysis  module header

    Module name:

        NtfsUtils.h

    Abstract:

        Ntfs disk analysis  module header.
        Defines constants, types, structures, functions, etc. 
        used for NTFS disk analysis 

    $Header: /home/CVS_DEFRAG/Defrag/Src/Engine/NtfsUtils.h,v 1.1 2009/11/24 14:52:15 dimas Exp $
    $Log: NtfsUtils.h,v $
    Revision 1.1  2009/11/24 14:52:15  dimas
    no message

*/


#ifndef __NTFS_UTILS__
#define __NTFS_UTILS__


// =================================================
//
// Constants, Macros & Data
//
// =================================================

//
// Disk/File System structures
// 

#pragma pack( push, 1 )


typedef enum _NTFS_STREAM_ATTRIBUTE_TYPE {
    MftAttrUninit                 = 0x00,
    MftAttrStandardInformation    = 0x10,
    MftAttrList                   = 0x20,
    MftAttrFileName               = 0x30,
    //MftAttrVolumeVersion          = 0x40, // NT 
    MftAttrObjectId               = 0x40,   // 2K
    MftAttrSecurityDescriptor     = 0x50,
    MftAttrVolumeName             = 0x60,
    MftAttrVolumeInformation      = 0x70,
    MftAttrData                   = 0x80,
    MftAttrIndexRoot              = 0x90,
    MftAttrIndexAllocation        = 0xA0,
    MftAttrBitmap                 = 0xB0,
    //MftAttrSymbolicLink           = 0xC0, // NT 
    MftAttrReparsePoint           = 0xC0,   // 2K 
    MftAttrEAInformation          = 0xD0,
    MftAttrEA                     = 0xE0,
    MftAttrPropertySet            = 0xF0,   // NT
    MftAttrLoggedUtilityStream    = 0x100,  // 2K
    MftAttrRecordEnd              = 0xFFFFFFFF
} NTFS_STREAM_ATTRIBUTE_TYPE, *PNTFS_STREAM_ATTRIBUTE_TYPE;

extern  WCHAR       *NTFS_STREAM_ATTRIBUTE_NAME[];


typedef struct _MFT_INODE_REFERENCE {
    union
    {
        struct
        {
            ULONG       MftIndexLowPart;
            USHORT      MftIndexHighPart;
            USHORT      SequenceNumber;
        };
        ULONGLONG       FileId;
    };
} MFT_INODE_REFERENCE, FILE_ID;

typedef union _RUN_EXTENT_ITEM
{
    ULONGLONG       QuadPart;
    BYTE            Byte[8];
} RUN_EXTENT_ITEM, *PRUN_EXTENT_ITEM;


typedef struct _MFT_RECORD_HEADER {
    ULONG       Type;
    USHORT      UsaOffset;
    USHORT      UsaCount;
    USN         Lsn;        // $LogFile Sequence Number
} MFT_RECORD_HEADER, *PMFT_RECORD_HEADER;

typedef struct _MFT_FILE_RECORD_HEADER {
    MFT_RECORD_HEADER       MftHeader;
    USHORT                  SequenceNumber;
    USHORT                  HardLinksCount;
    USHORT                  AttributeOffset;
    USHORT                  Flags;              // Flags. bit 1 = in use, bit 2 = directory
    ULONG                   BytesInUse;
    ULONG                   BytesAllocated;
    MFT_INODE_REFERENCE     BaseFileRecord;
    USHORT                  NextAttributeId;
    // TODO: check available doc for next fields
    USHORT                  Reserved;           // (XP/NTFS5)
    ULONG                   MFTRecordIndexber;    // Number of this MFT Record (XP) 
    USHORT                  UpdateSeqNum;
} MFT_FILE_RECORD_HEADER, *PMFT_FILE_RECORD_HEADER;


#define       MFT_ATTR_COMPRESSED     0x0001 
#define       MFT_ATTR_ENCRYPTED      0x0004 
#define       MFT_ATTR_SPARSED        0x0008 

typedef struct _MFT_ATTRIBUTE {
    NTFS_STREAM_ATTRIBUTE_TYPE  AttributeType;
    ULONG                   Length;
    BOOLEAN                 Nonresident;
    UCHAR                   NameLength;
    USHORT                  NameOffset;
    USHORT                  Flags;          // 0x0001 = Compressed, 0x4000 = Encrypted, 0x8000 = Sparse
    USHORT                  AttributeId;
} MFT_ATTRIBUTE, *PMFT_ATTRIBUTE;

typedef struct _MFT_RESIDENT_ATTRIBUTE {
    MFT_ATTRIBUTE   Attribute;
    ULONG           ValueLength;
    USHORT          ValueOffset;
    USHORT          Flags;  // 0x0001 = Indexed
} MFT_RESIDENT_ATTRIBUTE, *PMFT_RESIDENT_ATTRIBUTE;

typedef struct _MFT_NONRESIDENT_ATTRIBUTE {
    MFT_ATTRIBUTE   Attribute;
    ULONGLONG       StartingVcn;
    ULONGLONG       LastVcn;
    USHORT          RunArrayOffset;
    UCHAR           CompressionUnit;
    UCHAR           Reserved[5];
    ULONGLONG       AllocatedSize;
    ULONGLONG       DataSize;
    ULONGLONG       InitializedSize;
    ULONGLONG       CompressedSize;
} MFT_NONRESIDENT_ATTRIBUTE, *PMFT_NONRESIDENT_ATTRIBUTE;

typedef struct _MFT_ATTRIBUTE_LIST {
    NTFS_STREAM_ATTRIBUTE_TYPE AttributeType;
    USHORT          Length;
    UCHAR           NameLength;
    UCHAR           NameOffset;
    ULONGLONG       StartingVcn;
    MFT_INODE_REFERENCE AttributeInodeReference;
    USHORT          Instance;
    //USHORT          Reserved[3];
} MFT_ATTRIBUTE_LIST, *PMFT_ATTRIBUTE_LIST;

typedef struct _MFT_STANDARD_INFORMATION {
    ULONG64         CreationTime;
    ULONG64         AttrChangeTime; // FileChangeTime;
    ULONG64         LastWriteTime;  // MftChangeTime;
    ULONG64         LastAccessTime;
    ULONG           FileAttributes;
    ULONG           MaximumVersions;
    ULONG           VersionNumber;
    ULONG           ClassId;
    ULONG           OwnerId;        // NTFS 3.0 only
    ULONG           SecurityId;     // NTFS 3.0 only
    ULONGLONG       QuotaCharge;    // NTFS 3.0 only
    USN             Usn;            // NTFS 3.0 only
} MFT_STANDARD_INFORMATION, *PMFT_STANDARD_INFORMATION;

#define     NO_NAME     0x00
#define     LONG_NAME   0x01
#define     SHORT_NAME  0x02

typedef struct _MFT_FILENAME_ATTRIBUTE {
    MFT_INODE_REFERENCE     ParentDirectory;
    ULONG64                 CreationTime;       //
    ULONG64                 AttrChangeTime;     // this 4 field only changed when file name changes
    ULONG64                 LastWriteTime;      // and may differ from the same fields in MFT_STANDARD_INFORMATION struct
    ULONG64                 LastAccessTime;     //
    ULONGLONG               AllocatedSize;
    ULONGLONG               DataSize;
    ULONG                   FileAttributes;
    ULONG                   Reserved;
    UCHAR                   NameLength;
    UCHAR                   NameType;  // NTFS=0x01 (Long), DOS=0x02 (Short)
    WCHAR                   Name[1];
} MFT_FILENAME_ATTRIBUTE, *PMFT_FILENAME_ATTRIBUTE;

typedef struct _MFT_OBJECTID_ATTRIBUTE {
    GUID ObjectId;
    GUID BirthVolumeId;
    GUID BirthObjectId;
    GUID DomainId;
} MFT_OBJECTID_ATTRIBUTE, *PMFT_OBJECTID_ATTRIBUTE;

typedef struct _MFT_VOLUME_INFORMATION {
    LONGLONG    Reserved;
    UCHAR       MajorVersion;
    UCHAR       MinorVersion;
    USHORT      Flags;  // DIRTY=0x01, RESIZE_LOG_FILE=0x02
} MFT_VOLUME_INFORMATION, *PMFT_VOLUME_INFORMATION;

typedef struct _MFT_DIRECTORY_INDEX {
    ULONG       EntriesOffset;
    ULONG       IndexBlockLength;
    ULONG       AllocatedSize;
    ULONG       Flags;  // SMALL=0x00, LARGE=0x01
} MFT_DIRECTORY_INDEX, *PMFT_DIRECTORY_INDEX;

typedef struct _MFT_DIRECTORY_ENTRY {
    ULONGLONG   FileReferenceNumber;
    USHORT      Length;
    USHORT      AttributeLength;
    ULONG       Flags; // 0x01 = Has trailing VCN, 0x02 = Last entry
    //MFT_FILENAME_ATTRIBUTE Name;
    //ULONGLONG   Vcn;   // VCN in IndexAllocation of previous entries
} MFT_DIRECTORY_ENTRY, *PMFT_DIRECTORY_ENTRY;

typedef struct _MFT_INDEX_ROOT {
    NTFS_STREAM_ATTRIBUTE_TYPE  Type;
    ULONG                       CollationRule;
    ULONG                       BytesPerIndexBlock;
    ULONG                       ClustersPerIndexBlock;
    MFT_DIRECTORY_INDEX         DirectoryIndex;
} MFT_INDEX_ROOT, *PMFT_INDEX_ROOT;

typedef struct _MFT_INDEX_BLOCK_HEADER {
    MFT_RECORD_HEADER   Ntfs;
    ULONGLONG           IndexBlockVcn;
    MFT_DIRECTORY_INDEX DirectoryIndex;
} MFT_INDEX_BLOCK_HEADER, *PMFT_INDEX_BLOCK_HEADER;

typedef struct _MFT_REPARSE_POINT {
    ULONG       ReparseTag;
    USHORT      ReparseDataLength;
    USHORT      Reserved;
    UCHAR       ReparseData[1];
} MFT_REPARSE_POINT, *PMFT_REPARSE_POINT;

typedef struct _MFT_EA_INFORMATION {
    ULONG       EaLength;
    ULONG       EaQueryLength;
} MFT_EA_INFORMATION, *PMFT_EA_INFORMATION;

typedef struct _MFT_EA_ATTRIBUTE {
    ULONG       NextEntryOffset;
    UCHAR       Flags;
    UCHAR       EaNameLength;
    USHORT      EaValueLength;
    CHAR        EaName[1];
    //UCHAR       EaData[0];
} MFT_EA_ATTRIBUTE, *PMFT_EA_ATTRIBUTE;

struct _MFT_ATTRIBUTE_TYPE {
    WCHAR       AttributeName[64];
    ULONG       AttributeNumber;
    ULONG       Reserved[2];
    ULONG       Flags;
    ULONGLONG   MinimumSize;
    ULONGLONG   MaximumSize;
} MFT_ATTRIBUTE_TYPE, *PMFT_ATTRIBUTE_TYPE;


#pragma pack( pop )



// =================================================
//
// Function prototypes
//
// =================================================

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

int
NtfsGetFileList( 
    PDEFRAG_VOLUME_CONTEXT  VolumeCtx );

#endif // !__NTFS_UTILS__