#pragma once

// CDrvMapWnd
class CDriveMapView;



class CDrvMapWnd : public CWnd
{
	DECLARE_DYNAMIC(CDrvMapWnd)

public:
	CDrvMapWnd();
	virtual ~CDrvMapWnd();

protected:
	HBITMAP			m_hBmp;
    CDriveMapView   *m_pDriveView;


    CDriveMapView* GetParentView();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
protected:
    void DrawBlock(HDC dc, HDC bmp_hDc, int x, int y, BYTE State, BYTE Type);
public:
    virtual BOOL PreTranslateMessage(MSG* pMsg);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
};
