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
bool kbF1 = false;
bool kbF3 = false;
bool kbF4 = false;
size_t jurisdictionIndex = 0;
size_t oldJurisdictionSize = 0;

POINT SituPlugin::prevMousePt = { 0,0 };
int SituPlugin::prevMouseDelta = 0;
bool SituPlugin::mouseAtRest = false;

HHOOK appHook;
HHOOK mouseHook;

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

void SendMouseClick(DWORD mouseBut) {
    INPUT input{0};
    input.type = INPUT_MOUSE;
    input.mi.mouseData = 0;
    input.mi.time = 0;
    input.mi.dwFlags = mouseBut;

    SendInput(1, &input, sizeof(input));
}

void SituPlugin::SendKeyboardString(const string str) {
    std::vector<INPUT> vec;
    const auto key_board_layout = GetKeyboardLayout(0);

    INPUT iCaps = { 0 };
    iCaps.type = INPUT_KEYBOARD;
    iCaps.ki.dwFlags = 0;
    iCaps.ki.time = 0;
    iCaps.ki.wVk = VK_CAPITAL;
    iCaps.ki.wScan = MapVirtualKeyExW(VK_SHIFT, MAPVK_VK_TO_VSC, key_board_layout);
    iCaps.ki.dwExtraInfo = 0;
    vec.push_back(iCaps);

    iCaps.ki.dwFlags |= KEYEVENTF_KEYUP;
    vec.push_back(iCaps);

    for (auto ch : str)
    {
        INPUT input = { 0 };
        if (ch == '_') {
            INPUT inputShift = { 0 };
            inputShift.type = INPUT_KEYBOARD;
            inputShift.ki.dwFlags = 0;
            inputShift.ki.time = 0;
            inputShift.ki.wVk = VK_SHIFT;
            inputShift.ki.wScan = MapVirtualKeyExW(VK_SHIFT, MAPVK_VK_TO_VSC, key_board_layout);
            inputShift.ki.dwExtraInfo = 0;
            vec.push_back(inputShift);

            input.type = INPUT_KEYBOARD;
            input.ki.dwFlags = 0;
            input.ki.time = 0;
            input.ki.wVk = VK_OEM_MINUS;
            input.ki.wScan = MapVirtualKeyExW(VK_OEM_MINUS, MAPVK_VK_TO_VSC, key_board_layout);
            input.ki.dwExtraInfo = 0;
            vec.push_back(input);

            input.ki.dwFlags |= KEYEVENTF_KEYUP;
            vec.push_back(input);

            inputShift.ki.dwFlags |= KEYEVENTF_KEYUP;
            vec.push_back(inputShift);
        }
        else {

            input.type = INPUT_KEYBOARD;
            input.ki.dwFlags = 0;
            input.ki.time = 0;
            input.ki.wVk = VkKeyScanExW(ch, key_board_layout);
            input.ki.wScan = MapVirtualKeyExW(VkKeyScanExW(ch, key_board_layout), MAPVK_VK_TO_VSC, key_board_layout);
            input.ki.dwExtraInfo = 0;
            vec.push_back(input);

            input.ki.dwFlags |= KEYEVENTF_KEYUP;
            vec.push_back(input);
        }
    }

    iCaps.ki.dwFlags = 0;
    vec.push_back(iCaps);

    iCaps.ki.dwFlags |= KEYEVENTF_KEYUP;
    vec.push_back(iCaps);

    SendInput(vec.size(), vec.data(), sizeof(INPUT));
}

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {

    if (CSiTRadar::m_pRadScr == nullptr) { return CallNextHookEx(NULL, nCode, wParam, lParam); }

    if (CSiTRadar::menuState.focusedItem.m_focus_on) {
        CAppWindows* parentWin = CSiTRadar::GetAppWindow(CSiTRadar::menuState.focusedItem.m_focused_tf->m_parentWindowID);
        if (!(lParam & 0x40000000)) {
            if (wParam >= 0x30 && wParam <= 0x5A) {
                char l = MapVirtualKeyA(wParam, 2);

                CSiTRadar::menuState.focusedItem.m_focused_tf->m_text.push_back(l);
                CSiTRadar::m_pRadScr->RequestRefresh();
                return -1;
            }
            if (wParam == VK_OEM_2) {
                CSiTRadar::menuState.focusedItem.m_focused_tf->m_text.push_back('/');
                CSiTRadar::m_pRadScr->RequestRefresh();
                return -1;
            }
            if (wParam == VK_SPACE) {
                CSiTRadar::menuState.focusedItem.m_focused_tf->m_text.push_back(' ');
                CSiTRadar::m_pRadScr->RequestRefresh();
                return -1;
            }
            if (wParam == VK_BACK) {
                if (!CSiTRadar::menuState.focusedItem.m_focused_tf->m_text.empty()) {
                    CSiTRadar::menuState.focusedItem.m_focused_tf->m_text.pop_back();
                    CSiTRadar::m_pRadScr->RequestRefresh();
                }
                return -1;
            }
            if (wParam == VK_RETURN) {
                if (parentWin->m_winType == WINDOW_HANDOFF_EXT_CJS) {
              
                    CSiTRadar::m_pRadScr->GetPlugIn()->FlightPlanSelect(parentWin->m_callsign.c_str()).InitiateHandoff(
                        CSiTRadar::m_pRadScr->GetPlugIn()->ControllerSelectByPositionId(CSiTRadar::menuState.focusedItem.m_focused_tf->m_text.c_str()).GetCallsign()
                    );
                    CSiTRadar::CloseWindow(parentWin->m_windowId_);
                    CSiTRadar::m_pRadScr->RequestRefresh();
                    return -1;
                }
                if (parentWin->m_winType == WINDOW_CTRL_REMARKS) {

                    CSiTRadar::ModifyCtrlRemarks(CSiTRadar::menuState.focusedItem.m_focused_tf->m_text.c_str(), CSiTRadar::m_pRadScr->GetPlugIn()->FlightPlanSelect(parentWin->m_callsign.c_str()));
                    CSiTRadar::CloseWindow(parentWin->m_windowId_);
                    CSiTRadar::m_pRadScr->RequestRefresh();
                    return -1;
                }
            }
        }
    }

    if (CSiTRadar::menuState.SFIMode) {
        if (wParam > 0x40 && wParam < 0x5A) {
            char l = MapVirtualKeyA(wParam,2);
            string sfi;
            sfi = l;

            CSiTRadar::ModifySFI(sfi, CSiTRadar::m_pRadScr->GetPlugIn()->FlightPlanSelectASEL());
            CSiTRadar::menuState.SFIMode = false;
            return -1;
        }

    }

    if (
        wParam == VK_F1 ||
        wParam == VK_F3 ||
        wParam == VK_F4 ||
        wParam == VK_F9 ||
        wParam == VK_RETURN ||
        wParam == VK_ESCAPE ||
        wParam == VK_SNAPSHOT
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
                    kbF1 = true;
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

            case VK_F4: {
                if (GetAsyncKeyState(VK_F4) & 0x8000) {
                    kbF4 = true;
                    if (CSiTRadar::m_pRadScr->GetPlugIn()->RadarTargetSelectASEL().IsValid()) {
                        SituPlugin::SendKeyboardPresses({ 0x01 });
                    }
                    return -1;
                }
            }

            case VK_ESCAPE: {
                
                if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
                    if (CSiTRadar::menuState.handoffMode == TRUE  
                        || CSiTRadar::menuState.SFIMode == TRUE) {
                        
                        CSiTRadar::menuState.SFIMode = false;
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
                        if (kbF1) {
                            if (CSiTRadar::menuState.handoffMode == FALSE ||
                                CSiTRadar::menuState.jurisdictionalAC.size() != oldJurisdictionSize) {
                                oldJurisdictionSize = CSiTRadar::menuState.jurisdictionalAC.size();
                                CSiTRadar::menuState.jurisdictionIndex = 0;
                            }

                            CSiTRadar::menuState.handoffMode = TRUE;
                            CSiTRadar::menuState.SFIMode = FALSE;
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
                            kbF1 = false;
                            return -1;
                        }
                        return 0;
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

                    case VK_F4:
                    {
                        if (kbF4) {
                            if (CSiTRadar::menuState.SFIMode == FALSE ||
                                CSiTRadar::menuState.jurisdictionalAC.size() != oldJurisdictionSize) {
                                oldJurisdictionSize = CSiTRadar::menuState.jurisdictionalAC.size();
                                CSiTRadar::menuState.jurisdictionIndex = 0;
                            }

                            CSiTRadar::menuState.SFIMode = TRUE;
                            CSiTRadar::menuState.handoffMode = FALSE;
                            CSiTRadar::menuState.handoffModeStartTime = clock();

                            // ASEL the next aircraft in the handoff priority list
                            if (!CSiTRadar::menuState.jurisdictionalAC.empty()) {
                                if (CSiTRadar::menuState.jurisdictionIndex < CSiTRadar::menuState.jurisdictionalAC.size()) {

                                    // Cycle through jurisdictional aircraft and ASEL them
                                    CSiTRadar::m_pRadScr->GetPlugIn()->SetASELAircraft(CSiTRadar::m_pRadScr->GetPlugIn()->FlightPlanSelect(CSiTRadar::menuState.jurisdictionalAC.at(CSiTRadar::menuState.jurisdictionIndex).c_str()));
                                    CSiTRadar::menuState.jurisdictionIndex++;
                                }
                                else {
                                    CSiTRadar::menuState.jurisdictionIndex = 0;
                                    CSiTRadar::menuState.SFIMode = FALSE;
                                    //SituPlugin::SendKeyboardPresses({ 0x01 });
                                }
                            }

                            CSiTRadar::m_pRadScr->RequestRefresh();

                            held = false;
                            kbF4 = false;
                            return -1;
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

                    case VK_SNAPSHOT: {

                        CSiTRadar::menuState.mvaDisp = !CSiTRadar::menuState.mvaDisp;

                        CSiTRadar::m_pRadScr->GetPlugIn()->SelectActiveSectorfile();
                        for (CSectorElement sectorElement = CSiTRadar::m_pRadScr->GetPlugIn()->SectorFileElementSelectFirst(SECTOR_ELEMENT_FREE_TEXT); sectorElement.IsValid();
                            sectorElement = CSiTRadar::m_pRadScr->GetPlugIn()->SectorFileElementSelectNext(sectorElement, SECTOR_ELEMENT_FREE_TEXT)) {

                            string name = sectorElement.GetName();
                            if(name.find("VFR Call-Up") != string::npos) {

                                    CSiTRadar::m_pRadScr->ShowSectorFileElement(sectorElement, sectorElement.GetComponentName(0), CSiTRadar::menuState.mvaDisp);

                            }
                        }

                        CSiTRadar::m_pRadScr->RefreshMapContent();

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

LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
    
    POINT Pt;
    MOUSEHOOKSTRUCT* mouseStruct = (MOUSEHOOKSTRUCT*)lParam;
    RECT windowRect;
    GetWindowRect(GetActiveWindow(), &windowRect);
    Pt.x = mouseStruct->pt.x - windowRect.left;
    Pt.y = mouseStruct->pt.y - windowRect.top;

    int deltaPx, deltaPy;
    deltaPx = abs(Pt.x - SituPlugin::prevMousePt.x);
    deltaPy = abs(Pt.y - SituPlugin::prevMousePt.y);

    SituPlugin::prevMousePt = Pt;
    
    RECT winRect{};
    GetWindowRect(GetActiveWindow(), &winRect);

    if (nCode == HC_ACTION) {

        if (wParam == WM_MOUSEMOVE) {
            if (CSiTRadar::menuState.MB3hoverOn) {

                if (Pt.x < CPopUpMenu::totalRect.left ||
                    Pt.x > CPopUpMenu::totalRect.right ||
                    Pt.y < CPopUpMenu::totalRect.top ||
                    Pt.y > CPopUpMenu::totalRect.bottom) {
                    CSiTRadar::menuState.MB3hoverOn = false;
                    if (CSiTRadar::m_pRadScr != nullptr) {
                        CSiTRadar::m_pRadScr->RequestRefresh();
                    }
                }
            }
            if (CSiTRadar::menuState.haloCursor) {
                SituPlugin::prevMouseDelta++;
                // if starting from stationary, refresh right away
                if (deltaPx == 0 && deltaPy == 0)
                {
                    SituPlugin::mouseAtRest = true;
                }
                else {
                    SituPlugin::mouseAtRest = false;
                }

                if (SituPlugin::mouseAtRest) {
                    // if at rest, trigger a refresh on first move
                    if (CSiTRadar::m_pRadScr != nullptr) {
                        CSiTRadar::m_pRadScr->RequestRefresh();
                    }
                }

                // only draw every n WM_MOUSEMOVES
                if (SituPlugin::prevMouseDelta > 4) {
                    if (CSiTRadar::m_pRadScr != nullptr) {
                        CSiTRadar::m_pRadScr->RequestRefresh();
                    }
                    SituPlugin::prevMouseDelta = 0;
                }
            }
        }

        if (CSiTRadar::menuState.handoffMode || (CSiTRadar::menuState.MB3menu && !CSiTRadar::menuState.MB3hoverOn) || CSiTRadar::menuState.SFIMode) {

            if (wParam == WM_LBUTTONDOWN || wParam == WM_MBUTTONDOWN || wParam == WM_RBUTTONDOWN) {

                CSiTRadar::menuState.handoffMode = false;
                CSiTRadar::menuState.SFIMode = false;
                CSiTRadar::menuState.MB3menu = false;
                CSiTRadar::m_pRadScr->RequestRefresh();

                if (!CSiTRadar::menuState.jurisdictionalAC.empty()) {
                    SituPlugin::SendKeyboardPresses({ 0x01 });
                }
                return -1;
            }
            return 0;

        } // untoggle h/o if a click happens

        switch (wParam) {
        case WM_MBUTTONDOWN: {
            CSiTRadar::menuState.mouseMMB = true;
            SendMouseClick(MOUSEEVENTF_LEFTDOWN);
            CallNextHookEx(NULL, nCode, wParam, lParam);
            return -1;
        }
        case WM_MBUTTONUP: {
            CSiTRadar::menuState.mouseMMB = false;
            SendMouseClick(MOUSEEVENTF_LEFTUP);
            CallNextHookEx(NULL, nCode, wParam, lParam);
            return -1;
        }
        case WM_MBUTTONDBLCLK: {
        }
        default: {
            return CallNextHookEx(NULL, nCode, wParam, lParam);
        }
        }

    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

SituPlugin::SituPlugin()
	: EuroScopePlugIn::CPlugIn(EuroScopePlugIn::COMPATIBILITY_CODE,
		"VATCANSitu",
		"0.5.7.0",
		"Ron Yan",
		"Attribution-NonCommercial 4.0 International (CC BY-NC 4.0)")
{
    RegisterTagItemType("IFR Release", TAG_ITEM_IFR_REL);
    RegisterTagItemFunction("Request IFR Release", TAG_FUNC_IFR_REL_REQ);
    RegisterTagItemFunction("Grant IFR Release", TAG_FUNC_IFR_RELEASED);

    DWORD appProc = GetCurrentThreadId();
    appHook = SetWindowsHookEx(WH_KEYBOARD, KeyboardProc, NULL, appProc);
    mouseHook = SetWindowsHookEx(WH_MOUSE, MouseProc, NULL, appProc);
}

SituPlugin::~SituPlugin()
{
    UnhookWindowsHookEx(appHook);
    UnhookWindowsHookEx(mouseHook);
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

void SituPlugin::OnAirportRunwayActivityChanged()
{
    if (CSiTRadar::m_pRadScr != nullptr) {
        CSiTRadar::updateActiveRunways(0);
    }
    CSiTRadar::DisplayActiveRunways();
}

void SituPlugin::OnCompilePrivateChat(const char* sSenderCallsign,
    const char* sReceiverCallsign,
    const char* sChatMessage)
{

    string s, cs, msg;
    s = sChatMessage;
    string::size_type pos = s.find(" ");
    if (pos != s.npos) {
        cs = s.substr(0, pos);
        msg = s.substr(pos + 1);
    }

    for (auto& c : cs) {
        c = toupper(c);
    }

    for (auto& c : msg) {
        c = toupper(c);
    }

    if (CSiTRadar::mAcData[cs].pointOutFromMe) {
        if (!strcmp(msg.c_str(), "OK")) {
            CSiTRadar::mAcData[cs].POAcceptTime = clock();
        }
    }

}