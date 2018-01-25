/*
    Interface of the CSchedulerOptionsPage class

    Module name:

    SchedulerOptionsPage.h

    Abstract:

    Interface of the CSchedulerOptionsPage class

    $Header: /home/CVS_DEFRAG/Defrag/Src/ADefragUI/Settings/SchedulerOptionsPage.h,v 1.2 2009/11/24 18:02:52 roman Exp $
    $Log: SchedulerOptionsPage.h,v $
    Revision 1.2  2009/11/24 18:02:52  roman
    Initial commit

    Revision 1.1  2009/11/24 14:56:45  dimas
    no message

*/

#ifndef __SCHEDULEROPTIONSPAGE_H__
#define __SCHEDULEROPTIONSPAGE_H__

#pragma once

#include "DlgBanner.h"
#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// CSchedulerOptionsPage dialog

class CSchedulerOptionsPage : public CMFCPropertyPage
{
    DECLARE_DYNCREATE(CSchedulerOptionsPage)

// Construction
public:
    CSchedulerOptionsPage();
    ~CSchedulerOptionsPage();

    // Dialog Data
    //{{AFX_DATA(CSchedulerOptionsPage)
    enum { IDD = IDD_SCHEDULER_PAGE };
    //}}AFX_DATA

    // Overrides
public:
    virtual void OnOK();
protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    // Implementation
protected:
    //{{AFX_MSG(CSchedulerOptionsPage)
    afx_msg void OnSize(UINT nType, int cx, int cy);
    //}}AFX_MSG

    DECLARE_MESSAGE_MAP()

    virtual BOOL OnInitDialog();

protected:
    CDlgBanner m_wndBanner;
    BOOL m_NotStartBattery;
    BOOL m_StopBattery;
    BOOL m_WakeRun;
    BOOL m_RunMissed;
};

#endif // __SCHEDULEROPTIONSPAGE_H__
