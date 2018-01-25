/*
    Implementation of the CSchedulerView class

    Module name:

    SchedulerView.h

    Abstract:

    Implementation of the CSchedulerView class

    $Header: /home/CVS_DEFRAG/Defrag/Src/ADefragUI/Scheduler/SchedulerView.cpp,v 1.2 2009/11/24 18:02:52 roman Exp $
    $Log: SchedulerView.cpp,v $
    Revision 1.2  2009/11/24 18:02:52  roman
    Initial commit

    Revision 1.1  2009/11/24 14:57:02  dimas
    no message

*/

#include "stdafx.h"
#include "../ADefragUIDoc.h"
#include "SchedulerView.h"
#include "../ADefragUI.h"

#include "SchedulePropSheet.h"
#include "ScheduleDrivesPage.h"
#include "ScheduleMethodPage.h"
#include "ScheduleTimePage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CScheduleListCtrl

int CScheduleListCtrl::OnCompareItems(LPARAM lParam1, LPARAM lParam2, int iColumn)
{
    CScheduleTask* task1 = reinterpret_cast<CScheduleTask*>( lParam1 );
    CScheduleTask* task2 = reinterpret_cast<CScheduleTask*>( lParam2 );
    if( !task1 || !task2 ) return 0;
    int listIndex1 = -1;
    int listIndex2 = -1;
    for( int i = 0; i < GetItemCount(); ++i )
    {
        CScheduleTask* task = reinterpret_cast<CScheduleTask*>( GetItemData( i ) );
        if( task == task1 ) listIndex1 = i;
        if( task == task2 ) listIndex2 = i;
    }
    if( listIndex1 == -1 || listIndex2 == -1 ) return 0;
    CString strItem1 = GetItemText((int)(listIndex1 < listIndex2 ? listIndex1 : listIndex2), iColumn);
    CString strItem2 = GetItemText((int)(listIndex1 < listIndex2 ? listIndex2 : listIndex1), iColumn);
    return _tcsicmp(strItem1, strItem2);
}

/////////////////////////////////////////////////////////////////////////////
// CSchedulerView

IMPLEMENT_DYNCREATE(CSchedulerView, CView)

CSchedulerView::CSchedulerView() :
    m_lastUpdateTime( 0 )
{
}

CSchedulerView::~CSchedulerView()
{
}


BEGIN_MESSAGE_MAP(CSchedulerView, CView)
    ON_WM_CONTEXTMENU()
    ON_WM_WINDOWPOSCHANGING()
    ON_WM_CREATE()
    ON_WM_SIZE()
    ON_WM_TIMER()
    // Notify commands
    ON_NOTIFY(NM_DBLCLK, IDC_SCHEDULER_LIST, &CSchedulerView::OnEditItem)
    ON_NOTIFY(LVN_DELETEITEM,IDC_SCHEDULER_LIST,&CSchedulerView::OnDeleteItem)
    // Commands
    ON_UPDATE_COMMAND_UI(ID_SCHEDULER_NEW_ONETIME, &CSchedulerView::OnUpdateNewOneTime)
    ON_COMMAND(ID_SCHEDULER_NEW_ONETIME, &CSchedulerView::OnNewOneTime)
    ON_UPDATE_COMMAND_UI(ID_SCHEDULER_NEW_DAILY, &CSchedulerView::OnUpdateNewDaily)
    ON_COMMAND(ID_SCHEDULER_NEW_DAILY, &CSchedulerView::OnNewDaily)
    ON_UPDATE_COMMAND_UI(ID_SCHEDULER_NEW_WEEKLY, &CSchedulerView::OnUpdateNewWeekly)
    ON_COMMAND(ID_SCHEDULER_NEW_WEEKLY, &CSchedulerView::OnNewWeekly)
    ON_UPDATE_COMMAND_UI(ID_SCHEDULER_NEW_SCREENSAVER, &CSchedulerView::OnUpdateNewScreenSaver)
    ON_COMMAND(ID_SCHEDULER_NEW_SCREENSAVER, &CSchedulerView::OnNewScreenSaver)
    // Run
    ON_UPDATE_COMMAND_UI(ID_SCHEDULER_RUN, &CSchedulerView::OnUpdateRun)
    ON_COMMAND(ID_SCHEDULER_RUN, &CSchedulerView::OnRun)
    // Edit
    ON_UPDATE_COMMAND_UI(ID_SCHEDULER_ENABLE, &CSchedulerView::OnUpdateEnableDisable)
    ON_COMMAND(ID_SCHEDULER_ENABLE, &CSchedulerView::OnEnableDisable)
    ON_UPDATE_COMMAND_UI(ID_SCHEDULER_EDIT, &CSchedulerView::OnUpdateEdit)
    ON_COMMAND(ID_SCHEDULER_EDIT, &CSchedulerView::OnEdit)
    ON_UPDATE_COMMAND_UI(ID_SCHEDULER_DELETE, &CSchedulerView::OnUpdateDelete)
    ON_COMMAND(ID_SCHEDULER_DELETE, &CSchedulerView::OnDelete)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSchedulerView drawing

