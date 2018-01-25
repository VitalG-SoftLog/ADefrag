/*
    Defrag DriveHealth class module 

    Module name:

        DriveInfo.cpp

    Abstract:

        Contains implementation of the DriveHealth class.

    $Header: /home/CVS_DEFRAG/Defrag/Src/ADefragUI/DriveInfo.cpp,v 1.6 2009/12/24 22:02:50 roman Exp $
    $Log: DriveInfo.cpp,v $
    Revision 1.6  2009/12/24 22:02:50  roman
    getoveral fragmwentation info fix

    Revision 1.5  2009/12/21 23:28:08  roman
    Excluded file list impl.

    Revision 1.4  2009/12/14 10:27:42  dimas
    CVS headers included


*/

#include "stdafx.h"
#include "resource.h"
#include "DriveInfo.h"

DriveHealth::DriveHealth( const CDiskStatus& diskStatus, const DFRG_VOLUME_INFO& volumeInfo ) :
    m_diskStatus( diskStatus ),
    m_volumeInfo( volumeInfo )
{
}

double DriveHealth::GetOveralFragmentation()
{
    ULARGE_INTEGER FilesSize;
    FilesSize.QuadPart= m_diskStatus.nSize.QuadPart - m_diskStatus.nFree.QuadPart;
    return m_volumeInfo.Statistic.FragmentedFilesSize.QuadPart * 100. / FilesSize.QuadPart;
}

void DriveHealth::GetOveralFragmentationInfo( double& fragmentation, DriveHealth::HealthLevel& level, CString& description, CString& recomendations )
{
    fragmentation = GetOveralFragmentation();
    description.Format( IDS_OVERAL_FRAGMENTATION, m_diskStatus.Name, fragmentation );
    if( fragmentation <= 10. )
    {
        level = e_LevelGood;
        recomendations.LoadString( IDS_OVERAL_RECOMEND_GOOD );
    } else if( fragmentation <= 50. )
    {
        level = e_LevelWarning;
        recomendations.LoadString( IDS_OVERAL_RECOMEND_POOR );
    } else
    {
        level = e_LevelCritical;
        recomendations.LoadString( IDS_OVERAL_RECOMEND_BAD );
    }
}

void DriveHealth::GetMFTFragmentationInfo( ULARGE_INTEGER& fragments, DriveHealth::HealthLevel& level, CString& description, CString& recomendations )
{
	fragments.QuadPart = m_volumeInfo.Statistic.MftExcessNum.QuadPart;
    description.Format( IDS_MFT_FRAGMENTATION,
        fragments.QuadPart,
        m_volumeInfo.Statistic.MftFragmentationRatio,
        m_volumeInfo.Statistic.MetadataFilesFragmentationRatio );
    if( fragments.QuadPart <= 250 )
    {
        level = e_LevelGood;
        recomendations.LoadString( IDS_MFT_RECOMEND_GOOD );
    } else if( fragments.QuadPart <= 2000 )
    {
        level = e_LevelWarning;
        recomendations.LoadString( IDS_MFT_RECOMEND_POOR );
    } else
    {
        level = e_LevelCritical;
        recomendations.LoadString( IDS_MFT_RECOMEND_BAD );
    }
}

void DriveHealth::GetPageFileFragmentationInfo( ULARGE_INTEGER& fragments, DriveHealth::HealthLevel& level, CString& description, CString& recomendations )
{
    fragments.QuadPart = m_volumeInfo.Statistic.PagefileFilesExcessNum.QuadPart;
    description.Format( IDS_PAGEFILE_FRAGMENTATION,
        fragments.QuadPart, m_volumeInfo.Statistic.PagefileFilesFragmentationRatio );
    if( fragments.QuadPart <= 250 )
    {
        level = e_LevelGood;
        recomendations.LoadString( IDS_PAGEFILE_RECOMEND_GOOD );
    } else if( fragments.QuadPart <= 1500 )
    {
        level = e_LevelWarning;
        recomendations.LoadString( IDS_PAGEFILE_RECOMEND_POOR );
    } else
    {
        level = e_LevelCritical;
        recomendations.LoadString( IDS_PAGEFILE_RECOMEND_BAD );
    }
}

