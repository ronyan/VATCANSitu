#pragma once

#include "EuroScopePlugIn.h"
#include "CSiTRadar.h"

struct CAsync {
	CPlugIn* Plugin;
};

class CDataHandler

{
public:
	static void CDataHandler::GetVatsimAPIData(void* args);
};