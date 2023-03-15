//
// Created by giba on 13/03/23.
//

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <iterator>

void usage() {
        std::cout << "Usage: read_int32 <filein.bin> <fileout.txt>" << std::endl;
        exit(EXIT_FAILURE);
}

template <typename T>
std::istream& raw_read(std::istream& is, T& val, size_t size = sizeof(T)) {
        is.read(reinterpret_cast<char*>(&val), size);
}

int main(int argc, char** argv) {

        using namespace std;

        if (argc != 3) {
                usage();
        }

        ifstream is(argv[1], ios::binary);
        if (!is) {
                cout << "Error opening input file" << endl;
                usage();
        }

        ofstream os(argv[2]);
        if (!os) {
                cout << "Error opening output file" << endl;
                usage();
        }

        int32_t num;
        while (raw_read(is, num)) {
                os << num << endl;
        }

        return 0;
}