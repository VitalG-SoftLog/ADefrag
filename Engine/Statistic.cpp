/*
    Defrag Engine Display Utils

    Module name:

        DisplayUtils.cpp

    Abstract:

        Defrag Engine display and print functions module.
        Contains display and print functions, etc.

    $Header: /home/CVS_DEFRAG/Defrag/Src/Engine/Statistic.cpp,v 1.4 2009/12/29 09:50:16 dimas Exp $
    $Log: Statistic.cpp,v $
    Revision 1.4  2009/12/29 09:50:16  dimas
    Bug with sparsed files fragmentation detection fixed

    Revision 1.3  2009/12/23 13:01:47  dimas
    1. Full File Name processing implemented.
    2. Top 100 Fragmented functionality for all file list requests implemented.

    Revision 1.2  2009/12/21 17:00:13  dimas
    1. DEFRAG_CMD_GET_MOST_FRAGMENTED and similar requests implemented
    2. Basic Exclude files check added

    Revision 1.1  2009/11/24 14:52:15  dimas
    no message

*/

#include "stdafx.h"



// ===========================
//  Statistic Utils
// ===========================

#ifdef _DEBUG

//#include <math.h>

#if 0
float                   LoadFloatingPointSupport = 3.1415926F;
extern "C" float _fltused;

    double LocalLoadFloatingPointSupport = sqrtf(LoadFloatingPointSupport/2.F);

    LoadFloatingPointSupport = _fltused;
#endif


void
PrintStatistic(
    IN  PDEFRAG_JOB_CONTEXT     JobCtx )
{

#if (defined _DEBUG) && !(defined DEFRAG_NATIVE_APP)
//
// swprintf from ntdll do not support float formats
//

    PDEFRAG_VOLUME_CONTEXT     VolumeCtx = &(JobCtx->VolumeCtx);

    WCHAR                       Mess[MAX_PATH] = L"";


    OutMessage( L"" );
    swprintf( Mess, L"Volume %ls %ls.  Result %d  Status %08X\n", 
                VolumeCtx->VName, (JobCtx->Status&DEFRAG_JOB_FLAG_PROCESSED)?L"Processed":L"Analized", 
                JobCtx->Result, JobCtx->Status );
    OutMessage( Mess );

    swprintf( Mess, L"   Files %I64d [%I64d],  Dirs %I64d [%I64d],  Unmovable %I64d [%I64d], Defragmented %I64d [%I64d]", 
              VolumeCtx->Statistic.FilesNum,
              VolumeCtx->Statistic.FilesSize, 
              VolumeCtx->Statistic.DirsNum, 
              VolumeCtx->Statistic.DirsSize, 
              VolumeCtx->Statistic.UnmovableFilesNum,
              VolumeCtx->Statistic.UnmovableFilesSize,
              VolumeCtx->Statistic.DefragmentedFilesNum,
              VolumeCtx->Statistic.DefragmentedFilesSize );
    OutMessage( Mess );

    swprintf( Mess, L"   Recent %I64d [%I64d],  Medium %I64d  [%I64d],  Old %I64d  [%I64d]", 
              VolumeCtx->Statistic.RecentlyModifiedNum,
              VolumeCtx->Statistic.RecentlyModifiedSize, 
              VolumeCtx->Statistic.OccasionalyModifiedNum, 
              VolumeCtx->Statistic.OccasionalyModifiedSize, 
              VolumeCtx->Statistic.RarelyModifiedNum,
              VolumeCtx->Statistic.RarelyModifiedSize );
    OutMessage( Mess );

    swprintf( Mess, L"   Exe %I64d [%I64d],  Txt %I64d  [%I64d],  Pic %I64d  [%I64d],  Video %I64d  [%I64d],  Audio %I64d  [%I64d]\n", 
              VolumeCtx->Statistic.ProgramFilesNum,
              VolumeCtx->Statistic.ProgramFilesSize, 
              VolumeCtx->Statistic.TextFilesNum,
              VolumeCtx->Statistic.TextFilesSize,
              VolumeCtx->Statistic.GraphicFilesNum,
              VolumeCtx->Statistic.GraphicFilesSize,
              VolumeCtx->Statistic.VideoFilesNum,
              VolumeCtx->Statistic.VideoFilesSize,
              VolumeCtx->Statistic.MusicFilesNum,
              VolumeCtx->Statistic.MusicFilesSize );
    OutMessage( Mess );


    swprintf( Mess, L"   FreeSpace:  Num %I64d  Size %I64d  MaxLength %I64d  Ratio %.2f", //%d.%2d %%", 
              VolumeCtx->Statistic.GapsNum.QuadPart, 
              VolumeCtx->Statistic.FreeSpaceSize.QuadPart, 
              VolumeCtx->Statistic.MaxFreeSpaceGap.QuadPart, 
              VolumeCtx->Statistic.FreeSpaceFragmentationRatio );
              //(int)VolumeCtx->Statistic.FreeSpaceFragmentationRatio,
              //(int)(VolumeCtx->Statistic.FreeSpaceFragmentationRatio * 100.) % 100 );
    OutMessage( Mess );

    swprintf( Mess, L"   Fragmented: Num %I64d  Size %I64d  ExcessNum %I64d  Ratio %.2f", 
              VolumeCtx->Statistic.FragmentedFilesNum, 
              VolumeCtx->Statistic.FragmentedFilesSize, 
              VolumeCtx->Statistic.FragmentedFilesExcessNum, 
              VolumeCtx->Statistic.FragmentedFilesFragmentationRatio );
    OutMessage( Mess );

    swprintf( Mess, L"   Boot Files: Num %I64d  Size %I64d  ExcessNum %I64d  Ratio %.02f", 
              VolumeCtx->Statistic.BootFilesNum, 
              VolumeCtx->Statistic.BootFilesSize, 
              VolumeCtx->Statistic.BootFilesExcessNum, 
              VolumeCtx->Statistic.BootFilesFragmentationRatio );
    OutMessage( Mess );

    swprintf( Mess, L"   Page Files: Num %I64d  Size %I64d  ExcessNum %I64d  Ratio %.02f", 
              VolumeCtx->Statistic.PagefileFilesNum, 
              VolumeCtx->Statistic.PagefileFilesSize, 
              VolumeCtx->Statistic.PagefileFilesExcessNum, 
              VolumeCtx->Statistic.PagefileFilesFragmentationRatio );
    OutMessage( Mess );

    swprintf( Mess, L"   Mft       : Num %I64d  Size %I64d  ExcessNum %I64d  Ratio %.02f", 
              VolumeCtx->Statistic.MftNum, 
              VolumeCtx->Statistic.MftSize, 
              VolumeCtx->Statistic.MftExcessNum, 
              VolumeCtx->Statistic.MftFragmentationRatio );
    OutMessage( Mess );

    swprintf( Mess, L"   Metadata  : Num %I64d  Size %I64d  ExcessNum %I64d  Ratio %.02f", 
              VolumeCtx->Statistic.MetadataFilesNum, 
              VolumeCtx->Statistic.MetadataFilesSize, 
              VolumeCtx->Statistic.MetadataFilesExcessNum, 
              VolumeCtx->Statistic.MetadataFilesFragmentationRatio );
    OutMessage( Mess );

#endif // _DEBUG & !NATIVE

}

