/*
    Defrag Engine Core

    Module name:

        EngineCore.cpp

    Abstract:

        Defrag Engine Core functions module. 
        Contains core defragmentation functions.

    $Header: /home/CVS_DEFRAG/Defrag/Src/Engine/EngineCore.cpp,v 1.17 2009/12/29 09:50:16 dimas Exp $
    $Log: EngineCore.cpp,v $
    Revision 1.17  2009/12/29 09:50:16  dimas
    Bug with sparsed files fragmentation detection fixed

    Revision 1.16  2009/12/25 13:48:53  dimas
    Debug artefacts removed

    Revision 1.15  2009/12/24 15:02:51  dimas
    Some cosmetic improvements

    Revision 1.14  2009/12/24 12:23:50  dimas
    Some bugs fixed

    Revision 1.13  2009/12/24 10:52:20  dimas
    Check against Exclude Files list implemented

    Revision 1.12  2009/12/23 13:01:47  dimas
    1. Full File Name processing implemented.
    2. Top 100 Fragmented functionality for all file list requests implemented.

    Revision 1.11  2009/12/21 17:00:13  dimas
    1. DEFRAG_CMD_GET_MOST_FRAGMENTED and similar requests implemented
    2. Basic Exclude files check added

    Revision 1.10  2009/12/17 10:27:20  dimas
    1. Defragmentation of FAT volumes in release build enabled
    2. Debug function GetDisplayBlockCounts() implemented

    Revision 1.9  2009/12/16 14:13:34  dimas
    DEFRAG_CMD_GET_FILE_INFO request implemented

    Revision 1.8  2009/12/15 16:06:40  dimas
    OP_REMOVE_ALL pseudo operation on settings lists added

    Revision 1.7  2009/12/03 13:01:06  dimas
    Cluster Inspector implemented

    Revision 1.6  2009/12/02 14:42:16  dimas
    App/Service interaction improved

    Revision 1.5  2009/12/01 13:29:52  dimas
    Smart modes improved.

    Revision 1.4  2009/11/30 12:48:36  dimas
    1. Smart modes improved.
    2. FAT processing improved.

    Revision 1.3  2009/11/26 15:56:54  dimas
    1. Smart modes improved.
    2. File names support improved.

    Revision 1.2  2009/11/24 15:15:10  dimas
    Bugs 5.2, 6.*, 15.14 fixed

    Revision 1.1  2009/11/24 14:52:15  dimas
    no message

*/

#include "stdafx.h"


#ifdef _DEBUG

void
PrintJobInfo( 
    IN  PDEFRAG_JOB_CONTEXT  Job )
{

    WCHAR       Mess[MAX_PATH/2];


    swprintf( Mess, L"DfrgDoJob. '%ls' ", Job->VolumeCtx.VName ); 

    switch ( Job->Type )
    {
    case DEFRAG_JOB_ANALIZE_ONLY:
        wcscat( Mess, L"ANALIZE" );
        break;

    case DEFRAG_JOB_SIMPLE_DEFRAG:
        wcscat( Mess, L"SIMPLE_DEFRAG" );
        break;

    case DEFRAG_JOB_DEBUG1:
        wcscat( Mess, L"DEBUG1 == " );
    case DEFRAG_JOB_FREE_SPACE:
        wcscat( Mess, L"FREE_SPACE" );
        break;

    case DEFRAG_JOB_SMART_BY_NAME:
        wcscat( Mess, L"SMART_BY_NAME" );
        break;

    case DEFRAG_JOB_SMART_BY_ACCESS:
        wcscat( Mess, L"SMART_BY_ACCESS" );
        break;

    case DEFRAG_JOB_SMART_BY_MODIFY:
        wcscat( Mess, L"SMART_BY_MODIFY" );
        break;

    case DEFRAG_JOB_SMART_BY_SIZE:
        wcscat( Mess, L"SMART_BY_SIZE" );
        break;

    case DEFRAG_JOB_SMART_BY_CREATE:
        wcscat( Mess, L"SMART_BY_CREATE" );
        break;

    case DEFRAG_JOB_FORCE_TOGETHER:
        wcscat( Mess, L"FORCE_TOGETHER" );
        break;

    case DEFRAG_JOB_EMPTY:
        wcscat( Mess, L"EMPTY/TERMINATED" );
        break;

    default:
        wcscat( Mess, L"UNKNOWN" );
        break;

    }

    wcscat( Mess, L"\n" );
    OutMessage( Mess );
    GetInput();

} // end of PrintJobInfo

#endif // _DEBUG


BOOLEAN
ShouldContinue( 
    IN  PDEFRAG_JOB_CONTEXT  JobCtx )
{
    BOOLEAN     Result = TRUE;


    if ( (JobCtx->Status & DEFRAG_JOB_FLAG_PAUSED) && JobCtx->ResumeEvent )
    {
        WaitForSingleObject( JobCtx->ResumeEvent, INFINITE );
    }

    if ( (JobCtx->Status & DEFRAG_JOB_FLAG_STOPPED) )
    {
        Result = FALSE;
    }


    return Result;

} // end of ShouldContinue


int
DfrgProcessVolume(
    IN  PDEFRAG_VOLUME_CONTEXT  VolumeCtx )
{
    int                         Result = NO_ERROR;

    IN  PDEFRAG_JOB_CONTEXT     JobCtx = VolumeCtx->JobCtx;

//#ifdef _DEBUG
//    WCHAR   Mess[MAX_PATH];
//#endif


    __try
    {
        //
        // Adjust Sort mode
        //
        switch ( JobCtx->Type )
        {
        case DEFRAG_JOB_SMART_BY_NAME:

            JobCtx->SortMode = SortModeName; 
            //JobCtx->SortMode = SortModePath; 
            break;

        case DEFRAG_JOB_SMART_BY_ACCESS:

            JobCtx->SortMode = SortModeUsedDate; 
            break;

        case DEFRAG_JOB_SMART_BY_MODIFY:

            JobCtx->SortMode = SortModeModifiedDate; 
            break;

        case DEFRAG_JOB_SMART_BY_CREATE:

            JobCtx->SortMode = SortModeCreatedDate;
            break;

        case DEFRAG_JOB_FORCE_TOGETHER:

            //JobCtx->SortMode = SortModeGroup;
            break;

        //case DEFRAG_JOB_EMPTY:
        //case DEFRAG_JOB_SIMPLE_DEFRAG:
        //case DEFRAG_JOB_DEBUG1:
        //case DEFRAG_JOB_FREE_SPACE:
        //case DEFRAG_JOB_ANALIZE_ONLY:
        //case DEFRAG_JOB_SMART_BY_SIZE:
        //default:

        //    JobCtx->SortMode = SortModeSize;
        //    break;
        }

#ifdef _DEBUG
        PrintJobInfo( JobCtx );
#endif

        //
        // Mark progress
        //
        JobCtx->ProcessedObjectsNum.QuadPart = 0;
        JobCtx->ObjectsToProcessNum.QuadPart = 
            VolumeCtx->Statistic.FilesNum.QuadPart + VolumeCtx->GapsTree.NodeCnt.QuadPart;

        //
        // Start required defrag mode
        //
        switch ( JobCtx->Type )
        {
        case DEFRAG_JOB_SIMPLE_DEFRAG:

            Result = DfrgSimple( VolumeCtx );
            break;

        case DEFRAG_JOB_DEBUG1:
        case DEFRAG_JOB_FREE_SPACE:

            if ( (JobCtx->Options & DEFRAG_JOB_OPTION_BE_AGRESSIVE) )
            {
#ifdef _DEBUG
                OutMessage( L"DfrgFreeSpaceAgressive\n" );
#endif
                Result = DfrgFreeSpaceAgressive( VolumeCtx );
            }
            else
            {
                Result = DfrgFreeSpace( VolumeCtx );
            }

            break;

        case DEFRAG_JOB_SMART_BY_NAME:
        case DEFRAG_JOB_SMART_BY_ACCESS:
        case DEFRAG_JOB_SMART_BY_MODIFY:
        case DEFRAG_JOB_SMART_BY_SIZE:
        case DEFRAG_JOB_SMART_BY_CREATE:

            //Result = DfrgSmart( VolumeCtx );
            Result = DfrgSuperSmart( VolumeCtx );
            break;

        case DEFRAG_JOB_FORCE_TOGETHER:
        
            Result = DfrgForceTogether( VolumeCtx );
            break;

        case DEFRAG_JOB_EMPTY:
        default:
            //
            // Terminate
            //
            Result = DFRG_ERROR_ABORTED;
            break;
        }

        if ( NO_ERROR != Result )
        {
            __leave;
        }

    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        Result = GetExceptionCode();
        DislpayErrorMessage( L"DfrgProcessVolume(). Exception !!!", Result, ERR_OS );
    }

    //
    // Update Job state
    //
    VolumeCtx->JobCtx->Status |= DEFRAG_JOB_FLAG_PROCESSED;
    if ( NO_ERROR == Result )
    {
        VolumeCtx->JobCtx->Status &= ~DEFRAG_JOB_FLAG_ERROR;
    }
    else if ( DFRG_ERROR_ABORTED != Result )
    {
        VolumeCtx->JobCtx->Status |= DEFRAG_JOB_FLAG_ERROR;
    }
    VolumeCtx->JobCtx->Result = Result;

    //
    // Mark progress
    //
    VolumeCtx->JobCtx->ProcessedObjectsNum = VolumeCtx->JobCtx->ObjectsToProcessNum;


    return Result;

} // end of DfrgProcessVolume


