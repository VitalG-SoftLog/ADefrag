/*
    Defrag Engine medium level functions module

    Module name:

        DefragUtils.cpp

    Abstract:

        Defrag Engine medium level functions module. 
        Contains medium level defragmentation functions.

    $Header: /home/CVS_DEFRAG/Defrag/Src/Engine/DefragFreeUtils.cpp,v 1.4 2009/12/25 13:48:53 dimas Exp $
    $Log: DefragFreeUtils.cpp,v $
    Revision 1.4  2009/12/25 13:48:53  dimas
    Debug artefacts removed

    Revision 1.3  2009/12/21 17:00:13  dimas
    1. DEFRAG_CMD_GET_MOST_FRAGMENTED and similar requests implemented
    2. Basic Exclude files check added

    Revision 1.2  2009/11/26 15:56:54  dimas
    1. Smart modes improved.
    2. File names support improved.

    Revision 1.1  2009/11/24 14:52:15  dimas
    no message

*/

#include "stdafx.h"

//
// Free the contiguous space at the end of volume.
//
int
DfrgFreeSpace(
    IN  PDEFRAG_VOLUME_CONTEXT  VolumeCtx )
{
    int                         Result = NO_ERROR;

    PDEFRAG_ZONE                TargetZone = NULL, SourceZone = NULL;

    PDEFRAG_FILE_CONTEXT        FileCtx = NULL;
    PDEFRAG_LCN_EXTENT          Gap = NULL;
    PBTR_NODE                   FileNode = NULL;
    PBTR_NODE                   GapNode = NULL;

#ifdef _DEBUG
    WCHAR   Mess[MAX_PATH];
#endif


    __try
    {

#ifdef _DEBUG
        PrintFileList( VolumeCtx, &(VolumeCtx->FilesTree) );
#endif

        //
        // Mark progress
        //
        VolumeCtx->JobCtx->ProcessedObjectsNum.QuadPart = 0;
        VolumeCtx->JobCtx->ObjectsToProcessNum.QuadPart = 
            VolumeCtx->Statistic.FreeSpaceSize.QuadPart / VolumeCtx->NtfsData.BytesPerCluster;

        //
        // Calculate zones
        //
        DfrgBuildFreeSpaceZones( VolumeCtx, &TargetZone, &SourceZone );

#ifdef _DEBUG
        OutMessage( L"" );
        swprintf( Mess, L"DfrgFreeSpace. Total %I64d  Free %I64d  Reserved %I64d",
                        VolumeCtx->NtfsData.TotalClusters,
                        VolumeCtx->NtfsData.FreeClusters,
                        VolumeCtx->NtfsData.TotalReserved );
        OutMessage( Mess );
        swprintf( Mess, L"Target zone1 %I64d .. %I64d, Mft %I64d .. %I64d, zone2 %I64d .. %I64d",
                        TargetZone->Extents[0].StartLcn, TargetZone->Extents[0].Length,
                        VolumeCtx->NtfsData.MftZoneStart, VolumeCtx->NtfsData.MftZoneEnd, 
                        TargetZone->Extents[1].StartLcn, TargetZone->Extents[1].StartLcn.QuadPart + TargetZone->Extents[1].Length.QuadPart );
        OutMessage( Mess );
        swprintf( Mess, L"SourceZone %I64d .. %I64d",
                        SourceZone->Extents[0].StartLcn, SourceZone->Extents[0].StartLcn.QuadPart + SourceZone->Extents[0].Length.QuadPart );
        OutMessage( Mess );

        OutMessage( L"Iterate gaps\n" );
        GetInput();
#endif

        //
        // Iterate through gaps list and fill gaps with appropriate files
        //

        GapNode = FindFirstNode( &(VolumeCtx->GapsTree), LEFT_TO_RIGHT );

        if ( !(FileCtx = GetNextMoveableFile( &(VolumeCtx->FilesTree), &FileNode, LEFT_TO_RIGHT, FIND_FIRST_ITEM)) )
        {
            __leave;
        }

        //
        // TODO: Use Intersection only
        //
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
#ifdef _DEBUGW
            OutMessage( Mess );
#endif
#endif
            if ( IsGapInZone( Gap, &GapToUse, TargetZone ) )
            {
#ifdef _DEBUGW
                wcscat( Mess, L"   is in Target Zone" );
                OutMessage( Mess );
#endif
                do
                {
                    //
                    // Move file.
                    // TODO: use 2 pass strategy:
                    // - on first move totally match items;
                    // - on second move remained.
                    //

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


                    Result = DfrgMoveFileToGap( FileCtx, SourceZone, Gap );

                    if ( DFRG_ERROR_NOT_ENOUGH_SPACE == Result )
                    {
                        //
                        // Gap is empty. Continue with next.
                        //
#ifdef _DEBUG
                        OutMessage( L"Gap is empty. Continue with next" );
#endif
                        break;
                    }

#ifdef _DEBUG
                    if ( DFRG_ERROR_SKIP != Result )
                    {
                        swprintf( Mess, L"      Gap %I64d [%I64d]",
                                Gap->StartLcn.QuadPart, Gap->Length.QuadPart );
                        OutMessage( Mess );
                    }
#endif
                    if ( !Gap->Length.QuadPart && !(FileCtx->DefragFlags & DEFRAG_FILE_PROCESSED))
                    {
                        //
                        // Gap is empty. Continue with next.
                        // Could take place when move in small gap last file extent wich is in zone and fit in gap exectly ???
                        //
#ifdef _DEBUG
                        OutMessage( L"Gap is empty!!! Continue with next!!!" );
                        //GetInput();
#endif
                        break;
                    }

                    //
                    // If we finished with current file (successfully or not) - continue with next.
                    //
                    if ( !(FileCtx = GetNextMoveableFile( &(VolumeCtx->FilesTree), &FileNode, LEFT_TO_RIGHT, FIND_NEXT_ITEM)) )
                    {
                        __leave;
                    }

                } while ( Gap->Length.QuadPart );

            } // if ( IsGapInZone( Gap, TargetZone ) )

            //
            // Mark progress
            //
            //VolumeCtx->JobCtx->ProcessedObjectsNum.QuadPart++;

            //
            // Get next gap
            //
            //if ( GapNode ) {
            //Gap->GapFlags != DEFRAG_GAP_PROCESSED;
            GapNode = FindNextNode( &(VolumeCtx->GapsTree), LEFT_TO_RIGHT );

        } // while ( GapNode )

    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        Result = GetExceptionCode();
        DislpayErrorMessage( L"DfrgFreeSpace(). Exception !!!", Result, ERR_OS );
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
#endif
         Result = NO_ERROR;
    }


    return Result;

} // end of DfrgFreeSpace


