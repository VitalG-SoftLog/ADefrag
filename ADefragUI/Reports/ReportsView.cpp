// Reports\ReportsView.cpp : implementation file
//

#include "stdafx.h"
//#include "..\ADefragUI.h"
#include "ReportsView.h"
//#include "..\DriveMap.h"
#include "Reports.h"
#include "..\Resource.h"
#include "..\MainFrm.h"
#include "..\Ctrls\SSSDHtmlView.h"

#include "ReportSystemView.h"
#include "ReportFragmentedView.h"
#include "ReportDriveHealthView.h"
#include "ReportFileOrganizationView.h"
#include "ReportTop100View.h"
#include "ReportExcludedFilesView.h"

#include "..\ADefragUIDoc.h"
#include "..\local_strings.h"


// CReportsView
#define VER_SUITE_WH_SERVER     0x00008000

static const CString strUnknownInfo = _T("Unknown");

IMPLEMENT_DYNCREATE(CReportsView, CView)

CReportsView::CReportsView()
{
    m_nCurReportSel = -1;
}

CReportsView::~CReportsView()
{
}

BEGIN_MESSAGE_MAP(CReportsView, CView)
    ON_WM_CREATE()
    ON_WM_SIZE()
    ON_LBN_SELCHANGE(idList, &CReportsView::OnChangeReport)
    ON_WM_ERASEBKGND()
    ON_WM_SHOWWINDOW()
    ON_COMMAND(ID_REPORT_SAVE, &CReportsView::OnReportSave)
    ON_COMMAND(ID_REPORT_PRINT, &CReportsView::OnReportPrint)
    ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CReportsView::OnReportPrintPreview)
    ON_UPDATE_COMMAND_UI(ID_REPORT_SAVE, &CReportsView::OnUpdateReportCommand)
    ON_UPDATE_COMMAND_UI(ID_REPORT_PRINT, &CReportsView::OnUpdateReportCommand)
    ON_UPDATE_COMMAND_UI(ID_FILE_PRINT_SETUP, &CReportsView::OnUpdateReportCommand)
    ON_UPDATE_COMMAND_UI(ID_FILE_PRINT_PREVIEW, &CReportsView::OnUpdateReportCommand)
END_MESSAGE_MAP()


// CReportsView drawing

void CReportsView::OnDraw(CDC* pDC)
{
	//CADefragUIDoc* pDoc = GetDocument();
	// TODO: add draw code here

    //DrawDiagramToBmp( pDC->m_hDC );

    Gdiplus::Font fontSmall( L"Arial", 10, FontStyleBold );
    Gdiplus::Font fontBig( L"Arial", 14, FontStyleBold );

    Gdiplus::SolidBrush textBrush(Color(0, 0, 0));
    Gdiplus::SolidBrush textBrushRed(Color(128, 0, 0));
    Gdiplus::StringFormat stringFormat;

    stringFormat.SetAlignment(StringAlignmentNear);    
    stringFormat.SetLineAlignment(StringAlignmentNear);

    RECT rc;
    ::GetClientRect(m_hWnd, &rc);
    //
    // Create Window Graphics object
    //
    Graphics wndGraphics(pDC->m_hDC);

    //
    // Create temporary Graphics object
    //
    Bitmap backBuffer(rc.right, rc.bottom, &wndGraphics);    
    Graphics temp(&backBuffer);

    //
    // Create background brush
    //
    SolidBrush bgBrush( Color( Global.ColorManager.COLORREFtoARGB( Global.ColorManager.GetSchemeColor( CSSSColorManager::BGColor ) ) ) );
    
    //
    // Draw background
    //
    temp.FillRectangle(&bgBrush, rc.left, rc.top, rc.right, rc.bottom);

    //
    // Copy temporary to Window
    //
    wndGraphics.DrawImage(&backBuffer, 0, 0, 0, 0, rc.right, rc.bottom, UnitPixel);

}


// CReportsView diagnostics

#ifdef _DEBUG
void CReportsView::AssertValid() const
{
	CView::AssertValid();
}

#ifndef _WIN32_WCE
void CReportsView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif
#endif //_DEBUG


// CReportsView message handlers

