#pragma once

#include <cstdint>
#include <stdexcept>
#include <vector>

class BitArray {
public:
    BitArray(size_t size = 0) : size_(size), data_((size + 31) / 32, 0) {}

    void resize(size_t newSize) {
        size_ = newSize;
        data_.resize((newSize + 31) / 32, 0);
    }

    bool get(size_t pos) const {
        if (pos >= size_)
            throw std::out_of_range("BitArray::get");
        return (data_[pos / 32] >> (pos % 32)) & 1;
    }

    void set(size_t pos, bool value) {
        if (pos >= size_)
            throw std::out_of_range("BitArray::set");
        if (value)
            data_[pos / 32] |= (1U << (pos % 32));
        else
            data_[pos / 32] &= ~(1U << (pos % 32));
    }

    size_t size() const { return size_; }

private:
    size_t size_;
    std::vector<uint32_t> data_;
};