BOOLEAN
DfrgAddFileToLists( 
    PDEFRAG_VOLUME_CONTEXT      VolumeCtx,
    PDEFRAG_FILE_CONTEXT        FileCtx )
{
    BOOLEAN                     Result = TRUE;

#ifdef _DEBUGW
    WCHAR                       Mess[MAX_PATH];
    LARGE_INTEGER               Tik1, Tik2, Frequency = { 0 };
    DWORD                       DeltaN, DeltaP = 0;

    QueryPerformanceFrequency( &Frequency );
    Frequency.QuadPart /= 1000000; // to simplify convertion from tiks to mksec 
    QueryPerformanceCounter( &Tik1 );
#endif

__try
{
    //
    // Add file to files tree
    //
    if ( !AddNode( &(VolumeCtx->FilesTree), FileCtx ) )
    {
        //Result = ERROR_NOT_ENOUGH_MEMORY;
        Result = FALSE;
        OutMessage( L"DfrgAddFileToLists(). Add File Node - ERROR_NOT_ENOUGH_MEMORY" );
        return Result;
    }

    //
    // Additionally add dir to dirs tree
    //
    if ( (FileCtx->DefragFlags & DEFRAG_FILE_DIRECTORY) )
    {
        if ( !AddNode( &(VolumeCtx->DirsTree), FileCtx ) )
        {
            //Result = ERROR_NOT_ENOUGH_MEMORY;
            Result = FALSE;
            OutMessage( L"DfrgAddFileToLists(). Add File Node - ERROR_NOT_ENOUGH_MEMORY" );
            return Result;
        }
    }

    //
    // Add file's extents to extents tree
    //
    PDEFRAG_STREAM_CONTEXT      StreamCtx;
    PDEFRAG_FILES_EXTENT        Extent;
    LARGE_INTEGER               StartingVcn = { 0 }, ExtentLength = { 0 };

    //
    // Iterate through all file's streams
    //
    for ( StreamCtx = FileCtx->FirstStream; StreamCtx && StreamCtx->ClustersMap; StreamCtx = StreamCtx->NextStream )
    {
        //
        // Iterate through cluster map
        //
        StartingVcn = StreamCtx->ClustersMap->StartingVcn;

        ExtentLength.QuadPart = 0;

        for ( DWORD i=0; i<StreamCtx->ClustersMap->ExtentCount; 
            StartingVcn = StreamCtx->ClustersMap->Extents[i].NextVcn, i++ )
        {
            if ( VIRTUAL_LCN == StreamCtx->ClustersMap->Extents[i].Lcn.QuadPart ) 
            {
                //
                // Skip virtual (sparce) extent
                //
                ExtentLength.QuadPart = 0;
                continue;
            }

            if ( i && ExtentLength.QuadPart && // VIRTUAL_LCN != StreamCtx->ClustersMap->Extents[i-1].Lcn.QuadPart && 
                StreamCtx->ClustersMap->Extents[i].Lcn.QuadPart == 
                StreamCtx->ClustersMap->Extents[i-1].Lcn.QuadPart + ExtentLength.QuadPart )
            {
                //
                // This is continuation of previous extent - join them
                //
                Extent->Length.QuadPart += ExtentLength.QuadPart;
            }
            else
            {
                Extent = (PDEFRAG_FILES_EXTENT) malloc( sizeof(DEFRAG_FILES_EXTENT) );
                if ( !Extent )
                {
                    //Result = ERROR_NOT_ENOUGH_MEMORY;
                    Result = FALSE;
                    OutMessage( L"DfrgAddFileToLists(). malloc Extent - ERROR_NOT_ENOUGH_MEMORY" );
                    return Result;
                }

                Extent->StartLcn = StreamCtx->ClustersMap->Extents[i].Lcn;
                Extent->Length.QuadPart = StreamCtx->ClustersMap->Extents[i].NextVcn.QuadPart - StartingVcn.QuadPart;
                Extent->StartVcn = StartingVcn;
                Extent->StreamCtx = StreamCtx;

                if ( !AddNode( &(VolumeCtx->ExtentsTree), Extent ) )
                {   
                    //Result = ERROR_NOT_ENOUGH_MEMORY;
                    Result = FALSE;
                    OutMessage( L"DfrgAddFileToLists(). Add Extent Node - ERROR_NOT_ENOUGH_MEMORY" );
                    return Result;
                }
            }

            ExtentLength.QuadPart = StreamCtx->ClustersMap->Extents[i].NextVcn.QuadPart - StartingVcn.QuadPart;

        } // for ( i - extents

#ifdef _DEBUG
        //
        // Some random and not so checks
        //
        if ( FileCtx->FName && 
            ( //!wcscmp( FileCtx->FName, L"$Secure" ) || 
            !(VolumeCtx->ExtentsTree.NodeCnt.QuadPart % 1000) ) )
        {
            PRETRIEVAL_POINTERS_BUFFER  NewStreamClustersMap = NULL;
            WCHAR                       Mess[MAX_PATH*2];
            int ResultD = 0;

            ResultD  = DfrgOpenFileStream( StreamCtx );
            if ( NO_ERROR == Result )
            {
                ResultD = DfrgGetStreamClustersMap( FileCtx, &NewStreamClustersMap );
                if ( NO_ERROR == ResultD )
                {
                    if ( CheckStreamMaps( StreamCtx, NewStreamClustersMap, TRUE ) )
                    {
                        swprintf( Mess, L"### Wrong ClustersMap for '%ls'", FileCtx->FName );
                        OutMessage( Mess );
                    }
                }
                else
                {
                    swprintf( Mess, L"### DfrgGetStreamClustersMap() error for '%ls'", FileCtx->FName );
                    OutMessage( Mess );
                }
            }

            DfrgCloseFile( FileCtx );

            if ( NewStreamClustersMap )
            {
                free( NewStreamClustersMap );
            }
        }
        //swprintf( Mess, L"DeltaN %u > DeltaP  %u (msec)", DeltaN, DeltaP );
        //OutMessage( Mess );
#endif


    } // for ( Stream

}
__except(EXCEPTION_EXECUTE_HANDLER)
{
    int res = GetExceptionCode();
    DislpayErrorMessage( L"DfrgAddFileToLists(). Exception !!!", res, ERR_OS );
}


#ifdef _DEBUGW
    QueryPerformanceCounter( &Tik2 );
    DeltaN = (ULONG)((Tik2.QuadPart - Tik1.QuadPart) / Frequency.QuadPart);
    if ( DeltaN > DeltaP )
    {
        swprintf( Mess, L"DeltaN %u > DeltaP  %u (msec)", DeltaN, DeltaP );
        OutMessage( Mess );
        //GetInput();
        DeltaP = DeltaN;
    }
#endif

    return Result;

} // end of DfrgAddFileToLists

