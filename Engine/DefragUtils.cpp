/*
    Defrag Engine medium level functions module

    Module name:

        DefragUtils.cpp

    Abstract:

        Defrag Engine medium level functions module. 
        Contains medium level defragmentation functions.

    $Header: /home/CVS_DEFRAG/Defrag/Src/Engine/DefragUtils.cpp,v 1.11 2009/12/25 13:48:53 dimas Exp $
    $Log: DefragUtils.cpp,v $
    Revision 1.11  2009/12/25 13:48:53  dimas
    Debug artefacts removed

    Revision 1.10  2009/12/24 15:02:51  dimas
    Some cosmetic improvements

    Revision 1.9  2009/12/24 10:52:20  dimas
    Check against Exclude Files list implemented

    Revision 1.8  2009/12/23 13:01:47  dimas
    1. Full File Name processing implemented.
    2. Top 100 Fragmented functionality for all file list requests implemented.

    Revision 1.7  2009/12/21 17:00:13  dimas
    1. DEFRAG_CMD_GET_MOST_FRAGMENTED and similar requests implemented
    2. Basic Exclude files check added

    Revision 1.6  2009/12/02 14:42:16  dimas
    App/Service interaction improved

    Revision 1.5  2009/11/30 14:58:47  dimas
    Currently processing file name visualization implimented

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

//
// Array of bit masks
//
BYTE        BitMask[] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };


void
MarkProgress( 
    PDEFRAG_VOLUME_CONTEXT      VolumeCtx,
    LONGLONG                    ProgressIncrement )
{
    LONGLONG                    Progress;

    if ( ProgressIncrement > 0 )
    {
        Progress = VolumeCtx->JobCtx->ProcessedObjectsNum.QuadPart + ProgressIncrement;
        if ( VolumeCtx->JobCtx->ObjectsToProcessNum.QuadPart < Progress )
        {
            VolumeCtx->JobCtx->ObjectsToProcessNum.QuadPart += ProgressIncrement;
        }
        VolumeCtx->JobCtx->ProcessedObjectsNum.QuadPart = Progress;
    }
    else
    {
        Progress = VolumeCtx->JobCtx->ObjectsToProcessNum.QuadPart + ProgressIncrement;
        if ( Progress > VolumeCtx->JobCtx->ObjectsToProcessNum.QuadPart )
        {
            VolumeCtx->JobCtx->ObjectsToProcessNum.QuadPart += ProgressIncrement;
        }
    }

}


int
DfrgMoveExtentToGap( 
    PDEFRAG_FILES_EXTENT        Extent,
    PDEFRAG_LCN_EXTENT          Gap,
    DWORD                       Flags )
{
    int                         Result = NO_ERROR;

    PDEFRAG_STREAM_CONTEXT      StreamCtx = Extent->StreamCtx;
    PDEFRAG_FILE_CONTEXT        FileCtx = StreamCtx->FileCtx;
    PDEFRAG_VOLUME_CONTEXT      VolumeCtx = FileCtx->VolumeCtx;

    PRETRIEVAL_POINTERS_BUFFER  NewStreamClustersMap = NULL;

    BOOLEAN                     ExtentWasProcessed   = FALSE;

#ifdef _DEBUG
    WCHAR                       Mess[MAX_PATH*2];

    swprintf( Mess, L"* Want to move '%ls' Extent %I64d [%I64d] from Lcn %I64d  to  Gap %I64d [%I64d]",
        FileCtx->FName ? FileCtx->FName : L"NULL" ,
        Extent->StartVcn.QuadPart, Extent->Length.QuadPart, Extent->StartLcn.QuadPart, 
        Gap->StartLcn.QuadPart, Gap->Length.QuadPart );
    OutMessage( Mess );
#endif


    if ( (FileCtx->DefragFlags & DEFRAG_FILE_UNMOVABLE) )
    {
        return DFRG_ERROR_SKIP;
    }
 
    if ( Extent->StartLcn.QuadPart < 0 ||
        Extent->Length.QuadPart <= 0 ||
        Extent->StartVcn.QuadPart < 0 )
    {
        return ERROR_INVALID_PARAMETER;
    }
 
    if ( Gap->StartLcn.QuadPart < 0 ||
        Gap->Length.QuadPart <= 0 )
    {
        return ERROR_INVALID_PARAMETER;
    }
 
    //
    // Update Display :
    // - set processing mark on extent clusters;
    // - remove current extent info from display map.
    //
    UpdateDisplayForExtent( 
                FileCtx, 
                Extent->StartLcn.QuadPart, 
                Extent->Length.QuadPart, 
                (SUBSTRACT_CLASTERS | INCL_FLAGS_COUNTS | INCL_INPROC_COUNTS | INCL_SPACE_COUNTS) );

    __try
    {
        //
        // Make checks
        //
        if ( !StreamCtx->ClustersMap )
        {
            Result = ERROR_INVALID_PARAMETER;
            __leave;
        }

        if ( Extent->Length.QuadPart > Gap->Length.QuadPart )
        {
            //
            // Extent is too long for this gap
            //
            Result = DFRG_ERROR_NOT_ENOUGH_SPACE;
            __leave;
        }

        //
        // Open file stream (default or ADS)
        //
        if ( (Flags & SHOULD_OPEN_STREAM) )
        {
            Result = DfrgOpenFileStream( StreamCtx );
            if ( NO_ERROR != Result )
            {
                if ( !FileCtx->FHandle || INVALID_HANDLE_VALUE == FileCtx->FHandle )
                {
                    //
                    // File was not opened - mark it as locked.
                    //
                    FileCtx->DefragFlags |= (DEFRAG_FILE_LOCKED | DEFRAG_FILE_UNMOVABLE);
                    FileCtx->VolumeCtx->Statistic.UnmovableFilesNum.QuadPart++;
                }
                __leave;
            }
        }

        //
        // Move extent to zone
        //
#ifdef _DEBUG
        swprintf( Mess, L"* Move '%ls' Extent %I64d [%I64d] from Lcn %I64d  to  Gap %I64d [%I64d]", 
            FileCtx->FName ? FileCtx->FName : L"NULL",
            Extent->StartVcn.QuadPart, Extent->Length.QuadPart, Extent->StartLcn.QuadPart, 
            Gap->StartLcn.QuadPart, Gap->Length.QuadPart );
        OutMessage( Mess );
#endif

        Result = DfrgMoveClusters(
                        FileCtx,
                        Extent->StartVcn,
                        Gap->StartLcn,
                        Extent->Length.LowPart );
        if ( NO_ERROR != Result )
        {
#ifdef _DEBUG
            OutMessage( L"        DfrgMoveClusters error" );
            //GetInput();
#endif
            FileCtx->DefragFlags |= (DEFRAG_FILE_UNMOVABLE);
            __leave;
        }

#ifdef _DEBUG
        OutMessage( L"    Moved" );
#endif

        //
        // Update Display :
        // - remove processing mark from old extent clusters;
        //
        UpdateDisplayForExtent( 
                    FileCtx, 
                    Extent->StartLcn.QuadPart, 
                    Extent->Length.QuadPart, 
                    INCL_INPROC_COUNTS );

        //
        // Check success.
        //
        Result = DfrgGetStreamClustersMap( FileCtx, &NewStreamClustersMap );

        if ( NO_ERROR != Result || !NewStreamClustersMap )
        {
            OutMessage( L"      DfrgGetStreamClustersMap 2 error" );
            __leave;
        }

#ifdef _DEBUG
        if ( FileCtx->VolumeCtx->JobCtx->Type != DEFRAG_JOB_DEBUG1 &&
            !CheckStreamMaps( StreamCtx, NewStreamClustersMap, FALSE ) )
        {
            OutMessage( L"      ClustersMap not changed" );
        }
#endif

        //
        // Update Display :
        // - set (new) extent info in display map.
        //
        UpdateDisplayForExtent( 
                    FileCtx, 
                    Gap->StartLcn.QuadPart, 
                    Extent->Length.QuadPart, 
                    (INCL_FLAGS_COUNTS | INCL_SPACE_COUNTS) );

        //
        // Adjust the gap we used.
        //
        Gap->StartLcn.QuadPart += Extent->Length.QuadPart;
        Gap->Length.QuadPart   -= Extent->Length.QuadPart;

        if ( (Flags & SHOULD_ADJUST_USED_GAP) )
        {
            PBTR_NODE               GapNode = NULL;
            PDEFRAG_LCN_EXTENT      OldGap = Gap;
            DfrgUpdateGapNode( &(VolumeCtx->GapsTree), &GapNode, &OldGap );
        }

        //
        // Add new gap in place of moved extent to gaps tree.
        //
        if ( (Flags & SHOULD_ADJUST_NEW_GAP) )
        {
            PBTR_NODE               GapNode = NULL;
            PDEFRAG_LCN_EXTENT      NewGap = (PDEFRAG_LCN_EXTENT) malloc( sizeof(PDEFRAG_LCN_EXTENT) );
            if ( NewGap )
            {
                NewGap->Length = Extent->Length;
                NewGap->StartLcn= Extent->StartLcn;
                DfrgInsertGapNode( &(VolumeCtx->GapsTree), &GapNode, &NewGap );
            }
            else
            {
                Result = ERROR_NOT_ENOUGH_MEMORY;
            }
        }

        //
        // Remove extent from tree.
        //
        //if ( (Flags & SHOULD_ADJUST_USED_EXTENT) )
        //{
        //    DeleteNode(  &(VolumeCtx->ExtentsTree), &ExtentNode );
        //}

        //
        // Adjust ClustersMap.
        // If caller open stream - it is it's responsibility to adjust cluster map.
        //
        if ( (Flags & SHOULD_ADJUST_CLUSTER_MAP) )
        {
            free( StreamCtx->ClustersMap );
            StreamCtx->ClustersMap = NewStreamClustersMap;
            NewStreamClustersMap = NULL;
        }

        ExtentWasProcessed = TRUE;

    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        Result = GetExceptionCode();
        DislpayErrorMessage( L"DfrgMoveExtentToGap(). Exception !!!", Result, ERR_OS );
    }

    //
    // Close all handles
    //
    if ( (Flags & SHOULD_OPEN_STREAM) )
    {
        DfrgCloseFile( FileCtx );
    }

    if ( NewStreamClustersMap )
    {
        free( NewStreamClustersMap );
    }

    if ( !ExtentWasProcessed )
    {
        //
        // Update Display :
        // - remove processing mark on (old) extent clusters;
        // - restore old extent info in display map.
        //
        UpdateDisplayForExtent( 
                    FileCtx, 
                    Extent->StartLcn.QuadPart, 
                    Extent->Length.QuadPart, 
                    (INCL_FLAGS_COUNTS | INCL_INPROC_COUNTS | INCL_SPACE_COUNTS) );

        if ( NO_ERROR == Result )
        {
            Result = DFRG_ERROR_SKIP;
        }
    }
    else
    {
        //
        // Mark progress
        //
        MarkProgress( VolumeCtx, Extent->Length.QuadPart );
    }


    return Result;

} // end of DfrgMoveExtentToGap


int
DfrgSlideExtentToGap( 
    PDEFRAG_FILES_EXTENT        Extent,
    PDEFRAG_LCN_EXTENT          Gap,
    DWORD                       MoveExtentFlags )
{
    int                         Result = ERROR_NOT_SUPPORTED;

    DEFRAG_FILES_EXTENT         ExtentToMove = { 0 };
    BOOLEAN                     Direction;
    LONGLONG                    ExtentLength = 0, GapLength = 0;
    //DWORD                       MoveExtentFlags = SHOULD_ADJUST_CLUSTER_MAP;


    __try
    {

#ifdef _DEBUG
        WCHAR       Mess[MAX_PATH];
        swprintf( Mess, L"   Slide Extent from %I64d [%I64d] To Gap %I64d [%I64d]",
                Extent->StartLcn.QuadPart, Extent->Length.QuadPart,
                Gap->StartLcn.QuadPart, Gap->Length.QuadPart );
        OutMessage( Mess );
#endif

        //Result = DfrgOpenFileStream( Extent->StreamCtx );
        //if ( NO_ERROR != Result )
        //{
        //    __leave;
        //}

        ExtentLength = Extent->Length.QuadPart;
        GapLength    = Gap->Length.QuadPart;

        ExtentToMove.StreamCtx  = Extent->StreamCtx;

        if ( Extent->StartLcn.QuadPart < Gap->StartLcn.QuadPart )
        {
            Direction = LEFT_TO_RIGHT;

            ExtentToMove.StartLcn.QuadPart  = 
                Extent->StartLcn.QuadPart + ExtentLength - Gap->Length.QuadPart;
            ExtentToMove.StartVcn.QuadPart  = 
                Extent->StartVcn.QuadPart + ExtentLength - Gap->Length.QuadPart;
        }
        else
        {
            Direction = RIGHT_TO_LEFT;

            ExtentToMove.StartLcn   = Extent->StartLcn;
            ExtentToMove.StartVcn   = Extent->StartVcn;
        }


        while ( ExtentLength )
        {
            if ( ExtentLength < GapLength )
            {
                ExtentToMove.Length.QuadPart = ExtentLength;
                if ( LEFT_TO_RIGHT == Direction )
                {
                    Gap->StartLcn.QuadPart += GapLength - ExtentLength;
                }
            }
            else
            {
                ExtentToMove.Length.QuadPart = GapLength;
            }

            //
            // Move extent's part
            //
#ifdef _DEBUG
            swprintf( Mess, L"      slide extent from %I64d [%I64d]  to  gap %I64d [%I64d]",
                    ExtentToMove.StartLcn.QuadPart, ExtentToMove.Length.QuadPart,
                    Gap->StartLcn.QuadPart, Gap->Length.QuadPart );
            OutMessage( Mess );
#endif

            Result = DfrgMoveExtentToGap( &ExtentToMove, Gap, MoveExtentFlags );

            if ( NO_ERROR != Result )
            {
                break;
            }

            //
            // Set new gap position & next extent
            //
            Gap->Length.QuadPart = GapLength;
            Gap->StartLcn = ExtentToMove.StartLcn;

            if ( LEFT_TO_RIGHT == Direction )
            {
                //ThisExtent.StartVcn.QuadPart   += ExtentToMove.Length.QuadPart;
                //ThisExtent.StartLcn.QuadPart   += ExtentToMove.Length.QuadPart;
                ExtentToMove.StartVcn.QuadPart   -= ExtentToMove.Length.QuadPart;
                ExtentToMove.StartLcn.QuadPart   -= ExtentToMove.Length.QuadPart;
            }
            else
            {
                if ( ExtentLength < GapLength )
                {
                    Gap->StartLcn.QuadPart -= (GapLength - ExtentLength);
                }

                //ThisExtent.StartVcn.QuadPart   -= ExtentToMove.Length.QuadPart;
                //ThisExtent.StartLcn.QuadPart   -= ExtentToMove.Length.QuadPart;
                ExtentToMove.StartVcn.QuadPart   += ExtentToMove.Length.QuadPart;
                ExtentToMove.StartLcn.QuadPart   += ExtentToMove.Length.QuadPart;
            }

            ExtentLength -= ExtentToMove.Length.QuadPart;

#ifdef _DEBUGW
            swprintf( Mess, L"      after slide: extent %I64d [%I64d], gap %I64d [%I64d], left to move %I64d",
                    ExtentToMove.StartLcn.QuadPart, ExtentToMove.Length.QuadPart,
                    Gap->StartLcn.QuadPart, Gap->Length.QuadPart,
                    ExtentLength );
            OutMessage( Mess );
#endif

        } // while ()

        //
        // Adjust extent finally
        //
        if ( LEFT_TO_RIGHT == Direction )
        {
            Extent->StartLcn.QuadPart   += GapLength;
        }
        else
        {
            Extent->StartLcn.QuadPart   -= GapLength;
        }

#ifdef _DEBUG
        swprintf( Mess, L"   *** After Slide: Extent %I64d [%I64d], Gap %I64d [%I64d]",
                Extent->StartLcn.QuadPart, Extent->Length.QuadPart,
                Gap->StartLcn.QuadPart, Gap->Length.QuadPart );
        OutMessage( Mess );
#endif

    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        Result = GetExceptionCode();
        DislpayErrorMessage( L"DfrgSlideExtentToGap(). Exception !!!", Result, ERR_OS );
    }

    //DfrgCloseFile( Extent->StreamCtx->FileCtx );


    return Result;

} // end of DfrgSlideExtentToGap


int
DfrgMoveFileToGap( 
    PDEFRAG_FILE_CONTEXT        FileCtx,
    PDEFRAG_ZONE                SourceZone,
    PDEFRAG_LCN_EXTENT          Gap )
{
    int                         Result = NO_ERROR;

    PDEFRAG_VOLUME_CONTEXT      VolumeCtx = FileCtx->VolumeCtx;
    PDEFRAG_STREAM_CONTEXT      StreamCtx;
    PRETRIEVAL_POINTERS_BUFFER  StreamClustersMap = NULL;

    BOOLEAN                     GapIsEmpty          = FALSE;
    BOOLEAN                     FileWasProcessed    = FALSE;

#ifdef _DEBUG
    WCHAR                       Mess[MAX_PATH*2];
#endif


    __try
    {
        //
        // Iterate through all file's streams
        //
        for ( StreamCtx = FileCtx->FirstStream; StreamCtx && !GapIsEmpty; StreamCtx = StreamCtx->NextStream )
        {
            if ( !StreamCtx->ClustersMap )
            {
                continue;
            }

            //
            // Open file stream (default or ADS)
            //
            Result = DfrgOpenFileStream( StreamCtx );
            if ( NO_ERROR != Result )
            {
                if ( !FileCtx->FHandle || INVALID_HANDLE_VALUE == FileCtx->FHandle )
                {
                    //
                    // File was not opened - mark it as locked and break.
                    // Update Display & Flags
                    //
                    UpdateDisplayForStream( FileCtx, NULL, (SUBSTRACT_CLASTERS | INCL_FLAGS_COUNTS) );

                    FileCtx->DefragFlags |= (DEFRAG_FILE_LOCKED | DEFRAG_FILE_UNMOVABLE);

                    UpdateDisplayForStream( FileCtx, NULL, (INCL_FLAGS_COUNTS) );

                    VolumeCtx->Statistic.UnmovableFilesNum.QuadPart++;

                    __leave;
                }

                //
                // ADS was not opened - continue with next.
                //
                continue;
            }

            //
            // Update Display - file name 
            //
            if ( (DEFRAG_JOB_OPTION_SHOW_PROGRESS & VolumeCtx->JobCtx->Options) )
            {
                Result = DfrgGetFileName( VolumeCtx->JobCtx, FileCtx, &(VolumeCtx->JobCtx->CurrentFileName) );
                if ( NO_ERROR == Result )
                {
                    //
                    // TODO: Optimize
                    //
                    UpdateCurrentNameStatistic( VolumeCtx->JobCtx );
                }
            }

#ifdef _DEBUG
            Result = DfrgGetStreamClustersMap( FileCtx, &StreamClustersMap );
            if ( NO_ERROR != Result || !StreamClustersMap )
            {
                swprintf( Mess, L"      %ls '%ls' [%I64d]",
                        (FileCtx->DefragFlags & DEFRAG_FILE_DIRECTORY) ? L"Dir " : L"File",
                        FileCtx->FName, 
                        FileCtx->TotalClustersNum );
                OutMessage( Mess );
                OutMessage( L"      DfrgGetStreamClustersMap 1 error" );
                __leave;
            }

            if ( CheckStreamMaps( StreamCtx, StreamClustersMap, TRUE ) )
            {
                OutMessage( L"      Wrong map. File was modified ???" );
            }

            free( StreamClustersMap );
            StreamClustersMap = NULL;
#endif

            //
            // TODO: Try to move stream in one pass.
            // Update Display :
            // - set processing mark on stream clusters;
            // - remove current stream info from display map.
            //UpdateDisplayForStream( FileCtx, StreamCtx, (SUBSTRACT_CLASTERS | INCL_FLAGS_COUNTS | INCL_INPROC_COUNTS | INCL_SPACE_COUNTS) );
            //

            //
            // Iterate through cluster map.
            //
            BOOLEAN                     StreamWasProcessed = FALSE;
            DEFRAG_FILES_EXTENT         Extent, ExtentToMove;

            Extent.StreamCtx = StreamCtx;
            Extent.StartVcn  = StreamCtx->ClustersMap->StartingVcn;

            for( DWORD i=0; i<StreamCtx->ClustersMap->ExtentCount; 
                Extent.StartVcn = StreamCtx->ClustersMap->Extents[i].NextVcn, i++ )
            {
                if ( VIRTUAL_LCN == StreamCtx->ClustersMap->Extents[i].Lcn.QuadPart ) 
                {
                    //
                    // Skip virtual (sparce) extent
                    //
                    continue;
                }

                if ( !Gap->Length.QuadPart )
                {
                    //
                    // Gap is empty but some extents remained
                    // Finished with this stream (refresh cluster map) and then exit.
                    //
                    GapIsEmpty = TRUE;
                    //__leave;
                    break;
                }

                Extent.StartLcn = StreamCtx->ClustersMap->Extents[i].Lcn;
                Extent.Length.QuadPart = StreamCtx->ClustersMap->Extents[i].NextVcn.QuadPart - Extent.StartVcn.QuadPart;

                if ( Extent.Length.QuadPart > Gap->Length.QuadPart )
                {
                    //
                    // Extent is too long for this gap
                    //
                    continue;
                }

                if ( !SourceZone || IsGapInZone( (PDEFRAG_LCN_EXTENT)&Extent, (PDEFRAG_LCN_EXTENT)&ExtentToMove, SourceZone ) )
                {
                    //
                    // This extent is in Source zone and should be moved
                    //
#ifdef _DEBUG
                    swprintf( Mess, L"*** Move %ls '%ls' [%I64d]  to  %I64d [%I64d] ",
                            (FileCtx->DefragFlags & DEFRAG_FILE_DIRECTORY) ? L"Dir " : L"File",
                            FileCtx->FName, FileCtx->TotalClustersNum,
                            Gap->StartLcn.QuadPart, Gap->Length.QuadPart);
                    OutMessage( Mess );

                    if ( SourceZone && ExtentToMove.Length.QuadPart != Extent.Length.QuadPart )
                    {
                        //
                        // TODO: Move only part (ExtentToMove) that is in zone
                        //
                        OutMessage( L"        Extent is not fully in zone" );
                        //GetInput();
                    }
#endif
                    //
                    // TODO: ???
                    //
                    if ( Extent.Length.QuadPart > Gap->Length.QuadPart )
                    {
                        GapIsEmpty = TRUE;
                        Extent.Length = Gap->Length;
                    }

                    //
                    // Move extent to gap
                    //
                    DWORD   MoveExtentFlags = 0; //SHOULD_ADJUST_CLUSTER_MAP;
                    //if ( VolumeCtx->GapsTree.SortMode == SortModeLcn ) // SmartModes
                    //{
                    //    MoveExtentFlags |= SHOULD_ADJUST_USED_EXTENT; 
                    //}

                    Result = DfrgMoveExtentToGap( &Extent, Gap, MoveExtentFlags );

                    if ( NO_ERROR != Result )
                    {
                        continue;
                    }

                    //DeleteNode(  &(VolumeCtx->ExtentsTree), &ExtentNode );

                    StreamWasProcessed = TRUE;

                }
                else
                {
                    //
                    // Extent is not in Source zone 
                    //
                    continue;
                }

            } // for( DWORD i=0; i<StreamCtx->ClustersMap->ExtentCount; i++ )

            //
            // Update Display :
            // - remove processing mark from old stream clusters;
            //
            //UpdateDisplayForStream( FileCtx, StreamCtx, INCL_INPROC_COUNTS );

            if ( StreamWasProcessed )
            {
                FileWasProcessed = TRUE;

                //
                // Refresh cluster map and continue with next stream.
                //
                Result = DfrgGetStreamClustersMap( FileCtx, &StreamClustersMap );
                if ( NO_ERROR != Result || !StreamClustersMap )
                {
                    OutMessage( L"      DfrgGetStreamClustersMap 2 error" );
                }
                else
                {
#ifdef _DEBUGW
                    if ( VolumeCtx->JobCtx->Type != DEFRAG_JOB_DEBUG1 &&
                        !CheckStreamMaps( StreamCtx, StreamClustersMap, FALSE ) )
                    {
                        OutMessage( L"      ClustersMap not changed" );
                    }
#endif
                    free( StreamCtx->ClustersMap );
                    StreamCtx->ClustersMap = StreamClustersMap;
                    StreamClustersMap = NULL;
                }

                //
                // TODO.
                //
                if ( !GapIsEmpty )
                {
                    FileCtx->DefragFlags |= DEFRAG_FILE_PROCESSED;
                }
            }

            //
            // Update Display :
            // - add (restore) current stream info to display map.
            //
            //UpdateDisplayForStream( FileCtx, StreamCtx, (INCL_FLAGS_COUNTS | INCL_SPACE_COUNTS) );

            //
            // Close stream handle
            //
            DfrgCloseHandle( &(FileCtx->StreamHandle) );

        } // for ( StreamCtx )

    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        Result = GetExceptionCode();
        DislpayErrorMessage( L"DfrgMoveFileToGap(). Exception !!!", Result, ERR_OS );
    }

    //
    // Close all handles
    //
    DfrgCloseFile( FileCtx );

    if ( StreamClustersMap )
    {
        free( StreamClustersMap );
    }

    if ( FileWasProcessed )
    {
        //
        // TODO.
        //
        FileCtx->DefragFlags |= DEFRAG_FILE_PROCESSED;

        if ( FileCtx->DefaultStream && FileCtx->DefaultStream->ClustersMap )
        {
            if ( !IsStreamFragmented(FileCtx->DefaultStream) )
            {
                if ( (FileCtx->DefragFlags & DEFRAG_FILE_FRAGMENTED) )
                {
                    //
                    // Update Statistic 
                    //
                    if ( (DEFRAG_JOB_OPTION_SHOW_PROGRESS & VolumeCtx->JobCtx->Options) )
                    {
                        VolumeCtx->Statistic.DefragmentedFilesNum.QuadPart++;

                        if ( VolumeCtx->Statistic.FragmentedFilesNum.QuadPart )
                        {
                            DEFRAG_RATIO_TYPE       PrevFileFragmentation = 0.0;

                            if ( FileCtx->TotalClustersNum.QuadPart )
                            {
                                PrevFileFragmentation = 
                                    (FileCtx->TotalFragmentsNum.QuadPart * 100.F) / FileCtx->TotalClustersNum.QuadPart;
                            }

                            VolumeCtx->Statistic.FragmentedFilesFragmentationRatio -= 
                                (PrevFileFragmentation - VolumeCtx->Statistic.FragmentedFilesFragmentationRatio) / 
                                VolumeCtx->Statistic.FragmentedFilesNum.QuadPart;

                            VolumeCtx->Statistic.FragmentedFilesNum.QuadPart--;
                            VolumeCtx->Statistic.FragmentedFilesSize.QuadPart -= 
                                FileCtx->TotalClustersNum.QuadPart * VolumeCtx->NtfsData.BytesPerCluster;
                            VolumeCtx->Statistic.FragmentedFilesExcessNum.QuadPart -= 
                                FileCtx->TotalFragmentsNum.QuadPart;
                        }
                    }

                    //
                    // Update Display & Flags
                    //
                    UpdateDisplayForStream( FileCtx, NULL, (SUBSTRACT_CLASTERS | INCL_FLAGS_COUNTS) );

                    FileCtx->DefragFlags &= ~DEFRAG_FILE_FRAGMENTED;

                    UpdateDisplayForStream( FileCtx, NULL, (INCL_FLAGS_COUNTS) );
                }
            }
            else
            {
                if ( !(FileCtx->DefragFlags & DEFRAG_FILE_FRAGMENTED) )
                {
                    //
                    // Update Display & Flags
                    //
                    UpdateDisplayForStream( FileCtx, NULL, (SUBSTRACT_CLASTERS | INCL_FLAGS_COUNTS) );

                    FileCtx->DefragFlags |= DEFRAG_FILE_FRAGMENTED;

                    UpdateDisplayForStream( FileCtx, NULL, (INCL_FLAGS_COUNTS) );
                }
            }
        }
    }
    else
    {
        Result = DFRG_ERROR_SKIP;
    }

    if ( GapIsEmpty )
    {
        Result = DFRG_ERROR_NOT_ENOUGH_SPACE;
    }


    return Result;

} // end of DfrgMoveFileToGap

//
// Defrag the fragmented files/folders in fastest possible way.
//
int
DfrgSimple(
    IN  PDEFRAG_VOLUME_CONTEXT  VolumeCtx )
{
    int                         Result = NO_ERROR;

    PDEFRAG_FILE_CONTEXT        FileCtx = NULL;
    PDEFRAG_LCN_EXTENT          Gap = NULL;
    PBTR_NODE                   FileNode = NULL;
    PBTR_NODE                   GapNode = NULL;

#ifdef _DEBUG
    WCHAR   Mess[MAX_PATH];
#endif


    __try
    {
        //
        // Mark progress
        //
        VolumeCtx->JobCtx->ProcessedObjectsNum.QuadPart = 0;
        VolumeCtx->JobCtx->ObjectsToProcessNum.QuadPart = 
            VolumeCtx->Statistic.FragmentedFilesSize.QuadPart / VolumeCtx->NtfsData.BytesPerCluster;

        //
        // Set excluded target zone to Mft zone
        //
        PDEFRAG_ZONE                MftZone = NULL;

        if ( NtfsDisk == VolumeCtx->DiskType )
        {
            DfrgBuildSimpleZones( VolumeCtx, &MftZone );
        }


#ifdef _DEBUG
        OutMessage( L"" );
        swprintf( Mess, L"DfrgSimple. Total %I64d  Free %I64d  Reserved %I64d",
                        VolumeCtx->NtfsData.TotalClusters,
                        VolumeCtx->NtfsData.FreeClusters,
                        VolumeCtx->NtfsData.TotalReserved );
        OutMessage( Mess );

        if ( MftZone )
        {
            swprintf( Mess, L"MftZone %I64d .. %I64d",
                            MftZone->Extents[0].StartLcn, MftZone->Extents[0].StartLcn.QuadPart + MftZone->Extents[0].Length.QuadPart );
            OutMessage( Mess );
        }

        OutMessage( L"Iterate gaps\n" );
        GetInput();
#endif
        //
        // Iterate through gaps list and fill gaps with appropriate files.
        //
        GapNode = FindFirstNode( &(VolumeCtx->GapsTree), LEFT_TO_RIGHT );

        FileCtx = GetNextMoveableFile( 
                        &(VolumeCtx->FilesTree), 
                        &FileNode, 
                        LEFT_TO_RIGHT, 
                        FIND_FIRST_ITEM,
                        DEFRAG_FILE_FRAGMENTED ); // DEFRAG_FILE_STREAMED 
        if ( !FileCtx )
        {
            __leave;
        }

        DEFRAG_LCN_EXTENT  GapToUse;

        while ( GapNode )
        {
            if ( !ShouldContinue( VolumeCtx->JobCtx ) )
            {
                Result = DFRG_ERROR_ABORTED;
                __leave;
            }

            Gap = (PDEFRAG_LCN_EXTENT)(GapNode->Body);
#ifdef _DEBUGW
            swprintf( Mess, L"   Gap %I64d [%I64d]",
                    Gap->StartLcn.QuadPart, Gap->Length.QuadPart );
            OutMessage( Mess );
#endif

            if ( !IsGapInZone( Gap, &GapToUse, MftZone ) )
            {
                do
                {
                    if ( FileCtx->TotalClustersNum.QuadPart > Gap->Length.QuadPart )
                    {
                        //
                        // Gap is too small. Continue with next.
                        //
#ifdef _DEBUGW
                        swprintf( Mess, L"   Gap tail  %I64d [%I64d] is too small for %ls %ls [%I64d]. Continue with next",
                                Gap->StartLcn.QuadPart, Gap->Length.QuadPart,
                                (FileCtx->DefragFlags & DEFRAG_FILE_DIRECTORY) ? L"Dir " : L"File",
                                FileCtx->FName, FileCtx->TotalClustersNum );
                        OutMessage( Mess );
#endif
                        Result = DFRG_ERROR_NOT_ENOUGH_SPACE;
                        break;
                    }

                    //
                    // Move file.
                    // TODO: use 2 pass strategy:
                    // - on first move totally match items;
                    // - on second move remained.
                    //

                    //if ( FileCtx->DefragFlags & DEFRAG_FILE_STREAMED ||
                    if ( FileCtx->DefragFlags & DEFRAG_FILE_FRAGMENTED )
                    {
                        Result = DfrgMoveFileToGap( FileCtx, NULL, Gap );

                        if ( DFRG_ERROR_NOT_ENOUGH_SPACE == Result ) // NO_ERROR == Result || 
                        {
                            //
                            // Gap is empty. Continue with next.
                            //
#ifdef _DEBUG
                            OutMessage( L"Gap is empty. Continue with next" );
                            //GetInput();
#endif
                            break;
                        }
                    }

                    //
                    // If we finished with current file (successfully or not) - continue with next.
                    // Skip unmovable and not interest files.
                    //
                    FileCtx = GetNextMoveableFile( 
                                    &(VolumeCtx->FilesTree), 
                                    &FileNode, 
                                    LEFT_TO_RIGHT, 
                                    FIND_NEXT_ITEM,
                                    DEFRAG_FILE_FRAGMENTED ); // DEFRAG_FILE_STREAMED 
                    if ( !FileCtx )
                    {
                        __leave;
                    }

                } while ( Gap->Length.QuadPart );

            } // if ( !IsGapInZone( Gap, MftZone ) )
#ifdef _DEBUG
            else
            {
                OutMessage( L"   is in Mft Zone - skip" );
            }
#endif

            //
            // Mark progress
            //
            //VolumeCtx->JobCtx->ProcessedObjectsNum.QuadPart++;

            //
            // Get next gap
            //
            //Gap->GapFlags != DEFRAG_GAP_PROCESSED;
            GapNode = FindNextNode( &(VolumeCtx->GapsTree), LEFT_TO_RIGHT );

        } // while ( GapNode )

    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        Result = GetExceptionCode();
        DislpayErrorMessage( L"DfrgSimple(). Exception !!!", Result, ERR_OS );
    }

    if ( !GapNode )
    {
#ifdef _DEBUG
        OutMessage( L"Gaps pool end" );
#endif
        Result = NO_ERROR;
    }

    if ( !FileNode )
    {
#ifdef _DEBUG
        OutMessage( L"Files pool end" );
        PrintFileList( VolumeCtx, &(VolumeCtx->FilesTree), TRUE, 
            (DEFRAG_FILE_FRAGMENTED | DEFRAG_FILE_UNMOVABLE | DEFRAG_FILE_META | DEFRAG_FILE_LOCKED) );
        PrintGapsTree( &(VolumeCtx->GapsTree), FALSE );
#endif
        Result = NO_ERROR;
    }


    return Result;

} // end of DfrgSimple

