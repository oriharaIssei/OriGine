#pragma once

/// ECS
// component
#include "component/ComponentHandle.h"

/// math
#include <cstdint>

namespace OriGine {

/// <summary>
/// MeshRendererから特定のメッシュを識別するためのハンドル。
/// MeshRendererがマルチメッシュ前提なので、メッシュグループ内のインデックスも保持する。
/// </summary>
struct MeshHandle {
    ComponentHandle handle = {};
    uint32_t meshIndex     = 0;

    bool operator==(const MeshHandle&) const = default;
};

} // namespace OriGine

namespace std {

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
