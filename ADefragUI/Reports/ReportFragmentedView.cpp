// Reports\ReportFragmentedView.cpp : implementation file
//

#include "stdafx.h"
#include "ReportFragmentedView.h"
#include "Reports.h"
#include <atlpath.h>


// CReportFragmentedView

#define IMG_DRIVE               _T("drive_defrag.bmp")
#define IMG_LEGEND1             _T("defrag_legend1.bmp")
#define IMG_LEGEND2             _T("defrag_legend2.bmp")

IMPLEMENT_DYNCREATE(CReportFragmentedView, CSSSDHtmlView)

CReportFragmentedView::CReportFragmentedView()
{

}

CReportFragmentedView::~CReportFragmentedView()
{
}


BEGIN_MESSAGE_MAP(CReportFragmentedView, CSSSDHtmlView)
END_MESSAGE_MAP()

//
//
//
void CReportFragmentedView::UpdateHTML( CStringA& szHTML, LPCTSTR FilePath )
{
    CADefragUIDoc* pDoc = GetDocument();

    BOOL IsUnknown = FALSE;
    CDiskStatus ds;

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

    DFRG_VOLUME_INFO VolumeInfoBefore;
    DFRG_VOLUME_INFO VolumeInfoAfter;
    BOOL isUnknownVolumeInfo = FALSE;
    if( NO_ERROR != pDoc->GetVolumeInfo( &VolumeInfoBefore, &VolumeInfoAfter ) )
    {
        isUnknownVolumeInfo = TRUE;
    }

    DriveHealth health( ds, VolumeInfoAfter );

    ULARGE_INTEGER FilesSize;
    FilesSize.QuadPart= ds.nSize.QuadPart - ds.nFree.QuadPart;
    DrawDiagramToBmp( FilePath, FilesSize, VolumeInfoAfter.Statistic.FragmentedFilesSize );

    ReplacePattern( szHTML , _T("@DiskName@"),         ds.Name,      IsUnknown );
    ReplacePattern( szHTML , _T("@TotalSize@"),        ds.Size,      IsUnknown );
    CString ratio;
    ratio.Format( _T("%.03f"), VolumeInfoAfter.Statistic.FragmentedFilesSize.QuadPart * 100. / FilesSize.QuadPart );
    ReplacePattern( szHTML , _T("@Fragmentation@"),    ratio,      IsUnknown );

    ReplacePattern( szHTML , _T("@UsedSpace@"),          FilesSize,      IsUnknown );
    ReplacePattern( szHTML , _T("@FragmentedSpace@"),    VolumeInfoAfter.Statistic.FragmentedFilesSize, isUnknownVolumeInfo );

    ReplacePattern( szHTML , _T("@ElapsedTime@"),        _T("00:00:00") );    //?? TODO
    ReplacePattern( szHTML , _T("@CPUTime@"),            _T("00:00:00") );    //?? TODO
    ReplacePattern( szHTML , _T("@FilesDefragmented@"),   VolumeInfoAfter.Statistic.DefragmentedFilesNum, isUnknownVolumeInfo );
    ReplacePattern( szHTML , _T("@DriveHealth@"),        (float)health.GetOveralFragmentation(), isUnknownVolumeInfo );

    ReplacePattern( szHTML , _T("@TotalNumberOfFiles@"), VolumeInfoAfter.Statistic.FilesNum, isUnknownVolumeInfo );
    ReplacePattern( szHTML , _T("@FilesSize@"),          FilesSize,      IsUnknown );
    LARGE_INTEGER NumberNonFragmentedFiles;
    NumberNonFragmentedFiles.QuadPart = VolumeInfoBefore.Statistic.FilesNum.QuadPart - VolumeInfoBefore.Statistic.FragmentedFilesNum.QuadPart;
    ReplacePattern( szHTML , _T("@NumberNonFragmentedFiles@"), NumberNonFragmentedFiles, isUnknownVolumeInfo );
    LARGE_INTEGER NonFragmentedFilesSize;
    NonFragmentedFilesSize.QuadPart = FilesSize.QuadPart - VolumeInfoAfter.Statistic.FragmentedFilesSize.QuadPart;
    ReplacePattern( szHTML , _T("@NonFragmentedFilesSize@"), NonFragmentedFilesSize, isUnknownVolumeInfo );
    ReplacePattern( szHTML , _T("@FragmentedFilesSize@"), VolumeInfoAfter.Statistic.FragmentedFilesSize, isUnknownVolumeInfo );
    ReplacePattern( szHTML , _T("@NumberExcessFileFragments@"), VolumeInfoAfter.Statistic.FragmentedFilesExcessNum, isUnknownVolumeInfo );
    ReplacePattern( szHTML , _T("@FileFragmentationHealth@"), health.GetFileFragmentationHealth( NULL ), isUnknownVolumeInfo );
    ReplacePattern( szHTML , _T("@LargestFreeSpaceHealth@"), health.GetLargestFreeSpaceHealth(), isUnknownVolumeInfo );

    ReplacePattern( szHTML , _T("@LargestFreeSpaceChunkBefore@"), VolumeInfoBefore.Statistic.MaxFreeSpaceGap, isUnknownVolumeInfo );
    ReplacePattern( szHTML , _T("@PercentFreeSpaceFragmentationBefore@"), VolumeInfoBefore.Statistic.FreeSpaceFragmentationRatio, isUnknownVolumeInfo );
    ReplacePattern( szHTML , _T("@PercentFileFragmentationBefore@"), VolumeInfoBefore.Statistic.FragmentedFilesFragmentationRatio, isUnknownVolumeInfo  );
    ReplacePattern( szHTML , _T("@NumberFragmentedFilesBefore@"), VolumeInfoBefore.Statistic.FragmentedFilesNum, isUnknownVolumeInfo );

    ReplacePattern( szHTML , _T("@LargestFreeSpaceChunkAfter@"), VolumeInfoAfter.Statistic.MaxFreeSpaceGap, isUnknownVolumeInfo );
    ReplacePattern( szHTML , _T("@PercentFreeSpaceFragmentationAfter@"), VolumeInfoAfter.Statistic.FreeSpaceFragmentationRatio, isUnknownVolumeInfo );
    ReplacePattern( szHTML , _T("@PercentFileFragmentationAfter@"), VolumeInfoAfter.Statistic.FragmentedFilesFragmentationRatio, isUnknownVolumeInfo );
    ReplacePattern( szHTML , _T("@NumberFragmentedFilesAfter@"), VolumeInfoAfter.Statistic.FragmentedFilesNum, isUnknownVolumeInfo );
}


