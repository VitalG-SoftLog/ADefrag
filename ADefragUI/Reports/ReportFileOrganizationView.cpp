// Report\ReportFileOrganizationView.cpp : implementation file
//

#include "stdafx.h"
#include "ReportFileOrganizationView.h"
#include "Reports.h"


// CReportFileOrganizationView

IMPLEMENT_DYNCREATE(CReportFileOrganizationView, CSSSDHtmlView)

CReportFileOrganizationView::CReportFileOrganizationView()
{

}

CReportFileOrganizationView::~CReportFileOrganizationView()
{
}



BEGIN_MESSAGE_MAP(CReportFileOrganizationView, CSSSDHtmlView)
END_MESSAGE_MAP()



// CReportFileOrganizationView message handlers
void CReportFileOrganizationView::UpdateHTML( CStringA& szHTML, LPCTSTR FilePath )
{
    //SetHtmlElementValue( _T("BootNumber"), _T("000") );
    BOOL isUnknown = FALSE;
    DFRG_VOLUME_INFO VolumeInfoBefore;
    DFRG_VOLUME_INFO VolumeInfoAfter;
    CADefragUIDoc* pDoc = GetDocument();
    if( NO_ERROR != pDoc->GetVolumeInfo( &VolumeInfoBefore, &VolumeInfoAfter ) )
    {
        isUnknown = TRUE;
    }
    CDiskStatus* lpDiskStatus = NULL; 
    pDoc->GetStatInfo(lpDiskStatus);
    ULARGE_INTEGER size;
    if( lpDiskStatus ) size = lpDiskStatus->nSize;

    // Rarely Modified
    ReplacePattern( szHTML , _T("@AmountRarely@"), VolumeInfoAfter.Statistic.RarelyModifiedNum, isUnknown );
    ReplacePattern( szHTML , _T("@SizeRarely@"), VolumeInfoAfter.Statistic.RarelyModifiedSize, isUnknown );
    ReplacePattern( szHTML , _T("@DriveRarely@"),
        (float)(100.*VolumeInfoAfter.Statistic.RarelyModifiedSize.QuadPart / size.QuadPart), isUnknown );

    // Occasionally Modified
    ReplacePattern( szHTML , _T("@AmountOcc@"), VolumeInfoAfter.Statistic.OccasionalyModifiedNum, isUnknown );
    ReplacePattern( szHTML , _T("@SizeOcc@"), VolumeInfoAfter.Statistic.OccasionalyModifiedSize, isUnknown );
    ReplacePattern( szHTML , _T("@DriveOcc@"),
        (float)(100.*VolumeInfoAfter.Statistic.OccasionalyModifiedSize.QuadPart / size.QuadPart), isUnknown );

    // Recently Modified
    ReplacePattern( szHTML , _T("@AmountRecently@"), VolumeInfoAfter.Statistic.RecentlyModifiedNum, isUnknown );
    ReplacePattern( szHTML , _T("@SizeRecently@"), VolumeInfoAfter.Statistic.RecentlyModifiedSize, isUnknown );
    ReplacePattern( szHTML , _T("@DriveRecently@"),
        (float)(100.*VolumeInfoAfter.Statistic.RecentlyModifiedSize.QuadPart / size.QuadPart), isUnknown );

    // Directory
    ReplacePattern( szHTML , _T("@AmountDirectory@"), VolumeInfoAfter.Statistic.DirsNum, isUnknown );
    ReplacePattern( szHTML , _T("@SizeDirectory@"), VolumeInfoAfter.Statistic.DirsSize, isUnknown );
    ReplacePattern( szHTML , _T("@DriveDirectory@"),
        (float)(100.*VolumeInfoAfter.Statistic.DirsSize.QuadPart / size.QuadPart), isUnknown );

    // Boot
    ReplacePattern( szHTML , _T("@AmountBoot@"), VolumeInfoAfter.Statistic.BootFilesNum, isUnknown );
    ReplacePattern( szHTML , _T("@SizeBoot@"), VolumeInfoAfter.Statistic.BootFilesSize, isUnknown );
    ReplacePattern( szHTML , _T("@DriveBoot@"),
        (float)(100.*VolumeInfoAfter.Statistic.BootFilesSize.QuadPart / size.QuadPart), isUnknown );

    // Excluded
    ReplacePattern( szHTML , _T("@AmountExcluded@"), (DWORD)0, isUnknown ); //TODO
    ULARGE_INTEGER SizeExcluded = { 0 };
    ReplacePattern( szHTML , _T("@SizeExcluded@"), SizeExcluded, isUnknown ); //TODO
    ReplacePattern( szHTML , _T("@DriveExcluded@"), (DWORD)0, isUnknown ); //TODO

    // Free space
    ReplacePattern( szHTML , _T("@AmountFree@"), VolumeInfoAfter.Statistic.GapsNum, isUnknown );
    ReplacePattern( szHTML , _T("@SizeFree@"), VolumeInfoAfter.Statistic.FreeSpaceSize, isUnknown );
    ReplacePattern( szHTML , _T("@DriveFree@"),
        (float)(100.*VolumeInfoAfter.Statistic.FreeSpaceSize.QuadPart / size.QuadPart), isUnknown );

    // Free space outside the MFT Reserved zone
    ReplacePattern( szHTML , _T("@AmountFreeOutside@"), (DWORD)0, isUnknown ); //TODO
    ULARGE_INTEGER SizeFreeOutside = { 0 };
    ReplacePattern( szHTML , _T("@SizeFreeOutside@"), SizeFreeOutside, isUnknown ); //TODO
    ReplacePattern( szHTML , _T("@DriveFreeOutside@"), (DWORD)0, isUnknown ); //TODO

    // Free space inside the MFT Reserved zone
    ReplacePattern( szHTML , _T("@AmountFreeInside@"), (DWORD)0, isUnknown );  //TODO
    ReplacePattern( szHTML , _T("@SizeFreeInside@"), VolumeInfoAfter.Statistic.FreeSpaceInsideMftSize, isUnknown );
    ReplacePattern( szHTML , _T("@DriveFreeInside@"),
        (float)(100.*VolumeInfoAfter.Statistic.FreeSpaceInsideMftSize.QuadPart / size.QuadPart), isUnknown );

    //////////////////////////////////////////////////////////////////////////
    // File Types
    //////////////////////////////////////////////////////////////////////////
    // Text
    ReplacePattern( szHTML , _T("@TextSize@"), VolumeInfoAfter.Statistic.TextFilesSize, isUnknown );
    ReplacePattern( szHTML , _T("@TextPercent@"),
        (float)(100.*VolumeInfoAfter.Statistic.TextFilesSize.QuadPart / VolumeInfoAfter.Statistic.FilesSize.QuadPart), isUnknown );
    ReplacePattern( szHTML , _T("@TextNumber@"), VolumeInfoAfter.Statistic.TextFilesNum, isUnknown );

    // Graphic
    ReplacePattern( szHTML , _T("@GraphicSize@"), VolumeInfoAfter.Statistic.GraphicFilesSize, isUnknown );
    ReplacePattern( szHTML , _T("@GraphicPercent@"), 
        (float)(100.*VolumeInfoAfter.Statistic.GraphicFilesSize.QuadPart / VolumeInfoAfter.Statistic.FilesSize.QuadPart), isUnknown );
    ReplacePattern( szHTML , _T("@GraphicNumber@"), VolumeInfoAfter.Statistic.GraphicFilesNum, isUnknown );

    // Program
    ReplacePattern( szHTML , _T("@ProgramSize@"), VolumeInfoAfter.Statistic.ProgramFilesSize, isUnknown );
    ReplacePattern( szHTML , _T("@ProgramPercent@"),
        (float)(100.*VolumeInfoAfter.Statistic.ProgramFilesSize.QuadPart / VolumeInfoAfter.Statistic.FilesSize.QuadPart), isUnknown );
    ReplacePattern( szHTML , _T("@ProgramNumber@"), VolumeInfoAfter.Statistic.ProgramFilesNum, isUnknown );

    // Video
    ReplacePattern( szHTML , _T("@VideoSize@"), VolumeInfoAfter.Statistic.VideoFilesSize, isUnknown );
    ReplacePattern( szHTML , _T("@VideoPercent@"),
        (float)(100.*VolumeInfoAfter.Statistic.VideoFilesSize.QuadPart / VolumeInfoAfter.Statistic.FilesSize.QuadPart), isUnknown );
    ReplacePattern( szHTML , _T("@VideoNumber@"), VolumeInfoAfter.Statistic.VideoFilesNum, isUnknown );

    // Music
    ReplacePattern( szHTML , _T("@MusicSize@"), VolumeInfoAfter.Statistic.MusicFilesSize, isUnknown );
    ReplacePattern( szHTML , _T("@MusicPercent@"),
        (float)(100.*VolumeInfoAfter.Statistic.MusicFilesSize.QuadPart / VolumeInfoAfter.Statistic.FilesSize.QuadPart), isUnknown );
    ReplacePattern( szHTML , _T("@MusicNumber@"), VolumeInfoAfter.Statistic.MusicFilesNum, isUnknown );

    // Temporary
    ULARGE_INTEGER TemporarySize = { 0 };
    ReplacePattern( szHTML , _T("@TemporarySize@"), VolumeInfoAfter.Statistic.TmpFilesSize, isUnknown );
    ReplacePattern( szHTML , _T("@TemporaryPercent@"),
        (float)(100.*VolumeInfoAfter.Statistic.TmpFilesSize.QuadPart / VolumeInfoAfter.Statistic.FilesSize.QuadPart), isUnknown );
    ReplacePattern( szHTML , _T("@TemporaryNumber@"), VolumeInfoAfter.Statistic.TmpFilesNum, isUnknown );
}
