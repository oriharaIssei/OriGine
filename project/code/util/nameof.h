#pragma once

#include <string>
#include <typeinfo>

#define variable_name(x) #x

template <typename T>
constexpr std::string_view _cdecl nameof_view() {
    //_cdeclを付けないとMSVCの場合アプリケーションの既定呼び出し規約オプションによって文字数がずれるため。
    std::string_view name = __FUNCSIG__; // 展開例："class std::basic_string_view<char,struct std::char_traits<char> > __cdecl nameof<union TestU>(void)"
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
