//
// Created by giba on 14/05/23.
//

#include <matrix/mat.h>

#include <array>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <sstream>

#include "parse_header.h"

using vec3b = std::array<uint8_t, 3>;

bool combine (const std::string& ifile)
{
        mat<vec3b> rgb;

        // File opening
        std::ifstream is[3];
        is[0].open(ifile + "_R.pam", std::ios::binary);
        is[1].open(ifile + "_G.pam", std::ios::binary);
        is[2].open(ifile + "_B.pam", std::ios::binary);

        PAM_t header[3];
        // Header Parsing
        for (int i = 0; i < 3; ++i) {
                // Header parsing
                if (!parse_header(is[i], header[i])) {
                        std::cout << "Header non valido";
                        return false;
                }
        }

        int h, w;
        h = header[0].height;
        w = header[0].width;

        // Loading
        mat<uint8_t> R(h, w);
        mat<uint8_t> G(h, w);
        mat<uint8_t> B(h, w);

        is[0].read(R.rawdata(), R.rawsize());
        is[1].read(G.rawdata(), G.rawsize());
        is[2].read(B.rawdata(), B.rawsize());

        rgb.resize(h, w);

        for (int r = 0; r < rgb.rows(); ++r) {
                for (int c = 0; c < rgb.cols(); ++c) {
                        rgb(r, c)[0] = R(r, c);
                        rgb(r, c)[1] = G(r, c);
                        rgb(r, c)[2] = B(r, c);
                }
        }

        // Saving
        std::ofstream os;
        os.open(ifile + "_reconstructed.pam", std::ios::binary);

        // Header saving
        os << "P7\n";
        os << "WIDTH " << w << "\n";
        os << "HEIGHT " << h << "\n";
        os << "DEPTH 3\n";
        os << "MAXVAL 255\n";
        os << "TUPLTYPE RGB\n";
        os << "ENDHDR\n";

        os.write(rgb.rawdata(), rgb.rawsize());

        return true;
}


int main(int argc, char **argv)
{
        if (argc != 2) {
                std::cout << "Numero di parametri errato";
                return 1;
        }

        mat<vec3b> rgb;
        combine(argv[1]);

        return 0;
}