/******************************************************************************
*
* $Workfile: ADefragShellExt.cpp $
*
* Project  : ADefragShellExt
*
* Purpose  : Implementation of DLL Exports.
*
* Autor: Oleg Shaposhnikov
*
******************************************************************************/

#include "stdafx.h"
#include "resource.h"

// The module attribute causes DllMain, DllRegisterServer and DllUnregisterServer to be automatically implemented for you
[ module(dll, uuid = "{D0DC4031-3249-4be6-8061-E6068D0E6C9B}", 
         name = "ADefragShellExt", 
         //helpstring = "ADefragShellExt 1.0 Type Library",
         resource_name = "IDR_ADEFRAGSHELLEXT") ]
class CADefragShellExtModule
{
public:
// Override CAtlDllModuleT members
    HRESULT DllRegisterServer(BOOL bRegTypeLib = TRUE) throw()
    {
        CRegKey reg;
        if( ERROR_SUCCESS == reg.Open( HKEY_CLASSES_ROOT, _T("*\\shellex\\ContextMenuHandlers") ) )
        {
            reg.SetKeyValue( _T("ADefragShell"), _T("{8C30CF78-53F1-45a1-B8B2-DE2D80E70093}"), _T("") );
            reg.Close();
        }
        if( ERROR_SUCCESS == reg.Open( HKEY_CLASSES_ROOT, _T("Folder\\shellex\\ContextMenuHandlers") ) )
        {
            reg.SetKeyValue( _T("ADefragShell"), _T("{8C30CF78-53F1-45a1-B8B2-DE2D80E70093}"), _T("") );
            reg.Close();
        }
        if( ERROR_SUCCESS == reg.Open( HKEY_CLASSES_ROOT, _T("AllFilesystemObjects\\shellex\\ContextMenuHandlers") ) )
        {
            reg.SetKeyValue( _T("ADefragShell"), _T("{8C30CF78-53F1-45a1-B8B2-DE2D80E70093}"), _T("") );
            reg.Close();
        }
        if( ERROR_SUCCESS == reg.Open( HKEY_CLASSES_ROOT, _T("Drive\\shellex\\ContextMenuHandlers") ) )
        {
            reg.SetKeyValue( _T("ADefragShell"), _T("{8C30CF78-53F1-45a1-B8B2-DE2D80E70093}"), _T("") );
            reg.Close();
        }
        // to see for all users
        if( ERROR_SUCCESS == reg.Open( HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Approved") ) )
        {
            reg.SetStringValue( _T("{8C30CF78-53F1-45a1-B8B2-DE2D80E70093}"), _T("ADefragShell") );
            reg.Close();
        }
        SHChangeNotify( SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL );
        return __super::DllRegisterServer( bRegTypeLib );
    }

    HRESULT DllUnregisterServer(BOOL bUnRegTypeLib = TRUE) throw()
    {
        HRESULT hr = __super::DllUnregisterServer( bUnRegTypeLib );
        if( FAILED(hr) ) return hr;
        CRegKey reg;
        // to see for all users
        if( ERROR_SUCCESS == reg.Open( HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Approved") ) )
        {
            reg.DeleteValue( _T("{8C30CF78-53F1-45a1-B8B2-DE2D80E70093}") );
            reg.Close();
        }
        // Remove All registrations
        if( ERROR_SUCCESS == reg.Open( HKEY_CLASSES_ROOT, _T("*\\shellex\\ContextMenuHandlers") ) )
        {
            reg.DeleteSubKey( _T("ADefragShell") );
            reg.Close();
        }
        if( ERROR_SUCCESS == reg.Open( HKEY_CLASSES_ROOT, _T("Folder\\shellex\\ContextMenuHandlers") ) )
        {
            reg.DeleteSubKey( _T("ADefragShell") );
            reg.Close();
        }
        if( ERROR_SUCCESS == reg.Open( HKEY_CLASSES_ROOT, _T("AllFilesystemObjects\\shellex\\ContextMenuHandlers") ) )
        {
            reg.DeleteSubKey( _T("ADefragShell") );
            reg.Close();
        }
        if( ERROR_SUCCESS == reg.Open( HKEY_CLASSES_ROOT, _T("Drive\\shellex\\ContextMenuHandlers") ) )
        {
            reg.DeleteSubKey( _T("ADefragShell") );
            reg.Close();
        }

        SHChangeNotify( SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL );
        return hr;
    }
};
