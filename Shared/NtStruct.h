// =================================================
//
// NtStruct.h : Defines required undocumented structures, functions, etc.
//
// =================================================

#ifndef __NT_STRUCT_H__
#define __NT_STRUCT_H__


#ifdef __cplusplus
extern "C" {
#endif


// =================================================
//
// Common definitions
//
// =================================================

#ifndef STATUS_SUCCESS
    // definitions from ntstat.h
    #define STATUS_SUCCESS                  0
    #define STATUS_UNSUCCESSFUL             ((NTSTATUS)0xC0000001L)
    #define STATUS_INVALID_PARAMETER        ((NTSTATUS)0xC000000DL)

    #define STATUS_BUFFER_OVERFLOW          ((NTSTATUS)0x80000005L)
    #define STATUS_ACCESS_DENIED            ((NTSTATUS)0xC0000022L)
    #define STATUS_BUFFER_TOO_SMALL         ((NTSTATUS)0xC0000023L)

    #define STATUS_ALREADY_COMMITTED        ((NTSTATUS)0xC0000021L)
    #define STATUS_INVALID_DEVICE_REQUEST   ((NTSTATUS)0xC0000010L)
    #define STATUS_INSUFFICIENT_RESOURCES   ((NTSTATUS)0xC000009AL)
#endif

#ifndef NT_SUCCESS
    #define NT_SUCCESS( _status_ ) ((STATUS_SUCCESS == _status_))
#endif

#ifndef NERR_Success
    #define NERR_Success    0
#endif



#define OBJ_INHERIT             0x00000002L
#define OBJ_PERMANENT           0x00000010L
#define OBJ_EXCLUSIVE           0x00000020L
#define OBJ_CASE_INSENSITIVE    0x00000040L
#define OBJ_OPENIF              0x00000080L
#define OBJ_OPENLINK            0x00000100L
#define OBJ_KERNEL_HANDLE       0x00000200L
#define OBJ_FORCE_ACCESS_CHECK  0x00000400L
#define OBJ_VALID_ATTRIBUTES    0x000007F2L


#define InitializeObjectAttributes( p, n, a, r, s ) { \
    (p)->Length = sizeof( OBJECT_ATTRIBUTES );          \
    (p)->RootDirectory = r;                             \
    (p)->Attributes = a;                                \
    (p)->ObjectName = n;                                \
    (p)->SecurityDescriptor = s;                        \
    (p)->SecurityQualityOfService = NULL;               \
    }


// =================================================
//
// File definitions
//
// =================================================

//
// Define the create disposition values
//
#define FILE_SUPERSEDE                  0x00000000
#define FILE_OPEN                       0x00000001
#define FILE_CREATE                     0x00000002
#define FILE_OPEN_IF                    0x00000003
#define FILE_OVERWRITE                  0x00000004
#define FILE_OVERWRITE_IF               0x00000005
#define FILE_MAXIMUM_DISPOSITION        0x00000005

//
// Define the create/open option flags
//
#define FILE_DIRECTORY_FILE                     0x00000001
#define FILE_WRITE_THROUGH                      0x00000002
#define FILE_SEQUENTIAL_ONLY                    0x00000004
#define FILE_NO_INTERMEDIATE_BUFFERING          0x00000008

#define FILE_SYNCHRONOUS_IO_ALERT               0x00000010
#define FILE_SYNCHRONOUS_IO_NONALERT            0x00000020
#define FILE_NON_DIRECTORY_FILE                 0x00000040
#define FILE_CREATE_TREE_CONNECTION             0x00000080

#define FILE_COMPLETE_IF_OPLOCKED               0x00000100
#define FILE_NO_EA_KNOWLEDGE                    0x00000200
#define FILE_OPEN_REMOTE_INSTANCE               0x00000400
#define FILE_RANDOM_ACCESS                      0x00000800

#define FILE_DELETE_ON_CLOSE                    0x00001000
#define FILE_OPEN_BY_FILE_ID                    0x00002000
#define FILE_OPEN_FOR_BACKUP_INTENT             0x00004000
#define FILE_NO_COMPRESSION                     0x00008000

#define FILE_RESERVE_OPFILTER                   0x00100000
#define FILE_OPEN_REPARSE_POINT                 0x00200000
#define FILE_OPEN_NO_RECALL                     0x00400000
#define FILE_OPEN_FOR_FREE_SPACE_QUERY          0x00800000