#endif // _DEBUG 



DEFRAG_RATIO_TYPE       
DfrgCulcFileFragmentation( 
    PDEFRAG_FILE_CONTEXT    FileCtx )
{
    DEFRAG_RATIO_TYPE       CurrentFileFragmentation = 0.0;


    if ( FileCtx->TotalFragmentsNum.QuadPart > 1 && FileCtx->TotalClustersNum.QuadPart )
    {
        CurrentFileFragmentation = 
            (FileCtx->TotalFragmentsNum.QuadPart * 100.F) / FileCtx->TotalClustersNum.QuadPart;
    }

    return CurrentFileFragmentation;

} // end of DfrgCulcFileFragmentation


void
UpdateCurrentNameStatistic( 
    PDEFRAG_JOB_CONTEXT     JobCtx )
{
    PDEFRAG_VOLUME_CONTEXT  VolumeCtx = &(JobCtx->VolumeCtx);

    LONG                    NameLength = 0;
    PWCHAR                  VolumeNamePtr = NULL;

/*
    NameLength = min( MAX_PATH, (LONG)UniGetLen( &(JobCtx->CurrentFileName) ) );
    VolumeNamePtr = wcschr( JobCtx->CurrentFileName.Buffer, L':' );
    if ( VolumeNamePtr && VolumeNamePtr-1 > JobCtx->CurrentFileName.Buffer )
    {
        VolumeNamePtr--;
    }
    else
    {
        VolumeNamePtr = JobCtx->CurrentFileName.Buffer;
    }
*/

    NameLength = (LONG)UniGetLen( &(JobCtx->CurrentFileName) );
    VolumeNamePtr = JobCtx->CurrentFileName.Buffer;

    if ( wcsstr( JobCtx->CurrentFileName.Buffer, L"\\??\\" ) )
    {
        VolumeNamePtr += 4;
        NameLength -= 4;
    }

    NameLength = min( MAX_PATH, NameLength );

    memcpy( VolumeCtx->Statistic.CurrentFileName, VolumeNamePtr, NameLength*sizeof(WCHAR) );
    VolumeCtx->Statistic.CurrentFileName[NameLength] = L'\0';

#ifdef _DEBUGW
    //if ( NtfsDisk != VolumeCtx->DiskType )
    {
        WCHAR       Mess[MAX_PATH];
        swprintf( Mess, L"# STAT: CurrentFileName '%ls'\n",
            VolumeCtx->Statistic.CurrentFileName );
        OutMessage( Mess );
    }
#endif

} // end of UpdateCurrentNameStatistic


