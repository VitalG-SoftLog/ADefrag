/*
    Defrag CHelpView class module 

    Module name:

        HelpView.cpp

    Abstract:

        Contains implementation of the CHelpView class.

    $Header: /home/CVS_DEFRAG/Defrag/Src/ADefragUI/HelpView.cpp,v 1.3 2009/12/14 10:30:11 dimas Exp $
    $Log: HelpView.cpp,v $
    Revision 1.3  2009/12/14 10:30:11  dimas
    CVS headers included


*/

#include "stdafx.h"
#include "ADefragUI.h"
#include "resource.h"
#include "HelpView.h"

// CHelpView

IMPLEMENT_DYNCREATE(CHelpView, CFormView)

CHelpView::CHelpView()
: CFormView(CHelpView::IDD)
{

}

CHelpView::~CHelpView()
{
}

void CHelpView::DoDataExchange(CDataExchange* pDX)
{
    CFormView::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_URL_PRODUCT, m_productInfo);
    DDX_Control(pDX, IDC_URL_SUPPORT, m_support);
}

BEGIN_MESSAGE_MAP(CHelpView, CFormView)
    ON_WM_CLOSE()
    ON_WM_ERASEBKGND()
    ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// CHelpView diagnostics

#ifdef _DEBUG
void CHelpView::AssertValid() const
{
    CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CHelpView::Dump(CDumpContext& dc) const
{
    CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CHelpView message handlers
/******************************************************************************
*
*       Function :       OnInitialUpdate
*
*       Purpose :        This method is called in response to the WM_INITDIALOG message.
*
*       Return :         Specifies whether the application has set the input focus to
*                                one of the controls in the dialog box.
*
*       Parameters : None
*
*****************************************************************************/
void CHelpView::OnInitialUpdate()
{
    __super::OnInitialUpdate();
    m_productInfo.SetURL( _T("http://www.ADefrag.com") );
    m_productInfo.SizeToContent();
    m_support.SetURL( _T("support@adefrag.com") );
    m_support.SetURLPrefix( _T("mailto:") );
    m_support.SizeToContent();
}

/******************************************************************************
*
*       Function :       OnInitDialog                                                           
*
*       Purpose :        This method is called in response to the WM_CLOSE message.
*
*       Return :         None
*
*       Parameters : None
*
*****************************************************************************/
void CHelpView::OnClose()
{
    //m_brushDlg.DeleteObject();
    __super::OnClose();
}

/******************************************************************************
*
*       Function :       OnCtlColor                                                             
*
*       Purpose :        This method is called to get controls Brush.
*
*       Return :         Handle to Brush
*
*       Parameters : CDC* pDC, CWnd* pWnd, UINT nCtlColor
*
*****************************************************************************/
HBRUSH CHelpView::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
    UNREFERENCED_PARAMETER(pWnd);
    UNREFERENCED_PARAMETER(nCtlColor);
    UNREFERENCED_PARAMETER(pDC);

    return afxGlobalData.hbrWindow; //m_brushDlg;
}

/******************************************************************************
*
*       Function :       OnEraseBkgnd                                                           
*
*       Purpose :        This method is called to erase background.
*
*       Return :         FALSE to prevent original erasing
*
*       Parameters : CDC* pDC, CWnd* pWnd, UINT nCtlColor
*
*****************************************************************************/
BOOL CHelpView::OnEraseBkgnd(CDC* pDC)
{
    CRect rectWnd;
    GetClientRect( &rectWnd );
    pDC->FillRect( &rectWnd, CBrush::FromHandle(afxGlobalData.hbrWindow) ); //&m_brushDlg );
    return FALSE;
}

void CHelpView::OnDraw(CDC* pDC)
{
    __super::OnDraw(pDC);
    m_productInfo.RedrawWindow();
    m_support.RedrawWindow();
}
