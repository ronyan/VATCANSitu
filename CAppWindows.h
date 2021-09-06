#pragma once
#include "SituPlugin.h"
#include "constants.h"
#include <vector>

struct SWindowElements {
	RECT titleBarRect;
	vector<RECT> ListBoxRect;
};

struct SListBoxScrollBar {
	int m_height;
	int m_width;
	int m_list_box_id;
	int m_scroll_bar_id;
	int m_max_elements;
	double m_slider_height_ratio;
	int m_slider_location;
	int m_clicks;
	RECT uparrow;
	RECT downarrow;
	POINT m_origin;
	SListBoxScrollBar() {}

	SListBoxScrollBar(int h, int w, int listboxid, POINT origin, int firstelem, int clicks) {
		m_height = h;
		m_width = w;
		m_list_box_id = listboxid;
		m_origin = origin;
		m_slider_location = firstelem;

		m_clicks = clicks;
	}
	void Draw(CDC* dc) {
		int sDC = dc->SaveDC();

		HPEN targetPen = CreatePen(PS_SOLID, 1, C_MENU_GREY4);
		HBRUSH targetBrush = CreateSolidBrush(C_MENU_GREY1);
		HBRUSH tb2 = CreateSolidBrush(C_MENU_GREY4);
		dc->SelectObject(targetPen);
		dc->SelectObject(targetBrush);

		RECT scrollbar;
		RECT slider;

		scrollbar.top = m_origin.y;
		scrollbar.bottom = m_origin.y + m_height;
		scrollbar.left = m_origin.x;
		scrollbar.right = m_origin.x + m_width;

		uparrow.top = m_origin.y;
		uparrow.left = m_origin.x;
		uparrow.right = m_origin.x + m_width;
		uparrow.bottom = m_origin.y + 10;

		downarrow.top = m_origin.y + m_height - 10;
		downarrow.left = m_origin.x;
		downarrow.right = m_origin.x + m_width;
		downarrow.bottom = m_origin.y + m_height;
		
		slider.left = m_origin.x +1;
		slider.right = m_origin.x + m_width -1;

		int deltay = (downarrow.top - uparrow.bottom) / m_clicks;
		slider.top = uparrow.bottom + deltay* m_slider_location;
		slider.bottom = slider.top + round((downarrow.top - uparrow.bottom)/m_clicks);

		dc->MoveTo({ uparrow.left + 1, uparrow.bottom - 3 });
		dc->LineTo({ uparrow.left + 4, uparrow.top + 2 });
		dc->LineTo({ uparrow.right - 3, uparrow.bottom - 3 });
		dc->LineTo({ uparrow.left + 2, uparrow.bottom - 3 });

		dc->MoveTo({ downarrow.left + 1, downarrow.top +1 });
		dc->LineTo({ downarrow.left + 4, downarrow.bottom - 2 });
		dc->LineTo({ downarrow.right - 3, downarrow.top + 1 });
		dc->LineTo({ downarrow.left + 1, downarrow.top + 1 });

		dc->Draw3dRect(&scrollbar, C_MENU_GREY2, C_MENU_GREY4);
		dc->Draw3dRect(&uparrow, C_MENU_GREY2, C_MENU_GREY4);
		dc->Draw3dRect(&downarrow, C_MENU_GREY2, C_MENU_GREY4);
		dc->Draw3dRect(&slider, C_MENU_GREY4, C_MENU_GREY1);

		DeleteObject(targetPen);
		DeleteObject(targetBrush);
		DeleteObject(tb2);
		dc->RestoreDC(sDC);
	}
};


struct ObjectManager {

};

struct SListBoxElement {
	static unsigned long m_elementIDcount;
	int m_elementID;
	int m_width;
	int m_height{ 20 };
	int m_listElementHeight;
	bool m_selected_{ false };
	string m_ListBoxElementText;
	RECT m_ListBoxRect;

	SListBoxElement(int width, string text) {
		m_width = width;
		m_ListBoxElementText = text;
		m_elementID = m_elementIDcount;
		m_elementIDcount++;
	}
	SListBoxElement(int width, string text, bool selected) {
		m_width = width;
		m_ListBoxElementText = text;
		m_selected_ = selected;

		m_elementID = m_elementIDcount;
		m_elementIDcount++;
	}
	void Select() {
		m_selected_ = true;
	}
};

struct STextField {
	static unsigned long m_textFieldIDcount;
	unsigned long m_textFieldID;
	unsigned long m_parentWindowID;
	int m_width;
	int m_height;
	bool m_focused{ false };
	string m_text;
	RECT m_textRect;
	POINT m_location_;

	STextField() {
		m_textFieldID = m_textFieldIDcount;

		m_textFieldIDcount++;
	}
	STextField(POINT loc, int width, int height) {
		m_location_ = loc;
		m_width = width;
		m_height = height;
		m_textFieldID = m_textFieldIDcount;

		m_textFieldIDcount++;
	}
	void RenderTextField(CDC* m_dc, POINT origin);
};

struct SListBox {
	static unsigned long m_list_box_ids;
	int m_width;
	unsigned long m_ListBoxID;
	int m_windowID_;
	int m_nearestPtIdx;
	int m_LB_firstElem_idx{ 0 };
	int m_max_elements;
	int m_last_element;
	int m_height;
	bool m_has_scroll_bar{ false };
	string selectItem{};
	SListBoxScrollBar m_scrbar;
	POINT m_origin;
	CDC* m_dc;
	SListBox() {
		m_ListBoxID = m_list_box_ids;
		m_list_box_ids++;
	}

