// ADefragExt.cpp : Implementation of CADefragExt

#include "stdafx.h"
#include "ADefragExt.h"
#include "../Include/globalDefs.h"

#define IDM_ADEFRAG_DEFRAG       0

DWORD getLanguageID()
{
    return 0; // Default language is English
}

// CADefragExt
CADefragExt::CADefragExt()
{
}

CADefragExt::~CADefragExt()
{
}

HRESULT CADefragExt::FinalConstruct()
{
    m_hbmpIconDefrag = ::LoadBitmap( _AtlBaseModule.GetResourceInstance(), MAKEINTRESOURCE(IDB_ADEFRAG_DEFRAG) );
    return S_OK;
}

void CADefragExt::FinalRelease()
{
    if( m_hbmpIconDefrag )
    {
        ::DeleteObject( m_hbmpIconDefrag );
        m_hbmpIconDefrag = NULL;
    }
}

//***************************************************************************************
LPITEMIDLIST GetNextItem (LPCITEMIDLIST pidl)
{
    if (pidl == NULL)
    {
        return NULL;
    }

    return (LPITEMIDLIST)(LPBYTE)(((LPBYTE)pidl) + pidl->mkid.cb);
}

//***************************************************************************************
UINT GetItemSize(LPCITEMIDLIST pidl)
{
    UINT           cbTotal = 0;
    LPITEMIDLIST   pidlTemp = (LPITEMIDLIST) pidl;

    if (pidlTemp != NULL)
    {
        while (pidlTemp->mkid.cb != 0)
        {
            cbTotal += pidlTemp->mkid.cb;
            pidlTemp = GetNextItem (pidlTemp);
        }

        // Requires a 16 bit zero value for the NULL terminator
        cbTotal += 2 * sizeof(BYTE);
    }

    return cbTotal;
}

//////////////////////////////////////////////////////////////////////////
LPITEMIDLIST CreateItem (UINT cbSize)
{
    LPITEMIDLIST pidl = (LPITEMIDLIST)malloc(cbSize);
    if (pidl != NULL)
    {
        ZeroMemory (pidl, cbSize);
    }

    return pidl;
}


//////////////////////////////////////////////////////////////////////////
LPITEMIDLIST ConcatenateItem (LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
    UINT cb1 = 0;

    //-----------------------------------------------------------------------
    // Pidl1 can possibly be NULL if it points to the desktop.  Since we only
    // need a single NULL terminator, we remove the extra 2 bytes from the
    // size of the first ITEMIDLIST.
    //-----------------------------------------------------------------------
    if (pidl1 != NULL)
    {
        cb1 = GetItemSize (pidl1) - (2 * sizeof(BYTE));
    }

    UINT cb2 = GetItemSize (pidl2);

    //-----------------------------------------------------------------------
    // Create a new ITEMIDLIST that is the size of both pidl1 and pidl2, then
    // copy pidl1 and pidl2 to the new list.
    //-----------------------------------------------------------------------
    LPITEMIDLIST pidlNew = CreateItem(cb1 + cb2);

    if (pidlNew != NULL)
    {
        if (pidl1 != NULL)
        {
            CopyMemory(pidlNew, pidl1, cb1);
        }

        CopyMemory (((LPBYTE)pidlNew) + cb1, pidl2, cb2);
    }

    return pidlNew;
}

//////////////////////////////////////////////////////////////////////////
CString GetFilesByIDFolder( LPCITEMIDLIST pIDFolder )
{
    TCHAR szPath [MAX_PATH];
    if( SHGetPathFromIDList( pIDFolder, szPath ) )
    {
        return szPath;
    }
    CString toScan;
    LPSHELLFOLDER desktopFolder = NULL;
    LPSHELLFOLDER pParentFolder = NULL;
    HRESULT hr = S_OK;
    if( FAILED( hr = SHGetDesktopFolder( &desktopFolder ) ) )
    {
        ATLTRACE("Can't get the Desktop folder object.\n");
        return toScan;
    }
    if( FAILED( hr = desktopFolder->BindToObject( pIDFolder,
        NULL, IID_IShellFolder, (LPVOID*)&pParentFolder ) ) )
    {
        ATLTRACE("Can't get the specified object from given CSIDL.\n");
        return toScan;
    }

    if( !pIDFolder || !pParentFolder )
    {
        return toScan;
    }

    LPENUMIDLIST pEnum = NULL;
    SHCONTF flags = SHCONTF_FOLDERS | SHCONTF_INIT_ON_FIRST_NEXT | SHCONTF_STORAGE | SHCONTF_NONFOLDERS | SHCONTF_SHAREABLE;
    HRESULT hRes = pParentFolder->EnumObjects( NULL, flags, &pEnum );
    if( SUCCEEDED( hRes ) )
    {
        LPITEMIDLIST    pidlTemp;
        DWORD           dwFetched = 1;
        //enumerate the item's PIDLs
        while( pEnum->Next( 1, &pidlTemp, &dwFetched ) == S_OK && dwFetched )
        {
            LPITEMIDLIST pidlFQ = ConcatenateItem( pIDFolder, pidlTemp );

            TCHAR szPath [MAX_PATH];
            if( SHGetPathFromIDList( pidlFQ, szPath ) )
            {
                if( toScan.GetLength() > 0 )
                {
                    toScan += ADEFRAG_PATH_DELIMITER;
                }
                toScan + szPath;
            }
        }
    }
    return toScan;
}

