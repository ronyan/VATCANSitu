#pragma once
#include "SituPlugin.h"
#include "CSiTRadar.h"
#include <vector>

struct SPopUpElement
{
	string m_text;
	bool m_isHeader;
	bool m_hasArrow;
	RECT elementRect;
	SPopUpElement(string t, bool header, bool hasarrow) {
		m_text = t;
		m_isHeader = header;
		m_hasArrow = hasarrow;
	}
};

class CPopUpMenu
{
public:
	POINT m_origin;
	vector<SPopUpElement> m_listElements{};
	CFlightPlan* m_fp;
	CRadarScreen* m_rad;
	CDC* m_dc;
	static RECT prevRect;
	static RECT totalRect;

	CPopUpMenu(POINT p, CFlightPlan* fp, CRadarScreen* rad) {
		m_origin = p;
		m_fp = fp;
		m_rad = rad;
		m_dc = nullptr;
	}
	void drawPopUpMenu(CDC* dc);
	void highlightSelection(CDC* dc, RECT rect);
	void populateMenu();
	void drawElement(SPopUpElement& element, POINT p);
	
};

