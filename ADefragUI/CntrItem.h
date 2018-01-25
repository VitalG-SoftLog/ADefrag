/*
    Defrag CADefragUICntrItem class interface header

    Module name:

        CntrItem.h

    Abstract:

        Defines the interface of the CADefragUICntrItem class.

    $Header: /home/CVS_DEFRAG/Defrag/Src/ADefragUI/CntrItem.h,v 1.3 2009/12/03 15:36:55 dimas Exp $
    $Log: CntrItem.h,v $
    Revision 1.3  2009/12/03 15:36:55  dimas
    CVS headers included

*/


#pragma once

class CADefragUIDoc;
class CADefragUIView;

class CADefragUICntrItem : public COleClientItem
{
    DECLARE_SERIAL(CADefragUICntrItem)

// Constructors
public:
    CADefragUICntrItem(CADefragUIDoc* pContainer = NULL);
        // Note: pContainer is allowed to be NULL to enable IMPLEMENT_SERIALIZE
        //  IMPLEMENT_SERIALIZE requires the class have a constructor with
        //  zero arguments.  Normally, OLE items are constructed with a
        //  non-NULL document pointer

// Attributes
public:
    CADefragUIDoc* GetDocument()
        { return reinterpret_cast<CADefragUIDoc*>(COleClientItem::GetDocument()); }
    CADefragUIView* GetActiveView()
        { return reinterpret_cast<CADefragUIView*>(COleClientItem::GetActiveView()); }

    public:
    virtual void OnChange(OLE_NOTIFICATION wNotification, DWORD dwParam);
    virtual void OnActivate();
    protected:
    virtual void OnGetItemPosition(CRect& rPosition);
    virtual void OnDeactivateUI(BOOL bUndoable);
    virtual BOOL OnChangeItemPosition(const CRect& rectPos);

// Implementation
public:
    ~CADefragUICntrItem();
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif
    virtual void Serialize(CArchive& ar);
};

