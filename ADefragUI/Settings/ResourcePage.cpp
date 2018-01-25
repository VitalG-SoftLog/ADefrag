/*
    Implementation of the CResourcePage class

    Module name:

    ResourcePage.h

    Abstract:

    Implementation of the CResourcePage class

    $Header: /home/CVS_DEFRAG/Defrag/Src/ADefragUI/Settings/ResourcePage.cpp,v 1.2 2009/11/24 18:02:52 roman Exp $
    $Log: ResourcePage.cpp,v $
    Revision 1.2  2009/11/24 18:02:52  roman
    Initial commit

    Revision 1.1  2009/11/24 14:56:45  dimas
    no message

*/

#include "stdafx.h"
#include "ResourcePage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CResourcePage property page

IMPLEMENT_DYNCREATE(CResourcePage, CMFCPropertyPage)

CResourcePage::CResourcePage() : CMFCPropertyPage(CResourcePage::IDD)
{
}

CResourcePage::~CResourcePage()
{
}

void CResourcePage::DoDataExchange(CDataExchange* pDX)
{
    CMFCPropertyPage::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CResourcePage, CMFCPropertyPage)
    //{{AFX_MSG_MAP(CResourcePage)
    ON_BN_CLICKED(IDC_HOME, OnHome)
    ON_BN_CLICKED(IDC_CONTACT, OnContact)
    ON_BN_CLICKED(IDC_PRODUCT, OnProduct)
    ON_BN_CLICKED(IDC_SUPPORT, OnSupport)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CResourcePage message handlers

void CResourcePage::OnHome()
{
    OpenAddress( IDS_RESOURCE_HOME );
}

void CResourcePage::OnContact()
{
    OpenAddress( IDS_RESOURCE_CONTACT );
}

void CResourcePage::OnProduct()
{
    OpenAddress( IDS_RESOURCE_PRODUCT );
}

void CResourcePage::OnSupport()
{
    OpenAddress( IDS_RESOURCE_SUPPORT );
}

void CResourcePage::OpenAddress( UINT addressId )
{
    CString address;
    address.LoadString( addressId );
    if( (HINSTANCE)32 >= ::ShellExecute( m_hWnd, NULL, address, NULL, NULL, NULL ) )
    {
        CString message;
        message.Format( IDS_RESOURCE_OPEN_FAILED, address );
        AfxMessageBox( message, MB_OK | MB_ICONEXCLAMATION );
    }
}

