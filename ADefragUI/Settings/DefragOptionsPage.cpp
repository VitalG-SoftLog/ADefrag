/*
    Interface of the CDefragOptionsPage class

    Module name:

    DefragOptionsPage.cpp

    Abstract:

    Interface of the CDefragOptionsPage class

    $Header: /home/CVS_DEFRAG/Defrag/Src/ADefragUI/Settings/DefragOptionsPage.cpp,v 1.2 2009/11/24 18:02:52 roman Exp $
    $Log: DefragOptionsPage.cpp,v $
    Revision 1.2  2009/11/24 18:02:52  roman
    Initial commit

    Revision 1.1  2009/11/24 14:56:44  dimas
    no message

*/

#include "stdafx.h"
#include "DefragOptionsPage.h"
#include "../ADefragUI.h"
#include "../MainFrm.h"
#include "../ADefragUIDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDefragOptionsPage property page

IMPLEMENT_DYNCREATE(CDefragOptionsPage, CMFCPropertyPage)

CDefragOptionsPage::CDefragOptionsPage() : CMFCPropertyPage(CDefragOptionsPage::IDD)
    , m_AllowDefragFlash( FALSE )
    , m_AllowDefragRemovable( FALSE )
    , m_EnableThreshold( FALSE )
    , m_Threshold( 50 )
    , m_AggressivelyFreeSpace( FALSE )
    , m_AdjustCpuPriority( FALSE )
    , m_CpuPriority( 0 )
    , m_Throttle( FALSE )
    , m_VssEnable( FALSE )
    , m_VssMode( 0 )
{
    //{{AFX_DATA_INIT(CDefragOptionsPage)
    /*m_AllowDefragFlash = theApp.GetInt( _T("Defrag.AllowDefragFlash"), FALSE );
    m_AllowDefragRemovable = theApp.GetInt( _T("Defrag.AllowDefragRemovable"), FALSE );
    m_EnableThreshold = theApp.GetInt( _T("Defrag.EnableThreshold"), FALSE );
    m_Threshold = theApp.GetInt( _T("Defrag.Threshold"), 50 );
    m_AggressivelyFreeSpace = theApp.GetInt( _T("Defrag.AggressivelyFreeSpace"), FALSE );
    m_AdjustCpuPriority = theApp.GetInt( _T("Defrag.AdjustCpuPriority"), FALSE );
    m_CpuPriority = theApp.GetInt( _T("Defrag.CpuPriority"), 0 );
    m_Throttle = theApp.GetInt( _T("Defrag.Throttle"), FALSE );
    m_VssEnable = theApp.GetInt( _T("Defrag.VssEnable"), FALSE );
    m_VssMode = theApp.GetInt( _T("Defrag.VssMode"), 0 );*/
    //}}AFX_DATA_INIT
}

CDefragOptionsPage::~CDefragOptionsPage()
{
}

void CDefragOptionsPage::DoDataExchange(CDataExchange* pDX)
{
    CMFCPropertyPage::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CDefragOptionsPage)
    DDX_Control(pDX, IDC_BANER, m_wndBanner);
    DDX_Check(pDX, IDC_ALLOW_DEFRAG_FLASH, m_AllowDefragFlash );
    DDX_Check(pDX, IDC_ALLOW_DEFRAG_REMOVABLE, m_AllowDefragRemovable );

    DDX_Check(pDX, IDC_ENABLE_THRESHOLD, m_EnableThreshold );
    DDX_Text(pDX, IDC_THRESHOLD, m_Threshold );
    DDV_MinMaxInt(pDX, m_Threshold, 1, 100 );
    DDX_Control(pDX, IDC_THRESHOLD_SPIN, m_thresholdSpin);

    DDX_Check(pDX, IDC_AGGRESSIVELY_FREE_SPACE, m_AggressivelyFreeSpace );

    DDX_Check(pDX, IDC_ADJUST_CPU_PRIORITY, m_AdjustCpuPriority );
    DDX_Radio(pDX, IDC_PRIORITY_NORMAL, m_CpuPriority );

    DDX_Check(pDX, IDC_THROTTLE, m_Throttle );

    DDX_Check(pDX, IDC_VSS_ENABLE, m_VssEnable );
    DDX_Radio(pDX, IDC_VSS_STOP, m_VssMode );
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CDefragOptionsPage, CMFCPropertyPage)
    //{{AFX_MSG_MAP(CDefragOptionsPage)
    ON_WM_SIZE()
    //}}AFX_MSG_MAP
    ON_BN_CLICKED(IDC_ENABLE_THRESHOLD, &CDefragOptionsPage::OnBnClickedEnableThreshold)
    ON_BN_CLICKED(IDC_ADJUST_CPU_PRIORITY, &CDefragOptionsPage::OnBnClickedAdjustCpuPriority)
    ON_BN_CLICKED(IDC_VSS_ENABLE, &CDefragOptionsPage::OnBnClickedVssEnable)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDefragOptionsPage message handlers

