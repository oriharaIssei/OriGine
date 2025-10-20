#pragma once

#include <array>
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

static const std::array<std::string, static_cast<int32_t>(PrimitiveType::Count)> kPrimitiveTypes = {
    "Plane",
    "Ring",
    // PrimitiveType::Circle,
    "Box",
    "Sphere",
    // PrimitiveType::Torus,
    // PrimitiveType::Cylinder,
    // PrimitiveType::Cone,
};

namespace std {
std::string to_string(PrimitiveType _type);
}
