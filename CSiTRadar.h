#pragma once
#include "EuroScopePlugIn.h"
#include <chrono>
#include <string>
#include <map>

using namespace EuroScopePlugIn;
using namespace std;

class CSiTRadar :
    public EuroScopePlugIn::CRadarScreen

{
public:

    CSiTRadar(void);
    virtual ~CSiTRadar(void);

    static void RegisterButton(RECT rect) {

    };

    void OnRefresh(HDC hdc, int phase);

    void CSiTRadar::OnClickScreenObject(int ObjectType,
        const char* sObjectId,
        POINT Pt,
        RECT Area,
        int Button);

    int PixelsPerNM(void)
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

        int pixnm = (int)round(pixpernm);

        return pixnm;
    };

    inline virtual void OnAsrContentToBeClosed(void)
    {
        SaveDataToAsr("tagfamily", "Tag Family", "CSiT");
        SaveDataToAsr("above", "Filter Above", "600");
        SaveDataToAsr("below", "Filter Below", "0");
        delete this;
    };

protected:
    void ButtonToScreen(CSiTRadar* radscr, RECT rect, string btext, int itemtype);

    void OnAsrContentLoaded();
    
    bool halotool = FALSE;
    bool mousehalo = FALSE;
    bool pressed = FALSE;
    int haloidx = 1; // default halo radius = 3, corresponds to index of the halooptions

    map<string, bool> hashalo;
    double halorad = 3;
    string halooptions[9] = { "0.5", "3", "5", "10", "15", "20", "30", "60", "80" };
    string radtype;
    int above; 
    int below;
    string controllerID;
};

