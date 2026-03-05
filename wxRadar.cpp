#include "pch.h"
#include "wxRadar.h"

cell wxRadar::wxReturn[256][256];
string wxRadar::wxLatCtr = { "0.0" };
string wxRadar::wxLongCtr = { "0.0" };
int wxRadar::zoomLevel;
string wxRadar::ts;
std::map<string, string> wxRadar::arptAltimeter;
std::map<string, string> wxRadar::arptAtisLetter;
std::vector<CAsyncResponse> wxRadar::asyncMessages;
std::shared_mutex wxRadar::altimeterMutex;
std::shared_mutex wxRadar::atisLetterMutex;
json wxRadar::jsVatsimDataFeed;
std::unordered_map<uint32_t, int> wxRadar::colorDbzMap;

void wxRadar::initColorDbzMap() {
    colorDbzMap = {
        { 0x88DDEE, 15 }, { 0x6CD1EB, 16 }, { 0x51C5E8, 17 }, { 0x36BAE5, 18 },
        { 0x1BAEE2, 19 }, { 0x00A3E0, 20 }, { 0x009AD5, 21 }, { 0x0091CA, 22 },
        { 0x0088BF, 23 }, { 0x007FB4, 24 }, { 0x0077AA, 25 }, { 0x0070A3, 26 },
        { 0x00699C, 27 }, { 0x006295, 28 }, { 0x005B8E, 29 }, { 0x005588, 30 },
        { 0x005180, 31 }, { 0x004E78, 32 }, { 0x004A70, 33 }, { 0x004768, 34 },
        { 0xFFEE00, 35 }, { 0xFFE000, 36 }, { 0xFFD200, 37 }, { 0xFFC500, 38 },
        { 0xFFB700, 39 }, { 0xFFAA00, 40 }, { 0xFF9F00, 41 }, { 0xFF9500, 42 },
        { 0xFF8B00, 43 }, { 0xFF8100, 44 }, { 0xFF4400, 45 }, { 0xF23600, 46 },
        { 0xE62800, 47 }, { 0xD91B00, 48 }, { 0xCD0D00, 49 }, { 0xC10000, 50 },
        { 0xA80000, 51 }, { 0x8F0000, 52 }, { 0x760000, 53 }, { 0x5D0000, 54 },
        { 0xFFAAFF, 55 }, { 0xFF9FFF, 56 }, { 0xFF95FF, 57 }, { 0xFF8BFF, 58 },
        { 0xFF81FF, 59 }, { 0xFF77FF, 60 }, { 0xFF6CFF, 61 }, { 0xFF62FF, 62 },
        { 0xFF58FF, 63 }, { 0xFF4EFF, 64 }, { 0xFFFFFF, 65 },
    };
}

int wxRadar::colorToDbz(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
    if (a == 0) return -32; // fully transparent = no precip
    uint32_t key = ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
    auto it = colorDbzMap.find(key);
    if (it != colorDbzMap.end()) return it->second;
    return -32; // unknown colour = no return
}

void wxRadar::loadPNG(std::vector<unsigned char>& buffer, const std::string& filename) //designed for loading files from hard disk in an std::vector
{
    std::ifstream file(filename.c_str(), std::ios::in | std::ios::binary | std::ios::ate);

    //get filesize
    std::streamsize size = 0;
    if (file.seekg(0, std::ios::end).good()) size = file.tellg();
    if (file.seekg(0, std::ios::beg).good()) size -= file.tellg();

    //read contents of the file into the vector
    if (size > 0)
    {
        buffer.resize((size_t)size);
        file.read((char*)(&buffer[0]), size);
    }
    else buffer.clear();
}

std::string wxRadar::getSituWxDir() {
    char dllPath[MAX_PATH];
    HMODULE hMod = NULL;
    GetModuleHandleExA(
        GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        (LPCSTR)&wxRadar::getSituWxDir,
        &hMod
    );
    GetModuleFileNameA(hMod, dllPath, MAX_PATH);
    std::string dir(dllPath);
    dir = dir.substr(0, dir.find_last_of("\\/"));
    return dir + "\\situWx\\";
}

