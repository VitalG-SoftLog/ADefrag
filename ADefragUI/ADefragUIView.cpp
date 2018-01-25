/*
    Defrag CADefragUIView class module 

    Module name:

        ADefragUIView.cpp

    Abstract:

        Contains implementation of the CADefragUIView class.

    $Header: /home/CVS_DEFRAG/Defrag/Src/ADefragUI/ADefragUIView.cpp,v 1.3 2009/12/03 15:36:55 dimas Exp $
    $Log: ADefragUIView.cpp,v $
    Revision 1.3  2009/12/03 15:36:55  dimas
    CVS headers included

*/

#include "stdafx.h"
#include "ADefragUI.h"

#include "ADefragUIDoc.h"
#include "CntrItem.h"
#include "ADefragUIView.h"
#include "DriveMapView.h"
#include "Reports\ReportsView.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CADefragUIView

IMPLEMENT_DYNCREATE(CADefragUIView, CTabView)

BEGIN_MESSAGE_MAP(CADefragUIView, CTabView)
    ON_WM_DESTROY()
    ON_WM_CONTEXTMENU()
    ON_WM_RBUTTONUP()
    ON_WM_SETFOCUS()
    ON_WM_SIZE()
    ON_WM_CREATE()
    ON_WM_ERASEBKGND()
    ON_COMMAND(ID_OLE_INSERT_NEW, &CADefragUIView::OnInsertObject)
    ON_COMMAND(ID_CANCEL_EDIT_CNTR, &CADefragUIView::OnCancelEditCntr)
    ON_COMMAND(ID_FILE_PRINT, &CADefragUIView::OnFilePrint)
    ON_COMMAND(ID_FILE_PRINT_DIRECT, &CTabView::OnFilePrint)
    ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CADefragUIView::OnFilePrintPreview)
END_MESSAGE_MAP()

// CADefragUIView construction/destruction

CADefragUIView::CADefragUIView()
{
    m_pSelection = NULL;
    // TODO: add construction code here

}

CADefragUIView::~CADefragUIView()
{
}

BOOL CADefragUIView::PreCreateWindow(CREATESTRUCT& cs)
{
    // TODO: Modify the Window class or styles here by modifying
    //  the CREATESTRUCT cs

    return CTabView::PreCreateWindow(cs);
}

// CADefragUIView drawing

void CADefragUIView::OnDraw(CDC* pDC)
{
    if (!pDC)
        return;

    CADefragUIDoc* pDoc = GetDocument();
    ASSERT_VALID(pDoc);
    if (!pDoc)
        return;

    // TODO: add draw code for native data here
    // TODO: also draw all OLE items in the document

    // Draw the selection at an arbitrary position.  This code should be
    //  removed once your real drawing code is implemented.  This position
    //  corresponds exactly to the rectangle returned by CADefragUICntrItem,
    //  to give the effect of in-place editing.

    // TODO: remove this code when final draw code is complete.
    if (m_pSelection != NULL)
    {
        CSize size;
        CRect rect(10, 10, 210, 210);
        
        if (m_pSelection->GetExtent(&size, m_pSelection->m_nDrawAspect))
        {
            pDC->HIMETRICtoLP(&size);
            rect.right = size.cx + 10;
            rect.bottom = size.cy + 10;
        }
        m_pSelection->Draw(pDC, rect);
    }
}

void CADefragUIView::OnInitialUpdate()
{
    CTabView::OnInitialUpdate();

    // TODO: remove this code when final selection model code is written
    m_pSelection = NULL;    // initialize selection

}


// CADefragUIView printing


void CADefragUIView::OnFilePrintPreview()
{
    AFXPrintPreview(this);
}

BOOL CADefragUIView::OnPreparePrinting(CPrintInfo* pInfo)
{
    // default preparation
    return DoPreparePrinting(pInfo);
}

void CADefragUIView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
    // TODO: add extra initialization before printing
}

void CADefragUIView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
    // TODO: add cleanup after printing
}

void CADefragUIView::OnDestroy()
{
    // Deactivate the item on destruction; this is important
    // when a splitter view is being used
   COleClientItem* pActiveItem = GetDocument()->GetInPlaceActiveItem(this);
   if (pActiveItem != NULL && pActiveItem->GetActiveView() == this)
   {
      pActiveItem->Deactivate();
      ASSERT(GetDocument()->GetInPlaceActiveItem(this) == NULL);
   }
   CTabView::OnDestroy();
}



// OLE Client support and commands

BOOL CADefragUIView::IsSelected(const CObject* pDocItem) const
{
    // The implementation below is adequate if your selection consists of
    //  only CADefragUICntrItem objects.  To handle different selection
    //  mechanisms, the implementation here should be replaced

    // TODO: implement this function that tests for a selected OLE client item

    return pDocItem == m_pSelection;
}

