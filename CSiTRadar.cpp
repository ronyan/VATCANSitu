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

			// if VFR
			if (strcmp(radarTarget.GetCorrelatedFlightPlan().GetFlightPlanData().GetPlanType(),"V") == 0
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

			// if ptl tag applied, draw it
		}

		// Draw the CSiT Tools Menu
		// this point moves to the origin of each subsequent area
		POINT menutopleft = CPoint(radarea.left, radarea.top); 

		TopMenu::DrawBackground(dc, menutopleft, radarea.right, 60);

		// screen range
				
		// altitude filters
		menutopleft.y += 6;
		menutopleft.x += 10;
		TopMenu::DrawButton(dc, menutopleft, 50, 23, "Alt Filter", 0);

		menutopleft.y += 25;
		TopMenu::DrawButton(dc, menutopleft, 50, 23, "000-600", 1);
		menutopleft.y -= 25;
		menutopleft.x += 65; 

		// separation tools
		string haloText = "Halo " + halooptions[haloidx];
		RECT but = TopMenu::DrawButton(dc, menutopleft, 45, 23, haloText.c_str(), halotool);
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

		menutopleft.y -= 25;
		menutopleft.x += 60;
		string cid = "CJS -" + controllerID;

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

/*
		// Ground Radar Tags

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

}

void CSiTRadar::ButtonToScreen(CSiTRadar* radscr, RECT rect, string btext, int itemtype) {
	AddScreenObject(itemtype, btext.c_str(), rect, 0, "");
};

void CSiTRadar::OnAsrContentLoaded() {
	if (GetDataFromAsr("tagfamily")) { radtype = GetDataFromAsr("tagfamily"); }

	// getting altitude filter information
	if (GetDataFromAsr("below")) { radtype = GetDataFromAsr("below"); }
	if (GetDataFromAsr("above")) { radtype = GetDataFromAsr("above"); }

	// get the controller position ID
	if (GetPlugIn()->ControllerMyself().IsValid())
	{
		controllerID = GetPlugIn()->ControllerMyself().GetPositionId();
	}
}