// =================================================
//
// Windows Security definitions
//
// =================================================

/*
//
// Privelages
//
#define SE_CREATE_TOKEN_NAME              TEXT("SeCreateTokenPrivilege")
#define SE_ASSIGNPRIMARYTOKEN_NAME        TEXT("SeAssignPrimaryTokenPrivilege")
#define SE_LOCK_MEMORY_NAME               TEXT("SeLockMemoryPrivilege")
#define SE_INCREASE_QUOTA_NAME            TEXT("SeIncreaseQuotaPrivilege")
#define SE_UNSOLICITED_INPUT_NAME         TEXT("SeUnsolicitedInputPrivilege")
#define SE_MACHINE_ACCOUNT_NAME           TEXT("SeMachineAccountPrivilege")
* #define SE_TCB_NAME                       TEXT("SeTcbPrivilege")
* #define SE_SECURITY_NAME                  TEXT("SeSecurityPrivilege")
* #define SE_TAKE_OWNERSHIP_NAME            TEXT("SeTakeOwnershipPrivilege")
#define SE_LOAD_DRIVER_NAME               TEXT("SeLoadDriverPrivilege")
#define SE_SYSTEM_PROFILE_NAME            TEXT("SeSystemProfilePrivilege")
#define SE_SYSTEMTIME_NAME                TEXT("SeSystemtimePrivilege")
#define SE_PROF_SINGLE_PROCESS_NAME       TEXT("SeProfileSingleProcessPrivilege")
#define SE_INC_BASE_PRIORITY_NAME         TEXT("SeIncreaseBasePriorityPrivilege")
#define SE_CREATE_PAGEFILE_NAME           TEXT("SeCreatePagefilePrivilege")
#define SE_CREATE_PERMANENT_NAME          TEXT("SeCreatePermanentPrivilege")
* #define SE_BACKUP_NAME                    TEXT("SeBackupPrivilege")
* #define SE_RESTORE_NAME                   TEXT("SeRestorePrivilege")
#define SE_SHUTDOWN_NAME                  TEXT("SeShutdownPrivilege")
#define SE_DEBUG_NAME                     TEXT("SeDebugPrivilege")
#define SE_AUDIT_NAME                     TEXT("SeAuditPrivilege")
#define SE_SYSTEM_ENVIRONMENT_NAME        TEXT("SeSystemEnvironmentPrivilege")
#define SE_CHANGE_NOTIFY_NAME             TEXT("SeChangeNotifyPrivilege")
#define SE_REMOTE_SHUTDOWN_NAME           TEXT("SeRemoteShutdownPrivilege")
*/

#ifndef SE_RELABEL_NAME
    #define SE_RELABEL_NAME                   TEXT("SeRelabelPrivilege")
    #define SE_INC_WORKING_SET_NAME           TEXT("SeIncreaseWorkingSetPrivilege")
    #define SE_TIME_ZONE_NAME                 TEXT("SeTimeZonePrivilege")
    #define SE_CREATE_SYMBOLIC_LINK_NAME      TEXT("SeCreateSymbolicLinkPrivilege")
#endif


#ifndef PROTECTED_DACL_SECURITY_INFORMATION
    #define PROTECTED_DACL_SECURITY_INFORMATION     (0x80000000L)
    #define PROTECTED_SACL_SECURITY_INFORMATION     (0x40000000L)
    #define UNPROTECTED_DACL_SECURITY_INFORMATION   (0x20000000L)
    #define UNPROTECTED_SACL_SECURITY_INFORMATION   (0x10000000L)
#endif


#ifndef ACCESS_SYSTEM_SECURITY
    #define ACCESS_SYSTEM_SECURITY           (0x01000000L)
#endif


// =================================================
//
// Function prototypes
//
// =================================================



NTSYSAPI 
NTSTATUS 
NTAPI 
NtFsControlFile( 
      IN  HANDLE              FileHandle,
      IN  HANDLE              Event,
      IN  PIO_APC_ROUTINE     ApcRoutine,
      IN  PVOID               ApcContext,
      IN OUT PIO_STATUS_BLOCK IoStatusBlock,
      IN  ULONG               FsControlCode,
      IN  PVOID               InputBuffer,
      IN  ULONG               InputBufferLength,
      OUT PVOID               OutputBuffer,
      OUT ULONG               OutputBufferLength );

