//
// Created by giba on 29/04/23.
//

#include <cstdint>
#include <fstream>
#include <sstream>
#include <string>

#include "types.h"
#include "mat.h"


bool y4m_extract_color(const std::string& filename, std::vector<mat<vec3b>>& frames)
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

        if (h < 0 || w < 0 || chroma != "420jpeg") {
                return false;
        }

        mat<uint8_t> Y(h, w);
        mat<uint8_t> Cb(h / 2, w / 2);
        mat<uint8_t> Cr(h / 2, w / 2);

        mat<vec3b> rgb(h, w);

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

                // YCbCr to RGB conversion
                for (int r = 0; r < Y.rows(); ++r) {
                        for (int c = 0; c < Y.cols(); ++c) {

                                int y = Y(r, c);
                                int cb = Cb(r/2, c/2);
                                int cr = Cr(r/2, c/2);

                                y = y < 16 ? 16 : (y > 235) ? 235 : y;
                                cb = cb < 16 ? 16 : (cb > 235) ? 235 : cb;
                                cr = cr < 16 ? 16 : (cr > 235) ? 235 : cr;

                                int R = 1.164 * (y - 16) + 1.596 * (cr - 128);
                                int G = 1.164 * (y - 16) + -0.392 * (cb - 128) + -0.813 * (cr - 128);
                                int B = 1.164 * (y - 16) + 2.017 * (cb - 128);

                                R = R < 0 ? 0 : ((R > 255) ? 255 : R);
                                G = G < 0 ? 0 : ((G > 255) ? 255 : G);
                                B = B < 0 ? 0 : ((B > 255) ? 255 : B);


                                rgb(r, c) = {uint8_t (R), uint8_t (G), uint8_t (B)};
                        }
                }

                frames.push_back(rgb);
        }

        return true;
}