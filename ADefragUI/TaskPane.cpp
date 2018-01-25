/*
    Defrag CTaskPane class module 

    Module name:

        TaskPane.cpp

    Abstract:

        Contains implementation of the CTaskPane class.

    $Header: /home/CVS_DEFRAG/Defrag/Src/ADefragUI/TaskPane.cpp,v 1.5 2010/01/12 22:53:18 roman Exp $
    $Log: TaskPane.cpp,v $
    Revision 1.5  2010/01/12 22:53:18  roman
    Add "Recommended Defrag Method..." button

    Revision 1.4  2009/12/24 01:39:10  roman
    fix errors

    Revision 1.3  2009/12/03 15:36:55  dimas
    CVS headers included

*/

#include "stdafx.h"
#include "resource.h"
#include "TaskPane.h"
#include "DriveInfo.h"
#include "local_strings.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTaskPane

BEGIN_MESSAGE_MAP(CTaskPane, CMFCTasksPane)
    ON_WM_CREATE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTaskPane construction/destruction

CTaskPane::CTaskPane() :
    m_nResultPage(-1),
	m_OveralHealthLevel(DriveHealth::e_LevelGood)
{
}

CTaskPane::~CTaskPane()
{
}

CString GetTaskName( UINT taskId, BOOL left = FALSE )
{
    CString strTemp;
    BOOL bNameValid = strTemp.LoadString( taskId );
    ASSERT(bNameValid);
    int pos = strTemp.Find(_T('\n'));
    return left ? strTemp.Mid( 0, pos ): strTemp.Mid( pos + 1 );
}

/////////////////////////////////////////////////////////////////////////////
// CTaskPane message handlers

int CTaskPane::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
    if (CMFCTasksPane::OnCreate(lpCreateStruct) == -1)
        return -1;

    CString strTaskPaneWnd;
    BOOL bNameValid = strTaskPaneWnd.LoadString(IDS_TASKPANE_WND);
    ASSERT(bNameValid);
    SetCaption( strTaskPaneWnd );
    SetIconsList( IDB_DEFRAGSMALL, 16 );

    EnableNavigationToolbar (TRUE);
    EnableWrapLabels( TRUE );
    EnableWrapTasks( TRUE );
    EnableOffsetCustomControls (FALSE);

    // Main Group
    int nMainGroup = AddGroup( CLocalString(IDS_TASKPANE_MAIN_TASKS), FALSE, TRUE );

    AddTask( nMainGroup, GetTaskName(ID_DEFRAG_COMPUTER), 0, ID_DEFRAG_COMPUTER );

    // Defrag Group
    CString strTemp;
    bNameValid = strTemp.LoadString(IDS_RIBBON_DEFRAG);
    ASSERT(bNameValid);
    int nDefragGroup = AddGroup( strTemp );

    // Analyze btn
    AddTask( nDefragGroup, GetTaskName(ID_DEFRAG_ANALYZE), 1, ID_DEFRAG_ANALYZE );
    AddSeparator( nDefragGroup );

    // Free space btn    
    AddTask( nDefragGroup, GetTaskName( ID_DEFRAG_FREE_SPACE ), 2, ID_DEFRAG_FREE_SPACE );

    // Defrag btn
    AddTask( nDefragGroup, GetTaskName( ID_DEFRAG_SIMPLE ), 3, ID_DEFRAG_SIMPLE );

	// Recommend btn
    AddTask( nDefragGroup, GetTaskName( ID_DEFRAG_RECOMMEND ), 3, ID_DEFRAG_RECOMMEND );
    AddSeparator( nDefragGroup );

    // Defrag Order btn
    AddTask( nDefragGroup, GetTaskName( ID_DEFRAG_BY_FORCE_FILL ), -1, ID_DEFRAG_BY_FORCE_FILL );

    // Order Defrag Group
    int nOrderDefragGroup = AddGroup( GetTaskName( IDS_RIBBON_DEFRAG_ORDER, TRUE ) );
    AddTask( nOrderDefragGroup, GetTaskName( ID_DEFRAG_BY_NAME ), -1, ID_DEFRAG_BY_NAME );
    AddTask( nOrderDefragGroup, GetTaskName( ID_DEFRAG_BY_ACCESS ), -1, ID_DEFRAG_BY_ACCESS );
    AddTask( nOrderDefragGroup, GetTaskName( ID_DEFRAG_BY_MODIFY ), -1, ID_DEFRAG_BY_MODIFY );
    AddTask( nOrderDefragGroup, GetTaskName( ID_DEFRAG_BY_CREATION ), -1, ID_DEFRAG_BY_CREATION );
    AddTask( nOrderDefragGroup, GetTaskName( ID_DEFRAG_BY_SIZE ), -1, ID_DEFRAG_BY_SIZE );

    // Progress Group
    int nProgressGroup = AddGroup( GetTaskName( IDS_RIBBON_PROGRESS_PANEL ) );
    AddTask( nProgressGroup, GetTaskName( ID_DEFRAG_PAUSE ), 6, ID_DEFRAG_PAUSE );
    AddTask( nProgressGroup, GetTaskName( ID_DEFRAG_STOP ), 5, ID_DEFRAG_STOP );

    int nDetailsGroup = AddGroup( CLocalString(IDS_TASKPANE_MAIN_DETAILS), TRUE);
    AddLabel( nDetailsGroup, CLocalString(IDS_TASKPANE_DETAILS_INFO) );

    // Add second page "Result and Reliability":
    m_nResultPage = AddPage( CLocalString(IDS_TASKPANE_RESULT) );
    UpdateResultAndReliabilityPage( NULL, FALSE );

    //AddPage( _T("Test page") );

    return 0;
}

