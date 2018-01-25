// =================================================
//
// HelperUtils.h : Defines helper defragmentation functions, etc.
//
// =================================================

#ifndef __DFRG_HLP_H__
#define __DFRG_HLP_H__


// =================================================
//
// Constants, Macros & Data
//
// =================================================

#define     FIND_FIRST_ITEM     TRUE
#define     FIND_NEXT_ITEM      FALSE

// =================================================
//
// Function prototypes
//
// =================================================

// ===========================
//  Decision Utils
// ===========================

BOOLEAN
CheckIsStreamUnmoveable(
    IN  PDEFRAG_VOLUME_CONTEXT  VolumeCtx,
    IN  PDEFRAG_STREAM_CONTEXT  StreamCtx,
    IN  DWORD                   Flags,
    IN  BOOLEAN                 Target = FALSE ); // TRUE - the stream to be defragmrntred

BOOLEAN
ShouldMoveFile( 
    PDEFRAG_FILES_EXTENT        Extent,
    PDEFRAG_FILE_CONTEXT        FileCtx );

BOOLEAN
ShouldFreeExtentForFile( 
    PDEFRAG_FILES_EXTENT        Extent,
    PDEFRAG_FILE_CONTEXT        FileCtx );

PDEFRAG_FILE_CONTEXT
GetNextMoveableFile( 
    PBTR_TREE               FilesTree,
    PBTR_NODE               *Node,
    BOOLEAN                 Direction, // LEFT_TO_RIGHT 
    BOOLEAN                 Start,
    ULONG                   Flags = 0 );

PDEFRAG_FILES_EXTENT
GetNextMoveableExtent( 
    PBTR_TREE               ExtentsTree,
    PBTR_NODE               *Node,
    BOOLEAN                 Direction, // LEFT_TO_RIGHT 
    BOOLEAN                 Start );

LONGLONG
GetFileClustersInZone( 
    IN  PDEFRAG_LCN_EXTENT      ZoneGap, // from lcn - to the end of disk
    IN  PDEFRAG_FILE_CONTEXT    FileCtx );

PDEFRAG_FILE_CONTEXT
GetFileToMove( 
    IN  PBTR_TREE               FilesTree,
    IN  PDEFRAG_FILE_CONTEXT    FileTemplate,
    IN  PDEFRAG_LCN_EXTENT      SourceGap, 
    OUT PBTR_NODE               *Node );

LONGLONG
GetExtentsIntersection(
    PDEFRAG_LCN_EXTENT          Extent1,
    PDEFRAG_LCN_EXTENT          Extent2,
    PDEFRAG_LCN_EXTENT          Intersection );

PDEFRAG_FILES_EXTENT
GetExtentForGap( 
    IN  PBTR_TREE               ExtentsTree,
    IN  PDEFRAG_LCN_EXTENT      TargetGap, 
    IN  PDEFRAG_LCN_EXTENT      SourceGap, 
    IN  BTR_TREE_SORT_MODE      SortMode,
    IN  BOOLEAN                 Direction,
    IN  int                     SizeEquality,
    OUT PBTR_NODE               *Node );

PDEFRAG_LCN_EXTENT
GetBestGapForExtent( 
    IN  PBTR_TREE               GapsTree,
    IN  PDEFRAG_FILES_EXTENT    ExtentTemplate, 
    IN  PDEFRAG_LCN_EXTENT      SourceZone, 
    IN  BTR_TREE_SORT_MODE      SortMode,
    IN  BOOLEAN                 LookupDirection,
    IN  int                     SizeEquality,
    OUT PBTR_NODE               *Node );


// ===========================
//  Gap & Zone Utils
// ===========================

int
DfrgFindGap( 
    ULONGLONG                   Length,
    PBTR_TREE                   GapsTree,
    PBTR_NODE                   *OutGapNode, 
    PDEFRAG_ZONE                ExcludeZone,
    PDEFRAG_ZONE                IncludeZone );

void
DfrgBuildFreeSpaceZones( 
    IN  PDEFRAG_VOLUME_CONTEXT  VolumeCtx,
    OUT PDEFRAG_ZONE            *OutTargetZone, 
    OUT PDEFRAG_ZONE            *OutSourceZone );

void
DfrgBuildSimpleZones( 
    IN  PDEFRAG_VOLUME_CONTEXT  VolumeCtx,
    OUT PDEFRAG_ZONE            *OutMftZone );

void
ExpandZoneWithExtent( 
    IN      PDEFRAG_VOLUME_CONTEXT  VolumeCtx,
    IN  OUT PDEFRAG_ZONE            SourceZone,
    IN      PBTR_NODE               ExtentNode,
    IN      BOOLEAN                 ExtentsMoveDirection,
    IN      DWORD                   Flags );

void
DfrgBuildFreeZones( 
    IN  PDEFRAG_VOLUME_CONTEXT  VolumeCtx,
    OUT PDEFRAG_ZONE            *OutMftZone,
    OUT PDEFRAG_ZONE            *OutSourceZone );

BOOLEAN
IsGapInZone( 
    IN  PDEFRAG_LCN_EXTENT      InGap, 
    OUT PDEFRAG_LCN_EXTENT      OutGap, 
    IN  PDEFRAG_ZONE            Zone );

// ===========================
//  Gaps tree Utils
// ===========================

void
PrintGapsTree( 
    PBTR_TREE                   GapsTree,
    BOOLEAN                     PrintBody );

void
PrintGap(
    IN  PVOID                   InGapBody );

LONGLONG
CompareGaps( 
    IN  PVOID                   InGap1,
    IN  PVOID                   InGap2,
    IN  ULONG                   SortMode );

int
DfrgAddGapToTree( 
    IN  PDEFRAG_VOLUME_CONTEXT  VolumeCtx,
    LONGLONG                    FirstLcn, 
    LONGLONG                    GapLength );

void
DfrgUpdateGapNode( 
    PBTR_TREE                   GapsTree,
    PBTR_NODE                   *GapNode,
    PDEFRAG_LCN_EXTENT          *Gap );

void
DfrgInsertGapNode( 
    PBTR_TREE                   GapsTree,
    PBTR_NODE                   *GapNode,
    PDEFRAG_LCN_EXTENT          *Gap );

int
DfrgBuildGapsTree(
    IN      PDEFRAG_VOLUME_CONTEXT  VolumeCtx,
    IN OUT  PBTR_TREE               GapsTree,
    OUT     LARGE_INTEGER           *FreeSpace );

// ===========================
//  General (Volume) Utils
// ===========================

int
DfrgGetVolumeInfo( 
    PDEFRAG_VOLUME_CONTEXT      VolumeCtx );

int
DfrgGetVolumeSpaceMap( 
    PDEFRAG_VOLUME_CONTEXT  VolumeCtx,
    BOOLEAN                 BuildTreeOnly = FALSE );

#endif // !__DFRG_HLP_H__