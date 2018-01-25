#pragma once

#ifdef _WIN32_WCE
#error "CHtmlView is not supported for Windows CE."
#endif 

#include "..\Ctrls\SSSDHtmlView.h"
#include "Reports.h"

// CReportDriveHealthView html view

class CReportDriveHealthView : public CSSSDHtmlView
{
	DECLARE_DYNCREATE(CReportDriveHealthView)

protected:
	CReportDriveHealthView();           // protected constructor used by dynamic creation
	virtual ~CReportDriveHealthView();

protected:
    virtual CString GetDefaultFileName() { return _T("DriveHealth.htm"); }
    virtual UINT GetTemplateID() { return IDR_DRIVE_HEALTH_REPORT_HTML; }
    virtual void UpdateHTML( CStringA& szHTML, LPCTSTR FilePath );

protected:
	DECLARE_MESSAGE_MAP()
};


