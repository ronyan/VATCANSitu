#include "pch.h"
#include "CAppWindows.h"

unsigned long CAppWindows::windowIDs_ = 0;
unsigned long SListBoxElement::m_elementIDcount = 0;
unsigned long STextField::m_textFieldIDcount = 0;
unsigned long SListBox::m_list_box_ids;

std::string ZuluTimeFormated(std::time_t time) {
	std::tm timeInfo;

	if (gmtime_s(&timeInfo, &time) != 0) {
		return "Error in gmtime_s function.";
	}

	std::stringstream ss;
	ss << std::put_time(&timeInfo, "%H:%M");
	return ss.str();

}

CAppWindows::CAppWindows()
{
	
}

CAppWindows::CAppWindows(POINT origin, int winType, CFlightPlan& fp, RECT radarea, vector<CPDLCMessage>& cpdlcmsgs) {
	m_origin = origin;
	m_winType = winType;
	m_windowId_ = windowIDs_;
	string cs = fp.GetCallsign();

	if (winType == WINDOW_CPDLC_EDITOR) {
		windowTitle = "CPDLC Message Editor - " + cs;
		m_width = 500;
		m_height = 250;

		SWindowText s;

		s.text = "Open Downlink Dialogues";
		s.location = { 5, 22 };
		m_text_.push_back(s);

		s.text = "ID";
		s.location = { 12,34 };
		m_text_.push_back(s);

		s.text = "Time";
		s.location = { 28,34 };
		m_text_.push_back(s);

		s.text = "Text";
		s.location = { 78,34 };
		m_text_.push_back(s);

		s.text = "Uplink Messages";
		s.location = { 5,120 };
		m_text_.push_back(s);

		s.text = "Text";
		s.location = { 14,132 };
		m_text_.push_back(s);

		// Print the selected list box in the parent CPDLC window
		SListBoxElement lbe(0, ""); // dummy element
		SListBox lb;
		lb.listBox_.push_back(lbe);
		lb.m_max_elements = 8;
		lb.m_width = 495;
		lb.m_origin.x = 0;
		lb.m_origin.y = 27;
		m_listboxes_.push_back(lb);


		SWindowButton b;
		b.location = { 430, 219 };
		b.m_height = 25;
		b.m_width = 60;
		b.text = "Send";
		b.windowID = m_windowId_;

		m_buttons_.push_back(b);
		
		STextField stf;
		stf.m_parentWindowID = m_windowId_;
		stf.m_width = 488;
		stf.m_height = 72;
		stf.m_location_ = { 5,48 };
		stf.m_textfield_type = TEXTFIELD_CPDLC_MESSAGE;
		m_textfields_.push_back(stf);

		stf.m_location_ = { 5,145 };
		stf.m_textfield_type = TEXTFIELD_CPDLC_PENDING_UPLINK;
		m_textfields_.push_back(stf);

	}

	if (winType == WINDOW_CPDLC) {


		windowTitle = "CPDLC - " + cs;
		m_width = 420;
		m_height = 253;

		for (int i = 0; i < 5; i++) {
			SWindowButton b;
			b.location = { (5 + i * 65), 161 };
			b.m_height = 25;
			b.m_width = 64;
			b.windowID = m_windowId_;
			b.m_textcolor = RGB(0,200,0);
			if (i == 0) { b.text = "Unable"; }
			if (i == 1) { b.text = "Roger"; }
			if (i == 2) { b.text = "Affirm"; }
			if (i == 3) { b.text = "Negative"; }
			if (i == 4) { b.text = "Deferred"; }

			m_buttons_.push_back(b);
		}
		for (int i = 0; i < 5; i++) {
			for (int j = 0; j < 3; j++) {

				SWindowButton b;
				b.location = { (5 + i * 65), 186 + j*20 };
				b.m_height = 20;
				b.m_width = 64;
				b.windowID = m_windowId_;
				b.m_textcolor = RGB(0, 200, 0);

				if (j == 2 && i == 4) { b.text = "PDC"; }

				m_buttons_.push_back(b);
			}
		}
		
		SWindowButton close, flightplan, closedialog, endservice, connect, clrsend;

		close.location = { 333, 221 };
		close.m_height = 25;
		close.m_width = 80;
		close.text = "Close";
		close.windowID = m_windowId_;

		flightplan.location = { 333, 192 };
		flightplan.m_height = 25;
		flightplan.m_width = 80;
		flightplan.text = "Flight Plan";
		flightplan.windowID = m_windowId_;

		closedialog.location = { 333, 137 };
		closedialog.m_height = 25;
		closedialog.m_width = 80;
		closedialog.text = "Close Dialog";
		closedialog.windowID = m_windowId_;

		endservice.location = { 333, 82 };
		endservice.m_height = 25;
		endservice.m_width = 80;
		endservice.text = "End Service";
		endservice.windowID = m_windowId_;

		connect.location = { 333, 55 };
		connect.m_height = 25;
		connect.m_width = 80;
		connect.text = "Connect";
		connect.windowID = m_windowId_;

		m_buttons_.push_back(close);
		m_buttons_.push_back(flightplan);
		m_buttons_.push_back(closedialog);
		m_buttons_.push_back(endservice);
		m_buttons_.push_back(connect);

		SListBox lb;
		lb.m_max_elements = 8;
		lb.m_width = 328;
		lb.m_origin.x = 0;
		lb.m_origin.y = 0;
		lb.PopulateCPDLCListBox(cpdlcmsgs);
		m_listboxes_.emplace_back(lb);

	}

	m_origin.x = origin.x - m_width / 2;
	m_origin.y = origin.y - m_height / 2;

	if (m_origin.x < radarea.left) { m_origin.x = radarea.left; }
	if ((m_origin.x + m_width) > radarea.right) { m_origin.x = radarea.right - m_width; }
	if (m_origin.y < radarea.top + 60) { m_origin.y = radarea.top + 60; }
	if ((m_origin.y + m_height) > (radarea.bottom)) { m_origin.y = radarea.bottom - m_height; }

	windowIDs_++;


}

