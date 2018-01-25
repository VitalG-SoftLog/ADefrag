/*
    Defrag Server Scheduler header

    Module name:

        Scheduler.h

    Abstract:

        Defrag Server Scheduler header.
        Defines scheduler functions

    $Header: /home/CVS_DEFRAG/Defrag/Src/Defrag01/Scheduler.h,v 1.1 2009/11/24 14:52:31 dimas Exp $
    $Log: Scheduler.h,v $
    Revision 1.1  2009/11/24 14:52:31  dimas
    no message

*/

#ifndef __DEFRAG_SCHEDULER_H__
#define __DEFRAG_SCHEDULER_H__


// =================================================
//
// Function prototypes
//
// =================================================

int 
StartSchedulerTaskManager( void );

void 
StopSchedulerTaskManager( void );

void
SetShedulerEvent( void );

#endif __DEFRAG_SCHEDULER_H__
