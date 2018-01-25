/*
    Defrag CDriveMapView class module 

    Module name:

        DriveMapView.cpp

    Abstract:

        Contains implementation of the CDriveMapView class.

    $Header: /home/CVS_DEFRAG/Defrag/Src/ADefragUI/DriveMapView.cpp,v 1.7 2009/12/24 01:39:10 roman Exp $
    $Log: DriveMapView.cpp,v $
    Revision 1.7  2009/12/24 01:39:10  roman
    fix errors

    Revision 1.6  2009/12/18 00:16:19  roman
    CInspector multiple fragments view, Exclude list remove operations.

    Revision 1.5  2009/12/15 18:50:46  roman
    Fix bug with start/end cluster calculation from block number for CInspector.

    Revision 1.4  2009/12/14 10:27:42  dimas
    CVS headers included


*/

#include "stdafx.h"
#include "ADefragUIDoc.h"
#include "DriveMapView.h"
#include "ADefragUI.h"
#include "DriveMap.h"
#include "CInspector.h"
#include <math.h>



#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//const BYTE colrefSize       = sizeof(COLORREF) * 8;
//const BYTE byteSize         = sizeof(BYTE) * 8;


//
// Class definition
//
/////////////////////////////////////////////////////////////////////////////
// CDriveMapView

IMPLEMENT_DYNCREATE(CDriveMapView, CView)

CDriveMapView::CDriveMapView()
{
    m_MapLeft       = LEFT_MARGIN + LEFT_SUBMARGIN;
    m_MapTop        = TOP_MARGIN + TOP_HEADER + TOP_SUBMARGIN;

    strSection          = _T("Legend");
    strLegend1Item      = _T("DrvMapLegend");
    strLegend2Item      = _T("DrvMapLegend");
    strBlockWidthItem   = _T("BlockWidth");
    strBlockHeightItem  = _T("BlockHeight");
    strCurrentLegend    = _T("CurrentLegend");
    strShowLegend       = _T("ShowLegend");    

    for ( int  i = 0; i < BlockStateNewCount; i++ )
    {
        m_legendColors[ i ].BlockMap    = NULL;
        m_legendColors[ i ].BlockLegend = NULL;
    }

    LoadLegendOptions();
}

CDriveMapView::~CDriveMapView()
{
    DeleteAllBitmap();
    SaveLegendOptions();
}


BEGIN_MESSAGE_MAP(CDriveMapView, CView)
    ON_WM_CONTEXTMENU()
	ON_WM_WINDOWPOSCHANGING()
	//ON_WM_HSCROLL()
	ON_WM_ERASEBKGND()
	ON_WM_CREATE()
    ON_WM_LBUTTONDBLCLK()
    ON_WM_TIMER()
    ON_WM_MOUSEMOVE()    
    ON_WM_SIZE()
    ON_UPDATE_COMMAND_UI(ID_DRIVEMAPVIEWPOPUP_CLASTERINSPECTOR, &CDriveMapView::OnUpdateDrivemapviewpopupClasterinspector)
    ON_UPDATE_COMMAND_UI(ID_DRIVEMAPVIEWPOPUP_CUSTOMIZEBLOCK, &CDriveMapView::OnUpdateDrivemapviewpopupCustomizeblock)
    ON_COMMAND(ID_DRIVEMAPVIEWPOPUP_CLASTERINSPECTOR, &CDriveMapView::OnDrivemapviewpopupClasterinspector)
    ON_COMMAND(ID_DRIVEMAPVIEWPOPUP_CUSTOMIZEBLOCK, &CDriveMapView::OnDrivemapviewpopupCustomizeblock)
    ON_CBN_SELCHANGE(IDC_COMBOBOX, &CDriveMapView::OnComboBox)
END_MESSAGE_MAP()

//
//
//
int CDriveMapView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (__super::OnCreate(lpCreateStruct) == -1)
		return -1;

	//SetScrollSizes (MM_TEXT, CSize (1000, 1000));
//    SetScrollSizes (MM_TEXT, CSize (0, 0));

    SetTimer( IDTTimer, TimerPeriod, NULL );

    m_ComboBox.Create(
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | CBS_DROPDOWNLIST | LBS_NOTIFY,
        CRect(10, 10, 10, 10), this, IDC_COMBOBOX);    
    m_ComboBox.SendMessage( WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0) );
    //
    // Drive map type ComboBox
    //
    m_ComboBox.AddString( _T("File view") );
    m_ComboBox.AddString( _T("Performance view") );
    int SelItem;
    switch ( m_CurrentLegend )
    {    
    case LegendTypeDrvmap :
        SelItem = 0;
        break;

    case LegendTypePerfomance :
        SelItem = 1;
        break;

    default :
        m_CurrentLegend = LegendTypeDrvmap ;
        SelItem = 0;
        break;

    }
    m_ComboBox.SetCurSel( SelItem );
    
    //
    // Defrag ProgressBar
    //
    m_Progress.Create(WS_CHILD | WS_VISIBLE | PBS_SMOOTH, CRect( 100, 10, 200, 20 ), this, IDC_PROGRESS);    
    //m_Progress.Create(NULL, NULL, WS_CHILD | WS_VISIBLE | PBS_SMOOTH, CRect( 100, 10, 200, 20 ), this, IDC_PROGRESS);    
    m_Progress.SetBarColor( PROGRESSBARCOLOR );
    m_Progress.SetRange( 0, 100 );
    m_Progress.SetPos( 0 );
    m_ProgressVisible = FALSE;
    m_Progress.ShowWindow( m_ProgressVisible );
	
	m_DrvMap.Create(NULL, NULL, WS_CHILD | WS_VISIBLE, CRect(10, 10, 100, 100), this, 500);    
    
	return 0;
}

