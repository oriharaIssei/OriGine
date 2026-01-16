#include "Version.h"

std::string Version::toString() const {
    return std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(patch);
}

void Version::fromString(const std::string& _str) {
    // . を区切り文字として使用して、major、minor、patch を抽出する
    size_t firstDot  = _str.find('.');
    size_t secondDot = _str.find('.', firstDot + 1);
    if (firstDot != std::string::npos && secondDot != std::string::npos) {
        major = std::stoul(_str.substr(0, firstDot));
        minor = std::stoul(_str.substr(firstDot + 1, secondDot - firstDot - 1));
        patch = std::stoul(_str.substr(secondDot + 1));
    }
}