void wxRadar::parseRadarPNG(CRadarScreen* rad) {

    GetRainViewerJSON(rad);

    std::string situWxDir = wxRadar::getSituWxDir();
    std::string situWxFile = situWxDir + "0_0.png";

    if (CreateDirectoryA(situWxDir.c_str(), NULL)) {}

    CURL* pngDL = curl_easy_init();
    FILE* dlPNG;
    errno_t err;

    string tileCacheurl = wxRadar::ts + "/256/4/" + wxRadar::wxLatCtr + "/" + wxRadar::wxLongCtr + "/1/0_0.png";

    const char* filename = situWxFile.c_str();
    curl_easy_setopt(pngDL, CURLOPT_URL, tileCacheurl.c_str());
    curl_easy_setopt(pngDL, CURLOPT_WRITEFUNCTION, write_file);

    err = fopen_s(&dlPNG, filename, "wb");
    if (err == 0) {

        /* write the page body to this file handle */
        curl_easy_setopt(pngDL, CURLOPT_WRITEDATA, dlPNG);

        /* get it! */
        curl_easy_perform(pngDL);

        if (dlPNG != NULL) {
            fclose(dlPNG);
        }
    }
    else {
        return;
    }

    /* cleanup curl stuff */
    curl_easy_cleanup(pngDL);


    std::vector<unsigned char> buffer, image;
    loadPNG(buffer, filename);
    unsigned long w, h;
    int error = wxRadar::decodePNG(image, w, h, buffer.empty() ? 0 : &buffer[0], (unsigned long)buffer.size());

    if (error != 0) {
        rad->GetPlugIn()->DisplayUserMessage("VATCAN Situ", "WX Parser", string("PNG Failed to Parse").c_str(), true, false, false, false, false);
    }
    else {

        // convert vector into 2d array with dBa values only;
        // png starts as RGBARGBARGBA... etc. 
        CPosition radReturnTL;

        radReturnTL.m_Longitude = stod(wxRadar::wxLongCtr) - 11.25000;
        radReturnTL.m_Latitude = stod(wxRadar::wxLatCtr);

        // get the pixel coord of the latitude.
        int yCoord = lat2pixel(radReturnTL.m_Latitude, 4);

        // get coord of the top of the image
        yCoord = yCoord - 128;

        for (int i = 0; i < 256; i++) {

            double pixLat = pixel2lat(yCoord + i, 4);
            // calculate latitdue for each row, use the pixel coordinate

            for (int j = 0; j < 256; j++) {

                unsigned char pr = image[(i * 256 * 4) + (j * 4) + 0];
                unsigned char pg = image[(i * 256 * 4) + (j * 4) + 1];
                unsigned char pb = image[(i * 256 * 4) + (j * 4) + 2];
                unsigned char pa = image[(i * 256 * 4) + (j * 4) + 3];
                wxReturn[j][i].dbz = colorToDbz(pr, pg, pb, pa);

                wxReturn[j][i].cellPos.m_Longitude = radReturnTL.m_Longitude + (double)(j * (22.5 / 256.0));
                wxReturn[j][i].cellPos.m_Latitude = pixLat;
            }
        }
    }
}

