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

    static void drawHalo(CDC* dc, POINT p, double r, double pixpernm) 
    {
        int sDC = dc->SaveDC();

        //calculate pixels per nautical mile

        int pixoffset = (int)round(pixpernm * r);

       // draw the halo around point p with radius r in NM
        COLORREF targetPenColor = RGB(202, 205, 169);
        HPEN targetPen = CreatePen(PS_SOLID, 1, targetPenColor);
        dc->SelectObject(targetPen);
        dc->SelectStockObject(HOLLOW_BRUSH);
        dc->Ellipse(p.x - pixoffset, p.y - pixoffset, p.x + pixoffset, p.y + pixoffset); 

        DeleteObject(targetPen);
        
        dc->RestoreDC(sDC);
    };

    static void drawPTL(CDC* dc, POINT p, double ptlTime)
    {

    };
};

