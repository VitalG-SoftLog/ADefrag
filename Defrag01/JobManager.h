// =================================================
//
// Sheduler.h : Defines Sheduler and API constants, types, structures, functions, etc.
//
// =================================================

#ifndef __DFRG_JOB_MANAGER_H__
#define __DFRG_JOB_MANAGER_H__


// =================================================
//
// Constants, Macros & Data
//
// =================================================


//#pragma pack( push, 1 )
//
//
//#pragma pack( pop )


// =================================================
//
// Function prototypes
//
// =================================================

int
DfrgStartJobManager( 
    HANDLE *ObjectToWait );


void         
DfrgStopJobManager( 
    HANDLE ObjectToWait );

void
DfrgStopAllJobs( void );

//
// Job Context. Init is in DefragUtils.
//

void
InsertJob( 
    IN  PDEFRAG_JOB_CONTEXT     Job );

PDEFRAG_JOB_CONTEXT
FindJob( 
    IN  DWORD                   JobId );

PDEFRAG_JOB_CONTEXT
FreeJob(
    IN  PDEFRAG_JOB_CONTEXT     JobCtx );

void
FreeJobList(
    IN  PDEFRAG_JOB_CONTEXT     JobList );

int
StartJobThread(
    PDEFRAG_JOB_CONTEXT         Job );

#endif // !__DFRG_JOB_MANAGER_H__