int wxRadar::renderRadar(Graphics* g, CRadarScreen* rad, bool showAllPrecip) {

    HatchBrush lightPrecipHatch(HatchStyleDarkUpwardDiagonal, Color(64, 0, 43, 255), Color(0, 0, 0, 0));
    HatchBrush heavyPrecipHatch(HatchStyleDarkUpwardDiagonal, Color(128, 0, 32, 255), Color(0, 0, 0, 0));

    int alldBZ = 28;
    int highdBZ = 48;

    CPosition pos1;
    CPosition pos2;
    CPosition pos3;
    CPosition pos4;

    Point defp1;
    Point defp4;

    bool deferDraw = false;

    // Render radar returns
    for (int i = 0; i < 255; i++) {
        for (int j = 0; j < 255; j++) {

            if (wxReturn[j][i].dbz >= highdBZ || (wxReturn[j][i].dbz >= alldBZ && showAllPrecip)) {

                POINT pix1 = rad->ConvertCoordFromPositionToPixel(wxReturn[j][i].cellPos);
                POINT pix2 = rad->ConvertCoordFromPositionToPixel(wxReturn[j + 1][i].cellPos);
                POINT pix3 = rad->ConvertCoordFromPositionToPixel(wxReturn[j + 1][i + 1].cellPos);
                POINT pix4 = rad->ConvertCoordFromPositionToPixel(wxReturn[j][i + 1].cellPos);

                // draw X for high precip color
                Point p1 = Point(pix1.x, pix1.y);
                Point p2 = Point(pix2.x, pix2.y);
                Point p3 = Point(pix3.x, pix3.y);
                Point p4 = Point(pix4.x, pix4.y);
                Point radarPixel[4] = { p1, p2, p3, p4 };

                if (wxReturn[j][i].dbz >= highdBZ) {
                    // check if next pixel is also true, defer drawing to draw two pixels as one

                    if (j < 254 && wxReturn[j + 1][i].dbz >= highdBZ && !deferDraw) {

                        deferDraw = true;
                        defp1 = p1;
                        defp4 = p4;

                        continue;
                    }

                    if (deferDraw) {

                        Point defradarPixel[4] = { defp1, p2, p3, defp4 };

                        g->FillPolygon(&heavyPrecipHatch, defradarPixel, 4);
                        deferDraw = false;
                    }
                    else {
                        g->FillPolygon(&heavyPrecipHatch, radarPixel, 4);
                    }
                }

                if (wxReturn[j][i].dbz >= alldBZ && wxReturn[j][i].dbz < highdBZ && showAllPrecip) {
                    if (j < 254 && wxReturn[j + 1][i].dbz >= alldBZ && wxReturn[j + 1][i].dbz < highdBZ && !deferDraw) {

                        deferDraw = true;
                        defp1 = p1;
                        defp4 = p4;

                        continue;
                    }

                    if (deferDraw) {

                        Point defradarPixel[4] = { defp1, p2, p3, defp4 };

                        g->FillPolygon(&lightPrecipHatch, defradarPixel, 4);
                        deferDraw = false;
                    }
                    else {
                        g->FillPolygon(&lightPrecipHatch, radarPixel, 4);
                    }
                }

            }
        }

    }
    return 0;
}

