/*
    Defrag Engine

    Module name:

        HelperUtils.cpp

    Abstract:

        Defrag Engine helper defragmentation functions module.
        Contains helper defragmentation functions, etc.

    $Header: /home/CVS_DEFRAG/Defrag/Src/Engine/HelperUtils.cpp,v 1.8 2009/12/25 13:48:54 dimas Exp $
    $Log: HelperUtils.cpp,v $
    Revision 1.8  2009/12/25 13:48:54  dimas
    Debug artefacts removed

    Revision 1.7  2009/12/24 12:23:50  dimas
    Some bugs fixed

    Revision 1.6  2009/12/24 10:52:20  dimas
    Check against Exclude Files list implemented

    Revision 1.5  2009/12/23 13:01:47  dimas
    1. Full File Name processing implemented.
    2. Top 100 Fragmented functionality for all file list requests implemented.

    Revision 1.4  2009/12/21 17:00:13  dimas
    1. DEFRAG_CMD_GET_MOST_FRAGMENTED and similar requests implemented
    2. Basic Exclude files check added

    Revision 1.3  2009/11/26 15:56:54  dimas
    1. Smart modes improved.
    2. File names support improved.

    Revision 1.2  2009/11/24 15:15:10  dimas
    Bugs 5.2, 6.*, 15.14 fixed

    Revision 1.1  2009/11/24 14:52:15  dimas
    no message

*/

#include "stdafx.h"


// ===========================
//  Decision Utils
// ===========================

//
// Target:
//    TRUE  - the stream to be defragmentred
//    FALSE - the stream to be moved to free space
//
BOOLEAN
CheckIsStreamUnmoveable(
    IN  PDEFRAG_VOLUME_CONTEXT  VolumeCtx,
    IN  PDEFRAG_STREAM_CONTEXT  StreamCtx,
    IN  DWORD                   Flags,
    IN  BOOLEAN                 Target )
{
    BOOLEAN                     ExtentIsUnmoveable = FALSE;
    int                         Result;

    PDEFRAG_FILE_CONTEXT        FileCtx = NULL;


    FileCtx = StreamCtx->FileCtx;

    if ( ( FileCtx->DefragFlags & 
         (DEFRAG_FILE_UNMOVABLE | DEFRAG_FILE_PROCESSED | DEFRAG_FILE_EXCLUDED) ) )
    {
        ExtentIsUnmoveable = TRUE;
    }
    else if ( Target && (FileCtx->DefragFlags & DEFRAG_FILE_EXCLUDED) )
    {
        ExtentIsUnmoveable = TRUE;
    }
    else
    {
        Result = DfrgOpenFileStream( StreamCtx );
        if ( NO_ERROR != Result )
        {
            if ( !FileCtx->FHandle || INVALID_HANDLE_VALUE == FileCtx->FHandle )
            {
                //
                // File was not opened - mark it as locked.
                // Redraw map for this stream - Update Display & Flags.
                //
                UpdateDisplayForStream( FileCtx, StreamCtx, (SUBSTRACT_CLASTERS | INCL_FLAGS_COUNTS) );

                FileCtx->DefragFlags |= (DEFRAG_FILE_UNMOVABLE | DEFRAG_FILE_LOCKED);

                UpdateDisplayForStream( FileCtx, StreamCtx, (INCL_FLAGS_COUNTS) );

                VolumeCtx->Statistic.UnmovableFilesNum.QuadPart++;
            }

            ExtentIsUnmoveable = TRUE;
        }
    }



    if ( ExtentIsUnmoveable || !SHOULD_OPEN_STREAM )
    {    
        DfrgCloseFile( FileCtx );
    }


    return ExtentIsUnmoveable;

} // end of CheckIsStreamUnmoveable

//
// Iterate files tree and return first item matching defrag method's strategy/policy
//
PDEFRAG_FILE_CONTEXT
GetNextMoveableFile( 
    PBTR_TREE               FilesTree,
    PBTR_NODE               *Node,
    BOOLEAN                 Direction, // LEFT_TO_RIGHT 
    BOOLEAN                 Start,
    ULONG                   Flags )
{
    PDEFRAG_FILE_CONTEXT    FileCtx = NULL;
    BOOLEAN                 Accept;


    if ( Start )
    {
        *Node = FindFirstNode( FilesTree, Direction );
    }
    else
    {
        *Node = FindNextNode( FilesTree, Direction );
    }

    while( (*Node) )
    {
        FileCtx = (PDEFRAG_FILE_CONTEXT)((*Node)->Body);

        if ( !ShouldContinue( FileCtx->VolumeCtx->JobCtx ) )
        {
            FileCtx = NULL;
            break;
        }

        Accept = TRUE;

        //
        // Check file props
        //
        if ( !Flags || (FileCtx->DefragFlags & Flags) )
        {
            if ( DEFRAG_JOB_SIMPLE_DEFRAG == FileCtx->VolumeCtx->JobCtx->Type )
            {
                if ( IsFileExcluded( FileCtx->VolumeCtx->JobCtx, FileCtx ) )
                {
                    FileCtx->DefragFlags |= (DEFRAG_FILE_EXCLUDED);
                }
            }

            if ( !FileCtx->TotalClustersNum.QuadPart )
            {
                Accept = FALSE;
            }
            else if ( (FileCtx->DefragFlags & DEFRAG_FILE_UNMOVABLE) || 
                      (FileCtx->DefragFlags & DEFRAG_FILE_EXCLUDED)  ||
                      (FileCtx->DefragFlags & DEFRAG_FILE_PROCESSED) )
            {
                Accept = FALSE;
            }

            if ( Accept )
            {
                break;
            }
        }

        *Node = FindNextNode( FilesTree, Direction );
        FileCtx = NULL;
    }

    return  FileCtx;

} // end of GetNextMoveableFile


LONGLONG
GetFileClustersInZone( 
    IN  PDEFRAG_LCN_EXTENT      ZoneGap, // from lcn - to the end of disk
    IN  PDEFRAG_FILE_CONTEXT    FileCtx )
{
    PDEFRAG_STREAM_CONTEXT      StreamCtx = FileCtx->FirstStream;
    DEFRAG_LCN_EXTENT           Extent;
    LONGLONG                    StartingVcn;
    LONGLONG                    ClustersInZone = 0;
    //LONGLONG                    GapEnd_P1 = Gap->StartLcn.QuadPart + Gap->Length.QuadPart;
    PRETRIEVAL_POINTERS_BUFFER  ClustersMap;

#ifdef _DEBUG
    //WCHAR                   Mess[MAX_PATH];
    //swprintf( Mess, L"%ls %016I64X '%ls' total clusters %I64d", 
    //    (FileCtx->DefragFlags & DEFRAG_FILE_DIRECTORY) ? L"Dir " : L"File", 
    //    FileCtx->FId.FileId,
    //    FileCtx->FName ? FileCtx->FName : L"",
    //    FileCtx->TotalClustersNum.QuadPart );
    //OutMessage( Mess );
#endif


    while ( StreamCtx && StreamCtx->ClustersMap )
    {
        ClustersMap = StreamCtx->ClustersMap;
        StartingVcn = ClustersMap->StartingVcn.QuadPart;

        for ( DWORD i=0; i<ClustersMap->ExtentCount; 
            StartingVcn = ClustersMap->Extents[i].NextVcn.QuadPart, i++ )
        {
            if ( VIRTUAL_LCN != ClustersMap->Extents[i].Lcn.QuadPart ) 
            {
                Extent.StartLcn = ClustersMap->Extents[i].Lcn;
                Extent.Length.QuadPart = ClustersMap->Extents[i].NextVcn.QuadPart - StartingVcn;

                if ( Extent.StartLcn.QuadPart >= ZoneGap->StartLcn.QuadPart )
                {
                    ClustersInZone += Extent.Length.QuadPart;
                        //((Gap->StartLcn.QuadPart+Gap->Length.QuadPart) - (Gap->StartLcn.QuadPart+Gap->Length.QuadPart)) - // end
                        //(Gap->StartLcn.QuadPart - Gap->StartLcn.QuadPart ); // end
                }
            }
        }

        StreamCtx = StreamCtx->NextStream;
    }


    return ClustersInZone;

} // end of GetFileClustersInZone