//
// Width map in Block
//
int CDriveMapView::GetMapColCount( void )
{
    RECT rc;
    ::GetClientRect(m_hWnd, &rc);

    return (rc.right - (LEFT_SUBMARGIN + RIGHT_SUBMARGIN) - LEFT_MARGIN - RIGHT_MARGIN) / FBlockWidth;    
}

//
// Height map in Block
//
int CDriveMapView::GetMapRowCount( void )
{
    RECT rc;
    ::GetClientRect(m_hWnd, &rc);

    int bottom_margin = BOTTOM_MARGIN - ((m_ShowLegend)?0:LEGENDINDENTBOTTOM);
    return (rc.bottom - BOTTOM_SUBMARGIN - (TOP_HEADER + TOP_SUBMARGIN) - bottom_margin - TOP_MARGIN) / FBlockHeight;    
}



//
// Block count
//
int CDriveMapView::GetMapTotalBlockCount( void )
{
    RECT rc;
    ::GetClientRect(m_hWnd, &rc);

    int bottom_margin = BOTTOM_MARGIN - ((m_ShowLegend)?0:LEGENDINDENTBOTTOM);
    int cols = (rc.right - (LEFT_SUBMARGIN + RIGHT_SUBMARGIN) - LEFT_MARGIN - RIGHT_MARGIN) / FBlockWidth;    
    int rows = (rc.bottom - BOTTOM_SUBMARGIN - (TOP_HEADER + TOP_SUBMARGIN) - bottom_margin - TOP_MARGIN) / FBlockHeight;

	return cols * rows;
}

//
// Return index of legend item by map's block position
//
int CDriveMapView::GetBlockIndexInLegend( CPoint point )
{
    if ( ( point.x < m_MapLeft ) || ( point.y < m_MapTop ) ) return -1;

    int xBlock  = ( point.x - m_MapLeft ) / FBlockWidth;
    int yBlock  = ( point.y - m_MapTop ) / FBlockHeight;

    if ( !( ((xBlock >= 0) && (yBlock >= 0)) && ((xBlock < GetMapColCount()) && (yBlock < GetMapRowCount())) ) ) return -1;

    BYTE State = GetDocument()->GetDrvMapItem( xBlock + yBlock * GetMapColCount() );

    int Index = -1;

    for ( int i = 0; i < BlockStateNewCount; i++ )
        if ( DefaultLegendItem[ i ].BlockStateID == State )
            Index = i;

    return Index;
}

//
// Return legend by block position
// X range: [ 0..GetMapColCount() - 1 ]
// Y range: [ 0..GetMapRowCount() - 1 ]
//
CString CDriveMapView::GetBlockLegend( int x, int y )
{    
    //PBYTE drvMap;
    //int Len;
    //WCHAR drvName[ DSKNAMELENGTH ];
    CADefragUIDoc* Doc = (CADefragUIDoc*)GetDocument();

    
    //Doc->GetDrvMap(drvMap, Len, drvName);
    //if ( drvMap == NULL )
    //{
//        return GetBlockLegend(100);
//    }    

    //BYTE state = drvMap[ (int)( (float)(x + y * GetMapColCount()) * ( (float)Len / ( (float)GetMapColCount() * (float)GetMapRowCount() ) ) ) ];

    BYTE state = Doc->GetDrvMapItem( x + y * GetMapColCount() );

    return GetBlockLegend( state );
}

//
// Return legend by block type
// Index range: [ 0..BlockStateNewCount - 1 ]
//
CString CDriveMapView::GetBlockLegend( BYTE State )
{    
    CString Str;    

    for ( int i = 0; i < BlockStateNewCount; i++ )
        if ( DefaultLegendItem[ i ].BlockStateID == State )
            return DefaultLegendItem[ i ].Description;

    return _T("Unknown");
}

//
// is Point(x, y) in map region?
//
BOOL CDriveMapView::IsMapRegion( int x, int y )
{
    if ( ( x < m_MapLeft ) || ( y < m_MapTop ) ) return FALSE;

    int xBlock  = ( x - m_MapLeft ) / FBlockWidth;
    int yBlock  = ( y - m_MapTop ) / FBlockHeight;

    return ((xBlock >= 0) && (yBlock >= 0)) && ((xBlock < GetMapColCount()) && (yBlock < GetMapRowCount()));

}

//
// is point in map region?
//
BOOL CDriveMapView::IsMapRegion( CPoint point )
{
    return IsMapRegion( point.x, point.y );    
}

