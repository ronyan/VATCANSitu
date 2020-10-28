#pragma once

#include "EuroScopePlugIn.h"
#include <string>
#include <map>
#include <iostream>
#include <array>
#include "pch.h"
#include "constants.h"
#include "CSiTRadar.h"

using namespace std;

class TopMenu :
    public EuroScopePlugIn::CRadarScreen
{
protected:
    static CONST int padding = 2;
    static CONST int margin = 3;


public:

    static map<int, string> ButtonData() {

    };

    TopMenu(void);
    virtual ~TopMenu(void);

    static RECT DrawButton(HDC hdc, POINT p, int width, int height, const char* btext, bool pressed) 
    {        
        CDC dc;
        dc.Attach(hdc);
        
        CFont font;
        LOGFONT lgfont;

        memset(&lgfont, 0, sizeof(LOGFONT));
        lgfont.lfWeight = 700; 
        strcpy_s(lgfont.lfFaceName, _T("Segoe UI"));
        lgfont.lfHeight = 12;
        font.CreateFontIndirect(&lgfont);

        dc.SelectObject(font);
        dc.SetTextColor(RGB(230, 230, 230));

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

        dc.SelectObject(targetPen);
        dc.SelectObject(targetBrush);

        // button rectangle
        RECT rect1;
        rect1.left = p.x;
        rect1.right = p.x + width;
        rect1.top = p.y;
        rect1.bottom = p.y + height;

        // 3d apperance calls
        dc.Rectangle(&rect1);
        dc.Draw3dRect(&rect1, pcolortl, pcolorbr);
        InflateRect(&rect1, -1, -1);
        dc.Draw3dRect(&rect1, pcolortl, pcolorbr);
        dc.DrawText(CString(btext), &rect1, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

        DeleteObject(targetPen);
        DeleteObject(targetBrush);
        DeleteObject(font);

        dc.Detach();

        return rect1;
    };

    // To facilitate different button types Green Background Button
    static RECT DrawButton2(HDC hdc, POINT p, int width, int height, const char* btext, bool pressed)
    {
        CDC dc;
        dc.Attach(hdc);

        CFont font;
        LOGFONT lgfont;

        memset(&lgfont, 0, sizeof(LOGFONT));
        lgfont.lfWeight = 700;
        strcpy_s(lgfont.lfFaceName, _T("Segoe UI"));
        lgfont.lfHeight = 12;
        font.CreateFontIndirect(&lgfont);

        dc.SelectObject(font);
        dc.SetTextColor(RGB(230, 230, 230));

        //default is unpressed state
        COLORREF pressedcolor = RGB(0, 135, 0);
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

        dc.SelectObject(targetPen);
        dc.SelectObject(targetBrush);

        // button rectangle
        RECT rect1;
        rect1.left = p.x;
        rect1.right = p.x + width;
        rect1.top = p.y;
        rect1.bottom = p.y + height;

        // 3d apperance calls
        dc.Rectangle(&rect1);
        dc.Draw3dRect(&rect1, pcolortl, pcolorbr);
        InflateRect(&rect1, -1, -1);
        dc.Draw3dRect(&rect1, pcolortl, pcolorbr);
        dc.DrawText(CString(btext), &rect1, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

        DeleteObject(targetPen);
        DeleteObject(targetBrush);
        DeleteObject(font);

        dc.Detach();

        return rect1;
    };

    static void DrawBackground(HDC hdc, POINT p, int width, int height) {
        CDC dc;
        dc.Attach(hdc);

        COLORREF targetPenColor = RGB(166, 166, 166);
        HPEN targetPen = CreatePen(PS_SOLID, 1, targetPenColor);
        HBRUSH targetBrush = CreateSolidBrush(RGB(66, 66, 66));

        dc.SelectObject(targetPen);
        dc.SelectObject(targetBrush);

        dc.Rectangle(p.x, p.y, p.x + width, p.y + height);

        DeleteObject(targetPen);
        DeleteObject(targetBrush);

        dc.Detach();
    };


    static RECT MakeText(HDC hdc, POINT p, int width, int height, const char* btext)
    {
        CDC dc;
        dc.Attach(hdc);

        CFont font;
        LOGFONT lgfont;

        memset(&lgfont, 0, sizeof(LOGFONT));
        lgfont.lfWeight = 700;
        strcpy_s(lgfont.lfFaceName, _T("Segoe UI"));
        lgfont.lfHeight = 12;
        font.CreateFontIndirect(&lgfont);

        dc.SelectObject(font);
        dc.SetTextColor(RGB(230, 230, 230));

        // text rectangle
        RECT rect1;
        rect1.left = p.x;
        rect1.right = p.x + width;
        rect1.top = p.y;
        rect1.bottom = p.y + height;

        dc.DrawText(CString(btext), &rect1, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

        DeleteObject(font);

        dc.Detach();

        return rect1;
    };

    static RECT MakeDropDown(HDC hdc, POINT p, int width, int height, const char* btext) {
        CDC dc;
        dc.Attach(hdc);

        CFont font;
        LOGFONT lgfont;

        memset(&lgfont, 0, sizeof(LOGFONT));
        lgfont.lfWeight = 700;
        strcpy_s(lgfont.lfFaceName, _T("Segoe UI"));
        lgfont.lfHeight = 12;
        font.CreateFontIndirect(&lgfont);

        dc.SelectObject(font);
        dc.SetTextColor(RGB(230, 230, 230));

        //default is unpressed state

        COLORREF pressedcolor = RGB(66, 66, 66);
        COLORREF pcolortl = RGB(55, 55, 55);
        COLORREF pcolorbr = RGB(140, 140, 140);

        COLORREF targetPenColor = RGB(140, 140, 140);
        HPEN targetPen = CreatePen(PS_SOLID, 2, targetPenColor);
        HBRUSH targetBrush = CreateSolidBrush(pressedcolor);

        dc.SelectObject(targetPen);
        dc.SelectObject(targetBrush);

        // button rectangle
        RECT rect1;
        rect1.left = p.x;
        rect1.right = p.x + width;
        rect1.top = p.y;
        rect1.bottom = p.y + height;

        // 3d apperance calls
        dc.Rectangle(&rect1);
        dc.Draw3dRect(&rect1, pcolortl, pcolorbr);
        InflateRect(&rect1, -1, -1);
        dc.Draw3dRect(&rect1, pcolortl, pcolorbr);
        dc.DrawText(CString(btext), &rect1, DT_LEFT | DT_SINGLELINE | DT_VCENTER);

        DeleteObject(targetPen);
        DeleteObject(targetBrush);
        DeleteObject(font);

        dc.Detach();

        return rect1;

    }

    static RECT MakeField(HDC hdc, POINT p, int width, int height, const char* btext) {
        CDC dc;
        dc.Attach(hdc);

        CFont font;
        LOGFONT lgfont;

        memset(&lgfont, 0, sizeof(LOGFONT));
        lgfont.lfWeight = 700;
        strcpy_s(lgfont.lfFaceName, _T("Segoe UI"));
        lgfont.lfHeight = 12;
        font.CreateFontIndirect(&lgfont);

        dc.SelectObject(font);
        dc.SetTextColor(RGB(230, 230, 230));

        //default is unpressed state

        COLORREF pressedcolor = RGB(66, 66, 66);
        COLORREF pcolortl = RGB(55, 55, 55);
        COLORREF pcolorbr = RGB(140, 140, 140);

        COLORREF targetPenColor = RGB(140, 140, 140);
        HPEN targetPen = CreatePen(PS_SOLID, 2, targetPenColor);
        HBRUSH targetBrush = CreateSolidBrush(pressedcolor);

        dc.SelectObject(targetPen);
        dc.SelectObject(targetBrush);

        // button rectangle
        RECT rect1;
        rect1.left = p.x;
        rect1.right = p.x + width;
        rect1.top = p.y;
        rect1.bottom = p.y + height;

        // 3d apperance calls
        dc.Rectangle(&rect1);
        dc.Draw3dRect(&rect1, pcolortl, pcolorbr);
        InflateRect(&rect1, -1, -1);
        dc.Draw3dRect(&rect1, pcolortl, pcolorbr);
        dc.DrawText(CString(btext), &rect1, DT_LEFT | DT_SINGLELINE | DT_VCENTER);

        DeleteObject(targetPen);
        DeleteObject(targetBrush);
        DeleteObject(font);

        dc.Detach();

        return rect1;

    }

    static void DrawModule(HDC hdc, int module, POINT menutopleft) {
        CDC dc;
        dc.Attach(hdc);
        POINT p = CPoint(1, 1);
        int w = 1;
        int h = 1;

        // draw button loop
        menutopleft.y = menutopleft.y + 25;
        TopMenu::DrawButton(dc, menutopleft, 45, 23, "PTL 3", 0);

        menutopleft.y = menutopleft.y - 25;
        menutopleft.x = menutopleft.x + 55;
        TopMenu::DrawButton(dc, menutopleft, 35, 23, "RBL", 0);

        menutopleft.y = menutopleft.y + 25;
        TopMenu::DrawButton(dc, menutopleft, 35, 23, "PIV", 0);

        menutopleft.y = menutopleft.y - 25;
        menutopleft.x = menutopleft.x + 45;
        TopMenu::DrawButton(dc, menutopleft, 45, 23, "Rings 20", 0);

        menutopleft.y = menutopleft.y + 25;
        TopMenu::DrawButton(dc, menutopleft, 45, 23, "Grid", 0);

    }

    static void DrawHaloRadOptions(HDC hdc, POINT p, double halorad, string hoptions[]) {
        CDC dc;
        dc.Attach(hdc);

        int idx;

        p.x += 105;

        p.y += 31;
        for (idx = 0; idx < 9; idx++) {
            bool pressed = FALSE;
            if (stod(hoptions[idx]) == halorad) {
                pressed = TRUE;
            }
 
            TopMenu::DrawButton(dc, p, 20, 15, hoptions[idx].c_str(), pressed);
            p.x += 22;
        }

        dc.Detach();
    }

};

