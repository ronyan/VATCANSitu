#include "pch.h"
#include "cpdlc.h"

static std::size_t write_data(void* buffer, std::size_t size, std::size_t nmemb, void* userp) {
	((std::string*)userp)->append((char*)buffer, size * nmemb);
	return size * nmemb;
};

u_int CPDLCMessage::ids = 0;
std::string CPDLCMessage::hoppieCode = "";
std::string CPDLCMessage::hoppieICAO = "CYYZ";
bool CPDLCMessage::firstPeek = true;

std::string CPDLCMessage::YYMMDDString() {
	// Get the current time in UTC (Zulu time) using std::chrono
	auto currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

	// Buffer to store the result of gmtime_s
	std::tm timeInfoUTC;

	// Use gmtime_s to convert the time to a std::tm structure in UTC
	if (gmtime_s(&timeInfoUTC, &currentTime) != 0) {
		// Handle error (you may want to throw an exception or handle it appropriately)
		return "Error in gmtime_s";
	}

	// Extract year, month, and day components
	int year = timeInfoUTC.tm_year % 100; // Last two digits of the year
	int month = timeInfoUTC.tm_mon + 1;   // Month (0-based)
	int day = timeInfoUTC.tm_mday;        // Day of the month

	// Format the date as "YYMMDD"
	std::ostringstream oss;
	oss << std::setfill('0') << std::setw(2) << year
		<< std::setw(2) << month
		<< std::setw(2) << day;

	// Store the formatted date as a string
	std::string dateString = oss.str();
	return dateString;
}

std::string CPDLCMessage::FreqTruncate(double freq) {

	// Convert the double to a string with fixed precision
	std::ostringstream oss;
	oss << std::fixed << std::setprecision(3) << freq;
	std::string result = oss.str();

	// Remove trailing zeros and the decimal point if unnecessary
	size_t decimalPointPos = result.find_last_of('.');
	result.erase(result.find_last_not_of('0') + 1);
	if (decimalPointPos == result.size() - 1) {
		result.pop_back();  // Remove the trailing decimal point
	}

	return result;
}

CPDLCMessage::CPDLCMessage() {

	this->id = CPDLCMessage::ids; // assign an internal id to every message
	CPDLCMessage::ids++;

}

CPDLCMessage::~CPDLCMessage() {}

CPDLCMessage CPDLCMessage::parseDLMessage(std::string& rawMessage) { // breaks up rawstring and returns a CPDLCMessage object with each message

	CPDLCMessage parsedMessage;


	if (rawMessage.length() > 3) {

		auto currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		parsedMessage.timeParsed = currentTime;

		// UL messages are stored at creation, so downloaded messages are always DL
		parsedMessage.isdlMessage = true;
		parsedMessage.receipient = CPDLCMessage::hoppieICAO;

		size_t firstBrace = rawMessage.find('{');
		size_t secondBrace = rawMessage.find('}', firstBrace + 1) + 1;
		size_t substringStart = 0;
		size_t substringLength = 0;

		if (firstBrace != std::string::npos && secondBrace != std::string::npos) {
			substringStart = firstBrace + 1;
			substringLength = secondBrace - substringStart;
		}

		std::string result = rawMessage.substr(substringStart, substringLength);

		// the chopped message is returned by reference and truncated
		rawMessage = rawMessage.substr(secondBrace);

		// substring the hoppie messageID ** this only shows in PEEK **

		if (firstPeek) {
			size_t space = result.find(' ');
			if (space != std::string::npos) {
				parsedMessage.hoppieMessageID = result.substr(0, space);
				result = result.substr(space + 1);
			}
		}

		// get the callsign
		size_t space = result.find(' ');
		if (space != std::string::npos) {
			parsedMessage.sender = result.substr(0, space);
			result = result.substr(space+1);
		}
		// get the type
		space = result.find(' ');
		if (space != std::string::npos) {
			parsedMessage.messageType = result.substr(0, space);
			result = result.substr(space+1);
		}

		if (parsedMessage.messageType == "telex") {

			if (result.length() >= 2) {
				parsedMessage.rawMessageContent = result.substr(1, result.length()-2);
			}
			else {
				parsedMessage.rawMessageContent = "";
			}

		}

		// CPDLC Messages are in this format: {10220055 DLH2RQ cpdlc {/data2/2/19/N/WILCO}}
		// DL messages are coded as "Need response from ATC?" Yes = 'Y' No = 'N'
		// if response is needed, then when printing, can link this to an UP message

		if (parsedMessage.messageType == "cpdlc") {

			std::stringstream ss(result);
			std::string token;
			std::vector<std::string> components;

			// Split the string using '/'
			try {
				while (std::getline(ss, token, '/')) {
					components.push_back(token);
				}
			}
			catch (std::exception &e) {

			}

			if (!components.at(2).empty()) {
				parsedMessage.messageID = stoi(components.at(2));
			}
			if (!components.at(3).empty()) {
				parsedMessage.responseToMessageID = stoi(components.at(3));
			}
			parsedMessage.responseRequired = components.at(4);
			if (components.at(5).length() > 1) {
				parsedMessage.rawMessageContent = components.at(5);
				parsedMessage.rawMessageContent.pop_back();
			}

			int a = 0;
		}

	}

	return parsedMessage;
}

