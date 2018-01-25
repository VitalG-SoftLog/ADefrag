/******************************************************************************
*
* $Workfile: ADefragExt.h $
*
* Project  : ADefragShellExt
*
* Purpose  : Defines the CADefragExt context menu class
*
* Autor: Oleg Shaposhnikov
*
******************************************************************************/
#ifndef _ADEFRAGEXT_H
#define _ADEFRAGEXT_H

#pragma once
#include "resource.h"       // main symbols

// CADefragExt
[
    coclass,
    threading( apartment ),
    vi_progid("ADefragShellExt.ADefragExt"),
    progid("ADefragShellExt.ADefragExt.1"),
    version(1.0),
    uuid("8C30CF78-53F1-45a1-B8B2-DE2D80E70093"),
    helpstring("ADefragExt Class")
]
class ATL_NO_VTABLE CADefragExt : //public CComObjectRootEx<CComSingleThreadModel>,
    public IShellExtInit,
    public IContextMenu
{
public:
    CADefragExt();
    ~CADefragExt();

public:
    BEGIN_COM_MAP(CADefragExt)
        COM_INTERFACE_ENTRY(IShellExtInit)
        COM_INTERFACE_ENTRY_IID( IID_IContextMenu, IContextMenu )
    END_COM_MAP()

    DECLARE_PROTECT_FINAL_CONSTRUCT()
    HRESULT FinalConstruct();
    void FinalRelease();

// IShellExtInit
public:
    STDMETHOD(Initialize)(LPCITEMIDLIST pidlFolder, IDataObject* pdtobj, HKEY hkeyProgID);

// IContextMenu
public:
    STDMETHOD(GetCommandString)( UINT_PTR idCmd,
        UINT uFlags, UINT *pwReserved, LPSTR pszName, UINT cchMax );
    STDMETHOD(InvokeCommand)( LPCMINVOKECOMMANDINFO pici );
    STDMETHOD(QueryContextMenu)( HMENU hmenu,
        UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags );

protected:
    HBITMAP m_hbmpIconDefrag;   // bitmap icon for context menu item "Defrag"
    CString m_szFiles;    // The file name
};

#endif

/******************************************************************************
*
* End of $Workfile: ADefragExt.h $
* 
******************************************************************************/
