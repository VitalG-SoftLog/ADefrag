/*
    Defrag Engine

    Module name:

        Debug.cpp

    Abstract:

        Defrag debug module. 
        Defines error processing and debug output.

    $Header: /home/CVS_DEFRAG/Defrag/Src/Shared/Debug.cpp,v 1.2 2009/12/24 15:02:51 dimas Exp $
    $Log: Debug.cpp,v $
    Revision 1.2  2009/12/24 15:02:51  dimas
    Some cosmetic improvements

    Revision 1.1  2009/11/24 14:51:31  dimas
    no message

*/


#include "stdafx.h"


__declspec( thread ) TCHAR    ErrorMessage[MAX_PATH*2];


#ifndef DEFRAG_GUI_APP

int GetInput()
{
    int     Input = 0;

#if (defined _DEBUG) && !(defined DEFRAG_NATIVE_APP)
    WCHAR   BreakChar;

    if ( ServiceAppId != Global.AppId )
    {
        OutMessage( L"Press any key > " );

        BreakChar = _getwch();

        if ( BreakChar == L'\x1B' )
        {
            Input = -1;
        }
        else
        {
            if ( (L'\0' == BreakChar || 
                  L'\xE0' == BreakChar) &&
                _getwch() == L'\x1B' )
            {
                Input = -1;
            }
        }
    }
#endif

    return Input;

} // end of GetInput

#endif


int
OutMessage( 
    WCHAR *UserMess )
{

    if ( ServiceAppId == Global.AppId )
    {
        OutputDebugString( UserMess );
        OutputDebugString( L"\n" );
    }
    else if ( GuiAppId == Global.AppId )
    {
        OutputDebugString( L"[Client]  " );
        OutputDebugString( UserMess );
        OutputDebugString( L"\n" );
    }
    else
    {
        _putws( UserMess );
    }


    return NO_ERROR;

} // end of OutMessage


