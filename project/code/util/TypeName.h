#pragma once

#include <string>
#include <typeinfo>

// 型 T の名前を文字列として取得するテンプレート関数
template <typename T>
std::string getTypeName() {
    return typeid(T).name();
}
