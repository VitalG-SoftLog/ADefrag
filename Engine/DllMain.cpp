/*
    Defrag Engine

    Module name:

        DllMain.cpp

    Abstract:

        Defrag Engine Dll main module. 
        Defines the entry point for the Defrag Engine DLL.

    $Header: /home/CVS_DEFRAG/Defrag/Src/Engine/DllMain.cpp,v 1.1 2009/11/24 14:52:15 dimas Exp $
    $Log: DllMain.cpp,v $
    Revision 1.1  2009/11/24 14:52:15  dimas
    no message

*/


#include "stdafx.h"



ENGINE_GLOBAL      Global;

int
InitEngine( void )
{
    int     Result = NO_ERROR;


    __try
    {

        Global.AppId = ServiceAppId;

        //
        // Get required function address
        // 
        Global.NtQueryFileInfo = (NtQueryInformationFile_Ptr)GetFunctionAddr( L"ntdll.dll", "NtQueryInformationFile" );
        if ( !Global.NtQueryFileInfo )
        {
            OutMessage( L"InitEngine: Could not get NtQueryInformationFile address" );
            //GetInput();
            Result = ERROR_NOT_SUPPORTED;
            __leave;
        }

    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        Result = GetExceptionCode();
        DislpayErrorMessage( L"InitEngine Exception !!!", Result, ERR_OS );
    }

    return Result;

} // end of InitEngine



BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
        InitEngine();

	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

