/*
    Defrag Engine Display Utils

    Module name:

        DisplayUtils.cpp

    Abstract:

        Defrag Engine display and print functions module.
        Contains display and print functions, etc.

    $Header: /home/CVS_DEFRAG/Defrag/Src/Engine/DisplayMap.cpp,v 1.4 2009/12/23 13:01:47 dimas Exp $
    $Log: DisplayMap.cpp,v $
    Revision 1.4  2009/12/23 13:01:47  dimas
    1. Full File Name processing implemented.
    2. Top 100 Fragmented functionality for all file list requests implemented.

    Revision 1.3  2009/12/17 10:27:20  dimas
    1. Defragmentation of FAT volumes in release build enabled
    2. Debug function GetDisplayBlockCounts() implemented

    Revision 1.2  2009/12/15 16:06:40  dimas
    OP_REMOVE_ALL pseudo operation on settings lists added

    Revision 1.1  2009/11/24 14:52:15  dimas
    no message

*/

#include "stdafx.h"



void
SetDisplayMapEx( 
    PDEFRAG_JOB_CONTEXT         JobCtx )
{
    PDEFRAG_VOLUME_CONTEXT      VolumeCtx;
    ULONGLONG                   j, ClastersInBlock, ClasterNum, TotalClusters;
    DWORD                       MapLength;
    PBYTE                       BitMapBuffer;


    //
    // Ignore if no display/client
    //
    if ( !(DEFRAG_JOB_OPTION_SHOW_PROGRESS & JobCtx->Options) )
    {
        return;
    }


    //
    // Set values
    //
    VolumeCtx = &(JobCtx->VolumeCtx);

    TotalClusters   = VolumeCtx->NtfsData.TotalClusters.QuadPart;
    ClastersInBlock = JobCtx->ClastersInBlock;

    MapLength = JobCtx->DisplayMapExSize;
    if ( (ULONGLONG)MapLength > TotalClusters )
    {
        MapLength = VolumeCtx->NtfsData.TotalClusters.LowPart;
    }

    BitMapBuffer = ((PVOLUME_BITMAP_BUFFER)VolumeCtx->BitMapBuffer)->Buffer;

    //
    // Clear Display Map
    //
    memset( JobCtx->DisplayMapEx, 0, JobCtx->DisplayMapExSize * sizeof(DFRG_MAP_BLOCK_INFO) );


    //
    // Iterate all blocks and set free (FreeSpace) and used (Allocated) cluster counts
    //
    for( DWORD i=0; i<MapLength; i++ )
    {
        ClasterNum = ClastersInBlock * i;

        //
        // Iterate clusters in block
        //
        for ( j=0; j<ClastersInBlock && ClasterNum<TotalClusters; j++, ClasterNum++ ) 
        {
            if ( (BitMapBuffer[ClasterNum>>3] & BitMask[ClasterNum&7 ]) ) 
            {
                //
                // Cluster is Busy 
                //
                ((JobCtx->DisplayMapEx)+i)->Allocated++;
            }
            else
            {
                //
                // Cluster is Free
                //
                ((JobCtx->DisplayMapEx)+i)->FreeSpace++;
            }
        } // for( j  - clusters in block
    } // for( i - map blocks

    //
    // Mark Mft/Fat zone
    //
    DWORD   StartBlock, EndBlock;

    StartBlock = (DWORD)(VolumeCtx->NtfsData.MftZoneStart.QuadPart/ClastersInBlock);
    EndBlock   = (DWORD)(VolumeCtx->NtfsData.MftZoneEnd.QuadPart/ClastersInBlock);

    for( DWORD i=StartBlock; i<EndBlock && i<MapLength; i++ )
    {
        ((JobCtx->DisplayMapEx)+i)->MftZone = (DFRG_COUNT_TYPE)ClastersInBlock;
    }

} // end of SetDisplayMapEx


