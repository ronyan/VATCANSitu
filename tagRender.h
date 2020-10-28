#pragma once
#include "EuroScopePlugIn.h"
#include "CSiTRadar.h"


using namespace std;

class tagRender :
    public EuroScopePlugIn::CRadarScreen
{
public:
    RECT drawTag(CDC hc, const char* callSign, POINT p, bool detailed, CRadarTarget rt) {

        if (detailed == FALSE) {
            int alt = rt.GetPosition().GetPressureAltitude();
            int vs = rt.GetVerticalSpeed();
        }
    };

protected:
    int leaderLen = 10;
    int tagAngle = 30; 


};

