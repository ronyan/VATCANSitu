/*	Drawing on the main radar screen is done with this file
	This draws:
	1. CSiT Tools Menu
	2. VFR radar target
	3. Mouse halo if enabled
	4. Aircraft specific halos if enabled
	5. PTLS for aircrafts if enabled


	// DEBUG

	POINT q;
	q = ConvertCoordFromPositionToPixel(adsbSite);
	HPEN targetPen;
	targetPen = CreatePen(PS_SOLID, 1, C_WHITE);
	dc.SelectObject(targetPen);

	dc.MoveTo(q.x, q.y);
	dc.LineTo(q.x + 20, q.y + 20);

	CFont font;
	LOGFONT lgfont;
	memset(&lgfont, 0, sizeof(LOGFONT));
	lgfont.lfHeight = 14;
	lgfont.lfWeight = 500;
	strcpy_s(lgfont.lfFaceName, _T("EuroScope"));
	font.CreateFontIndirect(&lgfont);
	dc.SelectObject(font);

	RECT debug;
	debug.top = 250;
	debug.left = 250;
	dc.DrawText(to_string(adsbSite.m_Latitude).c_str(), &debug, DT_LEFT);
	debug.top += 10;

	DeleteObject(font);

	DeleteObject(targetPen);

	// DEBUG
*/

#include "pch.h"
#include "CSiTRadar.h"
#include "HaloTool.h"
#include "constants.h"
#include "TopMenu.h"
#include "SituPlugin.h"
#include "ACTag.h"
#include "PPS.h"
#include "wxRadar.h"
#include <chrono>

using namespace Gdiplus;

// Initialize Static Members
unordered_map<string, ACData> CSiTRadar::mAcData;
unordered_map<string, int> CSiTRadar::tempTagData;
map<string, menuButton> TopMenu::menuButtons;
unordered_map<string, clock_t> CSiTRadar::hoAcceptedTime;
buttonStates CSiTRadar::menuState = {};
double CSiTRadar::magvar = 361;
bool CSiTRadar::halfSecTick = FALSE;

CSiTRadar::CSiTRadar()
{
	halfSec = clock();
	halfSecTick = FALSE;

	menuState.ptlLength = 3;
	menuState.ptlTool = FALSE;

	wxRadar::GetRainViewerJSON(this);
	wxRadar::parseRadarPNG(this);

	CSiTRadar::mAcData.reserve(64);

	time = clock();
	oldTime = clock();
}

CSiTRadar::~CSiTRadar()
{
}