void CDefragOptionsPage::OnOK()
{
    UpdateData();

    /*theApp.WriteInt( _T("Defrag.AllowDefragFlash"), m_AllowDefragFlash );
    theApp.WriteInt( _T("Defrag.AllowDefragRemovable"), m_AllowDefragRemovable );
    theApp.WriteInt( _T("Defrag.EnableThreshold"), m_EnableThreshold );
    theApp.WriteInt( _T("Defrag.Threshold"), m_Threshold );
    theApp.WriteInt( _T("Defrag.AggressivelyFreeSpace"), m_AggressivelyFreeSpace );
    theApp.WriteInt( _T("Defrag.AdjustCpuPriority"), m_AdjustCpuPriority );
    theApp.WriteInt( _T("Defrag.Throttle"), m_Throttle );
    theApp.WriteInt( _T("Defrag.VssEnable"), m_VssEnable );*/
    CADefragUIDoc* doc = GetMainDocument();
    if( doc )
    {
        DWORD isAllowDefragFlash = m_AllowDefragFlash;
        if( ERROR_SUCCESS != doc->SetSettingsValue( AllowDefragFlashId, &isAllowDefragFlash, sizeof(isAllowDefragFlash) ) )
        {
            ASSERT( FALSE );
        }
        DWORD isAllowDefragRemovable = m_AllowDefragRemovable;
        if( ERROR_SUCCESS != doc->SetSettingsValue( AllowDefragRemovableId, &isAllowDefragRemovable, sizeof(isAllowDefragRemovable) ) )
        {
            ASSERT( FALSE );
        }

        DWORD isThresholdEnabled = m_EnableThreshold;
        if( ERROR_SUCCESS != doc->SetSettingsValue( ThresholdEnableId, &isThresholdEnabled, sizeof(isThresholdEnabled) ) )
        {
            ASSERT( FALSE );
        }
        DWORD ThresholdSize = m_Threshold;
        if( ERROR_SUCCESS != doc->SetSettingsValue( ThresholdSizeId, &ThresholdSize, sizeof(ThresholdSize) ) )
        {
            ASSERT( FALSE );
        }

        DWORD isAggressivelyFreeSpaceEnabled = m_AggressivelyFreeSpace;
        if( ERROR_SUCCESS != doc->SetSettingsValue( AggressivelyFreeSpaceEnableId, &isAggressivelyFreeSpaceEnabled, sizeof(isAggressivelyFreeSpaceEnabled) ) )
        {
            ASSERT( FALSE );
        }

        DWORD AdjustCpuPriority = m_AdjustCpuPriority;
        if( ERROR_SUCCESS != doc->SetSettingsValue( AdjustCpuPriorityId, &AdjustCpuPriority, sizeof(AdjustCpuPriority) ) )
        {
            ASSERT( FALSE );
        }
        DWORD CpuPriority = m_CpuPriority;
        if( ERROR_SUCCESS != doc->SetSettingsValue( CpuPriorityId, &CpuPriority, sizeof(CpuPriority) ) )
        {
            ASSERT( FALSE );
        }

        DWORD isThrottleEnabled = m_Throttle;
        if( ERROR_SUCCESS != doc->SetSettingsValue( ThrottleEnableId, &isThrottleEnabled, sizeof(isThrottleEnabled) ) )
        {
            ASSERT( FALSE );
        }

        DWORD isVssEnabled = m_VssEnable;
        if( ERROR_SUCCESS != doc->SetSettingsValue( VssEnableId, &isVssEnabled, sizeof(isVssEnabled) ) )
        {
            ASSERT( FALSE );
        }
        DWORD VssMode = m_VssMode;
        if( ERROR_SUCCESS != doc->SetSettingsValue( VssModeId, &VssMode, sizeof(VssMode) ) )
        {
            ASSERT( FALSE );
        }
    }


    CMFCPropertyPage::OnOK();
}

