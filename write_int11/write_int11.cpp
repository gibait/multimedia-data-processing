//
// Created by giba on 15/03/23.
//

#include <fstream>
#include <iostream>
#include <bitset>

void usage() {
        std::cout << "Usage: write_int11 <filein.txt> <fileout.bin>" << std::endl;
        exit(EXIT_FAILURE);
}

template <typename T>
std::ostream &raw_write(std::ostream& os, const T& val, size_t size = sizeof(T)) {
        os.write(reinterpret_cast<const char*>(&val), size);
}

class bitwriter {
        std::ostream& os_;
        uint8_t buffer_;
        size_t nbits_;

        // Shifts the buffer left by 1 and adds the least significant bit of given bit
        std::ostream& write_bit(uint32_t bit) {
                buffer_ = (buffer_ << 1) + (bit & 1);
                ++nbits_;
                if (nbits_ == 8) {
                        raw_write(os_, buffer_);
                        nbits_ = 0;
                }
                return os_;
        };

        // Writes the n least significant bits of given bit
        // from the most significant to the least significant
        std::ostream& write(uint32_t bit, uint8_t n) {
                while(n --> 0) {
                        write_bit(bit >> n);
                }
                return os_;
        }

public:

        explicit bitwriter(std::ostream& os) : os_(os), buffer_(0), nbits_(0) {};

        ~bitwriter() {
                flush();
        };

        std::ostream& operator()(uint32_t u, uint8_t n) {
                return write(u, n);
        }

        // Flushes the buffer by writing the remaining bits
        // in it and padding with 0s
        std::ostream& flush(uint32_t bit = 0) {
                while (nbits_ > 0) {
                        write_bit(bit);
                }
                return os_;
        }

};

int main(int argc, char** argv) {

        using namespace std;

        if (argc != 3) {
                usage();
        }

        ifstream is(argv[1]);
        if (!is) {
                cout << "Error opening input file: " << argv[1] << endl;
                usage();
        }

        ofstream os(argv[2], ios::binary);
        if (!os) {
                cout << "Error opening output file: " << argv[2] << endl;
                usage();
        }

        bitwriter bw(os);

        int32_t num;
        while (is >> num) {
                bw(num, 11);
        }

        return 0;
}