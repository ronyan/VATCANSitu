#pragma once
#include "SituPlugin.h"
#include "CSiTRadar.h"
#include <vector>
#include <map>

struct SPopUpElement
{
	std::string m_text;
	int m_isHeaderFooter;
	bool m_hasArrow;
	std::string m_function;
	int m_width;
	RECT elementRect;
	SPopUpElement(std::string t, std::string function, int header, bool hasarrow) {
		m_text = t;
		m_isHeaderFooter = header;
		m_hasArrow = hasarrow;
		m_function = function;
		m_width = 115;
	}
	SPopUpElement(std::string t, std::string function, int header, bool hasarrow, int width) {
		m_text = t;
		m_isHeaderFooter = header;
		m_hasArrow = hasarrow;
		m_function = function;
		m_width = width;
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
	int m_width_;
	COLORREF textColor = RGB(230, 230, 230);

	CPopUpMenu(POINT p) {
		m_origin = p;
	}

	CPopUpMenu(POINT p, CFlightPlan* fp, CRadarScreen* rad) {
		m_origin = p;
		m_fp = fp;
		m_rad = rad;
		m_dc = nullptr;
	}
	void drawPopUpMenu(CDC* dc);
	void highlightSelection(CDC* dc, RECT rect);
	void populateMenu();
	void populateCPDLCOptions(string func);
	void drawElement(SPopUpElement& element, POINT p);
	void populateSecondaryMenu(std::string type);
};