//
//
//
int CDriveMapView::GetLegendItemIndex( CPoint point )
{
    RECT rc;
    ::GetClientRect( m_hWnd, &rc );

    int x, y, ret;

    x = ( point.x - LEGENDLEFTMARGIN ) / LEGENDFIELDWIDTH;
    y = ( point.y - rc.bottom + LEGENDINDENTBOTTOM) / LEGENDFIELDHEIGHT;
    if ( ( x >= 0 ) && ( y >= 0 ) )
    {
        ret = x * LEGENDROW + y;
        return GetIndexByOrder( ret + 1 );        
    }    
    return -1;
}

void CDriveMapView::SetBlockColor( int Index, COLORREF color )
{
    AHSV tmp;
        
    
    m_legendColors[ Index ].SourceColor = color;    

    tmp = Global.ColorManager.RGBtoHSV( Global.ColorManager.COLORREFtoARGB( color ) );
    //Global.ColorManager.HSVChangeValue( tmp, 70 ); // (oshap) removed for improve visualization
    m_legendColors[ Index ].TopColor = Global.ColorManager.HSVtoRGB( tmp );
    //Global.ColorManager.HSVChangeValue( tmp, -140 ); // (oshap) removed for improve visualization
    m_legendColors[ Index ].BottomColor = Global.ColorManager.HSVtoRGB( tmp ); 

    if ( m_legendColors[ Index ].BlockLegend )
    {
        ::DeleteObject( m_legendColors[ Index ].BlockLegend );
        ::DeleteObject( m_legendColors[ Index ].BlockMap );

        m_legendColors[ Index ].BlockLegend = NULL;
        m_legendColors[ Index ].BlockMap    = NULL;
    }
}
//
//
//
void CDriveMapView::LoadLegendOptions()
{
    CWinApp* pApp = AfxGetApp();
    
    //
    // Block dimension
    //
    FBlockWidth     = pApp->GetProfileInt( strSection, strBlockWidthItem, -1 );
    FBlockHeight    = pApp->GetProfileInt( strSection, strBlockHeightItem, -1 );    
    
    if ( (FBlockWidth == -1 ) || ( FBlockHeight == -1 ) )
    {
        FBlockWidth         = DEFAULT_BLOCKWIDTH;
        FBlockHeight        = DEFAULT_BLOCKHEIGHT;
    }

    //
    // Legend type & state
    //
    m_CurrentLegend = pApp->GetProfileInt( strSection, strCurrentLegend, -1 );
    if ( m_CurrentLegend  == -1 )
        m_CurrentLegend = LegendDefaultType;
    int showLegend  = pApp->GetProfileInt( strSection, strShowLegend, -1 ); 
    if ( showLegend == -1)
        m_ShowLegend = LegendDefaultShow;
    else
        m_ShowLegend = (BOOL)showLegend;    

    //
    // Colors
    //
    BYTE *pBuf;
    UINT n;    

    pApp->GetProfileBinary( strSection, strLegend1Item, &pBuf, &n );
    if ( ( BlockStateNewCount * sizeof( COLORREF ) ) != n )
    {
        SetDefaultColor();
    }
    else
        for ( int i = 0; i < BlockStateNewCount; i++ )
        {
            
            SetBlockColor( i, ((COLORREF *)pBuf)[ i ] );
        }    

    delete [] pBuf;

}

//
//
//
void CDriveMapView::SaveLegendOptions()
{
    CWinApp* pApp = AfxGetApp();    

    pApp->WriteProfileInt( strSection, strBlockWidthItem, FBlockWidth );
    pApp->WriteProfileInt( strSection, strBlockHeightItem, FBlockHeight );

    pApp->WriteProfileInt( strSection, strCurrentLegend, m_CurrentLegend );
    pApp->WriteProfileInt( strSection, strShowLegend, (int)m_ShowLegend );    
        
    COLORREF colors[ BlockStateNewCount ];
    for ( int i = 0; i < BlockStateNewCount; i++ )
    {
        colors[ i ] = m_legendColors[ i ].SourceColor;
    }

    pApp->WriteProfileBinary( strSection, strLegend1Item, (LPBYTE)colors, sizeof( colors ) );
}

//
//
//
void CDriveMapView::DialogBlockColor( int Index )
{
    //int Index = GetLegendItemIndex( point );
    //if ( Index != -1 )
    //{            
    CColorDialog ColorDialog( m_legendColors[ Index ].SourceColor, CC_FULLOPEN, NULL );
    
    if ( ColorDialog.DoModal() == IDOK )
    {
        COLORREF color = ColorDialog.GetColor();
        SetBlockColor( Index, color );
        InvalidateRect( NULL, FALSE );               
    }
//    }
}

//
// WM_LBUTTONDBLCLK processing
//
void CDriveMapView::OnLButtonDblClk(UINT, CPoint point)
{
    if ( IsMapRegion( point ) )
    {
        CCInspector CInspDlg;
        CInspDlg.SetDocument( GetDocument() );
        int xBlock  = ( point.x - m_MapLeft ) / FBlockWidth;
        int yBlock  = ( point.y - m_MapTop ) / FBlockHeight;
        CInspDlg.SetInitialBlock( yBlock * GetMapColCount() + xBlock );
        CInspDlg.DoModal();

    }
    else
    {
        int Index = GetLegendItemIndex( point );

        if ( Index != -1 )
        {            
            DialogBlockColor( Index );
        }
    }

}

