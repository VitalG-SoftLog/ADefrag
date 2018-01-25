/*
    Interface of the CSchedulerOptionsPage class

    Module name:

    SchedulerOptionsPage.cpp

    Abstract:

    Interface of the CSchedulerOptionsPage class

    $Header: /home/CVS_DEFRAG/Defrag/Src/ADefragUI/Settings/SchedulerOptionsPage.cpp,v 1.2 2009/11/24 18:02:52 roman Exp $
    $Log: SchedulerOptionsPage.cpp,v $
    Revision 1.2  2009/11/24 18:02:52  roman
    Initial commit

    Revision 1.1  2009/11/24 14:56:45  dimas
    no message

*/

#include "stdafx.h"
#include "SchedulerOptionsPage.h"
#include "../ADefragUI.h"
#include "../MainFrm.h"
#include "../ADefragUIDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSchedulerOptionsPage property page

IMPLEMENT_DYNCREATE(CSchedulerOptionsPage, CMFCPropertyPage)

CSchedulerOptionsPage::CSchedulerOptionsPage() : CMFCPropertyPage(CSchedulerOptionsPage::IDD)
    , m_NotStartBattery( FALSE )
    , m_StopBattery( FALSE )
    , m_WakeRun( FALSE )
    , m_RunMissed( FALSE )
{
    //{{AFX_DATA_INIT(CSchedulerOptionsPage)
    /*m_NotStartBattery = theApp.GetInt( _T("Scheduler.NotStartBattery"), FALSE );
    m_StopBattery     = theApp.GetInt( _T("Scheduler.StopBattery"), FALSE );
    m_WakeRun         = theApp.GetInt( _T("Scheduler.WakeRun"), FALSE );
    m_RunMissed       = theApp.GetInt( _T("Scheduler.RunMissed"), FALSE );*/
    //}}AFX_DATA_INIT
}

CSchedulerOptionsPage::~CSchedulerOptionsPage()
{
}

void CSchedulerOptionsPage::DoDataExchange(CDataExchange* pDX)
{
    CMFCPropertyPage::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CSchedulerOptionsPage)
    DDX_Control(pDX, IDC_BANER, m_wndBanner);
    DDX_Check(pDX, IDC_NOT_START_BATTERY, m_NotStartBattery );
    DDX_Check(pDX, IDC_STOP_BATTERY, m_StopBattery );
    DDX_Check(pDX, IDC_WAKE_RUN, m_WakeRun );
    DDX_Check(pDX, IDC_RUN_MISSED, m_RunMissed );
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSchedulerOptionsPage, CMFCPropertyPage)
    //{{AFX_MSG_MAP(CSchedulerOptionsPage)
    ON_WM_SIZE()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSchedulerOptionsPage message handlers

void CSchedulerOptionsPage::OnOK()
{
    UpdateData();

    /*theApp.WriteInt( _T("Scheduler.NotStartBattery"), m_NotStartBattery );
    theApp.WriteInt( _T("Scheduler.StopBattery"), m_StopBattery );
    theApp.WriteInt( _T("Scheduler.WakeRun"), m_WakeRun );
    theApp.WriteInt( _T("Scheduler.RunMissed"), m_RunMissed );*/
    CADefragUIDoc* doc = GetMainDocument();
    if( doc )
    {
        DWORD isNotStartBatteryEnabled = m_NotStartBattery;
        if( ERROR_SUCCESS != doc->SetSettingsValue( SchedulerNotStartBatteryEnableId, &isNotStartBatteryEnabled, sizeof(isNotStartBatteryEnabled) ) )
        {
            ASSERT( FALSE );
        }
        DWORD isStopBatteryEnabled = m_StopBattery;
        if( ERROR_SUCCESS != doc->SetSettingsValue( SchedulerStopBatteryEnableId, &isStopBatteryEnabled, sizeof(isStopBatteryEnabled) ) )
        {
            ASSERT( FALSE );
        }
        DWORD isWakeRunEnabled = m_WakeRun;
        if( ERROR_SUCCESS != doc->SetSettingsValue( SchedulerWakeRunEnableId, &isWakeRunEnabled, sizeof(isWakeRunEnabled) ) )
        {
            ASSERT( FALSE );
        }
        DWORD isRunMissedEnabled = m_RunMissed;
        if( ERROR_SUCCESS != doc->SetSettingsValue( SchedulerRunMissedEnableId, &isRunMissedEnabled, sizeof(isRunMissedEnabled) ) )
        {
            ASSERT( FALSE );
        }
    }

    CMFCPropertyPage::OnOK();
}

BOOL CSchedulerOptionsPage::OnInitDialog()
{
    CMFCPropertyPage::OnInitDialog();

    CADefragUIDoc* doc = GetMainDocument();
    if( doc )
    {
        DWORD isNotStartBatteryEnabled = m_NotStartBattery;
        if( ERROR_SUCCESS == doc->GetSettingsValue( SchedulerNotStartBatteryEnableId, &isNotStartBatteryEnabled, sizeof(isNotStartBatteryEnabled) ) )
        {
            m_NotStartBattery = isNotStartBatteryEnabled ? TRUE : FALSE;
        }
        DWORD isStopBatteryEnabled = m_StopBattery;
        if( ERROR_SUCCESS == doc->GetSettingsValue( SchedulerStopBatteryEnableId, &isStopBatteryEnabled, sizeof(isStopBatteryEnabled) ) )
        {
            m_StopBattery = isStopBatteryEnabled ? TRUE : FALSE;
        }
        DWORD isWakeRunEnabled = m_WakeRun;
        if( ERROR_SUCCESS == doc->GetSettingsValue( SchedulerWakeRunEnableId, &isWakeRunEnabled, sizeof(isWakeRunEnabled) ) )
        {
            m_WakeRun = isWakeRunEnabled ? TRUE : FALSE;
        }
        DWORD isRunMissedEnabled = m_RunMissed;
        if( ERROR_SUCCESS == doc->GetSettingsValue( SchedulerRunMissedEnableId, &isRunMissedEnabled, sizeof(isRunMissedEnabled) ) )
        {
            m_RunMissed = isRunMissedEnabled ? TRUE : FALSE;
        }
    }
    UpdateData( FALSE );

    return TRUE;  // return TRUE unless you set the focus to a control
}

void CSchedulerOptionsPage::OnSize(UINT nType, int cx, int cy)
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