void DriveHealth::GetFreeSpaceFragmentationInfo( double& space, DriveHealth::HealthLevel& level, CString& description, CString& recomendations )
{
    space = m_diskStatus.nFree.QuadPart * 100. / m_diskStatus.nSize.QuadPart;
    description.Format( IDS_FREESPACE_FRAGMENTATION,
        space, m_volumeInfo.Statistic.FreeSpaceFragmentationRatio );
    if( space >= 15. )
    {
        level = e_LevelGood;
        recomendations.LoadString( IDS_FREESPACE_RECOMEND_GOOD );
    } else if( space >= 5. )
    {
        level = e_LevelWarning;
        recomendations.LoadString( IDS_FREESPACE_RECOMEND_POOR );
    } else
    {
        level = e_LevelCritical;
        recomendations.LoadString( IDS_FREESPACE_RECOMEND_BAD );
    }
}

CString DriveHealth::GetFileFragmentationHealth( CString* description )
{
    CString health;
    double overal = GetOveralFragmentation();
    if( overal < 10. )
    {
        health.LoadString( IDS_HEALTH_EXCELLENT );
        if( description ) description->LoadString( IDS_HEALTH_EXCELLENT_DESC );
    }
    else if( overal < 30. )
    {
        health.LoadString( IDS_HEALTH_GOOD );
        if( description ) description->LoadString( IDS_HEALTH_GOOD_DESC );
    }
    else if( overal < 50. )
    {
        health.LoadString( IDS_HEALTH_PASSED );
        if( description ) description->LoadString( IDS_HEALTH_PASSED_DESC );
    }
    else if( overal < 90. )
    {
        health.LoadString( IDS_HEALTH_POOR );
        if( description ) description->LoadString( IDS_HEALTH_POOR_DESC );
    }
    else
    {
        health.LoadString( IDS_HEALTH_DRAMATIC );
        if( description ) description->LoadString( IDS_HEALTH_DRAMATIC_DESC );
    }
    return health;
}

CString DriveHealth::GetLargestFreeSpaceHealth()
{
    CString health;
    double space = 0;
    DriveHealth::HealthLevel level = e_LevelGood;
    CString description;
    CString recomendations;
    GetFreeSpaceFragmentationInfo( space, level, description, recomendations );
    return GetHealth( level );
}

CString DriveHealth::GetBootFragmentationHealth()
{
    CString health;
    double overal = m_volumeInfo.Statistic.BootFilesFragmentationRatio;
    if( overal < 10. )
    {
        health.LoadString( IDS_HEALTH_EXCELLENT );
    }
    else if( overal < 30. )
    {
        health.LoadString( IDS_HEALTH_GOOD );
    }
    else if( overal < 50. )
    {
        health.LoadString( IDS_HEALTH_PASSED );
    }
    else if( overal < 90. )
    {
        health.LoadString( IDS_HEALTH_POOR );
    }
    else
    {
        health.LoadString( IDS_HEALTH_DRAMATIC );
    }
    return health;
}

CString DriveHealth::GetPageFileFragmentationHealth()
{
    ULARGE_INTEGER fragments;
    DriveHealth::HealthLevel level;
    CString description;
    CString recomendations;
    GetPageFileFragmentationInfo( fragments, level, description, recomendations );
    return GetHealth( level );
}

CString DriveHealth::GetHealth( HealthLevel level )
{
    CString health;
    switch( level )
    {
    case e_LevelGood: health.LoadString( IDS_HEALTH_GOOD ); break;
    case e_LevelWarning: health.LoadString( IDS_HEALTH_PASSED ); break;
    case e_LevelCritical: health.LoadString( IDS_HEALTH_POOR ); break;
    }
    return health;
}

CString DriveHealth::GetMFTFragmentationHealth()
{
    ULARGE_INTEGER fragments;
    DriveHealth::HealthLevel level;
    CString description;
    CString recomendations;
    GetMFTFragmentationInfo( fragments, level, description, recomendations );
    return GetHealth( level );
}

CString DriveHealth::GetMetadataFragmentationHealth()
{
    CString health;
    double overal = m_volumeInfo.Statistic.MetadataFilesFragmentationRatio;
    if( overal < 10. )
    {
        health.LoadString( IDS_HEALTH_EXCELLENT );
    }
    else if( overal < 30. )
    {
        health.LoadString( IDS_HEALTH_GOOD );
    }
    else if( overal < 50. )
    {
        health.LoadString( IDS_HEALTH_PASSED );
    }
    else if( overal < 90. )
    {
        health.LoadString( IDS_HEALTH_POOR );
    }
    else
    {
        health.LoadString( IDS_HEALTH_DRAMATIC );
    }
    return health;
}
