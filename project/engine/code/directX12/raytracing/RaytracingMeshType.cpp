#include "RaytracingMeshType.h"

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
