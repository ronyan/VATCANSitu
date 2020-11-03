#pragma once
#include "EuroScopePlugIn.h"
#include "CSiTRadar.h"

using namespace std;
using namespace EuroScopePlugIn;

class CPPS :
    public EuroScopePlugIn::CRadarScreen
{
    COLORREF colorCJS;
    COLORREF colorPPS;
    CRadarScreen& radScreen;
    CRadarTarget& radTarget;

    CPPS();

    void DrawPPS(CDC* dc, CRadarScreen& , CRadarTarget& );
};

