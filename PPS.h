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

public:
    static void CPPS::DrawPPS(CDC* dc, BOOL isCorrelated, BOOL isVFR, BOOL isADSB, BOOL isRVSM, int radFlag, COLORREF ppsColor, string squawk, POINT p);
};

