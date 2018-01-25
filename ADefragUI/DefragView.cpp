/*
    Defrag CDefragView class module 

    Module name:

        DefragView.cpp

    Abstract:

        Contains implementation of the CDefragView class.

    $Header: /home/CVS_DEFRAG/Defrag/Src/ADefragUI/DefragView.cpp,v 1.4 2009/12/24 01:39:10 roman Exp $
    $Log: DefragView.cpp,v $
    Revision 1.4  2009/12/24 01:39:10  roman
    fix errors

    Revision 1.3  2009/12/03 15:36:55  dimas
    CVS headers included

*/

#include "stdafx.h"
#include "ADefragUIDoc.h"
#include "DefragView.h"
#include "ADefragUI.h"
#include "DriveMap.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDefragView

#define IDT_DEFRAG_VIEW_TIMER       1
#define DEFRAG_VIEW_TIMER_ELAPSE    500    // each 0,5 sec

IMPLEMENT_DYNCREATE(CDefragView, CBaseListView)

CDefragView::CDefragView()
{
}

CDefragView::~CDefragView()
{
}

#define WM_FILECHANGEINFO       (WM_USER+10)

BEGIN_MESSAGE_MAP(CDefragView, CBaseListView)
    //ON_WM_KEYDOWN()
    ON_WM_CONTEXTMENU()
    //ON_WM_TIMER()
    ON_WM_DESTROY()
    //ON_WM_LBUTTONDBLCLK()
    //ON_NOTIFY_REFLECT(NM_CLICK, &CDefragView::OnNMClick)
    // Notify commands
    ON_NOTIFY(NM_CLICK, IDC_BASE_LIST, &CDefragView::OnNMClick)
    ON_NOTIFY(NM_DBLCLK, IDC_BASE_LIST, &CDefragView::OnLButtonDblClk)
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_BASE_LIST, &CDefragView::OnActiveDiskSelChanged)
    //ON_MESSAGE(WM_FILECHANGEINFO, OnFileChangeInfo)
    ON_MESSAGE(WM_DEVICECHANGE, OnFileChangeInfo)
    // Commands
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDefragView diagnostics

#ifdef _DEBUG
void CDefragView::AssertValid() const
{
    __super::AssertValid();    
}

