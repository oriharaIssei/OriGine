#pragma once

/// d3d12
#include <d3d12.h>
/// stl
#include <vector>

/// engine
// directX12
#include "directX12/DxResource.h"
#include "directX12/raytracing/RayTracingInstance.h"

/// math
#include "math/Matrix4x4.h"

namespace OriGine {

/// <summary>
/// TLAS (Top Level Acceleration Structure)
///     シーン全体の構造を管理し、複数のBLASを組み合わせて使用する。
/// </summary>
class TopLevelAccelerationStructure {
public:
    TopLevelAccelerationStructure();
    ~TopLevelAccelerationStructure();

    void Initialize();
    void Finalize();

    void CreateResource(
        ID3D12Device8* _device,
        ID3D12GraphicsCommandList6* _commandList,
        const std::vector<RayTracingInstance>& _instance,
        bool _allowUpdate);

    void Update(ID3D12Device8* _device, ID3D12GraphicsCommandList6* _commandList, const std::vector<RayTracingInstance>& _instances);

private:
    void CopyMatrix3x4(float (&dst)[3][4], const Matrix4x4& src);

private:
    // スクラッチリソース
    DxResource instanceResource_;
    DxResource scratchResource_;
    DxResource resultResource_;

    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs_;
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC buildDesc_;

    bool allowUpdate_;

public:
    ID3D12Resource* GetResource() {
        return resultResource_.GetResource().Get();
    }
};

} // namespace OriGine