//
// Check attributes, set flags and update display
//
BOOLEAN
DfrgMarkupFile( 
    PDEFRAG_JOB_CONTEXT     JobCtx, 
    PDEFRAG_FILE_CONTEXT    FileCtx )
{
    BOOLEAN                 Result = TRUE;

#ifdef _DEBUG
    WCHAR       Mess[MAX_PATH] = L"";
#endif

__try
{

    if ( NtfsDisk != JobCtx->VolumeCtx.DiskType && 
         (!wcscmp( FileCtx->FName, L"." ) ||
          !wcscmp( FileCtx->FName, L".." )) )
    {
        return FALSE;
    }

    //
    // Mark attributes
    //
    if ( (FileCtx->Attributes & FILE_ATTRIBUTE_DIRECTORY) )
    {
        FileCtx->DefragFlags |= (DEFRAG_FILE_DIRECTORY);

        if ( !(JobCtx->Options & DEFRAG_JOB_OPTION_BOOT_DEFRAG) )
        {
            if ( JobCtx->VolumeCtx.DiskType != NtfsDisk ) // FatDirectory
            {
                FileCtx->DefragFlags |= (DEFRAG_FILE_UNMOVABLE);
            }
        }
    }

    if ( (FileCtx->Attributes & FILE_ATTRIBUTE_COMPRESSED) )
    {
        FileCtx->DefragFlags |= (DEFRAG_FILE_COMPRESSED);
    }

    if ( (FileCtx->Attributes & FILE_ATTRIBUTE_ENCRYPTED) )
    {
        FileCtx->DefragFlags |= (DEFRAG_FILE_ENCRYPTED);
    }

    if ( (FileCtx->Attributes & FILE_ATTRIBUTE_SPARSE_FILE) )
    {
        FileCtx->DefragFlags |= (DEFRAG_FILE_SPARSED);
    }

    if ( (FileCtx->Attributes & FILE_ATTRIBUTE_OFFLINE) )
    {
        FileCtx->DefragFlags |= (DEFRAG_FILE_OFFLINE | DEFRAG_FILE_EXCLUDED);
    }

    if ( (FileCtx->Attributes & FILE_ATTRIBUTE_SYSTEM) )
    {
        FileCtx->DefragFlags |= (DEFRAG_FILE_SYSTEM);
    }

    if ( (FileCtx->Attributes & FILE_ATTRIBUTE_READONLY) )
    {
        FileCtx->DefragFlags |= (DEFRAG_FILE_READONLY);
    }

    if ( (FileCtx->Attributes & FILE_ATTRIBUTE_TEMPORARY) )
    {
        FileCtx->DefragFlags |= (DEFRAG_FILE_TEMPORARY | DEFRAG_FILE_EXCLUDED);
    }

    //
    // Mark Special files
    //
    if ( NtfsDisk == JobCtx->VolumeCtx.DiskType )
    {
        MFT_INODE_REFERENCE     FileMftIndex;
        FileMftIndex.FileId = FileCtx->FId.FileId;
        FileMftIndex.SequenceNumber = 0;

        if ( FileMftIndex.FileId < (ULONGLONG)32 )
        {
            FileCtx->DefragFlags |= (DEFRAG_FILE_SYSTEM | DEFRAG_FILE_META);

            if ( !FileMftIndex.FileId )
            {
                //
                // $Mft
                //
                FileCtx->DefragFlags |= (DEFRAG_FILE_MFT);
            }
        }
    }

    //
    // TODO: \pagefile.sys, \hiberfil.sys
    //       \[bootfiles]
    //
    if ( (FileCtx->DefragFlags & DEFRAG_FILE_SYSTEM) && FileCtx->FName )
    {
        if ( !wcscmp( FileCtx->FName, L"pagefile.sys" ) )
        {
            FileCtx->DefragFlags |= (DEFRAG_FILE_PAGING);
        }
        else if ( !wcscmp( FileCtx->FName, L"hiberfil.sys" ) )
        {
            FileCtx->DefragFlags |= (DEFRAG_FILE_HIBER);
        }
        else if ( 
          JobCtx->VolumeCtx.DiskType != NtfsDisk &&         // FAT 
            (!_wcsicmp( FileCtx->FName, L"MSDOS.SYS" ) || 
             //!_wcsicmp( FileCtx->FName, L"COMMAND.COM" ) ||
             //!_wcsicmp( FileCtx->FName, L"CONFIG.SYS" ) || 
             //!_wcsicmp( FileCtx->FName, L"AUTOEXEC.BAT" ) ||
             !_wcsicmp( FileCtx->FName, L"IO.SYS" )) || 
          JobCtx->VolumeCtx.DiskType == NtfsDisk &&         // NTFS
            (!_wcsicmp( FileCtx->FName, L"NTDETECT.COM" ) || 
             //!_wcsicmp( FileCtx->FName, L"boot.ini" ) || 
             !_wcsicmp( FileCtx->FName, L"ntldr" )) )
        {
            FileCtx->DefragFlags |= (DEFRAG_FILE_BOOT);         //  | DEFRAG_FILE_META
            FileCtx->DefragFlags |= (DEFRAG_FILE_UNMOVABLE | DEFRAG_FILE_EXCLUDED);    //  | DEFRAG_FILE_EXCLUDED
        }
    }

    //
    // Mark recently modified
    //
    if ( JobCtx->StartTime < FileCtx->ChangeTime.QuadPart + DFRG_RECENTLY_MODIFIED_VALUE ) 
    {
        FileCtx->DefragFlags2 |= (DEFRAG_FILE_RECENTLY_MODIFIED);
    }
    else if ( JobCtx->StartTime > FileCtx->ChangeTime.QuadPart + DFRG_RARELY_MODIFIED_VALUE ) 
    {
        FileCtx->DefragFlags2 |= (DEFRAG_FILE_RARELY_MODIFIED);
    }

    //
    // Mark excluded & unmovable
    //
#if 0 
    //
    // Mark metadata as Excluded
    //
    if ( ( FileCtx->DefragFlags & (DEFRAG_FILE_SYSTEM | DEFRAG_FILE_META) ) )
    {
        if ( !(JobCtx->Options & DEFRAG_JOB_OPTION_BOOT_DEFRAG) )
        {
            FileCtx->DefragFlags |= (DEFRAG_FILE_EXCLUDED); // | DEFRAG_FILE_UNMOVABLE 
        }
    }
#endif

    if ( NtfsDisk == JobCtx->VolumeCtx.DiskType && 
        !(FileCtx->TotalClustersNum.QuadPart) )
    {
        FileCtx->DefragFlags |= (DEFRAG_FILE_EXCLUDED);

        //if ( !(FileCtx->Attributes & FILE_ATTRIBUTE_DIRECTORY) )
        if ( !(FileCtx->DefragFlags & DEFRAG_FILE_DIRECTORY) )
        {
            Result = FALSE;
        }
    }

/*
    //
    // This check was moved to GetNextMoveableFile (for Simple Defrag).
    // TODO. What about other modes ?
    //
    if ( IsFileExcluded( JobCtx, FileCtx ) )
    {
        FileCtx->DefragFlags |= (DEFRAG_FILE_EXCLUDED);
    }
*/
    //
    // Mark fragmented files
    //
    //FileCtx->DefragFlags |= FileCtx->StreamNum ? DEFRAG_FILE_STREAMED : 0;
    //
    //if ( IsStreamFragmented( FileCtx->DefaultStream ) )
    if ( IsFileFragmented( FileCtx ) )
    {
        FileCtx->DefragFlags |= DEFRAG_FILE_FRAGMENTED;
    }

    //
    // Update Statistic
    //
    UpdateFilesStatistic( &(JobCtx->VolumeCtx), FileCtx );


    //
    // Update Display
    //
    UpdateDisplayForStream( FileCtx, NULL, INCL_FLAGS_COUNTS );

}
__except(EXCEPTION_EXECUTE_HANDLER)
{
    int res = GetExceptionCode();
    DislpayErrorMessage( L"DfrgMarkupFile(). Exception !!!", res, ERR_OS );
#ifdef _DEBUG
    swprintf( Mess, L"   FileCtx %p : '%ls'", 
        FileCtx, FileCtx->FName ? FileCtx->FName : L"NULL" );
    OutMessage( Mess );
#endif
}


    return Result;

} // end of DfrgMarkupFile

