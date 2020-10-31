#pragma once
#include "EuroScopePlugIn.h"
#include "constants.h"
#include "CSiTRadar.h"
#include "SituPlugin.h"


class CACTag :

    public EuroScopePlugIn::CRadarScreen
{
protected:

    // Generate grid
	int gridX;
	int gridY;
	int gridSpacing;
    POINT p;


public:

    static void DrawACTag(CDC* hdc, CRadarScreen* rad, CRadarTarget* rt, CFlightPlan* fp);
    static void DrawConnector(CDC* dc, CRadarScreen* rad, CRadarTarget* rt, CFlightPlan* fp);

};

