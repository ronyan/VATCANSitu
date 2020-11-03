#include "pch.h"
#include "PPS.h"

void CPPS::DrawPPS(CDC* dc, CRadarScreen& radScr, CRadarTarget& radTar)
{
	bool halfSecTick = FALSE; 
	clock_t tic = clock();
	double time = ((double)clock() - (double)tic) / ((double)CLOCKS_PER_SEC);
	if (time >= 0.5) {
		tic = clock();
		halfSecTick = !halfSecTick;
	}
	
	int sDC = dc->SaveDC();

	POINT p = ConvertCoordFromPositionToPixel(radTar.GetPosition().GetPosition());

	COLORREF targetPenColor;
	HPEN targetPen;

	// logic for the color of the PPS
	if (radTar.GetPosition().GetRadarFlags() == 0) { targetPenColor = C_WHITE; }
	else if (radTar.GetPosition().GetRadarFlags() == 1 && !radTar.GetCorrelatedFlightPlan().IsValid()) { targetPenColor = C_PPS_MAGENTA; }
	else if (radTar.GetPosition().GetSquawk() == "7500" || radTar.GetPosition().GetSquawk() == "7600") { targetPenColor = C_PPS_RED; }
	else { targetPenColor = C_PPS_YELLOW; }

	// if squawking ident, the colour alternates to white
	if (radTar.GetPosition().GetTransponderI()) { 
		if (halfSecTick) { targetPenColor = C_WHITE; }
	}

	// create the pen
	targetPen = CreatePen(PS_SOLID, 1, targetPenColor);
	dc->SelectObject(targetPen);
	dc->SelectStockObject(NULL_BRUSH);

	// Special Codes
	

	// else if no radar returns -> is it ADSB?


	// else handle the other radar flags (

	/*
	Symbol for uncorrelated primary only

	dc->MoveTo(p.x, p.y + 4);
	dc->LineTo(p.x, p.y);
	dc->LineTo(p.x - 4, p.y - 4);
	dc->MoveTo(p.x, p.y);
	dc->LineTo(p.x + 4, p.y - 4);

	*/

	// Add the screenobject

	// Draw halo

	// Draw the PTL


	dc->RestoreDC(sDC);
}

