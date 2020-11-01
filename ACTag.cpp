#include "pch.h"
#include "ACTag.h"

using namespace EuroScopePlugIn;

void CACTag::DrawACTag(CDC* dc, CRadarScreen* rad, CRadarTarget* rt, CFlightPlan* fp, map<string, POINT>* tOffset) {

	POINT p{0,0};
	int tagOffsetX = 0;
	int tagOffsetY = 0;

	// Get aircraft information

	string icaoACData = rt->GetCorrelatedFlightPlan().GetFlightPlanData().GetAircraftInfo();
	regex icaoADSB("(.*)\\/(.*)\\-(.*)\\/(.*)(E|L|B1|B2|U1|U2|V1|V2)(.*)");
	bool isADSB = regex_search(icaoACData, icaoADSB);

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
	tagGS.left = p.x + tagOffsetX + 30;
	tagGS.top = p.y + tagOffsetY + 10;
	

	if (rt->IsValid()) {
		
		dc->SetTextColor(C_PPS_YELLOW); // standard yellow color

		// Handle tag drawing for ADSB aircraft

		if (rt->GetPosition().GetRadarFlags() == 0) { // If no radar returns and has ADSB equipment
			dc->DrawText(rt->GetCallsign(), &tagCallsign, DT_LEFT | DT_CALCRECT);
			dc->DrawText(rt->GetCallsign(), &tagCallsign, DT_LEFT);
			rad->AddScreenObject(TAG_ITEM_CS, rt->GetCallsign(), tagCallsign, TRUE, rt->GetCallsign());
			string altThreeDigit = to_string(rt->GetPosition().GetFlightLevel() / 100);
			altThreeDigit.insert(altThreeDigit.begin(), 3 - altThreeDigit.size(), '0');
			dc->DrawText(altThreeDigit.c_str(), &tagAltitude, DT_LEFT | DT_CALCRECT);
			dc->DrawText(altThreeDigit.c_str(), &tagAltitude, DT_LEFT);
			dc->DrawText(to_string(rt->GetPosition().GetReportedGS()/10).c_str(), &tagGS, DT_LEFT);
		}
	}

	// If not a valid radar target, then use the FP Track location and filed altitude

	else {
		if (fp->IsValid()) {

			dc->SetTextColor(C_PPS_ORANGE); // FP Track in orange colour

			POINT p = rad->ConvertCoordFromPositionToPixel(fp->GetFPTrackPosition().GetPosition());

			fp->GetCallsign();
			fp->GetClearedAltitude();

			// Draw the text for the tag
			
			dc->DrawText(fp->GetCallsign(), &tagCallsign, DT_LEFT | DT_CALCRECT);
			dc->DrawText(fp->GetCallsign(), &tagCallsign, DT_LEFT);
			rad->AddScreenObject(TAG_ITEM_TYPE_CALLSIGN, fp->GetCallsign(), tagCallsign, TRUE, fp->GetCallsign());
			dc->DrawText(to_string(fp->GetFinalAltitude()/100).c_str(), &tagAltitude, DT_LEFT | DT_CALCRECT);
			dc->DrawText(to_string(fp->GetFinalAltitude() / 100).c_str(), &tagAltitude, DT_LEFT);
			rad->AddScreenObject(TAG_ITEM_TYPE_FINAL_ALTITUDE, fp->GetCallsign(), tagAltitude, TRUE, "ALT");
		}
	}


	// restore context
	dc->RestoreDC(sDC);

	// cleanup
	DeleteObject(font);
}

void CACTag::DrawConnector(CDC* dc, CRadarScreen* rad, CRadarTarget* rt, CFlightPlan* fp, COLORREF color, map<string, POINT>* tOffset)
{
	
	// save context
	int sDC = dc->SaveDC();

	POINT p{ 0,0 };
	int tagOffsetX = 0;
	int tagOffsetY = 0;

	// get the tag off set from the TagOffset<map>
	POINT pTag = tOffset->find(fp->GetCallsign())->second;

	tagOffsetX = pTag.x;
	tagOffsetY = pTag.y;

	if (rt->IsValid()) {
		p = rad->ConvertCoordFromPositionToPixel(rt->GetPosition().GetPosition());
	}
	else {
		if (fp->IsValid()) {
			p = rad->ConvertCoordFromPositionToPixel(fp->GetFPTrackPosition().GetPosition());
		}
	}

	double theta = 60 * PI / 180; // default will add variable angle logic

	int doglegX = 0;
	int doglegY = 0;
	doglegY = p.y + tagOffsetY + 7; // will equal the tag offset value once that is implemented

	// Calculate the x position of the intersection point
	doglegX = (int)(p.x + ((double)(p.y) - ((double)p.y - 15)) / tan(theta));

	// Draw the angled line and draw the horizontal line
	HPEN targetPen;
	targetPen = CreatePen(PS_SOLID, 1, color);
	dc->SelectObject(targetPen);
	dc->SelectStockObject(NULL_BRUSH);

	dc->MoveTo(p.x, p.y);
	dc->LineTo((int)doglegX, (int)doglegY);
	dc->LineTo((int)p.x + tagOffsetX - 3, (int)p.y + tagOffsetY + 7);

	DeleteObject(targetPen);

	// restore
	dc->RestoreDC(sDC);
}