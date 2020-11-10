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

	// Parse list of CTP CIDs

	string cidString;
	json cidJson;

	CURL* curl = curl_easy_init();
	CURL* curl1 = curl_easy_init();
	if (curl)
	{
		curl_easy_setopt(curl, CURLOPT_URL, "https://ganderoceanicoca.ams3.cdn.digitaloceanspaces.com/resources/data/ctpCID.json");
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &cidString);
		CURLcode res;
		res = curl_easy_perform(curl);
		curl_easy_cleanup(curl);
	}

	try {

		// Now we parse the json
		cidJson = json::parse(cidString);

		// Everything succeeded, show to user
		plugin->DisplayUserMessage("VATCAN Situ", "Update Successful", string("CTP CIDs Updated").c_str(), true, false, false, false, false);

	}
	catch (exception& e) {
		plugin->DisplayUserMessage("VATCAN Situ", "Error", string("Failed to parse CID data" + string(e.what())).c_str(), true, true, true, true, true);

	}

	// Parse CID data from the VATSIM API

	string responseString;

	if (curl1)
	{
		curl_easy_setopt(curl1, CURLOPT_URL, "http://cluster.data.vatsim.net/vatsim-data.json");
		curl_easy_setopt(curl1, CURLOPT_WRITEFUNCTION, write_data);
		curl_easy_setopt(curl1, CURLOPT_WRITEDATA, &responseString);
		CURLcode res;
		res = curl_easy_perform(curl1);
		curl_easy_cleanup(curl1);
	}

	try {

		// Now we parse the json
		auto jsonArray = json::parse(responseString);

		if (!jsonArray["clients"].empty()) {
			for (auto& array : jsonArray["clients"]) {
				string apiCallsign = array["callsign"];
				string apiCID = array["cid"];

				for (auto& pilots : cidJson["pilots"]) {
					if (pilots["cid"] == apiCID) {
						CSiTRadar::mAcData[array["callsign"]].hasCTP = TRUE;
						CSiTRadar::mAcData[array["callsign"]].slotTime = pilots["slot"];
					}
				}
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
}