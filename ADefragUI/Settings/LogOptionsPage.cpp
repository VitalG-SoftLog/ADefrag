/*
    Interface of the CLogOptionsPage class

    Module name:

    LogOptionsPage.cpp

    Abstract:

    Interface of the CLogOptionsPage class

    $Header: /home/CVS_DEFRAG/Defrag/Src/ADefragUI/Settings/LogOptionsPage.cpp,v 1.2 2009/11/24 18:02:52 roman Exp $
    $Log: LogOptionsPage.cpp,v $
    Revision 1.2  2009/11/24 18:02:52  roman
    Initial commit

    Revision 1.1  2009/11/24 14:56:45  dimas
    no message

*/

#include "stdafx.h"
#include "LogOptionsPage.h"
#include "../ADefragUI.h"
#include "../MainFrm.h"
#include "../ADefragUIDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLogOptionsPage property page

IMPLEMENT_DYNCREATE(CLogOptionsPage, CMFCPropertyPage)

CLogOptionsPage::CLogOptionsPage() : CMFCPropertyPage(CLogOptionsPage::IDD)
    , m_EnableApplicationLog( FALSE )
    , m_EnableEventLog( FALSE )
{
    //{{AFX_DATA_INIT(CLogOptionsPage)
    //m_EnableApplicationLog = theApp.GetInt( _T("Log.EnableApplicationLog"), FALSE );
    //m_EnableEventLog = theApp.GetInt( _T("Log.EnableEventLog"), FALSE );
    //}}AFX_DATA_INIT
}

CLogOptionsPage::~CLogOptionsPage()
{
}

void CLogOptionsPage::DoDataExchange(CDataExchange* pDX)
{
    CMFCPropertyPage::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CLogOptionsPage)
    DDX_Control(pDX, IDC_LOG_BANER, m_wndBanner);
    DDX_Check(pDX, IDC_APP_LOG_CHECK, m_EnableApplicationLog );
    DDX_Check(pDX, IDC_EVENT_LOG_CHECK, m_EnableEventLog );
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CLogOptionsPage, CMFCPropertyPage)
    //{{AFX_MSG_MAP(CLogOptionsPage)
    ON_WM_SIZE()
    ON_BN_CLICKED(IDC_VIEW_LOG, &CLogOptionsPage::OnBnClickedViewLog)
    ON_BN_CLICKED(IDC_CLEAR_LOG, &CLogOptionsPage::OnBnClickedClearLog)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLogOptionsPage message handlers

void CLogOptionsPage::OnOK()
{
    UpdateData();

    //theApp.WriteInt( _T("Log.EnableApplicationLog"), m_EnableApplicationLog );
    //theApp.WriteInt( _T("Log.EnableEventLog"), m_EnableEventLog );
    CADefragUIDoc* doc = GetMainDocument();
    if( doc )
    {
        DWORD isApplicationLogEnabled = m_EnableApplicationLog;
        if( ERROR_SUCCESS != doc->SetSettingsValue( ApplicationLogEnableId, &isApplicationLogEnabled, sizeof(isApplicationLogEnabled) ) )
        {
            ASSERT( FALSE );
        }
        DWORD isEventLogEnabled = m_EnableEventLog;
        if( ERROR_SUCCESS != doc->SetSettingsValue( EventLogEnableId, &isEventLogEnabled, sizeof(isEventLogEnabled) ) )
        {
            ASSERT( FALSE );
        }
    }

    __super::OnOK();
}

BOOL CLogOptionsPage::OnInitDialog()
{
    __super::OnInitDialog();

    CADefragUIDoc* doc = GetMainDocument();
    if( doc )
    {
        DWORD isApplicationLogEnabled = m_EnableApplicationLog;
        if( ERROR_SUCCESS == doc->GetSettingsValue( ApplicationLogEnableId, &isApplicationLogEnabled, sizeof(isApplicationLogEnabled) ) )
        {
            m_EnableApplicationLog = isApplicationLogEnabled ? TRUE : FALSE;
        }
        DWORD isEventLogEnabled = m_EnableEventLog;
        if( ERROR_SUCCESS == doc->GetSettingsValue( EventLogEnableId, &isEventLogEnabled, sizeof(isEventLogEnabled) ) )
        {
            m_EnableEventLog = isEventLogEnabled ? TRUE : FALSE;
        }

        if( ERROR_SUCCESS != doc->GetSettingsValue( LogFilePathId, m_LogFilePath.GetBuffer(MAX_PATH), MAX_PATH * sizeof(m_LogFilePath[0]) ) )
        {
            ASSERT( FALSE );
        }
        m_LogFilePath.ReleaseBuffer();
    }
    UpdateData( FALSE );

    return TRUE;  // return TRUE unless you set the focus to a control
}

void CLogOptionsPage::OnSize(UINT nType, int cx, int cy)
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


void CLogOptionsPage::OnBnClickedViewLog()
{
    if( (HINSTANCE)32 >= ::ShellExecute( m_hWnd, _T("open"), m_LogFilePath, NULL, NULL, SW_SHOWNORMAL ) )
    {
        CString message;
        message.Format( IDS_LOGFILE_VIEW_FAILED, m_LogFilePath );
        AfxMessageBox( message, MB_OK | MB_ICONEXCLAMATION );
    }
}

void CLogOptionsPage::OnBnClickedClearLog()
{
    if( ::DeleteFile( m_LogFilePath ) )
    {
        AfxMessageBox( IDS_LOGFILE_CLEAR_OK, MB_OK | MB_ICONINFORMATION );
    }
    else
    {
        AfxMessageBox( IDS_LOGFILE_CLEAR_FAILED, MB_OK | MB_ICONINFORMATION );
    }
}
