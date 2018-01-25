#pragma once

#include "..\Ctrls\SSSPropSheetListBox.h"

class CSSSDHtmlView;
class CADefragUIDoc;

// CReportsView view

class CReportsView : public CView
{
	DECLARE_DYNCREATE(CReportsView)

protected:
    static const UINT   idList                  = 100;
    static const UINT   idSaveBtn               = 102;
    static const UINT   idPrintBtn              = 103;
    static const int    LeftMargin              = 5;
    static const int    TopMargin               = 5;
    static const int    ListWidth               = 200;
    static const int    BottomIndent            = 7;
    static const int    RightIndent             = 5;

    static const int    ReportsCount            = 6;
    static const int    DefaultReportSel        = 0;

    static const int    PosReportSystem             = 0;
    static const int    PosReportFragmentation      = 1;
    static const int    PosReportHeals              = 2;
    static const int    PosReportFilesOrganization  = 3;
    static const int    PosReportTop100             = 4;
    static const int    PosReportExcludedFiles      = 5;

	CReportsView();           // protected constructor used by dynamic creation
	virtual ~CReportsView();
    CSSSDHtmlView* CreateView( CRuntimeClass* pViewClass );
    
    CSSSPropSheetListBox    m_wndList;

    CString                 m_CompName;
    CString                 m_OSVer;
    CString                 m_Temp;
    CString                 m_DiskDiagram;
    CLSID                   m_PicEncoder;


    int                     m_nCurReportSel;
    CSSSDHtmlView           *ReportsWnd[ ReportsCount ];

public:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
    CADefragUIDoc* GetDocument(void);
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

protected:
    afx_msg void OnShowWindow( BOOL, UINT );
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnChangeReport();
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);    
    afx_msg void OnReportSave();
    afx_msg void OnReportPrint();
    afx_msg void OnReportPrintPreview();
    afx_msg void OnUpdateReportCommand(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

protected:
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    virtual void OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/);
    void SetReportsVisibility( int Index );
    void SetHTMLValue( CString ID, CString Value, BOOL IsUnknown, int Index );
    void SetHTMLValue( CString ID, DWORD Value, BOOL IsUnknown, int Index );

public:
    virtual void OnInitialUpdate();
    void UpdateAllReports();

protected:
    // Activation
    virtual void OnActivateView(BOOL bActivate, CView* pActivateView,
        CView* pDeactiveView);
};



