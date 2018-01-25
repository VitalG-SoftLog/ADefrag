/*
    Implementation of the CMainFrame class

    Module name:

    MainFrm.cpp

    Abstract:

    Implementation of the CMainFrame class

    $Header: /home/CVS_DEFRAG/Defrag/Src/ADefragUI/MainFrm.cpp,v 1.7 2010/01/12 22:53:18 roman Exp $
    $Log: MainFrm.cpp,v $
    Revision 1.7  2010/01/12 22:53:18  roman
    Add "Recommended Defrag Method..." button

    Revision 1.6  2009/12/29 09:53:00  dimas
    Vista / w7 tabs wrong fix disabled

    Revision 1.5  2009/12/24 01:39:10  roman
    fix errors

    Revision 1.4  2009/12/17 07:48:55  roman
    Win7 and Vista support.

    Revision 1.3  2009/11/24 18:02:52  roman
    Initial commit

    Revision 1.2  2009/11/24 17:43:24  roman
    Initial commit

*/

#include "stdafx.h"
#include "ADefragUI.h"

#include "MainFrm.h"
#include "DefragView.h"
#include "Scheduler/SchedulerView.h"
#include "DriveMap.h"
#include "ADefragUIDoc.h"
#include "DriveMapView.h"
#include "HelpView.h"

#include "Settings/OptionsPropSheet.h"
#include "Settings/PopularOptionsPage.h"
#include "Settings/DefragOptionsPage.h"
#include "Settings/ExcludeListOptionsPage.h"
#include "Settings/SchedulerOptionsPage.h"
#include "Settings/LogOptionsPage.h"
#include "Settings/UpdateOptionsPage.h"
#include "Settings/ResourcePage.h"
#include "Settings/resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//const int IdShowProgressTimer  = 1;
UINT CMainFrame::m_TaskbarCreated = 0;
#define UM_TRAYNOTIFY   (WM_USER + 1)

// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWndEx)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWndEx)
    ON_WM_CREATE()
    ON_WM_DESTROY()
    ON_WM_CLOSE()
    ON_WM_TIMER()
    ON_WM_SYSCOMMAND()
    ON_COMMAND(ID_FILE_CLOSE, &CMainFrame::OnFileClose)
    ON_COMMAND_RANGE(ID_VIEW_APPLOOK_WIN_2000, ID_VIEW_APPLOOK_OFF_2007_AQUA, &CMainFrame::OnApplicationLook)
    ON_UPDATE_COMMAND_UI_RANGE(ID_VIEW_APPLOOK_WIN_2000, ID_VIEW_APPLOOK_OFF_2007_AQUA, &CMainFrame::OnUpdateApplicationLook)
    //??ON_COMMAND(ID_FILE_PRINT, &CMainFrame::OnFilePrint)
    //??ON_COMMAND(ID_FILE_PRINT_DIRECT, &CMainFrame::OnFilePrint)
    //??ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CMainFrame::OnFilePrintPreview)
    //??ON_UPDATE_COMMAND_UI(ID_FILE_PRINT_PREVIEW, &CMainFrame::OnUpdateFilePrintPreview)
    ON_UPDATE_COMMAND_UI(ID_VIEW_OUTPUTWND, &CMainFrame::OnUpdateOutputWnd)
    ON_COMMAND(ID_VIEW_OUTPUTWND, &CMainFrame::OnViewOutputWnd)
    ON_COMMAND(ID_APP_OPEN, OnAppOpen)
    // Registered message
    ON_REGISTERED_MESSAGE(AFX_WM_ON_CHANGE_RIBBON_CATEGORY, OnChangeRibbonCategory)
    // Analyze and Defrag
    ON_UPDATE_COMMAND_UI(ID_DEFRAG_ANALYZE, &CMainFrame::OnUpdateDefragAnalyze)
    ON_UPDATE_COMMAND_UI(ID_DEFRAG_FREE_SPACE, &CMainFrame::OnUpdateDefragFreeSpace)
    ON_UPDATE_COMMAND_UI(ID_DEFRAG_SIMPLE, &CMainFrame::OnUpdateDefragSimple)
    ON_UPDATE_COMMAND_UI(ID_DEFRAG_ORDER, &CMainFrame::OnUpdateDefragOrder)
    ON_UPDATE_COMMAND_UI(ID_DEFRAG_BY_NAME, &CMainFrame::OnUpdateDefragOrder)
    ON_UPDATE_COMMAND_UI(ID_DEFRAG_BY_ACCESS, &CMainFrame::OnUpdateDefragOrder)
    ON_UPDATE_COMMAND_UI(ID_DEFRAG_BY_MODIFY, &CMainFrame::OnUpdateDefragOrder)
    ON_UPDATE_COMMAND_UI(ID_DEFRAG_BY_CREATION, &CMainFrame::OnUpdateDefragOrder)
    ON_UPDATE_COMMAND_UI(ID_DEFRAG_BY_SIZE, &CMainFrame::OnUpdateDefragOrder)
    ON_UPDATE_COMMAND_UI(ID_DEFRAG_BY_FORCE_FILL, &CMainFrame::OnUpdateDefragOrder)
    ON_UPDATE_COMMAND_UI(ID_DEFRAG_PAUSE, &CMainFrame::OnUpdateDefragPause)
    ON_UPDATE_COMMAND_UI(ID_DEFRAG_STOP, &CMainFrame::OnUpdateDefragStop)
    ON_UPDATE_COMMAND_UI(ID_VIEW_STYLE_BLOCKSIZECB, &CMainFrame::OnUpdateComboBox)
    ON_UPDATE_COMMAND_UI(ID_VIEW_TASKPANE, &CMainFrame::OnUpdateViewTaskpane)
    ON_UPDATE_COMMAND_UI(ID_VIEW_STYLE_SHOWLEGEND, &CMainFrame::OnUpdateShowLegend)
    ON_UPDATE_COMMAND_UI(ID_VIEW_STYLE_SETDEFAULT, &CMainFrame::OnUpdateSetDefault)
	ON_UPDATE_COMMAND_UI(ID_DEFRAG_MOSTRECOMMENDED, &CMainFrame::OnUpdateDefragMostrecommended)
    ON_COMMAND(ID_DEFRAG_ANALYZE, &CMainFrame::OnDefragAnalyze)
    ON_COMMAND(ID_DEFRAG_FREE_SPACE, &CMainFrame::OnDefragFreeSpace)
    ON_COMMAND(ID_DEFRAG_SIMPLE, &CMainFrame::OnDefragSimple)
    ON_COMMAND_RANGE(ID_DEFRAG_BY_NAME,ID_DEFRAG_BY_FORCE_FILL, &CMainFrame::OnDefragOrder)
    ON_COMMAND(ID_DEFRAG_PAUSE, &CMainFrame::OnDefragPause)
    ON_COMMAND(ID_DEFRAG_STOP, &CMainFrame::OnDefragStop)
    ON_COMMAND(ID_VIEW_STYLE_BLOCKSIZECB, &CMainFrame::OnComboBox)
    ON_COMMAND(ID_VIEW_TASKPANE, &CMainFrame::OnViewTaskpane)
    ON_COMMAND(ID_VIEW_STYLE_SHOWLEGEND, &CMainFrame::OnShowLegend)
    ON_COMMAND(ID_VIEW_STYLE_SETDEFAULT, &CMainFrame::OnSetDefault)
    ON_COMMAND(ID_DEFRAG_SETTINGS, &CMainFrame::OnSettings)
    ON_MESSAGE(UM_TRAYNOTIFY, OnTrayNotify)
    ON_REGISTERED_MESSAGE( m_TaskbarCreated, OnTaskbarCreated )
    // Calls from Shell Extension
    ON_MESSAGE( WM_COPYDATA, OnExternalCall )
    ON_MESSAGE( WM_DEVICECHANGE, OnDevicesListChanged )
    ON_COMMAND(ID_HELP, &CMainFrame::OnHelp)
    ON_COMMAND(ID_INDICATOR_LABEL, &CMainFrame::OnLink)
	ON_COMMAND(ID_DEFRAG_MOSTRECOMMENDED, &CMainFrame::OnDefragMostrecommended)
	ON_COMMAND(ID_DEFRAG_RECOMMEND, &CMainFrame::OnDefragRecommend)
	ON_UPDATE_COMMAND_UI(ID_DEFRAG_RECOMMEND, &CMainFrame::OnUpdateDefragRecommend)
END_MESSAGE_MAP()

// CMainFrame construction/destruction

CMainFrame::CMainFrame() :
    m_hIcon( NULL ),
    m_showMaximized( true )
{
    //Global.DrvView      = NULL;
    theApp.m_nAppLook = theApp.GetInt(_T("ApplicationLook"), ID_VIEW_APPLOOK_OFF_2007_BLUE);
    for( int i = e_firstView; i < e_countOfView; ++i )
    {
        //m_allViews[e_countOfView] = TViewPair( NULL, NULL );
    }
    m_activeView = e_firstView;
    m_nProgressValue = -1;

    // Note that LoadIcon does not require a subsequent DestroyIcon in Win32
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

    // Initialize NOTIFYICONDATA
    memset(&m_nid, 0 , sizeof(m_nid));
    m_nid.cbSize = sizeof(m_nid);
    m_nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;

	m_bVistaOrLater = Is_Vista_or_Later();
}

