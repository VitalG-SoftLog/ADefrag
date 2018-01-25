/*
    Defrag Engine medium level functions header

    Module name:

        DefragUtils.h

    Abstract:

        Defrag Engine medium level functions header. 

    $Header: /home/CVS_DEFRAG/Defrag/Src/Engine/DefragSmartUtils.h,v 1.1 2009/11/24 14:52:15 dimas Exp $
    $Log: DefragSmartUtils.h,v $
    Revision 1.1  2009/11/24 14:52:15  dimas
    no message

*/

#ifndef __DFRG_SMART_UTILS_H__
#define __DFRG_SMART_UTILS_H__


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


int
DfrgSmart(
    IN  PDEFRAG_VOLUME_CONTEXT  VolumeCtx );

int
DfrgSuperSmart(
    IN  PDEFRAG_VOLUME_CONTEXT  VolumeCtx );


#endif // !__DFRG_SMART_UTILS_H__