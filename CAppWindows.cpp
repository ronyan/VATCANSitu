#include "pch.h"
#include "CAppWindows.h"

unsigned long CAppWindows::windowIDs_ = 0;
unsigned long SListBoxElement::m_elementIDcount = 0;

CAppWindows::CAppWindows()
{
	
}

CAppWindows::CAppWindows(POINT origin, int winType, CFlightPlan fp) {
	m_origin = origin;
	m_winType = winType;
	m_windowId_ = windowIDs_;
	m_callsign = fp.GetCallsign();

	if (winType == WINDOW_CTRL_REMARKS) {
		string s;
		s = fp.GetCallsign();
		s += " Ctrl Remarks";
		windowTitle = s.c_str();
		m_width = 300;
		m_height = 250;
		SListBox lb;
		lb.PopulateListBox();
		m_listboxes_.emplace_back(lb);

		SWindowButton blank, submit, cancel;

		submit.location = { 90, 210 };
		submit.m_height = 25;
		submit.m_width = 60;
		submit.text = "Submit";
		submit.windowID = m_windowId_;

		cancel.location = { 155, 210 };
		cancel.m_height = 25;
		cancel.m_width = 60;
		cancel.text = "Cancel";
		cancel.windowID = m_windowId_;

		blank.location = { 120, 22 };
		blank.m_height = 25;
		blank.m_width = 60;
		blank.text = "Blank";
		blank.windowID = m_windowId_;

		m_buttons_.push_back(submit);
		m_buttons_.push_back(blank);
		m_buttons_.push_back(cancel);
	}

	windowIDs_++;
}

SWindowElements CAppWindows::DrawWindow(CDC* dc) {
	SWindowElements w;
	
	int sDC = dc->SaveDC();
	CFont font;
	LOGFONT lgfont;

	memset(&lgfont, 0, sizeof(LOGFONT));
	lgfont.lfWeight = 500;
	strcpy_s(lgfont.lfFaceName, _T("Segoe UI"));
	lgfont.lfHeight = 14;
	font.CreateFontIndirect(&lgfont);

	dc->SelectObject(font);
	dc->SetTextColor(RGB(230, 230, 230));

	//default is unpressed state
	COLORREF pressedcolor = RGB(66, 66, 66);
	COLORREF pcolortl = RGB(140, 140, 140);
	COLORREF pcolorbr = RGB(55, 55, 55);

	COLORREF targetPenColor = RGB(66, 66, 66);
	HPEN targetPen = CreatePen(PS_SOLID, 1, targetPenColor);
	HBRUSH targetBrush = CreateSolidBrush(pressedcolor);

	dc->SelectObject(targetBrush);
	dc->SelectObject(targetPen);

	// Draw Title

	RECT windowRect = { m_origin.x, m_origin.y , m_origin.x + m_width, m_origin.y + m_height };
	RECT titleRect = { m_origin.x, m_origin.y , m_origin.x + m_width, m_origin.y + 24 };

	dc->Rectangle(&windowRect);
	dc->Draw3dRect(&windowRect, C_MENU_GREY4, C_MENU_GREY2);
	InflateRect(&windowRect, -3, -3);
	dc->Draw3dRect(&windowRect, C_MENU_GREY2, C_MENU_GREY4);

	dc->DrawText(this->windowTitle.c_str(), &titleRect, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
	InflateRect(&titleRect, -3, -3);
	dc->Draw3dRect(&titleRect, C_MENU_GREY2, C_MENU_GREY4);

	// Draw Secondary title if present

	int listboxDeltaY = 0;
	if (m_winType == WINDOW_CTRL_REMARKS) {
		listboxDeltaY = 25;
	}
	// Draw List Box if present
	for (auto& lb : this->m_listboxes_) {
		lb.m_dc = dc;
		lb.RenderListBox(1, 1, 1, { m_origin.x, titleRect.bottom + 2 + listboxDeltaY});
	}
	// Draw Buttons if present
	for (auto& but : this->m_buttons_) {
		but.m_dc = dc;
		but.RenderButton(m_origin);
	}

	DeleteObject(targetPen);
	DeleteObject(targetBrush);
	DeleteObject(font);
	dc->RestoreDC(sDC);

	w.titleBarRect = titleRect;
	return w;
	}

void SListBox::RenderListBox(int firstElem, int numElem, int maxElements, POINT winOrigin) {
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
	HBRUSH targetBrush = CreateSolidBrush(C_MENU_GREY1);
	HBRUSH tb2 = CreateSolidBrush(C_MENU_GREY4);

	m_dc->SelectObject(targetPen);
	m_dc->SelectObject(targetBrush);

	int deltay = 0;
	for (auto& element : listBox_) {
		m_dc->SelectObject(targetBrush);
		this->m_width = element.m_width;
		RECT r = { winOrigin.x + 16, winOrigin.y + deltay, winOrigin.x + m_width - 16, winOrigin.y + deltay + 20 };
		CopyRect(&element.m_ListBoxRect, &r);

		if (element.m_selected_) {
			m_dc->SetTextColor(C_MENU_GREY1);
			m_dc->SelectObject(tb2);
		}
		else {
			m_dc->SetTextColor(RGB(230, 230, 230));
		}

		m_dc->Rectangle(&r);
		r.left += 6;
		m_dc->DrawText(element.m_ListBoxElementText.c_str(), &r, DT_LEFT | DT_SINGLELINE | DT_VCENTER);

		r.top += deltay;
		deltay += 20;
	}
	RECT totalListBox{ winOrigin.x + 16, winOrigin.y,  winOrigin.x + m_width - 16, winOrigin.y + listBox_.size() * 20 };
	m_dc->Draw3dRect(&totalListBox, C_MENU_GREY2, C_MENU_GREY4);

	DeleteObject(targetPen);
	DeleteObject(targetBrush);
	DeleteObject(tb2);
	DeleteObject(font);
	m_dc->RestoreDC(sDC);
}