void CSchedulerView::OnDraw(CDC* /*pDC*/)
{
}

/////////////////////////////////////////////////////////////////////////////
// CSchedulerView diagnostics

#ifdef _DEBUG
void CSchedulerView::AssertValid() const
{
    CView::AssertValid();
}

void CSchedulerView::Dump(CDumpContext& dc) const
{
    CView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CSchedulerView message handlers

BOOL CSchedulerView::PreCreateWindow(CREATESTRUCT& cs) 
{
    //cs.style |= LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS;
    //cs.hMenu = (HMENU)IDC_LISTBOX1;
    return CView::PreCreateWindow(cs);
}

void CSchedulerView::OnInitialUpdate() 
{
    CView::OnInitialUpdate();

    CMFCListCtrl& wndList = GetListCtrl();
    wndList.SetExtendedStyle (LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
    wndList.EnableMultipleSort( TRUE );
    wndList.EnableToolTips( TRUE );

    UINT columsName[e_colCount] = { IDS_SCHEDULER_COL_STATUS, IDS_SCHEDULER_COL_ACTION, IDS_SCHEDULER_COL_TYPE,
        IDS_SCHEDULER_COL_LASTRUN, IDS_SCHEDULER_COL_RUNSTATUS, IDS_SCHEDULER_COL_NEXTRUN };
    int columsWidth[e_colCount] = { 50, 250, 100, 150, 150, 150 };
    int iColumn = 0;

    // Insert columns:
    for (iColumn = 0; iColumn < e_colCount; iColumn++)
    {
        CString strColumn;
        strColumn.LoadString( columsName[iColumn] );
        wndList.InsertColumn( iColumn, strColumn, LVCFMT_LEFT, columsWidth[iColumn] );
    }

    //////////////////////////////////////////////////////////////////////////
    // fill table
    UpdateAllTasks();
    SetTimer( 1, UpdateTimerPeriod, NULL );
}

void CSchedulerView::OnContextMenu(CWnd* /*pWnd*/, CPoint point) 
{
    theApp.ShowPopupMenu( IDR_POPUP_SCHEDULERVIEW, point, this );
    //theApp.GetContextMenuManager()->ShowPopupMenu( IDR_POPUP_SCHEDULERVIEW, point.x, point.y, this );
}

int CSchedulerView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
    if( CView::OnCreate(lpCreateStruct) == -1 )
        return -1;

    // Create the style
    DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_TABSTOP |
        LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS;

    // Create the list control.  Don't worry about specifying
    // correct coordinates.  That will be handled in OnSize()
    BOOL bResult = m_listCtrl.Create( dwStyle, CRect(0,0,0,0),
        this, IDC_SCHEDULER_LIST );

    // Load accelerators
    m_hAccel = ::LoadAccelerators( AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_POPUP_SCHEDULERVIEW) );

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
BOOL CSchedulerView::PreTranslateMessage( MSG* pMsg )
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
void CSchedulerView::OnSize(UINT nType, int cx, int cy) 
{
    CView::OnSize(nType, cx, cy);

    if( ::IsWindow( m_listCtrl.m_hWnd ) )
        m_listCtrl.MoveWindow( 0, 0, cx, cy, TRUE );
}//OnSize

void CSchedulerView::OnWindowPosChanging(WINDOWPOS FAR* lpwndpos) 
{
    CView::OnWindowPosChanging(lpwndpos);
    // Hide horizontal scrollbar:
    ShowScrollBar (SB_HORZ, FALSE);
    ModifyStyle (WS_HSCROLL, 0, SWP_DRAWFRAME);
}

void CSchedulerView::OnEditItem(NMHDR *pNMHDR, LRESULT *pResult)
{
    int Index = GetCurSel();
    if ( Index == -1 )
    {
        return;
    }
    EditSchedule( Index );
    *pResult = 0;
}

void CSchedulerView::OnDeleteItem(NMHDR *pNMHDR, LRESULT *pResult)
{
     NMLISTVIEW* info = (NMLISTVIEW*)pNMHDR;
     CScheduleTask* task = reinterpret_cast<CScheduleTask*>( GetListCtrl().GetItemData( info->iItem ) );
     GetListCtrl().SetItemData( info->iItem, NULL );
     if( task )
     {
         delete task;
     }
    *pResult = 0;
}
//
//
//
CADefragUIDoc* CSchedulerView::GetDocument( void )
{
    ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CADefragUIDoc)));
    return (CADefragUIDoc*)m_pDocument;
}

