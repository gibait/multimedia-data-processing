//
// Created by giba on 29/06/23.
//

#include "lzs.h"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <vector>

class bitreader {
        uint8_t buffer_{};
        uint8_t n_ = 0;
        std::istream &is_;

public:
        explicit bitreader(std::istream &is) : is_(is) {}

        uint8_t operator()(unsigned size) {
                return read(size);
        }

private:
        void raw_read() {
                is_.read(reinterpret_cast<char *>(&buffer_), 1);
        }

        uint8_t read_bit() {
                if (n_ == 0) {
                        raw_read();
                        n_ = 8;
                }
                --n_;
                return (buffer_ >> n_) & 1;
        }

        uint8_t read(size_t size) {
                assert(size > 0 && size <= 8);
                uint8_t toReturn = 0;
                while (size-- > 0) {
                        toReturn = (toReturn << 1) | read_bit();
                }
                return toReturn;
        }
};

void lzs_decompress(std::istream &is, std::ostream &os) {
        std::vector<uint8_t> buffer;
        bitreader br(is);

        do {
                if (br(1) == 0) {
                        // Literal
                        uint8_t byte = br(8);
                        buffer.push_back(byte);

                } else {
                        // Run Length
                        uint16_t offset = 0;
                        uint32_t length = 0;

                        // Offset
                        if (br(1) == 1) {
                                offset = br(7);
                                if (offset == 0) {
                                        // End Marker
                                        break;
                                }
                        } else {
                                offset = br(8);
                                offset = (offset << 3) | br(3);
                        }

                        // Length
                        uint8_t N = 1;
                        uint8_t xxxx;
                        uint8_t nextTwoBit = br(2);

                        if (nextTwoBit != 0b11) {
                                length = nextTwoBit + 2;
                        }
                        if (nextTwoBit == 0b11) {
                                length = 3 + br(2) + 2;
                        }
                        if (length == 8) {
                                uint8_t repeatingOnes = br(4);
                                while (repeatingOnes == 0b1111) {
                                        N++;
                                        repeatingOnes = br(4);
                                }
                                // xxxx
                                xxxx = repeatingOnes;

                                // Use N to define offset
                                length = xxxx + ((15 * N) - 7);
                        }

                        do {
                                for (unsigned it = buffer.size() - offset; it < buffer.size(); ++it) {
                                        buffer.push_back(buffer[it]);
                                        if (--length == 0) {
                                                break;
                                        }
                                }
                        } while (length-- > 0);
                }
        } while (is);


        os.write(reinterpret_cast<char *>(buffer.data()), buffer.size());
}