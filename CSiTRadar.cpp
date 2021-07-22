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
#include <future>

using namespace Gdiplus;

// Initialize Static Members
unordered_map<string, ACData> CSiTRadar::mAcData;
unordered_map<string, int> CSiTRadar::tempTagData;
map<string, menuButton> TopMenu::menuButtons;
unordered_map<string, clock_t> CSiTRadar::hoAcceptedTime;
map<string, bool> CSiTRadar::destAirportList;
buttonStates CSiTRadar::menuState = {};
bool CSiTRadar::halfSecTick = FALSE;
CRadarScreen* CSiTRadar::m_pRadScr;

CSiTRadar::CSiTRadar()
{
	m_pRadScr = this;

	halfSec = clock();
	halfSecTick = FALSE;

	menuState.haloRad = 5;
	menuState.ptlTool = FALSE;
	menuLayer = 0;

	// load settings file
	try {

		std::ifstream settings_file(".\\situWx\\settings.json");
		if (settings_file.is_open()) {
			json j = json::parse(settings_file);

			wxRadar::wxLatCtr = j["wxlat"];
			wxRadar::wxLongCtr = j["wxlong"];
		}
		else {
			std::ofstream settings_file(".\\situWx\\settings.json");

			json j;
			j["wxlat"] = wxRadar::wxLatCtr;
			j["wxlong"] = wxRadar::wxLongCtr;

			settings_file << j;
		}
	}
	catch (std::ifstream::failure e) {

	};


	try {
		std::future<void> fa = std::async(std::launch::async, wxRadar::GetRainViewerJSON, this);
		std::future<void> fb = std::async(std::launch::async, wxRadar::parseRadarPNG, this);
		lastWxRefresh = clock();
	}
	catch (...) {
		GetPlugIn()->DisplayUserMessage("VATCAN Situ", "WX Parser", string("PNG Failed to Parse").c_str(), true, false, false, false, false);
	}

	CSiTRadar::mAcData.reserve(64);

	time = clock();
	oldTime = clock();
}

CSiTRadar::~CSiTRadar()
{
}

