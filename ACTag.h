#pragma once
#include "EuroScopePlugIn.h"
#include "constants.h"
#include "CSiTRadar.h"
#include "SituPlugin.h"

using namespace std;

class CACTag
{
protected:

public:
    // Tags for FP predictions
    static void DrawFPACTag(CDC* hdc, CRadarScreen* rad, CRadarTarget* rt, CFlightPlan* fp, unordered_map<string, POINT>* tOffset);
    static void DrawFPConnector(CDC* dc, CRadarScreen* rad, CRadarTarget* rt, CFlightPlan* fp, COLORREF color, unordered_map<string, POINT>* tOffset);

    // Tags for Radar targets
    static void CACTag::DrawRTACTag(CDC* dc, CRadarScreen* rad, CRadarTarget* rt, CFlightPlan* fp, unordered_map<string, POINT>* tOffset);
    static void DrawNARDSTag(CDC* dc, CRadarScreen* rad, CRadarTarget* rt, CFlightPlan* fp, unordered_map<string, POINT>* tOffset);
    static void CACTag::DrawRTConnector(CDC* dc, CRadarScreen* rad, CRadarTarget* rt, CFlightPlan* fp, COLORREF color, unordered_map<string, POINT>* tOffset);
    static void DrawHistoryDots(CDC* dc, CRadarTarget* rt);
    static void DrawHistoryDots(CDC* dc, CFlightPlan* rt);
};

