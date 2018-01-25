/*
    Defrag Engine Core

    Module name:

        FileNames.cpp

    Abstract:

        Defrag Engine file names processing functions module. 
        Contains core defragmentation functions.

    $Header: /home/CVS_DEFRAG/Defrag/Src/Engine/FileNames.cpp,v 1.3 2009/12/24 15:02:51 dimas Exp $
    $Log: FileNames.cpp,v $
    Revision 1.3  2009/12/24 15:02:51  dimas
    Some cosmetic improvements

    Revision 1.2  2009/12/24 12:23:50  dimas
    Some bugs fixed

    Revision 1.1  2009/12/24 10:52:20  dimas
    Check against Exclude Files list implemented

*/

#include "stdafx.h"



void
DfrgSplitPath(
    PWCHAR      FullName, 
    PWCHAR      Drive, 
    PWCHAR      Dir, 
    PWCHAR      Name, 
    PWCHAR      Ext )
{
    PWCHAR      PathPtr, NamePtr, ExtPtr;
    WCHAR       SaveChar;


    *Drive  = L'\0'; 
    *Dir    = L'\0';
    *Name   = L'\0';
    *Ext    = L'\0';

    PathPtr = wcschr( FullName, L':' );

    if ( PathPtr )
    {
        PathPtr++;
        SaveChar = *PathPtr;
        *PathPtr = L'\0';
        wcscpy( Drive, FullName ); // Drive
        *PathPtr = SaveChar;
    }

    NamePtr = wcsrchr( FullName, L'\\' );
    if ( NamePtr )
    {
        NamePtr++;
    }
    else
    {
        NamePtr = PathPtr;
    }

    if ( !NamePtr )
    {
        return;
    }

    SaveChar = *NamePtr;
    *NamePtr = L'\0';
    wcscpy( Dir, PathPtr ); // Dir
    *NamePtr = SaveChar;

    ExtPtr = wcsrchr( NamePtr, L'.' );
    if ( ExtPtr )
    {
        SaveChar = *ExtPtr;
        *ExtPtr = L'\0';
    }

    wcscpy( Name, NamePtr ); // Name

    if ( ExtPtr )
    {
        wcscpy( Ext, ExtPtr+1 ); // Ext
        *ExtPtr = SaveChar;
    }

} // end of DfrgSplitPath

//
// This variables are static.
// Access (DfrgDoesNameMatch() call) is syncronized in caller.
//
WCHAR       FDrive[_MAX_DRIVE], FDir[_MAX_DIR], FName[_MAX_FNAME], FExt[_MAX_EXT];
WCHAR       MDrive[_MAX_DRIVE], MDir[_MAX_DIR], MName[_MAX_FNAME], MExt[_MAX_EXT];

BOOLEAN
DfrgDoesNameMatch( 
    PWCHAR      Name, 
    PWCHAR      Mask )
{
    PWCHAR  NamePtr = Name;


    if ( wcsstr( Name, L"\\??\\" ) )
    {
        NamePtr += 4;
    }

    _wcsupr( NamePtr );

#ifdef _DEBUGW
    if ( wcsstr( Name, L"BIN" ) )
    {
        int debug = 1;
    }
#endif

    if ( !wcscmp( NamePtr, Mask ) )
    {
        return TRUE;
    }

    DfrgSplitPath( NamePtr, FDrive, FDir, FName, FExt ); // _wsplitpath
    DfrgSplitPath( Mask, MDrive, MDir, MName, MExt );

    if ( wcscmp( FDrive, MDrive ) )
    {
        return FALSE;
    }

    size_t  NLength = wcslen( FDir );
    size_t  MLength = wcslen( MDir );

    if ( NLength < MLength )
    {
        return FALSE;
    }

    if ( wcsstr( FDir, MDir ) != FDir )
    {
        return FALSE;
    }

#ifdef _DEBUGW
    WCHAR   Mess[MAX_PATH*2];
    swprintf( Mess, L"   Name match '%ls' '%ls'", Name, Mask );
    OutMessage( Mess );
#endif

    return TRUE;

} // end of DfrgDoesNameMatch


