// Ctrls\SSSPropSheetListBox.cpp : implementation file
//

#include "stdafx.h"
#include "SSSPropSheetListBox.h"
#include "afxtrackmouse.h"


// CSSSPropSheetListBox



CSSSPropSheetListBox::CSSSPropSheetListBox()
{
    m_nHighlightedItem = -1;
	m_bTracked = FALSE;
    m_nPrevSelItem = -1;
}

CSSSPropSheetListBox::~CSSSPropSheetListBox()
{
}


BEGIN_MESSAGE_MAP(CSSSPropSheetListBox, CListBox)
    ON_WM_DRAWITEM_REFLECT()
	ON_WM_MEASUREITEM_REFLECT()
	ON_WM_MOUSEMOVE()
	ON_MESSAGE(WM_MOUSELEAVE, &CSSSPropSheetListBox::OnMouseLeave)
END_MESSAGE_MAP()



// CSSSPropSheetListBox message handlers

//
//
//
void CSSSPropSheetListBox::DrawItem(LPDRAWITEMSTRUCT lpDIS)
{
	int nIndex = lpDIS->itemID;

	if (nIndex < 0)
	{
		return;
	}

	CRect rect = lpDIS->rcItem;

    const BOOL bIsSelected = GetCurSel() == nIndex;
	const BOOL bIsHighlihted = nIndex == m_nHighlightedItem;

	CDC* pDC = CDC::FromHandle(lpDIS->hDC);
	ASSERT_VALID(pDC);

	pDC->SetBkMode(TRANSPARENT);

	CFont* pOldFont = pDC->SelectObject(&afxGlobalData.fontRegular);
	ASSERT_VALID(pOldFont);

	COLORREF clrText = (COLORREF)-1;

	if (bIsHighlihted || bIsSelected)
	{
		clrText = CMFCVisualManager::GetInstance()->OnDrawPropertySheetListItem(pDC, NULL, rect, bIsHighlihted, bIsSelected);
	}

	if (clrText == (COLORREF)-1)
	{
		pDC->SetTextColor(afxGlobalData.clrWindowText);
	}
	else
	{
		pDC->SetTextColor(clrText);
	}

	CRect rectText = rect;
	rectText.DeflateRect(10, 0);

	CString strText;
	GetText(nIndex, strText);

	pDC->DrawText(strText, rectText, DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS);
	pDC->SelectObject(pOldFont);
    
    if ( (m_nPrevSelItem != GetCurSel()) )
    {
        CRect rectItem;
        GetItemRect(m_nPrevSelItem, rectItem);
        InvalidateRect(rectItem);
        m_nPrevSelItem = GetCurSel();
    }
}

//
//
//
void CSSSPropSheetListBox::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	lpMeasureItemStruct->itemHeight = afxGlobalData.GetTextHeight() + 12;
}

//
//
//
void CSSSPropSheetListBox::OnMouseMove(UINT nFlags, CPoint point)
{
	CListBox::OnMouseMove(nFlags, point);

	ASSERT(this->IsWindowEnabled());

	CRect rectItem;

	int nHighlightedItem = -1;

	for (int i = 0; i < GetCount(); i++)
	{
		GetItemRect(i, rectItem);

		if (rectItem.PtInRect(point))
		{
			nHighlightedItem = i;
			break;
		}
	}

	if (!m_bTracked)
	{
		m_bTracked = TRUE;

		TRACKMOUSEEVENT trackmouseevent;
		trackmouseevent.cbSize = sizeof(trackmouseevent);
		trackmouseevent.dwFlags = TME_LEAVE;
		trackmouseevent.hwndTrack = GetSafeHwnd();
		trackmouseevent.dwHoverTime = HOVER_DEFAULT;
		AFXTrackMouse(&trackmouseevent);
	}

	if (nHighlightedItem != m_nHighlightedItem)
	{
		if (m_nHighlightedItem >= 0)
		{
			GetItemRect(m_nHighlightedItem, rectItem);
			InvalidateRect(rectItem);
		}

		m_nHighlightedItem = nHighlightedItem;

		if (m_nHighlightedItem >= 0)
		{
			GetItemRect(m_nHighlightedItem, rectItem);
			InvalidateRect(rectItem);
		}

		UpdateWindow();
	}
}

//
//
//
LRESULT CSSSPropSheetListBox::OnMouseLeave(WPARAM,LPARAM)
{
	m_bTracked = FALSE;

	if (m_nHighlightedItem >= 0)
	{
		CRect rectItem;
		GetItemRect(m_nHighlightedItem, rectItem);

		m_nHighlightedItem = -1;
		RedrawWindow(rectItem);
	}

	return 0;
}


