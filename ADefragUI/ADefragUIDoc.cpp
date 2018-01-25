/*
    Defrag CADefragUIDoc class module 

    Module name:

        ADefragUIDoc.cpp

    Abstract:

        Contains implementation of the CADefragUIDoc class.

    $Header: /home/CVS_DEFRAG/Defrag/Src/ADefragUI/ADefragUIDoc.cpp,v 1.17 2010/01/12 22:53:18 roman Exp $
    $Log: ADefragUIDoc.cpp,v $
    Revision 1.17  2010/01/12 22:53:18  roman
    Add "Recommended Defrag Method..." button

    Revision 1.16  2009/12/29 21:59:39  roman
    Show MFT and MFTZone in performance view

    Revision 1.15  2009/12/28 23:19:06  roman
    exclude unformatted hard disks from DriveList

    Revision 1.14  2009/12/28 21:35:50  roman
    stop jobs at exit

    Revision 1.13  2009/12/24 01:39:10  roman
    fix errors

    Revision 1.12  2009/12/21 23:28:08  roman
    Excluded file list impl.

    Revision 1.11  2009/12/20 18:56:25  roman
    minor changes

    Revision 1.10  2009/12/18 00:16:19  roman
    CInspector multiple fragments view, Exclude list remove operations.

    Revision 1.9  2009/12/15 18:50:46  roman
    Fix bug with start/end cluster calculation from block number for CInspector.

    Revision 1.8  2009/12/07 19:57:06  roman
    ClusterInspector implementation.

    Revision 1.7  2009/12/03 15:36:55  dimas
    CVS headers included

*/


#include "stdafx.h"
#include "ADefragUI.h"

#include "ADefragUIDoc.h"
#include "MainFrm.h"
#include "DefragView.h"
#include "CntrItem.h"
#include "SrvStub.h"

#include "DefragView.h"
#include <math.h>
#include <atlpath.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CADefragUIDoc

IMPLEMENT_DYNCREATE(CADefragUIDoc, COleDocument)

BEGIN_MESSAGE_MAP(CADefragUIDoc, COleDocument)
    // Enable default OLE container implementation
    ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, &COleDocument::OnUpdatePasteMenu)
    ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE_LINK, &COleDocument::OnUpdatePasteLinkMenu)
    ON_UPDATE_COMMAND_UI(ID_OLE_EDIT_CONVERT, &COleDocument::OnUpdateObjectVerbMenu)
    ON_COMMAND(ID_OLE_EDIT_CONVERT, &COleDocument::OnEditConvert)
    ON_UPDATE_COMMAND_UI(ID_OLE_EDIT_LINKS, &COleDocument::OnUpdateEditLinksMenu)
    ON_COMMAND(ID_OLE_EDIT_LINKS, &COleDocument::OnEditLinks)
    ON_UPDATE_COMMAND_UI_RANGE(ID_OLE_VERB_FIRST, ID_OLE_VERB_LAST, &COleDocument::OnUpdateObjectVerbMenu)
    ON_COMMAND(ID_FILE_SEND_MAIL, &CADefragUIDoc::OnFileSendMail)
    ON_UPDATE_COMMAND_UI(ID_FILE_SEND_MAIL, &CADefragUIDoc::OnUpdateFileSendMail)
END_MESSAGE_MAP()


// CADefragUIDoc construction/destruction
#define VER_SUITE_WH_SERVER     0x00008000

#define GUI_TMPDIR          1   // Not define - <Drive>:\Documents and Settings\<User_name>\Local Settings\Temp\ADefragUI
                                //     Define - ADefragUI exe path

CADefragUIDoc::CADefragUIDoc()
{
    // Use OLE compound files
    EnableCompoundFile();

    // TODO: add one-time construction code here

    Global.AppId          = GuiAppId;
    m_FInit               = FALSE;
    m_drvInfo.clear();
    m_FCurrentItem        = 0;
    m_FInTimer            = FALSE;    

    //
    // Computer name
    //
    TCHAR szCompName[ 100 ];
    DWORD n;
    GetComputerName( szCompName, &n );
    m_CompName =  szCompName;

    //
    // OS version
    //
    CString OS;
    OSVERSIONINFOEXW osvi;
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    GetVersionEx( (LPOSVERSIONINFO)&osvi );
    SYSTEM_INFO sysi;
    GetSystemInfo( &sysi );

    DWORD ver = osvi.dwMajorVersion * 10 + osvi.dwMinorVersion;
    switch ( ver )
    {
    case 50 :
        OS.Append( _T("Windows 2000") );
        break;

    case 51 :
        OS.Append( _T("Windows XP") );
        break;

    case 52 :
        if ( osvi.wSuiteMask == VER_SUITE_WH_SERVER )
            OS.Append( _T("Windows Home Server") );
        else
            if ( ( osvi.wProductType == VER_NT_WORKSTATION ) && ( sysi.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64 ) )
                OS.Append( _T("Windows XP Professional x64 Edition") );
            else
                OS.Append( _T("Windows Server 2003") );
        break;

    case 60 :
        if ( osvi.wProductType == VER_NT_WORKSTATION )
            OS.Append( _T("Windows Vista") );
        else
            OS.Append( _T("Windows Server 2008") );
        break;

    case 61 :
        if ( osvi.wProductType == VER_NT_WORKSTATION )
            OS.Append( _T("Windows 7") );
        else
            OS.Append( _T("Windows Server 2008 R2") );
        break;

    }
    CString Str;
    Str.Format( _T(" (Build %i) "), osvi.dwBuildNumber );
    OS.Append( Str );
    OS.Append( osvi.szCSDVersion );

    m_OSVer = OS;

    //
    // Temporary files catalog
    //    
#ifdef GUI_TMPDIR
    HINSTANCE hInstance = AfxGetResourceHandle();
    ASSERT( hInstance != NULL );

    GetModuleFileName( hInstance, m_Temp.GetBuffer(_MAX_PATH), _MAX_PATH );
    m_Temp.ReleaseBuffer();
    if( m_Temp.ReverseFind( _T('\\') ) > 0 )
    {
        m_Temp.Delete( m_Temp.ReverseFind( _T('\\') ), m_Temp.GetLength() );
    }
#else
    TCHAR szTemp[ _MAX_PATH ];
    GetEnvironmentVariable( _T("TEMP"), szTemp, _MAX_PATH );

    m_Temp = szTemp;
    m_Temp.Append( _T("\\") );
    m_Temp.Append( ADEFRAG_TMP_SUBDIR );
    if ( !ATLPath::FileExists( m_Temp ) )
        CreateDirectory( m_Temp, NULL );
#endif

    m_ReportsTemp = m_Temp;
    m_ReportsTemp.Append( _T("\\") );
    m_ReportsTemp.Append( REPORTS_SUBDIR );
    if ( !ATLPath::FileExists( m_ReportsTemp ) )
        CreateDirectory( m_ReportsTemp, NULL );

    m_ReportsImgTemp = m_ReportsTemp;
    m_ReportsImgTemp.Append( _T("\\") );
    m_ReportsImgTemp.Append( IMG_SUBDIR );
    if ( !ATLPath::FileExists( m_ReportsImgTemp ) )
        CreateDirectory( m_ReportsImgTemp, NULL );


}

CADefragUIDoc::~CADefragUIDoc()
{
    //StobAllJobs();
    m_FInit = FALSE;
    m_drvInfo.clear();
    Disconnect();
    unloadLibrary();
}

BOOL CADefragUIDoc::OnNewDocument()
{
    //
    // PicEncoder
    //
    UINT num = 0, size = 0;

    GetImageEncodersSize( &num, &size );
    ImageCodecInfo *pArray = (ImageCodecInfo*)(malloc( size ));

    GetImageEncoders( num, size, pArray );
    for( UINT j = 0; j < num; ++j )
    { 
        if ( pArray[j].FormatID == ImageFormatBMP )
        {
            m_PicEncoder = pArray[j].Clsid;
            break;
        }
    }

    free( pArray );
    if (!COleDocument::OnNewDocument())
        return FALSE;

    return TRUE;
}

// CADefragUIDoc serialization
void CADefragUIDoc::Serialize(CArchive& ar)
{
    if (ar.IsStoring())
    {
        // TODO: add storing code here
    }
    else
    {
        // TODO: add loading code here
    }

    // Calling the base class COleDocument enables serialization
    //  of the container document's COleClientItem objects.
    COleDocument::Serialize(ar);
}


// CADefragUIDoc diagnostics

#ifdef _DEBUG
void CADefragUIDoc::AssertValid() const
{
    COleDocument::AssertValid();
}

void CADefragUIDoc::Dump(CDumpContext& dc) const
{
    COleDocument::Dump(dc);
}
#endif //_DEBUG

//
// Send SIMPLE Job command (without data blocks transfer) to service
//
int
CADefragUIDoc::CommJob(
    DWORD *JobId,
    DWORD Command,
    PVOID InData)
{
    //DWORD       JobStatus = 0, JobProgress = 0;
    int         Result = NO_ERROR;

    if( !JobId ) 
    {
        return ERROR_INVALID_PARAMETER;
    }

    if( ((*JobId == INVALID_JOB_ID) || (*JobId == EMPTY_JOB_ID)) && 
        Command != DEFRAG_CMD_INIT_JOB) 
    {
        return ERROR_INVALID_PARAMETER;
    }

    if( InData && Command != DEFRAG_CMD_INIT_JOB ) 
    {
        return ERROR_INVALID_PARAMETER;
    }

    //Result = SendCommand( JobId, Command, InData, &JobStatus, &JobProgress );
    Result = SendCommandEx( JobId, Command, InData, NULL, NULL, NULL, NULL, NULL, NULL );

    return Result;
}

//
// Split arge number by tiad into the string
//
void CADefragUIDoc::SplitTriad( CString &Dest, ULARGE_INTEGER Src )
{
    CString Str;
	wchar_t coef = '\0';
	if (Src.QuadPart > (1024 * 1024)) {
		Src.QuadPart = Src.QuadPart / (1024 * 1024);
		coef = 'M';

		if (Src.QuadPart > 1024) {
			Src.QuadPart = Src.QuadPart / 1024;
			coef = 'G';
		}
	}

    Str.Format( _T("%I64d"), ( Src.QuadPart ) );

    for ( int i = Str.GetLength() - 3; i > 0; i = i - 3 )
        Str.Insert ( i, ',' );

    Dest.Append( Str );

	if (coef != '\0') {
		Dest.AppendFormat(_T(" %cB"), (coef));
	}
}