CAppWindows::CAppWindows(POINT origin, int winType, RECT radarea) {
	m_origin = origin;
	m_winType = winType;
	m_windowId_ = windowIDs_;

	if (winType == WINDOW_FREE_TEXT) {
		windowTitle = "Free Text";
		m_width = 300;
		m_height = 100;
		SListBox lb;

		SWindowButton submit, cancel;

		submit.location = { 90, 60 };
		submit.m_height = 25;
		submit.m_width = 60;
		submit.text = "Submit";
		submit.windowID = m_windowId_;

		cancel.location = { 155, 60 };
		cancel.m_height = 25;
		cancel.m_width = 60;
		cancel.text = "Cancel";
		cancel.windowID = m_windowId_;

		m_buttons_.push_back(submit);
		m_buttons_.push_back(cancel);

		STextField freetext;
		freetext.m_location_ = { 16, 30 };
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

	dc->SelectObject(CFontHelper::Segoe14);
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
	if (m_winType == WINDOW_CPDLC) {
		listboxDeltaX = -12;
	}
	// Draw List Box if present

	// 
	if(m_winType == WINDOW_CPDLC_EDITOR) { // skip LB draw for CPDLC EDITOR 
	}

	else {
		for (auto& lb : this->m_listboxes_) {
			lb.m_dc = dc;
			if (m_winType == WINDOW_CPDLC || m_winType == WINDOW_CPDLC_EDITOR) {
				lb.RenderCPDLCListBox(1, 1, 8, { m_origin.x, titleRect.bottom + 2 + lb.m_origin.y });
				if (lb.m_has_scroll_bar) {
					lb.m_scrbar.m_max_elements = lb.m_max_elements;
					lb.m_scrbar.m_origin = { m_origin.x + lb.m_width + 9, titleRect.bottom + 2 + listboxDeltaY };
					lb.m_scrbar.Draw(dc);
				}
			}
			else {
				lb.RenderListBox(1, 1, 1, { m_origin.x + listboxDeltaX, titleRect.bottom + 2 + listboxDeltaY });
				if (lb.m_has_scroll_bar) {
					lb.m_scrbar.m_max_elements = lb.m_max_elements;
					lb.m_scrbar.m_origin = { m_origin.x + lb.m_width + 9, titleRect.bottom + 2 + listboxDeltaY };
					lb.m_scrbar.Draw(dc);
				}
			}
		}
	}

	// Draw CPDLC Messages LB 

	// Draw Buttons if present
	for (auto& but : this->m_buttons_) {
		but.m_dc = dc;
		but.RenderButton(m_origin);
	}

	// Draw textfields if present
	for (auto& textf : this->m_textfields_) {

		// Dynamic Text
		if (!m_listboxes_.empty()) {
			if (!m_listboxes_.at(0).listBox_.empty())
			{
				textf.m_text = this->m_listboxes_.at(0).listBox_.at(0).m_cpdlc_message.rawMessageContent;
			}
		}
		textf.RenderTextField(dc, m_origin);
	}

	for (auto& text : this->m_text_) {
		text.RenderText(dc, m_origin);
	}

	DeleteObject(targetPen);
	DeleteObject(targetBrush);
	dc->RestoreDC(sDC);

	w.titleBarRect = titleRect;
	return w;
	}

void SListBox::RenderCPDLCListBox(int firstElem, int numElem, int maxElements, POINT winOrigin) {
	int sDC = m_dc->SaveDC();

	CFont font;
	LOGFONT lgfont;

	memset(&lgfont, 0, sizeof(LOGFONT));
	lgfont.lfWeight = 400;
	lgfont.lfWidth = 6;
	strcpy_s(lgfont.lfFaceName, _T("Euroscope"));
	lgfont.lfHeight = 14;
	font.CreateFontIndirect(&lgfont);

	m_dc->SelectObject(font);
	m_dc->SetTextColor(RGB(230, 230, 230));

	// zebrastripe with C_MENU_GREY2

	HPEN targetPen = CreatePen(PS_SOLID, 1, C_MENU_GREY1);
	HPEN targetPen2 = CreatePen(PS_SOLID, 1, C_MENU_GREY2);
	HBRUSH targetBrush = CreateSolidBrush(C_MENU_GREY1);
	HBRUSH targetBrush2 = CreateSolidBrush(C_MENU_GREY2);
	HBRUSH tb2 = CreateSolidBrush(C_MENU_GREY4);

	int deltay = 0;
	int row = 0;
	for (auto& element : listBox_) {

		m_dc->SelectObject(targetPen2);
		m_dc->SelectObject(targetBrush2);

		this->m_width = element.m_width;
		RECT r = { winOrigin.x + 3, winOrigin.y + deltay, winOrigin.x + m_width, winOrigin.y + deltay + 17 };
		CopyRect(&element.m_ListBoxRect, &r);

		if (element.m_selected_) {
			m_dc->SetTextColor(C_MENU_GREY1);
			m_dc->SelectObject(tb2);
		}
		else {
			m_dc->SetTextColor(RGB(230, 230, 230));
		}

		if (row % 2 == 0 && !element.m_selected_) {
			m_dc->SelectObject(targetPen);
			m_dc->SelectObject(targetBrush);
		}
		// make the string
		string cpdlcOutput;
		cpdlcOutput = ZuluTimeFormated(element.m_cpdlc_message.timeParsed);
		if (element.m_cpdlc_message.isdlMessage) {
			cpdlcOutput += "  |D/L  ";
		}
		else {
			cpdlcOutput += "  ^U/L  ";
		}
		if (element.m_cpdlc_message.rawMessageContent.length() > 28) {
			cpdlcOutput += element.m_cpdlc_message.rawMessageContent.substr(0,28);
			cpdlcOutput += "  >";
		}
		else {
			cpdlcOutput += element.m_cpdlc_message.rawMessageContent;
		}

		// need logic for whether the UP/DL pair is complete, or still pending.

		if (!element.m_cpdlc_message_response.rawMessageContent.empty()) {
			// if there is a response, put this on the second line, with the appropriate data. 

		}
		
		m_dc->Rectangle(&r);
		r.left += 6;
		m_dc->DrawText(cpdlcOutput.c_str(), &r, DT_LEFT | DT_SINGLELINE | DT_VCENTER );

		r.top += deltay;
		deltay += 17;
		row++;
	}
	RECT totalListBox{ winOrigin.x+4, winOrigin.y,  winOrigin.x + m_width, winOrigin.y + static_cast<int>(listBox_.size() * 17) };
	m_dc->Draw3dRect(&totalListBox, C_MENU_GREY2, C_MENU_GREY4);
	this->m_width = totalListBox.right - totalListBox.left;

	DeleteObject(targetPen);
	DeleteObject(targetPen2);
	DeleteObject(targetBrush);
	DeleteObject(targetBrush2);
	DeleteObject(tb2);
	DeleteObject(font);
	m_dc->RestoreDC(sDC);
}

void SListBox::RenderListBox(int firstElem, int numElem, int maxElements, POINT winOrigin) {
	int sDC = m_dc->SaveDC();

	m_dc->SelectObject(CFontHelper::Segoe14);
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
	RECT totalListBox{ winOrigin.x+16, winOrigin.y,  winOrigin.x + m_width-16, winOrigin.y + static_cast<int>(listBox_.size() * 20) };
	m_dc->Draw3dRect(&totalListBox, C_MENU_GREY2, C_MENU_GREY4);
	this->m_width = totalListBox.right - totalListBox.left;

	DeleteObject(targetPen);
	DeleteObject(targetBrush);
	DeleteObject(tb2);
	m_dc->RestoreDC(sDC);
}

void STextField::RenderTextField(CDC* m_dc, POINT origin) {
	int sDC = m_dc->SaveDC();

	CFont font;
	CFont CAATSfont;
	LOGFONT lgfont;

	memset(&lgfont, 0, sizeof(LOGFONT));
	lgfont.lfWeight = 500;
	strcpy_s(lgfont.lfFaceName, _T("Segoe UI"));
	lgfont.lfHeight = 14;
	font.CreateFontIndirect(&lgfont);
	lgfont.lfWidth = 6;
	lgfont.lfHeight = 14;
	strcpy_s(lgfont.lfFaceName, _T("Euroscope"));
	CAATSfont.CreateFontIndirect(&lgfont);

	m_dc->SelectObject(font);
	m_dc->SetTextColor(RGB(230, 230, 230));

	HPEN targetPen = CreatePen(PS_SOLID, 1, C_MENU_GREY1);
	HBRUSH targetBrush = CreateSolidBrush(C_MENU_GREY1);
	HBRUSH tb2 = CreateSolidBrush(C_MENU_GREY4);

	m_dc->SelectObject(targetPen);
	m_dc->SelectObject(targetBrush);

	if (m_textfield_type == TEXTFIELD_CPDLC_MESSAGE) {

		m_dc->SelectObject(CAATSfont);
		RECT r = { origin.x + m_location_.x, origin.y + m_location_.y, origin.x + m_width + m_location_.x, origin.y + m_height + m_location_.y };
		m_dc->Rectangle(&r);
		CopyRect(&m_textRect, &r);

		m_dc->Draw3dRect(&r, C_MENU_GREY2, C_MENU_GREY4);
		r.left += 8;

		if (m_cpdlcmessage.hoppieMessageID != "") { // will be "" if nothing was pushed to it
			// message ID
			if (m_cpdlcmessage.messageID != -1) {
				m_dc->DrawText(to_string(m_cpdlcmessage.messageID).c_str(), &r, DT_LEFT | DT_SINGLELINE | DT_CALCRECT);
				m_dc->DrawText(to_string(m_cpdlcmessage.messageID).c_str(), &r, DT_LEFT | DT_SINGLELINE);
			}

			r.left += 15;

			// message time
			m_dc->DrawText(ZuluTimeFormated(m_cpdlcmessage.timeParsed).c_str(), &r, DT_LEFT | DT_SINGLELINE | DT_CALCRECT);
			m_dc->DrawText(ZuluTimeFormated(m_cpdlcmessage.timeParsed).c_str(), &r, DT_LEFT | DT_SINGLELINE);

			// message content
			r.left += 50;
			r.right = origin.x + m_width + m_location_.x;
			r.bottom = origin.y + m_location_.y + m_height;
			m_dc->DrawText(m_cpdlcmessage.rawMessageContent.c_str(), &r, DT_LEFT | DT_WORDBREAK);
		}
	}

	else if (m_textfield_type == TEXTFIELD_CPDLC_PENDING_UPLINK) {

		m_dc->SelectObject(CAATSfont);
		RECT r = { origin.x + m_location_.x, origin.y + m_location_.y, origin.x + m_width + m_location_.x, origin.y + m_height + m_location_.y };
		m_dc->Rectangle(&r);
		CopyRect(&m_textRect, &r);

		m_dc->Draw3dRect(&r, C_MENU_GREY2, C_MENU_GREY4);
		r.left += 8;

		// message content
		r.right = origin.x + m_width + m_location_.x;
		r.bottom = origin.y + m_location_.y + m_height;

		string s;
		s = m_cpdlcmessage.rawMessageContent.c_str();

		// remove @ symbol for display in the UI
		size_t found = s.find('@');
		while (found != std::string::npos) {
			s.erase(found, 1);
			found = s.find('@', found); // Find the next "@" symbol
		}

		m_dc->DrawText(s.c_str(), &r, DT_LEFT | DT_WORDBREAK);

	}

	else {

		RECT r = { origin.x + m_location_.x, origin.y + m_location_.y, origin.x + m_width + m_location_.x, origin.y + m_height + m_location_.y };
		m_dc->Rectangle(&r);
		if (m_focused) {
			m_dc->Draw3dRect(&r, RGB(0, 200, 0), RGB(0, 200, 0));

		}
		else {
			m_dc->Draw3dRect(&r, C_MENU_GREY2, C_MENU_GREY4);
		}
		r.left += 6;
		m_dc->DrawText(m_text.c_str(), &r, DT_LEFT | DT_SINGLELINE | DT_VCENTER);

		CopyRect(&m_textRect, &r);
	}


	DeleteObject(targetPen);
	DeleteObject(targetBrush);
	DeleteObject(tb2);
	DeleteObject(font);
	DeleteObject(CAATSfont);
	m_dc->RestoreDC(sDC);
}

void SWindowText::RenderText(CDC* m_dc, POINT origin) {
	int sDC = m_dc->SaveDC();

	CFont font;
	LOGFONT lgfont;

	memset(&lgfont, 0, sizeof(LOGFONT));
	lgfont.lfWeight = 500;
	strcpy_s(lgfont.lfFaceName, _T("Segoe UI"));
	lgfont.lfHeight = 13;
	font.CreateFontIndirect(&lgfont);

	m_dc->SelectObject(font);
	m_dc->SetTextColor(RGB(230, 230, 230));

	RECT r = { origin.x + location.x, origin.y + location.y, origin.x + location.x, origin.y + location.y };
	m_dc->Rectangle(&r);
	m_dc->DrawText(text.c_str(), &r, DT_LEFT | DT_SINGLELINE | DT_CALCRECT);
	m_dc->DrawText(text.c_str(), &r, DT_LEFT | DT_SINGLELINE );

	DeleteObject(font);
	m_dc->RestoreDC(sDC);

}
