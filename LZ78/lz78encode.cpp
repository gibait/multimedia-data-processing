//
// Created by giba on 23/06/23.
//

#include <cstdint>
#include <fstream>
#include <iostream>
#include <map>
#include <vector>

class bitwriter
{
        uint8_t buffer_ = 0;
        int n_ = 0;
        std::ofstream& os_;

        std::ostream& raw_write(uint8_t val, uint8_t size = sizeof(val))
        {
                return os_.write(reinterpret_cast<const char*>(&val), size);
        }

        std::ostream& write_bit(uint8_t shifted)
        {
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
        explicit bitwriter(std::ofstream& os) : os_(os) {} ;

        std::ostream& write(uint8_t byte, uint8_t size)
        {
                // Write a bit from LSB to MSB
                for (int i = size - 1; i >= 0; --i) {
                        write_bit( byte >> i );
                }
                return os_;
        }

        std::ostream& operator() (uint8_t byte, uint8_t size)
        {
                return write(byte, size);
        }

        void flush()
        {
                if (n_ > 0) {
                        raw_write(buffer_, 8);
                }
        }

        ~bitwriter() {
                flush();
        }
};

auto check_n_bits(std::map<std::vector<uint8_t>, uint8_t>& dict, int maxbits, size_t& nbits)
{
        if (nbits > unsigned(maxbits)) {
                dict.clear();
                nbits = 1;
                return false;
        } else {
                return true;
        }
}

bool lz78encode(const std::string& input_filename, const std::string& output_filename, int maxbits)
{
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

        std::map<std::vector<uint8_t>, uint8_t> dict;
        uint8_t ch;
        size_t n_bits = 1;
        bitwriter bw(os);

        os << "LZ78";
        bw(maxbits, 5);

        is.read(reinterpret_cast<char*>(&ch), sizeof(ch));
        std::vector<uint8_t> vec = {ch};
        dict[vec] = dict.size() + 1;

        bw(ch, 8);
        do {
                uint8_t next = 0;
                uint8_t index = 0;

                if (is.peek() == EOF) {
                        break;
                }
                
                n_bits = std::bit_width(dict.size());
                check_n_bits(dict, maxbits, n_bits);

                is.read(reinterpret_cast<char*>(&ch), sizeof(ch));
                vec = {ch};

                if (!dict.contains(vec)) {        
                        dict[vec] = dict.size() + 1;
                } else {
                        do {
                                if (is.peek() == EOF) {
                                        break;
                                }
                                is.read(reinterpret_cast<char*>(&next), sizeof(next));
                                vec.push_back(next);
                        } while (dict.contains(vec));

                        // Aggiungiamo il nuovo carattere alla sequenza più lunga trovata
                        dict[vec] = dict.size() + 1;
                        // Lo eliminiamo dal vettore per ricava l'indice del dizionario da stampare
                        // della sequenza più lunga trovata fin'ora
                        vec.pop_back();
                        // Da stampare
                        index = dict[vec];

                        ch = next > 0 ? next : ch;
                }
                                                
                if (dict.size() != 1) {
                        bw(index, n_bits);
                }
                bw(ch, 8);
                
                
        } while (is);
        return true;
}
