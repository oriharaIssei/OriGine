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
    /// <summary>
    /// カテゴリ名とビット値を指定して生成する（衝突マスクはデフォルトで全カテゴリと衝突）
    /// </summary>
    /// <param name="_name">カテゴリ名</param>
    /// <param name="_categoryBits">カテゴリを表すビット値</param>
    CollisionCategory(const std::string& _name, uint32_t _categoryBits)
        : name_(_name), bits_(_categoryBits), maskBits_(0xFFFFFFFF) {}

private:
    std::string name_  = "Default"; // カテゴリ名
    uint32_t bits_     = 0; // 0 = 未登録
    uint32_t maskBits_ = 0xFFFFFFFF; // 衝突マスク（デフォルトは全カテゴリ）

public:
    const std::string& GetName() const { return name_; }
    uint32_t GetBits() const { return bits_; }
    uint32_t GetMaskBits() const { return maskBits_; }
    void SetMaskBits(uint32_t _mask) { maskBits_ = _mask; }

    /// <summary>
    /// 相手のカテゴリと衝突可能か判定
    /// </summary>
    bool CanCollideWith(uint32_t _otherCategoryBits) const {
        return (maskBits_ & _otherCategoryBits) != 0;
    }
};

} // namespace OriGine
