#pragma once

#include <string>
#include <typeinfo>

#define variable_name(x) #x

template <typename T>
constexpr std::string nameof() {
    std::string name = typeid(T).name();
    // Remove "struct" and "class" from the name
    std::string::size_type pos;
    if ((pos = name.find("struct ")) != std::string::npos) {
        name.erase(pos, 7);
    }
    if ((pos = name.find("class ")) != std::string::npos) {
        name.erase(pos, 6);
    }
    return name;
}