void wxRadar::parseVatsimMetar(int i) {

    CURL* metarCurlHandle = curl_easy_init();
    string metarString;
    CAsyncResponse response;

    if (metarCurlHandle) {
        curl_easy_setopt(metarCurlHandle, CURLOPT_URL, "https://metar.vatsim.net/metar.php?id=c");
        curl_easy_setopt(metarCurlHandle, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(metarCurlHandle, CURLOPT_WRITEDATA, &metarString);
        curl_easy_setopt(metarCurlHandle, CURLOPT_TIMEOUT_MS, 2500L);
        CURLcode res;
        res = curl_easy_perform(metarCurlHandle);
        if (res == CURLE_OPERATION_TIMEDOUT) {
            response.reponseMessage = "METAR Fetch Timed Out";
            response.responseCode = 1;
            wxRadar::asyncMessages.push_back(response);
        }
        curl_easy_cleanup(metarCurlHandle);
    }

    altimeterMutex.lock();

    try {
        std::istringstream in(metarString);
        regex altimeterSettingRegex("A[0-9]{4}");
        smatch altimeterSetting;
        string altimeter;

        for (string line; getline(in, line);) {
            string icao = line.substr(0, 4);
            if (regex_search(line, altimeterSetting, altimeterSettingRegex)) {
                altimeter = altimeterSetting[0].str().substr(1, 4);
            }
            else
            {
                altimeter = "****";
            }
            arptAltimeter[icao] = altimeter;
        }
    }
    catch (exception& e) {
        response.reponseMessage = e.what();
        response.responseCode = 1;
        wxRadar::asyncMessages.push_back(response);
    }

    altimeterMutex.unlock();
}

void wxRadar::parseVatsimATIS(int i) {
    CURL* vatsimURL = curl_easy_init();
    CURL* atisVatsimStatusJson = curl_easy_init();
    string strVatsimURL;
    string jsAtis;
    CAsyncResponse result;

    if (vatsimURL) {
        curl_easy_setopt(vatsimURL, CURLOPT_URL, "http://status.vatsim.net/status.json");
        curl_easy_setopt(vatsimURL, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(vatsimURL, CURLOPT_WRITEDATA, &strVatsimURL);
        curl_easy_setopt(vatsimURL, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(vatsimURL, CURLOPT_TIMEOUT_MS, 5000L);

        CURLcode res = curl_easy_perform(vatsimURL);
        curl_easy_cleanup(vatsimURL);

        if (res != CURLE_OK || strVatsimURL.empty()) return;
    }

    string dataURL;

    try {
        json jsVatsimURL = json::parse(strVatsimURL);
        dataURL = jsVatsimURL["data"]["v3"][0];
    }
    catch (exception& e) { string error = e.what(); }

    if (atisVatsimStatusJson && !dataURL.empty()) {
        curl_easy_setopt(atisVatsimStatusJson, CURLOPT_URL, dataURL.c_str());
        curl_easy_setopt(atisVatsimStatusJson, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(atisVatsimStatusJson, CURLOPT_WRITEDATA, &jsAtis);
        curl_easy_setopt(atisVatsimStatusJson, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(atisVatsimStatusJson, CURLOPT_TIMEOUT_MS, 1500L);

        CURLcode res = curl_easy_perform(atisVatsimStatusJson);
        curl_easy_cleanup(atisVatsimStatusJson);

        if (res != CURLE_OK || jsAtis.empty()) {
            result.reponseMessage = "VATSIM Datafeed Timed Out - ATIS letter may be incorrect";
            result.responseCode = 1;
            wxRadar::asyncMessages.push_back(result);
            return;
        }

        arptAtisLetter.clear();
    }

    try {
        wxRadar::jsVatsimDataFeed = json::parse(jsAtis);

        if (!wxRadar::jsVatsimDataFeed["pilots"].empty()) {
            CSiTRadar::acADSB.clear();
            CSiTRadar::acRVSM.clear();

            for (auto& pilot : wxRadar::jsVatsimDataFeed["pilots"]) {
                if (!pilot["flight_plan"]["aircraft"].is_null()) {
                    string icaoACData = pilot["flight_plan"]["aircraft"];

                    regex icaoADSB("(.*)\\/(.*)\\-(.*)\\/(.*)(E|L|B1|B2|U1|U2|V1|V2)(.*)");
                    regex icaoRVSM("(.*)\\/(.*)\\-(.*)[W](.*)\\/(.*)", regex::icase);

                    CSiTRadar::acADSB.emplace(pilot["callsign"], regex_search(icaoACData, icaoADSB));
                    CSiTRadar::acRVSM.emplace(pilot["callsign"], regex_search(icaoACData, icaoRVSM));
                }
            }
        }

        std::unique_lock<shared_mutex> lock(atisLetterMutex);
        if (wxRadar::jsVatsimDataFeed.contains("atis") && wxRadar::jsVatsimDataFeed["atis"].is_array()) {
            for (auto& atis : wxRadar::jsVatsimDataFeed["atis"]) {
                if (atis.contains("atis_code") && !atis["atis_code"].is_null()) {
                    string airport = atis["callsign"];
                    string code = atis["atis_code"];

                    if (!code.empty() && airport.size() >= 4) {
                        arptAtisLetter[airport.substr(0, 4)] = code;
                    }
                }
            }
        }
        lock.unlock();
    }
    catch (exception& e) {
        result.reponseMessage = string("VATSIM JSON Parse Error: ") + e.what();
        result.responseCode = 1;
        wxRadar::asyncMessages.push_back(result);
    }
}
