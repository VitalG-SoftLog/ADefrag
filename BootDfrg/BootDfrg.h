/*
    Defrag Native App (Boot Defrag) main module header

    Module name:

        BootDfrg.h

    Abstract:

        Defrag Native App (Boot Defrag) main module header.
        Defines service specific constants, types, structures, functions, etc.

    $Header: /home/CVS_DEFRAG/Defrag/Src/BootDfrg/BootDfrg.h,v 1.2 2009/11/30 12:48:36 dimas Exp $
    $Log: BootDfrg.h,v $
    Revision 1.2  2009/11/30 12:48:36  dimas
    1. Smart modes improved.
    2. FAT processing improved.

    Revision 1.1  2009/11/24 14:52:45  dimas
    no message

*/


#ifndef __NATIVE_MAIN__
#define __NATIVE_MAIN__

// =================================================
//
// Undocumented NT Constants, Macros & Data
//
// =================================================

#ifdef __cplusplus
extern "C" {
#endif

//
// Environment information
//
typedef struct {
    ULONG                       Unknown[21];     
    UNICODE_STRING              CommandLine;
    UNICODE_STRING              ImageFile;
} ENVIRONMENT_INFORMATION, *PENVIRONMENT_INFORMATION;

//
// This structure is passed as NtProcessStartup's parameter
//
typedef struct {
    ULONG                       Unknown[3];
    PENVIRONMENT_INFORMATION    Environment;
} STARTUP_ARGUMENT, *PSTARTUP_ARGUMENT;

typedef struct _PROCESS_MODULE_INFO             *PPROCESS_MODULE_INFO;  
typedef struct _PROCESS_PARAMETERS              *PPROCESS_PARAMETERS;      
typedef struct _CRITICAL_SECTION                *PCRITICAL_SECTION;
typedef struct _TEXT_INFO                       *PTEXT_INFO;


typedef struct _PROCESS_PARAMETERS {
DWORD             Allocated;
DWORD             Size;
DWORD             Flags;
DWORD             Reserved1;
LONG              Console;
DWORD             ProcessGroup;
HANDLE            StdInput;
HANDLE            StdOutput;
HANDLE            StdError;
UNICODE_STRING    WorkingDirectoryName;
HANDLE            WorkingDirectoryHandle;
UNICODE_STRING    SearchPath; //DllPath;
UNICODE_STRING    ImagePath;
UNICODE_STRING    CommandLine;
PWSTR             *Environment;
DWORD             X;
DWORD             Y;
DWORD             XSize;
DWORD             YSize;
DWORD             XCountChars;
DWORD             YCountChars;
DWORD             FillAttribute;
DWORD             Flags2;
WORD              ShowWindow;
WORD              Reserved2;
UNICODE_STRING    Title;
UNICODE_STRING    Desktop;
UNICODE_STRING    Reserved3;
UNICODE_STRING    Reserved4;

} PROCESS_PARAMETERS, *PPROCESS_PARAMETERS;


typedef struct _PEB {
BOOLEAN              InheritedAddressSpace;
BOOLEAN              ReadImageFileExecOptions;
BOOLEAN              BingDebugged;
BOOLEAN              SpareBool;  
PVOID                Mutant;
PVOID                SectionBaseAddress; // ImageBaseAddress : Ptr32 Void
PPROCESS_MODULE_INFO ProcessModuleInfo;  // Ldr  : Ptr32 _PEB_LDR_DATA
PPROCESS_PARAMETERS  ProcessParameters;  // Ptr32 _RTL_USER_PROCESS_PARAMETERS 
PVOID                SubSystemData;
HANDLE               ProcessHeap;
PCRITICAL_SECTION    FastPebLock;
PVOID                AcquireFastPebLock; // FastPebLockRoutine
PVOID                RelaseFastPebLock;  // FastPebUnlockRoutine
DWORD                EnvironmentUpdateCount;
PVOID                *User32Dispatch;    // KernelCallbackTable
DWORD                SystemReserved;
   struct {
                     DWORD ExecuteOptions  : 3;
                     DWORD SpareBits       : 28;
   };
PVOID                FreeList;           // Ptr32 _PEB_FREE_BLOCK
DWORD                TlsBitMapSize;      // TlsExpansionCounter;
PRTL_BITMAP          TlsBitMap;
DWORD                TlsBitMapData[2];   // TlsBitmapBits;
PVOID                ReadOnlySharedMemoryBase;
PVOID                ReadOnlySharedMemoryHeap;
PTEXT_INFO           TextInfo;           // ReadOnlyStaticServerData;
PVOID                InitAnsiCodePageData;      // AnsiCodePageData
PVOID                InitOemCodePageData;       // OemCodePageData
PVOID                InitUnicodeCaseTableData;  // UnicodeCaseTableData
DWORD                KeNumberProcessors;        // NumberOfProcessors
DWORD                NtGlobalFlag;
DWORD                d6C;                                
LARGE_INTEGER        MnCriticalSectionTimeout;
DWORD                MnHeapSegmentReserve;
DWORD                MnHeapSegmentCommit;
DWORD                MnHeapDeCommitTotalFreeThreshold;
DWORD                MnHeapDeCommitFreeBlockThreshold;
DWORD                NumberOfHeaps;
DWORD                AvailableHeaps;        // MaximumNumberOfHeaps
PHANDLE              ProcessHeapsListBuffer;// ProcessHeaps
PVOID                GdiSharedHandleTable;
PVOID                ProcessStarterHelper;
DWORD                GdiDCAttributeList;
PCRITICAL_SECTION    LoaderLock;
DWORD                NtMajorVersion;
DWORD                NtMinorVersion;
WORD                 NtBuildNumber; 
WORD                 CmNtCSDVersion;
DWORD                PlatformId;
DWORD                Subsystem;
DWORD                MajorSubsystemVersion;
DWORD                MinorSubsystemVersion;
KAFFINITY            AffinityMask;          // ImageProcessAffinityMask
DWORD                GdiHandleBuffer[34];
PVOID                PostProcessInitRoutine;
PVOID                TlsExpansionBitmap;
DWORD                TlsExpansionBitmapBits[32]; // 154 .. 1d0
HANDLE               Win32WindowStation;         // 1d4: SessionId : Uint4B
#if (VER_PRODUCTBUILD >= 2600)
   LARGE_INTEGER     AppCompatFlags;
   LARGE_INTEGER     AppCompatFlagsUser;
   PVOID             pShimData;
   PVOID             AppCompatInfo;
   UNICODE_STRING    CSDVersion;
   PVOID             ActivationContextData;
   PVOID             ProcessAssemblyStorageMap;
   PVOID             SystemDefaultActivationContextData;
   PVOID             SystemAssemblyStorageMap;
   DWORD             MinimumStackCommit;
#else
   PVOID             AppCompatInfo;
   UNICODE_STRING    CSDVersion;
   DWORD             d1E4; // !Anton - in windbg w2k sp2 - absent
#endif
} PEB, *PPEB;


// =================================================
//
// Undocumented NT Function prototypes
//
// =================================================

//
// Processes
//

NTSYSAPI
NTSTATUS
NTAPI
NtTerminateProcess (
    IN  HANDLE              ProcessHandle,
    IN  NTSTATUS            ExitStatus );

NTSYSAPI
NTSTATUS 
NTAPI 
NtDisplayString(
    IN  PUNICODE_STRING     String );

NTSYSAPI
PVOID 
NTAPI 
RtlNormalizeProcessParams( 
    IN OUT  PVOID ProcessParameters );

//
// I/O
//

NTSYSAPI
NTSTATUS
NTAPI
NtCancelIoFile(
    IN  HANDLE              FileHandle,
    OUT PIO_STATUS_BLOCK    IoStatusBlock );

NTSYSAPI
NTSTATUS
NTAPI
NtFlushBuffersFile(
    IN  HANDLE              FileHandle,
    OUT PIO_STATUS_BLOCK    IoStatusBlock );


//
// Syncronization
//

NTSYSAPI
NTSTATUS
NTAPI
NtCreateEvent(
    OUT PHANDLE             EventHandle,
    IN  ACCESS_MASK         DesiredAccess,
    IN  POBJECT_ATTRIBUTES  ObjectAttributes OPTIONAL,
    IN  EVENT_TYPE          EventType,
    IN  BOOLEAN             InitialState );

NTSYSAPI
NTSTATUS
NTAPI
NtSetEvent(
    IN  HANDLE              EventHandle,
    OUT PLONG               PreviousState OPTIONAL );

NTSYSAPI
NTSTATUS
NTAPI
NtResetEvent(
    IN  HANDLE              EventHandle,
    OUT PLONG               PreviousState OPTIONAL );

NTSYSAPI
NTSTATUS
NTAPI
NtWaitForSingleObject (
    IN  HANDLE              Handle,
    IN  BOOLEAN             Alertable,
    IN  PLARGE_INTEGER      Timeout OPTIONAL );

NTSYSAPI
NTSTATUS
NTAPI
NtWaitForMultipleObjects(
    IN  ULONG               HandleCount,
    IN  PHANDLE             Handles,
    IN  WAIT_TYPE           WaitType,
    IN  BOOLEAN             Alertable,
    IN  PLARGE_INTEGER      Timeout OPTIONAL );

//
// Registry
//
/*
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
*/
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

#define KEY_MAKE                0x00
#define KEY_BREAK               0x01
#define KEY_E0                  0x02
#define KEY_E1                  0x04
#define KEY_TERMSRV_SET_LED     0x08
#define KEY_TERMSRV_SHADOW      0x10
#define KEY_TERMSRV_VKPACKET    0x20

typedef struct _KEYBOARD_INPUT_DATA {
    USHORT    UnitId;
    USHORT    MakeCode;
    USHORT    Flags;
    USHORT    Reserved;
    ULONG     ExtraInformation;
} KEYBOARD_INPUT_DATA, *PKEYBOARD_INPUT_DATA;


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

#ifdef __cplusplus
}
#endif


