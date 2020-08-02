#pragma once
#include "EuroScopePlugIn.h"
#include "VATCANSitu.h"
#include "CSiTRadar.h"

using namespace std;
using namespace EuroScopePlugIn;

class HaloTool :
    public CRadarScreen
{
public:
    HaloTool(void);
    ~HaloTool(void);

    static void drawHalo(HDC hdc, POINT p, double r, int pixpernm) 
    {
        CDC dc;
        dc.Attach(hdc);
        //calculate pixels per nautical mile

        int pixoffset = (int)round(pixpernm * r);

       // draw the halo around point p with radius r in NM
        COLORREF targetPenColor = RGB(202, 205, 169);
        HPEN targetPen = CreatePen(PS_SOLID, 1, targetPenColor);
        dc.SelectObject(targetPen);
        dc.SelectStockObject(HOLLOW_BRUSH);
        dc.Ellipse(p.x - pixoffset, p.y - pixoffset, p.x + pixoffset, p.y + pixoffset); 

        DeleteObject(targetPen);
        dc.Detach();
    };
};

