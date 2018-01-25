/*
    Defrag CADefragUICntrItem class module 

    Module name:

        CntrItem.cpp

    Abstract:

        Contains implementation of the CADefragUICntrItem class.

    $Header: /home/CVS_DEFRAG/Defrag/Src/ADefragUI/CntrItem.cpp,v 1.3 2009/12/03 15:36:55 dimas Exp $
    $Log: CntrItem.cpp,v $
    Revision 1.3  2009/12/03 15:36:55  dimas
    CVS headers included

*/


#include "stdafx.h"
#include "ADefragUI.h"

#include "ADefragUIDoc.h"
#include "ADefragUIView.h"
#include "CntrItem.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CADefragUICntrItem implementation

IMPLEMENT_SERIAL(CADefragUICntrItem, COleClientItem, 0)

CADefragUICntrItem::CADefragUICntrItem(CADefragUIDoc* pContainer)
    : COleClientItem(pContainer)
{
    // TODO: add one-time construction code here
}

CADefragUICntrItem::~CADefragUICntrItem()
{
    // TODO: add cleanup code here
}

void CADefragUICntrItem::OnChange(OLE_NOTIFICATION nCode, DWORD dwParam)
{
    ASSERT_VALID(this);

    COleClientItem::OnChange(nCode, dwParam);

    // When an item is being edited (either in-place or fully open)
    //  it sends OnChange notifications for changes in the state of the
    //  item or visual appearance of its content.

    // TODO: invalidate the item by calling UpdateAllViews
    //  (with hints appropriate to your application)

    GetDocument()->UpdateAllViews(NULL);
        // for now just update ALL views/no hints
}

BOOL CADefragUICntrItem::OnChangeItemPosition(const CRect& rectPos)
{
    ASSERT_VALID(this);

    // During in-place activation CADefragUICntrItem::OnChangeItemPosition
    //  is called by the server to change the position of the in-place
    //  window.  Usually, this is a result of the data in the server
    //  document changing such that the extent has changed or as a result
    //  of in-place resizing.
    //
    // The default here is to call the base class, which will call
    //  COleClientItem::SetItemRects to move the item
    //  to the new position.

    if (!COleClientItem::OnChangeItemPosition(rectPos))
        return FALSE;

    // TODO: update any cache you may have of the item's rectangle/extent

    return TRUE;
}

void CADefragUICntrItem::OnGetItemPosition(CRect& rPosition)
{
    ASSERT_VALID(this);

    // During in-place activation, CADefragUICntrItem::OnGetItemPosition
    //  will be called to determine the location of this item.  Usually, this 
    //  rectangle would reflect the current position of the item relative to the 
    //  view used for activation.  You can obtain the view by calling 
    //  CADefragUICntrItem::GetActiveView.

    // TODO: return correct rectangle (in pixels) in rPosition

    CSize size;
    rPosition.SetRectEmpty();
    if (GetExtent(&size, m_nDrawAspect))
    {
        CADefragUIView* pView = GetActiveView();
        ASSERT_VALID(pView);
        if (!pView)
            return;
        CDC *pDC = pView->GetDC();
        ASSERT(pDC);
        if (!pDC)
            return;
        pDC->HIMETRICtoLP(&size);
        rPosition.SetRect(10, 10, size.cx + 10, size.cy + 10);
    }
    else
        rPosition.SetRect(10, 10, 210, 210);
}

void CADefragUICntrItem::OnActivate()
{
    // Allow only one inplace activate item per frame
    CADefragUIView* pView = GetActiveView();
    ASSERT_VALID(pView);
    if (!pView)
        return;
    COleClientItem* pItem = GetDocument()->GetInPlaceActiveItem(pView);
    if (pItem != NULL && pItem != this)
        pItem->Close();
    
    COleClientItem::OnActivate();
}

void CADefragUICntrItem::OnDeactivateUI(BOOL bUndoable)
{
    COleClientItem::OnDeactivateUI(bUndoable);

    DWORD dwMisc = 0;
    m_lpObject->GetMiscStatus(GetDrawAspect(), &dwMisc);
    if (dwMisc & OLEMISC_INSIDEOUT)
        DoVerb(OLEIVERB_HIDE, NULL);
}

void CADefragUICntrItem::Serialize(CArchive& ar)
{
    ASSERT_VALID(this);

    // Call base class first to read in COleClientItem data.
    // Since this sets up the m_pDocument pointer returned from
    //  CADefragUICntrItem::GetDocument, it is a good idea to call
    //  the base class Serialize first.
    COleClientItem::Serialize(ar);

    // now store/retrieve data specific to CADefragUICntrItem
    if (ar.IsStoring())
    {
        // TODO: add storing code here
    }
    else
    {
        // TODO: add loading code here
    }
}


// CADefragUICntrItem diagnostics

#ifdef _DEBUG
void CADefragUICntrItem::AssertValid() const
{
    COleClientItem::AssertValid();
}

void CADefragUICntrItem::Dump(CDumpContext& dc) const
{
    COleClientItem::Dump(dc);
}
#endif

