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

    static void DrawACTag(CDC* hdc, CRadarScreen* rad, CRadarTarget* rt, CFlightPlan* fp, map<string, POINT>* tOffset);
    static void DrawConnector(CDC* dc, CRadarScreen* rad, CRadarTarget* rt, CFlightPlan* fp, COLORREF color, map<string, POINT>* tOffset);

};