void CDefragView::Dump(CDumpContext& dc) const
{
    __super::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CDefragView message handlers

void CDefragView::OnInitialUpdate() 
{
    __super::OnInitialUpdate();

    CBaseListCtrl& wndList = GetListCtrl ();
    wndList.SetExtendedStyle (LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

    //
    // TODO: Columns header -> String table
    //
    const int nColumns = drivelist_col_count;
    UINT columsName[nColumns] = { 
        IDS_DRIVELIST_COL_NAME,
        IDS_DRIVELIST_COL_TYPE,
        IDS_DRIVELIST_COL_STATUS,
        IDS_DRIVELIST_COL_LABEL,
        IDS_DRIVELIST_COL_FILESYS,
        IDS_DRIVELIST_COL_SIZE,
        IDS_DRIVELIST_COL_FREE,
        IDS_DRIVELIST_COL_LAST,
        IDS_DRIVELIST_COL_FRAGMENTED,
     };
    int columsWidth[nColumns] = { 50, 70, 150, 70, 70, 120, 120, 120, 120 };
    int iColumn = 0;

    //
    // Insert columns:
    //
    for (iColumn = 0; iColumn < nColumns; iColumn++)
    {
        CString strColumn;
        strColumn.LoadString( columsName[iColumn] );
        int width = columsWidth[iColumn];
        width = theApp.GetInt( strColumn + _T("Width"), width );
        wndList.InsertColumn( iColumn, strColumn, LVCFMT_LEFT, width );
    }

    UpdateDrivesList();

    PrevPos = 0;
    if( wndList.GetItemCount() > 0 )
    {
        while( PrevPos + 1 < wndList.GetItemCount() && wndList.GetItemText( GetListIndex(PrevPos), drivelist_col_status ).GetLength() == 0 )
        {
            PrevPos++;
        }

        SetCurSel( PrevPos );
        GetDocument()->SetCurrentItem( static_cast<int>( GetListCtrl().GetItemData( PrevPos ) ) );
    }

    //SetTimer( IDT_DEFRAG_VIEW_TIMER, DEFRAG_VIEW_TIMER_ELAPSE, NULL );
}

void CDefragView::UpdateDrivesList()
{
    CBaseListCtrl& wndList = GetListCtrl();
    //
    // Enum drives    
    //
    CString strDrvRoot;
    DriveInfoMap& drvInfo = GetDocument()->GetDisksInfo();

    for ( int i = 0; i < (int)drvInfo.size(); i++ )
    {

        strDrvRoot.Format( _T("%s\\"), drvInfo[i].diskStatus.Name );
        int iItem = GetListIndex(i);
        if( iItem == -1 )
        {
            iItem = wndList.InsertItem( i, strDrvRoot, 0 );
            wndList.SetItemData( iItem, static_cast<DWORD_PTR>( i ) );
        }
        wndList.SetItemText ( iItem, drivelist_col_name, strDrvRoot );
        wndList.SetItemText ( iItem, drivelist_col_type, drvInfo[i].diskStatus.Type );
        wndList.SetItemText ( iItem, drivelist_col_label, drvInfo[i].diskStatus.Label );
        wndList.SetItemText ( iItem, drivelist_col_filesys, drvInfo[i].diskStatus.FileSystem );
        wndList.SetItemText ( iItem, drivelist_col_size, drvInfo[i].diskStatus.Size );
        wndList.SetItemText ( iItem, drivelist_col_free, drvInfo[i].diskStatus.Free );

        if( drvInfo[i].LastDefragTime != 0 )
        {
            COleDateTime date( drvInfo[i].LastDefragTime );
            wndList.SetItemText (iItem, drivelist_col_last, date.Format() ); //TODO
        }
        else
        {
            wndList.SetItemText (iItem, drivelist_col_last, L"");
        }

        CString framentationInfo;
        if( drvInfo[i].IsAnalysed )
        {
            framentationInfo.Format( _T("%s%%"), drvInfo[i].GetFragmentationRatio() );
        }
        else if( drvInfo[i].FAnalyzeBtnState )
        {
            framentationInfo.LoadString( IDS_NOT_ANALYSED_YET );
        }
        wndList.SetItemText (iItem, drivelist_col_fragmented, framentationInfo );
    }
    if( GetCurSel() >= (int)drvInfo.size() )
        SetCurSel( drvInfo.size() - 1 );
    while( wndList.GetItemCount() > (int)drvInfo.size() )
    {
        wndList.DeleteItem( wndList.GetItemCount() - 1 );
    }
    UpdateDrivesListStatus();
}

void CDefragView::UpdateDrivesListStatus()
{
    CBaseListCtrl& wndList = GetListCtrl();

    CString Str;
	CString framentationInfo;

    DriveInfoMap& drvInfo = GetDocument()->GetDisksInfo();
    //wndList.Get

    for ( int i = 0; i < (int)drvInfo.size(); i++ )
    {
        if ( GetDocument()->JobInProgress( i )  && drvInfo[i].CurrentJobCommand != 0 )
        {
            Str.Format( _T("%s - %i%%"),
                CADefragUIDoc::GetCommandName( drvInfo[i].CurrentJobCommand ),
                drvInfo[i].diskStatus.Progress );
        }
        else 
        {
            Str.Empty();
            if( drvInfo[i].FAnalyzeBtnState )
            {
                Str.LoadString( IDS_STATE_READY );
            }
            drvInfo[i].diskStatus.Progress = 101;
        }

        if ( Str != wndList.GetItemText( GetListIndex(i), drivelist_col_status ) )
            wndList.SetItemText ( GetListIndex(i), drivelist_col_status, Str );

		if (drvInfo[i].IsAnalysed) {
			framentationInfo.Format( _T("%s%%"), drvInfo[i].GetFragmentationRatio() );

			if ( framentationInfo != wndList.GetItemText( GetListIndex(i), drivelist_col_fragmented ) )
				wndList.SetItemText (GetListIndex(i), drivelist_col_fragmented, framentationInfo );
		}

        Str = _T("");
    }

    int activeIdx = GetCurSel();
    if( activeIdx >= 0 )
    {
        CString info;
        CString status = wndList.GetItemText( activeIdx, drivelist_col_status );
        if( status.GetLength() > 0 )
        {
            info.Format( _T("%s - %s"),
                wndList.GetItemText( activeIdx, drivelist_col_name ),
                status );
        }
        GetMainFrm()->UpdateStatusAndProgress( info,
            drvInfo[activeIdx].diskStatus.Progress );
    }
}

//
// Show context menu
//
void CDefragView::OnContextMenu( CWnd* pWnd, CPoint point) 
{    
    int Index = GetCurSel();
    if ( Index == -1 )
    {
        SetCurSel(PrevPos);
    }

    __super::OnContextMenu( pWnd, point );
}

//
// Cursor key processing
//
/******************************************************************************
*
*      Function :       OnActiveDiskSelChanged
*
*      Purpose :        Called by the framework when a new item in the call list is 
*               selected
*
*      Return :         None
*
*      Parameters : NMHDR* pNMHDR    - pointer to the new selected item        
*                               LRESULT* pResult - not used
*
*******************************************************************************/
void CDefragView::OnActiveDiskSelChanged(NMHDR *pNMHDR, LRESULT *pResult )
{
    LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
    if( pNMLV && (pNMLV->uNewState & LVIS_FOCUSED) == LVIS_FOCUSED || pNMLV->uNewState == LVIS_SELECTED )
    {
        //Create a pointer to the item
        int Index = pNMLV->iItem;
        PrevPos = Index;
        GetDocument()->SetCurrentItem( static_cast<int>( GetListCtrl().GetItemData( Index ) ) );
    }
    if( pResult ) *pResult = 0;
}

//
//
//
void CDefragView::OnLButtonDblClk( NMHDR *pNMHDR, LRESULT *pResult )
{
    //OnLButtonUp( nFlags, point );
    int Index = GetCurSel();
    if ( Index == -1 )
    {
        SetCurSel(PrevPos);
    }
}

//
// Item click
//
void CDefragView::OnNMClick(NMHDR *pNMHDR, LRESULT *pResult)
{
    int Index = GetCurSel();
    if ( Index == -1 )
    {
        SetCurSel(PrevPos);
    }
    else
    {
        // TODO: Send index to document
        //GetDocument()->SetCurrentDrv(L"");
        PrevPos = Index;
        GetDocument()->SetCurrentItem( static_cast<int>( GetListCtrl().GetItemData( Index ) ) );
    }
}


void CDefragView::OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/)
{
    //??GetDocument()->UpdateDrivesInfo();
    UpdateDrivesList();
}

void CDefragView::OnDestroy()
{
    __super::OnDestroy();
}

int CDefragView::GetListIndex( int driveIndex )
{
    for( int i = 0; i < GetListCtrl().GetItemCount(); ++i )
    {
        if( GetListCtrl().GetItemData( i ) == driveIndex )
        {
            return i;
        }
    }
    return -1;
}

LRESULT CDefragView::OnFileChangeInfo(WPARAM /* wParam */, LPARAM lParam)
{
    //PDEV_BROADCAST_HDR lpdb = (PDEV_BROADCAST_HDR)lParam;
    GetDocument()->UpdateDrivesInfo();
    UpdateDrivesList();
    return 0;
}