int CReportsView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CView::OnCreate(lpCreateStruct) == -1)
        return -1;
    
	const DWORD dwListStyle = LBS_OWNERDRAWFIXED | LBS_HASSTRINGS | LBS_NOINTEGRALHEIGHT | WS_CHILD | WS_VISIBLE | LBS_NOTIFY;    
	m_wndList.Create( dwListStyle, CRect( 0, 0, 0, 0 ), this, (UINT) idList );

    //
    // Set extended style
    //
    m_wndList.ModifyStyleEx(0, WS_EX_CLIENTEDGE);

    //
    // Set report list
    //
    m_wndList.AddString( CLocalString(IDS_REPORT_DRIVE_INFO) );
    m_wndList.AddString( CLocalString(IDS_REPORT_FRAGMENTATION) );
    m_wndList.AddString( CLocalString(IDS_REPORT_DRIVE_HEALTH) );
    m_wndList.AddString( CLocalString(IDS_REPORT_FILE_ORGANIZATION) );
    m_wndList.AddString( CLocalString(IDS_REPORT_100_FRAGMENTED) );
    m_wndList.AddString( CLocalString(IDS_REPORT_EXCLUDED) );

    for ( int i = 0; i < ReportsCount; i++ )
    {
        ReportsWnd[ i ] = NULL;
    }

    return 0;
}

//
//
//
CSSSDHtmlView* CReportsView::CreateView( CRuntimeClass* pViewClass )
{
    CCreateContext* pContext            = NULL;

    ASSERT_VALID(this);
    ENSURE(pViewClass != NULL);
    ENSURE(pViewClass->IsDerivedFrom(RUNTIME_CLASS(CView)));

    CView* pView = DYNAMIC_DOWNCAST(CView, pViewClass->CreateObject());
    ASSERT_VALID(pView);

    if (!pView->Create( NULL, _T(""), WS_CHILD | WS_VISIBLE, CRect(0, 0, 0, 0), this, (UINT) -1, pContext))
    {
        TRACE1("CReportsView:Failed to create view '%s'\n", pViewClass->m_lpszClassName);
        return NULL;
    }

    CDocument* pDoc = GetDocument();
    if (pDoc != NULL)
    {
        ASSERT_VALID(pDoc);

        BOOL bFound = FALSE;
        for (POSITION pos = pDoc->GetFirstViewPosition(); !bFound && pos != NULL;)
        {
            if (pDoc->GetNextView(pos) == pView)
            {
                bFound = TRUE;
            }
        }

        if (!bFound)
        {
            pDoc->AddView(pView);
        }
    }

    pView->ModifyStyleEx(0, WS_EX_CLIENTEDGE);

    return (CSSSDHtmlView *)pView;
}

//
//
//
void CReportsView::OnInitialUpdate()
{
    CView::OnInitialUpdate();    

    //
    // Create reports windows
    //
    ReportsWnd[ PosReportSystem ]               = CreateView( RUNTIME_CLASS( CReportSystemView ) );
    ReportsWnd[ PosReportFragmentation ]        = CreateView( RUNTIME_CLASS( CReportFragmentedView ) );

    ReportsWnd[ PosReportHeals ]                = CreateView( RUNTIME_CLASS( CReportDriveHealthView ) );
    //ReportsWnd[ PosReportHeals ]->LoadFromResource( IDR_DRIVE_HEALTH_REPORT_HTML );
    ReportsWnd[ PosReportFilesOrganization ]    = CreateView( RUNTIME_CLASS( CReportFileOrganizationView ) );
    //ReportsWnd[ PosReportFilesOrganization ]->LoadFromResource( IDR_ORGANIZATION_REPORT_HTML );
    ReportsWnd[ PosReportTop100 ]               = CreateView( RUNTIME_CLASS( CReportTop100View ) );
    //ReportsWnd[ PosReportTop100 ]->LoadFromResource( IDR_TOP100_REPORT_HTML );
    ReportsWnd[ PosReportExcludedFiles ]        = CreateView( RUNTIME_CLASS( CReportExcludedFilesView ) );
    //ReportsWnd[ PosReportExcludedFiles ]->LoadFromResource( IDR_EXCLUDED_FILES_REPORT_HTML );

    //
    // Set defaut selection
    //    
    SetReportsVisibility( DefaultReportSel );
    m_wndList.SetCurSel( m_nCurReportSel );
    UpdateAllReports();
}

