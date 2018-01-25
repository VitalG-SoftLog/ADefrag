/*
    Defrag Engine Core functions header

    Module name:

        EngineCore.h

    Abstract:

        Defrag Engine Core functions & structures header. 

    $Header: /home/CVS_DEFRAG/Defrag/Src/Engine/EngineApi.h,v 1.5 2009/12/21 17:00:13 dimas Exp $
    $Log: EngineApi.h,v $
    Revision 1.5  2009/12/21 17:00:13  dimas
    1. DEFRAG_CMD_GET_MOST_FRAGMENTED and similar requests implemented
    2. Basic Exclude files check added

    Revision 1.4  2009/12/17 10:27:20  dimas
    1. Defragmentation of FAT volumes in release build enabled
    2. Debug function GetDisplayBlockCounts() implemented

    Revision 1.3  2009/12/16 14:13:34  dimas
    DEFRAG_CMD_GET_FILE_INFO request implemented

    Revision 1.2  2009/12/03 13:01:06  dimas
    Cluster Inspector implemented

    Revision 1.1  2009/11/24 14:52:15  dimas
    no message

*/

#ifndef __DFRG_CORE_API_H__
#define __DFRG_CORE_API_H__


// =================================================
//
// Constants, Macros & Data
//
// =================================================

typedef     struct _DFRG_SETTINGS           *PDFRG_SETTINGS;
// =================================================
//
// Exported functions
//
// =================================================

DECLSPEC_ENGINE_IMPEX
int
DfrgInitJobCtx( 
    OUT PDEFRAG_JOB_CONTEXT     *Job,
    PWCHAR                      DriveName, 
    int                         Mode );

DECLSPEC_ENGINE_IMPEX
void
DfrgFreeJobCtx(
    IN  PDEFRAG_JOB_CONTEXT     JobCtx );

DECLSPEC_ENGINE_IMPEX
int
DfrgDoJob(
    IN  PDEFRAG_JOB_CONTEXT     Job,
    IN  PDFRG_SETTINGS          Settings );

DECLSPEC_ENGINE_IMPEX
int 
DfrgClusterInspector( 
    IN      PDEFRAG_JOB_CONTEXT     Job,
    IN      PCLUSTERS_INFO          ClustersBlock,
    OUT     PDEFRAG_CINSPECTOR_INFO ClusterInfo,
    IN  OUT PULONG                  DataLength );

DECLSPEC_ENGINE_IMPEX
int 
DfrgGetFileInfo( 
    IN      PDEFRAG_JOB_CONTEXT Job,
    IN      PWCHAR              FileName,
    OUT     PDEFRAG_FILE_INFO   FileInfo,
    IN  OUT PULONG              DataLength );

DECLSPEC_ENGINE_IMPEX
int 
DfrgGetMarkedFiles( 
    IN      PDEFRAG_JOB_CONTEXT Job,
    IN      ULONG               Command,
    OUT     PDEFRAG_FILE_LIST   FileList,
    IN  OUT PULONG              DataLength );


DECLSPEC_ENGINE_IMPEX
void
GetDisplayBlockCounts(
    PDEFRAG_JOB_CONTEXT         JobCtx,
    ULONGLONG                   Cluster,
    PDFRG_MAP_BLOCK_INFO        DisplayMapBlock,
    DWORD                       *BlockNum );

#endif // !__DFRG_CORE_H__