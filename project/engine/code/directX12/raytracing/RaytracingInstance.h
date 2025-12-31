#pragma once

/// windows
#include <d3d12.h>
#include <Windows.h>

/// math
#include "math/Matrix4x4.h"

namespace OriGine {

/// <summary>
/// レイトレーシングインスタンス情報
/// </summary>
struct RayTracingInstance {
    Matrix4x4 matrix;
    UINT instanceID : 24; // SV_InstanceID
    UINT mask : 8; // レイマスク
    UINT hitGroupIdx : 24; // ShaderTableのHitGroupインデックス
    UINT flags : 8; // D3D12_RAYTRACING_INSTANCE_FLAG_*
    ID3D12Resource* blas; // 対応するBLAS
};

} // namespace OriGine
