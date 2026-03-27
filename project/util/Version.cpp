#include "Version.h"

std::string Version::toString() const {
    return std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(patch);
}

void Version::fromString(const std::string& str) {
    // . を区切り文字として使用して、major、minor、patch を抽出する
    size_t firstDot  = str.find('.');
    size_t secondDot = str.find('.', firstDot + 1);
    if (firstDot != std::string::npos && secondDot != std::string::npos) {
        major = std::stoul(str.substr(0, firstDot));
        minor = std::stoul(str.substr(firstDot + 1, secondDot - firstDot - 1));
        patch = std::stoul(str.substr(secondDot + 1));
    }
}
