#include "pch.h"
#include "ACTag.h"

using namespace EuroScopePlugIn;

void CACTag::DrawFPACTag(CDC *dc, CRadarScreen *rad, CRadarTarget *rt, CFlightPlan *fp, unordered_map<string, POINT> *tOffset)
{

	POINT p{0, 0};
	int tagOffsetX = 0;
	int tagOffsetY = 0;

	// Initiate the default tag location, if no location is set already or find it in the map

	if (tOffset->find(fp->GetCallsign()) == tOffset->end())
	{
		POINT pTag{20, -20};
		tOffset->insert(pair<string, POINT>(fp->GetCallsign(), pTag));

		tagOffsetX = pTag.x;
		tagOffsetY = pTag.y;
	}
	else
	{
		POINT pTag = tOffset->find(fp->GetCallsign())->second;

		tagOffsetX = pTag.x;
		tagOffsetY = pTag.y;
	}

	// save context
	int sDC = dc->SaveDC();

	CFont font;
	CFont boldfont;
	LOGFONT lgfont;
	memset(&lgfont, 0, sizeof(LOGFONT));
	lgfont.lfHeight = 14;
	lgfont.lfWeight = 500;
	strcpy_s(lgfont.lfFaceName, _T("EuroScope"));
	font.CreateFontIndirect(&lgfont);
	dc->SelectObject(font);

	// Find position of aircraft
	if (rt->IsValid())
	{
		p = rad->ConvertCoordFromPositionToPixel(rt->GetPosition().GetPosition());
	}
	else
	{
		if (fp->IsValid())
		{
			p = rad->ConvertCoordFromPositionToPixel(fp->GetFPTrackPosition().GetPosition());
		}
	}

	// Tag formatting
	RECT tagCallsign;
	tagCallsign.left = p.x + tagOffsetX;
	tagCallsign.top = p.y + tagOffsetY;

	RECT tagAltitude;
	tagAltitude.left = p.x + tagOffsetX;
	tagAltitude.top = p.y + tagOffsetY + 10;

	RECT tagClearedAltitude;
	tagClearedAltitude.left = p.x + tagOffsetX + 40;
	tagClearedAltitude.top = p.y + tagOffsetY + 10;

	POINT line0 = { p.x + tagOffsetX, p.y + tagOffsetY - 12 };
	POINT line1 = { p.x + tagOffsetX, p.y + tagOffsetY };
	POINT line2 = { p.x + tagOffsetX, p.y + tagOffsetY + 11 };
	POINT line3 = { p.x + tagOffsetX, p.y + tagOffsetY + 22 };
	POINT line4 = { p.x + tagOffsetX, p.y + tagOffsetY + 33 };
	POINT line5 = { p.x + tagOffsetX, p.y + tagOffsetY + 44 };
	POINT line6 = { p.x + tagOffsetX, p.y + tagOffsetY + 55 };

	if (fp->IsValid())
	{
		dc->SetTextColor(C_PPS_ORANGE); // FP Track in orange colour

		POINT p = rad->ConvertCoordFromPositionToPixel(fp->GetFPTrackPosition().GetPosition());

		// Parse the CS and Wt Symbol
		string cs = fp->GetCallsign();
		string wtSymbol = "";
		if (fp->GetFlightPlanData().GetAircraftWtc() == 'H')
		{
			wtSymbol = "+";
		}
		if (fp->GetFlightPlanData().GetAircraftWtc() == 'L')
		{
			wtSymbol = "-";
		}
		if (fp->GetFlightPlanData().GetAircraftWtc() == 'J')
		{
			wtSymbol = "$";
		}
		cs = cs + wtSymbol;

		fp->GetClearedAltitude();

		string clearedAltStr = to_string(fp->GetControllerAssignedData().GetClearedAltitude() / 100);
		if (fp->GetControllerAssignedData().GetClearedAltitude() < 1)
			clearedAltStr = to_string(fp->GetFlightPlanData().GetFinalAltitude() / 100);

		string reportedAltitude = "rpt";
		if (strcmp(fp->GetControllerAssignedData().GetFlightStripAnnotation(8), "") != 0)
			reportedAltitude = fp->GetControllerAssignedData().GetFlightStripAnnotation(8);

		// Display extrapolated alpha tag
		if (fp->GetTrackingControllerIsMe())
		{
			// Line 0 - SSR code and coordination

			// Line 1 - Medevac status and callsign
			RECT rline1;
			rline1.top = line1.y;
			rline1.left = line1.x;
			rline1.bottom = line2.y;

			string remarks = fp->GetFlightPlanData().GetRemarks();
			if (!remarks.find("STS/MEDEVAC"))
			{
				dc->SetTextColor(C_PPS_RED);

				dc->SelectObject(boldfont);
				
				dc->DrawText("+", &rline1, DT_LEFT | DT_CALCRECT);
				dc->DrawText("+", &rline1, DT_LEFT);

				rline1.left = rline1.right;
				rline1.right = rline1.left;

				dc->SelectObject(font);
			}

			dc->SetTextColor(C_PPS_ORANGE);

			dc->DrawText(cs.c_str(), &rline1, DT_LEFT | DT_CALCRECT);
			dc->DrawText(cs.c_str(), &rline1, DT_LEFT);

			rad->AddScreenObject(TAG_ITEM_FP_CS, fp->GetCallsign(), rline1, TRUE, fp->GetCallsign());

			// Line 2 - Altitude, cleared altitude, handoff CJS, speed
			RECT rline2;
			rline2.top = line2.y;
			rline2.left = line2.x;
			rline2.bottom = line3.y;

			dc->DrawText(reportedAltitude.c_str(), &rline2, DT_LEFT | DT_CALCRECT);
			dc->DrawText(reportedAltitude.c_str(), &rline2, DT_LEFT);

			if (abs(rt->GetVerticalSpeed()) > 400)
			{
				rline2.left = rline2.right;
				dc->DrawText("/\\", &rline2, DT_LEFT | DT_CALCRECT);
				dc->DrawText("/\\", &rline2, DT_LEFT);

				rline2.left = rline2.right;
				dc->DrawText("/\\", &rline2, DT_LEFT | DT_CALCRECT);
				dc->DrawText("/\\", &rline2, DT_LEFT);
			}
			rline2.left = rline2.right + 8;

			string clrdAlt = to_string(fp->GetControllerAssignedData().GetClearedAltitude() / 100);
			if (clrdAlt.size() <= 3)
			{
				clrdAlt.insert(clrdAlt.begin(), 3 - clrdAlt.size(), '0');
			}
			if (fp->GetControllerAssignedData().GetClearedAltitude() == 0)
			{
				clrdAlt = "clr";
			}
			if (fp->GetControllerAssignedData().GetClearedAltitude() == 1)
			{
				clrdAlt = "APR";
			}
			if (fp->GetControllerAssignedData().GetClearedAltitude() == 2)
			{
				clrdAlt = "APR";
			}
			string fpAlt = to_string(fp->GetFlightPlanData().GetFinalAltitude() / 100);
			if (fpAlt.size() <= 3)
			{
				fpAlt.insert(fpAlt.begin(), 3 - fpAlt.size(), '0');
			}
			if (fp->GetFlightPlanData().GetFinalAltitude() == 0)
			{
				fpAlt = "fld";
			}
			string handoffCJS = fp->GetHandoffTargetControllerId();
			if (strcmp(fp->GetHandoffTargetControllerId(), rad->GetPlugIn()->ControllerMyself().GetPositionId()) == 0)
			{
				handoffCJS = fp->GetTrackingControllerId();
			}

			string groundSpeed = to_string((rt->GetPosition().GetReportedGS() + 5) / 10);
			string setSpeed = to_string(fp->GetControllerAssignedData().GetAssignedSpeed());
			string setMach = to_string(fp->GetControllerAssignedData().GetAssignedMach());
			string adsbMach = to_string(fp->GetFlightPlanData().PerformanceGetMach(rt->GetPosition().GetFlightLevel(), 0));

			if (
				// altitude differential
				true

				// or extended altitudes toggled on
				|| (CSiTRadar::menuState.extAltToggle && CSiTRadar::mAcData[rt->GetCallsign()].extAlt))
			{
				dc->DrawText(("C" + clrdAlt).c_str(), &rline2, DT_LEFT | DT_CALCRECT);
				dc->DrawText(("C" + clrdAlt).c_str(), &rline2, DT_LEFT);
				rline2.left = rline2.right + 8;
			}

			if (CSiTRadar::menuState.extAltToggle && CSiTRadar::mAcData[rt->GetCallsign()].extAlt)
			{
				dc->DrawText(("F" + fpAlt).c_str(), &rline2, DT_LEFT | DT_CALCRECT);
				dc->DrawText(("F" + fpAlt).c_str(), &rline2, DT_LEFT);
				rline2.left = rline2.right + 8;
			}

			dc->DrawText(to_string(fp->GetFlightPlanData().GetTrueAirspeed() / 10).c_str(), &rline2, DT_LEFT | DT_CALCRECT);
			dc->DrawText(to_string(fp->GetFlightPlanData().GetTrueAirspeed() / 10).c_str(), &rline2, DT_LEFT);

			string nextPoint;
			string nextPointTime;
			string nextNextPoint;
			string nextNextPointTime;

			// Line 3 - Next fix, next fix time, next next fix, next next fix time
			//RECT rline3;
			//rline3.top = line3.y;
			//rline3.left = line3.x;
			//rline3.bottom = line4.y;

			//int positionArrLength = fp->GetExtractedRoute().GetPointsNumber();

			//for (int i = 0; i < positionArrLength; i++) {
			//	if (fp->GetExtractedRoute().GetPointDistanceInMinutes(i) == -1)
			//		positionArrLength--;
			//	else {
			//		positionArrLength--;
			//		break;
			//	}
			//}

			//string line3;
			//if (positionArrLength > 0) {
			//	if (positionArrLength > 1)
			//	{
			//		nextNextPoint = fp->GetExtractedRoute().GetPointName(2);

			//		struct tm gmt;
			//		time_t t = std::time(0);
			//		t += static_cast<time_t>((fp->GetExtractedRoute().GetPointDistanceInMinutes(2) * 60));
			//		gmtime_s(&gmt, &t);

			//		char timeStr[50];
			//		strftime(timeStr, 50, "%H%I", &gmt);

			//		nextNextPointTime = timeStr;
			//	}
			//	if (positionArrLength > 0)
			//	{
			//		nextPoint = fp->GetExtractedRoute().GetPointName(1);

			//		struct tm gmt;
			//		time_t t = std::time(0);
			//		t += static_cast<time_t>((fp->GetExtractedRoute().GetPointDistanceInMinutes(1) * 60));
			//		gmtime_s(&gmt, &t);

			//		char timeStr[50];
			//		strftime(timeStr, 50, "%H%I", &gmt);

			//		nextPointTime = timeStr;

			//		nextNextPoint = fp->GetFlightPlanData().GetDestination();
			//		nextNextPointTime = fp->GetPositionPredictions().GetPointsNumber();
			//	}

			//	line3 = nextPoint + ' ' + nextPointTime + ' ' + nextNextPoint + ' ' + nextNextPointTime;
			//}
			//else {
			//	nextPoint = fp->GetFlightPlanData().GetDestination();

			//	struct tm gmt;
			//	time_t t = std::time(0);
			//	t += static_cast<time_t>((fp->GetPositionPredictions().GetPointsNumber() * 60));
			//	gmtime_s(&gmt, &t);

			//	char timeStr[50];
			//	strftime(timeStr, 50, "%H%I", &gmt);

			//	nextPointTime = timeStr;

			//	line3 = nextPoint + ' ' + nextPointTime;
			//}

			//dc->DrawText(line3.c_str(), &rline3, DT_LEFT | DT_CALCRECT);
			//dc->DrawText(line3.c_str(), &rline3, DT_LEFT);

			// Line 4 - AC Type, destination
			RECT rline4;
			rline4.top = line4.y;
			rline4.left = line4.x;
			rline4.bottom = line5.y;

			dc->DrawText(fp->GetFlightPlanData().GetAircraftFPType(), &rline4, DT_LEFT | DT_CALCRECT);
			dc->DrawText(fp->GetFlightPlanData().GetAircraftFPType(), &rline4, DT_LEFT);
			rad->AddScreenObject(TAG_ITEM_TYPE_PLANE_TYPE, rt->GetCallsign(), rline4, TRUE, "");
			rline4.left = rline4.right + 10;

			dc->DrawText(fp->GetFlightPlanData().GetDestination(), &rline4, DT_LEFT | DT_CALCRECT);
			dc->DrawText(fp->GetFlightPlanData().GetDestination(), &rline4, DT_LEFT);
			rad->AddScreenObject(TAG_ITEM_TYPE_DESTINATION, rt->GetCallsign(), rline4, TRUE, "");

			// Line 5 - Arrival runway, RNAV indicator, Controller remarks
		}
		// Display extrapolated tag
		else
		{
			dc->DrawText(cs.c_str(), &tagCallsign, DT_LEFT | DT_CALCRECT);
			dc->DrawText(cs.c_str(), &tagCallsign, DT_LEFT);

			dc->DrawText(reportedAltitude.c_str(), &tagAltitude, DT_LEFT | DT_CALCRECT);
			dc->DrawText(reportedAltitude.c_str(), &tagAltitude, DT_LEFT);

			dc->DrawText(clearedAltStr.c_str(), &tagClearedAltitude, DT_LEFT | DT_CALCRECT);
			dc->DrawText(clearedAltStr.c_str(), &tagClearedAltitude, DT_LEFT);

			// Add the screen obects, TAG_FP_AREA first so that the others go on top;

			rad->AddScreenObject(TAG_ITEM_FP_CS, fp->GetCallsign(), tagCallsign, TRUE, fp->GetCallsign());
			rad->AddScreenObject(TAG_ITEM_FP_FINAL_ALTITUDE, fp->GetCallsign(), tagAltitude, TRUE, "ALT");
		}
	}

	// restore context
	dc->RestoreDC(sDC);

	// cleanup
	DeleteObject(font);
}

