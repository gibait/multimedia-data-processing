//
// Created by giba on 27/06/23.
//

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <ranges>
#include <string>
#include <vector>

template<typename T>
class mat {
        int rows_, cols_;
        std::vector<T> data_;

public:
        mat(int row = 0, int cols = 0) : cols_(cols), rows_(row), data_(row * cols){};

        T operator()(int r, int c) {
                return data_[r * cols_ + c];
        }

        const T operator()(int r, int c) const {
                return data_[r * cols_ + c];
        }

        void resize(int rows, int cols) {
                rows_ = rows;
                cols_ = cols;
                data_.resize(rows * cols);
        }

        auto cols() { return cols_; }
        auto rows() { return rows_; }
        auto size() { return rows_ * cols_; }
        auto rawsize() { return size() * sizeof(T); }
        auto rawdata() { return reinterpret_cast<char *>(data_.data()); }
};

std::string createZ85Dictionary() {
        std::string representation;

        // da 0 a 9
        for (int i = 48; i <= 57; i++) {
                representation += char(i);
        }

        // da a a z
        for (int i = 97; i <= 122; i++) {
                representation += char(i);
        }

        // da A a Z
        for (int i = 65; i <= 90; i++) {
                representation += char(i);
        }

        // caratteri rimanenti
        representation += ".-:+=^!/*?&<>()[]{}@%$#";
        return representation;
}

uint32_t le2be(uint32_t le) {
        uint32_t be;
        be = (le >> 24) & 0xFF;
        be = (be << 8) | ((le >> 8)) & 0xFF;
        be = (be << 8) | ((le >> 16) & 0xFF);
        be = (be << 8) | ((le >> 24) & 0xFF);

        return be;
}

uint64_t be2le(uint64_t be) {
        uint64_t le = 0;
        le = (be >> 32) & 0xFF;
        le = le | ((be >> 24) & 0xFF) << 8;
        le = le | ((be >> 16) & 0xFF) << 16;
        le = le | ((be >> 8) & 0xFF) << 24;
        le = le | (be & 0xFF) << 32;

        return le;
}

struct PPM_t {
        unsigned width;
        unsigned height;
        unsigned maxVal;
        mat<std::array<uint8_t, 3>> pixels;
};

template<typename T>
void parse_pam(std::ifstream &is, mat<T> &img, PPM_t &header) {
        std::string str;
        is >> str;
        if (str != "P6") {
                return;
        }

        auto pos = is.tellg();
        is >> str;
        if (str == "#") {
                // Consume comment line
                getline(is, str);
        } else {
                is.seekg(pos);
        }

        is >> header.width;
        is >> header.height;
        is >> header.maxVal;

        is.get();
        img.resize(header.height, header.width);
        // is.read(img.rawdata(), img.rawsize());
}

void parse_bin(std::ifstream &is, PPM_t &header) {
        std::string str;
        char ch = is.get();
        while (ch != ',') {
                str += ch;
                ch = is.get();
        }
        header.width = stoi(str);

        str.clear();
        ch = is.get();
        while (ch != ',') {
                str += ch;
                ch = is.get();
        }
        header.height = stoi(str);
}

void usage() {
        std::cout << "Z85rot {c | d} <N> <input file> <output file>";
}


int main(int argc, char *argv[]) {
        char mode;
        unsigned N;
        std::ifstream is;
        std::ofstream os;

        if (argc < 5) {
                usage();
                return 1;
        }

        if (std::string(argv[1]) != "c" && std::string(argv[1]) != "d") {
                usage();
                return 1;
        }
        mode = argv[1][0];
        N = std::stoi(argv[2]);

        is.open(argv[3], std::ios::binary);
        if (!is) {
                usage();
                return 1;
        }

        os.open(argv[4], std::ios::binary);
        if (!os) {
                usage();
                return 1;
        }

        std::string dict = createZ85Dictionary();
        unsigned i = 0;

        switch (mode) {
                case 'c':
                        if (true) {
                                // Useless
                                mat<std::array<uint8_t, 3>> img;
                                PPM_t header;
                                parse_pam(is, img, header);
                                os << img.cols() << ',' << img.rows() << ',';
                        }

                        do {
                                uint32_t toEncode;
                                is.read(reinterpret_cast<char *>(&toEncode), 4);
                                toEncode = le2be(toEncode);

                                std::vector<uint8_t> encoded;
                                while (encoded.size() < 5) {
                                        encoded.push_back(toEncode % 85);
                                        toEncode = std::floor(toEncode / 85);
                                }
                                std::reverse(begin(encoded), end(encoded));

                                for (auto &c: encoded) {
                                        // Positive offset from dict N * i
                                        os.write(reinterpret_cast<char *>(&dict[(c + (85 - N * i)) % 85]), 1);
                                        i = N > 0 ? i + 1 : 0;
                                }

                        } while (is.peek() != EOF);
                        break;

                case 'd':
                        if (true) {
                                PPM_t header;
                                parse_bin(is, header);

                                os << "P6\n";
                                os << header.width << " " << header.height << "\n"
                                   << 255 << "\n";
                        }

                        do {
                                uint64_t toDecode = 0;

                                // Read actual data
                                is.read(reinterpret_cast<char *>(&toDecode), 5);
                                toDecode = be2le(toDecode);

                                std::vector<uint8_t> decoded;
                                uint8_t offset = 32;
                                while (decoded.size() < 5) {
                                        auto index = dict.find((toDecode >> offset) & 0xFF);
                                        index = (index + (N * i)) % 85;
                                        decoded.push_back(index);
                                        i = N > 0 ? i + 1 : 0;
                                        offset -= 8;
                                }

                                unsigned power = 4;
                                uint32_t base10Number = 0;
                                for (auto &c: decoded) {
                                        base10Number += c * std::pow(85, power);
                                        --power;
                                }

                                base10Number = le2be(base10Number);
                                os.write(reinterpret_cast<char *>(&base10Number), 4);

                        } while (is.peek() != EOF);
                        break;

                default:
                        return 1;
        }
}