#include "pch.h"
#include "SituPlugin.h"
#include "CSiTRadar.h"

const int TAG_ITEM_CTP_SLOT = 5000;
const int TAG_ITEM_CTP_CTOT = 5001;

SituPlugin::SituPlugin()
	: EuroScopePlugIn::CPlugIn(EuroScopePlugIn::COMPATIBILITY_CODE,
		"VATCANSitu",
		"0.3.0.2RC-CTP-minimal",
		"Ron Yan",
		"Attribution-NonCommercial 4.0 International (CC BY-NC 4.0)")
{
    RegisterTagItemType("CTP Slot", TAG_ITEM_CTP_SLOT);
    RegisterTagItemType("CTP CTOT", TAG_ITEM_CTP_CTOT);
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

    if (ItemCode == TAG_ITEM_CTP_SLOT) {
        if (CSiTRadar::mAcData[FlightPlan.GetCallsign()].hasCTP) {
            strcpy_s(sItemString, 16,"C");
        }
    }
    if (ItemCode == TAG_ITEM_CTP_CTOT) {
        strcpy_s(sItemString, 16, CSiTRadar::mAcData[FlightPlan.GetCallsign()].slotTime.c_str());
    }
}