void
UpdateFilesStatistic( 
    PDEFRAG_VOLUME_CONTEXT      VolumeCtx,
    PDEFRAG_FILE_CONTEXT        FileCtx )
{
    PDEFRAG_JOB_CONTEXT         JobCtx = VolumeCtx->JobCtx;

    DEFRAG_RATIO_TYPE           CurrentFileFragmentation;
    PDEFRAG_VOLUME_STATISTIC    Statistic = &(VolumeCtx->Statistic);
    ULONGLONG                   FileSize = FileCtx->TotalClustersNum.QuadPart * VolumeCtx->NtfsData.BytesPerCluster;
    ULONGLONG                   FragmentsNum = FileCtx->TotalFragmentsNum.QuadPart;

#ifdef _DEBUG
    WCHAR       Mess[MAX_PATH] = L"";
#endif

__try
{

    Statistic->FilesNum.QuadPart++;
    Statistic->FilesSize.QuadPart += FileSize;

    if ( (FileCtx->DefragFlags & DEFRAG_FILE_DIRECTORY) )
    {
        Statistic->DirsNum.QuadPart++;
        Statistic->DirsSize.QuadPart += FileSize;
    }


    if ( !(DEFRAG_JOB_OPTION_SHOW_PROGRESS & JobCtx->Options) ) // ???
    {
        return;
    }


    if ( (FileCtx->DefragFlags & DEFRAG_FILE_META) )
    {
        if ( (FileCtx->DefragFlags & DEFRAG_FILE_MFT) )
        {
            Statistic->MftNum.QuadPart++;
            Statistic->MftSize.QuadPart += FileSize;
            Statistic->MftExcessNum.QuadPart += FragmentsNum;

            CurrentFileFragmentation = DfrgCulcFileFragmentation( FileCtx );

            Statistic->MftFragmentationRatio += 
                (CurrentFileFragmentation - Statistic->MftFragmentationRatio) / 
                    Statistic->MftNum.QuadPart;
        }
        //else // ???

        Statistic->MetadataFilesNum.QuadPart++;
        Statistic->MetadataFilesSize.QuadPart += FileSize;
        Statistic->MetadataFilesExcessNum.QuadPart += FragmentsNum;

        CurrentFileFragmentation = DfrgCulcFileFragmentation( FileCtx );

        Statistic->MetadataFilesFragmentationRatio += 
            (CurrentFileFragmentation - Statistic->MetadataFilesFragmentationRatio) / 
                Statistic->MftNum.QuadPart;
    }

    if ( (FileCtx->DefragFlags & DEFRAG_FILE_PAGING) )
    {
        Statistic->PagefileFilesNum.QuadPart++;
        Statistic->PagefileFilesSize.QuadPart += FileSize;
        Statistic->PagefileFilesExcessNum.QuadPart += FragmentsNum;

        CurrentFileFragmentation = DfrgCulcFileFragmentation( FileCtx );

        Statistic->PagefileFilesFragmentationRatio += 
            (CurrentFileFragmentation - Statistic->PagefileFilesFragmentationRatio) / 
                Statistic->PagefileFilesNum.QuadPart;
    }


    if ( (FileCtx->DefragFlags & DEFRAG_FILE_BOOT) )
    {
        Statistic->BootFilesNum.QuadPart++;
        Statistic->BootFilesSize.QuadPart += FileSize;
        Statistic->BootFilesExcessNum.QuadPart += FragmentsNum;

        CurrentFileFragmentation = DfrgCulcFileFragmentation( FileCtx );

        Statistic->BootFilesFragmentationRatio += 
            (CurrentFileFragmentation - Statistic->BootFilesFragmentationRatio) / 
                Statistic->BootFilesNum.QuadPart;
    }


    if ( (FileCtx->DefragFlags & DEFRAG_FILE_FRAGMENTED) )
    {
        Statistic->FragmentedFilesNum.QuadPart++;
        Statistic->FragmentedFilesSize.QuadPart += FileSize;
        Statistic->FragmentedFilesExcessNum.QuadPart += FragmentsNum;

        DEFRAG_RATIO_TYPE   CurrentFileFragmentation = DfrgCulcFileFragmentation( FileCtx );

        Statistic->FragmentedFilesFragmentationRatio += 
            (CurrentFileFragmentation - Statistic->FragmentedFilesFragmentationRatio) / 
                Statistic->FragmentedFilesNum.QuadPart;

#ifdef _DEBUGW
        DEFRAG_RATIO_TYPE   PrevFilesFragmentationRatio = Statistic->FragmentedFilesFragmentationRatio;
        if ( (int)Statistic->FragmentedFilesFragmentationRatio < 0 ||
            Statistic->FragmentedFilesFragmentationRatio > 100 )
        {
            swprintf( Mess, L"*** On file '%ls' ratio %d ***\n", 
                FileCtx->FName?FileCtx->FName:L"NULL", (int)Statistic->FragmentedFilesFragmentationRatio );
            OutMessage( Mess );
        }
#endif

        //if ( FileCtx->DefaultStream && FileCtx->DefaultStream->ClustersMap )
        //{
        //    if ( FileCtx->DefaultStream->ClustersMap->ExtentCount > Statistic->MaxFragmentesNum )
        //    {
        //        Statistic->MaxFragmentesNum = FileCtx->DefaultStream->ClustersMap->ExtentCount;
        //    }
        //}
    }

    if ( (FileCtx->DefragFlags & (DEFRAG_FILE_UNMOVABLE | DEFRAG_FILE_LOCKED)) ) // DEFRAG_FILE_EXCLUDED | 
    {
        Statistic->UnmovableFilesNum.QuadPart++;
        Statistic->UnmovableFilesSize.QuadPart += FileSize;
    }


    if ( (FileCtx->DefragFlags2 & DEFRAG_FILE_RECENTLY_MODIFIED) )
    {
        Statistic->RecentlyModifiedNum.QuadPart++;
        Statistic->RecentlyModifiedSize.QuadPart += FileSize;
    }
    else if ( (FileCtx->DefragFlags2 & DEFRAG_FILE_RARELY_MODIFIED) )
    {
        Statistic->RarelyModifiedNum.QuadPart++;
        Statistic->RarelyModifiedSize.QuadPart += FileSize;
    }
    else
    {
        Statistic->OccasionalyModifiedNum.QuadPart++;
        Statistic->OccasionalyModifiedSize.QuadPart += FileSize;
    }


    if ( (DEFRAG_JOB_OPTION_SHOW_PROGRESS & JobCtx->Options) )
    {
        //
        // Classify file by type
        //
        if ( !(FileCtx->DefragFlags & DEFRAG_FILE_TEMPORARY) &&
            !(FileCtx->DefragFlags & DEFRAG_FILE_SYSTEM) && 
            FileCtx->FName )
        {
            DWORD   NameLength = (DWORD)wcslen( FileCtx->FName );

            if ( NameLength >= 4 )
            {
                WCHAR       *Ext = FileCtx->FName + NameLength - 4;

                if ( *Ext == L'.' )
                {
                    Ext++;
                    if ( !_wcsicmp( Ext, L"tmp" ) )
                    {
                        FileCtx->DefragFlags |= DEFRAG_FILE_TEMPORARY;
                    }
                    else if ( !_wcsicmp( Ext, L"txt" ) || 
                              !_wcsicmp( Ext, L"xls" ) ||
                              !_wcsicmp( Ext, L"doc" ) )
                    {
                        FileCtx->DefragFlags2 |= DEFRAG_FILE2_TEXT_DOC;
                    }
                    else if ( !_wcsicmp( Ext, L"exe" ) || 
                              !_wcsicmp( Ext, L"com" ) ||
                              !_wcsicmp( Ext, L"dll" ) ||
                              !_wcsicmp( Ext, L"bin" ) ||
                              !_wcsicmp( Ext, L"sys" ) ||
                              !_wcsicmp( Ext, L"vxd" ) ||
                              !_wcsicmp( Ext, L"drv" ) )
                    {
                        FileCtx->DefragFlags2 |= DEFRAG_FILE2_EXEC;
                    }
                    else if ( !_wcsicmp( Ext, L"mp3" ) || 
                              !_wcsicmp( Ext, L"wav" ) )
                    {
                        FileCtx->DefragFlags2 |= DEFRAG_FILE2_MUSIC;
                    }
                    else if ( !_wcsicmp( Ext, L"mp4" ) || 
                              !_wcsicmp( Ext, L"avi" ) )
                    {
                        FileCtx->DefragFlags2 |= DEFRAG_FILE2_VIDEO;
                    }
                    else if ( !_wcsicmp( Ext, L"jpg" ) || 
                              !_wcsicmp( Ext, L"gif" ) ||
                              !_wcsicmp( Ext, L"pic" ) ||
                              !_wcsicmp( Ext, L"tif" ) ||
                              !_wcsicmp( Ext, L"bmp" ) )
                    {
                        FileCtx->DefragFlags2 |= DEFRAG_FILE2_GRAPHIC;
                    }
                }
                else if ( NameLength >= 5 && *(Ext-1) == L'.' )
                {
                    if ( !_wcsicmp( Ext, L"mpeg" ) )
                    {
                        FileCtx->DefragFlags2 |= DEFRAG_FILE2_VIDEO;
                    }
                    else if ( !_wcsicmp( Ext, L"jpeg" ) || 
                              !_wcsicmp( Ext, L"tiff" ) )
                    {
                        FileCtx->DefragFlags2 |= DEFRAG_FILE2_GRAPHIC;
                    }
                }
            }
        }
    }

    if ( (FileCtx->DefragFlags & DEFRAG_FILE_TEMPORARY) )
    {
        Statistic->TmpFilesNum.QuadPart++;
        Statistic->TmpFilesSize.QuadPart += FileSize;
    }
    else 
    {
        //
        // Classify file by type
        //
        switch ( (FileCtx->DefragFlags2 & DEFRAG_FILE2_TYPE_MASK) )
        {
        case DEFRAG_FILE2_TEXT_DOC:

            Statistic->TextFilesNum.QuadPart++;
            Statistic->TextFilesSize.QuadPart += FileSize;

            break;

        case DEFRAG_FILE2_EXEC:

            Statistic->ProgramFilesNum.QuadPart++;
            Statistic->ProgramFilesSize.QuadPart += FileSize;

            break;

        case DEFRAG_FILE2_GRAPHIC:

            Statistic->GraphicFilesNum.QuadPart++;
            Statistic->GraphicFilesSize.QuadPart += FileSize;

            break;

        case DEFRAG_FILE2_VIDEO:

            Statistic->VideoFilesNum.QuadPart++;
            Statistic->VideoFilesSize.QuadPart += FileSize;

            break;

        case DEFRAG_FILE2_MUSIC:

            Statistic->MusicFilesNum.QuadPart++;
            Statistic->MusicFilesSize.QuadPart += FileSize;

            break;

        default:
            break;
        }
    }

}
__except(EXCEPTION_EXECUTE_HANDLER)
{
    int res = GetExceptionCode();
    DislpayErrorMessage( L"UpdateFilesStatistic(). Exception !!!", res, ERR_OS );
#ifdef _DEBUG
    swprintf( Mess, L"   FileCtx %p : '%ls'", 
        FileCtx, FileCtx->FName ? FileCtx->FName : L"NULL" );
    OutMessage( Mess );
#endif
}

} // end of UpdateFilesStatistic


