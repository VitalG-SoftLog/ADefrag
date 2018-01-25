/*
Defines the class behaviors for the application.

Module name:

ADefragUI.cpp

Abstract:

Defines the class behaviors for the application.

$Header: /home/CVS_DEFRAG/Defrag/Src/ADefragUI/ADefragUI.cpp,v 1.4 2009/11/24 18:02:51 roman Exp $
$Log: ADefragUI.cpp,v $
Revision 1.4  2009/11/24 18:02:51  roman
Initial commit

Revision 1.3  2009/11/24 18:01:05  roman
Initial commit

*/

#include "stdafx.h"
#include "afxwinappex.h"
#include "ADefragUI.h"
#include "MainFrm.h"

#include "ADefragUIDoc.h"
#include "ADefragUIView.h"
#include "DriveMap.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////////
const TCHAR g_MainWindowClassName[] = ADEFRAG_APP_WINDOW_CLASS_GUID;

// CADefragUIApp

BEGIN_MESSAGE_MAP(CADefragUIApp, CWinAppEx)
    ON_COMMAND(ID_APP_ABOUT, &CADefragUIApp::OnAppAbout)
    ON_COMMAND(ID_FILE_NEW_FRAME, &CADefragUIApp::OnFileNewFrame)
    ON_COMMAND(ID_FILE_NEW, &CADefragUIApp::OnFileNew)
    // Standard file based document commands
    ON_COMMAND(ID_FILE_OPEN, &CWinAppEx::OnFileOpen)
    // Standard print setup command
    ON_COMMAND(ID_FILE_PRINT_SETUP, &CWinAppEx::OnFilePrintSetup)
END_MESSAGE_MAP()

//
// CADefragUIApp construction
//
CADefragUIApp::CADefragUIApp()
{

    m_bHiColorIcons = TRUE;

    // TODO: add construction code here,
    // Place all significant initialization in InitInstance
}

// The one and only CADefragUIApp object

CADefragUIApp theApp;

// CADefragUIApp initialization

