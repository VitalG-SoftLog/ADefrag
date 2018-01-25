/*
    Interface of the CDefragOptionsPage class

    Module name:

    DefragOptionsPage.h

    Abstract:

    Interface of the CDefragOptionsPage class

    $Header: /home/CVS_DEFRAG/Defrag/Src/ADefragUI/Settings/DefragOptionsPage.h,v 1.2 2009/11/24 18:02:52 roman Exp $
    $Log: DefragOptionsPage.h,v $
    Revision 1.2  2009/11/24 18:02:52  roman
    Initial commit

    Revision 1.1  2009/11/24 14:56:44  dimas
    no message

*/

#ifndef __DEFRAGOPTIONSPAGE_H__
#define __DEFRAGOPTIONSPAGE_H__

#pragma once

#include "DlgBanner.h"
#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// CDefragOptionsPage dialog

class CDefragOptionsPage : public CMFCPropertyPage
{
    DECLARE_DYNCREATE(CDefragOptionsPage)

// Construction
public:
    CDefragOptionsPage();
    ~CDefragOptionsPage();

    // Dialog Data
    //{{AFX_DATA(CDefragOptionsPage)
    enum { IDD = IDD_DEFRAG_PAGE };
    //}}AFX_DATA

    // Overrides
public:
    virtual void OnOK();
protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    // Implementation
protected:
    //{{AFX_MSG(CDefragOptionsPage)
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnBnClickedEnableThreshold();
    afx_msg void OnBnClickedAdjustCpuPriority();
    afx_msg void OnBnClickedVssEnable();
    //}}AFX_MSG

    DECLARE_MESSAGE_MAP()

    virtual BOOL OnInitDialog();

protected:
    CDlgBanner m_wndBanner;
    BOOL m_AllowDefragFlash;
    BOOL m_AllowDefragRemovable;

    BOOL m_EnableThreshold;
    int m_Threshold;
    CSpinButtonCtrl m_thresholdSpin;

    BOOL m_AggressivelyFreeSpace;
    BOOL m_AdjustCpuPriority;
    int m_CpuPriority;
    BOOL m_Throttle;
    BOOL m_VssEnable;
    int m_VssMode;
};

#endif // __DEFRAGOPTIONSPAGE_H__
