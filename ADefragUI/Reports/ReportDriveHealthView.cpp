// Report\ReportDriveHealthView.cpp : implementation file
//

#include "stdafx.h"
#include "ReportDriveHealthView.h"
#include "Reports.h"


// CReportDriveHealthView

IMPLEMENT_DYNCREATE(CReportDriveHealthView, CSSSDHtmlView)

CReportDriveHealthView::CReportDriveHealthView()
{

}

CReportDriveHealthView::~CReportDriveHealthView()
{
}



BEGIN_MESSAGE_MAP(CReportDriveHealthView, CSSSDHtmlView)
END_MESSAGE_MAP()



// CReportDriveHealthView message handlers
void CReportDriveHealthView::UpdateHTML( CStringA& szHTML, LPCTSTR FilePath )
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
    if( NO_ERROR != pDoc->GetVolumeInfo( &VolumeInfoBefore, &VolumeInfoAfter ) )
    {
        IsUnknown = TRUE;
    }

    DriveHealth health( ds, VolumeInfoAfter );

    //SetHtmlElementValue( _T("BootNumber"), _T("000") );
    // Boot File Fragmentation
    ReplacePattern( szHTML , _T("@BootNumber@"), VolumeInfoAfter.Statistic.BootFilesNum, IsUnknown );
    ReplacePattern( szHTML , _T("@BootSize@"), VolumeInfoAfter.Statistic.BootFilesSize, IsUnknown );
    ReplacePattern( szHTML , _T("@BootFragments@"), VolumeInfoAfter.Statistic.BootFilesExcessNum, IsUnknown );
    ReplacePattern( szHTML , _T("@BootFragmented@"), VolumeInfoAfter.Statistic.BootFilesFragmentationRatio, IsUnknown );
    ReplacePattern( szHTML , _T("@BootHealth@"), health.GetBootFragmentationHealth() );

    // Page File Fragmentation
    ReplacePattern( szHTML , _T("@PageNumber@"), VolumeInfoAfter.Statistic.PagefileFilesNum, IsUnknown );
    ReplacePattern( szHTML , _T("@PageSize@"), VolumeInfoAfter.Statistic.PagefileFilesSize, IsUnknown );
    ReplacePattern( szHTML , _T("@PageFragments@"), VolumeInfoAfter.Statistic.PagefileFilesExcessNum, IsUnknown );
    ReplacePattern( szHTML , _T("@PageFragmented@"), VolumeInfoAfter.Statistic.PagefileFilesFragmentationRatio, IsUnknown );
    ReplacePattern( szHTML , _T("@PageHealth@"), health.GetPageFileFragmentationHealth() );

    // MFT Fragmentation
    ReplacePattern( szHTML , _T("@MFTNumber@"), VolumeInfoAfter.Statistic.MftNum, IsUnknown );
    ReplacePattern( szHTML , _T("@MFTSize@"), VolumeInfoAfter.Statistic.MftSize, IsUnknown );
    ReplacePattern( szHTML , _T("@MFTFragments@"), VolumeInfoAfter.Statistic.MftExcessNum, IsUnknown );
    ReplacePattern( szHTML , _T("@MFTFragmented@"), VolumeInfoAfter.Statistic.MftFragmentationRatio, IsUnknown );
    ReplacePattern( szHTML , _T("@MFTHealth@"), health.GetMFTFragmentationHealth() );

    // Metadata
    ReplacePattern( szHTML , _T("@MetadataNumber@"), VolumeInfoAfter.Statistic.MetadataFilesNum, IsUnknown );
    ReplacePattern( szHTML , _T("@MetadataSize@"), VolumeInfoAfter.Statistic.MetadataFilesSize, IsUnknown );
    ReplacePattern( szHTML , _T("@MetadataFragments@"), VolumeInfoAfter.Statistic.MetadataFilesExcessNum, IsUnknown );
    ReplacePattern( szHTML , _T("@MetadataFragmented@"), VolumeInfoAfter.Statistic.MetadataFilesFragmentationRatio, IsUnknown );
    ReplacePattern( szHTML , _T("@MetadataHealth@"), health.GetMetadataFragmentationHealth() );
}
