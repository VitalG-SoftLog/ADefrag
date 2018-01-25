// Report\ReportTop100View.cpp : implementation file
//

#include "stdafx.h"
#include "..\mainfrm.h"
#include "..\FileInfo.h"
#include "ReportTop100View.h"
#include "Reports.h"


// CReportTop100View

IMPLEMENT_DYNCREATE(CReportTop100View, CSSSDHtmlView)

CReportTop100View::CReportTop100View()
{

}

CReportTop100View::~CReportTop100View()
{
}



BEGIN_MESSAGE_MAP(CReportTop100View, CSSSDHtmlView)
END_MESSAGE_MAP()



// CReportTop100View message handlers
void CReportTop100View::UpdateHTML( CStringA& szHTML, LPCTSTR FilePath )
{
    CADefragUIDoc* pDoc = GetDocument();
    const TCHAR* RowTemplate = 
        _T("<tr>")
        _T("<td class=\"Text1\" align=\"center\">%s</td>") // Fragments
        _T("<td class=\"Text1\" align=\"center\">%s bytes</td>") // Size
        _T("<td class=\"Text1\" align=\"left\">%s</td>") // FileName
        _T("</tr>");

	CString top100Files;
	ULARGE_INTEGER longNum;
	if (int Count = UpdateFileInfo()) {
		PDEFRAG_FILE_LIST   FileList = (PDEFRAG_FILE_LIST)Global.ServerBuffer;
		for(int i=0; i < Count; i++, FileList++)
		{
			if (!(FileList->FileName && wcslen(FileList->FileName)))
				continue;

			CFileInfo fileInfo;
			fileInfo.Create(FileList->FileName);

			CString sizeStr;
			longNum.QuadPart = fileInfo.GetLength64();
	        pDoc->SplitTriad( sizeStr, longNum );

			CString extNumStr;
			longNum.QuadPart = FileList->ExtentsNum;
	        pDoc->SplitTriad( extNumStr, longNum );

			CString row;
	        row.Format( RowTemplate, extNumStr, sizeStr, FileList->FileName );
			top100Files += row;
		}
	}

    ReplacePattern( szHTML , _T("@Top100Files@"), top100Files );
}


int CReportTop100View::UpdateFileInfo()
{
	int Result;
	DWORD JobId;
	ULONG OutputLength = Global.BufferLength;

	CADefragUIDoc* pDoc = GetMainDocument();
	if (pDoc) {
		JobId = pDoc->GetLastJobID();
		OutputLength = Global.BufferLength;
		Result = SendCommandEx(&JobId, DEFRAG_CMD_GET_MOST_FRAGMENTED, NULL, NULL, NULL, NULL, &OutputLength, NULL, NULL );

		if ( NO_ERROR == Result ) {
			Result = OutputLength/sizeof(DEFRAG_FILE_LIST);
		} else {
			Result = 0;
		}
	}

	return Result;
}
