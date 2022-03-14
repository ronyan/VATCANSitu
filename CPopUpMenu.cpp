#include "pch.h"
#include "CPopUpMenu.h"

RECT CPopUpMenu::prevRect;
RECT CPopUpMenu::totalRect;

void CPopUpMenu::drawPopUpMenu(CDC* dc)
{
    m_dc = dc;
    int width = 0;
    int sDC = m_dc->SaveDC();

    POINT p = m_origin;
    
    for (auto& menuItem : m_listElements) {
       drawElement(menuItem, p);
       p.y -= 20;
       if (width !=  menuItem.m_width) { width = menuItem.m_width; }
    }

    RECT MB3MenuBack;
    MB3MenuBack.left = m_origin.x;
    MB3MenuBack.bottom = m_origin.y;
    MB3MenuBack.top = p.y;
    MB3MenuBack.right = m_origin.x + width;

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
    if (m_fp->GetTrackingControllerIsMe()) {
        this->m_listElements.emplace_back(SPopUpElement("Controller Remarks", "CtrlRemarks", 0, 0));
        this->m_listElements.emplace_back(SPopUpElement("DE Corr", "Decorrelate", 0, 0));
        if (CSiTRadar::mAcData[m_fp->GetCallsign()].pointOutFromMe) {
            this->m_listElements.emplace_back(SPopUpElement("Recall P/Out", "RecallPointOut", 0, 0));
        }
        else {
            this->m_listElements.emplace_back(SPopUpElement("Point Out", "PointOut", 0, 1));
        }
        this->m_listElements.emplace_back(SPopUpElement("Direct To", "DirectTo", 0, 0));
        this->m_listElements.emplace_back(SPopUpElement("Mod SFI", "ModSFI", 0, 1));
        this->m_listElements.emplace_back(SPopUpElement("Comm. Type", "SetComm", 0, 1));
    }
    if (CSiTRadar::mAcData[m_fp->GetCallsign()].pointOutPendingApproval) {
        this->m_listElements.emplace_back(SPopUpElement("Accept P/Out", "AcceptPointOut", 0, 0));
    }
    this->m_listElements.emplace_back(SPopUpElement("Flight Plan", "FltPlan", 0, 0));
    if (m_fp->GetTrackingControllerIsMe()) {
        if (strcmp(m_fp->GetHandoffTargetControllerId(), "")) {
            this->m_listElements.emplace_back(SPopUpElement("H/O Recall", "AssumeTrack", 0, 0));
        }
        else {
            this->m_listElements.emplace_back(SPopUpElement("H/O -> CJS", "ManHandoff", 0, 1));
            if (strcmp(autoHOTarget.c_str(), "H/O -> ")) {
                this->m_listElements.emplace_back(SPopUpElement(autoHOTarget, "AutoHandoff", 0, 0));
            }
        }
        this->m_listElements.emplace_back(SPopUpElement("Release", "DropTrack", 0, 0));
    }
    else {
        if (!strcmp(m_fp->GetTrackingControllerId(), "")) {
            this->m_listElements.emplace_back(SPopUpElement("Take Jurisdiction", "AssumeTrack", 0, 0));
        }
        this->m_listElements.emplace_back(SPopUpElement("DE Corr", "Decorrelate", 0, 0));
    }


    this->m_listElements.emplace_back(SPopUpElement(this->m_fp->GetCallsign(), this->m_fp->GetCallsign(), 1, 0));
    
}

void CPopUpMenu::populateSecondaryMenu(string type) {
    if (!strcmp(type.c_str(), "ManHandoff") || !strcmp(type.c_str(), "PointOut")) {

        std::map<string, bool>::reverse_iterator it;
        this->m_listElements.emplace_back(SPopUpElement("EXP", "EXP", 2, 0, 40));
        for (it = CSiTRadar::menuState.nearbyCJS.rbegin(); it != CSiTRadar::menuState.nearbyCJS.rend(); it++) {
            this->m_listElements.emplace_back(SPopUpElement(it->first, it->first, 0, 0, 40));
        }
        this->m_listElements.emplace_back(SPopUpElement("CJS", "CJS", 1, 0, 40));
    }
    if (!strcmp(type.c_str(), "ModSFI")) {
        string sfi = CSiTRadar::menuState.SFIPrefString;
        this->m_listElements.emplace_back(SPopUpElement("EXP", "EXP", 0, 0, 40));
        this->m_listElements.emplace_back(SPopUpElement("CLR", "CLR", 2, 0, 40));
        std::reverse(sfi.begin(), sfi.end());
        for (char& c : sfi) {
            string letter;
            letter = c;
            this->m_listElements.emplace_back(SPopUpElement(letter, letter, 0, 0, 40));
        }
    }
    if (!strcmp(type.c_str(), "SetComm")) {
        this->m_listElements.emplace_back(SPopUpElement("V", "V", 0, 0, 40));
        this->m_listElements.emplace_back(SPopUpElement("R", "R", 0, 0, 40));
        this->m_listElements.emplace_back(SPopUpElement("T", "T", 0, 0, 40));
    }
    if (!this->m_listElements.empty()) {
        m_width_ = m_listElements.at(0).m_width;
    }
}

void CPopUpMenu::drawElement(SPopUpElement& element, POINT p) {
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
    rect1.right = p.x + element.m_width;
    rect1.top = p.y - 20;
    rect1.bottom = p.y;

    element.elementRect = rect1;
    // 3d apperance calls
    m_dc->Rectangle(&rect1);
    if (element.m_isHeaderFooter == 1) {
        m_dc->Draw3dRect(&rect1, pcolortl, pcolorbr);
    }
    else if (element.m_isHeaderFooter == 2)
    {
        m_dc->Draw3dRect(&rect1, pcolorbr, pressedcolor);
    }

    if (element.m_isHeaderFooter == 1) {
        m_dc->DrawText(element.m_text.c_str(), &rect1, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
    }
    else {
        rect1.left += 5;
        m_dc->DrawText(element.m_text.c_str(), &rect1, DT_LEFT | DT_SINGLELINE | DT_VCENTER);
    }

    if (element.m_hasArrow) {
        HPEN targetPentl = CreatePen(PS_SOLID, 1, pcolortl);
        HPEN targetPenbr = CreatePen(PS_SOLID, 1, pcolorbr);
        m_dc->SelectObject(targetPentl);

        m_dc->MoveTo({ rect1.right - 6, rect1.top +9 });
        m_dc->LineTo({ rect1.right - 12, rect1.top + 6 });
        m_dc->LineTo({ rect1.right - 12, rect1.top + 14 });
        m_dc->SelectObject(targetPenbr);
        m_dc->LineTo({ rect1.right - 5, rect1.top + 12 });

        DeleteObject(targetPentl);
        DeleteObject(targetPenbr);
        
    }

    DeleteObject(targetPen);
    DeleteObject(targetBrush);
    DeleteObject(font);

    m_dc->RestoreDC(sDC);
}