#include "pch.h"
#include "PPS.h"

void CPPS::DrawPPS(CDC* dc, CRadarScreen* radScr, CRadarTarget* radTar, COLORREF ppsColor, POINT p)
{
	
	int sDC = dc->SaveDC();

	// Add the screenobject
	RECT prect;
	prect.left = p.x - 5;
	prect.top = p.y - 5;
	prect.right = p.x + 5;
	prect.bottom = p.y + 5;
	radScr->AddScreenObject(AIRCRAFT_SYMBOL, radTar->GetCallsign(), prect, FALSE, "");

	HPEN targetPen;

	// Get information about the Aircraft/Flightplan
	bool isCorrelated = radTar->GetCorrelatedFlightPlan().IsValid();
	bool isVFR = !strcmp(radTar->GetCorrelatedFlightPlan().GetFlightPlanData().GetPlanType(), "V");
	string icaoACData = radScr->GetPlugIn()->FlightPlanSelect(radTar->GetCallsign()).GetFlightPlanData().GetAircraftInfo(); // logic to 
	regex icaoRVSM("(.*)\\/(.*)\\-(.*)[W](.*)\\/(.*)", regex::icase);
	bool isRVSM = regex_search(icaoACData, icaoRVSM); // first check for ICAO; then check FAA
	if (radScr->GetPlugIn()->FlightPlanSelect(radTar->GetCallsign()).GetFlightPlanData().GetCapibilities() == 'L' ||
		radScr->GetPlugIn()->FlightPlanSelect(radTar->GetCallsign()).GetFlightPlanData().GetCapibilities() == 'W' ||
		radScr->GetPlugIn()->FlightPlanSelect(radTar->GetCallsign()).GetFlightPlanData().GetCapibilities() == 'Z') {
		isRVSM = TRUE;
	}
	/// DRAWING SYMBOLS ///

	targetPen = CreatePen(PS_SOLID, 1, ppsColor);
	dc->SelectObject(targetPen);
	dc->SelectStockObject(NULL_BRUSH);

	// else if no radar returns -> is it ADSB?
	if (radTar->GetPosition().GetRadarFlags() == 0) {
		// Draw ADSB PPS symbology
	}

	else {
		// Special Codes
		if (!strcmp(radTar->GetPosition().GetSquawk(), "7600") || !strcmp(radTar->GetPosition().GetSquawk(), "7700")) {
			HBRUSH targetBrush = CreateSolidBrush(ppsColor);
			dc->SelectObject(targetBrush);

			POINT vertices[] = { { p.x - 4, p.y + 4 } , { p.x, p.y - 4 } , { p.x + 4,p.y + 4 } }; // Red triangle
			dc->Polygon(vertices, 3);
			DeleteObject(targetBrush);
		}
		else if (radTar->GetPosition().GetRadarFlags() == 1) {
			
			dc->MoveTo(p.x, p.y + 4);	// Magenta Y 
			dc->LineTo(p.x, p.y);
			dc->LineTo(p.x - 4, p.y - 4);
			dc->MoveTo(p.x, p.y);
			dc->LineTo(p.x + 4, p.y - 4);
		}
		// IFR correlated
		else if (radTar->GetPosition().GetRadarFlags() == 2) {
			if (isCorrelated && !isVFR && !isRVSM) {
				dc->MoveTo(p.x - 4, p.y - 2);
				dc->LineTo(p.x - 4, p.y + 2);
				dc->LineTo(p.x, p.y + 5);
				dc->LineTo(p.x + 4, p.y + 2);
				dc->LineTo(p.x + 4, p.y - 2);
				dc->LineTo(p.x, p.y - 5);
				dc->LineTo(p.x - 4, p.y - 2);
			}
			if (isCorrelated && !isVFR && isRVSM) {
				dc->MoveTo(p.x, p.y - 5);
				dc->LineTo(p.x + 5, p.y);
				dc->LineTo(p.x, p.y + 5);
				dc->LineTo(p.x - 5, p.y);
				dc->LineTo(p.x, p.y - 5);
			}
			if (isCorrelated && isVFR) {
				dc->SelectStockObject(NULL_BRUSH);

				// draw the shape
				dc->Ellipse(p.x - 4, p.y - 4, p.x + 6, p.y + 6);

				dc->SelectObject(targetPen);
				dc->MoveTo(p.x - 3, p.y - 2);
				dc->LineTo(p.x + 1, p.y + 4);
				dc->LineTo(p.x + 4, p.y - 2);
			}

		}
		else if (radTar->GetPosition().GetRadarFlags() >= 3 && radTar->GetPosition().GetRadarFlags() <= 7) {
			if (isCorrelated && !isVFR && !isRVSM) {
				dc->MoveTo(p.x - 4, p.y - 2);
				dc->LineTo(p.x - 4, p.y + 2);
				dc->LineTo(p.x, p.y + 5);
				dc->LineTo(p.x + 4, p.y + 2);
				dc->LineTo(p.x + 4, p.y - 2);
				dc->LineTo(p.x, p.y - 5);
				dc->LineTo(p.x - 4, p.y - 2);

				dc->MoveTo(p.x - 4, p.y + 2);
				dc->LineTo(p.x, p.y - 4);
				dc->LineTo(p.x + 4, p.y + 2);
				dc->LineTo(p.x - 4, p.y + 2);
			}
			
			if (isCorrelated && !isVFR && isRVSM) {
				dc->MoveTo(p.x, p.y - 5);
				dc->LineTo(p.x + 5, p.y);
				dc->LineTo(p.x, p.y + 5);
				dc->LineTo(p.x - 5, p.y);
				dc->LineTo(p.x, p.y - 5);

				dc->MoveTo(p.x, p.y - 5);
				dc->LineTo(p.x, p.y + 5);
			}
			if (isCorrelated && isVFR) {
				dc->SelectStockObject(NULL_BRUSH);

				// draw the shape
				dc->Ellipse(p.x - 4, p.y - 4, p.x + 6, p.y + 6);

				dc->MoveTo(p.x - 3, p.y - 2);
				dc->LineTo(p.x + 1, p.y + 4);
				dc->LineTo(p.x + 4, p.y - 2);
			}
		}
	}
	
	DeleteObject(targetPen);

	/// END DRAWING SYMBOLS ///


	// Draw halo

	// Draw the PTL


	dc->RestoreDC(sDC);
}

