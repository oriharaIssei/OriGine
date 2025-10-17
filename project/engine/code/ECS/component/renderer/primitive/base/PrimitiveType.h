#pragma once

#include <string>

#include <cstdint>

enum class PrimitiveType : int32_t {
    Plane, // 面
    Ring, // 環(真ん中が空洞)
    // Circle, // 円
    Box, // 立方体
    Sphere, // 球
    //  Torus, // トーラス
    //  Cylinder, // 円柱
    //  Cone // 円錐

    Count
};

namespace std {
std::string to_string(PrimitiveType _type);
}
