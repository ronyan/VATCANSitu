#pragma once

#include "EuroScopePlugIn.h"
#include <string>
#include <map>
#include <array>
#include "pch.h"
#include "CSiTRadar.h"

using namespace std;

class TopMenu :
    public EuroScopePlugIn::CRadarScreen
{

public:

    static RECT DrawButton(CDC* dc, POINT p, int width, int height, const char* btext, bool pressed) 
    {        
        int sDC = dc->SaveDC();

        CFont font;
        LOGFONT lgfont;

        memset(&lgfont, 0, sizeof(LOGFONT));
        lgfont.lfWeight = 700; 
        strcpy_s(lgfont.lfFaceName, _T("Segoe UI"));
        lgfont.lfHeight = 12;
        font.CreateFontIndirect(&lgfont);

        dc->SelectObject(font);
        dc->SetTextColor(RGB(230, 230, 230));

        //default is unpressed state
        COLORREF pressedcolor = RGB(66, 66, 66); 
        COLORREF pcolortl = RGB(140, 140, 140);
        COLORREF pcolorbr = RGB(55, 55, 55);
        
        if (pressed == TRUE) {
            pressedcolor = RGB(40, 40, 40);
            pcolortl = RGB(55, 55, 55); 
            pcolorbr = RGB(140, 140, 140);
        }

        COLORREF targetPenColor = RGB(140, 140, 140);
        HPEN targetPen = CreatePen(PS_SOLID, 2, targetPenColor);
        HBRUSH targetBrush = CreateSolidBrush(pressedcolor);

        dc->SelectObject(targetPen);
        dc->SelectObject(targetBrush);

        // button rectangle
        RECT rect1;
        rect1.left = p.x;
        rect1.right = p.x + width;
        rect1.top = p.y;
        rect1.bottom = p.y + height;

        // 3d apperance calls
        dc->Rectangle(&rect1);
        dc->Draw3dRect(&rect1, pcolortl, pcolorbr);
        InflateRect(&rect1, -1, -1);
        dc->Draw3dRect(&rect1, pcolortl, pcolorbr);
        dc->DrawText(CString(btext), &rect1, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

        DeleteObject(targetPen);
        DeleteObject(targetBrush);
        DeleteObject(font);

        dc->RestoreDC(sDC);

        return rect1;
    };
};

