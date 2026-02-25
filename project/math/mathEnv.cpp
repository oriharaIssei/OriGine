#include "MathEnv.h"

namespace OriGine {

float RadianToDegree(float _radian) {
    return _radian * kRad2Deg;
}

float DegreeToRadian(float _degree) {
    return _degree * kDeg2Rad;
}

std::vector<int> CalculateDigitsFromFloat(float _value, int _intDigits, int _fracDigits) {
    std::vector<int> digits;

    _value = std::fabs(_value);

    // 整数部の抽出
    int intPart = static_cast<int>(_value);
    for (int i = _intDigits - 1; i >= 0; --i) {
        int divisor = static_cast<int>(std::pow(10, i));
        int digit   = intPart / divisor;
        digits.push_back(digit);
        intPart %= divisor;
    }

    // 小数部の抽出
    float fracPart = _value - static_cast<int>(_value);
    for (int i = 0; i < _fracDigits; ++i) {
        fracPart *= 10.0f;
        int digit = static_cast<int>(fracPart);
        digits.push_back(digit);
        fracPart -= digit;
    }

    return digits;
}

} // namespace OriGine
