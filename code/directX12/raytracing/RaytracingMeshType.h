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

constexpr size_t kRaytracingMeshTypeCount = static_cast<size_t>(RaytracingMeshType::Count); // RaytracingMeshTypeの要素数

/// <summary>
/// RaytracingMeshType を文字列表現に変換する.
/// </summary>
/// <param name="_type">変換対象の値</param>
/// <returns>種別を表す文字列</returns>
const char* RaytracingMeshTypeToString(RaytracingMeshType _type);

} // namespace OriGine
