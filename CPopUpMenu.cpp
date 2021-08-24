#include "pch.h"
#include "CPopUpMenu.h"

RECT CPopUpMenu::prevRect;
RECT CPopUpMenu::totalRect;

void CPopUpMenu::drawPopUpMenu(CDC* dc)
{
    m_dc = dc;

    int sDC = m_dc->SaveDC();

    POINT p = m_origin;
    populateMenu();
    for (auto& menuItem : m_listElements) {
       drawElement(menuItem, p);
       p.y -= 22;
    }

    RECT MB3MenuBack;
    MB3MenuBack.left = m_origin.x;
    MB3MenuBack.bottom = m_origin.y;
    MB3MenuBack.top = p.y;
    MB3MenuBack.right = m_origin.x + 100;

    CopyRect(&CPopUpMenu::totalRect, &MB3MenuBack);

    COLORREF pcolortl = RGB(140, 140, 140);
    COLORREF pcolorbr = RGB(55, 55, 55);
    m_dc->Draw3dRect(&MB3MenuBack, pcolortl, pcolorbr);

    m_dc->RestoreDC(sDC);
    
}

void CPopUpMenu::highlightSelection(CDC* dc, RECT rect) {
    
    m_dc = dc;

    int sDC = m_dc->SaveDC();

    COLORREF pcolortl = RGB(140, 140, 140);
    COLORREF pcolorbr = RGB(55, 55, 55);
    m_dc->Draw3dRect(&rect, pcolortl, pcolorbr);

    m_dc->RestoreDC(sDC);
    CopyRect(&CPopUpMenu::prevRect, &rect);

}

void CPopUpMenu::populateMenu()
{
    string autoHOTarget = "H/O";
    autoHOTarget += " -> ";
    autoHOTarget += m_rad->GetPlugIn()->ControllerSelect(m_rad->GetPlugIn()->FlightPlanSelect(m_fp->GetCallsign()).GetCoordinatedNextController()).GetPositionId();
    this->m_listElements.emplace_back(SPopUpElement("Mod SFI", 0, 1));
    this->m_listElements.emplace_back(SPopUpElement("Flt. Plan", 0, 1));
    this->m_listElements.emplace_back(SPopUpElement("H/O -> CJS", 0, 1));
    this->m_listElements.emplace_back(SPopUpElement(autoHOTarget, 0, 0));
    this->m_listElements.emplace_back(SPopUpElement(this->m_fp->GetCallsign(), 1, 0));

}

void CPopUpMenu::drawElement(SPopUpElement& element, POINT p) {
    int sDC = m_dc->SaveDC();

    CFont font;
    LOGFONT lgfont;

    memset(&lgfont, 0, sizeof(LOGFONT));
    lgfont.lfWeight = 700;
    strcpy_s(lgfont.lfFaceName, _T("Segoe UI"));
    lgfont.lfHeight = 14;
    font.CreateFontIndirect(&lgfont);

    m_dc->SelectObject(font);
    m_dc->SetTextColor(RGB(230, 230, 230));

    //default is unpressed state
    COLORREF pressedcolor = RGB(66, 66, 66);
    COLORREF pcolortl = RGB(140, 140, 140);
    COLORREF pcolorbr = RGB(55, 55, 55);

    COLORREF targetPenColor = RGB(66, 66, 66);
    HPEN targetPen = CreatePen(PS_SOLID, 1, targetPenColor);
    HBRUSH targetBrush = CreateSolidBrush(pressedcolor);

    m_dc->SelectObject(targetPen);
    m_dc->SelectObject(targetBrush);

    // button rectangle
    RECT rect1;
    rect1.left = p.x;
    rect1.right = p.x + 100;
    rect1.top = p.y - 22;
    rect1.bottom = p.y;

    element.elementRect = rect1;
    // 3d apperance calls
    m_dc->Rectangle(&rect1);
    if (element.m_isHeader) {
        m_dc->Draw3dRect(&rect1, pcolortl, pcolorbr);
    }

    if (element.m_isHeader) {
        m_dc->DrawText(element.m_text.c_str(), &rect1, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
    }
    else {
        rect1.left += 10;
        m_dc->DrawText(element.m_text.c_str(), &rect1, DT_LEFT | DT_SINGLELINE | DT_VCENTER);
    }

    CSiTRadar::m_pRadScr->AddScreenObject(BUTTON_MENU_RMB_MENU, "Test", rect1, true, "Test");
        //>AddScreenObject(BUTTON_MENU_RMB_MENU, element.m_text.c_str(), rect1, false, "");

    DeleteObject(targetPen);
    DeleteObject(targetBrush);
    DeleteObject(font);

    m_dc->RestoreDC(sDC);
}