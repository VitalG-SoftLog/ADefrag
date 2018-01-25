/*
    Defrag Engine

    Module name:

        IoUtils.cpp

    Abstract:

        Defrag Engine I/O module. 
        Defines I/O functions, etc.

    $Header: /home/CVS_DEFRAG/Defrag/Src/Engine/IoUtils.cpp,v 1.7 2009/12/24 15:02:51 dimas Exp $
    $Log: IoUtils.cpp,v $
    Revision 1.7  2009/12/24 15:02:51  dimas
    Some cosmetic improvements

    Revision 1.6  2009/12/24 10:52:20  dimas
    Check against Exclude Files list implemented

    Revision 1.5  2009/12/23 13:01:47  dimas
    1. Full File Name processing implemented.
    2. Top 100 Fragmented functionality for all file list requests implemented.

    Revision 1.4  2009/12/16 14:13:34  dimas
    DEFRAG_CMD_GET_FILE_INFO request implemented

    Revision 1.3  2009/11/26 15:56:54  dimas
    1. Smart modes improved.
    2. File names support improved.

    Revision 1.2  2009/11/24 15:15:10  dimas
    Bugs 5.2, 6.*, 15.14 fixed

    Revision 1.1  2009/11/24 14:52:15  dimas
    no message

*/

#include "stdafx.h"


#ifndef DEFRAG_NATIVE_APP

PVOID
GetFunctionAddr( 
    WCHAR   *LibName, 
    char    *FunctionName )
{
    PVOID               FunctionAddr = NULL;
    HMODULE             DLLHandle = NULL;


    if ( LibName && *LibName && FunctionName && *FunctionName )
    {
        DLLHandle = LoadLibrary( LibName );
        if ( DLLHandle )
        {
            FunctionAddr = GetProcAddress( DLLHandle, FunctionName ); 
        }
    }

    if ( DLLHandle )
    {
        FreeLibrary( DLLHandle );
        DLLHandle = NULL;
    }

    return FunctionAddr;

}

#endif

