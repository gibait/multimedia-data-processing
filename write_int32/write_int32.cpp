//
// Created by giba on 13/03/23.
//

#include <fstream>
#include <iostream>
#include <bitset>

void usage() {
        std::cout << "Usage: write_int32 <filein.txt> <fileout.bin>" << std::endl;
        exit(EXIT_FAILURE);
}

template <typename T>
std::ostream &raw_write(std::ostream &os, const T &val, size_t size = sizeof(T)) {
        os.write(reinterpret_cast<const char*>(&val), size);
}

int main(int argc, char** argv) {

        using namespace std;

        if (argc != 3) {
                usage();
        }

        ifstream is(argv[1]);
        if (!is) {
                cout << "Error opening input file" << endl;
                usage();
        }

        ofstream os(argv[2], ios::binary);
        if (!os) {
                cout << "Error opening output file" << endl;
                usage();
        }

        int32_t num;
        while (is >> num) {
                raw_write(os, num);
        }

        return 0;
}