#pragma once

#ifdef _WIN32_WCE
#error "CHtmlView is not supported for Windows CE."
#endif 

#include "..\Ctrls\SSSDHtmlView.h"
#include "Reports.h"

// CReportTop100View html view

class CReportTop100View : public CSSSDHtmlView
{
	DECLARE_DYNCREATE(CReportTop100View)

protected:
	CReportTop100View();           // protected constructor used by dynamic creation
	virtual ~CReportTop100View();

	int UpdateFileInfo();

protected:
    virtual CString GetDefaultFileName() { return _T("Top100.htm"); }
    virtual UINT GetTemplateID() { return IDR_TOP100_REPORT_HTML; }
    virtual void UpdateHTML( CStringA& szHTML, LPCTSTR FilePath );

protected:
	DECLARE_MESSAGE_MAP()
};