typedef 
NTSTATUS 
(NTAPI *NtFsControlFile_Ptr)(
      IN  HANDLE              FileHandle,
      IN  HANDLE              Event,
      IN  PIO_APC_ROUTINE     ApcRoutine,
      IN  PVOID               ApcContext,
      IN OUT PIO_STATUS_BLOCK IoStatusBlock,
      IN  ULONG               FsControlCode,
      IN  PVOID               InputBuffer,
      IN  ULONG               InputBufferLength,
      OUT PVOID               OutputBuffer,
      OUT ULONG               OutputBufferLength );

////
//// Security
////
//NTSYSAPI 
//NTSTATUS 
//NTAPI 
//ZwSetSecurityObject( 
//    IN  HANDLE                  FileHandle,
//    IN  SECURITY_INFORMATION    SecurityInformation,
//    IN  PSECURITY_DESCRIPTOR    SecurityDescriptor );
//
//NTSYSAPI 
//NTSTATUS 
//NTAPI 
//ZwQuerySecurityObject( 
//    IN  HANDLE                  Handle,
//    IN  SECURITY_INFORMATION    SecurityInformation,
//    OUT PSECURITY_DESCRIPTOR    SecurityDescriptor,
//    IN  ULONG                   Length,
//    OUT PULONG                  LengthNeeded ); 


//NTSYSAPI 
//VOID
//NTAPI 
//RtlInitUnicodeString(
//    IN OUT PUNICODE_STRING  DestinationString,
//    IN PCWSTR  SourceString);

//
// General File I/O
//
typedef enum _NT_FILE_INFORMATION_CLASS {
//    FileDirectoryInformation         = 1,
    FileFullDirectoryInformation = 2,   // 2
    FileBothDirectoryInformation,   // 3
    FileBasicInformation,           // 4
    FileStandardInformation,        // 5
    FileInternalInformation,        // 6
    FileEaInformation,              // 7
    FileAccessInformation,          // 8
    FileNameInformation,            // 9
    FileRenameInformation,          // 10
    FileLinkInformation,            // 11
    FileNamesInformation,           // 12
    FileDispositionInformation,     // 13
    FilePositionInformation,        // 14
    FileFullEaInformation,          // 15
    FileModeInformation,            // 16
    FileAlignmentInformation,       // 17
    FileAllInformation,             // 18
    FileAllocationInformation,      // 19
    FileEndOfFileInformation,       // 20
    FileAlternateNameInformation,   // 21
    FileStreamInformation,          // 22
    FilePipeInformation,            // 23
    FilePipeLocalInformation,       // 24
    FilePipeRemoteInformation,      // 25
    FileMailslotQueryInformation,   // 26
    FileMailslotSetInformation,     // 27
    FileCompressionInformation,     // 28
    FileObjectIdInformation,        // 29
    FileCompletionInformation,      // 30
    FileMoveClusterInformation,     // 31
    FileQuotaInformation,           // 32
    FileReparsePointInformation,    // 33
    FileNetworkOpenInformation,     // 34
    FileAttributeTagInformation,    // 35
    FileTrackingInformation,        // 36
    FileIdBothDirectoryInformation, // 37
    FileIdFullDirectoryInformation, // 38
    FileValidDataLengthInformation, // 39
    FileShortNameInformation,       // 40
    FileIoCompletionNotificationInformation, // 41
    FileIoStatusBlockRangeInformation,       // 42
    FileIoPriorityHintInformation,           // 43
    FileSfioReserveInformation,              // 44
    FileSfioVolumeInformation,               // 45
    FileHardLinkInformation,                 // 46
    FileProcessIdsUsingFileInformation,      // 47
    FileNormalizedNameInformation,           // 48
    FileNetworkPhysicalNameInformation,      // 49
    FileMaximumInformation
} NT_FILE_INFORMATION_CLASS, *PNT_FILE_INFORMATION_CLASS;


typedef struct _FILE_RENAME_INFORMATION {
    BOOLEAN ReplaceIfExists;
    HANDLE RootDirectory;
    ULONG FileNameLength;
    WCHAR FileName[1];
} FILE_RENAME_INFORMATION, *PFILE_RENAME_INFORMATION;

typedef struct _FILE_NAME_INFORMATION {
    ULONG  FileNameLength;
    WCHAR  FileName[1];
} FILE_NAME_INFORMATION, *PFILE_NAME_INFORMATION;

