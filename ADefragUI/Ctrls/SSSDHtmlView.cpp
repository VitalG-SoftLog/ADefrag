// Ctrls\SSSDHtmlView.cpp : implementation file
//

#include "stdafx.h"
#include "SSSDHtmlView.h"
#include "..\Resource.h"


// CSSSDHtmlView

IMPLEMENT_DYNCREATE(CSSSDHtmlView, CHtmlView)

CSSSDHtmlView::CSSSDHtmlView()
{

}

CSSSDHtmlView::~CSSSDHtmlView()
{
}

void CSSSDHtmlView::DoDataExchange(CDataExchange* pDX)
{
	CHtmlView::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CSSSDHtmlView, CHtmlView)
//    ON_WM_CREATE()
    ON_COMMAND(ID_REPORT_SAVE, &CSSSDHtmlView::OnReportSave)
    ON_COMMAND(ID_REPORT_PRINT, &CSSSDHtmlView::OnReportPrint)
    ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CSSSDHtmlView::OnFilePrintPreview)
END_MESSAGE_MAP()


// CSSSDHtmlView diagnostics

#ifdef _DEBUG
void CSSSDHtmlView::AssertValid() const
{
	CHtmlView::AssertValid();
}

void CSSSDHtmlView::Dump(CDumpContext& dc) const
{
	CHtmlView::Dump(dc);
}
#endif //_DEBUG


// CSSSDHtmlView message handlers


//
//
//
int CSSSDHtmlView::SetHtmlElementValue(CString sId, CString sValue)
{
	LPDISPATCH pDisp = GetHtmlDocument();
	//get HTML document pointer
	IHTMLDocument2* pHtmlDoc;
	HRESULT hr;
	hr = pDisp->QueryInterface(IID_IHTMLDocument2, (void**)&pHtmlDoc);
    if (hr != S_OK)
	{
#ifdef DEBUG_INFO
		AfxMessageBox("Can't get HTMLDocument2");
#endif
		return -1;
	}
	//get HTML document collection
	IHTMLElementCollection* pCollection;
	hr = pHtmlDoc->get_all(&pCollection);    
    if (hr != S_OK)
    {
#ifdef DEBUG_INFO
        AfxMessageBox("Can't get IHTMLElementCollection");
#endif
        pHtmlDoc->Release();
        return -1;
    }
	//get element by id
	LPDISPATCH pElementDisp;
	IHTMLElement* pElement;
	BSTR bstrKey = sId.AllocSysString();
	VARIANT name, index;
	name.vt = VT_BSTR;
	name.bstrVal = bstrKey;
	index.vt = VT_I4;
	index.lVal = 0;

	if(pCollection->item(name, index, (IDispatch**)&pElementDisp) != S_OK
		|| pElementDisp == NULL)
	{
#ifdef DEBUG_INFO
        AfxMessageBox("Can't find HTML item" + sId);
#endif
		SysFreeString(bstrKey);
        pCollection->Release();
		pHtmlDoc->Release();
		return -1;
	}
	hr = pElementDisp->QueryInterface(IID_IHTMLElement, (void**)&pElement);
    if (hr != S_OK)
    {
#ifdef DEBUG_INFO
        AfxMessageBox("Can't get IID_IHTMLElement");
#endif
        pElementDisp->Release();
        pCollection->Release();
        pHtmlDoc->Release();
        return -1;
    }
	
	//set new value for element
	BSTR bstrValue = sValue.AllocSysString();
	if(pElement->put_innerHTML(bstrValue) != S_OK)
	{
#ifdef DEBUG_INFO
		AfxMessageBox("Can't set HTML value");
#endif
	}

	SysFreeString(bstrValue);
	SysFreeString(bstrKey);
    pElement->Release();
    pElementDisp->Release();
    pCollection->Release();
    pHtmlDoc->Release();
	return 0;
}

//
//
//
CADefragUIDoc* CSSSDHtmlView::GetDocument(void)
{
    ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CADefragUIDoc)));
    return (CADefragUIDoc*)m_pDocument;
}

//
//
//
CString CSSSDHtmlView::CreateReport( CString PathToSave )
{
    CStringA szHTML;
    CString FilePath;

    CADefragUIDoc* pDoc = GetDocument();
    if ( PathToSave.GetLength() )
    {
        FilePath = PathToSave;
    }
    else
    {
        FilePath = pDoc->GetReportTmpDir();
        FilePath.Append( _T("\\") + GetDefaultFileName() );
    }

    szHTML = LoadHtmlResource( GetTemplateID() );

    UpdateHTML( szHTML, FilePath );

    DWORD nBytes = 0;
    HANDLE FileHandle = CreateFile( FilePath, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, 0, 0 );        
    WriteFile( FileHandle, szHTML, strlen( szHTML ), &nBytes, NULL );
    CloseHandle( FileHandle );

    if ( !PathToSave.GetLength() )
    {
        Navigate( FilePath );
    }

    return FilePath;
}