void CSiTRadar::OnRefresh(HDC hdc, int phase)
{
	if (m_pRadScr != this) {
		m_pRadScr = this;
	}
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

	if (((clock() - lastWxRefresh) / CLOCKS_PER_SEC) > 600 && (menuState.wxAll || menuState.wxHigh)) {

		// autorefresh weather download every 10 minutes
		std::future<void> fb = std::async(std::launch::async, wxRadar::parseRadarPNG, this);
		lastWxRefresh = clock();
	}

	if (((clock() - menuState.handoffModeStartTime) / CLOCKS_PER_SEC) > 10 && menuState.handoffMode) {
		menuState.handoffMode = FALSE;
	}

	// set up the drawing renderer
	CDC dc;
	dc.Attach(hdc);

	Graphics g(hdc);

	double pixnm = PixelsPerNM();

	// Check if ASR is an IFR file
	if (GetDataFromAsr("DisplayTypeName") != NULL) {
		string DisplayType = GetDataFromAsr("DisplayTypeName");

		// VFR asrs should show the NARDS displays
		if (strcmp(DisplayType.c_str(), "VFR") == 0) {
			if (phase == REFRESH_PHASE_AFTER_TAGS) {

				for (CRadarTarget radarTarget = GetPlugIn()->RadarTargetSelectFirst(); radarTarget.IsValid();
					radarTarget = GetPlugIn()->RadarTargetSelectNext(radarTarget))
				{
					
					string callSign = radarTarget.GetCallsign();
					CSiTRadar::mAcData[radarTarget.GetCallsign()].tagType = 1;
					bool isCorrelated = radarTarget.GetCorrelatedFlightPlan().IsValid();

					POINT p = ConvertCoordFromPositionToPixel(radarTarget.GetPosition().GetPosition());

					// Altitude filters
					if (!radarTarget.GetCorrelatedFlightPlan().GetTrackingControllerIsMe() || strcmp(radarTarget.GetCorrelatedFlightPlan().GetHandoffTargetControllerId(), GetPlugIn()->ControllerMyself().GetPositionId()) == 0) {
						if (altFilterOn && radarTarget.GetPosition().GetPressureAltitude() < altFilterLow * 100 && !menuState.filterBypassAll) {
							continue;
						}

						if (altFilterOn && altFilterHigh > 0 && radarTarget.GetPosition().GetPressureAltitude() > altFilterHigh * 100 && !menuState.filterBypassAll) {
							continue;
						}
					}

					// draw PPS
					bool isVFR = mAcData[callSign].hasVFRFP;
					bool isRVSM = mAcData[callSign].isRVSM;
					bool isADSB = mAcData[callSign].isADSB;

					if ((!isCorrelated && !isADSB) || (radarTarget.GetPosition().GetRadarFlags() != 0 && isADSB && !isCorrelated)) {
						mAcData[callSign].tagType = 3; // sets this if RT is uncorr
					}
					else if (isCorrelated && mAcData[callSign].tagType == 3) { mAcData[callSign].tagType = 0; } // only sets once to go from uncorr to corr
					// then allows it to be opened closed etc

					COLORREF ppsColor;

					// logic for the color of the PPS
					if (radarTarget.GetPosition().GetRadarFlags() == 0) { ppsColor = C_PPS_YELLOW; }
					else if (radarTarget.GetPosition().GetRadarFlags() == 1 && !isCorrelated) { ppsColor = C_PPS_MAGENTA; }
					else if (!strcmp(radarTarget.GetPosition().GetSquawk(), "7600") || !strcmp(radarTarget.GetPosition().GetSquawk(), "7700")) { ppsColor = C_PPS_RED; }
					else if (isVFR && isCorrelated) { ppsColor = C_PPS_ORANGE; }
					else { ppsColor = C_PPS_YELLOW; }

					if (radarTarget.GetPosition().GetTransponderI() == TRUE && halfSecTick) { ppsColor = C_WHITE; }

					RECT prect = CPPS::DrawPPS(&dc, isCorrelated, isVFR, isADSB, isRVSM, radarTarget.GetPosition().GetRadarFlags(), ppsColor, radarTarget.GetPosition().GetSquawk(), p);
					AddScreenObject(AIRCRAFT_SYMBOL, callSign.c_str(), prect, FALSE, "");

					// display CJS
					if ((radarTarget.GetPosition().GetRadarFlags() >= 2 && isCorrelated) || CSiTRadar::mAcData[radarTarget.GetCallsign()].isADSB) {
						string CJS = GetPlugIn()->FlightPlanSelect(callSign.c_str()).GetTrackingControllerId();

						CFont font;
						LOGFONT lgfont;
						COLORREF cjsColor = C_PPS_YELLOW;

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

					if (radarTarget.GetPosition().GetRadarFlags() != 0) {
						CACTag::DrawNARDSTag(&dc, this, &radarTarget, &radarTarget.GetCorrelatedFlightPlan(), &rtagOffset);
					}
				}

				// NARDS Menu
			}
		}


		// IFR asrs should display the CAATS QAB
		if (strcmp(DisplayType.c_str(), "IFR") == 0) {

			if (phase == REFRESH_PHASE_AFTER_TAGS) {

				// Draw the mouse halo before menu, so it goes behind it
				if (mousehalo == TRUE) {
					HaloTool::drawHalo(&dc, p, menuState.haloRad, pixnm);
					RequestRefresh();
				}

				for (CRadarTarget radarTarget = GetPlugIn()->RadarTargetSelectFirst(); radarTarget.IsValid();
					radarTarget = GetPlugIn()->RadarTargetSelectNext(radarTarget))
				{

					if (menuState.filterBypassAll) {
						mAcData[radarTarget.GetCallsign()].tagType = 1;
					}

					string callSign = radarTarget.GetCallsign();
					// altitude filtering 

					if (!radarTarget.GetCorrelatedFlightPlan().GetTrackingControllerIsMe() ||  // Do not filter aircraft being tracked by me
						strcmp(radarTarget.GetCorrelatedFlightPlan().GetHandoffTargetControllerId(), GetPlugIn()->ControllerMyself().GetPositionId()) == 0 || // Do not filter aircraft being handed off to me
						CSiTRadar::destAirportList.find(radarTarget.GetCorrelatedFlightPlan().GetFlightPlanData().GetDestination()) != CSiTRadar::destAirportList.end() // Do not filter aircraft on Dest Aiport List
						) {
						if (altFilterOn && radarTarget.GetPosition().GetPressureAltitude() < altFilterLow * 100 && !menuState.filterBypassAll) {
							continue;
						}

						if (altFilterOn && altFilterHigh > 0 && radarTarget.GetPosition().GetPressureAltitude() > altFilterHigh * 100 && !menuState.filterBypassAll) {
							continue;
						}
					}

					POINT p = ConvertCoordFromPositionToPixel(radarTarget.GetPosition().GetPosition());

					// Draw PTL
					if (hasPTL.find(radarTarget.GetCallsign()) != hasPTL.end()) {
						HaloTool::drawPTL(&dc, radarTarget, this, p, menuState.ptlLength);
					}
					else if (menuState.ptlAll && radarTarget.GetPosition().GetRadarFlags() != 0) {
						HaloTool::drawPTL(&dc, radarTarget, this, p, menuState.ptlLength);
					}

					// Get information about the Aircraft/Flightplan
					bool isCorrelated = radarTarget.GetCorrelatedFlightPlan().IsValid();
					bool isVFR = mAcData[callSign].hasVFRFP;
					bool isRVSM = mAcData[callSign].isRVSM;
					bool isADSB = mAcData[callSign].isADSB;

					if ((!isCorrelated && !isADSB) || (radarTarget.GetPosition().GetRadarFlags() != 0 && isADSB && !isCorrelated)) {
						mAcData[callSign].tagType = 3; // sets this if RT is uncorr
					}
					else if (isCorrelated && mAcData[callSign].tagType == 3) { mAcData[callSign].tagType = 0; } // only sets once to go from uncorr to corr
					// then allows it to be opened closed etc

					COLORREF ppsColor;

					// logic for the color of the PPS
					if (radarTarget.GetPosition().GetRadarFlags() == 0) { ppsColor = C_PPS_YELLOW; }
					else if (radarTarget.GetPosition().GetRadarFlags() == 1 && !isCorrelated) { ppsColor = C_PPS_MAGENTA; }
					else if (!strcmp(radarTarget.GetPosition().GetSquawk(), "7600") || !strcmp(radarTarget.GetPosition().GetSquawk(), "7700")) { ppsColor = C_PPS_RED; }
					else if (isVFR && isCorrelated) { ppsColor = C_PPS_ORANGE; }
					else { ppsColor = C_PPS_YELLOW; }

					if (radarTarget.GetPosition().GetTransponderI() == TRUE && halfSecTick) { ppsColor = C_WHITE; }

					RECT prect = CPPS::DrawPPS(&dc, isCorrelated, isVFR, isADSB, isRVSM, radarTarget.GetPosition().GetRadarFlags(), ppsColor, radarTarget.GetPosition().GetSquawk(), p);
					AddScreenObject(AIRCRAFT_SYMBOL, callSign.c_str(), prect, FALSE, "");

					if (radarTarget.GetPosition().GetRadarFlags() != 0) {
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
					if (hoAcceptedTime.find(callSign) != hoAcceptedTime.end() && (clock() - hoAcceptedTime[callSign]) / CLOCKS_PER_SEC > 12) {
						hoAcceptedTime.erase(callSign);

						// if quick look is open, defer closing the tag until quicklook is off;
						if (!menuState.filterBypassAll) {
							mAcData[callSign].tagType = 0;
						}
						if (menuState.filterBypassAll) {
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

					// show CJS for controller tracking aircraft // or if in handoff mode, show the target controller's CJS
					if ((radarTarget.GetPosition().GetRadarFlags() >= 2 && isCorrelated) || CSiTRadar::mAcData[radarTarget.GetCallsign()].isADSB) {

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

						string CJS = GetPlugIn()->FlightPlanSelect(callSign.c_str()).GetTrackingControllerId();

						if (menuState.handoffMode &&
							strcmp(radarTarget.GetCallsign(), GetPlugIn()->FlightPlanSelectASEL().GetCallsign()) == 0) {
							CJS = GetPlugIn()->ControllerSelect(GetPlugIn()->FlightPlanSelect(callSign.c_str()).GetCoordinatedNextController()).GetPositionId();
							dc.SetTextColor(C_WHITE);
						}

						dc.DrawText(CJS.c_str(), &rectCJS, DT_LEFT);

						DeleteObject(font);
					}

					// plane halo looks at the <map> hashalo to see if callsign has a halo, if so, draws halo
					if (hashalo.find(radarTarget.GetCallsign()) != hashalo.end()) {
						HaloTool::drawHalo(&dc, p, menuState.haloRad, pixnm);
					}


					// Draw the Selected Aircraft Box
					if (CSiTRadar::menuState.handoffMode) {
						if (strcmp(radarTarget.GetCallsign(), GetPlugIn()->FlightPlanSelectASEL().GetCallsign()) == 0) {
							HPEN targetPen;
							RECT selectBox{};
							selectBox.left = p.x - 6;
							selectBox.right = p.x + 7;
							selectBox.top = p.y - 6;
							selectBox.bottom = p.y + 7;

							targetPen = CreatePen(PS_SOLID, 1, C_WHITE);
							dc.SelectObject(targetPen);
							dc.SelectStockObject(NULL_BRUSH);

							dc.Rectangle(&selectBox);

							CFont font;
							LOGFONT lgfont;

							memset(&lgfont, 0, sizeof(LOGFONT));
							lgfont.lfWeight = 500;
							strcpy_s(lgfont.lfFaceName, _T("EuroScope"));
							lgfont.lfHeight = 12;
							font.CreateFontIndirect(&lgfont);

							dc.SelectObject(font);
							dc.SetTextColor(C_WHITE);

							RECT rectHO;
							rectHO.left = p.x - 16;
							rectHO.right = p.x + 20;
							rectHO.top = p.y + 8;
							rectHO.bottom = p.y + 28;

							dc.DrawText("H/O", &rectHO, DT_CENTER);

							DeleteObject(font);
							DeleteObject(targetPen);
						}
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
						gCont = g.BeginContainer();

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
				menutopleft.x += 6;

				if (menuLayer == 0) {

					POINT modOrigin;
					modOrigin.x = 8;

					string numCJSAC = to_string(menuState.numJurisdictionAC);
					TopMenu::MakeText(dc, { modOrigin.x, radarea.top + 14 }, 42, 15, numCJSAC.c_str());

					menuButton but_tagHL = { { modOrigin.x+45, radarea.top + 6 }, "Tag HL", 40, 23, C_MENU_GREY3, C_MENU_GREY2, C_MENU_TEXT_WHITE, 0 };
					but = TopMenu::DrawBut(&dc, but_tagHL);
					ButtonToScreen(this, but, "Tag HL", 0);

					menuButton but_setup = { { modOrigin.x + 85, radarea.top + 6 }, "Setup", 40, 23, C_MENU_GREY3, C_MENU_GREY2, C_MENU_TEXT_WHITE, 0 };
					but = TopMenu::DrawBut(&dc, but_setup);
					ButtonToScreen(this, but, "Setup", 0);

					menuButton but_aircraftState = { { modOrigin.x + 125, radarea.top + 6 }, "Aircraft State", 70, 23, C_MENU_GREY3, C_MENU_GREY2, C_MENU_TEXT_WHITE, 0 };
					but = TopMenu::DrawBut(&dc, but_aircraftState);
					ButtonToScreen(this, but, "Aircraft State", 0);

					menuButton but_SSR = { { modOrigin.x + 195, radarea.top + 6 }, "SSR:", 80, 23, C_MENU_GREY3, C_MENU_GREY2, C_MENU_TEXT_WHITE, 0 };
					but = TopMenu::DrawBut(&dc, but_SSR);
					ButtonToScreen(this, but, "SSR", 0);



					menuButton but_misc = { { modOrigin.x, radarea.top + 31 }, "Misc", 45, 23, C_MENU_GREY3, C_MENU_GREY2, C_MENU_GREY4, 0 };
					TopMenu::DrawBut(&dc, but_misc);

					menuButton but_areas = { { modOrigin.x + 45, radarea.top + 31 }, "Areas", 40, 23, C_MENU_GREY3, C_MENU_GREY2, C_MENU_TEXT_WHITE, 0 };
					but = TopMenu::DrawBut(&dc, but_areas);
					ButtonToScreen(this, but, "Areas", 0);

					menuButton but_tags = { { modOrigin.x + 85, radarea.top + 31 }, "Tags", 40, 23, C_MENU_GREY3, C_MENU_GREY2, C_MENU_TEXT_WHITE, 0 };
					but = TopMenu::DrawBut(&dc, but_tags);
					ButtonToScreen(this, but, "Tags", 0);


					menuButton but_flightPlan = { { modOrigin.x + 125, radarea.top + 31 }, "Flight Plan", 70, 23, C_MENU_GREY3, C_MENU_GREY2, C_MENU_TEXT_WHITE, 0 };
					but = TopMenu::DrawBut(&dc, but_flightPlan);
					ButtonToScreen(this, but, "Flight Plan", 0);

					menuButton but_destAirport = { { modOrigin.x + 195, radarea.top + 31 }, "Dest Airport", 80, 23, C_MENU_GREY3, C_MENU_GREY2, C_MENU_TEXT_WHITE, 0 };
					but = TopMenu::DrawBut(&dc, but_destAirport);
					ButtonToScreen(this, but, "Dest Airport", 0);

					menutopleft.x = 295;


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

					POINT downArrow[3] = {
						{-2,-2},
						{3,-2},
						{1, 3}
					};

					POINT downArrow1[3] = {
						{-2,-2},
						{3,-2},
						{1, 3}
					};

					POINT downArrow2[3] = {
						{-2,-2},
						{3,-2},
						{1, 3}
					};

					TopMenu::MakeDropDown(dc, menutopleft, 35, 15, rng.c_str());

					menuButton but_rngdropdown = { { menutopleft.x + 35, radarea.top + 38 }, "", 15, 15, C_MENU_GREY3, C_MENU_GREY2, C_MENU_GREY4, 0 };
					TopMenu::DrawBut(&dc, but_rngdropdown);
					TopMenu::DrawIconBut(&dc, but_rngdropdown, downArrow, 3);

					menutopleft.x += 65;
					menutopleft.y -= 32;

					menutopleft.y += 8;
					TopMenu::MakeText(dc, { menutopleft.x - 8, menutopleft.y-3 }, 35, 15, "Map");
					menutopleft.y += 18;

					TopMenu::MakeDropDown(dc, menutopleft, 75, 18, GetPlugIn()->ControllerMyself().GetSectorFileName());
					menuButton but_mapdropdown = { { menutopleft.x + 73, radarea.top + 32 }, "", 18, 18, C_MENU_GREY3, C_MENU_GREY2, C_MENU_GREY4, 0 };
					TopMenu::DrawBut(&dc, but_mapdropdown);
					TopMenu::DrawIconBut(&dc, but_mapdropdown, downArrow1, 3);

					menutopleft.y -= 26;
					menutopleft.x += 35;
					but = TopMenu::DrawButton(&dc, menutopleft, 55, 23, "Overlays", 1);

					menutopleft.x += 70;
					menutopleft.y += 8;
					menuButton but_preset = { {menutopleft.x, radarea.top + 6 }, "Preset", 45, 23, C_MENU_GREY3, C_MENU_GREY2, C_MENU_GREY4, 0 };
					TopMenu::DrawBut(&dc, but_preset);

					menutopleft.y += 18;
					TopMenu::MakeDropDown(dc, menutopleft, 73, 18, "preset");
					menuButton but_presetdropdown = { { menutopleft.x + 73, radarea.top + 32 }, "", 18, 18, C_MENU_GREY3, C_MENU_GREY2, C_MENU_GREY4, 0 };
					TopMenu::DrawBut(&dc, but_presetdropdown);
					TopMenu::DrawIconBut(&dc, but_presetdropdown, downArrow2, 3);

					menutopleft.y -= 26;
					menutopleft.x += 47;

					menuButton but_presetback = { {menutopleft.x, radarea.top + 6 }, "Back", 45, 23, C_MENU_GREY3, C_MENU_GREY2, C_MENU_GREY4, 0 };
					TopMenu::DrawBut(&dc, but_presetback);

					menutopleft.x += 58;

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
					menutopleft.x += 64;

					// separation tools
					string haloText = "Halo " + to_string(menuState.haloRad);
					but = TopMenu::DrawButton(&dc, menutopleft, 45, 23, haloText.c_str(), menuState.haloTool);
					ButtonToScreen(this, but, "Halo", BUTTON_MENU_HALO_TOOL);

					menutopleft.y = menutopleft.y + 25;
					string ptlText = "PTL " + to_string(menuState.ptlLength);
					but = TopMenu::DrawButton(&dc, menutopleft, 45, 23, ptlText.c_str(), CSiTRadar::menuState.ptlTool);
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

					menutopleft.y = menutopleft.y - 25;
					menutopleft.x = menutopleft.x + 52;
					TopMenu::DrawButton(&dc, menutopleft, 50, 23, "Airspace", 0);

					// get the controller position ID and display it (aesthetics :) )
					if (GetPlugIn()->ControllerMyself().IsValid())
					{
						controllerID = GetPlugIn()->ControllerMyself().GetPositionId();
					}

					menutopleft.x += 58;
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

					POINT targetModuleOrigin = { 980, radarea.top + 6 };

					menuButton but_psrpoor = { {targetModuleOrigin.x, radarea.top + 6 }, "", 30,23, C_MENU_GREY3, C_MENU_GREY2, C_MENU_GREY4, 0 };
					TopMenu::DrawBut(&dc, but_psrpoor);
					TopMenu::DrawIconBut(&dc, but_psrpoor, psrPoor, 13);

					menuButton but_ALL = { { targetModuleOrigin.x, radarea.top + 31 }, "ALL", 30, 23, C_MENU_GREY3, C_MENU_GREY2, C_MENU_TEXT_WHITE, menuState.filterBypassAll };
					but = TopMenu::DrawBut(&dc, but_ALL);
					ButtonToScreen(this, but, "Ovrd Filter ALL", BUTTON_MENU_OVRD_ALL);

					menuButton but_EXT = { { targetModuleOrigin.x + 30, radarea.top + 6 }, "Ext", 30, 23, C_MENU_GREY3, C_MENU_GREY2, C_MENU_TEXT_WHITE, menuState.extAltToggle };
					but = TopMenu::DrawBut(&dc, but_EXT);
					ButtonToScreen(this, but, "ExtAlt Toggle", BUTTON_MENU_EXT_ALT);

					menuButton but_EMode = { { targetModuleOrigin.x + 30, radarea.top + 31 }, "EMode", 60, 23, C_MENU_GREY3, C_MENU_GREY2, C_MENU_GREY4, 0 };
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

					menuButton but_FPE = { { targetModuleOrigin.x + 60, radarea.top + 6 }, "", 30, 23, C_MENU_GREY3, C_MENU_GREY2, C_MENU_TEXT_WHITE, menuState.showExtrapFP };
					but = TopMenu::DrawBut(&dc, but_FPE);
					TopMenu::DrawIconBut(&dc, but_FPE, plane, sizeof(plane) / sizeof(plane[0]));
					ButtonToScreen(this, but, "ExtrapolatedFP", BUTTON_MENU_EXTRAP_FP);
							
					menuButton but_autotag = { { targetModuleOrigin.x + 92, radarea.top + 6 }, "Auto Tag", 62, 23, C_MENU_GREY3, C_MENU_GREY2, C_MENU_GREY4, 0 };
					TopMenu::DrawBut(&dc, but_autotag);

					menuButton but_CRDA = { { targetModuleOrigin.x + 92, radarea.top + 31 }, "CRDA", 62, 23, C_MENU_GREY3, C_MENU_GREY2, C_MENU_GREY4, 0 };
					TopMenu::DrawBut(&dc, but_CRDA);

					menuButton but_ins1 = { { targetModuleOrigin.x + 162, radarea.top + 6 }, "Ins1", 30, 23, C_MENU_GREY3, C_MENU_GREY2, C_MENU_GREY4, 0 };
					TopMenu::DrawBut(&dc, but_ins1);

					menuButton but_ins2 = { { targetModuleOrigin.x + 162, radarea.top + 31 }, "Ins2", 30, 23, C_MENU_GREY3, C_MENU_GREY2, C_MENU_GREY4, 0 };
					TopMenu::DrawBut(&dc, but_ins2);

					modOrigin.x = 1180;

					menuButton but_highWx = { { modOrigin.x, radarea.top + 6 }, "High", 30, 23, C_MENU_GREY3, C_MENU_GREY2, C_MENU_TEXT_WHITE, menuState.wxHigh };
					but = TopMenu::DrawBut(&dc, but_highWx);
					ButtonToScreen(this, but, "WxHigh", BUTTON_MENU_WX_HIGH);

					menuButton but_allWx = { { modOrigin.x + 30, radarea.top + 6 }, "All", 30, 23, C_MENU_GREY3, C_MENU_GREY2, C_MENU_TEXT_WHITE, menuState.wxAll };
					but = TopMenu::DrawBut(&dc, but_allWx);
					ButtonToScreen(this, but, "WxAll", BUTTON_MENU_WX_ALL);

					menuButton but_topsWx = { { modOrigin.x, radarea.top + 31 }, "TOPS", 60, 23, C_MENU_GREY3, C_MENU_GREY2, C_MENU_GREY4, 0 };
					TopMenu::DrawBut(&dc, but_topsWx);

					POINT icon_bolt[7] = {
						{2,-5},
						{5,-5},
						{1, 0},
						{4,0},
						{-3,5},
						{0,0},
						{-2,0}
					};

					menuButton but_lightning = { { modOrigin.x+62, radarea.top + 6 }, "", 30, 23, C_MENU_GREY3, C_MENU_GREY2, C_MENU_GREY4, 0 };
					TopMenu::DrawBut(&dc, but_lightning);
					TopMenu::DrawIconBut(&dc, but_lightning, icon_bolt, 7);

					menuButton but_hist = { { modOrigin.x+62, radarea.top + 31 }, "Hist", 30, 23, C_MENU_GREY3, C_MENU_GREY2, C_MENU_GREY4, 0 };
					TopMenu::DrawBut(&dc, but_hist);

					menutopleft.x = menutopleft.x + 200;

				}

				else if (menuLayer == 1) {

				POINT elementOrigin = CPoint(radarea.left + 10, radarea.top + 6);
				RECT r;

				// Alt Filter Submenu
				if (altFilterOpts) {

					string altFilterLowFL = to_string(altFilterLow);
					if (altFilterLowFL.size() < 3) {
						altFilterLowFL.insert(altFilterLowFL.begin(), 3 - altFilterLowFL.size(), '0');
					}
					string altFilterHighFL = to_string(altFilterHigh);
					if (altFilterHighFL.size() < 3) {
						altFilterHighFL.insert(altFilterHighFL.begin(), 3 - altFilterHighFL.size(), '0');
					}


					r = TopMenu::DrawButton(&dc, elementOrigin, 50, 46, "Cancel", FALSE);
					AddScreenObject(BUTTON_MENU_ALT_FILT_OPT, "Cancel", r, 0, "");

					elementOrigin.x += 52;
					r = TopMenu::DrawButton(&dc, elementOrigin, 50, 46, "", FALSE);
					TopMenu::MakeText(dc, { elementOrigin.x, elementOrigin.y + 12 }, 50, 15, "Clear");
					TopMenu::MakeText(dc, { elementOrigin.x, elementOrigin.y + 22 }, 50, 15, "Filter");
					AddScreenObject(BUTTON_MENU_ALT_FILT_OPT, "Clear Filter", r, 0, "");

					elementOrigin.x += 52;
					r = TopMenu::DrawButton(&dc, elementOrigin, 50, 46, "OK", FALSE);
					AddScreenObject(BUTTON_MENU_ALT_FILT_OPT, "OK", r, 0, "");
					elementOrigin.x += 40;
					elementOrigin.y += 6;

					r = TopMenu::MakeText(dc, elementOrigin, 55, 15, "Top:");
					elementOrigin.x += 45;
					rHLim = TopMenu::MakeField(dc, elementOrigin, 25, 15, altFilterHighFL.c_str());
					AddScreenObject(BUTTON_MENU_ALT_FILT_OPT, "HLim", rHLim, 0, "");

					elementOrigin.x -= 45; elementOrigin.y += 20;

					TopMenu::MakeText(dc, elementOrigin, 55, 15, "Base:");
					elementOrigin.x += 45;
					rLLim = TopMenu::MakeField(dc, elementOrigin, 25, 15, altFilterLowFL.c_str());
					AddScreenObject(BUTTON_MENU_ALT_FILT_OPT, "LLim", rLLim, 0, "");

				}

				// Halo submenu

				if (menuState.haloTool) {

					r = TopMenu::DrawButton(&dc, elementOrigin, 70, 46, "Close", FALSE);
					AddScreenObject(BUTTON_MENU_HALO_CLOSE, "Close", r, 0, "");

					elementOrigin.x += 72;
					r = TopMenu::DrawButton(&dc, elementOrigin, 70, 46, "", FALSE);
					TopMenu::MakeText(dc, { elementOrigin.x, elementOrigin.y + 12 }, 70, 15, "Clear All");
					TopMenu::MakeText(dc, { elementOrigin.x, elementOrigin.y + 22 }, 70, 15, "Halos");
					AddScreenObject(BUTTON_MENU_HALO_CLEAR_ALL, "Clear All Halos", r, 0, "");

					elementOrigin.x += 72;
					TopMenu::MakeText(dc, elementOrigin, 220, 13, "Halo Radius - nm");

					// Halo options loop

					elementOrigin.y += 13;
					elementOrigin.x += 20;

					
					for (int idx = 0; idx < 6; idx++) {
						bool pressed = FALSE;
						if (haloOptions[idx] == menuState.haloRad) {
							pressed = TRUE;
						}
						RECT r = TopMenu::DrawButton(&dc, elementOrigin, 20, 15, to_string(haloOptions[idx]).c_str(), pressed);
						AddScreenObject(BUTTON_MENU_HALO_OPTIONS, to_string(haloOptions[idx]).c_str(), r, 0, "");
						elementOrigin.x += 22;
					}
					

					elementOrigin.y = radarea.top + 6;
					// End PTL options

					elementOrigin.x = 370;
					r = TopMenu::DrawButton(&dc, elementOrigin, 70, 46, "", mousehalo);
					TopMenu::MakeText(dc, { elementOrigin.x, elementOrigin.y + 12 }, 70, 15, "Display");
					TopMenu::MakeText(dc, { elementOrigin.x, elementOrigin.y + 22 }, 70, 15, "Halo Cursor");
					AddScreenObject(BUTTON_MENU_HALO_MOUSE, "Mouse", r, 0, "");

					elementOrigin.x = 460;
					TopMenu::MakeText(dc, elementOrigin, 200, 15, "Toggle Halo cursor ON - OFF.");
					elementOrigin.y += 16;
					TopMenu::MakeText(dc, elementOrigin, 200, 15, "Select a Halo Radius.");
					elementOrigin.y += 16;
					TopMenu::MakeText(dc, elementOrigin, 200, 15, "Click on target to toggle Halo ON - OFF.");
										
				}


				// PTL submenu
				if (menuState.ptlTool) {
					r = TopMenu::DrawButton(&dc, elementOrigin, 70, 46, "Close", FALSE);
					AddScreenObject(BUTTON_MENU_PTL_CLOSE, "Close", r, 0, "");

					
					elementOrigin.x += 72;
					r = TopMenu::DrawButton(&dc, elementOrigin, 70, 46, "", FALSE);
					TopMenu::MakeText(dc, { elementOrigin.x, elementOrigin.y + 12 }, 70, 15, "Clear All");
					TopMenu::MakeText(dc, { elementOrigin.x, elementOrigin.y + 22 }, 70, 15, "PTLs");
					AddScreenObject(BUTTON_MENU_PTL_CLEAR_ALL, "Clear All PTLs", r, 0, "");

					elementOrigin.x += 72;
					r = TopMenu::DrawButton(&dc, elementOrigin, 70, 46, "PTL All", menuState.ptlAll);
					AddScreenObject(BUTTON_MENU_PTL_ALL_ON, "PTL All on", r, 0, "");

					elementOrigin.x += 72;
					r = TopMenu::DrawButton(&dc, elementOrigin, 70, 23, "Uncorr", FALSE);

					r = TopMenu::DrawButton(&dc, { elementOrigin.x, elementOrigin.y + 23 }, 70, 23, "Timeout", FALSE);

					elementOrigin.x += 72;
					TopMenu::MakeText(dc, elementOrigin, 250, 15, "PTL Length - Minutes");

					// PTL options loop

					elementOrigin.y += 15;
					elementOrigin.x += 12;

					for (int idx = 0; idx < 20; idx++) {
						bool pressed = FALSE;
						if (ptlOptions[idx] == menuState.ptlLength) {
							pressed = TRUE;
						}
						if (idx == 10) {
							elementOrigin.y += 15;
							elementOrigin.x -= 220;
						}
						RECT r = TopMenu::DrawButton(&dc, elementOrigin, 20, 15, to_string(ptlOptions[idx]).c_str(), pressed);
						AddScreenObject(BUTTON_MENU_PTL_OPTIONS, to_string(ptlOptions[idx]).c_str(), r, 0, "");
						elementOrigin.x += 22;
					}

					elementOrigin.y = radarea.top + 6;
					// End PTL options

					elementOrigin.x = 540;
					r = TopMenu::DrawButton(&dc, elementOrigin, 35, 46, "WB", FALSE);
					elementOrigin.x = 577;
					r = TopMenu::DrawButton(&dc, elementOrigin, 35, 46, "EB", FALSE);

					elementOrigin.x = 610;
					TopMenu::MakeText(dc, elementOrigin, 150, 15, "Click on targets to toggle");
					elementOrigin.y += 16;
					TopMenu::MakeText(dc, elementOrigin, 150, 15, "PTL ON-OFF.");
					
				}

				// Rings submenu

				}
			}
		}
	}

	if (phase == REFRESH_PHASE_BACK_BITMAP) {
		if (menuState.wxAll || menuState.wxHigh) {
			
			std::future<int> wxImg = std::async(std::launch::async, wxRadar::renderRadar, &g, this, menuState.wxAll);
			// wxRadar::renderRadar( &g, this, menuState.wxAll);
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
		
		CRadarTarget rt = GetPlugIn()->RadarTargetSelect(sObjectId);
		string callsign = rt.GetCallsign();

		GetPlugIn()->SetASELAircraft(GetPlugIn()->FlightPlanSelect(sObjectId));

		if (Button == BUTTON_LEFT) {
			if (!menuState.haloTool && !menuState.ptlTool) {
				if (CSiTRadar::mAcData[sObjectId].tagType == 0) { CSiTRadar::mAcData[sObjectId].tagType = 1; }

				else if (CSiTRadar::mAcData[sObjectId].tagType == 1 &&
					!GetPlugIn()->FlightPlanSelect(sObjectId).GetTrackingControllerIsMe() &&
					!menuState.quickLook) {
					CSiTRadar::mAcData[sObjectId].tagType = 0;
				} // can't make bravo tag if you are tracking or if quick look is on
			}
			
			if (menuState.haloTool == TRUE) {

				if (hashalo.find(callsign) != hashalo.end()) {
					hashalo.erase(callsign);
				}
				else {
					hashalo[callsign] = TRUE;
				}
			}

			if (menuState.ptlTool == TRUE) {

				if (hasPTL.find(callsign) != hasPTL.end()) {
					hasPTL.erase(callsign);
				}
				else {
					hasPTL[callsign] = TRUE;
				}
			}
		}
	}
	
	if (ObjectType == BUTTON_MENU_HALO_TOOL) {
		menuState.haloTool = true;
		menuLayer = 1;
	}

	if (ObjectType == BUTTON_MENU_HALO_OPTIONS) {
		menuState.haloRad = stoi(sObjectId);
	}

	if (ObjectType == BUTTON_MENU_HALO_CLOSE) {
		menuState.haloTool = false;
		menuLayer = 0;
	}

	if (ObjectType == BUTTON_MENU_HALO_CLEAR_ALL) {
		hashalo.clear();
	}

	if (ObjectType == BUTTON_MENU_HALO_MOUSE) {
		mousehalo = !mousehalo;
	}

	if (ObjectType == BUTTON_MENU_PTL_OPTIONS) {
		menuState.ptlLength = stoi(sObjectId);
	}

	if (ObjectType == BUTTON_MENU_PTL_TOOL) {
		menuState.ptlTool = true;
		menuLayer = 1;
	}

	if (ObjectType == BUTTON_MENU_PTL_CLOSE) {
		menuState.ptlTool = false;
		menuLayer = 0;
	}

	if (ObjectType == BUTTON_MENU_PTL_CLEAR_ALL) { 
		hasPTL.clear();
		menuState.ptlAll = false;
	}

	if (ObjectType == BUTTON_MENU_PTL_ALL_ON) { menuState.ptlAll = !menuState.ptlAll; }

	if (ObjectType == BUTTON_MENU_EXTRAP_FP) {
		menuState.showExtrapFP = !menuState.showExtrapFP;
	}

	if (ObjectType == BUTTON_MENU_OVRD_ALL) {

		if (menuState.filterBypassAll == FALSE) {
			menuState.filterBypassAll = TRUE;

			for (auto& p : CSiTRadar::mAcData) {
				CSiTRadar::tempTagData[p.first] = p.second.tagType;
				// Do not open uncorrelated tags
				if (p.second.tagType == 0) {
					p.second.tagType = 1;
				}
			}

		}
		else if (menuState.filterBypassAll == TRUE) {

			for (auto& p : CSiTRadar::tempTagData) {
				// prevents closing of tags that became under your jurisdiction during quicklook
				if (!GetPlugIn()->FlightPlanSelect(p.first.c_str()).GetTrackingControllerIsMe()) {
					CSiTRadar::mAcData[p.first].tagType = p.second;
				}
			}

			tempTagData.clear();
			menuState.filterBypassAll = FALSE;
		}
	}

	if (ObjectType == BUTTON_MENU_EXT_ALT) {
		menuState.extAltToggle = !menuState.extAltToggle;
	}
	

	if (ObjectType == BUTTON_MENU_ALT_FILT_OPT) {
		if (!strcmp(sObjectId, "Alt Filt Opts")) { 
			altFilterOpts = !altFilterOpts;
			menuLayer = 1;
		}

		if (!strcmp(sObjectId, "Cancel")) {
			altFilterOpts = false;
			menuLayer = 0;
		}

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
		if (!strcmp(sObjectId, "OK")) {
			string s = to_string(altFilterHigh);
			SaveDataToAsr("altFilterHigh", "Alt Filter High Limit", s.c_str());
			s = to_string(altFilterLow);
			SaveDataToAsr("altFilterLow", "Alt Filter Low Limit", s.c_str());
			altFilterOpts = false;
			menuLayer = 0;
		}
	}

	if (ObjectType == BUTTON_MENU_ALT_FILT_ON) {
		altFilterOn = !altFilterOn;
	}

	if (ObjectType == BUTTON_MENU_QUICK_LOOK) {

	}

	if (ObjectType == BUTTON_MENU_WX_HIGH) {
		if (menuState.wxAll) { menuState.wxAll = false; }
		
		menuState.wxHigh = !menuState.wxHigh;
		RefreshMapContent();
		
		if (lastWxRefresh == 0 || (clock() - lastWxRefresh) / CLOCKS_PER_SEC > 600) {
			
			std::future<void> wxRend = std::async(std::launch::async, wxRadar::parseRadarPNG, this);
			lastWxRefresh = clock();
		}
	}

	if (ObjectType == BUTTON_MENU_WX_ALL) {
		if (menuState.wxHigh) { menuState.wxHigh = false; }
		
		menuState.wxAll = !menuState.wxAll;
		RefreshMapContent();

		if (lastWxRefresh == 0 || (clock() - lastWxRefresh) / CLOCKS_PER_SEC > 600) {
			std::future<void> wxRend = std::async(std::launch::async, wxRadar::parseRadarPNG, this);
			lastWxRefresh = clock();
		}
	}
	
	/* if (Button == BUTTON_MIDDLE) {
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

	} */

	// Handle tag functions when clicking on tags generated by the plugin
	
	if (ObjectType == TAG_ITEM_TYPE_CALLSIGN || ObjectType == TAG_ITEM_FP_CS) {
		GetPlugIn()->SetASELAircraft(GetPlugIn()->FlightPlanSelect(sObjectId)); // make sure aircraft is ASEL
		
		if (Button == BUTTON_LEFT) {
			if (mAcData[sObjectId].isHandoffToMe == TRUE) {
				GetPlugIn()->FlightPlanSelect(sObjectId).AcceptHandoff();
			}
			else {
				StartTagFunction(sObjectId, NULL, TAG_ITEM_TYPE_PLANE_TYPE, sObjectId, NULL, TAG_ITEM_FUNCTION_TOGGLE_ROUTE_DRAW, Pt, Area);
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
			
		}
		if (Button == BUTTON_RIGHT) {

			GetPlugIn()->SetASELAircraft(GetPlugIn()->FlightPlanSelect(sObjectId)); // make sure aircraft is ASEL
			
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
		if (Button == BUTTON_LEFT) {
			if(mAcData[sObjectId].destLabelType == 0) {
				mAcData[sObjectId].destLabelType = 1;
			}
			else if (mAcData[sObjectId].destLabelType == 1) {
				mAcData[sObjectId].destLabelType = 0;
			}
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
	int count = 0;
	for (CRadarTarget radarTarget = GetPlugIn()->RadarTargetSelectFirst(); radarTarget.IsValid();
		radarTarget = GetPlugIn()->RadarTargetSelectNext(radarTarget))
	{
		if (radarTarget.GetCorrelatedFlightPlan().GetTrackingControllerIsMe()) { count++; }

		// Maintain the aircrafts under controller CJS in hand-off priority order
		bool isInJurisdictionalList{};
		isInJurisdictionalList = find(CSiTRadar::menuState.jurisdictionalAC.begin(), CSiTRadar::menuState.jurisdictionalAC.end(), radarTarget.GetCallsign()) != CSiTRadar::menuState.jurisdictionalAC.end();

		if (!isInJurisdictionalList) {
			// all other aircrafts are at the back of the list if not already in list
			if (radarTarget.GetCorrelatedFlightPlan().GetTrackingControllerIsMe()) {
				CSiTRadar::menuState.jurisdictionalAC.push_back(radarTarget.GetCallsign());
			}
		}
		else if (isInJurisdictionalList) {
			// Handoffs initiated by controller take 2nd priority
			if (radarTarget.GetCorrelatedFlightPlan().GetHandoffTargetControllerCallsign() != "" && radarTarget.GetCorrelatedFlightPlan().GetTrackingControllerIsMe()) {
				CSiTRadar::menuState.jurisdictionalAC.erase(find(CSiTRadar::menuState.jurisdictionalAC.begin(), CSiTRadar::menuState.jurisdictionalAC.end(), radarTarget.GetCorrelatedFlightPlan().GetCallsign()));
				CSiTRadar::menuState.jurisdictionalAC.emplace_front(radarTarget.GetCorrelatedFlightPlan().GetCallsign());
			}
			// Handoffs to controller take top priority
			if (radarTarget.GetCorrelatedFlightPlan().GetHandoffTargetControllerCallsign() == CSiTRadar::GetPlugIn()->ControllerMyself().GetCallsign())
			{
				CSiTRadar::menuState.jurisdictionalAC.erase(find(CSiTRadar::menuState.jurisdictionalAC.begin(), CSiTRadar::menuState.jurisdictionalAC.end(), radarTarget.GetCorrelatedFlightPlan().GetCallsign()));
				CSiTRadar::menuState.jurisdictionalAC.emplace_front(radarTarget.GetCorrelatedFlightPlan().GetCallsign());
			}

			// Clean up jurisdictional list if no longer under CJS
			if (!radarTarget.GetCorrelatedFlightPlan().GetTrackingControllerIsMe()) {
				CSiTRadar::menuState.jurisdictionalAC.erase(find(CSiTRadar::menuState.jurisdictionalAC.begin(), CSiTRadar::menuState.jurisdictionalAC.end(), radarTarget.GetCorrelatedFlightPlan().GetCallsign()));
			}
		}


	}
	
	menuState.numJurisdictionAC = count;

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
	string remarks = FlightPlan.GetFlightPlanData().GetRemarks();
	
	string CJS = FlightPlan.GetTrackingControllerId();

	ACData acdata;
	acdata.hasVFRFP = isVFR;
	acdata.isADSB = isADSB;
	acdata.isRVSM = isRVSM;
	if (remarks.find("STS/MEDEVAC") != remarks.npos) { acdata.isMedevac = true; }
	if (remarks.find("STS/ADSB") != remarks.npos) { acdata.isADSB = true; }
	mAcData[callSign] = acdata;
}

void CSiTRadar::OnFlightPlanDisconnect(CFlightPlan FlightPlan) {
	string callSign = FlightPlan.GetCallsign();

	mAcData.erase(callSign);

}

void CSiTRadar::OnDoubleClickScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, int Button)
{
	
}

void CSiTRadar::OnAsrContentToBeSaved() {

}
