#include "pch.h"
#include "SituPlugin.h"
#include "CSiTRadar.h"
#include "constants.h"
#include "ACTag.h"

const int TAG_ITEM_IFR_REL = 5000;
const int TAG_FUNC_IFR_REL_REQ = 5001;
const int TAG_FUNC_IFR_RELEASED = 5002;

HHOOK appHook;

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {

    switch (wParam)
    {
    case VK_F3:
    {
        if(!(lParam & 0x40000000)) { // on button down
            CSiTRadar::menuState.ptlAll = !CSiTRadar::menuState.ptlAll;
            CSiTRadar::m_pRadScr->RequestRefresh();
            return -1;
        }
    }

    case VK_F9:
    {
        if (!(lParam & 0x40000000)) {
            if (CSiTRadar::menuState.filterBypassAll == FALSE) {
                CSiTRadar::menuState.filterBypassAll = TRUE;

                for (auto& p : CSiTRadar::mAcData) {
                    CSiTRadar::tempTagData[p.first] = p.second.tagType;
                    // Do not open uncorrelated tags
                    if (p.second.tagType == 0) {
                        p.second.tagType = 1;
                    }
                }

            }
            else if (CSiTRadar::menuState.filterBypassAll == TRUE) {

                for (auto& p : CSiTRadar::tempTagData) {
                    // prevents closing of tags that became under your jurisdiction during quicklook
                    if (!CSiTRadar::m_pRadScr->GetPlugIn()->FlightPlanSelect(p.first.c_str()).GetTrackingControllerIsMe()) {
                        CSiTRadar::mAcData[p.first].tagType = p.second;
                    }
                }

                CSiTRadar::tempTagData.clear();
                CSiTRadar::menuState.filterBypassAll = FALSE;
            }

            CSiTRadar::m_pRadScr->RequestRefresh();

            return -1;
        }
    }
    }

    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

SituPlugin::SituPlugin()
	: EuroScopePlugIn::CPlugIn(EuroScopePlugIn::COMPATIBILITY_CODE,
		"VATCANSitu",
		"0.4.4.6",
		"Ron Yan",
		"Attribution-NonCommercial 4.0 International (CC BY-NC 4.0)")
{
    RegisterTagItemType("IFR Release", TAG_ITEM_IFR_REL);
    RegisterTagItemFunction("Request IFR Release", TAG_FUNC_IFR_REL_REQ);
    RegisterTagItemFunction("Grant IFR Release", TAG_FUNC_IFR_RELEASED);

    DWORD appProc = GetCurrentThreadId();
    appHook = SetWindowsHookEx(WH_KEYBOARD, KeyboardProc, NULL, appProc);
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

    if (ItemCode == TAG_ITEM_IFR_REL) {

        strcpy_s(sItemString, 16, "\u00AC");
         *pColorCode = TAG_COLOR_RGB_DEFINED;
         COLORREF c = C_PPS_ORANGE;
         *pRGB = c;

        if (strncmp(FlightPlan.GetControllerAssignedData().GetScratchPadString(), "RREQ", 4) == 0) {
            COLORREF c = C_PPS_ORANGE;
            strcpy_s(sItemString, 16, "\u00A4");
            *pRGB = c;
        }
        if (strncmp(FlightPlan.GetControllerAssignedData().GetScratchPadString(), "RREL", 4) == 0) {
            strcpy_s(sItemString, 16, "\u00A4");
            COLORREF c = RGB(9, 171, 0);
            *pRGB = c;
        }
    }

}

inline void SituPlugin::OnFunctionCall(int FunctionId, const char* sItemString, POINT Pt, RECT Area)
{
    CFlightPlan fp;
    fp = FlightPlanSelectASEL();
    string spString = fp.GetControllerAssignedData().GetScratchPadString();

    if (FunctionId == TAG_FUNC_IFR_REL_REQ) {
        if (strncmp(spString.c_str(), "RREQ", 4) == 0) {
            fp.GetControllerAssignedData().SetScratchPadString("");
            if (spString.size() > 4) { fp.GetControllerAssignedData().SetScratchPadString(spString.substr(5).c_str()); }
        }
        else if (strncmp(spString.c_str(), "RREL", 4) == 0) {
            fp.GetControllerAssignedData().SetScratchPadString("");
            if (spString.size() > 4) { fp.GetControllerAssignedData().SetScratchPadString(spString.substr(5).c_str()); }
        }
        else {
            fp.GetControllerAssignedData().SetScratchPadString(("RREQ " + spString).c_str());
        }

    }
    if (FunctionId == TAG_FUNC_IFR_RELEASED) {

        // Only allow if APP, DEP or CTR
        if (ControllerMyself().GetFacility() >= 5) {

            if (strncmp(fp.GetControllerAssignedData().GetScratchPadString(), "RREQ", 4) == 0) {

                if (spString.size() > 4) {
                    fp.GetControllerAssignedData().SetScratchPadString(("RREL " + spString.substr(5)).c_str());
                }
                else {
                    fp.GetControllerAssignedData().SetScratchPadString("RREL");
                }
            }
        }
    }
}