#pragma once
#include "EuroScopePlugIn.h"
#include "SituPlugin.h"
#include <chrono>
#include <string>
#include <map>
#include <unordered_map>
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
    int tagType{ 0 };
    bool isHandoff{ FALSE };
    bool isHandoffToMe{ FALSE };
    int tagWidth{ 65 }; 
    string CID;
    bool extAlt{ FALSE };
};

struct buttonStates {
    bool haloTool;
    bool ptlTool;
    bool showExtrapFP{ FALSE };
    bool filterBypassAll{ FALSE };
    double ptlLength;
    bool quickLook{ FALSE };
    bool extAltToggle{ FALSE };
    bool wxAll{ FALSE };
    bool wxHigh{ FALSE };
    bool wxOn{ FALSE };
};

class CSiTRadar :
    public EuroScopePlugIn::CRadarScreen

{
public:

    CSiTRadar(void);
    virtual ~CSiTRadar(void);

    static unordered_map<string, ACData> mAcData;
    static unordered_map<string, int> tempTagData;
    static unordered_map<string, clock_t> hoAcceptedTime;

    static double magvar;
    static buttonStates menuState;

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
    static bool halfSecTick; // toggles on and off every half second

    inline virtual void OnDoubleClickScreenObject(int ObjectType,
        const char* sObjectId,
        POINT Pt,
        RECT Area,
        int Button);

protected:
    void ButtonToScreen(CSiTRadar* radscr, RECT rect, string btext, int itemtype);

    // helper functions
    clock_t time = clock();
    clock_t oldTime = clock();
    clock_t lastWxRefresh = 0;

    // menu states
    bool halotool = FALSE;
    bool mousehalo = FALSE;
    bool altFilterOpts = FALSE;
    bool altFilterOn = TRUE;
    bool autoRefresh = FALSE;

    bool pressed = FALSE;
    int haloidx = 1; // default halo radius = 3, corresponds to index of the halooptions

    clock_t halfSec = 0;

    map<string, bool> hashalo;
    map<string, bool> hasPTL;
    map<string, bool> isBlinking;
    map<string, bool> isHandOffHold;
    map<string, string> ppsCJS;

    // ADSB Radar Site
    CPosition adsbSite; 

    // Tag Properties
    unordered_map<string, POINT> rtagOffset;
    unordered_map<string, POINT> fptagOffset;// the centre of the aircraft tag
    unordered_map<string, POINT> connectorOrigin; // the Tag end of the connector, this flips from right side to left side

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
