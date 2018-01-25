#pragma once

#ifdef _WIN32_WCE
#error "CHtmlView is not supported for Windows CE."
#endif 

#include "..\Ctrls\SSSDHtmlView.h"
#include "Reports.h"

// CReportFragmentedView html view

class CReportFragmentedView : public CSSSDHtmlView
{
	DECLARE_DYNCREATE(CReportFragmentedView)

protected:
	CReportFragmentedView();           // protected constructor used by dynamic creation
	virtual ~CReportFragmentedView();

    void DrawDiagramToBmp( CString PathToSave, ULARGE_INTEGER totalSize, LARGE_INTEGER fragmentedSize );

    static const int DrivPicHeight          = 60;
    static const int DrivPicWidth           = 150;
    static const int DrivPicDY              = 10;
    static const int LegendBlockWidth       = 10;
    static const int LegendBlockHeight      = 10;

protected:
    virtual CString GetDefaultFileName() { return _T("fragmentation.htm"); }
    virtual UINT GetTemplateID() { return IDR_FRAGMENTATION_REPORT_HTML; }
    virtual void UpdateHTML( CStringA& szHTML, LPCTSTR FilePath );

protected:
	DECLARE_MESSAGE_MAP()
};