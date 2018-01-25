/*
    Interface of the CScheduleMethodPage class

    Module name:

    ScheduleMethodPage.cpp

    Abstract:

    Interface of the CScheduleMethodPage class

    $Header: /home/CVS_DEFRAG/Defrag/Src/ADefragUI/Scheduler/ScheduleMethodPage.cpp,v 1.2 2009/11/24 18:02:52 roman Exp $
    $Log: ScheduleMethodPage.cpp,v $
    Revision 1.2  2009/11/24 18:02:52  roman
    Initial commit

    Revision 1.1  2009/11/24 14:57:02  dimas
    no message

*/

#include "stdafx.h"
#include "ScheduleMethodPage.h"
#include "../ADefragUI.h"
#include "../MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CScheduleMethodPage property page

IMPLEMENT_DYNCREATE(CScheduleMethodPage, CMFCPropertyPage)

CScheduleMethodPage::CScheduleMethodPage() : CScheduleBasePage(CScheduleMethodPage::IDD)
{
    //{{AFX_DATA_INIT(CScheduleMethodPage)
    m_defragMethod = Scheduler::e_DefragFreeSpace;
    m_defragMode = Scheduler::e_DefragModeNormal;
    //}}AFX_DATA_INIT
}

CScheduleMethodPage::~CScheduleMethodPage()
{
}

void CScheduleMethodPage::DoDataExchange(CDataExchange* pDX)
{
    __super::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CScheduleMethodPage)
    DDX_Radio(pDX, IDC_METHOD_SIMPLE, m_defragMethod );
    DDX_Radio(pDX, IDC_DEFRAG_MODE_NORMAL, m_defragMode );
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CScheduleMethodPage, CMFCPropertyPage)
    //{{AFX_MSG_MAP(CScheduleMethodPage)
    ON_WM_SIZE()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CScheduleMethodPage message handlers

void CScheduleMethodPage::OnOK()
{
    UpdateData();
    __super::OnOK();
}

BOOL CScheduleMethodPage::OnInitDialog()
{
    __super::OnInitDialog();

    CScheduleTask* task = GetTask();
    if( task ) m_defragMethod = task->GetDefragMethod();
    if( task ) m_defragMode = task->GetDefragMode();

    UpdateData( FALSE );
    return TRUE;  // return TRUE unless you set the focus to a control
}

void CScheduleMethodPage::OnSize(UINT nType, int cx, int cy)
{
    CMFCPropertyPage::OnSize(nType, cx, cy);
}

//////////////////////////////////////////////////////////////////////////
BOOL CScheduleMethodPage::OnSetActive()
{
    CSchedulePropSheet* psheet = GetSheet();
    if( psheet ) psheet->SetWizardButtons( PSWIZB_BACK | PSWIZB_NEXT );
    return __super::OnSetActive();
}

//////////////////////////////////////////////////////////////////////////
BOOL CScheduleMethodPage::OnKillActive()
{
    UpdateData();
    CScheduleTask* task = GetTask();
    if( task )
    {
        task->SetDefragMethod( Scheduler::EScheduleDefragMethod( m_defragMethod ) );
        task->SetDefragMode( Scheduler::EScheduleDefragMode( m_defragMode ) );
    }
    return __super::OnKillActive();
}