typedef struct _FILE_INTERNAL_INFORMATION {
    LARGE_INTEGER IndexNumber;
} FILE_INTERNAL_INFORMATION, *PFILE_INTERNAL_INFORMATION;

typedef struct _FILE_STANDARD_INFORMATION {
    LARGE_INTEGER AllocationSize;
    LARGE_INTEGER EndOfFile;
    ULONG NumberOfLinks;
    BOOLEAN DeletePending;
    BOOLEAN Directory;
} FILE_STANDARD_INFORMATION, *PFILE_STANDARD_INFORMATION;

typedef enum _SECTION_INHERIT {
    ViewShare = 1,
    ViewUnmap = 2
} SECTION_INHERIT;

NTSYSAPI 
NTSTATUS 
NTAPI 
NtWriteFile(
    IN  HANDLE              FileHandle,
    IN  HANDLE              Event  OPTIONAL,
    IN  PIO_APC_ROUTINE     ApcRoutine  OPTIONAL,
    IN  PVOID               ApcContext  OPTIONAL,
    OUT PIO_STATUS_BLOCK    IoStatusBlock,
    IN  PVOID               Buffer,
    IN  ULONG               Length,
    IN  PLARGE_INTEGER      ByteOffset  OPTIONAL,
    IN  PULONG              Key  OPTIONAL
    );

NTSTATUS 
NTAPI
  NtReadFile(
    IN HANDLE               FileHandle,
    IN HANDLE               Event  OPTIONAL,
    IN PIO_APC_ROUTINE      ApcRoutine  OPTIONAL,
    IN PVOID                ApcContext  OPTIONAL,
    OUT PIO_STATUS_BLOCK    IoStatusBlock,
    OUT PVOID               Buffer,
    IN ULONG                Length,
    IN PLARGE_INTEGER       ByteOffset  OPTIONAL,
    IN PULONG               Key  OPTIONAL
    );

NTSYSAPI 
NTSTATUS 
NTAPI 
NtQueryInformationFile(
    IN  HANDLE                  FileHandle,
    OUT PIO_STATUS_BLOCK        IoStatusBlock,
    OUT PVOID                   FileInformation,
    IN  ULONG                   Length,
    IN  FILE_INFORMATION_CLASS  FileInformationClass
    );

typedef 
NTSTATUS 
(NTAPI *NtQueryInformationFile_Ptr)(
    IN  HANDLE                  FileHandle,
    OUT PIO_STATUS_BLOCK        IoStatusBlock,
    OUT PVOID                   FileInformation,
    IN  ULONG                   Length,
    IN  FILE_INFORMATION_CLASS  FileInformationClass
    );

NTSYSAPI 
NTSTATUS 
NTAPI 
NtCreateSection(
    OUT PHANDLE                 SectionHandle,
    IN  ACCESS_MASK             DesiredAccess,
    IN  POBJECT_ATTRIBUTES      ObjectAttributes OPTIONAL,
    IN  PLARGE_INTEGER          MaximumSize OPTIONAL,
    IN  ULONG                   SectionPageProtection,
    IN  ULONG                   AllocationAttributes,
    IN  HANDLE                  FileHandle OPTIONAL
    ); 

typedef 
NTSTATUS 
(NTAPI *NtCreateSection_Ptr)(
    OUT PHANDLE                 SectionHandle,
    IN  ACCESS_MASK             DesiredAccess,
    IN  POBJECT_ATTRIBUTES      ObjectAttributes OPTIONAL,
    IN  PLARGE_INTEGER          MaximumSize OPTIONAL,
    IN  ULONG                   SectionPageProtection,
    IN  ULONG                   AllocationAttributes,
    IN  HANDLE                  FileHandle OPTIONAL
    ); 

NTSYSAPI 
NTSTATUS 
NTAPI 
NtMapViewOfSection(
    IN HANDLE                   SectionHandle,
    IN HANDLE                   ProcessHandle,
    IN OUT PVOID                *BaseAddress,
    IN ULONG_PTR                ZeroBits,
    IN SIZE_T                   CommitSize,
    IN OUT PLARGE_INTEGER       SectionOffset  OPTIONAL,
    IN OUT PSIZE_T              ViewSize,
    IN SECTION_INHERIT          InheritDisposition,
    IN ULONG                    AllocationType,
    IN ULONG                    Win32Protect
    );

