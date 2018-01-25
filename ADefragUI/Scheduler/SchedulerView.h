/*
    Interface of the CSchedulerView class

    Module name:

    SchedulerView.h

    Abstract:

    Interface of the CSchedulerView class

    $Header: /home/CVS_DEFRAG/Defrag/Src/ADefragUI/Scheduler/SchedulerView.h,v 1.2 2009/11/24 18:02:52 roman Exp $
    $Log: SchedulerView.h,v $
    Revision 1.2  2009/11/24 18:02:52  roman
    Initial commit

    Revision 1.1  2009/11/24 14:57:02  dimas
    no message

*/

#ifndef __SCHEDULERVIEW_H__
#define __SCHEDULERVIEW_H__

#pragma once

//////////////////////////////////////////////////////////////////////////
// Forward declaration and includes
#include "ScheduleTask.h"

//////////////////////////////////////////////////////////////////////////
// Schedule List control
class CScheduleListCtrl : public CMFCListCtrl
{
    virtual int OnCompareItems(LPARAM lParam1, LPARAM lParam2, int iColumn);
};

/////////////////////////////////////////////////////////////////////////////
// CSchedulerView view

class CSchedulerView : public CView
{
public:
    CSchedulerView();           // protected constructor used by dynamic creation
    DECLARE_DYNCREATE(CSchedulerView)

    // Overrides
public:
    virtual void OnInitialUpdate();
    CMFCListCtrl& GetListCtrl() { return m_listCtrl; }

protected:
    virtual void OnDraw(CDC* pDC);      // overridden to draw this view
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    virtual BOOL PreTranslateMessage(MSG* pMsg);

    static const UINT UpdateTimerPeriod = 1000;
    enum ESchedulerColumns { e_firstCol, e_statusCol = e_firstCol, e_actionCol, e_typeCol, e_lastRunCol, e_lastRunStatusCol, e_nextRunCol, e_colCount };
    // Implementation
protected:
    virtual ~CSchedulerView();
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

    CADefragUIDoc* GetDocument( void );
    int GetCurSel( void );
    void SetCurSel( int Index );

protected:
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnWindowPosChanging(WINDOWPOS FAR* lpwndpos);
    afx_msg void OnEditItem(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnDeleteItem(NMHDR *pNMHDR, LRESULT *pResult);
    // Commands
    afx_msg void OnUpdateNewOneTime(CCmdUI* pCmdUI);
    afx_msg void OnNewOneTime();
    afx_msg void OnUpdateNewDaily(CCmdUI* pCmdUI);
    afx_msg void OnNewDaily();
    afx_msg void OnUpdateNewWeekly(CCmdUI* pCmdUI);
    afx_msg void OnNewWeekly();
    afx_msg void OnUpdateNewScreenSaver(CCmdUI* pCmdUI);
    afx_msg void OnNewScreenSaver();
    // Run
    afx_msg void OnUpdateRun(CCmdUI* pCmdUI);
    afx_msg void OnRun();
    // Edit
    afx_msg void OnUpdateEnableDisable(CCmdUI* pCmdUI);
    afx_msg void OnEnableDisable();
    afx_msg void OnUpdateEdit(CCmdUI* pCmdUI);
    afx_msg void OnEdit();
    afx_msg void OnUpdateDelete(CCmdUI* pCmdUI);
    afx_msg void OnDelete();
    // Timer
    afx_msg void OnTimer(UINT_PTR NIDEvent);
    DECLARE_MESSAGE_MAP()

    void NewSchedule( Scheduler::EPeriodType period );
    void AddSheduleTask( CScheduleTask* task );
    void UpdateSheduleTask( int index );
    void EditSchedule( int index );
    void UpdateAllTasks();

protected:
    CScheduleListCtrl m_listCtrl;
    HACCEL m_hAccel;
    __time64_t m_lastUpdateTime;
};

#endif // __SCHEDULERVIEW_H__