//
// Search files tree for file matching criteria
//
PDEFRAG_FILE_CONTEXT
GetFileToMove( 
    IN  PBTR_TREE               FilesTree,
    IN  PDEFRAG_FILE_CONTEXT    FileTemplate,
    IN  PDEFRAG_LCN_EXTENT      SourceGap, 
    OUT PBTR_NODE               *Node )
{
    PDEFRAG_FILE_CONTEXT        FileCtx = NULL;
    PDEFRAG_FILE_CONTEXT        CandidatCtx = NULL;
    PBTR_NODE                   CandidatNode = NULL;
    LONGLONG                    ClustersInGap, MaxClustersInGap = 0;


#ifdef _DEBUG
    WCHAR       Mess[MAX_PATH];
    swprintf( Mess, L"      Look for file with size <= %I64d",
                FileTemplate->TotalClustersNum.QuadPart );
    OutMessage( Mess );
#endif


    *Node = FindNode( FilesTree, FileTemplate, SortModeSize, _FIND_LE_ );

    while( *Node )
    {
        FileCtx = (PDEFRAG_FILE_CONTEXT)((*Node)->Body);

#ifdef _DEBUG
        swprintf( Mess, L"         find file with size %I64d",
                    FileCtx->TotalClustersNum.QuadPart );
        OutMessage( Mess );
#endif
        if ( MaxClustersInGap > FileCtx->TotalClustersNum.QuadPart )
        {
            break;
        }

        //
        // Check file props
        //
        //if ( !FileCtx->TotalClustersNum.QuadPart )
        //
        if ( !(FileCtx->DefragFlags & 
                    (DEFRAG_FILE_UNMOVABLE | DEFRAG_FILE_EXCLUDED | DEFRAG_FILE_PROCESSED)) )
        {
            ClustersInGap = GetFileClustersInZone( SourceGap, FileCtx );
            if ( ClustersInGap > MaxClustersInGap )
            {
                MaxClustersInGap = ClustersInGap;
                CandidatNode = *Node;
                CandidatCtx = FileCtx;
            }
        }

#ifdef _DEBUG
        OutMessage( L"         look for next" );
#endif

        *Node = FindNextNode( FilesTree, RIGHT_TO_LEFT );
        FileCtx = NULL;
    }

    *Node   = CandidatNode ;
    FileCtx = CandidatCtx;

#ifdef _DEBUG
    if ( FileCtx )
    {
        swprintf( Mess, L"         find file with size %I64d - MaxClustersInGap %I64d",
                  FileCtx->TotalClustersNum.QuadPart, MaxClustersInGap );
        OutMessage( Mess );
    }
    else
    {
        swprintf( Mess, L"         could not get file with size <= %I64d. MaxClustersInGap %I64d",
                  FileTemplate->TotalClustersNum.QuadPart, MaxClustersInGap );
        OutMessage( Mess );
    }
#endif


    return  FileCtx;

} // end of GetFileToMove


LONGLONG
GetExtentsIntersection(
    IN  PDEFRAG_LCN_EXTENT          Extent1,
    IN  PDEFRAG_LCN_EXTENT          Extent2,
    OUT PDEFRAG_LCN_EXTENT          OutIntersection )
{
    LONGLONG                        End_P1;
    DEFRAG_LCN_EXTENT               Intersection = { 0 };


    if ( !Extent1 || !Extent2 )
    {
        return -1;
    }

    //
    // A ^ B = AB:
    // AB.left  = max( A.left, B.left )
    // AB.right = min( A.right, B.right )
    // if ( AB.left <= AB.right ) true;
    //
    Intersection.StartLcn.QuadPart = 
        (Extent1->StartLcn.QuadPart >= Extent2->StartLcn.QuadPart) ?
            Extent1->StartLcn.QuadPart : Extent2->StartLcn.QuadPart;

    End_P1 = Extent1->StartLcn.QuadPart + Extent1->Length.QuadPart;
    if ( End_P1 >= Extent2->StartLcn.QuadPart + Extent2->Length.QuadPart )
    {
        End_P1 = Extent2->StartLcn.QuadPart + Extent2->Length.QuadPart;
    }

    Intersection.Length.QuadPart = End_P1 - Intersection.StartLcn.QuadPart;

    if ( OutIntersection )
    {
        memcpy( OutIntersection, &Intersection, sizeof(DEFRAG_LCN_EXTENT) );
    }


    return Intersection.Length.QuadPart;

} // GetExtentsIntersection


//
// Search extents tree for extent matching criteria:
// - extent should be in SourceGap
// - extent should have max size <= TargetGap size
// - if Direction 
//
PDEFRAG_FILES_EXTENT
GetExtentForGap( 
    IN  PBTR_TREE               ExtentsTree,
    IN  PDEFRAG_LCN_EXTENT      TargetGap, 
    IN  PDEFRAG_LCN_EXTENT      SourceGap, 
    IN  BTR_TREE_SORT_MODE      SortMode,
    IN  BOOLEAN                 LookupDirection,
    IN  int                     SizeEquality,
    OUT PBTR_NODE               *Node )
{
    PDEFRAG_FILES_EXTENT        CurrentExtent = NULL;
    PDEFRAG_FILES_EXTENT        CandidatCtx = NULL;
    PBTR_NODE                   CandidatNode = NULL;

    LONGLONG                    MaxClustersInGap = 0;
    int                         LcnEquality;
    DEFRAG_LCN_EXTENT           Intersection = { 0 };
    LONGLONG                    IntersectionLength;



#ifdef _DEBUG
    WCHAR       Mess[MAX_PATH];
    swprintf( Mess, L"      Look in zone %I64d [%I64d] from %ls for extent with %ls %ls %I64d",
        SourceGap?SourceGap->StartLcn.QuadPart:0,
        SourceGap?SourceGap->Length.QuadPart:0,
        (LookupDirection == LEFT_TO_RIGHT)?L"Begin":L"End",
        (SortMode == SortModeSize)?L"Size":L"Lcn",
        (SizeEquality == _FIND_LE_)?L"<=":((SizeEquality == _FIND_GE_)?L">=":L"=="),
        (SortMode == SortModeSize)?TargetGap->Length.QuadPart:TargetGap->StartLcn.QuadPart);
    OutMessage( Mess );
#endif


    if ( SourceGap )
    {
        DEFRAG_FILES_EXTENT         ExtentTemplate = { 0 };

        LcnEquality = _FIND_LE_;

        if ( LEFT_TO_RIGHT == LookupDirection )
        {
            ExtentTemplate.StartLcn = SourceGap->StartLcn;
        }
        else
        {
            ExtentTemplate.StartLcn.QuadPart = SourceGap->StartLcn.QuadPart + SourceGap->Length.QuadPart;
        }

        *Node = FindNode( ExtentsTree, &ExtentTemplate, SortModeDefault, LcnEquality ); 
    }
    else
    {
        *Node = FindFirstNode( ExtentsTree, LookupDirection );
    }

    while( (*Node) )
    {
        CurrentExtent = (PDEFRAG_FILES_EXTENT)((*Node)->Body);

        //if ( LEFT_TO_RIGHT == Direction && CurrentExtent->StartLcn.QuadPart >= EndLcn ||
        //     RIGHT_TO_LEFT == Direction && CurrentExtent->StartLcn.QuadPart < EndLcn )
        //{
        //    break;
        //}

        //FileCtx = CurrentExtent->StreamCtx->FileCtx;

#ifdef _DEBUGW
        swprintf( Mess, L"         find extent %I64d [%I64d]",
                  CurrentExtent->StartLcn.QuadPart, CurrentExtent->Length.QuadPart );
        OutMessage( Mess );
#endif

        IntersectionLength = GetExtentsIntersection( 
                                    SourceGap, 
                                    (PDEFRAG_LCN_EXTENT)CurrentExtent, 
                                    &Intersection );

        if ( IntersectionLength < 0 )
        {
            break;
        }

        //
        // Check extent / file props
        //

        //if ( SizeEquality == _FIND_LE_ && 
        //    IntersectionLength > TargetGap->Length.QuadPart )
        //{
        //    *Node = FindNextNode( ExtentsTree, Direction );
        //    continue;
        //}

        if ( SizeEquality == _FIND_MAX_ ||
            IntersectionLength <= TargetGap->Length.QuadPart )
        {
            if ( IntersectionLength >= MaxClustersInGap ) // 
            {
                MaxClustersInGap = IntersectionLength;
                CandidatNode = *Node;
                CandidatCtx = CurrentExtent;
            }
        }

        *Node = FindNextNode( ExtentsTree, LookupDirection );

    }


    *Node           = CandidatNode;
    CurrentExtent   = CandidatCtx;

#ifdef _DEBUG
    if ( CurrentExtent )
    {
        swprintf( Mess, L"         Find extent %I64d [%I64d]. MaxClustersInGap %I64d", 
                    CurrentExtent->StartLcn.QuadPart, CurrentExtent->Length.QuadPart,
                    MaxClustersInGap );
        OutMessage( Mess );
    }
    else
    {
        swprintf( Mess, L"         Could not get extent for gap %I64d [%I64d]. MaxClustersInGap %I64d",
                    TargetGap->StartLcn.QuadPart, TargetGap->Length.QuadPart,
                    MaxClustersInGap );
        OutMessage( Mess );
    }
#endif


    return  CurrentExtent;

} // end of GetExtentForGap

