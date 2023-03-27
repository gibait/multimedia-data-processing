//
// Created by giba on 22/03/23.
//
#include <fstream>

template<typename T>
std::ostream &raw_write(std::ostream &os, const T &val, size_t size = sizeof(T)) {
        return os.write(reinterpret_cast<const char *>(&val), size);
}

class bitwriter {
        uint8_t buffer_;
        int n_ = 0;
        std::ostream& os_;

        std::ostream &write_bit(uint32_t bit) {
                buffer_ = (buffer_ << 1) | (bit & 1);
                ++n_;
                if (n_ == 8) {
                        raw_write(os_, buffer_);
                        n_ = 0;
                }
                return os_;
        }

public:
        bitwriter(std::ostream& os) : os_(os) {}

        std::ostream &write(uint32_t u, uint8_t n) {
                for (int i = n - 1; i >= 0; --i) {
                        write_bit(u >> i);
                }
                return os_;
        }

        std::ostream &operator()(uint32_t u, uint8_t n) {
                return write(u, n);
        }

        std::ostream &flush(uint32_t bit = 0) {
                while (n_ > 0) {
                        write_bit(bit);
                }
                return os_;
        }

        ~bitwriter() {
                flush();
        }
};