BYTE        SourceZone_buf[100];

//
// Free the largest contiguous space (aggressively) to store the files/folders.
//
int
DfrgFreeSpaceAgressive(
    IN  PDEFRAG_VOLUME_CONTEXT  VolumeCtx )
{
    int                         Result = NO_ERROR;

    PDEFRAG_ZONE                SourceZone = (PDEFRAG_ZONE)&SourceZone_buf; //, TargetZone = NULL;
    PDEFRAG_ZONE                MftZone = NULL;

    DEFRAG_FILE_CONTEXT         FileTemplate = { 0 };
    PDEFRAG_FILE_CONTEXT        FileCtx = NULL;
    PBTR_NODE                   FileNode = NULL;
    PBTR_NODE                   GapNode = NULL;
    PDEFRAG_LCN_EXTENT          Gap = NULL;

#ifdef _DEBUG
    WCHAR   Mess[MAX_PATH];
#endif


    __try
    {

#ifdef _DEBUG
        PrintFileList( VolumeCtx, &(VolumeCtx->FilesTree) );
#endif

        //
        // Mark progress
        //
        VolumeCtx->JobCtx->ProcessedObjectsNum.QuadPart = 0;
        VolumeCtx->JobCtx->ObjectsToProcessNum.QuadPart = 
            VolumeCtx->Statistic.FreeSpaceSize.QuadPart / VolumeCtx->NtfsData.BytesPerCluster;

        //
        // Set excluded target zone to Mft zone
        //

        if ( NtfsDisk == VolumeCtx->DiskType )
        {
            DfrgBuildSimpleZones( VolumeCtx, &MftZone );
        }

        //
        // Step 1. Rebuild gaps list as nessessary.
        //
        if ( SortModeLcn != VolumeCtx->GapsTree.SortMode )
        {
            BTR_TREE                    GapsTree = { 0 };
            BTR_TREE_METHODS            Methods = { 0 };

            //
            // Init new tree
            //
            Methods.FreeBody        = NULL;
            Methods.CompareItems    = CompareGaps;
            Methods.CopyBody        = NULL;
            Methods.PrintBody       = PrintGap;

            InitTree( 
                &GapsTree, 
                &Methods, 
                SortModeLcn );

            //
            // Build new tree from old
            //
            GapNode = FindFirstNode( &(VolumeCtx->GapsTree), LEFT_TO_RIGHT );

            while( GapNode )
            {
                Gap = (PDEFRAG_LCN_EXTENT)(GapNode->Body);
                if ( Gap )
                {
                    AddNode( &GapsTree, Gap );
                }
                GapNode = FindNextNode( &(VolumeCtx->GapsTree), LEFT_TO_RIGHT );
            }

            //
            // Release old tree
            //
            ReleaseTree( &(VolumeCtx->GapsTree) );

            //
            // Copy new tree
            //
            memcpy( &(VolumeCtx->GapsTree), &GapsTree, sizeof( BTR_TREE ) );
        }
#if 0
        //
        // Use 2 pass strategy:
        // - on first move & defrag totally match items;
        // - on second move remained depending on policy.
        //

        //
        // Iterate through gaps list and fill gaps with appropriate files
        //
        while ( GapNode = FindFirstNode( &(VolumeCtx->GapsTree), LEFT_TO_RIGHT ) )
        {
            if ( !ShouldContinue( VolumeCtx->JobCtx ) )
            {
                Result = DFRG_ERROR_ABORTED;
                __leave;
            }

            Gap = (PDEFRAG_LCN_EXTENT)(GapNode->Body);
#ifdef _DEBUG
            swprintf( Mess, L"   Gap %I64d [%I64d]",
                    Gap->StartLcn.QuadPart, Gap->Length.QuadPart );
#ifdef _DEBUGW
            OutMessage( Mess );
#endif
#endif

            //
            // TODO: Use Intersection only
            //
            DEFRAG_LCN_EXTENT  GapToUse;

            if ( !IsGapInZone( Gap, &GapToUse, MftZone ) )
            {
#ifdef _DEBUG
                wcscat( Mess, L"   is in Target Zone" );
                OutMessage( Mess );
#endif
                do
                {
                    //
                    // Define source zone
                    //
                    SourceZone->ExtentCount = 1;
                    SourceZone->Extents[0].StartLcn.QuadPart = Gap->StartLcn.QuadPart + Gap->Length.QuadPart;
                    SourceZone->Extents[0].Length.QuadPart = 
                        VolumeCtx->NtfsData.TotalClusters.QuadPart - SourceZone->Extents[0].StartLcn.QuadPart;

                    //
                    // Get file 
                    //
                    FileTemplate.TotalClustersNum = Gap->Length;

                    if ( !(FileCtx = GetFileToMove( &(VolumeCtx->FilesTree), &FileTemplate, SourceZone->Extents, &FileNode ) ) )
                    {
                        //
                        // Could not get file for this gap. Continue with next.
                        //
#ifdef _DEBUGW
                        wcscpy( Mess, L"   Could not get file for this gap" );
                        OutMessage( Mess );
#endif
                        break;
                    }

                    //
                    // Move file.
                    //
                    Result = DfrgMoveFileToGap( FileCtx, SourceZone, Gap );

                    if ( DFRG_ERROR_NOT_ENOUGH_SPACE == Result )
                    {
                        //
                        // Gap is empty. Continue with next.
                        //
#ifdef _DEBUG
                        OutMessage( L"Gap is empty. Continue with next" );
#endif
                        break;
                    }

#ifdef _DEBUG
                    if ( DFRG_ERROR_SKIP != Result )
                    {
                        swprintf( Mess, L"      Gap %I64d [%I64d]",
                                Gap->StartLcn.QuadPart, Gap->Length.QuadPart );
                        OutMessage( Mess );
                    }
#endif
                    if ( !Gap->Length.QuadPart && !(FileCtx->DefragFlags & DEFRAG_FILE_PROCESSED))
                    {
                        //
                        // Gap is empty. Continue with next.
                        // Could take place when move in small gap last file extent wich is in zone and fit in gap exectly ???
                        //
#ifdef _DEBUG
                        OutMessage( L"Gap is empty!!! Continue with next!!!" );
                        //GetInput();
#endif
                        break;
                    }

                    //
                    // If we finished with current file (successfully or not) - continue with next.
                    // Skip unmovable and not interest files.
                    //
                } while ( Gap->Length.QuadPart );

            } // if ( IsGapInZone( Gap, TargetZone ) )

            //
            // Mark progress
            //
            //VolumeCtx->JobCtx->ProcessedObjectsNum.QuadPart++;

            //
            // Get next gap
            //
            //UpdateGap( &(VolumeCtx->GapsTree), &GapNode, &Gap );
            // delete gap from tree
            //
            DeleteNode(  &(VolumeCtx->GapsTree), GapNode );

            //GapNode = FindNextNode( &(VolumeCtx->GapsTree), LEFT_TO_RIGHT );

        } // while ( GapNode )

        //
        // Pass 2. Move remained.
        // if free space - slide files only
        // if free space aggressivly - do not fragment space - slide files and move extents 
        // if force together - can fragment space 
        //
        // Rebuild gaps tree first.
        //
        Result = DfrgGetVolumeSpaceMap( VolumeCtx, TRUE );
        if ( NO_ERROR != Result )
        {
            __leave;
        }
#endif

        //
        // Iterate through gaps list and fill gaps with appropriate extents
        //
        GapNode = FindFirstNode( &(VolumeCtx->GapsTree), LEFT_TO_RIGHT );

        while ( GapNode )
        {
            if ( !ShouldContinue( VolumeCtx->JobCtx ) )
            {
                Result = DFRG_ERROR_ABORTED;
                __leave;
            }

            Gap = (PDEFRAG_LCN_EXTENT)(GapNode->Body);
#ifdef _DEBUG
            swprintf( Mess, L"   Gap %I64d [%I64d]",
                    Gap->StartLcn.QuadPart, Gap->Length.QuadPart );
#ifdef _DEBUGW
            OutMessage( Mess );
#endif
#endif

            //
            // TODO: Use Intersection only
            //
            DEFRAG_LCN_EXTENT  GapToUse;

            if ( !IsGapInZone( Gap, &GapToUse, MftZone ) )
            {
#ifdef _DEBUG
                wcscat( Mess, L"   is in Target Zone" );
                OutMessage( Mess );
#endif
                do
                {
                    //
                    // Define source zone
                    //
                    SourceZone->ExtentCount = 1;
                    SourceZone->Extents[0].StartLcn.QuadPart = Gap->StartLcn.QuadPart + Gap->Length.QuadPart;
                    SourceZone->Extents[0].Length.QuadPart = 
                        VolumeCtx->NtfsData.TotalClusters.QuadPart - SourceZone->Extents[0].StartLcn.QuadPart;

                    //
                    //
                    //
                    //switch()
                    //{
                    //}

                    //
                    // Get extent
                    //
                    FileTemplate.TotalClustersNum = Gap->Length;

                    if ( !(FileCtx = GetFileToMove( &(VolumeCtx->FilesTree), &FileTemplate, SourceZone->Extents, &FileNode ) ) )
                    {
                        //
                        // Could not get file for this gap. Continue with next.
                        //
#ifdef _DEBUGW
                        wcscpy( Mess, L"   Could not get file for this gap" );
                        OutMessage( Mess );
#endif
                        break;
                    }

                    //
                    // Move file.
                    //
                    Result = DfrgMoveFileToGap( FileCtx, SourceZone, Gap );

                    if ( DFRG_ERROR_NOT_ENOUGH_SPACE == Result )
                    {
                        //
                        // Gap is empty. Continue with next.
                        //
#ifdef _DEBUG
                        OutMessage( L"Gap is empty. Continue with next" );
#endif
                        break;
                    }

#ifdef _DEBUG
                    if ( DFRG_ERROR_SKIP != Result )
                    {
                        swprintf( Mess, L"      Gap %I64d [%I64d]",
                                Gap->StartLcn.QuadPart, Gap->Length.QuadPart );
                        OutMessage( Mess );
                    }
#endif
                    if ( !Gap->Length.QuadPart && !(FileCtx->DefragFlags & DEFRAG_FILE_PROCESSED))
                    {
                        //
                        // Gap is empty. Continue with next.
                        // Could take place when move in small gap last file extent wich is in zone and fit in gap exectly ???
                        //
#ifdef _DEBUG
                        OutMessage( L"Gap is empty!!! Continue with next!!!" );
                        //GetInput();
#endif
                        break;
                    }

                    //
                    // If we finished with current file (successfully or not) - continue with next.
                    // Skip unmovable and not interest files.
                    //
                } while ( Gap->Length.QuadPart );

            } // if ( IsGapInZone( Gap, TargetZone ) )

            //
            // Get next gap
            //
            //UpdateGap( &(VolumeCtx->GapsTree), &GapNode, &Gap );
            // delete gap from tree
            //
            DeleteNode(  &(VolumeCtx->GapsTree), GapNode );

            //GapNode = FindNextNode( &(VolumeCtx->GapsTree), LEFT_TO_RIGHT );

        } // while ( GapNode )

    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        Result = GetExceptionCode();
        DislpayErrorMessage( L"DfrgFreeSpace(). Exception !!!", Result, ERR_OS );
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
#endif
         Result = NO_ERROR;
    }


    return Result;

} // end of DfrgFreeSpaceAgressive


