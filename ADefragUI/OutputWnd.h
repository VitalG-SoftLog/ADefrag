/*
    Defrag OutputWnd header

    Module name:

        OutputWnd.h

    Abstract:

        Defines the OutputWnd classes.

    $Header: /home/CVS_DEFRAG/Defrag/Src/ADefragUI/OutputWnd.h,v 1.3 2009/12/03 15:36:55 dimas Exp $
    $Log: OutputWnd.h,v $
    Revision 1.3  2009/12/03 15:36:55  dimas
    CVS headers included

*/

#pragma once

/////////////////////////////////////////////////////////////////////////////
// COutputList window

class COutputList : public CListBox
{
// Construction
public:
    COutputList();

// Implementation
public:
    virtual ~COutputList();

protected:
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
    afx_msg void OnEditCopy();
    afx_msg void OnEditClear();
    afx_msg void OnViewOutput();

    DECLARE_MESSAGE_MAP()
};

class COutputWnd : public CDockablePane
{
// Construction
public:
    COutputWnd();

// Attributes
protected:
    CFont m_Font;

    CMFCTabCtrl m_wndTabs;

    COutputList m_wndOutputBuild;
    COutputList m_wndOutputDebug;
    COutputList m_wndOutputFind;

protected:
    void FillBuildWindow();
    void FillDebugWindow();
    void FillFindWindow();

    void AdjustHorzScroll(CListBox& wndListBox);

// Implementation
public:
    virtual ~COutputWnd();

protected:
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSize(UINT nType, int cx, int cy);

    DECLARE_MESSAGE_MAP()
};

