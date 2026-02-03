#pragma once

/// stl
#include <string>

/// math
#include <cstdint>

namespace OriGine {

/// <summary>
/// 衝突カテゴリ情報（レイヤー名とビット値）
/// </summary>
class CollisionCategory {
public:
    CollisionCategory() = default;
    CollisionCategory(const std::string& _name, uint32_t _categoryBits)
        : name_(_name), bits_(_categoryBits) {}

private:
    std::string name_ = "Default";
    uint32_t bits_    = 0; // 0 = 未登録

public:
    const std::string& GetName() const { return name_; }
    uint32_t GetBits() const { return bits_; }
};

} // namespace OriGine