void CSiTRadar::OnRefresh(HDC hdc, int phase)
{
	// get cursor position and screen info
	POINT p;

	if (GetCursorPos(&p)) {
		if (ScreenToClient(GetActiveWindow(), &p)) {}
	}

	RECT radarea = GetRadarArea();
	
	// time based functions
	double time = ((double)clock() - (double)halfSec) / ((double)CLOCKS_PER_SEC);
	if (time >= 0.5) {
		halfSec = clock();
		halfSecTick = !halfSecTick;
	}

	// set up the drawing renderer
	CDC dc;
	dc.Attach(hdc);

	Graphics g(hdc);

	double pixnm = PixelsPerNM();

	if (phase == REFRESH_PHASE_AFTER_TAGS) {

		// Draw the mouse halo before menu, so it goes behind it
		if (mousehalo == TRUE) {
			HaloTool::drawHalo(&dc, p, halorad, pixnm);
			RequestRefresh();
		}

		for (CRadarTarget radarTarget = GetPlugIn()->RadarTargetSelectFirst(); radarTarget.IsValid();
			radarTarget = GetPlugIn()->RadarTargetSelectNext(radarTarget))
		{
			// to pull the magvar value from a plane; since can't get it easily from .sct -- do this only once
			if (magvar == 361) {
				magvar = (double)radarTarget.GetPosition().GetReportedHeading() - (double)radarTarget.GetPosition().GetReportedHeadingTrueNorth();
			}

			// altitude filtering 
			if (altFilterOn && radarTarget.GetPosition().GetPressureAltitude() < altFilterLow * 100 && !menuState.filterBypassAll) {
				continue;
			}

			if (altFilterOn && altFilterHigh > 0 && radarTarget.GetPosition().GetPressureAltitude() > altFilterHigh * 100 && !menuState.filterBypassAll) {
				continue;
			}

			POINT p = ConvertCoordFromPositionToPixel(radarTarget.GetPosition().GetPosition());
			string callSign = radarTarget.GetCallsign();

			// Draw PTL
			if (hasPTL.find(radarTarget.GetCallsign()) != hasPTL.end()) {
				HaloTool::drawPTL(&dc, radarTarget, p, 3, pixnm);
			}

			// Get information about the Aircraft/Flightplan
			bool isCorrelated = radarTarget.GetCorrelatedFlightPlan().IsValid();
			bool isVFR = mAcData[callSign].hasVFRFP;
			bool isRVSM = mAcData[callSign].isRVSM;
			bool isADSB = mAcData[callSign].isADSB;

			if (!isCorrelated && !isADSB) {
				mAcData[callSign].tagType = 3; // sets this if RT is uncorr
			} 
			else if (isCorrelated && mAcData[callSign].tagType == 3) { mAcData[callSign].tagType = 0; } // only sets once to go from uncorr to corr
			// then allows it to be opened closed etc

			COLORREF ppsColor;

			// logic for the color of the PPS
			if (radarTarget.GetPosition().GetRadarFlags() == 0) { ppsColor = C_PPS_YELLOW; }
			else if (radarTarget.GetPosition().GetRadarFlags() == 1 && !isCorrelated) { ppsColor = C_PPS_MAGENTA; }
			else if (!strcmp(radarTarget.GetPosition().GetSquawk(), "7600") || !strcmp(radarTarget.GetPosition().GetSquawk(), "7700")) { ppsColor = C_PPS_RED; }
			else if (isVFR) { ppsColor = C_PPS_ORANGE; }
			else { ppsColor = C_PPS_YELLOW; }

			if (radarTarget.GetPosition().GetTransponderI() == TRUE && halfSecTick) { ppsColor = C_WHITE; }

			RECT prect = CPPS::DrawPPS(&dc, isCorrelated, isVFR, isADSB, isRVSM, radarTarget.GetPosition().GetRadarFlags(), ppsColor, radarTarget.GetPosition().GetSquawk(), p);
			AddScreenObject(AIRCRAFT_SYMBOL, callSign.c_str(), prect, FALSE, "");

			if (radarTarget.GetPosition().GetRadarFlags() != 0 ) {
				CACTag::DrawRTACTag(&dc, this, &radarTarget, &radarTarget.GetCorrelatedFlightPlan(), &rtagOffset);
			}

			// ADSB targets; if no primary or secondary radar, but the plane has ADSB equipment suffix (assumed space based ADS-B with no gaps)
			if (radarTarget.GetPosition().GetRadarFlags() == 0
				&& isADSB) {
				if (mAcData[callSign].tagType != 0 && mAcData[callSign].tagType != 1) { mAcData[callSign].tagType = 1; }
				CACTag::DrawRTACTag(&dc, this, &radarTarget, &GetPlugIn()->FlightPlanSelect(callSign.c_str()), &rtagOffset);
				CACTag::DrawRTConnector(&dc, this, &radarTarget, &GetPlugIn()->FlightPlanSelect(callSign.c_str()), C_PPS_YELLOW, &rtagOffset);

			}

			// Tag Level Logic
			if (radarTarget.GetCorrelatedFlightPlan().GetTrackingControllerIsMe()) {
				CSiTRadar::mAcData[radarTarget.GetCallsign()].tagType = 1; // alpha tag if you have jurisdiction over the aircraft

				// if you are handing off to someone
				if (strcmp(radarTarget.GetCorrelatedFlightPlan().GetHandoffTargetControllerId(), "") != 0) {
					mAcData[callSign].isHandoff = TRUE;
				}
			}

			// Once the handoff is complete, 
			if (mAcData[callSign].isHandoff == TRUE && strcmp(radarTarget.GetCorrelatedFlightPlan().GetHandoffTargetControllerId(), "") == 0) {
				mAcData[callSign].isHandoff = FALSE;
				// record the time of handoff acceptance
				hoAcceptedTime[callSign] = clock();
			}

			// Post handoff blinking and then close the tag
			if ( hoAcceptedTime.find(callSign) != hoAcceptedTime.end() && (clock() - hoAcceptedTime[callSign]) / CLOCKS_PER_SEC > 12) {
				hoAcceptedTime.erase(callSign);
				
				// if quick look is open, defer closing the tag until quicklook is off;
				if (!menuState.quickLook) {
					mAcData[callSign].tagType = 0;
				}
				if (menuState.quickLook) {
					tempTagData[callSign] = 0;
				}
			}

			// Once the handoff to me is completed,
			if (mAcData[callSign].isHandoffToMe == TRUE && strcmp(radarTarget.GetCorrelatedFlightPlan().GetHandoffTargetControllerId(), "") == 0) {
				mAcData[callSign].isHandoffToMe = FALSE;
			}

			// Open a bravo tag, during a handoff to you
			if (strcmp(radarTarget.GetCorrelatedFlightPlan().GetHandoffTargetControllerId(), GetPlugIn()->ControllerMyself().GetPositionId()) == 0 &&
				strcmp(radarTarget.GetCorrelatedFlightPlan().GetHandoffTargetControllerId(), "") != 0) {
				CSiTRadar::mAcData[radarTarget.GetCallsign()].tagType = 1;
				mAcData[callSign].isHandoffToMe = TRUE;
			}

			// Handoff warning system: if the plane is within 2 minutes of exiting your airspace, CJS will blink

			COLORREF cjsColor = C_PPS_YELLOW;

			if (radarTarget.GetCorrelatedFlightPlan().GetTrackingControllerIsMe()) {
				if (radarTarget.GetCorrelatedFlightPlan().GetSectorExitMinutes() <= 2
					&& radarTarget.GetCorrelatedFlightPlan().GetSectorExitMinutes() >= 0
					&& halfSecTick == TRUE
					&& strcmp(radarTarget.GetCorrelatedFlightPlan().GetHandoffTargetControllerId(), "") == 0) {
					cjsColor = C_WHITE;
				}
			}

			// show CJS for controller tracking aircraft
			if ((radarTarget.GetPosition().GetRadarFlags() >= 2 && isCorrelated) || CSiTRadar::mAcData[radarTarget.GetCallsign()].isADSB) {
				string CJS = GetPlugIn()->FlightPlanSelect(callSign.c_str()).GetTrackingControllerId();

				CFont font;
				LOGFONT lgfont;

				memset(&lgfont, 0, sizeof(LOGFONT));
				lgfont.lfWeight = 500;
				strcpy_s(lgfont.lfFaceName, _T("EuroScope"));
				lgfont.lfHeight = 12;
				font.CreateFontIndirect(&lgfont);

				dc.SelectObject(font);
				dc.SetTextColor(cjsColor);

				RECT rectCJS;
				rectCJS.left = p.x - 6;
				rectCJS.right = p.x + 75;
				rectCJS.top = p.y - 18;
				rectCJS.bottom = p.y;

				dc.DrawText(CJS.c_str(), &rectCJS, DT_LEFT);

				DeleteObject(font);
			}
			
			// plane halo looks at the <map> hashalo to see if callsign has a halo, if so, draws halo
			if (hashalo.find(radarTarget.GetCallsign()) != hashalo.end()) {
				HaloTool::drawHalo(&dc, p, halorad, pixnm);
			}

		}

		// Flight plan loop. Goes through flight plans, and if not correlated will display
		for (CFlightPlan flightPlan = GetPlugIn()->FlightPlanSelectFirst(); flightPlan.IsValid();
			flightPlan = GetPlugIn()->FlightPlanSelectNext(flightPlan)) {
			
			if (flightPlan.GetCorrelatedRadarTarget().IsValid() || menuState.showExtrapFP == FALSE) { continue; } 

			// if the flightplan does not have a correlated radar target
			if (flightPlan.GetFPState() == FLIGHT_PLAN_STATE_SIMULATED
				&& !mAcData[flightPlan.GetCallsign()].isADSB) {

				CACTag::DrawFPACTag(&dc, this, &flightPlan.GetCorrelatedRadarTarget(), &flightPlan, &fptagOffset);
				CACTag::DrawFPConnector(&dc, this, &flightPlan.GetCorrelatedRadarTarget(), &flightPlan, C_PPS_ORANGE, &fptagOffset);

				POINT p = ConvertCoordFromPositionToPixel(flightPlan.GetFPTrackPosition().GetPosition());

				// draw the orange airplane symbol (credits andrewogden1678)
				GraphicsContainer gCont;
				gCont =  g.BeginContainer();

				// Airplane icon 

				Point points[19] = {
					Point(0,-6),
					Point(-1,-5),
					Point(-1,-2),
					Point(-8,3),
					Point(-8,4),
					Point(-1,2),
					Point(-1,6),
					Point(-4,8),
					Point(-4,9),
					Point(0,8),
					Point(4,9),
					Point(4,8),
					Point(1,6),
					Point(1,2),
					Point(8,4),
					Point(8,3),
					Point(1,-2),
					Point(1,-5),
					Point(0,-6)
				};

				g.RotateTransform((REAL)flightPlan.GetFPTrackPosition().GetReportedHeading());
				g.TranslateTransform((REAL)p.x, (REAL)p.y, MatrixOrderAppend);

				SolidBrush orangeBrush(Color(255, 242, 120, 57));

				g.FillPolygon(&orangeBrush, points, 19);
				g.EndContainer(gCont);

				DeleteObject(&orangeBrush);
								
			}

		}


		// Draw the CSiT Tools Menu; starts at rad area top left then moves right
		// this point moves to the origin of each subsequent area
		POINT menutopleft = CPoint(radarea.left, radarea.top); 

		TopMenu::DrawBackground(dc, menutopleft, radarea.right, 60);
		RECT but;

		// small amount of padding;
		menutopleft.y += 6;
		menutopleft.x += 10;

		// screen range, dummy buttons, not really necessary in ES.
		but = TopMenu::DrawButton(&dc, menutopleft, 70, 23, "Relocate", autoRefresh);
		ButtonToScreen(this, but, "Alt Filt Opts", BUTTON_MENU_RELOCATE);
		menutopleft.y += 25;

		TopMenu::DrawButton(&dc, menutopleft, 35, 23, "Zoom", 0); 
		menutopleft.x += 35;
		TopMenu::DrawButton(&dc, menutopleft, 35, 23, "Pan", 0);
		menutopleft.y -= 25;
		menutopleft.x += 55;
		
		// horizontal range calculation
		int range = (int)round(RadRange());
		string rng = to_string(range);
		TopMenu::MakeText(dc, menutopleft, 50, 15, "Range");
		menutopleft.y += 15;

		// 109 pix per in on my monitor
		int nmIn = (int)round(109 / pixnm);
		string nmtext = "1\" = " + to_string(nmIn) + "nm";
		TopMenu::MakeText(dc, menutopleft, 50, 15, nmtext.c_str());
		menutopleft.y += 17;

		TopMenu::MakeDropDown(dc, menutopleft, 40, 15, rng.c_str());

		menutopleft.x += 80;
		menutopleft.y -= 32;

		// altitude filters

		but = TopMenu::DrawButton(&dc, menutopleft, 50, 23, "Alt Filter", altFilterOpts);
		ButtonToScreen(this, but, "Alt Filt Opts", BUTTON_MENU_ALT_FILT_OPT);
		
		menutopleft.y += 25;

		string altFilterLowFL = to_string(altFilterLow);
		if (altFilterLowFL.size() < 3) {
			altFilterLowFL.insert(altFilterLowFL.begin(), 3 - altFilterLowFL.size(), '0');
		}
		string altFilterHighFL = to_string(altFilterHigh);
		if (altFilterHighFL.size() < 3) {
			altFilterHighFL.insert(altFilterHighFL.begin(), 3 - altFilterHighFL.size(), '0');
		}

		string filtText = altFilterLowFL + string(" - ") + altFilterHighFL;
		but = TopMenu::DrawButton(&dc, menutopleft, 50, 23, filtText.c_str(), altFilterOn);
		ButtonToScreen(this, but, "", BUTTON_MENU_ALT_FILT_ON);
		menutopleft.y -= 25;
		menutopleft.x += 65; 

		// separation tools
		string haloText = "Halo " + halooptions[haloidx];
		but = TopMenu::DrawButton(&dc, menutopleft, 45, 23, haloText.c_str(), halotool);
		ButtonToScreen(this, but, "Halo", BUTTON_MENU_HALO_OPTIONS);

		menutopleft.y = menutopleft.y + 25;
		but = TopMenu::DrawButton(&dc, menutopleft, 45, 23, "PTL 3", CSiTRadar::menuState.ptlTool);
		ButtonToScreen(this, but, "PTL", BUTTON_MENU_PTL_TOOL);

		menutopleft.y = menutopleft.y - 25;
		menutopleft.x = menutopleft.x + 47;
		TopMenu::DrawButton(&dc, menutopleft, 35, 23, "RBL", 0);

		menutopleft.y = menutopleft.y + 25;
		TopMenu::DrawButton(&dc, menutopleft, 35, 23, "PIV", 0);

		menutopleft.y = menutopleft.y - 25;
		menutopleft.x = menutopleft.x + 37;
		TopMenu::DrawButton(&dc, menutopleft, 50, 23, "Rings 20", 0);

		menutopleft.y = menutopleft.y + 25;
		TopMenu::DrawButton(&dc, menutopleft, 50, 23, "Grid", 0);

		// get the controller position ID and display it (aesthetics :) )
		if (GetPlugIn()->ControllerMyself().IsValid())
		{
			controllerID = GetPlugIn()->ControllerMyself().GetPositionId();
		}

		menutopleft.y -= 25;
		menutopleft.x += 60;
		string cid = "CJS - " + controllerID;

		RECT r = TopMenu::DrawButton2(dc, menutopleft, 55, 23, cid.c_str(), 0);

		menutopleft.y += 25;
		but = TopMenu::DrawButton(&dc, menutopleft, 55, 23, "Qck Look", menuState.quickLook);
		menutopleft.y -= 25;
		ButtonToScreen(this, but, "Qck Look", BUTTON_MENU_QUICK_LOOK);

		POINT psrPoor[13] = {
			{0,0},
			{0,-5},
			{0,5},
			{0,0},
			{4,-4},
			{-4,4},
			{0,0},
			{4,4},
			{-4,-4},
			{0,0},
			{-5,0},
			{5,0},
			{0,0}
		};

		menuButton but_psrpoor = { {455, radarea.top + 6 }, "", 30,23, C_MENU_GREY3, C_MENU_GREY2, C_MENU_GREY4, 0 };
		TopMenu::DrawBut(&dc, but_psrpoor);
		TopMenu::DrawIconBut(&dc, but_psrpoor, psrPoor, 13);

		menuButton but_ALL = { { 455, radarea.top + 31 }, "ALL", 30, 23, C_MENU_GREY3, C_MENU_GREY2, C_MENU_TEXT_WHITE, menuState.filterBypassAll};
		but = TopMenu::DrawBut(&dc, but_ALL);
		ButtonToScreen(this, but, "Ovrd Filter ALL", BUTTON_MENU_OVRD_ALL);

		menuButton but_EXT = { { 485, radarea.top + 6 }, "Ext", 30, 23, C_MENU_GREY3, C_MENU_GREY2, C_MENU_TEXT_WHITE, menuState.extAltToggle };
		but = TopMenu::DrawBut(&dc, but_EXT);
		ButtonToScreen(this, but, "ExtAlt Toggle", BUTTON_MENU_EXT_ALT);

		menuButton but_EMode = { { 485, radarea.top + 31 }, "EMode", 62, 23, C_MENU_GREY3, C_MENU_GREY2, C_MENU_GREY4, 0 };
		TopMenu::DrawBut(&dc, but_EMode);

		POINT plane[19] = {
			{0,-5},
			{-1,-4},
			{-1,-2},
			{-5,2},
			{-5,3},
			{-1,1},
			{-1,4},
			{-4,6},
			{-4,7},
			{0,6},
			{4,7},
			{4,6},
			{1,4},
			{1,1},
			{5,3},
			{5,2},
			{1,-2},
			{1,-4},
			{0,-5}
		};

		menuButton but_FPE = { { 517, radarea.top + 6 }, "", 30, 23, C_MENU_GREY3, C_MENU_GREY2, C_MENU_TEXT_WHITE, menuState.showExtrapFP };
		but = TopMenu::DrawBut(&dc, but_FPE);
		TopMenu::DrawIconBut(&dc, but_FPE, plane, sizeof(plane)/sizeof(plane[0]));
		ButtonToScreen(this, but, "ExtrapolatedFP", BUTTON_MENU_EXTRAP_FP);

		menuButton but_highWx = { { 557, radarea.top + 6 }, "High", 30, 23, C_MENU_GREY3, C_MENU_GREY2, C_MENU_TEXT_WHITE, menuState.wxHigh };
		but = TopMenu::DrawBut(&dc, but_highWx);
		ButtonToScreen(this, but, "WxHigh", BUTTON_MENU_WX_HIGH);

		menuButton but_allWx = { { 587, radarea.top + 6 }, "All", 30, 23, C_MENU_GREY3, C_MENU_GREY2, C_MENU_TEXT_WHITE, menuState.wxAll };
		but = TopMenu::DrawBut(&dc, but_allWx);
		ButtonToScreen(this, but, "WxAll", BUTTON_MENU_WX_ALL);

		menuButton but_topsWx = { { 557, radarea.top + 31 }, wxRadar::ts.c_str(), 60, 23, C_MENU_GREY3, C_MENU_GREY2, C_MENU_GREY4, 0 };
		TopMenu::DrawBut(&dc, but_topsWx);

		menutopleft.x = menutopleft.x + 250;

		// options for halo radius
		if (halotool) {
			TopMenu::DrawHaloRadOptions(dc, menutopleft, halorad, halooptions);
			RECT rect;
			RECT r;

			r = TopMenu::DrawButton(&dc, menutopleft, 35, 46, "End", FALSE);
			ButtonToScreen(this, r, "End", BUTTON_MENU_HALO_OPTIONS);
			menutopleft.x += 35;

			r = TopMenu::DrawButton(&dc, menutopleft, 35, 46, "All On", FALSE);
			ButtonToScreen(this, r, "All On", BUTTON_MENU_HALO_OPTIONS);
			menutopleft.x += 35;
			r = TopMenu::DrawButton(&dc, menutopleft, 35, 46, "Clr All", FALSE);
			ButtonToScreen(this, r, "Clr All", BUTTON_MENU_HALO_OPTIONS);
			menutopleft.x += 35;

			for (int idx = 0; idx < 9; idx++) {

				rect.left = menutopleft.x;
				rect.top = menutopleft.y + 31;
				rect.right = menutopleft.x + 127;
				rect.bottom = menutopleft.y + 46;
				string key = to_string(idx);
				AddScreenObject(BUTTON_MENU_HALO_OPTIONS, key.c_str(), rect, 0, "");
				menutopleft.x += 22;
			}
			r = TopMenu::DrawButton(&dc, menutopleft, 35, 46, "Mouse", mousehalo);
			ButtonToScreen(this, r, "Mouse", BUTTON_MENU_HALO_OPTIONS);
		}

		// options for the altitude filter sub menu
		
		if (altFilterOpts) {
			
			r = TopMenu::DrawButton(&dc, menutopleft, 35, 46, "End", FALSE);
			ButtonToScreen(this, r, "End", BUTTON_MENU_ALT_FILT_OPT);
			menutopleft.x += 45;
			menutopleft.y += 5;
			
			r = TopMenu::MakeText(dc, menutopleft, 55, 15, "High Lim");
			menutopleft.x += 55;
			rHLim = TopMenu::MakeField(dc, menutopleft, 55, 15, altFilterHighFL.c_str());
			AddScreenObject(BUTTON_MENU_ALT_FILT_OPT, "HLim", rHLim, 0, "");

			menutopleft.x -= 55; menutopleft.y += 20;
			
			TopMenu::MakeText(dc, menutopleft, 55, 15, "Low Lim");
			menutopleft.x += 55;
			rLLim = TopMenu::MakeField(dc, menutopleft, 55, 15, altFilterLowFL.c_str());
			AddScreenObject(BUTTON_MENU_ALT_FILT_OPT, "LLim", rLLim, 0, "");

			menutopleft.x += 75;
			menutopleft.y -= 25;
			r = TopMenu::DrawButton(&dc, menutopleft, 35, 46, "Save", FALSE);
			AddScreenObject(BUTTON_MENU_ALT_FILT_OPT, "Save", r, 0, "");

		}
	}

	if (phase == REFRESH_PHASE_BACK_BITMAP) {
		if (menuState.wxAll || menuState.wxHigh) {
			wxRadar::renderRadar(&g, this, menuState.wxAll);
		}
	}

	g.ReleaseHDC(hdc);
	dc.Detach();
}