//
//  WM_TIMER processing
//
void CDriveMapView::OnTimer(UINT_PTR NIDEvent)
{
    //MessageBox( _T("CDriveMapView::OnTimer") );
    CADefragUIDoc* Doc = (CADefragUIDoc*)GetDocument();
    Doc->Timer();

    //
    // Show progressBar if defrag process is active
    //
    if ( Doc->GetBtnState( STOPBTN_STATE ) !=  m_ProgressVisible)
    {
        m_ProgressVisible = Doc->GetBtnState( STOPBTN_STATE );
        m_Progress.ShowWindow( m_ProgressVisible );
        m_Progress.UpdateWindow();
        if ( !m_ProgressVisible ) m_Progress.SetPos( 0 );
    }

    //
    // "Animate" ProgressBar if visible
    //
    if (m_ProgressVisible)
    {
        int Min, Max;
        m_Progress.GetRange( Min, Max );
        if ( m_Progress.GetPos() >= Max)
            m_Progress.SetPos( 0 );
        
        //m_Progress.SetPos( m_Progress.GetPos() + 1 );
        CDiskStatus* Status = NULL;
        Doc->GetStatInfo( Status );
        if ( Status && ( Status->Progress >= 0 ) && ( Status->Progress <= 100 ) )
            m_Progress.SetPos( Status->Progress );

	    DFRG_VOLUME_INFO VolumeInfoBefore;
		DFRG_VOLUME_INFO VolumeInfoAfter;
		if( NO_ERROR == Doc->GetVolumeInfo( &VolumeInfoBefore, &VolumeInfoAfter ) )
		{
			strCurrentFileName = VolumeInfoAfter.Statistic.CurrentFileName;
		}

		OnUpdate(NULL,0L,NULL);
    }
    
}

//
//
//
HBITMAP CDriveMapView::DrawBlockToBmp(HDC dc, BYTE State, BYTE Type)
{
    int x = 0, y = 0;
    
	
    int Width, Height;

    if ( Type == LEGEND_BLOCK)
    {
        Width   = LegendDefaultBlockWidth;
        Height  = LegendDefaultBlockHeight;
            
    }
    else
    {
        Width   = FBlockWidth;
        Height  = FBlockHeight;
    }

    
    Graphics wndGraphics( dc );
	Bitmap backBuffer( Width + 1, Height + 1, &wndGraphics );	
	Graphics temp( &backBuffer );
  

    //temp.DrawEllipse(
    //RectF er(
    DrawBlock( temp, 0, 0, State, Type );

    HBITMAP ret;	
    backBuffer.GetHBITMAP( Color( 0, 0, 0), &ret );
//ImageFormatJPEG
//backBuffer.Save(
        
    return ret;
}

//
// 
//
void CDriveMapView::DrawBlock(
    Graphics& g,
    int x,
    int y,
    BYTE State,
    BYTE Type
                             )
{
    

    int Width, Height;
    int Index = 0;

    for ( int i = 0; i < BlockStateNewCount; i++ )
        if ( DefaultLegendItem[ i ].BlockStateID == State )
            Index = i;

    if ( Type == LEGEND_BLOCK)
    {
        Width   = LegendDefaultBlockWidth;
        Height  = LegendDefaultBlockHeight;
            
    }
    else
    {
        Width   = FBlockWidth;
        Height  = FBlockHeight;
    }

    Rect rcBlock( x, y, Width, Height );
    LinearGradientBrush gradientBrush(
        rcBlock,        
        Color( m_legendColors[ Index ].TopColor ),
        Color( m_legendColors[ Index ].BottomColor ),
        LinearGradientModeVertical);
    g.FillRectangle( &gradientBrush, rcBlock );

    //
    // Test
    // TODO: Define as const
    //    
    Gdiplus::Pen pen( Color( BLOCKBORDERCOLOR ) );

    g.DrawLine(&pen, x, y, x + Width, y); 
    g.DrawLine(&pen, x + Width, y, x + Width, y + Height);
    g.DrawLine(&pen, x + Width, y + Height, x, y + Height);
    g.DrawLine(&pen, x, y + Height, x, y);

    // ----------------------------------------
    
    //int i = (int)FBlockHeight / 2;

    //Rect rcBlockTop(x, y, FBlockWidth, i);
    //Rect rcBlockBottom(x, y + i - 1, FBlockWidth, FBlockHeight - i + 1);
    //

    //LinearGradientBrush gradientBrushTop(
    //    rcBlockTop,
    //    Color( m_legendColors[ Index ][ 0 ] ),
    //    Color( m_legendColors[ Index ][ 1 ] ),
    //    LinearGradientModeVertical);

    //LinearGradientBrush gradientBrushBottom(
    //    rcBlockBottom,
    //    Color( m_legendColors[ Index ][ 1 ] ),
    //    Color( m_legendColors[ Index ][ 0 ] ),
    //    LinearGradientModeVertical);

    //g.FillRectangle( &gradientBrushBottom, rcBlockBottom );
    //g.FillRectangle( &gradientBrushTop, rcBlockTop );
    //

    ////
    //// Test
    //// TODO: Define as const
    ////    
    //Gdiplus::Pen pen( Color( BLOCKBORDERCOLOR ) );

    //g.DrawLine(&pen, x, y, x + FBlockWidth, y); 
    //g.DrawLine(&pen, x + FBlockWidth, y, x + FBlockWidth, y + FBlockHeight);
    //g.DrawLine(&pen, x + FBlockWidth, y + FBlockHeight, x, y + FBlockHeight);
    //g.DrawLine(&pen, x, y + FBlockHeight, x, y);
}