//
// Search gaps tree (sorted by ) for gap matching criteria:
// - gap should be in SourceGap
// - gap should have size >= TargetGap size or max possible size
// - search in Direction 
//
PDEFRAG_LCN_EXTENT
GetBestGapForExtent( 
    IN  PBTR_TREE               GapsTree,
    IN  PDEFRAG_FILES_EXTENT    ExtentTemplate, 
    IN  PDEFRAG_LCN_EXTENT      SourceZone, 
    IN  BTR_TREE_SORT_MODE      SortMode,
    IN  BOOLEAN                 LookupDirection,
    IN  int                     SizeEquality,
    OUT PBTR_NODE               *Node )
{
    PDEFRAG_LCN_EXTENT          CurrentGap = NULL;
    PDEFRAG_LCN_EXTENT          CandidatCtx = NULL;
    PBTR_NODE                   CandidatNode = NULL;

    LONGLONG                    MaxLength = 0;
    int                         LcnEquality;
    DEFRAG_LCN_EXTENT           Intersection = { 0 };
    LONGLONG                    IntersectionLength;



#ifdef _DEBUG
    WCHAR       Mess[MAX_PATH];
    swprintf( Mess, L"      Search in zone %I64d [%I64d] from %ls gap for extent %I64d [%I64d]",
        SourceZone?SourceZone->StartLcn.QuadPart:0,
        SourceZone?SourceZone->Length.QuadPart:0,
        (LookupDirection == LEFT_TO_RIGHT)?L"Start":L"End",
        ExtentTemplate->StartLcn.QuadPart, ExtentTemplate->Length.QuadPart );
        //(SortMode == SortModeSize)?L"Size":L"Lcn",
        //(SizeEquality == _FIND_LE_)?L"<=":((SizeEquality == _FIND_GE_)?L">=":L"=="),
        //(SortMode == SortModeSize)?ExtentTemplate->Length.QuadPart:ExtentTemplate->StartLcn.QuadPart);
    OutMessage( Mess );
#endif


    if ( SourceZone )
    {
        DEFRAG_LCN_EXTENT         GapTemplate = { 0 };

        if ( LEFT_TO_RIGHT == LookupDirection )
        {
            GapTemplate.StartLcn = SourceZone->StartLcn;
            LcnEquality = _FIND_GE_;
        }
        else
        {
            GapTemplate.StartLcn.QuadPart = SourceZone->StartLcn.QuadPart + SourceZone->Length.QuadPart;
            LcnEquality = _FIND_LE_;
        }

        *Node = FindNode( GapsTree, &GapTemplate, SortModeDefault, LcnEquality ); 
    }
    else
    {
        *Node = FindFirstNode( GapsTree, LookupDirection );
    }


    while( (*Node) )
    {
        CurrentGap = (PDEFRAG_LCN_EXTENT)((*Node)->Body);

        if ( SourceZone &&
            ( LEFT_TO_RIGHT == LookupDirection && 
            CurrentGap->StartLcn.QuadPart >= SourceZone->StartLcn.QuadPart + SourceZone->Length.QuadPart ||
            RIGHT_TO_LEFT == LookupDirection && CurrentGap->StartLcn.QuadPart < SourceZone->StartLcn.QuadPart ) )
        {
            break;
        }

#ifdef _DEBUGW
        swprintf( Mess, L"         find gap %I64d [%I64d]",
                  CurrentGap->StartLcn.QuadPart, CurrentGap->Length.QuadPart );
        OutMessage( Mess );
#endif
/*
        IntersectionLength = GetExtentsIntersection( 
                                    SourceZone, 
                                    (PDEFRAG_LCN_EXTENT)CurrentExtent, 
                                    &Intersection );

        if ( IntersectionLength < 0 )
        {
            break;
        }
*/
        IntersectionLength = CurrentGap->Length.QuadPart;

        if ( IntersectionLength > MaxLength )
        {
            MaxLength = IntersectionLength;
            CandidatNode = *Node;
            CandidatCtx = CurrentGap;

            if ( IntersectionLength >= ExtentTemplate->Length.QuadPart )
            {
                break;
            }
        }

        *Node = FindNextNode( GapsTree, LookupDirection );

    }


    *Node           = CandidatNode;

#ifdef _DEBUG
    if ( CandidatCtx )
    {
        swprintf( Mess, L"         Find gap  %I64d [%I64d]. MaxLength Gap %I64d", 
                    CandidatCtx->StartLcn.QuadPart, CandidatCtx->Length.QuadPart,
                    MaxLength );
        OutMessage( Mess );
    }
    else
    {
        swprintf( Mess, L"         Could not get gap for extent %I64d [%I64d]. MaxLength Gap %I64d",
                    ExtentTemplate->StartLcn.QuadPart, ExtentTemplate->Length.QuadPart,
                    MaxLength );
        OutMessage( Mess );
    }
#endif


    return  CandidatCtx;

} // end of GetBestGapForExtent



// ===========================
//  Gap & Zone Utils
// ===========================

int
DfrgFindGap( 
    ULONGLONG                   Length,
    PBTR_TREE                   GapsTree,
    PBTR_NODE                   *OutGapNode, 
    PDEFRAG_ZONE                ExcludeZone,
    PDEFRAG_ZONE                IncludeZone )
{
    int                         Result = DFRG_ERROR_NOT_ENOUGH_SPACE;

    DEFRAG_LCN_EXTENT           GapTemplate; 
    PDEFRAG_LCN_EXTENT          Gap = NULL; 
    PBTR_NODE                   GapNode = NULL;


#ifdef _DEBUG
    //WCHAR   Mess[MAX_PATH];
#endif


    __try
    {
        *OutGapNode = NULL;

        GapTemplate.Length.QuadPart = Length;

        GapNode = FindNode( GapsTree, &GapTemplate, SortModeDefault, _FIND_GE_ );
        if ( !GapNode )
        {
            __leave;
        }

        do
        {
            Gap = (PDEFRAG_LCN_EXTENT)GapNode->Body;

            DEFRAG_LCN_EXTENT           GapToUse; 

            if ( (!ExcludeZone || !IsGapInZone( Gap, &GapToUse, ExcludeZone ))
                && (!IncludeZone || IsGapInZone( Gap, &GapToUse, IncludeZone )) )
            {
                *OutGapNode = GapNode;
                Result = NO_ERROR;
                __leave;
            }

#ifdef _DEBUGW
            swprintf( Mess, L"   DfrgFindGap(). Gap %I64d [%I64d] is not in desired zones ",
                Gap->StartLcn.QuadPart, Gap->Length.QuadPart );
            OutMessage( Mess );
#endif
            if ( IncludeZone && 
                1 == IncludeZone->ExtentCount &&
                Gap->Length.QuadPart > IncludeZone->Extents[0].Length.QuadPart )
            {
                //
                // Stop search.
                //
                __leave;
            }

            GapNode = FindNextNode( GapsTree, LEFT_TO_RIGHT );

        } while( GapNode );

    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        Result = GetExceptionCode();
        DislpayErrorMessage( L"DfrgFindGap(). Exception !!!", Result, ERR_OS );
    }
    

    return Result;

} // DfrgFindGap


BYTE        Zone1_buf[100];
BYTE        Zone2_buf[100];