// Draws tag for Radar Targets

COLORREF CACTag::ColorPicker(CRadarTarget* rt, bool handoff, COLORREF defColor = C_PPS_YELLOW)
{
	if (!strcmp(rt->GetPosition().GetSquawk(), "7600") || !strcmp(rt->GetPosition().GetSquawk(), "7700"))
	{
		return C_PPS_RED;
	}
	else if (rt->GetPosition().GetTransponderI())
		return C_WHITE;
	else if ((
			rt->GetCorrelatedFlightPlan().GetTrackingControllerIsMe() &&
			rt->GetCorrelatedFlightPlan().GetSectorExitMinutes() < 3 &&
			rt->GetCorrelatedFlightPlan().GetSectorExitMinutes() > 0 &&
			strcmp(rt->GetCorrelatedFlightPlan().GetHandoffTargetControllerId(), "") == 0
		) && defColor == C_PPS_YELLOW)
	{
		if (CSiTRadar::halfSecTick)
			return C_PPS_DARK_YELOW;
		else
			return C_PPS_YELLOW;
	}
	else
		return defColor;
}

void CACTag::DrawRTACTag(CDC *dc, CRadarScreen *rad, CRadarTarget *rt, CFlightPlan *fp, unordered_map<string, POINT> *tOffset)
{
	POINT p{0, 0};
	p = rad->ConvertCoordFromPositionToPixel(rt->GetPosition().GetPosition());
	int tagOffsetX = 0;
	int tagOffsetY = 0;

	COLORREF defaultColor;
	bool blinking;

	if ((strcmp(fp->GetHandoffTargetControllerId(), rad->GetPlugIn()->ControllerMyself().GetPositionId()) == 0 && rad->GetPlugIn()->ControllerMyself().IsController()) ||
		CSiTRadar::hoAcceptedTime.find(fp->GetCallsign()) != CSiTRadar::hoAcceptedTime.end()
	)
	{
		defaultColor = CACTag::ColorPicker(rt, true);
		blinking = true;
	}
	else
	{
		defaultColor = CACTag::ColorPicker(rt, false);
		blinking = false;
	}

	// Destination airport highlighting
	auto itr = std::find(begin(CSiTRadar::menuState.destICAO), end(CSiTRadar::menuState.destICAO), fp->GetFlightPlanData().GetDestination());
	bool isDest = false;

	if (itr != end(CSiTRadar::menuState.destICAO) && strcmp(fp->GetFlightPlanData().GetDestination(), "") != 0)
	{
		if (CSiTRadar::menuState.destArptOn[distance(CSiTRadar::menuState.destICAO, itr)])
		{

			HPEN targetPen = CreatePen(PS_SOLID, 1, C_WHITE);
			dc->SelectObject(targetPen);
			dc->SelectStockObject(HOLLOW_BRUSH);
			dc->Ellipse(p.x - 7, p.y - 6, p.x + 8, p.y + 8);

			isDest = true;

			DeleteObject(targetPen);
		}
	}

	// Line 0 Items
	string ssr = rt->GetPosition().GetSquawk();

	// Line 1 Items
	string cs = fp->GetCallsign();
	string wtSymbol = "";
	if (rad->GetPlugIn()->FlightPlanSelect(cs.c_str()).GetFlightPlanData().GetAircraftWtc() == 'H')
	{
		wtSymbol = "+";
	}
	if (rad->GetPlugIn()->FlightPlanSelect(cs.c_str()).GetFlightPlanData().GetAircraftWtc() == 'L')
	{
		wtSymbol = "-";
	}
	if (rad->GetPlugIn()->FlightPlanSelect(cs.c_str()).GetFlightPlanData().GetAircraftWtc() == 'J')
	{
		wtSymbol = "$";
	}
	cs = cs + wtSymbol;

	char commTypeChar = tolower(fp->GetControllerAssignedData().GetCommunicationType());
	if (commTypeChar == '\0')
	{
		commTypeChar = tolower(fp->GetFlightPlanData().GetCommunicationType());
	}
	string commType = "";
	if (commTypeChar != 'v')
	{
		commType = "/";
		commType += commTypeChar;
	}

	string sfi = fp->GetControllerAssignedData().GetScratchPadString();

	// Line 2 Items
	string altThreeDigit;
	if (rt->GetPosition().GetPressureAltitude() > rad->GetPlugIn()->GetTransitionAltitude())
	{
		altThreeDigit = to_string((rt->GetPosition().GetFlightLevel() + 50) / 100); // +50 to force rounding up
	}
	else
	{
		altThreeDigit = to_string((rt->GetPosition().GetPressureAltitude() + 50) / 100);
	}
	if (altThreeDigit.size() <= 3)
	{
		altThreeDigit.insert(altThreeDigit.begin(), 3 - altThreeDigit.size(), '0');
	}
	string vmi;
	if (rt->GetVerticalSpeed() > 400)
	{
		vmi = "^";
	}
	if (rt->GetVerticalSpeed() < -400)
	{
		vmi = "|";
	}; // up arrow "??!" = downarrow
	string vmr = to_string(abs(rt->GetVerticalSpeed() / 200));
	if (vmr.size() <= 2)
	{
		vmr.insert(vmr.begin(), 2 - vmr.size(), '0');
	}
	string clrdAlt = to_string(fp->GetControllerAssignedData().GetClearedAltitude() / 100);
	if (clrdAlt.size() <= 3)
	{
		clrdAlt.insert(clrdAlt.begin(), 3 - clrdAlt.size(), '0');
	}
	if (fp->GetControllerAssignedData().GetClearedAltitude() == 0)
	{
		clrdAlt = "clr";
	}
	if (fp->GetControllerAssignedData().GetClearedAltitude() == 1 || fp->GetControllerAssignedData().GetClearedAltitude() == 2)
	{
		clrdAlt = "APR";
	}
	string fpAlt = to_string(fp->GetFlightPlanData().GetFinalAltitude() / 100);
	if (fpAlt.size() <= 3)
	{
		fpAlt.insert(fpAlt.begin(), 3 - fpAlt.size(), '0');
	}
	if (fp->GetFlightPlanData().GetFinalAltitude() == 0)
	{
		fpAlt = "fld";
	}
	string handoffCJS = fp->GetHandoffTargetControllerId();
	if (strcmp(fp->GetHandoffTargetControllerId(), rad->GetPlugIn()->ControllerMyself().GetPositionId()) == 0)
	{
		handoffCJS = fp->GetTrackingControllerId();
	}

	string groundSpeed = to_string((rt->GetPosition().GetReportedGS() + 5) / 10);
	string setSpeed = to_string(fp->GetControllerAssignedData().GetAssignedSpeed());
	string setMach = to_string(fp->GetControllerAssignedData().GetAssignedMach());
	string adsbMach = to_string(fp->GetFlightPlanData().PerformanceGetMach(rt->GetPosition().GetFlightLevel(), 0));

	// Line 3 Items
	string acType = fp->GetFlightPlanData().GetAircraftFPType();
	string destination = fp->GetFlightPlanData().GetDestination();
	CPosition dest;

	rad->GetPlugIn()->SelectActiveSectorfile();
	for (CSectorElement sectorElement = rad->GetPlugIn()->SectorFileElementSelectFirst(SECTOR_ELEMENT_AIRPORT); sectorElement.IsValid();
		sectorElement = rad->GetPlugIn()->SectorFileElementSelectNext(sectorElement, SECTOR_ELEMENT_AIRPORT))
	{
		if (!strcmp(sectorElement.GetName(), destination.c_str()))
		{
			sectorElement.GetPosition(&dest, 0);
		}
	}
	string destinationDist, destinationTime;
	// if the destination airport is not in the sector file, have to use Euroscope's FP calculated distance and not a direct distance
	if (dest.m_Latitude == 0.0 && dest.m_Longitude == 0.0)
	{
		destinationDist = to_string((long)fp->GetDistanceToDestination());
	}
	// otherwise, the display should be direct distance which can be more accurate calculated if in the SCT file.
	else
	{
		destinationDist = to_string((long)rt->GetPosition().GetPosition().DistanceTo(dest));
	}

	string est;
	if (rt->GetGS() > 0)
	{
		struct tm gmt;
		time_t t = std::time(0);
		t += static_cast<time_t>(((rt->GetPosition().GetPosition().DistanceTo(dest) / rt->GetGS()) * 3600));
		gmtime_s(&gmt, &t);

		char timeStr[50];
		strftime(timeStr, 50, "%R", &gmt);
		est = timeStr;
	}

	if (isDest)
	{
		if (CSiTRadar::menuState.destDME && !CSiTRadar::menuState.destEST)
		{
			destination = destination + "-" + destinationDist;
		}
		else if (CSiTRadar::menuState.destEST && !CSiTRadar::menuState.destDME)
		{
			destination = destination + "-" + est;
		}
		else if (CSiTRadar::menuState.destEST && CSiTRadar::menuState.destDME)
		{
			destination = destination + "-" + destinationDist + "-" + est;
		}
	}
	else
	{
		if (CSiTRadar::mAcData[rt->GetCallsign()].destLabelType == 1)
		{
			destination = destination + "-" + destinationDist;
		}
		if (CSiTRadar::mAcData[rt->GetCallsign()].destLabelType == 2)
		{
			destination = destination + "-" + est;
		}

		if (CSiTRadar::mAcData[rt->GetCallsign()].destLabelType == 3)
		{
			destination = destination + "-" + destinationDist + "-" + est;
		}
	}

	// Initiate the default tag location, if no location is set already or find it in the map

	if (tOffset->find(rt->GetCallsign()) == tOffset->end())
	{
		POINT pTag{20, -20};
		tOffset->insert(pair<string, POINT>(rt->GetCallsign(), pTag));

		tagOffsetX = pTag.x;
		tagOffsetY = pTag.y;
	}
	else
	{
		POINT pTag = tOffset->find(rt->GetCallsign())->second;

		tagOffsetX = pTag.x;
		tagOffsetY = pTag.y;
	}

	POINT line0 = {p.x + tagOffsetX, p.y + tagOffsetY - 12};
	POINT line1 = {p.x + tagOffsetX, p.y + tagOffsetY};
	POINT line2 = {p.x + tagOffsetX, p.y + tagOffsetY + 11};
	POINT line3 = {p.x + tagOffsetX, p.y + tagOffsetY + 22};
	POINT line4 = {p.x + tagOffsetX, p.y + tagOffsetY + 33};

	// save context
	int sDC = dc->SaveDC();

	CFont font;
	CFont boldfont;
	CFont bigACIDfont;
	LOGFONT lgfont;
	std::memset(&lgfont, 0, sizeof(LOGFONT));
	lgfont.lfHeight = 14;
	lgfont.lfWeight = 500;
	strcpy_s(lgfont.lfFaceName, _T("EuroScope"));
	font.CreateFontIndirect(&lgfont);
	lgfont.lfWeight = 1200;
	boldfont.CreateFontIndirect(&lgfont);
	lgfont.lfWeight = 500;
	lgfont.lfHeight = 16;
	bigACIDfont.CreateFontIndirect(&lgfont);
	dc->SelectObject(font);

	RECT rline1; // bring scope out to allow connector to be drawn

	if (CSiTRadar::mAcData[rt->GetCallsign()].tagType == 1 ||
		(CSiTRadar::mAcData[fp->GetCallsign()].isADSB && CSiTRadar::mAcData[fp->GetCallsign()].tagType == 1)
	)
	{
		// Tag formatting
		RECT tagCallsign;
		tagCallsign.left = p.x + tagOffsetX;
		tagCallsign.top = p.y + tagOffsetY;

		dc->SetTextColor(defaultColor);

		// Line 1

		rline1.top = line1.y;
		rline1.left = line1.x;
		rline1.bottom = line2.y;

		if (CSiTRadar::menuState.bigACID) {
			dc->SelectObject(bigACIDfont);
			rline1.top -= 2;
		}

		if (CSiTRadar::mAcData[rt->GetCallsign()].isMedevac)
		{
			dc->SetTextColor(CACTag::ColorPicker(rt, blinking, C_TAG_RED));

			dc->SelectObject(boldfont);

			dc->DrawText("+", &rline1, DT_LEFT | DT_CALCRECT);
			dc->DrawText("+", &rline1, DT_LEFT);

			rline1.left = rline1.right;
			rline1.right = rline1.left;
		}

		dc->SetTextColor(C_PPS_YELLOW);
		if (CSiTRadar::menuState.bigACID) {
			dc->SelectObject(bigACIDfont);
		}

		dc->SetTextColor(defaultColor);

		// SFI mode changes the ASEL aircraft ACID to white

		if (CSiTRadar::menuState.SFIMode &&
			strcmp(fp->GetCallsign(), CSiTRadar::m_pRadScr->GetPlugIn()->FlightPlanSelectASEL().GetCallsign()) == 0)
		{
			dc->SetTextColor(C_WHITE);
		}

		dc->DrawText(cs.c_str(), &rline1, DT_LEFT | DT_CALCRECT);
		dc->DrawText(cs.c_str(), &rline1, DT_LEFT);
		rad->AddScreenObject(TAG_ITEM_TYPE_CALLSIGN, fp->GetCallsign(), rline1, TRUE, fp->GetCallsign());
		rline1.left = rline1.right;
		rline1.right = rline1.left + 8;

		if (CSiTRadar::menuState.SFIMode &&
			strcmp(fp->GetCallsign(), CSiTRadar::m_pRadScr->GetPlugIn()->FlightPlanSelectASEL().GetCallsign()) == 0)
		{
			dc->SetTextColor(C_PPS_YELLOW);
		}

		// Show Communication Type if not Voice
		rad->AddScreenObject(TAG_ITEM_TYPE_COMMUNICATION_TYPE, rt->GetCallsign(), rline1, TRUE, rt->GetCallsign());
		if (commType.size() > 0)
		{
			dc->DrawText(commType.c_str(), &rline1, DT_LEFT | DT_CALCRECT);
			dc->DrawText(commType.c_str(), &rline1, DT_LEFT);
		}
		rline1.left = rline1.right;

		if (sfi.size() > 1 && sfi.find(" ", 2) != sfi.npos && sfi.find(" ", 2) < 3 && sfi.at(0) == ' ')
		{
			dc->DrawText(sfi.substr(1, 1).c_str(), &rline1, DT_LEFT | DT_CALCRECT);
			dc->DrawText(sfi.substr(1, 1).c_str(), &rline1, DT_LEFT);
		}
		else if (sfi.size() < 3 && sfi.size() != 0)
		{
			dc->DrawText(sfi.substr(1, 1).c_str(), &rline1, DT_LEFT | DT_CALCRECT);
			dc->DrawText(sfi.substr(1, 1).c_str(), &rline1, DT_LEFT);
		}

		rad->AddScreenObject(CTR_DATA_TYPE_SCRATCH_PAD_STRING, rt->GetCallsign(), rline1, TRUE, rt->GetCallsign());

		// add some padding for the SFI + long callsigns
		if (sfi.size() == 0)
		{
			CSiTRadar::mAcData[rt->GetCallsign()].tagWidth = rline1.right - tagCallsign.left + 12;
		}
		else
		{
			CSiTRadar::mAcData[rt->GetCallsign()].tagWidth = rline1.right - tagCallsign.left + 6;
		}

		dc->SelectObject(font);

		// Line 2
		RECT rline2;
		rline2.top = line2.y;
		rline2.left = line2.x;
		rline2.bottom = line3.y;

		if (fp->GetFlightPlanData().GetCapibilities() == 'Q' || fp->GetFlightPlanData().GetCapibilities() == 'L' || fp->GetFlightPlanData().GetCapibilities() == 'R')
		{
			dc->SetTextColor(CACTag::ColorPicker(rt, blinking, C_TAG_GREEN));
		}

		double alt;

		if (rt->GetPosition().GetPressureAltitude() > rad->GetPlugIn()->GetTransitionAltitude())
		{
			alt = rt->GetPosition().GetFlightLevel(); // +50 to force rounding up
		}
		else
		{
			alt = rt->GetPosition().GetPressureAltitude();
		}

		double calt = fp->GetControllerAssignedData().GetClearedAltitude();
		if (calt == 0)
			calt = fp->GetFlightPlanData().GetFinalAltitude();

		if (alt < calt - 200 || alt > calt + 200)
		{
			// If the altitude is increasing beyond cleared altitude, or descending below cleared altitude, display the altitude in blue
			// Todo - add logic for descending before cleared
			if (
				(alt > calt && rt->GetVerticalSpeed() > -400) ||
				(alt < calt && rt->GetVerticalSpeed() < 400) ||
				(rt->GetVerticalSpeed() < 400 && rt->GetVerticalSpeed() > -400)
			) {
				dc->SetTextColor(CACTag::ColorPicker(rt, blinking, C_TAG_BLUE));
			}
		}

		dc->DrawText(altThreeDigit.c_str(), &rline2, DT_LEFT | DT_CALCRECT);
		dc->DrawText(altThreeDigit.c_str(), &rline2, DT_LEFT);
		rad->AddScreenObject(TAG_ITEM_TYPE_ALTITUDE, rt->GetCallsign(), rline2, TRUE, "");

		dc->SetTextColor(defaultColor);

		if (abs(rt->GetVerticalSpeed()) > 400)
		{
			rline2.left = rline2.right;
			dc->DrawText(vmi.c_str(), &rline2, DT_LEFT | DT_CALCRECT);
			dc->DrawText(vmi.c_str(), &rline2, DT_LEFT);

			rline2.left = rline2.right;
			dc->DrawText(vmr.c_str(), &rline2, DT_LEFT | DT_CALCRECT);
			dc->DrawText(vmr.c_str(), &rline2, DT_LEFT);
		}
		rline2.left = rline2.right + 8;

		if (
			// altitude differential
			(abs(alt - fp->GetControllerAssignedData().GetClearedAltitude()) > 200 &&
			 fp->GetControllerAssignedData().GetClearedAltitude() != 0)

			// or extended altitudes toggled on
			|| (CSiTRadar::menuState.extAltToggle && CSiTRadar::mAcData[rt->GetCallsign()].extAlt))
		{
			dc->SetTextColor(CACTag::ColorPicker(rt, blinking, C_PPS_ORANGE));
			dc->DrawText(("C" + clrdAlt).c_str(), &rline2, DT_LEFT | DT_CALCRECT);
			dc->DrawText(("C" + clrdAlt).c_str(), &rline2, DT_LEFT);
			rline2.left = rline2.right + 8;
		}

		if (CSiTRadar::menuState.extAltToggle && CSiTRadar::mAcData[rt->GetCallsign()].extAlt)
		{
			dc->SetTextColor(CACTag::ColorPicker(rt, blinking, C_PPS_ORANGE));
			dc->DrawText(("F" + fpAlt).c_str(), &rline2, DT_LEFT | DT_CALCRECT);
			dc->DrawText(("F" + fpAlt).c_str(), &rline2, DT_LEFT);
			rline2.left = rline2.right + 8;
		}

		dc->SetTextColor(CACTag::ColorPicker(rt, blinking, RGB(255, 234, 46)));
		dc->DrawText(handoffCJS.c_str(), &rline2, DT_LEFT | DT_CALCRECT);
		dc->DrawText((handoffCJS).c_str(), &rline2, DT_LEFT);
		rline2.left = rline2.right + 8;
		if (rline2.left < p.x + tagOffsetX + 38)
		{
			rline2.left = p.x + tagOffsetX + 38;
		}

		dc->SetTextColor(defaultColor);

		dc->DrawText(to_string(rt->GetPosition().GetReportedGS() / 10).c_str(), &rline2, DT_LEFT | DT_CALCRECT);
		dc->DrawText(to_string(rt->GetPosition().GetReportedGS() / 10).c_str(), &rline2, DT_LEFT);
		rad->AddScreenObject(TAG_ITEM_TYPE_GROUND_SPEED_WITH_N, fp->GetCallsign(), rline2, TRUE, "");

		rline2.left = rline2.right + 8;

		dc->SetTextColor(CACTag::ColorPicker(rt, blinking, C_PPS_ORANGE));

		if (rt->GetPosition().GetRadarFlags() == 4 && rt->GetPosition().GetRadarFlags() != 2 && rt->GetPosition().GetFlightLevel() >= 28000)
		{
			if (fp->GetControllerAssignedData().GetAssignedSpeed() != 0)
			{
				setSpeed = setSpeed.insert(0, "A");
				dc->DrawText(setSpeed.c_str(), &rline2, DT_LEFT | DT_CALCRECT);
				dc->DrawText(setSpeed.c_str(), &rline2, DT_LEFT);
				rline2.left = rline2.right + 8;
				rad->AddScreenObject(TAG_ITEM_TYPE_ASSIGNED_HEADING, fp->GetCallsign(), rline2, TRUE, "");
			}
			else if (fp->GetControllerAssignedData().GetAssignedMach() != 0)
			{
				setMach = setMach.insert(0, "A.");
				dc->DrawText(setMach.c_str(), &rline2, DT_LEFT | DT_CALCRECT);
				dc->DrawText(setMach.c_str(), &rline2, DT_LEFT);
				rline2.left = rline2.right + 8;
				rad->AddScreenObject(TAG_ITEM_TYPE_ASSIGNED_HEADING, fp->GetCallsign(), rline2, TRUE, "");
			}
			else
			{
				adsbMach = adsbMach.insert(0, "M.");
				dc->DrawText(adsbMach.c_str(), &rline2, DT_LEFT | DT_CALCRECT);
				dc->DrawText(adsbMach.c_str(), &rline2, DT_LEFT);
				rline2.left = rline2.right + 8;
				rad->AddScreenObject(TAG_ITEM_TYPE_ASSIGNED_HEADING, fp->GetCallsign(), rline2, TRUE, "");
			}
		}
		else
		{
			if (fp->GetControllerAssignedData().GetAssignedSpeed() != 0)
			{
				setSpeed = setSpeed.insert(0, "A");
				dc->DrawText(setSpeed.c_str(), &rline2, DT_LEFT | DT_CALCRECT);
				dc->DrawText(setSpeed.c_str(), &rline2, DT_LEFT);
				rline2.left = rline2.right + 8;
				rad->AddScreenObject(TAG_ITEM_TYPE_ASSIGNED_HEADING, fp->GetCallsign(), rline2, TRUE, "");
			}
			if (fp->GetControllerAssignedData().GetAssignedMach() != 0)
			{
				setMach = setMach.insert(0, "A.");
				dc->DrawText(setMach.c_str(), &rline2, DT_LEFT | DT_CALCRECT);
				dc->DrawText(setMach.c_str(), &rline2, DT_LEFT);
				rline2.left = rline2.right + 8;
				rad->AddScreenObject(TAG_ITEM_TYPE_ASSIGNED_HEADING, fp->GetCallsign(), rline2, TRUE, "");
			}
		}

		// Line 3
		dc->SetTextColor(defaultColor);

		RECT rline3;
		rline3.top = line3.y;
		rline3.left = line3.x;
		rline3.bottom = line4.y;
		dc->DrawText(acType.c_str(), &rline3, DT_LEFT | DT_CALCRECT);
		dc->DrawText(acType.c_str(), &rline3, DT_LEFT);
		rad->AddScreenObject(TAG_ITEM_TYPE_PLANE_TYPE, rt->GetCallsign(), rline3, TRUE, "");
		rline3.left = rline3.right + 10;

		if (isDest)
		{
			dc->SetTextColor(C_WHITE);
		}

		dc->DrawText(destination.c_str(), &rline3, DT_LEFT | DT_CALCRECT);
		dc->DrawText(destination.c_str(), &rline3, DT_LEFT);
		rad->AddScreenObject(TAG_ITEM_TYPE_DESTINATION, rt->GetCallsign(), rline3, TRUE, "");

		if (isDest)
		{
			dc->SetTextColor(defaultColor);
		}

		// Line 4
		RECT rline4;
		rline4.top = line4.y;
		rline4.left = line4.x;

		// RNAV indicator is a N
		const char cap = fp->GetFlightPlanData().GetCapibilities();
		if (cap == 'G' || cap == 'R' || cap == 'W' || cap == 'L')
		{
			dc->DrawText("N", &rline4, DT_LEFT | DT_CALCRECT);
			dc->DrawText("N", &rline4, DT_LEFT);
			rline4.left = rline4.right + 10;
		}

		// Arrival runway

		// Controller remarks (scratchpad)
		if (sfi.size() > 2)
		{
			if (sfi.find(" ") != sfi.npos && sfi.find(" ") == 0)
			{
				sfi = sfi.substr(sfi.find(" ") + 1);
				// Find the second space: " N REMARKS" -> "REMARKS"
				if (sfi.find(" ") != sfi.npos && sfi.find(" ") == 1)
				{ // allows for spaces in remarks i.e. "NEW PILOT"
					sfi = sfi.substr(sfi.find(" ") + 1);
				}
			}
			dc->DrawText(sfi.c_str(), &rline4, DT_LEFT | DT_CALCRECT);
			dc->DrawText(sfi.c_str(), &rline4, DT_LEFT);
			rad->AddScreenObject(CTR_DATA_TYPE_SCRATCH_PAD_STRING, rt->GetCallsign(), rline4, TRUE, rt->GetCallsign());
		}
	}

	// Draw Connector

	int doglegX = 0;
	int doglegY = 0;

	if (CSiTRadar::mAcData[rt->GetCallsign()].tagType == 1 ||
		CSiTRadar::mAcData[rt->GetCallsign()].tagType == 2)
	{

		POINT connector{0, 0};
		int tagOffsetX = 0;
		int tagOffsetY = 0;

		// get the tag off set from the TagOffset<map>
		POINT pTag = tOffset->find(rt->GetCallsign())->second;

		tagOffsetX = pTag.x;
		tagOffsetY = pTag.y;

		// determine if the tag is to the left or the right of the PPS

		if (pTag.x >= 0)
		{
			connector.x = (int)p.x + tagOffsetX - 3;
		};
		if (pTag.x < 0)
		{
			connector.x = rline1.right + 3;
		}
		connector.y = p.y + tagOffsetY + 7;

		// the connector is only drawn at 30, 45 or 60 degrees, set the theta to the nearest appropriate angle
		// get the angle between the line between the PPS and connector and horizontal

		// if vertical (don't divide by 0!)
		double theta = 30;
		double phi = 0;
		POINT leaderOrigin = p;
		int PPSAreaRad = 9;

		if (connector.x - p.x != 0)
		{

			double x = abs(connector.x - p.x); // use absolute value since coord system is upside down
			double y = abs(p.y - connector.y); // also cast as double for atan

			phi = atan(y / x);

			// logic for if phi is a certain value; unit circle! (with fudge factor)
			if (phi >= 0 && phi < PI / 6)
			{
				theta = 30;
			}
			else if (phi >= PI / 6 && phi < PI / 4)
			{
				theta = 45;
			}
			else if (phi >= PI / 4 && phi < PI / 3)
			{
				theta = 60;
			}

			theta = theta * PI / 180;		// to radians
			doglegY = p.y + tagOffsetY + 7; // small padding to line it up with the middle of the first line

			// Calculate the x position of the intersection point (probably there is a more efficient way, but the atan drove me crazy
			doglegX = (int)(p.x + ((double)(p.y - (double)connector.y) / tan(theta))); // quad 1
			leaderOrigin.y = (int)(p.y - (PPSAreaRad)*sin(theta));
			leaderOrigin.x = (int)(p.x + (PPSAreaRad)*cos(theta));

			if (connector.x < p.x)
			{

				doglegX = (int)(p.x - ((double)(p.y - (double)connector.y) / tan(theta)));

				leaderOrigin.y = (int)(p.y - (PPSAreaRad)*sin(theta));
				leaderOrigin.x = (int)(p.x - (PPSAreaRad)*cos(theta));

			} // quadrant 2
			if (connector.y > p.y && connector.x > p.x)
			{

				doglegX = (int)(p.x - ((double)(p.y - (double)connector.y) / tan(theta)));

				leaderOrigin.y = (int)(p.y + (PPSAreaRad)*sin(theta));
				leaderOrigin.x = (int)(p.x + (PPSAreaRad)*cos(theta));
			}

			// Quadrant 3
			if (connector.y > p.y && connector.x < p.x)
			{

				doglegX = (int)(p.x + ((double)(p.y - (double)connector.y) / tan(theta)));

				leaderOrigin.y = (int)(p.y + (PPSAreaRad)*sin(theta));
				leaderOrigin.x = (int)(p.x - (PPSAreaRad)*cos(theta));
			}

			if (phi >= PI / 3)
			{
				doglegX = p.x;

				if (doglegY < p.y)
				{
					leaderOrigin.x = p.x;
					leaderOrigin.y = p.y - PPSAreaRad;
				}
				else
				{
					leaderOrigin.x = p.x;
					leaderOrigin.y = p.y + PPSAreaRad;
				}
			} // same as directly above or below
		}
		else
		{
			doglegX = p.x; // if directly on top or below
			doglegY = p.y + tagOffsetY + 7;

			if (doglegY < p.y)
			{
				leaderOrigin.x = p.x;
				leaderOrigin.y = p.y - PPSAreaRad;
			}
			else
			{
				leaderOrigin.x = p.x;
				leaderOrigin.y = p.y + PPSAreaRad;
			}
		}

		if ((int)doglegY == p.y)
		{

			//doglegX = p.x + tagOffsetX;
			if (tagOffsetX > 0)
			{
				leaderOrigin.x = p.x + PPSAreaRad;
				leaderOrigin.y = p.y;
				doglegX = leaderOrigin.x;
			}
			else
			{
				leaderOrigin.x = p.x - PPSAreaRad;
				leaderOrigin.y = p.y;
				doglegX = leaderOrigin.x;
			}
		}

		// draw extension if tag is to the left of the PPS
		if (rline1.right < (int)doglegX)
		{
			HPEN targetPen;
			COLORREF conColor = CACTag::ColorPicker(rt, blinking);
			targetPen = CreatePen(PS_SOLID, 1, conColor);
			dc->SelectObject(targetPen);

			
			dc->MoveTo(rline1.right + 5, rline1.top + 7);
			if (CSiTRadar::menuState.bigACID) {
				dc->MoveTo(rline1.right + 5, rline1.top + 9);
			}
			dc->LineTo((int)doglegX, (int)doglegY);

			DeleteObject(targetPen);
		}

		// Draw the angled line and draw the horizontal line
		HPEN targetPen;
		COLORREF conColor = CACTag::ColorPicker(rt, blinking);

		targetPen = CreatePen(PS_SOLID, 1, conColor);
		dc->SelectObject(targetPen);
		dc->SelectStockObject(NULL_BRUSH);

		dc->MoveTo(leaderOrigin.x, leaderOrigin.y);
		dc->LineTo((int)doglegX, (int)doglegY);				// line to the dogleg
		dc->LineTo(connector.x, (int)p.y + tagOffsetY + 7); // line to the connector point

		// ADSB circle
		if (CSiTRadar::mAcData[rt->GetCallsign()].isADSB)
		{
			dc->Ellipse((int)doglegX - 3, (int)doglegY - 3, (int)doglegX + 4, (int)doglegY + 4);
		}

		DeleteObject(targetPen);
	}

	// Draw Connector Ends

	// BRAVO TAGS
	if (CSiTRadar::mAcData[rt->GetCallsign()].tagType == 0 && rt->GetPosition().GetRadarFlags() != 1)
	{

		RECT bline0{};
		RECT bline1{};
		RECT bline2{};
		RECT bline3{};

		dc->SetTextColor(CACTag::ColorPicker(rt, blinking));

		bline1.top = p.y - 7;
		bline1.left = p.x + 10;

		if (fp->GetFlightPlanData().GetCapibilities() == 'Q' || fp->GetFlightPlanData().GetCapibilities() == 'L' || fp->GetFlightPlanData().GetCapibilities() == 'R')
		{
			dc->SetTextColor(C_TAG_GREEN);
		}

		dc->DrawText(altThreeDigit.c_str(), &bline1, DT_LEFT | DT_CALCRECT);
		dc->DrawText(altThreeDigit.c_str(), &bline1, DT_LEFT);
		rad->AddScreenObject(TAG_ITEM_TYPE_ALTITUDE, rt->GetCallsign(), bline1, TRUE, "BRAVO ALT");

		dc->SetTextColor(CACTag::ColorPicker(rt, blinking));

		if (abs(rt->GetVerticalSpeed()) > 400)
		{
			bline1.left = bline1.right;
			dc->DrawText(vmi.c_str(), &bline1, DT_LEFT | DT_CALCRECT);
			dc->DrawText(vmi.c_str(), &bline1, DT_LEFT);

			bline1.left = bline1.right;
			dc->DrawText(vmr.c_str(), &bline1, DT_LEFT | DT_CALCRECT);
			dc->DrawText(vmr.c_str(), &bline1, DT_LEFT);
		}
		bline1.left = bline1.right + 5;

		bline3.top = bline1.bottom - 2;
		bline3.left = p.x + 38;
		if (isDest)
		{
			dc->SetTextColor(C_WHITE);
			dc->DrawText(destination.c_str(), &bline3, DT_LEFT | DT_CALCRECT);
			dc->DrawText(destination.c_str(), &bline3, DT_LEFT);
			rad->AddScreenObject(TAG_ITEM_TYPE_DESTINATION, rt->GetCallsign(), bline3, TRUE, "");
		}
	}

	// Uncorrelated
	if (CSiTRadar::mAcData[rt->GetCallsign()].tagType == 3 && rt->GetPosition().GetRadarFlags() != 1)
	{
		dc->SetTextColor(CACTag::ColorPicker(rt, blinking));

		RECT uline0{};
		RECT uline1{};
		RECT uline2{};
		RECT uline3{};

		uline0.top = p.y - 19;
		uline0.left = p.x + 10;
		if (CSiTRadar::halfSecTick && CSiTRadar::mAcData[rt->GetCallsign()].multipleDiscrete)
		{
			ssr = "";
		}
		dc->DrawText(ssr.c_str(), &uline0, DT_LEFT | DT_CALCRECT);
		dc->DrawText(ssr.c_str(), &uline0, DT_LEFT);
		rad->AddScreenObject(TAG_ITEM_TYPE_SQUAWK, rt->GetCallsign(), uline0, TRUE, "");

		uline1.top = p.y - 7;
		uline1.left = p.x + 10;
		dc->DrawText(altThreeDigit.c_str(), &uline1, DT_LEFT | DT_CALCRECT);
		dc->DrawText(altThreeDigit.c_str(), &uline1, DT_LEFT);
		rad->AddScreenObject(TAG_ITEM_TYPE_ALTITUDE, rt->GetCallsign(), uline1, TRUE, "Uncorr ALT");

		if (abs(rt->GetVerticalSpeed()) > 400)
		{
			uline1.left = uline1.right;
			dc->DrawText(vmi.c_str(), &uline1, DT_LEFT | DT_CALCRECT);
			dc->DrawText(vmi.c_str(), &uline1, DT_LEFT);

			uline1.left = uline1.right;
			dc->DrawText(vmr.c_str(), &uline1, DT_LEFT | DT_CALCRECT);
			dc->DrawText(vmr.c_str(), &uline1, DT_LEFT);
		}
		uline1.left = uline1.right + 5;
	}

	// cleanup
	DeleteObject(font);
	DeleteObject(boldfont);
	DeleteObject(bigACIDfont);

	// restore context
	dc->RestoreDC(sDC);
}

