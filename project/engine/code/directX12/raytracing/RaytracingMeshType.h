#pragma once

namespace OriGine {

/// <summary>
/// レイトレーシング用メッシュタイプ
/// </summary>
enum class RaytracingMeshType {
    Auto, // エンジンに任せる
    Static, // 明示的に Static
    Dynamic, // 明示的に Dynamic

    Count
};

constexpr size_t kRaytracingMeshTypeCount = static_cast<size_t>(RaytracingMeshType::Count);

const char* RaytracingMeshTypeToString(RaytracingMeshType _type);

} // namespace OriGine