void
DfrgBuildFreeSpaceZones( 
    IN  PDEFRAG_VOLUME_CONTEXT  VolumeCtx,
    OUT PDEFRAG_ZONE            *OutTargetZone, 
    OUT PDEFRAG_ZONE            *OutSourceZone )
{
    PDEFRAG_ZONE                TargetZone = NULL, SourceZone = NULL;
    LARGE_INTEGER               ZoneEdge, MftIntersection, MftZoneLength;


    TargetZone = (PDEFRAG_ZONE)Zone1_buf;
    memset( Zone1_buf, 0, sizeof(Zone1_buf) );
    SourceZone = (PDEFRAG_ZONE)Zone2_buf;
    memset( Zone2_buf, 0, sizeof(Zone2_buf) );

    //
    // Devide volume on two zones - according to free space.
    // Free space should be at the end of volume - this is source zone.
    // Target zone is at the beginning of volume.
    //
    MftZoneLength.QuadPart = VolumeCtx->NtfsData.MftZoneEnd.QuadPart - VolumeCtx->NtfsData.MftZoneStart.QuadPart; // + 1;
    MftIntersection = MftZoneLength;

    ZoneEdge.QuadPart = VolumeCtx->NtfsData.TotalClusters.QuadPart - 
        VolumeCtx->NtfsData.FreeClusters.QuadPart + VolumeCtx->NtfsData.TotalReserved.QuadPart;
    ZoneEdge.QuadPart = (ZoneEdge.QuadPart + 15) & ~(ULONGLONG)15; // adjust to 16 clusters boundary

    TargetZone->ExtentCount = 1;
    TargetZone->Extents[0].StartLcn.QuadPart = 0;
    TargetZone->Extents[0].Length = ZoneEdge;

    SourceZone->ExtentCount = 1;
    SourceZone->Extents[0].StartLcn = ZoneEdge;
    SourceZone->Extents[0].Length.QuadPart = 
        VolumeCtx->NtfsData.TotalClusters.QuadPart - ZoneEdge.QuadPart;

    //
    // Adjust zone edge as unmovable files could be in source zone.
    // In such a case move edge up.
    //

    PBTR_NODE                   FileNode;
    PDEFRAG_FILE_CONTEXT        FileCtx;
    PDEFRAG_STREAM_CONTEXT      StreamCtx;
    LARGE_INTEGER               StartingVcn, ExtentsLength = { 0 };
    DEFRAG_LCN_EXTENT           Extent, ExtentToMove;

    //
    // Iterate through file list 
    //
    FileNode = FindFirstNode( &(VolumeCtx->FilesTree), LEFT_TO_RIGHT );

    while ( FileNode )
    {
        FileCtx = (PDEFRAG_FILE_CONTEXT)FileNode->Body;

        if ( (FileCtx->DefragFlags & DEFRAG_FILE_UNMOVABLE) )
        {
            //
            // Iterate through all file's streams
            //
            for ( StreamCtx = FileCtx->FirstStream; StreamCtx; StreamCtx = StreamCtx->NextStream )
            {
                //
                // Iterate through all stream's extents
                //
                StartingVcn = StreamCtx->ClustersMap->StartingVcn; // 0

                for( DWORD i=0; i<StreamCtx->ClustersMap->ExtentCount; 
                    StartingVcn = StreamCtx->ClustersMap->Extents[i].NextVcn, i++ )
                {
                    if ( VIRTUAL_LCN == StreamCtx->ClustersMap->Extents[i].Lcn.QuadPart ) 
                    {
                        //
                        // Skip virtual (sparce) extent
                        //
                        continue;
                    }

                    Extent.StartLcn = StreamCtx->ClustersMap->Extents[i].Lcn;
                    Extent.Length.QuadPart = StreamCtx->ClustersMap->Extents[i].NextVcn.QuadPart - StartingVcn.QuadPart;

                    if ( IsGapInZone( &Extent, &ExtentToMove, TargetZone ) )
                    {
                        ExtentsLength.QuadPart += ExtentToMove.Length.QuadPart;
                    }
                }
            }

        }

        FileNode = FindNextNode( &(VolumeCtx->FilesTree), LEFT_TO_RIGHT );
    }

    if ( ExtentsLength.QuadPart )
    {
        ZoneEdge.QuadPart -= ExtentsLength.QuadPart;
        TargetZone->Extents[0].Length = ZoneEdge;
        SourceZone->Extents[0].StartLcn = ZoneEdge;
        SourceZone->Extents[0].Length.QuadPart = 
            VolumeCtx->NtfsData.TotalClusters.QuadPart - ZoneEdge.QuadPart;
    }


    //
    // Check if Mft Zone is in Target zone.
    // It could not be used - so should be substructed from Target zone and
    // zone edge should be moved down (to the end of volume).
    //
    if ( NtfsDisk == VolumeCtx->DiskType &&
        VolumeCtx->NtfsData.MftZoneStart.QuadPart < ZoneEdge.QuadPart )
    {
        //
        // MftZone is in Target Zone. Adjust Target Zone
        //
        if ( VolumeCtx->NtfsData.MftZoneEnd.QuadPart > ZoneEdge.QuadPart )
        {
            //
            // MftZone is partially in Target Zone. 
            //
            MftIntersection.QuadPart = ZoneEdge.QuadPart - VolumeCtx->NtfsData.MftZoneStart.QuadPart;
        }
        TargetZone->Extents[0].Length.QuadPart = VolumeCtx->NtfsData.MftZoneStart.QuadPart;
        TargetZone->Extents[1].StartLcn = VolumeCtx->NtfsData.MftZoneEnd; // + 1
        TargetZone->Extents[1].Length = MftIntersection;
        ZoneEdge.QuadPart = TargetZone->Extents[1].StartLcn.QuadPart + TargetZone->Extents[1].Length.QuadPart;
        TargetZone->ExtentCount = 2;
    }

    //
    // Set Source zone.
    //
    SourceZone->ExtentCount = 1;
    SourceZone->Extents[0].StartLcn = ZoneEdge;
    SourceZone->Extents[0].Length.QuadPart = 
        VolumeCtx->NtfsData.TotalClusters.QuadPart - ZoneEdge.QuadPart;


    *OutTargetZone = TargetZone;
    *OutSourceZone = SourceZone;

} // end of DfrgBuildFreeSpaceZones


void
DfrgBuildSimpleZones( 
    IN  PDEFRAG_VOLUME_CONTEXT  VolumeCtx,
    OUT PDEFRAG_ZONE            *OutMftZone )
{
    PDEFRAG_ZONE                MftZone = NULL;

    //
    // Set excluded target zone to Mft zone
    //
    MftZone = (PDEFRAG_ZONE)Zone1_buf;
    memset( Zone1_buf, 0, sizeof(Zone1_buf) );

    if ( NtfsDisk == VolumeCtx->DiskType )
    {
        MftZone->ExtentCount = 1;
        MftZone->Extents[0].StartLcn = VolumeCtx->NtfsData.MftZoneStart;
        MftZone->Extents[0].Length.QuadPart = VolumeCtx->NtfsData.MftZoneEnd.QuadPart - VolumeCtx->NtfsData.MftZoneStart.QuadPart;
    }

    *OutMftZone = MftZone;

} // end of DfrgBuildSimpleZones


void
ExpandZoneWithExtent( 
    IN      PDEFRAG_VOLUME_CONTEXT  VolumeCtx,
    IN  OUT PDEFRAG_ZONE            SourceZone,
    IN      PBTR_NODE               ExtentNode,
    IN      BOOLEAN                 ExtentsMoveDirection,
    IN      DWORD                   Flags )
{
    IN      PDEFRAG_FILES_EXTENT    Extent = (PDEFRAG_FILES_EXTENT)(ExtentNode->Body);
    

    //
    // Extent UNMOVABLE. Expand Source zone.
    //
    if ( RIGHT_TO_LEFT == ExtentsMoveDirection )
    {
        if ( Extent->StartLcn.QuadPart < SourceZone->Extents[0].StartLcn.QuadPart )
        {
            SourceZone->Extents[0].Length.QuadPart += 
                SourceZone->Extents[0].StartLcn.QuadPart - Extent->StartLcn.QuadPart;
            SourceZone->Extents[0].StartLcn.QuadPart = Extent->StartLcn.QuadPart;
        }
        else
        {
            SourceZone->Extents[0].Length.QuadPart += Extent->Length.QuadPart;
            SourceZone->Extents[0].StartLcn.QuadPart -= Extent->Length.QuadPart;
        }
    }
    else
    {
    }

#ifdef _DEBUG
    WCHAR       Mess[MAX_PATH];
    PDEFRAG_FILE_CONTEXT        FileCtx = Extent->StreamCtx->FileCtx;
    swprintf( Mess, L"      Extent %I64d [%I64d] (%ls) UNMOVABLE. Expand Source zone to %I64d [%I64d] .. %I64d",
            Extent->StartLcn.QuadPart, Extent->Length.QuadPart,
            FileCtx->FName ? FileCtx->FName : L"NULL",
            SourceZone->Extents[0].StartLcn.QuadPart, SourceZone->Extents[0].Length.QuadPart,
            SourceZone->Extents[0].StartLcn.QuadPart + SourceZone->Extents[0].Length.QuadPart );
    OutMessage( Mess );
#endif

/*
    if ( (Flags & SHOULD_ADJUST_USED_EXTENT) )
    {
        //
        // Delete extent
        //
#ifdef _DEBUG
        OutMessage( L"   Delete extent" );
#endif
        DeleteNode(  &(VolumeCtx->ExtentsTree), ExtentNode );
    }
*/

} // end of ExpandZoneWithExtent