LRESULT DefragFiles( LPCTSTR files )
{
    LRESULT hr = ERROR_SUCCESS;
    HWND existingWindow = ::FindWindow( ADEFRAG_APP_WINDOW_CLASS_GUID, NULL );
    if( existingWindow )
    {
        ATLTRACE( _T(">>Shelll extension: ADefrag window is present") );
        USES_CONVERSION;
        CComBSTR pathToDefrag( files );
        COPYDATASTRUCT copyStruct;
        copyStruct.dwData = ADEFRAG_ACTION_DEFRAG_PATHS;
        copyStruct.cbData = (pathToDefrag.Length() + 1)*sizeof(WCHAR);  // size of data
        copyStruct.lpData = pathToDefrag.m_str;
        hr = ::SendMessage( existingWindow, WM_COPYDATA, 0, (LPARAM)(LPVOID)&copyStruct );
        DWORD lastErr = ::GetLastError();
        {
            CString str; str.Format( _T(">>Shelll extension: send message return 0x%lx, last error = 0x%lx"), hr, lastErr );
            ATLTRACE( str );
        }
    }
    else
    {
        // Run executable file
    }
    return hr;
}

//////////////////////////////////////////////////////////////////////////
// IShellExtInit interface implementation

STDMETHODIMP CADefragExt::Initialize( LPCITEMIDLIST pIDFolder, 
    IDataObject *pDataObj, HKEY hRegKey )
{
    // If a data object pointer was passed in, save it and
    // extract the file name. 
    m_szFiles.Empty();
    if( pDataObj )
    {
        STGMEDIUM   medium;
        FORMATETC   fe = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};

        if(SUCCEEDED(pDataObj->GetData(&fe, &medium)))
        {
            // Get the count of files dropped.
            UINT uCount = DragQueryFile((HDROP)medium.hGlobal, (UINT)-1, NULL, 0);

            // Get the first file name from the CF_HDROP.
            for( UINT i = 0; i < uCount; ++i )
            {
                TCHAR szFile[MAX_PATH];
                DragQueryFile( (HDROP)medium.hGlobal, i, szFile, MAX_PATH );
                if( m_szFiles.GetLength() > 0 ) m_szFiles += ADEFRAG_PATH_DELIMITER;
                m_szFiles += szFile;
            }

            ReleaseStgMedium(&medium);
        }
    }

    if( m_szFiles.GetLength() == 0 )
    {
        //?? m_szFiles = GetFilesByIDFolder( pIDFolder );
    }

    return S_OK;
}


//////////////////////////////////////////////////////////////////////////
// IContextMenu interface implementation

