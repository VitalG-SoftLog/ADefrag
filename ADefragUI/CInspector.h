/*
    Defrag CInspector class header

    Module name:

        CInspector.h

    Abstract:

        Defines the CInspector class.

    $Header: /home/CVS_DEFRAG/Defrag/Src/ADefragUI/CInspector.h,v 1.5 2009/12/18 00:16:19 roman Exp $
    $Log: CInspector.h,v $
    Revision 1.5  2009/12/18 00:16:19  roman
    CInspector multiple fragments view, Exclude list remove operations.

    Revision 1.4  2009/12/14 10:27:42  dimas
    CVS headers included


*/

#pragma once
#include "DriveMap.h"
#include "afxcmn.h"


// CCInspector dialog
#define COLUMN1WIDTH        80
#define COLUMN2WIDTH        110
#define COLUMN3WIDTH        100
#define COLUMN4WIDTH        250

class CCInspector : public CDialog
{
	DECLARE_DYNAMIC(CCInspector)

public:
	CCInspector(CWnd* pParent = NULL);   // standard constructor    
	virtual ~CCInspector();

// Dialog Data
	enum { IDD = IDD_CINSPECTOR };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	void UpdateDataList(); 

	DECLARE_MESSAGE_MAP()

public:
    virtual BOOL OnInitDialog();
    void SetDocument( CADefragUIDoc *Doc );
    inline void SetInitialBlock( int block, int count = 1 ) {
		m_initialBlock = block;
		m_blockCount = count;
	}

protected:
    int					m_initialBlock;
	int					m_blockCount;
	ULONGLONG			m_startCluster;
	CString				m_currentFileName;

    CADefragUIDoc        *m_pDoc;

private:
	std::vector<DEFRAG_CINSPECTOR_INFO> info;

public:
	afx_msg void OnDeltaposSpin1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnEnChangeEdit1();
	afx_msg void OnBnClickedCiupdatebtn();
	afx_msg void OnBnClickedCiopenfilebtn();
//	afx_msg void OnLvnItemchangedCilist(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnItemActivateCilist(NMHDR *pNMHDR, LRESULT *pResult);
//	afx_msg void OnLvnItemchangedCilist(NMHDR *pNMHDR, LRESULT *pResult);
//	afx_msg void OnHdnItemclickCilist(NMHDR *pNMHDR, LRESULT *pResult);
//	afx_msg void OnLvnColumnclickCilist(NMHDR *pNMHDR, LRESULT *pResult);
//	afx_msg void OnLvnLinkClickedCilist(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickCilist(NMHDR *pNMHDR, LRESULT *pResult);
};