int 
DislpayErrorMessage( 
    const TCHAR *UserMess, 
    int         UserErr, 
    int         ErrorType,
    HWND        Parent,
    BOOLEAN     Suppress )
{
    int                 SysErr      = NO_ERROR;
    NTSTATUS            NtStatus    = STATUS_SUCCESS;
    NET_API_STATUS      NetStatus   = NERR_Success;

    HMODULE             Module = NULL; // default to system source
    LPTSTR              Mess = NULL;
    DWORD               FormatFlags  = FORMAT_MESSAGE_ALLOCATE_BUFFER |
                                       FORMAT_MESSAGE_IGNORE_INSERTS |
                                       FORMAT_MESSAGE_FROM_SYSTEM ;


    _tcsncpy( ErrorMessage, UserMess, ARRAYSIZE(ErrorMessage)/2 ); 
    *(ErrorMessage+ARRAYSIZE(ErrorMessage)/2) = 0;

    switch( ErrorType )
    {
    case ERR_NT_STATUS: 

        NtStatus = UserErr;
        if ( STATUS_SUCCESS == NtStatus )
        {
            return NO_ERROR;
        }

#ifndef DEFRAG_NATIVE_APP
        SysErr = LsaNtStatusToWinError( NtStatus );
        _stprintf( ErrorMessage+_tcslen(ErrorMessage), _T(" NtStat %08lX => %d - "), NtStatus, SysErr );
#else
        _stprintf( ErrorMessage+_tcslen(ErrorMessage), _T(" NtStat %08lX "), NtStatus );
#endif

        break;

    case ERR_NET_STATUS: 

        NetStatus = UserErr;
        if ( NERR_Success == NetStatus  )
        {
            return NO_ERROR;
        }

        //
        // If dwLastError is in the network range, load the message source.
        //
        if ( NetStatus >= NERR_BASE && NetStatus <= MAX_NERR ) 
        {
            Module = LoadLibraryEx( _T("netmsg.dll"), NULL, LOAD_LIBRARY_AS_DATAFILE);
            if ( Module )
            {
                FormatFlags |= FORMAT_MESSAGE_FROM_HMODULE;
            }
        }

        SysErr = NetStatus;

        _stprintf( ErrorMessage+_tcslen(ErrorMessage), _T(" NetErr %d (%08lX) - "), NetStatus, NetStatus );

        break;

    case ERR_INTERNAL:

        if ( DFRG_ERROR_ABORTED == UserErr )
        {
            return NO_ERROR;
        }

        switch( UserErr )
        {
        case DFRG_ERROR_ABORTED:
            _stprintf( ErrorMessage+_tcslen(ErrorMessage), _T(" Err %d - %ls"), UserErr, DFRG_ERROR_ABORTED_MESS );
            break;
        case DFRG_ERROR_UNKNOWN_FS:
            _stprintf( ErrorMessage+_tcslen(ErrorMessage), _T(" Err %d - %ls"), UserErr, DFRG_ERROR_UNKNOWN_FS_MESS );
            break;
        case DFRG_ERROR_FS_DATA_CORRUPTED:
            _stprintf( ErrorMessage+_tcslen(ErrorMessage), _T(" Err %d - %ls"), UserErr, DFRG_ERROR_FS_DATA_CORRUPTED_MESS );
            break;
        case DFRG_ERROR_NOT_ENOUGH_SPACE:
            _stprintf( ErrorMessage+_tcslen(ErrorMessage), _T(" Err %d - %ls"), UserErr, DFRG_ERROR_NOT_ENOUGH_SPACE_MESS );
            break;
        default:
            _stprintf( ErrorMessage+_tcslen(ErrorMessage), _T(" Err %d - %ls"), UserErr, DFRG_ERROR_UNKNOWN_MESS );
            break;
        }

        break;

    case ERR_OS: 
    default:

        SysErr = GetLastError();

        if ( NO_ERROR == SysErr && NO_ERROR == UserErr )
        {
            return NO_ERROR;
        }
        else if ( DFRG_ERROR_ABORTED == UserErr )
        {
            _stprintf( ErrorMessage+_tcslen(ErrorMessage), _T(" SysErr %d - "),
                             SysErr );
        }
        else if ( NO_ERROR == SysErr )
        {
            _stprintf( ErrorMessage+_tcslen(ErrorMessage), _T(" Error %d "),
                             UserErr );
        }
        else
        {
            _stprintf( ErrorMessage+_tcslen(ErrorMessage), _T(" Err %d - SysErr %d (%08lX) - "),
                             UserErr, SysErr, SysErr );
        }

        break;
    } // switch

#ifndef DEFRAG_NATIVE_APP
    if ( NO_ERROR != SysErr )
    {
        FormatMessage(  FormatFlags,
                        Module, 
                        SysErr, 
                        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                        (LPTSTR)&Mess, 
                        MAX_PATH, 
                        NULL );
        if ( Mess ) 
        {
            _tcscat( ErrorMessage, _T("\n")  );
            _tcsncpy( ErrorMessage+_tcslen(ErrorMessage), Mess, ARRAYSIZE(ErrorMessage)-_tcslen(ErrorMessage)-3 );
            ErrorMessage[ARRAYSIZE(ErrorMessage)-1] = 0;
            LocalFree( Mess ); // not free( Mess ) !!!
        }
        else 
        {
            _tcscat( ErrorMessage, _T("sorry, FormatMessage error") );
        }
    }
#endif

    //
    // Output debug error message
    // 
    if ( !Suppress )
    {
        OutMessage( ErrorMessage );

//#ifdef DEFRAG_GUI_APP
#ifndef DEFRAG_NATIVE_APP
        //
        // Output window with error message
        // 
        if ( Parent && GuiAppId == Global.AppId )
        {
            MessageBox( Parent, ErrorMessage, L"Error!", MB_OK | MB_ICONEXCLAMATION );
        }
#endif
    }

    //
    // If we loaded a message source, unload it.
    //
    if ( Module )
    {
        FreeLibrary( Module );
        Module = NULL;
    }

    return SysErr;

} // end of DislpayErrorMessage


