#pragma once
#include <EuroScopePlugIn.h>
#include <vector>
#include <string>

struct ACList {
    POINT p{ 0, 0 };
    int listType{ 0 };
    bool collapsed{ false };
};

struct inactiveRunway {
    EuroScopePlugIn::CPosition end1;
    EuroScopePlugIn::CPosition end2;
};

struct ACRoute {
    std::vector<EuroScopePlugIn::CPosition> route_fix_positions;
    std::vector<std::string> fix_names;
    int nearestPtIdx;
    int directPtIdx;
};

class SituPlugin :
    public EuroScopePlugIn::CPlugIn
{
public:

    SituPlugin();
    virtual ~SituPlugin();
    EuroScopePlugIn::CRadarScreen* OnRadarScreenCreated(const char* sDisplayName, bool NeedRadarContent, bool GeoReferenced, bool CanBeSaved, bool CanBeCreated);

    virtual void OnGetTagItem(EuroScopePlugIn::CFlightPlan FlightPlan,
        EuroScopePlugIn::CRadarTarget RadarTarget,
        int ItemCode,
        int TagData,
        char sItemString[16],
        int* pColorCode,
        COLORREF* pRGB,
        double* pFontSize);

    inline virtual void OnFunctionCall(int FunctionId,
        const char* sItemString,
        POINT Pt,
        RECT Area);

    virtual void OnAirportRunwayActivityChanged();

    inline virtual void OnCompilePrivateChat(const char* sSenderCallsign, const char* sReceiverCallsign, const char* sChatMessage);

    static void SendKeyboardPresses(std::vector<WORD> message);
    static void SendKeyboardString(std::string str);
    static POINT prevMousePt;
    static int prevMouseDelta;
    static bool mouseAtRest;

};