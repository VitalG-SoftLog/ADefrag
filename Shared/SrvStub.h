// =================================================
//
// SrvStub.h : Defines service entry point, callbacks, etc.
//
// =================================================

#ifndef __SRVSTUB_H__
#define __SRVSTUB_H__


//
// Service standard functions
//
VOID  
WINAPI
ServiceMain( 
    DWORD   argc, 
    LPTSTR  *argv );

DWORD 
WINAPI
ServiceCtrlHandler( 
    DWORD   opcode,
    DWORD   dwEventType,     // event type
    LPVOID  lpEventData,     // event data
    LPVOID  lpContext   );   // user-defined context data

int            
SelfRegister( 
    TCHAR *exePath );

int            
SelfUnregister(
    void );

int            
SelfStartService( 
    void );

int            
SelfStopService(
    void );

int
QuerySrv( 
    OUT LPSERVICE_STATUS    ServiceStatus );

#endif // !__SRVSTUB_H__

