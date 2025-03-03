#pragma once

#include <string>
#include <typeinfo>

#define variable_name(x) #x

template <typename T>
constexpr std::string_view _cdecl nameof_view() {
    std::string_view name = __FUNCSIG__;
    name.remove_prefix(name[87] == ' ' ? 88 : 87); // 87文字目が' 'ならstruct,それ以外はclass,union
    name.remove_suffix(7);
    return name;
}

template <typename T>
constexpr std::string _cdecl nameof() {
    std::string_view name = __FUNCSIG__;
    name.remove_prefix(name[87] == ' ' ? 88 : 87);
    name.remove_suffix(7);
    return std::string(name);
}