//
// Get Volume Files List
//
int
DfrgGetFileList( 
    PDEFRAG_VOLUME_CONTEXT  VolumeCtx )
{
    int                     Result = ERROR_INVALID_PARAMETER;

    BTR_TREE_METHODS        Methods = { 0 };


#ifdef _DEBUG
    if ( !VolumeCtx )
    {
        return Result;
    }
#endif

    __try
    {
        //
        // Init dirs tree
        //
        Methods.FreeBody        = FreeFileCtx;
        Methods.CompareItems    = CompareFileCtx;
        //Methods.CopyBody        = CopyFileCtx;
        //Methods.PrintBody       = PrintFileCtx;

        InitTree( 
            &(VolumeCtx->DirsTree), 
            &Methods, 
            SortModeId ); // SortModeGroup

        //
        // Init files tree
        //
        Methods.FreeBody        = FreeFileCtx;
        Methods.CompareItems    = CompareFileCtx;
        //Methods.CopyBody        = CopyFileCtx;
        //Methods.PrintBody       = PrintFileCtx;

        InitTree( 
            &(VolumeCtx->FilesTree), 
            &Methods, 
            VolumeCtx->JobCtx->SortMode ); //SortModeName );

        //
        // Init extents tree
        //
        Methods.FreeBody        = NULL; // use simple free
        Methods.CompareItems    = CompareGaps;
        //Methods.CopyBody        = NULL;
        //Methods.PrintBody       = NULL;

        InitTree( 
            &(VolumeCtx->ExtentsTree), 
            &Methods, 
            SortModeLcn );

        //
        // get files list
        //
        if ( NtfsDisk == VolumeCtx->DiskType )
        {
            Result = NtfsGetFileList( VolumeCtx ); 
            if ( NO_ERROR != Result )
            {
                __leave;
            }

#ifdef _DEBUG_EXCl
            DfrgResolveFileTree( VolumeCtx ); 
            //if ( NO_ERROR != Result )
            //{
            //    __leave;
            //}
#endif

        }
        else if ( Fat12Disk == VolumeCtx->DiskType ||
                  Fat16Disk == VolumeCtx->DiskType ||
                  Fat32Disk == VolumeCtx->DiskType )
        {
            Result = FatGetFileList( VolumeCtx ); 
            if ( NO_ERROR != Result )
            {
                __leave;
            }
        }
        else
        {
#if 0
            Result = RawGetFileList( VolumeCtx ); 
            if ( NO_ERROR != Result )
            {
                __leave;
            }
#endif
            __leave;
        }

#ifdef _DEBUGW
    OutMessage( L"DfrgGetFileList OK" );
#endif

    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        Result = GetExceptionCode();
        DislpayErrorMessage( L"DfrgGetFileList(). Exception !!!", Result, ERR_OS );
    }

#ifdef _DEBUGW
    OutMessage( L"DfrgGetFileList exit" );
#endif

    return Result;

} // end of DfrgGetFileList

//
// Analyze volume 
//
int
DfrgAnalyzeVolume( 
    PDEFRAG_VOLUME_CONTEXT  VolumeCtx ) 
{
    int                         Result = ERROR_INVALID_PARAMETER;


    if ( !VolumeCtx )
    {
        return Result;
    }

    __try
    {

        //
        // Mark progress
        // KeQuerySystemTime()
        //
#ifndef DEFRAG_NATIVE_APP
        if ( (DEFRAG_JOB_OPTION_SHOW_PROGRESS & VolumeCtx->JobCtx->Options) )
        {
            SYSTEMTIME  SystemTime;
            GetSystemTime( &SystemTime );
            SystemTimeToFileTime( &SystemTime, (FILETIME*)&(VolumeCtx->JobCtx->StartTime) );
        }
#endif

#ifdef _DEBUG
        PrintJobInfo( VolumeCtx->JobCtx );
#endif

        //
        // Get File list
        //
        Result = DfrgGetFileList( VolumeCtx );
        if ( NO_ERROR != Result )
        {
            __leave;
        }

#ifdef _DEBUG
        OutMessage( L"\nDirectories" );
        PrintFileList( VolumeCtx, &(VolumeCtx->DirsTree), (Fat12Disk == VolumeCtx->DiskType), 
            (DEFRAG_FILE_FRAGMENTED | DEFRAG_FILE_UNMOVABLE | DEFRAG_FILE_META | DEFRAG_FILE_LOCKED) );

        if ( Fat12Disk == VolumeCtx->DiskType )
        {
            OutMessage( L"\nExtentes" );
            PrintExtentList( VolumeCtx, 0, VolumeCtx->NtfsData.TotalClusters.QuadPart, TRUE );
        }
#endif
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        Result = GetExceptionCode();
        DislpayErrorMessage( L"DfrgAnalyzeVolume(). Exception !!!", Result, ERR_OS );
    }

    if ( NO_ERROR == Result )
    {
        VolumeCtx->JobCtx->Status |= DEFRAG_JOB_FLAG_ANALIZED;
        VolumeCtx->JobCtx->Status &= ~DEFRAG_JOB_FLAG_ERROR;
    }
    else if ( DFRG_ERROR_ABORTED != Result )
    {
        VolumeCtx->JobCtx->Status |= DEFRAG_JOB_FLAG_ERROR;
    }

    VolumeCtx->JobCtx->Result = Result;

    //
    // Mark progress
    //
    VolumeCtx->JobCtx->ProcessedObjectsNum = VolumeCtx->JobCtx->ObjectsToProcessNum;


    return Result;

} // end of DfrgAnalyzeVolume

//
// Init volume, Get & Analyze volume basic info
//
int
DfrgInitVolumeCtx( 
    PDEFRAG_VOLUME_CONTEXT  VolumeCtx ) 
{
    int                     Result = ERROR_INVALID_PARAMETER;

#ifdef _DEBUG
    //WCHAR                   Mess[MAX_PATH];
#endif


    if ( !VolumeCtx )
    {
        return Result;
    }

    __try
    {
        if ( !VolumeCtx->VHandle || INVALID_HANDLE_VALUE == VolumeCtx->VHandle )
        {
            Result = DfrgOpenVolume( VolumeCtx );
            if ( NO_ERROR != Result )
            {
                __leave;
            }
        }

        //
        // Get Volume information
        //
        Result = DfrgGetVolumeInfo( VolumeCtx );
        if ( NO_ERROR != Result )
        {
            __leave;
        }

        //
        // Get Volume bitmap
        //
        Result = DfrgGetVolumeSpaceMap( VolumeCtx );
        if ( NO_ERROR != Result )
        {
            __leave;
        }

#ifdef _DEBUG
        //GetInput();
#endif

    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        Result = GetExceptionCode();
        DislpayErrorMessage( L"DfrgInitVolumeCtx(). Exception !!!", Result, ERR_OS );
    }


    return Result;

} // end of DfrgInitVolumeCtx

