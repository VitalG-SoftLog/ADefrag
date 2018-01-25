/*
    Interface of the CPopularOptionsPage class

    Module name:

    PopularOptionsPage.cpp

    Abstract:

    Interface of the CPopularOptionsPage class

    $Header: /home/CVS_DEFRAG/Defrag/Src/ADefragUI/Settings/PopularOptionsPage.cpp,v 1.2 2009/11/24 18:02:52 roman Exp $
    $Log: PopularOptionsPage.cpp,v $
    Revision 1.2  2009/11/24 18:02:52  roman
    Initial commit

    Revision 1.1  2009/11/24 14:56:45  dimas
    no message

*/

#include "stdafx.h"
#include "PopularOptionsPage.h"
#include "OptionsPropSheet.h"
#include "../ADefragUI.h"
#include "../MainFrm.h"
#include "../ADefragUIDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPopularOptionsPage property page

IMPLEMENT_DYNCREATE(CPopularOptionsPage, CMFCPropertyPage)

CPopularOptionsPage::CPopularOptionsPage() : CMFCPropertyPage(CPopularOptionsPage::IDD)
    , m_BootOptimization( FALSE )
    , m_AutoUpdate( FALSE )
{
    //{{AFX_DATA_INIT(CPopularOptionsPage)
    m_nColorScheme = theApp.m_nAppLook - ID_VIEW_APPLOOK_OFF_2007_BLUE;
    m_nTooltipStyle = theApp.GetInt( _T("TooltipStyle"), 0 );
    //m_BootOptimization = theApp.GetInt( _T("BootOptimization"), FALSE );
    //m_AutoUpdate = theApp.GetInt( _T("Update.AutoUpdate"), FALSE );
    //}}AFX_DATA_INIT
}

CPopularOptionsPage::~CPopularOptionsPage()
{
}

void CPopularOptionsPage::DoDataExchange(CDataExchange* pDX)
{
    CMFCPropertyPage::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CPopularOptionsPage)
    DDX_Control(pDX, IDC_BANER, m_wndBanner);
    DDX_Control(pDX, IDC_STARTUP_BANER, m_StartupBanner);
    DDX_Control(pDX, IDC_UPDATE_BANER, m_UpdateBanner);
    DDX_Control(pDX, IDC_COLOR_SCHEME, m_wndColorScheme);
    DDX_CBIndex(pDX, IDC_COLOR_SCHEME, m_nColorScheme);
    DDX_CBIndex(pDX, IDC_SCREENTIP_STYLE, m_nTooltipStyle);
    DDX_Check(pDX, IDC_BOOT_OPTIMIZATION, m_BootOptimization );
    DDX_Check(pDX, IDC_AUTO_UPDATE, m_AutoUpdate );
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CPopularOptionsPage, CMFCPropertyPage)
    //{{AFX_MSG_MAP(CPopularOptionsPage)
    ON_WM_SIZE()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPopularOptionsPage message handlers

void CPopularOptionsPage::OnOK()
{
    UpdateData();

    theApp.m_nAppLook = m_nColorScheme + ID_VIEW_APPLOOK_OFF_2007_BLUE;
    theApp.WriteInt( _T("TooltipStyle"), m_nTooltipStyle );
    //theApp.WriteInt( _T("BootOptimization"), m_BootOptimization );
    //theApp.WriteInt( _T("Update.AutoUpdate"), m_AutoUpdate );

    CADefragUIDoc* doc = GetMainDocument();
    if( doc )
    {
        DWORD isBootOptimizationEnabled = m_BootOptimization ? 1 : 0;
        if( ERROR_SUCCESS != doc->SetSettingsValue( BootOptimizationEnableId, &isBootOptimizationEnabled, sizeof(isBootOptimizationEnabled) ) )
        {
            ASSERT( FALSE );
        }
        DWORD isAutoUpdateEnabled = m_AutoUpdate ? 1 : 0;
        if( ERROR_SUCCESS != doc->SetSettingsValue( AutoUpdateEnableId, &isAutoUpdateEnabled, sizeof(isAutoUpdateEnabled) ) )
        {
            ASSERT( FALSE );
        }
    }

    CMFCPropertyPage::OnOK();
}

BOOL CPopularOptionsPage::OnSetActive()
{
    CDefragOptionsPropSheet* propSheet = dynamic_cast<CDefragOptionsPropSheet*>( GetParent() );
    if( propSheet ) m_AutoUpdate = propSheet->GetAutoUpdateStatus();
    UpdateData( FALSE );
    return __super::OnSetActive();
}

BOOL CPopularOptionsPage::OnKillActive()
{
    CDefragOptionsPropSheet* propSheet = dynamic_cast<CDefragOptionsPropSheet*>( GetParent() );
    UpdateData();
    if( propSheet ) propSheet->SetAutoUpdateStatus( m_AutoUpdate );
    return __super::OnSetActive();
}

BOOL CPopularOptionsPage::OnInitDialog()
{
    CMFCPropertyPage::OnInitDialog();

    CADefragUIDoc* doc = GetMainDocument();
    CDefragOptionsPropSheet* propSheet = dynamic_cast<CDefragOptionsPropSheet*>( GetParent() );
    if( propSheet && propSheet->GetAutoUpdateStatus() >= 0 )
    {
        m_AutoUpdate = propSheet->GetAutoUpdateStatus();
    }
    else if( doc )
    {
        DWORD isBootOptimizationEnabled = m_BootOptimization;
        if( ERROR_SUCCESS == doc->GetSettingsValue( BootOptimizationEnableId, &isBootOptimizationEnabled, sizeof(isBootOptimizationEnabled) ) )
        {
            m_BootOptimization = isBootOptimizationEnabled ? TRUE : FALSE;
        }
        DWORD isAutoUpdateEnabled = m_AutoUpdate;
        if( ERROR_SUCCESS == doc->GetSettingsValue( AutoUpdateEnableId, &isAutoUpdateEnabled, sizeof(isAutoUpdateEnabled) ) )
        {
            m_AutoUpdate = isAutoUpdateEnabled ? TRUE : FALSE;
            if( propSheet ) propSheet->SetAutoUpdateStatus( isAutoUpdateEnabled );
        }
    }
    UpdateData( FALSE );

    return TRUE;  // return TRUE unless you set the focus to a control
}

void CPopularOptionsPage::OnSize(UINT nType, int cx, int cy)
{
    CMFCPropertyPage::OnSize(nType, cx, cy);

    if (m_wndBanner.GetSafeHwnd() == NULL)
    {
        return;
    }

    CRect rectParent;
    GetParent()->GetWindowRect(rectParent);

    CRect rectBanner;
    m_wndBanner.GetWindowRect(rectBanner);
    m_wndBanner.SetWindowPos(NULL, -1, -1, rectParent.right - rectBanner.left - 10, rectBanner.Height(), SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);

    CRect rectStartupBanner;
    m_StartupBanner.GetWindowRect(rectStartupBanner);
    m_StartupBanner.SetWindowPos(NULL, -1, -1, rectParent.right - rectBanner.left - 10, rectStartupBanner.Height(), SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);

    CRect rectUpdateBanner;
    m_UpdateBanner.GetWindowRect(rectUpdateBanner);
    m_UpdateBanner.SetWindowPos(NULL, -1, -1, rectParent.right - rectBanner.left - 10, rectUpdateBanner.Height(), SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
}

