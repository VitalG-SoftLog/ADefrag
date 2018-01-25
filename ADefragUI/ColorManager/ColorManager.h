/*
    Defrag Sheduler

    Module name:

        ColorManager.h

    Abstract:

        Work with GUI color scheme
        Work with HSV color model and transformation
        between HSV and RGB color model        

    $Header: /home/CVS_DEFRAG/Defrag/Src/ADefragUI/ColorManager/ColorManager.h,v 1.2 2009/11/24 18:02:52 roman Exp $
    $Log: ColorManager.h,v $
    Revision 1.2  2009/11/24 18:02:52  roman
    Initial commit

    Revision 1.1  2009/11/24 14:58:47  dimas
    no message

*/
#ifndef __COLORMANAGER_H__
#define __COLORMANAGER_H__

//
//
//
typedef struct _ahsv_st_
{
    BYTE Alpha;
    float Hue;
    float Saturation;
    float Value;
} AHSV, *LPAHSV;

//
//
//
class CSSSColorManager
{
public :
    enum Color
    {
        BGColor = 0,
        FontColor
    };

    CSSSColorManager();
    ~CSSSColorManager();

    void OnApplicationLook();


    COLORREF ARGBtoCOLORREF( Gdiplus::ARGB argb );
    Gdiplus::ARGB COLORREFtoARGB( COLORREF colorref );

    AHSV RGBtoHSV( Gdiplus::ARGB rgb );
    Gdiplus::ARGB HSVtoRGB( AHSV hsv );

    
    AHSV HSVChangeValue(AHSV &hsv, int Value);

    COLORREF GetSchemeColor(Color color);

protected :
    CMFCVisualManagerOffice2007::Style m_CurrentStyle;
};

#endif //__COLORMANAGER_H__