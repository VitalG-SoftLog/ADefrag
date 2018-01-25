// Report\ReportExcludedFilesView.cpp : implementation file
//

#include "stdafx.h"
#include "..\mainfrm.h"
#include "..\Settings\ExcludeListOptionsPage.h"
#include "ReportExcludedFilesView.h"
#include "Reports.h"


const WCHAR *ReasonTable[] = {
	L"Excluded",
	L"Unmovable",
	L"Locked",
	L"RootDir",
	L"UnknownReason"
};


// CReportExcludedFilesView

IMPLEMENT_DYNCREATE(CReportExcludedFilesView, CSSSDHtmlView)

CReportExcludedFilesView::CReportExcludedFilesView()
{

}

CReportExcludedFilesView::~CReportExcludedFilesView()
{
}



BEGIN_MESSAGE_MAP(CReportExcludedFilesView, CSSSDHtmlView)
END_MESSAGE_MAP()



// CReportExcludedFilesView message handlers
void CReportExcludedFilesView::UpdateHTML( CStringA& szHTML, LPCTSTR FilePath )
{
	CString excludedFiles;
	const TCHAR* RowTemplate = 
		_T("<tr>")
		_T("<td class=\"Text1\" style=\"text-align:left;\">%s</td>") // FileName
		_T("<td class=\"Text1\">%I64d</td>") // Fragments
		_T("<td class=\"Text1\">%s</td>") // Reason
		_T("</tr>");

	CString row;
	if (int Count = UpdateFileInfo()) {
		PDEFRAG_FILE_LIST   FileList = (PDEFRAG_FILE_LIST)Global.ServerBuffer;
		for(int i=0; i < Count; i++, FileList++)
		{
			if (!(FileList->FileName && wcslen(FileList->FileName)))
				continue;

			CString reason = GetReasonStr(FileList->DefragFlags);
			row.Format( RowTemplate, FileList->FileName, FileList->ExtentsNum, (LPCTSTR)reason);
			excludedFiles += row;
		}
	}

	ReplacePattern( szHTML , _T("@ExcludedFiles@"), excludedFiles, FALSE );
}


int CReportExcludedFilesView::UpdateFileArray()
{
	int error = ERROR_SUCCESS;
	int startPos = 0;

	// Read settings from service
	CADefragUIDoc* doc = GetMainDocument();
	if( doc )
	{
		// MULTI_SZ list
		error = doc->GetSettingsValue(ExcludedFilesId,
			m_excludeString.GetBufferSetLength( SERVER_SECTION_SIZE / sizeof(m_excludeString[0]) ),
			SERVER_SECTION_SIZE);
		if (error == ERROR_SUCCESS)
		{
			for( int i = 0; i < m_excludeString.GetLength() - 1; ++i )
			{
				if( m_excludeString.GetAt(i) == 0 )
				{
					m_fileArray.AddDir(m_excludeString.Mid(startPos, m_excludeString.ReverseFind('\\') + 1),
						_T("*.*"),
						TRUE,
						CFileInfoArray::AP_NOSORT,
						FALSE);

					if( m_excludeString[i+1] == 0 ) break;
					m_excludeString.SetAt( i, EXCLUDE_LIST_DELIMETER );
					startPos = i + 1;
				}
			}
		}
	}

	return error;
}


int CReportExcludedFilesView::UpdateFileInfo()
{
	int Result;
	DWORD JobId;
	ULONG OutputLength = Global.BufferLength;

	CADefragUIDoc* pDoc = GetMainDocument();
	if (pDoc) {
		JobId = pDoc->GetLastJobID();
		OutputLength = Global.BufferLength;
		Result = SendCommandEx(&JobId, DEFRAG_CMD_GET_SKIPPED, NULL, NULL, NULL, NULL, &OutputLength, NULL, NULL );

		if ( NO_ERROR == Result ) {
			Result = OutputLength/sizeof(DEFRAG_FILE_LIST);
		} else {
			Result = 0;
		}
	}

	return Result;
}


CString CReportExcludedFilesView::GetReasonStr(ULONG Flag)
{
	CString Reason;
	if (Flag & DEFRAG_FILE_EXCLUDED) {
		Reason = ReasonTable[Excluded];
	}

	if (Flag & DEFRAG_FILE_UNMOVABLE) {
		if (_tcslen(Reason))
			Reason += ";";

		Reason += ReasonTable[Unmovable];
	}

	if (Flag & DEFRAG_FILE_LOCKED) {
		if (_tcslen(Reason))
			Reason += ";";

		Reason += ReasonTable[Locked];
	}

	if (Flag & DEFRAG_FILE_ROOT_DIR) {
		if (_tcslen(Reason))
			Reason += ";";

		Reason += ReasonTable[Root];
	}

	if (!_tcslen(Reason)) {
		Reason += ReasonTable[Unknown];
	}

	return Reason;
}
