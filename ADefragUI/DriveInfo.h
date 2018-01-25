/*
    Defrag Drive Info classes header

    Module name:

        DriveInfo.h

    Abstract:

        Defines the CDiskStatus & DriveHealth classes.

    $Header: /home/CVS_DEFRAG/Defrag/Src/ADefragUI/DriveInfo.h,v 1.5 2010/01/12 22:53:18 roman Exp $
    $Log: DriveInfo.h,v $
    Revision 1.5  2010/01/12 22:53:18  roman
    Add "Recommended Defrag Method..." button

    Revision 1.4  2009/12/24 01:39:10  roman
    fix errors

    Revision 1.3  2009/12/14 10:27:42  dimas
    CVS headers included


*/

#pragma once

#include <map>

#define MAX_JOB             10
#define DSKNAMELENGTH       20

class CDiskStatus
{
public:
    CDiskStatus() : IsInvalid( TRUE ), AnalysedTime( 0 ) {}
    BOOL  IsInvalid;
    CString Name;
    CString MountPoint;
    CString Size;
    ULARGE_INTEGER nSize;
    CString Free;
    ULARGE_INTEGER nFree;
    CString Type;

    CString Label;
    CString FileSystem;
    DWORD FileSystemFlags;
    DWORD SerialNumber, TotalNumberOfClusters, SectorsPerCluster, BytesPerSector, NumberOfFreeClusters;

    //double DefagLevel;
    DWORD Progress;
    __time64_t AnalysedTime;

    //WORD JobState;
};

class CDriveInfo
{
public:
    CDriveInfo(void) :
      JobId( INVALID_JOB_ID ),
      LastJobId( INVALID_JOB_ID ),
      CurrentJobCommand( 0 ),
      JobLevel( 0 ),
      IsAnalysed( FALSE ),
      JobStackSize( 0 ),
      Map( NULL ),
      MapSize( 0 ),
      IsExternal( FALSE ),
      LastDefragTime( 0 ),
      FAnalyzeBtnState( TRUE ),
      FFreespaceBtnState( TRUE ),
      FDefragBtnState( TRUE ),
      FStopBtnState( FALSE ),
      FPauseBtnState( FALSE ),
	  FRecommendBtnState( FALSE )
    {
    }

    ~CDriveInfo(void)
    {
        if( Map )
        {
            free( Map );
            Map = NULL;
        }
    }

    CString GetFragmentationRatio()
    {
        CString ratio;
        ULARGE_INTEGER FilesSize;
        FilesSize.QuadPart = diskStatus.nSize.QuadPart - diskStatus.nFree.QuadPart;
        ratio.Format( _T("%.03f"), VolumeInfoAfter.Statistic.FragmentedFilesSize.QuadPart * 100. / FilesSize.QuadPart );
        return ratio;
    }
    // Handle of Job
    DWORD JobId;
    DWORD LastJobId;
    DWORD CurrentJobCommand;
    // 0 - Nothing 1 - InitJob; 2 - Analyze; 3 - Defrag; 
    int JobLevel;
    BOOL IsAnalysed;
    CString name;
    CDiskStatus diskStatus;

    int JobStackSize;
    int JobStack[ MAX_JOB ];

    PBYTE Map;
    DWORD MapSize;
    BOOL IsExternal;

    DFRG_VOLUME_INFO VolumeInfoBefore;
    DFRG_VOLUME_INFO VolumeInfoAfter;
    __time64_t LastDefragTime;

    BOOL FAnalyzeBtnState;
    BOOL FFreespaceBtnState;
    BOOL FDefragBtnState;
    BOOL FStopBtnState;
    BOOL FPauseBtnState;
	BOOL FRecommendBtnState;
};

typedef std::vector<CDriveInfo> DriveInfoMap;

class DriveHealth
{
public:
    DriveHealth( const CDiskStatus& diskStatus, const DFRG_VOLUME_INFO& volumeInfo );

    enum HealthLevel
    {
        e_LevelGood,
        e_LevelWarning,
        e_LevelCritical
    };

    static CString GetHealth( HealthLevel level );
    double GetOveralFragmentation();
    void GetOveralFragmentationInfo( double& fragmentation, HealthLevel& level, CString& description, CString& recomendations );
    void GetMFTFragmentationInfo( ULARGE_INTEGER& fragments, HealthLevel& level, CString& description, CString& recomendations );
    void GetPageFileFragmentationInfo( ULARGE_INTEGER& fragments, HealthLevel& level, CString& description, CString& recomendations );
    void GetFreeSpaceFragmentationInfo( double& space, HealthLevel& level, CString& description, CString& recomendations );
    CString GetFileFragmentationHealth( CString* description );
    CString GetLargestFreeSpaceHealth();
    CString GetBootFragmentationHealth();
    CString GetPageFileFragmentationHealth();
    CString GetMFTFragmentationHealth();
    CString GetMetadataFragmentationHealth();

	CString GetCurrentFileName() {
		return m_volumeInfo.Statistic.CurrentFileName;
	}

protected:
    const CDiskStatus& m_diskStatus;
    const DFRG_VOLUME_INFO& m_volumeInfo;
};
