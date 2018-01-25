/*
    Interface of the CPopularOptionsPage class

    Module name:

    PopularOptionsPage.h

    Abstract:

    Interface of the CPopularOptionsPage class

    $Header: /home/CVS_DEFRAG/Defrag/Src/ADefragUI/Settings/PopularOptionsPage.h,v 1.2 2009/11/24 18:02:52 roman Exp $
    $Log: PopularOptionsPage.h,v $
    Revision 1.2  2009/11/24 18:02:52  roman
    Initial commit

    Revision 1.1  2009/11/24 14:56:45  dimas
    no message

*/

#ifndef __POPULAROPTIONSPAGE_H__
#define __POPULAROPTIONSPAGE_H__

#pragma once

#include "DlgBanner.h"
#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// CPopularOptionsPage dialog

class CPopularOptionsPage : public CMFCPropertyPage
{
	DECLARE_DYNCREATE(CPopularOptionsPage)

// Construction
public:
    CPopularOptionsPage();
    ~CPopularOptionsPage();

    // Dialog Data
    //{{AFX_DATA(CPopularOptionsPage)
    enum { IDD = IDD_POPULAR_PAGE };
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
    //{{AFX_MSG(CPopularOptionsPage)
    afx_msg void OnSize(UINT nType, int cx, int cy);
    //}}AFX_MSG

    DECLARE_MESSAGE_MAP()

    virtual BOOL OnInitDialog();

protected:
    CDlgBanner m_wndBanner;
    CDlgBanner m_StartupBanner;
    CDlgBanner m_UpdateBanner;
    CComboBox m_wndColorScheme;
    int m_nColorScheme;
    int m_nTooltipStyle;
    BOOL m_BootOptimization;
    BOOL m_AutoUpdate;
};

#endif // __POPULAROPTIONSPAGE_H__