void CTaskPane::UpdateToolbar ()
{
    m_wndToolBar.RedrawWindow ();
}

void CTaskPane::UpdateResultAndReliabilityPage( DriveHealth* health, BOOL activate )
{
    RemoveAllGroups( m_nResultPage );
    if( !health )
    {
        int InfoGroup = AddGroup( m_nResultPage, _T(""), FALSE, TRUE );
        AddLabel( InfoGroup, CLocalString(IDS_TASKPANE_RESULT_NOTE) );
        RecalcLayout();
        return;
    }

    m_InfoGroup = AddGroup( m_nResultPage, CLocalString(IDS_TASKPANE_RESULT_MAIN), FALSE, TRUE );

    double fragmentation;
    CString description;
    CString recomendations;
	health->GetOveralFragmentationInfo(fragmentation,
									   *(enum DriveHealth::HealthLevel*)&m_OveralHealthLevel,
									   description,
									   recomendations);
    CString fileFragmentationDescription;
    health->GetFileFragmentationHealth( &fileFragmentationDescription );
    m_DescriptionTask = AddLabel( m_InfoGroup, description, 1, TRUE );
    m_FGMDescriptionTask = AddLabel( m_InfoGroup, fileFragmentationDescription );

    m_RecomendGroup = AddGroup( m_nResultPage, CLocalString(IDS_TASKPANE_RESULT_RECOMEND), FALSE, TRUE );
    AddLabel( m_RecomendGroup, recomendations, -1, TRUE );

    m_ReliabilityGroup = AddGroup( m_nResultPage, CLocalString(IDS_TASKPANE_RESULT_RELIABILITY), FALSE, TRUE );
    // MFT
    ULARGE_INTEGER MFTfragments;
    DriveHealth::HealthLevel MFTlevel;
    CString MFTdescription;
    CString MFTrecomendations;
    health->GetMFTFragmentationInfo( MFTfragments, MFTlevel, MFTdescription, MFTrecomendations );
    AddLabel( m_ReliabilityGroup, CLocalString(IDS_TASKPANE_RESULT_MFT), -1, TRUE );
    m_MFTDesc = AddLabel( m_ReliabilityGroup, MFTdescription, -1, FALSE );
    m_MFTReccomend = AddLabel( m_ReliabilityGroup, MFTrecomendations, -1, TRUE );
    AddSeparator( m_ReliabilityGroup );

    // Paging File Fragmentation
    ULARGE_INTEGER PageFileFragments;
    DriveHealth::HealthLevel PageFileLevel;
    CString PageFileDescription;
    CString PageFileRecomendations;
    health->GetPageFileFragmentationInfo( PageFileFragments, PageFileLevel, PageFileDescription, PageFileRecomendations );
    AddLabel( m_ReliabilityGroup, CLocalString(IDS_TASKPANE_RESULT_PAGEFILE), -1, TRUE );
    m_PageFileDesc = AddLabel( m_ReliabilityGroup, PageFileDescription, -1, FALSE );
    m_PageFileReccomend = AddLabel( m_ReliabilityGroup, PageFileRecomendations, -1, TRUE );
    AddSeparator( m_ReliabilityGroup );

    // Free Space Fragmentation
    double space;
    DriveHealth::HealthLevel FreeLevel;
    CString FreeDescription;
    CString FreeRecomendations;
    health->GetFreeSpaceFragmentationInfo( space, FreeLevel, FreeDescription, FreeRecomendations );
    AddLabel( m_ReliabilityGroup, CLocalString(IDS_TASKPANE_RESULT_FREE), -1, TRUE );
    m_FreeDesc = AddLabel( m_ReliabilityGroup, FreeDescription, -1, FALSE );
    m_FreeReccomend = AddLabel( m_ReliabilityGroup, FreeRecomendations, -1, TRUE );

    RecalcLayout();

    if( activate ) SetActivePage( m_nResultPage );
}



