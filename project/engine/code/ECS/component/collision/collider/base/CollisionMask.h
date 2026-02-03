#pragma once

/// stl
#include <string>

/// math
#include <cstdint>

namespace OriGine {

/// <summary>
/// 衝突マスク（どのカテゴリと衝突するかのビットフラグ）
/// </summary>
class CollisionMask {
private:
    uint32_t bits_ = 0xFFFFFFFF; // デフォルトは全カテゴリと衝突

public:
    CollisionMask() = default;
    explicit CollisionMask(uint32_t _bits) : bits_(_bits) {}

    // ビット操作
    void Reset() { bits_ = 0; }
    void SetAll() { bits_ = 0xFFFFFFFF; }
    void Set(uint32_t layer) { bits_ |= (1u << layer); }
    void Clear(uint32_t layer) { bits_ &= ~(1u << layer); }
    void Toggle(uint32_t layer) { bits_ ^= (1u << layer); }
    bool Has(uint32_t layer) const { return (bits_ & (1u << layer)) != 0; }

    // ビット値で直接操作
    void SetBits(uint32_t _bits) { bits_ |= _bits; }
    void ClearBits(uint32_t _bits) { bits_ &= ~_bits; }

    // カテゴリ名からセット（Manager経由）
    void Set(const std::string& _categoryName);

    // 衝突判定（相手のlayerビットが自分のmaskに含まれるか）
    bool CanCollideWith(uint32_t otherCategoryBits) const {
        return (bits_ & otherCategoryBits) != 0;
    }

    // Raw値アクセス
    uint32_t GetRaw() const { return bits_; }
    void SetRaw(uint32_t v) { bits_ = v; }
};

} // namespace OriGine
