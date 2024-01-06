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

		switch (radFlag) {
		case 0:
			// Code for radFlag equals 0
			break;

		case 1:
			if (isCorrelated) {
				dc->SelectStockObject(NULL_BRUSH);

				POINT vertices[] = { { p.x - 4, p.y + 4 } , { p.x, p.y - 4 } , { p.x + 4,p.y + 4 } }; // Yellow Triangle
				dc->Polygon(vertices, 3);
			}
			else {
				dc->MoveTo(p.x, p.y + 4);	// Magenta Y 
				dc->LineTo(p.x, p.y);
				dc->LineTo(p.x - 4, p.y - 4);
				dc->MoveTo(p.x, p.y);
				dc->LineTo(p.x + 4, p.y - 4);
			}
			break;

		case 2:												// Code for radFlag equals 2 = SSR ONLY
		case 3: {

			if (!strcmp(squawk.c_str(), "7600") || !strcmp(squawk.c_str(), "7700")) {
				HBRUSH targetBrush = CreateSolidBrush(ppsColor);
				dc->SelectObject(targetBrush);

				POINT vertices[] = { { p.x - 4, p.y + 4 } , { p.x, p.y - 4 } , { p.x + 4,p.y + 4 } }; // Red triangle
				dc->Polygon(vertices, 3);
				DeleteObject(targetBrush);

				break;
			}

			if (!strcmp(squawk.c_str(), "1200") && !isCorrelated && radFlag != 1) { // Eventually change to block squawk codes

				dc->SelectStockObject(NULL_BRUSH);

				POINT vertices[] = { { p.x - 4, p.y + 4 } , { p.x, p.y - 4 } , { p.x + 4,p.y + 4 } }; // Yellow Triangle
				dc->Polygon(vertices, 3);

				break;
			}

			if (isCorrelated && !isVFR && !isRVSM) {		// Code for radFlag equals 3 = SSR+PSR
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

				dc->MoveTo(p.x - 3, p.y - 2);
				dc->LineTo(p.x + 1, p.y + 4);
				dc->LineTo(p.x + 4, p.y - 2);
			}
			if (!isCorrelated) {

				dc->MoveTo(p.x - 4, p.y - 4);
				dc->LineTo(p.x + 5, p.y + 5);
				dc->MoveTo(p.x, p.y - 5);
				dc->LineTo(p.x, p.y + 6);
				dc->MoveTo(p.x + 4, p.y - 4);
				dc->LineTo(p.x - 5, p.y + 5);
				dc->MoveTo(p.x - 5, p.y);
				dc->LineTo(p.x + 6, p.y);

			}
			break;
		}

		case 4: {
			
			if(!isADSB) { break; } 
			
			else {

				if(isCorrelated) {
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

				else {
					// ADSB non-correlated synmbol
		
					dc->MoveTo(p.x - 4, p.y - 4);
					dc->LineTo(p.x + 4, p.y - 4);
					dc->LineTo(p.x + 4, p.y + 4);
					dc->LineTo(p.x - 4, p.y + 4);
					dc->LineTo(p.x - 4, p.y - 4);
					dc->MoveTo(p.x - 6, p.y - 6);
					dc->LineTo(p.x - 1, p.y - 1);
					dc->MoveTo(p.x + 6, p.y - 6);
					dc->LineTo(p.x + 1, p.y - 1);
					dc->MoveTo(p.x - 6, p.y + 6);
					dc->LineTo(p.x - 1, p.y + 1);
					dc->MoveTo(p.x + 6, p.y + 6);
					dc->LineTo(p.x + 1, p.y + 1);
		
					dc->MoveTo(p.x, p.y - 6);
					dc->LineTo(p.x, p.y - 1);
		
					dc->MoveTo(p.x + 6, p.y);
					dc->LineTo(p.x + 1, p.y);
		
					dc->MoveTo(p.x - 6, p.y);
					dc->LineTo(p.x - 1, p.y);
		
					dc->MoveTo(p.x, p.y + 6);
					dc->LineTo(p.x, p.y + 1);
	
				}
			}
		}
		case 5:
		case 6:
		case 7: {
			// Code for radFlag equals 4 = MODE C = ADSB

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

				// ADSB non-correlated

				if (!isCorrelated) {

					// ADSB non-correlated synmbol

					dc->MoveTo(p.x - 4, p.y - 4);
					dc->LineTo(p.x + 4, p.y - 4);
					dc->LineTo(p.x + 4, p.y + 4);
					dc->LineTo(p.x - 4, p.y + 4);
					dc->LineTo(p.x - 4, p.y - 4);
					dc->MoveTo(p.x - 6, p.y - 6);
					dc->LineTo(p.x - 1, p.y - 1);
					dc->MoveTo(p.x + 6, p.y - 6);
					dc->LineTo(p.x + 1, p.y - 1);
					dc->MoveTo(p.x - 6, p.y + 6);
					dc->LineTo(p.x - 1, p.y + 1);
					dc->MoveTo(p.x + 6, p.y + 6);
					dc->LineTo(p.x + 1, p.y + 1);

					dc->MoveTo(p.x, p.y - 6);
					dc->LineTo(p.x, p.y - 1);

					dc->MoveTo(p.x + 6, p.y);
					dc->LineTo(p.x + 1, p.y);

					dc->MoveTo(p.x - 6, p.y);
					dc->LineTo(p.x - 1, p.y);

					dc->MoveTo(p.x, p.y + 6);
					dc->LineTo(p.x, p.y + 1);

				}

			}
			else {
				// If not ADSB by equipment, treat like a RADFLAG 2 or 3
				if (isCorrelated && !isVFR && !isRVSM) {		// Code for radFlag equals 3 = SSR+PSR
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

					dc->MoveTo(p.x - 3, p.y - 2);
					dc->LineTo(p.x + 1, p.y + 4);
					dc->LineTo(p.x + 4, p.y - 2);
				}

				if (!isCorrelated) {

					dc->MoveTo(p.x - 4, p.y - 4);
					dc->LineTo(p.x + 5, p.y + 5);
					dc->MoveTo(p.x, p.y - 5);
					dc->LineTo(p.x, p.y + 6);
					dc->MoveTo(p.x + 4, p.y - 4);
					dc->LineTo(p.x - 5, p.y + 5);
					dc->MoveTo(p.x - 5, p.y);
					dc->LineTo(p.x + 6, p.y);

				}
			}

			break;
		}
		}

		DeleteObject(targetPen);

		dc->RestoreDC(sDC);
		return prect;
	};

    static void DrawCJS(CDC* dc, POINT p, string cjsText, COLORREF cjsColor);
};

