#pragma once

/// stl
#include <vector>

#include <concepts>
#include <stdexcept>

#include <bit>
#include <cstdint>

/// <summary>
/// 大きなビット列を管理するクラス
/// </summary>
/// <remarks>
/// ビット配列を効率的に管理するためのクラスです。内部的には整数型の配列を使用してビットを格納します。
/// </remarks>
/// <typeparam name="intergralType">bitを格納する数値タイプ</typeparam>
template <std::unsigned_integral intergralType = std::uint32_t>
class BitArray {
public:
    using BlockType                       = intergralType;
    static constexpr size_t BlockBitCount = sizeof(BlockType) * 8; // 8 は BiteからBitに変換するために

    BitArray(size_t size = 0) : size_(size), data_((size + BlockBitCount - 1) / BlockBitCount, 0) {}
    ~BitArray() {}

    /// <summary>
    /// サイズを変更する
    /// </summary>
    /// <param name="newSize"></param>
    void resize(size_t newSize);
    /// <summary>
    /// 空いているビットを1つ確保し、その位置を返す
    /// </summary>
    size_t allocateBit();

    /// <summary>
    /// 指定した位置のビットの値を取得する
    /// </summary>
    /// <param name="pos">ビットの位置</param>
    /// <returns>ビットの値(0,1)</returns>
    bool Get(size_t pos) const;

    /// <summary>
    /// trueのビットの数を取得する
    /// </summary>
    size_t GetTrueCount() const;
    /// <summary>
    /// falseのビットの数を取得する
    /// </summary>
    size_t GetFalseCount() const;

    /// <summary>
    /// 指定した位置のビットの値を設定する
    /// </summary>
    void Set(size_t pos, bool value);

    /// <summary>
    /// 内部のデータブロックを直接取得
    /// </summary>
    /// <param name="blockIndex">ブロックのインデックス</param>
    /// <returns>ブロックの値</returns>
    BlockType GetBlock(size_t blockIndex) const {
        if (blockIndex >= data_.size()) {
            throw std::out_of_range("BitArray::getBlock");
        }
        return data_[blockIndex];
    }

    /// <summary>
    /// 指定したブロックの値を設定する
    /// </summary>
    /// <param name="blockIndex"></param>
    /// <param name="value"></param>
    void SetBlock(size_t blockIndex, BlockType value) {
        if (blockIndex >= data_.size()) {
            throw std::out_of_range("BitArray::SetBlock");
        }
        data_[blockIndex] = value;
    }

    /// <summary>
    /// ビット配列のサイズを取得する
    /// </summary>
    size_t size() const { return size_; }

    /// <summary>
    /// 内部データへの参照を取得する
    /// </summary>
    /// <returns></returns>
    const std::vector<BlockType>& GetData() const { return data_; }

private:
    size_t size_;
    std::vector<BlockType> data_;
};

template <std::unsigned_integral intergralType>
void BitArray<intergralType>::resize(size_t newSize) {
    size_ = newSize;
    data_.resize((newSize + BlockBitCount - 1) / BlockBitCount, 0);
}

template <std::unsigned_integral intergralType>
inline size_t BitArray<intergralType>::allocateBit() {
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

template <std::unsigned_integral intergralType>
bool BitArray<intergralType>::Get(size_t pos) const {
    if (pos >= size_) {
        throw std::out_of_range("BitArray::get");
    }
    return (data_[pos / BlockBitCount] >> (pos % BlockBitCount)) & 1;
}

template <std::unsigned_integral intergralType>
size_t BitArray<intergralType>::GetTrueCount() const {
    size_t count = 0;
    for (const auto& block : data_) {
        count += std::popcount(block);
    }
    return count;
}

template <std::unsigned_integral intergralType>
inline size_t BitArray<intergralType>::GetFalseCount() const {
    return size_ - GetTrueCount();
}

template <std::unsigned_integral intergralType>
inline void BitArray<intergralType>::Set(size_t pos, bool value) {
    if (pos >= size_) {
        throw std::out_of_range("BitArray::Set");
    }

    size_t blockIndex = pos / BlockBitCount;
    size_t posInBlock = pos % BlockBitCount;

    if (value) {
        data_[blockIndex] |= (BlockType(1) << posInBlock);
    } else {
        data_[blockIndex] &= ~(BlockType(1) << posInBlock);
    }
}