void CADefragUIView::OnInsertObject()
{
    // Invoke the standard Insert Object dialog box to obtain information
    //  for new CADefragUICntrItem object
    COleInsertDialog dlg;
    if (dlg.DoModal() != IDOK)
        return;

    BeginWaitCursor();

    CADefragUICntrItem* pItem = NULL;
    TRY
    {
        // Create new item connected to this document
        CADefragUIDoc* pDoc = GetDocument();
        ASSERT_VALID(pDoc);
        pItem = new CADefragUICntrItem(pDoc);
        ASSERT_VALID(pItem);

        // Initialize the item from the dialog data
        if (!dlg.CreateItem(pItem))
            AfxThrowMemoryException();  // any exception will do
        ASSERT_VALID(pItem);
        
        if (dlg.GetSelectionType() == COleInsertDialog::createNewItem)
            pItem->DoVerb(OLEIVERB_SHOW, this);

        ASSERT_VALID(pItem);
        // As an arbitrary user interface design, this sets the selection
        //  to the last item inserted

        // TODO: reimplement selection as appropriate for your application
        m_pSelection = pItem;   // set selection to last inserted item
        pDoc->UpdateAllViews(NULL);
    }
    CATCH(CException, e)
    {
        if (pItem != NULL)
        {
            ASSERT_VALID(pItem);
            pItem->Delete();
        }
        AfxMessageBox(IDP_FAILED_TO_CREATE);
    }
    END_CATCH

    EndWaitCursor();
}

// The following command handler provides the standard keyboard
//  user interface to cancel an in-place editing session.  Here,
//  the container (not the server) causes the deactivation
void CADefragUIView::OnCancelEditCntr()
{
    // Close any in-place active item on this view.
    COleClientItem* pActiveItem = GetDocument()->GetInPlaceActiveItem(this);
    if (pActiveItem != NULL)
    {
        pActiveItem->Close();
    }
    ASSERT(GetDocument()->GetInPlaceActiveItem(this) == NULL);
}

// Special handling of OnSetFocus and OnSize are required for a container
//  when an object is being edited in-place
void CADefragUIView::OnSetFocus(CWnd* pOldWnd)
{
    COleClientItem* pActiveItem = GetDocument()->GetInPlaceActiveItem(this);
    if (pActiveItem != NULL &&
        pActiveItem->GetItemState() == COleClientItem::activeUIState)
    {
        // need to set focus to this item if it is in the same view
        CWnd* pWnd = pActiveItem->GetInPlaceWindow();
        if (pWnd != NULL)
        {
            pWnd->SetFocus();   // don't call the base class
            return;
        }
    }

    CTabView::OnSetFocus(pOldWnd);
}

void CADefragUIView::OnSize(UINT nType, int cx, int cy)
{
    CTabView::OnSize(nType, cx, cy);
    COleClientItem* pActiveItem = GetDocument()->GetInPlaceActiveItem(this);
    if (pActiveItem != NULL)
        pActiveItem->SetItemRects();
}

void CADefragUIView::OnFilePrint()
{
    //By default, we ask the Active document to print itself
    //using IOleCommandTarget. If you don't want this behavior
    //remove the call to COleDocObjectItem::DoDefaultPrinting.
    //If the call fails for some reason, we will try printing
    //the docobject using the IPrint interface.
    CPrintInfo printInfo;
    ASSERT(printInfo.m_pPD != NULL); 
    if (S_OK == COleDocObjectItem::DoDefaultPrinting(this, &printInfo))
        return;
    
    CTabView::OnFilePrint();

}


void CADefragUIView::OnRButtonUp(UINT nFlags, CPoint point)
{
    ClientToScreen(&point);
    OnContextMenu(this, point);
}

void CADefragUIView::OnContextMenu(CWnd* pWnd, CPoint point)
{
    //theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
}

int CADefragUIView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
    if (__super::OnCreate(lpCreateStruct) == -1)
        return -1;
   
    AddView( RUNTIME_CLASS( CDriveMapView ), _T("Drive Map "), 100 );   
    AddView( RUNTIME_CLASS( CReportsView ), _T("Reports "), 101 );
    //AddView( RUNTIME_CLASS( CReportsLogView ), _T("Reports log "), 102 );

    return 0;
}

BOOL CADefragUIView::OnEraseBkgnd(CDC* /*pDC*/) 
{
    return TRUE;
}

// CADefragUIView diagnostics

#ifdef _DEBUG
void CADefragUIView::AssertValid() const
{
    CTabView::AssertValid();
}

void CADefragUIView::Dump(CDumpContext& dc) const
{
    CTabView::Dump(dc);
}

CADefragUIDoc* CADefragUIView::GetDocument() const // non-debug version is inline
{
    ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CADefragUIDoc)));
    return (CADefragUIDoc*)m_pDocument;
}
#endif //_DEBUG


// CADefragUIView message handlers