void
DfrgBuildFreeZones( 
    IN  PDEFRAG_VOLUME_CONTEXT  VolumeCtx,
    OUT PDEFRAG_ZONE            *OutMftZone,
    OUT PDEFRAG_ZONE            *OutSourceZone )
{
    PDEFRAG_ZONE                MftZone = NULL, SourceZone = NULL;

    //
    // Set excluded target zone to Mft zone 
    // TODO: adjust (remove from list) gaps in Mft zone
    //
    MftZone = (PDEFRAG_ZONE)Zone1_buf;

    if ( NtfsDisk == VolumeCtx->DiskType )
    {
        memset( Zone1_buf, 0, sizeof(Zone1_buf) );
        MftZone->ExtentCount = 1;
        MftZone->Extents[0].StartLcn = VolumeCtx->NtfsData.MftZoneStart;
        MftZone->Extents[0].Length.QuadPart = VolumeCtx->NtfsData.MftZoneEnd.QuadPart - VolumeCtx->NtfsData.MftZoneStart.QuadPart;
    }

    *OutMftZone = MftZone;


    //
    // Set SourceZone to the end of disk
    //
    SourceZone = (PDEFRAG_ZONE)Zone2_buf;
    memset( Zone2_buf, 0, sizeof(Zone2_buf) );

    SourceZone->ExtentCount = 1;
    SourceZone->Extents[0].StartLcn.QuadPart = 
        VolumeCtx->NtfsData.TotalClusters.QuadPart - VolumeCtx->NtfsData.FreeClusters.QuadPart;
    SourceZone->Extents[0].Length.QuadPart = VolumeCtx->NtfsData.FreeClusters.QuadPart;

    *OutSourceZone = SourceZone;

    //
    // Adjust SourceZone 
    //
    BOOLEAN                 ExtentsMoveDirection = RIGHT_TO_LEFT;


    //
    // Reduce SourceZone by length of gaps that are in Mft zone but are not in Source Zone
    //
    DEFRAG_LCN_EXTENT       ZoneIntersection;
    LONGLONG                ZoneIntersectionLength = 0;
    LONGLONG                MftZoneWasteSpace = MftZone->Extents[0].Length.QuadPart;

    ZoneIntersectionLength = GetExtentsIntersection( 
                                &(MftZone->Extents[0]), 
                                &(SourceZone->Extents[0]), 
                                &ZoneIntersection );

    if ( ZoneIntersectionLength > 0 )
    {
#ifdef _DEBUG
        WCHAR       Mess[MAX_PATH];
        swprintf( Mess, L"   Intersection of Source zone with Mft zone: %I64d [%I64d]",
                ZoneIntersection.StartLcn.QuadPart, ZoneIntersection.Length.QuadPart );
        OutMessage( Mess );
#endif
        //if ( RIGHT_TO_LEFT == ExtentsMoveDirection ) {
        MftZoneWasteSpace -= ZoneIntersectionLength;
    }

    SourceZone->Extents[0].StartLcn.QuadPart += MftZoneWasteSpace;
    SourceZone->Extents[0].Length.QuadPart   -= MftZoneWasteSpace;

    //
    // Expand SourceZone by length of unmovable extents in SourceZone 
    //
    PDEFRAG_FILE_CONTEXT        FileCtx = NULL;
    PDEFRAG_FILES_EXTENT        CurrentExtent;
    PBTR_NODE                   ExtentNode, NextExtentNode ;
    DEFRAG_LCN_EXTENT           Intersection;
    LONGLONG                    IntersectionLength = 0;
    BOOLEAN                     ExtentIsUnmoveable;


    ExtentNode = FindFirstNode( &(VolumeCtx->ExtentsTree), ExtentsMoveDirection );

    while( ExtentNode )
    {
        NextExtentNode = FindNextNode( &(VolumeCtx->ExtentsTree), ExtentsMoveDirection );

        CurrentExtent = (PDEFRAG_FILES_EXTENT)(ExtentNode->Body);

        IntersectionLength = GetExtentsIntersection( 
                                    &(SourceZone->Extents[0]), 
                                    (PDEFRAG_LCN_EXTENT)CurrentExtent, 
                                    &Intersection );

        if ( IntersectionLength < 0 )
        {
#ifdef _DEBUGW
            WCHAR       Mess[MAX_PATH];
            swprintf( Mess, L"   Extent %I64d [%I64d] out of zone %I64d [%I64d]",
                    CurrentExtent->StartLcn.QuadPart, CurrentExtent->Length.QuadPart,
                    SourceZone->Extents[0].StartLcn.QuadPart, SourceZone->Extents[0].Length.QuadPart );
            OutMessage( Mess );
#endif
            break;
        }

        ExtentIsUnmoveable = CheckIsStreamUnmoveable( VolumeCtx, CurrentExtent->StreamCtx, 0 );

        if ( ExtentIsUnmoveable )
        {
            ExpandZoneWithExtent( 
                VolumeCtx,
                SourceZone, 
                ExtentNode, 
                ExtentsMoveDirection, 
                0 ); // SHOULD_ADJUST_USED_EXTENT );

            //
            // Delete extent
            //
#ifdef _DEBUG
            OutMessage( L"   Delete extent" );
#endif
            DeleteNode(  &(VolumeCtx->ExtentsTree), ExtentNode );

            VolumeCtx->ExtentsTree.Current = NextExtentNode;
        }

        ExtentNode = NextExtentNode;

    }

} // end of DfrgBuildFreeZones


BOOLEAN
IsGapInZone( 
    IN  PDEFRAG_LCN_EXTENT  InGap, 
    OUT PDEFRAG_LCN_EXTENT  OutGap, 
    IN  PDEFRAG_ZONE        Zone )
{
    BOOLEAN             BRes = FALSE;
    DWORD               i;


    if ( !Zone )
    {
        return BRes;
    }

    for( i=0; i<Zone->ExtentCount; i++ )
    {
/*
        if ( InGap->StartLcn.QuadPart >= Zone->Extents[i].StartLcn.QuadPart &&
             InGap->StartLcn.QuadPart < Zone->Extents[i].StartLcn.QuadPart + Zone->Extents[i].Length.QuadPart ||
             Zone->Extents[i].StartLcn.QuadPart >= InGap->StartLcn.QuadPart &&
             Zone->Extents[i].StartLcn.QuadPart < InGap->StartLcn.QuadPart + InGap->Length.QuadPart )
        {
            //
            // InGap (extent) is partialy in current zone extent
            // TODO: Return Intersection.
            //
            if ( InGap->StartLcn.QuadPart + InGap->Length.QuadPart <= Zone->Extents[i].StartLcn.QuadPart + Zone->Extents[i].Length.QuadPart )
            {
                //
                // InGap (extent) is fully in current zone extent
                //
            }
            BRes = TRUE;
            break;
        }
*/

        //
        // TODO: Return Intersection.
        //
        if ( InGap->StartLcn.QuadPart < Zone->Extents[i].StartLcn.QuadPart )
        {
            //
            // Intersection could start only on Zone start
            //
            if ( InGap->StartLcn.QuadPart + InGap->Length.QuadPart >= 
                Zone->Extents[i].StartLcn.QuadPart + Zone->Extents[i].Length.QuadPart )
            {
                //
                // Zone extent is in gap :)
                //
                OutGap->StartLcn = Zone->Extents[i].StartLcn;
                OutGap->Length   = Zone->Extents[i].Length;
                BRes = TRUE;
                break;
            }
            else if ( InGap->StartLcn.QuadPart + InGap->Length.QuadPart > Zone->Extents[i].StartLcn.QuadPart )
            {
                OutGap->StartLcn = Zone->Extents[i].StartLcn;
                OutGap->Length.QuadPart   = InGap->StartLcn.QuadPart + InGap->Length.QuadPart - Zone->Extents[i].StartLcn.QuadPart;
                BRes = TRUE;
                break;
            }
        }
        else 
        {
            //
            // Intersection could start only on Extent start
            //
            if ( InGap->StartLcn.QuadPart + InGap->Length.QuadPart <= 
                Zone->Extents[i].StartLcn.QuadPart + Zone->Extents[i].Length.QuadPart )
            {
                //
                // InGap (extent) is fully in current zone extent
                //
                OutGap->StartLcn = InGap->StartLcn;
                OutGap->Length   = InGap->Length;
                BRes = TRUE;
                break;
            }
            else if ( Zone->Extents[i].StartLcn.QuadPart + Zone->Extents[i].Length.QuadPart > InGap->StartLcn.QuadPart )
            {
                OutGap->StartLcn = InGap->StartLcn;
                OutGap->Length.QuadPart   = Zone->Extents[i].StartLcn.QuadPart + Zone->Extents[i].Length.QuadPart - InGap->Length.QuadPart;
                BRes = TRUE;
                break;
            }
        }

    }

    return BRes;

} // end of IsGapInZone