BOOL CADefragUIApp::InitInstance()
{
    // Restore and activate loaded application
    HWND existingWindow = ::FindWindow( g_MainWindowClassName, NULL );
    BOOL isUninstallMode = FALSE;
    ProcessCmdUninstallMode( isUninstallMode );
    if( existingWindow )
    {
        if( isUninstallMode )
        {
            DWORD processId = 0;
            GetWindowThreadProcessId( existingWindow, &processId );
            HANDLE process = OpenProcess( PROCESS_ALL_ACCESS, FALSE, processId );
            ::PostMessage( existingWindow, WM_CLOSE, 0, 0L );
            if( process )
            {
                if( WAIT_TIMEOUT == ::WaitForSingleObject( process, 10000 ) ) // INFINITE );
                {
                    ::TerminateProcess( process, 0 );
                }
                ::CloseHandle( process );
            }
            ::Sleep(200);
        }
        else
        {
            if( ::IsIconic( existingWindow ) )
                ::ShowWindow( existingWindow, SW_RESTORE );
            else
                ::ShowWindow( existingWindow, SW_SHOW );
            ::SetForegroundWindow( existingWindow );
        }
        return FALSE;
    }
    if( isUninstallMode ) return FALSE;

    // InitCommonControlsEx() is required on Windows XP if an application
    // manifest specifies use of ComCtl32.dll version 6 or later to enable
    // visual styles.  Otherwise, any window creation will fail.
    INITCOMMONCONTROLSEX InitCtrls;
    InitCtrls.dwSize = sizeof(InitCtrls);
    // Set this to include all the common control classes you want to use
    // in your application.
    InitCtrls.dwICC = ICC_WIN95_CLASSES;
    InitCommonControlsEx(&InitCtrls);

    CWinAppEx::InitInstance();

    // Initialize OLE libraries
    if (!AfxOleInit())
    {
        AfxMessageBox(IDP_OLE_INIT_FAILED);
        return FALSE;
    }
    AfxEnableControlContainer();

    if( !RegisterADefragClassName() )
    {
        return FALSE;
    }
    // Standard initialization
    // If you are not using these features and wish to reduce the size
    // of your final executable, you should remove from the following
    // the specific initialization routines you do not need
    // Change the registry key under which our settings are stored
    // TODO: You should modify this string to be something appropriate
    // such as the name of your company or organization
    SetRegistryKey(_T("ADefrag"));
    SetRegistryBase(_T("Settings"));

    LoadStdProfileSettings(0);  // Load standard INI file options

    InitContextMenuManager();
    InitShellManager();
    InitKeyboardManager();

    InitTooltipManager();
    CMFCToolTipInfo ttParams;
    ttParams.m_bVislManagerTheme = TRUE;
    theApp.GetTooltipManager()->SetTooltipParams(AFX_TOOLTIP_TYPE_ALL,
        RUNTIME_CLASS(CMFCToolTipCtrl), &ttParams);

    // Register the application's document templates.  Document templates
    //  serve as the connection between documents, frame windows and views
    CMultiDocTemplate* pDocTemplate;
    pDocTemplate = new CMultiDocTemplate(
        IDR_MAINFRAME,
        RUNTIME_CLASS(CADefragUIDoc),
        RUNTIME_CLASS(CMainFrame),       // main SDI frame window
        RUNTIME_CLASS(CADefragUIView));
    if (!pDocTemplate)
        return FALSE;
    pDocTemplate->SetContainerInfo(IDR_CNTR_INPLACE);
    m_pDocTemplate = pDocTemplate;
    AddDocTemplate(pDocTemplate);


    // Enable DDE Execute open
    //EnableShellOpen();
    //RegisterShellFileTypes(TRUE);

    // Parse command line for standard shell commands, DDE, file open
    CCommandLineInfo cmdInfo;
    ParseCommandLine(cmdInfo);


    // Dispatch commands specified on the command line.  Will return FALSE if
    // app was launched with /RegServer, /Register, /Unregserver or /Unregister.
    if (!ProcessShellCommand(cmdInfo))
        return FALSE;

    ((CMainFrame*)m_pMainWnd)->InitViews();

    // The one and only window has been initialized, so show and update it
    m_pMainWnd->ShowWindow(SW_SHOW);
    m_pMainWnd->UpdateWindow();
    // call DragAcceptFiles only if there's a suffix
    //  In an SDI app, this should occur after ProcessShellCommand
    // Enable drag/drop open
    // m_pMainWnd->DragAcceptFiles();
    return TRUE;
}


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
    CAboutDlg();

    // Dialog Data
    enum { IDD = IDD_ABOUTBOX };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    // Implementation
protected:
    DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

//
// App command to run the dialog
//
void CADefragUIApp::OnAppAbout()
{
    CAboutDlg aboutDlg;
    aboutDlg.DoModal();
}

// CADefragUIApp customization load/save methods

void CADefragUIApp::PreLoadState()
{
    BOOL bNameValid;
    CString strName;
    bNameValid = strName.LoadString(IDS_EDIT_MENU);
    ASSERT(bNameValid);
    GetContextMenuManager()->AddMenu(strName, IDR_POPUP_EDIT);

    //
    // Load popup menu for DefragView
    //
    bNameValid = strName.LoadString( IDS_DEFRAGVIEW_MENU );
    ASSERT(bNameValid);
    GetContextMenuManager()->AddMenu( strName, IDR_POPUP_DFRAGVIEW );

    //
    // Load popup menu for DriveMapView
    //
    bNameValid = strName.LoadString( IDS_DEFRAGVIEW_MENU );
    ASSERT(bNameValid);
    GetContextMenuManager()->AddMenu( strName, IDR_POPUP_DRIVEMAPVIEW );

    //
    // Load popup menu for SchedulerView
    //
    bNameValid = strName.LoadString( IDS_SCHEDULERVIEW_MENU );
    ASSERT(bNameValid);
    GetContextMenuManager()->AddMenu( strName, IDR_POPUP_SCHEDULERVIEW );
}

void CADefragUIApp::LoadCustomState()
{
}

void CADefragUIApp::SaveCustomState()
{
}

// CADefragUIApp message handlers