//
// Open volume for analyze/defrag
//
int
DfrgOpenVolume( 
    PDEFRAG_VOLUME_CONTEXT  VolumeCtx ) 
{
    int             Result = ERROR_INVALID_PARAMETER;
    NTSTATUS        Status = STATUS_SUCCESS;

    OBJECT_ATTRIBUTES   ObjectAttributes;
    IO_STATUS_BLOCK     IoStatus;
    UNICODE_STRING      UniName = { 0 };


    if ( !VolumeCtx || !VolumeCtx->VName || !*(VolumeCtx->VName) )
    {
        return Result;
    }

    __try
    {
        //
        // Open volume with native API
        //
        RtlInitUnicodeString( &UniName, VolumeCtx->VName );

        InitializeObjectAttributes( &ObjectAttributes, &UniName, OBJ_CASE_INSENSITIVE, NULL, NULL );

        Status = NtCreateFile(
                     &VolumeCtx->VHandle,
                     GENERIC_READ | SYNCHRONIZE,
                     &ObjectAttributes,
                     &IoStatus,
                     NULL,                          // allocation size
                     0,                             // FILE_ATTRIBUTE_NORMAL,
                     FILE_SHARE_READ | FILE_SHARE_WRITE,
                     FILE_OPEN,
                     FILE_SYNCHRONOUS_IO_NONALERT,  // | FILE_DIRECTORY_FILE
                     NULL,
                     0);

        if ( STATUS_SUCCESS == Status )
        {
            Result = NO_ERROR;
        }
        else
        {
            WCHAR       Mess[MAX_PATH];
            swprintf( Mess, L"DfrgOpenVolume '%s' NtCreateFile error", UniName.Buffer );
            Result = DislpayErrorMessage( Mess, Status, ERR_NT_STATUS );
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        Result = GetExceptionCode();
        DislpayErrorMessage( L"DfrgOpenVolume(). Exception !!!", Result, ERR_OS );
    }

	return Result;

} // end of DfrgOpenVolume

//
// Close volume
//
void
DfrgCloseVolume( 
    PDEFRAG_VOLUME_CONTEXT  VolumeCtx ) 
{

    if ( VolumeCtx && 
        VolumeCtx->VHandle && 
        INVALID_HANDLE_VALUE != VolumeCtx->VHandle )
    {
        NtClose( VolumeCtx->VHandle );
        VolumeCtx->VHandle = NULL;
    }
} // end of DfrgCloseVolume

//
// Query file full name
//
int
DfrgQueryFileName( 
    PDEFRAG_FILE_CONTEXT        FileCtx,
    PUNICODE_STRING             UniName )
{
    int                         Result = ERROR_INVALID_PARAMETER;
    NTSTATUS                    Status = STATUS_SUCCESS;

    ULONG                       NameLength = 0, AllocateLength = 0;
    FILE_NAME_INFORMATION       NameInfoStub, *NameInfo = &NameInfoStub;
    IO_STATUS_BLOCK             IoStatus;


#ifdef _DEBUG
    if ( !FileCtx || !FileCtx->FHandle || !UniName )
    {
        return Result;
    }
#endif

    __try
    {
        if ( !Global.NtQueryFileInfo )
        {
            __leave;
        }

        do
        {
            Status = Global.NtQueryFileInfo(
                         FileCtx->FHandle,
                         &IoStatus,
                         NameInfo,
                         NameLength + sizeof(FILE_NAME_INFORMATION),
                         (FILE_INFORMATION_CLASS)FileNameInformation );

            // 
            // If the operation is pending, wait for it to finish
            //
            if ( STATUS_PENDING == Status ) 
            {
                //WaitForSingleObject( FileCtx->FHandle, INFINITE );
                ZwWaitForSingleObject( FileCtx->FHandle, TRUE, NULL );
                Status = IoStatus.Status;
            }
        	
            if ( STATUS_SUCCESS == Status || // surprise !!!
                STATUS_BUFFER_OVERFLOW != Status ||
                NameInfo != &NameInfoStub )
            {
                break;
            }

            NameLength = NameInfo->FileNameLength;
            NameInfo = (PFILE_NAME_INFORMATION) malloc( NameLength + sizeof(FILE_NAME_INFORMATION) ); // extra WCHAR included
            if ( !NameInfo )
            {
                Result = ERROR_NOT_ENOUGH_MEMORY;
                __leave;
            }

        } while ( STATUS_BUFFER_OVERFLOW == Status );

        if ( STATUS_SUCCESS != Status )
        {
            WCHAR       Mess[MAX_PATH*2];
            swprintf( Mess, L"DfrgQueryFileName '%ls'. NtQueryFileInfo Error ", FileCtx->FName );

            Result = DislpayErrorMessage( Mess, Status, ERR_NT_STATUS );

            __leave;
        }

        NameInfo->FileName[NameLength/sizeof(WCHAR)] = L'\0';

        if ( UniInitUnicodeString( UniName, NameInfo->FileName ) )
        {
            Result = NO_ERROR;
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        Result = GetExceptionCode();
        DislpayErrorMessage( L"DfrgQueryFileName(). Exception !!!", Result, ERR_OS );
    }


    if ( NameInfo && NameInfo != &NameInfoStub )
    {
        free( NameInfo );
    }


	return Result;

} // end of DfrgQueryFileName

//
// Query file ID
//
int
DfrgQueryFileId( 
    PDEFRAG_FILE_CONTEXT        FileCtx )
{
    int                         Result = ERROR_INVALID_PARAMETER;
    NTSTATUS                    Status = STATUS_SUCCESS;

    FILE_INTERNAL_INFORMATION   InternalInfo = { 0 };
    IO_STATUS_BLOCK             IoStatus;


#ifdef _DEBUG
    WCHAR                       Mess[MAX_PATH];
    if ( !FileCtx || !FileCtx->FHandle )
    {
        return Result;
    }
#endif

    __try
    {
        if ( FileCtx->FId.FileId )
        {
            Result = NO_ERROR;
            __leave;
        }

        if ( !Global.NtQueryFileInfo )
        {
            __leave;
        }

        Status = Global.NtQueryFileInfo(
                     FileCtx->FHandle,
                     &IoStatus,
                     &InternalInfo,
                     sizeof(InternalInfo),
                     (FILE_INFORMATION_CLASS)FileInternalInformation );

        // 
        // If the operation is pending, wait for it to finish
        //
        if ( STATUS_PENDING == Status ) 
        {
            //WaitForSingleObject( FileCtx->FHandle, INFINITE );
            ZwWaitForSingleObject( FileCtx->FHandle, TRUE, NULL );
            Status = IoStatus.Status;
        }
    	
        if ( STATUS_SUCCESS != Status )
        {
            WCHAR   Mess[MAX_PATH*2];
            swprintf( Mess, L"DfrgQueryFileId '%ls'. NtQueryFileInfo Error ", 
                FileCtx->FName ? FileCtx->FName : L"" );

            Result = DislpayErrorMessage( Mess, Status, ERR_NT_STATUS );

            __leave;
        }

        Result = NO_ERROR;

#ifdef _DEBUG
        swprintf( Mess, L"   FileId: %016I64X", InternalInfo.IndexNumber.QuadPart );
        OutMessage( Mess);
#endif

        FileCtx->FId.FileId = InternalInfo.IndexNumber.QuadPart;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        Result = GetExceptionCode();
        DislpayErrorMessage( L"DfrgQueryFileId(). Exception !!!", Result, ERR_OS );
    }


	return Result;

} // end of DfrgQueryFileId

//
// Query file Info
//
#if 0
int
DfrgQueryFileInfo( 
    PDEFRAG_FILE_CONTEXT        FileCtx )
{
    int                         Result = ERROR_INVALID_PARAMETER;
    NTSTATUS                    Status = STATUS_SUCCESS;

    FILE_INTERNAL_INFORMATION   InternalInfo = { 0 };
    IO_STATUS_BLOCK             IoStatus;


#ifdef _DEBUG
    if ( !FileCtx || !FileCtx->FHandle )
    {
        return Result;
    }
#endif

    __try
    {
        if ( FileCtx->FId.FileId )
        {
            Result = NO_ERROR;
            __leave;
        }

        if ( !Global.NtQueryFileInfo )
        {
            __leave;
        }

        Status = Global.NtQueryFileInfo(
                     FileCtx->FHandle,
                     &IoStatus,
                     &InternalInfo,
                     sizeof(InternalInfo),
                     (FILE_INFORMATION_CLASS)FileInternalInformation );

        // 
        // If the operation is pending, wait for it to finish
        //
        if ( STATUS_PENDING == Status ) 
        {
            //WaitForSingleObject( FileCtx->FHandle, INFINITE );
            ZwWaitForSingleObject( FileCtx->FHandle, TRUE, NULL );
            Status = IoStatus.Status;
        }
    	
        if ( STATUS_SUCCESS != Status )
        {
            WCHAR   Mess[MAX_PATH*2];
            swprintf( Mess, L"DfrgQueryFileInfo '%ls'. NtQueryFileInfo Error ", FileCtx->FName );

            Result = DislpayErrorMessage( Mess, Status, ERR_NT_STATUS );

            __leave;
        }

        Result = NO_ERROR;

#ifdef _DEBUG
        WCHAR   Mess[MAX_PATH];
        swprintf( Mess, L"   FileId: %016I64X", InternalInfo.IndexNumber.QuadPart );
        OutMessage( Mess);
#endif

        FileCtx->FId.FileId = InternalInfo.IndexNumber.QuadPart;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        Result = GetExceptionCode();
        DislpayErrorMessage( L"DfrgQueryFileId(). Exception !!!", Result, ERR_OS );
    }


	return Result;

} // end of DfrgQueryFileInfo
#endif

//
// Open file for analyze/defrag
//
int
DfrgOpenFile( 
    PDEFRAG_FILE_CONTEXT    FileCtx,
    BOOLEAN                 OpenById,
    PWCHAR                  FilePath ) 
{
    int                     Result = ERROR_INVALID_PARAMETER;
    NTSTATUS                Status = STATUS_SUCCESS;

    OBJECT_ATTRIBUTES       ObjectAttributes;
    IO_STATUS_BLOCK         IoStatus;
    UNICODE_STRING          UniIdName = { 0 };

    UNICODE_STRING          UniName = { 0 };

#ifdef _DEBUG
    WCHAR                   Mess[MAX_PATH*2];

    //if ( !FileCtx || (!FilePath && !OpenById) )
    //{
    //    return Result;
    //}

    if ( OpenById && !(FileCtx->FId.FileId) )
    {
        swprintf( Mess, L"### DfrgOpenFile '%ls' by zero Id %016I64X ", 
            FileCtx->FName ? FileCtx->FName : L"", FileCtx->FId.FileId );
        OutMessage( Mess );
    }
#endif


    __try
    {
        //
        // Do not open file if it is already opened
        //
        if ( FileCtx->FHandle && INVALID_HANDLE_VALUE != FileCtx->FHandle )
        {
            Result = NO_ERROR;
            __leave;
        }

        //
        // Open file with native API
        //
        if ( OpenById )
        {
#ifdef _DEBUG
            if ( NtfsDisk != FileCtx->VolumeCtx->DiskType )
            {
                __leave;
            }
#endif
            //
            // Open by ID
            //
            UniIdName.Buffer = (PWSTR)&(FileCtx->FId.FileId);
            UniIdName.Length = UniIdName.MaximumLength = (USHORT)sizeof(ULONGLONG);

            InitializeObjectAttributes( 
                &ObjectAttributes, 
                &UniIdName, 
                OBJ_CASE_INSENSITIVE, 
                FileCtx->VolumeCtx->VHandle, 
                NULL );
        }
        else
        {
            //
            // Open by name
            //
            if ( !FilePath )
            {
                Result = DfrgBuildFileName( FileCtx->VolumeCtx->JobCtx, FileCtx, &UniName );
                if ( NO_ERROR != Result )
                {
                    __leave;
                }
            }
            else
            {
                if ( !UniInitUnicodeString( &UniName, FilePath ) )
                {
                    Result = ERROR_NOT_ENOUGH_MEMORY;
                    __leave;
                }
            }

#ifdef _DEBUGW
            WCHAR       Mess[MAX_PATH*2];
            swprintf( Mess, L"DfrgOpenFile  by Name '%ls' [%d]", UniName.Buffer, (int)UniName.Length );
            OutMessage( Mess );
#endif

            InitializeObjectAttributes( 
                &ObjectAttributes, 
                &UniName, 
                OBJ_CASE_INSENSITIVE, 
                NULL, 
                NULL );
        }

        Status = NtCreateFile(
                     &(FileCtx->FHandle),
                     ((FileCtx->DefragFlags & DEFRAG_FILE_DIRECTORY) ? GENERIC_READ : FILE_READ_ATTRIBUTES) | 
                     (OpenById ? SYNCHRONIZE : 0),
                     &ObjectAttributes,
                     &IoStatus,
                     NULL,                          // allocation size
                     0,                             // FILE_ATTRIBUTE_NORMAL,
                     FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                     FILE_OPEN,
                     FILE_NO_INTERMEDIATE_BUFFERING | 
                        (OpenById ? (FILE_OPEN_BY_FILE_ID | FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_FOR_BACKUP_INTENT) : 0),  // | FILE_DIRECTORY_FILE
                     NULL,
                     0);

        if ( STATUS_SUCCESS == Status )
        {
#ifdef _DEBUGW
            swprintf( Mess, L"ProcessFile '%s' NtCreateFile OK", UniName.Buffer );
            OutMessage( Mess );
#endif
            Result = NO_ERROR;
        }
        else
        {
#ifdef _DEBUG
            if ( OpenById )
            {
                swprintf( Mess, L"DfrgOpenFile by Id %016I64X (%ls) NtCreateFile error", 
                    *(PULONGLONG)(UniIdName.Buffer), FileCtx->FName ? FileCtx->FName : L"" );
            }
            else
            {
                swprintf( Mess, L"DfrgOpenFile by Name '%ls' NtCreateFile error", UniName.Buffer ); // 
            }
            Result = DislpayErrorMessage( Mess, Status, ERR_NT_STATUS );
#else
            Result = DislpayErrorMessage( L"", Status, ERR_NT_STATUS, NULL, TRUE );
#endif
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        Result = GetExceptionCode();
        DislpayErrorMessage( L"DfrgOpenFile(). Exception !!!", Result, ERR_OS );
    }

    UniFreeUnicodeString( &UniName );


	return Result;

} // end of DfrgOpenFile

//
// Open file for analyze/defrag
//
int
DfrgOpenNamedStream( 
    PDEFRAG_STREAM_CONTEXT      StreamCtx )
{
    int                         Result = ERROR_INVALID_PARAMETER;
    NTSTATUS                    Status = STATUS_SUCCESS;

    ULONG                       NameLength = 0;
    OBJECT_ATTRIBUTES           ObjectAttributes;
    IO_STATUS_BLOCK             IoStatus;

    UNICODE_STRING              UniName = { 0 };


#ifdef _DEBUG
    WCHAR               Mess[MAX_PATH*2];

    if ( !StreamCtx || !StreamCtx->FileCtx || 
        !StreamCtx->FileCtx->FHandle || INVALID_HANDLE_VALUE == StreamCtx->FileCtx->FHandle )
    {
        return Result;
    }
#endif

    _try
    {
        PDEFRAG_FILE_CONTEXT    FileCtx = StreamCtx->FileCtx;

        if ( FileCtx->StreamHandle )
        {
            DfrgCloseHandle( &(FileCtx->StreamHandle) );
        }

        //
        // Build stream name
        //
        if ( !UniAllocateAtLeast( &UniName, StreamCtx->NameLength + 25 ) )
        {
            Result = ERROR_NOT_ENOUGH_MEMORY;
            __leave;
        }
        *(UniName.Buffer) = L'\0';
        UniName.Length = 0;

        //
        // Append stream name
        //
        UniAppend( &UniName, L":" );
        if ( StreamCtx->NameLength )
        {
            UniAppend( &UniName, StreamCtx->StreamName );
        }

        //
        // Append attribute type
        //
        UniAppend( &UniName, L":" );
        UniAppend( &UniName, GetStreamTypeName( StreamCtx->MftStreamType ) );

#ifdef _DEBUG
        swprintf( Mess, L"      Stream Name  '%ls' ", UniName.Buffer );
        OutMessage( Mess );
#endif

        //
        // Init attributes
        //
        InitializeObjectAttributes( 
            &ObjectAttributes, 
            &UniName, 
            OBJ_CASE_INSENSITIVE, 
            FileCtx->FHandle, 
            NULL );

        //
        // Open file
        //
        Status = NtCreateFile(
                     &(FileCtx->StreamHandle),
                     ((FileCtx->DefragFlags & DEFRAG_FILE_DIRECTORY) ? GENERIC_READ : FILE_READ_ATTRIBUTES) | SYNCHRONIZE,
                     &ObjectAttributes,
                     &IoStatus,
                     NULL,                          // allocation size
                     0,                             // FILE_ATTRIBUTE_NORMAL,
                     FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                     FILE_OPEN,
                     FILE_NO_INTERMEDIATE_BUFFERING | FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_FOR_BACKUP_INTENT,
                     NULL,
                     0 );

        if ( STATUS_SUCCESS == Status )
        {
#ifdef _DEBUGW
            swprintf( Mess, L"ProcessFile '%s' NtCreateFile OK", UniName.Buffer );
            OutMessage( Mess );
#endif
            Result = NO_ERROR;
        }
        else
        {
#ifdef _DEBUG
            swprintf( Mess, L"DfrgOpenNamedStream '%ls'->'%ls' NtCreateFile error", 
                FileCtx->FName ? FileCtx->FName : L"", UniName.Buffer );
            Result = DislpayErrorMessage( Mess, Status, ERR_NT_STATUS );
#else
            Result = DislpayErrorMessage( L"", Status, ERR_NT_STATUS, NULL, TRUE );
#endif
        }

    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        Result = GetExceptionCode();
        DislpayErrorMessage( L"DfrgOpenNamedStream(). Exception !!!", Result, ERR_OS );
    }

    UniFreeUnicodeString( &UniName );


	return Result;

} // end of DfrgOpenNamedStream

//
// Open file's stream (default or ADS)
//
int
DfrgOpenFileStream( 
    PDEFRAG_STREAM_CONTEXT      StreamCtx )
{
    int                         Result = ERROR_INVALID_PARAMETER;
    NTSTATUS                    Status = STATUS_SUCCESS;

    PDEFRAG_FILE_CONTEXT        FileCtx = StreamCtx->FileCtx;


    //
    // Open file (default stream) for defrag (by ID on NTFS)
    //
    Result = DfrgOpenFile( FileCtx, (FileCtx->VolumeCtx->DiskType == NtfsDisk) );
    if ( NO_ERROR != Result )
    {
        return Result;
    }

    //if ( MftAttrData == StreamCtx->MftStreamType && !StreamCtx->NameLength ||             // default file data stream
    if ( StreamCtx == FileCtx->DefaultStream ||                                             // default file data stream
        //MftAttrIndexAllocation == StreamCtx->MftStreamType && StreamCtx->NameLength == 4 && // default directory stream
        //!wcscmp( StreamCtx->StreamName, L"$I30" ) || 
        MftAttrStandardInformation > StreamCtx->MftStreamType || MftAttrLoggedUtilityStream < StreamCtx->MftStreamType // unknown
        )
    {
        //
        // Use default file handle.
        // TODO: Check directories with unnamed $DATA streams
        //
#ifdef _DEBUGWW
        BOOLEAN  DefaultStreamProcessed = FALSE;
        if ( DefaultStreamProcessed )
        {
            //ASSERT( FALSE);
            DefaultStreamProcessed = TRUE;
        }
        DefaultStreamProcessed = TRUE;
#endif
    }
    else
    {
        //
        // Open ADS for defrag by name
        //
        Result = DfrgOpenNamedStream( StreamCtx );
    }


	return Result;

} // end of DfrgOpenFileStream

//
// Close file 
//
void
DfrgCloseHandle( 
    HANDLE      *Handle ) 
{
    if ( Handle && 
        *Handle && 
        INVALID_HANDLE_VALUE != *Handle )
    {
        NtClose( *Handle );
        *Handle = NULL;
    }
} // end of DfrgCloseFile



//
// Close file 
//
void
DfrgCloseFile( 
    PDEFRAG_FILE_CONTEXT  FileCtx ) 
{
    if ( FileCtx  )
    {
        DfrgCloseHandle( &(FileCtx->StreamHandle) );
        DfrgCloseHandle( &(FileCtx->FHandle) );
    }
} // end of DfrgCloseFile

//
// Get file clusters map.
//
int
DfrgGetStreamClustersMap( 
    PDEFRAG_FILE_CONTEXT        FileCtx,
    PRETRIEVAL_POINTERS_BUFFER  *StreamClustersMap )
{
    int                     Result = ERROR_INVALID_PARAMETER;
    NTSTATUS                Status = STATUS_BUFFER_OVERFLOW;

#ifdef _DEBUG
    //
    // Input parameters check.
    // Skip this for release version for performance reason
    //
    if ( !FileCtx || !FileCtx->VolumeCtx || !FileCtx->VolumeCtx->ClustersMapBuffer ||
        !FileCtx->FHandle || INVALID_HANDLE_VALUE == FileCtx->FHandle || !StreamClustersMap )
    {
        return Result;
    }
#endif

    __try
    {
        STARTING_VCN_INPUT_BUFFER   StartVcn = { 0 };
        ULONG                       MapLength = CLUSTER_MAP_BUFFER_SIZE;
        PRETRIEVAL_POINTERS_BUFFER  ClustersMap = (PRETRIEVAL_POINTERS_BUFFER)FileCtx->VolumeCtx->ClustersMapBuffer;;
        IO_STATUS_BLOCK             IoStatus;

        *StreamClustersMap = NULL;

        while( STATUS_BUFFER_OVERFLOW == Status )
        {
            Status = NtFsControlFile( 
                        FileCtx->StreamHandle ? FileCtx->StreamHandle : FileCtx->FHandle, 
                        NULL, 
                        NULL, 
                        0, 
                        &IoStatus,
                        FSCTL_GET_RETRIEVAL_POINTERS,
                        &StartVcn, 
                        sizeof( StartVcn ),
                        ClustersMap, 
                        MapLength );

            // 
            // If the operation is pending, wait for it to finish
            //
            if ( STATUS_PENDING == Status ) 
            {
                //WaitForSingleObject( FileCtx->FHandle, INFINITE );
                ZwWaitForSingleObject( FileCtx->FHandle, TRUE, NULL );
                Status = IoStatus.Status;
            }
        	
	        if ( STATUS_SUCCESS == Status && !*StreamClustersMap ||
                STATUS_BUFFER_OVERFLOW == Status ) 
            {
                if ( *StreamClustersMap )
                {
                    //
                    // Itterative overflow ???
                    //
                    free( *StreamClustersMap );
                    *StreamClustersMap = NULL;
                }

                MapLength = (ULONG)(sizeof(RETRIEVAL_POINTERS_BUFFER) + sizeof(LARGE_INTEGER)*2*(ClustersMap->ExtentCount-1));
                *StreamClustersMap = (PRETRIEVAL_POINTERS_BUFFER) malloc( MapLength );
                if ( !*StreamClustersMap )
                {
                    Result = ERROR_NOT_ENOUGH_MEMORY;
                    __leave;
                }

                if ( STATUS_SUCCESS == Status )
                {
                    //
                    // Got it from first read
                    //
                    memcpy( *StreamClustersMap, ClustersMap, MapLength );
                    break;
                }
                ClustersMap = *StreamClustersMap;
            }
        }

        if ( STATUS_SUCCESS == Status )
        {
            Result = NO_ERROR;
        }
        else
        {
            Result = DislpayErrorMessage( L"DfrgGetStreamClustersMap(): Get cluster map error", Status, ERR_NT_STATUS );
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        Result = GetExceptionCode();
        DislpayErrorMessage( L"DfrgGetStreamClustersMap(). Exception !!!", Result, ERR_OS );
    }


	return Result;

} // end of DfrgGetStreamClustersMap

//
// Get Volume bitmap frame
//
int
DfrgGetVolumeBitMap( 
    PDEFRAG_VOLUME_CONTEXT  VolumeCtx,
    PLARGE_INTEGER          StartingLcn )
{
    int                     Result = ERROR_INVALID_PARAMETER;
    NTSTATUS                Status = STATUS_SUCCESS;

    IO_STATUS_BLOCK         IoStatus;

    STARTING_LCN_INPUT_BUFFER   NextLcn = { 0 };


#ifdef _DEBUG
    if ( !VolumeCtx || !VolumeCtx->VHandle || INVALID_HANDLE_VALUE == VolumeCtx ->VHandle ||
        !VolumeCtx->BitMapBuffer )
    {
        return Result;
    }
#endif

    __try
    {
        DWORD                   BitMapLength;
        PBYTE                   BitMapBuffer;

        BitMapBuffer = VolumeCtx->BitMapBuffer;
        BitMapLength = VolumeCtx->BitMapLength.LowPart + sizeof(VOLUME_BITMAP_BUFFER);

        if ( StartingLcn )
        {
            NextLcn.StartingLcn.QuadPart = StartingLcn->QuadPart;
        }

        //NtCreateEvent(&hDevIoCtlDone,EVENT_ALL_ACCESS,NULL,SynchronizationEvent,FALSE);
        //NtDeviceIoControlFile
        //
        Status = NtFsControlFile( 
                    VolumeCtx->VHandle, 
                    NULL, 
                    NULL, 
                    0, 
                    &IoStatus,
                    FSCTL_GET_VOLUME_BITMAP,
                    &NextLcn, 
                    sizeof( NextLcn ),
                    BitMapBuffer, 
                    BitMapLength );

        // 
        // If the operation is pending, wait for it to finish
        //
        if ( STATUS_PENDING == Status ) 
        {
            //WaitForSingleObject( VolumeCtx->VHandle, INFINITE );
            ZwWaitForSingleObject( VolumeCtx->VHandle, TRUE, NULL );
            Status = IoStatus.Status;
        }
    	
	    if ( STATUS_SUCCESS == Status  ||
            STATUS_BUFFER_OVERFLOW == Status ) // 0x80000005
        {
            PVOLUME_BITMAP_BUFFER       BitMap = (PVOLUME_BITMAP_BUFFER)VolumeCtx->BitMapBuffer;

#ifdef _DEBUG
            WCHAR                       Mess[MAX_PATH];

            swprintf( Mess, L"   bitmap from %I64d to %I64d (=%I64d) (length: %I64d ?= %I64d), Stat %08X", 
                BitMap->StartingLcn, BitMap->BitmapSize, VolumeCtx->NtfsData.TotalClusters.QuadPart,
                VolumeCtx->NtfsData.TotalClusters.QuadPart/8, VolumeCtx->BitMapLength,
                Status );
            OutMessage( Mess );

            wcscpy ( Mess, L"   " );;
            for ( int i=0; i<16; i++ )
            {
                swprintf( Mess+wcslen(Mess), L"%02X ", ((PVOLUME_BITMAP_BUFFER)VolumeCtx->BitMapBuffer)->Buffer[i] );
            }
            OutMessage( Mess );
#endif
            Status = STATUS_SUCCESS;
	    }


        if ( STATUS_SUCCESS == Status )
        {
            Result = NO_ERROR;
        }
        else
        {
            Result = DislpayErrorMessage( L"DfrgGetVolumeBitMap: Get bitmap error", Status, ERR_NT_STATUS );
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        Result = GetExceptionCode();
        DislpayErrorMessage( L"DfrgGetVolumeBitMap(). Exception !!!", Result, ERR_OS );
    }


	return Result;

} // end of DfrgGetVolumeBitMap


//
// Get Volume NtfsData
//
int
DfrgGetVolumeNtfsData( 
    PDEFRAG_VOLUME_CONTEXT  VolumeCtx )
{
    int                     Result = ERROR_INVALID_PARAMETER;
    NTSTATUS                Status = STATUS_SUCCESS;

    IO_STATUS_BLOCK         IoStatus;


    if ( !VolumeCtx || !VolumeCtx->VHandle || INVALID_HANDLE_VALUE == VolumeCtx ->VHandle )
    {
        return Result;
    }

    __try
    {
        //NtCreateEvent(&hDevIoCtlDone,EVENT_ALL_ACCESS,NULL,SynchronizationEvent,FALSE);
        //NtDeviceIoControlFile
        //
        Status = NtFsControlFile( 
                    VolumeCtx->VHandle, 
                    NULL, 
                    NULL, 
                    0, 
                    &IoStatus,
                    FSCTL_GET_NTFS_VOLUME_DATA,
                    NULL, 
                    0,
                    &(VolumeCtx->NtfsData), 
                    sizeof(VolumeCtx->NtfsData) );

        // 
        // If the operation is pending, wait for it to finish
        //
        if ( STATUS_PENDING == Status ) 
        {
            //WaitForSingleObject( VolumeCtx->VHandle, INFINITE );
            ZwWaitForSingleObject( VolumeCtx->VHandle, TRUE, NULL );
            Status = IoStatus.Status;
        }
    	
        if ( STATUS_SUCCESS == Status )
        {
#ifdef _DEBUG
            WCHAR                       Mess[MAX_PATH];

            swprintf( Mess, L"Volume %ls NTFS data: total %I64d clusters,\n"
                            L"   MFT %I64d [%I64d], %I64d (zone %I64d.. %I64d)\n"
                            L"   Bytes/cluster per rec: %u %u", 
                VolumeCtx->VName, 
                VolumeCtx->NtfsData.TotalClusters, 
                VolumeCtx->NtfsData.MftStartLcn, 
                VolumeCtx->NtfsData.MftValidDataLength,
                VolumeCtx->NtfsData.Mft2StartLcn, 
                VolumeCtx->NtfsData.MftZoneStart, 
                VolumeCtx->NtfsData.MftZoneEnd,
                VolumeCtx->NtfsData.BytesPerFileRecordSegment,
                VolumeCtx->NtfsData.ClustersPerFileRecordSegment );
            OutMessage( Mess );
#endif
            VolumeCtx->ValidRecordsNum.QuadPart = 
                VolumeCtx->NtfsData.MftValidDataLength.QuadPart / VolumeCtx->NtfsData.BytesPerFileRecordSegment;

            Result = NO_ERROR;
        }
        else
        {
            Result = DislpayErrorMessage( L"DfrgGetVolumeNtfsData: Get NTFS data error", Status, ERR_NT_STATUS );
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        Result = GetExceptionCode();
        DislpayErrorMessage( L"DfrgGetVolumeNtfsData(). Exception !!!", Result, ERR_OS );
    }


	return Result;

} // end of DfrgGetVolumeNtfsData

//
// Get Volume data
//
int
DfrgReadVolumeData( 
    PDEFRAG_VOLUME_CONTEXT  VolumeCtx,
    PLARGE_INTEGER          Offset,
    PVOID                   Buffer,
    ULONG                   BufferLength )
{
    int                     Result = ERROR_INVALID_PARAMETER;
    NTSTATUS                Status = STATUS_SUCCESS;

    IO_STATUS_BLOCK         IoStatus;


#ifdef _DEBUG
    //
    // Input parameters check.
    // Skip this for release version for performance reason
    //
    if ( !VolumeCtx || !VolumeCtx->VHandle || INVALID_HANDLE_VALUE == VolumeCtx ->VHandle )
    {
        return Result;
    }
#endif

    __try
    {
        Status = NtReadFile(
                    VolumeCtx->VHandle, 
                    NULL, 
                    NULL, 
                    NULL, 
                    &IoStatus,
                    Buffer,
                    BufferLength,
                    Offset,
                    NULL );

        if ( STATUS_SUCCESS == Status )
        {
            Result = NO_ERROR;
        }
        else
        {
            WCHAR   Mess[MAX_PATH];
            swprintf( Mess, L"DfrgReadVolumeData: Read %d from %I64d error ", 
                BufferLength, Offset->QuadPart );
            Result = DislpayErrorMessage( Mess, Status, ERR_NT_STATUS );
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        Result = GetExceptionCode();
        DislpayErrorMessage( L"DfrgReadVolumeData(). Exception !!!", Result, ERR_OS );
    }


	return Result;

} // end of DfrgReadVolumeData

//
// Read files clusters from Volume.
// Currently is used to read meta files ($Mft) only.
// All lengths (BufferLength & FileOffset) should be multiple of BytesPerCluster.
// TODO: convert lengths to clusters.
//
int
DfrgReadStreamRaw( 
    IN  PDEFRAG_STREAM_CONTEXT  StreamCtx,
    IN  PLARGE_INTEGER          FileOffset,
    OUT PBYTE                   Buffer,
    IN  ULONG                   BufferLength,
    IN  BOOLEAN                 ReadSparce,
    OUT PULONG                  BytesRead )
{
    int                         Result = ERROR_INVALID_PARAMETER;

    LARGE_INTEGER               ExtentLength = { 0 }, ClustersToSkip = { 0 },
                                BytesToRead = { 0 }, VolumeOffset = { 0 };

    PRETRIEVAL_POINTERS_BUFFER  ClustersMap = NULL;
    STARTING_VCN_INPUT_BUFFER   StartVcn;
    DWORD                       BytesPerCluster;

#ifdef _DEBUG
    WCHAR                       Mess[MAX_PATH];


    //
    // Input parameters check.
    // Skip this for release version for performance reason
    //
    if ( !StreamCtx || !StreamCtx->FileCtx || !StreamCtx->FileCtx->VolumeCtx || !StreamCtx->ClustersMap || 
        !FileOffset || !Buffer || !BufferLength || !BytesRead )
    {
        return Result;
    }

    //
    // TODO: 
    //
    //DWORD       BytesPerSectorM1 = StreamCtx->VolumeCtx->NtfsData.BytesPerSector - 1;
    //if ( BufferLength & BytesPerSectorM1 )
    //{
    //    __leave;
    //}
#endif

    PDEFRAG_VOLUME_CONTEXT  VolumeCtx = StreamCtx->FileCtx->VolumeCtx;

    __try
    {
        *BytesRead = 0;

        BytesPerCluster = VolumeCtx->NtfsData.BytesPerCluster;

        ClustersToSkip.QuadPart = FileOffset->QuadPart / BytesPerCluster;

        ClustersMap = StreamCtx->ClustersMap;
        StartVcn.StartingVcn = ClustersMap->StartingVcn; // .QuadPart

        for ( DWORD i=0; 
            i<ClustersMap->ExtentCount && *BytesRead<BufferLength; 
            StartVcn.StartingVcn = ClustersMap->Extents[i].NextVcn, i++ )
        {
            ExtentLength.QuadPart = ClustersMap->Extents[i].NextVcn.QuadPart - StartVcn.StartingVcn.QuadPart;
            BytesToRead = ExtentLength;
            VolumeOffset = ClustersMap->Extents[i].Lcn;

            if ( !(*BytesRead) && ClustersToSkip.QuadPart )
            {
                //
                // First block. If FileOffset is define adjust length and offset.
                //
                if ( ClustersToSkip.QuadPart > ExtentLength.QuadPart )
                {
                    //
                    // Skip begining of file up to *FileOffset
                    //
                    ClustersToSkip.QuadPart -= ExtentLength.QuadPart;
                    continue;
                }
                else 
                {
                    //
                    // Adjust offset & length
                    //
                    VolumeOffset.QuadPart += ClustersToSkip.QuadPart;
                    BytesToRead.QuadPart -= ClustersToSkip.QuadPart;
                    ClustersToSkip.QuadPart = 0;
                }
            }

            VolumeOffset.QuadPart   *= BytesPerCluster;
            BytesToRead.QuadPart    *= BytesPerCluster;

            if ( BytesToRead.HighPart || BytesToRead.LowPart > (BufferLength - *BytesRead) )
            {
                //OutMessage( L"### DfrgReadStreamRaw() Panic. BytesToRead > 4Gb ?" );
                BytesToRead.HighPart = 0;
                BytesToRead.LowPart = BufferLength - *BytesRead;
            }

            //
            // TODO: Adjust length to sector size
            //
            //BytesToRead = (BytesToRead + BytesPerSectorM1) & (BytesPerSectorM1);
                
            if ( ClustersMap->Extents[i].Lcn.QuadPart == VIRTUAL_LCN ) 
            {
                //
                // Sparce extent. Skip it or zero it.
                //
#ifdef _DEBUG
                swprintf( Mess, L"   Sparce extent/run:  Vcn: %I64d  Length: %I64d",
                    StartVcn.StartingVcn.QuadPart, 
                    ExtentLength.QuadPart ); 
                OutMessage( Mess );
#endif
                if ( !ReadSparce ) // TODO: 
                {
                    continue;
                }
                memset( Buffer+*BytesRead, 0, BytesToRead.LowPart );
	        } 
            else 
            {
                //
                // Real extent
                //
                if ( NtfsDisk != VolumeCtx->DiskType )
                {
                    //
                    // Adjust offset on FAT volume
                    //
                    //VolumeOffset.QuadPart -= (2 * BytesPerCluster);
                    VolumeOffset.QuadPart += (VolumeCtx->DosData.DataOffset * VolumeCtx->NtfsData.BytesPerSector);
                }

#ifdef _DEBUGW
		        swprintf( Mess, L"   Vcn: %I64d  Lcn: %I64d  Length: %I64d",
			        StartVcn.StartingVcn.QuadPart, ClustersMap->Extents[i].Lcn.QuadPart, 
			        ExtentLength.QuadPart );
                OutMessage( Mess );
		        swprintf( Mess, L"   Read: %d bytes from offset %I64d",
                    BytesToRead.LowPart, VolumeOffset.QuadPart );
                OutMessage( Mess );
#endif
                Result = DfrgReadVolumeData( 
                                VolumeCtx,
                                &VolumeOffset,
                                Buffer+*BytesRead,
                                BytesToRead.LowPart );

                if ( NO_ERROR != Result )
                {
                    break;
                }
	        }

            *BytesRead += BytesToRead.LowPart;
        }
/*
        if ( NO_ERROR != Result )
        {
            WCHAR   Mess[MAX_PATH];
            swprintf( Mess, L"DfrgReadFileRaw: Read %d from %I64d error ", 
                BytesToRead.LowPart, VolumeOffset.QuadPart );
            Result = DislpayErrorMessage( Mess, Status, ERR_NT_STATUS );
        }
*/
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        Result = GetExceptionCode();
        DislpayErrorMessage( L"DfrgReadStreamRaw(). Exception !!!", Result, ERR_OS );
    }


	return Result;

} // end of DfrgReadStreamRaw

int
DfrgMoveClusters(
    PDEFRAG_FILE_CONTEXT        FileCtx,
    LARGE_INTEGER               StartingVcn,
    LARGE_INTEGER               TargetLcn,
    DWORD                       ClustersNum )
{
    int                         Result = NO_ERROR;
    NTSTATUS                    Status = STATUS_INVALID_PARAMETER;


#ifdef _DEBUG
    WCHAR   Mess[MAX_PATH];

    swprintf( Mess, L"   DfrgMoveClusters:  VCN %I64d [%d] to LCN %I64d",
        StartingVcn.QuadPart, ClustersNum, TargetLcn.QuadPart );
    OutMessage( Mess );

    if ( DEFRAG_JOB_DEBUG1 == FileCtx->VolumeCtx->JobCtx->Type )
    {
        return Result;
    }
#endif

    __try
    {
        MOVE_FILE_DATA              Move;
        IO_STATUS_BLOCK             IoStatus;

        //
        // Init MOVE_FILE_DATA descriptor and make the call
        //
        Move.FileHandle     = FileCtx->StreamHandle ? FileCtx->StreamHandle : FileCtx->FHandle; 
        Move.StartingVcn    = StartingVcn;
        Move.StartingLcn    = TargetLcn;
        Move.ClusterCount   = ClustersNum;

        Status = NtFsControlFile( 
                    FileCtx->VolumeCtx->VHandle, 
                    NULL, 
                    NULL, 
                    0, 
                    &IoStatus,
                    FSCTL_MOVE_FILE,
                    &Move, 
                    sizeof( Move ),
                    NULL, 
                    0 );

        // 
        // If the operation is pending, wait for it to finish
        //
        if ( STATUS_PENDING == Status ) 
        {
            //WaitForSingleObject( FileCtx->FHandle, INFINITE );
            ZwWaitForSingleObject( FileCtx->FHandle, TRUE, NULL );
            Status = IoStatus.Status;
        }
    	
        if ( STATUS_SUCCESS == Status )
        {
            Result = NO_ERROR;
        }
        else
        {
            WCHAR       Mess[MAX_PATH];
            swprintf( Mess, L"DfrgMoveClusters. NtFsControlFile" );
            Result = DislpayErrorMessage( Mess, Status, ERR_NT_STATUS );
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        Result = GetExceptionCode();
        DislpayErrorMessage( L"DfrgMoveClusters(). Exception !!!", Result, ERR_OS );
    }

    return Result;

} // end of DfrgMoveClusters
