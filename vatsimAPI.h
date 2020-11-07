#pragma once

#include "EuroScopePlugIn.h"
#include "CSiTRadar.h"

class CDataHandler

{
public:
	static int CDataHandler::GetVatsimAPIData(CPlugIn* plugin, CSiTRadar* radscr);
};