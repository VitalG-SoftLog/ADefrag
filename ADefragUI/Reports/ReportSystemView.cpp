// Reports\ReportSystemView.cpp : implementation file
//

#include "stdafx.h"
#include "ReportSystemView.h"
#include "Reports.h"
#include <atlpath.h>


// CReportSystemView

#define IMG_DRIVE               _T("drive_space.bmp")
#define IMG_LEGEND1             _T("space_legend1.bmp")
#define IMG_LEGEND2             _T("space_legend2.bmp")

IMPLEMENT_DYNCREATE(CReportSystemView, CSSSDHtmlView)

CReportSystemView::CReportSystemView()
{

}

CReportSystemView::~CReportSystemView()
{
}


BEGIN_MESSAGE_MAP(CReportSystemView, CSSSDHtmlView)
END_MESSAGE_MAP()

//
//
//
void CReportSystemView::UpdateHTML( CStringA& szHTML, LPCTSTR FilePath )
{
    CADefragUIDoc* pDoc = GetDocument();
    BOOL IsUnknown = FALSE;
    CDiskStatus ds;
    CString ImgPath;

    CDiskStatus* lpDiskStatus = NULL;
    pDoc->GetStatInfo(lpDiskStatus);
    if ( lpDiskStatus )
    {
        ds = *lpDiskStatus;
        IsUnknown = ds.IsInvalid;
    }
    else
    {
        IsUnknown = TRUE;
    }

    DrawDiagramToBmp( FilePath );

    ImgPath = IMG_SUBDIR;
    ImgPath.Append( _T("\\") );
    ImgPath.Append( IMG_DRIVE );

    ReplacePattern( szHTML , _T("@img@"),           ImgPath,      FALSE );

    ULARGE_INTEGER tmpULarge;
    tmpULarge.QuadPart= ds.nSize.QuadPart - ds.nFree.QuadPart;
    ReplacePattern( szHTML , _T("@Used@"),          tmpULarge,      IsUnknown );
    ReplacePattern( szHTML , _T("@Free@"),          ds.nFree, IsUnknown );
    ReplacePattern( szHTML , _T("@Name@"),          pDoc->GetCompName(),      FALSE );

    CString analysedStr( _T("-") );
    if( !ds.IsInvalid && ds.AnalysedTime != 0 )
    {
        analysedStr = COleDateTime( ds.AnalysedTime ).Format();
    }
    ReplacePattern( szHTML , _T("@Analyzed@"),      analysedStr,    FALSE );
    ReplacePattern( szHTML , _T("@DriveID@"),       ds.SerialNumber, IsUnknown );

    ReplacePattern( szHTML , _T("@MountPoint@"),    ds.MountPoint, IsUnknown );

    ReplacePattern( szHTML , _T("@DriveName@"),     ds.Label,      IsUnknown );
    ReplacePattern( szHTML , _T("@PDDriveName@"),   ds.Name,      IsUnknown );
    ReplacePattern( szHTML , _T("@Name1@"),         ds.Name,      IsUnknown );

    ReplacePattern( szHTML , _T("@OS@"),            pDoc->GetOSVer(),      FALSE );
    ReplacePattern( szHTML , _T("@FileSystem@"),    ds.FileSystem,      IsUnknown );
    ReplacePattern( szHTML , _T("@Size@"),          ds.Size,      IsUnknown );
    ReplacePattern( szHTML , _T("@Size1@"),         ds.Size,      IsUnknown );
    ReplacePattern( szHTML , _T("@ClusterSize@"),   ds.SectorsPerCluster * ds.BytesPerSector, IsUnknown );
}