void CADefragUIDoc::SplitTriad( CString &Dest, DWORD Src )
{
    CString Str;
    Str.Format( _T("%i"), ( Src ) );

    for ( int i = Str.GetLength() - 3; i > 0; i = i - 3 )
        Str.Insert ( i, ',' );

    Dest.Append( Str );
}

void CADefragUIDoc::SplitTriad( CString &Dest, double Src, BYTE Precision )
{
    CString Str, FmtStr;
    FmtStr.Format( _T("%%.%if"), Precision );
    Str.Format( FmtStr, Src );

    int StartPos = Str.GetLength();

    for ( int i = Str.GetLength(); i > 0; i-- )
        if ( Str[ i - 1 ] == _T('.') )
            StartPos = i - 1;

    for ( int i = StartPos - 3; i > 0; i = i - 3 )
        Str.Insert ( i, ',' );

    Dest.Append( Str );
}

int
StartSrv( 
    IN SC_HANDLE SCMgrHandle, 
    IN LPCTSTR ServiceName )
{
    int               err = NO_ERROR;
    SC_HANDLE         SrvHandle = NULL;
    TCHAR             *Parameters[] = { L"Service" };


    SrvHandle = OpenService( 
                    SCMgrHandle, 
                    ServiceName, 
                    SERVICE_ALL_ACCESS );
    if ( SrvHandle == NULL ) // || SrvHandle == INVALID_HANDLE_VALUE ) 
    {
        OutMessage( L"StartSrv OpenService() error" );
        return -2;
    }

    if ( !StartService(
                SrvHandle, 
                sizeof(Parameters)/sizeof(TCHAR*), 
                (LPCWSTR*)Parameters ) ) 
    {
        OutMessage( L"StartSrv StartService() error" );
        err = -3;
    }

    if ( SrvHandle ) 
    {
        CloseServiceHandle( SrvHandle );
        SrvHandle = NULL;
    }

    return err;

} // end of StartSrv


int
StopSrv( 
    IN SC_HANDLE SCMgrHandle, 
    IN LPCTSTR ServiceName )
{
    int               err = NO_ERROR;
    SC_HANDLE         SrvHandle = NULL;
    SERVICE_STATUS    serviceStatus;


    SrvHandle = OpenService( 
                    SCMgrHandle, 
                    ServiceName, 
                    SERVICE_ALL_ACCESS );
    if ( SrvHandle == NULL ) // || SrvHandle == INVALID_HANDLE_VALUE ) 
    {
        OutMessage( L"StopSrv OpenService() error" );
        return -2;
    }

    if ( !ControlService( 
                SrvHandle, 
                SERVICE_CONTROL_STOP, 
                &serviceStatus ) ) 
    {
        OutMessage( L"StopSrv ControlService() error" );
        err = -3;
    }

    if ( SrvHandle ) 
    {
        CloseServiceHandle( SrvHandle );
        SrvHandle = NULL;
    }

    return err;

} // end of StopSrv


int
SelfStartService( 
    void )
{
    int            err = NO_ERROR;
    SC_HANDLE      SCMgrHandle = NULL;

    //
    // Open Service Control Manager
    //
    SCMgrHandle = OpenSCManager( 
                            NULL,                     // machine (NULL == local)
                            NULL,                     // database (NULL == default)
                            SC_MANAGER_ALL_ACCESS );  // access required

    if ( SCMgrHandle == NULL ) 
    {
        OutMessage( L"SelfStart error" );
        return -1;
    }

    //
    // Restart
    //
    err = StopSrv( SCMgrHandle, SERVICE_NAME );
    if ( !err )
    {
        Sleep( 5000 );
    }

    err = StartSrv( SCMgrHandle, SERVICE_NAME );

    if ( SCMgrHandle ) 
    {
        CloseServiceHandle( SCMgrHandle );
        SCMgrHandle = NULL;
    }

    return err;

} // end of SelfStartService


#define NUM_TRIES	5
//
//
//
int CADefragUIDoc::Connect(HWND hwnd)
{
    m_Fhwnd = hwnd;
	int tryCount = 0, Result;
    //
    // Open connection
    //
    if(loadLibrary()) 
    {
        DislpayErrorMessage( _T("Can't load connection library"), 0, ERR_OS, hwnd );
        return 1;
    }
    //
    // Open connection
    //
	while ((Result = OpenConnection()) != NO_ERROR) {
		if ((Result = SelfStartService()) == NO_ERROR) {
			Sleep(1000);
			continue;
		}

		Sleep(1000);

		if (tryCount++ >= NUM_TRIES)
			break;
	}

    if ( NO_ERROR != Result )
    {
        //MessageBox( hwnd, _T("Open Service Connection error!"), _T(""), MB_OK );
        AfxMessageBox( IDS_SERVICE_CONNECTION_ERROR, MB_OK | MB_ICONEXCLAMATION );
        Result = DislpayErrorMessage( _T("Open Service Connection"), Result, ERR_OS, NULL );
        PostQuitMessage(-1);
        return 1;
    }

    UpdateDrivesInfo();

    m_FInit = TRUE;
    return 0;
}

int CADefragUIDoc::UpdateDrivesInfo(void)
{
    DWORD drvMap = GetLogicalDrives();

    int k = 0, j = 0; // Drive's letter counter
    WCHAR strItem[DSKNAMELENGTH]; //, strDrvRoot, strDrvType;
    const int MaxDiskCount      = 30;
    const DWORD MaxDiskMask     = ( 1 << MaxDiskCount );

    int prevPos = 0;

    for ( DWORD i = 1; i < MaxDiskMask; i = i * 2, ++j )
    {
        if( i != (drvMap & i) ) continue;
        CDriveInfo driveInfo;
        BOOL isReadOnlyDrive = FALSE;
        //
        // Check drive existens
        //
        CString strDrvType;
        wsprintf(strItem, _T("%C:"), 'A' + j );
        switch ( GetDriveType(strItem) ) 
        {
        case DRIVE_REMOVABLE :
            strDrvType.LoadString( IDS_DRIVE_REMOVABLE );
            break;

        case DRIVE_FIXED :
            strDrvType.LoadString( IDS_DRIVE_FIXED );
            break;

        case DRIVE_REMOTE :
            strDrvType.LoadString( IDS_DRIVE_REMOTE );
            break;

        case DRIVE_NO_ROOT_DIR :
            strDrvType.LoadString( IDS_DRIVE_NO_ROOT_DIR );
            continue;
            break;

        case DRIVE_CDROM :
            strDrvType.LoadString( IDS_DRIVE_CDROM );
            isReadOnlyDrive = TRUE;
            continue;
            break;

        case DRIVE_RAMDISK :
            strDrvType.LoadString( IDS_DRIVE_RAMDISK );
            break;

        case DRIVE_UNKNOWN : // skip
        default:
            continue;

        }

        driveInfo.name = strItem;

        //
        //
        //
        ULARGE_INTEGER bytesFreeCaller, bytesTotal, bytesTotalFree;

        if ( GetDiskFreeSpaceEx ( strItem, &bytesFreeCaller, &bytesTotal, &bytesTotalFree ) )
        {
            //FormatDiskSize ( m_drvInfo[m_drvCount].diskStatus.Size, bytesTotal);
            SplitTriad( driveInfo.diskStatus.Size, bytesTotal);
            driveInfo.diskStatus.nSize = bytesTotal;
            //FormatDiskSize ( m_drvInfo[m_drvCount].diskStatus.Free, bytesTotalFree );
            SplitTriad( driveInfo.diskStatus.Free, bytesTotalFree );
            driveInfo.diskStatus.nFree = bytesTotalFree;
            driveInfo.diskStatus.IsInvalid = FALSE;
        }
        else 
        {
            driveInfo.diskStatus.Free;
            driveInfo.diskStatus.Size;
            driveInfo.diskStatus.IsInvalid = TRUE;
        }
        driveInfo.diskStatus.Type = strDrvType;

        CString RootPathName; 
        RootPathName.Format( _T("%s\\"), strItem );

        DWORD MaximumComponentLength, FileSystemFlags;
        ::GetVolumeInformation(
            RootPathName,
            driveInfo.diskStatus.Label.GetBuffer(MAX_PATH+1), MAX_PATH+1, // Label
            &(driveInfo.diskStatus.SerialNumber),
            &MaximumComponentLength,
            &FileSystemFlags,
            driveInfo.diskStatus.FileSystem.GetBuffer(MAX_PATH+1), MAX_PATH+1 // File system
            );

		if (!driveInfo.diskStatus.SerialNumber) {
			// exclude unformatted hard disks
			continue;
		}

        driveInfo.diskStatus.Label.ReleaseBuffer();
        driveInfo.diskStatus.FileSystem.ReleaseBuffer();

        //DWORD TotalNumberOfClusters, SectorsPerCluster, BytesPerSector;
        GetDiskFreeSpace(
            RootPathName,   // address of root path 
            &(driveInfo.diskStatus.SectorsPerCluster),  // address of sectors per cluster 
            &(driveInfo.diskStatus.BytesPerSector), // address of bytes per sector 
            &(driveInfo.diskStatus.NumberOfFreeClusters),   // address of number of free clusters  
            &(driveInfo.diskStatus.TotalNumberOfClusters)   // address of total number of clusters  
            );
        isReadOnlyDrive = (FILE_READ_ONLY_VOLUME & FileSystemFlags) != 0;
        driveInfo.diskStatus.FileSystemFlags = FileSystemFlags;
        BOOL isSupportedSustem = driveInfo.diskStatus.FileSystem.Find( _T("FAT") ) >= 0
            || driveInfo.diskStatus.FileSystem.Find( _T("NTFS") ) >= 0;

        driveInfo.JobLevel      = 0;
        driveInfo.JobId         = INVALID_JOB_ID;
        driveInfo.JobStackSize  = 0;
        driveInfo.Map           = NULL;
        driveInfo.MapSize       = 0;
        driveInfo.IsExternal    = FALSE;
        driveInfo.FAnalyzeBtnState    = isSupportedSustem;
        driveInfo.FFreespaceBtnState  = isSupportedSustem && !isReadOnlyDrive;
        driveInfo.FDefragBtnState     = isSupportedSustem && !isReadOnlyDrive;
		driveInfo.FRecommendBtnState  = FALSE;
        driveInfo.FStopBtnState       = FALSE;
        driveInfo.FPauseBtnState      = FALSE;
        driveInfo.LastDefragTime      = theApp.GetInt( driveInfo.name + LAST_DEFRAG_TIME, 0 );
        ZeroMemory( &driveInfo.VolumeInfoBefore, sizeof(driveInfo.VolumeInfoBefore) );
        ZeroMemory( &driveInfo.VolumeInfoAfter, sizeof(driveInfo.VolumeInfoAfter) );

        //
        // TODO: Exclude m_drvInfo[].name
        //
        //wcscpy(m_drvInfo[m_drvCount].name, strItem);
        driveInfo.diskStatus.Name = strItem;
        GetVolumePathName( strItem, driveInfo.diskStatus.MountPoint.GetBuffer(MAX_PATH), MAX_PATH );
        driveInfo.diskStatus.MountPoint.ReleaseBuffer();

        try
        {
            ASSERT( prevPos <= (int)m_drvInfo.size() );
            if( prevPos == (int)m_drvInfo.size() )
            {
                m_drvInfo.push_back( driveInfo );
                prevPos++;
            }
            else
            {
                while( prevPos < (int)m_drvInfo.size() && m_drvInfo[prevPos].name < driveInfo.name )
                {
                    m_drvInfo[prevPos].JobStackSize = 0;
                    if( m_drvInfo[prevPos].JobLevel > 0 )
                    {
                        StopJobIn( prevPos );
                    }
                }
                if( prevPos < (int)m_drvInfo.size() && m_drvInfo[prevPos].name == driveInfo.name )
                { // Only update some fields
                    DWORD tmp = m_drvInfo[prevPos].diskStatus.Progress;
                    m_drvInfo[prevPos].diskStatus = driveInfo.diskStatus;
                    m_drvInfo[prevPos].diskStatus.Progress = tmp;
                    prevPos++;
                }
                else
                {
                    m_drvInfo.insert( m_drvInfo.begin() + prevPos, driveInfo );
                    prevPos++;
                }
            }
        }
        catch(...)
        {
            DislpayErrorMessage( _T("Memory allocation error"), 0, ERR_OS, NULL );
        }
    } // End of enume drive

    // clear last unused drive info
    for( int i = prevPos; i < (int)m_drvInfo.size(); ++i )
    {
        m_drvInfo[i].JobStackSize = 0;
        if( m_drvInfo[i].JobLevel > 0 )
        {
            StopJobIn( i );
        }
    }
    if( m_FCurrentItem >= prevPos )
    {
        SetCurrentItem( prevPos-1 );
    }
    m_drvInfo.resize( prevPos );
    return 0;
}

