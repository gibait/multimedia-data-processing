//
// Created by giba on 04/07/23.
//

#include <cstdint>
#include <fstream>

#ifndef MULTIMEDIA_DATA_PROCESSING_READ_HEADERS_H
#define MULTIMEDIA_DATA_PROCESSING_READ_HEADERS_H

struct bitmapInfoHeader {
        uint32_t size;
        int32_t width;
        int32_t height;
        uint16_t color_planes;
        uint16_t depth;
        uint32_t compression;
        uint32_t image_size;
        int32_t horizontal;
        int32_t vertical;
        uint32_t num_colors;
        uint32_t important;
};

bool readBMPHeaders(std::ifstream &is, bitmapInfoHeader &info) {
        // Read Bitmap File Header
        uint16_t magic;
        uint32_t file_size;
        uint32_t offset;

        is.read(reinterpret_cast<char *>(&magic), 2);
        if (magic != 19778) {
                return false;
        }

        is.read(reinterpret_cast<char *>(&file_size), 4);
        // Consume reserved bytes
        is.read(reinterpret_cast<char *>(&magic), 2);
        is.read(reinterpret_cast<char *>(&magic), 2);
        is.read(reinterpret_cast<char *>(&offset), 4);

        // Read Bitmap Info Header
        is.read(reinterpret_cast<char *>(&info), 40);
        if (info.color_planes != 1) {
                return false;
        }

        return true;
}

#endif//MULTIMEDIA_DATA_PROCESSING_READ_HEADERS_H
