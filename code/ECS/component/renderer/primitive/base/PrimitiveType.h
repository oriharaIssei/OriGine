#pragma once

/// stl
#include <array>
#include <string>
/// math
#include <cstdint>

namespace OriGine {

/// <summary>
/// 生成可能なプリミティブ形状の種類。
/// </summary>
enum class PrimitiveType : int32_t {
    Plane, // 面
    Ring, // 環(真ん中が空洞)
    // Circle, // 円
    Box, // 立方体
    Sphere, // 球
    //  Torus, // トーラス
    Cylinder, // 円柱
    //  Cone // 円錐

    Count
};

/// PrimitiveTypeの各値に対応する名前一覧（Countと同じ並び順）
static const std::array<std::string, static_cast<int32_t>(PrimitiveType::Count)> kPrimitiveTypes = {
    "Plane",
    "Ring",
    // PrimitiveType::Circle,
    "Box",
    "Sphere",
    // PrimitiveType::Torus,
    "Cylinder",
    // PrimitiveType::Cone,
};

} // namespace OriGine

namespace std {
/// PrimitiveTypeを文字列名に変換する
std::string to_string(OriGine::PrimitiveType _type);
}
