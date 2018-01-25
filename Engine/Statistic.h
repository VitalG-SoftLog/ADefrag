// =================================================
//
// DisplayUtils.h : Defines display and print functions, etc.
//
// =================================================

#ifndef __DFRG_STATISTIC_H__
#define __DFRG_STATISTIC_H__


// =================================================
//
// Constants, Macros & Data
//
// =================================================

#define     SUBSTRACT_CLASTERS      0x80000000

#define     INCL_FLAGS_COUNTS       0x00000001

#define     INCL_INPROC_COUNTS      0x00000010

#define     INCL_SPACE_COUNTS       0x00000100


// =================================================
//
// Function prototypes
//
// =================================================

void
PrintStatistic(
    PDEFRAG_JOB_CONTEXT     JobCtx );

DEFRAG_RATIO_TYPE       
DfrgCulcFileFragmentation( 
    PDEFRAG_FILE_CONTEXT    FileCtx );

void
UpdateCurrentNameStatistic( 
    PDEFRAG_JOB_CONTEXT     JobCtx );

void
UpdateFilesStatistic( 
    PDEFRAG_VOLUME_CONTEXT  VolumeCtx,
    PDEFRAG_FILE_CONTEXT    FileCtx );

void
UpdateGapsStatistic( 
    PDEFRAG_VOLUME_CONTEXT  VolumeCtx );

#endif // !__DFRG_STATISTIC_H__