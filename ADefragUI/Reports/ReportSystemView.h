#pragma once

#ifdef _WIN32_WCE
#error "CHtmlView is not supported for Windows CE."
#endif 

#include "..\Ctrls\SSSDHtmlView.h"
#include "Reports.h"

// CReportSystemView html view

class CReportSystemView : public CSSSDHtmlView
{
	DECLARE_DYNCREATE(CReportSystemView)

protected:
	CReportSystemView();           // protected constructor used by dynamic creation
	virtual ~CReportSystemView();

    void DrawDiagramToBmp( CString PathToSave = _T("") );

    static const int DrivPicHeight          = 60;
    static const int DrivPicWidth           = 150;
    static const int DrivPicDY              = 10;
    static const int LegendBlockWidth       = 10;
    static const int LegendBlockHeight      = 10;

protected:
    virtual CString GetDefaultFileName() { return _T("system.htm"); }
    virtual UINT GetTemplateID() { return IDR_SYSTEM_REPORT_HTML; }
    virtual void UpdateHTML( CStringA& szHTML, LPCTSTR FilePath );


protected:

	DECLARE_MESSAGE_MAP()
};


