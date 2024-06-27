#pragma once

#include "EuroScopePlugIn.h"
#include <string>
#include <map>
#include <iostream>
#include <array>
#include "pch.h"
#include "constants.h"
#include "CSiTRadar.h"
#include "CFontHelper.h"

using namespace std;

struct menuButton {
    POINT location;
    string butText;
    int width;
    int height;
    COLORREF pressedColor;
    COLORREF unpressedColor;
    COLORREF textColor = RGB(230, 230, 230);
    bool pressed;
};

struct module {
    int level;
    int width;
};

class TopMenu :
    public EuroScopePlugIn::CRadarScreen
{

public:

    static map<string, menuButton> menuButtons;

    static void DrawIconBut(CDC* dc, menuButton mbut, POINT icon[], int iconSize) {

        int sDC = dc->SaveDC();

        POINT x = { mbut.location.x + (mbut.width / 2), mbut.location.y + (mbut.height / 2) };

        // shift the icon to the centre of the button
        for (int idx = 0; idx < iconSize ; idx++) {
            icon[idx].x += x.x-1;
            icon[idx].y += x.y;
        }
        HPEN hPen = CreatePen(PS_SOLID, 1, mbut.textColor);
        HBRUSH iconBrush = CreateSolidBrush(mbut.textColor);
        dc->SelectObject(iconBrush);
        dc->SelectObject(hPen);

        dc->Polygon(icon, iconSize);
        
        DeleteObject(iconBrush);
        DeleteObject(hPen);
        dc->RestoreDC(sDC);
    }

    static RECT DrawBut(CDC* dc, menuButton mbut) {
        int sDC = dc->SaveDC();

        dc->SelectObject(CFontHelper::Segoe12);
        dc->SetTextColor(mbut.textColor);

        //default is unpressed state
        COLORREF pressedcolor = C_MENU_GREY3;
        COLORREF pcolortl = C_MENU_GREY4;
        COLORREF pcolorbr = C_MENU_GREY2;

        if (mbut.pressed == TRUE) {
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
        rect1.left = mbut.location.x;
        rect1.right = mbut.location.x + mbut.width;
        rect1.top = mbut.location.y;
        rect1.bottom = mbut.location.y + mbut.height;

        // 3d apperance calls
        dc->Rectangle(&rect1);
        dc->Draw3dRect(&rect1, pcolortl, pcolorbr);
        InflateRect(&rect1, -1, -1);
        dc->Draw3dRect(&rect1, pcolortl, pcolorbr);
        dc->DrawText(mbut.butText.c_str(), &rect1, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

        DeleteObject(targetPen);
        DeleteObject(targetBrush);

        dc->RestoreDC(sDC);

        return rect1;
    }

    static RECT DrawButton(CDC* dc, POINT p, int width, int height, const char* btext, bool pressed) 
    {        
        int sDC = dc->SaveDC();

        dc->SelectObject(CFontHelper::Segoe12);
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

        dc->RestoreDC(sDC);

        return rect1;
    };

    // To facilitate different button types Green Background Button
    static RECT DrawButton2(HDC hdc, POINT p, int width, int height, const char* btext, bool pressed)
    {
        CDC dc;
        dc.Attach(hdc);

        dc.SelectObject(CFontHelper::Segoe12);
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

        dc.SelectObject(CFontHelper::Segoe12);
        dc.SetTextColor(RGB(230, 230, 230));

        // text rectangle
        RECT rect1;
        rect1.left = p.x;
        rect1.right = p.x + width;
        rect1.top = p.y;
        rect1.bottom = p.y + height;

        dc.DrawText(CString(btext), &rect1, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

        dc.Detach();

        return rect1;
    };

    static RECT MakeTextLeft(HDC hdc, POINT p, int width, int height, const char* btext)
    {
        CDC dc;
        dc.Attach(hdc);

        dc.SelectObject(CFontHelper::Segoe12);
        dc.SetTextColor(RGB(230, 230, 230));

        // text rectangle
        RECT rect1;
        rect1.left = p.x;
        rect1.right = p.x + width;
        rect1.top = p.y;
        rect1.bottom = p.y + height;

        dc.DrawText(CString(btext), &rect1, DT_LEFT | DT_SINGLELINE | DT_VCENTER);

        dc.Detach();

        return rect1;
    };

    static RECT MakeDropDown(HDC hdc, POINT p, int width, int height, const char* btext) {
        CDC dc;
        dc.Attach(hdc);

        dc.SelectObject(CFontHelper::Segoe12);
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

        dc.Detach();

        return rect1;

    }

    static RECT MakeField(HDC hdc, POINT p, int width, int height, const char* btext) {
        CDC dc;
        dc.Attach(hdc);

        dc.SelectObject(CFontHelper::Segoe12);
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
        TopMenu::DrawButton(&dc, menutopleft, 45, 23, "PTL 3", 0);

        menutopleft.y = menutopleft.y - 25;
        menutopleft.x = menutopleft.x + 55;
        TopMenu::DrawButton(&dc, menutopleft, 35, 23, "RBL", 0);

        menutopleft.y = menutopleft.y + 25;
        TopMenu::DrawButton(&dc, menutopleft, 35, 23, "PIV", 0);

        menutopleft.y = menutopleft.y - 25;
        menutopleft.x = menutopleft.x + 45;
        TopMenu::DrawButton(&dc, menutopleft, 45, 23, "Rings 20", 0);

        menutopleft.y = menutopleft.y + 25;
        TopMenu::DrawButton(&dc, menutopleft, 45, 23, "Grid", 0);

    }

    static void DrawHaloRadOptions(HDC hdc, POINT p, double halorad, string hoptions[]) {
        CDC dc;
        dc.Attach(hdc);

        int idx;

        p.y += 31;
        for (idx = 0; idx < 9; idx++) {
            bool pressed = FALSE;
            if (stod(hoptions[idx]) == halorad) {
                pressed = TRUE;
            }
 
            TopMenu::DrawButton(&dc, p, 20, 15, hoptions[idx].c_str(), pressed);
            p.x += 22;
        }

        dc.Detach();
    }

    static RECT DrawWindow(CDC* dc, EuroScopePlugIn::CRadarScreen* radscr, POINT origin, int width, int height) {
        int sDC = dc->SaveDC();

        dc->SelectObject(CFontHelper::Segoe12);
        dc->SetTextColor(RGB(230, 230, 230));

        dc->RestoreDC(sDC);
    }

};