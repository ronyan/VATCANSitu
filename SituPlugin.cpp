#include "pch.h"
#include "SituPlugin.h"
#include "CSiTRadar.h"
#include "constants.h"
#include "ACTag.h"

const int TAG_ITEM_IFR_REL = 5000;
const int TAG_FUNC_IFR_REL_REQ = 5001;
const int TAG_FUNC_IFR_RELEASED = 5002;

bool held = false;
bool injected = false;
bool kbF3 = false;
size_t jurisdictionIndex = 0;
size_t oldJurisdictionSize = 0;

HHOOK appHook;

// Takes a vector of keycodes and sends as keyboard commands
void SituPlugin::SendKeyboardPresses(vector<WORD> message)
{
    std::vector<INPUT> vec;
    for (auto ch : message)
    {
        INPUT input = { 0 };
        input.type = INPUT_KEYBOARD;
        input.ki.dwFlags = KEYEVENTF_SCANCODE;
        input.ki.time = 0;
        input.ki.wVk = 0;
        input.ki.wScan = ch;
        input.ki.dwExtraInfo = 1;
        vec.push_back(input);

        input.ki.dwFlags |= KEYEVENTF_KEYUP;
        vec.push_back(input);
    }

    SendInput(vec.size(), &vec[0], sizeof(INPUT));
}

void SendLongPress(vector<WORD> message)
{
    std::vector<INPUT> vec;
    for (auto ch : message)
    {
        INPUT input = { 0 };
        input.type = INPUT_KEYBOARD;
        input.ki.dwFlags = KEYEVENTF_SCANCODE;
        input.ki.time = 0;
        input.ki.wVk = 0;
        input.ki.wScan = ch;
        vec.push_back(input);

        input.ki.dwFlags |= KEYEVENTF_KEYUP;
        vec.push_back(input);
    }

    SendInput(vec.size(), &vec[0], sizeof(INPUT));
}

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {

    if (
        wParam == VK_F1 ||
        wParam == VK_F3 ||
        wParam == VK_F9 ||
        wParam == VK_RETURN ||
        wParam == VK_ESCAPE 
        ) {

        if (!(lParam & 0x40000000)) { // if bit 30 is 0 this will evaluate true means key was previously up

            switch (wParam) {
            case VK_RETURN:
            {
                if (CSiTRadar::menuState.handoffMode) {
                    SituPlugin::SendKeyboardPresses({ 0x4E, 0x01 });
                    CSiTRadar::menuState.handoffMode = FALSE;
                }
                CSiTRadar::m_pRadScr->RequestRefresh();
                return 0;
            }

            case VK_F1: {

                if (GetAsyncKeyState(VK_F1) & 0x8000) {
                    if (CSiTRadar::m_pRadScr->GetPlugIn()->RadarTargetSelectASEL().IsValid()) {
                        SituPlugin::SendKeyboardPresses({ 0x01 });
                    }
                    return -1;
                }

            }

            case VK_F3: {
                if (GetAsyncKeyState(VK_F3) & 0x8000) {
                    kbF3 = true;
                    return -1;
                }
            }

            case VK_ESCAPE: {
                
                if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
                    if (CSiTRadar::menuState.handoffMode == TRUE) {
                        
                        CSiTRadar::menuState.handoffMode = FALSE;
                        CSiTRadar::menuState.jurisdictionIndex = 0;
                        CSiTRadar::m_pRadScr->RequestRefresh();

                        return 0;
                    }
                    else { return 0; }
                }
                
                else {
                    CSiTRadar::m_pRadScr->RequestRefresh();
                    return 0;
                }
            }

            }


            return -1;
        }
        else { // if key was previously down
            if (!(lParam & 0x80000000)) { // if bit 31 is 0 this will evaluate true, which means key is being pressed
                if (!(lParam & 0x0000ffff)) {  // if no repeats
                    return -1;
                }
                else {
                    held = true;
                    // Long Press Keyboard Commands will send the function direct to ES



                    // *** END LONG PRESS COMMANDS ***
                    return 0;
                }
            }
            else {
                if (held == false) {
                    // START OF SHORT PRESS KEYBOARD COMMANDS ***
                    switch (wParam) {
                    case VK_F1: {
                        // Toggle on hand-off mode
                        if (CSiTRadar::menuState.handoffMode == FALSE ||
                            CSiTRadar::menuState.jurisdictionalAC.size() != oldJurisdictionSize) {
                            oldJurisdictionSize = CSiTRadar::menuState.jurisdictionalAC.size();
                            CSiTRadar::menuState.jurisdictionIndex = 0;
                        }

                        CSiTRadar::menuState.handoffMode = TRUE;
                        CSiTRadar::menuState.handoffModeStartTime = clock();

                        // ASEL the next aircraft in the handoff priority list
                        if (!CSiTRadar::menuState.jurisdictionalAC.empty()) {
                            if (CSiTRadar::menuState.jurisdictionIndex < CSiTRadar::menuState.jurisdictionalAC.size()) {
                                CSiTRadar::m_pRadScr->GetPlugIn()->SetASELAircraft(CSiTRadar::m_pRadScr->GetPlugIn()->FlightPlanSelect(CSiTRadar::menuState.jurisdictionalAC.at(CSiTRadar::menuState.jurisdictionIndex).c_str()));
                                // if plane is being handed off to me, use F3 to accept handoff instead of F4 to deny
                                if (
                                    strcmp(
                                        CSiTRadar::m_pRadScr->GetPlugIn()->FlightPlanSelect(CSiTRadar::menuState.jurisdictionalAC.at(CSiTRadar::menuState.jurisdictionIndex).c_str()).GetHandoffTargetControllerId(),
                                           CSiTRadar::m_pRadScr->GetPlugIn()->ControllerMyself().GetPositionId()) == 0
                                    )
                                    {

                                    SituPlugin::SendKeyboardPresses({ 0x3D }); // send F3
                                }
                                else {

                                    SituPlugin::SendKeyboardPresses({ 0x3E }); // send F4 in keyboard presses
                                }
                                CSiTRadar::menuState.jurisdictionIndex++;
                            }
                            else {
                                CSiTRadar::menuState.jurisdictionIndex = 0;
                                CSiTRadar::menuState.handoffMode = FALSE;
                                SituPlugin::SendKeyboardPresses({ 0x01 });
                            }
                        }

                        CSiTRadar::m_pRadScr->RequestRefresh();

                        held = false;
                        return -1;
                    }

                    case VK_F3:
                    {
                        
                        if (kbF3) {

                            CSiTRadar::menuState.ptlAll = !CSiTRadar::menuState.ptlAll;
                            CSiTRadar::m_pRadScr->RequestRefresh();

                            kbF3 = false;
                            held = false;
                            return -1;
                        }
                        else {
                            

                        }
                        return 0;
                    }
                    case VK_F9:
                    {

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

                        held = false;
                        return -1;
                    }

                    // *** END OF SHORT KEYBOARD PRESS COMMANDS ***
                    }          
                }
                held = false;
            }
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

SituPlugin::SituPlugin()
	: EuroScopePlugIn::CPlugIn(EuroScopePlugIn::COMPATIBILITY_CODE,
		"VATCANSitu",
		"0.4.5.1",
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

