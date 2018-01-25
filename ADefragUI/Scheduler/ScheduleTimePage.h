/*
    Interface of the CScheduleTimePage class

    Module name:

    ScheduleTimePage.h

    Abstract:

    Interface of the CScheduleTimePage class

    $Header: /home/CVS_DEFRAG/Defrag/Src/ADefragUI/Scheduler/ScheduleTimePage.h,v 1.2 2009/11/24 18:02:52 roman Exp $
    $Log: ScheduleTimePage.h,v $
    Revision 1.2  2009/11/24 18:02:52  roman
    Initial commit

    Revision 1.1  2009/11/24 14:57:02  dimas
    no message

*/

#ifndef __SCHEDULETIMEPAGE_H__
#define __SCHEDULETIMEPAGE_H__

#pragma once

#include "SchedulePropSheet.h"
#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// CScheduleTimePage dialog

class CScheduleTimePage : public CScheduleBasePage
{
    DECLARE_DYNCREATE(CScheduleTimePage)

// Construction
public:
    CScheduleTimePage();
    ~CScheduleTimePage();

    // Dialog Data
    //{{AFX_DATA(CScheduleTimePage)
    enum { IDD = IDD_SCHEDULE_TIME };
    //}}AFX_DATA

    // Overrides
public:
    virtual void OnOK();
protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    // Implementation
protected:
    //{{AFX_MSG(CScheduleTimePage)
    afx_msg void OnSize(UINT nType, int cx, int cy);
    //}}AFX_MSG

    DECLARE_MESSAGE_MAP()

    virtual BOOL OnInitDialog();
    virtual BOOL OnSetActive();
    virtual BOOL OnKillActive();
    virtual BOOL OnWizardFinish() { return OnKillActive(); }

protected:
    CMonthCalCtrl m_startDate;
    CTime m_startTime;
    int m_dayPeriod;
    CSpinButtonCtrl m_dayPeriodSpin;
    int m_weekDay;
    BOOL m_EnableShutdown;
};

#endif // __SCHEDULETIMEPAGE_H__
