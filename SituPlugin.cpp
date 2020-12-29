#include "pch.h"
#include "SituPlugin.h"
#include "CSiTRadar.h"
#include "constants.h"
#include "ACTag.h"

const int TAG_ITEM_CTP_SLOT = 5000;
const int TAG_ITEM_CTP_CTOT = 5001;

HHOOK appHook;

LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case WM_MBUTTONDOWN:
		{
			
			break;
		}
	}
	return CallNextHookEx(0, nCode, wParam, lParam);
}

SituPlugin::SituPlugin()
	: EuroScopePlugIn::CPlugIn(EuroScopePlugIn::COMPATIBILITY_CODE,
		"VATCANSitu",
		"0.3.1.2",
		"Ron Yan",
		"Attribution-NonCommercial 4.0 International (CC BY-NC 4.0)")
{
    DWORD appProc = GetCurrentThreadId();
	appHook = SetWindowsHookEx(WH_MOUSE, MouseProc, NULL, appProc);
}

SituPlugin::~SituPlugin()
{
	UnhookWindowsHookEx(appHook);
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