CMainFrame::~CMainFrame()
{
    m_nid.hIcon = NULL;
    Shell_NotifyIcon (NIM_DELETE, &m_nid);
}

//
//
//
CADefragUIDoc* CMainFrame::GetDocument(void)
{
    CWinApp* pApp = AfxGetApp();

    POSITION DocTemplatePos = pApp->GetFirstDocTemplatePosition();
    CDocTemplate* DocTemplate = pApp->GetNextDocTemplate(DocTemplatePos);

    POSITION DocPos = DocTemplate->GetFirstDocPosition();
    CDocument* Doc = DocTemplate->GetNextDoc(DocPos);
    ASSERT(Doc->IsKindOf(RUNTIME_CLASS(CADefragUIDoc)));
    return (CADefragUIDoc*)Doc;
}

CDriveMapView* CMainFrame::GetDriveMapView()
{
    CADefragUIDoc *pDoc = GetDocument();

    POSITION ViewPos = pDoc->GetFirstViewPosition();
    while ( ViewPos != NULL )
    {
      CView* pView = pDoc->GetNextView( ViewPos );
      if ( pView->IsKindOf( RUNTIME_CLASS( CDriveMapView ) ) )
          return DYNAMIC_DOWNCAST( CDriveMapView, pView );
    }

    return NULL;
}

CDefragView* CMainFrame::GetDriveListView(void)
{
    CADefragUIDoc *pDoc = GetDocument();

    POSITION ViewPos = pDoc->GetFirstViewPosition();
    while ( ViewPos != NULL )
    {
        CView* pView = pDoc->GetNextView( ViewPos );
        if ( pView->IsKindOf( RUNTIME_CLASS( CDefragView ) ) )
            return DYNAMIC_DOWNCAST( CDefragView, pView );
    }

    return NULL;
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CFrameWndEx::OnCreate(lpCreateStruct) == -1)
        return -1;
    ModifyStyle( FWS_ADDTOTITLE | FWS_PREFIXTITLE, 0 );

    BOOL bNameValid;
    // set the visual manager and style based on persisted value
    OnApplicationLook(theApp.m_nAppLook);

    m_wndRibbonBar.Create(this);
    InitializeRibbon();

    if( !m_wndStatusBar.Create(this) )
    {
        TRACE0("Failed to create status bar\n");
        return -1;      // fail to create
    }

    m_wndStatusBar.AddElement( new CMFCRibbonLinkCtrl(ID_INDICATOR_LABEL, _T("www.adefrag.com"), _T("http://www.adefrag.com")), _T("Link to website") );
    m_wndStatusBar.AddSeparator();
    CString strTitlePane1;
    bNameValid = strTitlePane1.LoadString(IDS_STATUS_PANE1);
    ASSERT(bNameValid);
    m_wndStatusBar.AddElement(new CMFCRibbonStatusBarPane(ID_STATUSBAR_PANE1, strTitlePane1, TRUE), _T("Disk Status") );
    /*
    CString strTitlePane2;
    bNameValid = strTitlePane2.LoadString(IDS_STATUS_PANE2);
    ASSERT(bNameValid);
    m_wndStatusBar.AddExtendedElement(new CMFCRibbonStatusBarPane(ID_STATUSBAR_PANE2, strTitlePane2, TRUE), strTitlePane2);
    */

    // enable Visual Studio 2005 style docking window behavior
    CDockingManager::SetDockingMode(DT_SMART);
    // enable Visual Studio 2005 style docking window auto-hide behavior
    EnableAutoHidePanes(CBRS_ALIGN_ANY);

    // create docking windows
    if (!CreateDockingWindows())
    {
        TRACE0("Failed to create docking windows\n");
        return -1;
    }

    if( !CreateTaskPane() )
    {
        TRACE0("Failed to create task pane\n");
        return -1;
    }
    //m_wndOutput.EnableDocking(CBRS_ALIGN_ANY);
    //DockPane(&m_wndOutput);

    m_wndTaskPane.EnableDocking(CBRS_ALIGN_RIGHT | CBRS_ALIGN_LEFT);
    EnableDocking(CBRS_ALIGN_ANY);
    EnableAutoHidePanes(CBRS_ALIGN_ANY);
    DockPane(&m_wndTaskPane);


    GetDocument()->Connect(m_hWnd);
//    SetTimer(IDT_TIMER, TIMER_PERIOD, NULL);


    // Set tray notification window
    m_TaskbarCreated = RegisterWindowMessage( _T("TaskbarCreated") );
    BOOL addTray = AddTrayIcon();
    ASSERT( addTray );
    AllowReceiveWMCopyData();

    return 0;
}

void CMainFrame::OnDestroy(void)
{
}

void CMainFrame::OnClose(void)
{
    CFrameWndEx::OnClose();
}

void CMainFrame::OnTimer(UINT_PTR NIDEvent)
{
    /*if ( m_nProgressValue == -1 )
    {
        ShowProgress( FALSE );
        return;
    }

    if ( !GetDocument()->GetBtnState(STOPBTN_STATE) )
    {
        ShowProgress( FALSE );
        pProgress->SetPos( 0 );
        m_nProgressValue = -1;
        KillTimer( NIDEvent );
        return;
    }

    //m_nProgressValue++; // += 10;
    if ( m_nProgressValue > 100) m_nProgressValue = 100;
    pProgress->SetPos( m_nProgressValue );

    ShowProgress( TRUE );
    CMFCRibbonProgressBar* pStatusProgress = DYNAMIC_DOWNCAST(CMFCRibbonProgressBar, m_wndStatusBar.FindElement(ID_INDICATOR_PROGRESS));
    ASSERT_VALID(pStatusProgress);
    if( pStatusProgress ) pStatusProgress->SetPos( m_nProgressValue, TRUE );*/
}

void CMainFrame::ShowProgress(BOOL bShow)
{
    CMFCRibbonProgressBar* pStatusProgress = DYNAMIC_DOWNCAST(CMFCRibbonProgressBar, m_wndStatusBar.FindElement(ID_INDICATOR_PROGRESS));
    if( (pStatusProgress != NULL) == bShow ) return;
    if (bShow)
    {
        int cxFree = m_wndStatusBar.GetSpace();
        if (cxFree < 20)
        {
            // Not enough space for progress bar
            return;
        }

        int cxProgress = min(cxFree, 150);
        CMFCRibbonProgressBar* pProgressBar = new CMFCRibbonProgressBar(ID_INDICATOR_PROGRESS, cxProgress);
        pProgressBar->SetInfiniteMode(FALSE);
        m_wndStatusBar.AddDynamicElement(pProgressBar);
    }
    else
    {
        m_wndStatusBar.RemoveElement(ID_INDICATOR_PROGRESS);
    }

    m_wndStatusBar.RecalcLayout();
    m_wndStatusBar.RedrawWindow();
    CMFCPopupMenu::UpdateAllShadows();
}

void CMainFrame::UpdateStatusAndProgress( LPCTSTR status, int progress )
{
    m_nProgressValue = progress;
    if ( !GetDocument()->GetBtnState(STOPBTN_STATE) )
    {
        ShowProgress( FALSE );
        pProgress->SetPos( 0 );
        m_nProgressValue = -1;
    }
    else
    {
        if ( m_nProgressValue > 100) m_nProgressValue = 100;
        pProgress->SetPos( m_nProgressValue );

        ShowProgress( TRUE );
        CMFCRibbonProgressBar* pStatusProgress = DYNAMIC_DOWNCAST(CMFCRibbonProgressBar, m_wndStatusBar.FindElement(ID_INDICATOR_PROGRESS));
        ASSERT_VALID(pStatusProgress);
        if( pStatusProgress ) pStatusProgress->SetPos( m_nProgressValue, TRUE );
    }
    CMFCRibbonStatusBarPane* pStatusBar = DYNAMIC_DOWNCAST(CMFCRibbonStatusBarPane, m_wndStatusBar.FindElement(ID_STATUSBAR_PANE1));
    if( !pStatusBar ) return;
    pStatusBar->SetText( status );
    m_wndStatusBar.RecalcLayout();
    m_wndStatusBar.RedrawWindow();
}

BOOL CMainFrame::OnCreateClient( LPCREATESTRUCT lpcs, CCreateContext* pContext )
{
    //return __super::OnCreateClient( lpcs, pContext );
    CRect rectClient;
    GetClientRect(rectClient);

    m_splitterDefrag.CreateStatic( this, 2, 1 );
    m_splitterDefrag.CreateView( 0, 0, RUNTIME_CLASS(CDefragView), CSize(0, rectClient.Height() / 5), pContext );
    m_splitterDefrag.SetRowInfo( 0, 100, 30 );
    m_splitterDefrag.CreateView( 1, 0, pContext->m_pNewViewClass, CSize(0, rectClient.Height() * 4 / 5), pContext );
    m_splitterDefrag.SetRowInfo( 1, 400, 50 );

    return TRUE;
}

