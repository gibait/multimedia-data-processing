//
// Created by giba on 04/07/23.
//

#include <cstdint>
#include <fstream>

#ifndef MULTIMEDIA_DATA_PROCESSING_BITREADER_H
#define MULTIMEDIA_DATA_PROCESSING_BITREADER_H

template<typename T>
void raw_read(std::ifstream &is, T &val, size_t size = sizeof(T)) {
        is.read(reinterpret_cast<char *>(&val), size);
}

class bitreader {
        uint8_t buffer_;
        uint8_t n_ = 0;
        std::ifstream &is_;

public:
        explicit bitreader(std::ifstream &is) : is_(is) {}

        uint32_t read_bit() {
                if (n_ == 0) {
                        raw_read(is_, buffer_);
                        n_ = 8;
                }
                --n_;
                return (buffer_ >> n_) & 1;
        }

        uint32_t read(uint8_t n) {
                uint32_t u = 0;
                while (n-- > 0) {
                        u = (u << 1) | read_bit();
                }
                return u;
        }

        uint32_t operator()(uint8_t n) {
                return read(n);
        }

        bool fail() const {
                return is_.fail();
        }

        explicit operator bool() const {
                return !fail();
        }
};

#endif//MULTIMEDIA_DATA_PROCESSING_BITREADER_H