// ===========================
//  Gaps tree Utils
// ===========================


#ifdef _DEBUG

void
PrintGapsTree( 
    PBTR_TREE               GapsTree,
    BOOLEAN                 PrintBody )
{
    PBTR_NODE               WrkNode;
    PDEFRAG_LCN_EXTENT      WrkGap;
    LONGLONG                Cnt = 0;
    WCHAR                   Mess[MAX_PATH];

    BOOLEAN                 Direction = LEFT_TO_RIGHT; // RIGHT_TO_LEFT 


    WrkNode = FindFirstNode( GapsTree, Direction );

    while ( WrkNode )
    {
        Cnt++;

        WrkGap = (PDEFRAG_LCN_EXTENT)(WrkNode->Body);

        if ( WrkGap->Length.QuadPart )
        {
            if ( PrintBody )
            {
                swprintf( Mess, L"      Gap %I64d. %I64d [%I64d] .. %I64d",
                        Cnt-1, WrkGap->StartLcn.QuadPart, WrkGap->Length.QuadPart,
                        WrkGap->StartLcn.QuadPart + WrkGap->Length.QuadPart );
                OutMessage( Mess );
            }
        }
        else
        {
            swprintf( Mess, L"      !##! Empty Gap !##! %I64d [%I64d] ",
                WrkGap->StartLcn.QuadPart, WrkGap->Length.QuadPart );
            OutMessage( Mess );
        }

        WrkNode = FindNextNode( GapsTree, Direction );
    }

    swprintf( Mess, L"   PrintGapsTree(). NodeCnt %I64d == %I64d ",
            GapsTree->NodeCnt.QuadPart, Cnt );
    OutMessage( Mess );

}

#endif


void
PrintGap(
    IN  PVOID       InGapBody )
{
    WCHAR                   Mess[MAX_PATH];
    PDEFRAG_LCN_EXTENT      WrkGap = (PDEFRAG_LCN_EXTENT)InGapBody;

    swprintf( Mess, L"      Gap %I64d [%I64d] .. %I64d",
        WrkGap->StartLcn.QuadPart, WrkGap->Length.QuadPart,
        WrkGap->StartLcn.QuadPart + WrkGap->Length.QuadPart );
    OutMessage( Mess );

}


LONGLONG
CompareGaps( 
    IN  PVOID       InGap1,
    IN  PVOID       InGap2,
    IN  ULONG       SortMode )
{
    LONGLONG        Result = 0;

    PDEFRAG_LCN_EXTENT  Gap1 = (PDEFRAG_LCN_EXTENT)InGap1;
    PDEFRAG_LCN_EXTENT  Gap2 = (PDEFRAG_LCN_EXTENT)InGap2;


#ifdef _DEBUG
    if ( !Gap1 || !Gap2 )
    {
        return 0;
            //ASSERT( FALSE );
    }
#endif

    switch ( SortMode )
    {
    case SortModeSize:

        Result = (LONGLONG)(Gap1->Length.QuadPart - Gap2->Length.QuadPart);

        break;

    case SortModeLcn:

        Result = (LONGLONG)(Gap1->StartLcn.QuadPart - Gap2->StartLcn.QuadPart);

        break;

    default:

        Result = memcmp( Gap1, Gap2, sizeof(DEFRAG_LCN_EXTENT) );

        break;
    }


    return Result;

} // end of CompareGaps


int
DfrgAddGapToTree( 
    IN  PDEFRAG_VOLUME_CONTEXT  VolumeCtx,
    LONGLONG                    FirstLcn, 
    LONGLONG                    GapLength )
{
    int                         Result = NO_ERROR;

    PDEFRAG_LCN_EXTENT          Gap = NULL;


    //
    // We interesting only in gaps that are large enough for shortest file to be processed
    //
    //if ( GapLength >= VolumeCtx->JobCtx->MinFileSize.QuadPart )
    //{
        Gap = (PDEFRAG_LCN_EXTENT) malloc( sizeof(DEFRAG_LCN_EXTENT) );
        if ( !Gap )
        {
            Result = ERROR_NOT_ENOUGH_MEMORY;
        }
        else
        {
            Gap->StartLcn.QuadPart   = FirstLcn;
            Gap->Length.QuadPart     = GapLength;
#ifdef _DEBUGW
            swprintf( Mess, L"   LCN: %I64d LEN: %I64d", Gap->StartLcn, Gap->Length );
            OutMessage( Mess );
#endif
            if ( !AddNode( &(VolumeCtx->GapsTree), Gap ) )
            {
                Result = ERROR_NOT_ENOUGH_MEMORY;
                free( Gap );
            }
        }
    //}


    return Result;

} // end of DfrgAddGapToTree


//
// Add/Merge in gaps tree node with gap that was added
// GapsTree sort mode should be SortModeLcn
//
void
DfrgInsertGapNode( 
    IN      PBTR_TREE               GapsTree,
    OUT     PBTR_NODE               *GapNode,
    IN  OUT PDEFRAG_LCN_EXTENT      *InGap )
{
    PBTR_NODE                       NeighborNodeLeft, NeighborNodeRight;
    PDEFRAG_LCN_EXTENT              NeighborGap, Gap = *InGap;


    //
    // Find gap nearest to new from left & right
    //
    NeighborNodeLeft = FindNode( GapsTree, Gap, SortModeLcn, _FIND_LE_ );
    if ( NeighborNodeLeft )
    {
        NeighborNodeRight = FindNextNode( GapsTree, LEFT_TO_RIGHT );
    }
    else
    {
        NeighborNodeRight = FindNode( GapsTree, Gap, SortModeLcn, _FIND_GE_ );
    }

    //
    // Merge with left
    //
    if ( NeighborNodeLeft && NeighborNodeLeft->Body )
    {
        NeighborGap = (PDEFRAG_LCN_EXTENT)NeighborNodeLeft->Body;
        if ( NeighborGap->StartLcn.QuadPart + NeighborGap->Length.QuadPart == Gap->StartLcn.QuadPart )
        {
            Gap->StartLcn = NeighborGap->StartLcn;
            Gap->Length.QuadPart += NeighborGap->Length.QuadPart;

            DeleteNode( GapsTree, NeighborNodeLeft );
        }
    }

    //
    // Merge with right
    //
    if ( NeighborNodeRight && NeighborNodeRight->Body )
    {
        NeighborGap = (PDEFRAG_LCN_EXTENT)NeighborNodeRight->Body;
        if ( Gap->StartLcn.QuadPart + Gap->Length.QuadPart == NeighborGap->StartLcn.QuadPart )
        {
            Gap->Length.QuadPart += NeighborGap->Length.QuadPart;

            DeleteNode( GapsTree, NeighborNodeRight );
        }
    }


    *GapNode = AddNode( GapsTree, *InGap );


} // end of DfrgInsertGapNode

//
// Delete/Update in gaps tree node with gap that was reduced
//
void
DfrgUpdateGapNode( 
    PBTR_TREE               GapsTree,
    PBTR_NODE               *GapNode,
    PDEFRAG_LCN_EXTENT      *Gap )
{

    if ( !(*GapNode) )
    {
        return;
    }

    RemoveNode( GapsTree, *GapNode );
    free( *GapNode );
    *GapNode = NULL;

    if ( (*Gap) )
    {
        if ( (*Gap)->Length.QuadPart > 0 )
        {
            //*GapNode = AddNode( GapsTree, *Gap );
            DfrgInsertGapNode( GapsTree, GapNode, Gap );
            if ( !(*GapNode) )
            {
                free( *Gap );
                *Gap = NULL;
            }
        }
        else
        {
            free( *Gap );
            *Gap = NULL;
        }
    }

#ifdef _DEBUGW
    PrintGapsTree( GapsTree, FALSE );
#endif

} // end of DfrgUpdateGapNode

