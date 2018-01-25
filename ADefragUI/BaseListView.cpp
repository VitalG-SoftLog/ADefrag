/*
    Implementation of the CBaseListView class

    Module name:

    BaseListView.h

    Abstract:

    Implementation of the CBaseListView class

    $Header: /home/CVS_DEFRAG/Defrag/Src/ADefragUI/BaseListView.cpp,v 1.3 2009/11/24 18:02:52 roman Exp $
    $Log: BaseListView.cpp,v $
    Revision 1.3  2009/11/24 18:02:52  roman
    Initial commit

    Revision 1.2  2009/11/24 17:43:24  roman
    Initial commit

*/

#include "stdafx.h"
#include "ADefragUIDoc.h"
#include "BaseListView.h"
#include "ADefragUI.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CScheduleListCtrl

int CBaseListCtrl::OnCompareItems(LPARAM lParam1, LPARAM lParam2, int iColumn)
{
    DWORD_PTR task1 = lParam1;
    DWORD_PTR task2 = lParam2;
    if( !task1 && !task2 ) return 0;
    int listIndex1 = -1;
    int listIndex2 = -1;
    for( int i = 0; i < GetItemCount(); ++i )
    {
        DWORD_PTR task = GetItemData( i );
        if( task == task1 ) listIndex1 = i;
        if( task == task2 ) listIndex2 = i;
    }
    if( listIndex1 == -1 || listIndex2 == -1 ) return 0;
    CString strItem1 = GetItemText((int)(listIndex1 < listIndex2 ? listIndex1 : listIndex2), iColumn);
    CString strItem2 = GetItemText((int)(listIndex1 < listIndex2 ? listIndex2 : listIndex1), iColumn);
    return _tcsicmp(strItem1, strItem2);
}

/////////////////////////////////////////////////////////////////////////////
// CBaseListView

IMPLEMENT_DYNCREATE(CBaseListView, CView)

CBaseListView::CBaseListView()
{
}

CBaseListView::~CBaseListView()
{
}


BEGIN_MESSAGE_MAP(CBaseListView, CView)
    ON_WM_CONTEXTMENU()
    ON_WM_WINDOWPOSCHANGING()
    ON_WM_CREATE()
    ON_WM_SIZE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBaseListView drawing

void CBaseListView::OnDraw(CDC* /*pDC*/)
{
}

/////////////////////////////////////////////////////////////////////////////
// CBaseListView diagnostics

#ifdef _DEBUG
void CBaseListView::AssertValid() const
{
    CView::AssertValid();
}

void CBaseListView::Dump(CDumpContext& dc) const
{
    CView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CBaseListView message handlers

BOOL CBaseListView::PreCreateWindow(CREATESTRUCT& cs) 
{
    //cs.style |= LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS;
    //cs.hMenu = (HMENU)IDC_LISTBOX1;
    return CView::PreCreateWindow(cs);
}

void CBaseListView::OnInitialUpdate() 
{
    CView::OnInitialUpdate();

    CBaseListCtrl& wndList = GetListCtrl();
    wndList.SetExtendedStyle (LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
    wndList.EnableMultipleSort( FALSE );
    wndList.EnableToolTips( TRUE );
    InitialUpdateList();
}

void CBaseListView::OnContextMenu(CWnd* /*pWnd*/, CPoint point) 
{
    theApp.ShowPopupMenu( GetContextMenuID(), point, this );
    //theApp.GetContextMenuManager()->ShowPopupMenu( IDR_POPUP_BaseListVIEW, point.x, point.y, this );
}

int CBaseListView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
    if( CView::OnCreate(lpCreateStruct) == -1 )
        return -1;

    // Create the style
    DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_TABSTOP |
        LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS;

    // Create the list control.  Don't worry about specifying
    // correct coordinates.  That will be handled in OnSize()
    BOOL bResult = m_listCtrl.Create( dwStyle, CRect(0,0,0,0),
        this, IDC_BASE_LIST );

    // Load accelerators
    m_hAccel = ::LoadAccelerators( AfxGetInstanceHandle(), MAKEINTRESOURCE(GetAcceleratorID()) );

    return (bResult ? 0 : -1);
}

/******************************************************************************
*
*       Function :       PreTranslateMessage
*
*       Purpose :        Called by the framework when some mesages should be dispatched
*
*       Return :         Always TRUE
*
*       Parameters : MSG* pMsg    - message for dispatching
*
*******************************************************************************/
BOOL CBaseListView::PreTranslateMessage( MSG* pMsg )
{
    // If press "Enter" or "Esc" make process message
    if( (pMsg->message==WM_KEYDOWN) && (pMsg->wParam==VK_RETURN || pMsg->wParam==VK_ESCAPE) )
    {
        ::TranslateMessage(pMsg);
        ::DispatchMessage(pMsg);
        return TRUE;
    }

    // Check own accelerator and translate messages for it.
    if( m_hAccel != NULL && ::TranslateAccelerator( GetSafeHwnd(), m_hAccel, pMsg ) )
    {
        return TRUE;
    }

    // Translate message by parent handler
    return __super::PreTranslateMessage( pMsg );
}

// Override OnSize to resize the control to match the view
void CBaseListView::OnSize(UINT nType, int cx, int cy) 
{
    CView::OnSize(nType, cx, cy);

    if( ::IsWindow( m_listCtrl.m_hWnd ) )
        m_listCtrl.MoveWindow( 0, 0, cx, cy, TRUE );
}//OnSize

void CBaseListView::OnWindowPosChanging(WINDOWPOS FAR* lpwndpos) 
{
    CView::OnWindowPosChanging(lpwndpos);
    // Hide horizontal scrollbar:
    ShowScrollBar (SB_HORZ, FALSE);
    ModifyStyle (WS_HSCROLL, 0, SWP_DRAWFRAME);
}

//
CADefragUIDoc* CBaseListView::GetDocument( void )
{
    ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CADefragUIDoc)));
    return (CADefragUIDoc*)m_pDocument;
}

//
//
//
int CBaseListView::GetCurSel( void )
{
    CBaseListCtrl& m_myListCtrl = GetListCtrl();
    POSITION pos = m_myListCtrl.GetFirstSelectedItemPosition();
    if (pos != NULL)
    {
      return m_myListCtrl.GetNextSelectedItem(pos);
    }

    return -1;
}

//
// Set select
//
void CBaseListView::SetCurSel( int Index )
{
    CBaseListCtrl& wndList = GetListCtrl();
    wndList.SetFocus();
    wndList.EnsureVisible( Index, TRUE );
    wndList.SetItemState( Index, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
}
