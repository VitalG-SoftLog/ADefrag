/*
    Interface of the CScheduleDrivesPage class

    Module name:

    ScheduleDrivesPage.h

    Abstract:

    Interface of the CScheduleDrivesPage class

    $Header: /home/CVS_DEFRAG/Defrag/Src/ADefragUI/Scheduler/ScheduleDrivesPage.h,v 1.2 2009/11/24 18:02:52 roman Exp $
    $Log: ScheduleDrivesPage.h,v $
    Revision 1.2  2009/11/24 18:02:52  roman
    Initial commit

    Revision 1.1  2009/11/24 14:57:02  dimas
    no message

*/

#ifndef __SCHEDULEDRIVESPAGE_H__
#define __SCHEDULEDRIVESPAGE_H__

#pragma once

#include "SchedulePropSheet.h"
#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// CScheduleDrivesPage dialog

class CScheduleDrivesPage : public CScheduleBasePage
{
    DECLARE_DYNCREATE(CScheduleDrivesPage)

// Construction
public:
    CScheduleDrivesPage();
    ~CScheduleDrivesPage();

    // Dialog Data
    //{{AFX_DATA(CScheduleDrivesPage)
    enum { IDD = IDD_SCHEDULE_DRIVES };
    //}}AFX_DATA

    // Overrides
public:
    virtual void OnOK();
protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    // Implementation
protected:
    //{{AFX_MSG(CScheduleDrivesPage)
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnBnClickedDrivesSelect();
    afx_msg void OnBnClickedSequenceSelect();
    //}}AFX_MSG

    DECLARE_MESSAGE_MAP()

    virtual BOOL OnInitDialog();
    virtual BOOL OnSetActive();
    virtual BOOL OnKillActive();

protected:
    //CDlgBanner m_wndBanner;
    CTreeCtrl m_drivesTree;
    int m_drivesType;
    int m_sequence;
};

#endif // __SCHEDULEDRIVESPAGE_H__
