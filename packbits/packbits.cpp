#include <cstdint>
#include <fstream>
#include <iostream>
#include <vector>

#define ERROR (-1)
#define SUCCESS (0)


void compress(std::ifstream& is, std::ofstream& os) {
        uint8_t ch, old;

        is.read(reinterpret_cast<char*>(&ch), 1);

        do {
                size_t run_counter = 1;
                std::vector<uint8_t> copies;

                old = ch;
                is.read(reinterpret_cast<char*>(&ch), 1);

                if (ch != old) {
                        copies.push_back(old);
                        old = ch;
                        if (is.peek() == EOF) {
                                copies.push_back(ch);
                        }
                        is.read(reinterpret_cast<char*>(&ch), 1);

                        while (ch != old && copies.size() < 128) {

                                copies.push_back(old);
                                old = ch;
                                if (is.peek() == EOF) {
                                        copies.push_back(ch);
                                        break;
                                }

                                is.read(reinterpret_cast<char*>(&ch), 1);
                        }
                        if (is.peek() != EOF) {
                                is.seekg(-1, std::ios::cur);
                        }
                        if (copies.size() == 128) {
                                ch = old;
                        }

                        uint8_t size = copies.size() - 1;
                        os.write(reinterpret_cast<char*>(&size), sizeof(size));
                        os.write(reinterpret_cast<char*>(copies.data()), copies.size());

                } else if (ch == old) {
                        while (ch == old && run_counter < 128) {
                                run_counter++;
                                if (is.peek() == EOF) {
                                        break;
                                }
                                old = ch;
                                is.read(reinterpret_cast<char*>(&ch), 1);
                        }

                        if (run_counter > 1) {
                                uint8_t to_write = 257 - run_counter;
                                os.write(reinterpret_cast<char*>(&to_write), sizeof(to_write));
                                os.write(reinterpret_cast<char*>(&old), sizeof(old));
                        }
                }

        } while (is);

        uint8_t eod = 128;
        os.write(reinterpret_cast<char*>(&eod), sizeof(eod));
}

void decompress(std::ifstream& is, std::ofstream& os) {
        do {
                uint8_t cmd, ch;
                is.read(reinterpret_cast<char*>(&cmd), sizeof(cmd));
                if (cmd < 128) {
                        size_t i = cmd + 1;
                        while (i-- > 0) {
                                is.read(reinterpret_cast<char*>(&ch), sizeof(ch));
                                os.write(reinterpret_cast<char*>(&ch), sizeof(ch));
                        }
                } else if (cmd > 128) {
                        is.read(reinterpret_cast<char*>(&ch), sizeof(ch));
                        size_t i = 257 - cmd;
                        while (i-- > 0) {
                                os.write(reinterpret_cast<char*>(&ch), sizeof(ch));
                        }
                } else {
                        break;
                }
        } while (is);
}

int main(int argc, char** argv) {
        if (argc != 4) {
                std::cout << "Numero di parametri errato";
                return ERROR;
        }

        std::ifstream is;
        std::ofstream os;

        switch (argv[1][0]) {
                case 'c':
                        is.open(argv[2]);
                        if (!is) {
                                std::cout << "Errore nell'apertura del file di input";
                                return ERROR;
                        }

                        os.open(argv[3] + std::string(".packbits"), std::ios::binary);
                        if (!os) {
                                std::cout << "Errore nell'apertura del file di output";
                                return ERROR;
                        }

                        compress(is, os);
                        break;
                case 'd':
                        if (!std::string(argv[2]).ends_with(".bin") and !std::string(argv[2]).ends_with(".packbits")) {
                                std::cout << "Il file non è in formato packbits";
                                return ERROR;
                        }

                        is.open(argv[2], std::ios::binary);
                        if (!is) {
                                std::cout << "Errore nell'apertura del file di input";
                                return ERROR;
                        }

                        os.open(argv[3] + std::string(".txt"), std::ios::binary);
                        if (!os) {
                                std::cout << "Errore nell'apertura del file di output";
                                return ERROR;
                        }

                        decompress(is, os);
                        break;
                default:
                        std::cout << "Opzione non riconosciuta";
                        return ERROR;
        }

        return SUCCESS;
}