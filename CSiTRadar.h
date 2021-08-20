#pragma once
#include "EuroScopePlugIn.h"
#include "SituPlugin.h"
#include <chrono>
#include <string>
#include <map>
#include <set>
#include <unordered_map>
#include <array>
#include <regex>
#include <math.h>
#include <gdiplus.h>
#include <deque>
#include "constants.h"
#include "pch.h"
#include "wxRadar.h"

using namespace EuroScopePlugIn;
using namespace std;

struct ACList {
    POINT p{ 0, 0 };
    int listType{ 0 };
    bool collapsed{ false };
};

struct ACData {
    bool hasVFRFP;
    bool isADSB;
    bool isRVSM;
    bool isMedevac{ FALSE };
    int tagType{ 0 };
    bool isHandoff{ FALSE };
    bool isHandoffToMe{ FALSE };

    bool isJurisdictional{ FALSE };
    bool isOnScreen{ FALSE };
    bool isDestAprt{ FALSE };
    bool isQuickLooked{ false };

    int tagWidth{ 65 }; 
    string CID;
    bool extAlt{ FALSE };
    int destLabelType{ 0 };
};

struct buttonStates {
    bool haloTool;
    bool ptlTool;
    bool showExtrapFP{ FALSE };
    bool filterBypassAll{ FALSE };
    int ptlLength{ 3 };
    bool ptlAll{ FALSE };
    int haloRad;
    bool quickLook{ FALSE };
    bool quickLookDelta{ false };
    bool extAltToggle{ FALSE };
    int numJurisdictionAC{ 0 };
    bool destAirport{ false };


    bool wxAll{ FALSE };
    bool wxHigh{ FALSE };
    bool wxOn{ FALSE };
    bool mvaDisp{ false };

    set<string> activeArpt;
    map<string, bool> nearbyCJS;
    

    bool SFIMode{};
    bool handoffMode{};
    deque<string> jurisdictionalAC{}; // AC under jurisdiction + active handoffs to jurisdiction
    clock_t handoffModeStartTime{};
    size_t jurisdictionIndex{ 0 };

    bool destArptOn[5];
    string destICAO[5];
    bool destEST;
    bool destVFR;
    bool destDME;

    bool mouseMMB{ false };
};

class CAircraftList {

public:
    POINT m_p{ 0,0 };
    bool m_collapsed = false; 
    int m_listType{};
    int listType{};
    CRadarScreen* radscr;
    unordered_map<string, ACData>* acData{};
    unordered_map<int, ACList> acLists;
    CDC* dc;

    CAircraftList() 
        : m_p({ 0,0 }) 
    {}

    void Draw() 
    {
        int sDC = dc->SaveDC();
        CFont font;
        LOGFONT lgfont;
        memset(&lgfont, 0, sizeof(LOGFONT));
        lgfont.lfHeight = 14;
        lgfont.lfWeight = 500;
        strcpy_s(lgfont.lfFaceName, _T("EuroScope"));
        font.CreateFontIndirect(&lgfont);
        dc->SetTextColor(C_WHITE);
        dc->SelectObject(font);
        string header;

        // Draw the heading
        RECT listHeading{};
        listHeading.left = m_p.x;
        listHeading.top = m_p.y;

        // Draw the arrow
        HPEN targetPen = CreatePen(PS_SOLID, 1, C_WHITE);;
        HBRUSH targetBrush = CreateSolidBrush(C_WHITE);

        dc->SelectObject(targetPen);
        dc->SelectObject(targetBrush);

        bool collapsed{ false };
        bool showArrow = false;



        dc->RestoreDC(sDC);
        DeleteObject(font);
        DeleteObject(targetPen);
        DeleteObject(targetBrush);
    }
    void Move(int x, int y)
    {
        m_p.x = x;
        m_p.y = y;
    }
    void Collapse()
    {
        m_collapsed = true;
    }

    void Expand() 
    {
        m_collapsed = false;
    }
    
    int GetListType() 
    {
        return m_listType;
    }
};


class CSiTRadar :
    public EuroScopePlugIn::CRadarScreen

{
public:

    CSiTRadar(void);
    virtual ~CSiTRadar(void);

    // Pointer back at screen
    static CRadarScreen* m_pRadScr;

    static unordered_map<string, ACData> mAcData;
    static unordered_map<string, int> tempTagData;
    static unordered_map<string, clock_t> hoAcceptedTime;
    static map<string, bool> destAirportList;

    static buttonStates menuState;

    double wxRadLat;
    double wxRadLong;
    int wxRadZL;

    virtual void OnAsrContentLoaded(bool Loaded);
    void OnAsrContentToBeSaved();
    inline virtual void OnFlightPlanFlightPlanDataUpdate(CFlightPlan FlightPlan);
    inline virtual void OnFlightPlanDisconnect(CFlightPlan FlightPlan);
    static void updateActiveRunways(CRadarScreen* rad);
    inline virtual void OnControllerPositionUpdate(CController Controller);
    inline virtual void OnControllerDisconnect(CController Controller);

    static void RegisterButton(RECT rect) {

    };

    void OnRefresh(HDC hdc, int phase);

    void CSiTRadar::OnButtonDownScreenObject(int ObjectType,
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
    void ButtonToScreen(CSiTRadar* radscr, const RECT& rect, const string& btext, int itemtype);
    void DrawACList(POINT p, CDC* dc, unordered_map<string, ACData>& ac, int listType);

    // helper functions
    clock_t time = clock();
    clock_t oldTime = clock();
    clock_t lastWxRefresh = 0;
    clock_t lastMetarRefresh = 0;
    clock_t lastAtisRefresh = 0;

    // menu states
    bool halotool = FALSE;
    bool mousehalo = FALSE;
    int menuLayer = 0;
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
    int haloOptions[6] = { 3,5,10,15,20,25 };
    int ptlOptions[20] = { 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,20,25,30,35,40 };
    string controllerID;
    string radtype;
};
