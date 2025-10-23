#pragma once

#include "GlobalVariables.h"

/// <summary>
/// GlobalVariablesを介してシリアライズされる値
/// </summary>
/// <typeparam name="T"></typeparam>
template <typename T>
class SerializedField {
public:
    // コンストラクタ
    SerializedField(const std::string& scene, const std::string& group, const std::string& value) {
        value_ = GlobalVariables::getInstance()->addValue<T>(scene, group, value);
    }
    SerializedField(const std::string& scene, const std::string& group, const std::string& value, const T& defaultValue) {
        value_ = GlobalVariables::getInstance()->addValue<T>(scene, group, value);
        // 値が nullptr の場合はデフォルト値を設定
        if (!value_) {
            setValue(defaultValue);
        }
    }

    SerializedField()
        : value_(nullptr) {}

    // デストラクタ
    ~SerializedField() {}

    /// <summary>
    /// 実態がないSerializedFieldを作成する
    /// </summary>
    /// <returns></returns>
    static SerializedField CreateNull() {
        return SerializedField<T>();
    }

    /// <summary>
    /// 値を読み取る
    /// </summary>
    /// <returns></returns>
    const T* operator->() const { return value_; }
    /// <summary>
    /// 値を読み取る
    /// </summary>
    /// <returns></returns>
    const T* GetValue() const {
        return value_;
    }

    // 値を取得（暗黙変換用）
    operator T() const {
        if (value_) {
            return *value_;
        }
        throw std::runtime_error("Attempted to access uninitialized value.");
    }
    operator const T*() const {
        if (value_) {
            return value_;
        }
        throw std::runtime_error("Attempted to access uninitialized value.");
    }
    operator T*() const {
        if (value_) {
            return value_;
        }
        throw std::runtime_error("Attempted to access uninitialized value.");
    }

    template <typename U>
        requires std::is_same_v<U, T>
    bool operator==(const U& other) const {
        return value_ && (*value_ == other);
    }

    template <typename U>
        requires std::is_same_v<U, T>
    bool operator!=(const U& other) const {
        return value_ && (*value_ != other);
    }

    /// <summary>
    /// 値の型が一致する場合に別の型として取得する
    /// </summary>
    /// <typeparam name="U"></typeparam>
    /// <returns></returns>
    template <typename U>
    U as() const {
        if (value_) {
            return static_cast<U>(*value_);
        }
        throw std::runtime_error("Attempted to access uninitialized value.");
    }

    // 値を設定する
    void setValue(const T& newValue) {
        if (value_) {
            *value_ = newValue;
        }
    }

private:
    T* value_;
};

template <typename T, typename U>
    requires std::is_same_v<U, T>
bool operator==(const U& other, const SerializedField<T>& serializeField) {
    return (serializeField == other);
}

template <typename T, typename U>
    requires std::is_same_v<U, T>
bool operator!=(const U& other, const SerializedField<T>& serializeField) {
    return (serializeField != other);
}
