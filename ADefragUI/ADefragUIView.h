/*
    Defrag ADefragUIView class header

    Module name:

        ADefragUIView.h

    Abstract:

        Defines the ADefragUIView class.

    $Header: /home/CVS_DEFRAG/Defrag/Src/ADefragUI/ADefragUIView.h,v 1.3 2009/12/03 15:36:55 dimas Exp $
    $Log: ADefragUIView.h,v $
    Revision 1.3  2009/12/03 15:36:55  dimas
    CVS headers included

*/


#pragma once

class CADefragUICntrItem;

class CADefragUIView : public CTabView // CView
{
protected: // create from serialization only
    CADefragUIView();
    DECLARE_DYNCREATE(CADefragUIView)

// Attributes
public:
    CADefragUIDoc* GetDocument() const;
    // m_pSelection holds the selection to the current CADefragUICntrItem.
    // For many applications, such a member variable isn't adequate to
    //  represent a selection, such as a multiple selection or a selection
    //  of objects that are not CADefragUICntrItem objects.  This selection
    //  mechanism is provided just to help you get started

    // TODO: replace this selection mechanism with one appropriate to your app
    CADefragUICntrItem* m_pSelection;

// Operations
public:
    BOOL IsScrollBar () const
    {
        return FALSE; //TRUE
    }

// Overrides
public:
    virtual void OnDraw(CDC* pDC);  // overridden to draw this view
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
    virtual void OnInitialUpdate(); // called first time after construct
    virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
    virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
    virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
    virtual BOOL IsSelected(const CObject* pDocItem) const;// Container support

// Implementation
public:
    virtual ~CADefragUIView();
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
    afx_msg void OnDestroy();
    afx_msg void OnSetFocus(CWnd* pOldWnd);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnInsertObject();
    afx_msg void OnCancelEditCntr();
    afx_msg void OnFilePrint();
    afx_msg void OnFilePrintPreview();
    afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in ADefragUIView.cpp
inline CADefragUIDoc* CADefragUIView::GetDocument() const
   { return reinterpret_cast<CADefragUIDoc*>(m_pDocument); }
#endif

