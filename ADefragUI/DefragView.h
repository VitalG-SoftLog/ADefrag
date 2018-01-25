/*
    Defrag CDefragView class header

    Module name:

        DefragView.h

    Abstract:

        Defines the CDefragView class.

    $Header: /home/CVS_DEFRAG/Defrag/Src/ADefragUI/DefragView.h,v 1.3 2009/12/03 15:36:55 dimas Exp $
    $Log: DefragView.h,v $
    Revision 1.3  2009/12/03 15:36:55  dimas
    CVS headers included

*/

#pragma once

//////////////////////////////////////////////////////////////////////////
// Includes
#include "BaseListView.h"
#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// CDefragView view

class CDefragView : public CBaseListView
{
protected:
    CDefragView();           // protected constructor used by dynamic creation
    DECLARE_DYNCREATE(CDefragView)

// Overrides
public:
    virtual void OnInitialUpdate();
    virtual void OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/);
    void UpdateDrivesListStatus();

// Implementation
protected:
    virtual ~CDefragView();
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

    enum ViewColumns {
        drivelist_col_name,
        drivelist_col_type,
        drivelist_col_status,
        drivelist_col_label,
        drivelist_col_filesys,
        drivelist_col_size,
        drivelist_col_free,
        drivelist_col_last,
        drivelist_col_fragmented,
        drivelist_col_count
    };
    virtual void InitialUpdateList() {}
    virtual UINT GetContextMenuID() { return IDR_POPUP_DFRAGVIEW; }
    virtual UINT GetAcceleratorID() { return 0; }

    void UpdateDrivesList();

protected:
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
    afx_msg void OnActiveDiskSelChanged( NMHDR *pNMHDR, LRESULT *pResult );
    afx_msg void OnLButtonDblClk( NMHDR *pNMHDR, LRESULT *pResult );
    afx_msg void OnNMClick(NMHDR *pNMHDR, LRESULT *pResult);
    //afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg void OnDestroy();
    afx_msg LRESULT OnFileChangeInfo(WPARAM wParam, LPARAM lParam);
    DECLARE_MESSAGE_MAP()
    int GetListIndex( int driveIndex );

protected:
    int PrevPos;
};
