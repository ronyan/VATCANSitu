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
#include "GndRadar.h"
#include "ACTag.h"
#include "PPS.h"
#include <chrono>


using namespace Gdiplus;


CSiTRadar::CSiTRadar()
{
	halfSec = clock();
	halfSecTick = FALSE;
}

CSiTRadar::~CSiTRadar()
{
}

void CSiTRadar::OnRefresh(HDC hdc, int phase)
{
	if (phase != REFRESH_PHASE_AFTER_TAGS && phase != REFRESH_PHASE_BEFORE_TAGS) {
		return;
	}
	
	loopTime = clock() - nextLoop;
	nextLoop = clock();
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

	int pixnm = PixelsPerNM();

	if (phase == REFRESH_PHASE_AFTER_TAGS) {

		//debug
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
		dc.DrawText(to_string((float)loopTime / CLOCKS_PER_SEC).c_str(), &debug, DT_LEFT);
		debug.top += 10;

		DeleteObject(font);
		// debug

		// Draw the mouse halo before menu, so it goes behind it
		if (mousehalo == TRUE) {
			HaloTool::drawHalo(dc, p, halorad, pixnm);
			RequestRefresh();
		}

		// add orange PPS to aircrafts with VFR Flight Plans that have correlated targets
		// iterate over radar targets

		for (CRadarTarget radarTarget = GetPlugIn()->RadarTargetSelectFirst(); radarTarget.IsValid();
			radarTarget = GetPlugIn()->RadarTargetSelectNext(radarTarget))
		{
			// altitude filtering 
			if (altFilterOn && radarTarget.GetPosition().GetPressureAltitude() < altFilterLow * 100) {
				continue;
			}

			if (altFilterOn && altFilterHigh > 0 && radarTarget.GetPosition().GetPressureAltitude() > altFilterHigh * 100) {
				continue;
			}

			POINT p = ConvertCoordFromPositionToPixel(radarTarget.GetPosition().GetPosition());
			string callSign = radarTarget.GetCallsign();

			RECT prect;
			prect.left = p.x - 5;
			prect.top = p.y - 5;
			prect.right = p.x + 5;
			prect.bottom = p.y + 5;
			AddScreenObject(AIRCRAFT_SYMBOL, radarTarget.GetCallsign(), prect, FALSE, "");

			// Get information about the Aircraft/Flightplan
			bool isCorrelated = radarTarget.GetCorrelatedFlightPlan().IsValid();
			bool isVFR = hasVFRFP[callSign];
			bool isRVSM = hasRVSM[callSign];
			bool isADSB = hasADSB[callSign];

			COLORREF ppsColor;

			// logic for the color of the PPS
			if (radarTarget.GetPosition().GetRadarFlags() == 0) { ppsColor = C_WHITE; }
			else if (radarTarget.GetPosition().GetRadarFlags() == 1 && !isCorrelated) { ppsColor = C_PPS_MAGENTA; }
			else if (!strcmp(radarTarget.GetPosition().GetSquawk(), "7600") || !strcmp(radarTarget.GetPosition().GetSquawk(), "7700")) { ppsColor = C_PPS_RED; }
			else if (isVFR) { ppsColor = C_PPS_ORANGE; }
			else { ppsColor = C_PPS_YELLOW; }

			if (radarTarget.GetPosition().GetTransponderI() == TRUE && halfSecTick) { ppsColor = C_WHITE; }

			CPPS::DrawPPS(&dc, isCorrelated, isVFR, isADSB, isRVSM, radarTarget.GetPosition().GetRadarFlags(), ppsColor, radarTarget.GetPosition().GetSquawk(), p);

			// Handoff warning system: if the plane is within 2 minutes of exiting your airspace, CJS will blink

			if (radarTarget.GetCorrelatedFlightPlan().GetTrackingControllerIsMe()) {
				if (radarTarget.GetCorrelatedFlightPlan().GetSectorExitMinutes() <= 2 
					&& radarTarget.GetCorrelatedFlightPlan().GetSectorExitMinutes() >= 0) {
					isBlinking[callSign] = TRUE;
				}
			}
			else {
				isBlinking.erase(callSign);
			}

			// if in the process of handing off, flash the PPS (to be added), CJS and display the frequency 
			if (strcmp(radarTarget.GetCorrelatedFlightPlan().GetHandoffTargetControllerId(), "") != 0
				&& radarTarget.GetCorrelatedFlightPlan().GetTrackingControllerIsMe()
				) {
				string handOffFreq = "-" + to_string(GetPlugIn()->ControllerSelectByPositionId(radarTarget.GetCorrelatedFlightPlan().GetHandoffTargetControllerId()).GetPrimaryFrequency()).substr(0,6);
				string handOffCJS = radarTarget.GetCorrelatedFlightPlan().GetHandoffTargetControllerId();

				string handOffText = handOffCJS + handOffFreq;

				CFont font;
				LOGFONT lgfont;

				memset(&lgfont, 0, sizeof(LOGFONT));
				lgfont.lfWeight = 500;
				strcpy_s(lgfont.lfFaceName, _T("EuroScope"));
				lgfont.lfHeight = 12;
				font.CreateFontIndirect(&lgfont);

				dc.SetTextColor(RGB(255, 255, 255));

				dc.SelectObject(font);
				if (isBlinking.find(radarTarget.GetCallsign()) != isBlinking.end()
					&& halfSecTick) {
					handOffText=""; // blank CJS symbol drawing when blinked out
				}

				RECT rectCJS;
				rectCJS.left = p.x - 6;
				rectCJS.right = p.x + 75;
				rectCJS.top = p.y - 18;
				rectCJS.bottom = p.y;

				dc.DrawText(handOffText.c_str(), &rectCJS, DT_LEFT);

				DeleteObject(font);
			}
			else {

				// show CJS for controller tracking aircraft
				string CJS = radarTarget.GetCorrelatedFlightPlan().GetTrackingControllerId();

				CFont font;
				LOGFONT lgfont;

				memset(&lgfont, 0, sizeof(LOGFONT));
				lgfont.lfWeight = 500;
				strcpy_s(lgfont.lfFaceName, _T("EuroScope"));
				lgfont.lfHeight = 12;
				font.CreateFontIndirect(&lgfont);

				dc.SelectObject(font);
				dc.SetTextColor(RGB(202, 205, 169));

				RECT rectCJS;
				rectCJS.left = p.x - 6 ;
				rectCJS.right = p.x + 75;
				rectCJS.top = p.y - 18;
				rectCJS.bottom = p.y;

				dc.DrawText(CJS.c_str(), &rectCJS, DT_LEFT);

				DeleteObject(font);
			}

			// plane halo looks at the <map> hashalo to see if callsign has a halo, if so, draws halo
			if (hashalo.find(radarTarget.GetCallsign()) != hashalo.end()) {
				HaloTool::drawHalo(dc, p, halorad, pixnm);
			}

			// ADSB targets; if no primary or secondary radar, but the plane has ADSB equipment suffix (assumed space based ADS-B with no gaps)

			if (radarTarget.GetPosition().GetRadarFlags() == 0 
				&& isADSB) { // need to add ADSB equipment logic -- currently based on filed FP; no tag will display though. WIP

				// needs to be refactored

				CACTag::DrawRTACTag(&dc, this, &radarTarget, &radarTarget.GetCorrelatedFlightPlan(), &tagOffset);
				CACTag::DrawRTConnector(&dc, this, &radarTarget, &radarTarget.GetCorrelatedFlightPlan(), C_PPS_YELLOW, &tagOffset);
				
				COLORREF targetPenColor;
				targetPenColor = RGB(202, 205, 169); // amber colour
				HPEN targetPen;
				targetPen = CreatePen(PS_SOLID, 1, targetPenColor);
				dc.SelectObject(targetPen);
				dc.SelectStockObject(NULL_BRUSH);

				// draw the shape
				dc.MoveTo(p.x - 5, p.y - 5);
				dc.LineTo(p.x + 5, p.y -5);
				dc.LineTo(p.x + 5, p.y + 5);
				dc.LineTo(p.x - 5, p.y + 5);
				dc.LineTo(p.x - 5, p.y - 5);

				// if primary and secondary target, draw the middle line
				if (isRVSM) {
					dc.MoveTo(p.x, p.y - 5);
					dc.LineTo(p.x, p.y + 5);
				}

				// cleanup
				DeleteObject(targetPen);
			}

		}

		// Flight plan loop. Goes through flight plans, and if not correlated will display
		for (CFlightPlan flightPlan = GetPlugIn()->FlightPlanSelectFirst(); flightPlan.IsValid();
			flightPlan = GetPlugIn()->FlightPlanSelectNext(flightPlan)) {
			
			// aircraft equipment parsing
			string icaoACData = flightPlan.GetFlightPlanData().GetAircraftInfo(); // logic to 
			regex icaoADSB("(.*)\\/(.*)\\-(.*)\\/(.*)(E|L|B1|B2|U1|U2|V1|V2)(.*)");
			bool isADSB = regex_search(icaoACData, icaoADSB);

			// if the flightplan does not have a correlated radar target
			if (!flightPlan.GetCorrelatedRadarTarget().IsValid()
				&& flightPlan.GetFPState() == FLIGHT_PLAN_STATE_SIMULATED
				&& !isADSB) {

				CACTag::DrawFPACTag(&dc, this, &flightPlan.GetCorrelatedRadarTarget(), &flightPlan, &tagOffset);
				CACTag::DrawFPConnector(&dc, this, &flightPlan.GetCorrelatedRadarTarget(), &flightPlan, C_PPS_ORANGE, &tagOffset);

				// convert the predicted position to a point on the screen
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

				// Fill the shape

				SolidBrush orangeBrush(Color(255, 242, 120, 57));
				COLORREF targetPenColor;
				targetPenColor = RGB(242, 120, 57); // PPS orange color

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
		TopMenu::DrawButton(dc, menutopleft, 70, 23, "Relocate", 0);
		menutopleft.y += 25;

		TopMenu::DrawButton(dc, menutopleft, 35, 23, "Zoom", 0); 
		menutopleft.x += 35;
		TopMenu::DrawButton(dc, menutopleft, 35, 23, "Pan", 0);
		menutopleft.y -= 25;
		menutopleft.x += 55;
		
		// horizontal range calculation
		int range = (int)round(RadRange());
		string rng = to_string(range);
		TopMenu::MakeText(dc, menutopleft, 50, 15, "Range");
		menutopleft.y += 15;

		// 109 pix per in on my monitor
		int nmIn = 109 / pixnm;
		string nmtext = "1\" = " + to_string(nmIn) + "nm";
		TopMenu::MakeText(dc, menutopleft, 50, 15, nmtext.c_str());
		menutopleft.y += 17;

		TopMenu::MakeDropDown(dc, menutopleft, 40, 15, rng.c_str());

		menutopleft.x += 80;
		menutopleft.y -= 32;

		// altitude filters

		but = TopMenu::DrawButton(dc, menutopleft, 50, 23, "Alt Filter", altFilterOpts);
		ButtonToScreen(this, but, "Alt Filt Opts", BUTTON_MENU_ALT_FILT_OPT);
		
		menutopleft.y += 25;

		string altFilterLowFL = to_string(altFilterLow);
		altFilterLowFL.insert(altFilterLowFL.begin(), 3 - altFilterLowFL.size(), '0');
		string altFilterHighFL = to_string(altFilterHigh);
		altFilterHighFL.insert(altFilterHighFL.begin(), 3 - altFilterHighFL.size(), '0');

		string filtText = altFilterLowFL + string(" - ") + altFilterHighFL;
		but = TopMenu::DrawButton(dc, menutopleft, 50, 23, filtText.c_str(), altFilterOn);
		ButtonToScreen(this, but, "", BUTTON_MENU_ALT_FILT_ON);
		menutopleft.y -= 25;
		menutopleft.x += 65; 

		// separation tools
		string haloText = "Halo " + halooptions[haloidx];
		but = TopMenu::DrawButton(dc, menutopleft, 45, 23, haloText.c_str(), halotool);
		ButtonToScreen(this, but, "Halo", BUTTON_MENU_HALO_OPTIONS);

		menutopleft.y = menutopleft.y + 25;
		but = TopMenu::DrawButton(dc, menutopleft, 45, 23, "PTL 3", 0);
		ButtonToScreen(this, but, "PTL", BUTTON_MENU_HALO_OPTIONS);

		menutopleft.y = menutopleft.y - 25;
		menutopleft.x = menutopleft.x + 47;
		TopMenu::DrawButton(dc, menutopleft, 35, 23, "RBL", 0);

		menutopleft.y = menutopleft.y + 25;
		TopMenu::DrawButton(dc, menutopleft, 35, 23, "PIV", 0);

		menutopleft.y = menutopleft.y - 25;
		menutopleft.x = menutopleft.x + 37;
		TopMenu::DrawButton(dc, menutopleft, 50, 23, "Rings 20", 0);

		menutopleft.y = menutopleft.y + 25;
		TopMenu::DrawButton(dc, menutopleft, 50, 23, "Grid", 0);

		// get the controller position ID and display it (aesthetics :) )
		if (GetPlugIn()->ControllerMyself().IsValid())
		{
			controllerID = GetPlugIn()->ControllerMyself().GetPositionId();
		}

		menutopleft.y -= 25;
		menutopleft.x += 60;
		string cid = "CJS - " + controllerID;

		RECT r = TopMenu::DrawButton2(dc, menutopleft, 50, 23, cid.c_str(), 0);

		menutopleft.y += 25;
		TopMenu::DrawButton(dc, menutopleft, 50, 23, "Qck Look", 0);
		menutopleft.y -= 25;

		menutopleft.x = menutopleft.x + 100;

		// options for halo radius
		if (halotool) {
			TopMenu::DrawHaloRadOptions(dc, menutopleft, halorad, halooptions);
			RECT rect;
			RECT r;

			r = TopMenu::DrawButton(dc, menutopleft, 35, 46, "End", FALSE);
			ButtonToScreen(this, r, "End", BUTTON_MENU_HALO_OPTIONS);
			menutopleft.x += 35;

			r = TopMenu::DrawButton(dc, menutopleft, 35, 46, "All On", FALSE);
			ButtonToScreen(this, r, "All On", BUTTON_MENU_HALO_OPTIONS);
			menutopleft.x += 35;
			r = TopMenu::DrawButton(dc, menutopleft, 35, 46, "Clr All", FALSE);
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
			r = TopMenu::DrawButton(dc, menutopleft, 35, 46, "Mouse", mousehalo);
			ButtonToScreen(this, r, "Mouse", BUTTON_MENU_HALO_OPTIONS);
		}

		// options for the altitude filter sub menu
		
		if (altFilterOpts) {
			
			r = TopMenu::DrawButton(dc, menutopleft, 35, 46, "End", FALSE);
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
			r = TopMenu::DrawButton(dc, menutopleft, 35, 46, "Save", FALSE);
			AddScreenObject(BUTTON_MENU_ALT_FILT_OPT, "Save", r, 0, "");

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
	if (ObjectType == AIRCRAFT_SYMBOL && halotool == TRUE) {
		
		CRadarTarget rt = GetPlugIn()->RadarTargetSelect(sObjectId);
		string callsign = rt.GetCallsign();

		if (hashalo.find(callsign) != hashalo.end()) {
			hashalo.erase(callsign);
		}
		else {
			hashalo[callsign] = TRUE;
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
		if (!strcmp(sObjectId, "Halo")) { halotool = !halotool; }
	}

	if (ObjectType == BUTTON_MENU_ALT_FILT_OPT) {
		if (!strcmp(sObjectId, "Alt Filt Opts")) { altFilterOpts = !altFilterOpts; }
		if (!strcmp(sObjectId, "End")) { altFilterOpts = 0; }
		if (!strcmp(sObjectId, "LLim")) {
			string altFilterLowFL = to_string(altFilterLow);
			altFilterLowFL.insert(altFilterLowFL.begin(), 3 - altFilterLowFL.size(), '0');
			GetPlugIn()->OpenPopupEdit(rLLim, FUNCTION_ALT_FILT_LOW, altFilterLowFL.c_str());
		}
		if (!strcmp(sObjectId, "HLim")) {
			string altFilterHighFL = to_string(altFilterHigh);
			altFilterHighFL.insert(altFilterHighFL.begin(), 3 - altFilterHighFL.size(), '0');
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
	
	if (ObjectType == TAG_ITEM_TYPE_CALLSIGN || ObjectType == TAG_ITEM_FP_CS || ObjectType == TAG_ITEM_FP_FINAL_ALTITUDE) {
		GetPlugIn()->SetASELAircraft(GetPlugIn()->FlightPlanSelect(sObjectId)); // make sure aircraft is ASEL
		
		if (Button == BUTTON_RIGHT) {

			StartTagFunction(sObjectId, NULL, TAG_ITEM_TYPE_CALLSIGN, sObjectId, NULL, TAG_ITEM_FUNCTION_HANDOFF_POPUP_MENU, Pt, Area);
		}
	}

}

void CSiTRadar::OnMoveScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, bool Released) {
	
	// Handling moving of the tags rendered by the plugin
	CRadarTarget rt = GetPlugIn()->RadarTargetSelect(sObjectId);
	CFlightPlan fp = GetPlugIn()->FlightPlanSelect(sObjectId);

	POINT p{ 0,0 };

	if (ObjectType == TAG_ITEM_TYPE_CALLSIGN || ObjectType == TAG_ITEM_FP_CS || ObjectType == TAG_ITEM_FP_FINAL_ALTITUDE) {
		
		if (fp.IsValid()) {
			p = ConvertCoordFromPositionToPixel(fp.GetFPTrackPosition().GetPosition());
		}

		RECT temp = Area;

		POINT q;
		q.x = ((temp.right + temp.left) / 2) - p.x - 17; // Get centre of box 
		q.y = ((temp.top + temp.bottom) / 2) - p.y - 7;	 //(small nudge of a few pixels for error correcting with IRL behaviour) 

		// check maximal offset
		if (q.x > TAG_MAX_X_OFFSET) { q.x = TAG_MAX_X_OFFSET; }
		if (q.x < -TAG_MAX_X_OFFSET - TAG_WIDTH) { q.x = -TAG_MAX_X_OFFSET - TAG_WIDTH; }
		if (q.y > TAG_MAX_Y_OFFSET) { q.y = TAG_MAX_Y_OFFSET; }
		if (q.y < -TAG_MAX_Y_OFFSET - TAG_HEIGHT) { q.y = -TAG_MAX_Y_OFFSET - TAG_HEIGHT; }

		// nudge tag if necessary (near horizontal, or if directly above target)
		if (q.x > -((TAG_WIDTH) / 2) && q.x < 3) { q.x = 3; };
		if (q.x > -TAG_WIDTH && q.x <= -(TAG_WIDTH / 2)) { q.x = -TAG_WIDTH; }
		if (q.y > -14 && q.y < 0) { q.y = -7; }; //sticky horizon

		tagOffset[sObjectId] = q;
		
		if (!Released) {

		}
		else {
			// once released, check that the tag does not exceed the limits, and then save it to the map
		}
	}

	if (ObjectType == TAG_ITEM_TYPE_CALLSIGN) {

		if (rt.IsValid()) {
			p = ConvertCoordFromPositionToPixel(rt.GetPosition().GetPosition());
		}

		RECT temp = Area;

		POINT q;
		q.x = ((temp.right + temp.left) / 2) - p.x - 17; // Get centre of box 
		q.y = ((temp.top + temp.bottom) / 2) - p.y - 7;	 //(small nudge of a few pixels for error correcting with IRL behaviour) 

		// check maximal offset
		if (q.x > TAG_MAX_X_OFFSET) { q.x = TAG_MAX_X_OFFSET; }
		if (q.x < -TAG_MAX_X_OFFSET - TAG_WIDTH) { q.x = -TAG_MAX_X_OFFSET - TAG_WIDTH; }
		if (q.y > TAG_MAX_Y_OFFSET) { q.y = TAG_MAX_Y_OFFSET; }
		if (q.y < -TAG_MAX_Y_OFFSET - TAG_HEIGHT) { q.y = -TAG_MAX_Y_OFFSET - TAG_HEIGHT; }

		// nudge tag if necessary (near horizontal, or if directly above target)
		if (q.x > -((TAG_WIDTH) / 2) && q.x < 0) { q.x = 3; };
		if (q.x > -TAG_WIDTH && q.x <= -(TAG_WIDTH / 2)) { q.x = -TAG_WIDTH; }

		tagOffset[sObjectId] = q;

		if (!Released) {

		}
		else {
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

void CSiTRadar::OnFlightPlanFlightPlanDataUpdate ( CFlightPlan FlightPlan )
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

	hasADSB[callSign] = isADSB;
	hasVFRFP[callSign] = isVFR;
	hasRVSM[callSign] = isRVSM;

}

void CSiTRadar::OnFlightPlanDisconnect(CFlightPlan FlightPlan) {
	string callSign = FlightPlan.GetCallsign();

	hasADSB.erase(callSign);
	hasVFRFP.erase(callSign);
	hasRVSM.erase(callSign);
}

void CSiTRadar::OnAsrContentToBeSaved() {

}