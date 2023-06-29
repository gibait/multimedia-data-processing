#include <algorithm>
#include <array>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

template<typename T>
struct mat {
        size_t rows_, cols_;
        std::vector<T> data_;

        mat(size_t rows, size_t cols) : rows_(rows), cols_(cols), data_(rows * cols) {}

        const T &operator[](size_t i) const { return data_[i]; }
        T &operator[](size_t i) { return data_[i]; }

        size_t size() const { return rows_ * cols_; }
        size_t rows() const { return rows_; }
        size_t cols() const { return cols_; }

        const char *rawdata() const {
                return reinterpret_cast<const char *>(data_.data());
        }
        size_t rawsize() const { return size() * sizeof(T); }
};

uint32_t le2be(uint32_t le) {
        uint32_t be;
        be = le & 0xFF;
        be = (be << 8) | (le >> 8) & 0xFF;
        be = (be << 8) | (le >> 16) & 0xFF;
        be = (be << 8) | (le >> 24) & 0xFF;
        return be;
}

struct qoi_header {
        char magic[4];
        uint32_t width;
        uint32_t height;
        uint8_t channels;
        uint8_t colorspace;
};

qoi_header parse_header(std::ifstream &is) {
        qoi_header h{};
        is.read(reinterpret_cast<char *>(&h.magic), 4);
        is.read(reinterpret_cast<char *>(&h.width), 4);
        h.width = le2be(h.width);
        is.read(reinterpret_cast<char *>(&h.height), 4);
        h.height = le2be(h.height);
        is.read(reinterpret_cast<char *>(&h.channels), 1);
        is.read(reinterpret_cast<char *>(&h.colorspace), 1);
        return h;
}

unsigned hash_function(std::array<uint8_t, 4> &rgb) {
        return (rgb[0] * 3 + rgb[1] * 5 + rgb[2] * 7 + rgb[3] * 11) % 64;
}

using rgba = std::array<uint8_t, 4>;

// Helper Class
struct seenPixels {
        std::array<rgba, 64> data_{};

        std::array<uint8_t, 4> &operator()(unsigned index) {
                return data_.at(index);
        }

        void insert(std::array<uint8_t, 4> toPush) {
                auto index = hash_function(toPush);
                data_[index] = toPush;
        }
};

int main(int argc, char *argv[]) {
        if (argc != 3) {
                return EXIT_FAILURE;
        }

        std::ifstream is(argv[1], std::ios::binary);
        if (!is) {
                return EXIT_FAILURE;
        }

        qoi_header qh = parse_header(is);

        mat<rgba> img(qh.height, qh.width);

        unsigned i = 0;
        seenPixels array;

        do {
                uint8_t byte;
                is.read(reinterpret_cast<char *>(&byte), sizeof(byte));

                if (byte == 0x00) {
                        if (is.peek() == 0x00) {
                                break;
                        }
                }

                uint8_t twoBit = byte >> 6;
                if (twoBit != 0b11) {
                        if (twoBit == 0b00) {
                                // QOI_OP_INDEX

                                uint8_t index = byte & 0x3F;
                                img[i] = array(index);

                        } else if (twoBit == 0b01) {
                                // QOI_OP_DIFF

                                int8_t dr = ((byte >> 4) & 3) - 2;
                                int8_t dg = ((byte >> 2) & 3) - 2;
                                int8_t db = (byte & 3) - 2;

                                rgba diff;
                                if (i > 0)
                                        diff = img[i - 1];
                                else
                                        diff = {0, 0, 0, 255};

                                diff[0] += dr;
                                diff[1] += dg;
                                diff[2] += db;

                                img[i] = diff;
                                array.insert(diff);

                        } else if (twoBit == 0b10) {
                                // QOI_OP_LUMA

                                int8_t dg = (byte & 0x3F) - 32;

                                uint8_t next;
                                is.read(reinterpret_cast<char *>(&next), sizeof(next));

                                rgba diff;
                                if (i > 0)
                                        diff = img[i - 1];
                                else
                                        diff = {0, 0, 0, 255};

                                diff[0] += dg - 8 + ((next >> 4) & 0x0f);
                                diff[1] += dg;
                                diff[2] += dg - 8 + (next & 0x0f);

                                img[i] = diff;
                                array.insert(diff);
                        }
                } else {
                        rgba pixel;

                        if (byte == 0xFF) {
                                // QOI_OP_RGBA

                                is.read(reinterpret_cast<char *>(&pixel), 4);

                        } else if (byte == 0xFE) {
                                // QOI_OP_RGB

                                is.read(reinterpret_cast<char *>(&pixel), 3);
                                if (i > 0)
                                        pixel[3] = img[i - 1][3];
                                else
                                        pixel[3] = 255;

                        } else {
                                // QOI_OP_RUN

                                uint8_t runLength = byte & 0x3F;
                                runLength += 1;
                                rgba last;
                                if (i > 0)
                                        last = img[i - 1];
                                else
                                        last = {0, 0, 0, 255};
                                while (runLength-- > 0) {
                                        img[i] = last;
                                        ++i;
                                }
                                continue;
                        }

                        img[i] = pixel;
                        array.insert(pixel);
                }

                ++i;
        } while (is);

        // Questo è il formato di output PAM, è già pronto così, ma potete modificarlo se volete
        std::ofstream os(argv[2], std::ios::binary);// Questo utilizza il secondo parametro della linea di comando!
        os << "P7\nWIDTH " << img.cols() << "\nHEIGHT " << img.rows() << "\nDEPTH 4\nMAXVAL 255\nTUPLTYPE RGB_ALPHA\nENDHDR\n";
        os.write(img.rawdata(), img.rawsize());

        return EXIT_SUCCESS;
}