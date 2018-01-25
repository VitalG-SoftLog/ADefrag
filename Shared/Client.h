/*
    Defrag Engine Communication header

    Module name:

        Client.h

    Abstract:

        Defrag Engine Communication header. 
        Defines Defrag Engine Client constants, structures and functions.

    $Header: /home/CVS_DEFRAG/Defrag/Src/Shared/Client.h,v 1.2 2009/12/02 14:42:15 dimas Exp $
    $Log: Client.h,v $
    Revision 1.2  2009/12/02 14:42:15  dimas
    App/Service interaction improved

    Revision 1.1  2009/11/24 14:51:31  dimas
    no message

*/

#ifndef __DEFRAG_CLIENT_H__
#define __DEFRAG_CLIENT_H__



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
__stdcall
loadLibrary(void);

void
__stdcall
unloadLibrary(void);

void 
__stdcall
FormatLpcMsg( 
    WCHAR           *buf, 
    BOOLEAN         ServerSide,
    WCHAR           *prefix, 
    PORT_MESSAGE    *msg, 
    DWORD           ReqRes,
    DWORD           Id,
    WCHAR           *postfix = NULL );

int
__stdcall
OpenConnection( void );

void
__stdcall
CloseConnection( void );
/*
int
__stdcall
SendCommand( 
    IN  OUT DWORD       *JobId, 
    IN      DWORD       Command,
    IN      PVOID       InData,
    OUT     DWORD       *JobStatus,
    OUT     DWORD       *JobProgress );
*/
int 
__stdcall
SendCommandEx( 
    IN  OUT DWORD           *JobId, 
    IN      DWORD           Command,
    IN      PVOID           ControlData,
    IN      PVOID           InputData,
    IN      ULONG           InputDataLength,
    IN  OUT PVOID           OutputData,
    IN  OUT ULONG           *OutputDataLength,
    OUT     DWORD           *JobStatus,
    OUT     DWORD           *JobProgress );

int
__stdcall
WaitJobFinished( 
    IN OUT  DWORD       *JobId,
    OUT     DWORD       *JobStatus,
    OUT     DWORD       *JobProgress );

int
__stdcall
RunServiceJob( void );


#endif // !__DEFRAG_CLIENT_H__