BOOL CDefragOptionsPage::OnInitDialog()
{
    CMFCPropertyPage::OnInitDialog();
    m_thresholdSpin.SetRange32( 1, 99 );

    CADefragUIDoc* doc = GetMainDocument();
    if( doc )
    {
        DWORD isAllowDefragFlash = m_AllowDefragFlash;
        if( ERROR_SUCCESS == doc->GetSettingsValue( AllowDefragFlashId, &isAllowDefragFlash, sizeof(isAllowDefragFlash) ) )
        {
            m_AllowDefragFlash = isAllowDefragFlash ? TRUE : FALSE;
        }
        DWORD isAllowDefragRemovable = m_AllowDefragRemovable;
        if( ERROR_SUCCESS == doc->GetSettingsValue( AllowDefragRemovableId, &isAllowDefragRemovable, sizeof(isAllowDefragRemovable) ) )
        {
            m_AllowDefragRemovable = isAllowDefragRemovable ? TRUE : FALSE;
        }

        DWORD isThresholdEnabled = m_EnableThreshold;
        if( ERROR_SUCCESS == doc->GetSettingsValue( ThresholdEnableId, &isThresholdEnabled, sizeof(isThresholdEnabled) ) )
        {
            m_EnableThreshold = isThresholdEnabled ? TRUE : FALSE;
        }
        DWORD ThresholdSize = m_Threshold;
        if( ERROR_SUCCESS == doc->GetSettingsValue( ThresholdSizeId, &ThresholdSize, sizeof(ThresholdSize) ) )
        {
            m_Threshold = ThresholdSize;
        }

        DWORD isAggressivelyFreeSpaceEnabled = m_AggressivelyFreeSpace;
        if( ERROR_SUCCESS == doc->GetSettingsValue( AggressivelyFreeSpaceEnableId, &isAggressivelyFreeSpaceEnabled, sizeof(isAggressivelyFreeSpaceEnabled) ) )
        {
            m_AggressivelyFreeSpace = isAggressivelyFreeSpaceEnabled ? TRUE : FALSE;
        }

        DWORD AdjustCpuPriority = m_AdjustCpuPriority;
        if( ERROR_SUCCESS == doc->GetSettingsValue( AdjustCpuPriorityId, &AdjustCpuPriority, sizeof(AdjustCpuPriority) ) )
        {
            m_AdjustCpuPriority = AdjustCpuPriority ? TRUE : FALSE;
        }
        DWORD CpuPriority = m_CpuPriority;
        if( ERROR_SUCCESS == doc->GetSettingsValue( CpuPriorityId, &CpuPriority, sizeof(CpuPriority) ) )
        {
            m_CpuPriority = CpuPriority;
        }

        DWORD isThrottleEnabled = m_Throttle;
        if( ERROR_SUCCESS == doc->GetSettingsValue( ThrottleEnableId, &isThrottleEnabled, sizeof(isThrottleEnabled) ) )
        {
            m_Throttle = isThrottleEnabled ? TRUE : FALSE;
        }

        DWORD isVssEnabled = m_VssEnable;
        if( ERROR_SUCCESS == doc->GetSettingsValue( VssEnableId, &isVssEnabled, sizeof(isVssEnabled) ) )
        {
            m_VssEnable = isVssEnabled ? TRUE : FALSE;
        }
        DWORD VssMode = m_VssMode;
        if( ERROR_SUCCESS == doc->GetSettingsValue( VssModeId, &VssMode, sizeof(VssMode) ) )
        {
            m_VssMode = VssMode;
        }
    }

    UpdateData( FALSE );
    OnBnClickedEnableThreshold();
    OnBnClickedAdjustCpuPriority();
    OnBnClickedVssEnable();
    return TRUE;  // return TRUE unless you set the focus to a control
}

void CDefragOptionsPage::OnSize(UINT nType, int cx, int cy)
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


void CDefragOptionsPage::OnBnClickedEnableThreshold()
{
    UpdateData();
    GetDlgItem(IDC_THRESHOLD)->EnableWindow( m_EnableThreshold );
    GetDlgItem(IDC_THRESHOLD_SPIN)->EnableWindow( m_EnableThreshold );
}

void CDefragOptionsPage::OnBnClickedAdjustCpuPriority()
{
    UpdateData();
    GetDlgItem(IDC_PRIORITY_NORMAL)->EnableWindow( m_AdjustCpuPriority );
    GetDlgItem(IDC_PRIORITY_IDLE)->EnableWindow( m_AdjustCpuPriority );
    GetDlgItem(IDC_PRIORITY_TURBO)->EnableWindow( m_AdjustCpuPriority );
}

void CDefragOptionsPage::OnBnClickedVssEnable()
{
    UpdateData();
    GetDlgItem(IDC_VSS_STOP)->EnableWindow( m_VssEnable );
    GetDlgItem(IDC_VSS_DEFRAG)->EnableWindow( m_VssEnable );
    GetDlgItem(IDC_VSS_DEFRAG_NOTE)->EnableWindow( m_VssEnable );
    GetDlgItem(IDC_VSS_NORMAL)->EnableWindow( m_VssEnable );
}
