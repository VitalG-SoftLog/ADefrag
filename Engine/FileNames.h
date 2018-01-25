/*
    Defrag Engine Core functions header

    Module name:

        FileNames.h

    Abstract:

        Defrag Engine file names processing functions & structures header. 

    $Header: /home/CVS_DEFRAG/Defrag/Src/Engine/FileNames.h,v 1.1 2009/12/24 10:52:20 dimas Exp $
    $Log: FileNames.h,v $
    Revision 1.1  2009/12/24 10:52:20  dimas
    Check against Exclude Files list implemented

*/

#ifndef __DFRG_FNAMES_H__
#define __DFRG_FNAMES_H__


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

BOOLEAN
IsFileExcluded( 
    PDEFRAG_JOB_CONTEXT         JobCtx, 
    PDEFRAG_FILE_CONTEXT        FileCtx );

void
DfrgResolveFileParent( 
    PDEFRAG_VOLUME_CONTEXT      VolumeCtx,
    PDEFRAG_FILE_CONTEXT        FileCtx );

int
DfrgResolveFileTree( 
    PDEFRAG_VOLUME_CONTEXT      VolumeCtx );

int
DfrgBuildFileName( 
    PDEFRAG_JOB_CONTEXT         JobCtx, 
    PDEFRAG_FILE_CONTEXT        FileCtx,
    PUNICODE_STRING             UniName );

int
DfrgGetFileName(
    PDEFRAG_JOB_CONTEXT         JobCtx, 
    PDEFRAG_FILE_CONTEXT        FileCtx,
    PUNICODE_STRING             UniName,
    PULONG                      Flags = NULL );


#endif // !__DFRG_FNAMES_H__