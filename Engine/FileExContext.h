/*
    Defrag Engine context structures header

    Module name:

        ContextUtils.h

    Abstract:

        Defrag Engine context structures header. 

    $Header: /home/CVS_DEFRAG/Defrag/Src/Engine/FileExContext.h,v 1.7 2009/12/25 13:48:53 dimas Exp $
    $Log: FileExContext.h,v $
    Revision 1.7  2009/12/25 13:48:53  dimas
    Debug artefacts removed

    Revision 1.6  2009/12/23 13:01:47  dimas
    1. Full File Name processing implemented.
    2. Top 100 Fragmented functionality for all file list requests implemented.

    Revision 1.5  2009/12/21 17:00:13  dimas
    1. DEFRAG_CMD_GET_MOST_FRAGMENTED and similar requests implemented
    2. Basic Exclude files check added

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


#ifndef __DFRG_CTX_H__
#define __DFRG_CTX_H__


// =================================================
//
// Constants, Macros & Data
//
// =================================================

// =================================================
//
// Function prototypes
//
// =================================================

// ===========================
//  File & Stream Context Utils
// ===========================

void
FreeStreamCtx( 
    IN  PVOID                   InStreamCtx );

void
ReleaseFileCtx( 
    IN  PDEFRAG_FILE_CONTEXT    FileCtx );

    void
FreeFileCtx( 
    IN  PVOID                   InFileCtx );

void
CopyFileCtx( 
    IN  PVOID                   TrgFileCtx,
    IN  PVOID                   SrcFileCtx );

LONGLONG
CompareFileCtx( 
    IN  PVOID                   InFileCtx1,
    IN  PVOID                   InFileCtx2,
    IN  ULONG                   SortMode );

void
PrintFileCtx( 
    IN  PVOID                   InFileCtx );

void
PrintFileList( 
    IN  PDEFRAG_VOLUME_CONTEXT  VolumeCtx,
    IN  PBTR_TREE               Tree,
    IN  BOOLEAN                 PrintBody = FALSE,
    IN  ULONG                   Flags = 0 );

void
PrintExtentList( 
    IN  PDEFRAG_VOLUME_CONTEXT  VolumeCtx,
    LONGLONG                    StartLcn,
    LONGLONG                    EndLcn,
    BOOLEAN                     PrintFileBody );

// ===========================
//  Cluster Map Utils
// ===========================

void
PrintClustersMap( 
    PRETRIEVAL_POINTERS_BUFFER  ClustersMap );

BOOL
IsStreamFragmented(
    PDEFRAG_STREAM_CONTEXT      StreamCtx );

BOOL
IsFileFragmented(
    PDEFRAG_FILE_CONTEXT        FileCtx );

int
CheckStreamMaps( 
    PDEFRAG_STREAM_CONTEXT      StreamCtx, 
    PRETRIEVAL_POINTERS_BUFFER  FileClustersMap,
    BOOLEAN                     Print );

int
DfrgGetLengthClustersMap( 
    PRETRIEVAL_POINTERS_BUFFER  ClustersMap, 
    PLARGE_INTEGER              Size );


#endif // !__DFRG_CTX_H__
