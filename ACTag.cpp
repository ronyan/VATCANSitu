#include "pch.h"
#include "ACTag.h"

using namespace EuroScopePlugIn;

void CACTag::DrawFPACTag(CDC* dc, CRadarScreen* rad, CRadarTarget* rt, CFlightPlan* fp, unordered_map<string, POINT>* tOffset) {

	POINT p{0,0};
	int tagOffsetX = 0;
	int tagOffsetY = 0;

	// Initiate the default tag location, if no location is set already or find it in the map

	if (tOffset->find(fp->GetCallsign()) == tOffset->end()) {
		POINT pTag{ 20, -20 };
		tOffset->insert(pair<string, POINT>(fp->GetCallsign(), pTag));

		tagOffsetX = pTag.x;
		tagOffsetY = pTag.y;
	}
	else {
		POINT pTag = tOffset->find(fp->GetCallsign())->second;

		tagOffsetX = pTag.x;
		tagOffsetY = pTag.y;
	}

	// save context
	int sDC = dc->SaveDC();

	CFont font;
	LOGFONT lgfont;
	memset(&lgfont, 0, sizeof(LOGFONT));
	lgfont.lfHeight = 14;
	lgfont.lfWeight = 500;
	strcpy_s(lgfont.lfFaceName, _T("EuroScope"));
	font.CreateFontIndirect(&lgfont);
	dc->SelectObject(font);

	// Find position of aircraft 
	if (rt->IsValid()) {
		p = rad->ConvertCoordFromPositionToPixel(rt->GetPosition().GetPosition());
	}
	else {
		if (fp->IsValid()) {
			p = rad->ConvertCoordFromPositionToPixel(fp->GetFPTrackPosition().GetPosition());
		}
	}

	// Tag formatting
	RECT tagCallsign;
	tagCallsign.left = p.x + tagOffsetX;
	tagCallsign.top = p.y + tagOffsetY;

	RECT tagAltitude;
	tagAltitude.left = p.x + tagOffsetX;
	tagAltitude.top = p.y + tagOffsetY +10;
	
	RECT tagGS;
	tagGS.left = p.x + tagOffsetX + 40;
	tagGS.top = p.y + tagOffsetY + 10;
	

	if (fp->IsValid()) {

		dc->SetTextColor(C_PPS_ORANGE); // FP Track in orange colour

		POINT p = rad->ConvertCoordFromPositionToPixel(fp->GetFPTrackPosition().GetPosition());
		
		// Parse the CS and Wt Symbol
		string cs = fp->GetCallsign();
		string wtSymbol = "";
		if (fp->GetFlightPlanData().GetAircraftWtc() == 'H') { wtSymbol = "+"; }
		if (fp->GetFlightPlanData().GetAircraftWtc() == 'L') { wtSymbol = "-"; }
		cs = cs + wtSymbol;

		fp->GetClearedAltitude();

		// Draw the text for the tag
			
		dc->DrawText(cs.c_str(), &tagCallsign, DT_LEFT | DT_CALCRECT);
		dc->DrawText(cs.c_str(), &tagCallsign, DT_LEFT);

dc->DrawText(to_string(fp->GetFinalAltitude() / 100).c_str(), &tagAltitude, DT_LEFT | DT_CALCRECT);
dc->DrawText(to_string(fp->GetFinalAltitude() / 100).c_str(), &tagAltitude, DT_LEFT);

// Add the screen obects, TAG_FP_AREA first so that the others go on top;

rad->AddScreenObject(TAG_ITEM_FP_CS, fp->GetCallsign(), tagCallsign, TRUE, fp->GetCallsign());
rad->AddScreenObject(TAG_ITEM_FP_FINAL_ALTITUDE, fp->GetCallsign(), tagAltitude, TRUE, "ALT");
	}

	// restore context
	dc->RestoreDC(sDC);

	// cleanup
	DeleteObject(font);
}

// Draws tag for Radar Targets

