/*
    Defrag CTaskPane header

    Module name:

        TaskPane.h

    Abstract:

        Defines the CTaskPane class.

    $Header: /home/CVS_DEFRAG/Defrag/Src/ADefragUI/TaskPane.h,v 1.5 2010/01/12 22:53:18 roman Exp $
    $Log: TaskPane.h,v $
    Revision 1.5  2010/01/12 22:53:18  roman
    Add "Recommended Defrag Method..." button

    Revision 1.4  2009/12/24 01:39:10  roman
    fix errors

    Revision 1.3  2009/12/03 15:36:55  dimas
    CVS headers included

*/

#pragma once

class DriveHealth;

class CTaskPane : public CMFCTasksPane
{
    friend class CTasksPaneView;

    // Construction
public:
    CTaskPane();

    // Operations
public:
    void UpdateToolbar ();
    void UpdateResultAndReliabilityPage( DriveHealth* health, BOOL activate );
	void UpdateDriveHealth( DriveHealth* health );

    // Overrides
public:

protected:
    BOOL CreateTreeControl();
    BOOL CreateEditControl();

    // Implementation
public:
    virtual ~CTaskPane();

	int GetOveralHealthLevel() {
		return m_OveralHealthLevel;
	}

protected:
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    DECLARE_MESSAGE_MAP()

protected:
    int m_nResultPage;
	int m_InfoGroup;
	int m_DescriptionTask;
	int m_FGMDescriptionTask;

	int m_RecomendGroup;
	int m_ReliabilityGroup;

	int m_MFTDesc;
	int m_MFTReccomend;
	int m_PageFileDesc;
	int m_PageFileReccomend;
	int m_FreeDesc;
	int m_FreeReccomend;

	int m_OveralHealthLevel;
};

