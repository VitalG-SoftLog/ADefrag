/*
    Interface of the CSchedulePropSheet class

    Module name:

    OptionsPropSheet.h

    Abstract:

    Interface of the CSchedulePropSheet class

    $Header: /home/CVS_DEFRAG/Defrag/Src/ADefragUI/Scheduler/SchedulePropSheet.h,v 1.2 2009/11/24 18:02:52 roman Exp $
    $Log: SchedulePropSheet.h,v $
    Revision 1.2  2009/11/24 18:02:52  roman
    Initial commit

    Revision 1.1  2009/11/24 14:57:02  dimas
    no message

*/

#ifndef __SCHEDULEPROPSHEET_H__
#define __SCHEDULEPROPSHEET_H__

#pragma once

#include "resource.h"
#include "ScheduleTask.h"

class CSchedulePropSheet : public CMFCPropertySheet
{
    DECLARE_DYNAMIC(CSchedulePropSheet)
public:
    CSchedulePropSheet(CWnd* pWndParent, UINT nSelectedPage = 0);

public:
    virtual BOOL OnInitDialog();
    enum{ page_disks, page_methods, page_time };
    // Overrides
    virtual void OnActivatePage(CPropertyPage* pPage);
    virtual void OnDrawPageHeader (CDC* pDC, int nPage, CRect rectHeader);

    void SetTask( CScheduleTask* task ) { m_task = task; }
    CScheduleTask* GetTask() { return m_task; }

protected:
    CScheduleTask* m_task;
};

class CScheduleBasePage : public CMFCPropertyPage
{
    // Construction
public:
    CScheduleBasePage() {}
    CScheduleBasePage(UINT nIDTemplate, UINT nIDCaption = 0) : CMFCPropertyPage( nIDTemplate, nIDCaption ) {}
    virtual ~CScheduleBasePage() {}

protected:
    CSchedulePropSheet* GetSheet() { return DYNAMIC_DOWNCAST(CSchedulePropSheet, GetParent()); }
    CScheduleTask* GetTask()
    {
        CSchedulePropSheet* sheet = GetSheet();
        return sheet ? sheet->GetTask() : NULL;
    }
};

#endif // __SCHEDULEPROPSHEET_H__
