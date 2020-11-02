#pragma once
#include "EuroScopePlugIn.h"
#include "constants.h"
#include "CSiTRadar.h"
#include "SituPlugin.h"

using namespace std;

class CACTag :

    public EuroScopePlugIn::CRadarScreen
{
protected:

    POINT p;

public:

    // Tags for FP predictions
    static void DrawFPACTag(CDC* hdc, CRadarScreen* rad, CRadarTarget* rt, CFlightPlan* fp, map<string, POINT>* tOffset);
    static void DrawFPConnector(CDC* dc, CRadarScreen* rad, CRadarTarget* rt, CFlightPlan* fp, COLORREF color, map<string, POINT>* tOffset);

    // Tags for Radar targets
    static void CACTag::DrawRTACTag(CDC* dc, CRadarScreen* rad, CRadarTarget* rt, CFlightPlan* fp, map<string, POINT>* tOffset);
    static void CACTag::DrawRTConnector(CDC* dc, CRadarScreen* rad, CRadarTarget* rt, CFlightPlan* fp, COLORREF color, map<string, POINT>* tOffset);
};

