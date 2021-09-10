#include "pch.h"
#include "CAppWindows.h"

unsigned long CAppWindows::windowIDs_ = 0;
unsigned long SListBoxElement::m_elementIDcount = 0;
unsigned long STextField::m_textFieldIDcount = 0;
unsigned long SListBox::m_list_box_ids;

CAppWindows::CAppWindows()
{
	
}

CAppWindows::CAppWindows(POINT origin, int winType, CFlightPlan fp, RECT radarea, vector<string>* lbElements) {
	m_origin = origin;
	m_winType = winType;
	m_windowId_ = windowIDs_;
	m_callsign = fp.GetCallsign();
	string s;
	s = fp.GetCallsign();

	if (winType == WINDOW_CTRL_REMARKS) {
		s += " Ctrl Remarks";
		windowTitle = s.c_str();
		m_width = 300;
		m_height = 250;
		SListBox lb;
		lb.PopulateListBox(*lbElements);
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

		STextField freetext;
		freetext.m_location_ = { 16, 188 };
		freetext.m_height = 20;
		freetext.m_width = 268;
		freetext.m_parentWindowID = m_windowId_;
		m_textfields_.push_back(freetext);
	}

	m_origin.x = origin.x - m_width / 2;
	m_origin.y = origin.y - m_height / 2;

	if (origin.x < radarea.left) { m_origin.x = radarea.left; }
	if ((origin.x + m_width) > radarea.right) { m_origin.x = radarea.right - m_width; }
	if (origin.y < radarea.top + 60) { m_origin.y = radarea.top + 60; }
	if ((origin.y + m_height) > (radarea.bottom)) { m_origin.y = radarea.bottom - m_height; }

	windowIDs_++;
}

CAppWindows::CAppWindows(POINT origin, int winType, CFlightPlan fp, RECT radarea, ACRoute* rte) {
	m_origin = origin;
	m_winType = winType;
	m_windowId_ = windowIDs_;
	m_callsign = fp.GetCallsign();
	string s; 

	if (winType == WINDOW_DIRECT_TO) {
		windowTitle = m_callsign;
		m_height = 190;
		m_width = 110;

		SListBox lb;
		lb.m_max_elements = 5;
		lb.PopulateDirectListBox(rte, fp);
		lb.m_origin = m_origin;
		m_listboxes_.emplace_back(lb);

		SWindowButton submit, cancel;

		submit.location = { 6, 155 };
		submit.m_height = 25;
		submit.m_width = 45;
		submit.text = "Ok";
		submit.windowID = m_windowId_;

		cancel.location = { 56, 155 };
		cancel.m_height = 25;
		cancel.m_width = 45;
		cancel.text = "Cancel";
		cancel.windowID = m_windowId_;

		m_buttons_.push_back(submit);
		m_buttons_.push_back(cancel);

	}

	m_origin.x = origin.x - m_width / 2;
	m_origin.y = origin.y - m_height / 2;

	if (origin.x < radarea.left) { m_origin.x = radarea.left; }
	if ((origin.x + m_width) > radarea.right) { m_origin.x = radarea.right - m_width; }
	if (origin.y < radarea.top + 60) { m_origin.y = radarea.top + 60; }
	if ((origin.y + m_height) > (radarea.bottom)) { m_origin.y = radarea.bottom - m_height; }

	windowIDs_++;
}