void
TuneDisplayMapCounts( 
    PDFRG_MAP_BLOCK_INFO        DisplayMapBlock,
    ULONG                       DefragFlags,
    ULONG                       DefragFlags2,
    DFRG_COUNT_TYPE             AddCount,
    DWORD                       Flags )
{

    //
    // Update Allocated and FreeSpace counts
    //
    if ( (Flags & INCL_SPACE_COUNTS) )
    {
        //if ( DisplayMapBlock->Allocated )
            DisplayMapBlock->Allocated += AddCount;
        //if ( DisplayMapBlock->FreeSpace )
            DisplayMapBlock->FreeSpace -= AddCount;
    }

    //
    // Update Display (Flags) counts
    //
    if ( (Flags & INCL_FLAGS_COUNTS) )
    {
        if ( (DefragFlags & DEFRAG_FILE_MFT) )
        {
            DisplayMapBlock->Mft += AddCount;
        }
        if ( (DefragFlags & DEFRAG_FILE_PAGING) )
        {
            DisplayMapBlock->PageFile += AddCount;
        }
        if ( (DefragFlags & DEFRAG_FILE_HIBER) )
        {
            DisplayMapBlock->HiberFile += AddCount;
        }
        if ( (DefragFlags & DEFRAG_FILE_META) )
        {
            DisplayMapBlock->Metadata += AddCount;
        }
        //if ( (DefragFlags & DEFRAG_FILE_SYSTEM) )

        if ( (DefragFlags & DEFRAG_FILE_LOCKED) )
        {
            DisplayMapBlock->LockedFiles += AddCount;
        }
        if ( (DefragFlags & DEFRAG_FILE_DIRECTORY) )
        {
            DisplayMapBlock->Directories += AddCount;
        }
        if ( (DefragFlags & DEFRAG_FILE_COMPRESSED) )
        {
            DisplayMapBlock->CompressedFiles += AddCount;
        }
        if ( (DefragFlags & DEFRAG_FILE_FRAGMENTED) )
        {
            DisplayMapBlock->FragmentedFiles += AddCount;
        }

        if ( (DefragFlags2 & DEFRAG_FILE_RECENTLY_MODIFIED) )
        {
            DisplayMapBlock->RecentlyModifiedFiles += AddCount;
        }
        else
        {
            DisplayMapBlock->LeastModifiedFiles += AddCount;
        }
    }

    //
    // Update Inproc counts
    //
    if ( (Flags & INCL_INPROC_COUNTS) )
    {
        DisplayMapBlock->BeingProcessed -= AddCount;
    }


} // end of TuneDisplayMapCounts


void
UpdateDisplayForExtent(
    PDEFRAG_FILE_CONTEXT        FileCtx,
    ULONGLONG                   ExtentStart,
    ULONGLONG                   ExtentLength,
    DWORD                       Flags )
{
    PDEFRAG_VOLUME_CONTEXT      VolumeCtx = FileCtx->VolumeCtx;
    PDEFRAG_JOB_CONTEXT         JobCtx = VolumeCtx->JobCtx;
    DWORD                       MapLength, StartBlock, EndBlock, Before, After;
    DWORD                       ClastersInBlock = JobCtx->ClastersInBlock;
    int                         ClustersCnt;


    if ( !ExtentLength )
    {
        return;
    }

    StartBlock = (DWORD)(ExtentStart / ClastersInBlock);
    Before = (DWORD)(ExtentStart % ClastersInBlock);

    EndBlock   = (DWORD)((ExtentStart + ExtentLength) / ClastersInBlock);
    After = (DWORD)((ExtentStart + ExtentLength) % ClastersInBlock);

    if ( After )
    {
        After = (DWORD)ClastersInBlock - After;
    }
    else
    {
        EndBlock--;
    }

    MapLength = JobCtx->DisplayMapExSize;
    if ( (LONGLONG)MapLength > VolumeCtx->NtfsData.TotalClusters.QuadPart )
    {
        MapLength = VolumeCtx->NtfsData.TotalClusters.LowPart;
    }

    for( DWORD j=StartBlock; j<=EndBlock && j<MapLength; j++ )
    {
        ClustersCnt = ClastersInBlock;

        if ( j == StartBlock )
        {
            ClustersCnt -= Before;
        }
        if ( j == EndBlock )
        {
            ClustersCnt -= After;
        }

        if ( (Flags & SUBSTRACT_CLASTERS) )
        {
            ClustersCnt = -(ClustersCnt);
        }

        TuneDisplayMapCounts( 
            (JobCtx->DisplayMapEx)+j, 
            FileCtx->DefragFlags, 
            FileCtx->DefragFlags2, 
            (DFRG_COUNT_TYPE)ClustersCnt,
            Flags );

    } // for ( blocks


} // end of UpdateDisplayForExtent