void CSiTRadar::OnClickScreenObject(int ObjectType,
	const char* sObjectId,
	POINT Pt,
	RECT Area,
	int Button)
{
	if (ObjectType == AIRCRAFT_SYMBOL) {
		
		if (halotool == TRUE) {

			CRadarTarget rt = GetPlugIn()->RadarTargetSelect(sObjectId);
			string callsign = rt.GetCallsign();

			if (hashalo.find(callsign) != hashalo.end()) {
				hashalo.erase(callsign);
			}
			else {
				hashalo[callsign] = TRUE;
			}
		}

		if (menuState.ptlTool == TRUE) {

			CRadarTarget rt = GetPlugIn()->RadarTargetSelect(sObjectId);
			string callsign = rt.GetCallsign();

			if (hasPTL.find(callsign) != hasPTL.end()) {
				hasPTL.erase(callsign);
			}
			else {
				hasPTL[callsign] = TRUE;
			}
		}

	}

	if (ObjectType == BUTTON_MENU_HALO_OPTIONS) {
		if (!strcmp(sObjectId, "0")) { halorad = 0.5; haloidx = 0; }
		if (!strcmp(sObjectId, "1")) { halorad = 3; haloidx = 1; }
		if (!strcmp(sObjectId, "2")) { halorad = 5; haloidx = 2; }
		if (!strcmp(sObjectId, "3")) { halorad = 10; haloidx = 3; }
		if (!strcmp(sObjectId, "4")) { halorad = 15; haloidx = 4; }
		if (!strcmp(sObjectId, "5")) { halorad = 20; haloidx = 5; }
		if (!strcmp(sObjectId, "6")) { halorad = 30; haloidx = 6; }
		if (!strcmp(sObjectId, "7")) { halorad = 60; haloidx = 7; }
		if (!strcmp(sObjectId, "8")) { halorad = 80; haloidx = 8; }
		if (!strcmp(sObjectId, "Clr All")) { hashalo.clear(); }
		if (!strcmp(sObjectId, "End")) { halotool = !halotool; }
		if (!strcmp(sObjectId, "Mouse")) { mousehalo = !mousehalo; }
		if (!strcmp(sObjectId, "Halo")) { halotool = !halotool; menuState.ptlTool = FALSE; }
	}

	if (ObjectType == BUTTON_MENU_PTL_TOOL) {
		if (halotool) { halotool = FALSE; }
		menuState.ptlTool = !menuState.ptlTool;
	}

	if (ObjectType == BUTTON_MENU_EXTRAP_FP) {
		menuState.showExtrapFP = !menuState.showExtrapFP;
	}

	if (ObjectType == BUTTON_MENU_OVRD_ALL) {
		menuState.filterBypassAll = !menuState.filterBypassAll;
	}

	if (ObjectType == BUTTON_MENU_EXT_ALT) {
		menuState.extAltToggle = !menuState.extAltToggle;
	}
	

	if (ObjectType == BUTTON_MENU_ALT_FILT_OPT) {
		if (!strcmp(sObjectId, "Alt Filt Opts")) { altFilterOpts = !altFilterOpts; }
		if (!strcmp(sObjectId, "End")) { altFilterOpts = 0; }
		if (!strcmp(sObjectId, "LLim")) {
			string altFilterLowFL = to_string(altFilterLow);
			if (altFilterLowFL.size() < 3) {
				altFilterLowFL.insert(altFilterLowFL.begin(), 3 - altFilterLowFL.size(), '0');
			}
			GetPlugIn()->OpenPopupEdit(rLLim, FUNCTION_ALT_FILT_LOW, altFilterLowFL.c_str());
		}
		if (!strcmp(sObjectId, "HLim")) {
			string altFilterHighFL = to_string(altFilterHigh);
			if (altFilterHighFL.size() < 3) {
				altFilterHighFL.insert(altFilterHighFL.begin(), 3 - altFilterHighFL.size(), '0');
			}
			GetPlugIn()->OpenPopupEdit(rHLim, FUNCTION_ALT_FILT_HIGH, altFilterHighFL.c_str());
		}
		if (!strcmp(sObjectId, "Save")) {
			string s = to_string(altFilterHigh);
			SaveDataToAsr("altFilterHigh", "Alt Filter High Limit", s.c_str());
			s = to_string(altFilterLow);
			SaveDataToAsr("altFilterLow", "Alt Filter Low Limit", s.c_str());
			altFilterOpts = 0;
		}
	}

	if (ObjectType == BUTTON_MENU_ALT_FILT_ON) {
		altFilterOn = !altFilterOn;
	}

	if (ObjectType == BUTTON_MENU_QUICK_LOOK) {
		if (menuState.quickLook == FALSE) {
			menuState.quickLook = TRUE;

			for (auto& p : CSiTRadar::mAcData) {
				CSiTRadar::tempTagData[p.first] = p.second.tagType;
				// Do not open uncorrelated tags
				if (p.second.tagType == 0) {
					p.second.tagType = 1;
				}
			}

		}
		else if (menuState.quickLook == TRUE) {

			for (auto& p : CSiTRadar::tempTagData) {
				// prevents closing of tags that became under your jurisdiction during quicklook
				if (!GetPlugIn()->FlightPlanSelect(p.first.c_str()).GetTrackingControllerIsMe()) { 
					CSiTRadar::mAcData[p.first].tagType = p.second;
				}
			}

			tempTagData.clear();
			menuState.quickLook = FALSE;
		}
	}

	if (ObjectType == BUTTON_MENU_WX_HIGH) {
		if (menuState.wxAll) { menuState.wxAll = false; }
		RefreshMapContent();
		menuState.wxHigh = !menuState.wxHigh;
	}

	if (ObjectType == BUTTON_MENU_WX_ALL) {
		if (menuState.wxHigh) { menuState.wxHigh = false; }
		RefreshMapContent();
		menuState.wxAll = !menuState.wxAll;
		wxRadar::GetRainViewerJSON(this);
	}
	
	if (Button == BUTTON_MIDDLE) {
		// open Free Text menu

		RECT freeTextPopUp;
		freeTextPopUp.left = Pt.x;
		freeTextPopUp.top = Pt.y;
		freeTextPopUp.right = Pt.x + 20;
		freeTextPopUp.bottom = Pt.y + 10;

		GetPlugIn()->OpenPopupList(freeTextPopUp, "Free Text", 1);

		GetPlugIn()->AddPopupListElement("ADD FREE TEXT", "", ADD_FREE_TEXT);
		GetPlugIn()->AddPopupListElement("DELETE", "", DELETE_FREE_TEXT, FALSE, POPUP_ELEMENT_NO_CHECKBOX, true, false);
		GetPlugIn()->AddPopupListElement("DELETE ALL", "", DELETE_ALL_FREE_TEXT);

	}

	// Handle tag functions when clicking on tags generated by the plugin
	
	if (ObjectType == TAG_ITEM_TYPE_CALLSIGN || ObjectType == TAG_ITEM_FP_CS) {
		GetPlugIn()->SetASELAircraft(GetPlugIn()->FlightPlanSelect(sObjectId)); // make sure aircraft is ASEL
		
		if (Button == BUTTON_LEFT) {
			if (mAcData[sObjectId].isHandoffToMe == TRUE) {
				GetPlugIn()->FlightPlanSelect(sObjectId).AcceptHandoff();
			}
		}

		if (Button == BUTTON_RIGHT) {
			StartTagFunction(sObjectId, NULL, TAG_ITEM_TYPE_CALLSIGN, sObjectId, NULL, TAG_ITEM_FUNCTION_HANDOFF_POPUP_MENU, Pt, Area);
		}
	}

	if (ObjectType == TAG_ITEM_TYPE_SQUAWK) {
		GetPlugIn()->SetASELAircraft(GetPlugIn()->RadarTargetSelect(sObjectId));
		if (Button == BUTTON_RIGHT) {
			
			StartTagFunction(GetPlugIn()->RadarTargetSelect(sObjectId).GetSystemID(), NULL, TAG_ITEM_TYPE_SQUAWK, GetPlugIn()->RadarTargetSelect(sObjectId).GetSystemID(), NULL, TAG_ITEM_FUNCTION_CORRELATE_POPUP, Pt, Area);
		}
	}

	if (ObjectType == TAG_ITEM_TYPE_ALTITUDE) {
		if (Button == BUTTON_RIGHT) {		
			GetPlugIn()->SetASELAircraft(GetPlugIn()->FlightPlanSelect(sObjectId));
			StartTagFunction(sObjectId, NULL, TAG_ITEM_TYPE_ALTITUDE, sObjectId, NULL, TAG_ITEM_FUNCTION_TEMP_ALTITUDE_POPUP, Pt, Area);
		}

		if (Button == BUTTON_LEFT) {
			if (menuState.extAltToggle) {
				mAcData[sObjectId].extAlt = !mAcData[sObjectId].extAlt;
			}
		}
	}

	if (ObjectType == TAG_ITEM_TYPE_GROUND_SPEED_WITH_N) {
		if (Button == BUTTON_LEFT) {
			GetPlugIn()->SetASELAircraft(GetPlugIn()->FlightPlanSelect(sObjectId));
			StartTagFunction(sObjectId, NULL, TAG_ITEM_TYPE_GROUND_SPEED_WITH_N, sObjectId, NULL, TAG_ITEM_FUNCTION_ASSIGNED_SPEED_POPUP, Pt, Area);
		}

		if (Button == BUTTON_RIGHT) {
			GetPlugIn()->SetASELAircraft(GetPlugIn()->FlightPlanSelect(sObjectId));
			StartTagFunction(sObjectId, NULL, TAG_ITEM_TYPE_GROUND_SPEED_WITH_N, sObjectId, NULL, TAG_ITEM_FUNCTION_ASSIGNED_MACH_POPUP, Pt, Area);
		}
	}

	if (ObjectType == TAG_ITEM_TYPE_PLANE_TYPE) {
		GetPlugIn()->SetASELAircraft(GetPlugIn()->FlightPlanSelect(sObjectId));
		if (Button == BUTTON_LEFT) {
			StartTagFunction(sObjectId, NULL, TAG_ITEM_TYPE_PLANE_TYPE, sObjectId, NULL, TAG_ITEM_FUNCTION_TOGGLE_ROUTE_DRAW, Pt, Area);
		}
		if (Button == BUTTON_RIGHT) {
			StartTagFunction(sObjectId, NULL, TAG_ITEM_TYPE_PLANE_TYPE, sObjectId, NULL, TAG_ITEM_FUNCTION_OPEN_FP_DIALOG, Pt, Area);
		}
	}

	if (ObjectType == CTR_DATA_TYPE_SCRATCH_PAD_STRING || ObjectType == TAG_ITEM_TYPE_COMMUNICATION_TYPE) {
		GetPlugIn()->SetASELAircraft(GetPlugIn()->FlightPlanSelect(sObjectId));
		if (Button == BUTTON_LEFT) {
			StartTagFunction(sObjectId, NULL, CTR_DATA_TYPE_SCRATCH_PAD_STRING, sObjectId, NULL, TAG_ITEM_FUNCTION_EDIT_SCRATCH_PAD, Pt, Area);
		}
		if (Button == BUTTON_RIGHT) {
			StartTagFunction(sObjectId, NULL, TAG_ITEM_TYPE_COMMUNICATION_TYPE, sObjectId, NULL, TAG_ITEM_FUNCTION_COMMUNICATION_POPUP, Pt, Area);
		}
	}

	if (ObjectType == TAG_ITEM_TYPE_DESTINATION) {
		GetPlugIn()->SetASELAircraft(GetPlugIn()->FlightPlanSelect(sObjectId));
		if (Button == BUTTON_RIGHT) {
			StartTagFunction(sObjectId, NULL, TAG_ITEM_TYPE_DESTINATION, sObjectId, NULL, TAG_ITEM_FUNCTION_ASSIGNED_HEADING_POPUP, Pt, Area);
		}
	}
}

