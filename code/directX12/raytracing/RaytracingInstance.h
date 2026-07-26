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
/// <remarks>
/// instanceID/mask, hitGroupIdx/flags のビット幅(24:8)は、DXR API の
/// D3D12_RAYTRACING_INSTANCE_DESC が持つビットフィールドレイアウトと一致させてある。
/// TLAS 構築時にこの構造体からインスタンスディスクリプタへ詰め替えるため、幅を合わせておく必要がある。
/// </remarks>
struct RayTracingInstance {
    Matrix4x4 matrix; // インスタンスのワールド変換行列
    UINT instanceID : 24; // SV_InstanceID
    UINT mask : 8; // レイマスク
    UINT hitGroupIdx : 24; // ShaderTableのHitGroupインデックス
    UINT flags : 8; // D3D12_RAYTRACING_INSTANCE_FLAG_*
    ID3D12Resource* blas; // 対応するBLAS
};

} // namespace OriGine
