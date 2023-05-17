//
// Created by giba on 15/05/23.
//

#include <cstdint>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <array>

#include "matrix/mat.h"

using vec3b = std::array<uint8_t, 3>;

void be2le(uint16_t &big_endian)
{
        big_endian = (big_endian << 8) | ((big_endian >> 8) & 0x00ff);
}

uint8_t delta_h(uint8_t G4, uint8_t G6, uint8_t X5, uint8_t X3, uint8_t X7)
{
        return std::abs(G4 - G6) + std::abs(X5 - X3 + X5 - X7);
}

uint8_t delta_v(uint8_t G2, uint8_t G8, uint8_t X5, uint8_t X1, uint8_t X9)
{
        return std::abs(G2 - G8) + std::abs(X5- X1 + X5 - X9);
}

uint8_t delta_n(uint8_t X1, uint8_t X9, uint8_t G5, uint8_t G1, uint8_t G9)
{
        return std::abs(X1 - X9) + std::abs(G5 - G1 + G5 - G9);
}

uint8_t delta_p(uint8_t X3, uint8_t X7, uint8_t G5, uint8_t G3, uint8_t G7)
{
        return std::abs(X3 - X7) + std::abs(G5 - G3 + G5 - G7);
}

void interpolate_green(mat<vec3b> &rgb, int r, int c, int channel)
{
        uint8_t X1, X5, X3, X7, X9;
        uint8_t G4, G6, G2, G8;

        // Possono essere sia R che B
        X5 = rgb(r,c)[channel];
        X1 = rgb(r-2, c)[channel];
        X3 = rgb(r, c-2)[channel];
        X7 = rgb(r,c+2)[channel];
        X9 = rgb(r+2,c)[channel];

        G2 = rgb(r-1,c)[1];
        G4 = rgb(r,c-1)[1];
        G8 = rgb(r+1,c)[1];
        G6 = rgb(r,c+1)[1];

        uint8_t dh = delta_h(G4, G6, X5, X3, X7);
        uint8_t dv = delta_v(G2, G8, X5, X1, X9);

        if (dh < dv) {
                rgb(r,c)[1] = (G4 + G6)/2 + (X5 - X3 + X5 - X7)/4;
        } else if (dh > dv) {
                rgb(r,c)[1] = (G2 + G8)/2 + (X5 - X1 + X5 - X9)/4;
        } else {
                rgb(r,c)[1] = (G2+G4+G6+G8)/4 + (X5-X1+X5-X3+X5-X7+X5-X9)/8;
        }
}

void interpolate_remaining(mat<vec3b> &rgb, int r, int c, int channel)
{
        uint8_t G1, G3, G5, G7, G9;
        uint8_t X1, X3, X7, X9;
        uint8_t dn, dp;

        G1 = rgb(r-1, c-1)[1];
        G3 = rgb(r-1, c+1)[1];
        G5 = rgb(r,c)[1];
        G7 = rgb(r+1, c-1)[1];
        G9 = rgb(r+1, c+1)[1];

        X1 = rgb(r-1, c-1)[channel];
        X3 = rgb(r-1, c+1)[channel];
        X7 = rgb(r+1, c-1)[channel];
        X9 = rgb(r+1, c+1)[channel];

        dn = delta_n(X1, X9, G5, G1, G9);
        dp = delta_p(X3, X7, G5, G3, G7);

        int updated;
        if (dn < dp) {
                updated = (X1 + X9)/2 + (G5 - G1 + G5 - G9)/4;
        } else if (dn > dp) {
                updated = (X3 + X7)/2 + (G5 - G3 + G5 - G7)/4;
        } else {
                updated = (X1+X3+X7+X9)/4 + (G5-G1+G5-G3+G5-G7+G5-G9)/8;
        }
        rgb(r,c)[channel] = updated < 255 ? updated : 255;
}

bool parse_pgm(std::ifstream &is, const std::string &output_prefix, mat<uint16_t> &mat_)
{

        std::string line;
        getline(is, line);

        if (line != "P5") {
                return false;
        }

        int w, h, maxval;
        is >> w >> h >> maxval;
        is.get();
        if (maxval < 0 or maxval > 65536) {
                return false;
        }

        mat_.resize(h, w);
        is.read(mat_.rawdata(), mat_.rawsize());
        std::for_each(mat_.begin(), mat_.end(), be2le);

        mat<uint8_t> resized(h, w);
        for (int r = 0; r < mat_.rows(); ++r) {
                for (int c = 0; c < mat_.cols(); ++c) {
                        resized(r,c) = mat_(r, c) / 256;
                }
        }

        std::ofstream os(output_prefix + "_gray.pgm", std::ios::binary);
        if (!os) {
                return false;
        }

        os << "P5 " << w << " " << h << " " << 255 << " ";
        os.write(resized.rawdata(), resized.rawsize());


        mat<vec3b> rgb(h, w);
        for (int r = 0; r < resized.rows(); r+=2) {
                for (int c = 0; c < resized.cols(); c+=2) {
                        rgb(r, c)[0] = resized(r, c);
                        rgb(r + 1, c)[1] = resized(r + 1, c);
                        rgb(r, c + 1)[1] = resized(r, c + 1);
                        rgb(r + 1, c + 1)[2] = resized(r + 1, c + 1);
                }
        }

        std::ofstream osb(output_prefix + "_bayer.ppm");
        if (!osb) {
                return false;
        }

        osb.write("P6\n", 3);
        osb << w << " " << h << " " << 255 << "\n";
        osb.write(&rgb.rawdata()[0], rgb.rawsize());


        for (int r = 2; r < rgb.rows() - 2; r+=2) {
                for (int c = 2; c < rgb.cols() - 2; c+=2) {

                        // Red
                        interpolate_green(rgb, r, c, 0);

                        // Blue
                        interpolate_green(rgb, r+1, c+1, 2);
                }
        }

        std::ofstream osg(output_prefix + "_green.ppm");
        if (!osg) {
                return false;
        }

        osg.write("P6\n", 3);
        osg << w << " " << h << " " << 255 << "\n";
        osg.write(&rgb.rawdata()[0], rgb.rawsize());

        for (int r = 4; r < rgb.rows() - 4; r+=2) {
                for (int c = 4; c < rgb.cols() - 4; c+=2) {

                        rgb(r,c+1)[0] = (rgb(r,c)[0] + rgb(r,c+2)[0])/2;
                        rgb(r+1,c)[0] = (rgb(r,c)[0] + rgb(r+2,c)[0])/2;

                        rgb(r,c+1)[2] = (rgb(r-1,c+1)[2] + rgb(r+1,c+1)[2])/2;
                        rgb(r+1,c)[2] = (rgb(r+1,c-1)[2] + rgb(r+1,c+1)[2])/2;

                        // Red
                        interpolate_remaining(rgb, r, c, 2);
                        // Blue
                        interpolate_remaining(rgb, r+1, c+1, 0);
                }
        }

        std::ofstream osp(output_prefix + "_interp.ppm");
        if (!osp) {
                return false;
        }

        osp.write("P6\n", 3);
        osp << w << " " << h << " " << 255 << "\n";
        osp.write(&rgb.rawdata()[0], rgb.rawsize());

        return true;
}

int main(int argc, char **argv)
{
        if (argc != 3) {
                std::cout << "Numero di parametri errato";
                return 1;
        }

        std::ifstream is(argv[1], std::ios::binary);
        if (!is) {
                return 1;
        }

        mat<uint16_t> mat_;
        parse_pgm(is, argv[2], mat_);
}