void CSiTRadar::OnMoveScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, bool Released) {
	
	// Handling moving of the tags rendered by the plugin
	CRadarTarget rt = GetPlugIn()->RadarTargetSelect(sObjectId);
	CFlightPlan fp = GetPlugIn()->FlightPlanSelect(sObjectId);

	POINT p{ 0,0 };

	if (ObjectType == TAG_ITEM_FP_CS ) {
		
		if (fp.IsValid()) {
			p = ConvertCoordFromPositionToPixel(fp.GetFPTrackPosition().GetPosition());
		}

		RECT temp = Area;

		POINT q;
		q.x = ((temp.right + temp.left) / 2) - p.x - (TAG_WIDTH/2); // Get centre of box 
		q.y = ((temp.top + temp.bottom) / 2) - p.y - (TAG_HEIGHT/2);	 //(small nudge of a few pixels for error correcting with IRL behaviour) 

		// check maximal offset
		if (q.x > TAG_MAX_X_OFFSET) { q.x = TAG_MAX_X_OFFSET; }
		if (q.x < -TAG_MAX_X_OFFSET - CSiTRadar::mAcData[rt.GetCallsign()].tagWidth) { q.x = -TAG_MAX_X_OFFSET - TAG_WIDTH; }
		if (q.y > TAG_MAX_Y_OFFSET) { q.y = TAG_MAX_Y_OFFSET; }
		if (q.y < -TAG_MAX_Y_OFFSET - TAG_HEIGHT) { q.y = -TAG_MAX_Y_OFFSET - TAG_HEIGHT; }

		// nudge tag if necessary (near horizontal, or if directly above target)
		if (q.x > -((TAG_WIDTH) / 2) && q.x < 3) { q.x = 3; };
		if (q.x > -TAG_WIDTH && q.x <= -(TAG_WIDTH / 2)) { q.x = -TAG_WIDTH; }
		if (q.y > -14 && q.y < 0) { q.y = -7; }; //sticky horizon

		fptagOffset[sObjectId] = q;
		
		if (!Released) {

		}
		else {
			
		}
	}

	if (ObjectType == TAG_ITEM_TYPE_CALLSIGN) {

		if (fp.IsValid()) {
			p = ConvertCoordFromPositionToPixel(rt.GetPosition().GetPosition());
		}

		RECT temp = Area;

		POINT q;
		q.x = ((temp.right + temp.left) / 2) - p.x - (CSiTRadar::mAcData[rt.GetCallsign()].tagWidth / 2); // Get centre of box 
		q.y = ((temp.top + temp.bottom) / 2) - p.y - (TAG_HEIGHT / 2);	 //(small nudge of a few pixels for error correcting with IRL behaviour) 

		// check maximal offset
		if (q.x > TAG_MAX_X_OFFSET) { q.x = TAG_MAX_X_OFFSET; }
		if (q.x < -TAG_MAX_X_OFFSET - ( CSiTRadar::mAcData[rt.GetCallsign()].tagWidth)) { q.x = -TAG_MAX_X_OFFSET - CSiTRadar::mAcData[rt.GetCallsign()].tagWidth; }
		if (q.y > TAG_MAX_Y_OFFSET) { q.y = TAG_MAX_Y_OFFSET; }
		if (q.y < -TAG_MAX_Y_OFFSET - TAG_HEIGHT) { q.y = -TAG_MAX_Y_OFFSET - TAG_HEIGHT; }

		// nudge tag if necessary (near horizontal, or if directly above target)
		if (q.x > -((CSiTRadar::mAcData[rt.GetCallsign()].tagWidth) / 2) && q.x < 3) { q.x = 3; };
		if (q.x > -CSiTRadar::mAcData[rt.GetCallsign()].tagWidth && q.x <= -(CSiTRadar::mAcData[rt.GetCallsign()].tagWidth / 2)) { q.x = -CSiTRadar::mAcData[rt.GetCallsign()].tagWidth; }
		if (q.y > -14 && q.y < 0) { q.y = -7; }; //sticky horizon

		rtagOffset[sObjectId] = q;

		if (!Released) {

		}
		else {

			if (menuState.quickLook) { tempTagData[sObjectId] = 1; } // if you move a tag during quick look, it will stay open
			// once released, check that the tag does not exceed the limits, and then save it to the map
		}
	}

	RequestRefresh();
}

