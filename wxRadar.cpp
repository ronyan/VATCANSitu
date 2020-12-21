#include "pch.h"
#include "wxRadar.h"



int wxRadar::wxReturns[256][256];
bool wxRadar::wxDrawAll[256][256]; // true if between high and low
bool wxRadar::wxDrawHigh[256][256]; // true if DBA above threshold
double wxRadar::wxLatCtr;
double wxRadar::wxLongCtr;
int wxRadar::zoomLevel;

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
    
    const char* filename = ".\\situWx\\0_0.png";

    std::vector<unsigned char> buffer, image;
    loadPNG(buffer, filename);
    unsigned long w, h;
    int error = wxRadar::decodePNG(image, w, h, buffer.empty() ? 0 : &buffer[0], (unsigned long)buffer.size());

    if (error != 0) {
        rad->GetPlugIn()->DisplayUserMessage("VATCAN Situ", "WX Parser", string("PNG Failed to Parse").c_str(), true, false, false, false, false);
    };

    // convert vector into 2d array with dBa values only;
    // png starts as RGBARGBARGBA... etc. 

    int alldBZ = 30;
    int highdBZ = 50;

    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 256; j++) {
            wxReturns[j][i] = (int)image[(i * 256 * 4)  + (j*4)];
        }
    }

    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 256; j++) {
            if (wxReturns[j][i] > alldBZ && wxReturns[j][i] < highdBZ) {
                wxDrawAll[j][i] = true;
            }
            else {
                wxDrawAll[j][i] = false;
            }

            if (wxReturns[j][i] > highdBZ) {
                wxDrawHigh[j][i] = true;
            }
            else {
                wxDrawHigh[j][i] = false;
            }
        }
    }
}

void wxRadar::renderRadar(Graphics* g, CRadarScreen* rad, bool showAllPrecip) {

    SolidBrush lightPrecip(Color(64, 0, 32, 120));
    SolidBrush heavyPrecip(Color(128, 0, 32, 120));

    CPosition radReturnTL;
    CPosition pos1;
    CPosition pos2;
    CPosition pos3;
    CPosition pos4;

    CPosition pos5;
    CPosition pos6;
    CPosition pos7;
    CPosition pos8;

    int tempX = 0;
    int tempY = 0;
    int tempX2 = 0;
    int tempY2 = 0;

    radReturnTL.m_Longitude = -85.8 - 11.25;
    radReturnTL.m_Latitude = 50.55 + 11.25;

    // Render radar returns
    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 256; j++) {

            if (wxDrawHigh[j][i] == true || (wxDrawAll[j][i] == true && showAllPrecip)) {

                pos1.m_Longitude = radReturnTL.m_Longitude + (double)(j * 22.5 / 256);
                pos1.m_Latitude = radReturnTL.m_Latitude - (double)(i * 22.5 / 256);

                pos2.m_Longitude = radReturnTL.m_Longitude + (double)((j + 1) * 22.5 / 256);
                pos2.m_Latitude = radReturnTL.m_Latitude - (double)((i) * 22.5 / 256);

                pos3.m_Longitude = radReturnTL.m_Longitude + (double)((j + 1) * 22.5 / 256);
                pos3.m_Latitude = radReturnTL.m_Latitude - (double)((i + 1) * 22.5 / 256);

                pos4.m_Longitude = radReturnTL.m_Longitude + (double)((j) * 22.5 / 256);
                pos4.m_Latitude = radReturnTL.m_Latitude - (double)((i + 1) * 22.5 / 256);

                POINT pix1 = rad->ConvertCoordFromPositionToPixel(pos1);
                POINT pix2 = rad->ConvertCoordFromPositionToPixel(pos2);
                POINT pix3 = rad->ConvertCoordFromPositionToPixel(pos3);
                POINT pix4 = rad->ConvertCoordFromPositionToPixel(pos4);

                // draw X for high precip color
                Point p1 = Point(pix1.x, pix1.y);
                Point p2 = Point(pix2.x, pix2.y);
                Point p3 = Point(pix3.x, pix3.y);
                Point p4 = Point(pix4.x, pix4.y);
                Point radarPixel[4] = { p1, p2, p3, p4 };

                if (wxDrawHigh[j][i] == true) {
                    g->FillPolygon(&heavyPrecip, radarPixel, 4);
                }

                if (wxDrawAll[j][i] == true) {
                    g->FillPolygon(&lightPrecip, radarPixel, 4);
                }

            }
        }
    }
}