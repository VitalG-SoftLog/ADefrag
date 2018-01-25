/*
    Defrag Engine

    Module name:

        SecUtils.cpp

    Abstract:

        Defrag security module. 
        Defines required security related functions.

    $Header: /home/CVS_DEFRAG/Defrag/Src/Shared/SecUtils.cpp,v 1.1 2009/11/24 14:51:31 dimas Exp $
    $Log: SecUtils.cpp,v $
    Revision 1.1  2009/11/24 14:51:31  dimas
    no message

*/

#include "stdafx.h"


// ========================================
//
// Priveleges
//
// ========================================

//
// SetProcessAdminPrivilege() turn on Administrator privileges for current thread. 
// SE_RESTORE_NAME / SE_BACKUP_NAME required to open file regardless on its SID.
//
int   
SetProcessAdminPrivilege( 
    void )
{
    int                     i, j, PNUM=2, err = NO_ERROR;
    DWORD                   ccCount = 0;
    TOKEN_PRIVILEGES        tp;
    LUID                    luid;
    HANDLE                  hToken = NULL;
    TOKEN_PRIVILEGES        *TokenPrivs = NULL;
    WCHAR                   *privsarr[] = { SE_SECURITY_NAME, SE_TAKE_OWNERSHIP_NAME, 
                                            SE_RESTORE_NAME, SE_BACKUP_NAME, 
                                            SE_RELABEL_NAME };
    WCHAR                   mess[MAX_PATH];



    PNUM = ARRAYSIZE( privsarr );
    if ( Global.OsVersion.dwMajorVersion < 6 )
    {
        PNUM--;
    }
#ifdef _DEBUGW
    else
    {
        OutMessage( L"On Vista SE_RELABEL_NAME should be enabled" );
    }
#endif

    if ( PNUM == 0 ) 
    {
        return( err );
    }

    __try 
    {

        //if ( OpenProcessToken( GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken ) ) 
        if ( !OpenThreadToken( GetCurrentThread(), 
                                TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES,
                                FALSE, &hToken ) ) 
        {
            //
            // make process' token copy for curent thread
            //
            if ( !ImpersonateSelf( SecurityImpersonation ) ) 
            {
                err = DislpayErrorMessage( L"ImpersonateSelf", -1, ERR_OS );
                __leave;
            }
            else if ( !OpenThreadToken( GetCurrentThread(), 
                                        TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES,
                                        FALSE, &hToken ) ) 
            {
                err = DislpayErrorMessage( L"OpenProcess_or_ThreadToken", -1, ERR_OS );
                __leave;
            }
        }


        GetTokenInformation( hToken, TokenPrivileges, NULL, 0, &ccCount );

        if ( TokenPrivs ) 
        {
            free( TokenPrivs );
            TokenPrivs = NULL;
        }

        TokenPrivs = (TOKEN_PRIVILEGES *)malloc( ccCount );
        if ( !TokenPrivs ) 
        {
            swprintf( mess, _T("SetProcessAdminPrivilege: Error alloc %d bytes for GetTokenInformation"), ccCount );
            err = ERROR_NOT_ENOUGH_MEMORY;
            OutMessage( mess );
            __leave;
        }

        if ( !GetTokenInformation( hToken, TokenPrivileges, TokenPrivs, ccCount, &ccCount ) ) 
        {
            err = DislpayErrorMessage( L"Error GetTokenInformation", -1, ERR_OS );
            __leave;
        }

#ifdef _DEBUGW
        _stprintf( mess, _T("Thread token has %d privs"), (int)TokenPrivs->PrivilegeCount );
        OutMessage( mess );
#endif

        for ( j=0; j<PNUM; j++ ) 
        {

            if ( !LookupPrivilegeValue( NULL, privsarr[j], &luid ) ) 
            {
                _stprintf( mess, _T("Error LookupPrivilegeValue of %s"), privsarr[j] );
                DislpayErrorMessage( mess, -1, ERR_OS );
                continue;
            }

            for ( i=0; i<(int)TokenPrivs->PrivilegeCount; i++ ) 
            {

#ifdef _DEBUGW
                WCHAR       pnb[MAX_PATH];
                ccCount = sizeof(pnb);
                LookupPrivilegeName( NULL, &(TokenPrivs->Privileges[i].Luid), pnb, &ccCount );
#ifdef _DEBUG
                _stprintf( mess, _T("      - priv %s %08X%08X =? %08X%08X  (%c) \n"),
                                pnb,
                                TokenPrivs->Privileges[i].Luid.HighPart,
                                TokenPrivs->Privileges[i].Luid.LowPart,
                                luid.HighPart, luid.LowPart,
                                (TokenPrivs->Privileges[i].Attributes & SE_PRIVILEGE_ENABLED)?'+':'-' );
                OutMessage( mess );
#endif
#endif
                if ( TokenPrivs->Privileges[i].Luid.LowPart == luid.LowPart &&
                    TokenPrivs->Privileges[i].Luid.HighPart == luid.HighPart &&
                    (TokenPrivs->Privileges[i].Attributes & SE_PRIVILEGE_ENABLED) ) 
                {
#ifdef _DEBUGW
                    _stprintf( mess, _T("   - %s is already set\n"), privsarr[j] );
                    OutMessage( mess );
#endif
                    break;
                }
            }
            if ( i != (int)TokenPrivs->PrivilegeCount ) 
            {
                continue;
            }

            tp.PrivilegeCount = 1;
            tp.Privileges[0].Luid = luid;
            tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED; // disable = 0

            //
            // Enable the privilege or disable all privileges.
            //
            if ( !AdjustTokenPrivileges( hToken, FALSE,                       
                                       &tp, sizeof(TOKEN_PRIVILEGES),    
                                       (PTOKEN_PRIVILEGES)NULL, (PDWORD)NULL ) ) 
            { 
                swprintf( mess, _T("Error AdjustTokenPrivileges %s "), privsarr[j] );
                DislpayErrorMessage( mess, -1, ERR_OS );
            }
            else 
            {
#ifdef _DEBUGW
                _stprintf( mess, _T("  - AdjustTokenPrivileges %s OK"), privsarr[j] );
                OutMessage( mess );
#endif
            }

        } // for j < PNUM

    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        err = GetExceptionCode();
        OutMessage( L"SetProcessAdminPrivilege: exception occures" );
    }


    //
    // Cleanup
    //
    if ( hToken ) 
    {
        CloseHandle( hToken );
        hToken  = NULL;
    }

    if ( TokenPrivs ) 
    {
        free( TokenPrivs );
        TokenPrivs = NULL;
    }

    return( err );

} // end of SetProcessAdminPrivilege


