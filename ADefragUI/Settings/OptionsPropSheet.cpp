/*
    Implementation of the CDefragOptionsPropSheet class

    Module name:

    OptionsPropSheet.cpp

    Abstract:

    Implementation of the CDefragOptionsPropSheet class

    $Header: /home/CVS_DEFRAG/Defrag/Src/ADefragUI/Settings/OptionsPropSheet.cpp,v 1.2 2009/11/24 18:02:52 roman Exp $
    $Log: OptionsPropSheet.cpp,v $
    Revision 1.2  2009/11/24 18:02:52  roman
    Initial commit

    Revision 1.1  2009/11/24 14:56:45  dimas
    no message

*/

#include "stdafx.h"
#include "OptionsPropSheet.h"
#include "../ADefragUI.h"
#include "../MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CDefragOptionsPropSheet::CDefragOptionsPropSheet(CWnd* pWndParent, UINT nSelectedPage) :
    CMFCPropertySheet( IDS_SETTINGS_SHEET, pWndParent, nSelectedPage),
    m_autoUpdateStatus( -1 )
{
    m_Icons.SetImageSize( CSize(32, 32) );
    m_Icons.Load( IDB_OPTIONS );

    CMFCControlRendererInfo params(_T(""), CLR_DEFAULT, CRect(0, 0, 350, 60), CRect(83, 58, 266, 1), CRect(0, 0, 0, 0), CRect(0, 0, 0, 0), FALSE);

    params.m_uiBmpResID = IDB_HEADERPAT_1;
    m_Pat[0].Create(params);
    params.m_uiBmpResID = IDB_HEADERPAT_2;
    m_Pat[1].Create(params);
    params.m_uiBmpResID = IDB_HEADERPAT_3;
    m_Pat[2].Create(params);
    params.m_uiBmpResID = IDB_HEADERPAT_4;
    m_Pat[3].Create(params);
}

BOOL CDefragOptionsPropSheet::OnInitDialog()
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

    return bRes;
}

void CDefragOptionsPropSheet::OnDrawPageHeader(CDC* pDC, int nPage, CRect rectHeader)
{
    CSize sizeIcon = m_Icons.GetImageSize();
    CDrawingManager dm(*pDC);

    COLORREF clrFill = afxGlobalData.clrBarFace;
    CMFCControlRenderer* pRenderer = NULL;

    switch (theApp.m_nAppLook)
    {
    case ID_VIEW_APPLOOK_OFF_2007_BLUE:
        pRenderer = &m_Pat[1];
        break;
    case ID_VIEW_APPLOOK_OFF_2007_BLACK:
        pRenderer = &m_Pat[2];
        break;
    case ID_VIEW_APPLOOK_OFF_2007_SILVER:
        pRenderer = &m_Pat[0];
        break;
    case ID_VIEW_APPLOOK_OFF_2007_AQUA:
        pRenderer = &m_Pat[3];
        break;
    }

    if (pRenderer != NULL)
    {
        pRenderer->Draw(pDC, rectHeader);
    }
    else
    {
        dm.FillGradient(rectHeader, pDC->GetPixel(rectHeader.left, rectHeader.bottom), clrFill);
    }

    rectHeader.bottom -= 10;

    CRect rectIcon = rectHeader;
    rectIcon.left += 20;
    rectIcon.right = rectIcon.left + sizeIcon.cx;

    m_Icons.DrawEx(pDC, rectIcon, nPage, CMFCToolBarImages::ImageAlignHorzLeft, CMFCToolBarImages::ImageAlignVertCenter);

    CString strText;

    switch (nPage)
    {
    case page_pupular:
        strText.LoadString( IDS_PAGE_POPULAR_TITLE );
        break;

    case page_defrag:
        strText.LoadString( IDS_PAGE_DEFRAG_TITLE );
        break;

    case page_exclude:
        strText.LoadString( IDS_PAGE_EXCLUDELIST_TITLE );
        break;

    case page_scheduler:
        strText.LoadString( IDS_PAGE_SCHEDULER_TITLE );
        break;

    case page_log:
        strText.LoadString( IDS_PAGE_LOG_TITLE );
        break;

    case page_update:
        strText.LoadString( IDS_PAGE_UPDATE_TITLE );
        break;

    case page_customize:
        strText.LoadString( IDS_PAGE_CUSTOMIZE_TITLE );
        break;

    case page_resource:
        strText.LoadString( IDS_PAGE_RESOURCE_TITLE );
        break;
    }

    CRect rectText = rectHeader;
    rectText.left = rectIcon.right + 10;
    rectText.right -= 20;

    CFont* pOldFont = pDC->SelectObject(&afxGlobalData.fontBold);
    pDC->SetBkMode(TRANSPARENT);
    pDC->SetTextColor(afxGlobalData.clrBarText);

    UINT uiFlags = DT_SINGLELINE | DT_VCENTER;

    CRect rectTextCalc = rectText;
    pDC->DrawText(strText, rectTextCalc, uiFlags | DT_CALCRECT);

    if (rectTextCalc.right > rectText.right)
    {
        rectText.DeflateRect(0, 10);
        uiFlags = DT_WORDBREAK;
    }

    pDC->DrawText(strText, rectText, uiFlags);

    pDC->SelectObject(pOldFont);
}

