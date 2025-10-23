#pragma once

#include <string>
#include <typeinfo>

#define variable_name(x) #x

/// <summary>
/// 型名を文字列で取得する
/// </summary>
/// <typeparam name="T">変換対象の型</typeparam>
template <typename T>
constexpr std::string nameof(){
    std::string name = typeid(T).name();
    // Remove "struct" and "class" from the name
    std::string::size_type pos = 0;
    if((pos = name.find("struct ")) != std::string::npos){
        name.erase(pos,7);
    }
    if((pos = name.find("class ")) != std::string::npos){
        name.erase(pos,6);
    }
    return name;
}

/// <summary>
/// 型名を文字列で取得する
/// </summary>
/// <typeparam name="T">変換対象の型</typeparam>
template <typename T>
constexpr std::string nameof(const T& /*_t*/){
    std::string name = typeid(T).name();
    // Remove "struct" and "class" from the name
    std::string::size_type pos = 0;
    if((pos = name.find("struct ")) != std::string::npos){
        name.erase(pos,7);
    }
    if((pos = name.find("class ")) != std::string::npos){
        name.erase(pos,6);
    }
    return name;
}

/// <summary>
/// 型名を文字列で取得する
/// </summary>
/// <typeparam name="T">変換対象の型</typeparam>
template <typename T>
constexpr std::string nameof(T* /*_t*/){
    std::string name = typeid(T).name();
    // Remove "struct" and "class" from the name
    std::string::size_type pos = 0;
    if((pos = name.find("struct ")) != std::string::npos){
        name.erase(pos,7);
    }
    if((pos = name.find("class ")) != std::string::npos){
        name.erase(pos,6);
    }
    return name;
}