void
UpdateGapsStatistic( 
    PDEFRAG_VOLUME_CONTEXT  VolumeCtx )
{
    PDEFRAG_JOB_CONTEXT     JobCtx = VolumeCtx->JobCtx;


    if ( !(DEFRAG_JOB_OPTION_SHOW_PROGRESS & JobCtx->Options) ) // ???
    {
        return;
    }

    if ( VolumeCtx->GapsTree.Right && VolumeCtx->GapsTree.Right->Body )
    {
        VolumeCtx->Statistic.MaxFreeSpaceGap.QuadPart = 
            ((PDEFRAG_LCN_EXTENT)VolumeCtx->GapsTree.Right->Body)->Length.QuadPart * VolumeCtx->NtfsData.BytesPerCluster;;
    }

    VolumeCtx->Statistic.GapsNum = VolumeCtx->GapsTree.NodeCnt;

    VolumeCtx->Statistic.FreeSpaceSize.QuadPart = 
        VolumeCtx->NtfsData.FreeClusters.QuadPart * VolumeCtx->NtfsData.BytesPerCluster;

    if ( VolumeCtx->NtfsData.FreeClusters.QuadPart )
    {
        VolumeCtx->Statistic.FreeSpaceFragmentationRatio = 
            ((VolumeCtx->GapsTree.NodeCnt.QuadPart * 100.F) / VolumeCtx->NtfsData.FreeClusters.QuadPart);
    }

    //
    // TODO: 
    //
    // VolumeCtx->Statistic.FreeSpaceInsideMftSize

} // end of UpdateGapsStatistic