//
//
//
int CADefragUIDoc::Disconnect(void)
{
    if ( !m_FInit ) return 0;

    CloseConnection();
    return 0;
}

//
//
//
void CADefragUIDoc::AddAnalyzeJob(void)
{
    if ( !m_FInit ) return;

    switch(m_drvInfo[m_FCurrentItem].JobLevel) 
    {
    case 0 :
        m_drvInfo[m_FCurrentItem].JobStackSize = 2;
        m_drvInfo[m_FCurrentItem].JobStack[1] = DEFRAG_CMD_INIT_JOB;
        m_drvInfo[m_FCurrentItem].JobStack[0] = DEFRAG_CMD_ANALIZE_VOLUME;
        break;

    case 1 :
        m_drvInfo[m_FCurrentItem].JobStackSize = 1;
        m_drvInfo[m_FCurrentItem].JobStack[0] = DEFRAG_CMD_ANALIZE_VOLUME;
        break;

    default :
        StopJobIn(m_FCurrentItem);
        m_drvInfo[m_FCurrentItem].JobStackSize = 2;
        m_drvInfo[m_FCurrentItem].JobStack[1] = DEFRAG_CMD_INIT_JOB;
        m_drvInfo[m_FCurrentItem].JobStack[0] = DEFRAG_CMD_ANALIZE_VOLUME;
    }

    m_drvInfo[m_FCurrentItem].FAnalyzeBtnState     = FALSE;
    m_drvInfo[m_FCurrentItem].FFreespaceBtnState   = FALSE;
    m_drvInfo[m_FCurrentItem].FDefragBtnState      = FALSE;
	m_drvInfo[m_FCurrentItem].FRecommendBtnState   = FALSE;
    m_drvInfo[m_FCurrentItem].FStopBtnState        = TRUE;
    m_drvInfo[m_FCurrentItem].FPauseBtnState       = FALSE;
}

//
//
//
void CADefragUIDoc::AddDefragJob(int defragMode)
{
    if ( !m_FInit ) return;

    switch(m_drvInfo[m_FCurrentItem].JobLevel) {
    case 0 :
        m_drvInfo[m_FCurrentItem].JobStackSize = 3;
        m_drvInfo[m_FCurrentItem].JobStack[2] = DEFRAG_CMD_INIT_JOB;
        m_drvInfo[m_FCurrentItem].JobStack[1] = DEFRAG_CMD_ANALIZE_VOLUME;
        m_drvInfo[m_FCurrentItem].JobStack[0] = defragMode;
        //m_drvInfo[m_FCurrentItem].JobStack[0] = DEFRAG_CMD_SIMPLE_DEFRAG;
        break;

    case 1 :
        m_drvInfo[m_FCurrentItem].JobStackSize = 2;
        m_drvInfo[m_FCurrentItem].JobStack[1] = DEFRAG_CMD_ANALIZE_VOLUME;
        m_drvInfo[m_FCurrentItem].JobStack[0] = defragMode;
        //m_drvInfo[m_FCurrentItem].JobStack[0] = DEFRAG_CMD_SIMPLE_DEFRAG;
        break;

    case 2 :
        m_drvInfo[m_FCurrentItem].JobStackSize = 1;
        m_drvInfo[m_FCurrentItem].JobStack[0] = defragMode;
        //m_drvInfo[m_FCurrentItem].JobStack[0] = DEFRAG_CMD_SIMPLE_DEFRAG;
        break;
    
    default :
        StopJobIn(m_FCurrentItem);
        m_drvInfo[m_FCurrentItem].JobStackSize = 3;
        m_drvInfo[m_FCurrentItem].JobStack[2] = DEFRAG_CMD_INIT_JOB;
        m_drvInfo[m_FCurrentItem].JobStack[1] = DEFRAG_CMD_ANALIZE_VOLUME;
        m_drvInfo[m_FCurrentItem].JobStack[0] = defragMode;
        //m_drvInfo[m_FCurrentItem].JobStack[0] = DEFRAG_CMD_SIMPLE_DEFRAG;
    }

    m_drvInfo[m_FCurrentItem].FAnalyzeBtnState     = FALSE;
    m_drvInfo[m_FCurrentItem].FFreespaceBtnState   = FALSE;
    m_drvInfo[m_FCurrentItem].FDefragBtnState      = FALSE;
	m_drvInfo[m_FCurrentItem].FRecommendBtnState   = FALSE;
    m_drvInfo[m_FCurrentItem].FStopBtnState        = TRUE;
    m_drvInfo[m_FCurrentItem].FPauseBtnState       = FALSE;
}

int CADefragUIDoc::AddDefragJob( LPCTSTR path, int defragMode )
{
    if ( !m_FInit ) return ERROR_ACCESS_DENIED;
    int res = ERROR_INVALID_PARAMETER;
    int selectedDisk = m_FCurrentItem;
    CString pathToDefrag( path );
    for( int i = 0; i < (int)m_drvInfo.size(); ++ i )
    {
        // find the correspond disk
        if( pathToDefrag.Find( m_drvInfo[i].name ) >= 0 )
        {
            m_FCurrentItem = i;
            // TODO: add supporting "Include List"
            AddDefragJob( defragMode );
            res = NO_ERROR;
        }
    }
    m_FCurrentItem = selectedDisk;
    return res;
}

//
//
//
void CADefragUIDoc::AddFreSpaceJob(void)
{
    AddDefragJob( DEFRAG_CMD_FREE_SPACE );
}

//
//
//
void CADefragUIDoc::PauseJob(void)
{
    m_drvInfo[m_FCurrentItem].FPauseBtnState = TRUE;
    CommJob(&m_drvInfo[m_FCurrentItem].JobId, DEFRAG_CMD_PAUSE_JOB, NULL);
}

//
//
//
void CADefragUIDoc::ResumeJob(void)
{
    m_drvInfo[m_FCurrentItem].FPauseBtnState = FALSE;
    CommJob(&m_drvInfo[m_FCurrentItem].JobId, DEFRAG_CMD_RESUME_JOB, NULL);
}

//
//
//
void CADefragUIDoc::StobJob(void)
{
    if ( (int)m_drvInfo.size() <= m_FCurrentItem ) return;
    m_drvInfo[m_FCurrentItem].JobStackSize = 0;
    StopJobIn(m_FCurrentItem);
}

void CADefragUIDoc::StobAllJobs()
{
    for( int i = 0; i < (int)m_drvInfo.size(); ++i )
    {
        m_drvInfo[i].JobStackSize = 0;
        if( m_drvInfo[i].JobLevel > 0 )
        {
            StopJobIn( i );
            DeleteJobIn( i );
        }
    }
}

int CADefragUIDoc::DeleteJobIn(int item)
{
    DWORD jobId = INVALID_JOB_ID;
    if( m_drvInfo[item].LastJobId != INVALID_JOB_ID )
    {
        jobId = m_drvInfo[item].LastJobId;
    }
    m_drvInfo[item].LastJobId = m_drvInfo[item].JobId;
    m_drvInfo[item].JobId = INVALID_JOB_ID;;
    if( jobId == INVALID_JOB_ID ) return 0;
    int Result = CommJob(&jobId, DEFRAG_CMD_DELETE_JOB, NULL);

    DWORD JobStatus = DEFRAG_JOB_FLAG_INPROGRESS;
    DWORD JobProgress;
    int iteration = 0;
    while( NO_ERROR == Result && ((JobStatus) & DEFRAG_JOB_FLAG_INPROGRESS) && ++iteration < 500 ){
        //Result = SendCommand( &jobId, DEFRAG_CMD_GET_JOB_STATUS, NULL, &JobStatus, &JobProgress );
        Result = SendCommandEx( &jobId, DEFRAG_CMD_GET_JOB_STATUS, NULL, NULL, NULL, NULL, NULL, &JobStatus, &JobProgress );
        ::Sleep(10);
    }
    return Result;
}

