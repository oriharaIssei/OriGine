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

    /// <summary>
    /// 初期化
    /// </summary>
    void Initialize();

    /// <summary>
    /// 終了処理
    /// </summary>
    void Finalize();

    /// <summary>
    /// リソース作成
    /// </summary>
    /// <param name="_device">デバイス</param>
    /// <param name="_commandList">コマンドリスト</param>
    /// <param name="_instance">インスタンス</param>
    /// <param name="_allowUpdate">更新を許可するか</param>
    void CreateResource(
        ID3D12Device8* _device,
        ID3D12GraphicsCommandList6* _commandList,
        const std::vector<RayTracingInstance>& _instance,
        bool _allowUpdate);

    /// <summary>
    /// 更新処理
    /// </summary>
    /// <param name="_device">デバイス</param>
    /// <param name="_commandList">コマンドリスト</param>
    /// <param name="_instances">インスタンス</param>
    void Update(ID3D12Device8* _device, ID3D12GraphicsCommandList6* _commandList, const std::vector<RayTracingInstance>& _instances);

private:
    /// <summary>
    /// 行列のコピー (3x4フォーマット)
    /// </summary>
    void CopyMatrix3x4(float (&_dst)[3][4], const Matrix4x4& _src);

private:
    // スクラッチリソース
    DxResource instanceResource_;
    DxResource scratchResource_;
    DxResource resultResource_;

    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs_;
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC buildDesc_;

    bool allowUpdate_;

public:
    /// <summary>
    /// リソースの取得
    /// </summary>
    /// <returns>リソース</returns>
    ID3D12Resource* GetResource() {
        return resultResource_.GetResource().Get();
    }
};

} // namespace OriGine
