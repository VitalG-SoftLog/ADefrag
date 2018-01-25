/*
    Defrag CADefragUIDoc class header

    Module name:

        ADefragUIDoc.h

    Abstract:

        Defines the CADefragUIDoc class.

    $Header: /home/CVS_DEFRAG/Defrag/Src/ADefragUI/ADefragUIDoc.h,v 1.11 2010/01/12 22:53:18 roman Exp $
    $Log: ADefragUIDoc.h,v $
    Revision 1.11  2010/01/12 22:53:18  roman
    Add "Recommended Defrag Method..." button

    Revision 1.10  2009/12/29 21:59:39  roman
    Show MFT and MFTZone in performance view

    Revision 1.9  2009/12/24 01:39:10  roman
    fix errors

    Revision 1.8  2009/12/21 23:28:08  roman
    Excluded file list impl.

    Revision 1.7  2009/12/18 00:16:19  roman
    CInspector multiple fragments view, Exclude list remove operations.

    Revision 1.6  2009/12/15 18:50:46  roman
    Fix bug with start/end cluster calculation from block number for CInspector.

    Revision 1.5  2009/12/07 19:57:06  roman
    ClusterInspector implementation.

    Revision 1.4  2009/12/03 15:36:55  dimas
    CVS headers included

*/


#pragma once

#include "Scheduler/ScheduleTask.h"
#include "DriveInfo.h"

#define ADEFRAG_TMP_SUBDIR      _T("AdefragUI")
#define REPORTS_SUBDIR          _T("Reports")
#define IMG_SUBDIR              _T("img")
#define LAST_DEFRAG_TIME        _T("LastDefragTime")
//
// Disk Map states
//
typedef enum {
    FragmentedFiles = 0,    // 0 
    Directories,            // 1
    Mft,                    // 2
    Metadata,               // 3
    MftZone,                // 4
    PageFile,               // 5
    HiberFile,              // 6
    FreeSpace,              // 7
    LockedFiles,            // 8
    BeingProcessed,         // 9
    CompressedFiles,        // 10
    Allocated,              // 11
    LeastModifiedFiles,     // 12
    RecentlyModifiedFiles,  // 13
    BlockStateNewCount      // 14
} BlockStateNew;

//
// Legend types
//
typedef enum {
    LegendTypeDrvmap        = 1,
    LegendTypePerfomance    = 2
} LegendTypes;

#define LEGENDCOUNT                 2
#define BLOCK_DESCRIPTION_LEN       50

//
//
//
typedef struct _legend_item_ 
{
    BlockStateNew   BlockStateID;
    WCHAR           Description[ BLOCK_DESCRIPTION_LEN ];   

    BYTE            Legend;
    
    BYTE            Order[ LEGENDCOUNT ];

    COLORREF        SourceColor;

    double          Weight[ LEGENDCOUNT ];

} LEGEND_ITEM, *LPLEGEND_ITEM;

//
//
//
typedef struct _gui_legend_item_ 
{
    COLORREF        SourceColor;
    Gdiplus::ARGB   TopColor;
    Gdiplus::ARGB   BottomColor;
    HBITMAP         BlockMap;
    HBITMAP         BlockLegend;

} GUI_LEGEND_ITEM, *LPGUI_LEGEND_ITEM;


const LEGEND_ITEM DefaultLegendItem[ BlockStateNewCount ] = 
{
//  BlockState:         Description:            Present in legend:                              Order1      Order2      Color       Weight1     Weight2
    { FragmentedFiles,  L"Fragmented Files",    LegendTypeDrvmap,                             { 1,        0 },        0x4C4AD7,   { 8.0,        0.0} },
    { Directories,      L"Directories",         LegendTypeDrvmap,                             { 2,        0 },        0x3E91FF,   { 32.0,       0.0} },
    { Mft,              L"MFT",                 (LegendTypeDrvmap | LegendTypePerfomance),    { 3,        3 },        0x5FDBEE,   { 1024.0,     32.0} },
    { Metadata,         L"Metadata",            LegendTypeDrvmap,                             { 4,        0 },        0x5CD046,   { 64.0,       0.0} },
    { MftZone,          L"MFT Zone",            (LegendTypeDrvmap | LegendTypePerfomance),    { 5,        5 },        0x0F7B8E,   { 1024.0,     32.0} },
    { PageFile,         L"PageFile",            LegendTypeDrvmap,                             { 6,        0 },        0x525E97,   { 1024.0,     0.0} },
    { HiberFile,        L"Hiberfil",            LegendTypeDrvmap,                             { 7,        0 },        0xFB768A,   { 1024.0,     0.0} },
    { FreeSpace,        L"Free Space",          (LegendTypeDrvmap | LegendTypePerfomance),    { 8,        1 },        0xFFFFFF,   { 0.0,		0.0} },
    { LockedFiles,      L"Locked Files",        LegendTypeDrvmap,                             { 9,        0 },        0x4F4F4F,   { 128.0,      0.0} },
    { BeingProcessed,   L"Being processed",     LegendTypeDrvmap,                             { 10,       0 },        0xFFE5CC,   { 1000000.0,  0.0} },
    { CompressedFiles,  L"Compressed files",    LegendTypeDrvmap,                             { 11,       0 },        0xCCBF78,   { 16.0,       0.0} },
    { Allocated,        L"Allocated",           LegendTypeDrvmap,                             { 12,       0 },        0xE68153,   { 1.0,        0.1} },
    { LeastModifiedFiles,    L"Least Modified Files",       LegendTypePerfomance,             { 0,        2 },        0xE68153,   { 0.0,        1.0} },
    { RecentlyModifiedFiles, L"Recently Modified Files",    LegendTypePerfomance,             { 0,        4 },        0x5CD046,   { 0.0,        2.0} }
};