std::string CPDLCMessage::PollCPDLCMessages() { // Returns raw string of CPDLC messages; Should be called every 50-70s to get new messages
	std::string url;
	url = "http://www.hoppie.nl/acars/system/connect.html?logon=" + CPDLCMessage::hoppieCode + "&from=" + CPDLCMessage::hoppieICAO + "&to=SERVER";

	if (CPDLCMessage::firstPeek) {
		url += "&type=peek";
	} else { 
		url += "&type=poll";
	}

	//url = "https://ronyan.github.io/hoppie-html-test-cases/";

	std::string rawHoppiePollString;

	auto pollCurl = curl_easy_init();

	curl_easy_setopt(pollCurl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(pollCurl, CURLOPT_WRITEFUNCTION, write_data);
	curl_easy_setopt(pollCurl, CURLOPT_WRITEDATA, &rawHoppiePollString);

	auto res = curl_easy_perform(pollCurl);
	curl_easy_cleanup(pollCurl);

	return rawHoppiePollString;

}

void CPDLCMessage::SendCPDLCMessage() {

	std::string url;
	url = "http://www.hoppie.nl/acars/system/connect.html";
	//url = "https://ronyan.github.io/hoppie-html-test-cases/";

	std::string postfields = "logon=";
	postfields += this->hoppieCode;
	postfields += "&from=";
	postfields += this->hoppieICAO;
	postfields += "&to=";
	postfields += this->receipient;

	std::string curlFriendlyRawMsg;

	for (char character : this->rawMessageContent) {
		if (character == ' ') {
			curlFriendlyRawMsg += "%20";
		}
		else {
			curlFriendlyRawMsg += character;
		}
	}

	if (this->messageType == "cpdlc") {
		postfields += "&type=cpdlc";
		postfields += "&packet=/data2/";
		postfields += std::to_string(this->messageID);
		postfields += "/";
		postfields += std::to_string(this->responseToMessageID);
		postfields += "/";
		postfields += this->responseRequired;
		postfields += "/";
		postfields += this->rawMessageContent;
	}

	if (this->messageType == "telex") {
		postfields += "&type=telex";
		postfields += "&packet=";
		postfields += curlFriendlyRawMsg;
	}

	std::string postResponse;

	auto postCurl = curl_easy_init();

	curl_easy_setopt(postCurl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(postCurl, CURLOPT_POSTFIELDS, postfields.c_str());
	curl_easy_setopt(postCurl, CURLOPT_WRITEFUNCTION, write_data);
	curl_easy_setopt(postCurl, CURLOPT_WRITEDATA, &postResponse);
	auto res = curl_easy_perform(postCurl);
	curl_easy_cleanup(postCurl);

	if (res != CURLE_OK) {
		this->sent = false;
	}
	else {
		if (postResponse.substr(0, 2) == "ok") { this->sent = true; }
	}

}

void CPDLCMessage::GenerateReply(CPDLCMessage originalMessage) {

	this->receipient = originalMessage.sender;
	this->responseToMessageID = originalMessage.messageID;
	this->isdlMessage = !this->isdlMessage; // replies are the opposite type

}

void CPDLCMessage::MakePDCMessage(EuroScopePlugIn::CFlightPlan& flightplan, EuroScopePlugIn::CController& controller, std::string atisLetter) {

	this->sender = this->hoppieICAO;
	this->receipient = flightplan.GetCallsign();

	// Generate some hash number to generate a PDC identifier
	std::hash<std::string> hashFunction;
	size_t seedValue = hashFunction(this->receipient);

	// Convert the hash value to an integer (or use it directly)
	unsigned int seedInt = static_cast<unsigned int>(seedValue);

	char letter = 'A'; // filler letter for now
	std::string identifierLetter;
	u_int pdcNumbers = (static_cast<u_int>(this->timeParsed) + seedInt) % 900 + 100; // 3 digit number, generate with hoppie ID
	letter += (static_cast<u_int>(this->timeParsed) + seedInt) % 25;
	identifierLetter = letter;


	// truncate the route if too long
	std::string rteStr = flightplan.GetFlightPlanData().GetRoute();
	int nCharacters = 60;

	if (rteStr.length() > nCharacters) {

		int index = nCharacters;
		std::string substringBeforeSpace;
		while (index < rteStr.length()) {
			if (rteStr[index] == ' ') {
				// Extract substring before the space
				substringBeforeSpace = rteStr.substr(0, index);
				break;
			}
			index++;
		}

		rteStr = substringBeforeSpace + "// FILED ROUTE";
	}
	
	if (flightplan.GetFlightPlanData().GetOrigin() == "CYTZ") {

		this->responseRequired = "WU";
	
		this->rawMessageContent = "CLD "; // Generate the PDC string;
		this->rawMessageContent += ZuluTimeStringGen();
		this->rawMessageContent += " ";
		this->rawMessageContent += YYMMDDString();
		this->rawMessageContent += " ";
		this->rawMessageContent += flightplan.GetFlightPlanData().GetOrigin();
		this->rawMessageContent += " PDC ";
		this->rawMessageContent += std::to_string(pdcNumbers);
		this->rawMessageContent += " ";
		this->rawMessageContent += flightplan.GetCallsign();
		this->rawMessageContent += " CLRD TO @";
		this->rawMessageContent += flightplan.GetFlightPlanData().GetDestination();
		this->rawMessageContent += "@ OFF @";
		this->rawMessageContent += flightplan.GetFlightPlanData().GetDepartureRwy();
		this->rawMessageContent += "@ VIA @";
		this->rawMessageContent += flightplan.GetFlightPlanData().GetSidName();
		this->rawMessageContent += "@ SQUAWK @";
		this->rawMessageContent += flightplan.GetControllerAssignedData().GetSquawk();
		this->rawMessageContent += "@ ATIS @";
		this->rawMessageContent += atisLetter;
		this->rawMessageContent += "@ CONTACT @";
		this->rawMessageContent += controller.GetCallsign();
		this->rawMessageContent += "@ ON FREQ @";
		this->rawMessageContent += FreqTruncate(controller.GetPrimaryFrequency());
		this->rawMessageContent += "@";
	
		this->messageType = "cpdlc";

	}

	// ARINC 622 

	// CPDLC only available in CYTZ and CYYZ in CZYZ if someone requests from elsewhere send
	// ARINC 620/622 simulate a telex message
	/* -// ATC PA01 YYZOWAC 22JUN/1003 C-FITW/733/AC7281

		TIMESTAMP 22JUN21 10:03
		*PRE-DEPARTURE CLEARANCE*
		FLT ACA7281 CYYZ
		H/B77W/W FILED FL360
		XPRD 2264
		USE SID AVSEP6
		DEPARTURE RUNWAY 33R
		DESTINATION CYVR
		*** ADVISE ATC IF RUNUP REQUIRED ***
		CONTACT CLEARANCE WITH IDENTIFIER 360M
		AVSEP6 MUSIT SSM YQT GERTY
		PEMPA AXILI BOOTH CANUC5
		END

		*/

	else {

		this->rawMessageContent = "TIMESTAMP ";
		this->rawMessageContent += ZuluTimeStringGen();
		this->rawMessageContent += " @*PRE-DEPARTURE CLEARANCE* @FLT ";
		this->rawMessageContent += flightplan.GetCallsign();
		this->rawMessageContent += " ";
		this->rawMessageContent += flightplan.GetFlightPlanData().GetOrigin();
		this->rawMessageContent += " @";
		this->rawMessageContent += flightplan.GetFlightPlanData().GetAircraftFPType();
		this->rawMessageContent += " FILED";
		if (flightplan.GetFlightPlanData().GetFinalAltitude() > 18000) {
			this->rawMessageContent += " FL";
			this->rawMessageContent += std::to_string(flightplan.GetFlightPlanData().GetFinalAltitude()/100);
		}
		else {
			this->rawMessageContent += std::to_string(flightplan.GetFlightPlanData().GetFinalAltitude());
		}
		this->rawMessageContent += " @XPRD ";
		this->rawMessageContent += flightplan.GetControllerAssignedData().GetSquawk();
		this->rawMessageContent += " @USE SID ";
		this->rawMessageContent += flightplan.GetFlightPlanData().GetSidName();
		this->rawMessageContent += " @DEPARTURE RUNWAY ";
		this->rawMessageContent += flightplan.GetFlightPlanData().GetDepartureRwy();
		this->rawMessageContent += " @DESTINATION ";
		this->rawMessageContent += flightplan.GetFlightPlanData().GetDestination();
		this->rawMessageContent += " @*** ADVISE ATC IF RUNUP REQUIRED *** ";
		this->rawMessageContent += "@CONTACT CLEARANCE WITH IDENTIFIER ";
		this->rawMessageContent += std::to_string(pdcNumbers);
		this->rawMessageContent += identifierLetter;
		this->rawMessageContent += " @";
		this->rawMessageContent += rteStr;
		this->rawMessageContent += " @END";
		
		this->messageType = "telex";

	}

}

void CPDLCMessage::processMessage() { // should loop with every Poll try resending messages or automatically generate responses where appropriate




	if (!this->isdlMessage) {
		if (!this->sent) {

			// try resending UL messages if failed at last push;
			this->SendCPDLCMessage();
			this->sent = true;

		}

	}

	// Put garbage cleaning;

}

std::string CPDLCMessage::ZuluTimeStringGen() {

	auto currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

	// Buffer to store the result of gmtime_s
	std::tm timeInfoUTC;

	// Use gmtime_s to convert the time to a std::tm structure in UTC
	if (gmtime_s(&timeInfoUTC, &currentTime) != 0) {
		// Handle error (you may want to throw an exception or handle it appropriately)
		return "Error in gmtime_s";
	}

	// Format the time as "HHMM"
	std::ostringstream oss;
	oss << std::setfill('0') << std::setw(2) << timeInfoUTC.tm_hour
		<< std::setw(2) << timeInfoUTC.tm_min;

	// Store the formatted time as a string
	std::string zuluTimeString = oss.str();
	return zuluTimeString;
}

bool CPDLCMessage::isValidDLMessage() {

	std::vector<std::regex> acceptableMessages = {



	};

	bool isMatch = false;
	for (const auto& pattern : acceptableMessages) {
		if (std::regex_match(this->rawMessageContent, pattern)) {
			return true;
		}
	}
	return false;
}

/* enum CpdlcMessageExpectedResponseType {
	NotRequired = 'NE',
	WilcoUnable = 'WU',
	AffirmNegative = 'AN',
	Roger = 'R',
	No = 'N',
	Yes = 'Y'
}
*/