//
//
//
void CReportFragmentedView::DrawDiagramToBmp( CString PathToSave, ULARGE_INTEGER totalSize, LARGE_INTEGER fragmentedSize )
{
    CADefragUIDoc* pDoc = GetDocument();
    HDC dc = ::GetDC( m_hWnd );

    Graphics wndGraphics( dc );
    int Width = DrivPicWidth + 1;
    int Height = DrivPicHeight + DrivPicDY + 1;
	Bitmap backBuffer( Width, Height, &wndGraphics );
	Graphics temp( &backBuffer );
 
    Gdiplus::SolidBrush EllipseBrush( Color(0, 255, 0) );
    Gdiplus::SolidBrush SectorBrush( Color(255, 0, 0) );
    Gdiplus::Pen EllipsePen( Color(0, 0, 0) );     
    //Gdiplus::SolidBrush bgBrush( Color(128, 128, 128) );
    Gdiplus::SolidBrush bgBrush( Color(255, 255, 255) );

    double sweepAngle;
    //Gdiplus::float aa;
    if ( totalSize.QuadPart == 0 )
        sweepAngle = 0.0;
    else
        sweepAngle = -1 * ( 360.0 * fragmentedSize.QuadPart ) / (double)totalSize.QuadPart;

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

    if ( sweepAngle != 0 )
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

// CReportFragmentedView message handlers