void
GetDisplayBlockCounts(
    PDEFRAG_JOB_CONTEXT         JobCtx,
    ULONGLONG                   Cluster,
    PDFRG_MAP_BLOCK_INFO        DisplayMapBlock,
    DWORD                       *BlockNum )
{
    DWORD                       ClastersInBlock = JobCtx->ClastersInBlock;


    *BlockNum = (DWORD)(Cluster / ClastersInBlock);

    if ( DisplayMapBlock && JobCtx->DisplayMapEx )
    {
        memcpy( DisplayMapBlock, (JobCtx->DisplayMapEx)+(*BlockNum), sizeof(DFRG_MAP_BLOCK_INFO) );
    }
    else
    {
        memset( DisplayMapBlock, 0, sizeof(DFRG_MAP_BLOCK_INFO) );
    }

} // end of GetDisplayBlockCounts


void
UpdateDisplayForStream( 
    PDEFRAG_FILE_CONTEXT        FileCtx,
    PDEFRAG_STREAM_CONTEXT      StreamCtx,
    DWORD                       Flags )
{
    PDEFRAG_VOLUME_CONTEXT      VolumeCtx = FileCtx->VolumeCtx;
    PDEFRAG_JOB_CONTEXT         JobCtx = VolumeCtx->JobCtx;
    DWORD                       MapLength;

#ifdef _DEBUG
    BOOLEAN                     DebugFlag = FALSE;
    WCHAR                       Mess[MAX_PATH];
#endif


__try
{
    //
    // Ignore if no display/client
    //
    if ( !(DEFRAG_JOB_OPTION_SHOW_PROGRESS & JobCtx->Options) )
    {
        return;
    }

    //
    // Ignore bad clusters map
    //
    if ( FileCtx->FName && !wcscmp( FileCtx->FName, L"$BadClus" ) ) // $UsnJrnl
    {
        return;
    }

    PRETRIEVAL_POINTERS_BUFFER  ClustersMap;
    LARGE_INTEGER               StartingVcn;
    PDEFRAG_STREAM_CONTEXT      CurStreamCtx = FileCtx->FirstStream;

    MapLength = JobCtx->DisplayMapExSize;
    if ( (LONGLONG)MapLength > VolumeCtx->NtfsData.TotalClusters.QuadPart )
    {
        MapLength = VolumeCtx->NtfsData.TotalClusters.LowPart;
    }

    if ( StreamCtx )
    {
        CurStreamCtx = StreamCtx;
    }

    while ( CurStreamCtx && (ClustersMap = CurStreamCtx->ClustersMap) )
    {
        StartingVcn = ClustersMap->StartingVcn;

        for ( DWORD i=0; i<ClustersMap->ExtentCount; 
            StartingVcn = ClustersMap->Extents[i].NextVcn, i++ )
        {
            if ( VIRTUAL_LCN != ClustersMap->Extents[i].Lcn.QuadPart ) 
            {
#if 0
                ULONGLONG                   ExtentStart;
                if ( VolumeCtx->DiskType != NtfsDisk )
                {
                    //
                    // Adjust offset on FAT volume
                    //
                    ExtentStart = ClustersMap->Extents[i].Lcn.QuadPart - 2;
                }
                else
                {
                    ExtentStart = ClustersMap->Extents[i].Lcn.QuadPart;
                }
#endif
                UpdateDisplayForExtent( 
                    FileCtx,
                    ClustersMap->Extents[i].Lcn.QuadPart, // ExtentStart
                    ClustersMap->Extents[i].NextVcn.QuadPart - StartingVcn.QuadPart,   // ExtentLength 
                    Flags );

            } // if ( !VIRTUAL_LCN
        } // for ( i - extents

        if ( StreamCtx )
        {
            //
            // If we should process only one stream - exit
            //
            break;
        }

        CurStreamCtx = CurStreamCtx->NextStream;

    } // while ( streams

}
__except(EXCEPTION_EXECUTE_HANDLER)
{
    int Result = GetExceptionCode();
    DislpayErrorMessage( L"UpdateDisplayForStream(). Exception !!!", Result, ERR_OS );
#ifdef _DEBUG
    swprintf( Mess, L"   FileCtx %p : '%ls'", 
        FileCtx, FileCtx->FName ? FileCtx->FName : L"NULL" );
    OutMessage( Mess );
#endif
}

} // end of UpdateDisplayForStream