void CADefragUIApp::OnFileNewFrame() 
{
    ASSERT(m_pDocTemplate != NULL);

    CDocument* pDoc = NULL;
    CFrameWnd* pFrame = NULL;

    // Create a new instance of the document referenced
    // by the m_pDocTemplate member.
    if (m_pDocTemplate != NULL)
        pDoc = m_pDocTemplate->CreateNewDocument();

    if (pDoc != NULL)
    {
        // If creation worked, use create a new frame for
        // that document.
        pFrame = m_pDocTemplate->CreateNewFrame(pDoc, NULL);
        if (pFrame != NULL)
        {
            // Set the title, and initialize the document.
            // If document initialization fails, clean-up
            // the frame window and document.

            m_pDocTemplate->SetDefaultTitle(pDoc);
            if (!pDoc->OnNewDocument())
            {
                pFrame->DestroyWindow();
                pFrame = NULL;
            }
            else
            {
                // Otherwise, update the frame
                m_pDocTemplate->InitialUpdateFrame(pFrame, pDoc, TRUE);
            }
        }
    }

    // If we failed, clean up the document and show a
    // message to the user.

    if (pFrame == NULL || pDoc == NULL)
    {
        delete pDoc;
        AfxMessageBox(AFX_IDP_FAILED_TO_CREATE_DOC);
    }
}

void CADefragUIApp::OnFileNew() 
{
    CDocument* pDoc = NULL;
    CFrameWnd* pFrame;
    pFrame = DYNAMIC_DOWNCAST(CFrameWnd, CWnd::GetActiveWindow());

    if (pFrame != NULL)
        pDoc = pFrame->GetActiveDocument();

    if (pFrame == NULL || pDoc == NULL)
    {
        // if it's the first document, create as normal
        CWinApp::OnFileNew();
    }
    else
    {
        // Otherwise, see if we have to save modified, then
        // ask the document to reinitialize itself.
        if (!pDoc->SaveModified())
            return;

        CDocTemplate* pTemplate = pDoc->GetDocTemplate();
        ASSERT(pTemplate != NULL);

        if (pTemplate != NULL)
            pTemplate->SetDefaultTitle(pDoc);
        pDoc->OnNewDocument();
    }
}

/////////////////////////////////////////////////////////////////////////////
// Register your unique class name that you wish to use
BOOL CADefragUIApp::RegisterADefragClassName()
{
    // Register your unique class name that you wish to use
    WNDCLASS wndcls;

    memset(&wndcls, 0, sizeof(WNDCLASS));   // start with NULL
    // defaults

    wndcls.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;

    //you can specify your own window procedure
    wndcls.lpfnWndProc = ::DefWindowProc;
    wndcls.hInstance = AfxGetInstanceHandle();
    wndcls.hIcon = LoadIcon(IDR_MAINFRAME); // or load a different icon
    wndcls.hCursor = LoadCursor( IDC_ARROW );
    wndcls.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
    wndcls.lpszMenuName = NULL;

    // Specify your own class name for using FindWindow later
    wndcls.lpszClassName = g_MainWindowClassName;

    // Register the new class and exit if it fails
    if(!AfxRegisterClass(&wndcls))
    {
        TRACE("Class Registration Failed\n");
        return FALSE;
    }
    return TRUE;
}

//////////////////////////////////////////////////////////////////////////
// Check command line for uninstallation process 
void CADefragUIApp::ProcessCmdUninstallMode( BOOL& isUninstall )
{
    isUninstall = FALSE;
    for (int i = 1; i < __argc; i++)
    {
        CString pszParam = __targv[i];
        if( pszParam.CompareNoCase( _T("/uninstall") ) == 0 )
        {
            isUninstall = TRUE;
            break;
        }
    }
    if( isUninstall )
    {
        CRegKey key;
        if( ERROR_SUCCESS == key.Open( HKEY_LOCAL_MACHINE, _T("System\\CurrentControlSet\\Control\\Session Manager") ) )
        {
            ULONG bufSize = 1000;
            CString BootExecute;
            if( ERROR_SUCCESS == key.QueryMultiStringValue( _T("BootExecute"), BootExecute.GetBufferSetLength(bufSize), &bufSize ) )
            {
                BootExecute.ReleaseBufferSetLength( bufSize );
                BootExecute.Replace( _T("BootDfrg.exe\0"), _T("") );
                key.SetMultiStringValue( _T("BootExecute"), BootExecute );
            }
        }
    }
}
