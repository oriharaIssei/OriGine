#pragma once

/// engine
// directX12
#include "directX12/DxDescriptor.h"
#include "directX12/DxResource.h"
#include "directX12/mesh/Mesh.h"

namespace OriGine {

/// <summary>
/// Bottom Level Acceleration Structure (BLAS)
/// 頂点データやインデックスデータなどのジオメトリ情報を格納し、レイトレーシングの際に使用される。
/// メッシュごとに作成され、シーン内の個々のオブジェクトの形状を効率的に表現するために使用される。
/// </summary>
class BottomLevelAccelerationStructure {
public:
    BottomLevelAccelerationStructure();
    ~BottomLevelAccelerationStructure();

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
    /// <param name="device">デバイス</param>
    /// <param name="commandList">コマンドリスト</param>
    /// <param name="_vertexBuffStartAddress">頂点バッファ開始アドレス</param>
    /// <param name="_vertexCount">頂点数</param>
    /// <param name="_indexBuffStartAddress">インデックスバッファ開始アドレス</param>
    /// <param name="_indexCount">インデックス数</param>
    /// <param name="_allowUpdate">更新を許可するか</param>
    void CreateResource(
        ID3D12Device8* device,
        ID3D12GraphicsCommandList6* commandList,
        D3D12_GPU_VIRTUAL_ADDRESS _vertexBuffStartAddress,
        UINT _vertexCount,
        D3D12_GPU_VIRTUAL_ADDRESS _indexBuffStartAddress,
        UINT _indexCount,
        bool _allowUpdate);

    /// <summary>
    /// 更新処理
    /// </summary>
    /// <param name="commandList">コマンドリスト</param>
    void Update(ID3D12GraphicsCommandList6* commandList);

private:
    DxResource scratchResource_; // 生成時リソース (D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESCに格納されるため必要)
    DxResource resultResource_; // BLASの結果リソース

    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs_{};
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC buildDesc_{};
    D3D12_RAYTRACING_GEOMETRY_DESC geometryDesc_{};

    bool allowUpdate_ = false;

public:
    /// <summary>
    /// 結果リソースの取得
    /// </summary>
    /// <returns>リソース</returns>
    const DxResource& GetResultResource() const { return resultResource_; }
};

} // namespace OriGine