//
// Allocate memory for Job control structure 
// and init it with predefined values
// TODO: IN    Settings ( Min/MaxFileSize, etc.)
//
int
DfrgInitJobCtx( 
    OUT PDEFRAG_JOB_CONTEXT     *Job,
    PWCHAR                      DriveName, 
    int                         Mode )
{
    int                     Result = NO_ERROR;
    PDEFRAG_JOB_CONTEXT     JobLocal = NULL;


    if ( !Job  )
    {
        Result = ERROR_INVALID_PARAMETER;
        return Result;
    }

    __try
    {
        *Job = NULL;

        if ( !DriveName || !(*DriveName) )
        {
            Result = ERROR_INVALID_PARAMETER;
            __leave;
        }

        JobLocal = (PDEFRAG_JOB_CONTEXT) malloc( sizeof(DEFRAG_JOB_CONTEXT) );
        if ( !JobLocal ) 
        {
            Result = ERROR_NOT_ENOUGH_MEMORY;
            __leave;
        }

        //
        // Init zero context
        //
        memset( JobLocal, 0, sizeof(DEFRAG_JOB_CONTEXT) );

        JobLocal->VolumeCtx.Statistic.FragmentedFilesFragmentationRatio = 0.0;
        JobLocal->VolumeCtx.Statistic.BootFilesFragmentationRatio = 0.0;
        JobLocal->VolumeCtx.Statistic.PagefileFilesFragmentationRatio = 0.0;
        JobLocal->VolumeCtx.Statistic.MftFragmentationRatio = 0.0;
        JobLocal->VolumeCtx.Statistic.MetadataFilesFragmentationRatio = 0.0;
        JobLocal->VolumeCtx.Statistic.FreeSpaceFragmentationRatio = 0.0;

        JobLocal->ResumeEvent = CreateEvent( NULL, TRUE, TRUE, NULL );

        InitializeCriticalSection( &(JobLocal->JobSync) );

        JobLocal->Id = (DWORD)((ULONG_PTR)JobLocal);

        JobLocal->Options |= DEFRAG_JOB_OPTION_SHOW_PROGRESS;



        if ( Mode >= 0 && Mode < DEFRAG_JOB_MAX ) 
        {
            JobLocal->Type = Mode;
        }
        else
        {
            JobLocal->Type = DEFRAG_JOB_DEBUG1;
        }

        //JobLocal->MinFileSize.QuadPart      = FILE_MIN_SIZE;
        //JobLocal->MaxFileSize.QuadPart      = FILE_MAX_SIZE;
        //JobLocal->MaxFileExtent.QuadPart    = FILE_MAX_FRAGMENT;

        JobLocal->SortMode                  = SortModeSize;

        switch ( JobLocal->Type )
        {
        case DEFRAG_JOB_SIMPLE_DEFRAG:

            //JobLocal->MinFileSize.QuadPart  = 2;

        case DEFRAG_JOB_DEBUG1:
        case DEFRAG_JOB_FREE_SPACE:
        case DEFRAG_JOB_ANALIZE_ONLY:
        case DEFRAG_JOB_SMART_BY_SIZE:

            JobLocal->SortMode = SortModeSize;
            break;

        case DEFRAG_JOB_SMART_BY_NAME:

            JobLocal->SortMode = SortModeName; 
            //JobLocal->SortMode = SortModePath; 
            break;

        case DEFRAG_JOB_SMART_BY_ACCESS:

            JobLocal->SortMode = SortModeUsedDate; 
            break;

        case DEFRAG_JOB_SMART_BY_MODIFY:

            JobLocal->SortMode = SortModeModifiedDate; 
            break;

        case DEFRAG_JOB_SMART_BY_CREATE:

            JobLocal->SortMode = SortModeCreatedDate;
            break;

        case DEFRAG_JOB_FORCE_TOGETHER:

            JobLocal->SortMode = SortModeGroup;
            break;

        case DEFRAG_JOB_EMPTY:
        default:
            //
            // Terminate
            //
            Result = DFRG_ERROR_ABORTED;
            break;
        }

        //JobLocal->MinGapSize = JobLocal->MinFileSize;


        //
        // Init Volume context. Job / Volume have one to one relation
        //
        PDEFRAG_VOLUME_CONTEXT      VolumeCtx = NULL;

        VolumeCtx = &(JobLocal->VolumeCtx);

        VolumeCtx->JobCtx = JobLocal;

        if ( L'\\' != *DriveName )
        {
            wcscpy( VolumeCtx->VName, NATIVE_NAME_PREFIX );
            wcscat( VolumeCtx->VName, DriveName );
            if ( L':' != *(VolumeCtx->VName+wcslen(VolumeCtx->VName)-1) )
            {
                wcscat( VolumeCtx->VName, L":" );
            }
        }
        else
        {
            wcscat( VolumeCtx->VName, DriveName );
        }

        //
        // Allocate basic work buffers.
        //
        VolumeCtx->FileRecordsBuffer            = (PBYTE) malloc( FILE_RECS_BUFFER_SIZE ); // MftSize 
        //VolumeCtx->BitMapBuffer         = (PBYTE) malloc( BITMAP_BUFFER_SIZE );
        VolumeCtx->ClustersMapBuffer    = (PBYTE) malloc( CLUSTER_MAP_BUFFER_SIZE );
        
        if ( !VolumeCtx->FileRecordsBuffer || 
            //!VolumeCtx->BitMapBuffer ||
            !VolumeCtx->ClustersMapBuffer )
        {
            Result = ERROR_NOT_ENOUGH_MEMORY;
            __leave;
        }

        //VolumeCtx->BitMapLength.QuadPart = BUFFER_ITEMS_NUM;

        //
        // Init Volume - Open & Get Basic Volume information
        //
        Result = DfrgInitVolumeCtx( VolumeCtx );
        if ( NO_ERROR != Result )
        {
            __leave;
        }

    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        Result = GetExceptionCode();
        DislpayErrorMessage( L"DfrgInitJobCtx(). Exception !!!", Result, ERR_OS );
    }


    if ( JobLocal )
    {
        JobLocal->Status |= DEFRAG_JOB_FLAG_INITIALIZED;
        JobLocal->Result = Result;
    }

    *Job = JobLocal;


    return Result;

} // end of DfrgInitJobCtx


#ifdef _DEBUG
    #define _DEBUGS
#else
    //#define _DEBUG
    //#define _DEBUGS
    //#define _DEBUGSS
#endif