//void FormatNumber(WCHAR *Dest, DWORD sizeInfo)
//{    
//    WCHAR buf[100];
//    CString Str;
//
//    wsprintf ( buf, _T("%I64d"), sizeInfo.QuadPart );
//
//    Str = buf;
//
//    for ( int i = Str.GetLength() - 3; i > 0; i = i - 3 )
//        Str.Insert ( i, ',' );
//
//    wcscpy(Dest, Str);
//
//}

/////////////////////////////////////////////////////////////////////////////
// CDriveMapView drawing

void CDriveMapView::OnDraw(CDC* pDC)
{    
    
    int i, j;

    // Draw progress bar:
    /*CRect rectProgress = rectText;
    rectProgress.DeflateRect(1, 1);

    COLORREF clrBar = (pPane->clrProgressBar == (COLORREF)-1) ? afxGlobalData.clrHilite : pPane->clrProgressBar;

    CMFCVisualManager::GetInstance()->OnDrawStatusBarProgress(pDC, this, rectProgress,
    pPane->nProgressTotal, pPane->nProgressCurr, clrBar, pPane->clrProgressBarDest, pPane->clrProgressText, pPane->bProgressText);*/

    
    //return;

    
    
    //
    // Create a Font object based on the GDI font 
    //
    CFont* pFontOld = (CFont*) pDC->SelectStockObject( DEFAULT_GUI_FONT );
	ASSERT ( pFontOld != NULL );

    Gdiplus::Font font( pDC->m_hDC );

    pDC->SelectObject( pFontOld );
    
    COLORREF bgColor = Global.ColorManager.GetSchemeColor( CSSSColorManager::BGColor );
    //
    // Set ProgressBar bgColor
    //
    /*if ( m_Progress.GetBkColor() != bgColor )
        m_Progress.SetBkColor( bgColor );*/

    Gdiplus::SolidBrush textBrush( Color( Global.ColorManager.COLORREFtoARGB( Global.ColorManager.GetSchemeColor( CSSSColorManager::FontColor ) ) ) );
    Gdiplus::StringFormat stringFormat;

    stringFormat.SetAlignment( StringAlignmentNear );
    stringFormat.SetLineAlignment( StringAlignmentNear );

    //
    // Client size
    //
    RECT rc;
    ::GetClientRect( m_hWnd, &rc );

    //
    // Create Window Graphics object
    //
    Graphics wndGraphics( pDC->m_hDC );

    //
    // Create temporary Graphics object
    //
    Bitmap backBuffer( rc.right, rc.bottom, &wndGraphics );
	//backBuffer.GetHBITMAP(		
    Graphics temp( &backBuffer );

    //
    // Create background brush    
    //               
    SolidBrush bgBrush( Color( Global.ColorManager.COLORREFtoARGB( bgColor ) ) );
    
    //
    // Draw background
    //
    temp.FillRectangle( &bgBrush, rc.left, rc.top, rc.right, rc.bottom );

    //Rect rcHeader(rc.left + LEFT_MARGIN, rc.top + TOP_MARGIN + 1, rc.right - RIGHT_MARGIN - LEFT_MARGIN, TOP_HEADER);
    //
    //LinearGradientBrush gradientBrush(
    //    rcHeader,
    //    Color( 0xFFFFFFFF ),
    //    Color( ColorDown(rb, gb, bb) ),
    //    LinearGradientModeVertical);
    //temp.FillRectangle(&gradientBrush, rcHeader);

    //
    // Get data
    //
    CADefragUIDoc* Doc = (CADefragUIDoc*)GetDocument();
    //Doc->GetDrvMap( drvMap, Len, drvName );
    CDiskStatus* Status = NULL;
    Doc->GetStatInfo( Status );

    int xs = GetMapColCount();
    int ys = GetMapRowCount();

    if ( Status )
    {
		Doc->SetMapBlockCount(xs * ys);

        CString Str = Status->Name;
        //Str.Append( _T(" - Volume map") );
        temp.DrawString(Str, -1, &font,
                    PointF( float( rc.left + LEFT_MARGIN + 10 ), float( rc.top + TOP_MARGIN + 3 ) ), 
                    &stringFormat, &textBrush);    
        Str.Empty();

        double ClasterPerBlock = (double)(Status->TotalNumberOfClusters) / ( (double)xs * (double)ys );
        if ( ClasterPerBlock < 0 ) ClasterPerBlock = 0.0;

		Doc->SetClustersInBlock((int)ClasterPerBlock);
        Doc->SplitTriad( Str, (DWORD)ClasterPerBlock );
        //Doc->SplitTriad( Str, ClasterPerBlock, 2 );

        Str.Append( _T(" Clasters per Block.") );

		if (Doc->GetCurrentJobLevel() > 2 && Doc->GetBtnState( STOPBTN_STATE ))
		{
			Str.Append( _T("         Defragmented file: "));

			if (!strCurrentFileName.IsEmpty()) {
				Str.Append(strCurrentFileName + '.');
			} else {
				Str.Append(_T("Unknown."));
			}
		}
/*
        if ( Doc->GetBtnState( STOPBTN_STATE ) )
        {
            Str.Append( _T("  "));
            Doc->SplitTriad( Str, Status->Progress );
        }
*/
        temp.DrawString(Str, -1, &font,
                    PointF( float( COMBOLEFT + COMBOWIDTH + 5 ), float( rc.top + TOP_MARGIN + 3 ) ), 
                    &stringFormat, &textBrush);
    }

    m_DrvMap.Invalidate( FALSE );

    //
    // Draw legend
    //
    if ( m_ShowLegend )
    {
        BYTE state = 0;
        for( i = 0; i < ( ( BlockStateNewCount / LEGENDROW ) + 1 ); i++ )
            for( j = 0; j < LEGENDROW; j++ )
            {
                if ( state == INVALID_STATEID )
                    break;

                state = GetStateByOrder( ( i * LEGENDROW + j ) + 1 );                

                if ( state == INVALID_STATEID )
                    break;

                DrawBlock(temp, 
                    LEGENDLEFTMARGIN + i * LEGENDFIELDWIDTH, 
                    rc.bottom - LEGENDINDENTBOTTOM + j * LEGENDFIELDHEIGHT,
                    state, LEGEND_BLOCK);

                temp.DrawString(GetBlockLegend(state), -1, &font,
                    PointF( float( LEGENDLEFTMARGIN + i * LEGENDFIELDWIDTH + LEGENDTEXTINDENT ), float( rc.bottom - LEGENDINDENTBOTTOM + j * LEGENDFIELDHEIGHT ) ), 
                    &stringFormat, &textBrush);
             }
        
    }

    //
    // Copy temporary to Window
    //
    wndGraphics.DrawImage(&backBuffer, 0, 0, 0, 0, rc.right, rc.bottom, UnitPixel);
	
}

