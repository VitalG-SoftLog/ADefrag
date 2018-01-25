/*
    Interface of the CExcludeListOptionsPage class

    Module name:

    ExcludeListOptionsPage.cpp

    Abstract:

    Interface of the CExcludeListOptionsPage class

    $Header: /home/CVS_DEFRAG/Defrag/Src/ADefragUI/Settings/ExcludeListOptionsPage.cpp,v 1.5 2009/12/22 21:15:58 roman Exp $
    $Log: ExcludeListOptionsPage.cpp,v $
    Revision 1.5  2009/12/22 21:15:58  roman
    Minor changes

    Revision 1.4  2009/12/18 00:16:19  roman
    CInspector multiple fragments view, Exclude list remove operations.

    Revision 1.3  2009/12/15 18:51:42  roman
    Add exclude files to empty list.

    Revision 1.2  2009/11/24 18:02:52  roman
    Initial commit

    Revision 1.1  2009/11/24 14:56:45  dimas
    no message

*/

#include "stdafx.h"
#include "ExcludeListOptionsPage.h"
#include "../ADefragUI.h"
#include "../MainFrm.h"
#include "../ADefragUIDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CExcludeListOptionsPage property page

IMPLEMENT_DYNCREATE(CExcludeListOptionsPage, CMFCPropertyPage)

CExcludeListOptionsPage::CExcludeListOptionsPage() : CMFCPropertyPage(CExcludeListOptionsPage::IDD),
    m_ExcludeFiles( FALSE )
{
    //{{AFX_DATA_INIT(CExcludeListOptionsPage)
    //}}AFX_DATA_INIT
}

CExcludeListOptionsPage::~CExcludeListOptionsPage()
{
}

void CExcludeListOptionsPage::DoDataExchange(CDataExchange* pDX)
{
    CMFCPropertyPage::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CExcludeListOptionsPage)
    DDX_Control(pDX, IDC_BANER, m_wndBanner);
    DDX_Check(pDX, IDC_EXCLUDE_FILES, m_ExcludeFiles );
    DDX_Control(pDX, IDC_EXCLUDE_EDIT, m_excludeList);
    DDX_Control(pDX, IDC_EXCLUDE_TREE, m_excludeTree);
    //}}AFX_DATA_MAP
}

// define a custom message 
#define TREE_VIEW_CHECK_STATE_CHANGE (WM_USER + 100)

BEGIN_MESSAGE_MAP(CExcludeListOptionsPage, CMFCPropertyPage)
    //{{AFX_MSG_MAP(CExcludeListOptionsPage)
    ON_WM_SIZE()
    //}}AFX_MSG_MAP
    ON_BN_CLICKED(IDC_EXCLUDE_FILES, &CExcludeListOptionsPage::OnBnClickedExcludeFiles)
    ON_NOTIFY(NM_CLICK, IDC_EXCLUDE_TREE, &CExcludeListOptionsPage::OnNMClickExcludeTree)
    ON_NOTIFY(TVN_KEYDOWN, IDC_EXCLUDE_TREE, &CExcludeListOptionsPage::OnTvnKeydownExcludeTree)
    ON_MESSAGE(TREE_VIEW_CHECK_STATE_CHANGE, OnTreeViewCheckStateChange)
    ON_NOTIFY(TVN_ITEMEXPANDED, IDC_EXCLUDE_TREE, &CExcludeListOptionsPage::OnTvnItemexpandedExcludeTree)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CExcludeListOptionsPage message handlers

void CExcludeListOptionsPage::OnOK()
{
    UpdateData();

    //theApp.WriteInt( _T("Defrag.ExcludeFiles"), m_ExcludeFiles );
    CString excludeString;
    for( int i = 0; i < m_excludeList.GetCount(); ++i )
    {
        excludeString += m_excludeList.GetItemText(i) + EXCLUDE_LIST_DELIMETER;
    }
    //theApp.WriteString( _T("Defrag.ExcludeList"), excludeString );
    excludeString += EXCLUDE_LIST_DELIMETER;

    CADefragUIDoc* doc = GetMainDocument();
    if( doc )
    {
        DWORD isEnabled = m_ExcludeFiles ? 1 : 0;
        if( ERROR_SUCCESS != doc->SetSettingsValue( ExcludeFilesEnableId, &isEnabled, sizeof(isEnabled) ) )
        {
            ASSERT( FALSE );
        }
		// Clear excluded list settings
		if( ERROR_SUCCESS != doc->ClearSettingsValue(ExcludedFilesId))
        {
            ASSERT( FALSE );
        }
        // MULTI_SZ list
        excludeString.Replace( EXCLUDE_LIST_DELIMETER, 0 );
        if( ERROR_SUCCESS != doc->SetSettingsValue( ExcludedFilesId, excludeString.GetBuffer(), excludeString.GetLength() * sizeof(excludeString[0]) ) )
        {
            ASSERT( FALSE );
        }
    }

    CMFCPropertyPage::OnOK();
}

