/*
    Interface of the CDefragOptionsPropSheet class

    Module name:

    OptionsPropSheet.h

    Abstract:

    Interface of the CDefragOptionsPropSheet class

    $Header: /home/CVS_DEFRAG/Defrag/Src/ADefragUI/Settings/OptionsPropSheet.h,v 1.2 2009/11/24 18:02:52 roman Exp $
    $Log: OptionsPropSheet.h,v $
    Revision 1.2  2009/11/24 18:02:52  roman
    Initial commit

    Revision 1.1  2009/11/24 14:56:45  dimas
    no message

*/

#ifndef __OPTIONSPROPSHEET_H__
#define __OPTIONSPROPSHEET_H__

#pragma once

#include "DlgBanner.h"
#include "resource.h"

class CDefragOptionsPropSheet : public CMFCPropertySheet
{
public:
    CDefragOptionsPropSheet(CWnd* pWndParent, UINT nSelectedPage = 0);

public:
    virtual BOOL OnInitDialog();
    enum{ page_pupular, page_defrag, page_exclude, page_scheduler, page_log, page_update, page_customize, page_resource };
    int GetAutoUpdateStatus() const { return m_autoUpdateStatus; }
    void SetAutoUpdateStatus( int status ) { m_autoUpdateStatus = status; }

protected:
    virtual void OnDrawPageHeader(CDC* pDC, int nPage, CRect rectHeader);

    CMFCToolBarImages m_Icons;
    CMFCControlRenderer m_Pat[4];
    int m_autoUpdateStatus;
};

#endif // __OPTIONSPROPSHEET_H__
