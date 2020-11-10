#pragma once
#include "EuroScopePlugIn.h"
#include "SituPlugin.h"
#include <chrono>
#include <string>
#include <map>
#include <iostream>
#include <array>
#include <regex>
#include <math.h>
#include <gdiplus.h>
#include "constants.h"
#include "pch.h"

using namespace EuroScopePlugIn;
using namespace std;

struct ACData {
    bool hasVFRFP;
    bool isADSB;
    bool isRVSM;
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

    static map<string, bool> isCTP;
    static map<string, ACData> mAcData; 
    static map<string, string> CSiTRadar::slotTime;

    virtual void OnAsrContentLoaded(bool Loaded);
    void OnAsrContentToBeSaved();
    inline virtual void OnFlightPlanFlightPlanDataUpdate(CFlightPlan FlightPlan);
    inline virtual void OnFlightPlanDisconnect(CFlightPlan FlightPlan);

    static void RegisterButton(RECT rect) {

    };

    void OnRefresh(HDC hdc, int phase);

    void CSiTRadar::OnClickScreenObject(int ObjectType,
        const char* sObjectId,
        POINT Pt,
        RECT Area,
        int Button);

    void CSiTRadar::OnMoveScreenObject(int ObjectType, 
        const char* sObjectId, 
        POINT Pt, 
        RECT Area, 
        bool Released);

    void OnFunctionCall(int FunctionId, const char* sItemString, POINT Pt, RECT Area);

    double RadRange(void)
    {
        RECT radarea = GetRadarArea();
        POINT pl = CPoint((int)radarea.left, (int)radarea.top);
        POINT pr = CPoint((int)radarea.right, (int)radarea.top);

        CPosition posL = ConvertCoordFromPixelToPosition(pl);
        CPosition posR = ConvertCoordFromPixelToPosition(pr);

        double raddist = posL.DistanceTo(posR);

        return raddist;
    }

    double PixelsPerNM(void)
    {
        RECT radarea = GetRadarArea();
        POINT pl = CPoint((int)radarea.left, (int)radarea.top);
        POINT pr = CPoint((int)radarea.right, (int)radarea.top);

        CPosition posL = ConvertCoordFromPixelToPosition(pl);
        CPosition posR = ConvertCoordFromPixelToPosition(pr);

        double raddist = posL.DistanceTo(posR);

        double rr = radarea.right;
        double rl = radarea.left;
        double radwidth = rr - rl;

        double pixpernm = radwidth / raddist;

        return pixpernm;
    };

    inline virtual void OnAsrContentToBeClosed(void) {

        // saving settings to the ASR file

        /*
        const char* sv = radtype.c_str();
        SaveDataToAsr("tagfamily", "Tag Family", sv);
        */

        delete this;
    };

protected:
    void ButtonToScreen(CSiTRadar* radscr, RECT rect, string btext, int itemtype);

    // helper functions
    clock_t time = clock();
    clock_t oldTime = clock();

    // menu states
    bool halotool = FALSE;
    bool mousehalo = FALSE;
    bool altFilterOpts = FALSE;
    bool altFilterOn = TRUE;
    bool autoRefresh = FALSE;

    bool pressed = FALSE;
    int haloidx = 1; // default halo radius = 3, corresponds to index of the halooptions

    clock_t halfSec = 0;
    bool halfSecTick = FALSE; // toggles on and off every half second

    map<string, bool> hashalo;
    map<string, bool> isBlinking;
    map<string, bool> isHandOffHold;
    map<string, string> ppsCJS;

    // ADSB Radar Site
    CPosition adsbSite; 

    // Tag Properties
    map<string, POINT> tagOffset; // the centre of the aircraft tag
    map<string, POINT> connectorOrigin; // the Tag end of the connector, this flips from right side to left side

    // menu functions
    RECT rLLim = { 0, 0, 10, 10 };
    RECT rHLim = { 0, 0, 10, 10 };

    // menu settings
    int altFilterLow = 0;
    int altFilterHigh = 0; 

    double halorad = 3;
    string halooptions[9] = { "0.5", "3", "5", "10", "15", "20", "30", "60", "80" };
    string controllerID;
    string radtype;
};
