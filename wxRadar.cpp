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

void wxRadar::renderRadar(CDC* dc, CRadarScreen* rad, bool showAllPrecip) {

    int sDC = dc->SaveDC();

    HPEN targetPen;
    targetPen = CreatePen(PS_SOLID, 1, C_WX_BLUE);
    dc->SelectObject(targetPen);

    CPosition radReturnTL;

    radReturnTL.m_Longitude = -85.8 - 11.25;
    radReturnTL.m_Latitude = 50.55 + 11.25;


    // Render radar returns
    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 256; j++) {
            CPosition pixCoord1;
            pixCoord1.m_Longitude = radReturnTL.m_Longitude + (double)(j * 22.5 / 256);
            pixCoord1.m_Latitude = radReturnTL.m_Latitude - (double)(i * 22.5 / 256);

            POINT pixReturn = rad->ConvertCoordFromPositionToPixel(pixCoord1);

            if (wxDrawHigh[j][i] == true) {
                // draw X for high precip
                dc->MoveTo(pixReturn.x - 4, pixReturn.y - 4);
                dc->LineTo(pixReturn.x + 4, pixReturn.y + 4);

                dc->MoveTo(pixReturn.x + 4, pixReturn.y - 4);
                dc->LineTo(pixReturn.x - 4, pixReturn.y + 4);
            }
            //Draw slash for low precip
            if (showAllPrecip) {
                if (wxDrawAll[j][i] == true) {
                    dc->MoveTo(pixReturn.x - 4, pixReturn.y - 4);
                    dc->LineTo(pixReturn.x + 4, pixReturn.y + 4);
                }
            }
        }
    }
    DeleteObject(targetPen);
    // restore context
    dc->RestoreDC(sDC);
}