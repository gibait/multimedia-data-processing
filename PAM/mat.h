//
// Created by giba on 28/06/23.
//

#include <vector>

#ifndef MULTIMEDIA_DATA_PROCESSING_MAT_H
#define MULTIMEDIA_DATA_PROCESSING_MAT_H

template<typename T>
class mat {
        int rows_, cols_;
        std::vector<T> data_;

public:
        mat() {}
        mat(int r, int c) : rows_(r), cols_(c), data_(rows_ * cols_) {}

        T &operator()(int r, int c) {
                return data_[r * cols_ + c];
        }

        void resize(int r, int c) {
                rows_ = r;
                cols_ = c;
                data_.resize(rows_ * cols_);
        }

        auto rows() { return rows_; }
        auto cols() { return cols_; }
        auto size() { return cols_ * rows_; }
        auto rawsize() { return size() * sizeof(T); }
        auto rawdata() { return reinterpret_cast<char *>(data_.data()); }
};

#endif//MULTIMEDIA_DATA_PROCESSING_MAT_H
