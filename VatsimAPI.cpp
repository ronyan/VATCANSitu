#include "pch.h"
#include "vatsimAPI.h"
#include "SituPlugin.h"
#include <iostream>
#include <fstream>
#include "ACTag.h"
#include "json.hpp"
#include "curl/curl.h"

// Include dependency
using json = nlohmann::json;

static size_t write_data(void* buffer, size_t size, size_t nmemb, void* userp) {
	((std::string*)userp)->append((char*)buffer, size * nmemb);
	return size * nmemb;
}


int CDataHandler::GetVatsimAPIData(CPlugIn* plugin, CSiTRadar* radscr) {

	// Parse CID data from the VATSIM API

	string responseString;

	CURL* curl = curl_easy_init();
	if (curl)
	{
		curl_easy_setopt(curl, CURLOPT_URL, "http://cluster.data.vatsim.net/vatsim-data.json");
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseString);
		CURLcode res;
		res = curl_easy_perform(curl);
		curl_easy_cleanup(curl);
	}

	try {

		// Now we parse the json
		auto jsonArray = json::parse(responseString);

		if (!jsonArray["clients"].empty()) {
			for (auto& array : jsonArray["clients"]) {
				string apiCallsign = array["callsign"];
				string apiCID = array["cid"];

				radscr->pilotCID[apiCallsign] = apiCID;
			}
		}

		string timeStamp = jsonArray["general"]["update_timestamp"];

		// Everything succeeded, show to user
		plugin->DisplayUserMessage("VATCAN Situ", "Update Successful", string("VATSIM Data Successfully Loaded at " + timeStamp).c_str(), true, false, false, false, false);
		return 0;
	}
	catch (exception& e) {
		plugin->DisplayUserMessage("VATCAN Situ", "Error", string("Failed to parse CID data" + string(e.what())).c_str(), true, true, true, true, true);
		return 1;
	}

	// Parse the list from CZQO of CIDs with CTP slots
}