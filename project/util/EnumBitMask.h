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
    constexpr EnumBitmask(EnumType _e) noexcept : value_(static_cast<UnderlyingType>(_e)) {}
    constexpr EnumBitmask(UnderlyingType _v) noexcept : value_(_v) {}

    /// <summary>
    /// 指定したフラグが立っているかどうかを取得する
    /// </summary>
    /// <param name="flag"></param>
    /// <returns></returns>
    bool HasFlag(EnumType _flag) const noexcept {
        return (value_ & static_cast<UnderlyingType>(_flag)) != 0;
    }
    /// <summary>
    /// 指定したフラグが立っているかどうかを取得する
    /// </summary>
    /// <param name="flag"></param>
    /// <returns></returns>
    bool HasFlag(UnderlyingType _flag) const noexcept {
        return (value_ & _flag) != 0;
    }

    /// <summary>
    /// 指定したフラグをクリアする
    /// </summary>
    /// <param name="flag"></param>
    void SetFlag(EnumType _flag) noexcept {
        value_ |= static_cast<UnderlyingType>(_flag);
    }
    /// <summary>
    /// 指定したフラグをクリアする
    /// </summary>
    /// <param name="flag"></param>
    void SetFlag(UnderlyingType _flag) noexcept {
        value_ |= _flag;
    }

    /// <summary>
    /// 指定したフラグをクリアする
    /// </summary>
    void ClearFlag(EnumType _flag) noexcept {
        value_ &= ~static_cast<UnderlyingType>(_flag);
    }
    /// <summary>
    /// 指定したフラグをクリアする
    /// </summary>
    void ClearFlag(UnderlyingType _flag) noexcept {
        value_ &= ~_flag;
    }

    // ビット演算
    constexpr EnumBitmask operator|(EnumBitmask _rhs) const noexcept { return value_ | _rhs.value_; }
    constexpr EnumBitmask operator&(EnumBitmask _rhs) const noexcept { return value_ & _rhs.value_; }
    constexpr EnumBitmask operator^(EnumBitmask _rhs) const noexcept { return value_ ^ _rhs.value_; }
    constexpr EnumBitmask operator~() const noexcept { return ~value_; }

    constexpr EnumBitmask& operator|=(EnumBitmask _rhs) noexcept {
        value_ |= _rhs.value_;
        return *this;
    }
    constexpr EnumBitmask& operator&=(EnumBitmask _rhs) noexcept {
        value_ &= _rhs.value_;
        return *this;
    }
    constexpr EnumBitmask& operator^=(EnumBitmask _rhs) noexcept {
        value_ ^= _rhs.value_;
        return *this;
    }

    // 比較演算子
    constexpr bool operator==(EnumBitmask _rhs) const noexcept { return value_ == _rhs.value_; }
    constexpr bool operator!=(EnumBitmask _rhs) const noexcept { return value_ != _rhs.value_; }
    constexpr bool operator<(EnumBitmask _rhs) const noexcept { return value_ < _rhs.value_; }
    constexpr bool operator<=(EnumBitmask _rhs) const noexcept { return value_ <= _rhs.value_; }
    constexpr bool operator>(EnumBitmask _rhs) const noexcept { return value_ > _rhs.value_; }
    constexpr bool operator>=(EnumBitmask _rhs) const noexcept { return value_ >= _rhs.value_; }

    constexpr bool operator==(UnderlyingType _rhs) const noexcept { return value_ == _rhs; }
    constexpr bool operator!=(UnderlyingType _rhs) const noexcept { return value_ != _rhs; }
    constexpr bool operator<(UnderlyingType _rhs) const noexcept { return value_ < _rhs; }
    constexpr bool operator<=(UnderlyingType _rhs) const noexcept { return value_ <= _rhs; }
    constexpr bool operator>(UnderlyingType _rhs) const noexcept { return value_ > _rhs; }
    constexpr bool operator>=(UnderlyingType _rhs) const noexcept { return value_ >= _rhs; }

    constexpr EnumBitmask& operator=(EnumType _e) noexcept {
        value_ = static_cast<UnderlyingType>(_e);
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
    constexpr UnderlyingType ToUnderlying() const noexcept { return value_; }

protected:
    UnderlyingType value_;
};