int
DfrgBuildFileName( 
    PDEFRAG_JOB_CONTEXT         JobCtx, 
    PDEFRAG_FILE_CONTEXT        FileCtx,
    PUNICODE_STRING             UniName )
{
    int                         Result = ERROR_NOT_ENOUGH_MEMORY; //ERROR_INVALID_PARAMETER;


    __try
    {
#ifdef _DEBUGW
        WCHAR   Mess[MAX_PATH*2];
        swprintf( Mess, L"   Build full name for Id %016I64X '%ls' ", 
                FileCtx->FId.FileId, FileCtx->FName ? FileCtx->FName : L"" );
        OutMessage( Mess );
#endif

        if ( !UniAllocateAtLeast( UniName, MAX_PATH ) )
        {
            __leave;
        }

        //
        // Append drive name
        //
        if ( !UniInitUnicodeString( UniName, FileCtx->VolumeCtx->VName ) )
        {
            __leave;
        }

        int                     PathElementsNum = 0;
        PDEFRAG_FILE_CONTEXT    UpDirCtx;

        DfrgResolveFileParent( FileCtx->VolumeCtx, FileCtx );
        UpDirCtx = FileCtx->ParentDir;

        while( UpDirCtx )
        {
            PathElementsNum++;

            DfrgResolveFileParent( FileCtx->VolumeCtx, UpDirCtx );

            UpDirCtx = UpDirCtx->ParentDir;
        }

        if ( !UniAppend( UniName, L"\\" ) )
        {
            __leave;
        }

        for ( int i=PathElementsNum; i>0; i-- )
        {
            UpDirCtx = FileCtx;
            for ( int j=0; j<i; j++ )
            {
                UpDirCtx = UpDirCtx->ParentDir;
            }
            if ( UpDirCtx && UpDirCtx->FName && !(UpDirCtx->DefragFlags & DEFRAG_FILE_ROOT_DIR) )
            {
                if ( !UniAppend( UniName, UpDirCtx->FName ) )
                {
                    __leave;
                }
                if ( !UniAppend( UniName, L"\\" ) )
                {
                    __leave;
                }
            }
        }

        if ( FileCtx->FName )
        {
            if ( !UniAppend( UniName, FileCtx->FName ) )
            {
                __leave;
            }
        }

        Result = NO_ERROR;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        Result = GetExceptionCode();
        DislpayErrorMessage( L"DfrgBuildFileName(). Exception !!!", Result, ERR_OS );
    }


    if ( NO_ERROR != Result )
    {
        UniFreeUnicodeString( UniName );
    }


    return Result;

} // end of DfrgBuildFileName


int
DfrgGetFileName(
    PDEFRAG_JOB_CONTEXT         JobCtx, 
    PDEFRAG_FILE_CONTEXT        FileCtx,
    PUNICODE_STRING             UniName,
    PULONG                      Flags )
{
    int                         Result = NO_ERROR;


    __try
    {
        Result = DfrgBuildFileName( JobCtx, FileCtx, UniName );
        if ( NO_ERROR == Result )
        {
            __leave;
        }

        if ( !Flags && FileCtx->FName )
        {
            if ( UniInitUnicodeString( UniName, FileCtx->FName ) )
            {
                __leave;
            }
            Result = ERROR_NOT_ENOUGH_MEMORY;
        }
#if 0
        //
        // Query file Name
        //
        if ( NtfsDisk != JobCtx->VolumeCtx.DiskType )
        {
            __leave;
        }

        if ( NO_ERROR != DfrgOpenFile( FileCtx, TRUE, NULL ) ) )
        {
            FileCtx->DefragFlags |= (DEFRAG_FILE_UNMOVABLE | DEFRAG_FILE_LOCKED);
            __leave;
        }

        if ( NO_ERROR != DfrgQueryFileName( FileCtx, UniName ) )
        {
            __leave;
        }

        //
        // Append drive name
        //
        if ( !wcschr( WrkUniName.Buffer, L':' ) )
        {
            if ( !UniInitUnicodeString( &FileName, JobCtx->VolumeCtx.VName ) )
            {
                Result = ERROR_NOT_ENOUGH_MEMORY;
                __leave;
            }
        }

        if ( !UniAppend( &FileName, &WrkUniName ) )
        {
            Result = ERROR_NOT_ENOUGH_MEMORY;
            __leave;
        }
#endif
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        Result = GetExceptionCode();
        DislpayErrorMessage( L"DfrgGetFileName(). Exception !!!", Result, ERR_OS );
    }

    return Result;

} // end of DfrgGetFileName

