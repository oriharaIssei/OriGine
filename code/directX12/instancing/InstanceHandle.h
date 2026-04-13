#pragma once

/// stl
#include <cstdint>
#include <functional>
#include <limits>

namespace OriGine {

/// 前方宣言
struct ModelMeshData;

/// <summary>
/// インスタンシング描画における個別インスタンスを識別するためのハンドル.
/// InstancedMeshManager から発行され、インスタンスの更新・削除に使用する.
/// </summary>
struct InstanceHandle {
    /// <summary>所属するモデルグループ（ModelMeshData へのポインタ）</summary>
    ModelMeshData* group = nullptr;
    /// <summary>DenseSlotMap 内の安定ID</summary>
    uint32_t id = kInvalidId;

    /// <summary>無効IDを表す定数</summary>
    static constexpr uint32_t kInvalidId = (std::numeric_limits<uint32_t>::max)();

    /// <summary>ハンドルが有効かどうかを返す</summary>
    bool IsValid() const noexcept { return group != nullptr && id != kInvalidId; }

    /// <summary>一致比較</summary>
    bool operator==(const InstanceHandle&) const = default;
};

} // namespace OriGine

namespace std {

/// <summary>
/// OriGine::InstanceHandle を std::unordered_map 等のキーとして使用するためのハッシュ関数.
/// </summary>
template <>
struct hash<OriGine::InstanceHandle> {
    std::size_t operator()(const OriGine::InstanceHandle& h) const noexcept {
        constexpr size_t kHashCombine64 = 0x9e3779b97f4a7c15ULL;
        std::size_t seed               = 0;

        seed ^= std::hash<const void*>{}(h.group)
                + kHashCombine64
                + (seed << 6)
                + (seed >> 2);

        seed ^= std::hash<uint32_t>{}(h.id)
                + kHashCombine64
                + (seed << 6)
                + (seed >> 2);

        return seed;
    }
};

} // namespace std
