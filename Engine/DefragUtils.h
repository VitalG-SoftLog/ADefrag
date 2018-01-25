/*
    Defrag Engine medium level functions header

    Module name:

        DefragUtils.h

    Abstract:

        Defrag Engine medium level functions header. 

    $Header: /home/CVS_DEFRAG/Defrag/Src/Engine/DefragUtils.h,v 1.1 2009/11/24 14:52:15 dimas Exp $
    $Log: DefragUtils.h,v $
    Revision 1.1  2009/11/24 14:52:15  dimas
    no message

*/

#ifndef __DFRG_UTILS_H__
#define __DFRG_UTILS_H__


// =================================================
//
// Constants, Macros & Data
//
// =================================================

#define     SHOULD_OPEN_STREAM          0x00000001
#define     SHOULD_ADJUST_CLUSTER_MAP   0x00000002

#define     SHOULD_ADJUST_USED_GAP      0x00000010
#define     SHOULD_ADJUST_NEW_GAP       0x00000020

#define     SHOULD_ADJUST_USED_EXTENT   0x00000100

#define     SHOULD_ADJUST_GAPS_TREE     0x000000F0
#define     SHOULD_ADJUST_EXTENTS_TREE  0x00000F00


extern BYTE        BitMask[];



// =================================================
//
// Function prototypes
//
// =================================================


void
MarkProgress( 
    PDEFRAG_VOLUME_CONTEXT      VolumeCtx,
    LONGLONG                    ProgressIncrement );

int
DfrgMoveExtentToGap( 
    PDEFRAG_FILES_EXTENT        Extent,
    PDEFRAG_LCN_EXTENT          Gap,
    DWORD                       Flags );

int
DfrgSlideExtentToGap( 
    PDEFRAG_FILES_EXTENT        Extent,
    PDEFRAG_LCN_EXTENT          Gap,
    DWORD                       MoveExtentFlags );

int
DfrgMoveFileToGap( 
    PDEFRAG_FILE_CONTEXT        FileCtx,
    PDEFRAG_ZONE                SourceZone,
    PDEFRAG_LCN_EXTENT          Gap );

int
DfrgSimple(
    IN  PDEFRAG_VOLUME_CONTEXT  VolumeCtx );

#endif // !__DFRG_UTILS_H__