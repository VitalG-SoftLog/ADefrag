#pragma once

#ifdef _WIN32_WCE
#error "CHtmlView is not supported for Windows CE."
#endif 

#include "..\ADefragUIDoc.h"

// CSSSDHtmlView html view


class CSSSDHtmlView : public CHtmlView
{
	DECLARE_DYNCREATE(CSSSDHtmlView)

protected:
	CSSSDHtmlView();           // protected constructor used by dynamic creation
    virtual ~CSSSDHtmlView();

public:    
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif    

    int SetHtmlElementValue(CString sId, CString sValue);
    CADefragUIDoc* GetDocument(void);
    virtual CString CreateReport( CString PathToSave = _T("") );

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    CStringA LoadHtmlResource( UINT nRes );
    BOOL ReplacePattern( CStringA& Src, LPCTSTR Pattern, LPCTSTR Value, BOOL IsUnknown = FALSE );
    BOOL ReplacePattern( CStringA& Src, LPCTSTR Pattern, DWORD Value, BOOL IsUnknown = FALSE );
    BOOL ReplacePattern( CStringA& Src, LPCTSTR Pattern, float Value, BOOL IsUnknown = FALSE );
    BOOL ReplacePattern( CStringA& Src, LPCTSTR Pattern, ULARGE_INTEGER Value, BOOL IsUnknown = FALSE );
    BOOL ReplacePattern( CStringA& Src, LPCTSTR Pattern, LARGE_INTEGER Value, BOOL IsUnknown = FALSE );

    virtual CString GetDefaultFileName() { return _T("report.htm"); }
    virtual UINT GetTemplateID() { return 0; }
    virtual void UpdateHTML( CStringA& szHTML, LPCTSTR FilePath ) {}

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnReportSave();
    afx_msg void OnReportPrint();
    afx_msg void OnFilePrintPreview();
    virtual void OnInitialUpdate();
};


