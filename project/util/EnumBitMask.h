#pragma once

#include <type_traits>

/// <summary>
/// 列挙型のビットマスクを扱うためのクラス
/// </summary>
/// <typeparam name="E"></typeparam>
template <typename E>
class EnumBitmask {
    using EnumType       = E;
    using UnderlyingType = std::underlying_type_t<E>;

    static_assert(std::is_enum_v<EnumType>, "EnumBitmask requires enum type");

public:
    constexpr EnumBitmask() noexcept : value_(0) {}
    constexpr EnumBitmask(EnumType e) noexcept : value_(static_cast<UnderlyingType>(e)) {}
    constexpr EnumBitmask(UnderlyingType v) noexcept : value_(v) {}

    // ビット演算
    constexpr EnumBitmask operator|(EnumBitmask rhs) const noexcept { return value_ | rhs.value_; }
    constexpr EnumBitmask operator&(EnumBitmask rhs) const noexcept { return value_ & rhs.value_; }
    constexpr EnumBitmask operator^(EnumBitmask rhs) const noexcept { return value_ ^ rhs.value_; }
    constexpr EnumBitmask operator~() const noexcept { return ~value_; }

    constexpr EnumBitmask& operator|=(EnumBitmask rhs) noexcept {
        value_ |= rhs.value_;
        return *this;
    }
    constexpr EnumBitmask& operator&=(EnumBitmask rhs) noexcept {
        value_ &= rhs.value_;
        return *this;
    }
    constexpr EnumBitmask& operator^=(EnumBitmask rhs) noexcept {
        value_ ^= rhs.value_;
        return *this;
    }

    // 比較演算子
    constexpr bool operator==(EnumBitmask rhs) const noexcept { return value_ == rhs.value_; }
    constexpr bool operator!=(EnumBitmask rhs) const noexcept { return value_ != rhs.value_; }
    constexpr bool operator<(EnumBitmask rhs) const noexcept { return value_ < rhs.value_; }
    constexpr bool operator<=(EnumBitmask rhs) const noexcept { return value_ <= rhs.value_; }
    constexpr bool operator>(EnumBitmask rhs) const noexcept { return value_ > rhs.value_; }
    constexpr bool operator>=(EnumBitmask rhs) const noexcept { return value_ >= rhs.value_; }

    constexpr bool operator==(UnderlyingType rhs) const noexcept { return value_ == rhs; }
    constexpr bool operator!=(UnderlyingType rhs) const noexcept { return value_ != rhs; }
    constexpr bool operator<(UnderlyingType rhs) const noexcept { return value_ < rhs; }
    constexpr bool operator<=(UnderlyingType rhs) const noexcept { return value_ <= rhs; }
    constexpr bool operator>(UnderlyingType rhs) const noexcept { return value_ > rhs; }
    constexpr bool operator>=(UnderlyingType rhs) const noexcept { return value_ >= rhs; }

    constexpr EnumBitmask& operator=(EnumType e) noexcept {
        value_ = static_cast<UnderlyingType>(e);
        return *this;
    }
    constexpr EnumBitmask& operator=(UnderlyingType v) noexcept {
        value_ = v;
        return *this;
    }
    constexpr EnumBitmask& operator=(EnumBitmask rhs) noexcept {
        value_ = rhs.value_;
        return *this;
    }

    // 数値変換
    constexpr explicit operator UnderlyingType() const noexcept { return value_; }
    constexpr explicit operator bool() const noexcept { return value_ != 0; }

    // Enum値取得
    constexpr EnumType ToEnum() const noexcept { return static_cast<E>(value_); }

protected:
    UnderlyingType value_;
};