BOOL CExcludeListOptionsPage::OnInitDialog()
{
    CMFCPropertyPage::OnInitDialog();

    // Read settings from service
    CADefragUIDoc* doc = GetMainDocument();
    CString excludeString;
    if( doc )
    {
        DWORD isEnabled = 0;
        if( ERROR_SUCCESS == doc->GetSettingsValue( ExcludeFilesEnableId, &isEnabled, sizeof(isEnabled) ) )
        {
            m_ExcludeFiles = isEnabled ? TRUE : FALSE;
        }
        // MULTI_SZ list
        if( ERROR_SUCCESS == doc->GetSettingsValue( ExcludedFilesId, excludeString.GetBufferSetLength( SERVER_SECTION_SIZE / sizeof(excludeString[0]) ), SERVER_SECTION_SIZE ) )
        {
			if (_tcslen(excludeString)) {
				for( int i = 0; i < excludeString.GetLength() - 1; ++i )
				{
					if( excludeString.GetAt(i) == 0 )
					{
						excludeString.SetAt( i, EXCLUDE_LIST_DELIMETER );
						if( excludeString[i+1] == 0 ) break;
					}
				}
			}
        }
        excludeString.ReleaseBuffer();
    }

    m_excludeList.SetStandardButtons();
    //m_excludeList.EnableBrowseButton();
    while( excludeString.GetLength() > 0 )
    {
        int delimiterPos = excludeString.Find( EXCLUDE_LIST_DELIMETER );
        CString path = delimiterPos >= 0 ? excludeString.Left( delimiterPos ) : excludeString;
        excludeString.Delete( 0, path.GetLength() + 1 );
        m_excludeList.AddItem( path );
    }
    m_excludeList.m_pWndList->SetItemState( 0, 0, LVIS_SELECTED | LVIS_FOCUSED );
    m_excludeTree.EnableShellContextMenu();
    UpdateItemState( m_excludeTree.GetRootItem() );
    m_excludeTree.Expand( m_excludeTree.GetRootItem(), TVE_EXPAND );
    m_excludeList.SetShellTree( &m_excludeTree );
    UpdateData(FALSE);
    OnBnClickedExcludeFiles();
    return TRUE;  // return TRUE unless you set the focus to a control
}

