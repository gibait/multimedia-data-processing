//
// Created by giba on 03/07/23.
//


#include <algorithm>
#include <array>
#include <complex>
#include <fstream>
#include <iostream>
#include <vector>

#include "matrix.h"
#include "read_headers.h"

void bmp2pam(std::ifstream &is, std::ofstream &os) {

        bitmapInfoHeader info;
        readBMPHeaders(is, info);

        uint8_t bpp = 8;

        // Read Color Table
        std::vector<rgb> color_table;
        info.num_colors = info.num_colors ? info.num_colors : 1 << bpp;
        while (info.num_colors-- > 0) {
                uint8_t R, G, B;
                is.read(reinterpret_cast<char *>(&R), 1);
                is.read(reinterpret_cast<char *>(&G), 1);
                is.read(reinterpret_cast<char *>(&B), 1);
                is.get();
                color_table.push_back({B, G, R});
        }

        mat<rgb> img(info.height, info.width);

        for (int r = 0; r < img.rows_; ++r) {
                uint32_t padding = (img.cols_ * bpp) % 32;
                padding = 4 - padding / 8;

                for (int c = 0; c < img.cols_; ++c) {
                        uint8_t index;
                        is.read(reinterpret_cast<char *>(&index), 1);
                        img(img.rows_ - r - 1, c) = color_table[index];
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