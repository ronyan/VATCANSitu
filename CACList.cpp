#include "pch.h"
#include "CACList.h"
#include "CFontHelper.h"

unsigned long CACListItem::m_list_IDs_; 
unsigned int CACListItemElement::m_list_item_element_IDs_;

CACList::CACList() {

}

void CACList::DrawList()
{
	int sDC = m_dc->SaveDC();
	m_dc->SetTextColor(C_WHITE);
	m_dc->SelectObject(CFontHelper::Euroscope14);

	string header = m_header;

	RECT listHeading{};
	listHeading.left = origin.x;
	listHeading.top = origin.y;

	// Draw the arrow
	HPEN targetPen = CreatePen(PS_SOLID, 1, C_WHITE);;
	HBRUSH targetBrush = CreateSolidBrush(C_WHITE);

	m_dc->SelectObject(targetPen);
	m_dc->SelectObject(targetBrush);

	m_dc->DrawText(header.c_str(), &listHeading, DT_LEFT | DT_CALCRECT);
	m_dc->DrawText(header.c_str(), &listHeading, DT_LEFT);

	if (m_list_items_.empty()) {
		m_has_arrow = false;
	}

	int i = 0;
	for (auto &entry : m_list_items_) {
		entry.m_origin.y = listHeading.bottom;
		if (entry.m_priority == 3) { m_dc->SetTextColor(C_MENU_GREY4); }

		if (!this->m_collapsed) {
			for (auto& elem : entry.m_list_elements_) {

				RECT listElement{ entry.m_origin.x, entry.m_origin.y + (i*13), entry.m_origin.x + 10, entry.m_origin.y + 13 + (i*13) };
				m_dc->DrawText(elem.m_list_element_text.c_str(), &listElement, DT_LEFT | DT_CALCRECT);
				m_dc->DrawText(elem.m_list_element_text.c_str(), &listElement, DT_LEFT);


			}
		}
		i++;
		m_has_arrow = true;
	}

	if (m_has_arrow) {
		POINT vertices[] = { {listHeading.right + 5, listHeading.top + 3}, {listHeading.right + 15, listHeading.top + 3} ,  {listHeading.right + 10, listHeading.top + 10} };
		if (!m_collapsed)
		{
			vertices[0] = { listHeading.right + 5, listHeading.top + 10 };
			vertices[1] = { listHeading.right + 15, listHeading.top + 10 };
			vertices[2] = { listHeading.right + 10, listHeading.top + 3 };
		}
		m_dc->Polygon(vertices, 3);
	}

	DeleteObject(targetBrush);
	DeleteObject(targetPen);
}