//
//
//
void CReportsView::OnSize(UINT nType, int cx, int cy)
{
    CView::OnSize(nType, cx, cy);

    //
    // Set listbox size & positions
    //
    m_wndList.MoveWindow( LeftMargin, TopMargin, 
        ListWidth, cy - ( TopMargin + BottomIndent ), FALSE );

    //
    // Set reports windows size & positions
    //
    for ( int i = 0; i < ReportsCount; i++ )
        if ( ReportsWnd[ i ] )
            ReportsWnd[ i ]->SetWindowPos( NULL,LeftMargin + ListWidth + 10, TopMargin,
                cx - ( LeftMargin + ListWidth + 10 + RightIndent ), cy - ( TopMargin + BottomIndent ),        
                SWP_NOACTIVATE | SWP_NOZORDER);
}

//
//
//
void CReportsView::SetReportsVisibility( int Index )
{
    if ( m_nCurReportSel != Index )
    {
        for ( int i = 0; i < ReportsCount; i++ )
            if ( ReportsWnd[ i ] )
                ReportsWnd[ i ]->ShowWindow( i == Index );
        m_nCurReportSel = Index;
    }
}

//
//
//
void CReportsView::OnChangeReport()
{
    SetReportsVisibility( m_wndList.GetCurSel() );
}

//
//
//
BOOL CReportsView::PreCreateWindow(CREATESTRUCT& cs)
{    
    cs.style |= WS_CLIPCHILDREN;

    return CView::PreCreateWindow(cs);
}

void CReportsView::OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/)
{
    UpdateAllReports();
}

void CReportsView::UpdateAllReports()
{
    for ( int i = 0; i < ReportsCount; i++ )
    {
        if ( ReportsWnd[ i ] )
            ReportsWnd[ i ]->CreateReport();
    }
}

BOOL CReportsView::OnEraseBkgnd(CDC* pDC)
{
    return TRUE;
}

//
//
//


void CReportsView::OnReportPrint()
{
    if( m_nCurReportSel >= PosReportSystem && m_nCurReportSel <= PosReportExcludedFiles && ReportsWnd[ m_nCurReportSel ] )
    {
        ReportsWnd[ m_nCurReportSel ]->OnReportPrint();
    }
}

void CReportsView::OnReportPrintPreview()
{
    if( m_nCurReportSel >= PosReportSystem && m_nCurReportSel <= PosReportExcludedFiles && ReportsWnd[ m_nCurReportSel ] )
    {
        ReportsWnd[ m_nCurReportSel ]->OnFilePrintPreview();
    }
}

//
//
//

void CReportsView::OnReportSave()
{
    if( m_nCurReportSel >= PosReportSystem && m_nCurReportSel <= PosReportExcludedFiles && ReportsWnd[ m_nCurReportSel ] )
    {
        ReportsWnd[ m_nCurReportSel ]->OnReportSave();

    }
}


//
//
//
CADefragUIDoc* CReportsView::GetDocument(void)
{
    ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CADefragUIDoc)));
    return (CADefragUIDoc*)m_pDocument;
}

//
//
//
void CReportsView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView )
{
    __super::OnActivateView( bActivate, pActivateView, pDeactiveView );
    CMainFrame* mainFrm = GetMainFrm();
    if( mainFrm ) mainFrm->SwitchReportRibbon( IsWindowVisible() );
}

/******************************************************************************
*
*       Function :       OnShowWindow                                                           
*
*       Purpose :        The framework calls this member function when the CWnd object 
*                                is about to be hidden or shown
*
*       Return :         None
*
*       Parameters : BOOL bShow - Specifies whether a window is being shown.
*                                       It is TRUE if the window is being shown;
*                                       it is FALSE if the window is being hidden.
*                                UINT nStatus - Specifies the status of the window being shown.
*
*******************************************************************************/
void CReportsView::OnShowWindow( BOOL bShow, UINT nStatus )
{
    __super::OnShowWindow( bShow, nStatus );
    CMainFrame* mainFrm = GetMainFrm();
    if( mainFrm ) mainFrm->SwitchReportRibbon( bShow );
}

void CReportsView::OnUpdateReportCommand(CCmdUI* pCmdUI)
{
    pCmdUI->Enable( TRUE );
}
