#pragma once

#ifdef _WIN32_WCE
#error "CHtmlView is not supported for Windows CE."
#endif 

#include "..\Ctrls\SSSDHtmlView.h"
#include "Reports.h"

// CReportFileOrganizationView html view

class CReportFileOrganizationView : public CSSSDHtmlView
{
	DECLARE_DYNCREATE(CReportFileOrganizationView)

protected:
	CReportFileOrganizationView();           // protected constructor used by dynamic creation
	virtual ~CReportFileOrganizationView();

protected:
    virtual CString GetDefaultFileName() { return _T("FileOrganization.htm"); }
    virtual UINT GetTemplateID() { return IDR_ORGANIZATION_REPORT_HTML; }
    virtual void UpdateHTML( CStringA& szHTML, LPCTSTR FilePath );

protected:
	DECLARE_MESSAGE_MAP()
};


