/*
    Interface of the CUpdateOptionsPage class

    Module name:

    UpdateOptionsPage.cpp

    Abstract:

    Interface of the CUpdateOptionsPage class

    $Header: /home/CVS_DEFRAG/Defrag/Src/ADefragUI/Settings/UpdateOptionsPage.cpp,v 1.2 2009/11/24 18:02:52 roman Exp $
    $Log: UpdateOptionsPage.cpp,v $
    Revision 1.2  2009/11/24 18:02:52  roman
    Initial commit

    Revision 1.1  2009/11/24 14:56:45  dimas
    no message

*/

#include "stdafx.h"
#include "UpdateOptionsPage.h"
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
// CUpdateOptionsPage property page

IMPLEMENT_DYNCREATE(CUpdateOptionsPage, CMFCPropertyPage)

CUpdateOptionsPage::CUpdateOptionsPage() : CMFCPropertyPage(CUpdateOptionsPage::IDD)
    , m_EnableAutoUpdate( FALSE )
{
    //{{AFX_DATA_INIT(CUpdateOptionsPage)
    //m_EnableAutoUpdate = theApp.GetInt( _T("Update.EnableAutoUpdate"), FALSE );
    //}}AFX_DATA_INIT
}

CUpdateOptionsPage::~CUpdateOptionsPage()
{
}

void CUpdateOptionsPage::DoDataExchange(CDataExchange* pDX)
{
    CMFCPropertyPage::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CUpdateOptionsPage)
    DDX_Control(pDX, IDC_BANER, m_wndBanner);
    DDX_Check(pDX, IDC_AUTO_UPDATE, m_EnableAutoUpdate );
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CUpdateOptionsPage, CMFCPropertyPage)
    //{{AFX_MSG_MAP(CUpdateOptionsPage)
    ON_WM_SIZE()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUpdateOptionsPage message handlers

void CUpdateOptionsPage::OnOK()
{
    UpdateData();

    //theApp.WriteInt( _T("Update.EnableAutoUpdate"), m_EnableAutoUpdate );
    CADefragUIDoc* doc = GetMainDocument();
    if( doc )
    {
        DWORD isAutoUpdateEnabled = m_EnableAutoUpdate ? 1 : 0;
        if( ERROR_SUCCESS != doc->SetSettingsValue( AutoUpdateEnableId, &isAutoUpdateEnabled, sizeof(isAutoUpdateEnabled) ) )
        {
            ASSERT( FALSE );
        }
    }

    CMFCPropertyPage::OnOK();
}

BOOL CUpdateOptionsPage::OnSetActive()
{
    CDefragOptionsPropSheet* propSheet = dynamic_cast<CDefragOptionsPropSheet*>( GetParent() );
    if( propSheet ) m_EnableAutoUpdate = propSheet->GetAutoUpdateStatus();
    UpdateData( FALSE );
    return __super::OnSetActive();
}

BOOL CUpdateOptionsPage::OnKillActive()
{
    CDefragOptionsPropSheet* propSheet = dynamic_cast<CDefragOptionsPropSheet*>( GetParent() );
    UpdateData();
    if( propSheet ) propSheet->SetAutoUpdateStatus( m_EnableAutoUpdate );
    return __super::OnSetActive();
}

BOOL CUpdateOptionsPage::OnInitDialog()
{
    CMFCPropertyPage::OnInitDialog();

    CDefragOptionsPropSheet* propSheet = dynamic_cast<CDefragOptionsPropSheet*>( GetParent() );
    CADefragUIDoc* doc = GetMainDocument();
    if( propSheet && propSheet->GetAutoUpdateStatus() >= 0 )
    {
        m_EnableAutoUpdate = propSheet->GetAutoUpdateStatus();
    }
    else if( doc )
    {
        DWORD isAutoUpdateEnabled = m_EnableAutoUpdate;
        if( ERROR_SUCCESS == doc->GetSettingsValue( AutoUpdateEnableId, &isAutoUpdateEnabled, sizeof(isAutoUpdateEnabled) ) )
        {
            m_EnableAutoUpdate = isAutoUpdateEnabled ? TRUE : FALSE;
            if( propSheet ) propSheet->SetAutoUpdateStatus( isAutoUpdateEnabled );
        }
    }
    UpdateData( FALSE );

    return TRUE;  // return TRUE unless you set the focus to a control
}

void CUpdateOptionsPage::OnSize(UINT nType, int cx, int cy)
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