//
// TODO: refactore to be called in loop
//
int
DfrgBuildGapsTree(
    IN      PDEFRAG_VOLUME_CONTEXT  VolumeCtx,
    IN  OUT PBTR_TREE               GapsTree, 
    OUT     LARGE_INTEGER           *FreeSpace )
{
    int                         Result = NO_ERROR;
    BTR_TREE_METHODS            Methods = { 0 };


    Methods.FreeBody        = NULL; // use simple free
    Methods.CompareItems    = CompareGaps;
    Methods.CopyBody        = NULL;
    Methods.PrintBody       = NULL;

    InitTree( 
        GapsTree, 
        &Methods, 
        SortModeSize );


    //
    // Scan through bitmap looking for empty clusters chains (gaps)
    //
    PVOLUME_BITMAP_BUFFER       BitMap = (PVOLUME_BITMAP_BUFFER)VolumeCtx->BitMapBuffer;
    PBYTE                       BitMapBuffer = BitMap->Buffer;
    LARGE_INTEGER               ClustersNum = VolumeCtx->NtfsData.TotalClusters;
    LARGE_INTEGER               StartLcn = BitMap->StartingLcn;
    LARGE_INTEGER               FirstLcn;
    LARGE_INTEGER               GapLength = { 0 };
    LARGE_INTEGER               CurCluster = { 0 };
    LONGLONG                    i;
    BYTE                        PrevCluster = BitMapBuffer[0] & BitMask[0];

#ifdef _DEBUG
    WCHAR   Mess[MAX_PATH];
    OutMessage( L"" );
    swprintf( Mess, L"   Scan bitmap \n" );
    OutMessage( Mess );
    //GetInput();
#endif

    __try
    {
        // TODO: refactore to be called in loop
        if ( !PrevCluster )
        {
            GapLength.QuadPart = 1;
            FirstLcn.QuadPart = StartLcn.QuadPart;
        }

        // TODO: refactore to be called in loop
        FreeSpace->QuadPart += GapLength.QuadPart;

        for ( i=1; i<ClustersNum.QuadPart; i++ ) 
        {
            //if ( !(BitMapBuffer[i/8] & BitMask[i%8 ]) ) 
            if ( !(BitMapBuffer[i>>3] & BitMask[i&7 ]) ) 
            {
                //
                // Cluster is free
                //
                if ( PrevCluster ) 
                {
                    //
                    // PrevCluster is busy. This is the first cluster in gap.
                    //
                    FirstLcn.QuadPart = StartLcn.QuadPart + i;
                    GapLength.QuadPart = 1;
                    PrevCluster = 0;
                } 
                else 
                {
                    GapLength.QuadPart++;
                }
            } 
            else 
            {
                //
                // Cluster is not free
                //
                if ( !PrevCluster ) 
                {
                    //
                    // The previous cluster is the last cluster in gap
                    //
                    Result = DfrgAddGapToTree( VolumeCtx, FirstLcn.QuadPart, GapLength.QuadPart );
                    if ( NO_ERROR != Result )
                    {
                        break;
                    }

                    FreeSpace->QuadPart += GapLength.QuadPart;

                    PrevCluster = 1;
                } 
            }
        }

        // TODO: refactore to be called in loop
        if ( !PrevCluster ) // i == ClustersNum.QuadPart
        {
            Result = DfrgAddGapToTree( VolumeCtx, FirstLcn.QuadPart, GapLength.QuadPart );
            FreeSpace->QuadPart += GapLength.QuadPart;
        }

#ifdef _DEBUG
        swprintf( Mess, L"Total %I64d gapes: min %I64d .. max %I64d", 
            GapsTree->NodeCnt, 
            GapsTree->Left ? ((PDEFRAG_LCN_EXTENT)GapsTree->Left ->Body)->Length.QuadPart : 0, 
            GapsTree->Right? ((PDEFRAG_LCN_EXTENT)GapsTree->Right->Body)->Length.QuadPart : 0);
        OutMessage( Mess );
        //GetInput();
#endif

    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        Result = GetExceptionCode();
        DislpayErrorMessage( L"DfrgBuildGapsTree(). Exception !!!", Result, ERR_OS );
    }


    return Result;

} // end of DfrgBuildGapsTree

// ===========================
//  General (Volume) Utils
// ===========================

//
// Get Volume bitmap frame
// TODO: iterate in loop as map could take a lot of memory.
//
int
DfrgGetVolumeSpaceMap( 
    PDEFRAG_VOLUME_CONTEXT  VolumeCtx,
    BOOLEAN                 BuildTreeOnly )
{
    int                     Result = ERROR_INVALID_PARAMETER;

    PDEFRAG_JOB_CONTEXT     JobCtx = VolumeCtx->JobCtx;

#ifdef _DEBUG
    WCHAR                   Mess[MAX_PATH] = L"";
#endif


    __try
    {
        //
        // Get Bit Map
        //
        LARGE_INTEGER           BitMapLength;
        PBYTE                   BitMapBuffer = NULL;
        //ULONG_PTR               SectorSizeM1 = VolumeCtx->NtfsData.BytesPerSector - 1;

        //
        // Allocate bit map buffer
        //
        BitMapLength.QuadPart = VolumeCtx->NtfsData.TotalClusters.QuadPart / 8; // +1 included

        if ( !BitMapLength.HighPart )
        {
            BitMapBuffer = (PBYTE) malloc( BitMapLength.LowPart + sizeof(VOLUME_BITMAP_BUFFER) ); //+ SectorSizeM1 );
        }

        if ( BitMapBuffer ) 
        {
            //
            // Adjust buffer to sector bound
            //
            //VolumeCtx->BitMapBufferBase = BitMapBuffer;
            //if ( (ULONG_PTR)BitMapBuffer % (SectorSizeM1+1) )
            //{
            //    BitMapBuffer = (PBYTE)( ((ULONG_PTR)BitMapBuffer & (~SectorSizeM1)) + SectorSizeM1 + 1 );
            //}

            VolumeCtx->BitMapBuffer = BitMapBuffer;
            VolumeCtx->BitMapLength.QuadPart = BitMapLength.QuadPart;
        }
        else
        {
            //
            // TODO: Allocate smaller buffer and iterate in loop 
            //
            Result = ERROR_NOT_ENOUGH_MEMORY;
            __leave;
        }


        LARGE_INTEGER           Lcn = { 0 };

        Result = DfrgGetVolumeBitMap( VolumeCtx, &Lcn );
        if ( NO_ERROR != Result )
        {
            __leave;
        }

        //
        // Build gaps tree (free space map)
        //
        LARGE_INTEGER           FreeSpace = { 0 };

        Result = DfrgBuildGapsTree( VolumeCtx, &(VolumeCtx->GapsTree), &FreeSpace );
        if ( NO_ERROR != Result )
        {
            __leave;
        }

        if ( !VolumeCtx->NtfsData.FreeClusters.QuadPart )
        {
            VolumeCtx->NtfsData.FreeClusters = FreeSpace;
        }
#ifdef _DEBUG
        else if ( VolumeCtx->NtfsData.FreeClusters.QuadPart != FreeSpace.QuadPart )
        {
            swprintf( Mess, L"    NtfsData FreeSpace %I64d Gap  s FreeSpace %I64d", 
                      VolumeCtx->NtfsData.FreeClusters.QuadPart, FreeSpace.QuadPart );
            OutMessage( Mess );
        }
#endif
        
        if ( BuildTreeOnly )
        {
            __leave;
        }

        //
        // Update Statistic
        //
        UpdateGapsStatistic( VolumeCtx );


        //
        // Read Fat 
        //
        if ( Fat12Disk == VolumeCtx->DiskType ||
             Fat16Disk == VolumeCtx->DiskType ||
             Fat32Disk == VolumeCtx->DiskType )
        {
            DWORD           FatLength = VolumeCtx->DosData.FatLength * VolumeCtx->NtfsData.BytesPerSector;

            VolumeCtx->FatBuffer = (PBYTE) malloc( FatLength );
            if ( !VolumeCtx->FatBuffer )
            {
                Result = ERROR_NOT_ENOUGH_MEMORY;
                __leave;
            }

            LARGE_INTEGER   Offset = { 0 };

            Offset.LowPart = VolumeCtx->DosData.FatOffset * VolumeCtx->NtfsData.BytesPerSector; 
            Result = DfrgReadVolumeData( 
                            VolumeCtx, 
                            &Offset,
                            VolumeCtx->FatBuffer,
                            FatLength );
            if ( NO_ERROR != Result )
            {
                __leave;
            }
        }

        //
        // Allocate and set display buffers.
        //
        if ( (DEFRAG_JOB_OPTION_SHOW_PROGRESS & JobCtx->Options) )
        {
            JobCtx->DisplayMapExSize  = DFRG_MAP_BLOCKS_MAX_NUM; 
            JobCtx->DisplayMapEx      = (PDFRG_MAP_BLOCK_INFO) malloc( JobCtx->DisplayMapExSize * sizeof(DFRG_MAP_BLOCK_INFO) );

            if ( !JobCtx->DisplayMapEx ) 
            {
                Result = ERROR_NOT_ENOUGH_MEMORY;
                __leave;
            }
/*
            if ( DFRG_MAP_BLOCKS_MAX_NUM > TotalClusters )
            {
                ClastersInBlock = 1;
            }
            else
            {
                ClastersInBlock   = TotalClusters / ( DFRG_MAP_BLOCKS_MAX_NUM-1); 
            }
*/
            if ( (LONGLONG)JobCtx->DisplayMapExSize > VolumeCtx->NtfsData.TotalClusters.QuadPart )
            {
                //JobCtx->DisplayMapExSize = VolumeCtx->NtfsData.TotalClusters.LowPart;
                JobCtx->ClastersInBlock = 1;
            }
            else
            {
                JobCtx->ClastersInBlock   = (DWORD)VolumeCtx->NtfsData.TotalClusters.QuadPart / (JobCtx->DisplayMapExSize-1); 
            }

            //
            // Update Display (Initialize map)
            //
            SetDisplayMapEx( JobCtx );
#ifdef _DEBUG
            OutMessage( L"DfrgGetVolumeSpaceMap" );
#endif
        }

    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        Result = GetExceptionCode();
        DislpayErrorMessage( L"DfrgGetVolumeSpaceMap(). Exception !!!", Result, ERR_OS );
    }

    //
    // Free Bitmap buffer as it never used.
    //
    if ( VolumeCtx->BitMapBuffer )
    {
        free( VolumeCtx->BitMapBuffer );
        VolumeCtx->BitMapBuffer = NULL;
        VolumeCtx->BitMapLength.QuadPart = 0; 
    }



    return Result;

} // end of DfrgGetVolumeSpaceMap

