/*
    Defrag CInspector class module 

    Module name:

        CInspector.cpp

    Abstract:

        Contains implementation of the Cluster Inspector (CInspector class).

    $Header: /home/CVS_DEFRAG/Defrag/Src/ADefragUI/CInspector.cpp,v 1.7 2009/12/18 00:16:19 roman Exp $
    $Log: CInspector.cpp,v $
    Revision 1.7  2009/12/18 00:16:19  roman
    CInspector multiple fragments view, Exclude list remove operations.

    Revision 1.6  2009/12/15 18:50:46  roman
    Fix bug with start/end cluster calculation from block number for CInspector.

    Revision 1.5  2009/12/14 10:27:42  dimas
    CVS headers included


*/

#include "stdafx.h"
#include "ADefragUI.h"
#include "ADefragUIDoc.h"
#include "CInspector.h"



// CCInspector dialog

IMPLEMENT_DYNAMIC(CCInspector, CDialog)

CCInspector::CCInspector(CWnd* pParent /*=NULL*/)
	: CDialog(CCInspector::IDD, pParent)
    , m_initialBlock( 0 )
	, m_blockCount(0)
	, m_startCluster(0)
{
    m_pDoc = NULL;
}

CCInspector::~CCInspector()
{
	info.clear();
}

void CCInspector::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CCInspector, CDialog)

	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN1, &CCInspector::OnDeltaposSpin1)
	ON_EN_CHANGE(IDC_EDIT1, &CCInspector::OnEnChangeEdit1)
	ON_BN_CLICKED(IDC_CIUPDATEBTN, &CCInspector::OnBnClickedCiupdatebtn)
	ON_BN_CLICKED(IDC_CIOPENFILEBTN, &CCInspector::OnBnClickedCiopenfilebtn)
//	ON_NOTIFY(LVN_ITEMCHANGED, IDC_CILIST, &CCInspector::OnLvnItemchangedCilist)
	ON_NOTIFY(LVN_ITEMACTIVATE, IDC_CILIST, &CCInspector::OnLvnItemActivateCilist)
//	ON_NOTIFY(LVN_ITEMCHANGED, IDC_CILIST, &CCInspector::OnLvnItemchangedCilist)
//	ON_NOTIFY(HDN_ITEMCLICK, 0, &CCInspector::OnHdnItemclickCilist)
//ON_NOTIFY(LVN_COLUMNCLICK, IDC_CILIST, &CCInspector::OnLvnColumnclickCilist)
//ON_NOTIFY(LVN_LINKCLICK, IDC_CILIST, &CCInspector::OnLvnLinkClickedCilist)
ON_NOTIFY(NM_CLICK, IDC_CILIST, &CCInspector::OnNMClickCilist)
END_MESSAGE_MAP()


// CCInspector protected mebers

void CCInspector::UpdateDataList()
{
	if( !m_pDoc )
		return;

    CListCtrl* m_pList = (CListCtrl*)GetDlgItem(IDC_CILIST);
	if (!m_pList)
		return;

	// allocate memory as far as possible
	m_blockCount = m_pDoc->GetClustersInBlock();
	info.resize(m_blockCount);

	if (m_pDoc->GetClusterInformation( m_initialBlock, m_blockCount, &info[0] ) != ERROR_SUCCESS) {
		return;
	}
	m_startCluster = info[0].StartCluster;
	m_currentFileName = info[0].FileName;

	// Insert items in the list view control.
	CString Str;
	for (int i=0; i < m_blockCount; i++)
	{
		Str.Format(TEXT("%d"), info[i].StartCluster);

		// Insert the item, select every other item.
		m_pList->InsertItem(
			LVIF_TEXT|LVIF_STATE, i, Str, 0, 0, 0, 0);

		Str.Format(TEXT("%d"), info[i].NumberOfClusters);
		m_pList->SetItemText(i, 1, Str);

		Str.Format(TEXT("%d"), info[i].VCN);
		m_pList->SetItemText(i, 2, Str);

		Str.SetString(info[i].FileName);
		m_pList->SetItemText(i, 3, Str);
	}

	Str.Format(TEXT("%d"), m_startCluster);
	CWnd* pWnd = GetDlgItem(IDC_EDIT1);
	pWnd->SetWindowText(Str);
}



// CCInspector message handlers

