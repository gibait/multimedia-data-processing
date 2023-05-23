//
// Created by giba on 22/05/23.
//
#include <matrix/mat.h>

#include <array>
#include <fstream>
#include <iostream>

#include "parse_header.h"

using vec3b = std::array<uint8_t, 3>;


bool split(mat<vec3b>& rgb, const std::string& file)
{
        // Filename retrieving
        std::string name;
        name = file.substr(0, file.find('.'));

        std::ifstream is(file, std::ios::binary);
        if (!is) {
                std::cout << "Impossibile aprire il file";
                return false;
        }

        PAM_t header;
        // Header parsing
        if (!parse_header(is, header)) {
                std::cout << "Header non valido";
                return false;
        }

        int w, h;
        w = header.width;
        h = header.height;

        // Loading
        rgb.resize(h, w);
        is.read(rgb.rawdata(), rgb.rawsize());

        // Splitting
        mat<uint8_t> R(h, w);
        mat<uint8_t> G(h, w);
        mat<uint8_t> B(h, w);

        for (int r = 0; r < rgb.rows(); ++r) {
                for (int c = 0; c < rgb.cols(); ++c) {
                        R(r,c) = rgb(r,c)[0];
                        G(r,c) = rgb(r,c)[1];
                        B(r,c) = rgb(r,c)[2];
                }
        }

        // Saving
        std::ofstream os[3];
        os[0].open(name + "_R.pam", std::ios::binary);
        os[1].open(name + "_G.pam", std::ios::binary);
        os[2].open(name + "_B.pam", std::ios::binary);

        for (auto &o : os) {
                o << "P7\n";
                o << "WIDTH " << w << "\n";
                o << "HEIGHT " << h << "\n";
                o << "DEPTH 1\n";
                o << "MAXVAL 255\n";
                o << "TUPLTYPE GRAYSCALE\n";
                o << "ENDHDR\n";
        }

        os[0].write(R.rawdata(), R.rawsize());
        os[1].write(G.rawdata(), G.rawsize());
        os[2].write(B.rawdata(), B.rawsize());

        return true;
}

int main(int argc, char **argv)
{
        if (argc != 2) {
                std::cout << "Numero di parametri errato";
                return 1;
        }

        mat<vec3b> rgb;
        split(rgb, argv[1]);

        return 0;
}