//
//
//
int CSchedulerView::GetCurSel( void )
{
    CMFCListCtrl& m_myListCtrl = GetListCtrl();
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
void CSchedulerView::SetCurSel( int Index )
{
    CMFCListCtrl& wndList = GetListCtrl();
    wndList.SetFocus();    
    wndList.EnsureVisible( Index, TRUE );
    wndList.SetItemState( Index, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
}

void CSchedulerView::OnUpdateNewOneTime(CCmdUI* pCmdUI)
{
    pCmdUI->Enable( TRUE );
}

//////////////////////////////////////////////////////////////////////////
void CSchedulerView::OnNewOneTime()
{
    NewSchedule( Scheduler::e_PeriodOneTime );
}

//////////////////////////////////////////////////////////////////////////
// Daily Schedule
void CSchedulerView::OnUpdateNewDaily(CCmdUI* pCmdUI)
{
    pCmdUI->Enable( TRUE );
}

void CSchedulerView::OnNewDaily()
{
    NewSchedule( Scheduler::e_PeriodDay );
}

//////////////////////////////////////////////////////////////////////////
// New Weekly Schedule
void CSchedulerView::OnUpdateNewWeekly(CCmdUI* pCmdUI)
{
    pCmdUI->Enable( TRUE );
}

void CSchedulerView::OnNewWeekly()
{
    NewSchedule( Scheduler::e_PeriodWeek );
}

//////////////////////////////////////////////////////////////////////////
// New Screen Saver schedule
void CSchedulerView::OnUpdateNewScreenSaver(CCmdUI* pCmdUI)
{
    pCmdUI->Enable( TRUE );
}

void CSchedulerView::OnNewScreenSaver()
{
    NewSchedule( Scheduler::e_PeriodSceenSaver );
}

// Run
void CSchedulerView::OnUpdateRun(CCmdUI* pCmdUI)
{
    int index = GetCurSel();
    if( index < 0 )
    {
        pCmdUI->Enable( FALSE );
        return;
    }
    CScheduleTask* task = reinterpret_cast<CScheduleTask*>( GetListCtrl().GetItemData( index ) );
    if( !task )
    {
        pCmdUI->Enable( FALSE );
        return;
    }
    pCmdUI->Enable( task->GetDefragMode() != Scheduler::e_DefragModeBoot
        && task->GetTaskStatus() != Scheduler::e_RunStatusInProgress
        && task->GetTaskStatus() != Scheduler::e_RunStatusPaused );
}

void CSchedulerView::OnRun()
{
    int index = GetCurSel();
    ASSERT( index >= 0 );
    CScheduleTask* task = reinterpret_cast<CScheduleTask*>( GetListCtrl().GetItemData( index ) );
    ASSERT( task );
    if( !task ) return;
    GetDocument()->RunSchedulerTask( *task );
    UpdateSheduleTask( index );
}

//////////////////////////////////////////////////////////////////////////
// Edit - modify Status
void CSchedulerView::OnUpdateEnableDisable(CCmdUI* pCmdUI)
{
    pCmdUI->Enable( GetCurSel() >= 0 );
}

void CSchedulerView::OnEnableDisable()
{
    int index = GetCurSel();
    ASSERT( index >= 0 );
    CScheduleTask* task = reinterpret_cast<CScheduleTask*>( GetListCtrl().GetItemData( index ) );
    ASSERT( task );
    if( !task ) return;
    task->SetActive( !task->IsActive() );
    GetDocument()->SetSchedulerTask( *task );
    UpdateSheduleTask( index );
}

//////////////////////////////////////////////////////////////////////////
// Edit selected task
void CSchedulerView::OnUpdateEdit(CCmdUI* pCmdUI)
{
    pCmdUI->Enable( GetCurSel() >= 0 );
}

void CSchedulerView::OnEdit()
{
    ASSERT( GetCurSel() >= 0 );
    EditSchedule( GetCurSel() );
}

//////////////////////////////////////////////////////////////////////////
// Delete selected task
void CSchedulerView::OnUpdateDelete(CCmdUI* pCmdUI)
{
    pCmdUI->Enable( GetCurSel() >= 0 );
}

void CSchedulerView::OnDelete()
{
    int index = GetCurSel();
    ASSERT( index >= 0 );
    CScheduleTask* task = reinterpret_cast<CScheduleTask*>( GetListCtrl().GetItemData( index ) );
    ASSERT( task );
    if( task )
    {
        GetDocument()->RemoveSchedulerTask( *task );
    }
    GetListCtrl().DeleteItem( index );
}

//////////////////////////////////////////////////////////////////////////
void CSchedulerView::NewSchedule( Scheduler::EPeriodType period )
{
    CScheduleTask* task = new CScheduleTask;
    ASSERT( task );
    task->SetPeriod( period );
    CSchedulePropSheet schedulerPropSheet( this );
    schedulerPropSheet.SetTask( task );
    schedulerPropSheet.EnablePageHeader( max(20, afxGlobalData.GetTextHeight() * 2) );
    //schedulerPropSheet.m_psh.dwFlags |= PSH_NOAPPLYNOW;
    schedulerPropSheet.SetLook( CMFCPropertySheet::PropSheetLook_List, 180 );
    schedulerPropSheet.SetWizardMode();

    CScheduleDrivesPage page1;
    CScheduleMethodPage page2;
    CScheduleTimePage page3;
    schedulerPropSheet.AddPage( &page1 );
    schedulerPropSheet.AddPage( &page2 );
    schedulerPropSheet.AddPage( &page3 );

    if( schedulerPropSheet.DoModal() == ID_WIZFINISH )
    {
        GetDocument()->AddSchedulerTask( *task );
        AddSheduleTask( task );
    }
}

//////////////////////////////////////////////////////////////////////////
void CSchedulerView::AddSheduleTask( CScheduleTask* task )
{
    if( !task ) return;

    CMFCListCtrl& list = GetListCtrl();

    int newItem = list.InsertItem ( list.GetItemCount(), _T("") );
    list.SetItemData( newItem, reinterpret_cast<DWORD_PTR>(task) );
    UpdateSheduleTask( newItem );
}

//////////////////////////////////////////////////////////////////////////
void CSchedulerView::UpdateSheduleTask( int index )
{
    CMFCListCtrl& list = GetListCtrl();
    CScheduleTask* task = reinterpret_cast<CScheduleTask*>( list.GetItemData( index ) );
    ASSERT( task );
    if( !task ) return;

    // TODO: optimize
    CScheduleTaskList tasks;
    if( ERROR_SUCCESS == GetDocument()->GetSchedulerTasks( tasks ) && (int)tasks.size() > index )
    {
        *task = tasks[index];
    }

    //e_statusCol, e_actionCol, e_typeCol, e_lastRunCol, e_lastRunStatusCol, e_nextRunCol
    CString status;
    status.LoadString( task->IsActive() ? IDS_TASK_ACTIVE : IDS_TASK_DISABLED );
    list.SetItemText( index, e_statusCol, status );
    list.SetCheck( index, task->IsActive() );

    list.SetItemText( index, e_actionCol, task->GetActionDescription() );
    list.SetItemText( index, e_typeCol, task->GetTypeDescription() );
    CTime lastRun( task->GetLastRunTime() );
    list.SetItemText( index, e_lastRunCol, lastRun != CTime() ? COleDateTime( lastRun.GetTime() ).Format() : _T("") );
    list.SetItemText( index, e_lastRunStatusCol, task->GetLastRunStatus() );
    CTime nextRun( task->GetNextRunTime() );
    list.SetItemText( index, e_nextRunCol, nextRun != CTime() ? COleDateTime( nextRun.GetTime() ).Format() : _T("") );
}

//////////////////////////////////////////////////////////////////////////
void CSchedulerView::EditSchedule( int index )
{
    CMFCListCtrl& list = GetListCtrl();
    CScheduleTask* task = reinterpret_cast<CScheduleTask*>( list.GetItemData( index ) );
    ASSERT( task );
    if( !task ) return;

    CScheduleTask taskToEdit( *task );
    CSchedulePropSheet schedulerPropSheet( this );
    schedulerPropSheet.SetTask( &taskToEdit );
    schedulerPropSheet.EnablePageHeader( max(20, afxGlobalData.GetTextHeight() * 2) );
    schedulerPropSheet.SetLook( CMFCPropertySheet::PropSheetLook_List, 180 );
    schedulerPropSheet.SetWizardMode();

    CScheduleDrivesPage page1;
    CScheduleMethodPage page2;
    CScheduleTimePage page3;
    schedulerPropSheet.AddPage( &page1 );
    schedulerPropSheet.AddPage( &page2 );
    schedulerPropSheet.AddPage( &page3 );

    if( schedulerPropSheet.DoModal() == ID_WIZFINISH )
    {
        *task = taskToEdit;
        GetDocument()->SetSchedulerTask( *task );
        UpdateSheduleTask( index );
    }
}

//////////////////////////////////////////////////////////////////////////
void CSchedulerView::UpdateAllTasks()
{
    CMFCListCtrl& list = GetListCtrl();

    // get Tasks List from service
    CScheduleTaskList tasks;
    GetDocument()->GetSchedulerTasks( tasks );
    int count = (int)tasks.size();
    for( int i = 0; i < count; ++i )
    {
        if( i < list.GetItemCount() )
        {
            UpdateSheduleTask( i );
        }
        else
        {
            CScheduleTask* task = new CScheduleTask;
            if( task ) *task = tasks[i];
            AddSheduleTask( task );
        }
    }
    m_lastUpdateTime = _time64( NULL );
}

//
//  WM_TIMER processing
//
void CSchedulerView::OnTimer(UINT_PTR NIDEvent)
{
    DWORD lastUpdateTime = 0;
    GetDocument()->GetSettingsValue( SchedulerLastUpdateTimeId, &lastUpdateTime, sizeof(lastUpdateTime) );
    if( lastUpdateTime == 0 || m_lastUpdateTime == 0 || lastUpdateTime > (DWORD)m_lastUpdateTime )
    {
        UpdateAllTasks();
    }
}
