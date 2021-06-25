#pragma once

#include "EuroScopePlugIn.h"
#include "CSiTRadar.h"

using namespace std;
using namespace EuroScopePlugIn;

class CPPS :
    public EuroScopePlugIn::CRadarScreen
{

public:
    static RECT DrawPPS(CDC* dc, BOOL isCorrelated, BOOL isVFR, BOOL isADSB, BOOL isRVSM, int radFlag, COLORREF ppsColor, string squawk, POINT p)
	{

		int sDC = dc->SaveDC();

		// Add the screenobject
		RECT prect;
		prect.left = p.x - 5;
		prect.top = p.y - 5;
		prect.right = p.x + 5;
		prect.bottom = p.y + 5;

		HPEN targetPen;

		/// DRAWING SYMBOLS ///

		targetPen = CreatePen(PS_SOLID, 1, ppsColor);
		dc->SelectObject(targetPen);
		dc->SelectStockObject(NULL_BRUSH);

		if (radFlag == 0) {
			// Draw ADSB PPS symbology
			if (isADSB) {
				dc->MoveTo(p.x - 4, p.y - 4);
				dc->LineTo(p.x + 4, p.y - 4);
				dc->LineTo(p.x + 4, p.y + 4);
				dc->LineTo(p.x - 4, p.y + 4);
				dc->LineTo(p.x - 4, p.y - 4);

				// RVSM ADSB symbol
				if (isRVSM) {
					dc->MoveTo(p.x, p.y - 4);
					dc->LineTo(p.x, p.y + 4);
				}
			}
		}

		else {
			// Special Codes
			if (!strcmp(squawk.c_str(), "7600") || !strcmp(squawk.c_str(), "7700")) {
				HBRUSH targetBrush = CreateSolidBrush(ppsColor);
				dc->SelectObject(targetBrush);

				POINT vertices[] = { { p.x - 4, p.y + 4 } , { p.x, p.y - 4 } , { p.x + 4,p.y + 4 } }; // Red triangle
				dc->Polygon(vertices, 3);
				DeleteObject(targetBrush);
			}


			else if (!strcmp(squawk.c_str(), "1200") && !isCorrelated && radFlag != 1) { // Eventually change to block squawk codes
				HPEN targetPen1 = CreatePen(PS_SOLID, 1, C_PPS_YELLOW);
				dc->SelectStockObject(NULL_BRUSH);
				dc->SelectObject(targetPen1);

				POINT vertices[] = { { p.x - 4, p.y + 4 } , { p.x, p.y - 4 } , { p.x + 4,p.y + 4 } }; // Yellow Triangle
				dc->Polygon(vertices, 3);
				DeleteObject(targetPen1);
			}

			else if (radFlag == 1) {

				dc->MoveTo(p.x, p.y + 4);	// Magenta Y 
				dc->LineTo(p.x, p.y);
				dc->LineTo(p.x - 4, p.y - 4);
				dc->MoveTo(p.x, p.y);
				dc->LineTo(p.x + 4, p.y - 4);
			}
			// IFR correlated
			else if (radFlag == 2) {
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

					dc->MoveTo(p.x, p.y - 5);
					dc->LineTo(p.x, p.y + 5);

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
				if (!isCorrelated) {
					HPEN targetPen1 = CreatePen(PS_SOLID, 1, C_PPS_YELLOW);
					dc->SelectObject(targetPen1);

					dc->MoveTo(p.x - 4, p.y - 4);
					dc->LineTo(p.x + 5, p.y + 5);
					dc->MoveTo(p.x, p.y - 5);
					dc->LineTo(p.x, p.y +6 );
					dc->MoveTo(p.x + 4, p.y - 4);
					dc->LineTo(p.x - 5, p.y + 5);
					dc->MoveTo(p.x - 5, p.y);
					dc->LineTo(p.x + 6, p.y);

					DeleteObject(targetPen1);
				}

			}
			else if (radFlag >= 3 && radFlag <= 7) {
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
				if (!isCorrelated) {
					HPEN targetPen1 = CreatePen(PS_SOLID, 1, ppsColor);
					dc->SelectObject(targetPen1);

					dc->MoveTo(p.x - 4, p.y - 4);
					dc->LineTo(p.x + 5, p.y + 5);
					dc->MoveTo(p.x, p.y - 5);
					dc->LineTo(p.x, p.y + 6);
					dc->MoveTo(p.x + 4, p.y - 4);
					dc->LineTo(p.x - 5, p.y + 5);
					dc->MoveTo(p.x - 5, p.y);
					dc->LineTo(p.x + 6, p.y);

					DeleteObject(targetPen1);
				}
			}
		}

		DeleteObject(targetPen);

		/// END DRAWING SYMBOLS ///

		// Draw halo

		// Draw the PTL



		dc->RestoreDC(sDC);
		return prect;
	};

    static void DrawCJS(CDC* dc, POINT p, string cjsText, COLORREF cjsColor);
};

