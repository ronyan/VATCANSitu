#include "pch.h"
#include "SituPlugin.h"
#include "CSiTRadar.h"
#include "constants.h"

SituPlugin::SituPlugin()
	: EuroScopePlugIn::CPlugIn(EuroScopePlugIn::COMPATIBILITY_CODE,
		"NAVCANSitu",
		"0.3.0.0",
		"Ron Yan",
		"Attribution-NonCommercial 4.0 International (CC BY-NC 4.0)")
{
}

SituPlugin::~SituPlugin()
{
}

EuroScopePlugIn::CRadarScreen* SituPlugin::OnRadarScreenCreated(const char* sDisplayName, bool NeedRadarContent, bool GeoReferenced, bool CanBeSaved, bool CanBeCreated)
{
    return new CSiTRadar;
}

void SituPlugin::OnGetTagItem(EuroScopePlugIn::CFlightPlan FlightPlan,
    EuroScopePlugIn::CRadarTarget RadarTarget,
    int ItemCode,
    int TagData,
    char sItemString[16],
    int* pColorCode,
    COLORREF* pRGB,
    double* pFontSize) {
}