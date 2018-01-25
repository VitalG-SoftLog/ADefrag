/*
    Defrag CHelpView class header

    Module name:

        HelpView.h

    Abstract:

        Defines the CHelpView class.

    $Header: /home/CVS_DEFRAG/Defrag/Src/ADefragUI/HelpView.h,v 1.3 2009/12/14 10:30:11 dimas Exp $
    $Log: HelpView.h,v $
    Revision 1.3  2009/12/14 10:30:11  dimas
    CVS headers included


*/

#pragma once
#include "afxbutton.h"

// CHelpView form view

class CHelpView : public CFormView
{
    DECLARE_DYNCREATE(CHelpView)

public:
    CHelpView();           // protected constructor used by dynamic creation
    virtual ~CHelpView();

public:
    enum { IDD = IDD_HELPVIEW };
#ifdef _DEBUG
    virtual void AssertValid() const;
#ifndef _WIN32_WCE
    virtual void Dump(CDumpContext& dc) const;
#endif
#endif

    virtual void OnInitialUpdate();

protected:
    virtual void OnDraw(CDC* pDC);      // default does nothing
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    afx_msg void OnClose();
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
    DECLARE_MESSAGE_MAP()

protected:
    CMFCLinkCtrl m_productInfo;
    CMFCLinkCtrl m_support;
};


