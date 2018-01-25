/*
    Defrag Engine medium level functions module

    Module name:

        DefragUtils.cpp

    Abstract:

        Defrag Engine medium level functions module. 
        Contains medium level defragmentation functions.

    $Header: /home/CVS_DEFRAG/Defrag/Src/Engine/DefragSmartUtils.cpp,v 1.11 2009/12/25 13:48:53 dimas Exp $
    $Log: DefragSmartUtils.cpp,v $
    Revision 1.11  2009/12/25 13:48:53  dimas
    Debug artefacts removed

    Revision 1.10  2009/12/24 12:23:50  dimas
    Some bugs fixed

    Revision 1.9  2009/12/23 13:01:46  dimas
    1. Full File Name processing implemented.
    2. Top 100 Fragmented functionality for all file list requests implemented.

    Revision 1.8  2009/12/21 17:00:13  dimas
    1. DEFRAG_CMD_GET_MOST_FRAGMENTED and similar requests implemented
    2. Basic Exclude files check added

    Revision 1.7  2009/12/02 14:42:15  dimas
    App/Service interaction improved

    Revision 1.6  2009/12/01 13:29:52  dimas
    Smart modes improved.

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


#ifdef _DEBUG

void
PrintGap(
    WCHAR                   *Mess, 
    WCHAR                   *Prefix,
    PDEFRAG_LCN_EXTENT      WrkGap, 
    WCHAR                   *Suffix = NULL );

void
PrintExtent(
    WCHAR                   *Mess, 
    WCHAR                   *Prefix,
    PDEFRAG_FILES_EXTENT    WrkExtent,
    WCHAR                   *Suffix = NULL );

void
PrintMarker( 
    WCHAR                   *Mess, 
    WCHAR                   *Prefix,
    PDEFRAG_LCN_EXTENT      CurrentGap, 
    PDEFRAG_FILES_EXTENT    MarkerExtent,
    WCHAR                   *Suffix = NULL );


void
PrintGap(
    WCHAR                   *Mess, 
    WCHAR                   *Prefix,
    PDEFRAG_LCN_EXTENT      WrkGap, 
    WCHAR                   *Suffix )
{

    if ( Prefix )
    {
        wcscpy( Mess, Prefix );
    }
    else
    {
        *Mess = 0;
    }

    if ( WrkGap )
    {
        swprintf( Mess+wcslen(Mess), L" %I64d..%I64d [%I64d] ",
            WrkGap->StartLcn.QuadPart, 
            WrkGap->StartLcn.QuadPart + WrkGap->Length.QuadPart - 1,
            WrkGap->Length.QuadPart );
    }
    else
    {
        wcscat( Mess, L" Gap ### " );
    }

    if ( Suffix )
    {
        wcscat( Mess, Suffix );
    }

} // enf of PrintGap

void
PrintExtent(
    WCHAR                   *Mess, 
    WCHAR                   *Prefix,
    PDEFRAG_FILES_EXTENT    WrkExtent,
    WCHAR                   *Suffix )
{

    if ( Prefix )
    {
        wcscpy( Mess, Prefix );
    }
    else
    {
        *Mess = 0;
    }

    if ( WrkExtent )
    {
        swprintf( Mess+wcslen(Mess), L" %I64d %I64d..%I64d [%I64d] ",
            WrkExtent->StartVcn.QuadPart, 
            WrkExtent->StartLcn.QuadPart, 
            WrkExtent->StartLcn.QuadPart + WrkExtent->Length.QuadPart - 1,
            WrkExtent->Length.QuadPart );

    }
    else
    {
        wcscat( Mess, L" NULL ### " );
    }

    if ( Suffix )
    {
        wcscat( Mess, Suffix );
    }

} // enf of PrintExtent


void
PrintMarker( 
    WCHAR                   *Mess, 
    WCHAR                   *Prefix,
    PDEFRAG_LCN_EXTENT      CurrentGap, 
    PDEFRAG_FILES_EXTENT    MarkerExtent,
    WCHAR                   *Suffix )
{

    if ( Prefix )
    {
        wcscpy( Mess, Prefix );
    }
    else
    {
        *Mess = 0;
    }

    if ( CurrentGap )
    {
        swprintf( Mess+wcslen(Mess), L" Gap %I64d..%I64d [%I64d],",
            CurrentGap->StartLcn.QuadPart, 
            CurrentGap->StartLcn.QuadPart + CurrentGap->Length.QuadPart - 1,
            CurrentGap->Length.QuadPart );
    }
    else
    {
        wcscat( Mess, L" Gap NULL," );
    }

    if ( MarkerExtent )
    {
        swprintf( Mess+wcslen(Mess), L" Marker %I64d..%I64d [%I64d] ",
            MarkerExtent->StartLcn.QuadPart, 
            MarkerExtent->StartLcn.QuadPart + MarkerExtent->Length.QuadPart - 1,
            MarkerExtent->Length.QuadPart );
    }
    else
    {
        wcscat( Mess, L" Marker NULL ### " );
    }

    if ( Suffix )
    {
        wcscat( Mess, Suffix );
    }

} // end of PrintMarker

#endif // _DEBUG


void
FindGapBeforeMarker(
    IN      PDEFRAG_VOLUME_CONTEXT      VolumeCtx,
    IN  OUT PBTR_NODE                   *MarkerExtentNode,
    IN  OUT PBTR_NODE                   *CurrentGapNode )
{

    __try
    {
        if ( !(*MarkerExtentNode) )
        {
#ifdef _DEBUG
            OutMessage( L"      * FindGapBeforeMarker() invalid parameter" );
#endif
        }

        PDEFRAG_FILES_EXTENT    MarkerExtent = (PDEFRAG_FILES_EXTENT)((*MarkerExtentNode)->Body);

        if ( !(*CurrentGapNode) )
        {
            DEFRAG_LCN_EXTENT           GapTemplate = { 0 };

            GapTemplate.StartLcn = MarkerExtent->StartLcn;

            *CurrentGapNode = FindNode( &(VolumeCtx->GapsTree), &GapTemplate, SortModeLcn, _FIND_LE_ );
        }
        else
        {
            PDEFRAG_LCN_EXTENT  CurrentGap = (PDEFRAG_LCN_EXTENT)((*CurrentGapNode)->Body);

            if ( CurrentGap->StartLcn.QuadPart < MarkerExtent->StartLcn.QuadPart )
            {
#ifdef _DEBUG
                OutMessage( L"      * FindGapBeforeMarker() Gap exist" );
#endif
            }
            else
            {
                *CurrentGapNode = NULL;
            }
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        int Result = GetExceptionCode();
        DislpayErrorMessage( L"FindGapBeforeMarker(). Exception !!!", Result, ERR_OS );
    }

} // end of FindGapBeforeMarker


#define     SHOULD_DELETE_CURRENT_MARKER        0x00000001
#define     SHOULD_FIND_GAP_BEFORE_MARKER       0x00000002

void
FindNextMarker( 
    PDEFRAG_VOLUME_CONTEXT      VolumeCtx,
    PBTR_NODE                   *MarkerExtentNode, 
    PDEFRAG_FILES_EXTENT        *MarkerExtent, 
    PBTR_NODE                   *CurrentGapNode, 
    PDEFRAG_LCN_EXTENT          *CurrentGap, 
    DWORD                       Flags )
{
    PBTR_NODE                   WrkNode = *MarkerExtentNode;


    __try
    {
        if ( !(*MarkerExtentNode) )
        {
#ifdef _DEBUG
            OutMessage( L"      * FindNextMarker() invalid parameter" );
#endif
        }

        *MarkerExtent = (PDEFRAG_FILES_EXTENT)((*MarkerExtentNode)->Body);

        //
        // Find Next Marker
        //
        VolumeCtx->ExtentsTree.Current = *MarkerExtentNode;
        *MarkerExtentNode = FindNextNode( &(VolumeCtx->ExtentsTree), LEFT_TO_RIGHT );

        if ( (Flags & SHOULD_DELETE_CURRENT_MARKER) && !(*MarkerExtent)->Length.QuadPart )
        {
            DeleteNode( &(VolumeCtx->ExtentsTree), WrkNode );
        }

        if ( *MarkerExtentNode )
        {
            *MarkerExtent = (PDEFRAG_FILES_EXTENT)((*MarkerExtentNode)->Body);
            if ( (Flags & SHOULD_FIND_GAP_BEFORE_MARKER) )
            {
                FindGapBeforeMarker( VolumeCtx, MarkerExtentNode, CurrentGapNode );
                if ( *CurrentGapNode )
                {
                    *CurrentGap = (PDEFRAG_LCN_EXTENT)( (*CurrentGapNode)->Body);
                }
            }
        }
        else
        {
            *MarkerExtent = NULL;
        }

//#ifdef _DEBUG 
//        PrintMarker
//        OutMessage( Mess );
//#endif

    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        int Result = GetExceptionCode();
        DislpayErrorMessage( L"FindNextMarker(). Exception !!!", Result, ERR_OS );
    }

} // end of FindNextMarker

//
// Expand gap if nessesary to extent size or maximum possible
//
int
DfrgExpandGap( 
    IN      PDEFRAG_VOLUME_CONTEXT      VolumeCtx,
    IN  OUT PBTR_NODE                   *CurrentGapNode,
    IN      PBTR_NODE                   *MarkerExtentNode,
    IN      PBTR_NODE                   FileExtentNode,
    IN      LONGLONG                    RequiredGapSize )
{
    int                                 Result = NO_ERROR; //DFRG_ERROR_NOT_ENOUGH_SPACE ; //ERROR_NOT_ENOUGH_MEMORY;

    PDEFRAG_FILES_EXTENT                MarkerExtent = NULL;
    PDEFRAG_FILES_EXTENT                FileExtent = NULL;

    PDEFRAG_LCN_EXTENT                  CurrentGap = NULL;

    PDEFRAG_FILE_CONTEXT                MarkerFileCtx = NULL;

    PDEFRAG_FILES_EXTENT                NewExtent = NULL;

#ifdef _DEBUG
    WCHAR                               Mess[MAX_PATH];
#endif


    __try
    {
        if ( !MarkerExtentNode || !*MarkerExtentNode )
        {
            OutMessage( L"*** DfrgExpandGap(). Invalid Marker Parameter ***" );
            Result = ERROR_INVALID_PARAMETER;
            __leave;
        }

        FileExtent = (PDEFRAG_FILES_EXTENT)(FileExtentNode->Body);

        do
        {
            BOOLEAN ContinueWithExtent = TRUE;

            Result = NO_ERROR; 

            MarkerExtent = (PDEFRAG_FILES_EXTENT)((*MarkerExtentNode)->Body);

            MarkerFileCtx = MarkerExtent->StreamCtx->FileCtx;

            //
            // Find Gap before marker
            //
            if ( !*CurrentGapNode )
            {
                FindGapBeforeMarker( VolumeCtx, MarkerExtentNode, CurrentGapNode );
            }

            if ( *CurrentGapNode )
            {
                CurrentGap = (PDEFRAG_LCN_EXTENT)((*CurrentGapNode)->Body);
            }

#ifdef _DEBUG
            PrintMarker( Mess, L"      *** DfrgExpandGap() : ", CurrentGap, MarkerExtent, MarkerFileCtx->FName ); 
            OutMessage( Mess );
            swprintf( Mess, L"         * Required size %I64d", RequiredGapSize );
            OutMessage( Mess );
#endif

            if ( CurrentGap )
            {
                if ( CurrentGap->Length.QuadPart >= RequiredGapSize )
                {
                    //
                    // Return current work gap - it is fine
                    //
#ifdef _DEBUG
                    OutMessage( L"      *** DfrgExpandGap(). Gap length is enough. Return." );
#endif
                    __leave;
                }

                if ( CurrentGap->StartLcn.QuadPart + CurrentGap->Length.QuadPart < MarkerExtent->StartLcn.QuadPart  )
                {
                    //
                    // Return current work gap - it is fine
                    //
#ifdef _DEBUG
                    OutMessage( L"      *** DfrgExpandGap(). Gap do not tuch marker. Return." );
#endif
                    __leave;
                }
            }

            //
            // Check if we asked to free space for file's extent moving the previous file's extent 
            //
            if ( MarkerFileCtx == FileExtent->StreamCtx->FileCtx &&
                MarkerExtent->StartVcn.QuadPart < FileExtent->StartVcn.QuadPart )
            {
#ifdef _DEBUG
                swprintf( Mess, L"      * DfrgExpandGap(). Current file's extent was marker (is in place)" );
                OutMessage( Mess );
#endif
                //
                // --------------------------
                //    Find next marker 
                // --------------------------
                //
                FindNextMarker( 
                    VolumeCtx,
                    MarkerExtentNode, 
                    &MarkerExtent, 
                    CurrentGapNode, 
                    &CurrentGap, 
                    0 ); //SHOULD_FIND_GAP_BEFORE_MARKER ); // SHOULD_DELETE_CURRENT_MARKER | 

#ifdef _DEBUG
                PrintMarker( Mess, L"      DfrgExpandGap(). New  ", CurrentGap, MarkerExtent );
                OutMessage( Mess );
#endif
                //
                // Continue with next marker
                //
                continue;
            }

            //
            // Check if we asked to free space for file's extent moving the same extent
            //
            //if ( MarkerFileCtx == FileExtent->StreamCtx->FileCtx &&
            //    MarkerExtent->StartVcn.QuadPart == FileExtent->StartVcn.QuadPart )
            if ( FileExtentNode == *MarkerExtentNode )
            {
                PDEFRAG_FILES_EXTENT    WrkExtent = MarkerExtent;
#ifdef _DEBUG
                swprintf( Mess, L"      * DfrgExpandGap(). Current file's extent is marker" );
                OutMessage( Mess );
#endif
                //
                // --------------------------
                //    Find next marker 
                // --------------------------
                //
                FindNextMarker( 
                    VolumeCtx,
                    MarkerExtentNode, 
                    &MarkerExtent, 
                    CurrentGapNode, 
                    &CurrentGap, 
                    0 ); //SHOULD_FIND_GAP_BEFORE_MARKER ); // SHOULD_DELETE_CURRENT_MARKER | 

#ifdef _DEBUG
                PrintMarker( Mess, L"      DfrgExpandGap(). New  ", CurrentGap, MarkerExtent );
                OutMessage( Mess );
#endif

                if ( CurrentGap && CurrentGap->Length.QuadPart )
                {
                    //
                    // Return current work gap - it is fine
                    //
#ifdef _DEBUG
                    PrintGap( Mess, L"      *** DfrgExpandGap(). Return gap: ", CurrentGap, NULL );
                    OutMessage( Mess );
#endif
                    Result = NO_ERROR; 
                }
                else
                {
                    //
                    // Extent is in place - do not need to move
                    //
#ifdef _DEBUG
                    swprintf( Mess, L"      *** DfrgExpandGap(). Extent is in place" );
                    OutMessage( Mess );
#endif
                    //
                    // Mark progress.
                    //
                    MarkProgress( VolumeCtx, WrkExtent->Length.QuadPart );

                    Result = DFRG_ERROR_SKIP; //DFRG_ERROR_ABORTED_MESS; DFRG_ERROR_ALREADY_DONE;
                }
                __leave;
            }


            DEFRAG_FILES_EXTENT         ExtentToMove;
            LONGLONG                    ExtentLength = 0;

            memcpy( &ExtentToMove, MarkerExtent, sizeof(DEFRAG_FILES_EXTENT) );
            ExtentLength   = ExtentToMove.Length.QuadPart;

            //
            // Try to open Stream
            //
            BOOLEAN                     StreamIsUnmoveable = FALSE;

            StreamIsUnmoveable = CheckIsStreamUnmoveable( 
                                        VolumeCtx, 
                                        MarkerExtent->StreamCtx, 
                                        SHOULD_OPEN_STREAM );
            if ( StreamIsUnmoveable )
            {
#ifdef _DEBUG
                swprintf( Mess, L"      DfrgExpandGap(). Marker Stream (%ls) Is Unmoveable or already Moved (Processed)",
                    MarkerFileCtx->FName );
                OutMessage( Mess );
#endif
                if ( CurrentGap && CurrentGap->Length.QuadPart )
                {
                    //
                    // Return gap before unmoveable extent
                    //
#ifdef _DEBUG
                    PrintGap( Mess, L"      *** DfrgExpandGap(). Return gap: ", CurrentGap, NULL );
                    OutMessage( Mess );
#endif
                    __leave;
                }

                //
                // Continue with next marker
                //
                ContinueWithExtent = FALSE;
                //continue;
            }

            //
            // Move extent part by part
            //
            while( ContinueWithExtent && ExtentToMove.Length.QuadPart )
            {
                if ( !ShouldContinue( VolumeCtx->JobCtx ) )
                {
                    Result = DFRG_ERROR_ABORTED;
                    __leave;
                }

                //
                // Find gap for extent
                //
                PBTR_NODE           WrkGapNode = NULL;
                PDEFRAG_LCN_EXTENT  WrkGap = NULL;
                DEFRAG_LCN_EXTENT   SourceZone = { 0 };

                SourceZone.StartLcn.QuadPart = ExtentToMove.StartLcn.QuadPart + ExtentToMove.Length.QuadPart;
                SourceZone.Length.QuadPart = VolumeCtx->NtfsData.TotalClusters.QuadPart - SourceZone.StartLcn.QuadPart ;

                WrkGap = GetBestGapForExtent( 
                                &(VolumeCtx->GapsTree), 
                                &ExtentToMove,
                                &SourceZone,
                                SortModeSize,
                                RIGHT_TO_LEFT,
                                _FIND_GE_,
                                &WrkGapNode );

                if ( !WrkGap || !WrkGapNode || !WrkGap->Length.QuadPart )
                {
#ifdef _DEBUGW
                    OutMessage( L"      *** DfrgExpandGap(). Could not find gap  ***" );
#endif
                    Result = DFRG_ERROR_NOT_ENOUGH_SPACE;
                    __leave;
                }

                if ( WrkGapNode == *CurrentGapNode )
                {
#ifdef _DEBUG
                    OutMessage( L"      *** DfrgExpandGap(). Gap is the same as current ***" );
#endif
                    Result = DFRG_ERROR_NOT_ENOUGH_SPACE;
                    __leave;
                }
#if 0
                if ( ExtentToMove.StartLcn.QuadPart + ExtentToMove.Length.QuadPart == WrkGap->StartLcn.QuadPart )
                {
#ifdef _DEBUG
                    OutMessage( L"      *** DfrgExpandGap(). Slide extent ***" );
#endif
                    //
                    // *********************
                    //      Slide extent
                    // *********************
                    //
                    // Not use:
                    // - SHOULD_OPEN_STREAM as stream was opened while checking for moveability 
                    // - (SHOULD_ADJUST_USED_GAP | SHOULD_ADJUST_NEW_GAP | SHOULD_ADJUST_USED_EXTENT) as it is handled manualy
                    // Use:
                    // - SHOULD_ADJUST_CLUSTER_MAP
                    //
                    Result = DfrgSlideExtentToGap( 
                                    &ExtentToMove, 
                                    WrkGap, 
                                    SHOULD_ADJUST_CLUSTER_MAP );
                    __leave;
                }
#endif
                //
                // Shrink extent to gap's length
                //
                if ( ExtentToMove.Length.QuadPart > WrkGap->Length.QuadPart )
                {
                    ExtentToMove.Length = WrkGap->Length;
                }

                //
                // Prepare new extent pattern in the place of gap
                //
                if ( !NewExtent )
                {
                    NewExtent = (PDEFRAG_FILES_EXTENT) malloc( sizeof(DEFRAG_FILES_EXTENT) );
                    if ( !NewExtent )
                    {
                        Result = ERROR_NOT_ENOUGH_MEMORY;
                        __leave;
                    }
                }

                memcpy( NewExtent, &ExtentToMove, sizeof(DEFRAG_FILES_EXTENT) );
                NewExtent->StartLcn = WrkGap->StartLcn;

                //
                // *********************
                //      Slide extent
                // *********************
                //
                // Not use:
                // - SHOULD_OPEN_STREAM as stream was opened while checking for moveability 
                // - (SHOULD_ADJUST_USED_GAP | SHOULD_ADJUST_NEW_GAP | SHOULD_ADJUST_USED_EXTENT) as it is handled manualy
                // Use:
                // - SHOULD_ADJUST_CLUSTER_MAP
                //
                Result = DfrgMoveExtentToGap( 
                                &ExtentToMove, 
                                WrkGap, 
                                SHOULD_ADJUST_CLUSTER_MAP );

                DfrgUpdateGapNode( &(VolumeCtx->GapsTree), &WrkGapNode, &WrkGap );

                if ( NO_ERROR != Result )
                {
                    //
                    // Same as unmoveable
                    //
                    break;
                }

                //
                // Extent was moved. 
                //
#ifdef _DEBUG
                OutMessage( L"      DfrgExpandGap(). Extent was moved." );
#endif

                //
                // *******************************
                //   Adjust gaps & extents trees.
                // *******************************
                //
                // 1. Create new gap in the place of extent
                //
                PDEFRAG_LCN_EXTENT  NewGap = (PDEFRAG_LCN_EXTENT) malloc( sizeof(DEFRAG_LCN_EXTENT) );
                PBTR_NODE           NewGapNode = NULL;

                if ( NewGap )
                {
                    NewGap->StartLcn = ExtentToMove.StartLcn;
                    NewGap->Length   = ExtentToMove.Length;

                    DfrgInsertGapNode( &(VolumeCtx->GapsTree), &NewGapNode, &NewGap );
#ifdef _DEBUG
                    PrintGap( Mess, L"      DfrgExpandGap(). New Gap created as : ", NewGap, NULL );
                    OutMessage( Mess );
#endif
                    CurrentGap = NewGap;
                    *CurrentGapNode = NewGapNode;
                }

                //
                // 2. Insert new extent.
                //
                PBTR_NODE  NewExtentNode = AddNode( &(VolumeCtx->ExtentsTree), NewExtent );
                if ( !NewExtentNode )
                {
                    Result = ERROR_NOT_ENOUGH_MEMORY;
                    __leave;
                }
                //
                // Set NewExtent to NULL to allocate next as this one is in tree
                //
                NewExtent = NULL;

                //
                // Adjust extent to move.
                //
                ExtentToMove.StartVcn.QuadPart  += ExtentToMove.Length.QuadPart;
                ExtentToMove.StartLcn.QuadPart  += ExtentToMove.Length.QuadPart;
                ExtentLength                    -= ExtentToMove.Length.QuadPart;
                ExtentToMove.Length.QuadPart     = ExtentLength;

                memcpy( MarkerExtent, &ExtentToMove, sizeof(DEFRAG_FILES_EXTENT) );

                if ( !MarkerExtent->Length.QuadPart )
                {
                    ContinueWithExtent = FALSE;
                    break;
                }

            } // while( ExtentLength

            DfrgCloseFile( MarkerFileCtx );

            //
            // --------------------------
            //    Find next marker & delete current
            // --------------------------
            //
            FindNextMarker( 
                VolumeCtx,
                MarkerExtentNode, 
                &MarkerExtent, 
                CurrentGapNode, 
                &CurrentGap, 
                SHOULD_DELETE_CURRENT_MARKER | SHOULD_FIND_GAP_BEFORE_MARKER );
#ifdef _DEBUG
            PrintMarker( Mess, L"      DfrgExpandGap(). New  ", CurrentGap, MarkerExtent );
            OutMessage( Mess );
#endif
            if ( NO_ERROR != Result )
            {
                break;
            }
        } while ( *MarkerExtentNode );

    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        Result = GetExceptionCode();
        DislpayErrorMessage( L"### DfrgExpandGap(). Exception !!!", Result, ERR_OS );
    }


    DfrgCloseFile( MarkerFileCtx );

    if ( NewExtent )
    {
        free( NewExtent );
        NewExtent = NULL;
    }


    return Result;

} // end of DfrgExpandGap



int
DfrgMoveFileToMarker( 
    IN      PDEFRAG_VOLUME_CONTEXT      VolumeCtx,
    IN      PDEFRAG_FILE_CONTEXT        FileCtx,
    IN  OUT PBTR_NODE                   *MarkerExtentNode,
    IN  OUT PBTR_NODE                   *CurrentGapNode )
//    IN  OUT LONGLONG                    *CurrentCluster )
{
    int                         Result = NO_ERROR;

    PDEFRAG_JOB_CONTEXT         JobCtx = VolumeCtx->JobCtx;

#ifdef _DEBUG
    WCHAR                       Mess[MAX_PATH];
#endif

    PDEFRAG_FILES_EXTENT        NewExtent = NULL;

    BOOLEAN                     IsFileFragmented = FALSE;
    BOOLEAN                     WasFileFragmented = FALSE;
    BOOLEAN                     FileWasProcessed = FALSE;


    __try
    {
        if ( !MarkerExtentNode || !*MarkerExtentNode )
        {
            OutMessage( L"*** DfrgMoveFileToMarker(). Invalid Marker Parameter ***" );
            Result = ERROR_INVALID_PARAMETER;
            __leave;
        }

        PDEFRAG_STREAM_CONTEXT  StreamCtx = NULL;
        BOOLEAN                 StreamIsUnmoveable = FALSE;

        BOOLEAN                 FileIsUnmoveable = FALSE;
        LONGLONG                GapSize = 0;

        //
        // Test file movability
        //
        if ( FileCtx->DefaultStream )
        {
            FileIsUnmoveable = CheckIsStreamUnmoveable( 
                                        VolumeCtx, 
                                        FileCtx->DefaultStream, 
                                        0,
                                        TRUE );
            IsFileFragmented = IsStreamFragmented( FileCtx->DefaultStream );
            WasFileFragmented = IsFileFragmented;
        }

#ifdef _DEBUG
        swprintf( Mess, L"   DfrgMoveFileToMarker(). %ls '%ls' size %I64d flags %08X", 
            (FileCtx->DefragFlags & DEFRAG_FILE_DIRECTORY) ? L"Dir " : L"File", 
            FileCtx->FName ? FileCtx->FName : L"",
            FileCtx->TotalClustersNum.QuadPart, 
            FileCtx->DefragFlags );
#endif

        if ( FileIsUnmoveable )
        {
#ifdef _DEBUG
            wcscat( Mess, L" is UNMOVEABLE" );
            OutMessage( Mess );
#endif
            //
            // Mark progress
            //
            MarkProgress( VolumeCtx, FileCtx->TotalClustersNum.QuadPart );

            Result = DFRG_ERROR_SKIP;
            __leave;
        }

#ifdef _DEBUG
        OutMessage( Mess );
#endif

        PDEFRAG_FILES_EXTENT        MarkerExtent = NULL;
        PDEFRAG_LCN_EXTENT          CurrentGap = NULL;

        MarkerExtent = (PDEFRAG_FILES_EXTENT)( (*MarkerExtentNode)->Body);

        FindGapBeforeMarker( VolumeCtx, MarkerExtentNode, CurrentGapNode );
        if ( *CurrentGapNode )
        {
            CurrentGap = (PDEFRAG_LCN_EXTENT)( (*CurrentGapNode)->Body);
        }

#ifdef _DEBUG
        PrintMarker( Mess, L"   DfrgMoveFileToMarker() : ", CurrentGap, MarkerExtent );
        OutMessage( Mess );
#endif

        //
        // Move file - stream by stream, extent by extent.
        // Iterate file's streams.
        //
        for ( StreamCtx=FileCtx->FirstStream; StreamCtx; StreamCtx = StreamCtx->NextStream )
        {
            PRETRIEVAL_POINTERS_BUFFER  ClustersMap = StreamCtx->ClustersMap;

            if ( !ClustersMap || !ClustersMap->ExtentCount || !StreamCtx->DataSize.QuadPart )
            {
                continue;
            }

            //
            // Try to open Stream
            //
            BOOLEAN     StreamIsUnmoveable = CheckIsStreamUnmoveable( 
                                                VolumeCtx, 
                                                StreamCtx, 
                                                SHOULD_OPEN_STREAM,
                                                TRUE );
            if ( StreamIsUnmoveable )
            {
#ifdef _DEBUG
                swprintf( Mess, L"   DfrgMoveFileToMarker(). %ls '%ls':'%ls' %I64d is UNMOVEABLE", 
                    (FileCtx->DefragFlags & DEFRAG_FILE_DIRECTORY) ? L"Dir " : L"File", 
                    FileCtx->FName ? FileCtx->FName : L"", StreamCtx->StreamName ? StreamCtx->StreamName : L"",
                    StreamCtx->DataSize.QuadPart / VolumeCtx->NtfsData.BytesPerCluster );
                OutMessage( Mess );
#endif
                //
                // Mark progress
                //
                MarkProgress( VolumeCtx, StreamCtx->DataSize.QuadPart / VolumeCtx->NtfsData.BytesPerCluster );

                continue;
            }

            //
            // Update Display - file name 
            //
            if ( (DEFRAG_JOB_OPTION_SHOW_PROGRESS & JobCtx->Options) )
            {
                Result = DfrgGetFileName( JobCtx, FileCtx, &(JobCtx->CurrentFileName) );
                if ( NO_ERROR == Result )
                {
                    //
                    // TODO: Optimize
                    //
                    UpdateCurrentNameStatistic( JobCtx );
                }
            }

            //
            // Iterate stream's extents
            //
            DEFRAG_FILES_EXTENT         ExtentToMove; 

            ExtentToMove.StreamCtx = StreamCtx;

            BOOLEAN                     ContinueWithStream = TRUE;

            LARGE_INTEGER               StartingVcn = ClustersMap->StartingVcn;
            LARGE_INTEGER               NextVcnToProcess = ClustersMap->StartingVcn;

            do
            {
                if ( !ShouldContinue( VolumeCtx->JobCtx ) )
                {
                    Result = DFRG_ERROR_ABORTED;
                    __leave;
                }

                DWORD       i = (DWORD)(-1);
                LONGLONG    ExtentLength = 0;

                //
                // ClustersMap could be changed and reallocated by DfrgExpandGap()/DfrgSlideExtentToGap()/DfrgMoveExtentToGap().
                // Restore current position.
                //
                ClustersMap = StreamCtx->ClustersMap;
                if ( !ClustersMap )
                {
                    Result = ERROR_NOT_ENOUGH_MEMORY;
                    __leave;
                }

#ifdef _DEBUG
                OutMessage( L"" );
#endif

#ifdef _DEBUG
                swprintf( Mess, L"      Restoring ClustersMap position %I64d", NextVcnToProcess.QuadPart );
                OutMessage( Mess );
#endif

                StartingVcn = ClustersMap->StartingVcn;

                for ( DWORD j=0; j<ClustersMap->ExtentCount;
                    StartingVcn = ClustersMap->Extents[j].NextVcn, j++ )
                {
#ifdef _DEBUG
                    DEFRAG_FILES_EXTENT         FileExtent;

                    FileExtent.StreamCtx = StreamCtx;
                    ExtentLength = ClustersMap->Extents[j].NextVcn.QuadPart - StartingVcn.QuadPart;

                    FileExtent.StartVcn = StartingVcn;
                    FileExtent.StartLcn = ClustersMap->Extents[j].Lcn;
                    FileExtent.Length.QuadPart = ExtentLength;

                    PrintExtent( Mess, L"         Extent ", &FileExtent, NULL );
                    OutMessage( Mess );
#endif
                    if ( StartingVcn.QuadPart == NextVcnToProcess.QuadPart )
                    {
#ifdef _DEBUG
                        if ( 0 != j )
                        {
                            ExtentLength = ClustersMap->Extents[j].NextVcn.QuadPart - StartingVcn.QuadPart;
                            swprintf( Mess, L"   DfrgMoveFileToMarker(). Restore ClustersMap position: %I64d : %I64d..%I64d [%I64d]",
                                NextVcnToProcess.QuadPart, 
                                ClustersMap->Extents[j].Lcn.QuadPart, 
                                ClustersMap->Extents[j].Lcn.QuadPart + ExtentLength - 1,
                                ExtentLength );
                            OutMessage( Mess );
                        }
#endif
                        i = j;
                        break;
                    }
                }

                if ( (DWORD)(-1) == i )
                {
#ifdef _DEBUG
                    OutMessage( L"      * DfrgMoveFileToMarker(). Could not restore position or end" );
#endif
                    break;
                }

                if ( VIRTUAL_LCN == ClustersMap->Extents[i].Lcn.QuadPart ) 
                {
                    //NextVcnToProcess.QuadPart += ExtentLength;
                    NextVcnToProcess = ClustersMap->Extents[i].NextVcn;
                    continue;
                } 

                ExtentLength = ClustersMap->Extents[i].NextVcn.QuadPart - StartingVcn.QuadPart;

                ExtentToMove.StartVcn = StartingVcn;
                ExtentToMove.StartLcn = ClustersMap->Extents[i].Lcn;
                ExtentToMove.Length.QuadPart = ExtentLength;

                BOOLEAN     ContinueWithExtent = TRUE;

                //
                // Find this extent in tree
                //
                PDEFRAG_FILES_EXTENT        WrkExtent = NULL;
                PBTR_NODE                   WrkExtentNode = NULL;

                WrkExtentNode = FindNode( &(VolumeCtx->ExtentsTree), &ExtentToMove, SortModeLcn, _FIND_EQ_ );
                if ( !WrkExtentNode )
                {
#ifdef _DEBUG
                    PrintExtent( Mess, L"   DfrgMoveFileToMarker(). Could not find in tree extent ", &ExtentToMove, NULL );
                    OutMessage( Mess );
#endif
                    Result = DFRG_ERROR_FS_DATA_CORRUPTED;
                    __leave;
                }

                WrkExtent = (PDEFRAG_FILES_EXTENT)(WrkExtentNode->Body);

                //
                // Check if MarkerExtent == CurrentExtent - then Slide or skip
                //
                if ( WrkExtentNode == *MarkerExtentNode )
                {
#ifdef _DEBUG
                    PrintMarker( Mess, L"   DfrgMoveFileToMarker(). Current file's extent is marker", CurrentGap, WrkExtent );
                    OutMessage( Mess );
#endif
                    if ( CurrentGap && CurrentGap->Length.QuadPart )
                    {
                        if ( CurrentGap->StartLcn.QuadPart + CurrentGap->Length.QuadPart != 
                            WrkExtent->StartLcn.QuadPart )
                        {
                            //
                            // Some (unmoveable???) extent exist between gap and marker
                            //
                            goto MoveExtent;
                        }
#ifdef _DEBUG
                        OutMessage( L"      DfrgMoveFileToMarker(). Slide extent" );
#endif
                        //
                        // *********************
                        //      Slide extent
                        // *********************
                        //
                        // Not use:
                        // - (SHOULD_ADJUST_USED_GAP | SHOULD_ADJUST_NEW_GAP | SHOULD_ADJUST_USED_EXTENT) as it is handled manualy
                        // Use:
                        // - SHOULD_OPEN_STREAM as stream could be closed in DfrgExpandGap()
                        // - SHOULD_ADJUST_CLUSTER_MAP ???
                        //
                        Result = DfrgSlideExtentToGap( 
                                        &ExtentToMove, 
                                        CurrentGap, 
                                        SHOULD_OPEN_STREAM | SHOULD_ADJUST_CLUSTER_MAP );

                        //
                        // Adjust Gap
                        //
                        DfrgUpdateGapNode( &(VolumeCtx->GapsTree), CurrentGapNode, &CurrentGap );

                        if ( NO_ERROR != Result )
                        {
                            ContinueWithExtent = FALSE;
                            *CurrentGapNode = NULL;
                            CurrentGap = NULL;
                            break;
                        }

                        //
                        // Adjust Extent
                        //
                        WrkExtent->StartLcn = ExtentToMove.StartLcn;
                    }
                    else
                    {
                        //
                        // Skip this extent - it is in place.
                        // Mark progress.
                        //
                        MarkProgress( VolumeCtx, WrkExtent->Length.QuadPart );
                    }

                    NextVcnToProcess.QuadPart = ExtentToMove.StartVcn.QuadPart + ExtentToMove.Length.QuadPart;

                    //
                    // --------------------------
                    //    Find next marker 
                    // --------------------------
                    //
                    FindNextMarker( 
                        VolumeCtx,
                        MarkerExtentNode, 
                        &MarkerExtent, 
                        CurrentGapNode, 
                        &CurrentGap, 
                        SHOULD_FIND_GAP_BEFORE_MARKER ); // SHOULD_DELETE_CURRENT_MARKER | 
#ifdef _DEBUG
                    PrintMarker( Mess, L"   DfrgMoveFileToMarker(). New  ", CurrentGap, MarkerExtent );
                    OutMessage( Mess );
#endif
                    if ( !*MarkerExtentNode )
                    {
                        __leave;
                    }

                    continue;
                }
MoveExtent:
                //
                // Move extent part by part
                //
                while( ContinueWithExtent && ExtentToMove.Length.QuadPart )
                {
#ifdef _DEBUG
                    OutMessage( L"" );
                    PrintExtent( Mess, L"   DfrgMoveFileToMarker(). Extent to Move ", &ExtentToMove, NULL );
                    OutMessage( Mess );
#endif
                    if ( !*MarkerExtentNode )
                    {
#ifdef _DEBUG
                        OutMessage( L"   DfrgMoveFileToMarker(). MarkerExtentNode = NULL." );
#endif
                        __leave;
                    }

                    MarkerExtent = (PDEFRAG_FILES_EXTENT)( (*MarkerExtentNode)->Body);

#ifdef _DEBUG
                    PrintMarker( Mess, L"   DfrgMoveFileToMarker() Current  ", CurrentGap, MarkerExtent );
                    OutMessage( Mess );
#endif

                    //
                    // ************************
                    //  Prepare gap for extent
                    // ************************
                    //
                    Result = DfrgExpandGap( 
                                    VolumeCtx, 
                                    CurrentGapNode, 
                                    MarkerExtentNode, 
                                    WrkExtentNode, 
                                    ExtentLength );

                    if ( NO_ERROR != Result || !(*CurrentGapNode) )
                    {
                        if ( DFRG_ERROR_SKIP == Result )
                        {
                            //
                            // Continue with next extent
                            //
#ifdef _DEBUG
                            OutMessage( L"   *** DfrgMoveFileToMarker(). Extent in place. Continue with next ***" );
#endif
                            NextVcnToProcess.QuadPart = ExtentToMove.StartVcn.QuadPart + ExtentToMove.Length.QuadPart;

                            break; //continue;
                        }

                        if ( !(*CurrentGapNode) )
                        {
#ifdef _DEBUG
                            OutMessage( L"   *** DfrgMoveFileToMarker(). No gap returned from DfrgExpandGap() ***" );
#endif
                            __leave;
                        }
                    }

                    CurrentGap = (PDEFRAG_LCN_EXTENT)((*CurrentGapNode)->Body);

                    if ( !CurrentGap->Length.QuadPart )
                    {
#ifdef _DEBUG
                        OutMessage( L"   *** DfrgMoveFileToMarker(). Zero gap returned form DfrgExpandGap() ***" );
#endif
                        __leave;
                    }

                    //if ( WrkExtentNode == *MarkerExtentNode )
                    //{
                        //
                        // *********************
                        //      Slide extent
                        // *********************
                        //

                    //
                    // Shrink extent to gap's length
                    //
                    if ( ExtentToMove.Length.QuadPart > CurrentGap->Length.QuadPart )
                    {
                        ExtentToMove.Length = CurrentGap->Length;
                    }

                    //
                    // Prepare new exten pattern in the place of gap
                    //
                    if ( !NewExtent )
                    {
                        NewExtent = (PDEFRAG_FILES_EXTENT) malloc( sizeof(DEFRAG_FILES_EXTENT) );
                        if ( !NewExtent )
                        {
                            Result = ERROR_NOT_ENOUGH_MEMORY;
                            break;
                        }
                    }

                    memcpy( NewExtent, &ExtentToMove, sizeof(DEFRAG_FILES_EXTENT) );
                    NewExtent->StartLcn = CurrentGap->StartLcn;

                    //
                    // *********************
                    //      Move extent
                    // *********************
                    //
                    // Not use:
                    // - (SHOULD_ADJUST_USED_GAP | SHOULD_ADJUST_NEW_GAP | SHOULD_ADJUST_USED_EXTENT) as it is handled manualy
                    // Use:
                    // - SHOULD_OPEN_STREAM as stream could be closed in DfrgExpandGap()
                    // - SHOULD_ADJUST_CLUSTER_MAP ???
                    //
                    Result = DfrgMoveExtentToGap( 
                                    &ExtentToMove, 
                                    CurrentGap, 
                                    SHOULD_OPEN_STREAM | SHOULD_ADJUST_CLUSTER_MAP );

                    //
                    // Update used gap
                    //
                    DfrgUpdateGapNode( &(VolumeCtx->GapsTree), CurrentGapNode, &CurrentGap );

                    if ( NO_ERROR != Result )
                    {
                        ContinueWithExtent = FALSE;
                        ContinueWithStream = FALSE;
                        break;
                    }

                    //
                    // Extent was moved. 
                    //
#ifdef _DEBUG
                    OutMessage( L"      DfrgMoveFileToMarker(). Extent was moved." );
#endif

                    // 
                    // *******************************
                    //   Adjust gaps & extents trees.
                    // *******************************
                    //
                    // 1. Create new gap in the place of extent
                    //
                    PDEFRAG_LCN_EXTENT  NewGap = (PDEFRAG_LCN_EXTENT) malloc( sizeof(DEFRAG_LCN_EXTENT) );
                    PBTR_NODE           NewGapNode = NULL;

                    if ( NewGap )
                    {
                        NewGap->StartLcn = ExtentToMove.StartLcn;
                        NewGap->Length   = ExtentToMove.Length;

                        BOOLEAN     ShouldChangeCurrentGap = FALSE;

                        if ( CurrentGapNode && CurrentGap &&
                            GetExtentsIntersection( NewGap, CurrentGap, NULL ) >= 0 )
                        {
                            ShouldChangeCurrentGap = TRUE;
                        }

                        DfrgInsertGapNode( &(VolumeCtx->GapsTree), &NewGapNode, &NewGap );
#ifdef _DEBUG
                        PrintGap( Mess, L"   DfrgMoveFileToMarker(). New Gap created in tree as : ", NewGap, NULL ); 
                        OutMessage( Mess );
#endif
                        if ( ShouldChangeCurrentGap )
                        {
                            CurrentGap = NewGap;
                            *CurrentGapNode = NewGapNode;
                        }
                    }

                    //
                    // 2. Insert new extent in the place of gap.
                    //
                    PBTR_NODE  NewExtentNode = AddNode( &(VolumeCtx->ExtentsTree), NewExtent );
                    if ( !NewExtentNode )
                    {
                        Result = ERROR_NOT_ENOUGH_MEMORY;
                        __leave;
                    }
                    //
                    // Set NewExtent to NULL to allocate next as this one is in tree
                    //
                    NewExtent = NULL;

                    //
                    // 3. Adjust extent to move.
                    //
                    NextVcnToProcess.QuadPart = ExtentToMove.StartVcn.QuadPart + ExtentToMove.Length.QuadPart;

                    ExtentToMove.StartVcn.QuadPart  += ExtentToMove.Length.QuadPart;
                    ExtentToMove.StartLcn.QuadPart  += ExtentToMove.Length.QuadPart;
                    ExtentLength                    -= ExtentToMove.Length.QuadPart;
                    ExtentToMove.Length.QuadPart     = ExtentLength;

                    //
                    // 4. Adjust/Delete moved extent.
                    //
                    if ( !ExtentToMove.Length.QuadPart )
                    {
                        //
                        // Check if new marker point to the extent currently moved
                        //
                        if ( WrkExtentNode == *MarkerExtentNode )
                        {
                            //
                            // --------------------------------------
                            //    Find next marker & delete current
                            // --------------------------------------
                            //
                            FindNextMarker( 
                                VolumeCtx,
                                MarkerExtentNode, 
                                &MarkerExtent, 
                                CurrentGapNode, 
                                &CurrentGap, 
                                0 ); // SHOULD_FIND_GAP_BEFORE_MARKER ); // SHOULD_DELETE_CURRENT_MARKER | 

                            FindGapBeforeMarker( VolumeCtx, MarkerExtentNode, CurrentGapNode );
                            if ( *CurrentGapNode )
                            {
                                CurrentGap = (PDEFRAG_LCN_EXTENT)( (*CurrentGapNode)->Body);
                            }
#ifdef _DEBUG
                            //PrintMarker( Mess, L"   DfrgMoveFileToMarker(). Marker was moved & deleted. New ", CurrentGap, MarkerExtent, NULL ); 
                            PrintMarker( Mess, L"   DfrgMoveFileToMarker(). Marker was moved to:", CurrentGap, MarkerExtent, NULL ); 
                            OutMessage( Mess );
#endif
                        }
                        //else
                        {
                            DeleteNode( &(VolumeCtx->ExtentsTree), WrkExtentNode );
#ifdef _DEBUG
                            OutMessage( L"   DfrgMoveFileToMarker(). Extent node deleted" );
#endif
                        }

                    }
                    else
                    {
                        memcpy( WrkExtent, &ExtentToMove, sizeof(DEFRAG_FILES_EXTENT) );
                        if ( WrkExtentNode == *MarkerExtentNode && MarkerExtent )
                        {
                            memcpy( MarkerExtent, WrkExtent, sizeof(DEFRAG_FILES_EXTENT) );
#ifdef _DEBUG
                            PrintMarker( Mess, L"   DfrgMoveFileToMarker(). Marker was moved. ", CurrentGap, MarkerExtent, NULL ); 
                            OutMessage( Mess );
#endif
                        }
                    }

                } // while( ExtentLength

            } while ( ContinueWithStream );// for( Extent

            //
            // AdjustClusterMap.
            //

            //
            // Adjust stream display.
            // Mark fragmented status on display map.
            //
            if ( (DEFRAG_JOB_OPTION_SHOW_PROGRESS & VolumeCtx->JobCtx->Options) &&
                StreamCtx == FileCtx->DefaultStream )
            {
                BOOLEAN IsFileFragmentedNow = IsStreamFragmented( StreamCtx );

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


        } // for( StreamCtx=

        DfrgCloseFile( FileCtx );

        FileWasProcessed = TRUE;

    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        Result = GetExceptionCode();
        DislpayErrorMessage( L"DfrgMoveFileToMarker(). Exception !!!", Result, ERR_OS );
    }


    DfrgCloseFile( FileCtx );

    if ( NewExtent )
    {
        free( NewExtent );
        NewExtent = NULL;
    }


    if ( FileWasProcessed )
    {
        //
        // TODO.
        //
        FileCtx->DefragFlags |= DEFRAG_FILE_PROCESSED;

        if ( (DEFRAG_JOB_OPTION_SHOW_PROGRESS & VolumeCtx->JobCtx->Options) )
        {
            if ( WasFileFragmented != IsFileFragmented )
            {
                PDEFRAG_VOLUME_STATISTIC    Statistic = &(VolumeCtx->Statistic);

                if ( WasFileFragmented )
                {
                    //
                    // File was defragmented.
                    // Update Statistic.
                    //
                    Statistic->DefragmentedFilesNum.QuadPart++;

                    if ( Statistic->FragmentedFilesNum.QuadPart )
                    {
                        DEFRAG_RATIO_TYPE       PrevFileFragmentation = 0.0;

                        if ( FileCtx->TotalClustersNum.QuadPart )
                        {
                            PrevFileFragmentation = 
                                (FileCtx->TotalFragmentsNum.QuadPart * 100.F) / FileCtx->TotalClustersNum.QuadPart;
                        }

                        Statistic->FragmentedFilesFragmentationRatio -= 
                            (PrevFileFragmentation - Statistic->FragmentedFilesFragmentationRatio) / 
                            Statistic->FragmentedFilesNum.QuadPart;

                        Statistic->FragmentedFilesNum.QuadPart--;
                        Statistic->FragmentedFilesSize.QuadPart -= 
                            FileCtx->TotalClustersNum.QuadPart * VolumeCtx->NtfsData.BytesPerCluster;
                        Statistic->FragmentedFilesExcessNum.QuadPart -= 
                            FileCtx->TotalFragmentsNum.QuadPart;
                    }
                }
                else
                {
                    //
                    // File was fragmented.
                    // Update Statistic->
                    //
                    //Statistic->DefragmentedFilesNum.QuadPart--;

                    Statistic->FragmentedFilesNum.QuadPart++;
                    Statistic->FragmentedFilesSize.QuadPart += 
                        FileCtx->TotalClustersNum.QuadPart * VolumeCtx->NtfsData.BytesPerCluster;
                    Statistic->FragmentedFilesExcessNum.QuadPart += 
                        FileCtx->TotalFragmentsNum.QuadPart;

                    DEFRAG_RATIO_TYPE   CurrentFileFragmentation = DfrgCulcFileFragmentation( FileCtx );

                    Statistic->FragmentedFilesFragmentationRatio += 
                        (CurrentFileFragmentation - Statistic->FragmentedFilesFragmentationRatio) / 
                            Statistic->FragmentedFilesNum.QuadPart;
                }
            }
        }
    }


    return Result;

} // end of DfrgMoveFileToMarker


//
// Files and directories are to be arranged in a specified manner.
//
int
DfrgSuperSmart(
    IN  PDEFRAG_VOLUME_CONTEXT  VolumeCtx )
{
    int                         Result = NO_ERROR;

    PDEFRAG_JOB_CONTEXT         JobCtx = VolumeCtx->JobCtx;

    PDEFRAG_FILE_CONTEXT        FileCtx = NULL;
    PBTR_NODE                   FileNode = NULL;

    PBTR_NODE                   MarkerExtentNode = NULL;
    PDEFRAG_FILES_EXTENT        MarkerExtent = NULL;

    PBTR_NODE                   CurrentGapNode = NULL;
    PDEFRAG_LCN_EXTENT          CurrentGap = NULL;


#ifdef _DEBUG
    WCHAR   Mess[MAX_PATH];
#endif


    __try
    {
        //
        // Mark progress
        //
        JobCtx->ProcessedObjectsNum.QuadPart = 0;
        JobCtx->ObjectsToProcessNum.QuadPart = 
            (VolumeCtx->NtfsData.TotalClusters.QuadPart - VolumeCtx->NtfsData.FreeClusters.QuadPart) * 2;

        //
        // Set Mft zone
        //
        PDEFRAG_ZONE                MftZone = NULL;

        if ( NtfsDisk == VolumeCtx->DiskType )
        {
            DfrgBuildSimpleZones( VolumeCtx, &MftZone );
        }

        //
        // Step 1. Rebuild file list as nessessary.
        //
        if ( JobCtx->SortMode != VolumeCtx->FilesTree.SortMode )
        {
            BTR_TREE                    FilesTree = { 0 };
            BTR_TREE_METHODS            Methods = { 0 };

            //
            // Init new files tree
            //
            Methods.FreeBody        = FreeFileCtx;
            Methods.CompareItems    = CompareFileCtx;
            Methods.CopyBody        = CopyFileCtx;
            Methods.PrintBody       = PrintFileCtx;

            InitTree( 
                &FilesTree, 
                &Methods, 
                JobCtx->SortMode );

            //
            // Build new files tree from old
            //
            FileNode = FindFirstNode( &(VolumeCtx->FilesTree), LEFT_TO_RIGHT );

            while( FileNode )
            {
                FileCtx = (PDEFRAG_FILE_CONTEXT)(FileNode->Body);
                if ( FileCtx )
                {
                    AddNode( &FilesTree, FileCtx );
                }
                FileNode = FindNextNode( &(VolumeCtx->FilesTree), LEFT_TO_RIGHT );
            }

            //
            // Release old files tree
            //
            ReleaseTree( &(VolumeCtx->FilesTree) );

            //
            // Copy new files tree
            //
            memcpy( &(VolumeCtx->FilesTree), &FilesTree, sizeof( BTR_TREE ) );

#ifdef _DEBUG
            OutMessage( L"   Rebuilded FILE's list >" );
            PrintFileList( VolumeCtx, &(VolumeCtx->FilesTree), FALSE );
#endif
        }

        //
        // Step 2. Rebuild gaps list as nessessary.
        //
        if ( SortModeLcn != VolumeCtx->GapsTree.SortMode )
        {
#ifdef _DEBUGW
            OutMessage( L"   Rebuild GAP's list >" );
            PrintGapsTree( &(VolumeCtx->GapsTree), TRUE );
#endif

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
            PDEFRAG_LCN_EXTENT          Gap = NULL;
            PBTR_NODE                   GapNode = NULL;
            DEFRAG_LCN_EXTENT           ZoneIntersection;
            LONGLONG                    ZoneIntersectionLength;


            GapNode = FindFirstNode( &(VolumeCtx->GapsTree), LEFT_TO_RIGHT );
            if ( !GapNode )
            {
                __leave;
            }

            do
            {
                Gap = (PDEFRAG_LCN_EXTENT)(GapNode->Body);
                if ( !Gap )
                {
                    continue;
                }

                BOOLEAN     UseThisGap = TRUE;

                if ( MftZone )
                {
                    ZoneIntersectionLength = GetExtentsIntersection( &(MftZone->Extents[0]), Gap, &ZoneIntersection );

                    if ( ZoneIntersectionLength > 0 )
                    {

#ifdef _DEBUGW
                        swprintf( Mess, L"    Gap %I64d [%I64d] Intersection with MftZone [%I64d]",
                            Gap->StartLcn.QuadPart,
                            Gap->Length.QuadPart,
                            ZoneIntersectionLength );
                        OutMessage( Mess );
#endif
                        if ( ZoneIntersectionLength == Gap->Length.QuadPart )
                        {
                            UseThisGap = FALSE;
                        }
                        else
                        {
                            if ( ZoneIntersection.StartLcn.QuadPart == Gap->StartLcn.QuadPart )
                            {
                                Gap->StartLcn.QuadPart += ZoneIntersectionLength;
                            }
                            Gap->Length.QuadPart -= ZoneIntersectionLength;
                        }
                    }
                }

                if ( UseThisGap )
                {
                    AddNode( &GapsTree, Gap );
                }

            } while( GapNode = FindNextNode( &(VolumeCtx->GapsTree), LEFT_TO_RIGHT ) );

            //
            // Release old tree
            //
            ReleaseTree( &(VolumeCtx->GapsTree) );

            //
            // Copy new tree
            //
            memcpy( &(VolumeCtx->GapsTree), &GapsTree, sizeof( BTR_TREE ) );

#ifdef _DEBUG
            OutMessage( L"   Rebuilded GAP's list >" );
            PrintGapsTree( &(VolumeCtx->GapsTree), TRUE ); 
#endif
        }

        //
        // Step 3. Build extents list.
        //
        //LONGLONG    CurrentFileCluster = 0;
        //LONGLONG    CurrentDirCluster = 0;

        //
        // Iterate through file list.
        //
#ifdef _DEBUG
        OutMessage( L"" );
        swprintf( Mess, L"DfrgSuperSmart. Total %I64d  Free %I64d  Reserved %I64d",
                        VolumeCtx->NtfsData.TotalClusters,
                        VolumeCtx->NtfsData.FreeClusters,
                        VolumeCtx->NtfsData.TotalReserved );
        OutMessage( Mess );
        if ( MftZone )
        {
            swprintf( Mess, L"MftZone %I64d [%I64d] .. %I64d",
                            MftZone->Extents[0].StartLcn, MftZone->Extents[0].Length.QuadPart,
                            MftZone->Extents[0].StartLcn.QuadPart + MftZone->Extents[0].Length.QuadPart );
            OutMessage( Mess );
        }


        if ( Fat12Disk == VolumeCtx->DiskType )
        {
            PrintExtentList( VolumeCtx, 0, VolumeCtx->NtfsData.TotalClusters.QuadPart, TRUE );
        }

        OutMessage( L"Iterate files\n" );
        GetInput();
#endif

        BOOLEAN     FilesTreeDirection = LEFT_TO_RIGHT;

        if ( DEFRAG_JOB_SMART_BY_ACCESS == JobCtx->SortMode ||
            DEFRAG_JOB_SMART_BY_MODIFY == JobCtx->SortMode ||
            DEFRAG_JOB_SMART_BY_CREATE == JobCtx->SortMode )
        {
            FilesTreeDirection = RIGHT_TO_LEFT;
        }

        //
        // Find first extent 
        //
        MarkerExtentNode = FindFirstNode( &(VolumeCtx->ExtentsTree), LEFT_TO_RIGHT );
        if ( !MarkerExtentNode )
        {
            __leave;
        }

        MarkerExtent = (PDEFRAG_FILES_EXTENT)( MarkerExtentNode->Body);

        //
        // Find leading gap if any
        //
        if ( MarkerExtent->StartLcn.QuadPart > 0 )
        {
            CurrentGapNode = FindFirstNode( &(VolumeCtx->GapsTree), LEFT_TO_RIGHT );
            if ( !CurrentGapNode )
            {
                __leave;
            }
        }


        //
        // Step 3.1. Get file
        //
        if ( !(FileCtx = GetNextMoveableFile( &(VolumeCtx->FilesTree), &FileNode, FilesTreeDirection, FIND_FIRST_ITEM)) )
        {
            __leave;
        }

        do
        {
            if ( !ShouldContinue( VolumeCtx->JobCtx ) )
            {
                Result = DFRG_ERROR_ABORTED;
                __leave;
            }

            //if ( !ShouldMoveFile( CurrentExtent, FileCtx ) )
            if ( (FileCtx->DefragFlags & DEFRAG_FILE_PROCESSED) )
            {
                continue;
            }

#ifdef _DEBUG
            OutMessage( L"" );
            swprintf( Mess, L"*** File  '%ls' [%I64d] ***\n",
                FileCtx->FName ? FileCtx->FName : L"", FileCtx->TotalClustersNum.QuadPart );
            OutMessage( Mess );
#endif

            if ( !(FileCtx->DefragFlags & DEFRAG_FILE_DIRECTORY) )
            {
                Result = DfrgMoveFileToMarker( 
                                VolumeCtx, 
                                FileCtx, 
                                &MarkerExtentNode, 
                                &CurrentGapNode );
            }

#ifdef _DEBUG
            if ( Fat12Disk == VolumeCtx->DiskType )
            {
                PrintExtentList( VolumeCtx, 0, VolumeCtx->NtfsData.TotalClusters.QuadPart, TRUE );
            }
#endif
        } while(  FileCtx = GetNextMoveableFile( &(VolumeCtx->FilesTree), &FileNode, FilesTreeDirection, FIND_NEXT_ITEM) );

    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        Result = GetExceptionCode();
        DislpayErrorMessage( L"DfrgSuperSmart(). Exception !!!", Result, ERR_OS );
    }

/*
    if ( !GapNode )
    {
#ifdef _DEBUG
         OutMessage( L"Gaps pool end" );
#endif
         Result = NO_ERROR;
    }
*/
    if ( !CurrentGapNode )
    {
#ifdef _DEBUG
         OutMessage( L"Files pool end" );
#endif
         Result = NO_ERROR;
    }

    if ( !MarkerExtentNode )
    {
#ifdef _DEBUG
         OutMessage( L"Extents pool end" );
#endif
         Result = NO_ERROR;
    }


#ifdef _DEBUG
    if ( Fat12Disk == VolumeCtx->DiskType )
    {
        PrintExtentList( VolumeCtx, 0, VolumeCtx->NtfsData.TotalClusters.QuadPart, TRUE );
    }
#endif


    return Result;

} // end of DfrgSuperSmart