STDMETHODIMP CADefragExt::GetCommandString( UINT_PTR idCommand,
    UINT uFlags, LPUINT lpReserved, LPSTR pszName, UINT uMaxNameLen )
{
    HRESULT  hr = E_INVALIDARG;
    CString strName;

    if( idCommand != IDM_ADEFRAG_DEFRAG )
    {
        return hr;
    }

    UINT idHelp = IDS_ADEFRAG_DEFRAG_HELPTEXT;
    UINT idVerb = IDS_ADEFRAG_DEFRAG_VERB;

    switch(uFlags)
    {
    case GCS_HELPTEXTA:
        strName.LoadString( _AtlBaseModule.GetModuleInstance(), idHelp, (WORD)getLanguageID() );
        hr = StringCchCopyNA( pszName, uMaxNameLen, (CStringA)strName, uMaxNameLen );
        break; 

    case GCS_HELPTEXTW:
        strName.LoadString( _AtlBaseModule.GetModuleInstance(), idHelp, (WORD)getLanguageID() );
        hr = StringCchCopyNW( (LPWSTR)pszName, uMaxNameLen, (CStringW)strName, uMaxNameLen );
        break; 

    case GCS_VERBA:
        strName.LoadString( _AtlBaseModule.GetModuleInstance(), idVerb, (WORD)getLanguageID() );
        hr = StringCchCopyNA(pszName, uMaxNameLen, (CStringA)strName, uMaxNameLen);
        break; 

    case GCS_VERBW:
        strName.LoadString( _AtlBaseModule.GetModuleInstance(), idVerb, (WORD)getLanguageID() );
        hr = StringCchCopyNW((LPWSTR)pszName, uMaxNameLen, (CStringW)strName, uMaxNameLen);
        break; 

    default:
        hr = S_OK;
        break; 
    }
    return hr;
}

STDMETHODIMP CADefragExt::InvokeCommand( LPCMINVOKECOMMANDINFO lpcmi )
{
    BOOL fEx = FALSE;
    BOOL fUnicode = FALSE;

    if(lpcmi->cbSize = sizeof(CMINVOKECOMMANDINFOEX))
    {
        fEx = TRUE;
        if((lpcmi->fMask & CMIC_MASK_UNICODE))
        {
            fUnicode = TRUE;
        }
    }

    /*if( !fUnicode && HIWORD(lpcmi->lpVerb))
    {
        if(StrCmpIA(lpcmi->lpVerb, "ADefragcreate"))
        {
            return E_FAIL;
        }
    }

    else if( fUnicode && HIWORD(((CMINVOKECOMMANDINFOEX *) lpcmi)->lpVerbW))
    {
        if(StrCmpIW(((CMINVOKECOMMANDINFOEX *)lpcmi)->lpVerbW, L"ADefragcreate"))
        {
            return E_FAIL;
        }
    }*/

    switch( LOWORD(lpcmi->lpVerb) )
    {
    case IDM_ADEFRAG_DEFRAG:
        {
            HRESULT hr = DefragFiles( m_szFiles );
            if( FAILED(hr) )
            {
                CString strCaption(lpcmi->lpVerb);
                CString strTextError;
                strTextError.LoadString( _AtlBaseModule.GetModuleInstance(), IDS_ADEFRAG_DEFRAG_ERROR, (WORD)getLanguageID() );
                ::MessageBox( lpcmi->hwnd, strTextError, strCaption, MB_OK | MB_ICONERROR );
            }
            return hr;
        }
        break;
    default: return E_FAIL;
    };

    return S_OK;
}

//////////////////////////////////////////////////////////////////////////
// Check the list files
BOOL IsGoodListForDefrag( const CString& list )
{
    return list.GetLength() > 0;
}

//////////////////////////////////////////////////////////////////////////
// Main function for add menu items into context menu
STDMETHODIMP CADefragExt::QueryContextMenu( HMENU hmenu,
    UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags )
{
    if(!(CMF_DEFAULTONLY & uFlags) && m_szFiles.GetLength() > 0 )
    {
        HRESULT hr = MAKE_HRESULT( SEVERITY_SUCCESS, 0, USHORT(0) );
        UINT createdIdx = 0;
        if( IsGoodListForDefrag( m_szFiles ) )
        {
            CString strItemMenu;
            strItemMenu.LoadString( _AtlBaseModule.GetModuleInstance(),
                IDS_ADEFRAG_DEFRAG_MENU, (WORD)getLanguageID() );

            InsertMenu( hmenu, indexMenu + createdIdx, MF_STRING | MF_BYPOSITION | MF_USECHECKBITMAPS,
                idCmdFirst + IDM_ADEFRAG_DEFRAG, strItemMenu );
            if( m_hbmpIconDefrag )
            {
                SetMenuItemBitmaps( hmenu, indexMenu + createdIdx, MF_BYPOSITION, m_hbmpIconDefrag, m_hbmpIconDefrag );
            }
            createdIdx++;
            hr = MAKE_HRESULT( SEVERITY_SUCCESS, 0, USHORT(createdIdx) );
        }

        return hr;
    }
    return MAKE_HRESULT( SEVERITY_SUCCESS, 0, USHORT(0) );
}