/*
    Create the all additional views for switch in the application
*/
void CMainFrame::InitViews()
{
    m_allViews[e_DefragView] = TViewPair( &m_splitterDefrag, GetActiveView() );
    m_allViews[e_DefragReports] = TViewPair( &m_splitterDefrag, GetActiveView() );

    CView* allViews[e_countOfView];
    allViews[e_SchedulerView] = (CView*) new CSchedulerView;
    m_allViews[e_SchedulerView] = TViewPair( allViews[e_SchedulerView], allViews[e_SchedulerView] );
    allViews[e_HelpView] = (CView*) new CHelpView;
    m_allViews[e_HelpView] = TViewPair( allViews[e_HelpView], allViews[e_HelpView] );

    CDocument* pCurrentDoc = GetActiveDocument();

    // Initialize a CCreateContext to point to the active document.
    // With this context, the new view is added to the document
    // when the view is created in CView::OnCreate().
    CCreateContext newContext;
    newContext.m_pNewViewClass = NULL;
    newContext.m_pNewDocTemplate = NULL;
    newContext.m_pLastView = NULL;
    newContext.m_pCurrentFrame = NULL;
    newContext.m_pCurrentDoc = pCurrentDoc;

    CRect rect(0, 0, 0, 0); // gets resized later

    for( int view = e_SchedulerView; view < e_countOfView; view++ )
    {
        // Create the new view. In this example, the view persists for
        // the life of the application. The application automatically
        // deletes the view when the application is closed.
        allViews[view]->Create(NULL, NULL,
            (AFX_WS_DEFAULT_VIEW & ~WS_VISIBLE),
            // views are created with the style of AFX_WS_DEFAULT_VIEW
            // In MFC 4.0, this is (WS_BORDER | WS_VISIBLE | WS_CHILD)
            rect, this,
            AFX_IDW_PANE_FIRST + view, &newContext);

        // When a document template creates a view, the WM_INITIALUPDATE
        // message is sent automatically. However, this code must
        // explicitly send the message, as follows.
        allViews[view]->OnInitialUpdate();
    }

	m_activeView = e_DefragView;
}

/*
    This function used for activate the given view
*/
BOOL CMainFrame::ActivateView( ViewType view )
{
    ASSERT( view >= e_firstView && view < e_countOfView );
    if ( view < e_firstView || view >= e_countOfView ) return FALSE;

    CWnd* newView = m_allViews[view].first;
    if( !newView ) return FALSE;

    CWnd* activeView = m_allViews[m_activeView].first; //GetActiveView();
    if( !activeView )    // No currently active view
        return FALSE;

    if( newView == activeView )    // Already there
        return TRUE;

    // exchange view window ID's so RecalcLayout() works
    UINT temp = ::GetWindowLong( activeView->m_hWnd, GWL_ID );
    ::SetWindowLong( activeView->m_hWnd, GWL_ID,
        ::GetWindowLong( newView->m_hWnd, GWL_ID ) );
    ::SetWindowLong( newView->m_hWnd, GWL_ID, temp );

    // Display and update the new current view - hide the old one    
    activeView->ShowWindow( SW_HIDE );
    BOOL ret = newView->ShowWindow( SW_SHOW );
    SetActiveView( m_allViews[view].second );
    RecalcLayout();
    newView->Invalidate();
    m_activeView = view;
    return ret;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
    if (cs.lpszClass == NULL)
    {
        //VERIFY(AfxDeferRegisterClass(AFX_WNDFRAMEORVIEW_REG));
        cs.lpszClass = g_MainWindowClassName;
    }
    if( !CFrameWndEx::PreCreateWindow(cs) )
        return FALSE;
    // TODO: Modify the Window class or styles here by modifying
    //  the CREATESTRUCT cs

    return TRUE;
}