//
//
//
int CADefragUIDoc::StopJobIn(int item)
{
    if ( !m_FInit ) return 0;
    if ( (int)m_drvInfo.size() <= item ) return 1;

    int         Result;
    DWORD       JobStatus, JobProgress;

    //
    // Get Volume info
    //
    //Result = SendCommand( &m_drvInfo[item].JobId, DEFRAG_CMD_GET_STATISTIC, NULL, &JobStatus, &JobProgress );
    //if( Result == NO_ERROR )
    //{
    //    m_drvInfo[item].VolumeInfoAfter = *((PDFRG_VOLUME_INFO)Global.ServerBuffer);
    //}

    DWORD   OutputLength = sizeof( PDFRG_VOLUME_INFO );
    Result = SendCommandEx( 
                &m_drvInfo[item].JobId, 
                DEFRAG_CMD_GET_STATISTIC, 
                NULL, 
                NULL, 
                NULL, 
                &(m_drvInfo[item].VolumeInfoAfter), 
                &OutputLength,
                &JobStatus, 
                &JobProgress );

    if( m_drvInfo[item].JobLevel > 0 )
    {
        Result = CommJob(&m_drvInfo[item].JobId, DEFRAG_CMD_STOP_JOB, NULL);

        JobStatus = DEFRAG_JOB_FLAG_INPROGRESS;
        int iteration = 0;
        while( NO_ERROR == Result && ((JobStatus) & DEFRAG_JOB_FLAG_INPROGRESS) && ++iteration < 500 ){
            //Result = SendCommand( &m_drvInfo[item].JobId, DEFRAG_CMD_GET_JOB_STATUS, NULL, &JobStatus, &JobProgress );
            Result = SendCommandEx( &m_drvInfo[item].JobId, DEFRAG_CMD_GET_JOB_STATUS, NULL, NULL, NULL, NULL, NULL, &JobStatus, &JobProgress );
            ::Sleep(10);
        }

        Result = DeleteJobIn( item );
    }

    m_drvInfo[item].CurrentJobCommand = 0;
    m_drvInfo[item].JobLevel = 0;
    m_drvInfo[item].JobId = INVALID_JOB_ID;
    m_drvInfo[item].IsExternal = FALSE;

    m_drvInfo[item].FAnalyzeBtnState     = TRUE;
    m_drvInfo[item].FFreespaceBtnState   = TRUE;
    m_drvInfo[item].FDefragBtnState      = TRUE;
	m_drvInfo[item].FRecommendBtnState	 = GetDriveHealth(item) != DriveHealth::e_LevelGood;;
    m_drvInfo[item].FStopBtnState        = FALSE;
    m_drvInfo[item].FPauseBtnState       = FALSE;

    UpdateAllViews(NULL);

    return 0;
}

//
//
//
CString CADefragUIDoc::GetErrorMes(int JobCode)
{
    CString str( GetCommandName( JobCode ) );
    if( str.GetLength() == 0 ) 
    {
        str.Format(_T("Command #%d"), JobCode );
    }
    return str;
}

//
//
//
void CADefragUIDoc::UpdateMap(int item)
{
    // Copy & repaint bytemap
    if ( !m_drvInfo[item].Map )
    {
        //
        // Allocate Map buffer.
        // TODO: free it before exit !!!
        //
        m_drvInfo[item].Map = (PBYTE) malloc( Global.BufferLength );
        if ( !m_drvInfo[item].Map )
        {
            DislpayErrorMessage( _T("Drive map memory allocation error!"), 0, ERR_OS, NULL );
            return;
        }
        ZeroMemory( m_drvInfo[item].Map, Global.BufferLength );
    }

    //
    // Copy only !!!
    //
    CopyMemory( m_drvInfo[item].Map, Global.ServerBuffer, Global.BufferLength );
    m_drvInfo[item].MapSize   = Global.BufferLength;

    if ( item == m_FCurrentItem )
    {
        //UpdateAllViews(NULL);
        // Update only Drive map view
    }
}

//
//
//
void CADefragUIDoc::SetCurrentItem(int item)
{
    if ( !m_FInit ) return;
    if ( (item < 0) || (item >= (int)m_drvInfo.size()) ) return;
    if ( ( m_FCurrentItem == item) || ( m_FCurrentItem == -1 ) ) return;

    m_FCurrentItem = item;
    UpdateAllViews(NULL);

    if( m_drvInfo[m_FCurrentItem].IsAnalysed )
    {
        DriveHealth health( m_drvInfo[m_FCurrentItem].diskStatus, m_drvInfo[m_FCurrentItem].VolumeInfoAfter );
        GetMainFrm()->UpdateTaskpaneResultAndReliability( &health, FALSE );
    }
    else
    {
        GetMainFrm()->UpdateTaskpaneResultAndReliability( NULL, FALSE );
    }
}

/*
int CADefragUIDoc::GetActiveJobIDs( TActiveJobIDsVector& activeJobIDs )
{
    DWORD JobId = GLOBAL_JOB_ID;
    DWORD       JobStatus = 0, JobProgress = 0;

    int Result = SendCommand( &JobId, DEFRAG_CMD_GET_JOB_LIST, NULL, &JobStatus, &JobProgress );
    if( Result != ERROR_SUCCESS )
    {
        //??DislpayErrorMessage(GetErrorMes(DEFRAG_CMD_GET_JOB_LIST), Result, ERR_INTERNAL, m_Fhwnd );
        return Result;
    }
    activeJobIDs.clear();
    PDWORD ActiveJobIDs = (PDWORD)Global.ServerBuffer;
    for( DWORD i = 0; ActiveJobIDs[i] != INVALID_JOB_ID && ActiveJobIDs[i] != 0; ++i )
    {
        activeJobIDs.push_back( ActiveJobIDs[i] );
    }
    return Result;
}
*/

int CADefragUIDoc::GetActiveJobIDs( TActiveJobIDsVector& activeJobIDs )
{
    DWORD   JobId = GLOBAL_JOB_ID;
    ULONG   DataLength = 0;
    int     Result;

    Result = SendCommandEx( &JobId, DEFRAG_CMD_GET_JOB_LIST, NULL, NULL, NULL, NULL, &DataLength, NULL, NULL );
    if( Result != ERROR_SUCCESS )
    {
        //??DislpayErrorMessage(GetErrorMes(DEFRAG_CMD_GET_JOB_LIST), Result, ERR_INTERNAL, m_Fhwnd );
        return Result;
    }

    activeJobIDs.clear();

    PDWORD ActiveJobIDs = (PDWORD)malloc(DataLength);

    if ( ActiveJobIDs )
    {
        Result = SendCommandEx( 
                    &JobId, 
                    DEFRAG_CMD_GET_JOB_LIST, 
                    NULL, 
                    NULL, NULL, 
                    ActiveJobIDs, &DataLength, 
                    NULL, NULL );
        if( Result == ERROR_SUCCESS )
        {
            for( DWORD i = 0; i < DataLength / sizeof(DWORD) && ActiveJobIDs[i] != INVALID_JOB_ID && ActiveJobIDs[i] != 0; ++i )
            {
                activeJobIDs.push_back( ActiveJobIDs[i] );
            }
        }
        free( ActiveJobIDs );
    }
    else
    {
        Result = ERROR_NOT_ENOUGH_MEMORY;
    }

    return Result;
}

int CADefragUIDoc::UpdateJobStatus( int item, DWORD jobID )
{
    if ( !m_FInit ) return ERROR_NOT_READY;
    if ( (item < 0) || (item >= (int)m_drvInfo.size()) ) return ERROR_INVALID_PARAMETER;
    int Result = 0;

    if( ( m_drvInfo[item].JobId == INVALID_JOB_ID) || (m_drvInfo[item].JobId == 0) || (m_drvInfo[item].JobId != jobID) )
    {
        return ERROR_INVALID_PARAMETER;
    }

    DWORD JobStatus = 0, JobProgress = 0;
    //Result = SendCommand( &m_drvInfo[item].JobId, DEFRAG_CMD_GET_JOB_STATUS, NULL, &JobStatus, &JobProgress );
    Result = SendCommandEx( &m_drvInfo[item].JobId, DEFRAG_CMD_GET_JOB_STATUS, NULL, NULL, NULL, NULL, NULL, &JobStatus, &JobProgress );
    if( Result != NO_ERROR )
    {
        DislpayErrorMessage(GetErrorMes(DEFRAG_CMD_GET_JOB_STATUS), Result, ERR_INTERNAL, m_Fhwnd );
        return Result;
    }

    //Result = SendCommand( &m_drvInfo[item].JobId, DEFRAG_CMD_GET_MAP_AND_STATUS, NULL, &JobStatus, &JobProgress );
    Result = SendCommandEx( &m_drvInfo[item].JobId, DEFRAG_CMD_GET_MAP_AND_STATUS, NULL, NULL, NULL, NULL, NULL, &JobStatus, &JobProgress );
    if( Result != NO_ERROR )
    {
        DislpayErrorMessage(_T("Get Map and Status"), Result, ERR_INTERNAL, m_Fhwnd );
        return Result;
    }

    m_drvInfo[item].IsAnalysed = TRUE;
    m_drvInfo[item].diskStatus.Progress = JobProgress;
    UpdateMap(item);

    // Get Volume info
    if( m_drvInfo[item].JobLevel == 2 || // if Analize stage
        m_drvInfo[item].JobLevel == 3 )
    {
        //Result = SendCommand( &m_drvInfo[item].JobId, DEFRAG_CMD_GET_STATISTIC, NULL, &JobStatus, &JobProgress );
        //
        // BUGBUG: Data do not copied from Global.ServerBuffer
        //
        Result = SendCommandEx( &m_drvInfo[item].JobId, DEFRAG_CMD_GET_STATISTIC, NULL, NULL, NULL, NULL, NULL, &JobStatus, &JobProgress );
        if( Result == NO_ERROR )
        {
            if( m_drvInfo[item].JobLevel == 2 ) // if Analize stage
            {
                m_drvInfo[item].VolumeInfoBefore = *((PDFRG_VOLUME_INFO)Global.ServerBuffer);
            }
            m_drvInfo[item].VolumeInfoAfter = *((PDFRG_VOLUME_INFO)Global.ServerBuffer);
        }
        m_drvInfo[item].diskStatus.AnalysedTime = _time64( NULL );
    }

    if ( ((JobStatus) & DEFRAG_JOB_FLAG_INPROGRESS) == 0 )
    {
        if( m_drvInfo[item].JobLevel == 3 )
        {
            m_drvInfo[item].LastDefragTime = _time64(NULL);
            theApp.WriteInt( m_drvInfo[item].name + LAST_DEFRAG_TIME, (int)m_drvInfo[item].LastDefragTime );

            DriveHealth health( m_drvInfo[item].diskStatus, m_drvInfo[item].VolumeInfoAfter );
            GetMainFrm()->UpdateTaskpaneResultAndReliability( &health, m_drvInfo[item].JobStackSize == 0 );
        }
        if( m_drvInfo[item].JobLevel == 2 ) // if Analize stage
        {
            DriveHealth health( m_drvInfo[item].diskStatus, m_drvInfo[item].VolumeInfoAfter );
			GetMainFrm()->UpdateTaskpaneResultAndReliability( &health, m_drvInfo[item].JobStackSize == 0);
        }
        if( m_drvInfo[item].JobStackSize > 0 )
        {
            DWORD Command = m_drvInfo[item].JobStack[m_drvInfo[item].JobStackSize - 1];
            m_drvInfo[item].JobStackSize--;
            StartNewJob( item, Command );
        }
        else //if( !m_drvInfo[item].IsExternal )
        {
            //int jobLevel = m_drvInfo[item].JobLevel;
            StopJobIn(item);
            //m_drvInfo[item].JobLevel = jobLevel;
        }
	} else {
		// update task pane while in progress
        if( m_drvInfo[item].JobLevel == 3 )
        {
            DriveHealth health( m_drvInfo[item].diskStatus, m_drvInfo[item].VolumeInfoAfter );
            GetMainFrm()->UpdateTaskpaneResultAndReliability(&health, FALSE, FALSE);
        }
	}

    return Result;
}

