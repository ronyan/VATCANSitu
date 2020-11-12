#pragma once
#include "EuroScopePlugIn.h"
#include "SituPlugin.h"
#include <chrono>
#include <string>
#include <map>
#include <iostream>
#include "pch.h"

using namespace EuroScopePlugIn;
using namespace std;

struct ACData {
    bool hasCTP;
    string slotTime;
    string CID;
};

class CSiTRadar :
    public EuroScopePlugIn::CRadarScreen

{
public:

    CSiTRadar(void);
    virtual ~CSiTRadar(void);

    static map<string, ACData> mAcData; 
    static map<string, string> slotTime;

    inline virtual void OnFlightPlanDisconnect(CFlightPlan FlightPlan);
    static void RegisterButton(RECT rect) {};

    void OnRefresh(HDC hdc, int phase);

    void CSiTRadar::OnClickScreenObject(int ObjectType,
        const char* sObjectId,
        POINT Pt,
        RECT Area,
        int Button);

    inline virtual void OnAsrContentToBeClosed(void) {

        delete this;
    };

protected:
    void ButtonToScreen(CSiTRadar* radscr, RECT rect, string btext, int itemtype);

    const int BUTTON_MENU_RELOCATE = 1200;
    BOOL autoRefresh = FALSE;
    clock_t time; 
    clock_t oldTime;
};