typedef 
NTSTATUS 
(NTAPI *NtMapViewOfSection_Ptr)(
    IN HANDLE                   SectionHandle,
    IN HANDLE                   ProcessHandle,
    IN OUT PVOID                *BaseAddress,
    IN ULONG_PTR                ZeroBits,
    IN SIZE_T                   CommitSize,
    IN OUT PLARGE_INTEGER       SectionOffset  OPTIONAL,
    IN OUT PSIZE_T              ViewSize,
    IN SECTION_INHERIT          InheritDisposition,
    IN ULONG                    AllocationType,
    IN ULONG                    Win32Protect
    );

NTSYSAPI 
NTSTATUS 
NTAPI 
NtUnmapViewOfSection(
    IN HANDLE                   ProcessHandle,
    IN PVOID                    BaseAddress
    );

typedef 
NTSTATUS 
(NTAPI *NtUnmapViewOfSection_Ptr)(
    IN HANDLE                   ProcessHandle,
    IN PVOID                    BaseAddress
    );

// ==============================================
//
//          Rtl string functions
//
// ==============================================
NTSYSAPI 
NTSTATUS 
NTAPI 
RtlCompareUnicodeString( 
    IN  PUNICODE_STRING     String1, 
    IN  PUNICODE_STRING     String2, 
    IN  BOOLEAN             CaseInSensitive );

NTSYSAPI 
NTSTATUS 
NTAPI 
RtlCopyUnicodeString(
    IN  PUNICODE_STRING     String1, 
    IN  PCUNICODE_STRING    String2 );



// ==============================================
//
//          Syncro functions
//
// ==============================================

NTSYSAPI
NTSTATUS
NTAPI
ZwWaitForSingleObject(
    IN HANDLE Handle,
    IN BOOLEAN Alertable,
    IN PLARGE_INTEGER Timeout OPTIONAL );

// ==============================================
//
//      LPC const, structs & functions
//
// ==============================================

typedef struct _CLIENT_ID {
    HANDLE UniqueProcess;
    HANDLE UniqueThread;
} CLIENT_ID, *PCLIENT_ID;


#if defined(USE_LPC6432)
    #define LPC_CLIENT_ID   CLIENT_ID64
    #define LPC_SIZE_T      ULONGLONG
    #define LPC_PVOID       ULONGLONG
    #define LPC_HANDLE      ULONGLONG
#else
    #define LPC_CLIENT_ID   CLIENT_ID
    #define LPC_SIZE_T      SIZE_T
    #define LPC_PVOID       PVOID
    #define LPC_HANDLE      HANDLE
#endif


typedef struct _LPC_MESSAGE {
    //USHORT DataSize;
    //USHORT MessageSize;
    //USHORT MessageType;
    //USHORT VirtualRangesOffset;
    //CLIENT_ID ClientId;
    //ULONG MessageId;
    //ULONG SectionSize;

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

//    UCHAR Data[0];
} PORT_MESSAGE, *PPORT_MESSAGE, LPC_MESSAGE, *PLPC_MESSAGE;


// LPC message types

typedef enum _LPC_TYPE {
    LPC_NEW_MESSAGE,
    LPC_REQUEST,
    LPC_REPLY,
    LPC_DATAGRAM,
    LPC_LOST_REPLY,
    LPC_PORT_CLOSED,
    LPC_CLIENT_DIED,
    LPC_EXCEPTION,
    LPC_DEBUG_EVENT,
    LPC_ERROR_EVENT,
    LPC_CONNECTION_REQUEST
} LPC_TYPE;


typedef enum _PORT_INFORMATION_CLASS {
    PortNoInformation
} PORT_INFORMATION_CLASS;


// LPC message read / write

typedef struct {
    ULONG       Length;
    LPC_HANDLE  SectionHandle;
    ULONG       SectionOffset; //OffsetInSection;
    LPC_SIZE_T  ViewSize;
    LPC_PVOID   ViewBase;
    LPC_PVOID   TargetViewBase; // OtherSideViewBase, ViewRemoteBase
} PORT_SECTION_WRITE, *PPORT_SECTION_WRITE,
  LPC_THIS_SIDE_MEMORY, *PLPC_THIS_SIDE_MEMORY,
  PORT_VIEW, *PPORT_VIEW;


