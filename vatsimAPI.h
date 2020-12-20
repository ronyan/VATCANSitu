#pragma once

#include "EuroScopePlugIn.h"
#include "CSiTRadar.h"

struct CAsync {
	CPlugIn* Plugin;
};

class CDataHandler

{
public:
	static void GetVatsimAPIData();

	static void CDataHandler::GetVatsimAPIData(void* args);
	static void CDataHandler::AmendFlightPlans(void* args);

	static string url1;
	static int CDataHandler::refreshInterval;
	static string CDataHandler::tagLabel;
	static string CDataHandler::vatsimJson3URL;

};