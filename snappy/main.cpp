//
// Created by giba on 22/06/23.
//

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <span>
#include <valarray>
#include <vector>


void decompress(std::ifstream &is, std::ofstream &os) {
        /* Preamble reading */
        uint8_t preamble;
        std::vector<uint8_t> uncomp_size;
        is.read(reinterpret_cast<char *>(&preamble), sizeof(preamble));
        do {
                uncomp_size.push_back(preamble & 0x7F);
                is.read(reinterpret_cast<char *>(&preamble), sizeof(preamble));
        } while ((preamble & 0b10000000) == 0b10000000);
        uncomp_size.push_back(preamble & 0x7F);
        std::reverse(begin(uncomp_size), end(uncomp_size));

        uint32_t file_size;
        for (auto &byte: uncomp_size) {
                file_size = (file_size << 7) + byte;
        }

        std::vector<uint8_t> literals;
        uint8_t byte_tag = 0;
        uint8_t element = 0;

        do {
                if (literals.size() > 4294967296) {
                        literals.erase(begin(literals), end(literals) - 4294967296);
                }

                if (is.peek() == EOF) {
                        break;
                }
                is.read(reinterpret_cast<char *>(&byte_tag), sizeof(byte_tag));
                if ((byte_tag & 0x03) == 0x00) {
                        /* Literal */

                        // Length
                        uint8_t msb = byte_tag >> 2;
                        uint32_t length = 0;

                        if (msb < 60) {
                                length = msb;
                        } else if (msb == 60) {
                                length = is.get();
                        } else if (msb == 61) {
                                is.read(reinterpret_cast<char *>(&length), 2);
                        } else if (msb == 62) {
                                is.read(reinterpret_cast<char *>(&length), 3);
                        } else if (msb == 63) {
                                is.read(reinterpret_cast<char *>(&length), 4);
                        }
                        length += 1;

                        // Read literal data
                        for (int i = 0; i < length; ++i) {
                                is.read(reinterpret_cast<char *>(&element), sizeof(element));
                                literals.push_back(element);
                        }
                } else {
                        /* Copies */

                        uint8_t length = 0;
                        uint8_t offset_byte = byte_tag & 0x03;
                        uint32_t offset = 0;

                        if (offset_byte == 1) {
                                length = (byte_tag & 0b00011100) >> 2;
                                length += 4;

                                offset = ((byte_tag & 0b11100000) << 3) + is.get();

                        } else if (offset_byte == 2) {
                                length = (byte_tag & 0b11111100) >> 2;
                                length += 1;

                                is.read(reinterpret_cast<char *>(&offset), 2);

                        } else if (offset_byte == 3) {
                                length = (byte_tag & 0b11111100) >> 2;
                                length += 1;

                                is.read(reinterpret_cast<char *>(&offset), 3);
                        }

                        auto last = end(literals);
                        if (length > offset) {
                                std::vector<uint8_t> slice(last - offset, last);

                                for (int i = 0; i < length - offset; ++i) {
                                        slice.push_back(slice[i % slice.size()]);
                                }

                                literals.insert(end(literals), begin(slice), end(slice));
                        } else {
                                std::vector<uint8_t> slice(last - offset, (last - offset) + length);
                                literals.insert(end(literals), begin(slice), end(slice));
                        }
                }
        } while (is);

        os.write(reinterpret_cast<const char *>(literals.data()), literals.size());
}

int main(int argc, char **argv) {
        if (argc != 3) {
                std::cout << "usage: snappy_decomp <input file> <output file>";
                return 1;
        }

        std::ifstream is(argv[1], std::ios::binary);
        if (!is) {
                std::cout << "impossibile aprire il file di input";
                return 1;
        }

        std::ofstream os(argv[2] + std::string(".txt"));
        if (!os) {
                std::cout << "impossibile aprire il file di output";
                return 1;
        }

        decompress(is, os);

        return 0;
}