BOOL CDriveMapView::GetShowLegend()
{
    return m_ShowLegend;
}

void CDriveMapView::SetShowLegend( BOOL Value )
{
    if ( m_ShowLegend == Value ) return;

    m_ShowLegend = Value;
    m_DrvMap.MoveWindow( m_MapLeft, m_MapTop, GetMapColCount() * FBlockWidth + 1, GetMapRowCount() * FBlockHeight + 1, TRUE );
    InvalidateRect( NULL, FALSE );
}

//
//
//
void CDriveMapView::SetDefaultColor()
{
    int i;
    for ( i = 0; i < BlockStateNewCount; i++ )
    {
        SetBlockColor( i, DefaultLegendItem[ i ].SourceColor );
    }

    DeleteAllBitmap();

    InvalidateRect( NULL, FALSE );
}

//
//
//
int CDriveMapView::GetBlockWidth()
{
    return FBlockWidth;
}

int CDriveMapView::GetBlockHeight()
{
    return FBlockHeight;
}
void CDriveMapView::SetBlockWidth( int Value )
{
    if ( FBlockWidth == Value ) return;

    FBlockWidth = Value;

    DeleteAllBitmap();
    m_DrvMap.MoveWindow( m_MapLeft, m_MapTop, GetMapColCount() * FBlockWidth + 1, GetMapRowCount() * FBlockHeight + 1, TRUE );
    InvalidateRect( NULL, FALSE );
}
void CDriveMapView::SetBlockHeight( int Value )
{
    if ( FBlockHeight == Value ) return;

    FBlockHeight = Value;

    DeleteAllBitmap();
    m_DrvMap.MoveWindow( m_MapLeft, m_MapTop, GetMapColCount() * FBlockWidth + 1, GetMapRowCount() * FBlockHeight + 1, TRUE );
    InvalidateRect( NULL, FALSE );
}
BYTE CDriveMapView::GetLegendNum()
{
    return (BYTE)(log( (double)m_CurrentLegend ) / log( 2.0 ));
}

int CDriveMapView::GetIndexByOrder( BYTE order )
{
    for ( int i = 0; i < BlockStateNewCount; i++ )
        if ( ( DefaultLegendItem[ i ].Legend & m_CurrentLegend ) == m_CurrentLegend )        
            if ( order == DefaultLegendItem[ i ].Order[ GetLegendNum() ] )
            {               
                return i;
            }        

    return -1;
}

int CDriveMapView::GetStateByOrder( BYTE order )
{
    for ( int i = 0; i < BlockStateNewCount; i++ )
        if ( ( DefaultLegendItem[ i ].Legend & m_CurrentLegend ) == m_CurrentLegend )        
            if ( order == DefaultLegendItem[ i ].Order[ GetLegendNum() ] )
            {               
                return DefaultLegendItem[ i ].BlockStateID;
            }        

    return INVALID_STATEID;
}

