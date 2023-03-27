//
// Created by giba on 20/03/23.
//

#include <cmath>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

#include "bitwriter.h"
#include "bitreader.h"


using namespace std;

void usage() {
        cout << "elias [c|d] <filein> <fileout>" << endl;
        exit(EXIT_FAILURE);
}

class elias {

public:
        static void encode(std::istream& is, std::ostream& os) {
                int n;

                // Read input file into vector
                istream_iterator<int> start(is), end;
                vector<int> v(start, end);

                bitwriter bw(os);

                for (auto &x: v) {
                        if (x > 0) {
                                x *= 2;
                                x += 1;
                        } else if (x < 0) {
                                x = (-x) * 2;
                        } else {
                                x = 1;
                        }

                        // Number of bits needed to represent x
                        n = floor(log2(x));
                        bw(x, n * 2 + 1);
                }
        }

        static void decode(std::istream& is, std::ostream& os) {
                size_t n = 0;
                uint32_t read;
                int32_t buffer = 0;
                vector<int> v;

                bitreader br(is);

                while (!br.fail()) {
                        read = br(1);
                        if (read == 1) {
                                buffer = (buffer << 1) + read;
                                buffer = buffer << n | br(n);
                                n = 0;

                                if (buffer % 2 == 1) {
                                        buffer = (buffer - 1) / 2;
                                } else {
                                        buffer = -buffer / 2;
                                }

                                v.push_back(buffer);
                                buffer = 0;
                        } else {
                                buffer << 1;
                                n++;
                        }
                }

                std::copy(v.begin(), v.end(),
                          std::ostream_iterator<int>(os, "\n"));
        }
};


int main(int argc, char** argv) {
        string mode;

        if (argc != 4) {
                usage();
        }

        ifstream is;

        mode = argv[1];
        if (mode == "c") {
                is.open(argv[2]);
                if (!is) {
                        cout << "Error opening input file" << endl;
                        usage();
                }

                ofstream os(argv[3], ios::binary);
                if (!os) {
                        cout << "Error opening output file" << endl;
                        usage();
                }

                elias::encode(is, os);
        }
        else if (mode == "d") {

                is.open(argv[2], ios::binary);
                if (!is) {
                        cout << "Error opening input file" << endl;
                        usage();
                }

                ofstream os(argv[3]);
                if (!os) {
                        cout << "Error opening output file" << endl;
                        usage();
                }

                elias::decode(is, os);
        }
        else {
                usage();
        }

}
