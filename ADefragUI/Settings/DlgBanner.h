/*
    Interface of the CDlgBanner class

    Module name:

    DlgBanner.h

    Abstract:

    Interface of the CDlgBanner class

    $Header: /home/CVS_DEFRAG/Defrag/Src/ADefragUI/Settings/DlgBanner.h,v 1.2 2009/11/24 18:02:52 roman Exp $
    $Log: DlgBanner.h,v $
    Revision 1.2  2009/11/24 18:02:52  roman
    Initial commit

    Revision 1.1  2009/11/24 14:56:45  dimas
    no message

*/

#ifndef __DLGBANNER_H__
#define __DLGBANNER_H__

#pragma once

/////////////////////////////////////////////////////////////////////////////
// CDlgBanner window

class CDlgBanner : public CStatic
{
// Construction
public:
	CDlgBanner();

// Implementation
public:
	virtual ~CDlgBanner();

// Generated message map functions
protected:
	//{{AFX_MSG(CDlgBanner)
	afx_msg void OnPaint();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

#endif // __DLGBANNER_H__