int CADefragUIDoc::StartNewJob( int item, DWORD Command )
{
    PVOID       InData = NULL;
    if(Command == DEFRAG_CMD_INIT_JOB)
        InData = m_drvInfo[item].diskStatus.Name.GetBuffer();
    else
        InData = NULL;
    m_drvInfo[item].IsExternal = FALSE;

    int Result = CommJob(&m_drvInfo[item].JobId, Command, InData);
    if( Result == ERROR_SUCCESS )
    {
        m_drvInfo[item].CurrentJobCommand = Command;
        switch(Command)
        {
        case DEFRAG_CMD_INIT_JOB :
            m_drvInfo[item].JobLevel = 1;
            break;
        case DEFRAG_CMD_ANALIZE_VOLUME :
            m_drvInfo[item].JobLevel = 2;
            break;
        case DEFRAG_CMD_DEBUG1 :
        case DEFRAG_CMD_SIMPLE_DEFRAG :
        case DEFRAG_CMD_FREE_SPACE :
        case DEFRAG_CMD_SMART_BY_NAME:
        case DEFRAG_CMD_SMART_BY_ACCESS:
        case DEFRAG_CMD_SMART_BY_MODIFY:
        case DEFRAG_CMD_SMART_BY_SIZE:
        case DEFRAG_CMD_SMART_BY_CREATE:
        case DEFRAG_CMD_FORCE_TOGETHER:
            m_drvInfo[item].JobLevel = 3;
            break;
        case DEFRAG_CMD_STOP_JOB :
            m_drvInfo[item].JobLevel = 4;
            break;
        case DEFRAG_CMD_DELETE_JOB :
            m_drvInfo[item].JobLevel = 5;
            break;
        default:
            ASSERT(FALSE);
            m_drvInfo[item].JobLevel = 3;
            break;
        }
        if( Command != DEFRAG_CMD_INIT_JOB )
        {
            UpdateJobStatus(item,m_drvInfo[item].JobId);
        }
        else
        {
            ASSERT( m_drvInfo[item].JobStackSize > 0 );
            if( m_drvInfo[item].JobStackSize > 0 )
            {
                DWORD Command = m_drvInfo[item].JobStack[m_drvInfo[item].JobStackSize - 1];
                m_drvInfo[item].JobStackSize--;
                StartNewJob( item, Command );
            }
        }
    }
    else
    {
        m_drvInfo[item].CurrentJobCommand = 0;
        if(m_drvInfo[item].JobStackSize != 0)
        {
            StopJobIn(item);
            m_drvInfo[item].JobId = INVALID_JOB_ID;
            m_drvInfo[item].JobLevel = 0;
            m_drvInfo[item].JobStackSize = 0;
        }
        if ( NO_ERROR != Result )
        {
            Result = DislpayErrorMessage(GetErrorMes(Command), Result, ERR_OS, m_Fhwnd );
        }
        // Change button state
        m_drvInfo[item].FAnalyzeBtnState     = TRUE;
        m_drvInfo[item].FFreespaceBtnState   = TRUE;
        m_drvInfo[item].FDefragBtnState      = TRUE;
		m_drvInfo[item].FRecommendBtnState	 = GetDriveHealth(item) != DriveHealth::e_LevelGood;
        m_drvInfo[item].FStopBtnState        = FALSE;
        m_drvInfo[item].FPauseBtnState       = FALSE;
    }
    return Result;
}

DWORD CADefragUIDoc::JobTypeToCommand( DWORD jobType )
{
    switch ( jobType )
    {
    case DEFRAG_JOB_ANALIZE_ONLY: return DEFRAG_CMD_ANALIZE_VOLUME;
    case DEFRAG_JOB_DEBUG1: return DEFRAG_CMD_DEBUG1;
    case DEFRAG_JOB_FREE_SPACE: return DEFRAG_CMD_FREE_SPACE;
    case DEFRAG_JOB_SIMPLE_DEFRAG: return DEFRAG_CMD_SIMPLE_DEFRAG;
    case DEFRAG_JOB_SMART_BY_NAME: return DEFRAG_CMD_SMART_BY_NAME;
    case DEFRAG_JOB_SMART_BY_ACCESS: return DEFRAG_CMD_SMART_BY_ACCESS;
    case DEFRAG_JOB_SMART_BY_MODIFY: return DEFRAG_CMD_SMART_BY_MODIFY;
    case DEFRAG_JOB_SMART_BY_SIZE: return DEFRAG_CMD_SMART_BY_SIZE;
    case DEFRAG_JOB_SMART_BY_CREATE: return DEFRAG_CMD_SMART_BY_CREATE;
    case DEFRAG_JOB_FORCE_TOGETHER: return DEFRAG_CMD_FORCE_TOGETHER;
    }
    return 0;
}

//
//
//
void CADefragUIDoc::Timer(void)
{
    if ( !m_FInit ) return;
    if ( m_FInTimer ) return;

    m_FInTimer = TRUE;
    int Result = 0;

    TActiveJobIDsVector activeJobIDs;
    Result = GetActiveJobIDs( activeJobIDs );
    if( Result != ERROR_SUCCESS )
    {
        ASSERT(FALSE);
        m_FInTimer = FALSE;
        return;
    }

    BOOL needUpdateDriveList = FALSE;
    BOOL needUpdateDriveListStatus = !activeJobIDs.empty();

    // Update Job Status for exists jobs (for jobs started from UI)
    for ( int item = 0; item < (int)m_drvInfo.size(); item ++ )
    {
        if( ( m_drvInfo[item].JobId == INVALID_JOB_ID) || (m_drvInfo[item].JobId == 0) )
        {
            continue;
        }

        bool isJobActive = false;
        for( TActiveJobIDsVector::iterator i = activeJobIDs.begin(); i != activeJobIDs.end(); )
        {
            if( *i == m_drvInfo[item].JobId )
            {
                i = activeJobIDs.erase( i );
                isJobActive = true;
            }
            else
            {
                ++i;
            }
        }
        if( !isJobActive )
        {
            m_drvInfo[item].JobLevel = 0;
            m_drvInfo[item].JobId = INVALID_JOB_ID;
            m_drvInfo[item].FAnalyzeBtnState     = TRUE;
            m_drvInfo[item].FFreespaceBtnState   = TRUE;
            m_drvInfo[item].FDefragBtnState      = TRUE;
			m_drvInfo[item].FRecommendBtnState	 = GetDriveHealth(item) != DriveHealth::e_LevelGood;
            m_drvInfo[item].FStopBtnState        = FALSE;
            m_drvInfo[item].FPauseBtnState       = FALSE;
        }
        else
        {
            UpdateJobStatus( item, m_drvInfo[item].JobId );
        }
        if( m_drvInfo[item].JobId == INVALID_JOB_ID ) needUpdateDriveList = TRUE;
    }

    // Update Status for new activated Jobs (from Service)
    for( TActiveJobIDsVector::iterator i = activeJobIDs.begin(); i != activeJobIDs.end(); ++i )
    {
        DWORD JobId = *i;
        DWORD JobStatus = 0, JobProgress = 0;
        // get Drive name and detect item index
        //int Result = SendCommand( &JobId, DEFRAG_CMD_GET_JOB_OPTIONS, NULL, &JobStatus, &JobProgress );
        //
        // BUGBUG: Data do not copied from Global.ServerBuffer
        //
        int Result = SendCommandEx( &JobId, DEFRAG_CMD_GET_JOB_OPTIONS, NULL, NULL, NULL, NULL, NULL, &JobStatus, &JobProgress );
        if( Result != NO_ERROR )
        {
            ASSERT( FALSE );
            continue;
        }
        if( (JobStatus & DEFRAG_JOB_FLAG_INPROGRESS) == 0 && (JobStatus & DEFRAG_JOB_FLAG_STOPPED) != 0 )
            continue;   // skip already finished tasks
        PDEFRAG_JOB_OPTIONS JobOptions = (PDEFRAG_JOB_OPTIONS)Global.ServerBuffer;
        ASSERT(JobOptions);
        ASSERT( JobOptions->Id == JobId );
        CString volumeName = JobOptions->VolumeName;
        for ( int item = 0; item < (int)m_drvInfo.size(); item ++ )
        {
            if( volumeName.Find( m_drvInfo[item].name ) >= 0 )
            {
                m_drvInfo[item].JobId = JobId;
                m_drvInfo[item].IsExternal = TRUE;
                m_drvInfo[item].CurrentJobCommand = JobTypeToCommand( JobOptions->CurrentJobType );
                m_drvInfo[item].JobLevel = 2;
                // Change button state
                m_drvInfo[item].FAnalyzeBtnState     = FALSE;
                m_drvInfo[item].FFreespaceBtnState   = FALSE;
                m_drvInfo[item].FDefragBtnState      = FALSE;
				m_drvInfo[item].FRecommendBtnState	 = FALSE;
                m_drvInfo[item].FStopBtnState        = TRUE;
                m_drvInfo[item].FPauseBtnState       = FALSE;
                UpdateJobStatus( item, JobId );
                needUpdateDriveList = TRUE;
                break;
            }
        }
    }

    // Run new exists tasks
    for ( int item = 0; item < (int)m_drvInfo.size(); item ++ )
    {
        if( !(( m_drvInfo[item].JobId == INVALID_JOB_ID) || (m_drvInfo[item].JobId == 0)) )
        {
            continue; // skip already runned task
        }
        if(m_drvInfo[item].JobStackSize > 0)
        {
            DWORD Command = m_drvInfo[item].JobStack[m_drvInfo[item].JobStackSize - 1];
            m_drvInfo[item].JobStackSize--;
            DeleteJobIn( item );
            StartNewJob( item, Command );
            needUpdateDriveList = TRUE;
        }
    }

    CDefragView* driveListView = GetMainFrm()->GetDriveListView();
    if( needUpdateDriveList && driveListView )
    {
        driveListView->OnUpdate( NULL, 0L, NULL );
        //UpdateDrivesList();
    }
    else if( needUpdateDriveListStatus && driveListView ) // update Drive List View
    {
        driveListView->UpdateDrivesListStatus();
    }
    m_FInTimer = FALSE;
}

