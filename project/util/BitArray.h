#pragma once

#include <concepts>
#include <cstdint>
#include <stdexcept>
#include <vector>

template <std::unsigned_integral intergralType = std::uint32_t>
class BitArray {
public:
    using BlockType                       = intergralType;
    static constexpr size_t BlockBitCount = sizeof(BlockType) * 8; // 8 は BiteからBitに変換するために

    BitArray(size_t size = 0) : size_(size), data_((size + BlockBitCount - 1) / BlockBitCount, 0) {}
    ~BitArray() {}

    void resize(size_t newSize) {
        size_ = newSize;
        data_.resize((newSize + BlockBitCount - 1) / BlockBitCount, 0);
    }
    size_t allocateBit() {
        for (size_t i = 0; i < data_.size(); ++i) {
            if (data_[i] != BlockType(-1)) { // 全てのビットが1でない場合
                for (size_t j = 0; j < BlockBitCount; ++j) {
                    if ((data_[i] & (BlockType(1) << j)) == 0) { // ビットが0の場合
                        data_[i] |= (BlockType(1) << j); // ビットをセット

                        return i * BlockBitCount + j; // ビットの位置を返す
                    }
                }
            }
        }
        throw std::runtime_error("No free bits available");
    }

    bool get(size_t pos) const {
        if (pos >= size_) {
            throw std::out_of_range("BitArray::get");
        }
        return (data_[pos / BlockBitCount] >> (pos % BlockBitCount)) & 1;
    }

    size_t getTrueCount() const {
        size_t count = 0;
        for (const auto& block : data_) {
            count += std::popcount(block); // GCC/Clangのビルトイン関数を使用
        }
        return count;
    }
    size_t getFalseCount() const {
        return size_ - getTrueCount();
    }

    void set(size_t pos, bool value) {
        if (pos >= size_) {
            throw std::out_of_range("BitArray::set");
        }

        size_t blockIndex = pos / BlockBitCount;
        size_t posInBlock = pos % BlockBitCount;

        if (value) {
            data_[blockIndex] |= (BlockType(1) << posInBlock);
        } else {
            data_[blockIndex] &= ~(BlockType(1) << posInBlock);
        }
    }

    size_t size() const { return size_; }

private:
    size_t size_;
    std::vector<BlockType> data_;
};