// =================================================
//
// Constants, Macros & Data
//
// =================================================

#define     MAX_KBD_NUM     10

//
// Structures definitions
//

typedef struct _DEFRAG_KBD {
    int                         Num;
    HANDLE                      Handle;
    LARGE_INTEGER               CurPos;
    HANDLE                      ObjToWait;
} DEFRAG_KBD, *PDEFRAG_KBD;


typedef struct _DEFRAG_GLOBAL {

    //
    // Common State
    //
    DWORD                       AppId;
    WCHAR                       AppName[MAX_PATH];
    DWORD                       AppStatus;
    DWORD                       JobAction;

    DWORD                       RetCode;
    //OSVERSIONINFO               OsVersion;

    //
    // Defrag State
    //
    CRITICAL_SECTION            JobListSync;
    struct _DEFRAG_JOB_CONTEXT  *JobList;
    DWORD                       Flags;

    //
    // Memory Heap
    //
    HANDLE                      Heap;

    //
    // Keyboards
    //
    KEYBOARD_INPUT_DATA         KbdBuffer;
    DEFRAG_KBD                  Kbd[MAX_KBD_NUM];

    //
    // Specific functions, etc.
    //
    NtQueryInformationFile_Ptr  NtQueryFileInfo;

} DEFRAG_GLOBAL, *PDEFRAG_GLOBAL;

extern DEFRAG_GLOBAL      Global;


// =================================================
//
// Function prototypes
//
// =================================================

int
InitDefrag(
    int     argc, 
    WCHAR*  argv[] );

void
UninitDefrag( void );

NTSTATUS
ReadKbd(
    int                 i,
    PIO_STATUS_BLOCK    IoStatus );


#endif // !__NATIVE_MAIN__