//
//
//
BOOL CADefragUIDoc::GetBtnState(int index)
{
    if ( !m_FInit ) return FALSE;

    switch ( index )
    {
    case ANALYZEBTN_STATE :
        return m_drvInfo[m_FCurrentItem].FAnalyzeBtnState;

    case FREESPACEBTN_STATE :
        return m_drvInfo[m_FCurrentItem].FFreespaceBtnState;

    case DEFRAGBTN_STATE:
        return m_drvInfo[m_FCurrentItem].FDefragBtnState;

	case RECCOMEND_STATE:
		return m_drvInfo[m_FCurrentItem].FRecommendBtnState;

    case STOPBTN_STATE:
        return m_drvInfo[m_FCurrentItem].FStopBtnState;

    case PAUSEBTN_STATE:
        return m_drvInfo[m_FCurrentItem].FPauseBtnState;

    default:
        return TRUE;
    }
}

BOOL CADefragUIDoc::JobInProgress( int index )
{
    return m_drvInfo[ index ].FStopBtnState;
}
//
//
//
DriveInfoMap& CADefragUIDoc::GetDisksInfo()
{
    return m_drvInfo;
}

//
//
//
void CADefragUIDoc::GetDrvMap(PBYTE &Map, int &Size, WCHAR *drvName)
{    
    if ( !m_FInit )
    {
        Map = NULL;
        Size = 0;
        wcscpy(drvName, _T(""));
    }
    else
    {
        Map     = m_drvInfo[ m_FCurrentItem ].Map;
        Size    = m_drvInfo[ m_FCurrentItem ].MapSize;
        wcscpy(drvName, m_drvInfo[ m_FCurrentItem ].diskStatus.Name);
        //MessageBox(m_Fhwnd, drvName, m_drvInfo[ m_FCurrentItem ].diskStatus.Name, MB_OK);
    }

}

BYTE CADefragUIDoc::GetLegendNum()
{    
    return (BYTE)(log( (double)m_CurrentLegend ) / log( 2.0 ));
}

int CADefragUIDoc::GetIndexByState( BlockStateNew State )
{
    int i;
    int Index = -1;

    for ( i = 0; i < BlockStateNewCount; i++ )
        if ( DefaultLegendItem[ i ].BlockStateID == State)
            Index = i;

    if ( Index == -1 )
    {
        State = FreeSpace;
        for ( i = 0; i < BlockStateNewCount; i++ )
            if ( DefaultLegendItem[ i ].BlockStateID == State)
                Index = i;
    }
    return Index;
}



BlockStateNew CADefragUIDoc::GetBlocksState( int Start, int End )
{
    PDFRG_MAP_BLOCK_INFO Map = (PDFRG_MAP_BLOCK_INFO)(m_drvInfo[ m_FCurrentItem ].Map);
    double Weights[ BlockStateNewCount ];
    int i;

    memset( Weights, 0, sizeof( Weights ) );

    for ( i = Start; i <= End; i++ )
    {
        Weights[ FragmentedFiles ]       += (double)Map[ i ].FragmentedFiles;
        Weights[ Directories ]           += (double)Map[ i ].Directories;
        Weights[ Mft ]                   += (double)Map[ i ].Mft;
        Weights[ Metadata ]              += (double)Map[ i ].Metadata;
        Weights[ MftZone ]               += (double)Map[ i ].MftZone;
        Weights[ PageFile ]              += (double)Map[ i ].PageFile;
        Weights[ HiberFile ]             += (double)Map[ i ].HiberFile;
        Weights[ FreeSpace ]             += (double)Map[ i ].FreeSpace;
        Weights[ LockedFiles ]           += (double)Map[ i ].LockedFiles;
        Weights[ BeingProcessed ]        += (double)Map[ i ].BeingProcessed;
        Weights[ CompressedFiles ]       += (double)Map[ i ].CompressedFiles;
        Weights[ Allocated ]             += (double)Map[ i ].Allocated;

        Weights[ LeastModifiedFiles ]    += (double)Map[ i ].LeastModifiedFiles;
        Weights[ RecentlyModifiedFiles ] += (double)Map[ i ].RecentlyModifiedFiles;
    }


    BYTE State, Ret = FreeSpace;

    double Max = 0.0;
    for ( State = FragmentedFiles; State < BlockStateNewCount; State++ )
    {
        int j = GetIndexByState( (BlockStateNew)State );
        int NumLegend = GetLegendNum();
        Weights[ State ] *= DefaultLegendItem[ j ].Weight[ NumLegend ];
        if ( Max < Weights[ State ] )
        {
            Max = Weights[ State ];            
            Ret = State;
        }
    }
    return (BlockStateNew)Ret;

/*
//
// This is simple priority logic.
// Use it to compare.
//
    BYTE NewState = FreeSpace; 

    if ( Weights[ Mft ] )
    {
        NewState = Mft;
    }
    else if ( Weights[ MftZone ] )
    {
        NewState = MftZone;
    }
    else if ( Weights[ PageFile ] )
    {
        NewState = PageFile;
    }
    else if ( Weights[ HiberFile ] )
    {
        NewState = HiberFile;
    }
    else if ( Weights[ LockedFiles ] ) // (FileCtx->DefragFlags & DEFRAG_FILE_LOCKED) )
    {
        NewState = LockedFiles;
    }
    else if ( Weights[ Metadata ] ) // (FileCtx->DefragFlags & DEFRAG_FILE_SYSTEM) )
    {
        NewState = Metadata;
    }
    else if ( Weights[ Directories ] ) // (FileCtx->DefragFlags & DEFRAG_FILE_DIRECTORY) )
    {
        NewState = Directories;
    }
    else if ( Weights[ CompressedFiles ] ) // (FileCtx->DefragFlags & DEFRAG_FILE_COMPRESSED) )
    {
        NewState = CompressedFiles;
    }
    else if ( Weights[ FragmentedFiles ] ) // (FileCtx->DefragFlags & DEFRAG_FILE_FRAGMENTED) )
    {
        NewState = FragmentedFiles;
    }
    else if ( Weights[ Allocated ] ) 
    {
        NewState = Allocated;
    }
    else if ( Weights[ LeastModifiedFiles ] )
    {
        NewState = LeastModifiedFiles;
    }
    else if ( Weights[ RecentlyModifiedFiles ] )
    {
        NewState = RecentlyModifiedFiles;
    }

    //if ( NewState > OldState )
    //{
    //    ...
    //}

    return (BlockStateNew)NewState;
*/

}

//
//
//
void CADefragUIDoc::GetDrvMapStart( int Size, BYTE Legend )
{
    m_CurrentLegend = Legend;
    //m_DrvMapScale   = (double)m_drvInfo[ m_FCurrentItem ].MapSize / (double)Size;
    m_DrvMapScale   = (double)DFRG_MAP_BLOCKS_MAX_NUM / (double)Size;
    //m_PrevMapPos    = 0;
}

//
//
//
BlockStateNew CADefragUIDoc::GetDrvMapItem( int Index )
{
    if ( !m_FInit )
        return FreeSpace; 

    if ( m_drvInfo[ m_FCurrentItem ].Map == NULL ) 
        return FreeSpace; 
    
    //return m_drvInfo[ m_FCurrentItem ].Map[  ];

    //PDFRG_MAP_BLOCK_INFO Map = (PDFRG_MAP_BLOCK_INFO)(m_drvInfo[ m_FCurrentItem ].Map);

    int Pos  = (int)( (double)Index * m_DrvMapScale );
    int PosE = (int)( (double)(Index + 1) * m_DrvMapScale ) - 1;
    if ( Pos > PosE )
        PosE = Pos;
    
    return GetBlocksState( Pos, PosE );    
}



