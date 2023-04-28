//
// Created by giba on 17/04/23.
//

#ifndef MULTIMEDIA_DATA_PROCESSING_MATRIX_H
#define MULTIMEDIA_DATA_PROCESSING_MATRIX_H

#include <cassert>
#include <vector>

template<typename T>
class mat {
        int rows_, cols_;
        std::vector<T> data_;

public:
        mat(int rows = 0, int cols = 0) : rows_(rows), cols_(cols), data_(rows * cols) {}


        T& operator() (int r, int c) {
                assert(r >= 0 && r <= rows_ && r >= 0 && c <= cols_);
                return data_[r * cols_ + c];
        }

        T& operator() (int r, int c) const {
                assert(r >= 0 && r < rows_ && r >= 0 && c < cols_);
                return data_[r * cols_ + c];
        }


        void resize(int rows, int cols)
        {
                rows_ = rows;
                cols_ = cols;
                data_.resize(rows * cols);
        }

        int rows() const { return rows_; }
        int columns() const { return cols_; }
        int size() const { return rows_ * cols_; }
        int rawsize() const { return size() * sizeof(T); }

        char* rawdata() { return reinterpret_cast<char*>(data_.data()); }

};

#endif//MULTIMEDIA_DATA_PROCESSING_MATRIX_H