//
//
//
void CDriveMapView::DeleteAllBitmap()
{

    for ( int i = 0; i < BlockStateNewCount; i++ )

        if ( m_legendColors[ i ].BlockLegend ) {

            ::DeleteObject( m_legendColors[ i ].BlockLegend );
            ::DeleteObject( m_legendColors[ i ].BlockMap );

            m_legendColors[ i ].BlockLegend = NULL;
            m_legendColors[ i ].BlockMap    = NULL;

        }
}

//
// Show context menu
//
void CDriveMapView::OnContextMenu(CWnd* /*pWnd*/, CPoint point) 
{       
    m_ClientPoint = point;
    ScreenToClient( &m_ClientPoint );
    
    m_EnableCInspectorMI  = IsMapRegion( m_ClientPoint );
    //m_EnableColorMI = !m_EnableCInspectorMI; 

    int Index = GetLegendItemIndex( m_ClientPoint );

    if ( Index != -1 )

        m_EnableColorMI = TRUE;

    else

        m_EnableColorMI = FALSE;

    m_EnableColorMI |= m_EnableCInspectorMI;


    theApp.ShowPopupMenu( IDR_POPUP_DRIVEMAPVIEW, point, this );   
}

//
//
//
CADefragUIDoc* CDriveMapView::GetDocument(void)
{
    ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CADefragUIDoc)));
    return (CADefragUIDoc*)m_pDocument;
}

/////////////////////////////////////////////////////////////////////////////
// CDriveMapView diagnostics

#ifdef _DEBUG
void CDriveMapView::AssertValid() const
{
	CView::AssertValid();
}

void CDriveMapView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CDriveMapView message handlers
void CDriveMapView::OnInitialUpdate() 
{
	CView::OnInitialUpdate();
	
	//SetScrollSizes (MM_TEXT, CSize (1000, 1000));
//    SetScrollSizes (MM_TEXT, CSize (0, 0));

	CMFCTabCtrl* pTabWnd = DYNAMIC_DOWNCAST (CMFCTabCtrl, GetParent ());
	ASSERT_VALID (pTabWnd);

	pTabWnd->SynchronizeScrollBar ();

    //
    // Create ToolTip object & set params
    //
    //if (m_ToolTip.Create(this, TTS_ALWAYSTIP))    

    if (m_ToolTip.Create(&m_DrvMap, TTS_ALWAYSTIP))
	{
        CMFCToolTipInfo params;
	    params.m_bVislManagerTheme = TRUE;

	    m_ToolTip.SetParams (&params);

		m_ToolTip.SendMessage(TTM_SETMAXTIPWIDTH, 0, SHRT_MAX);
		m_ToolTip.SendMessage(TTM_SETDELAYTIME, TTDT_AUTOPOP, SHRT_MAX);
		m_ToolTip.SendMessage(TTM_SETDELAYTIME, TTDT_INITIAL, 50);
		m_ToolTip.SendMessage(TTM_SETDELAYTIME, TTDT_RESHOW, 50);
	}

}

//
//
//
void CDriveMapView::OnWindowPosChanging(WINDOWPOS FAR* lpwndpos) 
{
	CView::OnWindowPosChanging(lpwndpos);
	
	// Hide horizontal scrollbar:
	//!!ShowScrollBar (SB_HORZ, FALSE);
    //ShowScrollBar (SB_VERT, FALSE);
	//!!ModifyStyle (WS_HSCROLL, 0, SWP_DRAWFRAME);
}

//void CDriveMapView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* /*pScrollBar*/) 
//{
//	CMFCTabCtrl* pTabWnd = DYNAMIC_DOWNCAST (CMFCTabCtrl, GetParent ());
//	ASSERT_VALID (pTabWnd);
//
//	CScrollView::OnHScroll(nSBCode, nPos, pTabWnd->GetScrollBar ());
//}
//
//CScrollBar* CDriveMapView::GetScrollBarCtrl(int nBar) const
//{
//	if (nBar == SB_HORZ)
//	{
//		CMFCTabCtrl* pTabWnd = DYNAMIC_DOWNCAST (CMFCTabCtrl, GetParent ());
//		ASSERT_VALID (pTabWnd);
//
//		return pTabWnd->GetScrollBar ();
//	}
//	
//	return CScrollView::GetScrollBarCtrl(nBar);
//}

//
// Eliminate erase background
//
BOOL CDriveMapView::OnEraseBkgnd(CDC* /*pDC*/) 
{
	return TRUE;
}

//
// 
//
BOOL CDriveMapView::PreTranslateMessage(MSG* pMsg) 
{
    
	if (::IsWindow(m_ToolTip.m_hWnd) && (pMsg->hwnd == m_DrvMap.m_hWnd) )
	{
		switch(pMsg->message)
		{
		case WM_LBUTTONDOWN:
		case WM_MOUSEMOVE:
		case WM_LBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_MBUTTONUP:
			m_ToolTip.RelayEvent(pMsg);
			break;
		}
	}
	return CView::PreTranslateMessage(pMsg);
}


