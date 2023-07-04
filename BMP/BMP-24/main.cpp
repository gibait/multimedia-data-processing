//
// Created by giba on 03/07/23.
//

#include <algorithm>
#include <array>
#include <fstream>
#include <iostream>

#include "matrix.h"
#include "read_headers.h"

void bmp2pam(std::ifstream &is, std::ofstream &os) {

        bitmapInfoHeader info;
        readBMPHeaders(is, info);

        uint8_t bpp = 24;

        mat<rgb> img(info.height, info.width);

        for (int r = 0; r < img.rows_; ++r) {
                uint32_t padding = 32 - (img.cols_ * bpp) % 32;
                padding /= 8;

                for (int c = 0; c < info.width; ++c) {
                        rgb pixel;
                        pixel[2] = is.get();
                        pixel[1] = is.get();
                        pixel[0] = is.get();

                        img(img.rows_ - r - 1, c) = pixel;
                }

                while (padding-- > 0) {
                        is.get();
                }
        }

        os << "P7\n"
           << "WIDTH " << info.width << "\n"
           << "HEIGHT " << info.height << "\n"
           << "DEPTH 3\n"
           << "MAXVAL 255\n"
           << "TUPLTYPE RGB\n"
           << "ENDHDR\n";

        os.write(img.rawdata(), img.rawsize());
}


int main(int argc, char *argv[]) {
        if (argc != 3) {
                std::cout << "usage: bmp2pam <input file .BMP> <output file .PAM>";
                return EXIT_FAILURE;
        }

        std::ifstream is(argv[1]);
        if (!is) {
                std::cout << "Impossibile aprire il file di input";
                return EXIT_FAILURE;
        }
        std::ofstream os(argv[2]);
        if (!is) {
                std::cout << "Impossibile aprire il file di output";
                return EXIT_FAILURE;
        }
        bmp2pam(is, os);

        return EXIT_SUCCESS;
}