void CMainFrame::InitializeRibbon()
{
    BOOL bNameValid;

    CString strTemp;
    bNameValid = strTemp.LoadString(IDS_RIBBON_MAIN);
    ASSERT(bNameValid);

    // Load panel images:
    m_PanelImages.SetImageSize(CSize(16, 16));
    m_PanelImages.Load(IDB_BUTTONS);

    // Init main button:
    m_MainButton.SetImage(IDB_MAIN);
    m_MainButton.SetText(_T("\nm"));
    m_MainButton.SetToolTipText(strTemp);

    m_wndRibbonBar.SetApplicationButton(&m_MainButton, CSize (45, 45));
    CMFCRibbonMainPanel* pMainPanel = m_wndRibbonBar.AddMainCategory(strTemp, IDB_FILESMALL, IDB_FILELARGE);
    bNameValid = strTemp.LoadString(IDS_RIBBON_ABOUT);
    ASSERT(bNameValid);
    pMainPanel->Add(new CMFCRibbonButton(ID_APP_ABOUT, strTemp, 0, 0));

    bNameValid = strTemp.LoadString(IDS_RIBBON_EXIT);
    ASSERT(bNameValid);
    pMainPanel->AddToBottom(new CMFCRibbonMainPanelButton(ID_APP_EXIT, strTemp, 15));

    // Add "Defragmentation" category with "Defragment" panel:
    bNameValid = strTemp.LoadString(IDS_RIBBON_DEFRAG);
    ASSERT(bNameValid);
    CMFCRibbonCategory* pCategoryDefragmentation = m_wndRibbonBar.AddCategory(strTemp, IDB_DEFRAGSMALL, IDB_DEFRAGLARGE);

    // Create "Analyze and Defrag" panel:
    bNameValid = strTemp.LoadString(IDS_RIBBON_DEFRAG_PANEL);
    ASSERT(bNameValid);
    CMFCRibbonPanel* pPanelAnalyzeDefrag = pCategoryDefragmentation->AddPanel(strTemp, m_PanelImages.ExtractIcon(27));

    // Defrag Computer btn    
    bNameValid = strTemp.LoadString(IDS_RIBBON_DEFRAG_COMPUTER);
    ASSERT(bNameValid);
    pPanelAnalyzeDefrag->Add( new CMFCRibbonButton(ID_DEFRAG_COMPUTER, strTemp, 0, 0) );
    pPanelAnalyzeDefrag->Add(new CMFCRibbonSeparator());

    // Analyze btn    
    bNameValid = strTemp.LoadString(IDS_RIBBON_DEFRAG_ANALYZE);
    ASSERT(bNameValid);
    pPanelAnalyzeDefrag->Add( new CMFCRibbonButton(ID_DEFRAG_ANALYZE, strTemp, 1, 1) );

    // Free space btn    
    bNameValid = strTemp.LoadString(IDS_RIBBON_FREE_SPACE);
    ASSERT(bNameValid);
    pPanelAnalyzeDefrag->Add(new CMFCRibbonButton(ID_DEFRAG_FREE_SPACE, strTemp, 2, 2 ) );

    // Defrag btn
    bNameValid = strTemp.LoadString(IDS_RIBBON_DEFRAG_SIMPLE);
    ASSERT(bNameValid);
    pPanelAnalyzeDefrag->Add(new CMFCRibbonButton(ID_DEFRAG_SIMPLE, strTemp, 3, 3 ) );

	// Recommended btn
    bNameValid = strTemp.LoadString(IDS_RIBBON_DEFRAG_RECOMMEND);
    ASSERT(bNameValid);
    pPanelAnalyzeDefrag->Add(new CMFCRibbonButton(ID_DEFRAG_RECOMMEND, strTemp, 4, 4 ) );
    pPanelAnalyzeDefrag->Add(new CMFCRibbonSeparator());

    // Defrag Order btn
    bNameValid = strTemp.LoadString(IDS_RIBBON_DEFRAG_ORDER);
    ASSERT(bNameValid);
    CMFCRibbonButton* defragOrder = new CMFCRibbonButton(ID_DEFRAG_ORDER, strTemp, 5, 5 );
    defragOrder->SetMenu( IDR_RIBBON_DEFRAG_ORDER );
    pPanelAnalyzeDefrag->Add( defragOrder );

    // Create "Progress" panel:
    bNameValid = strTemp.LoadString(IDS_RIBBON_PROGRESS_PANEL);
    ASSERT(bNameValid);
    CMFCRibbonPanel* pPanelProgress = pCategoryDefragmentation->AddPanel(strTemp, m_PanelImages.ExtractIcon(25));
    pPanelProgress->SetCenterColumnVert();
    bNameValid = strTemp.LoadString(IDS_RIBBON_PROGRESS_CURRENT_LABEL);
    ASSERT(bNameValid);
    pPanelProgress->Add(new CMFCRibbonLabel(strTemp));

    pProgress = new CMFCRibbonProgressBar( ID_RIBBON_DEFRAG_PROGRESS, RIBBON_DEFRAG_PROGRESS_WIDTH );
    pProgress->SetInfiniteMode( FALSE );
    pPanelProgress->Add( pProgress );
    
    pPanelProgress->Add(new CMFCRibbonSeparator());
    // Pause Btn
    bNameValid = strTemp.LoadString(IDS_RIBBON_PAUSE);
    ASSERT(bNameValid);
    pPanelProgress->Add(new CMFCRibbonButton(ID_DEFRAG_PAUSE, strTemp, 7, 7 ) );
    // Stop Btn
    bNameValid = strTemp.LoadString(IDS_RIBBON_STOP);
    ASSERT(bNameValid);
    pPanelProgress->Add(new CMFCRibbonButton(ID_DEFRAG_STOP, strTemp, 6, 6 ) );

    // Create and add a "Settings" panel:
    bNameValid = strTemp.LoadString(IDS_RIBBON_VIEW);
    ASSERT(bNameValid);
    CMFCRibbonPanel* pPanelView = pCategoryDefragmentation->AddPanel(strTemp, m_PanelImages.ExtractIcon (7));
    pPanelView->SetCenterColumnVert();
    //CMFCRibbonPanel* pPanelView = pCategoryViews->AddPanel(strTemp, m_PanelImages.ExtractIcon (7));

    bNameValid = strTemp.LoadString(IDS_RIBBON_SETTINGS);
    ASSERT(bNameValid);
    pPanelView->Add(new CMFCRibbonButton(ID_DEFRAG_SETTINGS, strTemp, 8, 8 ) );
    pPanelView->Add(new CMFCRibbonSeparator());

    bNameValid = strTemp.LoadString(IDS_TASKPANE_WND);
    ASSERT(bNameValid);
    CMFCRibbonButton* pBtnTasksPane = new CMFCRibbonCheckBox(ID_VIEW_TASKPANE, strTemp);
    pPanelView->Add(pBtnTasksPane);

    bNameValid = strTemp.LoadString(IDS_RIBBON_STATUSBAR);
    ASSERT(bNameValid);
    CMFCRibbonButton* pBtnStatusBar = new CMFCRibbonCheckBox(ID_VIEW_STATUS_BAR, strTemp);
    pPanelView->Add(pBtnStatusBar);

    bNameValid = strTemp.LoadString( ID_VIEW_STYLE_SHOWLEGEND );
    ASSERT( bNameValid );
    CMFCRibbonButton* pShowLegend = new CMFCRibbonCheckBox( ID_VIEW_STYLE_SHOWLEGEND, strTemp );
    pPanelView->Add( pShowLegend );

    bNameValid = strTemp.LoadString(IDS_RIBBON_OUTPUT_WINDOW);
    ASSERT(bNameValid);
    //CMFCRibbonButton* pBtnOutputBar = new CMFCRibbonCheckBox(ID_VIEW_OUTPUTWND, strTemp);
    //pPanelView->Add(pBtnOutputBar);

    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // Add "View" category with "Style" panel:
    // Create "Style" panel:
    bNameValid = strTemp.LoadString( IDS_RIBBON_VIEW_STYLE );
    ASSERT( bNameValid );
    CMFCRibbonPanel* pPanelStyle = pCategoryDefragmentation->AddPanel( strTemp, m_PanelImages.ExtractIcon(27) );

    bNameValid = strTemp.LoadString( ID_VIEW_STYLE_BLOCKSIZECB );
    ASSERT( bNameValid );
    CMFCRibbonLabel *pLabel1 = new CMFCRibbonLabel( strTemp );
    pPanelStyle->Add( pLabel1 );

    //CMFCRibbonComboBox *pBlockSizeCB = new CMFCRibbonComboBox( ID_VIEW_STYLE_BLOCKSIZECB, TRUE, 74 );
    pBlockSizeCB = new CMFCRibbonComboBox( ID_VIEW_STYLE_BLOCKSIZECB, FALSE, 74 );    
    pBlockSizeCB->AddItem( _T("8 pixels") );
    pBlockSizeCB->AddItem( _T("10 pixels") );
    pBlockSizeCB->AddItem( _T("12 pixels") );
    pBlockSizeCB->AddItem( _T("14 pixels") );
    pBlockSizeCB->AddItem( _T("16 pixels") );
    pBlockSizeCB->AddItem( _T("18 pixels") );
    pBlockSizeCB->AddItem( _T("20 pixels") );
    
    pBlockSizeCB->SelectItem( GetDriveMapView()->GetBlockWidth() / 2 - 4 );
    pPanelStyle->Add( pBlockSizeCB );

    //pPanelStyle->Add( new CMFCRibbonSeparator() );

    bNameValid = strTemp.LoadString( ID_VIEW_STYLE_SETDEFAULT );
    ASSERT( bNameValid );
    CMFCRibbonButton* pSetDefault = new CMFCRibbonButton( ID_VIEW_STYLE_SETDEFAULT, strTemp );
    pPanelStyle->Add( pSetDefault );    

    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    // Add "Reports" category with "Reports" panel:
    bNameValid = strTemp.LoadString(IDS_RIBBON_REPORTS);
    ASSERT(bNameValid);
    CString strContextTemp;
    bNameValid = strContextTemp.LoadString(IDS_RIBBON_REPORTS_CONTEXT);
    ASSERT(bNameValid);
    CMFCRibbonCategory* pCategoryReports = m_wndRibbonBar.AddContextCategory(strTemp, strContextTemp, ID_REPORT_CONTEXT, AFX_CategoryColor_Green, IDB_FILESMALL, IDB_FILELARGE);

    // Create "Reports" panel:
    bNameValid = strTemp.LoadString(IDS_RIBBON_REPORTS_PANEL);
    ASSERT(bNameValid);
    CMFCRibbonPanel* pPanelReportsReports = pCategoryReports->AddPanel(strTemp, m_PanelImages.ExtractIcon(25));

    bNameValid = strTemp.LoadString( IDS_RIBBON_REPORT_SAVE );
    ASSERT( bNameValid );
    CMFCRibbonButton* pReportSave = new CMFCRibbonButton( ID_REPORT_SAVE, strTemp, 2, 2 );
    pPanelReportsReports->Add( pReportSave );
    pPanelReportsReports->Add(new CMFCRibbonSeparator());

    bNameValid = strTemp.LoadString( IDS_RIBBON_REPORT_PRINT );
    ASSERT( bNameValid );
    CMFCRibbonButton* pReportPrint = new CMFCRibbonButton( ID_REPORT_PRINT, strTemp, 6, 6 );
    pPanelReportsReports->Add( pReportPrint );

    bNameValid = strTemp.LoadString( IDS_RIBBON_REPORT_PRINT_SETUP );
    ASSERT( bNameValid );
    //CMFCRibbonButton* pReportPrintSetup = new CMFCRibbonButton( ID_REPORT_PRINT_SETUP, strTemp, 7, 7 );
    CMFCRibbonButton* pReportPrintSetup = new CMFCRibbonButton( ID_FILE_PRINT_SETUP, strTemp, 7, 7 );
    pPanelReportsReports->Add( pReportPrintSetup );

    bNameValid = strTemp.LoadString( IDS_RIBBON_REPORT_PRINT_PREVIEW );
    ASSERT( bNameValid );
    //CMFCRibbonButton* pReportPrintPreview = new CMFCRibbonButton( ID_REPORT_PRINT_PREVIEW, strTemp, 8, 8 );
    CMFCRibbonButton* pReportPrintPreview = new CMFCRibbonButton( ID_FILE_PRINT_PREVIEW, strTemp, 8, 8 );
    pPanelReportsReports->Add( pReportPrintPreview );

    m_wndRibbonBar.ShowContextCategories( ID_REPORT_CONTEXT, FALSE );

    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    // Add "Scheduler" category with "Create New Schedule" panel:
    bNameValid = strTemp.LoadString(IDS_RIBBON_SCHEDULER);
    ASSERT(bNameValid);
    CMFCRibbonCategory* pCategoryScheduler = m_wndRibbonBar.AddCategory(strTemp, IDB_SCHEDULERSMALL, IDB_SCHEDULERLARGE);

    // Create "Create New Schedule" panel:
    bNameValid = strTemp.LoadString(IDS_RIBBON_SCHEDULER_NEW_PANEL);
    ASSERT(bNameValid);
    CMFCRibbonPanel* panelNewSchedule = pCategoryScheduler->AddPanel(strTemp, m_PanelImages.ExtractIcon(25));
    // One Time Btn
    bNameValid = strTemp.LoadString(IDS_RIBBON_SCHEDULE_NEW_ONETIME);
    ASSERT(bNameValid);
    panelNewSchedule->Add(new CMFCRibbonButton(ID_SCHEDULER_NEW_ONETIME, strTemp, 0, 0 ) );
    // Daily Btn
    bNameValid = strTemp.LoadString(IDS_RIBBON_SCHEDULE_NEW_DAILY);
    ASSERT(bNameValid);
    panelNewSchedule->Add(new CMFCRibbonButton(ID_SCHEDULER_NEW_DAILY, strTemp, 1, 1 ) );
    // Weekly Btn
    bNameValid = strTemp.LoadString(IDS_RIBBON_SCHEDULE_NEW_WEEKLY);
    ASSERT(bNameValid);
    panelNewSchedule->Add(new CMFCRibbonButton(ID_SCHEDULER_NEW_WEEKLY, strTemp, 2, 2 ) );
    // Screen Saver Btn
    bNameValid = strTemp.LoadString(IDS_RIBBON_SCHEDULE_NEW_SCREENSAVER);
    ASSERT(bNameValid);
    panelNewSchedule->Add(new CMFCRibbonButton(ID_SCHEDULER_NEW_SCREENSAVER, strTemp, 3, 3 ) );

    // Create "Run Schedule" panel:
    bNameValid = strTemp.LoadString(IDS_RIBBON_SCHEDULER_RUN_PANEL);
    ASSERT(bNameValid);
    CMFCRibbonPanel* panelRunSchedule = pCategoryScheduler->AddPanel(strTemp, m_PanelImages.ExtractIcon(25));
    // Run Btn
    bNameValid = strTemp.LoadString(IDS_RIBBON_SCHEDULE_RUN);
    ASSERT(bNameValid);
    panelRunSchedule->Add(new CMFCRibbonButton(ID_SCHEDULER_RUN, strTemp, 4, 4 ) );

    // Create "Edit Schedule" panel:
    bNameValid = strTemp.LoadString(IDS_RIBBON_SCHEDULER_EDIT_PANEL);
    ASSERT(bNameValid);
    CMFCRibbonPanel* panelEditSchedule = pCategoryScheduler->AddPanel(strTemp, m_PanelImages.ExtractIcon(25));
    // Enable/Disable Btn
    bNameValid = strTemp.LoadString(IDS_RIBBON_SCHEDULE_ENABLE);
    ASSERT(bNameValid);
    panelEditSchedule->Add(new CMFCRibbonButton(ID_SCHEDULER_ENABLE, strTemp, 5, 5 ) );
    // Edit Btn
    bNameValid = strTemp.LoadString(IDS_RIBBON_SCHEDULE_EDIT);
    ASSERT(bNameValid);
    panelEditSchedule->Add(new CMFCRibbonButton(ID_SCHEDULER_EDIT, strTemp, 6, 6 ) );
    // Delete Btn
    bNameValid = strTemp.LoadString(IDS_RIBBON_SCHEDULE_DELETE);
    ASSERT(bNameValid);
    panelEditSchedule->Add(new CMFCRibbonButton(ID_SCHEDULER_DELETE, strTemp, 7, 7 ) );

    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    // Add "Help" category with "Help" panel:
    bNameValid = strTemp.LoadString(IDS_RIBBON_HELP);
    ASSERT(bNameValid);
    CMFCRibbonCategory* pCategoryHelp = m_wndRibbonBar.AddCategory(strTemp, IDB_HELPSMALL, IDB_HELPLARGE);

    // Create "Help" panel:
    bNameValid = strTemp.LoadString(IDS_RIBBON_HELP_PANEL);
    ASSERT(bNameValid);
    CMFCRibbonPanel* panelHelp = pCategoryHelp->AddPanel(strTemp, m_PanelImages.ExtractIcon(25));
    // About
    bNameValid = strTemp.LoadString(IDS_RIBBON_ABOUT);
    ASSERT(bNameValid);
    panelHelp->Add(new CMFCRibbonButton(ID_APP_ABOUT, strTemp, 0, 0 ) );
    // Help
    bNameValid = strTemp.LoadString(IDS_RIBBON_HELP_ACTION);
    ASSERT(bNameValid);
    panelHelp->Add(new CMFCRibbonButton(ID_HELP, strTemp, 1, 1 ) );

    // Create "License" panel:
    bNameValid = strTemp.LoadString(IDS_RIBBON_LICENSE_PANEL);
    ASSERT(bNameValid);
    CMFCRibbonPanel* panelLicense = pCategoryHelp->AddPanel(strTemp, m_PanelImages.ExtractIcon(0));
    bNameValid = strTemp.LoadString(IDS_LICENSE_INFO);
    ASSERT(bNameValid);
    panelLicense->Add( new CMFCRibbonLabel( strTemp, TRUE ) );
    panelLicense->AddSeparator();
    // Purchase Now
    bNameValid = strTemp.LoadString(IDS_RIBBON_BUY);
    ASSERT(bNameValid);
    panelLicense->Add(new CMFCRibbonButton(ID_APP_BUY, strTemp, 2, 2 ) );

    // Create "Update" panel:
    bNameValid = strTemp.LoadString(IDS_RIBBON_UPDATE_PANEL);
    ASSERT(bNameValid);
    CMFCRibbonPanel* panelUpdate = pCategoryHelp->AddPanel(strTemp, m_PanelImages.ExtractIcon(1));
    bNameValid = strTemp.LoadString(IDS_VERSION_INFO);
    ASSERT(bNameValid);
    panelUpdate->Add( new CMFCRibbonLabel( strTemp, TRUE ) );
    panelUpdate->AddSeparator();
    // Update Now
    bNameValid = strTemp.LoadString(IDS_RIBBON_UPDATE);
    ASSERT(bNameValid);
    panelUpdate->Add(new CMFCRibbonButton(ID_APP_UPDATE, strTemp, 3, 3 ) );

    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    //////////////////////////////////////////////////////////////////////////
    // Add elements to the right side of tabs:
    bNameValid = strTemp.LoadString(IDS_RIBBON_STYLE);
    ASSERT(bNameValid);
    CMFCRibbonButton* pVisualStyleButton = new CMFCRibbonButton(-1, strTemp, -1, -1);

    pVisualStyleButton->SetMenu(IDR_THEME_MENU, FALSE /* No default command */, TRUE /* Right align */);

    bNameValid = strTemp.LoadString(IDS_RIBBON_STYLE_TIP);
    ASSERT(bNameValid);
    pVisualStyleButton->SetToolTipText(strTemp);
    bNameValid = strTemp.LoadString(IDS_RIBBON_STYLE_DESC);
    ASSERT(bNameValid);
    pVisualStyleButton->SetDescription(strTemp);
    m_wndRibbonBar.AddToTabs(pVisualStyleButton);

    // Add quick access toolbar commands:
    CList<UINT, UINT> lstQATCmds;

    lstQATCmds.AddTail(ID_DEFRAG_ANALYZE);
    lstQATCmds.AddTail(ID_DEFRAG_FREE_SPACE);
    lstQATCmds.AddTail(ID_DEFRAG_SIMPLE);
	lstQATCmds.AddTail(ID_DEFRAG_RECOMMEND);
    lstQATCmds.AddTail(ID_DEFRAG_STOP);

    m_wndRibbonBar.SetQuickAccessCommands(lstQATCmds);
    m_wndRibbonBar.AddToTabs(new CMFCRibbonButton(ID_APP_ABOUT, _T("\na"), m_PanelImages.ExtractIcon (0)));
}