void CExcludeListOptionsPage::OnSize(UINT nType, int cx, int cy)
{
    CMFCPropertyPage::OnSize(nType, cx, cy);

    if (m_wndBanner.GetSafeHwnd() == NULL)
    {
        return;
    }

    CRect rectBanner;
    m_wndBanner.GetWindowRect(rectBanner);

    CRect rectParent;
    GetParent()->GetWindowRect(rectParent);

    m_wndBanner.SetWindowPos(NULL, -1, -1, rectParent.right - rectBanner.left - 10, rectBanner.Height(), SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
}

void CExcludeListOptionsPage::OnBnClickedExcludeFiles()
{
    UpdateData();
    GetDlgItem(IDC_EXCLUDE_EDIT)->EnableWindow( m_ExcludeFiles );
    GetDlgItem(IDC_EXCLUDE_TREE)->EnableWindow( m_ExcludeFiles );
}


void CExcludeListOptionsPage::OnNMClickExcludeTree(NMHDR *pNMHDR, LRESULT *pResult)
{
    HTREEITEM item = NULL;
    UINT flags = 0;
    // verify that we have a mouse click in the check box area 
    DWORD pos = GetMessagePos();
    CPoint point( LOWORD(pos), HIWORD(pos) );
    m_excludeTree.ScreenToClient( &point );
    item = m_excludeTree.HitTest( point, &flags );
    if( item && (flags & TVHT_ONITEMSTATEICON) )
    {
        // handle state change here or post message to another handler 
        // Post message state has changed… 
         PostMessage( TREE_VIEW_CHECK_STATE_CHANGE, 0, (LPARAM)item );
    }

    *pResult = 0;
}

void CExcludeListOptionsPage::OnTvnKeydownExcludeTree(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMTVKEYDOWN pTVKeyDown = reinterpret_cast<LPNMTVKEYDOWN>(pNMHDR);

    // we only want to handle the space key press 
    if( pTVKeyDown->wVKey == VK_SPACE )
    {
        // Determine the selected tree item. 
        HTREEITEM item = m_excludeTree.GetSelectedItem();
        if( item!=NULL )
        {
            // handle state change here or post message to another handler 
            // Post message state has changed… 
            PostMessage( TREE_VIEW_CHECK_STATE_CHANGE, 0, (LPARAM)item );
        }
    }

    *pResult = 0;
}

// implementation for TREE_VIEW_CHECK_STATE_CHANGE message handler 

LRESULT CExcludeListOptionsPage::OnTreeViewCheckStateChange(WPARAM wParam, LPARAM lParam) 
{
    // Handle message here… 
    HTREEITEM changedItem = (HTREEITEM)lParam; 
    int checkState = m_excludeTree.GetCheck(changedItem); 

    // Do something with check state change information… 
    // item was checked
    CString path;
    m_excludeTree.GetItemPath( path, changedItem );
    if( path.GetLength() > 0 )
    {
        path.Replace( _T('/'), _T('\\') );
        if( path.GetAt( path.GetLength() - 1 ) != _T('\\') )
        {
            path += _T("\\");
        }
        if( checkState )
        {
            m_excludeList.AddItem( path + EXCLUDE_LIST_BASEMASK );
        }
        else
        {
            m_excludeList.RemovePath( path );
        }
    }
    else if( checkState )
    {
        m_excludeTree.SetCheck( changedItem, FALSE );
    }
    return 0;
}

//////////////////////////////////////////////////////////////////////////
void CExcludeListOptionsPage::OnTvnItemexpandedExcludeTree(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
    // TODO: Add your control notification handler code here
    if( pNMTreeView && (pNMTreeView->action & (TVE_EXPAND | TVE_EXPANDPARTIAL | TVE_TOGGLE)) != 0 )
    {
        HTREEITEM item = m_excludeTree.GetChildItem( pNMTreeView->itemNew.hItem );
        while( item )
        {
            UpdateItemState( item );
            item = m_excludeTree.GetNextItem( item, TVGN_NEXT );
        }
    }
    *pResult = 0;
}

void CExcludeListOptionsPage::UpdateItemState( HTREEITEM item )
{
    if( !item ) return;
    CString path;
    m_excludeTree.GetItemPath( path, item );
    path.Replace( _T('/'), _T('\\') );
    if( path.GetLength() > 0 && path.GetAt( path.GetLength() - 1 ) != _T('\\') )
    {
        path += _T("\\");
    }
    m_excludeTree.SetCheck( item, path.GetLength() > 0 && m_excludeList.IsPathPresent( path ) );
}

void CExcludeListOptionsPage::UpdateAllTreeItemsState( HTREEITEM item )
{
    if( !item ) item = m_excludeTree.GetRootItem();
    while( item )
    {
        UpdateItemState( item );
        if( m_excludeTree.ItemHasChildren( item ) )
        {
            HTREEITEM child = m_excludeTree.GetChildItem( item );
            if( child ) UpdateAllTreeItemsState( child );
        }
        item = m_excludeTree.GetNextItem( item, TVGN_NEXT );
    }
}

//////////////////////////////////////////////////////////////////////////
// CExcludeListBox class implementation
BOOL CExcludeListBox::RemovePath( const CString& path )
{
    BOOL res = FALSE;
    for( int i = 0; path.GetLength() > 0 && i < GetCount(); ++i )
    {
        if( GetItemPath(i) == path )
        {
            res = RemoveItem( i );
        }
    }
    return res; // not found
} // end of CExcludeListBox::RemoveItem( const CString& path )

BOOL CExcludeListBox::IsPathPresent( const CString& path )
{
    for( int i = 0; path.GetLength() > 0 && i < GetCount(); ++i )
    {
        if( GetItemPath(i) == path )
        {
            return TRUE;
        }
    }
    return FALSE;
}

CString CExcludeListBox::GetItemPath(int index) const
{
    CString path( GetItemText( index ) );
    path.Replace( _T('/'), _T('\\') );
    path.Delete( path.ReverseFind( _T('\\') ) + 1, path.GetLength() );
    return path;
} // end of CString CExcludeListBox::GetItemPath(int index) const

void CExcludeListBox::OnSelectionChanged()
{
    CString path = GetItemPath( GetSelItem() );
    if( m_excludeTree && path.GetLength() > 0 )
    {
        m_excludeTree->SelectPath( path );
    }
}

BOOL CExcludeListBox::RemoveItem(int iIndex)
{
    BOOL res = __super::RemoveItem(iIndex);
    CExcludeListOptionsPage* page = DYNAMIC_DOWNCAST( CExcludeListOptionsPage, GetParent() );
    if( page )
    {
        page->UpdateAllTreeItemsState();
    }
    return res;
}

void CExcludeListBox::OnAfterRenameItem(int /*iItem*/)
{
    CExcludeListOptionsPage* page = DYNAMIC_DOWNCAST( CExcludeListOptionsPage, GetParent() );
    if( page )
    {
        page->UpdateAllTreeItemsState();
    }
}