void CACTag::DrawNARDSTag(CDC *dc, CRadarScreen *rad, CRadarTarget *rt, CFlightPlan *fp, unordered_map<string, POINT> *tOffset)
{
	POINT p{0, 0};
	p = rad->ConvertCoordFromPositionToPixel(rt->GetPosition().GetPosition());
	int tagOffsetX = 0;
	int tagOffsetY = 0;

	bool blinking = FALSE;
	if (strcmp(fp->GetHandoffTargetControllerId(), rad->GetPlugIn()->ControllerMyself().GetPositionId()) == 0)
	{
		blinking = TRUE;
	}
	if (rt->GetPosition().GetTransponderI())
	{
		blinking = TRUE;
	}
	if (CSiTRadar::hoAcceptedTime.find(rt->GetCallsign()) != CSiTRadar::hoAcceptedTime.end())
	{
		blinking = TRUE;
	}

	// Line 0 Items
	string ssr = rt->GetPosition().GetSquawk();

	// Line 1 Items
	string cs = rt->GetCallsign();
	string wtSymbol = "";
	if (rad->GetPlugIn()->FlightPlanSelect(cs.c_str()).GetFlightPlanData().GetAircraftWtc() == 'H')
	{
		wtSymbol = "+";
	}
	if (rad->GetPlugIn()->FlightPlanSelect(cs.c_str()).GetFlightPlanData().GetAircraftWtc() == 'L')
	{
		wtSymbol = "-";
	}
	cs = cs + wtSymbol;

	char commTypeChar = tolower(fp->GetControllerAssignedData().GetCommunicationType());
	if (commTypeChar == '\0')
	{
		commTypeChar = tolower(fp->GetFlightPlanData().GetCommunicationType());
	}
	string commType = "";
	if (commTypeChar != 'v')
	{
		commType = "/";
		commType += commTypeChar;
	}

	string sfi = fp->GetControllerAssignedData().GetScratchPadString();

	// Line 2 Items
	string altThreeDigit;
	if (rt->GetPosition().GetPressureAltitude() > rad->GetPlugIn()->GetTransitionAltitude())
	{
		altThreeDigit = to_string((rt->GetPosition().GetFlightLevel() + 50) / 100); // +50 to force rounding up
	}
	else
	{
		altThreeDigit = to_string((rt->GetPosition().GetPressureAltitude() + 50) / 100);
	}
	if (altThreeDigit.size() <= 3)
	{
		altThreeDigit.insert(altThreeDigit.begin(), 3 - altThreeDigit.size(), '0');
	}
	string vmi;
	if (rt->GetVerticalSpeed() > 400)
	{
		vmi = "^";
	}
	if (rt->GetVerticalSpeed() < -400)
	{
		vmi = "|";
	}; // up arrow "??!" = downarrow

	string groundSpeed = to_string((rt->GetPosition().GetReportedGS() + 5) / 10);

	// Initiate the default tag location, if no location is set already or find it in the map

	if (tOffset->find(rt->GetCallsign()) == tOffset->end())
	{
		POINT pTag{20, -24};
		tOffset->insert(pair<string, POINT>(rt->GetCallsign(), pTag));

		tagOffsetX = pTag.x;
		tagOffsetY = pTag.y;
	}
	else
	{
		POINT pTag = tOffset->find(rt->GetCallsign())->second;

		tagOffsetX = pTag.x;
		tagOffsetY = pTag.y;
	}

	POINT line0 = {p.x + tagOffsetX, p.y + tagOffsetY - 11};
	POINT line1 = {p.x + tagOffsetX, p.y + tagOffsetY};
	POINT line2 = {p.x + tagOffsetX, p.y + tagOffsetY + 11};
	POINT line3 = {p.x + tagOffsetX, p.y + tagOffsetY + 22};
	POINT line4 = {p.x + tagOffsetX, p.y + tagOffsetY + 33};

	// save context
	int sDC = dc->SaveDC();

	CFont font;
	CFont boldfont;
	LOGFONT lgfont;
	std::memset(&lgfont, 0, sizeof(LOGFONT));
	lgfont.lfHeight = 14;
	lgfont.lfWeight = 500;
	strcpy_s(lgfont.lfFaceName, _T("EuroScope"));
	font.CreateFontIndirect(&lgfont);
	lgfont.lfWeight = 1200;
	boldfont.CreateFontIndirect(&lgfont);
	dc->SelectObject(font);

	// Draw Connector

	int doglegX = 0;
	int doglegY = 0;

	if (CSiTRadar::mAcData[rt->GetCallsign()].tagType == 1 ||
		CSiTRadar::mAcData[rt->GetCallsign()].tagType == 2)
	{

		POINT connector{0, 0};
		int tagOffsetX = 0;
		int tagOffsetY = 0;

		// get the tag off set from the TagOffset<map>
		POINT pTag = tOffset->find(rt->GetCallsign())->second;

		tagOffsetX = pTag.x;
		tagOffsetY = pTag.y;

		bool blinking = FALSE;
		if (fp->GetHandoffTargetControllerId() == rad->GetPlugIn()->ControllerMyself().GetPositionId() && strcmp(fp->GetHandoffTargetControllerId(), "") != 0)
		{
			blinking = TRUE;
		}
		if (rt->GetPosition().GetTransponderI())
		{
			blinking = TRUE;
		}

		if (rt->IsValid())
		{
			p = rad->ConvertCoordFromPositionToPixel(rt->GetPosition().GetPosition());
		}

		// determine if the tag is to the left or the right of the PPS

		if (pTag.x >= 0)
		{
			connector.x = (int)p.x + tagOffsetX - 3;
		};
		if (pTag.x < 0)
		{
			connector.x = (int)p.x + tagOffsetX - 3 + CSiTRadar::mAcData[rt->GetCallsign()].tagWidth;
		}
		connector.y = p.y + tagOffsetY + 7;

		// the connector is only drawn at 30, 45 or 60 degrees, set the theta to the nearest appropriate angle
		// get the angle between the line between the PPS and connector and horizontal

		// if vertical (don't divide by 0!)
		double theta = 30;
		double phi = 0;

		if (connector.x - p.x != 0)
		{

			double x = abs(connector.x - p.x); // use absolute value since coord system is upside down
			double y = abs(p.y - connector.y); // also cast as double for atan

			phi = atan(y / x);

			// logic for if phi is a certain value; unit circle! (with fudge factor)
			if (phi >= 0 && phi < PI / 6)
			{
				theta = 30;
			}
			else if (phi >= PI / 6 && phi < PI / 4)
			{
				theta = 45;
			}
			else if (phi >= PI / 4 && phi < PI / 3)
			{
				theta = 60;
			}

			theta = theta * PI / 180;		// to radians
			doglegY = p.y + tagOffsetY + 7; // small padding to line it up with the middle of the first line

			// Calculate the x position of the intersection point (probably there is a more efficient way, but the atan drove me crazy
			doglegX = (int)(p.x + ((double)(p.y - (double)connector.y) / tan(theta))); // quad 1

			if (connector.x < p.x)
			{
				doglegX = (int)(p.x - ((double)(p.y - (double)connector.y) / tan(theta)));
			} // quadrant 2
			if (connector.y > p.y && connector.x > p.x)
			{
				doglegX = (int)(p.x - ((double)(p.y - (double)connector.y) / tan(theta)));
			}
			if (connector.y > p.y && connector.x < p.x)
			{
				doglegX = (int)(p.x + ((double)(p.y - (double)connector.y) / tan(theta)));
			}
			if (phi >= PI / 3)
			{
				doglegX = p.x;
			} // same as directly above or below
		}
		else
		{
			doglegX = p.x; // if direction on top or below
			doglegY = p.y + tagOffsetY + 7;
		}

		// Draw the angled line and draw the horizontal line
		HPEN targetPen;
		COLORREF conColor = C_PPS_YELLOW;
		if (CSiTRadar::halfSecTick == TRUE && blinking)
		{
			conColor = C_WHITE;
		}
		targetPen = CreatePen(PS_SOLID, 1, conColor);
		dc->SelectObject(targetPen);
		dc->SelectStockObject(NULL_BRUSH);

		dc->MoveTo(p.x, p.y);
		dc->LineTo((int)doglegX, (int)doglegY);				// line to the dogleg
		dc->LineTo(connector.x, (int)p.y + tagOffsetY + 7); // line to the connector point

		DeleteObject(targetPen);
	}

	// Draw Connector Ends

	if (CSiTRadar::mAcData[rt->GetCallsign()].tagType == 1 ||
		(CSiTRadar::mAcData[fp->GetCallsign()].isADSB && CSiTRadar::mAcData[fp->GetCallsign()].tagType == 1))
	{
		// Tag formatting
		RECT tagCallsign;
		tagCallsign.left = p.x + tagOffsetX;
		tagCallsign.top = p.y + tagOffsetY;

		dc->SetTextColor(C_PPS_YELLOW);
		if (blinking && CSiTRadar::halfSecTick)
		{
			dc->SetTextColor(C_WHITE);
		}

		// Line 0
		RECT rline0;
		rline0.top = line0.y;
		rline0.left = line0.x;
		rline0.bottom = line1.y;

		dc->DrawText(ssr.c_str(), &rline0, DT_LEFT | DT_CALCRECT);
		dc->DrawText(ssr.c_str(), &rline0, DT_LEFT);
		rad->AddScreenObject(TAG_ITEM_TYPE_SQUAWK, fp->GetCallsign(), rline0, TRUE, rt->GetPosition().GetSquawk());

		// Line 1
		RECT rline1;
		rline1.top = line1.y;
		rline1.left = line1.x;
		rline1.bottom = line2.y;

		if (CSiTRadar::mAcData[rt->GetCallsign()].isMedevac)
		{
			dc->SetTextColor(C_PPS_RED);
			if (blinking && CSiTRadar::halfSecTick)
			{
				dc->SetTextColor(C_WHITE);
			}

			dc->SelectObject(boldfont);

			dc->DrawText("+", &rline1, DT_LEFT | DT_CALCRECT);
			dc->DrawText("+", &rline1, DT_LEFT);
			dc->SetTextColor(C_PPS_YELLOW);

			dc->SelectObject(font);

			rline1.left = rline1.right;
			rline1.right = rline1.left;
		}

		dc->DrawText(cs.c_str(), &rline1, DT_LEFT | DT_CALCRECT);
		dc->DrawText(cs.c_str(), &rline1, DT_LEFT);
		rad->AddScreenObject(TAG_ITEM_TYPE_CALLSIGN, fp->GetCallsign(), rline1, TRUE, fp->GetCallsign());
		rline1.left = rline1.right;
		rline1.right = rline1.left + 8;

		// Show Communication Type if not Voice
		if (commType.size() > 0)
		{
			dc->DrawText(commType.c_str(), &rline1, DT_LEFT | DT_CALCRECT);
			dc->DrawText(commType.c_str(), &rline1, DT_LEFT);
		}
		rad->AddScreenObject(TAG_ITEM_TYPE_COMMUNICATION_TYPE, rt->GetCallsign(), rline1, TRUE, rt->GetCallsign());
		rline1.left = rline1.right;

		rad->AddScreenObject(CTR_DATA_TYPE_SCRATCH_PAD_STRING, rt->GetCallsign(), rline1, TRUE, rt->GetCallsign());

		// draw extension if tag is to the left of the PPS
		if (rline1.right < (int)doglegX)
		{
			HPEN targetPen;
			COLORREF conColor = C_PPS_YELLOW;
			if (CSiTRadar::halfSecTick == TRUE && blinking)
			{
				conColor = C_WHITE;
			}
			targetPen = CreatePen(PS_SOLID, 1, conColor);
			dc->SelectObject(targetPen);

			dc->MoveTo(rline1.right + 5, rline1.top + 7);
			if (CSiTRadar::menuState.bigACID) {
				dc->MoveTo(rline1.right + 5, rline1.top + 9);
			}
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

		if (abs(rt->GetVerticalSpeed()) > 400)
		{
			rline2.left = rline2.right;
			dc->DrawText(vmi.c_str(), &rline2, DT_LEFT | DT_CALCRECT);
			dc->DrawText(vmi.c_str(), &rline2, DT_LEFT);
		}
		rline2.left = rline2.right + 8;

		double alt;

		if (rt->GetPosition().GetPressureAltitude() > rad->GetPlugIn()->GetTransitionAltitude())
		{
			alt = rt->GetPosition().GetFlightLevel(); // +50 to force rounding up
		}
		else
		{
			alt = rt->GetPosition().GetPressureAltitude();
		}

		if (blinking && CSiTRadar::halfSecTick)
		{
			dc->SetTextColor(C_WHITE);
		}
		dc->DrawText(to_string(rt->GetPosition().GetReportedGS() / 10).c_str(), &rline2, DT_LEFT | DT_CALCRECT);
		dc->DrawText(to_string(rt->GetPosition().GetReportedGS() / 10).c_str(), &rline2, DT_LEFT);
		rad->AddScreenObject(TAG_ITEM_TYPE_GROUND_SPEED_WITH_N, fp->GetCallsign(), rline2, TRUE, "");
	}

	// restore context
	dc->RestoreDC(sDC);

	// cleanup
	DeleteObject(font);
	DeleteObject(boldfont);
}

void CACTag::DrawFPConnector(CDC *dc, CRadarScreen *rad, CRadarTarget *rt, CFlightPlan *fp, COLORREF color, unordered_map<string, POINT> *tOffset)
{

	// save context
	int sDC = dc->SaveDC();

	POINT p{0, 0};
	POINT connector{0, 0};
	int tagOffsetX = 0;
	int tagOffsetY = 0;

	// get the tag off set from the TagOffset<map>
	POINT pTag = tOffset->find(fp->GetCallsign())->second;

	tagOffsetX = pTag.x;
	tagOffsetY = pTag.y;

	if (fp->IsValid())
	{
		p = rad->ConvertCoordFromPositionToPixel(fp->GetFPTrackPosition().GetPosition());
	}
	else
	{
		if (rt->IsValid())
		{
			p = rad->ConvertCoordFromPositionToPixel(rt->GetPosition().GetPosition());
		}
	}

	// determine if the tag is to the left or the right of the PPS

	if (pTag.x >= 0)
	{
		connector.x = (int)p.x + tagOffsetX - 3;
	};
	if (pTag.x < 0)
	{
		connector.x = (int)p.x + tagOffsetX - 3 + TAG_WIDTH;
	}
	connector.y = p.y + tagOffsetY + 7;

	// the connector is only drawn at 30, 45 or 60 degrees, set the theta to the nearest appropriate angle
	// get the angle between the line between the PPS and connector and horizontal

	// if vertical (don't divide by 0!)
	double theta = 30;
	double phi = 0;

	int doglegX = 0;
	int doglegY = 0;

	if (connector.x - p.x != 0)
	{

		double x = abs(connector.x - p.x); // use absolute value since coord system is upside down
		double y = abs(p.y - connector.y); // also cast as double for atan

		phi = atan(y / x);

		// logic for if phi is a certain value; unit circle! (with fudge factor)
		if (phi >= 0 && phi < PI / 6)
		{
			theta = 30;
		}
		else if (phi >= PI / 6 && phi < PI / 4)
		{
			theta = 45;
		}
		else if (phi >= PI / 4 && phi < PI / 3)
		{
			theta = 60;
		}

		theta = theta * PI / 180;		// to radians
		doglegY = p.y + tagOffsetY + 7; // small padding to line it up with the middle of the first line

		// Calculate the x position of the intersection point (probably there is a more efficient way, but the atan drove me crazy
		doglegX = (int)(p.x + ((double)(p.y - (double)connector.y) / tan(theta))); // quad 1

		if (connector.x < p.x)
		{
			doglegX = (int)(p.x - ((double)(p.y - (double)connector.y) / tan(theta)));
		} // quadrant 2
		if (connector.y > p.y && connector.x > p.x)
		{
			doglegX = (int)(p.x - ((double)(p.y - (double)connector.y) / tan(theta)));
		}
		if (connector.y > p.y && connector.x < p.x)
		{
			doglegX = (int)(p.x + ((double)(p.y - (double)connector.y) / tan(theta)));
		}
		if (phi >= PI / 3)
		{
			doglegX = p.x;
		} // same as directly above or below
	}
	else
	{
		doglegX = p.x; // if direction on top or below
		doglegY = p.y + tagOffsetY + 7;
	}

	// Draw the angled line and draw the horizontal line
	HPEN targetPen;
	targetPen = CreatePen(PS_SOLID, 1, color);
	dc->SelectObject(targetPen);
	dc->SelectStockObject(NULL_BRUSH);

	dc->MoveTo(p.x, p.y);
	dc->LineTo((int)doglegX, (int)doglegY);				// line to the dogleg
	dc->LineTo(connector.x, (int)p.y + tagOffsetY + 7); // line to the connector point

	DeleteObject(targetPen);

	// restore
	dc->RestoreDC(sDC);
}

void CACTag::DrawRTConnector(CDC *dc, CRadarScreen *rad, CRadarTarget *rt, CFlightPlan *fp, COLORREF color, unordered_map<string, POINT> *tOffset)
{
}

void CACTag::DrawHistoryDots(CDC *dc, CRadarTarget *rt)
{
	int sDC = dc->SaveDC();

	CRadarTargetPositionData trailPt;
	POINT dot;
	trailPt = rt->GetPreviousPosition(rt->GetPosition());

	HPEN targetPen;
	COLORREF ppsColor = C_PPS_YELLOW;
	if (!strcmp(rt->GetCorrelatedFlightPlan().GetFlightPlanData().GetPlanType(), "V"))
	{
		ppsColor = C_PPS_ORANGE;
	}

	targetPen = CreatePen(PS_SOLID, 1, ppsColor);
	dc->SelectObject(targetPen);

	for (int i = 0; i < CSiTRadar::menuState.numHistoryDots; i++)
	{
		dot = CSiTRadar::m_pRadScr->ConvertCoordFromPositionToPixel(trailPt.GetPosition());
		RECT r = {dot.x - 1, dot.y - 1, dot.x + 1, dot.y + 1};
		dc->Ellipse(&r);
		trailPt = rt->GetPreviousPosition(trailPt);
	}

	DeleteObject(targetPen);
	dc->RestoreDC(sDC);
}

void CACTag::DrawHistoryDots(CDC *dc, CFlightPlan *fp)
{
	int sDC = dc->SaveDC();

	HPEN targetPen;
	COLORREF ppsColor = C_PPS_ORANGE;
	POINT dot;

	targetPen = CreatePen(PS_SOLID, 1, ppsColor);
	dc->SelectObject(targetPen);

	for (auto &pos : CSiTRadar::mAcData[fp->GetCallsign()].prevPosition)
	{
		dot = CSiTRadar::m_pRadScr->ConvertCoordFromPositionToPixel(pos);
		RECT r = {dot.x - 1, dot.y - 1, dot.x + 1, dot.y + 1};
		dc->Ellipse(&r);
	}

	DeleteObject(targetPen);
	dc->RestoreDC(sDC);
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