#if 0

void
UpdateBitMap( 
    PDEFRAG_FILE_CONTEXT            FileCtx,
    ULONGLONG                       GapStartLcn,
    ULONGLONG                       ExtentStartLcn,
    ULONGLONG                       ExtentLength )
{
    PDEFRAG_VOLUME_CONTEXT          VolumeCtx = FileCtx->VolumeCtx;
    PBYTE                           BitMapBuffer = ((PVOLUME_BITMAP_BUFFER)VolumeCtx->BitMapBuffer)->Buffer;
    ULONGLONG                       i;

    ULONGLONG                       Freed = ExtentStartLcn;
    ULONGLONG                       Occupied = GapStartLcn;


    //
    // Update bitmap
    //
    for ( i=0; i<ExtentLength; i++, Occupied++, Freed++ ) 
    {
        BitMapBuffer[Freed>>3]      &= ~BitMask[Freed&7];
        BitMapBuffer[Occupied>>3]   |=  BitMask[Occupied&7];
    }

/*
    //
    // Ignore if no display/client
    //
    if ( !(DEFRAG_JOB_OPTION_SHOW_PROGRESS & JobCtx->Options) )
    {
        return;
    }

    //
    // Update display map
    //
    ULONGLONG                       BlocksNum;

    if ( (DEFRAG_JOB_OPTION_SHOW_PROGRESS & VolumeCtx->JobCtx->Options) )
    {
        PDFRG_MAP_BLOCK_INFO    DisplayMapEx   = VolumeCtx->JobCtx->DisplayMapEx;
        ULONGLONG               ClastersInBlock, ClastersGroupInBlock;

        ClastersInBlock = VolumeCtx->ClustersNum.QuadPart / VolumeCtx->JobCtx->DisplayMapExSize; 
        ClastersGroupInBlock = ClastersInBlock >> 3; //  / 8; 

        Freed       = ExtentStartLcn / ClastersInBlock;
        Occupied    = GapStartLcn / ClastersInBlock;
        BlocksNum   = ExtentLength / ClastersInBlock;

        for ( i=0; i<BlocksNum; i++, Occupied++, Freed++ ) 
        {
            TuneDisplayMapCounts( DisplayMapEx+Occupied, FileCtx->DefragFlags, 1 );
            TuneDisplayMapCounts( DisplayMapEx+Freed, FileCtx->DefragFlags, (DFRG_COUNT_TYPE)(-1) );
        } // // for ( blocks
    }
*/
} // end of UpdateBitMap

#endif