//
//
//
void CDriveMapView::OnMouseMove(UINT nFlags, CPoint point) 
{
    int xBlock  = ( point.x - m_MapLeft ) / FBlockWidth;
    int yBlock  = ( point.y - m_MapTop ) / FBlockHeight;

    if (::IsWindow(m_ToolTip.m_hWnd))
    {
        //
        // Mouse pointer in drav map range
        //        
        if ( IsMapRegion( point ) )
        {
            //
            // Mouse pointer move to new block
            // ToolTip text not set yet
            //
//Block Info
//
//Block Type: Free space
//Start Cluster: Number
//Number of clusters: Number
//
//Clusters Info:
//FragmentedFiles: 0 clusters
//Directories: 0 clusters
//Mft: 0 clusters
//Metadata: 0 clusters
//MftZone: 0 clusters
//PageFile: 0 clusters
//HiberFile: 0 clusters
//FreeSpace: 0 clusters
//LockedFiles: 0 clusters
//BeingProcessed: 0 clusters
//CompressedFiles: 0 clusters
//Allocated: 0 clusters
//
//FreeSpace: 0 clusters (для втогоро типа легенды)
//LeastModifiedFiles: 0 clusters  (для втогоро типа легенды)
//RecentlyModifiedFiles: 0 clusters (для втогоро типа легенды)

            if( (m_PrevBlock.x != xBlock) || (m_PrevBlock.y != yBlock) )
            {
                m_ToolTip.DelTool( &m_DrvMap );

                CString Str;

                //Str.Format( _T("%s"), GetBlockLegend( xBlock, yBlock ));
				Str.Format(_T("Block Number: %d\n"), (yBlock * GetMapColCount()) + xBlock);

                Str.Append( _T("Block type: ") );
                Str.Append( GetBlockLegend( xBlock, yBlock ) );
                Str.Append( _T("\n\n") );

                CString Buf = GetDocument()->GetDrvMapItemInfo( yBlock * GetMapColCount() + xBlock );
                if ( Buf.GetLength() > 0 ) 
                {                    
                    Str.Append( Buf );
                }

                m_ToolTip.SetDescription(Str);
                m_ToolTip.AddTool( &m_DrvMap, _T("Block info") );
                m_PrevBlock.x   = xBlock;
                m_PrevBlock.y   = yBlock;
            }
        }
        else
        {
            m_ToolTip.DelTool( &m_DrvMap );
        }
    }   
}

//
// Mooving and sizing controls
//
void CDriveMapView::OnSize(UINT nType, int cx, int cy)
{
    CView::OnSize(nType, cx, cy);

    // TODO: Add your message handler code here    

    //m_ComboBox.MoveWindow(cx - COMBOXINDENT, cy - COMBOYINDENT, COMBOWIDTH, COMBOHEIGHT, FALSE);
    m_ComboBox.MoveWindow(COMBOLEFT, COMBOTOP, COMBOWIDTH, COMBOHEIGHT, FALSE);
    
    m_Progress.MoveWindow( m_MapLeft, PROGRESSTOP, GetMapColCount() * FBlockWidth, PROGRESSHEIGHT, TRUE);

    m_DrvMap.MoveWindow( m_MapLeft, m_MapTop, GetMapColCount() * FBlockWidth + 1, GetMapRowCount() * FBlockHeight + 1, TRUE );
}

//
// Change window style
//
BOOL CDriveMapView::PreCreateWindow(CREATESTRUCT& cs)
{
    // TODO: Add your specialized code here and/or call the base class

    cs.style |= WS_CLIPCHILDREN;

    return CView::PreCreateWindow(cs);
}

//
// Popup menu event processing
//
void CDriveMapView::OnUpdateDrivemapviewpopupClasterinspector(CCmdUI *pCmdUI)
{
    // TODO: Add your command update UI handler code here

    pCmdUI->Enable( m_EnableCInspectorMI );
}

void CDriveMapView::OnUpdateDrivemapviewpopupCustomizeblock(CCmdUI *pCmdUI)
{
    // TODO: Add your command update UI handler code here

    pCmdUI->Enable( m_EnableColorMI );
}

void CDriveMapView::OnDrivemapviewpopupClasterinspector()
{
    // TODO: Add your command handler code here

    OnLButtonDblClk( 0, m_ClientPoint );
}

void CDriveMapView::OnDrivemapviewpopupCustomizeblock()
{
    // TODO: Add your command handler code here
    
    int Index = GetBlockIndexInLegend( m_ClientPoint );

    if ( Index == -1 )
    {
        Index = GetLegendItemIndex( m_ClientPoint );
    }

    if ( Index != -1 )
    {            
        DialogBlockColor( Index );
    }        
}

void CDriveMapView::OnComboBox()//(UINT id, NMHDR *pNotifyStruct,LRESULT *result)
{
    //
    int NewLegend = ( 1 << m_ComboBox.GetCurSel() );
    if ( NewLegend != m_CurrentLegend )
    {
        m_CurrentLegend = NewLegend;
        Invalidate( FALSE );
    }
}