CAppWindows::CAppWindows(POINT origin, int winType, CFlightPlan fp, RECT radarea) {
	m_origin = origin;
	m_winType = winType;
	m_windowId_ = windowIDs_;
	m_callsign = fp.GetCallsign();
	string s;
	s = fp.GetCallsign();

	if (winType == WINDOW_HANDOFF_EXT_CJS) {
		s += " H/O:";
		windowTitle = s.c_str();
		m_width = 105;
		m_height = 50;

		SWindowButton cancel;
		cancel.location = { 40, 24 };
		cancel.m_height = 20;
		cancel.m_width = 50;
		cancel.text = "Cancel";
		cancel.windowID = m_windowId_;
		m_buttons_.push_back(cancel);

		STextField cjsText;
		cjsText.m_location_ = { 8,24 };
		cjsText.m_height = 19;
		cjsText.m_width = 30;
		cjsText.m_focused = true;
		cjsText.m_parentWindowID = m_windowId_;
		m_textfields_.push_back(cjsText);


	}

	if (winType == WINDOW_POINT_OUT) {
		windowTitle = "Point Out";
		m_height = 85;
		m_width = 210;

		SWindowButton submit, cancel;

		submit.location = { 50, 50 };
		submit.m_height = 25;
		submit.m_width = 60;
		submit.text = "Submit";
		submit.windowID = m_windowId_;

		cancel.location = { 112, 50 };
		cancel.m_height = 25;
		cancel.m_width = 60;
		cancel.text = "Cancel";
		cancel.windowID = m_windowId_;

		m_buttons_.push_back(submit);
		m_buttons_.push_back(cancel);

		STextField cjsText;
		cjsText.m_location_ = { 8,28 };
		cjsText.m_height = 19;
		cjsText.m_width = 38;
		cjsText.m_parentWindowID = m_windowId_;
		m_textfields_.push_back(cjsText);

		STextField poMessage;
		poMessage.m_location_ = { 48,28 };
		poMessage.m_height = 19;
		poMessage.m_width = 152;
		poMessage.m_parentWindowID = m_windowId_;
		m_textfields_.push_back(poMessage);
	}

	m_origin.x = origin.x - m_width / 2;
	m_origin.y = origin.y - m_height / 2;

	if (origin.x < radarea.left) { m_origin.x = radarea.left; }
	if ((origin.x + m_width) > radarea.right) { m_origin.x = radarea.right - m_width; }
	if (origin.y < radarea.top + 60) { m_origin.y = radarea.top + 60; }
	if ((origin.y + m_height) > (radarea.bottom)) { m_origin.y = radarea.bottom - m_height; }

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
	int listboxDeltaX = 0;
	if (m_winType == WINDOW_CTRL_REMARKS) {
		listboxDeltaY = 25;
	}
	if (m_winType == WINDOW_DIRECT_TO) {
		listboxDeltaX = -10;
	}
	// Draw List Box if present
	for (auto& lb : this->m_listboxes_) {
		lb.m_dc = dc;
		lb.RenderListBox(1, 1, 1, { m_origin.x + listboxDeltaX, titleRect.bottom + 2 + listboxDeltaY});
		if (lb.m_has_scroll_bar) {
			lb.m_scrbar.m_max_elements = lb.m_max_elements;
			lb.m_scrbar.m_origin = { m_origin.x + lb.m_width + 9, titleRect.bottom + 2 + listboxDeltaY };
			lb.m_scrbar.Draw(dc);
		}
	}



	// Draw Buttons if present
	for (auto& but : this->m_buttons_) {
		but.m_dc = dc;
		but.RenderButton(m_origin);
	}

	// Draw textfields if present
	for (auto& textf : this->m_textfields_) {
		textf.RenderTextField(dc, m_origin);
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
	RECT totalListBox{ winOrigin.x + 16, winOrigin.y,  winOrigin.x + m_width - 16, winOrigin.y + static_cast<int>(listBox_.size() * 20) };
	m_dc->Draw3dRect(&totalListBox, C_MENU_GREY2, C_MENU_GREY4);
	this->m_width = totalListBox.right - totalListBox.left;

	DeleteObject(targetPen);
	DeleteObject(targetBrush);
	DeleteObject(tb2);
	DeleteObject(font);
	m_dc->RestoreDC(sDC);
}

void STextField::RenderTextField(CDC* m_dc, POINT origin) {
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

	RECT r = { origin.x + m_location_.x, origin.y + m_location_.y, origin.x + m_width + m_location_.x, origin.y + m_height + m_location_.y };
	m_dc->Rectangle(&r);
	if (m_focused) {
		m_dc->Draw3dRect(&r, RGB(0,200,0), RGB(0, 200, 0));

	}
	else {
		m_dc->Draw3dRect(&r, C_MENU_GREY2, C_MENU_GREY4);
	}
	r.left += 6;
	m_dc->DrawText(m_text.c_str(), &r, DT_LEFT | DT_SINGLELINE | DT_VCENTER);

	CopyRect(&m_textRect, &r);

	DeleteObject(targetPen);
	DeleteObject(targetBrush);
	DeleteObject(tb2);
	DeleteObject(font);
	m_dc->RestoreDC(sDC);
}