BOOL CCInspector::OnInitDialog()
{
    CDialog::OnInitDialog();

    if( !m_pDoc )
    {
        return FALSE;
    }

    CDiskStatus* Status = NULL;
    m_pDoc->GetStatInfo( Status );

    if ( Status )
    {
        WCHAR lpStr[100];
        wsprintf(lpStr, L"%s\\ - ClusterInspector", Status->Name);
        SendMessage(WM_SETTEXT, 0, (LPARAM)lpStr);
    }

    CListCtrl* m_pList = (CListCtrl*)GetDlgItem(IDC_CILIST);
	if (!m_pList)
    {
        return FALSE;
    }

	m_pList->SetExtendedStyle(LVS_EX_FULLROWSELECT);

    CString Str;
    Str.LoadString( IDS_FIRSTCOL );
    m_pList->InsertColumn(0, Str, LVCFMT_LEFT, COLUMN1WIDTH);
    Str.LoadString( IDS_SECONDCOL );
    m_pList->InsertColumn(1, Str, LVCFMT_LEFT, COLUMN2WIDTH);
    Str.LoadString( IDS_THIRDCOL );
    m_pList->InsertColumn(2, Str, LVCFMT_LEFT, COLUMN3WIDTH);
    Str.LoadString( IDS_FOURTHCOL );
    m_pList->InsertColumn(3, Str, LVCFMT_LEFT, COLUMN4WIDTH);

	UpdateDataList();

    return TRUE;  // return TRUE unless you set the focus to a control
}


void CCInspector::SetDocument(CADefragUIDoc *Doc)
{
    m_pDoc = Doc;
}


void CCInspector::OnDeltaposSpin1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	*pResult = 0;

	m_startCluster += pNMUpDown->iDelta * (-1);

	CString Str;
	Str.Format(TEXT("%d"), m_startCluster);
	CWnd* pWnd = GetDlgItem(IDC_EDIT1);
	pWnd->SetWindowText(Str);
}


void CCInspector::OnEnChangeEdit1()
{
	CString Str;
	CWnd* pWnd = GetDlgItem(IDC_EDIT1);
	pWnd->GetWindowText(Str);

	ULONGLONG startCluster = _wtoi(Str.GetString());
	if (startCluster)
		m_startCluster = startCluster;
}


void CCInspector::OnBnClickedCiupdatebtn()
{
	if( !m_pDoc )
		return;

    CListCtrl* m_pList = (CListCtrl*)GetDlgItem(IDC_CILIST);
	if (!m_pList)
		return;

	m_blockCount = m_pDoc->GetClustersInBlock();
	info.resize(m_blockCount);

//	std::vector<DEFRAG_CINSPECTOR_INFO> info( m_blockCount );
    m_pDoc->GetClusterInfo( m_startCluster, m_blockCount, &info[0] );
	m_currentFileName = info[0].FileName;
	
	// Insert items in the list view control.
	CString Str;
	m_pList->DeleteAllItems();
	for (int i=0; i < m_blockCount; i++)
	{
		if (!info[i].StartCluster)
			break;

		Str.Format(TEXT("%d"), info[i].StartCluster);

		// Insert the item, select every other item.
		m_pList->InsertItem(
			LVIF_TEXT|LVIF_STATE, i, Str, 0, 0, 0, 0);

		Str.Format(TEXT("%d"), info[i].NumberOfClusters);
		m_pList->SetItemText(i, 1, Str);

		Str.Format(TEXT("%d"), info[i].VCN);
		m_pList->SetItemText(i, 2, Str);

		Str.SetString(info[i].FileName);
		m_pList->SetItemText(i, 3, Str);
	}

	Str.Format(TEXT("%d"), m_startCluster);
	CWnd* pWnd = GetDlgItem(IDC_EDIT1);
	pWnd->SetWindowText(Str);
}


void CCInspector::OnBnClickedCiopenfilebtn()
{
	CDiskStatus* Status = NULL;
	m_pDoc->GetStatInfo( Status );

	if ( Status )
	{
		m_currentFileName = Status->Name + m_currentFileName;
	}

	// Create an Open dialog; the default file name extension is ".my".
	CFileDialog fileDlg (TRUE, NULL, m_currentFileName,
		OFN_FILEMUSTEXIST|OFN_HIDEREADONLY, NULL, this);

	// Display the file dialog. When user clicks OK, fileDlg.DoModal() 
	// returns IDOK.
	if( fileDlg.DoModal ()==IDOK )
	{
		CString pathName = fileDlg.GetPathName();
		pathName.Replace(TEXT("\\"), TEXT("\\\\"));

		WinExec(CT2CA(pathName), SW_SHOWNORMAL);
	}
}


void CCInspector::OnLvnItemActivateCilist(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMIA = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	*pResult = 0;

	int index = pNMIA->iItem;
	m_currentFileName = info[index].FileName;

	OnBnClickedCiopenfilebtn();
}


void CCInspector::OnNMClickCilist(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	*pResult = 0;

	int index = pNMItemActivate->iItem;
	m_currentFileName = info[index].FileName;
}