CString CADefragUIDoc::GetDrvMapItemInfo( int Index )
{
    //DFRG_MAP_BLOCK_INFO Empty = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    if ( !m_FInit )
        return CString();

    if ( m_drvInfo[ m_FCurrentItem ].Map == NULL ) 
        return CString();
    
    //return m_drvInfo[ m_FCurrentItem ].Map[  ];

    //PDFRG_MAP_BLOCK_INFO Map = (PDFRG_MAP_BLOCK_INFO)(m_drvInfo[ m_FCurrentItem ].Map);

    int Pos  = (int)( (double)Index * m_DrvMapScale );
    int PosE = (int)( (double)(Index + 1) * m_DrvMapScale ) - 1;
    if ( Pos > PosE )
        PosE = Pos;

    PDFRG_MAP_BLOCK_INFO Map = (PDFRG_MAP_BLOCK_INFO)(m_drvInfo[ m_FCurrentItem ].Map);
    


    double Weights[ BlockStateNewCount ], Weights1[ BlockStateNewCount ];
    int i;

    memset( Weights, 0, sizeof( Weights ) );

    for ( i = Pos; i <= PosE; i++ )
    {
        Weights[ FragmentedFiles ]       += (double)Map[ i ].FragmentedFiles;
        Weights[ Directories ]           += (double)Map[ i ].Directories;
        Weights[ Mft ]                   += (double)Map[ i ].Mft;
        Weights[ Metadata ]              += (double)Map[ i ].Metadata;
        Weights[ MftZone ]               += (double)Map[ i ].MftZone;
        Weights[ PageFile ]              += (double)Map[ i ].PageFile;
        Weights[ HiberFile ]             += (double)Map[ i ].HiberFile;
        Weights[ FreeSpace ]             += (double)Map[ i ].FreeSpace;
        Weights[ LockedFiles ]           += (double)Map[ i ].LockedFiles;
        Weights[ BeingProcessed ]        += (double)Map[ i ].BeingProcessed;
        Weights[ CompressedFiles ]       += (double)Map[ i ].CompressedFiles;
        Weights[ Allocated ]             += (double)Map[ i ].Allocated;

        Weights[ LeastModifiedFiles ]    += (double)Map[ i ].LeastModifiedFiles;
        Weights[ RecentlyModifiedFiles ] += (double)Map[ i ].RecentlyModifiedFiles;
    }

    memcpy( Weights1, Weights, sizeof(Weights) );

    //BYTE State, Ret = FreeSpace;
    //if ( LegendTypePerfomance == m_CurrentLegend)
    //    Ret = LeastModifiedFiles;

    //double Max = 0.0;
    //for ( State = FragmentedFiles; State < BlockStateNewCount; State++ )
    //{
    //    int j = GetIndexByState( (BlockStateNew)State );
    //    int NumLegend = GetLegendNum();
    //    Weights[ State ] *= DefaultLegendItem[ j ].Weight[ NumLegend ];
    //    if ( Max < Weights[ State ] )
    //    {
    //        Max = Weights[ State ];            
    //        Ret = State;
    //    }
    //}

    CString StrInfo, StrFmt, StrFmt1;
    
    StrFmt1 = _T(":\t%.0f clusters\n");

    
    if ( m_CurrentLegend == LegendTypeDrvmap )
    {
        StrFmt.Append( _T("FragmentedFiles") );       StrFmt.Append( StrFmt1 );
        StrFmt.Append( _T("Directories") );           StrFmt.Append( StrFmt1 );
        StrFmt.Append( _T("Mft") );                   StrFmt.Append( StrFmt1 );
        StrFmt.Append( _T("Metadata") );              StrFmt.Append( StrFmt1 );
        StrFmt.Append( _T("MftZone") );               StrFmt.Append( StrFmt1 );
        StrFmt.Append( _T("PageFile") );              StrFmt.Append( StrFmt1 );
        StrFmt.Append( _T("HiberFile") );             StrFmt.Append( StrFmt1 );
        StrFmt.Append( _T("FreeSpace") );             StrFmt.Append( StrFmt1 );
        StrFmt.Append( _T("LockedFiles") );           StrFmt.Append( StrFmt1 );
        StrFmt.Append( _T("BeingProcessed") );        StrFmt.Append( StrFmt1 );
        StrFmt.Append( _T("CompressedFiles") );       StrFmt.Append( StrFmt1 );
        StrFmt.Append( _T("Allocated") );             StrFmt.Append( StrFmt1 );
        
        StrInfo.Format(StrFmt,        
        Weights1[ FragmentedFiles ],
        Weights1[ Directories ],
        Weights1[ Mft ],
        Weights1[ Metadata ],
        Weights1[ MftZone ],
        Weights1[ PageFile ],
        Weights1[ HiberFile ],
        Weights1[ FreeSpace ],
        Weights1[ LockedFiles ],
        Weights1[ BeingProcessed ],
        Weights1[ CompressedFiles ],
        Weights1[ Allocated ]        
        );
    }
    else
    {
        StrFmt.Append( _T("FreeSpace") );             StrFmt.Append( StrFmt1 );
        StrFmt.Append( _T("LeastModifiedFiles") );    StrFmt.Append( StrFmt1 );
        StrFmt.Append( _T("RecentlyModifiedFiles") ); StrFmt.Append( StrFmt1 );

        StrInfo.Format(StrFmt,       
        Weights1[ FreeSpace ],
        Weights1[ LeastModifiedFiles ],
        Weights1[ RecentlyModifiedFiles ]    
        
        );
    }

    {
        double perBlock = (double)m_drvInfo[ m_FCurrentItem ].diskStatus.TotalNumberOfClusters / (double)DFRG_MAP_BLOCKS_MAX_NUM;
        CString clusterInfo;
        clusterInfo.Format( _T("\nStart form cluster: %.0f\n")
            _T("Number of clusters in block: %.0f\n"), Pos * perBlock, ( PosE + 1.0 - Pos ) * perBlock * m_DrvMapScale);
        StrInfo += clusterInfo;
    }

    return StrInfo;
}


CString CADefragUIDoc::GetTmpDir()
{
    return m_Temp;
}

//
//
//
CString CADefragUIDoc::GetReportTmpDir()
{
    return m_ReportsTemp;
}

//
//
//
CString CADefragUIDoc::GetReportImgTmpDir()
{
    return m_ReportsImgTemp;
}

CLSID CADefragUIDoc::GetPicEncoder()
{
    return m_PicEncoder;
}

CString CADefragUIDoc::GetCompName()
{
    return m_CompName;
}

CString CADefragUIDoc::GetOSVer()
{
    return m_OSVer;
}

//
//
//
void CADefragUIDoc::GetStatInfo(CDiskStatus*& Status)
{
    if ( !m_FInit )
    {
        Status = NULL;
    }
    else
    {
        Status = &m_drvInfo[ m_FCurrentItem ].diskStatus;
    }
}

int CADefragUIDoc::GetVolumeInfo( DFRG_VOLUME_INFO* VolumeInfoBefore, DFRG_VOLUME_INFO* VolumeInfoAfter )
{
    if ( !m_FInit ) return ERROR_ACCESS_DENIED;
    if( VolumeInfoBefore ) *VolumeInfoBefore = m_drvInfo[ m_FCurrentItem ].VolumeInfoBefore;
    if( VolumeInfoAfter ) *VolumeInfoAfter = m_drvInfo[ m_FCurrentItem ].VolumeInfoAfter;
    return NO_ERROR;
}

//////////////////////////////////////////////////////////////////////////
// Get Cluster Information from service
//////////////////////////////////////////////////////////////////////////
int CADefragUIDoc::GetClusterInformation( int initialBlock, int& Count, DEFRAG_CINSPECTOR_INFO* ClusterInfo )
{
    int Pos  = (int)( (double)initialBlock * m_DrvMapScale ) + 1;

	double perBlock = (double)m_drvInfo[ m_FCurrentItem ].diskStatus.TotalNumberOfClusters / DFRG_MAP_BLOCKS_MAX_NUM;
    __int64 startCluster = static_cast<__int64>( Pos * perBlock );
    __int64 endCluster = static_cast<__int64>( startCluster + perBlock );

    //TODO: send command to service
    DWORD JobId = INVALID_JOB_ID;
    if( m_drvInfo[ m_FCurrentItem ].JobId != INVALID_JOB_ID )
    {
        JobId = m_drvInfo[ m_FCurrentItem ].JobId;
    }
    else if( m_drvInfo[ m_FCurrentItem ].LastJobId != INVALID_JOB_ID )
    {
        JobId = m_drvInfo[ m_FCurrentItem ].LastJobId;
    }
    else
    {
        return ERROR_INVALID_DRIVE;
    }

    DWORD Command = DEFRAG_CMD_GET_CLUSTER_FILES;
    DWORD JobStatus = ERROR_SUCCESS, JobProgress;

    CLUSTERS_INFO   ClusterRequestInfo;
    ClusterRequestInfo.StartCluster = startCluster;
    ClusterRequestInfo.ClustersNumber = endCluster - startCluster + 1; // (ULONGLONG)perBlock; //

    ULONG   DataLength = Count * sizeof( DEFRAG_CINSPECTOR_INFO );
    int Result = SendCommandEx( &JobId, Command, &ClusterRequestInfo, NULL, NULL, ClusterInfo, &DataLength, &JobStatus, &JobProgress );
    TRACE( _T(" Get cluster information for job with id=%d, result=%d\n"), JobId, Result );

    if( Result != ERROR_SUCCESS )
    {
		Count = 0;
        return Result;
    }

    Count = DataLength / sizeof( DEFRAG_CINSPECTOR_INFO );

    return 0;
}


int CADefragUIDoc::GetClusterInfo( ULONGLONG startCluster, int Count, DEFRAG_CINSPECTOR_INFO* ClusterInfo )
{
    DWORD JobId = INVALID_JOB_ID;
    if( m_drvInfo[ m_FCurrentItem ].JobId != INVALID_JOB_ID )
    {
        JobId = m_drvInfo[ m_FCurrentItem ].JobId;
    }
    else if( m_drvInfo[ m_FCurrentItem ].LastJobId != INVALID_JOB_ID )
    {
        JobId = m_drvInfo[ m_FCurrentItem ].LastJobId;
    }
    else
    {
        return ERROR_INVALID_DRIVE;
    }

    DWORD Command = DEFRAG_CMD_GET_CLUSTER_FILES;
    DWORD JobStatus = ERROR_SUCCESS, JobProgress;

    CLUSTERS_INFO   ClusterRequestInfo;
    ClusterRequestInfo.StartCluster = startCluster;
    ClusterRequestInfo.ClustersNumber = Count;

    ULONG   DataLength = Count * sizeof( DEFRAG_CINSPECTOR_INFO );
    int Result = SendCommandEx( &JobId, Command, &ClusterRequestInfo, NULL, NULL, ClusterInfo, &DataLength, &JobStatus, &JobProgress );
    TRACE( _T(" Get cluster information for job with id=%d, result=%d"), JobId, Result );

    if( Result != ERROR_SUCCESS )
    {
        return Result;
    }

    Count = DataLength / sizeof( DEFRAG_CINSPECTOR_INFO );

    return 0;
}


//////////////////////////////////////////////////////////////////////////
// Get Settings value from service
//////////////////////////////////////////////////////////////////////////
int CADefragUIDoc::GetSettingsValue( DWORD id, IN OUT LPVOID buffer, IN DWORD bufferSize )
{
    if ( !m_FInit ) return ERROR_ACCESS_DENIED;

    SETTINGS_INFO   Setting;
    //
    // Get settings with given ID
    //
    Setting.SettingId = id;
    Setting.SettingOp = OP_GET;
    //Setting.DataLength = bufferSize;

    DWORD JobId = GLOBAL_JOB_ID;
    DWORD Command = DEFRAG_CMD_GET_SETTING;
    //DWORD JobStatus = ERROR_SUCCESS, JobProgress;

    //int Result = SendCommand( &JobId, Command, &Setting, &JobStatus, &JobProgress );
    int Result = SendCommandEx( &JobId, Command, &Setting, NULL, NULL, buffer, &bufferSize, NULL, NULL );
    TRACE( _T(" Get settings with id=%d, result=%d\n"), id, Result );

    if( Result != ERROR_SUCCESS )
    {
        return Result;
    }

    //CopyMemory( buffer, Global.ServerBuffer, Global.BufferLength );

    return Result;
}

