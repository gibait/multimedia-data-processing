//
// Created by giba on 03/07/23.
//


#include <algorithm>
#include <array>
#include <complex>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <vector>

using rgb = std::array<uint8_t, 3>;
using rgba = std::array<uint8_t, 4>;

template<typename T>
struct mat {
        std::vector<T> data_;
        int rows_, cols_;

        mat(int rows, int cols) : rows_(rows), cols_(cols), data_(rows * cols){};

        T &operator()(int r, int c) {
                return data_[r * cols_ + c];
        }

        const T &operator()(int r, int c) const {
                return data_[r * cols_ + c];
        }

        void resize(int r, int c) {
                rows_ = r;
                cols_ = c;
                data_.resize(r * c);
        }

        auto size() { return cols_ * rows_; }
        auto rawsize() { return size() * sizeof(T); }
        auto rawdata() { return reinterpret_cast<char *>(data_.data()); }
};

struct bitmapInfoHeader {
        uint32_t size;
        int32_t width;
        int32_t height;
        uint16_t color_planes;
        uint16_t depth;
        uint32_t compression;
        uint32_t image_size;
        int32_t horizontal;
        int32_t vertical;
        uint32_t num_colors;
        uint32_t important;
};

void bmp2pam(std::ifstream &is, std::ofstream &os) {
        // Read Bitmap File Header
        uint16_t magic;
        uint32_t file_size;
        uint32_t offset;

        is.read(reinterpret_cast<char *>(&magic), 2);
        if (magic != 19778) {
                return;
        }

        is.read(reinterpret_cast<char *>(&file_size), 4);
        // Consume reserved bytes
        is.read(reinterpret_cast<char *>(&magic), 2);
        is.read(reinterpret_cast<char *>(&magic), 2);
        is.read(reinterpret_cast<char *>(&offset), 4);


        // Read Bitmap Info Header
        struct bitmapInfoHeader Info;
        is.read(reinterpret_cast<char *>(&Info), 40);
        if (Info.color_planes != 1) {
                return;
        }

        // Read Color Table
        std::vector<rgb> color_table;
        Info.num_colors = Info.num_colors ? Info.num_colors : 1 << 8;
        while (Info.num_colors-- > 0) {
                uint8_t R, G, B;
                is.read(reinterpret_cast<char *>(&R), 1);
                is.read(reinterpret_cast<char *>(&G), 1);
                is.read(reinterpret_cast<char *>(&B), 1);
                is.get();
                color_table.push_back({B, G, R});
        }

        mat<rgb> img(Info.height, Info.width);

        for (int r = 0; r < img.rows_; ++r) {
                uint32_t padding = (img.cols_ * 8) % 32;
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
           << "WIDTH " << Info.width << "\n"
           << "HEIGHT " << Info.height << "\n"
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