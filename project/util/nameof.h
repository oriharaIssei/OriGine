#pragma once

#include <string>
#include <typeinfo>

#define variable_name(x) #x

/// <summary>
/// 型名を文字列で取得する
/// </summary>
/// <typeparam name="T">変換対象の型</typeparam>
inline std::string ClearString(std::string name) {
    // Remove "struct " prefix
    if (size_t pos = name.find("struct "); pos != std::string::npos) {
        name.erase(pos, 7);
    }

    // Remove "class " prefix
    if (size_t pos = name.find("class "); pos != std::string::npos) {
        name.erase(pos, 6);
    }

    // Remove namespace (everything before last "::")
    if (size_t pos = name.rfind("::"); pos != std::string::npos) {
        name = name.substr(pos + 2);
    }

    return name;
}

/// <summary>
/// 型名を文字列で取得する (テンプレート引数版)
/// </summary>
template <typename T>
constexpr std::string nameof() {
    return ClearString(typeid(T).name());
}

/// <summary>
/// 型名を文字列で取得する (インスタンス参照版)
/// </summary>
template <typename T>
constexpr std::string nameof(const T&) {
    return ClearString(typeid(T).name());
}

/// <summary>
/// 型名を文字列で取得する (ポインタ版)
/// </summary>
template <typename T>
constexpr std::string nameof(T*) {
    return ClearString(typeid(T).name());
}
