/*
    Defrag Engine

    Module name:

        ContextUtils.cpp

    Abstract:

        Defrag Engine context structures operation utilities module. 
        Contains context structures operation utilities .

    $Header: /home/CVS_DEFRAG/Defrag/Src/Engine/FileExContext.cpp,v 1.10 2009/12/29 09:50:16 dimas Exp $
    $Log: FileExContext.cpp,v $
    Revision 1.10  2009/12/29 09:50:16  dimas
    Bug with sparsed files fragmentation detection fixed

    Revision 1.9  2009/12/25 13:48:53  dimas
    Debug artefacts removed

    Revision 1.8  2009/12/24 10:52:20  dimas
    Check against Exclude Files list implemented

    Revision 1.7  2009/12/23 13:01:47  dimas
    1. Full File Name processing implemented.
    2. Top 100 Fragmented functionality for all file list requests implemented.

    Revision 1.6  2009/12/21 17:00:13  dimas
    1. DEFRAG_CMD_GET_MOST_FRAGMENTED and similar requests implemented
    2. Basic Exclude files check added

    Revision 1.5  2009/12/16 14:13:34  dimas
    DEFRAG_CMD_GET_FILE_INFO request implemented

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



void
PrintFileCtx( 
    IN  PVOID       InFileBody  )
{
#ifdef _DEBUG
    PDEFRAG_FILE_CONTEXT    FileCtx     = (PDEFRAG_FILE_CONTEXT)InFileBody;
    PDEFRAG_STREAM_CONTEXT  StreamCtx   = FileCtx->FirstStream;
    WCHAR                   Mess[MAX_PATH];
    int                     StreamCnt = 0;


    swprintf( Mess, L"%ls %016I64X '%ls' flags %08X, total clusters %I64d", 
        (FileCtx->DefragFlags & DEFRAG_FILE_DIRECTORY) ? L"Dir " : L"File", 
        FileCtx->FId.FileId,
        FileCtx->FName ? FileCtx->FName : L"",
        FileCtx->DefragFlags,
        FileCtx->TotalClustersNum.QuadPart );
    OutMessage( Mess );

    while ( StreamCtx )
    {
        swprintf( Mess, L"   %ls '%ls' length %I64d", 
            GetStreamTypeName( StreamCtx->MftStreamType ), 
            StreamCtx->StreamName ? StreamCtx->StreamName : L"",
            StreamCtx->DataSize.QuadPart );
        OutMessage( Mess );

        if ( StreamCtx == FileCtx->DefaultStream && 
            FileCtx->VolumeCtx->DiskType != NtfsDisk )
        {
            PrintClustersMap( StreamCtx->ClustersMap );
            //GetInput();
        }

        StreamCtx = StreamCtx->NextStream;
        StreamCnt++;
    }
#endif // _DEBUG
}

#ifdef _DEBUG

void
PrintFileList( 
    IN  PDEFRAG_VOLUME_CONTEXT  VolumeCtx,
    IN  PBTR_TREE               Tree,
    IN  BOOLEAN                 PrintBody,
    IN  ULONG                   Flags )
{
    int                         Result = NO_ERROR;

    PDEFRAG_FILE_CONTEXT        FileCtx = NULL;
    PBTR_NODE                   FileNode;
    LARGE_INTEGER               Cnt = { 0 }, DirCnt = { 0 };
    WCHAR                       Mess[MAX_PATH*2];


    OutMessage( L"" );
    //GetInput();

    //
    // Iterate through file list and process files
    //
    FileNode = FindFirstNode( Tree, LEFT_TO_RIGHT );
    while ( FileNode )
    {
        Cnt.QuadPart++;
        if ( Cnt.QuadPart > Tree->NodeCnt.QuadPart )
        {
            break;
        }

        if ( (((PDEFRAG_FILE_CONTEXT)FileNode->Body)->DefragFlags & DEFRAG_FILE_DIRECTORY) )
        {
            DirCnt.QuadPart++;
        }

        if ( PrintBody )
        {
            PDEFRAG_FILE_CONTEXT    FileCtx = (PDEFRAG_FILE_CONTEXT)FileNode->Body;
            if ( !Flags || (Flags & FileCtx->DefragFlags ) )
            {
                PrintFileCtx( FileCtx );
                //Tree->Methods.PrintBody( FileNode->Body );
            }
        }

        FileNode = FindNextNode( Tree, LEFT_TO_RIGHT );
    }

    swprintf( Mess, L"Print_left %I64d node from total %I64d.\n"
                    L"Files num = %I64d (from %I64d), Dirs num = %I64d (from %I64d)", 
                    Cnt.QuadPart,    Tree->NodeCnt.QuadPart, 
                    Cnt.QuadPart,    VolumeCtx->Statistic.FilesNum.QuadPart, 
                    DirCnt.QuadPart, VolumeCtx->Statistic.DirsNum.QuadPart );
    OutMessage( Mess );

    OutMessage( L"" );

    if ( PrintBody )
    {
        GetInput();
    }

} // end of PrintFileList


void
PrintExtentList( 
    IN  PDEFRAG_VOLUME_CONTEXT  VolumeCtx,
    LONGLONG                    StartLcn,
    LONGLONG                    EndLcn,
    BOOLEAN                     PrintFileBody )
{
    int                         Result = NO_ERROR;

    PDEFRAG_FILES_EXTENT        Extent = NULL;
    PDEFRAG_FILE_CONTEXT        FileCtx = NULL;
    PBTR_NODE                   ExtentNode;

    LONGLONG                    Cnt = { 0 };
    WCHAR                       Mess[MAX_PATH*2];


    OutMessage( L"" );
    swprintf( Mess, L"Print_left extents %I64d .. %I64d.\n",
                StartLcn, EndLcn );
    OutMessage( Mess );

    //GetInput();

    //
    // Iterate through file list and process files
    //
    if ( !StartLcn )
    {
        ExtentNode = FindFirstNode( &(VolumeCtx->ExtentsTree), LEFT_TO_RIGHT );
    }
    else
    {
        DEFRAG_FILES_EXTENT        ExtentTemplate = { 0 };
        ExtentTemplate.StartLcn.QuadPart = StartLcn;
        ExtentNode = FindNode( &(VolumeCtx->ExtentsTree), &ExtentTemplate, SortModeLcn, _FIND_LE_ );
    }


    while ( ExtentNode )
    {
        Cnt++;

        Extent = (PDEFRAG_FILES_EXTENT)( ExtentNode->Body );

        if (  Extent->StartLcn.QuadPart > EndLcn )
        {
            break;
        }

        swprintf( Mess, L"   Extent %I64d..%I64d [%I64d], VCN %I64d ", 
            Extent->StartLcn.QuadPart, 
            Extent->StartLcn.QuadPart + Extent->Length.QuadPart - 1,
            Extent->Length.QuadPart,
            Extent->StartVcn.QuadPart );
        OutMessage( Mess );

        FileCtx = (PDEFRAG_FILE_CONTEXT)Extent->StreamCtx->FileCtx;

        if ( PrintFileBody )
        {
            swprintf( Mess, L"      %ls '%ls' length %I64d dates %I64d %I64d %I64d, ID: %016I64X ", 
                (FileCtx->DefragFlags & DEFRAG_FILE_DIRECTORY) ? L"Dir " : L"File", 
                FileCtx->FName ? FileCtx->FName : L"",
                FileCtx->TotalClustersNum.QuadPart,
                FileCtx->CreationTime.QuadPart,
                FileCtx->ChangeTime.QuadPart,
                FileCtx->LastAccessTime.QuadPart,
                FileCtx->FId.FileId );
            OutMessage( Mess );
        }

        ExtentNode = FindNextNode( &(VolumeCtx->ExtentsTree), LEFT_TO_RIGHT );
    }

    OutMessage( L"" );
    GetInput();

} // end of PrintExtentList

#endif // _DEBUG


void
ReleaseFileCtx( 
    IN  PDEFRAG_FILE_CONTEXT    FileCtx )
{

    if ( !FileCtx )
    {
        return;
    }

    PDEFRAG_STREAM_CONTEXT  StreamCtx   = FileCtx->FirstStream;
    PDEFRAG_STREAM_CONTEXT  NextStreamCtx;

#ifdef _DEBUG_FCTX
    WCHAR   Mess[MAX_PATH];

    *Mess = L'\0';
    swprintf( Mess+wcslen(Mess), L"   FreeFileCtx %p", FileCtx );
#endif

    DfrgCloseFile( FileCtx );

    if ( FileCtx->FName )
    {
#ifdef _DEBUG_FCTX
        swprintf( Mess+wcslen(Mess), L" : %ls", FileCtx->FName );
#endif
        free( FileCtx->FName );
        FileCtx->FName = NULL;
    }

    while ( StreamCtx )
    {
        NextStreamCtx = StreamCtx->NextStream;
        FreeStreamCtx( StreamCtx );
        StreamCtx = NextStreamCtx;
    }

} // end of ReleaseFileCtx


void
FreeFileCtx( 
    IN  PVOID       InFileCtx )
{

    if ( !InFileCtx )
    {
        return;
    }

    PDEFRAG_FILE_CONTEXT    FileCtx = (PDEFRAG_FILE_CONTEXT)InFileCtx;

    ReleaseFileCtx( FileCtx );

    free( FileCtx );

#ifdef _DEBUG_FCTX
    OutMessage( Mess );
#endif

} // end of FreeFileCtx


void
CopyFileCtx( 
    IN  PVOID       TrgFileCtx,
    IN  PVOID       SrcFileCtx )
{

    if ( TrgFileCtx && SrcFileCtx )
    {
        memcpy( TrgFileCtx, SrcFileCtx, sizeof(DEFRAG_FILE_CONTEXT) );
    }

} // end of CopyFileCtx


LONGLONG
CompareFileCtx( 
    IN  PVOID       InFileCtx1,
    IN  PVOID       InFileCtx2,
    IN  ULONG       SortMode )
{
    LONGLONG                Result = 0;

    PDEFRAG_FILE_CONTEXT    FileCtx1 = (PDEFRAG_FILE_CONTEXT)InFileCtx1;
    PDEFRAG_FILE_CONTEXT    FileCtx2 = (PDEFRAG_FILE_CONTEXT)InFileCtx2;

    UNICODE_STRING          Path1 = { 0 };
    UNICODE_STRING          Path2 = { 0 };
    PWCHAR                  PathPtr1 = NULL, PathPtr2 = NULL;

#ifdef _DEBUG
    if ( !FileCtx1 || !FileCtx2 )
    {
        return 0;
        //ASSERT( FALSE );
    }
#endif

    switch ( SortMode )
    {
    case SortModeId:

        Result = (LONGLONG)(FileCtx1->FId.FileId - FileCtx2->FId.FileId );

        break;

    case SortModeGroup:

        Result = (LONGLONG)(FileCtx1->ParentDirId.FileId - FileCtx2->ParentDirId.FileId );

        break;

    case SortModeSize:

        Result = (LONGLONG)(FileCtx1->TotalClustersNum.QuadPart - FileCtx2->TotalClustersNum.QuadPart);

        break;

    case SortModeName:

        if ( FileCtx1->FName && FileCtx2->FName )
        {
            Result = _wcsicmp( FileCtx1->FName, FileCtx2->FName );
        }

        break;

    case SortModePath:

        if ( !(FileCtx1->DefragFlags2 & DEFRAG_FILE2_FULL_NAME) )
        {
            if ( NO_ERROR != DfrgGetFileName( FileCtx1->VolumeCtx->JobCtx, FileCtx1, &Path1 ) )
            {
                break;
            }
            PathPtr1 = Path1.Buffer;
        }
        else
        {
            PathPtr1 = FileCtx1->FName;
        }

        if ( !(FileCtx2->DefragFlags2 & DEFRAG_FILE2_FULL_NAME) )
        {
            if ( NO_ERROR != DfrgGetFileName( FileCtx2->VolumeCtx->JobCtx, FileCtx2, &Path2 ) )
            {
                break;
            }
            PathPtr2 = Path2.Buffer;
        }
        else
        {
            PathPtr2 = FileCtx2->FName;
        }

        Result = _wcsicmp( PathPtr1, PathPtr2 );

        break;

    case SortModeCreatedDate:

        Result = (LONGLONG)(FileCtx1->CreationTime.QuadPart - FileCtx2->CreationTime.QuadPart);

        break;

    case SortModeUsedDate:

        Result = (LONGLONG)(FileCtx1->LastAccessTime.QuadPart - FileCtx2->LastAccessTime.QuadPart);

        break;

    case SortModeModifiedDate:

        Result = (LONGLONG)(FileCtx1->ChangeTime.QuadPart - FileCtx2->ChangeTime.QuadPart);

        break;

    case SortModeLcn:

        if ( !FileCtx1->DefaultStream || !FileCtx2->DefaultStream ||
            !FileCtx1->DefaultStream->ClustersMap || !FileCtx2->DefaultStream->ClustersMap ||
            !FileCtx1->DefaultStream->ClustersMap->ExtentCount || !FileCtx2->DefaultStream->ClustersMap->ExtentCount )

        {
            return 0;
        }
        else
        {
            Result = (LONGLONG)(FileCtx1->DefaultStream->ClustersMap->Extents[0].Lcn.QuadPart - 
                FileCtx2->DefaultStream->ClustersMap->Extents[0].Lcn.QuadPart);
        }

        break;

    default:

        Result = memcmp( FileCtx1, FileCtx2, sizeof(DEFRAG_FILE_CONTEXT) );

        break;
    }

    UniFreeUnicodeString( &Path1 );
    UniFreeUnicodeString( &Path2 );

    return Result;

} // end of CompareFileCtx


//#define _DEBUG_FCTX

void
FreeStreamCtx( 
    IN  PVOID       InStreamCtx )
{
    PDEFRAG_STREAM_CONTEXT  StreamCtx   = (PDEFRAG_STREAM_CONTEXT)InStreamCtx;

    if ( StreamCtx )
    {
        if ( StreamCtx->StreamName )
        {
    #ifdef _DEBUG_FCTX
            swprintf( Mess+wcslen(Mess), L" : %ls", StreamCtx->StreamName );
    #endif
            free( StreamCtx->StreamName );
            StreamCtx->StreamName = NULL;
        }

        if ( StreamCtx->ClustersMap )
        {
            free( StreamCtx->ClustersMap );
            StreamCtx->ClustersMap = NULL;
        }

        free( StreamCtx );
    }

} // end of FreeStreamCtx


// ===========================
//  Cluster Map Utils
// ===========================

#ifdef _DEBUG

void
PrintClustersMap( 
    PRETRIEVAL_POINTERS_BUFFER  ClustersMap )
{
    WCHAR                       Mess[MAX_PATH];
    LARGE_INTEGER               StartingVcn;
    
    
    OutMessage( L"ClustersMap >>>>> " );

    if ( !ClustersMap )
    {
        OutMessage( L"    NULL" );
        return;
    }

    StartingVcn = ClustersMap->StartingVcn;

    for ( DWORD i=0; i<ClustersMap->ExtentCount;
        StartingVcn = ClustersMap->Extents[i].NextVcn, i++ )
    {
        if ( VIRTUAL_LCN == ClustersMap->Extents[i].Lcn.QuadPart ) 
        {
            swprintf( Mess, L"   Sparce extent/run:  Vcn: %I64d  Length: %I64d",
                StartingVcn.QuadPart, 
                ClustersMap->Extents[i].NextVcn.QuadPart - StartingVcn.QuadPart ); 
        } 
        else 
        {
            swprintf( Mess, L"   Vcn: %I64d  Lcn: %I64d  Length: %I64d",
                StartingVcn.QuadPart, ClustersMap->Extents[i].Lcn.QuadPart, 
                ClustersMap->Extents[i].NextVcn.QuadPart - StartingVcn.QuadPart );
        }
        OutMessage( Mess );

    }

    OutMessage( L"" ); //L"<<<<< ClustersMap " );

} // end of PrintClustersMap

#endif

BOOL
IsStreamFragmented(
    PDEFRAG_STREAM_CONTEXT      StreamCtx )
{
    BOOLEAN                     Result = FALSE;
    DWORD                       i, VLcnCnt; 


    if ( StreamCtx && StreamCtx->ClustersMap && StreamCtx->DataSize.QuadPart )
    {
        if ( StreamCtx->ClustersMap->ExtentCount > 1 )
        {
            LARGE_INTEGER       StartingVcn, ExtentLength = { 0 };

            StartingVcn = StreamCtx->ClustersMap->StartingVcn; // 0

            for( i=0, VLcnCnt=0; i<StreamCtx->ClustersMap->ExtentCount; 
                StartingVcn = StreamCtx->ClustersMap->Extents[i].NextVcn, i++ )
            {
                if ( VIRTUAL_LCN == StreamCtx->ClustersMap->Extents[i].Lcn.QuadPart ) 
                {
                    VLcnCnt++;
                    continue;
                }

                if ( i && VLcnCnt < i &&
                    StreamCtx->ClustersMap->Extents[i].Lcn.QuadPart != 
                    StreamCtx->ClustersMap->Extents[i-1-VLcnCnt].Lcn.QuadPart + ExtentLength.QuadPart )
                {
                    Result = TRUE;
                    break;
                }

                VLcnCnt = 0;
                ExtentLength.QuadPart = StreamCtx->ClustersMap->Extents[i].NextVcn.QuadPart - StartingVcn.QuadPart;
            }
        }
    }


    return Result;

} // end of IsStreamFragmented


BOOL
IsFileFragmented(
    PDEFRAG_FILE_CONTEXT        FileCtx )
{
    BOOLEAN                     Result = FALSE;

    PDEFRAG_STREAM_CONTEXT      StreamCtx = NULL;


    if ( !FileCtx )
    {
        return Result;
    }

    //
    // TODO
    //
    //if ( !(FileCtx->DefragFlags & DEFRAG_FILE_EXCLUDED) )
    //{
    //    return Result;
    //}

    for ( StreamCtx=FileCtx->FirstStream; StreamCtx; StreamCtx = StreamCtx->NextStream )
    {
        Result = IsStreamFragmented( StreamCtx );
        if ( Result )
        {
            break;
        }
    }


    return Result;

} // IsFileFragmented

//
// Compare two file maps:
// - StreamCtx->ClustersMap is build from Mft
// - FileClustersMap is got by API
//
int
CheckStreamMaps( 
    PDEFRAG_STREAM_CONTEXT      StreamCtx, 
    PRETRIEVAL_POINTERS_BUFFER  FileClustersMap,
    BOOLEAN                     Print )
{
    int     Result = 0;

    size_t                      ExtentsLength;

#ifdef _DEBUG
    WCHAR                       Mess[MAX_PATH*2];
#endif

    ExtentsLength = FileClustersMap->ExtentCount*2*sizeof(LARGE_INTEGER);

    if ( StreamCtx->ClustersMap->ExtentCount != FileClustersMap->ExtentCount ||
        StreamCtx->ClustersMap->StartingVcn.QuadPart != FileClustersMap->StartingVcn.QuadPart ||
        memcmp( StreamCtx->ClustersMap->Extents, FileClustersMap->Extents, ExtentsLength ) )
    {
#ifdef _DEBUG
      if ( Print )
      {
        PDEFRAG_FILE_CONTEXT      FileCtx = StreamCtx->FileCtx;

        swprintf( Mess, L"      %ls '%ls' ID:%016I64X [%I64d]",
                (FileCtx->DefragFlags & DEFRAG_FILE_DIRECTORY) ? L"Dir " : L"File",
                FileCtx->FName, 
                FileCtx->FId.FileId, 
                FileCtx->TotalClustersNum );
        OutMessage( Mess );
        swprintf( Mess, L"      Wrong map. Stream[%d] vcn %I64d   File[%d] vcn %I64d", 
                        StreamCtx->ClustersMap->ExtentCount,  StreamCtx->ClustersMap->StartingVcn,
                        FileClustersMap->ExtentCount, FileClustersMap->StartingVcn );
        OutMessage( Mess );
        for( DWORD i=0; i<min(StreamCtx->ClustersMap->ExtentCount, FileClustersMap->ExtentCount); i++ )
        {
            swprintf( Mess, L"         %3d next vcn %4I64d,  lcn %8I64d \n"
                            L"             next vcn %4I64d,  lcn %8I64d ", 
                            i, 
                            StreamCtx->ClustersMap->Extents[i].NextVcn, StreamCtx->ClustersMap->Extents[i].Lcn, 
                            FileClustersMap->Extents[i].NextVcn,        FileClustersMap->Extents[i].Lcn );
            OutMessage( Mess ); // A0001555.cmd
        }
        OutMessage( L"" );
      }
#endif
      Result = -1;
    }

    return Result;

} // end of CheckStreamMaps

/*
//
// Get file length in clusters from ClustersMap
//
int
DfrgGetLengthClustersMap( 
    PRETRIEVAL_POINTERS_BUFFER  ClustersMap, 
    PLARGE_INTEGER              Size ) // Uncompressed
{

    LARGE_INTEGER       StartingVcn;

    StartingVcn = ClustersMap->StartingVcn; // 0

    for ( DWORD i=0; i<ClustersMap->ExtentCount; 
        StartingVcn = ClustersMap->Extents[i].NextVcn, i++ )
    {
        if ( VIRTUAL_LCN == StreamCtx->ClustersMap->Extents[i].Lcn.QuadPart ) 
        {
            continue;
        }
        Size->QuadPart += ClustersMap->Extents[i].NextVcn.QuadPart - StartingVcn.QuadPart;
    }

    return NO_ERROR;

} // end of DfrgGetLengthClustersMap
*/