void CACTag::DrawRTACTag(CDC* dc, CRadarScreen* rad, CRadarTarget* rt, CFlightPlan* fp, unordered_map<string, POINT>* tOffset) {

	POINT p{ 0,0 };
	p = rad->ConvertCoordFromPositionToPixel(rt->GetPosition().GetPosition());
	int tagOffsetX = 0;
	int tagOffsetY = 0;

	bool blinking = FALSE;
	if (strcmp(fp->GetHandoffTargetControllerId(), rad->GetPlugIn()->ControllerMyself().GetPositionId()) == 0) { blinking = TRUE; }
	if (rt->GetPosition().GetTransponderI()) { blinking = TRUE; }
	if (CSiTRadar::hoAcceptedTime.find(rt->GetCallsign()) != CSiTRadar::hoAcceptedTime.end()) { blinking = TRUE; }

	// Line 0 Items
	string ssr = rt->GetPosition().GetSquawk();
	
	// Line 1 Items
	string cs = rt->GetCallsign();
	string wtSymbol = "";
	if (rad->GetPlugIn()->FlightPlanSelect(cs.c_str()).GetFlightPlanData().GetAircraftWtc() == 'H') { wtSymbol = "+"; }
	if (rad->GetPlugIn()->FlightPlanSelect(cs.c_str()).GetFlightPlanData().GetAircraftWtc() == 'L') { wtSymbol = "-"; }
	cs = cs + wtSymbol;
	string sfi = fp->GetControllerAssignedData().GetScratchPadString();

	// Line 2 Items
	string altThreeDigit;
	if (rt->GetPosition().GetPressureAltitude() > rad->GetPlugIn()->GetTransitionAltitude()) {
		altThreeDigit = to_string((rt->GetPosition().GetFlightLevel() + 50) / 100); // +50 to force rounding up
	}
	else {
		altThreeDigit = to_string((rt->GetPosition().GetPressureAltitude() + 50) / 100);
	}
	if(altThreeDigit.size() <= 3) { altThreeDigit.insert(altThreeDigit.begin(), 3 - altThreeDigit.size(), '0'); }
	string vmi;
	if (rt->GetVerticalSpeed() > 400) { vmi = "^"; }
	if (rt->GetVerticalSpeed() < -400) { vmi = "|"; }; // up arrow "??!" = downarrow
	string vmr = to_string(abs(rt->GetVerticalSpeed()/200));
	if (vmr.size() <= 2) { vmr.insert(vmr.begin(), 2 - vmr.size(), '0'); }
	string clrdAlt = to_string(rt->GetCorrelatedFlightPlan().GetControllerAssignedData().GetClearedAltitude()/100);
	if (clrdAlt.size() <= 3) { clrdAlt.insert(clrdAlt.begin(), 3 - clrdAlt.size(), '0'); }
	if (rt->GetCorrelatedFlightPlan().GetControllerAssignedData().GetClearedAltitude() == 0) { clrdAlt = "clr"; }
	if (rt->GetCorrelatedFlightPlan().GetControllerAssignedData().GetClearedAltitude() == 1) { clrdAlt = "APR"; }
	if (rt->GetCorrelatedFlightPlan().GetControllerAssignedData().GetClearedAltitude() == 2) { clrdAlt = "APR"; }	
	string fpAlt = to_string(rt->GetCorrelatedFlightPlan().GetFlightPlanData().GetFinalAltitude() / 100);
	if (fpAlt.size() <=3) { fpAlt.insert(fpAlt.begin(), 3 - fpAlt.size(), '0'); }
	if (rt->GetCorrelatedFlightPlan().GetFlightPlanData().GetFinalAltitude() == 0) { fpAlt = "fld"; }
	string handoffCJS = rt->GetCorrelatedFlightPlan().GetHandoffTargetControllerId();
	if (strcmp(rt->GetCorrelatedFlightPlan().GetHandoffTargetControllerId(), rad->GetPlugIn()->ControllerMyself().GetPositionId()) == 0) {
		handoffCJS = rt->GetCorrelatedFlightPlan().GetTrackingControllerId();
	}
	string groundSpeed = to_string((rt->GetPosition().GetReportedGS() + 5) / 10);

	// Line 3 Items
	string acType = rt->GetCorrelatedFlightPlan().GetFlightPlanData().GetAircraftFPType();
	string destination = rt->GetCorrelatedFlightPlan().GetFlightPlanData().GetDestination();

	// Initiate the default tag location, if no location is set already or find it in the map

	if (tOffset->find(rt->GetCallsign()) == tOffset->end()) {
		POINT pTag{ 20, -20 };
		tOffset->insert(pair<string, POINT>(rt->GetCallsign(), pTag));

		tagOffsetX = pTag.x;
		tagOffsetY = pTag.y;
	}
	else {
		POINT pTag = tOffset->find(rt->GetCallsign())->second;

		tagOffsetX = pTag.x;
		tagOffsetY = pTag.y;
	}

	POINT line0 = { p.x + tagOffsetX, p.y + tagOffsetY - 12 };
	POINT line1 = { p.x + tagOffsetX, p.y + tagOffsetY };
	POINT line2 = { p.x + tagOffsetX, p.y + tagOffsetY + 12 };
	POINT line3 = { p.x + tagOffsetX, p.y + tagOffsetY + 24 };
	POINT line4 = { p.x + tagOffsetX, p.y + tagOffsetY + 36 };

	// save context
	int sDC = dc->SaveDC();

	CFont font;
	LOGFONT lgfont;
	memset(&lgfont, 0, sizeof(LOGFONT));
	lgfont.lfHeight = 14;
	lgfont.lfWeight = 500;
	strcpy_s(lgfont.lfFaceName, _T("EuroScope"));
	font.CreateFontIndirect(&lgfont);
	dc->SelectObject(font);

	// Draw Connector

	int doglegX = 0;
	int doglegY = 0;

	if (CSiTRadar::mAcData[rt->GetCallsign()].tagType == 1 ||
		CSiTRadar::mAcData[rt->GetCallsign()].tagType == 2) {

		POINT connector{ 0,0 };
		int tagOffsetX = 0;
		int tagOffsetY = 0;

		// get the tag off set from the TagOffset<map>
		POINT pTag = tOffset->find(rt->GetCallsign())->second;

		tagOffsetX = pTag.x;
		tagOffsetY = pTag.y;

		bool blinking = FALSE;
		if (fp->GetHandoffTargetControllerId() == rad->GetPlugIn()->ControllerMyself().GetPositionId()) { blinking = TRUE; }
		if (rt->GetPosition().GetTransponderI()) { blinking = TRUE; }

		if (rt->IsValid()) {
			p = rad->ConvertCoordFromPositionToPixel(rt->GetPosition().GetPosition());
		}

		// determine if the tag is to the left or the right of the PPS

		if (pTag.x >= 0) { connector.x = (int)p.x + tagOffsetX - 3; };
		if (pTag.x < 0) { connector.x = (int)p.x + tagOffsetX - 3 + CSiTRadar::mAcData[rt->GetCallsign()].tagWidth; }
		connector.y = p.y + tagOffsetY + 7;

		// the connector is only drawn at 30, 45 or 60 degrees, set the theta to the nearest appropriate angle
		// get the angle between the line between the PPS and connector and horizontal

		// if vertical (don't divide by 0!)
		double theta = 30;
		double phi = 0;

		if (connector.x - p.x != 0) {

			double x = abs(connector.x - p.x); // use absolute value since coord system is upside down
			double y = abs(p.y - connector.y); // also cast as double for atan

			phi = atan(y / x);

			// logic for if phi is a certain value; unit circle! (with fudge factor)
			if (phi >= 0 && phi < PI / 6) { theta = 30; }
			else if (phi >= PI / 6 && phi < PI / 4) { theta = 45; }
			else if (phi >= PI / 4 && phi < PI / 3) { theta = 60; }

			theta = theta * PI / 180; // to radians
			doglegY = p.y + tagOffsetY + 7; // small padding to line it up with the middle of the first line

			// Calculate the x position of the intersection point (probably there is a more efficient way, but the atan drove me crazy
			doglegX = (int)(p.x + ((double)(p.y - (double)connector.y) / tan(theta))); // quad 1

			if (connector.x < p.x) { doglegX = (int)(p.x - ((double)(p.y - (double)connector.y) / tan(theta))); } // quadrant 2
			if (connector.y > p.y && connector.x > p.x) { doglegX = (int)(p.x - ((double)(p.y - (double)connector.y) / tan(theta))); }
			if (connector.y > p.y && connector.x < p.x) { doglegX = (int)(p.x + ((double)(p.y - (double)connector.y) / tan(theta))); }
			if (phi >= PI / 3) { doglegX = p.x; } // same as directly above or below
		}
		else {
			doglegX = p.x; // if direction on top or below
			doglegY = p.y + tagOffsetY + 7;
		}

		// Draw the angled line and draw the horizontal line
		HPEN targetPen;
		COLORREF conColor = C_PPS_YELLOW;
		if (CSiTRadar::halfSecTick == TRUE && blinking) { conColor = C_WHITE; }
		targetPen = CreatePen(PS_SOLID, 1, conColor);
		dc->SelectObject(targetPen);
		dc->SelectStockObject(NULL_BRUSH);

		dc->MoveTo(p.x, p.y);
		dc->LineTo((int)doglegX, (int)doglegY); // line to the dogleg
		dc->LineTo(connector.x, (int)p.y + tagOffsetY + 7); // line to the connector point

		// ADSB circle
		if (CSiTRadar::mAcData[rt->GetCallsign()].isADSB) {
			dc->Ellipse((int)doglegX - 3, (int)doglegY - 3, (int)doglegX + 4, (int)doglegY + 4);
		}

		DeleteObject(targetPen);

	}

	// Draw Connector Ends

	if (CSiTRadar::mAcData[rt->GetCallsign()].tagType == 1) {
		// Tag formatting
		RECT tagCallsign;
		tagCallsign.left = p.x + tagOffsetX;
		tagCallsign.top = p.y + tagOffsetY;

		dc->SetTextColor(C_PPS_YELLOW);
		if (blinking && CSiTRadar::halfSecTick) {
			dc->SetTextColor(C_WHITE);
		}

		// Line 1
		RECT rline1;
		rline1.top = line1.y;
		rline1.left = line1.x;
		rline1.bottom = line2.y;
		dc->DrawText(cs.c_str(), &rline1, DT_LEFT | DT_CALCRECT);
		dc->DrawText(cs.c_str(), &rline1, DT_LEFT);
		rad->AddScreenObject(TAG_ITEM_TYPE_CALLSIGN, rt->GetCallsign(), rline1, TRUE, rt->GetCallsign());
		rline1.left = rline1.right;
		rline1.right = rline1.left + 8;
		if (sfi.size() > 0  && sfi.size() <= 2) {
			
			dc->DrawText(sfi.c_str(), &rline1, DT_LEFT | DT_CALCRECT);
			dc->DrawText(sfi.c_str(), &rline1, DT_LEFT);
		}
		rad->AddScreenObject(CTR_DATA_TYPE_SCRATCH_PAD_STRING, rt->GetCallsign(), rline1, TRUE, rt->GetCallsign());
			
		// add some padding for the SFI + long callsigns
		if (sfi.size() == 0) { CSiTRadar::mAcData[rt->GetCallsign()].tagWidth = rline1.right - tagCallsign.left + 12; }
		else { CSiTRadar::mAcData[rt->GetCallsign()].tagWidth = rline1.right - tagCallsign.left + 6; }


		// draw extension if tag is to the left of the PPS
		if (rline1.right < (int)doglegX) {
			HPEN targetPen;
			COLORREF conColor = C_PPS_YELLOW;
			if (CSiTRadar::halfSecTick == TRUE && blinking) { conColor = C_WHITE; }
			targetPen = CreatePen(PS_SOLID, 1, conColor);
			dc->SelectObject(targetPen);

			dc->MoveTo(rline1.right + 5, rline1.top + 7);
			dc->LineTo((int)doglegX, (int)doglegY);

			DeleteObject(targetPen);
		}


		// Line 2
		RECT rline2;
		rline2.top = line2.y;
		rline2.left = line2.x;
		rline2.bottom = line3.y;
		dc->DrawText(altThreeDigit.c_str(), &rline2, DT_LEFT | DT_CALCRECT);
		dc->DrawText(altThreeDigit.c_str(), &rline2, DT_LEFT);
		rad->AddScreenObject(TAG_ITEM_TYPE_ALTITUDE, rt->GetCallsign(), rline2, TRUE, "");

		if (abs(rt->GetVerticalSpeed()) > 400) {
			rline2.left = rline2.right;
			dc->DrawText(vmi.c_str(), &rline2, DT_LEFT | DT_CALCRECT);
			dc->DrawText(vmi.c_str(), &rline2, DT_LEFT);

			rline2.left = rline2.right;
			dc->DrawText(vmr.c_str(), &rline2, DT_LEFT | DT_CALCRECT);
			dc->DrawText(vmr.c_str(), &rline2, DT_LEFT);
		}
		rline2.left = rline2.right + 8;

		double alt;

		if (rt->GetPosition().GetPressureAltitude() > rad->GetPlugIn()->GetTransitionAltitude()) {
			alt = rt->GetPosition().GetFlightLevel(); // +50 to force rounding up
		}
		else {
			alt = rt->GetPosition().GetPressureAltitude();
		}

		if (
			// altitude differential 
			(abs(alt - rt->GetCorrelatedFlightPlan().GetControllerAssignedData().GetClearedAltitude()) > 200 &&
			rt->GetCorrelatedFlightPlan().GetControllerAssignedData().GetClearedAltitude() != 0) 
			
			// or extended altitudes toggled on
			|| (CSiTRadar::menuState.extAltToggle && CSiTRadar::mAcData[rt->GetCallsign()].extAlt)) {


			dc->SetTextColor(C_PPS_ORANGE);
			if (blinking && CSiTRadar::halfSecTick) {
				dc->SetTextColor(C_WHITE);
			}
			dc->DrawText(("C" + clrdAlt).c_str(), &rline2, DT_LEFT | DT_CALCRECT);
			dc->DrawText(("C" + clrdAlt).c_str(), &rline2, DT_LEFT);
			dc->SetTextColor(C_PPS_YELLOW);
			rline2.left = rline2.right + 8;
		}

		if (CSiTRadar::menuState.extAltToggle && CSiTRadar::mAcData[rt->GetCallsign()].extAlt) {
			dc->SetTextColor(C_PPS_ORANGE);
			if (blinking && CSiTRadar::halfSecTick) {
				dc->SetTextColor(C_WHITE);
			}
			dc->DrawText(("F" + fpAlt).c_str(), &rline2, DT_LEFT | DT_CALCRECT);
			dc->DrawText(("F" + fpAlt).c_str(), &rline2, DT_LEFT);
			dc->SetTextColor(C_PPS_YELLOW);
			rline2.left = rline2.right + 8;
		}

		dc->SetTextColor(RGB(255, 234, 46));
		if (blinking && CSiTRadar::halfSecTick) {
			dc->SetTextColor(C_WHITE);
		}
		dc->DrawText(handoffCJS.c_str(), &rline2, DT_LEFT | DT_CALCRECT);
		dc->DrawText((handoffCJS).c_str(), &rline2, DT_LEFT);
		rline2.left = rline2.right + 8;
		if (rline2.left < p.x + tagOffsetX + 38) { rline2.left = p.x + tagOffsetX + 38; }
		dc->SetTextColor(C_PPS_YELLOW);

		if (blinking && CSiTRadar::halfSecTick) {
			dc->SetTextColor(C_WHITE);
		}
		dc->DrawText(to_string(rt->GetPosition().GetReportedGS() / 10).c_str(), &rline2, DT_LEFT | DT_CALCRECT);
		dc->DrawText(to_string(rt->GetPosition().GetReportedGS() / 10).c_str(), &rline2, DT_LEFT);

		// Line 3

		RECT rline3;
		rline3.top = line3.y;
		rline3.left = line3.x;
		rline3.bottom = line4.y;
		dc->DrawText(acType.c_str(), &rline3, DT_LEFT | DT_CALCRECT);
		dc->DrawText(acType.c_str(), &rline3, DT_LEFT);
		rad->AddScreenObject(TAG_ITEM_TYPE_PLANE_TYPE, rt->GetCallsign(), rline3, TRUE, "");
		rline3.left = rline3.right + 10;
		dc->DrawText(destination.c_str(), &rline3, DT_LEFT | DT_CALCRECT);
		dc->DrawText(destination.c_str(), &rline3, DT_LEFT);
		rad->AddScreenObject(TAG_ITEM_TYPE_DESTINATION, rt->GetCallsign(), rline3, TRUE, "");

		dc->SetTextColor(C_PPS_YELLOW);

		// Line 4
		RECT rline4;
		rline4.top = line4.y;
		rline4.left = line4.x;
		if (sfi.size() >2) {
			dc->DrawText(sfi.c_str(), &rline4, DT_LEFT | DT_CALCRECT);
			dc->DrawText(sfi.c_str(), &rline4, DT_LEFT);
			rad->AddScreenObject(TAG_ITEM_TYPE_PLANE_TYPE, rt->GetCallsign(), rline3, TRUE, "");
		}
	}

	// BRAVO TAGS
	if (CSiTRadar::mAcData[rt->GetCallsign()].tagType == 0 && rt->GetPosition().GetRadarFlags() != 1) {
		RECT bline0;
		RECT bline1;
		RECT bline2;
		RECT bline3;

		bline1.top = p.y - 7;
		bline1.left = p.x + 10;
		dc->DrawText(altThreeDigit.c_str(), &bline1, DT_LEFT | DT_CALCRECT);
		dc->DrawText(altThreeDigit.c_str(), &bline1, DT_LEFT);
		rad->AddScreenObject(TAG_ITEM_TYPE_ALTITUDE, rt->GetCallsign(), bline1, TRUE, "BRAVO ALT");

		if (abs(rt->GetVerticalSpeed()) > 400) {
			bline1.left = bline1.right;
			dc->DrawText(vmi.c_str(), &bline1, DT_LEFT | DT_CALCRECT);
			dc->DrawText(vmi.c_str(), &bline1, DT_LEFT);

			bline1.left = bline1.right;
			dc->DrawText(vmr.c_str(), &bline1, DT_LEFT | DT_CALCRECT);
			dc->DrawText(vmr.c_str(), &bline1, DT_LEFT);
		}
		bline1.left = bline1.right + 5;

	}

	// Uncorrelated 
	if (CSiTRadar::mAcData[rt->GetCallsign()].tagType == 3 
		&& rt->GetPosition().GetRadarFlags() != 1) {
		RECT uline0;
		RECT uline1;
		RECT uline2;
		RECT uline3;

		uline0.top = p.y - 19;
		uline0.left = p.x + 10;
		dc->DrawText(ssr.c_str(), &uline0, DT_LEFT | DT_CALCRECT);
		dc->DrawText(ssr.c_str(), &uline0, DT_LEFT);
		rad->AddScreenObject(TAG_ITEM_TYPE_SQUAWK, rt->GetCallsign(), uline0, TRUE, "");

		uline1.top = p.y - 7;
		uline1.left = p.x + 10;
		dc->DrawText(altThreeDigit.c_str(), &uline1, DT_LEFT | DT_CALCRECT);
		dc->DrawText(altThreeDigit.c_str(), &uline1, DT_LEFT);
		rad->AddScreenObject(TAG_ITEM_TYPE_ALTITUDE, rt->GetCallsign(), uline1, TRUE, "Uncorr ALT");

		if (abs(rt->GetVerticalSpeed()) > 400) {
			uline1.left = uline1.right;
			dc->DrawText(vmi.c_str(), &uline1, DT_LEFT | DT_CALCRECT);
			dc->DrawText(vmi.c_str(), &uline1, DT_LEFT);

			uline1.left = uline1.right;
			dc->DrawText(vmr.c_str(), &uline1, DT_LEFT | DT_CALCRECT);
			dc->DrawText(vmr.c_str(), &uline1, DT_LEFT);
		}
		uline1.left = uline1.right + 5;
	}


	// restore context
	dc->RestoreDC(sDC);

	// cleanup
	DeleteObject(font);
}

