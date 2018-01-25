// =================================================
//
// IoUtils.h : Defines I/O functions, etc.
//
// =================================================

#ifndef __DFRG_IO_H__
#define __DFRG_IO_H__

// =================================================
//
// Constants, Macros & Data
//
// =================================================

#define     NATIVE_NAME_PREFIX      L"\\??\\"
#define     NATIVE_NAME_PREFIX_LEN  4

// =================================================
//
// Function prototypes
//
// =================================================

PVOID
GetFunctionAddr( 
    WCHAR                   *LibName, 
    char                    *FunctionName );

int
DfrgOpenVolume( 
    PDEFRAG_VOLUME_CONTEXT  VolumeCtx );

void
DfrgCloseVolume( 
    PDEFRAG_VOLUME_CONTEXT  VolumeCtx );

int
DfrgGetVolumeBitMap( 
    PDEFRAG_VOLUME_CONTEXT  VolumeCtx,
    PLARGE_INTEGER          StartingLcn );

int
DfrgGetVolumeInfo( 
    PDEFRAG_VOLUME_CONTEXT  VolumeCtx );

int
DfrgGetVolumeNtfsData( 
    PDEFRAG_VOLUME_CONTEXT  VolumeCtx );

int
DfrgReadVolumeData( 
    PDEFRAG_VOLUME_CONTEXT      VolumeCtx,
    PLARGE_INTEGER              Lcn,
    PVOID                       Buffer,
    ULONG                       BufferLength );

int
DfrgOpenFile( 
    PDEFRAG_FILE_CONTEXT        FileCtx,
    BOOLEAN                     OpenById,
    PWCHAR                      FilePath = NULL );

int
DfrgOpenNamedStream( 
    PDEFRAG_STREAM_CONTEXT      StreamCtx );

int
DfrgOpenFileStream( 
    PDEFRAG_STREAM_CONTEXT      StreamCtx );

int
DfrgQueryFileName( 
    PDEFRAG_FILE_CONTEXT        FileCtx,
    PUNICODE_STRING             UniName );

int
DfrgQueryFileId( 
    PDEFRAG_FILE_CONTEXT        FileCtx );

void
DfrgCloseHandle( 
    HANDLE                      *Handle );

void
DfrgCloseFile( 
    PDEFRAG_FILE_CONTEXT        FileCtx );

int
DfrgGetStreamClustersMap( 
    PDEFRAG_FILE_CONTEXT        FileCtx,
    PRETRIEVAL_POINTERS_BUFFER  *FileClustersMap );

int
DfrgReadStreamRaw( 
    IN  PDEFRAG_STREAM_CONTEXT  StreamCtx,
    IN  PLARGE_INTEGER          FileOffset,
    OUT PBYTE                   Buffer,
    IN  ULONG                   BufferLength,
    IN  BOOLEAN                 ReadSparce,
    OUT PULONG                  BytesRead );

int
DfrgMoveClusters(
    PDEFRAG_FILE_CONTEXT        FileCtx,
    LARGE_INTEGER               StartingVcn,
    LARGE_INTEGER               TargetLcn,
    DWORD                       ClustersNum );

#endif // !__DFRG_IO_H__