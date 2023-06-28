#include "pch.h"
#include "wxRadar.h"

cell wxRadar::wxReturn[256][256];
lightningStrikeProb wxRadar::lightningProbMap[512][256];
vector<lightningStrike> wxRadar::lightningBoltLoc;
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

void wxRadar::parseLightningPNG(CRadarScreen* rad) {

    if (CreateDirectory(".\\situWx\\", NULL)) {}

    CURL* pngDL = curl_easy_init();
    FILE* dlPNG;
    errno_t err;
    string tomorrowIOurl = "https://api.tomorrow.io/v4/map/tile/5/8/11/lightningFlashRateDensity/now.png?apikey=RmtPHTj25ePWp0e5piKy6sLxkm3T9CCZ";

    const char* light1 = ".\\situWx\\lightning1.png";
    curl_easy_setopt(pngDL, CURLOPT_URL, tomorrowIOurl.c_str());
    curl_easy_setopt(pngDL, CURLOPT_WRITEFUNCTION, write_file);

    err = fopen_s(&dlPNG, light1, "wb");
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

    pngDL = curl_easy_init();
    dlPNG;
    tomorrowIOurl = "https://api.tomorrow.io/v4/map/tile/5/" + (string)"9" + "/" + (string)"11" + "/lightningFlashRateDensity/now.png?apikey=RmtPHTj25ePWp0e5piKy6sLxkm3T9CCZ";

    const char* light2 = ".\\situWx\\lightning2.png";
    curl_easy_setopt(pngDL, CURLOPT_URL, tomorrowIOurl.c_str());
    curl_easy_setopt(pngDL, CURLOPT_WRITEFUNCTION, write_file);

    err = fopen_s(&dlPNG, light2, "wb");
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


    std::vector<unsigned char> buffer, image1, image2;
    loadPNG(buffer, light1);
    unsigned long w, h;
    int error = wxRadar::decodePNG(image1, w, h, buffer.empty() ? 0 : &buffer[0], (unsigned long)buffer.size());
    loadPNG(buffer, light2);
    error = wxRadar::decodePNG(image2, w, h, buffer.empty() ? 0 : &buffer[0], (unsigned long)buffer.size());

    if (error != 0) {
        rad->GetPlugIn()->DisplayUserMessage("VATCAN Situ", "WX Parser", string("Lightning PNG Failed to Parse").c_str(), true, false, false, false, false);
    }
    else {

        // convert vector into 2d array with dBa values only;
        // png starts as RGBARGBARGBA... etc. 
        CPosition lightningLoc;

        lightningLoc.m_Longitude = tilex2long(8,5);
        lightningLoc.m_Latitude = tiley2lat(11,5);

        // get the pixel coord of the latitude.
        int yCoord = lat2pixel(lightningLoc.m_Latitude, 5 /*zoom level*/);

        for (int i = 0; i < 256; i++) {

            double pixLat = pixel2lat(yCoord + i, 5);
            // calculate latitdue for each row, use the pixel coordinate

            for (int j = 0; j < 256; j++) {

                lightningProbMap[j][i].intensity = (int)image1[(i * 256 * 4 + 3 /*blue*/ ) + (j * 4)];
                lightningProbMap[j][i].lightningPos.m_Longitude = lightningLoc.m_Longitude + (double)(j * (11.25 / 256.0));
                lightningProbMap[j][i].lightningPos.m_Latitude = pixLat;
            }
        }

        lightningLoc.m_Longitude = tilex2long(9, 5);
        lightningLoc.m_Latitude = tiley2lat(11, 5);
        
        // repeat for second image
        for (int i = 0; i < 256; i++) {

            double pixLat = pixel2lat(yCoord + i, 5);
            // calculate latitdue for each row, use the pixel coordinate

            for (int j = 0; j < 256; j++) {

                lightningProbMap[j+256][i].intensity = (int)image2[(i * 256 * 4 + 3 /*blue*/) + (j * 4)];
                lightningProbMap[j+256][i].lightningPos.m_Longitude = lightningLoc.m_Longitude + (double)(j * (11.25 / 256.0));
                lightningProbMap[j+256][i].lightningPos.m_Latitude = pixLat;
            }
        }
    }
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

int wxRadar::renderLightning(Graphics* g, CRadarScreen* rad) {

    //calculate new strikes every minute
    if (((clock() - CSiTRadar::menuState.lightningLastCalc) / CLOCKS_PER_SEC > 30 || lightningBoltLoc.empty())) {
        // clean up old strikes > 5 minutes
        lightningBoltLoc.erase(std::remove_if(lightningBoltLoc.begin(), lightningBoltLoc.end(), [](const lightningStrike& t) {

            return (((clock() - t.strikeTime) / CLOCKS_PER_SEC) > 300);

            }), lightningBoltLoc.end());

        // seed new lightning strikes

        for (int i = 0; i < 512; i++) {
            for (int j = 0; j < 255; j++) {
                // Lightning bolt 

                if (lightningProbMap[i][j].intensity > 10) {

                    //prob of a strike
                    if (rand()%100 > 90) {
                        lightningStrike ls;
                        clock_t randToffset = rand() % 10;
                        ls.strikeTime = clock() - randToffset;
                        ls.strikePosition = lightningProbMap[i][j].lightningPos;

                        //fudge factor for random;
                        ls.strikePosition.m_Longitude = ls.strikePosition.m_Longitude + (rand() % 100) * (11.25 / 256.0) / 100;
                        ls.strikePosition.m_Latitude = ls.strikePosition.m_Latitude + (rand() % 100) * (85.05 / 32.0) / 100;
                        lightningBoltLoc.push_back(ls);

                    }
                }
            }
        }

        CSiTRadar::menuState.lightningLastCalc = clock();
    }

    if (CSiTRadar::menuState.lightningOn) {
        for (auto& strike : lightningBoltLoc) {

            SolidBrush lightningBrush(Color(255, 222, 77));
            Pen lightningPen(Color(255, 222, 77));

            // draw lightning strikes
            GraphicsContainer gCont;


            Point points[8] = {
                Point(0,-6),
                Point(4,-6),
                Point(1,-1),
                Point(5,-2),
                Point(-1,7),
                Point(0,1),
                Point(-4,1),
                Point(0,-6),
            };

            Point points2[4] = {
                Point(1, -6),
                Point(-2, 0),
                Point(+2, 0),
                Point(-1, 6)
            };

            if (((clock() - strike.strikeTime) / CLOCKS_PER_SEC) < 90) {

                gCont = g->BeginContainer();

                g->TranslateTransform((REAL)rad->ConvertCoordFromPositionToPixel(strike.strikePosition).x, (REAL)rad->ConvertCoordFromPositionToPixel(strike.strikePosition).y, MatrixOrderAppend);
                
                if ( ((clock() - strike.strikeTime) / CLOCKS_PER_SEC) <  20) {
                    g->FillPolygon(&lightningBrush, points, 8);
                }
                else if ( ((clock() - strike.strikeTime) / CLOCKS_PER_SEC ) < 40) {
                    g->DrawPolygon(&lightningPen, points, 8);
                }
                else {
                    g->DrawLines(&lightningPen, points2, 4);
                    // draw line only
                }
                g->EndContainer(gCont);
            }
            DeleteObject(&lightningBrush);
            DeleteObject(&lightningPen);

        }
    }
    return 1;

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
        curl_easy_setopt(metarCurlHandle, CURLOPT_URL, "http://metar.vatsim.net/metar.php?id=cy");
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