/*
    Defrag Engine medium level functions header

    Module name:

        DefragUtils.h

    Abstract:

        Defrag Engine medium level functions header. 

    $Header: /home/CVS_DEFRAG/Defrag/Src/Engine/DefragFreeUtils.h,v 1.1 2009/11/24 14:52:15 dimas Exp $
    $Log: DefragFreeUtils.h,v $
    Revision 1.1  2009/11/24 14:52:15  dimas
    no message

*/

#ifndef __DFRG_FREE_UTILS_H__
#define __DFRG_FREE_UTILS_H__


// =================================================
//
// Constants, Macros & Data
//
// =================================================



extern BYTE        BitMask[];



// =================================================
//
// Function prototypes
//
// =================================================


int
DfrgFreeSpaceAgressive(
    IN  PDEFRAG_VOLUME_CONTEXT  VolumeCtx );

int
DfrgFreeSpace(
    IN  PDEFRAG_VOLUME_CONTEXT  VolumeCtx );

int
DfrgForceTogether(
    IN  PDEFRAG_VOLUME_CONTEXT  VolumeCtx );

#endif // !__DFRG_FREE_UTILS_H__