/*
    Interface of the CScheduleTimePage class

    Module name:

    ScheduleTimePage.cpp

    Abstract:

    Interface of the CScheduleTimePage class

    $Header: /home/CVS_DEFRAG/Defrag/Src/ADefragUI/Scheduler/ScheduleTimePage.cpp,v 1.2 2009/11/24 18:02:52 roman Exp $
    $Log: ScheduleTimePage.cpp,v $
    Revision 1.2  2009/11/24 18:02:52  roman
    Initial commit

    Revision 1.1  2009/11/24 14:57:02  dimas
    no message

*/

#include "stdafx.h"
#include "ScheduleTimePage.h"
#include "../ADefragUI.h"
#include "../MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CScheduleTimePage property page

IMPLEMENT_DYNCREATE(CScheduleTimePage, CMFCPropertyPage)

CScheduleTimePage::CScheduleTimePage() : CScheduleBasePage(CScheduleTimePage::IDD)
{
    //{{AFX_DATA_INIT(CScheduleTimePage)
    m_startTime = CTime::GetCurrentTime();
    m_dayPeriod = 1;
    m_weekDay = 0;
    m_EnableShutdown = FALSE;
    //}}AFX_DATA_INIT
}

CScheduleTimePage::~CScheduleTimePage()
{
}

void CScheduleTimePage::DoDataExchange(CDataExchange* pDX)
{
    __super::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CScheduleTimePage)
    DDX_Check(pDX, IDC_CHECK_SHUTDOWN, m_EnableShutdown );
    DDX_Control(pDX, IDC_MONTHCALENDAR, m_startDate );
    DDX_DateTimeCtrl(pDX, IDC_DATETIMEPICKER, m_startTime );
    DDX_Text(pDX, IDC_EDIT_DAYPERIOD, m_dayPeriod );
    DDV_MinMaxInt(pDX, m_dayPeriod, 1, 99 );
    DDX_Control(pDX, IDC_SPIN_DAYPERIOD, m_dayPeriodSpin);
    DDX_CBIndex(pDX, IDC_COMBO_WEEKDAY, m_weekDay );
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CScheduleTimePage, CMFCPropertyPage)
    //{{AFX_MSG_MAP(CScheduleTimePage)
    ON_WM_SIZE()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CScheduleTimePage message handlers

void CScheduleTimePage::OnOK()
{
    UpdateData();
    CMFCPropertyPage::OnOK();
}

BOOL CScheduleTimePage::OnInitDialog()
{
    __super::OnInitDialog();
    m_dayPeriodSpin.SetRange32( 1, 99 );

    CScheduleTask* task = GetTask();
    if( task )
    {
        m_startDate.SetCurSel( task->GetStartDateTime() );
        m_startTime = task->GetStartDateTime();
        m_dayPeriod = task->GetDayPeriod();
        m_weekDay = task->GetWeekDay();
        m_EnableShutdown = task->GetEnableShutdown();
        switch( task->GetPeriod() )
        {
        case Scheduler::e_PeriodOneTime:
        case Scheduler::e_PeriodSceenSaver:
            GetDlgItem(IDC_STATIC_DAYPERIOD)->ShowWindow( SW_HIDE );
            GetDlgItem(IDC_EDIT_DAYPERIOD)->ShowWindow( SW_HIDE );
            GetDlgItem(IDC_SPIN_DAYPERIOD)->ShowWindow( SW_HIDE );
            GetDlgItem(IDC_STATIC_WEEKDAY)->ShowWindow( SW_HIDE );
            GetDlgItem(IDC_COMBO_WEEKDAY)->ShowWindow( SW_HIDE );
            break;
        case Scheduler::e_PeriodDay:
            GetDlgItem(IDC_STATIC_WEEKDAY)->ShowWindow( SW_HIDE );
            GetDlgItem(IDC_COMBO_WEEKDAY)->ShowWindow( SW_HIDE );
            break;
        case Scheduler::e_PeriodWeek:
            GetDlgItem(IDC_STATIC_DAYPERIOD)->ShowWindow( SW_HIDE );
            GetDlgItem(IDC_EDIT_DAYPERIOD)->ShowWindow( SW_HIDE );
            GetDlgItem(IDC_SPIN_DAYPERIOD)->ShowWindow( SW_HIDE );
            break;
        }
    }

    UpdateData( FALSE );
    return TRUE;  // return TRUE unless you set the focus to a control
}

void CScheduleTimePage::OnSize(UINT nType, int cx, int cy)
{
    __super::OnSize(nType, cx, cy);
}

//////////////////////////////////////////////////////////////////////////
BOOL CScheduleTimePage::OnSetActive()
{
    CSchedulePropSheet* psheet = GetSheet();
    if( psheet ) psheet->SetWizardButtons( PSWIZB_FINISH );
    return __super::OnSetActive();
}

//////////////////////////////////////////////////////////////////////////
BOOL CScheduleTimePage::OnKillActive()
{
    UpdateData();
    CScheduleTask* task = GetTask();
    if( task )
    {
        CTime selDate;
        m_startDate.GetCurSel( selDate );
        CTime startTime( selDate.GetYear(), selDate.GetMonth(), selDate.GetDay(),
            m_startTime.GetHour(), m_startTime.GetMinute(), m_startTime.GetSecond() );
        task->SetStartDateTime( startTime );
        task->SetDayPeriod( m_dayPeriod );
        task->SetWeekDay( Scheduler::EScheduleWeekDay( m_weekDay ) );
        task->SetEnableShutdown( m_EnableShutdown );
    }
    return __super::OnKillActive();
}