#define ANALYZEBTN_STATE    1
#define FREESPACEBTN_STATE  2
#define DEFRAGBTN_STATE     3
#define STOPBTN_STATE       4
#define PAUSEBTN_STATE      5
#define RECCOMEND_STATE		6


//////////////////////////////////////////////////////////////////////////
// Main CADefragUIDoc class 
class CADefragUIDoc : public COleDocument
{
protected: // create from serialization only
    CADefragUIDoc();
    DECLARE_DYNCREATE(CADefragUIDoc)

    static const WORD JobInProgresss        = 32768;

    static const WORD JobNone               = 1;
    static const WORD JobAnalyze            = 2;
    static const WORD JobDefrag             = 4;
    static const WORD JobFreeSpace          = 8;

    static const int DrivrPicRadius         = 50;


public:
// Operations
    void SplitTriad( CString &Dest, ULARGE_INTEGER Src );
    void SplitTriad( CString &Dest, DWORD Src );
    void SplitTriad( CString &Dest, double Src, BYTE Precision = 0 );
    int Connect(HWND hwnd);
    int Disconnect(void);
    
    void AddAnalyzeJob(void);
    void AddDefragJob(int defragMode = DEFRAG_CMD_DEBUG1);
    int AddDefragJob( LPCTSTR path, int defragMode = DEFRAG_CMD_SIMPLE_DEFRAG );
    void AddFreSpaceJob(void);
    void PauseJob(void);
    void ResumeJob(void);
    void StobJob(void);
    void StobAllJobs();

    void Timer( void );
    void SetCurrentItem(int item);
    BOOL GetBtnState( int index );
    BOOL JobInProgress( int index );

    // For CDefragView
    DriveInfoMap& GetDisksInfo();
    // For CDriveMapView
    void GetDrvMap( PBYTE &Map, int &Size, WCHAR *drvName );

    BYTE GetLegendNum();
    int GetIndexByState( BlockStateNew State );
    BlockStateNew GetBlocksState( int Start, int End );
    void GetDrvMapStart( int Size, BYTE Legend );
    BlockStateNew GetDrvMapItem( int Index );

    CString GetDrvMapItemInfo( int Index );

	void SetMapBlockCount(int Count) {
		m_MapBlockCount = Count;
	}

	int GetMapBlockCount() {
		return m_MapBlockCount;
	}

	void SetClustersInBlock(int NumClustersInBlock) {
		m_ClustersInBlock = NumClustersInBlock;
	}

	int GetClustersInBlock() {
		return m_ClustersInBlock;
	}

    CString GetTmpDir();
    CString GetReportTmpDir();
    CString GetReportImgTmpDir();
    CLSID GetPicEncoder();
    CString GetCompName();
    CString GetOSVer();

    // For CStatisticView
    void GetStatInfo( CDiskStatus*& Status );
    int GetVolumeInfo( DFRG_VOLUME_INFO* VolumeInfoBefore, DFRG_VOLUME_INFO* VolumeInfoAfter );

    // Cluster Inspector
    int GetClusterInformation( IN int initialBlock, IN OUT int& count, DEFRAG_CINSPECTOR_INFO* info );
	int GetClusterInfo( IN ULONGLONG initialCluster, IN int count, DEFRAG_CINSPECTOR_INFO* info);

    // Settings management
    int GetSettingsValue( DWORD id, IN OUT LPVOID buffer, IN DWORD bufferSize );
    int SetSettingsValue( DWORD id, IN LPVOID buffer, IN DWORD bufferSize );
    int GetSchedulerTasks( CScheduleTaskList& tasks );
    int AddSchedulerTask( CScheduleTask& task );
    int SetSchedulerTask( CScheduleTask& task );
    int RunSchedulerTask( CScheduleTask& task );
    int RemoveSchedulerTask( CScheduleTask& task );
    static DWORD JobTypeToCommand( DWORD jobType );

	DWORD GetLastJobID() {
		return m_drvInfo[m_FCurrentItem].LastJobId;
	}

	int GetCurrentJobLevel() {
		return m_drvInfo[m_FCurrentItem].JobLevel;
	}

	int ClearSettingsValue(DWORD id);

    // Overrides
public:
    virtual BOOL OnNewDocument();
    virtual void Serialize(CArchive& ar);
    static CString GetCommandName( DWORD command );
    int UpdateDrivesInfo(void);

    // Implementation
public:
    virtual ~CADefragUIDoc();
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

    // Generated message map functions
protected:
    DECLARE_MESSAGE_MAP()

protected:
    int CommJob(DWORD *JobId, DWORD Command, PVOID InData);
    int DeleteJobIn(int item);
    int StopJobIn(int item);
    void UpdateMap(int item);
    CString GetErrorMes(int JobCode);

	int GetDriveHealth(int item);

    typedef std::vector<DWORD>  TActiveJobIDsVector;
    int GetActiveJobIDs( TActiveJobIDsVector& activeJobIDs );
    int UpdateJobStatus( int item, DWORD jobID );
    int StartNewJob( int item, DWORD Command );

protected:
    HWND            m_Fhwnd;
    DriveInfoMap    m_drvInfo;
    //int             m_drvCount;
    BOOL            m_FInit;
    int             m_FCurrentItem;
    BOOL            m_FInTimer;

    BYTE            m_CurrentLegend;
    double          m_DrvMapScale;    

    CString         m_CompName;
    CString         m_OSVer;
    CString         m_Temp;
    CString         m_ReportsTemp;
    CString         m_ReportsImgTemp;
    CLSID           m_PicEncoder;

	int				m_MapBlockCount;
	int				m_ClustersInBlock;
};


