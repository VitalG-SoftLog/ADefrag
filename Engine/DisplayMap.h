// =================================================
//
// DisplayUtils.h : Defines display and print functions, etc.
//
// =================================================

#ifndef __DFRG_DISPLAY_H__
#define __DFRG_DISPLAY_H__


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
SetDisplayMapEx( 
    PDEFRAG_JOB_CONTEXT         JobCtx );

void
UpdateDisplayForStream( 
    PDEFRAG_FILE_CONTEXT        FileCtx,
    PDEFRAG_STREAM_CONTEXT      StreamCtx,
    DWORD                       Flags );

void
UpdateDisplayForExtent(
    PDEFRAG_FILE_CONTEXT        FileCtx,
    ULONGLONG                   ExtentStart,
    ULONGLONG                   ExtentLength,
    DWORD                       Flags );

void
UpdateBitMap( 
    PDEFRAG_FILE_CONTEXT            FileCtx,
    ULONGLONG                       GapStartLcn,
    ULONGLONG                       ExtentStartLcn,
    ULONGLONG                       ExtentLength );

#endif // !__DFRG_DISPLAY_H__