//
// Free Volume control structures
//
void
DfrgReleaseVolumeCtx ( 
    IN  PDEFRAG_VOLUME_CONTEXT     VolumeCtx )
{

#ifdef _DEBUGS
    WCHAR       Mess[MAX_PATH];
    swprintf( Mess, L"         DfrgReleaseVolumeCtx %08X", VolumeCtx );
    OutMessage( Mess );
#endif

    __try
    {
        if ( VolumeCtx )
        {
            if ( VolumeCtx->FileRecordsBuffer )
            {
                free( VolumeCtx->FileRecordsBuffer );
                VolumeCtx->FileRecordsBuffer = NULL;
            }

            if ( VolumeCtx->BitMapBuffer )
            {
                free( VolumeCtx->BitMapBuffer );
                VolumeCtx->BitMapBuffer = NULL;
            }

            if ( VolumeCtx->ClustersMapBuffer )
            {
                free( VolumeCtx->ClustersMapBuffer );
                VolumeCtx->ClustersMapBuffer = NULL;
            }

            if ( VolumeCtx->FatBuffer )
            {
                free( VolumeCtx->FatBuffer );
                VolumeCtx->FatBuffer = NULL;
            }

#ifdef _DEBUGS
            OutMessage( L"            Release Dirs Tree" );
#endif
            ReleaseTree( &(VolumeCtx->DirsTree) );

#ifdef _DEBUGS
            OutMessage( L"            Delete Files Tree" );
#endif
            DeleteTree( &(VolumeCtx->FilesTree) );

#ifdef _DEBUGS
            OutMessage( L"            Delete Extents Tree" );
#endif
            DeleteTree( &(VolumeCtx->ExtentsTree) );

#ifdef _DEBUGS
            OutMessage( L"            Delete Gaps Tree" );
#endif
            DeleteTree( &(VolumeCtx->GapsTree) );


            FreeFileCtx( VolumeCtx->MftFileCtx );

            //if ( RootDirCtx && VolumeCtx->DiskType != Fat32Disk )
            FreeFileCtx( VolumeCtx->RootDirCtx );

            DfrgCloseVolume( VolumeCtx );
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        int Result = GetExceptionCode();
        DislpayErrorMessage( L"DfrgReleaseVolumeCtx() Exception !!!", Result, ERR_OS );
    }

} // end of DfrgReleaseVolumeCtx

//
// Delete Job control structures
//
void
DfrgFreeJobCtx(
    IN  PDEFRAG_JOB_CONTEXT     JobCtx )
{
    PDEFRAG_JOB_CONTEXT     CurrentJob = JobCtx;

    if ( !CurrentJob )
    {
        return;
    }

    __try
    {

#ifdef _DEBUG
        WCHAR       Mess[MAX_PATH];
        swprintf( Mess, L"      DfrgFreeJobCtx %p %08X '%ls'", 
            CurrentJob, CurrentJob->Id, CurrentJob->VolumeCtx.VName );
        OutMessage( Mess );
#endif

        CurrentJob->Status |= DEFRAG_JOB_FLAG_DELETED;


        DfrgReleaseVolumeCtx( &(CurrentJob->VolumeCtx) );

#ifdef _DEBUGS
        swprintf( Mess, L"      Goto free( DisplayMapEx = %p [%d = %d * %d])", 
            CurrentJob->DisplayMapEx, CurrentJob->DisplayMapExSize*sizeof(DFRG_MAP_BLOCK_INFO),
            CurrentJob->DisplayMapExSize, sizeof(DFRG_MAP_BLOCK_INFO) );
        OutMessage( Mess );
#endif

        if ( CurrentJob->DisplayMapEx )
        {
            free( CurrentJob->DisplayMapEx );
            CurrentJob->DisplayMapEx = NULL;
        }

        UniFreeUnicodeString( &CurrentJob->CurrentFileName );

#ifdef _DEBUGS
        swprintf( Mess, L"      Goto DeleteCriticalSection " );
        OutMessage( Mess );
#endif

        DeleteCriticalSection( &(CurrentJob->JobSync) );

#ifdef _DEBUGS
        swprintf( Mess, L"      Goto CloseHandle( CurrentJob->ResumeEvent );" );
        OutMessage( Mess );
#endif

        if ( CurrentJob->ResumeEvent )
        {
#ifndef DEFRAG_NATIVE_APP
            CloseHandle( CurrentJob->ResumeEvent );
#else
            NtClose( CurrentJob->ResumeEvent );
#endif
            CurrentJob->ResumeEvent = NULL;
        }

#ifdef _DEBUGS
        swprintf( Mess, L"      Goto free CurrentJob %08X ", CurrentJob );
        OutMessage( Mess );
#endif

        free( CurrentJob );

    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        int Result = GetExceptionCode();
        DislpayErrorMessage( L"DfrgFreeJobCtx() Exception !!!", Result, ERR_OS );
    }


} // end of DfrgFreeJobCtx

#ifdef _DEBUGSS
    #undef _DEBUG
    #undef _DEBUGS
    #undef _DEBUGSS
#endif


//
// Core Defrag function. 
//
int
DfrgDoJob(
    IN  PDEFRAG_JOB_CONTEXT     JobCtx,
    IN  PDFRG_SETTINGS          Settings )
{
    int                         Result = NO_ERROR;


    __try
    {
        JobCtx->Settings = Settings;

        //
        // Analyze volume
        //
        if ( JobCtx->Type == DEFRAG_JOB_ANALIZE_ONLY ||
            !(JobCtx->Status & DEFRAG_JOB_FLAG_ANALIZED) )
        {
            Result = DfrgAnalyzeVolume( &JobCtx->VolumeCtx );

#if (defined _DEBUG) && !(defined DEFRAG_NATIVE_APP)
            PrintStatistic( JobCtx );
#endif

            if ( NO_ERROR != Result )
            {
                __leave;
            }
        }

        //
        // Defragment volume
        //
        if ( Settings->AggressivelyFreeSpaceEnable )
        {
            JobCtx->Options |= DEFRAG_JOB_OPTION_BE_AGRESSIVE;
        }

        if ( JobCtx->Type != DEFRAG_JOB_ANALIZE_ONLY )
        {
            Result = DfrgProcessVolume( &JobCtx->VolumeCtx );

#if (defined _DEBUG) && !(defined DEFRAG_NATIVE_APP)
            PrintStatistic( JobCtx );
#endif

            if ( NO_ERROR != Result )
            {
                __leave;
            }
        }

    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        Result = GetExceptionCode();
        DislpayErrorMessage( L"DfrgDoJob(). Exception !!!", Result, ERR_OS );
    }


    if ( NO_ERROR != Result )
    {
        if ( Result < 0 )
        {
            DislpayErrorMessage( L"DfrgDoJob(). Error", Result, ERR_INTERNAL );
        }
        else
        {
            DislpayErrorMessage( L"DfrgDoJob(). Error", Result, ERR_OS );
        }
    }


    //
    // Mark progress
    //
    JobCtx->ProcessedObjectsNum = JobCtx->ObjectsToProcessNum;

    JobCtx->VolumeCtx.Statistic.CurrentFileName[0] = L'\0';


    return Result;

} // end of DfrgDoJob


int 
DfrgClusterInspector( 
    IN  PDEFRAG_JOB_CONTEXT     Job,
    PCLUSTERS_INFO              ClustersBlock,
    PDEFRAG_CINSPECTOR_INFO     ClusterInfo,
    PULONG                      DataLength )
{
    int                         Result = NO_ERROR;

    PDEFRAG_VOLUME_CONTEXT      VolumeCtx = &(Job->VolumeCtx);
    DEFRAG_FILES_EXTENT         FirstExtent = { 0 };
    PDEFRAG_FILES_EXTENT        WrkExtent = NULL;
    PBTR_NODE                   WrkExtentNode = NULL;

    BOOLEAN                     IsInCriticalSection = FALSE;
    BOOLEAN                     IsFirstCluster;
    PDEFRAG_FILE_CONTEXT        FileCtx = NULL;
    UNICODE_STRING              FileName = { 0 };
    ULONGLONG                   Cnt = 0;
    ULONG                       WrkDataLength = 0;


    if ( !( (Job->Status & DEFRAG_JOB_FLAG_ANALIZED) &&
            (Job->Status & DEFRAG_JOB_FLAG_STOPPED) ) )
    {
        //
        // Volume should be analized & Job should be stopped/finished
        //
        Result = ERROR_INVALID_PARAMETER;
        return Result;
    }

#ifdef _DEBUG
    WCHAR       Mess[MAX_PATH];
    swprintf( Mess, L"   Get ClusterFiles from %I64d..%I64d [%I64d], count %d ",
        ClustersBlock->StartCluster, 
        ClustersBlock->StartCluster + ClustersBlock->ClustersNumber - 1,
        ClustersBlock->ClustersNumber,
        *DataLength / sizeof( DEFRAG_CINSPECTOR_INFO ) );
    OutMessage( Mess );
#endif

    if ( *DataLength < sizeof(DEFRAG_CINSPECTOR_INFO) )
    {
        Result = ERROR_BUFFER_OVERFLOW;
        return Result;
    }

    __try
    {
        //
        // Zero first record
        //
        memset( ClusterInfo, 0, sizeof(DEFRAG_CINSPECTOR_INFO) );

        //
        // This is not nesessary - Volume should be analized & Job should be stopped/finished
        //
        EnterCriticalSection( &(VolumeCtx->ExtentsTree.TreeSync) );
        IsInCriticalSection = TRUE;

        //
        // Find start extent in tree
        //
        IsFirstCluster = TRUE;

        FirstExtent.StartLcn.QuadPart = ClustersBlock->StartCluster;
        WrkExtentNode = FindNode( &(VolumeCtx->ExtentsTree), &FirstExtent, SortModeLcn, _FIND_LE_ );

        if ( WrkExtentNode )
        {
            WrkExtent = (PDEFRAG_FILES_EXTENT)(WrkExtentNode->Body);
        }

        if ( !WrkExtentNode || 
            WrkExtent->StartLcn.QuadPart + WrkExtent->Length.QuadPart <=
            ClustersBlock->StartCluster )
        {
            //
            // Extent is absent or is not in block
            //
            WrkExtentNode = FindNode( &(VolumeCtx->ExtentsTree), &FirstExtent, SortModeLcn, _FIND_GE_ );
            IsFirstCluster = FALSE;
        }

#ifdef _DEBUG
        if ( WrkExtentNode )
        {
            swprintf( Mess, L"      First Extent%ls. '%ls' %I64d..%I64d [%I64d]",
                IsFirstCluster ? L". IsFirstCluster," : L"",
                WrkExtent->StreamCtx->FileCtx->FName,
                WrkExtent->StartLcn.QuadPart, 
                WrkExtent->StartLcn.QuadPart + WrkExtent->Length.QuadPart - 1,
                WrkExtent->Length.QuadPart );
        }
        else
        {
            swprintf( Mess, L"      Could not find First Extent" );
        }
        OutMessage( Mess );
#endif

        while ( WrkExtentNode && 
            WrkDataLength + sizeof(DEFRAG_CINSPECTOR_INFO) <= *DataLength )
        {
            WrkExtent = (PDEFRAG_FILES_EXTENT)(WrkExtentNode->Body);

            if ( !IsFirstCluster &&
                WrkExtent->StartLcn.QuadPart >=
                ClustersBlock->StartCluster + ClustersBlock->ClustersNumber )
            {
                //
                // We got all extents in this block or block is empty
                //
#ifdef _DEBUG
                swprintf( Mess, L"      That's all. '%ls' %I64d..%I64d [%I64d]",
                    WrkExtent->StreamCtx->FileCtx->FName,
                    WrkExtent->StartLcn.QuadPart, 
                    WrkExtent->StartLcn.QuadPart + WrkExtent->Length.QuadPart - 1,
                    WrkExtent->Length.QuadPart );
                OutMessage( Mess );
#endif
                __leave;
            }

            IsFirstCluster = FALSE;

            ClusterInfo[Cnt].StartCluster       = WrkExtent->StartLcn.QuadPart;
            ClusterInfo[Cnt].NumberOfClusters   = WrkExtent->Length.QuadPart;
            ClusterInfo[Cnt].VCN                = WrkExtent->StartVcn.QuadPart;

            LONG    NameLength = 0;

            //
            // Get name
            //
            FileCtx = WrkExtent->StreamCtx->FileCtx;

            Result = DfrgGetFileName( Job, FileCtx, &FileName );

            if ( NO_ERROR == Result )
            {
                NameLength = (LONG)UniGetLen( &FileName );
                PWCHAR VolumeNamePtr = FileName.Buffer;

                if ( wcsstr( FileName.Buffer, L"\\??\\" ) )
                {
                    VolumeNamePtr += 4;
                    NameLength -= 4;
                }

                NameLength = min( MAX_PATH-1, NameLength );

                memcpy( ClusterInfo[Cnt].FileName, VolumeNamePtr, NameLength*sizeof(WCHAR) );
            }

            ClusterInfo[Cnt].FileName[NameLength] = L'\0';
            UniFreeUnicodeString( &FileName );

            Cnt++;
            WrkDataLength += sizeof(DEFRAG_CINSPECTOR_INFO);

            //
            // Get next extent
            //
            WrkExtentNode = FindNextNode( &(VolumeCtx->ExtentsTree), LEFT_TO_RIGHT );
        }

    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        Result = GetExceptionCode();
        DislpayErrorMessage( L"DfrgClusterInspector(). Exception !!!", Result, ERR_OS );
    }


    if ( IsInCriticalSection )
    {
        LeaveCriticalSection( &(VolumeCtx->ExtentsTree.TreeSync) );
        IsInCriticalSection = FALSE;
    }

    UniFreeUnicodeString( &FileName );

    DfrgCloseFile( FileCtx );

    *DataLength = WrkDataLength;


    return Result;

} // end of DfrgClusterInspector


PDEFRAG_FILE_CONTEXT
DfrgFindFile(
    IN  PBTR_TREE               Tree,
    PDEFRAG_FILE_CONTEXT        Template,
    BTR_TREE_SORT_MODE          Key )
{
    PBTR_NODE                   Node;
    PDEFRAG_FILE_CONTEXT        FileCtx = NULL;

    Node = FindFirstNode( Tree, LEFT_TO_RIGHT );

    while ( Node )
    {
        FileCtx = (PDEFRAG_FILE_CONTEXT)(Node->Body);

        if ( !Tree->Methods.CompareItems( Template, FileCtx, Key ) )
        {
            break;
        }

        Node = FindNextNode( Tree, LEFT_TO_RIGHT );
    }


    return FileCtx;

} // end of DfrgFindFile


int 
DfrgGetFileInfo( 
    IN      PDEFRAG_JOB_CONTEXT Job,
    IN      PWCHAR              FileName,
    OUT     PDEFRAG_FILE_INFO   FileInfo,
    IN  OUT PULONG              DataLength )
{
    int                         Result = NO_ERROR;

    PDEFRAG_VOLUME_CONTEXT      VolumeCtx = &(Job->VolumeCtx);
    DEFRAG_FILE_CONTEXT         WrkFileCtx = { 0 };

    PDEFRAG_FILE_CONTEXT        FileCtx = NULL;
    PDEFRAG_STREAM_CONTEXT      StreamCtx = NULL;

    PDEFRAG_EXTENT_INFO         IExtent = NULL;
    PDEFRAG_STREAM_INFO         IStream = NULL;

    BOOLEAN                     IsInCriticalSection = FALSE;
    ULONG                       WrkDataLength = 0;

    UNICODE_STRING              UniName = { 0 };


    if ( !( (Job->Status & DEFRAG_JOB_FLAG_ANALIZED) &&
            (Job->Status & DEFRAG_JOB_FLAG_STOPPED) ) )
    {
        //
        // Volume should be analized & Job should be stopped/finished
        //
        Result = ERROR_INVALID_PARAMETER;
        return Result;
    }

#ifdef _DEBUG
    WCHAR       Mess[MAX_PATH];
    swprintf( Mess, L"   Get DfrgGetFileInfo for %ls", FileName );
    OutMessage( Mess );
#endif

    if ( *DataLength < sizeof(DEFRAG_FILE_INFO) )
    {
        Result = ERROR_BUFFER_OVERFLOW;
        return Result;
    }

    __try
    {
        //
        // Zero record
        //
        memset( FileInfo, 0, sizeof(DEFRAG_FILE_INFO) );

        //
        // This is not nesessary - Volume should be analized & Job should be stopped/finished
        //
        //EnterCriticalSection( &(VolumeCtx->ExtentsTree.TreeSync) );
        //IsInCriticalSection = TRUE;

        //
        // Init work file context.
        // WrkFileCtx.FName should be zero before release as it is not allocated by us.
        //
        WrkFileCtx.VolumeCtx = VolumeCtx;
        WrkFileCtx.FName = FileName;

        //
        // Get unicode file name in internal format
        //
        if ( !UniInitUnicodeString( &UniName, VolumeCtx->VName ) )
        {
            Result = ERROR_NOT_ENOUGH_MEMORY;
            __leave;
        }

        if ( !UniAppend( &UniName, FileName ) )
        {
            Result = ERROR_NOT_ENOUGH_MEMORY;
            __leave;
        }

        //wcsupr( UniName.Buffer );
        WrkFileCtx.FName = UniName.Buffer;
        WrkFileCtx.DefragFlags2 |= DEFRAG_FILE2_FULL_NAME;

        if ( NtfsDisk == Job->VolumeCtx.DiskType )
        {
            Result = DfrgOpenFile( &WrkFileCtx, FALSE, UniName.Buffer );
            if ( NO_ERROR != Result )
            {
                __leave;
            }

            Result = DfrgQueryFileId( &WrkFileCtx );
            if ( NO_ERROR != Result )
            {
                __leave;
            }

            //
            // Find File in tree by Id
            //
            FileCtx = DfrgFindFile( &(VolumeCtx->FilesTree), &WrkFileCtx, SortModeId );
        }
        //if ( !FileCtx )
        else 
        {
            //
            // Find File in tree by name
            //
            FileCtx = DfrgFindFile( &(VolumeCtx->FilesTree), &WrkFileCtx, SortModePath );
        }

        if ( !FileCtx )
        {
            Result = ERROR_FILE_NOT_FOUND;
            __leave;
        }

        wcsncpy( FileInfo->FileName, FileName, MAX_PATH-1 );
        FileInfo->FileId            = FileCtx->FId.FileId;

        FileInfo->CreationTime      = FileCtx->CreationTime;
        FileInfo->ChangeTime        = FileCtx->ChangeTime;
        FileInfo->LastAccessTime    = FileCtx->LastAccessTime;
        //FileInfo->LastWriteTime     = FileCtx->LastWriteTime;

        FileInfo->Attributes        = FileCtx->Attributes;
        FileInfo->DefragFlags       = FileCtx->DefragFlags;

        //
        // Iterate through all file's streams
        //
        DWORD       k;

        WrkDataLength = sizeof(DEFRAG_FILE_INFO) - sizeof(DEFRAG_STREAM_INFO);

        for ( k=0, StreamCtx = FileCtx->FirstStream; StreamCtx; 
            StreamCtx = StreamCtx->NextStream, k++ )
        {
            FileInfo->StreamsNum++;
            WrkDataLength += sizeof(DEFRAG_STREAM_INFO);

            if ( WrkDataLength <= *DataLength )
            {
                memset( &(FileInfo->Stream[k]), 0, sizeof(DEFRAG_STREAM_INFO) );

                if ( NtfsDisk == VolumeCtx->DiskType )
                {
                    FileInfo->Stream[k].StreamType = StreamCtx->MftStreamType;
                    if ( StreamCtx->StreamName )
                    {
                        wcsncpy( FileInfo->Stream[k].StreamName, StreamCtx->StreamName, MAX_PATH-1 );
                    }
                }
            }

            FileInfo->Stream[k].ExtentsNum = 0;

            ULONGLONG   StartVcn = 0; //, StartLcn = 0, Length = 0;

            for( DWORD i=0; i<StreamCtx->ClustersMap->ExtentCount; 
                StartVcn = StreamCtx->ClustersMap->Extents[i].NextVcn.QuadPart, i++ )
            {
                FileInfo->Stream[k].ExtentsNum++;

                //if ( VIRTUAL_LCN == StreamCtx->ClustersMap->Extents[i].Lcn.QuadPart ) 
                //{
                //    //
                //    // Skip virtual (sparce) extent
                //    //
                //    continue;
                //}

                WrkDataLength += sizeof(DEFRAG_EXTENT_INFO);

                if ( WrkDataLength <= *DataLength )
                {
                    FileInfo->Stream[k].Extent[i].StartLcn.QuadPart = 
                        StreamCtx->ClustersMap->Extents[i].Lcn.QuadPart;
                    FileInfo->Stream[k].Extent[i].Length.QuadPart = 
                        StreamCtx->ClustersMap->Extents[i].NextVcn.QuadPart - StartVcn;
                    FileInfo->Stream[k].Extent[i].StartVcn.QuadPart = StartVcn;
                }
            }
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        Result = GetExceptionCode();
        DislpayErrorMessage( L"DfrgGetFileInfo(). Exception !!!", Result, ERR_OS );
    }


    //if ( IsInCriticalSection )
    //{
    //    LeaveCriticalSection( &(VolumeCtx->ExtentsTree.TreeSync) );
    //    IsInCriticalSection = FALSE;
    //}

    //
    // WrkFileCtx.FName should be zero before release as it is not allocated by us
    //
    WrkFileCtx.FName = NULL;
    ReleaseFileCtx( &WrkFileCtx );

    *DataLength = WrkDataLength;


    return Result;

} // end of DfrgGetFileInfo


int 
DfrgGetMarkedFiles( 
    IN      PDEFRAG_JOB_CONTEXT Job,
    IN      ULONG               Command,
    OUT     PDEFRAG_FILE_LIST   FileList,
    IN  OUT PULONG              DataLength )
{
    int                         Result = NO_ERROR;

    PDEFRAG_VOLUME_CONTEXT      VolumeCtx = &(Job->VolumeCtx);

    PBTR_NODE                   FileNode = NULL;
    PDEFRAG_FILE_CONTEXT        FileCtx = NULL;
    PDEFRAG_STREAM_CONTEXT      StreamCtx = NULL;

    ULONG                       Flags   = 0;
    ULONG                       Flags2  = 0;

    BOOLEAN                     IsInCriticalSection = FALSE;
    BOOLEAN                     LookMostFragmented = TRUE;
    int                         FileIndex = 0, MinFileIndex = -1;
    int                         FilesCnt = 0;
    ULONG                       WrkDataLength = 0;
    UNICODE_STRING              FileName = { 0 };


    if ( !( (Job->Status & DEFRAG_JOB_FLAG_ANALIZED) &&
            (Job->Status & DEFRAG_JOB_FLAG_STOPPED) ) )
    {
        //
        // Volume should be analized & Job should be stopped/finished
        //
        Result = ERROR_INVALID_PARAMETER;
        return Result;
    }

#ifdef _DEBUG
    WCHAR       Mess[MAX_PATH];
    swprintf( Mess, L"   Get DfrgGetMarkedFiles %u", Command );
    OutMessage( Mess );
#endif

    if ( *DataLength < sizeof(DEFRAG_FILE_LIST) )
    {
        Result = ERROR_BUFFER_OVERFLOW;
        return Result;
    }


    __try
    {
        //
        // Zero record
        //
        memset( FileList, 0, sizeof(DEFRAG_FILE_LIST) );

        //
        // This is not nesessary - Volume should be analized & Job should be stopped/finished
        //
        //EnterCriticalSection( &(VolumeCtx->ExtentsTree.TreeSync) );
        //IsInCriticalSection = TRUE;

        //
        // Determine limits and flags
        //
        DWORD       MaxFilesNum = min( DEFRAG_MAX_FILES_IN_LIST, *DataLength / sizeof(DEFRAG_FILE_LIST) );

        switch ( Command )
        {
        case DEFRAG_CMD_GET_UNMOV_FILES:
            Flags   = (DEFRAG_FILE_UNMOVABLE);
            break;

        case DEFRAG_CMD_GET_SYSTEM_FILES:
            Flags   = (DEFRAG_FILE_SYSTEM);
            break;

        case DEFRAG_CMD_GET_DIR_FILES:
            Flags   = (DEFRAG_FILE_DIRECTORY);
            break;

        case DEFRAG_CMD_GET_MOST_FRAGMENTED:
            Flags   = (DEFRAG_FILE_FRAGMENTED);
            LookMostFragmented = TRUE;
            break;

        case DEFRAG_CMD_GET_SKIPPED: // Excluded files
            Flags   = (DEFRAG_FILE_UNMOVABLE | DEFRAG_FILE_LOCKED | DEFRAG_FILE_EXCLUDED);
            break;

        default:
            Result = ERROR_INVALID_PARAMETER;
            __leave;
        }

        if ( LookMostFragmented )
        {
            //
            // We should return MaxFilesNum files with MAX fragmentation ratio.
            //
            MaxFilesNum++;
        }

        //
        // Iterate through file list and process files.
        // Start with the last file (after analysis - the longest one).
        //
        FileNode = FindFirstNode( &(VolumeCtx->FilesTree), RIGHT_TO_LEFT );

        if ( !FileNode )
        {
            __leave;
        }

        BOOLEAN     ShouldBreak = FALSE;

        do
        {
            FileCtx = (PDEFRAG_FILE_CONTEXT)FileNode->Body;

            if ( !(Flags & FileCtx->DefragFlags ) && 
                !(Flags2 & FileCtx->DefragFlags2 ) )
            {
                //
                // Skip this file 
                //
                continue;
            }

            //
            // Iterate through all file's streams
            //
            ULONGLONG   ExtentsNum = 0;

            for ( StreamCtx = FileCtx->FirstStream; StreamCtx; 
                StreamCtx = StreamCtx->NextStream )
            {
                ExtentsNum += StreamCtx->ClustersMap->ExtentCount;
            }

            FileIndex = FilesCnt;
            FilesCnt++;
            WrkDataLength += sizeof(DEFRAG_FILE_LIST);

            if ( FilesCnt == MaxFilesNum )
            {
                if ( LookMostFragmented )
                {
                    //
                    // In this case MaxFilesNum is overflowed. Shrink it.
                    //
                    FilesCnt--;
                    WrkDataLength -= sizeof(DEFRAG_FILE_LIST);

                    int     i;

                    //
                    // We should return only MaxFilesNum most fragmented files.
                    // Find less fragmented file in list.
                    //
                    if ( -1 == MinFileIndex )
                    {
                        MinFileIndex = 0;
                        for ( i=1; i<FilesCnt; i++ )
                        {
                            if ( FileList[i].ExtentsNum < FileList[MinFileIndex].ExtentsNum  )
                            {
                                MinFileIndex = i;
                            }
                        }
                    }

                    if ( FileList[MinFileIndex].ExtentsNum < ExtentsNum )
                    {
#ifdef _DEBUGW
                        swprintf( Mess, L"   Replace [%d] '%ls' : [%I64d] %08X %08X "
                                        L"   with  %016I64X  '%ls' : [%I64d] %08X %08X ", 
                            MinFileIndex,
                            FileList[MinFileIndex].FileName ? FileList[MinFileIndex].FileName : L"", 
                            FileList[MinFileIndex].ExtentsNum, FileList[MinFileIndex].DefragFlags, FileList[MinFileIndex].DefragFlags2,
                            FileCtx->FId.FileId,
                            FileCtx->FName ? FileCtx->FName : L"", 
                            ExtentsNum, FileCtx->DefragFlags, FileCtx->DefragFlags2 );
                        OutMessage( Mess );
#endif
                        FileIndex = MinFileIndex;
                        MinFileIndex = -1;
                    }
                    else
                    {
                        //
                        // Skip this file as it is less fragmented
                        //
                        continue;
                    }
                }
                else
                {
                    ShouldBreak = TRUE;
                }
            }

            if ( FileIndex >= FilesCnt )
            {
#ifdef _DEBUG
                 swprintf( Mess, L"   Index overflow %d >= %d ", FileIndex, FilesCnt );
                 OutMessage( Mess );
#endif
                 __leave;
            }

            //
            // Set file's info in list
            //
            //if ( FileCtx->FName )
            //{
            //    wcsncpy( FileList[FileIndex].FileName, FileCtx->FName, MAX_PATH-1 );
            //}

            LONG    NameLength = 0;

            Result = DfrgGetFileName( Job, FileCtx, &FileName );

            if ( NO_ERROR == Result )
            {
                NameLength = (LONG)UniGetLen( &FileName );
                PWCHAR  VolumeNamePtr = FileName.Buffer;

                if ( wcsstr( FileName.Buffer, L"\\??\\" ) )
                {
                    VolumeNamePtr += 4;
                    NameLength -= 4;
                }

                NameLength = min( MAX_PATH-1, NameLength );

                memcpy( FileList[FileIndex].FileName, VolumeNamePtr, NameLength*sizeof(WCHAR) );
            }

            FileList[FileIndex].FileName[NameLength] = L'\0'; // [MAX_PATH-1]
            UniFreeUnicodeString( &FileName );

            FileList[FileIndex].DefragFlags   = FileCtx->DefragFlags;
            FileList[FileIndex].DefragFlags2  = FileCtx->DefragFlags2;
            FileList[FileIndex].ExtentsNum = ExtentsNum;

        } while ( !ShouldBreak &&
            (FileNode = FindNextNode( &(VolumeCtx->FilesTree), RIGHT_TO_LEFT )) );
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        Result = GetExceptionCode();
        DislpayErrorMessage( L"DfrgGetMarkedFiles(). Exception !!!", Result, ERR_OS );
    }


    //if ( IsInCriticalSection )
    //{
    //    LeaveCriticalSection( &(VolumeCtx->ExtentsTree.TreeSync) );
    //    IsInCriticalSection = FALSE;
    //}

    UniFreeUnicodeString( &FileName );

    *DataLength = WrkDataLength;


    return Result;

} // end of DfrgGetMarkedFiles