BOOL CMainFrame::CreateTaskPane()
{
    CRect rectDummy(0, 0, 200, 400);
    CString strTaskPaneWnd;
    BOOL bNameValid = strTaskPaneWnd.LoadString(IDS_TASKPANE_WND);
    ASSERT(bNameValid);
    if (!m_wndTaskPane.Create( strTaskPaneWnd, this, rectDummy, TRUE, ID_VIEW_TASKPANE,
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_RIGHT | AFX_CBRS_CLOSE | AFX_CBRS_FLOAT))
    {
        TRACE0("Failed to create task pane\n");
        return FALSE;      // fail to create
    }
    m_wndTaskPane.EnableAutohideAll();

    return TRUE;
}

BOOL CMainFrame::CreateDockingWindows()
{
    BOOL bNameValid;
    // Create output window
    CString strOutputWnd;
    bNameValid = strOutputWnd.LoadString(IDS_OUTPUT_WND);
    ASSERT(bNameValid);
    //if (!m_wndOutput.Create(strOutputWnd, this, CRect(0, 0, 100, 100), TRUE, ID_VIEW_OUTPUTWND, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_BOTTOM | CBRS_FLOAT_MULTI))
    //{
    //    TRACE0("Failed to create Output window\n");
    //    return FALSE; // failed to create
    //}

    SetDockingWindowIcons(theApp.m_bHiColorIcons);
    return TRUE;
}

void CMainFrame::SetDockingWindowIcons(BOOL bHiColorIcons)
{
    HICON hOutputBarIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(bHiColorIcons ? IDI_OUTPUT_WND_HC : IDI_OUTPUT_WND), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
    //m_wndOutput.SetIcon(hOutputBarIcon, FALSE);

}

// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
    CFrameWndEx::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
    CFrameWndEx::Dump(dc);
}
#endif //_DEBUG


