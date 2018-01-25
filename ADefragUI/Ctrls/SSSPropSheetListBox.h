#pragma once


// CSSSPropSheetListBox

class CSSSPropSheetListBox : public CListBox
{	
public:    
	CSSSPropSheetListBox();
	virtual ~CSSSPropSheetListBox();	
	
protected:
    afx_msg void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg LRESULT OnMouseLeave(WPARAM,LPARAM);
	DECLARE_MESSAGE_MAP()

    int  m_nHighlightedItem;
    int  m_nPrevSelItem;
	BOOL m_bTracked;	
};


