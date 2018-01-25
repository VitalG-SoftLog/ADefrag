#pragma once

#ifdef _WIN32_WCE
#error "CHtmlView is not supported for Windows CE."
#endif 

#include "..\Ctrls\SSSDHtmlView.h"
#include "..\FileInfo.h"
#include "Reports.h"

// CReportExcludedFilesView html view

class CReportExcludedFilesView : public CSSSDHtmlView
{
	DECLARE_DYNCREATE(CReportExcludedFilesView)

	enum ExcludeType {
		Excluded,
		Unmovable,
		Locked,
		Root,
		Unknown
	};

protected:
	CReportExcludedFilesView();           // protected constructor used by dynamic creation
	virtual ~CReportExcludedFilesView();

protected:
    virtual CString GetDefaultFileName() { return _T("ExcludedFiles.htm"); }
    virtual UINT GetTemplateID() { return IDR_EXCLUDED_FILES_REPORT_HTML; }
    virtual void UpdateHTML( CStringA& szHTML, LPCTSTR FilePath );

	int UpdateFileArray();
	int UpdateFileInfo();

	CString GetReasonStr(ULONG Flag);

protected:
	DECLARE_MESSAGE_MAP()

protected:
    CString m_excludeString;
	CFileInfoArray m_fileArray;
};

#define MAX_EXCLUDED_FILE_COUNT		100