void CTaskPane::UpdateDriveHealth( DriveHealth* health )
{
    double fragmentation;
    CString description;
    CString recomendations;
	health->GetOveralFragmentationInfo(fragmentation,
									   *(enum DriveHealth::HealthLevel*)&m_OveralHealthLevel,
									   description,
									   recomendations);

    CString fileFragmentationDescription;
    health->GetFileFragmentationHealth( &fileFragmentationDescription );

	// overal health
	SetTaskName(m_InfoGroup, m_DescriptionTask, description);
	SetTaskName(m_InfoGroup, m_FGMDescriptionTask, fileFragmentationDescription);

	// reccomendations
	SetGroupName(m_RecomendGroup, recomendations);

	// reliability
    ULARGE_INTEGER MFTfragments;
    DriveHealth::HealthLevel MFTlevel;
    CString MFTdescription;
    CString MFTrecomendations;
    health->GetMFTFragmentationInfo( MFTfragments, MFTlevel, MFTdescription, MFTrecomendations );
    SetTaskName( m_ReliabilityGroup, m_MFTDesc, MFTdescription );
    SetTaskName( m_ReliabilityGroup, m_MFTReccomend, MFTrecomendations );

    ULARGE_INTEGER PageFileFragments;
    DriveHealth::HealthLevel PageFileLevel;
    CString PageFileDescription;
    CString PageFileRecomendations;
    health->GetPageFileFragmentationInfo( PageFileFragments, PageFileLevel, PageFileDescription, PageFileRecomendations );
    SetTaskName( m_ReliabilityGroup, m_PageFileDesc, PageFileDescription );
    SetTaskName( m_ReliabilityGroup, m_PageFileReccomend, PageFileRecomendations );

    double space;
    DriveHealth::HealthLevel FreeLevel;
    CString FreeDescription;
    CString FreeRecomendations;
    health->GetFreeSpaceFragmentationInfo( space, FreeLevel, FreeDescription, FreeRecomendations );
    SetTaskName( m_ReliabilityGroup, m_FreeDesc, FreeDescription );
    SetTaskName( m_ReliabilityGroup, m_FreeReccomend, FreeRecomendations );
}