//
// Move the files and directories to end or beginning of the disk forcefully
//
int
DfrgForceTogether(
    IN  PDEFRAG_VOLUME_CONTEXT  VolumeCtx )
{
    int                         Result = NO_ERROR;

    PDEFRAG_ZONE                MftZone = NULL, SourceZone = NULL;

    PBTR_NODE                   GapNode = NULL;
    PBTR_NODE                   NextGapNode = NULL;
    PDEFRAG_LCN_EXTENT          Gap = NULL;

    PBTR_NODE                   ExtentNode = NULL;
    PDEFRAG_FILES_EXTENT        Extent = NULL;

    PDEFRAG_FILE_CONTEXT        FileCtx = NULL;

    BOOLEAN                     BeAgressive = FALSE;
    BOOLEAN                     BePersistent = FALSE;

    DWORD                       MoveExtentFlags = SHOULD_ADJUST_CLUSTER_MAP;
    //DWORD                       MoveExtentFlags = (SHOULD_OPEN_STREAM | SHOULD_ADJUST_USED_GAP); // SHOULD_ADJUST_USED_EXTENT  | SHOULD_ADJUST_NEW_GAP;

    BTR_TREE                    NewGapsTree = { 0 };
    BTR_TREE_METHODS            Methods = { 0 };



#ifdef _DEBUG
    WCHAR   Mess[MAX_PATH*2];
#endif


__try
{

#ifdef _DEBUGW
    PrintFileList( VolumeCtx, &(VolumeCtx->FilesTree) );
#endif

    //
    // Mark progress
    //
    VolumeCtx->JobCtx->ProcessedObjectsNum.QuadPart = 0;
    VolumeCtx->JobCtx->ObjectsToProcessNum.QuadPart = 
        (VolumeCtx->Statistic.FreeSpaceSize.QuadPart / VolumeCtx->NtfsData.BytesPerCluster);


    //
    // Init tree for gaps in source zone
    //
    Methods.FreeBody        = NULL;
    Methods.CompareItems    = CompareGaps;
    Methods.CopyBody        = NULL;
    Methods.PrintBody       = PrintGap;

    InitTree( &NewGapsTree, &Methods, SortModeLcn );


    //
    // Calculate zones
    // Set excluded target zone to Mft zone and SourceZone to the end of disk
    // TODO: adjust (remove from list) gaps in Mft zone
    //
    DfrgBuildFreeZones( VolumeCtx, &MftZone, &SourceZone );

#ifdef _DEBUG
    OutMessage( L"" );
    swprintf( Mess, L"DfrgForceTogether. Clusters: Total %I64d  Free %I64d  Reserved %I64d",
                    VolumeCtx->NtfsData.TotalClusters,
                    VolumeCtx->NtfsData.FreeClusters,
                    VolumeCtx->NtfsData.TotalReserved );
    OutMessage( Mess );
    if ( NtfsDisk == VolumeCtx->DiskType )
    {
        swprintf( Mess, L"Mft zone %I64d [%I64d] .. %I64d",
                    MftZone->Extents[0].StartLcn.QuadPart, MftZone->Extents[0].Length.QuadPart,
                    MftZone->Extents[0].StartLcn.QuadPart + MftZone->Extents[0].Length.QuadPart );
        OutMessage( Mess );
    }
    swprintf( Mess, L"Source zone %I64d [%I64d] .. %I64d",
                    SourceZone->Extents[0].StartLcn.QuadPart, SourceZone->Extents[0].Length.QuadPart,
                    SourceZone->Extents[0].StartLcn.QuadPart + SourceZone->Extents[0].Length.QuadPart );
    OutMessage( Mess );
#endif


    BOOLEAN     ExtentsMoveDirection;
    //ExtentsMoveDirection = LEFT_TO_RIGHT;    // move files/extents forward to the end of disk
    ExtentsMoveDirection = RIGHT_TO_LEFT; // move files/extents up to the begining of disk

    BOOLEAN     ExtentSearchDirection;
    //ExtentSearchDirection = LEFT_TO_RIGHT; // look for extents in source zone from begining to the end 
    ExtentSearchDirection = RIGHT_TO_LEFT; // look for extents in source zone from end to the begining 

    BOOLEAN     GapSearchDirection;
    //GapSearchDirection = LEFT_TO_RIGHT; // iterate gaps from small to large
    GapSearchDirection = RIGHT_TO_LEFT; // iterate gaps from large to small 



    BOOLEAN                     ContinueWithExtent = FALSE;
    BOOLEAN                     ContinueWithGap = FALSE;

    DEFRAG_FILES_EXTENT         ExtentToMove;

    DEFRAG_LCN_EXTENT           Intersection;
    LONGLONG                    IntersectionLength = 0;


    do
    {
        //
        // Iterate through gaps list and fill gaps with appropriate extents from source zone
        //
        ContinueWithExtent = FALSE;
        ContinueWithGap = FALSE;

        DfrgCloseFile( FileCtx );

//StartIterate:

#ifdef _DEBUG
        OutMessage( L"Iterate gaps\n" );
        GetInput();
#endif

        GapNode = FindFirstNode( &(VolumeCtx->GapsTree), GapSearchDirection );

        while ( GapNode )
        {
            if ( !ShouldContinue( VolumeCtx->JobCtx ) )
            {
                Result = DFRG_ERROR_ABORTED;
                __leave;
            }

            if ( !ContinueWithGap )
            {
                NextGapNode = FindNextNode( &(VolumeCtx->GapsTree), GapSearchDirection );
            }

            ContinueWithGap = FALSE;

            Gap = (PDEFRAG_LCN_EXTENT)(GapNode->Body);
#ifdef _DEBUG
            swprintf( Mess, L"***  Gap %I64d [%I64d]",
                    Gap->StartLcn.QuadPart, Gap->Length.QuadPart );
            OutMessage( Mess );
#endif


            BOOLEAN                 UseThisGap = TRUE;
            DEFRAG_LCN_EXTENT       ZoneIntersection;
            LONGLONG                ZoneIntersectionLength = 0;

            if ( UseThisGap )
            {
                ZoneIntersectionLength = GetExtentsIntersection( 
                                            &(SourceZone->Extents[0]), 
                                            Gap, 
                                            &ZoneIntersection );
                if ( ZoneIntersectionLength > 0 )
                {
#ifdef _DEBUGW
                    swprintf( Mess, L"   Intersection with SourceZone: %I64d [%I64d]",
                            ZoneIntersection.StartLcn.QuadPart, ZoneIntersection.Length.QuadPart );
                    OutMessage( Mess );
#endif
                    if ( ZoneIntersectionLength == Gap->Length.QuadPart )
                    {
                        //
                        // Gap is in source zone.
                        // Move it to special tree and substract length from progress
                        //
                        UseThisGap = FALSE;

                        MarkProgress( VolumeCtx, -Gap->Length.QuadPart );

                        RemoveNode( &(VolumeCtx->GapsTree), GapNode );
                        free( GapNode );
                        GapNode = NULL;

                        VolumeCtx->GapsTree.Current = NextGapNode;

                        DfrgInsertGapNode( &NewGapsTree, &GapNode, &Gap );
#ifdef _DEBUG
                        swprintf( Mess, L"   Gap moved to new tree as : %I64d [%I64d] .. %I64d",
                                Gap->StartLcn.QuadPart, Gap->Length.QuadPart,
                                Gap->StartLcn.QuadPart + Gap->Length.QuadPart );
                        OutMessage( Mess );
#endif
                    }
                    else
                    {
                        if ( ZoneIntersection.StartLcn.QuadPart > Gap->StartLcn.QuadPart )
                        {
                            Gap->Length.QuadPart -= ZoneIntersection.Length.QuadPart;
                        }
                        else
                        {
                            Gap->StartLcn         = ZoneIntersection.StartLcn;
                            Gap->Length.QuadPart -= ((Gap->StartLcn.QuadPart + Gap->Length.QuadPart) - ZoneIntersection.StartLcn.QuadPart);
                        }

                        //
                        // TODO: Insert gaps tail in NewGapsTree
                        //
                    }
                }
            }


            if ( UseThisGap )
            {
                ZoneIntersectionLength = GetExtentsIntersection( 
                                            &(MftZone->Extents[0]), 
                                            Gap, 
                                            &ZoneIntersection );
                if ( ZoneIntersectionLength > 0 )
                {
#ifdef _DEBUG
                    swprintf( Mess, L"   Intersection with MftZone: %I64d [%I64d]",
                            ZoneIntersection.StartLcn.QuadPart, ZoneIntersection.Length.QuadPart );
                    OutMessage( Mess );
#endif
                    if ( ZoneIntersectionLength == Gap->Length.QuadPart )
                    {
                        UseThisGap = FALSE;
//#ifdef _DEBUG
//                        OutMessage( L"Delete Gap" );
//#endif
                        //DeleteNode( &(VolumeCtx->GapsTree), GapNode );
                        //VolumeCtx->GapsTree.Current = NextGapNode;
                    }
                    else
                    {
                        if ( ZoneIntersection.StartLcn.QuadPart > Gap->StartLcn.QuadPart )
                        {
                            Gap->Length.QuadPart -= ZoneIntersection.Length.QuadPart;
                        }
                        else
                        {
                            Gap->StartLcn         = ZoneIntersection.StartLcn;
                            Gap->Length.QuadPart -= ((Gap->StartLcn.QuadPart + Gap->Length.QuadPart) - ZoneIntersection.StartLcn.QuadPart);
                        }
                    }
                }
            }

            //
            // TODO: If zone modified - modify tree
            //

            if ( UseThisGap )
            {
#ifdef _DEBUG
                swprintf( Mess, L"   use gap %I64d [%I64d] %ls",
                        Gap->StartLcn.QuadPart, Gap->Length.QuadPart,
                        ContinueWithExtent ? L"(continue with Extent)" : L"");
                OutMessage( Mess );
#endif

                do // while ( Gap->Length.QuadPart );
                {

                //__try
                //{
                    //
                    // Find extent for this gap
                    //
                    if ( !ContinueWithExtent || !Extent )
                    {
                        do // while ( !Extent )
                        {
                            Extent = GetExtentForGap(
                                &(VolumeCtx->ExtentsTree),
                                Gap,
                                &(SourceZone->Extents[0]), 
                                SortModeSize,
                                ExtentSearchDirection, 
                                BePersistent ? _FIND_MAX_ : _FIND_LE_,
                                &ExtentNode );

                            ContinueWithExtent = FALSE;

                            if ( !Extent )
                            {
                                //
                                // Could not find extent for gap. Continue with next gap.
                                //
#ifdef _DEBUG
                                swprintf( Mess, L"      Could not find extent for gap." );
                                OutMessage( Mess );
#endif

                                if ( RIGHT_TO_LEFT == GapSearchDirection )
                                {
                                    if ( !BePersistent )
                                    {
                                        //
                                        // TODO: Do not ever try to find zone to slide to. 
                                        //

                                        //
                                        // Stop iteration
                                        // TODO: Could it changed if zone grove up ???
                                        //
#ifdef _DEBUG
                                        swprintf( Mess, L"*** Stop iteration - 'left' gaps are smaller." );
                                        OutMessage( Mess );
#endif
                                        NextGapNode = NULL;
                                    }
                                }

                                break;
                            }

                            FileCtx = Extent->StreamCtx->FileCtx;

                            BOOLEAN     ExtentIsUnmoveable = 
                                CheckIsStreamUnmoveable( VolumeCtx, Extent->StreamCtx, SHOULD_OPEN_STREAM );

                            if ( ExtentIsUnmoveable )
                            {
                                ExpandZoneWithExtent( 
                                    VolumeCtx,
                                    SourceZone, 
                                    ExtentNode, 
                                    ExtentsMoveDirection, 
                                    0 ); //SHOULD_ADJUST_USED_EXTENT );

                                //
                                // Delete extent
                                //
#ifdef _DEBUG
                                OutMessage( L"   Delete extent" );
#endif
                                DeleteNode(  &(VolumeCtx->ExtentsTree), ExtentNode );

                                //VolumeCtx->Extent.Current = NextExtentNode;

                                ExtentNode = NULL;
                                Extent = NULL;
                            }

                        } while ( !Extent );

                        if ( !Extent )
                        {
                            break;
                        }

                        //
                        // Check if we can/should slide extent
                        //
                        IntersectionLength = GetExtentsIntersection( 
                                                    Gap, 
                                                    (PDEFRAG_LCN_EXTENT)Extent, 
                                                    &Intersection );

                        if ( IntersectionLength == 0 )
                        {
                            //
                            // Extent and Gap are neighbours
                            //
#ifdef _DEBUG
                            swprintf( Mess, L"   *** Should slide Extent  %I64d [%I64d] to Gap %I64d [%I64d]",
                                    Extent->StartLcn.QuadPart, Extent->Length.QuadPart,
                                    Gap->StartLcn.QuadPart, Gap->Length.QuadPart );
                            OutMessage( Mess );
#endif
                            Result = DfrgSlideExtentToGap( Extent, Gap, SHOULD_ADJUST_CLUSTER_MAP ); // MoveExtentFlags );

                            if ( NO_ERROR == Result )
                            {
                                DfrgCloseFile( FileCtx );

                                ContinueWithGap = TRUE;
                                break;
                            }
                        } // if ( IntersectionLength == 0 ) - slide
                        else 
                        {
                            //
                            // Extent and Gap are not neighbours (intersection is impossible here)
                            //
                            if ( !BeAgressive && (Extent->Length.QuadPart > Gap->Length.QuadPart) )
                            {
                                ContinueWithExtent = FALSE;
                                ContinueWithGap = FALSE;
                                if ( RIGHT_TO_LEFT == GapSearchDirection &&
                                    Extent->Length.QuadPart / 16 > Gap->Length.QuadPart ) 
                                {
                                    //
                                    // Stop iteration
                                    // TODO: Could it changed if zone grove up ???
                                    //
#ifdef _DEBUG
                                    swprintf( Mess, L"*** Stop iteration - 'left' gaps are smaller." );
                                    OutMessage( Mess );
#endif
                                    NextGapNode = NULL;
                                }
                                break;
                            }
                        }


                        //
                        // Define extent's part (tail) that is in zone
                        //
                        IntersectionLength = GetExtentsIntersection( 
                                                    &(SourceZone->Extents[0]), 
                                                    (PDEFRAG_LCN_EXTENT)Extent, 
                                                    &Intersection );

                        if ( RIGHT_TO_LEFT == ExtentsMoveDirection )
                        {
                            ExtentToMove.StartLcn   = Intersection.StartLcn;
                            ExtentToMove.Length     = Intersection.Length;
                            ExtentToMove.StartVcn.QuadPart = 
                                Extent->StartVcn.QuadPart + Extent->Length.QuadPart - IntersectionLength;
                            ExtentToMove.StreamCtx  = Extent->StreamCtx;
                        }
                        else
                        {
                        }

                        if ( IntersectionLength > 0 )
                        {
                            //
                            // TODO:
                            // Devide extent onto two parts
                            //
                            //PDEFRAG_FILES_EXTENT    NewExtent = (DEFRAG_FILES_EXTENT) malloc( sizeof(DEFRAG_FILES_EXTENT) );
                        }

                    }  // if ( !ContinueWithExtent )


                    if ( ExtentToMove.Length.QuadPart > Gap->Length.QuadPart ) 
                    {
                        //
                        // Gap is too small. 
                        //
#ifdef _DEBUGW
                        swprintf( Mess, L"      Gap %I64d [%I64d] is too small for Extent %I64d [%I64d] (file %ls).",
                                Gap->StartLcn.QuadPart, Gap->Length.QuadPart,
                                ExtentToMove.StartLcn.QuadPart, ExtentToMove.Length.QuadPart,
                                (FileCtx && FileCtx->FName) ? FileCtx->FName : L"NULL" );
                        OutMessage( Mess );
#endif
                        if ( !BeAgressive )
                        {
                            //
                            // Continue with next.
                            //
                            ContinueWithExtent = FALSE;

                            Result = DFRG_ERROR_NOT_ENOUGH_SPACE;
                            break;
                        }

                        ContinueWithExtent = TRUE;

                        ExtentToMove.Length = Gap->Length;

                    } // Gap too small


                    BOOLEAN     IsFileFragmented = !!(FileCtx->DefragFlags & DEFRAG_FILE_FRAGMENTED);

                    Result = DfrgMoveExtentToGap( &ExtentToMove, Gap, MoveExtentFlags );

                    if ( NO_ERROR != Result )
                    {
                        //
                        // Extent was not moved. 
                        // TODO:
                        // Extend Source zone, Mark as UNMOVEABLE & delete.
                        //
#ifdef _DEBUG
                        swprintf( Mess, L"      Extent was not moved. Mark as UNMOVEABLE" );
                        OutMessage( Mess );
#endif
                        ContinueWithExtent = FALSE;
                    }
                    else
                    {
                        //
                        // Extent was moved. 
                        //
#ifdef _DEBUG
                        OutMessage( L"      Extent was moved." );
#endif
                        //
                        // 1. Create new gap on the place of extent
                        //
                        PDEFRAG_LCN_EXTENT  NewGap = (PDEFRAG_LCN_EXTENT) malloc( sizeof(DEFRAG_LCN_EXTENT) );
                        PBTR_NODE           NewGapNode = NULL;
                        if ( NewGap )
                        {
                            NewGap->StartLcn = ExtentToMove.StartLcn;
                            NewGap->Length   = ExtentToMove.Length;

                            DfrgInsertGapNode( &NewGapsTree, &NewGapNode, &NewGap );
#ifdef _DEBUG
                            swprintf( Mess, L"   New Gap created in new tree as : %I64d [%I64d] .. %I64d",
                                    NewGap->StartLcn.QuadPart, NewGap->Length.QuadPart,
                                    NewGap->StartLcn.QuadPart + NewGap->Length.QuadPart );
                            OutMessage( Mess );
#endif
                        }

                        //
                        // 2. Adjust extent.
                        //
                        ExtentToMove.StartVcn.QuadPart  += ExtentToMove.Length.QuadPart;
                        ExtentToMove.StartLcn.QuadPart  += ExtentToMove.Length.QuadPart;
                        IntersectionLength              -= ExtentToMove.Length.QuadPart;
                        ExtentToMove.Length.QuadPart     = IntersectionLength;

                        if ( ContinueWithExtent )
                        {
                            if ( !ExtentToMove.Length.QuadPart )
                            {
                                ContinueWithExtent = FALSE;
                            }
                        }
#ifdef _DEBUG
                        swprintf( Mess, L"      Used Gap adjusted to %I64d [%I64d],  Extent to %I64d [%I64d]",
                                Gap->StartLcn.QuadPart, Gap->Length.QuadPart,
                                ExtentToMove.StartLcn.QuadPart, ExtentToMove.Length.QuadPart );
                        OutMessage( Mess );
#endif


                        //
                        // 3. Mark fragmented status on display map
                        //
                        if ( (DEFRAG_JOB_OPTION_SHOW_PROGRESS & VolumeCtx->JobCtx->Options) &&
                            Extent->StreamCtx == FileCtx->DefaultStream )
                        {
                            BOOLEAN IsFileFragmentedNow = IsStreamFragmented( Extent->StreamCtx );

                            if ( IsFileFragmented != IsFileFragmentedNow )
                            {
                                //
                                // Redraw map for this stream - Update Display & Flags
                                //
                                UpdateDisplayForStream( FileCtx, NULL, (SUBSTRACT_CLASTERS | INCL_FLAGS_COUNTS) );

                                if ( IsFileFragmentedNow )
                                {
                                    FileCtx->DefragFlags |= DEFRAG_FILE_FRAGMENTED;
                                }
                                else
                                {
                                    FileCtx->DefragFlags &= ~DEFRAG_FILE_FRAGMENTED;
                                }

                                UpdateDisplayForStream( FileCtx, NULL, (INCL_FLAGS_COUNTS) );

                                IsFileFragmented = IsFileFragmentedNow;
                            }
                        }

                    }

                    if ( !ContinueWithExtent )
                    {
                        DfrgCloseFile( FileCtx );
#ifdef _DEBUG
                        swprintf( Mess, L"      Delete extent & continue with next extent." );
                        OutMessage( Mess );
#endif
                        if ( ExtentNode )
                        {
                            //
                            // Delete extent.
                            //
                            DeleteNode(  &(VolumeCtx->ExtentsTree), ExtentNode );

                            ExtentNode  = NULL;
                            Extent      = NULL;
                        }

                    } // if ( !ContinueWithExtent )

                    if ( !Gap->Length.QuadPart )
                    {
                        ContinueWithGap = FALSE;
#ifdef _DEBUG
                        OutMessage( L"      Delete zero gap." );
#endif
                        DeleteNode( &(VolumeCtx->GapsTree), GapNode );
                        VolumeCtx->GapsTree.Current = NextGapNode;

                        GapNode = NULL;
                        Gap     = NULL;

                        if ( !ContinueWithExtent )
                        {
#ifdef _DEBUG
                            //OutMessage( L"### Restart" );
#endif
                            //goto StartIterate;
                        }
                    }
                    else if ( !ContinueWithGap )
                    {
                        //
                        // It's no longer the largest gap.
                        //
#ifdef _DEBUG
                        OutMessage( L"      Update gap tree." );
#endif
                        DfrgUpdateGapNode( &(VolumeCtx->GapsTree), &GapNode, &Gap );
                        VolumeCtx->GapsTree.Current = NextGapNode;

                        if ( NextGapNode && Gap &&
                            ((PDEFRAG_LCN_EXTENT)(NextGapNode->Body))->Length.QuadPart <= 
                            Gap->Length.QuadPart )
                        {
                            ContinueWithGap = TRUE;
                        }
                        else
                        {
#ifdef _DEBUG
                            //OutMessage( L"### Restart" );
#endif
                            //goto StartIterate;
                        }
                    }

                } while ( Gap && Gap->Length.QuadPart );

            } // if ( UseThisGap )

            //
            // Get next gap
            //
            if ( !ContinueWithGap )
            {
#ifdef _DEBUG
                swprintf( Mess, L"      Continue with next gap.\n" );
                OutMessage( Mess );
#endif
                GapNode = NextGapNode;
            }
            else
            {
#ifdef _DEBUG
                swprintf( Mess, L"      Continue with this gap.\n" );
                OutMessage( Mess );
#endif
            }

        } // while ( GapNode )


#ifdef _DEBUG
        OutMessage( L"" );
        swprintf( Mess, L"### Next Pass ###" );
#endif

        if ( (VolumeCtx->JobCtx->Options & DEFRAG_JOB_OPTION_BE_AGRESSIVE) )
        {
            BeAgressive = TRUE;
#ifdef _DEBUG
            wcscat( Mess, L" - Try Agressive policy ###\n" );
#endif
        }

#ifdef _DEBUG
        OutMessage( Mess );
#endif

        BePersistent = !BePersistent;

        //
        // TODO: Final Pass - Lookup source zone for moveable extents and slide them all.
        //

    } while ( BePersistent );
    //} while ( ( BePersistent || BeAgressive ) );

    //
    // Move extents left in Source zone
    //
#ifdef _DEBUG
    OutMessage( L"" );
    OutMessage( L"### Last chance Pass ###" );
#endif

#ifdef _DEBUG
    PrintGapsTree( &NewGapsTree, TRUE );
#endif

    GapNode = FindFirstNode( &NewGapsTree, LEFT_TO_RIGHT );

    while ( GapNode )
    {
        if ( !ShouldContinue( VolumeCtx->JobCtx ) )
        {
            Result = DFRG_ERROR_ABORTED;
            __leave;
        }

        if ( !ContinueWithGap )
        {
            NextGapNode = FindNextNode( &NewGapsTree, LEFT_TO_RIGHT );
        }

        ContinueWithGap = FALSE;


        Gap = (PDEFRAG_LCN_EXTENT)(GapNode->Body);

#ifdef _DEBUG
            swprintf( Mess, L"***  Gap %I64d [%I64d]",
                    Gap->StartLcn.QuadPart, Gap->Length.QuadPart );
            OutMessage( Mess );
#endif

        DEFRAG_FILES_EXTENT         ExtentTemplate = { 0 };

        ExtentTemplate.StartLcn = Gap->StartLcn;

        ExtentNode = FindNode( &(VolumeCtx->ExtentsTree), &ExtentTemplate, SortModeLcn, _FIND_GE_ );
        if ( !ExtentNode )
        {
            break;
        }

        Extent = (PDEFRAG_FILES_EXTENT)(ExtentNode->Body);

        FileCtx = Extent->StreamCtx->FileCtx;

#ifdef _DEBUG
        swprintf( Mess, L"        Extent %I64d [%I64d]",
                Extent->StartLcn.QuadPart, Extent->Length.QuadPart );
        OutMessage( Mess );
#endif

        BOOLEAN     IsExtentUnmoveable = CheckIsStreamUnmoveable( VolumeCtx, Extent->StreamCtx, 0 ); //SHOULD_OPEN_STREAM );

        if ( !IsExtentUnmoveable &&
            GetExtentsIntersection( Gap, (PDEFRAG_LCN_EXTENT)Extent, &Intersection ) == 0 )
        {
            Result = DfrgSlideExtentToGap( Extent, Gap, MoveExtentFlags | SHOULD_OPEN_STREAM );

            DeleteNode( &(VolumeCtx->ExtentsTree), ExtentNode );

            if ( NO_ERROR == Result )
            {
                if ( NextGapNode )
                {
                    PDEFRAG_LCN_EXTENT  NextGap = (PDEFRAG_LCN_EXTENT)(NextGapNode->Body);

                    if ( RIGHT_TO_LEFT == ExtentsMoveDirection )
                    {
                        if ( Gap->StartLcn.QuadPart + Gap->Length.QuadPart ==
                            NextGap->StartLcn.QuadPart )
                        {
                            NextGap->StartLcn = Gap->StartLcn;
                            NextGap->Length.QuadPart += Gap->Length.QuadPart;
                        }
                        else
                        {
                            //
                            // Continue with gap
                            //
#ifdef _DEBUG
                            OutMessage( L"   Continue with gap" );
#endif
                            ContinueWithGap = TRUE;
                            continue;
                        }
                    }
                    else
                    {
                    }
                }
            }
            
        }
        else
        {
            PDEFRAG_FILES_EXTENT    MaxExtent = NULL;
            PBTR_NODE               MaxExtentNode = NULL; 
            LONGLONG                MaxLength = 0;

            do
            {
                Extent = (PDEFRAG_FILES_EXTENT)(ExtentNode->Body);

                FileCtx = Extent->StreamCtx->FileCtx;

                if ( CheckIsStreamUnmoveable( VolumeCtx, Extent->StreamCtx, 0 ) )
                {
                    continue;
                }

                if ( Extent->Length.QuadPart == Gap->Length.QuadPart )
                {
                    MaxExtentNode = ExtentNode;
                    MaxExtent = Extent;
                    MaxLength = Extent->Length.QuadPart;
                    break;
                }
                else if ( Extent->Length.QuadPart < Gap->Length.QuadPart )
                {
                    if ( Extent->Length.QuadPart > MaxLength )
                    {
                        MaxExtentNode = ExtentNode;
                        MaxExtent = Extent;
                        MaxLength = Extent->Length.QuadPart;
                    }
                }
            } while ( ExtentNode = FindNextNode( &(VolumeCtx->ExtentsTree), LEFT_TO_RIGHT ) );

            if ( MaxExtent )
            {
#ifdef _DEBUG
                swprintf( Mess, L"        Max Extent %I64d [%I64d]",
                        MaxExtent->StartLcn.QuadPart, MaxExtent->Length.QuadPart );
                OutMessage( Mess );
#endif
                Result = DfrgMoveExtentToGap( MaxExtent, Gap, MoveExtentFlags | SHOULD_OPEN_STREAM );

                DeleteNode( &(VolumeCtx->ExtentsTree), MaxExtentNode );

                if ( NO_ERROR == Result && Gap->Length.QuadPart )
                {
                    //
                    // Continue with gap
                    //
#ifdef _DEBUG
                    OutMessage( L"   Continue with gap" );
#endif
                    ContinueWithGap = TRUE;
                    continue;
                }
            }
        }

        if ( !ContinueWithGap )
        {
            GapNode = NextGapNode;
        }

    }

}
__except(EXCEPTION_EXECUTE_HANDLER)
{
    Result = GetExceptionCode();
    DislpayErrorMessage( L"DfrgForceTogether(). Exception !!!", Result, ERR_OS );
}


    DfrgCloseFile( FileCtx );


    if ( !GapNode )
    {
#ifdef _DEBUG
         OutMessage( L"Gaps pool end" );
#endif
         Result = NO_ERROR;
    }

    if ( !Extent )
    {
#ifdef _DEBUG
         OutMessage( L"Extents pool end" );
#endif
         Result = NO_ERROR;
    }

    DeleteTree( &NewGapsTree );


    return Result;

} // end of DfrgForceTogether

