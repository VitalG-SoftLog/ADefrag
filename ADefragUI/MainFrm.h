/*
    Interface of the CMainFrame class

    Module name:

    MainFrm.h

    Abstract:

    Interface of the CMainFrame class

    $Header: /home/CVS_DEFRAG/Defrag/Src/ADefragUI/MainFrm.h,v 1.6 2010/01/12 22:53:18 roman Exp $
    $Log: MainFrm.h,v $
    Revision 1.6  2010/01/12 22:53:18  roman
    Add "Recommended Defrag Method..." button

    Revision 1.5  2009/12/24 01:39:11  roman
    fix errors

    Revision 1.4  2009/12/17 07:48:55  roman
    Win7 and Vista support.

    Revision 1.3  2009/11/24 18:02:52  roman
    Initial commit

*/

#ifndef __MAINFRM_H__
#define __MAINFRM_H__

#pragma once

#include "OutputWnd.h"
#include "TaskPane.h"
class CADefragUIDoc;
class CDriveMapView;
class CDefragView;

#define RIBBON_DEFRAG_PROGRESS_WIDTH        100
#define ID_REPORT_CONTEXT                   199

class CMainFrame : public CFrameWndEx
{
protected: // create from serialization only
    CMainFrame();
    DECLARE_DYNCREATE(CMainFrame)

    // Overrides
public:
    virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    virtual BOOL LoadFrame(UINT nIDResource, DWORD dwDefaultStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, CWnd* pParentWnd = NULL, CCreateContext* pContext = NULL);
    void InitViews();

    static const int    ReportRibbonIndex      = 1;

    CADefragUIDoc* GetDocument(void);
    CDriveMapView* GetDriveMapView(void);
    CDefragView*   GetDriveListView(void);

    void SwitchReportRibbon( BOOL IsVis );
    void UpdateStatusAndProgress( LPCTSTR status, int progress );
    void UpdateTaskpaneResultAndReliability( DriveHealth* health, BOOL activate, BOOL recreate = TRUE );

    // Implementation
public:
    virtual ~CMainFrame();
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
    CMFCRibbonBar                   m_wndRibbonBar;
    CMFCRibbonApplicationButton     m_MainButton;
    CMFCToolBarImages               m_PanelImages;
    CMFCRibbonStatusBar             m_wndStatusBar;
    COutputWnd                      m_wndOutput;
    CMFCRibbonComboBox              *pBlockSizeCB;
    CMFCRibbonProgressBar           *pProgress;
    int                             m_nProgressValue;
    CTaskPane                       m_wndTaskPane;

    // Generated message map functions
protected:
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnDestroy(void);
    afx_msg void OnClose(void);
    afx_msg void OnTimer(UINT_PTR NIDEvent);
    afx_msg void OnFileClose();
    afx_msg void OnApplicationLook(UINT id);
    afx_msg void OnUpdateApplicationLook(CCmdUI* pCmdUI);
    afx_msg void OnFilePrint();
    afx_msg void OnFilePrintPreview();
    afx_msg void OnUpdateFilePrintPreview(CCmdUI* pCmdUI);
    afx_msg void OnUpdateOutputWnd(CCmdUI* pCmdUI);
    afx_msg void OnViewOutputWnd();
    // Registered messages
    afx_msg LRESULT OnChangeRibbonCategory(WPARAM,LPARAM);
    // Defragmentations massages
    afx_msg void OnUpdateDefragAnalyze(CCmdUI* pCmdUI);
    afx_msg void OnUpdateDefragFreeSpace(CCmdUI* pCmdUI);
    afx_msg void OnUpdateDefragSimple(CCmdUI* pCmdUI);
    afx_msg void OnUpdateDefragOrder(CCmdUI* pCmdUI);
    afx_msg void OnUpdateDefragStop(CCmdUI* pCmdUI);
    afx_msg void OnUpdateDefragPause(CCmdUI* pCmdUI);
    afx_msg void OnUpdateComboBox(CCmdUI* pCmdUI);
    afx_msg void OnUpdateViewTaskpane(CCmdUI* pCmdUI);
    afx_msg void OnUpdateShowLegend(CCmdUI* pCmdUI);
    afx_msg void OnUpdateSetDefault(CCmdUI* pCmdUI);
	afx_msg void OnUpdateDefragMostrecommended(CCmdUI *pCmdUI);
    afx_msg void OnDefragAnalyze();
    afx_msg void OnDefragFreeSpace();
    afx_msg void OnDefragSimple();
    afx_msg void OnDefragOrder(UINT id);
    afx_msg void OnDefragPause();
    afx_msg void OnDefragStop();
    afx_msg void OnComboBox();
    afx_msg void OnViewTaskpane();
    afx_msg void OnShowLegend();
    afx_msg void OnSetDefault();
    afx_msg void OnSettings();
	afx_msg void OnDefragMostrecommended();
    afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
    afx_msg LRESULT OnTrayNotify(WPARAM wp, LPARAM lp);
    afx_msg LRESULT OnTaskbarCreated(WPARAM,LPARAM);
    afx_msg void OnAppOpen();
    afx_msg LRESULT OnExternalCall( WPARAM, LPARAM lParam );
    afx_msg LRESULT OnDevicesListChanged( WPARAM, LPARAM lParam );
    afx_msg void OnHelp();
    afx_msg void OnLink();
    DECLARE_MESSAGE_MAP()

    enum ViewType { e_firstView, e_DefragView = e_firstView, e_DefragReports, e_SchedulerView, e_HelpView, e_countOfView };
//    enum ViewType { e_firstView, e_DefragView, e_DefragReports, e_SchedulerView, e_HelpView, e_countOfView };
    BOOL ActivateView( ViewType view );
    void InitializeRibbon();
    BOOL CreateTaskPane();
    BOOL CreateDockingWindows();
    void SetDockingWindowIcons(BOOL bHiColorIcons);
    void ShowOptions(int nPage);
    void OnTrayContextMenu();

    BOOL AddTrayIcon();
    void AllowReceiveWMCopyData();
    void ShowProgress(BOOL bShow);

	BOOL m_bVistaOrLater;
	static BOOL Is_Vista_or_Later();

// Attributes
protected:
    typedef std::pair<CWnd*,CView*> TViewPair;
    TViewPair m_allViews[e_countOfView];
    ViewType m_activeView;
    CSplitterWndEx m_splitterDefrag;
    HICON m_hIcon;
    bool m_showMaximized;
    NOTIFYICONDATA  m_nid; // struct for Shell_NotifyIcon args
    static UINT m_TaskbarCreated;
public:
	afx_msg void OnDefragRecommend();
	afx_msg void OnUpdateDefragRecommend(CCmdUI *pCmdUI);
};

inline CMainFrame* GetMainFrm()
{
    CMainFrame* frm = DYNAMIC_DOWNCAST( CMainFrame, AfxGetMainWnd() );
    if( !frm && AfxGetApp() ) frm = DYNAMIC_DOWNCAST( CMainFrame, AfxGetApp()->m_pMainWnd );
    return frm;
}

inline CADefragUIDoc* GetMainDocument()
{
    CMainFrame* frm = GetMainFrm();
    return frm ? frm->GetDocument() : NULL;
}

#endif // __MAINFRM_H__
