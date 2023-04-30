//
// Created by giba on 28/04/23.
//

#include <cstdint>
#include <fstream>
#include <sstream>
#include <string>

#include "mat.h"


bool y4m_extract_gray(const std::string& filename, std::vector<mat<uint8_t>>& frames)
{
        std::ifstream is(filename, std::ios::binary);
        if (!is) {
                return false;
        }

        std::string header;
        getline(is, header);
        std::stringstream ss(header);

        std::string tag;
        ss >> tag;

        if (tag != "YUV4MPEG2") {
                return false;
        }

        int w = -1; int h = -1;
        std::string chroma = "420jpeg";
        std::string interlacing, frame_rate, aspect_ratio, application;

        while (!ss.eof()) {
                char token;
                ss >> token;

                switch (token) {
                        case 'W':
                                ss >> w;
                                break;
                        case 'H':
                                ss >> h;
                                break;
                        case 'C':
                                ss >> chroma;
                                break;
                        case 'F':
                                ss >> frame_rate;
                                break;
                        case 'I':
                                ss >> interlacing;
                                break;
                        case 'A':
                                ss >> aspect_ratio;
                                break;
                        case 'X':
                                ss >> application;
                                break;
                        default:
                                return false;
                }
        }

        if (h <= 0 || w <= 0 || chroma != "420jpeg") {
                return false;
        }

        mat<uint8_t> Y(h, w);
        mat<uint8_t> Cb(h / 2, w / 2);
        mat<uint8_t> Cr(h / 2, w / 2);

        while (getline(is, header)) {
                ss.clear();
                ss.str(header);

                ss >> tag;
                if (tag != "FRAME") {
                        return false;
                }

                while (!ss.eof()) {
                        char token;
                        ss >> token;

                        if (token == 'I') {
                                ss >> interlacing;
                        } else if (token == 'X') {
                                ss >> application;
                        } else {
                                return false;
                        }
                }

                is.read(Y.rawdata(), Y.rawsize());
                is.read(Cb.rawdata(), Cb.rawsize());
                is.read(Cr.rawdata(), Cr.rawsize());

                // Add this frame to the vector
                frames.push_back(Y);
        }

        return true;
}