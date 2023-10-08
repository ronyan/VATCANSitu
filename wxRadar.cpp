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

void wxRadar::parseRadarPNG(CRadarScreen* rad) {
    
    GetRainViewerJSON(rad);

    if(CreateDirectory(".\\situWx\\", NULL)) {}

    CURL* pngDL = curl_easy_init();
    FILE* dlPNG;
    errno_t err;
    string tileCacheurl = "https://tilecache.rainviewer.com/v2/radar/" + wxRadar::ts + "/256/4/" + wxRadar::wxLatCtr + "/" + wxRadar::wxLongCtr + "/0/0_0.png";

    const char* filename = ".\\situWx\\0_0.png";
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

                wxReturn[j][i].dbz = (int)image[(i * 256 * 4) + (j * 4)];
                wxReturn[j][i].cellPos.m_Longitude = radReturnTL.m_Longitude + (double)(j * (22.5 / 256.0));
                wxReturn[j][i].cellPos.m_Latitude = pixLat;
            }
        }
    }
}

int wxRadar::renderRadar(Graphics* g, CRadarScreen* rad, bool showAllPrecip) {

    HatchBrush lightPrecipHatch(HatchStyleDarkUpwardDiagonal, Color(64, 0, 43, 255), Color(0, 0, 0, 0));
    HatchBrush heavyPrecipHatch(HatchStyleDarkUpwardDiagonal, Color(128, 0, 32, 255), Color(0, 0, 0, 0));

    int alldBZ = 60;
    int highdBZ = 80;

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

                    if (j<254 && wxReturn[j+1][i].dbz >= highdBZ && !deferDraw) {

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
                    if (j < 254 && wxReturn[j+1][i].dbz >= alldBZ && wxReturn[j+1][i].dbz < highdBZ && !deferDraw) {

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
        curl_easy_setopt(vatsimURL, CURLOPT_TIMEOUT_MS, 1500L);
        CURLcode res;
        res = curl_easy_perform(vatsimURL);
        if (res == CURLE_OPERATION_TIMEDOUT) {
            result.reponseMessage = "VATSIM Datafeed URL Fetch Timed Out";
            result.responseCode = 1;
            //asyncMessages.insert(result);
            return;
        }
        curl_easy_cleanup(vatsimURL);
    }

    string dataURL;

    try {
        json jsVatsimURL = json::parse(strVatsimURL);
        dataURL = jsVatsimURL["data"]["v3"][0];
    }
    catch (exception& e) { string error = e.what(); }

    if (atisVatsimStatusJson) {
        curl_easy_setopt(atisVatsimStatusJson, CURLOPT_URL, dataURL.c_str());
        curl_easy_setopt(atisVatsimStatusJson, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(atisVatsimStatusJson, CURLOPT_WRITEDATA, &jsAtis);
        curl_easy_setopt(atisVatsimStatusJson, CURLOPT_TIMEOUT_MS, 1500L);
        CURLcode res;
        res = curl_easy_perform(atisVatsimStatusJson);
        if (res == CURLE_OPERATION_TIMEDOUT) {
            result.reponseMessage = "VATSIM Datafeed Timed Out - ATIS letter may be incorrect";
            result.responseCode = 1;
            asyncMessages.push_back(result);
            return;
        }
        else {
            arptAtisLetter.clear();
        }
        curl_easy_cleanup(atisVatsimStatusJson);
    }
    else { return; }


    try {
        json jsVatsimAtis = json::parse(jsAtis.c_str());
        std::unique_lock<shared_mutex> lock(atisLetterMutex);

        if (!jsVatsimAtis["atis"].empty()) {
            for (auto& atis : jsVatsimAtis["atis"]) {
                if (!atis["atis_code"].is_null()) {
                    string airport = atis["callsign"];
                    arptAtisLetter[airport.substr(0, 4)] = atis["atis_code"];
                }
            }
        }
        lock.unlock();
    }
    catch (exception& e) { result.reponseMessage = e.what(); result.responseCode = 1; asyncMessages.push_back(result); return; }

    return;
}