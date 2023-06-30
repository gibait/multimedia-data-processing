//
// Created by giba on 29/06/23.
//

#include "lzs.h"
#include <fstream>

int main(int argc, char *argv[]) {
        std::ifstream is(argv[1], std::ios::binary);
        std::ofstream os(argv[2], std::ios::binary);
        lzs_decompress(is, os);
}