	vector<SListBoxElement> listBox_;
	void PopulateListBox(std::vector<string> lb_e_vector){
		for (auto& lbe : lb_e_vector) {
			listBox_.emplace_back(SListBoxElement(300, lbe));
		}
	}
	void PopulateDirectListBox(ACRoute* rte, CFlightPlan fp) {

		m_nearestPtIdx = fp.GetExtractedRoute().GetPointsCalculatedIndex();
		int directPtIdx = fp.GetExtractedRoute().GetPointsAssignedIndex();

		int i = 0;
		int j = 0;
		m_height = 0;
		m_last_element = (int)rte->fix_names.size();
		for (auto& rtefix : rte->fix_names) {
			if (i >= (m_nearestPtIdx + m_LB_firstElem_idx)) {
				if (j < m_max_elements) {
					SListBoxElement lbe(115, rtefix);
					// remember selected item on scroll
					if (!strcmp(rtefix.c_str(), this->selectItem.c_str())) {
						lbe.m_selected_ = true;
					}
					listBox_.emplace_back(lbe);
					m_height += lbe.m_height;
				}
				j++;
			}
			i++;
		}
		while (j < m_max_elements) {
			SListBoxElement lbe(115, "");
			listBox_.emplace_back(lbe);
			m_height += lbe.m_height;
			j++;
		}

		if (((int)rte->fix_names.size() - m_nearestPtIdx) > m_max_elements) {
			// Draw a scroll bar
			m_has_scroll_bar = true;
			SListBoxScrollBar scrollBar(m_height, 10, m_ListBoxID, m_origin, m_LB_firstElem_idx, ((int)rte->fix_names.size() - m_nearestPtIdx - m_max_elements) + 1);
			scrollBar.m_height = m_height;
			scrollBar.m_slider_height_ratio = (double)m_max_elements / (double)((double)rte->fix_names.size() - (double)m_nearestPtIdx);
			m_scrbar = scrollBar;
		}
	}
	void RenderListBox(int firstElem, int numElem, int maxElements, POINT winOrigin);
	void ScrollUp() {
		if (m_LB_firstElem_idx > 0) {
			m_LB_firstElem_idx--;
		}
	}
	void ScrollDown() {
		if (m_nearestPtIdx + m_LB_firstElem_idx + m_max_elements < m_last_element)
		{
			m_LB_firstElem_idx++;
		}
	}

	SListBoxElement GetListBoxElement(const char* elementID){
		return *find_if(listBox_.begin(), listBox_.end(), [&elementID](const SListBoxElement& obj) { return !strcmp(obj.m_ListBoxElementText.c_str(), elementID); });
	}
};

struct SWindowButton {
	int windowID;
	int m_width;
	int m_height;
	POINT location;
	int function;
	string text;
	RECT m_WindowButtonRect;
	CDC* m_dc;

	SWindowButton() {}
	
	SWindowButton(int w, int h, POINT loc) {
		m_width = w;
		m_height = h;
		location = loc;
	}

	void RenderButton(POINT p) {
		int sDC = m_dc->SaveDC();

		CFont font;
		LOGFONT lgfont;

		memset(&lgfont, 0, sizeof(LOGFONT));
		lgfont.lfWeight = 500;
		strcpy_s(lgfont.lfFaceName, _T("Segoe UI"));
		lgfont.lfHeight = 14;
		font.CreateFontIndirect(&lgfont);

		m_dc->SelectObject(font);
		m_dc->SetTextColor(RGB(230, 230, 230));

		HPEN targetPen = CreatePen(PS_SOLID, 1, C_MENU_GREY1);
		HBRUSH targetBrush = CreateSolidBrush(C_MENU_GREY3);
		HBRUSH tb2 = CreateSolidBrush(C_MENU_GREY3);

		m_dc->SelectObject(targetPen);
		m_dc->SelectObject(targetBrush);

		RECT button{ p.x+location.x, p.y+location.y,  p.x + location.x + m_width, p.y + location.y + m_height };
		m_dc->Rectangle(&button);
		m_dc->DrawText(text.c_str(), &button, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
		m_dc->Draw3dRect(&button, C_MENU_GREY4, C_MENU_GREY1);

		DeleteObject(targetPen);
		DeleteObject(targetBrush);
		DeleteObject(tb2);
		DeleteObject(font);
		m_dc->RestoreDC(sDC);

		CopyRect(&m_WindowButtonRect, &button);
	}
};

class CAppWindows
{
protected:
	static unsigned long CAppWindows::windowIDs_;

public:
	int m_windowId_;
	int m_winType;
	int m_width{ 200 };
	int m_height{ 200 };
	string m_callsign{};
	POINT m_origin;
	string windowTitle;
	bool m_visible_;
	vector<SListBox> m_listboxes_;
	vector<SWindowButton> m_buttons_;
	vector<STextField> m_textfields_;

	CAppWindows();
	CAppWindows(POINT origin, int winType, CFlightPlan fp, RECT radarea, vector<string>* lbElements);
	CAppWindows(POINT origin, int winType, CFlightPlan fp, RECT radarea, ACRoute* rte);
	CAppWindows(POINT origin, int winType, CFlightPlan fp, RECT radarea);
	SListBox GetListBox(int id) {
		return *find_if(m_listboxes_.begin(), m_listboxes_.end(), [&id](const SListBox& obj) { return obj.m_ListBoxID == id; });
	}
	SWindowElements DrawWindow(CDC* dc);

	void AddWindow(POINT origin, int winType, CFlightPlan* fp);
	void CloseWindow(int winID);
};
