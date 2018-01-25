// =================================================
//
// Debug.h : Defines error processing and debug output.
//
// =================================================

#ifndef __DFRG_DEBUG_H__
#define __DFRG_DEBUG_H__

#define ERR_OS              1
#define ERR_NT_STATUS       2
#define ERR_NET_STATUS      3
#define ERR_INTERNAL        4


int 
GetInput( void );


int
OutMessage( 
    WCHAR   *UserMess );

int
DislpayErrorMessage( 
    const TCHAR     *UserMess, 
    int             UserErr, 
    int             ErrorType,
    HWND            Parent = 0,
    BOOLEAN         Suppress = FALSE );

#endif // !__DFRG_DEBUG_H__