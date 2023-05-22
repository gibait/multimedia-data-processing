//
// Created by giba on 18/05/23.
//

#include <cstdint>
#include <fstream>
#include <iostream>
#include <vector>

std::vector<uint8_t> v;

bool read_header(std::ifstream& is, uint32_t& file_length) {
        uint32_t mn, constant;
        is.read(reinterpret_cast<char*>(&mn), 4);
        is.read(reinterpret_cast<char*>(&file_length), 4);
        is.read(reinterpret_cast<char*>(&constant), 4);

        if (mn != 0x184C2103) {
                return false;
        }

        if (constant != 0x4D000000) {
                return false;
        }

        return true;
}

void read_sequence(std::ifstream& is, uint32_t& bl);

bool read_block(std::ifstream& is, uint32_t& file_length) {
        uint32_t block_length;
        size_t size_curr = v.size();

        is.read(reinterpret_cast<char*>(&block_length), 4);

        while (block_length > 0) {
                read_sequence(is, block_length);
                file_length -= (v.size() - size_curr);
                size_curr = v.size();
        }

        return true;
}

void read_sequence(std::ifstream& is, uint32_t& bl) {
        uint8_t token;
        uint8_t ll;
        uint32_t match_length;

        is.read(reinterpret_cast<char*>(&token), 1);
        bl--;

        match_length = token & 0b00001111;
        ll = token >> 4;

        if (ll == 15) {
                uint8_t oversize;
                do {
                        is.read(reinterpret_cast<char*>(&oversize), 1);
                        bl--;
                        ll += oversize;
                } while (oversize == 255);
        }

        std::vector<uint8_t> literal(ll);
        if (ll != 0) {
                is.read(reinterpret_cast<char*>(literal.data()), ll);
                for (auto &x: literal) {
                        v.push_back(x);
                }
                bl -= ll;
        }

        if (bl == 0) {
                return;
        }

        uint16_t offset;
        is.read(reinterpret_cast<char*>(&offset), 2);
        bl -= 2;

        if (offset == 0) {
                return;
        }

        match_length += 4;
        if (match_length == 19) {
                uint8_t oversize = 0;
                do {
                        is.read(reinterpret_cast<char*>(&oversize), 1);
                        bl--;
                        match_length += oversize;
                } while (oversize == 255);
        }

        while ( match_length --> 0) {
                v.push_back(v[v.size() - offset]); // Credits to Tobia Poppi for this beautiful line
        }
}

void write_to_file(std::ofstream& os, std::vector<uint8_t>& buf)
{
        os.write(reinterpret_cast<const char*>(buf.data()), buf.size());
}

int main(int argc, char *argv[])
{
        uint32_t file_length;

        if (argc != 3) {
                std::cout << "Numero di parametri errato";
                return 1;
        }

        std::ifstream is(argv[1], std::ios::binary);
        if (!is) {
                return 1;
        }

        std::ofstream os(argv[2], std::ios::binary);
        if (!os) {
                return 1;
        }

        if (!read_header(is, file_length)) {
                return 1;
        }

        while (!is.eof()) {
                read_block(is, file_length);
        }

        write_to_file(os, v);

        return 0;
}