void CSiTRadar::OnFunctionCall(int FunctionId,
	const char* sItemString,
	POINT Pt,
	RECT Area) {
	if (FunctionId == FUNCTION_ALT_FILT_LOW) {
		try {
			altFilterLow = stoi(sItemString);
		}
		catch (...) {}
	}
	if (FunctionId == FUNCTION_ALT_FILT_HIGH) {
		try {
			altFilterHigh = stoi(sItemString);
		}
		catch (...) {}
	}
}

void CSiTRadar::ButtonToScreen(CSiTRadar* radscr, RECT rect, string btext, int itemtype) {
	AddScreenObject(itemtype, btext.c_str(), rect, 0, "");
}

void CSiTRadar::OnAsrContentLoaded(bool Loaded) {
	const char* filt = nullptr;

	// if (GetDataFromAsr("tagfamily")) { radtype = GetDataFromAsr("tagfamily"); }

	// getting altitude filter information
	if ((filt = GetDataFromAsr("altFilterHigh")) != NULL) {
		altFilterHigh = atoi(filt);
	}
	if ((filt = GetDataFromAsr("altFilterLow")) != NULL) {
		altFilterLow = atoi(filt);
	}

	// Find the position of ADSB radars
	GetPlugIn()->SelectActiveSectorfile();
	for (CSectorElement sectorElement = GetPlugIn()->SectorFileElementSelectFirst(SECTOR_ELEMENT_RADARS); sectorElement.IsValid();
		sectorElement = GetPlugIn()->SectorFileElementSelectNext(sectorElement, SECTOR_ELEMENT_RADARS))	{
		if (!strcmp(sectorElement.GetName(), "QER")) {
			sectorElement.GetPosition(&adsbSite, 0);
		}
	}
} 

