/*
    Defrag Sheduler

    Module name:

        ColorManager.cpp

    Abstract:

        Work with GUI color scheme
        Work with HSV color model and transformation
        between HSV and RGB color model

    $Header: /home/CVS_DEFRAG/Defrag/Src/ADefragUI/ColorManager/ColorManager.cpp,v 1.2 2009/11/24 18:02:52 roman Exp $
    $Log: ColorManager.cpp,v $
    Revision 1.2  2009/11/24 18:02:52  roman
    Initial commit

    Revision 1.1  2009/11/24 14:58:47  dimas
    no message

*/

#include "stdafx.h"
#include "ColorManager.h"

//
//
//
const BYTE colrefSize       = sizeof(COLORREF) * 8;
const BYTE byteSize         = sizeof(BYTE) * 8;

//
//
//
float MAX( float a1, float a2, float a3 )
{
    if ( (a1 > a2) && (a1 > a3) )
        return a1;
    else
        if ( a2 > a3 )
            return a2;
        else
            return a3;
}

//
//
//
float MIN( float a1, float a2, float a3 )
{
    if ( (a1 < a2) && (a1 < a3) )
        return a1;
    else
        if ( a2 < a3 )
            return a2;
        else
            return a3;
}

//
//
//
CSSSColorManager::CSSSColorManager()
{
}

CSSSColorManager::~CSSSColorManager()
{
}

//
// 
//
void CSSSColorManager::OnApplicationLook()
{
    m_CurrentStyle = CMFCVisualManagerOffice2007::GetStyle();
}

//
// Convert Gdiplus::ARGB value to COLORREF
//
COLORREF CSSSColorManager::ARGBtoCOLORREF( Gdiplus::ARGB argb )
{
    return 
    ((((argb << (colrefSize - byteSize * 1)) >> (colrefSize - byteSize)) << (byteSize * 2)) |   // R
    (((argb << (colrefSize - byteSize * 2)) >> (colrefSize - byteSize)) << byteSize) |          // G
    ((argb << (colrefSize - byteSize * 3)) >> (colrefSize - byteSize)));                        // B    
    
}

//
// Convert COLORREF value to Gdiplus::ARGB
//
Gdiplus::ARGB CSSSColorManager::COLORREFtoARGB( COLORREF colorref )
{
    //rb = (colorref << (colrefSize - byteSize * 1)) >> (colrefSize - byteSize);
    //gb = (colorref << (colrefSize - byteSize * 2)) >> (colrefSize - byteSize);
    //bb = (colorref << (colrefSize - byteSize * 3)) >> (colrefSize - byteSize);


    return ((0xFF << (colrefSize - byteSize)) | ARGBtoCOLORREF( colorref ) );

    //(((colorref << (colrefSize - byteSize * 1)) >> (colrefSize - byteSize)) << (byteSize * 2)) |    // R
    //(((colorref << (colrefSize - byteSize * 2)) >> (colrefSize - byteSize)) << byteSize) |          // G
    //((colorref << (colrefSize - byteSize * 3)) >> (colrefSize - byteSize)));                        // B    
}


//
// Convert color RGB -> HSV
//
AHSV CSSSColorManager::RGBtoHSV( Gdiplus::ARGB rgb )
{    
    // RGB - остновные цвета (0-1),
    // H - цветовой тон (0-360ш), 0ш - красный
    // S - насыщенность (0-1)
    // V - светлота (0-1)

    // MAX - функция определения максимума
    // MIN - функция определения минимума

    // Определение светлоты

    AHSV res;
    res.Alpha  = (rgb << (colrefSize - byteSize * 4)) >> (colrefSize - byteSize);
    float R = (float)((rgb << (colrefSize - byteSize * 3)) >> (colrefSize - byteSize));
    float G = (float)((rgb << (colrefSize - byteSize * 2)) >> (colrefSize - byteSize));
    float B = (float)((rgb << (colrefSize - byteSize * 1)) >> (colrefSize - byteSize));
    

    // Value = MAX( R, G, B )
    res.Value = MAX( R, G, B );
    float min = MIN( R, G, B );
    
    // Hue 
    if ( min == res.Value )
    {        
        res.Hue         = 0;
    }
    else
        if ( (res.Value == R) && (G >= B) )
        {
            res.Hue = 60 * ( (G - B) / (res.Value - min) ) + 0;
        }
        else
            if ( (res.Value == R) && (G < B) )
            {
                res.Hue = 60 * ( (G - B) / (res.Value - min) ) + 360;
            }
            else
                if ( res.Value == G )
                {
                    res.Hue = 60 * ( (B - R) / (res.Value - min) ) + 120;
                }
                else
                    if ( res.Value == B )
                    {
                        res.Hue = 60 * ( (R - G) / (res.Value - min) ) + 240;
       
                    }
    // Saturation
    if ( res.Value == 0 )
    {
        res.Saturation = 0;
    }
    else
        res.Saturation = 1 -  min / res.Value;


    
    return res;
}

//
// Convert color HSV -> RGB
//
Gdiplus::ARGB CSSSColorManager::HSVtoRGB( AHSV hsv )
{    
    BYTE h = (BYTE)(hsv.Hue / 60) % 6;
    float f = ( hsv.Hue / 60 ) - h;
    float p = hsv.Value * ( 1 - hsv.Saturation );
    float q = hsv.Value * ( 1 - f * hsv.Saturation );
    float t = hsv.Value * ( 1 - ( 1 - f ) * hsv.Saturation );
    BYTE R, G, B;
    switch ( h )
    {
    case 0 :
        R = (BYTE)hsv.Value; G = (BYTE)t; B = (BYTE)p;
        break;

    case 1 :
        R = (BYTE)q; G = (BYTE)hsv.Value; B = (BYTE)p;
        break;

    case 2 :
        R = (BYTE)p; G = (BYTE)hsv.Value; B = (BYTE)t;
        break;

    case 3 :
        R = (BYTE)p; G = (BYTE)q; B = (BYTE)hsv.Value;
        break;

    case 4 :
        R = (BYTE)t; G = (BYTE)p; B = (BYTE)hsv.Value;
        break;

    case 5 :
        R = (BYTE)hsv.Value; G = (BYTE)p; B = (BYTE)q;
        break;

    default :
        R = 0; G = 0; B = 0;
        break;

    }

    return ((hsv.Alpha << (colrefSize - byteSize)) | (R << (byteSize * 2)) | (G << byteSize) | B);    
}

//
//
//
AHSV CSSSColorManager::HSVChangeValue(AHSV &hsv, int Value)
{
    hsv.Value += Value;
    if ( hsv.Value < 0 ) hsv.Value = 0;
    if ( hsv.Value > 255 ) hsv.Value = 255;

    return hsv;
}

//
//
//
COLORREF CSSSColorManager::GetSchemeColor(Color color)
{
    COLORREF Colors[4][2] = {
        { 0xFFDBBF, 0x8B4215 },     // Office2007_LunaBlue
        { 0x535353, 0xFFFFFF },     // Office2007_ObsidianBlack
        { 0xD9CAC4, 0x6E1500 },     // Office2007_Silver
        { 0xDDD4D0, 0x595453 }      // Office2007_Aqua
    };

    return Colors[m_CurrentStyle][color];    
}
