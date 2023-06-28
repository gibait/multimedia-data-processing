//
// Created by giba on 22/05/23.
//

#ifndef MULTIMEDIA_DATA_PROCESSING_PARSE_HEADER_H
#define MULTIMEDIA_DATA_PROCESSING_PARSE_HEADER_H

#include <cstdint>
#include <fstream>
#include <iostream>
#include <sstream>

struct PAM_t {
        int width;
        int height;
        int depth;
        int maxval;
        std::string tupltype;
};

bool parse_header(std::ifstream &is, PAM_t &header) {
        std::string line;
        getline(is, line);
        if (line != "P7") {
                return false;
        }

        while (true) {
                getline(is, line);
                std::stringstream ss(line);

                std::string type;
                ss >> type;
                if (type == "WIDTH") {
                        ss >> header.width;
                } else if (type == "HEIGHT") {
                        ss >> header.height;
                } else if (type == "DEPTH") {
                        ss >> header.depth;
                        if (header.depth != 3 && header.depth != 1) {
                                return false;
                        }
                } else if (type == "MAXVAL") {
                        ss >> header.maxval;
                        if (header.maxval != 255) {
                                return false;
                        }
                } else if (type == "TUPLTYPE") {
                        ss >> header.tupltype;
                        if (header.tupltype != "RGB" && header.tupltype != "GRAYSCALE") {
                                return false;
                        }
                } else if (type == "ENDHDR") {
                        break;
                } else {
                        return false;
                }
        }
        return true;
}

#endif//MULTIMEDIA_DATA_PROCESSING_PARSE_HEADER_H