typedef struct {
    ULONG       Length;
    LPC_SIZE_T  ViewSize;
    LPC_PVOID   ViewBase; 
} PORT_SECTION_READ, *PPORT_SECTION_READ,
  LPC_OTHER_SIDE_MEMORY, *PLPC_OTHER_SIDE_MEMORY,
  REMOTE_PORT_VIEW, *PREMOTE_PORT_VIEW;


NTSYSAPI 
NTSTATUS 
NTAPI 
ZwCreatePort (
   OUT PHANDLE             ServerPort,
   IN POBJECT_ATTRIBUTES   ObjectAttributes,
   IN ULONG                MaxDataSize, //MaximumConnectionInfoLength,
   IN ULONG                MaxMessageSize, // MaximumMessageLength,
   IN ULONG                Reserved
);


NTSYSAPI 
NTSTATUS 
NTAPI 
ZwCreateWaitablePort (
   OUT PHANDLE             ServerPort,
   IN POBJECT_ATTRIBUTES   ObjectAttributes,
   IN ULONG                MaxDataSize, //MaximumConnectionInfoLength,
   IN ULONG                MaxMessageSize, // MaximumMessageLength,
   IN ULONG                Reserved
);


//NTSYSAPI 
//NTSTATUS 
//NTAPI 
//ZwConnectPort (
//    OUT PHANDLE                     ClientServerPort,
//    IN PUNICODE_STRING              ServerPortName,
//    IN PSECURITY_QUALITY_OF_SERVICE SecurityQos,
//    IN OUT PLPC_THIS_SIDE_MEMORY    ClientSharedMemory OPTIONAL,
//    IN OUT PLPC_OTHER_SIDE_MEMORY   ServerSharedMemory OPTIONAL,
//    OUT PULONG                      MaximumMessageLength OPTIONAL,
//    IN OUT PVOID                    ConnectionInfo OPTIONAL,
//    IN OUT PULONG                   ConnectionInfoLength OPTIONAL
//);



NTSYSAPI 
NTSTATUS 
NTAPI 
ZwSecureConnectPort (
    OUT PHANDLE                     ClientServerPort,
    IN PUNICODE_STRING              ServerPortName,
    IN PSECURITY_QUALITY_OF_SERVICE SecurityQos,
    IN OUT PLPC_THIS_SIDE_MEMORY    ClientSharedMemory OPTIONAL,
    IN PSID                         ServerSid OPTIONAL,
    IN OUT PLPC_OTHER_SIDE_MEMORY   ServerSharedMemory OPTIONAL,
    OUT PULONG                      MaximumMessageLength OPTIONAL,
    IN OUT PVOID                    ConnectionInfo OPTIONAL,
    IN OUT PULONG                   ConnectionInfoLength OPTIONAL
);


NTSYSAPI 
NTSTATUS 
NTAPI 
ZwListenPort (
    IN HANDLE           ServerPort,
    OUT PLPC_MESSAGE    ConnectionRequest
);


NTSYSAPI 
NTSTATUS 
NTAPI 
ZwAcceptConnectPort (
    OUT PHANDLE                     ServerServerPort,
    IN HANDLE                       AlternativeReceiveServerPort OPTIONAL,
    IN PLPC_MESSAGE                 ConnectionReply,
    IN BOOLEAN                      AcceptConnection,
    IN OUT PLPC_THIS_SIDE_MEMORY    ServerSharedMemory OPTIONAL,
    IN OUT PLPC_OTHER_SIDE_MEMORY   ClientSharedMemory OPTIONAL
);


NTSYSAPI 
NTSTATUS 
NTAPI 
ZwCompleteConnectPort (     IN HANDLE ServerPort );


NTSYSAPI 
NTSTATUS 
NTAPI 
ZwRequestPort ( IN HANDLE       ServerPort,
                IN PLPC_MESSAGE Request );


//NTSYSAPI 
//NTSTATUS 
//NTAPI 
//ZwRequestWaitReplyPort ( IN HANDLE           ServerPort,
//                         IN PLPC_MESSAGE     Request,
//                         OUT PLPC_MESSAGE    Reply );


NTSYSAPI 
NTSTATUS 
NTAPI 
ZwReplyPort ( IN HANDLE       ServerPort,
              IN PLPC_MESSAGE Reply );


NTSYSAPI 
NTSTATUS 
NTAPI 
ZwReplyWaitReplyPort ( IN HANDLE           ServerPort,
                       IN OUT PLPC_MESSAGE Reply );


