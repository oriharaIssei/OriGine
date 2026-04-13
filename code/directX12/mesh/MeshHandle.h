#pragma once

/// ECS
// component
#include "component/ComponentHandle.h"

/// math
#include <cstdint>

namespace OriGine {

/// <summary>
/// MeshRenderer コンポーネント内の特定のサブメッシュを指し示すための識別子.
/// MeshRenderer は複数のメッシュを保持するため、コンポーネントのハンドルとメッシュインデックスのペアで管理する.
/// </summary>
struct MeshHandle {
    /// <summary>MeshRenderer コンポーネントへのハンドル</summary>
    ComponentHandle handle = {};
    /// <summary>MeshRenderer 内のメッシュインデックス</summary>
    uint32_t meshIndex = 0;

    /// <summary>一致比較（合成比較）</summary>
    bool operator==(const MeshHandle&) const = default;
};

} // namespace OriGine

namespace std {

/// <summary>
/// OriGine::MeshHandle を std::unordered_map 等のキーとして使用するためのハッシュ関数定義.
/// </summary>
template <>
struct hash<OriGine::MeshHandle> {
    std::size_t operator()(const OriGine::MeshHandle& h) const noexcept {
        constexpr size_t kHashCombine64 = 0x9e3779b97f4a7c15ULL;
        std::size_t seed                = 0;

        seed ^= std::hash<OriGine::ComponentHandle>{}(h.handle)
                + kHashCombine64
                + (seed << 6)
                + (seed >> 2);

        seed ^= std::hash<uint32_t>{}(h.meshIndex)
                + kHashCombine64
                + (seed << 6)
                + (seed >> 2);

        return seed;
    }
};

}
