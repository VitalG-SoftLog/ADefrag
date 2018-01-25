/*
    Interface of the CBaseListView class

    Module name:

    BaseListView.h

    Abstract:

    Interface of the CBaseListView class

    $Header: /home/CVS_DEFRAG/Defrag/Src/ADefragUI/BaseListView.h,v 1.3 2009/11/24 18:02:52 roman Exp $
    $Log: BaseListView.h,v $
    Revision 1.3  2009/11/24 18:02:52  roman
    Initial commit

    Revision 1.2  2009/11/24 17:43:24  roman
    Initial commit

*/

#ifndef __BASELISTVIEW_H__
#define __BASELISTVIEW_H__

#pragma once

//////////////////////////////////////////////////////////////////////////
// Forward declaration and includes

//////////////////////////////////////////////////////////////////////////
// Schedule List control
class CBaseListCtrl : public CMFCListCtrl
{
    virtual int OnCompareItems(LPARAM lParam1, LPARAM lParam2, int iColumn);
};

/////////////////////////////////////////////////////////////////////////////
// CBaseListView view

class CBaseListView : public CView
{
public:
    CBaseListView();           // protected constructor used by dynamic creation
    DECLARE_DYNCREATE(CBaseListView)

    // Overrides
public:
    virtual void OnInitialUpdate();
    CBaseListCtrl& GetListCtrl() { return m_listCtrl; }

protected:
    virtual void OnDraw(CDC* pDC);      // overridden to draw this view
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    virtual BOOL PreTranslateMessage(MSG* pMsg);

    // Implementation
protected:
    virtual ~CBaseListView();
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

    virtual void InitialUpdateList() {}
    virtual UINT GetContextMenuID() { return 0; }
    virtual UINT GetAcceleratorID() { return 0; }

    CADefragUIDoc* GetDocument( void );
    int GetCurSel( void );
    void SetCurSel( int Index );

protected:
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnWindowPosChanging(WINDOWPOS FAR* lpwndpos);
    DECLARE_MESSAGE_MAP()

protected:
    CBaseListCtrl m_listCtrl;
    HACCEL m_hAccel;
};

#endif // __BaseListVIEW_H__
