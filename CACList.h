#pragma once
#include <vector>
#include <string>
#include "SituPlugin.h"
#include "constants.h"

class CACListItemElement {
public:
	static unsigned int m_list_item_element_IDs_;
	int m_list_ID_{};
	int m_list_element_ID{};
	std::string m_list_element_text;
	COLORREF m_list_item_color{};
	RECT list_item_element_rect{};
	int list_x_loc{ 0 };

	CACListItemElement(int listElementType, std::string text) {
		m_list_element_ID = m_list_item_element_IDs_;
		m_list_item_element_IDs_++;

		if (listElementType == LIST_ITEM_SIMPLE_STRING) {
			m_list_element_text = text;
		}
	};
};

class CACListItem {
public:
	static unsigned long m_list_IDs_;
	POINT m_origin;
	int m_list_ID_;
	int m_list_item_line;
	std::vector<CACListItemElement> m_list_elements_;
	bool shifted;
	RECT list_item_rect;

	CACListItem();

	// Simple string list overload: i.e. for message list;
	CACListItem(std::string str) {
		m_list_ID_ = m_list_IDs_;

		m_list_elements_.push_back(CACListItemElement(LIST_ITEM_SIMPLE_STRING, str));
		m_list_IDs_++;
	}

	void PopulateListItem() {
	}
};

class CACList
{
public:
	CDC* m_dc;
	int m_listType;
	POINT origin;
	int m_list_ID;
	bool m_has_arrow{ false };
	bool m_collapsed{ false };
	std::string m_header;
	std::vector<CACListItem> m_list_items_;

	CACList();
	CACList(CDC* dc, int listType) {
		m_dc = dc;
		m_listType = listType;
	}
	void PopulatetList(std::vector<std::string> listContents) {

		for (auto le : listContents) {
			m_list_items_.push_back(CACListItem(le));
		}

	}
	void DrawList();
	
};
