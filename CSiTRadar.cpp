/*	Drawing on the main radar screen is done with this file
	This draws:
	1. CSiT Tools Menu
	2. VFR radar target
	3. Mouse halo if enabled
	4. Aircraft specific halos if enabled
	5. PTLS for aircrafts if enabled
*/

#include "pch.h"
#include "CSiTRadar.h"
#include "HaloTool.h"
#include "constants.h"
#include "TopMenu.h"
#include "SituPlugin.h"
#include "GndRadar.h"
#include <chrono>

CSiTRadar::CSiTRadar()
{
	halfSec = clock();
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

	int pixnm = PixelsPerNM();

	if (phase == REFRESH_PHASE_AFTER_TAGS) {

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

			// get the target's position on the screen and add it as a screen object
			POINT p = ConvertCoordFromPositionToPixel(radarTarget.GetPosition().GetPosition());
			RECT prect;
			prect.left = p.x - 5;
			prect.top = p.y - 5;
			prect.right = p.x + 5;
			prect.bottom = p.y + 5;
			AddScreenObject(AIRCRAFT_SYMBOL, radarTarget.GetCallsign(), prect, FALSE, "");

			// plane halo looks at the <map> hashalo to see if callsign has a halo, if so, draws halo
			if (hashalo.find(radarTarget.GetCallsign()) != hashalo.end()) {
				HaloTool::drawHalo(dc, p, halorad, pixnm);
			}

			// if squawking ident, PPS blinks white
			if (radarTarget.GetPosition().GetTransponderI()
				&& radarTarget.GetPosition().GetRadarFlags() != 0) {
				COLORREF targetPenColor;
				targetPenColor = RGB(255, 255, 255); // white when squawking ident
				HPEN targetPen;
				targetPen = CreatePen(PS_SOLID, 1, targetPenColor);
				dc.SelectObject(targetPen);
				dc.SelectStockObject(NULL_BRUSH);

				// draw the shape; blinks (only drawn on half second ticks)
				if (halfSecTick) {
					dc.SelectObject(targetPen);
					dc.MoveTo(p.x - 3, p.y - 2);
					dc.LineTo(p.x - 3, p.y + 1);
					dc.LineTo(p.x, p.y + 4);
					dc.LineTo(p.x + 3, p.y + 2);
					dc.LineTo(p.x + 3, p.y - 2);
					dc.LineTo(p.x , p.y - 4);
					dc.LineTo(p.x - 3, p.y - 2);
					dc.MoveTo(p.x - 3, p.y + 2);
					dc.LineTo(p.x, p.y - 4);
					dc.LineTo(p.x + 3, p.y + 2);
					dc.LineTo(p.x - 3, p.y + 2);
				}

				// cleanup
				DeleteObject(targetPen);
			}

			// if primary target draw the symbol in magenta

			if (radarTarget.GetPosition().GetRadarFlags() == 1) {
				COLORREF targetPenColor;
				targetPenColor = RGB(197, 38, 212); // magenta colour
				HPEN targetPen;
				targetPen = CreatePen(PS_SOLID, 1, targetPenColor);
				dc.SelectObject(targetPen);
				dc.SelectStockObject(NULL_BRUSH);

				// draw the shape
				dc.MoveTo(p.x, p.y + 4);
				dc.LineTo(p.x, p.y);
				dc.LineTo(p.x - 4, p.y - 4);
				dc.MoveTo(p.x, p.y);
				dc.LineTo(p.x + 4, p.y - 4);

				// cleanup
				DeleteObject(targetPen);

			}


			// if VFR
			if (strcmp(radarTarget.GetCorrelatedFlightPlan().GetFlightPlanData().GetPlanType(), "V") == 0
				&& radarTarget.GetPosition().GetTransponderC() == TRUE
				&& radarTarget.GetPosition().GetRadarFlags() != 0) {

				COLORREF targetPenColor;
				targetPenColor = RGB(242, 120, 57); // PPS orange color
				HPEN targetPen;
				HBRUSH targetBrush;
				targetPen = CreatePen(PS_SOLID, 1, targetPenColor);
				targetBrush = CreateSolidBrush(RGB(0, 0, 0));
				dc.SelectObject(targetPen);
				dc.SelectObject(targetBrush);

				// draw the shape
				dc.Ellipse(p.x - 4, p.y - 4, p.x + 6, p.y + 6);

				dc.SelectObject(targetPen);
				dc.MoveTo(p.x - 3, p.y - 2);
				dc.LineTo(p.x + 1, p.y + 4);
				dc.LineTo(p.x + 4, p.y - 2);

				DeleteObject(targetPen);
				DeleteObject(targetBrush);
			}

			// if ptl tag applied, draw it => not implemented

			// Handoff warning system: if the plane is within 2 minutes of exiting your airspace, CJS will blink

			if (radarTarget.GetCorrelatedFlightPlan().GetTrackingControllerIsMe()) {
				if (radarTarget.GetCorrelatedFlightPlan().GetSectorExitMinutes() <= 2 
					&& radarTarget.GetCorrelatedFlightPlan().GetSectorExitMinutes() >= 0) {
					// blink the CJS
					string callsign = radarTarget.GetCallsign();
					isBlinking[callsign] = TRUE;

					if (isBlinking.find(radarTarget.GetCallsign()) != isBlinking.end()
						&& halfSecTick) {
						continue; // skips CJS symbol drawing when blinked out
					}
				}
			}
			else {
				string callsign = radarTarget.GetCallsign();

				isBlinking.erase(callsign);
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

				dc.SelectObject(font);
				dc.SetTextColor(RGB(255, 255, 255));

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
/*
		// Ground Radar Tags WIP

		for (CRadarTarget rt = GetPlugIn()->RadarTargetSelectFirst(); rt.IsValid(); rt = GetPlugIn()->RadarTargetSelectNext(rt))
		{
			if (!rt.IsValid())
				continue;

			if (strcmp(rt.GetCorrelatedFlightPlan().GetFlightPlanData().GetDestination(), "CYYZ")) {

				POINT p = ConvertCoordFromPositionToPixel(rt.GetPosition().GetPosition());
				GndRadar::DrawGndTag(dc, p, 0, rt, rt.GetCallsign());
			}
		}

*/
	}

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
}

void CSiTRadar::OnAsrContentToBeSaved() {

}