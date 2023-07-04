//
// Created by giba on 04/07/23.
//

#include <array>
#include <cstdint>
#include <vector>

#ifndef MULTIMEDIA_DATA_PROCESSING_MATRIX_H
#define MULTIMEDIA_DATA_PROCESSING_MATRIX_H

using rgb = std::array<uint8_t, 3>;

template<typename T>
struct mat {
        std::vector<T> data_;
        int rows_, cols_;

        mat(int rows, int cols) : rows_(rows), cols_(cols), data_(rows * cols){};

        T &operator()(int r, int c) {
                return data_[r * cols_ + c];
        }

        const T &operator()(int r, int c) const {
                return data_[r * cols_ + c];
        }

        void resize(int r, int c) {
                rows_ = r;
                cols_ = c;
                data_.resize(r * c);
        }

        auto size() { return cols_ * rows_; }
        auto rawsize() { return size() * sizeof(T); }
        auto rawdata() { return reinterpret_cast<char *>(data_.data()); }
};

#endif//MULTIMEDIA_DATA_PROCESSING_MATRIX_H
