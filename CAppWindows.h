#pragma once
#include "SituPlugin.h"
#include "constants.h"

struct SWindowElements {
	RECT titleBarRect;
	vector<RECT> ListBoxRect;
};

struct ObjectManager {

};

struct SListBoxElement {
	static unsigned long m_elementIDcount;
	int m_elementID;
	int m_width;
	int m_height;
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
	


};

struct SListBox {
	int m_width;
	int m_ListBoxID;
	int m_windowID_;
	int m_ListBoxIndex;
	CDC* m_dc;

	vector<SListBoxElement> listBox_;
	void PopulateListBox(){
		listBox_.emplace_back(SListBoxElement(300,"RNAV"));
		listBox_.emplace_back(SListBoxElement(300,"AFK"));
		listBox_.emplace_back(SListBoxElement(300,"VECTORS"));
		listBox_.emplace_back(SListBoxElement(300, ""));
		listBox_.emplace_back(SListBoxElement(300, ""));
		listBox_.emplace_back(SListBoxElement(300, ""));
		listBox_.emplace_back(SListBoxElement(300, ""));
		listBox_.emplace_back(SListBoxElement(300, ""));
	}
	void RenderListBox(int firstElem, int numElem, int maxElements, POINT winOrigin);
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

	CAppWindows();
	CAppWindows(POINT origin, int winType, CFlightPlan fp);
	SWindowElements DrawWindow(CDC* dc);
	void AddWindow(POINT origin, int winType, CFlightPlan* fp);
	void CloseWindow(int winID);
};