NTSYSAPI 
NTSTATUS 
NTAPI 
ZwReplyWaitReceivePort (
    IN HANDLE           ServerPort,
    OUT PHANDLE         ReceiveServerPort OPTIONAL,
    IN PLPC_MESSAGE     Reply OPTIONAL,
    OUT PLPC_MESSAGE    Request
);


NTSYSAPI 
NTSTATUS 
NTAPI 
ZwReplyWaitReceivePortEx (
    IN HANDLE           ServerPort,
    OUT PHANDLE         ReceiveServerPort OPTIONAL,
    IN PLPC_MESSAGE     Reply OPTIONAL,
    OUT PLPC_MESSAGE    Request,
    IN PLARGE_INTEGER   Timeout
);


NTSYSAPI 
NTSTATUS 
NTAPI 
ZwReadRequestData (
    IN HANDLE       ServerPort,
    IN PLPC_MESSAGE Request,
    IN ULONG        DataIndex,
    OUT PVOID       Buffer,
    IN ULONG        Length,
    OUT PULONG      ResultLength OPTIONAL
);


NTSYSAPI 
NTSTATUS 
NTAPI 
ZwWriteRequestData (
    IN HANDLE       ServerPort,
    IN PLPC_MESSAGE Request,
    IN ULONG        DataIndex,
    IN PVOID        Buffer,
    IN ULONG        Length,
    OUT PULONG      ResultLength OPTIONAL
);


NTSYSAPI 
NTSTATUS 
NTAPI 
ZwQueryInformationPort (
    IN HANDLE                   ServerPort,
    IN PORT_INFORMATION_CLASS   PortInformationClass,
    OUT PVOID                   PortInformation,
    IN ULONG                    Length,
    OUT PULONG                  ResultLength OPTIONAL
);


NTSYSAPI 
NTSTATUS 
NTAPI 
ZwImpersonateClientOfPort (
    IN HANDLE       ServerPort,
    IN PLPC_MESSAGE Request
);


// ==============================================
//
//      Registry const, structs & functions
//
// ==============================================

typedef struct _KEY_VALUE_FULL_INFORMATION {
    ULONG   TitleIndex;
    ULONG   Type;
    ULONG   DataOffset;
    ULONG   DataLength;
    ULONG   NameLength;
    WCHAR   Name[1];            // Variable size
//          Data[1];            // Variable size data not declared
} KEY_VALUE_FULL_INFORMATION, *PKEY_VALUE_FULL_INFORMATION;


typedef struct _KEY_VALUE_ENTRY {
    PUNICODE_STRING ValueName;
    ULONG           DataLength;
    ULONG           DataOffset;
    ULONG           Type;
} KEY_VALUE_ENTRY, *PKEY_VALUE_ENTRY;

typedef enum _KEY_VALUE_INFORMATION_CLASS {
    KeyValueBasicInformation,
    KeyValueFullInformation,
    KeyValuePartialInformation,
    KeyValueFullInformationAlign64,
    KeyValuePartialInformationAlign64,
    MaxKeyValueInfoClass  // MaxKeyValueInfoClass should always be the last enum
} KEY_VALUE_INFORMATION_CLASS;



NTSYSAPI
NTSTATUS
NTAPI
NtCreateKey(
    OUT PHANDLE             KeyHandle,
    IN  ACCESS_MASK         DesiredAccess,
    IN  POBJECT_ATTRIBUTES  ObjectAttributes,
        ULONG               TitleIndex,
    IN  PUNICODE_STRING     Class,
    IN  ULONG               CreateOptions,
    IN  PULONG              Disposition
    );

NTSYSAPI
NTSTATUS
NTAPI
NtQueryValueKey(
    IN  HANDLE              KeyHandle,
    IN  PUNICODE_STRING     ValueName,
    IN  KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
    OUT PVOID               KeyValueInformation,
    IN  ULONG               Length,
    OUT PULONG              ResultLength
    );

NTSYSAPI
NTSTATUS
NTAPI
NtSetValueKey(
    IN  HANDLE              KeyHandle,
    IN  PUNICODE_STRING     ValueName,
    IN  ULONG               TitleIndex,
    IN  ULONG               Type,
    IN  PVOID               Data,
    IN  ULONG               DataSize
    );



#ifdef __cplusplus
}
#endif

#endif // !__NT_STRUCT_H__