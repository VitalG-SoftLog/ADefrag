/*
    Interface of the CLogOptionsPage class

    Module name:

    LogOptionsPage.h

    Abstract:

    Interface of the CLogOptionsPage class

    $Header: /home/CVS_DEFRAG/Defrag/Src/ADefragUI/Settings/LogOptionsPage.h,v 1.2 2009/11/24 18:02:52 roman Exp $
    $Log: LogOptionsPage.h,v $
    Revision 1.2  2009/11/24 18:02:52  roman
    Initial commit

    Revision 1.1  2009/11/24 14:56:45  dimas
    no message

*/

#ifndef __LOGOPTIONSPAGE_H__
#define __LOGOPTIONSPAGE_H__

#pragma once

#include "DlgBanner.h"
#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// CLogOptionsPage dialog

class CLogOptionsPage : public CMFCPropertyPage
{
    DECLARE_DYNCREATE(CLogOptionsPage)

// Construction
public:
    CLogOptionsPage();
    ~CLogOptionsPage();

    // Dialog Data
    //{{AFX_DATA(CLogOptionsPage)
    enum { IDD = IDD_LOG_PAGE };
    //}}AFX_DATA

    // Overrides
public:
    virtual void OnOK();
protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    // Implementation
protected:
    //{{AFX_MSG(CLogOptionsPage)
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnBnClickedViewLog();
    afx_msg void OnBnClickedClearLog();
    //}}AFX_MSG

    DECLARE_MESSAGE_MAP()

    virtual BOOL OnInitDialog();

protected:
    CDlgBanner m_wndBanner;
    BOOL m_EnableApplicationLog;
    BOOL m_EnableEventLog;
    CString m_LogFilePath;
};

#endif // __LOGOPTIONSPAGE_H__
