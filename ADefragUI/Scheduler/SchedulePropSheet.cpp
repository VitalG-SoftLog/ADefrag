/*
    Implementation of the CSchedulePropSheet class

    Module name:

    SchedulePropSheet.cpp

    Abstract:

    Implementation of the CSchedulePropSheet class

    $Header: /home/CVS_DEFRAG/Defrag/Src/ADefragUI/Scheduler/SchedulePropSheet.cpp,v 1.2 2009/11/24 18:02:52 roman Exp $
    $Log: SchedulePropSheet.cpp,v $
    Revision 1.2  2009/11/24 18:02:52  roman
    Initial commit

    Revision 1.1  2009/11/24 14:57:02  dimas
    no message

*/

#include "stdafx.h"
#include "SchedulePropSheet.h"
#include "../ADefragUI.h"
#include "../MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CSchedulePropSheet, CMFCPropertySheet)

CSchedulePropSheet::CSchedulePropSheet(CWnd* pWndParent, UINT nSelectedPage) :
    CMFCPropertySheet( IDS_SCHEDULE_SHEET, pWndParent, nSelectedPage),
    m_task( NULL )
{
}

BOOL CSchedulePropSheet::OnInitDialog()
{
    BOOL bRes = CMFCPropertySheet::OnInitDialog();

    // Ensure that the Options dialog is fully visible on screen
    CRect rectDialog;
    GetWindowRect(&rectDialog);

    int cxScreen = GetSystemMetrics(SM_CXSCREEN);
    int cyScreen = GetSystemMetrics(SM_CYMAXIMIZED) - (GetSystemMetrics(SM_CYSCREEN) - GetSystemMetrics(SM_CYMAXIMIZED));

    if ((rectDialog.left < 0) || (rectDialog.top < 0))
    {
        SetWindowPos(NULL, rectDialog.left < 0 ? 0 : rectDialog.left, rectDialog.top < 0 ? 0 : rectDialog.top, 0, 0, SWP_NOSIZE);
    }
    else if ((rectDialog.right > cxScreen) || (rectDialog.bottom > cyScreen))
    {
        SetWindowPos(NULL, rectDialog.right > cxScreen ? cxScreen - rectDialog.Width() : rectDialog.left, rectDialog.bottom > cyScreen ? cyScreen - rectDialog.Height() : rectDialog.top, 0, 0, SWP_NOSIZE);
    }

    GetDlgItem(AFX_IDC_TAB_CONTROL)->ShowWindow( SW_HIDE );
    //GetDlgItem(0x3026)->ShowWindow( SW_HIDE );

    //SetWizardButtons( PSWIZB_BACK | PSWIZB_NEXT );
    //GetDlgItem(ID_WIZNEXT)->ShowWindow( SW_SHOW ); //, ID_WIZFINISH, ID_WIZBACK, IDCANCEL
    int ids[] = { ID_WIZBACK, ID_WIZNEXT, ID_WIZFINISH, IDCANCEL, IDHELP };

    int nTotalButtonsWidth = 0;
    const int nVertMargin = 5;
    const int nHorzMargin = 5;
    CRect rectClient;
    GetClientRect(rectClient);

    for (int iStep = 0; iStep < 2; iStep++)
    {
        for (int i = 0; i < sizeof(ids) / sizeof(ids [0]); i++)
        {
            CWnd* pButton = GetDlgItem(ids[i]);

            if (pButton != NULL)
            {
                if (ids [i] == IDHELP &&(m_psh.dwFlags & PSH_HASHELP) == 0)
                {
                    continue;
                }

                CRect rectButton;
                pButton->GetWindowRect(rectButton);
                ScreenToClient(rectButton);

                if (iStep == 0)
                {
                    // Align buttons at the bottom
                    pButton->SetWindowPos(&wndTop, rectButton.left, rectClient.bottom - rectButton.Height() - nVertMargin, -1, -1, SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);

                    nTotalButtonsWidth = rectButton.right;
                }
                else
                {
                    // Right align the buttons
                    pButton->SetWindowPos(&wndTop, rectButton.left + rectClient.right - nTotalButtonsWidth - nHorzMargin, rectButton.top, -1, -1, SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);
                }
            }
        }
    }

    return bRes;
}

void CSchedulePropSheet::OnActivatePage(CPropertyPage* page)
{
    CMFCPropertyPage* pageMFC = DYNAMIC_DOWNCAST(CMFCPropertyPage, page);
    if( pageMFC == NULL )
    {
        ASSERT(FALSE);
        return;
    }
    //pageMFC->SetLayeredWindowAttributes()
    __super::OnActivatePage( page );
    if( m_look == PropSheetLook_List && m_wndList.m_hWnd )
    {
        CRect rc;
        m_wndList.GetWindowRect( &rc );

        CRect rcClient;
        GetClientRect( &rcClient );

        CRect rcPage;
        page->GetWindowRect( &rcPage );
        ScreenToClient( &rcPage );

        int delta = rcClient.right - 5 - rcPage.right;
        rcPage.left += delta;
        rcPage.right += delta;
        rcPage.top = 5;
        //page->MoveWindow( &rcPage );
        ::SetWindowPos( page->m_hWnd, NULL, rcPage.left, rcPage.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE );
        CWnd* line = GetDlgItem(0x3026);
        if( line )
        {
            CRect rcLine;
            line->GetWindowRect( &rcLine );
            ScreenToClient( &rcLine );
            ::SetWindowPos( line->m_hWnd, NULL, rcPage.left, rcLine.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE );
        }
    }
}

void CSchedulePropSheet::OnDrawPageHeader (CDC* pDC, int nPage, CRect rectHeader)
{
    rectHeader.top += 2;
    rectHeader.right -= 7;
    rectHeader.bottom -= 2;

    pDC->FillRect (rectHeader, &afxGlobalData.brBtnFace);
    pDC->Draw3dRect (rectHeader, afxGlobalData.clrBtnShadow, afxGlobalData.clrBtnShadow);

    CDrawingManager dm (*pDC);
    dm.DrawShadow (rectHeader, 2);  

    CString strText;
    switch( nPage )
    {
    case page_disks: strText.LoadString( IDS_PAGE_DISKS_TITLE ); break;
    case page_methods: strText.LoadString( IDS_PAGE_METHODS_TITLE ); break;
    case page_time: strText.LoadString( IDS_PAGE_TIME_TITLE ); break;
    }

    CRect rectText = rectHeader;
    rectText.DeflateRect (10, 0);

    CFont* pOldFont = pDC->SelectObject (&afxGlobalData.fontBold);
    pDC->SetBkMode (TRANSPARENT);
    pDC->SetTextColor (afxGlobalData.clrBtnText);

    pDC->DrawText (strText, rectText, DT_SINGLELINE | DT_VCENTER);

    pDC->SelectObject (pOldFont);
}