int CADefragUIDoc::SetSettingsValue( DWORD id, IN LPVOID buffer, IN DWORD bufferSize )
{
    if ( !m_FInit ) return ERROR_ACCESS_DENIED;

    SETTINGS_INFO   Setting;
    //
    // Set given settings value
    //
    Setting.SettingId = id;
    Setting.SettingOp = OP_SET;
    //Setting.DataLength = bufferSize;
    //memcpy( Global.ClientBuffer, buffer, bufferSize );

    DWORD JobId = GLOBAL_JOB_ID;
    DWORD Command = DEFRAG_CMD_SET_SETTING;
    //DWORD JobStatus = ERROR_SUCCESS, JobProgress;

    //int Result = SendCommand( &JobId, Command, &Setting, &JobStatus, &JobProgress );
    int Result = SendCommandEx( &JobId, Command, &Setting, buffer, bufferSize, NULL, NULL, NULL, NULL );

    TRACE( _T(" Set properties with id=%d, result=%d\n"), id, Result );
    return Result;
}

int CADefragUIDoc::ClearSettingsValue(DWORD id)
{
    if ( !m_FInit ) return ERROR_ACCESS_DENIED;

    SETTINGS_INFO   Setting;
    //
    // Set given settings value
    //
    Setting.SettingId = id;
    Setting.SettingOp = OP_REMOVE_ALL;

    DWORD JobId = GLOBAL_JOB_ID;
    DWORD Command = DEFRAG_CMD_SET_SETTING;
    int Result = SendCommandEx( &JobId, Command, &Setting, NULL, 0, NULL, NULL, NULL, NULL );

    TRACE( _T(" Clear list with id=%d, result=%d\n"), id, Result );
    return Result;
}

//////////////////////////////////////////////////////////////////////////
/*
int CADefragUIDoc::GetSchedulerTasks( CScheduleTaskList& tasks )
{
    if ( !m_FInit ) return ERROR_ACCESS_DENIED;

    SETTINGS_INFO   Setting;
    //
    // Get settings with given ID
    //
    Setting.SettingId = SheduledJobsId;
    Setting.SettingOp = OP_GET;
    Setting.DataLength = Global.BufferLength;

    DWORD JobId = GLOBAL_JOB_ID;
    DWORD Command = DEFRAG_CMD_GET_SETTING;
    DWORD JobStatus = ERROR_SUCCESS, JobProgress;

    int Result = SendCommand( &JobId, Command, &Setting, &JobStatus, &JobProgress );
    TRACE( _T(" Get Scheduler settings result=%d"), Result );
    if( Result != ERROR_SUCCESS )
    {
        return Result;
    }

    tasks.clear();
    PDFRG_SHEDULED_JOB Job = (PDFRG_SHEDULED_JOB)Global.ServerBuffer;
    for( DWORD i = 0; i < Setting.DataLength / sizeof(DFRG_SHEDULED_JOB); ++i )
    {
        CScheduleTask newTask;
        newTask.SetServerData(Job + i);
        tasks.push_back( newTask );
    }

    return Result;
}
*/
int CADefragUIDoc::GetSchedulerTasks( CScheduleTaskList& tasks )
{
    if ( !m_FInit ) return ERROR_ACCESS_DENIED;

    SETTINGS_INFO   Setting;
    //
    // Get settings with given ID
    //
    Setting.SettingId = SheduledJobsId;
    Setting.SettingOp = OP_GET;

    DWORD JobId = GLOBAL_JOB_ID;
    DWORD Command = DEFRAG_CMD_GET_SETTING;
    DWORD   DataLength = Global.BufferLength;

    PDFRG_SHEDULED_JOB Job = (PDFRG_SHEDULED_JOB)malloc( DataLength );

    int Result = ERROR_NOT_ENOUGH_MEMORY;

    if ( Job )
    {
        Result = SendCommandEx( &JobId, Command, &Setting, NULL, NULL, Job, &DataLength, NULL, NULL );
        TRACE( _T(" Get Scheduler settings result=%d"), Result );

        if( Result == ERROR_SUCCESS )
        {
            tasks.clear();
            for( DWORD i = 0; i < DataLength / sizeof(DFRG_SHEDULED_JOB); ++i )
            {
                CScheduleTask newTask;
                newTask.SetServerData(Job + i);
                tasks.push_back( newTask );
            }
        }

        free( Job );
    }

    return Result;
}

//////////////////////////////////////////////////////////////////////////
int CADefragUIDoc::AddSchedulerTask( CScheduleTask& task )
{
    if ( !m_FInit ) return ERROR_ACCESS_DENIED;

    if( task.GetID() <= 0 ) // Update task ID
    {
        CScheduleTaskList tasks;
        GetSchedulerTasks( tasks );
        int id = 1;
        for( size_t i = 0; i < tasks.size(); ++i )
        {
            if( tasks[i].GetID() >= id ) id = tasks[i].GetID() + 1;
        }
        task.SetID( id );
    }

    SETTINGS_INFO   Setting;
    //
    // Get settings with given ID
    //
    Setting.SettingId = SheduledJobsId;
    Setting.SettingOp = OP_ADD;
    //Setting.DataLength = sizeof(DFRG_SHEDULED_JOB);
    //task.GetServerData( (PDFRG_SHEDULED_JOB)Global.ClientBuffer );
    DFRG_SHEDULED_JOB  Job;
    task.GetServerData( &Job );

    DWORD JobId = GLOBAL_JOB_ID;
    DWORD Command = DEFRAG_CMD_SET_SETTING;
    //DWORD JobStatus = ERROR_SUCCESS, JobProgress;

    //int Result = SendCommand( &JobId, Command, &Setting, &JobStatus, &JobProgress );
    int Result = SendCommandEx( &JobId, Command, &Setting, &Job, sizeof(DFRG_SHEDULED_JOB), NULL, NULL, NULL, NULL );
    TRACE( _T(" Add Scheduler task result=%d"), Result );

    if( Result != ERROR_SUCCESS )
    {
        return Result;
    }
    return Result;
}

//////////////////////////////////////////////////////////////////////////
int CADefragUIDoc::SetSchedulerTask( CScheduleTask& task )
{
    return AddSchedulerTask( task );
}

//////////////////////////////////////////////////////////////////////////
int CADefragUIDoc::RunSchedulerTask( CScheduleTask& task )
{
    task.SetManualNextRunTime( CTime::GetCurrentTime() );
    return AddSchedulerTask( task );
}

//////////////////////////////////////////////////////////////////////////
int CADefragUIDoc::RemoveSchedulerTask( CScheduleTask& task )
{
    if ( !m_FInit ) return ERROR_ACCESS_DENIED;

    SETTINGS_INFO   Setting;
    //
    // Get settings with given ID
    //
    Setting.SettingId = SheduledJobsId;
    Setting.SettingOp = OP_REMOVE;
    //Setting.DataLength = sizeof(DFRG_SHEDULED_JOB);
    //task.GetServerData( (PDFRG_SHEDULED_JOB)Global.ClientBuffer );
    DFRG_SHEDULED_JOB  Job;
    task.GetServerData( &Job );

    DWORD JobId = GLOBAL_JOB_ID;
    DWORD Command = DEFRAG_CMD_SET_SETTING;
    //DWORD JobStatus = ERROR_SUCCESS, JobProgress;

    //int Result = SendCommand( &JobId, Command, &Setting, &JobStatus, &JobProgress );
    int Result = SendCommandEx( &JobId, Command, &Setting, &Job, sizeof(DFRG_SHEDULED_JOB), NULL, NULL, NULL, NULL );
    TRACE( _T(" Add Scheduler task result=%d"), Result );

    if( Result != ERROR_SUCCESS )
    {
        return Result;
    }
    return Result;
}

CString CADefragUIDoc::GetCommandName( DWORD command )
{
    CString name;
    switch(command)
    {
    case DEFRAG_CMD_INIT_JOB :
        name.LoadString( IDS_DEFRAG_CMD_INIT_JOB );
        break;
    case DEFRAG_CMD_ANALIZE_VOLUME :
        name.LoadString( IDS_DEFRAG_CMD_ANALIZE_VOLUME );
        break;
    case DEFRAG_CMD_DEBUG1 :
        name.LoadString( IDS_DEFRAG_CMD_DEBUG1 );
        break;
    case DEFRAG_CMD_SIMPLE_DEFRAG :
        name.LoadString( IDS_DEFRAG_CMD_SIMPLE_DEFRAG );
        break;
    case DEFRAG_CMD_FREE_SPACE :
        name.LoadString( IDS_DEFRAG_CMD_FREE_SPACE );
        break;
    case DEFRAG_CMD_SMART_BY_NAME:
        name.LoadString( IDS_DEFRAG_CMD_SMART_BY_NAME );
        break;
    case DEFRAG_CMD_SMART_BY_ACCESS:
        name.LoadString( IDS_DEFRAG_CMD_SMART_BY_ACCESS );
        break;
    case DEFRAG_CMD_SMART_BY_MODIFY:
        name.LoadString( IDS_DEFRAG_CMD_SMART_BY_MODIFY );
        break;
    case DEFRAG_CMD_SMART_BY_SIZE:
        name.LoadString( IDS_DEFRAG_CMD_SMART_BY_SIZE );
        break;
    case DEFRAG_CMD_SMART_BY_CREATE:
        name.LoadString( IDS_DEFRAG_CMD_SMART_BY_CREATE );
        break;
    case DEFRAG_CMD_FORCE_TOGETHER:
        name.LoadString( IDS_DEFRAG_CMD_FORCE_TOGETHER );
        break;
    case DEFRAG_CMD_STOP_JOB :
    case DEFRAG_CMD_DELETE_JOB :
    default:
        break;
    }
    return name;
}

int CADefragUIDoc::GetDriveHealth(int item)
{
    DriveHealth health( m_drvInfo[item].diskStatus, m_drvInfo[item].VolumeInfoAfter );
	double fragmentation = health.GetOveralFragmentation();
	int level;

    if( fragmentation <= 10. ) {
		level = DriveHealth::e_LevelGood;
    } else if( fragmentation <= 50. ) {
        level = DriveHealth::e_LevelWarning;
    } else {
		level = DriveHealth::e_LevelCritical;
    }

	return level;
}


