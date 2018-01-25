/*
    Interface of the CResourcePage class

    Module name:

    ResourcePage.h

    Abstract:

    Interface of the CResourcePage class

    $Header: /home/CVS_DEFRAG/Defrag/Src/ADefragUI/Settings/ResourcePage.h,v 1.2 2009/11/24 18:02:52 roman Exp $
    $Log: ResourcePage.h,v $
    Revision 1.2  2009/11/24 18:02:52  roman
    Initial commit

    Revision 1.1  2009/11/24 14:56:45  dimas
    no message

*/

#ifndef __RESOURCEPAGE_H__
#define __RESOURCEPAGE_H__

#pragma once
#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// CResourcePage dialog

class CResourcePage : public CMFCPropertyPage
{
    DECLARE_DYNCREATE(CResourcePage)

    // Construction
public:
    CResourcePage();
    ~CResourcePage();

    // Dialog Data
    //{{AFX_DATA(CResourcePage)
    enum { IDD = IDD_RES_PAGE };
    //}}AFX_DATA

    // Overrides
protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    // Implementation
protected:
    //{{AFX_MSG(CResourcePage)
    afx_msg void OnHome();
    afx_msg void OnContact();
    afx_msg void OnProduct();
    afx_msg void OnSupport();
    //}}AFX_MSG

    DECLARE_MESSAGE_MAP()

    void OpenAddress( UINT address );
};

#endif // __RESOURCEPAGE_H__