void CSSSDHtmlView::OnReportSave()
{
    /*CFileDialog dlgFile( FALSE, _T("htm"), _T("report.htm"),
        OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, _T("ADefrag Report File (*.htm)|*.htm||") );

    if( dlgFile.DoModal() == IDOK && dlgFile.GetPathName().GetLength() > 0 )
    {
        CString filePath = dlgFile.GetPathName();
        CreateReport( filePath );
    }*/
    // get the HTMLDocument
    if (m_pBrowserApp != NULL)
    {
        CComPtr<IDispatch> spDisp;
        m_pBrowserApp->get_Document(&spDisp);
        if (spDisp != NULL)
        {
            // the control will handle all printing UI

            CComQIPtr<IOleCommandTarget> spTarget = spDisp;
            if (spTarget != NULL)
                spTarget->Exec(NULL, OLECMDID_SAVEAS, 0, NULL, NULL);
        }
    }
}

void CSSSDHtmlView::OnReportPrint()
{
    OnFilePrint();
}
//
//
//
void CSSSDHtmlView::OnFilePrintPreview()
{
    // get the HTMLDocument
    if (m_pBrowserApp != NULL)
    {
        CComPtr<IDispatch> spDisp;
        m_pBrowserApp->get_Document(&spDisp);
        if (spDisp != NULL)
        {
            // the control will handle all printing UI

            CComQIPtr<IOleCommandTarget> spTarget = spDisp;
            if (spTarget != NULL)
                spTarget->Exec(NULL, OLECMDID_PRINTPREVIEW, 0, NULL, NULL);
        }
    }
}

//
//
//
CStringA CSSSDHtmlView::LoadHtmlResource( UINT nRes )
{
    HINSTANCE hInstance = AfxGetResourceHandle();
    ASSERT( hInstance != NULL );

    HRSRC hrsHTMLRes = FindResource( hInstance, MAKEINTRESOURCE( nRes/*IDR_SYSTEM_REPORT_HTML*/ ), RT_HTML );
    ASSERT( hrsHTMLRes != NULL );
    if( !hrsHTMLRes ) return CStringA();

    HGLOBAL hglobHTMLRes = LoadResource( hInstance, hrsHTMLRes );
    ASSERT( hglobHTMLRes != NULL );
    if( !hglobHTMLRes ) return CStringA();
    DWORD ResourceSize = SizeofResource( hInstance, hrsHTMLRes );

    char* szHTMLRes = (char *)LockResource( hglobHTMLRes );
    ASSERT( szHTMLRes != NULL );

    CStringA szRetStr;
    memcpy( szRetStr.GetBufferSetLength(ResourceSize), szHTMLRes, ResourceSize );

    FreeResource( hglobHTMLRes );

    return szRetStr;
}

//
//
//
BOOL CSSSDHtmlView::ReplacePattern( CStringA& Src, LPCTSTR Pattern, DWORD Value, BOOL IsUnknown )
{
    CString Str;
    GetDocument()->SplitTriad( Str, Value );

    return ReplacePattern( Src, Pattern, Str, IsUnknown );
}

BOOL CSSSDHtmlView::ReplacePattern( CStringA& Src, LPCTSTR Pattern, float Value, BOOL IsUnknown )
{
    CString Str;
    Str.Format( _T("%.03f"), Value );

    return ReplacePattern( Src, Pattern, Str, IsUnknown );
}
//
BOOL CSSSDHtmlView::ReplacePattern( CStringA& Src, LPCTSTR Pattern, ULARGE_INTEGER Value, BOOL IsUnknown )
{
    CString Str;
    GetDocument()->SplitTriad( Str, Value );

    return ReplacePattern( Src, Pattern, Str, IsUnknown );
}

BOOL CSSSDHtmlView::ReplacePattern( CStringA& Src, LPCTSTR Pattern, LARGE_INTEGER Value, BOOL IsUnknown )
{
    ULARGE_INTEGER ul;
    ul.QuadPart = Value.QuadPart;
    return ReplacePattern( Src, Pattern, ul, IsUnknown );
}

//
//
//
BOOL CSSSDHtmlView::ReplacePattern( CStringA& Src, LPCTSTR Pattern, LPCTSTR Value, BOOL IsUnknown )
{
    CStringA _Value( IsUnknown ? "Unknown" : CStringA(Value) );
    return Src.Replace( CStringA(Pattern), _Value ) > 0;
}

//
//
//
void CSSSDHtmlView::OnInitialUpdate()
{
    CHtmlView::OnInitialUpdate();

    // TODO: Add your specialized code here and/or call the base class
}
