/*
    Interface of the CUpdateOptionsPage class

    Module name:

    UpdateOptionsPage.h

    Abstract:

    Interface of the CUpdateOptionsPage class

    $Header: /home/CVS_DEFRAG/Defrag/Src/ADefragUI/Settings/UpdateOptionsPage.h,v 1.2 2009/11/24 18:02:52 roman Exp $
    $Log: UpdateOptionsPage.h,v $
    Revision 1.2  2009/11/24 18:02:52  roman
    Initial commit

    Revision 1.1  2009/11/24 14:56:45  dimas
    no message

*/

#ifndef __UPDATEOPTIONSPAGE_H__
#define __UPDATEOPTIONSPAGE_H__

#pragma once

#include "DlgBanner.h"
#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// CUpdateOptionsPage dialog

class CUpdateOptionsPage : public CMFCPropertyPage
{
    DECLARE_DYNCREATE(CUpdateOptionsPage)

// Construction
public:
    CUpdateOptionsPage();
    ~CUpdateOptionsPage();

    // Dialog Data
    //{{AFX_DATA(CUpdateOptionsPage)
    enum { IDD = IDD_UPDATE_PAGE };
    //}}AFX_DATA

    // Overrides
public:
    virtual void OnOK();
    virtual BOOL OnSetActive();
    virtual BOOL OnKillActive();

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    // Implementation
protected:
    //{{AFX_MSG(CUpdateOptionsPage)
    afx_msg void OnSize(UINT nType, int cx, int cy);
    //}}AFX_MSG

    DECLARE_MESSAGE_MAP()

    virtual BOOL OnInitDialog();

protected:
    CDlgBanner m_wndBanner;
    BOOL m_EnableAutoUpdate;
};

#endif // __UPDATEOPTIONSPAGE_H__
