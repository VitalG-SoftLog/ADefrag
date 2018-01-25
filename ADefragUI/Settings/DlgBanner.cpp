/*
    Implementation of the CDlgBanner class

    Module name:

    DlgBanner.h

    Abstract:

    Implementation of the CDlgBanner class

    $Header: /home/CVS_DEFRAG/Defrag/Src/ADefragUI/Settings/DlgBanner.cpp,v 1.2 2009/11/24 18:02:52 roman Exp $
    $Log: DlgBanner.cpp,v $
    Revision 1.2  2009/11/24 18:02:52  roman
    Initial commit

    Revision 1.1  2009/11/24 14:56:44  dimas
    no message

*/

#include "stdafx.h"
#include "DlgBanner.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgBanner

CDlgBanner::CDlgBanner()
{
}

CDlgBanner::~CDlgBanner()
{
}

BEGIN_MESSAGE_MAP(CDlgBanner, CStatic)
    //{{AFX_MSG_MAP(CDlgBanner)
    ON_WM_PAINT()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgBanner message handlers

void CDlgBanner::OnPaint()
{
    CPaintDC dc(this); // device context for painting

    CRect rect;
    GetClientRect(rect);

    CFont* pOldFont = dc.SelectObject( &afxGlobalData.fontBold );
    ASSERT( pOldFont != NULL );

    dc.SetTextColor( CMFCVisualManager::GetInstance()->OnDrawMenuLabel(&dc, rect) );
    dc.SetBkMode( TRANSPARENT );

    rect.DeflateRect( 5, 0 );

    CString strText;
    GetWindowText( strText );

    dc.DrawText( strText, rect, DT_LEFT | DT_SINGLELINE | DT_VCENTER );

    dc.SelectObject( pOldFont );
}
