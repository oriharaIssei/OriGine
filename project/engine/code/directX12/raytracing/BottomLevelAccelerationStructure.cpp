#include "BottomLevelAccelerationStructure.h"

using namespace OriGine;

BottomLevelAccelerationStructure::BottomLevelAccelerationStructure() {}
BottomLevelAccelerationStructure::~BottomLevelAccelerationStructure() {}

void BottomLevelAccelerationStructure::Initialize() {
    // 特に無し
    // FinalizeがあるからInitializeを用意する
}

void BottomLevelAccelerationStructure::Finalize() {
    scratchResource_.Finalize();
    resultResource_.Finalize();
}

void BottomLevelAccelerationStructure::CreateResource(
    ID3D12Device8* device,
    ID3D12GraphicsCommandList6* commandList,
    D3D12_GPU_VIRTUAL_ADDRESS _vertexBuffStartAddress,
    UINT _vertexCount,
    D3D12_GPU_VIRTUAL_ADDRESS _indexBuffStartAddress,
    UINT _indexCount,
    bool _allowUpdate) {

    allowUpdate_ = _allowUpdate;

    // ジオメトリ設定
    geometryDesc_ = {};

    // descの設定
    {
        geometryDesc_                                      = {};
        geometryDesc_.Type                                 = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
        geometryDesc_.Triangles.VertexBuffer.StartAddress  = _vertexBuffStartAddress;
        geometryDesc_.Triangles.VertexBuffer.StrideInBytes = sizeof(TextureColorVertexData);
        geometryDesc_.Triangles.VertexFormat               = DXGI_FORMAT_R32G32B32_FLOAT; // 頂点(pos)
        geometryDesc_.Triangles.VertexCount                = _vertexCount;

        geometryDesc_.Triangles.IndexBuffer = _indexBuffStartAddress;
        geometryDesc_.Triangles.IndexCount  = _indexCount;
        geometryDesc_.Triangles.IndexFormat = DXGI_FORMAT_R32_UINT;
        geometryDesc_.Flags                 = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;
    }

    // Inputs設定
    {
        inputs_                = {};
        inputs_.Type           = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
        inputs_.Flags          = allowUpdate_ ? D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE : D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
        inputs_.NumDescs       = 1;
        inputs_.pGeometryDescs = &geometryDesc_;
    }
    // バッファサイズ取得
    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO buildInfo{};
    device->GetRaytracingAccelerationStructurePrebuildInfo(&inputs_, &buildInfo);

    // リソース作成処理
    scratchResource_.CreateUAVBuffer(device, buildInfo.ScratchDataSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON);
    resultResource_.CreateUAVBuffer(device, buildInfo.ResultDataMaxSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE);

    // buildDesc設定
    buildDesc_.DestAccelerationStructureData    = resultResource_.GetResource()->GetGPUVirtualAddress();
    buildDesc_.ScratchAccelerationStructureData = scratchResource_.GetResource()->GetGPUVirtualAddress();
    buildDesc_.SourceAccelerationStructureData  = 0;
    buildDesc_.Inputs                           = inputs_;

    // build実行
    commandList->BuildRaytracingAccelerationStructure(&buildDesc_, 0, nullptr);

    // UAVバリア
    D3D12_RESOURCE_BARRIER uavBarrier{};
    uavBarrier.Type          = D3D12_RESOURCE_BARRIER_TYPE_UAV;
    uavBarrier.UAV.pResource = resultResource_.GetResource().Get();
    commandList->ResourceBarrier(1, &uavBarrier);
}

void BottomLevelAccelerationStructure::Update(ID3D12GraphicsCommandList6* commandList) {
    if (!allowUpdate_) {
        return;
    }

    // 前フレームのresultをsourceとして指定
    buildDesc_.SourceAccelerationStructureData = resultResource_.GetResource()->GetGPUVirtualAddress();
    buildDesc_.Inputs.Flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE;
    commandList->BuildRaytracingAccelerationStructure(&buildDesc_, 0, nullptr);

    // バリア遷移
    D3D12_RESOURCE_BARRIER uavBarrier{};
    uavBarrier.Type          = D3D12_RESOURCE_BARRIER_TYPE_UAV;
    uavBarrier.UAV.pResource = resultResource_.GetResource().Get();
    commandList->ResourceBarrier(1, &uavBarrier);
}