// CMainFrame message handlers

void CMainFrame::OnApplicationLook(UINT id)
{
    CWaitCursor wait;

    theApp.m_nAppLook = id;

    switch (theApp.m_nAppLook)
    {
    case ID_VIEW_APPLOOK_WIN_2000:
        CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManager));
        break;

    case ID_VIEW_APPLOOK_OFF_XP:
        CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOfficeXP));
        break;

    case ID_VIEW_APPLOOK_WIN_XP:
        CMFCVisualManagerWindows::m_b3DTabsXPTheme = TRUE;
        CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));
        break;

    case ID_VIEW_APPLOOK_OFF_2003:
        CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2003));
        CDockingManager::SetDockingMode(DT_SMART);
        break;

    case ID_VIEW_APPLOOK_VS_2005:
        CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerVS2005));
        CDockingManager::SetDockingMode(DT_SMART);
        break;

    default:
        switch (theApp.m_nAppLook)
        {
        case ID_VIEW_APPLOOK_OFF_2007_BLUE:
            CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_LunaBlue);
            break;

        case ID_VIEW_APPLOOK_OFF_2007_BLACK:
            CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_ObsidianBlack);
            break;

        case ID_VIEW_APPLOOK_OFF_2007_SILVER:
            CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_Silver);
            break;

        case ID_VIEW_APPLOOK_OFF_2007_AQUA:
            CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_Aqua);
            break;
        }

        CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2007));
        CDockingManager::SetDockingMode(DT_SMART);
    }

    Global. ColorManager.OnApplicationLook();

    RedrawWindow(NULL, NULL, RDW_ALLCHILDREN | RDW_INVALIDATE | RDW_UPDATENOW | RDW_FRAME | RDW_ERASE);

    theApp.WriteInt(_T("ApplicationLook"), theApp.m_nAppLook);
}

void CMainFrame::OnUpdateApplicationLook(CCmdUI* pCmdUI)
{
    pCmdUI->SetRadio(theApp.m_nAppLook == pCmdUI->m_nID);
}

BOOL CMainFrame::LoadFrame(UINT nIDResource, DWORD dwDefaultStyle, CWnd* pParentWnd, CCreateContext* pContext) 
{
    // base class does the real work

    if (!CFrameWndEx::LoadFrame(nIDResource, dwDefaultStyle, pParentWnd, pContext))
    {
        return FALSE;
    }

    CWinApp* pApp = AfxGetApp();
    if (pApp->m_pMainWnd == NULL)
        pApp->m_pMainWnd = this;

    return TRUE;
}

void CMainFrame::OnFileClose()
{
    DestroyWindow();
}

void CMainFrame::OnFilePrint()
{
    if (IsPrintPreview())
    {
        PostMessage(WM_COMMAND, AFX_ID_PREVIEW_PRINT);
    }
}

void CMainFrame::OnFilePrintPreview()
{
    if (IsPrintPreview())
    {
        PostMessage(WM_COMMAND, AFX_ID_PREVIEW_CLOSE);  // force Print Preview mode closed
    }
}

void CMainFrame::OnUpdateFilePrintPreview(CCmdUI* pCmdUI)
{
    pCmdUI->SetCheck(IsPrintPreview());
}

void CMainFrame::OnUpdateOutputWnd(CCmdUI* pCmdUI)
{
    pCmdUI->SetCheck( m_wndOutput.IsVisible() );
}

void CMainFrame::OnViewOutputWnd()
{
    if( !m_wndOutput.IsVisible() )
    {
        m_wndOutput.ShowPane( TRUE, FALSE, TRUE );
    }
    else
    {
        m_wndOutput.ShowPane( FALSE, FALSE, FALSE );
    }
}

//////////////////////////////////////////////////////////////////////////
LRESULT CMainFrame::OnChangeRibbonCategory(WPARAM,LPARAM)
{
    CMFCRibbonCategory* pCategory = m_wndRibbonBar.GetActiveCategory();
    if( pCategory != NULL && pCategory->GetContextID() == 0 )
    {
        ASSERT_VALID( pCategory );
        int index = m_wndRibbonBar.GetCategoryIndex( pCategory );
        if ( index >= 0 )
        {
		    //if ( m_bVistaOrLater && index > 0 ) 
            //{
		    //	index--;
		    //}
            ViewType prevView = m_activeView;
            ActivateView( static_cast<ViewType>( index ) );
            if( m_activeView != e_DefragView ) // hide context ribbon for all other Views
            {
                SwitchReportRibbon( FALSE );
            }
            else //if( prevView != e_DefragView ) // activate drive map
            {
                CTabView* tabView = DYNAMIC_DOWNCAST( CTabView, m_splitterDefrag.GetPane(1,0) );
                if( tabView ) tabView->SetActiveView(0);
            }
        }
    }
    return 0;
}

/*
*/
void CMainFrame::OnUpdateDefragAnalyze(CCmdUI* pCmdUI)
{
    //BOOL State = CommDfrg->GetBtnState(ANALYZEBTN_STATE);
    //pCmdUI->Enable( State );

    BOOL State = GetDocument()->GetBtnState(ANALYZEBTN_STATE);
    pCmdUI->Enable( State );
}

void CMainFrame::OnUpdateDefragFreeSpace(CCmdUI* pCmdUI)
{
    //BOOL State = CommDfrg->GetBtnState(FREESPACEBTN_STATE);
    //pCmdUI->Enable( State );    

    BOOL State = GetDocument()->GetBtnState(FREESPACEBTN_STATE);
    pCmdUI->Enable( State );
}

void CMainFrame::OnUpdateDefragSimple(CCmdUI* pCmdUI)
{
    //BOOL State = CommDfrg->GetBtnState(DEFRAGBTN_STATE);
    //pCmdUI->Enable( State );

    BOOL State = GetDocument()->GetBtnState(DEFRAGBTN_STATE);
    pCmdUI->Enable( State );
}

void CMainFrame::OnUpdateDefragPause(CCmdUI* pCmdUI)
{
    pCmdUI->Enable( GetDocument()->GetBtnState(STOPBTN_STATE) );
    BOOL State = GetDocument()->GetBtnState(PAUSEBTN_STATE);
    pCmdUI->SetCheck( State ); // TODO
}

void CMainFrame::OnUpdateDefragOrder(CCmdUI* pCmdUI)
{
    BOOL State = GetDocument()->GetBtnState(DEFRAGBTN_STATE);
    pCmdUI->Enable( State );
}

void CMainFrame::OnUpdateDefragStop(CCmdUI* pCmdUI)
{
    //BOOL State = CommDfrg->GetBtnState(STOPBTN_STATE);
    //pCmdUI->Enable( State );

    BOOL State = GetDocument()->GetBtnState(STOPBTN_STATE);
    pCmdUI->Enable( State );
}


//
//
//
void CMainFrame::OnUpdateComboBox(CCmdUI* pCmdUI)
{    
    pCmdUI->Enable( TRUE ); 
}

void CMainFrame::OnViewTaskpane() 
{
    ShowPane( &m_wndTaskPane,
        !(m_wndTaskPane.IsVisible()),
        FALSE, TRUE);
    RecalcLayout ();
}

void CMainFrame::UpdateTaskpaneResultAndReliability( DriveHealth* health, BOOL activate, BOOL recreate )
{
	if (recreate) {
		m_wndTaskPane.UpdateResultAndReliabilityPage( health, activate );
	} else {
		m_wndTaskPane.UpdateDriveHealth(health);
	}

    if( health && activate )
    {
        ShowPane( &m_wndTaskPane, TRUE, FALSE, TRUE );
        RecalcLayout();
    }
}

void CMainFrame::OnUpdateViewTaskpane(CCmdUI* pCmdUI) 
{
    pCmdUI->SetCheck (m_wndTaskPane.IsVisible ());
}

//
//
//
void CMainFrame::OnUpdateShowLegend(CCmdUI* pCmdUI)
{
    pCmdUI->Enable( TRUE );

    CDriveMapView* pMapView = GetDriveMapView();
    if ( pMapView )
        pCmdUI->SetCheck( pMapView->GetShowLegend() );
    else
        pCmdUI->SetCheck( FALSE );
}

//
//
//
void CMainFrame::OnUpdateSetDefault(CCmdUI* pCmdUI)
{
    pCmdUI->Enable( TRUE );
}


