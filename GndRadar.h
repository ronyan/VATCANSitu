#pragma once
#include "EuroScopePlugIn.h"
#include "constants.h"
#include "CSiTRadar.h"
#include <gdiplus.h>

class GndRadar :
    public EuroScopePlugIn::CRadarScreen
{
public:
    static RECT DrawGndTag(HDC hdc, POINT p, int sts, CRadarTarget targ, const char *tagText) {
        CDC dc;
        dc.Attach(hdc);

        COLORREF depColor = RGB(200, 0, 0);
        COLORREF arrColor = RGB(0, 0, 200);

        COLORREF tagColor = RGB(0,0,0);

        if (sts == 0) { tagColor = depColor; }
        if (sts == 1) { tagColor = arrColor; }
 
        HPEN targetPen = CreatePen(PS_SOLID, 1, tagColor);
        HBRUSH targetBrush = CreateSolidBrush(RGB(50,50,50));

        // offset the tag location
        p.x += 10;
        p.y -= 25;
        RECT rect;
        rect.left = p.x;
        rect.right = p.x + 60;
        rect.top = p.y;
        rect.bottom = p.y + 15;

        dc.SelectObject(targetPen);
        dc.SelectObject(targetBrush);

        POINT roundness;
        roundness.x = 3;
        roundness.y = 3;
        dc.RoundRect(&rect, roundness);

        CFont font;
        LOGFONT lgfont;

        memset(&lgfont, 0, sizeof(LOGFONT));
        lgfont.lfHeight = 12;
        lgfont.lfWeight = 400;
        font.CreateFontIndirectW(&lgfont);
        dc.SelectObject(font);
        dc.SetTextColor(tagColor);

        rect.left += 2; // add padding
        rect.top += 2;

        dc.DrawText(CString(tagText), &rect, DT_LEFT);

        DeleteObject(targetBrush);
        DeleteObject(targetPen);

        dc.Detach();

        TAG_TYPE_DETAILED;
        return rect;
    }
};