/*
    Defrag DrvMapWnd class module 

    Module name:

        DrvMapWnd.cpp

    Abstract:

        Contains implementation of the DrvMapWnd class.

    $Header: /home/CVS_DEFRAG/Defrag/Src/ADefragUI/DrvMapWnd.cpp,v 1.3 2009/12/14 10:27:42 dimas Exp $
    $Log: DrvMapWnd.cpp,v $
    Revision 1.3  2009/12/14 10:27:42  dimas
    CVS headers included


*/

#include "stdafx.h"
#include "ADefragUI.h"
#include "DrvMapWnd.h"
#include "DriveMapView.h"



// CDrvMapWnd

IMPLEMENT_DYNAMIC(CDrvMapWnd, CWnd)

CDrvMapWnd::CDrvMapWnd()
{

}

CDrvMapWnd::~CDrvMapWnd()
{
}


BEGIN_MESSAGE_MAP(CDrvMapWnd, CWnd)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_CREATE()
    ON_WM_MOUSEMOVE()
    ON_WM_LBUTTONDBLCLK()
END_MESSAGE_MAP()



// CDrvMapWnd message handlers



BOOL CDrvMapWnd::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default

	return TRUE;
}

void CDrvMapWnd::DrawBlock(HDC dc, HDC bmp_hDc, int x, int y, BYTE State, BYTE Type)
{
    CDriveMapView* pDrvView = GetParentView();
    int Index;

    for ( int i = 0; i < BlockStateNewCount; i++ )
        if ( DefaultLegendItem[ i ].BlockStateID == State )
            Index = i;
    if ( pDrvView->m_legendColors[ Index ].BlockMap == NULL )
    {
        pDrvView->m_legendColors[ Index ].BlockMap      = pDrvView->DrawBlockToBmp( dc, State, MAP_BLOCK );
        pDrvView->m_legendColors[ Index ].BlockLegend   = pDrvView->DrawBlockToBmp( dc, State, LEGEND_BLOCK );
    }

    //bmp = (HBITMAP)GetWindowWord(hwnd, (data->blocks[j * xs + i]) * 2);    
    //HDC bmp_hDc = ::CreateCompatibleDC( dc );
    ::SelectObject( bmp_hDc, pDrvView->m_legendColors[ Index ].BlockMap );    
    ::BitBlt( dc, x, y, pDrvView->FBlockWidth + 1, pDrvView->FBlockHeight + 1, bmp_hDc, 0, 0, SRCCOPY );
    //::DeleteDC( bmp_hDc );


}

void CDrvMapWnd::OnPaint()
{
    CDriveMapView* pDrvView = GetParentView(); //m_pDriveView;
    
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
	// Do not call CWnd::OnPaint() for painting messages    

    //
    // Drive map
    //    
    int xs = pDrvView->GetMapColCount();
    int ys = pDrvView->GetMapRowCount();    

    pDrvView->GetDocument()->GetDrvMapStart( xs * ys, pDrvView->m_CurrentLegend );
            
    int k = 0;
    
    //
    // Draw empty map
    //
    HDC bmp_hDc = ::CreateCompatibleDC( dc );
    for( int j = 0; j < ys; j++ )
        for( int i = 0; i < xs; i++ )
        {
            DrawBlock( dc.m_hDC, bmp_hDc,
                i * pDrvView->FBlockWidth,                    
                j * pDrvView->FBlockHeight,
                pDrvView->GetDocument()->GetDrvMapItem( k++ ), MAP_BLOCK ); //j * xs + i

        }
    ::DeleteDC( bmp_hDc );

}

int CDrvMapWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here
	m_hBmp = NULL;

	return 0;
}

CDriveMapView* CDrvMapWnd::GetParentView()
{
    return (CDriveMapView *)GetParent();
}

BOOL CDrvMapWnd::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext)
{
    // TODO: Add your specialized code here and/or call the base class
    
    return CWnd::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);
}


BOOL CDrvMapWnd::PreTranslateMessage(MSG* pMsg)
{
    // TODO: Add your specialized code here and/or call the base class
    
    GetParentView()->PreTranslateMessage(pMsg);

    return CWnd::PreTranslateMessage(pMsg);
}

void CDrvMapWnd::OnMouseMove(UINT nFlags, CPoint point)
{
    // TODO: Add your message handler code here and/or call default
    //m_MapLeft, m_MapTop

    point.x += GetParentView()->m_MapLeft;
    point.y += GetParentView()->m_MapTop;
    GetParentView()->OnMouseMove( nFlags, point );
    //CWnd::OnMouseMove(nFlags, point);
}

void CDrvMapWnd::OnLButtonDblClk(UINT nFlags, CPoint point)
{
    // TODO: Add your message handler code here and/or call default

    point.x += GetParentView()->m_MapLeft;
    point.y += GetParentView()->m_MapTop;
    GetParentView()->OnLButtonDblClk( nFlags, point );

    //CWnd::OnLButtonDblClk(nFlags, point);
}