//
// Reports
//
void CMainFrame::SwitchReportRibbon( BOOL IsVis )
{
    //pOldCategory = m_wndRibbonBar.GetActiveCategory();
    //m_wndRibbonBar.SetActiveCategory( pCategoryReports, TRUE );
    //m_wndRibbonBar.ShowCategory( ReportRibbonIndex, IsVis );
    if( IsVis )
    {
        m_wndRibbonBar.ShowContextCategories( ID_REPORT_CONTEXT, TRUE );
        m_wndRibbonBar.ActivateContextCategory( ID_REPORT_CONTEXT );
    }
    else
    {
        CMFCRibbonCategory* activeCategory = m_wndRibbonBar.GetActiveCategory();
        if( activeCategory && activeCategory->GetContextID() != ID_REPORT_CONTEXT )
        {
            m_wndRibbonBar.ActivateContextCategory( ID_REPORT_CONTEXT );
            m_wndRibbonBar.ShowContextCategories( ID_REPORT_CONTEXT, FALSE );
            m_wndRibbonBar.SetActiveCategory( activeCategory );
        }
        else
        {
            m_wndRibbonBar.ShowContextCategories( ID_REPORT_CONTEXT, FALSE );
        }
    }
}

void CMainFrame::OnDefragAnalyze()
{
    //CommDfrg->AddAnalyzeJob();
    GetDocument()->AddAnalyzeJob();
    //SetTimer( IdShowProgressTimer, 10, NULL );
    m_nProgressValue = 0;
}

void CMainFrame::OnDefragFreeSpace()
{
    //CommDfrg->AddFreSpaceJob();
    GetDocument()->AddFreSpaceJob();
    //SetTimer( IdShowProgressTimer, 10, NULL );
    m_nProgressValue = 0;
}

void CMainFrame::OnDefragSimple()
{
    //CommDfrg->AddDefragJob();
    GetDocument()->AddDefragJob( DEFRAG_CMD_SIMPLE_DEFRAG );
    //SetTimer( IdShowProgressTimer, 10, NULL );
    m_nProgressValue = 0;
}

void CMainFrame::OnDefragOrder(UINT id)
{
    // TODO: set a method
    switch( id )
    {
    case ID_DEFRAG_BY_NAME:
        GetDocument()->AddDefragJob( DEFRAG_CMD_SMART_BY_NAME );
        break;
    case ID_DEFRAG_BY_ACCESS:
        GetDocument()->AddDefragJob( DEFRAG_CMD_SMART_BY_ACCESS );
        break;
    case ID_DEFRAG_BY_MODIFY:
        GetDocument()->AddDefragJob( DEFRAG_CMD_SMART_BY_MODIFY );
        break;
    case ID_DEFRAG_BY_CREATION:
        GetDocument()->AddDefragJob( DEFRAG_CMD_SMART_BY_CREATE );
        break;
    case ID_DEFRAG_BY_SIZE:
        GetDocument()->AddDefragJob( DEFRAG_CMD_SMART_BY_SIZE );
        break;
    case ID_DEFRAG_BY_FORCE_FILL:
        GetDocument()->AddDefragJob( DEFRAG_CMD_FORCE_TOGETHER );
        break;
    }
    //SetTimer( IdShowProgressTimer, 10, NULL );
    m_nProgressValue = 0;
}

void CMainFrame::OnDefragPause()
{
    if( GetDocument()->GetBtnState(PAUSEBTN_STATE) )
    {
        GetDocument()->ResumeJob();
    }
    else
    {
        GetDocument()->PauseJob();
    }
}

void CMainFrame::OnDefragStop()
{
    //CommDfrg->StobJob();
    GetDocument()->StobJob();
}

void CMainFrame::OnComboBox()
{    
    int Size = ( pBlockSizeCB->GetCurSel() + 4 ) * 2;
    GetDriveMapView()->SetBlockWidth( Size );
    GetDriveMapView()->SetBlockHeight( Size );
}

void CMainFrame::OnShowLegend()
{
    CDriveMapView* pMapView = GetDriveMapView();
    if ( pMapView )
        pMapView->SetShowLegend( !pMapView->GetShowLegend() );
    
}

void CMainFrame::OnSetDefault()
{
    GetDriveMapView()->SetDefaultColor();
}

void CMainFrame::OnSettings()
{
    ShowOptions( 0 );
}

/*
    Show Settings page
*/
void CMainFrame::ShowOptions(int nPage)
{
    CResourcePage *pPageRes = NULL;

    // Add "popular" items:
    CList<UINT, UINT> lstPopular;

    lstPopular.AddTail(ID_DEFRAG_ANALYZE);
    lstPopular.AddTail(ID_DEFRAG_FREE_SPACE);
    lstPopular.AddTail(ID_DEFRAG_SIMPLE);
	lstPopular.AddTail(ID_DEFRAG_RECOMMEND);
    lstPopular.AddTail(ID_DEFRAG_STOP);

    // Create "Customize" page:
    std::auto_ptr<CMFCRibbonCustomizePropertyPage> apPageCustomize(new CMFCRibbonCustomizePropertyPage(&m_wndRibbonBar));
    CString name;
    name.LoadString( IDS_SETTINGS_POPULAR );
    apPageCustomize->AddCustomCategory( name, lstPopular );

    // Add hidden commands:
    CList<UINT,UINT> lstHidden;
    m_wndRibbonBar.GetItemIDsList(lstHidden, TRUE);

    name.LoadString( IDS_SETTINGS_NOT_RIBON );
    apPageCustomize->AddCustomCategory( name, lstHidden);

    // Add all commands:
    CList<UINT,UINT> lstAll;
    m_wndRibbonBar.GetItemIDsList(lstAll);

    name.LoadString( IDS_SETTINGS_ALL_COMMANDS );
    apPageCustomize->AddCustomCategory( name, lstAll);

    // Create "Options" and "Resources" pages:
    std::auto_ptr<CPopularOptionsPage> apPageOptions(new CPopularOptionsPage);
    std::auto_ptr<CDefragOptionsPage> apPageDefrag(new CDefragOptionsPage);
    std::auto_ptr<CExcludeListOptionsPage> apPageExclude(new CExcludeListOptionsPage);
    std::auto_ptr<CSchedulerOptionsPage> apPageScheduler(new CSchedulerOptionsPage);
    std::auto_ptr<CLogOptionsPage> apPageLog(new CLogOptionsPage);
    std::auto_ptr<CUpdateOptionsPage> apPageUpdate(new CUpdateOptionsPage);
    std::auto_ptr<CResourcePage> apPageRes(new CResourcePage);

    // Create property sheet:
    std::tr1::shared_ptr<CDefragOptionsPropSheet> spPropSheet(new CDefragOptionsPropSheet(this, nPage));
    spPropSheet->EnablePageHeader(max(60, afxGlobalData.GetTextHeight() * 3));

    spPropSheet->m_psh.dwFlags |= PSH_NOAPPLYNOW;

    spPropSheet->SetLook(CMFCPropertySheet::PropSheetLook_List, 124);

    spPropSheet->AddPage(apPageOptions.release());
    spPropSheet->AddPage(apPageDefrag.release());
    spPropSheet->AddPage(apPageExclude.release());
    spPropSheet->AddPage(apPageScheduler.release());
    spPropSheet->AddPage(apPageLog.release());
    spPropSheet->AddPage(apPageUpdate.release());
    spPropSheet->AddPage(apPageCustomize.release());
    spPropSheet->AddPage(apPageRes.release());

    if (spPropSheet->DoModal() == IDOK)
    {
        // Show or hide "Developer" tab:
        /*int nTabDevIndex = m_wndRibbonBar.FindCategoryIndexByData(idTabDeveloper);

        if (nTabDevIndex >= 0)
        {
            m_wndRibbonBar.RemoveCategory(nTabDevIndex );
        }*/

        // Change application theme and tooltips:
        OnApplicationLook(theApp.m_nAppLook);

        int TooltipStyle = theApp.GetInt( _T("TooltipStyle"), 0 );
        m_wndRibbonBar.EnableToolTips( TooltipStyle == 0 || TooltipStyle == 1, TooltipStyle == 0 );
        //m_wndRibbonBar.EnableKeyTips( theApp.GetInt( _T("ShowKeyTips"), TRUE ) );
    }

}

//****************************************************************************************
void CMainFrame::OnSysCommand(UINT nID, LPARAM lParam)
{
    if( SC_MINIMIZE == nID )
    {
        __super::OnSysCommand(nID, lParam);
        WINDOWPLACEMENT placament;
        if( GetWindowPlacement( &placament ) )
        {
            m_showMaximized = (placament.showCmd == SW_SHOWMAXIMIZED) ||
                (placament.showCmd == SW_SHOWMINIMIZED);
        }
        ShowWindow( SW_SHOWMINIMIZED );
        theApp.HideApplication();
    }
    else
        if( SC_CLOSE == nID )
        {
            WINDOWPLACEMENT placament;
            if( GetWindowPlacement( &placament ) )
            {
                m_showMaximized = (placament.showCmd == SW_SHOWMAXIMIZED) ||
                    (placament.showCmd == SW_SHOWMINIMIZED);
            }
            ShowWindow( SW_SHOWMINIMIZED );
            theApp.HideApplication();
        }
        else
            __super::OnSysCommand(nID, lParam);
}

