/*
    Interface of the CExcludeListOptionsPage class

    Module name:

    ExcludeListOptionsPage.h

    Abstract:

    Interface of the CExcludeListOptionsPage class

    $Header: /home/CVS_DEFRAG/Defrag/Src/ADefragUI/Settings/ExcludeListOptionsPage.h,v 1.2 2009/11/24 18:02:52 roman Exp $
    $Log: ExcludeListOptionsPage.h,v $
    Revision 1.2  2009/11/24 18:02:52  roman
    Initial commit

    Revision 1.1  2009/11/24 14:56:45  dimas
    no message

*/

#ifndef __EXCLUDELISTOPTIONSPAGE_H__
#define __EXCLUDELISTOPTIONSPAGE_H__

#pragma once

#include "DlgBanner.h"
#include "resource.h"

#define EXCLUDE_LIST_DELIMETER       _T(';')
#define EXCLUDE_LIST_BASEMASK        _T("*.*")

//////////////////////////////////////////////////////////////////////////
//
class CExcludeListBox : public CVSListBox
{
    // Construction
public:
    CExcludeListBox() : m_excludeTree( NULL ) {}
    void SetShellTree( CMFCShellTreeCtrl* tree ) { m_excludeTree = tree; }

public: // Overwrites
    virtual BOOL RemovePath( const CString& path );
    virtual BOOL IsPathPresent( const CString& path );
    virtual CString GetItemPath(int iIndex) const;
    virtual BOOL RemoveItem(int iIndex);;

    virtual void OnSelectionChanged();
    virtual void OnAfterRenameItem(int /*iItem*/);

protected:
    CMFCShellTreeCtrl* m_excludeTree;
};

class CExcludeShellTreeCtrl: public CMFCShellTreeCtrl
{
};

/////////////////////////////////////////////////////////////////////////////
// CExcludeListOptionsPage dialog

class CExcludeListOptionsPage : public CMFCPropertyPage
{
    DECLARE_DYNCREATE(CExcludeListOptionsPage)

// Construction
public:
    CExcludeListOptionsPage();
    ~CExcludeListOptionsPage();

    // Dialog Data
    //{{AFX_DATA(CExcludeListOptionsPage)
    enum { IDD = IDD_EXCLUDE_PAGE };
    //}}AFX_DATA

    void UpdateAllTreeItemsState( HTREEITEM item = NULL );

    // Overrides
public:
    virtual void OnOK();
protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    // Implementation
protected:
    //{{AFX_MSG(CExcludeListOptionsPage)
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnBnClickedExcludeFiles();
    afx_msg void OnNMClickExcludeTree(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnTvnKeydownExcludeTree(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg LRESULT OnTreeViewCheckStateChange(WPARAM wParam, LPARAM lParam);
    afx_msg void OnTvnItemexpandedExcludeTree(NMHDR *pNMHDR, LRESULT *pResult);
    //}}AFX_MSG

    DECLARE_MESSAGE_MAP()

    virtual BOOL OnInitDialog();
    void UpdateItemState( HTREEITEM item );

protected:
    CDlgBanner m_wndBanner;
    BOOL m_ExcludeFiles;
    CExcludeListBox m_excludeList;
    CExcludeShellTreeCtrl m_excludeTree;
};

#endif // __EXCLUDELISTOPTIONSPAGE_H__
