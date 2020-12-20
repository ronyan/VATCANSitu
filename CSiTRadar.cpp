/*	
2020 CTP Plugin-minimal version 
VATUSA version
*/

#include "pch.h"
#include "CSiTRadar.h"
#include "TopMenu.h"
#include "SituPlugin.h"
#include "vatsimAPI.h"

map<string, ACData> CSiTRadar::mAcData;
map<string, string> CSiTRadar::slotTime;
bool CSiTRadar::canAmend;
int CSiTRadar::refreshStatus;
int CSiTRadar::amendStatus;
string CSiTRadar::eventCode;

CSiTRadar::CSiTRadar()
{	
	CSiTRadar::eventCode = "Enter Code";
	CDataHandler::GetVatsimAPIData();

	time = clock();
	oldTime = clock();
}

void CSiTRadar::OnRefresh(HDC hdc, int phase)
{


	if (phase != REFRESH_PHASE_AFTER_TAGS && phase != REFRESH_PHASE_BEFORE_TAGS) {
		return;
	}

	// set up the drawing renderer
	CDC dc;
	dc.Attach(hdc);

	if (phase == REFRESH_PHASE_AFTER_TAGS) {

		for (CRadarTarget radarTarget = GetPlugIn()->RadarTargetSelectFirst(); radarTarget.IsValid();
			radarTarget = GetPlugIn()->RadarTargetSelectNext(radarTarget))
		{
			POINT p = ConvertCoordFromPositionToPixel(radarTarget.GetPosition().GetPosition());
			string callSign = radarTarget.GetCallsign();

			if (radarTarget.GetPosition().GetRadarFlags() != 0) {

				if (mAcData[callSign].hasCTP) {
					CFont font;
					LOGFONT lgfont;

					memset(&lgfont, 0, sizeof(LOGFONT));
					lgfont.lfWeight = 500;
					strcpy_s(lgfont.lfFaceName, _T("EuroScope"));
					lgfont.lfHeight = 12;
					font.CreateFontIndirect(&lgfont);

					dc.SetTextColor(RGB(255,132,0));

					RECT rectPAM;
					rectPAM.left = p.x - 9;
					rectPAM.right = p.x + 75; rectPAM.top = p.y + 8;	rectPAM.bottom = p.y + 30;

					dc.DrawText(CDataHandler::tagLabel.c_str(), &rectPAM, DT_LEFT);

					DeleteObject(font);
				}
			}
		}

		POINT menu;
		RECT but;

		menu.y = 40;
		menu.x = 10;

		but = TopMenu::DrawButton(&dc, menu, 60, 23, "Refresh", autoRefresh);
		ButtonToScreen(this, but, "Refresh Slot Data", BUTTON_MENU_REFRESH);

		
		menu.x = 80;
		but = TopMenu::DrawButton(&dc, menu, 60, 23, CSiTRadar::eventCode.c_str(), 0);
		ButtonToScreen(this, but, "Settings", BUTTON_MENU_SETTINGS);
		
	}

	if (autoRefresh) {
		time = clock();
		if ((time - oldTime) / CLOCKS_PER_SEC > CDataHandler::refreshInterval) {
			
			CAsync* data = new CAsync();
			data->Plugin = GetPlugIn();
			_beginthread(CDataHandler::GetVatsimAPIData, 0, (void*) data);
			oldTime = clock();
		}
	}
	dc.Detach();
}

void CSiTRadar::OnClickScreenObject(int ObjectType,
	const char* sObjectId,
	POINT Pt,
	RECT Area,
	int Button)
{
	if (ObjectType == BUTTON_MENU_REFRESH) {
		if (Button == BUTTON_LEFT) { 
			
			CSiTRadar::canAmend = FALSE;

			CAsync* data = new CAsync();
			data->Plugin = GetPlugIn();
			_beginthread(CDataHandler::GetVatsimAPIData, 0, (void*) data);
				
			oldTime = clock(); }
		if (Button == BUTTON_RIGHT) { autoRefresh = !autoRefresh; }
	}

	if (ObjectType == BUTTON_MENU_SETTINGS) {
		if (Button == BUTTON_LEFT) {
			GetPlugIn()->OpenPopupEdit(Area, FUNCTION_SET_URL, CSiTRadar::eventCode.c_str());
		}
	}
}

void CSiTRadar::ButtonToScreen(CSiTRadar* radscr, RECT rect, string btext, int itemtype) {
	AddScreenObject(itemtype, btext.c_str(), rect, 0, "");
}

void CSiTRadar::OnFlightPlanDisconnect(CFlightPlan FlightPlan) {
	string callSign = FlightPlan.GetCallsign();

	mAcData.erase(callSign);
}

void CSiTRadar::OnFunctionCall(int FunctionId,
	const char* sItemString,
	POINT Pt,
	RECT Area) {
	if (FunctionId == FUNCTION_SET_URL) {
		try {
			CSiTRadar::eventCode = sItemString;
		}
		catch (...) {}
	}
}

CSiTRadar::~CSiTRadar()
{
}