//
// Notify messages from tray
//
LRESULT CMainFrame::OnTrayNotify(WPARAM /*wp*/, LPARAM lp)
{
    UINT uiMsg = (UINT) lp;

    switch (uiMsg)
    {
    case WM_RBUTTONUP:
        OnTrayContextMenu();
        return 1;

    case WM_LBUTTONUP:
    case WM_LBUTTONDBLCLK:
        OnAppOpen();
        return 1;
    }

    return 0;
}

//
// Show context menu for tray
//
void CMainFrame::OnTrayContextMenu ()
{
    CPoint point;
    ::GetCursorPos (&point);

    CMenu menu;
    menu.LoadMenu( IDR_TRAY_MENU );

    CMFCPopupMenu::SetForceShadow (TRUE);

    HMENU hMenu = menu.GetSubMenu (0)->Detach ();
    CMFCPopupMenu* pMenu = theApp.GetContextMenuManager()->ShowPopupMenu(hMenu, point.x, point.y, this, TRUE);
    pMenu->SetDefaultItem( ID_APP_OPEN );

    pMenu->SetForegroundWindow ();
}

void CMainFrame::OnAppOpen()
{
    if( IsIconic() )
    {
        ShowWindow( SW_SHOW );
        ShowWindow( SW_RESTORE );
        SetForegroundWindow();
        if( m_showMaximized )
        {
            ShowWindow( SW_SHOWMAXIMIZED );
        }
    }
    else
    {
        ShowWindow( SW_SHOW );
        SetForegroundWindow();
    }
    ShowOwnedPopups(TRUE);
}

//////////////////////////////////////////////////////////////////////////
// Add Tray icon
BOOL CMainFrame::AddTrayIcon()
{
    // Set tray notification window:
    m_nid.hWnd = GetSafeHwnd();
    m_nid.uCallbackMessage = UM_TRAYNOTIFY;
    // Set tray icon and tooltip:
    m_nid.hIcon = m_hIcon;
    CString strToolTip;
    strToolTip.LoadString( AFX_IDS_APP_TITLE );
    _tcsncpy_s( m_nid.szTip, sizeof(m_nid.szTip)/sizeof(m_nid.szTip[0]), strToolTip, strToolTip.GetLength() );
    return Shell_NotifyIcon( NIM_ADD, &m_nid );
}

LRESULT CMainFrame::OnTaskbarCreated(WPARAM,LPARAM)
{
    AddTrayIcon();
    return 0;
}

typedef WINUSERAPI BOOL (WINAPI *ChangeVistaWindowMessageFilter)( __in UINT message, __in DWORD dwFlag );

//////////////////////////////////////////////////////////////////////////
// Allow Shell Extension DLL send a WM_COPYDATA messages
void CMainFrame::AllowReceiveWMCopyData()
{
    // Get Windows version
    OSVERSIONINFO osvi;
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    ::GetVersionEx( &osvi );
    bool bIsWindowsXPorLater = (osvi.dwMajorVersion > 5) ||
        ( (osvi.dwMajorVersion == 5) && (osvi.dwMinorVersion >= 1) );

    bool bIsWindowsWistaorLater = (osvi.dwMajorVersion >= 6);
    if( bIsWindowsWistaorLater )
    {
        HMODULE user32dll = ::GetModuleHandle( _T("user32.dll") ); //::LoadLibrary( _T("user32.dll") );
        ATLASSERT( user32dll );
        if( user32dll )
        {
            ChangeVistaWindowMessageFilter ChangeVistaMessageFilter = ( ChangeVistaWindowMessageFilter )::GetProcAddress( user32dll, "ChangeWindowMessageFilter" );
            ATLASSERT( ChangeVistaMessageFilter );
            BOOL change = FALSE;
            if( ChangeVistaMessageFilter ) change = ChangeVistaMessageFilter( WM_COPYDATA, MSGFLT_ADD ); // Allow external call WM_COPYDATA
            if( !change )
            {
                ASSERT( FALSE );
                CString str; str.Format( _T(">>ChangeWindowMessageFilter() function return error 0x%lx"), ::GetLastError() );
                ATLTRACE( str );
            }
            //::FreeLibrary( user32dll ); // not free if use GetModuleHandle()
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// External calls - Shell Extension
LRESULT CMainFrame::OnExternalCall( WPARAM, LPARAM lParam )
{
    PCOPYDATASTRUCT copyData = reinterpret_cast<PCOPYDATASTRUCT>( lParam );
    if( !copyData ) return E_INVALIDARG;
    LRESULT res = 0;

    switch( copyData->dwData )
    {
    case ADEFRAG_ACTION_DEFRAG_PATHS:
        {
            CString pathToDefrag( (BSTR)copyData->lpData );
            res = GetDocument()->AddDefragJob( pathToDefrag );
            if( res == NO_ERROR )
            {
                //SetTimer( IdShowProgressTimer, 10, NULL );
                m_nProgressValue = 0;
                SetActiveWindow();
                SetForegroundWindow();
            }
        }
        return res;
    }

    return -1;
}

//////////////////////////////////////////////////////////////////////////
// External calls - Devices List Changed
LRESULT CMainFrame::OnDevicesListChanged( WPARAM wParam, LPARAM lParam )
{
    CDefragView* driveListView = GetDriveListView();
    //CWnd* devListView = m_splitterDefrag.GetPane(0,0);
    //return devListView ? devListView->PostMessage( WM_DEVICECHANGE, wParam, lParam ) : 0;
    return driveListView ? driveListView->PostMessage( WM_DEVICECHANGE, wParam, lParam ) : 0;
}

/******************************************************************************
*
*       Function :       OnHelp                                                         
*
*       Purpose :        Called by the main application to show Help
*
*       Return :         None
*
*       Parameters : None
*
*****************************************************************************/
void CMainFrame::OnHelp()
{
    AfxMessageBox( _T("call the help file") );
    WinHelp( HH_DISPLAY_TOPIC, 0 );
    /*HINSTANCE loader = NULL;
    if( (HINSTANCE)32 >= (loader = ::ShellExecute( m_hWnd, _T("open"), _T("\\manual.pdf"), NULL, NULL, SW_SHOWNORMAL ) ) )
    {
        int res = AfxMessageBox( _T("Failed to open User Manual (manual.pdf). Try to open Help file?"), MB_YESNO | MB_ICONEXCLAMATION );
        if( res == IDYES && _tcslen( theApp.m_pszHelpFilePath ) > 0 )
        {
            WinHelp( HH_DISPLAY_TOPIC, 0 );
            //HtmlHelp( HH_DISPLAY_TOPIC, 0 );
        }
    }*/
}

void CMainFrame::OnLink()
{
    CMFCRibbonLinkCtrl* pLink = (CMFCRibbonLinkCtrl*) m_wndStatusBar.FindByID(ID_INDICATOR_LABEL);
    if (pLink != NULL)
    {
        pLink->OpenLink();
    }
}


BOOL CMainFrame::Is_Vista_or_Later() 
{
	OSVERSIONINFOEX osvi;
	DWORDLONG dwlConditionMask = 0;
	int op=VER_GREATER_EQUAL;
    BOOL bRes;

	// Initialize the OSVERSIONINFOEX structure.
	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	osvi.dwMajorVersion = 6;

	// Initialize the condition mask.
	VER_SET_CONDITION( dwlConditionMask, VER_MAJORVERSION, op );

	// Perform the test.
    bRes = VerifyVersionInfo(
		&osvi, 
		VER_MAJORVERSION | VER_MINORVERSION | 
		VER_SERVICEPACKMAJOR | VER_SERVICEPACKMINOR,
		dwlConditionMask);

#ifdef _DEBUG
    if ( bRes )
    {
        OutMessage( L"IS VISTA" );
    }
    else
    {
        OutMessage( L"Is NOT Vista" );
    }
#endif

	return bRes;
}


void CMainFrame::OnDefragMostrecommended()
{
    //CommDfrg->AddDefragJob();
    GetDocument()->AddDefragJob( DEFRAG_CMD_SIMPLE_DEFRAG );
    //SetTimer( IdShowProgressTimer, 10, NULL );
    m_nProgressValue = 0;
}


void CMainFrame::OnUpdateDefragMostrecommended(CCmdUI *pCmdUI)
{
    BOOL State = GetDocument()->GetBtnState(RECCOMEND_STATE);
    pCmdUI->Enable( State );
}

void CMainFrame::OnDefragRecommend()
{
    //CommDfrg->AddDefragJob();
	int level = m_wndTaskPane.GetOveralHealthLevel();

	switch (level) {
		case DriveHealth::e_LevelWarning:
		    GetDocument()->AddDefragJob(DEFRAG_CMD_SIMPLE_DEFRAG);
			break;
		case DriveHealth::e_LevelCritical:
		    GetDocument()->AddDefragJob(DEFRAG_CMD_FORCE_TOGETHER);
			break;
		default:
			break;
	};

	m_nProgressValue = 0;
}

void CMainFrame::OnUpdateDefragRecommend(CCmdUI *pCmdUI)
{
    BOOL State = GetDocument()->GetBtnState(RECCOMEND_STATE);
    pCmdUI->Enable( State );
}