void CSiTRadar::OnFlightPlanFlightPlanDataUpdate(CFlightPlan FlightPlan)
{
	// These items don't need to be updated each loop, save loop type by storing data in a map
	string callSign = FlightPlan.GetCallsign();
	bool isVFR = !strcmp(FlightPlan.GetFlightPlanData().GetPlanType(), "V");

	// Get information about the Aircraft/Flightplan
	string icaoACData = FlightPlan.GetFlightPlanData().GetAircraftInfo(); // logic to 
	regex icaoRVSM("(.*)\\/(.*)\\-(.*)[W](.*)\\/(.*)", regex::icase);
	bool isRVSM = regex_search(icaoACData, icaoRVSM); // first check for ICAO; then check FAA
	if (FlightPlan.GetFlightPlanData().GetCapibilities() == 'L' ||
		FlightPlan.GetFlightPlanData().GetCapibilities() == 'W' ||
		FlightPlan.GetFlightPlanData().GetCapibilities() == 'Z') {
		isRVSM = TRUE;
	}
	regex icaoADSB("(.*)\\/(.*)\\-(.*)\\/(.*)(E|L|B1|B2|U1|U2|V1|V2)(.*)");
	bool isADSB = regex_search(icaoACData, icaoADSB);

	string CJS = FlightPlan.GetTrackingControllerId();

	ACData acdata;
	acdata.hasVFRFP = isVFR;
	acdata.isADSB = isADSB;
	acdata.isRVSM = isRVSM;
	mAcData[callSign] = acdata;
}

void CSiTRadar::OnFlightPlanDisconnect(CFlightPlan FlightPlan) {
	string callSign = FlightPlan.GetCallsign();

	mAcData.erase(callSign);

}

void CSiTRadar::OnDoubleClickScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, int Button)
{
	if (Button == BUTTON_LEFT) {
		if (ObjectType == TAG_ITEM_TYPE_ALTITUDE) {
			if (CSiTRadar::mAcData[sObjectId].tagType == 0) { CSiTRadar::mAcData[sObjectId].tagType = 1; }
		}
		if (ObjectType == TAG_ITEM_TYPE_CALLSIGN) {
			if (CSiTRadar::mAcData[sObjectId].tagType == 1 &&
				!GetPlugIn()->FlightPlanSelect(sObjectId).GetTrackingControllerIsMe() &&
				!menuState.quickLook) { CSiTRadar::mAcData[sObjectId].tagType = 0; } // can't make bravo tag if you are tracking or if quick look is on
		}
	}
}

void CSiTRadar::OnAsrContentToBeSaved() {

}