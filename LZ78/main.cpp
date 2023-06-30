//
// Created by giba on 23/06/23.
//

#include <cstdint>
#include <format>
#include <fstream>
#include <iostream>
#include <map>
#include <vector>

class bitwriter {
        uint8_t buffer_ = 0;
        int n_ = 0;
        std::ofstream &os_;

        std::ostream &raw_write(uint8_t val, uint8_t size = sizeof(val)) {
                return os_.write(reinterpret_cast<const char *>(&val), size);
        }

        std::ostream &write_bit(uint8_t shifted) {
                // Shift buffer and add LSB of to be written byte
                buffer_ = (buffer_ << 1) | (shifted & 1);
                // Increase number of bit in buffer
                n_++;
                // If buffer is full, empty it by writing on file
                if (n_ == 8) {
                        raw_write(buffer_);
                        n_ = 0;
                }
                return os_;
        }

public:
        explicit bitwriter(std::ofstream &os) : os_(os){};

        std::ostream &write(uint8_t byte, uint8_t size) {
                // Write a bit from LSB to MSB
                for (int i = size - 1; i >= 0; --i) {
                        write_bit(byte >> i);
                }
                return os_;
        }

        std::ostream &operator()(uint8_t byte, uint8_t size) {
                return write(byte, size);
        }

        void flush() {
                if (n_ > 0) {
                        raw_write(buffer_, 8);
                }
        }

        ~bitwriter() {
                flush();
        }
};

class Dict {
        uint8_t maxBits_ = 0;
        std::map<std::vector<uint8_t>, uint8_t> data_;

public:
        Dict(uint8_t nBits) : maxBits_(nBits){};

        void operator[](std::vector<uint8_t> vec) {
                if (std::bit_width(data_.size() + 1) > maxBits_) {
                        maxBits_ = 0;
                        data_.clear();
                        return;
                }
                data_[vec] = data_.size() + 1;
        }

        const uint8_t operator()(std::vector<uint8_t> vec) const {
                return data_.at(vec);
        }

        bool contains(std::vector<uint8_t> vec) {
                return data_.contains(vec);
        }

        size_t size() {
                return data_.size();
        }
};

bool lz78encode(const std::string &input_filename, const std::string &output_filename, int maxbits) {
        std::ifstream is(input_filename, std::ios::binary);
        if (!is) {
                std::cout << "errore durante l'apertura del file di input";
                return false;
        }

        std::ofstream os(output_filename + ".bin", std::ios::binary);
        if (!os) {
                std::cout << "errore durante l'apertura del file di output";
                return false;
        }

        if (maxbits < 1 || maxbits > 30) {
                std::cout << "maxbits deve essere compreso tra 1 e 30";
                return false;
        }

        uint8_t ch;
        size_t n_bits = 1;
        bitwriter bw(os);
        Dict dict(maxbits);

        os << "LZ78";
        bw(maxbits, 5);

        is.read(reinterpret_cast<char*>(&ch), sizeof(ch));
        std::vector<uint8_t> vec = {ch};
        dict[vec];

        bw(ch, 8);
        // std::cout << std::format("({}, {})\n", 0, char(ch));

        do {
                uint8_t next = 0;
                uint8_t index = 0;

                if (is.peek() == EOF) {
                        break;
                }

                is.read(reinterpret_cast<char *>(&ch), sizeof(ch));
                vec = {ch};

                if (!dict.contains(vec)) {
                        dict[vec];
                } else {
                        do {
                                if (is.peek() == EOF) {
                                        break;
                                }
                                is.read(reinterpret_cast<char *>(&next), sizeof(next));
                                vec.push_back(next);
                        } while (dict.contains(vec));

                        // Aggiungiamo il nuovo carattere alla sequenza più lunga trovata
                        dict[vec];
                        // Lo eliminiamo dal vettore per ricava l'indice del dizionario da stampare
                        // della sequenza più lunga trovata fin'ora
                        vec.pop_back();
                        // Da stampare
                        index = dict(vec);

                        ch = next > 0 ? next : ch;
                }

                std::cout << std::format("({}, {})", index, char(ch));
                std::cout << std::format(" dict.size = {}, n_bits = {}\n", dict.size() - 1, n_bits);
                if (dict.size() != 1) {
                        bw(index, n_bits);
                }
                bw(ch, 8);


        } while (is);
        return true;
}

int main(int argc, char **argv) {
        if (!lz78encode(argv[1], argv[2], 2)) {
                return 1;
        }

        return 0;
}
