#include "mathEnv.h"

float radianToDegree(float radian) {
    return radian * kRad2Deg;
}

float degreeToRadian(float degree) {
    return degree * kDeg2Rad;
}

std::vector<int> CalculateDigitsFromFloat(float value, int intDigits, int fracDigits) {
    std::vector<int> digits;

    value = std::fabs(value);

    // 整数部の抽出
    int intPart = static_cast<int>(value);
    for (int i = intDigits - 1; i >= 0; --i) {
        int divisor = static_cast<int>(std::pow(10, i));
        int digit   = intPart / divisor;
        digits.push_back(digit);
        intPart %= divisor;
    }

    // 小数部の抽出
    float fracPart = value - static_cast<int>(value);
    for (int i = 0; i < fracDigits; ++i) {
        fracPart *= 10.0f;
        int digit = static_cast<int>(fracPart);
        digits.push_back(digit);
        fracPart -= digit;
    }

    return digits;
}
