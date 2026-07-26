#include "RaytracingMeshType.h"

/// <summary>
/// RaytracingMeshType を文字列表現に変換する。
/// </summary>
const char* OriGine::RaytracingMeshTypeToString(RaytracingMeshType _type) {
    switch (_type) {
    case RaytracingMeshType::Auto:
        return "Auto";
    case RaytracingMeshType::Static:
        return "Static";
    case RaytracingMeshType::Dynamic:
        return "Dynamic";
    default:
        return "Unknown";
    }
}
