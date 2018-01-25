/*
    Interface of the CScheduleMethodPage class

    Module name:

    ScheduleMethodPage.h

    Abstract:

    Interface of the CScheduleMethodPage class

    $Header: /home/CVS_DEFRAG/Defrag/Src/ADefragUI/Scheduler/ScheduleMethodPage.h,v 1.2 2009/11/24 18:02:52 roman Exp $
    $Log: ScheduleMethodPage.h,v $
    Revision 1.2  2009/11/24 18:02:52  roman
    Initial commit

    Revision 1.1  2009/11/24 14:57:02  dimas
    no message

*/

#ifndef __SCHEDULEMETHODPAGE_H__
#define __SCHEDULEMETHODPAGE_H__

#pragma once

#include "SchedulePropSheet.h"
#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// CScheduleMethodPage dialog

class CScheduleMethodPage : public CScheduleBasePage
{
    DECLARE_DYNCREATE(CScheduleMethodPage)

// Construction
public:
    CScheduleMethodPage();
    ~CScheduleMethodPage();

    // Dialog Data
    //{{AFX_DATA(CScheduleMethodPage)
    enum { IDD = IDD_SCHEDULE_METHOD };
    //}}AFX_DATA

    // Overrides
public:
    virtual void OnOK();
protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    // Implementation
protected:
    //{{AFX_MSG(CScheduleMethodPage)
    afx_msg void OnSize(UINT nType, int cx, int cy);
    //}}AFX_MSG

    DECLARE_MESSAGE_MAP()

    virtual BOOL OnInitDialog();
    virtual BOOL OnSetActive();
    virtual BOOL OnKillActive();

protected:
    int m_defragMethod;
    int m_defragMode;
};

#endif // __SCHEDULEMETHODPAGE_H__