//
//
//
void CReportSystemView::DrawDiagramToBmp( CString PathToSave )
{
    HDC dc = ::GetDC( m_hWnd );

    CADefragUIDoc* pDoc = GetDocument();

    CDiskStatus ds;
    BOOL IsUnknown = FALSE;
    CDiskStatus* lpDiskStatus = NULL;

    pDoc->GetStatInfo(lpDiskStatus);
    if ( lpDiskStatus )
    {
        ds = *lpDiskStatus;
        IsUnknown = ds.IsInvalid;
    }
    else
    {
        IsUnknown = TRUE;
    }


    Graphics wndGraphics( dc );
    int Width = DrivPicWidth + 1;
    int Height = DrivPicHeight + DrivPicDY + 1;
	Bitmap backBuffer( Width, Height, &wndGraphics );	
	Graphics temp( &backBuffer );
 
    Gdiplus::SolidBrush EllipseBrush( Color(255, 0, 255) );
    Gdiplus::SolidBrush SectorBrush( Color(0, 0, 255) );
    Gdiplus::Pen EllipsePen( Color(0, 0, 0) );     
    //Gdiplus::SolidBrush bgBrush( Color(128, 128, 128) );
    Gdiplus::SolidBrush bgBrush( Color(255, 255, 255) );

    double sweepAngle;
    //Gdiplus::float aa;
    if ( IsUnknown )
        sweepAngle = 0.0;
    else
        sweepAngle = -1 * ( 360.0 * ( (double)ds.nSize.QuadPart - (double)ds.nFree.QuadPart ) ) / (double)ds.nSize.QuadPart;

    temp.FillRectangle( &bgBrush, 0, 0, Width, Height );


    Gdiplus::GraphicsPath DrivePath;
    //DrivePath.AddArc(0, 0, DrivPicWidth, DrivPicHeight, 180, float( 360 ) );
    DrivePath.AddLine( 0, DrivPicHeight / 2 /*+ DrivPicDY*/, 0, DrivPicHeight / 2 + DrivPicDY );
    DrivePath.AddArc(0, DrivPicDY, DrivPicWidth, DrivPicHeight, 180, float( -180 ) );
    DrivePath.AddLine( DrivPicWidth, DrivPicHeight / 2 + DrivPicDY, DrivPicWidth, DrivPicHeight / 2 );

    temp.FillPath( &EllipseBrush, &DrivePath );
    temp.DrawPath( &EllipsePen,  &DrivePath );

    RectF EllipseRect(  0.0, 0.0, float( DrivPicWidth ), float( DrivPicHeight ) );
    temp.FillEllipse( &EllipseBrush, EllipseRect );
    temp.DrawEllipse( &EllipsePen, EllipseRect );

    if ( !IsUnknown )
    {
        Gdiplus::GraphicsPath SectorPath1;
        PointF SectorPoint;
        double sweepAngle1 = (sweepAngle > -180.0)?(sweepAngle):(-180.0);
        SectorPath1.AddArc(0, 0, DrivPicWidth, DrivPicHeight, 180, float( sweepAngle1 ) );
        SectorPath1.GetLastPoint( &SectorPoint );
        SectorPath1.AddLine( SectorPoint.X, SectorPoint.Y, SectorPoint.X, SectorPoint.Y + DrivPicDY ); 

    
        SectorPath1.AddArc(0, DrivPicDY, DrivPicWidth, DrivPicHeight, float( 180 + sweepAngle1 ), float( -1 * sweepAngle1 ) );
    
        temp.FillPath( &SectorBrush, &SectorPath1 );
        temp.DrawPath( &EllipsePen,  &SectorPath1 );
    

        Gdiplus::GraphicsPath SectorPath2;
        SectorPath2.AddLine( DrivPicWidth / 2, DrivPicHeight / 2, 0, DrivPicHeight / 2 );        
        SectorPath2.AddArc(0, 0, DrivPicWidth, DrivPicHeight, 180, float( sweepAngle ) );
        SectorPath2.GetLastPoint( &SectorPoint );
        SectorPath2.AddLine( SectorPoint.X, SectorPoint.Y, float( DrivPicWidth / 2 ), float( DrivPicHeight / 2 ) ); 

        temp.FillPath( &SectorBrush, &SectorPath2 );
        temp.DrawPath( &EllipsePen,  &SectorPath2 );
    }

    CString Path;

    if ( PathToSave.GetLength() )
    {
        TCHAR lpszPath[ _MAX_PATH ];
        wcscpy( lpszPath, PathToSave );
        ATLPath::RemoveFileSpec( lpszPath );
    
        Path = lpszPath;
        Path.Append( _T("\\") );        
        Path.Append( IMG_SUBDIR );
        if ( !ATLPath::FileExists( Path ) )
            CreateDirectory( Path, NULL );
    }
    else
    {
        Path = pDoc->GetReportImgTmpDir();
    }

    
    Path.Append( _T("\\") );

    CString PathPic     = Path;
    CString PathLegend1 = Path;
    CString PathLegend2 = Path;

    PathPic.Append( IMG_DRIVE );
    PathLegend1.Append( IMG_LEGEND1 );
    PathLegend2.Append( IMG_LEGEND2 );
   
    CLSID PicEncoder = pDoc->GetPicEncoder();
    backBuffer.Save( PathPic, &PicEncoder );


    Bitmap backBuffer1( LegendBlockWidth, LegendBlockHeight, &wndGraphics );
	Graphics temp1( &backBuffer1 );
    temp1.FillRectangle( &EllipseBrush, 0, 0, LegendBlockWidth, LegendBlockHeight );
    backBuffer1.Save( PathLegend1, &PicEncoder );

    Bitmap backBuffer2( LegendBlockWidth, LegendBlockHeight, &wndGraphics );
	Graphics temp2( &backBuffer2 );
    temp2.FillRectangle( &SectorBrush, 0, 0, LegendBlockWidth, LegendBlockHeight );
    backBuffer2.Save( PathLegend2, &PicEncoder );


    ::ReleaseDC( m_hWnd, dc );
    
}

// CReportSystemView message handlers
