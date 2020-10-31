#include "pch.h"
#include "ACTag.h"

using namespace EuroScopePlugIn;

void CACTag::DrawACTag(CDC* dc, CRadarScreen* rad, CRadarTarget* rt, CFlightPlan* fp) {

	POINT p;
	// Get aircraft information

	string icaoACData = rt->GetCorrelatedFlightPlan().GetFlightPlanData().GetAircraftInfo();
	regex icaoADSB("(.*)\\/(.*)\\-(.*)\\/(.*)(E|L|B1|B2|U1|U2|V1|V2)(.*)");
	bool isADSB = regex_search(icaoACData, icaoADSB);

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
	tagCallsign.left = p.x + 20;
	tagCallsign.right = p.x + 90;
	tagCallsign.top = p.y - 20;
	tagCallsign.bottom = p.y;

	RECT tagAltitude;
	tagAltitude.left = p.x + 20;
	tagAltitude.right = p.x + 90;
	tagAltitude.top = p.y - 9;
	tagAltitude.bottom = p.y + 20;
	
	RECT tagGS;
	tagGS.left = p.x + 50;
	tagGS.right = p.x + 90;
	tagGS.top = p.y - 9;
	tagGS.bottom = p.y + 20;
	

	if (rt->IsValid()) {
		
		dc->SetTextColor(RGB(202, 205, 169)); // standard yellow color

		// Handle tag drawing for ADSB aircraft

		if (rt->GetPosition().GetRadarFlags() == 0) { // If no radar returns and has ADSB equipment
			dc->DrawText(rt->GetCallsign(), &tagCallsign, DT_LEFT);
			string altThreeDigit = to_string(rt->GetPosition().GetFlightLevel() / 100);
			altThreeDigit.insert(altThreeDigit.begin(), 3 - altThreeDigit.size(), '0');
			dc->DrawText(altThreeDigit.c_str(), &tagAltitude, DT_LEFT);
			dc->DrawText(to_string(rt->GetPosition().GetReportedGS()/10).c_str(), &tagGS, DT_LEFT);
		}
	}

	// If not a valid radar target, then use the FP Track location and filed altitude

	else {
		if (fp->IsValid()) {

			dc->SetTextColor(RGB(242, 120, 57)); // FP Track in orange colour

			POINT p = rad->ConvertCoordFromPositionToPixel(fp->GetFPTrackPosition().GetPosition());

			fp->GetCallsign();
			fp->GetClearedAltitude();

			// Draw the text for the tag
			
			dc->DrawText(fp->GetCallsign(), &tagCallsign, DT_LEFT);
			dc->DrawText(to_string(fp->GetFinalAltitude()/100).c_str(), &tagAltitude, DT_LEFT);
		}
	}


	// restore context
	dc->RestoreDC(sDC);

	// cleanup
	DeleteObject(font);
}

static void DrawConnector(HDC hdc) {

}