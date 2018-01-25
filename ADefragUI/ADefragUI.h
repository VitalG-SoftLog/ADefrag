/*
    Main header file for the ADefragUI application

    Module name:

    ADefragUI.h

    Abstract:

    Main header file for the ADefragUI application

    $Header: /home/CVS_DEFRAG/Defrag/Src/ADefragUI/ADefragUI.h,v 1.3 2009/11/24 18:02:51 roman Exp $
    $Log: ADefragUI.h,v $
    Revision 1.3  2009/11/24 18:02:51  roman
    Initial commit

    Revision 1.2  2009/11/24 17:43:24  roman
    Initial commit

*/

#ifndef __ADEFRAGUI_H__
#define __ADEFRAGUI_H__

#pragma once

#ifndef __AFXWIN_H__
#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols

extern const TCHAR g_MainWindowClassName[];
// CADefragUIApp:
// See ADefragUI.cpp for the implementation of this class
//

class CADefragUIApp : public CWinAppEx
{
public:
    CADefragUIApp();

    // Overrides
public:
    virtual BOOL InitInstance();

    // Implementation
    UINT  m_nAppLook;
    BOOL  m_bHiColorIcons;

    virtual void PreLoadState();
    virtual void LoadCustomState();
    virtual void SaveCustomState();

    afx_msg void OnAppAbout();
    afx_msg void OnFileNewFrame();
    afx_msg void OnFileNew();
    DECLARE_MESSAGE_MAP()

protected:
    BOOL RegisterADefragClassName();
    void ProcessCmdUninstallMode( BOOL& isUninstall );

protected:
    CMultiDocTemplate* m_pDocTemplate;
};

extern CADefragUIApp theApp;

#endif // __ADEFRAGUI_H__
