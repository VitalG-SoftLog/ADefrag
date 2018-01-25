/*
    Interface of the CScheduleDrivesPage class

    Module name:

    ScheduleDrivesPage.cpp

    Abstract:

    Interface of the CScheduleDrivesPage class

    $Header: /home/CVS_DEFRAG/Defrag/Src/ADefragUI/Scheduler/ScheduleDrivesPage.cpp,v 1.2 2009/11/24 18:02:52 roman Exp $
    $Log: ScheduleDrivesPage.cpp,v $
    Revision 1.2  2009/11/24 18:02:52  roman
    Initial commit

    Revision 1.1  2009/11/24 14:57:02  dimas
    no message

*/

#include "stdafx.h"
#include "ScheduleDrivesPage.h"
#include "../ADefragUI.h"
#include "../MainFrm.h"
#include "../ADefragUIDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CScheduleDrivesPage property page

IMPLEMENT_DYNCREATE(CScheduleDrivesPage, CMFCPropertyPage)

CScheduleDrivesPage::CScheduleDrivesPage() : CScheduleBasePage(CScheduleDrivesPage::IDD)
{
    //{{AFX_DATA_INIT(CScheduleDrivesPage)
    m_drivesType = Scheduler::e_DrivesAll;
    m_sequence = Scheduler::e_SequenceAuto;
    //}}AFX_DATA_INIT
}

CScheduleDrivesPage::~CScheduleDrivesPage()
{
}

void CScheduleDrivesPage::DoDataExchange(CDataExchange* pDX)
{
    __super::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CScheduleDrivesPage)
    //DDX_Control(pDX, IDC_BANER, m_wndBanner);
    //DDX_Check(pDX, IDC_AUTO_UPDATE, m_EnableAutoUpdate );
    DDX_Control(pDX, IDC_DRIVES_TREE, m_drivesTree);
    DDX_Radio(pDX, IDC_DRIVES_ALL, m_drivesType );
    DDX_Radio(pDX, IDC_SEQUENCE_ONEBYONE, m_sequence );
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CScheduleDrivesPage, CMFCPropertyPage)
    //{{AFX_MSG_MAP(CScheduleDrivesPage)
    ON_WM_SIZE()
    //}}AFX_MSG_MAP
    ON_BN_CLICKED(IDC_DRIVES_ALL, &CScheduleDrivesPage::OnBnClickedDrivesSelect)
    ON_BN_CLICKED(IDC_DRIVE_SELECTED, &CScheduleDrivesPage::OnBnClickedDrivesSelect)
    ON_BN_CLICKED(IDC_SEQUENCE_ONEBYONE, &CScheduleDrivesPage::OnBnClickedSequenceSelect)
    ON_BN_CLICKED(IDC_SEQUENCE_PARALLEL, &CScheduleDrivesPage::OnBnClickedSequenceSelect)
    ON_BN_CLICKED(IDC_SEQUENCE_AUTO, &CScheduleDrivesPage::OnBnClickedSequenceSelect)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CScheduleDrivesPage message handlers

void CScheduleDrivesPage::OnOK()
{
    UpdateData();

    __super::OnOK();
}

BOOL CScheduleDrivesPage::OnInitDialog()
{
    __super::OnInitDialog();

    CString drivesList;
    CScheduleTask* task = GetTask();
    if( task ) m_drivesType = task->GetDrivesType();
    if( task ) m_sequence = task->GetSequenceType();
    if( task ) drivesList = task->GetDrivesList();

    //
    // Drives Enumeration
    //
    CMainFrame* mainFrm = GetMainFrm();
    CADefragUIDoc* doc = mainFrm ? mainFrm->GetDocument() : NULL;
    m_drivesTree.EnableWindow( m_drivesType == Scheduler::e_DrivesSelected );
    m_drivesTree.ModifyStyle( TVS_CHECKBOXES, 0 );
    m_drivesTree.ModifyStyle( 0, TVS_CHECKBOXES );

    if( doc )
    {
        DriveInfoMap& drvInfo = doc->GetDisksInfo();
        for ( int i = 0; i < (int)drvInfo.size(); i++ )
        {
            BOOL isReadOnlyDrive = (FILE_READ_ONLY_VOLUME & drvInfo[i].diskStatus.FileSystemFlags) != 0;
            BOOL isSupportedSystem = drvInfo[i].diskStatus.FileSystem.Find( _T("FAT") ) >= 0
                || drvInfo[i].diskStatus.FileSystem.Find( _T("NTFS") ) >= 0;
            if( (isReadOnlyDrive || !isSupportedSystem) && !drvInfo[i].diskStatus.IsInvalid ) continue;
            CString drive;
            drive.Format( _T("%s\\"), drvInfo[i].diskStatus.Name );
            HTREEITEM item = m_drivesTree.InsertItem( drive );
            if( drivesList.Find( drive + TASK_DRIVES_DELIMETER ) >= 0 )
            {
                m_drivesTree.SetCheck( item );
            }
        }
    }
    m_drivesTree.UpdateData();
    UpdateData( FALSE );

    return TRUE;  // return TRUE unless you set the focus to a control
}

void CScheduleDrivesPage::OnSize(UINT nType, int cx, int cy)
{
    __super::OnSize(nType, cx, cy);
}

//////////////////////////////////////////////////////////////////////////
BOOL CScheduleDrivesPage::OnSetActive()
{
    CSchedulePropSheet* psheet = GetSheet();
    if( psheet ) psheet->SetWizardButtons( PSWIZB_NEXT );
    return __super::OnSetActive();
}

//////////////////////////////////////////////////////////////////////////
BOOL CScheduleDrivesPage::OnKillActive()
{
    UpdateData();
    CScheduleTask* task = GetTask();
    if( task && m_drivesType == Scheduler::e_DrivesSelected )
    {
        CString strDrives;
        for( HTREEITEM item = m_drivesTree.GetRootItem(); item; item = m_drivesTree.GetNextItem( item, TVGN_NEXT ) )
        {
            if( !m_drivesTree.GetCheck( item ) ) continue;
            strDrives += m_drivesTree.GetItemText( item ) + TASK_DRIVES_DELIMETER;
        }
        task->SetDrivesList( strDrives );
    }
    return __super::OnKillActive();
}

//////////////////////////////////////////////////////////////////////////
void CScheduleDrivesPage::OnBnClickedDrivesSelect()
{
    UpdateData();
    CScheduleTask* task = GetTask();
    if( task ) task->SetDrivesType( Scheduler::EDrivesType( m_drivesType ) );
    m_drivesTree.EnableWindow( m_drivesType == Scheduler::e_DrivesSelected );
}

//////////////////////////////////////////////////////////////////////////
void CScheduleDrivesPage::OnBnClickedSequenceSelect()
{
    UpdateData();
    CScheduleTask* task = GetTask();
    if( task ) task->SetSequenceType( Scheduler::ESequenceType( m_sequence ) );
}
