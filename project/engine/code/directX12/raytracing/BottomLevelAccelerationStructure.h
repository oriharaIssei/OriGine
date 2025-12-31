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

    void Initialize();
    void Finalize();

    /// <summary>
    /// リソース作成
    /// </summary>
    /// <param name="device"></param>
    /// <param name="commandList"></param>
    /// <param name="_mesh"></param>
    /// <param name="_skinningAnimation">MeshがSkinningを利用するならポインターを登録する。</param>
    void CreateResource(
        ID3D12Device8* device,
        ID3D12GraphicsCommandList6* commandList,
        D3D12_GPU_VIRTUAL_ADDRESS _vertexBuffStartAddress,
        UINT _vertexCount,
        D3D12_GPU_VIRTUAL_ADDRESS _indexBuffStartAddress,
        UINT _indexCount,
        bool _allowUpdate);

    void Update(ID3D12GraphicsCommandList6* commandList);

private:
    DxResource scratchResource_; // 生成時リソース (D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESCに格納されるため必要)
    DxResource resultResource_; // BLASの結果リソース

    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs_{};
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC buildDesc_{};
    D3D12_RAYTRACING_GEOMETRY_DESC geometryDesc_{};

    bool allowUpdate_ = false;

public:
    const DxResource& GetResultResource() const { return resultResource_; }
};

} // namespace OriGine
