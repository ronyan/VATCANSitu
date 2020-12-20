#pragma once

#include "picopng.h"
#include <iostream>
#include <fstream>
#include "EuroScopePlugIn.h"
#include "VATCANSitu.h"
#include "CSiTRadar.h"


class wxRadar :
    public CRadarScreen
{
public:

    void loadPNG(std::vector<unsigned char>& buffer, const std::string& filename) //designed for loading files from hard disk in an std::vector
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
    };

    int main(int argc, char* argv[])
    {
        const char* filename = argc > 1 ? argv[1] : "0_0.png";

        //load and decode
        std::vector<unsigned char> buffer, image;
        loadPNG(buffer, filename);
        unsigned long w, h;
        int error = decodePNG(image, w, h, buffer.empty() ? 0 : &buffer[0], (unsigned long)buffer.size());

        //if there's an error, display it
        if (error != 0) std::cout << "error: " << error << std::endl;

        //the pixels are now in the vector "image", use it as texture, draw it, ...
    };

    void renderRadar() {};
};