//
// Files filters
//
BOOLEAN
IsFileExcluded( 
    PDEFRAG_JOB_CONTEXT     JobCtx, 
    PDEFRAG_FILE_CONTEXT    FileCtx )
{
    BOOLEAN                 Result = FALSE;

    UNICODE_STRING          FileName    = { 0 };
    //UNICODE_STRING          WrkUniName  = { 0 };

    BOOLEAN                 ResourceIsLocked = FALSE;


    __try
    {
        //if ( FileCtx )
        //{
        //    __leave;
        //}

        if ( (FileCtx->DefragFlags & DEFRAG_FILE_MOVEABILITY_CHECKED) )
        {
            if ( (FileCtx->DefragFlags & DEFRAG_FILE_EXCLUDED) )
            {
                Result = TRUE;
            }
            __leave;
        }
        
        Result = DfrgGetFileName( JobCtx, FileCtx, &FileName );
        if ( NO_ERROR != Result )
        {
            __leave;
        }

#ifdef _DEBUGW
        WCHAR   Mess[MAX_PATH];
        swprintf( Mess, L"   IsFileExcluded '%ls'", FileName.Buffer );
        OutMessage( Mess );
#endif

        //
        // Is file name match exclude list
        //
        LockSettings( &(JobCtx->Settings->SyncObject) );
        ResourceIsLocked  = TRUE;

        if ( !IsEmptyUnicodeString( &(JobCtx->Settings->ExcludedFiles) ) )
        {
            PWCHAR ExcludePathName = JobCtx->Settings->ExcludedFiles.Buffer;

            //
            // Iterate through all the items and find matching one
            //
            while ( *ExcludePathName )
            {
                if ( DfrgDoesNameMatch( FileName.Buffer, ExcludePathName ) )
                {
                    Result = TRUE;
                    break;
                }

                ExcludePathName = ExcludePathName + wcslen(ExcludePathName) + 1;
            }
        }

        FileCtx->DefragFlags |= DEFRAG_FILE_MOVEABILITY_CHECKED;

    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        int Result = GetExceptionCode();
        DislpayErrorMessage( L"IsFileExcluded(). Exception !!!", Result, ERR_OS );
    }


    if ( ResourceIsLocked )
    {
        UnlockSettings( &(JobCtx->Settings->SyncObject) );
    }

    UniFreeUnicodeString( &FileName );
    //UniFreeUnicodeString( &WrkUniName );

    DfrgCloseFile( FileCtx );


    return Result;

} // end of IsFileExcluded


void
DfrgResolveFileParent( 
    PDEFRAG_VOLUME_CONTEXT  VolumeCtx,
    PDEFRAG_FILE_CONTEXT    FileCtx )
{
    DEFRAG_FILE_CONTEXT     Template = { 0 };
    PBTR_NODE               ParentNode = NULL;

#ifdef _DEBUG
    WCHAR                   Mess[MAX_PATH*2];
#endif


    if ( FileCtx->ParentDir )
    {
        return;
    }

    if ( !(FileCtx->ParentDirId.FileId) )
    {
#ifdef _DEBUG
        if ( !(FileCtx->DefragFlags & DEFRAG_FILE_ROOT_DIR) ) // & FileCtx->FId.MftIndexLowPart != 5 )
        {
            swprintf( Mess, L"### DfrgResolveFileParent. '%ls' Id %016I64X has zero Parent", 
                FileCtx->FName ? FileCtx->FName : L"", FileCtx->FId.FileId );
            OutMessage( Mess );
        }
#endif
        return;
    }

    Template.FId = FileCtx->ParentDirId;

    ParentNode = FindNode( &(VolumeCtx->DirsTree), &Template, SortModeId, _FIND_EQ_ );
    if ( ParentNode )
    {
        FileCtx->ParentDir = (PDEFRAG_FILE_CONTEXT)ParentNode->Body;
    }
#ifdef _DEBUG
    else
    {
        swprintf( Mess, L"### DfrgResolveFileParent. Could not resolve Parent for '%ls' Id %016I64X", 
            FileCtx->FName ? FileCtx->FName : L"", FileCtx->FId.FileId );
        OutMessage( Mess );
    }
#endif


    //return Result;

} // end of DfrgResolveFileParent


#ifdef _DEBUG
    //#define _DEBUG_EXCl
#endif

int
DfrgResolveFileTree( 
    PDEFRAG_VOLUME_CONTEXT  VolumeCtx )
{
    int                     Result = NO_ERROR; // ERROR_INVALID_PARAMETER;

    PBTR_NODE               FileNode = NULL;
    PDEFRAG_FILE_CONTEXT    FileCtx = NULL;


    __try
    {
#ifdef _DEBUG
        OutMessage( L"DfrgResolveFileTree(). START" );
#endif
        //
        // Iterate files and resolve parent reference
        //
        FileNode = FindFirstNode( &(VolumeCtx->FilesTree), RIGHT_TO_LEFT );

        while ( FileNode )
        {
            FileCtx = (PDEFRAG_FILE_CONTEXT)FileNode->Body;

#ifdef _DEBUG_EXCl
            if ( IsFileExcluded( VolumeCtx->JobCtx, FileCtx ) )
            {
                break;
            }
#endif
            if ( (FileCtx->DefragFlags & DEFRAG_FILE_DIRECTORY) )
            {
                DfrgResolveFileParent( VolumeCtx, FileCtx );
            }

            FileNode = FindNextNode( &(VolumeCtx->FilesTree), RIGHT_TO_LEFT );
        }
#ifdef _DEBUG
        OutMessage( L"DfrgResolveFileTree(). DONE" );
#endif


    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        Result = GetExceptionCode();
        DislpayErrorMessage( L"DfrgResolveFileTree(). Exception !!!", Result, ERR_OS );
    }


    return Result;

} // end of DfrgResolveFileTree

