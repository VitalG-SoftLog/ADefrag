/*
    Defrag CDriveMapView header

    Module name:

        DriveMapView.h

    Abstract:

        Defines the CDriveMapView class.

    $Header: /home/CVS_DEFRAG/Defrag/Src/ADefragUI/DriveMapView.h,v 1.5 2009/12/15 18:50:46 roman Exp $
    $Log: DriveMapView.h,v $
    Revision 1.5  2009/12/15 18:50:46  roman
    Fix bug with start/end cluster calculation from block number for CInspector.

    Revision 1.4  2009/12/14 10:27:42  dimas
    CVS headers included


*/

#pragma once

#include "ADefragUIDoc.h"
#include "DrvMapWnd.h"

#define DEFAULT_BLOCKWIDTH          10
#define DEFAULT_BLOCKHEIGHT         10

#define BLOCKBORDERCOLOR            0xFF808080

#define TOP_MARGIN                  5
#define LEFT_MARGIN                 5
#define RIGHT_MARGIN                5
#define BOTTOM_MARGIN               45

#define TOP_HEADER                  29
#define TOP_SUBMARGIN               3
#define LEFT_SUBMARGIN              3
#define RIGHT_SUBMARGIN             3
#define BOTTOM_SUBMARGIN            0

#define IDC_COMBOBOX                100
#define IDC_PROGRESS                101

#define COMBOLEFT                   33
#define COMBOTOP                    3

#define COMBOWIDTH                  150
#define COMBOHEIGHT                 100

#define PROGRESSTOP                 25
#define PROGRESSHEIGHT              10
#define PROGRESSBARCOLOR            0x00FF00

#define LEGENDROW                   2
#define LEGENDFIELDWIDTH            110
#define LEGENDFIELDHEIGHT           16
#define LEGENDINDENTBOTTOM          38
#define LEGENDLEFTMARGIN            8
#define LEGENDTEXTINDENT            15

#define MAP_BLOCK                   1
#define LEGEND_BLOCK                2

#define INVALID_STATEID             200


/////////////////////////////////////////////////////////////////////////////
// CDriveMapView view

class CDriveMapView : public CView
{
    friend CDrvMapWnd;
protected:
	CDriveMapView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CDriveMapView)

    //
    // Class constants
    //

    //
    // Timer constants
    //
    static const UINT_PTR IDTTimer              = 400;
    static const UINT TimerPeriod               = 300;
    //
    // Legend block defaults
    //
    static const BYTE LegendDefaultType         = LegendTypeDrvmap;
    static const BOOL LegendDefaultShow         = TRUE;
    static const int LegendDefaultBlockWidth    = 10;
    static const int LegendDefaultBlockHeight   = 10;
    //
    // Legend position & margins
    //

// Attributes
public:
    //
    // Block drawing type
    //
    enum {
        BlockView1,
        BlockView2
    };

// Operations
public:    
// Overrides
	public:
	virtual void OnInitialUpdate();
    virtual BOOL PreTranslateMessage(MSG* pMsg);
	//virtual CScrollBar* GetScrollBarCtrl(int nBar) const;

    BOOL GetShowLegend();
    void SetShowLegend( BOOL Value );
    void SetDefaultColor();
    int GetBlockWidth();
    int GetBlockHeight();
    void SetBlockWidth( int Value );
    void SetBlockHeight( int Value );
    
    HBITMAP DrawBlockToBmp(HDC dc, BYTE State, BYTE Type);

	protected:
    void DrawBlock(Graphics& g, int x, int y, BYTE State, BYTE Type);
    
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view

// Implementation
protected:
	virtual ~CDriveMapView();
    CADefragUIDoc* GetDocument(void);
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
protected:
    int m_MapLeft;
    int m_MapTop;
    CMFCToolTipCtrl m_ToolTip;
    POINT           m_PrevBlock;
    CComboBox       m_ComboBox;
    CProgressCtrl   m_Progress;
    //CSSSProgressBar m_Progress;
    BOOL            m_ProgressVisible;

	CString			strCurrentFileName;


    BOOL            m_EnableCInspectorMI;
    BOOL            m_EnableColorMI;
    CPoint          m_ClientPoint;

    int             m_CurrentLegend;    
    BOOL            m_ShowLegend;
    //Gdiplus::ARGB   m_legendColors[ BLOCKTYPES_COUNT ][ LEGENDCOUNT ][ BLOCK_COLORS ];
    GUI_LEGEND_ITEM     m_legendColors[ BlockStateNewCount ];

    CString         strSection;
    CString         strLegend1Item;
    CString         strLegend2Item;
    CString         strBlockWidthItem;
    CString         strBlockHeightItem;
    CString         strCurrentLegend;
    CString         strShowLegend;

	CDrvMapWnd      m_DrvMap;
	// Test
	//HBITMAP			m_hBmp;

    int GetMapColCount( void );
    int GetMapRowCount( void );
	int GetMapTotalBlockCount( void );
    int GetBlockIndexInLegend( CPoint point );
    CString GetBlockLegend( BYTE State );
    CString GetBlockLegend( int x, int y );
    BOOL IsMapRegion( int x, int y );
    BOOL IsMapRegion( CPoint point );
    int GetLegendItemIndex( CPoint point );
    void SetBlockColor( int Index, COLORREF color );
    void LoadLegendOptions();
    void SaveLegendOptions();
    BYTE GetLegendNum();
    int GetIndexByOrder( BYTE order );
    int GetStateByOrder( BYTE order );
    void DeleteAllBitmap();
    void DialogBlockColor( int Index );

private:
    int FBlockWidth, FBlockHeight;    

protected:
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnWindowPosChanging(WINDOWPOS FAR* lpwndpos);
	//afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnLButtonDblClk(UINT, CPoint);
    afx_msg void OnTimer(UINT_PTR NIDEvent);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);	
    afx_msg void OnComboBox();

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnSize(UINT nType, int cx, int cy);
protected:
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
public:
    afx_msg void OnUpdateDrivemapviewpopupClasterinspector(CCmdUI *pCmdUI);
    afx_msg void OnUpdateDrivemapviewpopupCustomizeblock(CCmdUI *pCmdUI);
    afx_msg void OnDrivemapviewpopupClasterinspector();
    afx_msg void OnDrivemapviewpopupCustomizeblock();      
};