void CACTag::DrawFPConnector(CDC* dc, CRadarScreen* rad, CRadarTarget* rt, CFlightPlan* fp, COLORREF color, unordered_map<string, POINT>* tOffset)
{
	
	// save context
	int sDC = dc->SaveDC();

	POINT p{ 0,0 };
	POINT connector{ 0,0 };
	int tagOffsetX = 0;
	int tagOffsetY = 0;

	// get the tag off set from the TagOffset<map>
	POINT pTag = tOffset->find(fp->GetCallsign())->second;

	tagOffsetX = pTag.x;
	tagOffsetY = pTag.y;

	if (fp->IsValid()) {
		p = rad->ConvertCoordFromPositionToPixel(fp->GetFPTrackPosition().GetPosition());
	}
	else {
		if (rt->IsValid()) {
			p = rad->ConvertCoordFromPositionToPixel(rt->GetPosition().GetPosition());
		}
	}

	// determine if the tag is to the left or the right of the PPS

	if (pTag.x >= 0) { connector.x = (int)p.x + tagOffsetX - 3; };
	if (pTag.x < 0) { connector.x = (int)p.x + tagOffsetX - 3 + TAG_WIDTH; }
	connector.y = p.y + tagOffsetY + 7;

	// the connector is only drawn at 30, 45 or 60 degrees, set the theta to the nearest appropriate angle
	// get the angle between the line between the PPS and connector and horizontal

	// if vertical (don't divide by 0!)
	double theta = 30;
	double phi = 0;

	int doglegX = 0;
	int doglegY = 0;

	if (connector.x - p.x != 0) {
		
		double x = abs(connector.x - p.x); // use absolute value since coord system is upside down
		double y = abs(p.y - connector.y); // also cast as double for atan

		phi = atan(y/x); 
	
		// logic for if phi is a certain value; unit circle! (with fudge factor)
		if (phi >= 0 && phi < PI/6 ) { theta = 30; }
		else if(phi >= PI / 6 && phi < PI / 4) { theta = 45; }
		else if (phi >= PI / 4 && phi < PI / 3) { theta = 60; }

		theta = theta * PI / 180; // to radians
		doglegY = p.y + tagOffsetY + 7; // small padding to line it up with the middle of the first line

		// Calculate the x position of the intersection point (probably there is a more efficient way, but the atan drove me crazy
		doglegX = (int)(p.x + ((double)(p.y - (double)connector.y) / tan(theta))); // quad 1

		if (connector.x < p.x) { doglegX = (int)(p.x - ((double)(p.y - (double)connector.y) / tan(theta))); } // quadrant 2
		if (connector.y > p.y && connector.x > p.x) { doglegX = (int)(p.x - ((double)(p.y - (double)connector.y) / tan(theta))); } 
		if (connector.y > p.y && connector.x < p.x) { doglegX = (int)(p.x + ((double)(p.y - (double)connector.y) / tan(theta))); }
		if (phi >= PI / 3) { doglegX = p.x; } // same as directly above or below
	}
	else {
		doglegX = p.x; // if direction on top or below
		doglegY = p.y + tagOffsetY + 7;
	}

	// Draw the angled line and draw the horizontal line
	HPEN targetPen;
	targetPen = CreatePen(PS_SOLID, 1, color);
	dc->SelectObject(targetPen);
	dc->SelectStockObject(NULL_BRUSH);

	dc->MoveTo(p.x, p.y);
	dc->LineTo((int)doglegX, (int)doglegY); // line to the dogleg
	dc->LineTo(connector.x, (int)p.y + tagOffsetY + 7); // line to the connector point

	DeleteObject(targetPen);

	// restore
	dc->RestoreDC(sDC);
}

void CACTag::DrawRTConnector(CDC* dc, CRadarScreen* rad, CRadarTarget* rt, CFlightPlan* fp, COLORREF color, unordered_map<string, POINT>* tOffset)
{

}

/* DEBUG CODE

	//debug

	CFont font;
	LOGFONT lgfont;
	memset(&lgfont, 0, sizeof(LOGFONT));
	lgfont.lfHeight = 14;
	lgfont.lfWeight = 500;
	strcpy_s(lgfont.lfFaceName, _T("EuroScope"));
	font.CreateFontIndirect(&lgfont);
	dc->SelectObject(font);

	RECT debug;
	debug.top = 250;
	debug.left = 250;
	dc->DrawText(to_string(connector.y).c_str(), &debug, DT_LEFT);
	debug.top += 10;
	dc->DrawText(to_string(p.y).c_str(), &debug, DT_LEFT);
	debug.top += 10;
	dc->DrawText(to_string(connector.x).c_str(), &debug, DT_LEFT);
	debug.top += 10;
	dc->DrawText(to_string(phi).c_str(), &debug, DT_LEFT);
	DeleteObject(font);

	//debug

*/