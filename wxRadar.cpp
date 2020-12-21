#include "pch.h"
#include "wxRadar.h"

cell wxRadar::wxReturn[256][256];
double wxRadar::wxLatCtr;
double wxRadar::wxLongCtr;
int wxRadar::zoomLevel;
string wxRadar::ts;

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
    CPosition radReturnTL;

    radReturnTL.m_Longitude = -85.8 - 11.25;
    radReturnTL.m_Latitude = 50.55 + 11.25;

    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 256; j++) {
            
            wxReturn[j][i].dbz = (int)image[(i * 256 * 4)  + (j*4)];
            wxReturn[j][i].cellPos.m_Longitude = radReturnTL.m_Longitude + (double)(j * 22.5 / 256);
            wxReturn[j][i].cellPos.m_Latitude = radReturnTL.m_Latitude - (double)(i * 22.5 / 256);
        }
    }
}

void wxRadar::renderRadar(Graphics* g, CRadarScreen* rad, bool showAllPrecip) {

    SolidBrush lightPrecip(Color(64, 0, 32, 120));
    SolidBrush heavyPrecip(Color(128, 0, 32, 120));

    int alldBZ = 30;
    int highdBZ = 50;

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
                    // j<254 makes sure last pixel doesn't get deferred 
                    if (j<254 && wxReturn[j+1][i].dbz >= highdBZ) {

                        deferDraw = true;
                        defp1 = p1;
                        defp4 = p4;

                        continue;
                    }

                    if (deferDraw) {

                        Point defradarPixel[4] = { defp1, p2, p3, defp4 };

                        g->FillPolygon(&heavyPrecip, defradarPixel, 4);
                        deferDraw = false;
                    }
                    else {
                        g->FillPolygon(&heavyPrecip, radarPixel, 4);
                    }
                }

                if (wxReturn[j][i].dbz >= alldBZ && wxReturn[j][i].dbz < highdBZ && showAllPrecip) {
                    if (j < 254 && wxReturn[j+1][i].dbz >= alldBZ && wxReturn[j+1][i].dbz < highdBZ) {

                        deferDraw = true;
                        defp1 = p1;
                        defp4 = p4;

                        continue;
                    }

                    if (deferDraw) {

                        Point defradarPixel[4] = { defp1, p2, p3, defp4 };

                        g->FillPolygon(&lightPrecip, defradarPixel, 4);
                        deferDraw = false;
                    }
                    else {
                        g->FillPolygon(&lightPrecip, radarPixel, 4);
                    }
                }

            }
        }                       
        
    }
}