//
// Get Volume Info 
//
int
DfrgGetVolumeInfo( 
    PDEFRAG_VOLUME_CONTEXT  VolumeCtx )
{
    int                         Result = DFRG_ERROR_UNKNOWN_FS;
    //NTSTATUS                    Status = STATUS_SUCCESS;

    BOOT_SECTOR                 Boot; // = &(Boot);
    LARGE_INTEGER               ByteOffset = { 0 };


    __try
    {
        Result = DfrgReadVolumeData( VolumeCtx, &ByteOffset, Boot.Buffer, sizeof(Boot.Buffer) );
        if ( NO_ERROR != Result )
        {
            __leave;
        }

        if ( NTFS_DRIVE_SIGN  == Boot.Ntfs.Sign ) 
        {
            //
            // NTFS Volume
            //
            VolumeCtx->DiskType = NtfsDisk;

            Result = DfrgGetVolumeNtfsData( VolumeCtx );

        }
        else if ( FAT_BOOT_SIGN == Boot.Dos.FatBootSign ) 
        {
            //
            // DOS Volume.
            // Get volume info from boot sector and save it in VolumeCtx->NtfsData.
            // Note: some Mft* fields are used to keep Fat Volume info.
            //
            VolumeCtx->DiskType = Fat32Disk;

            VolumeCtx->NtfsData.BytesPerSector = Boot.Dos.BytesPerSector;
            if ( !VolumeCtx->NtfsData.BytesPerSector )
            {
                Result = DFRG_ERROR_UNKNOWN_FS;
                __leave;
            }

            VolumeCtx->NtfsData.BytesPerCluster = 
                VolumeCtx->NtfsData.BytesPerSector * Boot.Dos.SectorsPerCluster;
            if ( !VolumeCtx->NtfsData.BytesPerCluster )
            {
                Result = DFRG_ERROR_UNKNOWN_FS;
                __leave;
            }

            VolumeCtx->NtfsData.TotalClusters.QuadPart = Boot.Dos.TotalSectorsNum;
            if ( !VolumeCtx->NtfsData.TotalClusters.QuadPart )
            {
                VolumeCtx->NtfsData.TotalClusters.QuadPart = Boot.Dos.TotalSectorsNum32;
            }

            //
            // Next values are in sectors.
            // TODO.
            //
            VolumeCtx->DosData.FatOffset        = Boot.Dos.ReservedSectorsNum; // = 1 for FAT12 and FAT16, typically = 32 for FAT32 

            VolumeCtx->DosData.FatLength        = Boot.Dos.SectorsPerFat; 
            if ( !VolumeCtx->DosData.FatLength )
            {
                VolumeCtx->DosData.FatLength    = Boot.Dos.Fat32.SectorsPerFat32;
            }

            VolumeCtx->DosData.FatsNumber       = Boot.Dos.FatNum;

            VolumeCtx->DosData.RootDirOffset    = 
                VolumeCtx->DosData.FatOffset + VolumeCtx->DosData.FatLength * VolumeCtx->DosData.FatsNumber;

            VolumeCtx->DosData.RootDirLength    = // Boot.Dos.RootDirLength = 0 for Fat32
                ((Boot.Dos.RootEntryCnt * sizeof(FAT_DIR_RECORD)) + (Boot.Dos.BytesPerSector - 1)) / Boot.Dos.BytesPerSector;

            //
            // The first sector of first data cluster. The first data cluster is cluster 2.
            // For next clusters FirstSectorofCluster = ((N – 2) * BPB_SecPerClus) + FirstDataSector.
            //
            VolumeCtx->DosData.DataOffset = VolumeCtx->DosData.RootDirOffset + VolumeCtx->DosData.RootDirLength;

            //
            // Culculate the number of clusters - the count of data clusters starting at cluster 2. 
            // The maximum valid cluster number for the volume is TotalClusters + 1, 
            // and the “count of clusters including the two reserved clusters” is TotalClusters + 2.
            //
            VolumeCtx->NtfsData.TotalClusters.QuadPart -= VolumeCtx->DosData.DataOffset;
            VolumeCtx->NtfsData.TotalClusters.QuadPart /= Boot.Dos.SectorsPerCluster;

            //
            // This is the one and only way that FAT type is determined. 
            //
            if ( VolumeCtx->NtfsData.TotalClusters.QuadPart < 4085 ) 
            {
                VolumeCtx->DiskType = Fat12Disk;
            } 
            else if ( VolumeCtx->NtfsData.TotalClusters.QuadPart < 65525 ) 
            {
                VolumeCtx->DiskType = Fat16Disk;
            } 
            else 
            {
                VolumeCtx->DiskType = Fat32Disk;

                VolumeCtx->DosData.RootDirOffset = Boot.Dos.Fat32.RootCluster;
                // This field is only defined for FAT32 media and does not exist on FAT12 and FAT16 media. 
                // This is set to the cluster number of the first cluster of the root directory, usually 2 but not required to be 2. 
                // NOTE:  Disk utilities that change the location of the root directory should make every effort to place the first 
                // cluster of the root directory in the first non-bad cluster on the drive (i.e., in cluster 2, unless it’s marked bad). 
                // This is specified so that disk repair utilities can easily find the root directory if this field accidentally gets zeroed. 

            }

#ifdef _DEBUG
            WCHAR                       Mess[MAX_PATH];
            swprintf( Mess, L"" );
            switch( VolumeCtx->DiskType )
            {
            case Fat12Disk:
                wcscat( Mess, L"FAT 12" );
                break;
            case Fat16Disk:
                wcscat( Mess, L"FAT 16" );
                break;
            case Fat32Disk:
                wcscat( Mess, L"FAT 32" );
                break;
            default:
                wcscat( Mess, L"UNKNOWN" );
                break;
            }
            OutMessage( Mess );
#endif

            Result = NO_ERROR;
        }
        else
        {
            //
            // Unknown Volume
            //
            VolumeCtx->DiskType = UnknownDisk;
        }

        //
        // Some checks
        //
        if ( !VolumeCtx->NtfsData.TotalClusters.QuadPart )
        {
            Result = DFRG_ERROR_UNKNOWN_FS;
        }

    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        Result = GetExceptionCode();
        DislpayErrorMessage( L"DfrgGetVolumeInfo(). Exception !!!", Result, ERR_OS );
    }

    return Result;

} // end of DfrgGetVolumeInfo

